#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <d3d9.h>
#include "config.h"
#include "dd.h"
#include "render_d3d9.h"
#include "render_gdi.h"
#include "render_ogl.h"
#include "hook.h"
#include "debug.h"


static BOOL cfg_get_bool(LPCSTR key, BOOL default_value);
static int cfg_get_int(LPCSTR key, int default_value);
static DWORD cfg_get_string(LPCSTR key, LPCSTR default_value, LPSTR out_string, DWORD out_size);
static void cfg_create_ini();

cnc_ddraw_config g_config = 
    { .window_rect = { .left = -32000, .top = -32000, .right = 0, .bottom = 0 }, .window_state = -1 };

void cfg_load()
{
    //set up settings ini
    char cwd[MAX_PATH];
    char tmp[256];
    GetCurrentDirectoryA(sizeof(cwd), cwd);
    _snprintf(g_config.ini_path, sizeof(g_config.ini_path), "%s\\ddraw.ini", cwd);

    if (GetFileAttributes(g_config.ini_path) == INVALID_FILE_ATTRIBUTES)
        cfg_create_ini();

    //get process filename
    char process_file_path[MAX_PATH] = { 0 };
    GetModuleFileNameA(NULL, process_file_path, MAX_PATH);
    _splitpath(process_file_path, NULL, NULL, g_config.process_file_name, NULL);

    //load settings from ini
    g_ddraw->windowed = cfg_get_bool("windowed", FALSE);
    g_ddraw->border = cfg_get_bool("border", TRUE);
    g_ddraw->boxing = cfg_get_bool("boxing", FALSE);
    g_ddraw->maintas = cfg_get_bool("maintas", FALSE);
    g_ddraw->adjmouse = cfg_get_bool("adjmouse", FALSE);
    g_ddraw->devmode = cfg_get_bool("devmode", FALSE);
    g_ddraw->vsync = cfg_get_bool("vsync", FALSE);
    g_ddraw->noactivateapp = cfg_get_bool("noactivateapp", FALSE);
    g_ddraw->vhack = cfg_get_bool("vhack", FALSE);
    g_ddraw->accurate_timers = cfg_get_bool("accuratetimers", FALSE);
    g_ddraw->resizable = cfg_get_bool("resizable", TRUE);
    g_ddraw->nonexclusive = cfg_get_bool("nonexclusive", FALSE);
    g_ddraw->sierrahack = cfg_get_bool("sierrahack", FALSE); // Sierra Caesar III, Pharaoh, and Zeus hack

    g_config.window_rect.right = cfg_get_int("width", 0);
    g_config.window_rect.bottom = cfg_get_int("height", 0);
    g_config.window_rect.left = cfg_get_int("posX", -32000);
    g_config.window_rect.top = cfg_get_int("posY", -32000);

    g_config.save_settings = cfg_get_int("savesettings", 2);

#ifdef _MSC_VER
    g_hook_method = cfg_get_int("hook", 4);
#endif
    
    g_ddraw->render.maxfps = cfg_get_int("maxfps", 60);

    if (g_ddraw->render.maxfps)
        g_ddraw->render.forcefps = cfg_get_bool("forcefps", FALSE);

    if (g_ddraw->accurate_timers || g_ddraw->vsync)
        g_ddraw->fps_limiter.htimer = CreateWaitableTimer(NULL, TRUE, NULL);
    //can't fully set it up here due to missing g_ddraw->mode.dmDisplayFrequency

    int max_ticks = cfg_get_int("maxgameticks", 0);

    if (max_ticks > 0 && max_ticks <= 1000)
    {
        if (g_ddraw->accurate_timers)
            g_ddraw->ticks_limiter.htimer = CreateWaitableTimer(NULL, TRUE, NULL);

        float len = 1000.0f / max_ticks;
        g_ddraw->ticks_limiter.tick_length_ns = len * 10000;
        g_ddraw->ticks_limiter.tick_length = len + 0.5f;
    }

    if (max_ticks >= 0)
    {
        //always using 60 fps for flip...
        if (g_ddraw->accurate_timers)
            g_ddraw->flip_limiter.htimer = CreateWaitableTimer(NULL, TRUE, NULL);

        float flip_len = 1000.0f / 60;
        g_ddraw->flip_limiter.tick_length_ns = flip_len * 10000;
        g_ddraw->flip_limiter.tick_length = flip_len + 0.5f;
    }

    if ((g_ddraw->fullscreen = cfg_get_bool("fullscreen", FALSE)))
    {
        g_config.window_rect.left = g_config.window_rect.top = -32000;
    }

    if (!(g_ddraw->handlemouse = cfg_get_bool("handlemouse", TRUE)))
    {
        g_ddraw->adjmouse = TRUE;
    }

    if (cfg_get_bool("singlecpu", TRUE))
    {
        SetProcessAffinityMask(GetCurrentProcess(), 1);
    }
    else
    {
        DWORD system_affinity;
        DWORD proc_affinity;
        HANDLE proc = GetCurrentProcess();

        if (GetProcessAffinityMask(proc, &proc_affinity, &system_affinity))
            SetProcessAffinityMask(proc, system_affinity);
    }

    g_ddraw->render.bpp = cfg_get_int("bpp", 0);

    if (g_ddraw->render.bpp != 16 && g_ddraw->render.bpp != 24 && g_ddraw->render.bpp != 32)
    {
        g_ddraw->render.bpp = 0;
    }

    // to do: read .glslp config file instead of the shader and apply the correct settings
    cfg_get_string("shader", "", g_ddraw->shader, sizeof(g_ddraw->shader));

    cfg_get_string("renderer", "auto", tmp, sizeof(tmp));
    dprintf("     Using %s renderer\n", tmp);

    if (tolower(tmp[0]) == 's' || tolower(tmp[0]) == 'g') //gdi
    {
        g_ddraw->renderer = gdi_render_main;
    }
    else if (tolower(tmp[0]) == 'd') //direct3d9
    {
        g_ddraw->renderer = d3d9_render_main;
    }
    else if (tolower(tmp[0]) == 'o') //opengl
    {
        if (oglu_load_dll())
        {
            g_ddraw->renderer = ogl_render_main;
        }
        else
        {
            g_ddraw->show_driver_warning = TRUE;
            g_ddraw->renderer = gdi_render_main;
        }
    }
    else //auto
    {
        if (!g_ddraw->wine && d3d9_is_available())
        {
            g_ddraw->renderer = d3d9_render_main;
        }
        else if (oglu_load_dll())
        {
            g_ddraw->renderer = ogl_render_main;
        }
        else
        {
            g_ddraw->show_driver_warning = TRUE;
            g_ddraw->renderer = gdi_render_main;
        }
    }
}

void cfg_save()
{
    if (!g_config.save_settings)
        return;

    char buf[16];
    char *section = g_config.save_settings == 1 ? "ddraw" : g_config.process_file_name;

    if (g_config.window_rect.right)
    {
        sprintf(buf, "%ld", g_config.window_rect.right);
        WritePrivateProfileString(section, "width", buf, g_config.ini_path);
    }
    
    if (g_config.window_rect.bottom)
    {
        sprintf(buf, "%ld", g_config.window_rect.bottom);
        WritePrivateProfileString(section, "height", buf, g_config.ini_path);
    }

    if (g_config.window_rect.left != -32000)
    {
        sprintf(buf, "%ld", g_config.window_rect.left);
        WritePrivateProfileString(section, "posX", buf, g_config.ini_path);
    }

    if (g_config.window_rect.top != -32000)
    {
        sprintf(buf, "%ld", g_config.window_rect.top);
        WritePrivateProfileString(section, "posY", buf, g_config.ini_path);
    }

    if (g_config.window_state != -1)
    {
        WritePrivateProfileString(section, "windowed", g_config.window_state ? "true" : "false", g_config.ini_path);
    }
}

static void cfg_create_ini()
{
    FILE *fh = fopen(g_config.ini_path, "w");
    if (fh)
    {
        fputs(
            "; cnc-ddraw - https://github.com/CnCNet/cnc-ddraw - https://cncnet.org\n"
            "\n"
            "[ddraw]\n"
            "; ### Optional settings ###\n"
            "; Use the following settings to adjust the look and feel to your liking\n"
            "\n"
            "\n"
            "; Stretch to custom resolution, 0 = defaults to the size game requests\n"
            "width=0\n"
            "height=0\n"
            "\n"
            "; Override the width/height settings shown above and always stretch to fullscreen\n"
            "; Note: Can be combined with 'windowed=true' to get windowed-fullscreen aka borderless mode\n"
            "fullscreen=false\n"
            "\n"
            "; Run in windowed mode rather than going fullscreen\n"
            "windowed=false\n"
            "\n"
            "; Maintain aspect ratio - (Requires 'handlemouse=true')\n"
            "maintas=false\n"
            "\n"
            "; Windowboxing / Integer Scaling - (Requires 'handlemouse=true')\n"
            "boxing=false\n"
            "\n"
            "; Real rendering rate, -1 = screen rate, 0 = unlimited, n = cap\n"
            "; Note: Does not have an impact on the game speed, to limit your game speed use 'maxgameticks='\n"
            "maxfps=60\n"
            "\n"
            "; Vertical synchronization, enable if you get tearing - (Requires 'renderer=auto/opengl/direct3d9')\n"
            "; Note: vsync=true can fix tearing but it will cause input lag\n"
            "vsync=false\n"
            "\n"
            "; Automatic mouse sensitivity scaling  - (Requires 'handlemouse=true')\n"
            "; Note: Only works if stretching is enabled. Sensitivity will be adjusted according to the size of the window\n"
            "adjmouse=false\n"
            "\n"
            "; Preliminary libretro shader support - (Requires 'renderer=opengl') https://github.com/libretro/glsl-shaders\n"
            "; 2x scaling example: https://imgur.com/a/kxsM1oY - 4x scaling example: https://imgur.com/a/wjrhpFV\n"
            "shader=Shaders\\simple-sharp-bilinear.glsl\n"
            "\n"
            "; Window position, -32000 = center to screen\n"
            "posX=-32000\n"
            "posY=-32000\n"
            "\n"
            "; Renderer, possible values: auto, opengl, gdi, direct3d9 (auto = try direct3d9/opengl, fallback = gdi)\n"
            "renderer=auto\n"
            "\n"
            "; Developer mode (don't lock the cursor)\n"
            "devmode=false\n"
            "\n"
            "; Show window borders in windowed mode\n"
            "border=true\n"
            "\n"
            "; Save window position/size/state on game exit and restore it automatically on next game start\n"
            "; Possible values: 0 = disabled, 1 = save to global 'ddraw' section, 2 = save to game specific section\n"
            "savesettings=2\n"
            "\n"
            "; Should the window be resizeable by the user in windowed mode?\n"
            "resizeable=true\n"
            "\n"
            "; Enable C&C video resize hack - Stretches C&C cutscenes to fullscreen\n"
            "vhack=false\n"
            "\n"
            "\n"
            "\n"
            "; ### Compatibility settings ###\n"
            "; Use the following settings in case there are any issues with the game\n"
            "\n"
            "\n"
            "; Hide WM_ACTIVATEAPP and WM_NCACTIVATE messages to prevent problems on alt+tab\n"
            "noactivateapp=false\n"
            "\n"
            "; Max game ticks per second, possible values: -1 = disabled, 0 = emulate 60hz vblank, 1-1000 = custom game speed\n"
            "; Note: Can be used to slow down a too fast running game, fix flickering or too fast animations\n"
            "; Note: Usually one of the following values will work: 60 / 30 / 25 / 20 / 15 (lower value = slower game speed)\n"
            "maxgameticks=0\n"
            "\n"
            "; Gives cnc-ddraw full control over the mouse cursor (required for adjmouse/boxing/maintas)\n"
            "; Note: Set this to 'false' if your cursor becomes invisible at some places in the game\n"
            "handlemouse=true\n"
            "\n"
            "; Windows API Hooking, Possible values: 0 = disabled, 1 = IAT Hooking, 2 = Microsoft Detours, 3 = IAT+Detours Hooking (All Modules), 4 = IAT Hooking (All Modules)\n"
            "; Note: Can be used to fix issues related to new features added by cnc-ddraw such as windowed mode or stretching\n"
            "hook=4\n"
            "\n"
            "; Force consistent FPS (Requires 'maxfps=' to be set to a value other than 0)\n"
            "; Note: Fixes flickering cursor issues in C&C games (Might be useful for some screen recorders too)\n"
            "forcefps=false\n"
            "\n"
            "; Disable fullscreen-exclusive mode for the OpenGL renderer\n"
            "; Note: Can be used in case some GUI elements like buttons/textboxes/videos/etc.. are invisible\n"
            "nonexclusive=false\n"
            "\n"
            "; Force CPU0 affinity, avoids crashes/freezing, *might* have a performance impact\n"
            "singlecpu=true\n"
            "\n"
            "\n"
            "\n"
            "; ### Game specific settings ###\n"
            "; The following settings override all settings shown above, section name = executable name\n"
            "\n"
            "\n"
            "; Command & Conquer: Red Alert - CnCNet\n"
            "[ra95-spawn]\n"
            "maxfps=125\n"
            "\n"
            "; Command & Conquer Gold - CnCNet\n"
            "[cnc95]\n"
            "maxfps=125\n"
            "\n"
            "; Carmageddon\n"
            "[CARMA95]\n"
            "renderer=opengl\n"
            "noactivateapp=true\n"
            "maxgameticks=60\n"
            "\n"
            "; Carmageddon\n"
            "[CARM95]\n"
            "renderer=opengl\n"
            "noactivateapp=true\n"
            "maxgameticks=60\n"
            "\n"
            "; Command & Conquer Gold\n"
            "[C&C95]\n"
            "maxgameticks=120\n"
            "maxfps=60\n"
            "forcefps=true\n"
            "\n"
            "; Command & Conquer: Red Alert\n"
            "[ra95]\n"
            "maxgameticks=120\n"
            "maxfps=60\n"
            "forcefps=true\n"
            "\n"
            "; Command & Conquer: Red Alert\n"
            "[ra95p]\n"
            "maxfps=60\n"
            "forcefps=true\n"
            "\n"
            "; Age of Empires\n"
            "[empires]\n"
            "handlemouse=false\n"
            "\n"
            "; Age of Empires: The Rise of Rome\n"
            "[empiresx]\n"
            "handlemouse=false\n"
            "\n"
            "; Age of Empires II\n"
            "[EMPIRES2]\n"
            "handlemouse=false\n"
            "\n"
            "; Age of Empires II: The Conquerors\n"
            "[age2_x1]\n"
            "handlemouse=false\n"
            "\n"
            "; Outlaws\n"
            "[olwin]\n"
            "noactivateapp=true\n"
            "maxgameticks=60\n"
            "handlemouse=false\n"
            "renderer=gdi\n"
            "\n"
            "; Dark Reign: The Future of War\n"
            "[DKReign]\n"
            "maxgameticks=60\n"
            "\n"
            "; Star Wars: Galactic Battlegrounds\n"
            "[battlegrounds]\n"
            "handlemouse=false\n"
            "\n"
            "; Star Wars: Galactic Battlegrounds: Clone Campaigns\n"
            "[battlegrounds_x1]\n"
            "handlemouse=false\n"
            "\n"
            "; Carmageddon 2\n"
            "[Carma2_SW]\n"
            "renderer=opengl\n"
            "noactivateapp=true\n"
            "maxgameticks=60\n"
            "\n"
            "; Atomic Bomberman\n"
            "[BM]\n"
            "maxgameticks=60\n"
            "\n"
            "; Dune 2000\n"
            "[dune2000]\n"
            "maxfps=59\n"
            "accuratetimers=true\n"
            "\n"
            "; Dune 2000 - CnCNet\n"
            "[dune2000-spawn]\n"
            "maxfps=59\n"
            "accuratetimers=true\n"
            "\n"
            "; Command & Conquer: Tiberian Sun / Command & Conquer: Red Alert 2\n"
            "[game]\n"
            "checkfile=.\\blowfish.dll\n"
            "noactivateapp=true\n"
            "handlemouse=false\n"
            "maxfps=60\n"
            "forcefps=true\n"
            "\n"
            "; Command & Conquer: Tiberian Sun Demo\n"
            "[SUN]\n"
            "noactivateapp=true\n"
            "handlemouse=false\n"
            "maxfps=60\n"
            "forcefps=true\n"
            "\n"
            "; Command & Conquer: Tiberian Sun - CnCNet\n"
            "[ts-spawn]\n"
            "noactivateapp=true\n"
            "handlemouse=false\n"
            "maxfps=60\n"
            "forcefps=true\n"
            "\n"
            "; Command & Conquer: Red Alert 2 - XWIS\n"
            "[ra2]\n"
            "noactivateapp=true\n"
            "handlemouse=false\n"
            "maxfps=60\n"
            "forcefps=true\n"
            "\n"
            "; Command & Conquer: Red Alert 2 - XWIS\n"
            "[Red Alert 2]\n"
            "noactivateapp=true\n"
            "handlemouse=false\n"
            "maxfps=60\n"
            "forcefps=true\n"
            "\n"
            "; Command & Conquer: Red Alert 2: Yuri's Revenge\n"
            "[gamemd]\n"
            "noactivateapp=true\n"
            "handlemouse=false\n"
            "maxfps=60\n"
            "forcefps=true\n"
            "\n"
            "; Command & Conquer: Red Alert 2: Yuri's Revenge - ?ModExe?\n"
            "[ra2md]\n"
            "noactivateapp=true\n"
            "handlemouse=false\n"
            "maxfps=60\n"
            "forcefps=true\n"
            "\n"
            "; Command & Conquer: Red Alert 2: Yuri's Revenge - CnCNet\n"
            "[gamemd-spawn]\n"
            "noactivateapp=true\n"
            "handlemouse=false\n"
            "maxfps=60\n"
            "forcefps=true\n"
            "\n"
            "; Command & Conquer: Red Alert 2: Yuri's Revenge - XWIS\n"
            "[Yuri's Revenge]\n"
            "noactivateapp=true\n"
            "handlemouse=false\n"
            "maxfps=60\n"
            "forcefps=true\n"
            "\n"
            "; Twisted Metal\n"
            "[TWISTED]\n"
            "renderer=opengl\n"
            "nonexclusive=true\n"
            "maxgameticks=25\n"
            "\n"
            "; Twisted Metal 2\n"
            "[Tm2]\n"
            "renderer=opengl\n"
            "nonexclusive=true\n"
            "maxgameticks=60\n"
            "handlemouse=false\n"
            "\n"
            "; Caesar III\n"
            "[c3]\n"
            "handlemouse=false\n"
            "sierrahack=true\n"
            "\n"
            "; Pharaoh\n"
            "[Pharaoh]\n"
            "handlemouse=false\n"
            "sierrahack=true\n"
            "\n"
            "; Master of Olympus - Zeus\n"
            "[Zeus]\n"
            "handlemouse=false\n"
            "sierrahack=true\n"
            "renderer=gdi\n"
            "hook=2\n"
            "\n"

            , fh);
        fclose(fh);
    }
}

static DWORD cfg_get_string(LPCSTR key, LPCSTR default_value, LPSTR out_string, DWORD out_size)
{
    DWORD s = GetPrivateProfileStringA(
        g_config.process_file_name, key, "", out_string, out_size, g_config.ini_path);

    if (s > 0)
    {
        char buf[MAX_PATH] = { 0 };

        if (GetPrivateProfileStringA(
            g_config.process_file_name, "checkfile", "", buf, sizeof(buf), g_config.ini_path) > 0)
        {
            if (GetFileAttributes(buf) != INVALID_FILE_ATTRIBUTES)
                return s;
        }
        else
            return s;
    }

    return GetPrivateProfileStringA("ddraw", key, default_value, out_string, out_size, g_config.ini_path);
}

static BOOL cfg_get_bool(LPCSTR key, BOOL default_value)
{
    char value[8];
    cfg_get_string(key, default_value ? "Yes" : "No", value, sizeof(value));

    return (_stricmp(value, "yes") == 0 || _stricmp(value, "true") == 0 || _stricmp(value, "1") == 0);
}

static int cfg_get_int(LPCSTR key, int default_value)
{
    char def_value[16];
    _snprintf(def_value, sizeof(def_value), "%d", default_value);

    char value[16];
    cfg_get_string(key, def_value, value, sizeof(value));

    return atoi(value);
}
