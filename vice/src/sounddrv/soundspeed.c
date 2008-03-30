/*
 * soundspeed.c - Implementation of the speed sound device
 *
 * Written by
 *  Teemu Rantanen (tvr@cs.hut.fi)
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

#ifdef STDC_HEADERS
#include <stdio.h>
#endif

#include "sound.h"

static int speed_write(warn_t *w, SWORD *pbuf, int nr)
{
    return 0;
}

static sound_device_t speed_device =
{
    "speed",
    NULL,
    speed_write,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

int sound_init_speed_device(void)
{
    return sound_register_device(&speed_device);
}
