/*
 * Copyright (c) 2018-2019, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "platform/include/tfm_platform_system.h"
#include "cmsis.h"

#include "ti/devices/cc27xx/driverlib/hapi.h"

void tfm_platform_hal_system_reset(void)
{
    /* Cold reset of entire chip causing boot code to run again */
    HapiResetDevice();
}

enum tfm_platform_err_t tfm_platform_hal_ioctl(tfm_platform_ioctl_req_t request,
                                               psa_invec  *in_vec,
                                               psa_outvec *out_vec)
{
    (void)request;
    (void)in_vec;
    (void)out_vec;

    /* Not needed for this platform */
    return TFM_PLATFORM_ERR_NOT_SUPPORTED;
}

