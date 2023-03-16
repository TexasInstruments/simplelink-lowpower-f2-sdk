/*
 * Copyright (c) 2022, Texas Instruments Incorporated
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
 *  @file       CryptoKeyKeyStore_PSA.h
 *  @brief      CryptoKeyKeyStore_PSA driver header
 *
 * @warning     This is a beta API. It may change in future releases.
 *
 * # Overview #
 * This file contains the APIs to import, export, copy, and destroy key store
 * CryptoKeys. Key store CryptoKeys reference keying material stored in flash or
 * RAM using a key identifier. These CryptoKeys are subject to enforced usage
 * restrictions as defined by the key attributes assigned during key import.
 * This file provides definitions that are common between the Non-Secure
 * Processing Environment (NSPE) and Secure Processing Environment (SPE).
 *
 * # Usage #
 *
 * After calling the key store initialization function, a CryptoKey must be
 * imported into the key store before it can be used for a crypto operation APIs
 * which takes a CryptoKey as an input.

 * @anchor ti_drivers_cryptoutils_cryptokey_CryptoKeyKeyStore_PSA_Example
 *
 * ## Importing and exporting AES-CCM KeyStore keys #
 *
 * @code
 *
 * #include <ti/drivers/cryptoutils/cryptokey/CryptoKeyKeyStore_PSA.h>
 * #include <ti/drivers/cryptoutils/cryptokey/CryptoKeyKeyStore_PSA_helpers.h>
 * ....
 *
 * uint8_t keyingMaterial[16]; //Assume keying material is already available
 * uint8_t keyingMaterial2[16];
 * CryptoKey cryptoKey;
 * KeyStore_PSA_KeyFileId keyID;
 * int_fast16_t status;
 * int_fast16_t returnedLength;
 * KeyStore_PSA_KeyAttributes attributes = KEYSTORE_PSA_KEY_ATTRIBUTES_INIT;
 *
 * // Assign key attributes.
 * KeyStore_PSA_setKeyUsageFlags(&attributes, (KEYSTORE_PSA_KEY_USAGE_DECRYPT | KEYSTORE_PSA_KEY_USAGE_ENCRYPT));
 * KeyStore_PSA_setKeyAlgorithm(&attributes, KEYSTORE_PSA_ALG_CCM);
 * KeyStore_PSA_setKeyType(&attributes, KEYSTORE_PSA_KEY_TYPE_AES);
 * KeyStore_PSA_setKeyLifetime(&attributes, KEYSTORE_PSA_KEY_LIFETIME_PERSISTENT);
 * keyID.key_id = KEYSTORE_PSA_KEY_ID_PERSISTENT_USER_MIN;
 * keyID.owner  = -1;
 * KeyStore_PSA_setKeyId(&attributes, keyID);
 *
 * // Import the keyingMaterial
 * status = KeyStore_PSA_importKey(&attributes, keyingMaterial, sizeof(keyingMaterial), &keyID);
 *
 * if (status != KEYSTORE_PSA_STATUS_SUCCESS)
 * {
 *      // Handle error
 * }
 *
 * //Initialize the cryptoKey
 * KeyStore_PSA_initKey(&cryptoKey, keyID, sizeof(keyingMaterial), NULL);
 *
 * // Export the previously imported CryptoKey using keyID
 * status = KeyStore_PSA_exportKey(keyID, keyingMaterial2, sizeof(keyingMaterial2), &returnedLength);
 *
 * if (status != KEYSTORE_PSA_STATUS_SUCCESS)
 * {
 *      // Handle error
 * }
 *
 * // Destroy key after use
 * status = KeyStore_PSA_destroyKey(keyID);
 *
 * if (status != KEYSTORE_PSA_STATUS_SUCCESS)
 * {
 *      // Handle error
 * }
 * @endcode
 *
 */

#ifndef ti_drivers_CryptoKeyKeyStore_PSA__include
#define ti_drivers_CryptoKeyKeyStore_PSA__include

#if (TFM_ENABLED == 0) || defined(TFM_PSA_API) /* TFM_PSA_API indicates this is a TF-M build */
    #include <third_party/mbedcrypto/include/psa/crypto.h>
#else
    #include <third_party/tfm/interface/include/psa/crypto.h>
#endif /* #if (TFM_ENABLED == 0) || defined(TFM_PSA_API) */

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Encoding of permitted usage on a key. */
typedef psa_key_usage_t KeyStore_PSA_KeyUsage;

/** Encoding of key lifetimes.
 *
 * The lifetime of a key indicates where it is stored and what system
 * actions may create and destroy it.
 *
 * Keys with the lifetime #KEYSTORE_PSA_KEY_LIFETIME_VOLATILE are automatically
 * destroyed when the application terminates or on a power reset.
 *
 * Keys with a lifetime other than #KEYSTORE_PSA_KEY_LIFETIME_VOLATILE are said
 * to be _persistent_.
 * Persistent keys are preserved if the application or the system restarts.
 */
typedef psa_key_lifetime_t KeyStore_PSA_KeyLifetime;

/** @defgroup KeyStore_PSA_Statuses Key Store status return values.
 */
/** The action was completed successfully. */
#define KEYSTORE_PSA_STATUS_SUCCESS ((int_fast16_t)PSA_SUCCESS)

/** An error occurred that does not correspond to any defined
 * failure cause.
 *
 * Implementations may use this error code if none of the other standard
 * error codes are applicable. */
#define KEYSTORE_PSA_STATUS_GENERIC_ERROR ((int_fast16_t)PSA_ERROR_GENERIC_ERROR)

/** The requested operation or a parameter is not supported
 * by this implementation.
 *
 * Implementations should return this error code when an enumeration
 * parameter such as a key type, algorithm, etc. is not recognized.
 * If a combination of parameters is recognized and identified as
 * not valid, return #KEYSTORE_PSA_STATUS_INVALID_ARGUMENT instead. */
#define KEYSTORE_PSA_STATUS_NOT_SUPPORTED ((int_fast16_t)PSA_ERROR_NOT_SUPPORTED)

/** The requested action is denied by a policy.
 *
 * Implementations should return this error code when the parameters
 * are recognized as valid and supported, and a policy explicitly
 * denies the requested operation.
 *
 * If a subset of the parameters of a function call identify a
 * forbidden operation, and another subset of the parameters are
 * not valid or not supported, it is unspecified whether the function
 * returns #KEYSTORE_PSA_STATUS_NOT_PERMITTED, #KEYSTORE_PSA_STATUS_NOT_SUPPORTED or
 * #KEYSTORE_PSA_STATUS_INVALID_ARGUMENT. */
#define KEYSTORE_PSA_STATUS_NOT_PERMITTED ((int_fast16_t)PSA_ERROR_NOT_PERMITTED)

/** The key ID is not valid or does not exist.
 */
#define KEYSTORE_PSA_STATUS_INVALID_KEY_ID ((int_fast16_t)PSA_ERROR_INVALID_HANDLE)

/** An output buffer is too small.
 *
 * Applications can call the @c PSA_xxx_SIZE macro listed in the function
 * description to determine a sufficient buffer size.
 *
 * Implementations should preferably return this error code only
 * in cases when performing the operation with a larger output
 * buffer would succeed. However implementations may return this
 * error if a function has invalid or unsupported parameters in addition
 * to the parameters that determine the necessary output buffer size. */
#define KEYSTORE_PSA_STATUS_BUFFER_TOO_SMALL ((int_fast16_t)PSA_ERROR_BUFFER_TOO_SMALL)

/** Asking for an item that already exists
 *
 * Implementations should return this error, when attempting
 * to write an item (like a key) that already exists. */
#define KEYSTORE_PSA_STATUS_ALREADY_EXISTS ((int_fast16_t)PSA_ERROR_ALREADY_EXISTS)

/** Asking for an item that doesn't exist
 *
 * Implementations should return this error, if a requested item (like
 * a key) does not exist. */
#define KEYSTORE_PSA_STATUS_DOES_NOT_EXIST ((int_fast16_t)PSA_ERROR_DOES_NOT_EXIST)

/** The requested action cannot be performed in the current state.
 *
 * Multipart operations return this error when one of the
 * functions is called out of sequence. Refer to the function
 * descriptions for permitted sequencing of functions.
 *
 * Implementations shall not return this error code to indicate that a key
 * either exists or not, but shall instead return
 * #KEYSTORE_PSA_STATUS_ALREADY_EXISTS or #KEYSTORE_PSA_STATUS_DOES_NOT_EXIST as
 * applicable.
 *
 * Implementations shall not return this error code to indicate that a
 * key ID is invalid, but shall return #KEYSTORE_PSA_STATUS_INVALID_KEY_ID
 * instead. */
#define KEYSTORE_PSA_STATUS_BAD_STATE ((int_fast16_t)PSA_ERROR_BAD_STATE)

/** The parameters passed to the function are invalid.
 *
 * Implementations may return this error any time a parameter or
 * combination of parameters are recognized as invalid.
 *
 * Implementations shall not return this error code to indicate that a
 * key ID is invalid, but shall return #KEYSTORE_PSA_STATUS_INVALID_KEY_ID
 * instead.
 */
#define KEYSTORE_PSA_STATUS_INVALID_ARGUMENT ((int_fast16_t)PSA_ERROR_INVALID_ARGUMENT)

/** There is not enough runtime memory.
 *
 * If the action is carried out across multiple security realms, this
 * error can refer to available memory in any of the security realms. */
#define KEYSTORE_PSA_STATUS_INSUFFICIENT_MEMORY ((int_fast16_t)PSA_ERROR_INSUFFICIENT_MEMORY)

/** There is not enough persistent storage.
 *
 * Functions that modify the key storage return this error code if
 * there is insufficient storage space on the host media. In addition,
 * many functions that do not otherwise access storage may return this
 * error code if the implementation requires a mandatory log entry for
 * the requested action and the log storage space is full. */
#define KEYSTORE_PSA_STATUS_INSUFFICIENT_STORAGE ((int_fast16_t)PSA_ERROR_INSUFFICIENT_STORAGE)

/** There was a communication failure inside the implementation.
 *
 * This can indicate a communication failure between the application
 * and an external cryptoprocessor or between the cryptoprocessor and
 * an external volatile or persistent memory. A communication failure
 * may be transient or permanent depending on the cause.
 *
 * @warning If a function returns this error, it is undetermined
 * whether the requested action has completed or not. Implementations
 * should return #KEYSTORE_PSA_STATUS_SUCCESS on successful completion
 * whenever possible, however functions may return
 * #KEYSTORE_PSA_STATUS_COMMUNICATION_FAILURE if the requested action was completed
 * successfully in an external cryptoprocessor but there was a breakdown of
 * communication before the cryptoprocessor could report the status to the
 * application.
 */
#define KEYSTORE_PSA_STATUS_COMMUNICATION_FAILURE ((int_fast16_t)PSA_ERROR_COMMUNICATION_FAILURE)

/** There was a storage failure that may have led to data loss.
 *
 * This error indicates that some persistent storage is corrupted.
 * It should not be used for a corruption of volatile memory
 * (use #KEYSTORE_PSA_STATUS_CORRUPTION_DETECTED), for a communication error
 * between the cryptoprocessor and its external storage (use
 * #KEYSTORE_PSA_STATUS_COMMUNICATION_FAILURE), or when the storage is
 * in a valid state but is full (use #KEYSTORE_PSA_STATUS_INSUFFICIENT_STORAGE).
 *
 * Note that a storage failure does not indicate that any data that was
 * previously read is invalid. However this previously read data may no
 * longer be readable from storage.
 *
 * When a storage failure occurs, it is no longer possible to ensure
 * the global integrity of the keystore. Depending on the global
 * integrity guarantees offered by the implementation, access to other
 * data may or may not fail even if the data is still readable but
 * its integrity cannot be guaranteed.
 *
 * Implementations should only use this error code to report a
 * permanent storage corruption. However application writers should
 * keep in mind that transient errors while reading the storage may be
 * reported using this error code. */
#define KEYSTORE_PSA_STATUS_STORAGE_FAILURE ((int_fast16_t)PSA_ERROR_STORAGE_FAILURE)

/** A hardware failure was detected.
 *
 * A hardware failure may be transient or permanent depending on the
 * cause. */
#define KEYSTORE_PSA_STATUS_HARDWARE_FAILURE ((int_fast16_t)PSA_ERROR_HARDWARE_FAILURE)

/** There is not enough entropy to generate random data needed
 * for the requested action.
 *
 * This error indicates a failure of a hardware random generator.
 * Application writers should note that this error can be returned not
 * only by functions whose purpose is to generate random data, such
 * as key, IV or nonce generation, but also by functions that execute
 * an algorithm with a randomized result, as well as functions that
 * use randomization of intermediate computations as a countermeasure
 * to certain attacks.
 *
 * Implementations should avoid returning this error after KeyStore_PSA_init()
 * has succeeded. Implementations should generate sufficient
 * entropy during initialization and subsequently use a cryptographically
 * secure pseudorandom generator (PRNG). However implementations may return
 * this error at any time if a policy requires the PRNG to be reseeded
 * during normal operation. */
#define KEYSTORE_PSA_STATUS_INSUFFICIENT_ENTROPY ((int_fast16_t)PSA_ERROR_INSUFFICIENT_ENTROPY)

/** A tampering attempt was detected.
 *
 * If an application receives this error code, there is no guarantee
 * that previously accessed or computed data was correct and remains
 * confidential. Applications should not perform any security function
 * and should enter a safe failure state.
 *
 * Implementations may return this error code if they detect an invalid
 * state that cannot happen during normal operation and that indicates
 * that the implementation's security guarantees no longer hold. Depending
 * on the implementation architecture and on its security and safety goals,
 * the implementation may forcibly terminate the application.
 *
 * This error code is intended as a last resort when a security breach
 * is detected and it is unsure whether the keystore data is still
 * protected. Implementations shall only return this error code
 * to report an alarm from a tampering detector, to indicate that
 * the confidentiality of stored data can no longer be guaranteed,
 * or to indicate that the integrity of previously returned data is now
 * considered compromised. Implementations shall not use this error code
 * to indicate a hardware failure that merely makes it impossible to
 * perform the requested operation (use #KEYSTORE_PSA_STATUS_COMMUNICATION_FAILURE,
 * #KEYSTORE_PSA_STATUS_STORAGE_FAILURE, #KEYSTORE_PSA_STATUS_HARDWARE_FAILURE or other
 * applicable error code instead).
 *
 * This error indicates an attack against the application. Implementations
 * shall not return this error code as a consequence of the behavior of
 * the application itself. */
#define KEYSTORE_PSA_STATUS_CORRUPTION_DETECTED ((int_fast16_t)PSA_ERROR_CORRUPTION_DETECTED)

/*!
 * @brief   An error status code returned if the hardware or software resource
 * is currently unavailable.
 *
 * KeyStore driver implementation may have limitations on how
 * many clients can simultaneously perform operations on the same key. This status code is
 * returned if the mutual exclusion mechanism signals that an operation cannot
 * currently be performed.
 */

#define KEYSTORE_PSA_STATUS_RESOURCE_UNAVAILABLE ((int_fast16_t)-250)

/** @brief Encoding of a key type.
 */
typedef psa_key_type_t KeyStore_PSA_KeyType;

/** Raw data.
 *
 * A "key" of this type cannot be used for any cryptographic operation.
 * Applications may use this type to store arbitrary data in the keystore. */
#define KEYSTORE_PSA_KEY_TYPE_RAW_DATA ((KeyStore_PSA_KeyType)PSA_KEY_TYPE_RAW_DATA)

/** HMAC key.
 *
 * The key policy determines which underlying hash algorithm the key can be
 * used for.
 *
 * HMAC keys should generally have the same size as the underlying hash. */
#define KEYSTORE_PSA_KEY_TYPE_HMAC ((KeyStore_PSA_KeyType)PSA_KEY_TYPE_HMAC)

/** A secret for key derivation.
 *
 * The key policy determines which key derivation algorithm the key
 * can be used for.
 */
#define KEYSTORE_PSA_KEY_TYPE_DERIVE ((KeyStore_PSA_KeyType)PSA_KEY_TYPE_DERIVE)

/** Key for a cipher, AEAD or MAC algorithm based on the AES block cipher.
 *
 * The size of the key can be 16 bytes (AES-128), 24 bytes (AES-192) or
 * 32 bytes (AES-256).
 */
#define KEYSTORE_PSA_KEY_TYPE_AES ((KeyStore_PSA_KeyType)PSA_KEY_TYPE_AES)

/** Key for a cipher or MAC algorithm based on DES or 3DES (Triple-DES).
 *
 * The size of the key can be 64 bits (single DES), 128 bits (2-key 3DES) or
 * 192 bits (3-key 3DES).
 *
 * Note that single DES and 2-key 3DES are weak and strongly
 * deprecated and should only be used to decrypt legacy data. 3-key 3DES
 * is weak and deprecated and should only be used in legacy protocols.
 */
#define KEYSTORE_PSA_KEY_TYPE_DES ((KeyStore_PSA_KeyType)PSA_KEY_TYPE_DES)

/** Key for a cipher, AEAD or MAC algorithm based on the
 * Camellia block cipher. */
#define KEYSTORE_PSA_KEY_TYPE_CAMELLIA ((KeyStore_PSA_KeyType)PSA_KEY_TYPE_CAMELLIA)

/** Key for the RC4 stream cipher.
 *
 * Note that RC4 is weak and deprecated and should only be used in
 * legacy protocols. */
#define KEYSTORE_PSA_KEY_TYPE_ARC4 ((KeyStore_PSA_KeyType)PSA_KEY_TYPE_ARC4)

/** Key for the ChaCha20 stream cipher or the Chacha20-Poly1305 AEAD algorithm.
 *
 * ChaCha20 and the ChaCha20_Poly1305 construction are defined in RFC 7539.
 *
 * Implementations must support 12-byte nonces, may support 8-byte nonces,
 * and should reject other sizes.
 */
#define KEYSTORE_PSA_KEY_TYPE_CHACHA20 ((KeyStore_PSA_KeyType)PSA_KEY_TYPE_CHACHA20)

/** RSA public key.
 *
 * The size of an RSA key is the bit size of the modulus.
 */
#define KEYSTORE_PSA_KEY_TYPE_RSA_PUBLIC_KEY ((KeyStore_PSA_KeyType)PSA_KEY_TYPE_RSA_PUBLIC_KEY)
/** RSA key pair (private and public key).
 *
 * The size of an RSA key is the bit size of the modulus.
 */
#define KEYSTORE_PSA_KEY_TYPE_RSA_KEY_PAIR   ((KeyStore_PSA_KeyType)PSA_KEY_TYPE_RSA_KEY_PAIR)
/** Whether a key type is an RSA key (pair or public-only). */
#define PSA_KEY_TYPE_IS_RSA(type)            (PSA_KEY_TYPE_PUBLIC_KEY_OF_KEY_PAIR(type) == PSA_KEY_TYPE_RSA_PUBLIC_KEY)

#define KEYSTORE_PSA_KEY_TYPE_ECC_PUBLIC_KEY_BASE ((KeyStore_PSA_KeyType)PSA_KEY_TYPE_ECC_PUBLIC_KEY_BASE)
#define KEYSTORE_PSA_KEY_TYPE_ECC_KEY_PAIR_BASE   ((KeyStore_PSA_KeyType)PSA_KEY_TYPE_ECC_KEY_PAIR_BASE)

/** @brief Encoding of a cryptographic algorithm.
 *
 * For algorithms that can be applied to multiple key types, this type
 * does not encode the key type. For example, for symmetric ciphers
 * based on a block cipher, #KeyStore_PSA_Algorithm encodes the block cipher
 * mode and the padding mode while the block cipher itself is encoded
 * via #KeyStore_PSA_KeyType.
 */
typedef psa_algorithm_t KeyStore_PSA_Algorithm;

/** SHA2-224 */
#define KEYSTORE_PSA_ALG_SHA_224     ((KeyStore_PSA_Algorithm)PSA_ALG_SHA_224)
/** SHA2-256 */
#define KEYSTORE_PSA_ALG_SHA_256     ((KeyStore_PSA_Algorithm)PSA_ALG_SHA_256)
/** SHA2-384 */
#define KEYSTORE_PSA_ALG_SHA_384     ((KeyStore_PSA_Algorithm)PSA_ALG_SHA_384)
/** SHA2-512 */
#define KEYSTORE_PSA_ALG_SHA_512     ((KeyStore_PSA_Algorithm)PSA_ALG_SHA_512)
/** SHA2-512/224 */
#define KEYSTORE_PSA_ALG_SHA_512_224 ((KeyStore_PSA_Algorithm)PSA_ALG_SHA_512_224)
/** SHA2-512/256 */
#define KEYSTORE_PSA_ALG_SHA_512_256 ((KeyStore_PSA_Algorithm)PSA_ALG_SHA_512_256)

/** The CBC-MAC construction over a block cipher
 *
 * @warning CBC-MAC is insecure in many cases.
 * A more secure mode, such as #KEYSTORE_PSA_ALG_CMAC, is recommended.
 */
#define KEYSTORE_PSA_ALG_CBC_MAC ((KeyStore_PSA_Algorithm)PSA_ALG_CBC_MAC)
/** The CMAC construction over a block cipher */
#define KEYSTORE_PSA_ALG_CMAC    ((KeyStore_PSA_Algorithm)PSA_ALG_CMAC)

/** The CTR stream cipher mode.
 *
 * CTR is a stream cipher which is built from a block cipher.
 * The underlying block cipher is determined by the key type.
 * For example, to use AES-128-CTR, use this algorithm with
 * a key of type #KEYSTORE_PSA_KEY_TYPE_AES and a length of 128 bits (16 bytes).
 */
#define KEYSTORE_PSA_ALG_CTR ((KeyStore_PSA_Algorithm)PSA_ALG_CTR)

/** The CBC block cipher chaining mode, with no padding.
 *
 * The underlying block cipher is determined by the key type.
 *
 * This symmetric cipher mode can only be used with messages whose lengths
 * are whole number of blocks for the chosen block cipher.
 */
#define KEYSTORE_PSA_ALG_CBC_NO_PADDING ((KeyStore_PSA_Algorithm)PSA_ALG_CBC_NO_PADDING)

/** The CCM authenticated encryption algorithm.
 *
 * The underlying block cipher is determined by the key type.
 */
#define KEYSTORE_PSA_ALG_CCM ((KeyStore_PSA_Algorithm)PSA_ALG_CCM)

/** The GCM authenticated encryption algorithm.
 *
 * The underlying block cipher is determined by the key type.
 */
#define KEYSTORE_PSA_ALG_GCM ((KeyStore_PSA_Algorithm)PSA_ALG_GCM)

/** The Chacha20-Poly1305 AEAD algorithm.
 *
 * The ChaCha20_Poly1305 construction is defined in RFC 7539.
 *
 * Implementations must support 12-byte nonces, may support 8-byte nonces,
 * and should reject other sizes.
 *
 * Implementations must support 16-byte tags and should reject other sizes.
 */
#define KEYSTORE_PSA_ALG_CHACHA20_POLY1305 ((KeyStore_PSA_Algorithm)PSA_ALG_CHACHA20_POLY1305)

/** The elliptic curve Diffie-Hellman (ECDH) key agreement algorithm.
 *
 * The shared secret produced by key agreement is the x-coordinate of
 * the shared secret point. It is always `ceiling(m / 8)` bytes long where
 * `m` is the bit size associated with the curve, i.e. the bit size of the
 * order of the curve's coordinate field. When `m` is not a multiple of 8,
 * the byte containing the most significant bit of the shared secret
 * is padded with zero bits. The byte order is either little-endian
 * or big-endian depending on the curve type.
 *
 * - For Montgomery curves (curve types `PSA_ECC_FAMILY_CURVEXXX`),
 *   the shared secret is the x-coordinate of `d_A Q_B = d_B Q_A`
 *   in little-endian byte order.
 *   The bit size is 448 for Curve448 and 255 for Curve25519.
 * - For Weierstrass curves over prime fields (curve types
 *   `PSA_ECC_FAMILY_SECPXXX` and `PSA_ECC_FAMILY_BRAINPOOL_PXXX`),
 *   the shared secret is the x-coordinate of `d_A Q_B = d_B Q_A`
 *   in big-endian byte order.
 *   The bit size is `m = ceiling(log_2(p))` for the field `F_p`.
 * - For Weierstrass curves over binary fields (curve types
 *   `PSA_ECC_FAMILY_SECTXXX`),
 *   the shared secret is the x-coordinate of `d_A Q_B = d_B Q_A`
 *   in big-endian byte order.
 *   The bit size is `m` for the field `F_{2^m}`.
 */
#define KEYSTORE_PSA_ALG_ECDH ((KeyStore_PSA_Algorithm)PSA_ALG_ECDH)

/* The encoding of curve identifiers is currently aligned with the
 * TLS Supported Groups Registry (formerly known as the
 * TLS EC Named Curve Registry)
 * https://www.iana.org/assignments/tls-parameters/tls-parameters.xhtml#tls-parameters-8
 * The values are defined by RFC 8422 and RFC 7027. */
#define KEYSTORE_PSA_ECC_CURVE_SECT163K1     ((KeyStore_PSA_KeyType)PSA_ECC_CURVE_SECT163K1)
#define KEYSTORE_PSA_ECC_CURVE_SECT163R1     ((KeyStore_PSA_KeyType)PSA_ECC_CURVE_SECT163R1)
#define KEYSTORE_PSA_ECC_CURVE_SECT163R2     ((KeyStore_PSA_KeyType)PSA_ECC_CURVE_SECT163R2)
#define KEYSTORE_PSA_ECC_CURVE_SECT193R1     ((KeyStore_PSA_KeyType)PSA_ECC_CURVE_SECT193R1)
#define KEYSTORE_PSA_ECC_CURVE_SECT193R2     ((KeyStore_PSA_KeyType)PSA_ECC_CURVE_SECT193R2)
#define KEYSTORE_PSA_ECC_CURVE_SECT233K1     ((KeyStore_PSA_KeyType)PSA_ECC_CURVE_SECT233K1)
#define KEYSTORE_PSA_ECC_CURVE_SECT233R1     ((KeyStore_PSA_KeyType)PSA_ECC_CURVE_SECT233R1)
#define KEYSTORE_PSA_ECC_CURVE_SECT239K1     ((KeyStore_PSA_KeyType)PSA_ECC_CURVE_SECT239K1)
#define KEYSTORE_PSA_ECC_CURVE_SECT283K1     ((KeyStore_PSA_KeyType)PSA_ECC_CURVE_SECT283K1)
#define KEYSTORE_PSA_ECC_CURVE_SECT283R1     ((KeyStore_PSA_KeyType)PSA_ECC_CURVE_SECT283R1)
#define KEYSTORE_PSA_ECC_CURVE_SECT409K1     ((KeyStore_PSA_KeyType)PSA_ECC_CURVE_SECT409K1)
#define KEYSTORE_PSA_ECC_CURVE_SECT409R1     ((KeyStore_PSA_KeyType)PSA_ECC_CURVE_SECT409R1)
#define KEYSTORE_PSA_ECC_CURVE_SECT571K1     ((KeyStore_PSA_KeyType)PSA_ECC_CURVE_SECT571K1)
#define KEYSTORE_PSA_ECC_CURVE_SECT571R1     ((KeyStore_PSA_KeyType)PSA_ECC_CURVE_SECT571R1)
#define KEYSTORE_PSA_ECC_CURVE_SECP160K1     ((KeyStore_PSA_KeyType)PSA_ECC_CURVE_SECP160K1)
#define KEYSTORE_PSA_ECC_CURVE_SECP160R1     ((KeyStore_PSA_KeyType)PSA_ECC_CURVE_SECP160R1)
#define KEYSTORE_PSA_ECC_CURVE_SECP160R2     ((KeyStore_PSA_KeyType)PSA_ECC_CURVE_SECP160R2)
#define KEYSTORE_PSA_ECC_CURVE_SECP192K1     ((KeyStore_PSA_KeyType)PSA_ECC_CURVE_SECP192K1)
#define KEYSTORE_PSA_ECC_CURVE_SECP192R1     ((KeyStore_PSA_KeyType)PSA_ECC_CURVE_SECP192R1)
#define KEYSTORE_PSA_ECC_CURVE_SECP224K1     ((KeyStore_PSA_KeyType)PSA_ECC_CURVE_SECP224K1)
#define KEYSTORE_PSA_ECC_CURVE_SECP224R1     ((KeyStore_PSA_KeyType)PSA_ECC_CURVE_SECP224R1)
#define KEYSTORE_PSA_ECC_CURVE_SECP256K1     ((KeyStore_PSA_KeyType)PSA_ECC_CURVE_SECP256K1)
#define KEYSTORE_PSA_ECC_CURVE_SECP256R1     ((KeyStore_PSA_KeyType)PSA_ECC_CURVE_SECP256R1)
#define KEYSTORE_PSA_ECC_CURVE_SECP384R1     ((KeyStore_PSA_KeyType)PSA_ECC_CURVE_SECP384R1)
#define KEYSTORE_PSA_ECC_CURVE_SECP521R1     ((KeyStore_PSA_KeyType)PSA_ECC_CURVE_SECP521R1)
#define KEYSTRORE_ECC_CURVE_BRAINPOOL_P256R1 ((KeyStore_PSA_KeyType)PSA_ECC_CURVE_BRAINPOOL_P256R1)
#define KEYSTRORE_ECC_CURVE_BRAINPOOL_P384R1 ((KeyStore_PSA_KeyType)PSA_ECC_CURVE_BRAINPOOL_P384R1)
#define KEYSTRORE_ECC_CURVE_BRAINPOOL_P512R1 ((KeyStore_PSA_KeyType)PSA_ECC_CURVE_BRAINPOOL_P512R1)
/** Cur KEYSTRORE_ECC_CURVE_SECPv((KeyStore_PSA_KeyType)e25519.
 *
 * This is the curve defined in Bernstein et al.,
 * _Curve25519: new Diffie-Hellman speed records_, LNCS 3958, 2006.
 * The algorithm #KEYSTORE_PSA_ALG_ECDH performs X25519 when used with this curve.
 */
#define KEYSTORE_PSA_ECC_CURVE_CURVE25519    ((KeyStore_PSA_KeyType)PSA_ECC_CURVE_CURVE25519)
/** Curve448
 *
 * This is the curve defined in Hamburg,
 * _Ed448-Goldilocks, a new elliptic curve_, NIST ECC Workshop, 2015.
 * The algorithm #KEYSTORE_PSA_ALG_ECDH performs X448 when used with this curve.
 */
#define KEYSTORE_PSA_ECC_CURVE_CURVE448      ((KeyStore_PSA_KeyType)PSA_ECC_CURVE_CURVE448)

/** Minimum value for a vendor-defined ECC curve identifier
 *
 * The range for vendor-defined curve identifiers is a subset of the IANA
 * registry private use range, `0xfe00` - `0xfeff`.
 */
#define KEYSTORE_PSA_ECC_CURVE_VENDOR_MIN ((KeyStore_PSA_KeyType)PSA_ECC_CURVE_VENDOR_MIN)
/** Maximum value for a vendor-defined ECC curve identifier
 *
 * The range for vendor-defined curve identifiers is a subset of the IANA
 * registry private use range, `0xfe00` - `0xfeff`.
 */
#define KEYSTORE_PSA_ECC_CURVE_VENDOR_MAX ((KeyStore_PSA_KeyType)PSA_ECC_CURVE_VENDOR_MAX)

/* Volatile Key Limit [1, PSA_KEY_SLOT_COUNT] for TF-Mv1.1
 * Upper limit for volatile keys, PSA_KEY_SLOT_COUNT (32) is the size of global slot array in mbedcrypto3.1
 */
#if (TFM_ENABLED == 0) || defined(TFM_PSA_API) /* TFM_PSA_API indicates this is a TF-M build */
    #define KEYSTORE_PSA_MAX_VOLATILE_KEY_ID PSA_KEY_SLOT_COUNT
#else
    /*
     * PSA_KEY_SLOT_COUNT is not available in TF-M's crypto.h so we must
     * hardcode it to match the value in mbedCrypto's header.
     */
    #define KEYSTORE_PSA_MAX_VOLATILE_KEY_ID 32
#endif

/* Macro to obtain size of struct member */
#define MEMBER_SIZE(type, member) sizeof(((type *)0)->member)

/** \defgroup key_lifetimes Key lifetimes
 * @{
 */

/** A volatile key only exists as long as the handle to it is not closed.
 * The key material is guaranteed to be erased on a power reset.
 */
#define KEYSTORE_PSA_KEY_LIFETIME_VOLATILE ((KeyStore_PSA_KeyLifetime)PSA_KEY_LIFETIME_VOLATILE)

/** The default storage area for persistent keys.
 *
 * A persistent key remains in storage until it is explicitly destroyed or
 * until the corresponding storage area is wiped. This specification does
 * not define any mechanism to wipe a storage area, but implementations may
 * provide their own mechanism (for example to perform a factory reset,
 * to prepare for device refurbishment, or to uninstall an application).
 *
 * This lifetime value is the default storage area for the calling
 * application. Implementations may offer other storage areas designated
 * by other lifetime values as implementation-specific extensions.
 */
#define KEYSTORE_PSA_KEY_LIFETIME_PERSISTENT ((KeyStore_PSA_KeyLifetime)PSA_KEY_LIFETIME_PERSISTENT)

/** The null key identifier.
 */
#define KEYSTORE_PSA_KEY_ID_NULL ((KeyStore_PSA_KeyFileId)0x0)

/** The minimum value for a key identifier chosen by the application.
 */
#define KEYSTORE_PSA_KEY_ID_USER_MIN ((KeyStore_PSA_KeyFileId)PSA_KEY_ID_USER_MIN)

/** The minimum value for a persistent key identifier chosen by the application.
 */
#define KEYSTORE_PSA_KEY_ID_PERSISTENT_USER_MIN 0x41

/** The maximum value for a persistent key identifier with associated certificate chosen by the application.
 * KeyStore uses Bit 29 in KeyID to indicate certificate storage.
 * Application may choose any key ID less than 2^29 for persistent keys
 * Persistent Key ID limit [KEYSTORE_PSA_MAX_VOLATILE_KEY_ID + 1, KEYSTORE_PSA_KEY_ID_USER_MIN]
 * (Setting bit 30 makes KeyID greater than PSA_KEY_ID_USER_MIN which will fall into VENDOR limit)
 * (Setting bit 31 makes KeyID greater than PSA_KEY_ID_VENDOR_MAX which is not a valid KeyID)
 */
#define KEYSTORE_PSA_KEY_ID_WITH_CERTIFICATE_USER_MAX 0x1fffffff

/**
 * @brief Bit mask to set 29th bit to indicate certificate storage
 *
 */
#define KEYSTORE_PSA_KEY_ID_CERTIFICATE_BIT (1 << 29)

/** The minimum value for a key identifier chosen by the implementation.
 */
#define KEYSTORE_PSA_KEY_ID_VENDOR_MIN ((KeyStore_PSA_KeyFileId)PSA_KEY_ID_VENDOR_MIN)

/** The maximum value for a key identifier chosen by the implementation.
 */
#define KEYSTORE_PSA_KEY_ID_VENDOR_MAX ((KeyStore_PSA_KeyFileId)PSA_KEY_ID_VENDOR_MAX)

/** Default Key Owner
 */
#define KEYSTORE_PSA_DEFAULT_OWNER -1

/**@}*/

/** \defgroup key_policies Key policies
 * @{
 */

/** Whether the key may be exported.
 *
 * A public key or the public part of a key pair may always be exported
 * regardless of the value of this permission flag.
 *
 * If a key does not have export permission, implementations shall not
 * allow the key to be exported in plain form from the cryptoprocessor,
 * whether through KeyStore_PSA_exportKey() or through a proprietary interface.
 * The key may however be exportable in a wrapped form, i.e. in a form
 * where it is encrypted by another key.
 */
#define KEYSTORE_PSA_KEY_USAGE_EXPORT ((KeyStore_PSA_KeyUsage)PSA_KEY_USAGE_EXPORT)

/** Whether the key may be copied.
 *
 * This flag allows the use of KeyStore_PSA_copyKey() to make a copy of the key
 * with the same policy or a more restrictive policy.
 *
 * For lifetimes for which the key is located in a secure element which
 * enforce the non-exportability of keys, copying a key outside the secure
 * element also requires the usage flag #KEYSTORE_PSA_KEY_USAGE_EXPORT.
 * Copying the key inside the secure element is permitted with just
 * #KEYSTORE_PSA_KEY_USAGE_COPY if the secure element supports it.
 * For keys with the lifetime #KEYSTORE_PSA_KEY_LIFETIME_VOLATILE or
 * #KEYSTORE_PSA_KEY_LIFETIME_PERSISTENT, the usage flag #KEYSTORE_PSA_KEY_USAGE_COPY
 * is sufficient to permit the copy.
 */
#define KEYSTORE_PSA_KEY_USAGE_COPY ((KeyStore_PSA_KeyUsage)PSA_KEY_USAGE_COPY)

/** Whether the key may be used to encrypt a message.
 *
 * This flag allows the key to be used for a symmetric encryption operation,
 * for an AEAD encryption-and-authentication operation,
 * or for an asymmetric encryption operation,
 * if otherwise permitted by the key's type and policy.
 *
 * For a key pair, this concerns the public key.
 */
#define KEYSTORE_PSA_KEY_USAGE_ENCRYPT ((KeyStore_PSA_KeyUsage)PSA_KEY_USAGE_ENCRYPT)

/** Whether the key may be used to decrypt a message.
 *
 * This flag allows the key to be used for a symmetric decryption operation,
 * for an AEAD decryption-and-verification operation,
 * or for an asymmetric decryption operation,
 * if otherwise permitted by the key's type and policy.
 *
 * For a key pair, this concerns the private key.
 */
#define KEYSTORE_PSA_KEY_USAGE_DECRYPT ((KeyStore_PSA_KeyUsage)PSA_KEY_USAGE_DECRYPT)

/** Whether the key may be used to sign a message.
 *
 * This flag allows the key to be used for a MAC calculation operation
 * or for an asymmetric signature operation,
 * if otherwise permitted by the key's type and policy.
 *
 * For a key pair, this concerns the private key.
 */
#define KEYSTORE_PSA_KEY_USAGE_SIGN_HASH ((KeyStore_PSA_KeyUsage)PSA_KEY_USAGE_SIGN_HASH)

/** Whether the key may be used to verify a message signature.
 *
 * This flag allows the key to be used for a MAC verification operation
 * or for an asymmetric signature verification operation,
 * if otherwise permitted by by the key's type and policy.
 *
 * For a key pair, this concerns the public key.
 */
#define KEYSTORE_PSA_KEY_USAGE_VERIFY_HASH ((KeyStore_PSA_KeyUsage)PSA_KEY_USAGE_VERIFY_HASH)

/** Whether the key may be used to derive other keys.
 */
#define KEYSTORE_PSA_KEY_USAGE_DERIVE ((KeyStore_PSA_KeyUsage)PSA_KEY_USAGE_DERIVE)
/**@}*/

#if (TFM_ENABLED == 0) || defined(TFM_PSA_API) /* TFM_PSA_API indicates this is a TF-M build */
    /** A Key owner is a PSA partition identifier. This definition follow
     * 'psa_key_owner_id_t' from crypto_platform.h */
    #if defined(MBEDTLS_PSA_CRYPTO_KEY_FILE_ID_ENCODES_OWNER)
/* Building for the PSA Crypto service on a PSA platform. */
/* A key owner is a PSA partition identifier. */
typedef psa_key_owner_id_t KeyStore_PSA_key_owner_id_t;
    #endif /* defined(MBEDTLS_PSA_CRYPTO_KEY_FILE_ID_ENCODES_OWNER) */

typedef psa_key_file_id_t KeyStore_PSA_KeyFileId;

/** \defgroup attributes Key attributes
 * @{
 */
/** The type of a structure containing key attributes.
 *
 * This is an opaque structure that can represent the metadata of a key
 * object. Metadata that can be stored in attributes includes:
 * - The location of the key in storage, indicated by its key identifier
 *   and its lifetime.
 * - The key's policy, comprising usage flags and a specification of
 *   the permitted algorithm(s).
 * - Information about the key itself: the key type and its size.
 *
 * The actual key material is not considered an attribute of a key.
 * Key attributes do not contain information that is generally considered
 * highly confidential.
 *
 * An attribute structure can be a simple data structure where each function
 * `KeyStore_PSA_setKeyXXX` sets a field and the corresponding function
 * `KeyStore_PSA_getKeyXXX` retrieves the value of the corresponding field.
 * However, implementations may report values that are equivalent to the
 * original one, but have a different encoding. For example, an
 * implementation may use a more compact representation for types where
 * many bit-patterns are invalid or not supported, and store all values
 * that it does not support as a special marker value. In such an
 * implementation, after setting an invalid value, the corresponding
 * get function returns an invalid value which may not be the one that
 * was originally stored.
 *
 * An attribute structure may contain references to auxiliary resources,
 * for example pointers to allocated memory or indirect references to
 * pre-calculated values. In order to free such resources, the application
 * must call KeyStore_PSA_resetKeyAttributes(). As an exception, calling
 * KeyStore_PSA_resetKeyAttributes() on an attribute structure is optional if
 * the structure has only been modified by the following functions
 * since it was initialized or last reset with
 * KeyStore_PSA_resetKeyAttributes():
 * - KeyStore_PSA_setKeyId()
 * - KeyStore_PSA_setKeyLifetime()
 * - KeyStore_PSA_setKeyType()
 * - KeyStore_PSA_setKeyBits()
 * - KeyStore_PSA_setKeyUsageFlags()
 * - KeyStore_PSA_setKeyAlgorithm()
 *
 * Before calling any function on a key attribute structure, the application
 * must initialize it by any of the following means:
 * - Set the structure to all-bits-zero, for example:
 *   \code
 *   KeyStore_PSA_KeyAttributes attributes;
 *   memset(&attributes, 0, sizeof(attributes));
 *   \endcode
 * - Initialize the structure to logical zero values, for example:
 *   \code
 *   KeyStore_PSA_KeyAttributes attributes = {0};
 *   \endcode
 * - Initialize the structure to the initializer
 * #KEYSTORE_PSA_KEY_ATTRIBUTES_INIT, for example: \code
 *   KeyStore_PSA_KeyAttributes attributes = KEYSTORE_PSA_KEY_ATTRIBUTES_INIT;
 *   \endcode
 *
 * A freshly initialized attribute structure contains the following
 * values:
 *
 * - lifetime: #KEYSTORE_PSA_KEY_LIFETIME_VOLATILE.
 * - key identifier: @c 0 (which is not a valid key identifier).
 * - type: @c 0 (meaning that the type is unspecified).
 * - key size: @c 0 (meaning that the size is unspecified).
 * - usage flags: @c 0 (which allows no usage except exporting a public
 * key).
 * - algorithm: @c 0 (which allows no cryptographic usage, but allows
 *   exporting).
 *
 * A typical sequence to create a key is as follows:
 * -# Create and initialize an attribute structure.
 * -# If the key is persistent, call KeyStore_PSA_setKeyId().
 *    Also call KeyStore_PSA_setKeyLifetime() to place the key in a non-default
 *    location.
 * -# Set the key policy with KeyStore_PSA_setKeyUsageFlags() and
 *    KeyStore_PSA_setKeyAlgorithm().
 * -# Set the key type with KeyStore_PSA_setKeyType().
 * -# When generating a random key with KeyStore_PSA_generateKey() or deriving a
 * key with KeyStore_PSA_key_derivation_output_key(), set the desired key size
 * with KeyStore_PSA_setKeyBits().
 * -# Call a key creation function: KeyStore_PSA_importKey(),
 * KeyStore_PSA_generateKey(), KeyStore_PSA_key_derivation_output_key(). This
 * function reads the attribute structure, creates a key with these
 * attributes, and outputs a handle to the newly created key.
 * -# The attribute structure is now no longer necessary.
 *    You may call KeyStore_PSA_resetKeyAttributes(), although this is optional
 *    with the workflow presented here because the attributes currently
 *    defined in this specification do not require any additional resources
 *    beyond the structure itself.
 *
 * A typical sequence to query a key's attributes is as follows:
 * -# Call KeyStore_PSA_getKeyAttributes().
 * -# Call `KeyStore_PSA_get_key_xxx` functions to retrieve the attribute(s)
 * that you are interested in.
 * -# Call KeyStore_PSA_resetKeyAttributes() to free any resources that may be
 *    used by the attribute structure.
 *
 * Once a key has been created, it is impossible to change its attributes.
 */
typedef psa_key_attributes_t KeyStore_PSA_KeyAttributes;

    #define KEYSTORE_PSA_KEY_ATTRIBUTES_INIT PSA_KEY_ATTRIBUTES_INIT

    /** Macro to assign and get keyID
     *
     * It depends on MBEDTLS_PSA_CRYPTO_KEY_FILE_ID_ENCODES_OWNER to assign keyID and
     * owner if multiple partition supported by mbedtls
     */
    #if defined(MBEDTLS_PSA_CRYPTO_KEY_FILE_ID_ENCODES_OWNER)
        #define GET_KEY_ID(keyID, ID) \
            keyID.key_id = ID;        \
            keyID.owner  = KEYSTORE_PSA_DEFAULT_OWNER;

        #define SET_KEY_ID(ID, keyID) ID = keyID.key_id
    #else
        #define GET_KEY_ID(keyID, ID) keyID = ID

        #define SET_KEY_ID(ID, keyID) ID = keyID
    #endif /* defined(MBEDTLS_PSA_CRYPTO_KEY_FILE_ID_ENCODES_OWNER) */

#else
/** Encoding of identifiers of persistent keys for client side.
 *
 * - Applications may freely choose key identifiers in the range
 *   #KEYSTORE_PSA_KEY_ID_USER_MIN to #KEYSTORE_PSA_KEY_ID_WITH_CERTIFICATE_USER_MAX or
 *   #KEYSTORE_PSA_KEY_ID_WITHOUT_CERTIFICATE_USER_MAX.
 * - Implementations may define additional key identifiers in the range
 *   #KEYSTORE_PSA_KEY_ID_VENDOR_MIN to #KEYSTORE_PSA_KEY_ID_VENDOR_MAX.
 * - 0 is reserved as an invalid key identifier.
 * - Key identifiers outside these ranges are reserved for future use.
 */
typedef psa_key_id_t KeyStore_PSA_KeyFileId;

    /** Macro to assign and get keyID */
    #define GET_KEY_ID(keyID, ID) keyID = ID
    #define SET_KEY_ID(ID, keyID) ID = keyID

/* This is the client view of the `key_attributes` structure. Only
 * fields which need to be set by the PSA crypto client are present.
 * The PSA crypto service will maintain a different version of the
 * data structure internally. */
typedef struct psa_client_key_attributes_s KeyStore_PSA_KeyAttributes;

    #define KEYSTORE_PSA_KEY_ATTRIBUTES_INIT PSA_CLIENT_KEY_ATTRIBUTES_INIT
#endif /* #if (TFM_ENABLED == 0) || defined(TFM_PSA_API) */
/**@}*/

/**@}*/

/**
 *  Starting address of Pre-provisioned keys.
 *
 * The Immutable platform Root of Trust stores the pre-provisioned key's programmed at
 * production. SKS implementation will read this address to obtain the
 * KeyStore_PSA_KeyFileId and other relevant meta data of all the pre-provisioned keys stored at this address
 */
#define KEYSTORE_PSA_PREPROVISIONED_AREA_ADDR 0x0000
/**
 * @brief Area size for pre-provisioned keys, 2KB - 256B (reserved for attestation data)
 */
#define KEYSTORE_PSA_PREPROVISIONED_AREA_SIZE (0x700) /* 1792 B */

/**
 * @brief Exportable pre-provisioned key prefix
 *
 * Prefix the plaintext key in pre-provisioned key that can be exported to the non-secure application with this flag
 */
#define KEYSTORE_PSA_PRE_PROVISIONED_KEY_EXPORT_FLAG        0xAAAA
#define KEYSTORE_PSA_PRE_PROVISIONED_KEY_EXPORT_FLAG_LENGTH (sizeof(KEYSTORE_PSA_PRE_PROVISIONED_KEY_EXPORT_FLAG))
#define KEYSTORE_PSA_PRE_PROVISIONED_KEY_NO_EXPORT_FLAG     0xA5A5

/**
 * @brief Macro to indicate empty pre-provisioned key memory
 */
#define KEYSTORE_PSA_PREPROVISIONED_KEYS_EMPTY 0xFFFF

/**
 * @brief Pre-provisioned key storage magic header.
 */
#define KEYSTORE_PSA_PRE_PROVISIONED_KEY_MAGIC_HEADER        "HUK\0KEY"
#define KEYSTORE_PSA_PRE_PROVISIONED_KEYS_END                0
#define KEYSTORE_PSA_PRE_PROVISIONED_KEY_MAGIC_HEADER_LENGTH (sizeof(KEYSTORE_PSA_PRE_PROVISIONED_KEY_MAGIC_HEADER))
#define KEYSTORE_PSA_PRE_PROVISIONED_KEYS_END_LENGTH         (sizeof(KEYSTORE_PSA_PRE_PROVISIONED_KEYS_END))
#define KEYSTORE_PSA_MAX_PREPROVISIONED_KEYS                 0x10

/**
 * @brief Pre-provisioned key Lifetime
 */
#define KEYSTORE_PSA_PRE_PROVISIONED_KEY_VALID_LIFETIME   0xAAAA
#define KEYSTORE_PSA_PRE_PROVISIONED_KEY_INVALID_LIFETIME 0x8888

/**
 * @brief Admissible key ID range for Pre-provisioned keys
 *
 * 0x21 - 0x40 is reserved to store pre-provisioned keys. Correspondingly, 0x10000021 - 0x10000040 is reserved for any
 * associated certificates.
 */
#define KEYSTORE_PSA_PRE_PROVISIONED_KEY_ID_MAX 0x40
#define KEYSTORE_PSA_PRE_PROVISIONED_KEY_ID_MIN 0x21
#define KEYSTORE_PSA_PRE_PROVISIONED_CERTIFICATE_ID_MAX \
    (KEYSTORE_PSA_PRE_PROVISIONED_KEY_ID_MAX | KEYSTORE_PSA_KEY_ID_CERTIFICATE_BIT)
#define KEYSTORE_PSA_PRE_PROVISIONED_CERTIFICATE_ID_MIN \
    (KEYSTORE_PSA_PRE_PROVISIONED_KEY_ID_MIN | KEYSTORE_PSA_KEY_ID_CERTIFICATE_BIT)

/**
 * @brief Reserved pre-provisioned key IDs
 *
 * Some known pre-provisioned key used by attestation service have reserved key IDs in the admissible range for
 * pre-provisioned keys
 */
#define KEYSTORE_PSA_PRE_PROVISIONED_ATTESTATION_PUB_KEY_ID 0x40
#define KEYSTORE_PSA_PRE_PROVISIONED_ATTESTATION_PRI_KEY_ID 0X3F
#define KEYSTORE_PSA_PRE_PROVISIONED_ATTESTATION_CERTIFICATE_ID \
    (KEYSTORE_PSA_PRE_PROVISIONED_ATTESTATION_PUB_KEY_ID | KEYSTORE_PSA_KEY_ID_CERTIFICATE_BIT)

/**
 * @brief Structure for storing pre-provisioned key's meta data
 *
 * - header      : magic header to indicate the start of the pre-provisioned key
 * - id          : key id (and its owner when applicable)
 * - export      : Flag that needs to be set with correct string to allow exporting to non-secure application
 * - alg         : Cryptographic algorithm permitted using this key
 * - usage       : Usage for the key as described the KeyStore API
 * - keyLength   : Length of the pre-provisioned key
 */
typedef struct
{
    uint8_t header[KEYSTORE_PSA_PRE_PROVISIONED_KEY_MAGIC_HEADER_LENGTH];
    uint32_t lifetime;
    KeyStore_PSA_KeyFileId id;
    uint32_t export;
    KeyStore_PSA_Algorithm alg;
    KeyStore_PSA_KeyUsage usage;
    uint32_t keyLength;
} KeyStore_PreProvisionedKeyMetaData;

/**
 *  @brief Size of the meta data associated with the pre-provisoned key
 */
#define KEYSTORE_PRE_PROVISIONED_KEY_METADATA_SIZE sizeof(KeyStore_PreProvisionedKeyMetaData)

/**
 * @brief Structure for storing pre-provisioned keys and its meta data
 *
 * - meta        : Structure to meta data associated with each pre-provisioned key
 * - KeyMaterial : A pointer to the plaintext key material
 * - fletcher    : 32-bit Fletcher checksum over the pre-provisioned key and its meta data
 */
typedef struct
{
    KeyStore_PreProvisionedKeyMetaData meta;
    uint8_t *keyMaterial;
    uint32_t fletcher;
} KeyStore_preProvisionedKeyStorageFormat;

/**
 * @brief Initialize a KeyStore_preProvisionedKeyStorageFormat struct variable
 */
#define KEYSTORE_PSA_PREPROVISIONED_KEY_FORMAT_INIT                                                                 \
    {                                                                                                               \
        KEYSTORE_PSA_PRE_PROVISIONED_KEY_MAGIC_HEADER, 0, KEYSTORE_PSA_PRE_PROVISIONED_KEY_EXPORT_FLAG, 0, 0, 0, 0, \
            KEYSTORE_PSA_PRE_PROVISIONED_KEYS_END, 0                                                                \
    }

/**
 * @brief Structure for storing IDs of pre-provisioned keys
 *
 * Also stores minimal meta data required to import pre-provisioned keys from persistent memory
 *
 * - addr            : Starting address of the plaintext key material in pre-provisioned key memory
 * - id              : Key ID (and its owner when applicable)
 * - alg             : Algorithm the key will be used for
 * - usage           : Key usage
 * - export          : Flag to indicate the exportability of of the pre-provisioned key
 * - keyLength       : Size of the pre-provisioned key
 */
typedef struct
{
    uint8_t *addr;
    KeyStore_PSA_KeyFileId id;
    KeyStore_PSA_Algorithm alg;
    KeyStore_PSA_KeyUsage usage;
    uint32_t export;
    uint32_t keyLength;
} KeyStore_PSA_preProvisionedKeyIDs;

/**
 * @brief Declare a key as persistent and set its key identifier.
 *
 * If the attribute structure currently declares the key as volatile (which
 * is the default content of an attribute structure), this function sets
 * the lifetime attribute to #KEYSTORE_PSA_KEY_LIFETIME_PERSISTENT.
 *
 * This function does not access storage, it merely stores the given
 * value in the structure.
 * The persistent key will be written to storage when the attribute
 * structure is passed to a key creation function such as
 * KeyStore_PSA_import_key(), KeyStore_PSA_generate_key(),
 * KeyStore_PSA_key_derivation_output_key() or KeyStore_PSA_copy_key().
 *
 * This function may be declared as `static` (i.e. without external
 * linkage). This function may be provided as a function-like macro,
 * but in this case it must evaluate each of its arguments exactly once.
 *
 * @param [out] attributes  The attribute structure to write to.
 * @param  key              The persistent identifier for the key.
 */
void KeyStore_PSA_setKeyId(KeyStore_PSA_KeyAttributes *attributes, KeyStore_PSA_KeyFileId key);

/**
 * @brief Set the location of a persistent key.
 *
 * To make a key persistent, you must give it a persistent key identifier
 * with KeyStore_PSA_setKeyId(). By default, a key that has a persistent
 * identifier is stored in the default storage area identifier by
 * #KEYSTORE_PSA_KEY_LIFETIME_PERSISTENT. Call this function to choose a storage
 * area, or to explicitly declare the key as volatile.
 *
 * This function does not access storage, it merely stores the given
 * value in the structure.
 * The persistent key will be written to storage when the attribute
 * structure is passed to a key creation function such as
 * KeyStore_PSA_import_key(), KeyStore_PSA_generate_key(),
 * KeyStore_PSA_key_derivation_output_key() or KeyStore_PSA_copy_key().
 *
 * This function may be declared as `static` (i.e. without external
 * linkage). This function may be provided as a function-like macro,
 * but in this case it must evaluate each of its arguments exactly once.
 *
 * @param [out] attributes       The attribute structure to write to.
 * @param  lifetime              The lifetime for the key.
 *                              If this is #KEYSTORE_PSA_KEY_LIFETIME_VOLATILE, the
 *                              key will be volatile, and the key identifier
 *                              attribute is reset to 0.
 */
void KeyStore_PSA_setKeyLifetime(KeyStore_PSA_KeyAttributes *attributes, KeyStore_PSA_KeyLifetime lifetime);

/**
 * @brief Retrieve the key identifier from key attributes.
 *
 * This function may be declared as `static` (i.e. without external
 * linkage). This function may be provided as a function-like macro,
 * but in this case it must evaluate its argument exactly once.
 *
 * @param [in] attributes        The key attribute structure to query.
 *
 * \return The persistent identifier stored in the attribute structure.
 *         This value is unspecified if the attribute structure declares
 *         the key as volatile.
 */
KeyStore_PSA_KeyFileId KeyStore_PSA_getKeyId(KeyStore_PSA_KeyAttributes *attributes);

/**
 * @brief Retrieve the lifetime from key attributes.
 *
 * This function may be declared as `static` (i.e. without external
 * linkage). This function may be provided as a function-like macro,
 * but in this case it must evaluate its argument exactly once.
 *
 * @param [in] attributes        The key attribute structure to query.
 *
 * \return The lifetime value stored in the attribute structure.
 */
KeyStore_PSA_KeyLifetime KeyStore_PSA_getKeyLifetime(KeyStore_PSA_KeyAttributes *attributes);

/**
 * @brief Declare usage flags for a key.
 *
 * Usage flags are part of a key's usage policy. They encode what
 * kind of operations are permitted on the key. For more details,
 * refer to the documentation of the type #KeyStore_PSA_KeyUsage.
 *
 * This function overwrites any usage flags
 * previously set in @p attributes.
 *
 * This function may be declared as `static` (i.e. without external
 * linkage). This function may be provided as a function-like macro,
 * but in this case it must evaluate each of its arguments exactly once.
 *
 * @param [out] attributes       The attribute structure to write to.
 * @param  usage_flags           The usage flags to write.
 */
void KeyStore_PSA_setKeyUsageFlags(KeyStore_PSA_KeyAttributes *attributes, KeyStore_PSA_KeyUsage usage_flags);

/**
 * @brief Retrieve the usage flags from key attributes.
 *
 * This function may be declared as `static` (i.e. without external
 * linkage). This function may be provided as a function-like macro,
 * but in this case it must evaluate its argument exactly once.
 *
 * @param [in] attributes        The key attribute structure to query.
 *
 * \return The usage flags stored in the attribute structure.
 */
KeyStore_PSA_KeyUsage KeyStore_PSA_getKeyUsageFlags(KeyStore_PSA_KeyAttributes *attributes);

/**
 * @brief Declare the permitted algorithm policy for a key.
 *
 * The permitted algorithm policy of a key encodes which algorithm or
 * algorithms are permitted to be used with this key. The following
 * algorithm policies are supported:
 * - 0 does not allow any cryptographic operation with the key. The key
 *   may be used for non-cryptographic actions such as exporting (if
 *   permitted by the usage flags).
 * - An algorithm value permits this particular algorithm.
 *
 * This function overwrites any algorithm policy
 * previously set in @p attributes.
 *
 * This function may be declared as `static` (i.e. without external
 * linkage). This function may be provided as a function-like macro,
 * but in this case it must evaluate each of its arguments exactly once.
 *
 * @param [out] attributes       The attribute structure to write to.
 * @param  alg                   The permitted algorithm policy to write.
 */
void KeyStore_PSA_setKeyAlgorithm(KeyStore_PSA_KeyAttributes *attributes, KeyStore_PSA_Algorithm alg);

/**
 * @brief Retrieve the algorithm policy from key attributes.
 *
 * This function may be declared as `static` (i.e. without external
 * linkage). This function may be provided as a function-like macro,
 * but in this case it must evaluate its argument exactly once.
 *
 * @param [in] attributes        The key attribute structure to query.
 *
 * \return The algorithm stored in the attribute structure.
 */
KeyStore_PSA_Algorithm KeyStore_PSA_getKeyAlgorithm(KeyStore_PSA_KeyAttributes *attributes);

/**
 * @brief Declare the type of a key.
 *
 * This function overwrites any key type
 * previously set in @p attributes.
 *
 * This function may be declared as `static` (i.e. without external
 * linkage). This function may be provided as a function-like macro,
 * but in this case it must evaluate each of its arguments exactly once.
 *
 * @param [out] attributes       The attribute structure to write to.
 * @param  type                  The key type to write.
 *                              If this is 0, the key type in @p attributes
 *                              becomes unspecified.
 */
void KeyStore_PSA_setKeyType(KeyStore_PSA_KeyAttributes *attributes, KeyStore_PSA_KeyType type);

/**
 * @brief Declare the size of a key.
 *
 * This function overwrites any key size previously set in @p attributes.
 *
 * This function may be declared as `static` (i.e. without external
 * linkage). This function may be provided as a function-like macro,
 * but in this case it must evaluate each of its arguments exactly once.
 *
 * @param [out] attributes       The attribute structure to write to.
 * @param  bits                  The key size in bits.
 *                              If this is 0, the key size in @p attributes
 *                              becomes unspecified. Keys of size 0 are
 *                              not supported.
 */
void KeyStore_PSA_setKeyBits(KeyStore_PSA_KeyAttributes *attributes, size_t bits);

/**
 * @brief Retrieve the key type from key attributes.
 *
 * This function may be declared as `static` (i.e. without external
 * linkage). This function may be provided as a function-like macro,
 * but in this case it must evaluate its argument exactly once.
 *
 * @param [in] attributes        The key attribute structure to query.
 *
 * \return The key type stored in the attribute structure.
 */
KeyStore_PSA_KeyType KeyStore_PSA_getKeyType(KeyStore_PSA_KeyAttributes *attributes);

/** @brief Retrieve the key size from key attributes.
 *
 * This function may be declared as `static` (i.e. without external
 * linkage). This function may be provided as a function-like macro,
 * but in this case it must evaluate its argument exactly once.
 *
 * @param [in] attributes        The key attribute structure to query.
 *
 * \return The key size stored in the attribute structure, in bits.
 */
size_t KeyStore_PSA_getKeyBits(KeyStore_PSA_KeyAttributes *attributes);

/** @brief Reset a key attribute structure to a freshly initialized state.
 *
 * You must initialize the attribute structure as described in the
 * documentation of the type #KeyStore_PSA_KeyAttributes before calling this
 * function. Once the structure has been initialized, you may call this
 * function at any time.
 *
 * This function frees any auxiliary resources that the structure
 * may contain.
 *
 * @param [in,out] attributes    The attribute structure to reset.
 */
void KeyStore_PSA_resetKeyAttributes(KeyStore_PSA_KeyAttributes *attributes);

/**
 * @brief Export a certificate in binary format.
 *
 * The output of this function can be passed to KeyStore_PSA_importCertificate()
 * to create an equivalent object.
 *
 * @param [in] key           Key file ID of the key associated with the
 * certificate to export.
 * @param [out] data         Buffer where the certificate data is to be written.
 * @param [in] dataSize     Size of the @p data buffer in bytes.
 * @param [out] dataLength  On success, the number
 * of bytes that make up the certificate data.
 *
 * @retval #KEYSTORE_PSA_STATUS_SUCCESS
 * @retval KEYSTORE_PSA_STATUS_RESOURCE_UNAVAILABLE
 * @retval #KEYSTORE_PSA_STATUS_INVALID_KEY_ID
 *         The key identifier does not exist.
 * @retval #KEYSTORE_PSA_STATUS_NOT_SUPPORTED
 * @retval #KEYSTORE_PSA_STATUS_BUFFER_TOO_SMALL
 *         The size of the @p data buffer is too small.
 * @retval #KEYSTORE_PSA_STATUS_STORAGE_FAILURE
 * @retval #KEYSTORE_PSA_STATUS_INSUFFICIENT_MEMORY
 * @retval #KEYSTORE_PSA_STATUS_BAD_STATE
 *         The library has not been previously initialized by
 * KeyStore_PSA_init(). It is implementation-dependent whether a failure to
 * initialize results in this error code.
 */
int_fast16_t KeyStore_PSA_exportCertificate(KeyStore_PSA_KeyFileId key,
                                            uint8_t *data,
                                            size_t dataSize,
                                            size_t *dataLength);

/**
 * @brief Export a public key or the public part of a key pair in binary
 * format.
 *
 * The output of this function can be passed to KeyStore_PSA_importKey() to
 * create an object that is equivalent to the public key.
 *
 * This specification supports a single format for each key type.
 * Implementations may support other formats as long as the standard
 * format is supported. Implementations that support other formats
 * should ensure that the formats are clearly unambiguous so as to
 * minimize the risk that an invalid input is accidentally interpreted
 * according to a different format.
 *
 * - For elliptic curve public keys, the format for:
 *   - Montgomery curves (curve types `PSA_ECC_CURVE_CURVEXXX`), is
 *      - `x_P` as a `ceiling(m/8)`-byte string, little-endian;
 *   - Weierstrass curves (curve types `PSA_ECC_CURVE_SECTXXX`,
 * `PSA_ECC_CURVE_SECPXXX` and `PSA_ECC_CURVE_BRAINPOOL_PXXX`), is the
 * uncompressed representation defined by SEC1 &sect;2.3.3 as the content of
 * an ECPoint. Let `m` be the bit size associated with the curve, i.e. the
 * bit size of `q` for a curve over `F_q`. The representation consists of:
 *      - The byte 0x04;
 *      - `x_P` as a `ceiling(m/8)`-byte string, big-endian;
 *      - `y_P` as a `ceiling(m/8)`-byte string, big-endian.
 * - For Diffie-Hellman key exchange public keys,
 *   the format is the representation of the public key `y = g^x mod p` as a
 *   big-endian byte string. The length of the byte string is the length of
 * the base prime `p` in bytes.
 *
 * Exporting a public key object or the public part of a key pair is
 * always permitted, regardless of the key's usage flags.
 *
 * @param [in] key           Key file ID of the key to export.
 * @param [out] data         Buffer where the key data is to be written.
 * @param [in] dataSize     Size of the @p data buffer in bytes.
 * @param [out] dataLength  On success, the number of bytes
 *                          that make up the key data.
 *
 * @retval #KEYSTORE_PSA_STATUS_SUCCESS
 * @retval KEYSTORE_PSA_STATUS_RESOURCE_UNAVAILABLE
 * @retval #KEYSTORE_PSA_STATUS_INVALID_KEY_ID
 * @retval #KEYSTORE_PSA_STATUS_INVALID_ARGUMENT
 *         The key is neither a public key nor a key pair.
 * @retval #KEYSTORE_PSA_STATUS_NOT_SUPPORTED
 * @retval #KEYSTORE_PSA_STATUS_BUFFER_TOO_SMALL
 *         The size of the @p data buffer is too small.
 * @retval #KEYSTORE_PSA_STATUS_COMMUNICATION_FAILURE
 * @retval #KEYSTORE_PSA_STATUS_HARDWARE_FAILURE
 * @retval #KEYSTORE_PSA_STATUS_CORRUPTION_DETECTED
 * @retval #KEYSTORE_PSA_STATUS_STORAGE_FAILURE
 * @retval #KEYSTORE_PSA_STATUS_INSUFFICIENT_MEMORY
 * @retval #KEYSTORE_PSA_STATUS_BAD_STATE
 *         The library has not been previously initialized by
 * KeyStore_PSA_init(). It is implementation-dependent whether a failure to
 * initialize results in this error code.
 */
int_fast16_t KeyStore_PSA_exportPublicKey(KeyStore_PSA_KeyFileId key,
                                          uint8_t *data,
                                          size_t dataSize,
                                          size_t *dataLength);

/**
 * @brief Export a key in binary format.
 *
 * The key must designated as exportable. The output of this function can be
 * passed to KeyStore_PSA_importKey() to create an equivalent object.
 *
 * If the implementation of KeyStore_PSA_importKey() supports other formats
 * beyond the format specified here, the output from KeyStore_PSA_exportKey()
 * must use the representation specified here, not the original
 * representation.
 *
 * For standard key types, the output format is as follows:
 *
 * - For symmetric keys (including MAC keys), the format is the
 *   raw bytes of the key.
 *
 * - For elliptic curve key pairs, the format is
 *   a representation of the private value as a `ceiling(m/8)`-byte string
 *   where `m` is the bit size associated with the curve, i.e. the bit size
 *   of the order of the curve's coordinate field. This byte string is
 *   in little-endian order for Montgomery curves (curve types
 *   `PSA_ECC_CURVE_CURVEXXX`), and in big-endian order for Weierstrass
 *   curves (curve types `PSA_ECC_CURVE_SECTXXX`, `PSA_ECC_CURVE_SECPXXX`
 *   and `PSA_ECC_CURVE_BRAINPOOL_PXXX`).
 *   This is the content of the `privateKey` field of the `ECPrivateKey`
 *   format defined by RFC 5915.
 * - For Diffie-Hellman key exchange key pairs, the
 *   format is the representation of the private key `x` as a big-endian
 * byte string. The length of the byte string is the private key size in
 * bytes (leading zeroes are not stripped).
 * - For public keys, the format is the same as for KeyStore_PSA_exportPublicKey().
 *
 * The policy on the key must have the usage flag #KEYSTORE_PSA_KEY_USAGE_EXPORT
 * set.
 *
 * @param [in] key           Key file ID of the key to export.
 * @param [out] data         Buffer where the key data is to be written.
 * @param [in] dataSize     Size of the @p data buffer in bytes.
 * @param [out] dataLength  On success, the number of bytes
 *                          that make up the key data.
 *
 * @retval #KEYSTORE_PSA_STATUS_SUCCESS
 * @retval KEYSTORE_PSA_STATUS_RESOURCE_UNAVAILABLE
 * @retval #KEYSTORE_PSA_STATUS_INVALID_KEY_ID
 *         The key identifier does not exist.
 * @retval #KEYSTORE_PSA_STATUS_NOT_PERMITTED
 *         The key does not have the #KEYSTORE_PSA_KEY_USAGE_EXPORT flag.
 * @retval #KEYSTORE_PSA_STATUS_NOT_SUPPORTED
 * @retval #KEYSTORE_PSA_STATUS_BUFFER_TOO_SMALL
 *         The size of the @p data buffer is too small.
 * @retval #KEYSTORE_PSA_STATUS_STORAGE_FAILURE
 * @retval #KEYSTORE_PSA_STATUS_INSUFFICIENT_MEMORY
 * @retval #KEYSTORE_PSA_STATUS_BAD_STATE
 *         The library has not been previously initialized by
 * KeyStore_PSA_init(). It is implementation-dependent whether a failure to
 * initialize results in this error code.
 */
int_fast16_t KeyStore_PSA_exportKey(KeyStore_PSA_KeyFileId key, uint8_t *data, size_t dataSize, size_t *dataLength);

/**
 * @brief Import a certificate in binary format.
 *
 * This function supports the output from KeyStore_PSA_exportCertificate().
 *
 * Multiple certificates cannot be imported for the same key ID. If an
 * additional certificate needs to be associated with a particular key pair
 * or public key, that key data must be imported again with a different key
 * ID. Additionally, the key store does not interpret the contents of the
 * certificate and cannot verify that the key embedded within the
 * certificate matches the associated key.
 *
 * Implementations must reject an attempt to import a certificate of size 0.
 *
 * @param [in] attributes    The attributes for the new certificate.
 *                          The certificate size is always determined from
 * the @p data buffer. If the certificate size in @p attributes is nonzero,
 *                          it must be equal to the size from @p data.
 * @param [out] key    On success, the key file ID of the previously imported
 * key associated with the certificate. @c 0 on failure.
 * @param [in] data Buffer containing the certificate data in binary format.
 * @param [in] dataLength Size of the @p data buffer in bytes.
 *
 * @retval #KEYSTORE_PSA_STATUS_SUCCESS
 *         Success.
 *         If the certificate is persistent, the certificate material and
 * the certificate's metadata have been saved to persistent storage.
 * @retval KEYSTORE_PSA_STATUS_RESOURCE_UNAVAILABLE
 * @retval #KEYSTORE_PSA_STATUS_ALREADY_EXISTS This is an attempt to create a
 * certificate, and there is already a certificate with the given
 * identifier.
 * @retval #KEYSTORE_PSA_STATUS_NOT_SUPPORTED The certificate type
 * or certificate size is not supported, either by the implementation in
 * general or in this particular persistent location.
 * @retval #KEYSTORE_PSA_STATUS_INVALID_ARGUMENT The certificate attributes, as a whole,
 * are invalid.
 * @retval #KEYSTORE_PSA_STATUS_INVALID_ARGUMENT The certificate
 * data is not correctly formatted.
 * @retval #KEYSTORE_PSA_STATUS_INVALID_ARGUMENT The size in @p attributes is nonzero
 * and does not match the size of the certificate data.
 * @retval #KEYSTORE_PSA_STATUS_INSUFFICIENT_MEMORY
 * @retval #KEYSTORE_PSA_STATUS_INSUFFICIENT_STORAGE
 * @retval #KEYSTORE_PSA_STATUS_COMMUNICATION_FAILURE
 * @retval #KEYSTORE_PSA_STATUS_STORAGE_FAILURE @retval
 * #KEYSTORE_PSA_STATUS_HARDWARE_FAILURE @retval
 * #KEYSTORE_PSA_STATUS_CORRUPTION_DETECTED @retval #KEYSTORE_PSA_STATUS_BAD_STATE
 *         The library has not been previously initialized by
 * KeyStore_PSA_init(). It is implementation-dependent whether a failure to
 * initialize results in this error code.
 */
int_fast16_t KeyStore_PSA_importCertificate(KeyStore_PSA_KeyAttributes *attributes,
                                            KeyStore_PSA_KeyFileId *key,
                                            uint8_t *data,
                                            size_t dataLength);

/**
 * @brief Import a key in binary format.
 *
 * This function supports any output from KeyStore_PSA_exportKey(). Refer to the
 * documentation of KeyStore_PSA_exportPublicKey() for the format of public keys
 * and to the documentation of KeyStore_PSA_exportKey() for the format for
 * other key types.
 *
 * The dataLength determines the key size. The attributes may optionally
 * specify a key size; in this case it must match the dataLength. A key
 * size of 0 in @p attributes indicates that the key size is solely
 * determined by the key data.
 *
 * Implementations must reject an attempt to import a key of size 0.
 *
 * This specification supports a single format for each key type.
 * Implementations may support other formats as long as the standard
 * format is supported. Implementations that support other formats
 * should ensure that the formats are clearly unambiguous so as to
 * minimize the risk that an invalid input is accidentally interpreted
 * according to a different format.
 *
 * @param [in] attributes    The attributes for the new key.
 *                          The key size is always determined from the
 *                          @p data buffer.
 *                          If the key size in @p attributes is nonzero,
 *                          it must be equal to the size from @p data.
 * @param [in] data    Buffer containing the key data. The content of this
 *                    buffer is interpreted according to the type declared
 *                    in @p attributes.
 *                    All implementations must support at least the format
 *                    described in the documentation
 *                    of KeyStore_PSA_exportKey() or KeyStore_PSA_exportPublicKey()
 * for the chosen type. Implementations may allow other formats, but should
 * be conservative: implementations should err on the side of rejecting
 * content if it may be erroneous (e.g. wrong type or truncated data).
 * @param [in] dataLength Size of the @p data buffer in bytes.
 * @param [out] key    On success, the key file ID of the newly created key.
 *                    @c 0 on failure.
 *
 * @retval #KEYSTORE_PSA_STATUS_SUCCESS
 *         Success.
 *         If the key is persistent, the key material and the key's metadata
 *         have been saved to persistent storage.
 * @retval KEYSTORE_PSA_STATUS_RESOURCE_UNAVAILABLE
 * @retval #KEYSTORE_PSA_STATUS_ALREADY_EXISTS
 *         This is an attempt to create a key, and there is
 *         already a key with the given key file identifier.
 * @retval #KEYSTORE_PSA_STATUS_NOT_SUPPORTED
 *         The key type or key size is not supported, either by the
 *         implementation in general or in this particular persistent
 * location. @retval #KEYSTORE_PSA_STATUS_INVALID_ARGUMENT The key attributes,
 * as a whole, are invalid. @retval #KEYSTORE_PSA_STATUS_INVALID_ARGUMENT The
 * key data is not correctly formatted. @retval
 * #KEYSTORE_PSA_STATUS_INVALID_ARGUMENT The size in @p attributes is nonzero
 * and does not match the size of the key data. @retval
 * #KEYSTORE_PSA_STATUS_INSUFFICIENT_MEMORY @retval
 * #KEYSTORE_PSA_STATUS_INSUFFICIENT_STORAGE @retval
 * #KEYSTORE_PSA_STATUS_COMMUNICATION_FAILURE @retval
 * #KEYSTORE_PSA_STATUS_STORAGE_FAILURE @retval
 * #KEYSTORE_PSA_STATUS_HARDWARE_FAILURE @retval
 * #KEYSTORE_PSA_STATUS_CORRUPTION_DETECTED @retval #KEYSTORE_PSA_STATUS_BAD_STATE
 *         The library has not been previously initialized by
 * KeyStore_PSA_init(). It is implementation-dependent whether a failure to
 * initialize results in this error code.
 */
int_fast16_t KeyStore_PSA_importKey(KeyStore_PSA_KeyAttributes *attributes,
                                    uint8_t *data,
                                    size_t dataLength,
                                    KeyStore_PSA_KeyFileId *key);

/** @brief Retrieve the attributes of a key.
 *
 * This function first resets the attribute structure as with
 * KeyStore_PSA_resetKeyAttributes(). It then copies the attributes of
 * the given key into the given attribute structure.
 *
 * @note This function may allocate memory or other resources.
 *       Once you have called this function on an attribute structure,
 *       you must call KeyStore_PSA_resetKeyAttributes() to free these
 * resources.
 *
 * @param [in] key               Identifier of the key to query.
 * @param [in,out] attributes    On success, the attributes of the key.
 *                              On failure, equivalent to a
 *                              freshly-initialized structure.
 *
 * @retval #KEYSTORE_PSA_STATUS_SUCCESS
 * @retval #KEYSTORE_PSA_STATUS_RESOURCE_UNAVAILABLE
 * @retval #KEYSTORE_PSA_STATUS_INVALID_KEY_ID
 * @retval #KEYSTORE_PSA_STATUS_INSUFFICIENT_MEMORY
 * @retval #KEYSTORE_PSA_STATUS_COMMUNICATION_FAILURE
 * @retval #KEYSTORE_PSA_STATUS_CORRUPTION_DETECTED
 * @retval #KEYSTORE_PSA_STATUS_STORAGE_FAILURE
 * @retval #KEYSTORE_PSA_STATUS_BAD_STATE
 *         The library has not been previously initialized by
 * KeyStore_PSA_crypto_init(). It is implementation-dependent whether a failure
 * to initialize results in this error code.
 */
int_fast16_t KeyStore_PSA_getKeyAttributes(KeyStore_PSA_KeyFileId key, KeyStore_PSA_KeyAttributes *attributes);

/**
 * @brief Remove non-essential copies of key material from memory.
 *
 * An implementation is permitted to make additional copies of key material
 * for keys that have been created with the cache policy, an implementation
 * is permitted to make additional copies of the key material that are not
 * in storage and not for the purpose of ongoing operations. This function
 * will remove these extra copies of the key material from memory.
 *
 * This function is not required to remove key material from memory in any
 * of the following situations:
 * - The key is currently in use in a cryptographic operation.
 * - The key is volatile
 *
 * @param [in] key    Key handle to close. .
 *                   If this is @c 0, do nothing and return @c
 * KEYSTORE_PSA_STATUS_SUCCESS.
 *
 * @retval #KEYSTORE_PSA_STATUS_SUCCESS
 *         @p Handle was valid and the key material that it
 *         referred to has been closed.
 *         Alternatively, @p Handle is @c 0.
 * @retval KEYSTORE_PSA_STATUS_RESOURCE_UNAVAILABLE
 * @retval #KEYSTORE_PSA_STATUS_INVALID_KEY_ID
 *         @p handle is not a valid handle nor @c 0.
 * @retval #KEYSTORE_PSA_STATUS_COMMUNICATION_FAILURE
 *         There was an failure in communication with the cryptoprocessor.
 *         The key material may still be present in the cryptoprocessor.
 * @retval #KEYSTORE_PSA_STATUS_STORAGE_FAILURE
 *         The storage is corrupted. Implementations shall make a best
 * effort to erase key material even in this stage, however applications
 *         should be aware that it may be impossible to guarantee that the
 *         key material is not recoverable in such cases.
 * @retval #KEYSTORE_PSA_STATUS_CORRUPTION_DETECTED
 *         An unexpected condition which is not a storage corruption or
 *         a communication failure occurred. The cryptoprocessor may have
 *         been compromised.
 * @retval #KEYSTORE_PSA_STATUS_BAD_STATE
 *         The library has not been previously initialized by
 * KeyStore_PSA_init(). It is implementation-dependent whether a failure to
 * initialize results in this error code.
 */
int_fast16_t KeyStore_PSA_purgeKey(KeyStore_PSA_KeyFileId key);

/**
 * @brief Destroy a certificate associated with a key.
 *
 * This function destroys a certificate from both volatile
 * memory and, if applicable, non-volatile storage.
 *
 * This function also erases any metadata such as policies and frees
 * resources associated with the certificate.
 *
 * @param [in] key    Key file ID of the key associated with certificate to
 * erase. If key ID portion is @c 0, do nothing and return @c
 * KEYSTORE_PSA_STATUS_SUCCESS.
 *
 * @retval #KEYSTORE_PSA_STATUS_SUCCESS
 *         @p ID was a valid ID and the certificate material that it
 *         referred to has been erased.
 *         Alternatively, @p ID is @c 0.
 * @retval KEYSTORE_PSA_STATUS_RESOURCE_UNAVAILABLE
 * @retval #KEYSTORE_PSA_STATUS_NOT_PERMITTED
 *         The certificate cannot be erased because it is
 *         read-only, either due to a policy or due to physical
 * restrictions.
 * @retval #KEYSTORE_PSA_STATUS_INVALID_KEY_ID @p ID is not a validI D.
 * @retval #KEYSTORE_PSA_STATUS_COMMUNICATION_FAILURE There was an failure
 * in communication with the cryptoprocessor. The certificate material may
 * still be present in the cryptoprocessor.
 * @retval #KEYSTORE_PSA_STATUS_STORAGE_FAILURE The storage is corrupted.
 * Implementations shall make a best effort to erase certificate material
 * even in this stage, however applications should be aware that it may be
 * impossible to guarantee that the certificate material is not recoverable
 * in such cases.
 * @retval #KEYSTORE_PSA_STATUS_CORRUPTION_DETECTED An unexpected
 * condition which is not a storage corruption or a communication failure
 * occurred. The cryptoprocessor may have been compromised.
 * @retval #KEYSTORE_PSA_STATUS_BAD_STATE The library has not been previously
 * initialized by KeyStore_PSA_init(). It is implementation-dependent whether a
 * failure to initialize results in this error code.
 */
int_fast16_t KeyStore_PSA_destroyCertificate(KeyStore_PSA_KeyFileId key);

/**
 * @brief Destroy a key.
 *
 * This function destroys a key from both volatile memory and, if
 * applicable, non-volatile storage. Implementations shall make a best
 * effort to ensure that that the key material cannot be recovered.
 *
 * If the key has an associated certificate, KeyStore_PSA_destroyCertificate()
 * must be called to destroy the certificate before destroying the key.
 *
 * This function also erases any metadata such as policies and frees
 * resources associated with the key.
 *
 * Destroying the key makes the ID invalid, and the key ID must not be used
 * again by the application.
 *
 * If a key is currently in use in a multipart operation, then destroying
 * the key will cause the multipart operation to fail.
 *
 * After a volatile key is destroyed, it is recommended that the
 * implementation does not immediately reuse the same key ID value for a
 * different key. This reduces the risk of an attack that is able to exploit
 * a key identifier reuse vulnerability within an application.
 *
 * @param [in] key    Key file ID of the key to erase.
 *                   If key ID portion is @c 0, do nothing and return @c
 * KEYSTORE_PSA_STATUS_SUCCESS.
 *
 * @retval #KEYSTORE_PSA_STATUS_SUCCESS
 *         @p ID was a valid ID and the key material that it
 *         referred to has been erased.
 *         Alternatively, @p ID is @c 0.
 * @retval KEYSTORE_PSA_STATUS_RESOURCE_UNAVAILABLE
 * @retval #KEYSTORE_PSA_STATUS_NOT_PERMITTED
 *         The key cannot be erased because it has an associated certificate
 * or is read-only, either due to a policy or due to physical restrictions.
 * @retval #KEYSTORE_PSA_STATUS_INVALID_KEY_ID
 *         @p ID is not a valid ID.
 * @retval #KEYSTORE_PSA_STATUS_COMMUNICATION_FAILURE
 *         There was an failure in communication with the cryptoprocessor.
 *         The key material may still be present in the cryptoprocessor.
 * @retval #KEYSTORE_PSA_STATUS_STORAGE_FAILURE
 *         The storage is corrupted. Implementations shall make a best
 * effort to erase key material even in this stage, however applications
 *         should be aware that it may be impossible to guarantee that the
 *         key material is not recoverable in such cases.
 * @retval #KEYSTORE_PSA_STATUS_CORRUPTION_DETECTED
 *         An unexpected condition which is not a storage corruption or
 *         a communication failure occurred. The cryptoprocessor may have
 *         been compromised.
 * @retval #KEYSTORE_PSA_STATUS_BAD_STATE
 *         The library has not been previously initialized by
 * KeyStore_PSA_init(). It is implementation-dependent whether a failure to
 * initialize results in this error code.
 */
int_fast16_t KeyStore_PSA_destroyKey(KeyStore_PSA_KeyFileId key);

#ifdef __cplusplus
}
#endif

#endif /* ti_drivers_KeyStore_PSA__include */
