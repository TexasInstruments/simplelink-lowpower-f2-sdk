/******************************************************************************
  Filename:       zcl_ota.c
  Revised:        $Date: 2015-04-20 11:29:22 -0700 (Mon, 20 Apr 2015) $
  Revision:       $Revision: 43465 $

  Description:    Zigbee Cluster Library - Over-the-Air Upgrade Cluster ( OTA )


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

/******************************************************************************
 * INCLUDES
 */
#include "ti_zstack_config.h"
#include "zcomdef.h"
#include "zcl.h"
#include "zcl_general.h"
#include "zcl_ota.h"
#include "ota_common.h"
#include "ti_drivers_config.h"


#if (defined OTA_CLIENT_STANDALONE) || (defined OTA_CLIENT_INTEGRATED)
#include "ota_client.h"
#include "flash_interface.h"
#include "oad_image_header.h"
#include "ext_flash_layout.h"
#include "crc32.h"
#endif


#include "string.h"

#include "board_key.h"
#include "board_led.h"

#if defined ( INTER_PAN ) || defined ( BDB_TL_INITIATOR ) || defined ( BDB_TL_TARGET )
#include "stub_aps.h"
#endif

#if defined OTA_MMO_SIGN
#include "ota_signature.h"
#endif

/******************************************************************************
 * MACROS
 */

/******************************************************************************
 * CONSTANTS
 */
#define OTA_MAX_TRANSACTIONS        4
#define OTA_TRANSACTION_EXPIRATION  1500

#define ZCL_OTA_HDR_LEN_OFFSET      6  // Header length location in OTA upgrade image
#define ZCL_OTA_STK_VER_OFFSET      18 // Stack version location in OTA upgrade image
#define ZCL_OTA_DL_CRC_OFFSET       62


/******************************************************************************
 * GLOBAL VARIABLES
 */
#if (defined OTA_CLIENT_STANDALONE) || (defined OTA_CLIENT_INTEGRATED)
#if defined __IOCC2538_H__
const otaCrc_t OTA_CRC @ ".crc" =
{
  0x20130521,  // CRC
  0x20130521   // CRC Shadow
};
#endif


#if defined __IOCC2538_H__
const preamble_t OTA_Preamble @ ".preamble" =
{
  HAL_OTA_RC_MAX,       // Default program length of max if not using post-processing tool.
  OTA_MANUFACTURER_ID,  // Manufacturer ID
  OTA_TYPE_ID,          // Image Type
  0x00000001            // Image Version
};

#endif

#endif

// Used by the client to correlate the Upgrade End Request and received
// Default Response.
 uint8_t zclOTA_OtaUpgradeEndReqTransSeq;

 uint8_t  zclOTA_Permit;
 uint8_t currentOtaEndpoint = OTA_UNUSED_ENDPOINT ;

//Offset in external flash at which the binary is being written
uint32_t binaryAddrOffset = 0;
//Address at which the Zigbee binary starts in flash
// Note: Offset to the NVS flash region, not an absolute address
uint32_t binaryAddrStart  = 0x0;


// oad parsing state
uint8_t oadPdState = 0;
#define OAD_GET_IMAGE_HDR_STATE      0
#define OAD_GET_IMAGE_PAYLOAD_STATE  1
#define OAD_IMAGE_COMPLETED_STATE    2

/******************************************************************************
 * LOCAL VARIABLES
 */

// Sequence number
#if (defined OTA_CLIENT_STANDALONE) || (defined OTA_CLIENT_INTEGRATED) || (defined OTA_SERVER)
static uint8_t zclOTA_SeqNo = 0;
#endif

#if defined OTA_MMO_SIGN
static OTA_MmoCtrl_t zclOTA_MmoHash;
static uint8_t zclOTA_DataToHash[OTA_MMO_HASH_SIZE];
static uint8_t zclOTA_HashPos;
static uint8_t zclOTA_SignerIEEE[Z_EXTADDR_LEN];
static uint8_t zclOTA_SignatureData[OTA_SIGNATURE_LEN];
static uint8_t zclOTA_Certificate[OTA_CERTIFICATE_LEN];
#endif // OTA_MMO_SIGN

#if (defined OTA_CLIENT_STANDALONE) || (defined OTA_CLIENT_INTEGRATED)
static uint16_t zclOTA_HeaderLen;            // Image header length
static uint16_t zclOTA_ElementTag;
static uint32_t zclOTA_ElementLen;
static uint32_t zclOTA_ElementPos;

// local OTA attribute pointers
static uint32_t *zclOTA_FileOffset;
static uint16_t *zclOTA_DownloadedZigBeeStackVersion;
static uint8_t  *zclOTA_ImageUpgradeStatus;

// OTA Header Magic Number Bytes
static const uint8_t zclOTA_HdrMagic[] = {0x1E, 0xF1, 0xEE, 0x0B};

imgHdr_t oad_imgHdr = {0};
uint16_t   oad_imgHdr_pos = 0;
#endif // (defined OTA_CLIENT_STANDALONE) || (defined OTA_CLIENT_INTEGRATED

/******************************************************************************
 * LOCAL FUNCTIONS
 */
#if (defined OTA_CLIENT_STANDALONE) || (defined OTA_CLIENT_INTEGRATED)
#if defined (ZCL_STANDALONE)
static uint8_t* buffer_uint32( uint8_t *buf, uint32_t val );
#endif
#endif

#if (defined OTA_CLIENT_STANDALONE) || (defined OTA_CLIENT_INTEGRATED)
static uint8_t oadEraseExtFlashPages(uint8_t imgStartPage, uint8_t imgPageLen);
static uint8_t oadCheckDL(uint8_t imagePage);
#endif

/******************************************************************************
 * @fn      zclOTA_PermitOta
 *
 * @brief   Called to enable/disable OTA operation.
 *
 * @param   permit - TRUE to enable OTA, FALSE to diable OTA
 *
 * @return  none
 */
void zclOTA_PermitOta ( uint8_t permit )
{
  zclOTA_Permit = permit;
}

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
uint8_t zclOTA_getSeqNo(void)
{
  return zclOTA_SeqNo;
}
#endif

#if !defined (ZCL_STANDALONE)
/******************************************************************************
 * @fn      zclOTA_ProcessZCLMsgs
 *
 * @brief   Process unhandled foundation ZCL messages for the OTA End Point.
 *
 * @param   pMsg - a Pointer to the ZCL message
 *
 * @return  none
 */
void zclOTA_ProcessUnhandledFoundationZCLMsgs ( zclIncomingMsg_t *pMsg )
{
  switch ( pMsg->zclHdr.commandID )
  {
    case ZCL_CMD_DEFAULT_RSP:
      zclOTA_ProcessInDefaultRspCmd( pMsg );
      break;
    default :
      break;
  }

  if ( pMsg->attrCmd )
  {
    OsalPort_free( pMsg->attrCmd );
    pMsg->attrCmd = NULL;
  }
}
#endif

#if defined OTA_SERVER
/******************************************************************************
 * @fn      zclOTA_SendImageNotify
 *
 * @brief   Send an OTA Image Notify message.
 *
 * @param   srcEp   - endpoint from which the message is send
 * @param   dstAddr - where you want the message to go
 * @param   pParams - message parameters
 *
 * @return  ZStatus_t
 */
ZStatus_t zclOTA_SendImageNotify (uint8_t srcEp, afAddrType_t *dstAddr,
                                   zclOTA_ImageNotifyParams_t *pParams )
{
  ZStatus_t status;
  uint8_t buf[PAYLOAD_MAX_LEN_IMAGE_NOTIFY];
  uint8_t *pBuf = buf;
  bool disableDefaultRsp = TRUE;

  *pBuf++ = pParams->payloadType;
  *pBuf++ = pParams->queryJitter;
  if ( pParams->payloadType >= NOTIFY_PAYLOAD_JITTER_MFG )
  {
    *pBuf++ = LO_UINT16 ( pParams->fileId.manufacturer );
    *pBuf++ = HI_UINT16 ( pParams->fileId.manufacturer );
  }
  if ( pParams->payloadType >= NOTIFY_PAYLOAD_JITTER_MFG_TYPE )
  {
    *pBuf++ = LO_UINT16 ( pParams->fileId.type );
    *pBuf++ = HI_UINT16 ( pParams->fileId.type );
  }
  if ( pParams->payloadType == NOTIFY_PAYLOAD_JITTER_MFG_TYPE_VERS )
  {
    pBuf = OsalPort_bufferUint32 ( pBuf, pParams->fileId.version );
  }

  if(afAddr16Bit == dstAddr->addrMode)
  {
    // For unicast disableDefaultRsp should not be set
    disableDefaultRsp = FALSE;
  }

  status = zcl_SendCommand ( srcEp, dstAddr, ZCL_CLUSTER_ID_OTA,
                             COMMAND_OTA_UPGRADE_IMAGE_NOTIFY, TRUE,
                             ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0,
                             zclOTA_SeqNo++, ( uint16_t ) ( pBuf - buf ), buf );

  return status;
}

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
ZStatus_t zclOTA_SendQueryNextImageRsp (uint8_t srcEp, afAddrType_t *dstAddr,
    zclOTA_QueryImageRspParams_t *pParams, uint8_t transSeqNum )
{
  ZStatus_t status;
  uint8_t buf[PAYLOAD_MAX_LEN_QUERY_NEXT_IMAGE_RSP];
  uint8_t *pBuf = buf;

  *pBuf++ = pParams->status;
  if ( pParams->status == ZCL_STATUS_SUCCESS )
  {
    *pBuf++ = LO_UINT16 ( pParams->fileId.manufacturer );
    *pBuf++ = HI_UINT16 ( pParams->fileId.manufacturer );
    *pBuf++ = LO_UINT16 ( pParams->fileId.type );
    *pBuf++ = HI_UINT16 ( pParams->fileId.type );
    pBuf = OsalPort_bufferUint32 ( pBuf, pParams->fileId.version );
    pBuf = OsalPort_bufferUint32 ( pBuf, pParams->imageSize );
  }

  status = zcl_SendCommand ( srcEp, dstAddr, ZCL_CLUSTER_ID_OTA,
                             COMMAND_OTA_UPGRADE_QUERY_NEXT_IMAGE_RESPONSE, TRUE,
                             ZCL_FRAME_SERVER_CLIENT_DIR, TRUE, 0,
                             transSeqNum, ( uint16_t ) ( pBuf - buf ), buf );

  return status;
}

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
ZStatus_t zclOTA_SendImageBlockRsp (uint8_t srcEp, afAddrType_t *dstAddr,
    zclOTA_ImageBlockRspParams_t *pParams, uint8_t transSeqNum )
{
  uint8_t *buf;
  uint8_t *pBuf;
  ZStatus_t status;
  uint8_t len;

  if ( pParams->status == ZCL_STATUS_SUCCESS )
  {
    len = PAYLOAD_MAX_LEN_IMAGE_BLOCK_RSP + pParams->rsp.success.dataSize;
  }
  else if ( pParams->status == ZCL_STATUS_WAIT_FOR_DATA )
  {
    len = PAYLOAD_MIN_LEN_IMAGE_BLOCK_WAIT;
  }
  else
  {
    len = 1;
  }

  buf = OsalPort_malloc ( len );

  if ( buf == NULL )
  {
    return ( ZMemError );
  }

  pBuf = buf;
  *pBuf++ = pParams->status;

  if ( pParams->status == ZCL_STATUS_SUCCESS )
  {
    *pBuf++ = LO_UINT16 ( pParams->rsp.success.fileId.manufacturer );
    *pBuf++ = HI_UINT16 ( pParams->rsp.success.fileId.manufacturer );
    *pBuf++ = LO_UINT16 ( pParams->rsp.success.fileId.type );
    *pBuf++ = HI_UINT16 ( pParams->rsp.success.fileId.type );
    pBuf = OsalPort_bufferUint32 ( pBuf, pParams->rsp.success.fileId.version );
    pBuf = OsalPort_bufferUint32 ( pBuf, pParams->rsp.success.fileOffset );
    *pBuf++ = pParams->rsp.success.dataSize;
    OsalPort_memcpy ( pBuf, pParams->rsp.success.pData, pParams->rsp.success.dataSize );
  }
  else if ( pParams->status == ZCL_STATUS_WAIT_FOR_DATA )
  {
    pBuf = OsalPort_bufferUint32 ( pBuf, pParams->rsp.wait.currentTime );
    pBuf = OsalPort_bufferUint32 ( pBuf, pParams->rsp.wait.requestTime );
    *pBuf++ = LO_UINT16 ( pParams->rsp.wait.blockReqDelay );
    *pBuf++ = HI_UINT16 ( pParams->rsp.wait.blockReqDelay );
  }

  status = zcl_SendCommand ( srcEp, dstAddr, ZCL_CLUSTER_ID_OTA,
                             COMMAND_OTA_UPGRADE_IMAGE_BLOCK_RESPONSE, TRUE,
                             ZCL_FRAME_SERVER_CLIENT_DIR, TRUE, 0,
                             transSeqNum, len, buf );

  OsalPort_free ( buf );

  return status;
}

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
ZStatus_t zclOTA_SendUpgradeEndRsp (uint8_t srcEp, afAddrType_t *dstAddr,
    zclOTA_UpgradeEndRspParams_t *pParams, uint8_t transSeqNum )
{
  ZStatus_t status;
  uint8_t buf[PAYLOAD_MAX_LEN_UPGRADE_END_RSP];
  uint8_t *pBuf = buf;

  *pBuf++ = LO_UINT16 ( pParams->fileId.manufacturer );
  *pBuf++ = HI_UINT16 ( pParams->fileId.manufacturer );
  *pBuf++ = LO_UINT16 ( pParams->fileId.type );
  *pBuf++ = HI_UINT16 ( pParams->fileId.type );
  pBuf = OsalPort_bufferUint32 ( pBuf, pParams->fileId.version );
  pBuf = OsalPort_bufferUint32 ( pBuf, pParams->currentTime );
  pBuf = OsalPort_bufferUint32 ( pBuf, pParams->upgradeTime );

  status = zcl_SendCommand ( srcEp, dstAddr, ZCL_CLUSTER_ID_OTA,
                             COMMAND_OTA_UPGRADE_UPGRADE_END_RESPONSE, TRUE,
                             ZCL_FRAME_SERVER_CLIENT_DIR, TRUE, 0,
                             transSeqNum, PAYLOAD_MAX_LEN_UPGRADE_END_RSP, buf );

  return status;
}

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
ZStatus_t zclOTA_SendQuerySpecificFileRsp (uint8_t srcEp, afAddrType_t *dstAddr,
    zclOTA_QueryImageRspParams_t *pParams, uint8_t transSeqNum )
{
  ZStatus_t status;
  uint8_t buf[PAYLOAD_MAX_LEN_QUERY_SPECIFIC_FILE_RSP];
  uint8_t *pBuf = buf;

  *pBuf++ = pParams->status;
  if ( pParams->status == ZCL_STATUS_SUCCESS )
  {
    *pBuf++ = LO_UINT16 ( pParams->fileId.manufacturer );
    *pBuf++ = HI_UINT16 ( pParams->fileId.manufacturer );
    *pBuf++ = LO_UINT16 ( pParams->fileId.type );
    *pBuf++ = HI_UINT16 ( pParams->fileId.type );
    pBuf = OsalPort_bufferUint32 ( pBuf, pParams->fileId.version );
    pBuf = OsalPort_bufferUint32 ( pBuf, pParams->imageSize );
  }

  status = zcl_SendCommand ( srcEp, dstAddr, ZCL_CLUSTER_ID_OTA,
                             COMMAND_OTA_UPGRADE_QUERY_DEVICE_SPECIFIC_FILE_RESPONSE, TRUE,
                             ZCL_FRAME_SERVER_CLIENT_DIR, TRUE, 0,
                             transSeqNum, ( uint16_t ) ( pBuf - buf ), buf );

  return status;
}
#endif // defined OTA_SERVER


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
void zclOTA_ProcessInDefaultRspCmd( zclIncomingMsg_t *pInMsg )
{
  // If the OTA server issued a Default Response, most likely something bad
  // happened.

  zclDefaultRspCmd_t *defRspCmd = (zclDefaultRspCmd_t*)pInMsg->attrCmd;

  switch ( defRspCmd->statusCode )
  {

    case ( ZCL_STATUS_ABORT ) :

      switch ( *zclOTA_ImageUpgradeStatus )
      {
        case ( OTA_STATUS_COMPLETE ) :
          if ( pInMsg->zclHdr.transSeqNum == zclOTA_OtaUpgradeEndReqTransSeq )
          {
            // The server has issued an ABORT while we were waiting for the
            // Upgrade End Response.
            *zclOTA_ImageUpgradeStatus = OTA_STATUS_NORMAL;

            zclOTA_OtaUpgradeEndReqTransSeq = 0;
          }
          break;

        // Handling for reception of the Default Response with status code ==
        // ABORT, in other OTA states, can be added here.

        default :
          break;
      }
      break;

    // Handling for other Defautl Response status codes and OTA states can
    // be added here.
    default :
      break;
  }
}
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
ZStatus_t zclOTA_SendQueryNextImageReq (uint8_t srcEp, afAddrType_t *dstAddr,
    zclOTA_QueryNextImageReqParams_t *pParams )
{
  ZStatus_t status;
  uint8_t buf[PAYLOAD_MAX_LEN_QUERY_NEXT_IMAGE_REQ];
  uint8_t *pBuf = buf;

  *pBuf++ = pParams->fieldControl;
  *pBuf++ = LO_UINT16 ( pParams->fileId.manufacturer );
  *pBuf++ = HI_UINT16 ( pParams->fileId.manufacturer );
  *pBuf++ = LO_UINT16 ( pParams->fileId.type );
  *pBuf++ = HI_UINT16 ( pParams->fileId.type );
  pBuf = buffer_uint32 ( pBuf, pParams->fileId.version );
  if ( pParams->fieldControl == 1 )
  {
    *pBuf++ = LO_UINT16 ( pParams->hardwareVersion );
    *pBuf++ = HI_UINT16 ( pParams->hardwareVersion );
  }

  status = zcl_SendCommand ( srcEp, dstAddr, ZCL_CLUSTER_ID_OTA,
                             COMMAND_OTA_UPGRADE_QUERY_NEXT_IMAGE_REQUEST, TRUE,
                             ZCL_FRAME_CLIENT_SERVER_DIR, FALSE, 0,
                             zclOTA_SeqNo++, ( uint16_t ) ( pBuf - buf ), buf );

  return status;
}

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
ZStatus_t zclOTA_SendImageBlockReq (uint8_t srcEp, afAddrType_t *dstAddr,
                                     zclOTA_ImageBlockReqParams_t *pParams )
{
  ZStatus_t status;
  uint8_t buf[PAYLOAD_MAX_LEN_IMAGE_BLOCK_REQ];
  uint8_t *pBuf = buf;

  *pBuf++ = pParams->fieldControl;
  *pBuf++ = LO_UINT16 ( pParams->fileId.manufacturer );
  *pBuf++ = HI_UINT16 ( pParams->fileId.manufacturer );
  *pBuf++ = LO_UINT16 ( pParams->fileId.type );
  *pBuf++ = HI_UINT16 ( pParams->fileId.type );
  pBuf = buffer_uint32 ( pBuf, pParams->fileId.version );
  pBuf = buffer_uint32 ( pBuf, pParams->fileOffset );
  *pBuf++ = pParams->maxDataSize;

  if ( ( pParams->fieldControl & OTA_BLOCK_FC_NODES_IEEE_PRESENT ) != 0 )
  {
    osal_cpyExtAddr ( pBuf, pParams->nodeAddr );
    pBuf += Z_EXTADDR_LEN;
  }

  if ( ( pParams->fieldControl & OTA_BLOCK_FC_REQ_DELAY_PRESENT ) != 0 )
  {
    *pBuf++ = LO_UINT16 ( pParams->blockReqDelay );
    *pBuf++ = HI_UINT16 ( pParams->blockReqDelay );
  }

  status = zcl_SendCommand ( srcEp, dstAddr, ZCL_CLUSTER_ID_OTA,
                             COMMAND_OTA_UPGRADE_IMAGE_BLOCK_REQUEST, TRUE,
                             ZCL_FRAME_CLIENT_SERVER_DIR, FALSE, 0,
                             zclOTA_SeqNo++, ( uint16_t ) ( pBuf - buf ), buf );

  return status;
}


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
ZStatus_t zclOTA_SendImagePageReq (uint8_t srcEp, afAddrType_t *dstAddr,
                                   zclOTA_ImagePageReqParams_t *pParams )
{
  ZStatus_t status;
  uint8_t buf[PAYLOAD_MAX_LEN_IMAGE_PAGE_REQ];
  uint8_t *pBuf = buf;

  *pBuf++ = pParams->fieldControl;
  *pBuf++ = LO_UINT16 ( pParams->fileId.manufacturer );
  *pBuf++ = HI_UINT16 ( pParams->fileId.manufacturer );
  *pBuf++ = LO_UINT16 ( pParams->fileId.type );
  *pBuf++ = HI_UINT16 ( pParams->fileId.type );
  pBuf = buffer_uint32 ( pBuf, pParams->fileId.version );
  pBuf = buffer_uint32 ( pBuf, pParams->fileOffset );
  *pBuf++ = pParams->maxDataSize;

  *pBuf++ = LO_UINT16 ( pParams->pageSize );
  *pBuf++ = HI_UINT16 ( pParams->pageSize );
  *pBuf++ = LO_UINT16 ( pParams->responseSpacing );
  *pBuf++ = HI_UINT16 ( pParams->responseSpacing );


  if ( ( pParams->fieldControl & OTA_BLOCK_FC_NODES_IEEE_PRESENT ) != 0 )
  {
    osal_cpyExtAddr ( pBuf, pParams->nodeAddr );
    pBuf += Z_EXTADDR_LEN;
  }

  status = zcl_SendCommand ( srcEp, dstAddr, ZCL_CLUSTER_ID_OTA,
                             COMMAND_OTA_UPGRADE_IMAGE_PAGE_REQUEST, TRUE,
                             ZCL_FRAME_CLIENT_SERVER_DIR, FALSE, 0,
                             zclOTA_SeqNo++, ( uint16_t ) ( pBuf - buf ), buf );

  return status;
}



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
ZStatus_t zclOTA_SendQueryDevSpecFileReq (uint8_t srcEp, afAddrType_t *dstAddr,
                                          zclOTA_QuerySpecificFileReqParams_t *pParams )
{
  ZStatus_t status;
  uint8_t buf[PAYLOAD_MAX_LEN_QUERY_SPECIFIC_FILE_REQ];
  uint8_t *pBuf = buf;

  osal_cpyExtAddr ( pBuf, pParams->nodeAddr );
  pBuf += Z_EXTADDR_LEN;

  *pBuf++ = LO_UINT16 ( pParams->fileId.manufacturer );
  *pBuf++ = HI_UINT16 ( pParams->fileId.manufacturer );
  *pBuf++ = LO_UINT16 ( pParams->fileId.type );
  *pBuf++ = HI_UINT16 ( pParams->fileId.type );
  pBuf = buffer_uint32 ( pBuf, pParams->fileId.version );

  *pBuf++ = LO_UINT16 ( pParams->stackVersion );
  *pBuf++ = HI_UINT16 ( pParams->stackVersion );


  status = zcl_SendCommand ( srcEp, dstAddr, ZCL_CLUSTER_ID_OTA,
                             COMMAND_OTA_UPGRADE_QUERY_DEVICE_SPECIFIC_FILE_REQUEST, TRUE,
                             ZCL_FRAME_CLIENT_SERVER_DIR, FALSE, 0,
                             zclOTA_SeqNo++, ( uint16_t ) ( pBuf - buf ), buf );

  return status;
}


#if defined (ZCL_STANDALONE)
/*********************************************************************
 * @fn      buffer_uint32
 *
 * @brief
 *
 *   Buffer an uint32_t value - LSB first.
 *
 * @param   buf - buffer
 * @param   val - uint32_t value
 *
 * @return  pointer to end of destination buffer
 */
static uint8_t* buffer_uint32( uint8_t *buf, uint32_t val )
{
  *buf++ = BREAK_UINT32( val, 0 );
  *buf++ = BREAK_UINT32( val, 1 );
  *buf++ = BREAK_UINT32( val, 2 );
  *buf++ = BREAK_UINT32( val, 3 );

  return buf;
}
#endif

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
ZStatus_t zclOTA_SendUpgradeEndReq (uint8_t srcEp, afAddrType_t *dstAddr,
                                     zclOTA_UpgradeEndReqParams_t *pParams )
{
  ZStatus_t status;
  uint8_t buf[PAYLOAD_MAX_LEN_UPGRADE_END_REQ];
  uint8_t *pBuf = buf;

  *pBuf++ = pParams->status;
  *pBuf++ = LO_UINT16 ( pParams->fileId.manufacturer );
  *pBuf++ = HI_UINT16 ( pParams->fileId.manufacturer );
  *pBuf++ = LO_UINT16 ( pParams->fileId.type );
  *pBuf++ = HI_UINT16 ( pParams->fileId.type );
  pBuf = buffer_uint32 ( pBuf, pParams->fileId.version );

  zclOTA_OtaUpgradeEndReqTransSeq = zclOTA_SeqNo++;

  status = zcl_SendCommand ( srcEp, dstAddr, ZCL_CLUSTER_ID_OTA,
                             COMMAND_OTA_UPGRADE_UPGRADE_END_REQUEST, TRUE,
                             ZCL_FRAME_CLIENT_SERVER_DIR, FALSE, 0,
                             zclOTA_OtaUpgradeEndReqTransSeq, ( uint16_t ) ( pBuf - buf ), buf );

  return status;
}


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
uint8_t zclOTA_ProcessImageData ( uint8_t *pData, uint8_t len )
{
  int8_t i;
#if defined OTA_MMO_SIGN
  uint8_t skipHash = FALSE;
#endif

  if ( *zclOTA_ImageUpgradeStatus != OTA_STATUS_IN_PROGRESS )
  {
    return ZCL_STATUS_ABORT;
  }

  for ( i=0; i<len; i++ )
  {
    switch ( zclOTA_ClientPdState )
    {
        // verify header magic number
      case ZCL_OTA_PD_MAGIC_0_STATE:
        // Initialize control variables
#if defined OTA_MMO_SIGN
        memset ( &zclOTA_MmoHash, 0, sizeof ( zclOTA_MmoHash ) );
        zclOTA_HashPos = 0;
#endif

        // Missing break intended
      case ZCL_OTA_PD_MAGIC_1_STATE:
      case ZCL_OTA_PD_MAGIC_2_STATE:
      case ZCL_OTA_PD_MAGIC_3_STATE:
        if ( pData[i] != zclOTA_HdrMagic[zclOTA_ClientPdState] )
        {
          return ZCL_STATUS_INVALID_IMAGE;
        }
        zclOTA_ClientPdState++;
        break;

      case ZCL_OTA_PD_HDR_LEN1_STATE:
        // get header length
        if ( *zclOTA_FileOffset == ZCL_OTA_HDR_LEN_OFFSET )
        {
          zclOTA_HeaderLen = pData[i];
          zclOTA_ClientPdState = ZCL_OTA_PD_HDR_LEN2_STATE;
        }
        break;

      case ZCL_OTA_PD_HDR_LEN2_STATE:
        zclOTA_HeaderLen |= ( ( ( uint16_t ) pData[i] ) << 8 ) & 0xFF00;
        zclOTA_ClientPdState = ZCL_OTA_PD_STK_VER1_STATE;
        break;

      case ZCL_OTA_PD_STK_VER1_STATE:
        // get stack version
        if ( *zclOTA_FileOffset == ZCL_OTA_STK_VER_OFFSET )
        {
          *zclOTA_DownloadedZigBeeStackVersion = pData[i];
          zclOTA_ClientPdState = ZCL_OTA_PD_STK_VER2_STATE;
        }
        break;

      case ZCL_OTA_PD_STK_VER2_STATE:
        *zclOTA_DownloadedZigBeeStackVersion |= ( ( ( uint16_t ) pData[i] ) << 8 ) & 0xFF00;
        zclOTA_ClientPdState = ZCL_OTA_PD_CONT_HDR_STATE;

        if ( *zclOTA_DownloadedZigBeeStackVersion != OTA_HDR_STACK_VERSION )
        {
          return ZCL_STATUS_INVALID_IMAGE;
        }
        break;

      case ZCL_OTA_PD_CONT_HDR_STATE:
        // Complete the header
        if ( *zclOTA_FileOffset == zclOTA_HeaderLen-1 )
        {
          zclOTA_ClientPdState = ZCL_OTA_PD_ELEM_TAG1_STATE;
        }
        break;

      case ZCL_OTA_PD_ELEM_TAG1_STATE:
        zclOTA_ElementTag = pData[i];
        zclOTA_ClientPdState = ZCL_OTA_PD_ELEM_TAG2_STATE;
        break;

      case ZCL_OTA_PD_ELEM_TAG2_STATE:
        zclOTA_ElementTag |= ( ( ( uint16_t ) pData[i] ) << 8 ) & 0xFF00;
        zclOTA_ElementPos = 0;
        zclOTA_ClientPdState = ZCL_OTA_PD_ELEM_LEN1_STATE;
        break;

      case ZCL_OTA_PD_ELEM_LEN1_STATE:
        zclOTA_ElementLen = pData[i];
        zclOTA_ClientPdState = ZCL_OTA_PD_ELEM_LEN2_STATE;
        break;

      case ZCL_OTA_PD_ELEM_LEN2_STATE:
        zclOTA_ElementLen |= ( ( uint32_t ) pData[i] << 8 ) & 0x0000FF00;
        zclOTA_ClientPdState = ZCL_OTA_PD_ELEM_LEN3_STATE;
        break;

      case ZCL_OTA_PD_ELEM_LEN3_STATE:
        zclOTA_ElementLen |= ( ( uint32_t ) pData[i] << 16 ) & 0x00FF0000;
        zclOTA_ClientPdState = ZCL_OTA_PD_ELEM_LEN4_STATE;
        break;

      case ZCL_OTA_PD_ELEM_LEN4_STATE:
        zclOTA_ElementLen |= ( ( uint32_t ) pData[i] << 24 ) & 0xFF000000;
        zclOTA_ClientPdState = ZCL_OTA_PD_ELEMENT_STATE;

        // Make sure the length of the element isn't bigger than the image
        if ( zclOTA_ElementLen > ( zclOTA_DownloadedImageSize - *zclOTA_FileOffset ) )
        {
          return ZCL_STATUS_INVALID_IMAGE;
        }

        if(zclOTA_ElementTag == OTA_UPGRADE_IMAGE_TAG_ID)
        {
            //Initialize OAD state
            oadPdState = OAD_GET_IMAGE_HDR_STATE;
            oad_imgHdr_pos = 0;
            memset(&oad_imgHdr,0,sizeof(oad_imgHdr));
        }
#if defined OTA_MMO_SIGN
        if ( zclOTA_ElementTag == OTA_ECDSA_SIGNATURE_TAG_ID )
        {
          if ( zclOTA_ElementLen != OTA_SIGNATURE_LEN + Z_EXTADDR_LEN )
          {
            return ZCL_STATUS_INVALID_IMAGE;
          }
        }
        else if ( zclOTA_ElementTag == OTA_ECDSA_CERT_TAG_ID )
        {
          if ( zclOTA_ElementLen != OTA_CERTIFICATE_LEN )
          {
            return ZCL_STATUS_INVALID_IMAGE;
          }
        }
#endif
        break;

      case ZCL_OTA_PD_ELEMENT_STATE:
#if defined OTA_MMO_SIGN
        if ( zclOTA_ElementTag == OTA_ECDSA_SIGNATURE_TAG_ID )
        {
          if ( zclOTA_ElementPos < Z_EXTADDR_LEN )
            zclOTA_SignerIEEE[zclOTA_ElementPos] = pData[i];
          else
          {
            zclOTA_SignatureData[zclOTA_ElementPos - Z_EXTADDR_LEN] = pData[i];

            skipHash = TRUE;
          }
        }
        else if ( zclOTA_ElementTag == OTA_ECDSA_CERT_TAG_ID )
        {
          zclOTA_Certificate[zclOTA_ElementPos] = pData[i];
        }
#endif

        if(zclOTA_ElementTag == OTA_UPGRADE_IMAGE_TAG_ID)
        {
            // Get the header from the OTA frame

            switch (oadPdState)
            {
                case OAD_GET_IMAGE_HDR_STATE:
                {
                    uint8_t oad_imageID[] = OAD_IMG_ID_VAL;
#ifndef OTA_ONCHIP
                    uint8_t oad_externalFLashID[] = OAD_EXTFL_ID_VAL;
#endif

                    if(oad_imgHdr_pos < sizeof(oad_imgHdr))
                    {
                        ((uint8_t*)&oad_imgHdr)[oad_imgHdr_pos] = pData[i];
                        oad_imgHdr_pos++;

                        //Keep track of OTA element bytes received
                        zclOTA_ElementPos++;
                        break;
                    }

                    //Header complete, validate the image header
                    //BIM version must be compatible, OTA image received must be Zigbee (for this release), OTA image received must be App+StackLib
                    if( (memcmp(oad_imgHdr.fixedHdr.imgID, oad_imageID, 8) != 0)     ||
                           (oad_imgHdr.fixedHdr.bimVer != BIM_VER )                    ||
                           (oad_imgHdr.fixedHdr.techType != OAD_WIRELESS_TECH_ZIGBEE ) ||
                           (oad_imgHdr.fixedHdr.imgType != OAD_IMG_TYPE_APPSTACKLIB  )  )
                    {
                       //Initialize OAD state
                       oadPdState = OAD_GET_IMAGE_HDR_STATE;
                       oad_imgHdr_pos = 0;

                       memset(&oad_imgHdr,0,sizeof(oad_imgHdr));

                       return ZCL_STATUS_INVALID_IMAGE;
                    }

                    /* Zigbee OAD assumptions:
                     * Factory New Metadata and binary image exist in external flash.
                     * Zigbee OAD will always take the next slot available after FN header and binary
                     */

                    if(flash_open() != 0)
                    {
#ifndef OTA_ONCHIP
                        //Search for a metadata header to fit the Zigbee image
                        ExtImageInfo_t oad_imgHdrFactoryNew;
#endif

                        //OAD binary pages required to store the binary
                        uint8_t   binaryPagesLen   = 0;

#ifndef OTA_ONCHIP
                        // Read the factory new metadata page
                        readFlash(EFL_ADDR_META_FACT_IMG, (uint8_t *)&oad_imgHdrFactoryNew, EFL_METADATA_LEN);

                        //is a valid header
                        if(memcmp(&oad_imgHdrFactoryNew.fixedHdr.imgID, oad_externalFLashID, sizeof(oad_externalFLashID)) != 0)
                        {
                            flash_close();
                            //This release does not support not having the Factory New image
                            return ZCL_STATUS_ABORT;
                        }

                        /* FROM BLE STACK:
                           Note currently we have problem in erasing last flash page,
                           workaround to leave last page */
                        //Where Factory New image is stored
                        binaryAddrOffset = EFL_FLASH_SIZE - EFL_PAGE_SIZE;
#else
                        binaryAddrOffset = binaryAddrStart;
#endif

#ifndef OTA_ONCHIP
                        //Offset the Factory New image size (as binaries are store from top to bottom of the external flash, by append we mean substract the address)
                        binaryAddrOffset -= EXT_FLASH_ADDRESS(EXT_FLASH_PAGE(oad_imgHdrFactoryNew.fixedHdr.len) + 1 , 0);

                        //Offset the Zigbee image size (as binaries are store from top to bottom of the external flash, by append we mean substract the address)
                        binaryAddrOffset -= EXT_FLASH_ADDRESS(EXT_FLASH_PAGE(oad_imgHdr.fixedHdr.len) + 1 , 0);
                        //Address at which the Zigbee binary starts
                        binaryAddrStart = binaryAddrOffset;

                        binaryPagesLen = EXT_FLASH_PAGE(oad_imgHdr.fixedHdr.len) + 1;
#else
                        binaryPagesLen = FLASH_PAGE(oad_imgHdr.fixedHdr.len) + 1;
#endif

                        //Erase the image space for the incoming binary
#ifndef OTA_ONCHIP
                        if(oadEraseExtFlashPages(EXT_FLASH_PAGE(binaryAddrOffset) , binaryPagesLen))
#else
                        if(oadEraseExtFlashPages(FLASH_PAGE(binaryAddrOffset) , binaryPagesLen))
#endif
                        {
                            flash_close();
                            //Something went wrong...
                            return ZCL_STATUS_ABORT;
                        }

#ifndef OTA_ONCHIP
                        //Erase the header file after the Factory New metadata header
                        if(eraseFlashPg(EXT_FLASH_PAGE((EFL_ADDR_META_FACT_IMG + EFL_PAGE_SIZE)) != FLASH_SUCCESS))
                        {
                            flash_close();
                            //Something went wrong...
                            return ZCL_STATUS_ABORT;
                        }

                        //We have the Zigbee OAD header, so lets write it.
                        //Populate the right metadata header for the table

                            ExtImageInfo_t  ExtImageInfo;

                            OsalPort_memcpy(&ExtImageInfo.fixedHdr,&oad_imgHdr.fixedHdr,OAD_IMG_HDR_LEN);

                            OsalPort_memcpy(&ExtImageInfo.fixedHdr.imgID, oad_externalFLashID,sizeof(oad_externalFLashID));

                            ExtImageInfo.extFlAddr = binaryAddrStart;
                            ExtImageInfo.counter = 0;

                            if(writeFlashPg(EXT_FLASH_PAGE((EFL_ADDR_META + EFL_PAGE_SIZE)),0, (uint8_t *)&ExtImageInfo, sizeof(ExtImageInfo_t)) != FLASH_SUCCESS)
                            {
                                flash_close();
                                //Something went wrong...
                                return ZCL_STATUS_ABORT;
                            }
#endif

                        //Also write the OAD header into the external flash binary section
#ifndef OTA_ONCHIP
                        if(writeFlashPg(EXT_FLASH_PAGE(binaryAddrOffset), 0,  (uint8_t *)&oad_imgHdr, sizeof (imgHdr_t)) != FLASH_SUCCESS)
#else
                        if(writeFlashPg(FLASH_PAGE(binaryAddrOffset), 0,  (uint8_t *)&oad_imgHdr, sizeof (imgHdr_t)) != FLASH_SUCCESS)
#endif
                        {
                            flash_close();
                            //Something went wrong...
                            return ZCL_STATUS_ABORT;
                        }

                        binaryAddrOffset += sizeof (imgHdr_t);

                        //add the remaining payload image into the image section
#ifndef OTA_ONCHIP
                        if(writeFlashPg(EXT_FLASH_PAGE(binaryAddrOffset), binaryAddrOffset & (~EXTFLASH_PAGE_MASK), &pData[i], len - i ) != FLASH_SUCCESS)
#else
                        if(writeFlashPg(FLASH_PAGE(binaryAddrOffset), binaryAddrOffset & (~INTFLASH_PAGE_MASK), &pData[i], len - i ) != FLASH_SUCCESS)
#endif
                        {
                            flash_close();
                            //Something went wrong...
                            return ZCL_STATUS_ABORT;
                        }

                        //Update the address offset in external flash
                        binaryAddrOffset += len - i;

                        //Keep track of OTA element bytes received
                        zclOTA_ElementPos += len - i;

                        *zclOTA_FileOffset += len - i - 1;  //Minus 1 due to *zclOTA_FileOffset being increased at if ( ++*zclOTA_FileOffset >= zclOTA_DownloadedImageSize )

                        //Had copied the whole packet already, so skip this frame.
                        i = len;

                        oadPdState = OAD_GET_IMAGE_PAYLOAD_STATE;
                    }
                    else
                    {
                        //Could not open the external flash
                        return ZCL_STATUS_ABORT;
                    }

                }
                break;

                case OAD_GET_IMAGE_PAYLOAD_STATE:

                    //Keep track of OTA element bytes received
                    zclOTA_ElementPos += len;

                    if(flash_open() != 0)
                    {
                        //If for some reason the OTA Upgrade file is larger than what OAD header indicates, only copy what OAD header indicates
                        if(binaryAddrOffset + len < binaryAddrStart + oad_imgHdr.fixedHdr.len)
                        {
                            //add the image payload into the image section
#ifndef OTA_ONCHIP
                            if(writeFlashPg(EXT_FLASH_PAGE(binaryAddrOffset), binaryAddrOffset & (~EXTFLASH_PAGE_MASK), &pData[i], len ) != FLASH_SUCCESS)
#else
                            if(writeFlashPg(FLASH_PAGE(binaryAddrOffset), binaryAddrOffset & (~INTFLASH_PAGE_MASK), &pData[i], len ) != FLASH_SUCCESS)
#endif
                            {
                                flash_close();
                                //Something went wrong...
                                return ZCL_STATUS_ABORT;
                            }
                            binaryAddrOffset += len;
                            *zclOTA_FileOffset += len - 1; //Minus 1 due to *zclOTA_FileOffset being increased at if ( ++*zclOTA_FileOffset >= zclOTA_DownloadedImageSize )
                        }
                        else
                        {
                            //Calculate the remaining bytes to be copied
                            uint32_t remaining;
                            remaining = oad_imgHdr.fixedHdr.len - (binaryAddrOffset - binaryAddrStart);

                            //add the image payload into the image section
#ifndef OTA_ONCHIP
                            if(writeFlashPg(EXT_FLASH_PAGE(binaryAddrOffset), binaryAddrOffset & (~EXTFLASH_PAGE_MASK), &pData[i],  remaining ) != FLASH_SUCCESS)
#else
                            if(writeFlashPg(FLASH_PAGE(binaryAddrOffset), binaryAddrOffset & (~INTFLASH_PAGE_MASK), &pData[i],  remaining ) != FLASH_SUCCESS)
#endif
                            {
                                flash_close();
                                //Something went wrong...
                                return ZCL_STATUS_ABORT;
                            }
                            binaryAddrOffset += remaining;
                            *zclOTA_FileOffset += remaining - 1; //Minus 1 due to *zclOTA_FileOffset being increased at if ( ++*zclOTA_FileOffset >= zclOTA_DownloadedImageSize )

                            //check the binary copied CRC
#ifndef OTA_ONCHIP
                            if(oadCheckDL(EXT_FLASH_PAGE(binaryAddrStart)) == ZSuccess)
#else
                            if(oadCheckDL(FLASH_PAGE(binaryAddrStart)) == ZSuccess)
#endif
                            {
                                oadPdState = OAD_IMAGE_COMPLETED_STATE;
                            }
                            else
                            {
                                flash_close();
                                return ZCL_STATUS_ABORT;
                            }
                        }
                    }

                    //As we copy the whole buffer, force the index to be the last byte
                    i = len;

                break;
            }
        }



        break;

      default:
        break;
    }

#if defined OTA_MMO_SIGN
    if ( !skipHash )
    {
      // Maintain a buffer of data to hash
      zclOTA_DataToHash[zclOTA_HashPos++] = pData[i];

      // When the buffer reaches OTA_MMO_HASH_SIZE, update the Hash
      if ( zclOTA_HashPos == OTA_MMO_HASH_SIZE )
      {
        OTA_CalculateMmoR3 ( &zclOTA_MmoHash, zclOTA_DataToHash, OTA_MMO_HASH_SIZE, FALSE );
        zclOTA_HashPos = 0;
      }
    }
#endif

    // Check if the download is complete
    if ( ++*zclOTA_FileOffset >= zclOTA_DownloadedImageSize )
    {
      *zclOTA_ImageUpgradeStatus = OTA_STATUS_COMPLETE;


#if defined OTA_MMO_SIGN
      // Complete the hash calcualtion
      OTA_CalculateMmoR3 ( &zclOTA_MmoHash, zclOTA_DataToHash, zclOTA_HashPos, TRUE );

      // Validate the hash
      if ( OTA_ValidateSignature ( zclOTA_MmoHash.hash, zclOTA_Certificate,
                                   zclOTA_SignatureData, zclOTA_SignerIEEE ) != ZSuccess )
        return ZCL_STATUS_INVALID_IMAGE;
#endif

      return ZSuccess;
    }
  }

  return ZSuccess;
}

/*********************************************************************
 * @fn      oadEraseExtFlashPages
 *
 * @brief   This function erases external flash pages
 *
 * @param   imgStartPage - image start page on external flash
 * @param   imgPageLen   - image length in pages
  *
 * @return  ZSuccess on successful erasure else
 *          ZFailure
 */
static uint8_t oadEraseExtFlashPages(uint8_t imgStartPage, uint8_t imgPageLen)
{
    uint8_t status = ZSuccess;

    for (uint8_t i = 0; i < imgPageLen; i++)
    {
       uint8_t flashStat = eraseFlashPg(imgStartPage);
        if(flashStat == FLASH_FAILURE)
        {
            // If we fail to pre-erase, then halt the OAD process
            status = ZFailure;
            break;
        }
        imgStartPage++;
    }
    return status;
}

/*********************************************************************
 * @fn      oadCheckDL
 *
 * @brief   Check validity of the downloaded image.
 *
 * @param   imagePage - Page at which the binary starts
 *
 * @return  TRUE or FALSE for image valid.
 */
static uint8_t oadCheckDL(uint8_t imagePage)
{
    uint32_t crcFromHdr;
    uint32_t crcCalculated;
    uint8_t crcStatus;
    uint8_t status = ZSuccess;

#ifdef OTA_ONCHIP
    imgHdr_t imgHdr;

    if(readFlashPg(imagePage, 0, (uint8_t *)(&imgHdr),
                   sizeof(imgHdr_t)) != FLASH_SUCCESS)
    {
        return (ZFailure);
    }
#else
    /* Zigbee OAD assumptions:
     * Factory New Metadata and binary image exist in external flash.
     * Zigbee OAD will always take the next slot available after FN header and binary
     */

    ExtImageInfo_t imgHdr;

    //Zigbee metadata header is always stored at the second page
    readFlashPg(EFL_FACT_IMG_META_PG + 1, 0, (uint8_t *)(&imgHdr),
                        sizeof(ExtImageInfo_t));
#endif

    crcStatus = imgHdr.fixedHdr.crcStat;
    crcFromHdr = imgHdr.fixedHdr.crc32;

    // If for some reason the header shows the CRC is invalid reject the image now
    if (crcStatus == CRC_INVALID)
    {
        return (ZFailure);
    }

    // Calculate CRC of downloaded image.
#ifdef OTA_ONCHIP
    // Force CRC32_calc to use relative address of the NVS region by using useExtFl as true
    crcCalculated = CRC32_calc(imagePage, INTFLASH_PAGE_SIZE, 0, imgHdr.fixedHdr.len, TRUE);
#else
    crcCalculated = CRC32_calc(imagePage, EFL_PAGE_SIZE, 0, imgHdr.fixedHdr.len, TRUE);
#endif

    if (crcCalculated == crcFromHdr)
    {
#ifdef OTA_ONCHIP
        crcStatus = CRC_VALID;
        // Write CRC status back to flash
        if(writeFlashPg(imagePage, CRC_STAT_OFFSET, &crcStatus,
                        sizeof(uint8_t)) != FLASH_SUCCESS)
        {
            return (ZFailure);
        }
#endif
        status = ZSuccess;
    }
    else
    {
        status = ZFailure;
    }

    return (status);
}

/******************************************************************************
 * @fn          zclOTA_getStatus
 *
 * @brief       Retrieves current ZCL OTA Status
 *
 * @param       none
 *
 * @return      ZCL OTA Status
 */
uint8_t zclOTA_getStatus ( void )
{
  return *zclOTA_ImageUpgradeStatus;
}

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
void zclOTA_setAttributes( const zclAttrRec_t *attrs, uint8_t numAttrs )
{
  uint8_t i;

  for ( i = 0; i < numAttrs; i++ )
  {
    if ( attrs[i].clusterID == ZCL_CLUSTER_ID_OTA )
    {
      switch(attrs[i].attr.attrId)
      {
        case ATTRID_OTA_UPGRADE_FILE_OFFSET:
        {
          zclOTA_FileOffset = attrs[i].attr.dataPtr;
        }
        break;
        case ATTRID_OTA_UPGRADE_DOWNLOADED_ZIG_BEE_STACK_VERSION:
        {
          zclOTA_DownloadedZigBeeStackVersion = attrs[i].attr.dataPtr;
        }
        break;
        case ATTRID_OTA_UPGRADE_IMAGE_UPGRADE_STATUS:
        {
          zclOTA_ImageUpgradeStatus = attrs[i].attr.dataPtr;
        }
        break;
        default:
        break;
      }
    }
  }
}

#endif // (defined OTA_CLIENT_STANDALONE) || (defined OTA_CLIENT_INTEGRATED
