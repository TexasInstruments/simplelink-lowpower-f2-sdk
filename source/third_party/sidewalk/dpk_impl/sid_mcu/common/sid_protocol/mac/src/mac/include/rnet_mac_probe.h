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

#ifndef RNET_PROBE_SUBMODULE_H_
#define RNET_PROBE_SUBMODULE_H_


#include "rnet_nw_mac_ifc.h"
#include "rnet_mac_submodule_common.h"
#include "rnet_transact_store.h"
#include "rnet_frames.h"
#include "rnet_nw_layer.h"
#include "rnet_network_gateway_discovery.h"
#include <sid_protocol_opts.h>

/*
 * Enumeration for the mode operation probe sub-module can be invoked on
 */
typedef enum {
    PROBE_RESPONSE_MODE,
    PROBE_REQUEST_MODE,
    PROBE_GW_TO_GW_DISCOVERY_MODE,
    PROBE_FFS_MODE,
    PROBE_RSSI_SURVEY
} probe_mode_t;

typedef struct{
    uint32_t ch_map;
    uint8_t retry;
    rnet_transact_store_t* payload;
    ngd_phy_mode_t phy_mode;
    bool is_wait_for_resp;
}probe_ctx_t;

#if RNET_MAC_MODE_FSK
#define RNET_MAC_PROBE_RADIO_MODE SID_PAL_RADIO_MODEM_MODE_FSK
#define RNET_MAC_PROBE_REGULATORY SID_REG_CGI_1
#else
#define RNET_MAC_PROBE_RADIO_MODE SID_PAL_RADIO_MODEM_MODE_LORA
#define RNET_MAC_PROBE_REGULATORY SID_REG_CGI_2
#endif

#define RNET_MAC_PROBE_REGULATORY_LDR SID_REG_CGI_2

#define RNET_MAC_SYNC_LL_FRAME_SIZE     15
#define RNET_MAC_SYNC_AUTH_SIZE         4
#define ENABLE_PROBE_SECURITY           0
#undef ENABLE_PROBE_DEBUG

#define PROBE_RSSI_SURVEY_MS 2

/*
 * @brief Function to initialize probe sub-module
 *
 * @param[in] Pointer to hold the MAC configuration <rnet_mac_config_t>.
 *            On initialization, probe sub-module registers all the necessary
 *            functions to receive notifications from the upper layer
 * @ret SUBMOD_PROCESS_SUCCESS if the initialization is done
 */
rnet_submod_err_t rnet_mac_probe_init(void);


/*
 * @brief function to make probe response frame
 *
 * @param[in] destination address
 * @param[in] the time stamp at which the request was received.
 */
rnet_transact_store_t* get_probe_meta_data(uint8_t* dst_addr, time_def rx_done_at);

/*
 * @brief function to calculate sync time to be sent to end-device
 *
 * @param[in] time stamp of while the request packet was received
 * @param[out] time delta between now and the request receive time stamp
 */
time_def get_sync_time(time_def rx_time);

#endif /* RNET_PROBE_SUBMODULE_H_ */
