/**************************************************************************************************
  Filename:       zcl_sampledoorlock.h
  Revised:        $Date: 2013-06-12 16:07:11 -0700 (Wed, 12 Jun 2013) $
  Revision:       $Revision: 34551 $

  Description:    This file contains the Zigbee Cluster Library Home
                  Automation Sample Application.


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
  PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
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
**************************************************************************************************/

#ifndef ZCL_SAMPLEDOORLOCK_H
#define ZCL_SAMPLEDOORLOCK_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "zcl.h"
#include "nvintf.h"
#ifndef CUI_DISABLE
#include "cui.h"
#endif

/*********************************************************************
 * CONSTANTS
 */
#define SAMPLEDOORLOCK_ENDPOINT           8


#define ZCLTESTAPP_DOORLOCK_ATTRIBUTES    7

#define LIGHT_OFF                       0x00
#define LIGHT_ON                        0x01

// Application Events
#define SAMPLEDOORLOCK_POLL_CONTROL_TIMEOUT_EVT     0x0001
#define SAMPLEAPP_END_DEVICE_REJOIN_EVT             0x0004

// Green Power Events
#define SAMPLEAPP_PROCESS_GP_DATA_SEND_EVT              0x0100
#define SAMPLEAPP_PROCESS_GP_EXPIRE_DUPLICATE_EVT       0x0200
#define SAMPLEAPP_PROCESS_GP_TEMP_MASTER_EVT            0x0400

// NV PIN Information
#define DLSAPP_NV_DOORLOCK_PIN          0x0010
#define DLSAPP_NV_DOORLOCK_PIN_LEN      5
#define DLSAPP_MAX_PIN_SIZE             4

#define SAMPLEAPP_END_DEVICE_REJOIN_DELAY 1000

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * VARIABLES
 */
extern SimpleDescriptionFormat_t zclSampleDoorLock_SimpleDesc;

extern CONST zclAttrRec_t zclSampleDoorLock_Attrs[];

extern CONST uint8_t zclSampleDoorLock_NumAttributes;

extern uint8_t  zclSampleDoorLock_OnOff;

extern uint16_t zclSampleDoorLock_IdentifyTime;

//Door Lock Cluster Attributes
extern uint8_t zclSampleDoorLock_LockType;
extern bool zclSampleDoorLock_ActuatorEnabled;

// Scenes attributes
extern uint8_t zclSampleDoorLock_ScenesCurrentScene;
extern uint16_t zclSampleDoorLock_ScenesCurrentGroup;
extern uint8_t zclSampleDoorLock_ScenesValid;
extern uint8_t zclSampleDoorLock_ScenesNameSupport;
/*********************************************************************
 * FUNCTIONS
 */

/*
 *  Reset all writable attributes to their default values.
 */
extern void zclSampleDoorLock_ResetAttributesToDefaultValues(void);

/*
 *  Function that updates the Lock State attribute, this will also update scene attribute.
 */
extern void zclSampleDoorLock_updateLockStateAttribute(uint8_t LockState);

/*
 *  Function to get the state of the Lock State attribute.
 */
extern uint8_t zclSampleDoorLock_getLockStateAttribute(void);


#ifndef CUI_DISABLE
extern void zclSampleDoorLock_UiActionEnterPin(const char _input, char* _lines[3], CUI_cursorInfo_t * _curInfo);
extern void zclSampleDoorLock_UiActionDoorLockDiscoverable(const int32_t _itemEntry);
extern void zclSampleDoorLock_UiActionLock(const int32_t _itemEntry);
extern void zclSampleDoorLock_UiActionUnlock(const int32_t _itemEntry);
#endif
/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* ZCL_SAMPLEDOORLOCK_H */
