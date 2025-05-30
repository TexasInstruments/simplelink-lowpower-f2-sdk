/******************************************************************************

 @file  cc13x2x7_cc26x2x7_oad_onchip.cmd

 @brief Linker configuration file for 15.4 OAD on cc13x2x7/cc26x2x7 devices

 Group: WCS LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2018-2025, Texas Instruments Incorporated
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
#ifdef FREERTOS
--entry_point resetISR
#else
--entry_point ResetISR
#endif
/* Allow main() to take args                                                 */
--args 0x8
#ifdef FREERTOS
--stack_size=512
--heap_size=0          /* minimize heap since we are using heap_4.c */
/* Retain interrupt vector table variable                                    */
--retain=resetVectors
#endif
/* Suppress warnings and errors:                                             */
/* - 10063: Warning about entry point not being _c_int00                     */
/* - 16011, 16012: 8-byte alignment errors. Observed when linking in object  */
/*   files compiled using Keil (ARM compiler)                                */
/* - 10068: Warning about no matching section when logs are disabled         */
--diag_suppress=10063,16011,16012,10068

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
#define FLASH_BASE                 0x00000000
#define RAM_BASE                   0x20000000
#define ROM_BASE                   0x10000000

#define FLASH_SIZE                 0x000B0000
#define RAM_SIZE                   0x00024000
#define ROM_SIZE                   0x00040000

#define PAGE_SIZE                  0x2000

/******************************************************************************
 Memory Definitions
 *****************************************************************************/
/* The last flash page is reserved for the BIM and CCFG */
#define NUM_RESERVED_FLASH_PAGES   1
#define RESERVED_FLASH_SIZE        (NUM_RESERVED_FLASH_PAGES * PAGE_SIZE)

#define OAD_HDR_SIZE               0xA8
#ifdef FREERTOS
#define ENTRY_SIZE                 0xD8
#else
#define ENTRY_SIZE                 0x40
#endif

/* The persistent application's starting address. Use this address when      */
/* flashing the .bin file if using BIM persistent image authentication.      */
/* Note that if the start address is changed, you must also change the reset */
/* vector address (m3Hwi.resetVectorAddress) in the TI-RTOS .cfg file.       */
#define IMG_A_FLASH_START          0x00086000

/* Image specific addresses */
#ifdef OAD_IMG_A
#ifdef FREERTOS
	#define ENTRY_START			   IMG_A_FLASH_START + 0x00000100
#endif
    #define OAD_HDR_START          IMG_A_FLASH_START
    #define FLASH_END              (FLASH_BASE + FLASH_SIZE - RESERVED_FLASH_SIZE - 1)
#else
#ifdef FREERTOS
	#define ENTRY_START			   0x00000100
#endif
    #define OAD_HDR_START          FLASH_BASE
    #define FLASH_END              (FLASH_BASE + IMG_A_FLASH_START - 1)
#endif

/* Common addresses */
#define OAD_HDR_END                (OAD_HDR_START + OAD_HDR_SIZE - 1)

#ifndef FREERTOS
#define ENTRY_START                (OAD_HDR_END + 1)
#endif
#define ENTRY_END                  (ENTRY_START + ENTRY_SIZE - 1)

#define FLASH_START                (ENTRY_END + 1)

/******************************************************************************
 System memory map
 *****************************************************************************/
MEMORY
{
    FLASH_IMG_HDR (RX) : origin = OAD_HDR_START, length = OAD_HDR_SIZE

    ENTRY (RX) : origin = ENTRY_START, length = ENTRY_SIZE

    /* Application stored in and executes from internal flash */
    FLASH (RX) : origin = FLASH_START, length = (FLASH_END - FLASH_START + 1)

    /* Application uses internal RAM for data */
    SRAM (RWX) : origin = RAM_BASE, length = RAM_SIZE

    /* Explicitly placed off target for the storage of logging data.
     * The data placed here is NOT loaded onto the target device.
     * This is part of 1 GB of external memory from 0x60000000 - 0x9FFFFFFF.
     * ARM memory map can be found here:
     * https://developer.arm.com/documentation/ddi0337/e/memory-map/about-the-memory-map
     */
    LOG_DATA (R) : origin = 0x90000000, length = 0x40000        /* 256 KB */
    LOG_PTR  (R) : origin = 0x94000008, length = 0x40000        /* 256 KB */
}

/******************************************************************************
 Section allocation in memory
 *****************************************************************************/
SECTIONS
{
    GROUP > FLASH_IMG_HDR
    {
        .image_header align PAGE_SIZE
    }

    GROUP > ENTRY
    {
        .resetVecs LOAD_START(prgEntryAddr)
        EntrySection
    }

    GROUP > FLASH
    {
        .text
        .const
        .constdata
        .rodata
        .emb_text
        .init_array
        .pinit
    }

    .cinit          :   > FLASH LOAD_END(flashEndAddr)

    .bss            :   >> SRAM
    GROUP > SRAM
    {
        .data LOAD_START(ramStartHere)
        .noinit
        .ramVecs
        .sysmem
        .nonretenvar
    } LOAD_END(heapStart)

    .stack          :   >  SRAM (HIGH) LOAD_START(heapEnd)
    .log_data       :   > LOG_DATA, type = COPY
    .log_ptr        : { *(.log_ptr*) } > LOG_PTR align 4, type = COPY
}

/* Create global constant that points to top of stack */
/* CCS: Change stack size under Project Properties    */
__STACK_TOP = __stack + __STACK_SIZE;
