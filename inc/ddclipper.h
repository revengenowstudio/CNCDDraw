#ifndef DDCLIPPER_H
#define DDCLIPPER_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <ddraw.h>
#include "IDirectDrawClipper.h"


HRESULT dd_CreateClipper(DWORD dwFlags, IDirectDrawClipperImpl** lplpDDClipper, IUnknown FAR* pUnkOuter);

#endif
