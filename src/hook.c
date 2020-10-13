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

void hook_create(char *module_name, char *function_name, PROC new_function, PROC *function)
{
#ifdef _MSC_VER
    if (g_hook_method == 2)
    {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach((PVOID *)function, (PVOID)new_function);
        DetourTransactionCommit();
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
                        hook_patch_iat(hmod, module_name, function_name, new_function);
                    }
                }
            }
        }
    }
#endif

    if (g_hook_method == 1)
    {
        hook_patch_iat(GetModuleHandle(NULL), module_name, function_name, new_function);
        hook_patch_iat(GetModuleHandle("storm.dll"), module_name, function_name, new_function);
    }
}

void hook_revert(char *module_name, char *function_name, PROC new_function, PROC *function)
{
#ifdef _MSC_VER
    if (g_hook_method == 2)
    {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourDetach((PVOID *)function, (PVOID)new_function);
        DetourTransactionCommit();
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
                        hook_patch_iat(
                            hmod,
                            module_name,
                            function_name,
                            GetProcAddress(GetModuleHandle(module_name), function_name));
                    }
                }
            }
        }
    }
#endif

    if (g_hook_method == 1)
    {
        hook_patch_iat(
            GetModuleHandle(NULL), 
            module_name, 
            function_name, 
            GetProcAddress(GetModuleHandle(module_name), function_name));

        hook_patch_iat(
            GetModuleHandle("storm.dll"),
            module_name,
            function_name,
            GetProcAddress(GetModuleHandle(module_name), function_name));
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

        hook_create("user32.dll", "GetCursorPos", (PROC)fake_GetCursorPos, (PROC *)&real_GetCursorPos);
        hook_create("user32.dll", "ClipCursor", (PROC)fake_ClipCursor, (PROC *)&real_ClipCursor);
        hook_create("user32.dll", "ShowCursor", (PROC)fake_ShowCursor, (PROC *)&real_ShowCursor);
        hook_create("user32.dll", "SetCursor", (PROC)fake_SetCursor, (PROC *)&real_SetCursor);
        hook_create("user32.dll", "GetWindowRect", (PROC)fake_GetWindowRect, (PROC *)&real_GetWindowRect);
        hook_create("user32.dll", "GetClientRect", (PROC)fake_GetClientRect, (PROC *)&real_GetClientRect);
        hook_create("user32.dll", "ClientToScreen", (PROC)fake_ClientToScreen, (PROC *)&real_ClientToScreen);
        hook_create("user32.dll", "ScreenToClient", (PROC)fake_ScreenToClient, (PROC *)&real_ScreenToClient);
        hook_create("user32.dll", "SetCursorPos", (PROC)fake_SetCursorPos, (PROC *)&real_SetCursorPos);
        hook_create("user32.dll", "GetClipCursor", (PROC)fake_GetClipCursor, (PROC *)&real_GetClipCursor);
        hook_create("user32.dll", "WindowFromPoint", (PROC)fake_WindowFromPoint, (PROC *)&real_WindowFromPoint);
        hook_create("user32.dll", "GetCursorInfo", (PROC)fake_GetCursorInfo, (PROC *)&real_GetCursorInfo);
        hook_create("user32.dll", "GetSystemMetrics", (PROC)fake_GetSystemMetrics, (PROC *)&real_GetSystemMetrics);
        hook_create("user32.dll", "SetWindowPos", (PROC)fake_SetWindowPos, (PROC *)&real_SetWindowPos);
        hook_create("user32.dll", "MoveWindow", (PROC)fake_MoveWindow, (PROC *)&real_MoveWindow);
        hook_create("user32.dll", "SendMessageA", (PROC)fake_SendMessageA, (PROC *)&real_SendMessageA);
        hook_create("user32.dll", "SetWindowLongA", (PROC)fake_SetWindowLongA, (PROC *)&real_SetWindowLongA);
        hook_create("user32.dll", "EnableWindow", (PROC)fake_EnableWindow, (PROC *)&real_EnableWindow);
        hook_create("user32.dll", "CreateWindowExA", (PROC)fake_CreateWindowExA, (PROC *)&real_CreateWindowExA);
        hook_create("user32.dll", "DestroyWindow", (PROC)fake_DestroyWindow, (PROC *)&real_DestroyWindow);
        hook_create("gdi32.dll",  "GetDeviceCaps", (PROC)fake_GetDeviceCaps, (PROC*)&real_GetDeviceCaps);

        if (g_hook_method == 4)
        {
            hook_create("kernel32.dll", "LoadLibraryA", (PROC)fake_LoadLibraryA, (PROC*)&real_LoadLibraryA);
            hook_create("kernel32.dll", "LoadLibraryW", (PROC)fake_LoadLibraryW, (PROC*)&real_LoadLibraryW);
            hook_create("kernel32.dll", "LoadLibraryExA", (PROC)fake_LoadLibraryExA, (PROC*)&real_LoadLibraryExA);
            hook_create("kernel32.dll", "LoadLibraryExW", (PROC)fake_LoadLibraryExW, (PROC*)&real_LoadLibraryExW);
        }
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

        hook_revert("user32.dll", "GetCursorPos", (PROC)fake_GetCursorPos, (PROC *)&real_GetCursorPos);
        hook_revert("user32.dll", "ClipCursor", (PROC)fake_ClipCursor, (PROC *)&real_ClipCursor);
        hook_revert("user32.dll", "ShowCursor", (PROC)fake_ShowCursor, (PROC *)&real_ShowCursor);
        hook_revert("user32.dll", "SetCursor", (PROC)fake_SetCursor, (PROC *)&real_SetCursor);
        hook_revert("user32.dll", "GetWindowRect", (PROC)fake_GetWindowRect, (PROC *)&real_GetWindowRect);
        hook_revert("user32.dll", "GetClientRect", (PROC)fake_GetClientRect, (PROC *)&real_GetClientRect);
        hook_revert("user32.dll", "ClientToScreen", (PROC)fake_ClientToScreen, (PROC *)&real_ClientToScreen);
        hook_revert("user32.dll", "ScreenToClient", (PROC)fake_ScreenToClient, (PROC *)&real_ScreenToClient);
        hook_revert("user32.dll", "SetCursorPos", (PROC)fake_SetCursorPos, (PROC *)&real_SetCursorPos);
        hook_revert("user32.dll", "GetClipCursor", (PROC)fake_GetClipCursor, (PROC *)&real_GetClipCursor);
        hook_revert("user32.dll", "WindowFromPoint", (PROC)fake_WindowFromPoint, (PROC *)&real_WindowFromPoint);
        hook_revert("user32.dll", "GetCursorInfo", (PROC)fake_GetCursorInfo, (PROC *)&real_GetCursorInfo);
        hook_revert("user32.dll", "GetSystemMetrics", (PROC)fake_GetSystemMetrics, (PROC *)&real_GetSystemMetrics);
        hook_revert("user32.dll", "SetWindowPos", (PROC)fake_SetWindowPos, (PROC *)&real_SetWindowPos);
        hook_revert("user32.dll", "MoveWindow", (PROC)fake_MoveWindow, (PROC *)&real_MoveWindow);
        hook_revert("user32.dll", "SendMessageA", (PROC)fake_SendMessageA, (PROC *)&real_SendMessageA);
        hook_revert("user32.dll", "SetWindowLongA", (PROC)fake_SetWindowLongA, (PROC *)&real_SetWindowLongA);
        hook_revert("user32.dll", "EnableWindow", (PROC)fake_EnableWindow, (PROC *)&real_EnableWindow);
        hook_revert("user32.dll", "CreateWindowExA", (PROC)fake_CreateWindowExA, (PROC *)&real_CreateWindowExA);
        hook_revert("user32.dll", "DestroyWindow", (PROC)fake_DestroyWindow, (PROC *)&real_DestroyWindow);
        hook_revert("gdi32.dll",  "GetDeviceCaps", (PROC)fake_GetDeviceCaps, (PROC*)&real_GetDeviceCaps);

        if (g_hook_method == 4)
        {
            hook_revert("kernel32.dll", "LoadLibraryA", (PROC)fake_LoadLibraryA, (PROC*)&real_LoadLibraryA);
            hook_revert("kernel32.dll", "LoadLibraryW", (PROC)fake_LoadLibraryW, (PROC*)&real_LoadLibraryW);
            hook_revert("kernel32.dll", "LoadLibraryExA", (PROC)fake_LoadLibraryExA, (PROC*)&real_LoadLibraryExA);
            hook_revert("kernel32.dll", "LoadLibraryExW", (PROC)fake_LoadLibraryExW, (PROC*)&real_LoadLibraryExW);
        }
    }
}
