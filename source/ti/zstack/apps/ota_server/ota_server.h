/**************************************************************************************************
  Filename:       ota_server.h
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
#include "zcl_ota.h"
#include "nvintf.h"

/*********************************************************************
 * CONSTANTS
 */
#define OTA_SERVER_ENDPOINT                     ZCL_OTA_ENDPOINT
#define OTA_SAMPLE_PROFILE_ID                   0x0104
#define OTA_SAMPLE_DEVICEID                     0

// Application Events
#define SAMPLELIGHT_POLL_CONTROL_TIMEOUT_EVT  0x0001
#define SAMPLELIGHT_LEVEL_CTRL_EVT            0x0002
#define SAMPLEAPP_END_DEVICE_REJOIN_EVT       0x0004
#define SAMPLEAPP_OTA_SERVER_NOTIFY_EVT       0x0008
#define SAMPLEAPP_KEY_EVT                     0x4000

// UI Events
#define SAMPLEAPP_UI_AUTO_REFRESH_EVT         0x0010
#define SAMPLEAPP_UI_INPUT_EVT           0x0040

// Green Power Events
#define SAMPLEAPP_PROCESS_GP_DATA_SEND_EVT              0x0100
#define SAMPLEAPP_PROCESS_GP_EXPIRE_DUPLICATE_EVT       0x0200
#define SAMPLEAPP_PROCESS_GP_TEMP_MASTER_EVT            0x0400

#define SAMPLEAPP_END_DEVICE_REJOIN_DELAY 1000
#define OTA_SEND_NOTIFY_TIMEOUT           4000


/*********************************************************************
 * MACROS
 */
/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * VARIABLES
 */

extern SimpleDescriptionFormat_t otaServerSimpleDesc;

extern CONST zclCommandRec_t otaServer_Cmds[];

extern CONST uint8_t zclCmdsArraySize;

// attribute list
extern CONST zclAttrRec_t otaServer_Attrs[];
extern CONST uint8_t otaServer_NumAttributes;

// Identify attributes
extern uint16_t otaServer_IdentifyTime;
extern uint8_t  otaServer_IdentifyCommissionState;

/*********************************************************************
 * FUNCTIONS
 */

/*
 *  Reset all writable attributes to their default values.
 */
extern void otaServer_ResetAttributesToDefaultValues(void); //implemented in ota_server_data.c

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* ZCL_SAMPLELIGHT_H */
