/*
 * drivecpu0.c - Template file of the 6502 processor in the Commodore
 * 1541, 1541-II, 1571, 1581 and 2031 floppy disk drive.
 *
 * Written by
 *  Ettore Perazzoli <ettore@comm2000.it>
 *  Andreas Boose <boose@linux.rz.fh-hannover.de>
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

#define DRIVE_CPU

#define mynumber 0

/* Define this to enable tracing of drive CPU instructions.
   Warning: this slows it down!  */
#undef TRACE

/* Force `TRACE' in unstable versions.  */
#if 0 && defined UNSTABLE && !defined TRACE
#define TRACE
#endif

#ifdef TRACE
/* Flag: do we trace instructions while they are executed?  */
int drive0_traceflg;
#endif

/* snapshot name */
#define	MYCPU_NAME	"DRIVECPU0"

#define mydrive_alarm_context drive0_alarm_context
#define mydrive_rmw_flag drive0_rmw_flag
#define mydrive_bank_read drive0_bank_read
#define mydrive_bank_peek drive0_bank_peek
#define mydrive_bank_store drive0_bank_store
#define mydrive_toggle_watchpoints drive0_toggle_watchpoints

#define mydrive_read drive0_read
#define mydrive_store drive0_store
#define mydrive_trigger_reset drive0_trigger_reset
#define mydrive_set_bank_base drive0_set_bank_base
#define mydrive_monitor_interface drive0_monitor_interface
#define mydrive_cpu_execute drive0_cpu_execute
#define mydrive_int_status drive0_int_status
#define mydrive_mem_init drive0_mem_init
#define mydrive_cpu_init drive0_cpu_init
#define mydrive_cpu_wake_up drive0_cpu_wake_up
#define mydrive_cpu_sleep drive0_cpu_sleep
#define mydrive_cpu_reset drive0_cpu_reset
#define mydrive_cpu_prevent_clk_overflow drive0_cpu_prevent_clk_overflow
#define mydrive_cpu_set_sync_factor drive0_cpu_set_sync_factor
#define mydrive_cpu_write_snapshot_module drive0_cpu_write_snapshot_module
#define mydrive_cpu_read_snapshot_module drive0_cpu_read_snapshot_module
#define mydrive_traceflg drive0_traceflg
#define mydrive_clk_guard drive0_clk_guard
#define mydrive_cpu_early_init drive0_cpu_early_init
#define mydrive_cpu_reset_clk drive0_cpu_reset_clk

#define mymonspace e_disk8_space
#define IDENTIFICATION_STRING "DRIVE#8"

#define myvia1_reset via1d0_reset
#define myvia2_reset via2d0_reset
#define mycia1571_reset cia1571d0_reset
#define mycia1581_reset cia1581d0_reset
#define mywd1770_reset wd1770d0_reset
#define myriot1_reset riot1d0_reset
#define myriot2_reset riot2d0_reset

#define myvia1_read via1d0_read
#define myvia1_store via1d0_store
#define myvia2_read via2d0_read
#define myvia2_store via2d0_store
#define mycia1571_read cia1571d0_read
#define mycia1571_store cia1571d0_store
#define mycia1581_read cia1581d0_read
#define mycia1581_store cia1581d0_store
#define mywd1770_read wd1770d0_read
#define mywd1770_store wd1770d0_store
#define myriot1_read riot1d0_read
#define myriot1_store riot1d0_store
#define myriot2_read riot2d0_read
#define myriot2_store riot2d0_store

#define myvia1_init via1d0_init
#define myvia2_init via2d0_init
#define mycia1571_init cia1571d0_init
#define mycia1581_init cia1581d0_init
#define mywd1770_init wd1770d0_init
#define myriot1_init riot1d0_init
#define myriot2_init riot2d0_init

#define	myfdc_reset fdc0_reset
#define	myfdc_init fdc0_init

#include "drivecpucore.c"

