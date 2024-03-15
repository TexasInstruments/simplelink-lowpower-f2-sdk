/*
 * Copyright (c) 2015-2019, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ======== oad.h ========
 */

#ifndef OAD_H
#define OAD_H

#include "coap_service_api.h"

/******************************************************************************
 Defines
 *****************************************************************************/
#define OAD_FWV_REQ_URI "oad/fwv"
#define OAD_NOTIF_URI "oad/ntf"
#define OAD_IMAGE_URI "oad/img"
#define OAD_ABORT_URI "oad/abort"

/******************************************************************************
 Structs and enums
 *****************************************************************************/
/**
 * @brief OAD tasklet event enums
 */
typedef enum oad_evt {
    OAD_INIT_EVT              = 0,
    OAD_BLOCK_REQ_EVT         = 1,
    OAD_BLOCK_RECV_EVT        = 2,
    OAD_COMPLETE_EVT          = 3,
    OAD_ABORT_EVT             = 4,
    OAD_BLOCK_REQ_TIMEOUT_EVT = 5,
    OAD_COMPLETE_TIMEOUT_EVT  = 6,
} oad_evt_t;

/**
 * @brief MCUBoot version struct used in MCUBoot image header
 */
typedef struct __attribute__((__packed__)) {
    uint8_t iv_major;
    uint8_t iv_minor;
    uint16_t iv_revision;
    uint32_t iv_build_num;
} mcuboot_image_version_t;

/**
 * @brief MCUBoot image header struct (prepended to image files by imgTool)
 */
typedef struct __attribute__((__packed__)) {
    uint32_t ih_magic;
    uint32_t ih_load_addr;
    uint16_t ih_hdr_size;           /* Size of image header (bytes). */
    uint16_t ih_protect_tlv_size;   /* Size of protected TLV area (bytes). */
    uint32_t ih_img_size;           /* Does not include header. */
    uint32_t ih_flags;              /* IMAGE_F_[...]. */
    mcuboot_image_version_t ih_ver;
} mcuboot_image_header_t;

/**
 * @brief OAD firmware version response CoAP message payload format
 */
typedef struct __attribute__((__packed__)) {
    uint8_t      img_id;
    uint8_t      platform_type;
    mcuboot_image_version_t image_version;
} oad_fw_version_rsp_msg_t;

/**
 * @brief OAD notification request CoAP message payload format
 */
typedef struct  __attribute__((packed)){
  uint8_t        img_id;
  uint8_t        platform_type;
  uint16_t       block_len;
  uint32_t       image_len;
  mcuboot_image_version_t image_version;
} oad_notif_req_msg_t;

/**
 * @brief OAD notification response CoAP message payload format
 */
typedef struct __attribute__((__packed__)) {
    uint8_t      img_id;
    uint8_t      status;
} oad_notif_rsp_msg_t;

/**
 * @brief OAD block request CoAP message payload format
 */
typedef struct  __attribute__((packed)){
    uint8_t      img_id;
    uint16_t     block_num;
    uint16_t     total_blocks;
} oad_block_req_msg_t;

/**
 * @brief OAD block response CoAP message payload format
 */
typedef struct  __attribute__((packed)){
  uint8_t        img_id;
  uint16_t       block_num;
  uint8_t        *data;
  // Note: data_len is not part of response message payload.
  // Populated by checking size of data.
  uint16_t       data_len; 
} oad_block_rsp_msg_t;

/**
 * @brief Handle for accessing OAD protocol tracking variables
 */
typedef struct {
    uint16_t curr_block_num;
    uint16_t total_blocks;
    uint32_t curr_bytes_recv;
    bool oad_in_progress;
    bool block_req_in_progress;
    uint8_t block_req_retry_count;
    oad_notif_req_msg_t oad_notif_req_info;
    uint8_t oad_source_address[16];
    oad_block_rsp_msg_t oad_curr_block;
} OAD_Handle_t;

/******************************************************************************
 Functions
 *****************************************************************************/

/**
 * @brief   Initializes the CoAP OAD tasket which handles CoAP block transfer
 *
 * @param   None.
 *
 * @return  None.
 */
void oad_tasklet_start(void);

/**
 * @brief   CoAP callback for the CoAP OAD service. Handles CoAP requests to the
 *          oad/fvw and oad/ntf URIs.
 *
 * @param   service_id     - Service ID for the CoAP server
 * @param   source_address - Source address of the CoAP request message
 * @param   source_port    - Source port of the CoAP request message
 * @param   request_ptr    - Pointer to the contents of the CoAP request message
 *
 * @return  0 on successfully processing CoAP request. -1 otherwise.
 */
int coap_oad_cb(int8_t service_id, uint8_t source_address[static 16],
                       uint16_t source_port, sn_coap_hdr_s *request_ptr);

#endif /* OAD_H */
