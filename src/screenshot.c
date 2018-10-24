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
#include <windows.h>
#include <stdio.h>
#include <ctype.h>
#include "ddraw.h"

#include "palette.h"
#include "surface.h"

#ifndef HAVE_LIBPNG

//https://docs.microsoft.com/en-us/windows/desktop/gdi/storing-an-image

BOOL screenshot(struct IDirectDrawSurfaceImpl *src)
{
    HANDLE hf;                 // file handle  
    BITMAPFILEHEADER hdr;       // bitmap file-header  
    PBITMAPINFOHEADER pbih;     // bitmap info-header  
    LPBYTE lpBits;              // memory pointer  
    DWORD dwTotal;              // total count of bytes  
    DWORD cb;                   // incremental count of bytes  
    BYTE *hp;                   // byte pointer  
    DWORD dwTmp;
    int i;
    char title[128];
    char filename[128];
    char str_time[64];
    time_t t = time(NULL);
    BOOL result = TRUE;

    strncpy(title, ddraw->title, sizeof(ddraw->title));

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
    _snprintf(filename, 128, "%s-%s.bmp", title, str_time);

    pbih = (PBITMAPINFOHEADER)src->bmi;
    lpBits = (LPBYTE)src->surface;

    if (!lpBits)
        return FALSE;

    // Create the .BMP file.  
    hf = CreateFile(filename,
        GENERIC_READ | GENERIC_WRITE,
        (DWORD)0,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        (HANDLE)NULL);

    if (hf == INVALID_HANDLE_VALUE)
        return FALSE;

    hdr.bfType = 0x4d42;        // 0x42 = "B" 0x4d = "M"  
                                // Compute the size of the entire file.  
    hdr.bfSize = (DWORD)(sizeof(BITMAPFILEHEADER) +
        pbih->biSize + pbih->biClrUsed
        * sizeof(RGBQUAD) + pbih->biSizeImage);
    hdr.bfReserved1 = 0;
    hdr.bfReserved2 = 0;

    // Compute the offset to the array of color indices.  
    hdr.bfOffBits = (DWORD) sizeof(BITMAPFILEHEADER) +
        pbih->biSize + pbih->biClrUsed
        * sizeof(RGBQUAD);

    // Copy the BITMAPFILEHEADER into the .BMP file.  
    if (!WriteFile(hf, (LPVOID)&hdr, sizeof(BITMAPFILEHEADER),
        (LPDWORD)&dwTmp, NULL))
    {
        result = FALSE;
    }

    // Copy the BITMAPINFOHEADER and RGBQUAD array into the file.  
    if (!WriteFile(hf, (LPVOID)pbih, sizeof(BITMAPINFOHEADER)
        + pbih->biClrUsed * sizeof(RGBQUAD),
        (LPDWORD)&dwTmp, (NULL)))
        result = FALSE;

    // Copy the array of color indices into the .BMP file.  
    dwTotal = cb = pbih->biSizeImage;
    hp = lpBits;
    if (!WriteFile(hf, (LPSTR)hp, (int)cb, (LPDWORD)&dwTmp, NULL))
        result = FALSE;

    // Close the .BMP file.  
    if (!CloseHandle(hf))
        result = FALSE;

    return result;
}

#else

#include <png.h>

BOOL screenshot(struct IDirectDrawSurfaceImpl *src)
{
    int i;
    FILE *fh;
    char title[128];
    char filename[128];

    png_structp png_ptr;
    png_infop info_ptr;
    png_bytep *row_pointers;
    png_color palette[256];

    char str_time[64];
    time_t t = time(NULL);

    strncpy(title, ddraw->title, sizeof(ddraw->title));

    for(i=0;i<strlen(title);i++) {
        if(title[i] == ' ')
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

    if(!src || !src->palette)
    {
        return FALSE;
    }

    if( !(fh = fopen(filename, "wb")) )
    {
        return FALSE;
    }

    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if(!png_ptr)
    {
        return FALSE;
    }

    info_ptr = png_create_info_struct(png_ptr);

    for(i=0;i<256;i++) {
        palette[i].red = src->palette->data_bgr[i];
        palette[i].green = src->palette->data_bgr[i] >> 8;
        palette[i].blue = src->palette->data_bgr[i] >> 16;
    }

    setjmp(png_jmpbuf(png_ptr));

    png_init_io(png_ptr, fh);

    png_set_IHDR(png_ptr, info_ptr, src->width, src->height, 8, PNG_COLOR_TYPE_PALETTE, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
    png_set_PLTE(png_ptr, info_ptr, (png_colorp)&palette, 256);

    row_pointers = (png_bytep *)png_malloc(png_ptr, sizeof(png_bytep) * src->height);

    for(i=0;i<src->height;i++) {
        row_pointers[i] = src->surface + (src->width * i);
    }

    png_set_rows(png_ptr, info_ptr, row_pointers);

    png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

    fclose(fh);

    return TRUE;
}

#endif
