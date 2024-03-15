/***************************************************************************************************
  Filename:       MT_APP_CONFIG.c
  Revised:        $Date: 2016-03-31 01:06:52 -0700 (Thu, 31 Marc 2016) $
  Revision:       $Revision:  $

  Description:    MonitorTest functions for application configuration.

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
#include "mt_app_config.h"


#include "bdb.h"
#include "bdb_interface.h"
#include "zd_app.h"
#include "osal_nv.h"

#include "zstack.h"

/***************************************************************************************************
* LOCAL FUNCTIONs
***************************************************************************************************/

#if defined (MT_APP_CNF_FUNC)

static void MT_AppCnfSetNwkFrameCounter(uint8_t *pBuf);
static void MT_AppCnfSetDefaultRemoteEndDeviceTimeout(uint8_t *pBuf);
static void MT_AppCnfSetDefaultParentInformation(uint8_t *pBuf);
static void MT_AppCnfSetEndDeviceTimeout(uint8_t *pBuf);
#if (ZG_BUILD_COORDINATOR_TYPE)
static void MT_AppCnfSetAllowRejoinTCPolicy(uint8_t *pBuf);
#endif

static void MT_AppCnfBDBSetChannel(uint8_t* pBuf);
static void MT_AppCnfBDBStartCommissioning(uint8_t* pBuf);
#if (ZG_BUILD_COORDINATOR_TYPE)
    static void MT_AppCnfBDBSetTCRequireKeyExchange(uint8_t *pBuf);
    static void MT_AppCnfBDBAddInstallCode(uint8_t *pBuf);
    static void MT_AppCnfBDBSetJoinUsesInstallCodeKey(uint8_t *pBuf);
#endif
#if (ZG_BUILD_JOINING_TYPE)
    static void MT_AppCnfBDBSetActiveCentralizedKey(uint8_t* pBuf);
    static void MT_AppCnfBDBRecoverNwk(uint8_t* pBuf);
#endif

#if (ZG_BUILD_ENDDEVICE_TYPE)
    static void MT_AppCnfSetPollRateType(uint8_t* pBuf);
#endif

#endif


/***************************************************************************************************
* External variables
***************************************************************************************************/
extern uint32_t nwkFrameCounter;
extern uint16_t nwkFrameCounterChanges;


#if defined (MT_APP_CNF_FUNC)
uint8_t MT_AppCnfCommandProcessing(uint8_t *pBuf)
{
  uint8_t status = MT_RPC_SUCCESS;

  switch (pBuf[MT_RPC_POS_CMD1])
  {
    case MT_APP_CNF_SET_NWK_FRAME_COUNTER:
      MT_AppCnfSetNwkFrameCounter(pBuf);
    break;
    case MT_APP_CNF_SET_DEFAULT_REMOTE_ENDDEVICE_TIMEOUT:
      MT_AppCnfSetDefaultRemoteEndDeviceTimeout(pBuf);
    break;
    case MT_APP_CNF_BDB_SET_DEFAULT_PARENT_INFO:
      MT_AppCnfSetDefaultParentInformation(pBuf);
    break;

    case MT_APP_CNF_SET_ENDDEVICETIMEOUT:
      MT_AppCnfSetEndDeviceTimeout(pBuf);
    break;
#if (ZG_BUILD_COORDINATOR_TYPE)
    case MT_APP_CNF_SET_ALLOWREJOIN_TC_POLICY:
      MT_AppCnfSetAllowRejoinTCPolicy(pBuf);
    break;
#endif


    case MT_APP_CNF_BDB_START_COMMISSIONING:
      MT_AppCnfBDBStartCommissioning(pBuf);
    break;
    case MT_APP_CNF_BDB_SET_CHANNEL:
      MT_AppCnfBDBSetChannel(pBuf);
    break;


#if (ZG_BUILD_COORDINATOR_TYPE)
      case MT_APP_CNF_BDB_ADD_INSTALLCODE:
        MT_AppCnfBDBAddInstallCode(pBuf);
      break;
      case MT_APP_CNF_BDB_SET_TC_REQUIRE_KEY_EXCHANGE:
        MT_AppCnfBDBSetTCRequireKeyExchange(pBuf);
      break;
      case MT_APP_CNF_BDB_SET_JOINUSESINSTALLCODEKEY:
        MT_AppCnfBDBSetJoinUsesInstallCodeKey(pBuf);
      break;
#endif
#if (ZG_BUILD_JOINING_TYPE)
      case MT_APP_CNF_BDB_SET_ACTIVE_DEFAULT_CENTRALIZED_KEY:
        MT_AppCnfBDBSetActiveCentralizedKey(pBuf);
      break;
      case MT_APP_CNF_BDB_RECOVER_NWK:
        MT_AppCnfBDBRecoverNwk(pBuf);
      break;
#endif

#if (ZG_BUILD_ENDDEVICE_TYPE)
      case MT_APP_CNF_SET_POLL_RATE_TYPE:
        MT_AppCnfSetPollRateType(pBuf);
      break;
#endif

  }
  return status;
}


/***************************************************************************************************
* @fn      MT_AppCnfCommissioningNotification
*
* @brief   Notify the host processor about an event in BDB
*
* @param   bdbCommissioningModeMsg - Commissioning notification message
*
* @return  void
***************************************************************************************************/
void MT_AppCnfCommissioningNotification(bdbCommissioningModeMsg_t* bdbCommissioningModeMsg)
{
  uint8_t retArray[3];

  retArray[0] = bdbCommissioningModeMsg->bdbCommissioningStatus;
  retArray[1] = bdbCommissioningModeMsg->bdbCommissioningMode;
  retArray[2] = bdbCommissioningModeMsg->bdbRemainingCommissioningModes;

  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_AREQ | (uint8_t)MT_RPC_SYS_APP_CNF), MT_APP_CNF_BDB_COMMISSIONING_NOTIFICATION, sizeof(bdbCommissioningModeMsg_t), retArray);
}


/***************************************************************************************************
* @fn      MT_AppCnfBDBStartCommissioning
*
* @brief   Start the commissioning process setting the commissioning mode given.
*
* @param   pBuf - pointer to received buffer
*
* @return  void
***************************************************************************************************/
static void MT_AppCnfBDBStartCommissioning(uint8_t* pBuf)
{
  uint8_t retValue = ZSuccess;
  uint8_t cmdId;

  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  bdb_StartCommissioning(*pBuf);

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_APP_CNF), cmdId, 1, &retValue);
}


/***************************************************************************************************
* @fn      MT_AppCnfBDBSetChannel
*
* @brief   Set the primary or seconday channel for discovery or formation procedure
*
* @param   pBuf - pointer to received buffer
*
* @return  void
***************************************************************************************************/
static void MT_AppCnfBDBSetChannel(uint8_t* pBuf)
{
  uint8_t retValue = ZSuccess;
  uint8_t cmdId;
  uint8_t isPrimary;
  uint32_t Channel;

  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  isPrimary = *pBuf;
  pBuf++;

  Channel = OsalPort_buildUint32(pBuf, sizeof(uint32_t));

  bdb_setChannelAttribute(isPrimary,Channel);

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_APP_CNF), cmdId, 1, &retValue);
}



#if (ZG_BUILD_COORDINATOR_TYPE)
/*********************************************************************
 * @fn          MT_AppCnfBDBSetTCRequireKeyExchange
 *
 * @brief       Configure bdbTrustCenterRequireKeyExchange attribute.
 *
 * @param       Set attribute to FALSE if *pBuf == 0, FALSE otherwise
 *
 * @return      none
 */
static void MT_AppCnfBDBSetTCRequireKeyExchange(uint8_t *pBuf)
{
  uint8_t retValue = ZSuccess;
  uint8_t cmdId;

  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  if(*pBuf)
  {
    bdb_setTCRequireKeyExchange(TRUE);
  }
  else
  {
    bdb_setTCRequireKeyExchange(FALSE);
  }
  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_APP_CNF), cmdId, 1, &retValue);
}

 /*********************************************************************
 * @fn          MT_AppCnfBDBSetJoinUsesInstallCodeKey
 *
 * @brief       Configure bdbJoinUsesInstallCodeKey attribute.
 *
 * @param       Set attribute to FALSE if *pBuf == 0, FALSE otherwise
 *
 * @return      none
 */
static void MT_AppCnfBDBSetJoinUsesInstallCodeKey(uint8_t *pBuf)
{
  uint8_t retValue = ZSuccess;
  uint8_t cmdId;

  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  if(*pBuf)
  {
    bdb_setJoinUsesInstallCodeKey(TRUE);
  }
  else
  {
    bdb_setJoinUsesInstallCodeKey(FALSE);
  }
  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_APP_CNF), cmdId, 1, &retValue);
}

 /*********************************************************************
 * @fn          MT_AppCnfBDBAddInstallCode
 *
 * @brief       Add a preconfigured key used as IC derived key to TC device
 *              see formats allowed in BDB_INSTALL_CODE_USE.
 *
 * @param       pBuf - pointer to received buffer
 *
 * @return      none
 */
static void MT_AppCnfBDBAddInstallCode(uint8_t* pBuf)
{
  uint8_t retValue = ZSuccess;
  uint8_t cmdId;
  uint8_t *pExtAddr;
  uint8_t installCodeFormat;

  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  installCodeFormat = *pBuf;

  pBuf++;

  /* Extended Addr */
  pExtAddr = pBuf;
  pBuf += Z_EXTADDR_LEN;  //Point to the IC data

  switch(installCodeFormat)
  {
    case BDB_INSTALL_CODE_USE_IC_CRC:
      retValue = bdb_addInstallCode(pBuf,pExtAddr);
    break;
    case BDB_INSTALL_CODE_USE_KEY:
      retValue = APSME_AddTCLinkKey(pBuf,pExtAddr);
    break;
  }

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_APP_CNF), cmdId, 1, &retValue);
}
#endif
#if (ZG_BUILD_JOINING_TYPE)

/*********************************************************************
 * @fn      MT_AppCnfBDBSetActiveCentralizedKey
 *
 * @brief   Set the active centralized key to be used.
 *
 * @param   pBuf - pointer to received buffer
 *
 * @return  void
 */
static void MT_AppCnfBDBSetActiveCentralizedKey(uint8_t* pBuf)
{
  uint8_t retValue;
  uint8_t cmdId;
  uint8_t keyMode;

  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  //get the key mode
  keyMode = *pBuf;

  //point to the key input
  pBuf++;

  retValue = bdb_setActiveCentralizedLinkKey(keyMode,pBuf);

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_APP_CNF), cmdId, 1, &retValue);
}

 /*********************************************************************
 * @fn      MT_AppCnfBDBRecoverNwk
 *
 * @brief   Instruct a joiner to try to rejoin its previous network
 *
 * @param   pBuf - pointer to received buffer
 *
 * @return  void
 */
static void MT_AppCnfBDBRecoverNwk(uint8_t* pBuf)
{
  uint8_t retValue;
  uint8_t cmdId;

  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  retValue = bdb_recoverNwk();

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_APP_CNF), cmdId, 1, &retValue);
}

#endif //#if(ZG_BUILD_JOINING_TYPE)


#if (ZG_BUILD_ENDDEVICE_TYPE)
/*********************************************************************
 * @fn      MT_AppCnfSetPollRateType
 *
 * @brief   Set poll rate types
 *
 * @param   pBuf - pointer to received buffer
 *
 * @return  void
 */
static void MT_AppCnfSetPollRateType(uint8_t* pBuf)
{

  uint8_t retValue = ZSuccess;
  uint8_t cmdId;
  uint16_t pollRateType;
  uint32_t rate;

  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  pollRateType = OsalPort_buildUint16(pBuf);
  pBuf += sizeof(uint16_t);
  rate = OsalPort_buildUint32(pBuf,4);

  nwk_SetConfigPollRate(pollRateType,rate);

  //Update the Nv Items
  osal_nv_write(ZCD_NV_POLL_RATE, sizeof(nwk_pollRateTable), &nwk_pollRateTable);

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_APP_CNF), cmdId, 1, &retValue);
}
#endif




#if (ZG_BUILD_COORDINATOR_TYPE)
/***************************************************************************************************
* @fn      MT_AppCnfSetAllowRejoinTCPolicy
*
* @brief   Set the AllowRejoin TC policy
*
* @param   pBuf - pointer to received buffer
*
* @return  void
***************************************************************************************************/
static void MT_AppCnfSetAllowRejoinTCPolicy(uint8_t *pBuf)
{
  uint8_t retValue = ZSuccess;
  uint8_t cmdId;

  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  if(*pBuf)
  {
    zgAllowRejoinsWithWellKnownKey = TRUE;
  }
  else
  {
    zgAllowRejoinsWithWellKnownKey = FALSE;
  }

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_APP_CNF), cmdId, 1, &retValue);
}
#endif

/***************************************************************************************************
 * @fn      MT_AppCnfSetEndDeviceTimeout
 *
 * @brief   Set End Device Timeout
 *
 * @param   pBuf - pointer to received buffer
 *
 * @return  void
 ***************************************************************************************************/
static void MT_AppCnfSetEndDeviceTimeout(uint8_t *pBuf)
{
  uint8_t retValue = ZSuccess;
  uint8_t cmdId;

  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  if(*pBuf > 14)
  {
    retValue = ZInvalidParameter;
  }
  else
  {
    /* Populate info */
    zgEndDeviceTimeoutValue = *pBuf;
    osal_nv_write( ZCD_NV_END_DEV_TIMEOUT_VALUE, sizeof(zgEndDeviceTimeoutValue), &zgEndDeviceTimeoutValue);
  }
  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_APP_CNF), cmdId, 1, &retValue);
}



/***************************************************************************************************
 * @fn      MT_AppCnfSetDefaultParentInformation
 *
 * @brief   Set Parent Information to give as response to End Device Timeout request
 *
 * @param   pBuf - pointer to received buffer
 *
 * @return  void
 ***************************************************************************************************/
static void MT_AppCnfSetDefaultParentInformation(uint8_t *pBuf)
{
  uint8_t retValue = ZSuccess;
  uint8_t cmdId;
  uint8_t  tempParentInformation;

  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  /* Populate info */
  tempParentInformation = *pBuf++;

  if(tempParentInformation & ~PARENT_INFO_VALID_PARAMETERS)
  {
    retValue = ZInvalidParameter;
  }
  else
  {
    /* Populate parent info */
    zgNwkParentInformation = tempParentInformation;
    osal_nv_write( ZCD_NV_NWK_PARENT_INFO, sizeof(zgNwkParentInformation), &zgNwkParentInformation );
  }

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_APP_CNF), cmdId, 1, &retValue);
}

/***************************************************************************************************
 * @fn      MT_AppCnfSetDefaultRemoteEndDeviceTimeout
 *
 * @brief   Set Remote End Device Timeout
 *
 * @param   pBuf - pointer to received buffer
 *
 * @return  void
 ***************************************************************************************************/
static void MT_AppCnfSetDefaultRemoteEndDeviceTimeout(uint8_t *pBuf)
{
  uint8_t retValue = ZSuccess;
  uint8_t cmdId;
  uint8_t  tempTimeoutIndex;

  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  /* Populate info */
  tempTimeoutIndex = *pBuf++;

  if(tempTimeoutIndex > 14)
  {
    retValue = ZInvalidParameter;
  }
  else
  {
    /* Populate info */
    zgNwkEndDeviceTimeoutDefault = tempTimeoutIndex;
    osal_nv_write( ZCD_NV_NWK_ENDDEV_TIMEOUT_DEF, sizeof(zgNwkEndDeviceTimeoutDefault), &zgNwkEndDeviceTimeoutDefault );
  }

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_APP_CNF), cmdId, 1, &retValue);
}

 /***************************************************************************************************
 * @fn      MT_AppCnfSetNwkFrameCounter
 *
 * @brief   Set the nwk frame counter to the specified value for the current network.
 *          THIS IS ONLY A DEBUG INTERFACE AND SHOULD NOT BE USE IN REAL APPLICATIONS
 *
 * @param   pBuf - nwk frame counter
 *
 * @return  void
 ***************************************************************************************************/
 static void MT_AppCnfSetNwkFrameCounter(uint8_t* pBuf)
{
  uint8_t retValue = ZSuccess;
  uint8_t cmdId;

  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  nwkFrameCounter = OsalPort_buildUint32(pBuf, sizeof(uint32_t));
  nwkFrameCounterChanges = 0;

  //Save the NwkFrameCounter
  ZDApp_SaveNwkKey();

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_APP_CNF), cmdId, 1, &retValue);
}

#endif

