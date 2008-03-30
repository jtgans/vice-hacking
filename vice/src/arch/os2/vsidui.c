/*
 * vsidui.c - Implementation of the VSID UI.
 *
 * Written by
 *  Thomas Bretz <tbretz@ph.tum.de>
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

#define INCL_DOSPROCESS // DosSetPriority
#define INCL_WINDIALOGS // WinSendDlgItemMsg
#include "vice.h"

#include <os2.h>

#include <stdlib.h>

#include "dialogs.h"
#include "dlg-vsid.h"
#include "dlg-emulator.h" // hwndVsidEmulator

#include "log.h"
#include "utils.h"
#include "drive.h"            // DRIVE_SYNC_PAL
#include "resources.h"
#include "snippets\pmwin2.h"  // Win*Spin

extern int trigger_shutdown;

HWND hwndVsid=NULLHANDLE;

void vsid_mainloop(VOID *arg)
{
    APIRET rc;

    //
    // get pm anchor, create msg queue
    //
    HAB hab = WinInitialize(0);            // Initialize PM
    HMQ hmq = WinCreateMsgQueue(hab, 0);   // Create Msg Queue

    //
    // open dialog
    //
    hwndVsid = vsid_dialog();

    if (rc=DosSetPriority(PRTYS_THREAD, PRTYC_REGULAR, +1, 0))
        log_message(LOG_DEFAULT, "vsidui.c: Error DosSetPriority (rc=%li)", rc);

    //
    // MAINLOOP
    //
    WinProcessDlg(hwndVsid);

    //
    // WinProcessDlg() does NOT destroy the window on return! Do it here,
    // otherwise the window procedure won't ever get a WM_DESTROY,
    // which we may want :-)
    //
    WinDestroyWindow(hwndVsid);

    //
    // destroy msg queue, release pm anchor
    //
    if (!WinDestroyMsgQueue(hmq))
        log_message(LOG_DEFAULT,"vsidui.c: Error! Destroying Msg Queue.");
    if (!WinTerminate (hab))
        log_message(LOG_DEFAULT,"vsidui.c: Error! Releasing PM anchor.");

    //
    // shutdown emulator thread
    //
    trigger_shutdown = 1;

    DosSleep(5000); // wait 5 seconds
    log_debug("Brutal Exit!");
    exit(0);        // end VICE in all cases
}

int vsid_ui_init(void)
{
    // resources_set_value("SoundDeviceName", (resource_value_t*)"dart2");

    _beginthread(vsid_mainloop, NULL, 0x4000, NULL);

    while (!hwndVsid)
        DosSleep(1);

    log_message(LOG_DEFAULT, "--> SID Player mode <--\n");

    return 0;
}

void vsid_ui_display_irqtype(const char *irq)
{
    char *txt = xmsprintf("Interrupt: %s", irq);
    WinSetDlgItemText(hwndVsid, ID_TIRQ, txt);
    free(txt);
}

void vsid_ui_display_name(const char *name)
{
    WinSetDlgItemText(hwndVsid, ID_TNAME, (char*)name);
}

void vsid_ui_display_author(const char *author)
{
    WinSetDlgItemText(hwndVsid, ID_TAUTHOR, (char*)author);
}

void vsid_ui_display_copyright(const char *copyright)
{
    WinSetDlgItemText(hwndVsid, ID_TCOPYRIGHT, (char*)copyright);
}

void vsid_ui_display_sync(int sync)
{
    char *txt;
    switch (sync)
    {
    case DRIVE_SYNC_PAL:
        txt = "Synchronization: PAL";
        break;
    case DRIVE_SYNC_NTSC:
        txt =  "Synchronization: NTSC";
        break;
    case DRIVE_SYNC_NTSCOLD:
        txt =  "Synchronization: NTSC (old)";
        break;
    }
    WinSetDlgItemText(hwndVsid, ID_TSYNC, txt);
    //log_debug(txt);
}

void vsid_ui_display_sid_model(int model)
{
    WinSetDlgItemText(hwndVsid, ID_TSID,
                      model ? "SID-Chip: MOS8580" : "SID-Chip: MOS6581");
}

void vsid_ui_set_default_tune(int nr)
{
    //log_debug("Default tune is #%d", nr);
}

void vsid_ui_display_tune_nr(int nr)
{
    char txt[3]="-";
    if (nr<100)
        sprintf(txt, "%d", nr);
    WinSetDlgItemText(hwndVsid, ID_TUNENO, txt);
    WinSetDlgSpinVal(hwndVsid, SPB_SETTUNE, nr);

    //log_debug("Playing tune no.%d", nr);
}

void vsid_ui_display_nr_of_tunes(int count)
{
    char txt[3]="-";
    if (count<100)
        sprintf(txt, "%d", count);
    WinSetDlgItemText(hwndVsid, ID_TUNES, txt);

    //log_debug("File contains %d tunes.", count);
}

void vsid_ui_display_time(unsigned int sec)
{
    char txt[6]="--:--";
    if (sec<600)
        sprintf(txt, "%02d:%02d", (sec/60)%100, sec%60);
    WinSetDlgItemText(hwndVsid, ID_TIME, txt);
}
