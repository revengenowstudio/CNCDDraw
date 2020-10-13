#ifndef DDSURFACE_H
#define DDSURFACE_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "ddraw.h"
#include "IDirectDrawSurface.h"

// enables redraw via blt/unlock if there wasn't any flip for X ms
#define FLIP_REDRAW_TIMEOUT 1000 / 10

HRESULT dds_AddAttachedSurface(IDirectDrawSurfaceImpl* This, LPDIRECTDRAWSURFACE lpDDSurface);
HRESULT dds_Blt(IDirectDrawSurfaceImpl* This, LPRECT lpDestRect, LPDIRECTDRAWSURFACE lpDDSrcSurface, LPRECT lpSrcRect, DWORD dwFlags, LPDDBLTFX lpDDBltFx);
HRESULT dds_BltFast(IDirectDrawSurfaceImpl* This, DWORD dst_x, DWORD dst_y, LPDIRECTDRAWSURFACE lpDDSrcSurface, LPRECT lpSrcRect, DWORD flags);
HRESULT dds_DeleteAttachedSurface(IDirectDrawSurfaceImpl* This, DWORD dwFlags, LPDIRECTDRAWSURFACE lpDDSurface);
HRESULT dds_GetSurfaceDesc(IDirectDrawSurfaceImpl* This, LPDDSURFACEDESC lpDDSurfaceDesc);
HRESULT dds_EnumAttachedSurfaces(IDirectDrawSurfaceImpl* This, LPVOID lpContext, LPDDENUMSURFACESCALLBACK lpEnumSurfacesCallback);
HRESULT dds_Flip(IDirectDrawSurfaceImpl* This, LPDIRECTDRAWSURFACE surface, DWORD flags);
HRESULT dds_GetAttachedSurface(IDirectDrawSurfaceImpl* This, LPDDSCAPS lpDdsCaps, LPDIRECTDRAWSURFACE FAR* surface);
HRESULT dds_GetCaps(IDirectDrawSurfaceImpl* This, LPDDSCAPS lpDDSCaps);
HRESULT dds_GetColorKey(IDirectDrawSurfaceImpl* This, DWORD flags, LPDDCOLORKEY colorKey);
HRESULT dds_GetDC(IDirectDrawSurfaceImpl* This, HDC FAR* a);
HRESULT dds_GetPalette(IDirectDrawSurfaceImpl* This, LPDIRECTDRAWPALETTE FAR* lplpDDPalette);
HRESULT dds_GetPixelFormat(IDirectDrawSurfaceImpl* This, LPDDPIXELFORMAT ddpfPixelFormat);
HRESULT dds_Lock(IDirectDrawSurfaceImpl* This, LPRECT lpDestRect, LPDDSURFACEDESC lpDDSurfaceDesc, DWORD dwFlags, HANDLE hEvent);
HRESULT dds_SetColorKey(IDirectDrawSurfaceImpl* This, DWORD flags, LPDDCOLORKEY colorKey);
HRESULT dds_SetPalette(IDirectDrawSurfaceImpl* This, LPDIRECTDRAWPALETTE lpDDPalette);
HRESULT dds_Unlock(IDirectDrawSurfaceImpl* This, LPVOID lpRect);
HRESULT dd_CreateSurface(LPDDSURFACEDESC lpDDSurfaceDesc, LPDIRECTDRAWSURFACE FAR* lpDDSurface, IUnknown FAR* unkOuter);


#endif
