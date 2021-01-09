#include "IDirectDrawGammaControl.h"
#include "debug.h"

HRESULT __stdcall IDirectDrawGammaControl__QueryInterface(IDirectDrawGammaControlImpl *This, REFIID riid, void **obj)
{
    dprintf("NOT_IMPLEMENTED -> %s(This=%p, riid=%08X, obj=%p)\n", __FUNCTION__, This, (unsigned int)riid, obj);
    HRESULT ret = S_OK;
    dprintf("NOT_IMPLEMENTED <- %s\n", __FUNCTION__);
    return ret;
}

ULONG __stdcall IDirectDrawGammaControl__AddRef(IDirectDrawGammaControlImpl *This)
{
    dprintf("-> %s(This=%p)\n", __FUNCTION__, This);
    ULONG ret = ++This->ref;
    dprintf("<- %s(This ref=%u)\n", __FUNCTION__, ret);
    return ret;
}

ULONG __stdcall IDirectDrawGammaControl__Release(IDirectDrawGammaControlImpl *This)
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

HRESULT __stdcall IDirectDrawGammaControl__GetGammaRamp(IDirectDrawGammaControlImpl *This, DWORD dwFlags, void *lpRampData)
{
    dprintf("NOT_IMPLEMENTED -> %s(This=%p)\n", __FUNCTION__, This);
    HRESULT ret = DDERR_EXCEPTION;
    dprintf("NOT_IMPLEMENTED <- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDrawGammaControl__SetGammaRamp(IDirectDrawGammaControlImpl *This, DWORD dwFlags, void *lpRampData)
{
    dprintf("NOT_IMPLEMENTED -> %s(This=%p)\n", __FUNCTION__, This);
    HRESULT ret = DDERR_EXCEPTION;
    dprintf("NOT_IMPLEMENTED <- %s\n", __FUNCTION__);
    return ret;
}

struct IDirectDrawGammaControlImplVtbl g_ddgc_vtbl =
{
    /* IUnknown */
    IDirectDrawGammaControl__QueryInterface,
    IDirectDrawGammaControl__AddRef,
    IDirectDrawGammaControl__Release,
    /* IDirectDrawGammaControl */
    IDirectDrawGammaControl__GetGammaRamp,
    IDirectDrawGammaControl__SetGammaRamp,
};
