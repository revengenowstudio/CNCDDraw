#ifndef FPS_LIMITER_H
#define FPS_LIMITER_H

#include <windows.h>
#include <dwmapi.h>


typedef struct _D3DKMT_WAITFORVERTICALBLANKEVENT {
    UINT hAdapter;
    UINT hDevice;
    UINT VidPnSourceId;
} D3DKMT_WAITFORVERTICALBLANKEVENT;

typedef struct _D3DKMT_OPENADAPTERFROMHDC {
    HDC  hDc;
    UINT hAdapter;
    LUID AdapterLuid;
    UINT VidPnSourceId;
} D3DKMT_OPENADAPTERFROMHDC;

typedef struct _D3DKMT_CLOSEADAPTER {
    UINT hAdapter;
} D3DKMT_CLOSEADAPTER;

typedef struct fps_limiter
{
    DWORD tick_start;
    DWORD tick_end;
    DWORD tick_length;
    LONGLONG tick_length_ns;
    HANDLE htimer;
    LARGE_INTEGER due_time;
    D3DKMT_WAITFORVERTICALBLANKEVENT vblank_event;
    D3DKMT_OPENADAPTERFROMHDC adapter;
    D3DKMT_CLOSEADAPTER close_adapter;
    HMODULE gdi32_dll;
    HMODULE dwmapi_dll;
    HRESULT(WINAPI* DwmFlush)(VOID);
    HRESULT(WINAPI* DwmIsCompositionEnabled)(BOOL*);
    NTSTATUS(WINAPI* D3DKMTWaitForVerticalBlankEvent)(const D3DKMT_WAITFORVERTICALBLANKEVENT* Arg1);
    NTSTATUS(WINAPI* D3DKMTOpenAdapterFromHdc)(D3DKMT_OPENADAPTERFROMHDC* Arg1);
    NTSTATUS(WINAPI* D3DKMTCloseAdapter)(D3DKMT_CLOSEADAPTER* Arg1);
    BOOL got_adapter;
} fps_limiter;

extern fps_limiter g_fpsl;

void fpsl_init();
BOOL fpsl_wait_for_vblank();
BOOL fpsl_dwm_is_enabled();
void fpsl_frame_start();
void fpsl_frame_end();

#endif
