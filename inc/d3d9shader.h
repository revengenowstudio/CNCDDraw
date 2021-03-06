#ifndef D3D9SHADER_H
#define D3D9SHADER_H

#if 0
//
// Generated by Microsoft (R) HLSL Shader Compiler 10.1
//
// Parameters:
//
//   sampler2D PaletteTex;
//   sampler2D SurfaceTex;
//
//
// Registers:
//
//   Name         Reg   Size
//   ------------ ----- ----
//   SurfaceTex   s0       1
//   PaletteTex   s1       1
//

    ps_2_0
    def c0, 0.99609375, 0.001953125, 0, 0
    dcl t0.xy
    dcl_2d s0
    dcl_2d s1
    texld r0, t0, s0
    mad r0.x, r0.x, c0.x, c0.y
    mov r0.y, c0.z
    texld r0, r0, s1
    mov oC0, r0

// approximately 5 instruction slots used (2 texture, 3 arithmetic)

// fxc.exe /Tps_2_0 shader.hlsl /Fhshader.h
/*
uniform sampler2D SurfaceTex;
uniform sampler2D PaletteTex;

float4 main(float2 texCoords : TEXCOORD) : COLOR
{
    float pIndex = tex2D(SurfaceTex, texCoords).r;
    return tex2D(PaletteTex, float2(pIndex * (255./256) + (0.5/256), 0));
}
*/
#endif

const BYTE D3D9_PALETTE_SHADER[] =
{
      0,   2, 255, 255, 254, 255, 
     44,   0,  67,  84,  65,  66, 
     28,   0,   0,   0, 131,   0, 
      0,   0,   0,   2, 255, 255, 
      2,   0,   0,   0,  28,   0, 
      0,   0,   0,   1,   0,   0, 
    124,   0,   0,   0,  68,   0, 
      0,   0,   3,   0,   1,   0, 
      1,   0,   0,   0,  80,   0, 
      0,   0,   0,   0,   0,   0, 
     96,   0,   0,   0,   3,   0, 
      0,   0,   1,   0,   0,   0, 
    108,   0,   0,   0,   0,   0, 
      0,   0,  80,  97, 108, 101, 
    116, 116, 101,  84, 101, 120, 
      0, 171,   4,   0,  12,   0, 
      1,   0,   1,   0,   1,   0, 
      0,   0,   0,   0,   0,   0, 
     83, 117, 114, 102,  97,  99, 
    101,  84, 101, 120,   0, 171, 
      4,   0,  12,   0,   1,   0, 
      1,   0,   1,   0,   0,   0, 
      0,   0,   0,   0, 112, 115, 
     95,  50,  95,  48,   0,  77, 
    105,  99, 114, 111, 115, 111, 
    102, 116,  32,  40,  82,  41, 
     32,  72,  76,  83,  76,  32, 
     83, 104,  97, 100, 101, 114, 
     32,  67, 111, 109, 112, 105, 
    108, 101, 114,  32,  49,  48, 
     46,  49,   0, 171,  81,   0, 
      0,   5,   0,   0,  15, 160, 
      0,   0, 127,  63,   0,   0, 
      0,  59,   0,   0,   0,   0, 
      0,   0,   0,   0,  31,   0, 
      0,   2,   0,   0,   0, 128, 
      0,   0,   3, 176,  31,   0, 
      0,   2,   0,   0,   0, 144, 
      0,   8,  15, 160,  31,   0, 
      0,   2,   0,   0,   0, 144, 
      1,   8,  15, 160,  66,   0, 
      0,   3,   0,   0,  15, 128, 
      0,   0, 228, 176,   0,   8, 
    228, 160,   4,   0,   0,   4, 
      0,   0,   1, 128,   0,   0, 
      0, 128,   0,   0,   0, 160, 
      0,   0,  85, 160,   1,   0, 
      0,   2,   0,   0,   2, 128, 
      0,   0, 170, 160,  66,   0, 
      0,   3,   0,   0,  15, 128, 
      0,   0, 228, 128,   1,   8, 
    228, 160,   1,   0,   0,   2, 
      0,   8,  15, 128,   0,   0, 
    228, 128, 255, 255,   0,   0
};


/* bilinear upscaling */

#if 0
//
// Generated by Microsoft (R) HLSL Shader Compiler 10.1
//
// Parameters:
//
//   sampler2D PaletteTex;
//   sampler2D SurfaceTex;
//   float4 TextureSize;
//
//
// Registers:
//
//   Name         Reg   Size
//   ------------ ----- ----
//   TextureSize  c0       1
//   SurfaceTex   s0       1
//   PaletteTex   s1       1
//

    ps_2_0
    def c1, 0.5, 0, 0.99609375, 0.001953125
    dcl t0.xy
    dcl_2d s0
    dcl_2d s1
    mov r0.y, c1.y
    rcp r1.x, c0.x
    rcp r1.y, c0.y
    mul r0.zw, t0.wzyx, c0.wzyx
    frc r0.zw, r0
    add r2.xy, -r0.wzyx, c1.x
    mad r2.xy, r2, r1, t0
    add r3.xy, r1, r2
    mov r1.z, c1.y
    add r4.x, r1.z, r2.x
    add r4.y, r1.y, r2.y
    add r1.x, r1.x, r2.x
    add r1.y, r1.z, r2.y
    texld r3, r3, s0
    texld r2, r2, s0
    texld r1, r1, s0
    texld r4, r4, s0
    mad r0.x, r3.x, c1.z, c1.w
    mov r3.y, c1.y
    mad r2.x, r2.x, c1.z, c1.w
    mad r1.x, r1.x, c1.z, c1.w
    mad r3.x, r4.x, c1.z, c1.w
    mov r1.y, c1.y
    mov r2.y, c1.y
    texld r4, r0, s1
    texld r3, r3, s1
    texld r1, r1, s1
    texld r2, r2, s1
    lrp r5, r0.w, r4, r3
    lrp r3, r0.w, r1, r2
    lrp r1, r0.z, r5, r3
    mov oC0, r1

// approximately 32 instruction slots used (8 texture, 24 arithmetic)

// fxc.exe /Tps_2_0 shader.hlsl /Fhshader.h
/*
uniform sampler2D SurfaceTex;
uniform sampler2D PaletteTex;

float4 TextureSize : register(c0);

float4 bilinear(float2 coord)
{
    float2 size = 1.0 / TextureSize.xy;
    float2 f = frac(coord * TextureSize.xy);
    
    coord += (.5 - f) * size;
    
    float tli = tex2D(SurfaceTex, coord).r;
    float tri = tex2D(SurfaceTex, coord + float2(size.x, 0.0)   ).r;
    float bli = tex2D(SurfaceTex, coord + float2(0.0,    size.y)).r;
    float bri = tex2D(SurfaceTex, coord + float2(size.x, size.y)).r;
    
    float4 tl = tex2D(PaletteTex, float2(tli * (255./256) + (0.5/256), 0));
    float4 tr = tex2D(PaletteTex, float2(tri * (255./256) + (0.5/256), 0));
    float4 bl = tex2D(PaletteTex, float2(bli * (255./256) + (0.5/256), 0));
    float4 br = tex2D(PaletteTex, float2(bri * (255./256) + (0.5/256), 0));

    float4 top = lerp(tl, tr, f.x);
    float4 bot = lerp(bl, br, f.x);
    
    return lerp(top, bot, f.y);
}

float4 main(float2 texCoords : TEXCOORD) : COLOR
{
    return bilinear(texCoords);
}
*/
#endif

const BYTE D3D9_PALETTE_SHADER_BILINEAR[] =
{
      0,   2, 255, 255, 254, 255, 
     56,   0,  67,  84,  65,  66, 
     28,   0,   0,   0, 179,   0, 
      0,   0,   0,   2, 255, 255, 
      3,   0,   0,   0,  28,   0, 
      0,   0,   0,   1,   0,   0, 
    172,   0,   0,   0,  88,   0, 
      0,   0,   3,   0,   1,   0, 
      1,   0,   0,   0, 100,   0, 
      0,   0,   0,   0,   0,   0, 
    116,   0,   0,   0,   3,   0, 
      0,   0,   1,   0,   0,   0, 
    128,   0,   0,   0,   0,   0, 
      0,   0, 144,   0,   0,   0, 
      2,   0,   0,   0,   1,   0, 
      2,   0, 156,   0,   0,   0, 
      0,   0,   0,   0,  80,  97, 
    108, 101, 116, 116, 101,  84, 
    101, 120,   0, 171,   4,   0, 
     12,   0,   1,   0,   1,   0, 
      1,   0,   0,   0,   0,   0, 
      0,   0,  83, 117, 114, 102, 
     97,  99, 101,  84, 101, 120, 
      0, 171,   4,   0,  12,   0, 
      1,   0,   1,   0,   1,   0, 
      0,   0,   0,   0,   0,   0, 
     84, 101, 120, 116, 117, 114, 
    101,  83, 105, 122, 101,   0, 
      1,   0,   3,   0,   1,   0, 
      4,   0,   1,   0,   0,   0, 
      0,   0,   0,   0, 112, 115, 
     95,  50,  95,  48,   0,  77, 
    105,  99, 114, 111, 115, 111, 
    102, 116,  32,  40,  82,  41, 
     32,  72,  76,  83,  76,  32, 
     83, 104,  97, 100, 101, 114, 
     32,  67, 111, 109, 112, 105, 
    108, 101, 114,  32,  49,  48, 
     46,  49,   0, 171,  81,   0, 
      0,   5,   1,   0,  15, 160, 
      0,   0,   0,  63,   0,   0, 
      0,   0,   0,   0, 127,  63, 
      0,   0,   0,  59,  31,   0, 
      0,   2,   0,   0,   0, 128, 
      0,   0,   3, 176,  31,   0, 
      0,   2,   0,   0,   0, 144, 
      0,   8,  15, 160,  31,   0, 
      0,   2,   0,   0,   0, 144, 
      1,   8,  15, 160,   1,   0, 
      0,   2,   0,   0,   2, 128, 
      1,   0,  85, 160,   6,   0, 
      0,   2,   1,   0,   1, 128, 
      0,   0,   0, 160,   6,   0, 
      0,   2,   1,   0,   2, 128, 
      0,   0,  85, 160,   5,   0, 
      0,   3,   0,   0,  12, 128, 
      0,   0,  27, 176,   0,   0, 
     27, 160,  19,   0,   0,   2, 
      0,   0,  12, 128,   0,   0, 
    228, 128,   2,   0,   0,   3, 
      2,   0,   3, 128,   0,   0, 
     27, 129,   1,   0,   0, 160, 
      4,   0,   0,   4,   2,   0, 
      3, 128,   2,   0, 228, 128, 
      1,   0, 228, 128,   0,   0, 
    228, 176,   2,   0,   0,   3, 
      3,   0,   3, 128,   1,   0, 
    228, 128,   2,   0, 228, 128, 
      1,   0,   0,   2,   1,   0, 
      4, 128,   1,   0,  85, 160, 
      2,   0,   0,   3,   4,   0, 
      1, 128,   1,   0, 170, 128, 
      2,   0,   0, 128,   2,   0, 
      0,   3,   4,   0,   2, 128, 
      1,   0,  85, 128,   2,   0, 
     85, 128,   2,   0,   0,   3, 
      1,   0,   1, 128,   1,   0, 
      0, 128,   2,   0,   0, 128, 
      2,   0,   0,   3,   1,   0, 
      2, 128,   1,   0, 170, 128, 
      2,   0,  85, 128,  66,   0, 
      0,   3,   3,   0,  15, 128, 
      3,   0, 228, 128,   0,   8, 
    228, 160,  66,   0,   0,   3, 
      2,   0,  15, 128,   2,   0, 
    228, 128,   0,   8, 228, 160, 
     66,   0,   0,   3,   1,   0, 
     15, 128,   1,   0, 228, 128, 
      0,   8, 228, 160,  66,   0, 
      0,   3,   4,   0,  15, 128, 
      4,   0, 228, 128,   0,   8, 
    228, 160,   4,   0,   0,   4, 
      0,   0,   1, 128,   3,   0, 
      0, 128,   1,   0, 170, 160, 
      1,   0, 255, 160,   1,   0, 
      0,   2,   3,   0,   2, 128, 
      1,   0,  85, 160,   4,   0, 
      0,   4,   2,   0,   1, 128, 
      2,   0,   0, 128,   1,   0, 
    170, 160,   1,   0, 255, 160, 
      4,   0,   0,   4,   1,   0, 
      1, 128,   1,   0,   0, 128, 
      1,   0, 170, 160,   1,   0, 
    255, 160,   4,   0,   0,   4, 
      3,   0,   1, 128,   4,   0, 
      0, 128,   1,   0, 170, 160, 
      1,   0, 255, 160,   1,   0, 
      0,   2,   1,   0,   2, 128, 
      1,   0,  85, 160,   1,   0, 
      0,   2,   2,   0,   2, 128, 
      1,   0,  85, 160,  66,   0, 
      0,   3,   4,   0,  15, 128, 
      0,   0, 228, 128,   1,   8, 
    228, 160,  66,   0,   0,   3, 
      3,   0,  15, 128,   3,   0, 
    228, 128,   1,   8, 228, 160, 
     66,   0,   0,   3,   1,   0, 
     15, 128,   1,   0, 228, 128, 
      1,   8, 228, 160,  66,   0, 
      0,   3,   2,   0,  15, 128, 
      2,   0, 228, 128,   1,   8, 
    228, 160,  18,   0,   0,   4, 
      5,   0,  15, 128,   0,   0, 
    255, 128,   4,   0, 228, 128, 
      3,   0, 228, 128,  18,   0, 
      0,   4,   3,   0,  15, 128, 
      0,   0, 255, 128,   1,   0, 
    228, 128,   2,   0, 228, 128, 
     18,   0,   0,   4,   1,   0, 
     15, 128,   0,   0, 170, 128, 
      5,   0, 228, 128,   3,   0, 
    228, 128,   1,   0,   0,   2, 
      0,   8,  15, 128,   1,   0, 
    228, 128, 255, 255,   0,   0
};

#endif
