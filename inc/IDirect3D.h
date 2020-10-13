#ifndef IDIRECTD3D_H 
#define IDIRECTD3D_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>


DEFINE_GUID(IID_IDirect3D, 0x3BBA0080, 0x2421, 0x11CF, 0xA3, 0x1A, 0x00, 0xAA, 0x00, 0xB9, 0x33, 0x56);
DEFINE_GUID(IID_IDirect3D2, 0x6aae1ec1, 0x662a, 0x11d0, 0x88, 0x9d, 0x00, 0xaa, 0x00, 0xbb, 0xb7, 0x6a);
DEFINE_GUID(IID_IDirect3D3, 0xbb223240, 0xe72b, 0x11d0, 0xa9, 0xb4, 0x00, 0xaa, 0x00, 0xc0, 0x99, 0x3e);
DEFINE_GUID(IID_IDirect3D7, 0xf5049e77, 0x4861, 0x11d2, 0xa4, 0x7, 0x0, 0xa0, 0xc9, 0x6, 0x29, 0xa8);

extern struct IDirect3DImplVtbl g_d3d_vtbl1;

struct IDirect3DImpl;
struct IDirect3DImplVtbl;

typedef struct IDirect3DImpl
{
    struct IDirect3DImplVtbl* lpVtbl;

    ULONG ref;

} IDirect3DImpl;

typedef struct IDirect3DImplVtbl IDirect3DImplVtbl;

struct IDirect3DImplVtbl
{
    HRESULT(__stdcall* QueryInterface) (IDirect3DImpl*, const IID* const riid, LPVOID* ppvObj);
    ULONG(__stdcall* AddRef) (IDirect3DImpl*);
    ULONG(__stdcall* Release) (IDirect3DImpl*);

    HRESULT(__stdcall* Initialize)(IDirect3DImpl*, int);
    HRESULT(__stdcall* EnumDevices)(IDirect3DImpl*, int, int);
    HRESULT(__stdcall* CreateLight)(IDirect3DImpl*, int, int);
    HRESULT(__stdcall* CreateMaterial)(IDirect3DImpl*, int, int);
    HRESULT(__stdcall* CreateViewport)(IDirect3DImpl*, int, int);
    HRESULT(__stdcall* FindDevice)(IDirect3DImpl*, int, int);
};

#endif
