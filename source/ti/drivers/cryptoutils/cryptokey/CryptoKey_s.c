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
#include <ti/drivers/cryptoutils/cryptokey/CryptoKey_s.h>

#include <third_party/tfm/interface/include/tfm_api.h>
#include <third_party/tfm/interface/include/psa/error.h>
#include <third_party/tfm/secure_fw/spm/include/tfm_memory_utils.h>

#include <third_party/tfm/platform/ext/target/ti/cc26x4/cmse.h> /* TI CMSE helper functions */

/*
 *  ======== CryptoKey_s_copyCryptoKeyFromClient ========
 */
void CryptoKey_s_copyCryptoKeyFromClient(CryptoKey *dstRandomKey, CryptoKey *srcRandomKey, int32_t clientId)
{
    CryptoKey_ns nsCryptoKey;
    /*
     * Validate the address range for randomKey. Use the sizeof(CryptoKey_ns) as the size of the secure and non-secure
     * CryptoKey varies between the secure and non-secure definitions
     */
    if (cmse_has_unpriv_nonsecure_read_access(srcRandomKey, sizeof(CryptoKey_ns)) != NULL)
    {
        /* Make a secure copy of the non-secure CryptoKey */
        (void)tfm_memcpy(&nsCryptoKey, (void *)srcRandomKey, sizeof(CryptoKey_ns));

        dstRandomKey->encoding = nsCryptoKey.encoding;

        if ((dstRandomKey->encoding == CryptoKey_BLANK_KEYSTORE) || (dstRandomKey->encoding == CryptoKey_KEYSTORE))
        {
            dstRandomKey->u.keyStore.keyID     = nsCryptoKey.u.nsKeyStore.keyID;
            dstRandomKey->u.keyStore.keyLength = nsCryptoKey.u.nsKeyStore.keyLength;

            /* Copy function always returns success */
            (void)KeyStore_s_copyKeyAttributesFromClient(&nsCryptoKey.u.nsKeyStore.nsAttributes,
                                                         clientId,
                                                         &dstRandomKey->u.keyStore.attributes);
        }
        else if ((dstRandomKey->encoding == CryptoKey_BLANK_PLAINTEXT) ||
                 (dstRandomKey->encoding == CryptoKey_BLANK_PLAINTEXT))
        {
            dstRandomKey->u.plaintext.keyLength   = nsCryptoKey.u.plaintext.keyLength;
            dstRandomKey->u.plaintext.keyMaterial = nsCryptoKey.u.plaintext.keyMaterial;
        }
    }
}