#include "IDirectDrawClipper.h"
#include "ddclipper.h"
#include "debug.h"

HRESULT __stdcall IDirectDrawClipper__QueryInterface(IDirectDrawClipperImpl* This, REFIID riid, LPVOID FAR* ppvObj)
{
    TRACE("NOT_IMPLEMENTED -> %s(This=%p, riid=%08X, ppvObj=%p)\n", __FUNCTION__, This, (unsigned int)riid, ppvObj);
    HRESULT ret = E_NOINTERFACE;
    TRACE("NOT_IMPLEMENTED <- %s\n", __FUNCTION__);
    return ret;
}

ULONG __stdcall IDirectDrawClipper__AddRef(IDirectDrawClipperImpl* This)
{
    TRACE("-> %s(This=%p)\n", __FUNCTION__, This);
    ULONG ret = ++This->ref;
    TRACE("<- %s(This ref=%u)\n", __FUNCTION__, ret);
    return ret;
}

ULONG __stdcall IDirectDrawClipper__Release(IDirectDrawClipperImpl* This)
{
    TRACE("-> %s(This=%p)\n", __FUNCTION__, This);

    ULONG ret = --This->ref;

    if (This->ref == 0)
    {
        TRACE("     Released (%p)\n", This);

        HeapFree(GetProcessHeap(), 0, This);
    }

    TRACE("<- %s(This ref=%u)\n", __FUNCTION__, ret);
    return ret;
}

HRESULT __stdcall IDirectDrawClipper__GetClipList(
    IDirectDrawClipperImpl* This,
    LPRECT lpRect,
    LPRGNDATA lpClipList,
    LPDWORD lpdwSiz)
{
    TRACE("NOT_IMPLEMENTED -> %s(This=%p)\n", __FUNCTION__, This);
    HRESULT ret = DDERR_NOCLIPLIST;
    TRACE("NOT_IMPLEMENTED <- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDrawClipper__GetHWnd(IDirectDrawClipperImpl* This, HWND FAR* lphWnd)
{
    TRACE("NOT_IMPLEMENTED -> %s(This=%p)\n", __FUNCTION__, This);
    HRESULT ret = DDERR_INVALIDOBJECT;
    TRACE("NOT_IMPLEMENTED <- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDrawClipper__Initialize(IDirectDrawClipperImpl* This, LPDIRECTDRAW lpDD, DWORD dwFlags)
{
    TRACE("NOT_IMPLEMENTED -> %s(This=%p)\n", __FUNCTION__, This);
    HRESULT ret = DD_OK;
    TRACE("NOT_IMPLEMENTED <- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDrawClipper__IsClipListChanged(IDirectDrawClipperImpl* This, BOOL FAR* lpbChanged)
{
    TRACE("NOT_IMPLEMENTED -> %s(This=%p)\n", __FUNCTION__, This);
    HRESULT ret = DDERR_INVALIDOBJECT;
    TRACE("NOT_IMPLEMENTED <- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDrawClipper__SetClipList(IDirectDrawClipperImpl* This, LPRGNDATA lpClipList, DWORD dwFlags)
{
    TRACE("NOT_IMPLEMENTED -> %s(This=%p)\n", __FUNCTION__, This);
    HRESULT ret = DD_OK;
    TRACE("NOT_IMPLEMENTED <- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDrawClipper__SetHWnd(IDirectDrawClipperImpl* This, DWORD dwFlags, HWND hWnd)
{
    TRACE("NOT_IMPLEMENTED -> %s(This=%p)\n", __FUNCTION__, This);
    HRESULT ret = DD_OK;
    TRACE("NOT_IMPLEMENTED <- %s\n", __FUNCTION__);
    return ret;
}

struct IDirectDrawClipperImplVtbl g_ddc_vtbl =
{
    /*** IUnknown methods ***/
    IDirectDrawClipper__QueryInterface,
    IDirectDrawClipper__AddRef,
    IDirectDrawClipper__Release,
    /*** IDirectDrawClipper methods ***/
    IDirectDrawClipper__GetClipList,
    IDirectDrawClipper__GetHWnd,
    IDirectDrawClipper__Initialize,
    IDirectDrawClipper__IsClipListChanged,
    IDirectDrawClipper__SetClipList,
    IDirectDrawClipper__SetHWnd
};
