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
#include "opengl.h"
#include "main.h"
#include "surface.h"

const GLchar *PaletteVertShaderSrc =
    "# version 110\n"
    "varying vec2 TexCoord0; \n"
    "\n"
    "void main(void)\n"
    "{\n"
    "    gl_Position = ftransform(); \n"
    "    TexCoord0 = gl_MultiTexCoord0.xy; \n"
    "}\n";

const GLchar *PaletteFragShaderSrc =
    "//Fragment shader\n"
    "#version 110\n"
    "uniform sampler2D PaletteTex; \n"
    "uniform sampler2D SurfaceTex; \n"
    "varying vec2 TexCoord0; \n"
    "\n"
    "void main()\n"
    "{\n"
    "   vec4 myindex = texture2D(SurfaceTex, TexCoord0); \n"
    "   vec4 texel = texture2D(PaletteTex, myindex.xy); \n"
    "   gl_FragColor = texel;\n"
    "}\n";


GLuint SurfaceTexId, PaletteTexId;
GLint SurfaceUniLoc, PaletteUniLoc;
GLuint PaletteConvProgram;

BOOL detect_cutscene();

DWORD WINAPI render_main(void)
{
    HGLRC hRC = wglCreateContext(ddraw->render.hDC);
    wglMakeCurrent(ddraw->render.hDC, hRC);

    OpenGL_Init();

    if (OpenGL_ExtExists("WGL_EXT_swap_control"))
    {
        BOOL(APIENTRY *wglSwapIntervalEXT)(int) = (BOOL(APIENTRY *)(int))wglGetProcAddress("wglSwapIntervalEXT");
        if (wglSwapIntervalEXT)
        {
            if (ddraw->vsync)
                wglSwapIntervalEXT(1);
            else
                wglSwapIntervalEXT(0);
        }
    }

    DWORD tick_start = 0;
    DWORD tick_end = 0;
    DWORD frame_len = 0;

    if (ddraw->render.maxfps < 0)
        ddraw->render.maxfps = ddraw->mode.dmDisplayFrequency;

    if (ddraw->render.maxfps > 0)
        frame_len = 1000.0f / ddraw->render.maxfps;

    int tex_width = ddraw->width > 1024 ? ddraw->width : 1024;
    int tex_height = ddraw->height > 1024 ? ddraw->height : 1024;
    int tex_size = tex_width * tex_height * sizeof(int);
    int *tex = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, tex_size);

    if (!PaletteConvProgram)
        PaletteConvProgram = OpenGL_BuildProgram(&PaletteVertShaderSrc, &PaletteFragShaderSrc);

    // primary surface texture
    glGenTextures(1, &SurfaceTexId);
    glBindTexture(GL_TEXTURE_2D, SurfaceTexId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    if (PaletteConvProgram)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, tex_width, tex_height, 0, GL_RED, GL_UNSIGNED_BYTE, 0);
    else
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tex_width, tex_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex);

    // palette texture
    glGenTextures(1, &PaletteTexId);
    glBindTexture(GL_TEXTURE_2D, PaletteTexId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 256, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

    glViewport(
        ddraw->render.viewport.x, ddraw->render.viewport.y,
        ddraw->render.viewport.width, ddraw->render.viewport.height);

    glBindTexture(GL_TEXTURE_2D, 0);

    if (PaletteConvProgram)
    {
        glUseProgram(PaletteConvProgram);
        SurfaceUniLoc = glGetUniformLocation(PaletteConvProgram, "SurfaceTex");
        PaletteUniLoc = glGetUniformLocation(PaletteConvProgram, "PaletteTex");
    }

    glEnable(GL_TEXTURE_2D);

    while (ddraw->render.run && WaitForSingleObject(ddraw->render.sem, INFINITE) != WAIT_FAILED)
    {
#if _DEBUG
        static DWORD tick_fps = 0;
        static DWORD frame_count = 0;
        tick_start = timeGetTime();
        if (tick_start >= tick_fps)
        {
            printf("Frames: %lu - Elapsed: %lu ms\n", frame_count, (tick_start - tick_fps) + 1000);
            frame_count = 0;
            tick_fps = tick_start + 1000;
        }
        frame_count++;
#endif

        float scale_w = (float)ddraw->width / tex_width;
        float scale_h = (float)ddraw->height / tex_height;

        if (ddraw->render.maxfps > 0)
            tick_start = timeGetTime();

        if (PaletteConvProgram && glActiveTexture && glUniform1i)
        {
            glActiveTexture(GL_TEXTURE0);
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, PaletteTexId);
            glUniform1i(PaletteUniLoc, 0);

            glActiveTexture(GL_TEXTURE1);
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, SurfaceTexId);
            glUniform1i(SurfaceUniLoc, 1);
        }

        EnterCriticalSection(&ddraw->cs);

        if (ddraw->primary && ddraw->primary->palette)
        {
            if (ddraw->vhack && detect_cutscene())
            {
                scale_w *= (float)CUTSCENE_WIDTH / ddraw->width;
                scale_h *= (float)CUTSCENE_HEIGHT / ddraw->height;

                if (!ddraw->incutscene)
                    ddraw->incutscene = TRUE;
            }
            else
            {
                if (ddraw->incutscene)
                    ddraw->incutscene = FALSE;
            }

            if (PaletteConvProgram)
            {
                glBindTexture(GL_TEXTURE_2D, PaletteTexId);
                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 256, 1, GL_RGBA, GL_UNSIGNED_BYTE, ddraw->primary->palette->data_bgr);

                glBindTexture(GL_TEXTURE_2D, SurfaceTexId);
                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, ddraw->width, ddraw->height, GL_RED, GL_UNSIGNED_BYTE, ddraw->primary->surface);
            }
            else
            {
                int i, j;
                for (i = 0; i<ddraw->height; i++)
                {
                    int i_dst = i*ddraw->width;
                    int i_src = i*ddraw->primary->lPitch;

                    for (j = 0; j<ddraw->width; j++)
                    {
                        tex[i_dst + j] =
                            ddraw->primary->palette->data_bgr[
                                ((unsigned char *)ddraw->primary->surface)[i_src + j*ddraw->primary->lXPitch]];
                    }
                }

            }
        }

        LeaveCriticalSection(&ddraw->cs);

        if (!PaletteConvProgram)
        {
            glBindTexture(GL_TEXTURE_2D, SurfaceTexId);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, ddraw->width, ddraw->height, GL_RGBA, GL_UNSIGNED_BYTE, tex);
        }

        glBegin(GL_TRIANGLE_FAN);
        glTexCoord2f(0, 0);              glVertex2f(-1, 1);
        glTexCoord2f(scale_w, 0);        glVertex2f(1, 1);
        glTexCoord2f(scale_w, scale_h);  glVertex2f(1, -1);
        glTexCoord2f(0, scale_h);        glVertex2f(-1, -1);
        glEnd();

        glBindTexture(GL_TEXTURE_2D, 0);

        SwapBuffers(ddraw->render.hDC);

        if (ddraw->render.maxfps > 0)
        {
            tick_end = timeGetTime();

            if (tick_end - tick_start < frame_len)
                Sleep(frame_len - (tick_end - tick_start));
        }

        SetEvent(ddraw->render.ev);
    }

    HeapFree(GetProcessHeap(), 0, tex);
    glDeleteTextures(1, &SurfaceTexId);
    glDeleteTextures(1, &PaletteTexId);

    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hRC);

    return 0;
}
