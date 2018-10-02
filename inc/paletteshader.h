// old

static const char *PassthroughVertShader110Src =
    "#version 110\n"
    "varying vec2 TexCoord0; \n"
    "\n"
    "void main()\n"
    "{\n"
    "    gl_Position = ftransform(); \n"
    "    TexCoord0 = gl_MultiTexCoord0.xy; \n"
    "}\n";

static const char *PaletteFragShader110Src =
    "#version 110\n"
    "uniform sampler2D SurfaceTex; \n"
    "uniform sampler2D PaletteTex; \n"
    "varying vec2 TexCoord0; \n"
    "\n"
    "void main()\n"
    "{\n"
    "   vec4 paletteIndex = texture2D(SurfaceTex, TexCoord0); \n"
    "   vec4 outTexel = texture2D(PaletteTex, vec2(paletteIndex.r, 0)); \n"
    "   gl_FragColor = outTexel;\n"
    "}\n";


// new 

static const char *PassthroughVertShaderSrc =
    "#version 130\n"
    "in vec4 VertexCoord;\n"
    "in vec4 COLOR;\n"
    "in vec4 TexCoord;\n"
    "out vec4 COL0;\n"
    "out vec4 TEX0;\n"
    "uniform mat4 MVPMatrix;\n"
    "\n"
    "void main()\n"
    "{\n"
    "    gl_Position = MVPMatrix * VertexCoord;\n"
    "    COL0 = COLOR;\n"
    "    TEX0.xy = TexCoord.xy;\n"
    "}\n";


static const char *PaletteFragShaderSrc =
    "#version 130\n"
    "out vec4 FragColor;\n"
    "uniform sampler2D SurfaceTex;\n"
    "uniform sampler2D PaletteTex;\n"
    "in vec4 TEX0;\n"
    "\n"
    "void main()\n"
    "{\n"
    "    vec4 paletteIndex = texture(SurfaceTex, TEX0.xy);\n"
    "    vec4 outTexel = texture(PaletteTex, vec2(paletteIndex.r, 0));\n"
    "    FragColor = outTexel;\n"
    "}\n";


static const BYTE PalettePixelShaderSrc[] =
{
    0,2,255,255,254,255,42,0,67,84,65,66,28,0,0,0,115,0,0,0,0,2,255,255,
    2,0,0,0,28,0,0,0,0,1,0,0,108,0,0,0,68,0,0,0,3,0,0,0,
    1,0,2,0,72,0,0,0,0,0,0,0,88,0,0,0,3,0,1,0,1,0,6,0,
    92,0,0,0,0,0,0,0,115,48,0,171,4,0,12,0,1,0,1,0,1,0,0,0,
    0,0,0,0,115,49,0,171,4,0,12,0,1,0,1,0,1,0,0,0,0,0,0,0,
    112,115,95,50,95,48,0,77,105,99,114,111,115,111,102,116,32,40,82,41,32,72,76,83,
    76,32,83,104,97,100,101,114,32,67,111,109,112,105,108,101,114,32,57,46,50,57,46,57,
    53,50,46,51,49,49,49,0,81,0,0,5,0,0,15,160,0,0,127,63,0,0,0,59,
    0,0,0,0,0,0,0,0,31,0,0,2,0,0,0,128,0,0,3,176,31,0,0,2,
    0,0,0,144,0,8,15,160,31,0,0,2,0,0,0,144,1,8,15,160,66,0,0,3,
    0,0,15,128,0,0,228,176,0,8,228,160,4,0,0,4,0,0,1,128,0,0,0,128,
    0,0,0,160,0,0,85,160,1,0,0,2,0,0,2,128,0,0,170,160,66,0,0,3,
    0,0,15,128,0,0,228,128,1,8,228,160,1,0,0,2,0,8,15,128,0,0,228,128,255,255,0,0
};
