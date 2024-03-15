/*
 * Copyright (c) 2021-2022, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

STACKSIZE = 1024;
HEAPSIZE = 8196; /* Size of heap buffer used by HeapMem */

MEMORY
{
    FLASH (RX)      : ORIGIN = 0x00000000, LENGTH = 0x5800
    /*
     * Customer Configuration Area and Bootloader Backdoor configuration in
     * flash, up to 512 bytes
     */
    FLASH_CCFG (RX) : ORIGIN = 0x4E020000, LENGTH = 0x00000800
    SRAM (RWX)      : ORIGIN = 0x20000000, LENGTH = 0x00007000
    /* Explicitly placed off target for the storage of logging data.
     * The data placed here is NOT loaded onto the target device.
     * This is part of 1 GB of external memory from 0x60000000 - 0x9FFFFFFF.
     * ARM memory map can be found here:
     * https://developer.arm.com/documentation/ddi0337/e/memory-map/about-the-memory-map
     */
    LOG_DATA (R) : ORIGIN = 0x90000000, LENGTH = 0x40000
}

REGION_ALIAS("REGION_TEXT", FLASH);
REGION_ALIAS("REGION_BSS", SRAM);
REGION_ALIAS("REGION_DATA", SRAM);
REGION_ALIAS("REGION_STACK", SRAM);
REGION_ALIAS("REGION_HEAP", SRAM);
REGION_ALIAS("REGION_LOG", LOG_DATA);
REGION_ALIAS("REGION_ARM_EXIDX", FLASH);
REGION_ALIAS("REGION_ARM_EXTAB", FLASH);

SECTIONS {

    PROVIDE (_resetVecs_base_address =
        DEFINED(_resetVecs_base_address) ? _resetVecs_base_address : 0x0);

    .resetVecs (_resetVecs_base_address) : AT (_resetVecs_base_address) {
        KEEP (*(.resetVecs))
    } > REGION_TEXT

    .ramVecs (NOLOAD) : ALIGN(1024){
        KEEP (*(.ramVecs))
    } > REGION_DATA

    /*
     * UDMACC23XX_CONFIG_BASE below must match UDMACC23XX_CONFIG_BASE defined
     * by ti/drivers/dma/UDMACC23XX.h
     * The user is allowed to change UDMACC23XX_CONFIG_BASE to move it away from
     * the default address 0x20000400, but remember it must be 1024 bytes aligned.
     */
    UDMACC23XX_CONFIG_BASE = 0x20000400;
/*
     * Define absolute addresses for the DMA channels.
     * DMA channels must always be allocated at a fixed offset from the DMA base address.
     * --------- DO NOT MODIFY -----------
     */
    DMA_CHANNEL0_CONTROL_TABLE_ENTRY_ADDRESS = (UDMACC23XX_CONFIG_BASE + 0x00);
    DMA_CHANNEL1_CONTROL_TABLE_ENTRY_ADDRESS = (UDMACC23XX_CONFIG_BASE + 0x10);
    DMA_CHANNEL2_CONTROL_TABLE_ENTRY_ADDRESS = (UDMACC23XX_CONFIG_BASE + 0x20);
    DMA_CHANNEL3_CONTROL_TABLE_ENTRY_ADDRESS = (UDMACC23XX_CONFIG_BASE + 0x30);
    DMA_CHANNEL4_CONTROL_TABLE_ENTRY_ADDRESS = (UDMACC23XX_CONFIG_BASE + 0x40);
    DMA_CHANNEL5_CONTROL_TABLE_ENTRY_ADDRESS = (UDMACC23XX_CONFIG_BASE + 0x50);
    DMA_CHANNEL6_CONTROL_TABLE_ENTRY_ADDRESS = (UDMACC23XX_CONFIG_BASE + 0x60);
    DMA_CHANNEL7_CONTROL_TABLE_ENTRY_ADDRESS = (UDMACC23XX_CONFIG_BASE + 0x70);

    DMA_CHANNEL0_ALT_CONTROL_TABLE_ENTRY_ADDRESS = (UDMACC23XX_CONFIG_BASE + 0x80);
    DMA_CHANNEL1_ALT_CONTROL_TABLE_ENTRY_ADDRESS = (UDMACC23XX_CONFIG_BASE + 0x90);
    DMA_CHANNEL2_ALT_CONTROL_TABLE_ENTRY_ADDRESS = (UDMACC23XX_CONFIG_BASE + 0xA0);
    DMA_CHANNEL3_ALT_CONTROL_TABLE_ENTRY_ADDRESS = (UDMACC23XX_CONFIG_BASE + 0xB0);
    DMA_CHANNEL4_ALT_CONTROL_TABLE_ENTRY_ADDRESS = (UDMACC23XX_CONFIG_BASE + 0xC0);
    DMA_CHANNEL5_ALT_CONTROL_TABLE_ENTRY_ADDRESS = (UDMACC23XX_CONFIG_BASE + 0xD0);
    DMA_CHANNEL6_ALT_CONTROL_TABLE_ENTRY_ADDRESS = (UDMACC23XX_CONFIG_BASE + 0xE0);
    DMA_CHANNEL7_ALT_CONTROL_TABLE_ENTRY_ADDRESS = (UDMACC23XX_CONFIG_BASE + 0xF0);


    /*
     * Allocate Channel n descriptors at absolute addresses.
     * --------- DO NOT MODIFY -----------
     */
    UDMACC23XX_channel0ControlTableEntry_is_placed = 0;
    .dmaChannel0ControlTableEntry DMA_CHANNEL0_CONTROL_TABLE_ENTRY_ADDRESS (NOLOAD) : AT (DMA_CHANNEL0_CONTROL_TABLE_ENTRY_ADDRESS) {*(.dmaChannel0ControlTableEntry)} > REGION_DATA
    UDMACC23XX_channel1ControlTableEntry_is_placed = 0;
    .dmaChannel1ControlTableEntry DMA_CHANNEL1_CONTROL_TABLE_ENTRY_ADDRESS (NOLOAD) : AT (DMA_CHANNEL1_CONTROL_TABLE_ENTRY_ADDRESS) {*(.dmaChannel1ControlTableEntry)} > REGION_DATA
    UDMACC23XX_channel2ControlTableEntry_is_placed = 0;
    .dmaChannel2ControlTableEntry DMA_CHANNEL2_CONTROL_TABLE_ENTRY_ADDRESS (NOLOAD) : AT (DMA_CHANNEL2_CONTROL_TABLE_ENTRY_ADDRESS) {*(.dmaChannel2ControlTableEntry)} > REGION_DATA
    UDMACC23XX_channel3ControlTableEntry_is_placed = 0;
    .dmaChannel3ControlTableEntry DMA_CHANNEL3_CONTROL_TABLE_ENTRY_ADDRESS (NOLOAD) : AT (DMA_CHANNEL3_CONTROL_TABLE_ENTRY_ADDRESS) {*(.dmaChannel3ControlTableEntry)} > REGION_DATA
    UDMACC23XX_channel4ControlTableEntry_is_placed = 0;
    .dmaChannel4ControlTableEntry DMA_CHANNEL4_CONTROL_TABLE_ENTRY_ADDRESS (NOLOAD) : AT (DMA_CHANNEL4_CONTROL_TABLE_ENTRY_ADDRESS) {*(.dmaChannel4ControlTableEntry)} > REGION_DATA
    UDMACC23XX_channel5ControlTableEntry_is_placed = 0;
    .dmaChannel5ControlTableEntry DMA_CHANNEL5_CONTROL_TABLE_ENTRY_ADDRESS (NOLOAD) : AT (DMA_CHANNEL5_CONTROL_TABLE_ENTRY_ADDRESS) {*(.dmaChannel5ControlTableEntry)} > REGION_DATA
    UDMACC23XX_channel6ControlTableEntry_is_placed = 0;
    .dmaChannel6ControlTableEntry DMA_CHANNEL6_CONTROL_TABLE_ENTRY_ADDRESS (NOLOAD) : AT (DMA_CHANNEL6_CONTROL_TABLE_ENTRY_ADDRESS) {*(.dmaChannel6ControlTableEntry)} > REGION_DATA
    UDMACC23XX_channel7ControlTableEntry_is_placed = 0;
    .dmaChannel7ControlTableEntry DMA_CHANNEL7_CONTROL_TABLE_ENTRY_ADDRESS (NOLOAD) : AT (DMA_CHANNEL7_CONTROL_TABLE_ENTRY_ADDRESS) {*(.dmaChannel7ControlTableEntry)} > REGION_DATA

    UDMACC23XX_channel0AltControlTableEntry_is_placed = 0;
    .dmaChannel0AltControlTableEntry DMA_CHANNEL0_ALT_CONTROL_TABLE_ENTRY_ADDRESS (NOLOAD) : AT (DMA_CHANNEL0_ALT_CONTROL_TABLE_ENTRY_ADDRESS) {*(.dmaChannel0AltControlTableEntry)} > REGION_DATA
    UDMACC23XX_channel1AltControlTableEntry_is_placed = 0;
    .dmaChannel1AltControlTableEntry DMA_CHANNEL1_ALT_CONTROL_TABLE_ENTRY_ADDRESS (NOLOAD) : AT (DMA_CHANNEL1_ALT_CONTROL_TABLE_ENTRY_ADDRESS) {*(.dmaChannel1AltControlTableEntry)} > REGION_DATA
    UDMACC23XX_channel2AltControlTableEntry_is_placed = 0;
    .dmaChannel2AltControlTableEntry DMA_CHANNEL2_ALT_CONTROL_TABLE_ENTRY_ADDRESS (NOLOAD) : AT (DMA_CHANNEL2_ALT_CONTROL_TABLE_ENTRY_ADDRESS) {*(.dmaChannel2AltControlTableEntry)} > REGION_DATA
    UDMACC23XX_channel3AltControlTableEntry_is_placed = 0;
    .dmaChannel3AltControlTableEntry DMA_CHANNEL3_ALT_CONTROL_TABLE_ENTRY_ADDRESS (NOLOAD) : AT (DMA_CHANNEL3_ALT_CONTROL_TABLE_ENTRY_ADDRESS) {*(.dmaChannel3AltControlTableEntry)} > REGION_DATA
    UDMACC23XX_channel4AltControlTableEntry_is_placed = 0;
    .dmaChannel4AltControlTableEntry DMA_CHANNEL4_ALT_CONTROL_TABLE_ENTRY_ADDRESS (NOLOAD) : AT (DMA_CHANNEL4_ALT_CONTROL_TABLE_ENTRY_ADDRESS) {*(.dmaChannel4AltControlTableEntry)} > REGION_DATA
    UDMACC23XX_channel5AltControlTableEntry_is_placed = 0;
    .dmaChannel5AltControlTableEntry DMA_CHANNEL5_ALT_CONTROL_TABLE_ENTRY_ADDRESS (NOLOAD) : AT (DMA_CHANNEL5_ALT_CONTROL_TABLE_ENTRY_ADDRESS) {*(.dmaChannel5AltControlTableEntry)} > REGION_DATA
    UDMACC23XX_channel6AltControlTableEntry_is_placed = 0;
    .dmaChannel6AltControlTableEntry DMA_CHANNEL6_ALT_CONTROL_TABLE_ENTRY_ADDRESS (NOLOAD) : AT (DMA_CHANNEL6_ALT_CONTROL_TABLE_ENTRY_ADDRESS) {*(.dmaChannel6AltControlTableEntry)} > REGION_DATA
    UDMACC23XX_channel7AltControlTableEntry_is_placed = 0;
    .dmaChannel7AltControlTableEntry DMA_CHANNEL7_ALT_CONTROL_TABLE_ENTRY_ADDRESS (NOLOAD) : AT (DMA_CHANNEL7_ALT_CONTROL_TABLE_ENTRY_ADDRESS) {*(.dmaChannel7AltControlTableEntry)} > REGION_DATA


    .text : {
        CREATE_OBJECT_SYMBOLS
        *(.text)
        *(.text.*)
        . = ALIGN(0x4);
        KEEP (*(.ctors))
        . = ALIGN(0x4);
        KEEP (*(.dtors))
        . = ALIGN(0x4);
        __init_array_start = .;
        KEEP (*(.init_array*))
        __init_array_end = .;
        *(.init)
        *(.fini*)
    } > REGION_TEXT AT> REGION_TEXT

    PROVIDE (__etext = .);
    PROVIDE (_etext = .);
    PROVIDE (etext = .);

    .rodata : {
        *(.rodata)
        *(.rodata.*)
    } > REGION_TEXT AT> REGION_TEXT

    .data : ALIGN(4) {
        __data_load__ = LOADADDR (.data);
        __data_start__ = .;
        *(.data)
        *(.data.*)
        . = ALIGN (4);
        __data_end__ = .;
    } > REGION_DATA AT> REGION_TEXT

    .ARM.exidx : {
        __exidx_start = .;
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
        __exidx_end = .;
    } > REGION_ARM_EXIDX AT> REGION_ARM_EXIDX

    .ARM.extab : {
        *(.ARM.extab* .gnu.linkonce.armextab.*)
    } > REGION_ARM_EXTAB AT> REGION_ARM_EXTAB

    .nvs (NOLOAD) : ALIGN(0x2000) {
        *(.nvs)
    } > REGION_TEXT

    .ccfg : {
        KEEP (*(.ccfg))
    } > FLASH_CCFG AT> FLASH_CCFG

    .bss : {
        __bss_start__ = .;
        *(.shbss)
        *(.bss)
        *(.bss.*)
        *(COMMON)
        . = ALIGN (4);
        __bss_end__ = .;
    } > REGION_BSS AT> REGION_BSS

    .heap : {
        __heap_start__ = .;
        end = __heap_start__;
        _end = end;
        __end = end;
        . = . + HEAPSIZE;
        KEEP(*(.heap))
        __heap_end__ = .;
        __HeapLimit = __heap_end__;
    } > REGION_HEAP AT> REGION_HEAP

    .stack (NOLOAD) : ALIGN(0x8) {
        _stack = .;
        __stack = .;
        KEEP(*(.stack))
        . += STACKSIZE;
        _stack_end = .;
        __stack_end = .;
    } > REGION_STACK AT> REGION_STACK

    .log_data (COPY) : {
        KEEP (*(.log_data))
    } > REGION_LOG
}
