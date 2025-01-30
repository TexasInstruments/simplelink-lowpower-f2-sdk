/******************************************************************************

 @file  npi_task.c

 @brief NPI is a TI RTOS Application Thread that provides a ! common
        Network Processor Interface framework.

 Group: WCS, LPC, BTS
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2015-2025, Texas Instruments Incorporated
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

#if defined(ICALL_EVENTS) && !defined(USE_DMM)
#include <ti/sysbios/knl/Event.h>
#else //!ICALL_EVENTS
#endif //ICALL_EVENTS
#include <pthread.h>
#include <ti/drivers/dpl/ClockP.h>
#include <ti/drivers/dpl/SemaphoreP.h>
#include <ti/drivers/utils/List.h>

#include <string.h>

#include "inc/npi_task.h"
#include "inc/npi_data.h"
#include "inc/npi_frame.h"
#include "inc/npi_rxbuf.h"
#include "inc/npi_tl.h"

#include "mt.h"

// ****************************************************************************
// defines
// ****************************************************************************

#if defined(ICALL_EVENTS) && !defined(USE_DMM)
#define NPITASK_ICALL_EVENT ICALL_MSG_EVENT_ID // Event_Id_31

//! \brief Transport layer RX Event (ie. bytes received, RX ISR etc.)
#define NPITASK_TRANSPORT_RX_EVENT Event_Id_00

//! \brief Transmit Complete Event (likely associated with TX ISR etc.)
#define NPITASK_TRANSPORT_TX_DONE_EVENT Event_Id_01

//! \brief ASYNC Message Received Event (no framing bytes)
#define NPITASK_FRAME_RX_EVENT Event_Id_02

//! \brief A framed message buffer is ready to be sent to the transport layer.
#define NPITASK_TX_READY_EVENT Event_Id_03

#if defined(NPI_SREQRSP)
//! \brief ASYNC Message Received Event (no framing bytes)
#define NPITASK_SYNC_FRAME_RX_EVENT Event_Id_04

//! \brief A SYNC framed message buffer is ready to be sent to the transport layer.
#define NPITASK_SYNC_TX_READY_EVENT Event_Id_05

//! \brief SYNC REQ/RSP Watchdog Timer Duration (in ms)
#define NPITASK_WD_TIMEOUT 500
#else // !NPI_SREQRSP
#define NPITASK_SYNC_FRAME_RX_EVENT Event_Id_NONE
#define NPITASK_SYNC_TX_READY_EVENT Event_Id_NONE
#endif // NPI_SREQRSP

//! \brief MRDY Received Event
#define NPITASK_MRDY_EVENT Event_Id_06

#define NPITASK_ALL_EVENTS (NPITASK_ICALL_EVENT | NPITASK_TRANSPORT_RX_EVENT | \
                            NPITASK_TRANSPORT_TX_DONE_EVENT | \
                            NPITASK_FRAME_RX_EVENT | NPITASK_TX_READY_EVENT | \
                            NPITASK_SYNC_FRAME_RX_EVENT | \
                            NPITASK_SYNC_TX_READY_EVENT | NPITASK_MRDY_EVENT)

#else //!ICALL_EVENTS
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

#endif //ICALL_EVENTS

//! \brief Size of stack created for NPI RTOS task
#ifndef Display_DISABLE_ALL
#ifdef __TI_COMPILER_VERSION__
#define NPITASK_STACK_SIZE 752
#else // !__TI_COMPILER_VERSION__
#define NPITASK_STACK_SIZE 650
#endif // __TI_COMPILER_VERSION__
#else // Display_DISABLE_ALL
#ifdef __TI_COMPILER_VERSION__
#define NPITASK_STACK_SIZE 752
#else // !__TI_COMPILER_VERSION__
#define NPITASK_STACK_SIZE 580
#endif // __TI_COMPILER_VERSION__
#endif // Display_DISABLE_ALL


//! \brief Task priority for NPI RTOS task
#ifndef NPITASK_PRIORITY
#define NPITASK_PRIORITY 2
#endif

// ****************************************************************************
// typedefs
// ****************************************************************************

//! \brief Queue record structure
//!
typedef struct NPI_QueueRec_t
{
//    Queue_Elem _elem;
    List_Elem _elem;
    NPIMSG_msg_t *npiMsg;
} NPI_QueueRec;


//*****************************************************************************
// globals
//*****************************************************************************

//! \brief ICall ID for stack which will be sending NPI messages
//!
static uint32_t stackServiceID = 0x0000;

//! \brief RTOS task structure for NPI task
//!
static pthread_t npiThread;

//! \brief Handle for the ASYNC TX Queue
//!
static List_List npiTxQueue;
//static Queue_Handle npiTxQueue;

//! \brief Handle for the ASYNC RX Queue
//!
static List_List npiRxQueue;
//static Queue_Handle npiRxQueue;

#if defined(NPI_SREQRSP)
//! \brief Handle for the SYNC TX Queue
//!
//static Queue_Handle npiSyncTxQueue;
static List_List npiSyncTxQueue;

//! \brief Handle for the SYNC RX Queue
//!
//static Queue_Handle npiSyncRxQueue;
static List_List npiSyncRxQueue;

//! \brief Flag/Counter indicating a Synchronous REQ/RSP is currently being
//!        processed.
static int8_t syncTransactionInProgress = 0;

//! \brief Clock Struct for Sync REQ/RSP watchdog timer
//!
static ClockP_Struct syncReqRspWatchDogClkStruct;
static ClockP_Handle syncReqRspWatchDogClkHandle;
#endif // NPI_SREQRSP

//! \brief NPI thread ICall Semaphore.
//!
#define FLE_DEBUG 1

#if defined(ICALL_EVENTS) && !defined(USE_DMM)
ICall_SyncHandle syncEvent;
#else //!ICALL_EVENTS
#ifdef FLE_DEBUG
#if defined(USE_ICALL) && !defined(USE_DMM)
static ICall_Semaphore npiSemHandle = NULL;
#else
SemaphoreP_Struct structnpiSem;
SemaphoreP_Handle npiSemHandle;
#endif
#else
static SemaphoreP_Struct structSem;
static SemaphoreP_Handle npiSemHandle;
#endif //FLE_DEBUG
#endif //ICALL_EVENTS

//! \brief NPI ICall Application Entity ID.
//!
#if defined(USE_ICALL) && !defined(USE_DMM)
ICall_EntityID npiAppEntityID = 0;
#else
uint8_t npiAppEntityID = 0;
#endif

#if !defined(ICALL_EVENTS) || defined(USE_DMM)
//! \brief Task pending events
//!
uint32_t NPITask_events = 0;

//! \brief Event flags for capturing Task-related events from ISR context
//!
static uint16_t TX_DONE_ISR_EVENT_FLAG = 0;
static uint16_t MRDY_ISR_EVENT_FLAG = 0;
static uint16_t TRANSPORT_RX_ISR_EVENT_FLAG = 0;
#endif //ICALL_EVENTS

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
//static npiIncomingEventCBack_t incomingTXEventAppCBFunc = NULL;
npiIncomingEventCBack_t incomingTXEventAppCBFunc = NULL;

//! \brief Type of rerouting for TX messages requested by Application
//!
//static NPI_IncomingNPIEventRerouteType incomingTXReroute = NONE;
NPI_IncomingNPIEventRerouteType incomingTXReroute = NONE;
#ifdef MAC_RADIO_USE_CSF
extern SemaphoreP_Handle nliSemHandle;
#endif
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
static void syncReqRspWatchDogTimeoutCB( uintptr_t a0 );
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
#ifdef OSAL_PORT2TIRTOS
static uint8_t NPITask_sendBufToStack(uint8_t appEntity, NPIMSG_msg_t *pMsg);
static uint_least16_t NPITask_processICallMsgEvent(uint8_t *pMsg,uint8_t src,uint8_t dest);
#else
static ICall_Errno NPITask_sendBufToStack(ICall_EntityID appEntity,NPIMSG_msg_t *msg);

//! \brief Function to handle incoming ICall Message Event
//!
static uint_least16_t NPITask_processICallMsgEvent(uint8_t *pMsg,
                                                   ICall_ServiceEnum src,
                                                   ICall_EntityID dest);
#endif

//! \brief Function to process incoming Msg from stack task.
//!
static void NPITask_processStackMsg(uint8_t *pMsg);


// -----------------------------------------------------------------------------
//! \brief      Initialization for the NPI Thread
//!
//! \return     void
// -----------------------------------------------------------------------------
static void NPITask_inititializeTask(void)
{
#if !defined(ICALL_EVENTS) || defined(USE_DMM)
    NPITask_events = 0;
#endif //ICALL_EVENTS

    // create a Tx Queue instance
    List_clearList(&npiTxQueue);
//     create an Rx Queue instance
    List_clearList(&npiRxQueue);

#if defined(NPI_SREQRSP)
    // create an Sync RX Queue instance
    List_clearList(&npiSyncTxQueue);
    // create an Sync TX Queue instance
    List_clearList(&npiSyncRxQueue);

    // Create clock for SYNC REQ/RSP message watchdog
    ClockP_Params clockParams;

    // Convert clockDuration in milliseconds to ticks.
    uint32_t clockTicks = NPITASK_WD_TIMEOUT * (1000 / ClockP_getSystemTickPeriod());

    // Setup parameters.
    ClockP_Params_init(&clockParams);

    // If period is 0, this is a one-shot timer.
    clockParams.period = 0;

    // Starts immediately after construction if true, otherwise wait for a call
    // to start.
    clockParams.startFlag = 0;

    // Initialize clock instance.
    ClockP_construct(&syncReqRspWatchDogClkStruct, syncReqRspWatchDogTimeoutCB,
                    clockTicks, &clockParams);

    syncReqRspWatchDogClkHandle = ClockP_handle(&syncReqRspWatchDogClkStruct);
#endif // NPI_SREQRSP

    /* Enroll the service that this stack represents */
#if defined(ICALL_EVENTS) && !defined(USE_DMM)
    ICall_enrollService ( ICALL_SERVICE_CLASS_NPI, NULL, &npiAppEntityID,
                          &syncEvent );
#else //!ICALL_EVENTS
#ifdef FLE_DEBUG
#if defined(USE_ICALL) && !defined(USE_DMM)
    ICall_enrollService( ICALL_SERVICE_CLASS_NPI, NULL, &npiAppEntityID, &npiSemHandle );
#else
    SemaphoreP_Params semParams;
    SemaphoreP_Params_init(&semParams);
    npiSemHandle = SemaphoreP_construct(&structnpiSem, 1, &semParams);

    /* register the NPI task */
    npiAppEntityID = OsalPort_registerTask(pthread_self(), npiSemHandle, &NPITask_events);
#endif
#else
    SemaphoreP_Params semParams;
    SemaphoreP_Params_init(&semParams);
    npiSemHandle = SemaphoreP_construct(&structSem, 1, &semParams);
#endif //FLE_DEBUG
#endif //ICALL_EVENTS
#ifdef MAC_RADIO_USE_CSF
    SemaphoreP_pend(nliSemHandle, SemaphoreP_WAIT_FOREVER);
#endif
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
#if defined(USE_ICALL) && !defined(USE_DMM)
    ICall_ServiceEnum stackid;
    ICall_EntityID dest;
#else
    uint8_t stackid;
    uint8_t dest;
#endif
    uint8_t *pMsg;
    MAP_ICall_CSState key;

    /* Forever loop */
    for (;; )
    {
        /* Wait for response message */

#if defined(ICALL_EVENTS) && !defined(USE_DMM)
        uint32_t NPITask_events;

        NPITask_events = Event_pend(syncEvent, Event_Id_NONE,
                                    NPITASK_ALL_EVENTS, BIOS_WAIT_FOREVER);
#else //!ICALL_EVENTS
        //if (ICall_wait(ICALL_TIMEOUT_FOREVER) == ICALL_ERRNO_SUCCESS)
        SemaphoreP_pend(npiSemHandle, SemaphoreP_WAIT_FOREVER);
#endif //ICALL_EVENTS
        {
            // Capture the ISR events flags now within a critical section.
            // We do this to avoid possible race conditions where the ISR is
            // modifying the event mask while the task is read/writing it.
            key = MAP_ICall_enterCriticalSection();

#if !defined(ICALL_EVENTS) || defined(USE_DMM)
            NPITask_events = NPITask_events | TX_DONE_ISR_EVENT_FLAG |
                             MRDY_ISR_EVENT_FLAG | TRANSPORT_RX_ISR_EVENT_FLAG;

            TX_DONE_ISR_EVENT_FLAG = 0;
            MRDY_ISR_EVENT_FLAG = 0;
            TRANSPORT_RX_ISR_EVENT_FLAG = 0;
#endif //ICALL_EVENTS

            MAP_ICall_leaveCriticalSection(key);

            // MRDY event
            if (NPITask_events & NPITASK_MRDY_EVENT)
            {
#if !defined(ICALL_EVENTS) || defined(USE_DMM)
                NPITask_events &= ~NPITASK_MRDY_EVENT;
#endif //ICALL_EVENTS
#if (NPI_FLOW_CTRL == 1)
                NPITL_handleMrdyEvent();
#endif // NPI_FLOW_CTRL = 1

            }

#if defined(NPI_SREQRSP)
            // Something is ready to send to the Host
            if(NPITask_events & NPITASK_SYNC_TX_READY_EVENT)
            {

                if (syncTransactionInProgress)
                {
                    // Prioritize Synchronous traffic
                    if ((!List_empty(&npiSyncTxQueue)) && !NPITL_checkNpiBusy())
                    {
                        // Push the pending Sync RSP to the host.
                        NPITask_ProcessSyncTXQ();
                    }
                }
                else
                {
                    // Not expected
                }

                if (List_empty(&npiSyncTxQueue))
                {
#if !defined(ICALL_EVENTS) || defined(USE_DMM)
                    // If the Sync Q is empty now (and it should be) clear the
                    // event.
                    NPITask_events &= ~NPITASK_SYNC_TX_READY_EVENT;
#endif //ICALL_EVENTS
                }
                else
                {
                    // If the Sync Q is not empty now :
                    // - It means we're handling "stacked" SYNC REQ/RSP's
                    //   (which shouldn't be happening).
                    // - Preserve the event flag and repost on the semaphore.
#if defined(ICALL_EVENTS) && !defined(USE_DMM)
                    Event_post(syncEvent, NPITASK_SYNC_TX_READY_EVENT);
#else //!ICALL_EVENTS
                    SemaphoreP_post(npiSemHandle);
#endif //ICALL_EVENTS
                }
            }
#endif // NPI_SREQRSP

            // ICall Message Event
#if defined(USE_ICALL) && !defined(USE_DMM)
            if (ICall_fetchServiceMsg(&stackid, &dest, (void * *) &pMsg)
                == ICALL_ERRNO_SUCCESS)
            {
#else
            if( (pMsg = OsalPort_msgReceive( npiAppEntityID )) != NULL)
            {
                stackid = 0;
                dest = npiAppEntityID;
#endif
                NPITask_processICallMsgEvent( pMsg, stackid, dest );
            }

#if defined(NPI_SREQRSP)
            // Synchronous Frame received from Host
            if(NPITask_events & NPITASK_SYNC_FRAME_RX_EVENT)
            {
                // Process it
                NPITask_processSyncRXQ();

                if (List_empty(&npiSyncRxQueue))
                {
#if !defined(ICALL_EVENTS) || defined(USE_DMM)
                    // Q is empty, it's safe to clear the event flag.
                    NPITask_events &= ~NPITASK_SYNC_FRAME_RX_EVENT;
#endif //ICALL_EVENTS
                }
                else
                {
#if defined(ICALL_EVENTS) && !defined(USE_DMM)
                    Event_post(syncEvent, NPITASK_SYNC_FRAME_RX_EVENT);
#else //!ICALL_EVENTS
                    // Q is not empty, there's more to handle so preserve the
                    // flag and repost to the task semaphore.
                    SemaphoreP_post(npiSemHandle);
#endif //ICALL_EVENTS
                }
            }
#endif // NPI_SREQRSP

            // An ASYNC message is ready to send to the Host
            if(NPITask_events & NPITASK_TX_READY_EVENT)
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
                    if ((!List_empty(&npiTxQueue)) && !NPITL_checkNpiBusy())
                    {
                        // Push the pending Async Msg to the host.
                        NPITask_ProcessTXQ();
                    }
#if defined(NPI_SREQRSP)
                }
#endif // NPI_SREQRSP

                if (List_empty(&npiTxQueue))
                {
#if !defined(ICALL_EVENTS) || defined(USE_DMM)
                    // Q is empty, it's safe to clear the event flag.
                    NPITask_events &= ~NPITASK_TX_READY_EVENT;
#endif //ICALL_EVENTS
                }
                else
                {
                    // Q is not empty, there's more to handle so preserve the
                    // flag and repost to the task semaphore.
#if defined(ICALL_EVENTS) && !defined(USE_DMM)
                    Event_post(syncEvent, NPITASK_TX_READY_EVENT);
#else //!ICALL_EVENTS
                    SemaphoreP_post(npiSemHandle);
#endif //ICALL_EVENTS
                }
            }


            // The Transport Layer has received some bytes
            if(NPITask_events & NPITASK_TRANSPORT_RX_EVENT)
            {
                // Call the packet/frame collector parser.  This function is
                // specific to the supported technology:
                // - HCI for BLE
                // - MT for ZigBee, TIMAC, RF4CE
                // - ? for your favorite technology
                NPIFrame_collectFrameData();

                if (NPIRxBuf_GetRxBufCount() == 0)
                {
#if !defined(ICALL_EVENTS) || defined(USE_DMM)
                    // No additional bytes to collect, clear the flag.
                    NPITask_events &= ~NPITASK_TRANSPORT_RX_EVENT;
#endif //ICALL_EVENTS
                }
                else
                {
                    // Additional bytes to collect, preserve the flag and repost
                    // to the semaphore
#if defined(ICALL_EVENTS) && !defined(USE_DMM)
                    Event_post(syncEvent, NPITASK_TRANSPORT_RX_EVENT);
#else //!ICALL_EVENTS
                    SemaphoreP_post(npiSemHandle);
#endif //ICALL_EVENTS
                }
            }

            // A complete frame (msg) has been received and is ready for handling
            if(NPITask_events & NPITASK_FRAME_RX_EVENT)
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
                    if (List_empty(&npiRxQueue))
                    {
#if !defined(ICALL_EVENTS) || defined(USE_DMM)
                        // Q is empty, it's safe to clear the event flag.
                        NPITask_events &= ~NPITASK_FRAME_RX_EVENT;
#endif //ICALL_EVENTS
                    }
                    else
                    {
                        // Q is not empty, there's more to handle so preserve the
                        // flag and repost to the task semaphore.
#if defined(ICALL_EVENTS) && !defined(USE_DMM)
                        Event_post(syncEvent, NPITASK_FRAME_RX_EVENT);
#else //!ICALL_EVENTS
                        SemaphoreP_post(npiSemHandle);
#endif //ICALL_EVENTS
                    }
#if defined(NPI_SREQRSP)
                }
                else
                {
#if defined(ICALL_EVENTS) && !defined(USE_DMM)
                    Event_post(syncEvent, NPITASK_FRAME_RX_EVENT);
#else //!ICALL_EVENTS
                    // Preserve the flag and repost to the task semaphore.
                    SemaphoreP_post(npiSemHandle);
#endif //ICALL_EVENTS
                }
#endif // NPI_SREQRSP
            }

            // The last transmission to the host has completed.
            if(NPITask_events & NPITASK_TRANSPORT_TX_DONE_EVENT)
            {
                // Current TX is done.
                NPITask_events &= ~NPITASK_TRANSPORT_TX_DONE_EVENT;

#if defined(NPI_SREQRSP)
                if (!List_empty(&npiSyncTxQueue))
                {
                    // There are pending SYNC RSP messages waiting to be sent
                    // to the host. Set the appropriate flag and post to
                    // the semaphore.
#if defined(ICALL_EVENTS) && !defined(USE_DMM)
                    Event_post(syncEvent, NPITASK_SYNC_TX_READY_EVENT);
#else //!ICALL_EVENTS
                    NPITask_events |= NPITASK_SYNC_TX_READY_EVENT;
                    SemaphoreP_post(npiSemHandle);
#endif //ICALL_EVENTS
                }
                else
                {
#endif // NPI_SREQRSP
                    if (!List_empty(&npiTxQueue))
                    {
                        // There are pending ASYNC messages waiting to be sent
                        // to the host. Set the appropriate flag and post to
                        // the semaphore.
#if defined(ICALL_EVENTS) && !defined(USE_DMM)
                        Event_post(syncEvent, NPITASK_TX_READY_EVENT);
#else //!ICALL_EVENTS
                        NPITask_events |= NPITASK_TX_READY_EVENT;
                        SemaphoreP_post(npiSemHandle);
#endif //ICALL_EVENTS
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
    // Initialize application
    NPITask_inititializeTask();

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
static void *NPITask_Fxn(void *arg0)
{
    NPITask_task();
    return NULL;
}

// -----------------------------------------------------------------------------
//! \brief      Task creation function for NPI
//!
//! \return     void
// -----------------------------------------------------------------------------
void NPITask_createTask(uint32_t stackID)
{
    // Set stackID for future ICall Messaging
    stackServiceID = stackID;
    pthread_attr_t      attrs;
    struct sched_param  priParam;
    int                 retc;

    /* Initialize the attributes structure with default values */
    pthread_attr_init(&attrs);

    /* Set priority, detach state, and stack size attributes */
    priParam.sched_priority = NPITASK_PRIORITY;
    retc = pthread_attr_setschedparam(&attrs, &priParam);
    retc |= pthread_attr_setdetachstate(&attrs, PTHREAD_CREATE_DETACHED);
    retc |= pthread_attr_setstacksize(&attrs, NPITASK_STACK_SIZE);
    if (retc != 0) {
        /* failed to set attributes */
        while (1) {}
    }
    retc = pthread_create(&npiThread, &attrs, NPITask_Fxn, NULL);
    if (retc != 0) {
        /* pthread_create() failed */
        while (1) {}
    }
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
//!             other (ICALL) APIs/Interfaces.
//!
//! \param[in]  pMsg    Pointer to "unframed" message buffer.
//!
//! \return     void
// -----------------------------------------------------------------------------
void NPITask_sendToHost(uint8_t *pMsg)
{
    MAP_ICall_CSState key;
    NPI_QueueRec *recPtr;

    NPIMSG_msg_t *pNPIMsg = NPIFrame_frameMsg(pMsg);

    recPtr = MAP_ICall_malloc(sizeof(NPI_QueueRec));

    // Enter CS to prevent higher priority tasks
    // from also enqueuing msg at the same time
    key = MAP_ICall_enterCriticalSection();

    if ( pNPIMsg != NULL && recPtr != NULL )
    {
        recPtr->npiMsg = pNPIMsg;

        switch (pNPIMsg->msgType)
        {
            // Enqueue to appropriate NPI Task Q and post corresponding event.
#if defined(NPI_SREQRSP)
            case NPIMSG_Type_SYNCRSP:
            {
                List_put(&npiSyncTxQueue, (List_Elem *)recPtr);
#if defined(ICALL_EVENTS) && !defined(USE_DMM)
                Event_post(syncevent, NPITASK_SYNC_TX_READY_EVENT);
#else //!ICALL_EVENTS
                NPITask_events |= NPITASK_SYNC_TX_READY_EVENT;
                SemaphoreP_post(npiSemHandle);
#endif //ICALL_EVENT
                break;
            }
#endif // NPI_SREQRSP
            case NPIMSG_Type_ASYNC:
            {
                List_put(&npiTxQueue, (List_Elem *)recPtr);
#if defined(ICALL_EVENTS) && !defined(USE_DMM)
                Event_post(syncEvent, NPITASK_TX_READY_EVENT);
#else //!ICALL_EVENTS
                NPITask_events |= NPITASK_TX_READY_EVENT;
                SemaphoreP_post(npiSemHandle);
#endif //ICALL_EVENTS
                break;
            }
            default:
            {
                //error
                break;
            }
        }
    }

    MAP_ICall_leaveCriticalSection(key);
}

// -----------------------------------------------------------------------------
// Event Handlers

// -----------------------------------------------------------------------------
//! \brief      Process an ICall message from the Stack
//!
//! \param[in]  pMsg  pointer to an unframed message buffer
//! \param[in]  pMsg  Service ID for  message source
//! \param[in]  pMsg  Entity ID for message destination
//!
//! \return     void
// -----------------------------------------------------------------------------
#ifdef OSAL_PORT2TIRTOS
static uint_least16_t NPITask_processICallMsgEvent(uint8_t *pMsg,
                                                   uint8_t src,
                                                   uint8_t dest)
{
    (void) src;
    (void) dest;

    // Message received from the Stack.
    NPITask_processStackMsg(pMsg);
#else
static uint_least16_t NPITask_processICallMsgEvent(uint8_t *pMsg,
                                                   ICall_ServiceEnum src,
                                                   ICall_EntityID dest)
{
    if (dest == npiAppEntityID)
    {
        // Message received from the Stack.
        NPITask_processStackMsg(pMsg);
    }
#endif

    return(0);
}


// -----------------------------------------------------------------------------
// Utility functions

// -----------------------------------------------------------------------------
//! \brief      Forward the message buffer on to the Stack thread.
//!
//! \param[in]  appEntity    ICall Entity ID of the caller.
//! \param[in]  pMtBuf       Pointer to a NPIMSG_msg_t container.
//!
//! \return     void
// -----------------------------------------------------------------------------
#ifdef OSAL_PORT2TIRTOS
static uint8_t NPITask_sendBufToStack(uint8_t appEntity,
                                          NPIMSG_msg_t *pMsg)
{
    uint8_t errno;

    (void) appEntity;
    errno = OsalPort_msgSend(stackServiceID,pMsg->pBuf);
#else
static ICall_Errno NPITask_sendBufToStack(ICall_EntityID appEntity,
                                          NPIMSG_msg_t *pMsg)
{

    ICall_Errno errno = ICALL_ERRNO_SUCCESS;

    // Send the message
    errno = ICall_sendServiceMsg(appEntity, stackServiceID,
                                 ICALL_MSG_FORMAT_KEEP, pMsg->pBuf);
#endif

    MAP_ICall_freeMsg(pMsg->pBuf);
    MAP_ICall_free(pMsg);

    return (errno);
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

        recPtr = MAP_ICall_malloc(sizeof(NPI_QueueRec));
        if(recPtr != NULL)
        {
            MAP_ICall_CSState key;

            // Enter CS to prevent higher priority tasks
            // from also enqueuing msg at the same time
            key = MAP_ICall_enterCriticalSection();

            recPtr->npiMsg = pNPIMsg;

            switch(pNPIMsg->msgType)
            {
                // Enqueue to appropriate NPI Task Q and post corresponding event.
#if defined(NPI_SREQRSP)
                case NPIMSG_Type_SYNCRSP:
                {
                    List_put(&npiSyncTxQueue, (List_Elem *)recPtr);
#if defined(ICALL_EVENTS) && !defined(USE_DMM)
                    Event_post(syncEvent, NPITASK_SYNC_TX_READY_EVENT);
#else //!ICALL_EVENTS
                    NPITask_events |= NPITASK_SYNC_TX_READY_EVENT;
                    SemaphoreP_post(npiSemHandle);
#endif //ICALL_EVENTS
                    break;
                }
#endif // NPI_SREQRSP
                case NPIMSG_Type_ASYNC:
                {
                    List_put(&npiTxQueue, (List_Elem *)recPtr);
#if defined(ICALL_EVENTS) && !defined(USE_DMM)
                    Event_post(syncEvent, NPITASK_TX_READY_EVENT);
#else //!ICALL_EVENTS
                    NPITask_events |= NPITASK_TX_READY_EVENT;
                    SemaphoreP_post(npiSemHandle);
#endif //ICALL_EVENTS
                    break;
                }
                default:
                {
                    /* Fail - unsupported message type */
                    MAP_ICall_free(recPtr);
                    MAP_ICall_freeMsg(pNPIMsg->pBuf);
                    MAP_ICall_free(pNPIMsg);
                    break;
                }
            }

            MAP_ICall_leaveCriticalSection(key);
        }
        else
        {
            /* Fail - couldn't get queue record */
            MAP_ICall_freeMsg(pNPIMsg->pBuf);
            MAP_ICall_free(pNPIMsg);
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
    MAP_ICall_CSState key;
    NPI_QueueRec *recPtr = NULL;

    // Processing of any TX Queue should only be done
    // in a critical section since any application
    // task can enqueue items freely
    key = MAP_ICall_enterCriticalSection();

    recPtr = (NPI_QueueRec*) List_get(&npiTxQueue);

    if (recPtr != NULL)
    {
        NPITL_writeTL(recPtr->npiMsg->pBuf, recPtr->npiMsg->pBufSize);

        List_remove(&npiTxQueue, (List_Elem* )recPtr);

        //free the Queue record
        MAP_ICall_freeMsg(recPtr->npiMsg->pBuf);
        MAP_ICall_free(recPtr->npiMsg);
        MAP_ICall_free(recPtr);
    }

    MAP_ICall_leaveCriticalSection(key);
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
    MAP_ICall_CSState key;
    NPI_QueueRec *recPtr = NULL;

    // Processing of any TX Queue should only be done
    // in a critical section since any application
    // task can enqueue items freely
    key = MAP_ICall_enterCriticalSection();

    recPtr = (NPI_QueueRec* ) List_get(&npiSyncTxQueue);

    if (recPtr != NULL)
    {
        NPITL_writeTL(recPtr->npiMsg->pBuf, recPtr->npiMsg->pBufSize);

        // Decrement the outstanding Sync REQ/RSP flag.
        syncTransactionInProgress--;

        // Stop watchdog clock.
        ClockP_stop(syncReqRspWatchDogClkHandle);

        if (syncTransactionInProgress < 0)
        {
            // not expected!
            syncTransactionInProgress = 0;
        }

        List_remove(&npiSyncTxQueue, (List_Elem* )recPtr);

        MAP_ICall_freeMsg(recPtr->npiMsg->pBuf);
        MAP_ICall_free(recPtr->npiMsg);
        MAP_ICall_free(recPtr);
    }

    MAP_ICall_leaveCriticalSection(key);
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

    recPtr = (NPI_QueueRec*) List_get(&npiRxQueue);

    if (recPtr != NULL)
    {
        if (incomingRXEventAppCBFunc != NULL)
        {
            switch (incomingRXReroute)
            {
                case ECHO:
                {
                    // send to stack and a copy to the application
                    incomingRXEventAppCBFunc((uint8_t *)recPtr->npiMsg);
                    NPITask_sendBufToStack(npiAppEntityID, recPtr->npiMsg);
                    break;
                }

                case INTERCEPT:
                {
                    // send a copy only to the application
                    // npiMsg need to be free in the callback
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
            // send to stack and a copy to the application
            NPITask_sendBufToStack(npiAppEntityID, recPtr->npiMsg);
        }

        //free the Queue record
        List_remove(&npiRxQueue, (List_Elem* )recPtr);
        MAP_ICall_free(recPtr);

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
        recPtr = (NPI_QueueRec* ) List_get(&npiSyncRxQueue);

        if (recPtr != NULL)
        {

            // Increment the outstanding Sync REQ/RSP flag.
            syncTransactionInProgress++;

            // Start the Sync REQ/RSP watchdog timer
            ClockP_start(syncReqRspWatchDogClkHandle);

            if (incomingRXEventAppCBFunc != NULL)
            {
                switch (incomingRXReroute)
                {
                    case ECHO:
                    {
                        // send to stack and a copy to the application
                        incomingRXEventAppCBFunc(recPtr->npiMsg->pBuf);
                        NPITask_sendBufToStack(npiAppEntityID, recPtr->npiMsg);
                        break;
                    }

                    case INTERCEPT:
                    {
                        // send a copy only to the application
                        incomingRXEventAppCBFunc(recPtr->npiMsg->pBuf);
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
                // send to stack and a copy to the application
                NPITask_sendBufToStack(npiAppEntityID, recPtr->npiMsg);
            }

            //free the Queue record
            List_remove(&npiSyncRxQueue, (List_Elem* )recPtr);
            MAP_ICall_free(recPtr);
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
#if defined(ICALL_EVENTS) && !defined(USE_DMM)
    Event_post(syncEvent, NPITASK_TRANSPORT_TX_DONE_EVENT);
#else //!ICALL_EVENTS
    TX_DONE_ISR_EVENT_FLAG = NPITASK_TRANSPORT_TX_DONE_EVENT;
    SemaphoreP_post(npiSemHandle);
#endif //ICALL_EVENTS
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
    NPI_QueueRec *recPtr = MAP_ICall_malloc(sizeof(NPI_QueueRec));

    // Allocate NPIMSG_msg_t container
    NPIMSG_msg_t *npiMsgPtr = MAP_ICall_malloc(sizeof(NPIMSG_msg_t));

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
                List_put(&npiRxQueue, (List_Elem *)recPtr);
#if defined(ICALL_EVENTS) && !defined(USE_DMM)
                Event_post(syncEvent, NPITASK_FRAME_RX_EVENT);
#else //!ICALL_EVENTS
                NPITask_events |= NPITASK_FRAME_RX_EVENT;
                SemaphoreP_post(npiSemHandle);
#endif //ICALL_EVENTS

                break;
            }

#if defined(NPI_SREQRSP)
            case NPIMSG_Type_SYNCREQ:
            {
                recPtr->npiMsg->msgType = NPIMSG_Type_SYNCREQ;
                List_put(&npiSyncRxQueue, (List_Elem *)recPtr);
#if defined(ICALL_EVENTS) && !defined(USE_DMM)
                Event_post(syncEvent, NPITASK_SYNC_FRAME_RX_EVENT);
#else //!ICALL_EVENTS
                NPITask_events |= NPITASK_SYNC_FRAME_RX_EVENT;
                SemaphoreP_post(npiSemHandle);
#endif //ICALL_EVENTS

                break;
            }
#endif // NPI_SREQRSP

            default:
            {
                // undefined msgType
                MAP_ICall_freeMsg(pFrame);
                MAP_ICall_free(npiMsgPtr);
                MAP_ICall_free(recPtr);

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
    // If NPI_FLOW_CTRL is not enabled then there is no way to for the Peripheral to
    // control the Controller transfer rate. With NPI_FLOW_CTRL the Peripheral has SRDY
    // to use as a software flow control mechanism.
    // When using NPI_FLOW_CTRL make sure to increase NPI_TL_BUF_SIZE
    // to suit the NPI frame length that is expected to be received.
    if ( size < NPIRxBuf_GetRxBufAvail() )
    {
        NPIRxBuf_Read(size);
#if defined(ICALL_EVENTS) && !defined(USE_DMM)
        Event_post(syncEvent, NPITASK_TRANSPORT_RX_EVENT);
#else //!ICALL_EVENTS
        TRANSPORT_RX_ISR_EVENT_FLAG = NPITASK_TRANSPORT_RX_EVENT;
        SemaphoreP_post(npiSemHandle);
#endif //ICALL_EVENTS
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
#if defined(ICALL_EVENTS) && !defined(USE_DMM)
    Event_post(syncEvent, NPITASK_MRDY_EVENT);
#else //!ICALL_EVENTS
    MRDY_ISR_EVENT_FLAG = NPITASK_MRDY_EVENT;
    SemaphoreP_post(npiSemHandle);
#endif //ICALL_EVENTS
}


#if defined(NPI_SREQRSP)
// -----------------------------------------------------------------------------
//! \brief      Sync REQ/RSP Watchdog Timer CB
//!
//! \param[in]  a0      uintptr_t
//!
//! \return     void
// -----------------------------------------------------------------------------
static void syncReqRspWatchDogTimeoutCB( uintptr_t a0 )
{
    // Something has happened to the SYNC REQ we're waiting on.
    if (syncTransactionInProgress > 0)
    {
        // reduce the number of transactions outstanding
        syncTransactionInProgress--;

        // check if there are more pending SYNC REQ's
        if (!List_empty(&npiSyncRxQueue))
        {
#if defined(ICALL_EVENTS) && !defined(USE_DMM)
            Event_post(syncEvent, NPITASK_SYNC_FRAME_RX_EVENT);
#else //!ICALL_EVENTS
            NPITask_events |= NPITASK_SYNC_FRAME_RX_EVENT;
#endif //ICALL_EVENTS
        }

#if !defined(ICALL_EVENTS) || defined(USE_DMM)
        // re-enter to Task event loop
        SemaphoreP_post(npiSemHandle);
#endif //ICALL_EVENTS
    }
    else
    {
        // not expected
    }
}
#endif // NPI_SREQRSP

