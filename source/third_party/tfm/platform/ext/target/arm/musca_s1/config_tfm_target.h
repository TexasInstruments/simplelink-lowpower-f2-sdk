/*
 * Copyright (c) 2022-2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef __CONFIG_TFM_TARGET_H__
#define __CONFIG_TFM_TARGET_H__

/* Use stored NV seed to provide entropy */
#define CRYPTO_NV_SEED                         0

#ifdef PSA_API_TEST_CRYPTO
#define CRYPTO_STACK_SIZE                      0x2500
#endif

#endif /* __CONFIG_TFM_TARGET_H__ */
