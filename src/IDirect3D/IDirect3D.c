#include "IDirect3D.h"
#include "debug.h"


HRESULT __stdcall IDirect3D__QueryInterface(IDirect3DImpl* This, REFIID riid, void** obj)
{
    dprintf("NOT_IMPLEMENTED -> %s(This=%p, riid=%08X, obj=%p)\n", __FUNCTION__, This, (unsigned int)riid, obj);
    HRESULT ret = E_FAIL;
    dprintf("NOT_IMPLEMENTED <- %s\n", __FUNCTION__);
    return ret;
}

ULONG __stdcall IDirect3D__AddRef(IDirect3DImpl* This)
{
    dprintf("-> %s(This=%p)\n", __FUNCTION__, This);
    ULONG ret = ++This->ref;
    dprintf("<- %s(This ref=%u)\n", __FUNCTION__, ret);
    return ret;
}

ULONG __stdcall IDirect3D__Release(IDirect3DImpl* This)
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

HRESULT __stdcall IDirect3D__Initialize(IDirect3DImpl* This, int a)
{
    dprintf("NOT_IMPLEMENTED -> %s(This=%p)\n", __FUNCTION__, This);
    HRESULT ret = E_FAIL;
    dprintf("NOT_IMPLEMENTED <- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirect3D__EnumDevices(IDirect3DImpl* This, int a, int b)
{
    dprintf("NOT_IMPLEMENTED -> %s(This=%p)\n", __FUNCTION__, This);
    HRESULT ret = E_FAIL;
    dprintf("NOT_IMPLEMENTED <- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirect3D__CreateLight(IDirect3DImpl* This, int a, int b)
{
    dprintf("NOT_IMPLEMENTED -> %s(This=%p)\n", __FUNCTION__, This);
    HRESULT ret = E_FAIL;
    dprintf("NOT_IMPLEMENTED <- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirect3D__CreateMaterial(IDirect3DImpl* This, int a, int b)
{
    dprintf("NOT_IMPLEMENTED -> %s(This=%p)\n", __FUNCTION__, This);
    HRESULT ret = E_FAIL;
    dprintf("NOT_IMPLEMENTED <- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirect3D__CreateViewport(IDirect3DImpl* This, int a, int b)
{
    dprintf("NOT_IMPLEMENTED -> %s(This=%p)\n", __FUNCTION__, This);
    HRESULT ret = E_FAIL;
    dprintf("NOT_IMPLEMENTED <- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirect3D__FindDevice(IDirect3DImpl* This, int a, int b)
{
    dprintf("NOT_IMPLEMENTED -> %s(This=%p)\n", __FUNCTION__, This);
    HRESULT ret = E_FAIL;
    dprintf("NOT_IMPLEMENTED <- %s\n", __FUNCTION__);
    return ret;
}

struct IDirect3DImplVtbl g_d3d_vtbl1 =
{
    /* IUnknown */
    IDirect3D__QueryInterface,
    IDirect3D__AddRef,
    IDirect3D__Release,
    /* IDirect3DImpl */
    IDirect3D__Initialize,
    IDirect3D__EnumDevices,
    IDirect3D__CreateLight,
    IDirect3D__CreateMaterial,
    IDirect3D__CreateViewport,
    IDirect3D__FindDevice,
};
