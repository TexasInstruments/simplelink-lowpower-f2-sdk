/*
 * Copyright (c) 2022 Arm Limited. All rights reserved.
 * Copyright (c) 2024, Texas Instruments Incorporated. All rights reserved.
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

DEFAULT_IRQ_HANDLER(INT_CPUIRQ0_Handler)
DEFAULT_IRQ_HANDLER(INT_CPUIRQ1_Handler)
DEFAULT_IRQ_HANDLER(INT_CPUIRQ2_Handler)
DEFAULT_IRQ_HANDLER(INT_CPUIRQ3_Handler)
DEFAULT_IRQ_HANDLER(INT_CPUIRQ4_Handler)
DEFAULT_IRQ_HANDLER(INT_GPIO_COMB_Handler)
DEFAULT_IRQ_HANDLER(INT_LRFD_IRQ0_Handler)
DEFAULT_IRQ_HANDLER(INT_LRFD_IRQ1_Handler)
DEFAULT_IRQ_HANDLER(INT_DMA_DONE_COMB_Handler)
DEFAULT_IRQ_HANDLER(INT_AES_COMB_Handler)
DEFAULT_IRQ_HANDLER(INT_SPI0_COMB_Handler)
DEFAULT_IRQ_HANDLER(INT_UART0_COMB_Handler)
DEFAULT_IRQ_HANDLER(INT_I2C0_IRQ_Handler)
DEFAULT_IRQ_HANDLER(INT_LGPT0_COMB_Handler)
DEFAULT_IRQ_HANDLER(INT_LGPT1_COMB_Handler)
DEFAULT_IRQ_HANDLER(INT_ADC_COMB_Handler)
DEFAULT_IRQ_HANDLER(INT_CPUIRQ16_Handler)
DEFAULT_IRQ_HANDLER(INT_CPUIRQ17_Handler)
DEFAULT_IRQ_HANDLER(INT_LGPT2_COMB_Handler)
DEFAULT_IRQ_HANDLER(INT_LGPT3_COMB_Handler)
DEFAULT_IRQ_HANDLER(INT_I2S_IRQ_Handler)
DEFAULT_IRQ_HANDLER(INT_CAN_IRQ_Handler)
DEFAULT_IRQ_HANDLER(INT_UART1_COMB_Handler)
DEFAULT_IRQ_HANDLER(INT_SPI1_COMB_Handler)
DEFAULT_IRQ_HANDLER(INT_VCE_IRQ_Handler)
DEFAULT_IRQ_HANDLER(INT_HSM_SEC_IRQ_Handler)
DEFAULT_IRQ_HANDLER(INT_HSM_NONSEC_IRQ_Handler)
DEFAULT_IRQ_HANDLER(INT_HSM_OTP_IRQ_Handler)
DEFAULT_IRQ_HANDLER(INT_AON_PMU_COMB_Handler)
DEFAULT_IRQ_HANDLER(INT_AON_CKM_COMB_Handler)
DEFAULT_IRQ_HANDLER(INT_AON_RTC_COMB_Handler)
DEFAULT_IRQ_HANDLER(INT_AON_LPCMP_IRQ_Handler)
DEFAULT_IRQ_HANDLER(INT_AON_IOC_COMB_Handler)
DEFAULT_IRQ_HANDLER(INT_SW0_Handler)
DEFAULT_IRQ_HANDLER(INT_SW1_Handler)


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

    /******** External Interrupts ********/
    INT_CPUIRQ0_Handler,                   /*   0: IRQ0: Configurable source controlled by
                                                   EVTSVT.CPUIRQ0SEL */
    INT_CPUIRQ1_Handler,                   /*   1: IRQ1: Configurable source controlled by
                                                   EVTSVT.CPUIRQ1SEL */
    INT_CPUIRQ2_Handler,                   /*   2: IRQ2: Configurable source controlled by
                                                   EVTSVT.CPUIRQ2SEL */
    INT_CPUIRQ3_Handler,                   /*   3: IRQ3: Configurable source controlled by
                                                   EVTSVT.CPUIRQ3SEL */
    INT_CPUIRQ4_Handler,                   /*   4: IRQ4: Configurable source controlled by
                                                   EVTSVT.CPUIRQ4SEL */
    INT_GPIO_COMB_Handler,                 /*   5: GPIO combined wake up interrupt,
                                                   interrupt flags can be found here
                                                   GPIO:MIS */
    INT_LRFD_IRQ0_Handler,                 /*   6: LRFD combined event, interrupt flags can
                                                   be found here LRFDDBELL:MIS0 */
    INT_LRFD_IRQ1_Handler,                 /*   7: LRFD combined event, interrupt flags can
                                                   be found here LRFDDBELL:MIS1 */
    INT_DMA_DONE_COMB_Handler,             /*   8: DMA combined done interrupt,
                                                   corresponding flags can be found here
                                                   DMA:REQDONE */
    INT_AES_COMB_Handler,                  /*   9: AES accelerator combined interrupt
                                                   request, interrupt flags can be found
                                                   here AES:MIS */
    INT_SPI0_COMB_Handler,                 /*  10: SPI0 combined interrupt request,
                                                   interrupt flags can be found here
                                                   SPI0:MIS */
    INT_UART0_COMB_Handler,                /*  11: UART0 combined interrupt, interrupt flags
                                                   are found here UART0:MIS */
    INT_I2C0_IRQ_Handler,                  /*  12: Interrupt event from I2C0, interrupt
                                                   flags can be found here I2C0:MIS */
    INT_LGPT0_COMB_Handler,                /*  13: LGPT0 combined interrupt, interrupt flags
                                                   are found here LGPT0:MIS */
    INT_LGPT1_COMB_Handler,                /*  14: LGPT1 combined interrupt, interrupt flags
                                                   are found here LGPT1:MIS */
    INT_ADC_COMB_Handler,                  /*  15: ADC combined interrupt request, interrupt
                                                   flags can be found here ADC:MIS0 */
    INT_CPUIRQ16_Handler,                  /*  16: IRQ16: Configurable source controlled by
                                                   EVTSVT.CPUIRQ16SEL */
    INT_CPUIRQ17_Handler,                  /*  17: IRQ17: Configurable source controlled by
                                                   EVTSVT.CPUIRQ17SEL */
    INT_LGPT2_COMB_Handler,                /*  18: LGPT2 combined interrupt, interrupt flags
                                                   are found here LGPT2:MIS */
    INT_LGPT3_COMB_Handler,                /*  19: LGPT3 combined interrupt, interrupt flags
                                                   are found here LGPT3:MIS */
    INT_I2S_IRQ_Handler,                   /*  20: I2S interrupt event, controlled by
                                                   I2S:IRQMASK */
    INT_CAN_IRQ_Handler,                   /*  21: MCAN interrupt event, interrupt flags can
                                                   be found here MCAN:MIS0 */
    INT_UART1_COMB_Handler,                /*  22: UART1 combined interrupt, interrupt flags
                                                   are found here UART1:MIS */
    INT_SPI1_COMB_Handler,                 /*  23: SPI1 combined interrupt request,
                                                   interrupt flags can be found here
                                                   SPI1:MIS */
    INT_VCE_IRQ_Handler,                   /*  24: VCE IRQ */
    INT_HSM_SEC_IRQ_Handler,               /*  25: HSM Secure IRQ */
    INT_HSM_NONSEC_IRQ_Handler,            /*  26: HSM Non-secure IRQ */
    INT_HSM_OTP_IRQ_Handler,               /*  27: HSM OTP IRQ */
    INT_AON_PMU_COMB_Handler,              /*  28: PMU combined interrupt request for
                                                   BATMON, interrupt flags can be found here
                                                   PMUD:EVENT */
    INT_AON_CKM_COMB_Handler,              /*  29: CKMD combined interrupt request,
                                                   interrupt flags can be found here
                                                   CKMD:MIS */
    INT_AON_RTC_COMB_Handler,              /*  30: AON_RTC event, controlled by the
                                                   RTC:IMASK setting */
    INT_AON_LPCMP_IRQ_Handler,             /*  31: AON LPCMP interrupt, controlled by
                                                   SYS0:LPCMPCFG */
    INT_AON_IOC_COMB_Handler,              /*  32: IOC synchronous combined event,
                                                   controlled by IOC:EVTCFG */
    INT_SW0_Handler,                       /*  33: Software Triggered Interrupt 0 */
    INT_SW1_Handler,                       /*  34: Software Triggered Interrupt 1 */
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
