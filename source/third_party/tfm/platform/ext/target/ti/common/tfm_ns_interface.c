/*
 * Copyright (c) 2022, Texas Instruments Incorporated. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */
#include <stdint.h>

#include <third_party/tfm/interface/include/tfm_api.h>
#include <third_party/tfm/interface/include/tfm_ns_interface.h>

int32_t tfm_ns_interface_dispatch(veneer_fn fn, uint32_t arg0, uint32_t arg1,
                                  uint32_t arg2, uint32_t arg3)
{
    /*
     * NS clients handle mutual exclusion for access to secure side on their own
     * so this dispatch fxn is merely a wrapper for the veneer fxn.
     */

    return fn(arg0, arg1, arg2, arg3);
}

enum tfm_status_e tfm_ns_interface_init(void)
{
    return TFM_SUCCESS;
}
