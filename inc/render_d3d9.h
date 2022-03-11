#ifndef RENDER_D3D9_H
#define RENDER_D3D9_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d9.h>

#define D3D9_TEXTURE_COUNT 2

typedef struct CUSTOMVERTEX { float x, y, z, rhw, u, v; } CUSTOMVERTEX;

typedef struct D3D9RENDERER
{
    HMODULE hmodule;
    D3DPRESENT_PARAMETERS params;
    LPDIRECT3D9 instance;
    LPDIRECT3DDEVICE9 device;
    LPDIRECT3DVERTEXBUFFER9 vertex_buf;
    IDirect3DTexture9* surface_tex[D3D9_TEXTURE_COUNT];
    IDirect3DTexture9* palette_tex[D3D9_TEXTURE_COUNT];
    IDirect3DPixelShader9* pixel_shader;
    IDirect3DPixelShader9* pixel_shader_bilinear;
    float scale_w;
    float scale_h;
    int bits_per_pixel;
    int tex_width;
    int tex_height;
} D3D9RENDERER;

BOOL d3d9_is_available();
DWORD WINAPI d3d9_render_main(void);
BOOL d3d9_create();
BOOL d3d9_reset();
BOOL d3d9_release();
BOOL d3d9_on_device_lost();
BOOL d3d9_device_initialized();


#endif
