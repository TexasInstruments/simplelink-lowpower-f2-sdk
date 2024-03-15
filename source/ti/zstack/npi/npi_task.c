/******************************************************************************

 @file  npi_task.c

 @brief NPI is a TI RTOS Application Thread that provides a ! common
        Network Processor Interface framework.

 Group: WCS, LPC, BTS
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2015-2024, Texas Instruments Incorporated
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

// ****************************************************************************
// includes
// ****************************************************************************
#include "rom_jt_154.h"
#include "zcomdef.h"
#include <xdc/std.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/knl/Queue.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/drivers/dpl/ClockP.h>
#include <ti/sysbios/BIOS.h>
#include <string.h>

#include "mt.h"
#include "mt_rpc.h"
#include "npi_task.h"
#include "npi_data.h"
#include "npi_frame.h"
#include "npi_rxbuf.h"
#include "npi_tl.h"
#include "npi_client.h"

// ****************************************************************************
// defines
// ****************************************************************************

//! \brief Transport layer RX Event (ie. bytes received, RX ISR etc.)
#define NPITASK_TRANSPORT_RX_EVENT 0x0002

//! \brief Transmit Complete Event (likely associated with TX ISR etc.)
#define NPITASK_TRANSPORT_TX_DONE_EVENT 0x0004

//! \brief ASYNC Message Received Event (no framing bytes)
#define NPITASK_FRAME_RX_EVENT 0x0008

//! \brief A framed message buffer is ready to be sent to the transport layer.
#define NPITASK_TX_READY_EVENT 0x0010

#if defined(NPI_SREQRSP)
//! \brief ASYNC Message Received Event (no framing bytes)
#define NPITASK_SYNC_FRAME_RX_EVENT 0x0020

//! \brief A SYNC framed message buffer is ready to be sent to the transport layer.
#define NPITASK_SYNC_TX_READY_EVENT 0x0040

//! \brief SYNC REQ/RSP Watchdog Timer Duration (in ms)
#define NPITASK_WD_TIMEOUT 500
#endif // NPI_SREQRSP

//! \brief MRDY Received Event
#define NPITASK_MRDY_EVENT 0x0080

//! \brief Size of stack created for NPI RTOS task
#define NPITASK_STACK_SIZE 1024

//! \brief Task priority for NPI RTOS task
#define NPITASK_PRIORITY 4


// ****************************************************************************
// typedefs
// ****************************************************************************

//! \brief Queue record structure
//!
typedef struct NPI_QueueRec_t
{
    Queue_Elem _elem;
    NPIMSG_msg_t *npiMsg;
} NPI_QueueRec;


//*****************************************************************************
// globals
//*****************************************************************************

//! \brief ID for stack which will be sending NPI messages
//!
static uint32_t MTServiceTaskID = 0x0000;

//! \brief RTOS task structure for NPI task
//!
static Task_Struct npiTaskStruct;

//! \brief Allocated memory block for NPI task's stack
//!
Char npiTaskStack[NPITASK_STACK_SIZE];

//! \brief Handle for the ASYNC TX Queue
//!
static Queue_Handle npiTxQueue;

//! \brief Handle for the ASYNC RX Queue
//!
static Queue_Handle npiRxQueue;

#if defined(NPI_SREQRSP)
//! \brief Handle for the SYNC TX Queue
//!
static Queue_Handle npiSyncTxQueue;

//! \brief Handle for the SYNC RX Queue
//!
static Queue_Handle npiSyncRxQueue;

//! \brief Flag/Counter indicating a Synchronous REQ/RSP is currently being
//!        processed.
static int8_t syncTransactionInProgress = 0;

//! \brief Clock Struct for Sync REQ/RSP watchdog timer
//!
static ClockP_Struct syncReqRspWatchDogClkStruct;
static ClockP_Handle syncReqRspWatchDogClkHandle;
#endif // NPI_SREQRSP

//! \brief NPI thread Semaphore.
//!
Semaphore_Struct structSem;
Semaphore_Handle npiSemHandle;
uint8_t  npiServiceTaskId;
/* App service task events, set by the stack service task when sending a message */
uint32_t npiServiceTaskEvents;

static uint16_t TX_DONE_ISR_EVENT_FLAG = 0;
static uint16_t MRDY_ISR_EVENT_FLAG = 0;
static uint16_t TRANSPORT_RX_ISR_EVENT_FLAG = 0;

//! \brief Pointer to Application RX event callback function for optional
//!        rerouting of messages to application.
//!
static npiIncomingEventCBack_t incomingRXEventAppCBFunc = NULL;

//! \brief Type of rerouting for RX messages requested by Application
//!
static NPI_IncomingNPIEventRerouteType incomingRXReroute = NONE;


//! \brief Pointer to Application TX event callback function for optional
//!        rerouting of messages to application.
//!
static npiIncomingEventCBack_t incomingTXEventAppCBFunc = NULL;

//! \brief Type of rerouting for TX messages requested by Application
//!
static NPI_IncomingNPIEventRerouteType incomingTXReroute = NONE;

extern Semaphore_Handle npiInitializationMutexHandle;

//*****************************************************************************
// function prototypes
//*****************************************************************************

//! \brief      NPI main event processing loop.
//!
static void NPITask_process(void);

//! \brief Callback function registered with Transport Layer
//!
static void NPITask_transportRXCallBack(int size);

//! \brief Callback function registered with Transport Layer
//!
static void NPITask_transportTxDoneCallBack(int size);

//! \brief Callback function registered with Transport Layer
//!
static void NPITask_MRDYEventCB(int size);

//! \brief ASYNC TX Q Processing function.
//!
static void NPITask_ProcessTXQ(void);

#if defined(NPI_SREQRSP)
//! \brief SYNC TX Q Processing function.
//!
static void NPITask_ProcessSyncTXQ(void);

//! \brief SYNC RX Q Processing function.
//!
static void NPITask_processSyncRXQ(void);

//! \brief Sync REQ/RSP Watchdog Timer CB
//!
static void syncReqRspWatchDogTimeoutCB( UArg a0 );
#endif // NPI_SREQRSP

//! \brief ASYNC RX Q Processing function.
//!
static void NPITask_processRXQ(void);

//! \brief Callback function registered with Frame module to handle successful
//!        reception of message from host.
//!
static void NPITask_incomingFrameCB(uint8_t frameSize, uint8_t *pFrame,
                                    NPIMSG_Type msgType);

//! \brief Function to send message buffer to stack task.  Note, message buffer
//!        referenced by NPIMSG_msg_t is "unframed".
//!
static uint8_t NPITask_sendBufToStack( NPIMSG_msg_t *pMsg );

//! \brief Function to process incoming Msg from stack task.
//!
static void NPITask_processStackMsg(uint8_t *pMsg);

void NPITask_inititializeTask(void);


// -----------------------------------------------------------------------------
//! \brief      Initialization for the NPI Thread
//!
//! \return     void
// -----------------------------------------------------------------------------
void NPITask_inititializeTask(void)
{
    // create a Tx Queue instance
    npiTxQueue = Queue_create(NULL, NULL);
    // create an Rx Queue instance
    npiRxQueue = Queue_create(NULL, NULL);

#if defined(NPI_SREQRSP)
    // create an Sync RX Queue instance
    npiSyncRxQueue = Queue_create(NULL, NULL);
    // create an Sync TX Queue instance
    npiSyncTxQueue = Queue_create(NULL, NULL);

    // Create clock for SYNC REQ/RSP message watchdog
    Clock_Params clockParams;

    // Convert clockDuration in milliseconds to ticks.
    uint32_t clockTicks = NPITASK_WD_TIMEOUT * (1000 / Clock_tickPeriod);

    // Setup parameters.
    Clock_Params_init(&clockParams);

    // If period is 0, this is a one-shot timer.
    clockParams.period = 0;

    // Starts immediately after construction if true, otherwise wait for a call
    // to start.
    clockParams.startFlag = 0;

    // Initialize clock instance.
    Clock_construct(&syncReqRspWatchDogClkStruct, syncReqRspWatchDogTimeoutCB,
                    clockTicks, &clockParams);

    syncReqRspWatchDogClkHandle = ClockP_Handle(&syncReqRspWatchDogClkStruct);
#endif // NPI_SREQRSP

    Semaphore_Params semParams;
    Semaphore_Params_init(&semParams);
    Semaphore_construct(&structSem, 1, &semParams);
    npiSemHandle = Semaphore_handle(&structSem);

    npiServiceTaskId = OsalPort_registerTask(Task_self(), npiSemHandle, &npiServiceTaskEvents);
    // Pass NPI Task ID to NPI Client (MT) for communication
    NPIClient_saveNPITaskInfo(npiServiceTaskId);

    // Initialize Frame Module
    NPIFrame_initialize(&NPITask_incomingFrameCB);

    // Initialize Network Processor Interface (NPI) and Transport Layer
    NPITL_initTL( &NPITask_transportTxDoneCallBack,
                  &NPITask_transportRXCallBack,
                  &NPITask_MRDYEventCB );

}


// -----------------------------------------------------------------------------
//! \brief      NPI main event processing loop.
//!
//! \return     void
// -----------------------------------------------------------------------------
#define ITM_Port32(n) (*((volatile unsigned int *)(0xE0000000+4*n)))

static void NPITask_process(void)
{
  uint32_t key;
  uint8_t *pMsg;

    /* Forever loop */
    for (;; )
    {
        /* Wait for response message */
        Semaphore_pend(npiSemHandle, BIOS_WAIT_FOREVER);
        {
            // Capture the ISR events flags now within a critical section.
            // We do this to avoid possible race conditions where the ISR is
            // modifying the event mask while the task is read/writing it.
            key = OsalPort_enterCS();

            npiServiceTaskEvents = npiServiceTaskEvents | TX_DONE_ISR_EVENT_FLAG |
                             MRDY_ISR_EVENT_FLAG | TRANSPORT_RX_ISR_EVENT_FLAG;

            TX_DONE_ISR_EVENT_FLAG = 0;
            MRDY_ISR_EVENT_FLAG = 0;
            TRANSPORT_RX_ISR_EVENT_FLAG = 0;

            OsalPort_leaveCS(key);

            // MRDY event
            if (npiServiceTaskEvents & NPITASK_MRDY_EVENT)
            {
                npiServiceTaskEvents &= ~NPITASK_MRDY_EVENT;
#if (NPI_FLOW_CTRL == 1)
                NPITL_handleMrdyEvent();
#endif // NPI_FLOW_CTRL = 1
            }

#if defined(NPI_SREQRSP)
            // Something is ready to send to the Host
            if(npiServiceTaskEvents & NPITASK_SYNC_TX_READY_EVENT)
            {

                if (syncTransactionInProgress)
                {
                    // Prioritize Synchronous traffic
                    if ((!Queue_empty(npiSyncTxQueue)) && !NPITL_checkNpiBusy())
                    {
                        // Push the pending Sync RSP to the host.
                        NPITask_ProcessSyncTXQ();
                    }
                }
                else
                {
                    // Not expected
                }

                if (Queue_empty(npiSyncTxQueue))
                {
                    // If the Sync Q is empty now (and it should be) clear the
                    // event.
                    npiServiceTaskEvents &= ~NPITASK_SYNC_TX_READY_EVENT;
                }
                else
                {
                    // If the Sync Q is not empty now :
                    // - It means we're handling "stacked" SYNC REQ/RSP's
                    //   (which shouldn't be happening).
                    // - Preserve the event flag and repost on the semaphore.
                    Semaphore_post(npiSemHandle);
                }
            }
#endif // NPI_SREQRSP

            // Message Event
            if ((pMsg = OsalPort_msgReceive( npiServiceTaskId )) != NULL)
            {
                NPITask_processStackMsg( pMsg );
            }

#if defined(NPI_SREQRSP)
            // Synchronous Frame received from Host
            if(npiServiceTaskEvents & NPITASK_SYNC_FRAME_RX_EVENT)
            {
                // Process it
                NPITask_processSyncRXQ();

                if (Queue_empty(npiSyncRxQueue))
                {
                    // Q is empty, it's safe to clear the event flag.
                    npiServiceTaskEvents &= ~NPITASK_SYNC_FRAME_RX_EVENT;
                }
                else
                {
                    // Q is not empty, there's more to handle so preserve the
                    // flag and repost to the task semaphore.
                    Semaphore_post(npiSemHandle);
                }
            }
#endif // NPI_SREQRSP

            // An ASYNC message is ready to send to the Host
            if(npiServiceTaskEvents & NPITASK_TX_READY_EVENT)
            {
#if defined(NPI_SREQRSP)
                // Check for outstanding SYNC REQ/RSP transactions.  If so,
                // this ASYNC message must remain Q'd while we wait for the
                // SYNC RSP.
                if (syncTransactionInProgress  == 0)
                {
                    // No outstanding SYNC REQ/RSP transactions, process
                    // ASYNC messages.
#endif // NPI_SREQRSP
                    if ((!Queue_empty(npiTxQueue)) && !NPITL_checkNpiBusy())
                    {
                        // Push the pending Async Msg to the host.
                        NPITask_ProcessTXQ();
                    }
#if defined(NPI_SREQRSP)
                }
#endif // NPI_SREQRSP

                if (Queue_empty(npiTxQueue))
                {
                    // Q is empty, it's safe to clear the event flag.
                    npiServiceTaskEvents &= ~NPITASK_TX_READY_EVENT;
                }
                else
                {
                    // Q is not empty, there's more to handle so preserve the
                    // flag and repost to the task semaphore.
                    Semaphore_post(npiSemHandle);
                }
            }


            // The Transport Layer has received some bytes
            if(npiServiceTaskEvents & NPITASK_TRANSPORT_RX_EVENT)
            {
                // Call the packet/frame collector parser.  This function is
                // specific to the supported technology:
                // - HCI for BLE
                // - MT for ZigBee, TIMAC, RF4CE
                // - ? for your favorite technology
                NPIFrame_collectFrameData();

                if (NPIRxBuf_GetRxBufCount() == 0)
                {
                    // No additional bytes to collect, clear the flag.
                    npiServiceTaskEvents &= ~NPITASK_TRANSPORT_RX_EVENT;
                }
                else
                {
                    // Additional bytes to collect, preserve the flag and repost
                    // to the semaphore
                    Semaphore_post(npiSemHandle);
                }
            }

            // A complete frame (msg) has been received and is ready for handling
            if(npiServiceTaskEvents & NPITASK_FRAME_RX_EVENT)
            {
#if defined(NPI_SREQRSP)
                // Check for outstanding SYNC REQ/RSP transactions.  If so,
                // this ASYNC message must remain Q'd while we wait for the
                // SYNC RSP.
                if (syncTransactionInProgress == 0)
                {
#endif // NPI_SREQRSP
                    // Process the ASYNC message
                    NPITask_processRXQ();
                    if (Queue_empty(npiRxQueue))
                    {
                        // Q is empty, it's safe to clear the event flag.
                        npiServiceTaskEvents &= ~NPITASK_FRAME_RX_EVENT;
                    }
                    else
                    {
                        // Q is not empty, there's more to handle so preserve the
                        // flag and repost to the task semaphore.
                        Semaphore_post(npiSemHandle);
                    }
#if defined(NPI_SREQRSP)
                }
                else
                {
                    // Preserve the flag and repost to the task semaphore.
                    Semaphore_post(npiSemHandle);
                }
#endif // NPI_SREQRSP
            }

            // The last transmission to the host has completed.
            if(npiServiceTaskEvents & NPITASK_TRANSPORT_TX_DONE_EVENT)
            {
                // Current TX is done.
                npiServiceTaskEvents &= ~NPITASK_TRANSPORT_TX_DONE_EVENT;

#if defined(NPI_SREQRSP)
                if (!Queue_empty(npiSyncTxQueue))
                {
                    // There are pending SYNC RSP messages waiting to be sent
                    // to the host. Set the appropriate flag and post to
                    // the semaphore.
                    npiServiceTaskEvents |= NPITASK_SYNC_TX_READY_EVENT;
                    Semaphore_post(npiSemHandle);
                }
                else
                {
#endif // NPI_SREQRSP
                    if (!Queue_empty(npiTxQueue))
                    {
                        // There are pending ASYNC messages waiting to be sent
                        // to the host. Set the appropriate flag and post to
                        // the semaphore.
                        npiServiceTaskEvents |= NPITASK_TX_READY_EVENT;
                        Semaphore_post(npiSemHandle);
                    }
#if defined(NPI_SREQRSP)
                }
#endif // NPI_SREQRSP
            }
        }
    }
}

// -----------------------------------------------------------------------------
//! \brief      NPI Task function called from within NPITask_Fxn
//!
//! \return     void
// -----------------------------------------------------------------------------
void NPITask_task(void)
{
    // // Initialize application
    NPITask_inititializeTask();

    Semaphore_post(npiInitializationMutexHandle);

    // No return from TestProfile2 process
    NPITask_process();
}


// -----------------------------------------------------------------------------
// Exported Functions


// -----------------------------------------------------------------------------
//! \brief      NPI task entry point.
//!
//! \return     void
// -----------------------------------------------------------------------------
Void NPITask_Fxn(UArg a0, UArg a1)
{
    // potential race condition: stack task thread must be higher
    // priority than NPI task thread or stackTask_getMTServiceTaskID
    // will return an invalid task ID value (stack task must
    // initialized and registered with OsalPort first)
    extern uint8_t MTTask_getServiceTaskId(void);
    MTServiceTaskID = MTTask_getServiceTaskId();

    NPITask_task();
}

// -----------------------------------------------------------------------------
//! \brief      Task creation function for NPI
//!
//! \return     void
// -----------------------------------------------------------------------------
void NPITask_createTask(void)
{
    memset(&npiTaskStack, 0xDD, sizeof(npiTaskStack));

    // Configure and create the NPI task.
    Task_Params npiTaskParams;
    Task_Params_init(&npiTaskParams);
    npiTaskParams.stack = npiTaskStack;
    npiTaskParams.stackSize = NPITASK_STACK_SIZE;
    npiTaskParams.priority = NPITASK_PRIORITY;

    Task_construct(&npiTaskStruct, NPITask_Fxn, &npiTaskParams, NULL);
}

/*********************************************************************
 * @fn      NPITask_getServiceTaskId
 *
 * @param   none
 *
 * @return  task service ID for the NPI service task
 */
uint8_t NPITask_getServiceTaskId(void)
{
    return npiServiceTaskId;
}

// -----------------------------------------------------------------------------
//! \brief      Register callback function to reroute incoming (from host)
//!             NPI messages.
//!
//! \param[in]  appRxCB   Callback function.
//! \param[in]  reRouteType Type of re-routing requested
//!
//! \return     void
// -----------------------------------------------------------------------------
void NPITask_registerIncomingRXEventAppCB(npiIncomingEventCBack_t appRxCB,
                                          NPI_IncomingNPIEventRerouteType reRouteType)
{
    incomingRXEventAppCBFunc = appRxCB;
    incomingRXReroute = reRouteType;
}

// -----------------------------------------------------------------------------
//! \brief      Register callback function to reroute outgoing (from stack)
//!             NPI messages.
//!
//! \param[in]  appTxCB   Callback function.
//! \param[in]  reRouteType Type of re-routing requested
//!
//! \return     void
// -----------------------------------------------------------------------------
void NPITask_registerIncomingTXEventAppCB(npiIncomingEventCBack_t appTxCB,
                                          NPI_IncomingNPIEventRerouteType reRouteType)
{
    incomingTXEventAppCBFunc = appTxCB;
    incomingTXReroute = reRouteType;
}

// -----------------------------------------------------------------------------
//! \brief      API for application task to send a message to the Host.
//!             NOTE: It's assumed all message traffic to the stack will use
//!             other APIs/Interfaces.
//!
//! \param[in]  pMsg    Pointer to "unframed" message buffer.
//!
//! \return     void
// -----------------------------------------------------------------------------
void NPITask_sendToHost(uint8_t *pMsg)
{
    uint32_t key;
    NPI_QueueRec *recPtr;

    NPIMSG_msg_t *pNPIMsg = NPIFrame_frameMsg(pMsg);

    recPtr = OsalPort_malloc(sizeof(NPI_QueueRec));

    // Enter CS to prevent higher priority tasks
    // from also enqueuing msg at the same time
    key = OsalPort_enterCS();

    if ( pNPIMsg != NULL && recPtr != NULL )
    {
        recPtr->npiMsg = pNPIMsg;

        switch (pNPIMsg->msgType)
        {
            // Enqueue to appropriate NPI Task Q and post corresponding event.
#if defined(NPI_SREQRSP)
            case NPIMSG_Type_SYNCRSP:
            {
                Queue_enqueue(npiSyncTxQueue, &recPtr->_elem);
                npiServiceTaskEvents |= NPITASK_SYNC_TX_READY_EVENT;
                Semaphore_post(npiSemHandle);
                break;
            }
#endif // NPI_SREQRSP
            case NPIMSG_Type_ASYNC:
            {
                Queue_enqueue(npiTxQueue, &recPtr->_elem);
                npiServiceTaskEvents |= NPITASK_TX_READY_EVENT;
                Semaphore_post(npiSemHandle);
                break;
            }
            default:
            {
                //error
                break;
            }
        }
    }

    OsalPort_leaveCS(key);
}

// -----------------------------------------------------------------------------
// Utility functions

// -----------------------------------------------------------------------------
//! \brief      Forward the message buffer on to the Stack thread.
//!
//! \param[in]  pMsg       Pointer to a NPIMSG_msg_t container.
//!
//! \return     void
// -----------------------------------------------------------------------------
static uint8_t NPITask_sendBufToStack( NPIMSG_msg_t *pMsg )
{
    uint8_t msgStatus = 0;

    mtOSALSerialData_t  *pOsalMsg;

    uint8 *pReq = (uint8 *)pMsg->pBuf;

    /* Allocate memory for the MT message */
    pOsalMsg = (mtOSALSerialData_t *)OsalPort_msgAllocate( sizeof ( mtOSALSerialData_t ) );

    if (pOsalMsg)
    {
        /* Fill up what we can */
        pOsalMsg->hdr.event = CMD_SERIAL_MSG;

        pOsalMsg->msg = OsalPort_malloc( MT_RPC_FRAME_HDR_SZ + pReq[MT_RPC_POS_LEN] );

        if(pOsalMsg->msg) {

          memcpy(pOsalMsg->msg, pReq, (MT_RPC_FRAME_HDR_SZ + pReq[MT_RPC_POS_LEN]) );

          msgStatus = OsalPort_msgSend( MTServiceTaskID, (byte *)pOsalMsg );
        }

    }

    OsalPort_msgDeallocate(pMsg->pBuf);
    OsalPort_free(pMsg);

    return (msgStatus);
}



// -----------------------------------------------------------------------------
// "Processor" functions

// -----------------------------------------------------------------------------
//! \brief      Process Response from Stack
//!
//! \param[in]  pMsg    Pointer to unframed message buffer
//!
//! \return     void
// -----------------------------------------------------------------------------
static void NPITask_processStackMsg(uint8_t *pMsg)
{
    NPIMSG_msg_t *pNPIMsg;

    if(incomingTXEventAppCBFunc != NULL)
    {
        switch(incomingTXReroute)
        {
            case INTERCEPT:
            {
                // Pass the message along to the application and the leave
                // this function with an immediate return.
                // The message needs to be free by the callback.
                incomingTXEventAppCBFunc(pMsg);
                return;
            }
            case ECHO:
            {
                // Pass the message along to the application
                incomingTXEventAppCBFunc(pMsg);
                break;
            }
            case NONE:
            default:
            {
                break;
            }
        }
    }

    pNPIMsg = NPIFrame_frameMsg(pMsg);
    if(pNPIMsg != NULL)
    {
        NPI_QueueRec *recPtr;

        recPtr = OsalPort_malloc(sizeof(NPI_QueueRec));
        if(recPtr != NULL)
        {
            uint32_t key;

            // Enter CS to prevent higher priority tasks
            // from also enqueuing msg at the same time
            key = OsalPort_enterCS();

            recPtr->npiMsg = pNPIMsg;

            switch(pNPIMsg->msgType)
            {
                // Enqueue to appropriate NPI Task Q and post corresponding event.
#if defined(NPI_SREQRSP)
                case NPIMSG_Type_SYNCRSP:
                {
                    Queue_enqueue(npiSyncTxQueue, &recPtr->_elem);
                    npiServiceTaskEvents |= NPITASK_SYNC_TX_READY_EVENT;
                    Semaphore_post(npiSemHandle);
                    break;
                }
#endif // NPI_SREQRSP
                case NPIMSG_Type_ASYNC:
                {
                    Queue_enqueue(npiTxQueue, &recPtr->_elem);
                    npiServiceTaskEvents |= NPITASK_TX_READY_EVENT;
                    Semaphore_post(npiSemHandle);
                    break;
                }
                default:
                {
                    /* Fail - unsupported message type */
                    OsalPort_free(recPtr);
                    OsalPort_msgDeallocate(pNPIMsg->pBuf);
                    OsalPort_free(pNPIMsg);
                    break;
                }
            }

            OsalPort_leaveCS(key);
        }
        else
        {
            /* Fail - couldn't get queue record */
          OsalPort_msgDeallocate(pNPIMsg->pBuf);
          OsalPort_free(pNPIMsg);
        }
    }
}

// -----------------------------------------------------------------------------
//! \brief      Dequeue next message in the ASYNC TX Queue and send to serial
//!             interface.
//!
//! \return     void
// -----------------------------------------------------------------------------
static void NPITask_ProcessTXQ(void)
{
    uint32_t key;
    NPI_QueueRec *recPtr = NULL;

    // Processing of any TX Queue should only be done
    // in a critical section since any application
    // task can enqueue items freely
    key = OsalPort_enterCS();

    recPtr = Queue_dequeue(npiTxQueue);

    if (recPtr != NULL)
    {
        NPITL_writeTL(recPtr->npiMsg->pBuf, recPtr->npiMsg->pBufSize);

        //free the Queue record
        OsalPort_msgDeallocate(recPtr->npiMsg->pBuf);
        OsalPort_free(recPtr->npiMsg);
        OsalPort_free(recPtr);
    }

    OsalPort_leaveCS(key);
}

#if defined(NPI_SREQRSP)
// -----------------------------------------------------------------------------
//! \brief      Dequeue next message in the SYNC TX Queue and send to serial
//!             interface.
//!
//! \return     void
// -----------------------------------------------------------------------------
static void NPITask_ProcessSyncTXQ(void)
{
    uint32_t key;
    NPI_QueueRec *recPtr = NULL;

    // Processing of any TX Queue should only be done
    // in a critical section since any application
    // task can enqueue items freely
    key = OsalPort_enterCS();

    recPtr = Queue_dequeue(npiSyncTxQueue);

    if (recPtr != NULL)
    {
        NPITL_writeTL(recPtr->npiMsg->pBuf, recPtr->npiMsg->pBufSize);

        // Decrement the outstanding Sync REQ/RSP flag.
        syncTransactionInProgress--;

        // Stop watchdog clock.
        Clock_stop(syncReqRspWatchDogClkHandle);

        if (syncTransactionInProgress < 0)
        {
            // not expected!
            syncTransactionInProgress = 0;
        }

        OsalPort_msgDeallocate(recPtr->npiMsg->pBuf);
        OsalPort_free(recPtr->npiMsg);
        OsalPort_free(recPtr);
    }

    OsalPort_leaveCS(key);
}
#endif // NPI_SREQRSP

// -----------------------------------------------------------------------------
//! \brief      Dequeue next message in the RX Queue and process it.
//!
//! \return     void
// -----------------------------------------------------------------------------
static void NPITask_processRXQ(void)
{
    NPI_QueueRec *recPtr = NULL;

    recPtr = Queue_dequeue(npiRxQueue);

    if (recPtr != NULL)
    {
        if (incomingRXEventAppCBFunc != NULL)
        {
            switch (incomingRXReroute)
            {
                case ECHO:
                {
                    // send to both the app and the stack
                    // note: do not free recPtr->npiMsg inside incomingRXEventAppCBFunc()
                    // NPITask_sendBufToStack will free the message
                    incomingRXEventAppCBFunc((uint8_t *)recPtr->npiMsg);
                    NPITask_sendBufToStack(recPtr->npiMsg);
                    break;
                }

                case INTERCEPT:
                {
                    // send a copy only to the application
                    // npiMsg needs to be freed in the callback
                    incomingRXEventAppCBFunc((uint8_t *)recPtr->npiMsg);
                    break;
                }

                case NONE:
                {
                    NPITask_sendBufToStack(recPtr->npiMsg);
                    break;
                }
            }
        }
        else
        {
            // send to the stack
            NPITask_sendBufToStack(recPtr->npiMsg);
        }

        //free the Queue record
        OsalPort_free(recPtr);
        // DON'T free the referenced npiMsg container.  This will be free'd in the
        // stack task.
    }
}

#if defined(NPI_SREQRSP)
// -----------------------------------------------------------------------------
//! \brief      Dequeue next message in the RX Queue and process it.
//!
//! \return     void
// -----------------------------------------------------------------------------
static void NPITask_processSyncRXQ(void)
{
    NPI_QueueRec *recPtr = NULL;

    if (syncTransactionInProgress == 0)
    {
        recPtr = Queue_dequeue(npiSyncRxQueue);

        if (recPtr != NULL)
        {

            // Increment the outstanding Sync REQ/RSP flag.
            syncTransactionInProgress++;

            // Start the Sync REQ/RSP watchdog timer
            Clock_start(syncReqRspWatchDogClkHandle);

            if (incomingRXEventAppCBFunc != NULL)
            {
                switch (incomingRXReroute)
                {
                    case ECHO:
                    {
                        // send to both the app and the stack
                        // note: do not free recPtr->npiMsg inside incomingRXEventAppCBFunc()
                        // NPITask_sendBufToStack will free the message
                        incomingRXEventAppCBFunc((uint8_t *)recPtr->npiMsg);
                        NPITask_sendBufToStack(recPtr->npiMsg);
                        break;
                    }

                    case INTERCEPT:
                    {
                        // send a copy only to the application
                        // npiMsg needs to be freed in the callback
                        incomingRXEventAppCBFunc((uint8_t *)recPtr->npiMsg);
                        break;
                    }

                    case NONE:
                    {
                        NPITask_sendBufToStack(npiAppEntityID, recPtr->npiMsg);
                        break;
                    }
                }
            }
            else
            {
                // send to the stack
                NPITask_sendBufToStack(npiAppEntityID, recPtr->npiMsg);
            }

            //free the Queue record
            OsalPort_free(recPtr);
            // DON'T free the referenced npiMsg container.  This will be free'd in the
            // stack task.
        }
    }
}
#endif // NPI_SREQRSP

// -----------------------------------------------------------------------------
// Call Back Functions

// -----------------------------------------------------------------------------
//! \brief      Call back function for TX Done event from transport layer.
//!
//! \param[in]  size    Number of bytes transmitted.
//!
//! \return     void
// -----------------------------------------------------------------------------
static void NPITask_transportTxDoneCallBack(int size)
{


    // Post the event to the NPI task thread.
    TX_DONE_ISR_EVENT_FLAG = NPITASK_TRANSPORT_TX_DONE_EVENT;
    Semaphore_post(npiSemHandle);
}

// -----------------------------------------------------------------------------
//! \brief      Call back function to handle complete RX frame from Frame module.
//!
//! \param[in]  frameSize       Size of message frame.
//! \param[in]  pFrame          Pointer to message frame.
//! \param[in]  msgType         Message type, SYNC or ASYNC
//!
//! \return     void
// -----------------------------------------------------------------------------
static void NPITask_incomingFrameCB(uint8_t frameSize, uint8_t *pFrame,
                                    NPIMSG_Type msgType)
{
    NPI_QueueRec *recPtr = OsalPort_malloc(sizeof(NPI_QueueRec));

    // Allocate NPIMSG_msg_t container
    NPIMSG_msg_t *npiMsgPtr = OsalPort_malloc(sizeof(NPIMSG_msg_t));

    if ((recPtr != NULL) && (npiMsgPtr != NULL))
    {
        npiMsgPtr->pBuf = pFrame;
        npiMsgPtr->pBufSize = frameSize;
        recPtr->npiMsg = npiMsgPtr;

        switch (msgType)
        {

            // Enqueue to appropriate NPI Task Q and post corresponding event.
            case NPIMSG_Type_ASYNC:
            {
                recPtr->npiMsg->msgType = NPIMSG_Type_ASYNC;
                Queue_enqueue(npiRxQueue, &recPtr->_elem);
                npiServiceTaskEvents |= NPITASK_FRAME_RX_EVENT;
                Semaphore_post(npiSemHandle);
                break;
            }

#if defined(NPI_SREQRSP)
            case NPIMSG_Type_SYNCREQ:
            {
                recPtr->npiMsg->msgType = NPIMSG_Type_SYNCREQ;
                Queue_enqueue(npiSyncRxQueue, &recPtr->_elem);
                npiServiceTaskEvents |= NPITASK_SYNC_FRAME_RX_EVENT;
                Semaphore_post(npiSemHandle);
                break;
            }
#endif // NPI_SREQRSP

            default:
            {
                // undefined msgType
                OsalPort_msgDeallocate(pFrame);
                OsalPort_free(npiMsgPtr);
                OsalPort_free(recPtr);

                break;
            }
        }
    }
}

// -----------------------------------------------------------------------------
//! \brief      RX Callback provided to Transport Layer for RX Event (ie.Bytes
//!             received).
//!
//! \param[in]  size    Number of bytes received.
//!
//! \return     void
// -----------------------------------------------------------------------------
static void NPITask_transportRXCallBack(int size)
{
    // Check for overflow of RxBuf:
    // If the buffer has overflowed there is no way to safely recover. All
    // received bytes can be packet fragments so if a packet fragment is lost
    // the frame parser behavior becomes undefined. The only way to prevent
    // RxBuf overflow is to enable NPI_FLOW_CTRL.
    //
    // If NPI_FLOW_CTRL is not enabled then there is no way to for slave to
    // control the master transfer rate. With NPI_FLOW_CTRL the slave has SRDY
    // to use as a software flow control mechanism.
    // When using NPI_FLOW_CTRL make sure to increase NPI_TL_BUF_SIZE
    // to suit the NPI frame length that is expected to be received.
    if ( size < NPIRxBuf_GetRxBufAvail() )
    {
        NPIRxBuf_Read(size);
        TRANSPORT_RX_ISR_EVENT_FLAG = NPITASK_TRANSPORT_RX_EVENT;
        Semaphore_post(npiSemHandle);
    }
    else
    {
        // Trap here for pending buffer overflow. If NPI_FLOW_CTRL is
        // enabled, increase size of RxBuf to handle larger frames from host.
        for(;;);
    }
}

// -----------------------------------------------------------------------------
//! \brief      RX Callback provided to Transport Layer for MRDY Event
//!
//! \param[in]  size    N/A
//!
//! \return     void
// -----------------------------------------------------------------------------
static void NPITask_MRDYEventCB(int size)
{
    MRDY_ISR_EVENT_FLAG = NPITASK_MRDY_EVENT;
    Semaphore_post(npiSemHandle);
}


#if defined(NPI_SREQRSP)
// -----------------------------------------------------------------------------
//! \brief      Sync REQ/RSP Watchdog Timer CB
//!
//! \param[in]  a0      UArg
//!
//! \return     void
// -----------------------------------------------------------------------------
static void syncReqRspWatchDogTimeoutCB( UArg a0 )
{
    // Something has happened to the SYNC REQ we're waiting on.
    if (syncTransactionInProgress > 0)
    {
        // reduce the number of transactions outstanding
        syncTransactionInProgress--;

        // check if there are more pending SYNC REQ's
        if (!Queue_empty(npiSyncRxQueue))
        {
            npiServiceTaskEvents |= NPITASK_SYNC_FRAME_RX_EVENT;
        }

        // re-enter to Task event loop
        Semaphore_post(npiSemHandle);
    }
    else
    {
        // not expected
    }
}
#endif // NPI_SREQRSP

