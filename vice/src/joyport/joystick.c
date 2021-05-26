/*
 * joystick.c - Common joystick emulation.
 *
 * Written by
 *  Andreas Boose <viceteam@t-online.de>
 *  Marco van den Heuvel <blackystardust68@yahoo.com>
 *
 * Based on old code by
 *  Ettore Perazzoli <ettore@comm2000.it>
 *  Jouko Valta <jopi@stekt.oulu.fi>
 *  Andre Fachat <fachat@physik.tu-chemnitz.de>
 *  Bernhard Kuhn <kuhn@eikon.e-technik.tu-muenchen.de>
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

#include <stdlib.h>
#include <string.h>

#include "archdep.h"
#include "alarm.h"
#include "cmdline.h"
#include "keyboard.h"
#include "joy.h"
#include "joyport.h"
#include "joystick.h"
#include "kbd.h"
#include "lib.h"
#include "machine.h"
#include "maincpu.h"
#include "network.h"
#include "resources.h"
#include "snapshot.h"
#include "types.h"
#include "uiapi.h"
#include "userport_joystick.h"
#include "vice-event.h"

/* Control port <--> Joystick connections:

   cport | joystick | I/O
   ----------------------
     1   | up       |  I
     2   | down     |  I
     3   | left     |  I
     4   | right    |  I
     6   | button   |  I
         |          |
     9   | button 2 |  I
     5   | button 3 |  I
 */

/* #define DEBUGJOY */

#ifdef DEBUGJOY
#define DBG(x)  printf x
#else
#define DBG(x)
#endif

#define JOYPAD_FIRE2 0x20
#define JOYPAD_FIRE3 0x40
#define JOYPAD_FIRE4 0x80
#define JOYPAD_FIRE5 0x100
#define JOYPAD_FIRE6 0x200
#define JOYPAD_FIRE7 0x400
#define JOYPAD_FIRE8 0x800
#define JOYPAD_FIRE 0x10
#define JOYPAD_E    0x08
#define JOYPAD_W    0x04
#define JOYPAD_S    0x02
#define JOYPAD_N    0x01
#define JOYPAD_SW   (JOYPAD_S | JOYPAD_W)
#define JOYPAD_SE   (JOYPAD_S | JOYPAD_E)
#define JOYPAD_NW   (JOYPAD_N | JOYPAD_W)
#define JOYPAD_NE   (JOYPAD_N | JOYPAD_E)

static int joyport_joystick[5] = { 0, 0, 0, 0, 0 };

/* Global joystick value.  */
/*! \todo SRT: document: what are these values joystick_value[0, 1, 2, ..., 5] used for? */
static uint16_t joystick_value[JOYPORT_MAX_PORTS] = { 0 };

typedef struct joystick_values_s {
    unsigned int last_used_joyport;
    uint16_t values[JOYPORT_MAX_PORTS];
} joystick_values_t;

static joystick_values_t network_joystick_value = { .last_used_joyport = JOYPORT_MAX_PORTS };

/* Latched joystick status.  */
static joystick_values_t latch_joystick_value = { .last_used_joyport = JOYPORT_MAX_PORTS };

/* mapping of the joystick ports */
int joystick_port_map[JOYPORT_MAX_PORTS] = { 0 };

/* to prevent illegal direction combinations */
static int joystick_opposite_enable = 0;
static const uint16_t joystick_opposite_direction[] = {
                                               /* E W S N */
    0,                                         /*         */
    JOYPAD_S,                                  /*       + */
    JOYPAD_N,                                  /*     +   */
    JOYPAD_S | JOYPAD_N,                       /*     + + */
    JOYPAD_E,                                  /*   +     */
    JOYPAD_E | JOYPAD_S,                       /*   +   + */
    JOYPAD_E | JOYPAD_N,                       /*   + +   */
    JOYPAD_E | JOYPAD_S | JOYPAD_N,            /*   + + + */
    JOYPAD_W,                                  /* +       */
    JOYPAD_W | JOYPAD_S,                       /* +     + */
    JOYPAD_W | JOYPAD_N,                       /* +   +   */
    JOYPAD_W | JOYPAD_S | JOYPAD_N,            /* +   + + */
    JOYPAD_E | JOYPAD_W,                       /* + +     */
    JOYPAD_E | JOYPAD_W | JOYPAD_S,            /* + +   + */
    JOYPAD_E | JOYPAD_W | JOYPAD_N,            /* + + +   */
    JOYPAD_E | JOYPAD_W | JOYPAD_S | JOYPAD_N  /* + + + + */
};

const uint8_t hat_map[9] = {
    0,
    JOYSTICK_DIRECTION_UP,
    JOYSTICK_DIRECTION_UP|JOYSTICK_DIRECTION_RIGHT,
    JOYSTICK_DIRECTION_RIGHT,
    JOYSTICK_DIRECTION_RIGHT|JOYSTICK_DIRECTION_DOWN,
    JOYSTICK_DIRECTION_DOWN,
    JOYSTICK_DIRECTION_DOWN|JOYSTICK_DIRECTION_LEFT,
    JOYSTICK_DIRECTION_LEFT,
    JOYSTICK_DIRECTION_LEFT|JOYSTICK_DIRECTION_UP
};

/* Callback to machine specific joystick routines, needed for lightpen triggering */
static joystick_machine_func_t joystick_machine_func = NULL;

static alarm_t *joystick_alarm = NULL;

static CLOCK joystick_delay;

#ifdef COMMON_JOYKEYS
int joykeys[JOYSTICK_KEYSET_NUM][JOYSTICK_KEYSET_NUM_KEYS];
#endif

/*! \todo SRT: offset is unused! */

static void joystick_latch_matrix(CLOCK offset)
{
    uint8_t idx;

    if (network_connected()) {
        idx = network_joystick_value.last_used_joyport;
        if (idx < JOYPORT_MAX_PORTS) {
            joystick_value[idx] = network_joystick_value.values[idx];
        } else {
            memcpy(joystick_value, network_joystick_value.values, sizeof(joystick_value));
        }
    } else {
        memcpy(joystick_value, latch_joystick_value.values, sizeof(joystick_value));
    }

    if (joystick_machine_func != NULL) {
        joystick_machine_func();
    }

    if (joyport_joystick[0]) {
        joyport_display_joyport(JOYPORT_ID_JOY1, joystick_value[JOYPORT_1]);
    }
    if (joyport_joystick[1]) {
        joyport_display_joyport(JOYPORT_ID_JOY2, joystick_value[JOYPORT_2]);
    }
    if (joyport_joystick[2]) {
        joyport_display_joyport(JOYPORT_ID_JOY3, joystick_value[JOYPORT_3]);
    }
    if (joyport_joystick[3]) {
        joyport_display_joyport(JOYPORT_ID_JOY4, joystick_value[JOYPORT_4]);
    }
    if (joyport_joystick[4]) {
        joyport_display_joyport(JOYPORT_ID_JOY5, joystick_value[JOYPORT_5]);
    }
}

/*-----------------------------------------------------------------------*/

static void joystick_event_record(void)
{
    event_record(EVENT_JOYSTICK_VALUE, (void *)joystick_value, sizeof(joystick_value));
}

void joystick_event_playback(CLOCK offset, void *data)
{
    memcpy(latch_joystick_value.values, data, sizeof(latch_joystick_value.values));

    joystick_latch_matrix(offset);
}

static void joystick_latch_handler(CLOCK offset, void *data)
{
    alarm_unset(joystick_alarm);
    alarm_context_update_next_pending(joystick_alarm->context);

    joystick_latch_matrix(offset);

    joystick_event_record();
}

void joystick_event_delayed_playback(void *data)
{
    /*! \todo SRT: why network_joystick_value?
     * and why sizeof latch_joystick_value,
     * if the target is network_joystick_value?
     */
    memcpy(&network_joystick_value, data, sizeof(latch_joystick_value));
    alarm_set(joystick_alarm, maincpu_clk + joystick_delay);
}

void joystick_register_machine(joystick_machine_func_t func)
{
    joystick_machine_func = func;
}

void joystick_register_delay(unsigned int delay)
{
    joystick_delay = delay;
}
/*-----------------------------------------------------------------------*/
static void joystick_process_latch(void)
{
    CLOCK delay = lib_unsigned_rand(1, (unsigned int)machine_get_cycles_per_frame());

    if (network_connected()) {
        network_event_record(EVENT_JOYSTICK_DELAY, (void *)&delay, sizeof(delay));
        network_event_record(EVENT_JOYSTICK_VALUE, (void *)&latch_joystick_value, sizeof(latch_joystick_value));
    } else {
        alarm_set(joystick_alarm, maincpu_clk + delay);
    }
}

void joystick_set_value_absolute(unsigned int joyport, uint16_t value)
{
    if (event_playback_active()) {
        return;
    }

    if (latch_joystick_value.values[joyport] != value) {
        latch_joystick_value.values[joyport] = value;
        latch_joystick_value.last_used_joyport = joyport;
        joystick_process_latch();
    }
}

/* set joystick bits */
void joystick_set_value_or(unsigned int joyport, uint16_t value)
{
    if (event_playback_active()) {
        return;
    }

    latch_joystick_value.values[joyport] |= value;

    if (!joystick_opposite_enable) {
        latch_joystick_value.values[joyport] &= (uint16_t)(~joystick_opposite_direction[value & 0xf]);
    }

    latch_joystick_value.last_used_joyport = joyport;
    joystick_process_latch();
}

/* release joystick bits */
void joystick_set_value_and(unsigned int joyport, uint16_t value)
{
    if (event_playback_active()) {
        return;
    }

    latch_joystick_value.values[joyport] &= value;
    latch_joystick_value.last_used_joyport = joyport;
    joystick_process_latch();
}

void joystick_clear(unsigned int joyport)
{
    latch_joystick_value.values[joyport] = 0;
    latch_joystick_value.last_used_joyport = joyport;
    joystick_latch_matrix(0);
}

void joystick_clear_all(void)
{
    memset(latch_joystick_value.values, 0, sizeof latch_joystick_value.values);
    latch_joystick_value.last_used_joyport = JOYPORT_MAX_PORTS;
    joystick_latch_matrix(0);
}

uint16_t get_joystick_value(int index)
{
    return joystick_value[index];
}

/*--------------------------------------------------------------------------*/

#ifdef COMMON_JOYKEYS

/* the order of values in joypad_bits is the same as in joystick_direction_t */
static int joypad_bits[JOYSTICK_KEYSET_NUM_KEYS] = {
    JOYPAD_FIRE,
    JOYPAD_SW,
    JOYPAD_S,
    JOYPAD_SE,
    JOYPAD_W,
    JOYPAD_E,
    JOYPAD_NW,
    JOYPAD_N,
    JOYPAD_NE,
    JOYPAD_FIRE2,
    JOYPAD_FIRE3,
    JOYPAD_FIRE4,
    JOYPAD_FIRE5,
    JOYPAD_FIRE6,
    JOYPAD_FIRE7,
    JOYPAD_FIRE8
};

static int joypad_status[JOYSTICK_KEYSET_NUM][JOYSTICK_KEYSET_NUM_KEYS];
static int joypad_vmask[JOYSTICK_KEYSET_NUM];
static int joypad_hmask[JOYSTICK_KEYSET_NUM];

/* convert the given keyset status array into the corrosponding bits for the
 * joystick
 */
static int getjoyvalue(int *status)
{
    int val = 0;
    int column;

    for (column = 0; column < JOYSTICK_KEYSET_NUM_KEYS; column++) {
        if (status[column]) {
            val |= joypad_bits[column];
        }
    }
    return val;
}

/* toggle keyset joystick.
   this disables any active key-based joystick and is useful for typing. */
static int joykeys_enable = 0;

static int set_joykeys_enable(int val, void *param)
{
    joykeys_enable = val ? 1 : 0;

    return 0;
}

#define DEFINE_SET_KEYSET(num)                       \
    static int set_keyset##num(int val, void *param) \
    {                                                \
        joykeys[num][vice_ptr_to_int(param)] = val;  \
                                                     \
        return 0;                                    \
    }

DEFINE_SET_KEYSET(1)
DEFINE_SET_KEYSET(2)

static const resource_int_t joykeys_resources_int[] = {
    { "KeySet1NorthWest", ARCHDEP_KEYBOARD_SYM_NONE, RES_EVENT_NO, NULL,
      &joykeys[JOYSTICK_KEYSET_IDX_A][JOYSTICK_KEYSET_NW], set_keyset1, (void *)JOYSTICK_KEYSET_NW },
    { "KeySet1North", ARCHDEP_KEYBOARD_SYM_NONE, RES_EVENT_NO, NULL,
      &joykeys[JOYSTICK_KEYSET_IDX_A][JOYSTICK_KEYSET_N], set_keyset1, (void *)JOYSTICK_KEYSET_N },
    { "KeySet1NorthEast", ARCHDEP_KEYBOARD_SYM_NONE, RES_EVENT_NO, NULL,
      &joykeys[JOYSTICK_KEYSET_IDX_A][JOYSTICK_KEYSET_NE], set_keyset1, (void *)JOYSTICK_KEYSET_NE },
    { "KeySet1East", ARCHDEP_KEYBOARD_SYM_NONE, RES_EVENT_NO, NULL,
      &joykeys[JOYSTICK_KEYSET_IDX_A][JOYSTICK_KEYSET_E], set_keyset1, (void *)JOYSTICK_KEYSET_E },
    { "KeySet1SouthEast", ARCHDEP_KEYBOARD_SYM_NONE, RES_EVENT_NO, NULL,
      &joykeys[JOYSTICK_KEYSET_IDX_A][JOYSTICK_KEYSET_SE], set_keyset1, (void *)JOYSTICK_KEYSET_SE },
    { "KeySet1South", ARCHDEP_KEYBOARD_SYM_NONE, RES_EVENT_NO, NULL,
      &joykeys[JOYSTICK_KEYSET_IDX_A][JOYSTICK_KEYSET_S], set_keyset1, (void *)JOYSTICK_KEYSET_S },
    { "KeySet1SouthWest", ARCHDEP_KEYBOARD_SYM_NONE, RES_EVENT_NO, NULL,
      &joykeys[JOYSTICK_KEYSET_IDX_A][JOYSTICK_KEYSET_SW], set_keyset1, (void *)JOYSTICK_KEYSET_SW },
    { "KeySet1West", ARCHDEP_KEYBOARD_SYM_NONE, RES_EVENT_NO, NULL,
      &joykeys[JOYSTICK_KEYSET_IDX_A][JOYSTICK_KEYSET_W], set_keyset1, (void *)JOYSTICK_KEYSET_W },
    { "KeySet1Fire", ARCHDEP_KEYBOARD_SYM_NONE, RES_EVENT_NO, NULL,
      &joykeys[JOYSTICK_KEYSET_IDX_A][JOYSTICK_KEYSET_FIRE], set_keyset1, (void *)JOYSTICK_KEYSET_FIRE },
    { "KeySet1Fire2", ARCHDEP_KEYBOARD_SYM_NONE, RES_EVENT_NO, NULL,
        &joykeys[JOYSTICK_KEYSET_IDX_A][JOYSTICK_KEYSET_FIRE2], set_keyset1, (void *)JOYSTICK_KEYSET_FIRE2 },
    { "KeySet1Fire3", ARCHDEP_KEYBOARD_SYM_NONE, RES_EVENT_NO, NULL,
        &joykeys[JOYSTICK_KEYSET_IDX_A][JOYSTICK_KEYSET_FIRE3], set_keyset1, (void *)JOYSTICK_KEYSET_FIRE3 },
    { "KeySet1Fire4", ARCHDEP_KEYBOARD_SYM_NONE, RES_EVENT_NO, NULL,
        &joykeys[JOYSTICK_KEYSET_IDX_A][JOYSTICK_KEYSET_FIRE4], set_keyset1, (void *)JOYSTICK_KEYSET_FIRE4 },
    { "KeySet1Fire5", ARCHDEP_KEYBOARD_SYM_NONE, RES_EVENT_NO, NULL,
        &joykeys[JOYSTICK_KEYSET_IDX_A][JOYSTICK_KEYSET_FIRE5], set_keyset1, (void *)JOYSTICK_KEYSET_FIRE5 },
    { "KeySet1Fire6", ARCHDEP_KEYBOARD_SYM_NONE, RES_EVENT_NO, NULL,
        &joykeys[JOYSTICK_KEYSET_IDX_A][JOYSTICK_KEYSET_FIRE6], set_keyset1, (void *)JOYSTICK_KEYSET_FIRE6 },
    { "KeySet1Fire7", ARCHDEP_KEYBOARD_SYM_NONE, RES_EVENT_NO, NULL,
        &joykeys[JOYSTICK_KEYSET_IDX_A][JOYSTICK_KEYSET_FIRE7], set_keyset1, (void *)JOYSTICK_KEYSET_FIRE7 },
    { "KeySet1Fire8", ARCHDEP_KEYBOARD_SYM_NONE, RES_EVENT_NO, NULL,
        &joykeys[JOYSTICK_KEYSET_IDX_A][JOYSTICK_KEYSET_FIRE8], set_keyset1, (void *)JOYSTICK_KEYSET_FIRE8 },
    { "KeySet2NorthWest", ARCHDEP_KEYBOARD_SYM_NONE, RES_EVENT_NO, NULL,
      &joykeys[JOYSTICK_KEYSET_IDX_B][JOYSTICK_KEYSET_NW], set_keyset2, (void *)JOYSTICK_KEYSET_NW },
    { "KeySet2North", ARCHDEP_KEYBOARD_SYM_NONE, RES_EVENT_NO, NULL,
      &joykeys[JOYSTICK_KEYSET_IDX_B][JOYSTICK_KEYSET_N], set_keyset2, (void *)JOYSTICK_KEYSET_N },
    { "KeySet2NorthEast", ARCHDEP_KEYBOARD_SYM_NONE, RES_EVENT_NO, NULL,
      &joykeys[JOYSTICK_KEYSET_IDX_B][JOYSTICK_KEYSET_NE], set_keyset2, (void *)JOYSTICK_KEYSET_NE },
    { "KeySet2East", ARCHDEP_KEYBOARD_SYM_NONE, RES_EVENT_NO, NULL,
      &joykeys[JOYSTICK_KEYSET_IDX_B][JOYSTICK_KEYSET_E], set_keyset2, (void *)JOYSTICK_KEYSET_E },
    { "KeySet2SouthEast", ARCHDEP_KEYBOARD_SYM_NONE, RES_EVENT_NO, NULL,
      &joykeys[JOYSTICK_KEYSET_IDX_B][JOYSTICK_KEYSET_SE], set_keyset2, (void *)JOYSTICK_KEYSET_SE },
    { "KeySet2South", ARCHDEP_KEYBOARD_SYM_NONE, RES_EVENT_NO, NULL,
      &joykeys[JOYSTICK_KEYSET_IDX_B][JOYSTICK_KEYSET_S], set_keyset2, (void *)JOYSTICK_KEYSET_S },
    { "KeySet2SouthWest", ARCHDEP_KEYBOARD_SYM_NONE, RES_EVENT_NO, NULL,
      &joykeys[JOYSTICK_KEYSET_IDX_B][JOYSTICK_KEYSET_SW], set_keyset2, (void *)JOYSTICK_KEYSET_SW },
    { "KeySet2West", ARCHDEP_KEYBOARD_SYM_NONE, RES_EVENT_NO, NULL,
      &joykeys[JOYSTICK_KEYSET_IDX_B][JOYSTICK_KEYSET_W], set_keyset2, (void *)JOYSTICK_KEYSET_W },
    { "KeySet2Fire", ARCHDEP_KEYBOARD_SYM_NONE, RES_EVENT_NO, NULL,
      &joykeys[JOYSTICK_KEYSET_IDX_B][JOYSTICK_KEYSET_FIRE], set_keyset2, (void *)JOYSTICK_KEYSET_FIRE },
    { "KeySet2Fire2", ARCHDEP_KEYBOARD_SYM_NONE, RES_EVENT_NO, NULL,
        &joykeys[JOYSTICK_KEYSET_IDX_B][JOYSTICK_KEYSET_FIRE2], set_keyset2, (void *)JOYSTICK_KEYSET_FIRE2 },
    { "KeySet2Fire3", ARCHDEP_KEYBOARD_SYM_NONE, RES_EVENT_NO, NULL,
        &joykeys[JOYSTICK_KEYSET_IDX_B][JOYSTICK_KEYSET_FIRE3], set_keyset2, (void *)JOYSTICK_KEYSET_FIRE3 },
    { "KeySet2Fire4", ARCHDEP_KEYBOARD_SYM_NONE, RES_EVENT_NO, NULL,
        &joykeys[JOYSTICK_KEYSET_IDX_B][JOYSTICK_KEYSET_FIRE4], set_keyset2, (void *)JOYSTICK_KEYSET_FIRE4 },
    { "KeySet2Fire5", ARCHDEP_KEYBOARD_SYM_NONE, RES_EVENT_NO, NULL,
        &joykeys[JOYSTICK_KEYSET_IDX_B][JOYSTICK_KEYSET_FIRE5], set_keyset2, (void *)JOYSTICK_KEYSET_FIRE5 },
    { "KeySet2Fire6", ARCHDEP_KEYBOARD_SYM_NONE, RES_EVENT_NO, NULL,
        &joykeys[JOYSTICK_KEYSET_IDX_B][JOYSTICK_KEYSET_FIRE6], set_keyset2, (void *)JOYSTICK_KEYSET_FIRE6 },
    { "KeySet2Fire7", ARCHDEP_KEYBOARD_SYM_NONE, RES_EVENT_NO, NULL,
        &joykeys[JOYSTICK_KEYSET_IDX_B][JOYSTICK_KEYSET_FIRE7], set_keyset2, (void *)JOYSTICK_KEYSET_FIRE7 },
    { "KeySet2Fire8", ARCHDEP_KEYBOARD_SYM_NONE, RES_EVENT_NO, NULL,
        &joykeys[JOYSTICK_KEYSET_IDX_B][JOYSTICK_KEYSET_FIRE8], set_keyset2, (void *)JOYSTICK_KEYSET_FIRE8 },
    { "KeySetEnable", 1, RES_EVENT_NO, NULL,
      &joykeys_enable, set_joykeys_enable, NULL },
    RESOURCE_INT_LIST_END
};

#ifdef DEBUGJOY
static void DBGSTATUS(int keysetnum, int value, int joyport, int key, int flg)
{
    int column;
    char *flags[3] = { "set", "unset", "ignored" };

    DBG((" key:%02x |", key));
    for (column = 0; column < JOYSTICK_KEYSET_NUM_KEYS; column++) {
        DBG((joypad_status[keysetnum][column] ? "*" : "."));
    }
    DBG(("|"));
    for (column = 5; column >= 0; column--) {
        DBG((((value >> column) & 1) ? "*" : "."));
    }
    DBG(("|"));
    for (column = 5; column >= 0; column--) {
        DBG((((joypad_vmask[keysetnum] >> column) & 1) ? "*" : "."));
    }
    DBG(("|"));
    for (column = 5; column >= 0; column--) {
        DBG((((joypad_hmask[keysetnum] >> column) & 1) ? "*" : "."));
    }
    DBG(("|"));
    for (column = 5; column >= 0; column--) {
        DBG((((latch_joystick_value[joyport] >> column) & 1) ? "*" : "."));
    }
    DBG((" (%s)\n", flags[flg]));
}
#else
#define DBGSTATUS(a, b, c, d, e)
#endif


/* called on key-down event */
int joystick_check_set(signed long key, int keysetnum, unsigned int joyport)
{
    int column, value;

    /* if joykeys are disabled then ignore key sets */
    if (!joykeys_enable) {
        return 0;
    }

    for (column = 0; column < JOYSTICK_KEYSET_NUM_KEYS; column++) {
        if (key == joykeys[keysetnum][column]) {
            DBG(("joystick_check_set:"));

            joypad_status[keysetnum][column] = 1;
            value = getjoyvalue(joypad_status[keysetnum]);

            if (!joystick_opposite_enable) {
                /* setup the mask for the opposite side of the pressed key */
                if ((column == JOYSTICK_KEYSET_N) || (column == JOYSTICK_KEYSET_NW) || (column == JOYSTICK_KEYSET_NE)) {
                    joypad_vmask[keysetnum] = ~JOYPAD_S;
                } else if ((column == JOYSTICK_KEYSET_S) || (column == JOYSTICK_KEYSET_SW) || (column == JOYSTICK_KEYSET_SE)) {
                    joypad_vmask[keysetnum] = ~JOYPAD_N;
                }
                if ((column == JOYSTICK_KEYSET_W) || (column == JOYSTICK_KEYSET_SW) || (column == JOYSTICK_KEYSET_NW)) {
                    joypad_hmask[keysetnum] = ~JOYPAD_E;
                } else if ((column == JOYSTICK_KEYSET_E) || (column == JOYSTICK_KEYSET_SE) || (column == JOYSTICK_KEYSET_NE)) {
                    joypad_hmask[keysetnum] = ~JOYPAD_W;
                }
                /* if two opposite directions are set, mask out the opposite side of
                 * the last pressed key */
                if ((value & joypad_bits[JOYSTICK_KEYSET_N]) && (value & joypad_bits[JOYSTICK_KEYSET_S])) {
                    value &= joypad_vmask[keysetnum];
                }
                if ((value & joypad_bits[JOYSTICK_KEYSET_E]) && (value & joypad_bits[JOYSTICK_KEYSET_W])) {
                    value &= joypad_hmask[keysetnum];
                }
            }

            joystick_set_value_absolute(joyport, (uint16_t)value);

            DBGSTATUS(keysetnum, value, joyport, key, 0);
            return 1;
        }
    }
    return 0;
}

/* called on key-up event */
int joystick_check_clr(signed long key, int keysetnum, unsigned int joyport)
{
    int column, value;

    /* if joykeys are disabled then ignore key sets */
    if (!joykeys_enable) {
        return 0;
    }

    for (column = 0; column < JOYSTICK_KEYSET_NUM_KEYS; column++) {
        if (key == joykeys[keysetnum][column]) {
            joypad_status[keysetnum][column] = 0;
            value = getjoyvalue(joypad_status[keysetnum]);

            if (!joystick_opposite_enable) {
                /* if two opposite directions are set, mask out the opposite side of
                 * the last pressed key */
                if ((value & joypad_bits[JOYSTICK_KEYSET_N]) && (value & joypad_bits[JOYSTICK_KEYSET_S])) {
                    value &= joypad_vmask[keysetnum];
                }
                if ((value & joypad_bits[JOYSTICK_KEYSET_E]) && (value & joypad_bits[JOYSTICK_KEYSET_W])) {
                    value &= joypad_hmask[keysetnum];
                }
            }

            joystick_set_value_absolute(joyport, (uint16_t)value);

            DBG(("joystick_check_clr:"));
            DBGSTATUS(keysetnum, value, joyport, key, 1);
            return 1;
        }
    }
    return 0;
}

void joystick_joypad_clear(void)
{
    memset(joypad_status, 0, sizeof(joypad_status));
}
#endif /* COMMON_JOYKEYS */

/*-----------------------------------------------------------------------*/

static int joyport_enable_joystick(int port, int val)
{
    joyport_joystick[port] = (val) ? 1 : 0;
    return 0;
}

static uint8_t read_joystick(int port)
{
    return (uint8_t)(~(joystick_value[port] & 0x1f));
}

static uint8_t read_potx(int port)
{
    /* printf("read_potx %d %02x %02x %02x\n", port, joystick_value[port + 1]); */
    return joystick_value[port] & JOYPAD_FIRE2 ? 0x00 : 0xff;
}

static uint8_t read_poty(int port)
{
    /* printf("read_poty %d %02x %02x %02x\n", port, joystick_value[port + 1]); */
    return joystick_value[port] & JOYPAD_FIRE3 ? 0x00 : 0xff;
}


/* Some prototypes are needed */
static int joystick_snapshot_write_module(snapshot_t *s, int port);
static int joystick_snapshot_read_module(snapshot_t *s, int port);

static joyport_t joystick_device = {
    "Joystick",                     /* name of the device */
    JOYPORT_RES_ID_NONE,            /* device doesn't have a class, multiple deviced of this kind can be active at the same time */
    JOYPORT_IS_NOT_LIGHTPEN,        /* device is NOT a lightpen */
    JOYPORT_POT_OPTIONAL,           /* device does NOT use the potentiometer lines */
    joyport_enable_joystick,        /* device enable function */
    read_joystick,                  /* digital line read function */
    NULL,                           /* NO digital line store function */
    read_potx,                      /* pot-x read function */
    read_poty,                      /* pot-y read function */
    joystick_snapshot_write_module, /* device write snapshot function */
    joystick_snapshot_read_module   /* device read snapshot function */
};

static int joystick_joyport_register(void)
{
    return joyport_device_register(JOYPORT_ID_JOYSTICK, &joystick_device);
}

/*--------------------------------------------------------------------------*/

static int set_joystick_opposite_enable(int val, void *param)
{
    joystick_opposite_enable = val ? 1 : 0;

    return 0;
}

#if (defined LINUX_JOYSTICK || defined HAS_USB_JOYSTICK || defined MAC_JOYSTICK)
/* Actions to perform on joystick input */
typedef enum {
    NONE = 0,

    /* Joystick movement or button press */
    JOYSTICK = 1,

    /* Keyboard key press */
    KEYBOARD = 2,
} joystick_action_t;


/* Input mapping for each direction/button/etc */
typedef struct joystick_mapping_s {
    /* Action to perform */
    joystick_action_t action;

    union {
        uint16_t joy_pin;

        /* key[0] = row, key[1] = column */
        int key[2];
    } value;
} joystick_mapping_t;

typedef struct joystick_axis_mapping_s {
    /* Previous state of input */
    uint8_t prev;
    struct joystick_mapping_s positive_direction;
    struct joystick_mapping_s negative_direction;
} joystick_axis_mapping_t;

typedef struct joystick_hat_mapping_s {
    /* Previous state of input */
    uint8_t prev;
    struct joystick_mapping_s up;
    struct joystick_mapping_s down;
    struct joystick_mapping_s left;
    struct joystick_mapping_s right;
} joystick_hat_mapping_t;

static int num_joystick_devices = 0;


/** \brief  Joystick device name length (including 0)
 */
#define JOYDEV_NAME_SIZE    0x80

/* device structure */
typedef struct joystick_device_s {
    struct joystick_driver_s *driver;
    char jname[JOYDEV_NAME_SIZE];
    int joyport;
    void *priv;
    joystick_axis_mapping_t *axis_mapping;
    joystick_mapping_t *button_mapping;
    joystick_hat_mapping_t *hat_mapping;
} joystick_device_t;

static struct joystick_device_s *joystick_devices = NULL;
#endif

static int set_joystick_device(int val, void *param)
{
    int port_idx = vice_ptr_to_int(param);

#if (defined LINUX_JOYSTICK || defined HAS_USB_JOYSTICK || defined MAC_JOYSTICK)
    if (joystick_port_map[port_idx] >= JOYDEV_REALJOYSTICK_MIN) {
        int olddev = joystick_port_map[port_idx] - JOYDEV_REALJOYSTICK_MIN;
        if (olddev < num_joystick_devices) {
            joystick_devices[olddev].joyport = -1;
        }
    }
#else
    if (joy_arch_set_device(port_idx, val) < 0) {
        return -1;
    }
#endif

    joystick_port_map[port_idx] = val;

#if (defined LINUX_JOYSTICK || defined HAS_USB_JOYSTICK || defined MAC_JOYSTICK)
    if (joystick_port_map[port_idx] >= JOYDEV_REALJOYSTICK_MIN) {
        int newdev = joystick_port_map[port_idx] - JOYDEV_REALJOYSTICK_MIN;
        if (newdev < num_joystick_devices) {
            joystick_devices[newdev].joyport = port_idx;
            int i;
            for (i = 0; i < JOYPORT_MAX_PORTS; i++) {
                if (i != port_idx && joystick_port_map[port_idx] == joystick_port_map[i]) {
                    joystick_port_map[i] = JOYDEV_NONE;
                }
            }
        }
    }
#endif

    return 0;
}

static const resource_int_t joyopposite_resources_int[] = {
    { "JoyOpposite", 0, RES_EVENT_NO, NULL,
      &joystick_opposite_enable, set_joystick_opposite_enable, NULL },
    RESOURCE_INT_LIST_END
};

static resource_int_t joy1_resources_int[] = {
    { "JoyDevice1", JOYDEV_NONE, RES_EVENT_NO, NULL,
      &joystick_port_map[JOYPORT_1], set_joystick_device, (void *)JOYPORT_1 },
    RESOURCE_INT_LIST_END
};

static resource_int_t joy2_resources_int[] = {
    { "JoyDevice2", JOYDEV_NONE, RES_EVENT_NO, NULL,
      &joystick_port_map[JOYPORT_2], set_joystick_device, (void *)JOYPORT_2 },
    RESOURCE_INT_LIST_END
};

static resource_int_t joy3_resources_int[] = {
    { "JoyDevice3", JOYDEV_NONE, RES_EVENT_NO, NULL,
      &joystick_port_map[JOYPORT_3], set_joystick_device, (void *)JOYPORT_3 },
    RESOURCE_INT_LIST_END
};

static resource_int_t joy4_resources_int[] = {
    { "JoyDevice4", JOYDEV_NONE, RES_EVENT_NO, NULL,
      &joystick_port_map[JOYPORT_4], set_joystick_device, (void *)JOYPORT_4 },
    RESOURCE_INT_LIST_END
};

static resource_int_t joy5_resources_int[] = {
    { "JoyDevice5", JOYDEV_NONE, RES_EVENT_NO, NULL,
      &joystick_port_map[JOYPORT_5], set_joystick_device, (void *)JOYPORT_5 },
    RESOURCE_INT_LIST_END
};


/** \brief  Initialize joystick resources
 *
 * \return  0 on success, -1 on failure
 */
int joystick_resources_init(void)
{
    if (joystick_joyport_register() < 0) {
        return -1;
    }

#ifdef COMMON_JOYKEYS
    if (resources_register_int(joykeys_resources_int) < 0) {
        return -1;
    }
#endif

    if (resources_register_int(joyopposite_resources_int) < 0) {
        return -1;
    }

#ifdef JOYDEV_DEFAULT
    switch (machine_class) {
        case VICE_MACHINE_C64:
        case VICE_MACHINE_C64SC:
        case VICE_MACHINE_C128:
        case VICE_MACHINE_C64DTV:
        case VICE_MACHINE_PLUS4:
        case VICE_MACHINE_SCPU64:
            joy2_resources_int[0].factory_value = JOYDEV_DEFAULT;
            break;
        case VICE_MACHINE_VIC20:
        case VICE_MACHINE_CBM5x0:
            joy1_resources_int[0].factory_value = JOYDEV_DEFAULT;
            break;
        case VICE_MACHINE_PET:
        case VICE_MACHINE_CBM6x0:
            break;
        default:
            break;
    }
#endif

    if (joyport_get_port_name(JOYPORT_1)) {
        if (resources_register_int(joy1_resources_int) < 0) {
            return -1;
        }
    }
    if (joyport_get_port_name(JOYPORT_2)) {
        if (resources_register_int(joy2_resources_int) < 0) {
            return -1;
        }
    }
    if (joyport_get_port_name(JOYPORT_3)) {
        if (resources_register_int(joy3_resources_int) < 0) {
            return -1;
        }
    }
    if (joyport_get_port_name(JOYPORT_4)) {
        if (resources_register_int(joy4_resources_int) < 0) {
            return -1;
        }
    }

    if (joyport_get_port_name(JOYPORT_5)) {
        if (resources_register_int(joy5_resources_int) < 0) {
            return -1;
        }
    }

#if (defined LINUX_JOYSTICK || defined HAS_USB_JOYSTICK || defined MAC_JOYSTICK)
    return 1;
#else
    return joy_arch_resources_init();
#endif
}

/* ------------------------------------------------------------------------- */

static const cmdline_option_t cmdline_options[] = {
    { "-joyopposite", SET_RESOURCE, CMDLINE_ATTRIB_NONE,
      NULL, NULL, "JoyOpposite", (resource_value_t)1,
      NULL, "Enable opposite joystick directions" },
    { "+joyopposite", SET_RESOURCE, CMDLINE_ATTRIB_NONE,
      NULL, NULL, "JoyOpposite", (resource_value_t)0,
      NULL, "Disable opposite joystick directions" },
#ifdef COMMON_JOYKEYS
    { "-keyset", SET_RESOURCE, CMDLINE_ATTRIB_NONE,
      NULL, NULL, "KeySetEnable", (resource_value_t)1,
      NULL, "Enable keyset" },
    { "+keyset", SET_RESOURCE, CMDLINE_ATTRIB_NONE,
      NULL, NULL, "KeySetEnable", (resource_value_t)0,
      NULL, "Disable keyset" },
#endif
    CMDLINE_LIST_END
};

/* Per-joystick command-line options.  */

static const cmdline_option_t joydev1cmdline_options[] = {
    { "-joydev1", SET_RESOURCE, CMDLINE_ATTRIB_NEED_ARGS,
      NULL, NULL, "JoyDevice1", NULL,
#ifdef HAS_USB_JOYSTICK
    "<0-13>", "Set device for joystick port 1 (0: None, 1: Numpad, 2: Keyset 1, 3: Keyset 2, 4: Analog joystick 0, 5: Analog joystick 1, 6: Analog joystick 2, 7: Analog joystick 3, 8: Analog joystick 4, 9: Analog joystick 5, 10: Digital joystick 0, 11: Digital joystick 1, 12: USB joystick 0, 13: USB joystick 1)" },
#else
    "<0-9>", "Set device for joystick port 1 (0: None, 1: Numpad, 2: Keyset 1, 3: Keyset 2, 4: Analog joystick 0, 5: Analog joystick 1, 6: Analog joystick 2, 7: Analog joystick 3, 8: Analog joystick 4, 9: Analog joystick 5)" },
#endif
    CMDLINE_LIST_END
};

static const cmdline_option_t joydev2cmdline_options[] = {
    { "-joydev2", SET_RESOURCE, CMDLINE_ATTRIB_NEED_ARGS,
      NULL, NULL, "JoyDevice2", NULL,
#ifdef HAS_USB_JOYSTICK
    "<0-13>", "Set device for joystick port 2 (0: None, 1: Numpad, 2: Keyset 1, 3: Keyset 2, 4: Analog joystick 0, 5: Analog joystick 1, 6: Analog joystick 2, 7: Analog joystick 3, 8: Analog joystick 4, 9: Analog joystick 5, 10: Digital joystick 0, 11: Digital joystick 1, 12: USB joystick 0, 13: USB joystick 1)" },
#else
    "<0-9>", "Set device for joystick port 2 (0: None, 1: Numpad, 2: Keyset 1, 3: Keyset 2, 4: Analog joystick 0, 5: Analog joystick 1, 6: Analog joystick 2, 7: Analog joystick 3, 8: Analog joystick 4, 9: Analog joystick 5)" },
#endif
    CMDLINE_LIST_END
};

static const cmdline_option_t joydev3cmdline_options[] = {
    { "-extrajoydev1", SET_RESOURCE, CMDLINE_ATTRIB_NEED_ARGS,
      NULL, NULL, "JoyDevice3", NULL,

#ifdef HAS_USB_JOYSTICK
    "<0-13>", "Set device for extra joystick port 1 (0: None, 1: Numpad, 2: Keyset 1, 3: Keyset 2, 4: Analog joystick 0, 5: Analog joystick 1, 6: Analog joystick 2, 7: Analog joystick 3, 8: Analog joystick 4, 9: Analog joystick 5, 10: Digital joystick 0, 11: Digital joystick 1, 12: USB joystick 0, 13: USB joystick 1)" },
#else
    "<0-9>", "Set device for extra joystick port 1 (0: None, 1: Numpad, 2: Keyset 1, 3: Keyset 2, 4: Analog joystick 0, 5: Analog joystick 1, 6: Analog joystick 2, 7: Analog joystick 3, 8: Analog joystick 4, 9: Analog joystick 5)" },
#endif
    CMDLINE_LIST_END
};

static const cmdline_option_t joydev4cmdline_options[] = {
    { "-extrajoydev2", SET_RESOURCE, CMDLINE_ATTRIB_NEED_ARGS,
      NULL, NULL, "JoyDevice4", NULL,
#ifdef HAS_USB_JOYSTICK
    "<0-13>", "Set device for extra joystick port 2 (0: None, 1: Numpad, 2: Keyset 1, 3: Keyset 2, 4: Analog joystick 0, 5: Analog joystick 1, 6: Analog joystick 2, 7: Analog joystick 3, 8: Analog joystick 4, 9: Analog joystick 5, 10: Digital joystick 0, 11: Digital joystick 1, 12: USB joystick 0, 13: USB joystick 1)" },
#else
    "<0-9>", "Set device for extra joystick port 2 (0: None, 1: Numpad, 2: Keyset 1, 3: Keyset 2, 4: Analog joystick 0, 5: Analog joystick 1, 6: Analog joystick 2, 7: Analog joystick 3, 8: Analog joystick 4, 9: Analog joystick 5)" },
#endif
    CMDLINE_LIST_END
};

static const cmdline_option_t joydev5cmdline_options[] = {
    { "-extrajoydev3", SET_RESOURCE, CMDLINE_ATTRIB_NEED_ARGS,
      NULL, NULL, "JoyDevice5", NULL,
#ifdef HAS_USB_JOYSTICK
    "<0-13>", "Set device for extra joystick port 3 (0: None, 1: Numpad, 2: Keyset 1, 3: Keyset 2, 4: Analog joystick 0, 5: Analog joystick 1, 6: Analog joystick 2, 7: Analog joystick 3, 8: Analog joystick 4, 9: Analog joystick 5, 10: Digital joystick 0, 11: Digital joystick 1, 12: USB joystick 0, 13: USB joystick 1)" },
#else
    "<0-9>", "Set device for extra joystick port 3 (0: None, 1: Numpad, 2: Keyset 1, 3: Keyset 2, 4: Analog joystick 0, 5: Analog joystick 1, 6: Analog joystick 2, 7: Analog joystick 3, 8: Analog joystick 4, 9: Analog joystick 5)" },
#endif
    CMDLINE_LIST_END
};


/** \brief  Initialize joystick command line options
 *
 * \return  0 on success, -1 on failure
 */
int joystick_cmdline_options_init(void)
{
    if (cmdline_register_options(cmdline_options) < 0) {
        return -1;
    }
    if (joyport_get_port_name(JOYPORT_1)) {
        if (cmdline_register_options(joydev1cmdline_options) < 0) {
            return -1;
        }
    }
    if (joyport_get_port_name(JOYPORT_2)) {
        if (cmdline_register_options(joydev2cmdline_options) < 0) {
            return -1;
        }
    }
    if (joyport_get_port_name(JOYPORT_3)) {
        if (cmdline_register_options(joydev3cmdline_options) < 0) {
            return -1;
        }
    }
    if (joyport_get_port_name(JOYPORT_4)) {
        if (cmdline_register_options(joydev4cmdline_options) < 0) {
            return -1;
        }
    }
    if (joyport_get_port_name(JOYPORT_5)) {
        if (cmdline_register_options(joydev5cmdline_options) < 0) {
            return -1;
        }
    }

#if (defined LINUX_JOYSTICK || defined HAS_USB_JOYSTICK || defined MAC_JOYSTICK)
    return 1;
#else
    return joy_arch_cmdline_options_init();
#endif
}

/*--------------------------------------------------------------------------*/

int joystick_init(void)
{
    joystick_alarm = alarm_new(maincpu_alarm_context, "Joystick",
                               joystick_latch_handler, NULL);

#ifdef COMMON_JOYKEYS
    kbd_initialize_numpad_joykeys(joykeys[0]);
#endif

#ifdef LINUX_JOYSTICK
    linux_joystick_init();
#elif defined HAS_USB_JOYSTICK
    usb_joystick_init();
#elif defined MAC_JOYSTICK
    joy_hidlib_init();
#else
    return joy_arch_init();
#endif

#if (defined LINUX_JOYSTICK || defined HAS_USB_JOYSTICK || defined MAC_JOYSTICK)
    int i;
    for (i = 0; i < JOYPORT_MAX_PORTS; i++) {
        if (joystick_port_map[i] >= JOYDEV_REALJOYSTICK_MIN) {
            if (joystick_port_map[i] - JOYDEV_REALJOYSTICK_MIN < num_joystick_devices) {
                joystick_devices[joystick_port_map[i] - JOYDEV_REALJOYSTICK_MIN].joyport = i;
            } else {
                joystick_port_map[i] = JOYDEV_NONE;
            }
        }
    }
    
    return 1;
#endif
}

/*--------------------------------------------------------------------------*/

#define DUMP_VER_MAJOR   1
#define DUMP_VER_MINOR   2

static int joystick_snapshot_write_module(snapshot_t *s, int port)
{
    snapshot_module_t *m;
    char snapshot_name[16];

    sprintf(snapshot_name, "JOYSTICK%d", port);

    m = snapshot_module_create(s, snapshot_name, DUMP_VER_MAJOR, DUMP_VER_MINOR);
    if (m == NULL) {
        return -1;
    }

    if (SMW_W(m, joystick_value[port]) < 0) {
        snapshot_module_close(m);
        return -1;
    }

    return snapshot_module_close(m);
}

static int joystick_snapshot_read_module(snapshot_t *s, int port)
{
    uint8_t major_version, minor_version;
    snapshot_module_t *m;
    char snapshot_name[16];

    sprintf(snapshot_name, "JOYSTICK%d", port);

    m = snapshot_module_open(s, snapshot_name, &major_version, &minor_version);
    if (m == NULL) {
        return -1;
    }

    if (!snapshot_version_is_equal(major_version, minor_version, DUMP_VER_MAJOR, DUMP_VER_MINOR)) {
        snapshot_module_close(m);
        return -1;
    }

    if (SMR_W(m, &joystick_value[port + 1]) < 0) {
        snapshot_module_close(m);
        return -1;
    }

    return snapshot_module_close(m);
}

/* ------------------------------------------------------------------------- */

#if (defined LINUX_JOYSTICK || defined BSD_JOYSTICK || defined MAC_JOYSTICK)
void register_joystick_driver(
    struct joystick_driver_s *driver,
    const char *jname,
    void *priv,
    int num_axes,
    int num_buttons,
    int num_hats)
{
    struct joystick_device_s *new_joystick_device;

    joystick_devices = lib_realloc(joystick_devices,
            sizeof(struct joystick_device_s) * (num_joystick_devices + 1));
    new_joystick_device = &joystick_devices[num_joystick_devices++];
    new_joystick_device->driver = driver;
    strncpy(new_joystick_device->jname, jname, JOYDEV_NAME_SIZE - 1);
    new_joystick_device->jname[JOYDEV_NAME_SIZE - 1] = '\0';
    new_joystick_device->axis_mapping = (joystick_axis_mapping_t*)lib_calloc(num_axes, sizeof(joystick_axis_mapping_t));
    if (num_axes > 1) {
        new_joystick_device->axis_mapping[0].positive_direction.action = JOYSTICK;
        new_joystick_device->axis_mapping[0].positive_direction.value.joy_pin = JOYSTICK_DIRECTION_RIGHT;
        new_joystick_device->axis_mapping[0].negative_direction.action = JOYSTICK;
        new_joystick_device->axis_mapping[0].negative_direction.value.joy_pin = JOYSTICK_DIRECTION_LEFT;
        new_joystick_device->axis_mapping[1].positive_direction.action = JOYSTICK;
        new_joystick_device->axis_mapping[1].positive_direction.value.joy_pin = JOYSTICK_DIRECTION_UP;
        new_joystick_device->axis_mapping[1].negative_direction.action = JOYSTICK;
        new_joystick_device->axis_mapping[1].negative_direction.value.joy_pin = JOYSTICK_DIRECTION_DOWN;
    }
    new_joystick_device->button_mapping = (joystick_mapping_t *)lib_calloc(num_buttons, sizeof(joystick_mapping_t));
    if (num_buttons > 0) {
        new_joystick_device->button_mapping[0].action = JOYSTICK;
        new_joystick_device->button_mapping[0].value.joy_pin = 16;
    }
    if (num_buttons > 1) {
        new_joystick_device->button_mapping[1].action = JOYSTICK;
        new_joystick_device->button_mapping[1].value.joy_pin = 32;
    }
    if (num_buttons > 2) {
        new_joystick_device->button_mapping[2].action = JOYSTICK;
        new_joystick_device->button_mapping[2].value.joy_pin = 64;
    }
    new_joystick_device->hat_mapping = (joystick_hat_mapping_t *)lib_calloc(num_hats, sizeof(joystick_hat_mapping_t));
    new_joystick_device->joyport = -1;
    new_joystick_device->priv = priv;
}

static void joy_perform_event(joystick_mapping_t *event, int joyport, int value)
{
    switch (event->action) {
        case JOYSTICK:
            if (joyport >=0 && joyport < JOYPORT_MAX_PORTS) {
                if (value) {
                    joystick_set_value_or(joyport, event->value.joy_pin);
                } else {
                    joystick_set_value_and(joyport, (uint8_t) ~(event->value.joy_pin));
                }
            }
            break;
        case KEYBOARD:
            keyboard_set_keyarr_any(event->value.key[0], event->value.key[1], value);
            break;
        case NONE:
        default:
            break;
    }
}


void joy_axis_event(uint8_t joynum, uint8_t axis, joystick_axis_value_t value)
{
    joystick_axis_value_t prev;
    int joyport;

    prev = joystick_devices[joynum].axis_mapping[axis].prev;


    if (value == prev) {
        return;
    }

    joyport = joystick_devices[joynum].joyport;

    if (value == JOY_AXIS_POSITIVE) {
        if (prev == JOY_AXIS_NEGATIVE) {
            joy_perform_event(&joystick_devices[joynum].axis_mapping[axis].negative_direction, joyport, 0);
        }
        joy_perform_event(&joystick_devices[joynum].axis_mapping[axis].positive_direction, joyport, 1);
    } else if (value == JOY_AXIS_NEGATIVE) {
        if (prev == JOY_AXIS_POSITIVE) {
            joy_perform_event(&joystick_devices[joynum].axis_mapping[axis].positive_direction, joyport, 0);
        }
        joy_perform_event(&joystick_devices[joynum].axis_mapping[axis].negative_direction, joyport, 1);
    } else {
        if (prev == JOY_AXIS_POSITIVE) {
            joy_perform_event(&joystick_devices[joynum].axis_mapping[axis].positive_direction, joyport, 0);
        } else {
            joy_perform_event(&joystick_devices[joynum].axis_mapping[axis].negative_direction, joyport, 0);
        }
    }

    joystick_devices[joynum].axis_mapping[axis].prev = value;
}

void joy_button_event(uint8_t joynum, uint8_t button, uint8_t value)
{
    joy_perform_event(&(joystick_devices[joynum].button_mapping[button]),
                      joystick_devices[joynum].joyport, value);
}

void joy_hat_event(uint8_t joynum, uint8_t hat, uint8_t value)
{
    uint8_t prev;
    int joyport;

    prev = joystick_devices[joynum].hat_mapping[hat].prev;
    if (value == prev) {
        return;
    }

    joyport = joystick_devices[joynum].joyport;

    if (!(value & JOYSTICK_DIRECTION_UP) && (prev & JOYSTICK_DIRECTION_UP)) {
        joy_perform_event(&joystick_devices[joynum].hat_mapping[hat].up, joyport, 0);
        if ((value & JOYSTICK_DIRECTION_DOWN) && !(prev & JOYSTICK_DIRECTION_DOWN)) {
            joy_perform_event(&joystick_devices[joynum].hat_mapping[hat].down, joyport, 1);
        }
    }
    if (!(value & JOYSTICK_DIRECTION_DOWN) && (prev & JOYSTICK_DIRECTION_DOWN)) {
        joy_perform_event(&joystick_devices[joynum].hat_mapping[hat].down, joyport, 0);
        if ((value & JOYSTICK_DIRECTION_UP) && !(prev & JOYSTICK_DIRECTION_UP)) {
            joy_perform_event(&joystick_devices[joynum].hat_mapping[hat].up, joyport, 1);
        }
    }
    if (!(value & JOYSTICK_DIRECTION_LEFT) && (prev & JOYSTICK_DIRECTION_LEFT)) {
        joy_perform_event(&joystick_devices[joynum].hat_mapping[hat].left, joyport, 0);
        if ((value & JOYSTICK_DIRECTION_RIGHT) && !(prev & JOYSTICK_DIRECTION_RIGHT)) {
            joy_perform_event(&joystick_devices[joynum].hat_mapping[hat].right, joyport, 1);
        }
    }
    if (!(value & JOYSTICK_DIRECTION_RIGHT) && (prev & JOYSTICK_DIRECTION_RIGHT)) {
        joy_perform_event(&joystick_devices[joynum].hat_mapping[hat].right, joyport, 0);
        if ((value & JOYSTICK_DIRECTION_LEFT) && !(prev & JOYSTICK_DIRECTION_LEFT)) {
            joy_perform_event(&joystick_devices[joynum].hat_mapping[hat].left, joyport, 1);
        }
    }

    joystick_devices[joynum].hat_mapping[hat].prev = value;
}

static int joystickdeviceidx = 0;

void joystick_ui_reset_device_list(void)
{
    joystickdeviceidx = 0;
}

const char *joystick_ui_get_next_device_name(int *id)
{
    if (joystickdeviceidx >=0 && joystickdeviceidx < num_joystick_devices) {
        *id = joystickdeviceidx + JOYDEV_REALJOYSTICK_MIN;
        return joystick_devices[joystickdeviceidx++].jname;
    }
    return NULL;
}

void joystick(void)
{
    int i;
    for (i = 0; i < num_joystick_devices; i++) {
        joystick_devices[i].driver->poll(i, joystick_devices[i].priv);
    }
}

void joystick_close(void)
{
    int i;

    for (i = 0; i < num_joystick_devices; i++) {
        joystick_devices[i].driver->close(joystick_devices[i].priv);
        lib_free(joystick_devices[i].axis_mapping);
        lib_free(joystick_devices[i].button_mapping);
        lib_free(joystick_devices[i].hat_mapping);
    }

    lib_free(joystick_devices);
}
#endif //LINUX_JOYSTICK||BSD_JOYSTICK||MAC_JOYSTICK
