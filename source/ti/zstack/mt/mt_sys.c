/******************************************************************************
  Filename:       MT_SYS.c
  Revised:        $Date: 2015-02-09 19:10:05 -0800 (Mon, 09 Feb 2015) $
  Revision:       $Revision: 42469 $

  Description:   MonitorTest functions for SYS commands.

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

 *****************************************************************************/

/******************************************************************************
 * INCLUDES
 *****************************************************************************/
#include "zcomdef.h"
#include "rom_jt_154.h"
#include "mt.h"
#include "mt_sys.h"
#include "mt_version.h"
#include "osal_nv.h"
#include "mac_low_level.h"
#include "zmac.h"

#include <driverlib/sys_ctrl.h>

#if !defined( CC253X_MACNP )
  #include "zglobals.h"
#endif
#if defined( FEATURE_NVEXID )
  #include "zstackconfig.h"
#endif
#if defined( FEATURE_SYSTEM_STATS )
#include "zdiags.h"
#endif

#ifdef FEATURE_UTC_TIME
  #include "utc_clock.h"
#endif //FEATURE_UTC_TIME

#if defined( ENABLE_MT_SYS_RESET_SHUTDOWN )
#include "mac_rx.h"
#include "mac_radio_defs.h"
#include <ti/drivers/power/PowerCC26XX.h>
#endif /* ENABLE_MT_SYS_RESET_SHUTDOWN */

/******************************************************************************
 * MACROS
 *****************************************************************************/

/* RPC_CMD responses for MT_SYS commands */
#define MT_ARSP_SYS ((uint8_t)MT_RPC_CMD_AREQ | (uint8_t)MT_RPC_SYS_SYS)
#define MT_SRSP_SYS ((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_SYS)

/* Max possible MT response length, limited by TX buffer and sizeof uint8_t */
#define MT_MAX_RSP_LEN  255

/* Max possible MT response data length, MT protocol overhead */
#define MT_MAX_RSP_DATA_LEN  ( (MT_MAX_RSP_LEN - 1) - SPI_0DATA_MSG_LEN )

#define MT_SYS_DEVICE_INFO_RESPONSE_LEN 14

#if !defined HAL_GPIO || !HAL_GPIO
#define GPIO_DIR_IN(IDX)
#define GPIO_DIR_OUT(IDX)
#define GPIO_TRI(IDX)
#define GPIO_PULL_UP(IDX)
#define GPIO_PULL_DN(IDX)
#define GPIO_SET(IDX)
#define GPIO_CLR(IDX)
#define GPIO_TOG(IDX)
#define GPIO_GET(IDX) 0
#define GPIO_HiD_SET() (val = 0)
#define GPIO_HiD_CLR() (val = 0)
#endif

#define RESET_HARD     0
#define RESET_SOFT     1
#define RESET_SHUTDOWN 2

/******************************************************************************
 * CONSTANTS
 *****************************************************************************/

#if !defined( MT_SYS_OSAL_NV_READ_CERTIFICATE_DATA )
#define MT_SYS_OSAL_NV_READ_CERTIFICATE_DATA  FALSE
#endif

#if defined( MT_SYS_FUNC )
static const uint16_t MT_SysOsalEventId[] =
{
  MT_SYS_OSAL_EVENT_0,
  MT_SYS_OSAL_EVENT_1,
  MT_SYS_OSAL_EVENT_2,
  MT_SYS_OSAL_EVENT_3
};
#endif

typedef enum {
  GPIO_DIR,
  GPIO_TRI,
  GPIO_SET,
  GPIO_CLR,
  GPIO_TOG,
  GPIO_GET,
  GPIO_HiD = 0x12
} GPIO_Op_t;

/******************************************************************************
 * EXTERNAL VARIABLES
 *****************************************************************************/
#if defined( FEATURE_NVEXID )
extern zstack_Config_t *pZStackCfg;
#endif /* FEATURE_NVEXID */

/******************************************************************************
 * LOCAL VARIABLES
 *****************************************************************************/

/******************************************************************************
 * LOCAL FUNCTIONS
 *****************************************************************************/
#if defined( MT_SYS_FUNC )
static void MT_SysReset(uint8_t *pBuf);
static void MT_SysPing(void);
static void MT_SysVersion(void);
static void MT_SysSetExtAddr(uint8_t *pBuf);
static void MT_SysGetExtAddr(void);
static void MT_SysOsalStartTimer(uint8_t *pBuf);
static void MT_SysOsalStopTimer(uint8_t *pBuf);
static void MT_SysRandom(void);
static void MT_SysGpio(uint8_t *pBuf);
static void MT_SysStackTune(uint8_t *pBuf);
#ifdef FEATURE_UTC_TIME
static void MT_SysSetUtcTime(uint8_t *pBuf);
static void MT_SysGetUtcTime(void);
#endif //FEATURE_UTC_TIME
static void MT_SysSetTxPower(uint8_t *pBuf);
#if !defined( CC26XX ) \
    && !defined (DeviceFamily_CC26X1) \
    && !defined (DeviceFamily_CC26X2) \
    && !defined (DeviceFamily_CC13X2) \
    && !defined (DeviceFamily_CC26X2X7) \
    && !defined (DeviceFamily_CC13X2X7) \
    && !defined (DeviceFamily_CC26X4) \
    && !defined (DeviceFamily_CC13X4) \
    && !defined (DeviceFamily_CC26X3)
static void MT_SysAdcRead(uint8_t *pBuf);
#endif /* !CC26xx */
#if !defined( CC253X_MACNP )
static void MT_SysOsalNVItemInit(uint8_t *pBuf);
static void MT_SysOsalNVDelete(uint8_t *pBuf);
static void MT_SysOsalNVLength(uint8_t *pBuf);
static void MT_SysOsalNVRead(uint8_t *pBuf);
static void MT_SysOsalNVWrite(uint8_t *pBuf);
static uint8_t MT_CheckNvId(uint16_t nvId);
#if defined( FEATURE_NVEXID )
static void MT_SysNvCompact(uint8_t *pBuf);
static void MT_SysNvCreate(uint8_t *pBuf);
static void MT_SysNvDelete(uint8_t *pBuf);
static void MT_SysNvLength(uint8_t *pBuf);
static void MT_SysNvRead(uint8_t *pBuf);
static void MT_SysNvWrite(uint8_t *pBuf);
static uint8_t MT_StackNvExtId( NVINTF_itemID_t *nvId );
static uint8_t *MT_ParseNvExtId( uint8_t *pBuf, NVINTF_itemID_t *nvId );
#endif /* FEATURE_NVEXID */
#endif /* !CC253X_MACNP */
#if defined( FEATURE_SYSTEM_STATS )
static void MT_SysZDiagsInitStats(void);
static void MT_SysZDiagsClearStats(uint8_t *pBuf);
static void MT_SysZDiagsGetStatsAttr(uint8_t *pBuf);
static void MT_SysZDiagsRestoreStatsFromNV(void);
static void MT_SysZDiagsSaveStatsToNV(void);
#endif /* FEATURE_SYSTEM_STATS */
#if defined( ENABLE_MT_SYS_RESET_SHUTDOWN )
static void powerOffSoc(void);
#endif /* ENABLE_MT_SYS_RESET_SHUTDOWN */
#endif /* MT_SYS_FUNC */

#if defined( MT_SYS_FUNC )
/******************************************************************************
 * @fn      MT_SysProcessing
 *
 * @brief   Process all the SYS commands that are issued by test tool
 *
 * @param   pBuf - pointer to the msg buffer
 *
 *          | LEN  | CMD0  | CMD1  |  DATA  |
 *          |  1   |   1   |   1   |  0-255 |
 *
 * @return  status
 *****************************************************************************/
uint8_t MT_SysCommandProcessing(uint8_t *pBuf)
{
  uint8_t status = MT_RPC_SUCCESS;

  switch (pBuf[MT_RPC_POS_CMD1])
  {
    case MT_SYS_RESET_REQ:
      MT_SysReset(pBuf);
      break;

    case MT_SYS_PING:
      MT_SysPing();
      break;

    case MT_SYS_VERSION:
      MT_SysVersion();
      break;

    case MT_SYS_SET_EXTADDR:
      MT_SysSetExtAddr(pBuf);
      break;

    case MT_SYS_GET_EXTADDR:
      MT_SysGetExtAddr();
      break;

    case MT_SYS_OSAL_START_TIMER:
      MT_SysOsalStartTimer(pBuf);
      break;

    case MT_SYS_OSAL_STOP_TIMER:
      MT_SysOsalStopTimer(pBuf);
      break;

    case MT_SYS_RANDOM:
      MT_SysRandom();
      break;

#if !defined( CC26XX ) \
    && !defined (DeviceFamily_CC26X1) \
    && !defined (DeviceFamily_CC26X2) \
    && !defined (DeviceFamily_CC13X2) \
    && !defined (DeviceFamily_CC26X2X7) \
    && !defined (DeviceFamily_CC13X2X7) \
    && !defined (DeviceFamily_CC26X4) \
    && !defined (DeviceFamily_CC13X4) \
    && !defined (DeviceFamily_CC26X3)
      case MT_SYS_ADC_READ:
      MT_SysAdcRead(pBuf);
      break;
#endif /* !CC26XX */

    case MT_SYS_GPIO:
      MT_SysGpio(pBuf);
      break;

    case MT_SYS_STACK_TUNE:
      MT_SysStackTune(pBuf);
      break;
#ifdef FEATURE_UTC_TIME
    case MT_SYS_SET_TIME:
      MT_SysSetUtcTime(pBuf);
      break;

    case MT_SYS_GET_TIME:
      MT_SysGetUtcTime();
      break;
#endif // FEATURE_UTC_TIME
    case MT_SYS_SET_TX_POWER:
      MT_SysSetTxPower(pBuf);
      break;

// CC253X MAC Network Processor does not have NV support
#if !defined( CC253X_MACNP )
    case MT_SYS_OSAL_NV_DELETE:
      MT_SysOsalNVDelete(pBuf);
      break;

    case MT_SYS_OSAL_NV_ITEM_INIT:
      MT_SysOsalNVItemInit(pBuf);
      break;

    case MT_SYS_OSAL_NV_LENGTH:
      MT_SysOsalNVLength(pBuf);
      break;

    case MT_SYS_OSAL_NV_READ:
    case MT_SYS_OSAL_NV_READ_EXT:
      MT_SysOsalNVRead(pBuf);
      break;

    case MT_SYS_OSAL_NV_WRITE:
    case MT_SYS_OSAL_NV_WRITE_EXT:
      MT_SysOsalNVWrite(pBuf);
      break;

#if defined( FEATURE_NVEXID )
    case MT_SYS_NV_COMPACT:
      MT_SysNvCompact(pBuf);
      break;

    case MT_SYS_NV_CREATE:
      MT_SysNvCreate(pBuf);
      break;

    case MT_SYS_NV_DELETE:
      MT_SysNvDelete(pBuf);
      break;

    case MT_SYS_NV_LENGTH:
      MT_SysNvLength(pBuf);
      break;

    case MT_SYS_NV_READ:
      MT_SysNvRead(pBuf);
      break;

    case MT_SYS_NV_WRITE:
    case MT_SYS_NV_UPDATE:
      MT_SysNvWrite(pBuf);
      break;
#endif  /* FEATURE_NVEXID */
#endif  /* !CC253X_MACNP */

#if defined( FEATURE_SYSTEM_STATS )
    case MT_SYS_ZDIAGS_INIT_STATS:
      MT_SysZDiagsInitStats();
      break;

    case MT_SYS_ZDIAGS_CLEAR_STATS:
      MT_SysZDiagsClearStats(pBuf);
      break;

    case MT_SYS_ZDIAGS_GET_STATS:
      MT_SysZDiagsGetStatsAttr(pBuf);
       break;

    case MT_SYS_ZDIAGS_RESTORE_STATS_NV:
      MT_SysZDiagsRestoreStatsFromNV();
      break;

    case MT_SYS_ZDIAGS_SAVE_STATS_TO_NV:
      MT_SysZDiagsSaveStatsToNV();
      break;
#endif /* FEATURE_SYSTEM_STATS */

    default:
      status = MT_RPC_ERR_COMMAND_ID;
      break;
  }

  return status;
}

/******************************************************************************
 * @fn      MT_SysReset
 *
 * @brief   Reset the device.
 * @param   typID: 0=reset, 1=serial bootloader,
 *
 * @return  None
 *****************************************************************************/
void MT_SysReset( uint8_t *pBuf )
{
  switch( pBuf[MT_RPC_POS_DAT0] )
  {
    case MT_SYS_RESET_HARD:
        SysCtrlSystemReset();
      break;

    case MT_SYS_RESET_SOFT:
#if !defined( HAL_BOARD_F5438 )
        SysCtrlSystemReset();
#endif
      break;

    case MT_SYS_RESET_SHUTDOWN:
      {
#if defined( ENABLE_MT_SYS_RESET_SHUTDOWN )
        // Disable interrupts and put into deep sleep, use hardware reset to wakeup
        powerOffSoc();
#endif
      }
      break;
  }
}

/******************************************************************************
 * @fn      MT_SysPing
 *
 * @brief   Process the Ping command
 *
 * @param   None
 *
 * @return  None
 *****************************************************************************/
static void MT_SysPing(void)
{
  uint16_t tmp16;
  uint8_t retArray[2];

  /* Build Capabilities */
  tmp16 = MT_CAP_SYS | MT_CAP_MAC  | MT_CAP_NWK  | MT_CAP_AF    |
          MT_CAP_ZDO | MT_CAP_SAPI | MT_CAP_UTIL | MT_CAP_DEBUG |
          MT_CAP_APP | MT_CAP_GP   | MT_CAP_ZOAD | MT_CAP_APP_CNF;

  /* Convert to high byte first into temp buffer */
  retArray[0] = LO_UINT16( tmp16 );
  retArray[1] = HI_UINT16( tmp16 );

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse( MT_SRSP_SYS, MT_SYS_PING,
                                sizeof(retArray), retArray );
}

/******************************************************************************
 * @fn      MT_SysVersion
 *
 * @brief   Process the Version command
 *
 * @param   None
 *
 * @return  None
 *****************************************************************************/
static void MT_SysVersion(void)
{
  /* Build and send back the default response */
  MT_BuildAndSendZToolResponse( MT_SRSP_SYS, MT_SYS_VERSION,
                                sizeof(MTVersionString),(uint8_t*)MTVersionString);
}

/******************************************************************************
 * @fn      MT_SysSetExtAddr
 *
 * @brief   Set the Extended Address
 *
 * @param   pBuf
 *
 * @return  None
 *****************************************************************************/
static void MT_SysSetExtAddr(uint8_t *pBuf)
{
  uint8_t retValue = ZFailure;

  /* Skip over RPC header */
  pBuf += MT_RPC_FRAME_HDR_SZ;

  if ( ZMacSetReq(ZMacExtAddr, pBuf) == ZMacSuccess )
  {
// CC253X MAC Network Processor does not have NV support
#if defined(CC253X_MACNP)
    retValue = ZSuccess;
#else
    retValue = osal_nv_write(ZCD_NV_EXTADDR, Z_EXTADDR_LEN, pBuf);
#endif
  }

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse( MT_SRSP_SYS, MT_SYS_SET_EXTADDR,
                                sizeof(retValue), &retValue);
}

/******************************************************************************
 * @fn      MT_SysGetExtAddr
 *
 * @brief   Get the Extended Address
 *
 * @param   None
 *
 * @return  None
 *****************************************************************************/
static void MT_SysGetExtAddr(void)
{
  uint8_t extAddr[Z_EXTADDR_LEN];

  ZMacGetReq( ZMacExtAddr, extAddr );

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse( MT_SRSP_SYS, MT_SYS_GET_EXTADDR,
                                sizeof(extAddr), extAddr);
}

#if !defined( CC253X_MACNP )
/******************************************************************************
 * @fn      MT_CheckNvId
 *
 * @brief   Check whether (ZigBee Stack) NV read should be blocked
 *
 * @param   nvId - NV item ID
 *
 * @return  'ZInvalidParameter' if blocked, otherwise 'ZSuccess'
 *****************************************************************************/
static uint8_t MT_CheckNvId( uint16_t nvId )
{
#if !MT_SYS_OSAL_NV_READ_CERTIFICATE_DATA
  if ((ZCD_NV_IMPLICIT_CERTIFICATE == nvId) ||
      (ZCD_NV_CA_PUBLIC_KEY == nvId)        ||
      (ZCD_NV_DEVICE_PRIVATE_KEY == nvId))
  {
    /* Access to Security Certificate Data is denied */
    return( ZInvalidParameter );
  }
#endif  /* MT_SYS_OSAL_NV_READ_CERTIFICATE_DATA */

#if !MT_SYS_KEY_MANAGEMENT
  if ( (nvId == ZCD_NV_NWK_ACTIVE_KEY_INFO) ||
       (nvId == ZCD_NV_NWK_ALTERN_KEY_INFO) ||
       (nvId == ZCD_NV_PRECFGKEY) )
  {
    /* Access to Security Key Data is denied */
    return( ZInvalidParameter );
  }
#endif  /* !MT_SYS_KEY_MANAGEMENT */

  return( ZSuccess );
}

/******************************************************************************
 * @fn      MT_SysOsalNVRead
 *
 * @brief   Attempt to read an NV value
 *
 * @param   pBuf - pointer to the data
 *
 * @return  None
 *****************************************************************************/
static void MT_SysOsalNVRead(uint8_t *pBuf)
{
  uint8_t error;
  uint8_t cmdId;
  uint16_t nvId;
  uint16_t dataLen;
  uint16_t dataOfs;
  uint16_t nvItemLen;

  /* MT command ID */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  /* Skip over RPC header */
  pBuf += MT_RPC_FRAME_HDR_SZ;

  /* NV item ID */
  nvId = OsalPort_buildUint16( pBuf );

#if defined( ZCD_NV_POLL_RATE_OLD16 )
  if( nvId == ZCD_NV_POLL_RATE_OLD16 )
  {
    // This ID shouldn't exist anymore, it was converted to the new size and ID
    // then deleted during initialization. But, a read of this item will
    // read the new item and convert the size and return the size expected.
    uint32_t pollRate;
    uint16_t *pOldPollRate;
    uint8_t respBuf[4];
    uint8_t respLen;
    // Convert from old uint16_t NV item to the new uint32_t NV item
    if ( osal_nv_read( ZCD_NV_POLL_RATE, 0, sizeof( uint32_t ), &pollRate ) == ZSUCCESS )
    {
      respBuf[0] = ZSuccess;
      respBuf[1] = sizeof( uint16_t );
      pOldPollRate = (uint16_t *)&respBuf[2];
      *pOldPollRate = (uint16_t)pollRate;
      respLen = 4;
    }
    else
    {
      respBuf[0] = ZFailure;
      respBuf[1] = 0;
      respLen = 2;
    }
    /* Build and send back the response */
    MT_BuildAndSendZToolResponse( MT_SRSP_SYS, MT_SYS_OSAL_NV_READ,
                                  respLen, respBuf );
    return;
  }
#endif

  /* Check whether read-access to this item is allowed */
  error = MT_CheckNvId( nvId );

  /* Get NV data offset */
  if( cmdId == MT_SYS_OSAL_NV_READ )
  {
    /* MT_SYS_OSAL_NV_READ has 1-byte offset */
    dataOfs = (uint16_t)pBuf[2];
  }
  else
  {
    /* MT_SYS_OSAL_NV_READ_EXT has 2-byte offset */
    dataOfs = OsalPort_buildUint16( pBuf+2 );
  }

  /* Length of entire NV item data */
  nvItemLen = osal_nv_item_len( nvId );
  if( nvItemLen <= dataOfs )
  {
    /* Offset is past end of data */
    error = ZInvalidParameter;
  }

  if( error == ZSuccess )
  {
    uint8_t *pRetBuf;
    uint8_t respLen = 2;  /* Response header: [0]=status,[1]=length */

    dataLen = nvItemLen - dataOfs;
    if (dataLen > (uint16_t)(MT_MAX_RSP_DATA_LEN - respLen))
    {
      /* Data length is limited by TX buffer size and MT protocol */
      dataLen = (MT_MAX_RSP_DATA_LEN - respLen);
    }
    respLen += dataLen;

    pRetBuf = OsalPort_malloc(respLen);
    if( pRetBuf != NULL )
    {
      memset(&pRetBuf[2], 0, dataLen);
      if (((osal_nv_read( nvId, dataOfs, dataLen, &pRetBuf[2] )) == ZSUCCESS))
      {
        pRetBuf[0] = ZSuccess;
        pRetBuf[1] = dataLen;
        MT_BuildAndSendZToolResponse( MT_SRSP_SYS, cmdId,
                                      respLen, pRetBuf );
      }
      else
      {
        error = NV_OPER_FAILED;
      }
      OsalPort_free(pRetBuf);
    }
    else
    {
      /* Could not get buffer for NV data */
      error = ZMemError;
    }
  }

  if( error != ZSuccess )
  {
    uint8_t tmp[2] = { error, 0 };
    MT_BuildAndSendZToolResponse( MT_SRSP_SYS, cmdId,
                                  sizeof(tmp), tmp);
  }
}

/******************************************************************************
 * @fn      MT_SysOsalNVWrite
 *
 * @brief   Attempt to write an NV item
 *
 * @param   pBuf - pointer to the data
 *
 * @return  None
 *****************************************************************************/
static void MT_SysOsalNVWrite(uint8_t *pBuf)
{
  uint8_t cmdId;
  uint16_t nvId;
  uint16_t dataLen;
  uint16_t dataOfs;
  uint16_t nvItemLen;
  uint8_t rtrn = ZSuccess;

  /* MT command ID */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  /* Skip over RPC header */
  pBuf += MT_RPC_FRAME_HDR_SZ;

  /* NV item ID */
  nvId = OsalPort_buildUint16( pBuf );

  /* Get NV data offset & length */
  if ( cmdId == MT_SYS_OSAL_NV_WRITE )
  {
    /* MT_SYS_OSAL_NV_WRITE has 1-byte offset & length */
    dataOfs = (uint16_t)pBuf[2];
    dataLen = (uint16_t)pBuf[3];
    pBuf += 4;
  }
  else
  {
    /* MT_SYS_OSAL_NV_WRITE_EXT has 2-byte offset & length */
    dataOfs = OsalPort_buildUint16( pBuf+2 );
    dataLen = OsalPort_buildUint16( pBuf+4 );
    pBuf += 6;
  }

#if defined ( ZCD_NV_POLL_RATE_OLD16 )
  if ( nvId == ZCD_NV_POLL_RATE_OLD16 )
  {
    // This ID shouldn't exist anymore, it was converted to the new size and ID
    // then deleted during initialization.  But a write to this item will
    // convert the 16 bits to the new 32 bits and write that value to the new
    // NV item.
    uint32_t pollRate;
    uint16_t *pOldPollRate = (uint16_t *)pBuf;
    uint16_t oldPollRate = *pOldPollRate;
    nvId = ZCD_NV_POLL_RATE;
    nvItemLen = sizeof ( uint32_t );
    pollRate = (uint32_t)oldPollRate;
    pBuf = (uint8_t *)&pollRate;
  }
#endif

  /* Length of entire NV item data */
  nvItemLen = osal_nv_item_len(nvId);
  if ((dataOfs + dataLen) <= nvItemLen)
  {
    if (dataOfs == 0)
    {
      /* Set the Z-Globals value of this NV item */
      zgSetItem( nvId, dataLen, pBuf );
    }

    if ((osal_nv_write(nvId, dataLen, pBuf)) == ZSUCCESS)
    {
      if (nvId == ZCD_NV_EXTADDR)
      {
        rtrn = ZMacSetReq(ZMacExtAddr, pBuf);
      }
    }
    else
    {
      rtrn = NV_OPER_FAILED;
    }
  }
  else
  {
    /* Bad length or/and offset */
    rtrn = ZInvalidParameter;
  }

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse( MT_SRSP_SYS, cmdId,
                                sizeof(rtrn), &rtrn);
}

/******************************************************************************
 * @fn      MT_SysOsalNVItemInit
 *
 * @brief   Attempt to create an NV item
 *
 * @param   pBuf - pointer to the data
 *
 * @return  None
 *****************************************************************************/
static void MT_SysOsalNVItemInit(uint8_t *pBuf)
{
  uint8_t ret;
  uint8_t idLen;
  uint16_t nvId;
  uint16_t nvLen;

  /* Skip over RPC header */
  pBuf += MT_RPC_FRAME_HDR_SZ;

  /* NV item ID */
  nvId = OsalPort_buildUint16( pBuf );
  /* NV item length */
  nvLen = OsalPort_buildUint16( pBuf+2 );
  /* Initialization data length */
  idLen = pBuf[4];
  pBuf += 5;

#if defined( ZCD_NV_POLL_RATE_OLD16 )
  if ( nvId == ZCD_NV_POLL_RATE_OLD16 )
  {
    /* This item shouldn't exist anymore.  Read and write will convert
     * to the new NV item, so return Success.
     */
    ret = ZSuccess;
  }
  else
#endif
  {
    if ( idLen < nvLen )
    {
      /* Attempt to create a new NV item */
      ret = osal_nv_item_init( nvId, nvLen, NULL );
      if ( (ret == NV_ITEM_UNINIT) && (idLen > 0) )
      {
        /* Write initialization data to first part of new item */
        (void) osal_nv_write( nvId, (uint16_t)idLen, pBuf );
      }
    }
    else
    {
      /* Attempt to create/initialize a new NV item */
      ret = osal_nv_item_init( nvId, nvLen, pBuf );
    }
  }

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse( MT_SRSP_SYS, MT_SYS_OSAL_NV_ITEM_INIT,
                                sizeof(ret), &ret);
}

/******************************************************************************
 * @fn      MT_SysOsalNVDelete
 *
 * @brief   Attempt to delete an NV item
 *
 * @param   pBuf - pointer to the data
 *
 * @return  None
 *****************************************************************************/
static void MT_SysOsalNVDelete(uint8_t *pBuf)
{
  uint16_t nvId;
  uint16_t nvLen;
  uint8_t ret;

  /* Skip over RPC header */
  pBuf += MT_RPC_FRAME_HDR_SZ;

  /* Get the ID */
  nvId = OsalPort_buildUint16( pBuf );
  /* Get the length */
  nvLen = OsalPort_buildUint16( pBuf+2 );

#if defined ( ZCD_NV_POLL_RATE_OLD16 )
  if ( nvId == ZCD_NV_POLL_RATE_OLD16 )
  {
    /* This item shouldn't exist anymore.  Read and write will convert
     * to the new NV item, so return Success.
     */
    ret = ZSuccess;
  }
  else
#endif
  {
    /* Attempt to delete the NV item */
    ret = osal_nv_delete( nvId, nvLen );
  }

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse( MT_SRSP_SYS, MT_SYS_OSAL_NV_DELETE,
                                sizeof(ret), &ret);
}

/******************************************************************************
 * @fn      MT_SysOsalNVLength
 *
 * @brief   Attempt to get the length to an NV item
 *
 * @param   pBuf - pointer to the data
 *
 * @return  None
 *****************************************************************************/
static void MT_SysOsalNVLength(uint8_t *pBuf)
{
  uint16_t nvId;
  uint16_t nvLen;
  uint8_t rsp[2];

  /* Skip over RPC header */
  pBuf += MT_RPC_FRAME_HDR_SZ;

  /* Get the ID */
  nvId = OsalPort_buildUint16( pBuf );

#if defined ( ZCD_NV_POLL_RATE_OLD16 )
  if ( nvId == ZCD_NV_POLL_RATE_OLD16 )
  {
    /* Ignore this item and force return  */
    nvLen = 0;
  }
  else
#endif
  {
    /* Attempt to get NV item length */
    nvLen = osal_nv_item_len( nvId );
  }

  rsp[0] = LO_UINT16( nvLen );
  rsp[1] = HI_UINT16( nvLen );

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse( MT_SRSP_SYS, MT_SYS_OSAL_NV_LENGTH,
                                sizeof(rsp), rsp);
}

#if defined( FEATURE_NVEXID )
/******************************************************************************
 * @fn      MT_ParseNvExtId
 *
 * @brief   Parse the incoming NV ID parameters
 *
 * @param   pBuf - pointer to incoming data
 * @param   nvId - pointer to outgoing NV ID
 *
 * @return  pointer to next incoming data byte
 *****************************************************************************/
static uint8_t *MT_ParseNvExtId( uint8_t *pBuf, NVINTF_itemID_t *nvId )
{
  /* Skip over RPC header */
  pBuf += MT_RPC_FRAME_HDR_SZ;

  nvId->systemID = pBuf[0];
  nvId->itemID = OsalPort_buildUint16( pBuf+1 );
  nvId->subID = OsalPort_buildUint16( pBuf+3 );

  return( pBuf + 5 );
}

/******************************************************************************
 * @fn      MT_StackNvExtId
 *
 * @brief   Check whether extended NV ID is from ZigBee Stack
 *
 * @param   nvId - pointer to extended NV ID
 *
 * @return  TRUE if ZigBee Stack NV item, otherwise FALSE
 *****************************************************************************/
static uint8_t MT_StackNvExtId( NVINTF_itemID_t *nvId )
{
  return( (nvId->systemID == NVINTF_SYSID_ZSTACK) && (nvId->itemID == 0) );
}

/******************************************************************************
 * @fn      MT_SysNvCompact
 *
 * @brief   Attempt to compact the active NV page
 *
 * @param   pBuf - pointer to the data
 *
 * @return  None
 *****************************************************************************/
static void MT_SysNvCompact(uint8_t *pBuf)
{
  uint8_t retVal;

  if (( pZStackCfg == NULL ) || ( pZStackCfg->nvFps.compactNV == NULL ))
  {
    /* NV item compact function not available */
    retVal = NVINTF_NOTREADY;
  }
  else
  {
    uint16_t minSize;

    /* Skip over RPC header */
    pBuf += MT_RPC_FRAME_HDR_SZ;

    /* Get the remaining size threshold */
    minSize = OsalPort_buildUint16( pBuf );

    /* Attempt to compact the active NV page */
    retVal = pZStackCfg->nvFps.compactNV( minSize );
  }

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse( MT_SRSP_SYS, MT_SYS_NV_COMPACT,
                                sizeof(retVal), &retVal);
}

/******************************************************************************
 * @fn      MT_SysNvCreate
 *
 * @brief   Attempt to create an NV item (extended item ID)
 *
 * @param   pBuf - pointer to the data
 *
 * @return  None
 *****************************************************************************/
static void MT_SysNvCreate(uint8_t *pBuf)
{
  uint8_t retVal = NVINTF_FAILURE;

  if(( pZStackCfg == NULL ) || ( pZStackCfg->nvFps.createItem == NULL ))
  {
    /* NV item create function not available */
    retVal = NVINTF_NOTREADY;
  }
  else
  {
    uint32_t nvLen;
    NVINTF_itemID_t nvId;

    /* Get the NV ID parameters */
    pBuf = MT_ParseNvExtId( pBuf, &nvId );

    /* Get the length */
    nvLen = OsalPort_buildUint32( pBuf, sizeof(nvLen) );

    if ( nvLen > 0 )
    {
      uint8_t *defaultBuf = OsalPort_malloc(nvLen);

      if ( defaultBuf )
      {
        memset(defaultBuf, 0x00, nvLen);

        /* Attempt to create the specified item with no initial data */
        retVal = pZStackCfg->nvFps.createItem( nvId, nvLen, defaultBuf );

        OsalPort_free(defaultBuf);
      }
    }
    else
    {
      retVal = NVINTF_BADLENGTH;
    }
  }

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse( MT_SRSP_SYS, MT_SYS_NV_CREATE,
                                sizeof(retVal), &retVal);
}

/******************************************************************************
 * @fn      MT_SysNvDelete
 *
 * @brief   Attempt to delete an NV item (extended item ID)
 *
 * @param   pBuf - pointer to the data
 *
 * @return  None
 *****************************************************************************/
static void MT_SysNvDelete(uint8_t *pBuf)
{
  uint8_t retVal;

  if(( pZStackCfg == NULL ) || ( pZStackCfg->nvFps.deleteItem == NULL ))
  {
    /* NV item delete function not available */
    retVal = NVINTF_NOTREADY;
  }
  else
  {
    NVINTF_itemID_t nvId;

    /* Get the NV ID parameters */
    MT_ParseNvExtId( pBuf, &nvId );

    /* Attempt to delete the specified item */
    retVal = pZStackCfg->nvFps.deleteItem( nvId );
  }

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse( MT_SRSP_SYS, MT_SYS_NV_DELETE,
                                sizeof(retVal), &retVal);
}

/******************************************************************************
 * @fn      MT_SysNvLength
 *
 * @brief   Attempt to delete an NV item (extended item ID)
 *
 * @param   pBuf - pointer to the data
 *
 * @return  None
 *****************************************************************************/
static void MT_SysNvLength(uint8_t *pBuf)
{
  uint32_t nvLen;
  uint8_t retBuf[4];

  if(( pZStackCfg == NULL ) || ( pZStackCfg->nvFps.getItemLen == NULL ))
  {
    /* NV item length function not available */
    nvLen = 0;
  }
  else
  {
    NVINTF_itemID_t nvId;

    /* Get the NV ID parameters */
    MT_ParseNvExtId( pBuf, &nvId );

    /* Attempt to get length of the specified item */
    nvLen = pZStackCfg->nvFps.getItemLen( nvId );
  }

  /* Serialize the length bytes */
  OsalPort_bufferUint32( retBuf, nvLen );

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse( MT_SRSP_SYS, MT_SYS_NV_LENGTH,
                                sizeof(retBuf), retBuf);
}

/******************************************************************************
 * @fn      MT_SysNvRead
 *
 * @brief   Attempt to read an NV item (extended item ID)
 *
 * @param   pBuf - pointer to the data
 *
 * @return  None
 *****************************************************************************/
static void MT_SysNvRead(uint8_t *pBuf)
{
  uint8_t error;

  if(( pZStackCfg == NULL ) || ( pZStackCfg->nvFps.readItem == NULL ))
  {
    /* NV item length/read function not available */
    error = NVINTF_NOTREADY;
  }
  else
  {
    uint8_t dataLen;
    uint16_t dataOfs;
    uint8_t *pRetBuf;
    uint8_t respLen = 2;  /* Response header: [0]=status,[1]=length */
    NVINTF_itemID_t nvId;

    /* Get the NV ID parameters */
    pBuf = MT_ParseNvExtId( pBuf, &nvId );

    if( MT_StackNvExtId(&nvId) == TRUE )
    {
      /* Check whether read-access to this ZigBee Stack item is allowed */
      error = MT_CheckNvId( nvId.subID );
    }
    else
    {
      /* It's OK to read this item */
      error = ZSuccess;
    }

    /* Get the read data offset */
    dataOfs = OsalPort_buildUint16( pBuf );

    /* And the read data length */
    dataLen = pBuf[2];

    if( dataLen > (MT_MAX_RSP_DATA_LEN - respLen) )
    {
      /* Data length is limited by TX buffer size and MT protocol */
      dataLen = (MT_MAX_RSP_DATA_LEN - respLen);
    }
    respLen += dataLen;

    pRetBuf = OsalPort_malloc(respLen);
    if( pRetBuf != NULL )
    {
      if( error == ZSuccess )
      {
        /* Attempt to read data from the specified item */
        error = pZStackCfg->nvFps.readItem( nvId, dataOfs, dataLen, pRetBuf+2 );
        if( error == NVINTF_SUCCESS )
        {
          pRetBuf[0] = ZSuccess;
          pRetBuf[1] = dataLen;
          MT_BuildAndSendZToolResponse( MT_SRSP_SYS, MT_SYS_NV_READ,
                                        respLen, pRetBuf );
        }
      }
      OsalPort_free(pRetBuf);
    }
    else
    {
      /* Could not get buffer for NV data */
      error = ZMemError;
    }
  }

  if( error != ZSuccess )
  {
    uint8_t tmp[2] = { error, 0 };
    MT_BuildAndSendZToolResponse( MT_SRSP_SYS, MT_SYS_NV_READ,
                                  sizeof(tmp), tmp );
  }
}

/******************************************************************************
 * @fn      MT_SysNvWrite
 *
 * @brief   Attempt to write an NV item (extended item ID)
 *
 * @param   pBuf - pointer to the data
 *
 * @return  None
 *****************************************************************************/
static void MT_SysNvWrite(uint8_t *pBuf)
{
  uint8_t cmdId;
  uint8_t error = NVINTF_FAILURE;

  /* MT command ID */
  cmdId = pBuf[MT_RPC_POS_CMD1];

  if(( pZStackCfg == NULL ) || ( pZStackCfg->nvFps.writeItem == NULL ))
  {
    /* NV item length/read function not available */
    error = NVINTF_NOTREADY;
  }
  else
  {
    uint8_t dataLen;
    uint16_t dataOfs = 0;
    NVINTF_itemID_t nvId;

    /* Get the NV ID parameters */
    pBuf = MT_ParseNvExtId( pBuf, &nvId );

    if( cmdId == MT_SYS_NV_WRITE )
    {
      /* Get data offset for Write command */
      dataOfs = OsalPort_buildUint16( pBuf );
      pBuf += 2;
    }

    /* Get the write data length */
    dataLen = pBuf[0];
    pBuf += 1;

    // F065 process change: writing to an offset is no longer allowed, i.e. dataOfs must be 0
    // however, the host application may still send this field, so keep it for backwards
    // compatibility.
    if( dataOfs == 0 )
    {
      if ( MT_StackNvExtId(&nvId) == TRUE )
      {
        /* Set the Z-Globals value of this NV item */
        zgSetItem( nvId.subID, dataLen, pBuf );

        if( nvId.subID == ZCD_NV_EXTADDR )
        {
          /* Give MAC the new 64-bit address */
          ZMacSetReq( ZMacExtAddr, pBuf );
        }
      }
      /* Attempt to update (create) data to the specified item */
      error = pZStackCfg->nvFps.writeItem( nvId, dataLen, pBuf );
    }
    else
    {
      error = NVINTF_BADPARAM;
    }
  }

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse( MT_SRSP_SYS, cmdId, sizeof(error), &error);
}
#endif  /* FEATURE_NVEXID */
#endif  /* !CC253X_MACNP */

/******************************************************************************
 * @fn      MT_SysOsalStartTimer
 *
 * @brief
 *
 * @param   uint8_t pBuf - pointer to the data
 *
 * @return  None
 *****************************************************************************/
static void MT_SysOsalStartTimer(uint8_t *pBuf)
{
  uint8_t retValue;

  /* Skip over RPC header */
  pBuf += MT_RPC_FRAME_HDR_SZ;

  if (*pBuf <= 3)
  {
    uint16_t timer = OsalPort_buildUint16( pBuf+1 );
    uint16_t eventId = (uint16_t)MT_SysOsalEventId[pBuf[0]];

    retValue = OsalPortTimers_startTimer(MT_TaskID, eventId, timer);
  }
  else
  {
    retValue = ZInvalidParameter;
  }

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse( MT_SRSP_SYS, MT_SYS_OSAL_START_TIMER,
                                sizeof(retValue), &retValue);
}

/******************************************************************************
 * @fn      MT_SysOsalStopTimer
 *
 * @brief
 *
 * @param   uint8_t pBuf - pointer to the data
 *
 * @return  None
 *****************************************************************************/
static void MT_SysOsalStopTimer(uint8_t *pBuf)
{
  uint16_t eventId;
  uint8_t retValue = ZFailure;

  /* Skip over RPC header */
  pBuf += MT_RPC_FRAME_HDR_SZ;

  if (*pBuf <= 3)
  {
    eventId = (uint16_t) MT_SysOsalEventId[*pBuf];
    retValue = OsalPortTimers_stopTimer(MT_TaskID, eventId);
  }
  else
  {
    retValue = ZInvalidParameter;
  }

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse( MT_SRSP_SYS, MT_SYS_OSAL_STOP_TIMER,
                                sizeof(retValue), &retValue );
}

/******************************************************************************
 * @fn      MT_SysRandom
 *
 * @brief
 *
 * @param   uint8_t pData - pointer to the data
 *
 * @return  None
 *****************************************************************************/
static void MT_SysRandom()
{
  uint16_t randValue = OsalPort_rand();
  uint8_t retArray[2];

  retArray[0] = LO_UINT16(randValue);
  retArray[1] = HI_UINT16(randValue);

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse( MT_SRSP_SYS, MT_SYS_RANDOM,
                                sizeof(retArray), retArray );
}

#if !defined( CC26XX ) \
    && !defined (DeviceFamily_CC26X1) \
    && !defined (DeviceFamily_CC26X2) \
    && !defined (DeviceFamily_CC13X2) \
    && !defined (DeviceFamily_CC26X2X7) \
    && !defined (DeviceFamily_CC13X2X7) \
    && !defined (DeviceFamily_CC26X4) \
    && !defined (DeviceFamily_CC13X4) \
    && !defined (DeviceFamily_CC26X3)
/******************************************************************************
 * @fn      MT_SysAdcRead
 *
 * @brief   Reading ADC value, temperature sensor and voltage
 *
 * @param   uint8_t pBuf - pointer to the data
 *
 * @return  None
 *****************************************************************************/
static void MT_SysAdcRead(uint8_t *pBuf)
{
  uint16_t tempValue = 0;
  uint8_t retArray[2];

  /* Skip over RPC header */
  pBuf += MT_RPC_FRAME_HDR_SZ;

  {
    uint8_t channel = *pBuf++;  /* ADC channel */
    uint8_t resolution = *pBuf++;  /* ADC resolution */

    /* Voltage reading */
    switch (channel)
    {
      case HAL_ADC_CHANNEL_TEMP:  /* Temperature sensor */
      case HAL_ADC_CHANNEL_VDD:   /* Voltage reading */
        resolution = HAL_ADC_RESOLUTION_14;

      case HAL_ADC_CHANNEL_0:  /* Analog input channels */
      case HAL_ADC_CHANNEL_1:
      case HAL_ADC_CHANNEL_2:
      case HAL_ADC_CHANNEL_3:
      case HAL_ADC_CHANNEL_4:
      case HAL_ADC_CHANNEL_5:
      case HAL_ADC_CHANNEL_6:
      case HAL_ADC_CHANNEL_7:
        tempValue = HalAdcRead(channel, resolution);
        break;

      default:  /* Undefined channel */
        break;
    }
  }

  retArray[0] = LO_UINT16(tempValue);
  retArray[1] = HI_UINT16(tempValue);

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse( MT_SRSP_SYS, MT_SYS_ADC_READ,
                                sizeof(retArray), retArray);
}
#endif /* !CC26XX */

/******************************************************************************
 * @fn      MT_SysGpio
 *
 * @brief   ZAccel RPC interface for controlling the available GPIO pins.
 *
 * @param   uint8_t pBuf - pointer to the data
 *
 * @return  None
 *****************************************************************************/
static void MT_SysGpio(uint8_t *pBuf)
{
  uint8_t val;
  GPIO_Op_t op;

  /* Skip over RPC header */
  pBuf += MT_RPC_FRAME_HDR_SZ;

  op = (GPIO_Op_t)(*pBuf++);
  val = *pBuf;

  switch (op)
  {
    case GPIO_DIR:
      if (val & BV(0)) {GPIO_DIR_OUT(0);} else {GPIO_DIR_IN(0);}
      if (val & BV(1)) {GPIO_DIR_OUT(1);} else {GPIO_DIR_IN(1);}
      if (val & BV(2)) {GPIO_DIR_OUT(2);} else {GPIO_DIR_IN(2);}
      if (val & BV(3)) {GPIO_DIR_OUT(3);} else {GPIO_DIR_IN(3);}
      break;

    case GPIO_TRI:
      if(val & BV(0)) {GPIO_TRI(0);} else if(val & BV(4)) {GPIO_PULL_DN(0);} else {GPIO_PULL_UP(0);}
      if(val & BV(1)) {GPIO_TRI(1);} else if(val & BV(5)) {GPIO_PULL_DN(1);} else {GPIO_PULL_UP(1);}
      if(val & BV(2)) {GPIO_TRI(2);} else if(val & BV(6)) {GPIO_PULL_DN(2);} else {GPIO_PULL_UP(2);}
      if(val & BV(3)) {GPIO_TRI(3);} else if(val & BV(7)) {GPIO_PULL_DN(3);} else {GPIO_PULL_UP(3);}
      break;

    case GPIO_SET:
      if (val & BV(0)) {GPIO_SET(0);}
      if (val & BV(1)) {GPIO_SET(1);}
      if (val & BV(2)) {GPIO_SET(2);}
      if (val & BV(3)) {GPIO_SET(3);}
      break;

    case GPIO_CLR:
      if (val & BV(0)) {GPIO_CLR(0);}
      if (val & BV(1)) {GPIO_CLR(1);}
      if (val & BV(2)) {GPIO_CLR(2);}
      if (val & BV(3)) {GPIO_CLR(3);}
      break;

    case GPIO_TOG:
      if (val & BV(0)) {GPIO_TOG(0);}
      if (val & BV(1)) {GPIO_TOG(1);}
      if (val & BV(2)) {GPIO_TOG(2);}
      if (val & BV(3)) {GPIO_TOG(3);}
      break;

    case GPIO_GET:
      break;

    case GPIO_HiD:
      (val) ? GPIO_HiD_SET() :  GPIO_HiD_CLR();
      break;

    default:
      break;
  }

  val  = (GPIO_GET(0)) ? BV(0) : 0;
  val |= (GPIO_GET(1)) ? BV(1) : 0;
  val |= (GPIO_GET(2)) ? BV(2) : 0;
  val |= (GPIO_GET(3)) ? BV(3) : 0;

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse( MT_SRSP_SYS, MT_SYS_GPIO,
                                sizeof(val), &val);
}

/******************************************************************************
 * @fn      MT_SysStackTune
 *
 * @brief   RPC interface for tuning the stack parameters to adjust performance
 *
 * @param   uint8_t pBuf - pointer to the data
 *
 * @return  None
 *****************************************************************************/
static void MT_SysStackTune(uint8_t *pBuf)
{
  uint8_t rtrn;

  /* Skip over RPC header */
  pBuf += MT_RPC_FRAME_HDR_SZ;

  switch (*pBuf++)
  {
  case STK_TX_PWR:
    rtrn = ZMacSetReq(ZMacPhyTransmitPowerSigned, pBuf);
    break;

  case STK_RX_ON_IDLE:
    if ((*pBuf != TRUE) && (*pBuf != FALSE))
    {
      (void)ZMacGetReq(ZMacRxOnIdle, &rtrn);
    }
    else
    {
      rtrn = ZMacSetReq(ZMacRxOnIdle, pBuf);
    }
    break;

  default:
    rtrn = ZInvalidParameter;
    break;
  }

  MT_BuildAndSendZToolResponse( MT_SRSP_SYS, MT_SYS_STACK_TUNE,
                                sizeof(rtrn), &rtrn);
}

#ifdef FEATURE_UTC_TIME

/******************************************************************************
 * @fn      MT_SysSetUtcTime
 *
 * @brief   Set the OSAL UTC Time. UTC rollover is: 06:28:16 02/07/2136
 *
 * @param   pBuf - pointer to time parameters
 *
 * @return  None
 *****************************************************************************/
static void MT_SysSetUtcTime(uint8_t *pBuf)
{
  uint8_t retStat;
  UTCTime utcSecs;

  /* Skip over RPC header */
  pBuf += MT_RPC_FRAME_HDR_SZ;

  utcSecs = OsalPort_buildUint32( pBuf, 4 );
  if ( utcSecs == 0 )
  {
    UTCTimeStruct utc;

    /* Skip past UTC time */
    pBuf += 4;

    /* Get time and date parameters */
    utc.hour = *pBuf++;
    utc.minutes = *pBuf++;
    utc.seconds = *pBuf++;
    utc.month = (*pBuf++) - 1;
    utc.day = (*pBuf++) - 1;
    utc.year = OsalPort_buildUint16 ( pBuf );

    if ((utc.hour < 24) && (utc.minutes < 60) && (utc.seconds < 60) &&
        (utc.month < 12) && (utc.day < 31) && (utc.year > 1999) && (utc.year < 2136))
    {
      /* Got past the course filter, now check for leap year */
      if ((utc.month != 1) || (utc.day < (IsLeapYear( utc.year ) ? 29 : 28)))
      {
        /* Numbers look reasonable, convert to UTC */
        utcSecs = UTC_convertUTCSecs( &utc );
      }
    }
  }

  if ( utcSecs == 0 )
  {
    /* Bad parameter(s) */
    retStat = ZInvalidParameter;
  }
  else
  {
    /* Parameters accepted, set the time */
    UTC_setClock( utcSecs );
    retStat = ZSuccess;
  }

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse( MT_SRSP_SYS, MT_SYS_SET_TIME,
                                sizeof(retStat), &retStat);
}

/******************************************************************************
 * @fn      MT_SysGetUtcTime
 *
 * @brief   Get the OSAL UTC time
 *
 * @param   None
 *
 * @return  32-bit and Parsed UTC time
 *****************************************************************************/
static void MT_SysGetUtcTime(void)
{
  uint8_t len;
  uint8_t *buf;

  len = sizeof( UTCTime ) + sizeof( UTCTimeStruct );

  buf = OsalPort_malloc( len );
  if ( buf )
  {
    uint8_t *pBuf;
    UTCTime utcSecs;
    UTCTimeStruct utcTime;

    // Get current 32-bit UTC time and parse it
    utcSecs = UTC_getClock();
    UTC_convertUTCTime( &utcTime, utcSecs );

    // Start with 32-bit UTC time
    pBuf = OsalPort_bufferUint32( buf, utcSecs );

    // Concatenate parsed UTC time fields
    *pBuf++ = utcTime.hour;
    *pBuf++ = utcTime.minutes;
    *pBuf++ = utcTime.seconds;
    *pBuf++ = utcTime.month + 1;  // Convert to human numbers
    *pBuf++ = utcTime.day + 1;
    *pBuf++ = LO_UINT16( utcTime.year );
    *pBuf++ = HI_UINT16( utcTime.year );

    /* Build and send back the response */
    MT_BuildAndSendZToolResponse( MT_SRSP_SYS, MT_SYS_GET_TIME,
                                 (uint8_t)(pBuf-buf), buf);

    OsalPort_free( buf );
  }
}

#endif //FEATURE_UTC_TIME

/******************************************************************************
 * @fn      MT_SysSetTxPower
 *
 * @brief   Set the transmit power.
 *
 * @param   pBuf - MT message containing the ZMacTransmitPower_t power level to set.
 *
 * @return  None
 *****************************************************************************/
static void MT_SysSetTxPower(uint8_t *pBuf)
{
  /* A local variable to hold the signed dBm value of TxPower that is being requested. */
  int8_t txPower;
  uint8_t status;

  /* Parse the requested dBm from the RPC message. */
  txPower = pBuf[MT_RPC_POS_DAT0];

  status = MAP_MAC_MlmeSetReq(MAC_PHY_TRANSMIT_POWER_SIGNED, &txPower);

  // Send back response that includes the status of the set command.
  // either: MAC_SUCCESS or MAC_INVALID_PARAMETER
  MT_BuildAndSendZToolResponse( MT_SRSP_SYS, MT_SYS_SET_TX_POWER, 1,
                                &status);
}

#if defined ( FEATURE_SYSTEM_STATS )
/******************************************************************************
 * @fn      MT_SysZDiagsInitStats
 *
 * @brief   Initialize the statistics table in NV or restore values from
 *          NV into the Statistics table in RAM
 *
 * @param   None
 *
 * @return  None
 *****************************************************************************/
static void MT_SysZDiagsInitStats(void)
{
  uint8_t retValue;

  retValue = ZDiagsInitStats();

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse( MT_SRSP_SYS, MT_SYS_ZDIAGS_INIT_STATS,
                                sizeof(retValue), &retValue);
}

/******************************************************************************
 * @fn      MT_SysZDiagsClearStats
 *
 * @brief   Clears the statistics table in RAM and NV if option flag set.
 *
 * @param   uint8_t pBuf - pointer to the data
 *
 * @return  None
 *****************************************************************************/
static void MT_SysZDiagsClearStats(uint8_t *pBuf)
{
  uint32_t sysClock;
  uint8_t retBuf[4];

  /* parse header */
  pBuf += MT_RPC_FRAME_HDR_SZ;

  /* returns the system clock of the time when the statistics were cleared */
  sysClock = ZDiagsClearStats( *pBuf );

  OsalPort_bufferUint32( retBuf, sysClock );

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse( MT_SRSP_SYS, MT_SYS_ZDIAGS_CLEAR_STATS,
                                sizeof(retBuf), retBuf);
}

/******************************************************************************
 * @fn      MT_SysZDiagsGetStatsAttr
 *
 * @brief   Reads specific system (attribute) ID statistics and/or metrics.
 *
 * @param   uint8_t pBuf - pointer to the data
 *
 * @return  None
 *****************************************************************************/
static void MT_SysZDiagsGetStatsAttr(uint8_t *pBuf)
{
  uint16_t attrId;
  uint32_t attrValue;
  uint8_t retBuf[4];

  /* parse header */
  pBuf += MT_RPC_FRAME_HDR_SZ;

  /* Get the Attribute ID */
  attrId = OsalPort_buildUint16( pBuf );

  attrValue = ZDiagsGetStatsAttr( attrId );

  OsalPort_bufferUint32( retBuf, attrValue );

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse( MT_SRSP_SYS, MT_SYS_ZDIAGS_GET_STATS,
                                sizeof(retBuf), retBuf);
}

/******************************************************************************
 * @fn      MT_SysZDiagsRestoreStatsFromNV
 *
 * @brief   Restores the statistics table from NV into the RAM table.
 *
 * @param   None
 *
 * @return  None
 *****************************************************************************/
static void MT_SysZDiagsRestoreStatsFromNV(void)
{
  uint8_t retValue;

  retValue = ZDiagsRestoreStatsFromNV();

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse( MT_SRSP_SYS, MT_SYS_ZDIAGS_RESTORE_STATS_NV,
                                sizeof(retValue), &retValue);
}

/******************************************************************************
 * @fn      MT_SysZDiagsSaveStatsToNV
 *
 * @brief   Saves the statistics table from RAM to NV.
 *
 * @param   None
 *
 * @return  None
 *****************************************************************************/
static void MT_SysZDiagsSaveStatsToNV(void)
{
  uint32_t sysClock;
  uint8_t retBuf[4];

  /* Returns system clock of the time when the statistics were saved to NV */
  sysClock = ZDiagsSaveStatsToNV();

  OsalPort_bufferUint32( retBuf, sysClock );

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse( MT_SRSP_SYS, MT_SYS_ZDIAGS_SAVE_STATS_TO_NV,
                                sizeof(retBuf), retBuf);
}
#endif /* FEATURE_SYSTEM_STATS */
#endif /* MT_SYS_FUNC */

/******************************************************************************
 * SUPPORT
 *****************************************************************************/

/******************************************************************************
 * @fn      MT_SysResetInd()
 *
 * @brief   Sends a ZTOOL "reset response" message.
 *
 * @param   None
 *
 * @return  None
 *
 *****************************************************************************/
void MT_SysResetInd(void)
{
  uint8_t retArray[6];

  retArray[0] = 0; // reason deprecated on cc26xx platform
  OsalPort_memcpy( &retArray[1], MTVersionString, 5 );   /* Revision info */

  /* Send out Reset Response message */
  MT_BuildAndSendZToolResponse( MT_ARSP_SYS, MT_SYS_RESET_IND,
                                sizeof(retArray), retArray);
}

/******************************************************************************
 * @fn      MT_SysOsalTimerExpired()
 *
 * @brief   Sends a SYS Osal Timer Expired
 *
 * @param   None
 *
 * @return  None
 *
 *****************************************************************************/
void MT_SysOsalTimerExpired(uint8_t Id)
{
  uint8_t retValue = Id;

  MT_BuildAndSendZToolResponse( MT_ARSP_SYS, MT_SYS_OSAL_TIMER_EXPIRED,
                                sizeof(retValue), &retValue);
}

#if defined( ENABLE_MT_SYS_RESET_SHUTDOWN )
/******************************************************************************
 * @fn          powerOffSoc
 *
 * @brief   Put the device in lowest power mode infinitely
 *
 * @param   None
 *
 * @return  None
 *****************************************************************************/
static void powerOffSoc(void)
{
  /* turn off the receiver */
  MAC_RADIO_RXTX_OFF();

  /* clear any receive interrupt that happened to squeak through */
  MAC_RADIO_CLEAR_RX_THRESHOLD_INTERRUPT_FLAG();

  /* put MAC timer to sleep */
  MAC_RADIO_TIMER_SLEEP();

  /* power of radio */
  MAC_RADIO_TURN_OFF_POWER();

  Power_releaseConstraint(PowerCC26XX_SD_DISALLOW);
  /* Go to shutdown */
  Power_shutdown(0, 0);

  // just in case we wake up for some unknown reason
  OsalPort_enterCS();
  while (1)
  {
    asm(" NOP");
  }
}
#endif

/******************************************************************************
 *****************************************************************************/
