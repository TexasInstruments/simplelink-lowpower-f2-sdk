/*
 * Copyright (c) 2018-2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <stdio.h>
#include "cmsis.h"
#include "target_cfg.h"
#include "utilities.h"
/* "exception_info.h" must be the last include because of the IAR pragma */
#include "exception_info.h"

void C_SCU_Handler(void)
{
    ERROR_MSG("Platform Exception: secure violation fault!!!");

    tfm_core_panic();
}

__attribute__((naked)) void SCU_IRQHandler(void)
{
    EXCEPTION_INFO(EXCEPTION_TYPE_PLATFORM);

    __ASM volatile(
        "BL        C_SCU_Handler           \n"
        "B         .                       \n"
    );
}

void C_MPC_Handler(void)
{
    /* Print fault message and block execution */
    ERROR_MSG("Platform Exception: MPC fault!!!");

    tfm_core_panic();
}

__attribute__((naked)) void MPC_Handler(void)
{
    EXCEPTION_INFO(EXCEPTION_TYPE_PLATFORM);

    __ASM volatile(
        "BL        C_MPC_Handler           \n"
        "B         .                       \n"
    );
}

void C_PPC_Handler(void)
{
    /* Print fault message and block execution */
    ERROR_MSG("Platform Exception: PPC fault!!!");

    tfm_core_panic();
}

__attribute__((naked)) void PPC_Handler(void)
{
    EXCEPTION_INFO(EXCEPTION_TYPE_PLATFORM);

    __ASM volatile(
        "BL        C_PPC_Handler           \n"
        "B         .                       \n"
    );
}
