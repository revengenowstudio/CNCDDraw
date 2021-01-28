#ifndef DEBUG_H
#define DEBUG_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

int dbg_exception_handler(EXCEPTION_POINTERS* exception);
void dbg_counter_start();
double dbg_counter_stop();
void dbg_debug_string(const char *format, ...);
void dbg_draw_frame_info_start();
void dbg_draw_frame_info_end();
int dbg_printf(const char *fmt, ...);
void dbg_init();
void dbg_dump_dds_blt_flags(DWORD flags);
void dbg_dump_dds_caps(DWORD caps);
void dbg_dump_dds_flags(DWORD flags);
void dbg_dump_dds_blt_fast_flags(DWORD flags);
void dbg_dump_dds_lock_flags(DWORD flags);

extern double g_dbg_frame_time;
extern DWORD g_dbg_frame_count;

//#define _DEBUG 1

//use OutputDebugStringA rather than printf
//#define _DEBUG_S 1

//log everything (slow)
//#define _DEBUG_X 1



#ifdef _DEBUG

#ifdef _DEBUG_S

#define dprintf(format, ...) dbg_debug_string("xDBG " format, ##__VA_ARGS__)

#ifdef _DEBUG_X
#define dprintfex(format, ...) dbg_debug_string("xDBG " format, ##__VA_ARGS__)
#else
#define dprintfex(format, ...)
#endif

#else

#define dprintf(format, ...) dbg_printf(format, ##__VA_ARGS__) 

#ifdef _DEBUG_X
#define dprintfex(format, ...) dbg_printf(format, ##__VA_ARGS__) 
#else
#define dprintfex(format, ...)
#endif

#endif 

#else 
#define dprintf(format, ...)
#define dprintfex(format, ...)
#endif
 
#endif
