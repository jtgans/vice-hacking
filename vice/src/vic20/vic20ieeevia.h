/*
 * vic20ieeevia.h - IEEE488 interface VIA emulation.
 *
 * Written by
 *  Andre' Fachat <fachat@physik.tu-chemnitz.de>
 *  Andreas Boose <viceteam@t-online.de>
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

#ifndef _VIC20IEEEVIA_H
#define _VIC20IEEEVIA_H

#include "types.h"

struct machine_context_s;
struct snapshot_s;
struct via_context_s;

extern void vic20ieeevia1_setup_context(struct machine_context_s
                                        *machine_context);
extern void ieeevia1_init(struct via_context_s *via_context);
extern void ieeevia1_reset(struct via_context_s *via_context);
extern void ieeevia1_signal(struct via_context_s *via_context, int line,
                            int edge);
extern void REGPARM2 ieeevia1_store(WORD addr, BYTE byte);
extern BYTE REGPARM1 ieeevia1_read(WORD addr);
extern BYTE REGPARM1 ieeevia1_peek(WORD addr);
extern int ieeevia1_snapshot_write_module(struct via_context_s *via_context,
                                          struct snapshot_s *p);
extern int ieeevia1_snapshot_read_module(struct via_context_s *via_context,
                                         struct snapshot_s *p);

extern void vic20ieeevia2_setup_context(struct machine_context_s
                                        *machine_context);
extern void ieeevia2_init(struct via_context_s *via_context);
extern void ieeevia2_reset(struct via_context_s *via_context);
extern void ieeevia2_signal(struct via_context_s *via_context, int line,
                            int edge);
extern void REGPARM2 ieeevia2_store(WORD addr, BYTE byte);
extern BYTE REGPARM1 ieeevia2_read(WORD addr);
extern BYTE REGPARM1 ieeevia2_peek(WORD addr);
extern int ieeevia2_snapshot_write_module(struct via_context_s *via_context,
                                          struct snapshot_s *p);
extern int ieeevia2_snapshot_read_module(struct via_context_s *via_context,
                                         struct snapshot_s *p);

extern void ieeevia2_set_tape_sense(int v);

#endif

