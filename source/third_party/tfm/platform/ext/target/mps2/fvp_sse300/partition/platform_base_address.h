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

/**
 * \file platform_base_address.h
 * \brief This file defines all the peripheral base addresses for Diadem FVP
 *        platform.
 */

#ifndef __PLATFORM_BASE_ADDRESS_H_
#define __PLATFORM_BASE_ADDRESS_H_

/* ======= Defines peripherals memory map addresses ======= */
/* Non-secure memory map addresses */
#define ITCM_BASE_NS                     0x00000000 /* Instruction TCM Non-Secure base address */
#define DTCM_BASE_NS                     0x20000000 /* Data TCM Non-Secure base address */
#define VM0_BASE_NS                      0x21000000 /* Volatile Memory Bank 0 Non-Secure base address */
#define VM1_BASE_NS                      0x21040000 /* Volatile Memory Bank 1 Non-Secure base address */
#define SSRAM2_BASE_NS                   0x28000000 /* SSRAM2 Non-Secure base address */
#define SSRAM3_BASE_NS                   0x28200000 /* SSRAM3 Non-Secure base address */
#define CPU0_PWRCTRL_BASE_NS             0x40012000 /* CPU 0 Power Control Block Non-Secure base address */
#define CPU0_IDENTITY_BASE_NS            0x4001F000 /* CPU 0 Identity Block Non-Secure base address */
#define SSE300_NSACFG_BASE_NS            0x40080000 /* SSE-300 Non-Secure Access Configuration Register Block Non-Secure base address */
#define GPIO0_CMSDK_BASE_NS              0x40100000 /* GPIO0 Non-Secure base address */
#define GPIO1_CMSDK_BASE_NS              0x40101000 /* GPIO1 Non-Secure base address */
#define GPIO2_CMSDK_BASE_NS              0x40102000 /* GPIO2 Non-Secure base address */
#define GPIO3_CMSDK_BASE_NS              0x40103000 /* GPIO3 Non-Secure base address */
#define DMA0_PL080_BASE_NS               0x40110000 /* DMA0 PL080 Non-Secure base address */
#define DMA1_PL080_BASE_NS               0x40111000 /* DMA1 PL080 Non-Secure base address */
#define DMA2_PL080_BASE_NS               0x40112000 /* DMA2 PL080 Non-Secure base address */
#define DMA3_PL080_BASE_NS               0x40113000 /* DMA3 PL080 Non-Secure base address */
#define UART0_CMSDK_BASE_NS              0x40200000 /* UART0 Non-Secure base address */
#define UART1_CMSDK_BASE_NS              0x40201000 /* UART1 Non-Secure base address */
#define UART2_CMSDK_BASE_NS              0x40202000 /* UART2 Non-Secure base address */
#define UART3_CMSDK_BASE_NS              0x40203000 /* Dummy Stub */
#define UART4_CMSDK_BASE_NS              0x40204000 /* Dummy Stub */
#define SPI_0_BASE_NS                    0x40205000 /* Dummy Stub */
#define SPI_1_BASE_NS                    0x40206000 /* PL022 SPI for LCD - Partial Modelled */
#define I2C_0_BASE_NS                    0x40207000 /* SBCon I2C (Touch) - Partial Modelled */
#define I2C_1_BASE_NS                    0x40208000 /* SBCon I2C (Audio Conf) - Dummy Stub */
#define SPI_2_BASE_NS                    0x40209000 /* Dummy Stub */
#define MPS2_IO_SCC_BASE_NS              0x40300000 /* MPS2 SCC IO Non-Secure base address */
#define I2S0_BASE_NS                     0x40301000 /* Dummy Stub */
#define MPS2_IO_FPGAIO_BASE_NS           0x40302000 /* MPS2 FPGA GPIO (System Ctrl + I/O) Non-Secure base address */
#define VGA_BASE_NS                      0x41000000 /* VGA Console and Image Non-Secure base address */
#define SMSC_BASE_NS                     0x42000000 /* SMSC 91C111 Ethernet controller */
#define SYSTIMER0_ARMV8_M_BASE_NS        0x48000000 /* System Timer 0 Non-Secure base address */
#define SYSTIMER1_ARMV8_M_BASE_NS        0x48001000 /* System Timer 1 Non-Secure base address */
#define SYSTIMER2_ARMV8_M_BASE_NS        0x48002000 /* System Timer 0 Non-Secure base address */
#define SYSTIMER3_ARMV8_M_BASE_NS        0x48003000 /* System Timer 1 Non-Secure base address */
#define SSE300_SYSINFO_BASE_NS           0x48020000 /* SSE-300 System info Block Non-Secure base address */
#define SLOWCLK_TIMER_CMSDK_BASE_NS      0x4802F000 /* CMSDK based SLOWCLK Timer Non-Secure base address */
#define SYSWDOG_ARMV8_M_CNTRL_BASE_NS    0x48040000 /* Non-Secure Watchdog Timer control frame Non-Secure base address */
#define SYSWDOG_ARMV8_M_REFRESH_BASE_NS  0x48041000 /* Non-Secure Watchdog Timer refresh frame Non-Secure base address */

/* Secure memory map addresses */
#define ITCM_BASE_S                      0x10000000 /* Instruction TCM Secure base address */
#define DTCM_BASE_S                      0x30000000 /* Data TCM Secure base address */
#define VM0_BASE_S                       0x31000000 /* Volatile Memory Bank 0 Secure base address */
#define VM1_BASE_S                       0x31040000 /* Volatile Memory Bank 1 Secure base address */
#define SSRAM2_BASE_S                    0x38000000 /* SSRAM2 Secure base address */
#define SSRAM3_BASE_S                    0x38200000 /* SSRAM3 Secure base address */
#define CPU0_SECCTRL_BASE_S              0x50011000 /* CPU 0 Local Security Control Block Secure base address */
#define CPU0_PWRCTRL_BASE_S              0x50012000 /* CPU 0 Power Control Block Secure base address */
#define CPU0_IDENTITY_BASE_S             0x5001F000 /* CPU 0 Identity Block Secure base address */
#define SSE300_SACFG_BASE_S              0x50080000 /* SSE-300 Secure Access Configuration Register Secure base address */
#define MPC_VM0_BASE_S                   0x50083000 /* VM0 Memory Protection Controller Secure base address */
#define MPC_VM1_BASE_S                   0x50084000 /* VM1 Memory Protection Controller Secure base address */
#define GPIO0_CMSDK_BASE_S               0x50100000 /* GPIO0 Secure base address */
#define GPIO1_CMSDK_BASE_S               0x50101000 /* GPIO1 Secure base address */
#define GPIO2_CMSDK_BASE_S               0x50102000 /* GPIO2 Secure base address */
#define GPIO3_CMSDK_BASE_S               0x50103000 /* GPIO3 Secure base address */
#define DMA0_PL080_BASE_S                0x50110000 /* DMA0 PL080 Secure base address */
#define DMA1_PL080_BASE_S                0x50111000 /* DMA1 PL080 Secure base address */
#define DMA2_PL080_BASE_S                0x50112000 /* DMA2 PL080 Secure base address */
#define DMA3_PL080_BASE_S                0x50113000 /* DMA3 PL080 Secure base address */
#define UART0_CMSDK_BASE_S               0x50200000 /* UART0 Secure base address */
#define UART1_CMSDK_BASE_S               0x50201000 /* UART1 Secure base address */
#define UART2_CMSDK_BASE_S               0x50202000 /* UART2 Secure base address */
#define UART3_CMSDK_BASE_S               0x50203000 /* Dummy Stub */
#define UART4_CMSDK_BASE_S               0x50204000 /* Dummy Stub */
#define SPI_0_BASE_S                     0x50205000 /* Dummy Stub */
#define SPI_1_BASE_S                     0x50206000 /* PL022 SPI for LCD - Partial Modelled */
#define I2C_0_BASE_S                     0x50207000 /* SBCon I2C (Touch) - Partial Modelled */
#define I2C_1_BASE_S                     0x50208000 /* SBCon I2C (Audio Conf) - Dummy Stub */
#define SPI_2_BASE_S                     0x50209000 /* Dummy Stub */
#define MPS2_IO_SCC_BASE_S               0x50300000 /* MPS2 SCC IO Secure base address */
#define I2S_FPGA_BASE_S                  0x50301000 /* Dummy Stub */
#define MPS2_IO_FPGAIO_BASE_S            0x50302000 /* MPS2 FPGA GPIO (System Ctrl + I/O) Non-Secure base address */
#define VGA_BASE_S                       0x51000000 /* VGA Console and Image Non-Secure base address */
#define SMSC_BASE_S                      0x52000000 /* SMSC 91C111 Ethernet controller */
#define SYSTIMER0_ARMV8_M_BASE_S         0x58000000 /* System Timer 0 Secure base address */
#define SYSTIMER1_ARMV8_M_BASE_S         0x58001000 /* System Timer 1 Secure base address */
#define SYSTIMER2_ARMV8_M_BASE_S         0x58002000 /* System Timer 0 Secure base address */
#define SYSTIMER3_ARMV8_M_BASE_S         0x58003000 /* System Timer 1 Secure base address */
#define MPC_SSRAM2_BASE_S                0x58008000 /* SSRAM2 Memory Protection Controller Secure base address */
#define MPC_SSRAM3_BASE_S                0x58009000 /* SSRAM3 Memory Protection Controller Secure base address */
#define SSE300_SYSINFO_BASE_S            0x58020000 /* SSE-300 System info Block Secure base address */
#define SSE300_SYSCTRL_BASE_S            0x58021000 /* SSE-300 System control Block Secure base address */
#define SSE300_SYSPPU_BASE_S             0x58022000 /* SSE-300 System Power Policy Unit Secure base address */
#define SSE300_CPU0PPU_BASE_S            0x58023000 /* SSE-300 CPU 0 Power Policy Unit Secure base address */
#define SSE300_MGMTPPU_BASE_S            0x58028000 /* SSE-300 Management Power Policy Unit Secure base address */
#define SSE300_DBGPPU_BASE_S             0x58029000 /* SSE-300 Debug Power Policy Unit Secure base address */
#define SLOWCLK_WDOG_CMSDK_BASE_S        0x5802E000 /* CMSDK based SLOWCLK Watchdog Secure base address */
#define SLOWCLK_TIMER_CMSDK_BASE_S       0x5802F000 /* CMSDK based SLOWCLK Timer Secure base address */
#define SYSWDOG_ARMV8_M_CNTRL_BASE_S     0x58040000 /* Secure Watchdog Timer control frame Secure base address */
#define SYSWDOG_ARMV8_M_REFRESH_BASE_S   0x58041000 /* Secure Watchdog Timer refresh frame Secure base address */

/* Memory map addresses exempt from memory attribution by both the SAU and IDAU */
#define SSE300_EWIC_BASE                 0xE0047000 /* External Wakeup Interrupt Controller
                                                     * Access from Non-secure software is only allowed
                                                     * if AIRCR.BFHFNMINS is set to 1 */

/* Memory size definitions */
#define ITCM_SIZE    (0x00040000) /* 256 KiB */
#define DTCM_SIZE    (0x00040000) /* 256 KiB */
#define VM0_SIZE     (0x00040000) /* 256 KiB */
#define VM1_SIZE     (0x00040000) /* 256 KiB */
#define SSRAM2_SIZE  (0x00200000) /* 2 MB */
#define SSRAM3_SIZE  (0x00200000) /* 2 MB */

/* VM0 -- 256 kB*/
#define MPC_VM0_RANGE_BASE_NS   (VM0_BASE_NS)
#define MPC_VM0_RANGE_LIMIT_NS  (VM0_BASE_NS + VM0_SIZE-1)
#define MPC_VM0_RANGE_BASE_S    (VM0_BASE_S)
#define MPC_VM0_RANGE_LIMIT_S   (VM0_BASE_S + VM0_SIZE-1)

/* VM1 -- 256 kB*/
#define MPC_VM1_RANGE_BASE_NS   (VM1_BASE_NS)
#define MPC_VM1_RANGE_LIMIT_NS  (VM1_BASE_NS + VM1_SIZE-1)
#define MPC_VM1_RANGE_BASE_S    (VM1_BASE_S)
#define MPC_VM1_RANGE_LIMIT_S   (VM1_BASE_S + VM1_SIZE-1)

/* SSRAM2 -- 2 MB*/
#define MPC_SSRAM2_RANGE_BASE_NS   (SSRAM2_BASE_NS)
#define MPC_SSRAM2_RANGE_LIMIT_NS  (SSRAM2_BASE_NS + SSRAM2_SIZE-1)
#define MPC_SSRAM2_RANGE_BASE_S    (SSRAM2_BASE_S)
#define MPC_SSRAM2_RANGE_LIMIT_S   (SSRAM2_BASE_S + SSRAM2_SIZE-1)

/* SSRAM3 -- 2 MB*/
#define MPC_SSRAM3_RANGE_BASE_NS   (SSRAM3_BASE_NS)
#define MPC_SSRAM3_RANGE_LIMIT_NS  (SSRAM3_BASE_NS + SSRAM3_SIZE-1)
#define MPC_SSRAM3_RANGE_BASE_S    (SSRAM3_BASE_S)
#define MPC_SSRAM3_RANGE_LIMIT_S   (SSRAM3_BASE_S + SSRAM3_SIZE-1)

#endif  /* __PLATFORM_BASE_ADDRESS_H_ */
