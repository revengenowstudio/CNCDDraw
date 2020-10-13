-include config.mk

WINDRES  ?= windres
LDFLAGS   = -Iinc -Wall -Wl,--enable-stdcall-fixup -s
CFLAGS    = -std=c99
LIBS      = -lgdi32 -lwinmm

FILES = src/debug.c \
        src/dd.c \
        src/ddpalette.c \
        src/ddsurface.c \
        src/ddclipper.c \
        src/IDirectDraw/IDirectDraw.c \
        src/IDirectDraw/IDirectDrawPalette.c \
        src/IDirectDraw/IDirectDrawClipper.c \
        src/IDirectDraw/IDirectDrawSurface.c \
        src/render_ogl.c \
        src/render_gdi.c \
        src/render_d3d9.c \
        src/IDirect3D/IDirect3D.c \
        src/mouse.c \
        src/winapi_hooks.c \
        src/screenshot.c \
        src/config.c \
        src/lodepng.c \
        src/directinput.c \
        src/hook.c \
        src/dllmain.c \
        src/wndproc.c \
        src/utils.c \
        src/opengl_utils.c

all:
	$(WINDRES) -J rc ddraw.rc ddraw.rc.o
	$(CC) $(CFLAGS) $(LDFLAGS) -shared -o ddraw.dll $(FILES) ddraw.def ddraw.rc.o $(LIBS)
#	$(CC) $(CFLAGS) $(LDFLAGS) -nostdlib -shared -o ddraw.dll $(FILES) ddraw.def ddraw.rc.o $(LIBS) -lkernel32 -luser32 -lmsvcrt

clean:
	$(RM) ddraw.dll ddraw.rc.o
