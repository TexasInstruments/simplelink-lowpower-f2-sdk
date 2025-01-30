/*
 * Copyright (c) 2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef __CC3XX_DRBG_HASH_H__
#define __CC3XX_DRBG_HASH_H__

#include <stdint.h>
#include "cc3xx_error.h"

#include "cc3xx_hash.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief The value of seedlen is fixed by the spec to 440 bits. This
 *        defines is used to convert it to bytes
 */
#define CC3XX_DRBG_HASH_SEEDLEN (440/8)

/**
 * @brief Contains the state of the HASH_DRBG
 *
 */
struct cc3xx_drbg_hash_state_t {
    uint8_t value_v[CC3XX_DRBG_HASH_SEEDLEN + 1]; /* +1 for word alignment */
    uint8_t constant_c[CC3XX_DRBG_HASH_SEEDLEN + 1];
    uint32_t reseed_counter;
};

/**
 * @brief Instantiate the HASH_DRBG generator
 *
 * @param[out] state              Pointer to a \ref cc3xx_drbg_hash_state_t context
 * @param[in] entropy             Pointer to the collected entropy
 * @param[in] entropy_len         Size in bytes of the entropy
 * @param[in] nonce               Pointer to the nonce to be used
 * @param[in] nonce_len           Size in bytes of the nonce
 * @param[in] personalization     Pointer to the personalisation string to be used
 * @param[in] personalization_len Size in bytes of the personalisation string
 * @return cc3xx_err_t
 */
cc3xx_err_t cc3xx_drbg_hash_init(
    struct cc3xx_drbg_hash_state_t *state,
    const uint8_t *entropy, size_t entropy_len,
    const uint8_t *nonce, size_t nonce_len,
    const uint8_t *personalization, size_t personalization_len);

/**
 * @brief Generates random bits from the HASH_DRBG instance
 *
 * @param[in,out] state            Pointer to an instantiated HASH_DRBG generator
 * @param[in] len_bits             Size in bits to be generated. Must be byte aligned for simplicity
 * @param[out] returned_bits       Pointer holding the returned random bit string
 * @param[in] additional_input     Pointer to the additional input to be used
 * @param[in] additional_input_len Size in bytes of the additional input to be used
 * @return cc3xx_err_t
 */
cc3xx_err_t cc3xx_drbg_hash_generate(
    struct cc3xx_drbg_hash_state_t *state,
    size_t len_bits, uint8_t *returned_bits,
    const uint8_t *additional_input, size_t additional_input_len);

/**
 * @brief Reseeds the HASH_DRBG instance
 *
 * @param[in,out] state            Pointer to an instantiated HASH_DRBG generator to reseed
 * @param[in] entropy              Pointer to the additional entropy to use for reseeding
 * @param[in] entropy_len          Size in bytes of the additional entropy
 * @param[in] additional_input     Pointer to the additional input to use for reseeding
 * @param[in] additional_input_len Size in bytes of the additional input buffer
 * @return cc3xx_err_t
 */
cc3xx_err_t cc3xx_drbg_hash_reseed(
    struct cc3xx_drbg_hash_state_t *state,
    const uint8_t *entropy, size_t entropy_len,
    const uint8_t *additional_input, size_t additional_input_len);

/**
 * @brief Un-initializes the state structure associated to the HASH_DRBG
 *
 * @param[out] state Pointer to the state structure
 * @return cc3xx_err_t
 */
cc3xx_err_t cc3xx_drbg_hash_uninit(struct cc3xx_drbg_hash_state_t *state);

#ifdef __cplusplus
}
#endif

#endif /* __CC3XX_DRBG_HASH_H__ */
