#ifndef DDSURFACE_H
#define DDSURFACE_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <ddraw.h>
#include "IDirectDrawSurface.h"
#include "IDirectDraw.h"

/* enables redraw via blt/unlock if there wasn't any flip for X ms */
#define FLIP_REDRAW_TIMEOUT 1000 / 10

HRESULT dds_AddAttachedSurface(IDirectDrawSurfaceImpl* This, IDirectDrawSurfaceImpl* lpDDSurface);
HRESULT dds_Blt(IDirectDrawSurfaceImpl* This, LPRECT lpDestRect, IDirectDrawSurfaceImpl* lpDDSrcSurface, LPRECT lpSrcRect, DWORD dwFlags, LPDDBLTFX lpDDBltFx);
HRESULT dds_BltFast(IDirectDrawSurfaceImpl* This, DWORD dwX, DWORD dwY, IDirectDrawSurfaceImpl* lpDDSrcSurface, LPRECT lpSrcRect, DWORD dwFlags);
HRESULT dds_DeleteAttachedSurface(IDirectDrawSurfaceImpl* This, DWORD dwFlags, IDirectDrawSurfaceImpl* lpDDSurface);
HRESULT dds_EnumAttachedSurfaces(IDirectDrawSurfaceImpl* This, LPVOID lpContext, LPDDENUMSURFACESCALLBACK7 lpEnumSurfacesCallback);
HRESULT dds_Flip(IDirectDrawSurfaceImpl* This, IDirectDrawSurfaceImpl* lpDDSurfaceTargetOverride, DWORD dwFlags);
HRESULT dds_GetAttachedSurface(IDirectDrawSurfaceImpl* This, LPDDSCAPS2 lpDdsCaps, IDirectDrawSurfaceImpl** lpDDsurface);
HRESULT dds_GetCaps(IDirectDrawSurfaceImpl* This, LPDDSCAPS2 lpDDSCaps);
HRESULT dds_GetClipper(IDirectDrawSurfaceImpl* This, IDirectDrawClipperImpl** lpClipper);
HRESULT dds_GetColorKey(IDirectDrawSurfaceImpl* This, DWORD dwFlags, LPDDCOLORKEY lpColorKey);
HRESULT dds_GetDC(IDirectDrawSurfaceImpl* This, HDC FAR* lpHDC);
HRESULT dds_GetPalette(IDirectDrawSurfaceImpl* This, IDirectDrawPaletteImpl** lplpDDPalette);
HRESULT dds_GetPixelFormat(IDirectDrawSurfaceImpl* This, LPDDPIXELFORMAT ddpfPixelFormat);
HRESULT dds_GetSurfaceDesc(IDirectDrawSurfaceImpl* This, LPDDSURFACEDESC2 lpDDSurfaceDesc);
HRESULT dds_Lock(IDirectDrawSurfaceImpl* This, LPRECT lpDestRect, LPDDSURFACEDESC2 lpDDSurfaceDesc, DWORD dwFlags, HANDLE hEvent);
HRESULT dds_ReleaseDC(IDirectDrawSurfaceImpl* This, HDC hDC);
HRESULT dds_SetClipper(IDirectDrawSurfaceImpl* This, IDirectDrawClipperImpl* lpClipper);
HRESULT dds_SetColorKey(IDirectDrawSurfaceImpl* This, DWORD dwFlags, LPDDCOLORKEY lpColorKey);
HRESULT dds_SetPalette(IDirectDrawSurfaceImpl* This, IDirectDrawPaletteImpl* lpDDPalette);
HRESULT dds_Unlock(IDirectDrawSurfaceImpl* This, LPRECT lpRect);
HRESULT dds_GetDDInterface(IDirectDrawSurfaceImpl* This, LPVOID* lplpDD);
void* dds_GetBuffer(IDirectDrawSurfaceImpl* This);
HRESULT dd_CreateSurface(IDirectDrawImpl* This, LPDDSURFACEDESC2 lpDDSurfaceDesc, IDirectDrawSurfaceImpl** lpDDSurface, IUnknown FAR* unkOuter);


#endif
