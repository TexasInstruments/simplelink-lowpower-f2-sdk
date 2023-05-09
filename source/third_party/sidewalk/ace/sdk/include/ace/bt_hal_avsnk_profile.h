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
 * @file bt_hal_avsnk_profile.h
 * @brief BT HAL provides the interfaces for A2DP Sink profile for local device.
 * @addtogroup HAL_BLUETOOTH
 * USAGE
 * -----
 *
 * @{
 */

#ifndef _BT_HAL_BT_AVSNK_PROFILE_H
#define _BT_HAL_BT_AVSNK_PROFILE_H

#include <stddef.h>

/**
 * @brief Bluetooth AV connection states
 */
typedef enum
{
    eBTAvsnkConnectionStateDisconnected = 0, /**< Disconnected */
    eBTAvsnkConnectionStateConnecting = 1,   /**< Connecting */
    eBTAvsnkConnectionStateConnected = 2,    /**< Connected */
    eBTAvsnkConnectionStateDisconnecting = 3 /**< Disconnecting */
} BTAvSnkConnectionState_t;

/**
 * @brief Bluetooth AV datapath states
 */
typedef enum
{
    eBTAvsnkAudioStateRemoteSuspend = 0, /**< Audio Suspended */
    eBTAvsnkAudioStateStopped = 1,       /**< Audio Stopped */
    eBTAvsnkAudioStateStarted = 2,       /**< Audio Started */
} BTAvSnkAudioState_t;

/**
 * @brief A2DP Sink connection state callback.
 *
 * @param[in] xState connection state
 * @param[in] pxBdAddr Address of the remote device.
 */
typedef void (* BTAvsnkConnectionStateCallback_t)( BTAvSnkConnectionState_t xState,
                                                   BTBdaddr_t * pxBdAddr );

/**
 * @brief A2DP Sink audio state callback.
 *
 * @param[in] xState audio state
 * @param[in] pxBdAddr Address of the remote device
 */
typedef void (* BTAvsnkAudioStateCallback_t)( BTAvSnkAudioState_t xState,
                                              BTBdaddr_t * pxBdAddr );

typedef struct
{
    size_t xSize;
    BTAvsnkConnectionStateCallback_t xConnStateCback;
    BTAvsnkAudioStateCallback_t xAudioStateCback;
} BTAvsnkCallbacks_t;

/** Represents the standard AV Sink connection interface. */
typedef struct
{
    /**
     * @brief Set this to size of BTAvsnkInterface_t
     */
    size_t xSize;

    /**
     * @brief Initializes A2DP Sink module.
     *
     * @param[in] callbacks @ref BTAvsnkCallbacks_t callback
     * @return Returns eBTStatusSuccess on successful call.
     */
    BTStatus_t ( * pxAvsnkInit )( BTAvsnkCallbacks_t * pxCallbacks );

    /**
     * @brief Cleans to A2DP Sink module
     *
     * @param None
     * @return Returns eBTStatusSuccess on successful call.
     */
    BTStatus_t ( * pxAvsnkCleanup )( void );

    /**
     * @brief Starts an A2DP Sink profile connection.
     *
     * @param[in] pxRemote_addr BT Address of remote device
     * @return Returns eBTStatusSuccess on successful call.
     */
    BTStatus_t ( * pxAvsnkConnect )( BTBdaddr_t * pxRemote_addr );

    /**
     * @brief Starts an A2DP Sink profile disconnect
     *
     * @param[in] pxRemote_addr BT Address of remote device
     * @return Returns eBTStatusSuccess on successful call.
     */
    BTStatus_t ( * pxAvsnkDisconnect )( BTBdaddr_t * pxRemote_addr );
} BTAvsnkInterface_t;

const BTAvsnkInterface_t * BT_GetAvsnkInterface( void );

#endif /* _BT_HAL_BT_AVSNK_PROFILE_H */
/** @} */
