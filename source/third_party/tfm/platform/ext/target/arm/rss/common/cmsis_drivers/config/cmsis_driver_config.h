/*
 * Copyright (c) 2019-2023 Arm Limited. All rights reserved.
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

#ifndef __CMSIS_DRIVER_CONFIG_H__
#define __CMSIS_DRIVER_CONFIG_H__

#include "cmsis.h"
#include "device_definition.h"
#include "RTE_Device.h"
#include "host_cmsis_driver_config.h"
#ifdef RSS_HAS_EXPANSION_PERIPHERALS
#include "rss_expansion_cmsis_driver_config.h"
#endif /* RSS_HAS_EXPANSION_PERIPHERALS */


#ifdef RSS_DEBUG_UART
#define UART0_DEV                    UART0_CMSDK_DEV_NS
#endif /* RSS_DEBUG_UART */

#define MPC_VM0_DEV                  MPC_VM0_DEV_S
#define MPC_VM1_DEV                  MPC_VM1_DEV_S
#ifdef RSS_XIP
#define MPC_SIC_DEV                  MPC_SIC_DEV_S
#endif /* RSS_XIP */

#define PPC_RSS_MAIN0_DEV            PPC_RSS_MAIN0_DEV_S
#define PPC_RSS_MAIN_EXP0_DEV        PPC_RSS_MAIN_EXP0_DEV_S
#define PPC_RSS_MAIN_EXP1_DEV        PPC_RSS_MAIN_EXP1_DEV_S
#define PPC_RSS_MAIN_EXP2_DEV        PPC_RSS_MAIN_EXP2_DEV_S
#define PPC_RSS_MAIN_EXP3_DEV        PPC_RSS_MAIN_EXP3_DEV_S
#define PPC_RSS_PERIPH0_DEV          PPC_RSS_PERIPH0_DEV_S
#define PPC_RSS_PERIPH1_DEV          PPC_RSS_PERIPH1_DEV_S
#define PPC_RSS_PERIPH_EXP0_DEV      PPC_RSS_PERIPH_EXP0_DEV_S
#define PPC_RSS_PERIPH_EXP1_DEV      PPC_RSS_PERIPH_EXP1_DEV_S
#define PPC_RSS_PERIPH_EXP2_DEV      PPC_RSS_PERIPH_EXP2_DEV_S
#define PPC_RSS_PERIPH_EXP3_DEV      PPC_RSS_PERIPH_EXP3_DEV_S

#endif  /* __CMSIS_DRIVER_CONFIG_H__ */
