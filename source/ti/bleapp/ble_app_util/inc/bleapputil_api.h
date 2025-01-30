/******************************************************************************

@file  bleapputil_api.h

@brief This file contains the bleapputil_api for use
with the Bluetooth Low Energy Protocol Stack.

Group: WCS, BTS
Target Device: cc13xx_cc26xx

******************************************************************************

 Copyright (c) 2022-2025, Texas Instruments Incorporated
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
/**
 *  @defgroup BLEAppUtil BLEAppUtil
 *  @brief This module implements BLEAppUtil functions
 *  @{
 *  @file  bleapputil_api.h
 *  @brief This file contains the bleapputil_api for use
 *  with the Bluetooth Low Energy Protocol Stack.
 */


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
#include <ti/bleapp/ble_app_util/inc/bleapputil_api_legacy.h>
/*********************************************************************
 * MACROS
 */

/// @cond NODOC
// Size of string-converted device address ("0xXXXXXXXXXXXX")
#define BLEAPPUTIL_ADDR_STR_SIZE     15
/// @endcond // NODOC

/*********************************************************************
 * TYPEDEFS
 */

/**
 *  @defgroup BLEAppUtil_Functions_Typedefs BLEAppUtil Functions Typedefs
 *  @brief This module implements BLEAppUtil Funcctions Typedefs
 *  @{
 */

// no description for ICall_Hdr in the api, so the user won't be able to see ICall_Hdr struct members inside the api.
// the definition of the struct is in: icall_ble_apimsg.h
typedef ICall_Hdr               BLEAppUtil_msgHdr_t;

/// @cond NODOC
// need to be internal
typedef ICall_EntityID          BLEAppUtil_entityId_t;
/// @endcond //NODOC

/// Device Address
typedef uint8_t                 BLEAppUtil_BDaddr[B_ADDR_LEN];

// Handlers types

/**
 * @brief   Event handler which used in @ref BLEAppUtil_EventHandler_t.
 *
 * @param   event - message event.
 * @param   pMsgData - pointer to message data.
 *
 * @return  none
 */
typedef void (*EventHandler_t)(uint32 event, BLEAppUtil_msgHdr_t *pMsgData);

/// @cond NODOC
// not being used for now
typedef void (*ErrorHandler_t)(int32 errorCode , void* pInfo);
/// @endcond //NODOC

/**
 * @brief   A callback which will be called when Device Initialization Done event is raised.
 *          Should be sent to @ref BLEAppUtil_init.
 *
 * @param deviceInitDoneData - Init Done data which shall be used by the callback. the data is of the format: @ref gapDeviceInitDoneEvent_t.
 *
 * @return  none
 */
typedef void (*StackInitDone_t)(gapDeviceInitDoneEvent_t *deviceInitDoneData);

/**
 * @brief   Profile/Service Invoke from BLE App Util context type.
 *          This callback is used by @ref BLEAppUtil_invokeFunction and @ref BLEAppUtil_invokeFunctionNoData
 *
 * @param pData - The data to provide the callback
 *
 * @return  none
 */
typedef void (*InvokeFromBLEAppUtilContext_t)(char *pData);

/** @} End BLEAppUtil_Functions_Typedefs */

/*********************************************************************
 * Enumerators
 */

/**
 *  @defgroup BLEAppUtil_Enumerators_Typedefs BLEAppUtil Enumerators Typedefs
 *  @brief This module implements BLEAppUtil Enumerators Typedefs
 *  @{
 */

/// Event Handler Types which can be registered to BLEAppUtil, used by @ref BLEAppUtil_EventHandler_t
typedef enum BLEAppUtil_eventHandlerType_e
{
    BLEAPPUTIL_GAP_CONN_TYPE,           //!< GAP Connection type
    BLEAPPUTIL_CONN_NOTI_TYPE,          //!< Connection Notification Ttype
    BLEAPPUTIL_GAP_ADV_TYPE,            //!< GAP Advertise type
    BLEAPPUTIL_GAP_SCAN_TYPE,           //!< GAP Scan type
    BLEAPPUTIL_GAP_PERIODIC_TYPE,       //!< GAP Periodic type
    BLEAPPUTIL_GATT_TYPE,               //!< GATT type
    BLEAPPUTIL_PASSCODE_TYPE,           //!< Passcode type
    BLEAPPUTIL_PAIR_STATE_TYPE,         //!< Pairing type
    BLEAPPUTIL_L2CAP_DATA_TYPE,         //!< L2CAP Data type
    BLEAPPUTIL_L2CAP_SIGNAL_TYPE,       //!< L2CAP Signal type
    BLEAPPUTIL_HCI_DATA_TYPE,           //!< HCI Data type
    BLEAPPUTIL_HCI_GAP_TYPE,            //!< HCI GAP type
    BLEAPPUTIL_HCI_SMP_TYPE,            //!< HCI SMP type
    BLEAPPUTIL_HCI_SMP_META_TYPE,       //!< HCI SMP Meta type
    BLEAPPUTIL_HCI_CTRL_TO_HOST_TYPE    //!< HCI Controller To Host type
} BLEAppUtil_eventHandlerType_e;

/// GAP Conn event mask
typedef enum BLEAppUtil_GAPConnEventMaskFlags_e
{
    BLEAPPUTIL_LINK_ESTABLISHED_EVENT               = (uint32_t)BV(0),   //!< Link established event
    BLEAPPUTIL_LINK_TERMINATED_EVENT                = (uint32_t)BV(1),   //!< Link terminated event
    BLEAPPUTIL_CONNECTING_CANCELLED_EVENT           = (uint32_t)BV(2),   //!< Connecting cancelled event
    BLEAPPUTIL_LINK_PARAM_UPDATE_EVENT              = (uint32_t)BV(3),   //!< Link parameters update event
    BLEAPPUTIL_LINK_PARAM_UPDATE_REQ_EVENT          = (uint32_t)BV(4),   //!< Link parameters update request event
    BLEAPPUTIL_LINK_PARAM_UPDATE_REJECT_EVENT       = (uint32_t)BV(5),   //!< Link parameters update reject event
    BLEAPPUTIL_SIGNATURE_UPDATED_EVENT              = (uint32_t)BV(6),   //!< Signature updated event
    BLEAPPUTIL_AUTHENTICATION_COMPLETE_EVENT        = (uint32_t)BV(7),   //!< Authentication complete event
    BLEAPPUTIL_PASSKEY_NEEDED_EVENT                 = (uint32_t)BV(8),   //!< Passkey needed event
    BLEAPPUTIL_PERIPHERAL_REQUESTED_SECURITY_EVENT  = (uint32_t)BV(9),   //!< Peripheral requested security event
    BLEAPPUTIL_BOND_COMPLETE_EVENT                  = (uint32_t)BV(10),  //!< Bond complete event
    BLEAPPUTIL_PAIRING_REQ_EVENT                    = (uint32_t)BV(11),  //!< Pairing request event
    BLEAPPUTIL_AUTHENTICATION_FAILURE_EVT           = (uint32_t)BV(12),  //!< Authentication failure event
    BLEAPPUTIL_BOND_LOST_EVENT                      = (uint32_t)BV(13)   //!< Bond lost event
} BLEAppUtil_GAPConnEventMaskFlags_e;

/// Connection event event mask
typedef enum BLEAppUtil_ConnEventNotiEventMaskFlags_e
{
    BLEAPPUTIL_CONN_NOTI_EVENT_INVALID             = (uint32_t)BV(0),  //!< @ref GAP_CB_EVENT_INVALID
    BLEAPPUTIL_CONN_NOTI_CONN_ESTABLISHED          = (uint32_t)BV(1),  //!< @ref GAP_CB_CONN_ESTABLISHED
    BLEAPPUTIL_CONN_NOTI_PHY_UPDATE                = (uint32_t)BV(2),  //!< @ref GAP_CB_PHY_UPDATE
    BLEAPPUTIL_CONN_NOTI_CONN_EVENT_ALL            = (uint32_t)BV(3),  //!< @ref GAP_CB_CONN_EVENT_ALL
} BLEAppUtil_ConnEventNotiEventMaskFlags_e;

/// GAP ADV event mask
typedef enum BLEAppUtil_GAPAdvEventMaskFlags_e
{
    BLEAPPUTIL_ADV_START_AFTER_ENABLE    = GAP_EVT_ADV_START_AFTER_ENABLE,  //!< Gets advertising handle using pBuf. Sent on the first advertisement after a @ref BLEAppUtil_advStart.
    BLEAPPUTIL_ADV_END_AFTER_DISABLE     = GAP_EVT_ADV_END_AFTER_DISABLE,   //!< Gets advertising handle using pBuf. Sent after advertising stops due to a @ref BLEAppUtil_advStop.
    BLEAPPUTIL_ADV_START                 = GAP_EVT_ADV_START,               //!< Gets advertising handle using pBuf. Sent at the beginning of each advertisement (for legacy advertising) or at the beginning of each each advertisement set (for extended advertising).
    BLEAPPUTIL_ADV_END                   = GAP_EVT_ADV_END,                 //!< Gets advertising handle using pBuf. Sent after each advertisement (for legacy advertising) or at the end of each each advertisement set (for extended advertising).
    BLEAPPUTIL_ADV_SET_TERMINATED        = GAP_EVT_ADV_SET_TERMINATED,      //!< Gets @ref GapAdv_setTerm_t using pBuf. pBuf should be cast to @ref GapAdv_setTerm_t. Sent when an advertisement set is terminated due to a connection establishment
    BLEAPPUTIL_SCAN_REQ_RECEIVED         = GAP_EVT_SCAN_REQ_RECEIVED,       //!< Gets @ref GapAdv_scanReqReceived_t using pBuf. pBuf should be cast to @ref GapAdv_scanReqReceived_t. Sent when a scan request is received.
    BLEAPPUTIL_ADV_DATA_TRUNCATED        = GAP_EVT_ADV_DATA_TRUNCATED,      //!< Gets @ref GapAdv_truncData_t using pBuf. pBuf should be cast to @ref GapAdv_truncData_t. Sent when the advertising data is truncated due to the limited advertisement data length for connectable advertisements.
    BLEAPPUTIL_ADV_INSUFFICIENT_MEMORY   = GAP_EVT_INSUFFICIENT_MEMORY      //!< A memory failure has occurred.
} BLEAppUtil_GAPAdvEventMaskFlags_e;


/// GAP Scan event mask
typedef enum BLEAppUtil_GAPScanEventMaskFlags_e
{
    BLEAPPUTIL_SCAN_ENABLED              = GAP_EVT_SCAN_ENABLED,        //!< @ref GAP_EVT_SCAN_ENABLED
    BLEAPPUTIL_SCAN_DISABLED             = GAP_EVT_SCAN_DISABLED,       //!< @ref GAP_EVT_SCAN_DISABLED
    BLEAPPUTIL_SCAN_PRD_ENDED            = GAP_EVT_SCAN_PRD_ENDED,      //!< @ref GAP_EVT_SCAN_PRD_ENDED
    BLEAPPUTIL_SCAN_DUR_ENDED            = GAP_EVT_SCAN_DUR_ENDED,      //!< @ref GAP_EVT_SCAN_DUR_ENDED
    BLEAPPUTIL_SCAN_INT_ENDED            = GAP_EVT_SCAN_INT_ENDED,      //!< @ref GAP_EVT_SCAN_INT_ENDED
    BLEAPPUTIL_SCAN_WND_ENDED            = GAP_EVT_SCAN_WND_ENDED,      //!< @ref GAP_EVT_SCAN_WND_ENDED
    BLEAPPUTIL_ADV_REPORT                = GAP_EVT_ADV_REPORT,          //!< @ref GAP_EVT_ADV_REPORT
    BLEAPPUTIL_ADV_REPORT_FULL           = GAP_EVT_ADV_REPORT_FULL,     //!< @ref GAP_EVT_ADV_REPORT_FULL
    BLEAPPUTIL_SCAN_INSUFFICIENT_MEMORY  = GAP_EVT_INSUFFICIENT_MEMORY  //!< @ref GAP_EVT_INSUFFICIENT_MEMORY
} BLEAppUtil_GAPScanEventMaskFlags_e;

/// GAP Periodic event mask
typedef enum BLEAppUtil_GAPPeriodicEventMaskFlags_e
{
	BLEAPPUTIL_ADV_SET_PERIODIC_ADV_PARAMS_EVENT = (uint32_t)BV(0),	 //!< @ref  GAP_ADV_SET_PERIODIC_ADV_PARAMS_EVENT
	BLEAPPUTIL_ADV_SET_PERIODIC_ADV_DATA_EVENT   = (uint32_t)BV(1),	 //!< @ref  GAP_ADV_SET_PERIODIC_ADV_DATA_EVENT
	BLEAPPUTIL_ADV_SET_PERIODIC_ADV_ENABLE_EVENT = (uint32_t)BV(2),	 //!< @ref  GAP_ADV_SET_PERIODIC_ADV_ENABLE_EVENT
	BLEAPPUTIL_SCAN_CREATE_SYNC_EVENT            = (uint32_t)BV(3),	 //!< @ref  GAP_SCAN_CREATE_SYNC_EVENT
	BLEAPPUTIL_SCAN_SYNC_CANCEL_EVENT            = (uint32_t)BV(4),	 //!< @ref  GAP_SCAN_SYNC_CANCEL_EVENT
	BLEAPPUTIL_SCAN_TERMINATE_SYNC_EVENT         = (uint32_t)BV(5),	 //!< @ref  GAP_SCAN_TERMINATE_SYNC_EVENT
	BLEAPPUTIL_SCAN_PERIODIC_RECEIVE_EVENT       = (uint32_t)BV(6),	 //!< @ref  GAP_SCAN_PERIODIC_RECEIVE_EVENT
	BLEAPPUTIL_SCAN_ADD_DEVICE_ADV_LIST_EVENT    = (uint32_t)BV(7),	 //!< @ref  GAP_SCAN_ADD_DEVICE_ADV_LIST_EVENT
	BLEAPPUTIL_SCAN_REMOVE_DEVICE_ADV_LIST_EVENT = (uint32_t)BV(8),	 //!< @ref  GAP_SCAN_REMOVE_DEVICE_ADV_LIST_EVENT
	BLEAPPUTIL_SCAN_READ_ADV_LIST_SIZE_EVENT     = (uint32_t)BV(9),	 //!< @ref  GAP_SCAN_READ_ADV_LIST_SIZE_EVENT
	BLEAPPUTIL_SCAN_CLEAR_ADV_LIST_EVENT         = (uint32_t)BV(10), //!< @ref  GAP_SCAN_CLEAR_ADV_LIST_EVENT
	BLEAPPUTIL_SCAN_PERIODIC_ADV_SYNC_EST_EVENT  = (uint32_t)BV(11), //!< @ref  GAP_SCAN_PERIODIC_ADV_SYNC_EST_EVENT
	BLEAPPUTIL_SCAN_PERIODIC_ADV_SYNC_LOST_EVENT = (uint32_t)BV(12), //!< @ref  GAP_SCAN_PERIODIC_ADV_SYNC_LOST_EVENT
	BLEAPPUTIL_SCAN_PERIODIC_ADV_REPORT_EVENT    = (uint32_t)BV(13)	 //!< @ref  GAP_SCAN_PERIODIC_ADV_REPORT_EVENT
} BLEAppUtil_GAPPeriodicEventMaskFlags_e;

/// GAP Pairing event mask
typedef enum BLEAppUtil_PairingEventMaskFlags_e
{
	BLEAPPUTIL_PAIRING_STATE_STARTED     = (uint32_t)BV(0),	//!< @ref  GAPBOND_PAIRING_STATE_STARTED
	BLEAPPUTIL_PAIRING_STATE_COMPLETE    = (uint32_t)BV(1),	//!< @ref  GAPBOND_PAIRING_STATE_COMPLETE
	BLEAPPUTIL_PAIRING_STATE_ENCRYPTED   = (uint32_t)BV(2),	//!< @ref  GAPBOND_PAIRING_STATE_ENCRYPTED
	BLEAPPUTIL_PAIRING_STATE_BOND_SAVED  = (uint32_t)BV(3),	//!< @ref  GAPBOND_PAIRING_STATE_BOND_SAVED
	BLEAPPUTIL_PAIRING_STATE_CAR_READ    = (uint32_t)BV(4),	//!< @ref  GAPBOND_PAIRING_STATE_CAR_READ
	BLEAPPUTIL_PAIRING_STATE_RPAO_READ   = (uint32_t)BV(5),	//!< @ref  GAPBOND_PAIRING_STATE_RPAO_READ
	BLEAPPUTIL_GENERATE_ECC_DONE         = (uint32_t)BV(6)	//!< @ref  GAPBOND_GENERATE_ECC_DONE
} BLEAppUtil_PairingEventMaskFlags_e;

/// GATT event mask
typedef enum BLEAppUtil_GATTEventMaskFlags_e
{
	BLEAPPUTIL_ATT_ERROR_RSP                   = (uint32_t)BV(0),	//!< @ref  ATT_ERROR_RSP
	BLEAPPUTIL_ATT_EXCHANGE_MTU_REQ            = (uint32_t)BV(1),	//!< @ref  ATT_EXCHANGE_MTU_REQ
	BLEAPPUTIL_ATT_EXCHANGE_MTU_RSP            = (uint32_t)BV(2),	//!< @ref  ATT_EXCHANGE_MTU_RSP
	BLEAPPUTIL_ATT_FIND_INFO_REQ               = (uint32_t)BV(3),	//!< @ref  ATT_FIND_INFO_REQ
	BLEAPPUTIL_ATT_FIND_INFO_RSP               = (uint32_t)BV(4),	//!< @ref  ATT_FIND_INFO_RSP
	BLEAPPUTIL_ATT_FIND_BY_TYPE_VALUE_REQ      = (uint32_t)BV(5),	//!< @ref  ATT_FIND_BY_TYPE_VALUE_REQ
	BLEAPPUTIL_ATT_FIND_BY_TYPE_VALUE_RSP      = (uint32_t)BV(6),	//!< @ref  ATT_FIND_BY_TYPE_VALUE_RSP
	BLEAPPUTIL_ATT_READ_BY_TYPE_REQ            = (uint32_t)BV(7),	//!< @ref  ATT_READ_BY_TYPE_REQ
	BLEAPPUTIL_ATT_READ_BY_TYPE_RSP            = (uint32_t)BV(8),	//!< @ref  ATT_READ_BY_TYPE_RSP
	BLEAPPUTIL_ATT_READ_REQ                    = (uint32_t)BV(9),	//!< @ref  ATT_READ_REQ
	BLEAPPUTIL_ATT_READ_RSP                    = (uint32_t)BV(10),	//!< @ref  ATT_READ_RSP
	BLEAPPUTIL_ATT_READ_BLOB_REQ               = (uint32_t)BV(11),	//!< @ref  ATT_READ_BLOB_REQ
	BLEAPPUTIL_ATT_READ_BLOB_RSP               = (uint32_t)BV(12),	//!< @ref  ATT_READ_BLOB_RSP
	BLEAPPUTIL_ATT_READ_MULTI_REQ              = (uint32_t)BV(13),	//!< @ref  ATT_READ_MULTI_REQ
	BLEAPPUTIL_ATT_READ_MULTI_RSP              = (uint32_t)BV(14),	//!< @ref  ATT_READ_MULTI_RSP
	BLEAPPUTIL_ATT_READ_BY_GRP_TYPE_REQ        = (uint32_t)BV(15),	//!< @ref  ATT_READ_BY_GRP_TYPE_REQ
	BLEAPPUTIL_ATT_READ_BY_GRP_TYPE_RSP        = (uint32_t)BV(16),	//!< @ref  ATT_READ_BY_GRP_TYPE_RSP
	BLEAPPUTIL_ATT_WRITE_REQ                   = (uint32_t)BV(17),	//!< @ref  ATT_WRITE_REQ
	BLEAPPUTIL_ATT_WRITE_RSP                   = (uint32_t)BV(18),	//!< @ref  ATT_WRITE_RSP
	BLEAPPUTIL_ATT_PREPARE_WRITE_REQ           = (uint32_t)BV(19),	//!< @ref  ATT_PREPARE_WRITE_REQ
	BLEAPPUTIL_ATT_PREPARE_WRITE_RSP           = (uint32_t)BV(20),	//!< @ref  ATT_PREPARE_WRITE_RSP
	BLEAPPUTIL_ATT_EXECUTE_WRITE_REQ           = (uint32_t)BV(21),	//!< @ref  ATT_EXECUTE_WRITE_REQ
	BLEAPPUTIL_ATT_EXECUTE_WRITE_RSP           = (uint32_t)BV(22),	//!< @ref  ATT_EXECUTE_WRITE_RSP
	BLEAPPUTIL_ATT_HANDLE_VALUE_NOTI           = (uint32_t)BV(23),	//!< @ref  ATT_HANDLE_VALUE_NOTI
	BLEAPPUTIL_ATT_HANDLE_VALUE_IND            = (uint32_t)BV(24),	//!< @ref  ATT_HANDLE_VALUE_IND
	BLEAPPUTIL_ATT_HANDLE_VALUE_CFM            = (uint32_t)BV(25),	//!< @ref  ATT_HANDLE_VALUE_CFM
	BLEAPPUTIL_ATT_UNSUPPORTED_METHOD          = (uint32_t)BV(26),	//!< @ref  ATT_UNSUPPORTED_METHOD
	BLEAPPUTIL_ATT_WRITE_CMD                   = (uint32_t)BV(27),	//!< @ref  ATT_WRITE_CMD
	BLEAPPUTIL_ATT_SIGNED_WRITE_CMD            = (uint32_t)BV(28),	//!< @ref  ATT_SIGNED_WRITE_CMD
	BLEAPPUTIL_ATT_TRANSACTION_READY_EVENT     = (uint32_t)BV(29),	//!< @ref  ATT_TRANSACTION_READY_EVENT
	BLEAPPUTIL_ATT_FLOW_CTRL_VIOLATED_EVENT    = (uint32_t)BV(30),	//!< @ref  ATT_FLOW_CTRL_VIOLATED_EVENT
	BLEAPPUTIL_ATT_MTU_UPDATED_EVENT           = (uint32_t)BV(31),	//!< @ref  ATT_MTU_UPDATED_EVENT
} BLEAppUtil_GATTEventMaskFlags_e;

/// L2CAP Signal event mask
typedef enum BLEAppUtil_L2CAPSignalEventMaskFlags_e
{
    BLEAPPUTIL_L2CAP_CHANNEL_ESTABLISHED_EVT   = (uint32_t)BV(0), //!< @ref L2CAP_CHANNEL_ESTABLISHED_EVT
    BLEAPPUTIL_L2CAP_CHANNEL_TERMINATED_EVT    = (uint32_t)BV(1), //!< @ref L2CAP_CHANNEL_TERMINATED_EVT
    BLEAPPUTIL_L2CAP_OUT_OF_CREDIT_EVT         = (uint32_t)BV(2), //!< @ref L2CAP_OUT_OF_CREDIT_EVT
    BLEAPPUTIL_L2CAP_PEER_CREDIT_THRESHOLD_EVT = (uint32_t)BV(3), //!< @ref L2CAP_PEER_CREDIT_THRESHOLD_EVT
    BLEAPPUTIL_L2CAP_SEND_SDU_DONE_EVT         = (uint32_t)BV(4), //!< @ref L2CAP_SEND_SDU_DONE_EVT
    BLEAPPUTIL_L2CAP_NUM_CTRL_DATA_PKT_EVT     = (uint32_t)BV(5)  //!< @ref L2CAP_NUM_CTRL_DATA_PKT_EVT
} BLEAppUtil_L2CAPSignalEventMaskFlags_e;

/// HCI event mask
typedef enum BLEAppUtil_HciEventMaskFlags_e
{
    BLEAPPUTIL_HCI_DISCONNECTION_COMPLETE_EVENT_CODE    = (uint32_t)BV(0), //!< @ref HCI_DISCONNECTION_COMPLETE_EVENT_CODE
    BLEAPPUTIL_HCI_COMMAND_COMPLETE_EVENT_CODE          = (uint32_t)BV(1), //!< @ref HCI_COMMAND_COMPLETE_EVENT_CODE
    BLEAPPUTIL_HCI_COMMAND_STATUS_EVENT_CODE            = (uint32_t)BV(2), //!< @ref HCI_COMMAND_STATUS_EVENT_CODE
    BLEAPPUTIL_HCI_LE_EVENT_CODE                        = (uint32_t)BV(3), //!< @ref HCI_LE_EVENT_CODE
    BLEAPPUTIL_HCI_VE_EVENT_CODE                        = (uint32_t)BV(4), //!< @ref HCI_VE_EVENT_CODE
    BLEAPPUTIL_HCI_BLE_HARDWARE_ERROR_EVENT_CODE        = (uint32_t)BV(5), //!< @ref HCI_BLE_HARDWARE_ERROR_EVENT_CODE
    BLEAPPUTIL_HCI_NUM_OF_COMPLETED_PACKETS_EVENT_CODE  = (uint32_t)BV(6), //!< @ref HCI_NUM_OF_COMPLETED_PACKETS_EVENT_CODE
    BLEAPPUTIL_HCI_APTO_EXPIRED_EVENT_CODE              = (uint32_t)BV(7), //!< @ref HCI_APTO_EXPIRED_EVENT_CODE
    BLEAPPUTIL_HCI_EVENT_PACKET                         = (uint32_t)BV(8), //!< Event Packet
    BLEAPPUTIL_HCI_ACL_DATA_PACKET                      = (uint32_t)BV(9)  //!< ACL Data Packet
} BLEAppUtil_HciEventMaskFlags_e;

/// Profile role mask
typedef enum BLEAppUtil_Profile_Roles_e
{
    BLEAPPUTIL_BROADCASTER_ROLE = GAP_PROFILE_BROADCASTER,  //!< @ref GAP_PROFILE_BROADCASTER
    BLEAPPUTIL_OBSERVER_ROLE    = GAP_PROFILE_OBSERVER,     //!< @ref GAP_PROFILE_OBSERVER
    BLEAPPUTIL_PERIPHERAL_ROLE  = GAP_PROFILE_PERIPHERAL,   //!< @ref GAP_PROFILE_PERIPHERAL
    BLEAPPUTIL_CENTRAL_ROLE     = GAP_PROFILE_CENTRAL       //!< @ref GAP_PROFILE_CENTRAL
} BLEAppUtil_Profile_Roles_e;

/** @} End BLEAppUtil_Enumerators_Typedefs */

/*********************************************************************
 * Structures
 */

/**
 *  @defgroup BLEAppUtil_Structures BLEAppUtil Structures
 *  @brief This module implements BLEAppUtil structures
 *  @{
 */

/**
 * @brief BLEAppUtil Adv Event Data Structure
 *
 * Used to store the data of adv event
 */
typedef struct
{
    uint32_t        event;  //!< see @ref BLEAppUtil_GAPAdvEventMaskFlags_e
    GapAdv_data_t   *pBuf;  //!< data potentially accompanying event
    uint32_t        *arg;   //!< custom application argument that can be return through this callback
} BLEAppUtil_AdvEventData_t;

/**
 * @brief BLEAppUtil Scan Event Data Structure
 *
 * Used to store the data of scan event
 */
typedef struct
{
    uint32_t        event; //!< see @ref BLEAppUtil_GAPScanEventMaskFlags_e
    GapScan_data_t  *pBuf; //!< data potentially accompanying event
    uint32_t        *arg;  //!< custom application argument that can be return through this callback
} BLEAppUtil_ScanEventData_t;

/**
 * @brief BLEAppUtil PairState Event Data Structure
 */
typedef struct
{
    uint16_t connHandle;        //!< connection handle of current pairing process
    uint32_t state;             //!< @ref BLEAppUtil_PairingEventMaskFlags_e
    uint8_t  status;            //!< pairing status
} BLEAppUtil_PairStateData_t;

/**
 * @brief BLEAppUtil Passcode Data Structure
 *
 * Used to store the passcode data
 */
typedef struct
{
    BLEAppUtil_BDaddr   deviceAddr;     //!< Pointer to 6-byte device address which the current pairing
    uint16_t            connHandle;     //!< Connection handle of the current pairing process
    uint8_t             uiInputs;       //!< if TRUE, the local device should accept a passcode input.
    uint8_t             uiOutputs;      //!< if TRUE, the local device should display the passcode.

    /**
     * If this is a nonzero value, then it is the code that
     * should be displayed for numeric comparison pairing. If this is zero,
     * then passkey entry pairing is occurring.
     */
    uint32_t            numComparison;
} BLEAppUtil_PasscodeData_t;

/**
 * @brief BLEAppUtil Adv Init Structure
 *
 * Should be created by the application and initialize
 * by calling @ref BLEAppUtil_initAdvSet.
 */
typedef struct
{
    uint16_t        advDataLen;         //!< length (in bytes) of advData
    uint8_t         *advData;           //!< pointer to array containing the advertise data
    uint16_t        scanRespDataLen;    //!< length (in bytes) of scanRespData
    uint8_t         *scanRespData;      //!< pointer to array containing the scan response data
    GapAdv_params_t *advParam;          //!< pointer to structure of adversing parameters
} BLEAppUtil_AdvInit_t;

/**
 * @brief BLEAppUtil Adv Start Structure
 *
 * Should be created by the application and passed to
 * @ref BLEAppUtil_advStart.
 */
typedef struct
{
    GapAdv_enableOptions_t  enableOptions;  //!< whether to advertise for the max possible time, for a user-specified duration, or for a user-specified number of advertising events

    /**
     * If enableOptions is set to @ref GAP_ADV_ENABLE_OPTIONS_USE_DURATION, this is the time (in 10 ms ticks) to advertise before stopping where the range is 10 ms - 655,540 ms
     * If enableOptions is set to @ref GAP_ADV_ENABLE_OPTIONS_USE_MAX_EVENTS, this is the maximum number of advertisements to send before stopping, where the range is 1-256
     * If enableOptions is set to @ref GAP_ADV_ENABLE_OPTIONS_USE_MAX, this parameter is not used
     * If enableOptions is set to @ref GAP_ADV_ENABLE_OPTIONS_USE_DURATION, this is the time (in 10 ms ticks) to advertise before stopping where the range is 10 ms - 655,540 ms
     */
    uint16_t                durationOrMaxEvents;
} BLEAppUtil_AdvStart_t;

/**
 * @brief BLEAppUtil Scan Init Structure
 *
 * Should be created by the application and initialized
 * by calling @ref BLEAppUtil_scanInit.
 */
typedef struct
{
    uint8_t                 primPhy;            //!< Primary PHY(s). Individual PHY values can be OR'ed.
    GapScan_ScanType_t      scanType;           //!< Scanning type. Passive or Active
    uint16_t                scanInterval;       //!< Scanning interval
    uint16_t                scanWindow;         //!< Scanning window
    uint16_t                advReportFields;    //!< Bitmap of adv report fields to keep in the advertising report list
    uint8_t                 scanPhys;           //!< PHYs on which Advs should be received on the primary Adv channel
    uint8_t                 fltPolicy;          //!< Scanning filter policy regarding White List and Adv type
    uint16_t                fltPduType;         //!< Filter by type of PDU
    int8_t                  fltMinRssi;         //!< Filter by RSSI. Only packets received with the specified RSSI or above will be reported.
    uint8_t                 fltDiscMode;        //!< Filter by Discovery Mode.
    uint8_t                 fltDup;             //!< Filter out duplicated packets, determining by address and DID.
} BLEAppUtil_ScanInit_t;

/**
 * @brief BLEAppUtil Scan Start Structure
 *
 * Should be created by the application and passed to
 * @ref BLEAppUtil_scanStart.
 */
typedef struct
{
    /** Scan period. Ignored if duration is zero. 1.28 sec unit.
     *  Range: 0x00-0xffff, where 0x00 is continuously scanning.
     */
    uint16_t scanPeriod;
    /**
     * Scan duration. 10 ms unit. The time of duration shall be
     * greater than the time of scan interval set by @ref BLEAppUtil_scanInit.
     * Range: 0x00-0xffff, where 0x00 is continuously scanning.
     */
    uint16_t scanDuration;
    /**
     * If non-zero, the list of advertising reports (the number
     * of which is up to maxNumReport) will be generated and come with
     * @ref BLEAPPUTIL_SCAN_DISABLED.
     */
    uint8_t  maxNumReport;
}BLEAppUtil_ScanStart_t;

/**
 * @brief BLEAppUtil Conn Params Structure.
 *        Should be created by the application and passed to @ref BLEAppUtil_setConnParams
 */
typedef struct
{
    uint8_t  initPhys;          //!< connection channel PHY(s). individual PHY values can be OR'ed.
    uint16_t scanInterval;      //!< see @ref INIT_PHYPARAM_SCAN_INTERVAL
    uint16_t scanWindow;        //!< see @ref INIT_PHYPARAM_SCAN_WINDOW
    uint16_t minConnInterval;   //!< see @ref INIT_PHYPARAM_CONN_INT_MIN
    uint16_t maxConnInterval;   //!< see @ref INIT_PHYPARAM_CONN_INT_MAX
    uint16_t connLatency;       //!< see @ref INIT_PHYPARAM_CONN_LATENCY
    uint16_t supTimeout;        //!< see @ref INIT_PHYPARAM_SUP_TIMEOUT
} BLEAppUtil_ConnParams_t;

/**
 * @brief BLEAppUtil Connect Params Structure.
 *        Should be created by the application and passed to @ref BLEAppUtil_Connect
 */
typedef struct
{
    uint8_t                 peerAddrType;   //!< peer device's address type
    BLEAppUtil_BDaddr       pPeerAddress;   //!< peer device's address
    uint8_t                 phys;           //!< PHY(s) to try making connection on. individual PHY values can be OR'ed
    /**
     * if there is no chance to initiate a connection
     * within timeout ms, this connect request will be cancelled
     * automatically. if timeout is 0, the initiator will keep
     * trying to get a chance to make a connection until
     * @ref BLEAppUtil_cancelConnect is called.
     */
    uint16_t                timeout;
}BLEAppUtil_ConnectParams_t;

/**
 * @brief Phy Preference on the current connection.
 *        Should be created by the application and passed to @ref BLEAppUtil_setConnPhy
 */
typedef struct
{
    uint16 connHandle;      //!< Connection handle
    uint8  allPhys;         //!< Host preference on how to handle txPhy and rxPhy
    uint8  txPhy;           //!< Bit field of Host preferred Tx PHY
    uint8  rxPhy;           //!< Bit field of Host preferred Rx PHY
    uint16 phyOpts;         //!< Bit field of Host preferred PHY options
}BLEAppUtil_ConnPhyParams_t;

/**
 * @brief BLEAppUtil General Params Structure
 *
 * Should be created by the application and passed to
 * @ref BLEAppUtil_init.
 */
typedef struct
{
    // Task related parameters - BLEAppUtil task priority. Higher values represent higher priorities
    int                         taskPriority;       //!< Task priority
    size_t                      taskStackSize;      //!< Task stack size

    // BLE stack related parameters
    BLEAppUtil_Profile_Roles_e  profileRole;        //!< Profile role. roles are defined at @ref BLEAppUtil_Profile_Roles_e

    // GAP GATT Server (GGS) Parameters
    uint8_t                     *deviceNameAtt;     //!< Device name attribute
    uint16_t                    appearanceAtt;      //!< Appearance attribute
    gapPeriConnectParams_t      periConnParamsAtt;  //!< Peripheral Preferred Connection Parameters

    // Device related parameters
    /**
     * Own address mode.
     * If always using Resolvable Private Address, set this to either @ref ADDRMODE_RP_WITH_PUBLIC_ID or @ref ADDRMODE_RP_WITH_RANDOM_ID.
     * If always using Identity Address, set this to either @ref ADDRMODE_PUBLIC or @ref ADDRMODE_RANDOM.
    */
    GAP_Addr_Modes_t            addressMode;
    /**
     * Pointer to 6-byte Random Static Address of this device that will be copied to the stack.
     * Valid only if addrMode is @ref ADDRMODE_RANDOM or @ref ADDRMODE_RP_WITH_RANDOM_ID and can not be NULL in these cases.
     * Ignored for other address types.
    */
    uint8_t                     *pDeviceRandomAddress;
} BLEAppUtil_GeneralParams_t;

/**
 * @brief BLEAppUtil Peripheral Central Params Structure
 *
 * Should be created by the application and passed to
 * @ref BLEAppUtil_init.
 */
typedef struct
{
    gapBondParams_t             *gapBondParams;             //!< GAP bond manager parameters structre
    Gap_updateDecision_t        connParamUpdateDecision;    //!< The param update configuration
} BLEAppUtil_PeriCentParams_t;

/// @cond NODOC
// not used for now
/**
 *  @brief GAP Connection Data
 */
typedef union
{
    gapEstLinkReqEvent_t            estLinkReqEvent;            //!< see @ref gapEstLinkReqEvent_t.             BLEAPPUTIL_LINK_ESTABLISHED_EVENT
    gapTerminateLinkEvent_t         termLinkEvent;              //!< see @ref gapTerminateLinkEvent_t.          BLEAPPUTIL_LINK_TERMINATED_EVENT
    gapConnCancelledEvent_t         gapConnCancelledEvent;      //!< see @ref gapConnCancelledEvent_t.          BLEAPPUTIL_CONNECTING_CANCELLED_EVENT
    gapLinkUpdateEvent_t            linkUpdateEvent;            //!< see @ref gapLinkUpdateEvent_t.             BLEAPPUTIL_LINK_PARAM_UPDATE_EVENT and BLEAPPUTIL_LINK_PARAM_UPDATE_REJECT_EVENT
    gapUpdateLinkParamReqEvent_t    paramUpdateReqEvent;        //!< see @ref gapUpdateLinkParamReqEvent_t.     BLEAPPUTIL_LINK_PARAM_UPDATE_REQ_EVENT
    gapSignUpdateEvent_t            signUpdateEvent;            //!< see @ref gapSignUpdateEvent_t.             BLEAPPUTIL_SIGNATURE_UPDATED_EVENT
    gapAuthCompleteEvent_t          authCompleteEvent;          //!< see @ref gapAuthCompleteEvent_t.           BLEAPPUTIL_AUTHENTICATION_COMPLETE_EVENT
    gapPasskeyNeededEvent_t         passkeyIsNeededEvent;       //!< see @ref gapPasskeyNeededEvent_t.          BLEAPPUTIL_PASSKEY_NEEDED_EVENT
    gapPeripheralSecurityReqEvent_t peripheralSecurityReqEvent; //!< see @ref gapPeripheralSecurityReqEvent_t.  BLEAPPUTIL_PERIPHERAL_REQUESTED_SECURITY_EVENT
    gapBondCompleteEvent_t          bondCompleteEvent;          //!< see @ref gapBondCompleteEvent_t.           BLEAPPUTIL_BOND_COMPLETE_EVENT
    gapPairingReqEvent_t            pairingReqEvent;            //!< see @ref gapPairingReqEvent_t.             BLEAPPUTIL_PAIRING_REQ_EVENT
    gapBondLostEvent_t              bondLostEvent;              //!< see @ref gapBondLostEvent_t.               BLEAPPUTIL_AUTHENTICATION_FAILURE_EVT - no data. BLEAPPUTIL_BOND_LOST_EVENT
    Gap_ConnEventRpt_t              connEventRpt;               //!< see @ref Gap_ConnEventRpt_t.               BLEAPPUTIL_GAP_CB_EVENT_INVALID , @ref BLEAPPUTIL_GAP_CB_CONN_ESTABLISHED and @ref BLEAPPUTIL_GAP_CB_PHY_UPDATE
    gapAuthParams_t                 authParams;                 //!< see @ref gapAuthParams_t.
    gapPairingReq_t                 pairingReq;                 //!< see @ref gapPairingReq_t.                  BLEAPPUTIL_PAIRING_REQ_EVENT
    gapUpdateLinkParamReq_t         paramUpdateReq;             //!< see @ref gapUpdateLinkParamReq_t.          BLEAPPUTIL_LINK_PARAM_UPDATE_REQ_EVENT
    gapUpdateLinkParamReqReply_t    paramUpdateReqReplay;       //!< see @ref gapUpdateLinkParamReqReply_t.
} BLEAppUtil_gapConnData_t;

/**
 *  @brief Message Data types
 */
typedef struct
{
    union
    {
        gapEventHdr_t               gapConnData;            //!< Msg Data of @ref BLEAPPUTIL_GAP_CONN_TYPE
        BLEAppUtil_AdvEventData_t   gapAdvData;             //!< Msg Data of @ref BLEAPPUTIL_GAP_ADV_TYPE
        BLEAppUtil_ScanEventData_t  gapScanData;            //!< Msg Data of @ref BLEAPPUTIL_GAP_SCAN_TYPE
        gapEventHdr_t               gapPeriodicData;        //!< Msg Data of @ref BLEAPPUTIL_GAP_PERIODIC_TYPE
        gattMsgEvent_t              gattData;               //!< Msg Data of @ref BLEAPPUTIL_GATT_TYPE
        BLEAppUtil_PasscodeData_t   passcodeData;           //!< Msg Data of @ref BLEAPPUTIL_PASSCODE_TYPE
        BLEAppUtil_PairStateData_t  pairStateData;          //!< Msg Data of @ref BLEAPPUTIL_PAIR_STATE_TYPE
        l2capDataEvent_t            l2capData;              //!< Msg Data of @ref BLEAPPUTIL_L2CAP_DATA_TYPE
        l2capSignalEvent_t          l2capSignalData;        //!< Msg Data of @ref BLEAPPUTIL_L2CAP_SIGNAL_TYPE
        hciDataEvent_t              hciData;                //!< Msg Data of @ref BLEAPPUTIL_HCI_DATA_TYPE
        ICall_HciExtEvt             hciGapData;             //!< Msg Data of @ref BLEAPPUTIL_HCI_GAP_TYPE
        ICall_HciExtEvt             hciSMPData;             //!< Msg Data of @ref BLEAPPUTIL_HCI_SMP_TYPE
        ICall_HciExtEvt             hciSMPMetaData;         //!< Msg Data of @ref BLEAPPUTIL_HCI_SMP_META_TYPE
    };
} BLEAppUtil_msgData_t;
/// @endcond // NODOC

/**
 * @brief BLEAppUtil Events handlers Structure
 *
 * This must be setup by the application and passed to the BLEAppUtil module
 * by calling @ref BLEAppUtil_registerEventHandler.
 */
typedef struct
{
    BLEAppUtil_eventHandlerType_e   handlerType;    //!< Handler type. see @ref BLEAppUtil_eventHandlerType_e
    EventHandler_t                  pEventHandler;  //!< The event handler
    uint32_t                        eventMask;      //!< Events mask
} BLEAppUtil_EventHandler_t;

/** @} End BLEAppUtil_Structures */

/*********************************************************************
 * GLOBAL VARIABLES
 */

/*********************************************************************
 * FUNCTIONS
 */

/**
 *  @defgroup BLEAppUtil_Functions BLEAppUtil Functions
 *  @brief This module implements BLEAppUtil functions
 *  @{
 */

/**
 * @brief   Initiate the application (queue, event, stack)
 *
 * @param   errorHandler       - Error handler to register
 * @param   initDoneHandler    - This handler will be called when the
 *                               device initialization is done
 * @param   initGeneralParams  - General parameters needed to initialize
 *                               the BLE stack
 * @param   initPeriCentParams - Connection related parameters needed
 *                               to initialize the BLE stack
 *
 * @return  None
 */
void BLEAppUtil_init(ErrorHandler_t errorHandler, StackInitDone_t initDoneHandler,
                     BLEAppUtil_GeneralParams_t *initGeneralParams,
                     BLEAppUtil_PeriCentParams_t *initPeriCentParams);

/**
 * @brief   Returns the self entity id needed when sending BLE stack
 *          commands
 *
 * @return  The entityId
 */
BLEAppUtil_entityId_t BLEAppUtil_getSelfEntity(void);

// Register function for BLE stack events

/**
 * @brief   Register application event handler
 *
 * @param   eventHandler - The handler to register
 *
 * @return  SUCCESS, FAILURE
 */
bStatus_t BLEAppUtil_registerEventHandler(BLEAppUtil_EventHandler_t *eventHandler);

/**
 * @brief   Un-register application event handler
 *
 * @param   eventHandler - The handler to un-register
 *
 * @return  SUCCESS, INVALIDPARAMETER
 */
bStatus_t BLEAppUtil_unRegisterEventHandler(BLEAppUtil_EventHandler_t *eventHandler);

// Profiles/Services APIs

/**
 * @brief   This function receives a callback and data and switches
 *          the context in order to call the callback from the BLE
 *          App Util module context
 *
 * @param   callback - The callback to invoke from the BLE App Util
 *                     module context
 *
 * @return  SUCCESS, FAILURE
 */
bStatus_t BLEAppUtil_invokeFunctionNoData(InvokeFromBLEAppUtilContext_t callback);

/**
 * @brief   This function receives a callback and data and switches
 *          the context in order to call the callback from the BLE
 *          App Util module context
 *
 * @param   callback - The callback to invoke from the BLE App Util
 *                     module context
 * @param   pData    - The data to provide the callback
 *
 * @return  SUCCESS, FAILURE
 */
bStatus_t BLEAppUtil_invokeFunction(InvokeFromBLEAppUtilContext_t callback, char* pData);

// Adv functions

/**
 * @brief   Initialize and starts advertise set (legacy or extended)
 *
 * @param   advHandle       - Return the created advertising handle
 * @param   advInitInfo     - Advertise set (legacy or extended) Initialization parameters
 *
 * @return  SUCCESS upon successful initialization,
 *          else, relevant error code upon failure
 */
bStatus_t BLEAppUtil_initAdvSet(uint8 *advHandle, const BLEAppUtil_AdvInit_t *advInitInfo);

/**
 * @brief   Enable Advertising.
 *          This will attempt to enable advertising for a set identified by the handle. The advertising set must first be created with GapAdv_create.
 *
 * Corresponding Events:
 *          @ref BLEAPPUTIL_ADV_START_AFTER_ENABLE after advertising is enabled
 *          @ref BLEAPPUTIL_ADV_START after each advertising set starts
 *          @ref BLEAPPUTIL_ADV_END after each advertising set ends
 *          @ref BLEAPPUTIL_SCAN_REQ_RECEIVED after received scan requests
 *
 * @param   handle	    - Handle of advertising set to enable
 * @param   advInitInfo - Initial advertising parameters according to @ref BLEAppUtil_AdvStart_t
 *
 * @return  @ref bleIncorrectMode : incorrect profile role or an update / prepare is in process
 * @return  @ref bleGAPNotFound : advertising set does not exist
 * @return  @ref bleNotReady : the advertising set has not yet been loaded with advertising data
 * @return  @ref bleAlreadyInRequestedMode : device is already advertising
 * @return  @ref SUCCESS
 */
bStatus_t BLEAppUtil_advStart(uint8 handle, const BLEAppUtil_AdvStart_t *advStartInfo);

/**
 * @brief   Disable an active advertising set.
 *          The set still exists and can be re-enabled with BLEAppUtil_advStart.
 *
 *          Corresponding Events:
 *          @ref BLEAPPUTIL_ADV_END_AFTER_DISABLE after advertising is disabled
 *
 * @param   handle  - Handle of advertising set to disable
 *
 * @return  @ref bleIncorrectMode : incorrect profile role or an update / prepare is in process.
 * @return  @ref bleGAPNotFound : advertising set does not exist.
 * @return  @ref bleAlreadyInRequestedMode : advertising set is not currently advertising
 * @return  @ref SUCCESS
 */
bStatus_t BLEAppUtil_advStop(uint8 handle);

// Scan functions

/**
 * @brief   Set initial scan and PHY parameters
 *
 * @param   scanInitInfo  - The parameters to set
 *
 * @return  @ref SUCCESS
 * @return  @ref bleInvalidRange
 * @return  @ref INVALIDPARAMETER
 * @return  @ref bleIncorrectMode
 */
bStatus_t BLEAppUtil_scanInit(const BLEAppUtil_ScanInit_t *scanInitInfo);

/**
 * @brief    Start scanning.
 *           If duration is zero period shall be ignored and the scanner
 *           will continue scanning until @ref BLEAppUtil_scanStop is called.
 *           If both period is zero and duration is non-zero, the scanner will
 *           scan once until duration has expired or @ref BLEAppUtil_scanStop
 *           is called. If both the duration and period are non-zero, the
 *           scanner will continue canning periodically until @ref BLEAppUtil_scanStop
 *           is called.
 *
 * @return  @ref SUCCESS
 * @return  @ref bleNotReady
 * @return  @ref bleInvalidRange
 * @return  @ref bleMemAllocError
 * @return  @ref bleAlreadyInRequestedMode
 * @return  @ref bleIncorrectMode
 */
bStatus_t BLEAppUtil_scanStart(const BLEAppUtil_ScanStart_t *scanStartInfo);

/**
 * @brief   Stop currently running scanning operation.
 *
 * @param   none
 * @return  @ref SUCCESS
 * @return  @ref FAILURE
 * @return  @ref bleIncorrectMode
 */
bStatus_t BLEAppUtil_scanStop(void);

// Connection initiation functions

/**
 * @brief   Set parameters dependent on PHY.
 *          Note that if phy contains more than one PHY,
 *          the same parameter of those PHYs will be set with the same value.
 * @param   connParams - connection parameters to set
 *
 * @return  @ref SUCCESS
 * @return  @ref bleInvalidRange
 */
bStatus_t BLEAppUtil_setConnParams(const BLEAppUtil_ConnParams_t *connParams);

/**
 * @brief   Initiate connection with the specified peer device
 *
 * @param   connParams - Peer device parameters
 *
 * @return  @ref SUCCESS
 * @return  @ref bleInvalidRange
 * @return  @ref bleMemAllocError
 * @return  @ref bleAlreadyInRequestedMode
 * @return  @ref bleNoResources
 */
bStatus_t BLEAppUtil_connect(BLEAppUtil_ConnectParams_t *connParams);

/**
 * @brief   Cancel the ongoing connection attempt.
 *
 * @param   none
 *
 * @return  @ref SUCCESS
 * @return  @ref bleIncorrectMode
 */
bStatus_t BLEAppUtil_cancelConnect(void);

// Connection general functions

/**
 * @brief   Terminate a link connection.
 *          Corresponding Events:
 *          GAP_LINK_TERMINATED_EVENT of type gapTerminateLinkEvent_t
 *
 * @param   connHandle - connection handle of link to terminate or LINKDB_CONNHANDLE_ALL
 *
 * @return  @ref SUCCESS - termination request sent to stack
 * @return  @ref bleIncorrectMode - No Link to terminate
 * @return  @ref bleInvalidTaskID - not app that established link
 */
bStatus_t BLEAppUtil_disconnect(uint16 connHandle);

/**
 * @brief   Set Phy Preference on the current connection. Apply the same value
 *          for RX and TX. For more information, see the LE 2M PHY section in the User's Guide:
 *          http://software-dl.ti.com/lprf/ble5stack-latest/
 *
 * @param   phyParams - Phy Preference on the current connection.
 *
 * @return  @ref HCI_SUCCESS
 */
bStatus_t BLEAppUtil_setConnPhy(BLEAppUtil_ConnPhyParams_t *phyParams);

// Connection Event notifications

/**
 * @brief   Register a connection event callback.
 *          It is only possible to register for one connection handle of for all connection handles.
 *          It it possible to register to certain type of connection event.
 *
 * @note    The callback needs to be registered for each reconnection. It is not retained across a disconnect / reconnect.
 *
 * @param   connHandle - if @ref LINKDB_CONNHANDLE_ALL, apply to all connections.
 *                       else, apply only for a specific connection.
 *
 * @return  @ref SUCCESS
 * @return  @ref bleGAPNotFound : connection handle not found
 * @return  @ref bleMemAllocError : there is not enough memory to register the callback.
 */
bStatus_t BLEAppUtil_registerConnNotifHandler(uint16_t connHandle);

/**
 * @brief   Unregister a connection event callback.
 *
 * @note    There is no need to unregister in order to change the type of connection event registered. It can be changed with a new call.
 *
 * @return  @ref SUCCESS
 * @return  @ref bleGAPNotFound : connection handle not found
 */
bStatus_t BLEAppUtil_unRegisterConnNotifHandler(void);

// Connection parameter update related functions

/**
 * @brief   Update the link parameters to a Central or Peripheral device.
 *          As long as LL connection updates are supported on the own device (which is the case by default),
 *          an LL Connection Update procedure will be attempted.
 *          If this fails, the stack will automatically attempt an L2CAP parameter update request.
 *
 * @param   pReq - Pointer to Link Request parameters
 *
 * @return  @ref SUCCESS : reply sent successfully
 * @return  @ref INVALIDPARAMETER : one of the parameters were invalid
 * @return  @ref bleIncorrectMode : invalid profile role
 * @return  @ref bleAlreadyInRequestedMode : already updating link parameters
 * @return  @ref bleNotConnected : not in a connection
 */
bStatus_t BLEAppUtil_paramUpdateReq(gapUpdateLinkParamReq_t *pReq);

/**
 * @brief   This function prepares an answer for connection parameters
 *          changing request, according to the decision made by the application,
 *          And then call for gap function with the response.
 *
 * @param   pReq - Pointer to central request
 * @param   accept - Application decision.
 *
 * @return  @ref SUCCESS : reply sent successfully
 * @return  @ref INVALIDPARAMETER : one of the parameters were invalid
 */
bStatus_t BLEAppUtil_paramUpdateRsp(gapUpdateLinkParamReqEvent_t *pReq, uint8 accept);

/**
 * @brief   Convert Bluetooth address to string.
 *
 * @param   pAddr - BD address
 *
 * @return  BD address as a string
 */
char *BLEAppUtil_convertBdAddr2Str(uint8_t *pAddr);

/** @} End BLEAppUtil_Functions */

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* BLEAPPUTIL_API_H */

/** @} End BLEAppUtil */
