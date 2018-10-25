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
#include "opengl.h"

BOOL ShowDriverWarning;

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
    if (ShowDriverWarning)
    {
        if (!ddraw->windowed)
            PostMessage(ddraw->hWnd, WM_AUTORENDERER, 0, 0);

        _snprintf(
            warningText, sizeof(warningText), 
            "-WARNING- Using slow software rendering, please update your graphics card driver (%s)", 
            strlen(OpenGL_Version) > 10 ? "" : OpenGL_Version);
    }
    else
        Sleep(500);

    while (ddraw->render.run && WaitForSingleObject(ddraw->render.sem, INFINITE) != WAIT_FAILED)
    {
#if _DEBUG
        DrawFrameInfoStart();
#endif

        EnterCriticalSection(&ddraw->cs);

        if (ddraw->primary && ddraw->primary->palette && ddraw->primary->palette->data_rgb)
        {
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

            BOOL scaleCutscene = ddraw->vhack && detect_cutscene();

            if (ddraw->vhack)
                InterlockedExchange(&ddraw->incutscene, scaleCutscene);

            if (scaleCutscene)
            {
                StretchDIBits(
                    ddraw->render.hDC, 
                    ddraw->render.viewport.x, 
                    ddraw->render.viewport.y,
                    ddraw->render.viewport.width, 
                    ddraw->render.viewport.height,
                    0, 
                    ddraw->height - 400, 
                    CUTSCENE_WIDTH, 
                    CUTSCENE_HEIGHT, 
                    ddraw->primary->surface,
                    ddraw->primary->bmi, 
                    DIB_RGB_COLORS, 
                    SRCCOPY);
            }
            else if (ddraw->render.width != ddraw->width || ddraw->render.height != ddraw->height)
            {
                StretchDIBits(
                    ddraw->render.hDC, 
                    ddraw->render.viewport.x, 
                    ddraw->render.viewport.y, 
                    ddraw->render.viewport.width, 
                    ddraw->render.viewport.height, 
                    0, 
                    0, 
                    ddraw->width, 
                    ddraw->height, 
                    ddraw->primary->surface, 
                    ddraw->primary->bmi, 
                    DIB_RGB_COLORS, 
                    SRCCOPY);
            }
            else
            {
                SetDIBitsToDevice(
                    ddraw->render.hDC, 
                    0, 
                    0, 
                    ddraw->width, 
                    ddraw->height, 
                    0, 
                    0, 
                    0, 
                    ddraw->height, 
                    ddraw->primary->surface, 
                    ddraw->primary->bmi, 
                    DIB_RGB_COLORS);
            }
        }

        LeaveCriticalSection(&ddraw->cs);

#if _DEBUG
        DrawFrameInfoEnd();
#endif

        SetEvent(ddraw->render.ev);
    }

    return TRUE;
}
