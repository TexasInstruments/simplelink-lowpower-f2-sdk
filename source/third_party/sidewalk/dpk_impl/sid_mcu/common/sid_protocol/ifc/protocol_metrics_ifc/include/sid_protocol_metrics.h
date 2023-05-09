/*
 * Copyright 2018-2020 Amazon.com, Inc. or its affiliates. All rights reserved.
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

#ifndef SID_PROTOCOL_METRICS_H
#define SID_PROTOCOL_METRICS_H

#include <rnet_errors.h>
#include <sid_protocol_defs.h>
#include <sid_time_types.h>

#include <stdbool.h>
#include <stdint.h>

#if defined(GW_SUPPORT) && GW_SUPPORT
#define SCHEDULER_TIMING_ARRAY 4
#define ZWAVE_METRICS_ARRAY_SIZE 12
#endif

//version for the EP2P frames
#define EP2P_METRICS_VERSION 0

//version for the noise floor frames
#define NOISEFLOOR_METRICS_VERSION 0

// version for the LDR PHY improvement frames
#define LDR_PHY_IMP_METRICS_VERSION 0

//FSK channels to num of buckets
#define NUM_CHANNELS_PER_BKT 4
#define GET_CHANNELINFO_CNT(x) (((x) + NUM_CHANNELS_PER_BKT - 1) / NUM_CHANNELS_PER_BKT)

// Array size for Reed Solomon histogram, should be  (NPSYM/2 + 1)
// TODO: Replace constant to corresponding define after RS is integrated
#define RS_HISTOGRAM_ARRAY_SIZE 17

typedef struct __attribute__((packed)) {
    uint16_t total_rx_time;
    uint16_t total_tx_time;
    uint8_t  ble_adv_time_h;
    uint16_t ble_adv_time_l;
    uint16_t ble_connection_time;
    uint16_t leds_on_time;
} battery_usage_report_t;

typedef enum {
    METRICS_TIME_CAD,
    METRICS_TIME_STANDBY,
    METRICS_TIME_SLEEP,
    METRICS_TIME_HDR_RX,
    METRICS_TIME_HDR_TX,
    METRICS_TIME_LDR_RX,
    METRICS_TIME_LDR_TX,
    METRICS_TIME_BEACON,
#if !defined(GW_SUPPORT) || !GW_SUPPORT
    METRICS_TIME_ASYNC,
    METRICS_TIME_SYNC,
    METRICS_TIME_DISCOVERY_CUM_ACTIVE_DUR,
    METRICS_TIME_DISCOVERY_CUM_OPERATION_DUR,
#else
    METRICS_TIME_DFU_TX,
    METRICS_TIME_DFU_RX,

#endif
    METRICS_TIME_ARRAY_SIZE,
} metrics_time_types_t;

typedef struct {
    uint32_t total_CADs;
    uint32_t total_rx_pkts;
    uint32_t total_tx_pkts;
    uint32_t total_retransmissions;
    uint32_t total_rx_other_PAN;
    uint32_t channel_busy_events;
    uint32_t CRC_errors;
    uint32_t bad_decode;
    uint32_t security_errors;
    uint32_t decrypt_errors;
    uint32_t auth_errors;
    uint32_t gcs_drift_status;
} p2p_metrics_t;

typedef struct {
    //HDR Time Sync Metrics
    uint32_t time_sync_hdr_relative_rx_time_num_samples; //Mean time sync error on synchronously received packets.
    float time_sync_hdr_relative_rx_time_mean; //Mean time sync error on synchronously received packets.
    float time_sync_hdr_relative_rx_time_S; // n times the variance of time sync error on synchronously received packets. The reason to keep S is for numerical stability.
    uint16_t hdr_rx_drop;                       //total rx packets dropped on hdr
    uint16_t timer_metrics_hdr_rx;              //placeholder for metrics_time_types_t METRICS_TIME_HDR_RX
    uint8_t hdr_tx_fail;                        // num of tx fails
    uint16_t hdr_tx_ack_pkts;                   // num of acks sent
    uint8_t hdr_tx_ack_timeouts;                // missed acks
    uint16_t timer_metrics_hdr_tx;              //placeholder for metrics_time_types_t METRICS_TIME_HDR_TX
    uint16_t hdr_rx_timeouts;                   //num of rx timeouts
    uint16_t ldr_rx_pkts;                       // num of ldr rx pkts
    uint16_t ldr_tx_pkts;                       // num of ldr tx pkts
    uint16_t ldr_tx_fail;                       // num of ldr tx fails
    uint16_t timer_metrics_ldr_tx;              //placeholder for metrics_time_types_t METRICS_TIME_LDR_TX
    uint16_t timer_metrics_ldr_rx;              //placeholder for metrics_time_types_t METRICS_TIME_LDR_RX
    uint16_t ldr_rx_error;
    uint16_t ldr_rx_hdr_error;
    uint16_t ldr_rx_preamble_error;
    int16_t  ldr_avg_error_rssi;
    int16_t  ldr_avg_prmb_error_rssi;
#if !defined(GW_SUPPORT) || !GW_SUPPORT
    /*BCN Metrics*/
    uint32_t total_bcn_miss; //Number of times the beacon is missed
    uint32_t async_trigger; //Number of times the async is triggered

    //Beacon Time Sync Metrics
    uint32_t time_sync_bcn_relative_fst_num_samples; //Mean time sync error on synchronously received packets.
    float time_sync_bcn_relative_fst_mean; //Mean time sync error on synchronously received packets.
    float time_sync_bcn_relative_fst_S; // n times the variance of time sync error on synchronously received packets. The reason to keep S is for numerical stability.
    /*Gateway Discovery Metrics*/
    uint16_t gwd_invoked_cnt;
    int16_t  hdr_avg_rssi;
    int16_t  ldr_avg_rssi;

    //The value reported iin the following metrics has overflow protection. I.e. If the duration is larger than
    //or equal to max the corresponding data type allows, the maximum for the data type is populated to indicate overflow.
    uint16_t total_time_spent_in_discovery;          //Cumulative amount of time (s) spent in bcn searching state in units of seconds.
    uint16_t unused_radio_time_in_active_discovery;  //The amount of time (ms) discovery submodule didn't use the radio while it was active.
    uint16_t num_of_times_discovery_is_activated;    //Total number of times discovery sampling is activated to search for the beacons.
    uint32_t num_of_channels_scanned_by_discovery;   //Total number of times discovery performed channel sampling.
    uint16_t num_of_preamble_detections_by_discovery;//Number of preamble detections without valid pkt reception by the discovery submodule.
    uint16_t num_of_pckt_detections_by_discovery;    //Number of non-Beacon type valid FSK packet receptions by the discovery submodule.
    uint16_t num_of_GW_detections_by_discovery;      //Number of valid FSK beacon packet detections by the discovery submodule.
#else
    uint32_t ldr_tx_reschedule_epoch;
    uint32_t ldr_tx_max_retries;
    uint8_t start_dfu;
    uint8_t stop_dfu;
    uint16_t dfu_rx_pkts;
    uint16_t dfu_tx_pkts;
    uint8_t dfu_nrf_timeouts;
    uint16_t timer_metrics_dfu_rx;           //placeholder for metrics_time_types_t METRICS_TIME_DFU_RX
    uint16_t timer_metrics_dfu_tx;           //placeholder for metrics_time_types_t METRICS_TIME_DFU_TX
    uint16_t bcn_tx_cnt;
#endif
    uint16_t radio_watchdog_count;
    uint16_t hdr_cad_tx_drop;
    uint16_t bcn_pkts_dcr_check_fail; //Beacon
    uint16_t ll_pkts_dcr_check_fail; //ll acks
    uint16_t hdr_pkts_dcr_check_fail; //HDR packets
    uint16_t ldr_pkts_dcr_check_fail; //LDR packets

} enhanced_p2p_metrics_t;

typedef struct {
    // Number of packets scheduled for transmission, retry attemps are not included
    uint16_t schedule_tx_pkts;
    // Number of duplicate packets detected
    uint16_t duplicate_pkts;

#if defined(GW_SUPPORT) && GW_SUPPORT
    // Histogram of total number of symbols error that were detected upon packet reception
    uint16_t rs_sym_err_cnt[RS_HISTOGRAM_ARRAY_SIZE];
    // Number of ACKs received during DL at Gateway
    uint16_t dl_ack_rx_pkts;
    // Number of ACKs missed during DL at Gateway
    uint16_t dl_ack_miss_pkts;
#else
    // Number of ACKs transmitted by End-node during DL
    uint16_t dl_ack_tx_pkts;
    // Total number of LDR network sync attempts
    uint16_t ldr_sync_tx_pkts;
    // Total number of successful LDR sync
    uint16_t ldr_sync_success_cnt;
    // Total number of LDR sync fails
    uint16_t ldr_sync_fail_cnt;
    // Total number of LDR network disconnects
    uint8_t ldr_network_disconnect_cnt;
#endif
} ldr_phy_imp_metrics_t;

typedef void (*on_metrics_rollover_t)(void * ctx);
void metric_increment_rollover_protection_uint8(uint8_t* ptr);
void metric_increment_rollover_protection_uint16(uint16_t* ptr);
void metric_increment_rollover_protection_uint32(uint32_t* ptr);
void metric_time_sync_hdr_relative_rx_update(float relative_rx_time);
void metric_time_sync_bcn_relative_fst(uint32_t relative_fst_int);
void metric_ldr_rx_error(int16_t rssi);
void metric_ldr_rx_done(int16_t rssi);
void rnet_tsync_update_metrics(struct sid_timespec * diff);
void metric_total_retransmissions(uint32_t retries);
void metric_on_metrics_rollover_set(on_metrics_rollover_t cb, void * ctx);
enhanced_p2p_metrics_t * metric_ep2p_metrics_get(void);
p2p_metrics_t * p2p_metrics_get(void);
ldr_phy_imp_metrics_t *ldr_phy_imp_metrics_get(void);

/* evaluates to false if a value reached overflow value uint32_t */
#define SAMPLE_CNT_OVERFLOW(x) ((x + 1) > 0)

/* evaluates to false if a value reached overflow value uint16_t */
#define SAMPLE_CNT_OVERFLOW16(x) ((uint16_t)(x + 1) > 0)

#if defined(GW_SUPPORT) && GW_SUPPORT

#include <sid_protocol_regulatory_config.h>

/*
 * Scheduler metrics reserve 1 more element to include "no event" (NO_SUBMODULE)
 * metrics, "no event" metrics is placed at the last element of the Tx buffer.
 */
#define SCHEDULER_METRICS_ARRAY_SZ (NUM_SUBMODULES + 1)
#define SCHEDULER_METRICS_TX_END_SUBMODULE (DISCO_SUBMODULE)
#define SCHEDULER_METRICS_NO_EVENT_IDX (NUM_SUBMODULES)

typedef struct {
//Scheduler Metrics
    uint32_t num_of_events[SCHEDULER_METRICS_ARRAY_SZ];
    uint32_t deficit_conflict[SCHEDULER_METRICS_ARRAY_SZ];
    uint32_t priority_conflict[SCHEDULER_METRICS_ARRAY_SZ];

    //Scheduler Timing Histogram
    uint16_t hdr_process_time[SCHEDULER_TIMING_ARRAY];
    uint16_t ldr_process_time[SCHEDULER_TIMING_ARRAY];
} __attribute__((packed)) enhanced_scheduler_metrics_t;

extern enhanced_scheduler_metrics_t ep2p_sched_metrics;

typedef enum {
    PHY_MODE_LORA_500KHz = 0,
    PHY_MODE_FSK_156KHz = 1
} channel_reportphy_mode_t;

typedef struct {
    uint32_t  sample_count;
    float     avg_rssi_value;
    float     avg_rssi_std;
} __attribute__((packed)) channel_rssi_info;

#define MAX_CHANNELINFO_CNT GET_CHANNELINFO_CNT(SID_REG_FSK_CHANNELINFO_SIZE)
extern channel_rssi_info fsk_channel_info[MAX_CHANNELINFO_CNT];
extern channel_rssi_info ldr_channel_info;
extern uint8_t channel_info_maxcnt;

#define ZWAVE_METRICS_ARRAY_SIZE 12

typedef struct {
    uint16_t zwave_rssi[ZWAVE_METRICS_ARRAY_SIZE];
    uint16_t zwave_bp_counter;
} __attribute__((packed)) dfu_zwave_metrics_t;

extern dfu_zwave_metrics_t zwave_metrics;
#endif


#if METRICS_ENABLED
void metrics_init(void);
void metrics_time_start(metrics_time_types_t type);
void metrics_time_stop(metrics_time_types_t type);
uint32_t metrics_time_get_sec(metrics_time_types_t type);
void clear_p2p_metrics(void);
uint8_t fill_p2p_metrics_pld(uint8_t *buf);
void metrics_error_set(rnet_error_t type);
void clear_p2p_usage_reports(void);
uint8_t fill_usage_report_pld(uint8_t *pld);
uint8_t fill_ep2p_metrics_pld(uint8_t *buf, uint16_t buffer_length);
void clear_ep2p_metrics(void);
void clear_ldr_phy_imp_metrics(void);
#if defined(GW_SUPPORT) && GW_SUPPORT
void metric_set_fsk_channel_info_max_count(uint8_t maxcnt);
uint8_t fill_noise_floor_metrics_data(uint8_t *buf, uint16_t buffer_length, uint8_t ldr_chnl);
void clear_noise_floor_metrics(void);
uint8_t fill_gateway_scheduler_metrics_data(uint8_t *buf,uint16_t buffer_length);
void clear_scheduler_metrics(void);
uint8_t fill_gateway_ldr_phy_imp_metrics_data(uint8_t *buf, uint16_t buffer_length);
void clear_gw_ldr_phy_imp_metrics(void);
uint8_t fill_zwave_metrics(uint8_t *buf, uint16_t buffer_length);
void clear_zwave_metrics(void);
#else
void clear_ldr_metrics(void);
void clear_end_device_ldr_metrics(void);
uint8_t fill_end_device_ldr_metrics_data(uint8_t *buf,uint16_t buffer_length);
void clear_end_device_ldr_phy_imp_metrics(void);
uint8_t fill_end_device_ldr_phy_imp1_metrics_data(uint8_t *buf, uint16_t buffer_length);
#endif //defined(GW_SUPPORT) && GW_SUPPORT
#else
static inline void metrics_init(void) { }
static inline void metrics_time_start(metrics_time_types_t type) {(void )type;}
static inline void metrics_time_stop(metrics_time_types_t type) {(void) type;}
static inline uint32_t metrics_time_get_sec(metrics_time_types_t type) {(void) type; return 0;}
static inline void clear_p2p_metrics(void) {}
static inline uint8_t fill_p2p_metrics_pld(uint8_t *buf) {(void) buf; return 0;}
static inline void metrics_error_set(rnet_error_t type) {(void) type; }
static inline void clear_p2p_usage_reports(void) {}
static inline uint8_t fill_usage_report_pld(uint8_t *pld) {(void) pld; return 0;}
static inline uint8_t fill_ep2p_metrics_pld(uint8_t *buf, uint16_t buffer_length) {(void) buf; (void) buffer_length; return 0;}
static inline void clear_ep2p_metrics(void) {}
static inline void clear_ldr_phy_imp_metrics(void) {}
#if defined(GW_SUPPORT) && GW_SUPPORT
static inline void metric_set_fsk_channel_info_max_count(uint8_t maxcnt) {(void)maxcnt;}
static inline uint8_t fill_noise_floor_metrics_data(uint8_t *buf, uint16_t buffer_length, uint8_t ldr_chnl) {(void)buf; (void)buffer_length; (void)ldr_chnl; return 0;}
static inline void clear_noise_floor_metrics(void) {}
static inline uint8_t fill_gateway_scheduler_metrics_data(uint8_t *buf,uint16_t buffer_length) {(void)buf; (void)buffer_length; return 0;}
static inline void clear_scheduler_metrics(void) {}
static inline uint8_t fill_gateway_ldr_phy_imp_metrics_data(uint8_t *buf, uint16_t buffer_length) {(void)buf; (void)buffer_length; return 0;}
static inline void clear_gw_ldr_phy_imp_metrics(void) {}
static inline uint8_t fill_zwave_metrics(uint8_t *buf, uint16_t buffer_length) {(void)buf; (void)buffer_length; return 0;}
static inline void clear_zwave_metrics(void) {}
#else
static inline void clear_ldr_metrics(void) {}
static inline void clear_end_device_ldr_metrics(void) {}
static inline uint8_t fill_end_device_ldr_metrics_data(uint8_t *buf,uint16_t buffer_length) {(void) buf; (void) buffer_length; return 0;}
static inline void clear_end_device_ldr_phy_imp_metrics(void) {}
static inline uint8_t fill_end_device_ldr_phy_imp1_metrics_data(uint8_t *buf, uint16_t buffer_length) {(void) buf; (void) buffer_length; return 0; }
#endif // defined(GW_SUPPORT) && GW_SUPPORT
#endif //METRICS_ENABLED

#endif // METRICS_H
