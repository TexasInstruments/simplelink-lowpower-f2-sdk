/*
 * Copyright (c) 2015-2020, Texas Instruments Incorporated
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
#include <stdint.h>
#include "ti-crypto/sl_crypto.h"
#include "mcuboot_config/mcuboot_logging.h"
#include "mcuboot_config.h"
#include "string.h"

#if !defined(DeviceFamily_CC23X0R5) && !defined(DeviceFamily_CC23X0R53) && !defined(DeviceFamily_CC23X0R2) && !defined(DeviceFamily_CC23X0R22)
#include "ti/common/cc26xx/sha2/sha2_driverlib.h"
#include "ti/common/cc26xx/ecc/ECDSACC26X4_driverlib.h"
#include "ti/common/cc26xx/ecc/AESCTRCC26X4_driverlib.h"
#else
#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(driverlib/hapi.h)
#include "ti/common/ecdsa_lpf3/ecdsa_lpf3.h"
#endif

#if defined(DeviceFamily_CC23X0R5) || defined(DeviceFamily_CC23X0R53) || defined(DeviceFamily_CC23X0R2) || defined(DeviceFamily_CC23X0R22)
static SHA256SW_Object sha256SWObject;
static SHA256SW_Handle sha256SWHandle = &sha256SWObject;
#endif

#if defined(DeviceFamily_CC13X2) || defined(DeviceFamily_CC26X2)
#define ECDSA_PUB_KEY_SIZE 64

/*********************************************************************
 * GLOBAL FUNCTION REFERENCES
 ********************************************************************/
extern uint8_t ECDSA_verif(uint32_t *, uint32_t *, uint32_t *, uint32_t *,
                                 uint32_t *);


uint32_t eccWorkzone[SECURE_FW_ECC_NIST_P256_WORKZONE_LEN_IN_BYTES + SECURE_FW_ECC_BUF_TOTAL_LEN(SECURE_FW_ECC_NIST_P256_KEY_LEN_IN_BYTES)*5] = {0};

#endif

#if defined(DeviceFamily_CC13X2) || defined(DeviceFamily_CC26X2)

/*********************************************************************
 * @fn         reverseOrder
 * @brief      Reverse the byte order and copy to output buffer
 *
 * @param      pBufIn - pointer to input buffer
 * @param      pBufOut - pointer to output buffer
 */
static void reverseOrder(const uint8_t *pBufIn,uint8_t *pBufOut)
{
  uint8_t i=0;
  for(i=0;i<SECURE_FW_SIGN_LEN;i++)
  {
    pBufOut[i] = pBufIn[SECURE_FW_SIGN_LEN-1-i];
  }
}

/*********************************************************************
 * @fn         copyBytes
 * @brief      Copy data between memory locatins
 *
 * @param      pDst - pointer to destination buffer
 * @param      pSrc - pointer to source buffer
 * @param      len  - length of data to be copied
 */
static void copyBytes(uint8_t *pDst, const uint8_t *pSrc, uint32_t len)
{
  uint32_t i;
  for(i=0; i<len; i++)
  {
      pDst[i]=pSrc[i];
  }
}

#endif

/*
 *  ======== SHA2 & HMAC ========
 */
void SlCrypto_sha256_init(void)
{
#if !defined(DeviceFamily_CC23X0R5) && !defined(DeviceFamily_CC23X0R53) && !defined(DeviceFamily_CC23X0R2) && !defined(DeviceFamily_CC23X0R22)
    SHA2_open();
#else
    HapiSha256SwStart(sha256SWHandle);
#endif
}


void SlCrypto_sha256_drop(void)
{
#if !defined(DeviceFamily_CC23X0R5) && !defined(DeviceFamily_CC23X0R53) && !defined(DeviceFamily_CC23X0R2) && !defined(DeviceFamily_CC23X0R22)
    SHA2_close();
#endif
}

int SlCrypto_sha256_update(const void *data,
                           uint32_t data_len)
{
#if !defined(DeviceFamily_CC23X0R5) && !defined(DeviceFamily_CC23X0R53) && !defined(DeviceFamily_CC23X0R2) && !defined(DeviceFamily_CC23X0R22)
    SHA2_open();
    return SHA2_addData(data, data_len);
#else
    return HapiSha256SwAddData(sha256SWHandle, data, data_len);
#endif
}

int SlCrypto_sha256_final(uint8_t *output)
{
    int rtn;
#if !defined(DeviceFamily_CC23X0R5) && !defined(DeviceFamily_CC23X0R53) && !defined(DeviceFamily_CC23X0R2) && !defined(DeviceFamily_CC23X0R22)
    SlCrypto_sha256_init();
    rtn = SHA2_finalize(output);
    SHA2_close();
#else
    rtn = HapiSha256SwFinalize(sha256SWHandle, (uint32_t*)output);
#endif

    return rtn;
}

#if !defined(DeviceFamily_CC23X0R5) && !defined(DeviceFamily_CC23X0R53) && !defined(DeviceFamily_CC23X0R2) && !defined(DeviceFamily_CC23X0R22)
int SlCrypto_sha256_setupHmac(const uint8_t *key, unsigned int key_size) {
    return (SHA2_setupHmac(key, key_size));
}

int SlCrypto_sha256_finalizeHmac(uint8_t *tag) {
    return (SHA2_finalizeHmac(tag));
}
#endif

/*
 *  ======== ECDSA & ECDH ========
 */

void SlCrypto_ecdsa_p256_init(void)
{
#if defined(DeviceFamily_CC13X2) || defined(DeviceFamily_CC26X2)

    /* Store client parameters into ECC ROM parameters */
    eccRom_param_p  = &NIST_Curve_P256_p;
    eccRom_param_r  = &NIST_Curve_P256_r;
    eccRom_param_a  = &NIST_Curve_P256_a;
    eccRom_param_b  = &NIST_Curve_P256_b;
    eccRom_param_Gx = &NIST_Curve_P256_Gx;
    eccRom_param_Gy = &NIST_Curve_P256_Gy;

    /* Initialize window size */
    eccRom_windowSize = SECURE_FW_ECC_WINDOW_SIZE;

#else
#if !defined(DeviceFamily_CC23X0R5) && !defined(DeviceFamily_CC23X0R53) && !defined(DeviceFamily_CC23X0R2) && !defined(DeviceFamily_CC23X0R22)
        ECDSA_open();
#endif
#endif
}

void SlCrypto_ecdsa_p256_drop(void)
{
#if defined(DeviceFamily_CC13X2) || defined(DeviceFamily_CC26X2)
    return;
#else
#if !defined(DeviceFamily_CC23X0R5) && !defined(DeviceFamily_CC23X0R53) && !defined(DeviceFamily_CC23X0R2) && !defined(DeviceFamily_CC23X0R22)
    ECDSA_close();
#endif
#endif
}

int SlCrypto_ecdsa_p256_verify(const uint8_t *pk, const uint8_t *hash, const uint8_t *sig)
{
#if defined(DeviceFamily_CC13X2) || defined(DeviceFamily_CC26X2)
    uint8_t *publicKeyXBuf;
    uint8_t *publicKeyYBuf;
    uint8_t *hashBuf;
    uint8_t *sign1Buf;
    uint8_t *sign2Buf;

    /* clear the ECC work zone Buffer */
    uint32_t *eccPayloadWorkzone = eccWorkzone;
    memset(eccPayloadWorkzone, 0, sizeof(eccWorkzone));

    // Verify the hash
    // Create temp buffer used for ECDSA sign verify, it should 6*ECDSA_KEY_LEN
    uint8_t tempWorkzone[ECDSA_SHA_TEMPWORKZONE_LEN];
    memset(tempWorkzone, 0, ECDSA_SHA_TEMPWORKZONE_LEN);

    // Variables to be allocated on the tempworkzone,
    /* Split allocated memory into buffers */
    uint8_t *reversedHash = tempWorkzone;
    uint8_t *reversedPubKeyX = reversedHash + ECDSA_KEY_LEN;
    uint8_t *reversedPubKeyY = reversedPubKeyX + ECDSA_KEY_LEN;
    uint8_t *reversedSign1 = reversedPubKeyY + ECDSA_KEY_LEN;
    uint8_t *reversedSign2 = reversedSign1 + ECDSA_KEY_LEN;

    reverseOrder(hash, reversedHash);
    reverseOrder(pk, reversedPubKeyX);
    reverseOrder(pk+32, reversedPubKeyY);
    reverseOrder(sig, reversedSign1);
    reverseOrder(sig+32, reversedSign2);

    /*total memory for operation: workzone and 5 key buffers*/
    eccRom_workzone = &eccWorkzone[0];

    /* Split allocated memory into buffers */
    publicKeyXBuf = (uint8_t *)eccRom_workzone +
                 SECURE_FW_ECC_NIST_P256_WORKZONE_LEN_IN_BYTES;
    publicKeyYBuf = publicKeyXBuf +
                 SECURE_FW_ECC_BUF_TOTAL_LEN(SECURE_FW_ECC_NIST_P256_KEY_LEN_IN_BYTES);
    hashBuf =  publicKeyYBuf +
               SECURE_FW_ECC_BUF_TOTAL_LEN(SECURE_FW_ECC_NIST_P256_KEY_LEN_IN_BYTES);
    sign1Buf  = hashBuf +
             SECURE_FW_ECC_BUF_TOTAL_LEN(SECURE_FW_ECC_NIST_P256_KEY_LEN_IN_BYTES);
    sign2Buf  = sign1Buf +
             SECURE_FW_ECC_BUF_TOTAL_LEN(SECURE_FW_ECC_NIST_P256_KEY_LEN_IN_BYTES);

    /* Set length of keys in words in the first word of each buffer*/
    *((uint32_t *)&publicKeyXBuf[SECURE_FW_ECC_KEY_LEN_OFFSET]) =
      (uint32_t)(SECURE_FW_ECC_UINT32_BLK_LEN(SECURE_FW_ECC_NIST_P256_KEY_LEN_IN_BYTES));

    *((uint32_t *)&publicKeyYBuf[SECURE_FW_ECC_KEY_LEN_OFFSET]) =
     (uint32_t)(SECURE_FW_ECC_UINT32_BLK_LEN(SECURE_FW_ECC_NIST_P256_KEY_LEN_IN_BYTES));

    *((uint32_t *)&hashBuf[SECURE_FW_ECC_KEY_LEN_OFFSET]) =
      (uint32_t)(SECURE_FW_ECC_UINT32_BLK_LEN(SECURE_FW_ECC_NIST_P256_KEY_LEN_IN_BYTES));

    *((uint32_t *)&sign1Buf[SECURE_FW_ECC_KEY_LEN_OFFSET]) =
      (uint32_t)(SECURE_FW_ECC_UINT32_BLK_LEN(SECURE_FW_ECC_NIST_P256_KEY_LEN_IN_BYTES));

    *((uint32_t *)&sign2Buf[SECURE_FW_ECC_KEY_LEN_OFFSET]) =
      (uint32_t)(SECURE_FW_ECC_UINT32_BLK_LEN(SECURE_FW_ECC_NIST_P256_KEY_LEN_IN_BYTES));

    /* Copy input key into buffer */
    copyBytes( publicKeyXBuf + SECURE_FW_ECC_KEY_OFFSET,
               reversedPubKeyX,
               SECURE_FW_ECC_NIST_P256_KEY_LEN_IN_BYTES);
    copyBytes( publicKeyYBuf + SECURE_FW_ECC_KEY_OFFSET,
               reversedPubKeyY,
               SECURE_FW_ECC_NIST_P256_KEY_LEN_IN_BYTES);
     /* copy hash into buffer */
    copyBytes( hashBuf + SECURE_FW_ECC_KEY_OFFSET,
               reversedHash,
               SECURE_FW_ECC_NIST_P256_KEY_LEN_IN_BYTES);

    copyBytes( sign1Buf + SECURE_FW_ECC_KEY_OFFSET,
               reversedSign1,
               SECURE_FW_ECC_NIST_P256_KEY_LEN_IN_BYTES);
    copyBytes( sign2Buf + SECURE_FW_ECC_KEY_OFFSET,
               reversedSign2,
               SECURE_FW_ECC_NIST_P256_KEY_LEN_IN_BYTES);


    uint8_t status = ECDSA_verif((uint32_t *)publicKeyXBuf,
                                 (uint32_t *)publicKeyYBuf,
                                 (uint32_t *)hashBuf,
                                 (uint32_t *)sign1Buf,
                                 (uint32_t *)sign2Buf);

    if (status != SECURE_FW_ECC_STATUS_VALID_SIGNATURE) {
        MCUBOOT_LOG_ERR("SlCrypto_ecdsa_p256_verify: fail");
        return -1;
    }

    return 0;
#else
    CryptoKey_Plaintext publicKey;
    ECDSA_OperationVerify operationVerify;
    int_fast16_t operationResult;

    /* Initialize the publicKey */
    uint8_t publicKeyingMaterial[2 * SECURE_FW_ECC_NIST_P256_KEY_LEN_IN_BYTES + 1] =  {0};
    publicKeyingMaterial[0] = 0x04;
    memcpy( &publicKeyingMaterial[1], pk, SECURE_FW_ECC_NIST_P256_KEY_LEN_IN_BYTES);
    memcpy( &publicKeyingMaterial[1 + SECURE_FW_ECC_NIST_P256_KEY_LEN_IN_BYTES],
            pk + SECURE_FW_ECC_NIST_P256_KEY_LEN_IN_BYTES,
            SECURE_FW_ECC_NIST_P256_KEY_LEN_IN_BYTES);

    CryptoKeyPlaintext_initKey(&publicKey,
                               publicKeyingMaterial,
                               sizeof(publicKeyingMaterial));

    /* Initialize the operation */
#if !defined(DeviceFamily_CC23X0R5) && !defined(DeviceFamily_CC23X0R53) && !defined(DeviceFamily_CC23X0R2) && !defined(DeviceFamily_CC23X0R22)
    operationVerify.curve           = &ECCParams_NISTP256;
#endif
    operationVerify.theirPublicKey  = &publicKey;
    operationVerify.hash            = hash;
    operationVerify.r               = sig;
    operationVerify.s               = sig + SECURE_FW_ECC_NIST_P256_KEY_LEN_IN_BYTES;

    operationResult = ECDSA_verify(&operationVerify);

    if (operationResult != ECDSA_STATUS_SUCCESS) {
        MCUBOOT_LOG_ERR("SlCrypto_ecdsa_p256_verify: fail");
        return -1;
    }
    return 0;
#endif
}

#if !defined(DeviceFamily_CC23X0R5) && !defined(DeviceFamily_CC23X0R53) && !defined(DeviceFamily_CC23X0R2) && !defined(DeviceFamily_CC23X0R22)
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
        return -1;
    }

    /* Reverse and concatenate x and y */
    uint32_t i = 0;
    for (i = 0; i < paramLength; i++)
    {
        buffer[i + 1]               = curveParams->generatorX[paramLength - i - 1];
        buffer[i + 1 + paramLength] = curveParams->generatorY[paramLength - i - 1];
    }

    buffer[0] = 0x04;
    /* Fill the remaining buffer with 0 if needed */
    memset(buffer + pointLength, 0, length - pointLength);

    return 0;
}


int SlCrypto_ecdh_p256_computeSharedSecret(const uint8_t *pk, const uint8_t *sk, uint8_t *z) {

    ECDH_OperationComputeSharedSecret operation;
    CryptoKey_Plaintext privateKey;
    CryptoKey_Plaintext publicKey;
    CryptoKey_Plaintext SharedKey;

    uint8_t privateKeyingMaterial[SECURE_FW_ECC_NIST_P256_KEY_LEN_IN_BYTES] =  {0};

    // set the size to be SECURE_FW_ECC_NIST_P256_KEY_LEN_IN_BYTES for little endian
    uint8_t publicKeyingMaterial[2* SECURE_FW_ECC_NIST_P256_KEY_LEN_IN_BYTES+ 1] =  {0};

    memcpy(privateKeyingMaterial, sk, SECURE_FW_ECC_NIST_P256_KEY_LEN_IN_BYTES);
    CryptoKeyPlaintext_initKey(&privateKey,
                               privateKeyingMaterial,
                               sizeof(privateKeyingMaterial));

    // set the size to be SECURE_FW_ECC_NIST_P256_KEY_LEN_IN_BYTES for little endian
    memcpy(publicKeyingMaterial, pk, 2*SECURE_FW_ECC_NIST_P256_KEY_LEN_IN_BYTES+1);
    CryptoKeyPlaintext_initKey(&publicKey,
                               publicKeyingMaterial,
                               sizeof(publicKeyingMaterial));

    // set the size to be SECURE_FW_ECC_NIST_P256_KEY_LEN_IN_BYTES for little endian
    CryptoKeyPlaintext_initKey(&SharedKey, z,
                               2*SECURE_FW_ECC_NIST_P256_KEY_LEN_IN_BYTES+1);

    /* Test code */
    //ECCParams_getUncompressedGeneratorPoint(&ECCParams_NISTP256,publicKeyingMaterial,sizeof(publicKeyingMaterial)); // only for test.

    /* Initialize the operation */
#if !defined(DeviceFamily_CC23X0R5) && !defined(DeviceFamily_CC23X0R53) && !defined(DeviceFamily_CC23X0R2) && !defined(DeviceFamily_CC23X0R22)
    operation.curve           = &ECCParams_NISTP256;
#endif
    operation.myPrivateKey = &privateKey;
    operation.theirPublicKey = &publicKey;
    operation.sharedSecret = &SharedKey;
//#ifdef ECDH_BIG_ENDIAN_KEY
//    operation.keyMaterialEndianness = ECDH_BIG_ENDIAN_KEY; //ECDH_LITTLE_ENDIAN_KEY;
//#elif
//    operation.keyMaterialEndianness = ECDH_LITTLE_ENDIAN_KEY;
//#endif


    return ECDH_computeSharedSecret(&operation);
}

/*
 *  ======== AESCTR ========
 */

extern AESCTR_OneStepOperation operation_g;
extern CryptoKey_Plaintext aesKey_g;

void SlCrypto_aesctr_init(void) {
    AES_open();
}

void SlCrypto_aesctr_drop(void) {
    AES_close();
}


int SlCrypto_aesctr_setKey(const uint8_t *keyingMaterial) {

    SlCrypto_aesctr_init(); //make sure the prcm peripherals are enabled

    /* init operation */
    memset(&operation_g, 0x00, sizeof(AESCTR_OneStepOperation));

    //uint8_t aesKeyMaterial[AES_CTR_KEY_SIZE] =  {0};
    //memcpy(aesKeyMaterial, keyingMaterial, AES_CTR_KEY_SIZE);

    CryptoKeyPlaintext_initKey(&aesKey_g,
                               (uint8_t *) keyingMaterial,
                               AES_CTR_KEY_SIZE);

    /* Get the key */
    operation_g.key = &aesKey_g;

    return 0;
}

int SlCrypto_aesctr_encrypt(uint8_t *counter, const uint8_t *m, uint32_t mlen, size_t blk_off, uint8_t *c)
{
    SlCrypto_aesctr_init(); //make sure the prcm peripherals are enabled
    operation_g.input             = m;
    operation_g.inputLength       = mlen;
    operation_g.initialCounter    = counter;
    operation_g.output            = c;

    return AESCTR_oneStepEncrypt (&operation_g);
    //return 0;
    //return AESCTR_startOneStepOperation(&operation_g, AESCTR_OPERATION_TYPE_ENCRYPT);

}

int SlCrypto_aesctr_decrypt(uint8_t *counter, const uint8_t *c, uint32_t clen, size_t blk_off, uint8_t *m)
{
    SlCrypto_aesctr_init(); //make sure the prcm peripherals are enabled
    operation_g.input             = c;
    operation_g.inputLength       = clen;
    operation_g.initialCounter    = counter;
    operation_g.output            = m;

    return AESCTR_oneStepDecrypt (&operation_g);
    //return AESCTR_startOneStepOperation(&operation_g, AESCTR_OPERATION_TYPE_DECRYPT);
}
#endif