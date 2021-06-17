#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "dd.h"
#include "hook.h"


void mouse_lock()
{
    RECT rc;

    if (g_ddraw->bnet_active)
        return;

    if (g_ddraw->devmode)
        return;

    if (g_hook_active && !g_ddraw->locked)
    {
        if (g_ddraw->adjmouse)
        {
            rc.top = g_ddraw->render.viewport.y;
            rc.left = g_ddraw->render.viewport.x;
            rc.right = g_ddraw->render.viewport.width + g_ddraw->render.viewport.x;
            rc.bottom = g_ddraw->render.viewport.height + g_ddraw->render.viewport.y;
        }
        else
        {
            rc.top = g_ddraw->render.viewport.y;
            rc.left = g_ddraw->render.viewport.x;
            rc.right = g_ddraw->width + g_ddraw->render.viewport.x;
            rc.bottom = g_ddraw->height + g_ddraw->render.viewport.y;
        }

        /* Convert the client area to screen coordinates  */
        POINT pt = { rc.left, rc.top };
        POINT pt2 = { rc.right, rc.bottom };

        real_ClientToScreen(g_ddraw->hwnd, &pt);
        real_ClientToScreen(g_ddraw->hwnd, &pt2);

        SetRect(&rc, pt.x, pt.y, pt2.x, pt2.y);

        rc.bottom -= (LONG)((g_ddraw->mouse_y_adjust * 2) * g_ddraw->render.scale_h);

        if (g_ddraw->adjmouse)
        {
            real_SetCursorPos(
                (int)(rc.left + (g_ddraw->cursor.x * g_ddraw->render.scale_w)),
                (int)(rc.top + ((g_ddraw->cursor.y - g_ddraw->mouse_y_adjust) * g_ddraw->render.scale_h)));
        }
        else
        {
            real_SetCursorPos(rc.left + g_ddraw->cursor.x, rc.top + g_ddraw->cursor.y - g_ddraw->mouse_y_adjust);
        }

        real_SetCursor(g_ddraw->old_cursor);

        int cur_count = real_ShowCursor(TRUE) - 1;
        real_ShowCursor(FALSE);

        int game_count = InterlockedExchangeAdd(&g_ddraw->show_cursor_count, 0);

        if (cur_count > game_count)
        {
            while (real_ShowCursor(FALSE) > game_count);
        }

        if (cur_count < game_count)
        {
            while (real_ShowCursor(TRUE) < game_count);
        }

        real_ClipCursor(&rc);

        g_ddraw->locked = TRUE;
    }
}

void mouse_unlock()
{
    RECT rc;

    if (g_ddraw->devmode)
        return;

    if (!g_hook_active)
        return;

    if (g_ddraw->locked)
    {
        g_ddraw->locked = FALSE;

        /* Get the window client area  */
        real_GetClientRect(g_ddraw->hwnd, &rc);

        /* Convert the client area to screen coordinates */
        POINT pt = { rc.left, rc.top };
        POINT pt2 = { rc.right, rc.bottom };

        real_ClientToScreen(g_ddraw->hwnd, &pt);
        real_ClientToScreen(g_ddraw->hwnd, &pt2);

        SetRect(&rc, pt.x, pt.y, pt2.x, pt2.y);

        CURSORINFO ci = { .cbSize = sizeof(CURSORINFO) };
        if (real_GetCursorInfo(&ci) && ci.flags == 0)
        {
            while (real_ShowCursor(TRUE) < 0);
        }

        real_SetCursor(LoadCursor(NULL, IDC_ARROW));

        real_ClipCursor(NULL);

        real_SetCursorPos(
            (int)(rc.left + g_ddraw->render.viewport.x + (g_ddraw->cursor.x * g_ddraw->render.scale_w)),
            (int)(rc.top + g_ddraw->render.viewport.y + ((g_ddraw->cursor.y + g_ddraw->mouse_y_adjust) * g_ddraw->render.scale_h)));
    }
}
