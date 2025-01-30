/*
 * Copyright (c) 2016-2022 Arm Limited
 * Copyright (c) 2023 Nuvoton Technology Corp.
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
 * \file platform_retarget_dev.c
 * \brief This file defines exports the structures based on the peripheral
 * definitions from device_cfg.h.
 * This retarget file is meant to be used as a helper for baremetal
 * applications and/or as an example of how to configure the generic
 * driver structures.
 */

#include "NuMicro.h"
#include "platform_retarget_dev.h"
#include "platform_retarget.h"
#include "system_core_init.h"
#include "tfm_plat_defs.h"

/* ARM UART driver structures */
#ifdef ARM_UART0_S
static const struct arm_uart_dev_cfg_t ARM_UART0_DEV_CFG_S = {
    .base = UART0_BASE_S,
    .default_baudrate = DEFAULT_UART_BAUDRATE};
static struct arm_uart_dev_data_t ARM_UART0_DEV_DATA_S = {
    .state = 0,
    .system_clk = 0,
    .baudrate = 0};
struct arm_uart_dev_t ARM_UART0_DEV_S = {&(ARM_UART0_DEV_CFG_S),
                                         &(ARM_UART0_DEV_DATA_S)};
#endif
#ifdef ARM_UART0_NS
static const struct arm_uart_dev_cfg_t ARM_UART0_DEV_CFG_NS = {
    .base = UART0_BASE_NS,
    .default_baudrate = DEFAULT_UART_BAUDRATE};
static struct arm_uart_dev_data_t ARM_UART0_DEV_DATA_NS = {
    .state = 0,
    .system_clk = 0,
    .baudrate = 0};
struct arm_uart_dev_t ARM_UART0_DEV_NS = {&(ARM_UART0_DEV_CFG_NS),
                                          &(ARM_UART0_DEV_DATA_NS)};
#endif

/* CMSDK Timer driver structures */

/* CMSDK Timer driver structures */
#ifdef CMSDK_TIMER0_S
static const struct cmsdk_timer_dev_cfg_t CMSDK_TIMER0_DEV_CFG_S
#ifdef TFM_PARTITION_SLIH_TEST
    TFM_LINK_SET_RO_IN_PARTITION_SECTION("TFM_SP_SLIH_TEST", "APP-ROT")
#elif defined(TFM_PARTITION_FLIH_TEST)
    TFM_LINK_SET_RO_IN_PARTITION_SECTION("TFM_SP_FLIH_TEST", "APP-ROT")
#endif
    = {.base = CMSDK_TIMER0_BASE_S};
static struct cmsdk_timer_dev_data_t CMSDK_TIMER0_DEV_DATA_S
#ifdef TFM_PARTITION_SLIH_TEST
    TFM_LINK_SET_RW_IN_PARTITION_SECTION("TFM_SP_SLIH_TEST", "APP-ROT")
#elif defined(TFM_PARTITION_FLIH_TEST)
    TFM_LINK_SET_RW_IN_PARTITION_SECTION("TFM_SP_FLIH_TEST", "APP-ROT")
#endif
    = {.is_initialized = 0};

struct cmsdk_timer_dev_t CMSDK_TIMER0_DEV_S
#ifdef TFM_PARTITION_SLIH_TEST
    TFM_LINK_SET_RW_IN_PARTITION_SECTION("TFM_SP_SLIH_TEST", "APP-ROT")
#elif defined(TFM_PARTITION_FLIH_TEST)
    TFM_LINK_SET_RW_IN_PARTITION_SECTION("TFM_SP_FLIH_TEST", "APP-ROT")
#endif
       = {&(CMSDK_TIMER0_DEV_CFG_S), &(CMSDK_TIMER0_DEV_DATA_S)};
#endif
#ifdef CMSDK_TIMER0_NS
static const struct cmsdk_timer_dev_cfg_t CMSDK_TIMER0_DEV_CFG_NS = {
    .base = CMSDK_TIMER0_BASE_NS};
static struct cmsdk_timer_dev_data_t CMSDK_TIMER0_DEV_DATA_NS = {
    .is_initialized = 0};
struct cmsdk_timer_dev_t CMSDK_TIMER0_DEV_NS = {&(CMSDK_TIMER0_DEV_CFG_NS),
                                                &(CMSDK_TIMER0_DEV_DATA_NS)};
#endif

#ifdef CMSDK_TIMER1_S
static const struct cmsdk_timer_dev_cfg_t CMSDK_TIMER1_DEV_CFG_S = {
    .base = CMSDK_TIMER1_BASE_S};
static struct cmsdk_timer_dev_data_t CMSDK_TIMER1_DEV_DATA_S = {
    .is_initialized = 0};
struct cmsdk_timer_dev_t CMSDK_TIMER1_DEV_S = {&(CMSDK_TIMER1_DEV_CFG_S),
                                               &(CMSDK_TIMER1_DEV_DATA_S)};
#endif
#ifdef CMSDK_TIMER1_NS
static const struct cmsdk_timer_dev_cfg_t CMSDK_TIMER1_DEV_CFG_NS = {
    .base = CMSDK_TIMER1_BASE_NS};
static struct cmsdk_timer_dev_data_t CMSDK_TIMER1_DEV_DATA_NS = {
    .is_initialized = 0};
struct cmsdk_timer_dev_t CMSDK_TIMER1_DEV_NS = {&(CMSDK_TIMER1_DEV_CFG_NS),
                                                &(CMSDK_TIMER1_DEV_DATA_NS)};
#endif

