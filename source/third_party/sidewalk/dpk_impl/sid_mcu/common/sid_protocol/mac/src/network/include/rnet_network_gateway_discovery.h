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

#ifndef _H_RNET_GATEWAY_DISCOVERY_AND_SYNC_SUBMODULE_H_
#define _H_RNET_GATEWAY_DISCOVERY_AND_SYNC_SUBMODULE_H_

#include "rnet_nw_mac_ifc.h"
#include "rnet_mac_discovery.h"
#include "rnet_dev_addr_def.h"
#include <sid_protocol_opts.h>
#if HALO_WITH_MODULE_HALO_LIB_HALO_MANAGEMENT
#include "rnet_hall.h"
#endif

#define ALL_SID_SUPPORTED_CH                    0x7E000003  //(bitmap for ch 0,1,25,26,27,28,29,30)
#define ALL_SID_HALO1_CH                        0xFF //(bitmap for ch 0 to 7)
#define MAX_NUM_OF_CMD_PER_SCAN                 0x02
#define DEFAULT_PASSIVE_GWD_TIMEOUT_SEC         40
#define DEFAULT_BEACON_AUTHENTICATION           1

/*
 * @ brief mode of operations discovery module could be invoked at
 *
 *      GWD_SYNC_MODE : Performs gateway discovery to facilitate synchronization.
 *      GWD_GW_TO_GW_MODE: Performs neighbor gateway discovery.
 *      GWD_FFS_MODE  : Performs gateway discovery to facilitate Frustration Free Setup.
 */
typedef enum {
    GWD_ACTIVE_MODE_NONE = 0,
    GWD_SYNC_MODE,
    GWD_GW_TO_GW_MODE,
    GWD_FFS_MODE,
    GWD_PASSIVE_SYNC_MODE
} rnet_discovery_mode_t;

typedef enum{
    GWD_RUN_MODE_INVALID = 0,
    GWD_RUN_AUTONOMUS,
    GWD_RUN_ASSISTED,
    GWD_RUN_PASSIVE_AUTONOMOUS,
}rnet_gwd_init_mode_t;

#define IS_RNET_GWD_RUN_AUTONOMOUS(X) (X == GWD_RUN_AUTONOMUS || X == GWD_RUN_PASSIVE_AUTONOMOUS)

typedef enum{
    NGD_DATA_RATE_NO_PREFERENCE = 0,
    NGD_DATA_RATE_HDR,
    NGD_DATA_RATE_LDR
}ngd_phy_mode_t;

/*
 * @ gateway discovery sub-module runtime status
 */
typedef struct {
    enum status {
        GWD_NOT_INIT = 0,
        GWD_ACTIVE,
        GWD_NOT_ACTIVE
    } status;
    rnet_discovery_mode_t active_disco_mode;
    rnet_gwd_init_mode_t init_mode;
} gwd_proc_status_t;

/*
 * @brief Structure to hold the gateway scan results
 */
typedef struct {
    rnet_error_t ret_code;                // Return code reflecting the result of last scan;
    time_def last_bcn_frame_tm;         // Time when the last beacon frame was sent by the gateway;
                                        // This value is directly consumed by the beacon submodule;
    int16_t rssi;                       // received signal strength;
    int8_t snr;                         // signal to noise ratio;
    uint8_t resp_gw_id[RNET_DEV_ID_SZ]; //responded gateway serial number;
} gwd_resp_t;

/*!
 * @brief enumeration type to
 * identify GW validation level
 */
typedef enum{
    GW_PASSED_VALIDATION = 0,
    GW_SECOND_BEST_MATCH,
    GW_IN_BLOCK_LIST, /*For internal consumption only*/
    GW_DISCOVERY_FAILED
}rnet_gw_validation_status_t;

/*
 * @brief Structure to hold the ffs scan results
 * Note: for internal (probe submodule ) usage
 */
struct ffs_resp_internal {
    rnet_error_t ret_code;
    time_def last_bcn_frame_tm;         // Time when the last beacon frame was sent by the gateway;
    int16_t rssi;                       // received signal strength;
    int8_t snr;                         // signal to noise ratio;
    rnet_pkt_t *ffs_rsp_pkt;
};

/*@brief: structure to send out the ffs discovery result to the caller*/
typedef struct{
    rnet_error_t ret_code; // Return code reflecting the result of last scan;
    struct ffs_resp_internal resp[MAX_NUM_OF_CMD_PER_SCAN];
} ffs_resp_t;

#if (RNET_MAC_CHANNEL_HOPPING && !GW_SUPPORT)
typedef struct {
    rnet_gw_validation_status_t ret_code;
    rnet_mac_disco_bcn_notification_data_t *bcn_data;
} passive_gwd_resp_t;
#endif //RNET_MAC_CHANNEL_HOPPING

/*
 * @brief: gwd payload structure
 */
typedef struct {
    rnet_transact_store_t *app_payload;
    bool is_resp_req;
} gwd_ctx_pld_t;

/*
 * @brief Structure to define the scan context for the gateway discovery
 */
typedef struct{
    uint8_t is_action_req:1;
    /* Note: if set true, GW discovery assistant
     * will call beacon go sync to sync to the GW
     */
    uint8_t phy_data_rate:3;
    /* Note: determines what data rate should be
     * used to continue the scan.
     */
    uint8_t is_authenticate_bcn:1;
    /* Note: if set true, discovery process will
     * only be allowed to discover and consumes
     * beacon that passes authentication
     */
    uint8_t is_only_connect_pan_gws:1;
    /* Note: if set true, the discovery process
     * will only discovers the GW(s) which belongs
     * to its PAN.
     */
    uint8_t* preferred_gw_id;
    /*
     * Note: if not NULL, discovery assistant will
     * only look for that GW to connect to and,
     * continue finding until timeout.
     * Currently, GW id are 5-bytes long.
     */
    uint32_t timeout_sec;
    /* Note: Max time duration until the scan will
     * continue to find the first available GW that
     * meets the criteria set by the session start parameter
     */
    uint32_t ch_map;
    /*Needed only for active scanning*/
    uint8_t retry_cnt;
    /* Needed only for active scanning*/
    gwd_ctx_pld_t* pld_array; //MAX allowed payload cnt is limited to MAX_NUM_OF_CMD_PER_SCAN
    /* Needed only for active scanning*/
    uint8_t input_pkt_cnt;
    /* Needed only for active scanning*/
    void (*gwd_rsp_handler)(void*);
    /* Note: Function pointer to be used in the call back
     * with discovery results.
     * In GWD_SYNC_MODE, it takes  <gwd_resp_t*> and
     * for GWD_GW_TO_GW_MODE, it takes <rnet_error_t*> as function parameter.
     */

}gwd_ctx_t;

/**
 * @brief This is a callback funtion for gateway discovery and
 *        is used to process the status of the gateway discovery process.
 *        If the GW is not found, timer is started for next GW discovery time
 * @param gw_resp_t this contains the status for the gateway discovery.
 */
void rnet_assisted_gwdisc_callback(void* resp);

/**
 * @brief This will restart the gw discovery proccess
 */
void rnet_restart_gwdisc_assisted(void);



/*
 * brief function to initialize gateway discovery process.
 *
 * @param[in] pointer to the variable containing all the config param for the NW layer
 *      upon initialization, this variable will have all the address to the functions
 *      that need to be called to have a seam-less connection with the layer below.
 * @param[in] mode of initialization
 */
void rnet_gateway_discovery_init(rnet_network_config_t* cfg, rnet_gwd_init_mode_t init_mode);

/*
 * @brief Function to deinitialize  gwd sub-module
 */
void rnet_gateway_discovery_deinit(void);

/*
 * @brief function to start the gateway discovery process.
 *
 * @param[in] mode of gateway discovery <rnet_discovery_mode_t>
 * @param[in] <gwd_ctx_t*>: pass the reference to the desired context
 *      to the discovery module to perform the scan on and accordingly.
 *      If it is NULL, scan will be performed according to the DEFAULT settings.
 *
 * @return true
 *
 * @example 1:
 *
 * void RespHndler(void* resp){
 *     gwd_resp_t * rsp = (gwd_resp_t*)resp;
 *     if(rsp && rsp->ret_code == RNET_SUCCESS){
 *          NRF_LOG_INFO("Discovery Success---------");
 *      }else{
 *          NRF_LOG_INFO("Discovery Failed---------");
 *      }
 *  }
 *
 * gwd_ctx_t ctx = {0};
 * ctx.ch_map = 0x21;   //channel 5 and 0
 * ctx.retry_cnt = 2;
 * ctx.is_action_req = true;
 * ctx.phy_data_rate = NGD_DATA_RATE_NO_PREFERENCE; // No data rate preference: will perform scan on HDL, LDR or both depending on the mode available
 * ctx.pld_array = NULL;
 * ctx.gwd_rsp_handler = RespHndler;
 *
 * rnet_start_gateway_discovery(GWD_SYNC_MODE, &ctx);
 *
 *
 * @example 2: (Gateway to gateway discovery: Neighbor Discovery)
 *
 * void RespHndler(void* resp){
 *     rnet_error_t * rsp = (rnet_error_t*)resp;
 *     if(rsp && (*rsp) == RNET_SUCCESS){
 *          NRF_LOG_INFO("Discovery Success---------");
 *      }else{
 *          NRF_LOG_INFO("Discovery Failed---------");
 *      }
 * }
 *
 * gwd_ctx_t ctx = {0};
 * gwd_ctx_pld_t pld[2];
 * pld[0].is_resp_req = false;
 * pld[0].app_payload = transaction_pointer1;
 * pld[1].is_resp_req = true;
 * pld[1].app_payload = transaction_pointer2;
 * ctx.ch_map = 0x21;   //channel 5 and 0
 * ctx.retry_cnt = 2;
 * ctx.is_action_req = false;
 * ctx.phy_data_rate = NGD_DATA_RATE_NO_PREFERENCE;
 * ctx.pld_array = pld;
 * ctx.input_pkt_cnt = 2;
 * ctx.gwd_rsp_handler = RespHndler;
 *
 * rnet_start_gateway_discovery(GWD_GW_TO_GW_MODE, &ctx);
 */
bool rnet_start_gateway_discovery(rnet_discovery_mode_t mode, gwd_ctx_t* gwd_ctx);

/*
 * @brief function to stop the operation of gateway discovery process
 * @return true/false based on the state
 */
bool rnet_stop_gateway_discovery(void);

/*
 * @brief function to restart the gw synchronization process
 * @param[in] context pointer in assisted mode.
 * @return true/false based on state
 */
bool rnet_reset_gwd_sync_mode(gwd_ctx_t *gwd_ctx);

/*
 * @brief function to force connect to or disconnect from a desired gateway
 * @param[in] pointer to desired gateway id array
 * @param[in] length of desired gateway id
 * @param[in] flag to indicate to connect or disconnect from desired gateway
 * @return end node status
 */
sid_error_t rnet_force_connect_disconnect_desired_gw(uint8_t *gw_id, uint8_t gw_id_len, bool is_connect);

/*
 * @brief function to get the status of gwd process
 * @returns gwd_proc_status_t
 */
gwd_proc_status_t rnet_get_gwd_process_status(void);

/*
 * @brief process the gwd event set by the gwd tmr
 */
void process_gwd_tmr_evnt(void);

#if ((RNET_MAC_CHANNEL_HOPPING) && !GW_SUPPORT)
/*
 * @brief: Function to start the passive discovery filter
 * Note: useful function to set a gw selection criteria
 *  Currently, it is only based on RSSI. However additional
 *  logic will be added in future iteration to make it more
 *  sophisticated.
 *  In addition: this function call will automatically initialize the
 *  GW blocklisting feature. This session will be open until
 *  an explicit call to the <rnet_gwd_stop_fliter()> function.
 *
 *  param[in]: rssi threshold
 *  return: bool
 *
 *  WARNING: Make sure to call rnet_gwd_stop_fliter() at the end
 *  of the session.
 *
 *======================================================================================
 *                             @@ Coding Example @@
 *======================================================================================
 *  1. rnet_gwd_start_fliter_session(-80);
 *
 *  2(a). rnet_start_gateway_discovery(rnet_discovery_mode_t mode, gwd_ctx_t* gwd_ctx);
 *     2(a.1) if (rnet_add_blocklist(uint8_t* gw_id)) {
 *                // blocklisting  success
 *            } else {
 *                // Block list full. start over the session.
 *            }
 *  2(a). rnet_start_gateway_discovery(rnet_discovery_mode_t mode, gwd_ctx_t* gwd_ctx);
 *  2(a). rnet_start_gateway_discovery(rnet_discovery_mode_t mode, gwd_ctx_t* gwd_ctx);
 *
 *  3. rnet_gwd_stop_fliter_session(); // marks the end of the filter session
 *==============================End of Example===========================================
 */
bool rnet_gwd_start_fliter_session(int16_t rssi);

/*
 * @brief: function to stop the filter.
 * Note: stop will empty out the existing blocklist
 * and enable the user to start a new session.
 */
void rnet_gwd_stop_fliter_session(void);

/*
 * @brief: function to add to blocklist
 * @param[in] 5-Bytes GW ID
 */
bool rnet_add_blocklist(uint8_t* gw_id);
#endif //#if ((RNET_MAC_CHANNEL_HOPPING) && !GW_SUPPORT)

#if !defined(GW_SUPPORT) || !GW_SUPPORT
/*!
 * @brief: function to force end-device to lose sync with GW
 */
void rnet_dev_go_async(void);
#endif

#if HALO_WITH_MODULE_HALO_LIB_HALO_MANAGEMENT
/*
 * @brief function to get start the channel survey
 * @param[in] chsurvey_status: channel survey status. success/failure
 * @param[in] noise_floor: array of noise floor values
 */
void rnet_chsurvey_process_complete(rnet_error_t chan_suvey_status, int8_t *noise_floor);
#endif
#endif /* _H_RNET_GATEWAY_DISCOVERY_AND_SYNC_SUBMODULE_H_ */
