#include <windows.h>
#include <ddraw.h>
#include <stdio.h>
#include "dllmain.h"
#include "dd.h"
#include "ddclipper.h"
#include "debug.h"
#include "config.h"
#include "directinput.h"
#include "IDirectDraw.h"
#include "hook.h"

// exports to force usage of discrete high performance graphics device
DWORD NvOptimusEnablement = 1;
DWORD AmdPowerXpressRequestHighPerformance = 1;

// export for cncnet cnc games
BOOL GameHandlesClose;

// export for some warcraft II tools
void *g_fake_primary_surface_export;


HMODULE g_ddraw_module;

BOOL WINAPI DllMain(HANDLE hDll, DWORD dwReason, LPVOID lpReserved)
{
    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
    {
#if _DEBUG 
        dbg_init();
#endif
        g_ddraw_module = hDll;

        char buf[1024];

        if (GetEnvironmentVariable("__COMPAT_LAYER", buf, sizeof(buf)))
        {
            char* s = strtok(buf, " ");

            while (s)
            {
                if (_strcmpi(s, "WIN95") == 0 || strcmpi(s, "WIN98") == 0 || strcmpi(s, "NT4SP5") == 0)
                {
                    char mes[128] = { 0 };

                    _snprintf(
                        mes,
                        sizeof(mes),
                        "Please disable the '%s' compatibility mode for all game executables and "
                        "then try to start the game again.",
                        s);

                    MessageBoxA(NULL, mes, "Compatibility modes detected - cnc-ddraw", MB_OK);

                    break;
                }

                s = strtok(NULL, " ");
            }
        }

        BOOL set_dpi_aware = FALSE;
        HMODULE hshcore = GetModuleHandle("shcore.dll");
        
        if (hshcore)
        {
            typedef HRESULT(__stdcall* SETPROCESSDPIAWERENESSPROC)(PROCESS_DPI_AWARENESS value);

            SETPROCESSDPIAWERENESSPROC set_awareness =
                (SETPROCESSDPIAWERENESSPROC)GetProcAddress(hshcore, "SetProcessDpiAwareness");

            if (set_awareness)
            {
                HRESULT result = set_awareness(PROCESS_PER_MONITOR_DPI_AWARE);

                set_dpi_aware = result == S_OK || result == E_ACCESSDENIED;
            }
        }

        if (!set_dpi_aware)
        {
            HMODULE huser32 = GetModuleHandle("user32.dll");
            
            if (huser32)
            {
                typedef BOOL(__stdcall* SETPROCESSDPIAWAREPROC)();

                SETPROCESSDPIAWAREPROC set_aware = 
                    (SETPROCESSDPIAWAREPROC)GetProcAddress(huser32, "SetProcessDPIAware");

                if (set_aware)
                    set_aware();
            }
        }

        timeBeginPeriod(1);
        dinput_hook();
        break;
    }
    case DLL_PROCESS_DETACH:
    {
        dprintf("cnc-ddraw DLL_PROCESS_DETACH\n");

        cfg_save();

        timeEndPeriod(1);
        hook_exit();
        dinput_unhook();
        break;
    }
    }

    return TRUE;
}

HRESULT WINAPI DirectDrawEnumerateA(LPDDENUMCALLBACK lpCallback, LPVOID lpContext)
{
    dprintf("-> %s(lpCallback=%p, lpContext=%p)\n", __FUNCTION__, lpCallback, lpContext);

    if (lpCallback)
        lpCallback(NULL, "display", "(null)", lpContext);

    dprintf("<- %s\n", __FUNCTION__);
    return DD_OK;
}

HRESULT WINAPI DirectDrawCreateClipper(DWORD dwFlags, LPDIRECTDRAWCLIPPER FAR* lplpDDClipper, IUnknown FAR* pUnkOuter)
{
    dprintf("-> %s(dwFlags=%08X, DDClipper=%p, unkOuter=%p)\n", __FUNCTION__, (int)dwFlags, lplpDDClipper, pUnkOuter);
    HRESULT ret = dd_CreateClipper(dwFlags, lplpDDClipper, pUnkOuter);
    dprintf("<- %s\n", __FUNCTION__);
    return ret;
}

HRESULT WINAPI DirectDrawCreate(GUID FAR* lpGUID, LPDIRECTDRAW FAR* lplpDD, IUnknown FAR* pUnkOuter)
{
    dprintf("-> %s(lpGUID=%p, lplpDD=%p, pUnkOuter=%p)\n", __FUNCTION__, lpGUID, lplpDD, pUnkOuter);

    if (g_ddraw)
    {
        /* FIXME: check the calling module before passing the call! */
        if (g_ddraw->DirectDrawCreate)
        {
            dprintf("<- %s\n", __FUNCTION__);
            return g_ddraw->DirectDrawCreate(lpGUID, lplpDD, pUnkOuter);
        }
            
        dprintf("<- %s\n", __FUNCTION__);
        return DDERR_DIRECTDRAWALREADYCREATED;
    }

    g_ddraw = (cnc_ddraw*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(cnc_ddraw));

    IDirectDrawImpl* dst_ddraw = (IDirectDrawImpl*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(IDirectDrawImpl));
    dst_ddraw->lpVtbl = &g_dd_vtbl1;

    *lplpDD = (LPDIRECTDRAW)dst_ddraw;
    IDirectDraw_AddRef(dst_ddraw);

    if (!g_ddraw->real_dll)
    {
        g_ddraw->real_dll = LoadLibrary("system32\\ddraw.dll");
    }

    if (g_ddraw->real_dll && !g_ddraw->DirectDrawCreate)
    {
        g_ddraw->DirectDrawCreate =
            (HRESULT(WINAPI*)(GUID FAR*, LPDIRECTDRAW FAR*, IUnknown FAR*))GetProcAddress(g_ddraw->real_dll, "DirectDrawCreate");

        if (g_ddraw->DirectDrawCreate == DirectDrawCreate)
            g_ddraw->DirectDrawCreate = NULL;
    }

    InitializeCriticalSection(&g_ddraw->cs);

    g_ddraw->render.sem = CreateSemaphore(NULL, 0, 1, NULL);
    g_ddraw->wine = GetProcAddress(GetModuleHandleA("ntdll.dll"), "wine_get_version") != 0;

    cfg_load();

    dprintf("<- %s\n", __FUNCTION__);
    return DD_OK;
}
