/*
 * Copyright (c) 2023, Texas Instruments Incorporated
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

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <ti/drivers/ansix936kdf/ANSIX936KDFXX.h>
#include <ti/drivers/ANSIX936KDF.h>
#include <ti/drivers/SHA2.h>
#include <ti/drivers/utils/Math.h>

#include <ti/drivers/dpl/HwiP.h>

#if (defined(__IAR_SYSTEMS_ICC__) || defined(__TI_COMPILER_VERSION__))
    #include <arm_acle.h>
    #define BSWAP32 __rev
#else
    #define BSWAP32 __builtin_bswap32
#endif

/* Static globals */
static bool ANSIX936KDF_isInitialized = false;

/* Forward declarations */
static ANSIX936KDFXX_Object *ANSIX936KDFXX_getObject(ANSIX936KDF_Handle handle);

/*
 *  ======== ANSIX936KDFXX_getObject ========
 */
static inline ANSIX936KDFXX_Object *ANSIX936KDFXX_getObject(ANSIX936KDF_Handle handle)
{
    return (ANSIX936KDFXX_Object *)handle->object;
}

/*
 *  ======== ANSIX936KDF_init ========
 */
void ANSIX936KDF_init(void)
{
    SHA2_init();

    ANSIX936KDF_isInitialized = true;
}

/*
 *  ======== ANSIX936KDF_construct ========
 */
ANSIX936KDF_Handle ANSIX936KDF_construct(ANSIX936KDF_Config *config, const ANSIX936KDF_Params *params)
{
    ANSIX936KDF_Handle handle            = (ANSIX936KDF_Config *)config;
    ANSIX936KDFXX_Object *object         = ANSIX936KDFXX_getObject(handle);
    const ANSIX936KDFXX_HWAttrs *hwAttrs = config->hwAttrs;
    SHA2_Params sha2Params;
    uintptr_t key;

    key = HwiP_disable();

    if (!ANSIX936KDF_isInitialized || object->isOpen)
    {
        HwiP_restore(key);
        return NULL;
    }

    object->isOpen = true;

    HwiP_restore(key);

    if (params == NULL)
    {
        params = &ANSIX936KDF_defaultParams;
    }

    /* Zero out the SHA2 object to ensure SHA2_construct() will not fail */
    memset(&object->sha2Object, 0, sizeof(object->sha2Object));
    object->sha2Config.object  = &object->sha2Object;
    object->sha2Config.hwAttrs = &hwAttrs->sha2HWAttrs;

    /* Initialize SHA2 params - default hash type is SHA2_HASH_TYPE_256 */
    SHA2_Params_init(&sha2Params);
    sha2Params.returnBehavior = (SHA2_ReturnBehavior)params->returnBehavior;

    object->sha2Handle = SHA2_construct(&object->sha2Config, &sha2Params);

    if (object->sha2Handle == NULL)
    {
        object->isOpen = false;

        handle = NULL;
    }

    return handle;
}

/*
 *  ======== ANSIX936KDF_close ========
 */
void ANSIX936KDF_close(ANSIX936KDF_Handle handle)
{
    ANSIX936KDFXX_Object *object = ANSIX936KDFXX_getObject(handle);

    SHA2_close(object->sha2Handle);

    object->isOpen = false;
}

/*
 *  ======== ANSIX936KDF_deriveKey ========
 */
int_fast16_t ANSIX936KDF_deriveKey(ANSIX936KDF_Handle handle,
                                   const void *input,
                                   size_t inputLen,
                                   const void *sharedInfo,
                                   size_t sharedInfoLen,
                                   void *output,
                                   size_t outputLen)
{
    ANSIX936KDFXX_Object *object = ANSIX936KDFXX_getObject(handle);
    int_fast16_t sha2Status      = SHA2_STATUS_SUCCESS;
    int_fast16_t status;
    SHA2_Handle sha2Handle = object->sha2Handle;
    size_t copyLen;
    size_t outputBytesRemaining = outputLen;
    uint32_t bigEndianCounter;
    uint32_t counter;
    uint32_t digest[SHA2_DIGEST_LENGTH_BYTES_256] = {0};
    uint8_t *dest                                 = (uint8_t *)output;

    if ((sharedInfo == NULL) && (sharedInfoLen != 0))
    {
        return ANSIX936KDF_STATUS_ERROR;
    }

    /* ========================================================================
     * Reference SEC 1 version 2.0 standard section 3.6.1.
     *
     * Skipping the check for "|Z| + |SharedInfo| + 4 < hashmaxlength" where Z
     * is the input data since the SHA2 driver can handle size_t length for
     * each individual component.
     *
     * Skipping the check for "keydatalen < hashlen * (2^32-1)" where keydatalen
     * is the output length since the output length is limited to size_t which
     * means the check will always pass.
     *
     * For i = 1 to keydatalen/hashLen, where keydatalen equals outputLen,
     *   Compute: Ki = SHA-2(Z || Counter || [SharedInfo])
     *     where Counter is 32-bit, big-endian byte order with initial value of 1.
     *
     *   The output is the concatenation of computed K values truncated to the
     *   specified output length.
     * ========================================================================
     */

    counter = 1;

    while ((outputBytesRemaining > 0U) && (sha2Status == SHA2_STATUS_SUCCESS))
    {
        if (sha2Status == SHA2_STATUS_SUCCESS)
        {
            /* Hash the input */
            sha2Status = SHA2_addData(sha2Handle, input, inputLen);
        }

        if (sha2Status == SHA2_STATUS_SUCCESS)
        {
            /* Hash the counter converted to big endian */
            bigEndianCounter = BSWAP32(counter);
            sha2Status       = SHA2_addData(sha2Handle, &bigEndianCounter, sizeof(counter));
        }

        if ((sharedInfoLen != 0) && (sha2Status == SHA2_STATUS_SUCCESS))
        {
            /* Hash the shared info */
            sha2Status = SHA2_addData(sha2Handle, sharedInfo, sharedInfoLen);
        }

        if (sha2Status == SHA2_STATUS_SUCCESS)
        {
            sha2Status = SHA2_finalize(sha2Handle, digest);
        }

        if (sha2Status == SHA2_STATUS_SUCCESS)
        {
            copyLen = Math_MIN(outputBytesRemaining, SHA2_DIGEST_LENGTH_BYTES_256);
            memcpy(dest, digest, copyLen);

            outputBytesRemaining -= copyLen;
            dest += copyLen;
        }

        counter++;
    }

    if (sha2Status == SHA2_STATUS_SUCCESS)
    {
        status = ANSIX936KDF_STATUS_SUCCESS;
    }
    else if (sha2Status == SHA2_STATUS_RESOURCE_UNAVAILABLE)
    {
        status = ANSIX936KDF_STATUS_RESOURCE_UNAVAILABLE;
    }
    else
    {
        status = ANSIX936KDF_STATUS_ERROR;
    }

    return status;
}
