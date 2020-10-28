#include <windows.h>
#include <windowsx.h>
#include "dllmain.h"
#include "dd.h"
#include "hook.h"
#include "mouse.h"
#include "render_d3d9.h"
#include "config.h"
#include "screenshot.h"
#include "winapi_hooks.h"
#include "wndproc.h"
#include "utils.h"


LRESULT CALLBACK fake_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    RECT rc = { 0, 0, g_ddraw->render.width, g_ddraw->render.height };

    static BOOL in_size_move = FALSE;
    static int redraw_count = 0;
    
    switch(uMsg)
    {
        case WM_GETMINMAXINFO:
        case WM_MOVING:
        case WM_NCLBUTTONDOWN:
        case WM_NCLBUTTONUP:
        case WM_NCPAINT:
        {
            return DefWindowProc(hWnd, uMsg, wParam, lParam);
        }
        case WM_NCACTIVATE:
        {
            if (g_ddraw->noactivateapp)
            {
                return DefWindowProc(hWnd, uMsg, wParam, lParam);
            }

            break;
        }
        case WM_NCHITTEST:
        {
            LRESULT result = DefWindowProc(hWnd, uMsg, wParam, lParam);

            if (!g_ddraw->resizable)
            {
                switch (result)
                {
                    case HTBOTTOM:
                    case HTBOTTOMLEFT:
                    case HTBOTTOMRIGHT:
                    case HTLEFT:
                    case HTRIGHT:
                    case HTTOP:
                    case HTTOPLEFT:
                    case HTTOPRIGHT:
                        return HTBORDER;
                }
            }

            return result;
        }
        case WM_SETCURSOR:
        {
            // show resize cursor on window borders
            if ((HWND)wParam == g_ddraw->hwnd)
            {
                WORD message = HIWORD(lParam);

                if (message == WM_MOUSEMOVE)
                {
                    WORD htcode = LOWORD(lParam);

                    switch (htcode)
                    {
                        case HTCAPTION:
                        case HTMINBUTTON:
                        case HTMAXBUTTON:
                        case HTCLOSE:
                        case HTBOTTOM:
                        case HTBOTTOMLEFT:
                        case HTBOTTOMRIGHT:
                        case HTLEFT:
                        case HTRIGHT:
                        case HTTOP:
                        case HTTOPLEFT:
                        case HTTOPRIGHT:
                            return DefWindowProc(hWnd, uMsg, wParam, lParam);
                        case HTCLIENT:
                            if (!g_ddraw->locked)
                                return DefWindowProc(hWnd, uMsg, wParam, lParam);
                        default:
                            break;
                    }
                }
            }

            break;
        }
        case WM_D3D9DEVICELOST:
        {
            if (g_ddraw->renderer == d3d9_render_main && d3d9_on_device_lost())
            {
                if (!g_ddraw->windowed)
                    mouse_lock();
            }
            return 0;
        }
        case WM_TIMER:
        {
            switch (wParam)
            {
                case IDT_TIMER_LEAVE_BNET:
                {
                    KillTimer(g_ddraw->hwnd, IDT_TIMER_LEAVE_BNET);

                    if (!g_ddraw->windowed)
                        g_ddraw->bnet_was_fullscreen = FALSE;

                    if (!g_ddraw->bnet_active)
                    {
                        if (g_ddraw->bnet_was_fullscreen)
                        {
                            int ws = g_config.window_state;
                            util_toggle_fullscreen();
                            g_config.window_state = ws;
                            g_ddraw->bnet_was_fullscreen = FALSE;
                        }
                        else if (g_ddraw->bnet_was_upscaled)
                        {
                            util_set_window_rect(0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
                            g_ddraw->bnet_was_upscaled = FALSE;
                        }
                    }

                    return 0;
                }   
            }
            break;
        }
        case WM_WINDOWPOSCHANGED:
        {
            WINDOWPOS *pos = (WINDOWPOS *)lParam;

            if (g_ddraw->wine && !g_ddraw->windowed && (pos->x > 0 || pos->y > 0) && g_ddraw->last_set_window_pos_tick + 500 < timeGetTime())
            {
                PostMessage(g_ddraw->hwnd, WM_WINEFULLSCREEN, 0, 0);
            }

            break;
        }
        case WM_WINEFULLSCREEN:
        {
            if (!g_ddraw->windowed)
            {
                g_ddraw->last_set_window_pos_tick = timeGetTime();
                real_SetWindowPos(g_ddraw->hwnd, HWND_TOPMOST, 1, 1, g_ddraw->render.width, g_ddraw->render.height, SWP_SHOWWINDOW);
                real_SetWindowPos(g_ddraw->hwnd, HWND_TOPMOST, 0, 0, g_ddraw->render.width, g_ddraw->render.height, SWP_SHOWWINDOW);
            }
            return 0;
        }
        case WM_ENTERSIZEMOVE:
        {
            if (g_ddraw->windowed)
            {
                in_size_move = TRUE;
            }
            break;
        }
        case WM_EXITSIZEMOVE:
        {
            if (g_ddraw->windowed)
            {
                in_size_move = FALSE;

                if (!g_ddraw->render.thread)
                    dd_SetDisplayMode(g_ddraw->width, g_ddraw->height, g_ddraw->bpp);
            }
            break;
        }
        case WM_SIZING:
        {
            RECT *windowrc = (RECT *)lParam;

            if (g_ddraw->windowed)
            {
                if (in_size_move)
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

                    RECT clientrc = { 0 };

                    // maintain aspect ratio
                    if (g_ddraw->maintas && 
                        CopyRect(&clientrc, windowrc) &&
                        util_unadjust_window_rect(&clientrc, GetWindowLong(hWnd, GWL_STYLE), FALSE, GetWindowLong(hWnd, GWL_EXSTYLE)) &&
                        SetRect(&clientrc, 0, 0, clientrc.right - clientrc.left, clientrc.bottom - clientrc.top))
                    {
                        float scaleH = (float)g_ddraw->height / g_ddraw->width;
                        float scaleW = (float)g_ddraw->width / g_ddraw->height;

                        switch (wParam)
                        {
                            case WMSZ_BOTTOMLEFT:
                            case WMSZ_BOTTOMRIGHT:
                            case WMSZ_LEFT:
                            case WMSZ_RIGHT:
                            {
                                windowrc->bottom += scaleH * clientrc.right - clientrc.bottom;
                                break;
                            }
                            case WMSZ_TOP:
                            case WMSZ_BOTTOM:
                            {
                                windowrc->right += scaleW * clientrc.bottom - clientrc.right;
                                break;
                            }
                            case WMSZ_TOPRIGHT:
                            case WMSZ_TOPLEFT:
                            {
                                windowrc->top -= scaleH * clientrc.right - clientrc.bottom;
                                break;
                            }
                        }
                    }

                    //enforce minimum window size
                    if (CopyRect(&clientrc, windowrc) &&
                        util_unadjust_window_rect(&clientrc, GetWindowLong(hWnd, GWL_STYLE), FALSE, GetWindowLong(hWnd, GWL_EXSTYLE)) &&
                        SetRect(&clientrc, 0, 0, clientrc.right - clientrc.left, clientrc.bottom - clientrc.top))
                    {
                        if (clientrc.right < g_ddraw->width)
                        {
                            switch (wParam)
                            {
                                case WMSZ_TOPRIGHT:
                                case WMSZ_BOTTOMRIGHT:
                                case WMSZ_RIGHT:
                                case WMSZ_BOTTOM:
                                case WMSZ_TOP:
                                {
                                    windowrc->right += g_ddraw->width - clientrc.right; 
                                    break;
                                }
                                case WMSZ_TOPLEFT:
                                case WMSZ_BOTTOMLEFT:
                                case WMSZ_LEFT:
                                {
                                    windowrc->left -= g_ddraw->width - clientrc.right; 
                                    break;
                                }
                            }
                        }
                            
                        if (clientrc.bottom < g_ddraw->height)
                        {
                            switch (wParam)
                            {
                                case WMSZ_BOTTOMLEFT:
                                case WMSZ_BOTTOMRIGHT:
                                case WMSZ_BOTTOM:
                                case WMSZ_RIGHT:
                                case WMSZ_LEFT:
                                {
                                    windowrc->bottom += g_ddraw->height - clientrc.bottom; 
                                    break;
                                }
                                case WMSZ_TOPLEFT:
                                case WMSZ_TOPRIGHT:
                                case WMSZ_TOP:
                                {
                                    windowrc->top -= g_ddraw->height - clientrc.bottom; 
                                    break;
                                }
                            }
                        }
                    }

                    //save new window position
                    if (CopyRect(&clientrc, windowrc) &&
                        util_unadjust_window_rect(&clientrc, GetWindowLong(hWnd, GWL_STYLE), FALSE, GetWindowLong(hWnd, GWL_EXSTYLE)))
                    {
                        g_config.window_rect.left = clientrc.left;
                        g_config.window_rect.top = clientrc.top;
                        g_config.window_rect.right = clientrc.right - clientrc.left;
                        g_config.window_rect.bottom = clientrc.bottom - clientrc.top;
                    }

                    return TRUE;
                }
            }
            break;
        }
        case WM_SIZE: 
        {
            if (g_ddraw->windowed)
            {
                if (wParam == SIZE_RESTORED)
                {
                    if (in_size_move && !g_ddraw->render.thread)
                    {
                        g_config.window_rect.right = LOWORD(lParam);
                        g_config.window_rect.bottom = HIWORD(lParam);
                    }
                    /*
                    else if (g_ddraw->wine)
                    {
                        WindowRect.right = LOWORD(lParam);
                        WindowRect.bottom = HIWORD(lParam);
                        if (WindowRect.right != g_ddraw->render.width || WindowRect.bottom != g_ddraw->render.height)
                            dd_SetDisplayMode(g_ddraw->width, g_ddraw->height, g_ddraw->bpp);
                    }
                    */
                }
            }

            if (!g_ddraw->handlemouse)
            {
                redraw_count = 2;
                RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN);
            }

            return DefWindowProc(hWnd, uMsg, wParam, lParam); /* Carmageddon fix */
        }
        case WM_MOVE:
        {
            if (g_ddraw->windowed)
            {
                int x = (int)(short)LOWORD(lParam);
                int y = (int)(short)HIWORD(lParam);

                if (x != -32000 && y != -32000)
                {
                    util_update_bnet_pos(x, y);
                }

                if (in_size_move || g_ddraw->wine)
                {
                    if (x != -32000)
                        g_config.window_rect.left = x; // -32000 = Exit/Minimize

                    if (y != -32000)
                        g_config.window_rect.top = y;
                }
            }
            
            if (!g_ddraw->handlemouse)
                RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN);

            if (g_ddraw->sierrahack)
            {
                lParam = 0;
                break;
            }

            return DefWindowProc(hWnd, uMsg, wParam, lParam); /* Carmageddon fix */
        }

        /* C&C and RA really don't want to close down */
        case WM_SYSCOMMAND:
            if (wParam == SC_CLOSE && !GameHandlesClose)
            {
                exit(0);
            }

            if (wParam == SC_KEYMENU)
                return 0;

            if (!GameHandlesClose)
                return DefWindowProc(hWnd, uMsg, wParam, lParam);

            break;

        //workaround for a bug where sometimes a background window steals the focus
        case WM_WINDOWPOSCHANGING:
        {
            if (g_ddraw->locked)
            {
                WINDOWPOS *pos = (WINDOWPOS *)lParam;
                
                if (pos->flags == SWP_NOMOVE + SWP_NOSIZE)
                {
                    mouse_unlock();
                    
                    if (GetForegroundWindow() == g_ddraw->hwnd)
                        mouse_lock();
                }
            }
            break;
        }

        case WM_MOUSELEAVE:
            mouse_unlock();
            return 0;

        case WM_ACTIVATE:
            if (wParam == WA_ACTIVE || wParam == WA_CLICKACTIVE)
            {
                if (!g_ddraw->windowed)
                {
                    if (g_ddraw->renderer != d3d9_render_main)
                    {
                        ChangeDisplaySettings(&g_ddraw->render.mode, CDS_FULLSCREEN);

                        if (wParam == WA_ACTIVE)
                        {
                            mouse_lock();
                        }
                    }
                }

                if (!g_ddraw->handlemouse)
                    RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN);
            }
            else if (wParam == WA_INACTIVE)
            {
                if (!g_ddraw->windowed && !g_ddraw->locked && g_ddraw->noactivateapp && !g_ddraw->devmode)
                    return 0;

                mouse_unlock();

                if (g_ddraw->wine && g_ddraw->last_set_window_pos_tick + 500 > timeGetTime())
                    return 0;

                /* minimize our window on defocus when in fullscreen */
                if (!g_ddraw->windowed)
                {
                    if (g_ddraw->renderer != d3d9_render_main)
                    {
                        ShowWindow(g_ddraw->hwnd, SW_MINIMIZE);
                        ChangeDisplaySettings(NULL, g_ddraw->bnet_active ? CDS_FULLSCREEN : 0);
                    }
                }
            }
            return 0;

        case WM_ACTIVATEAPP:
            /* C&C and RA stop drawing when they receive this with FALSE wParam, disable in windowed mode */
            if (g_ddraw->windowed || g_ddraw->noactivateapp)
            {
                // let it pass through once (tiberian sun)
                static BOOL one_time;

                if (wParam && !one_time && !g_ddraw->handlemouse && g_ddraw->noactivateapp)
                {
                    one_time = TRUE;
                    break;
                }

                return 0;
            }
            break;
        case WM_AUTORENDERER:
        {
            mouse_unlock();
            real_SetWindowPos(g_ddraw->hwnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
            real_SetWindowPos(g_ddraw->hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
            mouse_lock();
            return 0;
        }
        case WM_NCLBUTTONDBLCLK:
        {
            util_toggle_fullscreen();
            return 0;
        }
        case WM_SYSKEYDOWN:
        {
            BOOL context_code = (lParam & (1 << 29)) != 0;
            BOOL key_state    = (lParam & (1 << 30)) != 0;

            if (wParam == VK_RETURN && context_code && !key_state)
            {
                util_toggle_fullscreen();
                return 0;
            }

            break;
        }
        case WM_KEYDOWN:
        {
            if (wParam == VK_F11)
            {
                if (g_ddraw->render.thread)
                {
                    EnterCriticalSection(&g_ddraw->cs);
                    g_ddraw->render.run = FALSE;
                    ReleaseSemaphore(g_ddraw->render.sem, 1, NULL);
                    LeaveCriticalSection(&g_ddraw->cs);
                    WaitForSingleObject(g_ddraw->render.thread, INFINITE);

                    InterlockedExchange(&g_ddraw->render.palette_updated, TRUE);
                    InterlockedExchange(&g_ddraw->render.surface_updated, TRUE);
                    ReleaseSemaphore(g_ddraw->render.sem, 1, NULL);
                    g_ddraw->render.run = TRUE;
                    g_ddraw->render.thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)g_ddraw->renderer, NULL, 0, NULL);
                }

                return 0;
            }

            if (wParam == VK_CONTROL || wParam == VK_TAB)
            {
                if (GetAsyncKeyState(VK_CONTROL) & 0x8000 && GetAsyncKeyState(VK_TAB) & 0x8000)
                {
                    mouse_unlock();
                    return 0;
                }
            }

            if (wParam == VK_CONTROL || wParam == VK_MENU)
            {
                if ((GetAsyncKeyState(VK_RMENU) & 0x8000) && GetAsyncKeyState(VK_RCONTROL) & 0x8000)
                {
                    mouse_unlock();
                    return 0;
                }
            }

            break;
        }
        case WM_KEYUP:
        {
            if (wParam == VK_SNAPSHOT)
                ss_take_screenshot(g_ddraw->primary);

            break;
        }
        /* button up messages reactivate cursor lock */
        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
        case WM_MBUTTONUP:
        {
            if (!g_ddraw->devmode && !g_ddraw->locked)
            {
                int x = GET_X_LPARAM(lParam);
                int y = GET_Y_LPARAM(lParam);

                if (x > g_ddraw->render.viewport.x + g_ddraw->render.viewport.width ||
                    x < g_ddraw->render.viewport.x ||
                    y > g_ddraw->render.viewport.y + g_ddraw->render.viewport.height ||
                    y < g_ddraw->render.viewport.y)
                {
                    g_ddraw->cursor.x = g_ddraw->width / 2;
                    g_ddraw->cursor.y = g_ddraw->height / 2;
                }
                else
                {
                    g_ddraw->cursor.x = (x - g_ddraw->render.viewport.x) * g_ddraw->render.unscale_w;
                    g_ddraw->cursor.y = (y - g_ddraw->render.viewport.y) * g_ddraw->render.unscale_h;
                }

                mouse_lock();
                return 0;
            }
            /* fall through for lParam */
        }
        /* down messages are ignored if we have no cursor lock */
        case WM_XBUTTONDBLCLK:
        case WM_XBUTTONDOWN:
        case WM_XBUTTONUP:
        case WM_MOUSEWHEEL:
        case WM_MOUSEHOVER:
        case WM_LBUTTONDBLCLK:
        case WM_MBUTTONDBLCLK:
        case WM_RBUTTONDBLCLK:
        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_MOUSEMOVE:
        {
            if (!g_ddraw->devmode)
            {
                if (!g_ddraw->locked)
                {
                    return 0;
                }

                if (g_ddraw->adjmouse)
                {
                    fake_GetCursorPos(NULL); /* update our own cursor */
                    lParam = MAKELPARAM(g_ddraw->cursor.x, g_ddraw->cursor.y);
                }
            }

            if (g_ddraw->devmode)
            {
                mouse_lock();
                g_ddraw->cursor.x = GET_X_LPARAM(lParam);
                g_ddraw->cursor.y = GET_Y_LPARAM(lParam);
            }
            break;
        }
        case WM_PARENTNOTIFY:
        {
            if (!g_ddraw->handlemouse)
            {
                switch (LOWORD(wParam))
                {
                    case WM_DESTROY: //Workaround for invisible menu on Load/Save/Delete in Tiberian Sun
                        redraw_count = 2;
                        break;
                    case WM_LBUTTONDOWN:
                    case WM_MBUTTONDOWN:
                    case WM_RBUTTONDOWN:
                    case WM_XBUTTONDOWN:
                    {
                        if (!g_ddraw->devmode && !g_ddraw->locked)
                        {
                            int x = GET_X_LPARAM(lParam);
                            int y = GET_Y_LPARAM(lParam);

                            g_ddraw->cursor.x = (x - g_ddraw->render.viewport.x) * g_ddraw->render.unscale_w;
                            g_ddraw->cursor.y = (y - g_ddraw->render.viewport.y) * g_ddraw->render.unscale_h;

                            g_ddraw->hidecursor = FALSE;

                            mouse_lock();
                        }
                        break;
                    }
                }
            }
            break;
        }
        case WM_PAINT:
        {
            if (!g_ddraw->handlemouse && redraw_count > 0)
            {
                redraw_count--;
                RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN);
            }

            EnterCriticalSection(&g_ddraw->cs);
            ReleaseSemaphore(g_ddraw->render.sem, 1, NULL);
            LeaveCriticalSection(&g_ddraw->cs);
            break;
        }
        case WM_ERASEBKGND:
        {
            EnterCriticalSection(&g_ddraw->cs);
            FillRect(g_ddraw->render.hdc, &rc, (HBRUSH)GetStockObject(BLACK_BRUSH));
            ReleaseSemaphore(g_ddraw->render.sem, 1, NULL);
            LeaveCriticalSection(&g_ddraw->cs);
            break;
        }
    }

    return g_ddraw->wndproc(hWnd, uMsg, wParam, lParam);
}
