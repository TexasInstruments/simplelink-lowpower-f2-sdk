/******************************************************************************

@file  ble_app_services.h

@brief This file contains the ble_app_services for use
with the CC13XX_CC26XX Bluetooth Low Energy Protocol Stack.

Group: WCS, BTS
Target Device: cc13xx_cc26xx

******************************************************************************

 Copyright (c) 2022-2023, Texas Instruments Incorporated
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions
 are met:

 *  Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.

 *  Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.

 *  Neither the name of Texas Instruments Incorporated nor the names of
    its contributors may be used to endorse or promote products derived
    from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

******************************************************************************


*****************************************************************************/

#ifndef BLEAPPUTIL_API_H
#define BLEAPPUTIL_API_H

#ifdef __cplusplus
extern "C"
{
#endif


/*********************************************************************
 * INCLUDES
 */
#include <bcomdef.h>
#include "ble_stack_api.h"
#include <ti/display/Display.h>

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * TYPEDEFS
 */
typedef ICall_Hdr               BLEAppUtil_msgHdr_t;
typedef ICall_EntityID          BLEAppUtil_entityId_t;
typedef uint8_t                 BLEAppUtil_BDaddr[B_ADDR_LEN];

// Handlers types
typedef void (*EventHandler_t)(uint32 event, BLEAppUtil_msgHdr_t *pMsgData);
typedef void (*ErrorHandler_t)(int32 errorCode , void* pInfo);
typedef void (*StackInitDone_t)(gapDeviceInitDoneEvent_t *deviceInitDoneData);
// Profile/Service Invoke from BLE App Util context type
typedef void (*InvokeFromBLEAppUtilContext_t)(char *pData);

// Event Handler Types
typedef enum BLEAppUtil_eventHandlerType_e
{
    BLEAPPUTIL_GAP_CONN_TYPE,
    BLEAPPUTIL_CONN_NOTI_TYPE,
    BLEAPPUTIL_GAP_ADV_TYPE,
    BLEAPPUTIL_GAP_SCAN_TYPE,
    BLEAPPUTIL_GAP_PERIODIC_TYPE,
    BLEAPPUTIL_GATT_TYPE,
    BLEAPPUTIL_PASSCODE_TYPE,
    BLEAPPUTIL_PAIR_STATE_TYPE,
    BLEAPPUTIL_L2CAP_DATA_TYPE,
    BLEAPPUTIL_L2CAP_SIGNAL_TYPE,
    BLEAPPUTIL_HCI_DATA_TYPE,
    BLEAPPUTIL_HCI_GAP_TYPE,
    BLEAPPUTIL_HCI_SMP_TYPE,
    BLEAPPUTIL_HCI_SMP_META_TYPE,
    BLEAPPUTIL_HCI_CTRL_TO_HOST_TYPE
} BLEAppUtil_eventHandlerType_e;

// GAP Conn event mask
typedef enum BLEAppUtil_GAPConnEventMaskFlags_e
{
    BLEAPPUTIL_LINK_ESTABLISHED_EVENT           = (uint32_t)BV(0),
    BLEAPPUTIL_LINK_TERMINATED_EVENT            = (uint32_t)BV(1),
    BLEAPPUTIL_CONNECTING_CANCELLED_EVENT       = (uint32_t)BV(2),
    BLEAPPUTIL_LINK_PARAM_UPDATE_EVENT          = (uint32_t)BV(3),
    BLEAPPUTIL_LINK_PARAM_UPDATE_REQ_EVENT      = (uint32_t)BV(4),
    BLEAPPUTIL_LINK_PARAM_UPDATE_REJECT_EVENT   = (uint32_t)BV(5),
    BLEAPPUTIL_SIGNATURE_UPDATED_EVENT          = (uint32_t)BV(6),
    BLEAPPUTIL_AUTHENTICATION_COMPLETE_EVENT    = (uint32_t)BV(7),
    BLEAPPUTIL_PASSKEY_NEEDED_EVENT             = (uint32_t)BV(8),
    BLEAPPUTIL_SLAVE_REQUESTED_SECURITY_EVENT   = (uint32_t)BV(9),
    BLEAPPUTIL_BOND_COMPLETE_EVENT              = (uint32_t)BV(10),
    BLEAPPUTIL_PAIRING_REQ_EVENT                = (uint32_t)BV(11),
    BLEAPPUTIL_AUTHENTICATION_FAILURE_EVT       = (uint32_t)BV(12),
    BLEAPPUTIL_BOND_LOST_EVENT                  = (uint32_t)BV(13)
} BLEAppUtil_GAPConnEventMaskFlags_e;

// Conn event event mask
typedef enum BLEAppUtil_ConnEventNotiEventMaskFlags_e
{
    BLEAPPUTIL_CONN_NOTI_EVENT_INVALID             = (uint32_t)BV(0),
    BLEAPPUTIL_CONN_NOTI_CONN_ESTABLISHED          = (uint32_t)BV(1),
    BLEAPPUTIL_CONN_NOTI_PHY_UPDATE                = (uint32_t)BV(2),
    BLEAPPUTIL_CONN_NOTI_CONN_EVENT_ALL            = (uint32_t)BV(3),
} BLEAppUtil_ConnEventNotiEventMaskFlags_e;

// GAP ADV event mask
typedef enum BLEAppUtil_GAPAdvEventMaskFlags_e
{
    // Gets advertising handle using pBuf
    BLEAPPUTIL_ADV_START_AFTER_ENABLE    = GAP_EVT_ADV_START_AFTER_ENABLE,
    // Gets advertising handle using pBuf
    BLEAPPUTIL_ADV_END_AFTER_DISABLE     = GAP_EVT_ADV_END_AFTER_DISABLE,
    // Gets advertising handle using pBuf
    BLEAPPUTIL_ADV_START                 = GAP_EVT_ADV_START,
    // Gets advertising handle using pBuf
    BLEAPPUTIL_ADV_END                   = GAP_EVT_ADV_END,
    // Gets @ref GapAdv_setTerm_t using pBuf
    BLEAPPUTIL_ADV_SET_TERMINATED        = GAP_EVT_ADV_SET_TERMINATED,
    // Gets @ref GapAdv_scanReqReceived_t using pBuf
    BLEAPPUTIL_SCAN_REQ_RECEIVED         = GAP_EVT_SCAN_REQ_RECEIVED,
    // Gets @ref GapAdv_truncData_t using pBuf
    BLEAPPUTIL_ADV_DATA_TRUNCATED        = GAP_EVT_ADV_DATA_TRUNCATED,
    BLEAPPUTIL_ADV_INSUFFICIENT_MEMORY   = GAP_EVT_INSUFFICIENT_MEMORY
} BLEAppUtil_GAPAdvEventMaskFlags_e;

// GAP Scan event mask
typedef enum BLEAppUtil_GAPScanEventMaskFlags_e
{
    BLEAPPUTIL_SCAN_ENABLED              = GAP_EVT_SCAN_ENABLED,
    BLEAPPUTIL_SCAN_DISABLED             = GAP_EVT_SCAN_DISABLED,
    BLEAPPUTIL_SCAN_PRD_ENDED            = GAP_EVT_SCAN_PRD_ENDED,
    BLEAPPUTIL_SCAN_DUR_ENDED            = GAP_EVT_SCAN_DUR_ENDED,
    BLEAPPUTIL_SCAN_INT_ENDED            = GAP_EVT_SCAN_INT_ENDED,
    BLEAPPUTIL_SCAN_WND_ENDED            = GAP_EVT_SCAN_WND_ENDED,
    BLEAPPUTIL_ADV_REPORT                = GAP_EVT_ADV_REPORT,
    BLEAPPUTIL_ADV_REPORT_FULL           = GAP_EVT_ADV_REPORT_FULL,
    BLEAPPUTIL_SCAN_INSUFFICIENT_MEMORY  = GAP_EVT_INSUFFICIENT_MEMORY
} BLEAppUtil_GAPScanEventMaskFlags_e;

// GAP Periodic event mask
typedef enum BLEAppUtil_GAPPeriodicEventMaskFlags_e
{
    BLEAPPUTIL_ADV_SET_PERIODIC_ADV_PARAMS_EVENT = (uint32_t)BV(0),
    BLEAPPUTIL_ADV_SET_PERIODIC_ADV_DATA_EVENT   = (uint32_t)BV(1),
    BLEAPPUTIL_ADV_SET_PERIODIC_ADV_ENABLE_EVENT = (uint32_t)BV(2),
    BLEAPPUTIL_SCAN_CREATE_SYNC_EVENT            = (uint32_t)BV(3),
    BLEAPPUTIL_SCAN_SYNC_CANCEL_EVENT            = (uint32_t)BV(4),
    BLEAPPUTIL_SCAN_TERMINATE_SYNC_EVENT         = (uint32_t)BV(5),
    BLEAPPUTIL_SCAN_PERIODIC_RECEIVE_EVENT       = (uint32_t)BV(6),
    BLEAPPUTIL_SCAN_ADD_DEVICE_ADV_LIST_EVENT    = (uint32_t)BV(7),
    BLEAPPUTIL_SCAN_REMOVE_DEVICE_ADV_LIST_EVENT = (uint32_t)BV(8),
    BLEAPPUTIL_SCAN_READ_ADV_LIST_SIZE_EVENT     = (uint32_t)BV(9),
    BLEAPPUTIL_SCAN_CLEAR_ADV_LIST_EVENT         = (uint32_t)BV(10),
    BLEAPPUTIL_SCAN_PERIODIC_ADV_SYNC_EST_EVENT  = (uint32_t)BV(11),
    BLEAPPUTIL_SCAN_PERIODIC_ADV_SYNC_LOST_EVENT = (uint32_t)BV(12),
    BLEAPPUTIL_SCAN_PERIODIC_ADV_REPORT_EVENT    = (uint32_t)BV(13)
} BLEAppUtil_GAPPeriodicEventMaskFlags_e;

// GAP Pairing event mask
typedef enum BLEAppUtil_PairingEventMaskFlags_e
{
    BLEAPPUTIL_PAIRING_STATE_STARTED     = (uint32_t)BV(0),
    BLEAPPUTIL_PAIRING_STATE_COMPLETE    = (uint32_t)BV(1),
    BLEAPPUTIL_PAIRING_STATE_ENCRYPTED   = (uint32_t)BV(2),
    BLEAPPUTIL_PAIRING_STATE_BOND_SAVED  = (uint32_t)BV(3),
    BLEAPPUTIL_PAIRING_STATE_CAR_READ    = (uint32_t)BV(4),
    BLEAPPUTIL_PAIRING_STATE_RPAO_READ   = (uint32_t)BV(5),
    BLEAPPUTIL_GENERATE_ECC_DONE         = (uint32_t)BV(6)
} BLEAppUtil_PairingEventMaskFlags_e;

// GATT event mask
typedef enum BLEAppUtil_GATTEventMaskFlags_e
{
    BLEAPPUTIL_ATT_ERROR_RSP                   = (uint32_t)BV(0),
    BLEAPPUTIL_ATT_EXCHANGE_MTU_REQ            = (uint32_t)BV(1),
    BLEAPPUTIL_ATT_EXCHANGE_MTU_RSP            = (uint32_t)BV(2),
    BLEAPPUTIL_ATT_FIND_INFO_REQ               = (uint32_t)BV(3),
    BLEAPPUTIL_ATT_FIND_INFO_RSP               = (uint32_t)BV(4),
    BLEAPPUTIL_ATT_FIND_BY_TYPE_VALUE_REQ      = (uint32_t)BV(5),
    BLEAPPUTIL_ATT_FIND_BY_TYPE_VALUE_RSP      = (uint32_t)BV(6),
    BLEAPPUTIL_ATT_READ_BY_TYPE_REQ            = (uint32_t)BV(7),
    BLEAPPUTIL_ATT_READ_BY_TYPE_RSP            = (uint32_t)BV(8),
    BLEAPPUTIL_ATT_READ_REQ                    = (uint32_t)BV(9),
    BLEAPPUTIL_ATT_READ_RSP                    = (uint32_t)BV(10),
    BLEAPPUTIL_ATT_READ_BLOB_REQ               = (uint32_t)BV(11),
    BLEAPPUTIL_ATT_READ_BLOB_RSP               = (uint32_t)BV(12),
    BLEAPPUTIL_ATT_READ_MULTI_REQ              = (uint32_t)BV(13),
    BLEAPPUTIL_ATT_READ_MULTI_RSP              = (uint32_t)BV(14),
    BLEAPPUTIL_ATT_READ_BY_GRP_TYPE_REQ        = (uint32_t)BV(15),
    BLEAPPUTIL_ATT_READ_BY_GRP_TYPE_RSP        = (uint32_t)BV(16),
    BLEAPPUTIL_ATT_WRITE_REQ                   = (uint32_t)BV(17),
    BLEAPPUTIL_ATT_WRITE_RSP                   = (uint32_t)BV(18),
    BLEAPPUTIL_ATT_PREPARE_WRITE_REQ           = (uint32_t)BV(19),
    BLEAPPUTIL_ATT_PREPARE_WRITE_RSP           = (uint32_t)BV(20),
    BLEAPPUTIL_ATT_EXECUTE_WRITE_REQ           = (uint32_t)BV(21),
    BLEAPPUTIL_ATT_EXECUTE_WRITE_RSP           = (uint32_t)BV(22),
    BLEAPPUTIL_ATT_HANDLE_VALUE_NOTI           = (uint32_t)BV(23),
    BLEAPPUTIL_ATT_HANDLE_VALUE_IND            = (uint32_t)BV(24),
    BLEAPPUTIL_ATT_HANDLE_VALUE_CFM            = (uint32_t)BV(25),
    BLEAPPUTIL_ATT_UNSUPPORTED_METHOD          = (uint32_t)BV(26),
    BLEAPPUTIL_ATT_WRITE_CMD                   = (uint32_t)BV(27),
    BLEAPPUTIL_ATT_SIGNED_WRITE_CMD            = (uint32_t)BV(28),
    BLEAPPUTIL_ATT_TRANSACTION_READY_EVENT     = (uint32_t)BV(29),
    BLEAPPUTIL_ATT_FLOW_CTRL_VIOLATED_EVENT    = (uint32_t)BV(30),
    BLEAPPUTIL_ATT_MTU_UPDATED_EVENT           = (uint32_t)BV(31),
} BLEAppUtil_GATTEventMaskFlags_e;

// L2CAP Signal event mask
typedef enum BLEAppUtil_L2CAPSignalEventMaskFlags_e
{
    BLEAPPUTIL_L2CAP_CHANNEL_ESTABLISHED_EVT   = (uint32_t)BV(0),
    BLEAPPUTIL_L2CAP_CHANNEL_TERMINATED_EVT    = (uint32_t)BV(1),
    BLEAPPUTIL_L2CAP_OUT_OF_CREDIT_EVT         = (uint32_t)BV(2),
    BLEAPPUTIL_L2CAP_PEER_CREDIT_THRESHOLD_EVT = (uint32_t)BV(3),
    BLEAPPUTIL_L2CAP_SEND_SDU_DONE_EVT         = (uint32_t)BV(4),
    BLEAPPUTIL_L2CAP_NUM_CTRL_DATA_PKT_EVT     = (uint32_t)BV(5)
} BLEAppUtil_L2CAPSignalEventMaskFlags_e;

// HCI event mask
typedef enum BLEAppUtil_HciEventMaskFlags_e
{
    BLEAPPUTIL_HCI_DISCONNECTION_COMPLETE_EVENT_CODE    = (uint32_t)BV(0),
    BLEAPPUTIL_HCI_COMMAND_COMPLETE_EVENT_CODE          = (uint32_t)BV(1),
    BLEAPPUTIL_HCI_COMMAND_STATUS_EVENT_CODE            = (uint32_t)BV(2),
    BLEAPPUTIL_HCI_LE_EVENT_CODE                        = (uint32_t)BV(3),
    BLEAPPUTIL_HCI_VE_EVENT_CODE                        = (uint32_t)BV(4),
    BLEAPPUTIL_HCI_BLE_HARDWARE_ERROR_EVENT_CODE        = (uint32_t)BV(5),
    BLEAPPUTIL_HCI_NUM_OF_COMPLETED_PACKETS_EVENT_CODE  = (uint32_t)BV(6),
    BLEAPPUTIL_HCI_APTO_EXPIRED_EVENT_CODE              = (uint32_t)BV(7),
    BLEAPPUTIL_HCI_EVENT_PACKET                         = (uint32_t)BV(8),
    BLEAPPUTIL_HCI_ACL_DATA_PACKET                      = (uint32_t)BV(9)
} BLEAppUtil_HciEventMaskFlags_e;

// Profile role mask
typedef enum BLEAppUtil_Profile_Roles_e
{
    BLEAPPUTIL_BROADCASTER_ROLE = GAP_PROFILE_BROADCASTER,
    BLEAPPUTIL_OBSERVER_ROLE    = GAP_PROFILE_OBSERVER,
    BLEAPPUTIL_PERIPHERAL_ROLE  = GAP_PROFILE_PERIPHERAL,
    BLEAPPUTIL_CENTRAL_ROLE     = GAP_PROFILE_CENTRAL
} BLEAppUtil_Profile_Roles_e;

typedef struct
{
    uint8_t event;                // event type
    void    *pData;               // pointer to message
} BLEAppUtil_appEvt_t;

/**
 * BLEAppUtil Adv Event Data Structure
 *
 * Used to store the data of adv event
 */
typedef struct
{
    uint32_t        event;
    GapAdv_data_t   *pBuf;
    uint32_t        *arg;
} BLEAppUtil_AdvEventData_t;

/**
 * BLEAppUtil Scan Event Data Structure
 *
 * Used to store the data of scan event
 */
typedef struct
{
    uint32_t        event;
    GapScan_data_t  *pBuf;
    uint32_t        *arg;
} BLEAppUtil_ScanEventData_t;

/**
 * BLEAppUtil PairState Event Data Structure
 *
 * Used to store the pair state data
 */
typedef struct
{
    uint16_t connHandle;
    uint32_t state;
    uint8_t  status;
} BLEAppUtil_PairStateData_t;

/**
 * BLEAppUtil Passcode Data Structure
 *
 * Used to store the passcode data
 */
typedef struct
{
    BLEAppUtil_BDaddr   deviceAddr;
    uint16_t            connHandle;
    uint8_t             uiInputs;
    uint8_t             uiOutputs;
    uint32_t            numComparison;
} BLEAppUtil_PasscodeData_t;

typedef struct
{
    BLEAppUtil_BDaddr   address;
    uint8_t             addrType;
    uint8_t             rssi;
} BLEAppUtil_connCandidate_t;

/**
 * BLEAppUtil Adv Init Structure
 *
 * Should be created by the application and initialize
 * by calling @ref BLEAppUtil_initAdvSet.
 */
typedef struct
{
    uint16_t        advDataLen;
    uint8_t         *advData;
    uint16_t        scanRespDataLen;
    uint8_t         *scanRespData;
    GapAdv_params_t *advParam;
} BLEAppUtil_AdvInit_t;

/**
 * BLEAppUtil Adv Start Structure
 *
 * Should be created by the application and passed to
 * @ref BLEAppUtil_advStart.
 */
typedef struct
{
    GapAdv_enableOptions_t  enableOptions;
    uint16_t                durationOrMaxEvents;
} BLEAppUtil_AdvStart_t;

/**
 * BLEAppUtil Scan Init Structure
 *
 * Should be created by the application and initialized
 * by calling @ref BLEAppUtil_scanInit.
 */
typedef struct
{
    uint8_t                 primPhy;
    GapScan_ScanType_t      scanType;
    uint16_t                scanInterval;
    uint16_t                scanWindow;
    //!< Bitmap of adv report fields to keep in the advertising report list
    uint16_t                advReportFields;
    //!< PHYs on which Advs should be received on the primary Adv channel
    uint8_t                 scanPhys;
    //!< Scanning filter policy regarding White List and Adv type
    uint8_t                 fltPolicy;
    //!< Filter by type of PDU
    uint16_t                fltPduType;
    //!< Filter by RSSI. Only packets received with the specified RSSI or above will be reported.
    int8_t                  fltMinRssi;
    //!< Filter by Discovery Mode.
    uint8_t                 fltDiscMode;
    //!< Filter out duplicated packets, determining by address and DID.
    uint8_t                 fltDup;
} BLEAppUtil_ScanInit_t;

/**
 * BLEAppUtil Scan Start Structure
 *
 * Should be created by the application and passed to
 * @ref BLEAppUtil_scanStart.
 */
typedef struct
{
    uint16_t scanPeriod;
    uint16_t scanDuration;
    uint8_t  maxNumReport;
}BLEAppUtil_ScanStart_t;

/**
 * BLEAppUtil Conn Params Structure
 *
 * Should be created by the application and passed to
 * @ref BLEAppUtil_SetConnParams.
 */
typedef struct
{
    uint8_t  initPhys;
    uint16_t scanInterval;
    uint16_t scanWindow;
    uint16_t minConnInterval;
    uint16_t maxConnInterval;
    uint16_t connLatency;
    uint16_t supTimeout;
} BLEAppUtil_ConnParams_t;

/**
 * BLEAppUtil Connect Params Structure
 *
 * Should be created by the application and passed to
 * @ref BLEAppUtil_Connect.
 */
typedef struct
{
    GAP_Peer_Addr_Types_t   peerAddrType;
    BLEAppUtil_BDaddr       pPeerAddress;
    uint8_t                 phys;
    uint16_t                timeout;
}BLEAppUtil_ConnectParams_t;

/**
 * BLEAppUtil General Params Structure
 *
 * Should be created by the application and passed to
 * @ref BLEAppUtil_init.
 */
typedef struct
{
    // Task related parameters - BLEAppUtil task priority. Higher values represent higher priorities
    int                         taskPriority;
    size_t                      taskStackSize;
    // BLE stack related parameters
    BLEAppUtil_Profile_Roles_e  profileRole;
    // GAP GATT Server (GGS) Parameters
    uint8_t                     *deviceNameAtt;
    uint16_t                    appearanceAtt;
    gapPeriConnectParams_t      periConnParamsAtt;
    // Device related parameters
    GAP_Addr_Modes_t            addressMode;
    uint8_t                     *pDeviceRandomAddress;
} BLEAppUtil_GeneralParams_t;

/**
 * BLEAppUtil Peripheral Central Params Structure
 *
 * Should be created by the application and passed to
 * @ref BLEAppUtil_init.
 */
typedef struct
{
    gapBondParams_t             *gapBondParams;
    Gap_updateDecision_t        connParamUpdateDecision;
} BLEAppUtil_PeriCentParams_t;

typedef union
{
    // BLEAPPUTIL_LINK_ESTABLISHED_EVENT
    gapEstLinkReqEvent_t            estLinkReqEvent;
    // BLEAPPUTIL_LINK_TERMINATED_EVENT
    gapTerminateLinkEvent_t         termLinkEvent;
    // BLEAPPUTIL_CONNECTING_CANCELLED_EVENT
    gapConnCancelledEvent_t         gapConnCancelledEvent;
    // BLEAPPUTIL_LINK_PARAM_UPDATE_EVENT and
    // BLEAPPUTIL_LINK_PARAM_UPDATE_REJECT_EVENT
    gapLinkUpdateEvent_t            linkUpdateEvent;
    // BLEAPPUTIL_LINK_PARAM_UPDATE_REQ_EVENT
    gapUpdateLinkParamReqEvent_t    paramUpdateReqEvent;
    // BLEAPPUTIL_SIGNATURE_UPDATED_EVENT
    gapSignUpdateEvent_t            signUpdateEvent;
    // BLEAPPUTIL_AUTHENTICATION_COMPLETE_EVENT
    gapAuthCompleteEvent_t          authCompleteEvent;
    // BLEAPPUTIL_PASSKEY_NEEDED_EVENT
    gapPasskeyNeededEvent_t         passkeyIsNeededEvent;
    // BLEAPPUTIL_SLAVE_REQUESTED_SECURITY_EVENT
    gapSlaveSecurityReqEvent_t      slaveSecurityReqEvent;
    // BLEAPPUTIL_BOND_COMPLETE_EVENT
    gapBondCompleteEvent_t          bondCompleteEvent;
    // BLEAPPUTIL_PAIRING_REQ_EVENT
    gapPairingReqEvent_t            pairingReqEvent;
    // BLEAPPUTIL_AUTHENTICATION_FAILURE_EVT - no data
    // BLEAPPUTIL_BOND_LOST_EVENT
    gapBondLostEvent_t              bondLostEvent;
    // BLEAPPUTIL_GAP_CB_EVENT_INVALID, BLEAPPUTIL_GAP_CB_CONN_ESTABLISHED
    // and BLEAPPUTIL_GAP_CB_PHY_UPDATE
    Gap_ConnEventRpt_t              connEventRpt;
    gapAuthParams_t                 authParams;
    gapPairingReq_t                 pairingReq;
    gapUpdateLinkParamReq_t         paramUpdateReq;
    gapUpdateLinkParamReqReply_t    paramUpdateReqReplay;
} BLEAppUtil_gapConnData_t;

typedef struct
{
    union
    {
        // Msg Data of BLEAPPUTIL_GAP_CONN_TYPE
        gapEventHdr_t               gapConnData;
        // Msg Data of BLEAPPUTIL_GAP_ADV_TYPE
        BLEAppUtil_AdvEventData_t   gapAdvData;
        // Msg Data of BLEAPPUTIL_GAP_SCAN_TYPE
        BLEAppUtil_ScanEventData_t  gapScanData;
        // Msg Data of BLEAPPUTIL_GAP_PERIODIC_TYPE
        gapEventHdr_t               gapPeriodicData;
        // Msg Data of BLEAPPUTIL_GATT_TYPE
        gattMsgEvent_t              gattData;
        // Msg Data of BLEAPPUTIL_PASSCODE_TYPE
        BLEAppUtil_PasscodeData_t   passcodeData;
        // Msg Data of BLEAPPUTIL_PAIR_STATE_TYPE
        BLEAppUtil_PairStateData_t  pairStateData;
        // Msg Data of BLEAPPUTIL_L2CAP_DATA_TYPE
        l2capDataEvent_t            l2capData;
        // Msg Data of BLEAPPUTIL_L2CAP_SIGNAL_TYPE
        l2capSignalEvent_t          l2capSignalData;
        // Msg Data of BLEAPPUTIL_HCI_DATA_TYPE
        hciDataEvent_t              hciData;
        // Msg Data of BLEAPPUTIL_HCI_GAP_TYPE
        ICall_HciExtEvt             hciGapData;
        // Msg Data of BLEAPPUTIL_HCI_SMP_TYPE
        ICall_HciExtEvt             hciSMPData;
        // Msg Data of BLEAPPUTIL_HCI_SMP_META_TYPE
        ICall_HciExtEvt             hciSMPMetaData;
    };
} BLEAppUtil_msgData_t;

/**
 * BLEAppUtil Events handlers Structure
 *
 * This must be setup by the application and passed to the BLEAppUtil module
 * by calling @ref BLEAppUtil_registerEventHandler.
 */
typedef struct
{
    BLEAppUtil_eventHandlerType_e   handlerType;
    EventHandler_t                  pEventHandler;
    uint32_t                        eventMask;
} BLEAppUtil_EventHandler_t;

/*********************************************************************
 * GLOBAL VARIABLES
 */
// Display Interface
extern Display_Handle dispHandle;
extern uint8_t dispIndex;

/*********************************************************************
 * FUNCTIONS
 */

void BLEAppUtil_init(ErrorHandler_t errorHandler, StackInitDone_t initDoneHandler,
                     BLEAppUtil_GeneralParams_t *initGeneralParams,
                     BLEAppUtil_PeriCentParams_t *initPeriCentParams);
BLEAppUtil_entityId_t BLEAppUtil_getSelfEntity(void);

// Register function for BLE stack events
bStatus_t BLEAppUtil_registerEventHandler(BLEAppUtil_EventHandler_t *eventHandler);
bStatus_t BLEAppUtil_unRegisterEventHandler(BLEAppUtil_EventHandler_t *eventHandler);

// Profiles/Services APIs
bStatus_t BLEAppUtil_invokeFunction(InvokeFromBLEAppUtilContext_t callback, char* pData);

// Adv functions
bStatus_t BLEAppUtil_initAdvSet(uint8 *advHandle, const BLEAppUtil_AdvInit_t *advInitInfo);
bStatus_t BLEAppUtil_advStart(uint8 handle, const BLEAppUtil_AdvStart_t *advStartInfo);
bStatus_t BLEAppUtil_advStop(uint8 handle);
bStatus_t BLEAppUtil_advDestroy(uint8 handle, GapAdv_freeBufferOptions_t freeOptions);
bStatus_t BLEAppUtil_advSetParam(uint8 handle, GapAdv_ParamId_t paramID, void *pValue);
bStatus_t BLEAppUtil_advGetParam(uint8 handle, GapAdv_ParamId_t paramID, void *pValue, uint8_t *pLen);
bStatus_t BLEAppUtil_advLoadByHandle(uint8 handle, GapAdv_dataTypes_t dataType, uint16 len, uint8 *pBuf);
bStatus_t BLEAppUtil_advPrepareLoadByHandle(uint8 handle, GapAdv_freeBufferOptions_t freeOptions);
// TODO: Add periodic advertise functions

// Scan functions
bStatus_t BLEAppUtil_scanInit(const BLEAppUtil_ScanInit_t *scanInitInfo);
bStatus_t BLEAppUtil_scanStart(const BLEAppUtil_ScanStart_t *scanStartInfo);
bStatus_t BLEAppUtil_scanStop(void);
bStatus_t BLEAppUtil_setPhyParams(uint8_t primPhys, GapScan_ScanType_t type, uint16_t interval, uint16_t window);
bStatus_t BLEAppUtil_getPhyParams(uint8_t primPhy, GapScan_ScanType_t* pType, uint16_t* pInterval, uint16_t* pWindow);
bStatus_t BLEAppUtil_setParam(GapScan_ParamId_t paramId, void* pValue);
bStatus_t BLEAppUtil_getParam(GapScan_ParamId_t paramId, void* pValue, uint8_t *pLen);
bStatus_t BLEAppUtil_getAdvReport(uint8_t rptIdx, GapScan_Evt_AdvRpt_t* pAdvRpt);
// TODO: Add periodic scan functions

// Connection initiation functions
bStatus_t BLEAppUtil_SetConnParams(const BLEAppUtil_ConnParams_t *connParams);
bStatus_t BLEAppUtil_Connect(BLEAppUtil_ConnectParams_t *connParams);
bStatus_t BLEAppUtil_CancelConnect(void);

// Connection Event notifications
bStatus_t BLEAppUtil_registerConnNotifHandler(BLEAppUtil_EventHandler_t *eventHandler, uint16_t connHandle);
bStatus_t BLEAppUtil_unRegisterConnNotifHandler(void);
bStatus_t BLEAppUtil_paramUpdateRsp(gapUpdateLinkParamReqEvent_t *pReq, uint8 accept);

char *BLEAppUtil_convertBdAddr2Str(uint8_t *pAddr);
/*********************************************************************
*********************************************************************/


#ifdef __cplusplus
}
#endif

#endif /* BLEAPPUTIL_API_H */
