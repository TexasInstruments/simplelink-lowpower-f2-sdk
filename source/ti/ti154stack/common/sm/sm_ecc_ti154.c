/******************************************************************************

 @file sm_ecc.c

 @brief TI 15.4 Security Manager
 This file contains the SM Elliptic-Curve Cryptography ECDH logic
 for P-256 and Diffie-Hellman keys.

 Group: LPRF
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2018-2025, Texas Instruments Incorporated
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

/******************************************************************************
 Includes
 *****************************************************************************/
#include "ti_154stack_features.h"

#ifdef FEATURE_SECURE_COMMISSIONING
#include <string.h>
#include <stdint.h>

#include "mac_util.h"
#include "sm_ti154.h"
#include "ti_154stack_config.h"
#include "sm_ecc_ti154.h"
#include "api_mac.h"

#include "ti_drivers_config.h"

#include <ti/drivers/TRNG.h>
#include <ti/drivers/cryptoutils/cryptokey/CryptoKeyPlaintext.h>
#include <ti/drivers/ECDH.h>
#include "osal_port.h"

/******************************************************************************
 Constants and definitions
 *****************************************************************************/

/******************************************************************************
 Structures
 *****************************************************************************/


/******************************************************************************
 Global variables
 *****************************************************************************/

/******************************************************************************
 Local variables
 *****************************************************************************/
static ECDH_Handle ECDH_handle;

#ifdef SM_ECC_TEST_VECTOR
/* Test vectors obtained from: Invia vector_curves.c */
/* NIST256_testECDH_Bsk[1] */
static uint8_t testVectorPrivateKeymaterial[SM_ECC_PRIVATE_KEY_SIZE] = {
     0x93, 0xe1, 0x9b, 0x9e, 0x9f, 0x8f, 0xc5, 0xcf,
     0x7a, 0x4c, 0x4c, 0xd6, 0x9d, 0xda, 0x8c, 0xa4,
     0x21, 0x68, 0xb3, 0xa8, 0x0a, 0x82, 0xf6, 0x74,
     0x44, 0x9a, 0xb4, 0x83, 0x32, 0x7f, 0xcc, 0x07 };

/* NIST256_testECDH_BPx[1]:NIST256_testECDH_BPy[1] */
static const uint8_t testVectorPublicKey[SM_ECC_PUBLIC_KEY_SIZE] = {
     /* Need formatting byte of 0x04 in the front */
     0x04,
     0x42, 0xae, 0x85, 0xa6, 0x4c, 0xd0, 0x51, 0x7f,
     0x4e, 0x3e, 0x63, 0xb6, 0xcf, 0xe6, 0x97, 0x39,
     0x76, 0x66, 0xa7, 0xd9, 0x72, 0x74, 0x27, 0x7f,
     0x7b, 0x58, 0x26, 0xf4, 0x23, 0x92, 0x34, 0xd3,
     0xd7, 0x01, 0x90, 0xef, 0x2b, 0x35, 0x9a, 0x81,
     0xa0, 0xee, 0xc1, 0xe0, 0x4a, 0xfc, 0x19, 0x40,
     0x87, 0xf1, 0xa5, 0xca, 0x66, 0xb1, 0xf3, 0x75,
     0x25, 0x63, 0xc4, 0xec, 0xc4, 0x29, 0x87, 0xe9 };

/* NIST256_testECDH_APx[1]:NIST256_testECDH_APy[1] */
const uint8_t testVectorForeignPublicKey[SM_ECC_PUBLIC_KEY_SIZE] = {
    /* Need formatting byte of 0x04 in the front */
    0x04,
    0x84, 0xe0, 0x34, 0x9e, 0x78, 0xc9, 0x33, 0xea,
    0xda, 0xea, 0x66, 0x77, 0x05, 0x38, 0xeb, 0x48,
    0x05, 0xbd, 0x84, 0x92, 0x14, 0xb3, 0xeb, 0x48,
    0x6a, 0x01, 0x1b, 0x1c, 0x8e, 0x72, 0x70, 0x6a,
    0x14, 0x46, 0xc2, 0x7e, 0x76, 0x3d, 0x1c, 0xc6,
    0xdb, 0x80, 0xfb, 0xfa, 0x2d, 0x43, 0x98, 0xbd,
    0x66, 0xdd, 0x0e, 0x07, 0x97, 0x4e, 0x44, 0xa9,
    0x1f, 0x61, 0x38, 0x0c, 0x63, 0xcf, 0xc0, 0xfa };

/* NIST256_testECDH_CSx[1]:NIST256_testECDH_CSy[1]  */
const uint8_t testVectorSharedSecretKeyMaterial[SM_ECC_PUBLIC_KEY_SIZE] = {
     /* Need formatting byte of 0x04 in the front */
     0x04,
     0x24, 0x0f, 0x10, 0x7b, 0x17, 0x97, 0x82, 0x76,
     0x17, 0xf9, 0x97, 0xc8, 0x47, 0xac, 0xee, 0x5b,
     0x1c, 0x11, 0x40, 0x3b, 0xd4, 0x04, 0x6e, 0x7a,
     0x15, 0x98, 0x19, 0x0b, 0x90, 0x46, 0x92, 0x32,
     0xb1, 0x16, 0xae, 0xc4, 0x93, 0x48, 0x11, 0x0a,
     0x28, 0x2d, 0xc4, 0xab, 0x3a, 0x65, 0xb6, 0x7b,
     0x1c, 0xa5, 0xb0, 0x50, 0x92, 0x93, 0x02, 0xc4,
     0xc5, 0x8a, 0x8f, 0x8b, 0x60, 0x19, 0x5a, 0xf5 };
#endif
/******************************************************************************
 Local Function Prototypes
 *****************************************************************************/


/******************************************************************************
 Public Functions
 *****************************************************************************/
/*!
 Initialize this module.

 Public function defined in sm_ecc.h
 */
void SM_ECC_init(void)
{
    ECDH_Params ECDHParams;

    ECDH_init();

    ECDH_Params_init(&ECDHParams);

    /* Block until computation is complete */
    ECDHParams.returnBehavior =  ECDH_RETURN_BEHAVIOR_BLOCKING;
    ECDHParams.callbackFxn = NULL;

    ECDH_handle = ECDH_open(CONFIG_ECDH_0, &ECDHParams);
    if (!ECDH_handle)
    {
        /* abort */
        Main_assertHandler(SM_ENCRYPT_ERROR);
    }
}


/*!
 Initialize this module.

 Public function defined in sm_ecc.h
 */
bool SM_ECC_genLocalKeyPair(SM_ECC_securityDeviceDescriptor_t *secDevice)
{
    bool result = true;
    int_fast16_t operationResult;
    ECDH_OperationGeneratePublicKey opGenLocalPublicKey;

    /* Verify driver has been opened */
    if (!ECDH_handle)
    {
        /* abort */
        return (false);
    }

    /* Generate random key material */
#ifndef SM_ECC_TEST_VECTOR
    SM_ECC_genRandomPrivateKeyMaterial(secDevice->localPrivateKeyMaterial, SM_ECC_PRIVATE_KEY_SIZE);
#else
    /* Overwrite input key material with test vector */
    memcpy(secDevice->localPrivateKeyMaterial, testVectorPrivateKeymaterial, SM_ECC_PRIVATE_KEY_SIZE);
#endif

    /* Initialize local private/public key */
    CryptoKeyPlaintext_initKey(&secDevice->localPrivateKey, secDevice->localPrivateKeyMaterial, sizeof(secDevice->localPrivateKeyMaterial));
    CryptoKeyPlaintext_initBlankKey(&secDevice->localPublicKey, secDevice->localPublicKeyMaterial, sizeof(secDevice->localPublicKeyMaterial));

    /* Generate local key-pair */
    ECDH_OperationGeneratePublicKey_init(&opGenLocalPublicKey);
    opGenLocalPublicKey.curve                  = &ECCParams_NISTP256;
    opGenLocalPublicKey.myPrivateKey           = &secDevice->localPrivateKey;
    opGenLocalPublicKey.myPublicKey            = &secDevice->localPublicKey;
    opGenLocalPublicKey.keyMaterialEndianness  = ECDH_BIG_ENDIAN_KEY;

    /* Generate local public key based on the local private key */
    operationResult = ECDH_generatePublicKey(ECDH_handle, &opGenLocalPublicKey);
    if (operationResult != ECDH_STATUS_SUCCESS) {
        result = false;
    }

#ifdef SM_ECC_TEST_VECTOR
    /* Verify generated public key against test vector */
    if (memcmp(testVectorPublicKey, secDevice->localPublicKey.u.plaintext.keyMaterial,
               secDevice->localPublicKey.u.plaintext.keyLength) != 0) {
        result = false;
    }
#endif

    return (result);
}


/*!
 Utilize foreign public keying material to generate shared secret key.

 Public function defined in sm_ecc.h
 */
bool SM_ECC_genSharedSecretKey(SM_ECC_securityDeviceDescriptor_t *secDevice)
{
    bool result = true;
    int_fast16_t operationResult;
    ECDH_OperationComputeSharedSecret opGenSharedSecretKey;

    /*! Foreign Public key from third-party */
    CryptoKey foreignPublicKey;

    /* Verify driver has been opened */
    if (!ECDH_handle)
    {
        /* abort */
        return (false);
    }

#ifdef SM_ECC_TEST_VECTOR
    /* Overwrite input key material with test vector */
    memcpy(secDevice->foreignPublicKeyMaterial, testVectorForeignPublicKey, SM_ECC_PUBLIC_KEY_SIZE);
#endif

    /* Initialize the foreign public key and initialize a blank key for the shared secret */
    CryptoKeyPlaintext_initKey(&foreignPublicKey, secDevice->foreignPublicKeyMaterial, sizeof(secDevice->foreignPublicKeyMaterial));
    CryptoKeyPlaintext_initBlankKey(&secDevice->sharedSecretKey, secDevice->sharedSecretKeyMaterial, sizeof(secDevice->sharedSecretKeyMaterial));

    /* Generate shared secret from local private and foreign public keys */
    ECDH_OperationComputeSharedSecret_init(&opGenSharedSecretKey);
    opGenSharedSecretKey.curve                  = &ECCParams_NISTP256;
    opGenSharedSecretKey.myPrivateKey           = &secDevice->localPrivateKey;
    opGenSharedSecretKey.theirPublicKey         = &foreignPublicKey;
    opGenSharedSecretKey.sharedSecret           = &secDevice->sharedSecretKey;
    opGenSharedSecretKey.keyMaterialEndianness  = ECDH_BIG_ENDIAN_KEY;

    /* Compute the shared secret and copy it to sharedSecretKeyingMaterial */
    operationResult = ECDH_computeSharedSecret(ECDH_handle, &opGenSharedSecretKey);
    if (operationResult != ECDH_STATUS_SUCCESS) {
        result = false;
    }

#ifdef SM_ECC_TEST_VECTOR
    /* Verify generated public key against test vector */
    if (memcmp(testVectorSharedSecretKeyMaterial, secDevice->sharedSecretKey.u.plaintext.keyMaterial,
               secDevice->sharedSecretKey.u.plaintext.keyLength) != 0) {
        result = false;
    }
#endif

    return (result);
}

/*!
 Function to generate random number like private key material etc.

 Public function defined in sm_ecc.h
 */
void SM_ECC_genRandomPrivateKeyMaterial(uint8_t *buf, uint8_t keyLen)
{
    extern TRNG_Handle TRNG_handle;

    int_fast16_t result;

    CryptoKey entropyKey;
    uint8_t *entropyBuffer;
    entropyBuffer = OsalPort_malloc(keyLen);

    if(entropyBuffer)
    {
      CryptoKeyPlaintext_initBlankKey(&entropyKey, entropyBuffer, keyLen);

      result = TRNG_generateEntropy(TRNG_handle, &entropyKey);

      if (result != TRNG_STATUS_SUCCESS) {
          // Handle error
      }

      OsalPort_memcpy(buf, &entropyKey.u.plaintext.keyMaterial[0], keyLen);
      OsalPort_free(entropyBuffer);
    }
}




/******************************************************************************
 Local Functions
 *****************************************************************************/
#endif /*FEATURE_SECURE_COMMISSIONING*/
