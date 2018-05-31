#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>


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
