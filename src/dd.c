#include <windows.h>
#include "IDirectDraw.h"
#include "ddraw.h"
#include "dd.h"
#include "hook.h"
#include "config.h"
#include "mouse.h"
#include "wndproc.h"
#include "render_d3d9.h"
#include "render_gdi.h"
#include "render_ogl.h"
#include "fps_limiter.h"
#include "debug.h"
#include "utils.h"


cnc_ddraw *g_ddraw = NULL;

HRESULT dd_EnumDisplayModes(DWORD dwFlags, LPDDSURFACEDESC lpDDSurfaceDesc, LPVOID lpContext, LPDDENUMMODESCALLBACK lpEnumModesCallback)
{
    DWORD i = 0;
    DDSURFACEDESC s;

    // Some games crash when you feed them with too many resolutions...

    if (g_ddraw->bpp)
    {
        dprintf("     g_ddraw->bpp=%u\n", g_ddraw->bpp);

        //set up some filters to keep the list short
        DWORD refresh_rate = 0;
        DWORD bpp = 0;
        DWORD flags = 99998;
        DWORD fixed_output = 99998;
        DEVMODE m;

        memset(&m, 0, sizeof(DEVMODE));
        m.dmSize = sizeof(DEVMODE);

        while (EnumDisplaySettings(NULL, i, &m))
        {
            if (refresh_rate != 60 && m.dmDisplayFrequency >= 50)
                refresh_rate = m.dmDisplayFrequency;

            if (bpp != 32 && m.dmBitsPerPel >= 16)
                bpp = m.dmBitsPerPel;

            if (flags != 0)
                flags = m.dmDisplayFlags;

            if (fixed_output != DMDFO_DEFAULT)
                fixed_output = m.dmDisplayFixedOutput;

            memset(&m, 0, sizeof(DEVMODE));
            m.dmSize = sizeof(DEVMODE);
            i++;
        }

        memset(&m, 0, sizeof(DEVMODE));
        m.dmSize = sizeof(DEVMODE);
        i = 0;

        while (EnumDisplaySettings(NULL, i, &m))
        {
            if (refresh_rate == m.dmDisplayFrequency &&
                bpp == m.dmBitsPerPel &&
                flags == m.dmDisplayFlags &&
                fixed_output == m.dmDisplayFixedOutput)
            {
                dprintfex("     %d: %dx%d@%d %d bpp\n", (int)i, (int)m.dmPelsWidth, (int)m.dmPelsHeight, (int)m.dmDisplayFrequency, (int)m.dmBitsPerPel);

                memset(&s, 0, sizeof(DDSURFACEDESC));

                s.dwSize = sizeof(DDSURFACEDESC);
                s.dwFlags = DDSD_HEIGHT | DDSD_REFRESHRATE | DDSD_WIDTH | DDSD_PITCH | DDSD_PIXELFORMAT;
                s.dwHeight = m.dmPelsHeight;
                s.dwWidth = m.dmPelsWidth;
                s.lPitch = s.dwWidth;
                s.dwRefreshRate = 60;
                s.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);

                s.ddpfPixelFormat.dwFlags = DDPF_PALETTEINDEXED8 | DDPF_RGB;
                s.ddpfPixelFormat.dwRGBBitCount = 8;
                
                if (g_ddraw->bpp == 16)
                {
                    s.lPitch = s.dwWidth * 2;
                    s.ddpfPixelFormat.dwFlags = DDPF_RGB;
                    s.ddpfPixelFormat.dwRGBBitCount = 16;
                    s.ddpfPixelFormat.dwRBitMask = 0xF800;
                    s.ddpfPixelFormat.dwGBitMask = 0x07E0;
                    s.ddpfPixelFormat.dwBBitMask = 0x001F;
                }

                if (g_ddraw->bpp == 32)
                {
                    s.lPitch = s.dwWidth * 4;
                    s.ddpfPixelFormat.dwFlags = DDPF_RGB;
                    s.ddpfPixelFormat.dwRGBBitCount = 32;
                    s.ddpfPixelFormat.dwRBitMask = 0xFF0000;
                    s.ddpfPixelFormat.dwGBitMask = 0x00FF00;
                    s.ddpfPixelFormat.dwBBitMask = 0x0000FF;
                }

                if (lpEnumModesCallback(&s, lpContext) == DDENUMRET_CANCEL)
                {
                    dprintf("     DDENUMRET_CANCEL returned, stopping\n");
                    break;
                }
            }

            memset(&m, 0, sizeof(DEVMODE));
            m.dmSize = sizeof(DEVMODE);
            i++;
        }
    }
    else
    {
        SIZE resolutions[] =
        {
            { 320, 200 },
            { 320, 240 },
            { 512, 384 },
            { 640, 400 },
            { 640, 480 },
            { 800, 600 },
            { 1024, 768 },
            { 1280, 1024 },
            { 1600, 1200 },
            { 1280, 720 },
            { 1920, 1080 },
        };

        for (i = 0; i < sizeof(resolutions) / sizeof(resolutions[0]); i++)
        {
            memset(&s, 0, sizeof(DDSURFACEDESC));

            s.dwSize = sizeof(DDSURFACEDESC);
            s.dwFlags = DDSD_HEIGHT | DDSD_REFRESHRATE | DDSD_WIDTH | DDSD_PITCH | DDSD_PIXELFORMAT;
            s.dwHeight = resolutions[i].cy;
            s.dwWidth = resolutions[i].cx;
            s.lPitch = s.dwWidth;
            s.dwRefreshRate = 60;
            s.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
            s.ddpfPixelFormat.dwFlags = DDPF_PALETTEINDEXED8 | DDPF_RGB;
            s.ddpfPixelFormat.dwRGBBitCount = 8;

            if (lpEnumModesCallback(&s, lpContext) == DDENUMRET_CANCEL)
            {
                dprintf("     DDENUMRET_CANCEL returned, stopping\n");
                break;
            }

            s.lPitch = s.dwWidth * 2;
            s.ddpfPixelFormat.dwFlags = DDPF_RGB;
            s.ddpfPixelFormat.dwRGBBitCount = 16;
            s.ddpfPixelFormat.dwRBitMask = 0xF800;
            s.ddpfPixelFormat.dwGBitMask = 0x07E0;
            s.ddpfPixelFormat.dwBBitMask = 0x001F;

            if (lpEnumModesCallback(&s, lpContext) == DDENUMRET_CANCEL)
            {
                dprintf("     DDENUMRET_CANCEL returned, stopping\n");
                break;
            }

            s.lPitch = s.dwWidth * 4;
            s.ddpfPixelFormat.dwFlags = DDPF_RGB;
            s.ddpfPixelFormat.dwRGBBitCount = 32;
            s.ddpfPixelFormat.dwRBitMask = 0xFF0000;
            s.ddpfPixelFormat.dwGBitMask = 0x00FF00;
            s.ddpfPixelFormat.dwBBitMask = 0x0000FF;

            if (lpEnumModesCallback(&s, lpContext) == DDENUMRET_CANCEL)
            {
                dprintf("     DDENUMRET_CANCEL returned, stopping\n");
                break;
            }
        }
    }

    return DD_OK;
}

HRESULT dd_GetCaps(LPDDCAPS lpDDDriverCaps, LPDDCAPS lpDDEmulCaps)
{
    if (lpDDDriverCaps)
    {
        lpDDDriverCaps->dwSize = sizeof(DDCAPS);
        lpDDDriverCaps->dwCaps = DDCAPS_BLT | DDCAPS_PALETTE | DDCAPS_BLTCOLORFILL | DDCAPS_BLTSTRETCH | DDCAPS_CANCLIP | DDCAPS_CANBLTSYSMEM;
        lpDDDriverCaps->dwCKeyCaps = 0;
        lpDDDriverCaps->dwPalCaps = DDPCAPS_8BIT | DDPCAPS_PRIMARYSURFACE;
        lpDDDriverCaps->dwVidMemTotal = 16777216;
        lpDDDriverCaps->dwVidMemFree = 16777216;
        lpDDDriverCaps->dwMaxVisibleOverlays = 0;
        lpDDDriverCaps->dwCurrVisibleOverlays = 0;
        lpDDDriverCaps->dwNumFourCCCodes = 0;
        lpDDDriverCaps->dwAlignBoundarySrc = 0;
        lpDDDriverCaps->dwAlignSizeSrc = 0;
        lpDDDriverCaps->dwAlignBoundaryDest = 0;
        lpDDDriverCaps->dwAlignSizeDest = 0;
        lpDDDriverCaps->ddsCaps.dwCaps = DDSCAPS_FLIP;
    }

    if (lpDDEmulCaps)
    {
        lpDDEmulCaps->dwSize = 0;
    }

    return DD_OK;
}

HRESULT dd_GetDisplayMode(LPDDSURFACEDESC lpDDSurfaceDesc)
{
    if (lpDDSurfaceDesc)
    {
        memset(lpDDSurfaceDesc, 0, sizeof(DDSURFACEDESC));

        lpDDSurfaceDesc->dwSize = sizeof(DDSURFACEDESC);
        lpDDSurfaceDesc->dwFlags = DDSD_HEIGHT | DDSD_REFRESHRATE | DDSD_WIDTH | DDSD_PITCH | DDSD_PIXELFORMAT;
        lpDDSurfaceDesc->dwHeight = g_ddraw->height ? g_ddraw->height : 768;
        lpDDSurfaceDesc->dwWidth = g_ddraw->width ? g_ddraw->width : 1024;
        lpDDSurfaceDesc->lPitch = lpDDSurfaceDesc->dwWidth;
        lpDDSurfaceDesc->dwRefreshRate = 60;
        lpDDSurfaceDesc->ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);

        lpDDSurfaceDesc->ddpfPixelFormat.dwFlags = DDPF_PALETTEINDEXED8 | DDPF_RGB;
        lpDDSurfaceDesc->ddpfPixelFormat.dwRGBBitCount = 8;

        if (g_ddraw->bpp == 32)
        {
            lpDDSurfaceDesc->lPitch = lpDDSurfaceDesc->dwWidth * 4;
            lpDDSurfaceDesc->ddpfPixelFormat.dwFlags = DDPF_RGB;
            lpDDSurfaceDesc->ddpfPixelFormat.dwRGBBitCount = 32;
            lpDDSurfaceDesc->ddpfPixelFormat.dwRBitMask = 0xFF0000;
            lpDDSurfaceDesc->ddpfPixelFormat.dwGBitMask = 0x00FF00;
            lpDDSurfaceDesc->ddpfPixelFormat.dwBBitMask = 0x0000FF;
        }
        else if (g_ddraw->bpp != 8)
        {
            lpDDSurfaceDesc->lPitch = lpDDSurfaceDesc->dwWidth * 2;
            lpDDSurfaceDesc->ddpfPixelFormat.dwFlags = DDPF_RGB;
            lpDDSurfaceDesc->ddpfPixelFormat.dwRGBBitCount = 16;
            lpDDSurfaceDesc->ddpfPixelFormat.dwRBitMask = 0xF800;
            lpDDSurfaceDesc->ddpfPixelFormat.dwGBitMask = 0x07E0;
            lpDDSurfaceDesc->ddpfPixelFormat.dwBBitMask = 0x001F;
        }
    }

    return DD_OK;
}

HRESULT dd_GetMonitorFrequency(LPDWORD lpdwFreq)
{
    *lpdwFreq = 60;
    return DD_OK;
}

HRESULT dd_RestoreDisplayMode()
{
    if (!g_ddraw->render.run)
    {
        return DD_OK;
    }

    /* only stop drawing in GL mode when minimized */
    if (g_ddraw->renderer != gdi_render_main)
    {
        EnterCriticalSection(&g_ddraw->cs);
        g_ddraw->render.run = FALSE;
        ReleaseSemaphore(g_ddraw->render.sem, 1, NULL);
        LeaveCriticalSection(&g_ddraw->cs);

        if (g_ddraw->render.thread)
        {
            WaitForSingleObject(g_ddraw->render.thread, INFINITE);
            g_ddraw->render.thread = NULL;
        }

        if (g_ddraw->renderer == d3d9_render_main)
        {
            d3d9_release();
        }
    }
    
    if (!g_ddraw->windowed)
    {
        if (g_ddraw->renderer != d3d9_render_main)
        {
            ChangeDisplaySettings(NULL, 0);
        }
    }

    return DD_OK;
}

HRESULT dd_SetDisplayMode(DWORD width, DWORD height, DWORD bpp, BOOL set_by_game)
{
    if (bpp != 8 && bpp != 16 && bpp != 32)
        return DDERR_INVALIDMODE;

    if (g_ddraw->render.thread)
    {
        EnterCriticalSection(&g_ddraw->cs);
        g_ddraw->render.run = FALSE;
        ReleaseSemaphore(g_ddraw->render.sem, 1, NULL);
        LeaveCriticalSection(&g_ddraw->cs);

        WaitForSingleObject(g_ddraw->render.thread, INFINITE);
        g_ddraw->render.thread = NULL;
    }

    if (!g_ddraw->mode.dmPelsWidth)
    {
        ChangeDisplaySettings(NULL, 0);

        g_ddraw->mode.dmSize = sizeof(DEVMODE);
        g_ddraw->mode.dmDriverExtra = 0;

        if (EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &g_ddraw->mode) == FALSE)
        {
            g_ddraw->mode.dmSize = sizeof(DEVMODE);
            g_ddraw->mode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL | DM_DISPLAYFREQUENCY;
            g_ddraw->mode.dmPelsWidth = real_GetSystemMetrics(SM_CXSCREEN);
            g_ddraw->mode.dmPelsHeight = real_GetSystemMetrics(SM_CYSCREEN);
            g_ddraw->mode.dmDisplayFrequency = 60;
            g_ddraw->mode.dmBitsPerPel = 32;

            if (!g_ddraw->mode.dmPelsWidth || !g_ddraw->mode.dmPelsHeight)
            {
                g_ddraw->fullscreen = FALSE;
            }
        }
    }

    if (g_ddraw->altenter)
    {
        g_ddraw->altenter = FALSE;

        memset(&g_ddraw->render.mode, 0, sizeof(DEVMODE));

        g_ddraw->render.mode.dmSize = sizeof(DEVMODE);
        g_ddraw->render.mode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;
        g_ddraw->render.mode.dmPelsWidth = g_ddraw->render.width;
        g_ddraw->render.mode.dmPelsHeight = g_ddraw->render.height;

        if (g_ddraw->render.bpp)
        {
            g_ddraw->render.mode.dmFields |= DM_BITSPERPEL;
            g_ddraw->render.mode.dmBitsPerPel = g_ddraw->render.bpp;
        }

        if (ChangeDisplaySettings(&g_ddraw->render.mode, CDS_TEST) != DISP_CHANGE_SUCCESSFUL)
        {
            g_ddraw->render.width = g_ddraw->width;
            g_ddraw->render.height = g_ddraw->height;
        }
    }
    else
    {
        g_ddraw->render.width = g_config.window_rect.right;
        g_ddraw->render.height = g_config.window_rect.bottom;
    }

    //temporary fix: center window for games that keep changing their resolution
    if (g_ddraw->width && 
        (g_ddraw->width != width || g_ddraw->height != height) && 
        (width > g_config.window_rect.right || height > g_config.window_rect.bottom))
    {
        g_config.window_rect.left = -32000;
        g_config.window_rect.top = -32000;
    }

    g_ddraw->width = width;
    g_ddraw->height = height;
    g_ddraw->bpp = bpp;

    g_ddraw->cursor.x = width / 2;
    g_ddraw->cursor.y = height / 2;

    BOOL border = g_ddraw->border;

    if (g_ddraw->fullscreen)
    {
        g_ddraw->render.width = g_ddraw->mode.dmPelsWidth;
        g_ddraw->render.height = g_ddraw->mode.dmPelsHeight;

        if (g_ddraw->windowed) //windowed-fullscreen aka borderless
        {
            border = FALSE;

            g_config.window_rect.left = -32000;
            g_config.window_rect.top = -32000;

            // prevent OpenGL from going automatically into fullscreen exclusive mode
            if (g_ddraw->renderer == ogl_render_main)
                g_ddraw->render.height++;

        }
    }

    if (g_ddraw->render.width < g_ddraw->width)
    {
        g_ddraw->render.width = g_ddraw->width;
    }

    if (g_ddraw->render.height < g_ddraw->height)
    {
        g_ddraw->render.height = g_ddraw->height;
    }

    g_ddraw->render.run = TRUE;
    
    BOOL lock_mouse = g_ddraw->locked || g_ddraw->fullscreen;
    mouse_unlock();
	
    memset(&g_ddraw->render.mode, 0, sizeof(DEVMODE));

    g_ddraw->render.mode.dmSize = sizeof(DEVMODE);
    g_ddraw->render.mode.dmFields = DM_PELSWIDTH|DM_PELSHEIGHT;
    g_ddraw->render.mode.dmPelsWidth = g_ddraw->render.width;
    g_ddraw->render.mode.dmPelsHeight = g_ddraw->render.height;

    if (g_ddraw->render.bpp)
    {
        g_ddraw->render.mode.dmFields |= DM_BITSPERPEL;
        g_ddraw->render.mode.dmBitsPerPel = g_ddraw->render.bpp;
    }
    
    BOOL maintas = g_ddraw->maintas;

    if (!g_ddraw->windowed)
    {
        // Making sure the chosen resolution is valid
        int old_width = g_ddraw->render.width;
        int old_height = g_ddraw->render.height;

        if (ChangeDisplaySettings(&g_ddraw->render.mode, CDS_TEST) != DISP_CHANGE_SUCCESSFUL)
        {
            // fail... compare resolutions
            if (g_ddraw->render.width > g_ddraw->mode.dmPelsWidth || g_ddraw->render.height > g_ddraw->mode.dmPelsHeight)
            {
                // chosen game resolution higher than current resolution, use windowed mode for this case
                g_ddraw->windowed = TRUE;
            }
            else
            {
                // Try 2x scaling
                g_ddraw->render.width *= 2;
                g_ddraw->render.height *= 2;

                g_ddraw->render.mode.dmPelsWidth = g_ddraw->render.width;
                g_ddraw->render.mode.dmPelsHeight = g_ddraw->render.height;

                if ((g_ddraw->render.width > g_ddraw->mode.dmPelsWidth || g_ddraw->render.height > g_ddraw->mode.dmPelsHeight) ||
                    ChangeDisplaySettings(&g_ddraw->render.mode, CDS_TEST) != DISP_CHANGE_SUCCESSFUL)
                {
                    SIZE res = {0};

                    //try to get a resolution with the same aspect ratio as the requested resolution
                    BOOL found_res = util_get_lowest_resolution(
                        (float)old_width / old_height,
                        &res,
                        old_width + 1, //don't return the original resolution since we tested that one already
                        old_height + 1,
                        g_ddraw->mode.dmPelsWidth,
                        g_ddraw->mode.dmPelsHeight);

                    if (!found_res)
                    {
                        //try to get a resolution with the same aspect ratio as the current display mode
                        found_res = util_get_lowest_resolution(
                            (float)g_ddraw->mode.dmPelsWidth / g_ddraw->mode.dmPelsHeight,
                            &res,
                            old_width,
                            old_height,
                            g_ddraw->mode.dmPelsWidth,
                            g_ddraw->mode.dmPelsHeight);

                        if (found_res)
                            maintas = TRUE;
                    }

                    g_ddraw->render.width = res.cx;
                    g_ddraw->render.height = res.cy;

                    g_ddraw->render.mode.dmPelsWidth = g_ddraw->render.width;
                    g_ddraw->render.mode.dmPelsHeight = g_ddraw->render.height;
                    
                    if (!found_res || ChangeDisplaySettings(&g_ddraw->render.mode, CDS_TEST) != DISP_CHANGE_SUCCESSFUL)
                    {
                        // try current display settings
                        g_ddraw->render.width = g_ddraw->mode.dmPelsWidth;
                        g_ddraw->render.height = g_ddraw->mode.dmPelsHeight;

                        g_ddraw->render.mode.dmPelsWidth = g_ddraw->render.width;
                        g_ddraw->render.mode.dmPelsHeight = g_ddraw->render.height;

                        if (ChangeDisplaySettings(&g_ddraw->render.mode, CDS_TEST) != DISP_CHANGE_SUCCESSFUL)
                        {
                            // everything failed, use windowed mode instead
                            g_ddraw->render.width = old_width;
                            g_ddraw->render.height = old_height;

                            g_ddraw->render.mode.dmPelsWidth = g_ddraw->render.width;
                            g_ddraw->render.mode.dmPelsHeight = g_ddraw->render.height;

                            g_ddraw->windowed = TRUE;
                        }
                        else
                            maintas = TRUE;
                    }
                }
            }
        }
    }

    if (g_ddraw->nonexclusive && !g_ddraw->windowed && g_ddraw->renderer == ogl_render_main)
    {
        g_ddraw->render.height++;
    }

    if (!g_ddraw->handlemouse)
    {
        g_ddraw->boxing = maintas = FALSE;
    }

    g_ddraw->render.viewport.width = g_ddraw->render.width;
    g_ddraw->render.viewport.height = g_ddraw->render.height;
    g_ddraw->render.viewport.x = 0;
    g_ddraw->render.viewport.y = 0;
    
    if (g_ddraw->boxing)
    {
        g_ddraw->render.viewport.width = g_ddraw->width;
        g_ddraw->render.viewport.height = g_ddraw->height;

        int i;
        for (i = 20; i-- > 1;)
        {
            if (g_ddraw->width * i <= g_ddraw->render.width && g_ddraw->height * i <= g_ddraw->render.height)
            {
                g_ddraw->render.viewport.width *= i;
                g_ddraw->render.viewport.height *= i;
                break;
            }
        }

        g_ddraw->render.viewport.y = g_ddraw->render.height / 2 - g_ddraw->render.viewport.height / 2;
        g_ddraw->render.viewport.x = g_ddraw->render.width / 2 - g_ddraw->render.viewport.width / 2;
    }
    else if (maintas)
    {
        g_ddraw->render.viewport.width = g_ddraw->render.width;
        g_ddraw->render.viewport.height = (int)(((float)g_ddraw->height / g_ddraw->width) * g_ddraw->render.viewport.width);
        
        if (g_ddraw->render.viewport.height > g_ddraw->render.height)
        {
            g_ddraw->render.viewport.width = 
                (int)(((float)g_ddraw->render.viewport.width / g_ddraw->render.viewport.height) * g_ddraw->render.height);
                
            g_ddraw->render.viewport.height = g_ddraw->render.height;
        }
         
        g_ddraw->render.viewport.y = g_ddraw->render.height / 2 - g_ddraw->render.viewport.height / 2;
        g_ddraw->render.viewport.x = g_ddraw->render.width / 2 - g_ddraw->render.viewport.width / 2;
    }
    
    g_ddraw->render.scale_w = ((float)g_ddraw->render.viewport.width / g_ddraw->width);
    g_ddraw->render.scale_h = ((float)g_ddraw->render.viewport.height / g_ddraw->height);
    g_ddraw->render.unscale_w = ((float)g_ddraw->width / g_ddraw->render.viewport.width);
    g_ddraw->render.unscale_h = ((float)g_ddraw->height / g_ddraw->render.viewport.height);

    if (g_ddraw->windowed)
    {
        MSG msg; // workaround for "Not Responding" window problem in cnc games
        PeekMessage(&msg, g_ddraw->hwnd, 0, 0, PM_NOREMOVE);

        if (!border)
        {
            real_SetWindowLongA(g_ddraw->hwnd, GWL_STYLE, GetWindowLong(g_ddraw->hwnd, GWL_STYLE) & ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZE | WS_MAXIMIZE | WS_SYSMENU));
        }
        else
        {
            real_SetWindowLongA(g_ddraw->hwnd, GWL_STYLE, (GetWindowLong(g_ddraw->hwnd, GWL_STYLE) | WS_OVERLAPPEDWINDOW));// &~WS_MAXIMIZEBOX);
        }

        if (g_ddraw->wine)
            real_SetWindowLongA(g_ddraw->hwnd, GWL_STYLE, (GetWindowLong(g_ddraw->hwnd, GWL_STYLE) | WS_MINIMIZEBOX) & ~(WS_MAXIMIZEBOX | WS_THICKFRAME));

        /* center the window with correct dimensions */
        int cy = g_ddraw->mode.dmPelsWidth ? g_ddraw->mode.dmPelsWidth : g_ddraw->render.width;
        int cx = g_ddraw->mode.dmPelsHeight ? g_ddraw->mode.dmPelsHeight : g_ddraw->render.height;
        int x = (g_config.window_rect.left != -32000) ? g_config.window_rect.left : (cy / 2) - (g_ddraw->render.width / 2);
        int y = (g_config.window_rect.top != -32000) ? g_config.window_rect.top : (cx / 2) - (g_ddraw->render.height / 2);
        
        RECT dst = { x, y, g_ddraw->render.width + x, g_ddraw->render.height + y };
        
        AdjustWindowRect(&dst, GetWindowLong(g_ddraw->hwnd, GWL_STYLE), FALSE);
        real_SetWindowPos(g_ddraw->hwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
        real_MoveWindow(g_ddraw->hwnd, dst.left, dst.top, (dst.right - dst.left), (dst.bottom - dst.top), TRUE);

        BOOL d3d9_active = FALSE;

        if (g_ddraw->renderer == d3d9_render_main)
        {
            d3d9_active = d3d9_create();

            if (!d3d9_active)
            {
                d3d9_release();
                g_ddraw->show_driver_warning = TRUE;
                g_ddraw->renderer = gdi_render_main;
            }
        }

        if (lock_mouse)
            mouse_lock();
    }
    else
    {
        LONG style = GetWindowLong(g_ddraw->hwnd, GWL_STYLE);

        if ((style & WS_CAPTION))
        {
            real_SetWindowLongA(g_ddraw->hwnd, GWL_STYLE, style & ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZE | WS_MAXIMIZE | WS_SYSMENU));
        }

        BOOL d3d9_active = FALSE;

        if (g_ddraw->renderer == d3d9_render_main)
        {
            d3d9_active = d3d9_create();

            if (!d3d9_active)
            {
                d3d9_release();
                g_ddraw->show_driver_warning = TRUE;
                g_ddraw->renderer = gdi_render_main;
            }
        }

        if (!d3d9_active && ChangeDisplaySettings(&g_ddraw->render.mode, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
        {
            g_ddraw->render.run = FALSE;
            g_ddraw->windowed = TRUE;
            return dd_SetDisplayMode(width, height, bpp, FALSE);
        }

        if (g_ddraw->wine)
        {
            real_SetWindowLongA(g_ddraw->hwnd, GWL_STYLE, GetWindowLong(g_ddraw->hwnd, GWL_STYLE) | WS_MINIMIZEBOX);
        }

        real_SetWindowPos(g_ddraw->hwnd, HWND_TOPMOST, 0, 0, g_ddraw->render.width, g_ddraw->render.height, SWP_SHOWWINDOW);
        g_ddraw->last_set_window_pos_tick = timeGetTime();

        mouse_lock();
    }
    
    if (g_ddraw->render.viewport.x != 0 || g_ddraw->render.viewport.y != 0)
    {
        RedrawWindow(g_ddraw->hwnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE);
    }

    if (g_ddraw->render.thread == NULL)
    {
        InterlockedExchange(&g_ddraw->render.palette_updated, TRUE);
        InterlockedExchange(&g_ddraw->render.surface_updated, TRUE);
        ReleaseSemaphore(g_ddraw->render.sem, 1, NULL);

        g_ddraw->render.thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)g_ddraw->renderer, NULL, 0, NULL);
    }

    if (set_by_game)
    {
        //real_SendMessageA(g_ddraw->hwnd, WM_SIZE_DDRAW, 0, MAKELPARAM(g_ddraw->width, g_ddraw->height));
        real_SendMessageA(g_ddraw->hwnd, WM_MOVE_DDRAW, 0, MAKELPARAM(0, 0));
        real_SendMessageA(g_ddraw->hwnd, WM_DISPLAYCHANGE_DDRAW, g_ddraw->bpp, MAKELPARAM(g_ddraw->width, g_ddraw->height));
    }

    return DD_OK;
}

HRESULT dd_SetCooperativeLevel(HWND hwnd, DWORD dwFlags)
{
    PIXELFORMATDESCRIPTOR pfd;

    /* Red Alert for some weird reason does this on Windows XP */
    if (hwnd == NULL)
    {
        return DD_OK;
    }

    if (g_ddraw->hwnd == NULL)
    {
        g_ddraw->hwnd = hwnd;
    }

    if (!g_ddraw->wndproc)
    {
        hook_init();

        g_ddraw->wndproc = (WNDPROC)real_SetWindowLongA(g_ddraw->hwnd, GWL_WNDPROC, (LONG)fake_WndProc);

        if (!g_ddraw->render.hdc)
        {
            g_ddraw->render.hdc = GetDC(g_ddraw->hwnd);

            memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));

            pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
            pfd.nVersion = 1;
            pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER | (g_ddraw->renderer == ogl_render_main ? PFD_SUPPORT_OPENGL : 0);
            pfd.iPixelType = PFD_TYPE_RGBA;
            pfd.cColorBits = g_ddraw->render.bpp ? g_ddraw->render.bpp : g_ddraw->mode.dmBitsPerPel;
            pfd.iLayerType = PFD_MAIN_PLANE;

            SetPixelFormat(g_ddraw->render.hdc, ChoosePixelFormat(g_ddraw->render.hdc, &pfd), &pfd);
        }

        if (g_ddraw->handlemouse && g_ddraw->windowed)
        {
            while (real_ShowCursor(FALSE) > 0); //workaround for direct input games
            while (real_ShowCursor(TRUE) < 0);
        }

        real_SetCursor(LoadCursor(NULL, IDC_ARROW));

        GetWindowText(g_ddraw->hwnd, (LPTSTR)&g_ddraw->title, sizeof(g_ddraw->title));

        g_ddraw->isredalert = strcmp(g_ddraw->title, "Red Alert") == 0;
        g_ddraw->iscnc1 = strcmp(g_ddraw->title, "Command & Conquer") == 0;
        g_ddraw->iskkndx = strcmp(g_ddraw->title, "KKND Xtreme") == 0;

        if (g_ddraw->iskkndx)
        {
            g_ddraw->upscale_hack_width = 640;
            g_ddraw->upscale_hack_height = 480;
        }
        else if (g_ddraw->isredalert || g_ddraw->iscnc1)
        {
            g_ddraw->upscale_hack_width = 640;
            g_ddraw->upscale_hack_height = 400;
        }

        if (g_ddraw->vhack && !g_ddraw->isredalert && !g_ddraw->iscnc1 && !g_ddraw->iskkndx)
        {
            g_ddraw->vhack = 0;
        }
    }

    return DD_OK;
}

HRESULT dd_WaitForVerticalBlank(DWORD dwFlags, HANDLE h)
{
    if (g_ddraw->maxgameticks == -2)
    {
        if (fpsl_dwm_flush() || fpsl_wait_for_vblank(g_ddraw->render.maxfps >= 0 && !g_ddraw->vsync))
            return DD_OK;
    }

    if (!g_ddraw->flip_limiter.tick_length)
        return DD_OK;

    if (g_ddraw->flip_limiter.htimer)
    {
        FILETIME last_flip_ft = { 0 };
        GetSystemTimeAsFileTime(&last_flip_ft);

        if (!g_ddraw->flip_limiter.due_time.QuadPart)
        {
            memcpy(&g_ddraw->flip_limiter.due_time, &last_flip_ft, sizeof(LARGE_INTEGER));
        }
        else
        {
            while (CompareFileTime((FILETIME*)&g_ddraw->flip_limiter.due_time, &last_flip_ft) == -1)
                g_ddraw->flip_limiter.due_time.QuadPart += g_ddraw->flip_limiter.tick_length_ns;

            SetWaitableTimer(g_ddraw->flip_limiter.htimer, &g_ddraw->flip_limiter.due_time, 0, NULL, NULL, FALSE);
            WaitForSingleObject(g_ddraw->flip_limiter.htimer, g_ddraw->flip_limiter.tick_length * 2);
        }
    }
    else
    {
        static DWORD next_game_tick;

        if (!next_game_tick)
        {
            next_game_tick = timeGetTime();
            return DD_OK;
        }

        next_game_tick += g_ddraw->flip_limiter.tick_length;
        DWORD tick_count = timeGetTime();

        int sleep_time = next_game_tick - tick_count;

        if (sleep_time <= 0 || sleep_time > g_ddraw->flip_limiter.tick_length)
        {
            next_game_tick = tick_count;
        }
        else
        {
            Sleep(sleep_time);
        }
    }

    return DD_OK;
}

ULONG dd_AddRef()
{
    return ++g_ddraw->ref;
}

ULONG dd_Release()
{
    g_ddraw->ref--;

    if (g_ddraw->ref == 0)
    {
        if (g_ddraw->bpp)
        {
            cfg_save();
        }

        if (g_ddraw->render.run)
        {
            EnterCriticalSection(&g_ddraw->cs);
            g_ddraw->render.run = FALSE;
            ReleaseSemaphore(g_ddraw->render.sem, 1, NULL);
            LeaveCriticalSection(&g_ddraw->cs);

            if (g_ddraw->render.thread)
            {
                WaitForSingleObject(g_ddraw->render.thread, INFINITE);
                g_ddraw->render.thread = NULL;
            }

            if (g_ddraw->renderer == d3d9_render_main)
            {
                d3d9_release();
            }
            else if (!g_ddraw->windowed)
            {
                ChangeDisplaySettings(NULL, 0);
            }
        }

        if (g_ddraw->render.hdc)
        {
            ReleaseDC(g_ddraw->hwnd, g_ddraw->render.hdc);
            g_ddraw->render.hdc = NULL;
        }

        if (g_ddraw->ticks_limiter.htimer)
        {
            CancelWaitableTimer(g_ddraw->ticks_limiter.htimer);
            CloseHandle(g_ddraw->ticks_limiter.htimer);
            g_ddraw->ticks_limiter.htimer = NULL;
        }

        if (g_ddraw->flip_limiter.htimer)
        {
            CancelWaitableTimer(g_ddraw->flip_limiter.htimer);
            CloseHandle(g_ddraw->flip_limiter.htimer);
            g_ddraw->flip_limiter.htimer = NULL;
        }

        if (g_fpsl.htimer)
        {
            CancelWaitableTimer(g_fpsl.htimer);
            CloseHandle(g_fpsl.htimer);
            g_fpsl.htimer = NULL;
        }

        if (g_ddraw->real_dd)
        {
            g_ddraw->real_dd->lpVtbl->Release(g_ddraw->real_dd);
        }

        DeleteCriticalSection(&g_ddraw->cs);

        /* restore old wndproc, subsequent ddraw creation will otherwise fail */
        real_SetWindowLongA(g_ddraw->hwnd, GWL_WNDPROC, (LONG)g_ddraw->wndproc);

        HeapFree(GetProcessHeap(), 0, g_ddraw);
        g_ddraw = NULL;

        return 0;
    }

    return g_ddraw->ref;
}

HRESULT dd_GetAvailableVidMem(void* lpDDCaps, LPDWORD lpdwTotal, LPDWORD lpdwFree)
{
    *lpdwTotal = 16777216;
    *lpdwFree = 16777216;
    return DD_OK;
}

HRESULT dd_GetVerticalBlankStatus(LPBOOL lpbIsInVB)
{
    *lpbIsInVB = TRUE;
    return DD_OK;
}

HRESULT dd_CreateEx(GUID* lpGuid, LPVOID* lplpDD, REFIID iid, IUnknown* pUnkOuter)
{
    if (g_ddraw)
    {
        if (g_ddraw->passthrough)
        {
            if (iid && IsEqualGUID(&IID_IDirectDraw, iid))
            {
                if (!g_ddraw->real_dll)
                    g_ddraw->real_dll = LoadLibrary("system32\\ddraw.dll");

                if (g_ddraw->real_dll && !g_ddraw->DirectDrawCreate)
                    g_ddraw->DirectDrawCreate = (DIRECTDRAWCREATEPROC)GetProcAddress(g_ddraw->real_dll, "DirectDrawCreate");

                if (g_ddraw->DirectDrawCreate == DirectDrawCreate)
                    g_ddraw->DirectDrawCreate = NULL;

                if (g_ddraw->DirectDrawCreate)
                    return g_ddraw->DirectDrawCreate(lpGuid, (LPDIRECTDRAW*)lplpDD, pUnkOuter);
            }

            return DDERR_DIRECTDRAWALREADYCREATED;
        }
    }
    else
    {
        g_ddraw = (cnc_ddraw*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(cnc_ddraw));
        g_ddraw->ref++;

        InitializeCriticalSection(&g_ddraw->cs);

        g_ddraw->render.sem = CreateSemaphore(NULL, 0, 1, NULL);
        g_ddraw->wine = GetProcAddress(GetModuleHandleA("ntdll.dll"), "wine_get_version") != 0;

        cfg_load();
        g_ddraw->ref--;
    }

    IDirectDrawImpl* dd = (IDirectDrawImpl*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(IDirectDrawImpl));
    
    if (iid && IsEqualGUID(&IID_IDirectDraw, iid))
    {
        dprintf("     GUID = %08X (IID_IDirectDraw), ddraw = %p\n", ((GUID*)iid)->Data1, dd);

        dd->lpVtbl = &g_dd_vtbl1;
    }
    else
    {
        dprintf("     GUID = %08X (IID_IDirectDrawX), ddraw = %p\n", iid ? ((GUID*)iid)->Data1 : 0, dd);

        dd->lpVtbl = &g_dd_vtblx;
    }

    IDirectDraw_AddRef(dd);

    *lplpDD = (LPVOID)dd;

    return DD_OK;
}
