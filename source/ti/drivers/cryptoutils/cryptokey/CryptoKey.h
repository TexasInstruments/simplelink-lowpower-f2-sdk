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
 *  @file      CryptoKey.h
 *
 *  @brief     The CryptoKey type is an opaque representation of a cryptographic key.
 *
 *  @warning   This is a beta API. It may change in future releases.
 *
 *  Cryptographic keying material may be stored on an embedded system multiple ways.
 *   - plaintext: in plaintext in flash or RAM
 *   - key store: in a dedicated hardware database whose entries can not be directly
 *     read out.
 *
 *  Each storage option requires different approaches to handling the keying material
 *  when performing a crypto operation. In order to separate these concerns from
 *  the API of the various crypto drivers available with TI-RTOS, the CryptoKey
 *  type abstracts away from these details. It does not contain any cryptographic
 *  keying material itself but instead contains the details necessary for drivers to use the
 *  keying material. The driver implementation handles preparing and moving the keying
 *  material as necessary to perform the desired crypto operation.
 *
 *  The same CryptoKey may be passed to crypto APIs of different modes subject to
 *  restrictions placed on the key by their storage types. Plaintext keys may be used
 *  without restriction while key store keys have their permitted uses
 *  restricted when the keying material is loaded.
 *  These restrictions are specified in a CryptoKey_SecurityPolicy that is device-specific
 *  and depends on the hardware capability of the device.
 *
 *  An application should never access a field within a CryptoKey struct itself.
 *  Where needed, helper functions are provided to do so.
 *
 *  Before using a CryptoKey in another crypto API call, it must be initialized
 *  with a call to one of the initialization functions.
 *   - CryptoKeyPlaintext_initKey()
 *   - CryptoKeyPlaintext_initBlankKey()
 *   - KeyStore_PSA_initKey()
 *   - KeyStore_PSA_initBlankKey()
 *
 *  The keystore CryptoKeys may be used to load a key into a key store after
 *  its respective _init call.
 *
 *  CryptoKeys can be initialized "blank", without keying material but with an empty buffer
 *  or key store entry, to encode the destination of a key to be created in the
 *  future. This way, keys may be generated securely within a key store
 *  for example and never even be stored in RAM temporarily.
 *
 *  Not all devices support all CryptoKey functionality. This is hardware-dependent.
 *
 */

#ifndef ti_drivers_cryptoutils_cyptokey_CryptoKey__include
#define ti_drivers_cryptoutils_cyptokey_CryptoKey__include

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!

 */

/**
 *  @defgroup CryptoKey_CONTROL Status codes
 *  These CryptoKey macros are reservations for CryptoKey.h
 *  @{
 */

/*!
 *  Common CryptoKey_control status code reservation offset.
 *  CryptoKey driver implementations should offset status codes with
 *  CryptoKey_STATUS_RESERVED growing negatively.
 *
 *  Example implementation specific status codes:
 *  @code
 *  #define CryptoKeyXYZ_STATUS_ERROR0    CryptoKey_STATUS_RESERVED - 0
 *  #define CryptoKeyXYZ_STATUS_ERROR1    CryptoKey_STATUS_RESERVED - 1
 *  #define CryptoKeyXYZ_STATUS_ERROR2    CryptoKey_STATUS_RESERVED - 2
 *  @endcode
 */
#define CryptoKey_STATUS_RESERVED (-32)

/**
 *  @defgroup CryptoKey_STATUS Status Codes
 *  CryptoKey_STATUS_*  macros are general status codes returned by CryptoKey_control()
 *  @{
 *  @ingroup CryptoKey_CONTROL
 */

/*!
 *  @brief   Successful status code
 *
 *  CryptoKey_control() returns CryptoKey_STATUS_SUCCESS if the control code was executed
 *  successfully.
 */
#define CryptoKey_STATUS_SUCCESS (0)

/*!
 *  @brief   Generic error status code
 *
 *  CryptoKey_control() returns CryptoKey_STATUS_ERROR if the control code was not executed
 *  successfully.
 */
#define CryptoKey_STATUS_ERROR (-1)

/*!
 *  @brief   Returned if the encoding of a CryptoKey is not a CryptoKey_Encoding value
 *
 *  CryptoKey_control() returns CryptoKey_STATUS_ERROR if the control code was not executed
 *  successfully.
 */
#define CryptoKey_STATUS_UNDEFINED_ENCODING (-2)

/** @}*/

/** @}*/

/*
 * CRYPTOKEY_HSM is being used to mask bit 6 which determines which accelerator to use.
 * Any encoding that is ORed with CRYPTOKEY_HSM indicates that the HSM is the engine of choice for the operation
 */
#define CRYPTOKEY_HSM 0x20U

#define CRYPTOKEY_PLAINTEXT       0x02U
#define CRYPTOKEY_BLANK_PLAINTEXT 0x04U
#define CRYPTOKEY_KEYSTORE        0x08U
#define CRYPTOKEY_BLANK_KEYSTORE  0x10U

/*!
 *  @brief  List of the different types of CryptoKey.
 *  _HSM encodings are only available for select devices, CC27XX.
 */
typedef uint8_t CryptoKey_Encoding;
static const CryptoKey_Encoding CryptoKey_PLAINTEXT           = CRYPTOKEY_PLAINTEXT;
static const CryptoKey_Encoding CryptoKey_BLANK_PLAINTEXT     = CRYPTOKEY_BLANK_PLAINTEXT;
static const CryptoKey_Encoding CryptoKey_KEYSTORE            = CRYPTOKEY_KEYSTORE;
static const CryptoKey_Encoding CryptoKey_BLANK_KEYSTORE      = CRYPTOKEY_BLANK_KEYSTORE;
static const CryptoKey_Encoding CryptoKey_PLAINTEXT_HSM       = CRYPTOKEY_PLAINTEXT | CRYPTOKEY_HSM;
static const CryptoKey_Encoding CryptoKey_BLANK_PLAINTEXT_HSM = CRYPTOKEY_BLANK_PLAINTEXT | CRYPTOKEY_HSM;
static const CryptoKey_Encoding CryptoKey_KEYSTORE_HSM        = CRYPTOKEY_KEYSTORE | CRYPTOKEY_HSM;
static const CryptoKey_Encoding CryptoKey_BLANK_KEYSTORE_HSM  = CRYPTOKEY_BLANK_KEYSTORE | CRYPTOKEY_HSM;

/*!
 *  @brief  Plaintext CryptoKey datastructure.
 *
 *  This structure contains all the information necessary to access keying material stored
 *  in plaintext form in flash or RAM.
 */
typedef struct
{
    uint8_t *keyMaterial;
    uint32_t keyLength;
} CryptoKey_Plaintext;

/*!
 *  @brief  Key store CryptoKey datastructure.
 *
 *  This structure contains all the information necessary to access keying material stored
 *  in a dedicated key store or key database with memory access controls.
 *  The application must ensure that the key attributes struct used to initialize the pointer
 *  #keyAttributes must either be a global variable or it must be available in the context of the
 *  function that makes the call to import the key associated with the same key attribute.
 *  Otherwise, the keyAttributes pointer will point to a location in stack that could be deallocated.
 */
typedef struct
{
    uint32_t keyLength;
    uint32_t keyID;
    const void *keyAttributes;
} CryptoKey_KeyStore;

/*!
 *  @brief  CryptoKey datastructure.
 *
 *  This structure contains a CryptoKey_Encoding and one of
 * - CryptoKey_Plaintext
 * - CryptoKey_KeyStore
 */
typedef struct
{
    CryptoKey_Encoding encoding;
    union
    {
        CryptoKey_Plaintext plaintext;
        CryptoKey_KeyStore keyStore;
    } u;
} CryptoKey;

/*!
 *  @brief  Structure that specifies the restrictions on a CryptoKey
 *
 *  This structure is device-specific and declared here in incomplete form.
 *  The structure is fully defined in CryptoKeyDEVICE.h. This creates a link-time binding
 *  when using the structure with key store functions. If the instance
 *  of the CryptoKey_SecurityPolicy is kept in a device-specific application-file,
 *  the generic application code may still use references to it despite being
 *  an incomplete type in the generic application file at compile time.
 */
typedef struct CryptoKey_SecurityPolicy_ CryptoKey_SecurityPolicy;

/*!
 *  @brief Gets the key type of the CryptoKey
 *
 *  @param [in]     keyHandle   Pointer to a CryptoKey
 *  @param [out]    keyType     Type of the CryptoKey
 *
 *  @return Returns a status code
 */
int_fast16_t CryptoKey_getCryptoKeyType(const CryptoKey *keyHandle, CryptoKey_Encoding *keyType);

/*!
 *  @brief Whether the CryptoKey is 'blank' or represents valid keying material
 *
 *  @param [in]     keyHandle   Pointer to a CryptoKey
 *  @param [out]    isBlank     Whether the CryptoKey is 'blank' or not
 *
 *  @return Returns a status code
 */
int_fast16_t CryptoKey_isBlank(const CryptoKey *keyHandle, bool *isBlank);

/*!
 *  @brief Function to initialize the CryptoKey_SecurityPolicy struct to its defaults
 *
 *  This will zero-out all fields that cannot be set to safe defaults
 *
 *  @param [in]     policy   Pointer to a CryptoKey_SecurityPolicy
 *
 *  @return Returns a status code
 */
int_fast16_t CryptoKey_initSecurityPolicy(CryptoKey_SecurityPolicy *policy);

/*!
 *  @brief Function to verify a secure CryptoKey
 *
 *  This will check that the key type is valid and verify plaintext key material
 *  is located in non-secure read-access memory.
 *
 *  @note This function may not be available in all implementations
 *
 *  @param [in]     secureKey   Pointer to a CryptoKey struct located in secure memory
 *
 *  @retval CryptoKey_STATUS_SUCCESS  Key passes all verification checks
 *  @retval CryptoKey_STATUS_ERROR    Key fails any verification check
 */
int_fast16_t CryptoKey_verifySecureInputKey(const CryptoKey *secureKey);

/*!
 *  @brief Function to verify a secure output CryptoKey
 *
 *  This will check that the key type is valid and verify plaintext key material
 *  is located in non-secure RW-access memory.
 *
 *  @note This function may not be available in all implementations
 *
 *  @param [in]     secureKey   Pointer to a CryptoKey struct located in secure memory
 *
 *  @retval CryptoKey_STATUS_SUCCESS  Key passes all verification checks
 *  @retval CryptoKey_STATUS_ERROR    Key fails any verification check
 */
int_fast16_t CryptoKey_verifySecureOutputKey(const CryptoKey *secureKey);

/*!
 *  @brief Function to copy and verify a secure input CryptoKey
 *
 *  This will check that the source CryptoKey struct is located in non-secure
 *  read-access memory, copy the CryptoKey struct from the src to dst, and check
 *  that the key type is valid and verify plaintext key material is located in
 *  non-secure read-access memory.
 *
 *  @note This function may not be available in all implementations
 *
 *  @param [out]    dst         Pointer to the destination CryptoKey struct located in secure memory
 *  @param [in,out] src         Pointer to a source CryptoKey struct pointer located in secure memory
 *                              which will be updated to point to the destination CryptoKey struct
 *
 *  @retval CryptoKey_STATUS_SUCCESS  Key passes all verification checks
 *  @retval CryptoKey_STATUS_ERROR    Key fails any verification check
 */
int_fast16_t CryptoKey_copySecureInputKey(CryptoKey *dst, const CryptoKey **src);

/*!
 *  @brief Function to copy and verify a secure output CryptoKey
 *
 *  This will check that the source CryptoKey struct is located in non-secure
 *  RW-access memory, copy the CryptoKey struct from the src to dst, and check
 *  that the key type is valid and verify plaintext key material is located in
 *  non-secure RW-access memory.
 *
 *  @note This function may not be available in all implementations
 *
 *  @param [out]    dst         Pointer to the destination CryptoKey struct located in secure memory
 *  @param [in,out] src         Pointer to a source CryptoKey struct pointer located in secure memory
 *                              which will be updated to point to the destination CryptoKey struct
 *
 *  @retval CryptoKey_STATUS_SUCCESS  Key passes all verification checks
 *  @retval CryptoKey_STATUS_ERROR    Key fails any verification check
 */
int_fast16_t CryptoKey_copySecureOutputKey(CryptoKey *dst, CryptoKey **src);

#ifdef __cplusplus
}
#endif

#endif /* ti_drivers_cryptoutils_cyptokey_CryptoKey__include */
