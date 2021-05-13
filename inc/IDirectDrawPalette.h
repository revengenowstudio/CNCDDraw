#ifndef IDIRECTDRAWPALETTE_H
#define IDIRECTDRAWPALETTE_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "ddraw.h"


struct IDirectDrawPaletteImpl;
struct IDirectDrawPaletteImplVtbl;

typedef struct IDirectDrawPaletteImpl
{
    struct IDirectDrawPaletteImplVtbl* lpVtbl;

    ULONG ref;

    int data_bgr[256];
    RGBQUAD* data_rgb;
    DWORD flags;

} IDirectDrawPaletteImpl;

typedef struct IDirectDrawPaletteImplVtbl IDirectDrawPaletteImplVtbl;

struct IDirectDrawPaletteImplVtbl
{
    /* IUnknown */
    HRESULT(__stdcall* QueryInterface)(IDirectDrawPaletteImpl*, REFIID, void**);
    ULONG(__stdcall* AddRef)(IDirectDrawPaletteImpl*);
    ULONG(__stdcall* Release)(IDirectDrawPaletteImpl*);

    /* IDirectDrawPalette */
    HRESULT(__stdcall* GetCaps)(IDirectDrawPaletteImpl*, LPDWORD);
    HRESULT(__stdcall* GetEntries)(IDirectDrawPaletteImpl*, DWORD, DWORD, DWORD, LPPALETTEENTRY);
    HRESULT(__stdcall* Initialize)(IDirectDrawPaletteImpl*, LPDIRECTDRAW, DWORD, LPPALETTEENTRY);
    HRESULT(__stdcall* SetEntries)(IDirectDrawPaletteImpl*, DWORD, DWORD, DWORD, LPPALETTEENTRY);

};

extern struct IDirectDrawPaletteImplVtbl g_ddp_vtbl;

#endif
