/**
  @file  DummyStack.c
  @brief Dummy stack implementation

  <!--
  Copyright 2013 Texas Instruments Incorporated. All rights reserved.

  IMPORTANT: Your use of this Software is limited to those specific rights
  granted under the terms of a software license agreement between the user
  who downloaded the software, his/her employer (which must be your employer)
  and Texas Instruments Incorporated (the "License").  You may not use this
  Software unless you agree to abide by the terms of the License. The License
  limits your use, and you acknowledge, that the Software may not be modified,
  copied or distributed unless embedded on a Texas Instruments microcontroller
  or used solely and exclusively in conjunction with a Texas Instruments radio
  frequency transceiver, which is integrated into your product.  Other than for
  the foregoing purpose, you may not use, reproduce, copy, prepare derivative
  works of, modify, distribute, perform, display or sell this Software and/or
  its documentation for any purpose.

  YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE
  PROVIDED ``AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
  INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, TITLE,
  NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL
  TEXAS INSTRUMENTS OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER CONTRACT,
  NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER
  LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
  INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE
  OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT
  OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
  (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.

  Should you have any questions regarding your right to use this Software,
  contact Texas Instruments Incorporated at www.TI.com.
  -->
*/
#include <hal_mcu.h>
#include <OSAL.h>
#include <ICall.h>
#include <DummyBoard.h>

#include "DummyMSG.h"

#include "DummyStack.h"

#ifdef TEST_BLEBOARD
#include <hal_board_cfg.h>
#endif /* TEST_BLEBOARD */

/** @internal OSAL timer event to turn on flashing LED */
#define DUMMYSTACK_EVENT_TIMER_GPIO_HIGH        1

/** @internal OSAL timer event to turn off flashing LED */
#define DUMMYSTACK_EVENT_TIMER_GPIO_LOW         2

/** @internal OSAL task ID for Dummy stack */
static uint8 dummystack_id;

/**
 * @internal
 * OSAL task ID for a sub service OSAL task which is used to demonstrate
 * osal_enroll_senderid() function
 */
static uint8 dummystack_subserviceid;

/** @internal OSAL task ID for client that sent GAP_DeviceInit() message */
static uint8 dummystack_clientid;

#ifdef TEST_BLEBOARD
/* This code is just for testing BLEBoard service module
 * DummyStack application does not require this code. */
/** @internal counter to alternate use of LEDs */
static uint_least8_t count = 0;
#endif /* TEST_BLEBOARD */

/**
 * OSAL task initialization handler
 * @param taskId  OSAL task ID assigned to the dummy stack.
 */
void DummyStack_init(uint8 taskId)
{
  dummystack_id = taskId;

  /* Start a timer to show demo of GPIO access */
  osal_start_timerEx(taskId, DUMMYSTACK_EVENT_TIMER_GPIO_HIGH,
                     500);
}

/**
 * OSAL task event handler
 *
 * @param taskId  OSAL task ID of the Dummy stack
 * @param events  event flag bit field
 */
uint16 DummyStack_event(uint8 taskId, uint16 events)
{
  (void)taskId;

  /* handle events on osal msg queue */
  if (events & SYS_EVENT_MSG)
  {
    DummyMSG_Any *msg_ptr =
        (DummyMSG_Any *) osal_msg_receive(dummystack_id);
    while (msg_ptr != NULL)
    {
      if (msg_ptr->hdr.event == HCI_EXT_PARSED_CMD_EVENT)
      {
        if ((msg_ptr->hciExtCmd.opcode >> 7) == HCI_EXT_GAP_SUBGRP)
        {
          if ((msg_ptr->hciExtCmd.opcode & 0x7f) ==
              HCI_EXT_GAP_DEVICE_INIT)
          {
            /* GAP Device Init */
            gapDeviceInitDoneEvent_t *event = (gapDeviceInitDoneEvent_t *)
                osal_msg_allocate(sizeof(gapDeviceInitDoneEvent_t));

            if (event)
            {
              event->hdr.event = GAP_MSG_EVENT;
              event->hdr.status = SUCCESS;
              event->opcode = GAP_DEVICE_INIT_DONE_EVENT;
              event->dataPktLen = 0;
              event->numDataPkts = 0;

              /* Store the client ID */
              dummystack_clientid = msg_ptr->gapDeviceInit.hdr.srctaskid;

              /* Just to demonstrate use of osal_enroll_senderid(),
               * the message is sent to the 2nd OSAL task,
               * which then forward the message back to the source
               */
              osal_msg_send(dummystack_subserviceid,
                            (uint8 *) event);
            }
          }
        }
        osal_msg_deallocate((uint8 *)msg_ptr);
      }
      else if (msg_ptr->hdr.event == DUMMYMSG_ECHO)
      {
        /* Echo */
        osal_msg_send(msg_ptr->echo.srctaskid, (uint8 *) msg_ptr);
      }
      else
      {
        /* Unknown message */
        osal_msg_deallocate((uint8 *)msg_ptr);
      }
      msg_ptr = (DummyMSG_Any *) osal_msg_receive(dummystack_id);
    }
  }
  if (events & DUMMYSTACK_EVENT_TIMER_GPIO_HIGH)
  {
    DummyBoard_gpioWrite(DUMMYBOARD_GPIO_OUTPUT_FLASH, TRUE);
    /* Re-start a timer to show demo of GPIO access */
    osal_start_timerEx(taskId, DUMMYSTACK_EVENT_TIMER_GPIO_LOW,
                       500);

#ifdef TEST_BLEBOARD
    /* This is test code to test BLEBoard module,
     * and is irrelevant to Dummy Stack sample application. */
    if (count == 0)
    {
      HAL_TURN_OFF_LED3();
      HAL_TURN_ON_LED1();
    }
    else if (count == 1)
    {
      HAL_TURN_OFF_LED1();
      HAL_TURN_ON_LED2();
    }
    else if (count == 2)
    {
      HAL_TURN_OFF_LED2();
      HAL_TURN_ON_LED3();
    }
    count = (count + 1) % 3;
#endif /* TEST_BLEBOARD */

  }
  if (events & DUMMYSTACK_EVENT_TIMER_GPIO_LOW)
  {
    DummyBoard_gpioWrite(DUMMYBOARD_GPIO_OUTPUT_FLASH, FALSE);
    /* Re-start a timer to show demo of GPIO access */
    osal_start_timerEx(taskId, DUMMYSTACK_EVENT_TIMER_GPIO_HIGH,
                       500);
  }

  return 0;
}

/**
 * OSAL task initialization handler for another OSAL task,
 * in order to demonstrate use of osal
 * @param taskId  OSAL task ID assigned to the dummy stack.
 */
void DummyStack_subserviceInit(uint8 taskId)
{
  /* This task is not enrolled as a service */
  dummystack_subserviceid = taskId;
}

/**
 * OSAL task event handler for an OSAL task that forwards received message
 * to the client task.
 * This task is just to demonstrate that osal_enroll_senderid() works.
 *
 * @param taskId  OSAL task ID of the Dummy stack
 * @param events  event flag bit field
 */
uint16 DummyStack_subserviceEvent(uint8 taskId, uint16 events)
{
  (void)taskId;

  /* handle events on osal msg queue */
  if (events & SYS_EVENT_MSG)
  {
    uint8 *msg_ptr = osal_msg_receive(dummystack_subserviceid);
    while (msg_ptr != NULL)
    {
      osal_msg_send(dummystack_clientid, (uint8 *) msg_ptr);
      msg_ptr = osal_msg_receive(dummystack_id);
    }
  }
  return 0;
}
