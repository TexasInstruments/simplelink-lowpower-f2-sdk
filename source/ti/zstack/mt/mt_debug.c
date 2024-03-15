/***************************************************************************************************
  Filename:       MT_DEBUG.c
  Revised:        $Date: 2013-07-29 05:42:52 -0700 (Mon, 29 Jul 2013) $
  Revision:       $Revision: 34794 $

  Description:    MonitorTest functions for debug utilities.

  Copyright 2007-2013 Texas Instruments Incorporated.

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

 ***************************************************************************************************/

/***************************************************************************************************
 * INCLUDES
 ***************************************************************************************************/

#include "zcomdef.h"
#include "mt.h"
#include "mt_debug.h"

#include "mac_main.h"
#include "mac_data.h"
#include "mac_rx.h"
#include "mac_tx.h"
#include "nwk_globals.h"
#include "nwk_util.h"
#include "mac_radio_defs.h"
#include "osal_nv.h"

#include "bdb.h"
#if (ZG_BUILD_COORDINATOR_TYPE)
#include "ssp_hash.h"
#include "aps_mede.h"
#endif

#include "zglobals.h"
#include "nwk_util.h"
#ifdef APP_TP2
#include "zd_config.h"
#endif

/***************************************************************************************************
 * EXTERNAL FUNCTIONS
 ***************************************************************************************************/

#if defined ( APP_TP2_TEST_MODE )
extern void nwkListStatusAddToSortedList( uint16_t devAddr, uint8_t rxCost, uint8_t txCost,
                                  linkStatusListItem_t *pList, uint8_t neighbors );
#endif

/***************************************************************************************************
 * LOCAL FUNCTIONs
 ***************************************************************************************************/

#if defined (MT_DEBUG_FUNC)
static void MT_DebugSetThreshold(uint8_t *pBuf);
#if defined ( APP_TP2 )
  extern uint8_t TP2_securityEnabled;
  extern uint8_t zgAllowRejoinsWithWellKnownKey;
  static void MT_TP2_EnableApsSecurity(uint8_t *pBuf);
  static void MT_TP2_SetR20NodeDesc(uint8_t *pBuf);
  static void MT_TP2_SetEndDevTimeoutTo10s(uint8_t *pBuf);
  static void MT_TP2_SetFragBlockSize(uint8_t *pBuf);
  static void MT_TP2_SetZdoUseExtendedPanId(uint8_t *pBuf);
  #define EN_SECURITY                     0x40
#if defined ( APP_TP2_TEST_MODE )
  static void MT_TP2_SendLinkStatus(uint8_t *pBuf);
#endif // APP_TP2_TEST_MODE
#endif


static void MT_DebugMacDataDump(void);
#endif


#if defined (MT_DEBUG_FUNC)
/***************************************************************************************************
 * @fn      MT_DebugProcessing
 *
 * @brief   Process all the DEBUG commands that are issued by test tool
 *
 * @param   pBuf - pointer to received buffer
 *
 * @return  status
 ***************************************************************************************************/
uint8_t MT_DebugCommandProcessing(uint8_t *pBuf)
{
  uint8_t status = MT_RPC_SUCCESS;

  switch (pBuf[MT_RPC_POS_CMD1])
  {
    case MT_DEBUG_SET_THRESHOLD:
      MT_DebugSetThreshold(pBuf);
      break;



#if defined ( APP_TP2 )
  case MT_DEBUG_TP2_ENABLEAPSSECURITY:
    MT_TP2_EnableApsSecurity(pBuf);
  break;
  case MT_DEBUG_TP2_SET_NODE_R20:
    MT_TP2_SetR20NodeDesc(pBuf);
  break;
  case MT_DEBUG_TP2_SET_END_DEV_TIMEOUT_10S:
    MT_TP2_SetEndDevTimeoutTo10s(pBuf);
  break;
  case MT_DEBUG_TP2_SET_FRAG_BLOCK_SIZE:
    MT_TP2_SetFragBlockSize(pBuf);
  break;
  case MT_DEBUG_TP2_SET_ZDO_USEEXTPANID:
    MT_TP2_SetZdoUseExtendedPanId(pBuf);
  break;
#if defined ( APP_TP2_TEST_MODE )
  case MT_DEBUG_TP2_GU_SEND_LINKSTATUS:
    MT_TP2_SendLinkStatus(pBuf);
  break;
#endif // APP_TP2_TEST_MODE
#endif

    case MT_DEBUG_MAC_DATA_DUMP:
      MT_DebugMacDataDump();
      break;

    default:
      status = MT_RPC_ERR_COMMAND_ID;
      break;
  }

  return status;
}

/***************************************************************************************************
 * @fn      MT_DebugSetThreshold
 *
 * @brief   Process Debug Set Threshold
 *
 * @param   pBuf - pointer to received buffer
 *
 * @return  void
 ***************************************************************************************************/
static void MT_DebugSetThreshold(uint8_t *pBuf)
{
  uint8_t retValue = ZSuccess;
  uint8_t cmdId;

  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  /* Populate info */
  debugCompId = *pBuf++;
  debugThreshold = *pBuf;

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_DBG), cmdId, 1, &retValue);
}

#if defined ( APP_TP2 )
/***************************************************************************************************
 * @fn      MT_TP2_EnableApsSecurity
 *
 * @brief   Set the APS security on TP2 messages
 *
 * @param   pBuf - pointer to received buffer
 *
 * @return  void
 ***************************************************************************************************/
static void MT_TP2_EnableApsSecurity(uint8_t *pBuf)
{
  uint8_t retValue = ZSuccess;
  uint8_t cmdId;

  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  if(*pBuf)
  {
    TP2_securityEnabled = EN_SECURITY;
  }
  else
  {
    TP2_securityEnabled = 0;
  }

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_DBG), cmdId, 1, &retValue);
}

/***************************************************************************************************
 * @fn      MT_TP2_SetR20NodeDesc
 *
 * @brief   Set the device rev R20
 *
 * @param   pBuf - pointer to received buffer
 *
 * @return  void
 ***************************************************************************************************/
static void MT_TP2_SetR20NodeDesc(uint8_t *pBuf)
{
  uint8_t retValue = ZSuccess;
  uint8_t cmdId;

  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];

  //Mask to remove stack compliance Revision
  ZDO_Config_Node_Descriptor.ServerMask &= 0x1FFF;

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_DBG), cmdId, 1, &retValue);
}



/***************************************************************************************************
 * @fn      MT_TP2_SetEndDevTimeoutTo10s
 *
 * @brief   Set the device rev R20
 *
 * @param   pBuf - pointer to received buffer
 *
 * @return  void
 ***************************************************************************************************/
static void MT_TP2_SetEndDevTimeoutTo10s(uint8_t *pBuf)
{
  uint8_t retValue = ZSuccess;
  uint8_t cmdId;
  uint8_t index;

  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];

  for(index = 0; index < NWK_MAX_DEVICES; index++)
  {
    AssociatedDevList[index].timeoutCounter = 10;
    AssociatedDevList[index].endDev.deviceTimeout = 10;
  }

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_DBG), cmdId, 1, &retValue);
}

/***************************************************************************************************
 * @fn      MT_TP2_SetFragBlockSize
 *
 * @brief   Sets the max fragmentation block size
 *
 * @param   pBuf - pointer to received buffer
 *
 * @return  void
 ***************************************************************************************************/
static void MT_TP2_SetFragBlockSize(uint8_t *pBuf)
{
  uint8_t retValue = ZSuccess;
  uint8_t cmdId;

  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  // Sets aps maximum fragmentation block size
  guApsMaxFragBlockSize = *pBuf;

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_DBG), cmdId, 1, &retValue);
}

/***************************************************************************************************
 * @fn      MT_TP2_SetZdoUseExtendedPanId
 *
 * @brief   Set the ZDO_UseExtendedPANID on device
 *
 * @param   pBuf - pointer to received buffer
 *
 * @return  void
 ***************************************************************************************************/
static void MT_TP2_SetZdoUseExtendedPanId(uint8_t *pBuf)
{
  uint8_t retValue = ZSuccess;
  uint8_t cmdId;

  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  osal_cpyExtAddr( ZDO_UseExtendedPANID, pBuf );

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_DBG), cmdId, 1, &retValue);
}

#if defined ( APP_TP2_TEST_MODE )
/***************************************************************************************************
 * @fn      MT_TP2_SendLinkStatus
 *
 * @brief   Sends custom Link Status message
 *
 * @param   pBuf - pointer to received buffer
 *
 * @return  void
 ***************************************************************************************************/
static void MT_TP2_SendLinkStatus(uint8_t *pBuf)
{
  uint8_t retValue = ZSuccess;
  uint8_t cmdId;
  uint8_t i;
  uint8_t neighbors;

  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  linkStatusListItem_t *pList;

  /* Parse number of neighbors included in MT command */
  neighbors = *pBuf++;

  pList = OsalPort_malloc( neighbors * sizeof( linkStatusListItem_t ) );

  if ( pList )
  {
    memset( pList, 0xFF, (2 * sizeof ( linkStatusListItem_t )) );

    // Go through each entry in buffer ( devAddr, rxCost, txCost )
    for ( i = 0; i < neighbors; i++ )
    {
      uint16_t devAddr;
      devAddr = OsalPort_buildUint16( pBuf );
      pBuf += 2;
      nwkListStatusAddToSortedList( devAddr, pBuf[0], pBuf[1], pList, neighbors );
      pBuf += 2;
    }

    NLME_SendLinkStatus( neighbors, (LS_OPTION_FIRST_FRAME | LS_OPTION_LAST_FRAME), pList );

    OsalPort_free( pList );

    // Reset Link Status timer so that we don't send it more often than necessary
    NLME_SetLinkStatusTimer();
  }

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_DBG), cmdId, 1, &retValue);
}
#endif // APP_TP2_TEST_MODE

#endif // APP_TP2


/***************************************************************************************************
 * @fn      MT_DebugMacDataDump
 *
 * @brief   Process the debug MAC Data Dump request.
 *
 * @param   pBuf - pointer to received buffer
 *
 * @return  void
 ***************************************************************************************************/
static void MT_DebugMacDataDump(void)
{
  uint8_t buf[sizeof(mtDebugMacDataDump_t)];
  uint8_t *pBuf = buf;

#ifdef FEATURE_PACKET_FILTER_STATS
  *pBuf++ = BREAK_UINT32(nwkInvalidPackets, 0);
  *pBuf++ = BREAK_UINT32(nwkInvalidPackets, 1);
  *pBuf++ = BREAK_UINT32(nwkInvalidPackets, 2);
  *pBuf++ = BREAK_UINT32(nwkInvalidPackets, 3);
  *pBuf++ = BREAK_UINT32(rxCrcFailure, 0);
  *pBuf++ = BREAK_UINT32(rxCrcFailure, 1);
  *pBuf++ = BREAK_UINT32(rxCrcFailure, 2);
  *pBuf++ = BREAK_UINT32(rxCrcFailure, 3);
  *pBuf++ = BREAK_UINT32(rxCrcSuccess, 0);
  *pBuf++ = BREAK_UINT32(rxCrcSuccess, 1);
  *pBuf++ = BREAK_UINT32(rxCrcSuccess, 2);
  *pBuf++ = BREAK_UINT32(rxCrcSuccess, 3);
#endif
  *pBuf++ = 0;//FSMSTAT0;   //TODO: Update this register as per Agama device
  *pBuf++ = 0;//FSMSTAT1;
  *pBuf++ = macData.rxCount;
  *pBuf++ = macData.directCount;
  *pBuf++ = macMain.state;
  *pBuf++ = macRxActive;
  *pBuf   = macTxActive;

  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_DBG),
                                       MT_DEBUG_MAC_DATA_DUMP, sizeof(buf), buf);
}
#endif

/***************************************************************************************************
 * @fn      MT_ProcessDebugMsg
 *
 * @brief   Build and send a debug message.
 *
 * @param   byte *data - pointer to the data portion of the debug message
 *
 * @return  void
 ***************************************************************************************************/
void MT_ProcessDebugMsg( mtDebugMsg_t *msg )
{
  byte *msg_ptr;
  byte dataLen;
  uint8_t buf[11];
  uint8_t *pBuf;

  /* Calculate the data length based */
  dataLen = 5 + (msg->numParams * sizeof ( uint16_t ));

  /* Get a message buffer to build the debug message */
  msg_ptr = OsalPort_msgAllocate( (byte)(SPI_0DATA_MSG_LEN + dataLen + 1) );
  if ( msg_ptr )
  {
    /* Build the message */
    pBuf = buf;
    *pBuf++ = msg->compID;
    *pBuf++ = msg->severity;
    *pBuf++ = msg->numParams;

    if ( msg->numParams >= 1 )
    {
      *pBuf++ = LO_UINT16( msg->param1 );
      *pBuf++ = HI_UINT16( msg->param1 );
    }

    if ( msg->numParams >= 2 )
    {
      *pBuf++ = LO_UINT16( msg->param2 );
      *pBuf++ = HI_UINT16( msg->param2 );
    }

    if ( msg->numParams == 3 )
    {
      *pBuf++ = LO_UINT16( msg->param3 );
      *pBuf++ = HI_UINT16( msg->param3 );
    }

    *pBuf++ = LO_UINT16( msg->timestamp );
    *pBuf = HI_UINT16( msg->timestamp );

#ifdef MT_UART_DEFAULT_PORT
    /* Debug message is set to AREQ CMD 0x80 for now */
    /* Build and send back the response */
    MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_AREQ | (uint8_t)MT_RPC_SYS_DBG), 0x80, dataLen, buf);
#endif
    OsalPort_msgDeallocate( msg_ptr );
  }
}

/***************************************************************************************************
 * @fn      MT_ProcessDebugStr
 *
 * @brief   Build and send a debug string.
 *
 * @param   byte *dstr - pointer to the data portion of the debug message
 *
 * @return  void
 ***************************************************************************************************/
void MT_ProcessDebugStr(mtDebugStr_t *dstr)
{
  byte *msg_ptr;

  /* Get a message buffer to build the debug message */
  msg_ptr = OsalPort_malloc( (byte)(SPI_0DATA_MSG_LEN + dstr->strLen) );
  if ( msg_ptr )
  {
#ifdef MT_UART_DEFAULT_PORT
    /* Debug message is set to AREQ CMD 0x80 for now */
    /* Build and send back the response */
    MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_AREQ | (uint8_t)MT_RPC_SYS_DBG), MT_DEBUG_MSG, dstr->strLen, dstr->pString);
#endif
    OsalPort_free( msg_ptr );
  }
}

/**************************************************************************************************
 */
