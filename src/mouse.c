/*
 * Copyright (c) 2010 Toni Spets <toni.spets@iki.fi>
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

/* This is a special mouse coordinate fix for games that use GetCursorPos and expect to be in fullscreen */

#include <windows.h>
#include <stdio.h>
#include "main.h"
#include "surface.h"

BOOL mouse_active = FALSE;
int yAdjust = 0;

BOOL WINAPI fake_GetCursorPos(LPPOINT lpPoint)
{
    POINT pt, realpt;
    
    if (!GetCursorPos(&pt) || !ddraw)
        return FALSE;
    
    realpt.x = pt.x;
    realpt.y = pt.y;
    
    if(ddraw->locked && (!ddraw->windowed || ScreenToClient(ddraw->hWnd, &pt)))
    {
        //fallback solution for possible ClipCursor failure
        int diffx = 0, diffy = 0;
        int maxWidth = ddraw->adjmouse ? ddraw->render.viewport.width : ddraw->width;
        int maxHeight = ddraw->adjmouse ? ddraw->render.viewport.height : ddraw->height;

        if (pt.x < 0)
        {
            diffx = pt.x;
            pt.x = 0;
        }

        if (pt.y < 0)
        {
            diffy = pt.y;
            pt.y = 0;
        }

        if (pt.x > maxWidth)
        {
            diffx = pt.x - maxWidth;
            pt.x = maxWidth;
        }

        if (pt.y > maxHeight)
        {
            diffy = pt.y - maxHeight;
            pt.y = maxHeight;
        }

        if (diffx || diffy)
            SetCursorPos(realpt.x - diffx, realpt.y - diffy);


        if(ddraw->adjmouse)
        {
            ddraw->cursor.x = pt.x * ((float)ddraw->width / ddraw->render.viewport.width);
            ddraw->cursor.y = pt.y * ((float)ddraw->height / ddraw->render.viewport.height);
        }
        else
        {
            ddraw->cursor.x = pt.x;
            ddraw->cursor.y = pt.y;
        }

        if (ddraw->vhack && InterlockedExchangeAdd(&ddraw->incutscene, 0))
        {
            diffx = 0;
            diffy = 0;

            if (ddraw->cursor.x > CUTSCENE_WIDTH)
            {
                diffx = ddraw->cursor.x - CUTSCENE_WIDTH;
                ddraw->cursor.x = CUTSCENE_WIDTH;
            }
                
            if (ddraw->cursor.y > CUTSCENE_HEIGHT)
            {
                diffy = ddraw->cursor.y - CUTSCENE_HEIGHT;
                ddraw->cursor.y = CUTSCENE_HEIGHT;
            }

            if (diffx || diffy)
                SetCursorPos(realpt.x - diffx, realpt.y - diffy);
        }
    }

    if (lpPoint)
    {
        if (ddraw->fakecursorpos)
        {
            lpPoint->x = (int)ddraw->cursor.x;
            lpPoint->y = (int)ddraw->cursor.y;
        }
        else if (ddraw->locked || ddraw->devmode)
        {
            lpPoint->x = realpt.x;
            lpPoint->y = realpt.y;
        }
        else
            return FALSE;
    }
    
    return TRUE;
}

BOOL WINAPI fake_ClipCursor(const RECT *lpRect)
{
    if(lpRect)
    {
        /* hack for 640x480 mode */
        if (lpRect->bottom == 400 && ddraw->height == 480)
            yAdjust = 40;
    }
    return TRUE;
}

int WINAPI fake_ShowCursor(BOOL bShow)
{
    if (ddraw && !ddraw->hidemouse)
        return ShowCursor(bShow);

    return TRUE;
}

HCURSOR WINAPI fake_SetCursor(HCURSOR hCursor)
{
    if (ddraw && !ddraw->hidemouse)
        return SetCursor(hCursor); 
    
    return NULL;
}

void HookIAT(HMODULE hMod, char *moduleName, char *functionName, PROC newFunction)
{
    if (!hMod || hMod == INVALID_HANDLE_VALUE || !newFunction)
        return;

    PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)hMod;
    if (pDosHeader->e_magic != IMAGE_DOS_SIGNATURE)
        return;

    PIMAGE_NT_HEADERS pNTHeaders = (PIMAGE_NT_HEADERS)((DWORD)pDosHeader + (DWORD)pDosHeader->e_lfanew);
    if (pNTHeaders->Signature != IMAGE_NT_SIGNATURE)
        return;

    PIMAGE_IMPORT_DESCRIPTOR pImportDescriptor = (PIMAGE_IMPORT_DESCRIPTOR)((DWORD)pDosHeader +
        (DWORD)(pNTHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress));

    if (pImportDescriptor == (PIMAGE_IMPORT_DESCRIPTOR)pNTHeaders)
        return;

    while (pImportDescriptor->FirstThunk)
    {
        char *impModuleName = (char *)((DWORD)pDosHeader + (DWORD)(pImportDescriptor->Name));

        if (_stricmp(impModuleName, moduleName) == 0)
        {
            PIMAGE_THUNK_DATA pFirstThunk = 
                (PIMAGE_THUNK_DATA)((DWORD)pDosHeader + (DWORD)pImportDescriptor->FirstThunk);

            PIMAGE_THUNK_DATA pOrigFirstThunk = 
                (PIMAGE_THUNK_DATA)((DWORD)pDosHeader + (DWORD)pImportDescriptor->OriginalFirstThunk);

            while (pFirstThunk->u1.Function && pOrigFirstThunk->u1.AddressOfData)
            {
                PIMAGE_IMPORT_BY_NAME pImport = 
                    (PIMAGE_IMPORT_BY_NAME)((DWORD)pDosHeader + pOrigFirstThunk->u1.AddressOfData);

                if ((pOrigFirstThunk->u1.Ordinal & IMAGE_ORDINAL_FLAG) == 0 &&
                    _stricmp((const char *)pImport->Name, functionName) == 0)
                {
                    DWORD oldProtect;
                    MEMORY_BASIC_INFORMATION mbi;

                    if (VirtualQuery(&pFirstThunk->u1.Function, &mbi, sizeof(MEMORY_BASIC_INFORMATION)))
                    {
                        if (VirtualProtect(mbi.BaseAddress, mbi.RegionSize, PAGE_READWRITE, &oldProtect))
                        {
                            pFirstThunk->u1.Function = (DWORD)newFunction;
                            VirtualProtect(mbi.BaseAddress, mbi.RegionSize, oldProtect, &oldProtect);
                        }
                    }

                    break;
                }

                pFirstThunk++;
                pOrigFirstThunk++;
            }
        }

        pImportDescriptor++;
    }
}

void mouse_lock()
{
    RECT rc;

    if (ddraw->devmode)
    {
        if (ddraw->hidemouse)
            while(ShowCursor(FALSE) > 0);

        return;
    }

    if (mouse_active && !ddraw->locked)
    {
        // Get the window client area.
        GetClientRect(ddraw->hWnd, &rc);
        
        if(ddraw->adjmouse)
        {
            rc.right = ddraw->render.viewport.width;
            rc.bottom = ddraw->render.viewport.height;
        }
        else
        {
            rc.right = ddraw->width;
            rc.bottom = ddraw->height;
        }

        // Convert the client area to screen coordinates.
        POINT pt = { rc.left, rc.top };
        POINT pt2 = { rc.right, rc.bottom };
        ClientToScreen(ddraw->hWnd, &pt);
        ClientToScreen(ddraw->hWnd, &pt2);
        
        SetRect(&rc, pt.x, pt.y, pt2.x, pt2.y);
        
        rc.bottom -= (yAdjust * 2) * ((float)ddraw->render.viewport.height / ddraw->height);

        if(ddraw->adjmouse)
        {
            SetCursorPos(
                rc.left + (ddraw->cursor.x * ((float)ddraw->render.viewport.width / ddraw->width)), 
                rc.top + ((ddraw->cursor.y - yAdjust) * ((float)ddraw->render.viewport.height / ddraw->height)));
        }
        else
        {
            SetCursorPos(rc.left + ddraw->cursor.x, rc.top + ddraw->cursor.y - yAdjust);
        }

        SetCapture(ddraw->hWnd);
        ClipCursor(&rc);

        if (ddraw->hidemouse)
            while(ShowCursor(FALSE) > 0);

        ddraw->locked = TRUE;
    }
}

void mouse_unlock()
{
    RECT rc;

    if (ddraw->devmode)
    {
        if (ddraw->hidemouse)
            while(ShowCursor(TRUE) < 0);

        return;
    }

    if(!mouse_active)
    {
        return;
    }

    if(ddraw->locked)
    {
        ddraw->locked = FALSE;

        // Get the window client area.
        GetClientRect(ddraw->hWnd, &rc);
        
        // Convert the client area to screen coordinates.
        POINT pt = { rc.left, rc.top };
        POINT pt2 = { rc.right, rc.bottom };
        ClientToScreen(ddraw->hWnd, &pt);
        ClientToScreen(ddraw->hWnd, &pt2);
        SetRect(&rc, pt.x, pt.y, pt2.x, pt2.y);
       
        if (ddraw->hidemouse)
        {
            while (ShowCursor(TRUE) < 0);
            SetCursor(LoadCursor(NULL, IDC_ARROW));
        }

        ClipCursor(NULL);
        ReleaseCapture();
        
        SetCursorPos(
            rc.left + ddraw->render.viewport.x + 
                (ddraw->cursor.x * ((float)ddraw->render.viewport.width / ddraw->width)), 
            rc.top + ddraw->render.viewport.y + 
                ((ddraw->cursor.y + yAdjust) * ((float)ddraw->render.viewport.height / ddraw->height)));

    }
}

void mouse_init()
{
    HookIAT(GetModuleHandle(NULL), "user32.dll", "GetCursorPos", (PROC)fake_GetCursorPos);
    HookIAT(GetModuleHandle(NULL), "user32.dll", "ClipCursor", (PROC)fake_ClipCursor);
    HookIAT(GetModuleHandle(NULL), "user32.dll", "ShowCursor", (PROC)fake_ShowCursor);
    HookIAT(GetModuleHandle(NULL), "user32.dll", "SetCursor", (PROC)fake_SetCursor);
    mouse_active = TRUE;
}
