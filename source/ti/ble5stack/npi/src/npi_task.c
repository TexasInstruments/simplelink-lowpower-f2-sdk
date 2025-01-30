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

#include <xdc/std.h>
#ifdef ICALL_EVENTS
#include <ti/sysbios/knl/Event.h>
#else //!ICALL_EVENTS
#include <ti/sysbios/knl/Semaphore.h>
#endif //ICALL_EVENTS
#include <ti/sysbios/knl/Queue.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/BIOS.h>

#include <string.h>
#include <semaphore.h>
#include <mqueue.h>

#include "icall.h"

#include "inc/npi_task.h"
#include "inc/npi_data.h"
#include "inc/npi_frame.h"
#include "inc/npi_rxbuf.h"
#include "inc/npi_tl.h"

// ****************************************************************************
// defines
// ****************************************************************************

#if defined(NPI_SREQRSP)

#ifdef ICALL_EVENTS
//! \brief SYNC REQ/RSP Watchdog Timer Duration (in ms)
#define NPITASK_WD_TIMEOUT 500
#else //!ICALL_EVENTS
//! \brief SYNC REQ/RSP Watchdog Timer Duration (in ms)
#define NPITASK_WD_TIMEOUT 500
#endif // NPI_SREQRSP
#endif //ICALL_EVENTS

#if defined(CC26X2) || defined(CC13X2) || defined(CC13X2P)
#define NPITASK_STACK_SIZE (180*8)     /* in order to optimize memory, this value should be a multiple of 8 bytes */
#else // !CC26X2 && !CC13X2
//! \brief Size of stack created for NPI RTOS task
#ifndef Display_DISABLE_ALL
#ifdef __TI_COMPILER_VERSION__
#define NPITASK_STACK_SIZE 752      /* in order to optimize memory, this value should be a multiple of 8 bytes */
#else // !__TI_COMPILER_VERSION__
#define NPITASK_STACK_SIZE 656      /* in order to optimize memory, this value should be a multiple of 8 bytes */
#endif // __TI_COMPILER_VERSION__
#else // Display_DISABLE_ALL
#ifdef __TI_COMPILER_VERSION__
#define NPITASK_STACK_SIZE 752      /* in order to optimize memory, this value should be a multiple of 8 bytes */
#else // !__TI_COMPILER_VERSION__
#define NPITASK_STACK_SIZE 608      /* in order to optimize memory, this value should be a multiple of 8 bytes */
#endif // __TI_COMPILER_VERSION__
#endif // Display_DISABLE_ALL
#endif // CC26X2 || CC13X2

//! \brief Task priority for NPI RTOS task
#define NPITASK_PRIORITY 2

// ****************************************************************************
// defines
// ****************************************************************************
#define NPI_TX_QUEUE_SIZE                             16
#define NPI_RX_QUEUE_SIZE                             16
#define NPI_SYNC_TX_QUEUE_SIZE                        16
#define NPI_SYNC_RX_QUEUE_SIZE                        16

#define NPITASK_MRDY_FLAG                             TRUE

// ****************************************************************************
// typedefs
// ****************************************************************************

//*****************************************************************************
// globals
//*****************************************************************************

typedef struct
{
    //! \brief ICall ID for stack which will be sending NPI messages
    //!
    uint32_t stackServiceID;

    //! \brief RTOS task handle for NPI task
    pthread_t taskHandle;

    uint8_t *pTaskStack;

    Task_Struct TaskStruct;

    //! \brief Handle for the ASYNC TX Queue
    mqd_t      npiTxQueue;

    //! \brief Handle for the ASYNC RX Queue
    mqd_t      npiRxQueue;
//    mq_attr    txQueueAttr;
//    mq_attr    rxQueueAttr;
#if defined(NPI_SREQRSP)
    //! \brief Handle for the SYNC TX Queue
    mqd_t      npiSyncTxQueue;

    //! \brief Handle for the SYNC RX Queue
    mqd_t      npiSyncRxQueue;
#endif
    //! \brief Pointer to last tx message.  This is free'd once confirmation is
    //!        is received that the buffer has been transmitted
    //!        (ie. NPITASK_TRANSPORT_TX_DONE_EVENT)
    //!
    uint8_t *lastQueuedTxMsg;
    uint16_t lastQueuedTxMsgSize;

    //! \brief Handle for the semaphore
    sem_t *pNpiSem;
    //! \brief NPI ICall Application Entity ID.
    //!
    ICall_EntityID npiAppEntityID;

    uint8_t mrdyIsrEventFlag;

}NPITASK_AppData_t;

NPITASK_AppData_t gNpiTaskAppData;

#if defined(NPI_SREQRSP)
//! \brief Flag/Counter indicating a Synchronous REQ/RSP is currently being
//!        processed.
static int8_t syncTransactionInProgress = 0;

//! \brief Clock Struct for Sync REQ/RSP watchdog timer
//!
static Clock_Struct syncReqRspWatchDogClkStruct;
static Clock_Handle syncReqRspWatchDogClkHandle;
#endif // NPI_SREQRSP


//! \brief NPI thread ICall Semaphore.
//!
#define FLE_DEBUG 1

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
static NPITASK_STATUS NPITask_ProcessTXQ(void);

#if defined(NPI_SREQRSP)
//! \brief SYNC TX Q Processing function.
//!
static NPITASK_STATUS NPITask_ProcessSyncTXQ(void);

//! \brief SYNC RX Q Processing function.
//!
static NPITASK_STATUS NPITask_processSyncRXQ(void);

//! \brief Sync REQ/RSP Watchdog Timer CB
//!
static void syncReqRspWatchDogTimeoutCB( UArg a0 );
#endif // NPI_SREQRSP

//! \brief ASYNC RX Q Processing function.
//!
static NPITASK_STATUS NPITask_ProcessRXQ(void);

//! \brief Callback function registered with Frame module to handle successful
//!        reception of message from host.
//!
static void NPITask_incomingFrameCB(uint16_t frameSize, uint8_t *pFrame,
                                    NPIMSG_Type msgType);

//! \brief Function to send message buffer to stack task.  Note, message buffer
//!        referenced by NPIMSG_msg_t is "unframed".
//!
static ICall_Errno NPITask_sendBufToStack(ICall_EntityID appEntity,
                                          NPIMSG_msg_t *msg);

//! \brief Function to handle incoming ICall Message Event
//!
static NPITASK_STATUS NPITask_processICallMsgEvent(uint8_t *pMsg,
                                                   ICall_ServiceEnum src,
                                                   ICall_EntityID dest);

//! \brief Function to process incoming Msg from stack task.
//!
static void NPITask_processStackMsg(uint8_t *pMsg);

//! \brief Function to create a POSIX queue.
//!
static int NPITask_createPQueue(mqd_t *queueHandle, char *mq_name, uint32_t mq_size, uint32_t mq_msgsize, uint32_t mq_flags);

// -----------------------------------------------------------------------------
//! \brief      Initialization for the NPI Thread
//!
//! \return     void
// -----------------------------------------------------------------------------
static void NPITask_inititializeTask(void)
{
    // create a Tx Queue instance
    NPITask_createPQueue(&gNpiTaskAppData.npiTxQueue,     "NPI Tx Queue",      NPI_TX_QUEUE_SIZE,      sizeof(NPIMSG_msg_t), O_NONBLOCK);
    // create an Rx Queue instance
    NPITask_createPQueue(&gNpiTaskAppData.npiRxQueue,     "NPI Rx Queue",      NPI_RX_QUEUE_SIZE,      sizeof(NPIMSG_msg_t), O_NONBLOCK);

#if defined(NPI_SREQRSP)
    // create an Sync RX Queue instance
    NPITask_createPQueue(&gNpiTaskAppData.npiSyncRxQueue, "NPI Sync Tx Queue", NPI_SYNC_TX_QUEUE_SIZE, sizeof(NPIMSG_msg_t), 0 /* BLOCKING */);
    // create an Sync TX Queue instance
    NPITask_createPQueue(&gNpiTaskAppData.npiSyncTxQueue, "NPI Sync Rx Queue", NPI_SYNC_RX_QUEUE_SIZE, sizeof(NPIMSG_msg_t), O_NONBLOCK);

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

    syncReqRspWatchDogClkHandle = Clock_handle(&syncReqRspWatchDogClkStruct);
#endif // NPI_SREQRSP
    /* Enroll the service that this stack represents */
    ICall_enrollService( ICALL_SERVICE_CLASS_NPI, NULL, &gNpiTaskAppData.npiAppEntityID, (void *)&gNpiTaskAppData.pNpiSem );
    // create Semaphore instance
    // Initialize Frame Module
    NPIFrame_initialize(&NPITask_incomingFrameCB);

    // Initialize Network Processor Interface (NPI) and Transport Layer
    NPITL_initTL( &NPITask_transportTxDoneCallBack,
                  &NPITask_transportRXCallBack,
                  &NPITask_MRDYEventCB );
}

static NPITASK_STATUS NPITask_handleICallEvents()
{
  ICall_ServiceEnum src;
  ICall_EntityID dest;
  NPITASK_STATUS npiStatus = QUEUE_EMPTY;

  uint8_t *pMsg;

  // ICall Message Event
  if (ICall_fetchServiceMsg(&src, &dest, (void * *) &pMsg) == ICALL_ERRNO_SUCCESS)
  {
    npiStatus = NPITask_processICallMsgEvent( pMsg, src, dest );
  }
  return npiStatus;
}

static void NPITask_handleAsyncEvents()
{
  NPITASK_STATUS npiStatus = QUEUE_EMPTY;

  npiStatus |= NPITask_handleICallEvents();

  npiStatus |= NPITask_ProcessTXQ();

  npiStatus |= NPIFrame_collectFrameData();

  npiStatus |= NPITask_ProcessRXQ();

  /* As long as there was a processed message - Post the semaphore, so the NPI task
   * will run again, as there could be multiple number of messages in the queue.
   * In case all queues are empty - meaning the NPI task finished handling all
   * messages and there is no need for post for the semaphore. */
  if (npiStatus != QUEUE_EMPTY)
  {
    sem_post(gNpiTaskAppData.pNpiSem);
  }
}

static void NPITask_handleSyncEvents()
{
#if defined(NPI_SREQRSP)
  NPITASK_STATUS npiStatus = QUEUE_EMPTY;

  if (syncTransactionInProgress)
  {
    npiStatus = NPITask_ProcessSyncTXQ();
    if (npiStatus == MSG_PROCESSED)
    {
      npiStatus |= NPITask_handleICallEvents();
      npiStatus |= NPITask_ProcessSyncRXQ();
    }
    /* As long as there was a processed message - Post the semaphore, so the NPI task
     * will run again, as there could be multiple number of messages in the queue.
     * In case all queues are empty - meaning the NPI task finished handling all
     * messages and there is no need for post for the semaphore. */
    if (npiStatus != QUEUE_EMPTY)
    {
      sem_post(gNpiTaskAppData.pNpiSem);
    }
  }
#endif
}

static void NPITask_handleMrdyEvents(void)
{
#if (NPI_FLOW_CTRL == 1)
  if (gNpiTaskAppData.mrdyIsrEventFlag & NPITASK_MRDY_FLAG)
  {
    NPITL_handleMrdyEvent();
    gNpiTaskAppData.mrdyIsrEventFlag = 0;
  }
#endif // NPI_FLOW_CTRL = 1
}

// -----------------------------------------------------------------------------
//! \brief      NPI main event processing loop.
//!
//! \return     void
// -----------------------------------------------------------------------------
static void NPITask_process(void)
{
  while (true)
  {
//    mq_getattr(gNpiTaskAppData.npiTxQueue, &gNpiTaskAppData.txQueueAttr);
//    mq_getattr(gNpiTaskAppData.npiRxQueue, &gNpiTaskAppData.rxQueueAttr);

    sem_wait(gNpiTaskAppData.pNpiSem);

    NPITask_handleMrdyEvents();
    NPITask_handleSyncEvents();
    NPITask_handleAsyncEvents();
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
Void NPITask_Fxn(UArg a0, UArg a1)
{
    NPITask_task();
}

// -----------------------------------------------------------------------------
//! \brief      Task creation function for NPI
//!
//! \return     void
// -----------------------------------------------------------------------------
void NPITask_createTask(uint32_t stackID)
{
    memset(&gNpiTaskAppData, 0, sizeof(NPITASK_AppData_t));

    // Set stackID for future ICall Messaging
    gNpiTaskAppData.stackServiceID = stackID;

    gNpiTaskAppData.pTaskStack = ICall_malloc(NPITASK_STACK_SIZE);
    if (gNpiTaskAppData.pTaskStack == NULL)
    {
      return;
    }

    // Configure and create the NPI task.
    Task_Params npiTaskParams;
    Task_Params_init(&npiTaskParams);
    npiTaskParams.name = "npi";
    npiTaskParams.stack = gNpiTaskAppData.pTaskStack;
    npiTaskParams.stackSize = NPITASK_STACK_SIZE;
    npiTaskParams.priority = NPITASK_PRIORITY;

    gNpiTaskAppData.taskHandle = Task_construct(&gNpiTaskAppData.TaskStruct, NPITask_Fxn, &npiTaskParams, NULL);
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
    ICall_CSState key;

    // Enter CS to prevent higher priority tasks
    // from also enqueuing msg at the same time
    key = ICall_enterCriticalSection();

    NPIMSG_msg_t npiMsg;

    NPIFrame_frameMsg(&npiMsg, pMsg);

    switch (npiMsg.msgType)
    {
        // Enqueue to appropriate NPI Task Q and post corresponding event.
#if defined(NPI_SREQRSP)
        case NPIMSG_Type_SYNCRSP:
        {
            if (mq_send(gNpiTaskAppData.npiSyncTxQueue, (char*)&npiMsg, sizeof(NPIMSG_msg_t), 1) != 0)
            {
              // Failed to enqeue the message, free the allocations
                if (npiMsg.pBuf)
                {
                  ICall_freeMsg(npiMsg.pBuf);
                }
                ICall_leaveCriticalSection(key);
              return;
            }
            sem_post(gNpiTaskAppData.pNpiSem);
            break;
        }
#endif // NPI_SREQRSP
        case NPIMSG_Type_ASYNC:
        {
            if (mq_send(gNpiTaskAppData.npiTxQueue, (char *)&npiMsg, sizeof(NPIMSG_msg_t), 1) != 0)
            {
                // Failed to enqeue the message, free the allocations
              if (npiMsg.pBuf)
              {
                ICall_freeMsg(npiMsg.pBuf);
              }
              ICall_leaveCriticalSection(key);
              return;
            }
            sem_post(gNpiTaskAppData.pNpiSem);
            break;
        }
        default:
        {
            //error
            if (npiMsg.pBuf)
            {
              ICall_freeMsg(npiMsg.pBuf);
            }
            ICall_leaveCriticalSection(key);
            return;
        }
    }

    ICall_leaveCriticalSection(key);
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
static NPITASK_STATUS NPITask_processICallMsgEvent(uint8_t *pMsg,
                                                   ICall_ServiceEnum src,
                                                   ICall_EntityID dest)
{
    NPITASK_STATUS npiStatus = QUEUE_EMPTY;

    if (dest == gNpiTaskAppData.npiAppEntityID)
    {
        // Message received from the Stack.
        NPITask_processStackMsg(pMsg);
        npiStatus = MSG_PROCESSED;
    }

    return npiStatus;
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
static ICall_Errno NPITask_sendBufToStack(ICall_EntityID appEntity,
                                          NPIMSG_msg_t *pMsg)
{

    ICall_Errno errno = ICALL_ERRNO_SUCCESS;


    errno = ICall_sendServiceMsg(appEntity, gNpiTaskAppData.stackServiceID,
                                 ICALL_MSG_FORMAT_KEEP, pMsg->pBuf);

    /* Freeing the pMsg here will cause a double free in some cases, such as in NPITask_processRXQ(),
     * where the pMsg is re-used in the incomingRXEventAppCBFunc() */
//    ICall_free(pMsg);

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
            {
                break;
            }
        }
    }


            ICall_CSState key;

            // Enter CS to prevent higher priority tasks
            // from also enqueuing msg at the same time
            key = ICall_enterCriticalSection();
            NPIMSG_msg_t npiMsg;

            NPIFrame_frameMsg(&npiMsg, pMsg);

            switch(npiMsg.msgType)
            {
                // Enqueue to appropriate NPI Task Q and post corresponding event.
#if defined(NPI_SREQRSP)
                case NPIMSG_Type_SYNCRSP:
                {
                    if (mq_send(gNpiTaskAppData.npiSyncTxQueue, (char *)&npiMsg, sizeof(NPIMSG_msg_t), 1) != 0)
                    {
                        // Failed to enque the message, free the allocations
                        if (npiMsg.pBuf)
                        {
                          ICall_freeMsg(npiMsg.pBuf);
                        }
                        ICall_leaveCriticalSection(key);
                      return;
                    }

                    sem_post(gNpiTaskAppData.pNpiSem);
                    break;
                }
#endif // NPI_SREQRSP
                case NPIMSG_Type_ASYNC:
                {
                    if (mq_send(gNpiTaskAppData.npiTxQueue, (char *)&npiMsg, sizeof(NPIMSG_msg_t), 1) != 0)
                    {
                      // Failed to enqueue the message, free the allocations
                      if (npiMsg.pBuf)
                      {
                        ICall_freeMsg(npiMsg.pBuf);
                      }
                      ICall_leaveCriticalSection(key);
                      return;
                    }

                    sem_post(gNpiTaskAppData.pNpiSem);
                    break;
                }
                default:
                {
                  /* Fail - unsupported message type */
                    if (npiMsg.pBuf)
                    {
                      ICall_freeMsg(npiMsg.pBuf);
                    }
                    ICall_leaveCriticalSection(key);
                    return;
                }
            }

            ICall_leaveCriticalSection(key);
}

// -----------------------------------------------------------------------------
//! \brief      Dequeue next message in the ASYNC TX Queue and send to serial
//!             interface.
//!
//! \return     void
// -----------------------------------------------------------------------------
static NPITASK_STATUS NPITask_ProcessTXQ(void)
{
    NPITASK_STATUS npiStatus = QUEUE_EMPTY;
    NPIMSG_msg_t npiMsg;

    if (NPITask_NpiTlTestIsFree())
    {
      if (mq_receive(gNpiTaskAppData.npiTxQueue, (char*)&npiMsg, sizeof(NPIMSG_msg_t), NULL) == sizeof(NPIMSG_msg_t))
      {
        gNpiTaskAppData.lastQueuedTxMsg = npiMsg.pBuf;
        gNpiTaskAppData.lastQueuedTxMsgSize = npiMsg.pBufSize;

        if (npiMsg.pBuf)
        {
          NPITL_writeTL(npiMsg.pBuf, npiMsg.pBufSize);

          //free the Queue record
          ICall_freeMsg(npiMsg.pBuf);
        }

        npiStatus = MSG_PROCESSED;
      }
    }
    else
    {
      npiStatus = PROCESS_BUSY;
    }
    return npiStatus;
}

#if defined(NPI_SREQRSP)
// -----------------------------------------------------------------------------
//! \brief      Dequeue next message in the SYNC TX Queue and send to serial
//!             interface.
//!
//! \return     void
// -----------------------------------------------------------------------------
static NPITASK_STATUS NPITask_ProcessSyncTXQ(void)
{
    ICall_CSState key;
    NPITASK_STATUS npiStatus = QUEUE_EMPTY;

    // Processing of any TX Queue should only be done
    // in a critical section since any application
    // task can enqueue items freely
    NPIMSG_msg_t npiMsg;
    if (NPITask_NpiTlTestIsFree())
    {
      if (mq_receive(gNpiTaskAppData.npiSyncTxQueue, (char*)&npiMsg, sizeof(NPIMSG_msg_t), NULL) == sizeof(NPIMSG_msg_t))
      {
          gNpiTaskAppData.lastQueuedTxMsg = npiMsg.pBuf;

          NPITL_writeTL(npiMsg.pBuf, npiMsg.pBufSize);

          key = ICall_enterCriticalSection();
          // Decrement the outstanding Sync REQ/RSP flag.
          syncTransactionInProgress--;

          // Stop watchdog clock.
          Clock_stop(syncReqRspWatchDogClkHandle);

          if (syncTransactionInProgress < 0)
          {
              // not expected!
              syncTransactionInProgress = 0;
          }
          ICall_leaveCriticalSection(key);

          //free the Queue record
          if (npiMsg.pBuf)
          {
            ICall_freeMsg(npiMsg.pBuf);
          }

          npiStatus = MSG_PROCESSED;
      }
    }
    else
    {
      npiStatus = PROCESS_BUSY;
    }
    return npiStatus;
}
#endif // NPI_SREQRSP

// -----------------------------------------------------------------------------
//! \brief      Dequeue next message in the RX Queue and process it.
//!
//! \return     void
// -----------------------------------------------------------------------------
static NPITASK_STATUS NPITask_ProcessRXQ(void)
{
    ICall_CSState key;
    NPITASK_STATUS npiStatus = QUEUE_EMPTY;
    NPIMSG_msg_t npiMsg;
    ssize_t msg_size;

    key = ICall_enterCriticalSection();
    msg_size = mq_receive(gNpiTaskAppData.npiRxQueue, (char*)&npiMsg, sizeof(NPIMSG_msg_t), NULL);
    ICall_leaveCriticalSection(key);

    if (msg_size == sizeof(NPIMSG_msg_t))
    {
      if (incomingRXEventAppCBFunc != NULL)
      {
          switch (incomingRXReroute)
          {
              case ECHO:
              {
                  /* Race in freeing the npiMsg->pData (double free). Should not use this case! */
                  // send to stack and a copy to the application
                  NPITask_sendBufToStack(gNpiTaskAppData.npiAppEntityID, &npiMsg);
                      /*HostTestApp_handleNPIRxInterceptEvent*/
                  incomingRXEventAppCBFunc((uint8_t *) &npiMsg);
                  break;
              }

              case INTERCEPT:
              {
                  // send a copy only to the application
                  // npiMsg need to be free in the callback
                      /*HostTestApp_handleNPIRxInterceptEvent*/
                  incomingRXEventAppCBFunc((uint8_t *) &npiMsg);
                  break;
              }

              case NONE:
              {
                  NPITask_sendBufToStack(gNpiTaskAppData.npiAppEntityID, &npiMsg);
                  ICall_freeMsg(npiMsg.pBuf);
                  break;
              }
          }
      }
      else
      {
          // send to stack and a copy to the application
          NPITask_sendBufToStack(gNpiTaskAppData.npiAppEntityID, &npiMsg);
          ICall_freeMsg(npiMsg.pBuf);
      }
      npiStatus = MSG_PROCESSED;

      // DON'T free the referenced npiMsg container.  This will be free'd in the
      // stack task.
    }

    return npiStatus;
}

#if defined(NPI_SREQRSP)
// -----------------------------------------------------------------------------
//! \brief      Dequeue next message in the RX Queue and process it.
//!
//! \return     void
// -----------------------------------------------------------------------------
static NPITASK_STATUS NPITask_processSyncRXQ(void)
{
    NPITASK_STATUS npiStatus = QUEUE_EMPTY;

    NPIMSG_msg_t npiMsg;

    if (syncTransactionInProgress == 0)
    {
       if (mq_receive(gNpiTaskAppData.npiSyncRxQueue, (char*)&npiMsg, sizeof(NPIMSG_msg_t), NULL) == sizeof(NPIMSG_msg_t))
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
                       // send to stack and a copy to the application
                       NPITask_sendBufToStack(gNpiTaskAppData.npiAppEntityID, &npiMsg);
                       incomingRXEventAppCBFunc(npiMsg.pBuf);
                       break;
                   }

                   case INTERCEPT:
                   {
                       // send a copy only to the application
                       incomingRXEventAppCBFunc(npiMsg.pBuf);
                       break;
                   }

                   case NONE:
                   {
                       NPITask_sendBufToStack(gNpiTaskAppData.npiAppEntityID, &npiMsg);
                       ICall_freeMsg(npiMsg.pBuf);
                       break;
                   }
               }
           }
           else
           {
               // send to stack and a copy to the application
               NPITask_sendBufToStack(gNpiTaskAppData.npiAppEntityID, &npiMsg);
               ICall_freeMsg(npiMsg.pBuf);
           }

           //free the Queue record
           // DON'T free the referenced npiMsg container.  This will be free'd in the
           // stack task.
           npiStatus = MSG_PROCESSED;
       }
    }

    return npiStatus;
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
    sem_post(gNpiTaskAppData.pNpiSem);
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
static void NPITask_incomingFrameCB(uint16_t frameSize, uint8_t *pFrame,
                                    NPIMSG_Type msgType)
{
    // Create NPIMSG_msg_t
    NPIMSG_msg_t npiMsg;

    npiMsg.pBuf = pFrame;
    npiMsg.pBufSize = frameSize;

    switch (msgType)
    {

        // Enqueue to appropriate NPI Task Q and post corresponding event.
        case NPIMSG_Type_ASYNC:
        {
            npiMsg.msgType = NPIMSG_Type_ASYNC;
            if (mq_send(gNpiTaskAppData.npiRxQueue, (char*)&npiMsg, sizeof(NPIMSG_msg_t), 1) != 0)
            {
               // Failed to enqueue, free the allocation
               if (npiMsg.pBuf)
               {
                 ICall_freeMsg(npiMsg.pBuf);
               }
               return;
            }

            sem_post(gNpiTaskAppData.pNpiSem);

            break;
        }

#if defined(NPI_SREQRSP)
        case NPIMSG_Type_SYNCREQ:
        {
            npiMsg.msgType = NPIMSG_Type_SYNCREQ;
            if (mq_send(gNpiTaskAppData.npiSyncRxQueue, (char*)&npiMsg, sizeof(NPIMSG_msg_t), 1) != 0)
            {
                 // Failed to enqueue, free the allocation
                if (npiMsg.pBuf)
                {
                  ICall_freeMsg(npiMsg.pBuf);
                }
                return;
            }

            sem_post(gNpiTaskAppData.pNpiSem);

            break;
        }
#endif // NPI_SREQRSP

        default:
        {
            // undefined msgType
            if (npiMsg.pBuf)
            {
              ICall_freeMsg(npiMsg.pBuf);
            }
            return;
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
        sem_post(gNpiTaskAppData.pNpiSem);
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
  gNpiTaskAppData.mrdyIsrEventFlag = NPITASK_MRDY_FLAG;
  sem_post(gNpiTaskAppData.pNpiSem);
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

        // re-enter to Task event loop
        sem_post(gNpiTaskAppData.pNpiSem);
    }
    else
    {
        // not expected
    }
}
#endif // NPI_SREQRSP

// -----------------------------------------------------------------------------
//! \brief      Create a POSIX queue.
//!
//! \param    queueHandle     queue handle
//! \param    mq_name         name
//! \param    mq_size         number of elements for the queue
//! \param    mq_msgsize      size of queue element
//! \param    mq_flags        flags
//!
//! \return   int
// -----------------------------------------------------------------------------
static int NPITask_createPQueue(mqd_t *queueHandle, char *mq_name, uint32_t mq_size, uint32_t mq_msgsize, uint32_t mq_flags)
{
  struct mq_attr attr;

  attr.mq_flags =  O_CREAT | O_RDWR | mq_flags;
  attr.mq_curmsgs = 0;
  attr.mq_maxmsg = mq_size;
  attr.mq_msgsize = mq_msgsize;

  /* Create the message queue */
  *queueHandle = mq_open(mq_name, O_CREAT | O_RDWR | mq_flags, 0, &attr);

  return 0;
}

// -----------------------------------------------------------------------------
//! \brief      API for application task to properly release the NPI message
//!             internal buffer. The container is declared on stack and should not be freed.
//!
//! \param[in]  pMsg    Pointer to message buffer.
//!
//! \return     void
// -----------------------------------------------------------------------------
void NPITask_freeNpiMsg(uint8_t *pMsg)
{
  if (NULL != pMsg)
  {
    if (((NPIMSG_msg_t *)pMsg)->pBuf && ((NPIMSG_msg_t *)pMsg)->pBufSize)
    {
      // The data is stored as a message, free this first.
      ICall_freeMsg(((NPIMSG_msg_t *)pMsg)->pBuf);
    }

    // The pMsg is declared on stack and should not be freed
  }
}
