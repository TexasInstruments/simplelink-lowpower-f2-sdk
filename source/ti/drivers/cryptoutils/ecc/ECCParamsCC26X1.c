/*
 * Copyright (c) 2020-2021, Texas Instruments Incorporated
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
 *  ======== ECCParamsCC26X1.c ========
 *
 *  This file contains structure definitions for various ECC curves for use
 *  on CC26X1 devices.
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include <ti/drivers/cryptoutils/ecc/ECCParams.h>
#include <ti/drivers/cryptoutils/cryptokey/CryptoKey.h>

#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(driverlib/rom_ecc.h)

/*
 * NIST P256 curve params in little endian format.
 * byte[0-3] are the param length word as required by the ECC SW library.
 * byte[4] is the least significant byte of the curve parameter.
 *
 * NOTE: NIST P256 curve params are in ROM for CC26X1.
 */
const ECCParams_CurveParams ECCParams_NISTP256 = {.curveType  = ECCParams_CURVE_TYPE_SHORT_WEIERSTRASS_AN3,
                                                  .length     = ECC_NISTP256_PARAM_LENGTH_BYTES,
                                                  .prime      = ECC_NISTP256_prime.byte,
                                                  .order      = ECC_NISTP256_order.byte,
                                                  .a          = ECC_NISTP256_a.byte,
                                                  .b          = ECC_NISTP256_b.byte,
                                                  .generatorX = ECC_NISTP256_generatorX.byte,
                                                  .generatorY = ECC_NISTP256_generatorY.byte,
                                                  .cofactor   = 1};

/*
 * Curve25519 curve params in little endian format.
 * byte[0-3] are the param length word as required by the ECC SW library.
 * byte[4] is the least significant byte of the curve parameter.
 */
const ECC_Curve25519_Param ECC_Curve25519_generatorX = {
    .byte = {0x08, 0x00, 0x00, 0x00, /* Length word prefix */
             0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
             0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};

const ECC_Curve25519_Param ECC_Curve25519_generatorY = {
    .byte = {0x08, 0x00, 0x00, 0x00, /* Length word prefix */
             0xd9, 0xd3, 0xce, 0x7e, 0xa2, 0xc5, 0xe9, 0x29, 0xb2, 0x61, 0x7c, 0x6d, 0x7e, 0x4d, 0x3d, 0x92,
             0x4c, 0xd1, 0x48, 0x77, 0x2c, 0xdd, 0x1e, 0xe0, 0xb4, 0x86, 0xa0, 0xb8, 0xa1, 0x19, 0xae, 0x20}};

const ECC_Curve25519_Param ECC_Curve25519_prime = {
    .byte = {0x08, 0x00, 0x00, 0x00, /* Length word prefix */
             0xed, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
             0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f}};

const ECC_Curve25519_Param ECC_Curve25519_a = {
    .byte = {0x08, 0x00, 0x00, 0x00, /* Length word prefix */
             0x06, 0x6d, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
             0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};

const ECC_Curve25519_Param ECC_Curve25519_b = {
    .byte = {0x08, 0x00, 0x00, 0x00, /* Length word prefix */
             0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
             0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};

const ECC_Curve25519_Param ECC_Curve25519_order = {
    .byte = {0x08, 0x00, 0x00, 0x00, /* Length word prefix */
             0xb9, 0xdc, 0xf5, 0x5c, 0x1a, 0x63, 0x12, 0x58, 0xd6, 0x9c, 0xf7, 0xa2, 0xde, 0xf9, 0xde, 0x14,
             0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};

const ECCParams_CurveParams ECCParams_Curve25519 = {.curveType  = ECCParams_CURVE_TYPE_MONTGOMERY,
                                                    .length     = ECCParams_CURVE25519_LENGTH,
                                                    .prime      = ECC_Curve25519_prime.byte,
                                                    .order      = ECC_Curve25519_order.byte,
                                                    .a          = ECC_Curve25519_a.byte,
                                                    .b          = ECC_Curve25519_b.byte,
                                                    .generatorX = ECC_Curve25519_generatorX.byte,
                                                    .generatorY = ECC_Curve25519_generatorY.byte,
                                                    .cofactor   = 1};

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

    size_t paramLength = curveParams->length;
    size_t pointLength = (paramLength * 2) + 1;

    if (length < pointLength)
    {
        return ECCParams_STATUS_ERROR;
    }

    /* Reverse and concatenate x and y */
    uint32_t i = 0;
    for (i = 0; i < paramLength; i++)
    {
        buffer[i + 1]               = curveParams->generatorX[paramLength + ECC_LENGTH_PREFIX_BYTES - i - 1];
        buffer[i + 1 + paramLength] = curveParams->generatorY[paramLength + ECC_LENGTH_PREFIX_BYTES - i - 1];
    }

    buffer[0] = 0x04;
    /* Fill the remaining buffer with 0 if needed */
    memset(buffer + pointLength, 0, length - pointLength);

    return ECCParams_STATUS_SUCCESS;
}
