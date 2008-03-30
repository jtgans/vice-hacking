/*
 * ui.h - user interface for BeOS
 *
 * Written by
 *  Andreas Matthies <andreas.matthies@gmx.net>
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

#ifndef _UI_BEOS_H
#define _UI_BEOS_H

#include "types.h"
#include "uiapi.h"

/* Here some stuff for the connection of menuitems and resources */
typedef struct {
    /* Name of resource.  */
    const char *name;
    /* ID of the corresponding menu item.  */
    int item_id;
} ui_menu_toggle;

typedef struct {
    int value;
    int item_id; /* The last item_id has to be zero.  */
} ui_res_possible_values;

typedef struct {
    const char *name;
    ui_res_possible_values *vals;
} ui_res_value_list;


/* ------------------------------------------------------------------------- */
/* These are the commands that cannot be handled within a keyboard
   interrupt.  They are dispatched via 'ui_dispatch_events' and queued
   via 'ui_add_event'.  */

typedef	enum {
		UICMD_EXIT,
		UICMD_AUTOSTART,
		UICMD_ATTACH,
		UICMD_DETACH,
        UICMD_RESET,
        UICMD_HARD_RESET,
        UICMD_LOAD_SETTINGS,
        UICMD_SAVE_SETTINGS,
        UICMD_FREEZE,
        UICMD_FLIP_NEXT,
        UICMD_FLIP_PREVIOUS,
        UICMD_FLIP_ADD,
        UICMD_FLIP_REMOVE,
        UICMD_TOGGLE_RESOURCE
} uicmd_t;

typedef struct {
	uicmd_t nr;
    char *param;
} ui_command_type_t;

typedef void (*ui_machine_specific_t) (void* msg);

/*-------------------------------------------------------------------------*/

extern void ui_register_machine_specific(ui_machine_specific_t func);
extern void ui_register_menu_toggles(ui_menu_toggle *toggles);
extern void ui_register_res_values(ui_res_value_list *valuelist);
extern void ui_main(char hotkey);
extern void ui_set_warp_status(int status);
extern void ui_dispatch_events(void);
extern void ui_add_event(void *msg);
extern void ui_display_speed(float percent, float framerate, int warp_flag);
extern void ui_message(const char *format,...);
extern void ui_error(const char *format,...);
extern void ui_error_string(const char *text);
extern void ui_show_text(const char *caption, const char *header,const char *text);
extern void ui_cmdline_show_options(void);
extern void ui_update_menus(void);
extern void mon(ADDRESS a);

#endif
