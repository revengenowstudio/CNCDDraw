#include <windows.h>
#include <dbghelp.h>
#include <stdio.h>
#include "dd.h"
#include "ddraw.h"
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

void dbg_debug_string(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    char buffer[512] = { 0 };
    _vsnprintf(buffer, sizeof(buffer), format, args);
    OutputDebugStringA(buffer);
}

int dbg_printf(const char *fmt, ...)
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

    fprintf(stdout, "[%lu] %02d:%02d:%02d.%03d ", GetCurrentThreadId(), st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

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
        if (g_ddraw->primary->palette && g_ddraw->primary->palette->data_rgb)
            SetDIBColorTable(g_ddraw->primary->hdc, 0, 256, g_ddraw->primary->palette->data_rgb);

        DrawText(g_ddraw->primary->hdc, debug_text, -1, &debugrc, DT_NOCLIP);
    }
        
    DWORD tick_start = timeGetTime();
    if (tick_start >= tick_fps)
    {
        _snprintf(
            debug_text,
            sizeof(debug_text)-1,
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
        dprintf("     DDBLT_ALPHADEST\n");
    }
    if (flags & DDBLT_ALPHADESTCONSTOVERRIDE) {
        dprintf("     DDBLT_ALPHADESTCONSTOVERRIDE\n");
    }
    if (flags & DDBLT_ALPHADESTNEG) {
        dprintf("     DDBLT_ALPHADESTNEG\n");
    }
    if (flags & DDBLT_ALPHADESTSURFACEOVERRIDE) {
        dprintf("     DDBLT_ALPHADESTSURFACEOVERRIDE\n");
    }
    if (flags & DDBLT_ALPHAEDGEBLEND) {
        dprintf("     DDBLT_ALPHAEDGEBLEND\n");
    }
    if (flags & DDBLT_ALPHASRC) {
        dprintf("     DDBLT_ALPHASRC\n");
    }
    if (flags & DDBLT_ALPHASRCCONSTOVERRIDE) {
        dprintf("     DDBLT_ALPHASRCCONSTOVERRIDE\n");
    }
    if (flags & DDBLT_ALPHASRCNEG) {
        dprintf("     DDBLT_ALPHASRCNEG\n");
    }
    if (flags & DDBLT_ALPHASRCSURFACEOVERRIDE) {
        dprintf("     DDBLT_ALPHASRCSURFACEOVERRIDE\n");
    }
    if (flags & DDBLT_ASYNC) {
        dprintf("     DDBLT_ASYNC\n");
    }
    if (flags & DDBLT_COLORFILL) {
        dprintf("     DDBLT_COLORFILL\n");
    }
    if (flags & DDBLT_DDFX) {
        dprintf("     DDBLT_DDFX\n");
    }
    if (flags & DDBLT_DDROPS) {
        dprintf("     DDBLT_DDROPS\n");
    }
    if (flags & DDBLT_KEYDEST) {
        dprintf("     DDBLT_KEYDEST\n");
    }
    if (flags & DDBLT_KEYDESTOVERRIDE) {
        dprintf("     DDBLT_KEYDESTOVERRIDE\n");
    }
    if (flags & DDBLT_KEYSRC) {
        dprintf("     DDBLT_KEYSRC\n");
    }
    if (flags & DDBLT_KEYSRCOVERRIDE) {
        dprintf("     DDBLT_KEYSRCOVERRIDE\n");
    }
    if (flags & DDBLT_ROP) {
        dprintf("     DDBLT_ROP\n");
    }
    if (flags & DDBLT_ROTATIONANGLE) {
        dprintf("     DDBLT_ROTATIONANGLE\n");
    }
    if (flags & DDBLT_ZBUFFER) {
        dprintf("     DDBLT_ZBUFFER\n");
    }
    if (flags & DDBLT_ZBUFFERDESTCONSTOVERRIDE) {
        dprintf("     DDBLT_ZBUFFERDESTCONSTOVERRIDE\n");
    }
    if (flags & DDBLT_ZBUFFERDESTOVERRIDE) {
        dprintf("     DDBLT_ZBUFFERDESTOVERRIDE\n");
    }
    if (flags & DDBLT_ZBUFFERSRCCONSTOVERRIDE) {
        dprintf("     DDBLT_ZBUFFERSRCCONSTOVERRIDE\n");
    }
    if (flags & DDBLT_ZBUFFERSRCOVERRIDE) {
        dprintf("     DDBLT_ZBUFFERSRCOVERRIDE\n");
    }
    if (flags & DDBLT_WAIT) {
        dprintf("     DDBLT_WAIT\n");
    }
    if (flags & DDBLT_DEPTHFILL) {
        dprintf("     DDBLT_DEPTHFILL\n");
    }
#endif
}

void dbg_dump_dds_caps(DWORD caps)
{
#ifdef _DEBUG_X
    if (caps & DDSCAPS_ALPHA)
    {
        dprintf("     DDSCAPS_ALPHA\n");
    }
    if (caps & DDSCAPS_BACKBUFFER)
    {
        dprintf("     DDSCAPS_BACKBUFFER\n");
    }
    if (caps & DDSCAPS_FLIP)
    {
        dprintf("     DDSCAPS_FLIP\n");
    }
    if (caps & DDSCAPS_FRONTBUFFER)
    {
        dprintf("     DDSCAPS_FRONTBUFFER\n");
    }
    if (caps & DDSCAPS_PALETTE)
    {
        dprintf("     DDSCAPS_PALETTE\n");
    }
    if (caps & DDSCAPS_TEXTURE)
    {
        dprintf("     DDSCAPS_TEXTURE\n");
    }
    if (caps & DDSCAPS_PRIMARYSURFACE)
    {
        dprintf("     DDSCAPS_PRIMARYSURFACE\n");
    }
    if (caps & DDSCAPS_OFFSCREENPLAIN)
    {
        dprintf("     DDSCAPS_OFFSCREENPLAIN\n");
    }
    if (caps & DDSCAPS_VIDEOMEMORY)
    {
        dprintf("     DDSCAPS_VIDEOMEMORY\n");
    }
    if (caps & DDSCAPS_LOCALVIDMEM)
    {
        dprintf("     DDSCAPS_LOCALVIDMEM\n");
    }
#endif
}

void dbg_dump_dds_flags(DWORD flags)
{
#ifdef _DEBUG_X
    if (flags & DDSD_CAPS)
    {
        dprintf("     DDSD_CAPS\n");
    }
    if (flags & DDSD_HEIGHT)
    {
        dprintf("     DDSD_HEIGHT\n");
    }
    if (flags & DDSD_WIDTH)
    {
        dprintf("     DDSD_WIDTH\n");
    }
    if (flags & DDSD_PITCH)
    {
        dprintf("     DDSD_PITCH\n");
    }
    if (flags & DDSD_BACKBUFFERCOUNT)
    {
        dprintf("     DDSD_BACKBUFFERCOUNT\n");
    }
    if (flags & DDSD_ZBUFFERBITDEPTH)
    {
        dprintf("     DDSD_ZBUFFERBITDEPTH\n");
    }
    if (flags & DDSD_ALPHABITDEPTH)
    {
        dprintf("     DDSD_ALPHABITDEPTH\n");
    }
    if (flags & DDSD_LPSURFACE)
    {
        dprintf("     DDSD_LPSURFACE\n");
    }
    if (flags & DDSD_PIXELFORMAT)
    {
        dprintf("     DDSD_PIXELFORMAT\n");
    }
    if (flags & DDSD_CKDESTOVERLAY)
    {
        dprintf("     DDSD_CKDESTOVERLAY\n");
    }
    if (flags & DDSD_CKDESTBLT)
    {
        dprintf("     DDSD_CKDESTBLT\n");
    }
    if (flags & DDSD_CKSRCOVERLAY)
    {
        dprintf("     DDSD_CKSRCOVERLAY\n");
    }
    if (flags & DDSD_CKSRCBLT)
    {
        dprintf("     DDSD_CKSRCBLT\n");
    }
    if (flags & DDSD_MIPMAPCOUNT)
    {
        dprintf("     DDSD_MIPMAPCOUNT\n");
    }
    if (flags & DDSD_REFRESHRATE)
    {
        dprintf("     DDSD_REFRESHRATE\n");
    }
    if (flags & DDSD_LINEARSIZE)
    {
        dprintf("     DDSD_LINEARSIZE\n");
    }
    if (flags & DDSD_ALL)
    {
        dprintf("     DDSD_ALL\n");
    }
#endif
}

void dbg_dump_dds_blt_fast_flags(DWORD flags)
{
#ifdef _DEBUG_X
    if (flags & DDBLTFAST_NOCOLORKEY)
    {
        dprintf("     DDBLTFAST_NOCOLORKEY\n");
    }

    if (flags & DDBLTFAST_SRCCOLORKEY)
    {
        dprintf("     DDBLTFAST_SRCCOLORKEY\n");
    }

    if (flags & DDBLTFAST_DESTCOLORKEY)
    {
        dprintf("     DDBLTFAST_DESTCOLORKEY\n");
    }
#endif
}

void dbg_dump_dds_lock_flags(DWORD flags)
{
#ifdef _DEBUG_X
    if (flags & DDLOCK_SURFACEMEMORYPTR)
    {
        dprintf("     dwFlags: DDLOCK_SURFACEMEMORYPTR\n");
    }
    if (flags & DDLOCK_WAIT)
    {
        dprintf("     dwFlags: DDLOCK_WAIT\n");
    }
    if (flags & DDLOCK_EVENT)
    {
        dprintf("     dwFlags: DDLOCK_EVENT\n");
    }
    if (flags & DDLOCK_READONLY)
    {
        dprintf("     dwFlags: DDLOCK_READONLY\n");
    }
    if (flags & DDLOCK_WRITEONLY)
    {
        dprintf("     dwFlags: DDLOCK_WRITEONLY\n");
    }
#endif
}
