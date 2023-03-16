/*
 * Copyright (c) 2022, Texas Instruments Incorporated. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

/**
 * \file ti_psa_tfm.h
 *
 * \brief Include this header to avoid duplication of symbols between TI PSA and
 *        Mbed Crypto.
 *
 * \note  This file should be included before including any PSA Crypto headers
 *        from Mbed Crypto.
 */

#ifndef TI_PSA_TFM_H
#define TI_PSA_TFM_H

#define PSA_FUNCTION_NAME(x) ti_tfm__ ## x

#define psa_crypto_init \
        PSA_FUNCTION_NAME(psa_crypto_init)

#define psa_hash_setup \
        PSA_FUNCTION_NAME(psa_hash_setup)
#define psa_hash_update \
        PSA_FUNCTION_NAME(psa_hash_update)
#define psa_hash_finish \
        PSA_FUNCTION_NAME(psa_hash_finish)

#define psa_sign_hash \
        PSA_FUNCTION_NAME(psa_sign_hash)

#endif /* TI_PSA_TFM_H */
