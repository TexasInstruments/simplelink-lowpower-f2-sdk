/*
 * Copyright (c) 2022, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <ti/drivers/AESCTRDRBG.h>
#include <ti/drivers/AESCTR.h>
#include <ti/drivers/aesctrdrbg/AESCTRDRBGX4_ns.h>
#include <ti/drivers/aesctrdrbg/AESCTRDRBGX4_s.h>
#include <ti/drivers/crypto/CryptoCC26X4_ns.h>
#include <ti/drivers/Power.h>
#include <ti/drivers/power/PowerCC26XX.h>

#include <ti/drivers/dpl/DebugP.h>
#include <ti/drivers/dpl/HwiP.h>

/* Static globals */
static bool isInitialized = false;

static struct psa_invec invecs[1];
static struct psa_outvec outvecs[1];

/*
 *  ======== AESCTRDRBG_init ========
 */
void AESCTRDRBG_init(void)
{
    if (!isInitialized)
    {
        /* Initialize CryptoPSA semaphores */
        CryptoPSACC26X4_init();

        isInitialized = true;
    }
}

/*
 *  ======== AESCTRDRBG_open ========
 */
AESCTRDRBG_Handle AESCTRDRBG_open(uint_least8_t index, const AESCTRDRBG_Params *params)
{
    AESCTRDRBG_Handle handle = NULL;
    AESCTRDRBG_s_OpenMsg openMsg;
    uintptr_t key;

    key = HwiP_disable();

    if (!isInitialized)
    {
        HwiP_restore(key);
        return NULL;
    }

    HwiP_restore(key);

    /*
     * If params are NULL, use defaults. Secure driver can also set this, but
     * NS driver needs to store the return behavior.
     */
    if (params == NULL)
    {
        params = (AESCTRDRBG_Params *)&AESCTRDRBG_defaultParams;
    }

    /* Setup interface for input parameters */
    openMsg.index  = index;
    openMsg.params = params;
    invecs[0].base = &openMsg;
    invecs[0].len  = sizeof(openMsg);

    /* Setup interface for return value */
    outvecs[0].base = &handle;
    outvecs[0].len  = sizeof(handle);

    /*
     * Set power dependency before opening driver since the DRBG reseed will
     * take place immediately.
     */
    (void)Power_setDependency(PowerCC26XX_PERIPH_CRYPTO);

    /*
     * PSA call to secure driver:
     *
     * Return value can be ignored since handle (in outvecs) is initialized to NULL
     * and will only be updated if the PSA call is successful.
     */
    (void)CryptoPSACC26X4_call(AESCTRDRBG_S_MSG_TYPE_OPEN, invecs, outvecs);

    return handle;
}

/*
 *  ======== AESCTRDRBG_construct ========
 */
AESCTRDRBG_Handle AESCTRDRBG_construct(AESCTRDRBG_Config *config, const AESCTRDRBG_Params *params)
{
    AESCTRDRBG_Handle handle = NULL;
    AESCTRDRBG_s_ConstructMsg constructMsg;
    uintptr_t key;

    key = HwiP_disable();

    if (!isInitialized)
    {
        HwiP_restore(key);
        return NULL;
    }

    HwiP_restore(key);

    /*
     * If params are NULL, use defaults. Secure driver can also set this, but
     * NS driver needs to store the return behavior.
     */
    if (params == NULL)
    {
        params = (AESCTRDRBG_Params *)&AESCTRDRBG_defaultParams;
    }

    /* Setup interface for input parameters */
    constructMsg.config = config;
    constructMsg.params = params;
    invecs[0].base      = &constructMsg;
    invecs[0].len       = sizeof(constructMsg);

    /* Setup interface for return value */
    outvecs[0].base = &handle;
    outvecs[0].len  = sizeof(handle);

    /*
     * Set power dependency before constructing driver instance since the DRBG
     * reseed will take place immediately.
     */
    (void)Power_setDependency(PowerCC26XX_PERIPH_CRYPTO);

    /*
     * PSA call to secure driver:
     *
     * Return value can be ignored since handle (in outvecs) is initialized to NULL
     * and will only be updated if the PSA call is successful.
     */
    (void)CryptoPSACC26X4_call(AESCTRDRBG_S_MSG_TYPE_CONSTRUCT, invecs, outvecs);

    return handle;
}

/*
 *  ======== AESCTRDRBG_close ========
 */
void AESCTRDRBG_close(AESCTRDRBG_Handle handle)
{
    AESCTRDRBG_s_CloseMsg closeMsg;

    DebugP_assert(handle);

    /* Setup interface for input parameter */
    closeMsg.handle = handle;
    invecs[0].base  = &closeMsg;
    invecs[0].len   = sizeof(closeMsg);

    /* Setup interface for null return value */
    outvecs[0].base = NULL;
    outvecs[0].len  = 0;

    /* PSA call to secure driver */
    (void)CryptoPSACC26X4_call(AESCTRDRBG_S_MSG_TYPE_CLOSE, invecs, outvecs);

    /* Release power dependency */
    (void)Power_releaseDependency(PowerCC26XX_PERIPH_CRYPTO);
}

/*
 *  ======== AESCTRDRBG_getBytes ========
 */
int_fast16_t AESCTRDRBG_getBytes(AESCTRDRBG_Handle handle, CryptoKey *randomBytes)
{
    return AESCTRDRBG_generateKey(handle, randomBytes);
}

/*
 *  ======== AESCTRDRBG_generateKey ========
 */
int_fast16_t AESCTRDRBG_generateKey(AESCTRDRBG_Handle handle, CryptoKey *randomKey)
{
    AESCTRDRBG_s_GenerateKeyMsg generateMsg;
    int_fast16_t result = AESCTRDRBG_STATUS_ERROR;

    /* Setup interface for input parameters */
    generateMsg.handle    = handle;
    generateMsg.randomKey = randomKey;
    invecs[0].base        = &generateMsg;
    invecs[0].len         = sizeof(generateMsg);

    /* Setup interface for return value */
    outvecs[0].base = &result;
    outvecs[0].len  = sizeof(result);

    /*
     * PSA call to secure driver:
     *
     * Return value can be ignored since result (in outvecs) is initialized to AESCTRDRBG_STATUS_ERROR
     * and will only be updated if the PSA call is successful.
     */
    (void)CryptoPSACC26X4_call(AESCTRDRBG_S_MSG_TYPE_GENERATE_KEY, invecs, outvecs);

    return (result);
}

/*
 *  ======== AESCTRDRBG_getRandomBytes ========
 */
int_fast16_t AESCTRDRBG_getRandomBytes(AESCTRDRBG_Handle handle, void *randomBytes, size_t randomBytesSize)
{
    AESCTRDRBG_s_GetRandomBytesMsg getRandomMsg;
    int_fast16_t result = AESCTRDRBG_STATUS_ERROR;

    /* Setup interface for input parameters */
    getRandomMsg.handle          = handle;
    getRandomMsg.randomBytes     = randomBytes;
    getRandomMsg.randomBytesSize = randomBytesSize;
    invecs[0].base               = &getRandomMsg;
    invecs[0].len                = sizeof(getRandomMsg);

    /* Setup interface for return value */
    outvecs[0].base = &result;
    outvecs[0].len  = sizeof(result);

    /*
     * PSA call to secure driver:
     *
     * Return value can be ignored since result (in outvecs) is initialized to AESCTRDRBG_STATUS_ERROR
     * and will only be updated if the PSA call is successful.
     */
    (void)CryptoPSACC26X4_call(AESCTRDRBG_S_MSG_TYPE_GET_RANDOM_BYTES, invecs, outvecs);

    return (result);
}

/*
 *  ======== AESCTRDRBG_reseed ========
 */
int_fast16_t AESCTRDRBG_reseed(AESCTRDRBG_Handle handle,
                               const void *seed,
                               const void *additionalData,
                               size_t additionalDataLength)
{
    AESCTRDRBG_s_ReseedMsg reseedMsg;
    int_fast16_t result = AESCTRDRBG_STATUS_ERROR;

    /* Setup interface for input parameters */
    reseedMsg.handle               = handle;
    reseedMsg.seed                 = seed;
    reseedMsg.additionalData       = additionalData;
    reseedMsg.additionalDataLength = additionalDataLength;
    invecs[0].base                 = &reseedMsg;
    invecs[0].len                  = sizeof(reseedMsg);

    /* Setup interface for return value */
    outvecs[0].base = &result;
    outvecs[0].len  = sizeof(result);

    /*
     * PSA call to secure driver:
     *
     * Return value can be ignored since result (in outvecs) is initialized to AESCTRDRBG_STATUS_ERROR
     * and will only be updated if the PSA call is successful.
     */
    (void)CryptoPSACC26X4_call(AESCTRDRBG_S_MSG_TYPE_RESEED, invecs, outvecs);

    return (result);
}
