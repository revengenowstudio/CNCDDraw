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

    if (g_hook_active && !g_ddraw->locked && !IsIconic(g_ddraw->hwnd))
    {
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

        RECT rc = { 0 };
        real_GetClientRect(g_ddraw->hwnd, &rc);
        real_MapWindowPoints(g_ddraw->hwnd, HWND_DESKTOP, (LPPOINT)&rc, 2);
        OffsetRect(&rc, g_ddraw->render.viewport.x, g_ddraw->render.viewport.y);

        int cur_x = InterlockedExchangeAdd((LONG*)&g_ddraw->cursor.x, 0);
        int cur_y = InterlockedExchangeAdd((LONG*)&g_ddraw->cursor.y, 0);

        real_SetCursorPos(
            g_ddraw->adjmouse ? (int)(rc.left + (cur_x * g_ddraw->render.scale_w)) : rc.left + cur_x,
            g_ddraw->adjmouse ? (int)(rc.top + (cur_y * g_ddraw->render.scale_h)) : rc.top + cur_y);

        CopyRect(&rc, &g_ddraw->mouse.rc);
        real_MapWindowPoints(g_ddraw->hwnd, HWND_DESKTOP, (LPPOINT)&rc, 2);
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

        real_ClipCursor(NULL);

        RECT rc = { 0 };
        real_GetClientRect(g_ddraw->hwnd, &rc);
        real_MapWindowPoints(g_ddraw->hwnd, HWND_DESKTOP, (LPPOINT)&rc, 2);
        OffsetRect(&rc, g_ddraw->render.viewport.x, g_ddraw->render.viewport.y);

        int cur_x = InterlockedExchangeAdd((LONG*)&g_ddraw->cursor.x, 0);
        int cur_y = InterlockedExchangeAdd((LONG*)&g_ddraw->cursor.y, 0);

        real_SetCursorPos(
            (int)(rc.left + (cur_x * g_ddraw->render.scale_w)),
            (int)(rc.top + (cur_y * g_ddraw->render.scale_h)));

        while (real_ShowCursor(TRUE) < 0);
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
