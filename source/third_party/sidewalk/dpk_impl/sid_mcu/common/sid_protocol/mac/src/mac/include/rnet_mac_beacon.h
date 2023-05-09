/*
 * Copyright 2019-2022 Amazon.com, Inc. or its affiliates. All rights reserved.
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

#ifndef _H_SRC_MAC_HALO_INCLUDE_RNET_MAC_BEACON_H_
#define _H_SRC_MAC_HALO_INCLUDE_RNET_MAC_BEACON_H_

#include "rnet_nw_mac_ifc.h"
#include "rnet_time.h"
#include "rnet_dev_addr_def.h"
#include <sid_protocol_opts.h>

#include <sid_pal_radio_ifc.h>

#if defined(GW_SUPPORT) && GW_SUPPORT
#define HALO_PAN_COORDINATOR 1                  // When set, node takes beacon transmitter role, which is intended for a bridge
#else
#define HALO_PAN_COORDINATOR 0                     // When set, node takes beacon transmitter role, which is intended for a bridge
#endif

#if defined(GW_SUPPORT) && GW_SUPPORT
#define RNET_MAC_BEACON_DEBUGWITHGPIO      0         // When set, enables GPIO toggling to show radio state changes of the beacon submodule.
#define RNET_MAC_BEACON_DEBUG_PIN          2         // Use UART RX PIN of the Nordic in Bridge
#else
#define RNET_MAC_BEACON_DEBUGWITHGPIO      0         // When set, enables GPIO toggling to show radio state changes of the beacon submodule.
#define RNET_MAC_BEACON_DEBUG_PIN          PIN_NORD_TX_DISABLE
#endif

#define RNET_MAC_BEACON_ENABLE_RTT_DEBUG   1                        // Enables debug messages

#define RNET_MAC_BEACON_MSG_AUTHENTICATION RNET_SECURITY_BUILD    // Enables the authentication code generation/check on beacons

#if RNET_MAC_BEACON_DEBUGWITHGPIO
#define RNET_MAC_BEACON_DEBUG_PIN_TOGGLE \
        sid_pal_gpio_toggle(RNET_MAC_BEACON_DEBUG_PIN)
#define RNET_MAC_BEACON_DEBUG_PIN_WRITE(X) \
        sid_pal_gpio_write(RNET_MAC_BEACON_DEBUG_PIN,X)
#else
#ifndef RNET_MAC_BEACON_DEBUG_PIN_TOGGLE
#define RNET_MAC_BEACON_DEBUG_PIN_TOGGLE do {} while (0)
#endif
#ifndef RNET_MAC_BEACON_DEBUG_PIN_WRITE
#define RNET_MAC_BEACON_DEBUG_PIN_WRITE(X) do {} while (0)
#endif
#endif

#if RNET_MAC_MODE_FSK
#define RNET_MAC_BEACON_RADIO_MODE SID_PAL_RADIO_MODEM_MODE_FSK
#define RNET_MAC_BEACON_DATA_RATE SID_PAL_RADIO_DATA_RATE_50KBPS
#define RNET_MAC_BEACON_REGULATORY_NA SID_REG_CGI_1
#define RNET_MAC_BEACON_REGULATORY_EU SID_REG_CGI_1
#else
#define RNET_MAC_BEACON_RADIO_MODE SID_PAL_RADIO_MODEM_MODE_LORA
#define RNET_MAC_BEACON_DATA_RATE SID_PAL_RADIO_DATA_RATE_22KBPS
#define RNET_MAC_BEACON_REGULATORY_NA SID_REG_CGI_2
#endif

/**
 * @brief Beacon sidewalk opt in/out enumerations
 */
typedef enum{
    RNET_MAC_BCN_OPTED_OUT = 0,
    RNET_MAC_BCN_OPTED_IN = 1
}rnet_mac_bcn_sidewalk_state_t;

/**
 * @brief Beacon cloud connection health enumerations
 */
typedef enum{
    RNET_MAC_BCN_CLOUD_DISCONNECTED = 0,
    RNET_MAC_BCN_CLOUD_CONNECTION_UNSTABLE = 1,
    RNET_MAC_BCN_CLOUD_CONNECTION_DISTRESSED = 2,
    RNET_MAC_BCN_CLOUD_CONNECTED = 3
}rnet_mac_bcn_cloud_connection_state_t;
/**
 * @brief Gateway's operating mode
 */
typedef enum{
    RNET_MAC_BCN_HDR_AND_LDR_MODE = 0,
    RNET_MAC_BCN_HDR_ONLY_MODE = 1
}rnet_mac_beacon_op_mode_t;

/**
 * @brief Beacon load indicator enumerations
 */
typedef enum{
    RNET_MAC_BCN_LOAD_INDICATOR_LOW = 0,
    RNET_MAC_BCN_LOAD_INDICATOR_MID_LOW = 1,
    RNET_MAC_BCN_LOAD_INDICATOR_MID_HIGH = 2,
    RNET_MAC_BCN_LOAD_INDICATOR_HIGH = 3
}rnet_mac_beacon_load_indicator_t;

static const time_def HDR_LDR_CYCLE_PERIOD_TIMEDEF = { 0, MS_TO_TUS(RNET_MAC_SLOT_DURATION_MS) };

#if !HALO_PAN_COORDINATOR
typedef struct {
    uint8_t id[RNET_DEV_ID_SZ];
    S16 rssi;
    S8  snr;
    time_def rxtime;
    struct bcn_cs_slot_schedule_t cs_sch;
} rnet_mac_beacon_last_received_t;

/**
 * @brief The consumption of beacon packets
 *
 * @param[in]   len       Length of input buffer
 * @param[in]   ptr       Ptr to input buffer
 * @param[in]   ts        Time stamp of the packet taken at the time of receiving the last symbol.
 *
 * @return   bool     Success or failure to consume the packet. The packet will be rejected if the packet is not a beacon packet or if the authentication fails.
 *
 */
const rnet_mac_beacon_last_received_t* rnet_mac_beacon_get_last_beacon_info(void);
#endif

typedef struct {
    // New V2 Beacon fields
    uint8_t frame_type : 3;
    uint8_t protocol_version : 4;
    uint8_t hdr_version;
    uint8_t src[RNET_DEV_ID_SZ];
    uint8_t src_sz;
    uint8_t dst[RNET_DEV_ID_SZ];
    uint8_t dst_sz;
    uint8_t opt_in : 1;
    uint8_t cloud_connect_health : 2;
    uint8_t load_indicator : 2;
    uint8_t op_mode : 1;
    uint8_t hdr_downlink_eirp : 4;
    uint8_t hdr_uplink_adj : 4;
    uint8_t hdr_hop_seed[RNET_MAC_CH_RANDOM_SEED_ARRAY_MAX_SIZE];
    struct bcn_cs_slot_schedule_t cs_sch;
    uint8_t *auth_tag;
    uint8_t  auth_tag_len;

    // HALO-6366 - Remove legacy beacon code
#if RNET_MAC_CHANNEL_HOPPING
    uint8_t bcn_channel_hopping_identifier;  // 1 byte Identifier for the CH algorithm used by GW
    uint8_t bcn_random_ch_seed[RNET_MAC_CH_RANDOM_SEED_ARRAY_MAX_SIZE]; //6 byte //The seed for CH
#else
    uint8_t bcn_packet_identifier;  // 1 byte
#endif
} beacon_packet_t;

/**
 * @brief Retrieves the cs_schedule announced with the last beacon.
 *
 * @param[out]   sch     Pointer to the struct the output is written into.
 */
void rnet_mac_beacon_get_last_csslot_schedule(struct bcn_cs_slot_schedule_t *sch);

/**
 * @brief Declares sync status and updates the current start frame time definition.
 *
 * @param[in]   bcn_activate_data     Pointer to rnet_mac_bcn_activate_data_t object,
 * defining the operation of the BCN submodule. For a GW, only the start_time is used
 * while the rest of the parameters are ignored.
 *
 * For an end device a NULL argument returns a failure to start. For a GW, a null argument
 * implies starting with the previously set parameters and assuming the current time as the
 * start time of the current frame.
 *
 *  If previously deactivated, activates beacon submodule ie.
 *      On GW, starts periodic beacon transmissions
 *      On end-node, starts scheduling for periodically wake-ups to listen to the beacon transmissions following the beacon transmission schedule.
 *  If called while active, changes last beacon start time definition.
 *  If called in the middle of an operation, updates the frame start time but finishes the current operation.
 *      On going operation can overwrite the frame start time if a beacon packet is received/transmitted as a result of current operation.
 *
 *  If provided time is in the past, the current frame start time will be updated considering the beacon in between are missed.
 *  Note that, this can trigger a rnet_mac_beacon_go_async action if the number exceeds max_num_times_bcn_is_missed
 *  @ret success/failure
 */
bool rnet_mac_beacon_activate(const rnet_mac_bcn_activate_data_t *bcn_activate_data);
/**
 * @brief Forces beacon submodule to go to async status.
 *  Deactivates beacon submodule. i.e.
 *      On GW, stops scheduling for beacons transmissions.
 *      On end-node, stops scheduling for periodically wakeups to listen to the beacon
 *  The module returns the radio resource and deactivates itself if called in the middle of operation. However, it is not thread safe. If any operation is preempted, the behavious is undefined.
 *  @ret success/failure
 */
bool rnet_mac_beacon_deactivate(void);

/**
 * @brief Initializes and starts the module
 * On GW, next beacon transmission is scheduled
 * On end-node,
 * if RNET_MAC_BEACON_SELF_SYNC ==1 ,  end node starts
 * otherwise , end node stays disabled until rnet_mac_beacon_go_sync is called
 * @ret SUBMOD_PROCESS_SUCCESS if the initialization is done
 */
rnet_submod_err_t rnet_mac_beacon_init(void);

/**
 * @brief Returns starting time of the current frame
 * @return Start time of the frame, the node is in based on current time as obtained by rnet_get_current_timedef(timedef *) method
 */
time_def rnet_mac_beacon_cur_frame_start(void);

/**
 * @brief Calculates the max expected time drift between two nodes
 * over the expected tolerance due to missing sync intervals.
 * param[out] tsptr    time sync error over existing tolerance
 */
void rnet_mac_beacon_time_sync_tol_expansion(time_def *tsptr);

/**
 * @brief Returns the total time on air of a beacon in frequency hopping mode including its preamble.
 * @return The time on air in ms. On error 0 is returned.
 *
 */
uint32_t rnet_mac_beacon_calculate_beacon_time(void);

#if HALO_PAN_COORDINATOR == 0
/**
 * @brief Parse error indicators
 */
typedef enum{
    RNET_MAC_BCN_PARSE_INPUT_PARAM_ERROR = -10,
    RNET_MAC_BCN_PARSE_FRAME_TYPE_MISMATCH = -12,
    RNET_MAC_BCN_PARSE_INCOMPATIBLE_SEED_SIZE = -13,
    RNET_MAC_BCN_PARSE_UNKNOWN_CHSID = -14,
}rnet_mac_bcn_parse_error_t;
#define RNET_MAC_BCN_IS_PARSE_SUCCESS(X) (X>0)
/**
 * @brief Parsing the received buffer into beacon packet.
 *
 * @param[in]     len           Length of input buffer
 * @param[in]     ptr           Ptr to input buffer
 * @param[out]    out_pckt_ptr  Output pointer where parsed contents will be written.
 * @param[in/out] tsp           Time stamp of the packet taken at the time of receiving the last
*                           symbol. If the packet is successfully parsed, the calculated
*                           frame start time is written into this variable. If NULL, start time calculation is omitted.
* @param[in]      channel       Channel over which the beacon was received.
*
* @return In the case of failure, a negative number indicating the failure reason is returned.
*         In the case of success, the size of buffer parsed is returned.
*/
S16 rnet_mac_beacon_parse_packet(uint16_t len, uint8_t *ptr, beacon_packet_t *out_pckt_ptr, time_def *tsp, uint8_t channel);
/**
 * @brief The authentication of beacon packets
 *
 * @param[in]   bcn_pkt_ptr    Pointer to the bcn packet to be authenticated
 * @param[in]   ptr            Ptr to input raw buffer. The initial idx bytes used for generating
 *                             the auth tag and the following RNET_MAC_BEACON_AUTH_SIZE bytes
 *                             should contain the received auth tag that will be compared with the
 *                             generated tag. The size of the buffer should be larget than
 *                             (idx+RNET_MAC_BEACON_AUTH_SIZE) bytes.
 * @param[in]   idx            The starting location of the auth tag withing the ptr buffer.
 * @param[in]   tag_len        Length of auth tag
 * @return      bool           Success or failure of authentication.
 *
 */
bool rnet_mac_beacon_authenticate(beacon_packet_t *bcn_pkt_ptr, uint8_t *ptr, uint16_t idx, uint8_t tag_len) ;
#if RNET_MAC_CHANNEL_HOPPING
/**
 * @brief Returns the radio configuration used by the beacon submodule.
 *
 * @param[out]   radio_config       The pointer to which the radio config will be written. If the pointer is NULL, it is not populated.
 * @param[out]   mode_ptr           The pointer to which the mode_ptr will be written. If the pointer is NULL, it is not populated.
 * @return   void.
 */
void rnet_mac_beacon_get_radio_config(sid_pal_radio_packet_cfg_t *radio_config, sid_pal_radio_modem_mode_t *mode_ptr);
#endif //RNET_MAC_CHANNEL_HOPPING
#endif //HALO_PAN_COORDINATOR == 0

#endif /* _H_SRC_MAC_HALO_INCLUDE_RNET_MAC_BEACON_H_  */
