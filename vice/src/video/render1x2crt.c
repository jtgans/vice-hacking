/*
 * render1x2crt.c - 1x2 CRT renderers
 *
 * Written by
 *  groepaz <groepaz@gmx.net> based on the pal renderers written by
 *  John Selck <graham@cruise.de>
 *
 * This file is part of VICE, the Versatile Commodore Emulator.
 * See README for copyright notice.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *  02111-1307  USA.
 *
 */

#include "vice.h"

#include <stdio.h>

#include "render1x2.h"
#include "render1x2crt.h"
#include "types.h"
#include "uimenu.h"
#include "video-color.h"

/*
    this is the simpliest possible CRT emulation, meaning blur and scanlines only.

    TODO: use RGB color space
*/

static inline
void yuv_to_rgb(int32_t y, int32_t u, int32_t v, int16_t *red, int16_t *grn, int16_t *blu)
{
    *red = (y + v) >> 16;
    *blu = (y + u) >> 16;
    *grn = (y - ((50 * u + 130 * v) >> 8)) >> 16;
}

static inline
void store_line_and_scanline_4(
    video_render_color_tables_t *color_tab,
    uint8_t *const line, uint8_t *const scanline,
    int16_t *const prevline, const int shade, /* ignored by RGB modes */
    const int32_t y, const int32_t u, const int32_t v, const int32_t y2, const int32_t u2, const int32_t v2)
{
    int16_t red, grn, blu;
    uint32_t *tmp1, *tmp2;
    yuv_to_rgb(y, u, v, &red, &grn, &blu);

    tmp1 = (uint32_t *) scanline;
    tmp2 = (uint32_t *) line;
    *tmp1++ = color_tab->gamma_red_fac[512 + red + prevline[0]]
              | color_tab->gamma_grn_fac[512 + grn + prevline[1]]
              | color_tab->gamma_blu_fac[512 + blu + prevline[2]]
              | color_tab->alpha;
    *tmp2++ = color_tab->gamma_red[256 + red]
              | color_tab->gamma_grn[256 + grn]
              | color_tab->gamma_blu[256 + blu]
              | color_tab->alpha;

    prevline[0] = red;
    prevline[1] = grn;
    prevline[2] = blu;

    yuv_to_rgb(y2, u2, v2, &red, &grn, &blu);

    *tmp1 = color_tab->gamma_red_fac[512 + red + prevline[3]]
            | color_tab->gamma_grn_fac[512 + grn + prevline[4]]
            | color_tab->gamma_blu_fac[512 + blu + prevline[5]]
            | color_tab->alpha;
    *tmp2 = color_tab->gamma_red[256 + red]
            | color_tab->gamma_grn[256 + grn]
            | color_tab->gamma_blu[256 + blu]
            | color_tab->alpha;

    prevline[3] = red;
    prevline[4] = grn;
    prevline[5] = blu;
}

static inline
void get_yuv_from_video(
    const int32_t unew, const int32_t vnew,
    const int off_flip,
    int32_t *const u, int32_t *const v)
{
    *u = (unew) * off_flip;
    *v = (vnew) * off_flip;
}

static inline
void render_generic_1x2_crt(video_render_color_tables_t *color_tab,
                            const uint8_t *src, uint8_t *trg,
                            unsigned int width, const unsigned int height,
                            unsigned int xs, const unsigned int ys,
                            unsigned int xt, const unsigned int yt,
                            const unsigned int pitchs, const unsigned int pitcht,
                            viewport_t *viewport, unsigned int pixelstride,
                            const int write_interpolated_pixels, video_render_config_t *config)
{
    int16_t *prevrgblineptr;
    const int32_t *ytablel = color_tab->ytablel;
    const int32_t *ytableh = color_tab->ytableh;
    const uint8_t *tmpsrc;
    uint8_t *tmptrg, *tmptrgscanline;
    int32_t *cbtable, *crtable;
    uint32_t x, y, wfirst, wlast, yys;
    int32_t l, u, unew, v, vnew, off_flip, shade;
    int32_t l2 = 0;
    int32_t u2 = 0;
    int32_t v2 = 0;
    int first_line = viewport->first_line * 2;
    int last_line = (viewport->last_line * 2) + 1;

    src = src + pitchs * ys + xs - 2;
    trg = trg + pitcht * yt + xt * pixelstride;
    yys = (ys << 1) | (yt & 1);
    wfirst = xt & 1;
    width -= wfirst;
    wlast = width & 1;
/*    width >>= 1; */

    /* That's all initialization we need for full lines. Unfortunately, for
     * scanlines we also need to calculate the RGB color of the previous
     * full line, and that requires initialization from 2 full lines above our
     * rendering target. We just won't render the scanline above the target row,
     * so you need to call us with 1 line before the desired rectangle, and
     * for one full line after it! */

    /* Calculate odd line shading */
    shade = (int) ((float) config->video_resources.pal_scanlineshade / 1000.0f * 256.f);
    off_flip = 1 << 6;

    /* height & 1 == 0. */
    for (y = yys; y < yys + height + 1; y += 2) {
        /* when we are dealing with the last line, the rules change:
         * we no longer write the main output to screen, we just put it into
         * the scanline. */
        if (y == yys + height) {
            /* no place to put scanline in: we are outside viewport or still
             * doing the first iteration (y == yys), height == 0 */
            if (y == yys || y <= (unsigned int)first_line || y > (unsigned int)(last_line + 1)) {
                break;
            }
            tmptrg = &color_tab->rgbscratchbuffer[0];
            tmptrgscanline = trg - pitcht;
            if (y == (unsigned int)(last_line + 1)) {
                /* src would point after the source area, so rewind one line */
                src -= pitchs;
            }
        } else {
            /* pixel data to surface */
            tmptrg = trg;
            /* write scanline data to previous line if possible,
             * otherwise we dump it to the scratch region... We must never
             * render the scanline for the first row, because prevlinergb is not
             * yet initialized and scanline data would be bogus! */
            tmptrgscanline = y != yys && y > (unsigned int)first_line && y <= (unsigned int)last_line
                             ? trg - pitcht
                             : &color_tab->rgbscratchbuffer[0];
        }

        /* current source image for YUV xform */
        tmpsrc = src;

        cbtable = write_interpolated_pixels ? color_tab->cbtable : color_tab->cutable;
        crtable = write_interpolated_pixels ? color_tab->crtable : color_tab->cvtable;

        l = ytablel[tmpsrc[1]] + ytableh[tmpsrc[2]] + ytablel[tmpsrc[3]];
        unew = cbtable[tmpsrc[0]] + cbtable[tmpsrc[1]] + cbtable[tmpsrc[2]] + cbtable[tmpsrc[3]];
        vnew = crtable[tmpsrc[0]] + crtable[tmpsrc[1]] + crtable[tmpsrc[2]] + crtable[tmpsrc[3]];
        get_yuv_from_video(unew, vnew, off_flip, &u, &v);
        unew -= cbtable[tmpsrc[0]];
        vnew -= crtable[tmpsrc[0]];
        tmpsrc += 1;

        /* actual line */
        prevrgblineptr = &color_tab->prevrgbline[0];
        if (wfirst) {
            l2 = ytablel[tmpsrc[1]] + ytableh[tmpsrc[2]] + ytablel[tmpsrc[3]];
            unew += cbtable[tmpsrc[3]];
            vnew += crtable[tmpsrc[3]];
            get_yuv_from_video(unew, vnew, off_flip, &u2, &v2);
            unew -= cbtable[tmpsrc[0]];
            vnew -= crtable[tmpsrc[0]];
            tmpsrc += 1;
#if 0
            if (write_interpolated_pixels) {
                store_line_and_scanline_4(color_tab, tmptrg, tmptrgscanline, prevrgblineptr, shade, (l + l2) >> 1, (u + u2) >> 1, (v + v2) >> 1);
                tmptrgscanline += pixelstride;
                tmptrg += pixelstride;
                prevrgblineptr += 3;
            }
#endif
            l = l2;
            u = u2;
            v = v2;
        }
        for (x = 0; x < width; x++) {
#if 0
            store_line_and_scanline_4(color_tab, tmptrg, tmptrgscanline, prevrgblineptr, shade, l, u, v);
            tmptrgscanline += pixelstride;
            tmptrg += pixelstride;
            prevrgblineptr += 3;
#endif
            l2 = ytablel[tmpsrc[1]] + ytableh[tmpsrc[2]] + ytablel[tmpsrc[3]];
            unew += cbtable[tmpsrc[3]];
            vnew += crtable[tmpsrc[3]];
            get_yuv_from_video(unew, vnew, off_flip, &u2, &v2);
            unew -= cbtable[tmpsrc[0]];
            vnew -= crtable[tmpsrc[0]];
            tmpsrc += 1;
#if 0
            if (write_interpolated_pixels) {
                store_line_and_scanline_4(color_tab, tmptrg, tmptrgscanline, prevrgblineptr, shade, (l + l2) >> 1, (u + u2) >> 1, (v + v2) >> 1);
                tmptrgscanline += pixelstride;
                tmptrg += pixelstride;
                prevrgblineptr += 3;
            }
#endif
            x++;
            if (x == width) {
                break;
            }
#if 1
            store_line_and_scanline_4(color_tab, tmptrg, tmptrgscanline, prevrgblineptr, shade, l, u, v, l2, u2, v2);
            tmptrgscanline += pixelstride * 2;
            tmptrg += pixelstride * 2;
            prevrgblineptr += 6;
#endif
            l2 = ytablel[tmpsrc[1]] + ytableh[tmpsrc[2]] + ytablel[tmpsrc[3]];
            unew += cbtable[tmpsrc[3]];
            vnew += crtable[tmpsrc[3]];
            get_yuv_from_video(unew, vnew, off_flip, &u2, &v2);
            unew -= cbtable[tmpsrc[0]];
            vnew -= crtable[tmpsrc[0]];
            tmpsrc += 1;

            l = l2;
            u = u2;
            v = v2;
        }
        if (wlast) {
            store_line_and_scanline_4(color_tab, tmptrg, tmptrgscanline, prevrgblineptr, shade, l, u, v, l2, u2, v2);
        }

        src += pitchs;
        trg += pitcht * 2;
    }
}

void render_32_1x2_crt(video_render_color_tables_t *color_tab,
                       const uint8_t *src, uint8_t *trg,
                       unsigned int width, const unsigned int height,
                       const unsigned int xs, const unsigned int ys,
                       const unsigned int xt, const unsigned int yt,
                       const unsigned int pitchs, const unsigned int pitcht,
                       viewport_t *viewport, video_render_config_t *config)
{
    if (config->interlaced) {
        /*
         * The interlaced path doesn't currently support the CRT filter stuff,
         * other than by allowing the previous frame to partially show through
         * the scanlines of the current frame, using a 50% alpha black scanline.
         */
        render_32_1x2_04_interlaced(color_tab, src, trg, width, height, xs, ys,
                                    xt, yt, pitchs, pitcht, config, (color_tab->physical_colors[0] & 0x00ffffff) | 0x7f000000);
    } else {
        render_generic_1x2_crt(color_tab, src, trg, width, height, xs, ys,
                               xt, yt, pitchs, pitcht, viewport,
                               4, 1, config);
    }
}
