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

#ifndef RNET_APP_LAYER_H
#define RNET_APP_LAYER_H

#include <rnet_errors.h>
#include <sid_protocol_defs.h>
#include <sid_protocol_opts.h>
#include <storage_kv_keys.h>
#include "rnet_config.h"
#include "rnet_frames.h"
#include "rnet_transact_store.h"

#define RNET_APP_LAYER_VERSION	                (0x01)

#define RNET_MAX_CMD_DATA_LEN                   48
#define RNET_MAX_KA_INT_SEC                     86400

typedef union {
	struct {
	  U8            rsvd:2;
	  U8            tmi:1;
	  U8            rsi:1;
	  U8            bli:1;
	  U8            lei:1;
	  U8            asdi:1;
	  U8            sci:1;
	};
	U8 	        byte;
} rnet_cmd_data_hdr_t;

typedef struct {
  U16           class_id;
  U16           id;
  U8            data_hdr;
  U8            data_len;
  U8            expl_data_len;
  U8            seq_no_sz;
  U8            status_code;
  U8*           data;
  U32           seq_no;
  U8            txn_idx;
  U8            resp_req:1;
  U8            executed:1;
  U8            resp_vld:1;
  U8            reserved:5;
#if UNIFIED_ASL_SUPPORT
  rnet_unified_asl_nw_data_t *nw_data;
#endif

} rnet_cmd_t;

typedef enum {
    CMD_MGMT_KA_CHARGING_STATUS_DISCHARGING = 0,    //Cable not connected
    CMD_MGMT_KA_CHARGING_STATUS_CHARGING    = 1,    //Charging
    CMD_MGMT_KA_CHARGING_STATUS_FULL_CHARGE = 2     //Full charge when cable connected
}rnet_cmd_mgmt_ka_charging_status_t;

typedef enum {
    RNET_GEOFENCE_STATE_UNKNOWN = 0x02,
    RNET_GEOFENCE_STATE_OUTSIDE = 0x01,
    RNET_GEOFENCE_STATE_INSIDE  = 0x00,
} rnet_geofence_state_t;

typedef PACKED_STRUCT {
    int8_t rssi;

/*  TODO Add SNR to command after adding  protocol version  */
#if defined(WAN_SUPPORT) && WAN_SUPPORT
    int8_t snr;
#endif
    uint8_t battery_lvl;
#if !defined(WAN_SUPPORT) || !WAN_SUPPORT
    uint16_t battery_voltage;
#endif
    uint8_t app_status_type;
    union {
#ifdef RING_PRODUCT_BUDDY
        PACKED_STRUCT {
            uint8_t geofence0_status    : 2;
            uint8_t charging_status     : 2;
            uint8_t motion_status       : 2;
            uint8_t sound_on            : 1;
            uint8_t led_on              : 1;
            uint8_t reserved1           : 2;
            uint8_t geofence3_status    : 2;
            uint8_t geofence2_status    : 2;
            uint8_t geofence1_status    : 2;
        } type_0x02;
#else
#ifdef RING_PRODUCT_BRODY
        PACKED_STRUCT {
            uint8_t geofence0_status    : 2;
            uint8_t charging_status     : 2;
            uint8_t motion_status       : 2;
            uint8_t sound_on            : 1;
            uint8_t led_on              : 1;
            uint16_t battery_voltage;
            int16_t  temperature;
        } type_0x03;
#else
        PACKED_STRUCT {
            uint8_t als_enabled:1;
            uint8_t glow_mode:1;
            uint8_t motion_enabled:1;
            uint8_t led_status:1;
            uint8_t reserved:4;
            uint32_t timeout;
            uint8_t brightness;
            uint8_t motion_sensitivity;
            uint8_t als_threshold;
        } type_0x01;
#endif
#endif
    } app_status;
} get_status_t;

typedef uint8_t (*hw_id_func_t)(void *ctx);

// Macros for different command classes
#define CMD_CLASS_RINGNET_MGMT                  0
#define CMD_CLASS_ALERT                         1
#define CMD_CLASS_LIGHT                         2
#define CMD_CLASS_MOTION_SENSOR_PIR             3
#define CMD_CLASS_LIGHT_SENSOR                  4
#define CMD_CLASS_SECURITY                      5
#define CMD_CLASS_BATTERY                       6

#define CMD_CLASS_HALO_LOGGING                  0xA
#define CMD_CLASS_HALO_METRICS                  0xB
#define CMD_CLASS_BEAMS_MODES                   0x10
#define CMD_CLASS_SDB                           0x5B
#define CMD_CLASS_UASL_CUSTOM                   0x1
#define CMD_CLASS_CUSTOM                        0x8
#define CMD_CLASS_HALO_GATEWAY                  255

#define CMD_CLASS_RINGNET_SNDRCV                192

// Macros for different commands in LIGHT command class
#define CMD_ID_LIGHT_TURN_ON                    ((0x0 << 2) | RNET_ASL_OPC_WR)
#define CMD_ID_LIGHT_NOTIFY_TURN_ON             ((0x0 << 2) | RNET_ASL_OPC_NTFY)
#define CMD_ID_LIGHT_RESP_TURN_ON               ((0x0 << 2) | RNET_ASL_OPC_RSP)
#define CMD_ID_LIGHT_TURN_OFF                   ((0x1 << 2) | RNET_ASL_OPC_WR)
#define CMD_ID_LIGHT_NOTIFY_TURN_OFF            ((0x1 << 2) | RNET_ASL_OPC_NTFY)
#define CMD_ID_LIGHT_RESP_TURN_OFF              ((0x1 << 2) | RNET_ASL_OPC_RSP)
#define CMD_ID_LIGHT_SET_TIMEOUT                ((0x2 << 2) | RNET_ASL_OPC_WR)
#define CMD_ID_LIGHT_RESP_TIMEOUT               ((0x2 << 2) | RNET_ASL_OPC_RSP)
#define CMD_ID_LIGHT_SET_BRIGHTNESS_ON_MOTION   ((0x3 << 2) | RNET_ASL_OPC_WR)
#define CMD_ID_LIGHT_RESP_BRIGHTNESS_ON_MOTION  ((0x3 << 2) | RNET_ASL_OPC_RSP)


#define CMD_ID_LIGHT_GET_STATUS                 ((0x6 << 2) | RNET_ASL_OPC_RD)
#define CMD_ID_LIGHT_CUSTOM_CMD                 ((0x7 << 2) | RNET_ASL_OPC_WR)


// Macros for different commands in ALERT command class
#define CMD_ID_ALERT_MOTION_PIR                 ((0x0 << 2) | RNET_ASL_OPC_NTFY)
#define CMD_ID_ALERT_MOTION_PIR_STOPPED         ((0x1 << 2) | RNET_ASL_OPC_NTFY)
#define CMD_ID_ALERT_LOW_BATTERY                ((0x2 << 2) | RNET_ASL_OPC_NTFY)
#define CMD_ID_MOTION_IN_PROGRESS               ((0x3 << 2) | RNET_ASL_OPC_NTFY)
#define AMBIENT_LIGHT_CHANGE                    ((0x4 << 2) | RNET_ASL_OPC_NTFY)


// Macros for different commands in MOTION_PIR command class
#define CMD_ID_MOTION_PIR_ENABLE                ((0x0 << 2) | RNET_ASL_OPC_WR)
#define CMD_ID_MOTION_RESP_PIR_ENABLE           ((0x0 << 2) | RNET_ASL_OPC_RSP)
#define CMD_ID_MOTION_PIR_DISABLE               ((0x1 << 2) | RNET_ASL_OPC_WR)
#define CMD_ID_MOTION_RESP_PIR_DISABLE          ((0x1 << 2) | RNET_ASL_OPC_RSP)
#define CMD_ID_MOTION_GET_SENSITIVITY           ((0x2 << 2) | RNET_ASL_OPC_RD)
#define CMD_ID_MOTION_SET_SENSITIVITY           ((0x2 << 2) | RNET_ASL_OPC_WR)
#define CMD_ID_MOTION_RESP_SENSITIVITY          ((0x2 << 2) | RNET_ASL_OPC_RSP)


// Macros for different commands in LIGHT_SENSOR command class
#define CMD_ID_LIGHT_SENSOR_ENABLE              ((0x00 << 2) | RNET_ASL_OPC_WR)
#define CMD_ID_LIGHT_SENSOR_RESP_ENABLE         ((0x00 << 2) | RNET_ASL_OPC_RSP)
#define CMD_ID_LIGHT_SENSOR_DISABLE             ((0x01 << 2) | RNET_ASL_OPC_WR)
#define CMD_ID_LIGHT_SENSOR_RESP_DISABLE        ((0x01 << 2) | RNET_ASL_OPC_RSP)
#define CMD_ID_LIGHT_SENSOR_GET_THRESHOLD       ((0x02 << 2) | RNET_ASL_OPC_RD) // day mode / glow mode
#define CMD_ID_LIGHT_SENSOR_SET_THRESHOLD       ((0x02 << 2) | RNET_ASL_OPC_WR)
#define CMD_ID_LIGHT_SENSOR_RESP_THRESHOLD      ((0x02 << 2) | RNET_ASL_OPC_RSP)


// Macros for different commands in CUSTOM command class
#define CMD_ID_CUSTOM_GET                       ((0x00 << 2) | RNET_ASL_OPC_RD)
#define CMD_ID_CUSTOM_SET                       ((0x00 << 2) | RNET_ASL_OPC_WR)
#define CMD_ID_CUSTOM_NOTIFY                    ((0x00 << 2) | RNET_ASL_OPC_NTFY)
#define CMD_ID_RESP_CUSTOM                      ((0x00 << 2) | RNET_ASL_OPC_RSP)


// Macros for different commands in BEAMS_MODES command class
#define CMD_ID_TURN_ON_D2D_MODE                      ((0x2 << 2) | RNET_ASL_OPC_WR)
#define CMD_ID_RESP_TURN_ON_D2D_MODE                 ((0x2 << 2) | RNET_ASL_OPC_RSP)
#define CMD_ID_TURN_OFF_D2D_MODE                     ((0x3 << 2) | RNET_ASL_OPC_WR)
#define CMD_ID_RESP_TURN_OFF_D2D_MODE                ((0x3 << 2) | RNET_ASL_OPC_RSP)
#define CMD_ID_TURN_ON_SNOOZE_MODE                   ((0x4 << 2) | RNET_ASL_OPC_WR)
#define CMD_ID_RESP_TURN_ON_SNOOZE_MODE              ((0x4 << 2) | RNET_ASL_OPC_RSP)
#define CMD_ID_TURN_OFF_SNOOZE_MODE                  ((0x5 << 2) | RNET_ASL_OPC_WR)
#define CMD_ID_RESP_TURN_OFF_SNOOZE_MODE             ((0x5 << 2) | RNET_ASL_OPC_RSP)


// which class?
#define CMD_ID_UNIT_ID_CHANGE                   ((0x0C << 2) | RNET_ASL_OPC_WR)
#define CMD_ID_ANNOUNCE_ID                      ((0x0D << 2) | RNET_ASL_OPC_NTFY)
#define CMD_ID_GET_STATUS                       ((0x10 << 2) | RNET_ASL_OPC_RD)
#define CMD_ID_RESET                            ((0x3F << 2) | RNET_ASL_OPC_WR)


// Macros for different commands in RINGNET_MGMT command class
// are defined in network layer header

bool            is_protocol_class(U16 class_id, U16 command_id);
rnet_error_t      reset_set(uint8_t reset_type);
void            rnet_reset_notify(uint8_t value);
rnet_error_t      process_app_layer_stack_cmds();

rnet_error_t rnet_low_battery_notify(uint8_t level, uint32_t seq_no, uint32_t size, uint8_t *buf);


rnet_error_t      get_dev_id(U8* p_dev_id, U8 len);
rnet_error_t      get_pan_id(U8* p_pan_id);
rnet_error_t      set_dbg_dev_id(U8* dev_id);
rnet_error_t      get_dbg_dev_id(U8* p_dev_id);
rnet_error_t      rnet_get_group_id(U8* group_id, U8 len);
rnet_error_t      rnet_set_group_num(U8 group_num);
rnet_error_t      rnet_get_group_num(U8* p_grp_num);
rnet_error_t      set_pan_id(U8* p_pan_id);
rnet_error_t      get_p2p_channel(U8 *rfc);
rnet_error_t      set_p2p_channel(U8 rf_chnl);
rnet_error_t      set_pairing_state(U8 state);
rnet_error_t      set_ka_interval(U32 ka_interval);

/* @brief: Sets KA inteval without saving it flash
 *
 * @params[in]: ka_interval: KA interval to set
 * @return      Error code
 */
rnet_error_t      set_ka_interval_tmp(U32 ka_interval);

int16_t         get_currnet_pkt_rssi();

/**
 * Set getter for hardware ID
 *
 * @param[in] get_hw_id     Pointer to hw id getter
 * @param[in] ctx           Context for hw id getter
 * @return                  void
 */
void rnet_set_hw_id_getter(hw_id_func_t get_hw_id, void *ctx);

/**
 * Perform a reset on the device
 */
void rnet_reset();

#if !defined(GW_SUPPORT) || !GW_SUPPORT
int16_t         rssi_for_last_rcvd_pkt_from_bridge();
#endif

bool is_dfu_cmd(U32 cmd_class, U32 cmd_id);
bool is_start_dfu_cmd(U32 cmd_class, U32 cmd_id);

rnet_error_t set_p2p_channel(U8 rf_chnl);

bool rnet_is_metrics_cmd(U16 cmd_class, U16 cmd_id);

void rnet_factory_reset();
bool rnet_is_dfu_cmd(U16 cmd_class, U16 cmd_id);
rnet_error_t ringnet_send_proxy_light_state_change(U32 state, U8 *src_addr);
rnet_error_t rnet_create_sl_frame(rnet_pkt_t* pkt, rnet_cmd_t* p_cmd, tr_opts_t *tr_opts);
rnet_error_t set_p2p_security_key(U8* p_security_key, storage_kv_key_t storage_key);
void gateway_discovery(void);

#if defined(GW_SUPPORT) && GW_SUPPORT
rnet_error_t rnet_phy_dfu_settings(U8 *data, bool flag);
#endif
#endif
