CC=gcc
WINDRES=windres
CFLAGS=-Iinc -Wall -Wl,--enable-stdcall-fixup -s
LIBS=-lgdi32 -lwinmm

FILES = src/debug.c \
        src/main.c \
        src/mouse.c \
        src/palette.c \
        src/surface.c \
        src/clipper.c \
        src/render.c \
        src/render_soft.c \
        src/render_d3d9.c \
        src/screenshot.c \
        src/settings.c \
        src/lodepng.c \
        src/opengl.c

all:
	$(WINDRES) -J rc ddraw.rc ddraw.rc.o
	$(CC) $(CFLAGS) -shared -o ddraw.dll $(FILES) ddraw.def ddraw.rc.o $(LIBS)
#	$(CC) $(CFLAGS) -nostdlib -shared -o ddraw.dll $(FILES) ddraw.def ddraw.rc.o $(LIBS) -lkernel32 -luser32 -lmsvcrt

clean:
	rm -f ddraw.dll
