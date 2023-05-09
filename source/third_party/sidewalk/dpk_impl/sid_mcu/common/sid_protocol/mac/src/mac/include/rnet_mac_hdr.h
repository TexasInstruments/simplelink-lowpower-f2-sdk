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

#ifndef _H_SRC_MAC_HALO_INCLUDE_RNET_MAC_HDR_H
#define _H_SRC_MAC_HALO_INCLUDE_RNET_MAC_HDR_H


#include "rnet_nw_mac_ifc.h"
#include "rnet_mac_submodule_common.h"
#include "rnet_transact_store.h"
#include "rnet_mac_util.h"
#include <sid_protocol_opts.h>

#include <sid_pal_radio_ifc.h>
#include "sid_mac_controller.h"

#if defined(GW_SUPPORT) && GW_SUPPORT
#define RNET_MAC_HDR_DEBUG_WITH_GPIO    0         // When set, enables GPIO toggling to show radio state changes of the beacon submodule.
#define RNET_MAC_HDR_DEBUG_PIN          2         // Use UART RX PIN of the Nordic in Bridge
#else
#define RNET_MAC_HDR_DEBUG_WITH_GPIO    0         // When set, enables GPIO toggling to show radio state changes of the beacon submodule.
#define RNET_MAC_HDR_DEBUG_PIN          7
#endif

#if RNET_MAC_HDR_DEBUG_WITH_GPIO
#define RNET_MAC_HDR_DEBUG_PIN_TOGGLE sid_pal_gpio_toggle(RNET_MAC_HDR_DEBUG_PIN)
#define RNET_MAC_HDR_DEBUG_PIN_WRITE(X) sid_pal_gpio_write(RNET_MAC_HDR_DEBUG_PIN,X)
#else
#ifndef RNET_MAC_HDR_DEBUG_PIN_TOGGLE
#define RNET_MAC_HDR_DEBUG_PIN_TOGGLE do {} while (0)
#endif
#ifndef RNET_MAC_HDR_DEBUG_PIN_WRITE
#define RNET_MAC_HDR_DEBUG_PIN_WRITE(X) do {} while (0)
#endif
#endif

/* internal state of HDR module */
typedef enum {
    HDR_STATE_IDLE,
    HDR_STATE_RX,
    HDR_STATE_TX,
    HDR_STATE_CAD,
    HDR_STATE_ACK_TX,
    HDR_STATE_ACK_RX,
} rnet_hdr_state_t;

/* types of events scheduled by HDR */
typedef enum {
    HDR_EVENT_TX,
    HDR_EVENT_RX,
    HDR_EVENT_TXACK,
    HDR_EVENT_RXACK,
} rnet_hdr_event_t;

/* priority tx queue */
typedef struct rnet_priority_txq {
    RNET_LIST_ENTRY(rnet_priority_txq); /*< Token used by sortlist. */
    rnet_transact_store_t *tx_pkt;
    uint8_t num_tx_tries; /* num tx attempts for the tx frame */
    uint8_t last_backoff_slot; /* hdr slot used during previous Tx try */
    uint8_t num_event_resched; /* num of times the scheduler has rescheduled the tx event */
    uint16_t preamble_len;
} rnet_priority_txq;

#if !defined(X86_UNIT_TEST) || !X86_UNIT_TEST
/*
 * If size changed please update wiki and edit nrf_malloc configurations for
 * products
 * https://wiki.labcollab.net/confluence/x/C0BoQg
 */
_Static_assert(sizeof(rnet_priority_txq) == 16, "SIZEOF RNET_PRIORITY_TXQ CHANGED");
#endif

/* context for the hdr events */
typedef struct {
    rnet_hdr_event_t event_type;
    rnet_priority_txq *rnet_txq_entry;
    bool is_sync;
    uint8_t channel;
    uint32_t symbol_to;
} rnet_event_ctx;

/* lbt paramater per priority queue */
typedef struct {
    uint8_t lbt_fixed_backoff;
    uint8_t lbt_cw_min;
    uint8_t lbt_cw_max;
} rnet_lbt_param_t;

#if RNET_MAC_MODE_FSK
#define RNET_MAC_HDR_RADIO_MODE SID_PAL_RADIO_MODEM_MODE_FSK

/* Regulatory North America */
#define RNET_MAC_HDR_DATA_REGULATORY_NA SID_REG_CGI_1
#define RNET_MAC_HDR_ACK_REGULATORY_NA SID_REG_CGI_1

/* Regulatory Europe */
#define RNET_MAC_HDR_DATA_REGULATORY_EU SID_REG_CGI_2
#define RNET_MAC_HDR_ACK_REGULATORY_EU SID_REG_CGI_3
#else
#define RNET_MAC_HDR_RADIO_MODE SID_PAL_RADIO_MODEM_MODE_LORA
#define RNET_MAC_HDR_REGULATORY_NA SID_REG_CGI_2
#endif

#define HDR_WAIT_PERIOD_BETWEEN_TX_AND_ACK_MS 6 // Maximum wait period between TX and corresponding LL ACK.
#define HDR_FSK_PBL_TIMEOUT_MS                2 // Maximum wait period for preamble detection

#define HDR_UP2P_TIME_UNCERTAINITY_MS 60 // Wakeup period in HDR async mode

#define HDR_UP2P_TX_FIXED_BACKOFF   50 //fixed backoff in msec

#define HDR_MAX_TX_SCHED_ATTEMPTS   25   //number of scheduling attempts before aborting Tx

#define CW_MIN_DEF                  1  //default contention window wmin

#define CW_MAX_DEF                  5  //default contention window wmax

#define HDR_RADIO_GUARD_TIME        10 //guard time in msec

#define HDR_P2P_CHANNEL             5

#define HDR_INVERT_IQ               0

#define HDR_RX_SYMBOL_TIMEOUT       13

#define HDR_CRC_ENABLED             true

#define FSK_END_NODE_DEFAULT_OFFSET 0

#define FSK_END_NODE_DEFAULT_PERIODICITY 1

#define HDR_MAX_RX_PREAMBLE_SIZE 0xFFFF //Maximum expected size of preamble duration observed at the receiver.

#define FRAME_HAS_ACK_REQ(tr)        ((tr->pkt.ll_frame.dst_frmt == RNET_LL_DST_FRMT_DEVID \
                                        || tr->pkt.ll_frame.dst_frmt == RNET_LL_DST_FRMT_CLD) \
                                        && tr->pkt.ll_frame.ack_req)

/*!
 * @brief Initialization of HDR Module
 * @return SUBMOD_PROCESS_SUCCESS if the initialization is done
 */
rnet_submod_err_t rnet_mac_hdr_init(void);
rnet_submod_err_t rnet_mac_lora_hdr_init(void);

/*!
 * @brief Start TX. the frame is queued in priority Q and tx event scheduled
 * @param tx_pkt: pointer to tr entry of tx frame
 * return: SUCCESS if the tx frame was queued to the hdr queue
 */
rnet_mac_cmd_status_t rnet_mac_hdr_txframe(rnet_transact_store_t *tx_pkt, rnet_node_t* node);
rnet_mac_cmd_status_t rnet_mac_lora_hdr_txframe(rnet_transact_store_t *tx_pkt, rnet_node_t* node);

/* reason code param for rnet_mac_hdr_schedule_tx() */
typedef enum {
    HDR_TXSCHED_EXP_BKOFF,
    HDR_TXSCHED_NEXT_SLOT,
} rnet_txsched_reason_t;

#if !defined(GW_SUPPORT) || !GW_SUPPORT
/*!
 * @brief Call to switch to synchronous rx/tx operation.
 */
void rnet_hdr_go_sync();
/*!
 * @brief Call to switch to asynchrouns rx/tx operation
 */
void rnet_hdr_go_async();
#endif

/*!
 * @brief Queries the sync state of the hdr module
 * @return true for sync, false for async mode
 */
bool rnet_hdr_is_in_sync_mode();

/**
 * @brief Function to disable hdr submodule
 */
bool rnet_mac_disable_hdr(void);

/**
 * @brief Function to enable hdr submodule
 */
rnet_submod_err_t rnet_mac_enable_hdr(void);


/**
 * @brief Function to disable hdr lora submodule
*/
bool rnet_mac_disable_lora_hdr(void);

/**
 * @brief Function to enable hdr lora submodule
*/
rnet_submod_err_t rnet_mac_enable_lora_hdr(void);

/* @brief: function to clear HDR variables used to
 *  calculate metrics after sending metrics to
 * the cloud every 24 hours or by app layer cmd
 *
 */
void clear_hdr_metric_values(void);

/*
 * @brief Function to get HDR PHY parameter
 *
 * @param[in-out] pointer to the radio packet configuration.
 */
void rnet_get_hdr_default_tx_cfg(sid_pal_radio_packet_cfg_t* cfg);

/**
 * @brief Function to configure the FSK sub-module according to the rules
 * set by SALL.
 * @param[in] unicast_attr_array
 * @param unicast_attr_cnt
 * @param[in] grp_attr_array
 * @param grp_cnt
 * @return
 */
sid_error_t sid_set_fsk_attr(struct sid_unicast_attr const *unicast_attr_array, uint8_t unicast_attr_cnt,
                             struct sid_grp_attr const *grp_attr_array, uint8_t grp_cnt);
/**
 * @brief: function to save existing FSK attr
 */
void sid_save_fsk_existing_attr(void);

/**
 * @brief: function to apply previously saved FSK attr
 */
void sid_fsk_revert_back_to_prev_attr(void);

/**
 * @brief: function to enable continuous FSK tx slot
 * @param[in] enable
 */
void sid_fsk_continuous_tx_slot(bool enable);

/**
 * @brief: function to apply FSK attribute from configuration setting
 */
void sid_set_fsk_attr_from_config(void);
#endif /* _H_SRC_MAC_HALO_INCLUDE_RNET_MAC_HDR_H */
