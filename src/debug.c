#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>

void DebugPrint(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	char buffer[512];
	_vsnprintf(buffer, sizeof(buffer), format, args);
	OutputDebugStringA(buffer);
}
