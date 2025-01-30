/*
 * Copyright (c) 2022, Arm Limited. All rights reserved.
 * Copyright (c) 2024, Texas Instruments Incorporated. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef __CONFIG_TFM_PROJECT__H__
#define __CONFIG_TFM_PROJECT__H__


/* Attest Partition Configs */

/* Include optional claims in initial attestation token */
#define ATTEST_INCLUDE_OPTIONAL_CLAIMS         1

/* Include COSE key-id in initial attestation token */
#define ATTEST_INCLUDE_COSE_KEY_ID             0

/* The stack size of the Initial Attestation Secure Partition */
#define ATTEST_STACK_SIZE                      0x700

/* Set the initial attestation token profile */
#define ATTEST_TOKEN_PROFILE_PSA_IOT_1         1

/* ITS Partition Configs */

/* Create flash FS if it doesn't exist for Internal Trusted Storage partition */
#define ITS_CREATE_FLASH_LAYOUT                1

/* Enable emulated RAM FS for platforms that don't have flash for Internal Trusted Storage partition */
#define ITS_RAM_FS                             0

/* Validate filesystem metadata every time it is read from flash */
#define ITS_VALIDATE_METADATA_FROM_FLASH       1

/* The maximum asset size to be stored in the Internal Trusted Storage */
#define ITS_MAX_ASSET_SIZE                     1100

/* Size of the ITS internal data transfer buffer */
/* Set the ITS buffer size to the max asset size so that all requests can be handled in one iteration */
#define ITS_BUF_SIZE                           ITS_MAX_ASSET_SIZE

/* The maximum number of assets to be stored in the Internal Trusted Storage */
#define ITS_NUM_ASSETS                         32

/* The stack size of the Internal Trusted Storage Secure Partition */
#define ITS_STACK_SIZE                         0x720

/* SPM Partition Configs */

/* The maximal number of secure services that are connected or requested at the same time */
#define CONFIG_TFM_CONN_HANDLE_MAX_NUM         8

/* Enable the doorbell APIs */
#define CONFIG_TFM_DOORBELL_API                1

#endif /* __CONFIG_TFM_PROJECT__H__ */
