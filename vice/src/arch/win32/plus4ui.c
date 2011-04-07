/*
 * plus4ui.c - PLUS4-specific user interface.
 *
 * Written by
 *  Andreas Boose <viceteam@t-online.de>
 *  Ettore Perazzoli <ettore@comm2000.it>
 *  Tibor Biczo <crown@axelero.hu>
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
#include <windows.h>

#include "debug.h"
#include "plus4ui.h"
#include "res.h"
#include "resources.h"
#include "translate.h"
#include "ui.h"
#include "uiacia.h"
#include "uidriveplus4.h"
#include "uijoystick.h"
#include "uikeyboard.h"
#include "uilib.h"
#include "uiplus4cart.h"
#include "uiplus4mem.h"
#include "uirom.h"
#include "uisidcart.h"
#include "uiv364speech.h"
#include "uivideo.h"
#include "winmain.h"

static const unsigned int romset_dialog_resources[UIROM_TYPE_MAX] = {
    IDD_PLUS4ROM_RESOURCE_DIALOG,
    IDD_PLUS4ROMDRIVE_RESOURCE_DIALOG,
    0
};

static const ui_menu_toggle_t plus4_ui_menu_toggles[] = {
    { "TEDDoubleSize", IDM_TOGGLE_DOUBLESIZE },
    { "TEDDoubleScan", IDM_TOGGLE_DOUBLESCAN },
    { "TEDVideoCache", IDM_TOGGLE_VIDEOCACHE },
    { NULL, 0 }
};

static const uirom_settings_t uirom_settings[] = {
    { UIROM_TYPE_MAIN, TEXT("Kernal"), "KernalName",
      IDC_PLUS4ROM_KERNAL_FILE, IDC_PLUS4ROM_KERNAL_BROWSE,
      IDC_PLUS4ROM_KERNAL_RESOURCE },
    { UIROM_TYPE_MAIN, TEXT("Basic"), "BasicName",
      IDC_PLUS4ROM_BASIC_FILE, IDC_PLUS4ROM_BASIC_BROWSE,
      IDC_PLUS4ROM_BASIC_RESOURCE },
    { UIROM_TYPE_MAIN, TEXT("3 plus 1 LO"), "3plus1loName",
      IDC_PLUS4ROM_3P1LO_FILE, IDC_PLUS4ROM_3P1LO_BROWSE,
      IDC_PLUS4ROM_3P1LO_RESOURCE },
    { UIROM_TYPE_MAIN, TEXT("3 plus 1 HI"), "3plus1hiName",
      IDC_PLUS4ROM_3P1HI_FILE, IDC_PLUS4ROM_3P1HI_BROWSE,
      IDC_PLUS4ROM_3P1HI_RESOURCE },
    { UIROM_TYPE_DRIVE, TEXT("1541"), "DosName1541",
      IDC_DRIVEROM_1541_FILE, IDC_DRIVEROM_1541_BROWSE,
      IDC_DRIVEROM_1541_RESOURCE },
    { UIROM_TYPE_DRIVE, TEXT("1541-II"), "DosName1541ii",
      IDC_DRIVEROM_1541II_FILE, IDC_DRIVEROM_1541II_BROWSE,
      IDC_DRIVEROM_1541II_RESOURCE },
    { UIROM_TYPE_DRIVE, TEXT("1551"), "DosName1551",
      IDC_DRIVEROM_1551_FILE, IDC_DRIVEROM_1551_BROWSE,
      IDC_DRIVEROM_1551_RESOURCE },
    { UIROM_TYPE_DRIVE, TEXT("1570"), "DosName1570",
      IDC_DRIVEROM_1570_FILE, IDC_DRIVEROM_1570_BROWSE,
      IDC_DRIVEROM_1570_RESOURCE },
    { UIROM_TYPE_DRIVE, TEXT("1571"), "DosName1571",
      IDC_DRIVEROM_1571_FILE, IDC_DRIVEROM_1571_BROWSE,
      IDC_DRIVEROM_1571_RESOURCE },
    { UIROM_TYPE_DRIVE, TEXT("1581"), "DosName1581",
      IDC_DRIVEROM_1581_FILE, IDC_DRIVEROM_1581_BROWSE,
      IDC_DRIVEROM_1581_RESOURCE },
    { 0, NULL, NULL, 0, 0, 0 }
};

static const ui_res_value_list_t plus4_ui_res_values[] = {
    { NULL, NULL, 0 }
};

#define PLUS4UI_KBD_NUM_MAP 2

static const uikeyboard_mapping_entry_t mapping_entry[PLUS4UI_KBD_NUM_MAP] = {
    { IDC_PLUS4KBD_MAPPING_SELECT_SYM, IDC_PLUS4KBD_MAPPING_SYM,
      IDC_PLUS4KBD_MAPPING_SYM_BROWSE, "KeymapSymFile" },
    { IDC_PLUS4KBD_MAPPING_SELECT_POS, IDC_PLUS4KBD_MAPPING_POS,
      IDC_PLUS4KBD_MAPPING_POS_BROWSE, "KeymapPosFile" }
};

static uilib_localize_dialog_param plus4_kbd_trans[] = {
    { IDC_PLUS4KBD_MAPPING_SELECT_SYM, IDS_SYMBOLIC, 0 },
    { IDC_PLUS4KBD_MAPPING_SELECT_POS, IDS_POSITIONAL, 0 },
    { IDC_PLUS4KBD_MAPPING_SYM_BROWSE, IDS_BROWSE, 0 },
    { IDC_PLUS4KBD_MAPPING_POS_BROWSE, IDS_BROWSE, 0 },
    { IDC_PLUS4KBD_MAPPING_DUMP, IDS_DUMP_KEYSET, 0 },
    { IDC_KBD_SHORTCUT_DUMP, IDS_DUMP_SHORTCUTS, 0 },
    { 0, 0, 0 }
};

static uilib_dialog_group plus4_kbd_left_group[] = {
    { IDC_PLUS4KBD_MAPPING_SELECT_SYM, 1 },
    { IDC_PLUS4KBD_MAPPING_SELECT_POS, 1 },
    { 0, 0 }
};

static uilib_dialog_group plus4_kbd_middle_group[] = {
    { IDC_PLUS4KBD_MAPPING_SYM, 0 },
    { IDC_PLUS4KBD_MAPPING_POS, 0 },
    { 0, 0 }
};

static uilib_dialog_group plus4_kbd_right_group[] = {
    { IDC_PLUS4KBD_MAPPING_SYM_BROWSE, 0 },
    { IDC_PLUS4KBD_MAPPING_POS_BROWSE, 0 },
    { 0, 0 }
};

static uilib_dialog_group plus4_kbd_buttons_group[] = {
    { IDC_PLUS4KBD_MAPPING_DUMP, 1 },
    { IDC_KBD_SHORTCUT_DUMP, 1 },
    { 0, 0 }
};

static int plus4_kbd_move_buttons_group[] = {
    IDC_PLUS4KBD_MAPPING_DUMP,
    IDC_KBD_SHORTCUT_DUMP,
    0
};

static uikeyboard_config_t uikeyboard_config = {
    IDD_PLUS4KBD_MAPPING_SETTINGS_DIALOG,
    PLUS4UI_KBD_NUM_MAP,
    mapping_entry,
    IDC_PLUS4KBD_MAPPING_DUMP,
    plus4_kbd_trans,
    plus4_kbd_left_group,
    plus4_kbd_middle_group,
    plus4_kbd_right_group,
    plus4_kbd_buttons_group,
    plus4_kbd_move_buttons_group
};

ui_menu_translation_table_t plus4ui_menu_translation_table[] = {
    { IDM_EXIT, IDS_MI_EXIT },
    { IDM_ABOUT, IDS_MI_ABOUT },
    { IDM_HELP, IDS_MP_HELP },
    { IDM_PAUSE, IDS_MI_PAUSE },
    { IDM_EDIT_COPY, IDS_MI_EDIT_COPY },
    { IDM_EDIT_PASTE, IDS_MI_EDIT_PASTE },
    { IDM_AUTOSTART, IDS_MI_AUTOSTART },
    { IDM_RESET_HARD, IDS_MI_RESET_HARD },
    { IDM_RESET_SOFT, IDS_MI_RESET_SOFT },
    { IDM_RESET_DRIVE8, IDS_MI_DRIVE8 },
    { IDM_RESET_DRIVE9, IDS_MI_DRIVE9 },
    { IDM_RESET_DRIVE10, IDS_MI_DRIVE10 },
    { IDM_RESET_DRIVE11, IDS_MI_DRIVE11 },
    { IDM_ATTACH_8, IDS_MI_DRIVE8 },
    { IDM_ATTACH_9, IDS_MI_DRIVE9 },
    { IDM_ATTACH_10, IDS_MI_DRIVE10 },
    { IDM_ATTACH_11, IDS_MI_DRIVE11 },
    { IDM_DETACH_8, IDS_MI_DRIVE8 },
    { IDM_DETACH_9, IDS_MI_DRIVE9 },
    { IDM_DETACH_10, IDS_MI_DRIVE10 },
    { IDM_DETACH_11, IDS_MI_DRIVE11 },
    { IDM_ATTACH_TAPE, IDS_MI_ATTACH_TAPE },
    { IDM_DETACH_TAPE, IDS_MI_DETACH_TAPE },
    { IDM_DETACH_ALL, IDS_MI_DETACH_ALL },
    { IDM_TOGGLE_SOUND, IDS_MI_TOGGLE_SOUND },
    { IDM_TOGGLE_DOUBLESIZE, IDS_MI_TOGGLE_DOUBLESIZE },
    { IDM_TOGGLE_DOUBLESCAN, IDS_MI_TOGGLE_DOUBLESCAN },
    { IDM_TOGGLE_DRIVE_TRUE_EMULATION, IDS_MI_DRIVE_TRUE_EMULATION },
    { IDM_TOGGLE_AUTOSTART_HANDLE_TDE, IDS_MI_AUTOSTART_HANDLE_TDE },
    { IDM_TOGGLE_VIDEOCACHE, IDS_MI_TOGGLE_VIDEOCACHE },
    { IDM_DRIVE_SETTINGS, IDS_MI_DRIVE_SETTINGS },
    { IDM_CART_ATTACH_C1LO, IDS_MI_CART_ATTACH_C1LO },
    { IDM_CART_ATTACH_C1HI, IDS_MI_CART_ATTACH_C1HI },
    { IDM_CART_ATTACH_C2LO, IDS_MI_CART_ATTACH_C2LO },
    { IDM_CART_ATTACH_C2HI, IDS_MI_CART_ATTACH_C2HI },
    { IDM_CART_ATTACH_FUNCLO, IDS_MI_CART_ATTACH_FUNCLO },
    { IDM_CART_ATTACH_FUNCHI, IDS_MI_CART_ATTACH_FUNCHI },
    { IDM_FLIP_ADD, IDS_MI_FLIP_ADD },
    { IDM_FLIP_REMOVE, IDS_MI_FLIP_REMOVE },
    { IDM_FLIP_NEXT, IDS_MI_FLIP_NEXT },
    { IDM_FLIP_PREVIOUS, IDS_MI_FLIP_PREVIOUS },
    { IDM_FLIP_LOAD, IDS_MI_FLIP_LOAD },
    { IDM_FLIP_SAVE, IDS_MI_FLIP_SAVE },
    { IDM_DATASETTE_CONTROL_STOP, IDS_MI_DATASETTE_STOP },
    { IDM_DATASETTE_CONTROL_START, IDS_MI_DATASETTE_START },
    { IDM_DATASETTE_CONTROL_FORWARD, IDS_MI_DATASETTE_FORWARD },
    { IDM_DATASETTE_CONTROL_REWIND, IDS_MI_DATASETTE_REWIND },
    { IDM_DATASETTE_CONTROL_RECORD, IDS_MI_DATASETTE_RECORD },
    { IDM_DATASETTE_CONTROL_RESET, IDS_MI_DATASETTE_RESET },
    { IDM_DATASETTE_RESET_COUNTER, IDS_MI_DATASETTE_RESET_COUNTER },
    { IDM_CART_DETACH, IDS_MI_CART_DETACH },
    { IDM_MONITOR, IDS_MI_MONITOR },
#ifdef DEBUG
    { IDM_DEBUG_MODE_NORMAL, IDS_MI_DEBUG_MODE_NORMAL },
    { IDM_DEBUG_MODE_SMALL, IDS_MI_DEBUG_MODE_SMALL },
    { IDM_DEBUG_MODE_HISTORY, IDS_MI_DEBUG_MODE_HISTORY },
    { IDM_DEBUG_MODE_AUTOPLAY, IDS_MI_DEBUG_MODE_AUTOPLAY },
    { IDM_TOGGLE_MAINCPU_TRACE, IDS_MI_TOGGLE_MAINCPU_TRACE },
    { IDM_TOGGLE_DRIVE0CPU_TRACE, IDS_MI_TOGGLE_DRIVE0CPU_TRACE },
    { IDM_TOGGLE_DRIVE1CPU_TRACE, IDS_MI_TOGGLE_DRIVE1CPU_TRACE },
#endif
    { IDM_SNAPSHOT_LOAD, IDS_MI_SNAPSHOT_LOAD },
    { IDM_SNAPSHOT_SAVE, IDS_MI_SNAPSHOT_SAVE },
    { IDM_LOADQUICK, IDS_MI_LOADQUICK },
    { IDM_SAVEQUICK, IDS_MI_SAVEQUICK },
    { IDM_EVENT_TOGGLE_RECORD, IDS_MI_EVENT_TOGGLE_RECORD },
    { IDM_EVENT_TOGGLE_PLAYBACK, IDS_MI_EVENT_TOGGLE_PLAYBACK },
    { IDM_EVENT_SETMILESTONE, IDS_MI_EVENT_SETMILESTONE },
    { IDM_EVENT_RESETMILESTONE, IDS_MI_EVENT_RESETMILESTONE },
    { IDM_EVENT_START_MODE_SAVE, IDS_MI_EVENT_START_MODE_SAVE },
    { IDM_EVENT_START_MODE_LOAD, IDS_MI_EVENT_START_MODE_LOAD },
    { IDM_EVENT_START_MODE_RESET, IDS_MI_EVENT_START_MODE_RESET },
    { IDM_EVENT_START_MODE_PLAYBACK, IDS_MI_EVENT_START_MODE_PLAYBCK },
    { IDM_EVENT_DIRECTORY, IDS_MI_EVENT_DIRECTORY },
    { IDM_MEDIAFILE, IDS_MI_MEDIAFILE },
    { IDM_SOUND_RECORD_START, IDS_MI_SOUND_RECORD_START },
    { IDM_SOUND_RECORD_STOP, IDS_MI_SOUND_RECORD_STOP },
    { IDM_REFRESH_RATE_AUTO, IDS_MI_REFRESH_RATE_AUTO },
    { IDM_MAXIMUM_SPEED_NO_LIMIT, IDS_MI_MAXIMUM_SPEED_NO_LIMIT },
    { IDM_MAXIMUM_SPEED_CUSTOM, IDS_MI_MAXIMUM_SPEED_CUSTOM },
    { IDM_TOGGLE_WARP_MODE, IDS_MI_TOGGLE_WARP_MODE },
    { IDM_TOGGLE_DX9DISABLE, IDS_MI_TOGGLE_DX9DISABLE },
    { IDM_TOGGLE_ALWAYSONTOP, IDS_MI_TOGGLE_ALWAYSONTOP },
    { IDM_SWAP_JOYSTICK, IDS_MI_SWAP_JOYSTICK },
    { IDM_ALLOW_JOY_OPPOSITE_TOGGLE, IDS_MI_ALLOW_JOY_OPPOSITE },
    { IDM_JOYKEYS_TOGGLE, IDS_MI_JOYKEYS_TOGGLE },
    { IDM_TOGGLE_VIRTUAL_DEVICES, IDS_MI_TOGGLE_VIRTUAL_DEVICES },
    { IDM_AUTOSTART_SETTINGS, IDS_MI_AUTOSTART_SETTINGS },
    { IDM_VIDEO_SETTINGS, IDS_MI_VIDEO_SETTINGS },
    { IDM_DEVICEMANAGER, IDS_MI_DEVICEMANAGER },
    { IDM_JOY_SETTINGS, IDS_MI_JOY_SETTINGS },
    { IDM_EXTRA_JOY_SETTINGS, IDS_MI_SIDCART_JOY_SETTINGS },
    { IDM_KEYBOARD_SETTINGS, IDS_MI_KEYBOARD_SETTINGS },
    { IDM_SOUND_SETTINGS, IDS_MI_SOUND_SETTINGS },
    { IDM_ROM_SETTINGS, IDS_MI_ROM_SETTINGS },
    { IDM_RAM_SETTINGS, IDS_MI_RAM_SETTINGS },
    { IDM_DATASETTE_SETTINGS, IDS_MI_DATASETTE_SETTINGS },
    { IDM_RS232_SETTINGS, IDS_MI_RS232_SETTINGS },
    { IDM_ACIA_SETTINGS, IDS_MI_ACIA_SETTINGS },
    { IDM_V364SPEECH_SETTINGS, IDS_MI_V364SPEECH_SETTINGS },
    { IDM_SETTINGS_SAVE_FILE, IDS_MI_SETTINGS_SAVE_FILE },
    { IDM_SETTINGS_LOAD_FILE, IDS_MI_SETTINGS_LOAD_FILE },
    { IDM_SETTINGS_SAVE, IDS_MI_SETTINGS_SAVE },
    { IDM_SETTINGS_LOAD, IDS_MI_SETTINGS_LOAD },
    { IDM_SETTINGS_DEFAULT, IDS_MI_SETTINGS_DEFAULT },
    { IDM_TOGGLE_SAVE_SETTINGS_ON_EXIT, IDS_MI_SAVE_SETTINGS_ON_EXIT },
    { IDM_TOGGLE_CONFIRM_ON_EXIT, IDS_MI_CONFIRM_ON_EXIT },
    { IDM_LANG_EN, IDS_MI_LANG_EN },
    { IDM_LANG_DA, IDS_MI_LANG_DA },
    { IDM_LANG_DE, IDS_MI_LANG_DE },
    { IDM_LANG_FR, IDS_MI_LANG_FR },
    { IDM_LANG_HU, IDS_MI_LANG_HU },
    { IDM_LANG_KO, IDS_MI_LANG_KO },
    { IDM_LANG_NL, IDS_MI_LANG_NL },
    { IDM_LANG_PL, IDS_MI_LANG_PL },
    { IDM_LANG_RU, IDS_MI_LANG_RU },
    { IDM_LANG_SV, IDS_MI_LANG_SV },
    { IDM_LANG_TR, IDS_MI_LANG_TR },
    { IDM_CMDLINE, IDS_MI_CMDLINE },
    { IDM_CONTRIBUTORS, IDS_MI_CONTRIBUTORS },
    { IDM_LICENSE, IDS_MI_LICENSE },
    { IDM_WARRANTY, IDS_MI_WARRANTY },
    { IDM_TOGGLE_FULLSCREEN, IDS_MI_TOGGLE_FULLSCREEN },
    { IDM_SIDCART_SETTINGS, IDS_MI_SIDCART_SETTINGS },
    { IDM_PLUS4_SETTINGS, IDS_MI_PLUS4_SETTINGS },
    { 0, 0 }
};

ui_popup_translation_table_t plus4ui_popup_translation_table[] = {
    { 1, IDS_MP_FILE },
    { 2, IDS_MP_ATTACH_DISK_IMAGE },
    { 2, IDS_MP_DETACH_DISK_IMAGE },
    { 2, IDS_MP_FLIP_LIST },
    { 2, IDS_MP_DATASETTE_CONTROL },
    { 2, IDS_MP_ATTACH_CARTRIDGE_IMAGE },
    { 2, IDS_MP_RESET },
#ifdef DEBUG
    { 2, IDS_MP_DEBUG },
    { 3, IDS_MP_MODE },
#endif
    { 1, IDS_MP_EDIT },
    { 1, IDS_MP_SNAPSHOT },
    { 2, IDS_MP_RECORDING_START_MODE },
    { 1, IDS_MP_OPTIONS },
    { 2, IDS_MP_REFRESH_RATE },
    { 2, IDS_MP_MAXIMUM_SPEED },
    { 2, IDS_MP_VIDEO_STANDARD },
    { 1, IDS_MP_SETTINGS },
    { 2, IDS_MP_CARTRIDGE_IO_SETTINGS },
    { 1, IDS_MP_LANGUAGE },
    { 1, IDS_MP_HELP },
    { 0, 0 }
};

static uilib_localize_dialog_param plus4_main_trans[] = {
    { IDC_KERNAL, IDS_KERNAL, 0 },
    { IDC_PLUS4ROM_KERNAL_BROWSE, IDS_BROWSE, 0 },
    { IDC_BASIC, IDS_BASIC, 0 },
    { IDC_PLUS4ROM_BASIC_BROWSE, IDS_BROWSE, 0 },
    { IDC_3_PLUS_1_LO, IDS_3_PLUS_1_LO, 0 },
    { IDC_PLUS4ROM_3P1LO_BROWSE, IDS_BROWSE, 0 },
    { IDC_3_PLUS_1_HI, IDS_3_PLUS_1_HI, 0 },
    { IDC_PLUS4ROM_3P1HI_BROWSE, IDS_BROWSE, 0 },
    { 0, 0, 0 }
};

static uilib_localize_dialog_param plus4_drive_trans[] = {
    { IDC_DRIVEROM_1541_BROWSE, IDS_BROWSE, 0 },
    { IDC_DRIVEROM_1541II_BROWSE, IDS_BROWSE, 0 },
    { IDC_DRIVEROM_1551_BROWSE, IDS_BROWSE, 0 },
    { IDC_DRIVEROM_1570_BROWSE, IDS_BROWSE, 0 },
    { IDC_DRIVEROM_1571_BROWSE, IDS_BROWSE, 0 },
    { IDC_DRIVEROM_1581_BROWSE, IDS_BROWSE, 0 },
    { 0, 0, 0 }
};

static uilib_localize_dialog_param plus4_main_res_trans[] = {
    { 0, IDS_COMPUTER_RESOURCES_CAPTION, -1 },
    { IDC_COMPUTER_RESOURCES, IDS_COMPUTER_RESOURCES, 0 },
    { IDC_PLUS4ROM_KERNAL_RESOURCE, IDS_KERNAL, 0 },
    { IDC_PLUS4ROM_BASIC_RESOURCE, IDS_BASIC, 0 },
    { IDC_PLUS4ROM_3P1LO_RESOURCE, IDS_3_PLUS_1_LO, 0 },
    { IDC_PLUS4ROM_3P1HI_RESOURCE, IDS_3_PLUS_1_HI, 0 },
    { IDOK, IDS_OK, 0 },
    { IDCANCEL, IDS_CANCEL, 0 },
    { 0, 0, 0 }
};

static uilib_dialog_group plus4_main_left_group[] = {
    { IDC_KERNAL, 0 },
    { IDC_BASIC, 0 },
    { IDC_3_PLUS_1_LO, 0 },
    { IDC_3_PLUS_1_HI, 0 },
    { 0, 0 }
};

static uilib_dialog_group plus4_main_middle_group[] = {
    { IDC_PLUS4ROM_KERNAL_FILE, 0 },
    { IDC_PLUS4ROM_BASIC_FILE, 0 },
    { IDC_PLUS4ROM_3P1LO_FILE, 0 },
    { IDC_PLUS4ROM_3P1HI_FILE, 0 },
    { 0, 0 }
};

static uilib_dialog_group plus4_main_right_group[] = {
    { IDC_PLUS4ROM_KERNAL_BROWSE, 0 },
    { IDC_PLUS4ROM_BASIC_BROWSE, 0 },
    { IDC_PLUS4ROM_3P1LO_BROWSE, 0 },
    { IDC_PLUS4ROM_3P1HI_BROWSE, 0 },
    { 0, 0 }
};

static uilib_dialog_group plus4_drive_left_group[] = {
    { IDC_1541, 0 },
    { IDC_1541_II, 0 },
    { IDC_1551, 0 },
    { IDC_1570, 0 },
    { IDC_1571, 0 },
    { IDC_1581, 0 },
    { 0, 0 }
};

static uilib_dialog_group plus4_drive_middle_group[] = {
    { IDC_DRIVEROM_1541_FILE, 0 },
    { IDC_DRIVEROM_1541II_FILE, 0 },
    { IDC_DRIVEROM_1551_FILE, 0 },
    { IDC_DRIVEROM_1570_FILE, 0 },
    { IDC_DRIVEROM_1571_FILE, 0 },
    { IDC_DRIVEROM_1581_FILE, 0 },
    { 0, 0 }
};

static uilib_dialog_group plus4_drive_right_group[] = {
    { IDC_DRIVEROM_1541_BROWSE, 0 },
    { IDC_DRIVEROM_1541II_BROWSE, 0 },
    { IDC_DRIVEROM_1551_BROWSE, 0 },
    { IDC_DRIVEROM_1570_BROWSE, 0 },
    { IDC_DRIVEROM_1571_BROWSE, 0 },
    { IDC_DRIVEROM_1581_BROWSE, 0 },
    { 0, 0 }
};

static generic_trans_table_t plus4_generic_trans[] = {
    { IDC_1541, "1541" },
    { IDC_1541_II, "1541-II" },
    { IDC_1551, "1551" },
    { IDC_1570, "1570" },
    { IDC_1571, "1571" },
    { IDC_1581, "1581" },
    { 0, NULL }
};

static generic_trans_table_t plus4_generic_res_trans[] = {
    { IDC_DRIVEROM_1541_RESOURCE, "1541" },
    { IDC_DRIVEROM_1541II_RESOURCE, "1541-II" },
    { IDC_DRIVEROM_1551_RESOURCE, "1551" },
    { IDC_DRIVEROM_1570_RESOURCE, "1570" },
    { IDC_DRIVEROM_1571_RESOURCE, "1571" },
    { IDC_DRIVEROM_1581_RESOURCE, "1581" },
    { 0, NULL }
};

static void plus4_ui_specific(WPARAM wparam, HWND hwnd)
{
    uiplus4cart_proc(wparam, hwnd);

    switch (wparam) {
        case IDM_PLUS4_SETTINGS:
            ui_plus4_memory_dialog(hwnd);
            break;
        case IDM_V364SPEECH_SETTINGS:
            ui_v364speech_settings_dialog(hwnd);
            break;
        case IDM_JOY_SETTINGS:
            ui_joystick_settings_dialog(hwnd);
            break;
        case IDM_EXTRA_JOY_SETTINGS:
            ui_extra_joystick_settings_dialog(hwnd);
            break;
        case IDM_ROM_SETTINGS:
            uirom_settings_dialog(hwnd, IDD_PLUS4ROM_SETTINGS_DIALOG, IDD_PLUS4DRIVEROM_SETTINGS_DIALOG,
                                  romset_dialog_resources, uirom_settings,
                                  plus4_main_trans, plus4_drive_trans, plus4_generic_trans,
                                  plus4_main_left_group, plus4_main_middle_group, plus4_main_right_group,
                                  plus4_drive_left_group, plus4_drive_middle_group, plus4_drive_right_group,
                                  plus4_main_res_trans, plus4_generic_res_trans);
            break;
        case IDM_VIDEO_SETTINGS:
            ui_video_settings_dialog(hwnd, UI_VIDEO_CHIP_TED, UI_VIDEO_CHIP_NONE);
            break;
        case IDM_DRIVE_SETTINGS:
            uidriveplus4_settings_dialog(hwnd);
            break;
        case IDM_ACIA_SETTINGS:
            ui_acia_settings_dialog(hwnd, 0, NULL, 0, 0);
            break;
        case IDM_SIDCART_SETTINGS:
            ui_sidcart_settings_dialog(hwnd);
            break;
        case IDM_KEYBOARD_SETTINGS:
            uikeyboard_settings_dialog(hwnd, &uikeyboard_config);
            break;
    }
}

int plus4ui_init(void)
{
    uiplus4cart_init();

    ui_register_machine_specific(plus4_ui_specific);
    ui_register_menu_toggles(plus4_ui_menu_toggles);
    ui_register_res_values(plus4_ui_res_values);
    ui_register_translation_tables(plus4ui_menu_translation_table, plus4ui_popup_translation_table);
    return 0;
}

void plus4ui_shutdown(void)
{
}
