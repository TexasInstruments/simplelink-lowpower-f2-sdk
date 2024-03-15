/**************************************************************************************************
  Filename:       zcl_samplesw.h
  Revised:        $Date: 2015-08-19 17:11:00 -0700 (Wed, 19 Aug 2015) $
  Revision:       $Revision: 44460 $


  Description:    This file contains the Zigbee Cluster Library Home
                  Automation Sample Application.


  Copyright 2006-2013 Texas Instruments Incorporated. All rights reserved.

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

#ifndef ZCL_SAMPLESW_H
#define ZCL_SAMPLESW_H

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
#define SAMPLECIE_ENDPOINT               8

#define SAMPLECIE_ALARM_MAX_DURATION    0x00F0

#define SS_IAS_WD_ALARM_DEFAULT_DURATION    0x0014
#define SS_IAS_WD_STROBE_DEFAULT_DUTY_CYCLE          50   //50%

// Events for the sample app
#define SAMPLEAPP_END_DEVICE_REJOIN_EVT        0x0001
#define SAMPLEAPP_DISCOVER_DEVICE_EVT          0x0002

// Green Power Events
#define SAMPLEAPP_PROCESS_GP_DATA_SEND_EVT              0x0100
#define SAMPLEAPP_PROCESS_GP_EXPIRE_DUPLICATE_EVT       0x0200
#define SAMPLEAPP_PROCESS_GP_TEMP_MASTER_EVT            0x0400

#define SAMPLEAPP_END_DEVICE_REJOIN_DELAY 1000

//Some time to allow the device join completely
#define SAMPLECIE_SERVICE_DISCOVERY_RETRY_PERIOD   2000


/*********************************************************************
 * MACROS
 */
/*********************************************************************
 * TYPEDEFS
 */

typedef enum
{
  TRIP_TO_PAIR = 0,
  // AUTO_ENROLL_RESPONSE, /* not supported */
  AUTO_ENROLL_REQUEST,
  ENROLLMENT_MODE_ENUM_LENGTH,
} IAS_Zone_EnrollmentModes_t;

/*********************************************************************
 * VARIABLES
 */
extern SimpleDescriptionFormat_t zclSampleCIE_SimpleDesc;

extern CONST zclAttrRec_t zclSampleCIE_Attrs[];

extern uint16_t zclSampleCIE_IdentifyTime;

extern CONST uint8_t zclSampleCIE_NumAttributes;

/*********************************************************************
 * FUNCTIONS
 */

/*
 *  Reset all writable attributes to their default values.
 */
extern void zclSampleCIE_ResetAttributesToDefaultValues(void); //implemented in zcl_samplesw_data.c
#ifndef CUI_DISABLE
extern void zclSampleCIE_UiActionSendSquawk(const int32_t _itemEntry);
extern void zclSampleCIE_UiActionDiscoverZone(const int32_t _itemEntry);
extern void zclSampleCIE_UiActionConfigureServiceDiscovery(const char _input, char* _pLines[3], CUI_cursorInfo_t* _pCurInfo);
#endif

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* ZCL_SAMPLEAPP_H */
