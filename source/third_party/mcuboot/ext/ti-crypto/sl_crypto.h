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

#ifndef __TI_CRYPTO_H__
#define __TI_CRYPTO_H__

#include <stddef.h>

  /*********************************************************************
 * GLOBAL VARIABLES
 */
/* ECC ROM global window size and workzone buffer. */
extern uint8_t eccRom_windowSize;
extern uint32_t *eccRom_workzone;

/* ECC ROM global parameters */
extern uint32_t  *eccRom_param_p;
extern uint32_t  *eccRom_param_r;
extern uint32_t  *eccRom_param_a;
extern uint32_t  *eccRom_param_b;
extern uint32_t  *eccRom_param_Gx;
extern uint32_t  *eccRom_param_Gy;

/* NIST P-256 Curves in ROM
 Note: these are actually strings*/
extern uint32_t NIST_Curve_P256_p;
extern uint32_t NIST_Curve_P256_r;
extern uint32_t NIST_Curve_P256_a;
extern uint32_t NIST_Curve_P256_b;
extern uint32_t NIST_Curve_P256_Gx;
extern uint32_t NIST_Curve_P256_Gy;


/*********************************************************************
 * MACROS
 */

#define AES_CTR_KEY_SIZE        (16)

/* ECC Window Size.  Determines speed and workzone size of ECC operations.
 Recommended setting is 3. */
#define SECURE_FW_ECC_WINDOW_SIZE                3

/*! Invalid ECC Signature         */
#define SECURE_FW_ECC_STATUS_INVALID_SIGNATURE             0x5A
/*! ECC Signature Successfully Verified  */
#define SECURE_FW_ECC_STATUS_VALID_SIGNATURE               0xA5

#define ECDSA_KEY_LEN                32    //!< Length of the ECDSA security key
#define ECDSA_SHA_TEMPWORKZONE_LEN   ECDSA_KEY_LEN*6  //!< space for six buffers finalHash, reverseHash, pubkeyX, pubKeyY, sign1, sign2

/* Offset value for number of bytes occupied by length field */
#define SECURE_FW_ECC_KEY_OFFSET                 4

/* Offset of Key Length field */
#define SECURE_FW_ECC_KEY_LEN_OFFSET             0

/*!
 * ECC key length in bytes for NIST P-256 keys.
 */
#define SECURE_FW_ECC_NIST_P256_KEY_LEN_IN_BYTES                32

/*!
 * ECC Workzone length in bytes for NIST P-256 key and shared secret generation.
 * For use with ECC Window Size 3 only.  Used to store intermediary values in
 * ECC calculations.
 */
#define BIM_ECC_NIST_P256_WORKZONE_LEN_IN_BYTES          1100

#ifndef SECURE_FW_ECC_NIST_P256_WORKZONE_LEN_IN_BYTES
    #define SECURE_FW_ECC_NIST_P256_WORKZONE_LEN_IN_BYTES (BIM_ECC_NIST_P256_WORKZONE_LEN_IN_BYTES)
#endif

/*! Length of Sign */
#define SECURE_FW_SIGN_LEN 32

/* Total buffer size */
#define SECURE_FW_ECC_BUF_TOTAL_LEN(len)         ((len) + SECURE_FW_ECC_KEY_OFFSET)

/* Key size in uint32_t blocks */
#define SECURE_FW_ECC_UINT32_BLK_LEN(len)        (((len) + 3) / 4)

#define NUM_ECC_BYTES (256 / 8)


/*
 *  ======== SHA2 & HMAC ========
 */
void SlCrypto_sha256_init(void);

void SlCrypto_sha256_drop(void);

int SlCrypto_sha256_update(const void *data,
                           uint32_t data_len);

int SlCrypto_sha256_final(uint8_t *output);

#if !defined(DeviceFamily_CC23X0R5) && !defined(DeviceFamily_CC23X0R53) && !defined(DeviceFamily_CC23X0R2) && !defined(DeviceFamily_CC23X0R22)
int SlCrypto_sha256_setupHmac(const uint8_t *key, unsigned int key_size);

int SlCrypto_sha256_finalizeHmac(uint8_t *tag);
#endif

/*
 *  ======== ECDSA & ECDH ========
 */
void SlCrypto_ecdsa_p256_init(void);

void SlCrypto_ecdsa_p256_drop(void);

int SlCrypto_ecdsa_p256_verify(const uint8_t *pk,
                               const uint8_t *hash,
                               const uint8_t *sig);

#if !defined(DeviceFamily_CC23X0R5) && !defined(DeviceFamily_CC23X0R53) && !defined(DeviceFamily_CC23X0R2) && !defined(DeviceFamily_CC23X0R22)
int SlCrypto_ecdh_p256_computeSharedSecret(const uint8_t *pk, const uint8_t *sk, uint8_t *z);

/*
 *  ======== AESCTR ========
 */
void SlCrypto_aesctr_init(void);

void SlCrypto_aesctr_drop(void);

int SlCrypto_aesctr_setKey(const uint8_t *keyingMaterial);

int SlCrypto_aesctr_encrypt(uint8_t *counter, const uint8_t *m, uint32_t mlen, size_t blk_off, uint8_t *c);

int SlCrypto_aesctr_decrypt(uint8_t *counter, const uint8_t *c, uint32_t clen, size_t blk_off, uint8_t *m);
#endif


#endif /* __TI_CRYPTO_H__ */
