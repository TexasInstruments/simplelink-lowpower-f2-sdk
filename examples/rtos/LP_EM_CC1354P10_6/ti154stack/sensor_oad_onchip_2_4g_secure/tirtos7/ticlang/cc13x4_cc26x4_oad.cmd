/******************************************************************************

 @file  cc13x4_cc26x4_oad.cmd

 @brief Linker configuration file for sensor/collector on cc13x2/cc26x2 devices

 Group: WCS LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2016-2022, Texas Instruments Incorporated
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
-u_c_int00
--retain "*(.resetVecs)"
--retain "*(.vecs)"
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

/* The starting address of the application. Normally the interrupt vectors */
/* must be located at the beginning of the application. */

#define MCUBOOT
#define PRIMARY_SLOT

#define MCUBOOT_HEAD_SIZE 		   		            0x20
/*Memory resevered for the trailer at the end of a slot */
#define MCUBOOT_RESERVED_TRAILER_SIZE               0x2000
#define OAD_HDR_SIZE 			   		            0xA8
/* Entry size is padded for tiRTOS7 to keep size consistent between freeRTOS and tiRTOS */
#define ENTRY_SIZE                                  0xD8

#if defined(PRIMARY_SLOT)
#define PRIMARY_SLOT_BASE                           0x06000
#define SLOT_SIZE                                   0x7b000
#define FLASH_BASE                                  PRIMARY_SLOT_BASE + MCUBOOT_HEAD_SIZE + OAD_HDR_SIZE + ENTRY_SIZE
#define FLASH_SIZE                                  SLOT_SIZE - MCUBOOT_HEAD_SIZE - OAD_HDR_SIZE - ENTRY_SIZE - MCUBOOT_RESERVED_TRAILER_SIZE

#define OAD_HDR_START                               PRIMARY_SLOT_BASE + MCUBOOT_HEAD_SIZE + ENTRY_SIZE /*Start OAD header after MCUBoot header */


#endif //defined(PRIMARY_SLOT)

/* Common addresses */
#define OAD_HDR_END                                 (OAD_HDR_START + OAD_HDR_SIZE - 1)
#define ENTRY_START                                 (PRIMARY_SLOT_BASE + MCUBOOT_HEAD_SIZE)
#define FLASH_START                                 (OAD_HDR_END + 1)
#define SHARED_NV_START                             0xFC000
#define SHARED_NV_SIZE                              0x4000

//define variables for use in application code
_MCUBOOT_HEAD_SIZE = MCUBOOT_HEAD_SIZE;
_TRAILER_SIZE = MCUBOOT_RESERVED_TRAILER_SIZE;
_OAD_HDR_SIZE = OAD_HDR_SIZE;
_ENTRY_SIZE = ENTRY_SIZE;
_PRIMARY_SLOT_BASE = PRIMARY_SLOT_BASE;
_SLOT_SIZE = SLOT_SIZE;
_OAD_HDR_START = OAD_HDR_START;
_IMG_END_ADDR = PRIMARY_SLOT_BASE + SLOT_SIZE - MCUBOOT_RESERVED_TRAILER_SIZE;

#define CCFG_BASE 	                                0x50000000
#define RAM_BASE 	                                0x20000000
#define CCFG_SIZE 		                                 0x800
#define RAM_SIZE 	                                   0x40000
#define PAGE_SIZE 		                                0x0800

/******************************************************************************
System memory map
*****************************************************************************/
MEMORY
{
	FLASH_IMG_HDR (RX) : origin = OAD_HDR_START, length = OAD_HDR_SIZE

    ENTRY (RX) : origin = ENTRY_START, length = ENTRY_SIZE

    /* Application stored in and executes from internal flash */
    FLASH (RX) : origin = FLASH_START, length = FLASH_SIZE

    /* Network and OAD info stored here so it's not lost between OADs */
    SHARED_NV (RX) : origin = SHARED_NV_START, length = SHARED_NV_SIZE

    /* Last flash page */
    //CCFG (RX) : origin = CCFG_BASE, length = CCFG_SIZE
    /* Application uses internal RAM for data */
    SRAM (RWX) : origin = RAM_BASE, length = RAM_SIZE
}

/******************************************************************************
Section allocation in memory
*****************************************************************************/
SECTIONS
{
    GROUP > FLASH_IMG_HDR
    {
        .image_header
    }

    GROUP > ENTRY
    {
        .resetVecs LOAD_START(prgEntryAddr)
        .intvecs
    }
}

SECTIONS
{

    .vecs           : load > 0x20000000, type = NOLOAD
    /*
    Don't add the CCFG, that's handled by the MCUboot Bootloader
    */
    .text : > FLASH
    .const : > FLASH
    .constdata : > FLASH
    .rodata : > FLASH
    .cinit : > FLASH
    .pinit : > FLASH
    .init_array : > FLASH
    .emb_text : > FLASH

    .cinit          :   > FLASH LOAD_END(flashEndAddr)

  	GROUP > SRAM
    {
        .data LOAD_START(ramStartHere)
        .bss
        .noinit
        .vtable
        .vtable_ram
         vtable_ram
        .sysmem
        .nonretenvar
    } LOAD_END(heapStart)

    .stack : > SRAM (HIGH) LOAD_START(heapEnd)
}

/* Create global constant that points to top of stack */
/* CCS: Change stack size under Project Properties */
/*__STACK_TOP = __stack + __STACK_SIZE;*/

--symbol_map __TI_STACK_SIZE=__STACK_SIZE
--symbol_map __TI_STACK_BASE=__stack
