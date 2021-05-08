#ifndef IDIRECTDRAWSURFACE_H
#define IDIRECTDRAWSURFACE_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "IDirectDrawPalette.h"
#include "IDirectDrawClipper.h"
#include "ddraw.h"


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
};

extern struct IDirectDrawSurfaceImplVtbl g_dds_vtbl;

#endif
