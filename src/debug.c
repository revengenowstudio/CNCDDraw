#include <windows.h>
#include <dbghelp.h>
#include <stdio.h>
#include <ddraw.h>
#include "dd.h"
#include "ddsurface.h"
#include "debug.h"


double g_dbg_frame_time = 0;
DWORD g_dbg_frame_count = 0;

static LONGLONG g_dbg_counter_start_time = 0;
static double g_dbg_counter_freq = 0.0;

#if _DEBUG 
int dbg_exception_handler(EXCEPTION_POINTERS* exception)
{
    HANDLE dmp =
        CreateFile(
            "cnc-ddraw.dmp",
            GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_WRITE | FILE_SHARE_READ,
            0,
            CREATE_ALWAYS,
            0,
            0);

    if (dmp)
    {
        MINIDUMP_EXCEPTION_INFORMATION info;
        info.ThreadId = GetCurrentThreadId();
        info.ExceptionPointers = exception;
        info.ClientPointers = TRUE;

        MiniDumpWriteDump(
            GetCurrentProcess(),
            GetCurrentProcessId(),
            dmp,
            0,
            &info,
            NULL,
            NULL);

        CloseHandle(dmp);
    }

    return EXCEPTION_EXECUTE_HANDLER;
}
#endif

void dbg_init()
{
    static int stdout_open = 0;

    if (!stdout_open)
    {
        stdout_open = 1;

        freopen("cnc-ddraw.log", "w", stdout);
        setvbuf(stdout, NULL, _IOLBF, 1024);

        HKEY hkey;
        LONG status =
            RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", 0L, KEY_READ, &hkey);

        if (status == ERROR_SUCCESS)
        {
            char name[256] = { 0 };
            DWORD name_size = sizeof(name);
            RegQueryValueExA(hkey, "ProductName", NULL, NULL, (PVOID)&name, &name_size);

            char build[256] = { 0 };
            DWORD build_size = sizeof(build);
            RegQueryValueExA(hkey, "BuildLab", NULL, NULL, (PVOID)&build, &build_size);

            dbg_printf("%s (%s)\n", name, build);

            const char* (CDECL * wine_get_version)() =
                (void*)GetProcAddress(GetModuleHandleA("ntdll.dll"), "wine_get_version");

            if (wine_get_version)
                dbg_printf("Wine version = %s\n", wine_get_version());
        }
    }
}

void dbg_counter_start()
{
    LARGE_INTEGER li;
    QueryPerformanceFrequency(&li);
    g_dbg_counter_freq = (double)(li.QuadPart) / 1000.0;
    QueryPerformanceCounter(&li);
    g_dbg_counter_start_time = li.QuadPart;
}

double dbg_counter_stop()
{
    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
    return (double)(li.QuadPart - g_dbg_counter_start_time) / g_dbg_counter_freq;
}

void dbg_debug_string(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    char buffer[512] = { 0 };
    _vsnprintf(buffer, sizeof(buffer), format, args);
    OutputDebugStringA(buffer);
}

int dbg_printf(const char* fmt, ...)
{
    static CRITICAL_SECTION cs;
    static BOOL initialized;

    if (!initialized)
    {
        initialized = TRUE;
        InitializeCriticalSection(&cs);
    }

    EnterCriticalSection(&cs);

    va_list args;
    int ret;

    SYSTEMTIME st;
    GetLocalTime(&st);

    fprintf(
        stdout,
        "[%lu] %02d:%02d:%02d.%03d ",
        GetCurrentThreadId(),
        st.wHour,
        st.wMinute,
        st.wSecond,
        st.wMilliseconds);

    va_start(args, fmt);
    ret = vfprintf(stdout, fmt, args);
    va_end(args);

    fflush(stdout);

    LeaveCriticalSection(&cs);

    return ret;
}

void dbg_draw_frame_info_start()
{
    static DWORD tick_fps = 0;
    static char debug_text[512] = { 0 };

    RECT debugrc = { 0, 0, g_ddraw->width, g_ddraw->height };

    if (g_ddraw->primary)
    {
        HDC primary_dc;
        dds_GetDC(g_ddraw->primary, &primary_dc);

        DrawText(primary_dc, debug_text, -1, &debugrc, DT_NOCLIP);
    }

    DWORD tick_start = timeGetTime();
    if (tick_start >= tick_fps)
    {
        _snprintf(
            debug_text,
            sizeof(debug_text) - 1,
            "FPS: %lu | Time: %2.2f ms  ",
            g_dbg_frame_count,
            g_dbg_frame_time);

        g_dbg_frame_count = 0;
        tick_fps = tick_start + 1000;

        dbg_counter_start();
    }

    g_dbg_frame_count++;
}

void dbg_draw_frame_info_end()
{
    if (g_dbg_frame_count == 1)
        g_dbg_frame_time = dbg_counter_stop();
}

void dbg_dump_dds_blt_flags(DWORD flags)
{
#ifdef _DEBUG_X
    if (flags & DDBLT_ALPHADEST) {
        TRACE("     DDBLT_ALPHADEST\n");
    }
    if (flags & DDBLT_ALPHADESTCONSTOVERRIDE) {
        TRACE("     DDBLT_ALPHADESTCONSTOVERRIDE\n");
    }
    if (flags & DDBLT_ALPHADESTNEG) {
        TRACE("     DDBLT_ALPHADESTNEG\n");
    }
    if (flags & DDBLT_ALPHADESTSURFACEOVERRIDE) {
        TRACE("     DDBLT_ALPHADESTSURFACEOVERRIDE\n");
    }
    if (flags & DDBLT_ALPHAEDGEBLEND) {
        TRACE("     DDBLT_ALPHAEDGEBLEND\n");
    }
    if (flags & DDBLT_ALPHASRC) {
        TRACE("     DDBLT_ALPHASRC\n");
    }
    if (flags & DDBLT_ALPHASRCCONSTOVERRIDE) {
        TRACE("     DDBLT_ALPHASRCCONSTOVERRIDE\n");
    }
    if (flags & DDBLT_ALPHASRCNEG) {
        TRACE("     DDBLT_ALPHASRCNEG\n");
    }
    if (flags & DDBLT_ALPHASRCSURFACEOVERRIDE) {
        TRACE("     DDBLT_ALPHASRCSURFACEOVERRIDE\n");
    }
    if (flags & DDBLT_ASYNC) {
        TRACE("     DDBLT_ASYNC\n");
    }
    if (flags & DDBLT_COLORFILL) {
        TRACE("     DDBLT_COLORFILL\n");
    }
    if (flags & DDBLT_DDFX) {
        TRACE("     DDBLT_DDFX\n");
    }
    if (flags & DDBLT_DDROPS) {
        TRACE("     DDBLT_DDROPS\n");
    }
    if (flags & DDBLT_KEYDEST) {
        TRACE("     DDBLT_KEYDEST\n");
    }
    if (flags & DDBLT_KEYDESTOVERRIDE) {
        TRACE("     DDBLT_KEYDESTOVERRIDE\n");
    }
    if (flags & DDBLT_KEYSRC) {
        TRACE("     DDBLT_KEYSRC\n");
    }
    if (flags & DDBLT_KEYSRCOVERRIDE) {
        TRACE("     DDBLT_KEYSRCOVERRIDE\n");
    }
    if (flags & DDBLT_ROP) {
        TRACE("     DDBLT_ROP\n");
    }
    if (flags & DDBLT_ROTATIONANGLE) {
        TRACE("     DDBLT_ROTATIONANGLE\n");
    }
    if (flags & DDBLT_ZBUFFER) {
        TRACE("     DDBLT_ZBUFFER\n");
    }
    if (flags & DDBLT_ZBUFFERDESTCONSTOVERRIDE) {
        TRACE("     DDBLT_ZBUFFERDESTCONSTOVERRIDE\n");
    }
    if (flags & DDBLT_ZBUFFERDESTOVERRIDE) {
        TRACE("     DDBLT_ZBUFFERDESTOVERRIDE\n");
    }
    if (flags & DDBLT_ZBUFFERSRCCONSTOVERRIDE) {
        TRACE("     DDBLT_ZBUFFERSRCCONSTOVERRIDE\n");
    }
    if (flags & DDBLT_ZBUFFERSRCOVERRIDE) {
        TRACE("     DDBLT_ZBUFFERSRCOVERRIDE\n");
    }
    if (flags & DDBLT_WAIT) {
        TRACE("     DDBLT_WAIT\n");
    }
    if (flags & DDBLT_DEPTHFILL) {
        TRACE("     DDBLT_DEPTHFILL\n");
    }
    if (flags & DDBLT_DONOTWAIT) {
        TRACE("     DDBLT_DONOTWAIT\n");
    }
    if (flags & DDBLT_PRESENTATION) {
        TRACE("     DDBLT_PRESENTATION\n");
    }
    if (flags & DDBLT_LAST_PRESENTATION) {
        TRACE("     DDBLT_LAST_PRESENTATION\n");
    }
    if (flags & DDBLT_EXTENDED_FLAGS) {
        TRACE("     DDBLT_EXTENDED_FLAGS\n");
    }
    if (flags & DDBLT_EXTENDED_LINEAR_CONTENT) {
        TRACE("     DDBLT_EXTENDED_LINEAR_CONTENT\n");
    }
#endif
}

void dbg_dump_dds_blt_fx_flags(DWORD flags)
{
#ifdef _DEBUG_X
    if (flags & DDBLTFX_ARITHSTRETCHY) {
        TRACE("     DDBLTFX_ARITHSTRETCHY\n");
    }
    if (flags & DDBLTFX_MIRRORLEFTRIGHT) {
        TRACE("     DDBLTFX_MIRRORLEFTRIGHT\n");
    }
    if (flags & DDBLTFX_MIRRORUPDOWN) {
        TRACE("     DDBLTFX_MIRRORUPDOWN\n");
    }
    if (flags & DDBLTFX_NOTEARING) {
        TRACE("     DDBLTFX_NOTEARING\n");
    }
    if (flags & DDBLTFX_ROTATE180) {
        TRACE("     DDBLTFX_ROTATE180\n");
    }
    if (flags & DDBLTFX_ROTATE270) {
        TRACE("     DDBLTFX_ROTATE270\n");
    }
    if (flags & DDBLTFX_ROTATE90) {
        TRACE("     DDBLTFX_ROTATE90\n");
    }
    if (flags & DDBLTFX_ZBUFFERRANGE) {
        TRACE("     DDBLTFX_ZBUFFERRANGE\n");
    }
    if (flags & DDBLTFX_ZBUFFERBASEDEST) {
        TRACE("     DDBLTFX_ZBUFFERBASEDEST\n");
    }
#endif
}

void dbg_dump_dds_caps(DWORD caps)
{
#ifdef _DEBUG_X
    if (caps & DDSCAPS_RESERVED1) {
        TRACE("     DDSCAPS_RESERVED1\n");
    }
    if (caps & DDSCAPS_ALPHA) {
        TRACE("     DDSCAPS_ALPHA\n");
    }
    if (caps & DDSCAPS_BACKBUFFER) {
        TRACE("     DDSCAPS_BACKBUFFER\n");
    }
    if (caps & DDSCAPS_COMPLEX) {
        TRACE("     DDSCAPS_COMPLEX\n");
    }
    if (caps & DDSCAPS_FLIP) {
        TRACE("     DDSCAPS_FLIP\n");
    }
    if (caps & DDSCAPS_FRONTBUFFER) {
        TRACE("     DDSCAPS_FRONTBUFFER\n");
    }
    if (caps & DDSCAPS_OFFSCREENPLAIN) {
        TRACE("     DDSCAPS_OFFSCREENPLAIN\n");
    }
    if (caps & DDSCAPS_OVERLAY) {
        TRACE("     DDSCAPS_OVERLAY\n");
    }
    if (caps & DDSCAPS_PALETTE) {
        TRACE("     DDSCAPS_PALETTE\n");
    }
    if (caps & DDSCAPS_PRIMARYSURFACE) {
        TRACE("     DDSCAPS_PRIMARYSURFACE\n");
    }
    if (caps & DDSCAPS_RESERVED3) {
        TRACE("     DDSCAPS_RESERVED3\n");
    }
    if (caps & DDSCAPS_PRIMARYSURFACELEFT) {
        TRACE("     DDSCAPS_PRIMARYSURFACELEFT\n");
    }
    if (caps & DDSCAPS_SYSTEMMEMORY) {
        TRACE("     DDSCAPS_SYSTEMMEMORY\n");
    }
    if (caps & DDSCAPS_TEXTURE) {
        TRACE("     DDSCAPS_TEXTURE\n");
    }
    if (caps & DDSCAPS_3DDEVICE) {
        TRACE("     DDSCAPS_3DDEVICE\n");
    }
    if (caps & DDSCAPS_VIDEOMEMORY) {
        TRACE("     DDSCAPS_VIDEOMEMORY\n");
    }
    if (caps & DDSCAPS_VISIBLE) {
        TRACE("     DDSCAPS_VISIBLE\n");
    }
    if (caps & DDSCAPS_ZBUFFER) {
        TRACE("     DDSCAPS_ZBUFFER\n");
    }
    if (caps & DDSCAPS_OWNDC) {
        TRACE("     DDSCAPS_OWNDC\n");
    }
    if (caps & DDSCAPS_LIVEVIDEO) {
        TRACE("     DDSCAPS_LIVEVIDEO\n");
    }
    if (caps & DDSCAPS_HWCODEC) {
        TRACE("     DDSCAPS_HWCODEC\n");
    }
    if (caps & DDSCAPS_MODEX) {
        TRACE("     DDSCAPS_MODEX\n");
    }
    if (caps & DDSCAPS_MIPMAP) {
        TRACE("     DDSCAPS_MIPMAP\n");
    }
    if (caps & DDSCAPS_RESERVED2) {
        TRACE("     DDSCAPS_RESERVED2\n");
    }
    if (caps & DDSCAPS_ALLOCONLOAD) {
        TRACE("     DDSCAPS_ALLOCONLOAD\n");
    }
    if (caps & DDSCAPS_VIDEOPORT) {
        TRACE("     DDSCAPS_VIDEOPORT\n");
    }
    if (caps & DDSCAPS_LOCALVIDMEM) {
        TRACE("     DDSCAPS_LOCALVIDMEM\n");
    }
    if (caps & DDSCAPS_NONLOCALVIDMEM) {
        TRACE("     DDSCAPS_NONLOCALVIDMEM\n");
    }
    if (caps & DDSCAPS_STANDARDVGAMODE) {
        TRACE("     DDSCAPS_STANDARDVGAMODE\n");
    }
    if (caps & DDSCAPS_OPTIMIZED) {
        TRACE("     DDSCAPS_OPTIMIZED\n");
    }
#endif
}

void dbg_dump_dds_flags(DWORD flags)
{
#ifdef _DEBUG_X
    if (flags & DDSD_CAPS) {
        TRACE("     DDSD_CAPS\n");
    }
    if (flags & DDSD_HEIGHT) {
        TRACE("     DDSD_HEIGHT\n");
    }
    if (flags & DDSD_WIDTH) {
        TRACE("     DDSD_WIDTH\n");
    }
    if (flags & DDSD_PITCH) {
        TRACE("     DDSD_PITCH\n");
    }
    if (flags & DDSD_BACKBUFFERCOUNT) {
        TRACE("     DDSD_BACKBUFFERCOUNT\n");
    }
    if (flags & DDSD_ZBUFFERBITDEPTH) {
        TRACE("     DDSD_ZBUFFERBITDEPTH\n");
    }
    if (flags & DDSD_ALPHABITDEPTH) {
        TRACE("     DDSD_ALPHABITDEPTH\n");
    }
    if (flags & DDSD_LPSURFACE) {
        TRACE("     DDSD_LPSURFACE\n");
    }
    if (flags & DDSD_PIXELFORMAT) {
        TRACE("     DDSD_PIXELFORMAT\n");
    }
    if (flags & DDSD_CKDESTOVERLAY) {
        TRACE("     DDSD_CKDESTOVERLAY\n");
    }
    if (flags & DDSD_CKDESTBLT) {
        TRACE("     DDSD_CKDESTBLT\n");
    }
    if (flags & DDSD_CKSRCOVERLAY) {
        TRACE("     DDSD_CKSRCOVERLAY\n");
    }
    if (flags & DDSD_CKSRCBLT) {
        TRACE("     DDSD_CKSRCBLT\n");
    }
    if (flags & DDSD_MIPMAPCOUNT) {
        TRACE("     DDSD_MIPMAPCOUNT\n");
    }
    if (flags & DDSD_REFRESHRATE) {
        TRACE("     DDSD_REFRESHRATE\n");
    }
    if (flags & DDSD_LINEARSIZE) {
        TRACE("     DDSD_LINEARSIZE\n");
    }
    if (flags & DDSD_TEXTURESTAGE) {
        TRACE("     DDSD_TEXTURESTAGE\n");
    }
    if (flags & DDSD_FVF) {
        TRACE("     DDSD_FVF\n");
    }
    if (flags & DDSD_SRCVBHANDLE) {
        TRACE("     DDSD_SRCVBHANDLE\n");
    }
    if (flags & DDSD_DEPTH) {
        TRACE("     DDSD_DEPTH\n");
    }
    if (flags & DDSD_ALL) {
        TRACE("     DDSD_ALL\n");
    }
#endif
}

void dbg_dump_dds_blt_fast_flags(DWORD flags)
{
#ifdef _DEBUG_X
    if (flags & DDBLTFAST_NOCOLORKEY) {
        TRACE("     DDBLTFAST_NOCOLORKEY\n");
    }
    if (flags & DDBLTFAST_SRCCOLORKEY) {
        TRACE("     DDBLTFAST_SRCCOLORKEY\n");
    }
    if (flags & DDBLTFAST_DESTCOLORKEY) {
        TRACE("     DDBLTFAST_DESTCOLORKEY\n");
    }
    if (flags & DDBLTFAST_WAIT) {
        TRACE("     DDBLTFAST_WAIT\n");
    }
    if (flags & DDBLTFAST_DONOTWAIT) {
        TRACE("     DDBLTFAST_DONOTWAIT\n");
    }
#endif
}

void dbg_dump_dds_lock_flags(DWORD flags)
{
#ifdef _DEBUG_X
    if (flags & DDLOCK_SURFACEMEMORYPTR) {
        TRACE("     DDLOCK_SURFACEMEMORYPTR\n");
    }
    if (flags & DDLOCK_WAIT) {
        TRACE("     DDLOCK_WAIT\n");
    }
    if (flags & DDLOCK_EVENT) {
        TRACE("     DDLOCK_EVENT\n");
    }
    if (flags & DDLOCK_READONLY) {
        TRACE("     DDLOCK_READONLY\n");
    }
    if (flags & DDLOCK_WRITEONLY) {
        TRACE("     DDLOCK_WRITEONLY\n");
    }
    if (flags & DDLOCK_NOSYSLOCK) {
        TRACE("     DDLOCK_NOSYSLOCK\n");
    }
    if (flags & DDLOCK_NOOVERWRITE) {
        TRACE("     DDLOCK_NOOVERWRITE\n");
    }
    if (flags & DDLOCK_DISCARDCONTENTS) {
        TRACE("     DDLOCK_DISCARDCONTENTS\n");
    }
    if (flags & DDLOCK_OKTOSWAP) {
        TRACE("     DDLOCK_OKTOSWAP\n");
    }
    if (flags & DDLOCK_DONOTWAIT) {
        TRACE("     DDLOCK_DONOTWAIT\n");
    }
    if (flags & DDLOCK_HASVOLUMETEXTUREBOXRECT) {
        TRACE("     DDLOCK_HASVOLUMETEXTUREBOXRECT\n");
    }
    if (flags & DDLOCK_NODIRTYUPDATE) {
        TRACE("     DDLOCK_NODIRTYUPDATE\n");
    }
#endif
}

char* dbg_mes_to_str(int id)
{
    switch (id)
    {
    case 0: return "WM_NULL";
    case 1: return "WM_CREATE";
    case 2: return "WM_DESTROY";
    case 3: return "WM_MOVE";
    case 5: return "WM_SIZE";
    case 6: return "WM_ACTIVATE";
    case 7: return "WM_SETFOCUS";
    case 8: return "WM_KILLFOCUS";
    case 10: return "WM_ENABLE";
    case 11: return "WM_SETREDRAW";
    case 12: return "WM_SETTEXT";
    case 13: return "WM_GETTEXT";
    case 14: return "WM_GETTEXTLENGTH";
    case 15: return "WM_PAINT";
    case 16: return "WM_CLOSE";
    case 17: return "WM_QUERYENDSESSION";
    case 18: return "WM_QUIT";
    case 19: return "WM_QUERYOPEN";
    case 20: return "WM_ERASEBKGND";
    case 21: return "WM_SYSCOLORCHANGE";
    case 22: return "WM_ENDSESSION";
    case 24: return "WM_SHOWWINDOW";
    case 25: return "WM_CTLCOLOR";
    case 26: return "WM_WININICHANGE";
    case 27: return "WM_DEVMODECHANGE";
    case 28: return "WM_ACTIVATEAPP";
    case 29: return "WM_FONTCHANGE";
    case 30: return "WM_TIMECHANGE";
    case 31: return "WM_CANCELMODE";
    case 32: return "WM_SETCURSOR";
    case 33: return "WM_MOUSEACTIVATE";
    case 34: return "WM_CHILDACTIVATE";
    case 35: return "WM_QUEUESYNC";
    case 36: return "WM_GETMINMAXINFO";
    case 38: return "WM_PAINTICON";
    case 39: return "WM_ICONERASEBKGND";
    case 40: return "WM_NEXTDLGCTL";
    case 42: return "WM_SPOOLERSTATUS";
    case 43: return "WM_DRAWITEM";
    case 44: return "WM_MEASUREITEM";
    case 45: return "WM_DELETEITEM";
    case 46: return "WM_VKEYTOITEM";
    case 47: return "WM_CHARTOITEM";
    case 48: return "WM_SETFONT";
    case 49: return "WM_GETFONT";
    case 50: return "WM_SETHOTKEY";
    case 51: return "WM_GETHOTKEY";
    case 55: return "WM_QUERYDRAGICON";
    case 57: return "WM_COMPAREITEM";
    case 61: return "WM_GETOBJECT";
    case 65: return "WM_COMPACTING";
    case 68: return "WM_COMMNOTIFY";
    case 70: return "WM_WINDOWPOSCHANGING";
    case 71: return "WM_WINDOWPOSCHANGED";
    case 72: return "WM_POWER";
    case 73: return "WM_COPYGLOBALDATA";
    case 74: return "WM_COPYDATA";
    case 75: return "WM_CANCELJOURNAL";
    case 78: return "WM_NOTIFY";
    case 80: return "WM_INPUTLANGCHANGEREQUEST";
    case 81: return "WM_INPUTLANGCHANGE";
    case 82: return "WM_TCARD";
    case 83: return "WM_HELP";
    case 84: return "WM_USERCHANGED";
    case 85: return "WM_NOTIFYFORMAT";
    case 123: return "WM_CONTEXTMENU";
    case 124: return "WM_STYLECHANGING";
    case 125: return "WM_STYLECHANGED";
    case 126: return "WM_DISPLAYCHANGE";
    case 127: return "WM_GETICON";
    case 128: return "WM_SETICON";
    case 129: return "WM_NCCREATE";
    case 130: return "WM_NCDESTROY";
    case 131: return "WM_NCCALCSIZE";
    case 132: return "WM_NCHITTEST";
    case 133: return "WM_NCPAINT";
    case 134: return "WM_NCACTIVATE";
    case 135: return "WM_GETDLGCODE";
    case 136: return "WM_SYNCPAINT";
    case 160: return "WM_NCMOUSEMOVE";
    case 161: return "WM_NCLBUTTONDOWN";
    case 162: return "WM_NCLBUTTONUP";
    case 163: return "WM_NCLBUTTONDBLCLK";
    case 164: return "WM_NCRBUTTONDOWN";
    case 165: return "WM_NCRBUTTONUP";
    case 166: return "WM_NCRBUTTONDBLCLK";
    case 167: return "WM_NCMBUTTONDOWN";
    case 168: return "WM_NCMBUTTONUP";
    case 169: return "WM_NCMBUTTONDBLCLK";
    case 171: return "WM_NCXBUTTONDOWN";
    case 172: return "WM_NCXBUTTONUP";
    case 173: return "WM_NCXBUTTONDBLCLK";
    case 176: return "EM_GETSEL";
    case 177: return "EM_SETSEL";
    case 178: return "EM_GETRECT";
    case 179: return "EM_SETRECT";
    case 180: return "EM_SETRECTNP";
    case 181: return "EM_SCROLL";
    case 182: return "EM_LINESCROLL";
    case 183: return "EM_SCROLLCARET";
    case 185: return "EM_GETMODIFY";
    case 187: return "EM_SETMODIFY";
    case 188: return "EM_GETLINECOUNT";
    case 189: return "EM_LINEINDEX";
    case 190: return "EM_SETHANDLE";
    case 191: return "EM_GETHANDLE";
    case 192: return "EM_GETTHUMB";
    case 193: return "EM_LINELENGTH";
    case 194: return "EM_REPLACESEL";
    case 195: return "EM_SETFONT";
    case 196: return "EM_GETLINE";
    case 197: return "EM_LIMITTEXT, EM_SETLIMITTEXT";
    case 198: return "EM_CANUNDO";
    case 199: return "EM_UNDO";
    case 200: return "EM_FMTLINES";
    case 201: return "EM_LINEFROMCHAR";
    case 202: return "EM_SETWORDBREAK";
    case 203: return "EM_SETTABSTOPS";
    case 204: return "EM_SETPASSWORDCHAR";
    case 205: return "EM_EMPTYUNDOBUFFER";
    case 206: return "EM_GETFIRSTVISIBLELINE";
    case 207: return "EM_SETREADONLY";
    case 209: return "EM_SETWORDBREAKPROC, EM_GETWORDBREAKPROC";
    case 210: return "EM_GETPASSWORDCHAR";
    case 211: return "EM_SETMARGINS";
    case 212: return "EM_GETMARGINS";
    case 213: return "EM_GETLIMITTEXT";
    case 214: return "EM_POSFROMCHAR";
    case 215: return "EM_CHARFROMPOS";
    case 216: return "EM_SETIMESTATUS";
    case 217: return "EM_GETIMESTATUS";
    case 224: return "SBM_SETPOS";
    case 225: return "SBM_GETPOS";
    case 226: return "SBM_SETRANGE";
    case 227: return "SBM_GETRANGE";
    case 228: return "SBM_ENABLE_ARROWS";
    case 230: return "SBM_SETRANGEREDRAW";
    case 233: return "SBM_SETSCROLLINFO";
    case 234: return "SBM_GETSCROLLINFO";
    case 235: return "SBM_GETSCROLLBARINFO";
    case 240: return "BM_GETCHECK";
    case 241: return "BM_SETCHECK";
    case 242: return "BM_GETSTATE";
    case 243: return "BM_SETSTATE";
    case 244: return "BM_SETSTYLE";
    case 245: return "BM_CLICK";
    case 246: return "BM_GETIMAGE";
    case 247: return "BM_SETIMAGE";
    case 248: return "BM_SETDONTCLICK";
    case 255: return "WM_INPUT";
    case 256: return "WM_KEYDOWN";
    case 257: return "WM_KEYUP";
    case 258: return "WM_CHAR";
    case 259: return "WM_DEADCHAR";
    case 260: return "WM_SYSKEYDOWN";
    case 261: return "WM_SYSKEYUP";
    case 262: return "WM_SYSCHAR";
    case 263: return "WM_SYSDEADCHAR";
    case 265: return "WM_UNICHAR, WM_WNT_CONVERTREQUESTEX";
    case 266: return "WM_CONVERTREQUEST";
    case 267: return "WM_CONVERTRESULT";
    case 268: return "WM_INTERIM";
    case 269: return "WM_IME_STARTCOMPOSITION";
    case 270: return "WM_IME_ENDCOMPOSITION";
    case 271: return "WM_IME_COMPOSITION";
    case 272: return "WM_INITDIALOG";
    case 273: return "WM_COMMAND";
    case 274: return "WM_SYSCOMMAND";
    case 275: return "WM_TIMER";
    case 276: return "WM_HSCROLL";
    case 277: return "WM_VSCROLL";
    case 278: return "WM_INITMENU";
    case 279: return "WM_INITMENUPOPUP";
    case 280: return "WM_SYSTIMER";
    case 287: return "WM_MENUSELECT";
    case 288: return "WM_MENUCHAR";
    case 289: return "WM_ENTERIDLE";
    case 290: return "WM_MENURBUTTONUP";
    case 291: return "WM_MENUDRAG";
    case 292: return "WM_MENUGETOBJECT";
    case 293: return "WM_UNINITMENUPOPUP";
    case 294: return "WM_MENUCOMMAND";
    case 295: return "WM_CHANGEUISTATE";
    case 296: return "WM_UPDATEUISTATE";
    case 297: return "WM_QUERYUISTATE";
    case 306: return "WM_CTLCOLORMSGBOX";
    case 307: return "WM_CTLCOLOREDIT";
    case 308: return "WM_CTLCOLORLISTBOX";
    case 309: return "WM_CTLCOLORBTN";
    case 310: return "WM_CTLCOLORDLG";
    case 311: return "WM_CTLCOLORSCROLLBAR";
    case 312: return "WM_CTLCOLORSTATIC";
    case 512: return "WM_MOUSEMOVE";
    case 513: return "WM_LBUTTONDOWN";
    case 514: return "WM_LBUTTONUP";
    case 515: return "WM_LBUTTONDBLCLK";
    case 516: return "WM_RBUTTONDOWN";
    case 517: return "WM_RBUTTONUP";
    case 518: return "WM_RBUTTONDBLCLK";
    case 519: return "WM_MBUTTONDOWN";
    case 520: return "WM_MBUTTONUP";
    case 521: return "WM_MBUTTONDBLCLK";
    case 522: return "WM_MOUSEWHEEL";
    case 523: return "WM_XBUTTONDOWN";
    case 524: return "WM_XBUTTONUP";
    case 525: return "WM_XBUTTONDBLCLK";
    case 526: return "WM_MOUSEHWHEEL";
    case 528: return "WM_PARENTNOTIFY";
    case 529: return "WM_ENTERMENULOOP";
    case 530: return "WM_EXITMENULOOP";
    case 531: return "WM_NEXTMENU";
    case 532: return "WM_SIZING";
    case 533: return "WM_CAPTURECHANGED";
    case 534: return "WM_MOVING";
    case 536: return "WM_POWERBROADCAST";
    case 537: return "WM_DEVICECHANGE";
    case 544: return "WM_MDICREATE";
    case 545: return "WM_MDIDESTROY";
    case 546: return "WM_MDIACTIVATE";
    case 547: return "WM_MDIRESTORE";
    case 548: return "WM_MDINEXT";
    case 549: return "WM_MDIMAXIMIZE";
    case 550: return "WM_MDITILE";
    case 551: return "WM_MDICASCADE";
    case 552: return "WM_MDIICONARRANGE";
    case 553: return "WM_MDIGETACTIVE";
    case 560: return "WM_MDISETMENU";
    case 561: return "WM_ENTERSIZEMOVE";
    case 562: return "WM_EXITSIZEMOVE";
    case 563: return "WM_DROPFILES";
    case 564: return "WM_MDIREFRESHMENU";
    case 640: return "WM_IME_REPORT";
    case 641: return "WM_IME_SETCONTEXT";
    case 642: return "WM_IME_NOTIFY";
    case 643: return "WM_IME_CONTROL";
    case 644: return "WM_IME_COMPOSITIONFULL";
    case 645: return "WM_IME_SELECT";
    case 646: return "WM_IME_CHAR";
    case 648: return "WM_IME_REQUEST";
    case 656: return "WM_IMEKEYDOWN, WM_IME_KEYDOWN";
    case 657: return "WM_IMEKEYUP, WM_IME_KEYUP";
    case 672: return "WM_NCMOUSEHOVER";
    case 673: return "WM_MOUSEHOVER";
    case 674: return "WM_NCMOUSELEAVE";
    case 675: return "WM_MOUSELEAVE";
    case 768: return "WM_CUT";
    case 769: return "WM_COPY";
    case 770: return "WM_PASTE";
    case 771: return "WM_CLEAR";
    case 772: return "WM_UNDO";
    case 773: return "WM_RENDERFORMAT";
    case 774: return "WM_RENDERALLFORMATS";
    case 775: return "WM_DESTROYCLIPBOARD";
    case 776: return "WM_DRAWCLIPBOARD";
    case 777: return "WM_PAINTCLIPBOARD";
    case 778: return "WM_VSCROLLCLIPBOARD";
    case 779: return "WM_SIZECLIPBOARD";
    case 780: return "WM_ASKCBFORMATNAME";
    case 781: return "WM_CHANGECBCHAIN";
    case 782: return "WM_HSCROLLCLIPBOARD";
    case 783: return "WM_QUERYNEWPALETTE";
    case 784: return "WM_PALETTEISCHANGING";
    case 785: return "WM_PALETTECHANGED";
    case 786: return "WM_HOTKEY";
    case 791: return "WM_PRINT";
    case 792: return "WM_PRINTCLIENT";
    case 793: return "WM_APPCOMMAND";
    case 856: return "WM_HANDHELDFIRST";
    case 863: return "WM_HANDHELDLAST";
    case 864: return "WM_AFXFIRST";
    case 895: return "WM_AFXLAST";
    case 896: return "WM_PENWINFIRST";
    case 897: return "WM_RCRESULT";
    case 898: return "WM_HOOKRCRESULT";
    case 899: return "WM_GLOBALRCCHANGE, WM_PENMISCINFO";
    case 900: return "WM_SKB";
    case 901: return "WM_HEDITCTL, WM_PENCTL";
    case 902: return "WM_PENMISC";
    case 903: return "WM_CTLINIT";
    case 904: return "WM_PENEVENT";
    case 911: return "WM_PENWINLAST";
    case 1024: return "DDM_SETFMT, DM_GETDEFID, NIN_SELECT, TBM_GETPOS, WM_PSD_PAGESETUPDLG, WM_USER";
    case 1025: return "CBEM_INSERTITEMA, DDM_DRAW, DM_SETDEFID, HKM_SETHOTKEY, PBM_SETRANGE, RB_INSERTBANDA, SB_SETTEXTA, TB_ENABLEBUTTON, TBM_GETRANGEMIN, TTM_ACTIVATE, WM_CHOOSEFONT_GETLOGFONT, WM_PSD_FULLPAGERECT";
    case 1026: return "CBEM_SETIMAGELIST, DDM_CLOSE, DM_REPOSITION, HKM_GETHOTKEY, PBM_SETPOS, RB_DELETEBAND, SB_GETTEXTA, TB_CHECKBUTTON, TBM_GETRANGEMAX, WM_PSD_MINMARGINRECT";
    case 1027: return "CBEM_GETIMAGELIST, DDM_BEGIN, HKM_SETRULES, PBM_DELTAPOS, RB_GETBARINFO, SB_GETTEXTLENGTHA, TBM_GETTIC, TB_PRESSBUTTON, TTM_SETDELAYTIME, WM_PSD_MARGINRECT";
    case 1028: return "CBEM_GETITEMA, DDM_END, PBM_SETSTEP, RB_SETBARINFO, SB_SETPARTS, TB_HIDEBUTTON, TBM_SETTIC, TTM_ADDTOOLA, WM_PSD_GREEKTEXTRECT";
    case 1029: return "CBEM_SETITEMA, PBM_STEPIT, TB_INDETERMINATE, TBM_SETPOS, TTM_DELTOOLA, WM_PSD_ENVSTAMPRECT";
    case 1030: return "CBEM_GETCOMBOCONTROL, PBM_SETRANGE32, RB_SETBANDINFOA, SB_GETPARTS, TB_MARKBUTTON, TBM_SETRANGE, TTM_NEWTOOLRECTA, WM_PSD_YAFULLPAGERECT";
    case 1031: return "CBEM_GETEDITCONTROL, PBM_GETRANGE, RB_SETPARENT, SB_GETBORDERS, TBM_SETRANGEMIN, TTM_RELAYEVENT";
    case 1032: return "CBEM_SETEXSTYLE, PBM_GETPOS, RB_HITTEST, SB_SETMINHEIGHT, TBM_SETRANGEMAX, TTM_GETTOOLINFOA";
    case 1033: return "CBEM_GETEXSTYLE, CBEM_GETEXTENDEDSTYLE, PBM_SETBARCOLOR, RB_GETRECT, SB_SIMPLE, TB_ISBUTTONENABLED, TBM_CLEARTICS, TTM_SETTOOLINFOA";
    case 1034: return "CBEM_HASEDITCHANGED, RB_INSERTBANDW, SB_GETRECT, TB_ISBUTTONCHECKED, TBM_SETSEL, TTM_HITTESTA, WIZ_QUERYNUMPAGES";
    case 1035: return "CBEM_INSERTITEMW, RB_SETBANDINFOW, SB_SETTEXTW, TB_ISBUTTONPRESSED, TBM_SETSELSTART, TTM_GETTEXTA, WIZ_NEXT";
    case 1036: return "CBEM_SETITEMW, RB_GETBANDCOUNT, SB_GETTEXTLENGTHW, TB_ISBUTTONHIDDEN, TBM_SETSELEND, TTM_UPDATETIPTEXTA, WIZ_PREV";
    case 1037: return "CBEM_GETITEMW, RB_GETROWCOUNT, SB_GETTEXTW, TB_ISBUTTONINDETERMINATE, TTM_GETTOOLCOUNT";
    case 1038: return "CBEM_SETEXTENDEDSTYLE, RB_GETROWHEIGHT, SB_ISSIMPLE, TB_ISBUTTONHIGHLIGHTED, TBM_GETPTICS, TTM_ENUMTOOLSA";
    case 1039: return "SB_SETICON, TBM_GETTICPOS, TTM_GETCURRENTTOOLA";
    case 1040: return "RB_IDTOINDEX, SB_SETTIPTEXTA, TBM_GETNUMTICS, TTM_WINDOWFROMPOINT";
    case 1041: return "RB_GETTOOLTIPS, SB_SETTIPTEXTW, TBM_GETSELSTART, TB_SETSTATE, TTM_TRACKACTIVATE";
    case 1042: return "RB_SETTOOLTIPS, SB_GETTIPTEXTA, TB_GETSTATE, TBM_GETSELEND, TTM_TRACKPOSITION";
    case 1043: return "RB_SETBKCOLOR, SB_GETTIPTEXTW, TB_ADDBITMAP, TBM_CLEARSEL, TTM_SETTIPBKCOLOR";
    case 1044: return "RB_GETBKCOLOR, SB_GETICON, TB_ADDBUTTONSA, TBM_SETTICFREQ, TTM_SETTIPTEXTCOLOR";
    case 1045: return "RB_SETTEXTCOLOR, TB_INSERTBUTTONA, TBM_SETPAGESIZE, TTM_GETDELAYTIME";
    case 1046: return "RB_GETTEXTCOLOR, TB_DELETEBUTTON, TBM_GETPAGESIZE, TTM_GETTIPBKCOLOR";
    case 1047: return "RB_SIZETORECT, TB_GETBUTTON, TBM_SETLINESIZE, TTM_GETTIPTEXTCOLOR";
    case 1048: return "RB_BEGINDRAG, TB_BUTTONCOUNT, TBM_GETLINESIZE, TTM_SETMAXTIPWIDTH";
    case 1049: return "RB_ENDDRAG, TB_COMMANDTOINDEX, TBM_GETTHUMBRECT, TTM_GETMAXTIPWIDTH";
    case 1050: return "RB_DRAGMOVE, TBM_GETCHANNELRECT, TB_SAVERESTOREA, TTM_SETMARGIN";
    case 1051: return "RB_GETBARHEIGHT, TB_CUSTOMIZE, TBM_SETTHUMBLENGTH, TTM_GETMARGIN";
    case 1052: return "RB_GETBANDINFOW, TB_ADDSTRINGA, TBM_GETTHUMBLENGTH, TTM_POP";
    case 1053: return "RB_GETBANDINFOA, TB_GETITEMRECT, TBM_SETTOOLTIPS, TTM_UPDATE";
    case 1054: return "RB_MINIMIZEBAND, TB_BUTTONSTRUCTSIZE, TBM_GETTOOLTIPS, TTM_GETBUBBLESIZE";
    case 1055: return "RB_MAXIMIZEBAND, TBM_SETTIPSIDE, TB_SETBUTTONSIZE, TTM_ADJUSTRECT";
    case 1056: return "TBM_SETBUDDY, TB_SETBITMAPSIZE, TTM_SETTITLEA";
    case 1057: return "MSG_FTS_JUMP_VA, TB_AUTOSIZE, TBM_GETBUDDY, TTM_SETTITLEW";
    case 1058: return "RB_GETBANDBORDERS";
    case 1059: return "MSG_FTS_JUMP_QWORD, RB_SHOWBAND, TB_GETTOOLTIPS";
    case 1060: return "MSG_REINDEX_REQUEST, TB_SETTOOLTIPS";
    case 1061: return "MSG_FTS_WHERE_IS_IT, RB_SETPALETTE, TB_SETPARENT";
    case 1062: return "RB_GETPALETTE";
    case 1063: return "RB_MOVEBAND, TB_SETROWS";
    case 1064: return "TB_GETROWS";
    case 1065: return "TB_GETBITMAPFLAGS";
    case 1066: return "TB_SETCMDID";
    case 1067: return "RB_PUSHCHEVRON, TB_CHANGEBITMAP";
    case 1068: return "TB_GETBITMAP";
    case 1069: return "MSG_GET_DEFFONT, TB_GETBUTTONTEXTA";
    case 1070: return "TB_REPLACEBITMAP";
    case 1071: return "TB_SETINDENT";
    case 1072: return "TB_SETIMAGELIST";
    case 1073: return "TB_GETIMAGELIST";
    case 1074: return "TB_LOADIMAGES, EM_CANPASTE, TTM_ADDTOOLW";
    case 1075: return "EM_DISPLAYBAND, TB_GETRECT, TTM_DELTOOLW";
    case 1076: return "EM_EXGETSEL, TB_SETHOTIMAGELIST, TTM_NEWTOOLRECTW";
    case 1077: return "EM_EXLIMITTEXT, TB_GETHOTIMAGELIST, TTM_GETTOOLINFOW";
    case 1078: return "EM_EXLINEFROMCHAR, TB_SETDISABLEDIMAGELIST, TTM_SETTOOLINFOW";
    case 1079: return "EM_EXSETSEL, TB_GETDISABLEDIMAGELIST, TTM_HITTESTW";
    case 1080: return "EM_FINDTEXT, TB_SETSTYLE, TTM_GETTEXTW";
    case 1081: return "EM_FORMATRANGE, TB_GETSTYLE, TTM_UPDATETIPTEXTW";
    case 1082: return "EM_GETCHARFORMAT, TB_GETBUTTONSIZE, TTM_ENUMTOOLSW";
    case 1083: return "EM_GETEVENTMASK, TB_SETBUTTONWIDTH, TTM_GETCURRENTTOOLW";
    case 1084: return "EM_GETOLEINTERFACE, TB_SETMAXTEXTROWS";
    case 1085: return "EM_GETPARAFORMAT, TB_GETTEXTROWS";
    case 1086: return "EM_GETSELTEXT, TB_GETOBJECT";
    case 1087: return "EM_HIDESELECTION, TB_GETBUTTONINFOW";
    case 1088: return "EM_PASTESPECIAL, TB_SETBUTTONINFOW";
    case 1089: return "EM_REQUESTRESIZE, TB_GETBUTTONINFOA";
    case 1090: return "EM_SELECTIONTYPE, TB_SETBUTTONINFOA";
    case 1091: return "EM_SETBKGNDCOLOR, TB_INSERTBUTTONW";
    case 1092: return "EM_SETCHARFORMAT, TB_ADDBUTTONSW";
    case 1093: return "EM_SETEVENTMASK, TB_HITTEST";
    case 1094: return "EM_SETOLECALLBACK, TB_SETDRAWTEXTFLAGS";
    case 1095: return "EM_SETPARAFORMAT, TB_GETHOTITEM";
    case 1096: return "EM_SETTARGETDEVICE, TB_SETHOTITEM";
    case 1097: return "EM_STREAMIN, TB_SETANCHORHIGHLIGHT";
    case 1098: return "EM_STREAMOUT, TB_GETANCHORHIGHLIGHT";
    case 1099: return "EM_GETTEXTRANGE, TB_GETBUTTONTEXTW";
    case 1100: return "EM_FINDWORDBREAK, TB_SAVERESTOREW";
    case 1101: return "EM_SETOPTIONS, TB_ADDSTRINGW";
    case 1102: return "EM_GETOPTIONS, TB_MAPACCELERATORA";
    case 1103: return "EM_FINDTEXTEX, TB_GETINSERTMARK";
    case 1104: return "EM_GETWORDBREAKPROCEX, TB_SETINSERTMARK";
    case 1105: return "EM_SETWORDBREAKPROCEX, TB_INSERTMARKHITTEST";
    case 1106: return "EM_SETUNDOLIMIT, TB_MOVEBUTTON";
    case 1107: return "TB_GETMAXSIZE";
    case 1108: return "EM_REDO, TB_SETEXTENDEDSTYLE";
    case 1109: return "EM_CANREDO, TB_GETEXTENDEDSTYLE";
    case 1110: return "EM_GETUNDONAME, TB_GETPADDING";
    case 1111: return "EM_GETREDONAME, TB_SETPADDING";
    case 1112: return "EM_STOPGROUPTYPING, TB_SETINSERTMARKCOLOR";
    case 1113: return "EM_SETTEXTMODE, TB_GETINSERTMARKCOLOR";
    case 1114: return "EM_GETTEXTMODE, TB_MAPACCELERATORW";
    case 1115: return "EM_AUTOURLDETECT, TB_GETSTRINGW";
    case 1116: return "EM_GETAUTOURLDETECT, TB_GETSTRINGA";
    case 1117: return "EM_SETPALETTE";
    case 1118: return "EM_GETTEXTEX";
    case 1119: return "EM_GETTEXTLENGTHEX";
    case 1120: return "EM_SHOWSCROLLBAR";
    case 1121: return "EM_SETTEXTEX";
    case 1123: return "TAPI_REPLY";
    case 1124: return "ACM_OPENA, BFFM_SETSTATUSTEXTA, CDM_GETSPEC, EM_SETPUNCTUATION, IPM_CLEARADDRESS, WM_CAP_UNICODE_START";
    case 1125: return "ACM_PLAY, BFFM_ENABLEOK, CDM_GETFILEPATH, EM_GETPUNCTUATION, IPM_SETADDRESS, PSM_SETCURSEL, UDM_SETRANGE, WM_CHOOSEFONT_SETLOGFONT";
    case 1126: return "ACM_STOP, BFFM_SETSELECTIONA, CDM_GETFOLDERPATH, EM_SETWORDWRAPMODE, IPM_GETADDRESS, PSM_REMOVEPAGE, UDM_GETRANGE, WM_CAP_SET_CALLBACK_ERRORW, WM_CHOOSEFONT_SETFLAGS";
    case 1127: return "ACM_OPENW, BFFM_SETSELECTIONW, CDM_GETFOLDERIDLIST, EM_GETWORDWRAPMODE, IPM_SETRANGE, PSM_ADDPAGE, UDM_SETPOS, WM_CAP_SET_CALLBACK_STATUSW";
    case 1128: return "BFFM_SETSTATUSTEXTW, CDM_SETCONTROLTEXT, EM_SETIMECOLOR, IPM_SETFOCUS, PSM_CHANGED, UDM_GETPOS";
    case 1129: return "CDM_HIDECONTROL, EM_GETIMECOLOR, IPM_ISBLANK, PSM_RESTARTWINDOWS, UDM_SETBUDDY";
    case 1130: return "CDM_SETDEFEXT, EM_SETIMEOPTIONS, PSM_REBOOTSYSTEM, UDM_GETBUDDY";
    case 1131: return "EM_GETIMEOPTIONS, PSM_CANCELTOCLOSE, UDM_SETACCEL";
    case 1132: return "EM_CONVPOSITION, EM_CONVPOSITION, PSM_QUERYSIBLINGS, UDM_GETACCEL";
    case 1133: return "MCIWNDM_GETZOOM, PSM_UNCHANGED, UDM_SETBASE";
    case 1134: return "PSM_APPLY, UDM_GETBASE";
    case 1135: return "PSM_SETTITLEA, UDM_SETRANGE32";
    case 1136: return "PSM_SETWIZBUTTONS, UDM_GETRANGE32, WM_CAP_DRIVER_GET_NAMEW";
    case 1137: return "PSM_PRESSBUTTON, UDM_SETPOS32, WM_CAP_DRIVER_GET_VERSIONW";
    case 1138: return "PSM_SETCURSELID, UDM_GETPOS32";
    case 1139: return "PSM_SETFINISHTEXTA";
    case 1140: return "PSM_GETTABCONTROL";
    case 1141: return "PSM_ISDIALOGMESSAGE";
    case 1142: return "MCIWNDM_REALIZE, PSM_GETCURRENTPAGEHWND";
    case 1143: return "MCIWNDM_SETTIMEFORMATA, PSM_INSERTPAGE";
    case 1144: return "EM_SETLANGOPTIONS, MCIWNDM_GETTIMEFORMATA, PSM_SETTITLEW, WM_CAP_FILE_SET_CAPTURE_FILEW";
    case 1145: return "EM_GETLANGOPTIONS, MCIWNDM_VALIDATEMEDIA, PSM_SETFINISHTEXTW, WM_CAP_FILE_GET_CAPTURE_FILEW";
    case 1146: return "EM_GETIMECOMPMODE";
    case 1147: return "EM_FINDTEXTW, MCIWNDM_PLAYTO, WM_CAP_FILE_SAVEASW";
    case 1148: return "EM_FINDTEXTEXW, MCIWNDM_GETFILENAMEA";
    case 1149: return "EM_RECONVERSION, MCIWNDM_GETDEVICEA, PSM_SETHEADERTITLEA, WM_CAP_FILE_SAVEDIBW";
    case 1150: return "EM_SETIMEMODEBIAS, MCIWNDM_GETPALETTE, PSM_SETHEADERTITLEW";
    case 1151: return "EM_GETIMEMODEBIAS, MCIWNDM_SETPALETTE, PSM_SETHEADERSUBTITLEA";
    case 1152: return "MCIWNDM_GETERRORA, PSM_SETHEADERSUBTITLEW";
    case 1153: return "PSM_HWNDTOINDEX";
    case 1154: return "PSM_INDEXTOHWND";
    case 1155: return "MCIWNDM_SETINACTIVETIMER, PSM_PAGETOINDEX";
    case 1156: return "PSM_INDEXTOPAGE";
    case 1157: return "DL_BEGINDRAG, MCIWNDM_GETINACTIVETIMER, PSM_IDTOINDEX";
    case 1158: return "DL_DRAGGING, PSM_INDEXTOID";
    case 1159: return "DL_DROPPED, PSM_GETRESULT";
    case 1160: return "DL_CANCELDRAG, PSM_RECALCPAGESIZES";
    case 1164: return "MCIWNDM_GET_SOURCE";
    case 1165: return "MCIWNDM_PUT_SOURCE";
    case 1166: return "MCIWNDM_GET_DEST";
    case 1167: return "MCIWNDM_PUT_DEST";
    case 1168: return "MCIWNDM_CAN_PLAY";
    case 1169: return "MCIWNDM_CAN_WINDOW";
    case 1170: return "MCIWNDM_CAN_RECORD";
    case 1171: return "MCIWNDM_CAN_SAVE";
    case 1172: return "MCIWNDM_CAN_EJECT";
    case 1173: return "MCIWNDM_CAN_CONFIG";
    case 1174: return "IE_GETINK, MCIWNDM_PALETTEKICK";
    case 1175: return "IE_SETINK";
    case 1176: return "IE_GETPENTIP";
    case 1177: return "IE_SETPENTIP";
    case 1178: return "IE_GETERASERTIP";
    case 1179: return "IE_SETERASERTIP";
    case 1180: return "IE_GETBKGND";
    case 1181: return "IE_SETBKGND";
    case 1182: return "IE_GETGRIDORIGIN";
    case 1183: return "IE_SETGRIDORIGIN";
    case 1184: return "IE_GETGRIDPEN";
    case 1185: return "IE_SETGRIDPEN";
    case 1186: return "IE_GETGRIDSIZE";
    case 1187: return "IE_SETGRIDSIZE";
    case 1188: return "IE_GETMODE";
    case 1189: return "IE_SETMODE";
    case 1190: return "IE_GETINKRECT, WM_CAP_SET_MCI_DEVICEW";
    case 1191: return "WM_CAP_GET_MCI_DEVICEW";
    case 1204: return "WM_CAP_PAL_OPENW";
    case 1205: return "WM_CAP_PAL_SAVEW";
    case 1208: return "IE_GETAPPDATA";
    case 1209: return "IE_SETAPPDATA";
    case 1210: return "IE_GETDRAWOPTS";
    case 1211: return "IE_SETDRAWOPTS";
    case 1212: return "IE_GETFORMAT";
    case 1213: return "IE_SETFORMAT";
    case 1214: return "IE_GETINKINPUT";
    case 1215: return "IE_SETINKINPUT";
    case 1216: return "IE_GETNOTIFY";
    case 1217: return "IE_SETNOTIFY";
    case 1218: return "IE_GETRECOG";
    case 1219: return "IE_SETRECOG";
    case 1220: return "IE_GETSECURITY";
    case 1221: return "IE_SETSECURITY";
    case 1222: return "IE_GETSEL";
    case 1223: return "IE_SETSEL";
    case 1224: return "EM_SETBIDIOPTIONS, IE_DOCOMMAND, MCIWNDM_NOTIFYMODE";
    case 1225: return "EM_GETBIDIOPTIONS, IE_GETCOMMAND";
    case 1226: return "EM_SETTYPOGRAPHYOPTIONS, IE_GETCOUNT";
    case 1227: return "EM_GETTYPOGRAPHYOPTIONS, IE_GETGESTURE, MCIWNDM_NOTIFYMEDIA";
    case 1228: return "EM_SETEDITSTYLE, IE_GETMENU";
    case 1229: return "EM_GETEDITSTYLE, IE_GETPAINTDC, MCIWNDM_NOTIFYERROR";
    case 1230: return "IE_GETPDEVENT";
    case 1231: return "IE_GETSELCOUNT";
    case 1232: return "IE_GETSELITEMS";
    case 1233: return "IE_GETSTYLE";
    case 1243: return "MCIWNDM_SETTIMEFORMATW";
    case 1244: return "EM_OUTLINE, MCIWNDM_GETTIMEFORMATW";
    case 1245: return "EM_GETSCROLLPOS";
    case 1246: return "EM_SETSCROLLPOS, EM_SETSCROLLPOS";
    case 1247: return "EM_SETFONTSIZE";
    case 1248: return "EM_GETZOOM, MCIWNDM_GETFILENAMEW";
    case 1249: return "EM_SETZOOM, MCIWNDM_GETDEVICEW";
    case 1250: return "EM_GETVIEWKIND";
    case 1251: return "EM_SETVIEWKIND";
    case 1252: return "EM_GETPAGE, MCIWNDM_GETERRORW";
    case 1253: return "EM_SETPAGE";
    case 1254: return "EM_GETHYPHENATEINFO";
    case 1255: return "EM_SETHYPHENATEINFO";
    case 1259: return "EM_GETPAGEROTATE";
    case 1260: return "EM_SETPAGEROTATE";
    case 1261: return "EM_GETCTFMODEBIAS";
    case 1262: return "EM_SETCTFMODEBIAS";
    case 1264: return "EM_GETCTFOPENSTATUS";
    case 1265: return "EM_SETCTFOPENSTATUS";
    case 1266: return "EM_GETIMECOMPTEXT";
    case 1267: return "EM_ISIME";
    case 1268: return "EM_GETIMEPROPERTY";
    case 1293: return "EM_GETQUERYRTFOBJ";
    case 1294: return "EM_SETQUERYRTFOBJ";
    case 1536: return "FM_GETFOCUS";
    case 1537: return "FM_GETDRIVEINFOA";
    case 1538: return "FM_GETSELCOUNT";
    case 1539: return "FM_GETSELCOUNTLFN";
    case 1540: return "FM_GETFILESELA";
    case 1541: return "FM_GETFILESELLFNA";
    case 1542: return "FM_REFRESH_WINDOWS";
    case 1543: return "FM_RELOAD_EXTENSIONS";
    case 1553: return "FM_GETDRIVEINFOW";
    case 1556: return "FM_GETFILESELW";
    case 1557: return "FM_GETFILESELLFNW";
    case 1625: return "WLX_WM_SAS";
    case 2024: return "SM_GETSELCOUNT, UM_GETSELCOUNT, WM_CPL_LAUNCH";
    case 2025: return "SM_GETSERVERSELA, UM_GETUSERSELA, WM_CPL_LAUNCHED";
    case 2026: return "SM_GETSERVERSELW, UM_GETUSERSELW";
    case 2027: return "SM_GETCURFOCUSA, UM_GETGROUPSELA";
    case 2028: return "SM_GETCURFOCUSW, UM_GETGROUPSELW";
    case 2029: return "SM_GETOPTIONS, UM_GETCURFOCUSA";
    case 2030: return "UM_GETCURFOCUSW";
    case 2031: return "UM_GETOPTIONS";
    case 2032: return "UM_GETOPTIONS2";
    case 4096: return "LVM_GETBKCOLOR";
    case 4097: return "LVM_SETBKCOLOR";
    case 4098: return "LVM_GETIMAGELIST";
    case 4099: return "LVM_SETIMAGELIST";
    case 4100: return "LVM_GETITEMCOUNT";
    case 4101: return "LVM_GETITEMA";
    case 4102: return "LVM_SETITEMA";
    case 4103: return "LVM_INSERTITEMA";
    case 4104: return "LVM_DELETEITEM";
    case 4105: return "LVM_DELETEALLITEMS";
    case 4106: return "LVM_GETCALLBACKMASK";
    case 4107: return "LVM_SETCALLBACKMASK";
    case 4108: return "LVM_GETNEXTITEM";
    case 4109: return "LVM_FINDITEMA";
    case 4110: return "LVM_GETITEMRECT";
    case 4111: return "LVM_SETITEMPOSITION";
    case 4112: return "LVM_GETITEMPOSITION";
    case 4113: return "LVM_GETSTRINGWIDTHA";
    case 4114: return "LVM_HITTEST";
    case 4115: return "LVM_ENSUREVISIBLE";
    case 4116: return "LVM_SCROLL";
    case 4117: return "LVM_REDRAWITEMS";
    case 4118: return "LVM_ARRANGE";
    case 4119: return "LVM_EDITLABELA";
    case 4120: return "LVM_GETEDITCONTROL";
    case 4121: return "LVM_GETCOLUMNA";
    case 4122: return "LVM_SETCOLUMNA";
    case 4123: return "LVM_INSERTCOLUMNA";
    case 4124: return "LVM_DELETECOLUMN";
    case 4125: return "LVM_GETCOLUMNWIDTH";
    case 4126: return "LVM_SETCOLUMNWIDTH";
    case 4127: return "LVM_GETHEADER";
    case 4129: return "LVM_CREATEDRAGIMAGE";
    case 4130: return "LVM_GETVIEWRECT";
    case 4131: return "LVM_GETTEXTCOLOR";
    case 4132: return "LVM_SETTEXTCOLOR";
    case 4133: return "LVM_GETTEXTBKCOLOR";
    case 4134: return "LVM_SETTEXTBKCOLOR";
    case 4135: return "LVM_GETTOPINDEX";
    case 4136: return "LVM_GETCOUNTPERPAGE";
    case 4137: return "LVM_GETORIGIN";
    case 4138: return "LVM_UPDATE";
    case 4139: return "LVM_SETITEMSTATE";
    case 4140: return "LVM_GETITEMSTATE";
    case 4141: return "LVM_GETITEMTEXTA";
    case 4142: return "LVM_SETITEMTEXTA";
    case 4143: return "LVM_SETITEMCOUNT";
    case 4144: return "LVM_SORTITEMS";
    case 4145: return "LVM_SETITEMPOSITION32";
    case 4146: return "LVM_GETSELECTEDCOUNT";
    case 4147: return "LVM_GETITEMSPACING";
    case 4148: return "LVM_GETISEARCHSTRINGA";
    case 4149: return "LVM_SETICONSPACING";
    case 4150: return "LVM_SETEXTENDEDLISTVIEWSTYLE";
    case 4151: return "LVM_GETEXTENDEDLISTVIEWSTYLE";
    case 4152: return "LVM_GETSUBITEMRECT";
    case 4153: return "LVM_SUBITEMHITTEST";
    case 4154: return "LVM_SETCOLUMNORDERARRAY";
    case 4155: return "LVM_GETCOLUMNORDERARRAY";
    case 4156: return "LVM_SETHOTITEM";
    case 4157: return "LVM_GETHOTITEM";
    case 4158: return "LVM_SETHOTCURSOR";
    case 4159: return "LVM_GETHOTCURSOR";
    case 4160: return "LVM_APPROXIMATEVIEWRECT";
    case 4161: return "LVM_SETWORKAREAS";
    case 4162: return "LVM_GETSELECTIONMARK";
    case 4163: return "LVM_SETSELECTIONMARK";
    case 4164: return "LVM_SETBKIMAGEA";
    case 4165: return "LVM_GETBKIMAGEA";
    case 4166: return "LVM_GETWORKAREAS";
    case 4167: return "LVM_SETHOVERTIME";
    case 4168: return "LVM_GETHOVERTIME";
    case 4169: return "LVM_GETNUMBEROFWORKAREAS";
    case 4170: return "LVM_SETTOOLTIPS";
    case 4171: return "LVM_GETITEMW";
    case 4172: return "LVM_SETITEMW";
    case 4173: return "LVM_INSERTITEMW";
    case 4174: return "LVM_GETTOOLTIPS";
    case 4179: return "LVM_FINDITEMW";
    case 4183: return "LVM_GETSTRINGWIDTHW";
    case 4191: return "LVM_GETCOLUMNW";
    case 4192: return "LVM_SETCOLUMNW";
    case 4193: return "LVM_INSERTCOLUMNW";
    case 4211: return "LVM_GETITEMTEXTW";
    case 4212: return "LVM_SETITEMTEXTW";
    case 4213: return "LVM_GETISEARCHSTRINGW";
    case 4214: return "LVM_EDITLABELW";
    case 4235: return "LVM_GETBKIMAGEW";
    case 4236: return "LVM_SETSELECTEDCOLUMN";
    case 4237: return "LVM_SETTILEWIDTH";
    case 4238: return "LVM_SETVIEW";
    case 4239: return "LVM_GETVIEW";
    case 4241: return "LVM_INSERTGROUP";
    case 4243: return "LVM_SETGROUPINFO";
    case 4245: return "LVM_GETGROUPINFO";
    case 4246: return "LVM_REMOVEGROUP";
    case 4247: return "LVM_MOVEGROUP";
    case 4250: return "LVM_MOVEITEMTOGROUP";
    case 4251: return "LVM_SETGROUPMETRICS";
    case 4252: return "LVM_GETGROUPMETRICS";
    case 4253: return "LVM_ENABLEGROUPVIEW";
    case 4254: return "LVM_SORTGROUPS";
    case 4255: return "LVM_INSERTGROUPSORTED";
    case 4256: return "LVM_REMOVEALLGROUPS";
    case 4257: return "LVM_HASGROUP";
    case 4258: return "LVM_SETTILEVIEWINFO";
    case 4259: return "LVM_GETTILEVIEWINFO";
    case 4260: return "LVM_SETTILEINFO";
    case 4261: return "LVM_GETTILEINFO";
    case 4262: return "LVM_SETINSERTMARK";
    case 4263: return "LVM_GETINSERTMARK";
    case 4264: return "LVM_INSERTMARKHITTEST";
    case 4265: return "LVM_GETINSERTMARKRECT";
    case 4266: return "LVM_SETINSERTMARKCOLOR";
    case 4267: return "LVM_GETINSERTMARKCOLOR";
    case 4269: return "LVM_SETINFOTIP";
    case 4270: return "LVM_GETSELECTEDCOLUMN";
    case 4271: return "LVM_ISGROUPVIEWENABLED";
    case 4272: return "LVM_GETOUTLINECOLOR";
    case 4273: return "LVM_SETOUTLINECOLOR";
    case 4275: return "LVM_CANCELEDITLABEL";
    case 4276: return "LVM_MAPINDEXTOID";
    case 4277: return "LVM_MAPIDTOINDEX";
    case 4278: return "LVM_ISITEMVISIBLE";
    case 8192: return "OCM__BASE";
    case 8197: return "LVM_SETUNICODEFORMAT";
    case 8198: return "LVM_GETUNICODEFORMAT";
    case 8217: return "OCM_CTLCOLOR";
    case 8235: return "OCM_DRAWITEM";
    case 8236: return "OCM_MEASUREITEM";
    case 8237: return "OCM_DELETEITEM";
    case 8238: return "OCM_VKEYTOITEM";
    case 8239: return "OCM_CHARTOITEM";
    case 8249: return "OCM_COMPAREITEM";
    case 8270: return "OCM_NOTIFY";
    case 8465: return "OCM_COMMAND";
    case 8468: return "OCM_HSCROLL";
    case 8469: return "OCM_VSCROLL";
    case 8498: return "OCM_CTLCOLORMSGBOX";
    case 8499: return "OCM_CTLCOLOREDIT";
    case 8500: return "OCM_CTLCOLORLISTBOX";
    case 8501: return "OCM_CTLCOLORBTN";
    case 8502: return "OCM_CTLCOLORDLG";
    case 8503: return "OCM_CTLCOLORSCROLLBAR";
    case 8504: return "OCM_CTLCOLORSTATIC";
    case 8720: return "OCM_PARENTNOTIFY";
    case 32768: return "WM_APP";
    case 52429: return "WM_RASDIALEVENT";
    }

    return 0;
}
