#ifndef IDIRECTDRAWGAMMACONTROL_H
#define IDIRECTDRAWGAMMACONTROL_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "ddraw.h"


DEFINE_GUID(IID_IDirectDrawGammaControl, 0x69C11C3E, 0xB46B, 0x11D1, 0xAD, 0x7A, 0x00, 0xC0, 0x4F, 0xC2, 0x9B, 0x4E);

struct IDirectDrawGammaControlImpl;
struct IDirectDrawGammaControlImplVtbl;

typedef struct IDirectDrawGammaControlImpl
{
    struct IDirectDrawGammaControlImplVtbl *lpVtbl;

    ULONG ref;

} IDirectDrawGammaControlImpl;

typedef struct IDirectDrawGammaControlImplVtbl IDirectDrawGammaControlImplVtbl;

struct IDirectDrawGammaControlImplVtbl
{
    /* IUnknown */
    HRESULT (__stdcall *QueryInterface)(IDirectDrawGammaControlImpl *, REFIID, void **);
    ULONG (__stdcall *AddRef)(IDirectDrawGammaControlImpl *);
    ULONG (__stdcall *Release)(IDirectDrawGammaControlImpl *);

    /* IDirectDrawGammaControl */
    HRESULT(__stdcall* GetGammaRamp)(IDirectDrawGammaControlImpl*, DWORD, void*);
    HRESULT(__stdcall* SetGammaRamp)(IDirectDrawGammaControlImpl*, DWORD, void*);
};

extern struct IDirectDrawGammaControlImplVtbl g_ddgc_vtbl;

#endif
