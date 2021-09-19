#include <windows.h>
#include "debug.h"
#include "winapi_hooks.h"
#include "dd.h"
#include "hook.h"


HHOOK g_mouse_hook;
HOOKPROC g_mouse_proc;

void mouse_lock()
{
    if (g_ddraw->devmode || g_ddraw->bnet_active)
        return;

    if (g_hook_active && !g_ddraw->locked)
    {
        RECT rc = { 0 };
        CopyRect(&rc, &g_ddraw->mouse.rc);

        real_MapWindowPoints(g_ddraw->hwnd, HWND_DESKTOP, (LPPOINT)&rc, 2);

        int cur_x = InterlockedExchangeAdd((LONG*)&g_ddraw->cursor.x, 0);
        int cur_y = InterlockedExchangeAdd((LONG*)&g_ddraw->cursor.y, 0);

        if (g_ddraw->adjmouse)
        {
            real_SetCursorPos(
                (int)(rc.left + (cur_x * g_ddraw->render.scale_w)),
                (int)(rc.top + (cur_y * g_ddraw->render.scale_h)));
        }
        else
        {
            real_SetCursorPos(rc.left + cur_x, rc.top + cur_y);
        }

        int game_count = InterlockedExchangeAdd((LONG*)&g_ddraw->show_cursor_count, 0);
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

        real_SetCursor((HCURSOR)InterlockedExchangeAdd((LONG*)&g_ddraw->old_cursor, 0));

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

        while (real_ShowCursor(TRUE) < 0);

        real_ClipCursor(NULL);

        int cur_x = InterlockedExchangeAdd((LONG*)&g_ddraw->cursor.x, 0);
        int cur_y = InterlockedExchangeAdd((LONG*)&g_ddraw->cursor.y, 0);

        real_SetCursorPos(
            (int)(rc.left + g_ddraw->render.viewport.x + (cur_x * g_ddraw->render.scale_w)),
            (int)(rc.top + g_ddraw->render.viewport.y + (cur_y * g_ddraw->render.scale_h)));
    }
}

LRESULT CALLBACK mouse_hook_proc(int Code, WPARAM wParam, LPARAM lParam)
{
    if (!g_ddraw)
        return g_mouse_proc(Code, wParam, lParam);

    if (Code < 0 || (!g_ddraw->devmode && !g_ddraw->locked))
        return CallNextHookEx(g_mouse_hook, Code, wParam, lParam);

    fake_GetCursorPos(&((MOUSEHOOKSTRUCT*)lParam)->pt);

    return g_mouse_proc(Code, wParam, lParam);
}
