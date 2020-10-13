#ifndef IDIRECTDRAWCLIPPER_H
#define IDIRECTDRAWCLIPPER_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "ddraw.h"


struct IDirectDrawClipperImpl;
struct IDirectDrawClipperImplVtbl;

typedef struct IDirectDrawClipperImpl
{
    struct IDirectDrawClipperImplVtbl *lpVtbl;

    ULONG ref;

} IDirectDrawClipperImpl;

typedef struct IDirectDrawClipperImplVtbl IDirectDrawClipperImplVtbl;

struct IDirectDrawClipperImplVtbl
{
    /* IUnknown */
    HRESULT (__stdcall *QueryInterface)(IDirectDrawClipperImpl *, REFIID, void **);
    ULONG (__stdcall *AddRef)(IDirectDrawClipperImpl *);
    ULONG (__stdcall *Release)(IDirectDrawClipperImpl *);

    /* IDirectDrawClipper */
    HRESULT (__stdcall *GetClipList)(IDirectDrawClipperImpl *, LPRECT, LPRGNDATA, LPDWORD);
    HRESULT (__stdcall *GetHWnd)(IDirectDrawClipperImpl *, HWND FAR *);
    HRESULT (__stdcall *Initialize)(IDirectDrawClipperImpl *, LPDIRECTDRAW, DWORD);
    HRESULT (__stdcall *IsClipListChanged)(IDirectDrawClipperImpl *, BOOL FAR *);
    HRESULT (__stdcall *SetClipList)(IDirectDrawClipperImpl *, LPRGNDATA,DWORD);
    HRESULT (__stdcall *SetHWnd)(IDirectDrawClipperImpl *, DWORD, HWND );
};

extern struct IDirectDrawClipperImplVtbl g_ddc_vtbl;

#endif
