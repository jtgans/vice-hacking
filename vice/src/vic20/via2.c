
/*
 * ./vic20/via2.c
 * This file is generated from ./via-tmpl.c and ./vic20/via2.def,
 * Do not edit!
 */
/*
 * via-tmpl.c - Template file for VIA emulation.
 *
 * Written by
 *  Andr� Fachat (fachat@physik.tu-chemnitz.de)
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

/*
 * 24jan97 a.fachat
 * new interrupt handling, hopefully according to the specs now.
 * All interrupts (note: not timer events (i.e. alarms) are put
 * into one interrupt flag, I_VIA2FL.
 * if an interrupt condition changes, the function (i.e. cpp macro)
 * update_via2irq() id called, that checks the IRQ line state.
 * This is now possible, as ettore has decoupled A_* alarm events
 * from I_* interrupts for performance reasons.
 *
 * A new function for signaling rising/falling edges on the
 * control lines is introduced:
 *      via2_signal(VIA_SIG_[CA1|CA2|CB1|CB2], VIA_SIG_[RISE|FALL])
 * which signals the corresponding edge to the VIA. The constants
 * are defined in via.h.
 *
 * Except for shift register and input latching everything should be ok now.
 */

#define update_via2irq() \
        maincpu_set_nmi(I_VIA2FL, (via2ifr & via2ier & 0x7f) ? IK_NMI : 0)

#include "vice.h"

#include <stdio.h>
#include <time.h>

#include "vmachine.h"
#include "via.h"
#include "resources.h"


#include "true1541.h"
#include "kbd.h"

#include "interrupt.h"

/*#define VIA2_TIMER_DEBUG */

/* global */

BYTE    via2[16];
#if 0
int     via2ta_stop = 0; /* maybe 1? */
int     via2tb_stop = 0; /* maybe 1? */
int     via2ta_interrupt = 0;
int     via2tb_interrupt = 0;
#endif



/* local functions */

static void update_via2ta ( int );
static void update_via2tb ( int );

/*
 * Local variables
 */

static int   via2ifr;	/* Interrupt Flag register for via2 */
static int   via2ier;	/* Interrupt Enable register for via2 */

static int   via2ta;	/* value of via2 timer A at last update */
static int   via2tb;	/* value of via2 timer B at last update */

static CLOCK via2tau;	/* time when via2 timer A is updated */
static CLOCK via2tbu;	/* time when via2 timer B is updated */


/* ------------------------------------------------------------------------- */
/* VIA2 */



/*
 * according to Rockwell, all internal registers are cleared, except
 * for the Timer (1 and 2, counter and latches) and the shift register.
 */
void    reset_via2(void)
{
    int i;
#ifdef VIA2_TIMER_DEBUG
    if(app_resources.debugFlag) printf("VIA2: reset\n");
#endif
    /* clear registers */
    for(i=0;i<4;i++) via2[i]=0;
    for(i=11;i<16;i++) via2[i]=0;

    via2ier = 0;
    via2ifr = 0;

    /* disable vice interrupts */
#ifdef OLDIRQ
    maincpu_set_nmi(I_VIA2T1, 0); maincpu_unset_alarm(A_VIA2T1);
    maincpu_set_nmi(I_VIA2T2, 0); maincpu_unset_alarm(A_VIA2T2);
    maincpu_set_nmi(I_VIA2SR, 0);
    maincpu_set_nmi(I_VIA2FL, 0);
#else
    maincpu_unset_alarm(A_VIA2T1);
    maincpu_unset_alarm(A_VIA2T2);
    update_via2irq();
#endif


     serial_bus_pa_write(0xff);

}

void via2_signal(int line, int edge) {
        switch(line) {
        case VIA_SIG_CA1:
                via2ifr |= ((edge ^ via2[VIA_PCR]) & 0x01) ?
                                                        0 : VIA_IM_CA1;
                update_via2irq();
                break;
        case VIA_SIG_CA2:
                if( !(via2[VIA_PCR] & 0x08)) {
                  via2ifr |= (((edge<<2) ^ via2[VIA_PCR]) & 0x04) ?
                                                        0 : VIA_IM_CA2;
                  update_via2irq();
                }
                break;
        case VIA_SIG_CB1:
                via2ifr |= (((edge<<4) ^ via2[VIA_PCR]) & 0x10) ?
                                                        0 : VIA_IM_CB1;
                update_via2irq();
                break;
        case VIA_SIG_CB2:
                if( !(via2[VIA_PCR] & 0x80)) {
                  via2ifr |= (((edge<<6) ^ via2[VIA_PCR]) & 0x40) ?
                                                        0 : VIA_IM_CB2;
                  update_via2irq();
                }
                break;
        }
}

void REGPARM2 store_via2(ADDRESS addr, BYTE byte)
{
    addr &= 0xf;
#ifdef VIA2_TIMER_DEBUG
    if (app_resources.debugFlag)
	printf("store via2[%x] %x\n", (int) addr, (int) byte);
#endif

    switch (addr) {

	/* these are done with saving the value */
      case VIA_PRA: /* port A */
        via2ifr &= ~VIA_IM_CA1;
        if( (via2[VIA_PCR] & 0x0a) != 0x2) {
          via2ifr &= ~VIA_IM_CA2;
        }
#ifndef OLDIRQ
        update_via2irq();
#endif
      case VIA_PRA_NHS: /* port A, no handshake */
	via2[VIA_PRA_NHS] = byte;
	addr = VIA_PRA;
      case VIA_DDRA:

     via2[addr] = byte;
     serial_bus_pa_write(via2[VIA_PRA] | (~via2[VIA_DDRA]));
	break;

      case VIA_PRB: /* port B */
        via2ifr &= ~VIA_IM_CB1;
        if( (via2[VIA_PCR] & 0xa0) != 0x20) {
          via2ifr &= ~VIA_IM_CB2;
        }
#ifndef OLDIRQ
        update_via2irq();
#endif
      case VIA_DDRB:
	via2[addr] = byte;
	break;

      case VIA_SR: /* Serial Port output buffer */
	via2[addr] = byte;
	
	break;

	/* Timers */

      case VIA_T1CL:
      case VIA_T1LL:
	via2[VIA_T1LL] = byte;
	break;

      case VIA_T1CH /*TIMER_AH*/: /* Write timer A high */
#ifdef VIA2_TIMER_DEBUG
	if(app_resources.debugFlag) printf("Write timer A high: %02x\n",byte);
#endif
	via2[VIA_T1LH] = byte;
        /* load counter with latch value */
        via2[VIA_T1CL] = via2[VIA_T1LL];
        via2[VIA_T1CH] = via2[VIA_T1LH];
        /* Clear T1 interrupt */
        via2ifr &= ~VIA_IM_T1;
#ifdef OLDIRQ
        maincpu_set_nmi(I_VIA2T1, 0);
#else
        update_via2irq();
#endif
        update_via2ta(1);
        break;

      case VIA_T1LH: /* Write timer A high order latch */
        via2[addr] = byte;
        /* Clear T1 interrupt */
        via2ifr &= ~VIA_IM_T1;
#ifdef OLDIRQ
        maincpu_set_nmi(I_VIA2T1, 0);
#else
	update_via2irq();
#endif
        break;

      case VIA_T2LL:	/* Write timer 2 low latch */
	via2[VIA_T2LL] = byte;
	
	break;

      case VIA_T2CH: /* Write timer 2 high */
        via2[VIA_T2CH] = byte;
        via2[VIA_T2CL] = via2[VIA_T2LL]; /* bogus, both are identical */
        update_via2tb(1);
        /* Clear T2 interrupt */
        via2ifr &= ~VIA_IM_T2;
#ifdef OLDIRQ
        maincpu_set_nmi(I_VIA2T2, 0);
#else
	update_via2irq();
#endif
        break;

	/* Interrupts */

      case VIA_IFR: /* 6522 Interrupt Flag Register */
        via2ifr &= ~byte;
#ifdef OLDIRQ
        if(!via2ifr & VIA_IM_T1) maincpu_set_nmi(I_VIA2T1, 0);
        if(!via2ifr & VIA_IM_T2) maincpu_set_nmi(I_VIA2T2, 0);
#else
	update_via2irq();
#endif
        break;

      case VIA_IER: /* Interrupt Enable Register */
#if defined (VIA2_TIMER_DEBUG)
        printf ("Via#1 set VIA_IER: 0x%x\n", byte);
#endif
        if (byte & VIA_IM_IRQ) {
            /* set interrupts */
#ifdef OLDIRQ
            if ((byte & VIA_IM_T1) && (via2ifr & VIA_IM_T1)) {
                maincpu_set_nmi(I_VIA2T1, IK_NMI);
            }
            if ((byte & VIA_IM_T2) && (via2ifr & VIA_IM_T2)) {
                maincpu_set_nmi(I_VIA2T2, IK_NMI);
            }
#endif
            via2ier |= byte & 0x7f;
        }
        else {
            /* clear interrupts */
#ifdef OLDIRQ
            if( byte & VIA_IM_T1 ) maincpu_set_nmi(I_VIA2T1, 0);
            if( byte & VIA_IM_T2 ) maincpu_set_nmi(I_VIA2T2, 0);
#endif
            via2ier &= ~byte;
        }
#ifndef OLDIRQ
	update_via2irq();
#endif
        break;

	/* Control */

      case VIA_ACR:
	via2[addr] = byte;

	

	/* bit 7 timer 1 output to PB7 */
	/* bit 6 timer 1 run mode -- default seems to be continuous */

	/* bit 5 timer 2 count mode */
	if (byte & 32) {
/* TODO */
/*	    update_via2tb(0);*/	/* stop timer if mode == 1 */
	}

	/* bit 4, 3, 2 shift register control */

	/* bit 1, 0  latch enable port B and A */
	break;

      case VIA_PCR:

/*        if(viadebug) printf("VIA1: write %02x to PCR\n",byte);*/

	/* bit 7, 6, 5  CB2 handshake/interrupt control */
	/* bit 4  CB1 interrupt control */

	/* bit 3, 2, 1  CA2 handshake/interrupt control */
	/* bit 0  CA1 interrupt control */

	
	via2[addr] = byte;
	break;

      default:
	via2[addr] = byte;

    }  /* switch */
}


/* ------------------------------------------------------------------------- */

BYTE REGPARM1 read_via2(ADDRESS addr)
{
    addr &= 0xf;
#ifdef VIA2_TIMER_DEBUG
    if (app_resources.debugFlag)
	printf("read via2[%d]\n", addr);
#endif
    switch (addr) {

      case VIA_PRA: /* port A */
        via2ifr &= ~VIA_IM_CA1;
        if( (via2[VIA_PCR] & 0x0a) != 0x02) {
          via2ifr &= ~VIA_IM_CA2;
        }
#ifndef OLDIRQ
        update_via2irq();
#endif
      case VIA_PRA_NHS: /* port A, no handshake */

     {
	  BYTE joy_bits;

	  /*
	     Port A is connected this way:
	     
	     bit 0  IEC clock
	     bit 1  IEC data
	     bit 2  joystick switch 0 (up)
	     bit 3  joystick switch 1 (down)
	     bit 4  joystick switch 2 (left)
	     bit 5  joystick switch 4 (fire)
	     bit 6  IEC ATN

	  */

	  /* Setup joy bits (2 through 5).  Use the `or' of the values
             of both joysticks so that it works with every joystick
             setting.  This is a bit slow... we might think of a
             faster method.  */
	  joy_bits = ~(joy[1] | joy[2]);
	  joy_bits = ((joy_bits & 0x7) << 2) | ((joy_bits & 0x10) << 1);

	  /* We assume `serial_bus_pa_read()' returns the non-IEC bits
             as zeroes. */
	  return ((via2[VIA_PRA] & via2[VIA_DDRA]) 
		  | ((serial_bus_pa_read() | joy_bits) & ~via2[VIA_DDRA]));
     }

      case VIA_PRB: /* port B */
        via2ifr &= ~VIA_IM_CB1;
        if( (via2[VIA_PCR] & 0xa0) != 0x20) {
          via2ifr &= ~VIA_IM_CB2;
        }
#ifndef OLDIRQ
        update_via2irq();
#endif

     return (via2[VIA_PRB] & via2[VIA_DDRB]) | (0xff & ~via2[VIA_DDRB]);

	/* Timers */

      case VIA_T1CL /*TIMER_AL*/: /* timer A low */
        via2ifr &= ~VIA_IM_T1;
#ifdef OLDIRQ
        maincpu_set_nmi(I_VIA2T1, 0);
#else
	update_via2irq();
#endif
        return ((via2ta - clk + via2tau) & 0xff);


      case VIA_T1CH /*TIMER_AH*/: /* timer A high */
        return (((via2ta - clk + via2tau) >> 8) & 0xff);

      case VIA_T2CL /*TIMER_BL*/: /* timer B low */
        via2ifr &= ~VIA_IM_T2;
#ifdef OLDIRQ
        maincpu_set_nmi(I_VIA2T2, 0);
#else
	update_via2irq();
#endif
        return ((via2tb - clk + via2tbu) & 0xff);

      case VIA_T2CH /*TIMER_BH*/: /* timer B high */
        return (((via2tb - clk + via2tbu) >> 8) & 0xff);

      case VIA_SR: /* Serial Port Shift Register */
	return (via2[addr]);

	/* Interrupts */

      case VIA_IFR: /* Interrupt Flag Register */
	{
	    BYTE    t = via2ifr;
	    if (via2ifr & via2ier /*[VIA_IER]*/)
		t |= 0x80;
	    return (t);
	}

      case VIA_IER: /* 6522 Interrupt Control Register */
	    return (via2ier /*[VIA_IER]*/ | 0x80);

    }  /* switch */

    return (via2[addr]);
}


/* ------------------------------------------------------------------------- */

int    int_via2t1(long offset)
{
#ifdef VIA2_TIMER_DEBUG
    if (app_resources.debugFlag)
	printf("via2 timer A interrupt\n");
#endif

    if (!(via2[VIA_ACR] & 0x40)) { /* one-shot mode */
#if defined (VIA2_TIMER_DEBUG)
        printf ("VIA2 Timer A interrupt -- one-shot mode: next int won't happen\n");
#endif
	via2ta = 0;
	via2tau = clk;
	maincpu_unset_alarm(A_VIA2T1);		/*int_clk[I_VIA2T1] = 0;*/
    }
    else {		/* continuous mode */
        /* load counter with latch value */
        via2[VIA_T1CL] = via2[VIA_T1LL];
        via2[VIA_T1CH] = via2[VIA_T1LH];
	update_via2ta(1);
/*	int_clk[I_VIA2T1] = via2tau + via2ta;*/
    }
    via2ifr |= VIA_IM_T1;
#ifdef OLDIRQ
    if(via2ier /*[VIA_IER]*/ & VIA_IM_T1 )
	maincpu_set_nmi(I_VIA2T1, IK_NMI);
#else
    update_via2irq();
#endif
    return 0; /*(viaier & VIA_IM_T1) ? 1:0;*/
}

/*
 * Timer B is always in one-shot mode
 */

int    int_via2t2(long offset)
{
#ifdef VIA2_TIMER_DEBUG
    if (app_resources.debugFlag)
	printf("VIA2 timer B interrupt\n");
#endif
    via2tb = 0;
    via2tbu = clk;
    maincpu_unset_alarm(A_VIA2T2);	/*int_clk[I_VIA2T2] = 0;*/

    via2ifr |= VIA_IM_T2;
#ifdef OLDIRQ
    if( via2ier & VIA_IM_T2 ) maincpu_set_nmi(I_VIA2T2, IK_NMI);
#else
    update_via2irq();
#endif

    return 0;
}


/* ------------------------------------------------------------------------- */

static void update_via2ta(int force)
{
    if(force) {
#ifdef VIA2_TIMER_DEBUG
       if(app_resources.debugFlag)
          printf("update via timer A : latch=%d, counter =%d, clk = %d\n",
                via2[VIA_T1CL] + (via2[VIA_T1CH] << 8),
                via2[VIA_T1LL] + (via2[VIA_T1LH] << 8),
                clk);
#endif
      via2ta = via2[VIA_T1CL] + (via2[VIA_T1CH] << 8);
      via2tau = clk;
      maincpu_set_alarm(A_VIA2T1, via2ta);
    }
}

static void update_via2tb(int force)
{
    if(force) {
      via2tb = via2[VIA_T2CL] + (via2[VIA_T2CH] << 8);
      via2tbu = clk;
      maincpu_set_alarm(A_VIA2T2, via2tb);
    }
}

void via2_prevent_clk_overflow(void)
{
    via2tau -= PREVENT_CLK_OVERFLOW_SUB;
    via2tbu -= PREVENT_CLK_OVERFLOW_SUB;
}



