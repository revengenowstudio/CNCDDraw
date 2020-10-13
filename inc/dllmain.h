#ifndef DLLMAIN_H
#define DLLMAIN_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

extern BOOL GameHandlesClose;
extern HMODULE g_ddraw_module;

typedef enum PROCESS_DPI_AWARENESS {
    PROCESS_DPI_UNAWARE = 0,
    PROCESS_SYSTEM_DPI_AWARE = 1,
    PROCESS_PER_MONITOR_DPI_AWARE = 2
} PROCESS_DPI_AWARENESS;

#endif
