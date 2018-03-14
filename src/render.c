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

#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>

#include "main.h"
#include "surface.h"

#define CUTSCENE_WIDTH 640
#define CUTSCENE_HEIGHT 400

PFNGLGENBUFFERSARBPROC pglGenBuffersARB = 0;                     // VBO Name Generation Procedure
PFNGLBINDBUFFERARBPROC pglBindBufferARB = 0;                     // VBO Bind Procedure
PFNGLBUFFERDATAARBPROC pglBufferDataARB = 0;                     // VBO Data Loading Procedure
PFNGLDELETEBUFFERSARBPROC pglDeleteBuffersARB = 0;               // VBO Deletion Procedure
PFNGLMAPBUFFERARBPROC pglMapBufferARB = 0;                       // map VBO procedure
PFNGLUNMAPBUFFERARBPROC pglUnmapBufferARB = 0;                   // unmap VBO procedure
#define glGenBuffersARB           pglGenBuffersARB
#define glBindBufferARB           pglBindBufferARB
#define glBufferDataARB           pglBufferDataARB
#define glDeleteBuffersARB        pglDeleteBuffersARB
#define glMapBufferARB            pglMapBufferARB
#define glUnmapBufferARB          pglUnmapBufferARB

const GLenum PIXEL_FORMAT = GL_RGBA;

GLuint pboIds[2];
GLuint textureId;
BOOL pboSupported = FALSE;

BOOL detect_cutscene();

DWORD WINAPI render_main(void)
{
    int i,j;
    HGLRC hRC;

    int tex_width = ddraw->width > 1024 ? ddraw->width : 1024;
    int tex_height = ddraw->height > 1024 ? ddraw->height : 1024;
    int tex_size = tex_width * tex_height * sizeof(int);
    float scale_w = 1.0f;
    float scale_h = 1.0f;
    int *tex = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, tex_size);

    hRC = wglCreateContext( ddraw->render.hDC );
    wglMakeCurrent( ddraw->render.hDC, hRC );

    char *glext = (char *)glGetString(GL_EXTENSIONS);

    if(glext)
    {
        if (strstr(glext, "WGL_EXT_swap_control"))
        {
            BOOL (APIENTRY *wglSwapIntervalEXT)(int) = (BOOL (APIENTRY *)(int))wglGetProcAddress("wglSwapIntervalEXT");
            if(wglSwapIntervalEXT)
            {
                if(ddraw->vsync)
                {
                    wglSwapIntervalEXT(1);
                }
                else
                {
                    wglSwapIntervalEXT(0);
                }
            }
        }
        if (ddraw->opengl_pbo && strstr(glext, "GL_ARB_pixel_buffer_object"))
        {
            glGenBuffersARB = (PFNGLGENBUFFERSARBPROC)wglGetProcAddress("glGenBuffersARB");
            glBindBufferARB = (PFNGLBINDBUFFERARBPROC)wglGetProcAddress("glBindBufferARB");
            glBufferDataARB = (PFNGLBUFFERDATAARBPROC)wglGetProcAddress("glBufferDataARB");
            glDeleteBuffersARB = (PFNGLDELETEBUFFERSARBPROC)wglGetProcAddress("glDeleteBuffersARB");
            glMapBufferARB = (PFNGLMAPBUFFERARBPROC)wglGetProcAddress("glMapBufferARB");
            glUnmapBufferARB = (PFNGLUNMAPBUFFERARBPROC)wglGetProcAddress("glUnmapBufferARB");

            if(glGenBuffersARB && glBindBufferARB && glBufferDataARB && glMapBufferARB && glUnmapBufferARB && glDeleteBuffersARB)
            {
                pboSupported = TRUE;
            }
        }
    }

    DWORD tick_start = 0;
    DWORD tick_end = 0;
    DWORD frame_len = 0;

    if(ddraw->render.maxfps < 0)
    {
        ddraw->render.maxfps = ddraw->mode.dmDisplayFrequency;
    }

    if(ddraw->render.maxfps > 0)
    {
        frame_len = 1000.0f / ddraw->render.maxfps;
    }

    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tex_width, tex_height, 0, PIXEL_FORMAT, GL_UNSIGNED_BYTE, tex);

    DWORD dst_top = 0;
    DWORD dst_left = 0;
    DWORD dst_width = ddraw->render.width;
    DWORD dst_height = ddraw->render.height;
    
    if (ddraw->boxing)
    {
        dst_width = ddraw->width;
        dst_height = ddraw->height;

        int i;
        for (i = 20; i-- > 1;)
        {
            if (ddraw->width * i <= ddraw->render.width && ddraw->height * i <= ddraw->render.height)
            {
                dst_width *= i;
                dst_height *= i;
                break;
            }
        }

        dst_top = ddraw->render.height / 2 - dst_height / 2;
        dst_left = ddraw->render.width / 2 - dst_width / 2;
    }
    else if (ddraw->maintas)
    {
        dst_width = ddraw->render.width;
        dst_height = ((float)ddraw->height / ddraw->width) * dst_width;
        
        if (dst_height > ddraw->render.height)
        {
            dst_width = ((float)dst_width / dst_height) * ddraw->render.height;
            dst_height = ddraw->render.height;
        }
         
        dst_top = ddraw->render.height / 2 - dst_height / 2;
        dst_left = ddraw->render.width / 2 - dst_width / 2;
    }

    glViewport(dst_left, dst_top, dst_width, dst_height);
    
    if(ddraw->render.filter)
    {
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    }
    else
    {
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    }
    
    glBindTexture(GL_TEXTURE_2D, 0);
    
    if(pboSupported)
    {
        glGenBuffersARB(2, pboIds);
        glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, pboIds[0]);
        glBufferDataARB(GL_PIXEL_UNPACK_BUFFER_ARB, tex_size, 0, GL_STREAM_DRAW_ARB);
        glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, pboIds[1]);
        glBufferDataARB(GL_PIXEL_UNPACK_BUFFER_ARB, tex_size, 0, GL_STREAM_DRAW_ARB);
        glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
    }
    
    glEnable(GL_TEXTURE_2D);
    
    
    while(ddraw->render.run && WaitForSingleObject(ddraw->render.sem, INFINITE) != WAIT_FAILED)
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
        
        static int index = 0;
        scale_w = (float)ddraw->width/tex_width;
        scale_h = (float)ddraw->height/tex_height;
        
        index = (index + 1) % 2;
        int nextIndex = (index + 1) % 2;
        
        if(ddraw->render.maxfps > 0)
        {
            tick_start = timeGetTime();
        }

        /* convert ddraw surface to opengl texture */
        
        if(pboSupported)
        {
            glBindTexture(GL_TEXTURE_2D, textureId);
            glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, pboIds[index]);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, ddraw->width, ddraw->height, PIXEL_FORMAT, GL_UNSIGNED_BYTE, 0);
        }
        
        EnterCriticalSection(&ddraw->cs);

        if(ddraw->primary && ddraw->primary->palette)
        {
            if(ddraw->vhack && detect_cutscene())
            {
                scale_w *= (float)CUTSCENE_WIDTH / ddraw->width;
                scale_h *= (float)CUTSCENE_HEIGHT / ddraw->height;

                if (ddraw->cursorclip.width != CUTSCENE_WIDTH || ddraw->cursorclip.height != CUTSCENE_HEIGHT)
                {
                    ddraw->cursorclip.width = CUTSCENE_WIDTH;
                    ddraw->cursorclip.height = CUTSCENE_HEIGHT;
                    ddraw->cursor.x = CUTSCENE_WIDTH / 2;
                    ddraw->cursor.y = CUTSCENE_HEIGHT / 2;
                }
            }
            else
            {
                if (ddraw->cursorclip.width != ddraw->width || ddraw->cursorclip.height != ddraw->height)
                {
                    ddraw->cursorclip.width = ddraw->width;
                    ddraw->cursorclip.height = ddraw->height;
                    ddraw->cursor.x = ddraw->width / 2;
                    ddraw->cursor.y = ddraw->height / 2;
                }
            }

            if(pboSupported)
            {
                glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, pboIds[nextIndex]);
                
                glBufferDataARB(GL_PIXEL_UNPACK_BUFFER_ARB, tex_size, 0, GL_STREAM_DRAW_ARB);
                int *ptr = (int *)glMapBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, GL_WRITE_ONLY_ARB);
                if(ptr)
                {
                    for(i=0; i<ddraw->height; i++)
                    {
                        for(j=0; j<ddraw->width; j++)
                        {
                            ptr[i*ddraw->width+j] = ddraw->primary->palette->data_bgr[((unsigned char *)ddraw->primary->surface)[i*ddraw->primary->lPitch + j*ddraw->primary->lXPitch]];
                        }
                    }
                    
                    glUnmapBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB);
                }
                
                glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
            }
            else
            {
                for(i=0; i<ddraw->height; i++)
                {
                    for(j=0; j<ddraw->width; j++)
                    {
                        tex[i*ddraw->width+j] = ddraw->primary->palette->data_bgr[((unsigned char *)ddraw->primary->surface)[i*ddraw->primary->lPitch + j*ddraw->primary->lXPitch]];
                    }
                }
            }
        }
        
        LeaveCriticalSection(&ddraw->cs);

        if (!pboSupported)
        {
            glBindTexture(GL_TEXTURE_2D, textureId);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, ddraw->width, ddraw->height, PIXEL_FORMAT, GL_UNSIGNED_BYTE, tex);
        }

        glBegin(GL_TRIANGLE_FAN);
        glTexCoord2f(0,0);              glVertex2f(-1,  1);
        glTexCoord2f(scale_w,0);        glVertex2f( 1,  1);
        glTexCoord2f(scale_w,scale_h);  glVertex2f( 1, -1);	
        glTexCoord2f(0,scale_h);        glVertex2f(-1, -1);
        glEnd();
        
        glBindTexture(GL_TEXTURE_2D, 0);
        
        SwapBuffers(ddraw->render.hDC);

        if(ddraw->render.maxfps > 0)
        {        
            tick_end = timeGetTime();
            
            if(tick_end - tick_start < frame_len)
            {
                Sleep( frame_len - (tick_end - tick_start));
            }
        }
        
        SetEvent(ddraw->render.ev);
    }

    HeapFree(GetProcessHeap(), 0, tex);
    glDeleteTextures(1, &textureId);

    if(pboSupported)
        glDeleteBuffersARB(2, pboIds);

    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hRC);

    return 0;
}
