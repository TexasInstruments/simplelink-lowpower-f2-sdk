/**************************************************************************************************
  Filename:       zcl_partition.h
  Revised:        $Date: 2013-10-16 16:38:58 -0700 (Wed, 16 Oct 2013) $
  Revision:       $Revision: 35701 $

  Description:    This file contains the ZCL Partition Cluster definitions. The
  Partition Cluster allows up to 100K packets.

  Note: both side must agree on the following 4 attributes:
  PartitionedFrameSize (e.g. 0x40)
  LargeFrameSize (e.g. 1280 - an MTU for IPv6 for example)
  NumberOfAckFrame (e.g. 4)
  InterfaceDelay (e.g. 0x32)

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

#ifndef ZCL_PARTITION_H
#define ZCL_PARTITION_H

#ifdef __cplusplus
extern "C"
{
#endif

/******************************************************************************
 * INCLUDES
 */
#include "zcl.h"
#include "ti_zstack_config.h"

#ifdef ZCL_PARTITION

/******************************************************************************
 * CONSTANTS
 */

/*****************************************/
/***  Partition Cluster Attributes ***/
/*****************************************/

// Server Attributes, Section A.1.3.2 of 10-5685-00 ZigBee Telecomunications Applications Profile
#define ATTRID_PARTITION_MAX_INCOMING_TRANSFER_SIZE      0x0000   // M, R,  uint16_t
#define ATTRID_PARTITION_MAX_OUTGOING_TRANSFER_SIZE      0x0001   // M, R,  uint16_t
#define ATTRID_PARTITION_PARTITIONED_FRAME_SIZE          0x0002   // M, RW,  uint8_t
#define ATTRID_PARTITION_LARGE_FRAME_SIZE                0x0003   // M, RW, uint16_t
#define ATTRID_PARTITION_NUMBER_OF_ACK_FRAMES            0x0004   // M, RW,  uint8_t
#define ATTRID_PARTITION_NACK_TIMEOUT                    0x0005   // M, R,  uint16_t
#define ATTRID_PARTITION_INTERFRAME_DELAY                0x0006   // M, RW,  uint8_t
#define ATTRID_PARTITION_NUMBER_OF_SEND_RETRIES          0x0007   // M, R,   uint8_t
#define ATTRID_PARTITION_SENDER_TIMEOUT                  0x0008   // M, R,  uint16_t
#define ATTRID_PARTITION_RECEIVER_TIMEOUT                0x0009   // M, R,  uint16_t


// Server Attribute Defaults, Section A.1.3.2 of 10-5685-00 ZigBee Telecomunications Applications Profile
#define ATTR_DEFAULT_PARTITION_MAX_INCOMING_TRANSFER_SIZE      0x0500   // uint16_t,
#define ATTR_DEFAULT_PARTITION_MAX_OUTGOING_TRANSFER_SIZE      0x0500   // uint16_t,
#define ATTR_DEFAULT_PARTITION_PARTITIONED_FRAME_SIZE            0x50   //  uint8_t,
#define ATTR_DEFAULT_PARTITION_LARGE_FRAME_SIZE                0x0500   // uint16_t,
#define ATTR_DEFAULT_PARTITION_NUMBER_OF_ACK_FRAMES              0x64   //  uint8_t,
#define ATTR_DEFAULT_PARTITION_NACK_TIMEOUT                     15000   // uint16_t, 0x64 * 200ms * 5 = 186a0 (too large). Spec issue.
#define ATTR_DEFAULT_PARTITION_INTERFRAME_DELAY                  0x10   //  uint8_t, 10ms, see A.1.3.2.7
#define ATTR_DEFAULT_PARTITION_NUMBER_OF_SEND_RETRIES            0x03   //  uint8_t, 3 retries
#define ATTR_DEFAULT_PARTITION_SENDER_TIMEOUT                   60000   // uint16_t, 2 * 0x64 * 200ms * 5 = 30D40 (too large). Spec issue.
#define ATTR_DEFAULT_PARTITION_RECEIVER_TIMEOUT                 60000   // uint16_t, time to wait before timing out (1 minute)

// Commands received by Server
#define COMMAND_PARTITION_TRANSFER_PARTITIONED_FRAME             0x00     // M, zclCmdTransferPartitionedFrame_t
#define COMMAND_PARTITION_READ_HANDSHAKE_PARAM                   0x01     // M, zclCmdReadHandshakeFrame_t
#define COMMAND_PARTITION_WRITE_HANDSHAKE_PARAM                  0x02     // M, zclCmdWriteHandshakeFrame_t

// Commands generated by Server
#define COMMAND_PARTITION_MULTIPLE_ACK                           0x00     // M, zclCmdMultipleAckFrame_t
#define COMMAND_PARTITION_READ_HANDSHAKE_PARAM_RSP               0x01     // M, zclCmdReadHandshakeFrame_t


/*******************************************************************************
 * TYPEDEFS
 */

/*** ZCL Partition: Transfer Partitioned Frame ***/
typedef struct
{
  uint8_t   fragmentationOptions;
  uint16_t  partitionIndicator;   // total length on first block, else block index starting from 1
  uint8_t   frameLen;
  uint8_t   *pFrame;
} zclCmdTransferPartitionedFrame_t;

#define PAYLOAD_LEN_TRANSFER_PARTITIONED_FRAME    4 // not including pFrame

#define ZCL_PARTITION_OPTIONS_FIRSTBLOCK       0x01 // set true for 1st block
#define ZCL_PARTITION_OPTIONS_INDICATOR_16BIT  0x02 // use this for options to have 16-bit indicator field
#define ZCL_PARTITION_OPTIONS_INDICATOR_8BIT   0x00 // use this for options to have 8-bit indicator field

/*** ZCL Partition: Read Handshake Parameters ***/
typedef struct
{
  uint16_t    clusterID;
  uint8_t     seqNum;              // sequence # for response
  uint8_t     numAttrs;            // # of attributes to read
  uint16_t    *pAttrID;            // array of attribute IDs to read (usually 2,3,4,6)
} zclCmdReadHandshakeParam_t;

typedef struct
{
  uint16_t attrID;             // attribute ID
  uint8_t  dataType;           // attribute data type
  uint16_t attr;               // attribute data is here (8 or 16 bits)
} zclPartitionWriteRec_t;

#define PAYLOAD_LEN_WRITE_REC   5

/*** ZCL Partition: Write Handshake Parameters ***/
typedef struct
{
  uint16_t                  clusterID;
  uint8_t                   numRecords;     // # of records to write
  zclPartitionWriteRec_t *pWriteRecord;   // array of write records to write (attrs 2,3,4,6)
} zclCmdWriteHandshakeParam_t;

#define PAYLOAD_LEN_WRITE_HANDSHAKE_PARAM   2 // not including pWriteRecord or numRecords

/*** ZCL Partition: Multiple ACK ***/
typedef struct
{
  uint8_t    options;
  uint16_t   firstFrameID;      // corresponds to partitionIndicator on zclCmdTransferPartitionedFrame_t
  uint8_t    numNAcks;
  uint16_t   *pNAckID;           // array of NACK IDs
} zclCmdMultipleAck_t;

#define ZCL_PARTITION_OPTIONS_NACK_8BIT   0x00 // use this to indicate NACK IDs will fit in 8 bits
#define ZCL_PARTITION_OPTIONS_NACK_16BIT  0x01 // use this to indicate NACK IDs need 16-bits

typedef struct
{
  uint16_t  attrID;
  uint8_t   status;
  uint8_t   dataType;   // see ZCL_DATATYPE_UINT8 in zcl.h
  uint16_t  attr;       // attribute data, either uint8_t or uint16_t
} zclPartitionReadRec_t;

/*** ZCL Partition: Read Handshake Response ***/
#define COMMAND_PARTITION_READ_HANDSHAKE_PARAM_RSP               0x01     // M, zclCmdReadHandshakeFrame_t
typedef struct
{
  uint16_t                  clusterID;      // cluster ID
  uint8_t                   numRecords;     // # of records in array below (may be 0)
  zclPartitionReadRec_t  *pReadRecord;    // array of Read Attribute Response Status records
} zclCmdReadHandshakeParamRsp_t;

// Partition Cluster callback types
typedef ZStatus_t (*zclPartition_TransferPartitionedFrame_t)( afAddrType_t *srcAddr, zclCmdTransferPartitionedFrame_t *pCmd );
typedef ZStatus_t (*zclPartition_ReadHandshakeParam_t)( afAddrType_t *srcAddr, zclCmdReadHandshakeParam_t *pCmd );
typedef ZStatus_t (*zclPartition_WriteHandshakeParam_t)( zclCmdWriteHandshakeParam_t *pCmd );
typedef ZStatus_t (*zclPartition_MultipleAck_t)( afAddrType_t *srcAddr, zclCmdMultipleAck_t *pCmd );
typedef ZStatus_t (*zclPartition_ReadHandshakeParamRsp_t)( zclCmdReadHandshakeParamRsp_t *pCmd );


// Partition Cluster callback types
typedef struct
{
  zclPartition_TransferPartitionedFrame_t   pfnPartition_TransferPartitionedFrame;
  zclPartition_ReadHandshakeParam_t         pfnPartition_ReadHandshakeParam;
  zclPartition_WriteHandshakeParam_t        pfnPartition_WriteHandshakeParam;
  zclPartition_MultipleAck_t                pfnPartition_MultipleAck;
  zclPartition_ReadHandshakeParamRsp_t      pfnPartition_ReadHandshakeParamRsp;
} zclPartition_AppCallbacks_t;


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
extern ZStatus_t zclPartition_RegisterCmdCallbacks( uint8_t endpoint, zclPartition_AppCallbacks_t *callbacks );


// helper functions for ProcessIn functions and when interacting with SWAT or the MT interface
// All these helper functions returns ZSuccess if they worked (no problems with frame/buffer/memory allocation)
// All these helper functions allocate memory for their lists (e.g. pCmd->pWriteRecord). Make sure to free the list!
extern ZStatus_t zclPartition_ConvertOtaToNative_TransferPartitionedFrame( zclCmdTransferPartitionedFrame_t *pCmd, uint8_t *buf, uint8_t buflen );
extern ZStatus_t zclPartition_ConvertOtaToNative_ReadHandshakeParam( zclCmdReadHandshakeParam_t *pCmd, uint8_t *buf, uint8_t buflen );
extern ZStatus_t zclPartition_ConvertOtaToNative_WriteHandshakeParam( zclCmdWriteHandshakeParam_t *pCmd, uint8_t *buf, uint8_t buflen );
extern ZStatus_t zclPartition_ConvertOtaToNative_MultipleAck( zclCmdMultipleAck_t *pCmd, uint8_t *buf, uint8_t buflen );
extern ZStatus_t zclPartition_ConvertOtaToNative_ReadHandshakeParamRsp( zclCmdReadHandshakeParamRsp_t *pCmd, uint8_t *buf, uint8_t buflen );

/*********************************************************************
 * @fn      zclPartition_Send_TransferPartitionedFrame
 *
 * @brief   send a single block (partitioned frame) to a remote receiver
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - the partitioned frame
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclPartition_Send_TransferPartitionedFrame( uint8_t srcEP, afAddrType_t *dstAddr,
                                                             zclCmdTransferPartitionedFrame_t *pCmd,
                                                             uint8_t disableDefaultRsp, uint8_t seqNum );


/*********************************************************************
 * @fn      zclPartition_Send_ReadHandshakeParam
 *
 * @brief   Call to send out Poll Control CheckIn command from ZED to ZR/ZC. The Rsp
 *          will indicate whether to stay awake or go back to sleep.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - what parameters to read
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclPartition_Send_ReadHandshakeParam( uint8_t srcEP, afAddrType_t *dstAddr,
                                                       zclCmdReadHandshakeParam_t *pCmd,
                                                       uint8_t disableDefaultRsp, uint8_t seqNum );

/*********************************************************************
 * @fn      zclPartition_Send_WriteHandshakeParam
 *
 * @brief   Call to send out Poll Control CheckIn command from ZED to ZR/ZC. The Rsp
 *          will indicate whether to stay awake or go back to sleep.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - what parameters to write
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclPartition_Send_WriteHandshakeParam( uint8_t srcEP, afAddrType_t *dstAddr,
                                                        zclCmdWriteHandshakeParam_t *pCmd,
                                                        uint8_t disableDefaultRsp, uint8_t seqNum );

/*********************************************************************
 * @fn      zclPartition_Send_MultipleAck
 *
 * @brief   Call to send out Poll Control CheckIn command from ZED to ZR/ZC. The Rsp
 *          will indicate whether to stay awake or go back to sleep.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - multi ack response
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclPartition_Send_MultipleAck( uint8_t srcEP, afAddrType_t *dstAddr,
                                                zclCmdMultipleAck_t *pCmd,
                                                uint8_t disableDefaultRsp, uint8_t seqNum );

/*********************************************************************
 * @fn      zclPartition_Send_ReadHandshakeParamRsp
 *
 * @brief   Call to send out Poll Control CheckIn command from ZED to ZR/ZC. The Rsp
 *          will indicate whether to stay awake or go back to sleep.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - read multi-attribues response
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclPartition_Send_ReadHandshakeParamRsp( uint8_t srcEP, afAddrType_t *dstAddr,
                                                          zclCmdReadHandshakeParamRsp_t *pCmd,
                                                          uint8_t disableDefaultRsp, uint8_t seqNum );


/*********************************************************************
*********************************************************************/
#endif // ZCL_PARTITION

#ifdef __cplusplus
}
#endif

#endif /* ZCL_PARTITION_H */

