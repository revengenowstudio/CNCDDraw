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
#include "paletteshader.h"


BOOL detect_cutscene();
DWORD WINAPI render_soft_main(void);

DWORD WINAPI render_main(void)
{
    Sleep(500);

    HGLRC hRC = wglCreateContext(ddraw->render.hDC);
    BOOL madeCurrent = hRC && wglMakeCurrent(ddraw->render.hDC, hRC);

    if (!madeCurrent || (ddraw->autorenderer && glGetError() != GL_NO_ERROR))
    {
        if (madeCurrent)
        {
            wglMakeCurrent(NULL, NULL);
            wglDeleteContext(hRC);
        }

        ddraw->renderer = render_soft_main;
        return render_soft_main();
    }

    OpenGL_Init();

    if (OpenGL_ExtExists("WGL_EXT_swap_control"))
    {
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
    int maxfps = ddraw->render.maxfps;

    if (maxfps < 0)
        maxfps = ddraw->mode.dmDisplayFrequency;

    if (maxfps == 0)
        maxfps = 125;

    if (maxfps >= 1000)
        maxfps = 0;

    if (maxfps > 0)
        frame_len = 1000.0f / maxfps;

    int tex_width = 
        ddraw->width <= 1024 ? 1024 : ddraw->width <= 2048 ? 2048 : ddraw->width <= 4096 ? 4096 : ddraw->width;
    int tex_height = 
        ddraw->height <= tex_width ? tex_width : ddraw->height <= 2048 ? 2048 : ddraw->height <= 4096 ? 4096 : ddraw->height;
    
    tex_width = tex_width > tex_height ? tex_width : tex_height;

    float scale_w = (float)ddraw->width / tex_width;
    float scale_h = (float)ddraw->height / tex_height;

    int tex_size = tex_width * tex_height * sizeof(int);
    int *tex = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, tex_size);

    BOOL gotOpenglV3 = glGenFramebuffers && glBindFramebuffer && glFramebufferTexture2D && glDrawBuffers &&
        glCheckFramebufferStatus && glUniform4f && glActiveTexture && glUniform1i &&
        glGetAttribLocation && glGenBuffers && glBindBuffer && glBufferData && glVertexAttribPointer &&
        glEnableVertexAttribArray && glUniform2fv && glUniformMatrix4fv && glGenVertexArrays && glBindVertexArray &&
        glGetUniformLocation;

    BOOL gotOpenglV2 = glGetUniformLocation && glActiveTexture && glUniform1i;

    GLuint paletteConvProgram = 0; 
    if (gotOpenglV3)
        paletteConvProgram = OpenGL_BuildProgram(PassthroughVertShaderSrc, PaletteFragShaderSrc);
    else if (gotOpenglV2)
        paletteConvProgram = OpenGL_BuildProgram(PassthroughVertShader110Src, PaletteFragShader110Src);

    GLuint scaleProgram = 0;
    if (gotOpenglV3)
        scaleProgram = OpenGL_BuildProgramFromFile(ddraw->shader);

    // primary surface texture
    GLenum surfaceFormat = GL_LUMINANCE;
    GLuint surfaceTexId = 0;
    glGenTextures(1, &surfaceTexId);
    glBindTexture(GL_TEXTURE_2D, surfaceTexId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    while (glGetError() != GL_NO_ERROR);

    if (paletteConvProgram)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE8, tex_width, tex_height, 0, surfaceFormat = GL_LUMINANCE, GL_UNSIGNED_BYTE, 0);

        if (glGetError() != GL_NO_ERROR)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, tex_width, tex_height, 0, surfaceFormat = GL_RED, GL_UNSIGNED_BYTE, 0);

        if (glGetError() != GL_NO_ERROR)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, tex_width, tex_height, 0, surfaceFormat = GL_RED, GL_UNSIGNED_BYTE, 0);

        if (!ddraw->autorenderer && glGetError() != GL_NO_ERROR) // very slow...
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, tex_width, tex_height, 0, surfaceFormat = GL_RED, GL_UNSIGNED_BYTE, 0);
    }
    else
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, tex_width, tex_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    }


    // palette texture
    GLuint paletteTexId = 0;
    glGenTextures(1, &paletteTexId);
    glBindTexture(GL_TEXTURE_2D, paletteTexId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

    glViewport(
        ddraw->render.viewport.x, ddraw->render.viewport.y,
        ddraw->render.viewport.width, ddraw->render.viewport.height);


    GLint mainTexCoordAttrLoc = -1, mainVertexCoordAttrLoc = -1;
    GLuint mainVbos[3], mainVao;
    if (paletteConvProgram)
    {
        glUseProgram(paletteConvProgram);

        if (gotOpenglV3)
        {
            mainVertexCoordAttrLoc = glGetAttribLocation(paletteConvProgram, "VertexCoord");
            mainTexCoordAttrLoc = glGetAttribLocation(paletteConvProgram, "TexCoord");

            glGenBuffers(3, mainVbos);

            if (scaleProgram)
            {
                glBindBuffer(GL_ARRAY_BUFFER, mainVbos[0]);
                static const GLfloat vertexCoord[] = {
                    -1.0f, -1.0f,
                    -1.0f,  1.0f,
                     1.0f,  1.0f,
                     1.0f, -1.0f,
                };
                glBufferData(GL_ARRAY_BUFFER, sizeof(vertexCoord), vertexCoord, GL_STATIC_DRAW);
                glBindBuffer(GL_ARRAY_BUFFER, 0);

                glBindBuffer(GL_ARRAY_BUFFER, mainVbos[1]);
                GLfloat texCoord[] = {
                    0.0f,    0.0f,
                    0.0f,    scale_h,
                    scale_w, scale_h,
                    scale_w, 0.0f,
                };
                glBufferData(GL_ARRAY_BUFFER, sizeof(texCoord), texCoord, GL_STATIC_DRAW);
                glBindBuffer(GL_ARRAY_BUFFER, 0);
            }
            else
            {
                glBindBuffer(GL_ARRAY_BUFFER, mainVbos[0]);
                static const GLfloat vertexCoord[] = {
                    -1.0f, 1.0f,
                     1.0f, 1.0f,
                     1.0f,-1.0f,
                    -1.0f,-1.0f,
                };
                glBufferData(GL_ARRAY_BUFFER, sizeof(vertexCoord), vertexCoord, GL_STATIC_DRAW);
                glBindBuffer(GL_ARRAY_BUFFER, 0);

                glBindBuffer(GL_ARRAY_BUFFER, mainVbos[1]);
                GLfloat texCoord[] = {
                    0.0f,    0.0f,
                    scale_w, 0.0f,
                    scale_w, scale_h,
                    0.0f,    scale_h,
                };
                glBufferData(GL_ARRAY_BUFFER, sizeof(texCoord), texCoord, GL_STATIC_DRAW);
                glBindBuffer(GL_ARRAY_BUFFER, 0);
            }
            
            glGenVertexArrays(1, &mainVao);
            glBindVertexArray(mainVao);

            glBindBuffer(GL_ARRAY_BUFFER, mainVbos[0]);
            glVertexAttribPointer(mainVertexCoordAttrLoc, 2, GL_FLOAT, GL_FALSE, 0, NULL);
            glEnableVertexAttribArray(mainVertexCoordAttrLoc);
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            glBindBuffer(GL_ARRAY_BUFFER, mainVbos[1]);
            glVertexAttribPointer(mainTexCoordAttrLoc, 2, GL_FLOAT, GL_FALSE, 0, NULL);
            glEnableVertexAttribArray(mainTexCoordAttrLoc);
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mainVbos[2]);
            static const GLushort indices[] =
            {
                0, 1, 2,
                0, 2, 3,
            };
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

            glBindVertexArray(0);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

            const float mvpMatrix[16] = {
                1,0,0,0,
                0,1,0,0,
                0,0,1,0,
                0,0,0,1,
            };
            glUniformMatrix4fv(glGetUniformLocation(paletteConvProgram, "MVPMatrix"), 1, GL_FALSE, mvpMatrix);

        }

        glActiveTexture(GL_TEXTURE0);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, surfaceTexId);
        glUniform1i(glGetUniformLocation(paletteConvProgram, "SurfaceTex"), 0);

        glActiveTexture(GL_TEXTURE1);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, paletteTexId);
        glUniform1i(glGetUniformLocation(paletteConvProgram, "PaletteTex"), 1);

        glActiveTexture(GL_TEXTURE0);
    }

    GLint frameCountUniLoc = -1;
    GLuint frameBufferId = 0;
    GLuint frameBufferTexId = 0;
    GLuint scaleVbos[3], scaleVao;

    if (scaleProgram)
    {
        glUseProgram(scaleProgram);

        GLint vertexCoordAttrLoc = glGetAttribLocation(scaleProgram, "VertexCoord");
        GLint texCoordAttrLoc = glGetAttribLocation(scaleProgram, "TexCoord");
        frameCountUniLoc = glGetUniformLocation(scaleProgram, "FrameCount");

        glGenBuffers(3, scaleVbos);

        glBindBuffer(GL_ARRAY_BUFFER, scaleVbos[0]);
        static const GLfloat vertexCoord[] = {
           -1.0f, 1.0f,
            1.0f, 1.0f,
            1.0f,-1.0f,
           -1.0f,-1.0f,
        };
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertexCoord), vertexCoord, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glBindBuffer(GL_ARRAY_BUFFER, scaleVbos[1]);
        GLfloat texCoord[] = {
            0.0f,    0.0f,
            scale_w, 0.0f,
            scale_w, scale_h,
            0.0f,    scale_h,
        };
        glBufferData(GL_ARRAY_BUFFER, sizeof(texCoord), texCoord, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glGenVertexArrays(1, &scaleVao);
        glBindVertexArray(scaleVao);

        glBindBuffer(GL_ARRAY_BUFFER, scaleVbos[0]);
        glVertexAttribPointer(vertexCoordAttrLoc, 2, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(vertexCoordAttrLoc);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glBindBuffer(GL_ARRAY_BUFFER, scaleVbos[1]);
        glVertexAttribPointer(texCoordAttrLoc, 2, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(texCoordAttrLoc);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, scaleVbos[2]);
        static const GLushort indices[] =
        {
            0, 1, 2,
            0, 2, 3,
        };
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        glBindVertexArray(0);
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
        glUniform1i(glGetUniformLocation(scaleProgram, "FrameDirection"), 1);
        glUniform1i(glGetUniformLocation(scaleProgram, "Texture"), 0);

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
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, tex_width, tex_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

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
                glDeleteBuffers(3, scaleVbos);

            if (glDeleteVertexArrays)
                glDeleteVertexArrays(1, &scaleVao);

            if (paletteConvProgram)
            {
                glBindVertexArray(mainVao);
                glBindBuffer(GL_ARRAY_BUFFER, mainVbos[0]);
                static const GLfloat vertexCoordPal[] = {
                    -1.0f, 1.0f,
                     1.0f, 1.0f,
                     1.0f,-1.0f,
                    -1.0f,-1.0f,
                };
                glBufferData(GL_ARRAY_BUFFER, sizeof(vertexCoordPal), vertexCoordPal, GL_STATIC_DRAW);
                glVertexAttribPointer(mainVertexCoordAttrLoc, 2, GL_FLOAT, GL_FALSE, 0, NULL);
                glEnableVertexAttribArray(mainVertexCoordAttrLoc);
                glBindBuffer(GL_ARRAY_BUFFER, 0);
                glBindVertexArray(0);

                glBindVertexArray(mainVao);
                glBindBuffer(GL_ARRAY_BUFFER, mainVbos[1]);
                GLfloat texCoordPal[] = {
                    0.0f,    0.0f,
                    scale_w, 0.0f,
                    scale_w, scale_h,
                    0.0f,    scale_h,
                };
                glBufferData(GL_ARRAY_BUFFER, sizeof(texCoordPal), texCoordPal, GL_STATIC_DRAW);
                glVertexAttribPointer(mainTexCoordAttrLoc, 2, GL_FLOAT, GL_FALSE, 0, NULL);
                glEnableVertexAttribArray(mainTexCoordAttrLoc);
                glBindBuffer(GL_ARRAY_BUFFER, 0);
                glBindVertexArray(0);

                glUseProgram(paletteConvProgram);
            }
        }
        else
            glUseProgram(0);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
 
    glEnable(GL_TEXTURE_2D);

    BOOL useOpenGL = !(ddraw->autorenderer && (!paletteConvProgram || glGetError() != GL_NO_ERROR));

    while (useOpenGL && ddraw->render.run && WaitForSingleObject(ddraw->render.sem, INFINITE) != WAIT_FAILED)
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

        scale_w = (float)ddraw->width / tex_width;
        scale_h = (float)ddraw->height / tex_height;

        BOOL scaleChanged = FALSE;

        if (maxfps > 0)
            tick_start = timeGetTime();

        EnterCriticalSection(&ddraw->cs);

        if (ddraw->primary && ddraw->primary->palette)
        {
            if (ddraw->vhack && detect_cutscene())
            {
                scale_w *= (float)CUTSCENE_WIDTH / ddraw->width;
                scale_h *= (float)CUTSCENE_HEIGHT / ddraw->height;

                if (!ddraw->incutscene)
                {
                    scaleChanged = TRUE;
                    ddraw->incutscene = TRUE;
                }

            }
            else
            {
                if (ddraw->incutscene)
                {
                    scaleChanged = TRUE;
                    ddraw->incutscene = FALSE;
                }
            }

            if (paletteConvProgram)
            {
                glBindTexture(GL_TEXTURE_2D, paletteTexId);
                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 256, 1, GL_RGBA, GL_UNSIGNED_BYTE, ddraw->primary->palette->data_bgr);

                glBindTexture(GL_TEXTURE_2D, surfaceTexId);
                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, ddraw->width, ddraw->height, surfaceFormat, GL_UNSIGNED_BYTE, ddraw->primary->surface);
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

        if (scaleChanged)
        {
            if (scaleProgram && paletteConvProgram)
            {
                glBindVertexArray(mainVao);
                glBindBuffer(GL_ARRAY_BUFFER, mainVbos[1]);
                GLfloat texCoord[] = {
                    0.0f,    0.0f,
                    0.0f,    scale_h,
                    scale_w, scale_h,
                    scale_w, 0.0f,
                };
                glBufferData(GL_ARRAY_BUFFER, sizeof(texCoord), texCoord, GL_STATIC_DRAW);
                glVertexAttribPointer(mainTexCoordAttrLoc, 2, GL_FLOAT, GL_FALSE, 0, NULL);
                glEnableVertexAttribArray(mainTexCoordAttrLoc);
                glBindBuffer(GL_ARRAY_BUFFER, 0);
                glBindVertexArray(0);
            }
            else if (gotOpenglV3 && paletteConvProgram)
            {
                glBindVertexArray(mainVao);
                glBindBuffer(GL_ARRAY_BUFFER, mainVbos[1]);
                GLfloat texCoord[] = {
                    0.0f,    0.0f,
                    scale_w, 0.0f,
                    scale_w, scale_h,
                    0.0f,    scale_h,
                };
                glBufferData(GL_ARRAY_BUFFER, sizeof(texCoord), texCoord, GL_STATIC_DRAW);
                glVertexAttribPointer(mainTexCoordAttrLoc, 2, GL_FLOAT, GL_FALSE, 0, NULL);
                glEnableVertexAttribArray(mainTexCoordAttrLoc);
                glBindBuffer(GL_ARRAY_BUFFER, 0);
                glBindVertexArray(0);
            }
        }

        if (paletteConvProgram)
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, surfaceTexId);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, paletteTexId);
            glActiveTexture(GL_TEXTURE0);
        }

        if (scaleProgram && paletteConvProgram)
        {
            // draw surface into framebuffer
            glUseProgram(paletteConvProgram);

            glViewport(0, 0, ddraw->width, ddraw->height);

            glBindFramebuffer(GL_FRAMEBUFFER, frameBufferId);

            glBindVertexArray(mainVao);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
            glBindVertexArray(0);

            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, 0);

            glViewport(
                ddraw->render.viewport.x, ddraw->render.viewport.y,
                ddraw->render.viewport.width, ddraw->render.viewport.height);

            // apply filter

            glUseProgram(scaleProgram);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, frameBufferTexId);

            static int frames = 1;
            if (frameCountUniLoc != -1)
                glUniform1i(frameCountUniLoc, frames++);

            glBindVertexArray(scaleVao);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
            glBindVertexArray(0);
        }
        else if (gotOpenglV3 && paletteConvProgram)
        {
            glBindVertexArray(mainVao);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
            glBindVertexArray(0);
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

        SwapBuffers(ddraw->render.hDC);

        if (maxfps > 0)
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
            glDeleteBuffers(3, scaleVbos);

        if (glDeleteFramebuffers)
            glDeleteFramebuffers(1, &frameBufferId);

        if (glDeleteVertexArrays)
            glDeleteVertexArrays(1, &scaleVao);
    } 

    if (glDeleteProgram)
    {
        if (paletteConvProgram)
            glDeleteProgram(paletteConvProgram);

        if (scaleProgram)
            glDeleteProgram(scaleProgram);
    }

    if (gotOpenglV3)
    {
        if (paletteConvProgram)
        {
            if (glDeleteBuffers)
                glDeleteBuffers(3, mainVbos);

            if (glDeleteVertexArrays)
                glDeleteVertexArrays(1, &mainVao);
        }
    }
        
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hRC);

    if (!useOpenGL)
    {
        ddraw->renderer = render_soft_main;
        render_soft_main();
    }

    return 0;
}
