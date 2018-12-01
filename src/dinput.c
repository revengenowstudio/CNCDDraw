#include <windows.h>
#include <dinput.h>

void HookIAT(HMODULE hMod, char *moduleName, char *functionName, PROC newFunction);

typedef HRESULT (WINAPI *DInputCreateA)(HINSTANCE, DWORD, LPDIRECTINPUTA*, LPUNKNOWN);
typedef HRESULT (WINAPI *DICreateDevice)(IDirectInputA*, REFGUID, LPDIRECTINPUTDEVICEA *, LPUNKNOWN);
typedef HRESULT (WINAPI *DIDSetCooperativeLevel)(IDirectInputDeviceA *, HWND, DWORD);

static DInputCreateA DInputCreateA_;
static DICreateDevice DICreateDevice_;
static DIDSetCooperativeLevel DIDSetCooperativeLevel_;

static PROC HookFunc(PROC *orgFunc, PROC newFunc)
{
    PROC org = *orgFunc;
    DWORD oldProtect;

    if (VirtualProtect(orgFunc, sizeof(PROC), PAGE_EXECUTE_READWRITE, &oldProtect))
    {
        *orgFunc = newFunc;
        VirtualProtect(orgFunc, sizeof(PROC), oldProtect, &oldProtect);
        return org;
    }

    return 0;
}

static HRESULT WINAPI fake_DIDSetCooperativeLevel(IDirectInputDeviceA *This, HWND hwnd, DWORD dwFlags)
{
    return DIDSetCooperativeLevel_(This, hwnd, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE);
}

static HRESULT WINAPI fake_DICreateDevice(IDirectInputA *This, REFGUID rguid, LPDIRECTINPUTDEVICEA * lplpDIDevice, LPUNKNOWN pUnkOuter)
{
    HRESULT result = DICreateDevice_(This, rguid, lplpDIDevice, pUnkOuter);

    if (SUCCEEDED(result) && !DIDSetCooperativeLevel_)
    {
        DIDSetCooperativeLevel_ = 
            (DIDSetCooperativeLevel)HookFunc(
                (PROC *)&(*lplpDIDevice)->lpVtbl->SetCooperativeLevel, (PROC)fake_DIDSetCooperativeLevel);
    }

    return result;
}

static HRESULT WINAPI fake_DirectInputCreateA(HINSTANCE hinst, DWORD dwVersion, LPDIRECTINPUTA* lplpDirectInput, LPUNKNOWN punkOuter)
{
    DInputCreateA_ = (DInputCreateA)GetProcAddress(GetModuleHandle("dinput.dll"), "DirectInputCreateA");
    if (!DInputCreateA_)
        return DIERR_GENERIC;

    HRESULT result = DInputCreateA_(hinst, dwVersion, lplpDirectInput, punkOuter);

    if (SUCCEEDED(result) && !DICreateDevice_)
    {
        DICreateDevice_ =
            (DICreateDevice)HookFunc((PROC *)&(*lplpDirectInput)->lpVtbl->CreateDevice, (PROC)fake_DICreateDevice);
    }

    return result;
}

void dinput_init()
{
    HookIAT(GetModuleHandle(NULL), "dinput.dll", "DirectInputCreateA", (PROC)fake_DirectInputCreateA);
}
