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
 *  ======== oad.c ========
 */

#ifndef WISUN_NCP_ENABLE
#undef EXCLUDE_TRACE
#endif
#include <unistd.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* Driver Header files */
#include <ti/devices/DeviceFamily.h>
#include <chipinfo.h>

/* Driver configuration */
#include "ti_drivers_config.h"
#include "ti_wisunfan_config.h"

#include "eventOS_event_timer.h"
#include "wisun_tasklet.h"
#include "application.h"
#include "ns_trace.h"

#include "time.h"
#include "oad.h"
#include "oad_storage.h"

#include DeviceFamily_constructPath(driverlib/sys_ctrl.h)
#include DeviceFamily_constructPath(driverlib/chipinfo.h)

/******************************************************************************
 Defines & enums
 *****************************************************************************/
#define TRACE_GROUP "oad "

#define OAD_TIMEOUT 5000 // 5000 ms
#define OAD_BLOCK_REQ_TIMEOUT_ID 0
#define OAD_COMPLETE_TIMEOUT_ID 1
#define MAX_BLOCK_REQ_RETRY_COUNT 3

#define OAD_IMG_ID 123
#define MCUBOOT_HEADER_VER_ADDR 20
#define MCUBOOT_VERSION_PTR ((&__PRIMARY_SLOT_BASE) + MCUBOOT_HEADER_VER_ADDR)
#define OAD_COMPLETE_FLAG 0xFFFF

/******************************************************************************
 Static & Global Variables
 *****************************************************************************/
/* Linker file created symbol for addressing MCUBoot header */
extern uint8_t __PRIMARY_SLOT_BASE;

/* OAD handle for accessing protocol tracking variables */
OAD_Handle_t oad_handle;

/* OAD tasklet ID */
static int8_t oad_tasklet_id = -1;

/******************************************************************************
 Function declarations
 *****************************************************************************/
static void oad_tasklet(arm_event_s *event);
static void oad_update(uint8_t event_type);
static int oad_img_rsp_cb(int8_t service_id, uint8_t source_address[static 16],
                               uint16_t source_port, sn_coap_hdr_s *response_ptr);
static int oad_complete_rsp_cb(int8_t service_id, uint8_t source_address[static 16],
                               uint16_t source_port, sn_coap_hdr_s *response_ptr);
static int mcuboot_version_cmp(mcuboot_image_version_t *ver1,
                              mcuboot_image_version_t *ver2);

/******************************************************************************
 Public function definitions
 *****************************************************************************/
void oad_tasklet_start(void)
{
    oad_tasklet_id = eventOS_event_handler_create(
        &oad_tasklet,
        OAD_INIT_EVT);
}

int coap_oad_cb(int8_t service_id, uint8_t source_address[static 16],
                       uint16_t source_port, sn_coap_hdr_s *request_ptr)
{
    uint16_t ret;
    bool ntf_accepted = false;
    oad_notif_rsp_msg_t notif_rsp_msg;

    if ((memcmp(request_ptr->uri_path_ptr, OAD_FWV_REQ_URI, request_ptr->uri_path_len) == 0) &&
            request_ptr->msg_code == COAP_MSG_CODE_REQUEST_GET)
    {
        tr_info("OAD FWV Req received. Sending OAD FWV Resp message.");

        // ACK firmware request with firmware response
        oad_fw_version_rsp_msg_t fw_rsp_msg;
        fw_rsp_msg.img_id = OAD_IMG_ID;
        fw_rsp_msg.platform_type = ChipInfo_GetChipType();
        memcpy(&fw_rsp_msg.image_version, MCUBOOT_VERSION_PTR,
               sizeof(fw_rsp_msg.image_version));

        ret = coap_service_response_send(service_id, 0, request_ptr, COAP_MSG_CODE_RESPONSE_CONTENT,
                                   COAP_CT_TEXT_PLAIN, (uint8_t *) &fw_rsp_msg, sizeof(fw_rsp_msg));
    }
    else if (!oad_handle.oad_in_progress &&
            (memcmp(request_ptr->uri_path_ptr, OAD_NOTIF_URI, request_ptr->uri_path_len) == 0) &&
            (request_ptr->msg_code == COAP_MSG_CODE_REQUEST_PUT ||
            request_ptr->msg_code == COAP_MSG_CODE_REQUEST_POST))
    {
        tr_info("OAD Notif Req received. Determining whether to accept or reject.");

        // Store notification request info
        if (request_ptr->payload_len != sizeof(oad_handle.oad_notif_req_info))
        {
            tr_warn("OAD Notif Req rejected, invalid payload.");
            ret = coap_service_response_send(service_id, 0, request_ptr, COAP_MSG_CODE_RESPONSE_BAD_REQUEST,
                                       COAP_CT_TEXT_PLAIN, NULL, 0);
            return 0;
        }
        memcpy(&oad_handle.oad_notif_req_info, request_ptr->payload_ptr, request_ptr->payload_len);
        ntf_accepted = false;
        if (oad_handle.oad_notif_req_info.img_id != OAD_IMG_ID)
        {
            tr_warn("OAD Notif Req rejected. Wrong image ID. Sending OAD Notif Resp message.");
        }
        else if (oad_handle.oad_notif_req_info.platform_type != ChipInfo_GetChipType())
        {
            tr_warn("OAD Notif Req rejected. Wrong platform. Sending OAD Notif Resp message.");
        }
        else if (mcuboot_version_cmp(&oad_handle.oad_notif_req_info.image_version,
                                     (mcuboot_image_version_t *) MCUBOOT_VERSION_PTR) <= 0)
        {
            tr_warn("OAD Notif Req rejected. Image version lower than current version. "
                    "Sending OAD Notif Resp message.");
        }
        else
        {
            tr_info("OAD Notif Req accepted. Sending OAD Notif Resp message.");
            ntf_accepted = true;
        }

        notif_rsp_msg.img_id = oad_handle.oad_notif_req_info.img_id;
        if (ntf_accepted)
        {
            // ACK notification request with notification response (accepted)
            notif_rsp_msg.status = 1;
            ret = coap_service_response_send(service_id, 0, request_ptr, COAP_MSG_CODE_RESPONSE_CHANGED,
                                       COAP_CT_TEXT_PLAIN, (uint8_t *) &notif_rsp_msg, sizeof(notif_rsp_msg));
        }
        else
        {
            // ACK notification request with notification response (rejected)
            notif_rsp_msg.status = 0;
            ret = coap_service_response_send(service_id, 0, request_ptr, COAP_MSG_CODE_RESPONSE_CHANGED,
                                       COAP_CT_TEXT_PLAIN, (uint8_t *) &notif_rsp_msg, sizeof(notif_rsp_msg));
            return 0;
        }

        // Send initial block request message
        if (ret == 0)
        {
            tr_info("OAD Notif Resp sent. OAD block transfer started.");
            memcpy(oad_handle.oad_source_address, source_address, 16);
            oad_handle.oad_in_progress = true; // Set oad_in_progress to true
            // Reset tracking variables (do not reset oad_notif_req_info)
            oad_handle.block_req_in_progress = false;
            oad_handle.curr_block_num = 0;
            oad_handle.curr_bytes_recv = 0;
            oad_handle.block_req_retry_count = 0;
            // Free memory allocated for block transfer if not previously freed
            if (oad_handle.oad_curr_block.data != NULL)
            {
                free(oad_handle.oad_curr_block.data);
            }

            // Initialize temp memory for block storage
            oad_handle.oad_curr_block.data = malloc(oad_handle.oad_notif_req_info.block_len);
            if (oad_handle.oad_curr_block.data == NULL)
            {
                tr_err("Could not allocate memory for OAD block storage");
                oad_update(OAD_ABORT_EVT);
                return -1;
            }
            // Calculate total number of OAD blocks, round up if needed
            oad_handle.total_blocks = oad_handle.oad_notif_req_info.image_len/oad_handle.oad_notif_req_info.block_len;
            if( 0 != (oad_handle.oad_notif_req_info.image_len % (oad_handle.oad_notif_req_info.block_len)))
            {
                oad_handle.total_blocks += 1;
            }
            if (oad_handle.total_blocks == 0)
            {
                tr_err("Total block calculation failed.");
                oad_update(OAD_ABORT_EVT);
                return -1;
            }
            tr_info("Block calculation complete. Total blocks: %d", oad_handle.total_blocks);

            // Erase required flash pages to store new image
            OADStorage_init();
            ret = OADStorage_eraseImg(oad_handle.oad_notif_req_info.image_len);
            if (ret != 0)
            {
                tr_err("Could not erase flash pages for new OAD image");
                oad_update(OAD_ABORT_EVT);
                return -1;
            }
            // Send initial block request
            oad_update(OAD_BLOCK_REQ_EVT);
        }
    }
    return 0;
}

/**
 * @brief   Compare image version numbers not including the build number.
 *          Identical to MCUBoot version comparsion.
 *
 * @param   ver1 - Pointer to the first image version to compare.
 * @param   ver2 - Pointer to the second image version to compare.
 *
 * @return  -1 If ver1 is strictly less than ver2.
 * @return  0  If the image version numbers are equal, not including the build number.
 * @return  1  If ver1 is strictly greater than ver2.
 */
static int mcuboot_version_cmp(mcuboot_image_version_t *ver1,
                               mcuboot_image_version_t *ver2)
{
    if (ver1->iv_major > ver2->iv_major) {
        return 1;
    }
    if (ver1->iv_major < ver2->iv_major) {
        return -1;
    }
    /* The major version numbers are equal, continue comparison. */
    if (ver1->iv_minor > ver2->iv_minor) {
        return 1;
    }
    if (ver1->iv_minor < ver2->iv_minor) {
        return -1;
    }
    /* The minor version numbers are equal, continue comparison. */
    if (ver1->iv_revision > ver2->iv_revision) {
        return 1;
    }
    if (ver1->iv_revision < ver2->iv_revision) {
        return -1;
    }
    return 0;
}

/**
 * @brief   Callback for OAD image block response. Process the payload of the block
 *          response and trigger the OAD_BLOCK_RECV_EVT callback.
 *
 * @param   service_id     - Service ID for the CoAP block response
 * @param   source_address - Source address of the CoAP block response
 * @param   source_port    - Source port of the CoAP block response
 * @param   response_ptr    - Pointer to the contents of the CoAP block response
 *
 * @return  0 on successfully processing CoAP response. -1 otherwise.
 */
static int oad_img_rsp_cb(int8_t service_id, uint8_t source_address[static 16],
                               uint16_t source_port, sn_coap_hdr_s *response_ptr)
{
    uint8_t *payload_ptr = response_ptr->payload_ptr;
    if (oad_handle.block_req_in_progress) // OAD timeout not triggered, issue next block request
    {
        memcpy(&oad_handle.oad_curr_block.img_id, payload_ptr, sizeof(oad_handle.oad_curr_block.img_id));
        payload_ptr += sizeof(oad_handle.oad_curr_block.img_id);

        memcpy(&oad_handle.oad_curr_block.block_num, payload_ptr, sizeof(oad_handle.oad_curr_block.block_num));
        payload_ptr += sizeof(oad_handle.oad_curr_block.block_num);
        if (oad_handle.curr_block_num != oad_handle.oad_curr_block.block_num)
        {
            tr_err("Wrong block number received. Expected: %d | Received: %d",
                    oad_handle.curr_block_num, oad_handle.oad_curr_block.block_num);
            return -1;
        }

        oad_handle.oad_curr_block.data_len = response_ptr->payload_len -
                sizeof(oad_handle.oad_curr_block.img_id) - sizeof(oad_handle.oad_curr_block.block_num);
        if (oad_handle.oad_curr_block.data_len > oad_handle.oad_notif_req_info.block_len)
        {
            tr_err("More bytes than block size received. Expected: %d | Received: %d",
                   oad_handle.oad_notif_req_info.block_len, oad_handle.oad_curr_block.data_len);
            return -1;
        }
        memcpy(oad_handle.oad_curr_block.data, payload_ptr, oad_handle.oad_curr_block.data_len);

        oad_update(OAD_BLOCK_RECV_EVT);
    }
    else // OAD timeout triggered, block came back too late
    {
        tr_warn("Block request already timed out, rejecting current block. New block request issued.");
        return -1;
    }
    return 0;
}

/**
 * @brief   Callback for OAD complete response. This triggers boot into the newly downloaded image.
 *
 * @param   service_id     - Service ID for the CoAP complete response
 * @param   source_address - Source address of the CoAP complete response
 * @param   source_port    - Source port of the CoAP complete response
 * @param   request_ptr    - Pointer to the contents of the CoAP complete response
 *
 * @return  0 on successfully processing CoAP response. -1 otherwise. Should reset before ever returning.
 */

static int oad_complete_rsp_cb(int8_t service_id, uint8_t source_address[static 16],
                               uint16_t source_port, sn_coap_hdr_s *response_ptr)
{
    tr_info("Booting into new image");
    eventOS_event_timer_cancel(OAD_COMPLETE_TIMEOUT_ID, oad_tasklet_id);
    SysCtrlSystemReset(); // Reset device, allowing MCUBoot to copy & boot into downloaded image
    return 0; // Should never reach here
}

/**
 * @brief   Function to trigger an event in the oad_tasklet
 *
 * @param   event_type - What event to trigger. Enum of type oad_evt_t
 *
 * @return  None
 */
static void oad_update(uint8_t event_type)
{
    arm_event_s event = {
           .sender = 0,
           .receiver = oad_tasklet_id,
           .priority = ARM_LIB_LOW_PRIORITY_EVENT,
           .event_type = event_type,
           .event_id = 0,
           .event_data = 0
    };
    eventOS_event_send(&event);
}

/**
 * @brief   Tasklet for handling OAD block request/response and OAD complete request/response.
 *          Handles OAD_BLOCK_REQ_EVT, OAD_BLOCK_RECV_EVT, OAD_COMPLETE_EVT, OAD_ABORT_EVT,
 *          OAD_BLOCK_REQ_TIMEOUT_EVT, and OAD_COMPLETE_TIMEOUT_EVT
 *
 * @param   event - Event object containing event type to trigger
 *
 * @return  None
 */
static void oad_tasklet(arm_event_s *event)
{
    oad_evt_t event_type;
    uint8_t status;
    oad_block_req_msg_t block_req_msg;

    event_type = (oad_evt_t) event->event_type;

    switch (event_type) {
        // Init event called after tasklet creation
        case OAD_INIT_EVT:
            // Reset OAD tracking variables
            memset((void *) &oad_handle, 0, sizeof(oad_handle));
            break;
        // Trigger an OAD block request message
        case OAD_BLOCK_REQ_EVT:
            tr_info("Sending block request for block number %d", oad_handle.curr_block_num);
            oad_handle.block_req_in_progress = true;
            // Populate and send block request
            block_req_msg.img_id = oad_handle.oad_notif_req_info.img_id;
            block_req_msg.block_num = oad_handle.curr_block_num;
            block_req_msg.total_blocks = oad_handle.total_blocks;
            coap_service_request_send(service_id, 0, oad_handle.oad_source_address, COAP_PORT, COAP_MSG_TYPE_CONFIRMABLE,
                                      COAP_MSG_CODE_REQUEST_GET, OAD_IMAGE_URI, COAP_CT_TEXT_PLAIN,
                                      (uint8_t *) &block_req_msg, sizeof(block_req_msg), oad_img_rsp_cb);
            
            // Start OAD block request timeout timer
            eventOS_event_timer_request(OAD_BLOCK_REQ_TIMEOUT_ID, OAD_BLOCK_REQ_TIMEOUT_EVT, oad_tasklet_id, OAD_TIMEOUT);
            break;
        // Handle received OAD block response message
        case OAD_BLOCK_RECV_EVT:
            tr_info("Received block number %d", oad_handle.curr_block_num);
            oad_handle.block_req_in_progress = false;
            oad_handle.block_req_retry_count = 0;

            // Stop OAD block request timeout timer
            eventOS_event_timer_cancel(OAD_BLOCK_REQ_TIMEOUT_ID, oad_tasklet_id);

            // Copy block into flash
            status = OADStorage_imgBlockWrite(oad_handle.curr_block_num,
                                              oad_handle.oad_notif_req_info.block_len,
                                              oad_handle.oad_curr_block.data,
                                              oad_handle.oad_curr_block.data_len);
            if (status != OADStorage_Status_Success)
            {
                tr_err("OAD image block write failed");
                oad_update(OAD_ABORT_EVT);
                return;
            }
            // Increment bytes received
            oad_handle.curr_bytes_recv += oad_handle.oad_curr_block.data_len;

            // If last block, stop and send OAD complete message
            if ((oad_handle.curr_block_num + 1) == oad_handle.total_blocks)
            {
                if (oad_handle.curr_bytes_recv == oad_handle.oad_notif_req_info.image_len)
                {
                    tr_info("OAD image transfer complete, sending OAD complete message");
                    OADStorage_close();
                    oad_update(OAD_COMPLETE_EVT);
                }
                else // Did not receive correct number of bytes
                {
                    tr_err("Total bytes different from expected image length. Received: %d | Expected: %d",
                           oad_handle.curr_bytes_recv, oad_handle.oad_notif_req_info.image_len);
                    oad_update(OAD_ABORT_EVT);
                    return;
                }
            }
            else // Not last block, start new block transmission
            {
                // Increment current block
                oad_handle.curr_block_num += 1;

                // Send next block request event
                oad_update(OAD_BLOCK_REQ_EVT);
            }
            break;
        // Trigger an OAD complete message
        case OAD_COMPLETE_EVT:
            block_req_msg.img_id = oad_handle.oad_notif_req_info.img_id;
            block_req_msg.block_num = OAD_COMPLETE_FLAG;
            // Note: block_req_msg.total_blocks value is ignored by OAD server
            coap_service_request_send(service_id, 0, oad_handle.oad_source_address, COAP_PORT, COAP_MSG_TYPE_CONFIRMABLE,
                                      COAP_MSG_CODE_REQUEST_GET, OAD_IMAGE_URI, COAP_CT_TEXT_PLAIN,
                                      (uint8_t *) &block_req_msg, sizeof(block_req_msg), oad_complete_rsp_cb);
            eventOS_event_timer_request(OAD_COMPLETE_TIMEOUT_ID, OAD_COMPLETE_TIMEOUT_EVT, oad_tasklet_id, OAD_TIMEOUT);
            break;
        case OAD_ABORT_EVT:
            tr_err("Aborting OAD.");
            // Send abort message
            coap_service_request_send(service_id, 0, oad_handle.oad_source_address, COAP_PORT, COAP_MSG_TYPE_NON_CONFIRMABLE,
                                      COAP_MSG_CODE_REQUEST_POST, OAD_ABORT_URI, COAP_CT_TEXT_PLAIN, NULL, 0, NULL);

            // Reset OAD tracking variables
            // Free memory allocated for block transfer
            if (oad_handle.oad_curr_block.data != NULL)
            {
                free(oad_handle.oad_curr_block.data);
            }
            memset((void *) &oad_handle, 0, sizeof(oad_handle));
            break;
        // Handle OAD block request timeout
        case OAD_BLOCK_REQ_TIMEOUT_EVT:
            tr_warn("OAD block timeout, attempt: %d", oad_handle.block_req_retry_count + 1);
            if (oad_handle.block_req_retry_count < MAX_BLOCK_REQ_RETRY_COUNT)
            {
                oad_handle.block_req_retry_count += 1;
                // Re-send OAD block request
                oad_update(OAD_BLOCK_REQ_EVT);
            }
            else
            {
                // Abort OAD due to max retries.
                oad_update(OAD_ABORT_EVT);
                return;
            }
            break;
        // Handle OAD complete message timeout
        case OAD_COMPLETE_TIMEOUT_EVT:
            // Give up on sending OAD complete message and reset into new image.
            tr_info("Booting into new image");
            eventOS_event_timer_cancel(OAD_COMPLETE_TIMEOUT_ID, oad_tasklet_id);
            SysCtrlSystemReset(); // Reset device, allowing MCUBoot to copy & boot into downloaded image
            break;
        default:
            break;
    } // switch(event_type)
}
