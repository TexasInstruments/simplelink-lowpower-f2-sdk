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
 * @file bt_hid_host_profile.h
 * @brief BT HAL provides the interfaces for HID Host profile for local device.
 * @addtogroup HAL_BLUETOOTH
 * USAGE
 * -----
 *
 * @{
 */

#ifndef _BT_HAL_BT_HID_HOST_PROFILE_H
#define _BT_HAL_BT_HID_HOST_PROFILE_H

#include "bt_hal_manager_types.h"

#define BTHAL_HID_HOST_MAX_DSC_LEN    884

/**
 * @brief Bluetooth HID connection state type
 */
typedef enum
{
    eBTHIDHostStateConnected = 0,     /**< Connected */
    eBTHIDHostStateConnecting = 1,    /**< Connecting */
    eBTHIDHostStateDisconnected = 2,  /**< Disconnected */
    eBTHIDHostStateDisconnecting = 3, /**< Disconnecting */
    eBTHIDHostStateUnknown = 4        /**< Unknown */
} BTHIDHostConnectionState_t;

/**
 * @brief Bluetooth HID status type
 */
typedef enum
{
    /** Bluetooth HID status: Success */
    eBTHIDHostStatusOk = 0, /**< Sucess */

    /** Bluetooth HID status: Handshakes */
    eBTHIDHostStatusHsHIDNotReady = 1,     /**< HID Not Ready */
    eBTHIDHostStatusHsInvalidReportID = 2, /**< Invalid Report ID */
    eBTHIDHostStatusHsTransport = 3,       /**< Transaction not supported */
    eBTHIDHostStatusHsInvalidParam = 4,    /**< Invalid Parameter */
    eBTHIDHostStatusHsErr = 5,             /**< Unspecified Handshake Error */

    /** Bluetooth HID status: Errors */
    eBTHIDHostStatusErrGeneric = 6,         /**< Generic error */
    eBTHIDHostStatusErrSDP = 7,             /**< SDP error */
    eBTHIDHostStatusErrProtocol = 8,        /**< Set Protocol error */
    eBTHIDHostStatusErrDBFull = 9,          /**< Device Database full error */
    eBTHIDHostStatusErrTODUnsupported = 10, /**< Type of device not supported */
    eBTHIDHostStatusErrNoResources = 11,    /**< Out of system resource */
    eBTHIDHostStatusErrAuthFailed = 12,     /**< Authentication fail */
    eBTHIDHostStatusErrHDL = 13
} BTHIDHostStatus_t;

/**
 * @brief Bluetooth HID protocol type
 */
typedef enum
{
    eBTHIDHostModeReport = 0,        /**< Report Mode */
    eBTHIDHostModeBoot = 1,          /**< Boot Mode */
    eBTHIDHostModeUnsupported = 0xff /**< Unsupported Mode */
} BTHIDHostProtocolMode_t;

/**
 * @brief Bluetooth HID report type */
typedef enum
{
    eBTHIDHostReportInput = 1,  /**< Input Report */
    eBTHIDHostReportOutput = 2, /**< Output Report */
    eBTHIDHostReportFeature = 3 /**< Feature Report */
} BTHIDHostReportType_t;

/**
 * @brief Bluetooth HID info struct
 */
typedef struct
{
    int attrMask;
    uint8_t ucSubClass;
    uint8_t ucAppID;
    int vendorID;
    int productID;
    int version;
    uint8_t ucCtryCode;
    int dlLen;
    uint8_t ucDscList[ BTHAL_HID_HOST_MAX_DSC_LEN ];
} BTHIDHostHIDInfo_t;

/**
 * @brief HID connection state callback
 *
 * @param[in] pxBdAddr Address of the Remote device
 * @param[in] xState Connection state
 */
typedef void (* BTHIDHostConnectionStateCallback_t) ( BTBdaddr_t * pxBdAddr,
                                                      BTHIDHostConnectionState_t xState );

/**
 * @brief HID info callback
 *
 * @param[in] pxBdAddr Address of the Remote device
 * @param[in] pxInfo HID info
 */
typedef void (* BTHIDHostHIDInfoCallback_t) ( BTBdaddr_t * pxBdAddr,
                                              BTHIDHostHIDInfo_t * pxInfo );

/**
 * @brief HID protocol mode callback
 *
 * @param[in] pxBdAddr Address of the Remote device
 * @param[in] xStatus HID status
 * @param[in] xMode Protocol mode
 */
typedef void (* BTHIDHostProtocolModeCallback_t) ( BTBdaddr_t * pxBdAddr,
                                                   BTHIDHostStatus_t xStatus,
                                                   BTHIDHostProtocolMode_t xMode );

/**
 * @brief HID idle time callback
 *
 * @param[in] pxBdAddr Address of the Remote device
 * @param[in] xStatus HID status
 * @param[in] xIdleRate Idle time
 */
typedef void (* BTHIDHostIdleTimeCallback_t) ( BTBdaddr_t * pxBdAddr,
                                               BTHIDHostStatus_t xStatus,
                                               int xIdleRate );

/**
 * @brief HID get report callback
 *
 * @param[in] pxBdAddr Address of the Remote device
 * @param[in] xStatus HID status
 * @param[in] pxData Report data
 * @param[in] xSize Length of report data
 */
typedef void (* BTHIDHostGetReportCallback_t) ( BTBdaddr_t * pxBdAddr,
                                                BTHIDHostStatus_t xStatus,
                                                uint8_t * pxData,
                                                int xSize );

/**
 * @brief Virtual unplug callback
 *
 * @param[in] pxBdAddr Address of the Remote device
 * @param[in] xStatus HID status
 */
typedef void (* BTHIDHostVirtualUnplugCallback_t) ( BTBdaddr_t * pxBdAddr,
                                                    BTHIDHostStatus_t xStatus );

/**
 * @brief HID handshake callback
 *
 * @param[in] pxBdAddr Address of the Remote device
 * @param[in] xStatus HID status
 */
typedef void (* BTHIDHostHandshakeCallback_t) ( BTBdaddr_t * pxBdAddr,
                                                BTHIDHostStatus_t xStatus );

/**
 * @brief Battery report found callback
 *
 * @param[in] pxBdAddr Address of the Remote device
 * @param[in] xMin Minimum
 * @param[in] xMax Maximum
 */
typedef void (* BTHIDHostBatteryReportFoundCallback_t) ( BTBdaddr_t * pxBdAddr,
                                                         uint16_t xMin,
                                                         uint16_t xMax );

/**
 * @brief Battery level callback
 *
 * @param[in] pxBdAddr Address of the Remote device
 * @param[in] xStatus HID status
 * @param[in] xLevel Battery level
 * @param[in] xMin Minimum
 * @param[in] xMax Maximum
 */
typedef void (* BTHIDHostBatteryLevelCallback_t) ( BTBdaddr_t * pxBdAddr,
                                                   BTHIDHostStatus_t xStatus,
                                                   uint16_t xLevel,
                                                   uint16_t xMin,
                                                   uint16_t xMax );

/**
 * @brief Battery notification callback
 *
 * @param[in] pxBdAddr Address of the Remote device
 * @param[in] xLevel Battery level
 * @param[in] pxData Reserved for future use
 */
typedef void (* BTHIDHostBatteryNotifyCallback_t) ( BTBdaddr_t * pxBdAddr,
                                                    int32_t xLevel,
                                                    void * pxData );

/**
 * @brief Incoming HID data to be processed by upper layer
 *
 * @param[in] pxBdAddr Address of the Remote device
 * @param[in] pxData HID data
 * @param[in] xLength Length of data buffer
 */
typedef void (* BTHIDHostHIDEventCallback_t) ( BTBdaddr_t * pxBdAddr,
                                               uint8_t * pxData,
                                               uint16_t xLength );

typedef struct
{
    size_t xSize;
    BTHIDHostConnectionStateCallback_t xConnectionStateCback;
    BTHIDHostHIDInfoCallback_t xHIDInfoCback;
    BTHIDHostProtocolModeCallback_t xProtocolModeCback;
    BTHIDHostIdleTimeCallback_t xIdleTimeCback;
    BTHIDHostGetReportCallback_t xGetReportCback;
    BTHIDHostVirtualUnplugCallback_t xVirtualUnplugCback;
    BTHIDHostHandshakeCallback_t xHandshakeCback;
    BTHIDHostBatteryReportFoundCallback_t xBatteryReportFoundCback;
    BTHIDHostBatteryLevelCallback_t xBatteryLevelCback;
    BTHIDHostBatteryNotifyCallback_t xBatteryNotifyCback;
    BTHIDHostHIDEventCallback_t xHIDEventCback;
} BTHIDHostCallbacks_t;


typedef struct
{
    size_t xSize;

    /**
     * @brief Initializes HID Host module.
     *
     * @param[in] pxCallbacks HID Host callbacks
     * @return Returns eBTStatusSuccess on successful call.
     */
    BTStatus_t ( * pxHIDHostInit )( BTHIDHostCallbacks_t * pxCallbacks );

    /**
     * @brief Cleans HID Host module.
     *
     * @return Returns eBTStatusSuccess on successful call.
     */
    BTStatus_t ( * pxHIDHostCleanup )( void );

    /**
     * @brief Connect HID device
     *
     * @param[in] pxBdAddr Address of the Remote device
     * @return Returns eBTStatusSuccess on successful call.
     */
    BTStatus_t ( * pxHIDHostConnect )( BTBdaddr_t * pxBdAddr );

    /**
     * @brief Disconnect HID device
     *
     * @param[in] pxBdAddr Address of the Remote device
     * @return Returns eBTStatusSuccess on successful call.
     */
    BTStatus_t ( * pxHIDHostDisconnect )( BTBdaddr_t * pxBdAddr );

    /**
     * @brief Virtually unplug HID device
     *
     * @param[in] pxBdAddr Address of the Remote device
     * @return Returns eBTStatusSuccess on successful call.
     */
    BTStatus_t ( * pxHIDHostVirtualUnplug )( BTBdaddr_t * pxBdAddr );

    /**
     * @brief Set the HID device descriptor for the specified HID device
     *
     * @param[in] pxBdAddr Address of the Remote device
     * @param[in] pxInfo HID device descriptor
     * @return Returns eBTStatusSuccess on successful call.
     */
    BTStatus_t ( * pxHIDHostSetInfo )( BTBdaddr_t * pxBdAddr,
                                       BTHIDHostHIDInfo_t * pxInfo );

    /**
     * @brief Get protocol for HID device
     *
     * @param[in] pxBdAddr Address of the Remote device
     * @param[in] xMode Protocol mode
     * @return Returns eBTStatusSuccess on successful call.
     */
    BTStatus_t ( * pxHIDHostGetProtocol )( BTBdaddr_t * pxBdAddr,
                                           BTHIDHostProtocolMode_t xMode );

    /**
     * @brief Set protocol for HID device
     *
     * @param[in] pxBdAddr Address of the Remote device
     * @param[in] xMode Protocol mode
     * @return Returns eBTStatusSuccess on successful call.
     */
    BTStatus_t ( * pxHIDHostSetProtocol )( BTBdaddr_t * pxBdAddr,
                                           BTHIDHostProtocolMode_t xMode );

    /**
     * @brief Get HID report for HID device
     *
     * @param[in] pxBdAddr Address of the Remote device
     * @param[in] xType Report type
     * @param[in] xID Report ID
     * @param[in] xSize Buffer size
     * @return Returns eBTStatusSuccess on successful call.
     */
    BTStatus_t ( * pxHIDHostGetReport )( BTBdaddr_t * pxBdAddr,
                                         BTHIDHostReportType_t xType,
                                         uint8_t xID,
                                         int xSize );

    /**
     * @brief Set HID report for HID device
     *
     * @param[in] pxBdAddr Address of the Remote device
     * @param[in] xType Report type
     * @param[in] pxReport HID report
     * @return Returns eBTStatusSuccess on successful call.
     */
    BTStatus_t ( * pxHIDHostSetReport )( BTBdaddr_t * pxBdAddr,
                                         BTHIDHostReportType_t xType,
                                         char * pxReport );

    /**
     * @brief Send HID data to HID device
     *
     * @param[in] pxBdAddr Address of the Remote device
     * @param[in] pxData HID data
     * @return Returns eBTStatusSuccess on successful call.
     */
    BTStatus_t ( * pxHIDHostSendData )( BTBdaddr_t * pxBdAddr,
                                        char * pxData );

    /**
     * @brief Get HID battery level
     *
     * @param[in] pxBdAddr Address of the Remote device
     * @return Returns eBTStatusSuccess on successful call.
     */
    BTStatus_t ( * pxHIDHostGetBatteryLevel )( BTBdaddr_t * pxBdAddr );
} BTHIDHostInterface_t;

const BTHIDHostInterface_t * BT_GetHIDHostInterface( void );

#endif /* _BT_HAL_BT_HID_HOST_PROFILE_H */
/** @} */
