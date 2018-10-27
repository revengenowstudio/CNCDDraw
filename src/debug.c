
#include <windows.h>
#include <stdio.h>
#include "main.h"
#include "surface.h"

double DebugFrameTime = 0;
DWORD DebugFrameCount = 0;

static LONGLONG CounterStartTime = 0;
static double CounterFreq = 0.0;

void CounterStart()
{
    LARGE_INTEGER li;
    QueryPerformanceFrequency(&li);
    CounterFreq = (double)(li.QuadPart) / 1000.0;
    QueryPerformanceCounter(&li);
    CounterStartTime = li.QuadPart;
}

double CounterStop()
{
    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
    return (double)(li.QuadPart - CounterStartTime) / CounterFreq;
}

void DebugPrint(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	char buffer[512];
	_vsnprintf(buffer, sizeof(buffer), format, args);
	OutputDebugStringA(buffer);
}

void DrawFrameInfoStart()
{
    static DWORD tick_fps = 0;
    static char debugText[512] = { 0 };

    RECT debugrc = { 0, 0, ddraw->width, ddraw->height };

    if (ddraw->primary && ddraw->primary->palette)
    {
        if (ddraw->primary->palette->data_rgb)
            SetDIBColorTable(ddraw->primary->hDC, 0, 256, ddraw->primary->palette->data_rgb);

        DrawText(ddraw->primary->hDC, debugText, -1, &debugrc, DT_NOCLIP);
    }
        
    DWORD tick_start = timeGetTime();
    if (tick_start >= tick_fps)
    {
        snprintf(
            debugText,
            sizeof(debugText),
            "FPS: %lu | Time: %2.2f ms  ",
            DebugFrameCount * 2,
            DebugFrameTime);

        DebugFrameCount = 0;
        tick_fps = tick_start + 500;

        CounterStart();
    }

    DebugFrameCount++;
}

void DrawFrameInfoEnd()
{
    if (DebugFrameCount == 1) 
        DebugFrameTime = CounterStop();
}
