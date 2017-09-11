/*
 * Copyright (C) 2010 David Conrad
 * Copyright (C) 2010 Ronald S. Bultje
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/**
 * @file
 * VP8 compatible video decoder
 */

#ifndef AVCODEC_VP8DATA_H
#define AVCODEC_VP8DATA_H

#include "vp8.h"
#include "h264pred.h"

static const uint8_t vp7_pred4x4_mode[] = {
	[DC_PRED8x8]    = DC_PRED,
    [VERT_PRED8x8]  = TM_VP8_PRED,
    [HOR_PRED8x8]   = TM_VP8_PRED,
    [PLANE_PRED8x8] = TM_VP8_PRED,
};

static const uint8_t vp8_pred4x4_mode[] = {
	[DC_PRED8x8]    = DC_PRED,
    [VERT_PRED8x8]  = VERT_PRED,
    [HOR_PRED8x8]   = HOR_PRED,
    [PLANE_PRED8x8] = TM_VP8_PRED,
};

static const int8_t vp8_pred16x16_tree_intra[4][2] = {
    {   -MODE_I4x4,              1 }, // '0'
    {            2,              3 },
    {  -DC_PRED8x8,  -VERT_PRED8x8 }, // '100', '101'
    { -HOR_PRED8x8, -PLANE_PRED8x8 }, // '110', '111'
};

static const int8_t vp8_pred16x16_tree_inter[4][2] = {
    {    -DC_PRED8x8,            1 }, // '0'
    {              2,            3 },
    {  -VERT_PRED8x8, -HOR_PRED8x8 }, // '100', '101'
    { -PLANE_PRED8x8,   -MODE_I4x4 }, // '110', '111'
};

typedef struct VP7MVPred {
    int8_t yoffset;
    int8_t xoffset;
    uint8_t subblock;
    uint8_t score;
} VP7MVPred;

#define VP7_MV_PRED_COUNT 12
static const VP7MVPred vp7_mv_pred[VP7_MV_PRED_COUNT] = {
    { -1,  0, 12, 8 },
    {  0, -1,  3, 8 },
    { -1, -1, 15, 2 },
    { -1,  1, 12, 2 },
    { -2,  0, 12, 2 },
    {  0, -2,  3, 2 },
    { -1, -2, 15, 1 },
    { -2, -1, 15, 1 },
    { -2,  1, 12, 1 },
    { -1,  2, 12, 1 },
    { -2, -2, 15, 1 },
    { -2,  2, 12, 1 },
};

static const int vp7_mode_contexts[31][4] = {
    {   3,   3,   1, 246 },
    {   7,  89,  66, 239 },
    {  10,  90,  78, 238 },
    {  14, 118,  95, 241 },
    {  14, 123, 106, 238 },
    {  20, 140, 109, 240 },
    {  13, 155, 103, 238 },
    {  21, 158,  99, 240 },
    {  27,  82, 108, 232 },
    {  19,  99, 123, 217 },
    {  45, 139, 148, 236 },
    {  50, 117, 144, 235 },
    {  57, 128, 164, 238 },
    {  69, 139, 171, 239 },
    {  74, 154, 179, 238 },
    { 112, 165, 186, 242 },
    {  98, 143, 185, 245 },
    { 105, 153, 190, 250 },
    { 124, 167, 192, 245 },
    { 131, 186, 203, 246 },
    {  59, 184, 222, 224 },
    { 148, 215, 214, 213 },
    { 137, 211, 210, 219 },
    { 190, 227, 128, 228 },
    { 183, 228, 128, 228 },
    { 194, 234, 128, 228 },
    { 202, 236, 128, 228 },
    { 205, 240, 128, 228 },
    { 205, 244, 128, 228 },
    { 225, 246, 128, 228 },
    { 233, 251, 128, 228 },
};

static const int vp8_mode_contexts[6][4] = {
    {   7,   1,   1, 143 },
    {  14,  18,  14, 107 },
    { 135,  64,  57,  68 },
    {  60,  56, 128,  65 },
    { 159, 134, 128,  34 },
    { 234, 188, 128,  28 },
};

static const uint8_t vp8_mbsplits[5][16] = {
    { 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,  1,  1,  1,  1,  1,  1 },
    { 0, 0, 1, 1, 0, 0, 1, 1, 0, 0,  1,  1,  0,  0,  1,  1 },
    { 0, 0, 1, 1, 0, 0, 1, 1, 2, 2,  3,  3,  2,  2,  3,  3 },
    { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0,  0,  0,  0,  0,  0 }
};

static const uint8_t vp8_mbfirstidx[4][16] = {
    { 0, 8 },
    { 0, 2 },
    { 0, 2, 8, 10 },
    { 0, 1, 2,  3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 }
};

static const uint8_t vp8_mbsplit_count[4] = {
    2, 2, 4, 16
};
static const uint8_t vp8_mbsplit_prob[3] = {
    110, 111, 150
};

static const uint8_t vp7_submv_prob[3] = {
    180, 162, 25
};

static const uint8_t vp8_submv_prob[5][3] = {
    { 147, 136,  18 },
    { 106, 145,   1 },
    { 179, 121,   1 },
    { 223,   1,  34 },
    { 208,   1,   1 }
};

static const uint8_t vp8_pred16x16_prob_intra[4] = {
    145, 156, 163, 128
};
static const uint8_t vp8_pred16x16_prob_inter[4] = {
    112,  86, 140,  37
};

static const int8_t vp8_pred4x4_tree[9][2] = {
    {              -DC_PRED,                1 }, // '0'
    {          -TM_VP8_PRED,                2 }, // '10'
    {            -VERT_PRED,                3 }, // '110'
    {                     4,                6 },
    {             -HOR_PRED,                5 }, // '11100'
    { -DIAG_DOWN_RIGHT_PRED, -VERT_RIGHT_PRED }, // '111010', '111011'
    {  -DIAG_DOWN_LEFT_PRED,                7 }, // '11110'
    {       -VERT_LEFT_PRED,                8 }, // '111110'
    {        -HOR_DOWN_PRED,     -HOR_UP_PRED }, // '1111110', '1111111'
};

static const int8_t vp8_pred8x8c_tree[3][2] = {
    {   -DC_PRED8x8,              1 },  // '0'
    { -VERT_PRED8x8,              2 },  // '10
    {  -HOR_PRED8x8, -PLANE_PRED8x8 },  // '110', '111'
};

static const uint8_t vp8_pred8x8c_prob_intra[3] = {
    142, 114, 183
};
static const uint8_t vp8_pred8x8c_prob_inter[3] = {
    162, 101, 204
};
static const uint8_t vp8_pred4x4_prob_inter[9] = {
    120, 90, 79, 133, 87, 85, 80, 111, 151
};

static const uint8_t vp8_pred4x4_prob_intra[10][10][9] = {
    {
        {  39,  53, 200,  87,  26,  21,  43, 232, 171 },
        {  56,  34,  51, 104, 114, 102,  29,  93,  77 },
        {  88,  88, 147, 150,  42,  46,  45, 196, 205 },
        { 107,  54,  32,  26,  51,   1,  81,  43,  31 },
        {  39,  28,  85, 171,  58, 165,  90,  98,  64 },
        {  34,  22, 116, 206,  23,  34,  43, 166,  73 },
        {  34,  19,  21, 102, 132, 188,  16,  76, 124 },
        {  68,  25, 106,  22,  64, 171,  36, 225, 114 },
        {  62,  18,  78,  95,  85,  57,  50,  48,  51 },
        {  43,  97, 183, 117,  85,  38,  35, 179,  61 },
    },
    {
        { 112, 113,  77,  85, 179, 255,  38, 120, 114 },
        {  40,  42,   1, 196, 245, 209,  10,  25, 109 },
        { 193, 101,  35, 159, 215, 111,  89,  46, 111 },
        { 100,  80,   8,  43, 154,   1,  51,  26,  71 },
        {  88,  43,  29, 140, 166, 213,  37,  43, 154 },
        {  61,  63,  30, 155,  67,  45,  68,   1, 209 },
        {  41,  40,   5, 102, 211, 183,   4,   1, 221 },
        { 142,  78,  78,  16, 255, 128,  34, 197, 171 },
        {  51,  50,  17, 168, 209, 192,  23,  25,  82 },
        {  60, 148,  31, 172, 219, 228,  21,  18, 111 },
    },
    {
        { 175,  69, 143,  80,  85,  82,  72, 155, 103 },
        {  56,  58,  10, 171, 218, 189,  17,  13, 152 },
        { 231, 120,  48,  89, 115, 113, 120, 152, 112 },
        { 144,  71,  10,  38, 171, 213, 144,  34,  26 },
        { 114,  26,  17, 163,  44, 195,  21,  10, 173 },
        { 121,  24,  80, 195,  26,  62,  44,  64,  85 },
        {  63,  20,   8, 114, 114, 208,  12,   9, 226 },
        { 170,  46,  55,  19, 136, 160,  33, 206,  71 },
        {  81,  40,  11,  96, 182,  84,  29,  16,  36 },
        { 152, 179,  64, 126, 170, 118,  46,  70,  95 },
    },
    {
        {  75,  79, 123,  47,  51, 128,  81, 171,   1 },
        {  57,  17,   5,  71, 102,  57,  53,  41,  49 },
        { 125,  98,  42,  88, 104,  85, 117, 175,  82 },
        { 115,  21,   2,  10, 102, 255, 166,  23,   6 },
        {  38,  33,  13, 121,  57,  73,  26,   1,  85 },
        {  41,  10,  67, 138,  77, 110,  90,  47, 114 },
        {  57,  18,  10, 102, 102, 213,  34,  20,  43 },
        { 101,  29,  16,  10,  85, 128, 101, 196,  26 },
        { 117,  20,  15,  36, 163, 128,  68,   1,  26 },
        {  95,  84,  53,  89, 128, 100, 113, 101,  45 },
    },
    {
        {  63,  59,  90, 180,  59, 166,  93,  73, 154 },
        {  40,  40,  21, 116, 143, 209,  34,  39, 175 },
        { 138,  31,  36, 171,  27, 166,  38,  44, 229 },
        {  57,  46,  22,  24, 128,   1,  54,  17,  37 },
        {  47,  15,  16, 183,  34, 223,  49,  45, 183 },
        {  46,  17,  33, 183,   6,  98,  15,  32, 183 },
        {  40,   3,   9, 115,  51, 192,  18,   6, 223 },
        {  65,  32,  73, 115,  28, 128,  23, 128, 205 },
        {  87,  37,   9, 115,  59,  77,  64,  21,  47 },
        {  67,  87,  58, 169,  82, 115,  26,  59, 179 },
    },
    {
        {  54,  57, 112, 184,   5,  41,  38, 166, 213 },
        {  30,  34,  26, 133, 152, 116,  10,  32, 134 },
        { 104,  55,  44, 218,   9,  54,  53, 130, 226 },
        {  75,  32,  12,  51, 192, 255, 160,  43,  51 },
        {  39,  19,  53, 221,  26, 114,  32,  73, 255 },
        {  31,   9,  65, 234,   2,  15,   1, 118,  73 },
        {  56,  21,  23, 111,  59, 205,  45,  37, 192 },
        {  88,  31,  35,  67, 102,  85,  55, 186,  85 },
        {  55,  38,  70, 124,  73, 102,   1,  34,  98 },
        {  64,  90,  70, 205,  40,  41,  23,  26,  57 },
    },
    {
        {  86,  40,  64, 135, 148, 224,  45, 183, 128 },
        {  22,  26,  17, 131, 240, 154,  14,   1, 209 },
        { 164,  50,  31, 137, 154, 133,  25,  35, 218 },
        {  83,  12,  13,  54, 192, 255,  68,  47,  28 },
        {  45,  16,  21,  91,  64, 222,   7,   1, 197 },
        {  56,  21,  39, 155,  60, 138,  23, 102, 213 },
        {  18,  11,   7,  63, 144, 171,   4,   4, 246 },
        {  85,  26,  85,  85, 128, 128,  32, 146, 171 },
        {  35,  27,  10, 146, 174, 171,  12,  26, 128 },
        {  51, 103,  44, 131, 131, 123,  31,   6, 158 },
    },
    {
        {  68,  45, 128,  34,   1,  47,  11, 245, 171 },
        {  62,  17,  19,  70, 146,  85,  55,  62,  70 },
        { 102,  61,  71,  37,  34,  53,  31, 243, 192 },
        {  75,  15,   9,   9,  64, 255, 184, 119,  16 },
        {  37,  43,  37, 154, 100, 163,  85, 160,   1 },
        {  63,   9,  92, 136,  28,  64,  32, 201,  85 },
        {  56,   8,  17, 132, 137, 255,  55, 116, 128 },
        {  86,   6,  28,   5,  64, 255,  25, 248,   1 },
        {  58,  15,  20,  82, 135,  57,  26, 121,  40 },
        {  69,  60,  71,  38,  73, 119,  28, 222,  37 },
    },
    {
        { 101,  75, 128, 139, 118, 146, 116, 128,  85 },
        {  56,  41,  15, 176, 236,  85,  37,   9,  62 },
        { 190,  80,  35,  99, 180,  80, 126,  54,  45 },
        { 146,  36,  19,  30, 171, 255,  97,  27,  20 },
        {  71,  30,  17, 119, 118, 255,  17,  18, 138 },
        { 101,  38,  60, 138,  55,  70,  43,  26, 142 },
        {  32,  41,  20, 117, 151, 142,  20,  21, 163 },
        { 138,  45,  61,  62, 219,   1,  81, 188,  64 },
        { 112,  19,  12,  61, 195, 128,  48,   4,  24 },
        {  85, 126,  47,  87, 176,  51,  41,  20,  32 },
    },
    {
        {  66, 102, 167,  99,  74,  62,  40, 234, 128 },
        {  41,  53,   9, 178, 241, 141,  26,   8, 107 },
        { 134, 183,  89, 137,  98, 101, 106, 165, 148 },
        { 104,  79,  12,  27, 217, 255,  87,  17,   7 },
        {  74,  43,  26, 146,  73, 166,  49,  23, 157 },
        {  65,  38, 105, 160,  51,  52,  31, 115, 128 },
        {  47,  41,  14, 110, 182, 183,  21,  17, 194 },
        {  87,  68,  71,  44, 114,  51,  15, 186,  23 },
        {  66,  45,  25, 102, 197, 189,  23,  18,  22 },
        {  72, 187, 100, 130, 157, 111,  32,  75,  80 },
    },
};

static const int8_t vp8_segmentid_tree[][2] = {
    {  1,  2 },
    { -0, -1 }, // '00', '01'
    { -2, -3 }, // '10', '11'
};

static const uint8_t vp8_coeff_band[16] = {
    0, 1, 2, 3, 6, 4, 5, 6, 6, 6, 6, 6, 6, 6, 6, 7
};

/* Inverse of vp8_coeff_band: mappings of bands to coefficient indexes.
 * Each list is -1-terminated. */
static const int8_t vp8_coeff_band_indexes[8][10] = {
    {  0, -1 },
    {  1, -1 },
    {  2, -1 },
    {  3, -1 },
    {  5, -1 },
    {  6, -1 },
    {  4,  7, 8, 9, 10, 11, 12, 13, 14, -1 },
    { 15, -1 }
};

static const uint8_t vp8_dct_cat1_prob[] = {
    159, 0
};
static const uint8_t vp8_dct_cat2_prob[] = {
    165, 145, 0
};
static const uint8_t vp8_dct_cat3_prob[] = {
    173, 148, 140, 0
};
static const uint8_t vp8_dct_cat4_prob[] = {
    176, 155, 140, 135, 0
};
static const uint8_t vp8_dct_cat5_prob[] = {
    180, 157, 141, 134, 130, 0
};
static const uint8_t vp8_dct_cat6_prob[] = {
    254, 254, 243, 230, 196, 177, 153, 140, 133, 130, 129, 0
};

// only used for cat3 and above; cat 1 and 2 are referenced directly
const uint8_t *const ff_vp8_dct_cat_prob[] = {
    vp8_dct_cat3_prob,
    vp8_dct_cat4_prob,
    vp8_dct_cat5_prob,
    vp8_dct_cat6_prob,
};

static const uint8_t vp8_token_default_probs[4][8][3][NUM_DCT_TOKENS - 1] = {
    {
        {
            { 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128 },
            { 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128 },
            { 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128 },
        },
        {
            { 253, 136, 254, 255, 228, 219, 128, 128, 128, 128, 128 },
            { 189, 129, 242, 255, 227, 213, 255, 219, 128, 128, 128 },
            { 106, 126, 227, 252, 214, 209, 255, 255, 128, 128, 128 },
        },
        {
            {   1,  98, 248, 255, 236, 226, 255, 255, 128, 128, 128 },
            { 181, 133, 238, 254, 221, 234, 255, 154, 128, 128, 128 },
            {  78, 134, 202, 247, 198, 180, 255, 219, 128, 128, 128 },
        },
        {
            {   1, 185, 249, 255, 243, 255, 128, 128, 128, 128, 128 },
            { 184, 150, 247, 255, 236, 224, 128, 128, 128, 128, 128 },
            {  77, 110, 216, 255, 236, 230, 128, 128, 128, 128, 128 },
        },
        {
            {   1, 101, 251, 255, 241, 255, 128, 128, 128, 128, 128 },
            { 170, 139, 241, 252, 236, 209, 255, 255, 128, 128, 128 },
            {  37, 116, 196, 243, 228, 255, 255, 255, 128, 128, 128 },
        },
        {
            {   1, 204, 254, 255, 245, 255, 128, 128, 128, 128, 128 },
            { 207, 160, 250, 255, 238, 128, 128, 128, 128, 128, 128 },
            { 102, 103, 231, 255, 211, 171, 128, 128, 128, 128, 128 },
        },
        {
            {   1, 152, 252, 255, 240, 255, 128, 128, 128, 128, 128 },
            { 177, 135, 243, 255, 234, 225, 128, 128, 128, 128, 128 },
            {  80, 129, 211, 255, 194, 224, 128, 128, 128, 128, 128 },
        },
        {
            {   1,   1, 255, 128, 128, 128, 128, 128, 128, 128, 128 },
            { 246,   1, 255, 128, 128, 128, 128, 128, 128, 128, 128 },
            { 255, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128 },
        },
    },
    {
        {
            { 198,  35, 237, 223, 193, 187, 162, 160, 145, 155,  62 },
            { 131,  45, 198, 221, 172, 176, 220, 157, 252, 221,   1 },
            {  68,  47, 146, 208, 149, 167, 221, 162, 255, 223, 128 },
        },
        {
            {   1, 149, 241, 255, 221, 224, 255, 255, 128, 128, 128 },
            { 184, 141, 234, 253, 222, 220, 255, 199, 128, 128, 128 },
            {  81,  99, 181, 242, 176, 190, 249, 202, 255, 255, 128 },
        },
        {
            {   1, 129, 232, 253, 214, 197, 242, 196, 255, 255, 128 },
            {  99, 121, 210, 250, 201, 198, 255, 202, 128, 128, 128 },
            {  23,  91, 163, 242, 170, 187, 247, 210, 255, 255, 128 },
        },
        {
            {   1, 200, 246, 255, 234, 255, 128, 128, 128, 128, 128 },
            { 109, 178, 241, 255, 231, 245, 255, 255, 128, 128, 128 },
            {  44, 130, 201, 253, 205, 192, 255, 255, 128, 128, 128 },
        },
        {
            {   1, 132, 239, 251, 219, 209, 255, 165, 128, 128, 128 },
            {  94, 136, 225, 251, 218, 190, 255, 255, 128, 128, 128 },
            {  22, 100, 174, 245, 186, 161, 255, 199, 128, 128, 128 },
        },
        {
            {   1, 182, 249, 255, 232, 235, 128, 128, 128, 128, 128 },
            { 124, 143, 241, 255, 227, 234, 128, 128, 128, 128, 128 },
            {  35,  77, 181, 251, 193, 211, 255, 205, 128, 128, 128 },
        },
        {
            {   1, 157, 247, 255, 236, 231, 255, 255, 128, 128, 128 },
            { 121, 141, 235, 255, 225, 227, 255, 255, 128, 128, 128 },
            {  45,  99, 188, 251, 195, 217, 255, 224, 128, 128, 128 },
        },
        {
            {   1,   1, 251, 255, 213, 255, 128, 128, 128, 128, 128 },
            { 203,   1, 248, 255, 255, 128, 128, 128, 128, 128, 128 },
            { 137,   1, 177, 255, 224, 255, 128, 128, 128, 128, 128 },
        },
    },
    {
        {
            { 253,   9, 248, 251, 207, 208, 255, 192, 128, 128, 128 },
            { 175,  13, 224, 243, 193, 185, 249, 198, 255, 255, 128 },
            {  73,  17, 171, 221, 161, 179, 236, 167, 255, 234, 128 },
        },
        {
            {   1,  95, 247, 253, 212, 183, 255, 255, 128, 128, 128 },
            { 239,  90, 244, 250, 211, 209, 255, 255, 128, 128, 128 },
            { 155,  77, 195, 248, 188, 195, 255, 255, 128, 128, 128 },
        },
        {
            {   1,  24, 239, 251, 218, 219, 255, 205, 128, 128, 128 },
            { 201,  51, 219, 255, 196, 186, 128, 128, 128, 128, 128 },
            {  69,  46, 190, 239, 201, 218, 255, 228, 128, 128, 128 },
        },
        {
            {   1, 191, 251, 255, 255, 128, 128, 128, 128, 128, 128 },
            { 223, 165, 249, 255, 213, 255, 128, 128, 128, 128, 128 },
            { 141, 124, 248, 255, 255, 128, 128, 128, 128, 128, 128 },
        },
        {
            {   1,  16, 248, 255, 255, 128, 128, 128, 128, 128, 128 },
            { 190,  36, 230, 255, 236, 255, 128, 128, 128, 128, 128 },
            { 149,   1, 255, 128, 128, 128, 128, 128, 128, 128, 128 },
        },
        {
            {   1, 226, 255, 128, 128, 128, 128, 128, 128, 128, 128 },
            { 247, 192, 255, 128, 128, 128, 128, 128, 128, 128, 128 },
            { 240, 128, 255, 128, 128, 128, 128, 128, 128, 128, 128 },
        },
        {
            {   1, 134, 252, 255, 255, 128, 128, 128, 128, 128, 128 },
            { 213,  62, 250, 255, 255, 128, 128, 128, 128, 128, 128 },
            {  55,  93, 255, 128, 128, 128, 128, 128, 128, 128, 128 },
        },
        {
            { 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128 },
            { 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128 },
            { 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128 },
        },
    },
    {
        {
            { 202,  24, 213, 235, 186, 191, 220, 160, 240, 175, 255 },
            { 126,  38, 182, 232, 169, 184, 228, 174, 255, 187, 128 },
            {  61,  46, 138, 219, 151, 178, 240, 170, 255, 216, 128 },
        },
        {
            {   1, 112, 230, 250, 199, 191, 247, 159, 255, 255, 128 },
            { 166, 109, 228, 252, 211, 215, 255, 174, 128, 128, 128 },
            {  39,  77, 162, 232, 172, 180, 245, 178, 255, 255, 128 },
        },
        {
            {   1,  52, 220, 246, 198, 199, 249, 220, 255, 255, 128 },
            { 124,  74, 191, 243, 183, 193, 250, 221, 255, 255, 128 },
            {  24,  71, 130, 219, 154, 170, 243, 182, 255, 255, 128 },
        },
        {
            {   1, 182, 225, 249, 219, 240, 255, 224, 128, 128, 128 },
            { 149, 150, 226, 252, 216, 205, 255, 171, 128, 128, 128 },
            {  28, 108, 170, 242, 183, 194, 254, 223, 255, 255, 128 },
        },
        {
            {   1,  81, 230, 252, 204, 203, 255, 192, 128, 128, 128 },
            { 123, 102, 209, 247, 188, 196, 255, 233, 128, 128, 128 },
            {  20,  95, 153, 243, 164, 173, 255, 203, 128, 128, 128 },
        },
        {
            {   1, 222, 248, 255, 216, 213, 128, 128, 128, 128, 128 },
            { 168, 175, 246, 252, 235, 205, 255, 255, 128, 128, 128 },
            {  47, 116, 215, 255, 211, 212, 255, 255, 128, 128, 128 },
        },
        {
            {   1, 121, 236, 253, 212, 214, 255, 255, 128, 128, 128 },
            { 141,  84, 213, 252, 201, 202, 255, 219, 128, 128, 128 },
            {  42,  80, 160, 240, 162, 185, 255, 205, 128, 128, 128 },
        },
        {
            {   1,   1, 255, 128, 128, 128, 128, 128, 128, 128, 128 },
            { 244,   1, 255, 128, 128, 128, 128, 128, 128, 128, 128 },
            { 238,   1, 255, 128, 128, 128, 128, 128, 128, 128, 128 },
        },
    },
};

static const uint8_t vp8_token_update_probs[4][8][3][NUM_DCT_TOKENS - 1] = {
    {
        {
            { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
            { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
            { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
        },
        {
            { 176, 246, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
            { 223, 241, 252, 255, 255, 255, 255, 255, 255, 255, 255 },
            { 249, 253, 253, 255, 255, 255, 255, 255, 255, 255, 255 },
        },
        {
            { 255, 244, 252, 255, 255, 255, 255, 255, 255, 255, 255 },
            { 234, 254, 254, 255, 255, 255, 255, 255, 255, 255, 255 },
            { 253, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
        },
        {
            { 255, 246, 254, 255, 255, 255, 255, 255, 255, 255, 255 },
            { 239, 253, 254, 255, 255, 255, 255, 255, 255, 255, 255 },
            { 254, 255, 254, 255, 255, 255, 255, 255, 255, 255, 255 },
        },
        {
            { 255, 248, 254, 255, 255, 255, 255, 255, 255, 255, 255 },
            { 251, 255, 254, 255, 255, 255, 255, 255, 255, 255, 255 },
            { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
        },
        {
            { 255, 253, 254, 255, 255, 255, 255, 255, 255, 255, 255 },
            { 251, 254, 254, 255, 255, 255, 255, 255, 255, 255, 255 },
            { 254, 255, 254, 255, 255, 255, 255, 255, 255, 255, 255 },
        },
        {
            { 255, 254, 253, 255, 254, 255, 255, 255, 255, 255, 255 },
            { 250, 255, 254, 255, 254, 255, 255, 255, 255, 255, 255 },
            { 254, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
        },
        {
            { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
            { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
            { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
        },
    },
    {
        {
            { 217, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
            { 225, 252, 241, 253, 255, 255, 254, 255, 255, 255, 255 },
            { 234, 250, 241, 250, 253, 255, 253, 254, 255, 255, 255 },
        },
        {
            { 255, 254, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
            { 223, 254, 254, 255, 255, 255, 255, 255, 255, 255, 255 },
            { 238, 253, 254, 254, 255, 255, 255, 255, 255, 255, 255 },
        },
        {
            { 255, 248, 254, 255, 255, 255, 255, 255, 255, 255, 255 },
            { 249, 254, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
            { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
        },
        {
            { 255, 253, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
            { 247, 254, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
            { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
        },
        {
            { 255, 253, 254, 255, 255, 255, 255, 255, 255, 255, 255 },
            { 252, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
            { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
        },
        {
            { 255, 254, 254, 255, 255, 255, 255, 255, 255, 255, 255 },
            { 253, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
            { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
        },
        {
            { 255, 254, 253, 255, 255, 255, 255, 255, 255, 255, 255 },
            { 250, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
            { 254, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
        },
        {
            { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
            { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
            { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
        },
    },
    {
        {
            { 186, 251, 250, 255, 255, 255, 255, 255, 255, 255, 255 },
            { 234, 251, 244, 254, 255, 255, 255, 255, 255, 255, 255 },
            { 251, 251, 243, 253, 254, 255, 254, 255, 255, 255, 255 },
        },
        {
            { 255, 253, 254, 255, 255, 255, 255, 255, 255, 255, 255 },
            { 236, 253, 254, 255, 255, 255, 255, 255, 255, 255, 255 },
            { 251, 253, 253, 254, 254, 255, 255, 255, 255, 255, 255 },
        },
        {
            { 255, 254, 254, 255, 255, 255, 255, 255, 255, 255, 255 },
            { 254, 254, 254, 255, 255, 255, 255, 255, 255, 255, 255 },
            { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
        },
        {
            { 255, 254, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
            { 254, 254, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
            { 254, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
        },
        {
            { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
            { 254, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
            { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
        },
        {
            { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
            { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
            { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
        },
        {
            { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
            { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
            { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
        },
        {
            { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
            { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
            { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
        },
    },
    {
        {
            { 248, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
            { 250, 254, 252, 254, 255, 255, 255, 255, 255, 255, 255 },
            { 248, 254, 249, 253, 255, 255, 255, 255, 255, 255, 255 },
        },
        {
            { 255, 253, 253, 255, 255, 255, 255, 255, 255, 255, 255 },
            { 246, 253, 253, 255, 255, 255, 255, 255, 255, 255, 255 },
            { 252, 254, 251, 254, 254, 255, 255, 255, 255, 255, 255 },
        },
        {
            { 255, 254, 252, 255, 255, 255, 255, 255, 255, 255, 255 },
            { 248, 254, 253, 255, 255, 255, 255, 255, 255, 255, 255 },
            { 253, 255, 254, 254, 255, 255, 255, 255, 255, 255, 255 },
        },
        {
            { 255, 251, 254, 255, 255, 255, 255, 255, 255, 255, 255 },
            { 245, 251, 254, 255, 255, 255, 255, 255, 255, 255, 255 },
            { 253, 253, 254, 255, 255, 255, 255, 255, 255, 255, 255 },
        },
        {
            { 255, 251, 253, 255, 255, 255, 255, 255, 255, 255, 255 },
            { 252, 253, 254, 255, 255, 255, 255, 255, 255, 255, 255 },
            { 255, 254, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
        },
        {
            { 255, 252, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
            { 249, 255, 254, 255, 255, 255, 255, 255, 255, 255, 255 },
            { 255, 255, 254, 255, 255, 255, 255, 255, 255, 255, 255 },
        },
        {
            { 255, 255, 253, 255, 255, 255, 255, 255, 255, 255, 255 },
            { 250, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
            { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
        },
        {
            { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
            { 254, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
            { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
        },
    },
};

// fixme: copied from h264data.h
static const uint8_t zigzag_scan[16]={
    0 + 0 * 4, 1 + 0 * 4, 0 + 1 * 4, 0 + 2 * 4,
    1 + 1 * 4, 2 + 0 * 4, 3 + 0 * 4, 2 + 1 * 4,
    1 + 2 * 4, 0 + 3 * 4, 1 + 3 * 4, 2 + 2 * 4,
    3 + 1 * 4, 3 + 2 * 4, 2 + 3 * 4, 3 + 3 * 4,
};

static const uint8_t vp8_dc_qlookup[VP8_MAX_QUANT + 1] = {
      4,   5,   6,   7,   8,   9,  10,  10,  11,  12,  13,  14,  15,  16,  17,  17,
     18,  19,  20,  20,  21,  21,  22,  22,  23,  23,  24,  25,  25,  26,  27,  28,
     29,  30,  31,  32,  33,  34,  35,  36,  37,  37,  38,  39,  40,  41,  42,  43,
     44,  45,  46,  46,  47,  48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,
     59,  60,  61,  62,  63,  64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,
     75,  76,  76,  77,  78,  79,  80,  81,  82,  83,  84,  85,  86,  87,  88,  89,
     91,  93,  95,  96,  98, 100, 101, 102, 104, 106, 108, 110, 112, 114, 116, 118,
    122, 124, 126, 128, 130, 132, 134, 136, 138, 140, 143, 145, 148, 151, 154, 157,
};

static const uint16_t vp8_ac_qlookup[VP8_MAX_QUANT + 1] = {
      4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,  16,  17,  18,  19,
     20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,  32,  33,  34,  35,
     36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,  48,  49,  50,  51,
     52,  53,  54,  55,  56,  57,  58,  60,  62,  64,  66,  68,  70,  72,  74,  76,
     78,  80,  82,  84,  86,  88,  90,  92,  94,  96,  98, 100, 102, 104, 106, 108,
    110, 112, 114, 116, 119, 122, 125, 128, 131, 134, 137, 140, 143, 146, 149, 152,
    155, 158, 161, 164, 167, 170, 173, 177, 181, 185, 189, 193, 197, 201, 205, 209,
    213, 217, 221, 225, 229, 234, 239, 245, 249, 254, 259, 264, 269, 274, 279, 284,
};

static const uint8_t vp8_mv_update_prob[2][19] = {
    { 237,
      246,
      253, 253, 254, 254, 254, 254, 254,
      254, 254, 254, 254, 254, 250, 250, 252, /* VP8 only: */ 254, 254 },
    { 231,
      243,
      245, 253, 254, 254, 254, 254, 254,
      254, 254, 254, 254, 254, 251, 251, 254, /* VP8 only: */ 254, 254 }
};

static const uint8_t vp7_mv_default_prob[2][17] = {
    { 162,
      128,
      225, 146, 172, 147, 214,  39, 156,
      247, 210, 135,  68, 138, 220, 239, 246 },
    { 164,
      128,
      204, 170, 119, 235, 140, 230, 228,
      244, 184, 201,  44, 173, 221, 239, 253 }
};

static const uint8_t vp8_mv_default_prob[2][19] = {
    { 162,
      128,
      225, 146, 172, 147, 214, 39, 156,
      128, 129, 132,  75, 145, 178, 206, 239, 254, 254 },
    { 164,
      128,
      204, 170, 119, 235, 140, 230, 228,
      128, 130, 130,  74, 148, 180, 203, 236, 254, 254 }
};

static const uint8_t vp7_feature_value_size[2][4] = {
    { 7, 6, 0, 8 },
    { 7, 6, 0, 5 },
};

static const int8_t vp7_feature_index_tree[4][2] =
{
    {  1,  2 },
    { -0, -1 }, // '00', '01'
    { -2, -3 }, // '10', '11'
};

static const uint16_t vp7_ydc_qlookup[] = {
      4,   4,   5,   6,   6,   7,   8,   8,   9,  10,  11,  12,  13,  14,  15,
     16,  17,  18,  19,  20,  21,  22,  23,  23,  24,  25,  26,  27,  28,  29,
     30,  31,  32,  33,  33,  34,  35,  36,  36,  37,  38,  39,  39,  40,  41,
     41,  42,  43,  43,  44,  45,  45,  46,  47,  48,  48,  49,  50,  51,  52,
     53,  53,  54,  56,  57,  58,  59,  60,  62,  63,  65,  66,  68,  70,  72,
     74,  76,  79,  81,  84,  87,  90,  93,  96, 100, 104, 108, 112, 116, 121,
    126, 131, 136, 142, 148, 154, 160, 167, 174, 182, 189, 198, 206, 215, 224,
    234, 244, 254, 265, 277, 288, 301, 313, 327, 340, 355, 370, 385, 401, 417,
    434, 452, 470, 489, 509, 529, 550, 572,
};

static const uint16_t vp7_yac_qlookup[] = {
       4,    4,   5,   5,   6,   6,   7,   8,   9,  10,  11,  12,   13,   15,
      16,   17,  19,  20,  22,  23,  25,  26,  28,  29,  31,  32,   34,   35,
      37,   38,  40,  41,  42,  44,  45,  46,  48,  49,  50,  51,   53,   54,
      55,   56,  57,  58,  59,  61,  62,  63,  64,  65,  67,  68,   69,   70,
      72,   73,  75,  76,  78,  80,  82,  84,  86,  88,  91,  93,   96,   99,
     102,  105, 109, 112, 116, 121, 125, 130, 135, 140, 146, 152,  158,  165,
     172,  180, 188, 196, 205, 214, 224, 234, 245, 256, 268, 281,  294,  308,
     322,  337, 353, 369, 386, 404, 423, 443, 463, 484, 506, 529,  553,  578,
     604,  631, 659, 688, 718, 749, 781, 814, 849, 885, 922, 960, 1000, 1041,
    1083, 1127,
};

static const uint16_t vp7_y2dc_qlookup[] = {
       7,    9,  11,  13,  15,  17,  19,  21,  23,  26,  28,  30,   33,   35,
      37,   39,  42,  44,  46,  48,  51,  53,  55,  57,  59,  61,   63,   65,
      67,   69,  70,  72,  74,  75,  77,  78,  80,  81,  83,  84,   85,   87,
      88,   89,  90,  92,  93,  94,  95,  96,  97,  99, 100, 101,  102,  104,
     105,  106, 108, 109, 111, 113, 114, 116, 118, 120, 123, 125,  128,  131,
     134,  137, 140, 144, 148, 152, 156, 161, 166, 171, 176, 182,  188,  195,
     202,  209, 217, 225, 234, 243, 253, 263, 274, 285, 297, 309,  322,  336,
     350,  365, 381, 397, 414, 432, 450, 470, 490, 511, 533, 556,  579,  604,
     630,  656, 684, 713, 742, 773, 805, 838, 873, 908, 945, 983, 1022, 1063,
    1105, 1148,
};

static const uint16_t vp7_y2ac_qlookup[] = {
       7,    9,   11,   13,   16,   18,   21,   24,   26,   29,   32,   35,
      38,   41,   43,   46,   49,   52,   55,   58,   61,   64,   66,   69,
      72,   74,   77,   79,   82,   84,   86,   88,   91,   93,   95,   97,
      98,  100,  102,  104,  105,  107,  109,  110,  112,  113,  115,  116,
     117,  119,  120,  122,  123,  125,  127,  128,  130,  132,  134,  136,
     138,  141,  143,  146,  149,  152,  155,  158,  162,  166,  171,  175,
     180,  185,  191,  197,  204,  210,  218,  226,  234,  243,  252,  262,
     273,  284,  295,  308,  321,  335,  350,  365,  381,  398,  416,  435,
     455,  476,  497,  520,  544,  569,  595,  622,  650,  680,  711,  743,
     776,  811,  848,  885,  925,  965, 1008, 1052, 1097, 1144, 1193, 1244,
    1297, 1351, 1407, 1466, 1526, 1588, 1652, 1719,
};

#endif /* AVCODEC_VP8DATA_H */
