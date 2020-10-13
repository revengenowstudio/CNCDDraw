#include <windows.h>
#include <stdio.h>
#include "dd.h"
#include "ddsurface.h"
#include "opengl_utils.h"
#include "utils.h"
#include "wndproc.h"
#include "debug.h"


DWORD WINAPI gdi_render_main(void)
{
    DWORD warning_end_tick = timeGetTime() + (15 * 1000);
    char warning_text[512] = { 0 };

    if (g_ddraw->show_driver_warning)
    {
        g_ddraw->show_driver_warning = FALSE;

        if (!g_ddraw->windowed)
            PostMessage(g_ddraw->hwnd, WM_AUTORENDERER, 0, 0);

        _snprintf(
            warning_text, sizeof(warning_text), 
            "-WARNING- Using slow software rendering, please update your graphics card driver (%s)", 
            strlen(g_oglu_version) > 10 ? "" : g_oglu_version);
    }

    Sleep(500);

    DWORD tick_start = 0;
    DWORD tick_end = 0;

    int max_fps = g_ddraw->render.maxfps;

    g_ddraw->fps_limiter.tick_length_ns = 0;
    g_ddraw->fps_limiter.tick_length = 0;

    if (max_fps < 0)
        max_fps = g_ddraw->mode.dmDisplayFrequency;

    if (max_fps > 1000)
        max_fps = 0;

    if (max_fps > 0)
    {
        float len = 1000.0f / max_fps;
        g_ddraw->fps_limiter.tick_length_ns = len * 10000;
        g_ddraw->fps_limiter.tick_length = len + (g_ddraw->accurate_timers ? 0.5f : 0.0f);
    }

    while (g_ddraw->render.run &&
        (g_ddraw->render.forcefps || WaitForSingleObject(g_ddraw->render.sem, 200) != WAIT_FAILED))
    {
#if _DEBUG
        dbg_draw_frame_info_start();
#endif

        if (g_ddraw->fps_limiter.tick_length > 0)
            tick_start = timeGetTime();

        EnterCriticalSection(&g_ddraw->cs);

        if (g_ddraw->primary && (g_ddraw->bpp == 16 || (g_ddraw->primary->palette && g_ddraw->primary->palette->data_rgb)))
        {
            if (warning_text[0])
            {
                if (timeGetTime() < warning_end_tick)
                {
                    RECT rc = { 0, 0, g_ddraw->width, g_ddraw->height };
                    DrawText(g_ddraw->primary->hdc, warning_text, -1, &rc, DT_NOCLIP | DT_CENTER);
                }
                else
                    warning_text[0] = 0;
            }

            BOOL scale_cutscene = g_ddraw->vhack && util_detect_cutscene();

            if (g_ddraw->vhack)
                InterlockedExchange(&g_ddraw->incutscene, scale_cutscene);

            if (!g_ddraw->handlemouse)
            {
                g_ddraw->child_window_exists = FALSE;
                EnumChildWindows(g_ddraw->hwnd, util_enum_child_proc, (LPARAM)g_ddraw->primary);
            }

            if (g_ddraw->bnet_active)
            {
                RECT rc = { 0, 0, g_ddraw->render.width, g_ddraw->render.height };
                FillRect(g_ddraw->render.hdc, &rc, (HBRUSH)GetStockObject(BLACK_BRUSH));
            }
            else if (scale_cutscene)
            {
                StretchDIBits(
                    g_ddraw->render.hdc, 
                    g_ddraw->render.viewport.x, 
                    g_ddraw->render.viewport.y,
                    g_ddraw->render.viewport.width, 
                    g_ddraw->render.viewport.height,
                    0, 
                    g_ddraw->height - 400, 
                    CUTSCENE_WIDTH, 
                    CUTSCENE_HEIGHT, 
                    g_ddraw->primary->surface,
                    g_ddraw->primary->bmi, 
                    DIB_RGB_COLORS, 
                    SRCCOPY);
            }
            else if (!g_ddraw->child_window_exists && 
                     (g_ddraw->render.width != g_ddraw->width || g_ddraw->render.height != g_ddraw->height))
            {
                StretchDIBits(
                    g_ddraw->render.hdc, 
                    g_ddraw->render.viewport.x, 
                    g_ddraw->render.viewport.y, 
                    g_ddraw->render.viewport.width, 
                    g_ddraw->render.viewport.height, 
                    0, 
                    0, 
                    g_ddraw->width, 
                    g_ddraw->height, 
                    g_ddraw->primary->surface, 
                    g_ddraw->primary->bmi, 
                    DIB_RGB_COLORS, 
                    SRCCOPY);
            }
            else
            {
                SetDIBitsToDevice(
                    g_ddraw->render.hdc, 
                    0, 
                    0, 
                    g_ddraw->width, 
                    g_ddraw->height, 
                    0, 
                    0, 
                    0, 
                    g_ddraw->height, 
                    g_ddraw->primary->surface, 
                    g_ddraw->primary->bmi, 
                    DIB_RGB_COLORS);
            } 
        }

        LeaveCriticalSection(&g_ddraw->cs);

#if _DEBUG
        dbg_draw_frame_info_end();
#endif

        if (g_ddraw->fps_limiter.tick_length > 0)
        {
            if (g_ddraw->fps_limiter.htimer)
            {
                FILETIME ft = { 0 };
                GetSystemTimeAsFileTime(&ft);

                if (CompareFileTime((FILETIME*)&g_ddraw->fps_limiter.due_time, &ft) == -1)
                {
                    memcpy(&g_ddraw->fps_limiter.due_time, &ft, sizeof(LARGE_INTEGER));
                }
                else
                {
                    WaitForSingleObject(g_ddraw->fps_limiter.htimer, g_ddraw->fps_limiter.tick_length * 2);
                }

                g_ddraw->fps_limiter.due_time.QuadPart += g_ddraw->fps_limiter.tick_length_ns;
                SetWaitableTimer(g_ddraw->fps_limiter.htimer, &g_ddraw->fps_limiter.due_time, 0, NULL, NULL, FALSE);
            }
            else
            {
                tick_end = timeGetTime();

                if (tick_end - tick_start < g_ddraw->fps_limiter.tick_length)
                    Sleep(g_ddraw->fps_limiter.tick_length - (tick_end - tick_start));
            }
        }
    }

    return TRUE;
}
