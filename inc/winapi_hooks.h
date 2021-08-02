#ifndef WINAPI_HOOKS_H
#define WINAPI_HOOKS_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>


BOOL WINAPI fake_GetCursorPos(LPPOINT lpPoint);
BOOL WINAPI fake_ClipCursor(const RECT* lpRect);
int WINAPI fake_ShowCursor(BOOL bShow);
HCURSOR WINAPI fake_SetCursor(HCURSOR hCursor);
BOOL WINAPI fake_GetWindowRect(HWND hWnd, LPRECT lpRect);
BOOL WINAPI fake_GetClientRect(HWND hWnd, LPRECT lpRect);
BOOL WINAPI fake_ClientToScreen(HWND hWnd, LPPOINT lpPoint);
BOOL WINAPI fake_ScreenToClient(HWND hWnd, LPPOINT lpPoint);
BOOL WINAPI fake_SetCursorPos(int X, int Y);
HWND WINAPI fake_WindowFromPoint(POINT Point);
BOOL WINAPI fake_GetClipCursor(LPRECT lpRect);
BOOL WINAPI fake_GetCursorInfo(PCURSORINFO pci);
int WINAPI fake_GetSystemMetrics(int nIndex);
BOOL WINAPI fake_SetWindowPos(HWND hWnd, HWND hWndInsertAfter, int X, int Y, int cx, int cy, UINT uFlags);
BOOL WINAPI fake_MoveWindow(HWND hWnd, int X, int Y, int nWidth, int nHeight, BOOL bRepaint);
LRESULT WINAPI fake_SendMessageA(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
LONG WINAPI fake_SetWindowLongA(HWND hWnd, int nIndex, LONG dwNewLong);
LONG WINAPI fake_GetWindowLongA(HWND hWnd, int nIndex);
BOOL WINAPI fake_EnableWindow(HWND hWnd, BOOL bEnable);
BOOL WINAPI fake_DestroyWindow(HWND hWnd);
int WINAPI fake_MapWindowPoints(HWND hWndFrom, HWND hWndTo, LPPOINT lpPoints, UINT cPoints);
HHOOK WINAPI fake_SetWindowsHookExA(int idHook, HOOKPROC lpfn, HINSTANCE hmod, DWORD dwThreadId);
int WINAPI fake_GetDeviceCaps(HDC hdc, int index);
HMODULE WINAPI fake_LoadLibraryA(LPCSTR lpLibFileName);
HMODULE WINAPI fake_LoadLibraryW(LPCWSTR lpLibFileName);
HMODULE WINAPI fake_LoadLibraryExA(LPCSTR lpLibFileName, HANDLE hFile, DWORD dwFlags);
HMODULE WINAPI fake_LoadLibraryExW(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags);
HWND WINAPI fake_CreateWindowExA(
    DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int X, int Y,
    int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam);
HRESULT WINAPI fake_CoCreateInstance(
    REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID* ppv);

#endif
