/*
 * ssi2001-drv.c - SSI2001 (ISA SID card) support for WIN32.
 *
 * Written by
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

#include "vice.h"

#ifdef HAVE_SSI2001
#include <windows.h>
#include <fcntl.h>
#include <stdio.h>
#include <conio.h>
#include <assert.h>

#include "types.h"

#define SSI2008_BASE 0x280

static int ssi2001_use_lib = 0;

#define MAXSID 1

static int sids_found = -1;

#ifndef MSVC_RC
typedef short _stdcall (*inpfuncPtr)(short portaddr);
typedef void _stdcall (*oupfuncPtr)(short portaddr, short datum);

static inpfuncPtr inp32fp;
static oupfuncPtr oup32fp;
#else
typedef short (CALLBACK* Inp32_t)(short);
typedef void (CALLBACK* Out32_t)(short, short);

static Inp32_t Inp32;
static Out32_t Out32;
#endif

/* input/output functions */
static void ssi2001_outb(unsigned int addrint, short value)
{
    WORD addr = (WORD)addrint;

    /* make sure the above conversion did not loose any details */
    assert(addr == addrint);

    if (ssi2001_use_lib) {
#ifndef MSVC_RC
        (oup32fp)(addr, value);
#else
        Out32(addr, value);
#endif
    } else {
#ifdef  _M_IX86
#ifdef WATCOM_COMPILE
        outp(addr, value);
#else
        _outp(addr, value);
#endif
#endif
    }
}

static short ssi2001_inb(unsigned int addrint)
{
    WORD addr = (WORD)addrint;

    /* make sure the above conversion did not loose any details */
    assert(addr == addrint);

    if (ssi2001_use_lib) {
#ifndef MSVC_RC
        return (inp32fp)(addr);
#else
        return Inp32(addr);
#endif
    } else {
#ifdef  _M_IX86
#ifdef WATCOM_COMPILE
        return inp(addr);
#else
        return _inp(addr);
#endif
#endif
    }
}

int ssi2001_drv_read(WORD addr, int chipno)
{
    if (chipno < MAXSID && addr < 0x20) {
        return ssi2001_inb(SSI2008_BASE + (addr & 0x1f));
    }
    return 0;
}

void ssi2001_drv_store(WORD addr, BYTE outval, int chipno)
{
    if (chipno < MAXSID && addr < 0x20) {
        ssi2001_outb(SSI2008_BASE + (addr & 0x1f), outval);
    }
}

/*----------------------------------------------------------------------*/

static HINSTANCE hLib = NULL;

#ifdef _MSC_VER
#  ifdef _WIN64
#    define INPOUTDLLNAME "inpoutx64.dll"
#  else
#    define INPOUTDLLNAME "inpout32.dll"
#  endif
#else
#  if defined(__amd64__) || defined(__x86_64__)
#    define INPOUTDLLNAME "inpoutx64.dll"
#  else
#    define INPOUTDLLNAME "inpout32.dll"
#  endif
#endif

static int detect_sid(void)
{
    int i;

    for (i = 0x18; i >= 0; --i) {
        ssi2001_drv_store((WORD)i, 0, 0);
    }

    ssi2001_drv_store(0x12, 0xff, 0);

    for (i = 0; i < 100; ++i) {
        if (ssi2001_drv_read(0x1b, 0)) {
            return 0;
        }
    }

    ssi2001_drv_store(0x0e, 0xff, 0);
    ssi2001_drv_store(0x0f, 0xff, 0);
    ssi2001_drv_store(0x12, 0x20, 0);

    for (i = 0; i < 100; ++i) {
        if (ssi2001_drv_read(0x1b, 0)) {
            return 1;
        }
    }
    return 0;
}

int ssi2001_drv_open(void)
{
    if (!sids_found) {
        return -1;
    }

    if (sids_found > 0) {
        return 0;
    }

    sids_found = 0;

    if (hLib == NULL) {
        hLib = LoadLibrary(INPOUTDLLNAME);
    }

    ssi2001_use_lib = 0;

    if (hLib != NULL) {
#ifdef SSI2001_DEBUG
        printf("Loaded %s\n", INPOUTDLLNAME);
#endif
#ifndef MSVC_RC
        inp32fp = (inpfuncPtr)GetProcAddress(hLib, "Inp32");
        if (inp32fp != NULL) {
            oup32fp = (oupfuncPtr)GetProcAddress(hLib, "Out32");
            if (oup32fp != NULL) {
#ifdef SSI2001_DEBUG
                printf("Using Inp32 and Out32 functions found in %s\n", INPOUTDLLNAME);
#endif
                ssi2001_use_lib = 1;
            }
        }
#else
        Inp32 = (Inp32_t)GetProcAddress(hLib, "Inp32");
        if (Inp32 != NULL) {
            Out32 = (Out32_t)GetProcAddress(hLib, "Out32");
            if (Out32 != NULL) {
#ifdef SSI2001_DEBUG
                printf("Using Inp32 and Out32 functions found in %s\n", INPOUTDLLNAME);
#endif
                ssi2001_use_lib = 1;
            }
        }
#endif
    }

    if (!(GetVersion() & 0x80000000) && ssi2001_use_lib == 0) {
#ifdef SSI2001_DEBUG
        printf("Working on NT and no dll loaded\n");
#endif
        return -1;
    }

    if (detect_sid()) {
#ifdef SSI2001_DEBUG
        if (ssi2001_use_lib) {
            printf("SSI2001 found at $280 using Inp32() / Outp32() method\n");
        } else {
            printf("SSI2001 found at $280 using direct access method\n");
        }
#endif
        sids_found = 1;
        return 0;
    }
#ifdef SSI2001_DEBUG
    if (ssi2001_use_lib) {
        printf("NO SSI2001 found using Inp32() / Outp32() method\n");
    } else {
        printf("NO SSI2001 found using direct access method\n");
    }
#endif
    return -1;
}

int ssi2001_drv_close(void)
{
    if (ssi2001_use_lib) {
       FreeLibrary(hLib);
       hLib = NULL;
    }

    sids_found = -1;

    return 0;
}

int ssi2001_drv_available(void)
{
    return sids_found;
}
#endif
