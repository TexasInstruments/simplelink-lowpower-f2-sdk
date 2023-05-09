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

#ifndef AMA_API_H
#define AMA_API_H

#ifdef __cplusplus
extern "C" {
#endif

#include <ace/ama_defines.h>

/**
 * @defgroup ACE_AMA_DS_COMMON ACE AMA Common Data Structures
 * @{
 * @ingroup ACE_AMA_DS
 */

/** Enum For Common Event Handling */
typedef enum {
    /** Reserved */
    ACE_AMA_EVT_RESERVED = 0,

    /**
     * Event for State Changed
     * ctx is aceAma_gateway_t
     */
    ACE_AMA_EVT_STATE_CHANGED = 1,

    /**
     * Event for Packet Available
     * ctx is NULL
     */
    ACE_AMA_EVT_PACKET_AVAILABLE = 2,
} aceAma_eventId_t;

/** Struct To Hold Common Event Info */
typedef struct {
    /** Event ID */
    aceAma_eventId_t id;
    /** Context Pointer */
    const void* ctx;
} aceAma_event_t;

/** Struct To Hold Common Config */
typedef struct {
    /** Largest AMA transaction stack can send and receive */
    uint16_t trxn_limit;
    /** Number of concurrent gateways that can be added */
    uint8_t num_gateways;
} aceAma_config_t;

/** @} */

/**
 * @defgroup ACE_AMA_API_COMMON ACE AMA Common APIs
 * @{
 * @ingroup ACE_AMA_API
 */

/**
 * @brief Handler function to process all events.
 *
 * @param[in] event Event received
 */
typedef void (*aceAma_handler_t)(const aceAma_event_t* event);

/**
 * @brief   Initialize Common Stack Module
 *
 * Must be the very first API called regardless of configuration.
 *
 * @param[in]   handler     Handler function to process common events
 * @param[in]   config      Stack configuration
 *
 * @return ACE_STATUS_OK on success, or an error otherwise
 */
ace_status_t aceAma_init(const aceAma_handler_t handler,
                         const aceAma_config_t* config);

/**
 * @brief   Set Local Device Information
 *
 * Set local device information to be shared with remote gateways
 *
 * @param[in]   info    Local device information to be exchanged
 *
 * @return ACE_STATUS_OK on success, or an error otherwise
 */
ace_status_t aceAma_setDeviceInfo(const aceAma_deviceInformation_t* info);

/**
 * @brief   Get Local Device Information
 *
 * Get local device information to be shared with remote gateways
 *
 * @param[in]   info    Local device information to be retrived
 *
 * @return ACE_STATUS_OK on success, or an error otherwise
 */
ace_status_t aceAma_getDeviceInfo(aceAma_deviceInformation_t* info);

/**
 * @brief   Add Gateway
 *
 * @param[in]   gateway    aceAma_gateway_t.uuid - Gateway UUID to add
 *
 * @return ACE_STATUS_OK on success, or an error otherwise
 */
ace_status_t aceAma_addGateway(const aceAma_gateway_t* gateway);

/**
 * @brief   Remove Gateway
 *
 * @param[in]   gateway    aceAma_gateway_t.uuid - Gateway UUID to remove
 *
 * @return ACE_STATUS_OK on success, or an error otherwise
 */
ace_status_t aceAma_removeGateway(const aceAma_gateway_t* gateway);

/**
 * @brief   Set Gateway Logical State
 *
 * Set logical state of an gateway
 *
 * @param[in]   gateway     aceAma_gateway_t.uuid - Gateway UUID <br>
 *                          aceAma_gateway_t.state - Updated state
 *
 * @return ACE_STATUS_OK on success, or an error otherwise
 */
ace_status_t aceAma_setGatewayState(const aceAma_gateway_t* gateway);

/**
 * @brief   Set Gateway MTU
 *
 * Default value for all gateways is 20 bytes. MTU will reset to default value
 * after disconnection. MTU cannot be decreased.
 *
 * @param[in]   gateway     aceAma_gateway_t.uuid - Gateway UUID <br>
 *                          aceAma_gateway_t.state - Gateway state <br>
 *                          aceAma_gateway_t.mtu - Updated mtu
 *
 * @return ACE_STATUS_OK on success, or an error otherwise
 */
ace_status_t aceAma_setGatewayMTU(const aceAma_gateway_t* gateway);

/**
 * @brief   Get Gateway Information
 *
 * Retrieve gateway information including UUID, state and MTU.
 *
 * @param[in]   gateway     aceAma_gateway_t.uuid - Gateway UUID <br>
 *                          aceAma_gateway_t.state - Gateway state <br>
 *                          aceAma_gateway_t.mtu - Gateway mtu
 *
 * @return ACE_STATUS_OK on success, or an error otherwise
 */
ace_status_t aceAma_getGateway(aceAma_gateway_t* gateway);

/**
 * @brief   Submit incoming data to AMA.
 *
 * Submit received raw packet data to AMA for decoding.  Intended to be
 * called by the application/platform when new data arrives at the data link
 * (network socket, USB endpoint, BLE radio, etc.).  The supplied buffer will
 * be copied; the caller may reuse/free the buffer upon return.
 * This API is interrupt safe
 *
 * @param[in]  gateway      aceAma_gateway_t.uuid - Gateway UUID <br>
 * @param[in]  length       Length of data in buffer
 * @param[in]  data         Incoming data buffer
 *
 * @return ACE_STATUS_OK on success, or an error otherwise
 */
ace_status_t aceAma_packetReceived(const aceAma_gateway_t* gateway,
                                   uint16_t length, uint8_t* data);

/**
 * @brief   Get Next Packet To Send
 *
 * Get next AMA packet to send. App should pass in a pointer to an
 * aceAma_packet_t struct owned by the app. If packet->gateway is not NULL, app
 * should send data to specified gateway. If packet->gateway is NULL, app should
 * return and wait for the next packet available event.
 *
 * The data given back through this call is invalidated upon the next call of
 * this function. This function doubles as an implicit notification that the
 * previous packet has been sent.
 *
 * @param[out]   packet      packet to send
 *
 * @return ACE_STATUS_OK on packet retrieved or no packets left to send
 * @retval ACE_STATUS_BAD_PARAM if packet is NULL
 */
ace_status_t aceAma_getNextPacket(aceAma_packet_t* packet);
/** @} */

#ifdef __cplusplus
}
#endif

#endif  // AMA_API_H
