CC=gcc
WINDRES=windres
CFLAGS=-DHAVE_LIBPNG -Iinc -Wall -Wl,--enable-stdcall-fixup -O3 -s
LIBS=lib/libpng14.a lib/libz.a -lgdi32 -lopengl32 -lwinmm

FILES = src/main.c \
        src/mouse.c \
        src/palette.c \
        src/surface.c \
        src/clipper.c \
        src/render.c \
        src/render_soft.c \
        src/render_dummy.c \
        src/screenshot.c

all:
	$(WINDRES) -J rc ddraw.rc ddraw.rc.o
	$(CC) $(CFLAGS) -shared -o ddraw.dll $(FILES) ddraw.def ddraw.rc.o $(LIBS)

clean:
	rm -f ddraw.dll
