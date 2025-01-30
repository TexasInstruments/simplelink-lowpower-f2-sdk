/*
 * Copyright (c) 2016-2018 ARM Limited
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

#include "platform_retarget_dev.h"
#include "platform_retarget.h"
#include "system_core_init.h"

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

