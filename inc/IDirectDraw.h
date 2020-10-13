#ifndef IDIRECTDRAW_H 
#define IDIRECTDRAW_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "ddraw.h"


DEFINE_GUID(IID_IDirectDraw4, 0x9c59509a, 0x39bd, 0x11d1, 0x8c, 0x4a, 0x00, 0xc0, 0x4f, 0xd9, 0x30, 0xc5);
DEFINE_GUID(IID_IDirectDraw7, 0x15e65ec0, 0x3b9c, 0x11d2, 0xb9, 0x2f, 0x00, 0x60, 0x97, 0x97, 0xea, 0x5b);

extern struct IDirectDrawImplVtbl g_dd_vtbl1;
extern struct IDirectDrawImplVtbl g_dd_vtblx;

struct IDirectDrawImpl;
struct IDirectDrawImplVtbl;

typedef struct IDirectDrawImpl
{
    struct IDirectDrawImplVtbl* lpVtbl;

    ULONG ref;

} IDirectDrawImpl;

typedef struct IDirectDrawImplVtbl IDirectDrawImplVtbl;

struct IDirectDrawImplVtbl
{
    HRESULT(__stdcall* QueryInterface) (IDirectDrawImpl*, const IID* const riid, LPVOID* ppvObj);
    ULONG(__stdcall* AddRef) (IDirectDrawImpl*);
    ULONG(__stdcall* Release) (IDirectDrawImpl*);

    HRESULT(__stdcall* Compact)(IDirectDrawImpl*);
    HRESULT(__stdcall* CreateClipper)(IDirectDrawImpl*, DWORD, LPDIRECTDRAWCLIPPER*, IUnknown*);
    HRESULT(__stdcall* CreatePalette)(IDirectDrawImpl*, DWORD, LPPALETTEENTRY, LPDIRECTDRAWPALETTE*, IUnknown*);
    HRESULT(__stdcall* CreateSurface)(IDirectDrawImpl*, LPDDSURFACEDESC, LPDIRECTDRAWSURFACE*, IUnknown*);
    HRESULT(__stdcall* DuplicateSurface)(IDirectDrawImpl*, LPDIRECTDRAWSURFACE, LPDIRECTDRAWSURFACE*);
    HRESULT(__stdcall* EnumDisplayModes)(IDirectDrawImpl*, DWORD, LPDDSURFACEDESC, LPVOID, LPDDENUMMODESCALLBACK);
    HRESULT(__stdcall* EnumSurfaces)(IDirectDrawImpl*, DWORD, LPDDSURFACEDESC, LPVOID, LPDDENUMSURFACESCALLBACK);
    HRESULT(__stdcall* FlipToGDISurface)(IDirectDrawImpl*);
    HRESULT(__stdcall* GetCaps)(IDirectDrawImpl*, LPDDCAPS, LPDDCAPS);
    HRESULT(__stdcall* GetDisplayMode)(IDirectDrawImpl*, LPDDSURFACEDESC);
    HRESULT(__stdcall* GetFourCCCodes)(IDirectDrawImpl*, LPDWORD, LPDWORD);
    HRESULT(__stdcall* GetGDISurface)(IDirectDrawImpl*, LPDIRECTDRAWSURFACE*);
    HRESULT(__stdcall* GetMonitorFrequency)(IDirectDrawImpl*, LPDWORD);
    HRESULT(__stdcall* GetScanLine)(IDirectDrawImpl*, LPDWORD);
    HRESULT(__stdcall* GetVerticalBlankStatus)(IDirectDrawImpl*, LPBOOL);
    HRESULT(__stdcall* Initialize)(IDirectDrawImpl*, GUID*);
    HRESULT(__stdcall* RestoreDisplayMode)(IDirectDrawImpl*);
    HRESULT(__stdcall* SetCooperativeLevel)(IDirectDrawImpl*, HWND, DWORD);
    union
    {
        LPVOID d;
        HRESULT(__stdcall* SetDisplayMode)(IDirectDrawImpl*, DWORD, DWORD, DWORD);
        HRESULT(__stdcall* SetDisplayModeX)(IDirectDrawImpl*, DWORD, DWORD, DWORD, DWORD, DWORD);
    };
    HRESULT(__stdcall* WaitForVerticalBlank)(IDirectDrawImpl*, DWORD, HANDLE);
    // v2
    HRESULT(__stdcall* GetAvailableVidMem)(IDirectDrawImpl*, void*, LPDWORD, LPDWORD);
    // v4
    HRESULT(__stdcall* GetSurfaceFromDC)(IDirectDrawImpl*, HDC, void*);
    HRESULT(__stdcall* RestoreAllSurfaces)(IDirectDrawImpl*);
    HRESULT(__stdcall* TestCooperativeLevel)(IDirectDrawImpl*);
    HRESULT(__stdcall* GetDeviceIdentifier)(IDirectDrawImpl*, void*, DWORD);
    // v7
    HRESULT(__stdcall* StartModeTest)(IDirectDrawImpl*, LPSIZE, DWORD, DWORD);
    HRESULT(__stdcall* EvaluateMode)(IDirectDrawImpl*, DWORD, DWORD*);
};

#endif
