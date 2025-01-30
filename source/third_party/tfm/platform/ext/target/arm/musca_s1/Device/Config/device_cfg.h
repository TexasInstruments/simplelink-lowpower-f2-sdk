/*
 * Copyright (c) 2017-2021 Arm Limited. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __DEVICE_CFG_H__
#define __DEVICE_CFG_H__

/**
 * \file device_cfg.h
 * \brief Configuration file native driver re-targeting
 *
 * \details This file can be used to add native driver specific macro
 *          definitions to select which peripherals are available in the build.
 *
 * This is a default device configuration file with all peripherals enabled.
 */

/* ARM GPIO */
#define GPIO0_CMSDK_S

/* ARM Memory Protection Controller (MPC) SIE 200 */
#define MPC_ISRAM0_S
#define MPC_ISRAM1_S
#define MPC_ISRAM2_S
#define MPC_ISRAM3_S
#define MPC_CODE_SRAM_S
#define MPC_CODE_SRAM_NS
#define MPC_MRAM_S
#define MPC_MRAM_NS

/* ARM Peripheral Protection Controllers (PPC) */
#define AHB_PPC0_S
#define AHB_PPCEXP0_S

#define APB_PPC0_S
#define APB_PPC1_S
#define APB_PPCEXP0_S
#define APB_PPCEXP1_S

/*ARM UART Controller PL011*/
#define UART1_PL011_S
#define UART1_PL011_NS

/* CMSDK Timers */
#define CMSDK_TIMER0_S
#define CMSDK_TIMER0_NS
#define CMSDK_TIMER1_S
#define CMSDK_TIMER1_NS

/* ARM SCC */
#define MUSCA_S1_SCC_S

/* Cache */
#define SSE_200_CACHE_S

#define DEFAULT_UART_CONTROL 0
/* Default UART baud rate */
#define DEFAULT_UART_BAUDRATE 115200

/* Cadence QSPI Flash Controller */
#define QSPI_IP6514E_S

/* MT25QL Flash memory library */
#define MT25QL_S

#endif  /* __DEVICE_CFG_H__ */
