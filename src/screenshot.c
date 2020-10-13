#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <time.h>
#include "dd.h"
#include "ddpalette.h"
#include "ddsurface.h"
#include "lodepng.h"

BOOL ss_take_screenshot(struct IDirectDrawSurfaceImpl *src)
{
    if (!src || !src->palette || !src->surface)
        return FALSE;

    int i;
    char title[128];
    char filename[128];
    char str_time[64];
    time_t t = time(NULL);

    strncpy(title, g_ddraw->title, sizeof(g_ddraw->title));

    for (i = 0; i<strlen(title); i++) {
        if (title[i] == ' ')
        {
            title[i] = '_';
        }
        else
        {
            title[i] = tolower(title[i]);
        }
    }

    strftime(str_time, 64, "%Y-%m-%d-%H_%M_%S", localtime(&t));
    _snprintf(filename, 128, "%s-%s.png", title, str_time);

    unsigned char* png;
    size_t pngsize;
    LodePNGState state;

    lodepng_state_init(&state);
    
    for (i = 0; i < 256; i++)
    {
        RGBQUAD *c = &src->palette->data_rgb[i];
        lodepng_palette_add(&state.info_png.color, c->rgbRed, c->rgbGreen, c->rgbBlue, 255);
        lodepng_palette_add(&state.info_raw, c->rgbRed, c->rgbGreen, c->rgbBlue, 255);
    }

    state.info_png.color.colortype = LCT_PALETTE;
    state.info_png.color.bitdepth = 8;
    state.info_raw.colortype = LCT_PALETTE;
    state.info_raw.bitdepth = 8;
    state.encoder.auto_convert = 0;

    unsigned int error = lodepng_encode(&png, &pngsize, src->surface, src->width, src->height, &state);
    if (!error) lodepng_save_file(png, pngsize, filename);

    lodepng_state_cleanup(&state);
    free(png);

    return !error;
}
