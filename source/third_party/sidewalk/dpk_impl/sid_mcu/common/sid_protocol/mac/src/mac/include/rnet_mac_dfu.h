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

#ifndef _H_SRC_MAC_HALO_INCLUDE_RNET_MAC_DFU_SUBMODULE_H_
#define _H_SRC_MAC_HALO_INCLUDE_RNET_MAC_DFU_SUBMODULE_H_

#include "rnet_mac_submodule_common.h"
#include <sid_protocol_defs.h>
#include "rnet_ll_phy_interface.h"
#include "rnet_transact_store.h"
#include "rnet_nw_mac_ifc.h"
#include "rnet_mac_util.h"
#include "rnet_app_layer.h"

#define RNET_MAC_DFU_DEBUG_WITH_GPIO    0         // When set, enables GPIO toggling to show radio state changes of the DFU submodule.
#define RNET_MAC_DFU_DEBUG_PIN          2         // Use UART RX PIN of the Nordic in Bridge

#if RNET_MAC_DFU_DEBUG_WITH_GPIO
#define RNET_MAC_DFU_DEBUG_PIN_TOGGLE sid_pal_gpio_toggle(RNET_MAC_DFU_DEBUG_PIN)
#define RNET_MAC_DFU_DEBUG_PIN_WRITE(X) sid_pal_gpio_write(RNET_MAC_DFU_DEBUG_PIN,X)
#else
#ifndef RNET_MAC_DFU_DEBUG_PIN_TOGGLE
#define RNET_MAC_DFU_DEBUG_PIN_TOGGLE do {} while (0)
#endif
#ifndef RNET_MAC_DFU_DEBUG_PIN_WRITE
#define RNET_MAC_DFU_DEBUG_PIN_WRITE(X) do {} while (0)
#endif
#endif

#define RNET_MAC_DFU_RADIO_MODE SID_PAL_RADIO_MODEM_MODE_LORA
#define RNET_MAC_DFU_REGULATORY    SID_REG_CGI_2  //Region channel group index set to the same as LDR

#define ENCRYPTION_NEEDED 0
#define ENCRYPTION_NOT_NEEDED 1

typedef enum {
    START_DFU_ON_DFLT_CHANNEL1,
    START_DFU_ON_DFLT_CHANNEL2,
    START_DFU_COMPLETED,
} start_dfu_state_t;

typedef enum {
    DFU_STATE_INACTIVE,        //Submodule will be inactive until START_DFU cmd is srent out
    DFU_STATE_MAC_IDLE,        //Submodule will change the state to IDLE upon TX of start dfu cmd
    DFU_STATE_CAD,
    DFU_STATE_RX,
    DFU_STATE_RX_WAIT,
    DFU_STATE_TX,
} rnet_dfu_state_t;

/* priority tx queue */
typedef struct rnet_dfu_txq {
    RNET_LIST_ENTRY(rnet_dfu_txq);   /*< Token used by sortlist. */
    rnet_transact_store_t *tx_pkt;
} rnet_dfu_txq;

/*
 * If size changed please update wiki and edit nrf_malloc configurations for
 * products
 * https://wiki.labcollab.net/confluence/x/C0BoQg
 */
_Static_assert(sizeof(rnet_dfu_txq) == 8, "SIZEOF RNET_DFU_TXQ CHANGED");

/*@brief: API to set the Z-wave threshold to detect energy in Z-wave channels. When the measured
 * RSSI is greater than the threshold, presence of Z-wave signal is detected
 *
 * @param[in]: zwave_threshold_value: value in dBm
 * @param[in]: zwave_tr_store_timeout_s: lifetime value of a dfu pkt
 * in seconds
 */
void rnet_set_zwave_ed_threshold(int8_t zwave_threshold_value, uint8_t zwave_tr_store_timeout_s);

/*@brief: API to get the current Z-wave threshold value set
 *
 * @return: zwave_threshold_value: current threshold set for Z-wave energy detect in dBm
 */
int8_t rnet_get_zwave_ed_threshold(void);

/*@brief: API to enable/disable the Z-wave energy detection feature enable/disable
 *
 * @param[in]: flag: set to 0 for the feature to be disabled, otherwise > 0
 */
void rnet_set_zwave_ed_enable_flag(uint8_t flag);

/*@brief: API to get the Z-wave energy detection enable/disable flag
 *
 * @return: zwave_enable_flag: value is equal to 0 when the feature is disabled, otherwise > 0
 */
uint8_t rnet_get_zwave_ed_enable_flag(void);

void rnet_dfu_notify_plugin(uint8_t flag);

/* @brief: Function called by config manager to
 * check whether dfu is currently active or
 * not before making any config changes
 *
 *@param bool[out]: returns a flag denoting the state
 * of dfu
 */
bool rnet_get_dfu_status();

/*@brief: Function called by Frame manager to send out
 * Start-DFU packet on Default DFU channels
 */

void rnet_start_dfu_init(void);

/*
 * @brief: Initializes the DFU submodule
 * @ret SUBMOD_PROCESS_SUCCESS if the initialization is done
 */

rnet_submod_err_t rnet_mac_dfu_init(void);

/* @brief: Function called when setting DFU params
 * sent by plugin
 * @params channel[in]: channel between 0-7
 * @params data_rate[in]: spreading factor between 5-7
 */

void rnet_mac_dfu_settings_set(U8 channel, U8 data_rate);

/*@brief: To enqueue packet received from
 * the Frame Manager & add it to FIFO
 * Storing each packet's received time
 * @param tx_pkt[in]:transact store pkt
 *
 * @return rnet_mac_cmd_status_t Success or failure to consume the packet
*/

rnet_mac_cmd_status_t rnet_mac_dfu_txframe(rnet_transact_store_t* tx_pkt, rnet_node_t* node);


/*@brief: Function called when trying to stop
 * Preamble cycling for DFU. Clears the queue, removes
 * the event from he schedulers event list and enabled LDR
 *
 */
rnet_mac_cmd_status_t rnet_dfu_submodule_terminate(void);

#endif /* _H_SRC_MAC_HALO_INCLUDE_RNET_MAC_DFU_SUBMODULE_H_ */
