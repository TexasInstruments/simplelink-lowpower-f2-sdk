/*
 * Copyright 2022 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.TXT file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef SID_DEMO_TYPES_H
#define SID_DEMO_TYPES_H

#include <sid_parser_utils.h>

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * sid demo msg type
 */
enum sid_demo_app_cmd_type {
    SID_DEMO_MSG_TYPE_READ = 0x0,
    SID_DEMO_MSG_TYPE_WRITE = 0x1,
    SID_DEMO_MSG_TYPE_NOTIFY = 0x2,
    SID_DEMO_MSG_TYPE_RESP = 0x3,
    SID_DEMO_MSG_TYPE_LAST,
};

enum sid_demo_class_id {
    SID_DEMO_APP_CLASS = 0x0,
    SID_DEMO_APP_CLASS_LAST,
};

enum sid_demo_class_cmd_id {
    SID_DEMO_APP_CLASS_CMD_CAP_DISCOVERY_ID = 0x0,
    SID_DEMO_APP_CLASS_CMD_ACTION = 0x1,
    SID_DEMO_APP_CLASS_CMD_LAST,
};

enum sid_demo_msg_desc_attributes {
    SID_DEMO_APP_MSG_DESC_STATUS_HDR_IND_OFFSET = 7,
    SID_DEMO_APP_MSG_DESC_OP_CODE_OFFSET = 5,
    SID_DEMO_APP_MSG_DESC_CLASS_OFFSET = 3,
    SID_DEMO_APP_MSG_DESC_CMD_ID_OFFSET = 0,
    SID_DEMO_APP_MSG_DESC_STATUS_HDR_IND_MASK = 0x1,
    SID_DEMP_APP_MSG_DESC_OP_CODE_MASK = 0x3,
    SID_DEMO_APP_MSG_DESC_CLASS_MASK = 0x3,
    SID_DEMP_APP_MSG_DESC_CMD_ID_MASK = 0x7,
};
/**
 * Sid demo tag IDs and their sizes
 */
enum sid_demo_tag_id {
    SID_DEMO_TAG_NUMBER_OF_BUTTONS = 0x1,
    SID_DEMO_TAG_NUMBER_OF_LEDS = 0x2,
    SID_DEMO_TAG_LED_ON_ACTION_REQ  = 0x3,
    SID_DEMO_TAG_LED_OFF_ACTION_REQ = 0x4,
    SID_DEMO_TAG_BUTTON_PRESS_ACTION_NOTIFY = 0x5,
    SID_DEMO_TAG_TEMPERATURE_SENSOR_DATA_NOTIFY = 0x6,
    SID_DEMO_TAG_CURRENT_GPS_TIME_IN_SECONDS = 0x7,
    SID_DEMO_TAG_DL_LATENCY_IN_SECONDS = 0x8,
    SID_DEMO_TAG_LED_ON_RESP = 0x9,
    SID_DEMO_TAG_LED_OFF_RESP = 0xA,
    SID_DEMO_TAG_TEMP_SENSOR_AVAILABLE_AND_UNIT_REPRESENTATION = 0xB,
    SID_DEMO_TAG_LINK_TYPE = 0xC,
    SID_DEMO_TAG_BUTTON_PRESSED_RESP = 0xD,
    SID_DEMO_TAG_LAST,
    SID_DEMO_TAG_ID_MAX = 0x40
};

/**
 * Sid demo message descriptor
 */
struct sid_demo_msg_desc {
    /** Status header included field */
    bool status_hdr_ind;
    /** Type of message */
    enum sid_demo_app_cmd_type opc;
    /** class that the command belongs to */
    uint8_t cmd_class;
    /** Command Id within the class */
    uint8_t cmd_id;
    /** status code, optional, included only when status_hdr_ind is set to true */
    uint8_t status_code;
};

/**
 * Sid demo message
 */
struct sid_demo_msg {
    /** size of the payload */
    size_t payload_size;
    /** pointer to the payload */
    uint8_t *payload;
};

enum sid_demo_temperature_sensor {
    SID_DEMO_TEMPERATURE_SENSOR_NOT_SUPPORTED = 0,
    SID_DEMO_TEMPERATURE_SENSOR_UNITS_CELSIUS = 1,
    SID_DEMO_TEMPERATURE_SENSOR_UNITS_FARENHEIT = 2,
    SID_DEMO_TEMPERATURE_SENSOR_LAST,
};

struct sid_demo_capability_discovery {
    uint8_t num_buttons;
    uint8_t num_leds;
    enum sid_demo_temperature_sensor temp_sensor;
    uint8_t *button_id_arr;
    uint8_t *led_id_arr;
    uint8_t link_type;
};

enum sid_demo_led_action {
    SID_DEMO_ACTION_LED_NO_ACTION = 0,
    SID_DEMO_ACTION_LED_OFF = 1,
    SID_DEMO_ACTION_LED_ON = 2,
    SID_DEMO_ACTION_LED_LAST,
};

enum sid_demo_button_action {
    SID_DEMO_ACTION_BUTTON_NOT_PRESSED = 0,
    SID_DEMO_ACTION_BUTTON_PRESSED = 1,
    SID_DEMO_ACTION_BUTTON_LAST,
};

enum sid_demo_action_resp_type {
    SID_DEMO_ACTION_TYPE_NO_ACTION = 0,
    SID_DEMO_ACTION_TYPE_LED = 1,
    SID_DEMO_ACTION_TYPE_BUTTON = 2,
    SID_DEMO_ACTION_TYPE_LAST,
};

struct sid_demo_led_action_req {
    enum sid_demo_led_action action_req;
    uint8_t num_leds;
    uint8_t *led_id_arr;
    uint32_t gps_time_in_seconds;
};

struct sid_demo_led_action_resp {
    enum sid_demo_led_action action_resp;
    uint8_t num_leds;
    uint8_t *led_id_arr;
};

struct sid_demo_button_action_resp {
    enum sid_demo_button_action action_resp;
    uint8_t num_buttons;
    uint8_t *button_id_arr;
};

struct sid_demo_action_resp {
    enum sid_demo_action_resp_type resp_type;
    uint8_t link_type;
    uint32_t down_link_latency_secs;
    uint32_t gps_time_in_seconds;
    struct sid_demo_led_action_resp led_action_resp;
    struct sid_demo_button_action_resp button_action_resp;
};

struct sid_demo_action_notification {
    uint8_t link_type;
    enum sid_demo_temperature_sensor temp_sensor;
    int16_t temperature;
    uint32_t gps_time_in_seconds;
    struct sid_demo_button_action_resp button_action_notify;
};

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* SID_DEMO_TYPES_H */
