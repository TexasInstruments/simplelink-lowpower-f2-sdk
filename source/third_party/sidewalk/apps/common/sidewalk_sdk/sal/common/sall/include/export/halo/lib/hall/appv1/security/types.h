/*
 * Copyright 2019-2021 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.TXT file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef HALLO_ALL_APPV1_SECURITY_TYPES_H_
#define HALLO_ALL_APPV1_SECURITY_TYPES_H_

#include <stdint.h>
#include <sid_network_address.h>
#include <halo/lib/hall/platform.h>

#include <sid_time_types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define INTERMEDIATE_CERT_SERIAL_LEN 4
#define CLOUD_CERT_SERIAL_LEN        12
#define DEVICE_NONCE_LEN             8
#define REMOTE_NONCE_LEN             8
#define EXTENDED_DEVICE_NONCE_LEN    12
#define EXTENDED_REMOTE_NONCE_LEN    12
#define SERIAL_NUMBER_LEN            4
#define DEVICE_SERIAL_NUMBER_LEN     5
#define RNET_UUID_LEN                5
#define SECURITY_PUK_LEN             32
#define SECURITY_SIGNATURE_LEN       64
#define HAND_SHAKE_DEV_ID_LEN        5
#define HAND_SHAKE_RAND_LEN          8
#define FFS_HASH_CHOP_LEN            5
#define TIMING_CONFIG_LEN            6
#define PRODUCT_ID_LEN               4
#define CLOUD_CERT_LEN               4
#define TIMESYNC_SEED_LEN            4
#define D2D_GCM_KEY_LEN              16

//|1B Requester Mode|
typedef enum {
    HALO_HALL_APPV1_ECR_BLE = 0x01,
    HALO_HALL_APPV1_ECR_CLOUD = 0x02,
    HALO_HALL_APPV1_ECR_NETWORK = 0x03,
    HALO_HALL_APPV1_ECR_APPLICATION = 0x04,
    HALO_HALL_APPV1_ECR_MOBILE = 0x05,
    HALO_HALL_APPV1_NW_KEY_REFRESH = 0x06,
} halo_hall_appv1_ecr;

typedef enum {
    HALO_HALL_APPV1_ECMV_PRODUCTION = 0x03,
    HALO_HALL_APPV1_ECMV_DEV = 0x83,
} halo_hall_appv1_ecmv;

typedef enum {
    HALO_HALL_APPV1_ECSA_DISABLED = 0x00,
    HALO_HALL_APPV1_ECSA_ECDH_25519 = 0x01,
    HALO_HALL_APPV1_ECSA_ECDH_256R1 = 0x02,
} halo_hall_appv1_ecsa;

typedef enum {
    HALO_HALL_APPV1_INTERMEDIATE_LEVEL_DEVICE       = 0,
    HALO_HALL_APPV1_INTERMEDIATE_LEVEL_MODEL        = 1,
    HALO_HALL_APPV1_INTERMEDIATE_LEVEL_MANUFACTURER = 2,
    HALO_HALL_APPV1_INTERMEDIATE_LEVEL_ROOT         = 3 // AMZN
} halo_hall_appv1_intermediate_level_type;

typedef enum {
    HALO_HALL_APPV1_SUPPORTED_ALGO_SECURITY_DISABLED                     = 0,
    HALO_HALL_APPV1_SUPPORTED_ALGO_ECDH_25519_ED_25519_AES_CMAC_GCM_128  = 1,
    HALO_HALL_APPV1_SUPPORTED_ALGO_ECDH_256R1_ED_25519_AES_CMAC_GCM_128  = 2,
} halo_hall_appv1_supported_algo_type;

typedef enum {
    HALO_HALL_APPV1_DEVICE_ID_SIDEWALK_MASK = 0x00,
    HALO_HALL_APPV1_DEVICE_ID_SMSN_MASK     = 0x80,
} halo_hall_appv1_device_id_mask_type;

typedef enum {
    HALO_HALL_APPV1_NETWORK_KEY_REFRESH = 0x10,
    HALO_HALL_APPV1_APP_KEY_REFRESH     = 0x20,
} halo_hall_appv1_key_refresh_mask_type;

//|1B action|
typedef enum {
    HALO_HALL_APPV1_ACCEPT_SECURE_SESSION = 0x01,
    HALO_HALL_APPV1_START_BLE_ADVERTISEMENT = 0x02, // Not supported
} halo_hall_appv1_challenge_action;

//|1B Manufacturing Version| |1b device id type | 7b Supported Algo|
typedef struct {
    uint8_t manufacturing_version;
    uint8_t supported_algo; //halo_hall_appv1_device_id_mask_type|halo_hall_appv1_supported_algo_type
} resp_encryption_capabilities_t;

//|1B Intermediate Level| |4B Intermediate Certificate Serial|
typedef struct {
    uint8_t intermediate_certificate_serial[INTERMEDIATE_CERT_SERIAL_LEN];
    uint8_t intermediate_level; //halo_hall_appv1_intermediate_level_type;
} check_cert_serial_t;

// |1B frame count| |1B Cloud cert serial count|
// |1B Device Cert count| |12B Cloud cert serial| |8B Device Nonce|
typedef struct {
    uint8_t frame_count;
    uint8_t cloud_cert_serial_count;
    uint8_t device_cert_count;
    uint8_t cloud_cert_serial[CLOUD_CERT_SERIAL_LEN];
    uint8_t device_nonce[DEVICE_NONCE_LEN];
} resp_device_cert_count_nonce_t;

//|5B Serial number| | 32B Device PUK|
typedef struct {
    uint8_t serial_number[DEVICE_SERIAL_NUMBER_LEN];
    uint8_t device_puk[SECURITY_PUK_LEN];
} resp_device_serial_puk_t;

//|64B Device Signature|
typedef struct {
    uint8_t signature[SECURITY_SIGNATURE_LEN];
} security_signature_t;

//|32B Device ECDH PUK|
typedef struct {
    uint8_t ecdh_puk[SECURITY_PUK_LEN];
} ecdh_puk_t;

//|4B Serial number| | 32B Model PUK|
typedef struct {
    uint8_t serial_number[SERIAL_NUMBER_LEN];
    uint8_t model_puk[SECURITY_PUK_LEN];
} resp_model_serial_puk_t;

//|1B Frame Count| | 1B Cert Count| |8B Remote Nonce| |32B Remote ECDH PUK|
typedef struct {
    uint8_t frame_count;
    uint8_t cert_count;
    uint8_t remote_nonce[REMOTE_NONCE_LEN];
    uint8_t remote_ecdh_puk[SECURITY_PUK_LEN];
} remote_nonce_ecdh_puk_cert_count_t;

//|5B Dev Id| |8B Rand_A|
typedef struct {
    uint8_t dev_id[HAND_SHAKE_DEV_ID_LEN];
    uint8_t rand_a[HAND_SHAKE_RAND_LEN];
} hand_shake_t;

//|8B Rand_A| |8B Rand_B|
typedef struct {
    uint8_t rand_a[HAND_SHAKE_RAND_LEN];
    uint8_t rand_b[HAND_SHAKE_RAND_LEN];
} resp_hand_shake_t;

//|5B Dev Id| |8B Rand_A|
typedef struct {
    uint8_t action;
    uint8_t dev_id[HAND_SHAKE_DEV_ID_LEN];
    uint8_t rand_a[HAND_SHAKE_RAND_LEN];
} challenge_request_t;

//|8B Rand_A| |8B Rand_B|
typedef struct {
    uint8_t rand_a[HAND_SHAKE_RAND_LEN];
    uint8_t rand_b[HAND_SHAKE_RAND_LEN];
} challenge_response_t;

//|16B D2D GCM Key|
typedef struct {
    uint8_t key[D2D_GCM_KEY_LEN];
} d2d_gcm_key_t;

typedef enum {
    HALO_HALL_APPV1_SJNS_SUCCESS = 0x01,
    HALO_HALL_APPV1_SJNS_FAILED = 0x02,
    HALO_HALL_APPV1_SJNS_PENDING = 0x03,
} halo_hall_appv1_sjns;

typedef enum {
    HALO_HALL_APPV1_SECURITY_ERROR_UNKNOWN = 0x00,
    HALO_HALL_APPV1_SECURITY_ERROR_ENCRYPT = 0x01,
    HALO_HALL_APPV1_SECURITY_ERROR_DECRYPT = 0x02,
    HALO_HALL_APPV1_SECURITY_ERROR_AUTHENTICATION = 0x03,
    HALO_HALL_APPV1_SECURITY_ERROR_SECURITY = 0x04,
} halo_hall_appv1_security_error;

//|1B Type| |1B Error Code|
typedef struct {
    uint8_t error_type;
    uint8_t error_code;
} security_error_t;

typedef struct {
    uint8_t product_id[PRODUCT_ID_LEN];
    uint8_t cloud_cert[CLOUD_CERT_LEN];
    uint8_t manufacturing_version;
    uint8_t supported_algo;
    uint8_t device_nonce[EXTENDED_DEVICE_NONCE_LEN];
    uint8_t hash_chop[FFS_HASH_CHOP_LEN];
    uint8_t ed25519_ecdhe_puk[SECURITY_PUK_LEN];
} halo_hall_appv1_security_provisioning_init_t;

typedef struct {
    uint8_t frame_count;
    uint8_t cert_count;
    uint8_t remote_nonce[EXTENDED_REMOTE_NONCE_LEN];
    uint8_t remote_ed25519_ecdhe_puk[SECURITY_PUK_LEN];
    uint8_t dev_id[DEVICE_SERIAL_NUMBER_LEN];
} halo_hall_appv1_security_provisioning_init_response_t;

typedef struct {
    uint8_t timesync_seed[TIMESYNC_SEED_LEN];
    sid_time_t tx_uuid_rotation;
    sid_time_t gcm_rotation;
    sid_time_t cmac_rotation;
    sid_time_t wan_tx_uuid_rotation;
} halo_hall_appv1_security_wan_params_t;

/**
 * | 1b unit of interval | 15b app ctr / d2d gcm key rotation interval |
 */
typedef struct {
    sid_time_t ctr_key_rotation;
} halo_hall_appv1_security_app_params_t;

#ifdef __cplusplus
} // extern "C"
#endif

#endif // HALLO_ALL_APPV1_SECURITY_TYPES__H
