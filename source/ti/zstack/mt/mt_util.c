/***************************************************************************************************
  Filename:       MT_UTIL.c
  Revised:        $Date: 2015-01-26 08:25:50 -0800 (Mon, 26 Jan 2015) $
  Revision:       $Revision: 42025 $

  Description:    MonitorTest Utility Functions

  Copyright 2007-2015 Texas Instruments Incorporated.

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

#include "addr_mgr.h"

#include "osal_nv.h"
#include "rom_jt_154.h"
#include "nl_mede.h"
#include "mt.h"
#include "mt_util.h"
#include "mt_mac.h"
#include "mt_zdo.h"
#include "ssp.h"

#if !defined NONWK

#include "mt_nwk.h"
#include "mt_af.h"
#include "assoc_list.h"
#include "zd_app.h"
#include "zd_sec_mgr.h"
#endif

#if defined MT_SRNG
#include "hal_srng.h"
#endif

/***************************************************************************************************
 * CONSTANTS
 ***************************************************************************************************/
#define MT_UTIL_DEVICE_INFO_RESPONSE_LEN 14
#define MT_UTIL_STATUS_LEN    1
#define MT_UTIL_FRM_CTR_LEN   4
// Status + LinkKeyDataLen + Tx+Rx Frame counter.
#define MT_APSME_LINKKEY_GET_RSP_LEN (MT_UTIL_STATUS_LEN + SEC_KEY_LEN + (MT_UTIL_FRM_CTR_LEN * 2))
// Status + NV id
#define MT_APSME_LINKKEY_NV_ID_GET_RSP_LEN (MT_UTIL_STATUS_LEN + 2)

/***************************************************************************************************
 * EXTERNAL REFERENCES
 ***************************************************************************************************/
// extern uint32_t _zdoCallbackSub;

/***************************************************************************************************
 * LOCAL VARIABLES
 ***************************************************************************************************/

/***************************************************************************************************
 * LOCAL FUNCTIONS
 ***************************************************************************************************/
#ifdef AUTO_PEND
static void MT_UtilRevExtCpy( uint8_t *pDst, uint8_t *pSrc );
static void MT_UtilSpi2Addr( zAddrType_t *pDst, uint8_t *pSrc );
#endif

#if defined (MT_UTIL_FUNC)
static void MT_UtilGetDeviceInfo(void);
static void MT_UtilGetNvInfo(void);
static void MT_UtilSetPanID(uint8_t *pBuf);
static void MT_UtilSetChannels(uint8_t *pBuf);
static void MT_UtilSetSecLevel(uint8_t *pBuf);
static void MT_UtilSetPreCfgKey(uint8_t *pBuf);
static void MT_UtilCallbackSub(uint8_t *pData);
static void MT_UtilTimeAlive(void);
static void MT_UtilSrcMatchEnable (uint8_t *pBuf);
static void MT_UtilSrcMatchAddEntry (uint8_t *pBuf);
static void MT_UtilSrcMatchDeleteEntry (uint8_t *pBuf);
static void MT_UtilSrcMatchCheckSrcAddr (uint8_t *pBuf);
static void MT_UtilSrcMatchAckAllPending (uint8_t *pBuf);
static void MT_UtilSrcMatchCheckAllPending (uint8_t *pBuf);

static void MT_UtilGpioSetDirection(uint8_t *pBuf);
static void MT_UtilGpioRead(uint8_t *pBuf);
static void MT_UtilGpioWrite(uint8_t *pBuf);

#ifdef MT_SRNG
static void MT_UtilSrngGen(void);
#endif

#if !defined NONWK
static void MT_UtilDataReq(uint8_t *pBuf);
static void MT_UtilAddrMgrEntryLookupExt(uint8_t *pBuf);
static void MT_UtilAddrMgrEntryLookupNwk(uint8_t *pBuf);
#if defined MT_SYS_KEY_MANAGEMENT
static void MT_UtilAPSME_LinkKeyDataGet(uint8_t *pBuf);
static void MT_UtilAPSME_LinkKeyNvIdGet(uint8_t *pBuf);
#endif //MT_SYS_KEY_MANAGEMENT
static void MT_UtilAPSME_RequestKeyCmd(uint8_t *pBuf);
static void MT_UtilAssocCount(uint8_t *pBuf);
static void MT_UtilAssocFindDevice(uint8_t *pBuf);
static void MT_UtilAssocGetWithAddress(uint8_t *pBuf);
static void MT_UtilBindAddEntry(uint8_t *pBuf);
static void packDev_t(uint8_t *pBuf, associated_devices_t *pDev);
static void packBindEntry_t(uint8_t *pBuf, BindingEntry_t *pBind);
static void MT_UtilSync(void);
#endif // !defined NONWK
#endif // MT_UTIL_FUNC

#if defined (MT_UTIL_FUNC)
/***************************************************************************************************
* @fn      MT_UtilProcessing
*
* @brief   Process all the DEBUG commands that are issued by test tool
*
* @param   pBuf  - pointer to received SPI data message
*
* @return  status
***************************************************************************************************/
uint8_t MT_UtilCommandProcessing(uint8_t *pBuf)
{
  uint8_t status = MT_RPC_SUCCESS;

  switch (pBuf[MT_RPC_POS_CMD1])
  {
    // CC253X MAC Network Processor does not have NV support
#if !defined(CC253X_MACNP)
  case MT_UTIL_GET_DEVICE_INFO:
    MT_UtilGetDeviceInfo();
    break;

  case MT_UTIL_GET_NV_INFO:
    MT_UtilGetNvInfo();
    break;

  case MT_UTIL_SET_PANID:
    MT_UtilSetPanID(pBuf);
    break;

  case MT_UTIL_SET_CHANNELS:
    MT_UtilSetChannels(pBuf);
    break;

  case MT_UTIL_SET_SECLEVEL:
    MT_UtilSetSecLevel(pBuf);
    break;

  case MT_UTIL_SET_PRECFGKEY:
    MT_UtilSetPreCfgKey(pBuf);
    break;
#endif

  case MT_UTIL_CALLBACK_SUB_CMD:
    MT_UtilCallbackSub(pBuf);
    break;

  case MT_UTIL_TIME_ALIVE:
    MT_UtilTimeAlive();
    break;

  case MT_UTIL_SRC_MATCH_ENABLE:
    MT_UtilSrcMatchEnable(pBuf);
    break;

  case MT_UTIL_SRC_MATCH_ADD_ENTRY:
    MT_UtilSrcMatchAddEntry(pBuf);
    break;

  case MT_UTIL_SRC_MATCH_DEL_ENTRY:
    MT_UtilSrcMatchDeleteEntry(pBuf);
    break;

  case MT_UTIL_SRC_MATCH_CHECK_SRC_ADDR:
    MT_UtilSrcMatchCheckSrcAddr(pBuf);
    break;

  case MT_UTIL_SRC_MATCH_ACK_ALL_PENDING:
    MT_UtilSrcMatchAckAllPending(pBuf);
    break;

  case MT_UTIL_SRC_MATCH_CHECK_ALL_PENDING:
    MT_UtilSrcMatchCheckAllPending(pBuf);
    break;

  case MT_UTIL_TEST_LOOPBACK:
    MT_BuildAndSendZToolResponse((MT_RPC_CMD_SRSP|(uint8_t)MT_RPC_SYS_UTIL), MT_UTIL_TEST_LOOPBACK,
                                 pBuf[MT_RPC_POS_LEN], (pBuf+MT_RPC_FRAME_HDR_SZ));
    break;

  case MT_UTIL_GPIO_SET_DIRECTION:
    MT_UtilGpioSetDirection(pBuf);
    break;

  case MT_UTIL_GPIO_READ:
    MT_UtilGpioRead(pBuf);
    break;

  case MT_UTIL_GPIO_WRITE:
    MT_UtilGpioWrite(pBuf);
    break;

#if !defined NONWK
  case MT_UTIL_DATA_REQ:
    MT_UtilDataReq(pBuf);
    break;

  case MT_UTIL_ADDRMGR_EXT_ADDR_LOOKUP:
    MT_UtilAddrMgrEntryLookupExt(pBuf);
    break;

  case MT_UTIL_ADDRMGR_NWK_ADDR_LOOKUP:
    MT_UtilAddrMgrEntryLookupNwk(pBuf);
    break;

#if defined MT_SYS_KEY_MANAGEMENT
  case MT_UTIL_APSME_LINK_KEY_DATA_GET:
    MT_UtilAPSME_LinkKeyDataGet(pBuf);
    break;

  case MT_UTIL_APSME_LINK_KEY_NV_ID_GET:
    MT_UtilAPSME_LinkKeyNvIdGet(pBuf);
    break;
#endif // MT_SYS_KEY_MANAGEMENT

  case MT_UTIL_APSME_REQUEST_KEY_CMD:
    MT_UtilAPSME_RequestKeyCmd(pBuf);
    break;

  case MT_UTIL_ASSOC_COUNT:
    MT_UtilAssocCount(pBuf);
    break;

  case MT_UTIL_ASSOC_FIND_DEVICE:
    MT_UtilAssocFindDevice(pBuf);
    break;

  case MT_UTIL_ASSOC_GET_WITH_ADDRESS:
    MT_UtilAssocGetWithAddress(pBuf);
    break;

  case MT_UTIL_BIND_ADD_ENTRY:
    MT_UtilBindAddEntry(pBuf);
    break;

  case MT_UTIL_SYNC_REQ:
    MT_UtilSync();
    break;
#endif /* !defined NONWK */

#ifdef MT_SRNG
  case MT_UTIL_SRNG_GENERATE:
    MT_UtilSrngGen();
    break;
#endif

  default:
    status = MT_RPC_ERR_COMMAND_ID;
    break;
  }

  return status;
}

/***************************************************************************************************
* @fn      MT_UtilGetDeviceInfo
*
* @brief   The Get Device Info serial message.
*
* @param   None.
*
* @return  void
***************************************************************************************************/
static void MT_UtilGetDeviceInfo(void)
{
  uint8_t  *buf;
  uint8_t  *pBuf;
  uint8_t  bufLen = MT_UTIL_DEVICE_INFO_RESPONSE_LEN;
  uint16_t *assocList = NULL;

#if !defined NONWK
  uint8_t  assocCnt = 0;

  if (ZG_DEVICE_RTR_TYPE)
  {
    assocList = AssocMakeList( &assocCnt );
    bufLen += (assocCnt * sizeof(uint16_t));
  }
#endif

  buf = OsalPort_malloc( bufLen );
  if ( buf )
  {
    pBuf = buf;

    *pBuf++ = ZSUCCESS; // Status

    osal_nv_read( ZCD_NV_EXTADDR, 0, Z_EXTADDR_LEN, pBuf );
    pBuf += Z_EXTADDR_LEN;

#if defined NONWK
    // Skip past ZStack only parameters for NONWK
    *pBuf++ = 0;
    *pBuf++ = 0;
    *pBuf++ = 0;
    *pBuf++ = 0;
    *pBuf = 0;
#else
    {
      uint16_t shortAddr = NLME_GetShortAddr();
      *pBuf++ = LO_UINT16( shortAddr );
      *pBuf++ = HI_UINT16( shortAddr );
    }

    /* Return device type */
    *pBuf++ = ZSTACK_DEVICE_BUILD;

    /*Return device state */
    *pBuf++ = (uint8_t)devState;

    if (ZG_DEVICE_RTR_TYPE)
    {
      *pBuf++ = assocCnt;

      if ( assocCnt )
      {
        uint8_t x;
        uint16_t *puint16 = assocList;

        for ( x = 0; x < assocCnt; x++, puint16++ )
        {
          *pBuf++ = LO_UINT16( *puint16 );
          *pBuf++ = HI_UINT16( *puint16 );
        }
      }
    }
    else
    {
      *pBuf++ = 0;
    }
#endif

    MT_BuildAndSendZToolResponse( ((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_UTIL),
                                 MT_UTIL_GET_DEVICE_INFO,
                                 bufLen, buf );

    OsalPort_free( buf );
  }

  if ( assocList )
  {
    OsalPort_free( assocList );
  }
}

#ifdef MT_SRNG
/***************************************************************************************************
* @fn      MT_UtilSrngGen
*
* @brief   Generate Secure Random Numbers
*
* @param   None.
*
* @return  void
***************************************************************************************************/
static void MT_UtilSrngGen(void)
{
  static uint32_t count = 125000; /* 125000 * 8 bits = 1000000 bits */
  uint8_t outrng[100];
  uint8_t status;

  if(count > 0)
  {
    status = ssp_srng_generate((uint8_t *)outrng, 100, NULL);
    if (status != SRNG_SUCCESS)
    {
      if(RNG_INIT_ERROR == status)
      {
        ssp_srng_reseed();
      }
      else
      {
        while(1)
        {
          ASM_NOP;
        }
      } /* if(RNG_INIT_ERROR == status) */
    }/*if (status != SRNG_SUCCESS) */

    if(count >= 100)
    {
      count -= 100;
    }
    else
    {
      count = 0;
    }
    MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_AREQ |
                                 (uint8_t)MT_RPC_SYS_DBG),
                                  MT_DEBUG_MSG,
                                  100,
                                  outrng);
    OsalPortTimers_startTimer(MT_TaskID, MT_SRNG_EVENT, 100);
  }
}
#endif

/***************************************************************************************************
 * @fn      MT_UtilGetNvInfo
 *
 * @brief   The Get NV Info serial message.
 *
 * @param   None.
 *
 * @return  void
 ***************************************************************************************************/
static void MT_UtilGetNvInfo(void)
{
  uint8_t len;
  uint8_t stat;
  uint8_t *buf;
  uint8_t *pBuf;
  uint16_t tmp16;
  uint32_t tmp32;

  /*
    Get required length of buffer
    Status + ExtAddr + ChanList + PanID  + SecLevel + PreCfgKey
  */
  len = 1 + Z_EXTADDR_LEN + 4 + 2 + 1 + SEC_KEY_LEN;

  buf = OsalPort_malloc( len );
  if ( buf )
  {
    /* Assume NV not available */
    memset( buf, 0xFF, len );

    /* Skip over status */
    pBuf = buf + 1;

    /* Start with 64-bit extended address */
    stat = osal_nv_read( ZCD_NV_EXTADDR, 0, Z_EXTADDR_LEN, pBuf );
    if ( stat ) stat = 0x01;
    pBuf += Z_EXTADDR_LEN;

    /* Scan channel list (bit mask) */
    if (  osal_nv_read( ZCD_NV_CHANLIST, 0, sizeof( tmp32 ), &tmp32 ) )
    {
      stat |= 0x02;
    }
    else
    {
      pBuf[0] = BREAK_UINT32( tmp32, 3 );
      pBuf[1] = BREAK_UINT32( tmp32, 2 );
      pBuf[2] = BREAK_UINT32( tmp32, 1 );
      pBuf[3] = BREAK_UINT32( tmp32, 0 );
    }
    pBuf += sizeof( tmp32 );

    /* ZigBee PanID */
    if ( osal_nv_read( ZCD_NV_PANID, 0, sizeof( tmp16 ), &tmp16 ) )
    {
      stat |= 0x04;
    }
    else
    {
      pBuf[0] = LO_UINT16( tmp16 );
      pBuf[1] = HI_UINT16( tmp16 );
    }
    pBuf += sizeof( tmp16 );

    /* Security level */
    if ( osal_nv_read( ZCD_NV_SECURITY_LEVEL, 0, sizeof( uint8_t ), pBuf++ ) )
    {
      stat |= 0x08;
    }
    /* Pre-configured security key */
    if ( osal_nv_read( ZCD_NV_PRECFGKEY, 0, SEC_KEY_LEN, pBuf ) )
    {
      stat |= 0x10;
    }
    /* Status bit mask - bit=1 indicates failure */
    *buf = stat;

    MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_UTIL), MT_UTIL_GET_NV_INFO,
                                  len, buf );

    OsalPort_free( buf );
  }
}

/***************************************************************************************************
 * @fn      MT_UtilSetPanID
 *
 * @brief   Set PanID message
 *
 * @param   pBuf - pointer to the data
 *
 * @return  void
 ***************************************************************************************************/
static void MT_UtilSetPanID(uint8_t *pBuf)
{
  uint16_t temp16;
  uint8_t retValue;
  uint8_t cmdId;

  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  temp16 = OsalPort_buildUint16( pBuf );

  retValue = osal_nv_write(ZCD_NV_PANID, osal_nv_item_len( ZCD_NV_PANID ), &temp16);

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_UTIL), cmdId, 1, &retValue);
}

/***************************************************************************************************
 * @fn      MT_UtilSetChannels
 *
 * @brief   Set Channels
 *
 * @param   pBuf - pointer to the data
 *
 * @return  void
 ***************************************************************************************************/
static void MT_UtilSetChannels(uint8_t *pBuf)
{
  uint32_t tmp32;
  uint8_t retValue;
  uint8_t cmdId;

  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  tmp32 = OsalPort_buildUint32( pBuf, 4 );

  retValue = osal_nv_write(ZCD_NV_CHANLIST, osal_nv_item_len( ZCD_NV_CHANLIST ), &tmp32);

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_UTIL), cmdId, 1, &retValue);
}

/***************************************************************************************************
 * @fn      MT_UtilSetSecLevel
 *
 * @brief   Set Sec Level
 *
 * @param   byte *msg - pointer to the data
 *
 * @return  void
 ***************************************************************************************************/
static void MT_UtilSetSecLevel(uint8_t *pBuf)
{
  uint8_t retValue;
  uint8_t cmdId;

  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  retValue = osal_nv_write( ZCD_NV_SECURITY_LEVEL, osal_nv_item_len( ZCD_NV_SECURITY_LEVEL ), pBuf);

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_UTIL), cmdId, 1, &retValue );

}

/***************************************************************************************************
 * @fn      MT_UtilSetPreCfgKey
 *
 * @brief   Set Pre Cfg Key
 *
 * @param   pBuf - pointer to the data
 *
 * @return  void
 ***************************************************************************************************/
static void MT_UtilSetPreCfgKey(uint8_t *pBuf)
{
  uint8_t retValue;
  uint8_t cmdId;

  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  retValue = osal_nv_write( ZCD_NV_PRECFGKEY, osal_nv_item_len( ZCD_NV_PRECFGKEY ), pBuf);

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_UTIL), cmdId, 1, &retValue );

}

/***************************************************************************************************
 * @fn      MT_UtilCallbackSub
 *
 * @brief   The Callback subscribe.
 *
 * @param   pBuf - pointer to the data
 *
 * @return  void
 ***************************************************************************************************/
void MT_UtilCallbackSub(uint8_t *pBuf)
{
  uint8_t cmdId = pBuf[MT_RPC_POS_CMD1];
  uint8_t retValue = ZFailure;

#if defined(MT_MAC_CB_FUNC) || defined(MT_NWK_CB_FUNC) || defined(MT_ZDO_CB_FUNC) || defined(MT_AF_CB_FUNC) || defined(MT_SAPI_CB_FUNC)
  uint8_t subSystem;
  uint16_t subscribed_command;

  // Move past header
  retValue = ZSuccess;
  pBuf += MT_RPC_FRAME_HDR_SZ;

  /* Command */
  subscribed_command = OsalPort_buildUint16( pBuf );
  pBuf += 2;

  /* Subsystem - 5 bits on the MSB of the command */
  subSystem = HI_UINT16(subscribed_command) & 0x1F ;

  /* What is the action - SUBSCRIBE or !SUBSCRIBE */
  if (*pBuf)
  {
    /* Turn ON */
  #if defined( MT_MAC_CB_FUNC )
    if ((subSystem == MT_RPC_SYS_MAC) || (subscribed_command == 0xFFFF))
    {
      DMMGR_SaveMacCbReg( 0xFFFF );
    }
  #endif

  #if defined( MT_NWK_CB_FUNC )
    if ((subSystem == MT_RPC_SYS_NWK) || (subscribed_command == 0xFFFF))
      _nwkCallbackSub = 0xFFFF;
  #endif

  #if defined( MT_ZDO_CB_FUNC )
    if ((subSystem == MT_RPC_SYS_ZDO) || (subscribed_command == 0xFFFF))
      _zdoCallbackSub = 0xFFFFFFFF;
  #endif

  #if defined( MT_AF_CB_FUNC )
    if ((subSystem == MT_RPC_SYS_AF) || (subscribed_command == 0xFFFF))
      _afCallbackSub = 0xFFFF;
  #endif

  #if defined( MT_SAPI_CB_FUNC )
    if ((subSystem == MT_RPC_SYS_SAPI) || (subscribed_command == 0xFFFF))
      _sapiCallbackSub = 0xFFFF;
  #endif
  }
  else
  {
    /* Turn OFF */
  #if defined( MT_MAC_CB_FUNC )
    if ((subSystem == MT_RPC_SYS_MAC) || (subscribed_command == 0xFFFF))
      _macCallbackSub = 0x0000;
  #endif

  #if defined( MT_NWK_CB_FUNC )
    if ((subSystem == MT_RPC_SYS_NWK) || (subscribed_command == 0xFFFF))
      _nwkCallbackSub = 0x0000;
  #endif

  #if defined( MT_ZDO_CB_FUNC )
    if ((subSystem == MT_RPC_SYS_ZDO) || (subscribed_command == 0xFFFF))
      _zdoCallbackSub = 0x00000000;
  #endif

  #if defined( MT_AF_CB_FUNC )
    if ((subSystem == MT_RPC_SYS_AF) || (subscribed_command == 0xFFFF))
      _afCallbackSub = 0x0000;
  #endif

  #if defined( MT_SAPI_CB_FUNC )
    if ((subSystem == MT_RPC_SYS_SAPI) || (subscribed_command == 0xFFFF))
        _sapiCallbackSub = 0x0000;
  #endif
  }
#endif  // MT_MAC_CB_FUNC || MT_NWK_CB_FUNC || MT_ZDO_CB_FUNC || MT_AF_CB_FUNC || MT_SAPI_CB_FUNC || MT_SAPI_CB_FUNC

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_UTIL), cmdId, 1, &retValue );
}

/***************************************************************************************************
 * @fn      MT_UtilTimeAlive
 *
 * @brief   Process Time Alive
 *
 * @param   None.
 *
 * @return  None
 ***************************************************************************************************/
static void MT_UtilTimeAlive(void)
{
  uint8_t timeAlive[4];
  uint32_t tmp32;

  /* Time since last reset (seconds) */
  tmp32 = MAP_osal_GetSystemClock() / 1000;

  /* Convert to high byte first into temp buffer */
  OsalPort_bufferUint32( timeAlive, tmp32 );

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_UTIL),
                                       MT_UTIL_TIME_ALIVE, sizeof(timeAlive), timeAlive);
}

/***************************************************************************************************
 * @fn          MT_UtilSrcMatchEnable
 *
 * @brief      Enabled AUTOPEND and source address matching.
 *
 * @param      pBuf - Buffer contains the data
 *
 * @return     void
 ***************************************************************************************************/
static void MT_UtilSrcMatchEnable (uint8_t *pBuf)
{
  uint8_t retValue, cmdId;

  /* Parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

#ifdef AUTO_PEND
  /* Call the routine */
  retValue = ZMacSrcMatchEnable();
#else
  retValue = ZMacUnsupported;
#endif

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_UTIL), cmdId, 1, &retValue );

}

/***************************************************************************************************
 * @fn          MT_UtilSrcMatchAddEntry
 *
 * @brief       Add a short or extended address to source address table.
 *
 * @param       pBuf - Buffer contains the data
 *
 * @return      void
 ***************************************************************************************************/
static void MT_UtilSrcMatchAddEntry (uint8_t *pBuf)
{
  uint8_t retValue, cmdId;

  /* Parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

#ifdef AUTO_PEND
  uint16_t panID;
  zAddrType_t devAddr;

  /* Address mode */
  devAddr.addrMode = *pBuf++;

  /* Address based on the address mode */
  MT_UtilSpi2Addr( &devAddr, pBuf);
  pBuf += Z_EXTADDR_LEN;

  /* PanID */
  panID = OsalPort_buildUint16( pBuf );

  /* Call the routine */
  retValue =  ZMacSrcMatchAddEntry (&devAddr, panID);
#else
  retValue = ZMacUnsupported;
#endif

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_UTIL), cmdId, 1, &retValue );
}

/***************************************************************************************************
 * @fn          MT_UtilSrcMatchDeleteEntry
 *
 * @brief      Delete a short or extended address from source address table.
 *
 * @param      pBuf - Buffer contains the data
 *
 * @return     void
 ***************************************************************************************************/
static void MT_UtilSrcMatchDeleteEntry (uint8_t *pBuf)
{
  uint8_t retValue, cmdId;

  /* Parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

#ifdef AUTO_PEND
  uint16_t panID;
  zAddrType_t devAddr;

  /* Address mode */
  devAddr.addrMode = *pBuf++;

  /* Address based on the address mode */
  MT_UtilSpi2Addr( &devAddr, pBuf);
  pBuf += Z_EXTADDR_LEN;

  /* PanID */
  panID = OsalPort_buildUint16( pBuf );

  /* Call the routine */
  retValue =  ZMacSrcMatchDeleteEntry (&devAddr, panID);
#else
  retValue = ZMacUnsupported;
#endif

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_UTIL), cmdId, 1, &retValue );
}

/***************************************************************************************************
 * @fn          MT_UtilSrcMatchCheckSrcAddr
 *
 * @brief      Check if a short or extended address is in the source address table.
 *
 * @param      pBuf - Buffer contains the data
 *
 * @return     void
 ***************************************************************************************************/
static void MT_UtilSrcMatchCheckSrcAddr (uint8_t *pBuf)
{
  uint8_t cmdId;
  uint8_t retArray[2];

  /* Parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

#if 0  /* Unsupported  */
  uint16_t panID;
  zAddrType_t devAddr;

  /* Address mode */
  devAddr.addrMode = *pBuf++;

  /* Address based on the address mode */
  MT_UtilSpi2Addr( &devAddr, pBuf);
  pBuf += Z_EXTADDR_LEN;

  /* PanID */
  panID = OsalPort_buildUint16( pBuf );

  /* Call the routine */
  retArray[1] =  ZMacSrcMatchCheckSrcAddr (&devAddr, panID);

    /* Return failure if the index is invalid */
  if (retArray[1] == ZMacSrcMatchInvalidIndex )
  {
    retArray[0] = ZFailure;
  }
  else
  {
    retArray[0] = ZSuccess;
  }
#else
  retArray[0] = ZMacUnsupported;
  retArray[1] = ZMacSrcMatchInvalidIndex;
#endif

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_UTIL), cmdId, 2, retArray );
}

/***************************************************************************************************
 * @fn          MT_UtilSrcMatchAckAllPending
 *
 * @brief       Enabled/disable acknowledging all packets with pending bit set
 *              It is normally enabled when adding new entries to
 *              the source address table fails due to the table is full, or
 *              disabled when more entries are deleted and the table has
 *              empty slots.
 *
 * @param       pBuf - Buffer contains the data
 *
 * @return      void
 ***************************************************************************************************/
static void MT_UtilSrcMatchAckAllPending (uint8_t *pBuf)
{
  uint8_t retValue, cmdId;

  /* Parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

#ifdef AUTO_PEND
  /* Call the routine */
  retValue = ZMacSrcMatchAckAllPending(*pBuf);
#else
  retValue = ZMacUnsupported;
#endif

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_UTIL), cmdId, 1, &retValue );
}

/***************************************************************************************************
 * @fn          MT_UtilSrcMatchCheckAllPending
 *
 * @brief       Check if acknowledging all packets with pending bit set
 *              is enabled.
 *
 * @param       pBuf - Buffer contains the data
 *
 * @return      void
 ***************************************************************************************************/
static void MT_UtilSrcMatchCheckAllPending (uint8_t *pBuf)
{
  uint8_t retArray[2], cmdId;

  /* Parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

#ifdef AUTO_PEND
  /* Call the routine */
  retArray[0] = ZMacSuccess;
  retArray[1] = ZMacSrcMatchCheckAllPending();
#else
  retArray[0] = ZMacUnsupported;
  retArray[1] = FALSE;
#endif

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_UTIL), cmdId, 2, retArray );
}

/***************************************************************************************************
 * SUPPORT
 ***************************************************************************************************/

#ifdef AUTO_PEND
/***************************************************************************************************
 * @fn      MT_UtilRevExtCpy
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
static void MT_UtilRevExtCpy( uint8_t *pDst, uint8_t *pSrc )
{
  int8_t i;

  for ( i = Z_EXTADDR_LEN - 1; i >= 0; i-- )
  {
    *pDst++ = pSrc[i];
  }
}

/***************************************************************************************************
 * @fn      MT_UtilSpi2Addr
 *
 * @brief   Copy an address from an SPI message to an address struct.  The
 *          addrMode in pAddr must already be set.
 *
 * @param   pDst - Pointer to address struct
 * @param   pSrc - Pointer SPI message byte array
 *
 * @return  void
 ***************************************************************************************************/
static void MT_UtilSpi2Addr( zAddrType_t *pDst, uint8_t *pSrc )
{
  if ( pDst->addrMode == Addr16Bit )
  {
    pDst->addr.shortAddr = OsalPort_buildUint16( pSrc );
  }
  else if ( pDst->addrMode == Addr64Bit )
  {
    MT_UtilRevExtCpy( pDst->addr.extAddr, pSrc );
  }
}
#endif // AUTO_PEND

/***************************************************************************************************
 * @fn      MT_UtilGpioRead
 *
 * @brief   Read values of all GPIOs (P0_0 -P2_4)
 *
 * @param   void
 *
 * @return  P0, P1, P2, P0DIR, P1DIR, P2DIR
 ***************************************************************************************************/
static void MT_UtilGpioRead(uint8_t *pBuf)
{
#if defined ( HAL_MCU_CC2530 )
  uint8_t rtrn[6] = {P0, P1, P2, P0DIR, P1DIR, P2DIR};
#else
  uint8_t rtrn[6] = {0, 0, 0, 0, 0, 0};
#endif
  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_UTIL), MT_UTIL_GPIO_READ,
    6, rtrn);
}

/***************************************************************************************************
 * @fn      MT_UtilGpioSetDirection
 *
 * @brief   Set the direction of a specific GPIO (P0_0 -P2_4)
 *
 * @param   port - 0, 1 or 2
 * @param   bit - 0 - 7
 * @param   direction - 0 for input, 1 for output
 *
 * @return  oldP0DIR, oldP1DIR, oldP2DIR, newP0DIR, newP1DIR, newP2DIR
 ***************************************************************************************************/
static void MT_UtilGpioSetDirection(uint8_t *pBuf)
{
#if defined ( HAL_MCU_CC2530 )
  uint8_t rtrn[6] = {P0DIR, P1DIR, P2DIR, 0, 0, 0};
  uint8_t port = pBuf[MT_RPC_POS_DAT0 + 0];
  uint8_t bit = pBuf[MT_RPC_POS_DAT0 + 1];
  uint8_t direction = pBuf[MT_RPC_POS_DAT0 + 2];

  if (direction == 0)
  {
    switch (port)
    {
      case 0:
        P0DIR &= (~ BV(bit));
        break;
      case 1:
        P1DIR &= (~ BV(bit));
        break;
      case 2:
        P2DIR &= (~ BV(bit));
        break;
    }
  }
  else
  {
    switch (port)
    {
      case 0:
        P0DIR |= BV(bit);
        break;
      case 1:
        P1DIR |= BV(bit);
        break;
      case 2:
        P2DIR |= BV(bit);
        break;
    }
  }

  rtrn[3] = P0DIR;
  rtrn[4] = P1DIR;
  rtrn[5] = P2DIR;
#else
  uint8_t rtrn[6] = {0, 0, 0, 0, 0, 0};
#endif

  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_UTIL), MT_UTIL_GPIO_SET_DIRECTION,
    6, rtrn);
}

/***************************************************************************************************
 * @fn      MT_UtilGpioWrite
 *
 * @brief   Write value of a specific GPIO (P0_0 -P2_4)
 *
 * @param   port - 0, 1 or 2
 * @param   bit - 0 - 7
 * @param   value - 0 or 1
 *
 * @return  oldP0, oldP1, oldP2, newP0, newP1, newP2, P0DIR, P1DIR, P2DIR
 ***************************************************************************************************/
static void MT_UtilGpioWrite(uint8_t *pBuf)
{
#if defined ( HAL_MCU_CC2530 )
  uint8_t rtrn[9] = {P0, P1, P2, 0, 0, 0, P0DIR, P1DIR, P2DIR};
  uint8_t port = pBuf[MT_RPC_POS_DAT0 + 0];
  uint8_t bit = pBuf[MT_RPC_POS_DAT0 + 1];
  uint8_t value = pBuf[MT_RPC_POS_DAT0 + 2];

  if (value == 0)
  {
    switch (port)
    {
      case 0:
        P0 &= (~ BV(bit));
        break;
      case 1:
        P1 &= (~ BV(bit));
        break;
      case 2:
        P2 &= (~ BV(bit));
        break;
    }
  }
  else
  {
    switch (port)
    {
      case 0:
        P0 |= BV(bit);
        break;
      case 1:
        P1 |= BV(bit);
        break;
      case 2:
        P2 |= BV(bit);
        break;
    }
  }

  rtrn[3] = P0;
  rtrn[4] = P1;
  rtrn[5] = P2;

#else
  uint8_t rtrn[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
#endif

  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_UTIL), MT_UTIL_GPIO_WRITE,
    9, rtrn);
}

#if !defined NONWK
/**************************************************************************************************
 * @fn      MT_UtilDataReq
 *
 * @brief   Process the MAC Data Request command.
 *
 * @param   pBuf - pointer to the received data
 *
 * @return  None
**************************************************************************************************/
static void MT_UtilDataReq(uint8_t *pBuf)
{
  uint8_t rtrn = NwkPollReq(pBuf[MT_RPC_POS_DAT0]);
  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_UTIL), MT_UTIL_DATA_REQ,
                                                                               1, &rtrn);
}

/***************************************************************************************************
 * @fn      MT_UtilAddrMgrEntryLookupExt
 *
 * @brief   Proxy the AddrMgrEntryLookupExt() function.
 *
 * @param   pBuf - pointer to the received buffer
 *
 * @return  void
 ***************************************************************************************************/
static void MT_UtilAddrMgrEntryLookupExt(uint8_t *pBuf)
{
  uint8_t nwkAddr[2];
  AddrMgrEntry_t entry;
  uint8_t cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  OsalPort_memcpy(entry.extAddr, pBuf, Z_EXTADDR_LEN);
  (void)AddrMgrEntryLookupExt(&entry);

  nwkAddr[0] = LO_UINT16(entry.nwkAddr);
  nwkAddr[1] = HI_UINT16(entry.nwkAddr);
  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_UTIL),
                                       cmdId, sizeof(uint16_t), nwkAddr);
}

/***************************************************************************************************
 * @fn      MT_UtilAddrMgrEntryLookupNwk
 *
 * @brief   Proxy the AddrMgrEntryLookupNwk() function.
 *
 * @param   pBuf - pointer to the received buffer
 *
 * @return  void
 ***************************************************************************************************/
static void MT_UtilAddrMgrEntryLookupNwk(uint8_t *pBuf)
{
  AddrMgrEntry_t entry;
  uint8_t cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  entry.nwkAddr = OsalPort_buildUint16( pBuf );
  (void)AddrMgrEntryLookupNwk(&entry);

  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_UTIL),
                                       cmdId, Z_EXTADDR_LEN, entry.extAddr);
}

#if defined MT_SYS_KEY_MANAGEMENT
/***************************************************************************************************
 * @fn      MT_UtilAPSME_LinkKeyDataGet
 *
 * @brief   Retrieves APS Link Key data from NV.
 *
 * @param   pBuf - pointer to the received buffer
 *
 * @return  void
 ***************************************************************************************************/
static void MT_UtilAPSME_LinkKeyDataGet(uint8_t *pBuf)
{
  uint8_t rsp[MT_APSME_LINKKEY_GET_RSP_LEN];
  APSME_ApsLinkKeyNVEntry_t *pData = NULL;
  uint8_t cmdId = pBuf[MT_RPC_POS_CMD1];
  uint16_t apsLinkKeyNvId;
  uint32_t *apsRxFrmCntr;
  uint32_t *apsTxFrmCntr;

  pBuf += MT_RPC_FRAME_HDR_SZ;

  *rsp = APSME_LinkKeyNVIdGet(pBuf, &apsLinkKeyNvId);

  if (SUCCESS == *rsp)
  {
    pData = (APSME_ApsLinkKeyNVEntry_t *)OsalPort_malloc(sizeof(APSME_ApsLinkKeyNVEntry_t));

    if (pData != NULL)
    {
      // retrieve key from NV
      if ( osal_nv_read_ex( ZCD_NV_EX_APS_KEY_DATA_TABLE, apsLinkKeyNvId, 0,
                        sizeof(APSME_ApsLinkKeyNVEntry_t), pData) == SUCCESS)

      {
        uint8_t *ptr = rsp+1;
        apsRxFrmCntr = &ApsLinkKeyRAMEntry[apsLinkKeyNvId].rxFrmCntr;
        apsTxFrmCntr = &ApsLinkKeyRAMEntry[apsLinkKeyNvId].txFrmCntr;

        (void)OsalPort_memcpy(ptr, pData->key, SEC_KEY_LEN);
        ptr += SEC_KEY_LEN;
        OsalPort_bufferUint32( ptr, *apsTxFrmCntr );
        ptr += 4;
        OsalPort_bufferUint32( ptr, *apsRxFrmCntr );
      }

      // clear copy of key in RAM
      memset( pData, 0x00, sizeof(APSME_ApsLinkKeyNVEntry_t) );

      OsalPort_free(pData);
    }
  }
  else
  {
    // set data key and counters 0xFF
    memset(&rsp[1], 0xFF, SEC_KEY_LEN + (MT_UTIL_FRM_CTR_LEN * 2));
  }

  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_UTIL), cmdId,
                                       MT_APSME_LINKKEY_GET_RSP_LEN, rsp);

  // clear key data
  memset(rsp, 0x00, MT_APSME_LINKKEY_GET_RSP_LEN);

}

/***************************************************************************************************
 * @fn      MT_UtilAPSME_LinkKeyNvIdGet
 *
 * @brief   Retrieves APS Link Key NV ID from the entry table.
 *
 * @param   pBuf - pointer to the received buffer
 *
 * @return  void
 ***************************************************************************************************/
static void MT_UtilAPSME_LinkKeyNvIdGet(uint8_t *pBuf)
{
  uint8_t rsp[MT_APSME_LINKKEY_NV_ID_GET_RSP_LEN];
  uint8_t cmdId = pBuf[MT_RPC_POS_CMD1];
  uint16_t apsLinkKeyNvId;

  pBuf += MT_RPC_FRAME_HDR_SZ;

  *rsp = APSME_LinkKeyNVIdGet(pBuf, &apsLinkKeyNvId);

  if (SUCCESS == *rsp)
  {
    rsp[1] = LO_UINT16(apsLinkKeyNvId);
    rsp[2] = HI_UINT16(apsLinkKeyNvId);
  }
  else
  {
    // send failure response with invalid NV ID
    memset(&rsp[1], 0xFF, 2);
  }

  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_UTIL), cmdId,
                                       MT_APSME_LINKKEY_NV_ID_GET_RSP_LEN, rsp);
}
#endif // MT_SYS_KEY_MANAGEMENT

/***************************************************************************************************
 * @fn      MT_UtilAPSME_RequestKeyCmd
 *
 * @brief   Send RequestKey command message to TC for a specific partner Address.
 *
 * @param   pBuf  - pointer to the received buffer
 *
 * @return  void
 ***************************************************************************************************/
void MT_UtilAPSME_RequestKeyCmd(uint8_t *pBuf)
{
  uint8_t cmdId;
  uint8_t partnerAddr[Z_EXTADDR_LEN];
  uint8_t retValue;

  // parse header
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  /* PartnerAddress */
  OsalPort_memcpy(partnerAddr, pBuf, Z_EXTADDR_LEN);

  retValue = (uint8_t)ZDSecMgrRequestAppKey(partnerAddr);

  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_UTIL), cmdId, 1, &retValue);
}

/***************************************************************************************************
 * @fn      MT_UtilAssocCount
 *
 * @brief   Proxy the AssocCount() function.
 *
 * @param   pBuf - pointer to the received buffer
 *
 * @return  void
 ***************************************************************************************************/
static void MT_UtilAssocCount(uint8_t *pBuf)
{
  uint16_t cnt;
  uint8_t cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  cnt = AssocCount(pBuf[0], pBuf[1]);
  pBuf[0] = LO_UINT16(cnt);
  pBuf[1] = HI_UINT16(cnt);

  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_UTIL), cmdId, 2, pBuf);
}

/***************************************************************************************************
 * @fn      MT_UtilAssocFindDevice
 *
 * @brief   Get an associated device by index.
 *
 * @param   pBuf - pointer to the received buffer
 *
 * @return  void
 ***************************************************************************************************/
static void MT_UtilAssocFindDevice(uint8_t *pBuf)
{
  uint8_t cmdId = pBuf[MT_RPC_POS_CMD1];
  uint8_t buf[sizeof(associated_devices_t)];

  packDev_t(buf, AssocFindDevice(pBuf[MT_RPC_FRAME_HDR_SZ]));
  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_UTIL), cmdId,
                                       sizeof(associated_devices_t), buf);
}

/***************************************************************************************************
 * @fn      MT_UtilAssocGetWithAddress
 *
 * @brief   Get an associated device by address.
 *
 * @param   pBuf - pointer to the received buffer
 *
 * @return  void
 ***************************************************************************************************/
static void MT_UtilAssocGetWithAddress(uint8_t *pBuf)
{
  extern associated_devices_t *AssocGetWithAddress(uint8_t *extAddr, uint16_t shortAddr);
  uint8_t cmdId = pBuf[MT_RPC_POS_CMD1];
  uint8_t buf[sizeof(associated_devices_t)];

  pBuf += MT_RPC_FRAME_HDR_SZ;
  packDev_t(buf, AssocGetWithAddress(((AddrMgrExtAddrValid(pBuf)) ? pBuf : NULL),
                                  BUILD_UINT16(pBuf[Z_EXTADDR_LEN], pBuf[Z_EXTADDR_LEN+1])));

  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_UTIL), cmdId,
                                       sizeof(associated_devices_t), buf);
}

/***************************************************************************************************
 * @fn      MT_UtilBindAddEntry
 *
 * @brief   Add Binding Entry into Local Table.
 *
 * @param   pBuf - pointer to the received buffer
 *
 * @return  void
 ***************************************************************************************************/
static void MT_UtilBindAddEntry(uint8_t *pBuf)
{
  uint8_t srcEp;
  zAddrType_t dstAddr;
  uint8_t dstEp;
  uint8_t numClusterIds;
  uint16_t *clusterIds;
  uint8_t buf[sizeof(BindingEntry_t)];
  uint8_t cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  // Initialize the return buffer
  memset( buf, 0xFF, sizeof(BindingEntry_t) );
  buf[2] = 0xFE;    // set the default value of INVALID_NODE_ADDR
  buf[3] = 0xFF;    // set the default value of INVALID_NODE_ADDR

  srcEp = *pBuf++;

  // Destination Address mode
  dstAddr.addrMode = *pBuf++;

  // Destination Address
  if ( dstAddr.addrMode == Addr64Bit )
  {
    uint8_t *ptr; // Use this additional pointer because *pBuf is incremented later for both cases

    ptr = pBuf;
    osal_cpyExtAddr( dstAddr.addr.extAddr, ptr );
  }
  else
  {
    dstAddr.addr.shortAddr = OsalPort_buildUint16( pBuf );
  }
  // The short address occupies LSB two bytes
  pBuf += Z_EXTADDR_LEN;

  // DstEPInt
  dstEp = *pBuf++;

  numClusterIds = *pBuf++;

  if ( numClusterIds > 0 )
  {
    // copy list of clusters
    clusterIds = (uint16_t *)OsalPort_malloc( numClusterIds * sizeof(uint16_t) );

    if ( clusterIds != NULL )
    {
      OsalPort_memcpy( clusterIds, pBuf, numClusterIds * sizeof(uint16_t));
      // The response to MT interface has to be pack into buf
      packBindEntry_t( buf, bindAddEntry( srcEp, &dstAddr, dstEp, numClusterIds, clusterIds ));

      OsalPort_free( clusterIds );
    }
  }

  MT_BuildAndSendZToolResponse( ( (uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_UTIL ),
                                cmdId, sizeof(BindingEntry_t), buf );
}

/***************************************************************************************************
 * @fn      packDev_t
 *
 * @brief   Pack an associated_devices_t structure into a byte buffer (pack INVALID_NODE_ADDR if
 *          the pDev parameter is NULL).
 *
 * @param   pBuf - pointer to the buffer into which to pack the structure.
 * @param   pDev - pointer to the structure.
 *
 * @return  void
 ***************************************************************************************************/
static void packDev_t(uint8_t *pBuf, associated_devices_t *pDev)
{
  if (NULL == pDev)
  {
    uint16_t rtrn = INVALID_NODE_ADDR;
    *pBuf++ = LO_UINT16(rtrn);
    *pBuf++ = HI_UINT16(rtrn);
  }
  else
  {
    *pBuf++ = LO_UINT16(pDev->shortAddr);
    *pBuf++ = HI_UINT16(pDev->shortAddr);
    *pBuf++ = LO_UINT16(pDev->addrIdx);
    *pBuf++ = HI_UINT16(pDev->addrIdx);
    *pBuf++ = pDev->nodeRelation;
    *pBuf++ = pDev->devStatus;
    *pBuf++ = pDev->assocCnt;
    *pBuf++ = pDev->age;
    *pBuf++ = pDev->linkInfo.txCounter;
    *pBuf++ = pDev->linkInfo.txCost;
    *pBuf++ = pDev->linkInfo.rxLqi;
    *pBuf++ = pDev->linkInfo.inKeySeqNum;
    OsalPort_bufferUint32( pBuf, pDev->linkInfo.inFrmCntr );
    *pBuf += 4;
    *pBuf++ = LO_UINT16(pDev->linkInfo.txFailure);
    *pBuf++ = HI_UINT16(pDev->linkInfo.txFailure);
  }
}

/***************************************************************************************************
 * @fn      packBindEntry_t
 *
 * @brief   Pack a BindingEntry_t structure into a byte buffer (pack INVALID_NODE_ADDR
 *          as dstIdx if the pBind parameter is NULL).
 *
 * @param   pBuf - pointer to the buffer into which to pack the structure.
 * @param   pBind - pointer to the structure.
 *
 * @return  void
 ***************************************************************************************************/
static void packBindEntry_t(uint8_t *pBuf, BindingEntry_t *pBind)
{
  if ( NULL == pBind )
  {
    uint16_t rtrn = INVALID_NODE_ADDR;
    *pBuf++ = 0xFF;
    *pBuf++ = 0xFF;
    *pBuf++ = LO_UINT16(rtrn);
    *pBuf++ = HI_UINT16(rtrn);
    *pBuf++ = 0xFF;
    *pBuf++ = 0xFF;

  }
  else
  {
    *pBuf++ = pBind->srcEP;
    *pBuf++ = pBind->dstGroupMode;
    *pBuf++ = LO_UINT16( pBind->dstIdx );
    *pBuf++ = HI_UINT16( pBind->dstIdx );
    *pBuf++ = pBind->dstEP;
    *pBuf++ = pBind->numClusterIds;

    OsalPort_memcpy( pBuf, pBind->clusterIdList, pBind->numClusterIds * sizeof(uint16_t));
  }
}

/***************************************************************************************************
 * @fn      MT_UtilSync
 *
 * @brief   Process the MT_UTIL_SYNC command
 *
 * @param   None
 *
 * @return  None
 ***************************************************************************************************/
static void MT_UtilSync(void)
{
 MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_AREQ|(uint8_t)MT_RPC_SYS_UTIL),MT_UTIL_SYNC_REQ,0,0);
}
#endif /* !defined NONWK */
#endif /* MT_UTIL_FUNC */
/**************************************************************************************************
 **************************************************************************************************/
