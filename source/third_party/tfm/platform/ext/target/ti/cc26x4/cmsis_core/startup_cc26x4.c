/*
 * Copyright (c) 2022 Arm Limited. All rights reserved.
 * Copyright (c) 2023, Texas Instruments Incorporated. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * This file is derivative of CMSIS V5.9.0 startup_ARMCM33.c
 * Git SHA: 09825bcfe7659e019308e92f6b6cba3a11e66285
 */

#include "cmsis.h"

/*----------------------------------------------------------------------------
  External References
 *----------------------------------------------------------------------------*/
extern uint32_t __INITIAL_SP;
extern uint32_t __STACK_LIMIT;
#if defined (__ARM_FEATURE_CMSE) && (__ARM_FEATURE_CMSE == 3U)
extern uint64_t __STACK_SEAL;
#endif

extern void __PROGRAM_START(void) __NO_RETURN;

/*----------------------------------------------------------------------------
  Internal References
 *----------------------------------------------------------------------------*/
void Reset_Handler(void) __NO_RETURN;

/*----------------------------------------------------------------------------
  Exception / Interrupt Handler
 *----------------------------------------------------------------------------*/
#define DEFAULT_IRQ_HANDLER(handler_name)  \
void __WEAK handler_name(void) __NO_RETURN; \
void handler_name(void) { \
    while(1); \
}

/* Fault IRQ handler which copies banked NS registers to CPU registers to aid
 * in debugging and for display by CCS Runtime Object View (ROV)
 */
#define FAULT_IRQ_HANDLER(handler_name)  \
void __WEAK handler_name(void) __NO_RETURN; \
void handler_name(void) { \
    __asm volatile("mrs r0, msp_ns"); \
    __asm volatile("mrs r1, psp_ns"); \
    __asm volatile("mrs r2, control_ns"); \
    while(1); \
}

/* Exceptions */
FAULT_IRQ_HANDLER(NMI_Handler)
FAULT_IRQ_HANDLER(HardFault_Handler)
FAULT_IRQ_HANDLER(MemManage_Handler)
FAULT_IRQ_HANDLER(BusFault_Handler)
FAULT_IRQ_HANDLER(UsageFault_Handler)
FAULT_IRQ_HANDLER(SecureFault_Handler)
DEFAULT_IRQ_HANDLER(SVC_Handler)
DEFAULT_IRQ_HANDLER(DebugMon_Handler)
DEFAULT_IRQ_HANDLER(PendSV_Handler)
DEFAULT_IRQ_HANDLER(SysTick_Handler)

DEFAULT_IRQ_HANDLER(INT_AON_GPIO_EDGE_Handler)
DEFAULT_IRQ_HANDLER(INT_I2C_IRQ_Handler)
DEFAULT_IRQ_HANDLER(INT_RFC_CPE_1_Handler)
DEFAULT_IRQ_HANDLER(INT_PKA_IRQ_Handler)
DEFAULT_IRQ_HANDLER(INT_AON_RTC_COMB_Handler)
DEFAULT_IRQ_HANDLER(INT_UART0_COMB_Handler)
DEFAULT_IRQ_HANDLER(INT_AUX_SWEV0_Handler)
DEFAULT_IRQ_HANDLER(INT_SSI0_COMB_Handler)
DEFAULT_IRQ_HANDLER(INT_SSI1_COMB_Handler)
DEFAULT_IRQ_HANDLER(INT_RFC_CPE_0_Handler)
DEFAULT_IRQ_HANDLER(INT_RFC_HW_COMB_Handler)
DEFAULT_IRQ_HANDLER(INT_RFC_CMD_ACK_Handler)
DEFAULT_IRQ_HANDLER(INT_I2S_IRQ_Handler)
DEFAULT_IRQ_HANDLER(INT_AUX_SWEV1_Handler)
DEFAULT_IRQ_HANDLER(INT_WDT_IRQ_Handler)
DEFAULT_IRQ_HANDLER(INT_GPT0A_Handler)
DEFAULT_IRQ_HANDLER(INT_GPT0B_Handler)
DEFAULT_IRQ_HANDLER(INT_GPT1A_Handler)
DEFAULT_IRQ_HANDLER(INT_GPT1B_Handler)
DEFAULT_IRQ_HANDLER(INT_GPT2A_Handler)
DEFAULT_IRQ_HANDLER(INT_GPT2B_Handler)
DEFAULT_IRQ_HANDLER(INT_GPT3A_Handler)
DEFAULT_IRQ_HANDLER(INT_GPT3B_Handler)
DEFAULT_IRQ_HANDLER(INT_CRYPTO_RESULT_AVAIL_IRQ_Handler)
DEFAULT_IRQ_HANDLER(INT_DMA_DONE_COMB_Handler)
DEFAULT_IRQ_HANDLER(INT_DMA_ERR_Handler)
DEFAULT_IRQ_HANDLER(INT_FLASH_Handler)
DEFAULT_IRQ_HANDLER(INT_SWEV0_Handler)
DEFAULT_IRQ_HANDLER(INT_AUX_COMB_Handler)
DEFAULT_IRQ_HANDLER(INT_AON_PROG0_Handler)
DEFAULT_IRQ_HANDLER(INT_PROG0_Handler)
DEFAULT_IRQ_HANDLER(INT_AUX_COMPA_Handler)
DEFAULT_IRQ_HANDLER(INT_AUX_ADC_IRQ_Handler)
DEFAULT_IRQ_HANDLER(INT_TRNG_IRQ_Handler)
DEFAULT_IRQ_HANDLER(INT_OSC_COMB_Handler)
DEFAULT_IRQ_HANDLER(INT_AUX_TIMER2_EV0_Handler)
DEFAULT_IRQ_HANDLER(INT_UART1_COMB_Handler)
DEFAULT_IRQ_HANDLER(INT_BATMON_COMB_Handler)
DEFAULT_IRQ_HANDLER(INT_SSI2_COMB_Handler)
DEFAULT_IRQ_HANDLER(INT_SSI3_COMB_Handler)
DEFAULT_IRQ_HANDLER(INT_UART2_COMB_Handler)
DEFAULT_IRQ_HANDLER(INT_UART3_COMB_Handler)
DEFAULT_IRQ_HANDLER(INT_I2C1_IRQ_Handler)
DEFAULT_IRQ_HANDLER(INT_SWEV1_Handler)
DEFAULT_IRQ_HANDLER(INT_SWEV2_Handler)
DEFAULT_IRQ_HANDLER(INT_SWEV3_Handler)
DEFAULT_IRQ_HANDLER(INT_SWEV4_Handler)


/*----------------------------------------------------------------------------
  Exception / Interrupt Vector table
 *----------------------------------------------------------------------------*/

#if defined ( __GNUC__ )
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#endif

extern const VECTOR_TABLE_Type __VECTOR_TABLE[];
       const VECTOR_TABLE_Type __VECTOR_TABLE[] __VECTOR_TABLE_ATTRIBUTE = {
    (VECTOR_TABLE_Type)(&__INITIAL_SP),    /*      Initial Stack Pointer */
    Reset_Handler,                         /*      Reset Handler */
    NMI_Handler,                           /* -14: NMI Handler */
    HardFault_Handler,                     /* -13: Hard Fault Handler */
    MemManage_Handler,                     /* -12: MPU Fault Handler */
    BusFault_Handler,                      /* -11: Bus Fault Handler */
    UsageFault_Handler,                    /* -10: Usage Fault Handler */
    SecureFault_Handler,                   /*  -9: Secure Fault Handler */
    0,                                     /*  -8: Reserved */
    0,                                     /*  -7: Reserved */
    0,                                     /*  -6: Reserved */
    SVC_Handler,                           /*  -5: SVCall Handler */
    DebugMon_Handler,                      /*  -4: Debug Monitor Handler */
    0,                                     /*  -3: Reserved */
    PendSV_Handler,                        /*  -2: PendSV Handler */
    SysTick_Handler,                       /*  -1: SysTick Handler */

    INT_AON_GPIO_EDGE_Handler,             /*   0: Edge detect event from IOC */
    INT_I2C_IRQ_Handler,                   /*   1: Interrupt event from I2C */
    INT_RFC_CPE_1_Handler,                 /*   2: Combined Interrupt for CPE */
    INT_PKA_IRQ_Handler,                   /*   3: PKA Interrupt event */
    INT_AON_RTC_COMB_Handler,              /*   4: Event from AON_RTC */
    INT_UART0_COMB_Handler,                /*   5: UART0 combined interrupt */
    INT_AUX_SWEV0_Handler,                 /*   6: AUX software event 0 */
    INT_SSI0_COMB_Handler,                 /*   7: SSI0 combined interrupt */
    INT_SSI1_COMB_Handler,                 /*   8: SSI1 combined interrupt */
    INT_RFC_CPE_0_Handler,                 /*   9: Combined Interrupt for CPE */
    INT_RFC_HW_COMB_Handler,               /*  10: Combined RFC hardware interrupt */
    INT_RFC_CMD_ACK_Handler,               /*  11: RFC Doorbell Command */
    INT_I2S_IRQ_Handler,                   /*  12: Interrupt event from I2S */
    INT_AUX_SWEV1_Handler,                 /*  13: AUX software event 1 */
    INT_WDT_IRQ_Handler,                   /*  14: Watchdog interrupt event */
    INT_GPT0A_Handler,                     /*  15: GPT0A interrupt event */
    INT_GPT0B_Handler,                     /*  16: GPT0B interrupt event */
    INT_GPT1A_Handler,                     /*  17: GPT1A interrupt event */
    INT_GPT1B_Handler,                     /*  18: GPT1B interrupt event */
    INT_GPT2A_Handler,                     /*  19: GPT2A interrupt event */
    INT_GPT2B_Handler,                     /*  20: GPT2B interrupt event */
    INT_GPT3A_Handler,                     /*  21: GPT3A interrupt event */
    INT_GPT3B_Handler,                     /*  22: GPT3B interrupt event */
    INT_CRYPTO_RESULT_AVAIL_IRQ_Handler,   /*  23: CRYPTO result available interupt */
    INT_DMA_DONE_COMB_Handler,             /*  24: Combined DMA done */
    INT_DMA_ERR_Handler,                   /*  25: DMA bus error */
    INT_FLASH_Handler,                     /*  26: FLASH controller error event */
    INT_SWEV0_Handler,                     /*  27: Software event 0 */
    INT_AUX_COMB_Handler,                  /*  28: AUX combined event */
    INT_AON_PROG0_Handler,                 /*  29: AON programmable event 0 */
    INT_PROG0_Handler,                     /*  30: Programmable Interrupt 0 */
    INT_AUX_COMPA_Handler,                 /*  31: AUX Compare A event */
    INT_AUX_ADC_IRQ_Handler,               /*  32: AUX ADC interrupt event */
    INT_TRNG_IRQ_Handler,                  /*  33: TRNG Interrupt event */
    INT_OSC_COMB_Handler,                  /*  34: Combined event from Oscillator */
    INT_AUX_TIMER2_EV0_Handler,            /*  35: AUX Timer2 event 0 */
    INT_UART1_COMB_Handler,                /*  36: UART1 combined interrupt */
    INT_BATMON_COMB_Handler,               /*  37: Combined event from battery */
    INT_SSI2_COMB_Handler,                 /*  38: SSI2 combined interrupt */
    INT_SSI3_COMB_Handler,                 /*  39: SSI3 combined interrupt */
    INT_UART2_COMB_Handler,                /*  40: UART2 combined interrupt */
    INT_UART3_COMB_Handler,                /*  41: UART3 combined interrupt */
    INT_I2C1_IRQ_Handler,                  /*  42: Interrupt event from I2C1 */
    INT_SWEV1_Handler,                     /*  43: Software event 1 */
    INT_SWEV2_Handler,                     /*  44: Software event 2 */
    INT_SWEV3_Handler,                     /*  45: Software event 3 */
    INT_SWEV4_Handler,                     /*  46: Software event 4 */
};

#if defined ( __GNUC__ )
#pragma GCC diagnostic pop
#endif

/*----------------------------------------------------------------------------
  Reset Handler called on controller reset
 *----------------------------------------------------------------------------*/
void Reset_Handler(void)
{
#if defined (__ARM_FEATURE_CMSE) && (__ARM_FEATURE_CMSE == 3U)
    /* Disable interrupts for SPE only. Interrupts will be re-enabled during
     * platform initialization */
    __disable_irq();
#endif

    __set_PSP((uint32_t)(&__INITIAL_SP));

    __set_MSPLIM((uint32_t)(&__STACK_LIMIT));
    __set_PSPLIM((uint32_t)(&__STACK_LIMIT));

#if defined (__ARM_FEATURE_CMSE) && (__ARM_FEATURE_CMSE == 3U)
    /* Set stack seal for SPE only */
    __TZ_set_STACKSEAL_S((uint32_t *)(&__STACK_SEAL));
#endif

    SystemInit();                   /* CMSIS System Initialization */
    __PROGRAM_START();              /* Enter PreMain (C library entry point) */
}
