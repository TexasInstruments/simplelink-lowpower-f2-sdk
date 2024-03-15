/**************************************************************************************************
  Filename:       zcl_appliance_events_alerts.h
  Revised:        $Date: 2013-10-16 16:38:58 -0700 (Wed, 16 Oct 2013) $
  Revision:       $Revision: 35701 $

  Description:    This file contains the ZCL Appliance Events & Alerts definitions.


  Copyright (c) 2019, Texas Instruments Incorporated
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
**************************************************************************************************/

#ifndef ZCL_APPLIANCE_EVENTS_ALERTS_H
#define ZCL_APPLIANCE_EVENTS_ALERTS_H

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef ZCL_APPLIANCE_EVENTS_ALERTS

/*********************************************************************
 * INCLUDES
 */
#include "zcl.h"
#include "ti_zstack_config.h"

/*********************************************************************
 * CONSTANTS
 */

  // Server Commands Received
#define COMMAND_APPLIANCE_EVENTS_ALERTS_GET_ALERTS            0x00  // no payload

  // Server Commands Generated
#define COMMAND_APPLIANCE_EVENTS_ALERTS_GET_ALERTS_RSP        0x00  // zclApplianceEventsAlerts_t
#define COMMAND_APPLIANCE_EVENTS_ALERTS_ALERTS_NOTIFICATION   0x01  // zclApplianceEventsAlerts_t
#define COMMAND_APPLIANCE_EVENTS_ALERTS_EVENT_NOTIFICATION    0x02  // zclApplianceEventsAlertsEventNotification_t

  // Event ID values
#define APPLIANCE_EVENT_ID_END_OF_CYCLE                       0x01  // End of the working cycle reached
#define APPLIANCE_EVENT_ID_TEMPERATURE_REACHED                0x04  // Set temperature reached
#define APPLIANCE_EVENT_ID_END_OF_COOKING                     0x05  // End of cooking process
#define APPLIANCE_EVENT_ID_SWITCHING_OFF                      0x06
#define APPLIANCE_EVENT_ID_WRONG_DATA                         0xf7

/*********************************************************************
 * VARIABLES
 */

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * TYPEDEFS
 */

  /*** ZCL Appliance Events & Alerts Cluster: Get Alerts Response & Alerts Notification payload ***/
typedef struct
{
  uint8_t aAlert[3];     // each alert is 24 bits
} alertStructureRecord_t;

typedef struct
{
  uint8_t alertsCount;
  alertStructureRecord_t *pAlertStructureRecord;  // variable length array based on alertsCount
} zclApplianceEventsAlerts_t;

/*** ZCL Appliance Events & Alerts Cluster: Event Notification Command payload ***/
typedef struct
{
  uint8_t eventHeader;
  uint8_t eventID;      // e.g. APPLIANCE_EVENT_ID_END_OF_CYCLE
} zclApplianceEventsAlertsEventNotification_t;


typedef ZStatus_t (*zclAppliance_Events_Alerts_GetAlerts_t)( void );
typedef ZStatus_t (*zclAppliance_Events_Alerts_GetAlertsRsp_t)( zclApplianceEventsAlerts_t *pCmd );
typedef ZStatus_t (*zclAppliance_Events_Alerts_AlertsNotification_t)( zclApplianceEventsAlerts_t *pCmd );
typedef ZStatus_t (*zclAppliance_Events_Alerts_EventNotification_t)( zclApplianceEventsAlertsEventNotification_t *pCmd );

// Register Callbacks table entry - enter function pointers for callbacks that
// the application would like to receive
typedef struct
{
  zclAppliance_Events_Alerts_GetAlerts_t                  pfnApplianceEventsAlerts_GetAlerts;
  zclAppliance_Events_Alerts_GetAlertsRsp_t               pfnApplianceEventsAlerts_GetAlertsRsp;
  zclAppliance_Events_Alerts_AlertsNotification_t         pfnApplianceEventsAlerts_AlertsNotification;
  zclAppliance_Events_Alerts_EventNotification_t          pfnApplianceEventsAlerts_EventNotification;
} zclApplianceEventsAlerts_AppCallbacks_t;

/*********************************************************************
 * VARIABLES
 */

/*********************************************************************
 * FUNCTIONS
 */

/*** Register for callbacks from this cluster library ***/
extern ZStatus_t zclApplianceEventsAlerts_RegisterCmdCallbacks( uint8_t endpoint, zclApplianceEventsAlerts_AppCallbacks_t *callbacks );

/*********************************************************************
 * @fn      zclApplianceEventsAlerts_Send_GetAlerts
 *
 * @brief   Request sent to server for Get Alerts.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclApplianceEventsAlerts_Send_GetAlerts( uint8_t srcEP, afAddrType_t *dstAddr,
                                                          uint8_t disableDefaultRsp, uint8_t seqNum );

/*********************************************************************
 * @fn      zclApplianceEventsAlerts_Send_GetAlertsRsp
 *
 * @brief   Response sent to client for Get Alerts Response.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pPayload:
 *          alertsCount - Contains the length of the alert structure array
 *          aAlert - Contains the information of the Alert
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclApplianceEventsAlerts_Send_GetAlertsRsp( uint8_t srcEP, afAddrType_t *dstAddr,
                                                             zclApplianceEventsAlerts_t *pPayload,
                                                             uint8_t disableDefaultRsp, uint8_t seqNum );

/*********************************************************************
 * @fn      zclApplianceEventsAlerts_Send_AlertsNotification
 *
 * @brief   Response sent to client for Alerts Notification.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pPayload:
 *          alertsCount - Contains the length of the alert structure array
 *          aAlert - Contains the information of the Alert
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclApplianceEventsAlerts_Send_AlertsNotification( uint8_t srcEP, afAddrType_t *dstAddr,
                                                                   zclApplianceEventsAlerts_t *pPayload,
                                                                   uint8_t disableDefaultRsp, uint8_t seqNum );

/*********************************************************************
 * @fn      zclApplianceEventsAlerts_Send_EventNotification
 *
 * @brief   Response sent to client for Event Notification.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   eventHeader - a reserved field set to 0
 * @param   eventID - Identifies the event to be notified
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclApplianceEventsAlerts_Send_EventNotification( uint8_t srcEP, afAddrType_t *dstAddr,
                                                                  uint8_t eventHeader, uint8_t eventID,
                                                                  uint8_t disableDefaultRsp, uint8_t seqNum );

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif //ZCL_APPLIANCE_EVENTS_ALERTS
#endif /* ZCL_APPLIANCE_EVENTS_ALERTS_H */
