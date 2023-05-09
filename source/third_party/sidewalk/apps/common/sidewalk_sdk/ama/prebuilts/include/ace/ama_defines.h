/*
 * Copyright 2020 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and conditions
 * set forth in the
 * accompanying LICENSE.TXT file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO
 * THESE MATERIALS, ALL WARRANTIES, EXPRESS, IMPLIED, OR STATUTORY, INCLUDING
 * THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef AMA_DEFINES_H
#define AMA_DEFINES_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include <ace/ace_status.h>

/**
 * @defgroup ACE_AMA_DS_COMMON ACE AMA Common Data Structures
 * @{
 * @ingroup ACE_AMA_DS
 */

/** Gateway UUID Length */
#define ACE_AMA_GATEWAY_UUID_LENGTH 6

/** @brief Stream */
typedef enum {
    /** Restricted - Internal */
    ACE_AMA_STREAM_CONTROL = 0x00,
    /** Voice */
    ACE_AMA_STREAM_VOICE = 0x01,
    /** Firmware */
    ACE_AMA_STREAM_FIRMWARE = 0x02,
    /** Diagnostics */
    ACE_AMA_STREAM_DIAGNOSTICS = 0x03,
    /** Fitness */
    ACE_AMA_STREAM_FITNESS = 0x04,
    /** Bulk Transfer */
    ACE_AMA_STREAM_BULK_TRANSFER = 0x05,
    /** Alexa v3 Directives, Proto3 Encoded */
    ACE_AMA_STREAM_ALEXA = 0x06,
    /** Sidewalk */
    ACE_AMA_STREAM_SIDEWALK = 0x07,
    /** Stream Enum Max Value */
    ACE_AMA_STREAM_MAX
} aceAma_stream_t;

/** Protocol Gateway State */
typedef enum {
    /** Logically Disconnected */
    ACE_AMA_GATEWAY_DISCONNECTED = 0,
    /** Logically Connected */
    ACE_AMA_GATEWAY_CONNECTED = 1,
    /** AMA Exchange Complete - Gateway Ready For Other APIs */
    ACE_AMA_GATEWAY_ACTIVE = 2,
    /** Gateway State Max Enum */
    ACE_AMA_GATEWAY_MAX = 0xFFFF
} aceAma_gatewayState_t;

/** Protocol Gateway */
typedef struct {
    /** Gateway UUID */
    uint8_t uuid[ACE_AMA_GATEWAY_UUID_LENGTH];
    /** Gateway State */
    aceAma_gatewayState_t state;
    /** Gateway MTU */
    uint16_t mtu;
} aceAma_gateway_t;

/** Packet Type For Transports */
typedef struct {
    /** Gateway */
    aceAma_gateway_t* gateway;
    /** Packet Length */
    uint16_t length;
    /** Packet Payload */
    uint8_t* data;
} aceAma_packet_t;

/** Device Name Max Length */
#define ACE_AMA_DEVICE_NAME_LENGTH 16
/** Device Type Max Length */
#define ACE_AMA_DEVICE_TYPE_LENGTH 22
/** Device Serial Number Max Length */
#define ACE_AMA_DEVICE_SERIAL_NUMBER_LENGTH 20

/** Local Device Information */
typedef struct {
    /** Device Name */
    char name[ACE_AMA_DEVICE_NAME_LENGTH];
    /** Amazon Assigned Device Type */
    char device_type[ACE_AMA_DEVICE_TYPE_LENGTH];
    /** Device Serial Number */
    char serial_number[ACE_AMA_DEVICE_SERIAL_NUMBER_LENGTH];
} aceAma_deviceInformation_t;

/** @} */

#ifdef __cplusplus
}
#endif

#endif  // AMA_DEFINES_H
