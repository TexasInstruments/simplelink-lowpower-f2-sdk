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
 * @file bt_hfp.h
 * @brief ACE BT HAL provides the interfaces for Hands Free profile for local device.
 * @addtogroup HAL_BLUETOOTH
 * USAGE
 * -----
 *
 * @{
 */

#ifndef BT_HFP_PROFILE_H_
#define BT_HFP_PROFILE_H_

#include "bt_hal_manager_types.h"

#define MAX_HF_CLIENTS    1

/**
 * @brief Bluetooth HFP AT response code
 */
typedef enum
{
    eBTHFPAtResponseError = 0, /**< Error Respone */
    eBTHFPAtResponseOk = 1     /**< OK Response */
} BTHFPATResponse_t;

/**
 * @brief Bluetooth HFP Connection State
 */
typedef enum
{
    eBTHFPConnectionStateDisconnected = 0, /**< Disconnected */
    eBTHFPConnectionStateConnecting = 1,   /**< Connecting */
    eBTHFPConnectionStateConnected = 2,    /**< Connected */
    eBTHFPConnectionStateSlcConnected = 3, /**< SLC Connected */
    eBTHFPConnectionStateDisconnecting = 4 /**< Disconnected */
} BTHFPConnectionState_t;

/**
 * @brief Bluetooth HFP Audio State
 */
typedef enum
{
    eBTHFPAudioStateDisconnected = 0, /**< Audio Disconnected */
    eBTHFPAudioStateConnecting = 1,   /**< Audio Connecting */
    eBTHFPAudioStateConnected = 2,    /**< Audio Connected */
    eBTHFPAudioStateDisconnecting = 3 /**< Audio Disconnecting */
} BTHFPAudioState_t;

/**
 * @brief Bluetooth HFP VR State
 */
typedef enum
{
    eBTHFPVrStateStopped = 0, /**< HFP VR Stopped */
    eBTHFPVrStateStarted = 1  /**< HFP VR Started */
} BTHFPVrState_t;

/**
 * @brief Bluetooth HFP Volume Type
 */
typedef enum
{
    eBTHFPVolumeTypeSpk = 0, /**< Speaker */
    eBTHFPVolumeTypeMic = 1  /**< Mic */
} BTHFPVolumeType_t;

/**
 * @brief Bluetooth HFP Noise Reduction and Echo Cancellation
 */
typedef enum
{
    eBTHFPNRECStop = 0, /**< Noise Reduction and Echo Cancellation stopped */
    eBTHFPNRECStart = 1 /**< Noise Reduction and Echo Cancellation started */
} BTHFPNREC_t;

/**
 * @brief Bluetooth HFP WBS codec setting
 */
typedef enum
{
    eBTHFPWbsNone = 0, /**< Set codec to none */
    eBTHFPWbsNo = 1,   /**< Set codec to CVSD */
    eBTHFPWbsYes = 2   /**< Set codec to mSBC */
} BTHFPWbsConfig_t;

/**
 * @brief Bluetooth HFP Call held handling
 */
typedef enum
{
    eBTHFPCHLDTypeReleaseHeld = 0,             /**< Terminate all held or set UDUB("busy") to a waiting call */
    eBTHFPCHLDTypeReleaseActiveAcceptHeld = 1, /**< Terminate all active calls and accepts a waiting/held call */
    eBTHFPCHLDTypeHoldActiveAcceptHeld = 2,    /**< Hold all active calls and accepts a waiting/held call */
    eBTHFPCHLDTypeAddHeldToConf = 3,           /**< Add all held calls to a conference */
} BTHFPCHLDType_t;

/**
 * @brief Bluetooth HFP Indicators
 */
typedef enum
{
    eBTHFPIndEnhancedDriverSafety = 1, /**< Enhanced Driver Safety*/
    eBTHFPIndBatteryLevelStatus = 2    /**< Battery Level Status*/
} BTHFPIndType_t;

/**
 * @brief Bluetooth HFP Indications status
 */
typedef enum
{
    eBTHFPIndDisabled = 0, /**< Indication disabled */
    eBTHFPIndEnabled = 1   /**< Indication enabled */
} BTHFPIndStatus_t;

/**
 * @brief Bluetooth HFP Connection State callback
 *
 * @param[in] xState connection state
 * @param[in] pxBdAddr pointer to device address
 */
typedef void (* BTHFPConnectionStateCallback_t)( BTBdaddr_t * pxBdAddr,
                                                 BTHFPConnectionState_t xState );

/**
 * @brief Bluetooth HFP Audio State callback
 *
 * @param[in] xState audio state
 * @param[in] pxBdAddr pointer to device address
 */
typedef void (* BTHFPAudioStateCallback_t)( BTBdaddr_t * pxBdAddr,
                                            BTHFPAudioState_t xState );

/**
 * @brief Bluetooth HFP Voice Recognition State callback
 *
 * @param[in] xState vr state
 * @param[in] pxBdAddr pointer to device address
 */
typedef void (* BTHFPVrCmdCallback_t)( BTBdaddr_t * pxBdAddr,
                                       BTHFPVrState_t xState );

/**
 * @brief Bluetooth HFP Answer call cmd callback
 *
 * @param[in] pxBdAddr pointer to device address
 */
typedef void (* BTHFPAnswerCallCmdCallback_t)( BTBdaddr_t * pxBdAddr );

/**
 * @brief Bluetooth HFP Hangup call cmd callback
 *
 * @param[in] pxBdAddr pointer to device address
 */
typedef void (* BTHFPHangupCallCmdCallback_t)( BTBdaddr_t * pxBdAddr );

/**
 * @brief Bluetooth HFP Volume cmd callback
 *
 * @param[in] xVolType Volume type
 * @param[in] lVolume volume value
 * @param[in] pxBdAddr pointer to remote device address
 */
typedef void (* BTHFPVolumeCmdCallback_t)( BTBdaddr_t * pxBdAddr,
                                           BTHFPVolumeType_t xVolType,
                                           int lVolume );

/**
 * @brief Bluetooth HFP Dial call cmd callback
 *
 * @param[in] pcNumber Phone number string
 * @param[in] pxBdAddr pointer to remote device address
 */
typedef void (* BTHFPDialCallCmdCallback_t)( BTBdaddr_t * pxBdAddr,
                                             char * pcNumber );

/**
 * @brief Bluetooth HFP DTMF Tone callback
 *
 * @param[in] cTone Tone
 * @param[in] pxBdAddr pointer to remote device address
 */
typedef void (* BTHFPDtmfCmdCallback_t)( BTBdaddr_t * pxBdAddr,
                                         char cTone );

/**
 * @brief Bluetooth HFP Noise Reduction Echo Cancellation callback
 *
 * @param[in] xNrec Nrec settings
 * @param[in] pxBdAddr pointer to remote device address
 */
typedef void (* BTHFPNrecCmdCallback_t)( BTBdaddr_t * pxBdAddr,
                                         BTHFPNREC_t xNrec );

/**
 * @brief Bluetooth HFP WBS callback
 *
 * @param[in] xWbs wbs codec
 * @param[in] pxBdAddr pointer to device address
 */
typedef void (* BTHFPWbsCallback_t)( BTBdaddr_t * pxBdAddr,
                                     BTHFPWbsConfig_t xWbs );

/**
 * @brief Bluetooth HFP Call Held callback
 *
 * @param[in] pxBdAddr pointer to device address
 */
typedef void (* BTHFPChldCmdCallback_t)( BTBdaddr_t * pxBdAddr,
                                         BTHFPCHLDType_t xChld );

/**
 * @brief Bluetooth HFP Cnum callback
 *
 * @param[in] pxBdAddr pointer to device address
 */
typedef void (* BTHFPCnumCmdCallback_t)( BTBdaddr_t * pxBdAddr );

/**
 * @brief Bluetooth HFP CIND callback
 *
 * @param[in] pxBdAddr pointer to device address
 */
typedef void (* BTHFPCindCmdCallback_t)( BTBdaddr_t * pxBdAddr );

/**
 * @brief Bluetooth HFP COPS callback
 *
 * @param[in] pxBdAddr pointer to device address
 */
typedef void (* BTHFPCopsCmdCallback_t)( BTBdaddr_t * pxBdAddr );

/**
 * @brief Bluetooth HFP CLCC callback
 *
 * @param[in] pxBdAddr pointer to device address
 */
typedef void (* BTHFPClccCmdCallback_t)( BTBdaddr_t * pxBdAddr );

/**
 * @brief Bluetooth HFP Unknown AT cmd callback
 *
 * @param[in] pcAtString AT cmd string
 * @param[in] pxBdAddr pointer to device address
 */
typedef void (* BTHFPUnknownAtCmdCallback_t)( BTBdaddr_t * pxBdAddr,
                                              char * pcAtString );

/**
 * @brief Bluetooth HFP BIND cmd callback
 *
 * @param[in] pxBdAddr pointer to device address
 */
typedef void (* BTHFPBindCmdCallback_t)( BTBdaddr_t * pxBdAddr );

/**
 * @brief Bluetooth HFP BIEV cmd callback
 *
 * @param[in] pcAtString AT cmd string
 * @param[in] pxBdAddr pointer to device address
 */
typedef void (* BTHFPBievCmdCallback_t)( BTBdaddr_t * pxBdAddr,
                                         char * pcAtString );

/**
 * @brief Bluetooth HFP Key Pressed cmd callback
 *
 * @param[in] xIndId Indicator type
 * @param[in] lIndValue Indicator value
 * @param[in] pxBdAddr pointer to device address
 */
typedef void (* BTHFPKeyPressedCmdCallback_t)( BTBdaddr_t * pxBdAddr,
                                               BTHFPIndType_t xIndId,
                                               int lIndValue );

/** BT HFP callbacks */
typedef struct
{
    size_t size;
    BTHFPConnectionStateCallback_t pxConnectionStateCb;
    BTHFPAudioStateCallback_t pxAudioStateCb;
    BTHFPVrCmdCallback_t pxVrCmdCb;
    BTHFPAnswerCallCmdCallback_t pxAnswerCallCmdCb;
    BTHFPHangupCallCmdCallback_t pxHangupCallCmdCb;
    BTHFPVolumeCmdCallback_t pxVolumeCmdCb;
    BTHFPDialCallCmdCallback_t pxDialCallCmdCb;
    BTHFPDtmfCmdCallback_t pxDtmfCmdCb;
    BTHFPNrecCmdCallback_t pxNrecCmdCb;
    BTHFPWbsCallback_t pxWbsCb;
    BTHFPChldCmdCallback_t pxChldCmdCb;
    BTHFPCnumCmdCallback_t pxCnumCmdCb;
    BTHFPCindCmdCallback_t pxCindCmdCb;
    BTHFPCopsCmdCallback_t pxCopsCmdCb;
    BTHFPClccCmdCallback_t pxClccCmdCb;
    BTHFPUnknownAtCmdCallback_t pxUnknownAtCmdCb;
    BTHFPBindCmdCallback_t pxBindCmdCb;
    BTHFPBievCmdCallback_t pxBievCmdCb;
    BTHFPKeyPressedCmdCallback_t pxKeyPressedCmdCb;
} BTHfpCallbacks_t;

/**
 * @brief Network Status
 */
typedef enum
{
    eBTHFPNetworkStateNotAvailable = 0, /**< Network Not Available */
    eBTHFPNetworkStateAvailable = 1     /**< Network Available */
} BTHFPNetworkState_t;

/**
 * @brief Service Type
 */
typedef enum
{
    eBTHFPServiceTypeHome = 0,   /**< Service Type Home */
    eBTHFPServiceTypeRoaming = 1 /**< Service Type Roaming */
} BTHFPServiceType_t;

/**
 * @brief Call State
 */
typedef enum
{
    eBTHFPCallStateActive = 0,   /**< Active */
    eBTHFPCallStateHeld = 1,     /**< Held */
    eBTHFPCallStateDialing = 2,  /**< Dialing */
    eBTHFPCallStateAlerting = 3, /**< Alerting */
    eBTHFPCallStateIncoming = 4, /**< Incoming*/
    eBTHFPCallStateWaiting = 5,  /**< Waiting */
    eBTHFPCallStateIdle = 6      /**< Idle */
} BTHFPCallState_t;

/**
 * @brief Call Direction
 */
typedef enum
{
    eBTHFPCallDirectionOutgoing = 0, /**< Outgoing Call */
    eBTHFPCallDirectionIncoming = 1  /**< Incoming Call */
} BTHFPCallDirection_t;

/**
 * @brief Call Mode
 */
typedef enum
{
    eBTHFPCallTypeVoice = 0, /**< Voice */
    eBTHFPCallTypeData = 1,  /**< Data */
    eBTHFPCallTypeFax = 2    /**< Fax */
} BTHFPCallMode_t;

/**
 * @brief Call Mpty Type
 */
typedef enum
{
    eBTHFPCallMptyTypeSingle = 0, /**< Single */
    eBTHFPCallMptyTypeMulti = 1   /**< Multi */
} BTHFPCallMptyType_t;

/**
 * @brief Call Address Type
 */
typedef enum
{
    eBTHFPCallAddrTypeUnknown = 0x81,      /**< Unknown Address Type */
    eBTHFPCallAddrTypeInternational = 0x91 /**< International Address Type */
} BTHFPCallAddrType_t;

typedef struct
{
    size_t xSize;

    /**
     * @brief Initializes standard HFP module
     *
     * @param[in] pxCallbacks Initialized callbacks for HFP
     * @param[in] lMaxHfClients Maximum number of HFP clients allowed
     * @return Returns eBTStatusSuccess on successful call.
     */
    BTStatus_t ( * pxHfpInit )( BTHfpCallbacks_t * pxCallbacks,
                                int lMaxHfClients );

    /**
     * @brief Connect to HFP client
     *
     * @param[in] pxBdAddr Address of the Remote device.
     * @return Returns eBTStatusSuccess on successful call.
     */
    BTStatus_t ( * pxHfpConnect )( BTBdaddr_t * pxBdAddr );

    /**
     * @brief Disconnect from HFP client
     *
     * @param[in] pxBdAddr Address of the Remote device.
     * @return Returns eBTStatusSuccess on successful call.
     */
    BTStatus_t ( * pxHfpDisconnect )( BTBdaddr_t * pxBdAddr );

    /**
     * @brief Creates Audio Connection
     *
     * @param[in] pxBdAddr Address of the Remote device.
     * @return Returns eBTStatusSuccess on successful call.
     */
    BTStatus_t ( * pxHfpConnectAudio )( BTBdaddr_t * pxBdAddr );

    /**
     * @brief Closes Audio Connection
     *
     * @param[in] pxBdAddr Address of the Remote device.
     * @return Returns eBTStatusSuccess on successful call.
     */
    BTStatus_t ( * pxHfpDisconnectAudio )( BTBdaddr_t * pxBdAddr );

    /**
     * @brief Start voice recognition
     *
     * @param[in] pxBdAddr Address of the Remote device.
     * @return Returns eBTStatusSuccess on successful call.
     */
    BTStatus_t ( * pxHfpStartVoiceRecognition )( BTBdaddr_t * pxBdAddr );

    /**
     * @brief Stop voice recognition
     *
     * @param[in] pxBdAddr Address of the Remote device.
     * @return Returns eBTStatusSuccess on successful call.
     */
    BTStatus_t ( * pxHfpStopVoiceRecognition )( BTBdaddr_t * pxBdAddr );

    /**
     * @brief Volume Control
     *
     * @param[in] pxBdAddr Address of the Remote device.
     * @param[in] xType Volume type
     * @param[in] lVol Volume level
     * @return Returns eBTStatusSuccess on successful call.
     */
    BTStatus_t ( * pxVolumeControl )( BTBdaddr_t * pxBdAddr,
                                      BTHFPVolumeType_t xType,
                                      int lVol );

    /**
     * @brief Combined device status change notification
     *
     * @param[in] xNetworkState Network state
     * @param[in] xSvcType Service type
     * @param[in] lSignal Signal strength
     * @param[in] lBattChg Battery charge
     * @return Returns eBTStatusSuccess on successful call.
     */
    BTStatus_t ( * pxDeviceStatusNotification )( BTHFPNetworkState_t xNetworkState,
                                                 BTHFPServiceType_t xSvcType,
                                                 int lSignal,
                                                 int lBattChg );

    /**
     * @brief Response for COPS command
     *
     * @param[in] pxBdAddr Address of the Remote device.
     * @param[in] pxCops COPS response
     * @return Returns eBTStatusSuccess on successful call.
     */
    BTStatus_t ( * pxCopsResponse )( BTBdaddr_t * pxBdAddr,
                                     const char * pxCops );

    /**
     * @brief Response for CIND command
     *
     * @param[in] pxBdAddr Address of the Remote device
     * @param[in] lSvc Network service
     * @param[in] lNumActive Number of call active
     * @param[in] lNumHeld Number of call held
     * @param[in] xCallSetupState Call setup state
     * @param[in] lSignal Signal strength
     * @param[in] lRoam Roaming indicator
     * @param[in] lBattChg Battery Charge
     * @return Returns eBTStatusSuccess on successful call.
     */
    BTStatus_t ( * pxCindResponse )( BTBdaddr_t * pxBdAddr,
                                     int lSvc,
                                     int lNumActive,
                                     int lNumHeld,
                                     BTHFPCallState_t xCallSetupState,
                                     int lSignal,
                                     int lRoam,
                                     int lBattChg );

    /**
     * @brief Pre-formatted AT response, typically in response to unknown AT cmd
     *
     * @param[in] pxBdAddr Address of the Remote device
     * @param[in] pxRsp AT response
     * @return Returns eBTStatusSuccess on successful call.
     */
    BTStatus_t ( * pxFormattedATResponse )( BTBdaddr_t * pxBdAddr,
                                            const char * pxRsp );

    /**
     * @brief AT response
     *
     * @param[in] pxBdAddr Address of the Remote device
     * @param[in] xResponseCode OK/error response
     * @param[in] lErrorCode Error code
     * @return Returns eBTStatusSuccess on successful call.
     */
    BTStatus_t ( * pxATResponse )( BTBdaddr_t * pxBdAddr,
                                   BTHFPATResponse_t xResponseCode,
                                   int lErrorCode );

    /**
     * @brief Response for CLCC command
     *
     * @param[in] pxBdAddr Address of the Remote device
     * @param[in] lIndex Call index
     * @param[in] xDir Call direction
     * @param[in] xMode Call mode
     * @param[in] xMpty Multi-party type
     * @return Returns eBTStatusSuccess on successful call.
     */
    BTStatus_t ( * pxClccResponse )( BTBdaddr_t * pxBdAddr,
                                     int lIndex,
                                     BTHFPCallDirection_t xDir,
                                     BTHFPCallState_t xState,
                                     BTHFPCallMode_t xMode,
                                     BTHFPCallMptyType_t xMpty,
                                     const char * pxNumber,
                                     BTHFPCallAddrType_t xType );

    /**
     * @brief Notify of a call state change
     *
     * @param[in] lNumActive Number of active call
     * @param[in] lNumHeld  Number of held call
     * @param[in] xCallSetupState Call setup state
     * @param[in] pxNumber Phone number
     * @param[in] xType Call address type
     * @return Returns eBTStatusSuccess on successful call.
     */
    BTStatus_t ( * pxPhoneStateChange )( int lNumActive,
                                         int lNumHeld,
                                         BTHFPCallState_t xCallSetupState,
                                         const char * pxNumber,
                                         BTHFPCallAddrType_t xType );

    /**
     * @brief Clean up HFP module
     *
     * @param None
     */
    void ( * cleanup )( void );

    /**
     * @brief Change current WBS configuration.
     *
     * @param[in] pxBdAddr Address of the Remote device
     * @param[in] xConfig WBS configuration
     * @return Returns eBTStatusSuccess on successful call.
     */
    BTStatus_t ( * pxConfigureWbs )( BTBdaddr_t * pxBdAddr,
                                     BTHFPWbsConfig_t xConfig );

    /**
     * @brief Send +BIND response
     *
     * @param[in] pxBdAddr Address of the Remote device
     * @param[in] xIndId Indicator ID
     * @param[in] xIndStatus Indicator status
     * @return Returns eBTStatusSuccess on successful call.
     */
    BTStatus_t ( * bind_response )( BTBdaddr_t * pxBdAddr,
                                    BTHFPIndType_t xIndId,
                                    BTHFPIndStatus_t xIndStatus );
} BTHfpInterface_t;

const BTHfpInterface_t * BT_GetHfpInterface( void );

#endif /* BT_HID_DEVICE_PROFILE_H_ */
/** @} */
