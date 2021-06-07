#include <windows.h>
#include <stdio.h>
#include "fps_limiter.h"
#include "dd.h"
#include "ddsurface.h"
#include "opengl_utils.h"
#include "utils.h"
#include "wndproc.h"
#include "debug.h"


DWORD WINAPI gdi_render_main(void)
{
    static DWORD warning_end_tick = 0;
    static char warning_text[512] = { 0 };

    if (g_ddraw->show_driver_warning)
    {
        g_ddraw->show_driver_warning = FALSE;

        warning_end_tick = timeGetTime() + (15 * 1000);

        if (!g_ddraw->windowed)
            PostMessage(g_ddraw->hwnd, WM_AUTORENDERER, 0, 0);

        _snprintf(
            warning_text, sizeof(warning_text), 
            "-WARNING- Using slow software rendering, please update your graphics card driver (%s)", 
            strlen(g_oglu_version) > 10 ? "" : g_oglu_version);
    }

    Sleep(500);

    fpsl_init();

    DWORD timeout = g_ddraw->render.minfps > 0 ? g_ddraw->render.minfps_tick_len : INFINITE;

    while (g_ddraw->render.run &&
        (g_ddraw->render.minfps < 0 || WaitForSingleObject(g_ddraw->render.sem, timeout) != WAIT_FAILED))
    {
#if _DEBUG
        dbg_draw_frame_info_start();
#endif

        fpsl_frame_start();

        EnterCriticalSection(&g_ddraw->cs);

        if (g_ddraw->primary && (g_ddraw->bpp == 16 || g_ddraw->bpp == 32 || g_ddraw->primary->palette))
        {
            if (warning_end_tick)
            {
                if (timeGetTime() < warning_end_tick)
                {
                    HDC primary_dc;
                    dds_GetDC(g_ddraw->primary, &primary_dc);

                    RECT rc = { 0, 0, g_ddraw->width, g_ddraw->height };
                    DrawText(primary_dc, warning_text, -1, &rc, DT_NOCLIP | DT_CENTER);
                }
                else
                {
                    warning_end_tick = 0;
                }
            }

            BOOL upscale_hack = g_ddraw->vhack && util_detect_low_res_screen();

            if (g_ddraw->vhack)
                InterlockedExchange(&g_ddraw->upscale_hack_active, upscale_hack);

            if (!g_ddraw->handlemouse)
            {
                g_ddraw->child_window_exists = FALSE;
                EnumChildWindows(g_ddraw->hwnd, util_enum_child_proc, (LPARAM)g_ddraw->primary);
            }

            if (g_ddraw->primary->palette)
            {
                memcpy(&g_ddraw->primary->bmi->bmiColors[0], g_ddraw->primary->palette->data_rgb, 256 * sizeof(int));
            }

            if (g_ddraw->gdilinear)
            {
                SetStretchBltMode(g_ddraw->render.hdc, HALFTONE);
                SetBrushOrgEx(g_ddraw->render.hdc, 0, 0, NULL);
            }

            if (g_ddraw->bnet_active)
            {
                RECT rc = { 0, 0, g_ddraw->render.width, g_ddraw->render.height };
                FillRect(g_ddraw->render.hdc, &rc, (HBRUSH)GetStockObject(BLACK_BRUSH));
            }
            else if (upscale_hack)
            {
                StretchDIBits(
                    g_ddraw->render.hdc, 
                    g_ddraw->render.viewport.x, 
                    g_ddraw->render.viewport.y,
                    g_ddraw->render.viewport.width, 
                    g_ddraw->render.viewport.height,
                    0, 
                    g_ddraw->height - g_ddraw->upscale_hack_height,
                    g_ddraw->upscale_hack_width,
                    g_ddraw->upscale_hack_height,
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

        fpsl_frame_end();
    }

    return TRUE;
}
