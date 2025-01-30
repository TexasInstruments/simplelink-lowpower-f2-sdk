/*
 * Copyright (c) 2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */
#include <assert.h>
#include "cc3xx_drbg.h"
#include "cc3xx_config.h"

cc3xx_err_t cc3xx_drbg_init(
    cc3xx_drbg_id_t id,
    struct cc3xx_drbg_state_t *state,
    const uint8_t *entropy, size_t entropy_len,
    const uint8_t *nonce, size_t nonce_len,
    const uint8_t *personalization, size_t personalization_len)
{
    assert(id < CC3XX_DRBG_MAX);

    state->id = id;

    switch(id) {
#ifdef CC3XX_CONFIG_DRBG_CTR_ENABLE
    case CC3XX_DRBG_CTR:
        return cc3xx_drbg_ctr_init(
            &state->ctr,
            entropy, entropy_len,
            nonce, nonce_len,
            personalization, personalization_len);
#endif /* CC3XX_CONFIG_DRBG_CTR_ENABLE */
#ifdef CC3XX_CONFIG_DRBG_HMAC_ENABLE
    case CC3XX_DRBG_HMAC:
        return cc3xx_drbg_hmac_instantiate(
            &state->hmac,
            entropy, entropy_len,
            nonce, nonce_len,
            personalization, personalization_len);
#endif /* CC3XX_CONFIG_DRBG_HMAC_ENABLE */
#ifdef CC3XX_CONFIG_DRBG_HASH_ENABLE
    case CC3XX_DRBG_HASH:
        return cc3xx_drbg_hash_init(
            &state->hash,
            entropy, entropy_len,
            nonce, nonce_len,
            personalization, personalization_len);
#endif /* CC3XX_CONFIG_DRBG_HASH_ENABLE */
    default:
        return CC3XX_ERR_DRBG_INVALID_ID;
    }
}

cc3xx_err_t cc3xx_drbg_generate(
    struct cc3xx_drbg_state_t *state,
    size_t len_bits, uint8_t *returned_bits,
    const uint8_t *additional_input, size_t additional_input_len)
{
    switch(state->id) {
#ifdef CC3XX_CONFIG_DRBG_CTR_ENABLE
    case CC3XX_DRBG_CTR:
        return cc3xx_drbg_ctr_generate(
            &state->ctr, len_bits, returned_bits, additional_input, additional_input_len);
#endif /* CC3XX_CONFIG_DRBG_CTR_ENABLE */
#ifdef CC3XX_CONFIG_DRBG_HMAC_ENABLE
    case CC3XX_DRBG_HMAC:
        return cc3xx_drbg_hmac_generate(
            &state->hmac, len_bits, returned_bits, additional_input, additional_input_len);
#endif /* CC3XX_CONFIG_DRBG_HMAC_ENABLE */
#ifdef CC3XX_CONFIG_DRBG_HASH_ENABLE
    case CC3XX_DRBG_HASH:
        return cc3xx_drbg_hash_generate(
            &state->hash, len_bits, returned_bits, additional_input, additional_input_len);
#endif /* CC3XX_CONFIG_DRBG_HASH_ENABLE */
    default:
        return CC3XX_ERR_DRBG_INVALID_ID;
    }
}

cc3xx_err_t cc3xx_drbg_reseed(
    struct cc3xx_drbg_state_t *state,
    const uint8_t *entropy, size_t entropy_len,
    const uint8_t *additional_input, size_t additional_input_len)
{
    switch(state->id) {
#ifdef CC3XX_CONFIG_DRBG_CTR_ENABLE
    case CC3XX_DRBG_CTR:
        return cc3xx_drbg_ctr_reseed(
            &state->ctr, entropy, entropy_len, additional_input, additional_input_len);
#endif /* CC3XX_CONFIG_DRBG_CTR_ENABLE */
#ifdef CC3XX_CONFIG_DRBG_HMAC_ENABLE
    case CC3XX_DRBG_HMAC:
        return cc3xx_drbg_hmac_reseed(
            &state->hmac, entropy, entropy_len, additional_input, additional_input_len);
#endif /* CC3XX_CONFIG_DRBG_HMAC_ENABLE */
#ifdef CC3XX_CONFIG_DRBG_HASH_ENABLE
    case CC3XX_DRBG_HASH:
        return cc3xx_drbg_hash_reseed(
            &state->hash, entropy, entropy_len, additional_input, additional_input_len);
#endif /* CC3XX_CONFIG_DRBG_HASH_ENABLE */
    default:
        return CC3XX_ERR_DRBG_INVALID_ID;
    }
}

cc3xx_err_t cc3xx_drbg_uninit(struct cc3xx_drbg_state_t *state)
{
    switch(state->id) {
#ifdef CC3XX_CONFIG_DRBG_CTR_ENABLE
    case CC3XX_DRBG_CTR:
        return cc3xx_drbg_ctr_uninit(&state->ctr);
#endif /* CC3XX_CONFIG_DRBG_CTR_ENABLE */
#ifdef CC3XX_CONFIG_DRBG_HMAC_ENABLE
    case CC3XX_DRBG_HMAC:
        return cc3xx_drbg_hmac_uninit(&state->hmac);
#endif /* CC3XX_CONFIG_DRBG_HMAC_ENABLE */
#ifdef CC3XX_CONFIG_DRBG_HASH_ENABLE
    case CC3XX_DRBG_HASH:
        return cc3xx_drbg_hash_uninit(&state->hash);
#endif /* CC3XX_CONFIG_DRBG_HASH_ENABLE */
    default:
        return CC3XX_ERR_DRBG_INVALID_ID;
    }
}
