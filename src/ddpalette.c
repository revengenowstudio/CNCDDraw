#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "dd.h"
#include "ddpalette.h"
#include "ddsurface.h"
#include "IDirectDrawPalette.h"
#include "debug.h"


HRESULT ddp_GetEntries(
    IDirectDrawPaletteImpl* This,
    DWORD dwFlags,
    DWORD dwBase,
    DWORD dwNumEntries,
    LPPALETTEENTRY lpEntries)
{
    for (int i = dwBase, x = 0; i < dwBase + dwNumEntries; i++, x++)
    {
        lpEntries[x].peRed = This->data_rgb[i].rgbRed;
        lpEntries[x].peGreen = This->data_rgb[i].rgbGreen;
        lpEntries[x].peBlue = This->data_rgb[i].rgbBlue;
    }

    return DD_OK;
}

HRESULT ddp_SetEntries(
    IDirectDrawPaletteImpl* This,
    DWORD dwFlags,
    DWORD dwStartingEntry,
    DWORD dwCount,
    LPPALETTEENTRY lpEntries)
{
    for (int i = dwStartingEntry, x = 0; i < dwStartingEntry + dwCount; i++, x++)
    {
        This->data_bgr[i] = (lpEntries[x].peBlue << 16) | (lpEntries[x].peGreen << 8) | lpEntries[x].peRed;

        This->data_rgb[i].rgbRed = lpEntries[x].peRed;
        This->data_rgb[i].rgbGreen = lpEntries[x].peGreen;
        This->data_rgb[i].rgbBlue = lpEntries[x].peBlue;
        This->data_rgb[i].rgbReserved = 0;
    }

    if (!(This->flags & DDPCAPS_ALLOW256))
    {
        This->data_bgr[0] = 0;
        This->data_bgr[255] = (255 << 16) | (255 << 8) | 255;

        This->data_rgb[0].rgbRed = 0;
        This->data_rgb[0].rgbGreen = 0;
        This->data_rgb[0].rgbBlue = 0;
        This->data_rgb[0].rgbReserved = 0;

        This->data_rgb[255].rgbRed = 255;
        This->data_rgb[255].rgbGreen = 255;
        This->data_rgb[255].rgbBlue = 255;
        This->data_rgb[255].rgbReserved = 0;
    }

    if (g_ddraw->primary && g_ddraw->primary->palette == This && g_ddraw->render.run)
    {
        InterlockedExchange(&g_ddraw->render.palette_updated, TRUE);
        ReleaseSemaphore(g_ddraw->render.sem, 1, NULL);
    }

    return DD_OK;
}

HRESULT dd_CreatePalette(
    DWORD dwFlags,
    LPPALETTEENTRY lpDDColorArray,
    IDirectDrawPaletteImpl** lpDDPalette,
    IUnknown FAR* unkOuter)
{
    if (!lpDDPalette || !lpDDColorArray)
        return DDERR_INVALIDPARAMS;

    IDirectDrawPaletteImpl* p =
        (IDirectDrawPaletteImpl*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(IDirectDrawPaletteImpl));

    TRACE("     palette = %p\n", p);

    p->lpVtbl = &g_ddp_vtbl;
    p->flags = dwFlags;
    ddp_SetEntries(p, dwFlags, 0, 256, lpDDColorArray);
    IDirectDrawPalette_AddRef(p);

    *lpDDPalette = p;

    return DD_OK;
}
