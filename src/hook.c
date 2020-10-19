#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include "dd.h"
#include "winapi_hooks.h"
#include "hook.h"
#include "dllmain.h"

#ifdef _MSC_VER
#include "detours.h"
#endif

BOOL g_hook_active;
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
ENABLEWINDOWPROC real_EnableWindow = EnableWindow;
CREATEWINDOWEXAPROC real_CreateWindowExA = CreateWindowExA;
DESTROYWINDOWPROC real_DestroyWindow = DestroyWindow;
GETDEVICECAPSPROC real_GetDeviceCaps = GetDeviceCaps;
LOADLIBRARYAPROC real_LoadLibraryA = LoadLibraryA;
LOADLIBRARYWPROC real_LoadLibraryW = LoadLibraryW;
LOADLIBRARYEXAPROC real_LoadLibraryExA = LoadLibraryExA;
LOADLIBRARYEXWPROC real_LoadLibraryExW = LoadLibraryExW;

static hook_list g_hooks[] =
{
    {
        "user32.dll",
        TRUE,
        {
            { "GetCursorPos", (PROC)fake_GetCursorPos, (PROC*)&real_GetCursorPos },
            { "ClipCursor", (PROC)fake_ClipCursor, (PROC*)&real_ClipCursor },
            { "ShowCursor", (PROC)fake_ShowCursor, (PROC*)&real_ShowCursor },
            { "SetCursor", (PROC)fake_SetCursor, (PROC*)&real_SetCursor },
            { "GetWindowRect", (PROC)fake_GetWindowRect, (PROC*)&real_GetWindowRect },
            { "GetClientRect", (PROC)fake_GetClientRect, (PROC*)&real_GetClientRect },
            { "ClientToScreen", (PROC)fake_ClientToScreen, (PROC*)&real_ClientToScreen },
            { "ScreenToClient", (PROC)fake_ScreenToClient, (PROC*)&real_ScreenToClient },
            { "SetCursorPos", (PROC)fake_SetCursorPos, (PROC*)&real_SetCursorPos },
            { "GetClipCursor", (PROC)fake_GetClipCursor, (PROC*)&real_GetClipCursor },
            { "WindowFromPoint", (PROC)fake_WindowFromPoint, (PROC*)&real_WindowFromPoint },
            { "GetCursorInfo", (PROC)fake_GetCursorInfo, (PROC*)&real_GetCursorInfo },
            { "GetSystemMetrics", (PROC)fake_GetSystemMetrics, (PROC*)&real_GetSystemMetrics },
            { "SetWindowPos", (PROC)fake_SetWindowPos, (PROC*)&real_SetWindowPos },
            { "MoveWindow", (PROC)fake_MoveWindow, (PROC*)&real_MoveWindow },
            { "SendMessageA", (PROC)fake_SendMessageA, (PROC*)&real_SendMessageA },
            { "SetWindowLongA", (PROC)fake_SetWindowLongA, (PROC*)&real_SetWindowLongA },
            { "EnableWindow", (PROC)fake_EnableWindow, (PROC*)&real_EnableWindow },
            { "CreateWindowExA", (PROC)fake_CreateWindowExA, (PROC*)&real_CreateWindowExA },
            { "DestroyWindow", (PROC)fake_DestroyWindow, (PROC*)&real_DestroyWindow },
            { "", NULL }
        }
    },
    {
        "gdi32.dll",
        TRUE,
        {
            { "GetDeviceCaps", (PROC)fake_GetDeviceCaps, (PROC*)&real_GetDeviceCaps },
            { "", NULL }
        }
    },
    {
        "kernel32.dll",
        FALSE,
        {
            { "LoadLibraryA", (PROC)fake_LoadLibraryA, (PROC*)&real_LoadLibraryA },
            { "LoadLibraryW", (PROC)fake_LoadLibraryW, (PROC*)&real_LoadLibraryW },
            { "LoadLibraryExA", (PROC)fake_LoadLibraryExA, (PROC*)&real_LoadLibraryExA },
            { "LoadLibraryExW", (PROC)fake_LoadLibraryExW, (PROC*)&real_LoadLibraryExW },
            { "", NULL }
        }
    },
    {
        "",
        FALSE,
        {
            { "", NULL }
        }
    }
};

void hook_patch_iat(HMODULE hmod, char *module_name, char *function_name, PROC new_function)
{
    if (!hmod || hmod == INVALID_HANDLE_VALUE || !new_function)
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
            char* imp_module_name = (char*)((DWORD)dos_header + (DWORD)(import_desc->Name));

            if (_stricmp(imp_module_name, module_name) == 0)
            {
                PIMAGE_THUNK_DATA first_thunk =
                    (PIMAGE_THUNK_DATA)((DWORD)dos_header + (DWORD)import_desc->FirstThunk);

                PIMAGE_THUNK_DATA original_first_thunk =
                    (PIMAGE_THUNK_DATA)((DWORD)dos_header + (DWORD)import_desc->OriginalFirstThunk);

                while (first_thunk->u1.Function && original_first_thunk->u1.AddressOfData)
                {
                    PIMAGE_IMPORT_BY_NAME import =
                        (PIMAGE_IMPORT_BY_NAME)((DWORD)dos_header + original_first_thunk->u1.AddressOfData);

                    if ((original_first_thunk->u1.Ordinal & IMAGE_ORDINAL_FLAG) == 0 &&
                        _stricmp((const char*)import->Name, function_name) == 0)
                    {
                        DWORD old_protect;

                        if (VirtualProtect(&first_thunk->u1.Function, sizeof(DWORD), PAGE_READWRITE, &old_protect))
                        {
                            first_thunk->u1.Function = (DWORD)new_function;
                            VirtualProtect(&first_thunk->u1.Function, sizeof(DWORD), old_protect, &old_protect);
                        }

                        break;
                    }

                    first_thunk++;
                    original_first_thunk++;
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

void hook_create(hook_list* hooks)
{
#ifdef _MSC_VER
    if (g_hook_method == 2)
    {
        for (int i = 0; hooks[i].module_name[0]; i++)
        {
            if (!hooks[i].enabled)
                continue;

            for (int x = 0; hooks[i].data[x].function_name[0]; x++)
            {
                DetourTransactionBegin();
                DetourUpdateThread(GetCurrentThread());
                DetourAttach((PVOID*)hooks[i].data[x].function, (PVOID)hooks[i].data[x].new_function);
                DetourTransactionCommit();
            }
        }
    }

    if (g_hook_method == 3 || g_hook_method == 4)
    {
        WCHAR game_exe_path[MAX_PATH] = { 0 };
        WCHAR game_dir[MAX_PATH] = { 0 };

        if (GetModuleFileNameW(NULL, game_exe_path, MAX_PATH))
        {
            _wsplitpath(game_exe_path, NULL, game_dir, NULL, NULL);

            WCHAR mod_path[MAX_PATH] = { 0 };
            WCHAR mod_dir[MAX_PATH] = { 0 };
            HMODULE hmod = NULL;

            while (hmod = DetourEnumerateModules(hmod))
            {
                if (hmod == g_ddraw_module)
                    continue;

                if (GetModuleFileNameW(hmod, mod_path, MAX_PATH))
                {
                    _wsplitpath(mod_path, NULL, mod_dir, NULL, NULL);

                    if (_wcsnicmp(game_dir, mod_dir, wcslen(game_dir)) == 0)
                    {
                        for (int i = 0; hooks[i].module_name[0]; i++)
                        {
                            if (!hooks[i].enabled)
                                continue;

                            for (int x = 0; hooks[i].data[x].function_name[0]; x++)
                            {
                                hook_patch_iat(
                                    hmod,
                                    hooks[i].module_name,
                                    hooks[i].data[x].function_name,
                                    hooks[i].data[x].new_function);
                            }
                        }
                    }
                }
            }
        }
    }
#endif

    if (g_hook_method == 1)
    {
        for (int i = 0; hooks[i].module_name[0]; i++)
        {
            if (!hooks[i].enabled)
                continue;

            for (int x = 0; hooks[i].data[x].function_name[0]; x++)
            {
                hook_patch_iat(
                    GetModuleHandle(NULL), 
                    hooks[i].module_name, 
                    hooks[i].data[x].function_name,
                    hooks[i].data[x].new_function);
            }
        }
    }
}

void hook_revert(hook_list* hooks)
{
#ifdef _MSC_VER
    if (g_hook_method == 2)
    {
        for (int i = 0; hooks[i].module_name[0]; i++)
        {
            if (!hooks[i].enabled)
                continue;

            for (int x = 0; hooks[i].data[x].function_name[0]; x++)
            {
                DetourTransactionBegin();
                DetourUpdateThread(GetCurrentThread());
                DetourDetach((PVOID*)hooks[i].data[x].function, (PVOID)hooks[i].data[x].new_function);
                DetourTransactionCommit();
            }
        }
    }

    if (g_hook_method == 3 || g_hook_method == 4)
    {
        WCHAR game_exe_path[MAX_PATH] = { 0 };
        WCHAR game_dir[MAX_PATH] = { 0 };

        if (GetModuleFileNameW(NULL, game_exe_path, MAX_PATH))
        {
            _wsplitpath(game_exe_path, NULL, game_dir, NULL, NULL);

            WCHAR mod_path[MAX_PATH] = { 0 };
            WCHAR mod_dir[MAX_PATH] = { 0 };
            HMODULE hmod = NULL;

            while (hmod = DetourEnumerateModules(hmod))
            {
                if (hmod == g_ddraw_module)
                    continue;

                if (GetModuleFileNameW(hmod, mod_path, MAX_PATH))
                {
                    _wsplitpath(mod_path, NULL, mod_dir, NULL, NULL);

                    if (_wcsnicmp(game_dir, mod_dir, wcslen(game_dir)) == 0)
                    {
                        for (int i = 0; hooks[i].module_name[0]; i++)
                        {
                            if (!hooks[i].enabled)
                                continue;

                            for (int x = 0; hooks[i].data[x].function_name[0]; x++)
                            {
                                hook_patch_iat(
                                    hmod,
                                    hooks[i].module_name,
                                    hooks[i].data[x].function_name,
                                    GetProcAddress(GetModuleHandle(hooks[i].module_name), hooks[i].data[x].function_name));
                            }
                        }
                    }
                }
            }
        }
    }
#endif

    if (g_hook_method == 1)
    {
        for (int i = 0; hooks[i].module_name[0]; i++)
        {
            if (!hooks[i].enabled)
                continue;

            for (int x = 0; hooks[i].data[x].function_name[0]; x++)
            {
                hook_patch_iat(
                    GetModuleHandle(NULL),
                    hooks[i].module_name,
                    hooks[i].data[x].function_name,
                    GetProcAddress(GetModuleHandle(hooks[i].module_name), hooks[i].data[x].function_name));
            }
        }
    }
}

void hook_init()
{
    if (!g_hook_active || g_hook_method == 3 || g_hook_method == 4)
    {
#ifdef _MSC_VER
        if (!g_hook_active && g_hook_method == 3)
        {
            FARPROC proc = GetProcAddress(GetModuleHandle("kernelbase.dll"), "LoadLibraryExW");

            if (proc)
                real_LoadLibraryExW = (LOADLIBRARYEXWPROC)proc;

            DetourTransactionBegin();
            DetourUpdateThread(GetCurrentThread());
            DetourAttach((PVOID*)&real_LoadLibraryExW, (PVOID)fake_LoadLibraryExW);
            DetourTransactionCommit();
        }
#endif

        g_hook_active = TRUE;

        if (g_hook_method == 4)
        {
            for (int i = 0; g_hooks[i].module_name[0]; i++)
            {
                if (_stricmp(g_hooks[i].module_name, "kernel32.dll") == 0)
                {
                    g_hooks[i].enabled = TRUE;
                }
            }
        }

        hook_create((hook_list*)&g_hooks);
    }
}

void hook_exit()
{
    if (g_hook_active)
    {
        g_hook_active = FALSE;

#ifdef _MSC_VER
        if (g_hook_method == 3)
        {
            DetourTransactionBegin();
            DetourUpdateThread(GetCurrentThread());
            DetourDetach((PVOID*)&real_LoadLibraryExW, (PVOID)fake_LoadLibraryExW);
            DetourTransactionCommit();
        }
#endif

        hook_revert((hook_list*)&g_hooks);
    }
}
