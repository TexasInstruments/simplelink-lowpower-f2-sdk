/*
 * Copyright (c) 2014-2019, Arm Limited and affiliates.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * \file mac_mcps_sap.c
 * \brief Add short description about this file!!!
 *
 */
#include "nsconfig.h"
#include "ns_types.h"
#include "eventOS_event.h"
#include "eventOS_scheduler.h"
#include "eventOS_callback_timer.h"
#include "string.h"
#include "ns_trace.h"
#include "nsdynmemLIB.h"
#include "ccmLIB.h"
#include "mlme.h"
#include "mac_api.h"
#include "fhss_api.h"
#include "platform/arm_hal_interrupt.h"
#include "common_functions.h"
#include "Core/include/ns_monitor.h"
#include "randLIB.h"

#include "MAC/IEEE802_15_4/sw_mac_internal.h"
#include "MAC/IEEE802_15_4/mac_defines.h"
#include "MAC/IEEE802_15_4/mac_timer.h"
#include "MAC/IEEE802_15_4/mac_security_mib.h"
#include "MAC/IEEE802_15_4/mac_mlme.h"
#include "MAC/IEEE802_15_4/mac_filter.h"
#include "MAC/IEEE802_15_4/mac_pd_sap.h"
#include "MAC/IEEE802_15_4/mac_mcps_sap.h"
#include "MAC/IEEE802_15_4/mac_header_helper_functions.h"
#include "MAC/IEEE802_15_4/mac_indirect_data.h"
#include "MAC/IEEE802_15_4/mac_cca_threshold.h"
#include "MAC/rf_driver_storage.h"

#include "sw_mac.h"

#include "fhss_config.h"
#include "Service_Libs/fhss/fhss.h"
#include "Service_Libs/fhss/fhss_common.h"
#include "Service_Libs/fhss/fhss_ws.h"

#include "6LoWPAN/ws/ws_llc.h"
#include "6LoWPAN/ws/ws_common.h"
#include "NWK_INTERFACE/Include/protocol.h"

#include "rcp_host.h"
#include "rcp_queue.h"

#ifndef LINUX_NANOSTACK
#include "hmac_map_direct.h"
#endif

#define TRACE_GROUP "mRCp"

extern bool mac_filter_list_allow_packet(uint8_t addr[8]);

static void mac_mlme_write_mac16(protocol_interface_rf_mac_setup_s *rf_setup, uint8_t *addrPtr);
static void mac_mlme_write_mac64(struct protocol_interface_rf_mac_setup *rf_setup, uint8_t *addrPtr);
static bool mac_data_counter_too_small(uint32_t current_counter, uint32_t packet_counter);
static void mcps_data_confirm_cb(protocol_interface_rf_mac_setup_s *rf_mac_setup, mcps_data_conf_t *confirm, mac_pre_parsed_frame_t *ack_buf);
static void mac_pd_data_confirm_failure_event_handle(mcps_data_conf_t *mcps_data_conf);

static int8_t mac_tasklet_event_handler = -1;

typedef struct __rcp_sap_dbg__
{
    uint16_t num_sap_data_req;
    uint16_t num_err_edfe;
    uint16_t num_err_DstAddrMode;
    uint16_t num_err_SrcAddrMode;
    uint16_t num_err_unknown_security;
    uint16_t num_err_headerIE;
    uint16_t num_err_payloadIE;
    uint16_t num_err_mlme_para;
    uint16_t num_err_async_para;
    uint16_t num_err_msduLen;
    uint16_t num_err_mlme_transaction_overflow;
    uint16_t num_err_macState;
    uint16_t num_err_packet_build;
    uint16_t num_err_pkt_build_mlme_unavaliable_key;
    uint16_t num_err_pkt_build_security_parameter;
    uint16_t num_err_pkt_build_siez_too_long;
    uint16_t num_err_nbr_table_info_not_found;
    uint16_t num_err_nbr_table_info_not_found_eapol;
    uint16_t num_err_nbr_table_info_not_found_6lowpan;
    uint16_t num_err_tx_busy;
    uint16_t num_err_post_cnf_failure;
    uint16_t num_err_interface_not_found;
} RCP_SAP_DBG_s;

RCP_SAP_DBG_s rcpSapDbg;

extern rcp_host_internal_t rcp_host_store;
extern RCP_HOST_DBG_s rcp_host_dbg;

static void mac_mlme_write_mac16(protocol_interface_rf_mac_setup_s *rf_setup, uint8_t *addrPtr)
{
    common_write_16_bit(rf_setup->mac_short_address, addrPtr);
}

static void mac_mlme_write_mac64(protocol_interface_rf_mac_setup_s *rf_setup, uint8_t *addrPtr)
{
    memcpy(addrPtr, rf_setup->mac64, 8);
}

static bool mac_data_counter_too_small(uint32_t current_counter, uint32_t packet_counter)
{
    if ((current_counter - packet_counter) >= 2) {
        return true;
    }
    return false;
}

static void mcps_data_confirm_cb(protocol_interface_rf_mac_setup_s *rf_mac_setup, mcps_data_conf_t *confirm, mac_pre_parsed_frame_t *ack_buf)
{
    if (get_sw_mac_api(rf_mac_setup)) {
        if (rf_mac_setup->mac_extension_enabled) {
            mcps_data_conf_payload_t data_conf;
            memset(&data_conf, 0, sizeof(mcps_data_conf_payload_t));
            if (ack_buf) {
                data_conf.payloadIeList = ack_buf->payloadsIePtr;
                data_conf.payloadIeListLength = ack_buf->payloadsIeLength;
                data_conf.headerIeList = ack_buf->headerIePtr;
                data_conf.headerIeListLength = ack_buf->headerIeLength;
                data_conf.payloadLength = ack_buf->mac_payload_length;
                data_conf.payloadPtr = ack_buf->macPayloadPtr;
            }
            //Check Payload Here
            get_sw_mac_api(rf_mac_setup)->data_conf_ext_cb(get_sw_mac_api(rf_mac_setup), confirm, &data_conf);
        } else {
            get_sw_mac_api(rf_mac_setup)->data_conf_cb(get_sw_mac_api(rf_mac_setup), confirm);
        }
    }
}

static void mac_pd_data_confirm_failure_handle(protocol_interface_rf_mac_setup_s *rf_mac_setup)
{
    mcps_data_conf_t mcps_data_conf;
    memset(&mcps_data_conf, 0, sizeof(mcps_data_conf_t));
    mcps_data_conf.msduHandle = rf_mac_setup->mac_mcps_data_conf_fail.msduHandle;
    mcps_data_conf.status = rf_mac_setup->mac_mcps_data_conf_fail.status;
    mcps_data_confirm_cb(rf_mac_setup, &mcps_data_conf, NULL);
}

static void mac_pd_data_confirm_failure_event_handle(mcps_data_conf_t *mcps_data_conf)
{
    rcp_host_store.mac_api->data_conf_ext_cb(rcp_host_store.mac_api, mcps_data_conf, NULL);
    rcp_host_dbg.num_tx_conf_Txqueue_err++;
    rcp_tx_data_queue_tx_confirm_handler(mcps_data_conf->msduHandle);
}

static bool mac_ie_vector_length_validate(ns_ie_iovec_t *ie_vector, uint16_t iov_length,  uint16_t *length_out)
{
    if (length_out) {
        *length_out = 0;
    }

    if (!iov_length) {
        return true;
    }

    if (iov_length != 0 && !ie_vector) {
        return false;
    }

    uint16_t msg_length = 0;
    ns_ie_iovec_t *msg_iov = ie_vector;
    for (uint_fast16_t i = 0; i < iov_length; i++) {
        if (msg_iov->iovLen != 0 && !msg_iov->ieBase) {
            return false;
        }
        msg_length += msg_iov->iovLen;
        if (msg_length < msg_iov->iovLen) {
            return false;
        }
        msg_iov++;
    }

    if (length_out) {
        *length_out = msg_length;
    }

    return true;

}

static mlme_key_descriptor_t *mac_frame_security_key_get(protocol_interface_rf_mac_setup_s *rf_ptr, mac_pre_build_frame_t *buffer)
{
    /* Encrypt the packet payload if AES encyption bit is set */
    mlme_security_t key_source;
    key_source.KeyIdMode = buffer->aux_header.KeyIdMode;
    key_source.KeyIndex = buffer->aux_header.KeyIndex;
    key_source.SecurityLevel = buffer->aux_header.securityLevel;
    memcpy(key_source.Keysource, buffer->aux_header.Keysource, 8);
    return mac_sec_key_description_get(rf_ptr, &key_source, buffer->fcf_dsn.DstAddrMode, buffer->DstAddr, buffer->DstPANId);
}

static void mcps_generic_sequence_number_allocate(protocol_interface_rf_mac_setup_s *rf_ptr, mac_pre_build_frame_t *buffer)
{
    if (buffer->fcf_dsn.frameVersion < MAC_FRAME_VERSION_2015 || (buffer->fcf_dsn.frameVersion ==  MAC_FRAME_VERSION_2015 &&  !buffer->fcf_dsn.sequenceNumberSuppress)) {
        /* Allocate SQN */
        switch (buffer->fcf_dsn.frametype) {
            case MAC_FRAME_CMD:
            case MAC_FRAME_DATA:
                if (!buffer->DSN_allocated) {
                    buffer->fcf_dsn.DSN = mac_mlme_set_new_sqn(rf_ptr);
                    buffer->DSN_allocated = true;
                }
                break;
#if 0
            case MAC_FRAME_BEACON:
                buffer->fcf_dsn.DSN = mac_mlme_set_new_beacon_sqn(rf_ptr);
                break;
#endif
            default:
                break;
        }
    }
}

static int8_t mcps_generic_packet_build(protocol_interface_rf_mac_setup_s *rf_ptr, mac_pre_build_frame_t *buffer, uint8_t *frame_count_offset)
{
    phy_device_driver_s *dev_driver = rf_ptr->dev_driver->phy_driver;
    dev_driver_tx_buffer_s *tx_buf = &rf_ptr->dev_driver_tx_buffer;

    ccm_globals_t ccm_ptr;
#ifndef WISUN_RCP_ENABLE
    if (buffer->mac_header_length_with_security == 0) {
        rf_ptr->mac_tx_status.length = buffer->mac_payload_length;
        uint8_t *ptr = tx_buf->buf;
        if (dev_driver->phy_header_length) {
            ptr += dev_driver->phy_header_length;
        }
        tx_buf->len = buffer->mac_payload_length;

        memcpy(ptr, buffer->mac_payload, buffer->mac_payload_length);
        buffer->tx_time = mcps_generic_backoff_calc(rf_ptr);
        return 0;
    }
#endif
    //This will prepare MHR length with Header IE
    mac_header_information_elements_preparation(buffer);

    mcps_generic_sequence_number_allocate(rf_ptr, buffer);
    mlme_key_descriptor_t *key_desc = NULL;
    if (buffer->fcf_dsn.securityEnabled) {
        bool increment_framecounter = false;
        //Remember to update security counter here!
        key_desc = mac_frame_security_key_get(rf_ptr, buffer);
        if (!key_desc) {
            buffer->status = MLME_UNAVAILABLE_KEY;
            rcpSapDbg.num_err_pkt_build_mlme_unavaliable_key++;
            return -2;
        }

        //GET Counter
        uint32_t new_frameCounter = mac_sec_mib_key_outgoing_frame_counter_get(rf_ptr, key_desc);
        // If buffer frame counter is set, this is FHSS channel retry, update frame counter only if something was sent after failure
        if ((buffer->aux_header.frameCounter == 0xffffffff) || buffer->asynch_request || mac_data_counter_too_small(new_frameCounter, buffer->aux_header.frameCounter)) {
            buffer->aux_header.frameCounter = new_frameCounter;
            increment_framecounter = true;
        }
#ifndef WISUN_RCP_ENABLE
        if (!mac_frame_security_parameters_init(&ccm_ptr, rf_ptr, buffer, key_desc)) {
            rcpSapDbg.num_err_pkt_build_security_parameter++;
            return -2;
        }
#endif
        //Increment security counter
        if (increment_framecounter) {
            mac_sec_mib_key_outgoing_frame_counter_increment(rf_ptr, key_desc);
        }
    }

    //Calculate Payload length here with IE extension
    uint16_t frame_length = mac_buffer_total_payload_length(buffer);
    //Storage Mac Payload length here
    uint16_t mac_payload_length = frame_length;

    if (mac_payload_length > MAC_IEEE_802_15_4_MAX_MAC_SAFE_PAYLOAD_SIZE &&
            rf_ptr->phy_mtu_size == MAC_IEEE_802_15_4_MAX_PHY_PACKET_SIZE) {
        /* IEEE 802.15.4-2003 only allowed unsecured payloads up to 102 bytes
        * (always leaving room for maximum MAC overhead).
        * IEEE 802.15.4-2006 allows bigger if MAC header is small enough, but
        * we have to set the version field.
        */
        if (buffer->fcf_dsn.frameVersion < MAC_FRAME_VERSION_2006) {
            buffer->fcf_dsn.frameVersion = MAC_FRAME_VERSION_2006;
        }
    }
#ifndef WISUN_RCP_ENABLE
    if (rf_ptr->mac_ack_tx_active) {
        if (buffer->fcf_dsn.securityEnabled) {
            ccm_free(&ccm_ptr);
        }
        return 0;
    }
#endif
    //Add MHR length to total length
    frame_length += buffer->mac_header_length_with_security + buffer->security_mic_len;
    if ((frame_length) > rf_ptr->phy_mtu_size - 2) {
        tr_debug("Too Long %u, %u pa %u header %u mic %u", frame_length, mac_payload_length, buffer->mac_header_length_with_security,  buffer->security_mic_len, rf_ptr->phy_mtu_size);
        buffer->status = MLME_FRAME_TOO_LONG;
        //decrement security counter
        if (key_desc) {
            mac_sec_mib_key_outgoing_frame_counter_decrement(rf_ptr, key_desc);
        }
        rcpSapDbg.num_err_pkt_build_siez_too_long++;
        return -1;
    }

    rf_ptr->mac_tx_status.length = frame_length;
    uint8_t *ptr = tx_buf->buf;
    if (dev_driver->phy_header_length) {
        ptr += dev_driver->phy_header_length;
    }

    tx_buf->len = frame_length;
    // uint8_t *mhr_start = ptr;
    // if (buffer->ExtendedFrameExchange && buffer->fcf_dsn.SrcAddrMode == MAC_ADDR_MODE_NONE) {
    //     buffer->tx_time = mac_mcps_sap_get_phy_timestamp(rf_ptr) + 300; //Send 300 us later
    // } else {
    //     buffer->tx_time = mcps_generic_backoff_calc(rf_ptr);
    // }

    uint8_t *aux_sec_ptr = NULL;
    ptr = mac_generic_packet_write(rf_ptr, ptr, buffer, &aux_sec_ptr);
    if (aux_sec_ptr != NULL) {
        // Get offset of frame count of aux security header from start of frame
        *frame_count_offset = (uint8_t) ((aux_sec_ptr + 1) - ((uint8_t *) tx_buf->buf));
    }

#ifndef WISUN_RCP_ENABLE
    if (buffer->fcf_dsn.securityEnabled) {
        uint8_t open_payload = 0;
        if (buffer->fcf_dsn.frametype == MAC_FRAME_CMD) {
            open_payload = 1;
        }
        mac_security_data_params_set(&ccm_ptr, (mhr_start + (buffer->mac_header_length_with_security + open_payload)), (mac_payload_length - open_payload));
        mac_security_authentication_data_params_set(&ccm_ptr, mhr_start, (buffer->mac_header_length_with_security + open_payload));
        ccm_process_run(&ccm_ptr);
    }
#endif

    return 0;
}

int8_t mcps_sap_rcp_data_ind(rcp_data_ind_t *data_ind)
{
    if (mac_tasklet_event_handler < 0 || !data_ind) {
        return -1;
    }

    rcp_data_ind_t *buffer = ns_dyn_mem_temporary_alloc(sizeof(rcp_data_ind_t));
    if (buffer == NULL) {
        return -1;
    }
    memcpy(buffer, data_ind, sizeof(rcp_data_ind_t));

#ifdef WISUN_RCP_HOST_BR
    //copy pkt data
    if(data_ind->data_len > 0)
    {
        uint8_t *pktData = ns_dyn_mem_temporary_alloc(data_ind->data_len);
        if(pktData ==NULL) {
            return -1;
        }
        memcpy(pktData, data_ind->data_ptr, data_ind->data_len);
        buffer->data_ptr = pktData;
    }
    else{
        buffer->data_ptr = NULL;
    }
#endif

    arm_event_s event = {
        .receiver = mac_tasklet_event_handler,
        .sender = 0,
        .event_id = 0,
        .data_ptr = buffer,
        .event_type = MCPS_SAP_RCP_DATA_IND_EVENT,
        .priority = ARM_LIB_HIGH_PRIORITY_EVENT,
    };

    return eventOS_event_send(&event);
}

int8_t mcps_sap_rcp_data_cnf(rcp_data_cnf_t *data_cnf)
{
    if (mac_tasklet_event_handler < 0 || !data_cnf) {
        return -1;
    }

    rcp_data_cnf_t *buffer = ns_dyn_mem_temporary_alloc(sizeof(rcp_data_cnf_t));
    if (buffer == NULL) {
        return -1;
    }
    memcpy(buffer, data_cnf, sizeof(rcp_data_cnf_t));

#ifdef WISUN_RCP_HOST_BR
    //copy ack if present
    if(data_cnf->ack_frame_len > 0)
    {
        uint8_t *ackData = ns_dyn_mem_temporary_alloc(data_cnf->ack_frame_len);
        if (ackData == NULL) {
            return -1;
        }
        memcpy(ackData, data_cnf->ack_frame, data_cnf->ack_frame_len);
        buffer->ack_frame = ackData;
    }
    else{
        buffer->ack_frame = NULL;
    }
#endif

    arm_event_s event = {
        .receiver = mac_tasklet_event_handler,
        .sender = 0,
        .event_id = 0,
        .data_ptr = buffer,
        .event_type = MCPS_SAP_RCP_DATA_CNF_EVENT,
        .priority = ARM_LIB_HIGH_PRIORITY_EVENT,
    };

    return eventOS_event_send(&event);
}

int8_t mcps_sap_rcp_set_cnf(uint8_t event_type, rcp_mac_config_set_cnf_t *set_cnf)
{
    if (mac_tasklet_event_handler < 0 || !set_cnf) {
        return -1;
    }

    rcp_mac_config_set_cnf_t *buffer = ns_dyn_mem_temporary_alloc(sizeof(rcp_mac_config_set_cnf_t));
    if (buffer == NULL) {
        return -1;
    }
    memcpy(buffer, set_cnf, sizeof(rcp_mac_config_set_cnf_t));

    if (set_cnf->val_len > 0) {
        buffer->val = ns_dyn_mem_temporary_alloc(set_cnf->val_len);
        if (buffer->val == NULL) {
            return -1;
        }
        memcpy(buffer->val, set_cnf->val, set_cnf->val_len);
#ifndef WISUN_RCP_HOST_BR
        uint8_t *buf_free;
        //free set_cnf->val
        buf_free = set_cnf->val;
        MAP_mac_msg_deallocate((uint8_t **) &buf_free);
#endif
    } else {
        buffer->val = NULL;
    }

    arm_event_s event = {
        .receiver = mac_tasklet_event_handler,
        .sender = 0,
        .event_id = 0,
        .data_ptr = buffer,
        .event_type = event_type,
        .priority = ARM_LIB_HIGH_PRIORITY_EVENT,
    };

    return eventOS_event_send(&event);
}

static void mac_mcps_sap_data_tasklet(arm_event_s *event)
{
    rcp_init_cnf_t *rcp_init_cnf;
    rcp_mac_config_set_cnf_t *rcp_set_cnf;
    uint8_t event_type = event->event_type;

    switch (event_type) {
#ifdef WISUN_RCP_ENABLE
        case MCPS_SAP_RCP_DATA_CNF_EVENT:
            handle_rcp_data_cnf((rcp_data_cnf_t *) event->data_ptr);
            if (((rcp_data_cnf_t *) event->data_ptr)->ack_frame != NULL) {
#ifdef WISUN_RCP_HOST_BR
                ns_dyn_mem_free(((rcp_data_cnf_t *)(event->data_ptr))->ack_frame);
#else
                uint8_t *bufptr = ((rcp_data_cnf_t *)(event->data_ptr))->buf_free;
                MAP_mac_msg_deallocate((uint8_t **)&bufptr);          
#endif
            }
            ns_dyn_mem_free(event->data_ptr);
            break;
        case MCPS_SAP_RCP_DATA_IND_EVENT:
            handle_rcp_data_ind((rcp_data_ind_t *) event->data_ptr);
            if (((rcp_data_ind_t *) event->data_ptr)->data_ptr != NULL) {
#ifdef WISUN_RCP_HOST_BR
                ns_dyn_mem_free(((rcp_data_ind_t *)(event->data_ptr))->data_ptr); // data_ptr for the rcp_data_ind
#else
                uint8_t *bufptr = ((rcp_data_ind_t *)(event->data_ptr))->buf_free;
                MAP_mac_msg_deallocate((uint8_t **)&bufptr);               
#endif
            }
            ns_dyn_mem_free(event->data_ptr); // data_ptr for the event pointer
            break;
        case MCPS_SAP_RCP_MAC_INIT_CNF:
            rcp_set_cnf = (rcp_mac_config_set_cnf_t *) event->data_ptr;
            tr_info("<-----------------------RCP MAC init CNF received with status %u", rcp_set_cnf->status);
            if (rcp_set_cnf->val) {
                ns_dyn_mem_free(rcp_set_cnf->val);
            }
            ns_dyn_mem_free(event->data_ptr);
            break;
        case MCPS_SAP_RCP_FH_INIT_CNF:
            rcp_set_cnf = (rcp_mac_config_set_cnf_t *) event->data_ptr;
            tr_info("<------------------------RCP FH init CNF received with status %u", rcp_set_cnf->status);
            if (rcp_set_cnf->val) {
                ns_dyn_mem_free(rcp_set_cnf->val);
            }
            ns_dyn_mem_free(event->data_ptr);
            break;
        case MCPS_SAP_RCP_MAC_RESET_CNF:
            rcp_set_cnf = (rcp_mac_config_set_cnf_t *) event->data_ptr;
            tr_info("<---------------------RCP MAC reset CNF received with status %u", rcp_set_cnf->status);
            if (rcp_set_cnf->val) {
                ns_dyn_mem_free(rcp_set_cnf->val);
            }
            ns_dyn_mem_free(event->data_ptr);
            break;
        case MCPS_SAP_RCP_MAC_CONFIG_GET_CNF:
            rcp_set_cnf = (rcp_mac_config_set_cnf_t *) event->data_ptr;
            tr_info("RCP Get CNF received with status %u", rcp_set_cnf->status);
            if (rcp_set_cnf->val) {
                ns_dyn_mem_free(rcp_set_cnf->val);
            }
            ns_dyn_mem_free(event->data_ptr);
            break;
        case MCPS_SAP_RCP_MAC_CONFIG_SET_CNF:
            rcp_set_cnf = (rcp_mac_config_set_cnf_t *) event->data_ptr;
            tr_info("RCP Set CNF received with status %u, set attribute ID: %u", rcp_set_cnf->status, rcp_set_cnf->attr);
            if (rcp_set_cnf->val) {
                ns_dyn_mem_free(rcp_set_cnf->val);
            }
            ns_dyn_mem_free(event->data_ptr);
            break;
#endif
        case MCPS_SAP_DATA_CNF_FAIL_EVENT:
            mac_pd_data_confirm_failure_event_handle((mcps_data_conf_t *)event->data_ptr);
            ns_dyn_mem_free(event->data_ptr);
            break;

#ifdef WISUN_USE_NANOSTACK_FHSS_MAC_MODULE
        case MCPS_SAP_DATA_IND_EVENT:
            if (event->data_ptr) {
                mac_data_interface_frame_handler((mac_pre_parsed_frame_t *)event->data_ptr);
            }
            break;
        case MCPS_SAP_DATA_CNF_EVENT:
            //mac_data_interface_tx_done(event->data_ptr);
            mac_pd_data_confirm_handle((protocol_interface_rf_mac_setup_s *)event->data_ptr);
            break;
        case MCPS_SAP_DATA_ACK_CNF_EVENT:
            mac_pd_data_ack_handler((mac_pre_parsed_frame_t *)event->data_ptr);
            break;
        case MAC_MLME_EVENT_HANDLER:
            // mac_mlme_event_cb(event->data_ptr);
            break;
        case MAC_MCPS_INDIRECT_TIMER_CB:
            mac_indirect_data_ttl_handle((protocol_interface_rf_mac_setup_s *)event->data_ptr, (uint16_t)event->event_data);
            break;
        case MAC_MLME_SCAN_CONFIRM_HANDLER:
            mac_mlme_scan_confirmation_handle((protocol_interface_rf_mac_setup_s *) event->data_ptr);
            break;
        case MAC_CCA_THR_UPDATE:
            mac_cca_threshold_update((protocol_interface_rf_mac_setup_s *) event->data_ptr, event->event_data >> 8, (int8_t) event->event_data);
            break;
        case MAC_SAP_TRIG_TX:
            mac_clear_active_event((protocol_interface_rf_mac_setup_s *) event->data_ptr, MAC_SAP_TRIG_TX);
            mac_mcps_trig_buffer_from_queue((protocol_interface_rf_mac_setup_s *) event->data_ptr);
        //No break necessary
#endif
        default:
            break;
    }
}

int8_t mac_mcps_sap_tasklet_init(void)
{
    if (mac_tasklet_event_handler < 0) {
        mac_tasklet_event_handler = eventOS_event_handler_create(&mac_mcps_sap_data_tasklet, 0);
    }

    return mac_tasklet_event_handler;
}

mac_pre_build_frame_t *mcps_sap_prebuild_frame_buffer_get(uint16_t payload_size)
{
    mac_pre_build_frame_t *buffer = ns_dyn_mem_temporary_alloc(sizeof(mac_pre_build_frame_t));
    if (!buffer) {
        return NULL;
    }
    memset(buffer, 0, sizeof(mac_pre_build_frame_t));
    buffer->initial_tx_channel = 0xffff;
    buffer->aux_header.frameCounter = 0xffffffff;
    buffer->DSN_allocated = false;
    if (payload_size) {
        //Mac interlnal payload allocate
        buffer->mac_payload = ns_dyn_mem_temporary_alloc(payload_size);
        if (!buffer->mac_payload) {
            ns_dyn_mem_free(buffer);
            return NULL;
        }
        buffer->mac_allocated_payload_ptr = true;
        buffer->mac_payload_length = payload_size;
    } else {
        buffer->mac_allocated_payload_ptr = false;
    }
    return buffer;
}


void mcps_sap_prebuild_frame_buffer_free(mac_pre_build_frame_t *buffer)
{
    if (!buffer) {
        return;
    }

    if (buffer->mac_allocated_payload_ptr) {
        ns_dyn_mem_free(buffer->mac_payload);
    }
    //Free Buffer frame
    ns_dyn_mem_free(buffer);

}
#if 0
void mac_frame_src_address_set_from_interface(uint8_t SrcAddrMode, protocol_interface_rf_mac_setup_s *rf_ptr, uint8_t *addressPtr)
{
    if (!rf_ptr) {
        return;
    }
    if (SrcAddrMode == MAC_ADDR_MODE_16_BIT) {
        mac_mlme_write_mac16(rf_ptr, addressPtr);
    } else if (SrcAddrMode == MAC_ADDR_MODE_64_BIT) {
        mac_mlme_write_mac64(rf_ptr, addressPtr);
    }
}
#endif
void mac_generic_event_trig(uint8_t event_type, void *mac_ptr, bool low_latency)
{
#if 0
    arm_library_event_priority_e priority;
    if (low_latency) {
        priority = ARM_LIB_LOW_PRIORITY_EVENT;
    } else {
        priority = ARM_LIB_HIGH_PRIORITY_EVENT;
    }
    arm_event_s event = {
        .receiver = mac_tasklet_event_handler,
        .sender = 0,
        .event_id = 0,
        .data_ptr = mac_ptr,
        .event_type = event_type,
        .priority = priority,
    };

    eventOS_event_send(&event);
#endif
}

void mac_mcps_buffer_queue_free(protocol_interface_rf_mac_setup_s *rf_mac_setup)
{
#if 0
    if (rf_mac_setup->active_pd_data_request) {
        mcps_sap_prebuild_frame_buffer_free(rf_mac_setup->active_pd_data_request);
        rf_mac_setup->active_pd_data_request = NULL;
    }

    while (rf_mac_setup->pd_data_request_queue_to_go) {
        mac_pre_build_frame_t *buffer = mcps_sap_pd_req_queue_read(rf_mac_setup, false, true);
        if (buffer) {
            mcps_sap_prebuild_frame_buffer_free(buffer);
        }
    }

    while (rf_mac_setup->pd_data_request_bc_queue_to_go) {
        mac_pre_build_frame_t *buffer = mcps_sap_pd_req_queue_read(rf_mac_setup, true, true);
        if (buffer) {
            mcps_sap_prebuild_frame_buffer_free(buffer);
        }
    }

    while (rf_mac_setup->indirect_pd_data_request_queue) {
        mac_pre_build_frame_t *buffer = rf_mac_setup->indirect_pd_data_request_queue;
        if (buffer) {
            rf_mac_setup->indirect_pd_data_request_queue = buffer->next;
            mcps_sap_prebuild_frame_buffer_free(buffer);
        }
    }

    if (rf_mac_setup->pd_rx_ack_buffer) {
        if (rf_mac_setup->rf_pd_ack_buffer_is_in_use) {
            eventOS_cancel(&rf_mac_setup->mac_ack_event);
            rf_mac_setup->rf_pd_ack_buffer_is_in_use = false;
        }
        ns_dyn_mem_free(rf_mac_setup->pd_rx_ack_buffer);
        rf_mac_setup->pd_rx_ack_buffer = NULL;
        rf_mac_setup->allocated_ack_buffer_length = 0;
    }
#endif
}

int8_t mcps_sap_pd_confirm_failure(void *mac_ptr)
{
    if (mac_tasklet_event_handler < 0  || !mac_ptr) {
        return -2;
    }
    arm_event_s event = {
        .receiver = mac_tasklet_event_handler,
        .sender = 0,
        .event_id = 0,
        .data_ptr = mac_ptr,
        .event_type = MCPS_SAP_DATA_CNF_FAIL_EVENT,
        .priority = ARM_LIB_HIGH_PRIORITY_EVENT,
    };

    return eventOS_event_send(&event);
}

int8_t mcps_sap_pd_confirm_failure_event(uint8_t msdu_handle,uint8_t tx_error_status)
{
    mcps_data_conf_t *mcps_data_conf;

    if (mac_tasklet_event_handler < 0 ) {
        return -1;
    }
    mcps_data_conf = ns_dyn_mem_temporary_alloc(sizeof(mcps_data_conf_t));
    if (mcps_data_conf == NULL)
    {   // memory allocation failure
        return -2;
    }

    memset(mcps_data_conf, 0, sizeof(mcps_data_conf_t));
    // copy msdu handle and error status
    mcps_data_conf->msduHandle = msdu_handle;
    mcps_data_conf->status = tx_error_status;

    arm_event_s event = {
        .receiver = mac_tasklet_event_handler,
        .sender = 0,
        .event_id = 0,
        .data_ptr = mcps_data_conf,
        .event_type = MCPS_SAP_DATA_CNF_FAIL_EVENT,
        .priority = ARM_LIB_HIGH_PRIORITY_EVENT,
    };

    return eventOS_event_send(&event);
}
#ifndef WISUN_RCP_ENABLE
void mcps_sap_pd_req_queue_write(protocol_interface_rf_mac_setup_s *rf_mac_setup, mac_pre_build_frame_t *buffer)
{
    if (!rf_mac_setup || !buffer) {
        return;
    }
#if 0
    if (!rf_mac_setup->active_pd_data_request) {
        // Push broadcast buffers to queue when broadcast disabled flag is set
        if ((rf_mac_setup->macBroadcastDisabled == true) && !mac_is_ack_request_set(buffer)) {
            goto push_to_queue;
        }

        if (buffer->ExtendedFrameExchange) {
            //Update here state and store peer
            memcpy(rf_mac_setup->mac_edfe_info->PeerAddr, buffer->DstAddr, 8);
            rf_mac_setup->mac_edfe_info->state = MAC_EDFE_FRAME_CONNECTING;
        }
        if (rf_mac_setup->fhss_api && (buffer->asynch_request == false)) {
            uint16_t frame_length = buffer->mac_payload_length + buffer->headerIeLength + buffer->payloadsIeLength;
            if ((mcps_check_packet_blacklist(rf_mac_setup, buffer) == true) || rf_mac_setup->fhss_api->check_tx_conditions(rf_mac_setup->fhss_api, !mac_is_ack_request_set(buffer),
                                                                                                                           buffer->msduHandle, mac_convert_frame_type_to_fhss(buffer->fcf_dsn.frametype), frame_length,
                                                                                                                           rf_mac_setup->dev_driver->phy_driver->phy_header_length, rf_mac_setup->dev_driver->phy_driver->phy_tail_length) == false) {
                if (buffer->ExtendedFrameExchange) {
                    rf_mac_setup->mac_edfe_info->state = MAC_EDFE_FRAME_IDLE;
                }
                goto push_to_queue;
            }
        }

        //Start TX process immediately
        rf_mac_setup->active_pd_data_request = buffer;
        if (mcps_pd_data_request(rf_mac_setup, buffer) != 0) {
            rf_mac_setup->mac_tx_result = MAC_TX_PRECOND_FAIL;
            rf_mac_setup->macTxRequestAck = false;
            if (buffer->ExtendedFrameExchange) {
                rf_mac_setup->mac_edfe_info->state = MAC_EDFE_FRAME_IDLE;
            }
            if (mcps_sap_pd_confirm(rf_mac_setup) != 0) {
                // can't send event, try calling error handler directly
                rf_mac_setup->mac_mcps_data_conf_fail.msduHandle = buffer->msduHandle;
                rf_mac_setup->mac_mcps_data_conf_fail.status = buffer->status;
                if (buffer->ExtendedFrameExchange) {
                    rf_mac_setup->mac_edfe_info->state = MAC_EDFE_FRAME_IDLE;
                }
                mcps_sap_prebuild_frame_buffer_free(buffer);
                rf_mac_setup->active_pd_data_request = NULL;
                mac_pd_data_confirm_failure_handle(rf_mac_setup);
            }
        }
        return;
    }
#endif
push_to_queue:
    rf_mac_setup->direct_queue_bytes += buffer->mac_payload_length;
    mac_pre_build_frame_t *prev = NULL;
    mac_pre_build_frame_t *cur = rf_mac_setup->pd_data_request_queue_to_go;
    bool use_bc_queue = false;
#if 0
    // When FHSS is enabled, broadcast buffers are pushed to own queue
    if (rf_mac_setup->fhss_api && (buffer->asynch_request == false)) {
        if (rf_mac_setup->fhss_api->use_broadcast_queue(rf_mac_setup->fhss_api, !mac_is_ack_request_set(buffer),
                                                        mac_convert_frame_type_to_fhss(buffer->fcf_dsn.frametype)) == true) {
            cur = rf_mac_setup->pd_data_request_bc_queue_to_go;
            use_bc_queue = true;
            rf_mac_setup->broadcast_queue_size++;
        }
    }
    if (use_bc_queue == false) {
        rf_mac_setup->unicast_queue_size++;
    }
    sw_mac_stats_update(rf_mac_setup, STAT_MAC_TX_QUEUE, rf_mac_setup->unicast_queue_size + rf_mac_setup->broadcast_queue_size);

    //Push to queue
    if (!cur) {
        if (rf_mac_setup->fhss_api && (use_bc_queue == true)) {
            rf_mac_setup->pd_data_request_bc_queue_to_go = buffer;
            return;
        } else {
            rf_mac_setup->pd_data_request_queue_to_go = buffer;
            return;
        }
    }
#endif
    while (cur) {
        if (cur->priority < buffer->priority) {
            //Set before cur
            if (prev) {
                prev->next = buffer;
                buffer->next = cur;
            } else {
                buffer->next = cur;
                if (rf_mac_setup->fhss_api && (use_bc_queue == true)) {
                    rf_mac_setup->pd_data_request_bc_queue_to_go = buffer;
                } else {
                    rf_mac_setup->pd_data_request_queue_to_go = buffer;
                }
            }
            cur = NULL;

        } else if (cur->next == NULL) {
            cur->next = buffer;
            cur = NULL;
        } else {
            prev = cur;
            cur = cur->next;
        }
    }
}
#endif

bool mac_is_ack_request_set(mac_pre_build_frame_t *buffer)
{
    if (buffer->fcf_dsn.ackRequested || buffer->WaitResponse) {
        return true;
    }
    return false;
}

void mcps_sap_pre_parsed_frame_buffer_free(mac_pre_parsed_frame_t *buf)
{
    if (!buf) {
        return;
    }

    if (buf->mac_class_ptr && buf->fcf_dsn.frametype == FC_ACK_FRAME) {
        struct protocol_interface_rf_mac_setup *rf_mac_setup = buf->mac_class_ptr;
        if (rf_mac_setup->rf_pd_ack_buffer_is_in_use) {
            rf_mac_setup->rf_pd_ack_buffer_is_in_use = false;
            return;
        }
    }

    ns_dyn_mem_free(buf);
}

mac_pre_parsed_frame_t *mcps_sap_pre_parsed_frame_buffer_get(const uint8_t *data_ptr, uint16_t frame_length)
{
    mac_pre_parsed_frame_t *buffer = ns_dyn_mem_temporary_alloc(sizeof(mac_pre_parsed_frame_t) + frame_length);

    if (buffer) {
        memset(buffer, 0, sizeof(mac_pre_parsed_frame_t) + frame_length);
        buffer->frameLength = frame_length;
        memcpy(mac_header_message_start_pointer(buffer), data_ptr, frame_length);
    }

    return buffer;
}

void rcp_sap_data_req_handler_ext(protocol_interface_rf_mac_setup_s *rf_mac_setup, const mcps_data_req_t *data_req, const mcps_data_req_ie_list_t *ie_list, const channel_list_s *asynch_channel_list, mac_data_priority_t priority)
{
    uint8_t status = MLME_SUCCESS;
    mac_pre_build_frame_t *buffer = NULL;
    rcp_status_t rcp_status;

    rcpSapDbg.num_sap_data_req++;

    if (rf_mac_setup->mac_edfe_enabled && data_req->ExtendedFrameExchange) {
        if (rf_mac_setup->mac_edfe_info->state != MAC_EDFE_FRAME_IDLE) {
            tr_debug("Accept only 1 active Efde Data request push");
            status = MLME_UNSUPPORTED_LEGACY;
            rcpSapDbg.num_err_edfe++;
            goto verify_status;
        }

        if (data_req->DstAddrMode != MAC_ADDR_MODE_64_BIT) {
            status = MLME_INVALID_PARAMETER;
            rcpSapDbg.num_err_DstAddrMode++;
            goto verify_status;
        }
    }

    if (!rf_mac_setup->mac_security_enabled) {
        if (data_req->Key.SecurityLevel) {
            status = MLME_UNSUPPORTED_SECURITY;
            rcpSapDbg.num_err_unknown_security++;
            goto verify_status;
        }
    }

    uint16_t ie_header_length = 0;
    uint16_t ie_payload_length = 0;
    volatile uint8_t utie_time_offset = data_req->utie_time_offset;
    volatile uint8_t btie_time_offset = data_req->btie_time_offset;

    if (!mac_ie_vector_length_validate(ie_list->headerIeVectorList, ie_list->headerIovLength, &ie_header_length)) {
        status = MLME_INVALID_PARAMETER;
        rcpSapDbg.num_err_headerIE++;
        goto verify_status;
    }

    if (!mac_ie_vector_length_validate(ie_list->payloadIeVectorList, ie_list->payloadIovLength, &ie_payload_length)) {
        status = MLME_INVALID_PARAMETER;
        rcpSapDbg.num_err_payloadIE++;
        goto verify_status;
    }

    if ((ie_header_length || ie_payload_length || asynch_channel_list) && !rf_mac_setup->mac_extension_enabled) {
        //Report error when feature is not enaled yet
        status = MLME_INVALID_PARAMETER;
        rcpSapDbg.num_err_mlme_para++;
        goto verify_status;
    } else if (asynch_channel_list && data_req->TxAckReq) {
        //Report Asynch Message is not allowed to call with ACK requested.
        status = MLME_INVALID_PARAMETER;
        rcpSapDbg.num_err_async_para++;
        goto verify_status;
    }

    if ((data_req->msduLength + ie_header_length + ie_payload_length) > rf_mac_setup->phy_mtu_size - MAC_DATA_PACKET_MIN_HEADER_LENGTH) {
        tr_debug("packet %u, %u", data_req->msduLength, rf_mac_setup->phy_mtu_size);
        status = MLME_FRAME_TOO_LONG;
        rcpSapDbg.num_err_msduLen++;
        goto verify_status;
    }
    buffer = mcps_sap_prebuild_frame_buffer_get(0);
    //tr_debug("Data Req");
    if (!buffer) {
        //Make Confirm Here
        status = MLME_TRANSACTION_OVERFLOW;
        rcpSapDbg.num_err_mlme_transaction_overflow++;
        goto verify_status;
    }

    if (!rf_mac_setup->macUpState ) {
        status = MLME_TRX_OFF;
        rcpSapDbg.num_err_macState++;
        goto verify_status;
    }

    if (asynch_channel_list) {
        //Copy Asynch data list
        buffer->asynch_channel_list = *asynch_channel_list;
        buffer->asynch_request = true;
    }

    //Set Priority level
    switch (priority) {
        case MAC_DATA_EXPEDITE_FORWARD:
            buffer->priority = MAC_PD_DATA_EF_PRIORITY;
            // Enable FHSS expedited forwarding
            if (rf_mac_setup->fhss_api) {
                rf_mac_setup->fhss_api->synch_state_set(rf_mac_setup->fhss_api, FHSS_EXPEDITED_FORWARDING, 0);
            }
            break;
        case MAC_DATA_HIGH_PRIORITY:
            buffer->priority = MAC_PD_DATA_HIGH_PRIOTITY;
            break;
        case MAC_DATA_MEDIUM_PRIORITY:
            buffer->priority = MAC_PD_DATA_MEDIUM_PRIORITY;
            break;
        default:
            buffer->priority = MAC_PD_DATA_NORMAL_PRIORITY;
            break;
    }


    buffer->upper_layer_request = true;
    buffer->fcf_dsn.frametype = FC_DATA_FRAME;
    buffer->ExtendedFrameExchange = data_req->ExtendedFrameExchange;
    buffer->WaitResponse = data_req->TxAckReq;
    if (data_req->ExtendedFrameExchange) {
        buffer->fcf_dsn.ackRequested = false;
    } else {
        buffer->fcf_dsn.ackRequested = data_req->TxAckReq;
    }

    buffer->mac_header_length_with_security = 3;
    mac_header_security_parameter_set(&buffer->aux_header, &data_req->Key);
    buffer->security_mic_len = mac_security_mic_length_get(buffer->aux_header.securityLevel);
    if (buffer->aux_header.securityLevel) {
        buffer->fcf_dsn.frameVersion = MAC_FRAME_VERSION_2006;
        buffer->fcf_dsn.securityEnabled = true;
    }

    buffer->mac_header_length_with_security += mac_header_security_aux_header_length(buffer->aux_header.securityLevel, buffer->aux_header.KeyIdMode);

    buffer->msduHandle = data_req->msduHandle;
    buffer->fcf_dsn.DstAddrMode = data_req->DstAddrMode;
    memcpy(buffer->DstAddr, data_req->DstAddr, 8);
    buffer->DstPANId = data_req->DstPANId;
    buffer->SrcPANId = mac_mlme_get_panid(rf_mac_setup);
    buffer->fcf_dsn.SrcAddrMode = data_req->SrcAddrMode;
    buffer->fcf_dsn.framePending = data_req->PendingBit;

    if (buffer->fcf_dsn.SrcAddrMode == MAC_ADDR_MODE_NONE && !rf_mac_setup->mac_extension_enabled) {
        if (buffer->fcf_dsn.DstAddrMode == MAC_ADDR_MODE_NONE) {
            status = MLME_INVALID_ADDRESS;
            rcpSapDbg.num_err_SrcAddrMode++;
            goto verify_status;
        }

        if (rf_mac_setup->shortAdressValid) {
            buffer->fcf_dsn.SrcAddrMode = MAC_ADDR_MODE_16_BIT;
        } else {
            buffer->fcf_dsn.SrcAddrMode = MAC_ADDR_MODE_64_BIT;
        }
    }

    mac_frame_src_address_set_from_interface(buffer->fcf_dsn.SrcAddrMode, rf_mac_setup, buffer->SrcAddr);

    buffer->ie_elements.headerIeVectorList = ie_list->headerIeVectorList;
    buffer->ie_elements.headerIovLength = ie_list->headerIovLength;
    buffer->ie_elements.payloadIeVectorList = ie_list->payloadIeVectorList;
    buffer->ie_elements.payloadIovLength = ie_list->payloadIovLength;
    buffer->headerIeLength = ie_header_length;
    buffer->payloadsIeLength = ie_payload_length;


    if (rf_mac_setup->mac_extension_enabled) {
        //Handle mac extension's
        buffer->fcf_dsn.frameVersion = MAC_FRAME_VERSION_2015;
        if (ie_header_length || ie_payload_length) {
            buffer->fcf_dsn.informationElementsPresets = true;
        }

        buffer->fcf_dsn.sequenceNumberSuppress = data_req->SeqNumSuppressed;
        if (buffer->fcf_dsn.sequenceNumberSuppress) {
            buffer->mac_header_length_with_security--;
        }
        /* PAN-ID compression bit enable when necessary */
        if (buffer->fcf_dsn.SrcAddrMode == MAC_ADDR_MODE_NONE && buffer->fcf_dsn.DstAddrMode == MAC_ADDR_MODE_NONE) {
            buffer->fcf_dsn.intraPan = !data_req->PanIdSuppressed;
        } else if (buffer->fcf_dsn.DstAddrMode == MAC_ADDR_MODE_NONE) {
            buffer->fcf_dsn.intraPan = data_req->PanIdSuppressed;
        } else if (buffer->fcf_dsn.SrcAddrMode == MAC_ADDR_MODE_NONE || (buffer->fcf_dsn.SrcAddrMode == MAC_ADDR_MODE_64_BIT && buffer->fcf_dsn.DstAddrMode == MAC_ADDR_MODE_64_BIT)) {
            buffer->fcf_dsn.intraPan = data_req->PanIdSuppressed;
        } else { /* two addresses, at least one address short */
            // ignore or fault panidsuppressed
            if (buffer->DstPANId == buffer->SrcPANId) {
                buffer->fcf_dsn.intraPan = true;
            }
        }
    } else {
        /* PAN-ID compression bit enable when necessary */
        if ((buffer->fcf_dsn.DstAddrMode && buffer->fcf_dsn.SrcAddrMode) && (buffer->DstPANId == buffer->SrcPANId)) {
            buffer->fcf_dsn.intraPan = true;
        }
    }

    //Check PanID presents at header
    buffer->fcf_dsn.DstPanPresents = mac_dst_panid_present(&buffer->fcf_dsn);
    buffer->fcf_dsn.SrcPanPresents = mac_src_panid_present(&buffer->fcf_dsn);
    //Calculate address length
    buffer->mac_header_length_with_security += mac_header_address_length(&buffer->fcf_dsn);
    buffer->mac_payload = data_req->msdu;
    buffer->mac_payload_length = data_req->msduLength;
#ifndef WISUN_RCP_ENABLE
    buffer->cca_request_restart_cnt = rf_mac_setup->cca_failure_restart_max;
    // Multiply number of backoffs for higher priority packets
    if (buffer->priority == MAC_PD_DATA_EF_PRIORITY) {
        buffer->cca_request_restart_cnt *= MAC_PRIORITY_EF_BACKOFF_MULTIPLIER;
    }
    buffer->tx_request_restart_cnt = rf_mac_setup->tx_failure_restart_max;
#endif
    //Start TX process immediately
    rf_mac_setup->active_pd_data_request = buffer;

#ifdef WISUN_USE_NANOSTACK_FHSS_MAC_MODULE
    rf_mac_setup->mac_tx_start_channel = rf_mac_setup->mac_channel;
    mac_csma_param_init(rf_mac_setup);
#endif
    // update UTIE or BTIE time offset
    if (utie_time_offset)
    {
        utie_time_offset += buffer->mac_header_length_with_security;
    }

    if (btie_time_offset)
    {
        btie_time_offset += buffer->mac_header_length_with_security;
    }

    uint8_t frame_count_offset;
    if (mcps_generic_packet_build(rf_mac_setup, buffer, &frame_count_offset) != 0) {
        rcpSapDbg.num_err_packet_build++;
        status = MLME_PACKET_BUILD_ERROR;
        goto verify_status;
    }

    uint8_t *frame = rf_mac_setup->dev_driver_tx_buffer.buf;
    int frame_len = rf_mac_setup->dev_driver_tx_buffer.len;

    // tr_info("Frame Len: %d", frame_len);
    // tr_info("Frame Counter: %lu", buffer->aux_header.frameCounter);
    // tr_info("Frame Count Offset: %d", frame_count_offset);

    rcp_data_req_type data_req_type = RCP_DATA_REQ_ASYNC;
    // Note: asynch_channel_list->channel_mask currently unused. May want to add back?
    if (asynch_channel_list != NULL) {
        data_req_type = RCP_DATA_REQ_ASYNC;
    } else {
        if (data_req->TxAckReq) {
            data_req_type = RCP_DATA_REQ_UNICAST;
        } else {
            data_req_type = RCP_DATA_REQ_BROADCAST;
        }
    }

    fhnt_entry_t data_req_fhnt_entry;

    memset(&data_req_fhnt_entry, 0, sizeof (data_req_fhnt_entry));
    if (data_req_type == RCP_DATA_REQ_UNICAST)
    {
        llc_neighbour_req_t neighbor_buffer;
        neighbor_buffer.neighbor = NULL;
        neighbor_buffer.ws_neighbor = NULL;

        protocol_interface_info_entry_t *interface;
        interface = protocol_stack_interface_info_get_by_fhss_api(rf_mac_setup->fhss_api);

        if (interface == NULL) {
            rcpSapDbg.num_err_interface_not_found++;
            status = MLME_PACKET_BUILD_ERROR;
            goto verify_status;
        }
        neighbor_buffer.neighbor = mac_neighbor_table_address_discover(mac_neighbor_info(interface), data_req->DstAddr, ADDR_802_15_4_LONG);
        if (neighbor_buffer.neighbor) {
            neighbor_buffer.ws_neighbor = ws_neighbor_class_entry_get(&interface->ws_info->neighbor_storage, neighbor_buffer.neighbor->index);
        }

        if((neighbor_buffer.neighbor == NULL) || (neighbor_buffer.ws_neighbor == NULL)){
            //couldnt find the neighbor info
            rcpSapDbg.num_err_nbr_table_info_not_found++;
            if (data_req->mpx_id == MPX_KEY_MANAGEMENT_ENC_USER_ID) {
                rcpSapDbg.num_err_nbr_table_info_not_found_eapol++;
            } else if (data_req->mpx_id == MPX_LOWPAN_ENC_USER_ID) {
                rcpSapDbg.num_err_nbr_table_info_not_found_6lowpan++;
            }

            status = MLME_NO_NEIGHBOR_DATA;
            goto verify_status;
        }

        data_req_fhnt_entry.ufsi = neighbor_buffer.ws_neighbor->fhss_data.uc_timing_info.ufsi;
        data_req_fhnt_entry.ref_timeStamp = neighbor_buffer.ws_neighbor->fhss_data.uc_timing_info.utt_rx_timestamp;
        data_req_fhnt_entry.dwellInterval = neighbor_buffer.ws_neighbor->fhss_data.uc_timing_info.unicast_dwell_interval;
        data_req_fhnt_entry.channelFunc = neighbor_buffer.ws_neighbor->fhss_data.uc_timing_info.unicast_channel_function;
        // LMAC doesn't need the clockDrift and timingAccuracy
        if ( data_req_fhnt_entry.channelFunc == 0)
        {   // fixed channel
            data_req_fhnt_entry.fixedChannel = neighbor_buffer.ws_neighbor->fhss_data.uc_timing_info.fixed_channel;
            data_req_fhnt_entry.numChannels = 1;
        }
        else
        {   // hopping channel mask, LMAC only requires the excluded channel list
            uint8_t i=0,mask;
            for (i=0; i < MAC_154G_CHANNEL_BITMAP_SIZ;i++)
            {   // nanoStack provides the active channel, we need to convert to exclude list
                // fhss_data.uc_channel_list.channel_mask is uint32
                mask = (neighbor_buffer.ws_neighbor->fhss_data.uc_channel_list.channel_mask2[i]);
                // flip over the bit, convert into the excluded list
                mask = ~mask;
                data_req_fhnt_entry.bitMap[i] = mask;
            }
            data_req_fhnt_entry.numChannels = neighbor_buffer.ws_neighbor->fhss_data.uc_channel_list.channel_count;
        }
        // save the destination MAC address in NT entry
        memcpy(data_req_fhnt_entry.extAddr, data_req->DstAddr, SADDR_EXT_LEN);
    }

    // RCP_ERR_DATA_REQ_REJECT return value handling not needed. Handled by data cnf message.
    rcp_status = rcp_host_data_req(data_req_type, frame, frame_len, data_req->msduHandle, (mlme_security_t *) &data_req->Key, buffer->mac_header_length_with_security,
                         buffer->aux_header.frameCounter, frame_count_offset, utie_time_offset, btie_time_offset, &data_req_fhnt_entry);
    if ( rcp_status != RCP_SUCCESS )
    {
        status = MLME_BUSY_TX;
        rcpSapDbg.num_err_tx_busy++;
    }

verify_status:
    // free the MAC TX
    mcps_sap_prebuild_frame_buffer_free(buffer);
    if (status != MLME_SUCCESS) {
        tr_debug("DATA REQ msdu (%d) Fail %u", data_req->msduHandle,status);
        if (mcps_sap_pd_confirm_failure_event(data_req->msduHandle,status) !=0)
        {
            rcpSapDbg.num_err_post_cnf_failure++;
        }
    }
}

static uint8_t sec_frame_count_validate(protocol_interface_rf_mac_setup_s *rf_mac_setup, mac_pre_parsed_frame_t *b,
                                        mlme_security_t *security_params, uint8_t src_addr[8])
{
    protocol_interface_info_entry_t *interface = protocol_stack_interface_info_get_by_fhss_api(rf_mac_setup->fhss_api);
    mac_neighbor_table_entry_t *neighbor_entry = NULL;
    uint32_t rx_frame_counter = mcps_mac_security_frame_counter_read(b);
    uint32_t min_accepted_frame_counter = 0;

    if (interface == NULL) {
        tr_err("Could not find interface in sec frame count validate");
        return MLME_COUNTER_ERROR;
    }
    if (rx_frame_counter == 0xffffffff) {
        tr_debug("Max Framecounter value reached. Drop.");
        return MLME_COUNTER_ERROR;
    }

    neighbor_entry = mac_neighbor_table_address_discover(mac_neighbor_info(interface), src_addr, ADDR_802_15_4_LONG);
    if(neighbor_entry == NULL) {
        // If entry cannot be found, assume first entry and pass up to LLC
        return MLME_SUCCESS;
    }

    min_accepted_frame_counter = neighbor_entry->frame_count[security_params->KeyIndex - 1];
    if (rx_frame_counter < min_accepted_frame_counter) {
        tr_debug("RX frame counter less than min accepted frame count. Drop.");
        return MLME_COUNTER_ERROR;
    }

    // Increment accepted frame count
    neighbor_entry->frame_count[security_params->KeyIndex - 1] = rx_frame_counter++;

    return MLME_SUCCESS;
}

int8_t rcp_data_sap_rx_handler(mac_pre_parsed_frame_t *buf, protocol_interface_rf_mac_setup_s *rf_mac_setup, mac_api_t *mac)
{
    // note: very identical to mac_data_sap_rx_handler;
    //major difference: no need to decrypt packet ; we get decrypted packet from lmac

    int8_t retval = -1;
    uint8_t status;

    //allocate Data ind primitiv and parse packet to that
    mcps_data_ind_t *data_ind = ns_dyn_mem_temporary_alloc(sizeof(mcps_data_ind_t));

    if (!data_ind) {
        goto DROP_PACKET;
    }
    memset(data_ind, 0, sizeof(mcps_data_ind_t));


    //Parse data
    data_ind->DSN = buf->fcf_dsn.DSN;
    data_ind->DSN_suppressed = buf->fcf_dsn.sequenceNumberSuppress;
    data_ind->DstAddrMode = buf->fcf_dsn.DstAddrMode;
    mac_header_get_dst_address(&buf->fcf_dsn, mac_header_message_start_pointer(buf), data_ind->DstAddr);
    data_ind->SrcAddrMode = buf->fcf_dsn.SrcAddrMode;

    mac_header_get_src_address(&buf->fcf_dsn, mac_header_message_start_pointer(buf), data_ind->SrcAddr);

    data_ind->SrcPANId = mac_header_get_src_panid(&buf->fcf_dsn, mac_header_message_start_pointer(buf), rf_mac_setup->pan_id);
    data_ind->DstPANId = mac_header_get_dst_panid(&buf->fcf_dsn, mac_header_message_start_pointer(buf), rf_mac_setup->pan_id);

    data_ind->mpduLinkQuality = buf->LQI;
    data_ind->signal_dbm = buf->dbm;
    data_ind->timestamp = buf->timestamp;

    /* Parse security part */
    mac_header_security_components_read(buf, &data_ind->Key);

    //TODO: evaluate EDFE mode in split mac architecture
    // if (data_ind->SrcAddrMode == MAC_ADDR_MODE_NONE && rf_mac_setup->mac_edfe_enabled && rf_mac_setup->mac_edfe_info->state > MAC_EDFE_FRAME_CONNECTING) {
    //     memcpy(data_ind->SrcAddr, rf_mac_setup->mac_edfe_info->PeerAddr, 8);
    //     data_ind->SrcAddrMode = MAC_ADDR_MODE_64_BIT;
    // }

    //buf->neigh_info = mac_sec_mib_device_description_get(rf_mac_setup, data_ind->SrcAddr, data_ind->SrcAddrMode, data_ind->SrcPANId); //revisit

    if (buf->fcf_dsn.securityEnabled) {
        status = sec_frame_count_validate(rf_mac_setup, buf, &data_ind->Key, data_ind->SrcAddr);
        if (status != MLME_SUCCESS) {
            goto DROP_PACKET;
        }
    }

    /* address filtering enabled, using allowlist
    * if match is not found, do not allow packet */
    if(!mac_filter_list_allow_packet(data_ind->SrcAddr)) {
        goto DROP_PACKET;
    }

    if (!mac_payload_information_elements_parse(buf)) {
        goto DROP_PACKET;
    }
    data_ind->msduLength = buf->mac_payload_length;
    data_ind->msdu_ptr = buf->macPayloadPtr;

    // /* Validate Polling device */
    // if (!rf_mac_setup->macCapRxOnIdle) {
    //     if (mac_data_interface_host_accept_data(data_ind, rf_mac_setup) != 0) {
    //         //tr_debug("Drop by not Accept");
    //         goto DROP_PACKET;
    //     }
    // }

    if (mac) {

        if (buf->fcf_dsn.frameVersion == MAC_FRAME_VERSION_2015) {
            if (!rf_mac_setup->mac_extension_enabled) {
                goto DROP_PACKET;
            }
            mcps_data_ie_list_t ie_list;
            ie_list.payloadIeList = buf->payloadsIePtr;
            ie_list.payloadIeListLength = buf->payloadsIeLength;
            ie_list.headerIeList = buf->headerIePtr;
            ie_list.headerIeListLength = buf->headerIeLength;
            //Swap compressed address to broadcast when dst Address is elided
            if (buf->fcf_dsn.DstAddrMode == MAC_ADDR_MODE_NONE) {
                data_ind->DstAddrMode = MAC_ADDR_MODE_16_BIT;
                data_ind->DstAddr[0] = 0xff;
                data_ind->DstAddr[1] = 0xff;
            }
            mac->data_ind_ext_cb(mac, data_ind, &ie_list);

        } else {
            mac->data_ind_cb(mac, data_ind);
        }
        retval = 0;
    }

DROP_PACKET:
    ns_dyn_mem_free(data_ind);
    // mcps_sap_pre_parsed_frame_buffer_free(buf);
    return retval;
}
