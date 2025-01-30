/******************************************************************************

 @file  ECDSACC26X4_driverlib.h

 @brief Header file for an ECDSA driver using only driverlib APIs
        for the CC26X2 and CC26X4 device families with the following
        restrictions to minimize code size:
        - Single threaded applications only
        - Polling return behavior only
        - Signature verification only
        - P256 curve support only

 Group: CMCU
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2012-2025, Texas Instruments Incorporated
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

 ******************************************************************************
 
 
 *****************************************************************************/

/** ============================================================================
 *  # Hardware and Implementation Details #
 *
 * The CC26X2 family has a dedicated public key accelerator. It is capable of
 * multiple mathematical operations including dedicated ECC point addition,
 * doubling, and scalar multiplication. Only one operation can be carried out on
 * the accelerator at a time. Mutual exclusion is implemented at the driver
 * level and coordinated between all drivers relying on the accelerator. It is
 * transparent to the application and only noted ensure sensible access timeouts
 * are set.
 *
 * The large number maths engine (LNME) uses a dedicated 2kB block of RAM (PKA
 * RAM) for its operations. The operands of the maths operations must be copied
 * into and results out of the PKA ram. This necessitates a significant number
 * of reads and writes for each operation. The bus interface to the RAM only
 * allows for word-aligned reads and writes. The CPU splits the reads and writes
 * from and to general SRAM from unaligned addresses into multiple bus
 * operations while accumulating the data in a register until it is full. The
 * result of this hardware process is that providing buffers such as plaintext
 * CryptoKey keying material to ECC APIs that are word-aligned will
 * significantly speed up the operation and reduce power consumption.
 *
 * The driver implementation does not perform runtime checks for most input
 * parameters. Only values that are likely to have a stochastic element to them
 * are checked (such as whether a driver is already open). Higher input paramter
 * validation coverage is achieved by turning on assertions when compiling the
 * driver.
 *
 * # Supported Curve Types #
 *
 * The driver implementation supports the following curve types for ECDSA:
 *
 * | Curve Type        | Supported |
 * |-------------------|-----------|
 * | Short Weierstrass | Yes       |
 *
 * # Public Key Validation #
 *
 * When performing signature verification, the foreign public key will always be
 * validated. However, the implementation assumes that the cofactor, h, of the
 * curve is 1. This lets us skip the computationally expensive step of
 * multiplying the foreign key by the order and checking if it yields the point
 * at infinity. When the cofactor is 1, this property is implied by validating
 * that the point is not already the point at infinity and that it validates
 * against the curve equation. All curves supplied by default have cofactor = 1.
 */

#ifndef ECDSACC26X4_driverlib__include
#define ECDSACC26X4_driverlib__include

#include <stdint.h>
#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(driverlib/pka.h)

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * @brief   Successful status code.
 *
 * Functions return ECDSA_STATUS_SUCCESS if the function was executed
 * successfully.
 */
#define ECDSA_STATUS_SUCCESS         (0)
#define ECDH_STATUS_SUCCESS          (0)

/*!
 * @brief   Generic error status code.
 *
 * Functions return ECDSA_STATUS_ERROR if the function was not executed
 * successfully.
 */
#define ECDSA_STATUS_ERROR           (-1)

/*!
 * @brief   An error status code returned if the hardware or software resource
 * is currently unavailable.
 *
 * ECDSA driver implementations may have hardware or software limitations on how
 * many clients can simultaneously perform operations. This status code is
 * returned if the mutual exclusion mechanism signals that an operation cannot
 * currently be performed.
 */
#define ECDSA_STATUS_RESOURCE_UNAVAILABLE (-2)

/*!
 * @brief   The r value passed in is larger than the order of the curve.
 *
 * Signature components (r and s) must be integers in the interval [1, n - 1],
 * where n is the order of the curve.
 */
#define ECDSA_STATUS_R_LARGER_THAN_ORDER (-3)

/*!
 * @brief   The s value passed in is larger than the order of the curve.
 *
 * Signature components (r and s) must be integers in the interval [1, n - 1],
 * where n is the order of the curve.
 */
#define ECDSA_STATUS_S_LARGER_THAN_ORDER (-4)

/*!
 * @brief   The public key of the other party does not lie upon the curve.
 *
 * The public key received from the other party does not lie upon the agreed
 * upon curve.
 */
#define ECDSA_STATUS_PUBLIC_KEY_NOT_ON_CURVE (-5)

/*!
 * @brief   A coordinate of the public key of the other party is too large.
 *
 * A coordinate of the public key received from the other party is larger than
 * the prime of the curve. This implies that the point was not correctly
 * generated on that curve.
 */
#define ECDSA_STATUS_PUBLIC_KEY_LARGER_THAN_PRIME (-6)

/*!
  * @brief   The public key of the other party does not lie upon the curve.
  *
  * The public key received from the other party does not lie upon the agreed upon
  * curve.
  */
#define ECDH_STATUS_PUBLIC_KEY_NOT_ON_CURVE (-6)

/*!
 * @brief   The public key to verify against is the point at infinity.
 *
 * The point at infinity is not a valid input.
 */
#define ECDSA_STATUS_POINT_AT_INFINITY (-7)

/*!
 *  @brief  The ongoing operation was canceled.
 */
#define ECDSA_STATUS_CANCELED (-8)

 /*!
  *  @brief  The provided CryptoKey does not match the expected size
  *
  *  The driver expects the private key to have the same length as other curve
  *  parameters and the public key to have a length of twice that plus one.
  *  If the provided CryptoKeys for the public and private keys do not match this
  *  scheme, this error will be returned.
  */
 #define ECDH_STATUS_INVALID_KEY_SIZE (-9)

/*!
 *  @brief  The provided CryptoKey does not match the expected size
 *
 *  The driver expects the private key to have the same length as other curve
 *  parameters and the public key to have a length of twice that plus one.
 *  If the provided CryptoKeys for the public and private keys do not match this
 *  scheme, this error will be returned.
 */
#define ECDSA_STATUS_INVALID_KEY_SIZE (-10)

/*!
 *  @brief Enumeration of curve equations supported.
 *
 *  | Name              | Equation                      |
 *  |-------------------|-------------------------------|
 *  | Short Weierstrass | y^3 = x^2 + a*x + b mod p     |
 */
typedef uint32_t ECCParams_CurveType;

#define ECCParams_CURVE_TYPE_SHORT_WEIERSTRASS_AN3 1U

/*!
 *  @brief  Plaintext CryptoKey structure.
 *
 * This structure contains all the information necessary to access keying material stored
 * in plaintext form in flash or RAM.
 */
typedef struct {
    uint8_t *keyMaterial;
    uint32_t keyLength;
} CryptoKey_Plaintext;

/*!
 *
 *  @brief A structure containing the parameters of an elliptic curve in short Weierstrass form.
 *
 *  Elliptical Curve Cryptography (ECC) prime curve.
 *
 *  The equation used to define the curve is expressed in the short Weierstrass
 *  form y^3 = x^2 + a*x + b
 *
 */
typedef struct ECCParams_CurveParams
{
    const ECCParams_CurveType   curveType;
    const uint8_t               *prime;
    const uint8_t               *a;
    const uint8_t               *b;
    const uint8_t               *order;
    const uint8_t               *generatorX;
    const uint8_t               *generatorY;
    const size_t                length;
    uint8_t                     cofactor;
} ECCParams_CurveParams;

/*!
 *  @brief  Enum for the operation types supported by the driver.
 */
typedef enum
{
    ECDH_OPERATION_TYPE_GENERATE_PUBLIC_KEY   = 1,
    ECDH_OPERATION_TYPE_COMPUTE_SHARED_SECRET = 2,
} ECDH_OperationType;

/*!
 *
 *  @brief The NISTP256 curve parameters in short Weierstrass form.
 *
 */
extern const ECCParams_CurveParams ECCParams_NISTP256;

/*!
 *  @brief  Struct containing the parameters required for verifying a message.
 */
typedef struct {
    const ECCParams_CurveParams     *curve;             /*!< A pointer to the elliptic curve parameters */
    const CryptoKey_Plaintext       *theirPublicKey;    /*!< A pointer to the public key of the party
                                                         *   that signed the hash of the message
                                                         */
    const uint8_t                   *hash;              /*!< A pointer to the hash of the message in
                                                         *   octet string format.
                                                         *   Must be the same length as the other curve parameters.
                                                         */
    const uint8_t                   *r;                 /*!< A pointer to the r component of the received
                                                         *   signature.
                                                         *   Formatted in octet string format.
                                                         *   Must be of the same length
                                                         *   as other params of the curve used.
                                                         */
    const uint8_t                   *s;                 /*!< A pointer to the s component of the received
                                                         *   signature.
                                                         *   Formatted in octet string format.
                                                         *   Must be of the same length
                                                         *   as other params of the curve used.
                                                         */
} ECDSA_OperationVerify;

/*!
 *  @brief  Struct containing the parameters required to compute the shared secret.
 */
typedef struct
{
    const ECCParams_CurveParams *curve;               /*!< A pointer to the elliptic curve parameters for myPrivateKey.
                                                       *   If ECDH_generateKey() was used, this should be the same private key.
                                                       */
    const CryptoKey_Plaintext *myPrivateKey;          /*!< A pointer to the private ECC key which will be used in to
                                                       *   compute the shared secret.
                                                       */
    const CryptoKey_Plaintext *theirPublicKey;        /*!< A pointer to the public key of the party with whom the
                                                       *   shared secret will be generated.
                                                       */
    CryptoKey_Plaintext *sharedSecret;                /*!< A pointer to a CryptoKey which has been initialized blank.
                                                       *   The shared secret will be placed here.
                                                       *   The formatting byte will be filled in by the driver if the
                                                       *   keyMaterialEndianness requires it.
                                                       */
} ECDH_OperationComputeSharedSecret;

/*!
 *  @brief  Opens the ECDSA driver
 *
 *  @sa     ECDSA_close()
 */
void ECDSA_open(void);

/*!
 *  @brief  Closes the ECDSA driver
 *
 *  @pre    ECDSA_open() has to be called first.
 *
 *  @sa     ECDSA_open()
 */
void ECDSA_close(void);

/*!
 *  @brief  Verifies a received signature matches a hash and public key
 *
 *  @pre    The driver must have been opened by calling ECDSA_open() first.
 *
 *  @param [in]     operation       A struct containing the pointers to the
 *                                  buffers necessary to perform the operation.
 *                                  All struct members must be initialized.
 *
 *  @retval #ECDSA_STATUS_SUCCESS                       The operation succeeded.
 *  @retval #ECDSA_STATUS_ERROR                         The operation failed. This is the return status if the signature did not match.
 *  @retval #ECDSA_STATUS_RESOURCE_UNAVAILABLE          The required hardware resource was not available. Try again later.
 *  @retval #ECDSA_STATUS_CANCELED                      The operation was canceled.
 *  @retval #ECDSA_STATUS_R_LARGER_THAN_ORDER           The r value passed in is larger than the order of the curve.
 *  @retval #ECDSA_STATUS_S_LARGER_THAN_ORDER           The s value passed in is larger than the order of the curve.
 *  @retval #ECDSA_STATUS_PUBLIC_KEY_NOT_ON_CURVE       The public key of the other party does not lie upon the curve.
 *  @retval #ECDSA_STATUS_PUBLIC_KEY_LARGER_THAN_PRIME  One of the public key coordinates is larger the the curve's prime.
 *  @retval #ECDSA_STATUS_POINT_AT_INFINITY             The public key to verify against is the point at infinity.
 */
int_fast16_t ECDSA_verify(ECDSA_OperationVerify *operation);

/*!
 *  @brief Computes a shared secret
 *
 *  This secret can be used to generate shared keys for encryption and authentication.
 *
 *  @param      handle              A ECDH handle returned from ECDH_open()
 *
 *  @param      operation       A pointer to a struct containing the requisite
 *
 *  @pre Call ECDH_OperationComputeSharedSecret_init() on \c operation.
 *       Generate a shared secret off-chip or using ECDH_generatePublicKey()
 *
 *  @retval #ECDH_STATUS_SUCCESS                        The operation succeeded.
 */
int_fast16_t ECDH_computeSharedSecret(ECDH_OperationComputeSharedSecret *operation);

/*!
 *  @brief Initializes a plaintext CryptoKey
 *
 *  @param [in]     keyHandle   Pointer to a CryptoKey_Plaintext
 *  @param [in]     key         Pointer to keying material
 *  @param [in]     keyLength   Length of keying material in bytes
 */
void CryptoKeyPlaintext_initKey(CryptoKey_Plaintext *keyHandle,
                                uint8_t *key,
                                size_t keyLength);

#ifdef __cplusplus
}
#endif

#endif /* ECDSACC26X4_driverlib__include */
