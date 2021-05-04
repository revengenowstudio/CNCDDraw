#include <windows.h>
#include <stdio.h>
#include "dllmain.h"
#include "dd.h"
#include "hook.h"
#include "ddsurface.h"
#include "mouse.h"
#include "scale_pattern.h"
#include "IDirectDrawSurface.h"
#include "winapi_hooks.h"
#include "debug.h"
#include "utils.h"


HRESULT dds_AddAttachedSurface(IDirectDrawSurfaceImpl *This, LPDIRECTDRAWSURFACE lpDDSurface)
{
    IDirectDrawSurface_AddRef(lpDDSurface);
    return DD_OK;
}

HRESULT dds_Blt(IDirectDrawSurfaceImpl *This, LPRECT lpDestRect, LPDIRECTDRAWSURFACE lpDDSrcSurface, LPRECT lpSrcRect, DWORD dwFlags, LPDDBLTFX lpDDBltFx)
{
    IDirectDrawSurfaceImpl *src_surface = (IDirectDrawSurfaceImpl *)lpDDSrcSurface;

    dbg_dump_dds_blt_flags(dwFlags);

    RECT src_rect = { 0, 0, src_surface ? src_surface->width : 0, src_surface ? src_surface->height : 0 };
    RECT dst_rect = { 0, 0, This->width, This->height };

    if (lpSrcRect && src_surface)
        memcpy(&src_rect, lpSrcRect, sizeof(src_rect));

    if (lpDestRect)
        memcpy(&dst_rect, lpDestRect, sizeof(dst_rect));

    // stretch or clip?
    BOOL is_stretch_blt = 
        ((src_rect.right - src_rect.left) != (dst_rect.right - dst_rect.left)) ||
        ((src_rect.bottom - src_rect.top) != (dst_rect.bottom - dst_rect.top));

    if (src_surface)
    {
        if (src_rect.right > src_surface->width)
            src_rect.right = src_surface->width;

        if (src_rect.left > src_rect.right)
            src_rect.left = src_rect.right;

        if (src_rect.bottom > src_surface->height)
            src_rect.bottom = src_surface->height;

        if (src_rect.top > src_rect.bottom)
            src_rect.top = src_rect.bottom;
    }

    if (dst_rect.right > This->width)
        dst_rect.right = This->width;

    if (dst_rect.left > dst_rect.right)
        dst_rect.left = dst_rect.right;

    if (dst_rect.bottom > This->height)
        dst_rect.bottom = This->height;

    if (dst_rect.top > dst_rect.bottom)
        dst_rect.top = dst_rect.bottom;

    int src_w = src_rect.right - src_rect.left;
    int src_h = src_rect.bottom - src_rect.top;
    int src_x = src_rect.left;
    int src_y = src_rect.top;

    int dst_w = dst_rect.right - dst_rect.left;
    int dst_h = dst_rect.bottom - dst_rect.top;
    int dst_x = dst_rect.left;
    int dst_y = dst_rect.top;

    void* dst_buf = dds_GetBuffer(This);
    void* src_buf = dds_GetBuffer(src_surface);

    if (dst_buf && (dwFlags & DDBLT_COLORFILL) && dst_w > 0 && dst_h > 0)
    {
        unsigned char *dst = (unsigned char *)dst_buf + (dst_x * This->lx_pitch) + (This->l_pitch * dst_y);
        unsigned char *first_row = dst;
        unsigned int dst_pitch = dst_w * This->lx_pitch;
        int x, i;

        if (This->bpp == 8)
        {
            unsigned char color = (unsigned char)lpDDBltFx->dwFillColor;

            for (i = 0; i < dst_h; i++)
            {
                memset(dst, color, dst_pitch);
                dst += This->l_pitch;
            }
        }
        else if (This->bpp == 16)
        {
            unsigned short *row1 = (unsigned short *)dst;
            unsigned short color = (unsigned short)lpDDBltFx->dwFillColor;

            if ((color & 0xFF) == (color >> 8))
            {
                unsigned char c8 = (unsigned char)(color & 0xFF);

                for (i = 0; i < dst_h; i++)
                {
                    memset(dst, c8, dst_pitch);
                    dst += This->l_pitch;
                }
            }
            else
            {
                for (x = 0; x < dst_w; x++)
                    row1[x] = color;

                for (i = 1; i < dst_h; i++)
                {
                    dst += This->l_pitch;
                    memcpy(dst, first_row, dst_pitch);
                }
            }
        }
    }

    if (src_surface && src_w > 0 && src_h > 0 && dst_w > 0 && dst_h > 0)
    {
        if ((dwFlags & DDBLT_KEYSRC) || (dwFlags & DDBLT_KEYSRCOVERRIDE))
        {
            DDCOLORKEY color_key;

            color_key.dwColorSpaceLowValue =
                (dwFlags & DDBLT_KEYSRCOVERRIDE) ? 
                    lpDDBltFx->ddckSrcColorkey.dwColorSpaceLowValue : src_surface->color_key.dwColorSpaceLowValue;

            color_key.dwColorSpaceHighValue =
                (dwFlags & DDBLT_KEYSRCOVERRIDE) ?
                    lpDDBltFx->ddckSrcColorkey.dwColorSpaceHighValue : src_surface->color_key.dwColorSpaceHighValue;
            
            if (!is_stretch_blt)
            {
                int width = dst_w > src_w ? src_w : dst_w;
                int height = dst_h > src_h ? src_h : dst_h;

                if (This->bpp == 8)
                {
                    int y1, x1;
                    for (y1 = 0; y1 < height; y1++)
                    {
                        int ydst = This->width * (y1 + dst_y);
                        int ysrc = src_surface->width * (y1 + src_y);

                        for (x1 = 0; x1 < width; x1++)
                        {
                            unsigned char c = ((unsigned char *)src_buf)[x1 + src_x + ysrc];

                            if (c < color_key.dwColorSpaceLowValue || c > color_key.dwColorSpaceHighValue)
                            {
                                ((unsigned char *)dst_buf)[x1 + dst_x + ydst] = c;
                            }
                        }
                    }
                }
                else if (This->bpp == 16)
                {
                    int y1, x1;
                    for (y1 = 0; y1 < height; y1++)
                    {
                        int ydst = This->width * (y1 + dst_y);
                        int ysrc = src_surface->width * (y1 + src_y);

                        for (x1 = 0; x1 < width; x1++)
                        {
                            unsigned short c = ((unsigned short *)src_buf)[x1 + src_x + ysrc];

                            if (c < color_key.dwColorSpaceLowValue || c > color_key.dwColorSpaceHighValue)
                            {
                                ((unsigned short *)dst_buf)[x1 + dst_x + ydst] = c;
                            }
                        }
                    }
                }
            }
            else
            {
                dprintfex("NOT_IMPLEMENTED     DDBLT_KEYSRC / DDBLT_KEYSRCOVERRIDE does not support stretching\n");
            }
        }
        else
        {
            if (!is_stretch_blt)
            {
                int width = dst_w > src_w ? src_w : dst_w;
                int height = dst_h > src_h ? src_h : dst_h;

                unsigned char *src = 
                    (unsigned char *)src_buf + (src_x * src_surface->lx_pitch) + (src_surface->l_pitch * src_y);

                unsigned char *dst = 
                    (unsigned char *)dst_buf + (dst_x * This->lx_pitch) + (This->l_pitch * dst_y);

                unsigned int dst_pitch = width * This->lx_pitch;

                if (This == src_surface)
                {
                    if (dst_y > src_y)
                    {
                        src += src_surface->l_pitch * height;
                        dst += This->l_pitch * height;

                        for (int i = height; i-- > 0;)
                        {
                            src -= src_surface->l_pitch;
                            dst -= This->l_pitch;

                            memmove(dst, src, dst_pitch);
                        }
                    }
                    else
                    {
                        for (int i = 0; i < height; i++)
                        {
                            memmove(dst, src, dst_pitch);

                            src += src_surface->l_pitch;
                            dst += This->l_pitch;
                        }
                    }
                }
                else
                {
                    for (int i = 0; i < height; i++)
                    {
                        memcpy(dst, src, dst_pitch);

                        src += src_surface->l_pitch;
                        dst += This->l_pitch;
                    }
                }
            }
            else
            {
                /* Linear scaling using integer math
                * Since the scaling pattern for x will aways be the same, the pattern itself gets pre-calculated
                * and stored in an array.
                * Y scaling pattern gets calculated during the blit loop
                */
                unsigned int x_ratio = (unsigned int)((src_w << 16) / dst_w) + 1;
                unsigned int y_ratio = (unsigned int)((src_h << 16) / dst_h) + 1;

                unsigned int s_src_x, s_src_y;
                unsigned int dest_base, source_base;

                scale_pattern *pattern = malloc((dst_w + 1) * (sizeof(scale_pattern)));
                int pattern_idx = 0;
                unsigned int last_src_x = 0;

                if (pattern != NULL)
                {
                    pattern[pattern_idx] = (scale_pattern) { ONCE, 0, 0, 1 };

                    /* Build the pattern! */
                    int x;
                    for (x = 1; x < dst_w; x++) {
                        s_src_x = (x * x_ratio) >> 16;
                        if (s_src_x == last_src_x)
                        {
                            if (pattern[pattern_idx].type == REPEAT || pattern[pattern_idx].type == ONCE)
                            {
                                pattern[pattern_idx].type = REPEAT;
                                pattern[pattern_idx].count++;
                            }
                            else if (pattern[pattern_idx].type == SEQUENCE)
                            {
                                pattern_idx++;
                                pattern[pattern_idx] = (scale_pattern) { REPEAT, x, s_src_x, 1 };
                            }
                        }
                        else if (s_src_x == last_src_x + 1)
                        {
                            if (pattern[pattern_idx].type == SEQUENCE || pattern[pattern_idx].type == ONCE)
                            {
                                pattern[pattern_idx].type = SEQUENCE;
                                pattern[pattern_idx].count++;
                            }
                            else if (pattern[pattern_idx].type == REPEAT)
                            {
                                pattern_idx++;
                                pattern[pattern_idx] = (scale_pattern) { ONCE, x, s_src_x, 1 };
                            }
                        }
                        else
                        {
                            pattern_idx++;
                            pattern[pattern_idx] = (scale_pattern) { ONCE, x, s_src_x, 1 };
                        }
                        last_src_x = s_src_x;
                    }
                    pattern[pattern_idx + 1] = (scale_pattern) { END, 0, 0, 0 };


                    /* Do the actual blitting */
                    int count = 0;
                    int y;

                    for (y = 0; y < dst_h; y++) {

                        dest_base = dst_x + This->width * (y + dst_y);

                        s_src_y = (y * y_ratio) >> 16;

                        source_base = src_x + src_surface->width * (s_src_y + src_y);

                        pattern_idx = 0;
                        scale_pattern *current = &pattern[pattern_idx];

                        if (This->bpp == 8)
                        {
                            unsigned char *d, *s, v;
                            unsigned char *src = (unsigned char *)src_buf;
                            unsigned char *dst = (unsigned char *)dst_buf;

                            do {
                                switch (current->type)
                                {
                                case ONCE:
                                    dst[dest_base + current->dst_index] =
                                        src[source_base + current->src_index];
                                    break;

                                case REPEAT:
                                    d = (dst + dest_base + current->dst_index);
                                    v = src[source_base + current->src_index];

                                    count = current->count;
                                    while (count-- > 0)
                                        *d++ = v;

                                    break;

                                case SEQUENCE:
                                    d = dst + dest_base + current->dst_index;
                                    s = src + source_base + current->src_index;

                                    memcpy((void *)d, (void *)s, current->count * This->lx_pitch);
                                    break;

                                case END:
                                default:
                                    break;
                                }

                                current = &pattern[++pattern_idx];
                            } while (current->type != END);
                        }
                        else if (This->bpp == 16)
                        {
                            unsigned short *d, *s, v;
                            unsigned short *src = (unsigned short *)src_buf;
                            unsigned short *dst = (unsigned short *)dst_buf;

                            do {
                                switch (current->type)
                                {
                                case ONCE:
                                    dst[dest_base + current->dst_index] =
                                        src[source_base + current->src_index];
                                    break;

                                case REPEAT:
                                    d = (dst + dest_base + current->dst_index);
                                    v = src[source_base + current->src_index];

                                    count = current->count;
                                    while (count-- > 0)
                                        *d++ = v;

                                    break;

                                case SEQUENCE:
                                    d = dst + dest_base + current->dst_index;
                                    s = src + source_base + current->src_index;

                                    memcpy((void *)d, (void *)s, current->count * This->lx_pitch);
                                    break;

                                case END:
                                default:
                                    break;
                                }

                                current = &pattern[++pattern_idx];
                            } while (current->type != END);
                        }
                    }
                    free(pattern);
                }
            }

        }
    }

    if((This->caps & DDSCAPS_PRIMARYSURFACE) && g_ddraw->render.run)
    {
        InterlockedExchange(&g_ddraw->render.surface_updated, TRUE);

        if (!(This->flags & DDSD_BACKBUFFERCOUNT) || This->last_flip_tick + FLIP_REDRAW_TIMEOUT < timeGetTime())
        {
            This->last_blt_tick = timeGetTime();

            ReleaseSemaphore(g_ddraw->render.sem, 1, NULL);
            SwitchToThread();

            if (g_ddraw->ticks_limiter.tick_length > 0)
            {
                g_ddraw->ticks_limiter.use_blt_or_flip = TRUE;
                util_limit_game_ticks();
            }
        }
    }

    return DD_OK;
}

HRESULT dds_BltFast(IDirectDrawSurfaceImpl *This, DWORD dst_x, DWORD dst_y, LPDIRECTDRAWSURFACE lpDDSrcSurface, LPRECT lpSrcRect, DWORD flags)
{
    IDirectDrawSurfaceImpl *src_surface = (IDirectDrawSurfaceImpl *)lpDDSrcSurface;

    dbg_dump_dds_blt_fast_flags(flags);

    RECT src_rect = { 0, 0, src_surface ? src_surface->width : 0, src_surface ? src_surface->height : 0 };

    if (lpSrcRect && src_surface)
    {
        memcpy(&src_rect, lpSrcRect, sizeof(src_rect));

        if (src_rect.right > src_surface->width)
            src_rect.right = src_surface->width;

        if (src_rect.left > src_rect.right)
            src_rect.left = src_rect.right;

        if (src_rect.bottom > src_surface->height)
            src_rect.bottom = src_surface->height;

        if (src_rect.top > src_rect.bottom)
            src_rect.top = src_rect.bottom;
    }

    int src_x = src_rect.left;
    int src_y = src_rect.top;

    RECT dst_rect = { dst_x, dst_y, (src_rect.right - src_rect.left) + dst_x, (src_rect.bottom - src_rect.top) + dst_y };

    if (dst_rect.right > This->width)
        dst_rect.right = This->width;

    if (dst_rect.left > dst_rect.right)
        dst_rect.left = dst_rect.right;

    if (dst_rect.bottom > This->height)
        dst_rect.bottom = This->height;

    if (dst_rect.top > dst_rect.bottom)
        dst_rect.top = dst_rect.bottom;

    int dst_w = dst_rect.right - dst_rect.left;
    int dst_h = dst_rect.bottom - dst_rect.top;

    void* dst_buf = dds_GetBuffer(This);
    void* src_buf = dds_GetBuffer(src_surface);

    if (src_surface && dst_w > 0 && dst_h > 0)
    {
        if (flags & DDBLTFAST_SRCCOLORKEY)
        {
            if (This->bpp == 8)
            {
                int y1, x1;
                for (y1 = 0; y1 < dst_h; y1++)
                {
                    int ydst = This->width * (y1 + dst_y);
                    int ysrc = src_surface->width * (y1 + src_y);

                    for (x1 = 0; x1 < dst_w; x1++)
                    {
                        unsigned char c = ((unsigned char *)src_buf)[x1 + src_x + ysrc];

                        if (c < src_surface->color_key.dwColorSpaceLowValue || c > src_surface->color_key.dwColorSpaceHighValue)
                        {
                            ((unsigned char *)dst_buf)[x1 + dst_x + ydst] = c;
                        }
                    }
                }
            }
            else if (This->bpp == 16)
            {
                int y1, x1;
                for (y1 = 0; y1 < dst_h; y1++)
                {
                    int ydst = This->width * (y1 + dst_y);
                    int ysrc = src_surface->width * (y1 + src_y);

                    for (x1 = 0; x1 < dst_w; x1++)
                    {
                        unsigned short c = ((unsigned short *)src_buf)[x1 + src_x + ysrc];
                        
                        if (c < src_surface->color_key.dwColorSpaceLowValue || c > src_surface->color_key.dwColorSpaceHighValue)
                        {
                            ((unsigned short *)dst_buf)[x1 + dst_x + ydst] = c;
                        }
                    }
                }
            }
        }
        else
        {
            unsigned char *src =
                (unsigned char *)src_buf + (src_x * src_surface->lx_pitch) + (src_surface->l_pitch * src_y);

            unsigned char *dst =
                (unsigned char *)dst_buf + (dst_x * This->lx_pitch) + (This->l_pitch * dst_y);

            unsigned int dst_pitch = dst_w * This->lx_pitch;

            if (This == src_surface)
            {
                if (dst_y > src_y)
                {
                    src += src_surface->l_pitch * dst_h;
                    dst += This->l_pitch * dst_h;

                    for (int i = dst_h; i-- > 0;)
                    {
                        src -= src_surface->l_pitch;
                        dst -= This->l_pitch;

                        memmove(dst, src, dst_pitch);
                    }
                }
                else
                {
                    for (int i = 0; i < dst_h; i++)
                    {
                        memmove(dst, src, dst_pitch);

                        src += src_surface->l_pitch;
                        dst += This->l_pitch;
                    }
                }
            }
            else
            {
                for (int i = 0; i < dst_h; i++)
                {
                    memcpy(dst, src, dst_pitch);

                    src += src_surface->l_pitch;
                    dst += This->l_pitch;
                }
            }
        }
    }

    if ((This->caps & DDSCAPS_PRIMARYSURFACE) && g_ddraw->render.run)
    {
        InterlockedExchange(&g_ddraw->render.surface_updated, TRUE);

        DWORD time = timeGetTime();

        if (!(This->flags & DDSD_BACKBUFFERCOUNT) || 
            (This->last_flip_tick + FLIP_REDRAW_TIMEOUT < time && This->last_blt_tick + FLIP_REDRAW_TIMEOUT < time))
        {
            ReleaseSemaphore(g_ddraw->render.sem, 1, NULL);
        }
    }

    return DD_OK;
}

HRESULT dds_DeleteAttachedSurface(IDirectDrawSurfaceImpl *This, DWORD dwFlags, LPDIRECTDRAWSURFACE lpDDSurface)
{
    IDirectDrawSurface_Release(lpDDSurface);
    return DD_OK;
}

HRESULT dds_GetSurfaceDesc(IDirectDrawSurfaceImpl *This, LPDDSURFACEDESC lpDDSurfaceDesc)
{
    if (lpDDSurfaceDesc)
    {
        memset(lpDDSurfaceDesc, 0, sizeof(DDSURFACEDESC));

        lpDDSurfaceDesc->dwSize = sizeof(DDSURFACEDESC);
        lpDDSurfaceDesc->dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_PITCH | DDSD_PIXELFORMAT | DDSD_LPSURFACE;
        lpDDSurfaceDesc->dwWidth = This->width;
        lpDDSurfaceDesc->dwHeight = This->height;
        lpDDSurfaceDesc->lPitch = This->l_pitch;
        lpDDSurfaceDesc->lpSurface = dds_GetBuffer(This);
        lpDDSurfaceDesc->ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
        lpDDSurfaceDesc->ddpfPixelFormat.dwFlags = DDPF_RGB;
        lpDDSurfaceDesc->ddpfPixelFormat.dwRGBBitCount = This->bpp;
        lpDDSurfaceDesc->ddsCaps.dwCaps = This->caps | DDSCAPS_VIDEOMEMORY;

        if (This->bpp == 8)
        {
            lpDDSurfaceDesc->ddpfPixelFormat.dwFlags |= DDPF_PALETTEINDEXED8;
        }
        else if (This->bpp == 16)
        {
            lpDDSurfaceDesc->ddpfPixelFormat.dwRBitMask = 0xF800;
            lpDDSurfaceDesc->ddpfPixelFormat.dwGBitMask = 0x07E0;
            lpDDSurfaceDesc->ddpfPixelFormat.dwBBitMask = 0x001F;
        }
    }

    return DD_OK;
}

HRESULT dds_EnumAttachedSurfaces(IDirectDrawSurfaceImpl *This, LPVOID lpContext, LPDDENUMSURFACESCALLBACK lpEnumSurfacesCallback)
{
    /* this is not actually complete, but Carmageddon seems to call EnumAttachedSurfaces instead of GetSurfaceDesc to get the main surface description */
    static DDSURFACEDESC dd_surface_desc;
    memset(&dd_surface_desc, 0, sizeof(DDSURFACEDESC));

    dds_GetSurfaceDesc(This, &dd_surface_desc);
    This->caps |= DDSCAPS_BACKBUFFER; // Nox hack
    lpEnumSurfacesCallback((LPDIRECTDRAWSURFACE)This, &dd_surface_desc, lpContext);

    if ((This->caps & DDSCAPS_PRIMARYSURFACE) && (This->caps & DDSCAPS_FLIP) && !(This->caps & DDSCAPS_BACKBUFFER))
        IDirectDrawSurface_AddRef(This);

    This->caps &= ~DDSCAPS_BACKBUFFER;

    return DD_OK;
}

HRESULT dds_Flip(IDirectDrawSurfaceImpl *This, LPDIRECTDRAWSURFACE surface, DWORD flags)
{
    if(This->caps & DDSCAPS_PRIMARYSURFACE && g_ddraw->render.run)
    {
        if (This->backbuffer)
        {
            EnterCriticalSection(&g_ddraw->cs);
            void* surface = InterlockedExchangePointer(&This->surface, This->backbuffer->surface);
            HBITMAP bitmap = (HBITMAP)InterlockedExchangePointer(&This->bitmap, This->backbuffer->bitmap);
            HDC hdc = (HDC)InterlockedExchangePointer(&This->hdc, This->backbuffer->hdc);

            InterlockedExchangePointer(&This->backbuffer->surface, surface);
            InterlockedExchangePointer(&This->backbuffer->bitmap, bitmap);
            InterlockedExchangePointer(&This->backbuffer->hdc, hdc);
            LeaveCriticalSection(&g_ddraw->cs);
        }

        This->last_flip_tick = timeGetTime();

        InterlockedExchange(&g_ddraw->render.surface_updated, TRUE);
        ReleaseSemaphore(g_ddraw->render.sem, 1, NULL);
        SwitchToThread();

        if (flags & DDFLIP_WAIT)
        {
            dd_WaitForVerticalBlank(DDWAITVB_BLOCKEND, NULL);
        }

        if (g_ddraw->ticks_limiter.tick_length > 0)
        {
            g_ddraw->ticks_limiter.use_blt_or_flip = TRUE;
            util_limit_game_ticks();
        }
    }

    return DD_OK;
}

HRESULT dds_GetAttachedSurface(IDirectDrawSurfaceImpl *This, LPDDSCAPS lpDdsCaps, LPDIRECTDRAWSURFACE FAR *surface)
{
    if ((This->caps & DDSCAPS_PRIMARYSURFACE) && (This->caps & DDSCAPS_FLIP) && (lpDdsCaps->dwCaps & DDSCAPS_BACKBUFFER))
    {
        if (This->backbuffer)
        {
            IDirectDrawSurface_AddRef(This->backbuffer);
            *surface = (LPDIRECTDRAWSURFACE)This->backbuffer;
        }
        else
        {
            IDirectDrawSurface_AddRef(This);
            *surface = (LPDIRECTDRAWSURFACE)This;
        }
    }

    return DD_OK;
}

HRESULT dds_GetCaps(IDirectDrawSurfaceImpl *This, LPDDSCAPS lpDDSCaps)
{
    lpDDSCaps->dwCaps = This->caps;
    return DD_OK;
}

HRESULT dds_GetColorKey(IDirectDrawSurfaceImpl *This, DWORD flags, LPDDCOLORKEY colorKey)
{
    if (colorKey)
    {
        colorKey->dwColorSpaceHighValue = This->color_key.dwColorSpaceHighValue;
        colorKey->dwColorSpaceLowValue = This->color_key.dwColorSpaceLowValue;
    }

    return DD_OK;
}

HRESULT dds_GetDC(IDirectDrawSurfaceImpl *This, HDC FAR *a)
{
    if ((This->l_pitch % 4))
    {
        dprintf("NOT_IMPLEMENTED     GetDC: width=%d height=%d\n", This->width, This->height);
    }

    RGBQUAD *data = 
        This->palette && This->palette->data_rgb ? This->palette->data_rgb :
        g_ddraw->primary && g_ddraw->primary->palette ? g_ddraw->primary->palette->data_rgb :
        NULL;

    if (data)
        SetDIBColorTable(dds_GetHDC(This), 0, 256, data);

    *a = dds_GetHDC(This);
    return DD_OK;
}

HRESULT dds_GetPalette(IDirectDrawSurfaceImpl *This, LPDIRECTDRAWPALETTE FAR *lplpDDPalette)
{
    *lplpDDPalette = (LPDIRECTDRAWPALETTE)This->palette;
    
    if (This->palette)
    {
        return DD_OK;
    }
    else
    {
        return DDERR_NOPALETTEATTACHED;
    }
}

HRESULT dds_GetPixelFormat(IDirectDrawSurfaceImpl *This, LPDDPIXELFORMAT ddpfPixelFormat)
{
    if (ddpfPixelFormat)
    {
        memset(ddpfPixelFormat, 0, sizeof(DDPIXELFORMAT));

        ddpfPixelFormat->dwSize = sizeof(DDPIXELFORMAT);
        ddpfPixelFormat->dwFlags = DDPF_RGB;
        ddpfPixelFormat->dwRGBBitCount = This->bpp;

        if (This->bpp == 8)
        {
            ddpfPixelFormat->dwFlags |= DDPF_PALETTEINDEXED8;
        }
        else if (This->bpp == 16)
        {
            ddpfPixelFormat->dwRBitMask = 0xF800;
            ddpfPixelFormat->dwGBitMask = 0x07E0;
            ddpfPixelFormat->dwBBitMask = 0x001F;
        }

        return DD_OK;
    }

    return DDERR_INVALIDPARAMS;
}

HRESULT dds_Lock(IDirectDrawSurfaceImpl *This, LPRECT lpDestRect, LPDDSURFACEDESC lpDDSurfaceDesc, DWORD dwFlags, HANDLE hEvent)
{
    dbg_dump_dds_lock_flags(dwFlags);

    HRESULT ret = dds_GetSurfaceDesc(This, lpDDSurfaceDesc);

    if (lpDestRect && lpDDSurfaceDesc && lpDestRect->left >= 0 && lpDestRect->top >= 0)
    {
        lpDDSurfaceDesc->lpSurface = 
            (char*)dds_GetBuffer(This) + (lpDestRect->left * This->lx_pitch) + (lpDestRect->top * This->l_pitch);
    }

    return ret;
}

HRESULT dds_SetColorKey(IDirectDrawSurfaceImpl *This, DWORD flags, LPDDCOLORKEY colorKey)
{
    if (colorKey)
    {
        dprintfex("     dwColorSpaceHighValue=%d\n", colorKey->dwColorSpaceHighValue);
        dprintfex("     dwColorSpaceLowValue=%d\n", colorKey->dwColorSpaceLowValue);

        This->color_key.dwColorSpaceHighValue = colorKey->dwColorSpaceHighValue;
        This->color_key.dwColorSpaceLowValue = colorKey->dwColorSpaceLowValue;
    }

    return DD_OK;
}

HRESULT dds_SetPalette(IDirectDrawSurfaceImpl *This, LPDIRECTDRAWPALETTE lpDDPalette)
{
    if (!lpDDPalette)
        return DDERR_INVALIDPARAMS;

    IDirectDrawPalette_AddRef(lpDDPalette);

    if (This->palette)
    {
        IDirectDrawPalette_Release(This->palette);
    }

    EnterCriticalSection(&g_ddraw->cs);

    This->palette = (IDirectDrawPaletteImpl *)lpDDPalette;
    This->palette->data_rgb = &This->bmi->bmiColors[0];

    int i;
    for (i = 0; i < 256; i++)
    {
        This->palette->data_rgb[i].rgbRed = This->palette->data_bgr[i] & 0xFF;
        This->palette->data_rgb[i].rgbGreen = (This->palette->data_bgr[i] >> 8) & 0xFF;
        This->palette->data_rgb[i].rgbBlue = (This->palette->data_bgr[i] >> 16) & 0xFF;
        This->palette->data_rgb[i].rgbReserved = 0;
    }

    LeaveCriticalSection(&g_ddraw->cs);

    return DD_OK;
}

HRESULT dds_Unlock(IDirectDrawSurfaceImpl *This, LPVOID lpRect)
{
    HWND hwnd = g_ddraw->bnet_active ? FindWindowEx(HWND_DESKTOP, NULL, "SDlgDialog", NULL) : NULL;

    if (hwnd && (This->caps & DDSCAPS_PRIMARYSURFACE))
    {
        if (g_ddraw->primary->palette && g_ddraw->primary->palette->data_rgb)
            SetDIBColorTable(dds_GetHDC(g_ddraw->primary), 0, 256, g_ddraw->primary->palette->data_rgb);

        //GdiTransparentBlt idea taken from Aqrit's war2 ddraw

        RGBQUAD quad;
        GetDIBColorTable(dds_GetHDC(g_ddraw->primary), 0xFE, 1, &quad);
        COLORREF color = RGB(quad.rgbRed, quad.rgbGreen, quad.rgbBlue);
        BOOL erase = FALSE;

        do
        {
            RECT rc;
            if (fake_GetWindowRect(hwnd, &rc))
            {
                if (rc.bottom - rc.top == 479)
                    erase = TRUE;

                HDC hdc = GetDCEx(hwnd, NULL, DCX_PARENTCLIP | DCX_CACHE);

                GdiTransparentBlt(
                    hdc,
                    0,
                    0,
                    rc.right - rc.left,
                    rc.bottom - rc.top,
                    dds_GetHDC(g_ddraw->primary),
                    rc.left,
                    rc.top,
                    rc.right - rc.left,
                    rc.bottom - rc.top,
                    color
                );

                ReleaseDC(hwnd, hdc);
            }

        } while ((hwnd = FindWindowEx(HWND_DESKTOP, hwnd, "SDlgDialog", NULL)));

        if (erase)
        {
            BOOL x = g_ddraw->ticks_limiter.use_blt_or_flip;

            DDBLTFX fx = { .dwFillColor = 0xFE };
            IDirectDrawSurface_Blt(This, NULL, NULL, NULL, DDBLT_COLORFILL, &fx);

            g_ddraw->ticks_limiter.use_blt_or_flip = x;
        }
    }

    if ((This->caps & DDSCAPS_PRIMARYSURFACE) && g_ddraw->render.run)
    {
        InterlockedExchange(&g_ddraw->render.surface_updated, TRUE);

        DWORD time = timeGetTime();

        if (!(This->flags & DDSD_BACKBUFFERCOUNT) ||
            (This->last_flip_tick + FLIP_REDRAW_TIMEOUT < time && This->last_blt_tick + FLIP_REDRAW_TIMEOUT < time))
        {
            ReleaseSemaphore(g_ddraw->render.sem, 1, NULL);

            if (g_ddraw->ticks_limiter.tick_length > 0 && !g_ddraw->ticks_limiter.use_blt_or_flip)
                util_limit_game_ticks();
        }
    }

    return DD_OK;
}

void* dds_GetBuffer(IDirectDrawSurfaceImpl* This)
{
    if (!This)
        return NULL;

    if (This->backbuffer || (This->caps & DDSCAPS_BACKBUFFER))
        return (void*)InterlockedExchangeAdd(&This->surface, 0);

    return This->surface;
}

HDC dds_GetHDC(IDirectDrawSurfaceImpl* This)
{
    if (!This)
        return NULL;

    if (This->backbuffer || (This->caps & DDSCAPS_BACKBUFFER))
        return (HDC)InterlockedExchangeAdd(&This->hdc, 0);

    return This->hdc;
}

HRESULT dd_CreateSurface(LPDDSURFACEDESC lpDDSurfaceDesc, LPDIRECTDRAWSURFACE FAR *lpDDSurface, IUnknown FAR * unkOuter)
{
    dbg_dump_dds_flags(lpDDSurfaceDesc->dwFlags);

    IDirectDrawSurfaceImpl *dst_surface = (IDirectDrawSurfaceImpl *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(IDirectDrawSurfaceImpl));

    dst_surface->lpVtbl = &g_dds_vtbl;

    lpDDSurfaceDesc->dwFlags |= DDSD_CAPS;

    /* private stuff */
    dst_surface->bpp = g_ddraw->bpp;
    dst_surface->flags = lpDDSurfaceDesc->dwFlags;

    if(lpDDSurfaceDesc->dwFlags & DDSD_CAPS)
    {
        if(lpDDSurfaceDesc->ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE)
        {
            g_ddraw->primary = dst_surface;

            dst_surface->width = g_ddraw->width;
            dst_surface->height = g_ddraw->height;
        }

        dbg_dump_dds_caps(lpDDSurfaceDesc->ddsCaps.dwCaps);
        dst_surface->caps = lpDDSurfaceDesc->ddsCaps.dwCaps;
    }

    if( !(lpDDSurfaceDesc->dwFlags & DDSD_CAPS) || !(lpDDSurfaceDesc->ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE) )
    {
        dst_surface->width = lpDDSurfaceDesc->dwWidth;
        dst_surface->height = lpDDSurfaceDesc->dwHeight;
    }

    if(dst_surface->width && dst_surface->height)
    {
        if (dst_surface->width == 622 && dst_surface->height == 51) dst_surface->width = 624; //AoE2
        if (dst_surface->width == 71 && dst_surface->height == 24) dst_surface->width = 72; //Commandos
        if (dst_surface->width == 343 && g_ddraw->bpp == 16) dst_surface->width = 344; //Blade & Sword

        dst_surface->bmi = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * 256);
        dst_surface->bmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        dst_surface->bmi->bmiHeader.biWidth = dst_surface->width;
        dst_surface->bmi->bmiHeader.biHeight = -(dst_surface->height + 200);
        dst_surface->bmi->bmiHeader.biPlanes = 1;
        dst_surface->bmi->bmiHeader.biBitCount = dst_surface->bpp;
        dst_surface->bmi->bmiHeader.biCompression = dst_surface->bpp == 16 ? BI_BITFIELDS : BI_RGB;

        WORD clr_bits = (WORD)(dst_surface->bmi->bmiHeader.biPlanes * dst_surface->bmi->bmiHeader.biBitCount);

        if (clr_bits < 24)
        {
            dst_surface->bmi->bmiHeader.biClrUsed = (1 << clr_bits);
        }

        dst_surface->bmi->bmiHeader.biSizeImage = ((dst_surface->width * clr_bits + 31) & ~31) / 8 * dst_surface->height;

        if (dst_surface->bpp == 8)
        {
            int i;
            for (i = 0; i < 256; i++)
            {
                dst_surface->bmi->bmiColors[i].rgbRed = i;
                dst_surface->bmi->bmiColors[i].rgbGreen = i;
                dst_surface->bmi->bmiColors[i].rgbBlue = i;
                dst_surface->bmi->bmiColors[i].rgbReserved = 0;
            }
        }
        else if (dst_surface->bpp == 16)
        {
            ((DWORD *)dst_surface->bmi->bmiColors)[0] = 0xF800;
            ((DWORD *)dst_surface->bmi->bmiColors)[1] = 0x07E0;
            ((DWORD *)dst_surface->bmi->bmiColors)[2] = 0x001F;
        }

        dst_surface->lx_pitch = dst_surface->bpp / 8;
        dst_surface->l_pitch = dst_surface->width * dst_surface->lx_pitch;

        dst_surface->hdc = CreateCompatibleDC(g_ddraw->render.hdc);
        dst_surface->bitmap = CreateDIBSection(dst_surface->hdc, dst_surface->bmi, DIB_RGB_COLORS, (void **)&dst_surface->surface, NULL, 0);
        dst_surface->bmi->bmiHeader.biHeight = -dst_surface->height;

        if (!dst_surface->bitmap)
        {
            dst_surface->surface = 
                HeapAlloc(
                    GetProcessHeap(), 
                    HEAP_ZERO_MEMORY, 
                    dst_surface->l_pitch * (dst_surface->height + 200) * dst_surface->lx_pitch);
        }

        if (lpDDSurfaceDesc->ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE)
        {
            FakePrimarySurface = dst_surface->surface;
        }

        SelectObject(dst_surface->hdc, dst_surface->bitmap);
    }

    if (lpDDSurfaceDesc->dwFlags & DDSD_BACKBUFFERCOUNT)
    {
        dprintf("     dwBackBufferCount=%d\n", lpDDSurfaceDesc->dwBackBufferCount);

        if (g_ddraw->backbuffer)
        {
            DDSURFACEDESC desc;
            memset(&desc, 0, sizeof(DDSURFACEDESC));

            desc.ddsCaps.dwCaps |= DDSCAPS_BACKBUFFER;

            desc.dwWidth = dst_surface->width;
            desc.dwHeight = dst_surface->height;

            dd_CreateSurface(&desc, &dst_surface->backbuffer, unkOuter);
        }
    }

    dprintf("     surface = %p (%dx%d@%d)\n", dst_surface, (int)dst_surface->width, (int)dst_surface->height, (int)dst_surface->bpp);

    *lpDDSurface = (LPDIRECTDRAWSURFACE)dst_surface;

    dst_surface->ref = 0;
    IDirectDrawSurface_AddRef(dst_surface);
    
    return DD_OK;
}
