#ifndef IDIRECTDRAWSURFACE_H
#define IDIRECTDRAWSURFACE_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "IDirectDrawPalette.h"
#include "IDirectDrawClipper.h"
#include "IDirectDraw.h"
#include "ddraw.h"


DEFINE_GUID(IID_IDirectDrawSurface4, 0x0B2B8630, 0xAD35, 0x11D0, 0x8E, 0xA6, 0x00, 0x60, 0x97, 0x97, 0xEA, 0x5B);
DEFINE_GUID(IID_IDirectDrawSurface7, 0x06675a80, 0x3b9b, 0x11d2, 0xb9, 0x2f, 0x00, 0x60, 0x97, 0x97, 0xea, 0x5b);

struct IDirectDrawSurfaceImpl;
struct IDirectDrawSurfaceImplVtbl;

typedef struct IDirectDrawSurfaceImpl
{
    struct IDirectDrawSurfaceImplVtbl *lpVtbl;

    ULONG ref;

    DWORD width;
    DWORD height;
    DWORD bpp;
    DWORD flags;
    DWORD caps;

    IDirectDrawPaletteImpl* palette;

    void *surface;
    DWORD l_pitch;
    DWORD lx_pitch;

    PBITMAPINFO bmi;
    HBITMAP bitmap;
    HDC hdc;
    DDCOLORKEY color_key;
    DWORD last_flip_tick;
    DWORD last_blt_tick;

    struct IDirectDrawSurfaceImpl* backbuffer;
    struct IDirectDrawClipperImpl* clipper;
    struct IDirectDrawImpl* ddraw;

} IDirectDrawSurfaceImpl;

typedef struct IDirectDrawSurfaceImplVtbl IDirectDrawSurfaceImplVtbl;

struct IDirectDrawSurfaceImplVtbl
{
    /* IUnknown */
    HRESULT (__stdcall *QueryInterface)(IDirectDrawSurfaceImpl*, REFIID, void**);
    ULONG (__stdcall *AddRef)(IDirectDrawSurfaceImpl*);
    ULONG (__stdcall *Release)(IDirectDrawSurfaceImpl*);

    /* IDirectDrawSurface */
    HRESULT (__stdcall *AddAttachedSurface)(IDirectDrawSurfaceImpl*, LPDIRECTDRAWSURFACE);
    HRESULT (__stdcall *AddOverlayDirtyRect)(IDirectDrawSurfaceImpl*, LPRECT);
    HRESULT (__stdcall *Blt)(IDirectDrawSurfaceImpl*, LPRECT,LPDIRECTDRAWSURFACE, LPRECT,DWORD, LPDDBLTFX);
    HRESULT (__stdcall *BltBatch)(IDirectDrawSurfaceImpl*, LPDDBLTBATCH, DWORD, DWORD );
    HRESULT (__stdcall *BltFast)(IDirectDrawSurfaceImpl*, DWORD,DWORD,LPDIRECTDRAWSURFACE, LPRECT,DWORD);
    HRESULT (__stdcall *DeleteAttachedSurface)(IDirectDrawSurfaceImpl*, DWORD,LPDIRECTDRAWSURFACE);
    HRESULT (__stdcall *EnumAttachedSurfaces)(IDirectDrawSurfaceImpl*, LPVOID,LPDDENUMSURFACESCALLBACK);
    HRESULT (__stdcall *EnumOverlayZOrders)(IDirectDrawSurfaceImpl*, DWORD,LPVOID,LPDDENUMSURFACESCALLBACK);
    HRESULT (__stdcall *Flip)(IDirectDrawSurfaceImpl*, LPDIRECTDRAWSURFACE, DWORD);
    HRESULT (__stdcall *GetAttachedSurface)(IDirectDrawSurfaceImpl*, LPDDSCAPS, LPDIRECTDRAWSURFACE FAR *);
    HRESULT (__stdcall *GetBltStatus)(IDirectDrawSurfaceImpl*, DWORD);
    HRESULT (__stdcall *GetCaps)(IDirectDrawSurfaceImpl*, LPDDSCAPS);
    HRESULT (__stdcall *GetClipper)(IDirectDrawSurfaceImpl*, LPDIRECTDRAWCLIPPER FAR*);
    HRESULT (__stdcall *GetColorKey)(IDirectDrawSurfaceImpl*, DWORD, LPDDCOLORKEY);
    HRESULT (__stdcall *GetDC)(IDirectDrawSurfaceImpl*, HDC FAR *);
    HRESULT (__stdcall *GetFlipStatus)(IDirectDrawSurfaceImpl*, DWORD);
    HRESULT (__stdcall *GetOverlayPosition)(IDirectDrawSurfaceImpl*, LPLONG, LPLONG );
    HRESULT (__stdcall *GetPalette)(IDirectDrawSurfaceImpl*, LPDIRECTDRAWPALETTE FAR*);
    HRESULT (__stdcall *GetPixelFormat)(IDirectDrawSurfaceImpl*, LPDDPIXELFORMAT);
    HRESULT (__stdcall *GetSurfaceDesc)(IDirectDrawSurfaceImpl*, LPDDSURFACEDESC);
    HRESULT (__stdcall *Initialize)(IDirectDrawSurfaceImpl*, LPDIRECTDRAW, LPDDSURFACEDESC);
    HRESULT (__stdcall *IsLost)(IDirectDrawSurfaceImpl*);
    HRESULT (__stdcall *Lock)(IDirectDrawSurfaceImpl*, LPRECT,LPDDSURFACEDESC,DWORD,HANDLE);
    HRESULT (__stdcall *ReleaseDC)(IDirectDrawSurfaceImpl*, HDC);
    HRESULT (__stdcall *Restore)(IDirectDrawSurfaceImpl*);
    HRESULT (__stdcall *SetClipper)(IDirectDrawSurfaceImpl*, LPDIRECTDRAWCLIPPER);
    HRESULT (__stdcall *SetColorKey)(IDirectDrawSurfaceImpl*, DWORD, LPDDCOLORKEY);
    HRESULT (__stdcall *SetOverlayPosition)(IDirectDrawSurfaceImpl*, LONG, LONG );
    HRESULT (__stdcall *SetPalette)(IDirectDrawSurfaceImpl*, LPDIRECTDRAWPALETTE);
    HRESULT (__stdcall *Unlock)(IDirectDrawSurfaceImpl*, LPVOID);
    HRESULT (__stdcall *UpdateOverlay)(IDirectDrawSurfaceImpl*, LPRECT, LPDIRECTDRAWSURFACE,LPRECT,DWORD, LPDDOVERLAYFX);
    HRESULT (__stdcall *UpdateOverlayDisplay)(IDirectDrawSurfaceImpl*, DWORD);
    HRESULT (__stdcall *UpdateOverlayZOrder)(IDirectDrawSurfaceImpl*, DWORD, LPDIRECTDRAWSURFACE);
    // v2
    HRESULT (__stdcall *GetDDInterface)(IDirectDrawSurfaceImpl*, LPVOID*);
    HRESULT (__stdcall *PageLock)(IDirectDrawSurfaceImpl*, DWORD);
    HRESULT (__stdcall *PageUnlock)(IDirectDrawSurfaceImpl*, DWORD);
    // v3
    HRESULT (__stdcall *SetSurfaceDesc)(IDirectDrawSurfaceImpl*, LPDDSURFACEDESC, DWORD);
    // v4
    HRESULT (__stdcall *SetPrivateData)(IDirectDrawSurfaceImpl*, REFGUID, LPVOID, DWORD, DWORD);
    HRESULT (__stdcall *GetPrivateData)(IDirectDrawSurfaceImpl*, REFGUID, LPVOID, LPDWORD);
    HRESULT (__stdcall *FreePrivateData)(IDirectDrawSurfaceImpl*, REFGUID);
    HRESULT (__stdcall *GetUniquenessValue)(IDirectDrawSurfaceImpl*, LPDWORD);
    HRESULT (__stdcall *ChangeUniquenessValue)(IDirectDrawSurfaceImpl*);
    // v7
    HRESULT (__stdcall *SetPriority)(IDirectDrawSurfaceImpl*, DWORD);
    HRESULT (__stdcall *GetPriority)(IDirectDrawSurfaceImpl*, LPDWORD);
    HRESULT (__stdcall *SetLOD)(IDirectDrawSurfaceImpl*, DWORD);
    HRESULT (__stdcall *GetLOD)(IDirectDrawSurfaceImpl*, LPDWORD);
};

extern struct IDirectDrawSurfaceImplVtbl g_dds_vtbl;

#endif
