/*
 * Copyright 2019-2022 Amazon.com, Inc. or its affiliates. All rights reserved.
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

#ifndef HALO_CMD_H_
#define HALO_CMD_H_

#include <stdbool.h>
#include <stdint.h>

#include <lk/list.h>

#include <halo/lib/hall/appv1/message.h>
#include <halo/lib/hall/appv1/cmd_desc.h>
#include <sid_network_address.h>
#include <halo/lib/hall/hallerr.h>
#include <halo/lib/hall/platform.h>
#include <halo/lib/hall/core/policy.h>
#include <sid_time_types.h>
#include <sid_security_crypt_mask_ifc.h>
#include <sid_msg_info_utils.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint16_t hall_size_t;
typedef struct network_interface_data_ifc_s* network_interface_data_ifc;

typedef struct {
    uint8_t major;
    uint8_t minor;
} halo_hall_appv1_class_version_t;

// class version
typedef struct {
    halo_hall_appv1_class_version_t min_version;
    halo_hall_appv1_class_version_t max_version;
} halo_hall_appv1_class_versions_t;

typedef struct {
    int8_t rssi;
    int8_t snr;
} hall_phy_stats_t;

enum {
    HALO_HALL_MSG_SENT_STATUS_NOT_REPORTED = 0,
    HALO_HALL_MSG_SENT_STATUS_REPORTED = 1,
    HALO_HALL_MSG_SENT_STATUS_FAILED = 2
};

typedef struct {
    sid_error_t status;
    uint8_t retries;
    uint8_t priority;
    uint8_t sent_status;
} hall_delivery_info_t;

typedef struct halo_hall_msg_status_s {
    uint32_t seq_num;
    uint16_t class_id;
    uint16_t cmd_id;
    uint8_t cmd_type;
    uint16_t pld_size;
    sid_error_t status;
    uint8_t raw_packet_length;
    int8_t tx_power;
    struct sid_timespec timestamp;
} halo_hall_msg_status_t;

typedef enum {
    META_MSG_FLAGS_IS_RESPONSE_REQUIRED = 0x01,
    META_MSG_FLAGS_ENCRYPTION_ENABLED = 0x02,
    META_MSG_FLAGS_IS_PASS_THROUGH_PLD = 0x04,
    // TODO: HALO-6345: Remove sec_ena as part of whitelist command support
    META_MSG_FLAGS_SEC_ENA = 0x08,
    META_MSG_FLAGS_IS_SRC_ADDRESS_HASHED = 0x10,
    META_MSG_FLAGS_IS_UNIFIED_ASL = 0x20,
    META_MSG_FLAGS_IS_DUPLICATE = 0x40,
    META_MSG_FLAGS_MAX,
} meta_msg_flags;

_Static_assert(META_MSG_FLAGS_MAX < 0xff, "Update the meta_msg type");

typedef struct hall_meta_msg_s {
    struct sid_timespec timestamp;
    struct sid_address dst;
    struct sid_address src;
    hall_delivery_info_t delivery_info;
    const network_interface_data_ifc * net_adapter;
    const halo_hall_status_notify_t  * status_notify;
    void *frame_ctx;
    hall_phy_stats_t stats;
    hall_size_t frame_ctx_size;
    uint16_t seqn;
    uint8_t sec_type;
    uint8_t meta_flags;
    sid_security_crypt_mask_t crypt_mask;
    uint8_t rx_channel;
    uint8_t raw_packet_length;
    int8_t tx_power;
    uint8_t msg_crid[SID_MSG_CRID_SIZE];
} meta_msg_t;

static inline bool halo_hall_meta_msg_is_set(const uint8_t flag, const meta_msg_t* meta) {
    return (meta->meta_flags & flag) != 0;
}
static inline void halo_hall_meta_msg_set(const uint8_t flag, meta_msg_t* meta) {
    meta->meta_flags = (meta->meta_flags | flag);
}
static inline void halo_hall_meta_msg_clear(const uint8_t flag, meta_msg_t* meta) {
    meta->meta_flags = (meta->meta_flags & ~flag);
}

typedef enum {
    HALO_HALL_MSG_TYPE_RD  = 0x0,
    HALO_HALL_MSG_TYPE_WR  = 0x1,
    HALO_HALL_MSG_TYPE_NTFY = 0x2,
    HALO_HALL_MSG_TYPE_RESP = 0x3,
} halo_hall_msg_type_t;

enum halo_hall_cmd_priority {
    HALO_HALL_CMD_PRIORITY_MAX  = 0x0,
    HALO_HALL_CMD_PRIORITY_MIN  = UINT8_MAX,
};

typedef struct {
    hall_cmd_descriptor_t cmd;
    sall_app_error_t status_code;
    uint8_t pld_size;
} halo_appv1_cmd_descriptor_t;

typedef struct halo_all_cmd_s {
    list_node_t node;
    meta_msg_t meta_msg;
    hall_cmd_descriptor_t cmd;
    uint8_t* pld; //!< using address instead of uint8_t buffer[] with assumption that actual stack buffer might be in different storage
    uint8_t data_rate;
    sall_app_error_t status_code;
    hall_size_t size;
    hall_size_t pld_size;
    hall_size_t offset;
    halo_hall_policy_params_t policy;
} halo_all_cmd_t;

sid_error_t halo_hall_fill_all_cmd_from_appv1_cmd_descriptor(halo_all_cmd_t *all_cmd, const halo_appv1_cmd_descriptor_t* cmd);

sid_error_t halo_hall_fill_appv1_cmd_descriptor_from_all_cmd(halo_appv1_cmd_descriptor_t *cmd, const halo_all_cmd_t *all_cmd);

sid_error_t halo_hall_get_data_from_all_cmd_offset(const halo_all_cmd_t *all_cmd, void* data, hall_size_t size);

sid_error_t halo_hall_put_data_in_all_cmd_offset(halo_all_cmd_t *all_cmd, const void* data, hall_size_t size);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* HALO_CMD_H_ */
