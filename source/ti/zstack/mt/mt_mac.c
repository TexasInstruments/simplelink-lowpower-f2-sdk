/**************************************************************************************************
  Filename:       MT_MAC.c
  Revised:        $Date: 2014-10-09 12:38:39 -0700 (Thu, 09 Oct 2014) $
  Revision:       $Revision: 40541 $

  Description:    MonitorTest functions for the MAC layer.


  Copyright 2004-2014 Texas Instruments Incorporated.

  All rights reserved not granted herein.
  Limited License.

  Texas Instruments Incorporated grants a world-wide, royalty-free,
  non-exclusive license under copyrights and patents it now or hereafter
  owns or controls to make, have made, use, import, offer to sell and sell
  ("Utilize") this software subject to the terms herein. With respect to the
  foregoing patent license, such license is granted solely to the extent that
  any such patent is necessary to Utilize the software alone. The patent
  license shall not apply to any combinations which include this software,
  other than combinations with devices manufactured by or for TI ("TI
  Devices"). No hardware patent is licensed hereunder.

  Redistributions must preserve existing copyright notices and reproduce
  this license (including the above copyright notice and the disclaimer and
  (if applicable) source code license limitations below) in the documentation
  and/or other materials provided with the distribution.

  Redistribution and use in binary form, without modification, are permitted
  provided that the following conditions are met:

    * No reverse engineering, decompilation, or disassembly of this software
      is permitted with respect to any software provided in binary form.
    * Any redistribution and use are licensed by TI for use only with TI Devices.
    * Nothing shall obligate TI to provide you with source code for the software
      licensed and provided to you in object code.

  If software source code is provided to you, modification and redistribution
  of the source code are permitted provided that the following conditions are
  met:

    * Any redistribution and use of the source code, including any resulting
      derivative works, are licensed by TI for use only with TI Devices.
    * Any redistribution and use of any object code compiled from the source
      code and any resulting derivative works, are licensed by TI for use
      only with TI Devices.

  Neither the name of Texas Instruments Incorporated nor the names of its
  suppliers may be used to endorse or promote products derived from this
  software without specific prior written permission.

  DISCLAIMER.

  THIS SOFTWARE IS PROVIDED BY TI AND TI'S LICENSORS "AS IS" AND ANY EXPRESS
  OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
  IN NO EVENT SHALL TI AND TI'S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
  NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
  OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**************************************************************************************************/

#if defined (MT_MAC_FUNC) || defined (MT_MAC_CB_FUNC)  //MAC commands
/***************************************************************************************************
 * INCLUDES
 ***************************************************************************************************/
#include "zcomdef.h"
#include "rom_jt_154.h"
#include "mt.h"

#include "zmac.h"
#include "mt_mac.h"

#if !defined( WIN32 )

#endif

/* MAC radio */
#include "mac_radio_defs.h"

#ifdef FEATURE_MAC_SECURITY
  #include "mac_security_pib.h"
  #include "mac_security.h"
#endif

/***************************************************************************************************
 * MACROS
 ***************************************************************************************************/

/* The length in bytes of the pending address fields in the beacon */
#define MT_MAC_PEND_LEN(pendAddrSpec)   ((((pendAddrSpec) & 0x07) * 2) + \
                                        ((((pendAddrSpec) & 0x70) >> 4) * 8))

/* This matches the value used by nwk */
#define MT_MAC_ED_SCAN_MAXCHANNELS      MAC_CHAN_END + 1

/* Maximum size of pending address spec in beacon notify ind */
#define MT_MAC_PEND_LEN_MAX             32

/* Maximum size of the payload SDU in beacon notify ind */
#define MT_MAC_SDU_LEN_MAX              32

/* Maximum length of scan result in bytes */
#define MT_MAC_SCAN_RESULT_LEN_MAX      32

/* Maximum size of beacon payload */
#define MT_MAC_BEACON_PAYLOAD_MAX       16

/***************************************************************************************************
 * CONSTANTS
 ***************************************************************************************************/
#define DEFAULT_NSDU_HANDLE             0x00

#define MT_MAC_LEN_ASSOCIATE_IND        0x14          /* Associate Indication */
#define MT_MAC_LEN_ASSOCIATE_CNF        0x0E          /* Associate Confirmation */
#define MT_MAC_LEN_DISASSOCIATE_IND     0x14          /* Disassociate Indication */
#define MT_MAC_LEN_DISASSOCIATE_CNF     0x0c          /* Disassociate Confirmation */
#define MT_MAC_LEN_BEACON_NOTIFY_IND    0x63          /* Beacon Notification */
#define MT_MAC_LEN_ORPHAN_IND           0x13          /* Orphan Indication */
#define MT_MAC_LEN_SCAN_CNF             0x09          /* Scan Confirmation */
#define MT_MAC_LEN_SYNC_LOSS_IND        0x10          /* Sync Loss Indication */
#define MT_MAC_LEN_COMM_STATUS_IND      0x21          /* Comm Status Indication */
#define MT_MAC_LEN_DATA_CNF             0x0C          /* Data Confirmation */
#define MT_MAC_LEN_DATA_IND             0x2C          /* Data Indication */
#define MT_MAC_LEN_PURGE_CNF            0x02          /* Purge Confirmation */
#define MT_MAC_LEN_POLL_IND             0x0C          /* Poll Indication */

/***************************************************************************************************
 * GLOBAL VARIABLES
 ***************************************************************************************************/
#if defined ( MT_MAC_FUNC )
  uint16_t _macCallbackSub;
#endif

/* storage for MAC beacon payload */
static uint8_t mtMacBeaconPayload[MT_MAC_BEACON_PAYLOAD_MAX];

/***************************************************************************************************
 * LOCAL FUNCTIONS
 ***************************************************************************************************/
static void MT_MacSpi2Sec( ZMacSec_t *pSec, uint8_t *pSrc );
static void MT_MacSpi2Addr( zAddrType_t *pDst, uint8_t *pSrc );
static void MT_MacRevExtCpy( uint8_t *pDst, uint8_t *pSrc );

#if defined ( MT_MAC_CB_FUNC )
static void MT_MacAddr2Spi( uint8_t *pDst, zAddrType_t *pSrc );
#endif

void MT_MacResetReq(uint8_t *pBuf);
void MT_MacInit(uint8_t *pBuf);
void MT_MacStartReq(uint8_t *pBuf);
void MT_MacSyncReq(uint8_t *pBuf);
void MT_MacDataReq(uint8_t *pBuf);
void MT_MacAssociateReq(uint8_t *pBuf);
void MT_MacDisassociateReq(uint8_t *pBuf);
void MT_MacGetReq(uint8_t *pBuf);
void MT_MacSetReq(uint8_t *pBuf);
#ifdef FEATURE_MAC_SECURITY
void MT_MacSecurityGetReq(uint8_t *pBuf);
void MT_MacSecuritySetReq(uint8_t *pBuf);
void MT_MacUpdatePanId(uint8_t *pBuf);
#if !defined (MAC_TESTAPP)
void MT_MacAddDeviceReq(uint8_t *pBuf);
void MT_MacDeleteDeviceReq(uint8_t *pBuf);
void MT_MacDeleteAllDevicesReq(uint8_t *pBuf);
void MT_MacReadKeyWithIdReq(uint8_t *pBuf);
void MT_MacWriteKey(uint8_t *pBuf);
#endif /* MAC_TESTAPP */
#endif
void MT_MacScanReq(uint8_t * pBuf);
void MT_MacPollReq(uint8_t *pBuf);
void MT_MacPurgeReq(uint8_t *pBuf);
void MT_MacSetRxGainReq(uint8_t *pBuf);
void MT_MacAssociateRsp(uint8_t *pBuf);
void MT_MacOrphanRsp(uint8_t *pBuf);

/* Enhanced beacon request*/
void MT_MacEnhancedActiveScanReq(uint8_t * pBuf);
/***************************************************************************************************
 * @fn      MT_MacCommandProcessing
 *
 * @brief   Process all the MAC commands that are issued by test tool
 *
 * @param   pBuf - pointer to the msg buffer
 *
 *          | LEN  | CMD0  | CMD1  |  DATA  |
 *          |  1   |   1   |   1   |  0-255 |
 *
 * @return  void
 ***************************************************************************************************/
uint8_t MT_MacCommandProcessing (uint8_t *pBuf)
{
  uint8_t status = MT_RPC_SUCCESS;

  switch (pBuf[MT_RPC_POS_CMD1])
  {
    case MT_MAC_RESET_REQ:
      MT_MacResetReq(pBuf);
      break;

    case MT_MAC_INIT:
      MT_MacInit(pBuf);
      break;

    case MT_MAC_START_REQ:
      MT_MacStartReq(pBuf);
      break;

    case MT_MAC_SYNC_REQ:
      MT_MacSyncReq(pBuf);
      break;

    case MT_MAC_DATA_REQ:
      MT_MacDataReq(pBuf);
      break;

    case MT_MAC_ASSOCIATE_REQ:
      MT_MacAssociateReq(pBuf);
      break;

    case MT_MAC_DISASSOCIATE_REQ:
      MT_MacDisassociateReq(pBuf);
      break;

    case MT_MAC_GET_REQ:
      MT_MacGetReq(pBuf);
      break;

    case MT_MAC_SET_REQ:
      MT_MacSetReq(pBuf);
      break;

#ifdef FEATURE_MAC_SECURITY
    case MT_MAC_SECURITY_GET_REQ:
      MT_MacSecurityGetReq(pBuf);
      break;

    case MT_MAC_SECURITY_SET_REQ:
      MT_MacSecuritySetReq(pBuf);
      break;

    case MT_MAC_UPDATE_PAN_ID:
      MT_MacUpdatePanId(pBuf);
      break;

#if !defined (MAC_TESTAPP)
    case MT_MAC_ADD_DEVICE_REQ:
      MT_MacAddDeviceReq(pBuf);
      break;
    case MT_MAC_DELETE_DEVICE_REQ:
      MT_MacDeleteDeviceReq(pBuf);
      break;
    case MT_MAC_DELETE_ALL_DEVICES_REQ:
      MT_MacDeleteAllDevicesReq(pBuf);
      break;
    case MT_MAC_READ_KEY_WITH_ID_REQ:
      MT_MacReadKeyWithIdReq(pBuf);
      break;
    case MT_MAC_WRITE_KEY_WITH_ID_REQ:
      MT_MacWriteKey(pBuf);
      break;
#endif /* MAC_TESTAPP */
#endif /* FEATURE_MAC_SECURITY */

    case MT_MAC_GTS_REQ:
      /* Not supported */
      break;

    case MT_MAC_SCAN_REQ:
      MT_MacScanReq(pBuf);
      break;

    case MT_MAC_POLL_REQ:
      MT_MacPollReq(pBuf);
      break;

    case MT_MAC_PURGE_REQ:
      MT_MacPurgeReq(pBuf);
      break;

    case MT_MAC_SET_RX_GAIN_REQ:
      MT_MacSetRxGainReq(pBuf);
      break;

    case MT_MAC_ASSOCIATE_RSP:
      MT_MacAssociateRsp(pBuf);
      break;

    case MT_MAC_ORPHAN_RSP:
      MT_MacOrphanRsp(pBuf);
      break;

    case MT_MAC_ENHANCED_ACTIVE_SCAN_REQ:
      MT_MacEnhancedActiveScanReq(pBuf);
      break;

    default:
    status = MT_RPC_ERR_COMMAND_ID;
    break;
  }

  return status;
}

/***************************************************************************************************
 * @fn      MT_MacReset
 *
 * @brief   Process MAC Reset command that are issued by test tool
 *
 * @param   pBuf - Buffer contains the data
 *
 * @return  void
 ***************************************************************************************************/
void MT_MacResetReq(uint8_t *pBuf)
{
  uint8_t retValue, cmdId;

  /* Parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  retValue = ZMacReset(*pBuf);

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_MAC), cmdId, 1, &retValue );
}

/***************************************************************************************************
 * @fn      MT_MacInit
 *
 * @brief   Process Mac Init command that are issued by test tool
 *
 * @param   pBuf - Buffer contains the data
 *
 * @return  void
 ***************************************************************************************************/
void MT_MacInit(uint8_t *pBuf)
{
  uint8_t retValue, cmdId;

  /* Parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  retValue = ZMacInit();

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_MAC), cmdId, 1, &retValue );
}

/***************************************************************************************************
 * @fn      MT_MacStart
 *
 * @brief   Process MAC Start command that are issued by test tool
 *
 * @param   pBuf - Buffer contains the data
 *
 * @return  void
 ***************************************************************************************************/
void MT_MacStartReq(uint8_t *pBuf)
{
  uint8_t retValue, cmdId;
#ifdef RTR_NWK
  ZMacStartReq_t startReq;
#endif

  /* Parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

#ifdef RTR_NWK

    /* StartTime */
    startReq.StartTime = BUILD_UINT32 (pBuf[0], pBuf[1], pBuf[2], pBuf[3]);
    pBuf += 4;

    /* PanID */
    startReq.PANID = BUILD_UINT16( pBuf[0] , pBuf[1] );
    pBuf += 2;

    /* Fill in other fields sequentially incrementing the pointer*/

    startReq.LogicalChannel    =  *pBuf++;
    startReq.ChannelPage       =  *pBuf++;
    startReq.BeaconOrder       =  *pBuf++;
    startReq.SuperframeOrder   =  *pBuf++;
    startReq.PANCoordinator    =  *pBuf++;
    startReq.BatteryLifeExt    =  *pBuf++;
    startReq.CoordRealignment  =  *pBuf++;

    /* Realign Security Information */
    MT_MacSpi2Sec( &startReq.RealignSec, pBuf );
    pBuf += ZTEST_DEFAULT_SEC_LEN;

    /* Beacon Security Information */
    MT_MacSpi2Sec( &startReq.BeaconSec, pBuf );

    /* Call corresponding ZMAC function */
    retValue = ZMacStartReq( &startReq );

#else
   retValue = ZMacDenied;
#endif

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_MAC), cmdId, 1, &retValue );
}

/***************************************************************************************************
 * @fn      MT_MacSync
 *
 * @brief   Process MAC Sync command that are issued by test tool
 *
 * @param   pBuf - Buffer contains the data
 *
 * @return  void
 ***************************************************************************************************/
void MT_MacSyncReq(uint8_t *pBuf)
{
  uint8_t retValue, cmdId;
  ZMacSyncReq_t syncReq;

  /* Parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  /* LogicalChannel */
  syncReq.LogicalChannel = *pBuf++;

  /* ChannelPage */
  syncReq.ChannelPage = *pBuf++;

  /* TrackBeacon */
  syncReq.TrackBeacon    = *pBuf;

  /* Call corresponding ZMAC function */
  retValue = ZMacSyncReq( &syncReq );

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_MAC), cmdId, 1, &retValue );
}

/***************************************************************************************************
 * @fn      MT_MacDataReq
 *
 * @brief   Process MAC Data Request command that are issued by test tool
 *
 * @param   pBuf - Buffer contains the data
 *
 * @return  void
 ***************************************************************************************************/
void MT_MacDataReq(uint8_t *pBuf)
{
  uint8_t retValue, cmdId;
  ZMacDataReq_t dataReq;

  /* Parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

   /* Destination address mode */
  dataReq.DstAddr.addrMode = *pBuf++;

  /* Destination address */
  MT_MacSpi2Addr( &dataReq.DstAddr, pBuf);
  pBuf += Z_EXTADDR_LEN;

  /* Destination Pan ID */
  dataReq.DstPANId = BUILD_UINT16( pBuf[0] , pBuf[1] );
  pBuf += 2;

  /* Source address mode */
  dataReq.SrcAddrMode = *pBuf++;

  /* Handle */
  dataReq.Handle = *pBuf++;

  /* TxOptions */
  dataReq.TxOptions = *pBuf++;

  /* Channel */
  dataReq.Channel = *pBuf++;

  /* Power */
  dataReq.Power = *pBuf++;

  /* Data Security */
  MT_MacSpi2Sec( &dataReq.Sec, pBuf );
  pBuf += ZTEST_DEFAULT_SEC_LEN;

  /* Data length */
  dataReq.msduLength = *pBuf++;

  /* Data - Just pass the pointer to the structure */
  dataReq.msdu = pBuf;
  pBuf += dataReq.msduLength;

  /* Green Power (GP) Parameters */
  dataReq.GpOffset = *pBuf++;
  dataReq.GpDuration = *pBuf;

  /* Call corresponding ZMAC function */
  retValue = ZMacDataReq( &dataReq );

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_MAC), cmdId, 1, &retValue );
}

/***************************************************************************************************
 * @fn      MT_MacAssociateReq
 *
 * @brief   Process MAC Get Request command that are issued by test tool
 *
 * @param   pBuf - Buffer contains the data
 *
 * @return  void
 ***************************************************************************************************/
void MT_MacAssociateReq(uint8_t *pBuf)
{
  uint8_t retValue, cmdId;
  ZMacAssociateReq_t    assocReq;

  /* Parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  /* Logical Channel */
  assocReq.LogicalChannel = *pBuf++;

  /* Channel Page */
  assocReq.ChannelPage = *pBuf++;

  /* Address Mode */
  assocReq.CoordAddress.addrMode = *pBuf++;

  /* Coordinator Address, address mode must be set at this point */
  MT_MacSpi2Addr( &assocReq.CoordAddress, pBuf );
  pBuf += Z_EXTADDR_LEN;

  /* Coordinator PanID */
  assocReq.CoordPANId = BUILD_UINT16( pBuf[0] , pBuf[1] );
  pBuf += 2;

  /* Capability information */
  assocReq.CapabilityFlags = *pBuf++;

  /* Security Information */
  MT_MacSpi2Sec( &assocReq.Sec, pBuf );

  /* Call corresponding ZMAC function */
  retValue = ZMacAssociateReq( &assocReq );

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_MAC), cmdId, 1, &retValue );
}

/***************************************************************************************************
 * @fn      MT_MacDisassociateReq
 *
 * @brief   Process MAC Get Request command that are issued by test tool
 *
 * @param   pBuf - Buffer contains the data
 *
 * @return  void
 ***************************************************************************************************/
void MT_MacDisassociateReq(uint8_t *pBuf)
{
  uint8_t retValue, cmdId;
  ZMacDisassociateReq_t disassocReq;

  /* Parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  /* Device address mode */
  disassocReq.DeviceAddress.addrMode = *pBuf++;

  /* Device address - Device address mode have to be set to use this function*/
  MT_MacSpi2Addr( &disassocReq.DeviceAddress, pBuf);
  pBuf += Z_EXTADDR_LEN;

  /* Pan ID */
  disassocReq.DevicePanId = BUILD_UINT16( pBuf[0] , pBuf[1] );
  pBuf += 2;

  /* Disassociate reason */
  disassocReq.DisassociateReason = *pBuf++;

  /* TxIndirect */
  disassocReq.TxIndirect = *pBuf++;

  /* Security Information */
  MT_MacSpi2Sec( &disassocReq.Sec, pBuf );

  /* Call corresponding ZMAC function */
  retValue = ZMacDisassociateReq( &disassocReq );

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_MAC), cmdId, 1, &retValue );
}

/***************************************************************************************************
 * @fn      MT_MacGetReq
 *
 * @brief   Process MAC Get Request command that are issued by test tool
 *
 * @param   pBuf - Buffer contains the data
 *
 * @return  void
 ***************************************************************************************************/
void MT_MacGetReq(uint8_t *pBuf)
{
  uint8_t respLen, cmdId, attr;
  uint8_t *pRetBuf;

  /* Parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  /* Response length is 16 bytes + 1 byte status */
  respLen = ZTEST_DEFAULT_PARAM_LEN;

  /* Allocate */
  pRetBuf = OsalPort_malloc(respLen);

  /* Attribute to be read */
  attr = *pBuf;

  if (pRetBuf)
  {
    /* Zero everything */
    memset(pRetBuf, 0, respLen);
    /* Default to ZFailure */
    pRetBuf[0] = ZFailure;
    /* Read the pib value */
    pRetBuf[0] = ZMacGetReq(attr, &pRetBuf[1]);

    /* Build and send back the response */
    MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_MAC), cmdId, respLen, pRetBuf );

    /* Deallocate */
    OsalPort_free(pRetBuf);
  }
}

/***************************************************************************************************
 * @fn      MT_MacSetReq
 *
 * @brief   Process MAC Set Req command that are issued by test tool
 *
 * @param   pBuf - Buffer contains the data
 *
 * @return  void
 ***************************************************************************************************/
void MT_MacSetReq(uint8_t *pBuf)
{
  uint8_t retValue, cmdId, attr;

  /* Parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  /*
    In the data field of 'msg', the first byte is the attribute and remainder
    is the attribute value. So the pointer 'pBuf' points directly to the attribute.
    The value of the attribute is from the next byte position
  */
  attr = *pBuf;

  /* special case for beacon payload */
  if ( attr == ZMacBeaconMSDU )
  {
    OsalPort_memcpy( mtMacBeaconPayload, pBuf + 1, MT_MAC_BEACON_PAYLOAD_MAX );
    retValue = ZMacSetReq( (ZMacAttributes_t)attr ,  (byte *) &mtMacBeaconPayload );
  }
  else
  {
    retValue = ZMacSetReq( (ZMacAttributes_t)attr , pBuf + 1 );
  }

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_MAC), cmdId, 1, &retValue );
}

#ifdef FEATURE_MAC_SECURITY
/***************************************************************************************************
 * @fn      MT_MacSecurityGetReq
 *
 * @brief   Process MAC Security Get Request command that are issued by test tool
 *
 * @param   pBuf - Buffer contains the data
 *
 * @return  void
 ***************************************************************************************************/
void MT_MacSecurityGetReq(uint8_t *pBuf)
{
  uint8_t respLen, cmdId, attr;
  uint8_t *pRetBuf;

  /* Parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  /* Response length is 25 bytes + 2 bytes index + 1 byte status */
  respLen = ZTEST_DEFAULT_SEC_PARAM_LEN;

  /* Allocate */
  pRetBuf = OsalPort_malloc(respLen);

  /* Attribute to be read */
  attr = *pBuf++;

  if (pRetBuf)
  {
    /* Zero everything */
    memset(pRetBuf, 0, respLen);

    switch (attr)
    {
      case MAC_KEY_ID_LOOKUP_ENTRY:
      case MAC_KEY_DEVICE_ENTRY:
      case MAC_KEY_USAGE_ENTRY:
        /* These security PIBs have two parameters */
        pRetBuf[1] = *pBuf++;
        pRetBuf[2] = *pBuf;
        break;
      case MAC_KEY_ENTRY:
      case MAC_DEVICE_ENTRY:
      case MAC_SECURITY_LEVEL_ENTRY:
        /* These security PIBs have one parameter */
        pRetBuf[1] = *pBuf;
        break;
    }

    /* Other MAC Security PIB items. Read the pib value */
    pRetBuf[0] = ZMacSecurityGetReq(attr, &pRetBuf[1]);

    /* Build and send back the response */
    MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_MAC), cmdId, respLen, pRetBuf );

    /* Deallocate */
    OsalPort_free(pRetBuf);
  }
}

/***************************************************************************************************
 * @fn      MT_MacSecuritySetReq
 *
 * @brief   Process MAC Set Req command that are issued by test tool
 *
 * @param   pBuf - Buffer contains the data
 *
 * @return  void
 ***************************************************************************************************/
void MT_MacSecuritySetReq(uint8_t *pBuf)
{
  uint8_t cmdId, attr;
  uint8_t retValue = ZMAC_SUCCESS;

  /* Parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  /*
    In the data field of 'msg', the first byte is the attribute and remainder
    is the attribute value. So the pointer 'pBuf' points directly to the attribute.
    The value of the attribute is from the next byte position
  */
  attr = *pBuf++;
  if (attr == MAC_KEY_TABLE || attr == MAC_DEVICE_TABLE || attr == MAC_SECURITY_LEVEL_TABLE)
  {
    pBuf = NULL;
  }
  retValue = ZMacSecuritySetReq( (ZMacAttributes_t)attr , pBuf );

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_MAC), cmdId, 1, &retValue );
}

/***************************************************************************************************
 * @fn      MT_MacUpdatePanId
 *
 * @brief   Process MAC Updare Pan ID command that are issued by test tool
 *
 * @param   pBuf - Buffer contains the data
 *
 * @return  void
 ***************************************************************************************************/
void MT_MacUpdatePanId(uint8_t *pBuf)
{
  uint16_t panId;
  uint8_t cmdId;
  uint8_t retValue = ZMAC_SUCCESS;


  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;
  panId = (unsigned short) *pBuf | (((unsigned short) pBuf[1])<<8);

  MAP_macUpdatePanId(panId);

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_MAC), cmdId, 1, &retValue );
}


#if !defined (MAC_TESTAPP)
/***************************************************************************************************
 * @fn      MT_MacAddDeviceReq
 *
 * @brief   Process MAC Add Device command that are issued by test tool
 *
 * @param   pBuf - Buffer contains the data
 *
 * @return  void
 ***************************************************************************************************/
void MT_MacAddDeviceReq(uint8_t *pBuf)
{
  uint8_t retValue, cmdId;
  uint16_t panId, shortAddr;
  uint32_t frameCounter;
  extern uint8_t MAP_macWrapperAddDevice(unsigned short panId, unsigned short shortAddr,
                                   const unsigned char *extAddr, unsigned char exempt,
                                   unsigned char keyIdLookupDataSize,
                                   const unsigned char *keyIdLookupData,
                                   unsigned long frameCounter,
                                   unsigned char uniqueDevice,
                                   unsigned char duplicateDevFlag);

  /* Parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;
  panId = (unsigned short) *pBuf | (((unsigned short) pBuf[1])<<8);
  shortAddr = (unsigned short) pBuf[2] | (((unsigned short) pBuf[3])<<8);
  frameCounter = (uint32_t) pBuf[23] | (((uint32_t) pBuf[24])<<8) |
    (((uint32_t) pBuf[25])<<16) | (((uint32_t) pBuf[26])<<24);

  retValue = MAP_macWrapperAddDevice(panId, shortAddr, &pBuf[4], pBuf[12], pBuf[13], &pBuf[14],
                                 frameCounter, pBuf[27], pBuf[28]);

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_MAC), cmdId, 1, &retValue );
}

/***************************************************************************************************
 * @fn      MT_MacDeleteDeviceReq
 *
 * @brief   Process MAC Delete Device command that are issued by test tool
 *
 * @param   pBuf - Buffer contains the data
 *
 * @return  void
 ***************************************************************************************************/
void MT_MacDeleteDeviceReq(uint8_t *pBuf)
{
  uint8_t retValue, cmdId;
  extern uint8_t MAP_macWrapperDeleteDevice(const uint8_t *extAddr);

  /* Parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  retValue = MAP_macWrapperDeleteDevice(pBuf);

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_MAC), cmdId, 1, &retValue );
}

/***************************************************************************************************
 * @fn      MT_MacDeleteAllDevicesReq
 *
 * @brief   Process MAC Delete All Devices command that are issued by test tool
 *
 * @param   pBuf - Buffer contains the data
 *
 * @return  void
 ***************************************************************************************************/
void MT_MacDeleteAllDevicesReq(uint8_t *pBuf)
{
  uint8_t retValue, cmdId;
  extern uint8_t MAP_macWrapperDeleteAllDevices(void);

  /* Parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  retValue = MAP_macWrapperDeleteAllDevices();

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_MAC), cmdId, 1, &retValue );
}

/***************************************************************************************************
 * @fn      MT_MacReadKeyWithIdReq
 *
 * @brief   Process MAC Read Key With Id command that are issued by test tool
 *
 * @param   pBuf - Buffer contains the data
 *
 * @return  void
 ***************************************************************************************************/
void MT_MacReadKeyWithIdReq(uint8_t *pBuf)
{
  uint8_t retValue[1 + MAC_KEY_MAX_LEN + 4], cmdId;
  extern uint8_t MAP_macWrapperGetDefaultSourceKey(uint8_t keyid, uint32_t *pFrameCounter);

  /* Parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  retValue[0] = MAP_macWrapperGetDefaultSourceKey(*pBuf,
                                              (uint32_t *) &retValue[1 + MAC_KEY_MAX_LEN]);

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_MAC), cmdId,
                               sizeof(retValue), retValue );
}

/***************************************************************************************************
 * @fn      MT_MacWriteKey
 *
 * @brief   Writes the key into the MAC PIB.
 *
 * @param   pBuf - Buffer contains the data
 *
 * @return  void
 ***************************************************************************************************/
void MT_MacWriteKey(uint8_t *pBuf)
{
  int8_t retValue;
  uint8_t cmdId;
  uint32_t frameCntr;
  extern unsigned char MAP_macWrapperAddKeyInitFCtr( unsigned char *pKey,
                                                 uint32_t frameCounter,
                                                 unsigned char replaceKeyIndex,
                                                 unsigned char dupDevFlag,
                                                 uint8_t* lookupList);
  /* Parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  OsalPort_memcpy( &frameCntr, &pBuf[16], sizeof(frameCntr));
  retValue = MAP_macWrapperAddKeyInitFCtr(pBuf, frameCntr, pBuf[20], pBuf[21], &pBuf[22]);

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_MAC), cmdId, 1, (unsigned char*)&retValue );
}


#endif /* MAC_TESTAPP */
#endif /* FEATURE_MAC_SECURITY */

/***************************************************************************************************
 * @fn      MT_MacScanReq
 *
 * @brief   Process MAC Scan Req command that are issued by test tool
 *
 * @param   pBuf - Buffer contains the data
 *
 * @return  void
 ***************************************************************************************************/
void MT_MacScanReq(uint8_t * pBuf)
{
  uint8_t retValue, cmdId;
  ZMacScanReq_t scanReq;

  memset( &scanReq, 0, sizeof( ZMacScanReq_t ) );

  /* Parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  /* ScanChannels is the 32-bit channel list */
  scanReq.ScanChannels = BUILD_UINT32 (pBuf[0], pBuf[1], pBuf[2], pBuf[3]);
  pBuf += 4;

  /* Fill in fields sequentially incrementing the pointer */
  scanReq.ScanType = *pBuf++;

  /* ScanDuration */
  scanReq.ScanDuration = *pBuf++;

  /* Channel Page */
  scanReq.ChannelPage = *pBuf++;

  /* MaxResults */
  scanReq.MaxResults = *pBuf++;

  /* Security Information */
  MT_MacSpi2Sec( &scanReq.Sec, pBuf );

  /* Call corresponding ZMAC function */
  retValue =  ZMacScanReq( &scanReq );

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_MAC), cmdId, 1, &retValue );
}

/***************************************************************************************************
 * @fn      MT_MacPollReq
 *
 * @brief   Process MAC Poll Req command that are issued by test tool
 *
 * @param   pBuf - Buffer contains the data
 *
 * @return  void
 ***************************************************************************************************/
void MT_MacPollReq(uint8_t *pBuf)
{
  uint8_t retValue, cmdId;
  ZMacPollReq_t pollReq;

  /* Parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  /* Coordinator address mode */
  pollReq.CoordAddress.addrMode = *pBuf++;

  /* Coordinator address - Device address mode have to be set to use this function */
  MT_MacSpi2Addr( &pollReq.CoordAddress, pBuf);
  pBuf += Z_EXTADDR_LEN;

  /* Coordinator Pan ID */
  pollReq.CoordPanId = BUILD_UINT16(pBuf[0] , pBuf[1]);
  pBuf += 2;

  /* Security Information */
  MT_MacSpi2Sec(&pollReq.Sec, pBuf);

  /* Call corresponding ZMAC function */
  retValue = ZMacPollReq( &pollReq );

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_MAC), cmdId, 1, &retValue );
}

/***************************************************************************************************
 * @fn      MT_MacPurgeReq
 *
 * @brief   Process MAC Purge Req command that are issued by test tool
 *
 * @param   pBuf - Buffer contains the data
 *
 * @return  void
 ***************************************************************************************************/
void MT_MacPurgeReq(uint8_t *pBuf)
{
  uint8_t cmdId;
  uint8_t retValue = 1;

  /* Parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  /* First and only byte - MsduHandle */
  retValue = ZMacPurgeReq (*pBuf);

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_MAC), cmdId, 1, &retValue );
}

/***************************************************************************************************
 * @fn      MT_MacSetRxGainReq
 *
 * @brief   Process MAC Rx Gain Req command that are issued by test tool
 *
 * @param   pBuf - Buffer contains the data
 *
 * @return  void
 ***************************************************************************************************/
void MT_MacSetRxGainReq(uint8_t *pBuf)
{
  uint8_t retValue, cmdId;

  /* Parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

#ifdef HAL_PA_LNA
  /* Toggle PA/LNA mode */
  if (*pBuf)
  {
    HAL_PA_LNA_RX_HGM();
  }
  else
  {
    HAL_PA_LNA_RX_LGM();
  }
  retValue = ZSuccess;
#else
  retValue = ZFailure;
#endif


  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_MAC), cmdId, 1, &retValue );
}

/***************************************************************************************************
 * @fn          MT_MacAssociateRsp
 *
 * @brief       Process MAC Associate Rsp command that are issued by test tool
 *
 * @param       pBuf - Buffer contains the data
 *
 * @return      void
 ***************************************************************************************************/
void MT_MacAssociateRsp(uint8_t *pBuf)
{
  uint8_t retValue, cmdId;

  /* Parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

#ifdef RTR_NWK
    ZMacAssociateRsp_t assocRsp;
    /* The address of the device requesting association */
    osal_cpyExtAddr(assocRsp.DeviceAddress, pBuf);
    pBuf += Z_EXTADDR_LEN;

    /* The short address allocated to the (associated) device */
    assocRsp.AssocShortAddress = BUILD_UINT16(pBuf[0],pBuf[1]);
    pBuf += 2;

    /* Status of the association */
    assocRsp.Status = *pBuf++;

    /* Security Information */
    MT_MacSpi2Sec( &assocRsp.Sec, pBuf );

    /* Call corresponding ZMAC function */
    retValue = ZMacAssociateRsp( &assocRsp );
#else
  retValue = ZMacDenied;
#endif

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_MAC), cmdId, 1, &retValue);
}

/***************************************************************************************************
 * @fn      MT_MacOrphanRsp
 *
 * @brief   Process MAC Orphan Response command that are issued by test tool
 *
 * @param   pBuf - Buffer contains the data
 *
 * @return  void
 ***************************************************************************************************/
void MT_MacOrphanRsp(uint8_t *pBuf)
{
  uint8_t retValue, cmdId;
  ZMacOrphanRsp_t orphanRsp;

  /* Parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  /* Extended address of the device sending the notification */
  MT_MacRevExtCpy( orphanRsp.OrphanAddress, pBuf );
  pBuf += Z_EXTADDR_LEN;

  /* Short address of the orphan device */
  orphanRsp.ShortAddress = BUILD_UINT16( pBuf[0] , pBuf[1] );
  pBuf += 2;

  /* Associated member */
  orphanRsp.AssociatedMember = *pBuf++;

  /* Security Information */
  MT_MacSpi2Sec( &orphanRsp.Sec, pBuf );

  /* Call corresponding ZMAC function */
  retValue = ZMacOrphanRsp( &orphanRsp );

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_MAC), cmdId, 1, &retValue);
}

/***************************************************************************************************
 * @fn      MT_MacEnhancedActiveScanReq
 *
 * @brief   Process MAC Scan Req command that are issued by test tool
 *
 * @param   pBuf - Buffer contains the data
 *
 * @return  void
 ***************************************************************************************************/
void MT_MacEnhancedActiveScanReq(uint8_t * pBuf)
{
  uint8_t retValue, cmdId;//, idx;
  ZMacScanReq_t scanReq;

  /* Parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  /* ScanChannels is the 32-bit channel list */
  scanReq.ScanChannels = BUILD_UINT32 (pBuf[0], pBuf[1], pBuf[2], pBuf[3]);
  pBuf += 4;

  /* Fill in fields sequentially incrementing the pointer */
  scanReq.ScanType = *pBuf++;

  /* ScanDuration */
  scanReq.ScanDuration = *pBuf++;

  /* Channel Page */
  scanReq.ChannelPage = *pBuf++;

  /* MaxResults */
  scanReq.MaxResults = *pBuf++;

  /* Security Information */
  MT_MacSpi2Sec( &scanReq.Sec, pBuf );

  pBuf+=sizeof (ZMacSec_t);

  /* Adding fields for enhanced active scan request */
  scanReq.PermitJoining = *pBuf++;
  scanReq.LinkQuality = *pBuf++;
  scanReq.PercentFilter = *pBuf++;

  /* Call corresponding ZMAC function */
  retValue =  ZMacEnhancedActiveScanReq( &scanReq );

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_MAC), cmdId, 1, &retValue );
}

#if defined ( MT_MAC_FUNC )
#if defined ( MT_MAC_CB_FUNC )

/***************************************************************************************************
 * @fn          nwk_MTCallbackSubNwkSyncLossInd
 *
 * @brief       Process the callback subscription for nwk_sync_loss_ind
 *
 * @param       byte LossReason
 *
 * @return      None
 ***************************************************************************************************/
void nwk_MTCallbackSubNwkSyncLossInd( ZMacSyncLossInd_t *param )
{
  uint8_t respLen;
  uint8_t *pRetBuf, *tp;

  respLen = MT_MAC_LEN_SYNC_LOSS_IND;

  pRetBuf = OsalPort_malloc (respLen);

  if ( pRetBuf )
  {
    tp = pRetBuf;

    /*  Status - loss reason */
    *tp++ = param->hdr.Status;

    /* Pan Id */
    *tp++ = LO_UINT16( param->PANId );
    *tp++ = HI_UINT16( param->PANId );

    /* Logical Channel */
    *tp++ = param->LogicalChannel;

    /* Channel Page */
    *tp++ = param->ChannelPage;

    /* Security */
    MT_MacSpi2Sec ((ZMacSec_t *)tp, (uint8_t *)&param->Sec);

    /* Build and send back the response */
    MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_AREQ | (uint8_t)MT_RPC_SYS_MAC), MT_MAC_SYNC_LOSS_IND, respLen, pRetBuf);

    /* Deallocate */
    OsalPort_free(pRetBuf);
  }
}

/***************************************************************************************************
 * @fn          nwk_MTCallbackSubNwkAssociateInd
 *
 * @brief       Process the callback subscription for nwk_associate_ind
 *
 * @param       pointer of type macnwk_associate_ind_t
 *
 * @return      None
 ***************************************************************************************************/
void nwk_MTCallbackSubNwkAssociateInd( ZMacAssociateInd_t *param )
{
  uint8_t respLen;
  uint8_t *pRetBuf, *tp;

  respLen = MT_MAC_LEN_ASSOCIATE_IND;

  pRetBuf = OsalPort_malloc (respLen);

  if ( pRetBuf )
  {
    tp = pRetBuf;

    /* Extended address */
    MT_MacAddr2Spi(pRetBuf, (zAddrType_t*)param->DeviceAddress);
    tp += Z_EXTADDR_LEN;

    /* Capability Information */
    *tp++ = param->CapabilityFlags;

    /* Security */
    MT_MacSpi2Sec ((ZMacSec_t *)tp, (uint8_t *)&param->Sec);

    /* Build and send back the response */
    MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_AREQ | (uint8_t)MT_RPC_SYS_MAC), MT_MAC_ASSOCIATE_IND, respLen, pRetBuf);

    /* Deallocate */
    OsalPort_free(pRetBuf);
  }
}

/***************************************************************************************************
 * @fn          nwk_MTCallbackSubNwkAssociateCnf
 *
 * @brief       Process the callback subscription for nwk_associate_cnf
 *
 * @param       pointer of type macnwk_associate_cnf_t
 *
 * @return      SUCCESS if message sent succesfully , else N_FAIL
 ***************************************************************************************************/
void nwk_MTCallbackSubNwkAssociateCnf( ZMacAssociateCnf_t *param )
{
  uint8_t respLen;
  uint8_t *pRetBuf, *tp;

  respLen = MT_MAC_LEN_ASSOCIATE_CNF;

  pRetBuf = OsalPort_malloc (respLen);

  if ( pRetBuf )
  {
    tp = pRetBuf;

    /* Status */
    *tp++ = param->hdr.Status;

    /* Short address */
    *tp++ = LO_UINT16( param->AssocShortAddress );
    *tp++ = HI_UINT16( param->AssocShortAddress );

    /* Security */
    MT_MacSpi2Sec ((ZMacSec_t *)tp, (uint8_t *)&param->Sec);

    /* Build and send back the response */
    MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_AREQ | (uint8_t)MT_RPC_SYS_MAC), MT_MAC_ASSOCIATE_CNF, respLen, pRetBuf);

    /* Deallocate */
    OsalPort_free(pRetBuf);
  }
}

/***************************************************************************************************
 * @fn          nwk_MTCallbackSubNwkBeaconNotifyInd
 *
 * @brief       Process the callback subscription for
 *              beacon_notify_ind.
 *
 * @param       pointer to ZMacBeaconNotifyInd_t
 *
 * @return      None
 ***************************************************************************************************/
void nwk_MTCallbackSubNwkBeaconNotifyInd ( ZMacBeaconNotifyInd_t *param )
{
  uint8_t respLen;
  uint8_t *pRetBuf, *tp;

  respLen = MT_MAC_LEN_BEACON_NOTIFY_IND;

  pRetBuf = OsalPort_malloc (respLen);

  if ( pRetBuf )
  {
    tp = pRetBuf;

    /* BSN */
    *tp++ = param->BSN;

    /* Timestamp */
    *tp++ = BREAK_UINT32( param->pPanDesc->TimeStamp, 0 );
    *tp++ = BREAK_UINT32( param->pPanDesc->TimeStamp, 1 );
    *tp++ = BREAK_UINT32( param->pPanDesc->TimeStamp, 2 );
    *tp++ = BREAK_UINT32( param->pPanDesc->TimeStamp, 3 );

    /* Coordinator address mode */
    *tp++ = param->pPanDesc->CoordAddress.addrMode;

    /* Coordinator address */
    MT_MacAddr2Spi( tp, &param->pPanDesc->CoordAddress );
    tp += Z_EXTADDR_LEN;

    /* PAN ID */
    *tp++ = LO_UINT16( param->pPanDesc->CoordPANId );
    *tp++ = HI_UINT16( param->pPanDesc->CoordPANId );

    /* Superframe spec */
    *tp++ = LO_UINT16( param->pPanDesc->SuperframeSpec );
    *tp++ = HI_UINT16( param->pPanDesc->SuperframeSpec );

    /* LogicalChannel */
    *tp++ = param->pPanDesc->LogicalChannel;

    /* GTSPermit */
    *tp++ = param->pPanDesc->GTSPermit;

    /* LinkQuality */
    *tp++ = param->pPanDesc->LinkQuality;

    /* SecurityFailure */
    *tp++ = param->pPanDesc->SecurityFailure;

    /* Security */
    MT_MacSpi2Sec ((ZMacSec_t *)tp, (uint8_t *)&param->pPanDesc->Sec);
    tp += ZTEST_DEFAULT_SEC_LEN;

    /* PendingAddrSpec */
    *tp++ = param->PendAddrSpec;

    /* AddrList */
    memset( tp, 0, MT_MAC_PEND_LEN_MAX );
    OsalPort_memcpy( tp, param->AddrList, MIN(MT_MAC_PEND_LEN_MAX, MT_MAC_PEND_LEN(param->PendAddrSpec)) );
    tp += MT_MAC_PEND_LEN_MAX;

    /* SDULength */
    *tp++ = param->sduLength;

    /* SDU */
    memset( tp, 0, MT_MAC_SDU_LEN_MAX );
    OsalPort_memcpy( tp, param->sdu, MIN(MT_MAC_SDU_LEN_MAX, param->sduLength) );

    /* Build and send back the response */
    MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_AREQ | (uint8_t)MT_RPC_SYS_MAC), MT_MAC_BEACON_NOTIFY_IND, respLen, pRetBuf);

    /* Deallocate */
    OsalPort_free(pRetBuf);

  }
}

/***************************************************************************************************
 * @fn          nwk_MTCallbackSubNwkDataCnf
 *
 * @brief       Process the callback subscription for nwk_data_cnf
 *
 * @param       pointer of type macnwk_data_cnf_t
 *
 * @return      None
 ***************************************************************************************************/
void nwk_MTCallbackSubNwkDataCnf( ZMacDataCnf_t *param )
{
  uint8_t *pRetBuf, *tp;

  /* Allocate */
  pRetBuf = OsalPort_malloc(MT_MAC_LEN_DATA_CNF);

  if (pRetBuf)
  {
    tp = pRetBuf;

    /* Status */
    *tp++ = param->hdr.Status;

    /* Handle */
    *tp++ = param->msduHandle;

    /* Timestamp */
    *tp++ = BREAK_UINT32( param->Timestamp, 0 );
    *tp++ = BREAK_UINT32( param->Timestamp, 1 );
    *tp++ = BREAK_UINT32( param->Timestamp, 2 );
    *tp++ = BREAK_UINT32( param->Timestamp, 3 );

    /* Timestamp2 */
    *tp++ = LO_UINT16( param->Timestamp2);
    *tp++ = HI_UINT16( param->Timestamp2);

    /* retries */
    *tp++ = param->retries;

    /* mpduLinkQuality */
    *tp++ = param->mpduLinkQuality;

    /* correlation */
    *tp++ = param->correlation;

    /* rssi */
    *tp = param->rssi;

    /* Build and send back the response */
    MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_AREQ | (uint8_t)MT_RPC_SYS_MAC), MT_MAC_DATA_CNF, MT_MAC_LEN_DATA_CNF, pRetBuf);

    /* Deallocate */
    OsalPort_free(pRetBuf);
  }
}

/***************************************************************************************************
 * @fn          nwk_MTCallbackSubNwkDataInd
 *
 * @brief       Process the callback subscription for nwk_data_ind
 *
 * @param       pointer of type macnwk_data_ind_t
 *
 * @return      None
 ***************************************************************************************************/
void nwk_MTCallbackSubNwkDataInd( ZMacDataInd_t *param )
{
  uint8_t respLen, tempLen;
  uint8_t *pRetBuf, *tp;

  /* Packet length is 44 + 117 data */
  respLen = MT_MAC_LEN_DATA_IND + ZTEST_DEFAULT_DATA_LEN;

  pRetBuf = OsalPort_malloc (respLen);

  if ( pRetBuf )
  {
    tp = pRetBuf;

    /* Src address mode */
    *tp++ = param->SrcAddr.addrMode;

    if (param->SrcAddr.addrMode != SADDR_MODE_NONE)
    {
      /* Src Address */
      MT_MacAddr2Spi( tp, &param->SrcAddr );
    }
    else
    {
      /* No address */
      uint8_t i;
      for ( i = 0; i< Z_EXTADDR_LEN; i++ )
      {
        tp[i] = 0x00;
      }
    }
    tp += Z_EXTADDR_LEN;

    /* Dst address mode */
    *tp++ = param->DstAddr.addrMode;

    /* Dst address */
    MT_MacAddr2Spi( tp, &param->DstAddr );
    tp += Z_EXTADDR_LEN;

       /* Timestamp */
    *tp++ = BREAK_UINT32( param->Timestamp, 0 );
    *tp++ = BREAK_UINT32( param->Timestamp, 1 );
    *tp++ = BREAK_UINT32( param->Timestamp, 2 );
    *tp++ = BREAK_UINT32( param->Timestamp, 3 );

    /* Timestamp2 */
    *tp++ = LO_UINT16( param->Timestamp2);
    *tp++ = HI_UINT16( param->Timestamp2);

    /* Src Pan Id */
    *tp++ = LO_UINT16( param->SrcPANId );
    *tp++ = HI_UINT16( param->SrcPANId );

    /* Dst Pan Id */
    *tp++ = LO_UINT16( param->DstPANId );
    *tp++ = HI_UINT16( param->DstPANId );

    /* mpdu Link Quality */
    *tp++ = param->mpduLinkQuality;

    /* LQI */
    *tp++ = param->Correlation;

    /* RSSI */
    *tp++ = param->Rssi;

    /* DSN */
    *tp++ = param->Dsn;

    /* Security */
    MT_MacSpi2Sec ((ZMacSec_t *)tp, (uint8_t *)&param->Sec);
    tp += ZTEST_DEFAULT_SEC_LEN;

    /* Determine the length of the actual data */
    if ( param->msduLength < ZTEST_DEFAULT_DATA_LEN )
      tempLen = param->msduLength;
    else
      tempLen = ZTEST_DEFAULT_DATA_LEN;

    /* Length */
    *tp++ = tempLen;

#ifdef MT_FIXED_LENGTH_DATA_INDICATION
    /* Copy the data according to Len, the rest are zeroed out */
    memset( tp, 0, ZTEST_DEFAULT_DATA_LEN );
#else
    /* Override respLen with the actual packet length */
    respLen = (uint8_t)(tp - pRetBuf) + tempLen;
#endif /* MT_FIXED_LENGTH_DATA_INDICATION */

    /* Copy the data according to Len */
    OsalPort_memcpy( tp, param->msdu, tempLen );

    /* Build and send back the response */
    MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_AREQ | (uint8_t)MT_RPC_SYS_MAC), MT_MAC_DATA_IND, respLen, pRetBuf);

    /* Deallocate */
    OsalPort_free(pRetBuf);
  }
}

/***************************************************************************************************
 * @fn          nwk_MTCallbackSubNwkDisassociateInd
 *
 * @brief       Process the callback subscription for nwk_disassociate_ind
 *
 * @param       pointer of type macnwk_disassociate_ind_t
 *
 * @return      None
 ***************************************************************************************************/
void nwk_MTCallbackSubNwkDisassociateInd( ZMacDisassociateInd_t *param )
{
  uint8_t respLen;
  uint8_t *pRetBuf, *tp;

  respLen = MT_MAC_LEN_DISASSOCIATE_IND;

  pRetBuf = OsalPort_malloc (respLen);

  if ( pRetBuf )
  {
    tp = pRetBuf;

    /* Extended address */
    MT_MacRevExtCpy( pRetBuf, param->DeviceAddress );
    tp += Z_EXTADDR_LEN;

    /* Disassociate Reason */
    *tp++ = param->DisassociateReason;

    /* Security */
    MT_MacSpi2Sec ((ZMacSec_t *)tp, (uint8_t *)&param->Sec);

    /* Build and send back the response */
    MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_AREQ | (uint8_t)MT_RPC_SYS_MAC), MT_MAC_DISASSOCIATE_IND, respLen, pRetBuf);

    /* Deallocate */
    OsalPort_free(pRetBuf);
  }
}

/***************************************************************************************************
 * @fn          nwk_MTCallbackSubNwkDisassociateCnf
 *
 * @brief       Process the callback subscription for nwk_disassociate_cnf
 *
 * @param       param
 *
 * @return      None
 ***************************************************************************************************/
void nwk_MTCallbackSubNwkDisassociateCnf( ZMacDisassociateCnf_t *param )
{
  uint8_t respLen;
  uint8_t *pRetBuf, *tp;

  respLen = MT_MAC_LEN_DISASSOCIATE_CNF;

  pRetBuf = OsalPort_malloc (respLen);

  if ( pRetBuf )
  {
    tp = pRetBuf;

    /* Status */
    *tp++ = param->hdr.Status;

    /* DeviceAddress */
    *tp++ = param->DeviceAddress.addrMode;

    /* Copy Address */
    MT_MacAddr2Spi( tp, &param->DeviceAddress );
    tp += Z_EXTADDR_LEN;

    /* Pan ID */
    *tp++ = LO_UINT16( param->panID );
    *tp = HI_UINT16( param->panID );

    /* Build and send back the response */
    MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_AREQ | (uint8_t)MT_RPC_SYS_MAC), MT_MAC_DISASSOCIATE_CNF, respLen, pRetBuf);

    /* Deallocate */
    OsalPort_free(pRetBuf);
  }
}

/***************************************************************************************************
 * @fn          nwk_MTCallbackSubNwkPollInd
 *
 * @brief       Process the callback subscription for nwk_poll_ind
 *
 * @param       pointer of type macnwk_poll_ind_t
 *
 * @return      SUCCESS if message sent succesfully , else N_FAIL
 ***************************************************************************************************/
void nwk_MTCallbackSubNwkPollInd( ZMacPollInd_t *param )
{
  uint8_t respLen;
  uint8_t *pRetBuf, *tp;

  respLen = MT_MAC_LEN_POLL_IND;

  pRetBuf = OsalPort_malloc (respLen);

  if ( pRetBuf )
  {
    tp = pRetBuf;

    /* src address */
    *tp++ = param->srcAddr.addrMode;
    MT_MacRevExtCpy( tp, param->srcAddr.addr.extAddr );
    tp += Z_EXTADDR_LEN;

    *tp++ = LO_UINT16( param->srcPanId );
    *tp++ = HI_UINT16( param->srcPanId );

    *tp = param->noRsp;

     /* Build and send back the response */
    MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_AREQ | (uint8_t)MT_RPC_SYS_MAC), MT_MAC_POLL_IND, respLen, pRetBuf);

    /* Deallocate */
    OsalPort_free(pRetBuf);
  }
}

/***************************************************************************************************
 * @fn          nwk_MTCallbackSubNwkOrphanInd
 *
 * @brief       Process the callback subscription for nwk_orphan_ind
 *
 * @param       pointer of type macnwk_orphan_ind_t
 *
 * @return      SUCCESS if message sent succesfully , else N_FAIL
 ***************************************************************************************************/
void nwk_MTCallbackSubNwkOrphanInd( ZMacOrphanInd_t *param )
{
  uint8_t respLen;
  uint8_t *pRetBuf, *tp;

  respLen = MT_MAC_LEN_ORPHAN_IND;

  pRetBuf = OsalPort_malloc (respLen);

  if ( pRetBuf )
  {
    tp = pRetBuf;

    /* Extended address */
    MT_MacRevExtCpy( tp, param->OrphanAddress );
    tp += Z_EXTADDR_LEN;

    /* Security */
    MT_MacSpi2Sec ((ZMacSec_t *)tp, (uint8_t *)&param->Sec);

    /* Build and send back the response */
    MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_AREQ | (uint8_t)MT_RPC_SYS_MAC), MT_MAC_ORPHAN_IND, respLen, pRetBuf);

    /* Deallocate */
    OsalPort_free(pRetBuf);
  }
}

/***************************************************************************************************
 * @fn          nwk_MTCallbackSubNwkPollCnf
 *
 * @brief       Process the callback subscription for nwk_poll_cnf
 *
 * @param       byte Status
 *
 * @return      None
 ***************************************************************************************************/
void nwk_MTCallbackSubNwkPollCnf( byte Status )
{
  uint8_t retValue;

  /*The only data byte is Status */
  retValue = Status;

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_AREQ | (uint8_t)MT_RPC_SYS_MAC), MT_MAC_POLL_CNF, 1, &retValue);
}

/***************************************************************************************************
 * @fn          nwk_MTCallbackSubNwkScanCnf
 *
 * @brief       Process the callback subscription for nwk_scan_cnf
 *
 * @param       pointer of type macnwk_scan_cnf_t
 *
 * @return      SUCCESS if message sent succesfully , else N_FAIL
 ***************************************************************************************************/
void nwk_MTCallbackSubNwkScanCnf( ZMacScanCnf_t *param )
{
  uint8_t respLen, resultLen;
  uint8_t *pRetBuf, *tp;

  /* Depends on the type of scan, calculate the required length */
  if ( param->ScanType == ZMAC_ED_SCAN )
    resultLen = MT_MAC_ED_SCAN_MAXCHANNELS;
  else if ( param->ScanType == ZMAC_ACTIVE_SCAN )
    resultLen = (param->ResultListSize * sizeof( ZMacPanDesc_t ));
  else if ( param->ScanType == ZMAC_PASSIVE_SCAN )
    resultLen = (param->ResultListSize * sizeof( ZMacPanDesc_t ));
  else if ( param->ScanType == ZMAC_ORPHAN_SCAN )
    resultLen = 0;
  else if ( param->ScanType == ZMAC_ENHANCED_ACTIVE_SCAN )
    resultLen = (param->ResultListSize * sizeof( ZMacPanDesc_t ));
  else
    return;

  /* Make sure the result wont be more than the size */
  resultLen = MIN(resultLen, MT_MAC_SCAN_RESULT_LEN_MAX);

  respLen = MT_MAC_LEN_SCAN_CNF + MT_MAC_SCAN_RESULT_LEN_MAX + 1;  /* Extra byte for the length of the list */

  /* Allocate */
  pRetBuf = OsalPort_malloc (respLen);

  if ( pRetBuf )
  {
    tp = pRetBuf;

    /* Status */
    *tp++ = param->hdr.Status;

    /* ED max energy parameter no longer used */
    *tp++ = 0;

    /* Scan type */
    *tp++ = param->ScanType;

    /* Channel page */
    *tp++ = param->ChannelPage;

    /* Unscanned channel list */
    *tp++ = BREAK_UINT32( param->UnscannedChannels, 0 );
    *tp++ = BREAK_UINT32( param->UnscannedChannels, 1 );
    *tp++ = BREAK_UINT32( param->UnscannedChannels, 2 );
    *tp++ = BREAK_UINT32( param->UnscannedChannels, 3 );

    /* Result count */
    *tp++ = param->ResultListSize;

    /* MAX length of the result List */
    *tp++ = MT_MAC_SCAN_RESULT_LEN_MAX;

    /* clear extra buffer space */
    memset( tp, 0, MT_MAC_SCAN_RESULT_LEN_MAX );

    /* PAN descriptor information */
    OsalPort_memcpy( tp, param->Result.pPanDescriptor, resultLen );

    /* Build and send back the response */
    MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_AREQ | (uint8_t)MT_RPC_SYS_MAC), MT_MAC_SCAN_CNF, respLen, pRetBuf);

    /* Deallocate */
    OsalPort_free(pRetBuf);
  }
}

/***************************************************************************************************
 * @fn          nwk_MTCallbackSubCommStatusInd
 *
 * @brief       Process the callback subscription for comm_status_ind.
 *
 * @param       None
 *
 * @return      None
 ***************************************************************************************************/
void nwk_MTCallbackSubCommStatusInd ( ZMacCommStatusInd_t *param )
{
  uint8_t respLen;
  uint8_t *pRetBuf, *tp;

  respLen = MT_MAC_LEN_COMM_STATUS_IND;

  pRetBuf = OsalPort_malloc (respLen);

  if ( pRetBuf )
  {
    tp = pRetBuf;

    /* Status */
    *tp++ = param->hdr.Status;

    /* Source address */
    *tp++ = param->SrcAddress.addrMode;
    MT_MacAddr2Spi( tp, &param->SrcAddress );
    tp += Z_EXTADDR_LEN;

    /* Destination address */
    *tp++ = param->DstAddress.addrMode;
    MT_MacAddr2Spi( tp, &param->DstAddress );
    tp += Z_EXTADDR_LEN;

    /* PAN ID */
    *tp++ = LO_UINT16( param->PANId );
    *tp++ = HI_UINT16( param->PANId );

    /* Reason */
    *tp++ = param->Reason;

    /* Security */
    MT_MacSpi2Sec ((ZMacSec_t *)tp, (uint8_t *)&param->Sec);

    /* Build and send back the response */
    MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_AREQ | (uint8_t)MT_RPC_SYS_MAC), MT_MAC_COMM_STATUS_IND, respLen, pRetBuf);

    /* Deallocate */
    OsalPort_free(pRetBuf);
  }
}

/***************************************************************************************************
 * @fn          nwk_MTCallbackSubNwkStartCnf
 *
 * @brief       Process the callback subscription for nwk_start_cnf
 *
 * @param       byte Status
 *
 * @return      None
 ***************************************************************************************************/
void nwk_MTCallbackSubNwkStartCnf( uint8_t Status )
{
  uint8_t retValue;

  retValue = Status;

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_AREQ | (uint8_t)MT_RPC_SYS_MAC), MT_MAC_START_CNF, 1, &retValue);
}

/***************************************************************************************************
 * @fn          nwk_MTCallbackSubNwkRxEnableCnf
 *
 * @brief       Process the callback subscription for nwk_Rx_Enable_cnf
 *
 * @param
 *
 * @return      SUCCESS if message sent succesfully , else N_FAIL
 ***************************************************************************************************/
void nwk_MTCallbackSubNwkRxEnableCnf ( byte Status )
{
  uint8_t retValue;

  /* The only data byte is Status */
  retValue = Status;

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_AREQ | (uint8_t)MT_RPC_SYS_MAC), MT_MAC_RX_ENABLE_CNF, 1, &retValue);
}

/***************************************************************************************************
 * @fn          nwk_MTCallbackSubNwkPurgeCnf
 *
 * @brief       Process the callback subscription for nwk_purge_cnf
 *
 * @param       pointer of type ZMacPurgeCnf_t
 *
 * @return      SUCCESS if message sent succesfully , else N_FAIL
 ***************************************************************************************************/
void nwk_MTCallbackSubNwkPurgeCnf( ZMacPurgeCnf_t *param )
{
  uint8_t respLen;
  uint8_t *pRetBuf, *tp;

  respLen = MT_MAC_LEN_PURGE_CNF;

  pRetBuf = OsalPort_malloc (respLen);

  if ( pRetBuf )
  {
    tp = pRetBuf;

    /* Status */
    *tp++ = param->hdr.Status;

    /* Handle */
    *tp = param->msduHandle;

    /* Build and send back the response */
    MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_AREQ | (uint8_t)MT_RPC_SYS_MAC), MT_MAC_PURGE_CNF, respLen, pRetBuf);

    /* Deallocate */
    OsalPort_free(pRetBuf);
  }
}

#endif // MT_MAC_CB_FUNC
#endif // MT_MAC_FUNC

/***************************************************************************************************
 * SUPPORT
 ***************************************************************************************************/

/***************************************************************************************************
 * @fn      MT_MacRevExtCpy
 *
 * @brief
 *
 *   Reverse-copy an extended address.
 *
 * @param   pDst - Pointer to data destination
 * @param   pSrc - Pointer to data source
 *
 * @return  void
 ***************************************************************************************************/
static void MT_MacRevExtCpy( uint8_t *pDst, uint8_t *pSrc )
{
  int8_t i;

  for ( i = Z_EXTADDR_LEN - 1; i >= 0; i-- )
  {
    *pDst++ = pSrc[i];
  }
}

/***************************************************************************************************
 * @fn      MT_MacSpi2Addr
 *
 * @brief   Copy an address from an SPI message to an address struct.  The
 *          addrMode in pAddr must already be set.
 *
 * @param   pDst - Pointer to address struct
 * @param   pSrc - Pointer SPI message byte array
 *
 * @return  void
 ***************************************************************************************************/
static void MT_MacSpi2Addr( zAddrType_t *pDst, uint8_t *pSrc )
{
  if ( pDst->addrMode == Addr16Bit )
  {
    pDst->addr.shortAddr = BUILD_UINT16( pSrc[0] , pSrc[1] );
  }
  else if ( pDst->addrMode == Addr64Bit )
  {
    MT_MacRevExtCpy( pDst->addr.extAddr, pSrc );
  }
}

/***************************************************************************************************
 * @fn      MT_MacSpi2Sec
 *
 * @brief   Copy Security information from SPI message to a Sec structure
 *
 * @param   pSec - Pointer to security struct
 * @param   pSrc - Pointer SPI message byte array
 *
 * @return  void
 ***************************************************************************************************/
static void MT_MacSpi2Sec( ZMacSec_t *pSec, uint8_t *pSrc )
{
  /* Copy the security structure directly from the byte array */
  OsalPort_memcpy (pSec, pSrc, sizeof (ZMacSec_t));
}

#if defined ( MT_MAC_CB_FUNC )
/***************************************************************************************************
 * @fn      MT_MacAddr2Spi
 *
 * @brief   Copy an address from an address struct to an SPI message.
 *
 * @param   pDst - Pointer SPI message byte array
 * @param   pSrc - Pointer to address struct
 *
 * @return  void
 ***************************************************************************************************/
static void MT_MacAddr2Spi( uint8_t *pDst, zAddrType_t *pSrc )
{
  if ( pSrc->addrMode == Addr16Bit )
  {
    *pDst++ = LO_UINT16( pSrc->addr.shortAddr );
    *pDst++ = HI_UINT16( pSrc->addr.shortAddr );
    *pDst++ = 0; *pDst++ = 0; *pDst++ = 0;
    *pDst++ = 0; *pDst++ = 0; *pDst = 0;
  }
  else if ( pSrc->addrMode == Addr64Bit )
  {
    uint8_t i;
    for ( i = 0; i< Z_EXTADDR_LEN; i++ )
    {
     *pDst++ = pSrc->addr.extAddr[i];
    }
  }
  else
  {
    uint8_t i;
    for ( i = 0; i< Z_EXTADDR_LEN; i++ )
    {
     *pDst++ = pSrc->addr.extAddr[i];
    }
  }
}
#endif // MT_MAC_CB_FUNC

/***************************************************************************************************
 ***************************************************************************************************/
#endif // MT_MAC_FUNC
