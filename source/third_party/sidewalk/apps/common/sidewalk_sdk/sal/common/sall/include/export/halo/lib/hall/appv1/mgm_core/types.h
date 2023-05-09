/*
 * Copyright 2020-2022 Amazon.com, Inc. or its affiliates. All rights reserved.
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

#ifndef RNETC_PUBLIC_HALO_HALL_APPV1_MGM_CORE_TYPES_H_
#define RNETC_PUBLIC_HALO_HALL_APPV1_MGM_CORE_TYPES_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
// P2P channel

typedef struct {
    uint8_t channel;
    uint8_t delay_s;
} halo_hall_appv1_mgm_p2p_channel_t;

// OTA Params

typedef enum {
    HALO_HALL_APPV1_OTA_PARAMS_BUILD_DEBUG = 0x0,
    HALO_HALL_APPV1_OTA_PARAMS_BUILD_USERDEF = 0x1,
    HALO_HALL_APPV1_OTA_PARAMS_BUILD_RELEASE = 0x2,

    HALO_HALL_APPV1_OTA_PARAMS_TAG_MAX_LEN = 20,
    HALO_HALL_APPV1_OTA_PARAMS_POOL_MAX_LEN = 20
} halo_hall_appv1_ota_params;

typedef struct {
    uint8_t tag_len;
    uint8_t tag[HALO_HALL_APPV1_OTA_PARAMS_TAG_MAX_LEN];
    uint8_t pool_len;
    uint8_t pool[HALO_HALL_APPV1_OTA_PARAMS_TAG_MAX_LEN];
    uint8_t type;
} halo_hall_appv1_ota_params_t;

// Firmware version

typedef struct {
    uint8_t major;
    uint8_t minor;
    uint8_t patch;
    uint8_t build;
} halo_hall_appv1_mgm_firmware_version_t;

// Reset info
typedef enum {
    HALO_HALL_APPV1_RESET_PIN_MASK = 0x1,
    HALO_HALL_APPV1_RESET_PIN_OFFSET = 7,
    HALO_HALL_APPV1_WDT_RESET_MASK = 0x1,
    HALO_HALL_APPV1_WDT_RESET_OFFSET = 6,
    HALO_HALL_APPV1_SOFT_RESET_MASK = 0x1,
    HALO_HALL_APPV1_SOFT_RESET_OFFSET = 5,
    HALO_HALL_APPV1_LOCKUP_RESET_MASK = 0x1,
    HALO_HALL_APPV1_LOCKUP_RESET_OFFSET = 4,
    HALO_HALL_APPV1_OFF_MODE_RESET_MASK = 0x1,
    HALO_HALL_APPV1_OFF_MODE_RESET_OFFSET = 3
} halo_hall_appv1_reset_reason_fields;

typedef struct {
    uint8_t reset_pin;
    uint8_t wdt_reset;
    uint8_t soft_reset;
    uint8_t lockup_reset;
    uint8_t off_mode_reset;
    uint8_t unused;
} halo_hall_appv1_reset_reason_t;

typedef struct {
    uint32_t program_counter;
    uint8_t version;
    halo_hall_appv1_reset_reason_t reason;
} halo_hall_appv1_reset_info_t;

// Beams ID / Keys

typedef enum { HALO_HALL_APPV1_PAN_ID_LEN = 5, HALO_HALL_APPV1_DEVICE_ID_LEN = 5, HALO_HALL_APPV1_DEVICE_SECURITY_KEY_LEN = 16 } halo_hall_appv1_ids;

typedef struct {
    uint8_t id[HALO_HALL_APPV1_DEVICE_ID_LEN];
} halo_hall_appv1_device_id_t;

typedef halo_hall_appv1_device_id_t halo_hall_appv1_pan_id_t;

typedef struct {
    uint8_t key[HALO_HALL_APPV1_DEVICE_SECURITY_KEY_LEN];
} halo_hall_appv1_device_security_key_t;

// TODO HALO-11869: should be moved into beams code
typedef enum {
    HALO_HALL_APPV1_BEAMS_ID_LEN = HALO_HALL_APPV1_DEVICE_ID_LEN,
    HALO_HALL_APPV1_BEAMS_SECURITY_KEY_LEN = HALO_HALL_APPV1_DEVICE_SECURITY_KEY_LEN
} halo_hall_appv1_beams;
typedef halo_hall_appv1_device_id_t halo_hall_appv1_beams_id_t;
typedef halo_hall_appv1_device_security_key_t halo_hall_appv1_beams_security_key_t;
// XXX END of beams code

typedef struct {
    uint8_t state;
} halo_hall_appv1_mgm_config_complete_t;

// Status

typedef enum {
    HALO_HALL_APPV1_MGM_STATUS_DEV_TYPE_ENDPOINT = 0x00,
} halo_hall_appv1_mgm_status_dev_type;

// TODO HALO-11081 Move product dependent code out of hall lib
typedef enum {
    HALO_HALL_APPV1_MGM_STATUS_APP_TYPE_BEAMS   = 0x01,
    HALO_HALL_APPV1_MGM_STATUS_APP_TYPE_BUDDY   = 0x02,
    HALO_HALL_APPV1_MGM_STATUS_APP_TYPE_BRODY   = 0x03,
} halo_hall_appv1_mgm_status_app_type;

typedef enum {
    HALO_HALL_APPV1_GCS_UNCERTAINTY_1__32768 = 0x0,
    HALO_HALL_APPV1_GCS_UNCERTAINTY_32__32768 = 0x1,
    HALO_HALL_APPV1_GCS_UNCERTAINTY_1024__32768 = 0x2,
    HALO_HALL_APPV1_GCS_UNCERTAINTY_8192__32768 = 0x3
} halo_hall_appv1_gcs_uncertainty;

typedef struct {
    uint32_t sec;
    uint16_t fractions;
    uint16_t uncertainty;
    uint8_t uncertainty_units;
} halo_hall_appv1_gcs_time_t;

typedef struct {
    uint32_t rand_key;
    halo_hall_appv1_gcs_time_t gcs_time;
} halo_hall_appv1_gcs_time_wkey_t;

enum sall_mgm_factory_reset_type {
    SALL_MGM_FACTORY_RESET_TYPE_SOFT = 0x0,
    SALL_MGM_FACTORY_RESET_TYPE_HARD = 0x1,
    SALL_MGM_FACTORY_RESET_TYPE_REBOOT = 0x2,
};

enum sall_mgm_factory_reset_target_type {
    SALL_MGM_FACTORY_RESET_TARGET_TYPE_ALL = 0x0,
    SALL_MGM_FACTORY_RESET_TARGET_TYPE_MCU = 0x1,
    SALL_MGM_FACTORY_RESET_TARGET_TYPE_HOST = 0x2,
};

struct sall_mgm_factory_reset {
    uint8_t type; /** enum sall_mgm_factory_reset_type */
    uint8_t target_type; /** enum sall_mgm_factory_reset_target_type */
    uint8_t delay;
};

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* RNETC_PUBLIC_HALO_HALL_APPV1_MGM_CORE_TYPES_H_ */
