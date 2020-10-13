#include <windows.h>
#include <stdio.h>
#include "opengl_utils.h"

PFNWGLCREATECONTEXTPROC xwglCreateContext;
PFNWGLDELETECONTEXTPROC xwglDeleteContext;
PFNWGLGETPROCADDRESSPROC xwglGetProcAddress;
PFNWGLMAKECURRENTPROC xwglMakeCurrent;

PFNGLVIEWPORTPROC glViewport;
PFNGLBINDTEXTUREPROC glBindTexture;
PFNGLGENTEXTURESPROC glGenTextures;
PFNGLTEXPARAMETERIPROC glTexParameteri;
PFNGLDELETETEXTURESPROC glDeleteTextures;
PFNGLTEXIMAGE2DPROC glTexImage2D;
PFNGLDRAWELEMENTSPROC glDrawElements;
PFNGLTEXSUBIMAGE2DPROC glTexSubImage2D;
PFNGLGETERRORPROC glGetError;
PFNGLGETSTRINGPROC glGetString;
PFNGLGETTEXIMAGEPROC glGetTexImage;
PFNGLPIXELSTOREIPROC glPixelStorei;
PFNGLENABLEPROC glEnable;
PFNGLCLEARPROC glClear;
PFNGLBEGINPROC glBegin;
PFNGLENDPROC glEnd;
PFNGLTEXCOORD2FPROC glTexCoord2f;
PFNGLVERTEX2FPROC glVertex2f;
PFNGLCREATEPROGRAMPROC glCreateProgram;
PFNGLDELETEPROGRAMPROC glDeleteProgram;
PFNGLUSEPROGRAMPROC glUseProgram;
PFNGLATTACHSHADERPROC glAttachShader;
PFNGLDETACHSHADERPROC glDetachShader;
PFNGLLINKPROGRAMPROC glLinkProgram;
PFNGLGETPROGRAMIVPROC glGetProgramiv;
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
PFNGLUNIFORM1IPROC glUniform1i;
PFNGLUNIFORM1IVPROC glUniform1iv;
PFNGLUNIFORM2IVPROC glUniform2iv;
PFNGLUNIFORM3IVPROC glUniform3iv;
PFNGLUNIFORM4IVPROC glUniform4iv;
PFNGLUNIFORM1FPROC glUniform1f;
PFNGLUNIFORM1FVPROC glUniform1fv;
PFNGLUNIFORM2FVPROC glUniform2fv;
PFNGLUNIFORM3FVPROC glUniform3fv;
PFNGLUNIFORM4FVPROC glUniform4fv;
PFNGLUNIFORM4FPROC glUniform4f;
PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;
PFNGLGETATTRIBLOCATIONPROC glGetAttribLocation;
PFNGLVERTEXATTRIB1FPROC glVertexAttrib1f;
PFNGLVERTEXATTRIB1FVPROC glVertexAttrib1fv;
PFNGLVERTEXATTRIB2FVPROC glVertexAttrib2fv;
PFNGLVERTEXATTRIB3FVPROC glVertexAttrib3fv;
PFNGLVERTEXATTRIB4FVPROC glVertexAttrib4fv;
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray;
PFNGLBINDATTRIBLOCATIONPROC glBindAttribLocation;
PFNGLGETACTIVEUNIFORMPROC glGetActiveUniform;
PFNGLCREATESHADERPROC glCreateShader;
PFNGLDELETESHADERPROC glDeleteShader;
PFNGLSHADERSOURCEPROC glShaderSource;
PFNGLCOMPILESHADERPROC glCompileShader;
PFNGLGETSHADERIVPROC glGetShaderiv;
PFNGLGENBUFFERSPROC glGenBuffers;
PFNGLBINDBUFFERPROC glBindBuffer;
PFNGLBUFFERDATAPROC glBufferData;
PFNGLMAPBUFFERPROC glMapBuffer;
PFNGLUNMAPBUFFERPROC glUnmapBuffer;
PFNGLBUFFERSUBDATAPROC glBufferSubData;
PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
PFNGLDELETEBUFFERSPROC glDeleteBuffers;
PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays;
PFNGLACTIVETEXTUREPROC glActiveTexture;
PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers;
PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer;
PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D;
PFNGLDRAWBUFFERSPROC glDrawBuffers;
PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus;
PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers;
PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT;
PFNWGLGETEXTENSIONSSTRINGARBPROC wglGetExtensionsStringARB;
PFNGLTEXBUFFERPROC glTexBuffer;

HMODULE g_oglu_hmodule;
BOOL g_oglu_got_version2;
BOOL g_oglu_got_version3;
char g_oglu_version[128];

BOOL oglu_load_dll()
{
    if (!g_oglu_hmodule)
        g_oglu_hmodule = LoadLibrary("opengl32.dll");

    if (g_oglu_hmodule)
    {
        xwglCreateContext = (PFNWGLCREATECONTEXTPROC)GetProcAddress(g_oglu_hmodule, "wglCreateContext");
        xwglDeleteContext = (PFNWGLDELETECONTEXTPROC)GetProcAddress(g_oglu_hmodule, "wglDeleteContext");
        xwglGetProcAddress = (PFNWGLGETPROCADDRESSPROC)GetProcAddress(g_oglu_hmodule, "wglGetProcAddress");
        xwglMakeCurrent = (PFNWGLMAKECURRENTPROC)GetProcAddress(g_oglu_hmodule, "wglMakeCurrent");

        glViewport = (PFNGLVIEWPORTPROC)GetProcAddress(g_oglu_hmodule, "glViewport");
        glBindTexture = (PFNGLBINDTEXTUREPROC)GetProcAddress(g_oglu_hmodule, "glBindTexture");
        glGenTextures = (PFNGLGENTEXTURESPROC)GetProcAddress(g_oglu_hmodule, "glGenTextures");
        glTexParameteri = (PFNGLTEXPARAMETERIPROC)GetProcAddress(g_oglu_hmodule, "glTexParameteri");
        glDeleteTextures = (PFNGLDELETETEXTURESPROC)GetProcAddress(g_oglu_hmodule, "glDeleteTextures");
        glTexImage2D = (PFNGLTEXIMAGE2DPROC)GetProcAddress(g_oglu_hmodule, "glTexImage2D");
        glDrawElements = (PFNGLDRAWELEMENTSPROC)GetProcAddress(g_oglu_hmodule, "glDrawElements");
        glTexSubImage2D = (PFNGLTEXSUBIMAGE2DPROC)GetProcAddress(g_oglu_hmodule, "glTexSubImage2D");
        glGetError = (PFNGLGETERRORPROC)GetProcAddress(g_oglu_hmodule, "glGetError");
        glGetString = (PFNGLGETSTRINGPROC)GetProcAddress(g_oglu_hmodule, "glGetString");
        glGetTexImage = (PFNGLGETTEXIMAGEPROC)GetProcAddress(g_oglu_hmodule, "glGetTexImage");
        glPixelStorei = (PFNGLPIXELSTOREIPROC)GetProcAddress(g_oglu_hmodule, "glPixelStorei");
        glEnable = (PFNGLENABLEPROC)GetProcAddress(g_oglu_hmodule, "glEnable");
        glClear = (PFNGLCLEARPROC)GetProcAddress(g_oglu_hmodule, "glClear");

        glBegin = (PFNGLBEGINPROC)GetProcAddress(g_oglu_hmodule, "glBegin");
        glEnd = (PFNGLENDPROC)GetProcAddress(g_oglu_hmodule, "glEnd");
        glTexCoord2f = (PFNGLTEXCOORD2FPROC)GetProcAddress(g_oglu_hmodule, "glTexCoord2f");
        glVertex2f = (PFNGLVERTEX2FPROC)GetProcAddress(g_oglu_hmodule, "glVertex2f");
    }

    return xwglCreateContext && xwglDeleteContext && xwglGetProcAddress && xwglMakeCurrent && glViewport &&
        glBindTexture && glGenTextures && glTexParameteri && glDeleteTextures && glTexImage2D &&
        glDrawElements && glTexSubImage2D && glGetError && glGetString && glGetTexImage && glPixelStorei &&
        glEnable && glClear && glBegin && glEnd && glTexCoord2f && glVertex2f;
}

void oglu_init()
{
    // Program
    glCreateProgram = (PFNGLCREATEPROGRAMPROC)xwglGetProcAddress("glCreateProgram");
    glDeleteProgram = (PFNGLDELETEPROGRAMPROC)xwglGetProcAddress("glDeleteProgram");
    glUseProgram = (PFNGLUSEPROGRAMPROC)xwglGetProcAddress("glUseProgram");
    glAttachShader = (PFNGLATTACHSHADERPROC)xwglGetProcAddress("glAttachShader");
    glDetachShader = (PFNGLDETACHSHADERPROC)xwglGetProcAddress("glDetachShader");
    glLinkProgram = (PFNGLLINKPROGRAMPROC)xwglGetProcAddress("glLinkProgram");
    glGetProgramiv = (PFNGLGETPROGRAMIVPROC)xwglGetProcAddress("glGetProgramiv");
    glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)xwglGetProcAddress("glGetShaderInfoLog");
    glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)xwglGetProcAddress("glGetUniformLocation");
    glUniform1i = (PFNGLUNIFORM1IPROC)xwglGetProcAddress("glUniform1i");
    glUniform1iv = (PFNGLUNIFORM1IVPROC)xwglGetProcAddress("glUniform1iv");
    glUniform2iv = (PFNGLUNIFORM2IVPROC)xwglGetProcAddress("glUniform2iv");
    glUniform3iv = (PFNGLUNIFORM3IVPROC)xwglGetProcAddress("glUniform3iv");
    glUniform4iv = (PFNGLUNIFORM4IVPROC)xwglGetProcAddress("glUniform4iv");
    glUniform1f = (PFNGLUNIFORM1FPROC)xwglGetProcAddress("glUniform1f");
    glUniform1fv = (PFNGLUNIFORM1FVPROC)xwglGetProcAddress("glUniform1fv");
    glUniform2fv = (PFNGLUNIFORM2FVPROC)xwglGetProcAddress("glUniform2fv");
    glUniform3fv = (PFNGLUNIFORM3FVPROC)xwglGetProcAddress("glUniform3fv");
    glUniform4fv = (PFNGLUNIFORM4FVPROC)xwglGetProcAddress("glUniform4fv");
    glUniform4f = (PFNGLUNIFORM4FPROC)xwglGetProcAddress("glUniform4f");
    glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC)xwglGetProcAddress("glUniformMatrix4fv");
    glGetAttribLocation = (PFNGLGETATTRIBLOCATIONPROC)xwglGetProcAddress("glGetAttribLocation");
    glVertexAttrib1f = (PFNGLVERTEXATTRIB1FPROC)xwglGetProcAddress("glVertexAttrib1f");
    glVertexAttrib1fv = (PFNGLVERTEXATTRIB1FVPROC)xwglGetProcAddress("glVertexAttrib1fv");
    glVertexAttrib2fv = (PFNGLVERTEXATTRIB2FVPROC)xwglGetProcAddress("glVertexAttrib2fv");
    glVertexAttrib3fv = (PFNGLVERTEXATTRIB3FVPROC)xwglGetProcAddress("glVertexAttrib3fv");
    glVertexAttrib4fv = (PFNGLVERTEXATTRIB4FVPROC)xwglGetProcAddress("glVertexAttrib4fv");
    glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)xwglGetProcAddress("glEnableVertexAttribArray");
    glBindAttribLocation = (PFNGLBINDATTRIBLOCATIONPROC)xwglGetProcAddress("glBindAttribLocation");
    glCreateShader = (PFNGLCREATESHADERPROC)xwglGetProcAddress("glCreateShader");
    glDeleteShader = (PFNGLDELETESHADERPROC)xwglGetProcAddress("glDeleteShader");
    glShaderSource = (PFNGLSHADERSOURCEPROC)xwglGetProcAddress("glShaderSource");
    glCompileShader = (PFNGLCOMPILESHADERPROC)xwglGetProcAddress("glCompileShader");
    glGetShaderiv = (PFNGLGETSHADERIVPROC)xwglGetProcAddress("glGetShaderiv");
    glGenBuffers = (PFNGLGENBUFFERSPROC)xwglGetProcAddress("glGenBuffers");
    glBindBuffer = (PFNGLBINDBUFFERPROC)xwglGetProcAddress("glBindBuffer");
    glBufferData = (PFNGLBUFFERDATAPROC)xwglGetProcAddress("glBufferData");
    glMapBuffer = (PFNGLMAPBUFFERPROC)xwglGetProcAddress("glMapBuffer");
    glUnmapBuffer = (PFNGLUNMAPBUFFERPROC)xwglGetProcAddress("glUnmapBuffer");
    glBufferSubData = (PFNGLBUFFERSUBDATAPROC)xwglGetProcAddress("glBufferSubData");
    glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)xwglGetProcAddress("glVertexAttribPointer");
    glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)xwglGetProcAddress("glDeleteBuffers");
    glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC)xwglGetProcAddress("glGenVertexArrays");
    glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)xwglGetProcAddress("glBindVertexArray");
    glDeleteVertexArrays = (PFNGLDELETEVERTEXARRAYSPROC)xwglGetProcAddress("glDeleteVertexArrays");
    glActiveTexture = (PFNGLACTIVETEXTUREPROC)xwglGetProcAddress("glActiveTexture");
    glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC)xwglGetProcAddress("glGenFramebuffers");
    glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC)xwglGetProcAddress("glBindFramebuffer");
    glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC)xwglGetProcAddress("glFramebufferTexture2D");
    glDrawBuffers = (PFNGLDRAWBUFFERSPROC)xwglGetProcAddress("glDrawBuffers");
    glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC)xwglGetProcAddress("glCheckFramebufferStatus");
    glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC)xwglGetProcAddress("glDeleteFramebuffers");

    wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)xwglGetProcAddress("wglSwapIntervalEXT");
    wglGetExtensionsStringARB = (PFNWGLGETEXTENSIONSSTRINGARBPROC)xwglGetProcAddress("wglGetExtensionsStringARB");

    glTexBuffer = (PFNGLTEXBUFFERPROC)xwglGetProcAddress("glTexBuffer");

    char *glversion = (char *)glGetString(GL_VERSION);
    if (glversion)
    {
        strncpy(g_oglu_version, glversion, sizeof(g_oglu_version)-1);
        const char deli[2] = " ";
        strtok(g_oglu_version, deli);
    }
    else
        g_oglu_version[0] = '0';

    g_oglu_got_version2 = glGetUniformLocation && glActiveTexture && glUniform1i;

    g_oglu_got_version3 = glGenFramebuffers && glBindFramebuffer && glFramebufferTexture2D && glDrawBuffers &&
        glCheckFramebufferStatus && glUniform4f && glActiveTexture && glUniform1i &&
        glGetAttribLocation && glGenBuffers && glBindBuffer && glBufferData && glVertexAttribPointer &&
        glEnableVertexAttribArray && glUniform2fv && glUniformMatrix4fv && glGenVertexArrays && glBindVertexArray &&
        glGetUniformLocation && glversion && glversion[0] != '2';
}

BOOL oglu_ext_exists(char *ext, HDC hdc)
{
    char *glext = (char *)glGetString(GL_EXTENSIONS);
    if (glext)
    {
        if (strstr(glext, ext))
            return TRUE;
    }

    if (wglGetExtensionsStringARB)
    {
        char *wglext = (char *)wglGetExtensionsStringARB(hdc);
        if (wglext)
        {
            if (strstr(wglext, ext))
                return TRUE;
        }
    }

    return FALSE;
}

GLuint oglu_build_program(const GLchar *vert_source, const GLchar *frag_source)
{
    if (!glCreateShader || !glShaderSource || !glCompileShader || !glCreateProgram ||
        !glAttachShader || !glLinkProgram || !glUseProgram || !glDetachShader)
        return 0;

    GLuint vert_shader = glCreateShader(GL_VERTEX_SHADER);
    GLuint frag_shader = glCreateShader(GL_FRAGMENT_SHADER);

    if (!vert_shader || !frag_shader)
        return 0;

    glShaderSource(vert_shader, 1, &vert_source, NULL);
    glShaderSource(frag_shader, 1, &frag_source, NULL);

    GLint is_compiled = 0;

    glCompileShader(vert_shader);
    if (glGetShaderiv)
    {
        glGetShaderiv(vert_shader, GL_COMPILE_STATUS, &is_compiled);
        if (is_compiled == GL_FALSE)
        {
            if (glDeleteShader)
                glDeleteShader(vert_shader);

            return 0;
        }
    }

    glCompileShader(frag_shader);
    if (glGetShaderiv)
    {
        glGetShaderiv(frag_shader, GL_COMPILE_STATUS, &is_compiled);
        if (is_compiled == GL_FALSE)
        {
            if (glDeleteShader)
            {
                glDeleteShader(frag_shader);
                glDeleteShader(vert_shader);
            }

            return 0;
        }
    }

    GLuint program = glCreateProgram();
    if (program)
    {
        glAttachShader(program, vert_shader);
        glAttachShader(program, frag_shader);

        glLinkProgram(program);

        glDetachShader(program, vert_shader);
        glDetachShader(program, frag_shader);
        glDeleteShader(vert_shader);
        glDeleteShader(frag_shader);

        if (glGetProgramiv)
        {
            GLint is_linked = 0;
            glGetProgramiv(program, GL_LINK_STATUS, &is_linked);
            if (is_linked == GL_FALSE)
            {
                if (glDeleteProgram)
                    glDeleteProgram(program);

                return 0;
            }
        }
    }

    return program;
}

GLuint oglu_build_program_from_file(const char *file_path)
{
    GLuint program = 0;

    FILE *file = fopen(file_path, "rb");
    if (file)
    {
        fseek(file, 0, SEEK_END);
        long file_size = ftell(file);
        fseek(file, 0, SEEK_SET);

        char *source = file_size > 0 ? calloc(file_size + 1, 1) : NULL;
        if (source)
        {
            fread(source, file_size, 1, file);
            fclose(file);

            char *vert_source = calloc(file_size + 50, 1);
            char *frag_source = calloc(file_size + 50, 1);

            if (frag_source && vert_source)
            {
                char *version_start = strstr(source, "#version");
                if (version_start)
                {
                    const char deli[2] = "\n";
                    char *version = strtok(version_start, deli);

                    strcpy(vert_source, source);
                    strcpy(frag_source, source);
                    strcat(vert_source, "\n#define VERTEX\n");
                    strcat(frag_source, "\n#define FRAGMENT\n");
                    strcat(vert_source, version + strlen(version) + 1);
                    strcat(frag_source, version + strlen(version) + 1);

                    program = oglu_build_program(vert_source, frag_source);
                }
                else
                {
                    strcpy(vert_source, "#define VERTEX\n");
                    strcpy(frag_source, "#define FRAGMENT\n");
                    strcat(vert_source, source);
                    strcat(frag_source, source);

                    program = oglu_build_program(vert_source, frag_source);
                }

                free(vert_source);
                free(frag_source);
            }

            free(source);
        }
    }

    return program;
}
