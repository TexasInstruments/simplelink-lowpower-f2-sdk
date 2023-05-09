/*
 * Copyright 2019-2021 Amazon.com, Inc. or its affiliates. All rights reserved.
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

#ifndef RNET_MAC_SDB_H_
#define RNET_MAC_SDB_H_

#include <sid_network_address.h>
#include <sid_protocol_opts.h>
#include <sid_pal_radio_ifc.h>

#include "rnet_nw_mac_ifc.h"
#include "rnet_mac_submodule_common.h"
#include "rnet_transact_store.h"
#include "rnet_mac_util.h"
#include "sid_mac_controller.h"

#define RNET_MAC_SDB_RADIO_MODE SID_PAL_RADIO_MODEM_MODE_FSK
/* Regulatory North America */
#define RNET_MAC_SDB_DATA_REGULATORY_NA SID_REG_CGI_1
#define RNET_MAC_SDB_ACK_REGULATORY_NA SID_REG_CGI_1

/* Regulatory Europe */
#define RNET_MAC_SDB_DATA_REGULATORY_EU SID_REG_CGI_2
#define RNET_MAC_SDB_ACK_REGULATORY_EU SID_REG_CGI_3

enum sdb_mac_mode {
   SDB_MAC_MODE_DISCOVER_EP,
   SDB_MAC_MODE_CONNECTING,
   SDB_MAC_MODE_CONNECTED,
   SDB_MAC_MODE_DISCONNECTED,
};

struct sdb_open_params_t {
    struct sid_address dst;
};

struct sdb_connect_params_t {
    uint8_t channel;
};

struct sdb_connected_params_t {
    uint32_t chsid;
};

#define SDB_P2P_CHANNEL              5


#define SDB_INVERT_IQ               0

#define SDB_RX_SYMBOL_TIMEOUT       32

/*
 * RX Preamble size value needs to be clarified by semtech:
 * Value of 0xFFFF was recommended for LoRa and we have used the same value for GFSK
 * Jira tracking this is here: https://issues.labcollab.net/browse/HALO-20127
 */
#define SDB_MAX_RX_PREAMBLE_SIZE    0xFFFF //Maximum expected size of preamble duration observed at the receiver.

#define SDB_ASYNC_TIME_UNCERTAINITY_US (63 * SID_TIME_USEC_PER_MSEC)

#define SDB_RADIO_GUARD_TIME        10 //guard time in msec

#define SDB_WAIT_PERIOD_BETWEEN_TX_AND_ACK_MS  10

#define SDB_FRAME_HAS_ACK_REQ(tr)        ((tr->pkt.ll_frame.dst_frmt == RNET_LL_DST_FRMT_DEVID \
                                        || tr->pkt.ll_frame.dst_frmt == RNET_LL_DST_FRMT_CLD) \
                                        && tr->pkt.ll_frame.ack_req)

#define SDB_MAC_DWELL_MS            400

#define SDB_MAC_GUARD_MS            3

#define SDB_MAC_NUM_CHANNEL         69

/*!
 * @brief : Initialization of SDB Module if the SDB submodule is enabled
 */
rnet_submod_err_t rnet_mac_sdb_init(void);

/*
 * @brief : Function to disable the SDB
 * operation.
 * @params[in] sm_status : flag that indicates the status of the submodule. This is set to
 * true if the SDB operation needs to be terminated and the config params storing the
 * value of the SDB submodule needs to be set to true too. It will be set to false when
 * the value of the config params need to be changed as well, for example in case of changing
 * the mode of Gateway
 */
bool rnet_mac_disable_sdb(bool sm_status);

/* @brief : Function to change the config params value of the SDB submodule and initialize
 * the submodule.
 *
 */
rnet_submod_err_t rnet_mac_enable_sdb(void);

/* @brief : Function to queue the packets classified as Sideband packets.
 * @params[in] : pointer to tr entry of tx frame
 * @params[in] : pointer to the node
 * @params[out : SUCCESS if the tx frame was queued to the queue
 */
rnet_mac_cmd_status_t rnet_mac_sdb_txframe(rnet_transact_store_t *tx_pkt, rnet_node_t *node);

/* @brief : Function to set the connection parameters set by the sdb app layer.
 * @params[in] sdb_event : sdb_mac_mode set by the sdb app
 * @params[in] : void context holds parameters related to the sdb_event
 */
sid_error_t rnet_mac_sdb_set_conn_params(enum sdb_mac_mode sdb_event, void *context);

/* @brief : Function to set the state of the SDB submodule
 * @params[in] sdb_event : sdb_mac_mode set by the sdb app
 */
sid_error_t rnet_mac_sdb_set_state(enum sdb_mac_mode sdb_event);

#endif /* RNET_MAC_SDB_H_ */
