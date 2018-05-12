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

const GLchar *PassthroughVertShaderSrc =
    "//Vertex shader\n"
    "#version 110\n"
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
    "   vec4 index = texture2D(SurfaceTex, TexCoord0); \n"
    "   vec4 texel = texture2D(PaletteTex, index.xy); \n"
    "   gl_FragColor = texel;\n"
    "}\n";


BOOL detect_cutscene();

DWORD WINAPI render_main(void)
{
    Sleep(500);

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

    if (ddraw->render.maxfps == 0)
        ddraw->render.maxfps = 125;

    if (ddraw->render.maxfps >= 1000)
        ddraw->render.maxfps = 0;

    if (ddraw->render.maxfps > 0)
        frame_len = 1000.0f / ddraw->render.maxfps;

    int tex_width = ddraw->width > 1024 ? ddraw->width : 1024;
    int tex_height = ddraw->height > 1024 ? ddraw->height : 1024;
    int tex_size = tex_width * tex_height * sizeof(int);
    int *tex = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, tex_size);

    GLuint paletteConvProgram = 0; 
    if (glGetUniformLocation && glActiveTexture && glUniform1i)
        paletteConvProgram = OpenGL_BuildProgram(PassthroughVertShaderSrc, PaletteFragShaderSrc);

    GLuint scaleProgram = 0;
    if (glGenFramebuffers && glBindFramebuffer && glFramebufferTexture2D && glDrawBuffers && 
        glCheckFramebufferStatus && glUniform4f && glActiveTexture && glUniform1i &&
        glGetAttribLocation && glGenBuffers && glBindBuffer && glBufferData && glVertexAttribPointer &&
        glEnableVertexAttribArray && glUniform2fv && glUniformMatrix4fv)
        scaleProgram = OpenGL_BuildProgramFromFile(ddraw->shader);

    // primary surface texture
    GLuint surfaceTexId = 0;
    glGenTextures(1, &surfaceTexId);
    glBindTexture(GL_TEXTURE_2D, surfaceTexId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    while (glGetError() != GL_NO_ERROR);

    if (paletteConvProgram)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, tex_width, tex_height, 0, GL_RED, GL_UNSIGNED_BYTE, 0);

        if (glGetError() != GL_NO_ERROR)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, tex_width, tex_height, 0, GL_RED, GL_UNSIGNED_BYTE, 0);
    }
    else
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, tex_width, tex_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex);

        if (glGetError() != GL_NO_ERROR)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tex_width, tex_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex);
    }


    // palette texture
    GLuint paletteTexId = 0;
    glGenTextures(1, &paletteTexId);
    glBindTexture(GL_TEXTURE_2D, paletteTexId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    while (glGetError() != GL_NO_ERROR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 256, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    if (glGetError() != GL_NO_ERROR)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

    glViewport(
        ddraw->render.viewport.x, ddraw->render.viewport.y,
        ddraw->render.viewport.width, ddraw->render.viewport.height);

    GLint surfaceUniLoc = 0, paletteUniLoc = 0;
    if (paletteConvProgram)
    {
        surfaceUniLoc = glGetUniformLocation(paletteConvProgram, "SurfaceTex");
        paletteUniLoc = glGetUniformLocation(paletteConvProgram, "PaletteTex");
    }

    GLint textureUniLoc = -1, texCoordUniLoc = -1, frameCountUniLoc = -1;
    GLuint frameBufferId = 0;
    GLuint frameBufferTexId = 0;
    GLuint vboBuffers[3];

    if (scaleProgram)
    {
        glUseProgram(scaleProgram);

        GLint vertexCoordUniLoc = glGetAttribLocation(scaleProgram, "VertexCoord");
        texCoordUniLoc = glGetAttribLocation(scaleProgram, "TexCoord");
        textureUniLoc = glGetUniformLocation(scaleProgram, "Texture");
        frameCountUniLoc = glGetUniformLocation(scaleProgram, "FrameCount");

        glGenBuffers(3, vboBuffers);

        glBindBuffer(GL_ARRAY_BUFFER, vboBuffers[0]);
        static const GLfloat vertexCoord[] = {
           -1.0f, 1.0f,
            1.0f, 1.0f,
            1.0f,-1.0f,
           -1.0f,-1.0f,
        };
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertexCoord), vertexCoord, GL_STATIC_DRAW);
        glVertexAttribPointer(vertexCoordUniLoc, 2, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(vertexCoordUniLoc);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboBuffers[2]);
        static const GLushort indices[] =
        {
            0, 1, 2,
            0, 2, 3,
        };
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        float inputSize[2], outputSize[2], textureSize[2];

        inputSize[0] = ddraw->width;
        inputSize[1] = ddraw->height;
        textureSize[0] = tex_width;
        textureSize[1] = tex_height;
        outputSize[0] = ddraw->render.viewport.width;
        outputSize[1] = ddraw->render.viewport.height;

        glUniform2fv(glGetUniformLocation(scaleProgram, "OutputSize"), 1, outputSize);
        glUniform2fv(glGetUniformLocation(scaleProgram, "TextureSize"), 1, textureSize);
        glUniform2fv(glGetUniformLocation(scaleProgram, "InputSize"), 1, inputSize);

        glUniform4f(glGetAttribLocation(scaleProgram, "Color"), 1, 1, 1, 1);
        glUniform1i(glGetUniformLocation(scaleProgram, "FrameDirection"), 1);

        const float mvpMatrix[16] = {
            1,0,0,0,
            0,1,0,0,
            0,0,1,0,
            0,0,0,1,
        };
        glUniformMatrix4fv(glGetUniformLocation(scaleProgram, "MVPMatrix"), 1, GL_FALSE, mvpMatrix);

        glGenFramebuffers(1, &frameBufferId);
        glBindFramebuffer(GL_FRAMEBUFFER, frameBufferId);

        glGenTextures(1, &frameBufferTexId);
        glBindTexture(GL_TEXTURE_2D, frameBufferTexId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, tex_width, tex_height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, frameBufferTexId, 0);

        GLenum drawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
        glDrawBuffers(1, drawBuffers);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            glDeleteTextures(1, &frameBufferTexId);

            if (glDeleteFramebuffers)
                glDeleteFramebuffers(1, &frameBufferId);

            if (glDeleteProgram)
                glDeleteProgram(scaleProgram);

            scaleProgram = 0;

            if (glDeleteBuffers)
                glDeleteBuffers(3, vboBuffers);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
 
    glBindTexture(GL_TEXTURE_2D, 0);


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

            if (paletteConvProgram)
            {
                glBindTexture(GL_TEXTURE_2D, paletteTexId);
                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 256, 1, GL_RGBA, GL_UNSIGNED_BYTE, ddraw->primary->palette->data_bgr);

                glBindTexture(GL_TEXTURE_2D, surfaceTexId);
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

        if (!paletteConvProgram)
        {
            glBindTexture(GL_TEXTURE_2D, surfaceTexId);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, ddraw->width, ddraw->height, GL_RGBA, GL_UNSIGNED_BYTE, tex);
        }

        if (paletteConvProgram)
        {
            glUseProgram(paletteConvProgram);

            glActiveTexture(GL_TEXTURE0);
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, paletteTexId);
            glUniform1i(paletteUniLoc, 0);

            glActiveTexture(GL_TEXTURE1);
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, surfaceTexId);
            glUniform1i(surfaceUniLoc, 1);

            glActiveTexture(GL_TEXTURE0);
        }

        if (scaleProgram)
        {
            // draw surface into framebuffer

            glBindFramebuffer(GL_FRAMEBUFFER, frameBufferId);

            glPushAttrib(GL_VIEWPORT_BIT);
            glViewport(0, 0, tex_width, tex_height);

            glBegin(GL_TRIANGLE_FAN);
            glTexCoord2f(0, 0);   glVertex2f(-1, -1);
            glTexCoord2f(0, 1);   glVertex2f(-1, 1);
            glTexCoord2f(1, 1);   glVertex2f(1, 1);
            glTexCoord2f(1, 0);   glVertex2f(1, -1);
            glEnd();

            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            glPopAttrib();
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, 0);

            // apply filter

            glBindBuffer(GL_ARRAY_BUFFER, vboBuffers[1]);

            GLfloat texCoord[] = {
                0.0f,    0.0f,
                scale_w, 0.0f,
                scale_w, scale_h,
                0,       scale_h,
            };

            glBufferData(GL_ARRAY_BUFFER, sizeof(texCoord), texCoord, GL_STATIC_DRAW);
            glVertexAttribPointer(texCoordUniLoc, 2, GL_FLOAT, GL_FALSE, 0, NULL);
            glEnableVertexAttribArray(texCoordUniLoc);

            glBindBuffer(GL_ARRAY_BUFFER, 0);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboBuffers[2]);

            glUseProgram(scaleProgram);
            glActiveTexture(GL_TEXTURE0);
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, frameBufferTexId);
            glUniform1i(textureUniLoc, 0);

            static int frames = 1;
            if (frameCountUniLoc != -1)
                glUniform1i(frameCountUniLoc, frames++);

            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        }
        else
        {
            glBegin(GL_TRIANGLE_FAN);
            glTexCoord2f(0, 0);              glVertex2f(-1, 1);
            glTexCoord2f(scale_w, 0);        glVertex2f(1, 1);
            glTexCoord2f(scale_w, scale_h);  glVertex2f(1, -1);
            glTexCoord2f(0, scale_h);        glVertex2f(-1, -1);
            glEnd();
        }

        if (scaleProgram && !paletteConvProgram)
            glUseProgram(0);

        glBindTexture(GL_TEXTURE_2D, 0);

        SwapBuffers(ddraw->render.hDC);

        if (ddraw->render.maxfps > 0)
        {
            tick_end = timeGetTime();

            if (tick_end - tick_start < frame_len)
                Sleep(frame_len - (tick_end - tick_start));
        }
    }

    HeapFree(GetProcessHeap(), 0, tex);
    glDeleteTextures(1, &surfaceTexId);
    glDeleteTextures(1, &paletteTexId);
    
    if (glUseProgram)
        glUseProgram(0);

    if (scaleProgram)
    {
        glDeleteTextures(1, &frameBufferTexId);

        if (glDeleteBuffers)
            glDeleteBuffers(3, vboBuffers);

        if (glDeleteFramebuffers)
            glDeleteFramebuffers(1, &frameBufferId);
    } 

    if (glDeleteProgram)
    {
        if (paletteConvProgram)
            glDeleteProgram(paletteConvProgram);

        if (scaleProgram)
            glDeleteProgram(scaleProgram);
    }
        
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hRC);

    return 0;
}
