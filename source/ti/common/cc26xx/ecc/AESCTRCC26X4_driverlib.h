/******************************************************************************

 @file  AECCTRCC26X4_driverlib.h

 @brief This module implements an AESCTR driver using only driverlib APIs
        for the CC26X2 and CC26X4 device families with the following
        restrictions to minimize code size:
        - Single threaded applications only
        - Polling return behavior only
        - Signature verification only
        - P256 curve support only

 ******************************************************************************
 * Copyright (c) 2018-2024, Texas Instruments Incorporated
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
 *  @file       AESCTRCC26XX.h
 *
 *  @brief      AESCTR driver implementation for the CC26XX family
 *
 *  This file should only be included in the board file to fill the AESCTR_config
 *  struct.
 *
 *  # Hardware Accelerator #
 *  The CC26XX family has a dedicated hardware crypto accelerator. It is capable
 *  of multiple AES block cipher modes of operation including CTR. Only one operation
 *  can be carried out on the accelerator at a time. Mutual exclusion is
 *  implemented at the driver level and coordinated between all drivers relying on
 *  the accelerator. It is transparent to the application and only noted to ensure
 *  sensible access timeouts are set.
 *
 *  # Key Store #
 *  The CC26XX crypto module contains a key store. The only way to load a key into
 *  the AES accelerator is to first load it into the key store. To guarantee availability
 *  of open key locations in the key store for AES operations, the last two key
 *  locations (6 and 7) are reserved for ad-hoc operations. The key is loaded into the
 *  key store, the AES operation is carried out, and the key is deleted from the key store.
 *  Since the key store does not have retention and the keys can not survive going into
 *  standby, the key store is only used to load keys into the AES accelerator rather
 *  than store keys. Support for pre-loading keys into the key store and using them
 *  in an AES operation is not supported in this driver.
 *
 *  # Implementation Limitations
 *  - Only plaintext CryptoKeys are supported by this implementation.
 *
 *  # Runtime Parameter Validation #
 *  The driver implementation does not perform runtime checks for most input parameters.
 *  Only values that are likely to have a stochastic element to them are checked (such
 *  as whether a driver is already open). Higher input parameter validation coverage is
 *  achieved by turning on assertions when compiling the driver.
 */

#ifndef AESCTRCC26XX_driverlib_include
#define AESCTRCC26XX_driverlib_include

#include <stdint.h>
#include <stdbool.h>


#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(driverlib/aes.h)
#include "ECDSACC26X4_driverlib.h"

#ifdef __cplusplus
extern "C" {
#endif

// Field:    [29] SAVE_CONTEXT
//
// IV must be read before the AES engine can start a new operation.
#define CRYPTO_AESCTL_SAVE_CONTEXT                                  0x20000000
#define CRYPTO_AESCTL_SAVE_CONTEXT_BITN                                     29
#define CRYPTO_AESCTL_SAVE_CONTEXT_M                                0x20000000
#define CRYPTO_AESCTL_SAVE_CONTEXT_S                                        29

/*!
 *  @brief  Mask for the operation mode.
 */
#define AESCTR_OP_MODE_MASK 0x0F

/*!
 * @brief   Successful status code.
 *
 * Functions return #AES_STATUS_SUCCESS if the function was executed
 * successfully.
 */
#define AES_STATUS_SUCCESS ((int_fast16_t)0)

/*!
 * @brief   Generic error status code.
 *
 * Functions return #AES_STATUS_ERROR if the function was not executed
 * successfully and no more pertinent error code could be returned.
 */
#define AES_STATUS_ERROR ((int_fast16_t)-1)

/*!
 * @brief   The ongoing operation was canceled.
 */
#define AES_STATUS_CANCELED ((int_fast16_t)-3)

/*!
 *  @brief  Flag indicating a segmented operation.
 */
#define AESCTR_OP_FLAG_SEGMENTED 0x10 /* bit 4 */

/*!
 *  @brief  Flag indicating a finalize operation.
 */
#define AESCTR_OP_FLAG_FINALIZE 0x20 /* bit 5 */


/*!
 *  @brief  Struct containing the parameters required for encrypting/decrypting
 *          a message using a one-step operation.
 *
 *  The driver may access it at any point during the operation. It must remain
 *  in scope for the entire duration of the operation.
 */
typedef struct
{
    const CryptoKey_Plaintext *key;/*!< Pointer to a previously initialized CryptoKey. */
    const uint8_t *input;          /*!<
                                    *   - Encryption: The plaintext buffer to be
                                    *     encrypted in the CTR operation.
                                    *   - Decryption: The ciphertext to be decrypted.
                                    */
    uint8_t *output;               /*!<
                                    *   - Encryption: The output ciphertext buffer that
                                    *     the encrypted plaintext is copied to.
                                    *   - Decryption: The plaintext derived from the
                                    *     decrypted ciphertext is copied here.
                                    *   Size of the output buffer must be greater than
                                    *   or equal to the inputLength.
                                    */
    const uint8_t *initialCounter; /*!< A buffer containing an initial counter. Under
                                    *   the same key, each counter value may only be
                                    *   used to encrypt or decrypt a single input
                                    *   block. If NULL, zero will be used for the
                                    *   initial counter value. The buffer's size must
                                    *   be at least 16-bytes.
                                    */
    size_t inputLength;            /*!< Length of the input in bytes. An equal number
                                    *   of bytes will be output by the operation.
                                    *   Max length supported may be limited depending on
                                    *   the return behavior.
                                    */
} AESCTR_OneStepOperation;

/*!
 *  @brief  Struct containing the parameters required for encrypting/decrypting
 *          a message using a segmented operation.  This struct must be updated
 *          for each "add data" and "finalize" step. Modifying the structure and any buffers that
 *          it points to while an operation is in progress is prohibited.
 *
 *  The driver may access it at any point during the operation. It must remain
 *  in scope for the entire duration of the operation.
 */
typedef struct
{
    const uint8_t *input; /*!<
                           *   - Encryption: The plaintext buffer to be
                           *     encrypted in the CTR operation.
                           *   - Decryption: The ciphertext to be decrypted.
                           */
    uint8_t *output;      /*!<
                           *   - Encryption: The output ciphertext buffer that
                           *     the encrypted plaintext is copied to.
                           *   - Decryption: The plaintext derived from the
                           *     decrypted ciphertext is copied here.
                           *   Size of the output buffer must be greater than
                           *   or equal to the inputLength.
                           */
    size_t inputLength;   /*!< Length of the input in bytes. An equal number
                           *   of bytes will be output by the operation. Must
                           *   be a non-zero multiple of block size (16-bytes) when
                           *   calling #AESCTR_addData(). May be zero when calling
                           *   #AESCTR_finalize() to finalize a segmented
                           *   operation without additional data.
                           */
} AESCTR_SegmentedOperation;

/*!
 *  @brief Union containing a reference to a one-step and segmented operation
 *         structure.
 */
typedef union
{
    AESCTR_OneStepOperation oneStepOperation;     /* One-step operation element of the operation union */
    AESCTR_SegmentedOperation segmentedOperation; /* Segmented operation element of the operation union */
} AESCTR_OperationUnion;

/*!
 *  @brief  Enum for the direction of the CTR operation.
 */
typedef enum
{
    AESCTR_MODE_ENCRYPT = 1,
    AESCTR_MODE_DECRYPT = 2,
} AESCTR_Mode;

/*!
 *  @brief  Enum for the operation types supported by the driver.
 */
typedef enum
{
    AESCTR_OPERATION_TYPE_ENCRYPT           = AESCTR_MODE_ENCRYPT,
    AESCTR_OPERATION_TYPE_DECRYPT           = AESCTR_MODE_DECRYPT,
    AESCTR_OPERATION_TYPE_ENCRYPT_SEGMENTED = (AESCTR_MODE_ENCRYPT | AESCTR_OP_FLAG_SEGMENTED),
    AESCTR_OPERATION_TYPE_DECRYPT_SEGMENTED = (AESCTR_MODE_DECRYPT | AESCTR_OP_FLAG_SEGMENTED),
    AESCTR_OPERATION_TYPE_ENCRYPT_FINALIZE  = (AESCTR_MODE_ENCRYPT | AESCTR_OP_FLAG_FINALIZE),
    AESCTR_OPERATION_TYPE_DECRYPT_FINALIZE  = (AESCTR_MODE_DECRYPT | AESCTR_OP_FLAG_FINALIZE),
} AESCTR_OperationType;


/*!
 *  @brief      AESCTRCC26XX Object
 *
 *  The application must not access any member variables of this structure!
 */
typedef struct
{
    uint32_t counter[AES_BLOCK_SIZE / 4];
    AESCTR_OperationUnion *operation;
    const uint8_t *input;
    uint8_t *output;
    size_t inputLength;
    CryptoKey_Plaintext key;
    volatile int_fast16_t returnStatus;
    AESCTR_OperationType operationType;
    bool isOpen;
    volatile bool hwBusy;
    volatile bool operationInProgress;
} AESCTRCC26XX_Object;


/*
 *  ======== AESCTR_oneStepEncrypt ========
 */
int_fast16_t AESCTR_oneStepEncrypt (AESCTR_OneStepOperation *operationStruct);

/*
 *  ======== AESCTR_oneStepDecrypt ========
 */
int_fast16_t AESCTR_oneStepDecrypt (AESCTR_OneStepOperation *operationStruct );

/**
 * @fn         AES_open
 * @brief      Initializes the AES peripheral.
 *
 *             This function must be called before all other AES functions
  *
 * @return     AES_STATUS_SUCCESS on success
 */
int_fast16_t AES_open(void);

/**
 * @fn         AES_close
 * @brief      Initializes the AES peripheral.
 *
 *             This function must be called before all other AES functions
  *
 * @return     AES_STATUS_SUCCESS on success
 */
int_fast16_t AES_close(void);

#ifdef __cplusplus
}
#endif

#endif /* ti_drivers_aesctr_AESCTRCC26XX__include */
