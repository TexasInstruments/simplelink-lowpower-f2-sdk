/******************************************************************************

 @file  cc26x3.cmd

 @brief Linker configuration file for sensor/collector on cc13x2/cc26x2 devices

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

--stack_size=1024   /* C stack is also used for ISR stack */
/* Retain interrupt vector table variable                                    */
--retain=g_pfnVectors
/* Allow main() to take args                                                 */
--args 0x8
/* Suppress warnings and errors:                                             */
/* - 10063: Warning about entry point not being _c_int00                     */
/* - 16011, 16012: 8-byte alignment errors. Observed when linking in object  */
/*   files compiled using Keil (ARM compiler)                                */
--diag_suppress=10063,16011,16012

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

/* The starting address of the application.  Normally the interrupt vectors  */
/* must be located at the beginning of the application.                      */
#define FLASH_BASE				0x00000

#define CCFG_BASE               0x50000000
#define RAM_BASE                0x20000000

#define FLASH_SIZE				0x100000

#define CCFG_SIZE               0x800
#define RAM_SIZE                0x20000

/******************************************************************************
 System memory map
 *****************************************************************************/
MEMORY
{
    /* Application stored in and executes from internal flash */
    FLASH1 (RX) : origin = 0x00000, length = 0x1C000
    FLASH2 (RX) : origin = 0x24000, length = 0x38000
    FLASH3 (RX) : origin = 0x64000, length = 0x38000
    FLASH4 (RX) : origin = 0xA4000, length = 0x38000
    FLASH5 (RX) : origin = 0xE4000, length = 0x1C000


    /* Last flash page */
    CCFG (RX) : origin = CCFG_BASE, length = CCFG_SIZE
    /* Application uses internal RAM for data */
    SRAM (RWX) : origin = RAM_BASE, length = RAM_SIZE
}

/******************************************************************************
 Section allocation in memory
 *****************************************************************************/
SECTIONS
{
    .resetVecs      :   > FLASH_BASE
/*
    flash_last_text
    {
        mac_user_config.obj(.text)
        macTask.obj(.text)
        hmac_rom_init.obj (.const)
        osal_port.obj (.text)
    } > CCFG
*/
    .text           :   > FLASH1 | FLASH2 | FLASH3 | FLASH4 | FLASH5
    .const          :   > FLASH1 | FLASH2 | FLASH3 | FLASH4 | FLASH5
    .constdata      :   > FLASH1 | FLASH2 | FLASH3 | FLASH4 | FLASH5
    .rodata         :   > FLASH1 | FLASH2 | FLASH3 | FLASH4 | FLASH5
    .cinit          :   > FLASH1 | FLASH2 | FLASH3 | FLASH4 | FLASH5
    .pinit          :   > FLASH1 | FLASH2 | FLASH3 | FLASH4 | FLASH5
    .init_array     :   > FLASH1 | FLASH2 | FLASH3 | FLASH4 | FLASH5
    .emb_text       :   > FLASH1 | FLASH2 | FLASH3 | FLASH4 | FLASH5
    .ccfg           :   > CCFG

    GROUP > SRAM
    {
        .data
        .bss
        .ramVecs
        .sysmem
        .nonretenvar
    } LOAD_END(heapStart)

    .stack          :   >  SRAM (HIGH) LOAD_START(heapEnd)
}

/* Create global constant that points to top of stack */
/* CCS: Change stack size under Project Properties    */
/*__STACK_TOP = __stack + __STACK_SIZE;*/

--symbol_map __TI_STACK_SIZE=__STACK_SIZE
--symbol_map __TI_STACK_BASE=__stack

-u_c_int00
--retain "*(.resetVecs)"
--retain "*(.ramVecs)"

SECTIONS
{
    .resetVecs: load > 0
    .ramVecs: load > 0x20000000, type = NOLOAD
}
