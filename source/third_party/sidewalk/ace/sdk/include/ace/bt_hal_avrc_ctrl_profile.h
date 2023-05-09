/*
 * FreeRTOS BLE HAL V5.0.1
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://aws.amazon.com/freertos
 * http://www.FreeRTOS.org
 */


/**
 * @file bt_hal_avrc_ctrl_profile.h
 * @brief BT HAL provides the interfaces for Avrc Controller profile.
 * This interface provides APIs to send and receive passthrough keys for the profile.
 *
 * @addtogroup HAL_BLUETOOTH
 * @{
 */

#ifndef _BT_HAL_BT_AVRC_CTRL_PROFILE_H
#define _BT_HAL_BT_AVRC_CTRL_PROFILE_H

#include "bt_hal_manager.h"

/**
 * @brief AVRC Controller Key IDs that are supported for BT
 */
typedef enum
{
    BTHAL_AVRC_CTRL_KEY_ID_VOL_UP = 0x41,       /**< Up */
    BTHAL_AVRC_CTRL_KEY_ID_VOL_DOWN = 0x42,     /**< Down */
    BTHAL_AVRC_CTRL_KEY_ID_VOL_PLAY = 0x44,     /**< Play */
    BTHAL_AVRC_CTRL_KEY_ID_VOL_STOP = 0x45,     /**< Stop */
    BTHAL_AVRC_CTRL_KEY_ID_VOL_PAUSE = 0x46,    /**< Pause */
    BTHAL_AVRC_CTRL_KEY_ID_VOL_REWIND = 0x48,   /**< Rewind */
    BTHAL_AVRC_CTRL_KEY_ID_VOL_FF = 0x49,       /**< Fast Forward */
    BTHAL_AVRC_CTRL_KEY_ID_VOL_FORWARD = 0x4B,  /**< Forward */
    BTHAL_AVRC_CTRL_KEY_ID_VOL_BACKWARD = 0x4C, /**< Backward */
} BTAvrcCtrlKeyId_t;

/**
 * @brief AVRC Controller key states
 */
typedef enum
{
    BTHAL_AVRC_CTRL_KEY_STATE_PRESSED = 0, /**< Pressed */
    BTHAL_AVRC_CTRL_KEY_STATE_RELEASED = 1 /**< Released */
} BTAvrcCtrlKeyState_t;

/**
 * @brief Incoming AVRC Controller key event callback for passthrough command
 *
 * @param[in] p_remote_addr pointer to remote device address @ref BTBdaddr_t
 * @param[in] key_id The key sent by remote @ref aceBT_avrcCtrlKeyId_t
 * @param[in] key_state The key state sent @ref aceBT_avrcCtrlKeyState_t
 */
typedef void (* acebthal_incoming_rc_key_callback)( BTBdaddr_t * p_remote_addr,
                                                    BTAvrcCtrlKeyId_t key_id,
                                                    BTAvrcCtrlKeyState_t key_state );


typedef struct
{
    size_t size;
    acebthal_incoming_rc_key_callback rc_key_cb;
} BTAvrcCtrlCallbacks_t;

/** Represents the standard AVRC Controller interface. */
typedef struct
{
    /** Set this to size of BTAvrcCtrlInterface_t*/
    size_t size;

    /**
     * Initializes AVRCP Controller module.
     *
     * @param[in] callbacks callback for AVRC
     * @return Returns eBTStatusSuccess on successful call.
     */
    BTStatus_t ( * pxAvrcCtrlInit )( BTAvrcCtrlCallbacks_t * callbacks );

    /**
     * Cleans up AVRCP Controller module
     *
     * @param None
     * @return Returns eBTStatusSuccess on successful call.
     */
    BTStatus_t ( * pxAvrcCtrlCleanup )( void );

    /**
     * @brief API to send a passthrough key event for AVRC Controller profile
     *  of remote device.
     *
     * @param[in] p_remoteDevice pointer to remote device address @ref BTBdaddr_t
     * @param[in] key_id The key to be sent @ref aceBT_avrcCtrlKeyId_t
     * @param[in] key_state The key state to be sent @ref aceBT_avrcCtrlKeyState_t
     * @return Returns eBTStatusSuccess on successful call.
     */
    BTStatus_t ( * pxAvrcCtrlSendKeyEvent )( BTBdaddr_t * remote_addr,
                                             BTAvrcCtrlKeyId_t key_id,
                                             BTAvrcCtrlKeyState_t key_state );
} BTAvrcCtrlInterface_t;

const BTAvrcCtrlInterface_t * BT_GetAvrcCtrlInterface( void );

#endif /* _BT_HAL_BT_AVRC_CTRL_PROFILE_H */
/** @} */
