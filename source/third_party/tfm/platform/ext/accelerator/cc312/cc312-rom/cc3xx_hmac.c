/*
 * Copyright (c) 2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include "cc3xx_hash.h"
#include "cc3xx_hmac.h"

/* SHA-1, SHA-224 and SHA-256 have all the same block size */
#define SHA256_BLOCK_SIZE (64)

cc3xx_err_t cc3xx_hmac_set_key(
    struct cc3xx_hmac_state_t *state,
    const uint8_t *key,
    size_t key_size,
    cc3xx_hash_alg_t alg)
{
    const uint8_t ipad = 0x36;
    cc3xx_err_t err;
    size_t idx;
    /* In case the key is higher than B, it must be hashed first */
    uint32_t hash_key_output[SHA256_BLOCK_SIZE / sizeof(uint32_t)];
    const uint8_t *p_key = key;
    size_t key_length = key_size;

    err = cc3xx_hash_init(alg);
    if (err != CC3XX_ERR_SUCCESS) {
        return err;
    }

    if (key_size > CC3XX_HMAC_BLOCK_SIZE) {
        /* hash the key to L bytes */
        err = cc3xx_hash_update(key, key_size);
        if (err != CC3XX_ERR_SUCCESS) {
            goto out;
        }
        p_key = (const uint8_t *)hash_key_output;
        key_length = CC3XX_HASH_LENGTH(alg);
    }

    cc3xx_hash_finish(hash_key_output, sizeof(hash_key_output));

    /* K ^ ipad */
    for (idx = 0; idx < key_length; idx++) {
        state->key[idx] = p_key[idx] ^ ipad;
    }

    memset(&state->key[key_length], ipad, CC3XX_HMAC_BLOCK_SIZE - key_length);

    /* H(K ^ ipad) */
    err = cc3xx_hash_init(alg);
    if (err != CC3XX_ERR_SUCCESS) {
        goto out;
    }

    err = cc3xx_hash_update(state->key, CC3XX_HMAC_BLOCK_SIZE);

out:
    if (err == CC3XX_ERR_SUCCESS) {
        cc3xx_hash_get_state(&state->hash);
        state->alg = alg;
    }
    cc3xx_hash_uninit();
    return err;
}

cc3xx_err_t cc3xx_hmac_update(
    struct cc3xx_hmac_state_t *state,
    const uint8_t *data,
    size_t data_length)
{
    cc3xx_err_t err;

    cc3xx_hash_set_state(&state->hash);

    /* H(K ^ ipad | data)*/
    err = cc3xx_hash_update(data, data_length);

    if (err == CC3XX_ERR_SUCCESS) {
        cc3xx_hash_get_state(&state->hash);
    }
    cc3xx_hash_uninit();
    return err;
}

cc3xx_err_t cc3xx_hmac_finish(
    struct cc3xx_hmac_state_t *state,
    uint32_t *tag,
    size_t tag_size)
{
    uint32_t scratch[CC3XX_HASH_LENGTH(state->alg) / sizeof(uint32_t)];
    const uint8_t ixopad = 0x36 ^ 0x5c; /* ipad ^ opad */
    cc3xx_err_t err;
    size_t idx;

    assert(tag_size >= CC3XX_HASH_LENGTH(state->alg));

    cc3xx_hash_set_state(&state->hash);

    /* Produce H(K ^ ipad | data) */
    cc3xx_hash_finish(scratch, sizeof(scratch));

    /* K ^ opad */
    for (idx = 0; idx < CC3XX_HMAC_BLOCK_SIZE; idx++) {
        state->key[idx] ^= ixopad;
    }

    /* H( K ^ opad | H(K ^ ipad | data)) */
    err = cc3xx_hash_init(state->alg);
    if (err != CC3XX_ERR_SUCCESS) {
        goto out;
    }

    err = cc3xx_hash_update(state->key, CC3XX_HMAC_BLOCK_SIZE);
    if (err != CC3XX_ERR_SUCCESS) {
        goto out;
    }

    err = cc3xx_hash_update((const uint8_t *)scratch, sizeof(scratch));
    if (err != CC3XX_ERR_SUCCESS) {
        goto out;
    }

    cc3xx_hash_finish(tag, tag_size);

out:
    if (err == CC3XX_ERR_SUCCESS) {
        cc3xx_hash_get_state(&state->hash);
    }
    cc3xx_hash_uninit();
    return err;
}
