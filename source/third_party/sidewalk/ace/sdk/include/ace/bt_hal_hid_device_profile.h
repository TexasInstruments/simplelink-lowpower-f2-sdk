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
 * @file bt_hid_device_profile.h
 * @brief BT HAL provides the interfaces for HID device profile for local device.
 * @addtogroup HAL_BLUETOOTH
 * USAGE
 * -----
 *
 * @{
 */

#ifndef BT_HID_DEVICE_PROFILE_H
#define BT_HID_DEVICE_PROFILE_H

#include "bt_hal_manager.h"

/**
 * @brief Bluetooth HID Device Report Type
 */
typedef enum
{
    eBTHIDDeviceReportTypeOther = 0,  /**< Other report type */
    eBTHIDDeviceReportTypeInput = 1,  /**< Input report type */
    eBTHIDDeviceReportTypeOutput = 2, /**< Output report type */
    eBTHIDDeviceReportTypeFeature = 3 /**< Feature report type */
} BTHIDDeviceReportType_t;

/**
 * @brief Bluetooth HID Device Connection State
 */
typedef enum
{
    eBTHIDDeviceStateDisconnected = 0,  /**< Disconnected */
    eBTHIDDeviceStateConnecting = 1,    /**< Connecting */
    eBTHIDDeviceStateConnected = 2,     /**< Connected */
    eBTHIDDeviceStateDisconnecting = 3, /**< Disconnecting */
    eBTHIDDeviceStateUnknown = 4        /**< Unknown */
} BTHIDDeviceConnectionState_t;

/**
 * @brief Bluetooth HID Device App Parameter
 */
typedef struct
{
    const char * pcName;
    const char * pcDescription;
    const char * pcProvider;
    uint8_t ucSubclass;
    uint8_t * pucDescriptorList;
    int descriptorListLen;
} BTHIDDeviceAppParameter_t;

/**
 * @brief Bluetooth HID Device QoS Parameter
 */
typedef struct
{
    uint8_t ucServiceType;
    uint32_t ulTokenRate;
    uint32_t ulTokenBucketSize;
    uint32_t ulPeakBandwidth;
    uint32_t ulAccessLatency;
    uint32_t ulDelayVariation;
} BTHIDDeviceQoSParameter_t;

/**
 * @brief Bluetooth HID Device Connection State callback
 *
 * @param[in] pxBdAddr Address of the Remote device
 * @param[in] xTransport transport for connection
 * @param[in] pxState pointer to device connection state
 */
typedef void (* BTHIDDeviceConnectionStateCallback_t)( BTBdaddr_t * pxBdAddr,
                                                       BTTransport_t xTransport,
                                                       BTHIDDeviceConnectionState_t xState );

/**
 * @brief Bluetooth HID Device Application Get Report callback
 *
 * @param[in] ucType Report type
 * @param[in] ucId Report Id
 * @param[in] usBbufferSize Buffer size
 */
typedef void (* BTHIDDeviceGetReportCallback_t)( BTHIDDeviceReportType_t xType,
                                                 uint8_t ucId,
                                                 uint16_t usBufferSize );

/**
 * @brief Bluetooth HID Device Application Set Report callback
 *
 * @param[in] ucType Report type
 * @param[in] ucId Report ID
 * @param[in] usLen Data length
 * @param[in] pxData Data buffer
 */
typedef void (* BTHIDDeviceSetReportCallback_t)( BTHIDDeviceReportType_t xType,
                                                 uint8_t ucId,
                                                 uint16_t usLen,
                                                 uint8_t * pxData );

/**
 * @brief Bluetooth HID Device Set Protocol callback
 *
 * @param[in] ucProtocol Protocol mode
 */
typedef void (* BTHIDDeviceSetProtocolCallback_t)( uint8_t ucProtocol );

/**
 * @brief Bluetooth HID Device Virtual Cable Unplug callback
 */
typedef void (* BTHIDDeviceVirtualUnplugCallback_t)( void );

/** BT HID Device callbacks */
typedef struct
{
    size_t size;
    BTHIDDeviceConnectionStateCallback_t pxConnectionStateCb;
    BTHIDDeviceGetReportCallback_t pxGetReportCb;
    BTHIDDeviceSetReportCallback_t pxSetReportCb;
    BTHIDDeviceSetProtocolCallback_t pxSetProtocolCb;
    BTHIDDeviceVirtualUnplugCallback_t pxVirtualUnplugCb;
} BTHIDDeviceCallbacks_t;

/** Standard BT HD Device interface */
typedef struct
{
    size_t size;

    /**
     * @brief Initializes HID device interface and register callbacks.
     *
     * @param[in] pxCallbacks BTHIDDeviceCallbacks_t callbacks.
     * @return Returns eBTStatusSuccess on successful call.
     */
    BTStatus_t ( * pxHIDDeviceInit )( BTHIDDeviceCallbacks_t * pxCallbacks );

    /**
     * @brief Cleans up interface
     *
     * @param None
     * @return Returns eBTStatusSuccess on successful call.
     */
    void ( * pxHIDDeviceCleanup )( void );

    /**
     * @brief Config Devic QoS.
     *
     * @param[in] pxApplicationParam QoS Parameters
     * @param[in] pxInQos
     * @param[in] pxOutQoS
     * @return Returns eBTStatusSuccess on successful call.
     */
    BTStatus_t ( * pxHIDDeviceConfigQoS )( BTHIDDeviceAppParameter_t * pxApplicationParam,
                                           BTHIDDeviceQoSParameter_t * pxInQos,
                                           BTHIDDeviceQoSParameter_t * pxOutQoS );

    /**
     * @brief Connects to host with virtual cable.
     *
     * @param[in] pxBdAddr Address of the Remote device
     * @param[in] xTransport transport for connection
     * @return Returns eBTStatusSuccess on successful call.
     */
    BTStatus_t ( * pxHIDDeviceConnect )( BTBdaddr_t * pxBdAddr,
                                         BTTransport_t xTransport );

    /**
     * @brief Disconnects from currently connected host.
     *
     * @param[in] pxBdAddr Address of the Remote device
     * @return Returns eBTStatusSuccess on successful call.
     */
    BTStatus_t ( * pxHIDDeviceDisconnect )( BTBdaddr_t * pxBdAddr );

    /**
     * @brief Send report.
     *
     * @param[in] xType Report type
     * @param[in] ucId Report ID
     * @param[in] usLen Length of data buffer
     * @param[in] pxData Data buffer
     * @return Returns eBTStatusSuccess on successful call.
     */
    BTStatus_t ( * pxHIDDeviceSendReport )( BTHIDDeviceReportType_t xType,
                                            uint8_t ucId,
                                            uint16_t usLen,
                                            uint8_t * pxData );

    /**
     * @brief Notifies error for invalid SET_REPORT.
     *
     * @param[in] ucError Error code
     * @return Returns eBTStatusSuccess on successful call.
     */
    BTStatus_t ( * pxHIDDeviceReportError )( uint8_t ucError );

    /**
     * @brief Send Virtual Cable Unplug.
     *
     * @param[in] pxBdAddr Address of the Remote device
     * @return Returns eBTStatusSuccess on successful call.
     */
    BTStatus_t ( * pxHIDDeviceVirtualUnplug )( BTBdaddr_t * pxBdAddr );
} BTHIDDeviceInterface_t;

const BTHIDDeviceInterface_t * BT_GetHIDDeviceInterface( void );


#endif /* BT_HID_DEVICE_PROFILE_H_ */
/** @} */
