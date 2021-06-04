#ifndef HOOK_H
#define HOOK_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>


typedef struct hook_list_data { char function_name[32]; PROC new_function; PROC* function; } hook_list_data;
typedef struct hook_list { char module_name[32]; BOOL enabled; hook_list_data data[24]; } hook_list;

typedef BOOL (WINAPI* GETCURSORPOSPROC)(LPPOINT);
typedef BOOL(WINAPI* CLIPCURSORPROC)(const RECT*);
typedef int (WINAPI* SHOWCURSORPROC)(BOOL);
typedef HCURSOR (WINAPI* SETCURSORPROC)(HCURSOR);
typedef BOOL (WINAPI* GETWINDOWRECTPROC)(HWND, LPRECT);
typedef BOOL (WINAPI* GETCLIENTRECTPROC)(HWND, LPRECT);
typedef BOOL (WINAPI* CLIENTTOSCREENPROC)(HWND, LPPOINT);
typedef BOOL (WINAPI* SCREENTOCLIENTPROC)(HWND, LPPOINT);
typedef BOOL (WINAPI* SETCURSORPOSPROC)(int, int);
typedef HWND (WINAPI* WINDOWFROMPOINTPROC)(POINT);
typedef BOOL (WINAPI* GETCLIPCURSORPROC)(LPRECT);
typedef BOOL (WINAPI* GETCURSORINFOPROC)(PCURSORINFO);
typedef int (WINAPI* GETSYSTEMMETRICSPROC)(int);
typedef BOOL (WINAPI* SETWINDOWPOSPROC)(HWND, HWND, int, int, int, int, UINT);
typedef BOOL (WINAPI* MOVEWINDOWPROC)(HWND, int, int, int, int, BOOL);
typedef LRESULT (WINAPI* SENDMESSAGEAPROC)(HWND, UINT, WPARAM, LPARAM);
typedef LONG (WINAPI* SETWINDOWLONGAPROC)(HWND, int, LONG);
typedef LONG (WINAPI* GETWINDOWLONGAPROC)(HWND, int);
typedef BOOL (WINAPI* ENABLEWINDOWPROC)(HWND, BOOL);
typedef HWND (WINAPI* CREATEWINDOWEXAPROC)(DWORD, LPCSTR, LPCSTR, DWORD, int, int, int, int, HWND, HMENU, HINSTANCE, LPVOID);
typedef BOOL (WINAPI* DESTROYWINDOWPROC)(HWND);
typedef int (WINAPI* MAPWINDOWPOINTSPROC)(HWND, HWND, LPPOINT, UINT);
typedef int (WINAPI* GETDEVICECAPSPROC)(HDC, int);
typedef HMODULE (WINAPI* LOADLIBRARYAPROC)(LPCSTR);
typedef HMODULE (WINAPI* LOADLIBRARYWPROC)(LPCWSTR);
typedef HMODULE (WINAPI* LOADLIBRARYEXAPROC)(LPCSTR, HANDLE, DWORD);
typedef HMODULE (WINAPI* LOADLIBRARYEXWPROC)(LPCWSTR, HANDLE, DWORD);

extern GETCURSORPOSPROC real_GetCursorPos;
extern CLIPCURSORPROC real_ClipCursor;
extern SHOWCURSORPROC real_ShowCursor;
extern SETCURSORPROC real_SetCursor;
extern GETWINDOWRECTPROC real_GetWindowRect;
extern GETCLIENTRECTPROC real_GetClientRect;
extern CLIENTTOSCREENPROC real_ClientToScreen;
extern SCREENTOCLIENTPROC real_ScreenToClient;
extern SETCURSORPOSPROC real_SetCursorPos;
extern WINDOWFROMPOINTPROC real_WindowFromPoint;
extern GETCLIPCURSORPROC real_GetClipCursor;
extern GETCURSORINFOPROC real_GetCursorInfo;
extern GETSYSTEMMETRICSPROC real_GetSystemMetrics;
extern SETWINDOWPOSPROC real_SetWindowPos;
extern MOVEWINDOWPROC real_MoveWindow;
extern SENDMESSAGEAPROC real_SendMessageA;
extern SETWINDOWLONGAPROC real_SetWindowLongA;
extern GETWINDOWLONGAPROC real_GetWindowLongA;
extern ENABLEWINDOWPROC real_EnableWindow;
extern CREATEWINDOWEXAPROC real_CreateWindowExA;
extern DESTROYWINDOWPROC real_DestroyWindow;
extern MAPWINDOWPOINTSPROC real_MapWindowPoints;
extern GETDEVICECAPSPROC real_GetDeviceCaps;
extern LOADLIBRARYAPROC real_LoadLibraryA;
extern LOADLIBRARYWPROC real_LoadLibraryW;
extern LOADLIBRARYEXAPROC real_LoadLibraryExA;
extern LOADLIBRARYEXWPROC real_LoadLibraryExW;

extern int g_hook_method;
extern BOOL g_hook_active;

void hook_init();
void hook_early_init();
void hook_exit();
void hook_patch_iat(HMODULE hmod, BOOL unhook, char* module_name, char* function_name, PROC new_function);
void hook_patch_iat_list(HMODULE hmod, BOOL unhook, hook_list* hooks);
void hook_create(hook_list* hooks);
void hook_revert(hook_list* hooks);

#endif
