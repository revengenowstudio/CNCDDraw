#include "IDirectDrawPalette.h"
#include "ddpalette.h"
#include "ddsurface.h"
#include "debug.h"


HRESULT __stdcall IDirectDrawPalette__QueryInterface(IDirectDrawPaletteImpl* This, REFIID riid, void** obj)
{
    dprintf("NOT_IMPLEMENTED -> %s(This=%p, riid=%08X, obj=%p)\n", __FUNCTION__, This, (unsigned int)riid, obj);
    HRESULT ret = S_OK;
    dprintf("NOT_IMPLEMENTED <- %s\n", __FUNCTION__);
    return ret;
}

ULONG __stdcall IDirectDrawPalette__AddRef(IDirectDrawPaletteImpl* This)
{
    dprintf("-> %s(This=%p)\n", __FUNCTION__, This);
    ULONG ret = ++This->ref;
    dprintf("<- %s(This ref=%u)\n", __FUNCTION__, ret);
    return ret;
}

ULONG __stdcall IDirectDrawPalette__Release(IDirectDrawPaletteImpl* This)
{
    dprintf("-> %s(This=%p)\n", __FUNCTION__, This);

    ULONG ret = --This->ref;

    if (This->ref == 0)
    {
        dprintf("     Released (%p)\n", This);

        if (g_ddraw)
            g_ddraw->last_freed_palette = This;

        HeapFree(GetProcessHeap(), 0, This);
    }

    dprintf("<- %s(This ref=%u)\n", __FUNCTION__, ret);
    return ret;
}

HRESULT __stdcall IDirectDrawPalette__GetCaps(IDirectDrawPaletteImpl* This, LPDWORD caps)
{
    dprintf("NOT_IMPLEMENTED -> %s(This=%p, caps=%p)\n", __FUNCTION__, This, caps);
    HRESULT ret = DD_OK;
    dprintf("NOT_IMPLEMENTED <- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDrawPalette__GetEntries(IDirectDrawPaletteImpl *This, DWORD dwFlags, DWORD dwBase, DWORD dwNumEntries, LPPALETTEENTRY lpEntries)
{
    dprintf("-> %s(This=%p, dwFlags=%08X, dwBase=%d, dwNumEntries=%d, lpEntries=%p)\n", __FUNCTION__, This, (int)dwFlags, (int)dwBase, (int)dwNumEntries, lpEntries);
    HRESULT ret = ddp_GetEntries(This, dwFlags, dwBase, dwNumEntries, lpEntries);
    dprintf("<- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDrawPalette__Initialize(IDirectDrawPaletteImpl* This, LPDIRECTDRAW lpDD, DWORD dw, LPPALETTEENTRY paent)
{
    dprintf("NOT_IMPLEMENTED -> %s(This=%p)\n", __FUNCTION__, This);
    HRESULT ret = DD_OK;
    dprintf("NOT_IMPLEMENTED <- %s\n", __FUNCTION__);
    return ret;
}

HRESULT __stdcall IDirectDrawPalette__SetEntries(IDirectDrawPaletteImpl *This, DWORD dwFlags, DWORD dwStartingEntry, DWORD dwCount, LPPALETTEENTRY lpEntries)
{
    dprintfex("-> %s(This=%p, dwFlags=%08X, dwStartingEntry=%d, dwCount=%d, lpEntries=%p)\n", __FUNCTION__, This, (int)dwFlags, (int)dwStartingEntry, (int)dwCount, lpEntries);
    HRESULT ret = ddp_SetEntries(This, dwFlags, dwStartingEntry, dwCount, lpEntries);
    dprintfex("<- %s\n", __FUNCTION__);
    return ret;
}

struct IDirectDrawPaletteImplVtbl g_ddp_vtbl =
{
    /* IUnknown */
    IDirectDrawPalette__QueryInterface,
    IDirectDrawPalette__AddRef,
    IDirectDrawPalette__Release,
    /* IDirectDrawPalette */
    IDirectDrawPalette__GetCaps,
    IDirectDrawPalette__GetEntries,
    IDirectDrawPalette__Initialize,
    IDirectDrawPalette__SetEntries
};
