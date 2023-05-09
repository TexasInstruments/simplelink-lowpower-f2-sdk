/*
 * Copyright 2020 Amazon.com, Inc. or its affiliates. All rights reserved.
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

#ifndef AMA_STREAM_API_H
#define AMA_STREAM_API_H

#ifdef __cplusplus
extern "C" {
#endif

#include <ace/ama_defines.h>

/**
 * @defgroup ACE_AMA_DS_STREAM AMA Stream Data Structures
 * @{
 * @ingroup ACE_AMA_DS
 */

/** Stream Envelope */
typedef struct {
    /** Sender/Receiver Gateway */
    aceAma_gateway_t* gateway;
    /** Stream type */
    aceAma_stream_t stream;
    /** Encoded data length - Limited by aceAma_config_t.trxn_limit */
    uint16_t length;
    /** Encoded data */
    uint8_t* data;
} aceAma_streamEnvelope_t;

/** Enum For Stream Event Handling */
typedef enum {
    /** Reserved */
    ACE_AMA_STREAM_EVT_RESERVED = 0,

    /**
     * Event for Envelope Received
     * ctx is aceAma_streamEnvelope_t
     */
    ACE_AMA_STREAM_EVT_ENVELOPE_RECEIVED = 1,

    /**
     * Event for Envelope Sent
     * ctx is NULL
     */
    ACE_AMA_STREAM_EVT_ENVELOPE_SENT = 2,
} aceAma_streamEventId_t;

/** Struct To Hold Stream Event Info */
typedef struct {
    /** Event ID */
    aceAma_streamEventId_t id;
    /** Context Pointer */
    const void* ctx;
} aceAma_streamEvent_t;

/** Struct To Hold Stream Config */
typedef struct {
    /** Stream Envelope Version */
    uint32_t envelope_version;
} aceAma_streamConfig_t;

/** @} */

/**
 * @defgroup ACE_AMA_API_STREAM AMA Stream APIs
 * @{
 * @ingroup ACE_AMA_API
 */

/**
 * @brief Handler function to process all Stream events.
 *
 * param[in] event Event received
 */
typedef void (*aceAma_streamHandler_t)(const aceAma_streamEvent_t* event);

/**
 * @brief      Initialize Stream Module
 *
 * @param[in]   handler  Handler function to process Stream events
 * @param[in]   config   Stream configuration
 *
 * @return ACE_STATUS_OK on success, or an error otherwise
 * @retval ACE_AMA_STATUS_UNSUPPORTED If module is not configured
 */
ace_status_t aceAma_streamInit(const aceAma_streamHandler_t handler,
                               const aceAma_streamConfig_t* config);

/**
 * @brief   Async Request - Send Envelope of Selected Stream ID
 *
 * @param[in]   envelope    aceAma_streamEnvelope_t.gateway - UUID only <br>
 *                          aceAma_streamEnvelope_t.stream - Stream ID <br>
 *                          aceAma_streamEnvelope_t.length - Data length <br>
 *                          aceAma_streamEnvelope_t.data - Data bytes
 *
 * @return ACE_STATUS_OK on success, or an error otherwise
 * @retval ACE_STATUS_BAD_PARAM If function parameters are invalid
 * @retval ACE_AMA_STATUS_UNSUPPORTED If module is not configured
 * @retval ACE_STATUS_OUT_OF_MEMORY If stack is out of memory
 */
ace_status_t aceAma_streamSendEnvelope(aceAma_streamEnvelope_t* envelope);

/** @} */

#ifdef __cplusplus
}
#endif

#endif  // AMA_STREAM_API_H
