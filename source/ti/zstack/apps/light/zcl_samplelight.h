/**************************************************************************************************
  Filename:       zcl_samplelight.h
  Revised:        $Date: 2014-06-19 08:38:22 -0700 (Thu, 19 Jun 2014) $
  Revision:       $Revision: 39101 $

  Description:    This file contains the Zigbee Cluster Library Home
                  Automation Sample Application.


  Copyright 2006-2014 Texas Instruments Incorporated. All rights reserved.

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

#ifndef ZCL_SAMPLELIGHT_H
#define ZCL_SAMPLELIGHT_H

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


// Added to include Touchlink Target functionality
#if defined ( BDB_TL_INITIATOR ) || defined ( BDB_TL_TARGET )
  #include "zcl_general.h"
  #include "bdb_tl_commissioning.h"
#endif
/*********************************************************************
 * CONSTANTS
 */
#define SAMPLELIGHT_ENDPOINT            8

#define SAMPLELIGHT_NUM_GRPS            2 // Needed to include Touchlink Target functionality

#define LIGHT_OFF                       0x00
#define LIGHT_ON                        0x01

// Application Events
#define SAMPLELIGHT_POLL_CONTROL_TIMEOUT_EVT  0x0001
#define SAMPLELIGHT_LEVEL_CTRL_EVT            0x0002
#define SAMPLEAPP_END_DEVICE_REJOIN_EVT       0x0004
#define SAMPLEAPP_DISCOVERY_TIMEOUT_EVT       0x0008
#define SAMPLEAPP_PROV_CONNECT_EVT            0x0080
#define SAMPLEAPP_PROV_DISCONNECT_EVT         0x0010
#define SAMPLEAPP_POLICY_UPDATE_EVT           0x0020
#define SAMPLEAPP_SYNC_ATTR_EVT               0x0040

#if defined (Z_POWER_TEST)
#define SAMPLEAPP_POWER_TEST_START_EVT    0x1000
#if defined (POWER_TEST_POLL_DATA)
#define SAMPLEAPP_POWER_TEST_ZCL_DATA_EVT 0x2000
#endif
#endif // Z_POWER_TEST

// Green Power Events
#define SAMPLEAPP_PROCESS_GP_DATA_SEND_EVT              0x0200
#define SAMPLEAPP_PROCESS_GP_EXPIRE_DUPLICATE_EVT       0x0400
#define SAMPLEAPP_PROCESS_GP_TEMP_MASTER_EVT            0x0800


#define SAMPLEAPP_END_DEVICE_REJOIN_DELAY 1000
#define SAMPLEAPP_CONFIG_SYNC_TIMEOUT     500

#if defined (BDB_TL_TARGET) || defined (BDB_TL_INITIATOR)
#define TL_BDB_FB_EVT                     0x0100
#define TL_BDB_FB_DELAY                   16000
#endif // defined ( BDB_TL_TARGET ) || defined (BDB_TL_INITIATOR)

/*********************************************************************
 * MACROS
 */
/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * VARIABLES
 */

// Added to include Touchlink Target functionality
#if defined ( BDB_TL_INITIATOR ) || defined ( BDB_TL_TARGET )
  extern bdbTLDeviceInfo_t tlSampleLight_DeviceInfo;
#endif


extern SimpleDescriptionFormat_t zclSampleLight_SimpleDesc;

extern CONST zclCommandRec_t zclSampleLight_Cmds[];

extern CONST uint8_t zclCmdsArraySize;

// attribute list
extern CONST zclAttrRec_t zclSampleLight_Attrs[];
extern CONST uint8_t zclSampleLight_NumAttributes;

// Identify attributes
extern uint16_t zclSampleLight_IdentifyTime;
extern uint8_t  zclSampleLight_IdentifyCommissionState;

// Groups attributes
extern uint8_t zclSampleLight_GroupsNameSupport;

// Scenes attributes
extern uint8_t zclSampleLight_ScenesCurrentScene;
extern uint16_t zclSampleLight_ScenesCurrentGroup;
extern uint8_t zclSampleLight_ScenesValid;
extern uint8_t zclSampleLight_ScenesNameSupport;

// Level Control Attributes
#ifdef ZCL_LEVEL_CTRL
extern uint16_t zclSampleLight_LevelRemainingTime;
extern uint16_t zclSampleLight_LevelOnOffTransitionTime;
extern uint8_t  zclSampleLight_LevelOnLevel;
extern uint16_t zclSampleLight_LevelOnTransitionTime;
extern uint16_t zclSampleLight_LevelOffTransitionTime;
extern uint8_t  zclSampleLight_LevelDefaultMoveRate;
#endif





/*********************************************************************
 * FUNCTIONS
 */

/*
 *  Reset all writable attributes to their default values.
 */
extern void zclSampleLight_ResetAttributesToDefaultValues(void); //implemented in zcl_samplelight_data.c

/*
 *  Function that updates the OnOff attribute, this will also update scene attribute.
 */
extern void zclSampleLight_updateOnOffAttribute(uint8_t OnOff);

/*
 *  Function to get the state of the OnOff attribute.
 */
extern uint8_t zclSampleLight_getOnOffAttribute(void);


/*
 *  Function to toggle the light
 */
extern void zclSampleLight_UiActionToggleLight(const int32_t _itemEntry);

/*
 *  Function to discover switch devices to report state of the light
 */
extern void zclSampleLight_UiActionSwitchDiscovery(const int32_t _itemEntry);

#ifdef ZCL_LEVEL_CTRL
/*
 *  Function that updates the CurrentLevel attribute, this will also update scene attribute.
 */
extern void zclSampleLight_updateCurrentLevelAttribute(uint8_t CurrentLevel);

/*
 *  Function to get the state of the CurrentLevel attribute.
 */
extern uint8_t zclSampleLight_getCurrentLevelAttribute(void);
#endif

#ifndef CUI_DISABLE
#if defined (ENABLE_GREENPOWER_COMBO_BASIC)
void zclSampleLight_setGPSinkCommissioningMode(const int32_t _itemEntry);
#endif
#endif
/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* ZCL_SAMPLELIGHT_H */
