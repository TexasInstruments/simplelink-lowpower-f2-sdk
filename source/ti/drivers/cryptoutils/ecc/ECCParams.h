/*
 * Copyright (c) 2017-2024, Texas Instruments Incorporated
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
/** ============================================================================
 *  @file       ECCParams.h
 *
 *  This file contains a common definition for elliptic curve structures used
 *  throughout the ECC based drivers. Not all devices support every curve.
 */

#ifndef ti_drivers_cryptoutils_ecc_ECCParams__include
#define ti_drivers_cryptoutils_ecc_ECCParams__include

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <ti/drivers/cryptoutils/cryptokey/CryptoKey.h>
#include <ti/devices/DeviceFamily.h>

#if (DeviceFamily_PARENT == DeviceFamily_PARENT_CC27XX)
    #include <ti/drivers/cryptoutils/ecc/ECCParamsLPF3HSM.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Error status codes for the utility functions */

/*!
 * @brief   Successful status code.
 *
 * Function return ECCParams_STATUS_SUCCESS if the control code was executed
 * successfully.
 */
#define ECCParams_STATUS_SUCCESS (0)

/*!
 * @brief   Generic error status code.
 *
 * Functions return ECCParams_STATUS_ERROR if the control code was not executed
 * successfully.
 */
#define ECCParams_STATUS_ERROR (-1)

/*!
 *  @brief Enumeration of curve equations supported.
 *
 *  Elliptic curves can be expressed using multiple equations of polynomials over
 *  finite fields.
 *  All forms can be converted to one another using parameter substitution.
 *  Each curve has a default curve equations it was designed to use.
 *
 *  Some curve implementations have restrictions on which algorithms and schemes
 *  they work with. For example, Curve25519 was explicitely designed with ECDH in mind.
 *  It only uses and yields the X coordinate of a point on the elliptic curve in common
 *  implementations. Some implementations do provide X and Y affine coordinates but most
 *  do not.
 *  Therefore, ECDSA and ECJPAKE do not have compatible implementations
 *  for Curve25519 on some devices as the Y coordinate is required by them.
 *
 *  Check the header files of each device-specific implementation for information
 *  regarding curve-support for specific schemes on a device.
 *
 *  | Name              | Equation                      |
 *  |-------------------|-------------------------------|
 *  | Short Weierstrass | y^2 = x^3 + a*x + b mod p     |
 *  | Montgomery        | By^2 = x^3 + Ax^2 + x mod p   |
 *  | Edwards           | x^2 + y^2 = 1 + dx^2y^2 mod p |
 *
 */
typedef uint32_t ECCParams_CurveType;

#define ECCParams_CURVE_TYPE_NONE                  0U
#define ECCParams_CURVE_TYPE_SHORT_WEIERSTRASS_AN3 1U
#define ECCParams_CURVE_TYPE_SHORT_WEIERSTRASS_GEN 2U
#define ECCParams_CURVE_TYPE_MONTGOMERY            3U
#define ECCParams_CURVE_TYPE_EDWARDS               4U

/*!
 *  @brief Enumeration of ECC curve names supported by TF-M.
 */
typedef enum
{
    /*
     * WARNING: Do not alter the order or contents of this enum without updating
     * the corresponding curveParamTable array in ECCParamCC26X4_s.c
     */
    ECCParams_SecureCurve_NISTP224 = 0,
    ECCParams_SecureCurve_NISTP256,
    ECCParams_SecureCurve_NISTP384,
    ECCParams_SecureCurve_NISTP521,
    ECCParams_SecureCurve_BrainpoolP256R1,
    ECCParams_SecureCurve_BrainpoolP384R1,
    ECCParams_SecureCurve_BrainpoolP512R1,
    ECCParams_SecureCurve_Curve25519,
    ECCParams_SecureCurve_Ed25519,
    ECCParams_SecureCurve_Wei25519,
    ECCParams_SecureCurve_COUNT /* This element denotes the max enum value and is not a valid curve */
} ECCParams_SecureCurve;

/*
 * ECCParams_CurveParams have different struct members depending on the context
 * of the build (Secure-only, Non-secure, or Secure)
 */
#if (TFM_ENABLED == 0) || defined(TFM_BUILD) /* TFM_BUILD indicates this is a TF-M build */

/*!
 *  @brief A structure containing the parameters of an elliptic curve.
 *
 *  Elliptical Curve Cryptography (ECC) prime curve parameters.
 */

typedef struct ECCParams_CurveParams
{
    const ECCParams_CurveType curveType;
    const uint8_t *prime;
    const uint8_t *a;
    const uint8_t *b;
    const uint8_t *order;
    const uint8_t cofactor;
    const size_t length;
    const uint8_t *generatorX;
    const uint8_t *generatorY;
} ECCParams_CurveParams;

#else

/*!
 *  @brief A structure containing the curve name to reference elliptic curve
 *         parameters stored in secure memory.
 */
typedef struct ECCParams_CurveParams
{
    ECCParams_SecureCurve secureCurve;
} ECCParams_CurveParams;

#endif /* (TFM_ENABLED == 0) || defined(TFM_BUILD) */

#if defined(TFM_BUILD) /* TFM_BUILD indicates this is a TF-M build */

/*!
 *  @brief A structure containing the curve name to reference elliptic curve
 *         parameters stored in secure memory.
 */
/* This must match the ECCParams_CurveParams struct definition directly above */
typedef struct ECCParams_ns_CurveParams
{
    ECCParams_SecureCurve secureCurve;
} ECCParams_ns_CurveParams;

#endif /* defined(TFM_BUILD) */

/* Short Weierstrass curves */

/*!
 *
 *  @brief The NISTP192 curve in short Weierstrass form.
 *
 */
extern const ECCParams_CurveParams ECCParams_NISTP192;

/*!
 *
 *  @brief The NISTP224 curve in short Weierstrass form.
 *
 */
extern const ECCParams_CurveParams ECCParams_NISTP224;

/*!
 *
 *  @brief The NISTP256 curve in short Weierstrass form.
 *
 */
extern const ECCParams_CurveParams ECCParams_NISTP256;

/*!
 *
 *  @brief The NISTP384 curve in short Weierstrass form.
 *
 */
extern const ECCParams_CurveParams ECCParams_NISTP384;

/*!
 *
 *  @brief The NISTP521 curve in short Weierstrass form.
 *
 */
extern const ECCParams_CurveParams ECCParams_NISTP521;

/*!
 *
 *  @brief The BrainpoolP256R1 curve in short Weierstrass form.
 *
 */
extern const ECCParams_CurveParams ECCParams_BrainpoolP256R1;

/*!
 *
 *  @brief The BrainpoolP384R1 curve in short Weierstrass form.
 *
 */
extern const ECCParams_CurveParams ECCParams_BrainpoolP384R1;

/*!
 *
 *  @brief The BrainpoolP512R1 curve in short Weierstrass form.
 *
 */
extern const ECCParams_CurveParams ECCParams_BrainpoolP512R1;

/*!
 *  @brief A short Weierstrass equivalent representation of Ed25519.
 */
extern const ECCParams_CurveParams ECCParams_Wei25519;

/* Montgomery curves */

/*!
 *  @brief The Curve25519 curve in Montgomery form.
 */
extern const ECCParams_CurveParams ECCParams_Curve25519;

/* Edwards curves */

/*!
 *  @brief The Ed25519 curve in Edwards form.
 */
extern const ECCParams_CurveParams ECCParams_Ed25519;

/*!
 *  @brief Number of bytes for the length word prepended before all parameters
 *  passed into the ECC SW library functions.
 */
#define ECC_LENGTH_PREFIX_BYTES 4

#if (DeviceFamily_PARENT == DeviceFamily_PARENT_CC23X0) || (DeviceFamily_PARENT == DeviceFamily_PARENT_CC27XX) || \
    (DeviceFamily_PARENT == DeviceFamily_PARENT_CC35XX)

    /*!
     *  @defgroup nistp256_params NIST P256 curve params to be used with ECC SW library
     *  Note: CC26X1 uses NIST P256 curve params defined in driverlib/rom_ecc.h
     *  @{
     */

    /*!
     *  @brief Length of NIST P256 curve parameters in bytes
     */
    #define ECCParams_NISTP256_LENGTH 32

    /*!
     *  @brief Length in bytes of NISTP256 curve parameters including the prepended
     *  length word.
     */
    #define ECC_NISTP256_PARAM_LENGTH_WITH_PREFIX_BYTES (ECCParams_NISTP256_LENGTH + ECC_LENGTH_PREFIX_BYTES)

/*!
 *  @brief Union to access ECC_NISTP256 curve params in bytes or words.
 */
typedef union
{
    uint8_t byte[ECC_NISTP256_PARAM_LENGTH_WITH_PREFIX_BYTES];
    uint32_t word[ECC_NISTP256_PARAM_LENGTH_WITH_PREFIX_BYTES / sizeof(uint32_t)];
} ECC_NISTP256_Param;

/*!
 *  @brief X coordinate of the generator point of the ECC_NISTP256 curve.
 */
extern const ECC_NISTP256_Param ECC_NISTP256_generatorX;

/*!
 *  @brief Y coordinate of the generator point of the ECC_NISTP256 curve.
 */
extern const ECC_NISTP256_Param ECC_NISTP256_generatorY;

/*!
 *  @brief Prime of the generator point of the ECC_NISTP256 curve.
 */
extern const ECC_NISTP256_Param ECC_NISTP256_prime;

/*!
 *  @brief 'a' constant of the ECC_NISTP256 curve when expressed in short
 *  Weierstrass form (y^2 = x^3 + a*x + b).
 */
extern const ECC_NISTP256_Param ECC_NISTP256_a;

/*!
 *  @brief 'b' constant of the ECC_NISTP256 curve when expressed in short
 *  Weierstrass form (y^2 = x^3 + a*x + b).
 */
extern const ECC_NISTP256_Param ECC_NISTP256_b;

/*!
 *  @brief Order of the generator point of the ECC_NISTP256 curve.
 */
extern const ECC_NISTP256_Param ECC_NISTP256_order;

/*!
 *  @brief 'k' in Montgomery domain of the ECC_NISTP256 curve.
 */
extern const ECC_NISTP256_Param ECC_NISTP256_k_mont;

/*!
 *  @brief 'a' in Montgomery domain of the ECC_NISTP256 curve.
 */
extern const ECC_NISTP256_Param ECC_NISTP256_a_mont;

/*!
 *  @brief 'b' in Montgomery domain of the ECC_NISTP256 curve.
 */
extern const ECC_NISTP256_Param ECC_NISTP256_b_mont;

    /*! @} */ /* end of nistp256_params */

    /*!
     *  @defgroup nistp224_params NIST P224 curve params to be used with ECC SW library
     *  @{
     */

    /*!
     *  @brief Length of NIST P224 curve parameters in bytes
     */
    #define ECCParams_NISTP224_LENGTH 28

    /*!
     *  @brief Length in bytes of NISTP256 curve parameters including the prepended
     *  length word.
     */
    #define ECC_NISTP224_PARAM_LENGTH_WITH_PREFIX_BYTES (ECCParams_NISTP224_LENGTH + ECC_LENGTH_PREFIX_BYTES)

/*!
 *  @brief Union to access ECC_NISTP256 curve params in bytes or words.
 */
typedef union
{
    uint8_t byte[ECC_NISTP224_PARAM_LENGTH_WITH_PREFIX_BYTES];
    uint32_t word[ECC_NISTP224_PARAM_LENGTH_WITH_PREFIX_BYTES / sizeof(uint32_t)];
} ECC_NISTP224_Param;

/*!
 *  @brief X coordinate of the generator point of the ECC_NISTP224 curve.
 */
extern const ECC_NISTP224_Param ECC_NISTP224_generatorX;

/*!
 *  @brief Y coordinate of the generator point of the ECC_NISTP224 curve.
 */
extern const ECC_NISTP224_Param ECC_NISTP224_generatorY;

/*!
 *  @brief Prime of the generator point of the ECC_NISTP224 curve.
 */
extern const ECC_NISTP224_Param ECC_NISTP224_prime;

/*!
 *  @brief 'a' constant of the ECC_NISTP224 curve when expressed in short
 *  Weierstrass form (y^2 = x^3 + a*x + b).
 */
extern const ECC_NISTP224_Param ECC_NISTP224_a;

/*!
 *  @brief 'b' constant of the ECC_NISTP224 curve when expressed in short
 *  Weierstrass form (y^2 = x^3 + a*x + b).
 */
extern const ECC_NISTP224_Param ECC_NISTP224_b;

/*!
 *  @brief Order of the generator point of the ECC_NISTP224 curve.
 */
extern const ECC_NISTP224_Param ECC_NISTP224_order;

/*!
 *  @brief 'k' in Montgomery domain of the ECC_NISTP224 curve.
 */
extern const ECC_NISTP224_Param ECC_NISTP224_k_mont;

/*!
 *  @brief 'a' in Montgomery domain of the ECC_NISTP224 curve.
 */
extern const ECC_NISTP224_Param ECC_NISTP224_a_mont;

/*!
 *  @brief 'b' in Montgomery domain of the ECC_NISTP224 curve.
 */
extern const ECC_NISTP224_Param ECC_NISTP224_b_mont;

    /*! @} */ /* end of nistp224_params */

    /* Octet string format requires an extra byte at the start of the public key */
    #define OCTET_STRING_OFFSET 1

    /* Octet string format requires this value in the first byte of the public key */
    #define OCTET_STRING_PREFIX 0x04

    /* Length of offset in bytes */
    #define ECC_LENGTH_OFFSET_BYTES 4

    /* Param length needs to be equal to the length of the largest curve supported plus length offset bytes */
    #define ECC_PARAM_LENGTH_WITH_OFFSET_BYTES (ECCParams_NISTP256_LENGTH + ECC_LENGTH_OFFSET_BYTES)

/*!
 *  @brief Union to format inputs to ECC library.
 */
typedef union
{
    uint32_t word[ECC_PARAM_LENGTH_WITH_OFFSET_BYTES / sizeof(uint32_t)];
    uint8_t byte[ECC_PARAM_LENGTH_WITH_OFFSET_BYTES];
} ECC_Param;

#endif /* (DeviceFamily_PARENT == DeviceFamily_PARENT_CC23X0) || (DeviceFamily_PARENT == DeviceFamily_PARENT_CC27XX) \
          || (DeviceFamily_PARENT == DeviceFamily_PARENT_CC35XX) */

/*!
 *  @brief Length of Curve25519 curve parameters in bytes
 */
#define ECCParams_CURVE25519_LENGTH 32

/*!
 *  @defgroup curve25519_params Curve25519 curve params to be used with ECC SW library
 *  @{
 */

/*!
 *  @brief Length in bytes of Curve25519 curve parameters including the prepended
 *  length word.
 */
#define ECC_CURVE25519_LENGTH_WITH_PREFIX_BYTES (ECCParams_CURVE25519_LENGTH + ECC_LENGTH_PREFIX_BYTES)

/*!
 *  @brief Union to access ECC_Curve25519 curve params in bytes or words.
 */
typedef union
{
    uint8_t byte[ECC_CURVE25519_LENGTH_WITH_PREFIX_BYTES];
    uint32_t word[ECC_CURVE25519_LENGTH_WITH_PREFIX_BYTES / sizeof(uint32_t)];
} ECC_Curve25519_Param;

/*!
 *  @brief X coordinate of the generator point of the ECC_Curve25519 curve.
 */
extern const ECC_Curve25519_Param ECC_Curve25519_generatorX;

/*!
 *  @brief Y coordinate of the generator point of the ECC_Curve25519 curve.
 */
extern const ECC_Curve25519_Param ECC_Curve25519_generatorY;

/*!
 *  @brief Prime of the generator point of the ECC_Curve25519 curve.
 */
extern const ECC_Curve25519_Param ECC_Curve25519_prime;

/*!
 *  @brief 'a' constant of the ECC_Curve25519 curve when expressed in short
 *  Weierstrass form (y^2 = x^3 + a*x + b).
 */
extern const ECC_Curve25519_Param ECC_Curve25519_a;

/*!
 *  @brief 'b' constant of the ECC_Curve25519 curve when expressed in short
 *  Weierstrass form (y^2 = x^3 + a*x + b).
 */
extern const ECC_Curve25519_Param ECC_Curve25519_b;

/*!
 *  @brief Order of the generator point of the ECC_Curve25519 curve.
 */
extern const ECC_Curve25519_Param ECC_Curve25519_order;

/*! @} */ /* end of curve25519_params */

/* Utility functions */

/* #define used for backwards compatibility */
#define ECCParams_FormatCurve25519PrivateKey ECCParams_formatCurve25519PrivateKey

/*!
 *  @brief Formats a CryptoKey to conform to Curve25519 private key requirements.
 *
 *  Curve25519 has specific private key requirements specified by the curve definition.
 *  Specifically, the bottom three and the top bit may not be set and the second to
 *  last bit must be set.
 *
 *  @param  myPrivateKey    An initialized CryptoKey describing the entropy for a
 *                          Curve25519 private key. Platform-specific restrictions
 *                          for the location of the keying material apply. Some
 *                          implementations do not support modifying keying material
 *                          in flash for example.
 *
 *  @pre Initialize the CryptoKey with a 32-byte buffer in a compliant location.
 */
int_fast16_t ECCParams_formatCurve25519PrivateKey(CryptoKey *myPrivateKey);

/*!
 *  @brief Extracts the curve generator point from an ecliptic curve description.
 *
 *  The curve parameters #ECCParams_CurveParams::generatorX and
 *  #ECCParams_CurveParams::generatorY are extracted from \c curveParams and
 *  written as a concatenated octet string in big endian order to
 *  \c buffer. The format is defined in SEC 1: Elliptic Curve Cryptography section
 *  2.3.3.
 *
 *  The curve point has the format ``0x04 || X || Y`` and the length is
 *  ``2 * size_of_x_or_y + 1`` where ``0x04`` specifies octet string format.
 *  If the buffer \c length exceeds the curve point length, the remaining
 *  buffer space is zeroed.
 *
 *  @param  curveParams     Points to the input curve parameters
 *  @param  buffer          Points to the destination where the generator point will
 *                          be written to. Make sure that \c buffer is large enough to
 *                          hold
 *  @param  length          Maximum length of \c buffer in bytes.
 *
 *  @retval #ECCParams_STATUS_SUCCESS on success, #ECCParams_STATUS_ERROR if the
 *          provided buffer \c length is insufficient to hold the curve point.
 *
 */
int_fast16_t ECCParams_getUncompressedGeneratorPoint(const ECCParams_CurveParams *curveParams,
                                                     uint8_t *buffer,
                                                     size_t length);

#ifdef __cplusplus
}
#endif

#endif /* ti_drivers_cryptoutils_ecc_ECCParams__include */
