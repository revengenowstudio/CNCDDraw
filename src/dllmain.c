#include <windows.h>
#include <ddraw.h>
#include <stdio.h>
#include "dllmain.h"
#include "IDirectDraw.h"
#include "dd.h"
#include "ddclipper.h"
#include "debug.h"
#include "config.h"
#include "directinput.h"
#include "hook.h"

// exports to force usage of discrete high performance graphics device
DWORD NvOptimusEnablement = 1;
DWORD AmdPowerXpressRequestHighPerformance = 1;

// export for cncnet cnc games
BOOL GameHandlesClose;

// export for some warcraft II tools
PVOID FakePrimarySurface;


HMODULE g_ddraw_module;

BOOL WINAPI DllMain(HANDLE hDll, DWORD dwReason, LPVOID lpReserved)
{
    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
    {
#if _DEBUG 
        dbg_init();
        dprintf("cnc-ddraw = %p\n", hDll);
#endif
        g_ddraw_module = hDll;

        char buf[1024];

        if (GetEnvironmentVariable("__COMPAT_LAYER", buf, sizeof(buf)))
        {
            char* s = strtok(buf, " ");

            while (s)
            {
                if (_strcmpi(s, "WIN95") == 0 || _strcmpi(s, "WIN98") == 0 || _strcmpi(s, "NT4SP5") == 0)
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

HRESULT WINAPI DirectDrawCreate(GUID FAR* lpGUID, LPDIRECTDRAW FAR* lplpDD, IUnknown FAR* pUnkOuter)
{
    dprintf("-> %s(lpGUID=%p, lplpDD=%p, pUnkOuter=%p)\n", __FUNCTION__, lpGUID, lplpDD, pUnkOuter);
    HRESULT ret = dd_CreateEx(lpGUID, (LPVOID*)lplpDD, &IID_IDirectDraw, pUnkOuter);
    dprintf("<- %s\n", __FUNCTION__);
    return ret;
}

HRESULT WINAPI DirectDrawCreateClipper(DWORD dwFlags, LPDIRECTDRAWCLIPPER FAR* lplpDDClipper, IUnknown FAR* pUnkOuter)
{
    dprintf("-> %s(dwFlags=%08X, DDClipper=%p, unkOuter=%p)\n", __FUNCTION__, (int)dwFlags, lplpDDClipper, pUnkOuter);
    HRESULT ret = dd_CreateClipper(dwFlags, lplpDDClipper, pUnkOuter);
    dprintf("<- %s\n", __FUNCTION__);
    return ret;
}

HRESULT WINAPI DirectDrawCreateEx(GUID* lpGuid, LPVOID* lplpDD, REFIID iid, IUnknown* pUnkOuter)
{
    dprintf("-> %s(lpGUID=%p, lplpDD=%p, riid=%08X, pUnkOuter=%p)\n", __FUNCTION__, lpGuid, lplpDD, iid, pUnkOuter);
    HRESULT ret = dd_CreateEx(lpGuid, lplpDD, iid, pUnkOuter);
    dprintf("<- %s\n", __FUNCTION__);
    return ret;
}

HRESULT WINAPI DirectDrawEnumerateA(LPDDENUMCALLBACK lpCallback, LPVOID lpContext)
{
    dprintf("-> %s(lpCallback=%p, lpContext=%p)\n", __FUNCTION__, lpCallback, lpContext);

    if (lpCallback)
        lpCallback(NULL, "display", "(null)", lpContext);

    dprintf("<- %s\n", __FUNCTION__);
    return DD_OK;
}

HRESULT WINAPI DirectDrawEnumerateExA(LPDDENUMCALLBACKEXA lpCallback, LPVOID lpContext, DWORD dwFlags)
{
    dprintf("-> %s(lpCallback=%p, lpContext=%p, dwFlags=%d)\n", __FUNCTION__, lpCallback, lpContext, dwFlags);

    if (lpCallback)
        lpCallback(NULL, "display", "(null)", lpContext, NULL);

    dprintf("<- %s\n", __FUNCTION__);
    return DD_OK;
}

HRESULT WINAPI DirectDrawEnumerateExW(LPDDENUMCALLBACKEXW lpCallback, LPVOID lpContext, DWORD dwFlags)
{
    dprintf("-> %s(lpCallback=%p, lpContext=%p, dwFlags=%d)\n", __FUNCTION__, lpCallback, lpContext, dwFlags);

    if (lpCallback)
        lpCallback(NULL, L"display", L"(null)", lpContext, NULL);

    dprintf("<- %s\n", __FUNCTION__);
    return DD_OK;
}

HRESULT WINAPI DirectDrawEnumerateW(LPDDENUMCALLBACKW lpCallback, LPVOID lpContext)
{
    dprintf("-> %s(lpCallback=%p, lpContext=%p)\n", __FUNCTION__, lpCallback, lpContext);

    if (lpCallback)
        lpCallback(NULL, L"display", L"(null)", lpContext);

    dprintf("<- %s\n", __FUNCTION__);
    return DD_OK;
}
