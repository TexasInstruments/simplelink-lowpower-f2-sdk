/******************************************************************************

@file  BLEAppUtil_process.c

@brief This file contains the BLEAppUtil module events processing functions

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


/*********************************************************************
 * INCLUDES
 */
#include <ti/bleapp/ble_app_util/inc/bleapputil_api.h>
#include <ti/bleapp/ble_app_util/inc/bleapputil_internal.h>

/*********************************************************************
 * MACROS
 */
#define BLEAPPUTIL_GAP_PERIODIC_TABLE_SIZE      14
#define BLEAPPUTIL_GAP_PERIODIC_EVENTS_OFFSET   0x19

#define BLEAPPUTIL_GAP_CONN_TABLE_SIZE          20
#define BLEAPPUTIL_GAP_CONN_EVENTS_OFFSET       0x05

#define BLEAPPUTIL_GATT_TABLE_SIZE              38

#define BLEAPPUTIL_L2CAP_SIGNAL_EVENTS_OFFSET   0x60
#define BLEAPPUTIL_L2CAP_SIGNAL_TABLE_SIZE      6

/*********************************************************************
* CONSTANTS
*/

// The following look up table is used to convert GAP periodic events
// received from the BLE stack to BLEAppUtil periodic events
const uint32_t periodicEventsLookupTable[BLEAPPUTIL_GAP_PERIODIC_TABLE_SIZE] =
{
    BLEAPPUTIL_ADV_SET_PERIODIC_ADV_PARAMS_EVENT,
    BLEAPPUTIL_ADV_SET_PERIODIC_ADV_DATA_EVENT,
    BLEAPPUTIL_ADV_SET_PERIODIC_ADV_ENABLE_EVENT,
    BLEAPPUTIL_SCAN_CREATE_SYNC_EVENT,
    BLEAPPUTIL_SCAN_SYNC_CANCEL_EVENT,
    BLEAPPUTIL_SCAN_TERMINATE_SYNC_EVENT,
    BLEAPPUTIL_SCAN_PERIODIC_RECEIVE_EVENT,
    BLEAPPUTIL_SCAN_ADD_DEVICE_ADV_LIST_EVENT,
    BLEAPPUTIL_SCAN_REMOVE_DEVICE_ADV_LIST_EVENT,
    BLEAPPUTIL_SCAN_READ_ADV_LIST_SIZE_EVENT,
    BLEAPPUTIL_SCAN_CLEAR_ADV_LIST_EVENT,
    BLEAPPUTIL_SCAN_PERIODIC_ADV_SYNC_EST_EVENT,
    BLEAPPUTIL_SCAN_PERIODIC_ADV_SYNC_LOST_EVENT,
    BLEAPPUTIL_SCAN_PERIODIC_ADV_REPORT_EVENT
};

// The following look up table is used to convert GAP connection related
// events received from the BLE stack to BLEAppUtil GAP Conn events
const uint32_t gapConnEventsLookupTable[BLEAPPUTIL_GAP_CONN_TABLE_SIZE] =
{
    BLEAPPUTIL_LINK_ESTABLISHED_EVENT,
    BLEAPPUTIL_LINK_TERMINATED_EVENT,
    BLEAPPUTIL_LINK_PARAM_UPDATE_EVENT,
    0,
    BLEAPPUTIL_SIGNATURE_UPDATED_EVENT,
    BLEAPPUTIL_AUTHENTICATION_COMPLETE_EVENT,
    BLEAPPUTIL_PASSKEY_NEEDED_EVENT,
    BLEAPPUTIL_PERIPHERAL_REQUESTED_SECURITY_EVENT,
    0,
    BLEAPPUTIL_BOND_COMPLETE_EVENT,
    BLEAPPUTIL_PAIRING_REQ_EVENT,
    BLEAPPUTIL_AUTHENTICATION_FAILURE_EVT,
    BLEAPPUTIL_LINK_PARAM_UPDATE_REQ_EVENT,
    0,
    0,
    0,
    BLEAPPUTIL_CONNECTING_CANCELLED_EVENT,
    0,
    BLEAPPUTIL_BOND_LOST_EVENT,
    BLEAPPUTIL_LINK_PARAM_UPDATE_REJECT_EVENT
};

// The following look up table is used to convert GATT events received from
// the BLE stack to BLEAppUtil GATT events
const uint32_t gattEventsLookupTable[BLEAPPUTIL_GATT_TABLE_SIZE] =
{
     0,
     BLEAPPUTIL_ATT_ERROR_RSP,
     BLEAPPUTIL_ATT_EXCHANGE_MTU_REQ,
     BLEAPPUTIL_ATT_EXCHANGE_MTU_RSP,
     BLEAPPUTIL_ATT_FIND_INFO_REQ,
     BLEAPPUTIL_ATT_FIND_INFO_RSP,
     BLEAPPUTIL_ATT_FIND_BY_TYPE_VALUE_REQ,
     BLEAPPUTIL_ATT_FIND_BY_TYPE_VALUE_RSP,
     BLEAPPUTIL_ATT_READ_BY_TYPE_REQ,
     BLEAPPUTIL_ATT_READ_BY_TYPE_RSP,
     BLEAPPUTIL_ATT_READ_REQ,
     BLEAPPUTIL_ATT_READ_RSP,
     BLEAPPUTIL_ATT_READ_BLOB_REQ,
     BLEAPPUTIL_ATT_READ_BLOB_RSP,
     BLEAPPUTIL_ATT_READ_MULTI_REQ,
     BLEAPPUTIL_ATT_READ_MULTI_RSP,
     BLEAPPUTIL_ATT_READ_BY_GRP_TYPE_REQ,
     BLEAPPUTIL_ATT_READ_BY_GRP_TYPE_RSP,
     BLEAPPUTIL_ATT_WRITE_REQ,
     BLEAPPUTIL_ATT_WRITE_RSP,
     0,
     0,
     BLEAPPUTIL_ATT_PREPARE_WRITE_REQ,
     BLEAPPUTIL_ATT_PREPARE_WRITE_RSP,
     BLEAPPUTIL_ATT_EXECUTE_WRITE_REQ,
     BLEAPPUTIL_ATT_EXECUTE_WRITE_RSP,
     0,
     BLEAPPUTIL_ATT_HANDLE_VALUE_NOTI,
     0,
     BLEAPPUTIL_ATT_HANDLE_VALUE_IND,
     BLEAPPUTIL_ATT_HANDLE_VALUE_CFM,
     0,
     BLEAPPUTIL_ATT_UNSUPPORTED_METHOD,
     BLEAPPUTIL_ATT_WRITE_CMD,
     BLEAPPUTIL_ATT_SIGNED_WRITE_CMD,
     BLEAPPUTIL_ATT_TRANSACTION_READY_EVENT,
     BLEAPPUTIL_ATT_FLOW_CTRL_VIOLATED_EVENT,
     BLEAPPUTIL_ATT_MTU_UPDATED_EVENT
};

// The following look up table is used to convert L2CAP Signal events received
// from the BLE stack to BLEAppUtil L2CAP Signal events
const uint32_t l2capSignalEventsLookupTable[BLEAPPUTIL_L2CAP_SIGNAL_TABLE_SIZE] =
{
    BLEAPPUTIL_L2CAP_CHANNEL_ESTABLISHED_EVT,
    BLEAPPUTIL_L2CAP_CHANNEL_TERMINATED_EVT,
    BLEAPPUTIL_L2CAP_OUT_OF_CREDIT_EVT,
    BLEAPPUTIL_L2CAP_PEER_CREDIT_THRESHOLD_EVT,
    BLEAPPUTIL_L2CAP_SEND_SDU_DONE_EVT,
    BLEAPPUTIL_L2CAP_NUM_CTRL_DATA_PKT_EVT
};
/*********************************************************************
* TYPEDEFS
*/

/*********************************************************************
* GLOBAL VARIABLES
*/


/*********************************************************************
* LOCAL VARIABLES
*/

/*********************************************************************
* LOCAL FUNCTIONS
*/


/*********************************************************************
 * FUNCTIONS
 */

/*********************************************************************
 * @fn      BLEAppUtil_processGAPEvents
 *
 * @brief   Process GAP events received from the BLE stack, called
 *          from BLEAppUtil task context.
 *          This function converts the BLE stack events to BLEAppUtil
 *          events (which are bit mask events) and calls @ref
 *          BLEAppUtil_callEventHandler
 *
 * @param   pMsg - the message the process
 *
 * @return  None
 */
void BLEAppUtil_processGAPEvents(BLEAppUtil_msgHdr_t *pMsg)
{
    gapEventHdr_t * pMsgData = (gapEventHdr_t *)pMsg;

    switch(pMsgData->opcode)
    {

        // This event will not be received in one of the event handlers
        // registered by the application, the StackInitDone_t handler provided
        // by BLEAppUtil_init is called.
        case GAP_DEVICE_INIT_DONE_EVENT:
        {
            appInitDoneHandler((gapDeviceInitDoneEvent_t *)pMsgData);
            break;
        }

        // The following events are converted to events from type of
        // BLEAPPUTIL_GAP_CONN_TYPE
        case GAP_LINK_ESTABLISHED_EVENT:
        case GAP_LINK_TERMINATED_EVENT:
        case GAP_LINK_PARAM_UPDATE_EVENT:
        case GAP_SIGNATURE_UPDATED_EVENT:
        case GAP_AUTHENTICATION_COMPLETE_EVENT:
        case GAP_PASSKEY_NEEDED_EVENT:
        case GAP_PERIPHERAL_REQUESTED_SECURITY_EVENT:
        case GAP_BOND_COMPLETE_EVENT:
        case GAP_PAIRING_REQ_EVENT:
        case GAP_AUTHENTICATION_FAILURE_EVT:
        case GAP_UPDATE_LINK_PARAM_REQ_EVENT:
        case GAP_CONNECTING_CANCELLED_EVENT:
        case GAP_BOND_LOST_EVENT:
        case GAP_LINK_PARAM_UPDATE_REJECT_EVENT:
        {
            // Pass the event and msg to BLEAppUtil_callEventHandler which calls the
            // handler of the application
            BLEAppUtil_callEventHandler(
                    gapConnEventsLookupTable[pMsgData->opcode - BLEAPPUTIL_GAP_CONN_EVENTS_OFFSET],
                    pMsg, BLEAPPUTIL_GAP_CONN_TYPE);
            break;
        }

        // The following events are converted to events from type of
        // BLEAPPUTIL_GAP_PERIODIC_TYPE
        case GAP_ADV_SET_PERIODIC_ADV_PARAMS_EVENT:
        case GAP_ADV_SET_PERIODIC_ADV_DATA_EVENT:
        case GAP_ADV_SET_PERIODIC_ADV_ENABLE_EVENT:
        case GAP_SCAN_CREATE_SYNC_EVENT:
        case GAP_SCAN_SYNC_CANCEL_EVENT:
        case GAP_SCAN_TERMINATE_SYNC_EVENT:
        case GAP_SCAN_PERIODIC_RECEIVE_EVENT:
        case GAP_SCAN_ADD_DEVICE_ADV_LIST_EVENT:
        case GAP_SCAN_REMOVE_DEVICE_ADV_LIST_EVENT:
        case GAP_SCAN_READ_ADV_LIST_SIZE_EVENT:
        case GAP_SCAN_CLEAR_ADV_LIST_EVENT:
        case GAP_SCAN_PERIODIC_ADV_SYNC_EST_EVENT:
        case GAP_SCAN_PERIODIC_ADV_SYNC_LOST_EVENT:
        case GAP_SCAN_PERIODIC_ADV_REPORT_EVENT:
        {
            // Pass the event and msg to BLEAppUtil_callEventHandler which calls the
            // handler of the application
            BLEAppUtil_callEventHandler(
                    periodicEventsLookupTable[pMsgData->opcode - BLEAPPUTIL_GAP_PERIODIC_EVENTS_OFFSET],
                    pMsg, BLEAPPUTIL_GAP_PERIODIC_TYPE);
            break;
        }

        default:
        {
            break;
        }

    }
}

/*********************************************************************
 * @fn      BLEAppUtil_processGATTEvents
 *
 * @brief   Process GATT events received from the BLE stack, called
 *          from BLEAppUtil task context.
 *          This function converts the BLE stack events to BLEAppUtil
 *          events (which are bit mask events) and calls @ref
 *          BLEAppUtil_callEventHandler.
 *          All the events processed in this function will be received by
 *          handlers form type of BLEAPPUTIL_GATT_TYPE
 *
 * @param   pMsg - the message the process
 *
 * @return  None
 */
void BLEAppUtil_processGATTEvents(BLEAppUtil_msgHdr_t *pMsg)
{
    gattMsgEvent_t * pMsgData = (gattMsgEvent_t *)pMsg;
    uint32_t event = 0;

    switch (pMsgData->method)
    {
        case ATT_WRITE_CMD:
        {
            event = BLEAPPUTIL_ATT_WRITE_CMD;
            break;
        }

        case ATT_SIGNED_WRITE_CMD:
        {
            event = BLEAPPUTIL_ATT_SIGNED_WRITE_CMD;
            break;
        }

        case ATT_TRANSACTION_READY_EVENT:
        {
            event = BLEAPPUTIL_ATT_TRANSACTION_READY_EVENT;
            break;
        }

        case ATT_FLOW_CTRL_VIOLATED_EVENT:
        {
            event = BLEAPPUTIL_ATT_FLOW_CTRL_VIOLATED_EVENT;
            break;
        }

        case ATT_MTU_UPDATED_EVENT:
        {
            event = BLEAPPUTIL_ATT_MTU_UPDATED_EVENT;
            break;
        }

        default:
        {
            event = gattEventsLookupTable[pMsgData->method];
            break;
        }
    }

    // Pass the event and msg to BLEAppUtil_callEventHandler which calls the
    // handler of the application
    BLEAppUtil_callEventHandler(event, pMsg, BLEAPPUTIL_GATT_TYPE);
}

/*********************************************************************
 * @fn      BLEAppUtil_processHCIGAPEvents
 *
 * @brief   Process HCI GAP events received from the BLE stack, called
 *          from BLEAppUtil task context.
 *          This function converts the BLE stack events to BLEAppUtil
 *          events (which are bit mask events) and calls @ref
 *          BLEAppUtil_callEventHandler.
 *          All the events processed in this function will be received by
 *          handlers form type of BLEAPPUTIL_HCI_GAP_TYPE
 *
 * @param   pMsg - the message the process
 *
 * @return  None
 */
void BLEAppUtil_processHCIGAPEvents(BLEAppUtil_msgHdr_t *pMsg)
{
    ICall_Hdr * pMsgData = (ICall_Hdr *)pMsg;
    uint32_t event = 0;

    switch(pMsgData->status)
    {
        case HCI_DISCONNECTION_COMPLETE_EVENT_CODE:
        {
            event = BLEAPPUTIL_HCI_DISCONNECTION_COMPLETE_EVENT_CODE;
            break;
        }

        case HCI_COMMAND_COMPLETE_EVENT_CODE:
        {
            event = BLEAPPUTIL_HCI_COMMAND_COMPLETE_EVENT_CODE;
            break;
        }

        case HCI_COMMAND_STATUS_EVENT_CODE:
        {
            event = BLEAPPUTIL_HCI_COMMAND_STATUS_EVENT_CODE;
            break;
        }

        case HCI_LE_EVENT_CODE:
        {
            event = BLEAPPUTIL_HCI_LE_EVENT_CODE;
            break;
        }

        case HCI_VE_EVENT_CODE:
        {
            event = BLEAPPUTIL_HCI_VE_EVENT_CODE;
            break;
        }

        case HCI_BLE_HARDWARE_ERROR_EVENT_CODE:
        {
            event = BLEAPPUTIL_HCI_BLE_HARDWARE_ERROR_EVENT_CODE;
            break;
        }

        default:
        {
            break;
        }
    }

    // Pass the event and msg to BLEAppUtil_callEventHandler which calls the
    // handler of the application
    BLEAppUtil_callEventHandler(event, pMsg, BLEAPPUTIL_HCI_GAP_TYPE);
}

/*********************************************************************
 * @fn      BLEAppUtil_processHCIDataEvents
 *
 * @brief   Process HCI Data events received from the BLE stack, called
 *          from BLEAppUtil task context.
 *          This function converts the BLE stack events to BLEAppUtil
 *          events (which are bit mask events) and calls @ref
 *          BLEAppUtil_callEventHandler.
 *          All the events processed in this function will be received by
 *          handlers form type of BLEAPPUTIL_HCI_DATA_TYPE
 *
 * @param   pMsg - the message the process
 *
 * @return  None
 */
void BLEAppUtil_processHCIDataEvents(bleStack_msgHdt_t *pMsg)
{
    hciDataEvent_t * pMsgData = (hciDataEvent_t *)pMsg;
    uint32_t event = 0;

    switch(pMsgData->hdr.status)
    {
        case HCI_NUM_OF_COMPLETED_PACKETS_EVENT_CODE:
        {
            event = BLEAPPUTIL_HCI_NUM_OF_COMPLETED_PACKETS_EVENT_CODE;
            break;
        }

        case HCI_APTO_EXPIRED_EVENT_CODE:
        {
            event = BLEAPPUTIL_HCI_APTO_EXPIRED_EVENT_CODE;
            break;
        }

        default:
        {
            break;
        }
    }

    // Pass the event and msg to BLEAppUtil_callEventHandler which calls the
    // handler of the application
    BLEAppUtil_callEventHandler(event, pMsg, BLEAPPUTIL_HCI_DATA_TYPE);
}

/*********************************************************************
 * @fn      BLEAppUtil_processHCISMPEvents
 *
 * @brief   Process HCI SMP events received from the BLE stack, called
 *          from BLEAppUtil task context.
 *          This function converts the BLE stack events to BLEAppUtil
 *          events (which are bit mask events) and calls @ref
 *          BLEAppUtil_callEventHandler.
 *          All the events processed in this function will be received by
 *          handlers form type of BLEAPPUTIL_HCI_SMP_TYPE
 *
 * @param   pMsg - the message the process
 *
 * @return  None
 */
void BLEAppUtil_processHCISMPEvents(bleStack_msgHdt_t *pMsg)
{
    ICall_HciExtEvt * pMsgData = (ICall_HciExtEvt *)pMsg;
    uint32_t event = 0;

    switch(pMsgData->hdr.status)
    {
        case HCI_COMMAND_COMPLETE_EVENT_CODE:
        {
            event = BLEAPPUTIL_HCI_COMMAND_COMPLETE_EVENT_CODE;
            break;
        }

        case HCI_LE_EVENT_CODE:
        {
            event = BLEAPPUTIL_HCI_LE_EVENT_CODE;
            break;
        }

        default:
        {
            break;
        }
    }

    // Pass the event and msg to BLEAppUtil_callEventHandler which calls the
    // handler of the application
    BLEAppUtil_callEventHandler(event, pMsg, BLEAPPUTIL_HCI_SMP_TYPE);
}

/*********************************************************************
 * @fn      BLEAppUtil_processHCISMPMetaEvents
 *
 * @brief   Process HCI SMP Meta events received from the BLE stack, called
 *          from BLEAppUtil task context.
 *          This function converts the BLE stack events to BLEAppUtil
 *          events (which are bit mask events) and calls @ref
 *          BLEAppUtil_callEventHandler.
 *          All the events processed in this function will be received by
 *          handlers form type of BLEAPPUTIL_HCI_SMP_META_TYPE
 *
 * @param   pMsg - the message the process
 *
 * @return  None
 */
void BLEAppUtil_processHCICTRLToHostEvents(BLEAppUtil_msgHdr_t *pMsg)
{
    hciPacket_t *pBuf = (hciPacket_t *)pMsg;
    uint32_t event = 0;

    switch(pBuf->pData[0])
    {
        case HCI_EVENT_PACKET:
        {
            event = BLEAPPUTIL_HCI_EVENT_PACKET;
            break;
        }

        case HCI_ACL_DATA_PACKET:
        {
           event = BLEAPPUTIL_HCI_ACL_DATA_PACKET;
           break;
        }

        default:
        {
            break;
        }
    }

    // Pass the event and msg to BLEAppUtil_callEventHandler which calls the
    // handler of the application
    BLEAppUtil_callEventHandler(event, pMsg, BLEAPPUTIL_HCI_CTRL_TO_HOST_TYPE);
}
/*********************************************************************
 * @fn      BLEAppUtil_processHCISMPMetaEvents
 *
 * @brief   Process HCI SMP Meta events received from the BLE stack, called
 *          from BLEAppUtil task context.
 *          This function converts the BLE stack events to BLEAppUtil
 *          events (which are bit mask events) and calls @ref
 *          BLEAppUtil_callEventHandler.
 *          All the events processed in this function will be received by
 *          handlers form type of BLEAPPUTIL_HCI_SMP_META_TYPE
 *
 * @param   pMsg - the message the process
 *
 * @return  None
 */
void BLEAppUtil_processHCISMPMetaEvents(bleStack_msgHdt_t *pMsg)
{
    ICall_HciExtEvt * pMsgData = (ICall_HciExtEvt *)pMsg;
    uint32_t event = 0;

    switch(pMsgData->hdr.status)
    {
        case HCI_LE_EVENT_CODE:
        {
            event = BLEAPPUTIL_HCI_LE_EVENT_CODE;
            break;
        }

        default:
        {
            break;
        }
    }

    // Pass the event and msg to BLEAppUtil_callEventHandler which calls the
    // handler of the application
    BLEAppUtil_callEventHandler(event, pMsg, BLEAPPUTIL_HCI_SMP_META_TYPE);
}

/*********************************************************************
 * @fn      BLEAppUtil_processL2CAPDataMsg
 *
 * @brief   Process L2CAP Data received from the BLE stack, called
 *          from BLEAppUtil task context.
 *          This function calls @ref BLEAppUtil_callEventHandler.
 *          The handler is from the type of BLEAPPUTIL_L2CAP_DATA_TYPE
 *
 * @param   pMsg - the message the process
 *
 * @return  None
 */
void BLEAppUtil_processL2CAPDataMsg(BLEAppUtil_msgHdr_t *pMsg)
{
    // Pass the msg to BLEAppUtil_callEventHandler which calls the
    // handler of the application
    // Note: The event is not needed since the L2CAP data contains only data
    BLEAppUtil_callEventHandler(0, pMsg, BLEAPPUTIL_L2CAP_DATA_TYPE);
}

/*********************************************************************
 * @fn      BLEAppUtil_processL2CAPSignalEvents
 *
 * @brief   Process L2CAP Signal events received from the BLE stack, called
 *          from BLEAppUtil task context.
 *          This function converts the BLE stack events to BLEAppUtil
 *          events (which are bit mask events) and calls @ref
 *          BLEAppUtil_callEventHandler.
 *          All the events processed in this function will be received by
 *          handlers form type of BLEAPPUTIL_L2CAP_SIGNAL_TYPE
 *
 * @param   pMsg - the message the process
 *
 * @return  None
 */
void BLEAppUtil_processL2CAPSignalEvents(BLEAppUtil_msgHdr_t *pMsg)
{
    l2capSignalEvent_t *pMsgData = (l2capSignalEvent_t *)pMsg;

    // Pass the event and msg to BLEAppUtil_callEventHandler which calls the
    // handler of the application
    BLEAppUtil_callEventHandler(l2capSignalEventsLookupTable[pMsgData->opcode - BLEAPPUTIL_L2CAP_SIGNAL_EVENTS_OFFSET],
                                pMsg, BLEAPPUTIL_L2CAP_SIGNAL_TYPE);
}

/*********************************************************************
 * @fn      BLEAppUtil_processAdvEventMsg
 *
 * @brief   Process Adv events received from the BLE stack, called
 *          from BLEAppUtil task context.
 *          This function converts the BLE stack events to BLEAppUtil
 *          events (which are bit mask events) and calls @ref
 *          BLEAppUtil_callEventHandler.
 *          All the events processed in this function will be received by
 *          handlers form type of BLEAPPUTIL_GAP_ADV_TYPE
 *
 * @param   pMsg - the message the process
 *
 * @return  None
 */
void BLEAppUtil_processAdvEventMsg(BLEAppUtil_msgHdr_t *pMsg)
{
    // Pass the event and msg to BLEAppUtil_callEventHandler which calls the
    // handler of the application
    BLEAppUtil_callEventHandler(((BLEAppUtil_AdvEventData_t *)pMsg)->event,
                                pMsg, BLEAPPUTIL_GAP_ADV_TYPE);
}

/*********************************************************************
 * @fn      BLEAppUtil_processScanEventMsg
 *
 * @brief   Process Scan events received from the BLE stack, called
 *          from BLEAppUtil task context.
 *          This function converts the BLE stack events to BLEAppUtil
 *          events (which are bit mask events) and calls @ref
 *          BLEAppUtil_callEventHandler.
 *          All the events processed in this function will be received by
 *          handlers form type of BLEAPPUTIL_GAP_SCAN_TYPE
 *
 * @param   pMsg - the message the process
 *
 * @return  None
 */
void BLEAppUtil_processScanEventMsg(BLEAppUtil_msgHdr_t *pMsg)
{
    // Pass the event and msg to BLEAppUtil_callEventHandler which calls the
    // handler of the application
    BLEAppUtil_callEventHandler(((BLEAppUtil_ScanEventData_t *)pMsg)->event,
                                pMsg, BLEAPPUTIL_GAP_SCAN_TYPE);
}

/*********************************************************************
 * @fn      BLEAppUtil_processPairStateMsg
 *
 * @brief   Process PairState events received from the BLE stack, called
 *          from BLEAppUtil task context.
 *          This function converts the BLE stack events to BLEAppUtil
 *          events (which are bit mask events) and calls @ref
 *          BLEAppUtil_callEventHandler.
 *          All the events processed in this function will be received by
 *          handlers form type of BLEAPPUTIL_PAIR_STATE_TYPE
 *
 * @param   pMsg - the message the process
 *
 * @return  None
 */
void BLEAppUtil_processPairStateMsg(BLEAppUtil_msgHdr_t *pMsgData)
{
    // Pass the event and msg to BLEAppUtil_callEventHandler which calls the
    // handler of the application
    BLEAppUtil_callEventHandler(((BLEAppUtil_PairStateData_t *)pMsgData)->state,
                                pMsgData, BLEAPPUTIL_PAIR_STATE_TYPE);
}

/*********************************************************************
 * @fn      BLEAppUtil_processPasscodeMsg
 *
 * @brief   Process PassCode received from the BLE stack, called
 *          from BLEAppUtil task context.
 *          This function calls @ref BLEAppUtil_callEventHandler.
 *          The handler is from the type of BLEAPPUTIL_PASSCODE_TYPE
 *
 * @param   pMsg - the message the process
 *
 * @return  None
 */
void BLEAppUtil_processPasscodeMsg(BLEAppUtil_msgHdr_t *pMsgData)
{
    // Pass the msg to BLEAppUtil_callEventHandler which calls the
    // handler of the application
    // Note: The event is not needed since the passcode contains only data
    BLEAppUtil_callEventHandler(0, pMsgData, BLEAPPUTIL_PASSCODE_TYPE);
}

/*********************************************************************
 * @fn      BLEAppUtil_processConnEventMsg
 *
 * @brief   Process L2CAP Signal events received from the BLE stack, called
 *          from BLEAppUtil task context.
 *          This function converts the BLE stack events to BLEAppUtil
 *          events (which are bit mask events) and calls @ref
 *          BLEAppUtil_callEventHandler.
 *          All the events processed in this function will be received by
 *          handlers form type of BLEAPPUTIL_CONN_NOTI_TYPE
 *
 * @param   pMsg - the message the process
 *
 * @return  None
 */
void BLEAppUtil_processConnEventMsg(BLEAppUtil_msgHdr_t *pMsg)
{
    Gap_ConnEventRpt_t * pMsgData = (Gap_ConnEventRpt_t *)pMsg;
    uint32_t event = 0;

    switch(pMsgData->eventType)
    {
        case GAP_CB_EVENT_INVALID:
        {
            event = BLEAPPUTIL_CONN_NOTI_EVENT_INVALID;
            break;
        }

        case GAP_CB_CONN_ESTABLISHED:
        {
            event = BLEAPPUTIL_CONN_NOTI_CONN_ESTABLISHED;
            break;
        }

        case GAP_CB_PHY_UPDATE:
        {
            event = BLEAPPUTIL_CONN_NOTI_PHY_UPDATE;
            break;
        }

        case GAP_CB_CONN_EVENT_ALL:
        {
            event = BLEAPPUTIL_CONN_NOTI_CONN_EVENT_ALL;
            break;
        }

        default:
        {
            break;
        }
    }

    // Pass the event and msg to BLEAppUtil_callEventHandler which calls the
    // handler of the application
    BLEAppUtil_callEventHandler(event ,pMsg, BLEAPPUTIL_CONN_NOTI_TYPE);
}

/*********************************************************************
 * @fn      BLEAppUtil_callEventHandler
 *
 * @brief   Process L2CAP Signal events received from the BLE stack, called
 *          from BLEAppUtil task context.
 *          This function converts the BLE stack events to BLEAppUtil
 *          events (which are bit mask events) and calls @ref
 *          BLEAppUtil_callEventHandler.
 *          All the events processed in this function will be received by
 *          handlers form type of BLEAPPUTIL_CONN_NOTI_TYPE
 *
 * @param   event   - The event the application handler will receive
 * @param   pMsg    - The msg the application handler will receive
 * @param   type    - The handler type to call
 *
 * @return  None
 */
void BLEAppUtil_callEventHandler(uint32_t event, BLEAppUtil_msgHdr_t *pMsg, BLEAppUtil_eventHandlerType_e type)
{
    BLEAppUtil_EventHandlersList_t *iter = BLEAppUtilEventHandlersHead;

    // Lock the Mutex
    pthread_mutex_lock(&mutex);

    // Iterat over the handlers list
    while(iter != NULL)
    {
        // Verify that the handler exist and it is from the correct type
        if(iter->eventHandler->pEventHandler && iter->eventHandler->handlerType == type)
        {
            // If the handler is from PASSCODE or L2CAP_DATA types or
            // (for all other types) the event is part of the event mask,
            // call the handler
            if(type == BLEAPPUTIL_PASSCODE_TYPE ||
               type == BLEAPPUTIL_L2CAP_DATA_TYPE ||
               (iter->eventHandler->eventMask & event))
            {
                iter->eventHandler->pEventHandler(event, pMsg);
            }
        }

        // Next item in the list
        iter = (BLEAppUtil_EventHandlersList_t *)iter->next;
    }

    // Unlock the Mutex - handlers were called
    pthread_mutex_unlock(&mutex);
}
