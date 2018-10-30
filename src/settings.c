#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <d3d9.h>
#include "main.h"
#include "opengl.h"
#include "render_d3d9.h"

static char SettingsIniPath[MAX_PATH];
static char ProcessFileName[96];

static BOOL GetBool(LPCSTR key, BOOL defaultValue);
static int GetInt(LPCSTR key, int defaultValue);
static DWORD GetString(LPCSTR key, LPCSTR defaultValue, LPSTR outString, DWORD outSize);
static void CreateSettingsIni();

void Settings_Load()
{
    //set up settings ini
    char cwd[MAX_PATH];
    char tmp[256];
    GetCurrentDirectoryA(sizeof(cwd), cwd);
    _snprintf(SettingsIniPath, sizeof(SettingsIniPath), "%s\\ddraw.ini", cwd);

    if (GetFileAttributes(SettingsIniPath) == INVALID_FILE_ATTRIBUTES)
        CreateSettingsIni();

    //get process filename
    char ProcessFilePath[MAX_PATH] = { 0 };
    GetModuleFileNameA(NULL, ProcessFilePath, MAX_PATH);
    _splitpath(ProcessFilePath, NULL, NULL, ProcessFileName, NULL);

    //load settings from ini
    ddraw->windowed = GetBool("windowed", FALSE);
    ddraw->border = GetBool("border", TRUE);
    ddraw->boxing = GetBool("boxing", FALSE);
    ddraw->maintas = GetBool("maintas", FALSE);
    ddraw->adjmouse = GetBool("adjmouse", FALSE);
    ddraw->devmode = GetBool("devmode", FALSE);
    ddraw->vsync = GetBool("vsync", FALSE);
    ddraw->fakecursorpos = GetBool("fakecursorpos", TRUE);
    ddraw->noactivateapp = GetBool("noactivateapp", FALSE);
    ddraw->vhack = GetBool("vhack", FALSE);
    ddraw->hidemouse = GetBool("hidemouse", TRUE);

    ddraw->sleep = GetInt("sleep", 0);
    ddraw->render.maxfps = GetInt("maxfps", 125);
    WindowRect.right = GetInt("width", 0);
    WindowRect.bottom = GetInt("height", 0);
    WindowRect.left = GetInt("posX", -32000);
    WindowRect.top = GetInt("posY", -32000);

    GetString("screenshotKey", "G", tmp, sizeof(tmp));
    ddraw->screenshotKey = toupper(tmp[0]);

    if ((ddraw->fullscreen = GetBool("fullscreen", FALSE)))
        WindowRect.left = WindowRect.top = -32000;

    if (GetBool("singlecpu", TRUE))
        SetProcessAffinityMask(GetCurrentProcess(), 1);

    ddraw->render.bpp = GetInt("bpp", 32);
    if (ddraw->render.bpp != 16 && ddraw->render.bpp != 24 && ddraw->render.bpp != 32)
        ddraw->render.bpp = 0;

    // to do: read .glslp config file instead of the shader and apply the correct settings
    GetString("shader", "", ddraw->shader, sizeof(ddraw->shader));

    GetString("renderer", "auto", tmp, sizeof(tmp));
    printf("Using %s renderer\n", tmp);

    if (tolower(tmp[0]) == 's' || tolower(tmp[0]) == 'g') //gdi
    {
        ddraw->renderer = render_soft_main;
    }
    else if (tolower(tmp[0]) == 'd') //direct3d9
    {
        ddraw->renderer = render_d3d9_main;
    }
    else if (tolower(tmp[0]) == 'o') //opengl
    {
        if (OpenGL_LoadDll())
        {
            ddraw->renderer = render_main;
        }
        else
        {
            ShowDriverWarning = TRUE;
            ddraw->renderer = render_soft_main;
        }
    }
    else //auto
    {
        LPDIRECT3D9 d3d = NULL;

        // Windows = Direct3D 9, Wine = OpenGL
        if (!ddraw->wine && (Direct3D9_hModule = LoadLibrary("d3d9.dll")))
        {
            IDirect3D9 *(WINAPI *D3DCreate9)(UINT) =
                (IDirect3D9 *(WINAPI *)(UINT))GetProcAddress(Direct3D9_hModule, "Direct3DCreate9");

            if (D3DCreate9 && (d3d = D3DCreate9(D3D_SDK_VERSION)))
                d3d->lpVtbl->Release(d3d);
        }

        if (d3d)
        {
            ddraw->renderer = render_d3d9_main;
        }
        else if (OpenGL_LoadDll())
        {
            ddraw->renderer = render_main;
        }
        else
        {
            ShowDriverWarning = TRUE;
            ddraw->renderer = render_soft_main;
        }
    }
}

void Settings_SaveWindowRect(RECT *lpRect)
{
    char buf[16];

    if (lpRect->right)
    {
        sprintf(buf, "%ld", lpRect->right);
        WritePrivateProfileString(ProcessFileName, "width", buf, SettingsIniPath);
    }
    
    if (lpRect->bottom)
    {
        sprintf(buf, "%ld", lpRect->bottom);
        WritePrivateProfileString(ProcessFileName, "height", buf, SettingsIniPath);
    }

    if (lpRect->left != -32000)
    {
        sprintf(buf, "%ld", lpRect->left);
        WritePrivateProfileString(ProcessFileName, "posX", buf, SettingsIniPath);
    }

    if (lpRect->top != -32000)
    {
        sprintf(buf, "%ld", lpRect->top);
        WritePrivateProfileString(ProcessFileName, "posY", buf, SettingsIniPath);
    }
}

static void CreateSettingsIni()
{
    FILE *fh = fopen(SettingsIniPath, "w");
    if (fh)
    {
        fputs(
            "[ddraw]\n"
            "; stretch to custom resolution, 0 = defaults to the size game requests\n"
            "width=0\n"
            "height=0\n"
            "; override width/height and always stretch to fullscreen\n"
            "fullscreen=false\n"
            "; bits per pixel, possible values: 16, 24 and 32, 0 = auto\n"
            "bpp=0\n"
            "; Run in windowed mode rather than going fullscreen\n"
            "windowed=false\n"
            "; show window borders in windowed mode\n"
            "border=true\n"
            "; maintain aspect ratio\n"
            "maintas=false\n"
            "; use letter- or windowboxing to make a best fit (Integer Scaling)\n"
            "boxing=false\n"
            "; real rendering rate, -1 = screen rate, 0 = unlimited, n = cap (OpenGL / Direct3D only)\n"
            "maxfps=125\n"
            "; vertical synchronization, enable if you get tearing (OpenGL / Direct3D only)\n"
            "vsync=false\n"
            "; automatic mouse sensitivity scaling\n"
            "adjmouse=false\n"
            "; enable C&C video resize hack\n"
            "vhack=false\n"
            "; auto, opengl, gdi, direct3d9 (auto = try opengl/direct3d9, fallback = gdi)\n"
            "renderer=auto\n"
            "; force CPU0 affinity, avoids crashes with RA, *might* have a performance impact\n"
            "singlecpu=true\n"
            "; Window position, -32000 = center to screen\n"
            "posX=-32000\n"
            "posY=-32000\n"
            "; Screenshot Hotkey, default = CTRL + G\n"
            "screenshotKey=G\n"
            "; Fake cursor position for games that use GetCursorPos and expect to be in fullscreen\n"
            "fakecursorpos=true\n"
            "; Hide WM_ACTIVATEAPP messages to prevent freezing on alt+tab (Carmageddon)\n"
            "noactivateapp=false\n"
            "; developer mode (don't lock the cursor)\n"
            "devmode=false\n"
            "; preliminary libretro shader support - e.g. cubic.glsl (OpenGL only) https://github.com/libretro/glsl-shaders\n"
            "shader=\n"
            "; Sleep for X ms after drawing each frame (Slows down scrollrate on C&C95 / Prevents visual glitches on Carmageddon)\n"
            "sleep=0\n"
            "; Hide/Show the mouse cursor on lock/unlock (Ctrl+Tab)\n"
            "hidemouse=true\n"
            "\n"
            "[CARMA95]\n"
            "fakecursorpos=false\n"
            "noactivateapp=true\n"
            "sleep=33\n"
            "\n"
            "[C&C95]\n"
            "sleep=10\n"
            "\n"
            "[empires]\n"
            "hidemouse=false\n"
            "border=false\n"
            "posX=0\n"
            "posY=0\n"
            "\n"
            "[empiresx]\n"
            "hidemouse=false\n"
            "border=false\n"
            "posX=0\n"
            "posY=0\n"
            "\n"
            "[EMPIRES2]\n"
            "hidemouse=false\n"
            "border=false\n"
            "posX=0\n"
            "posY=0\n"
            "\n"
            "[age2_x1]\n"
            "hidemouse=false\n"
            "border=false\n"
            "posX=0\n"
            "posY=0\n"
            "\n"
            "[olwin]\n"
            "noactivateapp=true\n"
            "sleep=10\n"
            "\n"
            "[KEEPER95]\n"
            "border=false\n"
            "posX=0\n"
            "posY=0\n"
            "\n"

            , fh);
        fclose(fh);
    }
}

static DWORD GetString(LPCSTR key, LPCSTR defaultValue, LPSTR outString, DWORD outSize)
{
    DWORD s = GetPrivateProfileStringA(ProcessFileName, key, "", outString, outSize, SettingsIniPath);
    if (s > 0)
        return s;

	return GetPrivateProfileStringA("ddraw", key, defaultValue, outString, outSize, SettingsIniPath);
}

static BOOL GetBool(LPCSTR key, BOOL defaultValue)
{
	char value[8];
	GetString(key, defaultValue ? "Yes" : "No", value, sizeof(value));

	return (_stricmp(value, "yes") == 0 || _stricmp(value, "true") == 0 || _stricmp(value, "1") == 0);
}

static int GetInt(LPCSTR key, int defaultValue)
{
	char defvalue[16];
    _snprintf(defvalue, sizeof(defvalue), "%d", defaultValue);

	char value[16];
	GetString(key, defvalue, value, sizeof(value));

	return atoi(value);
}
