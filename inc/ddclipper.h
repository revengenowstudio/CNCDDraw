#ifndef DDCLIPPER_H
#define DDCLIPPER_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "ddraw.h"


HRESULT dd_CreateClipper(DWORD dwFlags, LPDIRECTDRAWCLIPPER FAR* lplpDDClipper, IUnknown FAR* pUnkOuter);

#endif
