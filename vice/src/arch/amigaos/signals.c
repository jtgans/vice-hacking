/*
 * signals.c
 *
 * Written by
 *  Mathias Roslund <vice.emu@amidog.se>
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

#ifdef AMIGA_AROS
#define __AROS_OFF_T_DECLARED
#define __AROS_PID_T_DECLARED
#endif

#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

#include "log.h"
#include "monitor.h"
#include "signals.h"


static RETSIGTYPE break64(int sig)
{
#ifdef SYS_SIGLIST_DECLARED
    log_message(LOG_DEFAULT, _("Received signal %d (%s)."),
                sig, sys_siglist[sig]);
#else
    log_message(LOG_DEFAULT, _("Received signal %d."), sig);
#endif

    exit (-1);
}

void signals_init(int do_core_dumps)
{
    signal(SIGINT, break64);
    signal(SIGTERM, break64);

    if (!do_core_dumps) {
        signal(SIGSEGV,  break64);
        signal(SIGILL,   break64);
    }
}

typedef void (*signal_handler_t)(int);

static signal_handler_t old_handler;

static void handle_abort(int signo)
{
    monitor_abort();
    signal(SIGINT, (signal_handler_t)handle_abort);
}

void signals_abort_set(void)
{
    old_handler = signal(SIGINT, handle_abort);
}

void signals_abort_unset(void)
{
    signal(SIGINT, old_handler);
}

