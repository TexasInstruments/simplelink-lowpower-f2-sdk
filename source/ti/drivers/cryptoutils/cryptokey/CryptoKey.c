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

#include <ti/drivers/cryptoutils/cryptokey/CryptoKey.h>

#if defined(TFM_PSA_API) /* TFM_PSA_API indicates this is a TF-M build */

    #include <third_party/tfm/platform/ext/target/ti/cc26x4/cmse.h> /* TI CMSE helper functions */

/*
 *  ======== CryptoKey_verifySecureKey ========
 */
static int_fast16_t CryptoKey_verifySecureKey(CryptoKey *secureKey, bool isWriteable)
{
    int_fast16_t status = CryptoKey_STATUS_ERROR;
    void *ptr;

    if ((secureKey->encoding == CryptoKey_PLAINTEXT) || (secureKey->encoding == CryptoKey_BLANK_PLAINTEXT))
    {
        /* Verify key material address range */
        if (isWriteable)
        {
            ptr = cmse_has_unpriv_nonsecure_rw_access(secureKey->u.plaintext.keyMaterial,
                                                      secureKey->u.plaintext.keyLength);
        }
        else
        {
            ptr = cmse_has_unpriv_nonsecure_read_access(secureKey->u.plaintext.keyMaterial,
                                                        secureKey->u.plaintext.keyLength);
        }

        if (ptr != NULL)
        {
            status = CryptoKey_STATUS_SUCCESS;
        }
    }
    else if ((secureKey->encoding == CryptoKey_KEYSTORE) || (secureKey->encoding == CryptoKey_BLANK_KEYSTORE))
    {
        status = CryptoKey_STATUS_SUCCESS;
    }

    return status;
}

/*
 *  ======== CryptoKey_verifySecureInputKey ========
 */
int_fast16_t CryptoKey_verifySecureInputKey(CryptoKey *secureKey)
{
    return CryptoKey_verifySecureKey(secureKey, false);
}

/*
 *  ======== CryptoKey_verifySecureOutputKey ========
 */
int_fast16_t CryptoKey_verifySecureOutputKey(CryptoKey *secureKey)
{
    return CryptoKey_verifySecureKey(secureKey, true);
}

#endif
