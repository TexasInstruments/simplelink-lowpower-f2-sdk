/**************************************************************************************************
  Filename:       zcl_appliance_statistics.h
  Revised:        $Date: 2013-10-16 16:38:58 -0700 (Wed, 16 Oct 2013) $
  Revision:       $Revision: 35701 $

  Description:    This file contains the ZCL Appliance Statistics definitions.


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

#ifndef ZCL_APPLIANCE_STATISTICS_H
#define ZCL_APPLIANCE_STATISTICS_H

#ifdef ZCL_APPLIANCE_STATISTICS

#ifdef __cplusplus
extern "C"
{
#endif

/******************************************************************************
 * INCLUDES
 */
#include "zcl.h"
#include "ti_zstack_config.h"

/******************************************************************************
 * CONSTANTS
 */

/************************************************/
/***  Appliance Statistics Cluster Attributes ***/
/************************************************/

// Server Attributes
#define ATTRID_APPLIANCE_STATISTICS_LOG_MAX_SIZE                             0x0000  // M, R, uint32_t
#define ATTRID_APPLIANCE_STATISTICS_LOG_QUEUE_MAX_SIZE                       0x0001  // M, R, uint8_t


// Server Attribute Defaults
#define ATTR_DEFAULT_APPLIANCE_STATISTICS_LOG_MAX_SIZE                       0x0000003C
#define ATTR_DEFAULT_APPLIANCE_STATISTICS_LOG_QUEUE_MAX_SIZE                 0x01

// Server commands received (Server-to-Client in ZCL Header)
#define COMMAND_APPLIANCE_STATISTICS_LOG_NOTIFICATION                   0x00  // M, zclApplianceStatisticsLogNotification_t
#define COMMAND_APPLIANCE_STATISTICS_LOG_RSP                            0x01  // M, zclApplianceStatisticsLogRsp_t
#define COMMAND_APPLIANCE_STATISTICS_LOG_QUEUE_RSP                      0x02  // M, zclApplianceStatisticsLogQueueRsp_t
#define COMMAND_APPLIANCE_STATISTICS_STATISTICS_AVAILABLE               0x03  // M, zclApplianceStatisticsStatisticsAvailable_t

// Client commands generated (Client-to-Server in ZCL Header)
#define COMMAND_APPLIANCE_STATISTICS_LOG_REQ                            0x00  // M, zclApplianceStatisticsLogReq_t
#define COMMAND_APPLIANCE_STATISTICS_LOG_QUEUE_REQ                      0x01  // M, no payload

/*******************************************************************************
 * TYPEDEFS
 */

/*** SERVER COMMANDS GENERATED ***/

/*** ZCL Appliance Statistics Log Notification payload ***/
typedef struct
{
  uint32_t timeStamp;
  uint32_t logID;
  uint32_t logLength;
  uint8_t *pLogPayload;
} zclCmdApplianceStatisticsLogNotificationPayload_t;

typedef struct
{
  uint32_t timeStamp;
  uint32_t logID;
  uint32_t logLength;
  uint8_t *pLogPayload;
} zclCmdApplianceStatisticsLogRspPayload_t;

typedef struct
{
  uint8_t logQueueSize;
  uint32_t *pLogID;
} zclCmdApplianceStatisticsLogQueueRspPayload_t;

typedef struct
{
  uint8_t logQueueSize;
  uint32_t *pLogID;
} zclCmdApplianceStatisticsStatisticsAvailablePayload_t;

/*** CLIENT COMMANDS GENERATED ***/
typedef struct
{
  uint32_t logID;
} zclApplianceStatisticsLogReq_t;

// prototypes for each command
typedef ZStatus_t (*zclAppliance_Statistics_LogNotification_t)( zclCmdApplianceStatisticsLogNotificationPayload_t *pCmd );
typedef ZStatus_t (*zclAppliance_Statistics_LogRsp_t)( zclCmdApplianceStatisticsLogRspPayload_t *pCmd );
typedef ZStatus_t (*zclAppliance_Statistics_LogQueueRsp_t)( zclCmdApplianceStatisticsLogQueueRspPayload_t *pCmd );
typedef ZStatus_t (*zclAppliance_Statistics_StatisticsAvailable_t)( zclCmdApplianceStatisticsStatisticsAvailablePayload_t *pCmd );
typedef ZStatus_t (*zclAppliance_Statistics_LogReq_t)( zclIncoming_t *pInMsg, zclApplianceStatisticsLogReq_t *pCmd );
typedef ZStatus_t (*zclAppliance_Statistics_LogQueueReq_t)( zclIncoming_t *pInMsg );

// Register Callbacks table entry - enter function pointers for callbacks that
// the application would like to receive

typedef struct
{
  zclAppliance_Statistics_LogNotification_t               pfnApplianceStatistics_LogNotification;   // server-side functions
  zclAppliance_Statistics_LogRsp_t                        pfnApplianceStatistics_LogRsp;
  zclAppliance_Statistics_LogQueueRsp_t                   pfnApplianceStatistics_LogQueueRsp;
  zclAppliance_Statistics_StatisticsAvailable_t           pfnApplianceStatistics_StatisticsAvailable;
  zclAppliance_Statistics_LogReq_t                        pfnApplianceStatistics_LogReq;            // client-side functions
  zclAppliance_Statistics_LogQueueReq_t                   pfnApplianceStatistics_LogQueueReq;
} zclApplianceStatistics_AppCallbacks_t;

/******************************************************************************
 * FUNCTION MACROS
 */

/******************************************************************************
 * VARIABLES
 */

/******************************************************************************
 * FUNCTIONS
 */
/*
 * Register for callbacks from this cluster library
 */
extern ZStatus_t zclApplianceStatistics_RegisterCmdCallbacks( uint8_t endpoint, zclApplianceStatistics_AppCallbacks_t *callbacks );

/*********************************************************************
 * @fn      zclApplianceStatistics_Send_LogNotification
 *
 * @brief   Request sent to client for Log Notification.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pPayload - command payload
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclApplianceStatistics_Send_LogNotification( uint8_t srcEP, afAddrType_t *dstAddr,
                                                              zclCmdApplianceStatisticsLogNotificationPayload_t *pPayload,
                                                              uint8_t disableDefaultRsp, uint8_t seqNum );

/*********************************************************************
 * @fn      zclApplianceStatistics_Send_LogRsp
 *
 * @brief   Response sent to client from LogReq cmd.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pPayload:
 *          timeStamp - timestamp of the notification
 *          logID - identifies uniquely the log information contained in log payload
 *          logLength - indicates the length in bytes of log payload
 *          logPayload - variable length payload
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclApplianceStatistics_Send_LogRsp( uint8_t srcEP, afAddrType_t *dstAddr,
                                                     zclCmdApplianceStatisticsLogRspPayload_t *pPayload,
                                                     uint8_t disableDefaultRsp, uint8_t seqNum );

/*********************************************************************
 * @fn      zclApplianceStatistics_Send_LogQueueRsp
 *
 * @brief   Response sent to client from LogQueueReq cmd.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pPayload:
 *          logQueueSize - defines the # of logID records
 *          logID - variable length list of logIDs
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclApplianceStatistics_Send_LogQueueRsp( uint8_t srcEP, afAddrType_t *dstAddr,
                                                          zclCmdApplianceStatisticsLogQueueRspPayload_t *pPayload,
                                                          uint8_t disableDefaultRsp, uint8_t seqNum );

/*********************************************************************
 * @fn      zclApplianceStatistics_Send_StatisticsAvailable
 *
 * @brief   Request sent to client for Statistics Available.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pPayload:
 *          logQueueSize - # elements in the logID list
 *          logID - variable length list of logIDs
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclApplianceStatistics_Send_StatisticsAvailable( uint8_t srcEP, afAddrType_t *dstAddr,
                                                           zclCmdApplianceStatisticsStatisticsAvailablePayload_t *pPayload,
                                                           uint8_t disableDefaultRsp, uint8_t seqNum );

/*********************************************************************
 * @fn      zclApplianceStatistics_Send_LogReq
 *
 * @brief   Request sent to server for Log Request.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   logID - identifies uniquely the log information contained in log payload
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclApplianceStatistics_Send_LogReq( uint8_t srcEP, afAddrType_t *dstAddr,
                                                     uint32_t logID,
                                                     uint8_t disableDefaultRsp, uint8_t seqNum );

/*********************************************************************
 * @fn      zclApplianceStatistics_Send_LogQueueReq
 *
 * @brief   Request sent to server for Log Queue Request.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclApplianceStatistics_Send_LogQueueReq( uint8_t srcEP, afAddrType_t *dstAddr,
                                                          uint8_t disableDefaultRsp, uint8_t seqNum );

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif
#endif // ZCL_APPLIANCE_STATISTICS
#endif /* ZCL_APPLIANCE_STATISTICS_H */
