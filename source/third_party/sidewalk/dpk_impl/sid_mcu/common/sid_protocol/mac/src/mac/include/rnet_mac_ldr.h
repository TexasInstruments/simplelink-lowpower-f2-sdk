/*
 * Copyright 2019-2020 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.txt file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef _H_SRC_MAC_HALO_INCLUDE_RNET_MAC_LDR_H
#define _H_SRC_MAC_HALO_INCLUDE_RNET_MAC_LDR_H

#include "rnet_nw_mac_ifc.h"
#include "rnet_frame_manager.h"
#include "rnet_mac_submodule_common.h"
#include <rnet_ll_phy_interface.h>
#include "rnet_mac_util.h"
#include <sid_protocol_opts.h>
#include "sid_mac_controller.h"

#if defined(GW_SUPPORT) && GW_SUPPORT
#define RNET_MAC_LDR_DEBUG_WITH_GPIO    0         // When set, enables GPIO toggling to show radio state changes of the beacon submodule.
#define RNET_MAC_LDR_DEBUG_PIN          3           // Use UART TX PIN of the Nordic in Bridge
#else
#define RNET_MAC_LDR_DEBUG_WITH_GPIO    0         // When set, enables GPIO toggling to show radio state changes of the beacon submodule.
#define RNET_MAC_LDR_DEBUG_PIN          7        //PIN_NORD_TX_DISABLE
#endif

#if RNET_MAC_LDR_DEBUG_WITH_GPIO
#define RNET_MAC_LDR_DEBUG_PIN_TOGGLE sid_pal_gpio_toggle(RNET_MAC_LDR_DEBUG_PIN)
#define RNET_MAC_LDR_DEBUG_PIN_WRITE(X) sid_pal_gpio_write(RNET_MAC_LDR_DEBUG_PIN,X)
#else
#ifndef RNET_MAC_LDR_DEBUG_PIN_TOGGLE
#define RNET_MAC_LDR_DEBUG_PIN_TOGGLE do {} while (0)
#endif
#ifndef RNET_MAC_LDR_DEBUG_PIN_WRITE
#define RNET_MAC_LDR_DEBUG_PIN_WRITE(X) do {} while (0)
#endif
#endif

#if LDR_TIME_DIVERSITY
// Default reliability value
#define SID_LDR_DEFAULT_RELIABILITY 0

// Default secondary window enabled for SF8
#define SID_LDR_DEFAULT_SEC_RX_WINDOWS_ENABLE 1

// LDR Data Rate
#define DEFAULT_LDR_DATA_RATE SID_PAL_RADIO_DATA_RATE_12_5KBPS

// Uplink preamble is longer to support preamble cycling
#define LDR_DEFAULT_UPLINK_PREAMBLE_SIZE 28

// Default preamble size for ACK Transmission
#define LDR_DEFAULT_ACK_PREAMBLE_SIZE 8

// Payload size for LDR ACK frame
#define LDR_ACK_FRAME_SZ 15

#define LDR_SYMBOL_US LDR_SF8BW500_SYMBOL_US

// Number of LoRa symbols to wait for in Gateway Rx mode before Radio times out
#define LDR_DEFAULT_RX_SYMBOL_TIMEOUT 112

// Secondary Rx window
#define LDR_SECONDARY_RX_WINDOW_INTERVAL_MS 1335
#define LDR_SECONDARY_RX_WINDOW_INTERVAL_TUS MS_TO_TUS(LDR_SECONDARY_RX_WINDOW_INTERVAL_MS)
#define LDR_SECONDARY_RX_WINDOW_INTERVAL_US MS_TO_MICRO_SEC(LDR_SECONDARY_RX_WINDOW_INTERVAL_MS)
#define LDR_SECONDARY_RX_NUM_OF_TX_BINS 8

// Time gaps within a secondary Rx window
#define LDR_TX_BIN_START_RX_OFFSET_US 565000
#define LDR_TX_BIN_SUCCESSIVE_GAP_US 92500

// Primary Rx window
#define LDR_PRIMARY_RX_MAX_NUM_OF_SECONDARY_RX 3 // Distributed evenly in a primary Rx window interval
#define LDR_PRIMARY_RX_WINDOW_INTERVAL_TUS (LDR_SECONDARY_RX_WINDOW_INTERVAL_TUS * \
                                           (LDR_PRIMARY_RX_MAX_NUM_OF_SECONDARY_RX + 1))
#define LDR_PRIMARY_RX_NUM_OF_TX_BINS (LDR_SECONDARY_RX_NUM_OF_TX_BINS * \
                                       (LDR_PRIMARY_RX_MAX_NUM_OF_SECONDARY_RX + 1))
#else
// LDR Data Rate
#define DEFAULT_LDR_DATA_RATE SID_PAL_RADIO_DATA_RATE_2KBPS

// Uplink preamble is longer to support preamble cycling
#define LDR_DEFAULT_UPLINK_PREAMBLE_SIZE 16

#define LDR_SYMBOL_US LDR_SF11BW500_SYMBOL_US

// Number of LoRa symbols to wait for in Gateway Rx mode before Radio times out
#define LDR_DEFAULT_RX_SYMBOL_TIMEOUT 14

// Periodic Rx window opens up after Epoch Tx Frame
#define LDR_DEFAULT_PERIODIC_RX_TIME            5
#endif /* LDR_TIME_DIVERSITY */

// Default symbol timeout for Tx Config
#define LDR_DEFAULT_TX_SYMBOL_TIMEOUT 0

// Define radio modem type
#define RNET_MAC_LDR_RADIO_MODE SID_PAL_RADIO_MODEM_MODE_LORA

// Regulatory North America
#define RNET_MAC_LDR_REGULATORY_NA SID_REG_CGI_2

// IQ is inverted for uplink & downlink to avoid interference
#define LDR_DEFAULT_UPLINK_INVERT_IQ 0

#define LDR_DEFAULT_DOWNLINK_INVERT_IQ 1

// Number of LoRa symbols to wait for in End-device Rx mode before Radio times out
#define LDR_DEFAULT_TRACKER_RX_SYMBOL_TIMEOUT 8

// Downlink preamble is shorter as downlink is synchronized to uplink (epoch) packet
#define LDR_DEFAULT_DOWNLINK_PREAMBLE_SIZE 8

/* Semtech recommends to use rx preamble to be more than tx preamble when the length of
 * Tx preamble is not known. The exact settings would be explained in HALO-3445
 */
// Add extra guard symbols on top of maximum transmit preamble length
#define LDR_RX_PREAMBLE_GUARD_SYMBOLS 8

// Total preamble symbol length to be set at Gateway
#define LDR_RX_PREAMBLE_SIZE_GW (LDR_DEFAULT_UPLINK_PREAMBLE_SIZE + \
                                LDR_RX_PREAMBLE_GUARD_SYMBOLS)

#define LDR_SF8BW500_SYMBOL_US 512
#define LDR_SF11BW500_SYMBOL_US 4096

/*
 * account for extra preamble sent during downlink to compensate for RTC drift
 * Max RTC drift = 2 x RNET_DEFAULT_MAX_LDR_DRIFT_TIME_CMP_US
 * Extra preamble symbols = ceil(Max RTC drift/4096) => ~15 SF11BW500 preamble symbols
 */
#define LDR_RX_MAX_DRIFT_PREAMBLE_SYMB (((2 * (RNET_DEFAULT_MAX_LDR_DRIFT_TIME_CMP_US)) + \
                                       (LDR_SYMBOL_US) - 1) / (LDR_SYMBOL_US))

// Total preamble symbol length to be set at End-Node
#define LDR_RX_PREAMBLE_SIZE_END_NODE (LDR_DEFAULT_DOWNLINK_PREAMBLE_SIZE + \
                                      LDR_RX_MAX_DRIFT_PREAMBLE_SYMB + \
                                      LDR_RX_PREAMBLE_GUARD_SYMBOLS)

// Minimum TX delay between two consecutive tx opportunities for distress GW
#define DOWNLINK_TO_DISTRESS_GW_MIN_TX_DELAY_MS 250

// Max PHY Payload size in LDR
#define RNET_MAX_LDR_PHY_OVERHEAD_SZ 20

// Time gap between Tx and Rx slot in sec
#define LDR_DEFAULT_TX_RX_TIME_GAP              1

/* NEEDS SYSTEM ANALYSIS. COULD PERHAPS BE OPTIMIZED
 * Extra time for submodule to complete operation
 */
#define LDR_RADIO_GUARD_TIME_MS                 10

// Minimum time in tu
#define LDR_RTC_MIN_TUS                         5

/* maximum time elapsed since epoch beyond which the gateway would not send downlink
   if the time since last uplink is beyond the threshold, then the clock would have drifted and
   downlink would not be possible
 */
#define RNET_LDR_DEFAULT_MAX_DOWNLINK_DELAY_SINCE_EPOCH_SEC     1200

// Maximum time drift supported for a given device. Units in micro seconds per second
#define RNET_LDR_DEFAULT_MAX_NWK_DRIFT_PER_SEC                  100

/*
 * default value of maximum amount of drift time in micro sec that would be compensated due
 * to clock offset
 */
#define RNET_DEFAULT_MAX_LDR_DRIFT_TIME_CMP_US                  30000

#if LDR_TIME_DIVERSITY
/* SID_LDR_DEFAULT_RELIABILITY needs to be replaced with e.g. txq_head->snd_st.reliability
 * in the future
 */
#define SID_LDR_PKT_MAX_RETRY_IS_HIT(X) ((X)->snd_st.tx_cnt >= sid_ldr_pkt_rel_param(SID_LDR_DEFAULT_RELIABILITY)->num_of_retries + 1)
#else
#define SID_LDR_PKT_MAX_RETRY_IS_HIT(X) ((X)->snd_st.tx_cnt >= 1)
#endif

#if defined(GW_SUPPORT) && GW_SUPPORT
// (1 << 10) = 1024 samples should provide average over last (1024 x HDR_LDR_SLOT) ms
#define RNET_LDR_NOISE_SAMPLE_SIZE_BIT          10
// Must be in the range [1, (1 << RNET_LDR_NOISE_SAMPLE_SIZE_BIT)]
#define RNET_LDR_NOISE_WEIGHT                   100
#define RNET_LDR_NOISE_TOTAL_SAMPLE_SIZE        (1 << RNET_LDR_NOISE_SAMPLE_SIZE_BIT)
#define RNET_LDR_NOISE_SUM_WEIGHT               (RNET_LDR_NOISE_TOTAL_SAMPLE_SIZE - RNET_LDR_NOISE_WEIGHT)
#endif

#if LDR_TIME_DIVERSITY
/*!
 * @brief Compute Tx offset based on Tx Bin index
 * @param[in] tx_bin_idx Tx Bin index
 * @return the time in ms since primary Rx window at which to transmit epoch frame
 */
uint32_t sid_get_ldr_tx_offset(uint8_t tx_bin_idx);
#else
/*!
 * @brief Set Tx Bin gap based on maximum Uplink packet length
 * @param max_pkt_time maximum time on air
 * @param rx_period_sec Periodic Rx window
 */
void rnet_ldr_set_tx_offset_bin(uint32_t max_pkt_time);

/*!
 * @brief Initialization of Tx Offset Parameters based on Lehmer's Equation
 * @param seed pointer to the seed
 * @param sz size of seed array
 */
void rnet_ldr_tx_offset_init(uint8_t* seed, uint8_t sz);

/*!
 * @brief Compute Tx Offset Index based on Lehmer's Equation
 * @param seq_num link layer sequence number of the epoch frame
 * return tx_offset the time in msec since Rx Window at which to transmit epoch frame
 */
uint32_t rnet_set_ldr_tx_offset(uint32_t seq_num);

/*!
 * @brief Compute Tx Offset Index based on Lehmer's Equation
 * @param seed pointer to the seed
 * @param sz size of seed array
 * @param seq_num link layer sequence number of the epoch frame
 * return tx_offset the time in msec since Rx Window at which to transmit epoch frame
 */
uint32_t rnet_get_ldr_tx_offset(uint8_t* seed, uint8_t sz, uint32_t seq_num);
#endif /* LDR_TIME_DIVERSITY */

typedef enum {
    LDR_STATE_IDLE,
    LDR_STATE_RX,
    LDR_STATE_TX,
    LDR_STATE_ACK_TX,
    LDR_STATE_ACK_RX,
    LDR_STATE_ACK_TX_DONE,
    LDR_STATE_ACK_RX_DONE,
} rnet_ldr_state_t;

/* types of events scheduled by LDR */
typedef enum {
    LDR_EVENT_TX, LDR_EVENT_RX,
} rnet_ldr_event_t;

/* priority tx queue */
typedef struct rnet_ldr_txq {
    RNET_LIST_ENTRY(rnet_ldr_txq);
    rnet_transact_store_t *trans_store_pkt;
    rnet_node_t *node;
    uint8_t num_event_scheduled; /* num of times the TX event was scheduled/rescheduled */
} rnet_ldr_txq;

#if LDR_TIME_DIVERSITY
/* packet reliability parameters */
typedef struct sid_ldr_pkt_rel_param{
    uint8_t num_of_retries:3;
    uint8_t ack_required:1;
} sid_ldr_pkt_rel_param_t;

/*!
 * @brief Get packet reliability parameters set
 * @param reliability: reliability value
 * return: pointer to packet reliability parameter set
 */
const sid_ldr_pkt_rel_param_t *sid_ldr_pkt_rel_param(uint8_t reliability);

/*!
 * @brief Encode LDR link layer ack
 * @param[out] ack_frame Pointer to the buffer for the encoded data
 * @param[in] ack_frame_sz Ack frame buffer size
 * @param[in] rx_pkt Pointer to the incoming packet
 * @param[in] raw_rx_pkt Pointer to the raw RX buffer. This is needed to
 * decode the link layer header to get the UUID/RingNet ID
 * return number of bytes encoded, negative number for error code
 */
int16_t sid_ldr_encode_ack_frame(uint8_t *ack_frame, uint8_t ack_frame_sz,
                                 rnet_pkt_t *rx_pkt, rnet_ll_rx_packet_t *raw_rx_pkt);
#endif /* LDR_TIME_DIVERSITY */

#if !defined(X86_UNIT_TEST) || !X86_UNIT_TEST
/*
 * If size changed please update wiki and edit nrf_malloc configurations for
 * products
 * https://wiki.labcollab.net/confluence/x/C0BoQg
 */
_Static_assert(sizeof(rnet_ldr_txq) == 16, "SIZEOF RNET_LDR_TXQ CHANGED");
#endif

/* context for the ldr events */
typedef struct {
    rnet_ldr_event_t event_type;
} rnet_ldr_event_ctx;

/*!
 * @brief Initialization of LDR Module
 * @returns SUBMOD_PROCESS_SUCCESS if the initialization is done
*/
rnet_submod_err_t rnet_mac_ldr_init(void);

/**
 * @brief: function to set LDR mac rules
 * @param unicast_attr
 * @return
 */
sid_error_t sid_set_ldr_attr(struct sid_unicast_attr const *unicast_attr);

#if defined(GW_SUPPORT) && GW_SUPPORT

// Maximum number of time a Downlink packet would be re-scheduled
#define RNET_LDR_MAX_TX_SCHEDULE_LIMIT                  2

/* @brief: function to clear LDR variables used to
 *  calculate metrics after sending metrics to
 * the cloud every 24 hours or by app layer cmd
 */
void clear_ldr_metric_values(void);

/*!
 * @brief Start TX. the frame is queued in priority Q and tx event scheduled
 * @param tx_pkt: pointer to tr entry of tx frame
 * @param node: pointer to node DB of the device
 * return: SUCCESS if the tx frame was queued to the ldr queue
 */
rnet_mac_cmd_status_t rnet_mac_ldr_txframe(rnet_transact_store_t *tx_pkt, rnet_node_t *node);

/*!
 * @brief Set ldr_max_drift_us value
 * @param value: maximum drift value in us
 */
void set_ldr_max_drift(uint16_t time_value_in_us);

/*!
 * @brief Set the maximum latency ldr_max_downlink_latency value
 * @param value: maximum downlink latency since epoch in sec
 */
void set_ldr_max_downlink_latency(uint16_t time_value_in_sec);

/*!
 * @brief Set the rate of drift ldr_max_drift_per_sec
 * @param value: rate at which clock drift in us/sec
 */
void set_ldr_max_drift_rate(uint16_t value_in_us_per_sec);

/*!
 * @brief: function to modify the LDR setting from GW
 * mode to end-node mode
 */
void rnet_mac_ldr_update_to_end_node_settings(void);

/*!
 * @brief: function to modify the LDR setting from end-node
 * mode to gateway mode
 */
void rnet_mac_ldr_update_to_gw_settings(void);

/**
 * @brief LDR's notifier function
 * @param rcvd_ctx: updated context from the config submodule
 * @return rnet_error_t: always return true
 */
rnet_error_t rnet_mac_ldr_ntfy_hndlr(rnet_config_context_t rcvd_ctx);

/*
 * @brief Function to get LDR PHY parameter
 *
 * @param[in,out] pointer to the radio packet configuration.
 */
void rnet_get_ldr_default_rx_cfg(sid_pal_radio_packet_cfg_t* cfg);

#else
/*!
 * @brief Start TX. the frame is queued in priority Q and tx event scheduled
 * @param tx_pkt: pointer to tr entry of tx frame
 * return: SUCCESS if the tx frame was queued to the ldr queue
 */
rnet_mac_cmd_status_t rnet_mac_ldr_txframe(rnet_transact_store_t *tx_pkt);
#endif
#endif
