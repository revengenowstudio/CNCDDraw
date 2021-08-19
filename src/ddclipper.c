#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "IDirectDrawClipper.h"
#include "ddclipper.h"
#include "debug.h"


HRESULT dd_CreateClipper(DWORD dwFlags, IDirectDrawClipperImpl** lplpDDClipper, IUnknown FAR* pUnkOuter)
{
    if (!lplpDDClipper)
        return DDERR_INVALIDPARAMS;

    IDirectDrawClipperImpl* c =
        (IDirectDrawClipperImpl*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(IDirectDrawClipperImpl));

    TRACE("     clipper = %p\n", c);

    c->lpVtbl = &g_ddc_vtbl;
    IDirectDrawClipper_AddRef(c);

    *lplpDDClipper = c;

    return DD_OK;
}
