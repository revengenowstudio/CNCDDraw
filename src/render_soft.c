/*
 * Copyright (c) 2011 Toni Spets <toni.spets@iki.fi>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <windows.h>
#include <stdio.h>

#include "main.h"
#include "surface.h"


static unsigned char getPixel(int x, int y)
{
    return ((unsigned char *)ddraw->primary->surface)[y*ddraw->primary->lPitch + x*ddraw->primary->lXPitch];
}

int* InMovie = (int*)0x00665F58;
int* IsVQA640 = (int*)0x0065D7BC; 
BYTE* ShouldStretch = (BYTE*)0x00607D78;

BOOL detect_cutscene()
{
    if(ddraw->width <= CUTSCENE_WIDTH || ddraw->height <= CUTSCENE_HEIGHT)
        return FALSE;
        
    //if (ddraw->isredalert && *InMovie)
    //    return !*IsVQA640;
        
    if (ddraw->isredalert)
    {
        if ((*InMovie && !*IsVQA640) || *ShouldStretch)
        {
            return TRUE;
        }
        return FALSE;
    }
    else if (ddraw->iscnc1)
    {
        return getPixel(CUTSCENE_WIDTH + 1, 0) == 0 || getPixel(CUTSCENE_WIDTH + 5, 1) == 0 ? TRUE : FALSE;
    }

    return FALSE;
}

DWORD WINAPI render_soft_main(void)
{
    DWORD warningEndTick = timeGetTime() + (15 * 1000);
    char warningText[512] = { 0 };
    if (OpenglVersion[0])
    {
        if (!ddraw->windowed)
            PostMessage(ddraw->hWnd, WM_AUTORENDERER, 0, 0);

        snprintf(
            warningText, sizeof(warningText), 
            "-WARNING- Using slow software rendering, please update your graphics card driver (%s)", 
            strlen(OpenglVersion) > 10 ? "" : OpenglVersion);
    }
    else
        Sleep(500);

    DWORD tick_start = 0;
    DWORD tick_end = 0;
    DWORD frame_len = 0;
    int maxfps = ddraw->render.maxfps;

    if(maxfps < 0)
       maxfps = ddraw->mode.dmDisplayFrequency;

    if (maxfps >= 1000)
        maxfps = 0;

    if(maxfps > 0)
        frame_len = 1000.0f / maxfps;

    while (ddraw->render.run && WaitForSingleObject(ddraw->render.sem, INFINITE) != WAIT_FAILED)
    {
#if _DEBUG
        static DWORD tick_fps = 0;
        static DWORD frame_count = 0;
        static char debugText[512] = { 0 };
        static double frameTime = 0;
        RECT debugrc = { 0, 0, ddraw->width, ddraw->height };

        if (ddraw->primary && ddraw->primary->palette)
            DrawText(ddraw->primary->hDC, debugText, -1, &debugrc, DT_NOCLIP);

        tick_start = timeGetTime();
        if (tick_start >= tick_fps)
        {
            snprintf(
                debugText, sizeof(debugText),
                "FPS: %lu | Time: %2.2f ms  ",
                frame_count, frameTime);

            frame_count = 0;
            tick_fps = tick_start + 1000;

            CounterStart();
        }
        frame_count++;
#endif

        if(maxfps > 0)
        {
            tick_start = timeGetTime();
        }
        
        EnterCriticalSection(&ddraw->cs);

        if (ddraw->primary && (ddraw->primary->palette || ddraw->bpp == 16))
        {
            if (ddraw->primary->palette && ddraw->primary->palette->data_rgb == NULL)
            {
                ddraw->primary->palette->data_rgb = &ddraw->primary->bmi->bmiColors[0];
            }

            if (warningText[0])
            {
                if (timeGetTime() < warningEndTick)
                {
                    RECT rc = { 0, 0, ddraw->width, ddraw->height };
                    DrawText(ddraw->primary->hDC, warningText, -1, &rc, DT_NOCLIP | DT_CENTER);
                }
                else
                    warningText[0] = 0;
            }

            if ((ddraw->render.width != ddraw->width || ddraw->render.height != ddraw->height) && 
                !(ddraw->vhack && detect_cutscene()) )
            {
                StretchDIBits(
                    ddraw->render.hDC, ddraw->render.viewport.x, ddraw->render.viewport.y, 
                    ddraw->render.viewport.width, ddraw->render.viewport.height, 
                    0, 0, ddraw->width, ddraw->height, ddraw->primary->surface, 
                    ddraw->primary->bmi, DIB_RGB_COLORS, SRCCOPY);
            }
            else if (!(ddraw->vhack && detect_cutscene()))
            {
                SetDIBitsToDevice(
                    ddraw->render.hDC, 0, 0, ddraw->width, ddraw->height, 0, 0, 0, 
                    ddraw->height, ddraw->primary->surface, ddraw->primary->bmi, DIB_RGB_COLORS);
            }

        }
        if (ddraw->vhack && ddraw->primary && detect_cutscene()) // for vhack
        {
            if (ddraw->primary->palette && ddraw->primary->palette->data_rgb == NULL)
            {
                ddraw->primary->palette->data_rgb = &ddraw->primary->bmi->bmiColors[0];
            }
            
            StretchDIBits(
                ddraw->render.hDC, ddraw->render.viewport.x, ddraw->render.viewport.y, 
                ddraw->render.viewport.width, ddraw->render.viewport.height, 
                0, ddraw->height-400, CUTSCENE_WIDTH, CUTSCENE_HEIGHT, ddraw->primary->surface, 
                ddraw->primary->bmi, DIB_RGB_COLORS, SRCCOPY);

            if (ddraw->primary->palette && !ddraw->incutscene)
            {
                ddraw->incutscene = TRUE;
            }
        }
        else if(ddraw->primary && ddraw->primary->palette && ddraw->incutscene)
        {
            ddraw->incutscene = FALSE;
        }

        LeaveCriticalSection(&ddraw->cs);

#if _DEBUG
        if (frame_count == 1) frameTime = CounterStop();
#endif

        if(maxfps > 0)
        {
            tick_end = timeGetTime();

            if(tick_end - tick_start < frame_len)
            {
                Sleep( frame_len - (tick_end - tick_start));
            }
        }
        
        SetEvent(ddraw->render.ev);
    }

    return TRUE;
}
