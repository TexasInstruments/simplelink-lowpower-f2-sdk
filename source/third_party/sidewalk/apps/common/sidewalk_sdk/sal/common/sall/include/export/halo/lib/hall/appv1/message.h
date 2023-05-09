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

#ifndef RNETC_PUBLIC_HALO_HALL_APPV1_MESSAGE_H_
#define RNETC_PUBLIC_HALO_HALL_APPV1_MESSAGE_H_

#include <halo/lib/hall/hallerr.h>
#include <sid_network_address.h>
#include <sid_security_crypt_mask_ifc.h>
#include <sid_network_data_ifc.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum { HALO_HALL_MESSAGE_DESCRIPTOR_MSG_ID_ANY = 0xFFFF } halo_hall_message_descriptor;

typedef enum {
    HALO_HALL_MSG_GET_INFO_TYPE_PHY_STATS         = 0,
    HALO_HALL_MSG_GET_INFO_TYPE_TIME_RECEIVED     = 1,
    HALO_HALL_MSG_GET_INFO_TYPE_RECEIVE_INTERFACE = 2,
    HALO_HALL_MSG_GET_INFO_TYPE_TIME_PROXY_CTX    = 3,
    HALO_HALL_MSG_GET_INFO_RAW_MSG_DATA           = 4,
    HALO_HALL_MSG_GET_INFO_RAW_MSG_DATA_LEN       = 5,
    HALO_HALL_MSG_GET_INFO_RX_CHANNEL             = 6,
    HALO_HALL_MSG_GET_INFO_IS_DUPLICATE           = 7,
    HALO_HALL_MSG_GET_INFO_RAW_PACKET_LENGTH      = 8,
} halo_hall_msg_get_info_type_t;

typedef enum {
    HALO_HALL_MSG_INFO_TYPE_TIME_PROXY_CTX_SIZE = 44,
} halo_hall_msg_info_type_size_t;

typedef const void* halo_hall_receive_interface_t;

typedef struct {
    int8_t rssi;
    int8_t snr;
} halo_hall_msg_get_info_phy_stats_t;

typedef const void *halo_hall_status_handler_ctx_t;

struct sall_status_notify_meta_info {
    const void *user_ctx;
    uint16_t msg_id;
    sid_error_t status;
    uint8_t msg_type;
    uint8_t raw_packet_length;
    int8_t tx_power;
    const network_interface_data_ifc *net_adapter;
};

/**
    * @brief Message delivery callback
    *
    * @param[in] user_ctx  user context which is given with this callback in @ref halo_hall_status_notify_t structure
    * @param[in] msg_id    unique message identifier, returned to user in message descriptor during
    *                          send operation of any Application Layer Command
    * @param[in] status    message delivery status
    */
typedef void (*halo_hall_ntfy_handler_t) (const struct sall_status_notify_meta_info *const info);

typedef struct {
    halo_hall_ntfy_handler_t notify_handler;
    halo_hall_status_handler_ctx_t user_ctx;
} halo_hall_status_notify_t;

/**
 * @brief halo_hall_flag_resp_req_t
 * Do not modify/add/delete these values,
 * FLAG_RESP_REQUIRED_NO and FLAG_RESP_REQUIRED_YES have direct mapping to bit value
 */
typedef enum {
    FLAG_RESP_REQUIRED_NO  = 0x00,
    FLAG_RESP_REQUIRED_YES = 0x01,
} halo_hall_flag_resp_req_t;

typedef enum {
    HALL_SEC_TYPE_GCS      = 0x00,
    HALL_SEC_TYPE_COUNTER  = 0x01,
} hall_sec_type_t;

typedef struct network_interface_data_ifc_s* network_interface_data_ifc;

typedef struct {
    void *opaque;
    const halo_hall_status_notify_t *status_notify;
    const network_interface_data_ifc *net_ifc;
    struct sid_address remote;
    struct sid_address inner_src;
    struct sid_address inner_dst;
    sall_app_error_t status;
    uint32_t msg_id;
    //TODO: HALO-6345: Remove sec_ena as part of whitelist command support
    uint8_t is_nack;
    uint8_t enc_ena;
    uint8_t sec_ena;
    uint8_t sec_type;
    uint8_t num_of_retries;
    uint8_t frame_ctx_size;
    uint8_t resp_required :4;
    uint8_t is_unified_asl :4;
    sid_security_crypt_mask_t crypt_mask;
    uint16_t ttl_sec;
} halo_hall_message_descriptor_t;

sid_error_t halo_hall_msg_get_info(const halo_hall_message_descriptor_t* desc, halo_hall_msg_get_info_type_t type, void* result);

sid_error_t halo_hall_msg_get_destination(const halo_hall_message_descriptor_t* desc, struct sid_address * dst);

sid_error_t halo_hall_message_descriptor_init_generic(halo_hall_message_descriptor_t* desc);

sid_error_t halo_hall_message_descriptor_init_ptr(halo_hall_message_descriptor_t* desc, const struct sid_address* remote);

sid_error_t halo_hall_message_descriptor_cpy(halo_hall_message_descriptor_t* dst, const halo_hall_message_descriptor_t* src);

halo_hall_message_descriptor_t halo_hall_message_descriptor_init(const struct sid_address* remote);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* RNETC_PUBLIC_HALO_HALL_APPV1_MESSAGE_H_ */
