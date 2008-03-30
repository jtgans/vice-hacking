/*
 * statusbar.c - Status bar code.
 *
 * Written by
 *  Tibor Biczo <crown@mtavnet.hu>
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
#include <windows.h>
#include <tchar.h>
#ifdef HAVE_COMMCTRL_H
#include <commctrl.h>
#endif

#include "datasette.h"
#include "drive.h"
#include "lib.h"
#include "res.h"
#include "system.h"
#include "ui.h"
#include "statusbar.h"


static HWND status_hwnd[2];
static int number_of_status_windows = 0;
static int status_height;

static unsigned int enabled_drives;
static ui_drive_enable_t    status_enabled;
static int status_led[DRIVE_NUM];
/* Translate from window index -> drive index */
static int status_map[DRIVE_NUM];
/* Translate from drive index -> window index */
static int status_partindex[DRIVE_NUM];
static double status_track[DRIVE_NUM];
static int *drive_active_led;

static int tape_enabled = 0;
static int tape_motor;
static int tape_counter;
static int tape_control;

static BYTE joyport[3] = { 0, 0, 0 };

static int event_part;
static int event_mode;
static unsigned int event_time_current, event_time_total;

static char  emu_status_text[1024];
static TCHAR st_emu_status_text[1024];

static HBRUSH b_red;
static HBRUSH b_green;
static HBRUSH b_black;
static HBRUSH b_yellow;
static HBRUSH b_grey;


static void SetStatusWindowParts(HWND hwnd)
{
    int last_part;
    RECT rect;
    int *posx;
    int width;
    int i;
    int disk_update_part;

    /* one part for statusinfo, one for joystick and tape */
    last_part = 2;

    /* the disk parts */
    enabled_drives = 0;
    for (i = 0; i < DRIVE_NUM; i++) {
        int the_drive = 1 << i;

        if (status_enabled & the_drive) {
            status_map[enabled_drives++] = i;
            if (enabled_drives & 1) {
                last_part++;
                status_map[enabled_drives] = -1;
            }
            status_partindex[i] = last_part - 1;
        }
    }
    disk_update_part = last_part - 1;

    /* the event history part */
    if (event_mode != EVENT_OFF) {
        event_part = last_part;
        last_part++;
    }

    posx = lib_malloc(last_part * sizeof(int));
    i = last_part - 1;
    GetWindowRect(hwnd, &rect);
    width = rect.right-rect.left;

    if (event_mode != EVENT_OFF) {
        posx[i--] = width;
        width -= 80;
    }

    while(i > 0) {
        posx[i--] = width;
        width -= 70;
    }

    posx[0] = width - 20;

    SendMessage(hwnd, SB_SETPARTS, last_part, (LPARAM)posx);
    SendMessage(hwnd, SB_SETTEXT, disk_update_part|SBT_OWNERDRAW,0);
    SendMessage(hwnd, SB_SETTEXT, 1|SBT_OWNERDRAW, 0);
    SendMessage(hwnd, SB_SETTEXT, last_part - 1|SBT_OWNERDRAW,0);

    lib_free(posx);
}


void statusbar_create(HWND hwnd)
{
    RECT rect;

    status_hwnd[number_of_status_windows] =
        CreateStatusWindow(WS_CHILD | WS_VISIBLE, TEXT(""), hwnd,
                           IDM_STATUS_WINDOW);
    SendMessage(status_hwnd[number_of_status_windows], SB_SETMINHEIGHT, 40, (LPARAM)0);
    SendMessage(status_hwnd[number_of_status_windows], WM_SIZE, 0, (LPARAM)0);
    
    GetClientRect(status_hwnd[number_of_status_windows], &rect);
    status_height = rect.bottom - rect.top;
    SetStatusWindowParts(status_hwnd[number_of_status_windows]);
    number_of_status_windows++;
}

void statusbar_destroy(void)
{
    int i;

    for (i = 0; i < number_of_status_windows; i++) {
        DestroyWindow(status_hwnd[i]);
    }
    status_height = 0;
    number_of_status_windows = 0;
}

void statusbar_create_brushes(void)
{
    b_green = CreateSolidBrush(0xff00);
    b_red = CreateSolidBrush(0xff);
    b_black = CreateSolidBrush(0x00);
    b_yellow = CreateSolidBrush(0xffff);
    b_grey = CreateSolidBrush(0x808080);
}

int statusbar_get_status_height(void)
{
    return status_height;
}

void statusbar_setstatustext(const char *text)
{
    int i;

    strcpy(emu_status_text, text);
    for (i = 0; i < number_of_status_windows; i++) {
        SendMessage(status_hwnd[i], SB_SETTEXT, 0 | SBT_OWNERDRAW, 0);
    }
}

void statusbar_enable_drive_status(ui_drive_enable_t enable,
                                   int *drive_led_color)
{
    int i;

    status_enabled = enable;
    drive_active_led = drive_led_color;
    for (i = 0; i < number_of_status_windows; i++) {
        SetStatusWindowParts(status_hwnd[i]);
    }
}

void statusbar_display_drive_track(int drivenum, int drive_base,
                                   double track_number)
{
    int i;

    status_track[drivenum] = track_number;
    for (i = 0; i < number_of_status_windows; i++) {
        SendMessage(status_hwnd[i], SB_SETTEXT,
                    (status_partindex[drivenum]) | SBT_OWNERDRAW, 0);
    }
}


void statusbar_display_drive_led(int drivenum, int status)
{
    int i;

    status_led[drivenum] = status;
    for (i = 0; i < number_of_status_windows; i++) {
        SendMessage(status_hwnd[i], SB_SETTEXT,
                    (status_partindex[drivenum]) | SBT_OWNERDRAW, 0);
    }
}

void statusbar_set_tape_status(int tape_status)
{
    int i;

    tape_enabled = tape_status;
    for (i = 0; i < number_of_status_windows; i++) {
        SendMessage(status_hwnd[i], SB_SETTEXT, 1 | SBT_OWNERDRAW, 0);
    }
}

void statusbar_display_tape_motor_status(int motor)
{   
    int i;

    tape_motor = motor;
    for (i = 0; i < number_of_status_windows; i++) {
        SendMessage(status_hwnd[i], SB_SETTEXT, 1 | SBT_OWNERDRAW, 0);
    }
}

void statusbar_display_tape_control_status(int control)
{
    int i;

    tape_control = control;
    for (i = 0; i < number_of_status_windows; i++) {
        SendMessage(status_hwnd[i], SB_SETTEXT, 1 | SBT_OWNERDRAW, 0);
    }
}

void statusbar_display_tape_counter(int counter)
{
    int i;

    if (counter != tape_counter) {
        tape_counter = counter;
        for (i = 0; i < number_of_status_windows; i++) {
            SendMessage(status_hwnd[i], SB_SETTEXT, 1 | SBT_OWNERDRAW, 0);
        }
    }
}

void statusbar_display_joyport(BYTE *joystick_status)
{
    int i;

    joyport[1] = joystick_status[1];
    joyport[2] = joystick_status[2];
    for (i = 0; i < number_of_status_windows; i++)
        SendMessage(status_hwnd[i], SB_SETTEXT, 1 | SBT_OWNERDRAW, 0);
}

void statusbar_event_status(int mode)
{
    int i;

    event_time_current = 0;
    event_time_total = 0;
    event_mode = mode;
    for (i = 0; i < number_of_status_windows; i++) {
        SetStatusWindowParts(status_hwnd[i]);
    }
}

void statusbar_event_time(unsigned int current, unsigned int total)
{
    int i;

    event_time_current = current;
    event_time_total = total;
    for (i = 0; i < number_of_status_windows; i++)
        SendMessage(status_hwnd[i], SB_SETTEXT, event_part | SBT_OWNERDRAW, 0);
}

void statusbar_handle_WMSIZE(UINT msg, WPARAM wparam, LPARAM lparam,
                             int window_index)
{
    SendMessage(status_hwnd[window_index], msg, wparam, lparam);
    SetStatusWindowParts(status_hwnd[window_index]);
}

void statusbar_handle_WMDRAWITEM(WPARAM wparam, LPARAM lparam)
{
    RECT led;
    TCHAR text[256];

    if (wparam == IDM_STATUS_WINDOW) {
        int part_top = ((DRAWITEMSTRUCT*)lparam)->rcItem.top;
        int part_left = ((DRAWITEMSTRUCT*)lparam)->rcItem.left;
        HDC hDC = ((DRAWITEMSTRUCT*)lparam)->hDC;
        UINT itemID = ((DRAWITEMSTRUCT*)lparam)->itemID;

        SetBkColor(hDC, (COLORREF)GetSysColor(COLOR_3DFACE));
        SetTextColor(hDC, (COLORREF)GetSysColor(COLOR_MENUTEXT));

        if (itemID == 0) {
            /* it's the status info */
            led = ((DRAWITEMSTRUCT*)lparam)->rcItem;
            led.left += 2;
            led.right -= 2;
            led.top += 2;
            led.bottom -= 2;
            system_mbstowcs(st_emu_status_text, emu_status_text, 1024);
            DrawText(hDC, st_emu_status_text, -1, &led, DT_WORDBREAK);
            return;
        }
        if (itemID == 1) {
            const int offset_x[] = { 5, 0, -5, 10, -5 };
            const int offset_y[] = { 0, 10, -5, 0, 0 };
            int dir_index, joynum;

            if (tape_enabled) {
                /* tape status */
                POINT tape_control_sign[3];

                /* the leading "Tape:" */
                led.top = part_top + 2;
                led.bottom = part_top + 18;
                led.left = part_left + 2;
                led.right = part_left + 34;
                DrawText(hDC, TEXT("Tape:"), -1, &led, 0);

                /* the tape-motor */
                led.top = part_top + 1;
                led.bottom = part_top + 15;
                led.left = part_left + 36;
                led.right = part_left + 50;
                FillRect(hDC, &led, tape_motor ? b_yellow : b_grey);

                /* the tape-control */
                led.top += 3;
                led.bottom -= 3;
                led.left += 3;
                led.right -= 3;
                tape_control_sign[0].x = led.left;
                tape_control_sign[1].x = led.left+4;
                tape_control_sign[2].x = led.left;
                tape_control_sign[0].y = led.top;
                tape_control_sign[1].y = led.top+4;
                tape_control_sign[2].y = led.top+8;
                switch (tape_control) {
                case DATASETTE_CONTROL_STOP:
                    FillRect(hDC, &led, b_black);
                    break;
                case DATASETTE_CONTROL_START:
                case DATASETTE_CONTROL_RECORD:
                    SelectObject(hDC, b_black);
                    Polygon(hDC, tape_control_sign, 3);
                    if (tape_control == DATASETTE_CONTROL_RECORD) {
                        SelectObject(hDC, b_red);
                        Ellipse(hDC, led.left + 16, led.top + 1,
                                led.left + 23, led.top + 8);
                    }
                    break;
                case DATASETTE_CONTROL_REWIND:
                    tape_control_sign[0].x += 4;
                    tape_control_sign[1].x -= 4;
                    tape_control_sign[2].x += 4;
                case DATASETTE_CONTROL_FORWARD:
                    Polyline(hDC, tape_control_sign, 3);
                    tape_control_sign[0].x += 4;
                    tape_control_sign[1].x += 4;
                    tape_control_sign[2].x += 4;
                    Polyline(hDC, tape_control_sign, 3);
                }

                /* the tape-counter */
                led.top = part_top + 2;
                led.bottom = part_top + 18;
                led.left = part_left + 65;
                led.right = part_left + 100;
                _stprintf(text, TEXT("%03i"), tape_counter);
                DrawText(hDC, text, -1, &led, 0);
            }

            /* the joysticks */
            led.left = part_left + 2;
            led.right = part_left + 48;
            led.top = part_top + 22;
            led.bottom = part_top + 38;

            DrawText(hDC, TEXT("Joystick:"), -1, &led, 0);

            for (joynum = 1; joynum <= 2; joynum ++) {

                led.top = part_top + 22;
                led.left = part_left + (joynum - 1) * 18 + 52;
                led.bottom = led.top + 3;
                led.right = led.left + 3;

                for (dir_index = 0; dir_index < 5; dir_index++) {
                    HBRUSH brush;

                    if (joyport[joynum] & (1 << dir_index))
                        brush = (dir_index < 4 ? b_green : b_red);
                    else
                        brush = b_grey;

                    OffsetRect(&led, offset_x[dir_index], offset_y[dir_index]);

                    FillRect(hDC, &led, brush);

                }
            }
            return;
        }
        if (itemID > 1 && itemID <= ((enabled_drives + 3) >> 1)) {
            /* it's a disk part*/
            int y;
            int index = ((itemID - 2) << 1);
            for (y = 0; y < 2 && status_map[index] >= 0; y++, index++) {
                led.top = part_top + 20 * y + 2 ;
                led.bottom = led.top + 16;
                led.left = part_left + 2;
                led.right = part_left + 45;
                _stprintf(text, TEXT("%2d: %.1f"), status_map[index] + 8,
                          status_track[status_map[index]]);
                DrawText(hDC, text, -1, &led, 0);

                led.bottom = led.top + 12;
                led.left = part_left + 47;
                led.right = part_left + 47 + 16;
                FillRect(hDC, &led, status_led[status_map[index]] ? 
                            (drive_active_led[status_map[index]] ? 
                                b_green : b_red ) : b_black);
            }
            return;
        }
        if (itemID > ((enabled_drives + 3) >> 1)) {
            /* it's the event history part */
            switch (event_mode) {
                case EVENT_RECORDING:
                    _stprintf(text, TEXT("Recording\n%02d:%02d"),
                        event_time_current / 60,
                        event_time_current % 60);
                    break;
                case EVENT_PLAYBACK:
                    _stprintf(text, TEXT("Playback\n%02d:%02d (%02d:%02d)"),
                        event_time_current / 60,
                        event_time_current % 60,
                        event_time_total / 60,
                        event_time_total % 60);
                    break;
                default:
                    _stprintf(text, TEXT("Unknown"));
            }
            led = ((DRAWITEMSTRUCT*)lparam)->rcItem;
            led.left += 2;
            led.right -= 2;
            led.top += 2;
            led.bottom -= 2;
            DrawText(hDC, text, -1, &led, DT_WORDBREAK);
        }
    }
}

