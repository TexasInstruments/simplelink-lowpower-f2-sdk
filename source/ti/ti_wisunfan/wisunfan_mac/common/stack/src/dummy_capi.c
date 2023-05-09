/**
  @file  dummy_capi.c
  @brief Dummy stack C interface implementation on top of
         dispatcher messaging interface.

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
#include "Dummy.h"
#include "DummyMSG.h"

#include <ICall.h>

/* Implementation of GAP_DeviceInit() function
 * over messaging interface using dispatcher.
 * See function API header file to find the description
 * of the function interface.
 */
bStatus_t GAP_DeviceInit( ICall_EntityID srcEntity,
                            uint8 profileRole,
                            uint8 maxScanResponses,
                            uint8 *pIRK,
                            uint8 *pSRK,
                            uint32 *pSignCounter )
{
  ICall_Errno errno;

  /* Allocate message buffer space */
  DummyMSG_GAPDeviceInit *msg = (DummyMSG_GAPDeviceInit *)
      ICall_allocMsg(sizeof(*msg));

  /* Fill in the message content */
  msg->hdr.hdr.event = HCI_EXT_PARSED_CMD_EVENT;
  msg->hdr.hdr.status = 0;
  msg->hdr.opcode = (HCI_EXT_GAP_SUBGRP << 7) | HCI_EXT_GAP_DEVICE_INIT;

  /* Note that srctaskid shall be filled in via sendmsg() */

  msg->profileRole = profileRole;
  msg->maxScanResponses = maxScanResponses;

  /* Note that decision to map the data pointer as is
   * depends on how the pointer is actually used by the other side.
   * It is safely done so here since GAP_DeviceInit() expects,
   * the buffers passed by pointers to persist while the device
   * is powered on.
   */
  msg->pIRK = pIRK;
  msg->pSRK = pSRK;
  msg->pSignCounter = (uint32_t *) pSignCounter;

  /* Send the message.
   * In real code for BLE, the message should be sent to
   * ICALL_SERVICE_CLASS_BLE. */
  errno = ICall_sendServiceMsg(srcEntity, ICALL_SERVICE_CLASS_DUMMY,
                       ICALL_MSG_FORMAT_3RD_CHAR_TASK_ID, msg);

  /* The following code could be made into a generic
   * return type conversion.
   */
  if (errno == ICALL_ERRNO_SUCCESS)
  {
    return SUCCESS;
  }
  if (errno == ICALL_ERRNO_NO_RESOURCE)
  {
    return MSG_BUFFER_NOT_AVAIL;
  }
  if (errno == ICALL_ERRNO_INVALID_PARAMETER)
  {
    return INVALIDPARAMETER;
  }
  return FAILURE;
}
