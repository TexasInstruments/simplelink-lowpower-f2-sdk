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

#ifndef _H_SRC_MAC_HALO_RNET_MAC_SUBMODULE_COMMON_H_
#define _H_SRC_MAC_HALO_RNET_MAC_SUBMODULE_COMMON_H_

//#include "rnet_mac_phy_interface.h"
#include "rnet_event_handlers.h"
#include <sid_protocol_opts.h>
#include "rnet_mac_util.h"
#include "rnet_time.h"
#include "rnet_mac_channel_hopping.h"
#include "rnet_frames.h"
#include <sid_protocol_metrics.h>
#include "rnet_config.h"

#define RNET_MAC_CURRENT_TUS RNET_CURRENT_TUS
#define GET_TIMEDEF_RNET_MAC(X) RNET_GET_TIMEDEF(X)

typedef void * context_t;

typedef struct {
    uint8_t is_enabled: 1;
    uint8_t log_level: 3;
} wan_log_filter_t;

typedef struct {
    wan_log_filter_t beacon;
    wan_log_filter_t probe;
    wan_log_filter_t hdr;
    wan_log_filter_t ldr;
    wan_log_filter_t dfu;
    wan_log_filter_t event_list;
    wan_log_filter_t mac_cntlr;
    wan_log_filter_t gnr;
} wan_log_t;

extern volatile wan_log_t wan_log;

typedef enum
{
    MAC_STATUS_FAIL = -1,
    MAC_STATUS_SUCCESS = 0,
}rnet_mac_cmd_status_t;

typedef enum
{
    PRIORITY_0 = 0,
    PRIORITY_1,
    PRIORITY_2,
    PRIORITY_3,
    PRIORITY_4,
}rnet_mac_event_priority_t;

/* error list for init/enable submodule calls */
typedef enum {
    SUBMOD_PROCESS_SUCCESS,
    SUBMOD_PROCESS_FAIL,
    SUBMOD_REGION_UNSUPPORTED,
} rnet_submod_err_t;

struct rnet_mac_event_t;

typedef struct
{
    rnet_mac_cmd_status_t (*rnet_mac_submodule_start) (struct rnet_mac_event_t *); /*The callback that is called to activate the submodule */
    rnet_mac_cmd_status_t (*rnet_mac_submodule_main) (struct rnet_mac_event_t *); /*The callback that is called for subsequent rtc calls to the submodule */
    rnet_mac_cmd_status_t (*rnet_mac_submodule_stop) ( void ); /*The callback to request the submodule to stop. TODO: NOT IMPLEMENTED YET. WILL NOT BE EXERCIZED*/
    uint32_t (*rnet_mac_submodule_next_event_time) ( void ); /*Subsequent rtc scheduling by the submodule*/
    rnet_mac_cmd_status_t (*rnet_mac_submodule_reschedule_event) (struct rnet_mac_event_t *, time_def *); /*The callback that is called to reschedule an event because of conflict */
}rnet_mac_submodule_function_t;

// System drift settings
typedef struct {
    uint16_t max_drift_us; // maximum drift supported in micro sec
    uint16_t max_drift_per_sec; // rate at which the clock is drifting in microsec/sec
}rnet_mac_drift_settings_t;

extern bool metrics_rollover_flag;

struct __attribute__((__packed__)) slot_schedule_t{
    uint8_t offset;
    uint8_t period;
};

struct __attribute__((__packed__)) bcn_cs_slot_schedule_t {
    struct slot_schedule_t ul;
    struct slot_schedule_t dl;
};

typedef struct rnet_mac_event_t {
    RNET_LIST_ENTRY(rnet_mac_event_t);  /* Token used by timelist. */
    uint32_t start_time;                /* Absolute raw tu indicating the start time of event */
    uint32_t end_time;                  /* Absolute raw tu indicating the end time of event */
    bool is_added; /* Internal flag to determine if the event has been added to the event list */
    rnet_mac_submodule_t submodule_name;  /* The name of the submodule */
    const rnet_mac_submodule_function_t* submodule_function;  /* Pointer to callback functions of the submodule */
    rnet_mac_event_priority_t priority; /* Priority of the event */
    void (*context); /* Some context that might be used to identify the purpose of the event by the submodule */
}rnet_mac_event_t;

#define RNET_MAC_SLOT_DURATION_MS 63  //The amount of time spent for a full cycle of HDR and LDR preamble sampling periods in mili seconds
#define RNET_MAC_SCHEDULE_GUARD_TIME_MS 1
#define RNET_MAC_HDR_SHORT_RX_DURATION_MS 3
#define RNET_MAC_HDR_RX_DURATION_MS 4
#define RNET_MAC_LDR_RX_DURATION_MS (RNET_MAC_SLOT_DURATION_MS - RNET_MAC_HDR_RX_DURATION_MS - RNET_MAC_SCHEDULE_GUARD_TIME_MS)
#define RNET_MAC_DFU_RX_DURATION_MS 6
#define RNET_MAC_NUMBER_OF_SLOTS_PER_FRAME 160 //Number of HDR/LDR cycles in a frame. The first HDR/LDR period is used by Beacon
#define RNET_MAC_FRAME_PERIOD_MS (uint64_t)RNET_MAC_NUMBER_OF_SLOTS_PER_FRAME * (uint64_t)RNET_MAC_SLOT_DURATION_MS
#define RNET_MAC_NUMBER_OF_SLOTS_PER_GROUP_FRAME 10

#define RNET_MAC_FIRST_HDR_SLOT_NUM 3
#define RNET_MAC_LAST_HDR_SLOT_NUM 158

#define MAX_RADIO_STUCKCNT 10

#define RNET_MAC_IS_SLOT_BEYOND_FRAME_BOUNDARY(X) (X >= RNET_MAC_NUMBER_OF_SLOTS_PER_FRAME)
#define RNET_MAC_IS_VALID_HDR_SLOT(X) ( \
        (X%RNET_MAC_NUMBER_OF_SLOTS_PER_FRAME) >= RNET_MAC_FIRST_HDR_SLOT_NUM && \
        (X%RNET_MAC_NUMBER_OF_SLOTS_PER_FRAME) <= RNET_MAC_LAST_HDR_SLOT_NUM      )

static const time_def BEACON_PERIOD_TIMEDEF = {
    .secs = ( RNET_MAC_FRAME_PERIOD_MS / 1000),
    .tu = (U16)(MS_TO_TUS(RNET_MAC_FRAME_PERIOD_MS % 1000))
};

// Maximum allowable time of 152.5us as difference between current time and past time
#define MAX_PAST_TIME_DIFF_IN_TUS             5

#define RNET_MAC_RADIO_SLEEP_WAKEUP_TIME 13

#define RNET_MAC_PHASE_0_GROUP_PERIODICITY      1
#define RNET_MAC_PHASE_0_UNICAST_PERIODICITY    4

/* wakeup schedules for HDR */
typedef enum {
    GW_WAKEUP,                         //every 63 ms
    END_DEV_WAKEUP,                    //every n*63 ms - TODO: separate schd for lights and fetch
    GRP_WAKEUP,                        //every 63 ms
    END_DEV_CS_WAKEUP,                 //CS Slot schedule
#if HALO_JOIN_PROCESS
    END_DEV_PRE_JOIN_WAKEUP           //every 63ms
#endif
} rnet_hdr_wakeup_schd_t;

/* slot scheduling parameters */
typedef struct {
    rnet_hdr_wakeup_schd_t wakeup_schd;
    uint8_t offset;
    uint8_t prdcty;
}hdr_slot_schdl_t;

#define GNR_LOG_DEBUG(...)                                                         \
    if (wan_log.gnr.is_enabled && wan_log.gnr.log_level >= SID_HAL_LOG_SEVERITY_DEBUG) \
    {                                                                              \
        SID_HAL_LOG_DEBUG(__VA_ARGS__);                                                \
    }

#define GNR_LOG_INFO(...)                                                         \
    if (wan_log.gnr.is_enabled && wan_log.gnr.log_level >= SID_HAL_LOG_SEVERITY_INFO) \
    {                                                                             \
        SID_HAL_LOG_INFO(__VA_ARGS__);                                                \
    }

#define GNR_LOG_WARNING(...)                                                          \
    if (wan_log.gnr.is_enabled  && wan_log.gnr.log_level >= SID_HAL_LOG_SEVERITY_WARNING) \
    {                                                                                 \
        SID_HAL_LOG_WARNING(__VA_ARGS__);                                                 \
    }

#define GNR_LOG_ERROR(...)                                                          \
    if (wan_log.gnr.is_enabled) \
    {                                                                               \
        SID_HAL_LOG_ERROR(__VA_ARGS__);                                                 \
    }

/*!
 * @brief Checks for undefined schedules
 * @param[in] scp: Slot schedule to be checked
 * @return True if either the input pointer is null or the schedule is undefined. False otherwise.
*/
bool rnet_mac_is_undefined_slot_schedule(struct slot_schedule_t const *scp);

/*!
 * @brief Checks for slot schedules explicitly disables access
 * @param[in] scp: Slot schedule to be checked
 * @return True if either the input pointer if null or the input schedule is disabled. False otherwise
*/
bool rnet_mac_is_disabled_slot_schedule(struct slot_schedule_t const *scp);

/*!
 * @brief Checks whether bcn_cs_slot schedule allows FSK_WAN use
 * @param[in] scp: BCN CS Slot schedule to be checked
 * @return True if the input pointer is not null and the input cs schedule is fsk wan capable. False otherwise
*/
bool rnet_mac_is_fskwan_capable_slot_schedule(struct bcn_cs_slot_schedule_t const *scp);

/*!
 * @brief Function for the active SubModule to indicate end of its event operation
 * @param Submodule making the call
 * @return SUCCESS or FAIL
*/
rnet_mac_cmd_status_t rnet_mac_handler_done(rnet_mac_submodule_t submodule_name);

#if defined(GW_SUPPORT) && GW_SUPPORT
/*!
 * @brief Calculates the slot information depending on whether pkt is multicast or unicast.
 * For end devices, grp slot info is stored in flash. Gateway gets this info from cloud/host per end device.
 * @param[in/out] rnet_hdr_wakeup_schd_t schd: wakeup schedule based on dev and pkt type
 * @param[in] rnet_pkt_t* pkt_ptr : tx pkt pointer to get group number and ringnet id
 * @return void
*/
void get_next_slot_info(hdr_slot_schdl_t* schdl_params, rnet_pkt_t* pkt_ptr);
#endif
/*!
 * @brief Function to get the next slot time of the submodule in the sync mode
 * @param submodule_name name of the submodule which is requesting the slot time
 * @param[in] event_future_time pointer to a future time in timedef after which the slot time must
 *  be computed. If NULL then slot immediately after current time is selected
 *  @param schd_params supporting params for the next grp/unicast slot.
 *  @param[out] channel Channel that is used in the calculated slot time for HDR.
 *              Works only for the HDR submodule. If the submodule_name is not HDR, the pointer is not modified.
 *              If the input pointer is NULL, the channel is not calculated.
 *  @param[out] out_slot_num The slot number with respect to the start of the current frame.
 *              In the case of a the calculated slot is after the next beacon, the returned value will be larger than the number of slots in a frame.
 *              If the pointer is NULL, the slot number is not populated.
 * @return start of submodule event time in timedef
*/
time_def rnet_mac_get_next_slot_time(rnet_mac_submodule_t submodule_name, const time_def *event_future_time, hdr_slot_schdl_t schd_params, uint8_t* channel,  uint16_t* out_slot_num);

/*!
 * @brief Add a new event in the list
 * @param event pointer to an Event to be added to the sorted list
 * @return SUCCESS or FAIL
*/
rnet_mac_cmd_status_t rnet_add_mac_event( rnet_mac_event_t *event);

/*!
 * @brief Remove an event from the list
 * @param event pointer to the Event which needs to be removed from the Event list
 * @return SUCCESS or FAIL
*/
rnet_mac_cmd_status_t rnet_remove_mac_event( rnet_mac_event_t *event);

/*
 * @brief: function to get the raw rf buffer pointer
 *
 * @param[in-out] pointer to carry-out buffer size
 * @return: buffer pointer
 */
uint8_t* rnet_get_raw_rf_buffer(uint8_t* sz);

/**
 * @brief Max time drift between two nodes provided they sync every beacon interval
 *
 * param[out] tsptr    Maximum time sync error
 */
void rnet_mac_time_sync_tolerance(time_def* tsptr);

/**
 * @brief Get HDR RX period based on phy rx timeout event delay
 * HALO-14924: LR1110 takes longer time on radio processing than SX126x
 * In this delay case, use short HDR rx duration for pulling in the LDR start rx timing.
 * @param void
 * @return hdr rx period in ms
*/
uint32_t rnet_mac_get_hdr_rx_period_ms(void);

/*!
 * @brief this is invoked to indicate radio status. If number of erros exceed threshold radio reset is called
 * @param submodule_id: submodule which has radio  event
 * @param status: true if no radio error false if radio error is encountered
 */
void rnet_mac_radio_error(rnet_mac_submodule_t submodule_id, bool status);

#endif
