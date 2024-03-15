/*
 * Copyright (c) 2018-2023, Arm Limited. All rights reserved.
 * Copyright (c) 2023, Texas Instruments Incorporated. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */
/**
 * \file psa/crypto_extra.h
 *
 * \brief PSA cryptography module: vendor extensions
 *
 * \note This file may not be included directly. Applications must
 * include psa/crypto.h.
 *
 * This file is reserved for vendor-specific definitions.
 */

#ifndef PSA_CRYPTO_EXTRA_H
#define PSA_CRYPTO_EXTRA_H

#include "crypto_types.h"
#include "crypto_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

/** \addtogroup crypto_types
 * @{
 */

/** DSA public key.
 *
 * The import and export format is the
 * representation of the public key `y = g^x mod p` as a big-endian byte
 * string. The length of the byte string is the length of the base prime `p`
 * in bytes.
 */
#define PSA_KEY_TYPE_DSA_PUBLIC_KEY                 ((psa_key_type_t) 0x4002)

/** DSA key pair (private and public key).
 *
 * The import and export format is the
 * representation of the private key `x` as a big-endian byte string. The
 * length of the byte string is the private key size in bytes (leading zeroes
 * are not stripped).
 *
 * Deterministic DSA key derivation with psa_generate_derived_key follows
 * FIPS 186-4 &sect;B.1.2: interpret the byte string as integer
 * in big-endian order. Discard it if it is not in the range
 * [0, *N* - 2] where *N* is the boundary of the private key domain
 * (the prime *p* for Diffie-Hellman, the subprime *q* for DSA,
 * or the order of the curve's base point for ECC).
 * Add 1 to the resulting integer and use this as the private key *x*.
 *
 */
#define PSA_KEY_TYPE_DSA_KEY_PAIR                    ((psa_key_type_t) 0x7002)

/** Whether a key type is a DSA key (pair or public-only). */
#define PSA_KEY_TYPE_IS_DSA(type)                                       \
    (PSA_KEY_TYPE_PUBLIC_KEY_OF_KEY_PAIR(type) == PSA_KEY_TYPE_DSA_PUBLIC_KEY)

#define PSA_ALG_DSA_BASE                        ((psa_algorithm_t) 0x06000400)
/** DSA signature with hashing.
 *
 * This is the signature scheme defined by FIPS 186-4,
 * with a random per-message secret number (*k*).
 *
 * \param hash_alg      A hash algorithm (\c PSA_ALG_XXX value such that
 *                      #PSA_ALG_IS_HASH(\p hash_alg) is true).
 *                      This includes #PSA_ALG_ANY_HASH
 *                      when specifying the algorithm in a usage policy.
 *
 * \return              The corresponding DSA signature algorithm.
 * \return              Unspecified if \p hash_alg is not a supported
 *                      hash algorithm.
 */
#define PSA_ALG_DSA(hash_alg)                             \
    (PSA_ALG_DSA_BASE | ((hash_alg) & PSA_ALG_HASH_MASK))
#define PSA_ALG_DETERMINISTIC_DSA_BASE          ((psa_algorithm_t) 0x06000500)
#define PSA_ALG_DSA_DETERMINISTIC_FLAG PSA_ALG_ECDSA_DETERMINISTIC_FLAG
/** Deterministic DSA signature with hashing.
 *
 * This is the deterministic variant defined by RFC 6979 of
 * the signature scheme defined by FIPS 186-4.
 *
 * \param hash_alg      A hash algorithm (\c PSA_ALG_XXX value such that
 *                      #PSA_ALG_IS_HASH(\p hash_alg) is true).
 *                      This includes #PSA_ALG_ANY_HASH
 *                      when specifying the algorithm in a usage policy.
 *
 * \return              The corresponding DSA signature algorithm.
 * \return              Unspecified if \p hash_alg is not a supported
 *                      hash algorithm.
 */
#define PSA_ALG_DETERMINISTIC_DSA(hash_alg)                             \
    (PSA_ALG_DETERMINISTIC_DSA_BASE | ((hash_alg) & PSA_ALG_HASH_MASK))
#define PSA_ALG_IS_DSA(alg)                                             \
    (((alg) & ~PSA_ALG_HASH_MASK & ~PSA_ALG_DSA_DETERMINISTIC_FLAG) ==  \
     PSA_ALG_DSA_BASE)
#define PSA_ALG_DSA_IS_DETERMINISTIC(alg)               \
    (((alg) & PSA_ALG_DSA_DETERMINISTIC_FLAG) != 0)
#define PSA_ALG_IS_DETERMINISTIC_DSA(alg)                       \
    (PSA_ALG_IS_DSA(alg) && PSA_ALG_DSA_IS_DETERMINISTIC(alg))
#define PSA_ALG_IS_RANDOMIZED_DSA(alg)                          \
    (PSA_ALG_IS_DSA(alg) && !PSA_ALG_DSA_IS_DETERMINISTIC(alg))


/* We need to expand the sample definition of this macro from
 * the API definition. */
#undef PSA_ALG_IS_VENDOR_HASH_AND_SIGN
#define PSA_ALG_IS_VENDOR_HASH_AND_SIGN(alg)    \
    PSA_ALG_IS_DSA(alg)

/* TI-TFM: Added J-PAKE algorithm */
/** The Password-authenticated key exchange by juggling (J-PAKE) algorithm.
 *
 * This is J-PAKE as defined by RFC 8236, instantiated with the following
 * parameters:
 *
 * - The group can be either an elliptic curve or defined over a finite field.
 * - Schnorr NIZK proof as defined by RFC 8235 and using the same group as the
 *   J-PAKE algorithm.
 * - A cryptographic hash function.
 *
 * To select these parameters and set up the cipher suite, call these functions
 * in any order:
 *
 * \code
 * psa_pake_cs_set_algorithm(cipher_suite, PSA_ALG_JPAKE);
 * psa_pake_cs_set_primitive(cipher_suite,
 *                           PSA_PAKE_PRIMITIVE(type, family, bits));
 * psa_pake_cs_set_hash(cipher_suite, hash);
 * \endcode
 *
 * For more information on how to set a specific curve or field, refer to the
 * documentation of the individual \c PSA_PAKE_PRIMITIVE_TYPE_XXX constants.
 *
 * After initializing a J-PAKE operation, call
 *
 * \code
 * psa_pake_setup(operation, cipher_suite);
 * psa_pake_set_user(operation, ...);
 * psa_pake_set_peer(operation, ...);
 * psa_pake_set_password_key(operation, ...);
 * \endcode
 *
 * The password is provided as a key. This can be the password text itself,
 * in an agreed character encoding, or some value derived from the password
 * as required by a higher level protocol.
 *
 * (The implementation converts the key material to a number as described in
 * Section 2.3.8 of _SEC 1: Elliptic Curve Cryptography_
 * (https://www.secg.org/sec1-v2.pdf), before reducing it modulo \c q. Here
 * \c q is order of the group defined by the primitive set in the cipher suite.
 * The \c psa_pake_set_password_key() function returns an error if the result
 * of the reduction is 0.)
 *
 * The key exchange flow for J-PAKE is as follows:
 * -# To get the first round data that needs to be sent to the peer, call
 *    \code
 *    // Get g1
 *    psa_pake_output(operation, #PSA_PAKE_STEP_KEY_SHARE, ...);
 *    // Get the ZKP public key for x1
 *    psa_pake_output(operation, #PSA_PAKE_STEP_ZK_PUBLIC, ...);
 *    // Get the ZKP proof for x1
 *    psa_pake_output(operation, #PSA_PAKE_STEP_ZK_PROOF, ...);
 *    // Get g2
 *    psa_pake_output(operation, #PSA_PAKE_STEP_KEY_SHARE, ...);
 *    // Get the ZKP public key for x2
 *    psa_pake_output(operation, #PSA_PAKE_STEP_ZK_PUBLIC, ...);
 *    // Get the ZKP proof for x2
 *    psa_pake_output(operation, #PSA_PAKE_STEP_ZK_PROOF, ...);
 *    \endcode
 * -# To provide the first round data received from the peer to the operation,
 *    call
 *    \code
 *    // Set g3
 *    psa_pake_input(operation, #PSA_PAKE_STEP_KEY_SHARE, ...);
 *    // Set the ZKP public key for x3
 *    psa_pake_input(operation, #PSA_PAKE_STEP_ZK_PUBLIC, ...);
 *    // Set the ZKP proof for x3
 *    psa_pake_input(operation, #PSA_PAKE_STEP_ZK_PROOF, ...);
 *    // Set g4
 *    psa_pake_input(operation, #PSA_PAKE_STEP_KEY_SHARE, ...);
 *    // Set the ZKP public key for x4
 *    psa_pake_input(operation, #PSA_PAKE_STEP_ZK_PUBLIC, ...);
 *    // Set the ZKP proof for x4
 *    psa_pake_input(operation, #PSA_PAKE_STEP_ZK_PROOF, ...);
 *    \endcode
 * -# To get the second round data that needs to be sent to the peer, call
 *    \code
 *    // Get A
 *    psa_pake_output(operation, #PSA_PAKE_STEP_KEY_SHARE, ...);
 *    // Get ZKP public key for x2*s
 *    psa_pake_output(operation, #PSA_PAKE_STEP_ZK_PUBLIC, ...);
 *    // Get ZKP proof for x2*s
 *    psa_pake_output(operation, #PSA_PAKE_STEP_ZK_PROOF, ...);
 *    \endcode
 * -# To provide the second round data received from the peer to the operation,
 *    call
 *    \code
 *    // Set B
 *    psa_pake_input(operation, #PSA_PAKE_STEP_KEY_SHARE, ...);
 *    // Set ZKP public key for x4*s
 *    psa_pake_input(operation, #PSA_PAKE_STEP_ZK_PUBLIC, ...);
 *    // Set ZKP proof for x4*s
 *    psa_pake_input(operation, #PSA_PAKE_STEP_ZK_PROOF, ...);
 *    \endcode
 * -# To access the shared secret call
 *    \code
 *    // Get Ka=Kb=K
 *    psa_pake_get_implicit_key()
 *    \endcode
 *
 * For more information consult the documentation of the individual
 * \c PSA_PAKE_STEP_XXX constants.
 *
 * At this point there is a cryptographic guarantee that only the authenticated
 * party who used the same password is able to compute the key. But there is no
 * guarantee that the peer is the party it claims to be and was able to do so.
 *
 * That is, the authentication is only implicit (the peer is not authenticated
 * at this point, and no action should be taken that assume that they are - like
 * for example accessing restricted files).
 *
 * To make the authentication explicit there are various methods, see Section 5
 * of RFC 8236 for two examples.
 *
 */
#define PSA_ALG_JPAKE                   ((psa_algorithm_t) 0x0a000100)

/**@}*/

#ifdef __cplusplus
}
#endif

#endif /* PSA_CRYPTO_EXTRA_H */
