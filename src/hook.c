#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <psapi.h>
#include "directinput.h"
#include "dd.h"
#include "winapi_hooks.h"
#include "hook.h"
#include "debug.h"
#include "dllmain.h"

#ifdef _MSC_VER
#include "detours.h"
#endif

BOOL g_hook_active;
BOOL g_hook_dinput;
int g_hook_method = 1;

GETCURSORPOSPROC real_GetCursorPos = GetCursorPos;
CLIPCURSORPROC real_ClipCursor = ClipCursor;
SHOWCURSORPROC real_ShowCursor = ShowCursor;
SETCURSORPROC real_SetCursor = SetCursor;
GETWINDOWRECTPROC real_GetWindowRect = GetWindowRect;
GETCLIENTRECTPROC real_GetClientRect = GetClientRect;
CLIENTTOSCREENPROC real_ClientToScreen = ClientToScreen;
SCREENTOCLIENTPROC real_ScreenToClient = ScreenToClient;
SETCURSORPOSPROC real_SetCursorPos = SetCursorPos;
WINDOWFROMPOINTPROC real_WindowFromPoint = WindowFromPoint;
GETCLIPCURSORPROC real_GetClipCursor = GetClipCursor;
GETCURSORINFOPROC real_GetCursorInfo = GetCursorInfo;
GETSYSTEMMETRICSPROC real_GetSystemMetrics = GetSystemMetrics;
SETWINDOWPOSPROC real_SetWindowPos = SetWindowPos;
MOVEWINDOWPROC real_MoveWindow = MoveWindow;
SENDMESSAGEAPROC real_SendMessageA = SendMessageA;
SETWINDOWLONGAPROC real_SetWindowLongA = SetWindowLongA;
GETWINDOWLONGAPROC real_GetWindowLongA = GetWindowLongA;
ENABLEWINDOWPROC real_EnableWindow = EnableWindow;
CREATEWINDOWEXAPROC real_CreateWindowExA = CreateWindowExA;
DESTROYWINDOWPROC real_DestroyWindow = DestroyWindow;
MAPWINDOWPOINTSPROC real_MapWindowPoints = MapWindowPoints;
SHOWWINDOWPROC real_ShowWindow = ShowWindow;
SETWINDOWSHOOKEXAPROC real_SetWindowsHookExA = SetWindowsHookExA;
GETDEVICECAPSPROC real_GetDeviceCaps = GetDeviceCaps;
LOADLIBRARYAPROC real_LoadLibraryA = LoadLibraryA;
LOADLIBRARYWPROC real_LoadLibraryW = LoadLibraryW;
LOADLIBRARYEXAPROC real_LoadLibraryExA = LoadLibraryExA;
LOADLIBRARYEXWPROC real_LoadLibraryExW = LoadLibraryExW;
COCREATEINSTANCEPROC real_CoCreateInstance = CoCreateInstance;

static HOOKLIST g_hooks[] =
{
    {
        "user32.dll",
        {
            { "GetCursorPos", (PROC)fake_GetCursorPos, (PROC*)&real_GetCursorPos, 0 },
            { "ClipCursor", (PROC)fake_ClipCursor, (PROC*)&real_ClipCursor, 0 },
            { "ShowCursor", (PROC)fake_ShowCursor, (PROC*)&real_ShowCursor, 0 },
            { "SetCursor", (PROC)fake_SetCursor, (PROC*)&real_SetCursor, 0 },
            { "GetWindowRect", (PROC)fake_GetWindowRect, (PROC*)&real_GetWindowRect, SKIP_HOOK3 },
            { "GetClientRect", (PROC)fake_GetClientRect, (PROC*)&real_GetClientRect, SKIP_HOOK3 },
            { "ClientToScreen", (PROC)fake_ClientToScreen, (PROC*)&real_ClientToScreen, 0 },
            { "ScreenToClient", (PROC)fake_ScreenToClient, (PROC*)&real_ScreenToClient, 0 },
            { "SetCursorPos", (PROC)fake_SetCursorPos, (PROC*)&real_SetCursorPos, 0 },
            { "GetClipCursor", (PROC)fake_GetClipCursor, (PROC*)&real_GetClipCursor, 0 },
            { "WindowFromPoint", (PROC)fake_WindowFromPoint, (PROC*)&real_WindowFromPoint, 0 },
            { "GetCursorInfo", (PROC)fake_GetCursorInfo, (PROC*)&real_GetCursorInfo, 0 },
            { "GetSystemMetrics", (PROC)fake_GetSystemMetrics, (PROC*)&real_GetSystemMetrics, 0 },
            { "SetWindowPos", (PROC)fake_SetWindowPos, (PROC*)&real_SetWindowPos, 0 },
            { "MoveWindow", (PROC)fake_MoveWindow, (PROC*)&real_MoveWindow, 0 },
            { "SendMessageA", (PROC)fake_SendMessageA, (PROC*)&real_SendMessageA, 0 },
            { "SetWindowLongA", (PROC)fake_SetWindowLongA, (PROC*)&real_SetWindowLongA, 0 },
            { "GetWindowLongA", (PROC)fake_GetWindowLongA, (PROC*)&real_GetWindowLongA, 0 },
            { "EnableWindow", (PROC)fake_EnableWindow, (PROC*)&real_EnableWindow, 0 },
            { "CreateWindowExA", (PROC)fake_CreateWindowExA, (PROC*)&real_CreateWindowExA, 0 },
            { "DestroyWindow", (PROC)fake_DestroyWindow, (PROC*)&real_DestroyWindow, 0 },
            { "MapWindowPoints", (PROC)fake_MapWindowPoints, (PROC*)&real_MapWindowPoints, 0 },
            { "ShowWindow", (PROC)fake_ShowWindow, (PROC*)&real_ShowWindow, 0 },
            { "", NULL, NULL, 0 }
        }
    },
    {
        "gdi32.dll",
        {
            { "GetDeviceCaps", (PROC)fake_GetDeviceCaps, (PROC*)&real_GetDeviceCaps, SKIP_HOOK3 },
            { "", NULL, NULL, 0 }
        }
    },
    {
        "kernel32.dll",
        {
            { "LoadLibraryA", (PROC)fake_LoadLibraryA, (PROC*)&real_LoadLibraryA, SKIP_HOOK2 | SKIP_HOOK3 },
            { "LoadLibraryW", (PROC)fake_LoadLibraryW, (PROC*)&real_LoadLibraryW, SKIP_HOOK2 | SKIP_HOOK3 },
            { "LoadLibraryExA", (PROC)fake_LoadLibraryExA, (PROC*)&real_LoadLibraryExA, SKIP_HOOK2 | SKIP_HOOK3 },
            { "LoadLibraryExW", (PROC)fake_LoadLibraryExW, (PROC*)&real_LoadLibraryExW, SKIP_HOOK2 | SKIP_HOOK3 },
            { "", NULL, NULL, 0 }
        }
    },
    {
        "",
        {
            { "", NULL, NULL, 0 }
        }
    }
};

void hook_patch_iat(HMODULE hmod, BOOL unhook, char* module_name, char* function_name, PROC new_function)
{
    HOOKLIST hooks[2];
    memset(&hooks, 0, sizeof(hooks));

    hooks[0].data[0].new_function = new_function;

    strncpy(hooks[0].module_name, module_name, sizeof(hooks[0].module_name) - 1);
    strncpy(hooks[0].data[0].function_name, function_name, sizeof(hooks[0].data[0].function_name) - 1);

    hook_patch_iat_list(hmod, unhook, (HOOKLIST*)&hooks);
}

void hook_patch_obfuscated_iat_list(HMODULE hmod, BOOL unhook, HOOKLIST* hooks)
{
    if (!hmod || hmod == INVALID_HANDLE_VALUE || !hooks)
        return;

#ifdef _MSC_VER
    __try
    {
#endif
        PIMAGE_DOS_HEADER dos_header = (PIMAGE_DOS_HEADER)hmod;
        if (dos_header->e_magic != IMAGE_DOS_SIGNATURE)
            return;

        PIMAGE_NT_HEADERS nt_headers = (PIMAGE_NT_HEADERS)((DWORD)dos_header + (DWORD)dos_header->e_lfanew);
        if (nt_headers->Signature != IMAGE_NT_SIGNATURE)
            return;

        PIMAGE_IMPORT_DESCRIPTOR import_desc = (PIMAGE_IMPORT_DESCRIPTOR)((DWORD)dos_header +
            (DWORD)(nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress));

        if (import_desc == (PIMAGE_IMPORT_DESCRIPTOR)nt_headers)
            return;

        while (import_desc->FirstThunk)
        {
            for (int i = 0; hooks[i].module_name[0]; i++)
            {
                char* imp_module_name = (char*)((DWORD)dos_header + (DWORD)(import_desc->Name));

                if (_stricmp(imp_module_name, hooks[i].module_name) == 0)
                {
                    HMODULE cur_mod = GetModuleHandle(hooks[i].module_name);

                    PIMAGE_THUNK_DATA first_thunk =
                        (PIMAGE_THUNK_DATA)((DWORD)dos_header + (DWORD)import_desc->FirstThunk);

                    PIMAGE_THUNK_DATA original_first_thunk =
                        (PIMAGE_THUNK_DATA)((DWORD)dos_header + (DWORD)import_desc->OriginalFirstThunk);

                    while (first_thunk->u1.Function)
                    {
                        for (int x = 0; hooks[i].data[x].function_name[0]; x++)
                        {
                            DWORD org_function =
                                (DWORD)GetProcAddress(
                                    cur_mod,
                                    hooks[i].data[x].function_name);

                            if (!hooks[i].data[x].new_function || !org_function)
                                continue;

                            if (unhook)
                            {
                                if (first_thunk->u1.Function == (DWORD)hooks[i].data[x].new_function)
                                {
                                    DWORD op;

                                    if (VirtualProtect(
                                        &first_thunk->u1.Function, 
                                        sizeof(DWORD), 
                                        PAGE_READWRITE, 
                                        &op))
                                    {
                                        first_thunk->u1.Function = org_function;

                                        VirtualProtect(&first_thunk->u1.Function, sizeof(DWORD), op, &op);
                                    }

                                    break;
                                }
                            }
                            else
                            {
                                if (first_thunk->u1.Function == org_function)
                                {
                                    DWORD op;

                                    if (VirtualProtect(
                                        &first_thunk->u1.Function, 
                                        sizeof(DWORD), 
                                        PAGE_READWRITE, 
                                        &op))
                                    {
                                        first_thunk->u1.Function = (DWORD)hooks[i].data[x].new_function;

                                        VirtualProtect(&first_thunk->u1.Function, sizeof(DWORD), op, &op);
                                    }

                                    break;
                                }
                            }
                        }

                        first_thunk++;
                        original_first_thunk++;
                    }
                }
            }

            import_desc++;
        }
#ifdef _MSC_VER
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
    }
#endif
}

void hook_patch_iat_list(HMODULE hmod, BOOL unhook, HOOKLIST* hooks)
{
    hook_patch_obfuscated_iat_list(hmod, unhook, hooks);

    if (!hmod || hmod == INVALID_HANDLE_VALUE || !hooks)
        return;

#ifdef _MSC_VER
    __try
    {
#endif
        PIMAGE_DOS_HEADER dos_header = (PIMAGE_DOS_HEADER)hmod;
        if (dos_header->e_magic != IMAGE_DOS_SIGNATURE)
            return;

        PIMAGE_NT_HEADERS nt_headers = (PIMAGE_NT_HEADERS)((DWORD)dos_header + (DWORD)dos_header->e_lfanew);
        if (nt_headers->Signature != IMAGE_NT_SIGNATURE)
            return;

        PIMAGE_IMPORT_DESCRIPTOR import_desc = (PIMAGE_IMPORT_DESCRIPTOR)((DWORD)dos_header +
            (DWORD)(nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress));

        if (import_desc == (PIMAGE_IMPORT_DESCRIPTOR)nt_headers)
            return;

        while (import_desc->FirstThunk)
        {
            for (int i = 0; hooks[i].module_name[0]; i++)
            {
                char* imp_module_name = (char*)((DWORD)dos_header + (DWORD)(import_desc->Name));

                if (_stricmp(imp_module_name, hooks[i].module_name) == 0)
                {
                    PIMAGE_THUNK_DATA first_thunk =
                        (PIMAGE_THUNK_DATA)((DWORD)dos_header + (DWORD)import_desc->FirstThunk);

                    PIMAGE_THUNK_DATA original_first_thunk =
                        (PIMAGE_THUNK_DATA)((DWORD)dos_header + (DWORD)import_desc->OriginalFirstThunk);

                    while (first_thunk->u1.Function && original_first_thunk->u1.AddressOfData)
                    {
                        PIMAGE_IMPORT_BY_NAME import =
                            (PIMAGE_IMPORT_BY_NAME)((DWORD)dos_header + original_first_thunk->u1.AddressOfData);

                        if ((original_first_thunk->u1.Ordinal & IMAGE_ORDINAL_FLAG) == 0)
                        {
                            for (int x = 0; hooks[i].data[x].function_name[0]; x++)
                            {
                                if (!unhook && !hooks[i].data[x].new_function)
                                    continue;

                                if (_stricmp((const char*)import->Name, hooks[i].data[x].function_name) == 0)
                                {
                                    DWORD op;

                                    if (VirtualProtect(
                                        &first_thunk->u1.Function, 
                                        sizeof(DWORD), 
                                        PAGE_READWRITE, 
                                        &op))
                                    {
                                        if (unhook)
                                        {
                                            DWORD org =
                                                (DWORD)GetProcAddress(
                                                    GetModuleHandle(hooks[i].module_name),
                                                    hooks[i].data[x].function_name);

                                            if (org && first_thunk->u1.Function != org)
                                            {
                                                first_thunk->u1.Function = org;
                                            }
                                        }
                                        else
                                        {
                                            if (first_thunk->u1.Function != (DWORD)hooks[i].data[x].new_function)
                                                first_thunk->u1.Function = (DWORD)hooks[i].data[x].new_function;
                                        }

                                        VirtualProtect(&first_thunk->u1.Function, sizeof(DWORD), op, &op);
                                    }

                                    break;
                                }
                            }
                        }

                        first_thunk++;
                        original_first_thunk++;
                    }
                }
            }

            import_desc++;
        }
#ifdef _MSC_VER
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
    }
#endif
}

void hook_create(HOOKLIST* hooks, BOOL initial_hook)
{
#ifdef _MSC_VER
    if ((g_hook_method == 2 || g_hook_method == 3) && initial_hook)
    {
        for (int i = 0; hooks[i].module_name[0]; i++)
        {
            for (int x = 0; hooks[i].data[x].function_name[0]; x++)
            {
                if (g_hook_method == 2 && (hooks[i].data[x].flags & SKIP_HOOK2))
                    continue;

                if (g_hook_method == 3 && (hooks[i].data[x].flags & SKIP_HOOK3))
                    continue;

                DetourTransactionBegin();
                DetourUpdateThread(GetCurrentThread());
                DetourAttach((PVOID*)hooks[i].data[x].function, (PVOID)hooks[i].data[x].new_function);
                DetourTransactionCommit();
            }
        }
    }
#endif

    if (g_hook_method == 3 || g_hook_method == 4)
    {
        char game_exe_path[MAX_PATH] = { 0 };
        char game_dir[MAX_PATH] = { 0 };

        if (GetModuleFileNameA(NULL, game_exe_path, MAX_PATH))
        {
            _splitpath(game_exe_path, NULL, game_dir, NULL, NULL);

            char mod_path[MAX_PATH] = { 0 };
            char mod_dir[MAX_PATH] = { 0 };
            char mod_filename[MAX_PATH] = { 0 };
            HMODULE hmod = NULL;
            HANDLE process = NULL;

#ifndef _MSC_VER
            HMODULE mods[512];
            memset(mods, 0, sizeof(mods));
            process = GetCurrentProcess();
            EnumProcessModules(process, mods, sizeof(mods) - sizeof(mods[0]), NULL);
            for (int i = 0; i < sizeof(mods) / sizeof(mods[0]) && (hmod = mods[i]); i++)
#else
            while (hmod = DetourEnumerateModules(hmod))
#endif
            {
                if (hmod == g_ddraw_module)
                    continue;

                if (GetModuleFileNameA(hmod, mod_path, MAX_PATH))
                {
                    TRACE_EXT("Module %s = %p\n", mod_path, hmod);

                    _splitpath(mod_path, NULL, mod_dir, mod_filename, NULL);

                    // Don't hook reshade
                    if (!g_ddraw->enable_reshade && _strcmpi(mod_filename, "d3d9") == 0)
                        continue;

                    // Don't hook swiftshader/mesa3d
                    if (_strcmpi(mod_filename, "opengl32") == 0 ||
                        _strcmpi(mod_filename, "Shw32") == 0)
                        continue;

                    if (_strnicmp(game_dir, mod_dir, strlen(game_dir)) == 0 ||
                        _strcmpi(mod_filename, "quartz") == 0 ||
                        _strcmpi(mod_filename, "winmm") == 0)
                    {
                        hook_patch_iat_list(hmod, FALSE, hooks);
                    }
                }
            }

            if (process)
                CloseHandle(process);
        }
    }

    if (g_hook_method == 1)
    {
        hook_patch_iat_list(GetModuleHandle(NULL), FALSE, hooks);
    }
}

void hook_revert(HOOKLIST* hooks)
{
#ifdef _MSC_VER
    if (g_hook_method == 2 || g_hook_method == 3)
    {
        for (int i = 0; hooks[i].module_name[0]; i++)
        {
            for (int x = 0; hooks[i].data[x].function_name[0]; x++)
            {
                if (g_hook_method == 2 && (hooks[i].data[x].flags & SKIP_HOOK2))
                    continue;

                if (g_hook_method == 3 && (hooks[i].data[x].flags & SKIP_HOOK3))
                    continue;

                DetourTransactionBegin();
                DetourUpdateThread(GetCurrentThread());
                DetourDetach((PVOID*)hooks[i].data[x].function, (PVOID)hooks[i].data[x].new_function);
                DetourTransactionCommit();
            }
        }
    }
#endif

    if (g_hook_method == 3 || g_hook_method == 4)
    {
        char game_exe_path[MAX_PATH] = { 0 };
        char game_dir[MAX_PATH] = { 0 };

        if (GetModuleFileNameA(NULL, game_exe_path, MAX_PATH))
        {
            _splitpath(game_exe_path, NULL, game_dir, NULL, NULL);

            char mod_path[MAX_PATH] = { 0 };
            char mod_dir[MAX_PATH] = { 0 };
            char mod_filename[MAX_PATH] = { 0 };
            HMODULE hmod = NULL;
            HANDLE process = NULL;

#ifndef _MSC_VER
            HMODULE mods[512];
            memset(mods, 0, sizeof(mods));
            process = GetCurrentProcess();
            EnumProcessModules(process, mods, sizeof(mods) - sizeof(mods[0]), NULL);
            for (int i = 0; i < sizeof(mods) / sizeof(mods[0]) && (hmod = mods[i]); i++)
#else
            while (hmod = DetourEnumerateModules(hmod))
#endif
            {
                if (hmod == g_ddraw_module)
                    continue;

                if (GetModuleFileNameA(hmod, mod_path, MAX_PATH))
                {
                    _splitpath(mod_path, NULL, mod_dir, mod_filename, NULL);

                    if (_strnicmp(game_dir, mod_dir, strlen(game_dir)) == 0 ||
                        _strcmpi(mod_filename, "quartz") == 0 ||
                        _strcmpi(mod_filename, "winmm") == 0)
                    {
                        hook_patch_iat_list(hmod, TRUE, hooks);
                    }
                }
            }

            if (process)
                CloseHandle(process);
        }
    }

    if (g_hook_method == 1)
    {
        hook_patch_iat_list(GetModuleHandle(NULL), TRUE, hooks);
    }
}

void hook_init()
{
    if (!g_hook_active || g_hook_method == 3 || g_hook_method == 4)
    {
        BOOL initial_hook = !g_hook_active;

#ifdef _MSC_VER
        if (initial_hook && g_hook_dinput)
        {
            real_DirectInputCreateA =
                (DIRECTINPUTCREATEAPROC)GetProcAddress(LoadLibraryA("dinput.dll"), "DirectInputCreateA");

            if (real_DirectInputCreateA)
            {
                DetourTransactionBegin();
                DetourUpdateThread(GetCurrentThread());
                DetourAttach((PVOID*)&real_DirectInputCreateA, (PVOID)fake_DirectInputCreateA);
                DetourTransactionCommit();
            }

            real_DirectInputCreateW =
                (DIRECTINPUTCREATEWPROC)GetProcAddress(LoadLibraryA("dinput.dll"), "DirectInputCreateW");

            if (real_DirectInputCreateW)
            {
                DetourTransactionBegin();
                DetourUpdateThread(GetCurrentThread());
                DetourAttach((PVOID*)&real_DirectInputCreateW, (PVOID)fake_DirectInputCreateW);
                DetourTransactionCommit();
            }

            real_DirectInputCreateEx =
                (DIRECTINPUTCREATEEXPROC)GetProcAddress(LoadLibraryA("dinput.dll"), "DirectInputCreateEx");

            if (real_DirectInputCreateEx)
            {
                DetourTransactionBegin();
                DetourUpdateThread(GetCurrentThread());
                DetourAttach((PVOID*)&real_DirectInputCreateEx, (PVOID)fake_DirectInputCreateEx);
                DetourTransactionCommit();
            }

            real_DirectInput8Create =
                (DIRECTINPUT8CREATEPROC)GetProcAddress(LoadLibraryA("dinput8.dll"), "DirectInput8Create");

            if (real_DirectInput8Create)
            {
                DetourTransactionBegin();
                DetourUpdateThread(GetCurrentThread());
                DetourAttach((PVOID*)&real_DirectInput8Create, (PVOID)fake_DirectInput8Create);
                DetourTransactionCommit();
            }
        }
#endif

        g_hook_active = TRUE;

        hook_create((HOOKLIST*)&g_hooks, initial_hook);
    }
}

void hook_early_init()
{
    /*
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach((PVOID*)&real_CoCreateInstance, (PVOID)fake_CoCreateInstance);
    DetourTransactionCommit();
    */

    hook_patch_iat(GetModuleHandle(NULL), FALSE, "ole32.dll", "CoCreateInstance", (PROC)fake_CoCreateInstance);
    hook_patch_iat(GetModuleHandle(NULL), FALSE, "dinput.dll", "DirectInputCreateA", (PROC)fake_DirectInputCreateA);
    hook_patch_iat(GetModuleHandle(NULL), FALSE, "dinput.dll", "DirectInputCreateW", (PROC)fake_DirectInputCreateW);
    hook_patch_iat(GetModuleHandle(NULL), FALSE, "dinput.dll", "DirectInputCreateEx", (PROC)fake_DirectInputCreateEx);
    hook_patch_iat(GetModuleHandle(NULL), FALSE, "dinput8.dll", "DirectInput8Create", (PROC)fake_DirectInput8Create);
    hook_patch_iat(GetModuleHandle(NULL), FALSE, "user32.dll", "GetClientRect", (PROC)fake_GetClientRect); //anno 1602
    hook_patch_iat(GetModuleHandle("AcGenral"), FALSE, "user32.dll", "SetWindowsHookExA", (PROC)fake_SetWindowsHookExA);
    hook_patch_iat(GetModuleHandle(NULL), FALSE, "user32.dll", "SetWindowsHookExA", (PROC)fake_SetWindowsHookExA);
}

void hook_exit()
{
    if (g_hook_active)
    {
        g_hook_active = FALSE;

#ifdef _MSC_VER
        if (g_hook_dinput)
        {
            if (real_DirectInputCreateA)
            {
                DetourTransactionBegin();
                DetourUpdateThread(GetCurrentThread());
                DetourDetach((PVOID*)&real_DirectInputCreateA, (PVOID)fake_DirectInputCreateA);
                DetourTransactionCommit();
            }

            if (real_DirectInputCreateW)
            {
                DetourTransactionBegin();
                DetourUpdateThread(GetCurrentThread());
                DetourDetach((PVOID*)&real_DirectInputCreateW, (PVOID)fake_DirectInputCreateW);
                DetourTransactionCommit();
            }

            if (real_DirectInputCreateEx)
            {
                DetourTransactionBegin();
                DetourUpdateThread(GetCurrentThread());
                DetourDetach((PVOID*)&real_DirectInputCreateEx, (PVOID)fake_DirectInputCreateEx);
                DetourTransactionCommit();
            }

            if (real_DirectInput8Create)
            {
                DetourTransactionBegin();
                DetourUpdateThread(GetCurrentThread());
                DetourDetach((PVOID*)&real_DirectInput8Create, (PVOID)fake_DirectInput8Create);
                DetourTransactionCommit();
            }
        }
#endif

        hook_revert((HOOKLIST*)&g_hooks);
    }

    hook_patch_iat(GetModuleHandle(NULL), TRUE, "ole32.dll", "CoCreateInstance", (PROC)fake_CoCreateInstance);
    hook_patch_iat(GetModuleHandle(NULL), TRUE, "dinput.dll", "DirectInputCreateA", (PROC)fake_DirectInputCreateA);
    hook_patch_iat(GetModuleHandle(NULL), TRUE, "dinput.dll", "DirectInputCreateW", (PROC)fake_DirectInputCreateW);
    hook_patch_iat(GetModuleHandle(NULL), TRUE, "dinput.dll", "DirectInputCreateEx", (PROC)fake_DirectInputCreateEx);
    hook_patch_iat(GetModuleHandle(NULL), TRUE, "dinput8.dll", "DirectInput8Create", (PROC)fake_DirectInput8Create);
    hook_patch_iat(GetModuleHandle(NULL), TRUE, "user32.dll", "GetClientRect", (PROC)fake_GetClientRect); //anno 1602
    hook_patch_iat(GetModuleHandle("AcGenral"), TRUE, "user32.dll", "SetWindowsHookExA", (PROC)fake_SetWindowsHookExA);
    hook_patch_iat(GetModuleHandle(NULL), TRUE, "user32.dll", "SetWindowsHookExA", (PROC)fake_SetWindowsHookExA);
}
