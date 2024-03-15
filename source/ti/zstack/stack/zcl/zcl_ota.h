/******************************************************************************
  Filename:       zcl_ota.h
  Revised:        $Date: 2015-04-14 21:59:34 -0700 (Tue, 14 Apr 2015) $
  Revision:       $Revision: 43420 $

  Description:    ZCL Over-the-Air Upgrade Cluster definitions.


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
******************************************************************************/

#ifndef ZCL_OTA_H
#define ZCL_OTA_H

#ifdef __cplusplus
extern "C"
{
#endif

/******************************************************************************
 * INCLUDES
 */
#include "zcl.h"
#include "ota_common.h"
#include "ti_zstack_config.h"
#if defined ZCL_SYSCONFIG
#include "zcl_config.h"
#endif

/******************************************************************************
 * CONSTANTS
 */
#define ZCL_SE_PROFILE_ID                             0x0109
#define ZCL_HA_PROFILE_ID                             0x0104

#define ZCL_SE_DEVICEID_PHYSICAL                      0x0507

/**
 * @defgroup ZCL_OTA_CLUSTER ZCL OTA Cluster
 * @{
 */

/**
 * @defgroup ZCL_OTA_CLUSTER_MACROS ZCL OTA Cluster Macros
 * @{
 */

/// The value indicates the largest possible length of data (in bytes) that the
/// client can receive at once.
#define OTA_MAX_MTU                                   32
/// Maximum block request retries
#define OTA_MAX_BLOCK_RETRIES                         10
/// Maximum end request retries
#define OTA_MAX_END_REQ_RETRIES                       2
/// Maximum block response timeout
#define OTA_MAX_BLOCK_RSP_WAIT_TIME                   ((uint16_t)5000)

// Simple descriptor values
/// Simple descriptor data for OTA cluster endpoint
#define ZCL_OTA_ENDPOINT                              20
#define ZCL_OTA_SAMPLE_PROFILE_ID                     ZCL_HA_PROFILE_ID
#define ZCL_OTA_SAMPLE_DEVICEID                       0
#define ZCL_OTA_DEVICE_VERSION                        0
#define ZCL_OTA_FLAGS                                 0

// OTA Device ID values
/// Manufacturer ID
#if !defined OTA_MANUFACTURER_ID
#define OTA_MANUFACTURER_ID                           0x5678
#endif
#if !defined OTA_TYPE_ID
#define OTA_TYPE_ID                                   0x1234
#endif

/// OTA Header Version
#define OTA_HDR_VERSION                               0x0100

/// OTA Header Field Control Bits
#define OTA_HDR_FC_SEC_CRED_PRESENT                   0x01
#define OTA_HDR_FC_DEV_SPEC_FILE                      0x02
#define OTA_HDR_FC_HW_VER_PRESENT                     0x04

/// OTA Header Manufacturer ID 'Match All'
#define OTA_HDR_MFG_MATCH_ALL                         0xFFFF

/// OTA Header Image Type
#define OTA_HDR_IMAGE_SEC_CRED                        0xFFC0
#define OTA_HDR_IMAGE_CONFIG                          0xFFC1
#define OTA_HDR_IMAGE_LOG                             0xFFC2
#define OTA_HDR_IMAGE_MATCH_ALL                       0xFFFF

/// OTA Header File Version 'Match All'
#define OTA_HDR_FILE_VER_MATCH_ALL                    0xFFFFFFFF

/// OTA ZigBee Stack Version
#define OTA_STACK_VER_2006                            0x0000
#define OTA_STACK_VER_2007                            0x0001
#define OTA_STACK_VER_PRO                             0x0002
#define OTA_STACK_VER_IP                              0x0003

/// OTA Security Credential Version
#define OTA_SEC_CRED_VER_10                           0x00
#define OTA_SEC_CRED_VER_1X                           0x01
#define OTA_SEC_CRED_VER_20                           0x02

// OTA Attribute IDs
/// The attribute is used to store the IEEE address of the upgrade server
/// resulted from the discovery of the upgrade server's identity
#define ATTRID_OTA_UPGRADE_UPGRADE_SERVER_ID                      0x0000
/// The parameter indicates the current location in the OTA upgrade image.
#define ATTRID_OTA_UPGRADE_FILE_OFFSET                            0x0001
/// The file version of the running firmware image on the device.
#define ATTRID_OTA_UPGRADE_CURRENT_FILE_VERSION                   0x0002
/// The ZigBee stack version of the running image on the device.
#define ATTRID_OTA_UPGRADE_CURRENT_ZIG_BEE_STACK_VERSION          0x0003
/// The file version of the downloaded image on additional memory space on
/// the device.
#define ATTRID_OTA_UPGRADE_DOWNLOADED_FILE_VERSION                0x0004
/// The ZigBee stack version of the downloaded image on additional memory space
/// on the device.
#define ATTRID_OTA_UPGRADE_DOWNLOADED_ZIG_BEE_STACK_VERSION       0x0005
/// The upgrade status of the client device. The status indicates where the
/// client device is at in terms of the download and upgrade process.
#define ATTRID_OTA_UPGRADE_IMAGE_UPGRADE_STATUS                   0x0006
/// This attribute SHALL reflect the ZigBee assigned value for the manufacturer
/// of the device.
#define ATTRID_OTA_UPGRADE_MANUFACTURER_ID                        0x0007  // uint16_t, R, O
/// This attribute SHALL indicate the image type identifier of the file that
/// the client is currently downloading, or a file that has been completely
/// downloaded but not upgraded to yet.
#define ATTRID_OTA_UPGRADE_IMAGE_TYPE_ID                          0x0008  // uint16_t, R, O
/// This attribute acts as a rate limiting feature for the server to slow down
/// the client download and prevent saturating the network with block requests.
#define ATTRID_OTA_UPGRADE_MINIMUM_BLOCK_PERIOD                   0x0009  // uint16_t, R, O
/// This attribute acts as a second verification to identify the image in the
/// case that sometimes developers of the application have forgotten to increase
/// the firmware version attribute.
#define ATTRID_OTA_UPGRADE_IMAGE_STAMP                            0x000A  // uint32_t, R, O
#define ATTRID_OTA_UPGRADE_UPGRADE_ACTIVATION_POLICY              0x000B
#define ATTRID_OTA_UPGRADE_UPGRADE_TIMEOUT_POLICY                 0x000C

/// OTA Upgrade Status
#define OTA_STATUS_NORMAL                             0x00
#define OTA_STATUS_IN_PROGRESS                        0x01
#define OTA_STATUS_COMPLETE                           0x02
#define OTA_STATUS_UPGRADE_WAIT                       0x03
#define OTA_STATUS_COUNTDOWN                          0x04
#define OTA_STATUS_WAIT_FOR_MORE                      0x05

/// OTA Upgrade Time 'wait for upgrade'
#define OTA_UPGRADE_TIME_WAIT                         0xFFFFFFFF

// OTA Cluster Command Frames
/// The purpose of sending Image Notify command is so the server has a way to
/// notify client devices of when the OTA upgrade images are available for them.
#define COMMAND_OTA_UPGRADE_IMAGE_NOTIFY                                    0x00
/// Client devices SHALL send a Query Next Image Request command to the server
/// to see if there is new OTA upgrade image available.
#define COMMAND_OTA_UPGRADE_QUERY_NEXT_IMAGE_REQUEST                        0x01
/// The upgrade server sends a Query Next Image Response with one of the
/// following status: SUCCESS, NO_IMAGE_AVAILABLE or NOT_AUTHORIZED.
/// When a SUCCESS status is sent, it is considered to be the explicit
/// authorization to a device by the upgrade server that the device MAY upgrade
/// to a specific software image.
#define COMMAND_OTA_UPGRADE_QUERY_NEXT_IMAGE_RESPONSE                       0x02
/// The client device requests the image data at its leisure by sending Image
/// Block Request command to the upgrade server.
#define COMMAND_OTA_UPGRADE_IMAGE_BLOCK_REQUEST                             0x03
/// The support for the command is optional. The client device MAY choose to
/// request OTA upgrade data in one page size at a time from upgrade server.
#define COMMAND_OTA_UPGRADE_IMAGE_PAGE_REQUEST                              0x04
/// Upon receipt of an Image Block Request command the server SHALL generate
/// an Image Block Response.
#define COMMAND_OTA_UPGRADE_IMAGE_BLOCK_RESPONSE                            0x05
/// Upon reception all the image data, the client SHOULD verify the image to
/// ensure its integrity and validity.
#define COMMAND_OTA_UPGRADE_UPGRADE_END_REQUEST                             0x06
/// When an upgrade server receives an Upgrade End Request command with a status
/// of INVALID_IMAGE, REQUIRE_MORE_IMAGE, or ABORT, no additional processing
/// SHALL be done in its part. If the upgrade server receives an Upgrade
/// End Request command with a status of SUCCESS, it SHALL generate an Upgrade
/// End Response 17245 with the manufacturer code and image type received in
/// the Upgrade End Request along with the times indicating when the device
/// SHOULD upgrade to the new image.
#define COMMAND_OTA_UPGRADE_UPGRADE_END_RESPONSE                            0x07
/// Client devices SHALL send a Query Device Specific File Request command to
/// the server to request for a file that is specific and unique to it.
#define COMMAND_OTA_UPGRADE_QUERY_DEVICE_SPECIFIC_FILE_REQUEST              0x08
/// The server sends Query Device Specific File Response after receiving Query
/// Device Specific File Request from a client.
#define COMMAND_OTA_UPGRADE_QUERY_DEVICE_SPECIFIC_FILE_RESPONSE             0x09

// OTA Cluster Command Frame Payload Lengths
#define PAYLOAD_MAX_LEN_IMAGE_NOTIFY                  10
#define PAYLOAD_MAX_LEN_QUERY_NEXT_IMAGE_REQ          11
#define PAYLOAD_MAX_LEN_QUERY_NEXT_IMAGE_RSP          13
#define PAYLOAD_MAX_LEN_IMAGE_BLOCK_REQ               24
#define PAYLOAD_MAX_LEN_IMAGE_PAGE_REQ                26
#define PAYLOAD_MAX_LEN_IMAGE_BLOCK_RSP               14
#define PAYLOAD_MAX_LEN_UPGRADE_END_REQ               9
#define PAYLOAD_MAX_LEN_UPGRADE_END_RSP               16
#define PAYLOAD_MAX_LEN_QUERY_SPECIFIC_FILE_REQ       18
#define PAYLOAD_MAX_LEN_QUERY_SPECIFIC_FILE_RSP       13
#define PAYLOAD_MIN_LEN_IMAGE_NOTIFY                  2
#define PAYLOAD_MIN_LEN_QUERY_NEXT_IMAGE_REQ          9
#define PAYLOAD_MIN_LEN_QUERY_NEXT_IMAGE_RSP          1
#define PAYLOAD_MIN_LEN_IMAGE_BLOCK_REQ               14
#define PAYLOAD_MIN_LEN_IMAGE_PAGE_REQ                18
#define PAYLOAD_MIN_LEN_IMAGE_BLOCK_RSP               14
#define PAYLOAD_MIN_LEN_IMAGE_BLOCK_WAIT              11
#define PAYLOAD_MIN_LEN_UPGRADE_END_REQ               1
#define PAYLOAD_MIN_LEN_UPGRADE_END_RSP               16
#define PAYLOAD_MIN_LEN_QUERY_SPECIFIC_FILE_REQ       18
#define PAYLOAD_MIN_LEN_QUERY_SPECIFIC_FILE_RSP       1
#define PAYLOAD_LEN_QUERY_RSP_ADDR_MODE_OFFSET                    8
#define PAYLOAD_FILE_READ_RSP_PAYLOAD_LEN_16BIT_ADDR_MODE        20     //This len is counting address mode of 16 bit
#define PAYLOAD_FILE_READ_RSP_PAYLOAD_OFFSET_16BIT_ADDR_MODE     19     //This offset is counting address mode of 16 bit
#define PAYLOAD_FILE_READ_RSP_PAYLOAD_LEN_64BIT_ADDR_MODE        26     //This len is counting address mode of 64 bit
#define PAYLOAD_FILE_READ_RSP_PAYLOAD_OFFSET_64BIT_ADDR_MODE     25     //This offset is counting address mode of 64 bit

/// Image Block Request Field Control Bitmask
#define OTA_BLOCK_FC_GENERIC                          0x00
#define OTA_BLOCK_FC_NODES_IEEE_PRESENT               0x01
#define OTA_BLOCK_FC_REQ_DELAY_PRESENT                0x02

/// Image Notify Command Payload Type
#define NOTIFY_PAYLOAD_JITTER                         0x00
#define NOTIFY_PAYLOAD_JITTER_MFG                     0x01
#define NOTIFY_PAYLOAD_JITTER_MFG_TYPE                0x02
#define NOTIFY_PAYLOAD_JITTER_MFG_TYPE_VERS           0x03

/// Client Task Events
#define ZCL_OTA_IMAGE_BLOCK_WAIT_EVT                  0x0001
#define ZCL_OTA_UPGRADE_WAIT_EVT                      0x0002
#define ZCL_OTA_QUERY_SERVER_EVT                      0x0004
#define ZCL_OTA_BLOCK_RSP_TO_EVT                      0x0008
#define ZCL_OTA_IMAGE_QUERY_TO_EVT                    0x0010
#define ZCL_OTA_IMAGE_BLOCK_REQ_DELAY_EVT             0x0020
#define ZCL_OTA_SEND_MATCH_DESCRIPTOR_EVT             0x0040


// The OTA Upgrade delay is the number of seconds before the client
// should wait before switching to the upgrade image
#define OTA_UPGRADE_DELAY                             60
#define OTA_SEND_BLOCK_WAIT                           0 // use blockReqDelay for rate limiting

// OTA Element Tag Identifiers
#define OTA_UPGRADE_IMAGE_TAG_ID                      0
#define OTA_ECDSA_SIGNATURE_TAG_ID                    1
#define OTA_ECDSA_CERT_TAG_ID                         2

// OTA Client process data states
#define ZCL_OTA_PD_MAGIC_0_STATE                      0
#define ZCL_OTA_PD_MAGIC_1_STATE                      1
#define ZCL_OTA_PD_MAGIC_2_STATE                      2
#define ZCL_OTA_PD_MAGIC_3_STATE                      3
#define ZCL_OTA_PD_HDR_LEN1_STATE                     4
#define ZCL_OTA_PD_HDR_LEN2_STATE                     5
#define ZCL_OTA_PD_STK_VER1_STATE                     6
#define ZCL_OTA_PD_STK_VER2_STATE                     7
#define ZCL_OTA_PD_CONT_HDR_STATE                     8
#define ZCL_OTA_PD_ELEM_TAG1_STATE                    9
#define ZCL_OTA_PD_ELEM_TAG2_STATE                    10
#define ZCL_OTA_PD_ELEM_LEN1_STATE                    11
#define ZCL_OTA_PD_ELEM_LEN2_STATE                    12
#define ZCL_OTA_PD_ELEM_LEN3_STATE                    13
#define ZCL_OTA_PD_ELEM_LEN4_STATE                    14
#define ZCL_OTA_PD_ELEMENT_STATE                      15

// OTA Unused endpoint
#define OTA_UNUSED_ENDPOINT  0

#define ZCL_OTA_MAX_OPTIONS 1

/** @} End ZCL_OTA_CLUSTER_MACROS */

/**
 * @defgroup ZCL_OTA_CLUSTER_TYPEDEFS ZCL OTA Cluster Typedefs
 * @{
 */

/******************************************************************************
 * TYPEDEFS
 */
// Message Parameter Structures
typedef struct
{
  uint8_t payloadType;       //!< Enum that specify the optional fields included
                           //!< in the payload
  uint8_t queryJitter;       //!< The parameter indicates whether the client
                           //!< receiving Image Notify Command SHOULD send in
                           //!< Query Next Image Request command or not.
  zclOTA_FileID_t fileId;  //!< File Id structure.
} zclOTA_ImageNotifyParams_t;

typedef struct
{
  uint8_t fieldControl;     //!< The field control indicates whether additional
                          //!< information such as device's current running
                          //!< hardware version is included as part of the Query
                          //!< Next Image Request command.
  zclOTA_FileID_t fileId; //!< File Id structure.
  uint16_t hardwareVersion; //!< The hardware version if included in the payload
                          //!< represents the device's current running hardware
                          //!< version.
} zclOTA_QueryNextImageReqParams_t;

typedef struct
{
  uint8_t status;           //!< Indicates the status of response.
  zclOTA_FileID_t fileId; //!< File Id structure.
  uint32_t imageSize;       //!< The value represents the total size of the image
                          //!< (in bytes) including header and all sub-elements.
} zclOTA_QueryImageRspParams_t;

typedef struct
{
  uint8_t fieldControl;     //!< Field control value is used to indicate additional
                          //!< optional fields that MAY be included in the payload
                          //!< of Image Block Request command.
  zclOTA_FileID_t fileId; //!< File Id structure.
  uint32_t fileOffset;      //!< The value indicates number of bytes of data offset
                          //!< from the beginning of the file.
  uint8_t maxDataSize;      //!< The value indicates the largest possible length of
                          //!< data (in bytes) that the client can receive at once.
  uint8_t nodeAddr[Z_EXTADDR_LEN]; //!< This is the IEEE address of the client device.
  uint16_t blockReqDelay;   //!< This is the current value of the MinimumBlockPeriod
                          //!< attribute of the device that is making the request
                          //!< as set by the server.
} zclOTA_ImageBlockReqParams_t;

typedef struct
{
  uint8_t fieldControl;     //!< Field control value is used to indicate additional
                          //!< optional fields that MAY be included in the payload
                          //!< of Image Page Request command.
  zclOTA_FileID_t fileId; //!< File Id structure.
  uint32_t fileOffset;      //!< The value indicates number of bytes of data offset
                          //!< from the beginning of the file.
  uint8_t maxDataSize;      //!< The value indicates the largest possible length of
                          //!< data (in bytes) that the client can receive at once.
  uint16_t pageSize;        //!< The value indicates the number of bytes to be sent
                          //!< by the server before the client sends another Image
                          //!< Page Request command.
  uint16_t responseSpacing; //!< The value indicates how fast the server SHALL
                          //!< send the data (via Image Block Response command)
                          //!< to the client.
  uint8_t nodeAddr[Z_EXTADDR_LEN]; //!< This is the IEEE address of the client device.
} zclOTA_ImagePageReqParams_t;

typedef struct
{
  zclOTA_FileID_t fileId; //!< File Id structure.
  uint32_t fileOffset;      //!< The value indicates number of bytes of data offset
                          //!< from the beginning of the file.
  uint8_t dataSize;         //!< The value indicates the length of the image data
                          //!< (in bytes) that is being included in the command.
  uint8_t *pData;           //!< The actual OTA upgrade image data with the length
                          //!< equals to data size value.
} imageBlockRspSuccess_t;

typedef struct
{
  uint32_t currentTime;    //!< Current time
  uint32_t requestTime;    //!< Request time
  uint16_t blockReqDelay;  //!< Delay timeout
} imageBlockRspWait_t;

typedef union
{
  imageBlockRspSuccess_t success; //!< Indicates the status of response.
  imageBlockRspWait_t wait;       //!< Block Response wait structure.
} imageBlockRsp_t;

typedef struct
{
  uint8_t status;        //!< Indicates the status of response.
  imageBlockRsp_t rsp; //!< Block Response structure.
} zclOTA_ImageBlockRspParams_t;

typedef struct
{
  uint8_t status;           //!< Status of request.
  zclOTA_FileID_t fileId; //!< File Id structure.
} zclOTA_UpgradeEndReqParams_t;

typedef struct
{
  zclOTA_FileID_t fileId; //!< File Id structure.
  uint32_t currentTime;     //!< Current time value.
  uint32_t upgradeTime;     //!< Scheduled upgrade time value.
} zclOTA_UpgradeEndRspParams_t;

typedef struct
{
  uint8_t nodeAddr[Z_EXTADDR_LEN]; //!< This is the IEEE address of the client device.
  zclOTA_FileID_t fileId;        //!< File Id structure.
  uint16_t stackVersion;           //!< Stack version value.
} zclOTA_QuerySpecificFileReqParams_t;

typedef struct
{
  OsalPort_EventHdr hdr;
  uint8_t ota_event;
} zclOTA_CallbackMsg_t;

/** @} End ZCL_OTA_CLUSTER_TYPEDEFS */

/**
 * @defgroup ZCL_OTA_CLUSTER_GLOBALS ZCL OTA Cluster Globals
 * @{
 */

/******************************************************************************
 * GLOBAL VARIABLES
 */
extern uint8_t currentOtaEndpoint;
extern uint8_t zclOTA_OtaUpgradeEndReqTransSeq;
extern uint8_t zclOTA_Permit;

/** @} End ZCL_OTA_CLUSTER_GLOBALS */

/**
 * @defgroup ZCL_OTA_CLUSTER_FUNCTIONS ZCL OTA Cluster Functions
 * @{
 */

/******************************************************************************
 * FUNCTIONS
 */

/******************************************************************************
 * @fn      zclOTA_Register
 *
 * @brief   Called by an application to register for callback messages
 *          from the OTA.
 *
 * @param   callbackTask - Application Task ID
 *
 * @return  ZStatus_t
 */
extern void zclOTA_Register(uint8_t applicationTaskId);

/******************************************************************************
 * @fn      zclOTA_PermitOta
 *
 * @brief   Called to enable/disable OTA operation.
 *
 * @param   permit - TRUE to enable OTA, FALSE to diable OTA
 *
 * @return  none
 */
extern void zclOTA_PermitOta(uint8_t permit);

/******************************************************************************
 * @fn          zclOTA_getStatus
 *
 * @brief       Retrieves current ZCL OTA Status
 *
 * @param       none
 *
 * @return      ZCL OTA Status
 */
extern uint8_t zclOTA_getStatus( void );

#if defined OTA_SERVER
/*********************************************************************
 * @fn      zclOTA_getSeqNo
 *
 * @brief   Get the next ZCL OTA Frame Counter for packet sequence number
 *
 * @param   none
 *
 * @return  next ZCL OTA frame counter
 */
extern uint8_t zclOTA_getSeqNo(void);
#endif

#if (defined OTA_CLIENT_STANDALONE) || (defined OTA_CLIENT_INTEGRATED)
/******************************************************************************
 * @fn      zclOTA_ProcessInDefaultRspCmd
 *
 * @brief   Passed along from application.
 *
 * @param   pInMsg - Pointer to Default Response Command
 *
 * @return  void
 */
extern void zclOTA_ProcessInDefaultRspCmd( zclIncomingMsg_t *pInMsg );

/*********************************************************************
 * @fn      zclOTA_setAttributes
 *
 * @brief   Sets pointers to attributes used by ZCL OTA module
 *
 * @param   *attrs - set of attributes from the application
 * @param   numAttrs - number of attributes in the list
 *
 * @return  void
 */
extern void zclOTA_setAttributes( const zclAttrRec_t *attrs, uint8_t numAttrs );

/******************************************************************************
 * @fn      zclOTA_SendQueryNextImageReq
 *
 * @brief   Send an OTA Query Next Image Request mesage.
 *
 * @param   srcEp   - endpoint from which the message is send
 * @param   dstAddr - where you want the message to go
 * @param   pParams - message parameters
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclOTA_SendQueryNextImageReq (uint8_t srcEp, afAddrType_t *dstAddr, zclOTA_QueryNextImageReqParams_t *pParams );

/******************************************************************************
 * @fn      zclOTA_SendUpgradeEndReq
 *
 * @brief   Send an OTA Upgrade End Request mesage.
 *
 * @param   srcEp   - endpoint from which the message is send
 * @param   dstAddr - where you want the message to go
 * @param   pParams - message parameters
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclOTA_SendUpgradeEndReq (uint8_t srcEp, afAddrType_t *dstAddr, zclOTA_UpgradeEndReqParams_t *pParams );

/******************************************************************************
 * @fn      zclOTA_ProcessImageData
 *
 * @brief   Process image data as it is received from the host.
 *
 * @param   pData - pointer to the data
 * @param   len - length of the data
 *
 * @return  status of the operation
 */
extern uint8_t zclOTA_ProcessImageData ( uint8_t *pData, uint8_t len );

/******************************************************************************
 * @fn      zclOTA_SendImageBlockReq
 *
 * @brief   Send an OTA Image Block Request mesage.
 *
 * @param   srcEp   - endpoint from which the message is send
 * @param   dstAddr - where you want the message to go
 * @param   pParams - message parameters
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclOTA_SendImageBlockReq (uint8_t srcEp, afAddrType_t *dstAddr, zclOTA_ImageBlockReqParams_t *pParams );

/******************************************************************************
 * @fn      zclOTA_SendImagePageReq
 *
 * @brief   Send an OTA Image Page Request mesage.
 *
 * @param   srcEp   - endpoint from which the message is send
 * @param   dstAddr - where you want the message to go
 * @param   pParams - message parameters
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclOTA_SendImagePageReq (uint8_t srcEp, afAddrType_t *dstAddr, zclOTA_ImagePageReqParams_t *pParams );

/******************************************************************************
 * @fn      zclOTA_SendQueryDevSpecFileReq
 *
 * @brief   Send an OTA Query Device Specific File Request mesage.
 *
 * @param   srcEp   - endpoint from which the message is send
 * @param   dstAddr - where you want the message to go
 * @param   pParams - message parameters
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclOTA_SendQueryDevSpecFileReq (uint8_t srcEp, afAddrType_t *dstAddr, zclOTA_QuerySpecificFileReqParams_t *pParams );

#endif

extern void zclOTA_ProcessUnhandledFoundationZCLMsgs ( zclIncomingMsg_t *pMsg );

#if defined OTA_SERVER
/******************************************************************************
 * @fn      zclOTA_SendImageNotify
 *
 * @brief   Called by a server to send an Image Notify to an OTA client.
 *
 * @param   srcEp   - endpoint from which the message is send
 * @param   dstAddr - Short address of the client
 * @param   pParams - Parameters of the Image Notify message
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclOTA_SendImageNotify(uint8_t srcEndpoint, afAddrType_t *dstAddr, zclOTA_ImageNotifyParams_t *pParams);

/******************************************************************************
 * @fn      zclOTA_SendQuerySpecificFileRsp
 *
 * @brief   Send an OTA Query Specific File Response mesage.
 *
 * @param   srcEp   - endpoint from which the message is send
 * @param   dstAddr - where you want the message to go
 * @param   pParams - message parameters
 * @param   transSeqNum - Transaction Sequence Number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclOTA_SendQuerySpecificFileRsp (uint8_t srcEp, afAddrType_t *dstAddr, zclOTA_QueryImageRspParams_t *pParams, uint8_t transSeqNum );

/******************************************************************************
 * @fn      zclOTA_SendQueryNextImageRsp
 *
 * @brief   Send an OTA Query Next Image Response message.
 *
 * @param   srcEp   - endpoint from which the message is send
 * @param   dstAddr - where you want the message to go
 * @param   pParams - message parameters
 * @param   transSeqNum - Transaction Sequence Number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclOTA_SendQueryNextImageRsp (uint8_t srcEp, afAddrType_t *dstAddr, zclOTA_QueryImageRspParams_t *pParams, uint8_t transSeqNum );

/******************************************************************************
 * @fn      zclOTA_SendImageBlockRsp
 *
 * @brief   Send an OTA Image Block Response mesage.
 *
 * @param   srcEp   - endpoint from which the message is send
 * @param   dstAddr - where you want the message to go
 * @param   pParams - message parameters
 * @param   transSeqNum - Transaction Sequence Number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclOTA_SendImageBlockRsp (uint8_t srcEp, afAddrType_t *dstAddr, zclOTA_ImageBlockRspParams_t *pParams, uint8_t transSeqNum );

/******************************************************************************
 * @fn      zclOTA_SendUpgradeEndRsp
 *
 * @brief   Send an OTA Upgrade End Response mesage.
 *
 * @param   srcEp   - endpoint from which the message is send
 * @param   dstAddr - where you want the message to go
 * @param   pParams - message parameters
 * @param   transSeqNum - Transaction Sequence Number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclOTA_SendUpgradeEndRsp (uint8_t srcEp, afAddrType_t *dstAddr, zclOTA_UpgradeEndRspParams_t *pParams, uint8_t transSeqNum );

/** @} End ZCL_OTA_CLUSTER_FUNCTIONS */

#endif // OTA_SERVER


#ifdef __cplusplus
}
#endif

#endif /* ZCL_OTA_H */

/** @} End ZCL_OTA_CLUSTER */
