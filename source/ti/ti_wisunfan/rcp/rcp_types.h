/*
 * Copyright (c) 2024, Texas Instruments Incorporated
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
 *  ======== rcp_types.h ========
 */

#ifndef RCP_TYPES_H_
#define RCP_TYPES_H_

#include <stddef.h>
#include <stdint.h>
#include "saddr.h"
#include "mac_mcps.h"
#include "fhss_ws_extension.h"
#include "mlme_primitives.h"
#include "rcp_mac_common.h"

#define INVALID_KEY_INDEX 0xFF

/* RCP return status */
typedef enum {
     RCP_SUCCESS                  = 0x00,
     RCP_ERR_DATA_REQ_ALLOC_FAIL  = 0x01,
     RCP_ERR_DATA_REQ_REJECT      = 0x02,
     RCP_ERR_MLME_SET             = 0x03,
} rcp_status_t;

typedef enum {
    /* MAC PIB config: 0x1xxx */
    RCP_CONFIG_EXT_ADDR = macExtendedAddress,
    RCP_CONFIG_PHY_DESCRIPTOR_ID = macPhyCurrentDescriptorId,
    RCP_CONFIG_REG_DOMAIN = macRegDomain,
    RCP_CONFIG_CHANNEL_PAGE = macChannelPage,
    RCP_CONFIG_TX_POWER = macPhyTransmitPowerSigned,
    RCP_CONFIG_PAN_ID = macPANId,
    RCP_CONFIG_RX_ON_WHEN_IDLE = macRxOnWhenIdle,

#if defined(MAC_DUTY_CYCLE_CHECKING) || defined(MAC_OVERRIDE_TX_DELAY)
    RCP_CONFIG_DUTY_CYCLE_ENABLE = macDutyCycleEnable,
    RCP_CONFIG_DUTY_CYCLE_REGULATED = macDutyCycleRegulated,
    RCP_CONFIG_CUSTOM_MIN_TX_OFF_ENABLE = macCustomMinTxOffEnabled,
    RCP_CONFIG_CUSTOM_MIN_TX_OFF_TIME = macMinTxOffTime,
#endif

    /* Other Config: 0x1xxxx */
    RCP_CONFIG_RCP_VERSION = 0x1000,
    RCP_CONFIG_ASYNC_CHANNEL_MASK = 0x1001,
    RCP_CONFIG_BC_TIMING_INFO = 0x1002,
    RCP_CONFIG_SEC_KEY = 0x1003,
    RCP_CONFIG_SEC_FRAME_COUNT = 0x1004,
    RCP_CONFIG_PHY_INIT = 0x1005,

    /* FH PIB Config: 0x2xxx */
    // RCP_CONFIG_TRACK_PARENT_EUI = MAC_FHPIB_TRACK_PARENT_EUI,
    RCP_CONFIG_BC_INTERVAL = MAC_FHPIB_BC_INTERVAL,
    RCP_CONFIG_UC_EXCLUDED_CHANNELS = MAC_FHPIB_UC_EXCLUDED_CHANNELS,
    RCP_CONFIG_BC_EXCLUDED_CHANNELS = MAC_FHPIB_BC_EXCLUDED_CHANNELS,
    RCP_CONFIG_UC_DWELL_INTERVAL = MAC_FHPIB_UC_DWELL_INTERVAL,
    RCP_CONFIG_BC_DWELL_INTERVAL = MAC_FHPIB_BC_DWELL_INTERVAL,
    // RCP_CONFIG_CLOCK_DRIFT = MAC_FHPIB_CLOCK_DRIFT,
    // RCP_CONFIG_TIMING_ACCURACY = MAC_FHPIB_TIMING_ACCURACY,
    RCP_CONFIG_UC_CHANNEL_FUNC = MAC_FHPIB_UC_CHANNEL_FUNCTION,
    RCP_CONFIG_BC_CHANNEL_FUNC = MAC_FHPIB_BC_CHANNEL_FUNCTION,
    RCP_CONFIG_USE_PARENT_BS_IE = MAC_FHPIB_USE_PARENT_BS_IE,
    RCP_CONFIG_BROCAST_SCHED_ID = MAC_FHPIB_BROCAST_SCHED_ID,
    RCP_CONFIG_UC_FIXED_CHANNEL = MAC_FHPIB_UC_FIXED_CHANNEL,
    RCP_CONFIG_BC_FIXED_CHANNEL = MAC_FHPIB_BC_FIXED_CHANNEL,

} rcp_config_attr_t;

/* RCP Host debug count
*/
typedef struct _rcp_host_dbg
{
    uint32_t num_tx;
    uint32_t num_config_tx;
    uint16_t num_config_rcp_init;
    uint16_t num_config_mac_init;
    uint16_t num_config_fh_init;
    uint16_t num_config_mac_reset;
    uint16_t num_config_set_config;

    uint32_t num_data_tx;
    uint32_t num_tx_cnf;
    uint16_t num_tx_cnf_reject;
    uint16_t num_tx_cnf_alloc_err;
    uint16_t num_tx_cnf_failure;

    uint32_t num_tx_async;
    uint32_t num_tx_cnf_async;
    uint32_t num_tx_cnf_async_reject;
    uint16_t num_tx_cnf_async_alloc_err;
    uint16_t num_tx_cnf_async_failure;

    uint16_t num_tx_unicast;
    uint16_t num_tx_cnf_unicast;
    uint16_t num_tx_cnf_unicast_reject;
    uint16_t num_tx_cnf_unicast_alloc_err;
    uint16_t num_tx_cnf_unicast_failure;
    uint16_t num_tx_conf_Txqueue_lmac;
    uint16_t num_tx_conf_Txqueue_err;

    uint32_t num_tx_broadcast;
    uint32_t num_tx_cnf_broadcast;
    uint16_t num_tx_cnf_broadcast_reject;
    uint16_t num_tx_cnf_broadcast_alloc_err;
    uint16_t num_tx_cnf_broadcast_failure;

    uint32_t num_rx_ind;
    uint16_t num_err_mem_alloc_fail_data_ind;
    uint16_t num_err_mem_alloc_fail_tx_conf;
    uint16_t num_err_wrong_type;
    uint16_t num_err_data_cnf_no_key;

    // RCP BC schedule debugging
    uint16_t num_err_uc_no_ack;
    uint16_t num_err_uc_ack_parse_err_len;
    uint16_t num_err_uc_ack_parse_err_ie;

} RCP_HOST_DBG_s;

typedef struct {
    uint32_t num_tx_async;
    uint16_t num_tx_unicast;
    uint32_t num_tx_broadcast;

    uint32_t num_rx_secure_pkt;
    uint32_t num_rx_err_key_idmode;
    uint32_t num_rx_err_decrypt;
    uint32_t num_tx_ack_no_secure;
    uint32_t num_tx_ack_secure;
    uint16_t num_err_mem_alloc_fail_data_req;
    uint16_t num_err_wrong_type;
    uint16_t num_err_mem_alloc_fail;

    // RCP BC schedule debugging
    uint16_t num_rcp_set_parent;
    uint16_t num_forch_synch;

    uint16_t num_rx_edfe_frm;
    uint16_t num_rx_edfe_ffrm;
    uint16_t num_tx_edfe_ffrm;
    uint16_t num_edfe_rcp_cb;
    uint16_t num_edfe_rcp_cb_ok;
    uint16_t num_err_tx_edfe_time;
    uint16_t num_err_tx_edfe_frm;
    uint16_t num_err_edfe_tx_fail;
} rcp_lmac_dbg_t;


typedef struct {    
    //generic errors
    uint32_t num_err_mt_msg_send_failure;
    uint32_t num_err_mt_msg_parse_err;
    uint32_t num_err_bad_tx_len;

    //Host to LMAC path
    uint32_t num_rcp_init_req_sent;
    uint32_t num_mac_init_req_sent;
    uint32_t num_fh_init_req_sent;
    uint32_t num_mac_reset_req_sent;
    uint32_t num_mac_cfg_get_req_sent;
    uint32_t num_mac_cfg_set_req_sent;
    uint32_t num_data_req_sent;

    //cnf from LMAC to Host path
    uint32_t num_data_ind_rxed;
    uint32_t num_data_cnf_rxed;
    uint32_t num_rcp_init_cnf_rxed;
    uint32_t num_mac_init_cnf_rxed;
    uint32_t num_fh_init_cnf_rxed;
    uint32_t num_mac_reset_cnf_rxed;
    uint32_t num_mac_cfg_get_cnf_rxed;
    uint32_t num_mac_cfg_set_cnf_rxed;

    uint32_t num_uart_bytes_txed;
    uint32_t num_uart_bytes_rxed;
}MT_rcp_Host_dbg_t;

typedef struct {  
    
    //generic catch all errors
    uint32_t num_err_mt_msg_send_failure;
    uint32_t num_err_mt_msg_parse_err;

    //Host to LMAC path
    uint32_t num_rcp_init_req_rxed;
    uint32_t num_mac_init_req_rxed;
    uint32_t num_fh_init_req_rxed;
    uint32_t num_mac_reset_req_rxed;
    uint32_t num_mac_cfg_get_req_rxed;
    uint32_t num_mac_cfg_set_req_rxed;
    uint32_t num_data_req_rxed;    

    //cnf from LMAC to Host path
    uint32_t num_data_ind_sent;
    uint32_t num_data_cnf_sent;
    uint32_t num_data_cnf_async_sent;
    uint32_t num_data_cnf_bc_sent;
    uint32_t num_data_cnf_uc_sent;

    uint32_t num_rcp_init_cnf_sent;
    uint32_t num_mac_init_cnf_sent;
    uint32_t num_fh_init_cnf_sent;
    uint32_t num_mac_reset_cnf_sent;
    uint32_t num_mac_cfg_get_cnf_sent;
    uint32_t num_mac_cfg_set_cnf_sent;

    uint32_t num_mt_ext_msg_sent;

    uint32_t num_npi_wrong_state1;
    uint32_t num_npi_wrong_state2;
    uint32_t num_npi_wrong_state3;
    uint32_t num_npi_length_error;
    uint32_t num_npi_alloc_error;

    uint32_t num_uart_bytes_txed;
    uint32_t num_uart_bytes_rxed;
} MT_rcp_LMAC_dbg_t;
  

/* MAC RCP tasklet debug count */
typedef struct __mac_rcp_dbg_
{
    // RCP incoming messages
    uint16_t num_start_cnf;
    uint16_t num_comm_status;
    uint16_t num_async_ind;
    uint16_t num_purge_cnf;
    uint32_t num_async_cnf;
    uint32_t num_data_cnf;
    uint32_t num_data_cnf_unicast;
    uint32_t num_data_cnf_broad_cast;
    uint32_t num_data_ind;
} MAC_RCP_Tasklet_DBG_s;

typedef struct rcp_btie_debug_s {
    uint32_t btieMinDelay;
    uint32_t btieMaxDelay;
    uint32_t btieDelay;
    uint32_t last_rx_btie_TimeStamp;
    uint32_t curTimeStamp;
    uint32_t rx_btie_TimeStamp;
    uint32_t rx_bfio;
    uint32_t rx_bfio_update;
    uint16_t num_bfio_error_long_delay;
    uint16_t num_bfio_need_udate;
    uint16_t num_bfio_udate_positive;
    uint16_t num_bfio_udate_negative;
} rcp_btie_debug_t;

typedef struct {
  uint8_t fskModScheme;                 /* 0: 2-FSK/2-GFSK; 1: 4-FSK/4-GFSK */
  uint16_t symbolRate;                   /* Symbol rate selection */
  uint8_t fskModIndex;                  /* Modulation index as a value encoded in
                                       * MR-FSK Generic PHY Descriptor IE
                                       * (IEEE802.15.4g section 5.2.4.20c).
                                       *
                                       * 2FSK MI = 0.25 + Modulation Index * 0.05
                                       * 4FSK MI is a third of 2FSK MI
                                       */
  uint8_t ccaType;                      /* Channel clearance algorithm selection */
} phyIDTableEntry_t;

typedef enum {
    RCP_INIT,
    RCP_MAC_INIT,
    RCP_FH_INIT,
    RCP_MAC_RESET,
    RCP_MAC_CONFIG_GET,
    RCP_MAC_CONFIG_SET,
    RCP_DATA_REQ,
} rcp_host_cmd_types;

typedef enum {
    RCP_DATA_IND,
    RCP_DATA_CNF,
    RCP_INIT_CNF,
    RCP_MAC_INIT_CNF,
    RCP_FH_INIT_CNF,
    RCP_MAC_RESET_CNF,
    RCP_MAC_CONFIG_GET_CNF,
    RCP_MAC_CONFIG_SET_CNF
} rcp_lmac_cmd_types;

typedef enum {
    RCP_DATA_REQ_ASYNC,
    RCP_DATA_REQ_UNICAST,
    RCP_DATA_REQ_BROADCAST,
} rcp_data_req_type;

typedef enum {
    RCP_DATA_CNF_ASYNC,
    RCP_DATA_CNF_UNICAST,
    RCP_DATA_CNF_BROADCAST,
} rcp_data_cnf_type;

typedef struct __attribute__((__packed__)) {
    uint8_t req_type;
    uint8_t msdu_handle;
    uint32_t min_tx_frame_count; // Needed for NV restore
    uint8_t frame_count_offset; // Offset to frame count in data_ptr
    mlme_security_t sec;
    uint16_t mdata_offset; // Offset to encrypted data in data_ptr
    uint16_t utie_offset;
    uint16_t btie_offset;
    fhnt_entry_t fhnt_entry;
    uint16_t data_len;
    uint8_t *data_ptr;
} rcp_data_req_t;


typedef struct __attribute__((__packed__)) {
    uint8_t attr;
} rcp_mac_config_get_t;

typedef struct __attribute__((__packed__)) {
    uint16_t attr;
    uint16_t val_len;
    uint8_t *val;
} rcp_mac_config_set_t;

typedef struct { // Not packing this struct due to unaligned pointer warnings
    broadcast_timing_info_t bc_timing_info;
    uint8_t force_sync;
} rcp_mac_config_bc_timing_t;

typedef struct { // Not packing this struct due to unaligned pointer warnings
    uint8_t key_index;
    uint8_t key[16];
} rcp_mac_config_sec_key_t;

typedef struct { // Not packing this struct due to unaligned pointer warnings
    uint8_t key_index;
    uint32_t frame_count;
} rcp_mac_config_sec_frame_count_t;

typedef struct {
    bool ffd;
    uint8_t phy_id;
    uint16_t config_channel_spacing;
    uint32_t ch0_center_frequency; //in KHz
    uint8_t config_number_of_channels;
    // See phyIDTableEntry_t for details on below parameters
    uint8_t mod_scheme;
    uint16_t symbol_rate;
    uint8_t mod_index;
    uint8_t cca_type;
} rcp_mac_config_phy_init;

typedef struct __attribute__((__packed__)) {
    uint8_t cnf_type; 
    uint8_t msdu_handle;
    uint8_t status; //for the TX command
    uint32_t tx_timestamp;     /** check if this is RX-ACK or TX timestamp */
    uint32_t rx_ack_timestamp;
    //Non-standard extension
    uint8_t cca_retries;    /**< Number of CCA retries used during sending */
    uint8_t tx_retries;     /**< Number of retries done during sending, 0 means no retries */
    uint32_t tx_frame_count; /* Frame counter value used (if any) for the transmitted frame */ //for async last used framecounter will be sent
    uint8_t key_index;      // key used to encrypt out-going frame

    //related to ack
    uint16_t ack_frame_len;
    uint8_t *ack_frame;
    uint8_t *buf_free;
} rcp_data_cnf_t;

typedef struct __attribute__((__packed__)) {
    uint32_t rx_timestamp;
    uint8_t link_quality;    /**< LQI value measured during reception of the MPDU */
    int8_t signal_dbm;          /**< This extension for normal IEEE 802.15.4 Data indication */ 
    uint16_t data_len;
    uint8_t *data_ptr;
    uint8_t *buf_free;
} rcp_data_ind_t;

typedef struct __attribute__((__packed__)) {
    uint8_t device_ext_addr[MAC_ADDR_LEN];
} rcp_init_cnf_t;

typedef struct __attribute__((__packed__)) {
    uint16_t attr;
    uint8_t status; // RCP_SUCCESS or RCP_FAILURE
    uint16_t val_len;
    uint8_t *val; // values valid on if RCP_SUCCESS
} rcp_mac_config_get_cnf_t;
 
typedef struct __attribute__((__packed__)) {
    uint16_t attr;
    uint8_t status; // RCP_SUCCESS or RCP_FAILURE
    uint16_t val_len;
    uint8_t *val;
} rcp_mac_config_set_cnf_t;

typedef struct __attribute__((__packed__)) {
    uint8_t rcp_cmd_type;
    uint8_t *rcp_data;
} rcp_cmd_t;

#endif //RCP_TYPES_H_
