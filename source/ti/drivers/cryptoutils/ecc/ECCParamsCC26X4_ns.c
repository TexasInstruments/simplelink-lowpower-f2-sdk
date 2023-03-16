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
/*
 *  ======== ECCParamsCC26X4_ns.c ========
 *
 *  This file contains structure definitions for various ECC curves for use
 *  on CC26X4 devices when utilizing the TF-M.
 */

#include <stddef.h>
#include <stdint.h>

#include <ti/drivers/cryptoutils/ecc/ECCParams.h>
#include <ti/drivers/cryptoutils/ecc/ECCParamsCC26X4_s.h>
#include <ti/drivers/cryptoutils/cryptokey/CryptoKey.h>

#include <ti/drivers/dpl/HwiP.h>

const ECCParams_CurveParams ECCParams_NISTP224 = {.secureCurve = ECCParams_SecureCurve_NISTP224};

const ECCParams_CurveParams ECCParams_NISTP256 = {.secureCurve = ECCParams_SecureCurve_NISTP256};

const ECCParams_CurveParams ECCParams_NISTP384 = {.secureCurve = ECCParams_SecureCurve_NISTP384};

const ECCParams_CurveParams ECCParams_NISTP521 = {.secureCurve = ECCParams_SecureCurve_NISTP521};

const ECCParams_CurveParams ECCParams_BrainpoolP256R1 = {.secureCurve = ECCParams_SecureCurve_BrainpoolP256R1};

const ECCParams_CurveParams ECCParams_BrainpoolP384R1 = {.secureCurve = ECCParams_SecureCurve_BrainpoolP384R1};

const ECCParams_CurveParams ECCParams_BrainpoolP512R1 = {.secureCurve = ECCParams_SecureCurve_BrainpoolP512R1};

const ECCParams_CurveParams ECCParams_Curve25519 = {.secureCurve = ECCParams_SecureCurve_Curve25519};

const ECCParams_CurveParams ECCParams_Ed25519 = {.secureCurve = ECCParams_SecureCurve_Ed25519};

const ECCParams_CurveParams ECCParams_Wei25519 = {.secureCurve = ECCParams_SecureCurve_Wei25519};

/*
 *  ======== ECCParams_formatCurve25519PrivateKey ========
 */
int_fast16_t ECCParams_formatCurve25519PrivateKey(CryptoKey *myPrivateKey)
{
    myPrivateKey->u.plaintext.keyMaterial[31] &= 0xF8;
    myPrivateKey->u.plaintext.keyMaterial[0] &= 0x7F;
    myPrivateKey->u.plaintext.keyMaterial[0] |= 0x40;

    return ECCParams_STATUS_SUCCESS;
}

/*
 *  ======== ECCParams_getUncompressedGeneratorPoint ========
 */
int_fast16_t ECCParams_getUncompressedGeneratorPoint(const ECCParams_CurveParams *curveParams,
                                                     uint8_t *buffer,
                                                     size_t length)
{
    int_fast16_t status;
    uintptr_t key;

    /*
     * Critical section to prevent non-secure task switching while calling
     * secure veneer function.
     */
    key    = HwiP_disable();
    status = ECCParams_s_getUncompressedGeneratorPoint(curveParams, buffer, length);
    HwiP_restore(key);

    return status;
}
