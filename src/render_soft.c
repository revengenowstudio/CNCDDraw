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

        if (ddraw->primary && (ddraw->bpp == 16 || (ddraw->primary->palette && ddraw->primary->palette->data_rgb)))
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

            if (!ddraw->handlemouse)
            {
                ChildWindowExists = FALSE;
                EnumChildWindows(ddraw->hWnd, EnumChildProc, (LPARAM)ddraw->primary);
            }

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
            else if (!ChildWindowExists && (ddraw->render.width != ddraw->width || ddraw->render.height != ddraw->height))
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
