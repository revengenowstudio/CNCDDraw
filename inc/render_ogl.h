#ifndef RENDER_OGL_H
#define RENDER_OGL_H

#include <windows.h>
#include "opengl_utils.h"

#define TEXTURE_COUNT 4

typedef struct OGLRENDERER
{
    HGLRC context;
    GLuint main_program;
    GLuint scale_program;
    BOOL got_error;
    int surface_tex_width;
    int surface_tex_height;
    int* surface_tex;
    GLenum surface_format;
    GLenum surface_type;
    GLuint surface_tex_ids[TEXTURE_COUNT];
    GLuint palette_tex_ids[TEXTURE_COUNT];
    float scale_w;
    float scale_h;
    GLint main_tex_coord_attr_loc;
    GLint main_vertex_coord_attr_loc;
    GLuint main_vbos[3];
    GLuint main_vao;
    GLint frame_count_uni_loc;
    GLuint frame_buffer_id;
    GLuint frame_buffer_tex_id;
    GLint scale_tex_coord_attr_loc;
    GLuint scale_vbos[3];
    GLuint scale_vao;
    BOOL use_opengl;
    BOOL adjust_alignment;
    BOOL filter_bilinear;
} OGLRENDERER;

DWORD WINAPI ogl_render_main(void);

#endif
