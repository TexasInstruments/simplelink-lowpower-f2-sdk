/******************************************************************************
 @file  osal_port.h

 @brief This API maps ICALL and OSAL API's used by the MAC to TIRTOS API's

 Group: WCS, LPC, BTS
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2004-2025, Texas Instruments Incorporated
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

#ifndef OsalPort_H
#define OsalPort_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <pthread.h>
#include <semaphore.h>
#include <ti/drivers/dpl/ClockP.h>

/*********************************************************************
 * MACROS
 */
#define OsalPort_MSG_NEXT(pMsg)      ((OsalPort_MsgHdr *) (pMsg) - 1)->next
#define OsalPort_MSG_Q_INIT(pQ)      *(pQ) = NULL
#define OsalPort_MSG_Q_EMPTY(pQ)     (*(pQ) == NULL)
#define OsalPort_MSG_Q_HEAD(pQ)      (*(pQ))
#define OsalPort_MSG_LEN(pMsg)      ((OsalPort_MsgHdr *) (pMsg) - 1)->len
#define OsalPort_MSG_ID(pMsg)      ((OsalPort_MsgHdr *) (pMsg) - 1)->dest_id

#define OsalPort_OFFSET_OF(type, member) ((uint32) &(((type *) 0)->member))

/*********************************************************************
 * CONSTANTS
 */

#define OsalPort_SYS_EVENT_MSG            0x8000

/*** Generic Status Return Values ***/
#define OsalPort_SUCCESS                   0x00
#define OsalPort_FAILURE                   0x01
#define OsalPort_INVALIDPARAMETER          0x02
#define OsalPort_INVALID_TASK              0x03
#define OsalPort_MSG_BUFFER_NOT_AVAIL      0x04
#define OsalPort_INVALID_MSG_POINTER       0x05
#define OsalPort_INVALID_EVENT_ID          0x06
#define OsalPort_INVALID_INTERRUPT_ID      0x07
#define OsalPort_NO_TIMER_AVAIL            0x08
#define OsalPort_NV_ITEM_UNINIT            0x09
#define OsalPort_NV_OPER_FAILED            0x0A
#define OsalPort_INVALID_MEM_SIZE          0x0B
#define OsalPort_NV_BAD_ITEM_LEN           0x0C

#define OsalPort_TASK_NO_TASK              0xFF

#define OsalPort_PWR_CONSERVE 0
#define OsalPort_PWR_HOLD     1

#ifdef DBG_OSAL
struct osal_debug{
    uint32_t memAllocatedCnt;
    uint32_t memDeAllocatedCnt;
    uint16_t memAllocFailures;
    uint16_t pk_drop_no_rx_buffer;
    uint16_t mac_data_req_fail_cnt;
    /* track for TIMAC stack heap usage */
    uint32_t memAllocSize;
    uint32_t memFreeSize;
    uint32_t memTotalUsage;
    uint32_t memHighWaterMark;
    uint16_t memFreeMultipleTime;
    uint16_t memAllocfail;
    uint16_t memFreeFail;
};
#endif

/*********************************************************************
 * TYPEDEFS
 */
typedef void (*OsalPort_TimerCback)(void *arg);

/** Timer id data type */
typedef void *OsalPort_TimerID;

typedef struct
{
  void   *next;

  /* OSAL port to TI-RTOS requires compatibility with ROM
   * code compiled with USE_ICALL compile flag.  */
  uint32_t  reserved;

  uint16_t len;
  uint8_t  dest_id;
} OsalPort_MsgHdr;

typedef struct
{
  uint8_t  event;
  uint8_t  status;
} OsalPort_EventHdr;

typedef void * OsalPort_MsgQ;

/*********************************************************************
 * GLOBAL VARIABLES
 */

/*********************************************************************
 * FUNCTIONS
 */

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
 * @param   Semaphore_Handle    taskSem - taskSemaphore
 * @param   uint32_t* -         pEvent - pointer to event flag
 *
 * @return  Task ID
 */
uint8_t OsalPort_registerTask(void* taskHndl, void* taskSem, uint32_t * pEvent);


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
extern uint8_t * OsalPort_msgAllocate(uint16_t len );

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
 * @return  SUCCESS, INVALID_MSG_POINTER
 */
extern uint8_t OsalPort_msgDeallocate( uint8_t *pMsg );

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
 * @return  SUCCESS, INVALID_TASK, INVALID_MSG_POINTER
 */
extern uint8_t OsalPort_msgSend( uint8_t destinationTask, uint8_t *pMsg );

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
extern uint8_t *OsalPort_msgReceive( uint8_t taskId );

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
extern OsalPort_EventHdr* OsalPort_msgFind(uint8_t taskId, uint8_t event);

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
 * @return  SUCCESS, MSG_BUFFER_NOT_AVAIL, FAILURE, INVALID_TASK
 */
extern uint8_t OsalPort_setEvent( uint8_t destinationTask, uint32_t eventFlag );

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
uint32_t OsalPort_waitEvent(uint8_t taskId);

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
void OsalPort_blockOnEvent(void* taskHndl);

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
extern void OsalPort_clearEvent(uint8_t TaskID, uint32_t eventFlag);

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
extern void OsalPort_msgEnqueue( OsalPort_MsgQ *pQ, void *pMsg );

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
extern uint8_t OsalPort_msgEnqueueMax( OsalPort_MsgQ *pQ, void *pMsg, uint8_t max );

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
extern void *OsalPort_msgDequeue( OsalPort_MsgQ *pQ );

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
extern OsalPort_EventHdr* OsalPort_msgFindDequeue(uint8_t taskId, uint8_t event);

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
extern void OsalPort_msgPush( OsalPort_MsgQ *pQ, void *pMsg );

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
extern void OsalPort_msgExtract( OsalPort_MsgQ *pQ, void *pMsg, void *pPrev );

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
 * @return  TRUE if power is required; FALSE is power is not required
 */
extern uint8_t OsalPort_pwrmgrTaskState( uint8_t taskId, uint8_t state );

/*********************************************************************
 * @fn      OsalPort_setTimer
 *
 * @brief   This function creates if necessary and starts a TIRTOS timer
 *
 * @param   ticks - Timer timeout in ticks
 *          cback - Timer Callback
 *          arg   - Timer args
 *          pClockHandle - ponter to a clock handle
 *
 * @return  0 for SUCCESS
 */
extern uint8_t OsalPort_setTimer(uint32_t ticks, OsalPort_TimerCback cback, void *arg, OsalPort_TimerID* pClockHandle);

/*********************************************************************
 * @fn      OsalPort_deleteTimer
 *
 * @brief   This function deletes and if necessary stops a TIRTOS timer
 *
 * @param   pClockHandle - ponter to a clock handle
 *
 * @return  0 for SUCCESS
 */
extern uint8_t OsalPort_deleteTimer(OsalPort_TimerID *pClockHandle);

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
 * @return  TRUE if power is required; FALSE is power is not required
 */
extern void OsalPort_stopTimer(OsalPort_TimerID *pClockHandle);

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
extern uint32_t OsalPort_buildUint32( uint8_t *swapped, uint8_t len );

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
extern uint8_t* OsalPort_bufferUint32( uint8_t *buf, uint32_t val );

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
 * @return  TRUE - same, FALSE - different
 */
uint8_t OsalPort_memcmp( const void *src1, const void *src2, uint32_t len );

/*********************************************************************
 * @fn      OsalPort_memcpy
 *
 * @brief
 *
 *   Generic memory cpy. Returns pointer to end of destination buffer
 *
 */
void *OsalPort_memcpy( void *dst, const void *src, unsigned int len );

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
void *OsalPort_revmemcpy( void *dst, const void *src, unsigned int len );

/*********************************************************************
 * @fn      OsalPort_memset
 *
 * @brief
 *
 *   Generic memory set. Returns pointer to end of destination buffer.
 *   Note that this differs from the posix return behavior, which
 *   returns buf.
 *
 * @param   buf - pointer to buffer to set
 * @param   val - byte value to set all buf bytes to
 * @param   len - number of bytes to set
 *
 * @return  pointer to end of buf
 */
void *OsalPort_memset( void *buf, uint8_t val, unsigned int len );

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
void* OsalPort_malloc(uint32_t size);

/*********************************************************************
 * @fn      OsalPort_malloc
 *
 * @brief
 *
 *   Frees memory allocated in the heap. Currently maps directly to system free
 *
 * @param   size - size of allocation
 *
 * @return  pointer o allocated memory
 */
void OsalPort_free(void* buf);


/*********************************************************************
 * @fn      OsalPort_malloc
 *
 * @brief
 *
 *   Enters a critical section
 *
 * @param   size - size of allocation
 *
 * @return  key used for exiting the critical section
 */
extern uint32_t OsalPort_enterCS(void);

/*********************************************************************
 * @fn      OsalPort_malloc
 *
 * @brief
 *
 *   Exits a critical section
 *
 * @param   key used for exiting the critical section
 *
 */
void OsalPort_leaveCS(uint32_t key);

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
uint16_t OsalPort_buildUint16( uint8_t *swapped );

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
uint32_t OsalPort_buildUint32( uint8_t *swapped, uint8_t len );

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
 * @return  TRUE if all "val"
 *          FALSE otherwise
 */
uint8_t OsalPort_isBufSet( uint8_t *buf, uint8_t val, uint8_t len );

/*********************************************************************
 * @fn        OsalPort_rand
 *
 * @brief    Random number generator
 *
 * @param   none
 *
 * @return  uint16 - new random number
 */
uint16_t OsalPort_rand( void );

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* OsalPort_H */
