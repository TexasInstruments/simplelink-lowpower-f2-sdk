/******************************************************************************

 @file osal2tirtos.c

 @brief converts ICAll and OSAL API's to native TIRTOS/POSIX calls

 Group: WCS LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2016-2025, Texas Instruments Incorporated
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

/***** Includes *****/
#include "osal_port.h"

#include "stdlib.h"

/* POSIX Header files */
#include <pthread.h>
#include <semaphore.h>

#include <ti/drivers/dpl/HwiP.h>
#include <ti/drivers/dpl/ClockP.h>
#include <ti/drivers/Power.h>
#include <ti/drivers/power/PowerCC26XX.h>
#include <ti/drivers/utils/Random.h>

/* TI Logging Files */
#include <ti/log/Log.h>

/***** Defines *****/
/* Only 1 application can talk to the MAC */
#define MAX_TASKS 15

/***** Variable declarations *****/


typedef union _osal_port_state_union_t
{
  /** critical section variable as declared in the interface */
  uint32_t  state;
  /** @internal field used to access internal data */
  struct _each
  {
    /** field to store Task_disable() return value */
    uint16_t taskkey;
    /** field to store Hwi_disable() return value */
    uint16_t hwikey;
  } each;
} OsalPort_CSStateUnion;

typedef struct
{
    uint8_t taskId;
    pthread_t taskHndl;
    OsalPort_MsgQ qHandle;
    sem_t* taskSem;
    bool conservePower;
    uint32_t* pEventFlag;
} TaskEntry;

// required for rtos_heaposal.h
// TODO
typedef struct
{
  uint32_t totalSize;
  uint32_t totalFreeSize;
  uint32_t largestFreeSize;
} ICall_heapStats_t;

/*static*/ TaskEntry taskTbl[MAX_TASKS];
/*static*/ uint8_t taskCnt = 0;

/* instantiate variable referenced in ROM but not used */
uint16_t *macTasksEvents = 0;

/**
 * @internal
 * Wakeup schedule data structure definition
 */
typedef struct _osalPort_schedule_t
{
  ClockP_Handle  clock;
  OsalPort_TimerCback cback;
  void *arg;
} OsalPort_ScheduleEntry;

/***** Private function definitions *****/
#ifndef FREERTOS_SUPPORT
// DMM currently uses ICall Heap
#ifdef USE_DMM
extern void *ICall_heapMalloc(uint32_t size);
extern void *ICall_heapRealloc(void *blk, uint32_t size);
extern void ICall_heapFree(void *blk);

#define OsalPort_heapMalloc         ICall_heapMalloc
#define OsalPort_heapRealloc        ICall_heapRealloc
#define OsalPort_heapFree           ICall_heapFree
#else
/***** Public function definitions *****/
/* Implementing a simple heap using heapmgr.h template.
 * This simple heap depends on critical section implementation
 * and hence the template is used after critical section definition. */
static void *OsalPort_heapMalloc(uint32_t size);
static void *OsalPort_heapRealloc(void *blk, uint32_t size);
static void OsalPort_heapFree(void *blk);

#define HEAPMGR_INIT            OsalPort_heapInit
#define HEAPMGR_MALLOC          OsalPort_heapMalloc
#define HEAPMGR_FREE            OsalPort_heapFree
#define HEAPMGR_REALLOC         OsalPort_heapRealloc
#define HEAPMGR_GETSTATS        OsalPort_heapGetStats
#define HEAPMGR_MALLOC_LIMITED  OsalPort_heapMallocLimited

#ifdef HEAPMGR_METRICS
void OsalPort_heapMgrGetMetrics(uint32_t *pBlkMax,
                             uint32_t *pBlkCnt,
                             uint32_t *pBlkFree,
                             uint32_t *pMemAlo,
                             uint32_t *pMemMax,
                             uint32_t *pMemUB);

#define HEAPMGR_GETMETRICS      OsalPort_heapMgrGetMetrics
#endif

#define HEAPMGR_LOCK()                                       \
  do { OsalPort_heapCSState = OsalPort_enterCS(); } while (0)
#define HEAPMGR_UNLOCK()                                     \
  do { OsalPort_leaveCS(OsalPort_heapCSState); } while (0)
#define HEAPMGR_IMPL_INIT()

/* Note that a static variable can be used to contain critical section
 * state since heapmgr.h template ensures that there is no nested
 * lock call. */
static uint32_t OsalPort_heapCSState;

#if defined(HEAPMGR_CONFIG) && ((HEAPMGR_CONFIG == 0) || (HEAPMGR_CONFIG == 0x80))
#include <rtos_heaposal.h>
#elif defined(HEAPMGR_CONFIG) && ( (HEAPMGR_CONFIG == 1) || (HEAPMGR_CONFIG == 0x81))
#include <rtos_heapmem.h>
#elif defined(HEAPMGR_CONFIG) && ( (HEAPMGR_CONFIG == 2) || (HEAPMGR_CONFIG == 0x82))
#include <rtos_heaptrack.h>
#else
#include <rtos_heaposal.h>
#endif
#endif // USE_DMM
#endif
/*********************************************************************
 * @fn      OsalPort_registerTask
 *
 * @brief
 *
 *    This function is used to deallocate a message buffer. This function
 *    is called by a task (or processing element) after it has finished
 *    processing a received message.
 *
 *
 * @param   uint8_t             *pMsg - pointer to new message buffer
 * @param   sem_t*               taskSem - taskSemaphore
 * @param   uint32_t* -         pEvent - pointer to event flag
 *
 * @return  Task ID
 */
uint8_t OsalPort_registerTask(void* taskHndl, void* taskSem, uint32_t* pEvent)
{
    if(taskCnt < MAX_TASKS)
    {
        taskTbl[taskCnt].taskId = taskCnt;
        taskTbl[taskCnt].taskHndl = taskHndl;
        taskTbl[taskCnt].taskSem = (sem_t*) taskSem;
        taskTbl[taskCnt].qHandle = NULL;
        taskTbl[taskCnt].conservePower = false;
        taskTbl[taskCnt].pEventFlag = pEvent;
    }

    taskCnt++;

    /* return task count as the task ID
     */
    return taskCnt-1;
}

/*********************************************************************
 * @fn      OsalPort_msgAllocate
 *
 * @brief
 *
 *    This function is called by a task to allocate a message buffer
 *    into which the task will encode the particular message it wishes
 *    to send.  This common buffer scheme is used to strictly limit the
 *    creation of message buffers within the system due to RAM size
 *    limitations on the microprocessor.   Note that all message buffers
 *    are a fixed size (at least initially).  The parameter len is kept
 *    in case a message pool with varying fixed message sizes is later
 *    created (for example, a pool of message buffers of size LARGE,
 *    MEDIUM and SMALL could be maintained and allocated based on request
 *    from the tasks).
 *
 *
 * @param   uint8_t len  - wanted buffer length
 *
 *
 * @return  pointer to allocated buffer or NULL if allocation failed.
 */
#ifdef MALLOC_DEBUG
#ifdef OsalPort_msgAllocate
#undef OsalPort_msgAllocate
#endif
uint8_t* OsalPort_msgAllocate_dbg(uint16_t len, const char *caller);
#endif


uint8_t * OsalPort_msgAllocate(uint16_t len )
{
    uint8_t *pMsg = NULL;
    OsalPort_MsgHdr* pHdr;


    if ( len == 0 )
        return ( NULL );

    pHdr = (OsalPort_MsgHdr*) OsalPort_malloc( len + sizeof( OsalPort_MsgHdr ) );

    if ( pHdr )
    {
        pHdr->next = NULL;
        pHdr->len = len;
        pHdr->dest_id = OsalPort_TASK_NO_TASK;

        pMsg = (uint8_t *)((uint8_t *)pHdr + sizeof( OsalPort_MsgHdr ));
    }

    return pMsg;
}

#ifdef MALLOC_DEBUG
uint8_t*  OsalPort_msgAllocate_dbg(uint16_t len, const char *caller)
{
    uint32_t addr = (uint32_t)__builtin_extract_return_addr (__builtin_return_address (0));

    Log_printf(LogModule_154_MALLOC_DEBUG, Log_DEBUG, "OsalPort_msgAllocate: Size) %u Called By) %u", len + sizeof( OsalPort_MsgHdr ), addr);

    return  OsalPort_msgAllocate(len );
}
#endif

/*********************************************************************
 * @fn      OsalPort_msgDeallocate
 *
 * @brief
 *
 *    This function is used to deallocate a message buffer. This function
 *    is called by a task (or processing element) after it has finished
 *    processing a received message.
 *
 *
 * @param   uint8_t *pMsg - pointer to new message buffer
 *
 * @return  OsalPort_SUCCESS, OsalPort_INVALID_MSG_POINTER
 */

#ifdef MALLOC_DEBUG
#ifdef OsalPort_msgDeallocate
#undef OsalPort_msgDeallocate
#endif
uint8_t OsalPort_msgDeallocate_dbg(uint8_t *pMsg, const char *caller);
#endif

uint8_t OsalPort_msgDeallocate( uint8_t *pMsg )
{
    uint8_t *x;

    if ( pMsg == NULL )
        return ( OsalPort_INVALID_MSG_POINTER );

    // don't deallocate queued buffer
    if ( OsalPort_MSG_ID( pMsg ) != OsalPort_TASK_NO_TASK )
    {
#ifdef MALLOC_DEBUG
        Log_printf(LogModule_154_MALLOC_DEBUG, Log_DEBUG, "OsalPort_msgDeallocate_dbg: Failed to deallocate buffer: %u", pMsg);
#endif
        return ( OsalPort_MSG_BUFFER_NOT_AVAIL );
    }

    x = (uint8_t *)((uint8_t *)pMsg - sizeof( OsalPort_MsgHdr ));


    OsalPort_free( (void *)x );

    return ( OsalPort_SUCCESS );
}

#ifdef MALLOC_DEBUG
uint8_t OsalPort_msgDeallocate_dbg(uint8_t *pMsg, const char *caller)
{
    uint32_t addr = (uint32_t)__builtin_extract_return_addr (__builtin_return_address (0));

    Log_printf(LogModule_154_MALLOC_DEBUG, Log_DEBUG, "OsalPort_msgDeallocate_dbg: Buffer Address) %u Called By) %u", (uint8_t *)((uint8_t *)pMsg - sizeof( OsalPort_MsgHdr )), addr);

    return  OsalPort_msgDeallocate(pMsg);
}
#endif

/*********************************************************************
 * @fn      OsalPort_msgSend
 *
 * @brief
 *
 *    This function is called by a task to send a command message to
 *    another task or processing element.  The sending_task field must
 *    refer to a valid task, since the task ID will be used
 *    for the response message.  This function will also set a message
 *    ready event in the destination tasks event list.
 *
 *
 * @param   uint8_t destinationTask - Send msg to Task ID
 * @param   uint8_t *pMsg - pointer to new message buffer
 *
 * @return  OsalPort_SUCCESS, OsalPort_INVALID_TASK, OsalPort_INVALID_MSG_POINTER
 */
uint8_t OsalPort_msgSend( uint8_t destinationTask, uint8_t *pMsg )
{
    uint8_t taskIdx;
    uint32_t key;

    if(pMsg == NULL)
    {
        return OsalPort_INVALID_MSG_POINTER;
    }

    /*find dest task */
    for(taskIdx = 0; taskIdx < taskCnt; taskIdx++)
    {
        if(taskTbl[taskIdx].taskId == destinationTask)
        {
            key = OsalPort_enterCS();

            OsalPort_msgEnqueue(&taskTbl[taskIdx].qHandle, pMsg );
            OsalPort_setEvent(destinationTask, OsalPort_SYS_EVENT_MSG);

            OsalPort_leaveCS(key);

            return OsalPort_SUCCESS;
        }
    }

    return OsalPort_INVALID_TASK;
}

/**************************************************************************************************
 * @fn          OsalPort_msgFind
 *
 * @brief       This function finds in place an OSAL message matching the task_id and event
 *              parameters.
 *
 * input parameters
 *
 * @param       taskId - The ID of the task that enqueued the message.
 * @param       event - The OSAL event id that the enqueued OSAL message must match.
 *
 * output parameters
 *
 * None.
 *
 * @return      NULL if no match, otherwise an in place pointer to the matching OSAL message.
 **************************************************************************************************
 */
OsalPort_EventHdr* OsalPort_msgFind(uint8_t taskId, uint8_t event)
{
    uint8_t taskIdx;
    uint32_t key;
    OsalPort_MsgHdr *pHdr = NULL;

    key = OsalPort_enterCS();

    /*find dest task */
    for(taskIdx = 0; taskIdx < taskCnt; taskIdx++)
    {
        if(taskTbl[taskIdx].taskId == taskId)
        {
            pHdr = (OsalPort_MsgHdr*) taskTbl[taskIdx].qHandle;

            // Look through the tasks queue for a message that matches the task_id and event parameters.
            while (pHdr != NULL)
            {
              if (((OsalPort_EventHdr *)pHdr)->event == event)
              {
                break;
              }

              pHdr = OsalPort_MSG_NEXT(pHdr);
            }
            break;
        }
    }

    OsalPort_leaveCS(key);

    return (OsalPort_EventHdr *)pHdr;
}

/*********************************************************************
 * @fn      OsalPort_msgReceive
 *
 * @brief
 *
 *    This function is called by a task to retrieve a received command
 *    message. The calling task must deallocate the message buffer after
 *    processing the message using the osal_msg_deallocate() call.
 *
 * @param   uint8_t taskId - receiving tasks ID
 *
 * @return  *uint8_t - message information or NULL if no message
 */
uint8_t *OsalPort_msgReceive( uint8_t destinationTask )
{
    uint8_t taskIdx;
    uint8_t* pMsg = NULL;

    for(taskIdx = 0; taskIdx < taskCnt; taskIdx++)
    {
        if(taskTbl[taskIdx].taskId == destinationTask)
        {
            pMsg = OsalPort_msgDequeue( &taskTbl[taskIdx].qHandle );

            // Are there any more messages?
            if ( OsalPort_MSG_Q_EMPTY(&taskTbl[taskIdx].qHandle) )
            {
                // Clear message event
                OsalPort_clearEvent(taskIdx, OsalPort_SYS_EVENT_MSG);
            }
            else
            {
                // Signal the task that another message is waiting
                OsalPort_setEvent(taskIdx, OsalPort_SYS_EVENT_MSG);
            }

        }
    }

    return pMsg;
}

/*********************************************************************
 * @fn      OsalPort_setEvent
 *
 * @brief
 *
 *    This function is called to set the event flags for a task. The
 *    event passed in is OR'd into the task's event variable.
 *
 * @param   uint8_t destinationTask - receiving tasks ID
 * @param   uint32_t eventFlag - what event to set
 *
 * @return  OsalPort_SUCCESS, OsalPort_FAILURE, OsalPort_INVALID_TASK
 */
uint8_t OsalPort_setEvent( uint8_t destinationTask, uint32_t eventFlag )
{
    uint8_t taskIdx;
    uint8_t status = OsalPort_INVALID_TASK;
    uint32_t key;

    for(taskIdx = 0; taskIdx < taskCnt; taskIdx++)
    {
        if(taskTbl[taskIdx].taskId == destinationTask)
        {
            key = OsalPort_enterCS();

            *taskTbl[taskIdx].pEventFlag |= (uint32_t)eventFlag;

            if(taskTbl[taskIdx].taskSem)
            {
                sem_post(taskTbl[taskIdx].taskSem);
            }

            status = OsalPort_SUCCESS;

            OsalPort_leaveCS(key);
        }
    }

    return status;
}

/*********************************************************************
 * @fn      OsalPort_waitEvent
 *
 * @brief
 *
 *    This function is called to end and wait the event bit mask for a task
 *
 * @param   taskId *pQ - service function ID to get event for
 *
 * @return  event ID
 */
uint32_t OsalPort_waitEvent(uint8_t taskId)
{
    uint8_t taskIdx;

    for(taskIdx = 0; taskIdx < taskCnt; taskIdx++)
    {
        if(taskTbl[taskIdx].taskId == taskId)
        {
            sem_wait(taskTbl[taskIdx].taskSem);
            return *taskTbl[taskIdx].pEventFlag;
        }
    }

    return 0;
}

/*********************************************************************
 * @fn      OsalPort_blockOnEvent
 *
 * @brief
 *
 *    This function is called to on an event posted to the TIRTOS task specified
 *
 *
 * @return  none
 */
void OsalPort_blockOnEvent(void* taskHndl)
{
    uint8_t taskIdx;

    for(taskIdx = 0; taskIdx < taskCnt; taskIdx++)
    {
        if(taskTbl[taskIdx].taskHndl == taskHndl)
        {
            sem_wait(taskTbl[taskIdx].taskSem);
        }
    }

    return;
}

/*********************************************************************
 * @fn      OsalPort_clearEvent
 *
 * @brief
 *
 *    This function is called to clear the event flags for a task. The
 *    inverse of event passed in is AND'd with the task's event variable.
 *
 * @param   uint8_t destinationTask - receiving tasks ID (if
 *          OsalPort_TASK_NO_TASK use the current running task)
 *
 * @param   uint32_t eventFlag - what event to set
 *
 */
void OsalPort_clearEvent(uint8_t taskId, uint32_t eventFlag)
{
    uint8_t taskIdx;
    uint32_t key;

    for(taskIdx = 0; taskIdx < taskCnt; taskIdx++)
    {
        if( ((taskId != OsalPort_TASK_NO_TASK) && (taskTbl[taskIdx].taskId == taskId)) ||
            ((taskId == OsalPort_TASK_NO_TASK) && (taskTbl[taskIdx].taskHndl == pthread_self())))
        {
            key = OsalPort_enterCS();
            *taskTbl[taskIdx].pEventFlag &=  ~(uint32_t)eventFlag;
            OsalPort_leaveCS(key);
            break;
        }
    }
}

/*********************************************************************
 * @fn      OsalPort_msgEnqueue
 *
 * @brief
 *
 *    This function enqueues an OSAL message into an OSAL queue.
 *    Used for intra-task messages (MAC internal only).
 *
 * @param   OsalPort_MsgQ *pQ - OSAL queue
 * @param   void *pMsg  - OSAL message
 *
 * @return  none
 */
void OsalPort_msgEnqueue( OsalPort_MsgQ *pQ, void *pMsg )
{
    void *list;
    uint32_t key;

    // Hold off interrupts
    key = OsalPort_enterCS();

    if (pMsg) {
        OsalPort_MSG_NEXT( pMsg ) = NULL;
        // If first message in queue
        if ( *pQ == NULL )
        {
          *pQ = pMsg;
        }
        else
        {
            // Find end of queue
            for ( list = *pQ; OsalPort_MSG_NEXT( list ) != NULL; list = OsalPort_MSG_NEXT( list ) );

            // Add message to end of queue
            OsalPort_MSG_NEXT( list ) = pMsg;
        }
    }

    // Re-enable interrupts
    OsalPort_leaveCS(key);
}

/*********************************************************************
 * @fn      OsalPort_msgEnqueueMax
 *
 * @brief
 *
 *    This function enqueues an OSAL message into an OSAL queue if
 *    the length of the queue is less than max.
 *
 * @param   osal_msg_q_t *pQ - OSAL queue
 * @param   void *pMsg  - OSAL message
 * @param   uint8_t max - maximum length of queue
 *
 * @return  true if message was enqueued, false otherwise
 */
uint8_t OsalPort_msgEnqueueMax( OsalPort_MsgQ *pQ, void *pMsg, uint8_t max )
{
    void *list;
    uint32_t key;
    uint32_t qCount = 0;
    uint8_t status = 0;

    // Hold off interrupts
    key = OsalPort_enterCS();

    // Find element count
    if(*pQ != NULL)
    {
        for ( list = *pQ; OsalPort_MSG_NEXT( list ) != NULL; list = OsalPort_MSG_NEXT( list ), qCount++ );
    }

    if(qCount < max)
    {
        OsalPort_msgEnqueue(pQ, pMsg);
        status = 1;
    }

    // Re-enable interrupts
    OsalPort_leaveCS(key);

    return status;
}

/*********************************************************************
 * @fn      OsalPort_msgDequeue
 *
 * @brief
 *
 *    This function dequeues an OSAL message from an OSAL queue.
 *
 * @param   OsalPort_MsgQ *pQ - OSAL queue
 *
 * @return  void * - pointer to OSAL message or NULL if queue is empty.
 */
void *OsalPort_msgDequeue( OsalPort_MsgQ *pQ )
{
    void *pMsg = NULL;
    uint32_t key;

    // Hold off interrupts
    key = OsalPort_enterCS();

    if ( *pQ != NULL )
    {
      // Dequeue message
      pMsg = *pQ;
      *pQ = OsalPort_MSG_NEXT( pMsg );
      OsalPort_MSG_NEXT( pMsg ) = NULL;
      OsalPort_MSG_ID( pMsg ) = OsalPort_TASK_NO_TASK;
    }

    // Re-enable interrupts
    OsalPort_leaveCS(key);

    return pMsg;
}

/*********************************************************************
 * @fn      OsalPort_msgFindDequeue
 *
 * @brief
 *
 *    This function finds a specific message based on taskId and event
 *    and dequeues and returns the message
 *
 * @param   taskId - target Task Id of the message
 * @param   event -  target Event ID of the message
 *
 * @return  void * - pointer to OSAL message or NULL if no message is found.
 */
OsalPort_EventHdr* OsalPort_msgFindDequeue(uint8_t taskId, uint8_t event)
{
    uint8_t taskIdx;
    uint32_t key;
    OsalPort_MsgHdr *pHdr = NULL;
    OsalPort_MsgHdr *pPrev = NULL;

    // Hold off interrupts
    key = OsalPort_enterCS();

    /*find dest task */
    for(taskIdx = 0; taskIdx < taskCnt; taskIdx++)
    {
        if(taskTbl[taskIdx].taskId == taskId)
        {
            pHdr = (OsalPort_MsgHdr*) taskTbl[taskIdx].qHandle;

            // Look through the tasks queue for a message that matches the task_id and event parameters.
            while (pHdr != NULL)
            {
              if (((OsalPort_EventHdr *)pHdr)->event == event)
              {

                if(pPrev == NULL)
                {
                  OsalPort_MSG_Q_HEAD(&taskTbl[taskIdx].qHandle) = OsalPort_MSG_NEXT(pHdr);
                }
                else
                {
                  OsalPort_MSG_NEXT(pPrev) = OsalPort_MSG_NEXT(pHdr);
                }
                OsalPort_MSG_NEXT( pHdr ) = NULL;
                OsalPort_MSG_ID( pHdr ) = OsalPort_TASK_NO_TASK;
                break;
              }

              pPrev = pHdr;
              pHdr = OsalPort_MSG_NEXT(pHdr);
            }
            break;
        }
    }

    OsalPort_leaveCS(key);

    return (OsalPort_EventHdr *)pHdr;
}

/*********************************************************************
 * @fn      OsalPort_msgPush
 *
 * @brief
 *
 *    This function pushes an OSAL message to the head of an OSAL
 *    queue.
 *
 * @param   OsalPort_MsgQ *pQ - OSAL queue
 * @param   void *pMsg  - OSAL message
 *
 * @return  none
 */
void OsalPort_msgPush( OsalPort_MsgQ *pQ, void *pMsg )
{
    uint32_t key;

    // Hold off interrupts
    key = OsalPort_enterCS();

    // Push message to head of queue
    OsalPort_MSG_NEXT( pMsg ) = *pQ;
    *pQ = pMsg;

    // Re-enable interrupts
    OsalPort_leaveCS(key);
}

/*********************************************************************
 * @fn      OsalPort_msgExtract
 *
 * @brief
 *
 *    This function extracts and removes an OSAL message from the
 *    middle of an OSAL queue.
 *
 * @param   OsalPort_MsgQ *pQ - OSAL queue
 * @param   void *pMsg  - OSAL message to be extracted
 * @param   void *pPrev  - OSAL message before pMsg in queue
 *
 * @return  none
 */
void OsalPort_msgExtract( OsalPort_MsgQ *pQ, void *pMsg, void *pPrev )
{
    uint32_t key;

    // Hold off interrupts
    key = OsalPort_enterCS();

    if ( pMsg == *pQ )
    {
        // remove from first
        *pQ = OsalPort_MSG_NEXT( pMsg );
    }
    else
    {
        // remove from middle
        OsalPort_MSG_NEXT( pPrev ) = OsalPort_MSG_NEXT( pMsg );
    }
    OsalPort_MSG_NEXT( pMsg ) = NULL;
    OsalPort_MSG_ID( pMsg ) = OsalPort_TASK_NO_TASK;

    // Re-enable interrupts
    OsalPort_leaveCS(key);
}

/*********************************************************************
 * @fn      OsalPort_pwrmgr_task_state
 *
 * @brief   This function is called by each task to state whether or
 *          not this task wants to conserve power.
 *
 * @param   task_id - calling task ID.
 *          state - whether the calling task wants to
 *          conserve power or not (OsalPort_PWR_CONSERVE or
 *          OsalPort_PWR_HOLD).
 *
 * @return  true if power is required; false is power is not required
 */
uint8_t OsalPort_pwrmgrTaskState( uint8_t destinationTask, uint8_t state )
{
    static uint8_t pwrContraint = 0;
    bool conservePower = false;
    uint8_t taskIdx;

    for(taskIdx = 0; taskIdx < taskCnt; taskIdx++)
    {
        if(taskTbl[taskIdx].taskId == destinationTask)
        {
            taskTbl[taskIdx].conservePower = state;
        }

        conservePower |= taskTbl[taskIdx].conservePower;
    }


    if(conservePower == OsalPort_PWR_CONSERVE)
    {
        if(pwrContraint == 1)
        {
            Power_releaseConstraint(PowerCC26XX_SD_DISALLOW);
            Power_releaseConstraint(PowerCC26XX_SB_DISALLOW);
            pwrContraint--;
        }
    }
    else
    {
        if(pwrContraint == 0)
        {
            Power_setConstraint(PowerCC26XX_SD_DISALLOW);
            Power_setConstraint(PowerCC26XX_SB_DISALLOW);
            pwrContraint++;
        }
    }

    return conservePower;
}

/**
 * @internal
 * Clock event handler function.
 * This function is used to implement the wakeup scheduler.
 *
 * @param arg  an @ref ICall_ScheduleEntry
 */
static void OsalPort_clockFunc(uintptr_t arg)
{
  OsalPort_ScheduleEntry *entry = (OsalPort_ScheduleEntry *) arg;

  entry->cback(entry->arg);
}

/*********************************************************************
 * @fn      OsalPort_setTimer
 *
 * @brief   This function creates if necessary and starts a TIRTOS timer
 *
 * @param   ticks - Timer timeout in ticks
 *          cback - Timer Callback
 *          arg   - Timer args
 *          clockHandle - clock handle
 *
 * @return  0 for OsalPort_SUCCESS
 */
uint8_t OsalPort_setTimer(uint32_t ticks, OsalPort_TimerCback cback, void *arg, OsalPort_TimerID *pClockHandle)
{
    OsalPort_ScheduleEntry *entry;

    if(pClockHandle == NULL)
    {
        return OsalPort_INVALIDPARAMETER;
    }

    if(*pClockHandle == NULL)
    {
        /* Construct BIOS Objects */
        ClockP_Params clkParams;

        entry = (OsalPort_TimerID) OsalPort_malloc(sizeof(OsalPort_ScheduleEntry));

        if(entry == NULL)
        {
            return OsalPort_NO_TIMER_AVAIL;
        }

        ClockP_Params_init(&clkParams);
        clkParams.period = 0;
        clkParams.startFlag = false;
        clkParams.arg = (uintptr_t) entry;

        entry->clock = ClockP_create((ClockP_Fxn)OsalPort_clockFunc, ticks, &clkParams);
        entry->cback = cback;
        entry->arg = arg;

        if(entry->clock == NULL)
        {
            return OsalPort_NO_TIMER_AVAIL;
        }
        *pClockHandle = entry;
    }
    else
    {
        uint32_t key;
        entry = (OsalPort_ScheduleEntry *) *pClockHandle;
        // Hold off interrupts so timer does not expire and call callback
        key = OsalPort_enterCS();
        if(ClockP_isActive(entry->clock))
        {
            ClockP_stop(entry->clock);
        }
        // Re-enable interrupts
        OsalPort_leaveCS(key);
    }

    ClockP_setTimeout(entry->clock, ticks);
    ClockP_start(entry->clock);

    return OsalPort_SUCCESS;
}

/*********************************************************************
 * @fn      OsalPort_deleteTimer
 *
 * @brief   This function stops a running timer if necessary and deletes
 *          the memory occupied by the timer.
 *
 * @param  clockHandle - clock handle
 *
 * @return  0 for OsalPort_SUCCESS
 */
uint8_t OsalPort_deleteTimer(OsalPort_TimerID *pClockHandle)
{
    uint32_t key;
    OsalPort_ScheduleEntry *entry;

    if ((pClockHandle == NULL) || (*pClockHandle == NULL) || (((OsalPort_ScheduleEntry *)*pClockHandle)->clock == NULL))
    {
        return OsalPort_INVALIDPARAMETER;
    }
    else
    {
        // Attempt to stop running clock
        entry = (OsalPort_ScheduleEntry *) *pClockHandle;

        // Hold off interrupts so timer does not expire and call callback
        key = OsalPort_enterCS();
        if(ClockP_isActive(entry->clock))
        {
            ClockP_stop(entry->clock);
        }

        // Clean up clock memory
        ClockP_delete(entry->clock);
        OsalPort_free((void*)entry);

        /* Clear input pointer as memory is no longer valid */
        *pClockHandle = NULL;

        // Re-enable interrupts
        OsalPort_leaveCS(key);
    }

    return OsalPort_SUCCESS;
}

/*********************************************************************
 * @fn      OsalPort_stopTimer
 *
 * @brief   This function creates if necessary and starts a TIRTOS timer
 *
 * @param   ticks - Timer timeout in ticks
 *          cback - Timer Callback
 *          arg   - Timer args
 *          clockHandle - clock handle
 *
 * @return  true if power is required; false is power is not required
 */
void OsalPort_stopTimer(OsalPort_TimerID *pClockHandle)
{
    if((pClockHandle != NULL) && (*pClockHandle != NULL))
    {
        ClockP_stop(*pClockHandle);
    }
}

/*********************************************************************
 * @fn      OsalPort_memcmp
 *
 * @brief
 *
 *   Generic memory compare.
 *
 * @param   src1 - source 1 address
 * @param   src2 - source 2 address
 * @param   len - number of bytes to compare
 *
 * @return  true - same, false - different
 */
uint8_t OsalPort_memcmp( const void *src1, const void *src2, uint32_t len )
{
  const uint8_t *pSrc1;
  const uint8_t *pSrc2;

  pSrc1 = src1;
  pSrc2 = src2;

  while ( len-- )
  {
    if( *pSrc1++ != *pSrc2++ )
      return false;
  }
  return true;
}

/*********************************************************************
 * @fn      OsalPort_memcpy
 *
 * @brief
 *
 *   Generic memory cpy. Returns pointer to end of destination buffer
 *
 */
void *OsalPort_memcpy( void *dst, const void *src, unsigned int len )
{
  uint8_t *pDst;
  const uint8_t *pSrc;

  pSrc = src;
  pDst = dst;

  while ( len-- )
    *pDst++ = *pSrc++;

  return ( pDst );
}

/*********************************************************************
 * @fn      osal_revmemcpy
 *
 * @brief   Generic reverse memory copy.  Starts at the end of the
 *   source buffer, by taking the source address pointer and moving
 *   pointer ahead "len" bytes, then decrementing the pointer.
 *
 *   Note: This function differs from the standard memcpy(), since
 *         it returns the pointer to the next destination uint8. The
 *         standard memcpy() returns the original destination address.
 *
 * @param   dst - destination address
 * @param   src - source address
 * @param   len - number of bytes to copy
 *
 * @return  pointer to end of destination buffer
 */
void *OsalPort_revmemcpy( void *dst, const void *src, unsigned int len )
{
  uint8_t *pDst;
  const uint8_t *pSrc;

  pSrc = src;
  pSrc += (len-1);
  pDst = dst;

  while ( len-- )
    *pDst++ = *pSrc--;

  return ( pDst );
}


/*********************************************************************
 * @fn      OsalPort_malloc
 *
 * @brief
 *
 *   Allocates memory from the heap. Currently maps directly to system malloc.
 *
 * @param   size - size of allocation
 *
 * @return  pointer o allocated memory
 */
#ifdef MALLOC_DEBUG
#ifdef OsalPort_malloc
#undef OsalPort_malloc
#endif
void* OsalPort_malloc_dbg(uint32_t size, const char *caller);
#endif
void* OsalPort_malloc(uint32_t size)
{
    void* buf;

#ifdef FREERTOS_SUPPORT
    buf = malloc(size);
#else
     buf = (OsalPort_heapMalloc(size));
#endif
    return buf;
}

#ifdef MALLOC_DEBUG
void* OsalPort_malloc_dbg(uint32_t size, const char *caller)
{
    uint32_t* buf;
   buf =  OsalPort_malloc(size);
   uint32_t addr = (uint32_t)__builtin_extract_return_addr (__builtin_return_address (0));

   Log_printf(LogModule_154_MALLOC_DEBUG, Log_DEBUG, "OsalPort_malloc_dbg: Size) %u Called By) %u", size,(uint32_t) addr);
   return buf;
}
#endif

/*********************************************************************
 * @fn      OsalPort_free
 *
 * @brief
 *
 *   Frees memory allocated in the heap. Currently maps directly to system free
 *
 * @param   size - size of allocation
 *
 * @return  pointer to allocated memory
 */

#ifdef MALLOC_DEBUG
#ifdef OsalPort_free
#undef OsalPort_free
#endif
void OsalPort_free_dbg(void* buf, const char *caller);
#endif

void OsalPort_free(void* buf)
{
#ifdef FREERTOS_SUPPORT
    free(buf);
#else
    OsalPort_heapFree(buf);
#endif
}

#ifdef MALLOC_DEBUG
void OsalPort_free_dbg(void* buf, const char *caller)
{
    uint32_t addr = (uint32_t)__builtin_extract_return_addr (__builtin_return_address (0));

    Log_printf(LogModule_154_MALLOC_DEBUG, Log_DEBUG, "OsalPort_free_dbg: Buffer Address) %u Called By) %u", buf ,(uint32_t) addr);

    OsalPort_free(buf);
}
#endif


/*********************************************************************
 * @fn      OsalPort_enterCS
 *
 * @brief
 *
 *   Enters a critical section
 *
 * @param   size - size of allocation
 *
 * @return  key used for exiting the critical section
 */
uint32_t OsalPort_enterCS(void)
{
    OsalPort_CSStateUnion cu;
    cu.each.hwikey = (uint_least16_t) HwiP_disable();
    return cu.state;
}

/*********************************************************************
 * @fn      OsalPort_leaveCS
 *
 * @brief
 *
 *   Exits a critical section
 *
 * @param   key used for exiting the critical section
 *
 */
void OsalPort_leaveCS(uint32_t key)
{
    OsalPort_CSStateUnion *cu = (OsalPort_CSStateUnion *) &key;
    HwiP_restore((uint32_t) cu->each.hwikey);
}

/*********************************************************************
 * @fn      OsalPort_buildUint16
 *
 * @brief
 *
 *   Build a uint16 out of 2 bytes (0 then 1).
 *
 * @param   swapped - 0 then 1
 *
 * @return  uint16
 */
uint16_t OsalPort_buildUint16( uint8_t *swapped )
{
    return (uint16_t)OsalPort_buildUint32(swapped, 2);
}

/*********************************************************************
 * @fn      OsalPort_buildUint32
 *
 * @brief
 *
 *   Build a uint32 out of sequential bytes.
 *
 * @param   swapped - sequential bytes
 * @param   len - number of bytes in the uint8 array
 *
 * @return  uint32
 */
uint32_t OsalPort_buildUint32( uint8_t *swapped, uint8_t len )
{
    if ( len == 1 )
    {
        return  ((uint32_t)swapped[0]);
    }
    else if ( len == 2 )
    {
        return  ((uint32_t)((uint32_t)((swapped[0]) & 0x00FF) +
                ((uint32_t)((swapped[1]) & 0x00FF) << 8)));
    }
    else if ( len == 3 ) {
        return  ((uint32_t)((uint32_t)((swapped[0]) & 0x00FF) +
                ((uint32_t)((swapped[1]) & 0x00FF) << 8) +
                ((uint32_t)((swapped[2]) & 0x00FF) << 16)));
    }
    else if ( len == 4 ) {
        return  ((uint32_t)((uint32_t)((swapped[0]) & 0x00FF) +
                ((uint32_t)((swapped[1]) & 0x00FF) << 8) +
                ((uint32_t)((swapped[2]) & 0x00FF) << 16) +
                ((uint32_t)((swapped[3]) & 0x00FF) << 24)));
    }
    else {
        return 0xFEFEFEFE;
    }
}

/*********************************************************************
 * @fn      OsalPort_bufferUint32
 *
 * @brief
 *
 *   Buffer an uint32 value - LSB first.
 *
 * @param   buf - buffer
 * @param   val - uint32 value
 *
 * @return  pointer to end of destination buffer
 */
uint8_t* OsalPort_bufferUint32( uint8_t *buf, uint32_t val )
{
    *buf++ = (uint8_t)((uint32_t)(val & 0x00FF));
    *buf++ = (uint8_t)((uint32_t)(((val) >>((1) * 8)) & 0x00FF));
    *buf++ = (uint8_t)((uint32_t)(((val) >>((2) * 8)) & 0x00FF));
    *buf++ = (uint8_t)((uint32_t)(((val) >>((3) * 8)) & 0x00FF));

    return buf;
}

/*********************************************************************
 * @fn      OsalPort_isBufSet
 *
 * @brief
 *
 *   Is all of the array elements set to a value?
 *
 * @param   buf - buffer to check
 * @param   val - value to check each array element for
 * @param   len - length to check
 *
 * @return  true if all "val"
 *          false otherwise
 */
uint8_t OsalPort_isBufSet( uint8_t *buf, uint8_t val, uint8_t len )
{
    uint8_t x;

    if ( buf == NULL )
    {
        return ( false );
    }

    for ( x = 0; x < len; x++ )
    {
        // Check for non-initialized value
        if ( buf[x] != val )
        {
            return ( false );
        }
    }
    return ( true );
}

/*********************************************************************
 * @fn        OsalPort_rand
 *
 * @brief    Random number generator
 *
 * @param   none
 *
 * @return  uint16 - new random number
 */
uint16_t OsalPort_rand( void )
{
    return (Random_getNumber() & 0xFFFF);
}

/*********************************************************************
 * @fn        OsalPort_clearTaskQueues
 *
 * @brief    Clears all Queues used for intertask communications. If
 *           needed, make it atomic at caller level
 *
 * @param   none
 *
 * @return  none
 */
void OsalPort_clearTaskQueues( void )
{
    uint8_t  i;
    uint8_t *pMsg;

    for(i = 0; i < taskCnt; i++)
    {
      while((pMsg = OsalPort_msgReceive(i)) != NULL)
      {
          OsalPort_msgDeallocate(pMsg);
      }
    }
}

