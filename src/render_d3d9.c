#include <windows.h>
#include <stdio.h>
#include <d3d9.h>
#include "fps_limiter.h"
#include "dd.h"
#include "ddsurface.h"
#include "d3d9shader.h"
#include "render_d3d9.h"
#include "utils.h"
#include "wndproc.h"
#include "debug.h"


static BOOL d3d9_create_resouces();
static BOOL d3d9_set_states();
static BOOL d3d9_update_vertices(BOOL upscale_hack, BOOL stretch);

static D3D9RENDERER g_d3d9;

BOOL d3d9_is_available()
{
    LPDIRECT3D9 d3d9 = NULL;

    if ((g_d3d9.hmodule = LoadLibrary("d3d9.ext")))
    {
        IDirect3D9* (WINAPI * d3d_create9)(UINT) =
            (IDirect3D9 * (WINAPI*)(UINT))GetProcAddress(g_d3d9.hmodule, "Direct3DCreate9");

        if (d3d_create9 && (d3d9 = d3d_create9(D3D_SDK_VERSION)))
            IDirect3D9_Release(d3d9);
    }

    return d3d9 != NULL;
}

BOOL d3d9_create()
{
    if (!d3d9_release())
        return FALSE;

    if (!g_d3d9.hmodule)
        g_d3d9.hmodule = LoadLibrary("d3d9.ext");

    if (g_d3d9.hmodule)
    {
        if (g_ddraw->nonexclusive)
        {
            int (WINAPI * d3d9_enable_shim)(BOOL) =
                (int (WINAPI*)(BOOL))GetProcAddress(g_d3d9.hmodule, "Direct3D9EnableMaximizedWindowedModeShim");

            if (d3d9_enable_shim)
                d3d9_enable_shim(TRUE);
        }

        IDirect3D9* (WINAPI * d3d_create9)(UINT) =
            (IDirect3D9 * (WINAPI*)(UINT))GetProcAddress(g_d3d9.hmodule, "Direct3DCreate9");

        if (d3d_create9 && (g_d3d9.instance = d3d_create9(D3D_SDK_VERSION)))
        {
            g_d3d9.bits_per_pixel = g_ddraw->render.bpp ? g_ddraw->render.bpp : g_ddraw->mode.dmBitsPerPel;

            memset(&g_d3d9.params, 0, sizeof(g_d3d9.params));

            g_d3d9.params.Windowed = g_ddraw->windowed;
            g_d3d9.params.SwapEffect = D3DSWAPEFFECT_DISCARD;
            g_d3d9.params.hDeviceWindow = g_ddraw->hwnd;
            g_d3d9.params.PresentationInterval = g_ddraw->vsync ? D3DPRESENT_INTERVAL_ONE : D3DPRESENT_INTERVAL_IMMEDIATE;
            g_d3d9.params.BackBufferWidth = g_d3d9.params.Windowed ? 0 : g_ddraw->render.width;
            g_d3d9.params.BackBufferHeight = g_d3d9.params.Windowed ? 0 : g_ddraw->render.height;
            g_d3d9.params.BackBufferFormat = g_d3d9.bits_per_pixel == 16 ? D3DFMT_R5G6B5 : D3DFMT_X8R8G8B8;
            g_d3d9.params.BackBufferCount = 1;

            DWORD behavior_flags[] = {
                D3DCREATE_MULTITHREADED | D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_PUREDEVICE,
                D3DCREATE_MULTITHREADED | D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_PUREDEVICE,
                D3DCREATE_MULTITHREADED | D3DCREATE_HARDWARE_VERTEXPROCESSING,
                D3DCREATE_MULTITHREADED | D3DCREATE_MIXED_VERTEXPROCESSING,
                D3DCREATE_MULTITHREADED | D3DCREATE_SOFTWARE_VERTEXPROCESSING,
            };

            for (int i = 0; i < sizeof(behavior_flags) / sizeof(behavior_flags[0]); i++)
            {
                if (SUCCEEDED(
                    IDirect3D9_CreateDevice(
                        g_d3d9.instance,
                        D3DADAPTER_DEFAULT,
                        D3DDEVTYPE_HAL,
                        g_ddraw->hwnd,
                        behavior_flags[i] | (g_ddraw->fpupreserve ? D3DCREATE_FPU_PRESERVE : 0),
                        &g_d3d9.params,
                        &g_d3d9.device)))
                    return g_d3d9.device && d3d9_create_resouces() && d3d9_set_states();
            }
        }
    }

    return FALSE;
}

BOOL d3d9_on_device_lost()
{
    if (g_d3d9.device && IDirect3DDevice9_TestCooperativeLevel(g_d3d9.device) == D3DERR_DEVICENOTRESET)
    {
        return d3d9_reset();
    }

    return FALSE;
}

BOOL d3d9_reset()
{
    g_d3d9.params.Windowed = g_ddraw->windowed;
    g_d3d9.params.BackBufferWidth = g_d3d9.params.Windowed ? 0 : g_ddraw->render.width;
    g_d3d9.params.BackBufferHeight = g_d3d9.params.Windowed ? 0 : g_ddraw->render.height;
    g_d3d9.params.BackBufferFormat = g_d3d9.bits_per_pixel == 16 ? D3DFMT_R5G6B5 : D3DFMT_X8R8G8B8;

    if (g_d3d9.device && SUCCEEDED(IDirect3DDevice9_Reset(g_d3d9.device, &g_d3d9.params)))
    {
        return d3d9_set_states();
    }

    return FALSE;
}

BOOL d3d9_release()
{
    if (g_d3d9.pixel_shader)
    {
        IDirect3DPixelShader9_Release(g_d3d9.pixel_shader);
        g_d3d9.pixel_shader = NULL;
    }

    if (g_d3d9.pixel_shader_bilinear)
    {
        IDirect3DPixelShader9_Release(g_d3d9.pixel_shader_bilinear);
        g_d3d9.pixel_shader_bilinear = NULL;
    }

    for (int i = 0; i < D3D9_TEXTURE_COUNT; i++)
    {
        if (g_d3d9.surface_tex[i])
        {
            IDirect3DTexture9_Release(g_d3d9.surface_tex[i]);
            g_d3d9.surface_tex[i] = NULL;
        }

        if (g_d3d9.palette_tex[i])
        {
            IDirect3DTexture9_Release(g_d3d9.palette_tex[i]);
            g_d3d9.palette_tex[i] = NULL;
        }
    }

    if (g_d3d9.vertex_buf)
    {
        IDirect3DVertexBuffer9_Release(g_d3d9.vertex_buf);
        g_d3d9.vertex_buf = NULL;
    }

    if (g_d3d9.device)
    {
        IDirect3DDevice9_Release(g_d3d9.device);
        g_d3d9.device = NULL;
    }

    if (g_d3d9.instance)
    {
        IDirect3D9_Release(g_d3d9.instance);
        g_d3d9.instance = NULL;
    }

    return TRUE;
}

static BOOL d3d9_create_resouces()
{
    BOOL err = FALSE;

    int width = g_ddraw->width;
    int height = g_ddraw->height;

    g_d3d9.tex_width =
        width <= 1024 ? 1024 : width <= 2048 ? 2048 : width <= 4096 ? 4096 : width;

    g_d3d9.tex_height =
        height <= g_d3d9.tex_width ? g_d3d9.tex_width : height <= 2048 ? 2048 : height <= 4096 ? 4096 : height;

    g_d3d9.tex_width = g_d3d9.tex_width > g_d3d9.tex_height ? g_d3d9.tex_width : g_d3d9.tex_height;

    g_d3d9.scale_w = (float)width / g_d3d9.tex_width;;
    g_d3d9.scale_h = (float)height / g_d3d9.tex_height;

    err = err || FAILED(
        IDirect3DDevice9_CreateVertexBuffer(
            g_d3d9.device,
            sizeof(CUSTOMVERTEX) * 4, 0,
            D3DFVF_XYZRHW | D3DFVF_TEX1,
            D3DPOOL_MANAGED,
            &g_d3d9.vertex_buf,
            NULL));

    err = err || !d3d9_update_vertices(InterlockedExchangeAdd(&g_ddraw->upscale_hack_active, 0), TRUE);

    for (int i = 0; i < D3D9_TEXTURE_COUNT; i++)
    {
        err = err || FAILED(
            IDirect3DDevice9_CreateTexture(
                g_d3d9.device,
                g_d3d9.tex_width,
                g_d3d9.tex_height,
                1,
                0,
                g_ddraw->bpp == 16 ? D3DFMT_R5G6B5 : g_ddraw->bpp == 32 ? D3DFMT_X8R8G8B8 : D3DFMT_L8,
                D3DPOOL_MANAGED,
                &g_d3d9.surface_tex[i],
                0));

        err = err || !g_d3d9.surface_tex[i];

        if (g_ddraw->bpp == 8)
        {
            err = err || FAILED(
                IDirect3DDevice9_CreateTexture(
                    g_d3d9.device,
                    256,
                    256,
                    1,
                    0,
                    D3DFMT_X8R8G8B8,
                    D3DPOOL_MANAGED,
                    &g_d3d9.palette_tex[i],
                    0));

            err = err || !g_d3d9.palette_tex[i];
        }
    }

    if (g_ddraw->bpp == 8)
    {
        err = err || FAILED(
            IDirect3DDevice9_CreatePixelShader(g_d3d9.device, (DWORD*)D3D9_PALETTE_SHADER, &g_d3d9.pixel_shader));

        IDirect3DDevice9_CreatePixelShader(
            g_d3d9.device, 
            (DWORD*)D3D9_PALETTE_SHADER_BILINEAR, 
            &g_d3d9.pixel_shader_bilinear);
    }

    return g_d3d9.vertex_buf && (g_d3d9.pixel_shader || g_ddraw->bpp == 16 || g_ddraw->bpp == 32) && !err;
}

static BOOL d3d9_set_states()
{
    BOOL err = FALSE;

    err = err || FAILED(IDirect3DDevice9_SetFVF(g_d3d9.device, D3DFVF_XYZRHW | D3DFVF_TEX1));
    err = err || FAILED(IDirect3DDevice9_SetStreamSource(g_d3d9.device, 0, g_d3d9.vertex_buf, 0, sizeof(CUSTOMVERTEX)));
    err = err || FAILED(IDirect3DDevice9_SetTexture(g_d3d9.device, 0, (IDirect3DBaseTexture9*)g_d3d9.surface_tex[0]));

    if (g_ddraw->bpp == 8)
    {
        err = err || FAILED(IDirect3DDevice9_SetTexture(g_d3d9.device, 1, (IDirect3DBaseTexture9*)g_d3d9.palette_tex[0]));
        
        BOOL bilinear =
            g_ddraw->d3d9linear &&
            g_d3d9.pixel_shader_bilinear &&
            (g_ddraw->render.viewport.width != g_ddraw->width || g_ddraw->render.viewport.height != g_ddraw->height);

        err = err || FAILED(
            IDirect3DDevice9_SetPixelShader(
                g_d3d9.device, 
                bilinear ? g_d3d9.pixel_shader_bilinear : g_d3d9.pixel_shader));

        if (bilinear)
        {
            float texture_size[4] = { (float)g_d3d9.tex_width, (float)g_d3d9.tex_height, 0, 0 };
            err = err || FAILED(IDirect3DDevice9_SetPixelShaderConstantF(g_d3d9.device, 0, texture_size, 1));
        }
    }
    else
    {
        if (g_ddraw->d3d9linear)
        {
            IDirect3DDevice9_SetSamplerState(g_d3d9.device, 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
            IDirect3DDevice9_SetSamplerState(g_d3d9.device, 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
        }
    }

    /*
    D3DVIEWPORT9 view_data = {
        g_ddraw->render.viewport.x,
        g_ddraw->render.viewport.y,
        g_ddraw->render.viewport.width,
        g_ddraw->render.viewport.height,
        0.0f,
        1.0f };

    err = err || FAILED(IDirect3DDevice9_SetViewport(g_d3d9.device, &view_data));
    */
    return !err;
}

static BOOL d3d9_update_vertices(BOOL upscale_hack, BOOL stretch)
{
    float vp_x = stretch ? (float)g_ddraw->render.viewport.x : 0.0f;
    float vp_y = stretch ? (float)g_ddraw->render.viewport.y : 0.0f;

    float vp_w = stretch ? (float)(g_ddraw->render.viewport.width + g_ddraw->render.viewport.x) : (float)g_ddraw->width;
    float vp_h = stretch ? (float)(g_ddraw->render.viewport.height + g_ddraw->render.viewport.y) : (float)g_ddraw->height;

    float s_h = upscale_hack ? g_d3d9.scale_h * ((float)g_ddraw->upscale_hack_height / g_ddraw->height) : g_d3d9.scale_h;
    float s_w = upscale_hack ? g_d3d9.scale_w * ((float)g_ddraw->upscale_hack_width / g_ddraw->width) : g_d3d9.scale_w;

    CUSTOMVERTEX vertices[] =
    {
        { vp_x - 0.5f, vp_h - 0.5f, 0.0f, 1.0f, 0.0f, s_h },
        { vp_x - 0.5f, vp_y - 0.5f, 0.0f, 1.0f, 0.0f, 0.0f },
        { vp_w - 0.5f, vp_h - 0.5f, 0.0f, 1.0f, s_w,  s_h },
        { vp_w - 0.5f, vp_y - 0.5f, 0.0f, 1.0f, s_w,  0.0f }
    };

    void* data;
    if (g_d3d9.vertex_buf && SUCCEEDED(IDirect3DVertexBuffer9_Lock(g_d3d9.vertex_buf, 0, 0, (void**)&data, 0)))
    {
        memcpy(data, vertices, sizeof(vertices));

        IDirect3DVertexBuffer9_Unlock(g_d3d9.vertex_buf);
        return TRUE;
    }

    return FALSE;
}

DWORD WINAPI d3d9_render_main(void)
{
    Sleep(500);

    fpsl_init();

    BOOL needs_update = FALSE;
    LONG clear_count = 0;

    DWORD timeout = g_ddraw->render.minfps > 0 ? g_ddraw->render.minfps_tick_len : 200;

    while (g_ddraw->render.run &&
        (g_ddraw->render.minfps < 0 || WaitForSingleObject(g_ddraw->render.sem, timeout) != WAIT_FAILED))
    {
#if _DEBUG
        dbg_draw_frame_info_start();
#endif

        static int tex_index = 0, pal_index = 0;

        if (InterlockedExchange(&g_ddraw->render.clear_screen, FALSE))
            clear_count = 10;

        fpsl_frame_start();

        EnterCriticalSection(&g_ddraw->cs);

        if (g_ddraw->primary && 
            g_ddraw->primary->bpp == g_ddraw->bpp &&
            (g_ddraw->bpp == 16 || g_ddraw->bpp == 32 || g_ddraw->primary->palette))
        {
            if (g_ddraw->vhack)
            {
                if (util_detect_low_res_screen())
                {
                    if (!InterlockedExchange(&g_ddraw->upscale_hack_active, TRUE))
                        d3d9_update_vertices(TRUE, TRUE);
                }
                else
                {
                    if (InterlockedExchange(&g_ddraw->upscale_hack_active, FALSE))
                        d3d9_update_vertices(FALSE, TRUE);
                }
            }

            D3DLOCKED_RECT lock_rc;

            if (InterlockedExchange(&g_ddraw->render.surface_updated, FALSE) || g_ddraw->render.minfps == -2)
            {
                if (++tex_index >= D3D9_TEXTURE_COUNT)
                    tex_index = 0;

                RECT rc = { 0, 0, g_ddraw->width, g_ddraw->height };

                if (SUCCEEDED(IDirect3DDevice9_SetTexture(g_d3d9.device, 0, (IDirect3DBaseTexture9*)g_d3d9.surface_tex[tex_index])) &&
                    SUCCEEDED(IDirect3DTexture9_LockRect(g_d3d9.surface_tex[tex_index], 0, &lock_rc, &rc, 0)))
                {
                    unsigned char* src = (unsigned char*)g_ddraw->primary->surface;
                    unsigned char* dst = (unsigned char*)lock_rc.pBits;

                    int i;
                    for (i = 0; i < g_ddraw->height; i++)
                    {
                        memcpy(dst, src, g_ddraw->primary->l_pitch);

                        src += g_ddraw->primary->l_pitch;
                        dst += lock_rc.Pitch;
                    }

                    IDirect3DTexture9_UnlockRect(g_d3d9.surface_tex[tex_index], 0);
                }
            }

            if (g_ddraw->bpp == 8 &&
                (InterlockedExchange(&g_ddraw->render.palette_updated, FALSE) || g_ddraw->render.minfps == -2))
            {
                if (++pal_index >= D3D9_TEXTURE_COUNT)
                    pal_index = 0;

                RECT rc = { 0,0,256,1 };

                if (SUCCEEDED(IDirect3DDevice9_SetTexture(g_d3d9.device, 1, (IDirect3DBaseTexture9*)g_d3d9.palette_tex[pal_index])) &&
                    SUCCEEDED(IDirect3DTexture9_LockRect(g_d3d9.palette_tex[pal_index], 0, &lock_rc, &rc, 0)))
                {
                    memcpy(lock_rc.pBits, g_ddraw->primary->palette->data_rgb, 256 * sizeof(int));

                    IDirect3DTexture9_UnlockRect(g_d3d9.palette_tex[pal_index], 0);
                }
            }

            if (g_ddraw->fixchilds)
            {
                g_ddraw->child_window_exists = FALSE;
                EnumChildWindows(g_ddraw->hwnd, util_enum_child_proc, (LPARAM)g_ddraw->primary);

                if (g_ddraw->render.width != g_ddraw->width || g_ddraw->render.height != g_ddraw->height)
                {
                    if (g_ddraw->child_window_exists)
                    {
                        IDirect3DDevice9_Clear(g_d3d9.device, 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);

                        if (!needs_update && d3d9_update_vertices(FALSE, FALSE))
                            needs_update = TRUE;
                    }
                    else if (needs_update)
                    {
                        if (d3d9_update_vertices(FALSE, TRUE))
                            needs_update = FALSE;
                    }
                }
            }
        }

        LeaveCriticalSection(&g_ddraw->cs);

        if (clear_count > 0)
        {
            clear_count--;
            IDirect3DDevice9_Clear(g_d3d9.device, 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
        }

        IDirect3DDevice9_BeginScene(g_d3d9.device);
        IDirect3DDevice9_DrawPrimitive(g_d3d9.device, D3DPT_TRIANGLESTRIP, 0, 2);
        IDirect3DDevice9_EndScene(g_d3d9.device);

        if (g_ddraw->bnet_active)
        {
            IDirect3DDevice9_Clear(g_d3d9.device, 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
        }

        if (FAILED(IDirect3DDevice9_Present(g_d3d9.device, NULL, NULL, NULL, NULL)))
        {
            DWORD_PTR result;
            SendMessageTimeout(g_ddraw->hwnd, WM_D3D9DEVICELOST, 0, 0, 0, 1000, &result);
        }

#if _DEBUG
        dbg_draw_frame_info_end();
#endif

        fpsl_frame_end();
    }

    if (g_ddraw->vhack)
        InterlockedExchange(&g_ddraw->upscale_hack_active, FALSE);

    return 0;
}
