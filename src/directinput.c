
#include <windows.h>
#include <dinput.h>
#include "debug.h"
#include "hook.h"
#include "dd.h"

typedef HRESULT (WINAPI *DIRECTINPUTCREATEAPROC)(HINSTANCE, DWORD, LPDIRECTINPUTA*, LPUNKNOWN);
typedef HRESULT (WINAPI *DICREATEDEVICEPROC)(IDirectInputA*, REFGUID, LPDIRECTINPUTDEVICEA *, LPUNKNOWN);
typedef HRESULT (WINAPI *DIDSETCOOPERATIVELEVELPROC)(IDirectInputDeviceA *, HWND, DWORD);
typedef HRESULT (WINAPI *DIDGETDEVICEDATAPROC)(IDirectInputDeviceA*, DWORD, LPDIDEVICEOBJECTDATA, LPDWORD, DWORD);

static DIRECTINPUTCREATEAPROC real_DirectInputCreateA;
static DICREATEDEVICEPROC real_di_CreateDevice;
static DIDSETCOOPERATIVELEVELPROC real_did_SetCooperativeLevel;
static DIDGETDEVICEDATAPROC real_did_GetDeviceData;

static PROC hook_func(PROC *org_func, PROC new_func)
{
    PROC org = *org_func;
    DWORD old_protect;

    if (VirtualProtect(org_func, sizeof(PROC), PAGE_EXECUTE_READWRITE, &old_protect))
    {
        *org_func = new_func;
        VirtualProtect(org_func, sizeof(PROC), old_protect, &old_protect);
        return org;
    }

    return 0;
}

static HRESULT WINAPI fake_did_SetCooperativeLevel(IDirectInputDeviceA *This, HWND hwnd, DWORD dwFlags)
{
    return real_did_SetCooperativeLevel(This, hwnd, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE);
}

static HRESULT WINAPI fake_did_GetDeviceData(IDirectInputDeviceA *This, DWORD cbObjectData, LPDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD dwFlags)
{
    HRESULT result = real_did_GetDeviceData(This, cbObjectData, rgdod, pdwInOut, dwFlags);

    if (SUCCEEDED(result) && g_ddraw && !g_ddraw->locked)
    {
        *pdwInOut = 0;
    }

    return result;
}

static HRESULT WINAPI fake_di_CreateDevice(IDirectInputA *This, REFGUID rguid, LPDIRECTINPUTDEVICEA * lplpDIDevice, LPUNKNOWN pUnkOuter)
{
    HRESULT result = real_di_CreateDevice(This, rguid, lplpDIDevice, pUnkOuter);

    if (SUCCEEDED(result) && !real_did_SetCooperativeLevel)
    {
        real_did_SetCooperativeLevel =
            (DIDSETCOOPERATIVELEVELPROC)hook_func(
                (PROC *)&(*lplpDIDevice)->lpVtbl->SetCooperativeLevel, (PROC)fake_did_SetCooperativeLevel);

        real_did_GetDeviceData =
            (DIDGETDEVICEDATAPROC)hook_func(
                (PROC*)&(*lplpDIDevice)->lpVtbl->GetDeviceData, (PROC)fake_did_GetDeviceData);
    }

    return result;
}

static HRESULT WINAPI fake_DirectInputCreateA(HINSTANCE hinst, DWORD dwVersion, LPDIRECTINPUTA* lplpDirectInput, LPUNKNOWN punkOuter)
{
    dprintf("DirectInputCreateA\n");

    real_DirectInputCreateA =
        (DIRECTINPUTCREATEAPROC)GetProcAddress(GetModuleHandle("dinput.dll"), "DirectInputCreateA");

    if (!real_DirectInputCreateA)
        return DIERR_GENERIC;

    HRESULT result = real_DirectInputCreateA(hinst, dwVersion, lplpDirectInput, punkOuter);

    if (SUCCEEDED(result) && !real_di_CreateDevice)
    {
        real_di_CreateDevice =
            (DICREATEDEVICEPROC)hook_func((PROC *)&(*lplpDirectInput)->lpVtbl->CreateDevice, (PROC)fake_di_CreateDevice);
    }

    return result;
}

void dinput_hook()
{
    hook_patch_iat(GetModuleHandle(NULL), "dinput.dll", "DirectInputCreateA", (PROC)fake_DirectInputCreateA);
}

void dinput_unhook()
{
    hook_patch_iat(
        GetModuleHandle(NULL), 
        "dinput.dll", 
        "DirectInputCreateA", 
        (PROC)GetProcAddress(GetModuleHandle("dinput.dll"), "DirectInputCreateA"));
}
