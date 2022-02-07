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


HRESULT dds_AddAttachedSurface(IDirectDrawSurfaceImpl* This, IDirectDrawSurfaceImpl* lpDDSurface)
{
    if (lpDDSurface)
    {
        IDirectDrawSurface_AddRef(lpDDSurface);

        if (!This->backbuffer)
        {
            lpDDSurface->caps |= DDSCAPS_BACKBUFFER;
            This->backbuffer = lpDDSurface;
        }
    }

    return DD_OK;
}

HRESULT dds_Blt(
    IDirectDrawSurfaceImpl* This, 
    LPRECT lpDestRect, 
    IDirectDrawSurfaceImpl* lpDDSrcSurface, 
    LPRECT lpSrcRect, 
    DWORD dwFlags, 
    LPDDBLTFX lpDDBltFx)
{
    dbg_dump_dds_blt_flags(dwFlags);
    dbg_dump_dds_blt_fx_flags((dwFlags & DDBLT_DDFX) && lpDDBltFx ? lpDDBltFx->dwDDFX : 0);

    if (g_ddraw->iskkndx &&
        (dwFlags & DDBLT_COLORFILL) &&
        lpDestRect &&
        lpDestRect->right == 640 &&
        lpDestRect->bottom == 480)
    {
        if (This->backbuffer)
        {
            dds_Blt(This->backbuffer, lpDestRect, NULL, NULL, dwFlags, lpDDBltFx);
        }

        lpDestRect = NULL;
    }

    IDirectDrawSurfaceImpl* src_surface = lpDDSrcSurface;

    RECT src_rect = { 0, 0, src_surface ? src_surface->width : 0, src_surface ? src_surface->height : 0 };
    RECT dst_rect = { 0, 0, This->width, This->height };

    if (lpSrcRect && src_surface)
        memcpy(&src_rect, lpSrcRect, sizeof(src_rect));

    if (lpDestRect)
        memcpy(&dst_rect, lpDestRect, sizeof(dst_rect));

    /* stretch or clip? */
    BOOL is_stretch_blt =
        ((src_rect.right - src_rect.left) != (dst_rect.right - dst_rect.left)) ||
        ((src_rect.bottom - src_rect.top) != (dst_rect.bottom - dst_rect.top));

    if (src_surface)
    {
        if (src_rect.left < 0)
            src_rect.left = 0;

        if (src_rect.top < 0)
            src_rect.top = 0;

        if (src_rect.right > src_surface->width)
            src_rect.right = src_surface->width;

        if (src_rect.left > src_rect.right)
            src_rect.left = src_rect.right;

        if (src_rect.bottom > src_surface->height)
            src_rect.bottom = src_surface->height;

        if (src_rect.top > src_rect.bottom)
            src_rect.top = src_rect.bottom;
    }

    if (dst_rect.left < 0)
        dst_rect.left = 0;

    if (dst_rect.top < 0)
        dst_rect.top = 0;

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
        unsigned char* dst = (unsigned char*)dst_buf + (dst_x * This->lx_pitch) + (This->l_pitch * dst_y);
        unsigned char* first_row = dst;
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
            unsigned short* row1 = (unsigned short*)dst;
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
        else if (This->bpp == 32)
        {
            unsigned int* row1 = (unsigned int*)dst;
            unsigned int color = lpDDBltFx->dwFillColor;

            if ((color & 0xFF) == ((color >> 8) & 0xFF) &&
                (color & 0xFF) == ((color >> 16) & 0xFF) &&
                (color & 0xFF) == ((color >> 24) & 0xFF))
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
        BOOL got_fx = (dwFlags & DDBLT_DDFX) && lpDDBltFx;
        BOOL mirror_left_right = got_fx && (lpDDBltFx->dwDDFX & DDBLTFX_MIRRORLEFTRIGHT);
        BOOL mirror_up_down = got_fx && (lpDDBltFx->dwDDFX & DDBLTFX_MIRRORUPDOWN);

        if (This->bpp != src_surface->bpp)
        {
            TRACE_EXT("     NOT_IMPLEMENTED This->bpp=%u, src_surface->bpp=%u\n", This->bpp, src_surface->bpp);

            HDC dst_dc;
            dds_GetDC(This, &dst_dc);

            HDC src_dc;
            dds_GetDC(src_surface, &src_dc);

            StretchBlt(dst_dc, dst_x, dst_y, dst_w, dst_h, src_dc, src_x, src_y, src_w, src_h, SRCCOPY);
        }
        else if (
            (dwFlags & DDBLT_KEYSRC) || 
            (dwFlags & DDBLT_KEYSRCOVERRIDE) || 
            mirror_left_right ||
            mirror_up_down)
        {
            DDCOLORKEY color_key = { 0xFFFFFFFF, 0 };

            if ((dwFlags & DDBLT_KEYSRC) || (dwFlags & DDBLT_KEYSRCOVERRIDE))
            {
                color_key.dwColorSpaceLowValue =
                    (dwFlags & DDBLT_KEYSRCOVERRIDE) ?
                    lpDDBltFx->ddckSrcColorkey.dwColorSpaceLowValue : src_surface->color_key.dwColorSpaceLowValue;

                color_key.dwColorSpaceHighValue =
                    (dwFlags & DDBLT_KEYSRCOVERRIDE) ?
                    lpDDBltFx->ddckSrcColorkey.dwColorSpaceHighValue : src_surface->color_key.dwColorSpaceHighValue;
            }

            float scale_w = (float)src_w / dst_w;
            float scale_h = (float)src_h / dst_h;

            if (This->bpp == 8)
            {
                unsigned char key_low = (unsigned char)color_key.dwColorSpaceLowValue;
                unsigned char key_high = (unsigned char)color_key.dwColorSpaceHighValue;

                for (int y = 0; y < dst_h; y++)
                {
                    int scaled_y = (int)(y * scale_h);

                    if (mirror_up_down)
                        scaled_y = src_h - 1 - scaled_y;

                    int src_row = src_surface->width * (scaled_y + src_y);
                    int dst_row = This->width * (y + dst_y);

                    for (int x = 0; x < dst_w; x++)
                    {
                        int scaled_x = (int)(x * scale_w);

                        if (mirror_left_right)
                            scaled_x = src_w - 1 - scaled_x;

                        unsigned char c = ((unsigned char*)src_buf)[scaled_x + src_x + src_row];

                        if (c < key_low || c > key_high)
                        {
                            ((unsigned char*)dst_buf)[x + dst_x + dst_row] = c;
                        }
                    }
                }
            }
            else if (This->bpp == 16)
            {
                unsigned short key_low = (unsigned short)color_key.dwColorSpaceLowValue;
                unsigned short key_high = (unsigned short)color_key.dwColorSpaceHighValue;

                for (int y = 0; y < dst_h; y++)
                {
                    int scaled_y = (int)(y * scale_h);

                    if (mirror_up_down)
                        scaled_y = src_h - 1 - scaled_y;

                    int src_row = src_surface->width * (scaled_y + src_y);
                    int dst_row = This->width * (y + dst_y);

                    for (int x = 0; x < dst_w; x++)
                    {
                        int scaled_x = (int)(x * scale_w);

                        if (mirror_left_right)
                            scaled_x = src_w - 1 - scaled_x;

                        unsigned short c = ((unsigned short*)src_buf)[scaled_x + src_x + src_row];

                        if (c < key_low || c > key_high)
                        {
                            ((unsigned short*)dst_buf)[x + dst_x + dst_row] = c;
                        }
                    }
                }
            }
            else if (This->bpp == 32)
            {
                unsigned int key_low = color_key.dwColorSpaceLowValue;
                unsigned int key_high = color_key.dwColorSpaceHighValue;

                for (int y = 0; y < dst_h; y++)
                {
                    int scaled_y = (int)(y * scale_h);

                    if (mirror_up_down)
                        scaled_y = src_h - 1 - scaled_y;

                    int src_row = src_surface->width * (scaled_y + src_y);
                    int dst_row = This->width * (y + dst_y);

                    for (int x = 0; x < dst_w; x++)
                    {
                        int scaled_x = (int)(x * scale_w);

                        if (mirror_left_right)
                            scaled_x = src_w - 1 - scaled_x;

                        unsigned int c = ((unsigned int*)src_buf)[scaled_x + src_x + src_row];

                        if (c < key_low || c > key_high)
                        {
                            ((unsigned int*)dst_buf)[x + dst_x + dst_row] = c;
                        }
                    }
                }
            }
        }
        else
        {
            if (!is_stretch_blt)
            {
                int width = dst_w > src_w ? src_w : dst_w;
                int height = dst_h > src_h ? src_h : dst_h;

                unsigned char* src =
                    (unsigned char*)src_buf + (src_x * src_surface->lx_pitch) + (src_surface->l_pitch * src_y);

                unsigned char* dst =
                    (unsigned char*)dst_buf + (dst_x * This->lx_pitch) + (This->l_pitch * dst_y);

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

                scale_pattern* pattern = malloc((dst_w + 1) * (sizeof(scale_pattern)));
                int pattern_idx = 0;
                unsigned int last_src_x = 0;

                if (pattern != NULL)
                {
                    pattern[pattern_idx] = (scale_pattern){ ONCE, 0, 0, 1 };

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
                                pattern[pattern_idx] = (scale_pattern){ REPEAT, x, s_src_x, 1 };
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
                                pattern[pattern_idx] = (scale_pattern){ ONCE, x, s_src_x, 1 };
                            }
                        }
                        else
                        {
                            pattern_idx++;
                            pattern[pattern_idx] = (scale_pattern){ ONCE, x, s_src_x, 1 };
                        }
                        last_src_x = s_src_x;
                    }
                    pattern[pattern_idx + 1] = (scale_pattern){ END, 0, 0, 0 };


                    /* Do the actual blitting */
                    int count = 0;
                    int y;

                    for (y = 0; y < dst_h; y++) {

                        dest_base = dst_x + This->width * (y + dst_y);

                        s_src_y = (y * y_ratio) >> 16;

                        source_base = src_x + src_surface->width * (s_src_y + src_y);

                        pattern_idx = 0;
                        scale_pattern* current = &pattern[pattern_idx];

                        if (This->bpp == 8)
                        {
                            unsigned char* d, * s, v;
                            unsigned char* src = (unsigned char*)src_buf;
                            unsigned char* dst = (unsigned char*)dst_buf;

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

                                    memcpy((void*)d, (void*)s, current->count * This->lx_pitch);
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
                            unsigned short* d, * s, v;
                            unsigned short* src = (unsigned short*)src_buf;
                            unsigned short* dst = (unsigned short*)dst_buf;

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

                                    memcpy((void*)d, (void*)s, current->count * This->lx_pitch);
                                    break;

                                case END:
                                default:
                                    break;
                                }

                                current = &pattern[++pattern_idx];
                            } while (current->type != END);
                        }
                        else if (This->bpp == 32)
                        {
                            unsigned int* d, * s, v;
                            unsigned int* src = (unsigned int*)src_buf;
                            unsigned int* dst = (unsigned int*)dst_buf;

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

                                    memcpy((void*)d, (void*)s, current->count * This->lx_pitch);
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

    if ((This->caps & DDSCAPS_PRIMARYSURFACE) && g_ddraw->render.run)
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

HRESULT dds_BltFast(
    IDirectDrawSurfaceImpl* This, 
    DWORD dwX, 
    DWORD dwY, 
    IDirectDrawSurfaceImpl* lpDDSrcSurface, 
    LPRECT lpSrcRect, 
    DWORD dwFlags)
{
    dbg_dump_dds_blt_fast_flags(dwFlags);

    IDirectDrawSurfaceImpl* src_surface = lpDDSrcSurface;

    RECT src_rect = { 0, 0, src_surface ? src_surface->width : 0, src_surface ? src_surface->height : 0 };

    if (lpSrcRect && src_surface)
    {
        memcpy(&src_rect, lpSrcRect, sizeof(src_rect));

        if (src_rect.left < 0)
            src_rect.left = 0;

        if (src_rect.top < 0)
            src_rect.top = 0;

        if (src_rect.right > src_surface->width)
            src_rect.right = src_surface->width;

        if (src_rect.left > src_rect.right)
            src_rect.left = src_rect.right;

        if (src_rect.bottom > src_surface->height)
            src_rect.bottom = src_surface->height;

        if (src_rect.top > src_rect.bottom)
            src_rect.top = src_rect.bottom;
    }

    int dst_x = dwX;
    int dst_y = dwY;

    if (dst_x < 0)
    {
        src_rect.left += abs(dst_x);

        if (src_rect.left > src_rect.right)
            src_rect.left = src_rect.right;

        dst_x = 0;
    }

    if (dst_y < 0)
    {
        src_rect.top += abs(dst_y);

        if (src_rect.top > src_rect.bottom)
            src_rect.top = src_rect.bottom;

        dst_y = 0;
    }

    int src_x = src_rect.left;
    int src_y = src_rect.top;

    RECT dst_rect = { dst_x, dst_y, (src_rect.right - src_rect.left) + dst_x, (src_rect.bottom - src_rect.top) + dst_y };

    if (dst_rect.left < 0)
        dst_rect.left = 0;

    if (dst_rect.top < 0)
        dst_rect.top = 0;

    if (dst_rect.right > This->width)
        dst_rect.right = This->width;

    if (dst_rect.left > dst_rect.right)
        dst_rect.left = dst_rect.right;

    if (dst_rect.bottom > This->height)
        dst_rect.bottom = This->height;

    if (dst_rect.top > dst_rect.bottom)
        dst_rect.top = dst_rect.bottom;

    dst_x = dst_rect.left;
    dst_y = dst_rect.top;
    int dst_w = dst_rect.right - dst_rect.left;
    int dst_h = dst_rect.bottom - dst_rect.top;

    void* dst_buf = dds_GetBuffer(This);
    void* src_buf = dds_GetBuffer(src_surface);

    if (src_surface && dst_w > 0 && dst_h > 0)
    {
        if (This->bpp != src_surface->bpp)
        {
            TRACE_EXT("     NOT_IMPLEMENTED This->bpp=%u, src_surface->bpp=%u\n", This->bpp, src_surface->bpp);

            HDC dst_dc;
            dds_GetDC(This, &dst_dc);

            HDC src_dc;
            dds_GetDC(src_surface, &src_dc);

            BitBlt(dst_dc, dst_x, dst_y, dst_w, dst_h, src_dc, src_x, src_y, SRCCOPY);
        }
        else if (dwFlags & DDBLTFAST_SRCCOLORKEY)
        {
            if (This->bpp == 8)
            {
                unsigned char key_low = (unsigned char)src_surface->color_key.dwColorSpaceLowValue;
                unsigned char key_high = (unsigned char)src_surface->color_key.dwColorSpaceHighValue;

                for (int y = 0; y < dst_h; y++)
                {
                    int dst_row = This->width * (y + dst_y);
                    int src_row = src_surface->width * (y + src_y);

                    for (int x = 0; x < dst_w; x++)
                    {
                        unsigned char c = ((unsigned char*)src_buf)[x + src_x + src_row];

                        if (c < key_low || c > key_high)
                        {
                            ((unsigned char*)dst_buf)[x + dst_x + dst_row] = c;
                        }
                    }
                }
            }
            else if (This->bpp == 16)
            {
                unsigned short key_low = (unsigned short)src_surface->color_key.dwColorSpaceLowValue;
                unsigned short key_high = (unsigned short)src_surface->color_key.dwColorSpaceHighValue;

                for (int y = 0; y < dst_h; y++)
                {
                    int dst_row = This->width * (y + dst_y);
                    int src_row = src_surface->width * (y + src_y);

                    for (int x = 0; x < dst_w; x++)
                    {
                        unsigned short c = ((unsigned short*)src_buf)[x + src_x + src_row];

                        if (c < key_low || c > key_high)
                        {
                            ((unsigned short*)dst_buf)[x + dst_x + dst_row] = c;
                        }
                    }
                }
            }
            else if (This->bpp == 32)
            {
                unsigned int key_low = src_surface->color_key.dwColorSpaceLowValue;
                unsigned int key_high = src_surface->color_key.dwColorSpaceHighValue;

                for (int y = 0; y < dst_h; y++)
                {
                    int dst_row = This->width * (y + dst_y);
                    int src_row = src_surface->width * (y + src_y);

                    for (int x = 0; x < dst_w; x++)
                    {
                        unsigned int c = ((unsigned int*)src_buf)[x + src_x + src_row];

                        if (c < key_low || c > key_high)
                        {
                            ((unsigned int*)dst_buf)[x + dst_x + dst_row] = c;
                        }
                    }
                }
            }
        }
        else
        {
            unsigned char* src =
                (unsigned char*)src_buf + (src_x * src_surface->lx_pitch) + (src_surface->l_pitch * src_y);

            unsigned char* dst =
                (unsigned char*)dst_buf + (dst_x * This->lx_pitch) + (This->l_pitch * dst_y);

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

HRESULT dds_DeleteAttachedSurface(IDirectDrawSurfaceImpl* This, DWORD dwFlags, IDirectDrawSurfaceImpl* lpDDSurface)
{
    if (lpDDSurface)
    {
        IDirectDrawSurface_Release(lpDDSurface);

        if (lpDDSurface == This->backbuffer)
            This->backbuffer = NULL;
    }

    return DD_OK;
}

HRESULT dds_GetSurfaceDesc(IDirectDrawSurfaceImpl* This, LPDDSURFACEDESC lpDDSurfaceDesc)
{
    if (lpDDSurfaceDesc)
    {
        int size = lpDDSurfaceDesc->dwSize == sizeof(DDSURFACEDESC2) ? sizeof(DDSURFACEDESC2) : sizeof(DDSURFACEDESC);

        memset(lpDDSurfaceDesc, 0, size);

        lpDDSurfaceDesc->dwSize = size;
        lpDDSurfaceDesc->dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_PITCH | DDSD_PIXELFORMAT | DDSD_LPSURFACE;
        lpDDSurfaceDesc->dwWidth = This->width;
        lpDDSurfaceDesc->dwHeight = This->height;
        lpDDSurfaceDesc->lPitch = This->l_pitch;
        lpDDSurfaceDesc->lpSurface = dds_GetBuffer(This);
        lpDDSurfaceDesc->ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
        lpDDSurfaceDesc->ddpfPixelFormat.dwFlags = DDPF_RGB;
        lpDDSurfaceDesc->ddpfPixelFormat.dwRGBBitCount = This->bpp;
        lpDDSurfaceDesc->ddsCaps.dwCaps = This->caps;

        if (!g_ddraw->novidmem || (This->caps & (DDSCAPS_PRIMARYSURFACE | DDSCAPS_BACKBUFFER)))
        {
            lpDDSurfaceDesc->ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY;
        }

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
        else if (This->bpp == 32)
        {
            lpDDSurfaceDesc->ddpfPixelFormat.dwRBitMask = 0xFF0000;
            lpDDSurfaceDesc->ddpfPixelFormat.dwGBitMask = 0x00FF00;
            lpDDSurfaceDesc->ddpfPixelFormat.dwBBitMask = 0x0000FF;
        }
    }

    return DD_OK;
}

HRESULT dds_EnumAttachedSurfaces(
    IDirectDrawSurfaceImpl* This, 
    LPVOID lpContext, 
    LPDDENUMSURFACESCALLBACK lpEnumSurfacesCallback)
{
    static DDSURFACEDESC2 desc;

    memset(&desc, 0, sizeof(desc));

    if (This->backbuffer)
    {
        dds_GetSurfaceDesc(This->backbuffer, (LPDDSURFACEDESC)&desc);
        IDirectDrawSurface_AddRef(This->backbuffer);
        lpEnumSurfacesCallback((LPDIRECTDRAWSURFACE)This->backbuffer, (LPDDSURFACEDESC)&desc, lpContext);
    }

    return DD_OK;
}

HRESULT dds_Flip(IDirectDrawSurfaceImpl* This, IDirectDrawSurfaceImpl* lpDDSurfaceTargetOverride, DWORD dwFlags)
{
    if (This->backbuffer)
    {
        EnterCriticalSection(&g_ddraw->cs);
        IDirectDrawSurfaceImpl* backbuffer = lpDDSurfaceTargetOverride ? lpDDSurfaceTargetOverride : This->backbuffer;

        void* buf = InterlockedExchangePointer(&This->surface, backbuffer->surface);
        HBITMAP bitmap = (HBITMAP)InterlockedExchangePointer(&This->bitmap, backbuffer->bitmap);
        HDC dc = (HDC)InterlockedExchangePointer(&This->hdc, backbuffer->hdc);

        InterlockedExchangePointer(&backbuffer->surface, buf);
        InterlockedExchangePointer(&backbuffer->bitmap, bitmap);
        InterlockedExchangePointer(&backbuffer->hdc, dc);
        LeaveCriticalSection(&g_ddraw->cs);

        if (!lpDDSurfaceTargetOverride && This->backbuffer->backbuffer)
        {
            dds_Flip(This->backbuffer, NULL, 0);
        }
    }

    if (This->caps & DDSCAPS_PRIMARYSURFACE && g_ddraw->render.run)
    {
        This->last_flip_tick = timeGetTime();

        InterlockedExchange(&g_ddraw->render.surface_updated, TRUE);
        ReleaseSemaphore(g_ddraw->render.sem, 1, NULL);
        SwitchToThread();

        if ((dwFlags & DDFLIP_WAIT) || g_ddraw->maxgameticks == -2)
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

HRESULT dds_GetAttachedSurface(IDirectDrawSurfaceImpl* This, LPDDSCAPS lpDdsCaps, IDirectDrawSurfaceImpl** lpDDsurface)
{
    if ((This->caps & DDSCAPS_PRIMARYSURFACE) && (This->caps & DDSCAPS_FLIP) && (lpDdsCaps->dwCaps & DDSCAPS_BACKBUFFER))
    {
        if (This->backbuffer)
        {
            IDirectDrawSurface_AddRef(This->backbuffer);
            *lpDDsurface = This->backbuffer;
        }
        else
        {
            IDirectDrawSurface_AddRef(This);
            *lpDDsurface = This;
        }
    }

    return DD_OK;
}

HRESULT dds_GetCaps(IDirectDrawSurfaceImpl* This, LPDDSCAPS lpDDSCaps)
{
    lpDDSCaps->dwCaps = This->caps;
    return DD_OK;
}

HRESULT dds_GetClipper(IDirectDrawSurfaceImpl* This, IDirectDrawClipperImpl** lpClipper)
{
    if (!lpClipper)
        return DDERR_INVALIDPARAMS;

    *lpClipper = This->clipper;

    if (This->clipper)
    {
        IDirectDrawClipper_AddRef(This->clipper);
        return DD_OK;
    }
    else
    {
        return DDERR_NOCLIPPERATTACHED;
    }
}

HRESULT dds_GetColorKey(IDirectDrawSurfaceImpl* This, DWORD dwFlags, LPDDCOLORKEY lpColorKey)
{
    if (lpColorKey)
    {
        lpColorKey->dwColorSpaceHighValue = This->color_key.dwColorSpaceHighValue;
        lpColorKey->dwColorSpaceLowValue = This->color_key.dwColorSpaceLowValue;
    }

    return DD_OK;
}

HRESULT dds_GetDC(IDirectDrawSurfaceImpl* This, HDC FAR* lpHDC)
{
    if (!This)
    {
        if (lpHDC)
            *lpHDC = NULL;

        return DDERR_INVALIDPARAMS;
    }

    if ((This->l_pitch % 4))
    {
        TRACE("NOT_IMPLEMENTED     GetDC: width=%d height=%d\n", This->width, This->height);
    }

    RGBQUAD* data =
        This->palette ? This->palette->data_rgb :
        g_ddraw->primary && g_ddraw->primary->palette ? g_ddraw->primary->palette->data_rgb :
        NULL;

    HDC dc = This->hdc;

    if (This->backbuffer || (This->caps & DDSCAPS_BACKBUFFER))
        dc = (HDC)InterlockedExchangeAdd((LONG*)&This->hdc, 0);

    if (This->bpp == 8 && data)
        SetDIBColorTable(dc, 0, 256, data);

    if (lpHDC)
        *lpHDC = dc;

    return DD_OK;
}

HRESULT dds_GetPalette(IDirectDrawSurfaceImpl* This, IDirectDrawPaletteImpl** lplpDDPalette)
{
    if (!lplpDDPalette)
        return DDERR_INVALIDPARAMS;

    *lplpDDPalette = This->palette;

    if (This->palette)
    {
        IDirectDrawPalette_AddRef(This->palette);
        return DD_OK;
    }
    else
    {
        return DDERR_NOPALETTEATTACHED;
    }
}

HRESULT dds_GetPixelFormat(IDirectDrawSurfaceImpl* This, LPDDPIXELFORMAT ddpfPixelFormat)
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
        else if (This->bpp == 32)
        {
            ddpfPixelFormat->dwRBitMask = 0xFF0000;
            ddpfPixelFormat->dwGBitMask = 0x00FF00;
            ddpfPixelFormat->dwBBitMask = 0x0000FF;
        }

        return DD_OK;
    }

    return DDERR_INVALIDPARAMS;
}

HRESULT dds_Lock(
    IDirectDrawSurfaceImpl* This, 
    LPRECT lpDestRect, 
    LPDDSURFACEDESC lpDDSurfaceDesc, 
    DWORD dwFlags, 
    HANDLE hEvent)
{
    dbg_dump_dds_lock_flags(dwFlags);

    if (g_ddraw && g_ddraw->fixnotresponding)
    {
        MSG msg; /* workaround for "Not Responding" window problem */
        PeekMessage(&msg, g_ddraw->hwnd, 0, 0, PM_NOREMOVE);
    }

    HRESULT ret = dds_GetSurfaceDesc(This, lpDDSurfaceDesc);

    if (lpDestRect && lpDDSurfaceDesc)
    {
        if (lpDestRect->left < 0 ||
            lpDestRect->top < 0 ||
            lpDestRect->left > lpDestRect->right ||
            lpDestRect->top > lpDestRect->bottom ||
            lpDestRect->right > This->width ||
            lpDestRect->bottom > This->height)
        {
            return DDERR_INVALIDPARAMS;
        }

        lpDDSurfaceDesc->lpSurface =
            (char*)dds_GetBuffer(This) + (lpDestRect->left * This->lx_pitch) + (lpDestRect->top * This->l_pitch);
    }

    return ret;
}

HRESULT dds_ReleaseDC(IDirectDrawSurfaceImpl* This, HDC hDC)
{
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

HRESULT dds_SetClipper(IDirectDrawSurfaceImpl* This, IDirectDrawClipperImpl* lpClipper)
{
    if (lpClipper)
        IDirectDrawClipper_AddRef(lpClipper);

    if (This->clipper)
        IDirectDrawClipper_Release(This->clipper);

    This->clipper = lpClipper;

    return DD_OK;
}

HRESULT dds_SetColorKey(IDirectDrawSurfaceImpl* This, DWORD dwFlags, LPDDCOLORKEY lpColorKey)
{
    if (lpColorKey)
    {
        TRACE_EXT("     dwColorSpaceHighValue=%d\n", lpColorKey->dwColorSpaceHighValue);
        TRACE_EXT("     dwColorSpaceLowValue=%d\n", lpColorKey->dwColorSpaceLowValue);

        This->color_key.dwColorSpaceHighValue = lpColorKey->dwColorSpaceHighValue;
        This->color_key.dwColorSpaceLowValue = lpColorKey->dwColorSpaceLowValue;
    }

    return DD_OK;
}

HRESULT dds_SetPalette(IDirectDrawSurfaceImpl* This, IDirectDrawPaletteImpl* lpDDPalette)
{
    if (lpDDPalette)
        IDirectDrawPalette_AddRef(lpDDPalette);

    if (This->palette)
        IDirectDrawPalette_Release(This->palette);

    if (This->caps & DDSCAPS_PRIMARYSURFACE)
    {
        EnterCriticalSection(&g_ddraw->cs);
        This->palette = lpDDPalette;
        LeaveCriticalSection(&g_ddraw->cs);

        if (g_ddraw->render.run)
        {
            InterlockedExchange(&g_ddraw->render.palette_updated, TRUE);
            ReleaseSemaphore(g_ddraw->render.sem, 1, NULL);
        }
    }
    else
    {
        This->palette = lpDDPalette;
    }

    return DD_OK;
}

HRESULT dds_Unlock(IDirectDrawSurfaceImpl* This, LPRECT lpRect)
{
    /* Hack for Warcraft II BNE and Diablo */
    HWND hwnd = g_ddraw->bnet_active ? FindWindowEx(HWND_DESKTOP, NULL, "SDlgDialog", NULL) : NULL;

    if (hwnd && (This->caps & DDSCAPS_PRIMARYSURFACE))
    {
        HDC primary_dc;
        dds_GetDC(This, &primary_dc);

        /* GdiTransparentBlt idea taken from Aqrit's war2 ddraw */

        RGBQUAD quad;
        GetDIBColorTable(primary_dc, 0xFE, 1, &quad);
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
                    primary_dc,
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

    /* Hack for Star Trek Armada */
    hwnd = g_ddraw->armadahack ? FindWindowEx(HWND_DESKTOP, NULL, "#32770", NULL) : NULL;

    if (hwnd && (This->caps & DDSCAPS_PRIMARYSURFACE))
    {
        HDC primary_dc;
        dds_GetDC(This, &primary_dc);

        RECT rc;
        if (fake_GetWindowRect(hwnd, &rc))
        {
            HDC hdc = GetDC(hwnd);

            GdiTransparentBlt(
                hdc,
                0,
                0,
                rc.right - rc.left,
                rc.bottom - rc.top,
                primary_dc,
                rc.left,
                rc.top,
                rc.right - rc.left,
                rc.bottom - rc.top,
                0
            );

            ReleaseDC(hwnd, hdc);
        }

        BOOL x = g_ddraw->ticks_limiter.use_blt_or_flip;

        DDBLTFX fx = { .dwFillColor = 0 };
        IDirectDrawSurface_Blt(This, NULL, NULL, NULL, DDBLT_COLORFILL, &fx);

        g_ddraw->ticks_limiter.use_blt_or_flip = x;
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

HRESULT dds_GetDDInterface(IDirectDrawSurfaceImpl* This, LPVOID* lplpDD)
{
    if (!lplpDD)
        return DDERR_INVALIDPARAMS;

    *lplpDD = This->ddraw;
    IDirectDraw_AddRef(This->ddraw);

    return DD_OK;
}

void* dds_GetBuffer(IDirectDrawSurfaceImpl* This)
{
    if (!This)
        return NULL;

    if (This->backbuffer || (This->caps & DDSCAPS_BACKBUFFER))
        return (void*)InterlockedExchangeAdd((LONG*)&This->surface, 0);

    return This->surface;
}

HRESULT dd_CreateSurface(
    IDirectDrawImpl* This, 
    LPDDSURFACEDESC lpDDSurfaceDesc, 
    IDirectDrawSurfaceImpl** lpDDSurface, 
    IUnknown FAR* unkOuter)
{
    dbg_dump_dds_flags(lpDDSurfaceDesc->dwFlags);
    dbg_dump_dds_caps(lpDDSurfaceDesc->ddsCaps.dwCaps);

    if ((lpDDSurfaceDesc->ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE) &&
        g_ddraw->primary &&
        g_ddraw->primary->width == g_ddraw->width &&
        g_ddraw->primary->height == g_ddraw->height &&
        g_ddraw->primary->bpp == g_ddraw->bpp)
    {
        *lpDDSurface = g_ddraw->primary;
        IDirectDrawSurface_AddRef(g_ddraw->primary);

        return DD_OK;
    }

    IDirectDrawSurfaceImpl* dst_surface = 
        (IDirectDrawSurfaceImpl*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(IDirectDrawSurfaceImpl));

    dst_surface->lpVtbl = &g_dds_vtbl;

    lpDDSurfaceDesc->dwFlags |= DDSD_CAPS;

    dst_surface->bpp = g_ddraw->bpp == 0 ? 16 : g_ddraw->bpp;
    dst_surface->flags = lpDDSurfaceDesc->dwFlags;
    dst_surface->caps = lpDDSurfaceDesc->ddsCaps.dwCaps;
    dst_surface->ddraw = This;

    if (dst_surface->flags & DDSD_PIXELFORMAT)
    {
        switch (lpDDSurfaceDesc->ddpfPixelFormat.dwRGBBitCount)
        {
        case 8:
            dst_surface->bpp = 8;
            break;
        case 15:
            TRACE("     NOT_IMPLEMENTED bpp=%u\n", lpDDSurfaceDesc->ddpfPixelFormat.dwRGBBitCount);
        case 16:
            dst_surface->bpp = 16;
            break;
        case 24:
            TRACE("     NOT_IMPLEMENTED bpp=%u\n", lpDDSurfaceDesc->ddpfPixelFormat.dwRGBBitCount);
        case 32:
            dst_surface->bpp = 32;
            break;
        default:
            TRACE("     NOT_IMPLEMENTED bpp=%u\n", lpDDSurfaceDesc->ddpfPixelFormat.dwRGBBitCount);
            break;
        }
    }

    if (dst_surface->caps & DDSCAPS_PRIMARYSURFACE)
    {
        dst_surface->width = g_ddraw->width;
        dst_surface->height = g_ddraw->height;
    }
    else
    {
        dst_surface->width = lpDDSurfaceDesc->dwWidth;
        dst_surface->height = lpDDSurfaceDesc->dwHeight;
    }

    if (dst_surface->width && dst_surface->height)
    {
        if (dst_surface->width == 71 && dst_surface->height == 24) dst_surface->width = 72; //Commandos

        dst_surface->lx_pitch = dst_surface->bpp / 8;
        dst_surface->l_pitch = dst_surface->width * dst_surface->lx_pitch;

        if (g_ddraw->fixpitch && !(dst_surface->caps & (DDSCAPS_PRIMARYSURFACE | DDSCAPS_BACKBUFFER)))
        {
            while (dst_surface->l_pitch % 4)
            {
                dst_surface->l_pitch = ++dst_surface->width * dst_surface->lx_pitch;
            }
        }

        dst_surface->bmi = 
            HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * 256);

        dst_surface->bmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        dst_surface->bmi->bmiHeader.biWidth = dst_surface->width;
        dst_surface->bmi->bmiHeader.biHeight = -((int)dst_surface->height + 200);
        dst_surface->bmi->bmiHeader.biPlanes = 1;
        dst_surface->bmi->bmiHeader.biBitCount = dst_surface->bpp;
        dst_surface->bmi->bmiHeader.biCompression = dst_surface->bpp == 8 ? BI_RGB : BI_BITFIELDS;

        WORD clr_bits = (WORD)(dst_surface->bmi->bmiHeader.biPlanes * dst_surface->bmi->bmiHeader.biBitCount);

        if (clr_bits < 24)
        {
            dst_surface->bmi->bmiHeader.biClrUsed = (1 << clr_bits);
        }

        dst_surface->bmi->bmiHeader.biSizeImage = 
            ((dst_surface->width * clr_bits + 31) & ~31) / 8 * dst_surface->height;

        if (dst_surface->bpp == 8)
        {
            for (int i = 0; i < 256; i++)
            {
                dst_surface->bmi->bmiColors[i].rgbRed = i;
                dst_surface->bmi->bmiColors[i].rgbGreen = i;
                dst_surface->bmi->bmiColors[i].rgbBlue = i;
                dst_surface->bmi->bmiColors[i].rgbReserved = 0;
            }
        }
        else if (dst_surface->bpp == 16)
        {
            ((DWORD*)dst_surface->bmi->bmiColors)[0] = 0xF800;
            ((DWORD*)dst_surface->bmi->bmiColors)[1] = 0x07E0;
            ((DWORD*)dst_surface->bmi->bmiColors)[2] = 0x001F;
        }
        else if (dst_surface->bpp == 32)
        {
            ((DWORD*)dst_surface->bmi->bmiColors)[0] = 0xFF0000;
            ((DWORD*)dst_surface->bmi->bmiColors)[1] = 0x00FF00;
            ((DWORD*)dst_surface->bmi->bmiColors)[2] = 0x0000FF;
        }

        dst_surface->hdc = CreateCompatibleDC(g_ddraw->render.hdc);
        dst_surface->bitmap = 
            CreateDIBSection(dst_surface->hdc, dst_surface->bmi, DIB_RGB_COLORS, (void**)&dst_surface->surface, NULL, 0);

        dst_surface->bmi->bmiHeader.biHeight = -((int)dst_surface->height);

        if (!dst_surface->bitmap)
        {
            dst_surface->surface =
                HeapAlloc(
                    GetProcessHeap(),
                    HEAP_ZERO_MEMORY,
                    dst_surface->l_pitch * (dst_surface->height + 200) * dst_surface->lx_pitch);
        }

        if (dst_surface->caps & DDSCAPS_PRIMARYSURFACE)
        {
            g_ddraw->primary = dst_surface;
            FakePrimarySurface = dst_surface->surface;
        }

        SelectObject(dst_surface->hdc, dst_surface->bitmap);
    }

    if (dst_surface->flags & DDSD_BACKBUFFERCOUNT)
    {
        TRACE("     dwBackBufferCount=%d\n", lpDDSurfaceDesc->dwBackBufferCount);

        DDSURFACEDESC desc;
        memset(&desc, 0, sizeof(desc));

        if (lpDDSurfaceDesc->dwBackBufferCount > 1)
        {
            desc.dwBackBufferCount = lpDDSurfaceDesc->dwBackBufferCount - 1;
            desc.dwFlags |= DDSD_BACKBUFFERCOUNT;
        }

        desc.ddsCaps.dwCaps |= DDSCAPS_BACKBUFFER;

        desc.dwWidth = dst_surface->width;
        desc.dwHeight = dst_surface->height;

        dd_CreateSurface(This, &desc, &dst_surface->backbuffer, unkOuter);
    }

    TRACE(
        "     surface = %p (%ux%u@%u)\n", 
        dst_surface, 
        dst_surface->width, 
        dst_surface->height, 
        dst_surface->bpp);

    *lpDDSurface = dst_surface;

    dst_surface->ref = 0;
    IDirectDrawSurface_AddRef(dst_surface);

    return DD_OK;
}
