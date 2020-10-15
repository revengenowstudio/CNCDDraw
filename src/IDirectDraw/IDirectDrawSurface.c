#include "IDirectDrawSurface.h"
#include "ddsurface.h"
#include "dd.h"
#include "debug.h"


HRESULT __stdcall IDirectDrawSurface__QueryInterface(IDirectDrawSurfaceImpl *This, REFIID riid, void **obj)
{
    dprintf("NOT_IMPLEMENTED -> %s(This=%p, riid=%08X, obj=%p)\n", __FUNCTION__, This, (unsigned int)riid, obj);
    HRESULT ret = S_OK;

    if (riid && !IsEqualGUID(&IID_IDirectDrawSurface, riid))
    {
        dprintf("     GUID = %08X\n", ((GUID *)riid)->Data1);

        IDirectDrawSurface_AddRef(This);
    }

    *obj = This;

    dprintf("NOT_IMPLEMENTED <- %s\n", __FUNCTION__);
    return ret;
}

ULONG __stdcall IDirectDrawSurface__AddRef(IDirectDrawSurfaceImpl *This)
{
    dprintf("-> %s(This=%p)\n", __FUNCTION__, This);
    ULONG ret = ++This->ref;
    dprintf("<- %s(This ref=%u)\n", __FUNCTION__, ret);
    return ret;
}

ULONG __stdcall IDirectDrawSurface__Release(IDirectDrawSurfaceImpl *This)
{
    dprintf("-> %s(This=%p)\n", __FUNCTION__, This);

    ULONG ret = --This->ref;

    if(This->ref == 0)
    {
        dprintf("     Released (%p)\n", This);

        if ((This->caps & DDSCAPS_PRIMARYSURFACE))
        {
            EnterCriticalSection(&g_ddraw->cs);
            g_ddraw->primary = NULL;
            LeaveCriticalSection(&g_ddraw->cs);
        }

        if (This->bitmap)
        {
            DeleteObject(This->bitmap);
        }
        else if (This->surface)
        {
            HeapFree(GetProcessHeap(), 0, This->surface);
        }

        if (This->hdc)
            DeleteDC(This->hdc);

        if (This->bmi)
            HeapFree(GetProcessHeap(), 0, This->bmi);

        if(This->palette && (!g_ddraw || (void*)This->palette != g_ddraw->last_freed_palette))
        {
            IDirectDrawPalette_Release(This->palette);
        }

        HeapFree(GetProcessHeap(), 0, This);
    }

    dprintf("<- %s(This ref=%u)\n", __FUNCTION__, ret);
    return ret;
}

HRESULT __stdcall IDirectDrawSurface__AddAttachedSurface(IDirectDrawSurfaceImpl *This, LPDIRECTDRAWSURFACE lpDDSurface)
{
    dprintf("-> %s(This=%p, lpDDSurface=%p)\n", __FUNCTION__, This, lpDDSurface);
    HRESULT ret = dds_AddAttachedSurface(This, lpDDSurface);
    dprintf("<- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDrawSurface__AddOverlayDirtyRect(IDirectDrawSurfaceImpl *This, LPRECT a)
{
    dprintf("NOT_IMPLEMENTED -> %s(This=%p)\n", __FUNCTION__, This);
    HRESULT ret = DD_OK;
    dprintf("NOT_IMPLEMENTED <- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDrawSurface__Blt(IDirectDrawSurfaceImpl *This, LPRECT lpDestRect, LPDIRECTDRAWSURFACE lpDDSrcSurface, LPRECT lpSrcRect, DWORD dwFlags, LPDDBLTFX lpDDBltFx)
{
    dprintfex("-> %s(This=%p, lpDestRect=%p, lpDDSrcSurface=%p, lpSrcRect=%p, dwFlags=%08X, lpDDBltFx=%p)\n", __FUNCTION__, This, lpDestRect, lpDDSrcSurface, lpSrcRect, (int)dwFlags, lpDDBltFx);
    HRESULT ret = dds_Blt(This, lpDestRect, lpDDSrcSurface, lpSrcRect, dwFlags, lpDDBltFx);
    dprintfex("<- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDrawSurface__BltBatch(IDirectDrawSurfaceImpl *This, LPDDBLTBATCH a, DWORD b, DWORD c)
{
    dprintf("NOT_IMPLEMENTED -> %s(This=%p)\n", __FUNCTION__, This);
    HRESULT ret = DD_OK;
    dprintf("NOT_IMPLEMENTED <- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDrawSurface__BltFast(IDirectDrawSurfaceImpl *This, DWORD dst_x, DWORD dst_y, LPDIRECTDRAWSURFACE lpDDSrcSurface, LPRECT lpSrcRect, DWORD flags)
{
    dprintfex("-> %s(This=%p, x=%d, y=%d, lpDDSrcSurface=%p, lpSrcRect=%p, flags=%08X)\n", __FUNCTION__, This, dst_x, dst_y, lpDDSrcSurface, lpSrcRect, flags);
    HRESULT ret = dds_BltFast(This, dst_x, dst_y, lpDDSrcSurface, lpSrcRect, flags);
    dprintfex("<- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDrawSurface__DeleteAttachedSurface(IDirectDrawSurfaceImpl *This, DWORD dwFlags, LPDIRECTDRAWSURFACE lpDDSurface)
{
    dprintf("-> %s(This=%p, dwFlags=%08X, lpDDSurface=%p)\n", __FUNCTION__, This, (int)dwFlags, lpDDSurface);
    HRESULT ret = dds_DeleteAttachedSurface(This, dwFlags, lpDDSurface);
    dprintf("<- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDrawSurface__GetSurfaceDesc(IDirectDrawSurfaceImpl *This, LPDDSURFACEDESC lpDDSurfaceDesc)
{
    dprintfex("-> %s(This=%p, lpDDSurfaceDesc=%p)\n", __FUNCTION__, This, lpDDSurfaceDesc);
    HRESULT ret = dds_GetSurfaceDesc(This, lpDDSurfaceDesc);
    dprintfex("<- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDrawSurface__EnumAttachedSurfaces(IDirectDrawSurfaceImpl *This, LPVOID lpContext, LPDDENUMSURFACESCALLBACK lpEnumSurfacesCallback)
{
    dprintf("-> %s(This=%p, lpContext=%p, lpEnumSurfacesCallback=%p)\n", __FUNCTION__, This, lpContext, lpEnumSurfacesCallback);
    HRESULT ret = dds_EnumAttachedSurfaces(This, lpContext, lpEnumSurfacesCallback);
    dprintf("<- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDrawSurface__EnumOverlayZOrders(IDirectDrawSurfaceImpl *This, DWORD a, LPVOID b, LPDDENUMSURFACESCALLBACK c)
{
    dprintf("NOT_IMPLEMENTED -> %s(This=%p)\n", __FUNCTION__, This);
    HRESULT ret = DD_OK;
    dprintf("NOT_IMPLEMENTED <- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDrawSurface__Flip(IDirectDrawSurfaceImpl *This, LPDIRECTDRAWSURFACE surface, DWORD flags)
{
    dprintfex("-> %s(This=%p, surface=%p, flags=%08X)\n", __FUNCTION__, This, surface, flags);
    HRESULT ret = dds_Flip(This, surface, flags);
    dprintfex("<- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDrawSurface__GetAttachedSurface(IDirectDrawSurfaceImpl *This, LPDDSCAPS lpDdsCaps, LPDIRECTDRAWSURFACE FAR *surface)
{
    dprintf("-> %s(This=%p, dwCaps=%08X, surface=%p)\n", __FUNCTION__, This, lpDdsCaps->dwCaps, surface);
    HRESULT ret = dds_GetAttachedSurface(This, lpDdsCaps, surface);
    dprintf("<- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDrawSurface__GetBltStatus(IDirectDrawSurfaceImpl *This, DWORD a)
{
    dprintfex("NOT_IMPLEMENTED -> %s(This=%p)\n", __FUNCTION__, This);
    HRESULT ret = DD_OK;
    dprintfex("NOT_IMPLEMENTED <- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDrawSurface__GetCaps(IDirectDrawSurfaceImpl *This, LPDDSCAPS lpDDSCaps)
{
    dprintf("-> %s(This=%p, lpDDSCaps=%p)\n", __FUNCTION__, This, lpDDSCaps);
    HRESULT ret = dds_GetCaps(This, lpDDSCaps);
    dprintf("<- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDrawSurface__GetClipper(IDirectDrawSurfaceImpl *This, LPDIRECTDRAWCLIPPER FAR *a)
{
    dprintfex("NOT_IMPLEMENTED -> %s(This=%p)\n", __FUNCTION__, This);
    HRESULT ret = DD_OK;
    dprintfex("NOT_IMPLEMENTED <- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDrawSurface__GetColorKey(IDirectDrawSurfaceImpl *This, DWORD flags, LPDDCOLORKEY colorKey)
{
    dprintfex("-> %s(This=%p, flags=0x%08X, color_key=%p)\n", __FUNCTION__, This, flags, colorKey);
    HRESULT ret = dds_GetColorKey(This, flags, colorKey);
    dprintfex("<- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDrawSurface__GetDC(IDirectDrawSurfaceImpl *This, HDC FAR *a)
{
    dprintfex("-> %s(This=%p, ...)\n", __FUNCTION__, This);
    HRESULT ret = dds_GetDC(This, a);
    dprintfex("<- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDrawSurface__GetFlipStatus(IDirectDrawSurfaceImpl *This, DWORD a)
{
    dprintfex("NOT_IMPLEMENTED -> %s(This=%p)\n", __FUNCTION__, This);
    HRESULT ret = DD_OK;
    dprintfex("NOT_IMPLEMENTED <- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDrawSurface__GetOverlayPosition(IDirectDrawSurfaceImpl *This, LPLONG a, LPLONG b)
{
    dprintf("NOT_IMPLEMENTED -> %s(This=%p)\n", __FUNCTION__, This);
    HRESULT ret = DD_OK;
    dprintf("NOT_IMPLEMENTED <- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDrawSurface__GetPalette(IDirectDrawSurfaceImpl *This, LPDIRECTDRAWPALETTE FAR *lplpDDPalette)
{
    dprintf("-> %s(This=%p, lplpDDPalette=%p)\n", __FUNCTION__, This, lplpDDPalette);
    HRESULT ret = dds_GetPalette(This, lplpDDPalette);
    dprintf("<- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDrawSurface__GetPixelFormat(IDirectDrawSurfaceImpl *This, LPDDPIXELFORMAT ddpfPixelFormat)
{
    dprintfex("-> %s(This=%p, ...)\n", __FUNCTION__, This);
    HRESULT ret = dds_GetPixelFormat(This, ddpfPixelFormat);
    dprintfex("<- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDrawSurface__Initialize(IDirectDrawSurfaceImpl *This, LPDIRECTDRAW a, LPDDSURFACEDESC b)
{
    dprintf("NOT_IMPLEMENTED -> %s(This=%p)\n", __FUNCTION__, This);
    HRESULT ret = DD_OK;
    dprintf("NOT_IMPLEMENTED <- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDrawSurface__IsLost(IDirectDrawSurfaceImpl *This)
{
    dprintfex("NOT_IMPLEMENTED -> %s(This=%p)\n", __FUNCTION__, This);
    HRESULT ret = DD_OK;
    dprintfex("NOT_IMPLEMENTED <- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDrawSurface__Lock(IDirectDrawSurfaceImpl *This, LPRECT lpDestRect, LPDDSURFACEDESC lpDDSurfaceDesc, DWORD dwFlags, HANDLE hEvent)
{
    dprintfex("-> %s(This=%p, lpDestRect=%p, lpDDSurfaceDesc=%p, dwFlags=%08X, hEvent=%p)\n", __FUNCTION__, This, lpDestRect, lpDDSurfaceDesc, (int)dwFlags, hEvent);
    HRESULT ret = dds_Lock(This, lpDestRect, lpDDSurfaceDesc, dwFlags, hEvent);
    dprintfex("<- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDrawSurface__ReleaseDC(IDirectDrawSurfaceImpl *This, HDC a)
{
    dprintfex("-> %s(This=%p)\n", __FUNCTION__, This);
    HRESULT ret = DD_OK;
    dprintfex("<- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDrawSurface__Restore(IDirectDrawSurfaceImpl *This)
{
    dprintfex("NOT_IMPLEMENTED -> %s(This=%p)\n", __FUNCTION__, This);
    HRESULT ret = DD_OK;
    dprintfex("NOT_IMPLEMENTED <- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDrawSurface__SetClipper(IDirectDrawSurfaceImpl *This, LPDIRECTDRAWCLIPPER a)
{
    dprintf("NOT_IMPLEMENTED -> %s(This=%p)\n", __FUNCTION__, This);
    HRESULT ret = DD_OK;
    dprintf("NOT_IMPLEMENTED <- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDrawSurface__SetColorKey(IDirectDrawSurfaceImpl *This, DWORD flags, LPDDCOLORKEY colorKey)
{
    dprintfex("-> %s(This=%p, flags=0x%08X, color_key=%p)\n", __FUNCTION__, This, flags, colorKey);
    HRESULT ret = dds_SetColorKey(This, flags, colorKey);
    dprintfex("<- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDrawSurface__SetOverlayPosition(IDirectDrawSurfaceImpl *This, LONG a, LONG b)
{
    dprintf("NOT_IMPLEMENTED -> %s(This=%p)\n", __FUNCTION__, This);
    HRESULT ret = DD_OK;
    dprintf("NOT_IMPLEMENTED <- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDrawSurface__SetPalette(IDirectDrawSurfaceImpl *This, LPDIRECTDRAWPALETTE lpDDPalette)
{
    dprintf("-> %s(This=%p, lpDDPalette=%p)\n", __FUNCTION__, This, lpDDPalette);
    HRESULT ret = dds_SetPalette(This, lpDDPalette);
    dprintf("<- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDrawSurface__Unlock(IDirectDrawSurfaceImpl *This, LPVOID lpRect)
{
    dprintfex("-> %s(This=%p, lpRect=%p)\n", __FUNCTION__, This, lpRect);
    HRESULT ret = dds_Unlock(This, lpRect);
    dprintfex("<- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDrawSurface__UpdateOverlay(IDirectDrawSurfaceImpl *This, LPRECT a, LPDIRECTDRAWSURFACE b, LPRECT c, DWORD d, LPDDOVERLAYFX e)
{
    dprintf("NOT_IMPLEMENTED -> %s(This=%p)\n", __FUNCTION__, This);
    HRESULT ret = DD_OK;
    dprintf("NOT_IMPLEMENTED <- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDrawSurface__UpdateOverlayDisplay(IDirectDrawSurfaceImpl *This, DWORD a)
{
    dprintf("NOT_IMPLEMENTED -> %s(This=%p)\n", __FUNCTION__, This);
    HRESULT ret = DD_OK;
    dprintf("NOT_IMPLEMENTED <- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDrawSurface__UpdateOverlayZOrder(IDirectDrawSurfaceImpl *This, DWORD a, LPDIRECTDRAWSURFACE b)
{
    dprintf("NOT_IMPLEMENTED -> %s(This=%p)\n", __FUNCTION__, This);
    HRESULT ret = DD_OK;
    dprintf("NOT_IMPLEMENTED <- %s\n", __FUNCTION__);
    return ret;
}

struct IDirectDrawSurfaceImplVtbl g_dds_vtbl =
{
    /* IUnknown */
    IDirectDrawSurface__QueryInterface,
    IDirectDrawSurface__AddRef,
    IDirectDrawSurface__Release,
    /* IDirectDrawSurface */
    IDirectDrawSurface__AddAttachedSurface,
    IDirectDrawSurface__AddOverlayDirtyRect,
    IDirectDrawSurface__Blt,
    IDirectDrawSurface__BltBatch,
    IDirectDrawSurface__BltFast,
    IDirectDrawSurface__DeleteAttachedSurface,
    IDirectDrawSurface__EnumAttachedSurfaces,
    IDirectDrawSurface__EnumOverlayZOrders,
    IDirectDrawSurface__Flip,
    IDirectDrawSurface__GetAttachedSurface,
    IDirectDrawSurface__GetBltStatus,
    IDirectDrawSurface__GetCaps,
    IDirectDrawSurface__GetClipper,
    IDirectDrawSurface__GetColorKey,
    IDirectDrawSurface__GetDC,
    IDirectDrawSurface__GetFlipStatus,
    IDirectDrawSurface__GetOverlayPosition,
    IDirectDrawSurface__GetPalette,
    IDirectDrawSurface__GetPixelFormat,
    IDirectDrawSurface__GetSurfaceDesc,
    IDirectDrawSurface__Initialize,
    IDirectDrawSurface__IsLost,
    IDirectDrawSurface__Lock,
    IDirectDrawSurface__ReleaseDC,
    IDirectDrawSurface__Restore,
    IDirectDrawSurface__SetClipper,
    IDirectDrawSurface__SetColorKey,
    IDirectDrawSurface__SetOverlayPosition,
    IDirectDrawSurface__SetPalette,
    IDirectDrawSurface__Unlock,
    IDirectDrawSurface__UpdateOverlay,
    IDirectDrawSurface__UpdateOverlayDisplay,
    IDirectDrawSurface__UpdateOverlayZOrder
};
