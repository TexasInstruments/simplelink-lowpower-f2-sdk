/*
 * Copyright (c) 2021-2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "cmsis.h"
#include "common_target_cfg.h"
#include "tfm_hal_platform.h"

void tfm_hal_system_reset(void)
{
    __disable_irq();
    mpc_revert_non_secure_to_secure_cfg();

    NVIC_SystemReset();
}

void tfm_hal_system_halt(void)
{
    /*
     * Disable IRQs to stop all threads, not just the thread that
     * halted the system.
     */
    __disable_irq();

    /*
     * Enter sleep to reduce power consumption and do it in a loop in
     * case a signal wakes up the CPU.
     */
    while (1) {
        __WFE();
    }
}
