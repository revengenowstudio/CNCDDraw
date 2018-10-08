#include <windows.h>
#include <stdio.h>
#include <d3d9.h>
#include "main.h"
#include "surface.h"
#include "d3d9shader.h"

typedef struct CUSTOMVERTEX { float x, y, z, rhw, u, v; } CUSTOMVERTEX;

BOOL D3D9_Enabled;
HMODULE D3D9_hModule;

static D3DPRESENT_PARAMETERS D3dpp;
static LPDIRECT3D9 D3d;
static LPDIRECT3DDEVICE9 D3dDev;
static LPDIRECT3DVERTEXBUFFER9 VertexBuf;
static IDirect3DTexture9 *SurfaceTex;
static IDirect3DTexture9 *PaletteTex;
static IDirect3DPixelShader9 *PixelShader;
static float ScaleW;
static float ScaleH;
static int MaxFPS;
static DWORD FrameLength;

static BOOL CreateDirect3D();
static BOOL CreateResources();
static BOOL SetStates();
static void UpdateVertices(BOOL inCutscene);
static BOOL Reset();
static void SetMaxFPS();
static void Render();
static BOOL ReleaseDirect3D();

BOOL detect_cutscene();
DWORD WINAPI render_soft_main(void);

DWORD WINAPI render_d3d9_main(void)
{
    Sleep(500);

    D3D9_Enabled = CreateDirect3D();
    if (D3D9_Enabled)
    {
        SetMaxFPS();

        Render();
    }

    ReleaseDirect3D();

    if (!D3D9_Enabled)
    {
        ShowDriverWarning = TRUE;
        ddraw->renderer = render_soft_main;
        render_soft_main();
    }

    return 0;
}

static BOOL CreateDirect3D()
{
    if (!ReleaseDirect3D())
        return FALSE;

    if (!D3D9_hModule)
        D3D9_hModule = LoadLibrary("d3d9.dll");

    if (D3D9_hModule)
    {
        IDirect3D9 *(WINAPI *D3DCreate9)(UINT) =
            (IDirect3D9 *(WINAPI *)(UINT))GetProcAddress(D3D9_hModule, "Direct3DCreate9");

        if (D3DCreate9 && (D3d = D3DCreate9(D3D_SDK_VERSION)))
        {
            D3dpp.Windowed = ddraw->windowed;
            D3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
            D3dpp.hDeviceWindow = ddraw->hWnd;
            D3dpp.PresentationInterval = ddraw->vsync ? D3DPRESENT_INTERVAL_ONE : D3DPRESENT_INTERVAL_IMMEDIATE;
            D3dpp.BackBufferWidth = D3dpp.Windowed ? 0 : ddraw->render.width;
            D3dpp.BackBufferHeight = D3dpp.Windowed ? 0 : ddraw->render.height;
            D3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;
            D3dpp.BackBufferCount = 1;

            DWORD behaviorFlags[] = {
                D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_PUREDEVICE,
                D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_PUREDEVICE,
                D3DCREATE_HARDWARE_VERTEXPROCESSING,
                D3DCREATE_MIXED_VERTEXPROCESSING,
                D3DCREATE_SOFTWARE_VERTEXPROCESSING,
            };

            int i;
            for (i = 0; i < sizeof(behaviorFlags) / sizeof(behaviorFlags[0]); i++)
            {
                if (SUCCEEDED(D3d->lpVtbl->CreateDevice(
                    D3d,
                    D3DADAPTER_DEFAULT,
                    D3DDEVTYPE_HAL,
                    ddraw->hWnd,
                    D3DCREATE_MULTITHREADED | D3DCREATE_NOWINDOWCHANGES | behaviorFlags[i],
                    &D3dpp,
                    &D3dDev)))
                    break;
            }
        }
    }

    return D3d && D3dDev && CreateResources() && SetStates();
}

static BOOL CreateResources()
{
    int width = ddraw->width;
    int height = ddraw->height;

    int texWidth =
        width <= 1024 ? 1024 : width <= 2048 ? 2048 : width <= 4096 ? 4096 : width;

    int texHeight =
        height <= texWidth ? texWidth : height <= 2048 ? 2048 : height <= 4096 ? 4096 : height;

    texWidth = texWidth > texHeight ? texWidth : texHeight;

    ScaleW = (float)width / texWidth;;
    ScaleH = (float)height / texHeight;

    D3dDev->lpVtbl->CreateVertexBuffer(
        D3dDev, sizeof(CUSTOMVERTEX) * 4, 0, D3DFVF_XYZRHW | D3DFVF_TEX1, D3DPOOL_MANAGED, &VertexBuf, NULL);

    UpdateVertices(InterlockedExchangeAdd(&ddraw->incutscene, 0));
    D3dDev->lpVtbl->CreateTexture(D3dDev, texWidth, texHeight, 1, 0, D3DFMT_L8, D3DPOOL_MANAGED, &SurfaceTex, 0);
    D3dDev->lpVtbl->CreateTexture(D3dDev, 256, 256, 1, 0, D3DFMT_X8R8G8B8, D3DPOOL_MANAGED, &PaletteTex, 0);
    D3dDev->lpVtbl->CreatePixelShader(D3dDev, (DWORD *)PalettePixelShaderSrc, &PixelShader);

    return SurfaceTex && PaletteTex && VertexBuf && PixelShader;
}

static BOOL SetStates()
{
    BOOL err = FALSE;

    err = err || FAILED(D3dDev->lpVtbl->SetFVF(D3dDev, D3DFVF_XYZRHW | D3DFVF_TEX1));
    err = err || FAILED(D3dDev->lpVtbl->SetStreamSource(D3dDev, 0, VertexBuf, 0, sizeof(CUSTOMVERTEX)));
    err = err || FAILED(D3dDev->lpVtbl->SetTexture(D3dDev, 0, (IDirect3DBaseTexture9 *)SurfaceTex));
    err = err || FAILED(D3dDev->lpVtbl->SetTexture(D3dDev, 1, (IDirect3DBaseTexture9 *)PaletteTex));
    err = err || FAILED(D3dDev->lpVtbl->SetPixelShader(D3dDev, PixelShader));

    D3DVIEWPORT9 viewData = {
        ddraw->render.viewport.x,
        ddraw->render.viewport.y,
        ddraw->render.viewport.width,
        ddraw->render.viewport.height,
        0.0f,
        1.0f };

    err = err || FAILED(D3dDev->lpVtbl->SetViewport(D3dDev, &viewData));

    return !err;
}

static void UpdateVertices(BOOL inCutscene)
{
    float vpX = (float)ddraw->render.viewport.x;
    float vpY = (float)ddraw->render.viewport.y;

    float vpW = (float)(ddraw->render.viewport.width + ddraw->render.viewport.x);
    float vpH = (float)(ddraw->render.viewport.height + ddraw->render.viewport.y);

    float sH = inCutscene ? ScaleH * ((float)CUTSCENE_HEIGHT / ddraw->height) : ScaleH;
    float sW = inCutscene ? ScaleW * ((float)CUTSCENE_WIDTH / ddraw->width) : ScaleW;

    CUSTOMVERTEX vertices[] =
    {
        { vpX - 0.5f, vpH - 0.5f, 0.0f, 1.0f, 0.0f, sH },
        { vpX - 0.5f, vpY - 0.5f, 0.0f, 1.0f, 0.0f, 0.0f },
        { vpW - 0.5f, vpH - 0.5f, 0.0f, 1.0f, sW,   sH },
        { vpW - 0.5f, vpY - 0.5f, 0.0f, 1.0f, sW,   0.0f }
    };

    void *data;
    if (VertexBuf && SUCCEEDED(VertexBuf->lpVtbl->Lock(VertexBuf, 0, 0, (void**)&data, 0)))
    {
        memcpy(data, vertices, sizeof(vertices));
        VertexBuf->lpVtbl->Unlock(VertexBuf);
    }
}

static BOOL Reset()
{
    D3dpp.BackBufferWidth = D3dpp.Windowed ? 0 : ddraw->render.width;
    D3dpp.BackBufferHeight = D3dpp.Windowed ? 0 : ddraw->render.height;
    D3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;

    if (SUCCEEDED(D3dDev->lpVtbl->Reset(D3dDev, &D3dpp)))
        return SetStates();

    return FALSE;
}

static void SetMaxFPS()
{
    MaxFPS = ddraw->render.maxfps;

    if (MaxFPS < 0)
        MaxFPS = ddraw->mode.dmDisplayFrequency;

    if (MaxFPS == 0)
        MaxFPS = 125;

    if (MaxFPS >= 1000 || ddraw->vsync)
        MaxFPS = 0;

    if (MaxFPS > 0)
        FrameLength = 1000.0f / MaxFPS;
}

static void Render()
{
    DWORD tickStart = 0;
    DWORD tickEnd = 0;
    BOOL active = TRUE;

    while (ddraw->render.run && WaitForSingleObject(ddraw->render.sem, 200) != WAIT_FAILED)
    {
        if (!active)
        {
            Sleep(500);

            if (!InterlockedExchangeAdd(&ddraw->minimized, 0) && CreateDirect3D())
                active = TRUE;

            continue;
        }

#if _DEBUG
        DrawFrameInfoStart();
#endif

        if (MaxFPS > 0)
            tickStart = timeGetTime();

        EnterCriticalSection(&ddraw->cs);

        if (ddraw->primary && ddraw->primary->palette && ddraw->primary->palette->data_rgb)
        {
            if (ddraw->vhack)
            {
                if (detect_cutscene())
                {
                    if (!InterlockedExchange(&ddraw->incutscene, TRUE))
                        UpdateVertices(TRUE);
                }
                else
                {
                    if (InterlockedExchange(&ddraw->incutscene, FALSE))
                        UpdateVertices(FALSE);
                }
            }

            D3DLOCKED_RECT lock_rc;

            if (InterlockedExchange(&ddraw->render.surfaceUpdated, FALSE))
            {
                RECT rc = { 0,0,ddraw->width,ddraw->height };

                if (SUCCEEDED(SurfaceTex->lpVtbl->LockRect(SurfaceTex, 0, &lock_rc, &rc, 0)))
                {
                    unsigned char *src = (unsigned char *)ddraw->primary->surface;
                    unsigned char *dst = (unsigned char *)lock_rc.pBits;

                    int i;
                    for (i = 0; i < ddraw->height; i++)
                    {
                        memcpy(dst, src, ddraw->width);

                        src += ddraw->width;
                        dst += lock_rc.Pitch;
                    }

                    SurfaceTex->lpVtbl->UnlockRect(SurfaceTex, 0);
                }
            }

            if (InterlockedExchange(&ddraw->render.paletteUpdated, FALSE))
            {
                RECT rc = { 0,0,256,1 };

                if (SUCCEEDED(PaletteTex->lpVtbl->LockRect(PaletteTex, 0, &lock_rc, &rc, 0)))
                {
                    memcpy(lock_rc.pBits, ddraw->primary->palette->data_rgb, 4 * 256);

                    PaletteTex->lpVtbl->UnlockRect(PaletteTex, 0);
                }
            }
        }

        LeaveCriticalSection(&ddraw->cs);

        HRESULT hr = D3dDev->lpVtbl->TestCooperativeLevel(D3dDev);
        LONG modeChanged = InterlockedExchange(&ddraw->displayModeChanged, FALSE);
        LONG minimized = InterlockedExchangeAdd(&ddraw->minimized, 0);

        if (minimized || modeChanged)
        {
            active = FALSE;
            BOOL released = ReleaseDirect3D();
            Sleep(100);

            if (minimized)
            {
                ShowWindow(ddraw->hWnd, SW_MINIMIZE);

                if (!released)
                    ChangeDisplaySettings(&ddraw->mode, 0);
            }
        }
        else if (hr == D3DERR_DEVICENOTRESET && D3dpp.Windowed)
        {
            Reset();
        }
        else if (SUCCEEDED(hr))
        {
            D3dDev->lpVtbl->BeginScene(D3dDev);
            D3dDev->lpVtbl->DrawPrimitive(D3dDev, D3DPT_TRIANGLESTRIP, 0, 2);
            D3dDev->lpVtbl->EndScene(D3dDev);

            D3dDev->lpVtbl->Present(D3dDev, NULL, NULL, NULL, NULL);
        }

#if _DEBUG
        DrawFrameInfoEnd();
#endif

        if (MaxFPS > 0)
        {
            tickEnd = timeGetTime();

            if (tickEnd - tickStart < FrameLength)
                Sleep(FrameLength - (tickEnd - tickStart));
        }
    }
}

static BOOL ReleaseDirect3D()
{
    if (VertexBuf)
    {
        VertexBuf->lpVtbl->Release(VertexBuf);
        VertexBuf = NULL;
    }
        
    if (SurfaceTex)
    {
        SurfaceTex->lpVtbl->Release(SurfaceTex);
        SurfaceTex = NULL;
    }

    if (PaletteTex)
    {
        PaletteTex->lpVtbl->Release(PaletteTex);
        PaletteTex = NULL;
    }

    if (PixelShader)
    {
        PixelShader->lpVtbl->Release(PixelShader);
        PixelShader = NULL;
    }

    if (D3dDev)
    {
        if (FAILED(D3dDev->lpVtbl->Release(D3dDev)))
            return FALSE;

        D3dDev = NULL;
    }

    if (D3d)
    {
        if (FAILED(D3d->lpVtbl->Release(D3d)))
            return FALSE;

        D3d = NULL;
    }

    return TRUE;
}
