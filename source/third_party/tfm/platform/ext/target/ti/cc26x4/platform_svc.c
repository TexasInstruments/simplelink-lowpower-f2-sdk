/*
 * Copyright (c) 2023, Texas Instruments Incorporated. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "cmsis.h"
//#include "platform_svc_numbers.h"
#include "target_cfg.h"
#include "tfm_hal_platform.h"
#include "tfm_platform_system.h"

int32_t platform_svc_handlers(uint8_t svc_num, uint32_t *svc_args, uint32_t lr)
{
    (void) lr;

    int32_t retval;

    switch (svc_num) {
        default:
            retval = PSA_ERROR_GENERIC_ERROR;
            break;
    }

    return retval;
}
