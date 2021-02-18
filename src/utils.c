#include <windows.h>
#include "ddraw.h"
#include "dd.h"
#include "ddsurface.h"
#include "hook.h"
#include "mouse.h"
#include "render_d3d9.h"
#include "utils.h"
#include "config.h"


void util_limit_game_ticks()
{
    if (g_ddraw->ticks_limiter.htimer)
    {
        FILETIME ft = { 0 };
        GetSystemTimeAsFileTime(&ft);

        if (CompareFileTime((FILETIME *)&g_ddraw->ticks_limiter.due_time, &ft) == -1)
        {
            memcpy(&g_ddraw->ticks_limiter.due_time, &ft, sizeof(LARGE_INTEGER));
        }
        else
        {
            WaitForSingleObject(g_ddraw->ticks_limiter.htimer, g_ddraw->ticks_limiter.tick_length * 2);
        }

        g_ddraw->ticks_limiter.due_time.QuadPart += g_ddraw->ticks_limiter.tick_length_ns;
        SetWaitableTimer(g_ddraw->ticks_limiter.htimer, &g_ddraw->ticks_limiter.due_time, 0, NULL, NULL, FALSE);
    }
    else
    {
        static DWORD next_game_tick;

        if (!next_game_tick)
        {
            next_game_tick = timeGetTime();
            return;
        }

        next_game_tick += g_ddraw->ticks_limiter.tick_length;
        DWORD tick_count = timeGetTime();

        int sleep_time = next_game_tick - tick_count;

        if (sleep_time <= 0 || sleep_time > g_ddraw->ticks_limiter.tick_length)
        {
            next_game_tick = tick_count;
        }
        else
        {
            Sleep(sleep_time);
        }
    }
}

void util_update_bnet_pos(int new_x, int new_y)
{
    static int old_x = -32000;
    static int old_y = -32000;

    if (old_x == -32000 || old_y == -32000 || !g_ddraw->bnet_active)
    {
        old_x = new_x;
        old_y = new_y;
        return;
    }

    POINT pt = { 0, 0 };
    real_ClientToScreen(g_ddraw->hwnd, &pt);

    RECT mainrc;
    SetRect(&mainrc, pt.x, pt.y, pt.x + g_ddraw->width, pt.y + g_ddraw->height);

    int adj_y = 0;
    int adj_x = 0;

    HWND hwnd = FindWindowEx(HWND_DESKTOP, NULL, "SDlgDialog", NULL);

    while (hwnd != NULL)
    {
        RECT rc;
        real_GetWindowRect(hwnd, &rc);

        OffsetRect(&rc, new_x - old_x, new_y - old_y);

        real_SetWindowPos(
            hwnd,
            0,
            rc.left,
            rc.top,
            0,
            0,
            SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

        if (rc.bottom - rc.top <= g_ddraw->height)
        {
            if (rc.bottom > mainrc.bottom && abs(mainrc.bottom - rc.bottom) > abs(adj_y))
            {
                adj_y = mainrc.bottom - rc.bottom;
            }
            else if (rc.top < mainrc.top && abs(mainrc.top - rc.top) > abs(adj_y))
            {
                adj_y = mainrc.top - rc.top;
            }
        }

        if (rc.right - rc.left <= g_ddraw->width)
        {
            if (rc.right > mainrc.right && abs(mainrc.right - rc.right) > abs(adj_x))
            {
                adj_x = mainrc.right - rc.right;
            }
            else if (rc.left < mainrc.left && abs(mainrc.left - rc.left) > abs(adj_x))
            {
                adj_x = mainrc.left - rc.left;
            }
        }

        hwnd = FindWindowEx(HWND_DESKTOP, hwnd, "SDlgDialog", NULL);
    }
    
    if (adj_x || adj_y)
    {
        HWND hwnd = FindWindowEx(HWND_DESKTOP, NULL, "SDlgDialog", NULL);

        while (hwnd != NULL)
        {
            RECT rc;
            real_GetWindowRect(hwnd, &rc);

            OffsetRect(&rc, adj_x, adj_y);

            real_SetWindowPos(
                hwnd,
                0,
                rc.left,
                rc.top,
                0,
                0,
                SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

            hwnd = FindWindowEx(HWND_DESKTOP, hwnd, "SDlgDialog", NULL);
        }
    }

    old_x = new_x;
    old_y = new_y;
}

BOOL util_get_lowest_resolution(float ratio, SIZE *out_res, DWORD min_width, DWORD min_height, DWORD max_width, DWORD max_height)
{
    BOOL result = FALSE;
    int org_ratio = (int)((ratio + 0.005f) * 100);
    SIZE lowest = { .cx = max_width + 1, .cy = max_height + 1 };
    DWORD i = 0;
    DEVMODE m;
    memset(&m, 0, sizeof(DEVMODE));
    m.dmSize = sizeof(DEVMODE);

    while (EnumDisplaySettings(NULL, i, &m))
    {
        if  (m.dmPelsWidth >= min_width &&
            m.dmPelsHeight >= min_height &&
            m.dmPelsWidth <= max_width &&
            m.dmPelsHeight <= max_height &&
            m.dmPelsWidth < lowest.cx &&
            m.dmPelsHeight < lowest.cy)
        {
            int res_ratio = (int)((((float)m.dmPelsWidth / m.dmPelsHeight) + 0.005f) * 100);

            if (res_ratio == org_ratio)
            {
                result = TRUE;
                out_res->cx = lowest.cx = m.dmPelsWidth;
                out_res->cy = lowest.cy = m.dmPelsHeight;
            }
        }

        memset(&m, 0, sizeof(DEVMODE));
        m.dmSize = sizeof(DEVMODE);
        i++;
    }

    return result;
}

void util_toggle_maximize()
{
    RECT work_rc;
    RECT client_rc;

    if (real_GetClientRect(g_ddraw->hwnd, &client_rc) &&
        SystemParametersInfo(SPI_GETWORKAREA, 0, &work_rc, 0))
    {
        if (client_rc.right != g_ddraw->width || client_rc.bottom != g_ddraw->height)
        {
            util_set_window_rect(
                (work_rc.right / 2) - (g_ddraw->width / 2),
                (work_rc.bottom / 2) - (g_ddraw->height / 2),
                g_ddraw->width,
                g_ddraw->height,
                0);
        }
        else if (
            util_unadjust_window_rect(
                &work_rc,
                GetWindowLong(g_ddraw->hwnd, GWL_STYLE),
                FALSE,
                GetWindowLong(g_ddraw->hwnd, GWL_EXSTYLE)))
        {
            util_set_window_rect(
                work_rc.left,
                work_rc.top,
                work_rc.right - work_rc.left,
                work_rc.bottom - work_rc.top,
                0);
        }
    }
}

void util_toggle_fullscreen()
{
    if (g_ddraw->bnet_active)
        return;

    if (g_ddraw->windowed)
    {
        mouse_unlock();

        g_config.window_state = g_ddraw->windowed = FALSE;
        real_SetWindowLongA(g_ddraw->hwnd, GWL_STYLE, GetWindowLong(g_ddraw->hwnd, GWL_STYLE) & ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZE | WS_MAXIMIZE | WS_SYSMENU));
        g_ddraw->altenter = TRUE;
        dd_SetDisplayMode(g_ddraw->width, g_ddraw->height, g_ddraw->bpp);
        util_update_bnet_pos(0, 0);

        mouse_lock();
    }
    else
    {
        mouse_unlock();
        g_config.window_state = g_ddraw->windowed = TRUE;

        if (g_ddraw->renderer == d3d9_render_main)
        {
            d3d9_reset();
        }
        else
        {
            ChangeDisplaySettings(NULL, g_ddraw->bnet_active ? CDS_FULLSCREEN : 0);
        }

        dd_SetDisplayMode(g_ddraw->width, g_ddraw->height, g_ddraw->bpp);
        mouse_lock();
    }
}

BOOL util_unadjust_window_rect(LPRECT prc, DWORD dwStyle, BOOL fMenu, DWORD dwExStyle)
{
    RECT rc;
    SetRectEmpty(&rc);

    BOOL fRc = AdjustWindowRectEx(&rc, dwStyle, fMenu, dwExStyle);

    if (fRc)
    {
        prc->left -= rc.left;
        prc->top -= rc.top;
        prc->right -= rc.right;
        prc->bottom -= rc.bottom;
    }

    return fRc;
}

void util_set_window_rect(int x, int y, int width, int height, UINT flags)
{
    if (g_ddraw->windowed)
    {
        if (g_ddraw->render.thread)
        {
            EnterCriticalSection(&g_ddraw->cs);
            g_ddraw->render.run = FALSE;
            ReleaseSemaphore(g_ddraw->render.sem, 1, NULL);
            LeaveCriticalSection(&g_ddraw->cs);

            WaitForSingleObject(g_ddraw->render.thread, INFINITE);
            g_ddraw->render.thread = NULL;
        }

        if ((flags & SWP_NOMOVE) == 0)
        {
            g_config.window_rect.left = x;
            g_config.window_rect.top = y;
        }

        if ((flags & SWP_NOSIZE) == 0)
        {
            g_config.window_rect.bottom = height;
            g_config.window_rect.right = width;
        }

        dd_SetDisplayMode(g_ddraw->width, g_ddraw->height, g_ddraw->bpp);
    }
}

BOOL CALLBACK util_enum_child_proc(HWND hwnd, LPARAM lparam)
{
    IDirectDrawSurfaceImpl* this = (IDirectDrawSurfaceImpl*)lparam;

    RECT size;
    RECT pos;

    if (real_GetClientRect(hwnd, &size) && real_GetWindowRect(hwnd, &pos) && size.right > 1 && size.bottom > 1)
    {
        g_ddraw->child_window_exists = TRUE;

        if (g_ddraw->fixchildwindows)
        {
            HDC hdc = GetDC(hwnd);

            MapWindowPoints(HWND_DESKTOP, g_ddraw->hwnd, (LPPOINT)&pos, 2);

            BitBlt(hdc, 0, 0, size.right, size.bottom, this->hdc, pos.left, pos.top, SRCCOPY);

            ReleaseDC(hwnd, hdc);
        }
    }

    return FALSE;
}

static unsigned char util_get_pixel(int x, int y)
{
    return ((unsigned char*)g_ddraw->primary->surface)[y * g_ddraw->primary->l_pitch + x * g_ddraw->primary->lx_pitch];
}

BOOL util_detect_cutscene()
{
    static int* in_movie = (int*)0x00665F58;
    static int* is_vqa_640 = (int*)0x0065D7BC;
    static BYTE* should_stretch = (BYTE*)0x00607D78;

    if (g_ddraw->width <= CUTSCENE_WIDTH || g_ddraw->height <= CUTSCENE_HEIGHT)
    {
        return FALSE;
    }

    if (g_ddraw->isredalert)
    {
        if ((*in_movie && !*is_vqa_640) || *should_stretch)
        {
            return TRUE;
        }

        return FALSE;
    }
    else if (g_ddraw->iscnc1)
    {
        return util_get_pixel(CUTSCENE_WIDTH + 1, 0) == 0 || util_get_pixel(CUTSCENE_WIDTH + 5, 1) == 0 ? TRUE : FALSE;
    }

    return FALSE;
}
