/*
 * c64memlimit.h -- Builds the C64 memory limit table.
 *
 * Written by
 *  Andreas Boose <viceteam@t-online.de>
 *  Marco van den Heuvel <blackystardust68@yahoo.com>
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

#ifndef VICE_C64MEMLIMIT_H
#define VICE_C64MEMLIMIT_H
#include "types.h"

#define NUM_CONFIGS 32

extern void mem_limit_init(DWORD mem_read_limit_tab[NUM_CONFIGS][0x101]);
extern void mem_limit_plus60k_init(DWORD mem_read_limit_tab[NUM_CONFIGS][0x101]);
extern void mem_limit_256k_init(DWORD mem_read_limit_tab[NUM_CONFIGS][0x101]);
extern void mem_limit_max_init(DWORD mem_read_limit_tab[NUM_CONFIGS][0x101]);

#endif
