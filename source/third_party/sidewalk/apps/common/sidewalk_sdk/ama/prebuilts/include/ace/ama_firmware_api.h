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

#ifndef AMA_FIRMWARE_API_H
#define AMA_FIRMWARE_API_H

#ifdef __cplusplus
extern "C" {
#endif

#include <ace/ama_defines.h>

#define ACE_AMA_FIRMWARE_NAME_MAX_LENGTH 16
#define ACE_AMA_FIRMWARE_LOCALE_MAX_LENGTH 8
#define ACE_AMA_FIRMWARE_SIGNATURE_MAX_LENGTH 65

/**
 * @defgroup ACE_AMA_DS_FIRMWARE ACE AMA Firmware Data Structures
 * @{
 * @ingroup ACE_AMA_DS
 */

/** Struct To Hold Common Config */
typedef struct {
    uint16_t component_count;
} aceAma_firmwareConfig_t;

/** Struct To Hold Firmware Update Progress */
typedef struct {
    /** Offset Of Current Data */
    uint32_t offset;
    /** Length Of Current Data */
    uint16_t length;
    /** Pointer To Data */
    uint8_t* data;
    /** Does Server Request An Ack */
    bool ack_required;
} aceAma_firmwareProgress_t;

/** Struct To Hold Firmware Update Info */
typedef struct {
    /** Name of Component */
    char component_name[ACE_AMA_FIRMWARE_NAME_MAX_LENGTH];
    /** Size of Component */
    uint32_t component_size;
    /** Segment Signature */
    char segment_signature[ACE_AMA_FIRMWARE_SIGNATURE_MAX_LENGTH];
} aceAma_firmwareUpdate_t;

/** Struct To Hold Firmware Component Info */
typedef struct {
    /** Component Version */
    uint32_t version;
    /** Component Name */
    char name[ACE_AMA_FIRMWARE_NAME_MAX_LENGTH];
} aceAma_firmwareComponent_t;

/** Struct To Hold Firmware Infomation */
typedef struct {
    /** Firmware Version */
    uint32_t version;
    /** Firmware Name */
    char name[ACE_AMA_FIRMWARE_NAME_MAX_LENGTH];
    /** Firmware Locale */
    char locale[ACE_AMA_FIRMWARE_LOCALE_MAX_LENGTH];
    /** Firmware Version Name */
    char version_name[ACE_AMA_FIRMWARE_NAME_MAX_LENGTH];
    /** Number Of Components */
    uint16_t component_count;
    /** Components */
    aceAma_firmwareComponent_t components[1];
} aceAma_firmwareInformation_t;

/** Struct To Hold Firmware State Changed Info */
typedef struct {
    /** Firmware Info */
    aceAma_firmwareUpdate_t* update;
    /** Firmware Update State */
    bool state;
} aceAma_firmwareStateEvent_t;

/** Struct To Hold Firmware Apply Firmware Info */
typedef struct {
    /** Firmware Info */
    aceAma_firmwareInformation_t* info;
    /** Restart After Update */
    bool restart;
} aceAma_firmwareApplyEvent_t;

/** Enum For Firmware Event Handling */
typedef enum {
    /** Reserved */
    ACE_AMA_FIRMWARE_EVT_RESERVED = 0,

    /**
     * Event for Firmware State Changed
     * ctx is aceAma_firmwareStateEvent_t
     */
    ACE_AMA_FIRMWARE_EVT_STATE_CHANGED = 1,

    /**
     * Event for Firmware Progress Updated
     * ctx is aceAma_firmwareProgress_t
     */
    ACE_AMA_FIRMWARE_EVT_PROGRESS_UPDATED = 2,

    /**
     * Event for Apply Firmware
     * ctx is aceAma_firmwareApplyEvent_t
     */
    ACE_AMA_FIRMWARE_EVT_APPLY_FIRMWARE = 3,
} aceAma_firmwareEventId_t;

/** Struct To Hold Firmware Event Info */
typedef struct {
    /** Event ID */
    aceAma_firmwareEventId_t id;
    /** Context Pointer */
    const void* ctx;
} aceAma_firmwareEvent_t;

/**
 * @defgroup ACE_AMA_API_FIRMWARE AMA Firmware APIs
 * @{
 * @ingroup ACE_AMA_API
 */

/**
 * @brief Handler function to process all firmware events.
 *
 * @param[in] event Event received
 */
typedef void (*aceAma_firmwareHandler_t)(const aceAma_firmwareEvent_t* event);

/**
 * @brief   Initialize Firmware Client Module
 *
 * @param[in]   handler  Firmware event handler
 * @param[in]   config  Config values for Firmware
 *
 * @return ACE_AMA_STATUS_SUCCESS on successful init
 * @return ACE_STATUS_BAD_PARAM on NULL params
 */
ace_status_t aceAma_firmwareInit(const aceAma_firmwareHandler_t handler,
                                 const aceAma_firmwareConfig_t* config);

/**
 * @brief   Set Local Device Firmware Information
 *
 * @param[in]  info  Local device firmware information
 *
 * @return ACE_AMA_STATUS_SUCCESS on successful
 * @return ACE_STATUS_BAD_PARAM on NULL params
 */
ace_status_t aceAma_firmwareSetInformation(aceAma_firmwareInformation_t* info);

/**
 * @brief   Get Local Device Firmware Information
 *
 * @param[in]  info  Local device firmware information
 *
 * @return ACE_AMA_STATUS_SUCCESS on successful
 * @return ACE_STATUS_BAD_PARAM on NULL params
 */
ace_status_t aceAma_firmwareGetInformation(aceAma_firmwareInformation_t* info);

/**
 * @brief   Acknowledge Component Update Request
 *
 * This API is used to ACK/NACK \ref ACE_AMA_FIRMWARE_EVT_PROGRESS_UPDATED if it
 * was requested from the remote device.
 *
 * @param[in]  ack  ACK/NACK received component sent
 *
 * @return ACE_STATUS_OK on successful send
 * @return ACE_STATUS_BAD_PARAM on bad firmware state
 */
ace_status_t aceAma_firmwareAckProgress(bool ack);

/**
 * @brief   Confirm Firmware Update Sequence
 *
 * This API is used to confirm the success of the firmware update sequence after
 * the transfer is complete. This API can also be called before the transfer is
 * complete, to cancel the firmware update sequence.
 *
 * @param[in]   confirm  true - System will update its firmware
 *                       false - System will not update its firmware (i.e busy)
 *
 * @return ACE_STATUS_OK on successful send
 * @return ACE_STATUS_BAD_PARAM on bad firmware state
 */
ace_status_t aceAma_firmwareConfirmUpdate(bool confirm);

/**
 * @brief   Confirm Firmware Apply Request
 *
 * This API sends a confirmation reply back to the AMA Gateway to communicate
 * whether or not the firmware was successfully installed.
 *
 * @param[in]   confirm  true - System will update its firmware
 *                       false - System will not update its firmware (i.e busy)
 *
 * @return ACE_STATUS_OK on successful send
 * @return ACE_STATUS_BAD_PARAM on bad firmware state
 */
ace_status_t aceAma_firmwareConfirmApply(bool confirm);

/** @} */

#ifdef __cplusplus
}
#endif

#endif  // AMA_FIRMWARE_API_H
