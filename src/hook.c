#include <windows.h>
#include <stdio.h>
#include "main.h"
#include "mouse.h"
#include "hook.h"

#ifdef _MSC_VER
#include "detours.h"
#endif

BOOL Hook_Active;
int HookingMethod = 1;
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


void Hook_PatchIAT(HMODULE hMod, char *moduleName, char *functionName, PROC newFunction)
{
    if (!hMod || hMod == INVALID_HANDLE_VALUE || !newFunction)
        return;

#ifdef _MSC_VER
    __try
    {
#endif
        PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)hMod;
        if (pDosHeader->e_magic != IMAGE_DOS_SIGNATURE)
            return;

        PIMAGE_NT_HEADERS pNTHeaders = (PIMAGE_NT_HEADERS)((DWORD)pDosHeader + (DWORD)pDosHeader->e_lfanew);
        if (pNTHeaders->Signature != IMAGE_NT_SIGNATURE)
            return;

        PIMAGE_IMPORT_DESCRIPTOR pImportDescriptor = (PIMAGE_IMPORT_DESCRIPTOR)((DWORD)pDosHeader +
            (DWORD)(pNTHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress));

        if (pImportDescriptor == (PIMAGE_IMPORT_DESCRIPTOR)pNTHeaders)
            return;

        while (pImportDescriptor->FirstThunk)
        {
            char* impModuleName = (char*)((DWORD)pDosHeader + (DWORD)(pImportDescriptor->Name));

            if (_stricmp(impModuleName, moduleName) == 0)
            {
                PIMAGE_THUNK_DATA pFirstThunk =
                    (PIMAGE_THUNK_DATA)((DWORD)pDosHeader + (DWORD)pImportDescriptor->FirstThunk);

                PIMAGE_THUNK_DATA pOrigFirstThunk =
                    (PIMAGE_THUNK_DATA)((DWORD)pDosHeader + (DWORD)pImportDescriptor->OriginalFirstThunk);

                while (pFirstThunk->u1.Function && pOrigFirstThunk->u1.AddressOfData)
                {
                    PIMAGE_IMPORT_BY_NAME pImport =
                        (PIMAGE_IMPORT_BY_NAME)((DWORD)pDosHeader + pOrigFirstThunk->u1.AddressOfData);

                    if ((pOrigFirstThunk->u1.Ordinal & IMAGE_ORDINAL_FLAG) == 0 &&
                        _stricmp((const char*)pImport->Name, functionName) == 0)
                    {
                        DWORD oldProtect;

                        if (VirtualProtect(&pFirstThunk->u1.Function, sizeof(DWORD), PAGE_READWRITE, &oldProtect))
                        {
                            pFirstThunk->u1.Function = (DWORD)newFunction;
                            VirtualProtect(&pFirstThunk->u1.Function, sizeof(DWORD), oldProtect, &oldProtect);
                        }

                        break;
                    }

                    pFirstThunk++;
                    pOrigFirstThunk++;
                }
            }

            pImportDescriptor++;
        }
#ifdef _MSC_VER
    }
    __except (EXCEPTION_EXECUTE_HANDLER) 
    {
    }
#endif
}

void Hook_Create(char *moduleName, char *functionName, PROC newFunction, PROC *function)
{
#ifdef _MSC_VER
    if (HookingMethod == 2)
    {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach((PVOID *)function, (PVOID)newFunction);
        DetourTransactionCommit();
    }

    if (HookingMethod == 3 || HookingMethod == 4)
    {
        WCHAR gameExePath[MAX_PATH] = { 0 };
        WCHAR gameDir[MAX_PATH] = { 0 };

        if (GetModuleFileNameW(NULL, gameExePath, MAX_PATH))
        {
            _wsplitpath(gameExePath, NULL, gameDir, NULL, NULL);

            WCHAR modPath[MAX_PATH] = { 0 };
            WCHAR modDir[MAX_PATH] = { 0 };
            HMODULE hMod = NULL;

            while (hMod = DetourEnumerateModules(hMod))
            {
                if (hMod == DDrawModule)
                    continue;

                if (GetModuleFileNameW(hMod, modPath, MAX_PATH))
                {
                    _wsplitpath(modPath, NULL, modDir, NULL, NULL);

                    if (_wcsnicmp(gameDir, modDir, wcslen(gameDir)) == 0)
                    {
                        Hook_PatchIAT(hMod, moduleName, functionName, newFunction);
                    }
                }
            }
        }
    }
#endif

    if (HookingMethod == 1)
    {
        Hook_PatchIAT(GetModuleHandle(NULL), moduleName, functionName, newFunction);
        Hook_PatchIAT(GetModuleHandle("storm.dll"), moduleName, functionName, newFunction);
    }
}

void Hook_Revert(char *moduleName, char *functionName, PROC newFunction, PROC *function)
{
#ifdef _MSC_VER
    if (HookingMethod == 2)
    {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourDetach((PVOID *)function, (PVOID)newFunction);
        DetourTransactionCommit();
    }

    if (HookingMethod == 3 || HookingMethod == 4)
    {
        WCHAR gameExePath[MAX_PATH] = { 0 };
        WCHAR gameDir[MAX_PATH] = { 0 };

        if (GetModuleFileNameW(NULL, gameExePath, MAX_PATH))
        {
            _wsplitpath(gameExePath, NULL, gameDir, NULL, NULL);

            WCHAR modPath[MAX_PATH] = { 0 };
            WCHAR modDir[MAX_PATH] = { 0 };
            HMODULE hMod = NULL;

            while (hMod = DetourEnumerateModules(hMod))
            {
                if (hMod == DDrawModule)
                    continue;

                if (GetModuleFileNameW(hMod, modPath, MAX_PATH))
                {
                    _wsplitpath(modPath, NULL, modDir, NULL, NULL);

                    if (_wcsnicmp(gameDir, modDir, wcslen(gameDir)) == 0)
                    {
                        Hook_PatchIAT(
                            hMod,
                            moduleName,
                            functionName,
                            GetProcAddress(GetModuleHandle(moduleName), functionName));
                    }
                }
            }
        }
    }
#endif

    if (HookingMethod == 1)
    {
        Hook_PatchIAT(
            GetModuleHandle(NULL), 
            moduleName, 
            functionName, 
            GetProcAddress(GetModuleHandle(moduleName), functionName));

        Hook_PatchIAT(
            GetModuleHandle("storm.dll"),
            moduleName,
            functionName,
            GetProcAddress(GetModuleHandle(moduleName), functionName));
    }
}

void Hook_Init()
{
    if (!Hook_Active || HookingMethod == 3 || HookingMethod == 4)
    {
#ifdef _MSC_VER
        if (!Hook_Active && HookingMethod == 3)
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

        Hook_Active = TRUE;

        Hook_Create("user32.dll", "GetCursorPos", (PROC)fake_GetCursorPos, (PROC *)&real_GetCursorPos);
        Hook_Create("user32.dll", "ClipCursor", (PROC)fake_ClipCursor, (PROC *)&real_ClipCursor);
        Hook_Create("user32.dll", "ShowCursor", (PROC)fake_ShowCursor, (PROC *)&real_ShowCursor);
        Hook_Create("user32.dll", "SetCursor", (PROC)fake_SetCursor, (PROC *)&real_SetCursor);
        Hook_Create("user32.dll", "GetWindowRect", (PROC)fake_GetWindowRect, (PROC *)&real_GetWindowRect);
        Hook_Create("user32.dll", "GetClientRect", (PROC)fake_GetClientRect, (PROC *)&real_GetClientRect);
        Hook_Create("user32.dll", "ClientToScreen", (PROC)fake_ClientToScreen, (PROC *)&real_ClientToScreen);
        Hook_Create("user32.dll", "ScreenToClient", (PROC)fake_ScreenToClient, (PROC *)&real_ScreenToClient);
        Hook_Create("user32.dll", "SetCursorPos", (PROC)fake_SetCursorPos, (PROC *)&real_SetCursorPos);
        Hook_Create("user32.dll", "GetClipCursor", (PROC)fake_GetClipCursor, (PROC *)&real_GetClipCursor);
        Hook_Create("user32.dll", "WindowFromPoint", (PROC)fake_WindowFromPoint, (PROC *)&real_WindowFromPoint);
        Hook_Create("user32.dll", "GetCursorInfo", (PROC)fake_GetCursorInfo, (PROC *)&real_GetCursorInfo);
        Hook_Create("user32.dll", "GetSystemMetrics", (PROC)fake_GetSystemMetrics, (PROC *)&real_GetSystemMetrics);
        Hook_Create("user32.dll", "SetWindowPos", (PROC)fake_SetWindowPos, (PROC *)&real_SetWindowPos);
        Hook_Create("user32.dll", "MoveWindow", (PROC)fake_MoveWindow, (PROC *)&real_MoveWindow);
        Hook_Create("user32.dll", "SendMessageA", (PROC)fake_SendMessageA, (PROC *)&real_SendMessageA);
        Hook_Create("user32.dll", "SetWindowLongA", (PROC)fake_SetWindowLongA, (PROC *)&real_SetWindowLongA);
        Hook_Create("user32.dll", "EnableWindow", (PROC)fake_EnableWindow, (PROC *)&real_EnableWindow);
        Hook_Create("user32.dll", "CreateWindowExA", (PROC)fake_CreateWindowExA, (PROC *)&real_CreateWindowExA);
        Hook_Create("user32.dll", "DestroyWindow", (PROC)fake_DestroyWindow, (PROC *)&real_DestroyWindow);
        Hook_Create("gdi32.dll",  "GetDeviceCaps", (PROC)fake_GetDeviceCaps, (PROC*)&real_GetDeviceCaps);

        if (HookingMethod == 4)
        {
            Hook_Create("kernel32.dll", "LoadLibraryA", (PROC)fake_LoadLibraryA, (PROC*)&real_LoadLibraryA);
            Hook_Create("kernel32.dll", "LoadLibraryW", (PROC)fake_LoadLibraryW, (PROC*)&real_LoadLibraryW);
            Hook_Create("kernel32.dll", "LoadLibraryExA", (PROC)fake_LoadLibraryExA, (PROC*)&real_LoadLibraryExA);
            Hook_Create("kernel32.dll", "LoadLibraryExW", (PROC)fake_LoadLibraryExW, (PROC*)&real_LoadLibraryExW);
        }
    }
}

void Hook_Exit()
{
    if (Hook_Active)
    {
        Hook_Active = FALSE;

#ifdef _MSC_VER
        if (HookingMethod == 3)
        {
            DetourTransactionBegin();
            DetourUpdateThread(GetCurrentThread());
            DetourDetach((PVOID*)&real_LoadLibraryExW, (PVOID)fake_LoadLibraryExW);
            DetourTransactionCommit();
        }
#endif

        Hook_Revert("user32.dll", "GetCursorPos", (PROC)fake_GetCursorPos, (PROC *)&real_GetCursorPos);
        Hook_Revert("user32.dll", "ClipCursor", (PROC)fake_ClipCursor, (PROC *)&real_ClipCursor);
        Hook_Revert("user32.dll", "ShowCursor", (PROC)fake_ShowCursor, (PROC *)&real_ShowCursor);
        Hook_Revert("user32.dll", "SetCursor", (PROC)fake_SetCursor, (PROC *)&real_SetCursor);
        Hook_Revert("user32.dll", "GetWindowRect", (PROC)fake_GetWindowRect, (PROC *)&real_GetWindowRect);
        Hook_Revert("user32.dll", "GetClientRect", (PROC)fake_GetClientRect, (PROC *)&real_GetClientRect);
        Hook_Revert("user32.dll", "ClientToScreen", (PROC)fake_ClientToScreen, (PROC *)&real_ClientToScreen);
        Hook_Revert("user32.dll", "ScreenToClient", (PROC)fake_ScreenToClient, (PROC *)&real_ScreenToClient);
        Hook_Revert("user32.dll", "SetCursorPos", (PROC)fake_SetCursorPos, (PROC *)&real_SetCursorPos);
        Hook_Revert("user32.dll", "GetClipCursor", (PROC)fake_GetClipCursor, (PROC *)&real_GetClipCursor);
        Hook_Revert("user32.dll", "WindowFromPoint", (PROC)fake_WindowFromPoint, (PROC *)&real_WindowFromPoint);
        Hook_Revert("user32.dll", "GetCursorInfo", (PROC)fake_GetCursorInfo, (PROC *)&real_GetCursorInfo);
        Hook_Revert("user32.dll", "GetSystemMetrics", (PROC)fake_GetSystemMetrics, (PROC *)&real_GetSystemMetrics);
        Hook_Revert("user32.dll", "SetWindowPos", (PROC)fake_SetWindowPos, (PROC *)&real_SetWindowPos);
        Hook_Revert("user32.dll", "MoveWindow", (PROC)fake_MoveWindow, (PROC *)&real_MoveWindow);
        Hook_Revert("user32.dll", "SendMessageA", (PROC)fake_SendMessageA, (PROC *)&real_SendMessageA);
        Hook_Revert("user32.dll", "SetWindowLongA", (PROC)fake_SetWindowLongA, (PROC *)&real_SetWindowLongA);
        Hook_Revert("user32.dll", "EnableWindow", (PROC)fake_EnableWindow, (PROC *)&real_EnableWindow);
        Hook_Revert("user32.dll", "CreateWindowExA", (PROC)fake_CreateWindowExA, (PROC *)&real_CreateWindowExA);
        Hook_Revert("user32.dll", "DestroyWindow", (PROC)fake_DestroyWindow, (PROC *)&real_DestroyWindow);
        Hook_Revert("gdi32.dll",  "GetDeviceCaps", (PROC)fake_GetDeviceCaps, (PROC*)&real_GetDeviceCaps);

        if (HookingMethod == 4)
        {
            Hook_Revert("kernel32.dll", "LoadLibraryA", (PROC)fake_LoadLibraryA, (PROC*)&real_LoadLibraryA);
            Hook_Revert("kernel32.dll", "LoadLibraryW", (PROC)fake_LoadLibraryW, (PROC*)&real_LoadLibraryW);
            Hook_Revert("kernel32.dll", "LoadLibraryExA", (PROC)fake_LoadLibraryExA, (PROC*)&real_LoadLibraryExA);
            Hook_Revert("kernel32.dll", "LoadLibraryExW", (PROC)fake_LoadLibraryExW, (PROC*)&real_LoadLibraryExW);
        }
    }
}
