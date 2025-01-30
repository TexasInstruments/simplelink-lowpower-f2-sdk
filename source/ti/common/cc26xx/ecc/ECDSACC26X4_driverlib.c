/******************************************************************************

 @file  ECDSACC26X4_driverlib.c

 @brief This module implements an ECDSA driver using only driverlib APIs
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

#include <stdint.h>
#include <stdbool.h>

#include "ti/common/cc26xx/ecc/ECDSACC26X4_driverlib.h"
#include "mcuboot_config.h"

#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(inc/hw_memmap.h)
#include DeviceFamily_constructPath(inc/hw_types.h)
#include DeviceFamily_constructPath(inc/hw_pka.h)
#include DeviceFamily_constructPath(inc/hw_pka_ram.h)
#include DeviceFamily_constructPath(driverlib/pka.h)
#include DeviceFamily_constructPath(driverlib/prcm.h)

/* Octet string format requires an extra byte at the start of the public key */
#define OCTET_STRING_OFFSET 1

#define SCRATCH_KEY_OFFSET 512
#define SCRATCH_KEY_SIZE 96
#define SCRATCH_PRIVATE_KEY ((uint32_t *)(PKA_RAM_BASE + SCRATCH_KEY_OFFSET))
#define SCRATCH_PUBLIC_X ((uint32_t *)(PKA_RAM_BASE + SCRATCH_KEY_OFFSET + 1 * SCRATCH_KEY_SIZE))
#define SCRATCH_PUBLIC_Y ((uint32_t *)(PKA_RAM_BASE + SCRATCH_KEY_OFFSET + 2 * SCRATCH_KEY_SIZE))

#define SCRATCH_BUFFER_OFFSET 1024
#define SCRATCH_BUFFER_SIZE 256
#define SCRATCH_BUFFER_0 ((uint32_t *)(PKA_RAM_BASE + SCRATCH_BUFFER_OFFSET + 0 * SCRATCH_BUFFER_SIZE))
#define SCRATCH_BUFFER_1 ((uint32_t *)(PKA_RAM_BASE + SCRATCH_BUFFER_OFFSET + 1 * SCRATCH_BUFFER_SIZE))

/*!
 *  @brief NIST P256 Curve Parameters
 */
const ECCParams_CurveParams ECCParams_NISTP256 =
{
    .curveType      = ECCParams_CURVE_TYPE_SHORT_WEIERSTRASS_AN3,
    .length         = NISTP256_PARAM_SIZE_BYTES,
    .prime          = NISTP256_prime.byte,
    .order          = NISTP256_order.byte,
    .a              = NISTP256_a.byte,
    .b              = NISTP256_b.byte,
    .generatorX     = NISTP256_generator.x.byte,
    .generatorY     = NISTP256_generator.y.byte,
    .cofactor       = 1
};

/* Forward declarations */
static void CryptoUtils_reverseCopyPad(const void *source,
                                       uint32_t *destination,
                                       size_t sourceLength);
static inline bool CryptoUtils_buffersMatchWordAligned(const volatile uint32_t *volatile buffer0,
                                                       const volatile uint32_t *volatile buffer1,
                                                       size_t bufferByteLength);
static inline int_fast16_t ECDSACC26X4_runVerifyPolling(ECDSA_OperationVerify *operation);
static int_fast16_t ECDSACC26X4_convertPKAErrorStatus(uint32_t pkaResult);

/* Static globals */
static bool periphPwrRequired;
static uint32_t resultAddress;
static uint32_t scratchBuffer0Size = SCRATCH_BUFFER_SIZE;
static uint32_t scratchBuffer1Size = SCRATCH_BUFFER_SIZE;
static uint32_t resultPKAMemAddr;


/*
 *  ======== CryptoUtils_reverseCopyPad ========
 */
/*
*  ======== CryptoUtils_copyPad ========
*/
#ifndef ECDH_BIG_ENDIAN_KEY
static void CryptoUtils_copyPad(const void *source, uint32_t *destination, size_t sourceLength)
{
    uint32_t i;
    uint8_t remainder;
    uint32_t temp;
    uint8_t *tempBytePointer;
    const uint8_t *sourceBytePointer;

    remainder         = sourceLength % sizeof(uint32_t);
    temp              = 0;
    tempBytePointer   = (uint8_t *)&temp;
    sourceBytePointer = (uint8_t *)source;

    /* Copy source to destination starting at the end of source and the
     * beginning of destination.
     * We assemble each word in normal order and write one word at a
     * time since the PKA_RAM requires word-aligned reads and writes.
     */

    for (i = 0; i < sourceLength / sizeof(uint32_t); i++)
    {
        uint32_t sourceOffset = sizeof(uint32_t) * i;

        tempBytePointer[0] = sourceBytePointer[sourceOffset + 0];
        tempBytePointer[1] = sourceBytePointer[sourceOffset + 1];
        tempBytePointer[2] = sourceBytePointer[sourceOffset + 2];
        tempBytePointer[3] = sourceBytePointer[sourceOffset + 3];

        *(destination + i) = temp;
    }

    /* Reset to 0 so we do not have to zero-out individual bytes */
    temp = 0;

    /* If sourceLength is not a word-multiple, we need to copy over the
     * remaining bytes and zero pad the word we are writing to PKA_RAM.
     */
    if (remainder == 1)
    {

        tempBytePointer[0] = sourceBytePointer[0];

        /* i is reused from the loop above. This write zero-pads the
         * destination buffer to word-length.
         */
        *(destination + i) = temp;
    }
    else if (remainder == 2)
    {

        tempBytePointer[0] = sourceBytePointer[0];
        tempBytePointer[1] = sourceBytePointer[1];

        *(destination + i) = temp;
    }
    else if (remainder == 3)
    {

        tempBytePointer[0] = sourceBytePointer[0];
        tempBytePointer[1] = sourceBytePointer[1];
        tempBytePointer[2] = sourceBytePointer[2];

        *(destination + i) = temp;
    }
}
#endif
static void CryptoUtils_reverseCopyPad(const void *source,
                                       uint32_t *destination,
                                       size_t sourceLength)
{
    uint32_t i;
    uint8_t remainder;
    uint32_t temp;
    uint8_t *tempBytePointer;
    const uint8_t *sourceBytePointer;

    remainder = sourceLength % sizeof(uint32_t);
    temp = 0;
    tempBytePointer = (uint8_t *)&temp;
    sourceBytePointer = (uint8_t *)source;

    /*
     * Copy source to destination starting at the end of source and the
     * beginning of destination.
     * We assemble each word in byte-reversed order and write one word at a
     * time since the PKA_RAM requires word-aligned reads and writes.
     */

    for (i = 0; i < sourceLength / sizeof(uint32_t); i++)
    {
        uint32_t sourceOffset = sourceLength - 1 - sizeof(uint32_t) * i;

        tempBytePointer[3] = sourceBytePointer[sourceOffset - 3];
        tempBytePointer[2] = sourceBytePointer[sourceOffset - 2];
        tempBytePointer[1] = sourceBytePointer[sourceOffset - 1];
        tempBytePointer[0] = sourceBytePointer[sourceOffset - 0];

        *(destination + i) = temp;
    }

    /* Reset to 0 so we do not have to zero-out individual bytes */
    temp = 0;

    /*
     * If sourceLength is not a word-multiple, we need to copy over the
     * remaining bytes and zero pad the word we are writing to PKA_RAM.
     */
    if (remainder == 1)
    {

        tempBytePointer[0] = sourceBytePointer[0];

        /*
         * i is reused from the loop above. This write zero-pads the
         * destination buffer to word-length.
         */
        *(destination + i) = temp;
    }
    else if (remainder == 2)
    {

        tempBytePointer[0] = sourceBytePointer[1];
        tempBytePointer[1] = sourceBytePointer[0];

        *(destination + i) = temp;
    }
    else if (remainder == 3)
    {

        tempBytePointer[0] = sourceBytePointer[2];
        tempBytePointer[1] = sourceBytePointer[1];
        tempBytePointer[2] = sourceBytePointer[0];

        *(destination + i) = temp;
    }
}

/*
 *  ======== CryptoUtils_buffersMatchWordAligned ========
 */
static inline bool CryptoUtils_buffersMatchWordAligned(const volatile uint32_t *volatile buffer0,
                                                       const volatile uint32_t *volatile buffer1,
                                                       size_t bufferByteLength)
{
    volatile uint32_t tempResult = 0;
    uint32_t word0;
    uint32_t word1;
    size_t i;

    /* We could skip the branch and just set tempResult equal to the
     * statement below for the same effect but this is more explicit.
     */
    if (bufferByteLength % sizeof(uint32_t) != 0)
    {
        return false;
    }

    /* XOR each 32-bit word of the buffer together and OR the results.
     * If the OR'd result is non-zero, the buffers do not match.
     * There is no branch based on the content of the buffers here to avoid
     * timing attacks.
     */
    for (i = 0; i < bufferByteLength / sizeof(uint32_t); i++)
    {
        word0 = buffer0[i];
        word1 = buffer1[i];

        tempResult |= word0 ^ word1;
    }

    return tempResult == 0;
}

#ifdef ECDH_BIG_ENDIAN_KEY
/*
 *  ======== CryptoUtils_reverseBufferBytewise ========
 */
void CryptoUtils_reverseBufferBytewise(void *buffer, size_t bufferByteLength)
{
    uint8_t *bufferLow  = buffer;
    uint8_t *bufferHigh = bufferLow + bufferByteLength - 1;
    uint8_t tmp;

    while (bufferLow < bufferHigh)
    {
        tmp         = *bufferLow;
        *bufferLow  = *bufferHigh;
        *bufferHigh = tmp;
        bufferLow++;
        bufferHigh--;
    }
}
#endif
/*
 *  ======== ECDHCC26X2_getKeyResult ========
 */
static int_fast16_t ECDHCC26X2_getKeyResult(CryptoKey_Plaintext *key,
                                            const ECCParams_CurveParams *curve,
                                            ECDH_OperationType opType)
{
    uint8_t *keyMaterial;
    uint8_t *xCoordinate;
    uint8_t *yCoordinate;
    size_t keyLength;
    size_t bytesToBeWritten;

    /* Keep track of number of bytes of key written using driverlib function and check if it does not exceed the length
     * of the keyMaterial provided in CryptoKey
     */
    bytesToBeWritten = 0;

    /* Initialize the coordinates to NULL */
    xCoordinate = NULL;
    yCoordinate = NULL;


    /*
     * Support for both Plaintext and KeyStore keys for myPrivateKey
     */
    keyMaterial = key->keyMaterial;
    keyLength   = key->keyLength;

    bytesToBeWritten += 2 * curve->length;
#ifdef ECDH_BIG_ENDIAN_KEY
    if(opType == ECDH_OPERATION_TYPE_GENERATE_PUBLIC_KEY)
    {
        keyMaterial[0] = 0x04;
    }
    bytesToBeWritten += OCTET_STRING_OFFSET;
#endif

    if(bytesToBeWritten != keyLength)
    {
        return -1;
    }


    xCoordinate = keyMaterial;
    yCoordinate = keyMaterial + curve->length;

    PKAEccMultiplyGetResult(xCoordinate, yCoordinate, resultPKAMemAddr, curve->length);

#ifdef ECDH_BIG_ENDIAN_KEY
    /* Byte-reverse integer X coordinate for octet string format */
    CryptoUtils_reverseBufferBytewise(xCoordinate, curve->length);

    /* curve type = ECCParams_CURVE_TYPE_SHORT_WEIERSTRASS_AN3 */
    /* Byte-reverse integer Y coordinate for octet string format */
    CryptoUtils_reverseBufferBytewise(yCoordinate, curve->length);
#endif
    return ECDH_STATUS_SUCCESS;



}
static inline int_fast16_t ECDH_computeSharedSecretPolling(ECDH_OperationComputeSharedSecret *operation) {

    uint32_t pkaResult;

    /*MULT_PRIVATE_KEY_BY_PUB_KEY*/
    while (PKAGetOpsStatus() == PKA_STATUS_OPERATION_BUSY) {}

    /* If we are using a short Weierstrass curve, we need to validate the public key */
    pkaResult = PKAEccVerifyPublicKeyWeierstrassStart((uint8_t *)SCRATCH_PUBLIC_X,
                                                      (uint8_t *)SCRATCH_PUBLIC_Y,
                                                      operation->curve->prime,
                                                      operation->curve->a,
                                                      operation->curve->b,
                                                      operation->curve->order,
                                                      operation->curve->length);

    if (pkaResult != PKA_STATUS_SUCCESS)
    {
        return ECDH_STATUS_PUBLIC_KEY_NOT_ON_CURVE;
    }

    /* Perform an elliptic curve multiplication on a short Weierstrass curve */
    PKAEccMultiplyStart((uint8_t *)SCRATCH_PRIVATE_KEY,
                        (uint8_t *)SCRATCH_PUBLIC_X,
                        (uint8_t *)SCRATCH_PUBLIC_Y,
                        operation->curve->prime,
                        operation->curve->a,
                        operation->curve->b,
                        operation->curve->length,
                        &resultPKAMemAddr);

    while (PKAGetOpsStatus() == PKA_STATUS_OPERATION_BUSY) {}

    /*MULT_PRIVATE_KEY_BY_PUB_KEY_RESULT*/
    ECDHCC26X2_getKeyResult(operation->sharedSecret,
                                     operation->curve,
                                     ECDH_OPERATION_TYPE_COMPUTE_SHARED_SECRET);

    while (PKAGetOpsStatus() == PKA_STATUS_OPERATION_BUSY) {}

    /*RETURN*/
    return ECDH_STATUS_SUCCESS;
}

/*
 *  ======== ECDSACC26X4_runVerifyPolling ========
 */
static inline int_fast16_t ECDSACC26X4_runVerifyPolling(ECDSA_OperationVerify *operation)
{
    uint32_t pkaResult;

    /* VERIFY_R_S_IN_RANGE */

    /* Convert r from OS format to little-endian integer */
    CryptoUtils_reverseCopyPad(operation->r,
                               SCRATCH_BUFFER_0,
                               operation->curve->length);

    PKABigNumCmpStart((uint8_t *)SCRATCH_BUFFER_0,
                      operation->curve->order,
                      operation->curve->length);

    while (PKAGetOpsStatus() == PKA_STATUS_OPERATION_BUSY) {}

    pkaResult = PKABigNumCmpGetResult();

    if (pkaResult != PKA_STATUS_A_LESS_THAN_B)
    {
        return ECDSA_STATUS_R_LARGER_THAN_ORDER;
    }

    /* Convert s from OS format to little-endian integer */
    CryptoUtils_reverseCopyPad(operation->s,
                               SCRATCH_BUFFER_0,
                               operation->curve->length);

    PKABigNumCmpStart((uint8_t *)SCRATCH_BUFFER_0,
                      operation->curve->order,
                      operation->curve->length);

    while (PKAGetOpsStatus() == PKA_STATUS_OPERATION_BUSY) {}

    pkaResult = PKABigNumCmpGetResult();

    if (pkaResult != PKA_STATUS_A_LESS_THAN_B)
    {
        return ECDSA_STATUS_S_LARGER_THAN_ORDER;
    }

    /* VERIFY_VALIDATE_PUBLIC_KEY */

    CryptoUtils_reverseCopyPad(operation->theirPublicKey->keyMaterial + OCTET_STRING_OFFSET,
                               SCRATCH_PUBLIC_X,
                               operation->curve->length);

    CryptoUtils_reverseCopyPad(operation->theirPublicKey->keyMaterial + OCTET_STRING_OFFSET + operation->curve->length,
                               SCRATCH_PUBLIC_Y,
                               operation->curve->length);

    pkaResult = PKAEccVerifyPublicKeyWeierstrassStart((uint8_t *)SCRATCH_PUBLIC_X,
                                                      (uint8_t *)SCRATCH_PUBLIC_Y,
                                                      operation->curve->prime,
                                                      operation->curve->a,
                                                      operation->curve->b,
                                                      operation->curve->order,
                                                      operation->curve->length);

    if (pkaResult != PKA_STATUS_SUCCESS)
    {
        return ECDSACC26X4_convertPKAErrorStatus(pkaResult);
    }

    /* VERIFY_COMPUTE_S_INV */

    /* Convert s from OS format to little-endian integer */
    CryptoUtils_reverseCopyPad(operation->s,
                               SCRATCH_BUFFER_0,
                               operation->curve->length);

    PKABigNumInvModStart((uint8_t *)SCRATCH_BUFFER_0,
                         operation->curve->length,
                         operation->curve->order,
                         operation->curve->length,
                         &resultAddress);

    while (PKAGetOpsStatus() == PKA_STATUS_OPERATION_BUSY) {}

    /* VERIFY_COMPUTE_S_INV_RESULT */

    pkaResult = PKABigNumInvModGetResult((uint8_t *)SCRATCH_BUFFER_1,
                                         operation->curve->length,
                                         resultAddress);

    if (pkaResult != PKA_STATUS_SUCCESS)
    {
        return ECDSACC26X4_convertPKAErrorStatus(pkaResult);
    }

    /* VERIFY_MULT_S_INV_HASH */

    /* Convert hash from OS format to little-endian integer */
    CryptoUtils_reverseCopyPad(operation->hash,
                               SCRATCH_BUFFER_0,
                               operation->curve->length);

    PKABigNumMultiplyStart((uint8_t *)SCRATCH_BUFFER_1,
                           operation->curve->length,
                           (uint8_t *)SCRATCH_BUFFER_0,
                           operation->curve->length,
                           &resultAddress);

    while (PKAGetOpsStatus() == PKA_STATUS_OPERATION_BUSY) {}

    /* VERIFY_MULT_S_INV_HASH_RESULT */

    scratchBuffer0Size = SCRATCH_BUFFER_SIZE;

    pkaResult = PKABigNumMultGetResult((uint8_t *)SCRATCH_BUFFER_0,
                                       &scratchBuffer0Size,
                                       resultAddress);

    if (pkaResult != PKA_STATUS_SUCCESS)
    {
        return ECDSACC26X4_convertPKAErrorStatus(pkaResult);
    }

    /* VERIFY_S_INV_MULT_HASH_MOD_N */

    PKABigNumModStart((uint8_t *)SCRATCH_BUFFER_0,
                      scratchBuffer0Size,
                      operation->curve->order,
                      operation->curve->length,
                      &resultAddress);

    while (PKAGetOpsStatus() == PKA_STATUS_OPERATION_BUSY) {}

    /* VERIFY_S_INV_MULT_HASH_MOD_N_RESULT */
    // Check previous result
    pkaResult = PKABigNumModGetResult((uint8_t *)SCRATCH_BUFFER_0,
                                      operation->curve->length,
                                      resultAddress);

    scratchBuffer0Size = operation->curve->length;

    if (pkaResult != PKA_STATUS_SUCCESS)
    {
        return ECDSACC26X4_convertPKAErrorStatus(pkaResult);
    }

    /* VERIFY_MULT_G */

    PKAEccMultiplyStart((uint8_t *)SCRATCH_BUFFER_0,
                        operation->curve->generatorX,
                        operation->curve->generatorY,
                        operation->curve->prime,
                        operation->curve->a,
                        operation->curve->b,
                        operation->curve->length,
                        &resultAddress);

    while (PKAGetOpsStatus() == PKA_STATUS_OPERATION_BUSY) {}

    /* VERIFY_MULT_G_RESULT */

    pkaResult = PKAEccMultiplyGetResult((uint8_t *)SCRATCH_BUFFER_0,
                                        (uint8_t *)SCRATCH_BUFFER_0 + operation->curve->length,
                                        resultAddress,
                                        operation->curve->length);

    if (pkaResult != PKA_STATUS_SUCCESS)
    {
        return ECDSACC26X4_convertPKAErrorStatus(pkaResult);
    }

    /* VERIFY_MULT_S_INV_R */

    /* Convert r from OS format to little-endian integer */
    CryptoUtils_reverseCopyPad(operation->r,
                               SCRATCH_PRIVATE_KEY,
                               operation->curve->length);

    PKABigNumMultiplyStart((uint8_t *)SCRATCH_BUFFER_1,
                           operation->curve->length,
                           (uint8_t *)SCRATCH_PRIVATE_KEY,
                           operation->curve->length,
                           &resultAddress);

    while (PKAGetOpsStatus() == PKA_STATUS_OPERATION_BUSY) {}

    /* VERIFY_MULT_S_INV_R_RESULT */

    scratchBuffer1Size = SCRATCH_BUFFER_SIZE;

    pkaResult = PKABigNumMultGetResult((uint8_t *)SCRATCH_BUFFER_1,
                                       &scratchBuffer1Size,
                                       resultAddress);

    if (pkaResult != PKA_STATUS_SUCCESS)
    {
        return ECDSACC26X4_convertPKAErrorStatus(pkaResult);
    }

    /* VERIFY_MULT_S_INV_R_MOD_N */

    PKABigNumModStart((uint8_t *)SCRATCH_BUFFER_1,
                      scratchBuffer1Size,
                      operation->curve->order,
                      operation->curve->length,
                      &resultAddress);

    while (PKAGetOpsStatus() == PKA_STATUS_OPERATION_BUSY) {}

    /* VERIFY_MULT_S_INV_R_MOD_N_RESULT */

    pkaResult = PKABigNumModGetResult((uint8_t *)SCRATCH_BUFFER_1,
                                      operation->curve->length,
                                      resultAddress);

    scratchBuffer1Size = operation->curve->length;

    if (pkaResult != PKA_STATUS_SUCCESS)
    {
        return ECDSACC26X4_convertPKAErrorStatus(pkaResult);
    }

    /* VERIFY_MULT_PUB_KEY */

    CryptoUtils_reverseCopyPad(operation->theirPublicKey->keyMaterial + OCTET_STRING_OFFSET,
                               SCRATCH_PUBLIC_X,
                               operation->curve->length);

    CryptoUtils_reverseCopyPad(operation->theirPublicKey->keyMaterial + OCTET_STRING_OFFSET + operation->curve->length,
                               SCRATCH_PUBLIC_Y,
                               operation->curve->length);

    PKAEccMultiplyStart((uint8_t *)SCRATCH_BUFFER_1,
                        (uint8_t *)SCRATCH_PUBLIC_X,
                        (uint8_t *)SCRATCH_PUBLIC_Y,
                        operation->curve->prime,
                        operation->curve->a,
                        operation->curve->b,
                        operation->curve->length,
                        &resultAddress);

    while (PKAGetOpsStatus() == PKA_STATUS_OPERATION_BUSY) {}

    /* VERIFY_MULT_PUB_KEY_RESULT */

    pkaResult = PKAEccMultiplyGetResult((uint8_t *)SCRATCH_BUFFER_1,
                                        (uint8_t *)SCRATCH_BUFFER_1 + operation->curve->length,
                                        resultAddress,
                                        operation->curve->length);

    if (pkaResult != PKA_STATUS_SUCCESS)
    {
        return ECDSACC26X4_convertPKAErrorStatus(pkaResult);
    }

    /* VERIFY_ADD_MULT_RESULTS */

    PKAEccAddStart((uint8_t *)SCRATCH_BUFFER_0,
                   (uint8_t *)SCRATCH_BUFFER_0 + operation->curve->length,
                   (uint8_t *)SCRATCH_BUFFER_1,
                   (uint8_t *)SCRATCH_BUFFER_1 + operation->curve->length,
                   operation->curve->prime,
                   operation->curve->a,
                   operation->curve->length,
                   &resultAddress);

    while (PKAGetOpsStatus() == PKA_STATUS_OPERATION_BUSY) {}

    /* VERIFY_ADD_MULT_RESULTS_RESULT */

    pkaResult = PKAEccAddGetResult((uint8_t *)SCRATCH_BUFFER_0,
                                   NULL,
                                   resultAddress,
                                   operation->curve->length);

    if (pkaResult != PKA_STATUS_SUCCESS)
    {
        return ECDSACC26X4_convertPKAErrorStatus(pkaResult);
    }

    /* VERIFY_POINTX_MOD_N */

    PKABigNumModStart((uint8_t *)SCRATCH_BUFFER_0,
                      operation->curve->length,
                      operation->curve->order,
                      operation->curve->length,
                      &resultAddress);

    while (PKAGetOpsStatus() == PKA_STATUS_OPERATION_BUSY) {}

    /* VERIFY_POINTX_MOD_N_RESULT */

    pkaResult = PKABigNumModGetResult((uint8_t *)SCRATCH_BUFFER_0,
                                      operation->curve->length,
                                      resultAddress);

    if (pkaResult != PKA_STATUS_SUCCESS)
    {
        return ECDSACC26X4_convertPKAErrorStatus(pkaResult);
    }

    /* VERIFY_COMPARE_RESULT_R */

    CryptoUtils_reverseCopyPad(operation->r,
                               SCRATCH_PRIVATE_KEY,
                               operation->curve->length);

    /* The CPU will rearrange each word in r to take care of aligned
     * access. The scratch buffer location is already word aligned.
     */
    if (!CryptoUtils_buffersMatchWordAligned(SCRATCH_BUFFER_0,
                                             SCRATCH_PRIVATE_KEY,
                                             operation->curve->length))
    {
        return ECDSA_STATUS_ERROR;
    }

    return ECDSA_STATUS_SUCCESS;
}

/*
 *  ======== ECDSACC26X4_convertPKAErrorStatus ========
 */
static int_fast16_t ECDSACC26X4_convertPKAErrorStatus(uint32_t pkaResult)
{
    /* Make sure there is no keying material remaining in PKA RAM */
    PKAClearPkaRam();

    switch (pkaResult)
    {
        case PKA_STATUS_X_ZERO: /* Fall through */
        case PKA_STATUS_Y_ZERO: /* Fall through */
        case PKA_STATUS_RESULT_0:
            /*
             * Theoretically, PKA_STATUS_RESULT_0 might be caused by other
             * operations failing but the only one that really should yield
             * 0 is ECC multiplication with invalid inputs that yield the
             * point at infinity.
             */
            return ECDSA_STATUS_POINT_AT_INFINITY;

        case PKA_STATUS_X_LARGER_THAN_PRIME: /* Fall through */
        case PKA_STATUS_Y_LARGER_THAN_PRIME:
            return ECDSA_STATUS_PUBLIC_KEY_LARGER_THAN_PRIME;

        case PKA_STATUS_POINT_NOT_ON_CURVE:
            return ECDSA_STATUS_PUBLIC_KEY_NOT_ON_CURVE;

        default:
            return ECDSA_STATUS_ERROR;
    }
}

/*
 *  ======== ECDSA_verify ========
 */
int_fast16_t ECDSA_verify(ECDSA_OperationVerify *operation)
{
    /* Validate key sizes to make sure octet string format is used */
    if ((operation->theirPublicKey->keyLength != (2 * operation->curve->length + OCTET_STRING_OFFSET)) ||
        (operation->theirPublicKey->keyMaterial[0] != 0x04))
    {
        return ECDSA_STATUS_INVALID_KEY_SIZE;
    }

    return ECDSACC26X4_runVerifyPolling(operation);
}


/*
 *  ======== ECDSA_open ========
 */
void ECDSA_open(void)
{
    /* Enable power to peripheral domain if necessary */
    periphPwrRequired = true;
    if (PRCMPowerDomainsAllOn(PRCM_DOMAIN_PERIPH) != PRCM_DOMAIN_POWER_ON)
    {
        periphPwrRequired = false;

        PRCMPowerDomainOn(PRCM_DOMAIN_PERIPH);
        while (PRCMPowerDomainsAllOn(PRCM_DOMAIN_PERIPH) != PRCM_DOMAIN_POWER_ON);
    }

    /* Power up and enable clock for PKA module. */
    PRCMPeripheralRunEnable(PRCM_PERIPH_PKA);
    PRCMLoadSet();
    while (!PRCMLoadGet()) {}
}

/*
 *  ======== ECDSA_close ========
 */
void ECDSA_close(void)
{
    /*
     * Only power off the peripheral domain if it was originally off when calling
     * ECDSA_open()
     */
    if (!periphPwrRequired)
    {
        PRCMPowerDomainOff(PRCM_DOMAIN_PERIPH);
        while (PRCMPowerDomainsAllOn(PRCM_DOMAIN_PERIPH) != PRCM_DOMAIN_POWER_OFF);
    }

    /* Power down and disable clock for PKA module */
    PRCMPeripheralRunDisable(PRCM_PERIPH_PKA);
    PRCMLoadSet();
    while (!PRCMLoadGet()) {}
}


/*
 *  ======== ECDH_computeSharedSecret ========
 */
int_fast16_t ECDH_computeSharedSecret(ECDH_OperationComputeSharedSecret *operation) {

    int_fast16_t status;
    uint8_t *myPrivateKeyMaterial;
    size_t myPrivateKeyLength;
    uint8_t *theirPublicKeyMaterial;
    size_t theirPublicKeyLength;
    size_t sharedSecretKeyLength;

    myPrivateKeyMaterial = operation->myPrivateKey->keyMaterial;
    myPrivateKeyLength   = operation->myPrivateKey->keyLength;

    theirPublicKeyMaterial = operation->theirPublicKey->keyMaterial;
    theirPublicKeyLength   = operation->theirPublicKey->keyLength;

    sharedSecretKeyLength = operation->sharedSecret->keyLength;

#ifdef ECDH_BIG_ENDIAN_KEY
    if ((myPrivateKeyLength != operation->curve->length) ||
            (theirPublicKeyLength != 2 * operation->curve->length + OCTET_STRING_OFFSET) ||
            (theirPublicKeyMaterial[0] != 0x04) ||
            (sharedSecretKeyLength != 2 * operation->curve->length + OCTET_STRING_OFFSET))
    {
        return ECDH_STATUS_INVALID_KEY_SIZE;
    }
#else
    if ((myPrivateKeyLength != operation->curve->length) ||
            (theirPublicKeyLength != 2 * operation->curve->length) ||
            (sharedSecretKeyLength != 2 * operation->curve->length))
    {
        return ECDH_STATUS_INVALID_KEY_SIZE;
    }
#endif

#ifdef ECDH_BIG_ENDIAN_KEY
    CryptoUtils_reverseCopyPad(myPrivateKeyMaterial, SCRATCH_PRIVATE_KEY, operation->curve->length);

    CryptoUtils_reverseCopyPad(theirPublicKeyMaterial + OCTET_STRING_OFFSET,
                               SCRATCH_PUBLIC_X,
                               operation->curve->length);

    CryptoUtils_reverseCopyPad(theirPublicKeyMaterial + OCTET_STRING_OFFSET + operation->curve->length,
                               SCRATCH_PUBLIC_Y,
                               operation->curve->length);
#else
    CryptoUtils_copyPad(myPrivateKeyMaterial, SCRATCH_PRIVATE_KEY, operation->curve->length);

    CryptoUtils_copyPad(theirPublicKeyMaterial, SCRATCH_PUBLIC_X, operation->curve->length);

    CryptoUtils_copyPad(theirPublicKeyMaterial + operation->curve->length,
                        SCRATCH_PUBLIC_Y,
                        operation->curve->length);
#endif
    status = ECDH_computeSharedSecretPolling(operation);

    PKAClearPkaRam();

    return status;
}


/*
 *  ======== CryptoKeyPlaintext_initKey ========
 */
void CryptoKeyPlaintext_initKey(CryptoKey_Plaintext *keyHandle,
                                uint8_t *key,
                                size_t keyLength)
{
    keyHandle->keyMaterial = key;
    keyHandle->keyLength = keyLength;
}
