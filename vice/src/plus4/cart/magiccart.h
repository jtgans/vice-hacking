
/*
 * magiccart.h - c264 magic cart handling
 *
 * Written by
 *  groepaz <groepaz@gmx.net>
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

#ifndef VICE_MAGICCART_H
#define VICE_MAGICCART_H

#include "types.h"

extern uint8_t magiccart_c1lo_read(uint16_t addr);

extern void magiccart_config_setup(uint8_t *rawcart);
extern int magiccart_bin_attach(const char *filename, uint8_t *rawcart);
extern int magiccart_crt_attach(FILE *fd, uint8_t *rawcart);

extern void magiccart_detach(void);

extern void magiccart_reset(void);

extern int magiccart_snapshot_write_module(snapshot_t *s);
extern int magiccart_snapshot_read_module(snapshot_t *s);

#endif
