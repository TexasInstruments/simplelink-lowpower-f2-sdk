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

#ifndef RNET_NETWORK_LAYER_INTERFACE_H_
#define RNET_NETWORK_LAYER_INTERFACE_H_

#include "rnet_mac_submodule_common.h"
#include "rnet_config.h"

/*
 * @brief list of all the possible MAC events that can be indicated to the network layer
 */
typedef enum {
    PROBE_SUCCESS = 0,
    PROBE_ALL_CH_SCAN_DONE,
    PROBE_FAIL_DWELL_TIMEOUT,
    PROBE_FAIL_OTHER,
    PASSIVE_SCAN_TIMEOUT,
    BEACON_SYNC_LOST, // synchronization is lost
    BEACON_SYNC_UPDATED,  //a beacon is received
    BEACON_MISSED, // a beacon is missed
    BEACON_SYNC_ACHIEVED,
    CHSURVEY_SUCCESS,
    CHSURVEY_FAILURE,
} rnet_mac_event_id_t;

/*
 * @brief Structure for the mac indication event
 */
typedef struct {
    rnet_mac_event_id_t event_id;
    void * event_data;
} rnet_mac_ind_event_t;

/*
 * @brief Structure for the mac indication event context
 */
typedef struct {
    rnet_mac_submodule_t mac_sub_module_name;
    rnet_mac_ind_event_t event;
} rnet_mac_ind_event_context_t;

/*
 * @brief Structure for enabling/disabling different mac sub-modules
 */
typedef struct {
    bool enable_probe;
    bool enable_beacon;
    bool enable_hdr;
    bool enable_hdr_lora;   // cleanup this variable as part of removal of halo1 migration jira
    bool enable_ldr;
    bool enable_dfu;
    bool enable_sdb;
} mac_submodule_enable_t;


typedef struct rnet_mac_bcn_activate_data{
    uint8_t* gw_id; //GW_ID
    size_t gw_id_size;
    time_def* ts;
    uint8_t bcn_channel_hopping_identifier;  // 1 byte Identifier for the CH algorithm used by GW
    uint8_t* bcn_random_ch_seed; //6 byte //The seed for CH
    bool require_authentication;
    uint8_t bcn_rx_channel; // Beacon RX channel
} rnet_mac_bcn_activate_data_t;

/*
 * @brief Structure to configure the MAC layer on startup
 *      this structure also holds the function pointers to pass the
 *      notifications to the different mac layer's entities from the upper layer
 */
typedef struct {
    mac_submodule_enable_t sub_module_flag;
    U8 is_dev_in_sync : 1;
    U8 is_gw_in_distress : 1;            //distress mode flag
    U8 is_dev_in_dedicated_lnk : 1;      //flag to indicate whether the device is participation in dedicated link comm.
    bool (*start_probe_submodule)(uint8_t, void*);
    void (*stop_probe_submodule)(void);
    bool (*beacon_go_sync)(const rnet_mac_bcn_activate_data_t*);
    void (*hdr_go_sync)(void);
    rnet_submod_err_t (*enable_ldr)(void);
    bool (*disable_ldr)(void);
    void (*dfu_dummy_func)(void);
#if defined(GW_SUPPORT) && GW_SUPPORT
    rnet_submod_err_t (*enable_sdb)(void);
    bool (*disable_sdb)(bool);
#endif
} rnet_mac_config_t;

/*
 * @brief Structure to configure the Network layer on startup
 *      this structure also holds the function pointers to pass the
 *      notifications to the different network layer's entities from the upper layer
 */
typedef struct {
    bool is_protocol_init;
    bool enable_gw_discovery;
    void (*process_probe_indication)(rnet_mac_ind_event_t);
    void (*run_resync_proc)(void);
} rnet_network_config_t;

extern rnet_network_config_t network_config;

/** rnet phy event callback*/
//typedef void (*rnet_mac_event_notify_t)(rnet_mac_ind_event_t events);

/*
 * @brief Event handler for the indication event coming from the MAC layer
 *
 * @param[in] <rnet_mac_ind_event_t> carry out mac event and event-data
 */
void rnet_mac_notify(rnet_mac_ind_event_context_t evnt);

/*
 * @brief Function to initialize MAC layer and all of it's associated sub-modules
 *
 * @param[in] pointer to hold MAC configuration parameter <rnet_mac_config_t>
 */
rnet_error_t rnet_mac_init(rnet_mac_config_t* config, uint8_t max_num_mod);

/*
 * @brief Function to initialize the interface between Network and MAC layer
 * @param[in] nw_config: network later configuration
 */
void rnet_init_nw_mac_ifc(rnet_network_config_t *nw_config);

/* @brief : Function to trigger the Gateway discovery
 *
 */
void rnet_trigger_gateway_discovery(void);

#if !GW_SUPPORT && SID_FSK_WAN_SUPPORT
/*
 * @brief Stop compare and update cs slot
 */
void rnet_mac_stop_update_cs_slot(void);

/*
 * @brief Apply CS schedule settings to HDR submodule.
 *
 * @param[in] cs_sch_p: The cs schedule
 * @param force_update: If false, cs schedule settings are compared with previously settings and applied only if there is a change.
 *        If true, new settings are applied without comparison.
 *
 */
bool rnet_mac_apply_cs_slot_schedule_settings(struct bcn_cs_slot_schedule_t const *cs_sch_p, bool force_update);
#endif

#endif /* RNET_NETWORK_LAYER_INTERFACE_H_ */
