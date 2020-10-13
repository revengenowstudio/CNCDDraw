#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "IDirectDrawClipper.h"
#include "ddclipper.h"
#include "debug.h"


HRESULT dd_CreateClipper(DWORD dwFlags, LPDIRECTDRAWCLIPPER FAR *lplpDDClipper, IUnknown FAR *pUnkOuter )
{
    if (!lplpDDClipper)
        return DDERR_INVALIDPARAMS;

    IDirectDrawClipperImpl *c = (IDirectDrawClipperImpl *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(IDirectDrawClipperImpl));
    c->lpVtbl = &g_ddc_vtbl;
    dprintf("     Clipper = %p\n", c);
    *lplpDDClipper = (LPDIRECTDRAWCLIPPER)c;

    IDirectDrawClipper_AddRef(c);

    return DD_OK;
}
