#ifndef DD_H 
#define DD_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "ddraw.h"


ULONG dd_AddRef();
ULONG dd_Release();
HRESULT dd_EnumDisplayModes(DWORD dwFlags, LPDDSURFACEDESC lpDDSurfaceDesc, LPVOID lpContext, LPDDENUMMODESCALLBACK lpEnumModesCallback);
HRESULT dd_WaitForVerticalBlank(DWORD dwFlags, HANDLE h);
HRESULT dd_SetDisplayMode(DWORD width, DWORD height, DWORD bpp);
HRESULT dd_SetCooperativeLevel(HWND hwnd, DWORD dwFlags);
HRESULT dd_RestoreDisplayMode();
HRESULT dd_GetCaps(LPDDCAPS lpDDDriverCaps, LPDDCAPS lpDDEmulCaps);
HRESULT dd_GetDisplayMode(LPDDSURFACEDESC lpDDSurfaceDesc);
HRESULT dd_GetMonitorFrequency(LPDWORD lpdwFreq);
HRESULT dd_GetAvailableVidMem(void* lpDDCaps, LPDWORD lpdwTotal, LPDWORD lpdwFree);
HRESULT dd_GetVerticalBlankStatus(LPBOOL lpbIsInVB);
HRESULT dd_CreateEx(GUID* lpGuid, LPVOID* lplpDD, REFIID iid, IUnknown* pUnkOuter);

typedef struct speed_limiter
{
    DWORD tick_length;
    LONGLONG tick_length_ns;
    HANDLE htimer;
    LARGE_INTEGER due_time;
    BOOL use_blt_or_flip;
} speed_limiter;

struct IDirectDrawSurfaceImpl;

extern struct cnc_ddraw *g_ddraw;

typedef struct cnc_ddraw
{
    ULONG ref;

    DWORD width;
    DWORD height;
    DWORD bpp;
    BOOL windowed;
    BOOL border;
    BOOL boxing;
    DEVMODE mode;
    struct IDirectDrawSurfaceImpl *primary;
    char title[128];
    HMODULE real_dll;

    /* real export from system32\ddraw.dll */
    HRESULT (WINAPI *DirectDrawCreate)(GUID FAR*, LPDIRECTDRAW FAR*, IUnknown FAR*);
    CRITICAL_SECTION cs;

    struct
    {
        int maxfps;
        int minfps;
        DWORD minfps_tick_len;
        int width;
        int height;
        int bpp;

        HDC hdc;
        int *tex;

        HANDLE thread;
        BOOL run;
        HANDLE sem;
        DEVMODE mode;
        struct { int width; int height; int x; int y; } viewport;

        LONG palette_updated;
        LONG surface_updated;

        float scale_w;
        float scale_h;
        float unscale_w;
        float unscale_h;
    } render;

    HWND hwnd;
    WNDPROC wndproc;
    struct { float x; float y; } cursor;
    BOOL locked;
    BOOL adjmouse;
    BOOL devmode;
    BOOL vsync;
    BOOL vhack;
    BOOL isredalert;
    BOOL iscnc1;
    LONG incutscene;
    DWORD (WINAPI *renderer)(void);
    BOOL fullscreen;
    BOOL maintas;
    BOOL noactivateapp;
    BOOL handlemouse;
    char shader[MAX_PATH];
    BOOL wine;
    BOOL altenter;
    BOOL hidecursor;
    BOOL accurate_timers;
    BOOL resizable;
    BOOL sierrahack;
    BOOL dk2hack;
    BOOL nonexclusive;
    BOOL fixchildwindows;
    BOOL d3d9linear;
    int maxgameticks;
    BOOL bnet_active;
    BOOL bnet_was_fullscreen;
    BOOL bnet_was_upscaled;
    RECT bnet_win_rect;
    POINT bnet_pos;
    int mouse_y_adjust;
    void* last_freed_palette; // Dungeon Keeper hack
    BOOL child_window_exists;
    DWORD last_set_window_pos_tick; // WINE hack
    BOOL show_driver_warning;
    speed_limiter ticks_limiter;
    speed_limiter flip_limiter;
    
} cnc_ddraw;

#endif
