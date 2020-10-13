#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "dd.h"
#include "ddpalette.h"
#include "ddsurface.h"
#include "IDirectDrawPalette.h"
#include "debug.h"


HRESULT ddp_GetEntries(IDirectDrawPaletteImpl *This, DWORD dwFlags, DWORD dwBase, DWORD dwNumEntries, LPPALETTEENTRY lpEntries)
{
    int i, x;

    for (i = dwBase, x = 0; i < dwBase + dwNumEntries; i++, x++)
    {
        if (This->data_rgb)
        {
            lpEntries[x].peRed = This->data_rgb[i].rgbRed;
            lpEntries[x].peGreen = This->data_rgb[i].rgbGreen;
            lpEntries[x].peBlue = This->data_rgb[i].rgbBlue;
        }
    }

    return DD_OK;
}

HRESULT ddp_SetEntries(IDirectDrawPaletteImpl *This, DWORD dwFlags, DWORD dwStartingEntry, DWORD dwCount, LPPALETTEENTRY lpEntries)
{
    int i, x;

    for (i = dwStartingEntry, x = 0; i < dwStartingEntry + dwCount; i++, x++)
    {
        This->data_bgr[i] = (lpEntries[x].peBlue << 16) | (lpEntries[x].peGreen << 8) | lpEntries[x].peRed;

        if (This->data_rgb)
        {
            This->data_rgb[i].rgbRed = lpEntries[x].peRed;
            This->data_rgb[i].rgbGreen = lpEntries[x].peGreen;
            This->data_rgb[i].rgbBlue = lpEntries[x].peBlue;
            This->data_rgb[i].rgbReserved = 0;
        }
    }

    if (g_ddraw->primary && g_ddraw->primary->palette == This && g_ddraw->render.run)
    {
        InterlockedExchange(&g_ddraw->render.palette_updated, TRUE);
        ReleaseSemaphore(g_ddraw->render.sem, 1, NULL);
    }

    return DD_OK;
}

HRESULT dd_CreatePalette(DWORD dwFlags, LPPALETTEENTRY lpDDColorArray, LPDIRECTDRAWPALETTE FAR * lpDDPalette, IUnknown FAR * unkOuter)
{
    IDirectDrawPaletteImpl *p = (IDirectDrawPaletteImpl *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(IDirectDrawPaletteImpl));
    p->lpVtbl = &g_ddp_vtbl;
    dprintf("     Palette = %p\n", p);
    *lpDDPalette = (LPDIRECTDRAWPALETTE)p;

    ddp_SetEntries(p, dwFlags, 0, 256, lpDDColorArray);

    IDirectDrawPalette_AddRef(p);

    return DD_OK;
}
