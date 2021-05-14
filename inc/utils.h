#ifndef UTILS_H
#define UTILS_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>


void util_limit_game_ticks();
void util_update_bnet_pos(int newX, int newY);
BOOL util_get_lowest_resolution(float ratio, SIZE* outRes, DWORD minWidth, DWORD minHeight, DWORD maxWidth, DWORD maxHeight);
void util_toggle_maximize();
void util_toggle_fullscreen();
BOOL util_unadjust_window_rect(LPRECT prc, DWORD dwStyle, BOOL fMenu, DWORD dwExStyle);
void util_set_window_rect(int x, int y, int width, int height, UINT flags);
BOOL CALLBACK util_enum_child_proc(HWND hwnd, LPARAM lParam);
BOOL util_detect_cutscene();

#endif
