/******************************************************************************

@file  BLEAppUtil_task.c

@brief This file contains the BLEAppUtil module task function and related
       functionality

Group: WCS, BTS
Target Device: cc13xx_cc26xx

******************************************************************************

 Copyright (c) 2022, Texas Instruments Incorporated
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


/*********************************************************************
* CONSTANTS
*/
// Task configuration
#define appTaskStack            NULL
#define BLEAPPUTIL_QUEUE_EVT    0x40000000
#define EVENT_PEND_FOREVER      0xFFFFFFFF

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
void BLEAppUtil_Task(void);

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
* LOCAL FUNCTIONS
*/

/*********************************************************************
 * @fn      BLEAppUtil_createBLEAppUtilTask
 *
 * @brief   Create the BLEAppUtil task.
 *
 * @return  None
 */
TaskP_Handle BLEAppUtil_createBLEAppUtilTask(void)
{
    const TaskP_Params taskParams = {
        .arg       = NULL,
        .priority  = BLEAppUtilLocal_GeneralParams->taskPriority,
        .stackSize = BLEAppUtilLocal_GeneralParams->taskStackSize,
        .stack     = appTaskStack
    };
    return TaskP_create((TaskP_Function)BLEAppUtil_Task, &taskParams);
}

/*********************************************************************
 * @fn      BLEAppUtil_eventPend
 *
 * @brief   Pend (blocked) on stack/application events
 *
 * @return  status of the pend
 */
uint32_t BLEAppUtil_eventPend()
{
    return EventP_pend(BLEAppUtilSyncEvent, BLEAPPUTIL_QUEUE_EVT, 1, EVENT_PEND_FOREVER);
}

/*********************************************************************
 * @fn      BLEAppUtil_queueEmpty
 *
 * @brief   check if App queue is empty
 *
 * @return  TRUE if queue is empty
 */
bool BLEAppUtil_queueEmpty()
{
    return QueueP_empty(BLEAppUtilMsgQueueHandle);
}

/*********************************************************************
 * @fn      BLEAppUtil_dequeueMsg
 *
 * @brief   dequeue message from App queue
 *
 * @return  status of the dequeue
 */
uint8_t *BLEAppUtil_dequeueMsg()
{
    return QueueP_dequeue(BLEAppUtilMsgQueueHandle);
}

/*********************************************************************
 * @fn      BLEAppUtil_enqueueMsg
 *
 * @brief   Creates a message and puts the message in application queue.
 *
 * @param   event - message event.
 * @param   state - message state.
 *
 * @return  SUCCESS - message was enqueued successfully
 * @return  FAILURE - message was not enqueued
 * @return  bleMemAllocError - could not allocate memory for the message
 */
status_t BLEAppUtil_enqueueMsg(uint8_t event, void *pData)
{
    uint8_t success;
    BLEAppUtil_appEvt_t *pMsg;

    if (BLEAppUtilMsgQueueHandle == 0)
    {
        DebugP_log0("BLEAppUtil_enqueueMsg: FATAL ERROR BLEAppUtilMsgQueueHandle is NULL");
        return(bleNotReady);
    }

    // Create dynamic pointer to message.
    pMsg = BLEAppUtil_malloc(sizeof(BLEAppUtil_appEvt_t));
    if (pMsg)
    {
        pMsg->event = event;
        pMsg->pData = pData;

        // Enqueue the message.
        success = QueueP_enqueue(BLEAppUtilMsgQueueHandle,(uint8_t *)pMsg);
        EventP_post(BLEAppUtilSyncEvent, BLEAPPUTIL_QUEUE_EVT);

        return (success) ? SUCCESS : FAILURE;
    }

    return(bleMemAllocError);
}

/*********************************************************************
 * @fn      BLEAppUtil_Task
 *
 * @brief   The BLEAppUtil task function.
 *          This function registers to get stack events, call the stack
 *          initializing function and process the events that are
 *          enqueued to it's queue.
 *          Note: The data is freed in this function, the application
 *                needs to copy the data in order to save it in order
 *                to use it outside the event handler of the application.
 *
 * @return  None
 */
void BLEAppUtil_Task(void)
{
    // Register to the stack and create queue and event
    BLEAppUtil_stackRegister();

    // Init the ble stack
    BLEAppUtil_stackInit();

    // Application main loop
    for (;;)
    {
        BLEAppUtil_appEvt_t *pAppEvt;

        // Waits for an event to be posted
        BLEAppUtil_eventPend();

        // Check if there are any messages in the queue
        while (!BLEAppUtil_queueEmpty())
        {
          pAppEvt = (BLEAppUtil_appEvt_t *)BLEAppUtil_dequeueMsg();
          BLEAppUtil_msgHdr_t *pMsgData = (BLEAppUtil_msgHdr_t *)pAppEvt->pData;
          bool freeMsg = TRUE;

          switch (pAppEvt->event)
          {
            case BLEAPPUTIL_EVT_STACK_CALLBACK:
            {
                switch (pMsgData->event)
                {
                    case GAP_MSG_EVENT:
                        BLEAppUtil_processGAPEvents(pMsgData);
                        break;

                    case GATT_MSG_EVENT:
                        BLEAppUtil_processGATTEvents(pMsgData);
                        break;

                    case L2CAP_DATA_EVENT:
                        BLEAppUtil_processL2CAPDataMsg(pMsgData);
                        break;

                    case L2CAP_SIGNAL_EVENT:
                        BLEAppUtil_processL2CAPSignalEvents(pMsgData);
                        break;

                    case HCI_GAP_EVENT_EVENT:
                        BLEAppUtil_processHCIGAPEvents(pMsgData);
                        break;

                    case HCI_DATA_EVENT:
                        BLEAppUtil_processHCIDataEvents(pMsgData);
                        break;

                    case HCI_SMP_EVENT_EVENT:
                        BLEAppUtil_processHCISMPEvents(pMsgData);
                        break;

                    case HCI_SMP_META_EVENT_EVENT:
                        BLEAppUtil_processHCISMPMetaEvents(pMsgData);
                        break;

                    default:
                        break;
                }
                break;
            }

            case BLEAPPUTIL_EVT_ADV_CB_EVENT:
            {
                BLEAppUtil_processAdvEventMsg(pMsgData);
                freeMsg = FALSE;
                if (((BLEAppUtil_AdvEventData_t *)pMsgData)->event != BLEAPPUTIL_ADV_INSUFFICIENT_MEMORY &&
                    ((BLEAppUtil_AdvEventData_t *)pMsgData)->pBuf)
                {
                    BLEAppUtil_free(((BLEAppUtil_AdvEventData_t *)pMsgData)->pBuf);
                }
                break;
            }

            case BLEAPPUTIL_EVT_SCAN_CB_EVENT:
            {
                BLEAppUtil_processScanEventMsg(pMsgData);
                freeMsg = FALSE;
                if (((BLEAppUtil_ScanEventData_t *)pMsgData)->event == BLEAPPUTIL_ADV_REPORT &&
                    ((BLEAppUtil_ScanEventData_t *)pMsgData)->pBuf->pAdvReport.pData)
                {
                    BLEAppUtil_free(((BLEAppUtil_ScanEventData_t *)pMsgData)->pBuf->pAdvReport.pData);
                }
                if (((BLEAppUtil_ScanEventData_t *)pMsgData)->event != BLEAPPUTIL_SCAN_INSUFFICIENT_MEMORY &&
                    ((BLEAppUtil_ScanEventData_t *)pMsgData)->pBuf)
                {
                    BLEAppUtil_free(((BLEAppUtil_ScanEventData_t *)pMsgData)->pBuf);
                }
                break;
            }

            case BLEAPPUTIL_EVT_PAIRING_STATE_CB:
                BLEAppUtil_processPairStateMsg(pMsgData);
                freeMsg = FALSE;
              break;

            case BLEAPPUTIL_EVT_PASSCODE_NEEDED_CB:
                BLEAppUtil_processPasscodeMsg(pMsgData);
                freeMsg = FALSE;
              break;

            case BLEAPPUTIL_EVT_CONN_EVENT_CB:
                BLEAppUtil_processConnEventMsg(pMsgData);
                freeMsg = FALSE;
              break;

            default:
                break;
          }

          // Free the data
          if (pMsgData && freeMsg)
          {
              BLEAppUtil_freeMsg(pMsgData);
          }
          else if (pMsgData && freeMsg == FALSE)
          {
              BLEAppUtil_free(pMsgData);
          }

          BLEAppUtil_free(pAppEvt);
        }
    }
}

/////////////////////////////////////////////////////////////////////////
// Help functions
/////////////////////////////////////////////////////////////////////////

/*********************************************************************
 * @fn      BLEAppUtil_convertBdAddr2Str
 *
 * @brief   Convert Bluetooth address to string.
 *
 * @param   pAddr - BD address
 *
 * @return  BD address as a string
 */
char *BLEAppUtil_convertBdAddr2Str(uint8_t *pAddr)
{
    uint8_t     charCnt;
    char        hex[] = "0123456789ABCDEF";
    static char str[(2*B_ADDR_LEN)+3];
    char        *pStr = str;

    *pStr++ = '0';
    *pStr++ = 'x';

    // Start from end of addr
    pAddr += B_ADDR_LEN;

    for (charCnt = B_ADDR_LEN; charCnt > 0; charCnt--)
    {
        *pStr++ = hex[*--pAddr >> 4];
        *pStr++ = hex[*pAddr & 0x0F];
    }
    *pStr = 0;

    return str;
}
