/******************************************************************************

 *  @file  ecdsa_lpf3.c
 *
 *  @brief      This modules implements ECDSA driver for the LPF3 devices with
 *              following restrictions to minimize code size
 *              - Polling return behaviour only
 *              - Verify only
 *              - NISTP256 curve only

 Group: CMCU
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2023-2025, Texas Instruments Incorporated
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
#include <stddef.h>

#include <third_party/ecc/include/ECCSW.h>
#include "ecdsa_lpf3.h"

/* Defines */

/* Octet string format requires an extra byte at the start of the public key */
#define OCTET_STRING_OFFSET 1

/*
 * Curve parameters are formatted as little-endian integers with a prepended
 * length word in words as required by the ECC SW library.
 */

/*
 * NIST P256 curve params in little endian format.
 * byte[0-3] are the param length word as required by the ECC SW library.
 * byte[4] is the least significant byte of the curve parameter.
 */
const ECC_NISTP256_Param ECC_NISTP256_generatorX = {
    .byte = {0x08, 0x00, 0x00, 0x00, /* Length word prefix */
             0x96, 0xc2, 0x98, 0xd8, 0x45, 0x39, 0xa1, 0xf4,
             0xa0, 0x33, 0xeb, 0x2d, 0x81, 0x7d, 0x03, 0x77,
             0xf2, 0x40, 0xa4, 0x63, 0xe5, 0xe6, 0xbc, 0xf8,
             0x47, 0x42, 0x2c, 0xe1, 0xf2, 0xd1, 0x17, 0x6b}
};

const ECC_NISTP256_Param ECC_NISTP256_generatorY = {
    .byte = {0x08, 0x00, 0x00, 0x00, /* Length word prefix */
             0xf5, 0x51, 0xbf, 0x37, 0x68, 0x40, 0xb6, 0xcb,
             0xce, 0x5e, 0x31, 0x6b, 0x57, 0x33, 0xce, 0x2b,
             0x16, 0x9e, 0x0f, 0x7c, 0x4a, 0xeb, 0xe7, 0x8e,
             0x9b, 0x7f, 0x1a, 0xfe, 0xe2, 0x42, 0xe3, 0x4f}
};

const ECC_NISTP256_Param ECC_NISTP256_prime = {
    .byte = {0x08, 0x00, 0x00, 0x00, /* Length word prefix */
             0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
             0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,
             0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
             0x01, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff}
};

const ECC_NISTP256_Param ECC_NISTP256_a = {
    .byte = {0x08, 0x00, 0x00, 0x00, /* Length word prefix */
             0xfc, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
             0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,
             0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
             0x01, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff}
};

const ECC_NISTP256_Param ECC_NISTP256_b = {
    .byte = {0x08, 0x00, 0x00, 0x00, /* Length word prefix */
             0x4b, 0x60, 0xd2, 0x27, 0x3e, 0x3c, 0xce, 0x3b,
             0xf6, 0xb0, 0x53, 0xcc, 0xb0, 0x06, 0x1d, 0x65,
             0xbc, 0x86, 0x98, 0x76, 0x55, 0xbd, 0xeb, 0xb3,
             0xe7, 0x93, 0x3a, 0xaa, 0xd8, 0x35, 0xc6, 0x5a}
};

const ECC_NISTP256_Param ECC_NISTP256_order = {
    .byte = {0x08, 0x00, 0x00, 0x00, /* Length word prefix */
             0x51, 0x25, 0x63, 0xfc, 0xc2, 0xca, 0xb9, 0xf3,
             0x84, 0x9e, 0x17, 0xa7, 0xad, 0xfa, 0xe6, 0xbc,
             0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
             0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff}
};

/*
 * Curve params in Montgomery domain are used for public key validation only
 * and are not prefixed with a length word.
 */
const ECC_NISTP256_Param ECC_NISTP256_k_mont = {
    .byte = {0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
             0xff, 0xff, 0xff, 0xff, 0xfb, 0xff, 0xff, 0xff,
             0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
             0xfd, 0xff, 0xff, 0xff, 0x04, 0x00, 0x00, 0x00}
};

const ECC_NISTP256_Param ECC_NISTP256_a_mont = {
    .byte = {0xfc, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
             0xff, 0xff, 0xff, 0xff, 0x03, 0x00, 0x00, 0x00,
             0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
             0x04, 0x00, 0x00, 0x00, 0xfc, 0xff, 0xff, 0xff}
};

const ECC_NISTP256_Param ECC_NISTP256_b_mont = {
    .byte = {0xdf, 0xbd, 0xc4, 0x29, 0x62, 0xdf, 0x9c, 0xd8,
             0x90, 0x30, 0x84, 0x78, 0xcd, 0x05, 0xf0, 0xac,
             0xd6, 0x2e, 0x21, 0xf7, 0xab, 0x20, 0xa2, 0xe5,
             0x34, 0x48, 0x87, 0x04, 0x1d, 0x06, 0x30, 0xdc}
};


/*
 *  ======== CryptoUtils_reverseCopyPad ========
 */
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
 *  ======== ECDSA_verify ========
 */
int_fast16_t ECDSA_verify(ECDSA_OperationVerify *operation)
{
    int_fast16_t returnStatus = ECDSA_STATUS_ERROR;
    uint8_t eccStatus;
    uint32_t eccWorkZone[ECDSA_LPF3_ECC_WORKZONE_WORDS];

    /* Initialize eccState with NIST-P256 curve */
    ECC_State eccState =
    {
        .data_Gx     = ECC_NISTP256_generatorX.word,
        .data_Gy     = ECC_NISTP256_generatorY.word,
        .data_p      = ECC_NISTP256_prime.word,
        .data_r      = ECC_NISTP256_order.word,
        .data_a      = ECC_NISTP256_a.word,
        .data_b      = ECC_NISTP256_b.word,
        .data_a_mont = ECC_NISTP256_a_mont.word,
        .data_b_mont = ECC_NISTP256_b_mont.word,
        .data_k_mont = ECC_NISTP256_k_mont.word,
        .win         = ECDSA_LPF3_ECC_WINDOW_SIZE,

        .workzone    = eccWorkZone,
    };

    /* Validate key sizes to make sure octet string format is used */
    if ((operation->theirPublicKey->keyLength != (2 * ECCParams_NISTP256_LENGTH + OCTET_STRING_OFFSET)) ||
        (operation->theirPublicKey->keyMaterial[0] != 0x04))
    {
        return ECDSA_STATUS_INVALID_KEY_SIZE;
    }

    /*
     * Allocate local copies of the private and public keys because the ECC
     * library implementation requires the word length to be prepended to every
     * array input.
     */
    ECC_NISTP256_Param publicKeyUnionX;
    ECC_NISTP256_Param publicKeyUnionY;
    ECC_NISTP256_Param hashUnion;
    ECC_NISTP256_Param rUnion;
    ECC_NISTP256_Param sUnion;

    /* Prepend the word length - always 8 words for P256 */
    publicKeyUnionX.word[0] = 0x08;
    publicKeyUnionY.word[0] = 0x08;
    hashUnion.word[0] = 0x08;
    rUnion.word[0] = 0x08;
    sUnion.word[0] = 0x08;

    /*
     * Since we are receiving the private and public keys in octet string
     * format, we need to convert them to little-endian for use with the
     * ECC library functions.
     */
    CryptoUtils_reverseCopyPad(operation->hash,
                               &hashUnion.word[1],
                               ECCParams_NISTP256_LENGTH);

    CryptoUtils_reverseCopyPad(operation->r,
                               &rUnion.word[1],
                               ECCParams_NISTP256_LENGTH);

    CryptoUtils_reverseCopyPad(operation->s,
                               &sUnion.word[1],
                               ECCParams_NISTP256_LENGTH);

    CryptoUtils_reverseCopyPad(operation->theirPublicKey->keyMaterial
                               + OCTET_STRING_OFFSET,
                               &publicKeyUnionX.word[1],
                               ECCParams_NISTP256_LENGTH);

    CryptoUtils_reverseCopyPad(operation->theirPublicKey->keyMaterial
                               + ECCParams_NISTP256_LENGTH
                               + OCTET_STRING_OFFSET,
                               &publicKeyUnionY.word[1],
                               ECCParams_NISTP256_LENGTH);

    eccStatus = ECCSW_validatePublicKeyWeierstrass(&eccState,
                                                   publicKeyUnionX.word,
                                                   publicKeyUnionY.word);

    if (eccStatus == STATUS_ECC_POINT_ON_CURVE)
    {
        /* Call ECC library to perform verification */
        eccStatus = ECCSW_ECDSAVerify(&eccState,
                                      publicKeyUnionX.word,
                                      publicKeyUnionY.word,
                                      hashUnion.word,
                                      rUnion.word,
                                      sUnion.word);

        /* Check the ECC library return code and set the driver status accordingly */
        if (eccStatus == STATUS_ECDSA_VALID_SIGNATURE)
        {
            returnStatus = ECDSA_STATUS_SUCCESS;
        }
    }

    return returnStatus;
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
