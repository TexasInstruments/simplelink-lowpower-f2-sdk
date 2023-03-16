/*
 * Copyright (c) 2019-2020 Arm Limited
 *
 * Licensed under the Apache License Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing software
 * distributed under the License is distributed on an "AS IS" BASIS
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __PLATFORM_IRQN_H__
#define __PLATFORM_IRQN_H__

typedef enum _IRQn_Type {
    NonMaskableInt_IRQn            = -14,  /* Non Maskable Interrupt */
    HardFault_IRQn                 = -13,  /* HardFault Interrupt */
    MemoryManagement_IRQn          = -12,  /* Memory Management Interrupt */
    BusFault_IRQn                  = -11,  /* Bus Fault Interrupt */
    UsageFault_IRQn                = -10,  /* Usage Fault Interrupt */
    SecureFault_IRQn               = -9,   /* Secure Fault Interrupt */
    SVCall_IRQn                    = -5,   /* SV Call Interrupt */
    DebugMonitor_IRQn              = -4,   /* Debug Monitor Interrupt */
    PendSV_IRQn                    = -2,   /* Pend SV Interrupt */
    SysTick_IRQn                   = -1,   /* System Tick Interrupt */

    NONSEC_WATCHDOG_RESET_IRQn     = 0,    /* Non-Secure Watchdog Reset
                                            * Interrupt */
    NONSEC_WATCHDOG_IRQn           = 1,    /* Non-Secure Watchdog Interrupt */
    SLOWCLK_Timer_IRQn             = 2,    /* SLOWCLK Timer Interrupt */
    TIMER0_IRQn                    = 3,    /* TIMER 0 Interrupt */
    TIMER1_IRQn                    = 4,    /* TIMER 1 Interrupt */
    TIMER2_IRQn                    = 5,    /* TIMER 2 Interrupt */
    /* Reserved                    = 8:6,     Reserved */
    MPC_IRQn                       = 9,    /* MPC Combined (Secure) Interrupt */
    PPC_IRQn                       = 10,   /* PPC Combined (Secure) Interrupt */
    MSC_IRQn                       = 11,   /* MSC Combined (Secure) Interrput */
    BRIDGE_ERROR_IRQn              = 12,   /* Bridge Error Combined
                                            * (Secure) Interrupt */
    /* Reserved                    = 13       Reserved */
    MGMT_PPU_IRQn                  = 14,   /* MGMT PPU Interrupt */
    SYS_PPU_IRQn                   = 15,   /* SYS PPU Interrupt */
    CPU0_PPU_IRQn                  = 16,   /* CPU0 PPU Interrupt */
    /* Reserved                    = 25:17    Reserved */
    DEBUG_PPU_IRQn                 = 26,   /* DEBUG PPU Interrupt */
    TIMER3_IRQn                    = 27,   /* TIMER 3 Interrupt */
    CTI_REQ0_IRQn                  = 28,   /* CTI request 0 */
    CTI_REQ1_IRQn                  = 29,   /* CTI request 1 */
    /* Reserved                    = 31:30    Reserved */

    /* External interrupts */
    UARTRX0_IRQn                   = 32,   /* UART 0 RX Interrupt */
    UARTTX0_IRQn                   = 33,   /* UART 0 TX Interrupt */
    UARTRX1_IRQn                   = 34,   /* UART 1 RX Interrupt */
    UARTTX1_IRQn                   = 35,   /* UART 1 TX Interrupt */
    UARTRX2_IRQn                   = 36,   /* UART 2 RX Interrupt */
    UARTTX2_IRQn                   = 37,   /* UART 2 TX Interrupt */
    /* Reserved                    = 46:38    Reserved */
    UARTOVF_IRQn                   = 47,   /* UART Overflow Interrupt */
    ETHERNET_IRQn                  = 48,   /* SMSC 91C111 Etherne Interrupt */
    I2S0_IRQn                      = 49,   /* I2S 0 Interrupt */
    TOUCH_SCREEN_IRQn              = 50,   /* Touch Screen Interrupt */
    SPI0_IRQn                      = 51,   /* SPI 0 (SPI Header) Interrupt */
    SPI1_IRQn                      = 52,   /* SPI 1 (CLCD) Interrupt */
    SPI2_IRQn                      = 53,   /* SPI 2 (ADC) Interrupt */
    /* Reserved                    = 55:54    Reserved */
    DMA0_ERROR_IRQn                = 56,   /* DMA 0 Error Interrupt */
    DMA0_TC_IRQn                   = 57,   /* DMA 0 Terminal Count Interrupt */
    DMA0_IRQn                      = 58,   /* DMA 0 Combined Interrupt */
    DMA1_ERROR_IRQn                = 59,   /* DMA 1 Error Interrupt */
    DMA1_TC_IRQn                   = 60,   /* DMA 1 Terminal Count Interrupt */
    DMA1_IRQn                      = 61,   /* DMA 1 Combined Interrupt */
    DMA2_ERROR_IRQn                = 62,   /* DMA 2 Error Interrupt */
    DMA2_TC_IRQn                   = 63,   /* DMA 2 Terminal Count Interrupt */
    DMA2_IRQn                      = 64,   /* DMA 2 Combined Interrupt */
    DMA3_ERROR_IRQn                = 65,   /* DMA 3 Error Interrupt */
    DMA3_TC_IRQn                   = 66,   /* DMA 3 Terminal Count Interrupt */
    DMA3_IRQn                      = 67,   /* DMA 3 Combined Interrupt */
    GPIO0_IRQn                     = 68,   /* GPIO 0 Combined Interrupt */
    GPIO1_IRQn                     = 69,   /* GPIO 1 Combined Interrupt */
    GPIO2_IRQn                     = 70,   /* GPIO 2 Combined Interrupt */
    GPIO3_IRQn                     = 71,   /* GPIO 3 Combined Interrupt */
    /* Reserved                    = 95:72    Reserved */
}IRQn_Type;

#endif  /* __PLATFORM_IRQN_H__ */
