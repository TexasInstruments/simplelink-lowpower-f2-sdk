/*
 * Copyright 2020 Amazon.com, Inc. or its affiliates. All rights reserved.
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

#ifndef HALO_LIB_RINGNET_WAN_SRC_MAC_HALO_RNET_MAC_DISCOVERY_H_
#define HALO_LIB_RINGNET_WAN_SRC_MAC_HALO_RNET_MAC_DISCOVERY_H_

#include "rnet_mac_submodule_common.h"
#include "rnet_mac_beacon.h"

#define RNET_MAC_DISCO_DEBUG_WITH_GPIO    0         // When set, enables GPIO toggling to show radio state changes of the beacon submodule.
#define RNET_MAC_DISCO_DEBUG_PIN          PIN_NORD_TX_DISABLE

#if RNET_MAC_DISCO_DEBUG_WITH_GPIO
#define RNET_MAC_DISCO_DEBUG_PIN_INIT() \
    do{\
        sid_pal_gpio_set_direction(RNET_MAC_DISCO_DEBUG_PIN, SID_PAL_GPIO_DIRECTION_OUTPUT);\
    }while(0)

#define RNET_MAC_DISCO_DEBUG_PIN_TOGGLE \
        sid_pal_gpio_toggle(RNET_MAC_DISCO_DEBUG_PIN)
#define RNET_MAC_DISCO_DEBUG_PIN_WRITE(X) \
    do{\
        sid_pal_gpio_write(RNET_MAC_DISCO_DEBUG_PIN,X); \
    }while(0)
#else
#define RNET_MAC_DISCO_DEBUG_PIN_INIT() do {} while (0)
#ifndef RNET_MAC_DISCO_DEBUG_PIN_TOGGLE
#define RNET_MAC_DISCO_DEBUG_PIN_TOGGLE do {} while (0)
#endif
#ifndef RNET_MAC_DISCO_DEBUG_PIN_WRITE
#define RNET_MAC_DISCO_DEBUG_PIN_WRITE(X) do {} while (0)
#endif
#endif

#define DISCO_RX_SYMBOL_TIMEOUT       10
#define DISCO_PREAMBLE_SIZE           10

typedef struct rnet_mac_disco_bcn_notification_data{
    uint8_t* gw_id;
    size_t gw_id_size;
    time_def* ts;
    uint8_t bcn_channel_hopping_identifier;  // 1 byte Identifier for the CH algorithm used by GW
    uint8_t* bcn_random_ch_seed; //6 byte //The seed for CH
    rnet_mac_bcn_cloud_connection_state_t bcn_cld_conn_state;
    uint8_t* pan_id;
    int16_t rssi;
    int8_t snr;
    bool sidewalk_consent;
    struct bcn_cs_slot_schedule_t cs_sch;
    uint8_t bcn_rx_channel; // Beacon RX channel
} rnet_mac_disco_bcn_notification_data_t;

typedef void (*rnet_mac_disco_bcn_notification_t)(rnet_mac_disco_bcn_notification_data_t* r);

/**
 * @brief Initialize discovery submodule.
 * @ret SUBMOD_PROCESS_SUCCESS if the initialization is done
 */
rnet_submod_err_t rnet_mac_disco_init(void);

/**
 * @brief Start scanning for beacons. The operation continues until stopped explicitly.
 * For each received beacon, the callback is triggered.
 *
 *  @param f: Callback function pointer that is invoked with each discovered beacon
 *       fitting criteria.
 *  @param req_auth: When set to true, the discovered beacons are checked for
 *       authentication and the callback is executed only for beacons passing authentication.
 *       When set to false, the discovered beacons are not checked for authentication.
 *  @return Returns an error condition if operation fails to start. Otherwise returns RNET_SUCCESS.
 *
 */
rnet_error_t rnet_mac_disco_start_sampling(rnet_mac_disco_bcn_notification_t f, bool req_auth);

/**
 * @brief Stops scanning process.
 * @return Returns an error condition if fails to stop. Otherwise returns RNET_SUCCESS.
 *
 */
rnet_error_t rnet_mac_disco_stop_sampling();

#endif /* HALO_LIB_RINGNET_WAN_SRC_MAC_HALO_RNET_MAC_DISCOVERY_H_ */
