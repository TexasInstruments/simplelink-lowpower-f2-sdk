/*
 *  Copyright (C) 2017-2023, Texas Instruments Incorporated, All Rights Reserved
 *  SPDX-License-Identifier: Apache-2.0
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may
 *  not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */
#ifndef ENTROPY_ALT_H
#define ENTROPY_ALT_H

#include "mbedtls/build_info.h"

#if defined(MBEDTLS_ENTROPY_HARDWARE_ALT)

#ifdef __cplusplus
extern "C" {
#endif

#if (DeviceFamily_PARENT == DeviceFamily_PARENT_CC13X4_CC26X3_CC26X4)
    #include <ti/drivers/trng/TRNGCC26XX.h>
    typedef TRNGCC26XX_Object TRNG_Object;
    typedef TRNGCC26XX_HWAttrs TRNG_HWAttrs;
#else
    #error "No valid DeviceFamily found for the TRNG alternate implementation!"
#endif

/**
 * \brief           Entropy poll callback for platform hardware source
 *
 * \note            This must accept NULL as its first argument.
 */
int mbedtls_hardware_poll(void *data, unsigned char *output, size_t len, size_t *olen);

#ifdef __cplusplus
}
#endif

#endif /* MBEDTLS_ENTROPY_HARDWARE_ALT */
#endif /* ENTROPY_ALT_H */