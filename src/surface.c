/*
 * Copyright (c) 2010 Toni Spets <toni.spets@iki.fi>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <windows.h>
#include <stdio.h>

#include "main.h"
#include "surface.h"

void dump_ddbltflags(DWORD dwFlags);
void dump_ddscaps(DWORD dwCaps);
void dump_ddsd(DWORD dwFlags);
DWORD WINAPI render_soft_main(void);

HRESULT __stdcall ddraw_surface_QueryInterface(IDirectDrawSurfaceImpl *This, REFIID riid, void **obj)
{
    printf("DirectDrawSurface::QueryInterface(This=%p, riid=%08X, obj=%p) ???\n", This, (unsigned int)riid, obj);
    return S_OK;
}

ULONG __stdcall ddraw_surface_AddRef(IDirectDrawSurfaceImpl *This)
{
    printf("DirectDrawSurface::AddRef(This=%p)\n", This);
    This->Ref++;
    return This->Ref;
}

ULONG __stdcall ddraw_surface_Release(IDirectDrawSurfaceImpl *This)
{
    printf("DirectDrawSurface::Release(This=%p)\n", This);

    This->Ref--;

    if(This->Ref == 0)
    {
        if(This->caps == DDSCAPS_PRIMARYSURFACE)
        {
            EnterCriticalSection(&ddraw->cs);
            ddraw->primary = NULL;
            LeaveCriticalSection(&ddraw->cs);
        }

        if (This->bitmap)
            DeleteObject(This->bitmap);
        else if (This->surface)
            HeapFree(GetProcessHeap(), 0, This->surface);

        if (This->hDC)
            DeleteDC(This->hDC);

        if (This->bmi)
            HeapFree(GetProcessHeap(), 0, This->bmi);

        if(This->palette)
        {
            IDirectDrawPalette_Release(This->palette);
        }
        HeapFree(GetProcessHeap(), 0, This);
        return 0;
    }
    return This->Ref;
}

HRESULT __stdcall ddraw_surface_AddAttachedSurface(IDirectDrawSurfaceImpl *This, LPDIRECTDRAWSURFACE lpDDSurface)
{
    printf("DirectDrawSurface::AddAttachedSurface(This=%p, lpDDSurface=%p) ???\n", This, lpDDSurface);
    IDirectDrawSurface_AddRef(lpDDSurface);
    return DD_OK;
}

HRESULT __stdcall ddraw_surface_AddOverlayDirtyRect(IDirectDrawSurfaceImpl *This, LPRECT a)
{
    printf("DirectDrawSurface::AddOverlayDirtyRect(This=%p, ...) ???\n", This);
    return DD_OK;
}

HRESULT __stdcall ddraw_surface_Blt(IDirectDrawSurfaceImpl *This, LPRECT lpDestRect, LPDIRECTDRAWSURFACE lpDDSrcSurface, LPRECT lpSrcRect, DWORD dwFlags, LPDDBLTFX lpDDBltFx)
{
    IDirectDrawSurfaceImpl *Source = (IDirectDrawSurfaceImpl *)lpDDSrcSurface;

#if _DEBUG_X
    printf("DirectDrawSurface::Blt(This=%p, lpDestRect=%p, lpDDSrcSurface=%p, lpSrcRect=%p, dwFlags=%08X, lpDDBltFx=%p)\n", This, lpDestRect, lpDDSrcSurface, lpSrcRect, (int)dwFlags, lpDDBltFx);

    dump_ddbltflags(dwFlags);
    
    if(lpDestRect)
    {
        printf(" dest: l: %d t: %d r: %d b: %d\n", (int)lpDestRect->left, (int)lpDestRect->top, (int)lpDestRect->right, (int)lpDestRect->bottom);
    }
    if(lpSrcRect)
    {
        printf("  src: l: %d t: %d r: %d b: %d\n", (int)lpSrcRect->left, (int)lpSrcRect->top, (int)lpSrcRect->right, (int)lpSrcRect->bottom);
    }
#endif

    if (This->surface && (dwFlags & DDBLT_COLORFILL))
    {
        int dst_w = lpDestRect ? lpDestRect->right - lpDestRect->left : This->width;
        int dst_h = lpDestRect ? lpDestRect->bottom - lpDestRect->top : This->height;
        int dst_x = lpDestRect ? lpDestRect->left : 0;
        int dst_y = lpDestRect ? lpDestRect->top : 0;

        int y, x;
        for (y = 0; y < dst_h; y++)
        {
            int ydst = This->width * (y + dst_y);

            for (x = 0; x < dst_w; x++)
            {
                ((unsigned char *)This->surface)[x + dst_x + ydst] = lpDDBltFx->dwFillColor;
            }
        }
    }

    if (Source)
    {
        if (dwFlags & DDBLT_KEYSRC)
        {
            int src_w = lpSrcRect ? lpSrcRect->right - lpSrcRect->left : Source->width;
            int src_h = lpSrcRect ? lpSrcRect->bottom - lpSrcRect->top : Source->height;
            int src_x = lpSrcRect ? lpSrcRect->left : 0;
            int src_y = lpSrcRect ? lpSrcRect->top : 0;
            int dst_x = lpDestRect ? lpDestRect->left : 0;
            int dst_y = lpDestRect ? lpDestRect->top : 0;

            unsigned char* dstSurface = (unsigned char *)This->surface;
            unsigned char* srcSurface = (unsigned char *)Source->surface;

            int y1, x1;
            for (y1 = 0; y1 < src_h; y1++)
            {
                int ydst = This->width * (y1 + dst_y);
                int ysrc = Source->width * (y1 + src_y);

                for (x1 = 0; x1 < src_w; x1++)
                {
                    unsigned char index = srcSurface[x1 + src_x + ysrc];

                    if (index != Source->colorKey.dwColorSpaceLowValue)
                        dstSurface[x1 + dst_x + ydst] = index;
                }
            }
        }
        else
        {
            int dx = 0, dy = 0;
            if (lpDestRect)
            {
                dx = lpDestRect->left;
                dy = lpDestRect->top;
            }
            int x0 = 0, y0 = 0, x1 = Source->width, y1 = Source->height;
            if (lpSrcRect)
            {
                x0 = max(x0, lpSrcRect->left);
                x1 = min(x1, lpSrcRect->right);
                y0 = max(y0, lpSrcRect->top);
                y1 = min(y1, lpSrcRect->bottom);
            }
            unsigned char* to = (unsigned char *)This->surface + dy*This->width + dx;
            unsigned char* from = (unsigned char *)Source->surface + y0*Source->width + x0;
            int s = x1 - x0;

            int y;
            for (y = y0; y < y1; ++y, to += This->width, from += Source->width)
            {
                memcpy(to, from, s);
            }
        }
    }

    if(This->caps & DDSCAPS_PRIMARYSURFACE && !(This->flags & DDSD_BACKBUFFERCOUNT) && ddraw->render.run)
    {
        InterlockedExchange(&ddraw->render.surfaceUpdated, TRUE);
        ReleaseSemaphore(ddraw->render.sem, 1, NULL);
        if (ddraw->renderer == render_soft_main)
        {
            WaitForSingleObject(ddraw->render.ev, INFINITE);
            ResetEvent(ddraw->render.ev);
        }
        else
        {
            SwitchToThread();
        }

        if (ddraw->sleep > 0)
            Sleep(ddraw->sleep);
    }

    return DD_OK;
}

HRESULT __stdcall ddraw_surface_BltBatch(IDirectDrawSurfaceImpl *This, LPDDBLTBATCH a, DWORD b, DWORD c)
{
    printf("IDirectDrawSurface::BltBatch(This=%p, ...) ???\n", This);
    return DD_OK;
}

HRESULT __stdcall ddraw_surface_BltFast(IDirectDrawSurfaceImpl *This, DWORD dst_x, DWORD dst_y, LPDIRECTDRAWSURFACE lpDDSrcSurface, LPRECT lpSrcRect, DWORD flags)
{
    IDirectDrawSurfaceImpl *Source = (IDirectDrawSurfaceImpl *)lpDDSrcSurface;

#if _DEBUG_X
    printf("IDirectDrawSurface::BltFast(This=%p, x=%d, y=%d, lpDDSrcSurface=%p, lpSrcRect=%p, flags=%08X)\n", This, dst_x, dst_y, lpDDSrcSurface, lpSrcRect, flags);

    if (flags & DDBLTFAST_NOCOLORKEY)
    {
        printf("  DDBLTFAST_NOCOLORKEY\n");
    }

    if (flags & DDBLTFAST_SRCCOLORKEY)
    {
        printf("  DDBLTFAST_SRCCOLORKEY\n");
    }

    if (flags & DDBLTFAST_DESTCOLORKEY)
    {
        printf("  DDBLTFAST_DESTCOLORKEY\n");
    }
#endif

    if (Source)
    {
        if (flags & DDBLTFAST_SRCCOLORKEY)
        {
            int src_w = lpSrcRect ? lpSrcRect->right - lpSrcRect->left : Source->width;
            int src_h = lpSrcRect ? lpSrcRect->bottom - lpSrcRect->top : Source->height;
            int src_x = lpSrcRect ? lpSrcRect->left : 0;
            int src_y = lpSrcRect ? lpSrcRect->top : 0;

            unsigned char* dstSurface = (unsigned char *)This->surface;
            unsigned char* srcSurface = (unsigned char *)Source->surface;

            int y1, x1;
            for (y1 = 0; y1 < src_h; y1++)
            {
                int ydst = This->width * (y1 + dst_y);
                int ysrc = Source->width * (y1 + src_y);

                for (x1 = 0; x1 < src_w; x1++)
                {
                    unsigned char index = srcSurface[x1 + src_x + ysrc];

                    if (index != Source->colorKey.dwColorSpaceLowValue)
                        dstSurface[x1 + dst_x + ydst] = index;
                }
            }
        }
        else
        {
            int x0 = 0, y0 = 0, x1 = Source->width, y1 = Source->height;
            if (lpSrcRect)
            {
                x0 = max(x0, lpSrcRect->left);
                x1 = min(x1, lpSrcRect->right);
                y0 = max(y0, lpSrcRect->top);
                y1 = min(y1, lpSrcRect->bottom);
            }
            unsigned char* to = (unsigned char *)This->surface + dst_y*This->width + dst_x;
            unsigned char* from = (unsigned char *)Source->surface + y0*Source->width + x0;
            int s = x1 - x0;

            int y;
            for (y = y0; y < y1; ++y, to += This->width, from += Source->width)
            {
                memcpy(to, from, s);
            }
        }
    }

    if (This->caps & DDSCAPS_PRIMARYSURFACE && !(This->flags & DDSD_BACKBUFFERCOUNT) && ddraw->render.run)
    {
        InterlockedExchange(&ddraw->render.surfaceUpdated, TRUE);
        ReleaseSemaphore(ddraw->render.sem, 1, NULL);
    }

    return DD_OK;
}

HRESULT __stdcall ddraw_surface_DeleteAttachedSurface(IDirectDrawSurfaceImpl *This, DWORD dwFlags, LPDIRECTDRAWSURFACE lpDDSurface)
{
    printf("IDirectDrawSurface::DeleteAttachedSurface(This=%p, dwFlags=%08X, lpDDSurface=%p)\n", This, (int)dwFlags, lpDDSurface);
    IDirectDrawSurface_Release(lpDDSurface);
    return DD_OK;
}

HRESULT __stdcall ddraw_surface_GetSurfaceDesc(IDirectDrawSurfaceImpl *This, LPDDSURFACEDESC lpDDSurfaceDesc)
{
#if DEBUG_X
    printf("IDirectDrawSurface::GetSurfaceDesc(This=%p, lpDDSurfaceDesc=%p)\n", This, lpDDSurfaceDesc);
#endif

    lpDDSurfaceDesc->dwSize = sizeof(DDSURFACEDESC);
    lpDDSurfaceDesc->dwFlags = DDSD_WIDTH|DDSD_HEIGHT|DDSD_PITCH|DDSD_PIXELFORMAT|DDSD_LPSURFACE;
    lpDDSurfaceDesc->dwWidth = This->width;
    lpDDSurfaceDesc->dwHeight = This->height;
    lpDDSurfaceDesc->lPitch = This->lPitch;
    lpDDSurfaceDesc->lpSurface = This->surface;
    lpDDSurfaceDesc->ddpfPixelFormat.dwFlags = DDPF_RGB;
    lpDDSurfaceDesc->ddpfPixelFormat.dwRGBBitCount = This->bpp;

    if (This->bpp == 16)
    {
        /* RGB 555 */
        lpDDSurfaceDesc->ddpfPixelFormat.dwRBitMask = 0x7C00;
        lpDDSurfaceDesc->ddpfPixelFormat.dwGBitMask = 0x03E0;
        lpDDSurfaceDesc->ddpfPixelFormat.dwBBitMask = 0x001F;
    }

    return DD_OK;
}

HRESULT __stdcall ddraw_surface_EnumAttachedSurfaces(IDirectDrawSurfaceImpl *This, LPVOID lpContext, LPDDENUMSURFACESCALLBACK lpEnumSurfacesCallback)
{
    printf("IDirectDrawSurface::EnumAttachedSurfaces(This=%p, lpContext=%p, lpEnumSurfacesCallback=%p)\n", This, lpContext, lpEnumSurfacesCallback);

    /* this is not actually complete, but Carmageddon seems to call EnumAttachedSurfaces instead of GetSurfaceDesc to get the main surface description */
    static LPDDSURFACEDESC lpDDSurfaceDesc;
    lpDDSurfaceDesc = (LPDDSURFACEDESC)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(DDSURFACEDESC));
    ddraw_surface_GetSurfaceDesc(This, lpDDSurfaceDesc);
    lpEnumSurfacesCallback((LPDIRECTDRAWSURFACE)This, lpDDSurfaceDesc, lpContext);
    HeapFree(GetProcessHeap(), 0, lpDDSurfaceDesc);

    return DD_OK;
}

HRESULT __stdcall ddraw_surface_EnumOverlayZOrders(IDirectDrawSurfaceImpl *This, DWORD a, LPVOID b, LPDDENUMSURFACESCALLBACK c)
{
    printf("IDirectDrawSurface::EnumOverlayZOrders(This=%p, ...) ???\n", This);
    return DD_OK;
}

HRESULT __stdcall ddraw_surface_Flip(IDirectDrawSurfaceImpl *This, LPDIRECTDRAWSURFACE surface, DWORD flags)
{
#if _DEBUG_X
    printf("IDirectDrawSurface::Flip(This=%p, surface=%p, flags=%08X)\n", This, surface, flags);
#endif

    if(This->caps & DDSCAPS_PRIMARYSURFACE && ddraw->render.run)
    {
        InterlockedExchange(&ddraw->render.surfaceUpdated, TRUE);
        ReleaseSemaphore(ddraw->render.sem, 1, NULL);
        if (ddraw->renderer == render_soft_main)
        {
            ResetEvent(ddraw->render.ev);
            WaitForSingleObject(ddraw->render.ev, INFINITE);
        }
        else
        {
            SwitchToThread();
        }

        if (ddraw->sleep > 0)
            Sleep(ddraw->sleep);
    }

    return DD_OK;
}

HRESULT __stdcall ddraw_surface_GetAttachedSurface(IDirectDrawSurfaceImpl *This, LPDDSCAPS lpDdsCaps, LPDIRECTDRAWSURFACE FAR *surface)
{
    printf("IDirectDrawSurface::GetAttachedSurface(This=%p, dwCaps=%08X, surface=%p) ???\n", This, lpDdsCaps->dwCaps, surface);
    
    if ((This->caps & DDSCAPS_PRIMARYSURFACE) && (This->caps & DDSCAPS_FLIP) && (lpDdsCaps->dwCaps & DDSCAPS_BACKBUFFER))
    {
        This->Ref++;
        *surface = (LPDIRECTDRAWSURFACE)This;
    }

    return DD_OK;
}

HRESULT __stdcall ddraw_surface_GetBltStatus(IDirectDrawSurfaceImpl *This, DWORD a)
{
#if _DEBUG_X
    printf("IDirectDrawSurface::GetBltStatus(This=%p, ...) ???\n", This);
#endif
    return DD_OK;
}

HRESULT __stdcall ddraw_surface_GetCaps(IDirectDrawSurfaceImpl *This, LPDDSCAPS lpDDSCaps)
{
    printf("DirectDrawSurface::GetCaps(This=%p, lpDDSCaps=%p)\n", This, lpDDSCaps);
    lpDDSCaps->dwCaps = This->caps;
    return DD_OK;
}

HRESULT __stdcall ddraw_surface_GetClipper(IDirectDrawSurfaceImpl *This, LPDIRECTDRAWCLIPPER FAR *a)
{
#if _DEBUG_X
    printf("IDirectDrawSurface::GetClipper(This=%p, ...) ???\n", This);
#endif
    return DD_OK;
}

HRESULT __stdcall ddraw_surface_GetColorKey(IDirectDrawSurfaceImpl *This, DWORD flags, LPDDCOLORKEY colorKey)
{
    printf("IDirectDrawSurface::GetColorKey(This=%p, ...) ???\n", This);
    return DD_OK;
}

HRESULT __stdcall ddraw_surface_GetDC(IDirectDrawSurfaceImpl *This, HDC FAR *a)
{
#if _DEBUG_X
    printf("IDirectDrawSurface::GetDC(This=%p, ...)\n", This);
    if (This->width % 4)
        printf("   width=%d height=%d ???\n", This->width, This->height);
#endif

    RGBQUAD *data = 
        This->palette && This->palette->data_rgb ? This->palette->data_rgb :
        ddraw->primary && ddraw->primary->palette ? ddraw->primary->palette->data_rgb :
        NULL;

    if (data)
        SetDIBColorTable(This->hDC, 0, 256, data);

    *a = This->hDC;
    return DD_OK;
}

HRESULT __stdcall ddraw_surface_GetFlipStatus(IDirectDrawSurfaceImpl *This, DWORD a)
{
    printf("IDirectDrawSurface::GetFlipStatus(This=%p, ...) ???\n", This);
    return DD_OK;
}

HRESULT __stdcall ddraw_surface_GetOverlayPosition(IDirectDrawSurfaceImpl *This, LPLONG a, LPLONG b)
{
    printf("IDirectDrawSurface::GetOverlayPosition(This=%p, ...) ???\n", This);
    return DD_OK;
}

HRESULT __stdcall ddraw_surface_GetPalette(IDirectDrawSurfaceImpl *This, LPDIRECTDRAWPALETTE FAR *lplpDDPalette)
{
    printf("DirectDrawSurface::GetPalette(This=%p, lplpDDPalette=%p)\n", This, lplpDDPalette);
    *lplpDDPalette = (LPDIRECTDRAWPALETTE)This->palette;
    return DD_OK;
}

HRESULT __stdcall ddraw_surface_GetPixelFormat(IDirectDrawSurfaceImpl *This, LPDDPIXELFORMAT a)
{
    printf("IDirectDrawSurface::GetPixelFormat(This=%p, ...) ???\n", This);
    return DD_OK;
}

HRESULT __stdcall ddraw_surface_Initialize(IDirectDrawSurfaceImpl *This, LPDIRECTDRAW a, LPDDSURFACEDESC b)
{
    printf("IDirectDrawSurface::Initialize(This=%p, ...) ???\n", This);
    return DD_OK;
}

HRESULT __stdcall ddraw_surface_IsLost(IDirectDrawSurfaceImpl *This)
{
#if _DEBUG_X
    printf("IDirectDrawSurface::IsLost(This=%p)\n", This);
#endif
    return DD_OK;
}

HRESULT __stdcall ddraw_surface_Lock(IDirectDrawSurfaceImpl *This, LPRECT lpDestRect, LPDDSURFACEDESC lpDDSurfaceDesc, DWORD dwFlags, HANDLE hEvent)
{
#if _DEBUG_X
    printf("DirectDrawSurface::Lock(This=%p, lpDestRect=%p, lpDDSurfaceDesc=%p, dwFlags=%08X, hEvent=%p)\n", This, lpDestRect, lpDDSurfaceDesc, (int)dwFlags, hEvent);

    if(dwFlags & DDLOCK_SURFACEMEMORYPTR)
    {
        printf(" dwFlags: DDLOCK_SURFACEMEMORYPTR\n");
    }
    if(dwFlags & DDLOCK_WAIT)
    {
        printf(" dwFlags: DDLOCK_WAIT\n");
    }
    if(dwFlags & DDLOCK_EVENT)
    {
        printf(" dwFlags: DDLOCK_EVENT\n");
    }
    if(dwFlags & DDLOCK_READONLY)
    {
        printf(" dwFlags: DDLOCK_READONLY\n");
    }
    if(dwFlags & DDLOCK_WRITEONLY)
    {
        printf(" dwFlags: DDLOCK_WRITEONLY\n");
    }
#endif

    return ddraw_surface_GetSurfaceDesc(This, lpDDSurfaceDesc);
}

HRESULT __stdcall ddraw_surface_ReleaseDC(IDirectDrawSurfaceImpl *This, HDC a)
{
#if _DEBUG_X
    printf("DirectDrawSurface::ReleaseDC(This=%p, ...)\n", This);
#endif
    return DD_OK;
}

HRESULT __stdcall ddraw_surface_Restore(IDirectDrawSurfaceImpl *This)
{
    printf("DirectDrawSurface::Restore(This=%p) ???\n", This);
    return DD_OK;
}

HRESULT __stdcall ddraw_surface_SetClipper(IDirectDrawSurfaceImpl *This, LPDIRECTDRAWCLIPPER a)
{
    printf("DirectDrawSurface::SetClipper(This=%p, ...) ???\n", This);
    return DD_OK;
}

HRESULT __stdcall ddraw_surface_SetColorKey(IDirectDrawSurfaceImpl *This, DWORD flags, LPDDCOLORKEY colorKey)
{
#if _DEBUG_X
    printf("DirectDrawSurface::SetColorKey(This=%p, flags=0x%08X, colorKey=%p) ???\n", This, flags, colorKey);

    if (colorKey)
    {
        printf("  dwColorSpaceHighValue=%d\n", colorKey->dwColorSpaceHighValue);
        printf("  dwColorSpaceLowValue=%d\n", colorKey->dwColorSpaceLowValue);
    }
#endif

    This->colorKey.dwColorSpaceHighValue = colorKey->dwColorSpaceHighValue;
    This->colorKey.dwColorSpaceLowValue = colorKey->dwColorSpaceLowValue;

    return DD_OK;
}

HRESULT __stdcall ddraw_surface_SetOverlayPosition(IDirectDrawSurfaceImpl *This, LONG a, LONG b)
{
    printf("DirectDrawSurface::SetOverlayPosition(This=%p, ...) ???\n", This);
    return DD_OK;
}

HRESULT __stdcall ddraw_surface_SetPalette(IDirectDrawSurfaceImpl *This, LPDIRECTDRAWPALETTE lpDDPalette)
{
    printf("DirectDrawSurface::SetPalette(This=%p, lpDDPalette=%p)\n", This, lpDDPalette);

    IDirectDrawPalette_AddRef(lpDDPalette);

    if(This->palette)
    {
        IDirectDrawPalette_Release(This->palette);
    }

    EnterCriticalSection(&ddraw->cs);

    This->palette = (IDirectDrawPaletteImpl *)lpDDPalette;
    This->palette->data_rgb = &This->bmi->bmiColors[0];

    LeaveCriticalSection(&ddraw->cs);

    return DD_OK;
}

HRESULT __stdcall ddraw_surface_Unlock(IDirectDrawSurfaceImpl *This, LPVOID lpRect)
{
#if _DEBUG_X
    printf("DirectDrawSurface::Unlock(This=%p, lpRect=%p)\n", This, lpRect);
#endif
    
    if(This->caps & DDSCAPS_PRIMARYSURFACE && !(This->flags & DDSD_BACKBUFFERCOUNT) && ddraw->render.run)
    {
        InterlockedExchange(&ddraw->render.surfaceUpdated, TRUE);
        ReleaseSemaphore(ddraw->render.sem, 1, NULL);
    }

    return DD_OK;
}

HRESULT __stdcall ddraw_surface_UpdateOverlay(IDirectDrawSurfaceImpl *This, LPRECT a, LPDIRECTDRAWSURFACE b, LPRECT c, DWORD d, LPDDOVERLAYFX e)
{
    printf("DirectDrawSurface::UpdateOverlay(This=%p, ...) ???\n", This);
    return DD_OK;
}

HRESULT __stdcall ddraw_surface_UpdateOverlayDisplay(IDirectDrawSurfaceImpl *This, DWORD a)
{
    printf("DirectDrawSurface::UpdateOverlayDisplay(This=%p, ...) ???\n", This);
    return DD_OK;
}

HRESULT __stdcall ddraw_surface_UpdateOverlayZOrder(IDirectDrawSurfaceImpl *This, DWORD a, LPDIRECTDRAWSURFACE b)
{
    printf("DirectDrawSurface::UpdateOverlayZOrder(This=%p, ...) ???\n", This);
    return DD_OK;
}

struct IDirectDrawSurfaceImplVtbl siface =
{
    /* IUnknown */
    ddraw_surface_QueryInterface,
    ddraw_surface_AddRef,
    ddraw_surface_Release,
    /* IDirectDrawSurface */
    ddraw_surface_AddAttachedSurface,
    ddraw_surface_AddOverlayDirtyRect,
    ddraw_surface_Blt,
    ddraw_surface_BltBatch,
    ddraw_surface_BltFast,
    ddraw_surface_DeleteAttachedSurface,
    ddraw_surface_EnumAttachedSurfaces,
    ddraw_surface_EnumOverlayZOrders,
    ddraw_surface_Flip,
    ddraw_surface_GetAttachedSurface,
    ddraw_surface_GetBltStatus,
    ddraw_surface_GetCaps,
    ddraw_surface_GetClipper,
    ddraw_surface_GetColorKey,
    ddraw_surface_GetDC,
    ddraw_surface_GetFlipStatus,
    ddraw_surface_GetOverlayPosition,
    ddraw_surface_GetPalette,
    ddraw_surface_GetPixelFormat,
    ddraw_surface_GetSurfaceDesc,
    ddraw_surface_Initialize,
    ddraw_surface_IsLost,
    ddraw_surface_Lock,
    ddraw_surface_ReleaseDC,
    ddraw_surface_Restore,
    ddraw_surface_SetClipper,
    ddraw_surface_SetColorKey,
    ddraw_surface_SetOverlayPosition,
    ddraw_surface_SetPalette,
    ddraw_surface_Unlock,
    ddraw_surface_UpdateOverlay,
    ddraw_surface_UpdateOverlayDisplay,
    ddraw_surface_UpdateOverlayZOrder
};

HRESULT __stdcall ddraw_CreateSurface(IDirectDrawImpl *This, LPDDSURFACEDESC lpDDSurfaceDesc, LPDIRECTDRAWSURFACE FAR *lpDDSurface, IUnknown FAR * unkOuter)
{
    printf("DirectDraw::CreateSurface(This=%p, lpDDSurfaceDesc=%p, lpDDSurface=%p, unkOuter=%p)\n", This, lpDDSurfaceDesc, lpDDSurface, unkOuter);

    dump_ddsd(lpDDSurfaceDesc->dwFlags);

    IDirectDrawSurfaceImpl *Surface = (IDirectDrawSurfaceImpl *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(IDirectDrawSurfaceImpl));

    Surface->lpVtbl = &siface;

    /* private stuff */
    Surface->bpp = This->bpp;
    Surface->flags = lpDDSurfaceDesc->dwFlags;

    if(lpDDSurfaceDesc->dwFlags & DDSD_CAPS)
    {
        if(lpDDSurfaceDesc->ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE)
        {
            ddraw->primary = Surface;

            Surface->width = This->width;
            Surface->height = This->height;
        }

        dump_ddscaps(lpDDSurfaceDesc->ddsCaps.dwCaps);
        Surface->caps = lpDDSurfaceDesc->ddsCaps.dwCaps;
    }

    if( !(lpDDSurfaceDesc->dwFlags & DDSD_CAPS) || !(lpDDSurfaceDesc->ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE) )
    {
        Surface->width = lpDDSurfaceDesc->dwWidth;
        Surface->height = lpDDSurfaceDesc->dwHeight;
    }

    if(Surface->width && Surface->height)
    {
        if (Surface->width == 622 && Surface->height == 51) Surface->width = 624; //AoE2

        Surface->bmi = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * 256);
        Surface->bmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        Surface->bmi->bmiHeader.biWidth = Surface->width;
        Surface->bmi->bmiHeader.biHeight = -Surface->height;
        Surface->bmi->bmiHeader.biPlanes = 1;
        Surface->bmi->bmiHeader.biBitCount = Surface->bpp;
        Surface->bmi->bmiHeader.biCompression = BI_RGB;

        WORD cClrBits = (WORD)(Surface->bmi->bmiHeader.biPlanes * Surface->bmi->bmiHeader.biBitCount);
        if (cClrBits < 24)
            Surface->bmi->bmiHeader.biClrUsed = (1 << cClrBits);

        Surface->bmi->bmiHeader.biSizeImage = ((Surface->width * cClrBits + 31) & ~31) / 8 * Surface->height;

        int i;
        for (i = 0; i < 256; i++)
        {
            Surface->bmi->bmiColors[i].rgbRed = i;
            Surface->bmi->bmiColors[i].rgbGreen = i;
            Surface->bmi->bmiColors[i].rgbBlue = i;
            Surface->bmi->bmiColors[i].rgbReserved = 0;
        }

        Surface->lXPitch = Surface->bpp / 8;
        Surface->lPitch = Surface->width * Surface->lXPitch;

        Surface->hDC = CreateCompatibleDC(ddraw->render.hDC);
        Surface->bitmap = CreateDIBSection(Surface->hDC, Surface->bmi, DIB_RGB_COLORS, (void **)&Surface->surface, NULL, 0);
        
        if (!Surface->bitmap)
            Surface->surface = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, Surface->lPitch * Surface->height * Surface->lXPitch);

        SelectObject(Surface->hDC, Surface->bitmap);
    }

    printf(" Surface = %p (%dx%d@%d)\n", Surface, (int)Surface->width, (int)Surface->height, (int)Surface->bpp);

    *lpDDSurface = (LPDIRECTDRAWSURFACE)Surface;

    Surface->Ref = 0;
    ddraw_surface_AddRef(Surface);
    
    return DD_OK;
}

void dump_ddbltflags(DWORD dwFlags)
{
    if (dwFlags & DDBLT_ALPHADEST) {
        printf("  DDBLT_ALPHADEST\n");
    }
    if (dwFlags & DDBLT_ALPHADESTCONSTOVERRIDE) {
        printf("  DDBLT_ALPHADESTCONSTOVERRIDE\n");
    }
    if (dwFlags & DDBLT_ALPHADESTNEG) {
        printf("  DDBLT_ALPHADESTNEG\n");
    }
    if (dwFlags & DDBLT_ALPHADESTSURFACEOVERRIDE) {
        printf("  DDBLT_ALPHADESTSURFACEOVERRIDE\n");
    }
    if (dwFlags & DDBLT_ALPHAEDGEBLEND) {
        printf("  DDBLT_ALPHAEDGEBLEND\n");
    }
    if (dwFlags & DDBLT_ALPHASRC) {
        printf("  DDBLT_ALPHASRC\n");
    }
    if (dwFlags & DDBLT_ALPHASRCCONSTOVERRIDE) {
        printf("  DDBLT_ALPHASRCCONSTOVERRIDE\n");
    }
    if (dwFlags & DDBLT_ALPHASRCNEG) {
        printf("  DDBLT_ALPHASRCNEG\n");
    }
    if (dwFlags & DDBLT_ALPHASRCSURFACEOVERRIDE) {
        printf("  DDBLT_ALPHASRCSURFACEOVERRIDE\n");
    }
    if (dwFlags & DDBLT_ASYNC) {
        printf("  DDBLT_ASYNC\n");
    }
    if (dwFlags & DDBLT_COLORFILL) {
        printf("  DDBLT_COLORFILL\n");
    }
    if (dwFlags & DDBLT_DDFX) {
        printf("  DDBLT_DDFX\n");
    }
    if (dwFlags & DDBLT_DDROPS) {
        printf("  DDBLT_DDROPS\n");
    }
    if (dwFlags & DDBLT_KEYDEST) {
        printf("  DDBLT_KEYDEST\n");
    }
    if (dwFlags & DDBLT_KEYDESTOVERRIDE) {
        printf("  DDBLT_KEYDESTOVERRIDE\n");
    }
    if (dwFlags & DDBLT_KEYSRC) {
        printf("  DDBLT_KEYSRC\n");
    }
    if (dwFlags & DDBLT_KEYSRCOVERRIDE) {
        printf("  DDBLT_KEYSRCOVERRIDE\n");
    }
    if (dwFlags & DDBLT_ROP) {
        printf("  DDBLT_ROP\n");
    }
    if (dwFlags & DDBLT_ROTATIONANGLE) {
        printf("  DDBLT_ROTATIONANGLE\n");
    }
    if (dwFlags & DDBLT_ZBUFFER) {
        printf("  DDBLT_ZBUFFER\n");
    }
    if (dwFlags & DDBLT_ZBUFFERDESTCONSTOVERRIDE) {
        printf("  DDBLT_ZBUFFERDESTCONSTOVERRIDE\n");
    }
    if (dwFlags & DDBLT_ZBUFFERDESTOVERRIDE) {
        printf("  DDBLT_ZBUFFERDESTOVERRIDE\n");
    }
    if (dwFlags & DDBLT_ZBUFFERSRCCONSTOVERRIDE) {
        printf("  DDBLT_ZBUFFERSRCCONSTOVERRIDE\n");
    }
    if (dwFlags & DDBLT_ZBUFFERSRCOVERRIDE) {
        printf("  DDBLT_ZBUFFERSRCOVERRIDE\n");
    }
    if (dwFlags & DDBLT_WAIT) {
        printf("  DDBLT_WAIT\n");
    }
    if (dwFlags & DDBLT_DEPTHFILL) {
        printf("  DDBLT_DEPTHFILL\n");
    }
}

void dump_ddscaps(DWORD dwCaps)
{
    if (dwCaps & DDSCAPS_ALPHA)
    {
        printf("    DDSCAPS_ALPHA\n");
    }
    if (dwCaps & DDSCAPS_BACKBUFFER)
    {
        printf("    DDSCAPS_BACKBUFFER\n");
    }
    if (dwCaps & DDSCAPS_FLIP)
    {
        printf("    DDSCAPS_FLIP\n");
    }
    if (dwCaps & DDSCAPS_FRONTBUFFER)
    {
        printf("    DDSCAPS_FRONTBUFFER\n");
    }
    if (dwCaps & DDSCAPS_PALETTE)
    {
        printf("    DDSCAPS_PALETTE\n");
    }
    if (dwCaps & DDSCAPS_TEXTURE)
    {
        printf("    DDSCAPS_TEXTURE\n");
    }
    if(dwCaps & DDSCAPS_PRIMARYSURFACE)
    {
        printf("    DDSCAPS_PRIMARYSURFACE\n");
    }
    if(dwCaps & DDSCAPS_OFFSCREENPLAIN)
    {
        printf("    DDSCAPS_OFFSCREENPLAIN\n");
    }
    if(dwCaps & DDSCAPS_VIDEOMEMORY)
    {
        printf("    DDSCAPS_VIDEOMEMORY\n");
    }
    if(dwCaps & DDSCAPS_LOCALVIDMEM)
    {
        printf("    DDSCAPS_LOCALVIDMEM\n");
    }
}

void dump_ddsd(DWORD dwFlags)
{
    if(dwFlags & DDSD_CAPS)
    {
        printf("    DDSD_CAPS\n");
    }
    if(dwFlags & DDSD_HEIGHT)
    {
        printf("    DDSD_HEIGHT\n");
    }
    if(dwFlags & DDSD_WIDTH)
    {
        printf("    DDSD_WIDTH\n");
    }
    if(dwFlags & DDSD_PITCH)
    {
        printf("    DDSD_PITCH\n");
    }
    if(dwFlags & DDSD_BACKBUFFERCOUNT)
    {
        printf("    DDSD_BACKBUFFERCOUNT\n");
    }
    if(dwFlags & DDSD_ZBUFFERBITDEPTH)
    {
        printf("    DDSD_ZBUFFERBITDEPTH\n");
    }
    if(dwFlags & DDSD_ALPHABITDEPTH)
    {
        printf("    DDSD_ALPHABITDEPTH\n");
    }
    if(dwFlags & DDSD_LPSURFACE)
    {
        printf("    DDSD_LPSURFACE\n");
    }
    if(dwFlags & DDSD_PIXELFORMAT)
    {
        printf("    DDSD_PIXELFORMAT\n");
    }
    if(dwFlags & DDSD_CKDESTOVERLAY)
    {
        printf("    DDSD_CKDESTOVERLAY\n");
    }
    if(dwFlags & DDSD_CKDESTBLT)
    {
        printf("    DDSD_CKDESTBLT\n");
    }
    if(dwFlags & DDSD_CKSRCOVERLAY)
    {
        printf("    DDSD_CKSRCOVERLAY\n");
    }
    if(dwFlags & DDSD_CKSRCBLT)
    {
        printf("    DDSD_CKSRCBLT\n");
    }
    if(dwFlags & DDSD_MIPMAPCOUNT)
    {
        printf("    DDSD_MIPMAPCOUNT\n");
    }
    if(dwFlags & DDSD_REFRESHRATE)
    {
        printf("    DDSD_REFRESHRATE\n");
    }
    if(dwFlags & DDSD_LINEARSIZE)
    {
        printf("    DDSD_LINEARSIZE\n");
    }
    if(dwFlags & DDSD_ALL)
    {
        printf("    DDSD_ALL\n");
    }
}
