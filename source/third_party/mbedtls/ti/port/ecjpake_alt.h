/******************************************************************************
 Copyright (c) 2022-2023, Texas Instruments Incorporated
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions
 are met:

 *  Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.

 *  Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.

 *  Neither the name of Texas Instruments Incorporated nor the names of
    its contributors may be used to endorse or promote products derived
    from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************/

#ifndef MBEDTLS_ECJPAKE_ALT_H
#define MBEDTLS_ECJPAKE_ALT_H

#include "mbedtls/build_info.h"

#if defined(MBEDTLS_ECJPAKE_ALT)

#include "mbedtls/ecp.h"
#include "mbedtls/md.h"

#include <ti/drivers/ECJPAKE.h>
#include <ti/drivers/cryptoutils/cryptokey/CryptoKey.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Length of parameters for the NIST p256r1.
 *
 * ECJ-PAKE is only defined for p256r1
 */
#define NISTP256_CURVE_LENGTH_BYTES (32)

/**
 * Size in bytes of the identifier at the beginning of the point in big-endian format
 */
#define OCTET_STRING_OFFSET 1

    /**
     * Size in bytes of a point expressed in the TLS point format
     *
     * id_byte || X coord || Y coord
     */
#define NISTP256_PUBLIC_KEY_LENGTH_BYTES (OCTET_STRING_OFFSET + (NISTP256_CURVE_LENGTH_BYTES * 2))

/**
 * EC J-PAKE context structure.
 */
typedef struct
{
    const mbedtls_md_info_t *md_info;
    mbedtls_ecp_group_id curve;
    mbedtls_ecjpake_role role;
    int point_format;

    bool roundTwoGenerated;
    /*
     * XXX: possible size reduction by moving ephemeral material to round
     * calculations.
     */
    unsigned char myPrivateKeyMaterial1[NISTP256_CURVE_LENGTH_BYTES];
    unsigned char myPrivateKeyMaterial2[NISTP256_CURVE_LENGTH_BYTES];
    unsigned char myPrivateVMaterial1[NISTP256_CURVE_LENGTH_BYTES];
    unsigned char myPrivateVMaterial2[NISTP256_CURVE_LENGTH_BYTES];
    unsigned char myPrivateVMaterial3[NISTP256_CURVE_LENGTH_BYTES];
    unsigned char myPublicKeyMaterial1[NISTP256_PUBLIC_KEY_LENGTH_BYTES];
    unsigned char myPublicKeyMaterial2[NISTP256_PUBLIC_KEY_LENGTH_BYTES];
    unsigned char myPublicVMaterial1[NISTP256_PUBLIC_KEY_LENGTH_BYTES];
    unsigned char myPublicVMaterial2[NISTP256_PUBLIC_KEY_LENGTH_BYTES];
    unsigned char myPublicVMaterial3[NISTP256_PUBLIC_KEY_LENGTH_BYTES];
    unsigned char myCombinedPublicKeyMaterial1[NISTP256_PUBLIC_KEY_LENGTH_BYTES];
    unsigned char myCombinedPrivateKeyMaterial1[NISTP256_CURVE_LENGTH_BYTES];
    unsigned char nistP256Generator[NISTP256_PUBLIC_KEY_LENGTH_BYTES];
    unsigned char myGenerator[NISTP256_PUBLIC_KEY_LENGTH_BYTES];
    unsigned char theirPublicKeyMaterial1[NISTP256_PUBLIC_KEY_LENGTH_BYTES];
    unsigned char theirPublicKeyMaterial2[NISTP256_PUBLIC_KEY_LENGTH_BYTES];
    unsigned char theirCombinedPublicKeyMaterial1[NISTP256_PUBLIC_KEY_LENGTH_BYTES];
    unsigned char theirGenerator[NISTP256_PUBLIC_KEY_LENGTH_BYTES];
    unsigned char preSharedSecretKeyingMaterial[NISTP256_CURVE_LENGTH_BYTES];

    CryptoKey nistP256GeneratorCryptoKey;
    CryptoKey preSharedSecretCryptoKey;
    CryptoKey myPrivateCryptoKey1;
    CryptoKey myPrivateCryptoKey2;
    CryptoKey myPrivateCryptoV1;
    CryptoKey myPrivateCryptoV2;
    CryptoKey myPrivateCryptoV3;
    CryptoKey myCombinedPrivateKey;
    CryptoKey myPublicCryptoKey1;
    CryptoKey myPublicCryptoKey2;
    CryptoKey myPublicCryptoV1;
    CryptoKey myPublicCryptoV2;
    CryptoKey myPublicCryptoV3;
    CryptoKey myCombinedPublicKey;
    CryptoKey myGeneratorKey;
    CryptoKey theirPublicCryptoKey1;
    CryptoKey theirPublicCryptoKey2;
    CryptoKey theirCombinedPublicKey;
    CryptoKey theirGeneratorKey;

    ECJPAKE_Handle handle;
} mbedtls_ecjpake_context;

#ifdef __cplusplus
}
#endif

#endif /* MBEDTLS_ECJPAKE_ALT */

#endif /* MBEDTLS_ECJPAKE_ALT_H */