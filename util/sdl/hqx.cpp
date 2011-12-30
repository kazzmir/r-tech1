#ifdef USE_SDL

/* by byuu
 * originally from http://nesdev.parodius.com/bbs/viewtopic.php?p=82770#82770
 * http://pastebin.com/YXpmqvW5
 */

#include <stdint.h>
#include "util/bitmap.h"
#include <SDL/SDL.h>

static void rgb555_to_rgb888(uint8_t red, uint8_t green, uint8_t blue,
                             uint8_t & red_output, uint8_t & green_output, uint8_t & blue_output){
    red_output = (red << 3) | (red >> 2);
    green_output = (green << 3) | (green >> 2);
    blue_output = (blue << 3) | (blue >> 2);
}

static void rgb565_to_rgb888(uint8_t red, uint8_t green, uint8_t blue,
                             uint8_t & red_output, uint8_t & green_output, uint8_t & blue_output){
    red_output = (red << 3) | (red >> 2);
    green_output = (green << 2) | (green >> 4);
    blue_output = (blue << 3) | (blue >> 2);
}

static void rgb888_to_rgb565(uint8_t red, uint8_t green, uint8_t blue,
                             uint8_t & red_output, uint8_t & green_output, uint8_t & blue_output){
    red_output = red >> 3;
    green_output = green >> 2;
    blue_output = blue >> 3;
}

static void rgb888_to_yuv888(uint8_t red, uint8_t green, uint8_t blue,
                             double & y, double & u, double & v){
    y = (red + green + blue) * (0.25f * (63.5f / 48.0f));
    u = ((red - blue) * 0.25f + 128.0f) * (7.5f / 7.0f);
    v = ((green * 2.0f - red - blue) * 0.125f + 128.0f) * (7.5f / 6.0f);
}

static void yuv888_to_rgb888(double y, double u, double v,
                             uint8_t & red, uint8_t & green, uint8_t & blue){
    blue = 1.164 * (y - 16) + 2.018 * (u - 128);
    green = 1.164 * (y - 16) - 0.813 * (v - 128) - 0.391 * (u - 128);
    red = 1.164 * (y - 16) + 1.596 * (v - 128);
}

/* 0x03e07c1f: 11111000000111110000011111 */
/* grow/pack are used to do psuedo-simd operations on pixels,
 * operations on all the components can be done in parallel because they are separated
 * by enough bits.
 * Grow duplicates values, pack unduplicates them.
 *
 */

/* 0x03e07e1f: 011111000000111111000011111 */
/* 0x03e07c1f: 011111000000111110000011111 */
/* 0x03e0fc1f: 011111000001111110000011111 */
/* 0x07E0F81F: 111111000001111100000011111 */

#define RGB32_555 0x3e07c1f
// #define RGB32_565 0x3e0fc1f 
#define RGB32_565 0x7E0F81F
static void grow(uint32_t &n) { n |= n << 16; n &= RGB32_565; }
static uint16_t pack(uint32_t n) { n &= RGB32_565; return n | (n >> 16); }

namespace hq2x{

enum {
    /* 440: 10001000000
     * 207: 01000000111
     * 407: 10000000111
     */
    diff_offset = (0x440 << 21) + (0x207 << 11) + 0x407,
    /* 380: 0b1110000000
     * 1f0: 0b0111110000
     * 3f0: 0b1111110000
     */
    diff_mask   = (0x380 << 21) + (0x1f0 << 11) + 0x3f0,
};

uint32_t yuvTable[65536];
// uint16_t rgb565Table[65536];
uint8_t rotate[256];

/* FIXME: replace these with readable constant names */
const uint8_t hqTable[256] = {
  4, 4, 6,  2, 4, 4, 6,  2, 5,  3, 15, 12, 5,  3, 17, 13,
  4, 4, 6, 18, 4, 4, 6, 18, 5,  3, 12, 12, 5,  3,  1, 12,
  4, 4, 6,  2, 4, 4, 6,  2, 5,  3, 17, 13, 5,  3, 16, 14,
  4, 4, 6, 18, 4, 4, 6, 18, 5,  3, 16, 12, 5,  3,  1, 14,
  4, 4, 6,  2, 4, 4, 6,  2, 5, 19, 12, 12, 5, 19, 16, 12,
  4, 4, 6,  2, 4, 4, 6,  2, 5,  3, 16, 12, 5,  3, 16, 12,
  4, 4, 6,  2, 4, 4, 6,  2, 5, 19,  1, 12, 5, 19,  1, 14,
  4, 4, 6,  2, 4, 4, 6, 18, 5,  3, 16, 12, 5, 19,  1, 14,
  4, 4, 6,  2, 4, 4, 6,  2, 5,  3, 15, 12, 5,  3, 17, 13,
  4, 4, 6,  2, 4, 4, 6,  2, 5,  3, 16, 12, 5,  3, 16, 12,
  4, 4, 6,  2, 4, 4, 6,  2, 5,  3, 17, 13, 5,  3, 16, 14,
  4, 4, 6,  2, 4, 4, 6,  2, 5,  3, 16, 13, 5,  3,  1, 14,
  4, 4, 6,  2, 4, 4, 6,  2, 5,  3, 16, 12, 5,  3, 16, 13,
  4, 4, 6,  2, 4, 4, 6,  2, 5,  3, 16, 12, 5,  3,  1, 12,
  4, 4, 6,  2, 4, 4, 6,  2, 5,  3, 16, 12, 5,  3,  1, 14,
  4, 4, 6,  2, 4, 4, 6,  2, 5,  3,  1, 12, 5,  3,  1, 14,
};

static void initialize() {
    static bool initialized = false;
    if (initialized == true){
        return;
    }
    initialized = true;

    for (unsigned int i = 0; i < 65536; i++) {
        uint8_t B = (i >>  0) & 31;
        uint8_t G = (i >>  5) & 63;
        uint8_t R = (i >> 11) & 31;

        uint8_t r, g, b;
        rgb565_to_rgb888(R, G, B, r, g, b);

        double y, u, v;
        rgb888_to_yuv888(r, g, b, y, u, v);

        // rgb565Table[i] = Graphics::makeColor(r, g, b);

        /* Pack the YUV parameters into a single int */
        yuvTable[i] = ((unsigned)y << 21) + ((unsigned)u << 11) + ((unsigned)v);
    }

    for (unsigned int n = 0; n < 256; n++){
        /* What does this do? */
        rotate[n] = ((n >> 2) & 0x11) | ((n << 2) & 0x88) |
                    ((n & 0x01) << 5) | ((n & 0x08) << 3) |
                    ((n & 0x10) >> 3) | ((n & 0x80) >> 5);
    }
}

static bool same(uint16_t x, uint16_t y) {
  return !((yuvTable[x] - yuvTable[y] + diff_offset) & diff_mask);
}

static bool diff(uint32_t x, uint16_t y) {
  return ((x - yuvTable[y]) & diff_mask);
}

static uint16_t blend1(uint32_t A, uint32_t B) {
  grow(A); grow(B);
  A = (A * 3 + B) >> 2;
  return pack(A);
}

static uint16_t blend2(uint32_t A, uint32_t B, uint32_t C) {
  grow(A); grow(B); grow(C);
  return pack((A * 2 + B + C) >> 2);
}

static uint16_t blend3(uint32_t A, uint32_t B, uint32_t C) {
  grow(A); grow(B); grow(C);
  return pack((A * 5 + B * 2 + C) >> 3);
}

static uint16_t blend4(uint32_t A, uint32_t B, uint32_t C) {
  grow(A); grow(B); grow(C);
  return pack((A * 6 + B + C) >> 3);
}

static uint16_t blend5(uint32_t A, uint32_t B, uint32_t C) {
  grow(A); grow(B); grow(C);
  return pack((A * 2 + (B + C) * 3) >> 3);
}

static uint16_t blend6(uint32_t A, uint32_t B, uint32_t C) {
  grow(A); grow(B); grow(C);
  return pack((A * 14 + B + C) >> 4);
}

static uint16_t blend(unsigned rule, uint16_t E, uint16_t A, uint16_t B, uint16_t D, uint16_t F, uint16_t H) {
    switch(rule) { default:
        case  0: return E;
        case  1: return blend1(E, A);
        case  2: return blend1(E, D);
        case  3: return blend1(E, B);
        case  4: return blend2(E, D, B);
        case  5: return blend2(E, A, B);
        case  6: return blend2(E, A, D);
        case  7: return blend3(E, B, D);
        case  8: return blend3(E, D, B);
        case  9: return blend4(E, D, B);
        case 10: return blend5(E, D, B);
        case 11: return blend6(E, D, B);
        case 12: return same(B, D) ? blend2(E, D, B) : E;
        case 13: return same(B, D) ? blend5(E, D, B) : E;
        case 14: return same(B, D) ? blend6(E, D, B) : E;
        case 15: return same(B, D) ? blend2(E, D, B) : blend1(E, A);
        case 16: return same(B, D) ? blend4(E, D, B) : blend1(E, A);
        case 17: return same(B, D) ? blend5(E, D, B) : blend1(E, A);
        case 18: return same(B, F) ? blend3(E, B, D) : blend1(E, D);
        case 19: return same(D, H) ? blend3(E, D, B) : blend1(E, B);
    }
}

/* why is this here */
void filter_size(unsigned &width, unsigned &height) {
    initialize();
    width  *= 2;
    height *= 2;
}

void hq2x(SDL_Surface * input, SDL_Surface * output){
    initialize();
    /* pitch is adjusted by the bit depth (2 bytes per pixel) so we
     * divide by two since we are using int16_t
     */
    int pitch = input->pitch / 2;
    int outpitch = output->pitch / 2;

    //#pragma omp parallel for
    for (unsigned int y = 0; y < input->h; y++){
        const uint16_t * in = (uint16_t*) input->pixels + y * pitch;
        uint16_t *out0 = (uint16_t*) output->pixels + y * outpitch * 2;
        uint16_t *out1 = (uint16_t*) output->pixels + y * outpitch * 2 + outpitch;

        int prevline = (y == 0 ? 0 : pitch);
        int nextline = (y == input->h - 1 ? 0 : pitch);

        // *out0 = Graphics::makeColor(255, 255, 255);
        *out0++ = *in; *out0++ = *in;
        *out1++ = *in; *out1++ = *in;
        in++;

        for (unsigned int x = 1; x < input->w - 1; x++){
            uint16_t A = *(in - prevline - 1);
            uint16_t B = *(in - prevline + 0);
            uint16_t C = *(in - prevline + 1);
            uint16_t D = *(in - 1);
            uint16_t E = *(in + 0);
            uint16_t F = *(in + 1);
            uint16_t G = *(in + nextline - 1);
            uint16_t H = *(in + nextline + 0);
            uint16_t I = *(in + nextline + 1);
            uint32_t e = yuvTable[E] + diff_offset;

            uint8_t pattern;
            pattern  = diff(e, A) << 0;
            pattern |= diff(e, B) << 1;
            pattern |= diff(e, C) << 2;
            pattern |= diff(e, D) << 3;
            pattern |= diff(e, F) << 4;
            pattern |= diff(e, G) << 5;
            pattern |= diff(e, H) << 6;
            pattern |= diff(e, I) << 7;

            /* upper left */
            *(out0 + 0) = blend(hqTable[pattern], E, A, B, D, F, H); pattern = rotate[pattern];
            /* upper right */
            *(out0 + 1) = blend(hqTable[pattern], E, C, F, B, H, D); pattern = rotate[pattern];
            /* lower right */
            *(out1 + 1) = blend(hqTable[pattern], E, I, H, F, D, B); pattern = rotate[pattern];
            /* lower left */
            *(out1 + 0) = blend(hqTable[pattern], E, G, D, H, B, F);

            in++;
            out0 += 2;
            out1 += 2;
        }

        *out0++ = *in; *out0++ = *in;
        *out1++ = *in; *out1++ = *in;
        in++;
    }
}

}

namespace hqx{

//hq4x filter demo program
//----------------------------------------------------------
//Copyright (C) 2003 MaxSt ( maxst@hiend3d.com )

//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU Lesser General Public
//License as published by the Free Software Foundation; either
//version 2.1 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
//Lesser General Public License for more details.
//
//You should have received a copy of the GNU Lesser General Public
//License along with this program; if not, write to the Free Software
//Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

// static int   LUT16to32[65536];
static int   RGBtoYUV[65536];
static int   YUV1, YUV2;
static const  int   Ymask = 0x00FF0000;
static const  int   Umask = 0x0000FF00;
static const  int   Vmask = 0x000000FF;
static const  int   trY   = 0x00300000;
static const  int   trU   = 0x00000700;
static const  int   trV   = 0x00000006;

/* 1111100000000000 */
static const int redMask = 0xf800;
/* 0000011111100000 */
static const int greenMask = 0x7e0;
/* 0000000000011111 */
static const int blueMask = 0x1f;

static const int not_greenMask = (redMask | blueMask);

static const int partial_red_green_mask = 0x1f80;
static const int partial_red_blue_mask = 0xe07c;

#define PIXEL_TYPE uint16_t

/*
inline void Interp1(unsigned char * pc, int c1, int c2){
  *((PIXEL_TYPE*)pc) = (c1*3+c2) >> 2;
}

inline void Interp2(unsigned char * pc, int c1, int c2, int c3){
  *((PIXEL_TYPE*)pc) = (c1*2+c2+c3) >> 2;
}

inline void Interp3(unsigned char * pc, int c1, int c2){
  *((PIXEL_TYPE*)pc) = ((((c1 & greenMask)*7 + (c2 & greenMask) ) & partial_red_green_mask) +
                 (((c1 & not_greenMask)*7 + (c2 & not_greenMask) ) & partial_red_blue_mask)) >> 3;
}

inline void Interp5(unsigned char * pc, int c1, int c2){
  *((PIXEL_TYPE*)pc) = (c1+c2) >> 1;
}

inline void Interp6(unsigned char * pc, int c1, int c2, int c3){
  *((PIXEL_TYPE*)pc) = ((((c1 & greenMask)*5 + (c2 & greenMask)*2 + (c3 & greenMask) ) & partial_red_green_mask) +
                 (((c1 & not_greenMask)*5 + (c2 & not_greenMask)*2 + (c3 & not_greenMask) ) & partial_red_blue_mask)) >> 3;
}

inline void Interp7(unsigned char * pc, int c1, int c2, int c3){
  *((PIXEL_TYPE*)pc) = ((((c1 & greenMask)*6 + (c2 & greenMask) + (c3 & greenMask) ) & partial_red_green_mask) +
                 (((c1 & not_greenMask)*6 + (c2 & not_greenMask) + (c3 & not_greenMask) ) & partial_red_blue_mask)) >> 3;
}

inline void Interp8(unsigned char * pc, int c1, int c2){
  *((PIXEL_TYPE*)pc) = ((((c1 & greenMask)*5 + (c2 & greenMask)*3 ) & partial_red_green_mask) +
                 (((c1 & not_greenMask)*5 + (c2 & not_greenMask)*3 ) & partial_red_blue_mask)) >> 3;
}
*/

inline void Interp1(unsigned char * pc, uint32_t c1, uint32_t c2){
    grow(c1); grow(c2);
    *((PIXEL_TYPE*)pc) = pack((c1*3+c2) >> 2);
}

inline void Interp2(unsigned char * pc, uint32_t c1, uint32_t c2, uint32_t c3){
    grow(c1); grow(c2); grow(c3);
    *((PIXEL_TYPE*)pc) = pack((c1*2+c2+c3) >> 2);
}

inline void Interp3(unsigned char * pc, uint32_t c1, uint32_t c2){
    grow(c1); grow(c2);
    *((PIXEL_TYPE*)pc) = pack((c1*7+c2)/8);

    /*
  *((PIXEL_TYPE*)pc) = ((((c1 & 0x00FF00)*7 + (c2 & 0x00FF00) ) & 0x0007F800) +
                 (((c1 & 0xFF00FF)*7 + (c2 & 0xFF00FF) ) & 0x07F807F8)) >> 3;
                 */
}

inline void Interp4(unsigned char * pc, uint32_t c1, uint32_t c2, uint32_t c3){
    grow(c1); grow(c2); grow(c3);
  *((PIXEL_TYPE*)pc) = pack((c1*2+(c2+c3)*7)/16);

  /*
  *((int*)pc) = ((((c1 & 0x00FF00)*2 + ((c2 & 0x00FF00) + (c3 & 0x00FF00))*7 ) & 0x000FF000) +
                 (((c1 & 0xFF00FF)*2 + ((c2 & 0xFF00FF) + (c3 & 0xFF00FF))*7 ) & 0x0FF00FF0)) >> 4;
                 */
}


inline void Interp5(unsigned char * pc, uint32_t c1, uint32_t c2){
    grow(c1); grow(c2);
    *((PIXEL_TYPE*)pc) = pack((c1+c2) >> 1);
}

inline void Interp6(unsigned char * pc, uint32_t c1, uint32_t c2, uint32_t c3){
    grow(c1); grow(c2); grow(c3);
    *((PIXEL_TYPE*)pc) = pack((c1*5+c2*2+c3)/8);

  /*
  *((PIXEL_TYPE*)pc) = ((((c1 & 0x00FF00)*5 + (c2 & 0x00FF00)*2 + (c3 & 0x00FF00) ) & 0x0007F800) +
                 (((c1 & 0xFF00FF)*5 + (c2 & 0xFF00FF)*2 + (c3 & 0xFF00FF) ) & 0x07F807F8)) >> 3;
                 */
}

inline void Interp7(unsigned char * pc, uint32_t c1, uint32_t c2, uint32_t c3){
    grow(c1); grow(c2); grow(c3);
    *((PIXEL_TYPE*)pc) = pack((c1*6+c2+c3)/8);

  /*
  *((PIXEL_TYPE*)pc) = ((((c1 & 0x00FF00)*6 + (c2 & 0x00FF00) + (c3 & 0x00FF00) ) & 0x0007F800) +
                 (((c1 & 0xFF00FF)*6 + (c2 & 0xFF00FF) + (c3 & 0xFF00FF) ) & 0x07F807F8)) >> 3;
                 */
}

inline void Interp8(unsigned char * pc, uint32_t c1, uint32_t c2){
    grow(c1); grow(c2);
    *((PIXEL_TYPE*)pc) = pack((c1*5+c2*3)/8);

  /*
  *((PIXEL_TYPE*)pc) = ((((c1 & 0x00FF00)*5 + (c2 & 0x00FF00)*3 ) & 0x0007F800) +
                 (((c1 & 0xFF00FF)*5 + (c2 & 0xFF00FF)*3 ) & 0x07F807F8)) >> 3;
                 */
}

#define PIXEL00_0     *((PIXEL_TYPE*)(pOut)) = c[5];
#define PIXEL00_11    Interp1(pOut, c[5], c[4]);
#define PIXEL00_12    Interp1(pOut, c[5], c[2]);
#define PIXEL00_20    Interp2(pOut, c[5], c[2], c[4]);
#define PIXEL00_50    Interp5(pOut, c[2], c[4]);
#define PIXEL00_80    Interp8(pOut, c[5], c[1]);
#define PIXEL00_81    Interp8(pOut, c[5], c[4]);
#define PIXEL00_82    Interp8(pOut, c[5], c[2]);
#define PIXEL01_0     *((PIXEL_TYPE*)(pOut+sizeof(PIXEL_TYPE))) = c[5];
#define PIXEL01_10    Interp1(pOut+sizeof(PIXEL_TYPE), c[5], c[1]);
#define PIXEL01_12    Interp1(pOut+sizeof(PIXEL_TYPE), c[5], c[2]);
#define PIXEL01_14    Interp1(pOut+sizeof(PIXEL_TYPE), c[2], c[5]);
#define PIXEL01_21    Interp2(pOut+sizeof(PIXEL_TYPE), c[2], c[5], c[4]);
#define PIXEL01_31    Interp3(pOut+sizeof(PIXEL_TYPE), c[5], c[4]);
#define PIXEL01_50    Interp5(pOut+sizeof(PIXEL_TYPE), c[2], c[5]);
#define PIXEL01_60    Interp6(pOut+sizeof(PIXEL_TYPE), c[5], c[2], c[4]);
#define PIXEL01_61    Interp6(pOut+sizeof(PIXEL_TYPE), c[5], c[2], c[1]);
#define PIXEL01_82    Interp8(pOut+sizeof(PIXEL_TYPE), c[5], c[2]);
#define PIXEL01_83    Interp8(pOut+sizeof(PIXEL_TYPE), c[2], c[4]);
#define PIXEL02_0     *((PIXEL_TYPE*)(pOut+sizeof(PIXEL_TYPE)*2)) = c[5];
#define PIXEL02_10    Interp1(pOut+sizeof(PIXEL_TYPE)*2, c[5], c[3]);
#define PIXEL02_11    Interp1(pOut+sizeof(PIXEL_TYPE)*2, c[5], c[2]);
#define PIXEL02_13    Interp1(pOut+sizeof(PIXEL_TYPE)*2, c[2], c[5]);
#define PIXEL02_21    Interp2(pOut+sizeof(PIXEL_TYPE)*2, c[2], c[5], c[6]);
#define PIXEL02_32    Interp3(pOut+sizeof(PIXEL_TYPE)*2, c[5], c[6]);
#define PIXEL02_50    Interp5(pOut+sizeof(PIXEL_TYPE)*2, c[2], c[5]);
#define PIXEL02_60    Interp6(pOut+sizeof(PIXEL_TYPE)*2, c[5], c[2], c[6]);
#define PIXEL02_61    Interp6(pOut+sizeof(PIXEL_TYPE)*2, c[5], c[2], c[3]);
#define PIXEL02_81    Interp8(pOut+sizeof(PIXEL_TYPE)*2, c[5], c[2]);
#define PIXEL02_83    Interp8(pOut+sizeof(PIXEL_TYPE)*2, c[2], c[6]);
#define PIXEL03_0     *((PIXEL_TYPE*)(pOut+sizeof(PIXEL_TYPE)*3)) = c[5];
#define PIXEL03_11    Interp1(pOut+sizeof(PIXEL_TYPE)*3, c[5], c[2]);
#define PIXEL03_12    Interp1(pOut+sizeof(PIXEL_TYPE)*3, c[5], c[6]);
#define PIXEL03_20    Interp2(pOut+sizeof(PIXEL_TYPE)*3, c[5], c[2], c[6]);
#define PIXEL03_50    Interp5(pOut+sizeof(PIXEL_TYPE)*3, c[2], c[6]);
#define PIXEL03_80    Interp8(pOut+sizeof(PIXEL_TYPE)*3, c[5], c[3]);
#define PIXEL03_81    Interp8(pOut+sizeof(PIXEL_TYPE)*3, c[5], c[2]);
#define PIXEL03_82    Interp8(pOut+sizeof(PIXEL_TYPE)*3, c[5], c[6]);
#define PIXEL10_0     *((PIXEL_TYPE*)(pOut+BpL)) = c[5];
#define PIXEL10_10    Interp1(pOut+BpL, c[5], c[1]);
#define PIXEL10_11    Interp1(pOut+BpL, c[5], c[4]);
#define PIXEL10_13    Interp1(pOut+BpL, c[4], c[5]);
#define PIXEL10_21    Interp2(pOut+BpL, c[4], c[5], c[2]);
#define PIXEL10_32    Interp3(pOut+BpL, c[5], c[2]);
#define PIXEL10_50    Interp5(pOut+BpL, c[4], c[5]);
#define PIXEL10_60    Interp6(pOut+BpL, c[5], c[4], c[2]);
#define PIXEL10_61    Interp6(pOut+BpL, c[5], c[4], c[1]);
#define PIXEL10_81    Interp8(pOut+BpL, c[5], c[4]);
#define PIXEL10_83    Interp8(pOut+BpL, c[4], c[2]);
#define PIXEL11_0     *((PIXEL_TYPE*)(pOut+BpL+sizeof(PIXEL_TYPE))) = c[5];
#define PIXEL11_30    Interp3(pOut+BpL+sizeof(PIXEL_TYPE), c[5], c[1]);
#define PIXEL11_31    Interp3(pOut+BpL+sizeof(PIXEL_TYPE), c[5], c[4]);
#define PIXEL11_32    Interp3(pOut+BpL+sizeof(PIXEL_TYPE), c[5], c[2]);
#define PIXEL11_70    Interp7(pOut+BpL+sizeof(PIXEL_TYPE), c[5], c[4], c[2]);
#define PIXEL12_0     *((PIXEL_TYPE*)(pOut+BpL+sizeof(PIXEL_TYPE)*2)) = c[5];
#define PIXEL12_30    Interp3(pOut+BpL+sizeof(PIXEL_TYPE)*2, c[5], c[3]);
#define PIXEL12_31    Interp3(pOut+BpL+sizeof(PIXEL_TYPE)*2, c[5], c[2]);
#define PIXEL12_32    Interp3(pOut+BpL+sizeof(PIXEL_TYPE)*2, c[5], c[6]);
#define PIXEL12_70    Interp7(pOut+BpL+sizeof(PIXEL_TYPE)*2, c[5], c[6], c[2]);
#define PIXEL13_0     *((PIXEL_TYPE*)(pOut+BpL+sizeof(PIXEL_TYPE)*3)) = c[5];
#define PIXEL13_10    Interp1(pOut+BpL+sizeof(PIXEL_TYPE)*3, c[5], c[3]);
#define PIXEL13_12    Interp1(pOut+BpL+sizeof(PIXEL_TYPE)*3, c[5], c[6]);
#define PIXEL13_14    Interp1(pOut+BpL+sizeof(PIXEL_TYPE)*3, c[6], c[5]);
#define PIXEL13_21    Interp2(pOut+BpL+sizeof(PIXEL_TYPE)*3, c[6], c[5], c[2]);
#define PIXEL13_31    Interp3(pOut+BpL+sizeof(PIXEL_TYPE)*3, c[5], c[2]);
#define PIXEL13_50    Interp5(pOut+BpL+sizeof(PIXEL_TYPE)*3, c[6], c[5]);
#define PIXEL13_60    Interp6(pOut+BpL+sizeof(PIXEL_TYPE)*3, c[5], c[6], c[2]);
#define PIXEL13_61    Interp6(pOut+BpL+sizeof(PIXEL_TYPE)*3, c[5], c[6], c[3]);
#define PIXEL13_82    Interp8(pOut+BpL+sizeof(PIXEL_TYPE)*3, c[5], c[6]);
#define PIXEL13_83    Interp8(pOut+BpL+sizeof(PIXEL_TYPE)*3, c[6], c[2]);
#define PIXEL20_0     *((PIXEL_TYPE*)(pOut+BpL+BpL)) = c[5];
#define PIXEL20_10    Interp1(pOut+BpL+BpL, c[5], c[7]);
#define PIXEL20_12    Interp1(pOut+BpL+BpL, c[5], c[4]);
#define PIXEL20_14    Interp1(pOut+BpL+BpL, c[4], c[5]);
#define PIXEL20_21    Interp2(pOut+BpL+BpL, c[4], c[5], c[8]);
#define PIXEL20_31    Interp3(pOut+BpL+BpL, c[5], c[8]);
#define PIXEL20_50    Interp5(pOut+BpL+BpL, c[4], c[5]);
#define PIXEL20_60    Interp6(pOut+BpL+BpL, c[5], c[4], c[8]);
#define PIXEL20_61    Interp6(pOut+BpL+BpL, c[5], c[4], c[7]);
#define PIXEL20_82    Interp8(pOut+BpL+BpL, c[5], c[4]);
#define PIXEL20_83    Interp8(pOut+BpL+BpL, c[4], c[8]);
#define PIXEL21_0     *((PIXEL_TYPE*)(pOut+BpL+BpL+sizeof(PIXEL_TYPE))) = c[5];
#define PIXEL21_30    Interp3(pOut+BpL+BpL+sizeof(PIXEL_TYPE), c[5], c[7]);
#define PIXEL21_31    Interp3(pOut+BpL+BpL+sizeof(PIXEL_TYPE), c[5], c[8]);
#define PIXEL21_32    Interp3(pOut+BpL+BpL+sizeof(PIXEL_TYPE), c[5], c[4]);
#define PIXEL21_70    Interp7(pOut+BpL+BpL+sizeof(PIXEL_TYPE), c[5], c[4], c[8]);
#define PIXEL22_0     *((PIXEL_TYPE*)(pOut+BpL+BpL+sizeof(PIXEL_TYPE)*2)) = c[5];
#define PIXEL22_30    Interp3(pOut+BpL+BpL+sizeof(PIXEL_TYPE)*2, c[5], c[9]);
#define PIXEL22_31    Interp3(pOut+BpL+BpL+sizeof(PIXEL_TYPE)*2, c[5], c[6]);
#define PIXEL22_32    Interp3(pOut+BpL+BpL+sizeof(PIXEL_TYPE)*2, c[5], c[8]);
#define PIXEL22_70    Interp7(pOut+BpL+BpL+sizeof(PIXEL_TYPE)*2, c[5], c[6], c[8]);
#define PIXEL23_0     *((PIXEL_TYPE*)(pOut+BpL+BpL+sizeof(PIXEL_TYPE)*3)) = c[5];
#define PIXEL23_10    Interp1(pOut+BpL+BpL+sizeof(PIXEL_TYPE)*3, c[5], c[9]);
#define PIXEL23_11    Interp1(pOut+BpL+BpL+sizeof(PIXEL_TYPE)*3, c[5], c[6]);
#define PIXEL23_13    Interp1(pOut+BpL+BpL+sizeof(PIXEL_TYPE)*3, c[6], c[5]);
#define PIXEL23_21    Interp2(pOut+BpL+BpL+sizeof(PIXEL_TYPE)*3, c[6], c[5], c[8]);
#define PIXEL23_32    Interp3(pOut+BpL+BpL+sizeof(PIXEL_TYPE)*3, c[5], c[8]);
#define PIXEL23_50    Interp5(pOut+BpL+BpL+sizeof(PIXEL_TYPE)*3, c[6], c[5]);
#define PIXEL23_60    Interp6(pOut+BpL+BpL+sizeof(PIXEL_TYPE)*3, c[5], c[6], c[8]);
#define PIXEL23_61    Interp6(pOut+BpL+BpL+sizeof(PIXEL_TYPE)*3, c[5], c[6], c[9]);
#define PIXEL23_81    Interp8(pOut+BpL+BpL+sizeof(PIXEL_TYPE)*3, c[5], c[6]);
#define PIXEL23_83    Interp8(pOut+BpL+BpL+sizeof(PIXEL_TYPE)*3, c[6], c[8]);
#define PIXEL30_0     *((PIXEL_TYPE*)(pOut+BpL+BpL+BpL)) = c[5];
#define PIXEL30_11    Interp1(pOut+BpL+BpL+BpL, c[5], c[8]);
#define PIXEL30_12    Interp1(pOut+BpL+BpL+BpL, c[5], c[4]);
#define PIXEL30_20    Interp2(pOut+BpL+BpL+BpL, c[5], c[8], c[4]);
#define PIXEL30_50    Interp5(pOut+BpL+BpL+BpL, c[8], c[4]);
#define PIXEL30_80    Interp8(pOut+BpL+BpL+BpL, c[5], c[7]);
#define PIXEL30_81    Interp8(pOut+BpL+BpL+BpL, c[5], c[8]);
#define PIXEL30_82    Interp8(pOut+BpL+BpL+BpL, c[5], c[4]);
#define PIXEL31_0     *((PIXEL_TYPE*)(pOut+BpL+BpL+BpL+sizeof(PIXEL_TYPE))) = c[5];
#define PIXEL31_10    Interp1(pOut+BpL+BpL+BpL+sizeof(PIXEL_TYPE), c[5], c[7]);
#define PIXEL31_11    Interp1(pOut+BpL+BpL+BpL+sizeof(PIXEL_TYPE), c[5], c[8]);
#define PIXEL31_13    Interp1(pOut+BpL+BpL+BpL+sizeof(PIXEL_TYPE), c[8], c[5]);
#define PIXEL31_21    Interp2(pOut+BpL+BpL+BpL+sizeof(PIXEL_TYPE), c[8], c[5], c[4]);
#define PIXEL31_32    Interp3(pOut+BpL+BpL+BpL+sizeof(PIXEL_TYPE), c[5], c[4]);
#define PIXEL31_50    Interp5(pOut+BpL+BpL+BpL+sizeof(PIXEL_TYPE), c[8], c[5]);
#define PIXEL31_60    Interp6(pOut+BpL+BpL+BpL+sizeof(PIXEL_TYPE), c[5], c[8], c[4]);
#define PIXEL31_61    Interp6(pOut+BpL+BpL+BpL+sizeof(PIXEL_TYPE), c[5], c[8], c[7]);
#define PIXEL31_81    Interp8(pOut+BpL+BpL+BpL+sizeof(PIXEL_TYPE), c[5], c[8]);
#define PIXEL31_83    Interp8(pOut+BpL+BpL+BpL+sizeof(PIXEL_TYPE), c[8], c[4]);
#define PIXEL32_0     *((PIXEL_TYPE*)(pOut+BpL+BpL+BpL+sizeof(PIXEL_TYPE)*2)) = c[5];
#define PIXEL32_10    Interp1(pOut+BpL+BpL+BpL+sizeof(PIXEL_TYPE)*2, c[5], c[9]);
#define PIXEL32_12    Interp1(pOut+BpL+BpL+BpL+sizeof(PIXEL_TYPE)*2, c[5], c[8]);
#define PIXEL32_14    Interp1(pOut+BpL+BpL+BpL+sizeof(PIXEL_TYPE)*2, c[8], c[5]);
#define PIXEL32_21    Interp2(pOut+BpL+BpL+BpL+sizeof(PIXEL_TYPE)*2, c[8], c[5], c[6]);
#define PIXEL32_31    Interp3(pOut+BpL+BpL+BpL+sizeof(PIXEL_TYPE)*2, c[5], c[6]);
#define PIXEL32_50    Interp5(pOut+BpL+BpL+BpL+sizeof(PIXEL_TYPE)*2, c[8], c[5]);
#define PIXEL32_60    Interp6(pOut+BpL+BpL+BpL+sizeof(PIXEL_TYPE)*2, c[5], c[8], c[6]);
#define PIXEL32_61    Interp6(pOut+BpL+BpL+BpL+sizeof(PIXEL_TYPE)*2, c[5], c[8], c[9]);
#define PIXEL32_82    Interp8(pOut+BpL+BpL+BpL+sizeof(PIXEL_TYPE)*2, c[5], c[8]);
#define PIXEL32_83    Interp8(pOut+BpL+BpL+BpL+sizeof(PIXEL_TYPE)*2, c[8], c[6]);
#define PIXEL33_0     *((PIXEL_TYPE*)(pOut+BpL+BpL+BpL+sizeof(PIXEL_TYPE)*3)) = c[5];
#define PIXEL33_11    Interp1(pOut+BpL+BpL+BpL+sizeof(PIXEL_TYPE)*3, c[5], c[6]);
#define PIXEL33_12    Interp1(pOut+BpL+BpL+BpL+sizeof(PIXEL_TYPE)*3, c[5], c[8]);
#define PIXEL33_20    Interp2(pOut+BpL+BpL+BpL+sizeof(PIXEL_TYPE)*3, c[5], c[8], c[6]);
#define PIXEL33_50    Interp5(pOut+BpL+BpL+BpL+sizeof(PIXEL_TYPE)*3, c[8], c[6]);
#define PIXEL33_80    Interp8(pOut+BpL+BpL+BpL+sizeof(PIXEL_TYPE)*3, c[5], c[9]);
#define PIXEL33_81    Interp8(pOut+BpL+BpL+BpL+sizeof(PIXEL_TYPE)*3, c[5], c[6]);
#define PIXEL33_82    Interp8(pOut+BpL+BpL+BpL+sizeof(PIXEL_TYPE)*3, c[5], c[8]);

inline bool Diff(unsigned int w1, unsigned int w2){
  YUV1 = RGBtoYUV[w1];
  YUV2 = RGBtoYUV[w2];
  return ((abs((YUV1 & Ymask) - (YUV2 & Ymask)) > trY) ||
          (abs((YUV1 & Umask) - (YUV2 & Umask)) > trU) ||
          (abs((YUV1 & Vmask) - (YUV2 & Vmask)) > trV));
}

void InitLUTs(void){
    static bool initialized = false;
    if (initialized){
        return;
    }
    initialized = true;
  // int i, j, k, r, g, b, Y, u, v;

  /*
  for (i=0; i<65536; i++)
    LUT16to32[i] = ((i & 0xF800) << 8) + ((i & 0x07E0) << 5) + ((i & 0x001F) << 3);
    */

  for (unsigned int i = 0; i < 65536; i++) {
        uint8_t B = (i >>  0) & 31;
        uint8_t G = (i >>  5) & 63;
        uint8_t R = (i >> 11) & 31;

        uint8_t r, g, b;
        rgb565_to_rgb888(R, G, B, r, g, b);

        // LUT16to32[i] = Graphics::makeColor(r, g, b);

        double y, u, v;
        rgb888_to_yuv888(r, g, b, y, u, v);

        /*
        int y = (r + g + b) >> 2;
        int u = 128 + ((r - b) >> 2);
        int v = 128 + ((-r + 2*g -b)>>3);
        */

        /* Pack the YUV parameters into a single int */
        RGBtoYUV[i] = ((unsigned)y << 16) + ((unsigned)u << 8) + ((unsigned)v);
  }

  /*
  for (i=0; i<32; i++){
      for (j=0; j<64; j++){
          for (k=0; k<32; k++){
              r = i << 3;
              g = j << 2;
              b = k << 3;
              Y = (r + g + b) >> 2;
              u = 128 + ((r - b) >> 2);
              v = 128 + ((-r + 2*g -b)>>3);
              RGBtoYUV[(i << 11) + (j << 5) + k] = (Y<<16) + (u<<8) + v;
          }
      }
  }
  */
}

void hq4x_16(unsigned char * input, unsigned char * output, int Xres, int Yres, int inputPitch, int outputPitch){
  //   +----+----+----+
  //   |    |    |    |
  //   | w1 | w2 | w3 |
  //   +----+----+----+
  //   |    |    |    |
  //   | w4 | w5 | w6 |
  //   +----+----+----+
  //   |    |    |    |
  //   | w7 | w8 | w9 |
  //   +----+----+----+

  const int BpL = outputPitch;

  for (int j=0; j<Yres; j++){
    int prevline, nextline;
    if (j>0)      prevline = -inputPitch; else prevline = 0;
    if (j<Yres-1) nextline =  inputPitch; else nextline = 0;

    unsigned char * pIn = input + j * inputPitch;
    /* move by 4 lines each time */
    unsigned char * pOut = output + j * outputPitch * 4;

    for (int i=0; i<Xres; i++){
        int w[10];
        int c[10];
      w[2] = *((unsigned short*)(pIn + prevline));
      w[5] = *((unsigned short*)pIn);
      w[8] = *((unsigned short*)(pIn + nextline));

      if (i>0){
        w[1] = *((unsigned short*)(pIn + prevline - sizeof(PIXEL_TYPE)));
        w[4] = *((unsigned short*)(pIn - sizeof(PIXEL_TYPE)));
        w[7] = *((unsigned short*)(pIn + nextline - sizeof(PIXEL_TYPE)));
      } else {
        w[1] = w[2];
        w[4] = w[5];
        w[7] = w[8];
      }

      if (i<Xres-1) {
        w[3] = *((unsigned short*)(pIn + prevline + sizeof(PIXEL_TYPE)));
        w[6] = *((unsigned short*)(pIn + sizeof(PIXEL_TYPE)));
        w[9] = *((unsigned short*)(pIn + nextline + sizeof(PIXEL_TYPE)));
      } else {
        w[3] = w[2];
        w[6] = w[5];
        w[9] = w[8];
      }

      int pattern = 0;
      int flag = 1;

      YUV1 = RGBtoYUV[w[5]];

      for (int k=1; k<=9; k++){
        if (k==5) continue;

        if ( w[k] != w[5] )
        {
          YUV2 = RGBtoYUV[w[k]];
          if ( ( abs((YUV1 & Ymask) - (YUV2 & Ymask)) > trY ) ||
               ( abs((YUV1 & Umask) - (YUV2 & Umask)) > trU ) ||
               ( abs((YUV1 & Vmask) - (YUV2 & Vmask)) > trV ) )
            pattern |= flag;
        }
        flag <<= 1;
      }

      for (int k = 1; k <= 9; k++){
        // c[k] = LUT16to32[w[k]];
        c[k] = w[k];
      }

      switch (pattern){
        case 0:
        case 1:
        case 4:
        case 32:
        case 128:
        case 5:
        case 132:
        case 160:
        case 33:
        case 129:
        case 36:
        case 133:
        case 164:
        case 161:
        case 37:
        case 165:
        {
          PIXEL00_20
          PIXEL01_60
          PIXEL02_60
          PIXEL03_20
          PIXEL10_60
          PIXEL11_70
          PIXEL12_70
          PIXEL13_60
          PIXEL20_60
          PIXEL21_70
          PIXEL22_70
          PIXEL23_60
          PIXEL30_20
          PIXEL31_60
          PIXEL32_60
          PIXEL33_20
          break;
        }
        case 2:
        case 34:
        case 130:
        case 162:
        {
          PIXEL00_80
          PIXEL01_10
          PIXEL02_10
          PIXEL03_80
          PIXEL10_61
          PIXEL11_30
          PIXEL12_30
          PIXEL13_61
          PIXEL20_60
          PIXEL21_70
          PIXEL22_70
          PIXEL23_60
          PIXEL30_20
          PIXEL31_60
          PIXEL32_60
          PIXEL33_20
          break;
        }
        case 16:
        case 17:
        case 48:
        case 49:
        {
          PIXEL00_20
          PIXEL01_60
          PIXEL02_61
          PIXEL03_80
          PIXEL10_60
          PIXEL11_70
          PIXEL12_30
          PIXEL13_10
          PIXEL20_60
          PIXEL21_70
          PIXEL22_30
          PIXEL23_10
          PIXEL30_20
          PIXEL31_60
          PIXEL32_61
          PIXEL33_80
          break;
        }
        case 64:
        case 65:
        case 68:
        case 69:
        {
          PIXEL00_20
          PIXEL01_60
          PIXEL02_60
          PIXEL03_20
          PIXEL10_60
          PIXEL11_70
          PIXEL12_70
          PIXEL13_60
          PIXEL20_61
          PIXEL21_30
          PIXEL22_30
          PIXEL23_61
          PIXEL30_80
          PIXEL31_10
          PIXEL32_10
          PIXEL33_80
          break;
        }
        case 8:
        case 12:
        case 136:
        case 140:
        {
          PIXEL00_80
          PIXEL01_61
          PIXEL02_60
          PIXEL03_20
          PIXEL10_10
          PIXEL11_30
          PIXEL12_70
          PIXEL13_60
          PIXEL20_10
          PIXEL21_30
          PIXEL22_70
          PIXEL23_60
          PIXEL30_80
          PIXEL31_61
          PIXEL32_60
          PIXEL33_20
          break;
        }
        case 3:
        case 35:
        case 131:
        case 163:
        {
          PIXEL00_81
          PIXEL01_31
          PIXEL02_10
          PIXEL03_80
          PIXEL10_81
          PIXEL11_31
          PIXEL12_30
          PIXEL13_61
          PIXEL20_60
          PIXEL21_70
          PIXEL22_70
          PIXEL23_60
          PIXEL30_20
          PIXEL31_60
          PIXEL32_60
          PIXEL33_20
          break;
        }
        case 6:
        case 38:
        case 134:
        case 166:
        {
          PIXEL00_80
          PIXEL01_10
          PIXEL02_32
          PIXEL03_82
          PIXEL10_61
          PIXEL11_30
          PIXEL12_32
          PIXEL13_82
          PIXEL20_60
          PIXEL21_70
          PIXEL22_70
          PIXEL23_60
          PIXEL30_20
          PIXEL31_60
          PIXEL32_60
          PIXEL33_20
          break;
        }
        case 20:
        case 21:
        case 52:
        case 53:
        {
          PIXEL00_20
          PIXEL01_60
          PIXEL02_81
          PIXEL03_81
          PIXEL10_60
          PIXEL11_70
          PIXEL12_31
          PIXEL13_31
          PIXEL20_60
          PIXEL21_70
          PIXEL22_30
          PIXEL23_10
          PIXEL30_20
          PIXEL31_60
          PIXEL32_61
          PIXEL33_80
          break;
        }
        case 144:
        case 145:
        case 176:
        case 177:
        {
          PIXEL00_20
          PIXEL01_60
          PIXEL02_61
          PIXEL03_80
          PIXEL10_60
          PIXEL11_70
          PIXEL12_30
          PIXEL13_10
          PIXEL20_60
          PIXEL21_70
          PIXEL22_32
          PIXEL23_32
          PIXEL30_20
          PIXEL31_60
          PIXEL32_82
          PIXEL33_82
          break;
        }
        case 192:
        case 193:
        case 196:
        case 197:
        {
          PIXEL00_20
          PIXEL01_60
          PIXEL02_60
          PIXEL03_20
          PIXEL10_60
          PIXEL11_70
          PIXEL12_70
          PIXEL13_60
          PIXEL20_61
          PIXEL21_30
          PIXEL22_31
          PIXEL23_81
          PIXEL30_80
          PIXEL31_10
          PIXEL32_31
          PIXEL33_81
          break;
        }
        case 96:
        case 97:
        case 100:
        case 101:
        {
          PIXEL00_20
          PIXEL01_60
          PIXEL02_60
          PIXEL03_20
          PIXEL10_60
          PIXEL11_70
          PIXEL12_70
          PIXEL13_60
          PIXEL20_82
          PIXEL21_32
          PIXEL22_30
          PIXEL23_61
          PIXEL30_82
          PIXEL31_32
          PIXEL32_10
          PIXEL33_80
          break;
        }
        case 40:
        case 44:
        case 168:
        case 172:
        {
          PIXEL00_80
          PIXEL01_61
          PIXEL02_60
          PIXEL03_20
          PIXEL10_10
          PIXEL11_30
          PIXEL12_70
          PIXEL13_60
          PIXEL20_31
          PIXEL21_31
          PIXEL22_70
          PIXEL23_60
          PIXEL30_81
          PIXEL31_81
          PIXEL32_60
          PIXEL33_20
          break;
        }
        case 9:
        case 13:
        case 137:
        case 141:
        {
          PIXEL00_82
          PIXEL01_82
          PIXEL02_60
          PIXEL03_20
          PIXEL10_32
          PIXEL11_32
          PIXEL12_70
          PIXEL13_60
          PIXEL20_10
          PIXEL21_30
          PIXEL22_70
          PIXEL23_60
          PIXEL30_80
          PIXEL31_61
          PIXEL32_60
          PIXEL33_20
          break;
        }
        case 18:
        case 50:
        {
          PIXEL00_80
          PIXEL01_10
          if (Diff(w[2], w[6]))
          {
            PIXEL02_10
            PIXEL03_80
            PIXEL12_30
            PIXEL13_10
          }
          else
          {
            PIXEL02_50
            PIXEL03_50
            PIXEL12_0
            PIXEL13_50
          }
          PIXEL10_61
          PIXEL11_30
          PIXEL20_60
          PIXEL21_70
          PIXEL22_30
          PIXEL23_10
          PIXEL30_20
          PIXEL31_60
          PIXEL32_61
          PIXEL33_80
          break;
        }
        case 80:
        case 81:
        {
          PIXEL00_20
          PIXEL01_60
          PIXEL02_61
          PIXEL03_80
          PIXEL10_60
          PIXEL11_70
          PIXEL12_30
          PIXEL13_10
          PIXEL20_61
          PIXEL21_30
          if (Diff(w[6], w[8]))
          {
            PIXEL22_30
            PIXEL23_10
            PIXEL32_10
            PIXEL33_80
          }
          else
          {
            PIXEL22_0
            PIXEL23_50
            PIXEL32_50
            PIXEL33_50
          }
          PIXEL30_80
          PIXEL31_10
          break;
        }
        case 72:
        case 76:
        {
          PIXEL00_80
          PIXEL01_61
          PIXEL02_60
          PIXEL03_20
          PIXEL10_10
          PIXEL11_30
          PIXEL12_70
          PIXEL13_60
          if (Diff(w[8], w[4]))
          {
            PIXEL20_10
            PIXEL21_30
            PIXEL30_80
            PIXEL31_10
          }
          else
          {
            PIXEL20_50
            PIXEL21_0
            PIXEL30_50
            PIXEL31_50
          }
          PIXEL22_30
          PIXEL23_61
          PIXEL32_10
          PIXEL33_80
          break;
        }
        case 10:
        case 138:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_80
            PIXEL01_10
            PIXEL10_10
            PIXEL11_30
          }
          else
          {
            PIXEL00_50
            PIXEL01_50
            PIXEL10_50
            PIXEL11_0
          }
          PIXEL02_10
          PIXEL03_80
          PIXEL12_30
          PIXEL13_61
          PIXEL20_10
          PIXEL21_30
          PIXEL22_70
          PIXEL23_60
          PIXEL30_80
          PIXEL31_61
          PIXEL32_60
          PIXEL33_20
          break;
        }
        case 66:
        {
          PIXEL00_80
          PIXEL01_10
          PIXEL02_10
          PIXEL03_80
          PIXEL10_61
          PIXEL11_30
          PIXEL12_30
          PIXEL13_61
          PIXEL20_61
          PIXEL21_30
          PIXEL22_30
          PIXEL23_61
          PIXEL30_80
          PIXEL31_10
          PIXEL32_10
          PIXEL33_80
          break;
        }
        case 24:
        {
          PIXEL00_80
          PIXEL01_61
          PIXEL02_61
          PIXEL03_80
          PIXEL10_10
          PIXEL11_30
          PIXEL12_30
          PIXEL13_10
          PIXEL20_10
          PIXEL21_30
          PIXEL22_30
          PIXEL23_10
          PIXEL30_80
          PIXEL31_61
          PIXEL32_61
          PIXEL33_80
          break;
        }
        case 7:
        case 39:
        case 135:
        {
          PIXEL00_81
          PIXEL01_31
          PIXEL02_32
          PIXEL03_82
          PIXEL10_81
          PIXEL11_31
          PIXEL12_32
          PIXEL13_82
          PIXEL20_60
          PIXEL21_70
          PIXEL22_70
          PIXEL23_60
          PIXEL30_20
          PIXEL31_60
          PIXEL32_60
          PIXEL33_20
          break;
        }
        case 148:
        case 149:
        case 180:
        {
          PIXEL00_20
          PIXEL01_60
          PIXEL02_81
          PIXEL03_81
          PIXEL10_60
          PIXEL11_70
          PIXEL12_31
          PIXEL13_31
          PIXEL20_60
          PIXEL21_70
          PIXEL22_32
          PIXEL23_32
          PIXEL30_20
          PIXEL31_60
          PIXEL32_82
          PIXEL33_82
          break;
        }
        case 224:
        case 228:
        case 225:
        {
          PIXEL00_20
          PIXEL01_60
          PIXEL02_60
          PIXEL03_20
          PIXEL10_60
          PIXEL11_70
          PIXEL12_70
          PIXEL13_60
          PIXEL20_82
          PIXEL21_32
          PIXEL22_31
          PIXEL23_81
          PIXEL30_82
          PIXEL31_32
          PIXEL32_31
          PIXEL33_81
          break;
        }
        case 41:
        case 169:
        case 45:
        {
          PIXEL00_82
          PIXEL01_82
          PIXEL02_60
          PIXEL03_20
          PIXEL10_32
          PIXEL11_32
          PIXEL12_70
          PIXEL13_60
          PIXEL20_31
          PIXEL21_31
          PIXEL22_70
          PIXEL23_60
          PIXEL30_81
          PIXEL31_81
          PIXEL32_60
          PIXEL33_20
          break;
        }
        case 22:
        case 54:
        {
          PIXEL00_80
          PIXEL01_10
          if (Diff(w[2], w[6]))
          {
            PIXEL02_0
            PIXEL03_0
            PIXEL13_0
          }
          else
          {
            PIXEL02_50
            PIXEL03_50
            PIXEL13_50
          }
          PIXEL10_61
          PIXEL11_30
          PIXEL12_0
          PIXEL20_60
          PIXEL21_70
          PIXEL22_30
          PIXEL23_10
          PIXEL30_20
          PIXEL31_60
          PIXEL32_61
          PIXEL33_80
          break;
        }
        case 208:
        case 209:
        {
          PIXEL00_20
          PIXEL01_60
          PIXEL02_61
          PIXEL03_80
          PIXEL10_60
          PIXEL11_70
          PIXEL12_30
          PIXEL13_10
          PIXEL20_61
          PIXEL21_30
          PIXEL22_0
          if (Diff(w[6], w[8]))
          {
            PIXEL23_0
            PIXEL32_0
            PIXEL33_0
          }
          else
          {
            PIXEL23_50
            PIXEL32_50
            PIXEL33_50
          }
          PIXEL30_80
          PIXEL31_10
          break;
        }
        case 104:
        case 108:
        {
          PIXEL00_80
          PIXEL01_61
          PIXEL02_60
          PIXEL03_20
          PIXEL10_10
          PIXEL11_30
          PIXEL12_70
          PIXEL13_60
          if (Diff(w[8], w[4]))
          {
            PIXEL20_0
            PIXEL30_0
            PIXEL31_0
          }
          else
          {
            PIXEL20_50
            PIXEL30_50
            PIXEL31_50
          }
          PIXEL21_0
          PIXEL22_30
          PIXEL23_61
          PIXEL32_10
          PIXEL33_80
          break;
        }
        case 11:
        case 139:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_0
            PIXEL01_0
            PIXEL10_0
          }
          else
          {
            PIXEL00_50
            PIXEL01_50
            PIXEL10_50
          }
          PIXEL02_10
          PIXEL03_80
          PIXEL11_0
          PIXEL12_30
          PIXEL13_61
          PIXEL20_10
          PIXEL21_30
          PIXEL22_70
          PIXEL23_60
          PIXEL30_80
          PIXEL31_61
          PIXEL32_60
          PIXEL33_20
          break;
        }
        case 19:
        case 51:
        {
          if (Diff(w[2], w[6]))
          {
            PIXEL00_81
            PIXEL01_31
            PIXEL02_10
            PIXEL03_80
            PIXEL12_30
            PIXEL13_10
          }
          else
          {
            PIXEL00_12
            PIXEL01_14
            PIXEL02_83
            PIXEL03_50
            PIXEL12_70
            PIXEL13_21
          }
          PIXEL10_81
          PIXEL11_31
          PIXEL20_60
          PIXEL21_70
          PIXEL22_30
          PIXEL23_10
          PIXEL30_20
          PIXEL31_60
          PIXEL32_61
          PIXEL33_80
          break;
        }
        case 146:
        case 178:
        {
          PIXEL00_80
          PIXEL01_10
          if (Diff(w[2], w[6]))
          {
            PIXEL02_10
            PIXEL03_80
            PIXEL12_30
            PIXEL13_10
            PIXEL23_32
            PIXEL33_82
          }
          else
          {
            PIXEL02_21
            PIXEL03_50
            PIXEL12_70
            PIXEL13_83
            PIXEL23_13
            PIXEL33_11
          }
          PIXEL10_61
          PIXEL11_30
          PIXEL20_60
          PIXEL21_70
          PIXEL22_32
          PIXEL30_20
          PIXEL31_60
          PIXEL32_82
          break;
        }
        case 84:
        case 85:
        {
          PIXEL00_20
          PIXEL01_60
          PIXEL02_81
          if (Diff(w[6], w[8]))
          {
            PIXEL03_81
            PIXEL13_31
            PIXEL22_30
            PIXEL23_10
            PIXEL32_10
            PIXEL33_80
          }
          else
          {
            PIXEL03_12
            PIXEL13_14
            PIXEL22_70
            PIXEL23_83
            PIXEL32_21
            PIXEL33_50
          }
          PIXEL10_60
          PIXEL11_70
          PIXEL12_31
          PIXEL20_61
          PIXEL21_30
          PIXEL30_80
          PIXEL31_10
          break;
        }
        case 112:
        case 113:
        {
          PIXEL00_20
          PIXEL01_60
          PIXEL02_61
          PIXEL03_80
          PIXEL10_60
          PIXEL11_70
          PIXEL12_30
          PIXEL13_10
          PIXEL20_82
          PIXEL21_32
          if (Diff(w[6], w[8]))
          {
            PIXEL22_30
            PIXEL23_10
            PIXEL30_82
            PIXEL31_32
            PIXEL32_10
            PIXEL33_80
          }
          else
          {
            PIXEL22_70
            PIXEL23_21
            PIXEL30_11
            PIXEL31_13
            PIXEL32_83
            PIXEL33_50
          }
          break;
        }
        case 200:
        case 204:
        {
          PIXEL00_80
          PIXEL01_61
          PIXEL02_60
          PIXEL03_20
          PIXEL10_10
          PIXEL11_30
          PIXEL12_70
          PIXEL13_60
          if (Diff(w[8], w[4]))
          {
            PIXEL20_10
            PIXEL21_30
            PIXEL30_80
            PIXEL31_10
            PIXEL32_31
            PIXEL33_81
          }
          else
          {
            PIXEL20_21
            PIXEL21_70
            PIXEL30_50
            PIXEL31_83
            PIXEL32_14
            PIXEL33_12
          }
          PIXEL22_31
          PIXEL23_81
          break;
        }
        case 73:
        case 77:
        {
          if (Diff(w[8], w[4]))
          {
            PIXEL00_82
            PIXEL10_32
            PIXEL20_10
            PIXEL21_30
            PIXEL30_80
            PIXEL31_10
          }
          else
          {
            PIXEL00_11
            PIXEL10_13
            PIXEL20_83
            PIXEL21_70
            PIXEL30_50
            PIXEL31_21
          }
          PIXEL01_82
          PIXEL02_60
          PIXEL03_20
          PIXEL11_32
          PIXEL12_70
          PIXEL13_60
          PIXEL22_30
          PIXEL23_61
          PIXEL32_10
          PIXEL33_80
          break;
        }
        case 42:
        case 170:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_80
            PIXEL01_10
            PIXEL10_10
            PIXEL11_30
            PIXEL20_31
            PIXEL30_81
          }
          else
          {
            PIXEL00_50
            PIXEL01_21
            PIXEL10_83
            PIXEL11_70
            PIXEL20_14
            PIXEL30_12
          }
          PIXEL02_10
          PIXEL03_80
          PIXEL12_30
          PIXEL13_61
          PIXEL21_31
          PIXEL22_70
          PIXEL23_60
          PIXEL31_81
          PIXEL32_60
          PIXEL33_20
          break;
        }
        case 14:
        case 142:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_80
            PIXEL01_10
            PIXEL02_32
            PIXEL03_82
            PIXEL10_10
            PIXEL11_30
          }
          else
          {
            PIXEL00_50
            PIXEL01_83
            PIXEL02_13
            PIXEL03_11
            PIXEL10_21
            PIXEL11_70
          }
          PIXEL12_32
          PIXEL13_82
          PIXEL20_10
          PIXEL21_30
          PIXEL22_70
          PIXEL23_60
          PIXEL30_80
          PIXEL31_61
          PIXEL32_60
          PIXEL33_20
          break;
        }
        case 67:
        {
          PIXEL00_81
          PIXEL01_31
          PIXEL02_10
          PIXEL03_80
          PIXEL10_81
          PIXEL11_31
          PIXEL12_30
          PIXEL13_61
          PIXEL20_61
          PIXEL21_30
          PIXEL22_30
          PIXEL23_61
          PIXEL30_80
          PIXEL31_10
          PIXEL32_10
          PIXEL33_80
          break;
        }
        case 70:
        {
          PIXEL00_80
          PIXEL01_10
          PIXEL02_32
          PIXEL03_82
          PIXEL10_61
          PIXEL11_30
          PIXEL12_32
          PIXEL13_82
          PIXEL20_61
          PIXEL21_30
          PIXEL22_30
          PIXEL23_61
          PIXEL30_80
          PIXEL31_10
          PIXEL32_10
          PIXEL33_80
          break;
        }
        case 28:
        {
          PIXEL00_80
          PIXEL01_61
          PIXEL02_81
          PIXEL03_81
          PIXEL10_10
          PIXEL11_30
          PIXEL12_31
          PIXEL13_31
          PIXEL20_10
          PIXEL21_30
          PIXEL22_30
          PIXEL23_10
          PIXEL30_80
          PIXEL31_61
          PIXEL32_61
          PIXEL33_80
          break;
        }
        case 152:
        {
          PIXEL00_80
          PIXEL01_61
          PIXEL02_61
          PIXEL03_80
          PIXEL10_10
          PIXEL11_30
          PIXEL12_30
          PIXEL13_10
          PIXEL20_10
          PIXEL21_30
          PIXEL22_32
          PIXEL23_32
          PIXEL30_80
          PIXEL31_61
          PIXEL32_82
          PIXEL33_82
          break;
        }
        case 194:
        {
          PIXEL00_80
          PIXEL01_10
          PIXEL02_10
          PIXEL03_80
          PIXEL10_61
          PIXEL11_30
          PIXEL12_30
          PIXEL13_61
          PIXEL20_61
          PIXEL21_30
          PIXEL22_31
          PIXEL23_81
          PIXEL30_80
          PIXEL31_10
          PIXEL32_31
          PIXEL33_81
          break;
        }
        case 98:
        {
          PIXEL00_80
          PIXEL01_10
          PIXEL02_10
          PIXEL03_80
          PIXEL10_61
          PIXEL11_30
          PIXEL12_30
          PIXEL13_61
          PIXEL20_82
          PIXEL21_32
          PIXEL22_30
          PIXEL23_61
          PIXEL30_82
          PIXEL31_32
          PIXEL32_10
          PIXEL33_80
          break;
        }
        case 56:
        {
          PIXEL00_80
          PIXEL01_61
          PIXEL02_61
          PIXEL03_80
          PIXEL10_10
          PIXEL11_30
          PIXEL12_30
          PIXEL13_10
          PIXEL20_31
          PIXEL21_31
          PIXEL22_30
          PIXEL23_10
          PIXEL30_81
          PIXEL31_81
          PIXEL32_61
          PIXEL33_80
          break;
        }
        case 25:
        {
          PIXEL00_82
          PIXEL01_82
          PIXEL02_61
          PIXEL03_80
          PIXEL10_32
          PIXEL11_32
          PIXEL12_30
          PIXEL13_10
          PIXEL20_10
          PIXEL21_30
          PIXEL22_30
          PIXEL23_10
          PIXEL30_80
          PIXEL31_61
          PIXEL32_61
          PIXEL33_80
          break;
        }
        case 26:
        case 31:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_0
            PIXEL01_0
            PIXEL10_0
          }
          else
          {
            PIXEL00_50
            PIXEL01_50
            PIXEL10_50
          }
          if (Diff(w[2], w[6]))
          {
            PIXEL02_0
            PIXEL03_0
            PIXEL13_0
          }
          else
          {
            PIXEL02_50
            PIXEL03_50
            PIXEL13_50
          }
          PIXEL11_0
          PIXEL12_0
          PIXEL20_10
          PIXEL21_30
          PIXEL22_30
          PIXEL23_10
          PIXEL30_80
          PIXEL31_61
          PIXEL32_61
          PIXEL33_80
          break;
        }
        case 82:
        case 214:
        {
          PIXEL00_80
          PIXEL01_10
          if (Diff(w[2], w[6]))
          {
            PIXEL02_0
            PIXEL03_0
            PIXEL13_0
          }
          else
          {
            PIXEL02_50
            PIXEL03_50
            PIXEL13_50
          }
          PIXEL10_61
          PIXEL11_30
          PIXEL12_0
          PIXEL20_61
          PIXEL21_30
          PIXEL22_0
          if (Diff(w[6], w[8]))
          {
            PIXEL23_0
            PIXEL32_0
            PIXEL33_0
          }
          else
          {
            PIXEL23_50
            PIXEL32_50
            PIXEL33_50
          }
          PIXEL30_80
          PIXEL31_10
          break;
        }
        case 88:
        case 248:
        {
          PIXEL00_80
          PIXEL01_61
          PIXEL02_61
          PIXEL03_80
          PIXEL10_10
          PIXEL11_30
          PIXEL12_30
          PIXEL13_10
          if (Diff(w[8], w[4]))
          {
            PIXEL20_0
            PIXEL30_0
            PIXEL31_0
          }
          else
          {
            PIXEL20_50
            PIXEL30_50
            PIXEL31_50
          }
          PIXEL21_0
          PIXEL22_0
          if (Diff(w[6], w[8]))
          {
            PIXEL23_0
            PIXEL32_0
            PIXEL33_0
          }
          else
          {
            PIXEL23_50
            PIXEL32_50
            PIXEL33_50
          }
          break;
        }
        case 74:
        case 107:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_0
            PIXEL01_0
            PIXEL10_0
          }
          else
          {
            PIXEL00_50
            PIXEL01_50
            PIXEL10_50
          }
          PIXEL02_10
          PIXEL03_80
          PIXEL11_0
          PIXEL12_30
          PIXEL13_61
          if (Diff(w[8], w[4]))
          {
            PIXEL20_0
            PIXEL30_0
            PIXEL31_0
          }
          else
          {
            PIXEL20_50
            PIXEL30_50
            PIXEL31_50
          }
          PIXEL21_0
          PIXEL22_30
          PIXEL23_61
          PIXEL32_10
          PIXEL33_80
          break;
        }
        case 27:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_0
            PIXEL01_0
            PIXEL10_0
          }
          else
          {
            PIXEL00_50
            PIXEL01_50
            PIXEL10_50
          }
          PIXEL02_10
          PIXEL03_80
          PIXEL11_0
          PIXEL12_30
          PIXEL13_10
          PIXEL20_10
          PIXEL21_30
          PIXEL22_30
          PIXEL23_10
          PIXEL30_80
          PIXEL31_61
          PIXEL32_61
          PIXEL33_80
          break;
        }
        case 86:
        {
          PIXEL00_80
          PIXEL01_10
          if (Diff(w[2], w[6]))
          {
            PIXEL02_0
            PIXEL03_0
            PIXEL13_0
          }
          else
          {
            PIXEL02_50
            PIXEL03_50
            PIXEL13_50
          }
          PIXEL10_61
          PIXEL11_30
          PIXEL12_0
          PIXEL20_61
          PIXEL21_30
          PIXEL22_30
          PIXEL23_10
          PIXEL30_80
          PIXEL31_10
          PIXEL32_10
          PIXEL33_80
          break;
        }
        case 216:
        {
          PIXEL00_80
          PIXEL01_61
          PIXEL02_61
          PIXEL03_80
          PIXEL10_10
          PIXEL11_30
          PIXEL12_30
          PIXEL13_10
          PIXEL20_10
          PIXEL21_30
          PIXEL22_0
          if (Diff(w[6], w[8]))
          {
            PIXEL23_0
            PIXEL32_0
            PIXEL33_0
          }
          else
          {
            PIXEL23_50
            PIXEL32_50
            PIXEL33_50
          }
          PIXEL30_80
          PIXEL31_10
          break;
        }
        case 106:
        {
          PIXEL00_80
          PIXEL01_10
          PIXEL02_10
          PIXEL03_80
          PIXEL10_10
          PIXEL11_30
          PIXEL12_30
          PIXEL13_61
          if (Diff(w[8], w[4]))
          {
            PIXEL20_0
            PIXEL30_0
            PIXEL31_0
          }
          else
          {
            PIXEL20_50
            PIXEL30_50
            PIXEL31_50
          }
          PIXEL21_0
          PIXEL22_30
          PIXEL23_61
          PIXEL32_10
          PIXEL33_80
          break;
        }
        case 30:
        {
          PIXEL00_80
          PIXEL01_10
          if (Diff(w[2], w[6]))
          {
            PIXEL02_0
            PIXEL03_0
            PIXEL13_0
          }
          else
          {
            PIXEL02_50
            PIXEL03_50
            PIXEL13_50
          }
          PIXEL10_10
          PIXEL11_30
          PIXEL12_0
          PIXEL20_10
          PIXEL21_30
          PIXEL22_30
          PIXEL23_10
          PIXEL30_80
          PIXEL31_61
          PIXEL32_61
          PIXEL33_80
          break;
        }
        case 210:
        {
          PIXEL00_80
          PIXEL01_10
          PIXEL02_10
          PIXEL03_80
          PIXEL10_61
          PIXEL11_30
          PIXEL12_30
          PIXEL13_10
          PIXEL20_61
          PIXEL21_30
          PIXEL22_0
          if (Diff(w[6], w[8]))
          {
            PIXEL23_0
            PIXEL32_0
            PIXEL33_0
          }
          else
          {
            PIXEL23_50
            PIXEL32_50
            PIXEL33_50
          }
          PIXEL30_80
          PIXEL31_10
          break;
        }
        case 120:
        {
          PIXEL00_80
          PIXEL01_61
          PIXEL02_61
          PIXEL03_80
          PIXEL10_10
          PIXEL11_30
          PIXEL12_30
          PIXEL13_10
          if (Diff(w[8], w[4]))
          {
            PIXEL20_0
            PIXEL30_0
            PIXEL31_0
          }
          else
          {
            PIXEL20_50
            PIXEL30_50
            PIXEL31_50
          }
          PIXEL21_0
          PIXEL22_30
          PIXEL23_10
          PIXEL32_10
          PIXEL33_80
          break;
        }
        case 75:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_0
            PIXEL01_0
            PIXEL10_0
          }
          else
          {
            PIXEL00_50
            PIXEL01_50
            PIXEL10_50
          }
          PIXEL02_10
          PIXEL03_80
          PIXEL11_0
          PIXEL12_30
          PIXEL13_61
          PIXEL20_10
          PIXEL21_30
          PIXEL22_30
          PIXEL23_61
          PIXEL30_80
          PIXEL31_10
          PIXEL32_10
          PIXEL33_80
          break;
        }
        case 29:
        {
          PIXEL00_82
          PIXEL01_82
          PIXEL02_81
          PIXEL03_81
          PIXEL10_32
          PIXEL11_32
          PIXEL12_31
          PIXEL13_31
          PIXEL20_10
          PIXEL21_30
          PIXEL22_30
          PIXEL23_10
          PIXEL30_80
          PIXEL31_61
          PIXEL32_61
          PIXEL33_80
          break;
        }
        case 198:
        {
          PIXEL00_80
          PIXEL01_10
          PIXEL02_32
          PIXEL03_82
          PIXEL10_61
          PIXEL11_30
          PIXEL12_32
          PIXEL13_82
          PIXEL20_61
          PIXEL21_30
          PIXEL22_31
          PIXEL23_81
          PIXEL30_80
          PIXEL31_10
          PIXEL32_31
          PIXEL33_81
          break;
        }
        case 184:
        {
          PIXEL00_80
          PIXEL01_61
          PIXEL02_61
          PIXEL03_80
          PIXEL10_10
          PIXEL11_30
          PIXEL12_30
          PIXEL13_10
          PIXEL20_31
          PIXEL21_31
          PIXEL22_32
          PIXEL23_32
          PIXEL30_81
          PIXEL31_81
          PIXEL32_82
          PIXEL33_82
          break;
        }
        case 99:
        {
          PIXEL00_81
          PIXEL01_31
          PIXEL02_10
          PIXEL03_80
          PIXEL10_81
          PIXEL11_31
          PIXEL12_30
          PIXEL13_61
          PIXEL20_82
          PIXEL21_32
          PIXEL22_30
          PIXEL23_61
          PIXEL30_82
          PIXEL31_32
          PIXEL32_10
          PIXEL33_80
          break;
        }
        case 57:
        {
          PIXEL00_82
          PIXEL01_82
          PIXEL02_61
          PIXEL03_80
          PIXEL10_32
          PIXEL11_32
          PIXEL12_30
          PIXEL13_10
          PIXEL20_31
          PIXEL21_31
          PIXEL22_30
          PIXEL23_10
          PIXEL30_81
          PIXEL31_81
          PIXEL32_61
          PIXEL33_80
          break;
        }
        case 71:
        {
          PIXEL00_81
          PIXEL01_31
          PIXEL02_32
          PIXEL03_82
          PIXEL10_81
          PIXEL11_31
          PIXEL12_32
          PIXEL13_82
          PIXEL20_61
          PIXEL21_30
          PIXEL22_30
          PIXEL23_61
          PIXEL30_80
          PIXEL31_10
          PIXEL32_10
          PIXEL33_80
          break;
        }
        case 156:
        {
          PIXEL00_80
          PIXEL01_61
          PIXEL02_81
          PIXEL03_81
          PIXEL10_10
          PIXEL11_30
          PIXEL12_31
          PIXEL13_31
          PIXEL20_10
          PIXEL21_30
          PIXEL22_32
          PIXEL23_32
          PIXEL30_80
          PIXEL31_61
          PIXEL32_82
          PIXEL33_82
          break;
        }
        case 226:
        {
          PIXEL00_80
          PIXEL01_10
          PIXEL02_10
          PIXEL03_80
          PIXEL10_61
          PIXEL11_30
          PIXEL12_30
          PIXEL13_61
          PIXEL20_82
          PIXEL21_32
          PIXEL22_31
          PIXEL23_81
          PIXEL30_82
          PIXEL31_32
          PIXEL32_31
          PIXEL33_81
          break;
        }
        case 60:
        {
          PIXEL00_80
          PIXEL01_61
          PIXEL02_81
          PIXEL03_81
          PIXEL10_10
          PIXEL11_30
          PIXEL12_31
          PIXEL13_31
          PIXEL20_31
          PIXEL21_31
          PIXEL22_30
          PIXEL23_10
          PIXEL30_81
          PIXEL31_81
          PIXEL32_61
          PIXEL33_80
          break;
        }
        case 195:
        {
          PIXEL00_81
          PIXEL01_31
          PIXEL02_10
          PIXEL03_80
          PIXEL10_81
          PIXEL11_31
          PIXEL12_30
          PIXEL13_61
          PIXEL20_61
          PIXEL21_30
          PIXEL22_31
          PIXEL23_81
          PIXEL30_80
          PIXEL31_10
          PIXEL32_31
          PIXEL33_81
          break;
        }
        case 102:
        {
          PIXEL00_80
          PIXEL01_10
          PIXEL02_32
          PIXEL03_82
          PIXEL10_61
          PIXEL11_30
          PIXEL12_32
          PIXEL13_82
          PIXEL20_82
          PIXEL21_32
          PIXEL22_30
          PIXEL23_61
          PIXEL30_82
          PIXEL31_32
          PIXEL32_10
          PIXEL33_80
          break;
        }
        case 153:
        {
          PIXEL00_82
          PIXEL01_82
          PIXEL02_61
          PIXEL03_80
          PIXEL10_32
          PIXEL11_32
          PIXEL12_30
          PIXEL13_10
          PIXEL20_10
          PIXEL21_30
          PIXEL22_32
          PIXEL23_32
          PIXEL30_80
          PIXEL31_61
          PIXEL32_82
          PIXEL33_82
          break;
        }
        case 58:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_80
            PIXEL01_10
            PIXEL10_10
            PIXEL11_30
          }
          else
          {
            PIXEL00_20
            PIXEL01_12
            PIXEL10_11
            PIXEL11_0
          }
          if (Diff(w[2], w[6]))
          {
            PIXEL02_10
            PIXEL03_80
            PIXEL12_30
            PIXEL13_10
          }
          else
          {
            PIXEL02_11
            PIXEL03_20
            PIXEL12_0
            PIXEL13_12
          }
          PIXEL20_31
          PIXEL21_31
          PIXEL22_30
          PIXEL23_10
          PIXEL30_81
          PIXEL31_81
          PIXEL32_61
          PIXEL33_80
          break;
        }
        case 83:
        {
          PIXEL00_81
          PIXEL01_31
          if (Diff(w[2], w[6]))
          {
            PIXEL02_10
            PIXEL03_80
            PIXEL12_30
            PIXEL13_10
          }
          else
          {
            PIXEL02_11
            PIXEL03_20
            PIXEL12_0
            PIXEL13_12
          }
          PIXEL10_81
          PIXEL11_31
          PIXEL20_61
          PIXEL21_30
          if (Diff(w[6], w[8]))
          {
            PIXEL22_30
            PIXEL23_10
            PIXEL32_10
            PIXEL33_80
          }
          else
          {
            PIXEL22_0
            PIXEL23_11
            PIXEL32_12
            PIXEL33_20
          }
          PIXEL30_80
          PIXEL31_10
          break;
        }
        case 92:
        {
          PIXEL00_80
          PIXEL01_61
          PIXEL02_81
          PIXEL03_81
          PIXEL10_10
          PIXEL11_30
          PIXEL12_31
          PIXEL13_31
          if (Diff(w[8], w[4]))
          {
            PIXEL20_10
            PIXEL21_30
            PIXEL30_80
            PIXEL31_10
          }
          else
          {
            PIXEL20_12
            PIXEL21_0
            PIXEL30_20
            PIXEL31_11
          }
          if (Diff(w[6], w[8]))
          {
            PIXEL22_30
            PIXEL23_10
            PIXEL32_10
            PIXEL33_80
          }
          else
          {
            PIXEL22_0
            PIXEL23_11
            PIXEL32_12
            PIXEL33_20
          }
          break;
        }
        case 202:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_80
            PIXEL01_10
            PIXEL10_10
            PIXEL11_30
          }
          else
          {
            PIXEL00_20
            PIXEL01_12
            PIXEL10_11
            PIXEL11_0
          }
          PIXEL02_10
          PIXEL03_80
          PIXEL12_30
          PIXEL13_61
          if (Diff(w[8], w[4]))
          {
            PIXEL20_10
            PIXEL21_30
            PIXEL30_80
            PIXEL31_10
          }
          else
          {
            PIXEL20_12
            PIXEL21_0
            PIXEL30_20
            PIXEL31_11
          }
          PIXEL22_31
          PIXEL23_81
          PIXEL32_31
          PIXEL33_81
          break;
        }
        case 78:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_80
            PIXEL01_10
            PIXEL10_10
            PIXEL11_30
          }
          else
          {
            PIXEL00_20
            PIXEL01_12
            PIXEL10_11
            PIXEL11_0
          }
          PIXEL02_32
          PIXEL03_82
          PIXEL12_32
          PIXEL13_82
          if (Diff(w[8], w[4]))
          {
            PIXEL20_10
            PIXEL21_30
            PIXEL30_80
            PIXEL31_10
          }
          else
          {
            PIXEL20_12
            PIXEL21_0
            PIXEL30_20
            PIXEL31_11
          }
          PIXEL22_30
          PIXEL23_61
          PIXEL32_10
          PIXEL33_80
          break;
        }
        case 154:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_80
            PIXEL01_10
            PIXEL10_10
            PIXEL11_30
          }
          else
          {
            PIXEL00_20
            PIXEL01_12
            PIXEL10_11
            PIXEL11_0
          }
          if (Diff(w[2], w[6]))
          {
            PIXEL02_10
            PIXEL03_80
            PIXEL12_30
            PIXEL13_10
          }
          else
          {
            PIXEL02_11
            PIXEL03_20
            PIXEL12_0
            PIXEL13_12
          }
          PIXEL20_10
          PIXEL21_30
          PIXEL22_32
          PIXEL23_32
          PIXEL30_80
          PIXEL31_61
          PIXEL32_82
          PIXEL33_82
          break;
        }
        case 114:
        {
          PIXEL00_80
          PIXEL01_10
          if (Diff(w[2], w[6]))
          {
            PIXEL02_10
            PIXEL03_80
            PIXEL12_30
            PIXEL13_10
          }
          else
          {
            PIXEL02_11
            PIXEL03_20
            PIXEL12_0
            PIXEL13_12
          }
          PIXEL10_61
          PIXEL11_30
          PIXEL20_82
          PIXEL21_32
          if (Diff(w[6], w[8]))
          {
            PIXEL22_30
            PIXEL23_10
            PIXEL32_10
            PIXEL33_80
          }
          else
          {
            PIXEL22_0
            PIXEL23_11
            PIXEL32_12
            PIXEL33_20
          }
          PIXEL30_82
          PIXEL31_32
          break;
        }
        case 89:
        {
          PIXEL00_82
          PIXEL01_82
          PIXEL02_61
          PIXEL03_80
          PIXEL10_32
          PIXEL11_32
          PIXEL12_30
          PIXEL13_10
          if (Diff(w[8], w[4]))
          {
            PIXEL20_10
            PIXEL21_30
            PIXEL30_80
            PIXEL31_10
          }
          else
          {
            PIXEL20_12
            PIXEL21_0
            PIXEL30_20
            PIXEL31_11
          }
          if (Diff(w[6], w[8]))
          {
            PIXEL22_30
            PIXEL23_10
            PIXEL32_10
            PIXEL33_80
          }
          else
          {
            PIXEL22_0
            PIXEL23_11
            PIXEL32_12
            PIXEL33_20
          }
          break;
        }
        case 90:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_80
            PIXEL01_10
            PIXEL10_10
            PIXEL11_30
          }
          else
          {
            PIXEL00_20
            PIXEL01_12
            PIXEL10_11
            PIXEL11_0
          }
          if (Diff(w[2], w[6]))
          {
            PIXEL02_10
            PIXEL03_80
            PIXEL12_30
            PIXEL13_10
          }
          else
          {
            PIXEL02_11
            PIXEL03_20
            PIXEL12_0
            PIXEL13_12
          }
          if (Diff(w[8], w[4]))
          {
            PIXEL20_10
            PIXEL21_30
            PIXEL30_80
            PIXEL31_10
          }
          else
          {
            PIXEL20_12
            PIXEL21_0
            PIXEL30_20
            PIXEL31_11
          }
          if (Diff(w[6], w[8]))
          {
            PIXEL22_30
            PIXEL23_10
            PIXEL32_10
            PIXEL33_80
          }
          else
          {
            PIXEL22_0
            PIXEL23_11
            PIXEL32_12
            PIXEL33_20
          }
          break;
        }
        case 55:
        case 23:
        {
          if (Diff(w[2], w[6]))
          {
            PIXEL00_81
            PIXEL01_31
            PIXEL02_0
            PIXEL03_0
            PIXEL12_0
            PIXEL13_0
          }
          else
          {
            PIXEL00_12
            PIXEL01_14
            PIXEL02_83
            PIXEL03_50
            PIXEL12_70
            PIXEL13_21
          }
          PIXEL10_81
          PIXEL11_31
          PIXEL20_60
          PIXEL21_70
          PIXEL22_30
          PIXEL23_10
          PIXEL30_20
          PIXEL31_60
          PIXEL32_61
          PIXEL33_80
          break;
        }
        case 182:
        case 150:
        {
          PIXEL00_80
          PIXEL01_10
          if (Diff(w[2], w[6]))
          {
            PIXEL02_0
            PIXEL03_0
            PIXEL12_0
            PIXEL13_0
            PIXEL23_32
            PIXEL33_82
          }
          else
          {
            PIXEL02_21
            PIXEL03_50
            PIXEL12_70
            PIXEL13_83
            PIXEL23_13
            PIXEL33_11
          }
          PIXEL10_61
          PIXEL11_30
          PIXEL20_60
          PIXEL21_70
          PIXEL22_32
          PIXEL30_20
          PIXEL31_60
          PIXEL32_82
          break;
        }
        case 213:
        case 212:
        {
          PIXEL00_20
          PIXEL01_60
          PIXEL02_81
          if (Diff(w[6], w[8]))
          {
            PIXEL03_81
            PIXEL13_31
            PIXEL22_0
            PIXEL23_0
            PIXEL32_0
            PIXEL33_0
          }
          else
          {
            PIXEL03_12
            PIXEL13_14
            PIXEL22_70
            PIXEL23_83
            PIXEL32_21
            PIXEL33_50
          }
          PIXEL10_60
          PIXEL11_70
          PIXEL12_31
          PIXEL20_61
          PIXEL21_30
          PIXEL30_80
          PIXEL31_10
          break;
        }
        case 241:
        case 240:
        {
          PIXEL00_20
          PIXEL01_60
          PIXEL02_61
          PIXEL03_80
          PIXEL10_60
          PIXEL11_70
          PIXEL12_30
          PIXEL13_10
          PIXEL20_82
          PIXEL21_32
          if (Diff(w[6], w[8]))
          {
            PIXEL22_0
            PIXEL23_0
            PIXEL30_82
            PIXEL31_32
            PIXEL32_0
            PIXEL33_0
          }
          else
          {
            PIXEL22_70
            PIXEL23_21
            PIXEL30_11
            PIXEL31_13
            PIXEL32_83
            PIXEL33_50
          }
          break;
        }
        case 236:
        case 232:
        {
          PIXEL00_80
          PIXEL01_61
          PIXEL02_60
          PIXEL03_20
          PIXEL10_10
          PIXEL11_30
          PIXEL12_70
          PIXEL13_60
          if (Diff(w[8], w[4]))
          {
            PIXEL20_0
            PIXEL21_0
            PIXEL30_0
            PIXEL31_0
            PIXEL32_31
            PIXEL33_81
          }
          else
          {
            PIXEL20_21
            PIXEL21_70
            PIXEL30_50
            PIXEL31_83
            PIXEL32_14
            PIXEL33_12
          }
          PIXEL22_31
          PIXEL23_81
          break;
        }
        case 109:
        case 105:
        {
          if (Diff(w[8], w[4]))
          {
            PIXEL00_82
            PIXEL10_32
            PIXEL20_0
            PIXEL21_0
            PIXEL30_0
            PIXEL31_0
          }
          else
          {
            PIXEL00_11
            PIXEL10_13
            PIXEL20_83
            PIXEL21_70
            PIXEL30_50
            PIXEL31_21
          }
          PIXEL01_82
          PIXEL02_60
          PIXEL03_20
          PIXEL11_32
          PIXEL12_70
          PIXEL13_60
          PIXEL22_30
          PIXEL23_61
          PIXEL32_10
          PIXEL33_80
          break;
        }
        case 171:
        case 43:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_0
            PIXEL01_0
            PIXEL10_0
            PIXEL11_0
            PIXEL20_31
            PIXEL30_81
          }
          else
          {
            PIXEL00_50
            PIXEL01_21
            PIXEL10_83
            PIXEL11_70
            PIXEL20_14
            PIXEL30_12
          }
          PIXEL02_10
          PIXEL03_80
          PIXEL12_30
          PIXEL13_61
          PIXEL21_31
          PIXEL22_70
          PIXEL23_60
          PIXEL31_81
          PIXEL32_60
          PIXEL33_20
          break;
        }
        case 143:
        case 15:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_0
            PIXEL01_0
            PIXEL02_32
            PIXEL03_82
            PIXEL10_0
            PIXEL11_0
          }
          else
          {
            PIXEL00_50
            PIXEL01_83
            PIXEL02_13
            PIXEL03_11
            PIXEL10_21
            PIXEL11_70
          }
          PIXEL12_32
          PIXEL13_82
          PIXEL20_10
          PIXEL21_30
          PIXEL22_70
          PIXEL23_60
          PIXEL30_80
          PIXEL31_61
          PIXEL32_60
          PIXEL33_20
          break;
        }
        case 124:
        {
          PIXEL00_80
          PIXEL01_61
          PIXEL02_81
          PIXEL03_81
          PIXEL10_10
          PIXEL11_30
          PIXEL12_31
          PIXEL13_31
          if (Diff(w[8], w[4]))
          {
            PIXEL20_0
            PIXEL30_0
            PIXEL31_0
          }
          else
          {
            PIXEL20_50
            PIXEL30_50
            PIXEL31_50
          }
          PIXEL21_0
          PIXEL22_30
          PIXEL23_10
          PIXEL32_10
          PIXEL33_80
          break;
        }
        case 203:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_0
            PIXEL01_0
            PIXEL10_0
          }
          else
          {
            PIXEL00_50
            PIXEL01_50
            PIXEL10_50
          }
          PIXEL02_10
          PIXEL03_80
          PIXEL11_0
          PIXEL12_30
          PIXEL13_61
          PIXEL20_10
          PIXEL21_30
          PIXEL22_31
          PIXEL23_81
          PIXEL30_80
          PIXEL31_10
          PIXEL32_31
          PIXEL33_81
          break;
        }
        case 62:
        {
          PIXEL00_80
          PIXEL01_10
          if (Diff(w[2], w[6]))
          {
            PIXEL02_0
            PIXEL03_0
            PIXEL13_0
          }
          else
          {
            PIXEL02_50
            PIXEL03_50
            PIXEL13_50
          }
          PIXEL10_10
          PIXEL11_30
          PIXEL12_0
          PIXEL20_31
          PIXEL21_31
          PIXEL22_30
          PIXEL23_10
          PIXEL30_81
          PIXEL31_81
          PIXEL32_61
          PIXEL33_80
          break;
        }
        case 211:
        {
          PIXEL00_81
          PIXEL01_31
          PIXEL02_10
          PIXEL03_80
          PIXEL10_81
          PIXEL11_31
          PIXEL12_30
          PIXEL13_10
          PIXEL20_61
          PIXEL21_30
          PIXEL22_0
          if (Diff(w[6], w[8]))
          {
            PIXEL23_0
            PIXEL32_0
            PIXEL33_0
          }
          else
          {
            PIXEL23_50
            PIXEL32_50
            PIXEL33_50
          }
          PIXEL30_80
          PIXEL31_10
          break;
        }
        case 118:
        {
          PIXEL00_80
          PIXEL01_10
          if (Diff(w[2], w[6]))
          {
            PIXEL02_0
            PIXEL03_0
            PIXEL13_0
          }
          else
          {
            PIXEL02_50
            PIXEL03_50
            PIXEL13_50
          }
          PIXEL10_61
          PIXEL11_30
          PIXEL12_0
          PIXEL20_82
          PIXEL21_32
          PIXEL22_30
          PIXEL23_10
          PIXEL30_82
          PIXEL31_32
          PIXEL32_10
          PIXEL33_80
          break;
        }
        case 217:
        {
          PIXEL00_82
          PIXEL01_82
          PIXEL02_61
          PIXEL03_80
          PIXEL10_32
          PIXEL11_32
          PIXEL12_30
          PIXEL13_10
          PIXEL20_10
          PIXEL21_30
          PIXEL22_0
          if (Diff(w[6], w[8]))
          {
            PIXEL23_0
            PIXEL32_0
            PIXEL33_0
          }
          else
          {
            PIXEL23_50
            PIXEL32_50
            PIXEL33_50
          }
          PIXEL30_80
          PIXEL31_10
          break;
        }
        case 110:
        {
          PIXEL00_80
          PIXEL01_10
          PIXEL02_32
          PIXEL03_82
          PIXEL10_10
          PIXEL11_30
          PIXEL12_32
          PIXEL13_82
          if (Diff(w[8], w[4]))
          {
            PIXEL20_0
            PIXEL30_0
            PIXEL31_0
          }
          else
          {
            PIXEL20_50
            PIXEL30_50
            PIXEL31_50
          }
          PIXEL21_0
          PIXEL22_30
          PIXEL23_61
          PIXEL32_10
          PIXEL33_80
          break;
        }
        case 155:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_0
            PIXEL01_0
            PIXEL10_0
          }
          else
          {
            PIXEL00_50
            PIXEL01_50
            PIXEL10_50
          }
          PIXEL02_10
          PIXEL03_80
          PIXEL11_0
          PIXEL12_30
          PIXEL13_10
          PIXEL20_10
          PIXEL21_30
          PIXEL22_32
          PIXEL23_32
          PIXEL30_80
          PIXEL31_61
          PIXEL32_82
          PIXEL33_82
          break;
        }
        case 188:
        {
          PIXEL00_80
          PIXEL01_61
          PIXEL02_81
          PIXEL03_81
          PIXEL10_10
          PIXEL11_30
          PIXEL12_31
          PIXEL13_31
          PIXEL20_31
          PIXEL21_31
          PIXEL22_32
          PIXEL23_32
          PIXEL30_81
          PIXEL31_81
          PIXEL32_82
          PIXEL33_82
          break;
        }
        case 185:
        {
          PIXEL00_82
          PIXEL01_82
          PIXEL02_61
          PIXEL03_80
          PIXEL10_32
          PIXEL11_32
          PIXEL12_30
          PIXEL13_10
          PIXEL20_31
          PIXEL21_31
          PIXEL22_32
          PIXEL23_32
          PIXEL30_81
          PIXEL31_81
          PIXEL32_82
          PIXEL33_82
          break;
        }
        case 61:
        {
          PIXEL00_82
          PIXEL01_82
          PIXEL02_81
          PIXEL03_81
          PIXEL10_32
          PIXEL11_32
          PIXEL12_31
          PIXEL13_31
          PIXEL20_31
          PIXEL21_31
          PIXEL22_30
          PIXEL23_10
          PIXEL30_81
          PIXEL31_81
          PIXEL32_61
          PIXEL33_80
          break;
        }
        case 157:
        {
          PIXEL00_82
          PIXEL01_82
          PIXEL02_81
          PIXEL03_81
          PIXEL10_32
          PIXEL11_32
          PIXEL12_31
          PIXEL13_31
          PIXEL20_10
          PIXEL21_30
          PIXEL22_32
          PIXEL23_32
          PIXEL30_80
          PIXEL31_61
          PIXEL32_82
          PIXEL33_82
          break;
        }
        case 103:
        {
          PIXEL00_81
          PIXEL01_31
          PIXEL02_32
          PIXEL03_82
          PIXEL10_81
          PIXEL11_31
          PIXEL12_32
          PIXEL13_82
          PIXEL20_82
          PIXEL21_32
          PIXEL22_30
          PIXEL23_61
          PIXEL30_82
          PIXEL31_32
          PIXEL32_10
          PIXEL33_80
          break;
        }
        case 227:
        {
          PIXEL00_81
          PIXEL01_31
          PIXEL02_10
          PIXEL03_80
          PIXEL10_81
          PIXEL11_31
          PIXEL12_30
          PIXEL13_61
          PIXEL20_82
          PIXEL21_32
          PIXEL22_31
          PIXEL23_81
          PIXEL30_82
          PIXEL31_32
          PIXEL32_31
          PIXEL33_81
          break;
        }
        case 230:
        {
          PIXEL00_80
          PIXEL01_10
          PIXEL02_32
          PIXEL03_82
          PIXEL10_61
          PIXEL11_30
          PIXEL12_32
          PIXEL13_82
          PIXEL20_82
          PIXEL21_32
          PIXEL22_31
          PIXEL23_81
          PIXEL30_82
          PIXEL31_32
          PIXEL32_31
          PIXEL33_81
          break;
        }
        case 199:
        {
          PIXEL00_81
          PIXEL01_31
          PIXEL02_32
          PIXEL03_82
          PIXEL10_81
          PIXEL11_31
          PIXEL12_32
          PIXEL13_82
          PIXEL20_61
          PIXEL21_30
          PIXEL22_31
          PIXEL23_81
          PIXEL30_80
          PIXEL31_10
          PIXEL32_31
          PIXEL33_81
          break;
        }
        case 220:
        {
          PIXEL00_80
          PIXEL01_61
          PIXEL02_81
          PIXEL03_81
          PIXEL10_10
          PIXEL11_30
          PIXEL12_31
          PIXEL13_31
          if (Diff(w[8], w[4]))
          {
            PIXEL20_10
            PIXEL21_30
            PIXEL30_80
            PIXEL31_10
          }
          else
          {
            PIXEL20_12
            PIXEL21_0
            PIXEL30_20
            PIXEL31_11
          }
          PIXEL22_0
          if (Diff(w[6], w[8]))
          {
            PIXEL23_0
            PIXEL32_0
            PIXEL33_0
          }
          else
          {
            PIXEL23_50
            PIXEL32_50
            PIXEL33_50
          }
          break;
        }
        case 158:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_80
            PIXEL01_10
            PIXEL10_10
            PIXEL11_30
          }
          else
          {
            PIXEL00_20
            PIXEL01_12
            PIXEL10_11
            PIXEL11_0
          }
          if (Diff(w[2], w[6]))
          {
            PIXEL02_0
            PIXEL03_0
            PIXEL13_0
          }
          else
          {
            PIXEL02_50
            PIXEL03_50
            PIXEL13_50
          }
          PIXEL12_0
          PIXEL20_10
          PIXEL21_30
          PIXEL22_32
          PIXEL23_32
          PIXEL30_80
          PIXEL31_61
          PIXEL32_82
          PIXEL33_82
          break;
        }
        case 234:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_80
            PIXEL01_10
            PIXEL10_10
            PIXEL11_30
          }
          else
          {
            PIXEL00_20
            PIXEL01_12
            PIXEL10_11
            PIXEL11_0
          }
          PIXEL02_10
          PIXEL03_80
          PIXEL12_30
          PIXEL13_61
          if (Diff(w[8], w[4]))
          {
            PIXEL20_0
            PIXEL30_0
            PIXEL31_0
          }
          else
          {
            PIXEL20_50
            PIXEL30_50
            PIXEL31_50
          }
          PIXEL21_0
          PIXEL22_31
          PIXEL23_81
          PIXEL32_31
          PIXEL33_81
          break;
        }
        case 242:
        {
          PIXEL00_80
          PIXEL01_10
          if (Diff(w[2], w[6]))
          {
            PIXEL02_10
            PIXEL03_80
            PIXEL12_30
            PIXEL13_10
          }
          else
          {
            PIXEL02_11
            PIXEL03_20
            PIXEL12_0
            PIXEL13_12
          }
          PIXEL10_61
          PIXEL11_30
          PIXEL20_82
          PIXEL21_32
          PIXEL22_0
          if (Diff(w[6], w[8]))
          {
            PIXEL23_0
            PIXEL32_0
            PIXEL33_0
          }
          else
          {
            PIXEL23_50
            PIXEL32_50
            PIXEL33_50
          }
          PIXEL30_82
          PIXEL31_32
          break;
        }
        case 59:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_0
            PIXEL01_0
            PIXEL10_0
          }
          else
          {
            PIXEL00_50
            PIXEL01_50
            PIXEL10_50
          }
          if (Diff(w[2], w[6]))
          {
            PIXEL02_10
            PIXEL03_80
            PIXEL12_30
            PIXEL13_10
          }
          else
          {
            PIXEL02_11
            PIXEL03_20
            PIXEL12_0
            PIXEL13_12
          }
          PIXEL11_0
          PIXEL20_31
          PIXEL21_31
          PIXEL22_30
          PIXEL23_10
          PIXEL30_81
          PIXEL31_81
          PIXEL32_61
          PIXEL33_80
          break;
        }
        case 121:
        {
          PIXEL00_82
          PIXEL01_82
          PIXEL02_61
          PIXEL03_80
          PIXEL10_32
          PIXEL11_32
          PIXEL12_30
          PIXEL13_10
          if (Diff(w[8], w[4]))
          {
            PIXEL20_0
            PIXEL30_0
            PIXEL31_0
          }
          else
          {
            PIXEL20_50
            PIXEL30_50
            PIXEL31_50
          }
          PIXEL21_0
          if (Diff(w[6], w[8]))
          {
            PIXEL22_30
            PIXEL23_10
            PIXEL32_10
            PIXEL33_80
          }
          else
          {
            PIXEL22_0
            PIXEL23_11
            PIXEL32_12
            PIXEL33_20
          }
          break;
        }
        case 87:
        {
          PIXEL00_81
          PIXEL01_31
          if (Diff(w[2], w[6]))
          {
            PIXEL02_0
            PIXEL03_0
            PIXEL13_0
          }
          else
          {
            PIXEL02_50
            PIXEL03_50
            PIXEL13_50
          }
          PIXEL10_81
          PIXEL11_31
          PIXEL12_0
          PIXEL20_61
          PIXEL21_30
          if (Diff(w[6], w[8]))
          {
            PIXEL22_30
            PIXEL23_10
            PIXEL32_10
            PIXEL33_80
          }
          else
          {
            PIXEL22_0
            PIXEL23_11
            PIXEL32_12
            PIXEL33_20
          }
          PIXEL30_80
          PIXEL31_10
          break;
        }
        case 79:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_0
            PIXEL01_0
            PIXEL10_0
          }
          else
          {
            PIXEL00_50
            PIXEL01_50
            PIXEL10_50
          }
          PIXEL02_32
          PIXEL03_82
          PIXEL11_0
          PIXEL12_32
          PIXEL13_82
          if (Diff(w[8], w[4]))
          {
            PIXEL20_10
            PIXEL21_30
            PIXEL30_80
            PIXEL31_10
          }
          else
          {
            PIXEL20_12
            PIXEL21_0
            PIXEL30_20
            PIXEL31_11
          }
          PIXEL22_30
          PIXEL23_61
          PIXEL32_10
          PIXEL33_80
          break;
        }
        case 122:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_80
            PIXEL01_10
            PIXEL10_10
            PIXEL11_30
          }
          else
          {
            PIXEL00_20
            PIXEL01_12
            PIXEL10_11
            PIXEL11_0
          }
          if (Diff(w[2], w[6]))
          {
            PIXEL02_10
            PIXEL03_80
            PIXEL12_30
            PIXEL13_10
          }
          else
          {
            PIXEL02_11
            PIXEL03_20
            PIXEL12_0
            PIXEL13_12
          }
          if (Diff(w[8], w[4]))
          {
            PIXEL20_0
            PIXEL30_0
            PIXEL31_0
          }
          else
          {
            PIXEL20_50
            PIXEL30_50
            PIXEL31_50
          }
          PIXEL21_0
          if (Diff(w[6], w[8]))
          {
            PIXEL22_30
            PIXEL23_10
            PIXEL32_10
            PIXEL33_80
          }
          else
          {
            PIXEL22_0
            PIXEL23_11
            PIXEL32_12
            PIXEL33_20
          }
          break;
        }
        case 94:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_80
            PIXEL01_10
            PIXEL10_10
            PIXEL11_30
          }
          else
          {
            PIXEL00_20
            PIXEL01_12
            PIXEL10_11
            PIXEL11_0
          }
          if (Diff(w[2], w[6]))
          {
            PIXEL02_0
            PIXEL03_0
            PIXEL13_0
          }
          else
          {
            PIXEL02_50
            PIXEL03_50
            PIXEL13_50
          }
          PIXEL12_0
          if (Diff(w[8], w[4]))
          {
            PIXEL20_10
            PIXEL21_30
            PIXEL30_80
            PIXEL31_10
          }
          else
          {
            PIXEL20_12
            PIXEL21_0
            PIXEL30_20
            PIXEL31_11
          }
          if (Diff(w[6], w[8]))
          {
            PIXEL22_30
            PIXEL23_10
            PIXEL32_10
            PIXEL33_80
          }
          else
          {
            PIXEL22_0
            PIXEL23_11
            PIXEL32_12
            PIXEL33_20
          }
          break;
        }
        case 218:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_80
            PIXEL01_10
            PIXEL10_10
            PIXEL11_30
          }
          else
          {
            PIXEL00_20
            PIXEL01_12
            PIXEL10_11
            PIXEL11_0
          }
          if (Diff(w[2], w[6]))
          {
            PIXEL02_10
            PIXEL03_80
            PIXEL12_30
            PIXEL13_10
          }
          else
          {
            PIXEL02_11
            PIXEL03_20
            PIXEL12_0
            PIXEL13_12
          }
          if (Diff(w[8], w[4]))
          {
            PIXEL20_10
            PIXEL21_30
            PIXEL30_80
            PIXEL31_10
          }
          else
          {
            PIXEL20_12
            PIXEL21_0
            PIXEL30_20
            PIXEL31_11
          }
          PIXEL22_0
          if (Diff(w[6], w[8]))
          {
            PIXEL23_0
            PIXEL32_0
            PIXEL33_0
          }
          else
          {
            PIXEL23_50
            PIXEL32_50
            PIXEL33_50
          }
          break;
        }
        case 91:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_0
            PIXEL01_0
            PIXEL10_0
          }
          else
          {
            PIXEL00_50
            PIXEL01_50
            PIXEL10_50
          }
          if (Diff(w[2], w[6]))
          {
            PIXEL02_10
            PIXEL03_80
            PIXEL12_30
            PIXEL13_10
          }
          else
          {
            PIXEL02_11
            PIXEL03_20
            PIXEL12_0
            PIXEL13_12
          }
          PIXEL11_0
          if (Diff(w[8], w[4]))
          {
            PIXEL20_10
            PIXEL21_30
            PIXEL30_80
            PIXEL31_10
          }
          else
          {
            PIXEL20_12
            PIXEL21_0
            PIXEL30_20
            PIXEL31_11
          }
          if (Diff(w[6], w[8]))
          {
            PIXEL22_30
            PIXEL23_10
            PIXEL32_10
            PIXEL33_80
          }
          else
          {
            PIXEL22_0
            PIXEL23_11
            PIXEL32_12
            PIXEL33_20
          }
          break;
        }
        case 229:
        {
          PIXEL00_20
          PIXEL01_60
          PIXEL02_60
          PIXEL03_20
          PIXEL10_60
          PIXEL11_70
          PIXEL12_70
          PIXEL13_60
          PIXEL20_82
          PIXEL21_32
          PIXEL22_31
          PIXEL23_81
          PIXEL30_82
          PIXEL31_32
          PIXEL32_31
          PIXEL33_81
          break;
        }
        case 167:
        {
          PIXEL00_81
          PIXEL01_31
          PIXEL02_32
          PIXEL03_82
          PIXEL10_81
          PIXEL11_31
          PIXEL12_32
          PIXEL13_82
          PIXEL20_60
          PIXEL21_70
          PIXEL22_70
          PIXEL23_60
          PIXEL30_20
          PIXEL31_60
          PIXEL32_60
          PIXEL33_20
          break;
        }
        case 173:
        {
          PIXEL00_82
          PIXEL01_82
          PIXEL02_60
          PIXEL03_20
          PIXEL10_32
          PIXEL11_32
          PIXEL12_70
          PIXEL13_60
          PIXEL20_31
          PIXEL21_31
          PIXEL22_70
          PIXEL23_60
          PIXEL30_81
          PIXEL31_81
          PIXEL32_60
          PIXEL33_20
          break;
        }
        case 181:
        {
          PIXEL00_20
          PIXEL01_60
          PIXEL02_81
          PIXEL03_81
          PIXEL10_60
          PIXEL11_70
          PIXEL12_31
          PIXEL13_31
          PIXEL20_60
          PIXEL21_70
          PIXEL22_32
          PIXEL23_32
          PIXEL30_20
          PIXEL31_60
          PIXEL32_82
          PIXEL33_82
          break;
        }
        case 186:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_80
            PIXEL01_10
            PIXEL10_10
            PIXEL11_30
          }
          else
          {
            PIXEL00_20
            PIXEL01_12
            PIXEL10_11
            PIXEL11_0
          }
          if (Diff(w[2], w[6]))
          {
            PIXEL02_10
            PIXEL03_80
            PIXEL12_30
            PIXEL13_10
          }
          else
          {
            PIXEL02_11
            PIXEL03_20
            PIXEL12_0
            PIXEL13_12
          }
          PIXEL20_31
          PIXEL21_31
          PIXEL22_32
          PIXEL23_32
          PIXEL30_81
          PIXEL31_81
          PIXEL32_82
          PIXEL33_82
          break;
        }
        case 115:
        {
          PIXEL00_81
          PIXEL01_31
          if (Diff(w[2], w[6]))
          {
            PIXEL02_10
            PIXEL03_80
            PIXEL12_30
            PIXEL13_10
          }
          else
          {
            PIXEL02_11
            PIXEL03_20
            PIXEL12_0
            PIXEL13_12
          }
          PIXEL10_81
          PIXEL11_31
          PIXEL20_82
          PIXEL21_32
          if (Diff(w[6], w[8]))
          {
            PIXEL22_30
            PIXEL23_10
            PIXEL32_10
            PIXEL33_80
          }
          else
          {
            PIXEL22_0
            PIXEL23_11
            PIXEL32_12
            PIXEL33_20
          }
          PIXEL30_82
          PIXEL31_32
          break;
        }
        case 93:
        {
          PIXEL00_82
          PIXEL01_82
          PIXEL02_81
          PIXEL03_81
          PIXEL10_32
          PIXEL11_32
          PIXEL12_31
          PIXEL13_31
          if (Diff(w[8], w[4]))
          {
            PIXEL20_10
            PIXEL21_30
            PIXEL30_80
            PIXEL31_10
          }
          else
          {
            PIXEL20_12
            PIXEL21_0
            PIXEL30_20
            PIXEL31_11
          }
          if (Diff(w[6], w[8]))
          {
            PIXEL22_30
            PIXEL23_10
            PIXEL32_10
            PIXEL33_80
          }
          else
          {
            PIXEL22_0
            PIXEL23_11
            PIXEL32_12
            PIXEL33_20
          }
          break;
        }
        case 206:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_80
            PIXEL01_10
            PIXEL10_10
            PIXEL11_30
          }
          else
          {
            PIXEL00_20
            PIXEL01_12
            PIXEL10_11
            PIXEL11_0
          }
          PIXEL02_32
          PIXEL03_82
          PIXEL12_32
          PIXEL13_82
          if (Diff(w[8], w[4]))
          {
            PIXEL20_10
            PIXEL21_30
            PIXEL30_80
            PIXEL31_10
          }
          else
          {
            PIXEL20_12
            PIXEL21_0
            PIXEL30_20
            PIXEL31_11
          }
          PIXEL22_31
          PIXEL23_81
          PIXEL32_31
          PIXEL33_81
          break;
        }
        case 205:
        case 201:
        {
          PIXEL00_82
          PIXEL01_82
          PIXEL02_60
          PIXEL03_20
          PIXEL10_32
          PIXEL11_32
          PIXEL12_70
          PIXEL13_60
          if (Diff(w[8], w[4]))
          {
            PIXEL20_10
            PIXEL21_30
            PIXEL30_80
            PIXEL31_10
          }
          else
          {
            PIXEL20_12
            PIXEL21_0
            PIXEL30_20
            PIXEL31_11
          }
          PIXEL22_31
          PIXEL23_81
          PIXEL32_31
          PIXEL33_81
          break;
        }
        case 174:
        case 46:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_80
            PIXEL01_10
            PIXEL10_10
            PIXEL11_30
          }
          else
          {
            PIXEL00_20
            PIXEL01_12
            PIXEL10_11
            PIXEL11_0
          }
          PIXEL02_32
          PIXEL03_82
          PIXEL12_32
          PIXEL13_82
          PIXEL20_31
          PIXEL21_31
          PIXEL22_70
          PIXEL23_60
          PIXEL30_81
          PIXEL31_81
          PIXEL32_60
          PIXEL33_20
          break;
        }
        case 179:
        case 147:
        {
          PIXEL00_81
          PIXEL01_31
          if (Diff(w[2], w[6]))
          {
            PIXEL02_10
            PIXEL03_80
            PIXEL12_30
            PIXEL13_10
          }
          else
          {
            PIXEL02_11
            PIXEL03_20
            PIXEL12_0
            PIXEL13_12
          }
          PIXEL10_81
          PIXEL11_31
          PIXEL20_60
          PIXEL21_70
          PIXEL22_32
          PIXEL23_32
          PIXEL30_20
          PIXEL31_60
          PIXEL32_82
          PIXEL33_82
          break;
        }
        case 117:
        case 116:
        {
          PIXEL00_20
          PIXEL01_60
          PIXEL02_81
          PIXEL03_81
          PIXEL10_60
          PIXEL11_70
          PIXEL12_31
          PIXEL13_31
          PIXEL20_82
          PIXEL21_32
          if (Diff(w[6], w[8]))
          {
            PIXEL22_30
            PIXEL23_10
            PIXEL32_10
            PIXEL33_80
          }
          else
          {
            PIXEL22_0
            PIXEL23_11
            PIXEL32_12
            PIXEL33_20
          }
          PIXEL30_82
          PIXEL31_32
          break;
        }
        case 189:
        {
          PIXEL00_82
          PIXEL01_82
          PIXEL02_81
          PIXEL03_81
          PIXEL10_32
          PIXEL11_32
          PIXEL12_31
          PIXEL13_31
          PIXEL20_31
          PIXEL21_31
          PIXEL22_32
          PIXEL23_32
          PIXEL30_81
          PIXEL31_81
          PIXEL32_82
          PIXEL33_82
          break;
        }
        case 231:
        {
          PIXEL00_81
          PIXEL01_31
          PIXEL02_32
          PIXEL03_82
          PIXEL10_81
          PIXEL11_31
          PIXEL12_32
          PIXEL13_82
          PIXEL20_82
          PIXEL21_32
          PIXEL22_31
          PIXEL23_81
          PIXEL30_82
          PIXEL31_32
          PIXEL32_31
          PIXEL33_81
          break;
        }
        case 126:
        {
          PIXEL00_80
          PIXEL01_10
          if (Diff(w[2], w[6]))
          {
            PIXEL02_0
            PIXEL03_0
            PIXEL13_0
          }
          else
          {
            PIXEL02_50
            PIXEL03_50
            PIXEL13_50
          }
          PIXEL10_10
          PIXEL11_30
          PIXEL12_0
          if (Diff(w[8], w[4]))
          {
            PIXEL20_0
            PIXEL30_0
            PIXEL31_0
          }
          else
          {
            PIXEL20_50
            PIXEL30_50
            PIXEL31_50
          }
          PIXEL21_0
          PIXEL22_30
          PIXEL23_10
          PIXEL32_10
          PIXEL33_80
          break;
        }
        case 219:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_0
            PIXEL01_0
            PIXEL10_0
          }
          else
          {
            PIXEL00_50
            PIXEL01_50
            PIXEL10_50
          }
          PIXEL02_10
          PIXEL03_80
          PIXEL11_0
          PIXEL12_30
          PIXEL13_10
          PIXEL20_10
          PIXEL21_30
          PIXEL22_0
          if (Diff(w[6], w[8]))
          {
            PIXEL23_0
            PIXEL32_0
            PIXEL33_0
          }
          else
          {
            PIXEL23_50
            PIXEL32_50
            PIXEL33_50
          }
          PIXEL30_80
          PIXEL31_10
          break;
        }
        case 125:
        {
          if (Diff(w[8], w[4]))
          {
            PIXEL00_82
            PIXEL10_32
            PIXEL20_0
            PIXEL21_0
            PIXEL30_0
            PIXEL31_0
          }
          else
          {
            PIXEL00_11
            PIXEL10_13
            PIXEL20_83
            PIXEL21_70
            PIXEL30_50
            PIXEL31_21
          }
          PIXEL01_82
          PIXEL02_81
          PIXEL03_81
          PIXEL11_32
          PIXEL12_31
          PIXEL13_31
          PIXEL22_30
          PIXEL23_10
          PIXEL32_10
          PIXEL33_80
          break;
        }
        case 221:
        {
          PIXEL00_82
          PIXEL01_82
          PIXEL02_81
          if (Diff(w[6], w[8]))
          {
            PIXEL03_81
            PIXEL13_31
            PIXEL22_0
            PIXEL23_0
            PIXEL32_0
            PIXEL33_0
          }
          else
          {
            PIXEL03_12
            PIXEL13_14
            PIXEL22_70
            PIXEL23_83
            PIXEL32_21
            PIXEL33_50
          }
          PIXEL10_32
          PIXEL11_32
          PIXEL12_31
          PIXEL20_10
          PIXEL21_30
          PIXEL30_80
          PIXEL31_10
          break;
        }
        case 207:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_0
            PIXEL01_0
            PIXEL02_32
            PIXEL03_82
            PIXEL10_0
            PIXEL11_0
          }
          else
          {
            PIXEL00_50
            PIXEL01_83
            PIXEL02_13
            PIXEL03_11
            PIXEL10_21
            PIXEL11_70
          }
          PIXEL12_32
          PIXEL13_82
          PIXEL20_10
          PIXEL21_30
          PIXEL22_31
          PIXEL23_81
          PIXEL30_80
          PIXEL31_10
          PIXEL32_31
          PIXEL33_81
          break;
        }
        case 238:
        {
          PIXEL00_80
          PIXEL01_10
          PIXEL02_32
          PIXEL03_82
          PIXEL10_10
          PIXEL11_30
          PIXEL12_32
          PIXEL13_82
          if (Diff(w[8], w[4]))
          {
            PIXEL20_0
            PIXEL21_0
            PIXEL30_0
            PIXEL31_0
            PIXEL32_31
            PIXEL33_81
          }
          else
          {
            PIXEL20_21
            PIXEL21_70
            PIXEL30_50
            PIXEL31_83
            PIXEL32_14
            PIXEL33_12
          }
          PIXEL22_31
          PIXEL23_81
          break;
        }
        case 190:
        {
          PIXEL00_80
          PIXEL01_10
          if (Diff(w[2], w[6]))
          {
            PIXEL02_0
            PIXEL03_0
            PIXEL12_0
            PIXEL13_0
            PIXEL23_32
            PIXEL33_82
          }
          else
          {
            PIXEL02_21
            PIXEL03_50
            PIXEL12_70
            PIXEL13_83
            PIXEL23_13
            PIXEL33_11
          }
          PIXEL10_10
          PIXEL11_30
          PIXEL20_31
          PIXEL21_31
          PIXEL22_32
          PIXEL30_81
          PIXEL31_81
          PIXEL32_82
          break;
        }
        case 187:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_0
            PIXEL01_0
            PIXEL10_0
            PIXEL11_0
            PIXEL20_31
            PIXEL30_81
          }
          else
          {
            PIXEL00_50
            PIXEL01_21
            PIXEL10_83
            PIXEL11_70
            PIXEL20_14
            PIXEL30_12
          }
          PIXEL02_10
          PIXEL03_80
          PIXEL12_30
          PIXEL13_10
          PIXEL21_31
          PIXEL22_32
          PIXEL23_32
          PIXEL31_81
          PIXEL32_82
          PIXEL33_82
          break;
        }
        case 243:
        {
          PIXEL00_81
          PIXEL01_31
          PIXEL02_10
          PIXEL03_80
          PIXEL10_81
          PIXEL11_31
          PIXEL12_30
          PIXEL13_10
          PIXEL20_82
          PIXEL21_32
          if (Diff(w[6], w[8]))
          {
            PIXEL22_0
            PIXEL23_0
            PIXEL30_82
            PIXEL31_32
            PIXEL32_0
            PIXEL33_0
          }
          else
          {
            PIXEL22_70
            PIXEL23_21
            PIXEL30_11
            PIXEL31_13
            PIXEL32_83
            PIXEL33_50
          }
          break;
        }
        case 119:
        {
          if (Diff(w[2], w[6]))
          {
            PIXEL00_81
            PIXEL01_31
            PIXEL02_0
            PIXEL03_0
            PIXEL12_0
            PIXEL13_0
          }
          else
          {
            PIXEL00_12
            PIXEL01_14
            PIXEL02_83
            PIXEL03_50
            PIXEL12_70
            PIXEL13_21
          }
          PIXEL10_81
          PIXEL11_31
          PIXEL20_82
          PIXEL21_32
          PIXEL22_30
          PIXEL23_10
          PIXEL30_82
          PIXEL31_32
          PIXEL32_10
          PIXEL33_80
          break;
        }
        case 237:
        case 233:
        {
          PIXEL00_82
          PIXEL01_82
          PIXEL02_60
          PIXEL03_20
          PIXEL10_32
          PIXEL11_32
          PIXEL12_70
          PIXEL13_60
          PIXEL20_0
          PIXEL21_0
          PIXEL22_31
          PIXEL23_81
          if (Diff(w[8], w[4]))
          {
            PIXEL30_0
          }
          else
          {
            PIXEL30_20
          }
          PIXEL31_0
          PIXEL32_31
          PIXEL33_81
          break;
        }
        case 175:
        case 47:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_0
          }
          else
          {
            PIXEL00_20
          }
          PIXEL01_0
          PIXEL02_32
          PIXEL03_82
          PIXEL10_0
          PIXEL11_0
          PIXEL12_32
          PIXEL13_82
          PIXEL20_31
          PIXEL21_31
          PIXEL22_70
          PIXEL23_60
          PIXEL30_81
          PIXEL31_81
          PIXEL32_60
          PIXEL33_20
          break;
        }
        case 183:
        case 151:
        {
          PIXEL00_81
          PIXEL01_31
          PIXEL02_0
          if (Diff(w[2], w[6]))
          {
            PIXEL03_0
          }
          else
          {
            PIXEL03_20
          }
          PIXEL10_81
          PIXEL11_31
          PIXEL12_0
          PIXEL13_0
          PIXEL20_60
          PIXEL21_70
          PIXEL22_32
          PIXEL23_32
          PIXEL30_20
          PIXEL31_60
          PIXEL32_82
          PIXEL33_82
          break;
        }
        case 245:
        case 244:
        {
          PIXEL00_20
          PIXEL01_60
          PIXEL02_81
          PIXEL03_81
          PIXEL10_60
          PIXEL11_70
          PIXEL12_31
          PIXEL13_31
          PIXEL20_82
          PIXEL21_32
          PIXEL22_0
          PIXEL23_0
          PIXEL30_82
          PIXEL31_32
          PIXEL32_0
          if (Diff(w[6], w[8]))
          {
            PIXEL33_0
          }
          else
          {
            PIXEL33_20
          }
          break;
        }
        case 250:
        {
          PIXEL00_80
          PIXEL01_10
          PIXEL02_10
          PIXEL03_80
          PIXEL10_10
          PIXEL11_30
          PIXEL12_30
          PIXEL13_10
          if (Diff(w[8], w[4]))
          {
            PIXEL20_0
            PIXEL30_0
            PIXEL31_0
          }
          else
          {
            PIXEL20_50
            PIXEL30_50
            PIXEL31_50
          }
          PIXEL21_0
          PIXEL22_0
          if (Diff(w[6], w[8]))
          {
            PIXEL23_0
            PIXEL32_0
            PIXEL33_0
          }
          else
          {
            PIXEL23_50
            PIXEL32_50
            PIXEL33_50
          }
          break;
        }
        case 123:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_0
            PIXEL01_0
            PIXEL10_0
          }
          else
          {
            PIXEL00_50
            PIXEL01_50
            PIXEL10_50
          }
          PIXEL02_10
          PIXEL03_80
          PIXEL11_0
          PIXEL12_30
          PIXEL13_10
          if (Diff(w[8], w[4]))
          {
            PIXEL20_0
            PIXEL30_0
            PIXEL31_0
          }
          else
          {
            PIXEL20_50
            PIXEL30_50
            PIXEL31_50
          }
          PIXEL21_0
          PIXEL22_30
          PIXEL23_10
          PIXEL32_10
          PIXEL33_80
          break;
        }
        case 95:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_0
            PIXEL01_0
            PIXEL10_0
          }
          else
          {
            PIXEL00_50
            PIXEL01_50
            PIXEL10_50
          }
          if (Diff(w[2], w[6]))
          {
            PIXEL02_0
            PIXEL03_0
            PIXEL13_0
          }
          else
          {
            PIXEL02_50
            PIXEL03_50
            PIXEL13_50
          }
          PIXEL11_0
          PIXEL12_0
          PIXEL20_10
          PIXEL21_30
          PIXEL22_30
          PIXEL23_10
          PIXEL30_80
          PIXEL31_10
          PIXEL32_10
          PIXEL33_80
          break;
        }
        case 222:
        {
          PIXEL00_80
          PIXEL01_10
          if (Diff(w[2], w[6]))
          {
            PIXEL02_0
            PIXEL03_0
            PIXEL13_0
          }
          else
          {
            PIXEL02_50
            PIXEL03_50
            PIXEL13_50
          }
          PIXEL10_10
          PIXEL11_30
          PIXEL12_0
          PIXEL20_10
          PIXEL21_30
          PIXEL22_0
          if (Diff(w[6], w[8]))
          {
            PIXEL23_0
            PIXEL32_0
            PIXEL33_0
          }
          else
          {
            PIXEL23_50
            PIXEL32_50
            PIXEL33_50
          }
          PIXEL30_80
          PIXEL31_10
          break;
        }
        case 252:
        {
          PIXEL00_80
          PIXEL01_61
          PIXEL02_81
          PIXEL03_81
          PIXEL10_10
          PIXEL11_30
          PIXEL12_31
          PIXEL13_31
          if (Diff(w[8], w[4]))
          {
            PIXEL20_0
            PIXEL30_0
            PIXEL31_0
          }
          else
          {
            PIXEL20_50
            PIXEL30_50
            PIXEL31_50
          }
          PIXEL21_0
          PIXEL22_0
          PIXEL23_0
          PIXEL32_0
          if (Diff(w[6], w[8]))
          {
            PIXEL33_0
          }
          else
          {
            PIXEL33_20
          }
          break;
        }
        case 249:
        {
          PIXEL00_82
          PIXEL01_82
          PIXEL02_61
          PIXEL03_80
          PIXEL10_32
          PIXEL11_32
          PIXEL12_30
          PIXEL13_10
          PIXEL20_0
          PIXEL21_0
          PIXEL22_0
          if (Diff(w[6], w[8]))
          {
            PIXEL23_0
            PIXEL32_0
            PIXEL33_0
          }
          else
          {
            PIXEL23_50
            PIXEL32_50
            PIXEL33_50
          }
          if (Diff(w[8], w[4]))
          {
            PIXEL30_0
          }
          else
          {
            PIXEL30_20
          }
          PIXEL31_0
          break;
        }
        case 235:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_0
            PIXEL01_0
            PIXEL10_0
          }
          else
          {
            PIXEL00_50
            PIXEL01_50
            PIXEL10_50
          }
          PIXEL02_10
          PIXEL03_80
          PIXEL11_0
          PIXEL12_30
          PIXEL13_61
          PIXEL20_0
          PIXEL21_0
          PIXEL22_31
          PIXEL23_81
          if (Diff(w[8], w[4]))
          {
            PIXEL30_0
          }
          else
          {
            PIXEL30_20
          }
          PIXEL31_0
          PIXEL32_31
          PIXEL33_81
          break;
        }
        case 111:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_0
          }
          else
          {
            PIXEL00_20
          }
          PIXEL01_0
          PIXEL02_32
          PIXEL03_82
          PIXEL10_0
          PIXEL11_0
          PIXEL12_32
          PIXEL13_82
          if (Diff(w[8], w[4]))
          {
            PIXEL20_0
            PIXEL30_0
            PIXEL31_0
          }
          else
          {
            PIXEL20_50
            PIXEL30_50
            PIXEL31_50
          }
          PIXEL21_0
          PIXEL22_30
          PIXEL23_61
          PIXEL32_10
          PIXEL33_80
          break;
        }
        case 63:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_0
          }
          else
          {
            PIXEL00_20
          }
          PIXEL01_0
          if (Diff(w[2], w[6]))
          {
            PIXEL02_0
            PIXEL03_0
            PIXEL13_0
          }
          else
          {
            PIXEL02_50
            PIXEL03_50
            PIXEL13_50
          }
          PIXEL10_0
          PIXEL11_0
          PIXEL12_0
          PIXEL20_31
          PIXEL21_31
          PIXEL22_30
          PIXEL23_10
          PIXEL30_81
          PIXEL31_81
          PIXEL32_61
          PIXEL33_80
          break;
        }
        case 159:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_0
            PIXEL01_0
            PIXEL10_0
          }
          else
          {
            PIXEL00_50
            PIXEL01_50
            PIXEL10_50
          }
          PIXEL02_0
          if (Diff(w[2], w[6]))
          {
            PIXEL03_0
          }
          else
          {
            PIXEL03_20
          }
          PIXEL11_0
          PIXEL12_0
          PIXEL13_0
          PIXEL20_10
          PIXEL21_30
          PIXEL22_32
          PIXEL23_32
          PIXEL30_80
          PIXEL31_61
          PIXEL32_82
          PIXEL33_82
          break;
        }
        case 215:
        {
          PIXEL00_81
          PIXEL01_31
          PIXEL02_0
          if (Diff(w[2], w[6]))
          {
            PIXEL03_0
          }
          else
          {
            PIXEL03_20
          }
          PIXEL10_81
          PIXEL11_31
          PIXEL12_0
          PIXEL13_0
          PIXEL20_61
          PIXEL21_30
          PIXEL22_0
          if (Diff(w[6], w[8]))
          {
            PIXEL23_0
            PIXEL32_0
            PIXEL33_0
          }
          else
          {
            PIXEL23_50
            PIXEL32_50
            PIXEL33_50
          }
          PIXEL30_80
          PIXEL31_10
          break;
        }
        case 246:
        {
          PIXEL00_80
          PIXEL01_10
          if (Diff(w[2], w[6]))
          {
            PIXEL02_0
            PIXEL03_0
            PIXEL13_0
          }
          else
          {
            PIXEL02_50
            PIXEL03_50
            PIXEL13_50
          }
          PIXEL10_61
          PIXEL11_30
          PIXEL12_0
          PIXEL20_82
          PIXEL21_32
          PIXEL22_0
          PIXEL23_0
          PIXEL30_82
          PIXEL31_32
          PIXEL32_0
          if (Diff(w[6], w[8]))
          {
            PIXEL33_0
          }
          else
          {
            PIXEL33_20
          }
          break;
        }
        case 254:
        {
          PIXEL00_80
          PIXEL01_10
          if (Diff(w[2], w[6]))
          {
            PIXEL02_0
            PIXEL03_0
            PIXEL13_0
          }
          else
          {
            PIXEL02_50
            PIXEL03_50
            PIXEL13_50
          }
          PIXEL10_10
          PIXEL11_30
          PIXEL12_0
          if (Diff(w[8], w[4]))
          {
            PIXEL20_0
            PIXEL30_0
            PIXEL31_0
          }
          else
          {
            PIXEL20_50
            PIXEL30_50
            PIXEL31_50
          }
          PIXEL21_0
          PIXEL22_0
          PIXEL23_0
          PIXEL32_0
          if (Diff(w[6], w[8]))
          {
            PIXEL33_0
          }
          else
          {
            PIXEL33_20
          }
          break;
        }
        case 253:
        {
          PIXEL00_82
          PIXEL01_82
          PIXEL02_81
          PIXEL03_81
          PIXEL10_32
          PIXEL11_32
          PIXEL12_31
          PIXEL13_31
          PIXEL20_0
          PIXEL21_0
          PIXEL22_0
          PIXEL23_0
          if (Diff(w[8], w[4]))
          {
            PIXEL30_0
          }
          else
          {
            PIXEL30_20
          }
          PIXEL31_0
          PIXEL32_0
          if (Diff(w[6], w[8]))
          {
            PIXEL33_0
          }
          else
          {
            PIXEL33_20
          }
          break;
        }
        case 251:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_0
            PIXEL01_0
            PIXEL10_0
          }
          else
          {
            PIXEL00_50
            PIXEL01_50
            PIXEL10_50
          }
          PIXEL02_10
          PIXEL03_80
          PIXEL11_0
          PIXEL12_30
          PIXEL13_10
          PIXEL20_0
          PIXEL21_0
          PIXEL22_0
          if (Diff(w[6], w[8]))
          {
            PIXEL23_0
            PIXEL32_0
            PIXEL33_0
          }
          else
          {
            PIXEL23_50
            PIXEL32_50
            PIXEL33_50
          }
          if (Diff(w[8], w[4]))
          {
            PIXEL30_0
          }
          else
          {
            PIXEL30_20
          }
          PIXEL31_0
          break;
        }
        case 239:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_0
          }
          else
          {
            PIXEL00_20
          }
          PIXEL01_0
          PIXEL02_32
          PIXEL03_82
          PIXEL10_0
          PIXEL11_0
          PIXEL12_32
          PIXEL13_82
          PIXEL20_0
          PIXEL21_0
          PIXEL22_31
          PIXEL23_81
          if (Diff(w[8], w[4]))
          {
            PIXEL30_0
          }
          else
          {
            PIXEL30_20
          }
          PIXEL31_0
          PIXEL32_31
          PIXEL33_81
          break;
        }
        case 127:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_0
          }
          else
          {
            PIXEL00_20
          }
          PIXEL01_0
          if (Diff(w[2], w[6]))
          {
            PIXEL02_0
            PIXEL03_0
            PIXEL13_0
          }
          else
          {
            PIXEL02_50
            PIXEL03_50
            PIXEL13_50
          }
          PIXEL10_0
          PIXEL11_0
          PIXEL12_0
          if (Diff(w[8], w[4]))
          {
            PIXEL20_0
            PIXEL30_0
            PIXEL31_0
          }
          else
          {
            PIXEL20_50
            PIXEL30_50
            PIXEL31_50
          }
          PIXEL21_0
          PIXEL22_30
          PIXEL23_10
          PIXEL32_10
          PIXEL33_80
          break;
        }
        case 191:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_0
          }
          else
          {
            PIXEL00_20
          }
          PIXEL01_0
          PIXEL02_0
          if (Diff(w[2], w[6]))
          {
            PIXEL03_0
          }
          else
          {
            PIXEL03_20
          }
          PIXEL10_0
          PIXEL11_0
          PIXEL12_0
          PIXEL13_0
          PIXEL20_31
          PIXEL21_31
          PIXEL22_32
          PIXEL23_32
          PIXEL30_81
          PIXEL31_81
          PIXEL32_82
          PIXEL33_82
          break;
        }
        case 223:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_0
            PIXEL01_0
            PIXEL10_0
          }
          else
          {
            PIXEL00_50
            PIXEL01_50
            PIXEL10_50
          }
          PIXEL02_0
          if (Diff(w[2], w[6]))
          {
            PIXEL03_0
          }
          else
          {
            PIXEL03_20
          }
          PIXEL11_0
          PIXEL12_0
          PIXEL13_0
          PIXEL20_10
          PIXEL21_30
          PIXEL22_0
          if (Diff(w[6], w[8]))
          {
            PIXEL23_0
            PIXEL32_0
            PIXEL33_0
          }
          else
          {
            PIXEL23_50
            PIXEL32_50
            PIXEL33_50
          }
          PIXEL30_80
          PIXEL31_10
          break;
        }
        case 247:
        {
          PIXEL00_81
          PIXEL01_31
          PIXEL02_0
          if (Diff(w[2], w[6]))
          {
            PIXEL03_0
          }
          else
          {
            PIXEL03_20
          }
          PIXEL10_81
          PIXEL11_31
          PIXEL12_0
          PIXEL13_0
          PIXEL20_82
          PIXEL21_32
          PIXEL22_0
          PIXEL23_0
          PIXEL30_82
          PIXEL31_32
          PIXEL32_0
          if (Diff(w[6], w[8]))
          {
            PIXEL33_0
          }
          else
          {
            PIXEL33_20
          }
          break;
        }
        case 255:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_0
          }
          else
          {
            PIXEL00_20
          }
          PIXEL01_0
          PIXEL02_0
          if (Diff(w[2], w[6]))
          {
            PIXEL03_0
          }
          else
          {
            PIXEL03_20
          }
          PIXEL10_0
          PIXEL11_0
          PIXEL12_0
          PIXEL13_0
          PIXEL20_0
          PIXEL21_0
          PIXEL22_0
          PIXEL23_0
          if (Diff(w[8], w[4]))
          {
            PIXEL30_0
          }
          else
          {
            PIXEL30_20
          }
          PIXEL31_0
          PIXEL32_0
          if (Diff(w[6], w[8]))
          {
            PIXEL33_0
          }
          else
          {
            PIXEL33_20
          }
          break;
        }
      }
      pIn += sizeof(PIXEL_TYPE);
      /* move by 4 pixels each time */
      pOut += sizeof(PIXEL_TYPE) * 4;
    }
  }
}

#undef PIXEL00_0    
#undef PIXEL00_11   
#undef PIXEL00_12   
#undef PIXEL00_20   
#undef PIXEL00_50   
#undef PIXEL00_80   
#undef PIXEL00_81   
#undef PIXEL00_82   
#undef PIXEL01_0    
#undef PIXEL01_10   
#undef PIXEL01_12   
#undef PIXEL01_14   
#undef PIXEL01_21   
#undef PIXEL01_31   
#undef PIXEL01_50   
#undef PIXEL01_60   
#undef PIXEL01_61   
#undef PIXEL01_82   
#undef PIXEL01_83   
#undef PIXEL02_0    
#undef PIXEL02_10   
#undef PIXEL02_11   
#undef PIXEL02_13   
#undef PIXEL02_21   
#undef PIXEL02_32   
#undef PIXEL02_50   
#undef PIXEL02_60   
#undef PIXEL02_61   
#undef PIXEL02_81   
#undef PIXEL02_83   
#undef PIXEL03_0    
#undef PIXEL03_11   
#undef PIXEL03_12   
#undef PIXEL03_20   
#undef PIXEL03_50   
#undef PIXEL03_80   
#undef PIXEL03_81   
#undef PIXEL03_82   
#undef PIXEL10_0    
#undef PIXEL10_10   
#undef PIXEL10_11   
#undef PIXEL10_13   
#undef PIXEL10_21   
#undef PIXEL10_32   
#undef PIXEL10_50   
#undef PIXEL10_60   
#undef PIXEL10_61   
#undef PIXEL10_81   
#undef PIXEL10_83   
#undef PIXEL11_0    
#undef PIXEL11_30   
#undef PIXEL11_31   
#undef PIXEL11_32   
#undef PIXEL11_70   
#undef PIXEL12_0    
#undef PIXEL12_30   
#undef PIXEL12_31   
#undef PIXEL12_32   
#undef PIXEL12_70   
#undef PIXEL13_0    
#undef PIXEL13_10   
#undef PIXEL13_12   
#undef PIXEL13_14   
#undef PIXEL13_21   
#undef PIXEL13_31   
#undef PIXEL13_50   
#undef PIXEL13_60   
#undef PIXEL13_61   
#undef PIXEL13_82   
#undef PIXEL13_83   
#undef PIXEL20_0    
#undef PIXEL20_10   
#undef PIXEL20_12   
#undef PIXEL20_14   
#undef PIXEL20_21   
#undef PIXEL20_31   
#undef PIXEL20_50   
#undef PIXEL20_60   
#undef PIXEL20_61   
#undef PIXEL20_82   
#undef PIXEL20_83   
#undef PIXEL21_0    
#undef PIXEL21_30   
#undef PIXEL21_31   
#undef PIXEL21_32   
#undef PIXEL21_70   
#undef PIXEL22_0    
#undef PIXEL22_30   
#undef PIXEL22_31   
#undef PIXEL22_32   
#undef PIXEL22_70   
#undef PIXEL23_0    
#undef PIXEL23_10   
#undef PIXEL23_11   
#undef PIXEL23_13   
#undef PIXEL23_21   
#undef PIXEL23_32   
#undef PIXEL23_50   
#undef PIXEL23_60   
#undef PIXEL23_61   
#undef PIXEL23_81   
#undef PIXEL23_83   
#undef PIXEL30_0    
#undef PIXEL30_11   
#undef PIXEL30_12   
#undef PIXEL30_20   
#undef PIXEL30_50   
#undef PIXEL30_80   
#undef PIXEL30_81   
#undef PIXEL30_82   
#undef PIXEL31_0    
#undef PIXEL31_10   
#undef PIXEL31_11   
#undef PIXEL31_13   
#undef PIXEL31_21   
#undef PIXEL31_32   
#undef PIXEL31_50   
#undef PIXEL31_60   
#undef PIXEL31_61   
#undef PIXEL31_81   
#undef PIXEL31_83   
#undef PIXEL32_0    
#undef PIXEL32_10   
#undef PIXEL32_12   
#undef PIXEL32_14   
#undef PIXEL32_21   
#undef PIXEL32_31   
#undef PIXEL32_50   
#undef PIXEL32_60   
#undef PIXEL32_61   
#undef PIXEL32_82   
#undef PIXEL32_83   
#undef PIXEL33_0    
#undef PIXEL33_11   
#undef PIXEL33_12   
#undef PIXEL33_20   
#undef PIXEL33_50   
#undef PIXEL33_80   
#undef PIXEL33_81   
#undef PIXEL33_82   

void hq4x(SDL_Surface * input, SDL_Surface * output){
    InitLUTs();
    hq4x_16((unsigned char*) input->pixels, (unsigned char*) output->pixels,
            input->w, input->h, input->pitch, output->pitch);
}

//hq3x filter demo program
//----------------------------------------------------------
//Copyright (C) 2003 MaxSt ( maxst@hiend3d.com )

//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU Lesser General Public
//License as published by the Free Software Foundation; either
//version 2.1 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
//Lesser General Public License for more details.
//
//You should have received a copy of the GNU Lesser General Public
//License along with this program; if not, write to the Free Software
//Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

/*
static int   LUT16to32[65536];
static int   YUV1, YUV2;
const  int   Ymask = 0x00FF0000;
const  int   Umask = 0x0000FF00;
const  int   Vmask = 0x000000FF;
const  int   trY   = 0x00300000;
const  int   trU   = 0x00000700;
const  int   trV   = 0x00000006;

inline void Interp1(unsigned char * pc, int c1, int c2){
  *((int*)pc) = (c1*3+c2) >> 2;
}

inline void Interp2(unsigned char * pc, int c1, int c2, int c3)
{
  *((int*)pc) = (c1*2+c2+c3) >> 2;
}

inline void Interp3(unsigned char * pc, int c1, int c2)
{
  *((int*)pc) = (c1*7+c2)/8;

  *((int*)pc) = ((((c1 & 0x00FF00)*7 + (c2 & 0x00FF00) ) & 0x0007F800) +
                 (((c1 & 0xFF00FF)*7 + (c2 & 0xFF00FF) ) & 0x07F807F8)) >> 3;
}

inline void Interp5(unsigned char * pc, int c1, int c2)
{
  *((int*)pc) = (c1+c2) >> 1;
}
*/


#define PIXEL00_1M  Interp1(pOut, c[5], c[1]);
#define PIXEL00_1U  Interp1(pOut, c[5], c[2]);
#define PIXEL00_1L  Interp1(pOut, c[5], c[4]);
#define PIXEL00_2   Interp2(pOut, c[5], c[4], c[2]);
#define PIXEL00_4   Interp4(pOut, c[5], c[4], c[2]);
#define PIXEL00_5   Interp5(pOut, c[4], c[2]);
#define PIXEL00_C   *((PIXEL_TYPE*)(pOut))   = c[5];

#define PIXEL01_1   Interp1(pOut+sizeof(PIXEL_TYPE), c[5], c[2]);
#define PIXEL01_3   Interp3(pOut+sizeof(PIXEL_TYPE), c[5], c[2]);
#define PIXEL01_6   Interp1(pOut+sizeof(PIXEL_TYPE), c[2], c[5]);
#define PIXEL01_C   *((PIXEL_TYPE*)(pOut+sizeof(PIXEL_TYPE))) = c[5];

#define PIXEL02_1M  Interp1(pOut+sizeof(PIXEL_TYPE)*2, c[5], c[3]);
#define PIXEL02_1U  Interp1(pOut+sizeof(PIXEL_TYPE)*2, c[5], c[2]);
#define PIXEL02_1R  Interp1(pOut+sizeof(PIXEL_TYPE)*2, c[5], c[6]);
#define PIXEL02_2   Interp2(pOut+sizeof(PIXEL_TYPE)*2, c[5], c[2], c[6]);
#define PIXEL02_4   Interp4(pOut+sizeof(PIXEL_TYPE)*2, c[5], c[2], c[6]);
#define PIXEL02_5   Interp5(pOut+sizeof(PIXEL_TYPE)*2, c[2], c[6]);
#define PIXEL02_C   *((PIXEL_TYPE*)(pOut+sizeof(PIXEL_TYPE)*2)) = c[5];

#define PIXEL10_1   Interp1(pOut+BpL, c[5], c[4]);
#define PIXEL10_3   Interp3(pOut+BpL, c[5], c[4]);
#define PIXEL10_6   Interp1(pOut+BpL, c[4], c[5]);
#define PIXEL10_C   *((PIXEL_TYPE*)(pOut+BpL)) = c[5];

#define PIXEL11     *((PIXEL_TYPE*)(pOut+BpL+sizeof(PIXEL_TYPE))) = c[5];

#define PIXEL12_1   Interp1(pOut+BpL+sizeof(PIXEL_TYPE)*2, c[5], c[6]);
#define PIXEL12_3   Interp3(pOut+BpL+sizeof(PIXEL_TYPE)*2, c[5], c[6]);
#define PIXEL12_6   Interp1(pOut+BpL+sizeof(PIXEL_TYPE)*2, c[6], c[5]);
#define PIXEL12_C   *((PIXEL_TYPE*)(pOut+BpL+sizeof(PIXEL_TYPE)*2)) = c[5];

#define PIXEL20_1M  Interp1(pOut+BpL+BpL, c[5], c[7]);
#define PIXEL20_1D  Interp1(pOut+BpL+BpL, c[5], c[8]);
#define PIXEL20_1L  Interp1(pOut+BpL+BpL, c[5], c[4]);
#define PIXEL20_2   Interp2(pOut+BpL+BpL, c[5], c[8], c[4]);
#define PIXEL20_4   Interp4(pOut+BpL+BpL, c[5], c[8], c[4]);
#define PIXEL20_5   Interp5(pOut+BpL+BpL, c[8], c[4]);
#define PIXEL20_C   *((PIXEL_TYPE*)(pOut+BpL+BpL)) = c[5];

#define PIXEL21_1   Interp1(pOut+BpL+BpL+sizeof(PIXEL_TYPE), c[5], c[8]);
#define PIXEL21_3   Interp3(pOut+BpL+BpL+sizeof(PIXEL_TYPE), c[5], c[8]);
#define PIXEL21_6   Interp1(pOut+BpL+BpL+sizeof(PIXEL_TYPE), c[8], c[5]);
#define PIXEL21_C   *((PIXEL_TYPE*)(pOut+BpL+BpL+sizeof(PIXEL_TYPE))) = c[5];

#define PIXEL22_1M  Interp1(pOut+BpL+BpL+sizeof(PIXEL_TYPE)*2, c[5], c[9]);
#define PIXEL22_1D  Interp1(pOut+BpL+BpL+sizeof(PIXEL_TYPE)*2, c[5], c[8]);
#define PIXEL22_1R  Interp1(pOut+BpL+BpL+sizeof(PIXEL_TYPE)*2, c[5], c[6]);
#define PIXEL22_2   Interp2(pOut+BpL+BpL+sizeof(PIXEL_TYPE)*2, c[5], c[6], c[8]);
#define PIXEL22_4   Interp4(pOut+BpL+BpL+sizeof(PIXEL_TYPE)*2, c[5], c[6], c[8]);
#define PIXEL22_5   Interp5(pOut+BpL+BpL+sizeof(PIXEL_TYPE)*2, c[6], c[8]);
#define PIXEL22_C   *((PIXEL_TYPE*)(pOut+BpL+BpL+sizeof(PIXEL_TYPE)*2)) = c[5];

/*
inline bool Diff(unsigned int w1, unsigned int w2){
  YUV1 = RGBtoYUV[w1];
  YUV2 = RGBtoYUV[w2];
  return ( ( abs((YUV1 & Ymask) - (YUV2 & Ymask)) > trY ) ||
           ( abs((YUV1 & Umask) - (YUV2 & Umask)) > trU ) ||
           ( abs((YUV1 & Vmask) - (YUV2 & Vmask)) > trV ) );
}
*/

void hq3x_16(unsigned char * input, unsigned char * output, int Xres, int Yres, int inputPitch, int outputPitch){
  //   +----+----+----+
  //   |    |    |    |
  //   | w1 | w2 | w3 |
  //   +----+----+----+
  //   |    |    |    |
  //   | w4 | w5 | w6 |
  //   +----+----+----+
  //   |    |    |    |
  //   | w7 | w8 | w9 |
  //   +----+----+----+

  const int BpL = outputPitch;
  for (int j=0; j<Yres; j++){
      int prevline, nextline;
    if (j>0)      prevline = -inputPitch; else prevline = 0;
    if (j<Yres-1) nextline =  inputPitch; else nextline = 0;

    unsigned char * pIn = input + j * inputPitch;
    /* move by 3 lines each time */
    unsigned char * pOut = output + j * outputPitch * 3;

    for (int i=0; i<Xres; i++){
        int  w[10];
        int  c[10];
      w[2] = *((unsigned short*)(pIn + prevline));
      w[5] = *((unsigned short*)pIn);
      w[8] = *((unsigned short*)(pIn + nextline));

      if (i>0)
      {
        w[1] = *((unsigned short*)(pIn + prevline - 2));
        w[4] = *((unsigned short*)(pIn - 2));
        w[7] = *((unsigned short*)(pIn + nextline - 2));
      }
      else
      {
        w[1] = w[2];
        w[4] = w[5];
        w[7] = w[8];
      }

      if (i<Xres-1)
      {
        w[3] = *((unsigned short*)(pIn + prevline + 2));
        w[6] = *((unsigned short*)(pIn + 2));
        w[9] = *((unsigned short*)(pIn + nextline + 2));
      }
      else
      {
        w[3] = w[2];
        w[6] = w[5];
        w[9] = w[8];
      }

      int pattern = 0;
      int flag = 1;

      YUV1 = RGBtoYUV[w[5]];

      for (int k=1; k<=9; k++)
      {
        if (k==5) continue;

        if ( w[k] != w[5] )
        {
          YUV2 = RGBtoYUV[w[k]];
          if ( ( abs((YUV1 & Ymask) - (YUV2 & Ymask)) > trY ) ||
               ( abs((YUV1 & Umask) - (YUV2 & Umask)) > trU ) ||
               ( abs((YUV1 & Vmask) - (YUV2 & Vmask)) > trV ) )
            pattern |= flag;
        }
        flag <<= 1;
      }

      for (int k=1; k<=9; k++)
        c[k] = w[k];

      switch (pattern){
        case 0:
        case 1:
        case 4:
        case 32:
        case 128:
        case 5:
        case 132:
        case 160:
        case 33:
        case 129:
        case 36:
        case 133:
        case 164:
        case 161:
        case 37:
        case 165:
        {
          PIXEL00_2
          PIXEL01_1
          PIXEL02_2
          PIXEL10_1
          PIXEL11
          PIXEL12_1
          PIXEL20_2
          PIXEL21_1
          PIXEL22_2
          break;
        }
        case 2:
        case 34:
        case 130:
        case 162:
        {
          PIXEL00_1M
          PIXEL01_C
          PIXEL02_1M
          PIXEL10_1
          PIXEL11
          PIXEL12_1
          PIXEL20_2
          PIXEL21_1
          PIXEL22_2
          break;
        }
        case 16:
        case 17:
        case 48:
        case 49:
        {
          PIXEL00_2
          PIXEL01_1
          PIXEL02_1M
          PIXEL10_1
          PIXEL11
          PIXEL12_C
          PIXEL20_2
          PIXEL21_1
          PIXEL22_1M
          break;
        }
        case 64:
        case 65:
        case 68:
        case 69:
        {
          PIXEL00_2
          PIXEL01_1
          PIXEL02_2
          PIXEL10_1
          PIXEL11
          PIXEL12_1
          PIXEL20_1M
          PIXEL21_C
          PIXEL22_1M
          break;
        }
        case 8:
        case 12:
        case 136:
        case 140:
        {
          PIXEL00_1M
          PIXEL01_1
          PIXEL02_2
          PIXEL10_C
          PIXEL11
          PIXEL12_1
          PIXEL20_1M
          PIXEL21_1
          PIXEL22_2
          break;
        }
        case 3:
        case 35:
        case 131:
        case 163:
        {
          PIXEL00_1L
          PIXEL01_C
          PIXEL02_1M
          PIXEL10_1
          PIXEL11
          PIXEL12_1
          PIXEL20_2
          PIXEL21_1
          PIXEL22_2
          break;
        }
        case 6:
        case 38:
        case 134:
        case 166:
        {
          PIXEL00_1M
          PIXEL01_C
          PIXEL02_1R
          PIXEL10_1
          PIXEL11
          PIXEL12_1
          PIXEL20_2
          PIXEL21_1
          PIXEL22_2
          break;
        }
        case 20:
        case 21:
        case 52:
        case 53:
        {
          PIXEL00_2
          PIXEL01_1
          PIXEL02_1U
          PIXEL10_1
          PIXEL11
          PIXEL12_C
          PIXEL20_2
          PIXEL21_1
          PIXEL22_1M
          break;
        }
        case 144:
        case 145:
        case 176:
        case 177:
        {
          PIXEL00_2
          PIXEL01_1
          PIXEL02_1M
          PIXEL10_1
          PIXEL11
          PIXEL12_C
          PIXEL20_2
          PIXEL21_1
          PIXEL22_1D
          break;
        }
        case 192:
        case 193:
        case 196:
        case 197:
        {
          PIXEL00_2
          PIXEL01_1
          PIXEL02_2
          PIXEL10_1
          PIXEL11
          PIXEL12_1
          PIXEL20_1M
          PIXEL21_C
          PIXEL22_1R
          break;
        }
        case 96:
        case 97:
        case 100:
        case 101:
        {
          PIXEL00_2
          PIXEL01_1
          PIXEL02_2
          PIXEL10_1
          PIXEL11
          PIXEL12_1
          PIXEL20_1L
          PIXEL21_C
          PIXEL22_1M
          break;
        }
        case 40:
        case 44:
        case 168:
        case 172:
        {
          PIXEL00_1M
          PIXEL01_1
          PIXEL02_2
          PIXEL10_C
          PIXEL11
          PIXEL12_1
          PIXEL20_1D
          PIXEL21_1
          PIXEL22_2
          break;
        }
        case 9:
        case 13:
        case 137:
        case 141:
        {
          PIXEL00_1U
          PIXEL01_1
          PIXEL02_2
          PIXEL10_C
          PIXEL11
          PIXEL12_1
          PIXEL20_1M
          PIXEL21_1
          PIXEL22_2
          break;
        }
        case 18:
        case 50:
        {
          PIXEL00_1M
          if (Diff(w[2], w[6]))
          {
            PIXEL01_C
            PIXEL02_1M
            PIXEL12_C
          }
          else
          {
            PIXEL01_3
            PIXEL02_4
            PIXEL12_3
          }
          PIXEL10_1
          PIXEL11
          PIXEL20_2
          PIXEL21_1
          PIXEL22_1M
          break;
        }
        case 80:
        case 81:
        {
          PIXEL00_2
          PIXEL01_1
          PIXEL02_1M
          PIXEL10_1
          PIXEL11
          PIXEL20_1M
          if (Diff(w[6], w[8]))
          {
            PIXEL12_C
            PIXEL21_C
            PIXEL22_1M
          }
          else
          {
            PIXEL12_3
            PIXEL21_3
            PIXEL22_4
          }
          break;
        }
        case 72:
        case 76:
        {
          PIXEL00_1M
          PIXEL01_1
          PIXEL02_2
          PIXEL11
          PIXEL12_1
          if (Diff(w[8], w[4]))
          {
            PIXEL10_C
            PIXEL20_1M
            PIXEL21_C
          }
          else
          {
            PIXEL10_3
            PIXEL20_4
            PIXEL21_3
          }
          PIXEL22_1M
          break;
        }
        case 10:
        case 138:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_1M
            PIXEL01_C
            PIXEL10_C
          }
          else
          {
            PIXEL00_4
            PIXEL01_3
            PIXEL10_3
          }
          PIXEL02_1M
          PIXEL11
          PIXEL12_1
          PIXEL20_1M
          PIXEL21_1
          PIXEL22_2
          break;
        }
        case 66:
        {
          PIXEL00_1M
          PIXEL01_C
          PIXEL02_1M
          PIXEL10_1
          PIXEL11
          PIXEL12_1
          PIXEL20_1M
          PIXEL21_C
          PIXEL22_1M
          break;
        }
        case 24:
        {
          PIXEL00_1M
          PIXEL01_1
          PIXEL02_1M
          PIXEL10_C
          PIXEL11
          PIXEL12_C
          PIXEL20_1M
          PIXEL21_1
          PIXEL22_1M
          break;
        }
        case 7:
        case 39:
        case 135:
        {
          PIXEL00_1L
          PIXEL01_C
          PIXEL02_1R
          PIXEL10_1
          PIXEL11
          PIXEL12_1
          PIXEL20_2
          PIXEL21_1
          PIXEL22_2
          break;
        }
        case 148:
        case 149:
        case 180:
        {
          PIXEL00_2
          PIXEL01_1
          PIXEL02_1U
          PIXEL10_1
          PIXEL11
          PIXEL12_C
          PIXEL20_2
          PIXEL21_1
          PIXEL22_1D
          break;
        }
        case 224:
        case 228:
        case 225:
        {
          PIXEL00_2
          PIXEL01_1
          PIXEL02_2
          PIXEL10_1
          PIXEL11
          PIXEL12_1
          PIXEL20_1L
          PIXEL21_C
          PIXEL22_1R
          break;
        }
        case 41:
        case 169:
        case 45:
        {
          PIXEL00_1U
          PIXEL01_1
          PIXEL02_2
          PIXEL10_C
          PIXEL11
          PIXEL12_1
          PIXEL20_1D
          PIXEL21_1
          PIXEL22_2
          break;
        }
        case 22:
        case 54:
        {
          PIXEL00_1M
          if (Diff(w[2], w[6]))
          {
            PIXEL01_C
            PIXEL02_C
            PIXEL12_C
          }
          else
          {
            PIXEL01_3
            PIXEL02_4
            PIXEL12_3
          }
          PIXEL10_1
          PIXEL11
          PIXEL20_2
          PIXEL21_1
          PIXEL22_1M
          break;
        }
        case 208:
        case 209:
        {
          PIXEL00_2
          PIXEL01_1
          PIXEL02_1M
          PIXEL10_1
          PIXEL11
          PIXEL20_1M
          if (Diff(w[6], w[8]))
          {
            PIXEL12_C
            PIXEL21_C
            PIXEL22_C
          }
          else
          {
            PIXEL12_3
            PIXEL21_3
            PIXEL22_4
          }
          break;
        }
        case 104:
        case 108:
        {
          PIXEL00_1M
          PIXEL01_1
          PIXEL02_2
          PIXEL11
          PIXEL12_1
          if (Diff(w[8], w[4]))
          {
            PIXEL10_C
            PIXEL20_C
            PIXEL21_C
          }
          else
          {
            PIXEL10_3
            PIXEL20_4
            PIXEL21_3
          }
          PIXEL22_1M
          break;
        }
        case 11:
        case 139:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_C
            PIXEL01_C
            PIXEL10_C
          }
          else
          {
            PIXEL00_4
            PIXEL01_3
            PIXEL10_3
          }
          PIXEL02_1M
          PIXEL11
          PIXEL12_1
          PIXEL20_1M
          PIXEL21_1
          PIXEL22_2
          break;
        }
        case 19:
        case 51:
        {
          if (Diff(w[2], w[6]))
          {
            PIXEL00_1L
            PIXEL01_C
            PIXEL02_1M
            PIXEL12_C
          }
          else
          {
            PIXEL00_2
            PIXEL01_6
            PIXEL02_5
            PIXEL12_1
          }
          PIXEL10_1
          PIXEL11
          PIXEL20_2
          PIXEL21_1
          PIXEL22_1M
          break;
        }
        case 146:
        case 178:
        {
          if (Diff(w[2], w[6]))
          {
            PIXEL01_C
            PIXEL02_1M
            PIXEL12_C
            PIXEL22_1D
          }
          else
          {
            PIXEL01_1
            PIXEL02_5
            PIXEL12_6
            PIXEL22_2
          }
          PIXEL00_1M
          PIXEL10_1
          PIXEL11
          PIXEL20_2
          PIXEL21_1
          break;
        }
        case 84:
        case 85:
        {
          if (Diff(w[6], w[8]))
          {
            PIXEL02_1U
            PIXEL12_C
            PIXEL21_C
            PIXEL22_1M
          }
          else
          {
            PIXEL02_2
            PIXEL12_6
            PIXEL21_1
            PIXEL22_5
          }
          PIXEL00_2
          PIXEL01_1
          PIXEL10_1
          PIXEL11
          PIXEL20_1M
          break;
        }
        case 112:
        case 113:
        {
          if (Diff(w[6], w[8]))
          {
            PIXEL12_C
            PIXEL20_1L
            PIXEL21_C
            PIXEL22_1M
          }
          else
          {
            PIXEL12_1
            PIXEL20_2
            PIXEL21_6
            PIXEL22_5
          }
          PIXEL00_2
          PIXEL01_1
          PIXEL02_1M
          PIXEL10_1
          PIXEL11
          break;
        }
        case 200:
        case 204:
        {
          if (Diff(w[8], w[4]))
          {
            PIXEL10_C
            PIXEL20_1M
            PIXEL21_C
            PIXEL22_1R
          }
          else
          {
            PIXEL10_1
            PIXEL20_5
            PIXEL21_6
            PIXEL22_2
          }
          PIXEL00_1M
          PIXEL01_1
          PIXEL02_2
          PIXEL11
          PIXEL12_1
          break;
        }
        case 73:
        case 77:
        {
          if (Diff(w[8], w[4]))
          {
            PIXEL00_1U
            PIXEL10_C
            PIXEL20_1M
            PIXEL21_C
          }
          else
          {
            PIXEL00_2
            PIXEL10_6
            PIXEL20_5
            PIXEL21_1
          }
          PIXEL01_1
          PIXEL02_2
          PIXEL11
          PIXEL12_1
          PIXEL22_1M
          break;
        }
        case 42:
        case 170:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_1M
            PIXEL01_C
            PIXEL10_C
            PIXEL20_1D
          }
          else
          {
            PIXEL00_5
            PIXEL01_1
            PIXEL10_6
            PIXEL20_2
          }
          PIXEL02_1M
          PIXEL11
          PIXEL12_1
          PIXEL21_1
          PIXEL22_2
          break;
        }
        case 14:
        case 142:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_1M
            PIXEL01_C
            PIXEL02_1R
            PIXEL10_C
          }
          else
          {
            PIXEL00_5
            PIXEL01_6
            PIXEL02_2
            PIXEL10_1
          }
          PIXEL11
          PIXEL12_1
          PIXEL20_1M
          PIXEL21_1
          PIXEL22_2
          break;
        }
        case 67:
        {
          PIXEL00_1L
          PIXEL01_C
          PIXEL02_1M
          PIXEL10_1
          PIXEL11
          PIXEL12_1
          PIXEL20_1M
          PIXEL21_C
          PIXEL22_1M
          break;
        }
        case 70:
        {
          PIXEL00_1M
          PIXEL01_C
          PIXEL02_1R
          PIXEL10_1
          PIXEL11
          PIXEL12_1
          PIXEL20_1M
          PIXEL21_C
          PIXEL22_1M
          break;
        }
        case 28:
        {
          PIXEL00_1M
          PIXEL01_1
          PIXEL02_1U
          PIXEL10_C
          PIXEL11
          PIXEL12_C
          PIXEL20_1M
          PIXEL21_1
          PIXEL22_1M
          break;
        }
        case 152:
        {
          PIXEL00_1M
          PIXEL01_1
          PIXEL02_1M
          PIXEL10_C
          PIXEL11
          PIXEL12_C
          PIXEL20_1M
          PIXEL21_1
          PIXEL22_1D
          break;
        }
        case 194:
        {
          PIXEL00_1M
          PIXEL01_C
          PIXEL02_1M
          PIXEL10_1
          PIXEL11
          PIXEL12_1
          PIXEL20_1M
          PIXEL21_C
          PIXEL22_1R
          break;
        }
        case 98:
        {
          PIXEL00_1M
          PIXEL01_C
          PIXEL02_1M
          PIXEL10_1
          PIXEL11
          PIXEL12_1
          PIXEL20_1L
          PIXEL21_C
          PIXEL22_1M
          break;
        }
        case 56:
        {
          PIXEL00_1M
          PIXEL01_1
          PIXEL02_1M
          PIXEL10_C
          PIXEL11
          PIXEL12_C
          PIXEL20_1D
          PIXEL21_1
          PIXEL22_1M
          break;
        }
        case 25:
        {
          PIXEL00_1U
          PIXEL01_1
          PIXEL02_1M
          PIXEL10_C
          PIXEL11
          PIXEL12_C
          PIXEL20_1M
          PIXEL21_1
          PIXEL22_1M
          break;
        }
        case 26:
        case 31:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_C
            PIXEL10_C
          }
          else
          {
            PIXEL00_4
            PIXEL10_3
          }
          PIXEL01_C
          if (Diff(w[2], w[6]))
          {
            PIXEL02_C
            PIXEL12_C
          }
          else
          {
            PIXEL02_4
            PIXEL12_3
          }
          PIXEL11
          PIXEL20_1M
          PIXEL21_1
          PIXEL22_1M
          break;
        }
        case 82:
        case 214:
        {
          PIXEL00_1M
          if (Diff(w[2], w[6]))
          {
            PIXEL01_C
            PIXEL02_C
          }
          else
          {
            PIXEL01_3
            PIXEL02_4
          }
          PIXEL10_1
          PIXEL11
          PIXEL12_C
          PIXEL20_1M
          if (Diff(w[6], w[8]))
          {
            PIXEL21_C
            PIXEL22_C
          }
          else
          {
            PIXEL21_3
            PIXEL22_4
          }
          break;
        }
        case 88:
        case 248:
        {
          PIXEL00_1M
          PIXEL01_1
          PIXEL02_1M
          PIXEL11
          if (Diff(w[8], w[4]))
          {
            PIXEL10_C
            PIXEL20_C
          }
          else
          {
            PIXEL10_3
            PIXEL20_4
          }
          PIXEL21_C
          if (Diff(w[6], w[8]))
          {
            PIXEL12_C
            PIXEL22_C
          }
          else
          {
            PIXEL12_3
            PIXEL22_4
          }
          break;
        }
        case 74:
        case 107:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_C
            PIXEL01_C
          }
          else
          {
            PIXEL00_4
            PIXEL01_3
          }
          PIXEL02_1M
          PIXEL10_C
          PIXEL11
          PIXEL12_1
          if (Diff(w[8], w[4]))
          {
            PIXEL20_C
            PIXEL21_C
          }
          else
          {
            PIXEL20_4
            PIXEL21_3
          }
          PIXEL22_1M
          break;
        }
        case 27:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_C
            PIXEL01_C
            PIXEL10_C
          }
          else
          {
            PIXEL00_4
            PIXEL01_3
            PIXEL10_3
          }
          PIXEL02_1M
          PIXEL11
          PIXEL12_C
          PIXEL20_1M
          PIXEL21_1
          PIXEL22_1M
          break;
        }
        case 86:
        {
          PIXEL00_1M
          if (Diff(w[2], w[6]))
          {
            PIXEL01_C
            PIXEL02_C
            PIXEL12_C
          }
          else
          {
            PIXEL01_3
            PIXEL02_4
            PIXEL12_3
          }
          PIXEL10_1
          PIXEL11
          PIXEL20_1M
          PIXEL21_C
          PIXEL22_1M
          break;
        }
        case 216:
        {
          PIXEL00_1M
          PIXEL01_1
          PIXEL02_1M
          PIXEL10_C
          PIXEL11
          PIXEL20_1M
          if (Diff(w[6], w[8]))
          {
            PIXEL12_C
            PIXEL21_C
            PIXEL22_C
          }
          else
          {
            PIXEL12_3
            PIXEL21_3
            PIXEL22_4
          }
          break;
        }
        case 106:
        {
          PIXEL00_1M
          PIXEL01_C
          PIXEL02_1M
          PIXEL11
          PIXEL12_1
          if (Diff(w[8], w[4]))
          {
            PIXEL10_C
            PIXEL20_C
            PIXEL21_C
          }
          else
          {
            PIXEL10_3
            PIXEL20_4
            PIXEL21_3
          }
          PIXEL22_1M
          break;
        }
        case 30:
        {
          PIXEL00_1M
          if (Diff(w[2], w[6]))
          {
            PIXEL01_C
            PIXEL02_C
            PIXEL12_C
          }
          else
          {
            PIXEL01_3
            PIXEL02_4
            PIXEL12_3
          }
          PIXEL10_C
          PIXEL11
          PIXEL20_1M
          PIXEL21_1
          PIXEL22_1M
          break;
        }
        case 210:
        {
          PIXEL00_1M
          PIXEL01_C
          PIXEL02_1M
          PIXEL10_1
          PIXEL11
          PIXEL20_1M
          if (Diff(w[6], w[8]))
          {
            PIXEL12_C
            PIXEL21_C
            PIXEL22_C
          }
          else
          {
            PIXEL12_3
            PIXEL21_3
            PIXEL22_4
          }
          break;
        }
        case 120:
        {
          PIXEL00_1M
          PIXEL01_1
          PIXEL02_1M
          PIXEL11
          PIXEL12_C
          if (Diff(w[8], w[4]))
          {
            PIXEL10_C
            PIXEL20_C
            PIXEL21_C
          }
          else
          {
            PIXEL10_3
            PIXEL20_4
            PIXEL21_3
          }
          PIXEL22_1M
          break;
        }
        case 75:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_C
            PIXEL01_C
            PIXEL10_C
          }
          else
          {
            PIXEL00_4
            PIXEL01_3
            PIXEL10_3
          }
          PIXEL02_1M
          PIXEL11
          PIXEL12_1
          PIXEL20_1M
          PIXEL21_C
          PIXEL22_1M
          break;
        }
        case 29:
        {
          PIXEL00_1U
          PIXEL01_1
          PIXEL02_1U
          PIXEL10_C
          PIXEL11
          PIXEL12_C
          PIXEL20_1M
          PIXEL21_1
          PIXEL22_1M
          break;
        }
        case 198:
        {
          PIXEL00_1M
          PIXEL01_C
          PIXEL02_1R
          PIXEL10_1
          PIXEL11
          PIXEL12_1
          PIXEL20_1M
          PIXEL21_C
          PIXEL22_1R
          break;
        }
        case 184:
        {
          PIXEL00_1M
          PIXEL01_1
          PIXEL02_1M
          PIXEL10_C
          PIXEL11
          PIXEL12_C
          PIXEL20_1D
          PIXEL21_1
          PIXEL22_1D
          break;
        }
        case 99:
        {
          PIXEL00_1L
          PIXEL01_C
          PIXEL02_1M
          PIXEL10_1
          PIXEL11
          PIXEL12_1
          PIXEL20_1L
          PIXEL21_C
          PIXEL22_1M
          break;
        }
        case 57:
        {
          PIXEL00_1U
          PIXEL01_1
          PIXEL02_1M
          PIXEL10_C
          PIXEL11
          PIXEL12_C
          PIXEL20_1D
          PIXEL21_1
          PIXEL22_1M
          break;
        }
        case 71:
        {
          PIXEL00_1L
          PIXEL01_C
          PIXEL02_1R
          PIXEL10_1
          PIXEL11
          PIXEL12_1
          PIXEL20_1M
          PIXEL21_C
          PIXEL22_1M
          break;
        }
        case 156:
        {
          PIXEL00_1M
          PIXEL01_1
          PIXEL02_1U
          PIXEL10_C
          PIXEL11
          PIXEL12_C
          PIXEL20_1M
          PIXEL21_1
          PIXEL22_1D
          break;
        }
        case 226:
        {
          PIXEL00_1M
          PIXEL01_C
          PIXEL02_1M
          PIXEL10_1
          PIXEL11
          PIXEL12_1
          PIXEL20_1L
          PIXEL21_C
          PIXEL22_1R
          break;
        }
        case 60:
        {
          PIXEL00_1M
          PIXEL01_1
          PIXEL02_1U
          PIXEL10_C
          PIXEL11
          PIXEL12_C
          PIXEL20_1D
          PIXEL21_1
          PIXEL22_1M
          break;
        }
        case 195:
        {
          PIXEL00_1L
          PIXEL01_C
          PIXEL02_1M
          PIXEL10_1
          PIXEL11
          PIXEL12_1
          PIXEL20_1M
          PIXEL21_C
          PIXEL22_1R
          break;
        }
        case 102:
        {
          PIXEL00_1M
          PIXEL01_C
          PIXEL02_1R
          PIXEL10_1
          PIXEL11
          PIXEL12_1
          PIXEL20_1L
          PIXEL21_C
          PIXEL22_1M
          break;
        }
        case 153:
        {
          PIXEL00_1U
          PIXEL01_1
          PIXEL02_1M
          PIXEL10_C
          PIXEL11
          PIXEL12_C
          PIXEL20_1M
          PIXEL21_1
          PIXEL22_1D
          break;
        }
        case 58:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_1M
          }
          else
          {
            PIXEL00_2
          }
          PIXEL01_C
          if (Diff(w[2], w[6]))
          {
            PIXEL02_1M
          }
          else
          {
            PIXEL02_2
          }
          PIXEL10_C
          PIXEL11
          PIXEL12_C
          PIXEL20_1D
          PIXEL21_1
          PIXEL22_1M
          break;
        }
        case 83:
        {
          PIXEL00_1L
          PIXEL01_C
          if (Diff(w[2], w[6]))
          {
            PIXEL02_1M
          }
          else
          {
            PIXEL02_2
          }
          PIXEL10_1
          PIXEL11
          PIXEL12_C
          PIXEL20_1M
          PIXEL21_C
          if (Diff(w[6], w[8]))
          {
            PIXEL22_1M
          }
          else
          {
            PIXEL22_2
          }
          break;
        }
        case 92:
        {
          PIXEL00_1M
          PIXEL01_1
          PIXEL02_1U
          PIXEL10_C
          PIXEL11
          PIXEL12_C
          if (Diff(w[8], w[4]))
          {
            PIXEL20_1M
          }
          else
          {
            PIXEL20_2
          }
          PIXEL21_C
          if (Diff(w[6], w[8]))
          {
            PIXEL22_1M
          }
          else
          {
            PIXEL22_2
          }
          break;
        }
        case 202:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_1M
          }
          else
          {
            PIXEL00_2
          }
          PIXEL01_C
          PIXEL02_1M
          PIXEL10_C
          PIXEL11
          PIXEL12_1
          if (Diff(w[8], w[4]))
          {
            PIXEL20_1M
          }
          else
          {
            PIXEL20_2
          }
          PIXEL21_C
          PIXEL22_1R
          break;
        }
        case 78:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_1M
          }
          else
          {
            PIXEL00_2
          }
          PIXEL01_C
          PIXEL02_1R
          PIXEL10_C
          PIXEL11
          PIXEL12_1
          if (Diff(w[8], w[4]))
          {
            PIXEL20_1M
          }
          else
          {
            PIXEL20_2
          }
          PIXEL21_C
          PIXEL22_1M
          break;
        }
        case 154:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_1M
          }
          else
          {
            PIXEL00_2
          }
          PIXEL01_C
          if (Diff(w[2], w[6]))
          {
            PIXEL02_1M
          }
          else
          {
            PIXEL02_2
          }
          PIXEL10_C
          PIXEL11
          PIXEL12_C
          PIXEL20_1M
          PIXEL21_1
          PIXEL22_1D
          break;
        }
        case 114:
        {
          PIXEL00_1M
          PIXEL01_C
          if (Diff(w[2], w[6]))
          {
            PIXEL02_1M
          }
          else
          {
            PIXEL02_2
          }
          PIXEL10_1
          PIXEL11
          PIXEL12_C
          PIXEL20_1L
          PIXEL21_C
          if (Diff(w[6], w[8]))
          {
            PIXEL22_1M
          }
          else
          {
            PIXEL22_2
          }
          break;
        }
        case 89:
        {
          PIXEL00_1U
          PIXEL01_1
          PIXEL02_1M
          PIXEL10_C
          PIXEL11
          PIXEL12_C
          if (Diff(w[8], w[4]))
          {
            PIXEL20_1M
          }
          else
          {
            PIXEL20_2
          }
          PIXEL21_C
          if (Diff(w[6], w[8]))
          {
            PIXEL22_1M
          }
          else
          {
            PIXEL22_2
          }
          break;
        }
        case 90:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_1M
          }
          else
          {
            PIXEL00_2
          }
          PIXEL01_C
          if (Diff(w[2], w[6]))
          {
            PIXEL02_1M
          }
          else
          {
            PIXEL02_2
          }
          PIXEL10_C
          PIXEL11
          PIXEL12_C
          if (Diff(w[8], w[4]))
          {
            PIXEL20_1M
          }
          else
          {
            PIXEL20_2
          }
          PIXEL21_C
          if (Diff(w[6], w[8]))
          {
            PIXEL22_1M
          }
          else
          {
            PIXEL22_2
          }
          break;
        }
        case 55:
        case 23:
        {
          if (Diff(w[2], w[6]))
          {
            PIXEL00_1L
            PIXEL01_C
            PIXEL02_C
            PIXEL12_C
          }
          else
          {
            PIXEL00_2
            PIXEL01_6
            PIXEL02_5
            PIXEL12_1
          }
          PIXEL10_1
          PIXEL11
          PIXEL20_2
          PIXEL21_1
          PIXEL22_1M
          break;
        }
        case 182:
        case 150:
        {
          if (Diff(w[2], w[6]))
          {
            PIXEL01_C
            PIXEL02_C
            PIXEL12_C
            PIXEL22_1D
          }
          else
          {
            PIXEL01_1
            PIXEL02_5
            PIXEL12_6
            PIXEL22_2
          }
          PIXEL00_1M
          PIXEL10_1
          PIXEL11
          PIXEL20_2
          PIXEL21_1
          break;
        }
        case 213:
        case 212:
        {
          if (Diff(w[6], w[8]))
          {
            PIXEL02_1U
            PIXEL12_C
            PIXEL21_C
            PIXEL22_C
          }
          else
          {
            PIXEL02_2
            PIXEL12_6
            PIXEL21_1
            PIXEL22_5
          }
          PIXEL00_2
          PIXEL01_1
          PIXEL10_1
          PIXEL11
          PIXEL20_1M
          break;
        }
        case 241:
        case 240:
        {
          if (Diff(w[6], w[8]))
          {
            PIXEL12_C
            PIXEL20_1L
            PIXEL21_C
            PIXEL22_C
          }
          else
          {
            PIXEL12_1
            PIXEL20_2
            PIXEL21_6
            PIXEL22_5
          }
          PIXEL00_2
          PIXEL01_1
          PIXEL02_1M
          PIXEL10_1
          PIXEL11
          break;
        }
        case 236:
        case 232:
        {
          if (Diff(w[8], w[4]))
          {
            PIXEL10_C
            PIXEL20_C
            PIXEL21_C
            PIXEL22_1R
          }
          else
          {
            PIXEL10_1
            PIXEL20_5
            PIXEL21_6
            PIXEL22_2
          }
          PIXEL00_1M
          PIXEL01_1
          PIXEL02_2
          PIXEL11
          PIXEL12_1
          break;
        }
        case 109:
        case 105:
        {
          if (Diff(w[8], w[4]))
          {
            PIXEL00_1U
            PIXEL10_C
            PIXEL20_C
            PIXEL21_C
          }
          else
          {
            PIXEL00_2
            PIXEL10_6
            PIXEL20_5
            PIXEL21_1
          }
          PIXEL01_1
          PIXEL02_2
          PIXEL11
          PIXEL12_1
          PIXEL22_1M
          break;
        }
        case 171:
        case 43:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_C
            PIXEL01_C
            PIXEL10_C
            PIXEL20_1D
          }
          else
          {
            PIXEL00_5
            PIXEL01_1
            PIXEL10_6
            PIXEL20_2
          }
          PIXEL02_1M
          PIXEL11
          PIXEL12_1
          PIXEL21_1
          PIXEL22_2
          break;
        }
        case 143:
        case 15:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_C
            PIXEL01_C
            PIXEL02_1R
            PIXEL10_C
          }
          else
          {
            PIXEL00_5
            PIXEL01_6
            PIXEL02_2
            PIXEL10_1
          }
          PIXEL11
          PIXEL12_1
          PIXEL20_1M
          PIXEL21_1
          PIXEL22_2
          break;
        }
        case 124:
        {
          PIXEL00_1M
          PIXEL01_1
          PIXEL02_1U
          PIXEL11
          PIXEL12_C
          if (Diff(w[8], w[4]))
          {
            PIXEL10_C
            PIXEL20_C
            PIXEL21_C
          }
          else
          {
            PIXEL10_3
            PIXEL20_4
            PIXEL21_3
          }
          PIXEL22_1M
          break;
        }
        case 203:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_C
            PIXEL01_C
            PIXEL10_C
          }
          else
          {
            PIXEL00_4
            PIXEL01_3
            PIXEL10_3
          }
          PIXEL02_1M
          PIXEL11
          PIXEL12_1
          PIXEL20_1M
          PIXEL21_C
          PIXEL22_1R
          break;
        }
        case 62:
        {
          PIXEL00_1M
          if (Diff(w[2], w[6]))
          {
            PIXEL01_C
            PIXEL02_C
            PIXEL12_C
          }
          else
          {
            PIXEL01_3
            PIXEL02_4
            PIXEL12_3
          }
          PIXEL10_C
          PIXEL11
          PIXEL20_1D
          PIXEL21_1
          PIXEL22_1M
          break;
        }
        case 211:
        {
          PIXEL00_1L
          PIXEL01_C
          PIXEL02_1M
          PIXEL10_1
          PIXEL11
          PIXEL20_1M
          if (Diff(w[6], w[8]))
          {
            PIXEL12_C
            PIXEL21_C
            PIXEL22_C
          }
          else
          {
            PIXEL12_3
            PIXEL21_3
            PIXEL22_4
          }
          break;
        }
        case 118:
        {
          PIXEL00_1M
          if (Diff(w[2], w[6]))
          {
            PIXEL01_C
            PIXEL02_C
            PIXEL12_C
          }
          else
          {
            PIXEL01_3
            PIXEL02_4
            PIXEL12_3
          }
          PIXEL10_1
          PIXEL11
          PIXEL20_1L
          PIXEL21_C
          PIXEL22_1M
          break;
        }
        case 217:
        {
          PIXEL00_1U
          PIXEL01_1
          PIXEL02_1M
          PIXEL10_C
          PIXEL11
          PIXEL20_1M
          if (Diff(w[6], w[8]))
          {
            PIXEL12_C
            PIXEL21_C
            PIXEL22_C
          }
          else
          {
            PIXEL12_3
            PIXEL21_3
            PIXEL22_4
          }
          break;
        }
        case 110:
        {
          PIXEL00_1M
          PIXEL01_C
          PIXEL02_1R
          PIXEL11
          PIXEL12_1
          if (Diff(w[8], w[4]))
          {
            PIXEL10_C
            PIXEL20_C
            PIXEL21_C
          }
          else
          {
            PIXEL10_3
            PIXEL20_4
            PIXEL21_3
          }
          PIXEL22_1M
          break;
        }
        case 155:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_C
            PIXEL01_C
            PIXEL10_C
          }
          else
          {
            PIXEL00_4
            PIXEL01_3
            PIXEL10_3
          }
          PIXEL02_1M
          PIXEL11
          PIXEL12_C
          PIXEL20_1M
          PIXEL21_1
          PIXEL22_1D
          break;
        }
        case 188:
        {
          PIXEL00_1M
          PIXEL01_1
          PIXEL02_1U
          PIXEL10_C
          PIXEL11
          PIXEL12_C
          PIXEL20_1D
          PIXEL21_1
          PIXEL22_1D
          break;
        }
        case 185:
        {
          PIXEL00_1U
          PIXEL01_1
          PIXEL02_1M
          PIXEL10_C
          PIXEL11
          PIXEL12_C
          PIXEL20_1D
          PIXEL21_1
          PIXEL22_1D
          break;
        }
        case 61:
        {
          PIXEL00_1U
          PIXEL01_1
          PIXEL02_1U
          PIXEL10_C
          PIXEL11
          PIXEL12_C
          PIXEL20_1D
          PIXEL21_1
          PIXEL22_1M
          break;
        }
        case 157:
        {
          PIXEL00_1U
          PIXEL01_1
          PIXEL02_1U
          PIXEL10_C
          PIXEL11
          PIXEL12_C
          PIXEL20_1M
          PIXEL21_1
          PIXEL22_1D
          break;
        }
        case 103:
        {
          PIXEL00_1L
          PIXEL01_C
          PIXEL02_1R
          PIXEL10_1
          PIXEL11
          PIXEL12_1
          PIXEL20_1L
          PIXEL21_C
          PIXEL22_1M
          break;
        }
        case 227:
        {
          PIXEL00_1L
          PIXEL01_C
          PIXEL02_1M
          PIXEL10_1
          PIXEL11
          PIXEL12_1
          PIXEL20_1L
          PIXEL21_C
          PIXEL22_1R
          break;
        }
        case 230:
        {
          PIXEL00_1M
          PIXEL01_C
          PIXEL02_1R
          PIXEL10_1
          PIXEL11
          PIXEL12_1
          PIXEL20_1L
          PIXEL21_C
          PIXEL22_1R
          break;
        }
        case 199:
        {
          PIXEL00_1L
          PIXEL01_C
          PIXEL02_1R
          PIXEL10_1
          PIXEL11
          PIXEL12_1
          PIXEL20_1M
          PIXEL21_C
          PIXEL22_1R
          break;
        }
        case 220:
        {
          PIXEL00_1M
          PIXEL01_1
          PIXEL02_1U
          PIXEL10_C
          PIXEL11
          if (Diff(w[8], w[4]))
          {
            PIXEL20_1M
          }
          else
          {
            PIXEL20_2
          }
          if (Diff(w[6], w[8]))
          {
            PIXEL12_C
            PIXEL21_C
            PIXEL22_C
          }
          else
          {
            PIXEL12_3
            PIXEL21_3
            PIXEL22_4
          }
          break;
        }
        case 158:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_1M
          }
          else
          {
            PIXEL00_2
          }
          if (Diff(w[2], w[6]))
          {
            PIXEL01_C
            PIXEL02_C
            PIXEL12_C
          }
          else
          {
            PIXEL01_3
            PIXEL02_4
            PIXEL12_3
          }
          PIXEL10_C
          PIXEL11
          PIXEL20_1M
          PIXEL21_1
          PIXEL22_1D
          break;
        }
        case 234:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_1M
          }
          else
          {
            PIXEL00_2
          }
          PIXEL01_C
          PIXEL02_1M
          PIXEL11
          PIXEL12_1
          if (Diff(w[8], w[4]))
          {
            PIXEL10_C
            PIXEL20_C
            PIXEL21_C
          }
          else
          {
            PIXEL10_3
            PIXEL20_4
            PIXEL21_3
          }
          PIXEL22_1R
          break;
        }
        case 242:
        {
          PIXEL00_1M
          PIXEL01_C
          if (Diff(w[2], w[6]))
          {
            PIXEL02_1M
          }
          else
          {
            PIXEL02_2
          }
          PIXEL10_1
          PIXEL11
          PIXEL20_1L
          if (Diff(w[6], w[8]))
          {
            PIXEL12_C
            PIXEL21_C
            PIXEL22_C
          }
          else
          {
            PIXEL12_3
            PIXEL21_3
            PIXEL22_4
          }
          break;
        }
        case 59:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_C
            PIXEL01_C
            PIXEL10_C
          }
          else
          {
            PIXEL00_4
            PIXEL01_3
            PIXEL10_3
          }
          if (Diff(w[2], w[6]))
          {
            PIXEL02_1M
          }
          else
          {
            PIXEL02_2
          }
          PIXEL11
          PIXEL12_C
          PIXEL20_1D
          PIXEL21_1
          PIXEL22_1M
          break;
        }
        case 121:
        {
          PIXEL00_1U
          PIXEL01_1
          PIXEL02_1M
          PIXEL11
          PIXEL12_C
          if (Diff(w[8], w[4]))
          {
            PIXEL10_C
            PIXEL20_C
            PIXEL21_C
          }
          else
          {
            PIXEL10_3
            PIXEL20_4
            PIXEL21_3
          }
          if (Diff(w[6], w[8]))
          {
            PIXEL22_1M
          }
          else
          {
            PIXEL22_2
          }
          break;
        }
        case 87:
        {
          PIXEL00_1L
          if (Diff(w[2], w[6]))
          {
            PIXEL01_C
            PIXEL02_C
            PIXEL12_C
          }
          else
          {
            PIXEL01_3
            PIXEL02_4
            PIXEL12_3
          }
          PIXEL10_1
          PIXEL11
          PIXEL20_1M
          PIXEL21_C
          if (Diff(w[6], w[8]))
          {
            PIXEL22_1M
          }
          else
          {
            PIXEL22_2
          }
          break;
        }
        case 79:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_C
            PIXEL01_C
            PIXEL10_C
          }
          else
          {
            PIXEL00_4
            PIXEL01_3
            PIXEL10_3
          }
          PIXEL02_1R
          PIXEL11
          PIXEL12_1
          if (Diff(w[8], w[4]))
          {
            PIXEL20_1M
          }
          else
          {
            PIXEL20_2
          }
          PIXEL21_C
          PIXEL22_1M
          break;
        }
        case 122:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_1M
          }
          else
          {
            PIXEL00_2
          }
          PIXEL01_C
          if (Diff(w[2], w[6]))
          {
            PIXEL02_1M
          }
          else
          {
            PIXEL02_2
          }
          PIXEL11
          PIXEL12_C
          if (Diff(w[8], w[4]))
          {
            PIXEL10_C
            PIXEL20_C
            PIXEL21_C
          }
          else
          {
            PIXEL10_3
            PIXEL20_4
            PIXEL21_3
          }
          if (Diff(w[6], w[8]))
          {
            PIXEL22_1M
          }
          else
          {
            PIXEL22_2
          }
          break;
        }
        case 94:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_1M
          }
          else
          {
            PIXEL00_2
          }
          if (Diff(w[2], w[6]))
          {
            PIXEL01_C
            PIXEL02_C
            PIXEL12_C
          }
          else
          {
            PIXEL01_3
            PIXEL02_4
            PIXEL12_3
          }
          PIXEL10_C
          PIXEL11
          if (Diff(w[8], w[4]))
          {
            PIXEL20_1M
          }
          else
          {
            PIXEL20_2
          }
          PIXEL21_C
          if (Diff(w[6], w[8]))
          {
            PIXEL22_1M
          }
          else
          {
            PIXEL22_2
          }
          break;
        }
        case 218:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_1M
          }
          else
          {
            PIXEL00_2
          }
          PIXEL01_C
          if (Diff(w[2], w[6]))
          {
            PIXEL02_1M
          }
          else
          {
            PIXEL02_2
          }
          PIXEL10_C
          PIXEL11
          if (Diff(w[8], w[4]))
          {
            PIXEL20_1M
          }
          else
          {
            PIXEL20_2
          }
          if (Diff(w[6], w[8]))
          {
            PIXEL12_C
            PIXEL21_C
            PIXEL22_C
          }
          else
          {
            PIXEL12_3
            PIXEL21_3
            PIXEL22_4
          }
          break;
        }
        case 91:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_C
            PIXEL01_C
            PIXEL10_C
          }
          else
          {
            PIXEL00_4
            PIXEL01_3
            PIXEL10_3
          }
          if (Diff(w[2], w[6]))
          {
            PIXEL02_1M
          }
          else
          {
            PIXEL02_2
          }
          PIXEL11
          PIXEL12_C
          if (Diff(w[8], w[4]))
          {
            PIXEL20_1M
          }
          else
          {
            PIXEL20_2
          }
          PIXEL21_C
          if (Diff(w[6], w[8]))
          {
            PIXEL22_1M
          }
          else
          {
            PIXEL22_2
          }
          break;
        }
        case 229:
        {
          PIXEL00_2
          PIXEL01_1
          PIXEL02_2
          PIXEL10_1
          PIXEL11
          PIXEL12_1
          PIXEL20_1L
          PIXEL21_C
          PIXEL22_1R
          break;
        }
        case 167:
        {
          PIXEL00_1L
          PIXEL01_C
          PIXEL02_1R
          PIXEL10_1
          PIXEL11
          PIXEL12_1
          PIXEL20_2
          PIXEL21_1
          PIXEL22_2
          break;
        }
        case 173:
        {
          PIXEL00_1U
          PIXEL01_1
          PIXEL02_2
          PIXEL10_C
          PIXEL11
          PIXEL12_1
          PIXEL20_1D
          PIXEL21_1
          PIXEL22_2
          break;
        }
        case 181:
        {
          PIXEL00_2
          PIXEL01_1
          PIXEL02_1U
          PIXEL10_1
          PIXEL11
          PIXEL12_C
          PIXEL20_2
          PIXEL21_1
          PIXEL22_1D
          break;
        }
        case 186:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_1M
          }
          else
          {
            PIXEL00_2
          }
          PIXEL01_C
          if (Diff(w[2], w[6]))
          {
            PIXEL02_1M
          }
          else
          {
            PIXEL02_2
          }
          PIXEL10_C
          PIXEL11
          PIXEL12_C
          PIXEL20_1D
          PIXEL21_1
          PIXEL22_1D
          break;
        }
        case 115:
        {
          PIXEL00_1L
          PIXEL01_C
          if (Diff(w[2], w[6]))
          {
            PIXEL02_1M
          }
          else
          {
            PIXEL02_2
          }
          PIXEL10_1
          PIXEL11
          PIXEL12_C
          PIXEL20_1L
          PIXEL21_C
          if (Diff(w[6], w[8]))
          {
            PIXEL22_1M
          }
          else
          {
            PIXEL22_2
          }
          break;
        }
        case 93:
        {
          PIXEL00_1U
          PIXEL01_1
          PIXEL02_1U
          PIXEL10_C
          PIXEL11
          PIXEL12_C
          if (Diff(w[8], w[4]))
          {
            PIXEL20_1M
          }
          else
          {
            PIXEL20_2
          }
          PIXEL21_C
          if (Diff(w[6], w[8]))
          {
            PIXEL22_1M
          }
          else
          {
            PIXEL22_2
          }
          break;
        }
        case 206:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_1M
          }
          else
          {
            PIXEL00_2
          }
          PIXEL01_C
          PIXEL02_1R
          PIXEL10_C
          PIXEL11
          PIXEL12_1
          if (Diff(w[8], w[4]))
          {
            PIXEL20_1M
          }
          else
          {
            PIXEL20_2
          }
          PIXEL21_C
          PIXEL22_1R
          break;
        }
        case 205:
        case 201:
        {
          PIXEL00_1U
          PIXEL01_1
          PIXEL02_2
          PIXEL10_C
          PIXEL11
          PIXEL12_1
          if (Diff(w[8], w[4]))
          {
            PIXEL20_1M
          }
          else
          {
            PIXEL20_2
          }
          PIXEL21_C
          PIXEL22_1R
          break;
        }
        case 174:
        case 46:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_1M
          }
          else
          {
            PIXEL00_2
          }
          PIXEL01_C
          PIXEL02_1R
          PIXEL10_C
          PIXEL11
          PIXEL12_1
          PIXEL20_1D
          PIXEL21_1
          PIXEL22_2
          break;
        }
        case 179:
        case 147:
        {
          PIXEL00_1L
          PIXEL01_C
          if (Diff(w[2], w[6]))
          {
            PIXEL02_1M
          }
          else
          {
            PIXEL02_2
          }
          PIXEL10_1
          PIXEL11
          PIXEL12_C
          PIXEL20_2
          PIXEL21_1
          PIXEL22_1D
          break;
        }
        case 117:
        case 116:
        {
          PIXEL00_2
          PIXEL01_1
          PIXEL02_1U
          PIXEL10_1
          PIXEL11
          PIXEL12_C
          PIXEL20_1L
          PIXEL21_C
          if (Diff(w[6], w[8]))
          {
            PIXEL22_1M
          }
          else
          {
            PIXEL22_2
          }
          break;
        }
        case 189:
        {
          PIXEL00_1U
          PIXEL01_1
          PIXEL02_1U
          PIXEL10_C
          PIXEL11
          PIXEL12_C
          PIXEL20_1D
          PIXEL21_1
          PIXEL22_1D
          break;
        }
        case 231:
        {
          PIXEL00_1L
          PIXEL01_C
          PIXEL02_1R
          PIXEL10_1
          PIXEL11
          PIXEL12_1
          PIXEL20_1L
          PIXEL21_C
          PIXEL22_1R
          break;
        }
        case 126:
        {
          PIXEL00_1M
          if (Diff(w[2], w[6]))
          {
            PIXEL01_C
            PIXEL02_C
            PIXEL12_C
          }
          else
          {
            PIXEL01_3
            PIXEL02_4
            PIXEL12_3
          }
          PIXEL11
          if (Diff(w[8], w[4]))
          {
            PIXEL10_C
            PIXEL20_C
            PIXEL21_C
          }
          else
          {
            PIXEL10_3
            PIXEL20_4
            PIXEL21_3
          }
          PIXEL22_1M
          break;
        }
        case 219:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_C
            PIXEL01_C
            PIXEL10_C
          }
          else
          {
            PIXEL00_4
            PIXEL01_3
            PIXEL10_3
          }
          PIXEL02_1M
          PIXEL11
          PIXEL20_1M
          if (Diff(w[6], w[8]))
          {
            PIXEL12_C
            PIXEL21_C
            PIXEL22_C
          }
          else
          {
            PIXEL12_3
            PIXEL21_3
            PIXEL22_4
          }
          break;
        }
        case 125:
        {
          if (Diff(w[8], w[4]))
          {
            PIXEL00_1U
            PIXEL10_C
            PIXEL20_C
            PIXEL21_C
          }
          else
          {
            PIXEL00_2
            PIXEL10_6
            PIXEL20_5
            PIXEL21_1
          }
          PIXEL01_1
          PIXEL02_1U
          PIXEL11
          PIXEL12_C
          PIXEL22_1M
          break;
        }
        case 221:
        {
          if (Diff(w[6], w[8]))
          {
            PIXEL02_1U
            PIXEL12_C
            PIXEL21_C
            PIXEL22_C
          }
          else
          {
            PIXEL02_2
            PIXEL12_6
            PIXEL21_1
            PIXEL22_5
          }
          PIXEL00_1U
          PIXEL01_1
          PIXEL10_C
          PIXEL11
          PIXEL20_1M
          break;
        }
        case 207:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_C
            PIXEL01_C
            PIXEL02_1R
            PIXEL10_C
          }
          else
          {
            PIXEL00_5
            PIXEL01_6
            PIXEL02_2
            PIXEL10_1
          }
          PIXEL11
          PIXEL12_1
          PIXEL20_1M
          PIXEL21_C
          PIXEL22_1R
          break;
        }
        case 238:
        {
          if (Diff(w[8], w[4]))
          {
            PIXEL10_C
            PIXEL20_C
            PIXEL21_C
            PIXEL22_1R
          }
          else
          {
            PIXEL10_1
            PIXEL20_5
            PIXEL21_6
            PIXEL22_2
          }
          PIXEL00_1M
          PIXEL01_C
          PIXEL02_1R
          PIXEL11
          PIXEL12_1
          break;
        }
        case 190:
        {
          if (Diff(w[2], w[6]))
          {
            PIXEL01_C
            PIXEL02_C
            PIXEL12_C
            PIXEL22_1D
          }
          else
          {
            PIXEL01_1
            PIXEL02_5
            PIXEL12_6
            PIXEL22_2
          }
          PIXEL00_1M
          PIXEL10_C
          PIXEL11
          PIXEL20_1D
          PIXEL21_1
          break;
        }
        case 187:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_C
            PIXEL01_C
            PIXEL10_C
            PIXEL20_1D
          }
          else
          {
            PIXEL00_5
            PIXEL01_1
            PIXEL10_6
            PIXEL20_2
          }
          PIXEL02_1M
          PIXEL11
          PIXEL12_C
          PIXEL21_1
          PIXEL22_1D
          break;
        }
        case 243:
        {
          if (Diff(w[6], w[8]))
          {
            PIXEL12_C
            PIXEL20_1L
            PIXEL21_C
            PIXEL22_C
          }
          else
          {
            PIXEL12_1
            PIXEL20_2
            PIXEL21_6
            PIXEL22_5
          }
          PIXEL00_1L
          PIXEL01_C
          PIXEL02_1M
          PIXEL10_1
          PIXEL11
          break;
        }
        case 119:
        {
          if (Diff(w[2], w[6]))
          {
            PIXEL00_1L
            PIXEL01_C
            PIXEL02_C
            PIXEL12_C
          }
          else
          {
            PIXEL00_2
            PIXEL01_6
            PIXEL02_5
            PIXEL12_1
          }
          PIXEL10_1
          PIXEL11
          PIXEL20_1L
          PIXEL21_C
          PIXEL22_1M
          break;
        }
        case 237:
        case 233:
        {
          PIXEL00_1U
          PIXEL01_1
          PIXEL02_2
          PIXEL10_C
          PIXEL11
          PIXEL12_1
          if (Diff(w[8], w[4]))
          {
            PIXEL20_C
          }
          else
          {
            PIXEL20_2
          }
          PIXEL21_C
          PIXEL22_1R
          break;
        }
        case 175:
        case 47:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_C
          }
          else
          {
            PIXEL00_2
          }
          PIXEL01_C
          PIXEL02_1R
          PIXEL10_C
          PIXEL11
          PIXEL12_1
          PIXEL20_1D
          PIXEL21_1
          PIXEL22_2
          break;
        }
        case 183:
        case 151:
        {
          PIXEL00_1L
          PIXEL01_C
          if (Diff(w[2], w[6]))
          {
            PIXEL02_C
          }
          else
          {
            PIXEL02_2
          }
          PIXEL10_1
          PIXEL11
          PIXEL12_C
          PIXEL20_2
          PIXEL21_1
          PIXEL22_1D
          break;
        }
        case 245:
        case 244:
        {
          PIXEL00_2
          PIXEL01_1
          PIXEL02_1U
          PIXEL10_1
          PIXEL11
          PIXEL12_C
          PIXEL20_1L
          PIXEL21_C
          if (Diff(w[6], w[8]))
          {
            PIXEL22_C
          }
          else
          {
            PIXEL22_2
          }
          break;
        }
        case 250:
        {
          PIXEL00_1M
          PIXEL01_C
          PIXEL02_1M
          PIXEL11
          if (Diff(w[8], w[4]))
          {
            PIXEL10_C
            PIXEL20_C
          }
          else
          {
            PIXEL10_3
            PIXEL20_4
          }
          PIXEL21_C
          if (Diff(w[6], w[8]))
          {
            PIXEL12_C
            PIXEL22_C
          }
          else
          {
            PIXEL12_3
            PIXEL22_4
          }
          break;
        }
        case 123:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_C
            PIXEL01_C
          }
          else
          {
            PIXEL00_4
            PIXEL01_3
          }
          PIXEL02_1M
          PIXEL10_C
          PIXEL11
          PIXEL12_C
          if (Diff(w[8], w[4]))
          {
            PIXEL20_C
            PIXEL21_C
          }
          else
          {
            PIXEL20_4
            PIXEL21_3
          }
          PIXEL22_1M
          break;
        }
        case 95:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_C
            PIXEL10_C
          }
          else
          {
            PIXEL00_4
            PIXEL10_3
          }
          PIXEL01_C
          if (Diff(w[2], w[6]))
          {
            PIXEL02_C
            PIXEL12_C
          }
          else
          {
            PIXEL02_4
            PIXEL12_3
          }
          PIXEL11
          PIXEL20_1M
          PIXEL21_C
          PIXEL22_1M
          break;
        }
        case 222:
        {
          PIXEL00_1M
          if (Diff(w[2], w[6]))
          {
            PIXEL01_C
            PIXEL02_C
          }
          else
          {
            PIXEL01_3
            PIXEL02_4
          }
          PIXEL10_C
          PIXEL11
          PIXEL12_C
          PIXEL20_1M
          if (Diff(w[6], w[8]))
          {
            PIXEL21_C
            PIXEL22_C
          }
          else
          {
            PIXEL21_3
            PIXEL22_4
          }
          break;
        }
        case 252:
        {
          PIXEL00_1M
          PIXEL01_1
          PIXEL02_1U
          PIXEL11
          PIXEL12_C
          if (Diff(w[8], w[4]))
          {
            PIXEL10_C
            PIXEL20_C
          }
          else
          {
            PIXEL10_3
            PIXEL20_4
          }
          PIXEL21_C
          if (Diff(w[6], w[8]))
          {
            PIXEL22_C
          }
          else
          {
            PIXEL22_2
          }
          break;
        }
        case 249:
        {
          PIXEL00_1U
          PIXEL01_1
          PIXEL02_1M
          PIXEL10_C
          PIXEL11
          if (Diff(w[8], w[4]))
          {
            PIXEL20_C
          }
          else
          {
            PIXEL20_2
          }
          PIXEL21_C
          if (Diff(w[6], w[8]))
          {
            PIXEL12_C
            PIXEL22_C
          }
          else
          {
            PIXEL12_3
            PIXEL22_4
          }
          break;
        }
        case 235:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_C
            PIXEL01_C
          }
          else
          {
            PIXEL00_4
            PIXEL01_3
          }
          PIXEL02_1M
          PIXEL10_C
          PIXEL11
          PIXEL12_1
          if (Diff(w[8], w[4]))
          {
            PIXEL20_C
          }
          else
          {
            PIXEL20_2
          }
          PIXEL21_C
          PIXEL22_1R
          break;
        }
        case 111:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_C
          }
          else
          {
            PIXEL00_2
          }
          PIXEL01_C
          PIXEL02_1R
          PIXEL10_C
          PIXEL11
          PIXEL12_1
          if (Diff(w[8], w[4]))
          {
            PIXEL20_C
            PIXEL21_C
          }
          else
          {
            PIXEL20_4
            PIXEL21_3
          }
          PIXEL22_1M
          break;
        }
        case 63:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_C
          }
          else
          {
            PIXEL00_2
          }
          PIXEL01_C
          if (Diff(w[2], w[6]))
          {
            PIXEL02_C
            PIXEL12_C
          }
          else
          {
            PIXEL02_4
            PIXEL12_3
          }
          PIXEL10_C
          PIXEL11
          PIXEL20_1D
          PIXEL21_1
          PIXEL22_1M
          break;
        }
        case 159:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_C
            PIXEL10_C
          }
          else
          {
            PIXEL00_4
            PIXEL10_3
          }
          PIXEL01_C
          if (Diff(w[2], w[6]))
          {
            PIXEL02_C
          }
          else
          {
            PIXEL02_2
          }
          PIXEL11
          PIXEL12_C
          PIXEL20_1M
          PIXEL21_1
          PIXEL22_1D
          break;
        }
        case 215:
        {
          PIXEL00_1L
          PIXEL01_C
          if (Diff(w[2], w[6]))
          {
            PIXEL02_C
          }
          else
          {
            PIXEL02_2
          }
          PIXEL10_1
          PIXEL11
          PIXEL12_C
          PIXEL20_1M
          if (Diff(w[6], w[8]))
          {
            PIXEL21_C
            PIXEL22_C
          }
          else
          {
            PIXEL21_3
            PIXEL22_4
          }
          break;
        }
        case 246:
        {
          PIXEL00_1M
          if (Diff(w[2], w[6]))
          {
            PIXEL01_C
            PIXEL02_C
          }
          else
          {
            PIXEL01_3
            PIXEL02_4
          }
          PIXEL10_1
          PIXEL11
          PIXEL12_C
          PIXEL20_1L
          PIXEL21_C
          if (Diff(w[6], w[8]))
          {
            PIXEL22_C
          }
          else
          {
            PIXEL22_2
          }
          break;
        }
        case 254:
        {
          PIXEL00_1M
          if (Diff(w[2], w[6]))
          {
            PIXEL01_C
            PIXEL02_C
          }
          else
          {
            PIXEL01_3
            PIXEL02_4
          }
          PIXEL11
          if (Diff(w[8], w[4]))
          {
            PIXEL10_C
            PIXEL20_C
          }
          else
          {
            PIXEL10_3
            PIXEL20_4
          }
          if (Diff(w[6], w[8]))
          {
            PIXEL12_C
            PIXEL21_C
            PIXEL22_C
          }
          else
          {
            PIXEL12_3
            PIXEL21_3
            PIXEL22_2
          }
          break;
        }
        case 253:
        {
          PIXEL00_1U
          PIXEL01_1
          PIXEL02_1U
          PIXEL10_C
          PIXEL11
          PIXEL12_C
          if (Diff(w[8], w[4]))
          {
            PIXEL20_C
          }
          else
          {
            PIXEL20_2
          }
          PIXEL21_C
          if (Diff(w[6], w[8]))
          {
            PIXEL22_C
          }
          else
          {
            PIXEL22_2
          }
          break;
        }
        case 251:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_C
            PIXEL01_C
          }
          else
          {
            PIXEL00_4
            PIXEL01_3
          }
          PIXEL02_1M
          PIXEL11
          if (Diff(w[8], w[4]))
          {
            PIXEL10_C
            PIXEL20_C
            PIXEL21_C
          }
          else
          {
            PIXEL10_3
            PIXEL20_2
            PIXEL21_3
          }
          if (Diff(w[6], w[8]))
          {
            PIXEL12_C
            PIXEL22_C
          }
          else
          {
            PIXEL12_3
            PIXEL22_4
          }
          break;
        }
        case 239:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_C
          }
          else
          {
            PIXEL00_2
          }
          PIXEL01_C
          PIXEL02_1R
          PIXEL10_C
          PIXEL11
          PIXEL12_1
          if (Diff(w[8], w[4]))
          {
            PIXEL20_C
          }
          else
          {
            PIXEL20_2
          }
          PIXEL21_C
          PIXEL22_1R
          break;
        }
        case 127:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_C
            PIXEL01_C
            PIXEL10_C
          }
          else
          {
            PIXEL00_2
            PIXEL01_3
            PIXEL10_3
          }
          if (Diff(w[2], w[6]))
          {
            PIXEL02_C
            PIXEL12_C
          }
          else
          {
            PIXEL02_4
            PIXEL12_3
          }
          PIXEL11
          if (Diff(w[8], w[4]))
          {
            PIXEL20_C
            PIXEL21_C
          }
          else
          {
            PIXEL20_4
            PIXEL21_3
          }
          PIXEL22_1M
          break;
        }
        case 191:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_C
          }
          else
          {
            PIXEL00_2
          }
          PIXEL01_C
          if (Diff(w[2], w[6]))
          {
            PIXEL02_C
          }
          else
          {
            PIXEL02_2
          }
          PIXEL10_C
          PIXEL11
          PIXEL12_C
          PIXEL20_1D
          PIXEL21_1
          PIXEL22_1D
          break;
        }
        case 223:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_C
            PIXEL10_C
          }
          else
          {
            PIXEL00_4
            PIXEL10_3
          }
          if (Diff(w[2], w[6]))
          {
            PIXEL01_C
            PIXEL02_C
            PIXEL12_C
          }
          else
          {
            PIXEL01_3
            PIXEL02_2
            PIXEL12_3
          }
          PIXEL11
          PIXEL20_1M
          if (Diff(w[6], w[8]))
          {
            PIXEL21_C
            PIXEL22_C
          }
          else
          {
            PIXEL21_3
            PIXEL22_4
          }
          break;
        }
        case 247:
        {
          PIXEL00_1L
          PIXEL01_C
          if (Diff(w[2], w[6]))
          {
            PIXEL02_C
          }
          else
          {
            PIXEL02_2
          }
          PIXEL10_1
          PIXEL11
          PIXEL12_C
          PIXEL20_1L
          PIXEL21_C
          if (Diff(w[6], w[8]))
          {
            PIXEL22_C
          }
          else
          {
            PIXEL22_2
          }
          break;
        }
        case 255:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_C
          }
          else
          {
            PIXEL00_2
          }
          PIXEL01_C
          if (Diff(w[2], w[6]))
          {
            PIXEL02_C
          }
          else
          {
            PIXEL02_2
          }
          PIXEL10_C
          PIXEL11
          PIXEL12_C
          if (Diff(w[8], w[4]))
          {
            PIXEL20_C
          }
          else
          {
            PIXEL20_2
          }
          PIXEL21_C
          if (Diff(w[6], w[8]))
          {
            PIXEL22_C
          }
          else
          {
            PIXEL22_2
          }
          break;
        }
      }
      pIn += sizeof(PIXEL_TYPE);
      pOut += sizeof(PIXEL_TYPE) * 3;
    }
  }
}

#undef PIXEL00_1M  
#undef PIXEL00_1U  
#undef PIXEL00_1L  
#undef PIXEL00_2   
#undef PIXEL00_4   
#undef PIXEL00_5   
#undef PIXEL00_C   

#undef PIXEL01_1   
#undef PIXEL01_3   
#undef PIXEL01_6   
#undef PIXEL01_C   

#undef PIXEL02_1M  
#undef PIXEL02_1U  
#undef PIXEL02_1R  
#undef PIXEL02_2   
#undef PIXEL02_4   
#undef PIXEL02_5   
#undef PIXEL02_C   

#undef PIXEL10_1   
#undef PIXEL10_3   
#undef PIXEL10_6   
#undef PIXEL10_C   

#undef PIXEL11     

#undef PIXEL12_1   
#undef PIXEL12_3   
#undef PIXEL12_6   
#undef PIXEL12_C   

#undef PIXEL20_1M  
#undef PIXEL20_1D  
#undef PIXEL20_1L  
#undef PIXEL20_2   
#undef PIXEL20_4   
#undef PIXEL20_5   
#undef PIXEL20_C   

#undef PIXEL21_1   
#undef PIXEL21_3   
#undef PIXEL21_6   
#undef PIXEL21_C   

#undef PIXEL22_1M  
#undef PIXEL22_1D  
#undef PIXEL22_1R  
#undef PIXEL22_2   
#undef PIXEL22_4   
#undef PIXEL22_5   
#undef PIXEL22_C   



void hq3x(SDL_Surface * input, SDL_Surface * output){
    InitLUTs();
    hq3x_16((unsigned char*) input->pixels, (unsigned char*) output->pixels,
            input->w, input->h, input->pitch, output->pitch);
}

}

#endif
