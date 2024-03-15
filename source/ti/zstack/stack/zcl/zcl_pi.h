/**************************************************************************************************
  Filename:       zcl_pi.h
  Revised:        $Date: 2013-10-16 16:38:58 -0700 (Wed, 16 Oct 2013) $
  Revision:       $Revision: 35701 $

  Description:    This file contains the ZCL Protocol Interfaces Definitions


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

#ifndef ZCL_PI_H
#define ZCL_PI_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "zcl.h"
#include "ti_zstack_config.h"

#ifdef ZCL_PI
/*********************************************************************
 * CONSTANTS
 */

/*************************************************/
/***   Protocol Interface Cluster Attributes   ***/
/*************************************************/
  // Attributes of the Generic Tunnel cluster
#define ATTRID_PI_GENERIC_TUNNEL_MAX_IN_TRANSFER_SIZE      0x0001
#define ATTRID_PI_GENERIC_TUNNEL_MAX_OUT_TRANSFER_SIZE     0x0002
#define ATTRID_PI_GENERIC_TUNNEL_PROTOCOL_ADDR             0x0003

  // The BACnet Protocol Tunnel cluster does not contain any attributes

  // Attributes of the 11073 Protocol Tunnel cluster
#define ATTRID_PI_11073_TUNNEL_DEVICEID_LIST               0x0000
#define ATTRID_PI_11073_TUNNEL_MANAGER_TARGET              0x0001
#define ATTRID_PI_11073_TUNNEL_MANAGER_ENDPOINT            0x0002
#define ATTRID_PI_11073_TUNNEL_CONNECTED                   0x0003
#define ATTRID_PI_11073_TUNNEL_PREEMPTIBLE                 0x0004
#define ATTRID_PI_11073_TUNNEL_IDLE_TIMEOUT                0x0005

  // Attributes of the Input, Output and Value (BACnet Reqular) cluster
#define ATTRID_IOV_BACNET_REG_CHANGE_OF_STATE_CNT          0x000F
#define ATTRID_IOV_BACNET_REG_CHANGE_OF_STATE_TIME         0x0010
#define ATTRID_IOV_BACNET_REG_COV_INCREMENT                0x0016
#define ATTRID_IOV_BACNET_REG_DEVICE_TYPE                  0x001F
#define ATTRID_IOV_BACNET_REG_ELAPSED_ACT_TIME             0x0021
#define ATTRID_IOV_BACNET_REG_FEEDBACK_VALUE               0x0028
#define ATTRID_IOV_BACNET_REG_OBJECT_ID                    0x004B
#define ATTRID_IOV_BACNET_REG_OBJECT_NAME                  0x004D
#define ATTRID_IOV_BACNET_REG_OBJECT_TYPE                  0x004F
#define ATTRID_IOV_BACNET_REG_UPDATE_INT                   0x0076
#define ATTRID_IOV_BACNET_REG_TIME_OF_AT_RESET             0x0072
#define ATTRID_IOV_BACNET_REG_TIME_OF_SC_RESET             0x0073
#define ATTRID_IOV_BACNET_REG_PROFILE_NAME                 0x00A8

  // Attributes of the Input, Output and Value (BACnet Extended) cluster
#define ATTRID_IOV_BACNET_EXT_ACKED_TRANSIT                0x0000
#define ATTRID_IOV_BACNET_EXT_ALARM_VALUE                  0x0006
//#define ATTRID_IOV_BACNET_EXT_ALARM_VALUES                 0x0006
#define ATTRID_IOV_BACNET_EXT_NOTIFY_CLASS                 0x0011
#define ATTRID_IOV_BACNET_EXT_DEADBAND                     0x0019
#define ATTRID_IOV_BACNET_EXT_EVENT_ENABLE                 0x0023
#define ATTRID_IOV_BACNET_EXT_EVENT_STATE                  0x0024
#define ATTRID_IOV_BACNET_EXT_FAULT_VALUES                 0x0025
#define ATTRID_IOV_BACNET_EXT_HI_LIMIT                     0x002D
#define ATTRID_IOV_BACNET_EXT_LIMIT_ENABLE                 0x0034
#define ATTRID_IOV_BACNET_EXT_LO_LIMIT                     0x003B
#define ATTRID_IOV_BACNET_EXT_NOTIFY_TYPE                  0x0048
#define ATTRID_IOV_BACNET_EXT_TIME_DELAY                   0x0071
#define ATTRID_IOV_BACNET_EXT_EV_TIME_STAMPS               0x0082

/*** Connect Control field: 8-bit bitmap ***/
// Preemptible (bit 1) - indicates whether or not this connection can be
// removed by a different Data Management device.
#define CONNECT_CTRL_PREEMPTIBLE_BIT                       0x01

/*** Connect status values ***/
#define CONNECT_STATUS_DISCONNECTED                        0x00
#define CONNECT_STATUS_CONNECTED                           0x01
#define CONNECT_STATUS_NOT_AUTHORIZED                      0x02
#define CONNECT_STATUS_RECONNECT_REQ                       0x03
#define CONNECT_STATUS_ALREADY_CONNECTED                   0x04

/*** AckedTransitions attribute bits ***/
#define ACKED_TRANSIT_TO_OFFNORMAL                         0x01
#define ACKED_TRANSIT_TO_FAULT                             0x02
#define ACKED_TRANSIT_TO_NORMAL                            0x04

/*** EventState attribute values ***/
#define EVENT_STATE_NORMAL                                 0x00
#define EVENT_STATE_FAULT                                  0x01
#define EVENT_STATE_OFFNORMAL                              0x02
#define EVENT_STATE_HIGH_LIMIT                             0x03
#define EVENT_STATE_LOW_LIMIT                              0x04

/*** NotifyType attribute values ***/
#define NORIFY_TYPE_EVENTS                                 0x00
#define NORIFY_TYPE_ALARMS                                 0x01

/*************************************************/
/***    Protocol Interface Cluster Commands    ***/
/*************************************************/
  // Command IDs for the Generic Tunnel Cluster
#define COMMAND_PI_GENERIC_TUNNEL_MATCH_PROTOCOL_ADDR      0x00

#define COMMAND_PI_GENERIC_TUNNEL_MATCH_PROTOCOL_ADDR_RSP  0x00
#define COMMAND_PI_GENERIC_TUNNEL_ADVERTISE_PROTOCOL_ADDR  0x01

  // Command IDs for the BACnet Protocol Tunnel Cluster
#define COMMAND_PI_BACNET_TUNNEL_TRANSFER_NPDU             0x00

  // Command IDs for the 11073 Protocol Tunnel Cluster
#define COMMAND_PI_11073_TUNNEL_TRANSFER_APDU              0x00
#define COMMAND_PI_11073_TUNNEL_CONNECT_REQ                0x01
#define COMMAND_PI_11073_TUNNEL_DISCONNECT_REQ             0x02
#define COMMAND_PI_11073_TUNNEL_CONNECT_STATUS_NOTI        0x03


    //Command IDs for the 7818 Protocol Tunnel Cluster
#define COMMAND_PI_7818_TUNNEL_TRANSFER_APDU                0x00
#define COMMAND_PI_7818_TUNNEL_INSERT_SMART_CARD            0x01
#define COMMAND_PI_7818_TUNNEL_EXTRACT_SMART_CARD           0x02


/************************************************************************************
 * MACROS
 */


/****************************************************************************
 * TYPEDEFS
 */
/*** Structures used for callback functions ***/

/*** ZCL Generic Tunnel Cluster: Match Protocol Address command ***/
typedef struct
{
  afAddrType_t *srcAddr;  // requestor's address
  uint8_t seqNum;           // sequence number received with command
  uint8_t len;              // length of address protocol
  uint8_t *protocolAddr;    // protocol address
} zclPIMatchProtocolAddr_t;

/*** ZCL Generic Tunnel Cluster: Match Protocol Address Response ***/
typedef struct
{
  afAddrType_t *srcAddr;  // responder's address
  uint8_t *ieeeAddr;        // device address
  uint8_t len;              // length of address protocol
  uint8_t *protocolAddr;    // protocol address
} zclPIMatchProtocolAddrRsp_t;

/*** ZCL Generic Tunnel Cluster: Advertise Protocol Address command ***/
typedef struct
{
  afAddrType_t *srcAddr;  // requestor's address
  uint8_t len;              // length of address protocol
  uint8_t *protocolAddr;    // protocol address
} zclPIAdvertiseProtocolAddr_t;

/*** ZCL BACnet Protocol Tunnel Cluster: Transfer NPDU command ***/
typedef struct
{
  afAddrType_t *srcAddr;  // requestor's address
  uint16_t len;             // length of BACnet NPDU
  uint8_t *npdu;            // BACnet NPDU
} zclBACnetTransferNPDU_t;


typedef struct
{
    afAddrType_t *srcAddr;  // requestor's address
    uint8_t status;
}zcl7818TransferAPDU_t;

typedef struct
{
    afAddrType_t *srcAddr;  // requestor's address
}zcl7818InsertSmartCard_t;

typedef struct
{
    afAddrType_t *srcAddr;  // requestor's address
}zcl7818ExtractSmartCard_t;


/*** ZCL 11073 Protocol Tunnel Cluster: Transfer APDU command ***/
typedef struct
{
  afAddrType_t *srcAddr;  // requestor's address
  uint16_t len;             // length of 11073 APDU
  uint8_t *apdu;            // 11073 APDU
} zcl11073TransferAPDU_t;

/*** ZCL 11073 Protocol Tunnel Cluster: Connect Request command ***/
typedef struct
{
  afAddrType_t *srcAddr; // requestor's address
  uint8_t seqNum;          // sequence number received with command
  uint8_t connectCtrl;     // connect control
  uint16_t idleTimeout;    // inactivity time (in minutes) which Data Management device
                         // will wait w/o receiving any data before it disconnects
  uint8_t *managerAddr;    // IEEE address (64-bit) of Data Management device
                         // transmitting this frame
  uint8_t managerEP;       // source endpoint from which Data Management device is
                         // transmitting this frame
} zcl11073ConnectReq_t;

/*** ZCL 11073 Protocol Tunnel Cluster: Disconnect Request command ***/
typedef struct
{
  afAddrType_t *srcAddr; // requestor's address
  uint8_t seqNum;          // sequence number received with command
  uint8_t *managerAddr;    // IEEE address (64-bit) of Data Management device
                         // transmitting this frame
} zcl11073DisconnectReq_t;

/*** ZCL 11073 Protocol Tunnel Cluster: Connect Status Notification command ***/
typedef struct
{
  afAddrType_t *srcAddr; // requestor's address
  uint8_t connectStatus;   // connect status
} zcl11073ConnectStatusNoti_t;





// This callback is called to process a Match Protocol Address command
typedef void (*zclPICB_MatchProtocolAddr_t)( zclPIMatchProtocolAddr_t *pCmd );

// This callback is called to process a Match Protocol Address response
typedef void (*zclPICB_MatchProtocolAddrRsp_t)( zclPIMatchProtocolAddrRsp_t *pRsp );

// This callback is called to process a Advertise Protocol Address command
typedef void (*zclPICB_AdvertiseProtocolAddr_t)( zclPIAdvertiseProtocolAddr_t *pCmd );

// This callback is called to process a BACnet Transfer NPDU command
typedef void (*zclPICB_BACnetTransferNPDU_t)( zclBACnetTransferNPDU_t *pCmd );

/* 7818*/
typedef void (*zclPICB_7818TransferAPDU_t)( zcl7818TransferAPDU_t *pCmd );

typedef void (*zclPICB_7818InsertSmartCard_t)( zcl7818InsertSmartCard_t *pCmd );

typedef void (*zclPICB_7818ExtractSmartCard_t)( zcl7818ExtractSmartCard_t *pCmd );


// This callback is called to process an 11037 Transfer APDU command
typedef void (*zclPICB_11073TransferAPDU_t)( zcl11073TransferAPDU_t *pCmd );

// This callback is called to process an 11037 Connect Request command
typedef void (*zclPICB_11073ConnectReq_t)( zcl11073ConnectReq_t *pCmd );

// This callback is called to process an 11037 Disconnect Request command
typedef void (*zclPICB_11073DisconnectReq_t)( zcl11073DisconnectReq_t *pCmd );

// This callback is called to process an 11037 Connect Status Notification command
typedef void (*zclPICB_11073ConnectStatusNoti_t)( zcl11073ConnectStatusNoti_t *pCmd );




// Register Callbacks table entry - enter function pointers for callbacks that
// the application would like to receive
typedef struct
{
  zclPICB_MatchProtocolAddr_t      pfnPI_MatchProtocolAddr;
  zclPICB_MatchProtocolAddrRsp_t   pfnPI_MatchProtocolAddrRsp;
  zclPICB_AdvertiseProtocolAddr_t  pfnPI_AdvertiseProtocolAddr;
  zclPICB_BACnetTransferNPDU_t     pfnPI_BACnetTransferNPDU;
  zclPICB_7818TransferAPDU_t       pfnPI_7818TransferAPDU;
  zclPICB_7818InsertSmartCard_t    pfnPI_7818InsertSmartCard;
  zclPICB_7818ExtractSmartCard_t   pfnPI_7818ExtractSmartCard;
  zclPICB_11073TransferAPDU_t      pfnPI_11073TransferAPDU;
  zclPICB_11073ConnectReq_t        pfnPI_11073ConnectReq;
  zclPICB_11073DisconnectReq_t     pfnPI_11073DisconnectReq;
  zclPICB_11073ConnectStatusNoti_t pfnPI_11073ConnectStatusNoti;
} zclPI_AppCallbacks_t;


/*********************************************************************
 * FUNCTION MACROS
 */

/*
 *  Send a BACnet Transfer NPDU Command. This command is used when a
 *  BACnet network layer wishes to transfer a BACnet NPDU across a
 *  ZigBee tunnel to another BACnet network layer.
 *
 *  Use like: ZStatus_t zclPI_Send_BACnetTransferNPDUCmd( uint16_t srcEP, afAddrType_t *dstAddr,
 *                                                        uint16_t len, uint8_t *npdu,
 *                                                        uint8_t disableDefaultRsp, uint8_t seqNum );
 *  @param   srcEP - Sending application's endpoint
 *  @param   dstAddr - where you want the message to go
 *  @param   len - length of NPDU
 *  @param   npdu - pointer to NPDU to be sent
 *  @param   disableDefaultRsp - whether to disable the Default Response command
 *  @param   seqNum - sequence number
 *
 *  @return  ZStatus_t
 */
#define zclPI_Send_BACnetTransferNPDUCmd(a,b,c,d,e,f) zcl_SendCommand( (a), (b), ZCL_CLUSTER_ID_PI_BACNET_PROTOCOL_TUNNEL,\
                                                                       COMMAND_PI_BACNET_TUNNEL_TRANSFER_NPDU, TRUE,\
                                                                       ZCL_FRAME_CLIENT_SERVER_DIR, (e), 0, (f), (c), (d) )
/*
 *  Call to send out an 11073 Disconnect Request Command. This command
 *  is generated when a Data Management device wishes to disconnect a
 *  tunnel connection existing on an agent device.
 *
 *  Use Like: ZStatus_t zclPI_Send_11073DisconnectReq( uint8_t srcEP, afAddrType_t *dstAddr,
 *                                                     uint8_t *managerAddr,
 *                                                     uint8_t disableDefaultRsp, uint8_t seqNum );
 *  @param   srcEP - Sending application's endpoint
 *  @param   dstAddr - where you want the message to go
 *  @param   managerAddr - IEEE address (64-bit) of Data Management device
                           transmitting this frame
 *  @param   disableDefaultRsp - whether to disable the Default Response command
 *  @param   seqNum - sequence number
 *
 *  @return  ZStatus_t
 */
#define zclPI_Send_11073DisconnectReq(a,b,c,d,e) zcl_SendCommand( (a), (b), ZCL_CLUSTER_ID_PI_11073_PROTOCOL_TUNNEL,\
                                                                  COMMAND_PI_11073_TUNNEL_DISCONNECT_REQ, TRUE,\
                                                                  ZCL_FRAME_CLIENT_SERVER_DIR, (d), 0, (e),\
                                                                  Z_EXTADDR_LEN, (c) )
/*
 *  Call to send out an 11073 Connect Status Notification Command. This
 *  command is generated by an agent device in response to a connect
 *  request command, disconnect command, or in response to some other
 *  event that causes the tunnel to become connected or disconnected.
 *  It is also sent by the agent device to request the Data Management
 *  device to reconnect a tunnel.
 *
 *  Use Like: ZStatus_t zclPI_Send_11073ConnectStatusNoti( uint8_t srcEP, afAddrType_t *dstAddr,
 *                                                         uint8_t connectStatus, uint8_t disableDefaultRsp,
 *                                                         uint8_t seqNum );
 *  @param   srcEP - Sending application's endpoint
 *  @param   dstAddr - where you want the message to go
 *  @param   connectStatus - connect status
 *  @param   disableDefaultRsp - whether to disable the Default Response command
 *  @param   seqNum - sequence number
 *
 *  @return  ZStatus_t
 */
#define zclPI_Send_11073ConnectStatusNoti(a,b,c,d,e) zcl_SendCommand( (a), (b), ZCL_CLUSTER_ID_PI_11073_PROTOCOL_TUNNEL,\
                                                                      COMMAND_PI_11073_TUNNEL_CONNECT_STATUS_NOTI, TRUE,\
                                                                      ZCL_FRAME_SERVER_CLIENT_DIR, (d), 0, (e), 1, &(c) )

/****************************************************************************
 * VARIABLES
 */


/****************************************************************************
 * FUNCTIONS
 */

 /*
  * Register for callbacks from this cluster library
  */
extern ZStatus_t zclPI_RegisterCmdCallbacks( uint8_t endpoint, zclPI_AppCallbacks_t *callbacks );




/*
 * Send out a Match Protocol Address Command
 */
extern ZStatus_t zclPI_Send_MatchProtocolAddrCmd( uint8_t srcEP, afAddrType_t *dstAddr,
                                                  uint8_t len, uint8_t *protocolAddr,
                                                  uint8_t disableDefaultRsp, uint8_t seqNum );
/*
 *  Send a Match Protocol Address Response
*/
extern ZStatus_t zclPI_Send_MatchProtocolAddrRsp( uint8_t srcEP, afAddrType_t *dstAddr,
                                                  uint8_t *ieeeAddr, uint8_t len, uint8_t *protocolAddr,
                                                  uint8_t disableDefaultRsp, uint8_t seqNum );
/*
 * Send out an Advertise Protocol Address Command
 */
extern ZStatus_t zclPI_Send_AdvertiseProtocolAddrCmd( uint8_t srcEP, afAddrType_t *dstAddr,
                                                      uint8_t len, uint8_t *protocolAddr,
                                                      uint8_t disableDefaultRsp, uint8_t seqNum );


extern ZStatus_t zclPI_Send_7818TransferAPDUCmd( uint8_t srcEP, afAddrType_t *dstAddr,
                                                  uint16_t len, uint8_t *apdu, uint8_t direction, uint8_t seqNum );
/*
 * Send out an 11073 Transfer APDU Command
 */
extern ZStatus_t zclPI_Send_11073TransferAPDUCmd( uint8_t srcEP, afAddrType_t *dstAddr,
                                                  uint16_t len, uint8_t *apdu, uint8_t seqNum );
/*
 * Send out an 11073 Connect Request Command
 */
extern ZStatus_t zclPI_Send_11073ConnectReq( uint8_t srcEP, afAddrType_t *dstAddr,
                                             uint8_t connectCtrl, uint16_t idleTimeout,
                                             uint8_t *managerAddr, uint8_t managerEP,
                                             uint8_t disableDefaultRsp, uint8_t seqNum );


/*********************************************************************
*********************************************************************/
#endif // ZCL_PI

#ifdef __cplusplus
}
#endif

#endif /* ZCL_PI_H */
