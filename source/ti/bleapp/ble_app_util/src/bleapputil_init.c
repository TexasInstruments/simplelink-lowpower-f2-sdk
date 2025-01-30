/******************************************************************************

@file  BLEAppUtil_init.c

@brief This file contains the BLEAppUtil module initialization functions

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
#include "ble_stack_api.h"

/*********************************************************************
 * MACROS
 */

/*********************************************************************
* CONSTANTS
*/

/*********************************************************************
* TYPEDEFS
*/

/*********************************************************************
* GLOBAL VARIABLES
*/
BLEAppUtil_TheardEntity_t BLEAppUtil_theardEntity;

/*********************************************************************
* LOCAL VARIABLES
*/
// Entity ID globally used to check for source and/or destination of messages
static BLEAppUtil_entityId_t BLEAppUtilSelfEntity;

// BLEAppUtil parameters given in the init function
BLEAppUtil_GeneralParams_t *BLEAppUtilLocal_GeneralParams = NULL;
BLEAppUtil_PeriCentParams_t *BLEAppUtilLocal_PeriCentParams = NULL;

// Callback functions handlers
ErrorHandler_t errorHandlerCb;
StackInitDone_t appInitDoneHandler;
BLEAppUtil_EventHandlersList_t *BLEAppUtilEventHandlersHead = NULL;

// GAP Bond Manager Callbacks
gapBondCBs_t BLEAppUtil_bondMgrCBs =
{
    BLEAppUtil_passcodeCB, // Passcode callback
    BLEAppUtil_pairStateCB // Pairing/Bonding state Callback
};

pthread_mutex_t mutex;

/*********************************************************************
* LOCAL FUNCTIONS
*/
static bStatus_t BLEAppUtil_createQueue(void);

/*********************************************************************
 * EXTERN FUNCTIONS
*/

/*********************************************************************
* CALLBACKS
*/

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      BLEAppUtil_init
 *
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
                     BLEAppUtil_PeriCentParams_t *initPeriCentParams)
{
    // Register the application error handler
    errorHandlerCb = errorHandler;

    // Register the init done handler - will be called from the GAP_DEVICE_INIT_DONE_EVENT
    appInitDoneHandler = initDoneHandler;

    // Assign the BLEAppUtil parameters from the user to the local parameters
    BLEAppUtilLocal_GeneralParams = initGeneralParams;
    BLEAppUtilLocal_PeriCentParams = initPeriCentParams;

    // Create a message queue for message to be sent to BLEAppUtil
    BLEAppUtil_createQueue();

    // Create BLE stack task
    bleStack_createTasks();

    // Create local app task
    BLEAppUtil_createBLEAppUtilTask();

    // Construct a mutex that will be used by the following functions:
    // BLEAppUtil_registerEventHandler
    // BLEAppUtil_unRegisterEventHandler
    // BLEAppUtil_callEventHandler
    pthread_mutex_init(&mutex, NULL);
}

/*********************************************************************
 * @fn      BLEAppUtil_registerEventHandler
 *
 * @brief   Register application event handler
 *
 * @param   eventHandler - The handler to register
 *
 * @return  SUCCESS, FAILURE
 */
bStatus_t BLEAppUtil_registerEventHandler(BLEAppUtil_EventHandler_t *eventHandler)
{
    BLEAppUtil_EventHandlersList_t *newHandler;

    // Lock the Mutex
    pthread_mutex_lock(&mutex);

    // Allocate the new handler space
    newHandler = (BLEAppUtil_EventHandlersList_t *)BLEAppUtil_malloc(sizeof(BLEAppUtil_EventHandlersList_t));

    // If the allocation failed, return an error
    if(newHandler == NULL)
    {
        return FAILURE;
    }

    // Set the parameters of the new item
    newHandler->eventHandler = eventHandler;
    newHandler->next = NULL;

    // The head is NULL
    if(BLEAppUtilEventHandlersHead == NULL)
    {
        BLEAppUtilEventHandlersHead = newHandler;
    }
    else
    {
        // Add item to be the head of the list
        BLEAppUtil_EventHandlersList_t *iter = BLEAppUtilEventHandlersHead;

        // Iterate through the list to get to the last item
        while(iter->next != NULL)
        {
          iter = (BLEAppUtil_EventHandlersList_t *)iter->next;
        }

        // Add to the end of the list
        iter->next = (struct BLEAppUtil_EventHandlersList_t *)newHandler;
    }

    // Unlock the Mutex - item was added to the list
    pthread_mutex_unlock(&mutex);

    return SUCCESS;
}

/*********************************************************************
 * @fn      BLEAppUtil_unRegisterEventHandler
 *
 * @brief   Un-register application event handler
 *
 * @param   eventHandler - The handler to un-register
 *
 * @return  SUCCESS, INVALIDPARAMETER
 */
bStatus_t BLEAppUtil_unRegisterEventHandler(BLEAppUtil_EventHandler_t *eventHandler)
{
    BLEAppUtil_EventHandlersList_t *curr = BLEAppUtilEventHandlersHead;
    BLEAppUtil_EventHandlersList_t *prev = NULL;
    bStatus_t status = INVALIDPARAMETER;

    // Lock the Mutex
    pthread_mutex_lock(&mutex);

    // Go over the handlers list
    while(curr != NULL)
    {
        // The item containing the handler to un-register is found
        if(curr->eventHandler == eventHandler)
        {
            // The item is the head
            if(prev == NULL)
            {
                // Change the head to point the next item in the list
                BLEAppUtilEventHandlersHead = (BLEAppUtil_EventHandlersList_t *)curr->next;
            }
            // The item is not the head
            else
            {
                prev->next = curr->next;
            }

            // Free the item
            BLEAppUtil_free(curr);

            // Set the status to SUCCESS
            status = SUCCESS;
            break;
        }

        // Update the prev item to point the current item
        prev = curr;
        // Update the current item to point the next item
        curr = (BLEAppUtil_EventHandlersList_t *)prev->next;
    }

    // Unlock the Mutex - handler was removed from the list
    pthread_mutex_unlock(&mutex);

    return status;
}


/*********************************************************************
 * @fn      BLEAppUtil_invokeFunctionNoData
 *
 * @brief   This function receives a callback and data and switches
 *          the context in order to call the callback from the BLE
 *          App Util module context
 *
 * @param   callback - The callback to invoke from the BLE App Util
 *                     module context
 *
 * @return  SUCCESS, FAILURE
 */
bStatus_t BLEAppUtil_invokeFunctionNoData(InvokeFromBLEAppUtilContext_t callback)
{
    return BLEAppUtil_invokeFunction(callback, NULL);
}

/*********************************************************************
 * @fn      BLEAppUtil_invokeFunction
 *
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
bStatus_t BLEAppUtil_invokeFunction(InvokeFromBLEAppUtilContext_t callback, char* pData)
{
    BLEAppUtil_CallbackToInvoke_t *pDataMsg = NULL;

    // If the callback doesn't exist retun an error
    if(callback == NULL)
    {
        return FAILURE;
    }

    // Create a BLEAppUtil_CallbackToInvoke_t and assign parameters
    pDataMsg = BLEAppUtil_malloc(sizeof(BLEAppUtil_CallbackToInvoke_t));

    // If the allocation failed, return an error
    if(pDataMsg == NULL)
    {
        if(pData != NULL)
        {
            BLEAppUtil_free(pData);
        }
        return FAILURE;
    }

    pDataMsg->callback = callback;
    pDataMsg->data     = pData;

    // Queue the event and data to switch context
    if (BLEAppUtil_enqueueMsg(BLEAPPUTIL_EVT_CALL_IN_BLEAPPUTIL_CONTEXT, pDataMsg) != SUCCESS)
    {
        if(pData != NULL)
        {
            BLEAppUtil_free(pData);
        }
        BLEAppUtil_free(pDataMsg);
        return FAILURE;
    }
    return SUCCESS;
}

/*********************************************************************
* LOCAL FUNCTIONS
*/

/*********************************************************************
 * @fn      BLEAppUtil_stackRegister
 *
 * @brief   Register to stack messages callback
 *
 * @return  None
 */
void BLEAppUtil_stackRegister(void)
{
    bleStack_errno_t status;

    // ******************************************************************
    // NO STACK API CALLS CAN OCCUR BEFORE THIS CALL TO bleStack_register
    // ******************************************************************
    // Register the stack messages callback
    // See ble_stack_api.c - #ifdef ICALL_NO_APP_EVENTS
    status = bleStack_register(&BLEAppUtilSelfEntity,
                               BLEAppUtil_processStackMsgCB);
    if(status != SUCCESS)
    {
        // TODO: Call Error Handler
    }
}

/*********************************************************************
 * @fn      BLEAppUtil_stackInit
 *
 * @brief   Initialize the BLE stack host using the parameters from @ref
 *          BLEAppUtil_GeneralParams_t and @ref BLEAppUtil_PeriCentParams_t,
 *          provided by @ref BLEAppUtil_init
 *
 * @return  None
 */
void BLEAppUtil_stackInit(void)
{
    bStatus_t status;

    // Init GAP
    status = bleStack_initGap(BLEAppUtilLocal_GeneralParams->profileRole,
                              BLEAppUtilSelfEntity, BLEAppUtil_scanCB,
                              BLEAppUtilLocal_PeriCentParams->connParamUpdateDecision);
    if(status != SUCCESS)
    {
        // TODO: Call Error Handler
    }

    // Init GapBond
    status = bleStack_initGapBond(BLEAppUtilLocal_PeriCentParams->gapBondParams,
                                  &BLEAppUtil_bondMgrCBs);
    if(status != SUCCESS)
    {
        // TODO: Call Error Handler
    }

#if defined( HOST_CONFIG ) && ( HOST_CONFIG & ( PERIPHERAL_CFG | CENTRAL_CFG ) )
    // Init GATT
    status = bleStack_initGatt(BLEAppUtilLocal_GeneralParams->profileRole,
                               BLEAppUtilSelfEntity,
                               BLEAppUtilLocal_GeneralParams->deviceNameAtt);
    if(status != SUCCESS)
    {
        // TODO: Call Error Handler
    }
#endif

    // Initialize GAP layer to receive GAP events
    status = GAP_DeviceInit(BLEAppUtilLocal_GeneralParams->profileRole,
                            BLEAppUtilSelfEntity,
                            BLEAppUtilLocal_GeneralParams->addressMode,
                            BLEAppUtilLocal_GeneralParams->pDeviceRandomAddress);
    if(status != SUCCESS)
    {
        // TODO: Call Error Handler
    }
}

/*********************************************************************
 * @fn      BLEAppUtil_getSelfEntity
 *
 * @brief   Returns the self entity id needed when sending BLE stack
 *          commands
 *
 * @return  The entityId
 */
BLEAppUtil_entityId_t BLEAppUtil_getSelfEntity(void)
{
    return BLEAppUtilSelfEntity;
}

/*********************************************************************
 * @fn      BLEAppUtil_createQueue
 *
 * @brief   Create a message queue for message to be sent to BLEAppUtil
 *
 * @return  SUCCESS, FAILURE
 */
static bStatus_t BLEAppUtil_createQueue(void)
{
     struct mq_attr attr;

     attr.mq_flags = 0; // Parameter mq_flags is ignored when the queue is created with O_CREAT
     attr.mq_curmsgs = 0;
     attr.mq_maxmsg = 8;
     attr.mq_msgsize = sizeof(BLEAppUtil_appEvt_t);

     /* Create the message queue */
     BLEAppUtil_theardEntity.queueHandle = mq_open("BLEAppUtil_theardQueue", O_CREAT , 0, &attr);

     if (BLEAppUtil_theardEntity.queueHandle == (mqd_t)-1)
     {
         return FAILURE;
     }
     return SUCCESS;
}

/////////////////////////////////////////////////////////////////////////
// Host Functions Encapsulation
/////////////////////////////////////////////////////////////////////////

bStatus_t BLEAppUtil_initAdvSet(uint8 *advHandle, const BLEAppUtil_AdvInit_t *advInitInfo)
{
    return bleStk_initAdvSet(BLEAppUtil_advCB, advHandle, GAP_ADV_EVT_MASK_ALL,
                             advInitInfo->advParam, advInitInfo->advDataLen ,
                             advInitInfo->advData, advInitInfo->scanRespDataLen,
                             advInitInfo->scanRespData);
}

bStatus_t BLEAppUtil_advStart(uint8 handle, const BLEAppUtil_AdvStart_t *advStartInfo)
{
    return GapAdv_enable(handle, advStartInfo->enableOptions ,
                         advStartInfo->durationOrMaxEvents);
}

bStatus_t BLEAppUtil_advStop(uint8 handle)
{
    return GapAdv_disable(handle);
}


bStatus_t BLEAppUtil_scanInit(const BLEAppUtil_ScanInit_t *scanInitInfo)
{
    bStatus_t status;

    // Set Scan PHY parameters
    status = GapScan_setPhyParams(scanInitInfo->primPhy,
                                  scanInitInfo->scanType,
                                  scanInitInfo->scanInterval,
                                  scanInitInfo->scanWindow);
    if (status != SUCCESS)
    {
        return status;
    }

    // Set scan parameters
    status = GapScan_setParam(SCAN_PARAM_RPT_FIELDS, &scanInitInfo->advReportFields);
    if(status != SUCCESS)
    {
        return(status);
    }
    status = GapScan_setParam(SCAN_PARAM_PRIM_PHYS, &scanInitInfo->scanPhys);
    if(status != SUCCESS)
    {
        return(status);
    }
    status = GapScan_setParam(SCAN_PARAM_FLT_POLICY, &scanInitInfo->fltPolicy);
    if(status != SUCCESS)
    {
        return(status);
    }
    status = GapScan_setParam(SCAN_PARAM_FLT_PDU_TYPE, &scanInitInfo->fltPduType);
    if(status != SUCCESS)
    {
        return(status);
    }
    status = GapScan_setParam(SCAN_PARAM_FLT_MIN_RSSI, &scanInitInfo->fltMinRssi);
    if(status != SUCCESS)
    {
        return(status);
    }
    status = GapScan_setParam(SCAN_PARAM_FLT_DISC_MODE, &scanInitInfo->fltDiscMode);
    if(status != SUCCESS)
    {
        return(status);
    }
    status = GapScan_setParam(SCAN_PARAM_FLT_DUP, &scanInitInfo->fltDup);

    return status;
}

bStatus_t BLEAppUtil_scanStart(const BLEAppUtil_ScanStart_t *scanStartInfo)
{
    return GapScan_enable(scanStartInfo->scanPeriod, scanStartInfo->scanDuration, scanStartInfo->maxNumReport);
}

bStatus_t BLEAppUtil_scanStop(void)
{
    return GapScan_disable();
}

bStatus_t BLEAppUtil_setConnParams(const BLEAppUtil_ConnParams_t *connParams)
{
    bStatus_t status;

    status = GapInit_setPhyParam(connParams->initPhys, INIT_PHYPARAM_SCAN_INTERVAL, connParams->scanInterval);
    if(status != SUCCESS)
    {
        return(status);
    }
    status = GapInit_setPhyParam(connParams->initPhys, INIT_PHYPARAM_SCAN_WINDOW, connParams->scanWindow);
    if(status != SUCCESS)
    {
        return(status);
    }
    status = GapInit_setPhyParam(connParams->initPhys, INIT_PHYPARAM_CONN_INT_MIN, connParams->minConnInterval);
    if(status != SUCCESS)
    {
        return(status);
    }
    status = GapInit_setPhyParam(connParams->initPhys, INIT_PHYPARAM_CONN_INT_MAX, connParams->maxConnInterval);
    if(status != SUCCESS)
    {
        return(status);
    }
    status = GapInit_setPhyParam(connParams->initPhys, INIT_PHYPARAM_CONN_LATENCY, connParams->connLatency);
    if(status != SUCCESS)
    {
        return(status);
    }
    status = GapInit_setPhyParam(connParams->initPhys, INIT_PHYPARAM_SUP_TIMEOUT, connParams->supTimeout);
    if(status != SUCCESS)
    {
        return(status);
    }

    return SUCCESS;
}

bStatus_t BLEAppUtil_connect(BLEAppUtil_ConnectParams_t *connParams)
{
    return GapInit_connect(connParams->peerAddrType, connParams->pPeerAddress,
                           connParams->phys, connParams->timeout);
}

bStatus_t BLEAppUtil_cancelConnect()
{
    return GapInit_cancelConnect();
}

bStatus_t BLEAppUtil_disconnect(uint16 connHandle)
{
  return GAP_TerminateLinkReq(connHandle, HCI_DISCONNECT_REMOTE_USER_TERM);
}

bStatus_t BLEAppUtil_setConnPhy(BLEAppUtil_ConnPhyParams_t *phyParams)
{
    // Set Phy Preference on the current connection. Apply the same value
    // for RX and TX. For more information, see the LE 2M PHY section in the User's Guide:
    // http://software-dl.ti.com/lprf/ble5stack-latest/
    return HCI_LE_SetPhyCmd(phyParams->connHandle, phyParams->allPhys, phyParams->txPhy, phyParams->rxPhy, phyParams->phyOpts);
}

bStatus_t BLEAppUtil_registerConnNotifHandler(uint16_t connHandle)
{
    return Gap_RegisterConnEventCb(BLEAppUtil_connEventCB, GAP_CB_REGISTER, GAP_CB_CONN_EVENT_ALL, connHandle);
}
bStatus_t BLEAppUtil_unRegisterConnNotifHandler()
{
    return Gap_RegisterConnEventCb(BLEAppUtil_connEventCB, GAP_CB_UNREGISTER, GAP_CB_CONN_EVENT_ALL, LINKDB_CONNHANDLE_INVALID);
}

/*********************************************************************
 * @fn      BLEAppUtil_paramUpdateRsp
 *
 * @brief   This function prepares an answer for connection parameters
 *          changing request, according to the decision made by the application,
 *          And then call for gap function with the response.
 *
 * @param   pReq - Pointer to master request
 *          accept - Application decision.
 *
 * @return  status
 */
bStatus_t BLEAppUtil_paramUpdateReq(gapUpdateLinkParamReq_t *pReq)
{
    bStatus_t status = FAILURE;
    linkDBInfo_t linkInfo;

    // Verify that the connection is active and send the param update request
    if (linkDB_GetInfo(pReq->connectionHandle, &linkInfo) == SUCCESS)
    {
        status = GAP_UpdateLinkParamReq(pReq);
    }

    return status;
}

/*********************************************************************
 * @fn      BLEAppUtil_paramUpdateRsp
 *
 * @brief   This function prepares an answer for connection parameters
 *          changing request, according to the decision made by the application,
 *          And then call for gap function with the response.
 *
 * @param   pReq - Pointer to central request
 *          accept - Application decision.
 *
 * @return  SUCCESS, INVALIDPARAMETER
 */
bStatus_t BLEAppUtil_paramUpdateRsp(gapUpdateLinkParamReqEvent_t *pReq, uint8 accept)
{
    bStatus_t status = SUCCESS;
    gapUpdateLinkParamReqReply_t rsp;

    rsp.connectionHandle = pReq->req.connectionHandle;
    rsp.signalIdentifier = pReq->req.signalIdentifier;

    if(accept)
    {
        rsp.intervalMin = pReq->req.intervalMin;
        rsp.intervalMax = pReq->req.intervalMax;
        rsp.connLatency = pReq->req.connLatency;
        rsp.connTimeout = pReq->req.connTimeout;
        rsp.accepted = TRUE;
    }
    else
    {
        rsp.accepted = FALSE;
    }
    status = GAP_UpdateLinkParamReqReply(&rsp);
    // Send Reply and return

    return (status);
}
