/******************************************************************************

 @file  cc13x0lp_oad.cmd

 @brief Linker configuration file for MAC on CC13x0 LaunchPad

 Group: WCS LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2018-2023, Texas Instruments Incorporated
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
--retain=g_pfnVectors
/* Override default entry point.                                             */
--entry_point ResetISR
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

/* The starting address of the application.  Normally the interrupt vectors  */
/* must be located at the beginning of the application.                      */

#ifdef TIRTOS_IN_ROM
#define FLASH_APP_BASE          0x000014F0
#else
#define FLASH_APP_BASE          0x00001000
#endif
#define FLASH_OAD_IMG_HDR_SIZE  0x10

#ifdef TIRTOS_IN_ROM
#define FLASH_CONST_BASE        0x00001000
#endif

#ifdef ONE_PAGE_NV
#define FLASH_NV_BASE           0x1E000
#define FLASH_NV_SIZE           0x1000
#define FLASH_APP_SIZE          FLASH_NV_BASE - FLASH_APP_BASE
#else
#define FLASH_NV_BASE           0x1D000
#define FLASH_NV_SIZE           0x2000
#define FLASH_APP_SIZE          FLASH_NV_BASE - FLASH_APP_BASE
#endif

#define FLASH_LAST_BASE         0x1F000
#define FLASH_LAST_SIZE         0x1000
#define RAM_BASE                0x20000000
#define RAM_SIZE                0x5000


/* System memory map */

MEMORY
{
#ifdef TIRTOS_IN_ROM
    /* Const data from TIRTOS in ROM */
    FLASH_CONST (RX) : origin = FLASH_CONST_BASE, length = FLASH_APP_BASE - FLASH_CONST_BASE
#endif
    /* Image header included for compatability with the BLE OAD */
    IMAGE_HEADER (RX) : origin = FLASH_APP_BASE, length = FLASH_OAD_IMG_HDR_SIZE
    /* Application stored in and executes from internal flash */
    FLASH (RX) : origin = FLASH_APP_BASE + FLASH_OAD_IMG_HDR_SIZE, length = FLASH_APP_SIZE - FLASH_OAD_IMG_HDR_SIZE
    /* NV space */
    FLASH_NV (RW): origin = FLASH_NV_BASE, length = FLASH_NV_SIZE
    /* Application uses internal RAM for data */
    SRAM (RWX) : origin = RAM_BASE, length = RAM_SIZE
}

/* Section allocation in memory */

SECTIONS
{
    .imgHdr         :   > IMAGE_HEADER
    .intvecs        :   > FLASH
    .text           :   > FLASH

    config_const { mac_user_config.obj(.const) } > FLASH

#ifdef TIRTOS_IN_ROM
    .const          :   > FLASH_CONST | FLASH
    .constdata      :   > FLASH_CONST | FLASH
#else
    .const          :   > FLASH
    .constdata      :   > FLASH
#endif

    .rodata         :   > FLASH
    .cinit          :   > FLASH
    .pinit          :   > FLASH
    .init_array     :   > FLASH
    .emb_text       :   > FLASH
    .ccfg           :   > FLASH

    /* make a hole in the NV section and fill it with 0xFF */
/*    .sect {
        . = FLASH_NV_SIZE;
    } fill = 0xFFFFFFFF > FLASH_NV
*/
    GROUP > SRAM
    {
        .data
        .bss
        .vtable
        .vtable_ram
         vtable_ram
        .sysmem
        .nonretenvar
    } LOAD_END(heapStart)

    .stack          :   >  SRAM (HIGH) LOAD_START(heapEnd)
}

/* Create global constant that points to top of stack */
/* CCS: Change stack size under Project Properties    */
__STACK_TOP = __stack + __STACK_SIZE;
