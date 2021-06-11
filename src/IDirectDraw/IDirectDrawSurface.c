#include <initguid.h>
#include "IDirectDrawSurface.h"
#include "IDirectDrawGammaControl.h"
#include "ddsurface.h"
#include "dd.h"
#include "debug.h"


HRESULT __stdcall IDirectDrawSurface__QueryInterface(IDirectDrawSurfaceImpl* This, REFIID riid, LPVOID FAR* ppvObj)
{
    TRACE("-> %s(This=%p, riid=%08X, ppvObj=%p)\n", __FUNCTION__, This, (unsigned int)riid, ppvObj);
    HRESULT ret = S_OK;

    if (riid)
    {
        if (IsEqualGUID(&IID_IDirectDrawSurface, riid) ||
            IsEqualGUID(&IID_IDirectDrawSurface2, riid) ||
            IsEqualGUID(&IID_IDirectDrawSurface3, riid) ||
            IsEqualGUID(&IID_IDirectDrawSurface4, riid) ||
            IsEqualGUID(&IID_IDirectDrawSurface7, riid))
        {
            TRACE("     GUID = %08X (IID_IDirectDrawSurfaceX)\n", ((GUID*)riid)->Data1);

            IDirectDrawSurface_AddRef(This);

            *ppvObj = This;

            ret = S_OK;
        }
        else if (IsEqualGUID(&IID_IDirectDrawGammaControl, riid))
        {
            IDirectDrawGammaControlImpl* gc =
                (IDirectDrawGammaControlImpl*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(IDirectDrawGammaControlImpl));

            TRACE(
                "NOT_IMPLEMENTED      GUID = %08X (IID_IDirectDrawGammaControl), gammacontrol = %p\n",
                ((GUID*)riid)->Data1,
                gc);

            gc->lpVtbl = &g_ddgc_vtbl;
            gc->lpVtbl->AddRef(gc);

            *ppvObj = gc;

            ret = S_OK;
        }
        else
        {
            TRACE("NOT_IMPLEMENTED     GUID = %08X\n", ((GUID*)riid)->Data1);

            ret = E_NOINTERFACE;
        }
    }

    TRACE("<- %s\n", __FUNCTION__);
    return ret;
}

ULONG __stdcall IDirectDrawSurface__AddRef(IDirectDrawSurfaceImpl* This)
{
    TRACE("-> %s(This=%p)\n", __FUNCTION__, This);
    ULONG ret = ++This->ref;
    TRACE("<- %s(This ref=%u)\n", __FUNCTION__, ret);
    return ret;
}

ULONG __stdcall IDirectDrawSurface__Release(IDirectDrawSurfaceImpl* This)
{
    TRACE("-> %s(This=%p)\n", __FUNCTION__, This);

    ULONG ret = --This->ref;

    if (This->ref == 0)
    {
        TRACE("     Released (%p)\n", This);

        if (g_ddraw && (This->caps & DDSCAPS_PRIMARYSURFACE))
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

        if (This->backbuffer)
            IDirectDrawSurface_Release(This->backbuffer);

        if (This->clipper)
            IDirectDrawClipper_Release(This->clipper);

        if (This->palette && (!g_ddraw || (void*)This->palette != g_ddraw->last_freed_palette))
        {
            IDirectDrawPalette_Release(This->palette);
        }

        HeapFree(GetProcessHeap(), 0, This);
    }

    TRACE("<- %s(This ref=%u)\n", __FUNCTION__, ret);
    return ret;
}

HRESULT __stdcall IDirectDrawSurface__AddAttachedSurface(IDirectDrawSurfaceImpl* This, LPDIRECTDRAWSURFACE7 lpDDSurface)
{
    TRACE("-> %s(This=%p, lpDDSurface=%p)\n", __FUNCTION__, This, lpDDSurface);
    HRESULT ret = dds_AddAttachedSurface(This, (IDirectDrawSurfaceImpl*)lpDDSurface);
    TRACE("<- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDrawSurface__AddOverlayDirtyRect(IDirectDrawSurfaceImpl* This, LPRECT lpRect)
{
    TRACE("NOT_IMPLEMENTED -> %s(This=%p)\n", __FUNCTION__, This);
    HRESULT ret = DD_OK;
    TRACE("NOT_IMPLEMENTED <- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDrawSurface__Blt(
    IDirectDrawSurfaceImpl* This,
    LPRECT lpDestRect,
    LPDIRECTDRAWSURFACE7 lpDDSrcSurface,
    LPRECT lpSrcRect,
    DWORD dwFlags,
    LPDDBLTFX lpDDBltFx)
{
    TRACE_EXT(
        "-> %s(This=%p, lpDestRect=%p, lpDDSrcSurface=%p, lpSrcRect=%p, dwFlags=%08X, lpDDBltFx=%p)\n",
        __FUNCTION__,
        This,
        lpDestRect,
        lpDDSrcSurface,
        lpSrcRect,
        dwFlags,
        lpDDBltFx);

    HRESULT ret = dds_Blt(This, lpDestRect, (IDirectDrawSurfaceImpl*)lpDDSrcSurface, lpSrcRect, dwFlags, lpDDBltFx);

    TRACE_EXT("<- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDrawSurface__BltBatch(
    IDirectDrawSurfaceImpl* This,
    LPDDBLTBATCH lpDDBltBatch,
    DWORD dwCount,
    DWORD dwFlags)
{
    TRACE("NOT_IMPLEMENTED -> %s(This=%p)\n", __FUNCTION__, This);
    HRESULT ret = DD_OK;
    TRACE("NOT_IMPLEMENTED <- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDrawSurface__BltFast(
    IDirectDrawSurfaceImpl* This,
    DWORD dwX,
    DWORD dwY,
    LPDIRECTDRAWSURFACE7 lpDDSrcSurface,
    LPRECT lpSrcRect,
    DWORD dwFlags)
{
    TRACE_EXT(
        "-> %s(This=%p, dwX=%d, dwY=%d, lpDDSrcSurface=%p, lpSrcRect=%p, dwFlags=%08X)\n",
        __FUNCTION__,
        This,
        dwX,
        dwY,
        lpDDSrcSurface,
        lpSrcRect,
        dwFlags);

    HRESULT ret = dds_BltFast(This, dwX, dwY, (IDirectDrawSurfaceImpl*)lpDDSrcSurface, lpSrcRect, dwFlags);

    TRACE_EXT("<- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDrawSurface__DeleteAttachedSurface(
    IDirectDrawSurfaceImpl* This,
    DWORD dwFlags,
    LPDIRECTDRAWSURFACE7 lpDDSurface)
{
    TRACE("-> %s(This=%p, dwFlags=%08X, lpDDSurface=%p)\n", __FUNCTION__, This, dwFlags, lpDDSurface);
    HRESULT ret = dds_DeleteAttachedSurface(This, dwFlags, (IDirectDrawSurfaceImpl*)lpDDSurface);
    TRACE("<- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDrawSurface__EnumAttachedSurfaces(
    IDirectDrawSurfaceImpl* This,
    LPVOID lpContext,
    LPDDENUMSURFACESCALLBACK7 lpEnumSurfacesCallback)
{
    TRACE(
        "-> %s(This=%p, lpContext=%p, lpEnumSurfacesCallback=%p)\n",
        __FUNCTION__,
        This,
        lpContext,
        lpEnumSurfacesCallback);

    HRESULT ret = dds_EnumAttachedSurfaces(This, lpContext, lpEnumSurfacesCallback);

    TRACE("<- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDrawSurface__EnumOverlayZOrders(
    IDirectDrawSurfaceImpl* This,
    DWORD dwFlags,
    LPVOID lpContext,
    LPDDENUMSURFACESCALLBACK7 lpfnCallback)
{
    TRACE("NOT_IMPLEMENTED -> %s(This=%p)\n", __FUNCTION__, This);
    HRESULT ret = DD_OK;
    TRACE("NOT_IMPLEMENTED <- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDrawSurface__Flip(
    IDirectDrawSurfaceImpl* This,
    LPDIRECTDRAWSURFACE7 lpDDSurfaceTargetOverride,
    DWORD dwFlags)
{
    TRACE_EXT(
        "-> %s(This=%p, lpDDSurfaceTargetOverride=%p, dwFlags=%08X)\n",
        __FUNCTION__, 
        This, 
        lpDDSurfaceTargetOverride, 
        dwFlags);

    HRESULT ret = dds_Flip(This, (IDirectDrawSurfaceImpl*)lpDDSurfaceTargetOverride, dwFlags);

    TRACE_EXT("<- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDrawSurface__GetAttachedSurface(
    IDirectDrawSurfaceImpl* This,
    LPDDSCAPS2 lpDdsCaps,
    LPDIRECTDRAWSURFACE7 FAR* lpDDsurface)
{
    TRACE("-> %s(This=%p, dwCaps=%08X, lpDDsurface=%p)\n", __FUNCTION__, This, lpDdsCaps->dwCaps, lpDDsurface);
    HRESULT ret = dds_GetAttachedSurface(This, lpDdsCaps, (IDirectDrawSurfaceImpl**)lpDDsurface);
    TRACE("<- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDrawSurface__GetBltStatus(IDirectDrawSurfaceImpl* This, DWORD dwFlags)
{
    TRACE_EXT("-> %s(This=%p)\n", __FUNCTION__, This);
    HRESULT ret = DD_OK;
    TRACE_EXT("<- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDrawSurface__GetCaps(IDirectDrawSurfaceImpl* This, LPDDSCAPS2 lpDDSCaps)
{
    TRACE("-> %s(This=%p, lpDDSCaps=%p)\n", __FUNCTION__, This, lpDDSCaps);
    HRESULT ret = dds_GetCaps(This, lpDDSCaps);
    TRACE("<- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDrawSurface__GetClipper(IDirectDrawSurfaceImpl* This, LPDIRECTDRAWCLIPPER FAR* lpClipper)
{
    TRACE_EXT("-> %s(This=%p, lpClipper=%p)\n", __FUNCTION__, This, lpClipper);
    HRESULT ret = dds_GetClipper(This, (IDirectDrawClipperImpl**)lpClipper);
    TRACE_EXT("<- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDrawSurface__GetColorKey(IDirectDrawSurfaceImpl* This, DWORD dwFlags, LPDDCOLORKEY lpColorKey)
{
    TRACE_EXT("-> %s(This=%p, dwFlags=0x%08X, lpColorKey=%p)\n", __FUNCTION__, This, dwFlags, lpColorKey);
    HRESULT ret = dds_GetColorKey(This, dwFlags, lpColorKey);
    TRACE_EXT("<- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDrawSurface__GetDC(IDirectDrawSurfaceImpl* This, HDC FAR* lpHDC)
{
    TRACE_EXT("-> %s(This=%p, lpHDC=%p)\n", __FUNCTION__, This, lpHDC);
    HRESULT ret = dds_GetDC(This, lpHDC);
    TRACE_EXT("<- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDrawSurface__GetFlipStatus(IDirectDrawSurfaceImpl* This, DWORD dwFlags)
{
    TRACE_EXT("-> %s(This=%p)\n", __FUNCTION__, This);
    HRESULT ret = DD_OK;
    TRACE_EXT("<- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDrawSurface__GetOverlayPosition(IDirectDrawSurfaceImpl* This, LPLONG lplX, LPLONG lplY)
{
    TRACE("NOT_IMPLEMENTED -> %s(This=%p)\n", __FUNCTION__, This);
    HRESULT ret = DDERR_NOTAOVERLAYSURFACE;
    TRACE("NOT_IMPLEMENTED <- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDrawSurface__GetPalette(IDirectDrawSurfaceImpl* This, LPDIRECTDRAWPALETTE FAR* lplpDDPalette)
{
    TRACE("-> %s(This=%p, lplpDDPalette=%p)\n", __FUNCTION__, This, lplpDDPalette);
    HRESULT ret = dds_GetPalette(This, (IDirectDrawPaletteImpl**)lplpDDPalette);
    TRACE("<- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDrawSurface__GetPixelFormat(IDirectDrawSurfaceImpl* This, LPDDPIXELFORMAT ddpfPixelFormat)
{
    TRACE_EXT("-> %s(This=%p, ...)\n", __FUNCTION__, This);
    HRESULT ret = dds_GetPixelFormat(This, ddpfPixelFormat);
    TRACE_EXT("<- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDrawSurface__GetSurfaceDesc(IDirectDrawSurfaceImpl* This, LPDDSURFACEDESC2 lpDDSurfaceDesc)
{
    TRACE_EXT("-> %s(This=%p, lpDDSurfaceDesc=%p)\n", __FUNCTION__, This, lpDDSurfaceDesc);
    HRESULT ret = dds_GetSurfaceDesc(This, lpDDSurfaceDesc);
    TRACE_EXT("<- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDrawSurface__Initialize(
    IDirectDrawSurfaceImpl* This,
    LPDIRECTDRAW lpDD,
    LPDDSURFACEDESC2 lpDDSurfaceDesc)
{
    TRACE("NOT_IMPLEMENTED -> %s(This=%p)\n", __FUNCTION__, This);
    HRESULT ret = DD_OK;
    TRACE("NOT_IMPLEMENTED <- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDrawSurface__IsLost(IDirectDrawSurfaceImpl* This)
{
    TRACE_EXT("-> %s(This=%p)\n", __FUNCTION__, This);
    HRESULT ret = DD_OK;
    TRACE_EXT("<- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDrawSurface__Lock(
    IDirectDrawSurfaceImpl* This,
    LPRECT lpDestRect,
    LPDDSURFACEDESC2 lpDDSurfaceDesc,
    DWORD dwFlags,
    HANDLE hEvent)
{
    TRACE_EXT(
        "-> %s(This=%p, lpDestRect=%p, lpDDSurfaceDesc=%p, dwFlags=%08X, hEvent=%p)\n",
        __FUNCTION__,
        This,
        lpDestRect,
        lpDDSurfaceDesc,
        dwFlags,
        hEvent);

    HRESULT ret = dds_Lock(This, lpDestRect, lpDDSurfaceDesc, dwFlags, hEvent);

    TRACE_EXT("<- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDrawSurface__ReleaseDC(IDirectDrawSurfaceImpl* This, HDC hDC)
{
    TRACE_EXT("-> %s(This=%p)\n", __FUNCTION__, This);
    HRESULT ret = dds_ReleaseDC(This, hDC);
    TRACE_EXT("<- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDrawSurface__Restore(IDirectDrawSurfaceImpl* This)
{
    TRACE_EXT("-> %s(This=%p)\n", __FUNCTION__, This);
    HRESULT ret = DD_OK;
    TRACE_EXT("<- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDrawSurface__SetClipper(IDirectDrawSurfaceImpl* This, LPDIRECTDRAWCLIPPER lpClipper)
{
    TRACE("-> %s(This=%p, lpClipper=%p)\n", __FUNCTION__, This, lpClipper);
    HRESULT ret = dds_SetClipper(This, (IDirectDrawClipperImpl*)lpClipper);
    TRACE("<- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDrawSurface__SetColorKey(IDirectDrawSurfaceImpl* This, DWORD dwFlags, LPDDCOLORKEY lpColorKey)
{
    TRACE_EXT("-> %s(This=%p, dwFlags=0x%08X, lpColorKey=%p)\n", __FUNCTION__, This, dwFlags, lpColorKey);
    HRESULT ret = dds_SetColorKey(This, dwFlags, lpColorKey);
    TRACE_EXT("<- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDrawSurface__SetOverlayPosition(IDirectDrawSurfaceImpl* This, LONG lX, LONG lY)
{
    TRACE("NOT_IMPLEMENTED -> %s(This=%p)\n", __FUNCTION__, This);
    HRESULT ret = DD_OK;
    TRACE("NOT_IMPLEMENTED <- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDrawSurface__SetPalette(IDirectDrawSurfaceImpl* This, LPDIRECTDRAWPALETTE lpDDPalette)
{
    TRACE("-> %s(This=%p, lpDDPalette=%p)\n", __FUNCTION__, This, lpDDPalette);
    HRESULT ret = dds_SetPalette(This, (IDirectDrawPaletteImpl*)lpDDPalette);
    TRACE("<- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDrawSurface__Unlock(IDirectDrawSurfaceImpl* This, LPRECT lpRect)
{
    TRACE_EXT("-> %s(This=%p, lpRect=%p)\n", __FUNCTION__, This, lpRect);
    HRESULT ret = dds_Unlock(This, lpRect);
    TRACE_EXT("<- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDrawSurface__UpdateOverlay(
    IDirectDrawSurfaceImpl* This,
    LPRECT lpSrcRect,
    LPDIRECTDRAWSURFACE7 lpDDDestSurface,
    LPRECT lpDestRect,
    DWORD dwFlags,
    LPDDOVERLAYFX lpDDOverlayFx)
{
    TRACE("NOT_IMPLEMENTED -> %s(This=%p)\n", __FUNCTION__, This);
    HRESULT ret = DD_OK;
    TRACE("NOT_IMPLEMENTED <- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDrawSurface__UpdateOverlayDisplay(IDirectDrawSurfaceImpl* This, DWORD dwFlags)
{
    TRACE("NOT_IMPLEMENTED -> %s(This=%p)\n", __FUNCTION__, This);
    HRESULT ret = DD_OK;
    TRACE("NOT_IMPLEMENTED <- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDrawSurface__UpdateOverlayZOrder(
    IDirectDrawSurfaceImpl* This,
    DWORD dwFlags,
    LPDIRECTDRAWSURFACE7 lpDDSReference)
{
    TRACE("NOT_IMPLEMENTED -> %s(This=%p)\n", __FUNCTION__, This);
    HRESULT ret = DD_OK;
    TRACE("NOT_IMPLEMENTED <- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDrawSurface__GetDDInterface(IDirectDrawSurfaceImpl* This, LPVOID* lplpDD)
{
    TRACE("-> %s(This=%p)\n", __FUNCTION__, This);
    HRESULT ret = dds_GetDDInterface(This, lplpDD);
    TRACE("<- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDrawSurface__PageLock(IDirectDrawSurfaceImpl* This, DWORD dwFlags)
{
    TRACE("NOT_IMPLEMENTED -> %s(This=%p)\n", __FUNCTION__, This);
    HRESULT ret = DD_OK;
    TRACE("NOT_IMPLEMENTED <- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDrawSurface__PageUnlock(IDirectDrawSurfaceImpl* This, DWORD dwFlags)
{
    TRACE("NOT_IMPLEMENTED -> %s(This=%p)\n", __FUNCTION__, This);
    HRESULT ret = DD_OK;
    TRACE("NOT_IMPLEMENTED <- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDrawSurface__SetSurfaceDesc(IDirectDrawSurfaceImpl* This, LPDDSURFACEDESC2 lpDDSD, DWORD dwFlags)
{
    TRACE("NOT_IMPLEMENTED -> %s(This=%p)\n", __FUNCTION__, This);
    HRESULT ret = DDERR_UNSUPPORTED;
    TRACE("NOT_IMPLEMENTED <- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDrawSurface__SetPrivateData(
    IDirectDrawSurfaceImpl* This,
    REFGUID rtag,
    LPVOID lpData,
    DWORD dwSize,
    DWORD dwFlags)
{
    TRACE("NOT_IMPLEMENTED -> %s(This=%p)\n", __FUNCTION__, This);
    HRESULT ret = DDERR_OUTOFMEMORY;
    TRACE("NOT_IMPLEMENTED <- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDrawSurface__GetPrivateData(
    IDirectDrawSurfaceImpl* This,
    REFGUID rtag,
    LPVOID lpBuffer,
    LPDWORD lpdwBufferSize)
{
    TRACE("NOT_IMPLEMENTED -> %s(This=%p)\n", __FUNCTION__, This);
    HRESULT ret = DDERR_NOTFOUND;
    TRACE("NOT_IMPLEMENTED <- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDrawSurface__FreePrivateData(IDirectDrawSurfaceImpl* This, REFGUID rtag)
{
    TRACE("NOT_IMPLEMENTED -> %s(This=%p)\n", __FUNCTION__, This);
    HRESULT ret = DD_OK;
    TRACE("NOT_IMPLEMENTED <- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDrawSurface__GetUniquenessValue(IDirectDrawSurfaceImpl* This, LPDWORD lpdwValue)
{
    TRACE("NOT_IMPLEMENTED -> %s(This=%p)\n", __FUNCTION__, This);
    HRESULT ret = DDERR_INVALIDOBJECT;
    TRACE("NOT_IMPLEMENTED <- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDrawSurface__ChangeUniquenessValue(IDirectDrawSurfaceImpl* This)
{
    TRACE("NOT_IMPLEMENTED -> %s(This=%p)\n", __FUNCTION__, This);
    HRESULT ret = DDERR_INVALIDOBJECT;
    TRACE("NOT_IMPLEMENTED <- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDrawSurface__SetPriority(IDirectDrawSurfaceImpl* This, DWORD dwPrio)
{
    TRACE("NOT_IMPLEMENTED -> %s(This=%p)\n", __FUNCTION__, This);
    HRESULT ret = DDERR_INVALIDOBJECT;
    TRACE("NOT_IMPLEMENTED <- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDrawSurface__GetPriority(IDirectDrawSurfaceImpl* This, LPDWORD lpdwPrio)
{
    TRACE("NOT_IMPLEMENTED -> %s(This=%p)\n", __FUNCTION__, This);
    HRESULT ret = DDERR_INVALIDOBJECT;
    TRACE("NOT_IMPLEMENTED <- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDrawSurface__SetLOD(IDirectDrawSurfaceImpl* This, DWORD dwLod)
{
    TRACE("NOT_IMPLEMENTED -> %s(This=%p)\n", __FUNCTION__, This);
    HRESULT ret = DDERR_INVALIDOBJECT;
    TRACE("NOT_IMPLEMENTED <- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDrawSurface__GetLOD(IDirectDrawSurfaceImpl* This, LPDWORD lpdwLod)
{
    TRACE("NOT_IMPLEMENTED -> %s(This=%p)\n", __FUNCTION__, This);
    HRESULT ret = DDERR_INVALIDOBJECT;
    TRACE("NOT_IMPLEMENTED <- %s\n", __FUNCTION__);
    return ret;
}

struct IDirectDrawSurfaceImplVtbl g_dds_vtbl =
{
    /*** IUnknown methods ***/
    IDirectDrawSurface__QueryInterface,
    IDirectDrawSurface__AddRef,
    IDirectDrawSurface__Release,
    /*** IDirectDrawSurface methods ***/
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
    IDirectDrawSurface__UpdateOverlayZOrder,
    /*** Added in the v2 interface ***/
    IDirectDrawSurface__GetDDInterface,
    IDirectDrawSurface__PageLock,
    IDirectDrawSurface__PageUnlock,
    /*** Added in the v3 interface ***/
    IDirectDrawSurface__SetSurfaceDesc,
    /*** Added in the v4 interface ***/
    IDirectDrawSurface__SetPrivateData,
    IDirectDrawSurface__GetPrivateData,
    IDirectDrawSurface__FreePrivateData,
    IDirectDrawSurface__GetUniquenessValue,
    IDirectDrawSurface__ChangeUniquenessValue,
    /*** Added in the v7 interface ***/
    IDirectDrawSurface__SetPriority,
    IDirectDrawSurface__GetPriority,
    IDirectDrawSurface__SetLOD,
    IDirectDrawSurface__GetLOD
};
