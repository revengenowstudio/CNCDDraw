#include "IDirectDrawClipper.h"
#include "ddclipper.h"
#include "debug.h"

HRESULT __stdcall IDirectDrawClipper__QueryInterface(IDirectDrawClipperImpl *This, REFIID riid, void **obj)
{
    dprintf("NOT_IMPLEMENTED -> %s(This=%p, riid=%08X, obj=%p)\n", __FUNCTION__, This, (unsigned int)riid, obj);
    HRESULT ret = S_OK;
    dprintf("NOT_IMPLEMENTED <- %s\n", __FUNCTION__);
    return ret;
}

ULONG __stdcall IDirectDrawClipper__AddRef(IDirectDrawClipperImpl *This)
{
    dprintf("-> %s(This=%p)\n", __FUNCTION__, This);
    ULONG ret = ++This->ref;
    dprintf("<- %s(This ref=%u)\n", __FUNCTION__, ret);
    return ret;
}

ULONG __stdcall IDirectDrawClipper__Release(IDirectDrawClipperImpl *This)
{
    dprintf("-> %s(This=%p)\n", __FUNCTION__, This);

    ULONG ret = --This->ref;

    if (This->ref == 0)
    {
        dprintf("     Released (%p)\n", This);

        HeapFree(GetProcessHeap(), 0, This);
    }

    dprintf("<- %s(This ref=%u)\n", __FUNCTION__, ret);
    return ret;
}

HRESULT __stdcall IDirectDrawClipper__GetClipList(IDirectDrawClipperImpl *This, LPRECT a, LPRGNDATA b, LPDWORD c)
{
    dprintf("NOT_IMPLEMENTED -> %s(This=%p)\n", __FUNCTION__, This);
    HRESULT ret = DD_OK;
    dprintf("NOT_IMPLEMENTED <- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDrawClipper__GetHWnd(IDirectDrawClipperImpl *This, HWND FAR *a)
{
    dprintf("NOT_IMPLEMENTED -> %s(This=%p)\n", __FUNCTION__, This);
    HRESULT ret = DD_OK;
    dprintf("NOT_IMPLEMENTED <- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDrawClipper__Initialize(IDirectDrawClipperImpl *This, LPDIRECTDRAW a, DWORD b)
{
    dprintf("NOT_IMPLEMENTED -> %s(This=%p)\n", __FUNCTION__, This);
    HRESULT ret = DD_OK;
    dprintf("NOT_IMPLEMENTED <- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDrawClipper__IsClipListChanged(IDirectDrawClipperImpl *This, BOOL FAR *a)
{
    dprintf("NOT_IMPLEMENTED -> %s(This=%p)\n", __FUNCTION__, This);
    HRESULT ret = DD_OK;
    dprintf("NOT_IMPLEMENTED <- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDrawClipper__SetClipList(IDirectDrawClipperImpl *This, LPRGNDATA a, DWORD b)
{
    dprintf("NOT_IMPLEMENTED -> %s(This=%p)\n", __FUNCTION__, This);
    HRESULT ret = DD_OK;
    dprintf("NOT_IMPLEMENTED <- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDrawClipper__SetHWnd(IDirectDrawClipperImpl *This, DWORD a, HWND b)
{
    dprintf("NOT_IMPLEMENTED -> %s(This=%p)\n", __FUNCTION__, This);
    HRESULT ret = DD_OK;
    dprintf("NOT_IMPLEMENTED <- %s\n", __FUNCTION__);
    return ret;
}

struct IDirectDrawClipperImplVtbl g_ddc_vtbl =
{
    /* IUnknown */
    IDirectDrawClipper__QueryInterface,
    IDirectDrawClipper__AddRef,
    IDirectDrawClipper__Release,
    /* IDirectDrawClipper */
    IDirectDrawClipper__GetClipList,
    IDirectDrawClipper__GetHWnd,
    IDirectDrawClipper__Initialize,
    IDirectDrawClipper__IsClipListChanged,
    IDirectDrawClipper__SetClipList,
    IDirectDrawClipper__SetHWnd
};
