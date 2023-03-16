/*
 * Copyright (c) 2009-2020 Arm Limited. All rights reserved.
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
 * This file is derivative of CMSIS V5.6.0 startup_ARMv81MML.c
 * Git SHA: b5f0603d6a584d1724d952fd8b0737458b90d62b
 */

#include "cmsis.h"

/*----------------------------------------------------------------------------
  Exception / Interrupt Handler Function Prototype
 *----------------------------------------------------------------------------*/
typedef void( *pFunc )( void );

/*----------------------------------------------------------------------------
  External References
 *----------------------------------------------------------------------------*/

#define __MSP_INITIAL_SP              Image$$ARM_LIB_STACK_MSP$$ZI$$Limit
#define __MSP_STACK_LIMIT             Image$$ARM_LIB_STACK_MSP$$ZI$$Base

extern uint32_t __MSP_INITIAL_SP;
extern uint32_t __MSP_STACK_LIMIT;

extern uint32_t __INITIAL_SP;
extern uint32_t __STACK_LIMIT;

extern void __PROGRAM_START(void) __NO_RETURN;

/*----------------------------------------------------------------------------
  Internal References
 *----------------------------------------------------------------------------*/
void Reset_Handler  (void) __NO_RETURN;

/*----------------------------------------------------------------------------
  Exception / Interrupt Handler
 *----------------------------------------------------------------------------*/
#define DEFAULT_IRQ_HANDLER(handler_name)  \
void __WEAK handler_name(void); \
void handler_name(void) { \
    while(1); \
}

/* Exceptions */
DEFAULT_IRQ_HANDLER(NMI_Handler)
DEFAULT_IRQ_HANDLER(HardFault_Handler)
DEFAULT_IRQ_HANDLER(MemManage_Handler)
DEFAULT_IRQ_HANDLER(BusFault_Handler)
DEFAULT_IRQ_HANDLER(UsageFault_Handler)
DEFAULT_IRQ_HANDLER(SecureFault_Handler)
DEFAULT_IRQ_HANDLER(SVC_Handler)
DEFAULT_IRQ_HANDLER(DebugMon_Handler)
DEFAULT_IRQ_HANDLER(PendSV_Handler)
DEFAULT_IRQ_HANDLER(SysTick_Handler)

DEFAULT_IRQ_HANDLER(NONSEC_WATCHDOG_RESET_Handler)
DEFAULT_IRQ_HANDLER(NONSEC_WATCHDOG_Handler)
DEFAULT_IRQ_HANDLER(SLOWCLK_Timer_Handler)
DEFAULT_IRQ_HANDLER(TFM_TIMER0_IRQ_Handler)
DEFAULT_IRQ_HANDLER(TIMER1_Handler)
DEFAULT_IRQ_HANDLER(TIMER2_Handler)
DEFAULT_IRQ_HANDLER(MPC_Handler)
DEFAULT_IRQ_HANDLER(PPC_Handler)
DEFAULT_IRQ_HANDLER(MSC_Handler)
DEFAULT_IRQ_HANDLER(BRIDGE_ERROR_Handler)
DEFAULT_IRQ_HANDLER(MGMT_PPU_Handler)
DEFAULT_IRQ_HANDLER(SYS_PPU_Handler)
DEFAULT_IRQ_HANDLER(CPU0_PPU_Handler)
DEFAULT_IRQ_HANDLER(DEBUG_PPU_Handler)
DEFAULT_IRQ_HANDLER(TIMER3_Handler)
DEFAULT_IRQ_HANDLER(CTI_REQ0_IRQHandler)
DEFAULT_IRQ_HANDLER(CTI_REQ1_IRQHandler)

DEFAULT_IRQ_HANDLER(UARTRX0_Handler)
DEFAULT_IRQ_HANDLER(UARTTX0_Handler)
DEFAULT_IRQ_HANDLER(UARTRX1_Handler)
DEFAULT_IRQ_HANDLER(UARTTX1_Handler)
DEFAULT_IRQ_HANDLER(UARTRX2_Handler)
DEFAULT_IRQ_HANDLER(UARTTX2_Handler)
DEFAULT_IRQ_HANDLER(UARTOVF_Handler)
DEFAULT_IRQ_HANDLER(ETHERNET_Handler)
DEFAULT_IRQ_HANDLER(I2S0_Handler)
DEFAULT_IRQ_HANDLER(TOUCH_SCREEN_Handler)
DEFAULT_IRQ_HANDLER(SPI0_Handler)
DEFAULT_IRQ_HANDLER(SPI1_Handler)
DEFAULT_IRQ_HANDLER(SPI2_Handler)
DEFAULT_IRQ_HANDLER(DMA0_ERROR_Handler)
DEFAULT_IRQ_HANDLER(DMA0_TC_Handler)
DEFAULT_IRQ_HANDLER(DMA0_Handler)
DEFAULT_IRQ_HANDLER(DMA1_ERROR_Handler)
DEFAULT_IRQ_HANDLER(DMA1_TC_Handler)
DEFAULT_IRQ_HANDLER(DMA1_Handler)
DEFAULT_IRQ_HANDLER(DMA2_ERROR_Handler)
DEFAULT_IRQ_HANDLER(DMA2_TC_Handler)
DEFAULT_IRQ_HANDLER(DMA2_Handler)
DEFAULT_IRQ_HANDLER(DMA3_ERROR_Handler)
DEFAULT_IRQ_HANDLER(DMA3_TC_Handler)
DEFAULT_IRQ_HANDLER(DMA3_Handler)
DEFAULT_IRQ_HANDLER(GPIO0_Handler)
DEFAULT_IRQ_HANDLER(GPIO1_Handler)
DEFAULT_IRQ_HANDLER(GPIO2_Handler)
DEFAULT_IRQ_HANDLER(GPIO3_Handler)

/*----------------------------------------------------------------------------
  Exception / Interrupt Vector table
 *----------------------------------------------------------------------------*/

#if defined ( __GNUC__ )
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#endif

extern const pFunc __VECTOR_TABLE[496];
       const pFunc __VECTOR_TABLE[496] __VECTOR_TABLE_ATTRIBUTE = {
  (pFunc)(&__MSP_INITIAL_SP),           /*      Initial Stack Pointer */
  Reset_Handler,                    /*      Reset Handler */
  NMI_Handler,                      /* -14: NMI Handler */
  HardFault_Handler,                /* -13: Hard Fault Handler */
  MemManage_Handler,                /* -12: MPU Fault Handler */
  BusFault_Handler,                 /* -11: Bus Fault Handler */
  UsageFault_Handler,               /* -10: Usage Fault Handler */
  SecureFault_Handler,              /*  -9: Secure Fault Handler */
  0,                                /*      Reserved */
  0,                                /*      Reserved */
  0,                                /*      Reserved */
  SVC_Handler,                      /*  -5: SVCall Handler */
  DebugMon_Handler,                 /*  -4: Debug Monitor Handler */
  0,                                /*      Reserved */
  PendSV_Handler,                   /*  -2: PendSV Handler */
  SysTick_Handler,                  /*  -1: SysTick Handler */

  NONSEC_WATCHDOG_RESET_Handler,    /*   0: Non-Secure Watchdog Reset Handler */
  NONSEC_WATCHDOG_Handler,          /*   1: Non-Secure Watchdog Handler */
  SLOWCLK_Timer_Handler,            /*   2: SLOWCLK Timer Handler */
  TFM_TIMER0_IRQ_Handler,                   /*   3: TIMER 0 Handler */
  TIMER1_Handler,                   /*   4: TIMER 1 Handler */
  TIMER2_Handler,                   /*   5: TIMER 2 Handler */
  0,                                /*   6: Reserved */
  0,                                /*   7: Reserved */
  0,                                /*   8: Reserved */
  MPC_Handler,                      /*   9: MPC Combined (Secure) Handler */
  PPC_Handler,                      /*  10: PPC Combined (Secure) Handler */
  MSC_Handler,                      /*  11: MSC Combined (Secure) Handler */
  BRIDGE_ERROR_Handler,             /*  12: Bridge Error (Secure) Handler */
  0,                                /*  13: Reserved */
  MGMT_PPU_Handler,                 /*  14: MGMT PPU Handler */
  SYS_PPU_Handler,                  /*  15: SYS PPU Handler */
  CPU0_PPU_Handler,                 /*  16: CPU0 PPU Handler */
  0,                                /*  17: Reserved */
  0,                                /*  18: Reserved */
  0,                                /*  19: Reserved */
  0,                                /*  20: Reserved */
  0,                                /*  21: Reserved */
  0,                                /*  22: Reserved */
  0,                                /*  23: Reserved */
  0,                                /*  24: Reserved */
  0,                                /*  25: Reserved */
  DEBUG_PPU_Handler,                /*  26: DEBUG PPU Handler */
  TIMER3_Handler,                   /*  27: TIMER 3 Handler */
  CTI_REQ0_IRQHandler,              /*  28: CTI request 0 IRQ Handler */
  CTI_REQ1_IRQHandler,              /*  29: CTI request 1 IRQ Handler */
  0,                                /*  30: Reserved */
  0,                                /*  31: Reserved */

  /* External interrupts */
  UARTRX0_Handler,                  /*  32: UART 0 RX Handler */
  UARTTX0_Handler,                  /*  33: UART 0 TX Handler */
  UARTRX1_Handler,                  /*  34: UART 1 RX Handler */
  UARTTX1_Handler,                  /*  35: UART 1 TX Handler */
  UARTRX2_Handler,                  /*  36: UART 2 RX Handler */
  UARTTX2_Handler,                  /*  37: UART 2 TX Handler */
  0,                                /*  38: Reserved */
  0,                                /*  39: Reserved */
  0,                                /*  40: Reserved */
  0,                                /*  41: Reserved */
  0,                                /*  42: Reserved */
  0,                                /*  43: Reserved */
  0,                                /*  44: Reserved */
  0,                                /*  45: Reserved */
  0,                                /*  46: Reserved */
  UARTOVF_Handler,                  /*  47: UART 0,1,2 Overflow Handler */
  ETHERNET_Handler,                 /*  48: SMSC 91C111 Etherne Interrupt */
  I2S0_Handler,                     /*  49: I2S0 Handler */
  TOUCH_SCREEN_Handler,             /*  50: Touch Screen Interrupt */
  SPI0_Handler,                     /*  51: SPI 0 (SPI Header) Interrupt */
  SPI1_Handler,                     /*  52: SPI 1 (CLCD) Interrupt */
  SPI2_Handler,                     /*  53: SPI 2 (ADC) Interrupt */
  0,                                /*  54: Reserved */
  0,                                /*  55: Reserved */
  DMA0_ERROR_Handler,               /*  56: DMA 0 Error Handler */
  DMA0_TC_Handler,                  /*  57: DMA 0 Terminal Count Handler */
  DMA0_Handler,                     /*  58: DMA 0 Combined Handler */
  DMA1_ERROR_Handler,               /*  59: DMA 1 Error Handler */
  DMA1_TC_Handler,                  /*  60: DMA 1 Terminal Count Handler */
  DMA1_Handler,                     /*  61: DMA 1 Combined Handler */
  DMA2_ERROR_Handler,               /*  62: DMA 2 Error Handler */
  DMA2_TC_Handler,                  /*  63: DMA 2 Terminal Count Handler */
  DMA2_Handler,                     /*  64: DMA 2 Combined Handler */
  DMA3_ERROR_Handler,               /*  65: DMA 3 Error Handler */
  DMA3_TC_Handler,                  /*  66: DMA 3 Terminal Count Handler */
  DMA3_Handler,                     /*  67: DMA 3 Combined Handler */
  GPIO0_Handler,                    /*  68: GPIO 0 Combined Handler */
  GPIO1_Handler,                    /*  69: GPIO 1 Combined Handler */
  GPIO2_Handler,                    /*  70: GPIO 2 Combined Handler */
  GPIO3_Handler,                    /*  71: GPIO 3 Combined Handler */
};

#if defined ( __GNUC__ )
#pragma GCC diagnostic pop
#endif

/*----------------------------------------------------------------------------
  Reset Handler called on controller reset
 *----------------------------------------------------------------------------*/
void Reset_Handler(void)
{
  __set_MSPLIM((uint32_t)(&__MSP_STACK_LIMIT));

  SystemInit();                             /* CMSIS System Initialization */
  __ASM volatile("MRS     R0, control\n"    /* Get control value */
                 "ORR     R0, R0, #1\n"     /* Select switch to unprivilage mode */
                 "ORR     R0, R0, #2\n"     /* Select switch to PSP */
                 "MSR     control, R0\n"    /* Load control register */
                 :
                 :
                 : "r0");
  __PROGRAM_START();                        /* Enter PreMain (C library entry point) */
}
