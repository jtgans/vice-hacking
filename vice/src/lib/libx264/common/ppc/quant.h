/*****************************************************************************
 * quant.c: ppc quantization
 *****************************************************************************
 * Copyright (C) 2007-2014 x264 project
 *
 * Authors: Guillaume Poirier <gpoirier@mplayerhq.hu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111, USA.
 *
 * This program is also available under a commercial proprietary license.
 * For more information, contact us at licensing@x264.com.
 *****************************************************************************/

#ifndef X264_PPC_QUANT_H
#define X264_PPC_QUANT_H

int x264_quant_4x4_altivec( int16_t dct[16], uint16_t mf[16], uint16_t bias[16] );
int x264_quant_8x8_altivec( int16_t dct[64], uint16_t mf[64], uint16_t bias[64] );

int x264_quant_4x4_dc_altivec( int16_t dct[16], int mf, int bias );
int x264_quant_2x2_dc_altivec( int16_t dct[4], int mf, int bias );

void x264_dequant_4x4_altivec( int16_t dct[16], int dequant_mf[6][16], int i_qp );
void x264_dequant_8x8_altivec( int16_t dct[64], int dequant_mf[6][64], int i_qp );
#endif
