/*
 * Copyright (c) 2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef __CONFIG_TFM_TARGET_H__
#define __CONFIG_TFM_TARGET_H__

/* Use stored NV seed to provide entropy */
#if (TEST_S != ON)
#define CRYPTO_NV_SEED                         0
#endif

/* The maximum asset size to be stored in the Protected Storage area. */
#define PS_MAX_ASSET_SIZE                      512

#endif /* __CONFIG_TFM_TARGET_H__ */
