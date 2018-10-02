#include <windows.h>
#include <stdio.h>
#include <d3d9.h>
#include "main.h"
#include "surface.h"
#include "paletteshader.h"

typedef struct CUSTOMVERTEX { float x, y, z, rhw, u, v; } CUSTOMVERTEX;

static HMODULE hD3D9;
static LPDIRECT3D9 D3d;
static LPDIRECT3DDEVICE9 D3ddev;
static LPDIRECT3DVERTEXBUFFER9 D3dvb;
static IDirect3DTexture9 *SurfaceTex;
static IDirect3DTexture9 *PaletteTex;
static IDirect3DPixelShader9 *PixelShader;
static D3DPRESENT_PARAMETERS D3dpp;
static float ScaleW;
static float ScaleH;
static int MaxFPS;
static DWORD FrameLength;

static BOOL CreateDirect3D();
static BOOL CreateResources();
static void SetStates();
static void UpdateVertices(BOOL inCutscene);
static BOOL Reset();
static void SetMaxFPS(int baseMaxFPS);
static void Render();
static void ReleaseDirect3D();

BOOL detect_cutscene();
DWORD WINAPI render_soft_main(void);

DWORD WINAPI render_d3d9_main(void)
{
    Sleep(500);

    BOOL useDirect3D = CreateDirect3D() && CreateResources();
    if (useDirect3D)
    {
        SetMaxFPS(ddraw->render.maxfps);
        SetStates();

        Render();
    }  

    ReleaseDirect3D();

    if (!useDirect3D)
    {
        ShowDriverWarning = TRUE;
        ddraw->renderer = render_soft_main;
        render_soft_main();
    }

    return 0;
}

static BOOL CreateDirect3D()
{
    D3d = NULL;
    D3ddev = NULL;
    SurfaceTex = NULL;
    PaletteTex = NULL;
    D3dvb = NULL;
    PixelShader = NULL;

    hD3D9 = LoadLibrary("d3d9.dll");
    if (hD3D9)
    {
        IDirect3D9 *(WINAPI *D3DCreate9)(UINT) =
            (IDirect3D9 *(WINAPI *)(UINT))GetProcAddress(hD3D9, "Direct3DCreate9");

        if (D3DCreate9 && (D3d = D3DCreate9(D3D_SDK_VERSION)))
        {
            D3dpp.Windowed = TRUE;
            D3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
            D3dpp.hDeviceWindow = ddraw->hWnd;
            D3dpp.PresentationInterval = ddraw->vsync ? D3DPRESENT_INTERVAL_ONE : D3DPRESENT_INTERVAL_IMMEDIATE;
            D3dpp.BackBufferWidth = ddraw->render.width;
            D3dpp.BackBufferHeight = ddraw->render.height;
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
                    D3DCREATE_NOWINDOWCHANGES | behaviorFlags[i],
                    &D3dpp,
                    &D3ddev)))
                    break;
            }
        }
    }

    return D3d && D3ddev;
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

    D3ddev->lpVtbl->CreateVertexBuffer(
        D3ddev, sizeof(CUSTOMVERTEX) * 4, 0, D3DFVF_XYZRHW | D3DFVF_TEX1, D3DPOOL_MANAGED, &D3dvb, NULL);

    UpdateVertices(InterlockedExchangeAdd(&ddraw->incutscene, 0));
    D3ddev->lpVtbl->CreateTexture(D3ddev, texWidth, texHeight, 1, 0, D3DFMT_L8, D3DPOOL_MANAGED, &SurfaceTex, 0);
    D3ddev->lpVtbl->CreateTexture(D3ddev, 256, 256, 1, 0, D3DFMT_X8R8G8B8, D3DPOOL_MANAGED, &PaletteTex, 0);
    D3ddev->lpVtbl->CreatePixelShader(D3ddev, (DWORD *)PalettePixelShaderSrc, &PixelShader);

    return SurfaceTex && PaletteTex && D3dvb && PixelShader;
}

static void SetStates()
{
    D3ddev->lpVtbl->SetFVF(D3ddev, D3DFVF_XYZRHW | D3DFVF_TEX1);
    D3ddev->lpVtbl->SetStreamSource(D3ddev, 0, D3dvb, 0, sizeof(CUSTOMVERTEX));
    D3ddev->lpVtbl->SetTexture(D3ddev, 0, (IDirect3DBaseTexture9 *)SurfaceTex);
    D3ddev->lpVtbl->SetTexture(D3ddev, 1, (IDirect3DBaseTexture9 *)PaletteTex);
    D3ddev->lpVtbl->SetPixelShader(D3ddev, PixelShader);

    D3DVIEWPORT9 viewData = {
        ddraw->render.viewport.x,
        ddraw->render.viewport.y,
        ddraw->render.viewport.width,
        ddraw->render.viewport.height,
        0.0f,
        1.0f };

    D3ddev->lpVtbl->SetViewport(D3ddev, &viewData);
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
    if (D3dvb && SUCCEEDED(D3dvb->lpVtbl->Lock(D3dvb, 0, 0, (void**)&data, 0)))
    {
        memcpy(data, vertices, sizeof(vertices));
        D3dvb->lpVtbl->Unlock(D3dvb);
    }
}

static BOOL Reset()
{
    if (SUCCEEDED(D3ddev->lpVtbl->Reset(D3ddev, &D3dpp)))
    {
        SetStates();
        return TRUE;
    }

    return FALSE;
}

static void SetMaxFPS(int baseMaxFPS)
{
    MaxFPS = baseMaxFPS;

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
    DWORD tick_start = 0;
    DWORD tick_end = 0;

    while (ddraw->render.run && WaitForSingleObject(ddraw->render.sem, INFINITE) != WAIT_FAILED)
    {
#if _DEBUG
        DrawFrameInfoStart();
#endif

        if (MaxFPS > 0)
            tick_start = timeGetTime();

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

        HRESULT hr = D3ddev->lpVtbl->TestCooperativeLevel(D3ddev);

        if (hr == D3DERR_DEVICENOTRESET)
        {
            Reset();
        }
        else if (SUCCEEDED(hr))
        {
            D3ddev->lpVtbl->BeginScene(D3ddev);
            D3ddev->lpVtbl->DrawPrimitive(D3ddev, D3DPT_TRIANGLESTRIP, 0, 2);
            D3ddev->lpVtbl->EndScene(D3ddev);

            D3ddev->lpVtbl->Present(D3ddev, NULL, NULL, NULL, NULL);
        }

#if _DEBUG
        DrawFrameInfoEnd();
#endif

        if (MaxFPS > 0)
        {
            tick_end = timeGetTime();

            if (tick_end - tick_start < FrameLength)
                Sleep(FrameLength - (tick_end - tick_start));
        }
    }
}

static void ReleaseDirect3D()
{
    if (D3dvb)
        D3dvb->lpVtbl->Release(D3dvb);

    if (SurfaceTex)
        SurfaceTex->lpVtbl->Release(SurfaceTex);

    if (PaletteTex)
        PaletteTex->lpVtbl->Release(PaletteTex);

    if (PixelShader)
        PixelShader->lpVtbl->Release(PixelShader);

    if (D3ddev)
        D3ddev->lpVtbl->Release(D3ddev);

    if (D3d)
        D3d->lpVtbl->Release(D3d);

    if (hD3D9)
        FreeLibrary(hD3D9);
}
