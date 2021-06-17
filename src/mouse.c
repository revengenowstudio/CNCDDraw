#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "debug.h"
#include "dd.h"
#include "hook.h"


void mouse_lock()
{
    if (g_ddraw->devmode || g_ddraw->bnet_active)
        return;

    if (g_hook_active && !g_ddraw->locked)
    {
        RECT rc = {
            g_ddraw->render.viewport.x,
            g_ddraw->render.viewport.y,
            g_ddraw->width + g_ddraw->render.viewport.x,
            g_ddraw->height + g_ddraw->render.viewport.y,
        };

        if (g_ddraw->adjmouse)
        {
            rc.right = g_ddraw->render.viewport.width + g_ddraw->render.viewport.x;
            rc.bottom = g_ddraw->render.viewport.height + g_ddraw->render.viewport.y;
        }

        real_MapWindowPoints(g_ddraw->hwnd, HWND_DESKTOP, (LPPOINT)&rc, 2);

        rc.bottom -= (LONG)((g_ddraw->mouse_y_adjust * 2) * g_ddraw->render.scale_h);

        int cur_x = InterlockedExchangeAdd((LONG*)&g_ddraw->cursor.x, 0);
        int cur_y = InterlockedExchangeAdd((LONG*)&g_ddraw->cursor.y, 0);

        if (g_ddraw->adjmouse)
        {
            real_SetCursorPos(
                (int)(rc.left + (cur_x * g_ddraw->render.scale_w)),
                (int)(rc.top + ((cur_y - g_ddraw->mouse_y_adjust) * g_ddraw->render.scale_h)));
        }
        else
        {
            real_SetCursorPos(rc.left + cur_x, rc.top + cur_y - g_ddraw->mouse_y_adjust);
        }

        real_SetCursor(g_ddraw->old_cursor);

        int game_count = (int)InterlockedExchangeAdd((LONG*)&g_ddraw->show_cursor_count, 0);
        int cur_count = real_ShowCursor(TRUE) - 1;
        real_ShowCursor(FALSE);

        if (cur_count > game_count)
        {
            while (real_ShowCursor(FALSE) > game_count);
        }
        else if (cur_count < game_count)
        {
            while (real_ShowCursor(TRUE) < game_count);
        }

        real_ClipCursor(&rc);

        g_ddraw->locked = TRUE;
    }
}

void mouse_unlock()
{
    if (g_ddraw->devmode || !g_hook_active)
        return;

    if (g_ddraw->locked)
    {
        g_ddraw->locked = FALSE;

        RECT rc;
        real_GetClientRect(g_ddraw->hwnd, &rc);
        real_MapWindowPoints(g_ddraw->hwnd, HWND_DESKTOP, (LPPOINT)&rc, 2);

        CURSORINFO ci = { .cbSize = sizeof(CURSORINFO) };
        if (real_GetCursorInfo(&ci) && ci.flags == 0)
        {
            while (real_ShowCursor(TRUE) < 0);
        }

        real_SetCursor(LoadCursor(NULL, IDC_ARROW));
        real_ClipCursor(NULL);

        int cur_x = InterlockedExchangeAdd((LONG*)&g_ddraw->cursor.x, 0);
        int cur_y = InterlockedExchangeAdd((LONG*)&g_ddraw->cursor.y, 0);

        real_SetCursorPos(
            (int)(rc.left + g_ddraw->render.viewport.x + (cur_x * g_ddraw->render.scale_w)),
            (int)(rc.top + g_ddraw->render.viewport.y + ((cur_y + g_ddraw->mouse_y_adjust) * g_ddraw->render.scale_h)));
    }
}
