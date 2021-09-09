#ifndef CONFIG_H
#define CONFIG_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>


typedef struct CNCDDRAWCONFIG
{
    RECT window_rect;
    int window_state;
    char ini_path[MAX_PATH];
    char game_path[MAX_PATH];
    char process_file_name[MAX_PATH];
    int save_settings;

} CNCDDRAWCONFIG;

extern CNCDDRAWCONFIG g_config;

void cfg_load();
void cfg_save();

#endif
