#include <windows.h>
#include <stdio.h>
#include <d3d9.h>
#include "main.h"
#include "surface.h"

// TO DO:
// Try to get fullscreen exclusive working
// vhack isn't working

const BYTE PalettePixelShaderSrc[] =
{
    0,2,255,255,254,255,42,0,67,84,65,66,28,0,0,0,115,0,0,0,0,2,255,255,
    2,0,0,0,28,0,0,0,0,1,0,0,108,0,0,0,68,0,0,0,3,0,0,0,
    1,0,2,0,72,0,0,0,0,0,0,0,88,0,0,0,3,0,1,0,1,0,6,0,
    92,0,0,0,0,0,0,0,115,48,0,171,4,0,12,0,1,0,1,0,1,0,0,0,
    0,0,0,0,115,49,0,171,4,0,12,0,1,0,1,0,1,0,0,0,0,0,0,0,
    112,115,95,50,95,48,0,77,105,99,114,111,115,111,102,116,32,40,82,41,32,72,76,83,
    76,32,83,104,97,100,101,114,32,67,111,109,112,105,108,101,114,32,57,46,50,57,46,57,
    53,50,46,51,49,49,49,0,81,0,0,5,0,0,15,160,0,0,127,63,0,0,0,59,
    0,0,0,0,0,0,0,0,31,0,0,2,0,0,0,128,0,0,3,176,31,0,0,2,
    0,0,0,144,0,8,15,160,31,0,0,2,0,0,0,144,1,8,15,160,66,0,0,3,
    0,0,15,128,0,0,228,176,0,8,228,160,4,0,0,4,0,0,1,128,0,0,0,128,
    0,0,0,160,0,0,85,160,1,0,0,2,0,0,2,128,0,0,170,160,66,0,0,3,
    0,0,15,128,0,0,228,128,1,8,228,160,1,0,0,2,0,8,15,128,0,0,228,128,255,255,0,0
};

LPDIRECT3D9 D3d;
LPDIRECT3DDEVICE9 D3ddev;
LPDIRECT3DVERTEXBUFFER9 D3dvb;
IDirect3DTexture9 *SurfaceTex;
IDirect3DTexture9 *PaletteTex;
IDirect3DPixelShader9 *PixelShader;
D3DPRESENT_PARAMETERS D3dpp;

static void InitDirect3D(BOOL reset)
{
    if (reset)
    {
        D3dvb->lpVtbl->Release(D3dvb);
        SurfaceTex->lpVtbl->Release(SurfaceTex);
        PaletteTex->lpVtbl->Release(PaletteTex);
        PixelShader->lpVtbl->Release(PixelShader);

        if (FAILED(D3ddev->lpVtbl->Reset(D3ddev, &D3dpp)))
            return;
    }

    int width = ddraw->width;
    int height = ddraw->height;

    int surfaceTexWidth =
        width <= 1024 ? 1024 : width <= 2048 ? 2048 : width <= 4096 ? 4096 : width;

    int surfaceTexHeight =
        height <= 512 ? 512 : height <= 1024 ? 1024 : height <= 2048 ? 2048 : height <= 4096 ? 4096 : height;

    float scaleW = (float)width / surfaceTexWidth;;
    float scaleH = (float)height / surfaceTexHeight;

    float vpX = (float)ddraw->render.viewport.x;
    float vpY = (float)ddraw->render.viewport.y;

    float vpW = (float)(ddraw->render.viewport.width + vpX);
    float vpH = (float)(ddraw->render.viewport.height + vpY);

    typedef struct CUSTOMVERTEX { float x, y, z, rhw, u, v; } CUSTOMVERTEX;
    CUSTOMVERTEX vertices[] =
    {
        { vpX - 0.5f, vpH - 0.5f, 0.0f, 1.0f, 0.0f,   scaleH },
        { vpX - 0.5f, vpY - 0.5f, 0.0f, 1.0f, 0.0f,   0.0f },
        { vpW - 0.5f, vpH - 0.5f, 0.0f, 1.0f, scaleW, scaleH },
        { vpW - 0.5f, vpY - 0.5f, 0.0f, 1.0f, scaleW, 0.0f }
    };

    D3ddev->lpVtbl->SetFVF(D3ddev, D3DFVF_XYZRHW | D3DFVF_TEX1);
    D3ddev->lpVtbl->CreateVertexBuffer(
        D3ddev, sizeof(vertices), 0, D3DFVF_XYZRHW | D3DFVF_TEX1, D3DPOOL_MANAGED, &D3dvb, NULL);

    void *data;
    if (SUCCEEDED(D3dvb->lpVtbl->Lock(D3dvb, 0, 0, (void**)&data, 0)))
    {
        memcpy(data, vertices, sizeof(vertices));
        D3dvb->lpVtbl->Unlock(D3dvb);
    }

    D3ddev->lpVtbl->SetStreamSource(D3ddev, 0, D3dvb, 0, sizeof(CUSTOMVERTEX));

    D3ddev->lpVtbl->CreateTexture(
        D3ddev, surfaceTexWidth, surfaceTexHeight, 1, 0, D3DFMT_L8, D3DPOOL_MANAGED, &SurfaceTex, 0);
    D3ddev->lpVtbl->SetTexture(D3ddev, 0, (IDirect3DBaseTexture9 *)SurfaceTex);

    D3ddev->lpVtbl->CreateTexture(D3ddev, 256, 256, 1, 0, D3DFMT_X8R8G8B8, D3DPOOL_MANAGED, &PaletteTex, 0);
    D3ddev->lpVtbl->SetTexture(D3ddev, 1, (IDirect3DBaseTexture9 *)PaletteTex);

    D3ddev->lpVtbl->CreatePixelShader(D3ddev, (DWORD *)PalettePixelShaderSrc, &PixelShader);
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

DWORD WINAPI render_d3d9_main(void)
{
    Sleep(500);

    DWORD tick_start = 0;
    DWORD tick_end = 0;
    DWORD frame_len = 0;

    int maxfps = ddraw->render.maxfps;

    if (maxfps < 0)
        maxfps = ddraw->mode.dmDisplayFrequency;

    if (maxfps == 0)
        maxfps = 125;

    if (maxfps >= 1000 || ddraw->vsync)
        maxfps = 0;

    if (maxfps > 0)
        frame_len = 1000.0f / maxfps;

    HMODULE hD3D9 = LoadLibrary("d3d9.dll");
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

            D3d->lpVtbl->CreateDevice(
                D3d,
                D3DADAPTER_DEFAULT,
                D3DDEVTYPE_HAL,
                ddraw->hWnd,
                D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                &D3dpp,
                &D3ddev);

            InitDirect3D(FALSE);
        }
    }

    while (D3d && ddraw->render.run && WaitForSingleObject(ddraw->render.sem, INFINITE) != WAIT_FAILED)
    {
#if _DEBUG
        static DWORD tick_fps = 0;
        static DWORD frame_count = 0;
        static char debugText[512] = { 0 };
        static double frameTime = 0;
        RECT debugrc = { 0, 0, ddraw->width, ddraw->height };

        if (ddraw->primary && ddraw->primary->palette)
            DrawText(ddraw->primary->hDC, debugText, -1, &debugrc, DT_NOCLIP);

        tick_start = timeGetTime();
        if (tick_start >= tick_fps)
        {
            snprintf(
                debugText, 
                sizeof(debugText),
                "FPS: %lu | Time: %2.2f ms  ",
                frame_count, 
                frameTime);

            frame_count = 0;
            tick_fps = tick_start + 1000;

            CounterStart();
        }
        frame_count++;
#endif

        if (maxfps > 0)
            tick_start = timeGetTime();

        EnterCriticalSection(&ddraw->cs);

        if (ddraw->primary && ddraw->primary->palette && ddraw->primary->palette->data_rgb)
        {
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
            InitDirect3D(TRUE);
        }
        else if (SUCCEEDED(hr))
        {
            D3ddev->lpVtbl->BeginScene(D3ddev);
            D3ddev->lpVtbl->DrawPrimitive(D3ddev, D3DPT_TRIANGLESTRIP, 0, 2);
            D3ddev->lpVtbl->EndScene(D3ddev);

            D3ddev->lpVtbl->Present(D3ddev, NULL, NULL, NULL, NULL);
        }

#if _DEBUG
        if (frame_count == 1) frameTime = CounterStop();
#endif

        if (maxfps > 0)
        {
            tick_end = timeGetTime();

            if (tick_end - tick_start < frame_len)
                Sleep(frame_len - (tick_end - tick_start));
        }
    }

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

    return 0;
}
