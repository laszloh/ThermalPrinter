/*-------------------------------------------------------------------------
   serial.c - this module implements serial interrupt handler and IO
              routinwes using two 256 byte cyclic buffers. Bit variables
              can be used as flags for real-time kernel tasks

   Copyright (C) 1996, Dmitry S. Obukhov <dmitry.obukhov AT gmail.com>

   This library is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by the
   Free Software Foundation; either version 2, or (at your option) any
   later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this library; see the file COPYING. If not, write to the
   Free Software Foundation, 51 Franklin Street, Fifth Floor, Boston,
   MA 02110-1301, USA.

   As a special exception, if you link this library with other files,
   some of which are compiled with SDCC, to produce an executable,
   this library does not by itself cause the resulting executable to
   be covered by the GNU General Public License. This exception does
   not however invalidate any other reasons why the executable file
   might be covered by the GNU General Public License.
-------------------------------------------------------------------------*/

/* This module contains definition of I8051 registers */
#include <C8051F320.h>
#include <stdint.h>

#include "serial.h"

#define SER_BUF_SIZE 0x40

static volatile uint8_t stx_index_in, srx_index_in, stx_index_out, srx_index_out;
static volatile char __xdata stx_buffer[SER_BUF_SIZE];
static volatile char __xdata srx_buffer[SER_BUF_SIZE];

static volatile __bit work_flag_byte_arrived;
static volatile __bit work_flag_buffer_transfered;
static volatile __bit tx_serial_buffer_empty;
static volatile __bit rx_serial_buffer_empty;
static volatile __bit rx_serial_buffer_overflow;

void serial_init(void)
{
    TMOD = 0x20;
    CKCON = 0x08;
    TH1 = 0x98;
    TR1 = 1;

    SCON0 = 0x30;

    RI = 0;
    TI = 0;

    stx_index_in = srx_index_in = stx_index_out = srx_index_out = 0;
    rx_serial_buffer_empty = tx_serial_buffer_empty = 1;
    work_flag_byte_arrived = work_flag_buffer_transfered = rx_serial_buffer_overflow = 0;
    ES = 1;
}

void serial_interrupt_handler(void) __interrupt 4 __using 1
{
    ES = 0;
    if (RI)
    {
        RI = 0;
        srx_buffer[srx_index_in++] = SBUF;
        if (srx_index_in >= SER_BUF_SIZE)
        {
            srx_index_in = SER_BUF_SIZE;
            rx_serial_buffer_overflow = 1;
        }
        work_flag_byte_arrived = 1;
        rx_serial_buffer_empty = 0;
    }
    if (TI)
    {
        TI = 0;
        if (stx_index_out == stx_index_in)
        {
            tx_serial_buffer_empty = 1;
            work_flag_buffer_transfered = 1;
        }
        else
            SBUF = stx_buffer[stx_index_out++];
    }
    ES = 1;
}

/* Next two functions are interface */

void serial_putc(unsigned char c)
{
    stx_buffer[stx_index_in++] = c;
    ES = 0;
    if (tx_serial_buffer_empty)
    {
        tx_serial_buffer_empty = 0;
        TI = 1;
    }
    ES = 1;
}

unsigned char serial_getc(void)
{
    unsigned char tmp = srx_buffer[srx_index_out++];
    ES = 0;
    if (srx_index_out == srx_index_in)
        rx_serial_buffer_empty = 1;
    ES = 1;
    return tmp;
}

bool serial_avaliable()
{
    return rx_serial_buffer_empty;
}

bool serial_tx_empty()
{
    return tx_serial_buffer_empty;
}
