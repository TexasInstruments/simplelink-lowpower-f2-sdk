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
#include <stdlib.h>
#include <time.h>
#include <mbedtls/entropy.h>
#include <entropy_alt.h>

#if defined(MBEDTLS_ENTROPY_HARDWARE_ALT)
#include <ti/drivers/TRNG.h>
static const TRNG_HWAttrs defaultTrngHwAttrs = {0};

static bool trngInitialized = false;

int mbedtls_hardware_poll(void *data, unsigned char *output, size_t len, size_t *olen)
{
    int_fast16_t status;
    unsigned int seed;
    TRNG_Handle handle;
    TRNG_Config config;
    TRNG_Object object;
    TRNG_Params params;

    if ((output == NULL) || (len == NULL) || (olen == NULL))
    {
        return MBEDTLS_ERR_ENTROPY_SOURCE_FAILED;
    }

    if (trngInitialized == false)
    {
        TRNG_init();
        trngInitialized = true;
    }

    TRNG_Params_init(&params);
    params.returnBehavior = TRNG_RETURN_BEHAVIOR_POLLING;

    /* Zeroise the dynamic object before it is used by TRNG_construct */
    memset(&object, 0x00, sizeof(object));

    config.object  = &object;
    config.hwAttrs = &defaultTrngHwAttrs;

    handle = TRNG_construct(&config, &params);

    if (handle == NULL)
    {
        return MBEDTLS_ERR_ENTROPY_SOURCE_FAILED;
    }

    status = TRNG_getRandomBytes(handle, output, len);

    if (status == TRNG_STATUS_SUCCESS)
    {
        *olen = len;
    }
    else
    {
        status = MBEDTLS_ERR_ENTROPY_SOURCE_FAILED;
    }

    TRNG_close(handle);

    return (int)status;
}

#endif /* MBEDTLS_ENTROPY_HARDWARE_ALT */
