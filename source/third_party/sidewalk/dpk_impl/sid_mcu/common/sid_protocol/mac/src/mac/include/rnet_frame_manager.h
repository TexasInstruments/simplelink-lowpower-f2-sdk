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

#ifndef _H_SRC_MAC_HALO_INCLUDE_FRAME_MANAGER_H
#define _H_SRC_MAC_HALO_INCLUDE_FRAME_MANAGER_H
#include "rnet_transact_store.h"
#include "rnet_mac_submodule_common.h"
#include "rnet_nw_mac_ifc.h"

/* MAC and Frame Manager interface structs
* @brief: The members of this struct are used as
* input params
*/
typedef struct{
    rnet_ll_rx_packet_t* ll_rx_pkt;
    int8_t avg_noise;
    uint8_t channel;
    uint8_t sub_mod;
    bool ll_decode;
}rnet_ll_rx_input_t;

/*@brief: The members of this struct are
 * mainly used as output params
 */

typedef struct{
    rnet_transact_store_t * tr;
    rnet_node_t* node_db_elem_addr;
    bool* tr_duplicate;
}rnet_ll_rx_output_t;

/*
 * @brief: structure to pass parameters
 * for any tx done event
 */
typedef struct {
    rnet_mac_cmd_status_t result;   // result of associated with the tx process: (success/failure)
    rnet_transact_store_t *rnet_tr; // tr buffer entry carrying actual packet and related link info
    rnet_node_t         *rnet_node; // node entry for the destination address
    uint8_t* raw_buff;              // encoded raw buffer
    uint8_t raw_data_sz;            // encoded raw buffer length
#if !defined(GW_SUPPORT) || !GW_SUPPORT
    int8_t tx_power;                // tx power in DBM
#endif
} rnet_ll_tx_done_param_t;

/* @brief: function to return the pointer to
 * rnet_ll_frm_mngr_iface_t
 */
rnet_ll_rx_input_t * rnet_get_ll_rx_input_iface(void);

rnet_ll_rx_output_t * rnet_get_ll_rx_output_iface(void);

extern rnet_transact_store_t rnet_transactions[TRANSACT_STORE_DEPTH];

/*!
 * @brief Function to raise a SWI to indicate a new tx ebtry in transact store.
 * @param [out]: returns RNET_ERROR_INVALID_STATE if the protocol is disabled
 */
rnet_error_t rnet_frame_manager_txframe(void);

/**
 * @brief Function to init frame manager module.
 */
void rnet_frame_manager_init(void);

/**
 * @brief loop through transact store & look for new TX frames desitined to radio.
 */
void rnet_frame_manager_process(void);

/**
 * @brief Function to indicate rx frame by submodule.
 * @param[in]: pointer to rnet_ll_frm_mngr_iface_t function
 * @param[out]: returns a success or failure
 */
rnet_mac_cmd_status_t rnet_frame_manager_rx(rnet_ll_rx_input_t* ll_input_iface, rnet_ll_rx_output_t* ll_output_iface);

/**
 * @brief Function to decode and decrypt the frame.
 * @param[in]: pointer to rnet_ll_frm_mngr_iface_t function
 * @param[out]: returns a success or failure
 */
rnet_mac_cmd_status_t rnet_frame_manager_rx_decode(rnet_ll_rx_input_t *ll_input_iface, rnet_ll_rx_output_t *ll_output_iface);

/**
 * @brief Function invoked by submodules to indicate tx done.
 * @param rnet_ll_tx_done_param_t: parameters associated with the tx done
 */
void rnet_frame_manager_txdone (rnet_ll_tx_done_param_t* tx_done_params);

#if RNET_LOG_RAW_PKT
/**
 * @brief Function to log Tx raw packet
 * @param[in] tx_done_params Pointer to the structure of Tx done parameters
 * @param[in] log_print Print raw packet to logger or not
 */
void rnet_frame_manager_log_tx_packet(rnet_ll_tx_done_param_t *tx_done_params, bool log_print);
#endif

/**
 * @brief Function to queue the tx frame in the per node txq
 * @param[in] pointer to transaction store entry of the frame
 * @param[in] pointer to the node entry of the destination
 * @param[in] priority of the frame
 */
void rnet_node_queue_tx(rnet_transact_store_t *rnet_tr, rnet_node_t *node, U8 priority);

/**
 * @brief Function to return the highest priority frame queued for a node
 * @param[in] pointer to the node entry
 * @return    pointer to the transaction store of the frame
 */
rnet_transact_store_t *rnet_node_get_tx(rnet_node_t *node);

/**
 * @brief Function to remove the frame at the head of node entry
 * @param[in] pointer to the node entry
 * @param[in] tx done status
 * @return    status of the operation
 */
bool rnet_node_dequeue_tx(rnet_node_t *node, rnet_mac_cmd_status_t tx_status);

#endif /* _H_SRC_MAC_HALO_INCLUDE_FRAME_MANAGER_H */
