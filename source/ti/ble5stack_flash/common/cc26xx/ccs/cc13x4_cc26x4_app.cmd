/******************************************************************************

 @file  cc13x4_cc26x4_app.cmd

 @brief CC26X2R1F and CC13X2R1F3 linker configuration file for TI-RTOS with Code
        Composer Studio.

        Imported Symbols
        Note: Linker defines are located in the CCS IDE project by placing them
        in
        Properties->Build->Linker->Advanced Options->Command File Preprocessing.

        CACHE_AS_RAM:       Disable system cache to be used as GPRAM for
                            additional volatile memory storage.
        FLASH_ROM_BUILD:	If defined, it should be set to 1 or 2 to indicate
                            the ROM version of the device being used. When using
                            Flash-only configuration, this symbol should not be
                            defined.
        ICALL_RAM0_START:   RAM start of BLE stack.
        ICALL_STACK0_START: Flash start of BLE stack.
        PAGE_AlIGN:         Align BLE stack boundary to a page boundary.
                            Aligns to Flash word boundary by default.

 Group: WCS, BTS
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2017-2025, Texas Instruments Incorporated
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

/*******************************************************************************
 * CCS Linker configuration
 */
#if defined(FREERTOS)
--entry_point=resetISR
/* Retain interrupt vector table variable                                    */
--retain "*(.resetVecs)"
#else
/* Retain interrupt vector table variable                                    */
--retain=g_pfnVectors
#endif //defined(FREERTOS)
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
#if defined(FREERTOS)
--heap_size=0
--stack_size=2048
#else
--stack_size=1024
#endif //defined(FREERTOS)

/* --library=rtsv7M3_T_le_eabi.lib                                           */

/* The starting address of the application.  Normally the interrupt vectors  */
/* must be located at the beginning of the application. Flash is 128KB, with */
/* sector length of 4KB                                                      */
/*******************************************************************************
 * Memory Sizes
 */
#define FLASH_BASE   0x00000000
#define GPRAM_BASE   0x11000000
#define RAM_BASE     0x20000000
#define CCFG_BASE    0x50000000
#define CCFG_SIZE    0x00000800

#define FLASH_SIZE   0x00100000
#define GPRAM_SIZE   0x00002000
#define RAM_SIZE     0x00040000

#define RTOS_RAM_SIZE           0x0000012C
#define RESERVED_RAM_SIZE_ROM_1 0x00000B08
#define RESERVED_RAM_SIZE_ROM_2 0x00000EB3

#define NVS_SIZE        0x4000
#define NVS_BASE        (FLASH_SIZE - NVS_SIZE)

#if defined(OAD_APP_OFFCHIP) || defined(OAD_APP_ONCHIP) || defined(OAD_PERSISTENT)
#define MCU_HDR_SIZE    0x100
#define MCUBOOT_BASE    FLASH_BASE
#define MCUBOOT_SIZE    0x6000
#define APP_HDR_BASE    APP_HDR_ADDR
#endif //defined(OAD_APP_OFFCHIP) || defined(OAD_APP_ONCHIP) || defined(OAD_PERSISTENT)

#if defined(OAD_APP_ONCHIP)|| defined(OAD_PERSISTENT)
#define PERSISTENT_HDR_BASE 0x6000
#define PERSISTENT_BASE     (PERSISTENT_HDR_BASE + MCU_HDR_SIZE)
#define APP_BASE            (APP_HDR_BASE + MCU_HDR_SIZE)
#define PERSISTENT_SIZE     (APP_HDR_BASE - PERSISTENT_BASE)
#define APP_SIZE            (FLASH_SIZE - APP_BASE - NVS_SIZE)
#endif //defined(OAD_APP_ONCHIP)|| defined(OAD_PERSISTENT)

#ifdef OAD_APP_OFFCHIP
#define APP_BASE        (APP_HDR_BASE + MCU_HDR_SIZE)
#define APP_SIZE        (FLASH_SIZE - APP_BASE - NVS_SIZE)
#endif //OAD_APP_OFFCHIP

/*******************************************************************************
 * Memory Definitions
 ******************************************************************************/

/*******************************************************************************
 * RAM
 */
#define RAM_START      RAM_BASE
#ifdef ICALL_RAM0_START
  #define RAM_END      (ICALL_RAM0_START - 1)
#else
  #define RAM_END      (RAM_BASE + RAM_SIZE - 1)
#endif /* ICALL_RAM0_START */

/* For ROM 2 devices, the following section needs to be allocated and reserved */
#define RTOS_RAM_START RAM_BASE
#define RTOS_RAM_END   (RAM_BASE + RTOS_RAM_SIZE - 1)

/*******************************************************************************
 * Flash
 */

#define FLASH_START                FLASH_BASE
#define WORD_SIZE                  4

#define PAGE_SIZE                  0x800

#ifdef PAGE_ALIGN
  #define FLASH_MEM_ALIGN          PAGE_SIZE
#else
  #define FLASH_MEM_ALIGN          WORD_SIZE
#endif /* PAGE_ALIGN */

#define PAGE_MASK                  0xFFFFE000

/* The last Flash page is reserved for the application. */
#define NUM_RESERVED_FLASH_PAGES   1
#define RESERVED_FLASH_SIZE        (NUM_RESERVED_FLASH_PAGES * PAGE_SIZE)

/* Check if page alingment with the Stack image is required.  If so, do not link
 * into a page shared by the Stack.
 */
#ifdef ICALL_STACK0_START
  #ifdef PAGE_ALIGN
    #define ADJ_ICALL_STACK0_START (ICALL_STACK0_START * PAGE_MASK)
  #else
    #define ADJ_ICALL_STACK0_START ICALL_STACK0_START
  #endif /* PAGE_ALIGN */

  #define FLASH_END                (ADJ_ICALL_STACK0_START - 1)
#else
  #define FLASH_END                (FLASH_BASE + FLASH_SIZE - RESERVED_FLASH_SIZE - 1)
#endif /* ICALL_STACK0_START */

#define FLASH_LAST_PAGE_START      (FLASH_SIZE - PAGE_SIZE)

/*******************************************************************************
 * Stack
 */

/* Create global constant that points to top of stack */
/* CCS: Change stack size under Project Properties    */
__STACK_TOP = __stack + __STACK_SIZE;

/*******************************************************************************
 * GPRAM
 */

#ifdef CACHE_AS_RAM
  #define GPRAM_START GPRAM_BASE
  #define GPRAM_END   (GPRAM_START + GPRAM_SIZE - 1)
#endif /* CACHE_AS_RAM */

/*******************************************************************************
 * ROV
 * These symbols are used by ROV2 to extend the valid memory regions on device.
 * Without these defines, ROV will encounter a Java exception when using an
 * autosized heap. This is a posted workaround for a known limitation of
 * RTSC/rta. See: https://bugs.eclipse.org/bugs/show_bug.cgi?id=487894
 *
 * Note: these do not affect placement in RAM or FLASH, they are only used
 * by ROV2, see the BLE Stack User's Guide for more info on a workaround
 * for ROV Classic
 *
 */
__UNUSED_SRAM_start__ = RAM_BASE;
__UNUSED_SRAM_end__ = RAM_BASE + RAM_SIZE;

__UNUSED_FLASH_start__ = FLASH_BASE;
__UNUSED_FLASH_end__ = FLASH_BASE + FLASH_SIZE;

/*******************************************************************************
 * Main arguments
 */

/* Allow main() to take args */
/* --args 0x8 */

/*******************************************************************************
 * System Memory Map
 ******************************************************************************/
MEMORY
{
  /* EDITOR'S NOTE:
   * the FLASH and SRAM lengths can be changed by defining
   * ICALL_STACK0_START or ICALL_RAM0_START in
   * Properties->ARM Linker->Advanced Options->Command File Preprocessing.
   */

#if defined(OAD_APP_OFFCHIP)|| defined(OAD_APP_ONCHIP) || defined(OAD_PERSISTENT)

    MCUBOOT_SLOT(RX)       : origin = MCUBOOT_BASE        ,length = MCUBOOT_SIZE
    APP_HDR_SLOT(RX)       : origin = APP_HDR_BASE        ,length = MCU_HDR_SIZE
    APP_SLOT (RX)          : origin = APP_BASE            ,length = APP_SIZE

#if defined(OAD_APP_ONCHIP) || defined(OAD_PERSISTENT)

    PESISTENT_HDR_SLOT(RX) : origin = PERSISTENT_HDR_BASE ,length = MCU_HDR_SIZE
    PERSITENT_SLOT(RX)     : origin = PERSISTENT_BASE     ,length = PERSISTENT_SIZE

#endif //defined(OAD_APP_ONCHIP) || defined(OAD_PERSISTENT)

#else //Without mcuboot

    FLASH (RX)   : origin = 0x0,      length = (FLASH_SIZE - NVS_SIZE)

#endif //defined(OAD_APP_OFFCHIP)|| defined(OAD_APP_ONCHIP) || defined(OAD_PERSISTENT)

  NVS_SLOT(RX) : origin = NVS_BASE ,length = NVS_SIZE
  /* Application uses internal RAM for data */
  SRAM (RWX) : origin = RAM_START, length = (RAM_END - RAM_START + 1)

  #ifdef CACHE_AS_RAM
      GPRAM(RWX) : origin = GPRAM_START, length = GPRAM_SIZE
  #endif /* CACHE_AS_RAM */
  CCFG (RW) : origin = CCFG_BASE, length = CCFG_SIZE
    /* Explicitly placed off target for the storage of logging data.
     * The ARM memory map allocates 1 GB of external memory from 0x60000000 - 0x9FFFFFFF.
     * Unlikely that all of this will be used, so we are using the upper parts of the region.
     * ARM memory map: https://developer.arm.com/documentation/ddi0337/e/memory-map/about-the-memory-map*/
}
/*******************************************************************************
 * Section Allocation in Memory
 ******************************************************************************/
SECTIONS
{
#if defined(OAD_APP_OFFCHIP) || defined(OAD_APP_ONCHIP) || defined(OAD_PERSISTENT)

	.primary_hdr    :   > APP_HDR_SLOT, type = NOLOAD

#if defined(OAD_APP_OFFCHIP) || defined(OAD_APP_ONCHIP)

    .resetVecs      :   > APP_BASE
    .text           :   > APP_SLOT
    .const          :   > APP_SLOT
    .constdata      :   > APP_SLOT
    .rodata         :   > APP_SLOT
    .binit          :   > APP_SLOT
    .cinit          :   > APP_SLOT
    .pinit          :   > APP_SLOT
    .init_array     :   > APP_SLOT
    .emb_text       :   > APP_SLOT
    .log_data       :   > APP_SLOT

#else

    .resetVecs      :   > PERSISTENT_BASE
    .text           :   > PERSITENT_SLOT
    .const          :   > PERSITENT_SLOT
    .constdata      :   > PERSITENT_SLOT
    .rodata         :   > PERSITENT_SLOT
    .binit          :   > PERSITENT_SLOT
    .cinit          :   > PERSITENT_SLOT
    .pinit          :   > PERSITENT_SLOT
    .init_array     :   > PERSITENT_SLOT
    .emb_text       :   > PERSITENT_SLOT
    .log_data       :   > PERSITENT_SLOT

#endif //defined(OAD_APP_OFFCHIP) || defined(OAD_APP_ONCHIP)

#else

    .resetVecs      :   load > 0
    .intvecs        :   > FLASH_BASE
    .text           :   > FLASH
    .TI.ramfunc     :   {} load=FLASH, run=SRAM, table(BINIT)
    .const          :   > FLASH
    .constdata      :   > FLASH
    .rodata         :   > FLASH
    .binit          :   > FLASH
    .cinit          :   > FLASH
    .pinit          :   > FLASH
    .init_array     :   > FLASH
    .emb_text       :   > FLASH
    .log_data       :   > FLASH

#endif //defined(OAD_APP_OFFCHIP) || defined(OAD_APP_ONCHIP) || defined(OAD_PERSISTENT)

    .ccfg           :   > CCFG
    .vecs: load > 0x20000000, type = NOLOAD

  GROUP > SRAM
  {
    .data
    #ifndef CACHE_AS_RAM
    .bss
    #endif /* CACHE_AS_RAM */
    .vtable
    .vtable_ram
    vtable_ram
    .sysmem
    .nonretenvar
    /*This keeps ll.o objects out of GPRAM, if no ll.o would be placed here
      the warning #10068 is supressed.*/
    #ifdef CACHE_AS_RAM
    ll_bss
    {
      --library=*ll_*.a<ll.o> (.bss)
      --library=*ll_*.a<ll_ae.o> (.bss)
    }
    #endif /* CACHE_AS_RAM */
    .ramVecs
  } LOAD_END(heapStart)

  .stack            :   >  SRAM (HIGH) LOAD_START(heapEnd)

  #ifdef CACHE_AS_RAM

  .bss :
  {
    *(.bss)
  } > GPRAM
  #endif /* CACHE_AS_RAM */
}

--symbol_map __TI_STACK_SIZE=__STACK_SIZE
--symbol_map __TI_STACK_BASE=__stack

-u_c_int00
--retain "*(.resetVecs)"
--retain "*(.vecs)"
