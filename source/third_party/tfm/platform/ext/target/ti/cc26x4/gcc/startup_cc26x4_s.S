;/*
; * Copyright (c) 2016-2018 ARM Limited
; *
; * Licensed under the Apache License, Version 2.0 (the "License");
; * you may not use this file except in compliance with the License.
; * You may obtain a copy of the License at
; *
; *     http://www.apache.org/licenses/LICENSE-2.0
; *
; * Unless required by applicable law or agreed to in writing, software
; * distributed under the License is distributed on an "AS IS" BASIS,
; * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
; * See the License for the specific language governing permissions and
; * limitations under the License.
; *
; *
; * This file is derivative of CMSIS V5.00 startup_ARMCM33.S
; */

    .syntax     unified
    .arch       armv8-m.main

    .section    .vectors
    .align      2

    .global     __Vectors
__Vectors:
    .long   Image$$ARM_LIB_STACK_MSP$$ZI$$Limit
                                        /* 0  Top of Stack */
    /* Core interrupts */

    .long   Reset_Handler             /* 1  Reset Handler         */
    .long   NMI_Handler               /* 2  NMI Handler           */
    .long   HardFault_Handler         /* 3  Hard Fault Handler    */
    .long   MemManage_Handler         /* 4  MPU Fault Handler     */
    .long   BusFault_Handler          /* 5  Bus Fault Handler     */
    .long   UsageFault_Handler        /* 6  Usage Fault Handler   */
    .long   SecureFault_Handler       /* 7  Secure Fault Handler  */
    .long   0                         /* 8  Reserved              */
    .long   0                         /* 9  Reserved              */
    .long   0                         /* 10 Reserved              */
    .long   SVC_Handler               /* 11 SVCall Handler        */
    .long   DebugMon_Handler          /* 12 Debug Monitor Handler */
    .long   0                         /* 13 Reserved              */
    .long   PendSV_Handler            /* 14 PendSV Handler        */
    .long   SysTick_Handler           /* 15 SysTick Handler       */

    /*  Peripheral Exceptions Numbers */

    .long INT_AON_GPIO_EDGE_Handler             // 0 Edge detect event from IOC
    .long INT_I2C_IRQ_Handler                   // 1 Interrupt event from I2C
    .long INT_RFC_CPE_1_Handler                 // 2 Combined Interrupt for CPE
    .long INT_PKA_IRQ_Handler                   // 3 PKA Interrupt event
    .long INT_AON_RTC_COMB_Handler              // 4 Event from AON_RTC
    .long INT_UART0_COMB_Handler                // 5 UART0 combined interrupt
    .long INT_AUX_SWEV0_Handler                 // 6 AUX software event 0
    .long INT_SSI0_COMB_Handler                 // 7 SSI0 combined interrupt
    .long INT_SSI1_COMB_Handler                 // 8 SSI1 combined interrupt
    .long INT_RFC_CPE_0_Handler                 // 9 Combined Interrupt for CPE
    .long INT_RFC_HW_COMB_Handler               // 10 Combined RFC hardware interrupt
    .long INT_RFC_CMD_ACK_Handler               // 11 RFC Doorbell Command
    .long INT_I2S_IRQ_Handler                   // 12 Interrupt event from I2S
    .long INT_AUX_SWEV1_Handler                 // 13 AUX software event 1
    .long INT_WDT_IRQ_Handler                   // 14 Watchdog interrupt event
    .long INT_GPT0A_Handler                     // 15 GPT0A interrupt event
    .long INT_GPT0B_Handler                     // 16 GPT0B interrupt event
    .long INT_GPT1A_Handler                     // 17 GPT1A interrupt event
    .long INT_GPT1B_Handler                     // 18 GPT1B interrupt event
    .long INT_GPT2A_Handler                     // 19 GPT2A interrupt event
    .long INT_GPT2B_Handler                     // 20 GPT2B interrupt event
    .long INT_GPT3A_Handler                     // 21 GPT3A interrupt event
    .long INT_GPT3B_Handler                     // 22 GPT3B interrupt event
    .long INT_CRYPTO_RESULT_AVAIL_IRQ_Handler   // 23 CRYPTO result available interupt
    .long INT_DMA_DONE_COMB_Handler             // 24 Combined DMA done
    .long INT_DMA_ERR_Handler                   // 25 DMA bus error
    .long INT_FLASH_Handler                     // 26 FLASH controller error event
    .long INT_SWEV0_Handler                     // 27 Software event 0
    .long INT_AUX_COMB_Handler                  // 28 AUX combined event
    .long INT_AON_PROG0_Handler                 // 29 AON programmable event 0
    .long INT_PROG0_Handler                     // 30 Programmable Interrupt 0
    .long INT_AUX_COMPA_Handler                 // 31 AUX Compare A event
    .long INT_AUX_ADC_IRQ_Handler               // 32 AUX ADC interrupt event
    .long INT_TRNG_IRQ_Handler                  // 33 TRNG Interrupt event
    .long INT_OSC_COMB_Handler                  // 34 Combined event from Oscillator
    .long INT_AUX_TIMER2_EV0_Handler            // 35 AUX Timer2 event 0
    .long INT_UART1_COMB_Handler                // 36 UART1 combined interrupt
    .long INT_BATMON_COMB_Handler               // 37 Combined event from battery
    .long INT_SSI2_COMB_Handler                 // 38 SSI2 combined interrupt
    .long INT_SSI3_COMB_Handler                 // 39 SSI3 combined interrupt
    .long INT_UART2_COMB_Handler                // 40 UART2 combined interrupt
    .long INT_UART3_COMB_Handler                // 41 UART3 combined interrupt
    .long INT_I2C1_IRQ_Handler                  // 42 Interrupt event from I2C1
    .long INT_SWEV1_Handler                     // 43 Software event 1. Name not provided by DOC yet and thus preliminary */
    .long INT_SWEV2_Handler                     // 44 Software event 2. Name not provided by DOC yet and thus preliminary */
    .long INT_SWEV3_Handler                     // 45 Software event 3. Name not provided by DOC yet and thus preliminary */
    .long INT_SWEV4_Handler                     // 46 Software event 4. Name not provided by DOC yet and thus preliminary */

    .size   __Vectors, . - __Vectors

    .text
    .thumb
    .thumb_func
    .align  6
    .global Reset_Handler
    .type   Reset_Handler, %function
Reset_Handler:
    ldr     r0, =Image$$ARM_LIB_STACK_MSP$$ZI$$Limit
    msr     msp, r0
/*  Firstly it copies data from read only memory to RAM. There are two schemes
 *  to copy. One can copy more than one sections. Another can only copy
 *  one section.  The former scheme needs more instructions and read-only
 *  data to implement than the latter.
 *  Macro __STARTUP_COPY_MULTIPLE is used to choose between two schemes.  */

#ifdef __STARTUP_COPY_MULTIPLE
/*  Multiple sections scheme.
 *
 *  Between symbol address __copy_table_start__ and __copy_table_end__,
 *  there are array of triplets, each of which specify:
 *    offset 0: LMA of start of a section to copy from
 *    offset 4: VMA of start of a section to copy to
 *    offset 8: size of the section to copy. Must be multiply of 4
 *
 *  All addresses must be aligned to 4 bytes boundary.
 */
    ldr     r4, =__copy_table_start__
    ldr     r5, =__copy_table_end__

.L_loop0:
    cmp     r4, r5
    bge     .L_loop0_done
    ldr     r1, [r4]
    ldr     r2, [r4, #4]
    ldr     r3, [r4, #8]

.L_loop0_0:
    subs    r3, #4
    ittt    ge
    ldrge   r0, [r1, r3]
    strge   r0, [r2, r3]
    bge     .L_loop0_0

    adds    r4, #12
    b       .L_loop0

.L_loop0_done:
#else
/*  Single section scheme.
 *
 *  The ranges of copy from/to are specified by following symbols
 *    __etext: LMA of start of the section to copy from. Usually end of text
 *    __data_start__: VMA of start of the section to copy to
 *    __data_end__: VMA of end of the section to copy to
 *
 *  All addresses must be aligned to 4 bytes boundary.
 */
    ldr     r1, =__etext
    ldr     r2, =__data_start__
    ldr     r3, =__data_end__

.L_loop1:
    cmp     r2, r3
    ittt    lt
    ldrlt   r0, [r1], #4
    strlt   r0, [r2], #4
    blt     .L_loop1
#endif /*__STARTUP_COPY_MULTIPLE */

/*  This part of work usually is done in C library startup code. Otherwise,
 *  define this macro to enable it in this startup.
 *
 *  There are two schemes too. One can clear multiple BSS sections. Another
 *  can only clear one section. The former is more size expensive than the
 *  latter.
 *
 *  Define macro __STARTUP_CLEAR_BSS_MULTIPLE to choose the former.
 *  Otherwise efine macro __STARTUP_CLEAR_BSS to choose the later.
 */
#ifdef __STARTUP_CLEAR_BSS_MULTIPLE
/*  Multiple sections scheme.
 *
 *  Between symbol address __copy_table_start__ and __copy_table_end__,
 *  there are array of tuples specifying:
 *    offset 0: Start of a BSS section
 *    offset 4: Size of this BSS section. Must be multiply of 4
 */
    ldr     r3, =__zero_table_start__
    ldr     r4, =__zero_table_end__

.L_loop2:
    cmp     r3, r4
    bge     .L_loop2_done
    ldr     r1, [r3]
    ldr     r2, [r3, #4]
    movs    r0, 0

.L_loop2_0:
    subs    r2, #4
    itt     ge
    strge   r0, [r1, r2]
    bge     .L_loop2_0

    adds    r3, #8
    b       .L_loop2
.L_loop2_done:
#elif defined (__STARTUP_CLEAR_BSS)
/*  Single BSS section scheme.
 *
 *  The BSS section is specified by following symbols
 *    __bss_start__: start of the BSS section.
 *    __bss_end__: end of the BSS section.
 *
 *  Both addresses must be aligned to 4 bytes boundary.
 */
    ldr     r1, =__bss_start__
    ldr     r2, =__bss_end__

    movs    r0, 0
.L_loop3:
    cmp     r1, r2
    itt     lt
    strlt   r0, [r1], #4
    blt     .L_loop3
#endif /* __STARTUP_CLEAR_BSS_MULTIPLE || __STARTUP_CLEAR_BSS */

#ifndef __NO_SYSTEM_INIT
    bl      SystemInit
#endif

    mrs     r0, control    /* Get control value */
    orr     r0, r0, #2     /* Select switch to PSP */
    msr     control, r0
    ldr     r0, =Image$$ARM_LIB_STACK$$ZI$$Limit
    msr     psp, r0

#ifndef __START
#define __START _start
#endif
    bl      __START

    .global Shared_exception_handler_loop
    .align  1
    .thumb_func
NMI_Handler:
    .thumb_func
HardFault_Handler:
    .thumb_func
MemManage_Handler:
    .thumb_func
BusFault_Handler:
    .thumb_func
UsageFault_Handler:
    .thumb_func
SecureFault_Handler:
    .thumb_func
DebugMon_Handler:
    .thumb_func
    mrs     r0, msp_ns
    mrs     r1, psp_ns
    mrs     r2, control_ns
Shared_exception_handler_loop:
    b       Shared_exception_handler_loop

    .pool
    .size   Reset_Handler, . - Reset_Handler


    /*  Macro to define default handlers. */
    .macro  def_irq_handler    handler_name
    .align  1
    .thumb_func
    .weak    \handler_name
    \handler_name:
    b        \handler_name
    .endm

    /* Core interrupts */
    def_irq_handler      SVC_Handler
    def_irq_handler      PendSV_Handler
    def_irq_handler      SysTick_Handler

    /*  Peripheral Exceptions Numbers */

    def_irq_handler INT_AON_GPIO_EDGE_Handler             // 0 Edge detect event from IOC
    def_irq_handler INT_I2C_IRQ_Handler                   // 1 Interrupt event from I2C
    def_irq_handler INT_RFC_CPE_1_Handler                 // 2 Combined Interrupt for CPE
    def_irq_handler INT_PKA_IRQ_Handler                   // 3 PKA Interrupt event
    def_irq_handler INT_AON_RTC_COMB_Handler              // 4 Event from AON_RTC
    def_irq_handler INT_UART0_COMB_Handler                // 5 UART0 combined interrupt
    def_irq_handler INT_AUX_SWEV0_Handler                 // 6 AUX software event 0
    def_irq_handler INT_SSI0_COMB_Handler                 // 7 SSI0 combined interrupt
    def_irq_handler INT_SSI1_COMB_Handler                 // 8 SSI1 combined interrupt
    def_irq_handler INT_RFC_CPE_0_Handler                 // 9 Combined Interrupt for CPE
    def_irq_handler INT_RFC_HW_COMB_Handler               // 10 Combined RFC hardware interrupt
    def_irq_handler INT_RFC_CMD_ACK_Handler               // 11 RFC Doorbell Command
    def_irq_handler INT_I2S_IRQ_Handler                   // 12 Interrupt event from I2S
    def_irq_handler INT_AUX_SWEV1_Handler                 // 13 AUX software event 1
    def_irq_handler INT_WDT_IRQ_Handler                   // 14 Watchdog interrupt event
    def_irq_handler INT_GPT0A_Handler                     // 15 GPT0A interrupt event
    def_irq_handler INT_GPT0B_Handler                     // 16 GPT0B interrupt event
    def_irq_handler INT_GPT1A_Handler                     // 17 GPT1A interrupt event
    def_irq_handler INT_GPT1B_Handler                     // 18 GPT1B interrupt event
    def_irq_handler INT_GPT2A_Handler                     // 19 GPT2A interrupt event
    def_irq_handler INT_GPT2B_Handler                     // 20 GPT2B interrupt event
    def_irq_handler INT_GPT3A_Handler                     // 21 GPT3A interrupt event
    def_irq_handler INT_GPT3B_Handler                     // 22 GPT3B interrupt event
    def_irq_handler INT_CRYPTO_RESULT_AVAIL_IRQ_Handler   // 23 CRYPTO result available interupt
    def_irq_handler INT_DMA_DONE_COMB_Handler             // 24 Combined DMA done
    def_irq_handler INT_DMA_ERR_Handler                   // 25 DMA bus error
    def_irq_handler INT_FLASH_Handler                     // 26 FLASH controller error event
    def_irq_handler INT_SWEV0_Handler                     // 27 Software event 0
    def_irq_handler INT_AUX_COMB_Handler                  // 28 AUX combined event
    def_irq_handler INT_AON_PROG0_Handler                 // 29 AON programmable event 0
    def_irq_handler INT_PROG0_Handler                     // 30 Programmable Interrupt 0
    def_irq_handler INT_AUX_COMPA_Handler                 // 31 AUX Compare A event
    def_irq_handler INT_AUX_ADC_IRQ_Handler               // 32 AUX ADC interrupt event
    def_irq_handler INT_TRNG_IRQ_Handler                  // 33 TRNG Interrupt event
    def_irq_handler INT_OSC_COMB_Handler                  // 34 Combined event from Oscillator
    def_irq_handler INT_AUX_TIMER2_EV0_Handler            // 35 AUX Timer2 event 0
    def_irq_handler INT_UART1_COMB_Handler                // 36 UART1 combined interrupt
    def_irq_handler INT_BATMON_COMB_Handler               // 37 Combined event from battery
    def_irq_handler INT_SSI2_COMB_Handler                 // 38 SSI2 combined interrupt
    def_irq_handler INT_SSI3_COMB_Handler                 // 39 SSI3 combined interrupt
    def_irq_handler INT_UART2_COMB_Handler                // 40 UART2 combined interrupt
    def_irq_handler INT_UART3_COMB_Handler                // 41 UART3 combined interrupt
    def_irq_handler INT_I2C1_IRQ_Handler                  // 42 Interrupt event from I2C1
    def_irq_handler INT_SWEV1_Handler                     // 43 Software event 1. Name not provided by DOC yet and thus preliminary */
    def_irq_handler INT_SWEV2_Handler                     // 44 Software event 2. Name not provided by DOC yet and thus preliminary */
    def_irq_handler INT_SWEV3_Handler                     // 45 Software event 3. Name not provided by DOC yet and thus preliminary */
    def_irq_handler INT_SWEV4_Handler                     // 46 Software event 4. Name not provided by DOC yet and thus preliminary */

    def_irq_handler UNDEFINED_INT_Handler                 // Undefined Interrupt

    .end
