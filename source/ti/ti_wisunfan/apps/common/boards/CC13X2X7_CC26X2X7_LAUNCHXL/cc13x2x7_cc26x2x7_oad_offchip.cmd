/******************************************************************************

 @file  cc13x2x7_cc26x2x7_oad_offchip.cmd

 @brief Linker configuration file

 Group: WCS LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2016-2025, Texas Instruments Incorporated
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions
 are met:

 *  Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.

 *  Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.

 *  Neither the name of Texas Instruments Incorporated nor the names of
    its contributors may be used to endorse or promote products derived
    from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 ******************************************************************************
 
 
 *****************************************************************************/

/* Retain interrupt vector table variable                                    */
--retain "*(.resetVecs)"
/* Override default entry point.                                             */
#ifdef FREERTOS
--entry_point resetISR
--heap_size=0          /* minimize heap since we are using heap_4.c */
#else
--entry_point ResetISR
#endif
/* Allow main() to take args                                                 */
--args 0x8
--stack_size=1600
/* Suppress warnings and errors:                                             */
/* - 10063: Warning about entry point not being _c_int00                     */
/* - 16011, 16012: 8-byte alignment errors. Observed when linking in object  */
/*   files compiled using Keil (ARM compiler)                                */
--diag_suppress=10063,16011,16012,10247-D

/* The following command line options are set as part of the CCS project.    */
/* If you are building using the command line, or for some reason want to    */
/* define them here, you can uncomment and modify these lines as needed.     */
/* If you are using CCS for building, it is probably better to make any such */
/* modifications in your CCS project and leave this file alone.              */
/*                                                                           */
/* --heap_size=0                                                             */
/* --stack_size=256                                                          */
/* --library=rtsv7M3_T_le_eabi.lib                                           */

/******************************************************************************
 Memory Sizes
 *****************************************************************************/
#define MCUBOOT_HEAD_SIZE 0x80
#define ENTRY_SIZE 0x100
#define MCUBOOT_RESERVED_TRAILER_SIZE   0x2000

#define PRIMARY_SLOT_BASE 0x0
#define FLASH_BASE PRIMARY_SLOT_BASE + MCUBOOT_HEAD_SIZE + ENTRY_SIZE
#define FLASH_SIZE 0x56000 - MCUBOOT_HEAD_SIZE - ENTRY_SIZE - MCUBOOT_RESERVED_TRAILER_SIZE

#define ENTRY_START (PRIMARY_SLOT_BASE + MCUBOOT_HEAD_SIZE)
#define ENTRY_END   (ENTRY_START + ENTRY_SIZE - 1)

#define RAM_BASE 0x20000000
#define RAM_SIZE 0x24000

#define NVS_BASE 0xA8000
#define NVS_SIZE 0x4000

/******************************************************************************
 System memory map
 *****************************************************************************/
MEMORY
{
    /* Application stored in and executes from internal flash */
    FLASH (RX) : origin = FLASH_BASE, length = FLASH_SIZE

    ENTRY (RX) : origin = ENTRY_START, length = ENTRY_SIZE

    /* NVS area (unused in linker file, reserved for shared NV) */
    NVS (RX) : origin = NVS_BASE, length = NVS_SIZE

    /* Application uses internal RAM for data */
    SRAM (RWX) : origin = RAM_BASE, length = RAM_SIZE
}

/******************************************************************************
 Section allocation in memory
 *****************************************************************************/
SECTIONS
{
    .resetVecs      :   > ENTRY
    .text           :   > FLASH
    .const          :   > FLASH
    .constdata      :   > FLASH
    .rodata         :   > FLASH
    .cinit          :   > FLASH
    .pinit          :   > FLASH
    .init_array     :   > FLASH
    .emb_text       :   > FLASH

#ifdef FREERTOS
    .ramVecs        :   > RAM_BASE, type=NOINIT
    .data           :   > SRAM
    .bss            :   > SRAM
    .sysmem         :   > SRAM
    .stack          :   > SRAM (HIGH)
    .nonretenvar    :   > SRAM
#else
    .bss            :   >> SRAM
    GROUP > SRAM
    {
        .data
        .ramVecs
        .sysmem
        .nonretenvar
    } LOAD_END(heapStart)

    .stack          :   >  SRAM (HIGH) LOAD_START(heapEnd)
#endif
}

/* Create global constant that points to top of stack */
/* CCS: Change stack size under Project Properties    */
__STACK_TOP = __stack + __STACK_SIZE;
__PRIMARY_SLOT_BASE = PRIMARY_SLOT_BASE;

#ifdef TIRTOS7
--symbol_map __TI_STACK_SIZE=__STACK_SIZE
--symbol_map __TI_STACK_BASE=__stack

-u_c_int00
#endif
