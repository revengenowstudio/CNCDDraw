#ifndef CONFIG_H
#define CONFIG_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>


typedef struct cnc_ddraw_config
{
    RECT window_rect;
    int window_state;
    char ini_path[MAX_PATH];
    char process_file_name[96];
    int save_settings;

} cnc_ddraw_config;

extern cnc_ddraw_config g_config;

void cfg_load();
void cfg_save();

#endif
