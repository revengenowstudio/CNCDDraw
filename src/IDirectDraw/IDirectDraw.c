#include <initguid.h>
#include "IDirectDraw.h"
#include "IDirect3D.h"
#include "IAMMediaStream.h"
#include "dd.h"
#include "ddclipper.h"
#include "ddpalette.h"
#include "ddsurface.h"
#include "debug.h"


HRESULT __stdcall IDirectDraw__QueryInterface(IDirectDrawImpl* This, REFIID riid, void** obj)
{
    dprintf("-> %s(This=%p, riid=%08X, obj=%p)\n", __FUNCTION__, This, (unsigned int)riid, obj);
    HRESULT ret = DDERR_UNSUPPORTED;

    if (riid)
    {
        if (IsEqualGUID(&IID_IDirectDraw2, riid) ||
            IsEqualGUID(&IID_IDirectDraw4, riid) ||
            IsEqualGUID(&IID_IDirectDraw7, riid))
        {
            IDirectDrawImpl* dd = (IDirectDrawImpl*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(IDirectDrawImpl));
            
            dprintf("     GUID = %08X (IID_IDirectDrawX), ddraw = %p\n", ((GUID*)riid)->Data1, dd);

            dd->lpVtbl = &g_dd_vtblx;
            IDirectDraw_AddRef(dd);

            *obj = dd;

            ret = S_OK;
        }
        else if (IsEqualGUID(&IID_IDirectDraw, riid))
        {
            IDirectDrawImpl* dd = (IDirectDrawImpl*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(IDirectDrawImpl));
            
            dprintf("     GUID = %08X (IID_IDirectDraw), ddraw = %p\n", ((GUID*)riid)->Data1, dd);

            dd->lpVtbl = &g_dd_vtbl1;
            IDirectDraw_AddRef(dd);

            *obj = dd;

            ret = S_OK;
        }
        else if (IsEqualGUID(&IID_IDirect3D, riid))
        {
            IDirect3DImpl* d3d = (IDirect3DImpl*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(IDirect3DImpl));
            
            dprintf("     GUID = %08X (IID_IDirect3D), d3d = %p\n", ((GUID*)riid)->Data1, d3d);
            
            d3d->lpVtbl = &g_d3d_vtbl;
            d3d->lpVtbl->AddRef(d3d);

            *obj = d3d;

            ret = S_OK;
        }
        else if (IsEqualGUID(&IID_IDirect3D2, riid))
        {
            IDirect3D2Impl* d3d = (IDirect3D2Impl*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(IDirect3D2Impl));

            dprintf("     GUID = %08X (IID_IDirect3D2), d3d = %p\n", ((GUID*)riid)->Data1, d3d);

            d3d->lpVtbl = &g_d3d2_vtbl;
            d3d->lpVtbl->AddRef(d3d);

            *obj = d3d;

            ret = S_OK;
        }
        else if (IsEqualGUID(&IID_IDirect3D3, riid))
        {
            IDirect3D3Impl* d3d = (IDirect3D3Impl*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(IDirect3D3Impl));

            dprintf("     GUID = %08X (IID_IDirect3D3), d3d = %p\n", ((GUID*)riid)->Data1, d3d);

            d3d->lpVtbl = &g_d3d3_vtbl;
            d3d->lpVtbl->AddRef(d3d);

            *obj = d3d;

            ret = S_OK;
        }
        else if (IsEqualGUID(&IID_IDirect3D7, riid))
        {
            IDirect3D7Impl* d3d = (IDirect3D7Impl*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(IDirect3D7Impl));

            dprintf("     GUID = %08X (IID_IDirect3D7), d3d = %p\n", ((GUID*)riid)->Data1, d3d);

            d3d->lpVtbl = &g_d3d7_vtbl;
            d3d->lpVtbl->AddRef(d3d);

            *obj = d3d;

            ret = S_OK;
        }
        /*
        else if (
            !g_ddraw->passthrough && 
            (IsEqualGUID(&IID_IMediaStream, riid) || IsEqualGUID(&IID_IAMMediaStream, riid)))
        {
            IAMMediaStreamImpl* ms = 
                (IAMMediaStreamImpl*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(IAMMediaStreamImpl));

            dprintf("     GUID = %08X (IID_IXXMediaStream), ms = %p\n", ((GUID*)riid)->Data1, ms);

            ms->lpVtbl = &g_ms_vtbl;
            ms->lpVtbl->AddRef(ms);

            *obj = ms;

            ret = S_OK;
        }
        */
        else
        {
            dprintf("NOT_IMPLEMENTED     GUID = %08X\n", ((GUID*)riid)->Data1);

            if (!g_ddraw->real_dll)
                g_ddraw->real_dll = LoadLibrary("system32\\ddraw.dll");

            if (g_ddraw->real_dll && !g_ddraw->DirectDrawCreate)
                g_ddraw->DirectDrawCreate = (void*)GetProcAddress(g_ddraw->real_dll, "DirectDrawCreate");

            if (g_ddraw->DirectDrawCreate == DirectDrawCreate)
                g_ddraw->DirectDrawCreate = NULL;

            if (!g_ddraw->real_dd && g_ddraw->DirectDrawCreate)
                g_ddraw->DirectDrawCreate(NULL, &g_ddraw->real_dd, NULL);

            if (g_ddraw->real_dd)
                return IDirectDraw_QueryInterface(g_ddraw->real_dd, riid, obj);

            ret = E_NOINTERFACE;
        }
    }

    dprintf("<- %s\n", __FUNCTION__);
    return ret;
}

ULONG __stdcall IDirectDraw__AddRef(IDirectDrawImpl* This)
{
    dprintf("-> %s(This=%p)\n", __FUNCTION__, This);
    ULONG ret = ++This->ref;
    ULONG glob_ref = dd_AddRef();
    dprintf("<- %s(This ref=%u, global ref=%u)\n", __FUNCTION__, ret, glob_ref);
    return ret;
}

ULONG __stdcall IDirectDraw__Release(IDirectDrawImpl* This)
{
    dprintf("-> %s(This=%p)\n", __FUNCTION__, This);

    ULONG ret = --This->ref;

    if (This->ref == 0)
    {
        dprintf("     Released (%p)\n", This);

        HeapFree(GetProcessHeap(), 0, This);
    }

    ULONG glob_ref = dd_Release();

    dprintf("<- %s(This ref=%u, global ref=%u)\n", __FUNCTION__, ret, glob_ref);
    return ret;
}

HRESULT __stdcall IDirectDraw__Compact(IDirectDrawImpl* This)
{
    dprintf("NOT_IMPLEMENTED -> %s(This=%p)\n", __FUNCTION__, This);
    HRESULT ret = DD_OK;
    dprintf("NOT_IMPLEMENTED <- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDraw__CreateClipper(IDirectDrawImpl* This, DWORD dwFlags, LPDIRECTDRAWCLIPPER FAR* lplpDDClipper, IUnknown FAR* pUnkOuter)
{
    dprintf("-> %s(This=%p, dwFlags=%08X, DDClipper=%p, unkOuter=%p)\n", __FUNCTION__, This, (int)dwFlags, lplpDDClipper, pUnkOuter);
    HRESULT ret = dd_CreateClipper(dwFlags, lplpDDClipper, pUnkOuter);
    dprintf("<- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDraw__CreatePalette(IDirectDrawImpl* This, DWORD dwFlags, LPPALETTEENTRY lpDDColorArray, LPDIRECTDRAWPALETTE FAR* lpDDPalette, IUnknown FAR* unkOuter)
{
    dprintf("-> %s(This=%p, dwFlags=%08X, DDColorArray=%p, DDPalette=%p, unkOuter=%p)\n", __FUNCTION__, This, (int)dwFlags, lpDDColorArray, lpDDPalette, unkOuter);
    HRESULT ret = dd_CreatePalette(dwFlags, lpDDColorArray, lpDDPalette, unkOuter);
    dprintf("<- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDraw__CreateSurface(IDirectDrawImpl* This, LPDDSURFACEDESC lpDDSurfaceDesc, LPDIRECTDRAWSURFACE FAR* lpDDSurface, IUnknown FAR* unkOuter)
{
    dprintf("-> %s(This=%p, lpDDSurfaceDesc=%p, lpDDSurface=%p, unkOuter=%p)\n", __FUNCTION__, This, lpDDSurfaceDesc, lpDDSurface, unkOuter);
    HRESULT ret = dd_CreateSurface(This, lpDDSurfaceDesc, lpDDSurface, unkOuter);
    dprintf("<- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDraw__DuplicateSurface(IDirectDrawImpl* This, LPDIRECTDRAWSURFACE src, LPDIRECTDRAWSURFACE* dest)
{
    dprintf("NOT_IMPLEMENTED -> %s(This=%p)\n", __FUNCTION__, This);
    HRESULT ret = DD_OK;
    dprintf("NOT_IMPLEMENTED <- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDraw__EnumDisplayModes(IDirectDrawImpl* This, DWORD dwFlags, LPDDSURFACEDESC lpDDSurfaceDesc, LPVOID lpContext, LPDDENUMMODESCALLBACK lpEnumModesCallback)
{
    dprintf("-> %s(This=%p, dwFlags=%08X, lpDDSurfaceDesc=%p, lpContext=%p, lpEnumModesCallback=%p)\n", __FUNCTION__, This, (unsigned int)dwFlags, lpDDSurfaceDesc, lpContext, lpEnumModesCallback);
    HRESULT ret = dd_EnumDisplayModes(dwFlags, lpDDSurfaceDesc, lpContext, lpEnumModesCallback);
    dprintf("<- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDraw__EnumSurfaces(IDirectDrawImpl* This, DWORD a, LPDDSURFACEDESC b, LPVOID c, LPDDENUMSURFACESCALLBACK d)
{
    dprintf("NOT_IMPLEMENTED -> %s(This=%p)\n", __FUNCTION__, This);
    HRESULT ret = DD_OK;
    dprintf("NOT_IMPLEMENTED <- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDraw__FlipToGDISurface(IDirectDrawImpl* This)
{
    dprintf("NOT_IMPLEMENTED -> %s(This=%p)\n", __FUNCTION__, This);
    HRESULT ret = DD_OK;
    dprintf("NOT_IMPLEMENTED <- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDraw__GetCaps(IDirectDrawImpl* This, LPDDCAPS lpDDDriverCaps, LPDDCAPS lpDDEmulCaps)
{
    dprintf("-> %s(This=%p, lpDDDriverCaps=%p, lpDDEmulCaps=%p)\n", __FUNCTION__, This, lpDDDriverCaps, lpDDEmulCaps);
    HRESULT ret = dd_GetCaps(lpDDDriverCaps, lpDDEmulCaps);
    dprintf("<- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDraw__GetDisplayMode(IDirectDrawImpl* This, LPDDSURFACEDESC lpDDSurfaceDesc)
{
    dprintf("-> %s(This=%p)\n", __FUNCTION__, This);
    HRESULT ret = dd_GetDisplayMode(lpDDSurfaceDesc);
    dprintf("<- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDraw__GetFourCCCodes(IDirectDrawImpl* This, LPDWORD a, LPDWORD b)
{
    dprintf("NOT_IMPLEMENTED -> %s(This=%p)\n", __FUNCTION__, This);
    HRESULT ret = DDERR_INVALIDOBJECT;
    dprintf("NOT_IMPLEMENTED <- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDraw__GetGDISurface(IDirectDrawImpl* This, LPDIRECTDRAWSURFACE* a)
{
    dprintf("NOT_IMPLEMENTED -> %s(This=%p)\n", __FUNCTION__, This);
    HRESULT ret = DD_OK;
    dprintf("NOT_IMPLEMENTED <- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDraw__GetMonitorFrequency(IDirectDrawImpl* This, LPDWORD lpdwFreq)
{
    dprintf("-> %s(This=%p)\n", __FUNCTION__, This);
    HRESULT ret = dd_GetMonitorFrequency(lpdwFreq);
    dprintf("<- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDraw__GetScanLine(IDirectDrawImpl* This, LPDWORD a)
{
    dprintf("NOT_IMPLEMENTED -> %s(This=%p)\n", __FUNCTION__, This);
    HRESULT ret = DDERR_UNSUPPORTED;
    dprintf("NOT_IMPLEMENTED <- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDraw__GetVerticalBlankStatus(IDirectDrawImpl* This, LPBOOL lpbIsInVB)
{
    dprintf("-> %s(This=%p, lpbIsInVB=%p)\n", __FUNCTION__, This, lpbIsInVB);
    HRESULT ret = dd_GetVerticalBlankStatus(lpbIsInVB);
    dprintf("<- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDraw__Initialize(IDirectDrawImpl* This, GUID* a)
{
    dprintf("NOT_IMPLEMENTED -> %s(This=%p)\n", __FUNCTION__, This);
    HRESULT ret = DD_OK;
    dprintf("NOT_IMPLEMENTED <- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDraw__RestoreDisplayMode(IDirectDrawImpl* This)
{
    dprintf("-> %s(This=%p)\n", __FUNCTION__, This);
    HRESULT ret = dd_RestoreDisplayMode();
    dprintf("<- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDraw__SetCooperativeLevel(IDirectDrawImpl* This, HWND hwnd, DWORD dwFlags)
{
    dprintf("-> %s(This=%p, hwnd=0x%08X, dwFlags=0x%08X)\n", __FUNCTION__, This, (unsigned int)hwnd, (unsigned int)dwFlags);
    HRESULT ret = dd_SetCooperativeLevel(hwnd, dwFlags);
    dprintf("<- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDraw__SetDisplayMode(IDirectDrawImpl* This, DWORD width, DWORD height, DWORD bpp)
{
    dprintf("-> %s(This=%p, width=%d, height=%d, bpp=%d)\n", __FUNCTION__, This, (unsigned int)width, (unsigned int)height, (unsigned int)bpp);
    HRESULT ret = dd_SetDisplayMode(width, height, bpp, TRUE);
    dprintf("<- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDraw__SetDisplayModeX(IDirectDrawImpl* This, DWORD width, DWORD height, DWORD bpp, DWORD refreshRate, DWORD flags)
{
    dprintf("-> %s(This=%p, width=%d, height=%d, bpp=%d, refreshRate=%d, flags=%d)\n", __FUNCTION__, This, (unsigned int)width, (unsigned int)height, (unsigned int)bpp, (unsigned int)refreshRate, (unsigned int)flags);
    HRESULT ret = dd_SetDisplayMode(width, height, bpp, TRUE);
    dprintf("<- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDraw__WaitForVerticalBlank(IDirectDrawImpl* This, DWORD dwFlags, HANDLE h)
{
    dprintfex("-> %s(This=%p, flags=%08X, handle=%p)\n", __FUNCTION__, This, dwFlags, h);
    HRESULT ret = dd_WaitForVerticalBlank(dwFlags, h);
    dprintfex("<- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDraw__GetAvailableVidMem(IDirectDrawImpl* This, void* lpDDCaps, LPDWORD lpdwTotal, LPDWORD lpdwFree)
{
    dprintf("-> %s(This=%p, lpDDCaps=%p, lpdwTotal=%p, lpdwFree=%p)\n", __FUNCTION__, This, lpDDCaps, lpdwTotal, lpdwFree);
    HRESULT ret = dd_GetAvailableVidMem(lpDDCaps, lpdwTotal, lpdwFree);
    dprintf("<- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDraw__GetSurfaceFromDC(IDirectDrawImpl* This, HDC hdc, void* lplpDDSurface)
{
    dprintf("NOT_IMPLEMENTED -> %s(This=%p)\n", __FUNCTION__, This);
    HRESULT ret = DDERR_GENERIC;
    dprintf("NOT_IMPLEMENTED <- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDraw__RestoreAllSurfaces(IDirectDrawImpl* This)
{
    dprintf("NOT_IMPLEMENTED -> %s(This=%p)\n", __FUNCTION__, This);
    HRESULT ret = DD_OK;
    dprintf("NOT_IMPLEMENTED <- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDraw__TestCooperativeLevel(IDirectDrawImpl* This)
{
    dprintfex("-> %s(This=%p)\n", __FUNCTION__, This);
    HRESULT ret = DD_OK;
    dprintfex("<- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDraw__GetDeviceIdentifier(IDirectDrawImpl* This, void* pDDDI, DWORD dwFlags)
{
    dprintf("NOT_IMPLEMENTED -> %s(This=%p)\n", __FUNCTION__, This);
    HRESULT ret = DDERR_INVALIDPARAMS;
    dprintf("NOT_IMPLEMENTED <- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDraw__StartModeTest(IDirectDrawImpl* This, LPSIZE pModes, DWORD dwNumModes, DWORD dwFlags)
{
    dprintf("NOT_IMPLEMENTED -> %s(This=%p)\n", __FUNCTION__, This);
    HRESULT ret = DDERR_CURRENTLYNOTAVAIL;
    dprintf("NOT_IMPLEMENTED <- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDraw__EvaluateMode(IDirectDrawImpl* This, DWORD dwFlags, DWORD* pTimeout)
{
    dprintf("NOT_IMPLEMENTED -> %s(This=%p)\n", __FUNCTION__, This);
    HRESULT ret = DDERR_INVALIDOBJECT;
    dprintf("NOT_IMPLEMENTED <- %s\n", __FUNCTION__);
    return ret;
}

struct IDirectDrawImplVtbl g_dd_vtbl1 =
{
    /* IUnknown */
    IDirectDraw__QueryInterface,
    IDirectDraw__AddRef,
    IDirectDraw__Release,
    /* IDirectDrawImpl */
    IDirectDraw__Compact,
    IDirectDraw__CreateClipper,
    IDirectDraw__CreatePalette,
    IDirectDraw__CreateSurface,
    IDirectDraw__DuplicateSurface,
    IDirectDraw__EnumDisplayModes,
    IDirectDraw__EnumSurfaces,
    IDirectDraw__FlipToGDISurface,
    IDirectDraw__GetCaps,
    IDirectDraw__GetDisplayMode,
    IDirectDraw__GetFourCCCodes,
    IDirectDraw__GetGDISurface,
    IDirectDraw__GetMonitorFrequency,
    IDirectDraw__GetScanLine,
    IDirectDraw__GetVerticalBlankStatus,
    IDirectDraw__Initialize,
    IDirectDraw__RestoreDisplayMode,
    IDirectDraw__SetCooperativeLevel,
    {IDirectDraw__SetDisplayMode},
    IDirectDraw__WaitForVerticalBlank,
    // v2
    IDirectDraw__GetAvailableVidMem,
    // v4
    IDirectDraw__GetSurfaceFromDC,
    IDirectDraw__RestoreAllSurfaces,
    IDirectDraw__TestCooperativeLevel,
    IDirectDraw__GetDeviceIdentifier,
    // v7
    IDirectDraw__StartModeTest,
    IDirectDraw__EvaluateMode,
};

struct IDirectDrawImplVtbl g_dd_vtblx =
{
    /* IUnknown */
    IDirectDraw__QueryInterface,
    IDirectDraw__AddRef,
    IDirectDraw__Release,
    /* IDirectDrawImpl */
    IDirectDraw__Compact,
    IDirectDraw__CreateClipper,
    IDirectDraw__CreatePalette,
    IDirectDraw__CreateSurface,
    IDirectDraw__DuplicateSurface,
    IDirectDraw__EnumDisplayModes,
    IDirectDraw__EnumSurfaces,
    IDirectDraw__FlipToGDISurface,
    IDirectDraw__GetCaps,
    IDirectDraw__GetDisplayMode,
    IDirectDraw__GetFourCCCodes,
    IDirectDraw__GetGDISurface,
    IDirectDraw__GetMonitorFrequency,
    IDirectDraw__GetScanLine,
    IDirectDraw__GetVerticalBlankStatus,
    IDirectDraw__Initialize,
    IDirectDraw__RestoreDisplayMode,
    IDirectDraw__SetCooperativeLevel,
    {IDirectDraw__SetDisplayModeX},
    IDirectDraw__WaitForVerticalBlank,
    // v2
    IDirectDraw__GetAvailableVidMem,
    // v4
    IDirectDraw__GetSurfaceFromDC,
    IDirectDraw__RestoreAllSurfaces,
    IDirectDraw__TestCooperativeLevel,
    IDirectDraw__GetDeviceIdentifier,
    // v7
    IDirectDraw__StartModeTest,
    IDirectDraw__EvaluateMode,
};
