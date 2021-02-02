#include <windows.h>
#include "fps_limiter.h"
#include "dd.h"
#include "debug.h"

fps_limiter g_fpsl;

void fpsl_init()
{
    int max_fps = g_ddraw->render.maxfps;

    g_fpsl.tick_length_ns = 0;
    g_fpsl.tick_length = 0;

    if (max_fps < 0 || g_ddraw->vsync)
        max_fps = g_ddraw->mode.dmDisplayFrequency;

    if (max_fps > 1000)
        max_fps = 0;

    if (max_fps > 0)
    {
        float len = 1000.0f / max_fps;
        g_fpsl.tick_length_ns = len * 10000;
        g_fpsl.tick_length = len;// + 0.5f;
    }

    if (g_fpsl.got_adapter && g_fpsl.D3DKMTCloseAdapter)
    {
        g_fpsl.got_adapter = FALSE;
        g_fpsl.close_adapter.hAdapter = g_fpsl.adapter.hAdapter;
        g_fpsl.D3DKMTCloseAdapter(&g_fpsl.close_adapter);
    }

    g_fpsl.DwmFlush =
        (HRESULT(WINAPI*)(VOID))GetProcAddress(GetModuleHandleA("Dwmapi.dll"), "DwmFlush");

    g_fpsl.DwmIsCompositionEnabled =
        (HRESULT(WINAPI*)(BOOL*))GetProcAddress(GetModuleHandleA("Dwmapi.dll"), "DwmIsCompositionEnabled");

    g_fpsl.D3DKMTWaitForVerticalBlankEvent =
        (NTSTATUS(WINAPI*)(const D3DKMT_WAITFORVERTICALBLANKEVENT * Arg1))
        GetProcAddress(GetModuleHandleA("gdi32.dll"), "D3DKMTWaitForVerticalBlankEvent");

    g_fpsl.D3DKMTOpenAdapterFromHdc =
        (NTSTATUS(WINAPI*)(D3DKMT_OPENADAPTERFROMHDC * Arg1))
        GetProcAddress(GetModuleHandleA("gdi32.dll"), "D3DKMTOpenAdapterFromHdc");

    g_fpsl.D3DKMTCloseAdapter =
        (NTSTATUS(WINAPI*)(D3DKMT_CLOSEADAPTER * Arg1))
        GetProcAddress(GetModuleHandleA("gdi32.dll"), "D3DKMTCloseAdapter");
}

BOOL fpsl_wait_for_vblank()
{
    if (g_fpsl.D3DKMTOpenAdapterFromHdc && !g_fpsl.got_adapter)
    {
        g_fpsl.adapter.hDc = g_ddraw->render.hdc;

        if (g_fpsl.D3DKMTOpenAdapterFromHdc(&g_fpsl.adapter) == 0)
        {
            g_fpsl.vblank_event.hAdapter = g_fpsl.adapter.hAdapter;
            g_fpsl.got_adapter = TRUE;
        }
    }

    if (g_fpsl.got_adapter && g_fpsl.D3DKMTWaitForVerticalBlankEvent)
    {
        return g_fpsl.D3DKMTWaitForVerticalBlankEvent(&g_fpsl.vblank_event) == 0;
    }

    return FALSE;
}

BOOL fpsl_dwn_is_enabled()
{
    BOOL dwm_enabled = FALSE;

    if (g_fpsl.DwmIsCompositionEnabled)
        g_fpsl.DwmIsCompositionEnabled(&dwm_enabled);

    return dwm_enabled;
}

void fpsl_frame_start()
{
    if (g_fpsl.tick_length > 0)
        g_fpsl.tick_start = timeGetTime();
}

void fpsl_frame_end()
{
    if (g_ddraw->render.maxfps < 0 || g_ddraw->vsync)
    {
        if (fpsl_dwn_is_enabled() && g_fpsl.DwmFlush && SUCCEEDED(g_fpsl.DwmFlush()))
            return;

        if (fpsl_wait_for_vblank())
            return;
    }

    if (g_fpsl.tick_length > 0)
    {
        if (g_fpsl.htimer)
        {
            if (g_ddraw->vsync)
            {
                WaitForSingleObject(g_fpsl.htimer, g_fpsl.tick_length * 2);
                LARGE_INTEGER due_time = { .QuadPart = -g_fpsl.tick_length_ns };
                SetWaitableTimer(g_fpsl.htimer, &due_time, 0, NULL, NULL, FALSE);
            }
            else
            {
                FILETIME ft = { 0 };
                GetSystemTimeAsFileTime(&ft);

                if (CompareFileTime((FILETIME*)&g_fpsl.due_time, &ft) == -1)
                {
                    memcpy(&g_fpsl.due_time, &ft, sizeof(LARGE_INTEGER));
                }
                else
                {
                    WaitForSingleObject(g_fpsl.htimer, g_fpsl.tick_length * 2);
                }

                g_fpsl.due_time.QuadPart += g_fpsl.tick_length_ns;
                SetWaitableTimer(g_fpsl.htimer, &g_fpsl.due_time, 0, NULL, NULL, FALSE);
            }
        }
        else
        {
            g_fpsl.tick_end = timeGetTime();

            if (g_fpsl.tick_end - g_fpsl.tick_start < g_fpsl.tick_length)
            {
                Sleep(g_fpsl.tick_length - (g_fpsl.tick_end - g_fpsl.tick_start));
            }
        }
    }
}
