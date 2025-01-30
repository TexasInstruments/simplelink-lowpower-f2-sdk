/*
 * Copyright (c) 2022-2023, Texas Instruments Incorporated
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
 *  ======== ECCParamsCC26X4_s.c ========
 *
 *  This file contains structure definitions for various ECC curves for use
 *  on CC26X4 devices.
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <ti/drivers/cryptoutils/ecc/ECCParams.h>
#include <ti/drivers/cryptoutils/cryptokey/CryptoKey.h>

#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(driverlib/pka.h)

#include <third_party/tfm/interface/include/psa/error.h>
#include <third_party/tfm/secure_fw/spm/include/utilities.h>
#include <third_party/tfm/secure_fw/include/security_defs.h> /* __tz_c_veneer */

#include <third_party/tfm/platform/ext/target/ti/cc26x4/cmse.h> /* TI CMSE helper functions */

static const ECCParams_CurveParams ECCParams_s_NISTP224 = {.curveType  = ECCParams_CURVE_TYPE_SHORT_WEIERSTRASS_AN3,
                                                           .length     = NISTP224_PARAM_SIZE_BYTES,
                                                           .prime      = NISTP224_prime.byte,
                                                           .order      = NISTP224_order.byte,
                                                           .a          = NISTP224_a.byte,
                                                           .b          = NISTP224_b.byte,
                                                           .generatorX = NISTP224_generator.x.byte,
                                                           .generatorY = NISTP224_generator.y.byte,
                                                           .cofactor   = 1};

static const ECCParams_CurveParams ECCParams_s_NISTP256 = {.curveType  = ECCParams_CURVE_TYPE_SHORT_WEIERSTRASS_AN3,
                                                           .length     = NISTP256_PARAM_SIZE_BYTES,
                                                           .prime      = NISTP256_prime.byte,
                                                           .order      = NISTP256_order.byte,
                                                           .a          = NISTP256_a.byte,
                                                           .b          = NISTP256_b.byte,
                                                           .generatorX = NISTP256_generator.x.byte,
                                                           .generatorY = NISTP256_generator.y.byte,
                                                           .cofactor   = 1};

static const ECCParams_CurveParams ECCParams_s_NISTP384 = {.curveType  = ECCParams_CURVE_TYPE_SHORT_WEIERSTRASS_AN3,
                                                           .length     = NISTP384_PARAM_SIZE_BYTES,
                                                           .prime      = NISTP384_prime.byte,
                                                           .order      = NISTP384_order.byte,
                                                           .a          = NISTP384_a.byte,
                                                           .b          = NISTP384_b.byte,
                                                           .generatorX = NISTP384_generator.x.byte,
                                                           .generatorY = NISTP384_generator.y.byte,
                                                           .cofactor   = 1};

static const ECCParams_CurveParams ECCParams_s_NISTP521 = {.curveType  = ECCParams_CURVE_TYPE_SHORT_WEIERSTRASS_AN3,
                                                           .length     = NISTP521_PARAM_SIZE_BYTES,
                                                           .prime      = NISTP521_prime.byte,
                                                           .order      = NISTP521_order.byte,
                                                           .a          = NISTP521_a.byte,
                                                           .b          = NISTP521_b.byte,
                                                           .generatorX = NISTP521_generator.x.byte,
                                                           .generatorY = NISTP521_generator.y.byte,
                                                           .cofactor   = 1};

static const ECCParams_CurveParams ECCParams_s_BrainpoolP256R1 =
    {.curveType  = ECCParams_CURVE_TYPE_SHORT_WEIERSTRASS_AN3,
     .length     = BrainpoolP256R1_PARAM_SIZE_BYTES,
     .prime      = BrainpoolP256R1_prime.byte,
     .order      = BrainpoolP256R1_order.byte,
     .a          = BrainpoolP256R1_a.byte,
     .b          = BrainpoolP256R1_b.byte,
     .generatorX = BrainpoolP256R1_generator.x.byte,
     .generatorY = BrainpoolP256R1_generator.y.byte,
     .cofactor   = 1};

static const ECCParams_CurveParams ECCParams_s_BrainpoolP384R1 =
    {.curveType  = ECCParams_CURVE_TYPE_SHORT_WEIERSTRASS_AN3,
     .length     = BrainpoolP384R1_PARAM_SIZE_BYTES,
     .prime      = BrainpoolP384R1_prime.byte,
     .order      = BrainpoolP384R1_order.byte,
     .a          = BrainpoolP384R1_a.byte,
     .b          = BrainpoolP384R1_b.byte,
     .generatorX = BrainpoolP384R1_generator.x.byte,
     .generatorY = BrainpoolP384R1_generator.y.byte,
     .cofactor   = 1};

static const ECCParams_CurveParams ECCParams_s_BrainpoolP512R1 =
    {.curveType  = ECCParams_CURVE_TYPE_SHORT_WEIERSTRASS_AN3,
     .length     = BrainpoolP512R1_PARAM_SIZE_BYTES,
     .prime      = BrainpoolP512R1_prime.byte,
     .order      = BrainpoolP512R1_order.byte,
     .a          = BrainpoolP512R1_a.byte,
     .b          = BrainpoolP512R1_b.byte,
     .generatorX = BrainpoolP512R1_generator.x.byte,
     .generatorY = BrainpoolP512R1_generator.y.byte,
     .cofactor   = 1};

static const ECCParams_CurveParams ECCParams_s_Curve25519 = {.curveType  = ECCParams_CURVE_TYPE_MONTGOMERY,
                                                             .length     = 32,
                                                             .prime      = Curve25519_prime.byte,
                                                             .order      = Curve25519_order.byte,
                                                             .a          = Curve25519_a.byte,
                                                             .b          = Curve25519_b.byte,
                                                             .generatorX = Curve25519_generator.x.byte,
                                                             .generatorY = Curve25519_generator.y.byte,
                                                             .cofactor   = 1};

/*
 * Ed25519 constants in little endian format. byte[0] is the least
 * significant byte and byte[Ed25519_PARAM_SIZE_BYTES - 1] is the most
 * significant.
 */
static const PKA_EccPoint256 Ed25519_generator = {
    .x = {.byte = {0x1a, 0xd5, 0x25, 0x8f, 0x60, 0x2d, 0x56, 0xc9, 0xb2, 0xa7, 0x25, 0x95, 0x60, 0xc7, 0x2c, 0x69,
                   0x5c, 0xdc, 0xd6, 0xfd, 0x31, 0xe2, 0xa4, 0xc0, 0xfe, 0x53, 0x6e, 0xcd, 0xd3, 0x36, 0x69, 0x21}},
    .y = {.byte = {0x58, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66,
                   0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66}},
};

static const PKA_EccParam256 Ed25519_prime = {.byte = {0xed, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                                                       0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                                                       0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f}};

static const PKA_EccParam256 Ed25519_order = {.byte = {0xed, 0xd3, 0xf5, 0x5c, 0x1a, 0x63, 0x12, 0x58, 0xd6, 0x9c, 0xf7,
                                                       0xa2, 0xde, 0xf9, 0xde, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                                       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10}};

static const PKA_EccParam256 Ed25519_a = {.byte = {0xec, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                                                   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                                                   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f}};

static const PKA_EccParam256 Ed25519_d = {.byte = {0xa3, 0x78, 0x59, 0x13, 0xca, 0x4d, 0xeb, 0x75, 0xab, 0xd8, 0x41,
                                                   0x41, 0x4d, 0x0a, 0x70, 0x00, 0x98, 0xe8, 0x79, 0x77, 0x79, 0x40,
                                                   0xc7, 0x8c, 0x73, 0xfe, 0x6f, 0x2b, 0xee, 0x6c, 0x03, 0x52}};

const ECCParams_CurveParams ECCParams_s_Ed25519 = {.curveType  = ECCParams_CURVE_TYPE_EDWARDS,
                                                   .length     = 32,
                                                   .prime      = Ed25519_prime.byte,
                                                   .order      = Ed25519_order.byte,
                                                   .a          = Ed25519_a.byte,
                                                   .b          = Ed25519_d.byte,
                                                   .generatorX = Ed25519_generator.x.byte,
                                                   .generatorY = Ed25519_generator.y.byte,
                                                   .cofactor   = 8};

/*
 * Wei25519 constants in little endian format. byte[0] is the least
 * significant byte and byte[Wei25519_PARAM_SIZE_BYTES - 1] is the most
 * significant.
 */
static const PKA_EccPoint256 Wei25519_generator = {
    .x = {.byte = {0x5a, 0x24, 0xad, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
                   0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0x2a}},
    .y = {.byte = {0xd9, 0xd3, 0xce, 0x7e, 0xa2, 0xc5, 0xe9, 0x29, 0xb2, 0x61, 0x7c, 0x6d, 0x7e, 0x4d, 0x3d, 0x92,
                   0x4c, 0xd1, 0x48, 0x77, 0x2c, 0xdd, 0x1e, 0xe0, 0xb4, 0x86, 0xa0, 0xb8, 0xa1, 0x19, 0xae, 0x20}},
};

static const PKA_EccParam256 Wei25519_prime = {
    .byte = {0xed, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
             0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f}};

static const PKA_EccParam256 Wei25519_order = {
    .byte = {0xed, 0xd3, 0xf5, 0x5c, 0x1a, 0x63, 0x12, 0x58, 0xd6, 0x9c, 0xf7, 0xa2, 0xde, 0xf9, 0xde, 0x14,
             0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10}};

static const PKA_EccParam256 Wei25519_a = {.byte = {0x44, 0xa1, 0x14, 0x49, 0x98, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
                                                    0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
                                                    0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0x2a}};

static const PKA_EccParam256 Wei25519_b = {.byte = {0x64, 0xc8, 0x10, 0x77, 0x9c, 0x5e, 0x0b, 0x26, 0xb4, 0x97, 0xd0,
                                                    0x5e, 0x42, 0x7b, 0x09, 0xed, 0x25, 0xb4, 0x97, 0xd0, 0x5e, 0x42,
                                                    0x7b, 0x09, 0xed, 0x25, 0xb4, 0x97, 0xd0, 0x5e, 0x42, 0x7b}};

const ECCParams_CurveParams ECCParams_s_Wei25519 = {.curveType  = ECCParams_CURVE_TYPE_SHORT_WEIERSTRASS_GEN,
                                                    .length     = 32,
                                                    .prime      = Wei25519_prime.byte,
                                                    .order      = Wei25519_order.byte,
                                                    .a          = Wei25519_a.byte,
                                                    .b          = Wei25519_b.byte,
                                                    .generatorX = Wei25519_generator.x.byte,
                                                    .generatorY = Wei25519_generator.y.byte,
                                                    .cofactor   = 8};

/* Curve param pointer table indexed by the ECCParams_SecureCurve enum value */
static const ECCParams_CurveParams *const ECCParams_s_curveParamTable[ECCParams_SecureCurve_COUNT] =
    {&ECCParams_s_NISTP224,
     &ECCParams_s_NISTP256,
     &ECCParams_s_NISTP384,
     &ECCParams_s_NISTP521,
     &ECCParams_s_BrainpoolP256R1,
     &ECCParams_s_BrainpoolP384R1,
     &ECCParams_s_BrainpoolP512R1,
     &ECCParams_s_Curve25519,
     &ECCParams_s_Ed25519,
     &ECCParams_s_Wei25519};

/*
 *  ======== ECCParams_s_getCurveParams ========
 */
const ECCParams_CurveParams *ECCParams_s_getCurveParams(const ECCParams_CurveParams *curveParams)
{
    ECCParams_ns_CurveParams params_ns;
    const ECCParams_CurveParams *params_s = NULL;

    /*
     * Validate curve param address range. Note that 'sizeof(params_ns)' is used
     * for the length because ECCParams_CurveParams struct has different sizes
     * within the secure and non-secure code.
     */
    if (cmse_has_unpriv_nonsecure_read_access((void *)curveParams, sizeof(params_ns)) != NULL)
    {
        /* Make a secure copy of the param struct to avoid typecast */
        (void)spm_memcpy(&params_ns, (void *)curveParams, sizeof(params_ns));

        if (params_ns.secureCurve < ECCParams_SecureCurve_COUNT)
        {
            params_s = ECCParams_s_curveParamTable[params_ns.secureCurve];
        }
    }

    return params_s;
}

/*
 *  ======== ECCParams_getUncompressedGeneratorPoint ========
 */
__tz_c_veneer int_fast16_t ECCParams_s_getUncompressedGeneratorPoint(const ECCParams_CurveParams *curveParams,
                                                                     uint8_t *buffer,
                                                                     size_t length)
{
    const ECCParams_CurveParams *params_s;
    size_t paramLength;
    size_t pointLength;
    uint_fast8_t i;

    params_s = ECCParams_s_getCurveParams(curveParams);
    if (params_s == NULL)
    {
        return (int_fast16_t)PSA_ERROR_PROGRAMMER_ERROR;
    }

    paramLength = params_s->length;
    pointLength = (paramLength * 2) + 1; /* Point format: 0x04 || X || Y */

    /* Validate length */
    if (length < pointLength)
    {
        return ECCParams_STATUS_ERROR;
    }

    /* Validate output buffer address range */
    if (cmse_has_unpriv_nonsecure_rw_access((void *)buffer, length) == NULL)
    {
        return (int_fast16_t)PSA_ERROR_PROGRAMMER_ERROR;
    }

    /* Reverse and concatenate x and y */
    for (i = 0; i < paramLength; i++)
    {
        buffer[i + 1]               = params_s->generatorX[paramLength - i - 1];
        buffer[i + 1 + paramLength] = params_s->generatorY[paramLength - i - 1];
    }

    buffer[0] = 0x04;
    /* Fill the remaining buffer with 0 if needed */
    memset(buffer + pointLength, 0, length - pointLength);

    return ECCParams_STATUS_SUCCESS;
}
