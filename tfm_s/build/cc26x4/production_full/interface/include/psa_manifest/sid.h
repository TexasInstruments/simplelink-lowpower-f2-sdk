/*
 * Copyright (c) 2019-2020, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

/*********** WARNING: This is an auto-generated file. Do not edit! ***********/

#ifndef __PSA_MANIFEST_SID_H__
#define __PSA_MANIFEST_SID_H__

#ifdef __cplusplus
extern "C" {
#endif

/******** INTERNAL_STORAGE_SP ********/
#define FLASH_SP_SERVICE_SID                                       (0xAE000070U)
#define FLASH_SP_SERVICE_VERSION                                   (1U)

/******** CRYPTO_SP ********/
#define CRYPTO_SP_SERVICE_SID                                      (0xAE000080U)
#define CRYPTO_SP_SERVICE_VERSION                                  (1U)
#define TFM_ITS_SET_SID                                            (0x00000070U)
#define TFM_ITS_SET_VERSION                                        (1U)
#define TFM_ITS_GET_SID                                            (0x00000071U)
#define TFM_ITS_GET_VERSION                                        (1U)
#define TFM_ITS_GET_INFO_SID                                       (0x00000072U)
#define TFM_ITS_GET_INFO_VERSION                                   (1U)
#define TFM_ITS_REMOVE_SID                                         (0x00000073U)
#define TFM_ITS_REMOVE_VERSION                                     (1U)

/******** TFM_SP_INITIAL_ATTESTATION ********/
#define TFM_ATTEST_GET_TOKEN_SID                                   (0x00000020U)
#define TFM_ATTEST_GET_TOKEN_VERSION                               (1U)
#define TFM_ATTEST_GET_TOKEN_SIZE_SID                              (0x00000021U)
#define TFM_ATTEST_GET_TOKEN_SIZE_VERSION                          (1U)
#define TFM_ATTEST_GET_PUBLIC_KEY_SID                              (0x00000022U)
#define TFM_ATTEST_GET_PUBLIC_KEY_VERSION                          (1U)

#ifdef __cplusplus
}
#endif

#endif /* __PSA_MANIFEST_SID_H__ */
