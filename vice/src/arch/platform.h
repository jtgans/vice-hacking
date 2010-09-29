/*
 * platform.h - port/platform specific discovery macros.
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

/* Operating systems supported:
 *
 * platform     | support
 * ----------------------
 * aix          | yes
 * amigaos4     | yes
 * aros         | yes
 * beos         | yes
 * bsdi         | yes
 * cygwin       | yes
 * dragonflybsd | yes
 * freebsd      | yes
 * hpux         | yes
 * irix         | yes
 * linux        | yes, but no libc type and version yet
 * macosx       | yes
 * morphos      | yes
 * netbsd       | yes
 * openbsd      | yes
 * openserver   | yes
 * qnx          | yes
 * solaris      | yes
 * sunos        | yes
 * unixware     | yes
 * uwin         | yes
 * win32        | yes
 * win64        | yes
 */

#ifndef VICE_PLATFORM_H
#define VICE_PLATFORM_H

#include "vice.h"

/* win32/64 discovery */
#ifdef WIN32_COMPILE
#  ifdef _WIN64
#    ifdef WINIA64
#      define PLATFORM_CPU "IA64"
#    else
#      define PLATFORM_CPU "X64"
#    endif
#    define PLATFORM_OS "WIN64"
#    define PLATFORM_COMPILER "MSVC"
#  else
#    ifdef MSVC_RC
#      ifdef WATCOM_COMPILE
#        define PLATFORM_COMPILER "WATCOM"
#      else
#        define PLATFORM_COMPILER "MSVC"
#      endif
#    endif
#    define PLATFORM_OS "WIN32"
#    define FIND_X86_CPU
#  endif
#endif

#if !defined(WIN32_COMPILE) && defined(__CYGWIN32__)
#define PLATFORM_OS "Cygwin"
#define FIND_X86_CPU
#endif

/* MacOS X discovery */
#ifdef __APPLE__
#   define PLATFORM_OS "Mac OS X"
#   ifdef __llvm__
#       define PLATFORM_COMPILER  "llvm"
#   endif
#   ifdef __POWERPC__
#       define PLATFORM_CPU "ppc"
#   else
#       ifdef __x86_64
#           define PLATFORM_CPU "x86_64"
#       else
#           define PLATFORM_CPU "i386"
#       endif
#   endif
#endif

/* AIX discovery */

#ifdef _AIX

/* find out which compiler is being used */
#ifdef __TOS_AIX__
#  define PLATFORM_COMPILER "xLC"
#endif

/* Get AIX version */
#include "platform_aix_version.h"

#endif /* AIX */


/* AmigaOS 4.x discovery */
#ifdef AMIGA_OS4
#define PLATFORM_OS "AmigaOS 4.x"
#define PLATFORM_CPU "PPC"
#endif


/* AROS discovery */
#ifdef AMIGA_AROS
#define PLATFORM_OS "AROS"
#endif


/* MorphOS discovery */
#ifdef AMIGA_MORPHOS
#define PLATFORM_OS "MorphOS"
#endif


/* BeOS discovery */
#ifdef __BEOS__

#ifdef WORDS_BIGENDIAN
#define PLATFORM_CPU "PPC"
#define PLATFORM_COMPILER "MetroWerks"
#else
#define FIND_X86_CPU
#endif

#define PLATFORM_OS "BeOS"

#endif /* __BEOS__ */


/* BSDI discovery */
#ifdef __bsdi__
#define PLATFORM_OS "BSDi"
#define FIND_X86_CPU
#endif


/* DragonFly BSD discovery */
#ifdef __DragonFly__
#define PLATFORM_OS "DragonFly BSD"
#define FIND_X86_CPU
#endif


/* FreeBSD discovery */
#ifdef __FreeBSD__

/* Get FreeBSD version */
#include "platform_freebsd_version.h"

#endif


/* NetBSD discovery */
#ifdef __NetBSD__

/* Get NetBSD version */
#include "platform_netbsd_version.h"

#endif

/* OpenBSD discovery */
#ifdef __OpenBSD__

/* Get OpenBSD version */
#include "platform_openbsd_version.h"

#endif

/* QNX 4.x discovery */
#if defined(__QNX__) && !defined(__QNXNTO__)
#define PLATFORM_OS "QNX 4.x"
#define PLATFORM_COMPILER "Watcom"
#define FIND_X86_CPU
#endif


/* QNX 6.x discovery */
#ifdef __QNXNTO__

/* Get QNX version */
#include "platform_qnx6_version.h"

#ifdef __arm__
#define PLATFORM_CPU "ARMLE"
#endif

#ifdef __mips__
#define PLATFORM_CPU "MIPSLE"
#endif

#ifdef __sh__
#define PLATFORM_CPU "SHLE"
#endif

#if defined(__powerpc__) || defined(__ppc__)
#define PLATFORM_CPU "PPCBE"
#endif

#ifndef PLATFORM_CPU
#define FIND_X86_CPU
#endif

#endif


/* HPUX discovery */
#ifdef _hpux
#define PLATFORM_OS "HPUX"
#define PLATFORM_COMPILER "HP UPC"
#endif

#if defined(__hpux) && !defined(_hpux)
#define PLATFORM_OS "HPUX"
#endif


/* IRIX discovery */
#ifdef __sgi
#define PLATFORM_OS "IRIX"
#endif


/* OpenServer 5.x discovery */
#ifdef OPENSERVER5_COMPILE
#define PLATFORM_OS "OpenServer 5.x"
#define FIND_X86_CPU
#endif


/* OpenServer 6.x discovery */
#ifdef OPENSERVER6_COMPILE
#define PLATFORM_OS "OpenServer 6.x"
#define FIND_X86_CPU
#endif


/* UnixWare 7.x discovery */
#ifdef _UNIXWARE7
#define PLATFORM_OS "UnixWare 7.x"
#define FIND_X86_CPU
#endif


/* SunOS and Solaris discovery */
#if defined(sun) || defined(__sun)
#  if defined(__SVR4) || defined(__svr4__)
#    define PLATFORM_OS "Solaris"
# else
#    define PLATFORM_OS "SunOS"
#  endif
#endif


/* UWIN discovery */
#ifdef _UWIN
#define PLATFORM_OS "UWIN"
#define FIND_X86_CPU
#endif


/* Linux discovery */
#ifdef __linux
#include "platform_linux_libc_version.h"
#endif


/* Generic cpu discovery */
#include "platform_cpu_type.h"


#if !defined(PLATFORM_COMPILER) && defined(__GNUC__)
#define PLATFORM_COMPILER "GCC"
#endif

/* Fallbacks for unidentified systems */
#ifndef PLATFORM_CPU
#define PLATFORM_CPU "unknown CPU"
#endif
#ifndef PLATFORM_OS
#define PLATFORM_OS "unknown OS"
#endif
#ifndef PLATFORM_COMPILER
#define PLATFORM_COMPILER "unknown compiler"
#endif

#endif
