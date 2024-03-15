/**************************************************************************************************
  Filename:       bdb.c
  Revised:        $Date: 2016-02-25 11:51:49 -0700 (Thu, 25 Feb 2016) $
  Revision:       $Revision: - $

  Description:    This file contains the Base Device Behavior functions and attributes.


  Copyright 2006-2015 Texas Instruments Incorporated.

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

/*********************************************************************
 * INCLUDES
 */

#include "ti_zstack_config.h"
#include "bdb.h"
#include "zd_app.h"
#include "rom_jt_154.h"
#include "zd_config.h"
#include "zd_object.h"
#include "osal_nv.h"
#include "addr_mgr.h"
#include "zd_sec_mgr.h"
#include "nwk.h"
#include "nwk_util.h"
#include "ssp_hash.h"
#include "zcl.h"
#include "bdb_interface.h"
#include "zstack.h"
#include "zglobals.h"

#ifdef BDB_REPORTING
#include "bdb_reporting.h"
#endif

#if !defined (DISABLE_GREENPOWER_BASIC_PROXY) && (ZG_BUILD_RTR_TYPE)
#include "gp_interface.h"
#include "gp_proxy.h"
#include "gp_common.h"
#include "dgp_stub.h"
#endif


#if defined ( BDB_TL_INITIATOR )
#include "bdb_touchlink_initiator.h"
#endif
#if defined ( BDB_TL_TARGET )
#include "bdb_touchlink_target.h"
#endif


#if ( defined ( BDB_TL_INITIATOR ) || defined ( BDB_TL_TARGET ) )
  #include "bdb_touchlink.h"
#endif

#ifdef MT_APP_CNF_FUNC
#include "mt_app_config.h"
#endif

#include "zstacktask.h"
#include "ti_zstack_config.h"


 /*********************************************************************
 * MACROS
 */
//This is actually the channels used
#define vScanChannels  zgDefaultChannelList

 /*********************************************************************
 * CONSTANTS
 */

#define NUMBER_OF_CHANNELS     16

#define CHANNEL_11_MASK_POS    11
#define CHANNEL_26_MASK_POS    26

uint8_t bdb_FB_InitiatorCurrentCyclesNumber = 0; //last cycle is #1 (i.e. cycles-left = (bdb_FB_InitiatorCurrentCyclesNumber - 1))

/*********************************************************************
 * TYPEDEFS
 */


 /*********************************************************************
 * GLOBAL VARIABLES
 */

byte bdb_TaskID;
bdbAttributes_t bdbAttributes = BDB_ATTRIBUTES_DEFAULT_CONFIG;
epList_t *bdb_HeadEpDescriptorList = NULL;
epList_t *bdb_CurrEpDescriptorList = NULL;

bdbFindingBindingRespondent_t *pRespondentHead = NULL;
bdbFindingBindingRespondent_t *pRespondentCurr = NULL;
bdbFindingBindingRespondent_t *pRespondentNext = NULL;

bdbCommissioningProcedureState_t bdbCommissioningProcedureState;
bool bdb_initialization = FALSE;  //Variable to tell if the initialization process has been started

//Nwk formation and nwk steering for nodes not in nwk
bool vDoPrimaryScan = TRUE;

uint8_t zgBdbInstallCodeCRC[INSTALL_CODE_LEN + INSTALL_CODE_CRC_LEN] = {0x83,0xFE,0xD3,0x40,0x7A,0x93,0x97,0x23,0xA5,0xC6,0x39,0xB2,0x69,0x16,0xD5,0x05,0xC3,0xB5};

//Pointer of the nwk being tried in association process
#if (ZG_BUILD_JOINING_TYPE)
static networkDesc_t *pBDBListNwk = NULL;
#endif

bool touchLinkTargetEnabled = FALSE;

uint8_t bdbSecureRejoinAttempts = 0;

bool bdb_performingTCRejoin = FALSE;

bool bdb_acceptNewTrustCenterLinkKey = FALSE;

 /*********************************************************************
 * EXTERNAL VARIABLES
 */

extern devStartModes_t devStartMode;
extern bool  requestNewTrustCenterLinkKey;
extern uint32_t requestLinkKeyTimeout;

#if (BDB_FINDING_BINDING_CAPABILITY_ENABLED==1)
extern bdbGCB_IdentifyTimeChange_t pfnIdentifyTimeChangeCB;
extern uint8_t bdbIndentifyActiveEndpoint;
#endif

extern bdbFindingBindingRespondent_t *pRespondentNext;

#ifndef DISABLE_GREENPOWER_BASIC_PROXY
extern ZDO_DeviceAnnce_t aliasConflictAnnce;
#endif

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

extern void ZDApp_ResetTimerStart( uint16_t delay );
extern void ZDApp_NodeProfileSync( uint8_t stackProfile );
extern uint8_t ZDApp_RestoreNwkKey( uint8_t incrFrmCnt );
extern uint8_t ZDApp_ReadNetworkRestoreState( void );

extern bdbFindingBindingRespondent_t* bdb_getRespondentRetry(bdbFindingBindingRespondent_t* pRespondentHead);
extern void bdb_ProcessSimpleDesc( zdoIncomingMsg_t *msgPtr );
extern void bdb_ProcessIEEEAddrRsp(zdoIncomingMsg_t *pMsg);

/*********************************************************************
 * LOCAL VARIABLES
 */
#if (ZG_BUILD_JOINING_TYPE)
  static uint8_t bdb_nwkAssocRetriesCount = 0;
#endif
#if (ZG_BUILD_COORDINATOR_TYPE)
  static bdb_joiningDeviceList_t *bdb_joiningDeviceList = NULL;
#endif

#if (BDB_FINDING_BINDING_CAPABILITY_ENABLED==1)
//Latch to save the status success of any attempt in the periodic F&B process
static uint8_t bdb_FBStateSuccessLatch = FALSE;
#endif
 /*********************************************************************
 * LOCAL FUNCTIONS
 */
static void bdb_ProcessOSALMsg(bdbInMsg_t *msgPtr);
void bdb_NotifyCommissioningModeStart(uint8_t commissioningMode);
static void bdb_processZDOMgs(zdoIncomingMsg_t *pMsg);

#if (ZG_BUILD_JOINING_TYPE)
static void bdb_requestTCStackVersion(void);
static void bdb_requestTCLinkKey(void);
static void bdb_requestVerifyTCLinkKey(void);

#endif


static void bdb_processTimeout(void);
static void bdb_startResumeCommissioningProcess(void);
static void bdb_nwkSteeringDeviceOnNwk(void);
static void bdb_nwkJoiningFormation(bool isJoining);

#if !defined (DISABLE_GREENPOWER_BASIC_PROXY) && (ZG_BUILD_RTR_TYPE)
static uint8_t gp_ChangeChannelReq(void);
static void gp_CBInit(void);
#endif


#if (ZG_BUILD_COORDINATOR_TYPE)
static void bdb_TCProcessJoiningList(void);
static ZStatus_t bdb_TCJoiningDeviceFree(bdb_joiningDeviceList_t* JoiningDeviceToRemove);
#endif
#if (ZG_BUILD_COORDINATOR_TYPE)
static bdbGCB_TCLinkKeyExchangeProcess_t  pfnTCLinkKeyExchangeProcessCB = NULL;
#endif
static bdbGCB_CommissioningStatus_t       pfnCommissioningStatusCB = NULL;
#if (ZG_BUILD_JOINING_TYPE)
static bdbGCB_CBKETCLinkKeyExchange_t     pfnCBKETCLinkKeyExchange = NULL;
static bdbGCB_FilterNwkDesc_t             pfnFilterNwkDesc = NULL;
#endif



static void bdb_calculateCCITT_CRC (uint8_t *Mb, uint32_t msglen, uint16_t *crc);
static void bdb_crcInit(uint16_t *crc, uint16_t *crcinit_direct, uint16_t *crcinit_nondirect);
static uint16_t bdb_crcReflect (uint16_t crc, uint16_t bitnum);
static uint16_t bdb_crcBitByBitFast(uint8_t * p, uint32_t len, uint16_t crcinit_direct, uint16_t crcinit_nondirect);
#if (ZG_BUILD_JOINING_TYPE)
static void bdb_ProcessNodeDescRsp(zdoIncomingMsg_t *pMsg);
#endif

/*********************************************************************
 * PUBLIC FUNCTIONS
 *********************************************************************/
#if (ZG_BUILD_JOINING_TYPE)
void bdb_filterNwkDisc(void);
ZStatus_t bdb_joinProcess(networkDesc_t *pChosenNwk);
#endif

ZStatus_t bdb_TCAddJoiningDevice(uint16_t parentAddr, uint8_t* JoiningExtAddr);
void bdb_TCjoiningDeviceComplete(uint8_t* JoiningExtAddr);

 /*********************************************************************
 * @fn          bdb_Init
 *
 * @brief       Initialization function for the Base Device Behavior.
 *
 * @param       task_id - bdb_TaskID Task ID
 *
 * @return      none
 */
void bdb_Init( byte task_id )
{
  bdb_TaskID = task_id;

#if (ZG_BUILD_COORDINATOR_TYPE)
  if(ZG_DEVICE_COORDINATOR_TYPE)
  {
    if(bdbAttributes.bdbJoinUsesInstallCodeKey)
    {
      zgAllowInstallCodes = ZG_IC_MUST_USED;
    }
  }
#endif

#ifdef BDB_REPORTING
  bdb_RepInit();
#endif

  //Register ZDO callbacks
  ZDO_RegisterForZDOMsg ( task_id, Node_Desc_rsp );
#if (BDB_FINDING_BINDING_CAPABILITY_ENABLED==1)
  ZDO_RegisterForZDOMsg ( task_id, IEEE_addr_rsp );
  ZDO_RegisterForZDOMsg ( task_id, Simple_Desc_rsp );
#endif

#if !defined (DISABLE_GREENPOWER_BASIC_PROXY) && (ZG_BUILD_RTR_TYPE)
  gp_RegisterGPChangeChannelReqForBDBCB(gp_ChangeChannelReq);
  gp_CBInit();
#endif


}




/*********************************************************************
 * @fn      bdb_RegisterSimpleDescriptor
 *
 * @brief   Register the Simple descriptor. This function also registers
 *          the profile's cluster conversion table.
 *
 * @param   simpleDesc - a pointer to a valid SimpleDescriptionFormat_t, must not be NULL.
 *
 * @return  none
 */
void bdb_RegisterSimpleDescriptor( SimpleDescriptionFormat_t *simpleDesc )
{
  endPointDesc_t *epDesc;

  // Register the application's endpoint descriptor
  //  - This memory is allocated and never freed.
  epDesc = OsalPort_malloc( sizeof ( endPointDesc_t ) );
  if ( epDesc )
  {
    // Fill out the endpoint description.
    epDesc->endPoint = simpleDesc->EndPoint;
    epDesc->simpleDesc = simpleDesc;
    epDesc->latencyReq = noLatencyReqs;

    // Register the endpoint description with the AF
    afRegister( epDesc );
  }
}

#if (BDB_FINDING_BINDING_CAPABILITY_ENABLED==1)
/*********************************************************************
 * @fn      bdb_ZclIdentifyCmdInd
 *
 * @brief   Callback from the ZCL General Cluster Library when
 *          it received an Identity Command for this application.
 *
 * @param   identifyTime - the number of seconds to identify yourself
 * @param   endpoint - destination endpoint
 *
 * @return  none
 */
void bdb_ZclIdentifyCmdInd( uint16_t identifyTime, uint8_t endpoint )
{
  zclAttrRec_t identifyAttrRec;

  if ( zclFindAttrRec( endpoint, ZCL_CLUSTER_ID_GENERAL_IDENTIFY,
                      ATTRID_IDENTIFY_IDENTIFY_TIME, &identifyAttrRec ) )
  {
    //If we are processing an actual change
    if(*(uint16_t*)identifyAttrRec.attr.dataPtr != identifyTime)
    {
      if ( identifyTime > 0 )
      {
        *((uint16_t*)identifyAttrRec.attr.dataPtr) = identifyTime;
        OsalPortTimers_startTimer( bdb_TaskID, BDB_IDENTIFY_TIMEOUT, 1000 );
      }
      else if ( identifyTime <= 0 )
      {
        *((uint16_t*)identifyAttrRec.attr.dataPtr) = 0;
        OsalPortTimers_stopTimer( bdb_TaskID, BDB_IDENTIFY_TIMEOUT );
      }

      if(pfnIdentifyTimeChangeCB != NULL)
      {
        pfnIdentifyTimeChangeCB(endpoint);
      }
    }
  }
}
#endif


#if (ZG_BUILD_JOINING_TYPE)
 /*********************************************************************
 * @fn          bdb_setActiveCentralizedLinkKey
 *
 * @brief       Set the active centralized key to be used, Global or IC derived.
 *
 * @param       zstack_CentralizedLinkKeyModes - Key to be used for joining centralized network
 * @param       pKey - Key to be used (if any)
 *
 * @return      ZStatus_t - ZFailure when no valid BDB_INSTALL_CODE_USE is used
 *                          ZInvalidParameter when IC buffer is null
 */
ZStatus_t bdb_setActiveCentralizedLinkKey(uint8_t zstack_CentralizedLinkKeyModes, uint8_t* pKey)
{
  ZStatus_t Status;

  uint8_t extAddr[Z_EXTADDR_LEN];

  memset(extAddr,0x00,Z_EXTADDR_LEN);

  if(pKey == NULL)
  {
    return ZInvalidParameter;
  }

  //Clear it, if the request requires it, it will be set
  gZDSECMGR_TC_ATTEMPT_DEFAULT_KEY = FALSE;

  switch(zstack_CentralizedLinkKeyModes)
  {
    case zstack_UseDefaultGlobalTrustCenterLinkKey:
      //Set the default key to be used in centralized networks as defaultTCLinkKey
      Status = APSME_SetDefaultKey();
    break;

    case zstack_UseInstallCodeWithFallback:
      //same as zstack_UseInstallCode but attempt default TRUE
      gZDSECMGR_TC_ATTEMPT_DEFAULT_KEY = TRUE;
    case zstack_UseInstallCode:
       //Set the install code as default key
      Status = bdb_addInstallCode(pKey,extAddr);
    break;

    case zstack_UseAPSKeyWithFallback:
      //same as zstack_UseAPSKey but attempt default TRUE
      gZDSECMGR_TC_ATTEMPT_DEFAULT_KEY = TRUE;
    case zstack_UseAPSKey:
      //Set the key as global default
      Status = APSME_AddTCLinkKey(pKey,extAddr);
    break;

    default:
      Status = ZInvalidParameter;
    break;
  }

  return Status;
}
#endif




/******************************************************************************
 * @fn          bdb_addInstallCode
 *
 * @brief       Interface to add an install codes and adds a APS TC Link key.
 *
 * @param       pInstallCode - [in] Install Code with CRC (buffer size of 18 bytes).
 *              pExt - [in] Extended address of the node.
 *
 * @return      ZStatus_t
 */
ZStatus_t bdb_addInstallCode(uint8_t* pInstallCode, uint8_t* pExt)
{
  uint8_t  hashOutput[16];
  uint16_t CRC;

#if (ZG_BUILD_COORDINATOR_TYPE)
  if(ZG_DEVICE_COORDINATOR_TYPE)
  {
    if(zgAllowInstallCodes == ZG_IC_NOT_SUPPORTED)
    {
      return ZFailure;
    }
  }
#endif

  if((pInstallCode == NULL) || (pExt == NULL))
  {
    return ZInvalidParameter;
  }

  CRC = bdb_GenerateInstallCodeCRC(pInstallCode);

  //Validate CRC
  if(CRC != OsalPort_buildUint16(&pInstallCode[INSTALL_CODE_LEN]))
  {
    return ZInvalidParameter;
  }

  sspMMOHash (NULL, 0, pInstallCode,(INSTALL_CODE_LEN + INSTALL_CODE_CRC_LEN) * BITS_PER_BYTE, hashOutput);

  return APSME_AddTCLinkKey(hashOutput,pExt);
}

#if (defined (USE_ICALL) || defined (OSAL_PORT2TIRTOS)) && !defined (DISABLE_GREENPOWER_BASIC_PROXY) && (ZG_BUILD_RTR_TYPE)
/*********************************************************************
* @fn      bdb_zstackTaskRegisterGPDataIndCB
*
* @brief   Register a callback to receive data indication from Green Power stub
*
* @param   secRecCB - application callback
*
* @return  none
*/
void bdb_zstackTaskRegisterGPDataIndCB(GP_DataIndGCB_t dataIndCB)
{
  GP_DataIndGCB = dataIndCB;
}

/*********************************************************************
* @fn      bdb_zstackTaskRegisterGPSecReqCB
*
* @brief   Register a callback to receive security request from Green Power stub
*
* @param   secRecCB - application callback
*
* @return  none
*/
void bdb_zstackTaskRegisterGPSecReqCB(GP_SecReqGCB_t secRecCB)
{
  GP_SecReqGCB = secRecCB;
}

/*********************************************************************
* @fn      bdb_zstackTaskRegisterGPCheckDevAnnceCB
*
* @brief   Register a callback to check a device announce
*
* @param   secRecCB - application callback
*
* @return  none
*/
void bdb_zstackTaskRegisterGPCheckDevAnnceCB(GP_CheckAnnouncedDevice_t checkDevAnnceCB)
{
  GP_CheckAnnouncedDeviceGCB = checkDevAnnceCB;
}
#endif

#if (ZG_BUILD_COORDINATOR_TYPE)
 /*********************************************************************
 * @fn      bdb_RegisterTCLinkKeyExchangeProcessCB
 *
 * @brief   Register a callback to receive notifications on the joining devices
 *          and its status on TC link key exchange
 *
 * @param   bdbGCB_TCLinkKeyExchangeProcess - application callback
 *          (extended address of device, status: 0 = Joining, 1 = TC link key exchange success, 2 = TC link key exchange failed)
 *
 * @return  none
 */
void bdb_RegisterTCLinkKeyExchangeProcessCB(bdbGCB_TCLinkKeyExchangeProcess_t bdbGCB_TCLinkKeyExchangeProcess)
{
  pfnTCLinkKeyExchangeProcessCB = bdbGCB_TCLinkKeyExchangeProcess;
}



 /*********************************************************************
 * @fn          bdb_setTCRequireKeyExchange
 *
 * @brief       Set the bdb_setTCRequireKeyExchange attribute
 *
 * @param       isKeyExchangeRequired - True if TC will remove devices that do
 *              not perform key exchange after bdbTrustCenterNodeJoinTimeout,
 *              False to not remove devices.
 *
 * @return      none
 */
void bdb_setTCRequireKeyExchange(bool isKeyExchangeRequired)
{
  bdbAttributes.bdbTrustCenterRequireKeyExchange = isKeyExchangeRequired;
}



 /*********************************************************************
 * @fn          bdb_TCAddJoiningDevice
 *
 * @brief       Add a joining device to the list of devices that must request a
 *              key before bdbTrustCenterNodeJoinTimeout.
 *
 * @param       parentAddr - Address of the parent device
 * @param       JoiningExtAddr - IEEE address of the joining device
 *
 * @return      ZStatus_t - ZFailure No memory to allocate the device in the list
 *                          ZInvalidParameter
 */
ZStatus_t bdb_TCAddJoiningDevice(uint16_t parentAddr, uint8_t* JoiningExtAddr)
{
  bdb_joiningDeviceList_t* tempJoiningDescNode;

  if((parentAddr == INVALID_NODE_ADDR) || (JoiningExtAddr == NULL))
  {
    return ZInvalidParameter;
  }

  //If the list was empty and element was allocated, then start the timer
  if(bdb_joiningDeviceList == NULL)
  {
    bdb_joiningDeviceList = OsalPort_malloc(sizeof(bdb_joiningDeviceList_t));
    if(bdb_joiningDeviceList == NULL)
    {
      return ZFailure;
    }

    OsalPortTimers_startReloadTimer(bdb_TaskID,BDB_TC_JOIN_TIMEOUT,1000);
    tempJoiningDescNode = bdb_joiningDeviceList;
  }
  //if the list was not empty then add the entry at the end of the list
  else
  {
    tempJoiningDescNode = bdb_joiningDeviceList;

    //Validate that this is not already in the list... somehow
    if(OsalPort_memcmp(JoiningExtAddr,tempJoiningDescNode->bdbJoiningNodeEui64,Z_EXTADDR_LEN))
    {
      //The device added is already in the list, refresh its time and do nothing else
      tempJoiningDescNode->NodeJoinTimeout = bdbAttributes.bdbTrustCenterNodeJoinTimeout;
      return ZSuccess;
    }

    while(tempJoiningDescNode->nextDev != NULL)
    {
      tempJoiningDescNode = tempJoiningDescNode->nextDev;

      //Validate that this is not already in the list... somehow
      if(OsalPort_memcmp(JoiningExtAddr,tempJoiningDescNode->bdbJoiningNodeEui64,Z_EXTADDR_LEN))
      {
        //The device added is already in the list, refresh its time and do nothing else
        tempJoiningDescNode->NodeJoinTimeout = bdbAttributes.bdbTrustCenterNodeJoinTimeout;
        return ZSuccess;
      }
    }

    tempJoiningDescNode->nextDev = OsalPort_malloc(sizeof(bdb_joiningDeviceList_t));
    if(tempJoiningDescNode->nextDev == NULL)
    {
      return ZFailure;
    }

    tempJoiningDescNode = tempJoiningDescNode->nextDev;
  }

  tempJoiningDescNode->nextDev = NULL;
  tempJoiningDescNode->NodeJoinTimeout = bdbAttributes.bdbTrustCenterNodeJoinTimeout;
  tempJoiningDescNode->parentAddr = parentAddr;
  OsalPort_memcpy(tempJoiningDescNode->bdbJoiningNodeEui64, JoiningExtAddr, Z_EXTADDR_LEN);

  if(pfnTCLinkKeyExchangeProcessCB)
  {
    bdb_TCLinkKeyExchProcess_t bdb_TCLinkKeyExchProcess;
    OsalPort_memcpy(bdb_TCLinkKeyExchProcess.extAddr,tempJoiningDescNode->bdbJoiningNodeEui64, Z_EXTADDR_LEN);
    AddrMgrNwkAddrLookup(tempJoiningDescNode->bdbJoiningNodeEui64, &bdb_TCLinkKeyExchProcess.nwkAddr);
    bdb_TCLinkKeyExchProcess.status = BDB_TC_LK_EXCH_PROCESS_JOINING;

    bdb_SendMsg(bdb_TaskID, BDB_TC_LINK_KEY_EXCHANGE_PROCESS, BDB_MSG_EVENT_SUCCESS,sizeof(bdb_TCLinkKeyExchProcess_t),(uint8_t*)&bdb_TCLinkKeyExchProcess);
  }

  return ZSuccess;
}

/****************************************************************************
 * @fn          bdb_TCProcessJoiningList
 *
 * @brief       Process the timer to handle the joining devices if the TC link
 *              key is mandatory for all devices
 *
 * @param       none
 *
 * @return      none
 */
void bdb_TCProcessJoiningList(void)
{
  bdb_joiningDeviceList_t* tempJoiningDescNode;

  if(bdb_joiningDeviceList)
  {
    tempJoiningDescNode = bdb_joiningDeviceList;

    while(tempJoiningDescNode)
    {
      if(tempJoiningDescNode->NodeJoinTimeout)
      {
        tempJoiningDescNode->NodeJoinTimeout--;
      }

      if(tempJoiningDescNode->NodeJoinTimeout == 0)
      {
        uint8_t isTCLKExchangeRequired = bdb_doTrustCenterRequireKeyExchange();
        //Check if the key exchange is required
        if(isTCLKExchangeRequired)
        {
            AddrMgrEntry_t entry;

            entry.user = ADDRMGR_USER_DEFAULT;
            OsalPort_memcpy(entry.extAddr,tempJoiningDescNode->bdbJoiningNodeEui64, Z_EXTADDR_LEN);

            if(AddrMgrEntryLookupExt(&entry))
            {
              ZDSecMgrAPSRemove(entry.nwkAddr,entry.extAddr,tempJoiningDescNode->parentAddr);
            }
        }

        // If we are here, a joining device has been expired due to timeout either because it is a
        // legacy device (does not perform key exchange), it is an Z3.0 device that did not perform
        // key exchange intentionally, or it is a Z3.0 device that has failed to perform key exchange.
        // Depending on our TC settings, below we decide if this joiner should be removed from the
        // security manager

        uint16_t keyNvIndex;
        uint16_t index;
        APSME_TCLinkKeyNVEntry_t TCLKDevEntry;
        uint8_t found;

        //search for the entry in the TCLK table
        keyNvIndex = APSME_SearchTCLinkKeyEntry(tempJoiningDescNode->bdbJoiningNodeEui64,&found, &TCLKDevEntry);

        uint16_t nwkAddr;
        //Look up nwkAddr before it is cleared by ZDSecMgrAddrClear
        AddrMgrNwkAddrLookup(tempJoiningDescNode->bdbJoiningNodeEui64, &nwkAddr);

        // If TC is mandating key exchange, remove devices that have not successfully performed key exchange.
        // Keep entries for ZG_PROVISIONAL_KEY so install code derived key is maintained, so joiner can reattempt join
        // If we got here and have a ZG_VERIFIED_KEY (unexpected), do not remove this entry either
        if( (isTCLKExchangeRequired == true) &&
            (TCLKDevEntry.keyAttributes != ZG_PROVISIONAL_KEY) &&
            (TCLKDevEntry.keyAttributes != ZG_VERIFIED_KEY)
          )
        {
          //Remove the entry in address manager
          ZDSecMgrAddrClear(tempJoiningDescNode->bdbJoiningNodeEui64);

          //If found, erase it.
          if(found == TRUE)
          {
            memset(&TCLKDevEntry,0,sizeof(APSME_TCLinkKeyNVEntry_t));
            TCLKDevEntry.keyAttributes = ZG_DEFAULT_KEY;

            //Increase the shift by one. Validate the maximum shift of the seed which is 15
            index = keyNvIndex;

            TCLinkKeyRAMEntry[index].rxFrmCntr = 0;
            TCLinkKeyRAMEntry[index].txFrmCntr = 0;
            TCLinkKeyRAMEntry[index].entryUsed = FALSE;

            //Update the entry
            osal_nv_write_ex( ZCD_NV_EX_TCLK_TABLE, keyNvIndex,
                              sizeof(APSME_TCLinkKeyNVEntry_t),
                              &TCLKDevEntry );
          }
        }

        if(pfnTCLinkKeyExchangeProcessCB)
        {
          bdb_TCLinkKeyExchProcess_t bdb_TCLinkKeyExchProcess;
          OsalPort_memcpy(bdb_TCLinkKeyExchProcess.extAddr,tempJoiningDescNode->bdbJoiningNodeEui64, Z_EXTADDR_LEN);
          bdb_TCLinkKeyExchProcess.nwkAddr = nwkAddr;
          bdb_TCLinkKeyExchProcess.status = BDB_TC_LK_EXCH_PROCESS_EXCH_FAIL;

          bdb_SendMsg(bdb_TaskID, BDB_TC_LINK_KEY_EXCHANGE_PROCESS, BDB_MSG_EVENT_SUCCESS,sizeof(bdb_TCLinkKeyExchProcess_t),(uint8_t*)&bdb_TCLinkKeyExchProcess);
        }
        //Free the device from the list
        bdb_TCJoiningDeviceFree(tempJoiningDescNode);
      }
      tempJoiningDescNode = tempJoiningDescNode->nextDev;
    }
  }

  //we are done with the list
  if(bdb_joiningDeviceList == NULL)
  {
    OsalPortTimers_stopTimer(bdb_TaskID,BDB_TC_JOIN_TIMEOUT);
  }
}



/****************************************************************************
 * @fn          bdb_TCjoiningDeviceComplete
 *
 * @brief       This function frees a joining device from the list that has
 *              finished TC link key exchange process
 *
 * @param       JoiningExtAddr - Extended address of the device
 *
 * @return      none
 */
void bdb_TCjoiningDeviceComplete(uint8_t* JoiningExtAddr)
{
  bdb_joiningDeviceList_t* tempJoiningDescNode;

  if((bdb_joiningDeviceList != NULL) && (JoiningExtAddr != NULL))
  {
    tempJoiningDescNode = bdb_joiningDeviceList;

    while(tempJoiningDescNode != NULL)
    {
      if(OsalPort_memcmp(tempJoiningDescNode->bdbJoiningNodeEui64,JoiningExtAddr,Z_EXTADDR_LEN))
      {
        if(pfnTCLinkKeyExchangeProcessCB)
        {
          bdb_TCLinkKeyExchProcess_t bdb_TCLinkKeyExchProcess;
          OsalPort_memcpy(bdb_TCLinkKeyExchProcess.extAddr,tempJoiningDescNode->bdbJoiningNodeEui64, Z_EXTADDR_LEN);
          AddrMgrNwkAddrLookup(tempJoiningDescNode->bdbJoiningNodeEui64, &bdb_TCLinkKeyExchProcess.nwkAddr);
          bdb_TCLinkKeyExchProcess.status = BDB_TC_LK_EXCH_PROCESS_EXCH_SUCCESS;

          bdb_SendMsg(bdb_TaskID, BDB_TC_LINK_KEY_EXCHANGE_PROCESS, BDB_MSG_EVENT_SUCCESS,sizeof(bdb_TCLinkKeyExchProcess_t),(uint8_t*)&bdb_TCLinkKeyExchProcess);
        }

        bdb_TCJoiningDeviceFree(tempJoiningDescNode);
        break;
      }
      tempJoiningDescNode = tempJoiningDescNode->nextDev;
    }

    if(bdb_joiningDeviceList == NULL)
    {
      OsalPortTimers_stopTimer(bdb_TaskID,BDB_TC_JOIN_TIMEOUT);
    }
  }
}



/****************************************************************************
 * @fn          bdb_TCJoiningDeviceFree
 *
 * @brief       This function frees a joining device from the list.
 *
 * @param       ZSuccess - If the device was found and erased
 * @param       ZInvalidParameter - Not found
 *
 * @return      none
 */
ZStatus_t bdb_TCJoiningDeviceFree(bdb_joiningDeviceList_t* JoiningDeviceToRemove)
{
  bdb_joiningDeviceList_t* descCurrent;
  bdb_joiningDeviceList_t* descPrev;

  //validate empty list?

  //Is it the first?
  if(OsalPort_memcmp(bdb_joiningDeviceList->bdbJoiningNodeEui64, JoiningDeviceToRemove->bdbJoiningNodeEui64, Z_EXTADDR_LEN))
  {
    descCurrent = bdb_joiningDeviceList;
    bdb_joiningDeviceList = bdb_joiningDeviceList->nextDev;
    OsalPort_free( descCurrent );
    return ZSuccess;
  }

  descPrev = NULL;
  descCurrent = bdb_joiningDeviceList;

  while(descCurrent != NULL)
  {
    if(OsalPort_memcmp(descCurrent->nextDev->bdbJoiningNodeEui64, JoiningDeviceToRemove->bdbJoiningNodeEui64, Z_EXTADDR_LEN))
    {
      descPrev = descCurrent;
      break;
    }
    descCurrent = descCurrent->nextDev;
  }
  if(descPrev == NULL)
  {
    //Not found
    return ZInvalidParameter;
  }

  descPrev->nextDev = descPrev->nextDev->nextDev;

  OsalPort_free( JoiningDeviceToRemove );
  return ZSuccess;

}

 /*********************************************************************
 * @fn          bdb_setJoinUsesInstallCodeKey
 *
 * @brief       Set BDB attribute bdbJoinUsesInstallCodeKey.
 *
 * @param       set - If TRUE only devices with IC register in TC can join the
 *              nwk, otherwise devices may or not have a IC register
 *
 * @return      none
 */
void bdb_setJoinUsesInstallCodeKey(bool set)
{
  bdbAttributes.bdbJoinUsesInstallCodeKey = set;
  if(set)
  {
    zgAllowInstallCodes = ZG_IC_MUST_USED;
  }
  else
  {
    zgAllowInstallCodes = ZG_IC_SUPPORTED_NOT_REQUIRED;
  }
}
#endif

 /*********************************************************************
 * @fn          bdb_StartCommissioning
 *
 * @brief       Start the commissioning process setting the commissioning mode given.
 *
 * @param       mode - refer to bdbCommissioningMode
 *
 * @return      none
 */
void bdb_StartCommissioning(uint8_t mode)
{
  //Application cannot request to set the device in initialization mode or parent lost
  mode &= ~(BDB_COMMISSIONING_MODE_INITIALIZATION | BDB_COMMISSIONING_MODE_PARENT_LOST);

#ifdef PER_TEST_DISABLE_FINDING_BINDING
  //Disable F&B to perform PER_TEST
  mode &= ~BDB_COMMISSIONING_MODE_FINDING_BINDING;
#endif

#ifdef BDB_TL_INITIATOR
  if ( touchlinkFNReset == TRUE )
  {
    return;
  }
#else
  //Commissioning mode used only for initiator
  mode &= ~BDB_COMMISSIONING_MODE_INITIATOR_TL;
#endif

#if (BDB_FINDING_BINDING_CAPABILITY_ENABLED==0)
  //Commissioning mode used only for devices with F&B
  mode &= ~BDB_COMMISSIONING_MODE_FINDING_BINDING;
#endif

  //If we have running process or the machine state is triggered, then just append and it will be excecuted
  if((bdbAttributes.bdbCommissioningMode) || (OsalPortTimers_getTimerTimeout(bdb_TaskID,BDB_CHANGE_COMMISSIONING_STATE)))
  {

      //if TC link key exchange is ongoing then do not do anything, return
      if(bdbCommissioningProcedureState.bdbTCExchangeState != BDB_TC_LINK_KEY_EXCHANGE_NOT_ACTIVE )
      {
          return;
      }

#if ZG_BUILD_ENDDEVICE_TYPE
    if(ZG_DEVICE_ENDDEVICE_TYPE)
    {
      //Devices with parent lost are not allowed to perform actions
      if(bdbAttributes.bdbCommissioningMode & BDB_COMMISSIONING_MODE_PARENT_LOST)
      {
        return;
      }
    }
#endif

    //If finished initialization, we are on the network, and got requested to do nwk steering,
    // we do not need to wait other process, just sent permit joining and report the application
    if( ((bdbAttributes.bdbNodeIsOnANetwork) && (mode & BDB_COMMISSIONING_MODE_NWK_STEERING)) &&
        !(bdbAttributes.bdbCommissioningMode & BDB_COMMISSIONING_MODE_INITIALIZATION) )
    {
      bdb_nwkSteeringDeviceOnNwk();
      bdb_reportCommissioningState(BDB_COMMISSIONING_STATE_STEERING_ON_NWK, TRUE);

      //Clean nwk steering
      mode ^= BDB_COMMISSIONING_MODE_NWK_STEERING;
    }

    //add the remaining valid commissioning modes requested, those will be process when bdb finish its current process
    bdbAttributes.bdbCommissioningMode |= mode & BDB_COMMISSIONING_MODES;
    return;
  }

  //Save the commissioning modes valid requested
  bdbAttributes.bdbCommissioningMode |= mode & BDB_COMMISSIONING_MODES;


  //Start processing the initialization, once per power cycle.
  if(!bdb_initialization)
  {
    bdb_initialization = TRUE;

#ifdef BDB_REPORTING
    //Delete NV data if startup was with factory reset
    if(ZDO_INITDEV_NEW_NETWORK_STATE == ZDApp_ReadNetworkRestoreState())
    {
      //Factory reset bdb reporting NV data
      uint16_t attrRepNvLen = osal_nv_item_len( ZCD_NV_BDBREPORTINGCONFIG );
      if ( attrRepNvLen > 0 )
      {
        osal_nv_delete( ZCD_NV_BDBREPORTINGCONFIG, attrRepNvLen );
      }
    }

    //Construct the Endpoint-cluster array
    bdb_RepConstructReportingData();
#endif //BDB_REPORTING

    osal_nv_read(ZCD_NV_BDBNODEISONANETWORK,0,sizeof(bdbAttributes.bdbNodeIsOnANetwork),&bdbAttributes.bdbNodeIsOnANetwork);

    //Are we on a network
    if(bdbAttributes.bdbNodeIsOnANetwork == TRUE)
    {
#ifdef ZG_BUILD_JOINING_TYPE
      //Only for joining devices validate the joining procedure
      if(ZG_DEVICE_JOINING_TYPE)
      {
        //If we got into a network
        if(!OsalPort_isBufSet( AIB_apsTrustCenterAddress, 0x00, Z_EXTADDR_LEN ))
        {
          //Which is not distributed
          if(!APSME_IsDistributedSecurity())
          {
              uint8_t found;
              uint16_t entryIndex;
              APSME_TCLinkKeyNVEntry_t APSME_TCLKDevEntry;

              entryIndex = APSME_SearchTCLinkKeyEntry(AIB_apsTrustCenterAddress, &found, &APSME_TCLKDevEntry);

            //If we must perform the TCLK exchange and we didn't complete it, then reset to FN
            if(requestNewTrustCenterLinkKey && (APSME_TCLKDevEntry.keyAttributes != ZG_NON_R21_NWK_JOINED) && (APSME_TCLKDevEntry.keyAttributes != ZG_VERIFIED_KEY))
            {
              if(entryIndex < gZDSECMGR_TC_DEVICE_MAX)
              {
                //Force to initialize the entry
                memset(&APSME_TCLKDevEntry,0,sizeof(APSME_TCLinkKeyNVEntry_t));
                APSME_TCLKDevEntry.keyAttributes = ZG_DEFAULT_KEY;
                osal_nv_write_ex(ZCD_NV_EX_TCLK_TABLE, entryIndex,
                                sizeof(APSME_TCLinkKeyNVEntry_t),
                                &APSME_TCLKDevEntry);

                TCLinkKeyRAMEntry[entryIndex].txFrmCntr = 0;
                TCLinkKeyRAMEntry[entryIndex].rxFrmCntr = 0;
                TCLinkKeyRAMEntry[entryIndex].entryUsed = FALSE;
              }

              //reset the device parameters to FN
              bdbAttributes.bdbNodeIsOnANetwork = FALSE;
              osal_nv_write(ZCD_NV_BDBNODEISONANETWORK, sizeof(bdbAttributes.bdbNodeIsOnANetwork), &bdbAttributes.bdbNodeIsOnANetwork);
              zgWriteStartupOptions(ZG_STARTUP_SET, ZCD_STARTOPT_DEFAULT_CONFIG_STATE | ZCD_STARTOPT_DEFAULT_NETWORK_STATE);

              //Then start the commissioning process requested
              bdbCommissioningProcedureState.bdbCommissioningState = BDB_COMMISSIONING_STATE_START_RESUME;
              OsalPort_setEvent( bdb_TaskID, BDB_CHANGE_COMMISSIONING_STATE );
              return;
            }
          }
        }
      }
#endif //ZG_BUILD_JOINING_TYPE

      //Set the initialization
      bdbAttributes.bdbCommissioningMode |= BDB_COMMISSIONING_MODE_INITIALIZATION;
      bdbCommissioningProcedureState.bdbCommissioningState = BDB_INITIALIZATION;
      bdbAttributes.bdbCommissioningMode |= mode & BDB_COMMISSIONING_MODES;

      if(ZDOInitDevice(0) == ZDO_INITDEV_RESTORED_NETWORK_STATE)
      {
#ifdef BDB_REPORTING
        //Mark the clusterEndpoint entries that have binding, starts reporting if at least one entry was marked
        bdb_RepUpdateMarkBindings();
#endif
        return;
      }
      bdb_setNodeIsOnANetwork(FALSE);
      //Not in the network
      bdb_reportCommissioningState(BDB_INITIALIZATION,FALSE);
      return;
    }
  }

  //Got requested only to initialize, if so, report that it failed
  if(bdbAttributes.bdbCommissioningMode == 0)
  {
    //Set the initialization state and report it to fail
    bdbCommissioningProcedureState.bdbCommissioningState = BDB_INITIALIZATION;
    bdb_reportCommissioningState(BDB_INITIALIZATION,FALSE);
    return;
  }


  //Start the commissioning process
  bdbCommissioningProcedureState.bdbCommissioningState = BDB_COMMISSIONING_STATE_START_RESUME;
  OsalPort_setEvent( bdb_TaskID, BDB_CHANGE_COMMISSIONING_STATE );
}


 /*********************************************************************
 * @fn          bdb_NotifyCommissioningModeStart
 *
 * @brief       Notify the user about a commissioning method just started
 *
 * @param       commissioningMode
 *
 * @return      none
 */
void bdb_NotifyCommissioningModeStart(uint8_t commissioningMode)
{
  bdbCommissioningModeMsg_t bdbCommissioningModeMsg;

  bdbCommissioningModeMsg.bdbCommissioningMode = commissioningMode;
  bdbCommissioningModeMsg.bdbCommissioningStatus = BDB_COMMISSIONING_IN_PROGRESS;
  //Remaining commissioning modes are set just before the call to the application to avoid race conditions

  bdb_NotifyApp((uint8_t*)&bdbCommissioningModeMsg);
}



#if (ZG_BUILD_JOINING_TYPE)
 /*********************************************************************
 * @fn          bdb_setNodeJoinLinkKeyType
 *
 * @brief       Set the key type in use in the network joined. Global centralized key is used by default
 *
 * @param       none
 *
 * @return      none
 */
void bdb_setNodeJoinLinkKeyType(uint8_t KeyType)
{
  bdbAttributes.bdbNodeJoinLinkKeyType = KeyType;
}
#endif

 /*********************************************************************
 * @fn          bdb_setFN
 *
 * @brief       Set configuration for FN. This FN configuration will be perfome
 *              upon call to ZDOInitDevice
 *
 * @param       none
 *
 * @return      none
 */
void bdb_setFN(void)
{
  bdb_setNodeIsOnANetwork(FALSE);

#if defined ( BDB_TL_INITIATOR )
    touchLink_InitFreeRanges( TRUE );
    touchLink_UpdateNV( TOUCHLINK_UPDATE_NV_RANGES );
#endif
#if defined ( BDB_TL_TARGET )
    touchLink_InitFreeRanges( FALSE );
    touchLink_UpdateNV( TOUCHLINK_UPDATE_NV_RANGES );
#endif

  //Set the device as factory new
  zgWriteStartupOptions(ZG_STARTUP_SET, ZCD_STARTOPT_DEFAULT_CONFIG_STATE | ZCD_STARTOPT_DEFAULT_NETWORK_STATE);
}

 /*********************************************************************
 * @fn          bdb_resetLocalAction
 *
 * @brief       Application interface to perform BDB Reset to FN.
 *
 * @param       none
 *
 * @return      none
 */
void bdb_resetLocalAction(void)
{
  //Process reset as nwk leave if the device is on the network and is able to process it
  if((ZG_BUILD_JOINING_TYPE) && (bdbAttributes.bdbNodeIsOnANetwork) && (!(bdbAttributes.bdbCommissioningMode & BDB_COMMISSIONING_MODE_PARENT_LOST)))
  {
    NLME_LeaveReq_t leaveReq;
    // Set every field to 0
    memset( &leaveReq, 0, sizeof( NLME_LeaveReq_t ) );

    bdb_setFN();

    NLME_LeaveReq( &leaveReq );

    return;
  }
  else
  {
    bdb_setFN();

    ZDApp_ResetTimerStart( 500 );
  }
}


 /*********************************************************************
 * @fn          bdb_parentLost
 *
 * @brief       Notify bdb that connection with parent is lost
 *
 * @return      none
 */
void bdb_parentLost(void)
{
#if ZG_BUILD_ENDDEVICE_TYPE
  if(ZG_DEVICE_ENDDEVICE_TYPE)
  {
    while(pBDBListNwk)
    {
      bdb_nwkDescFree(pBDBListNwk);
    }

    nwk_desc_list_free();
    if(bdbCommissioningProcedureState.bdbCommissioningState != BDB_PARENT_LOST)
    {
      //If parent lost during TCLK exchange, then report TCLK exchange fail
      if(bdbCommissioningProcedureState.bdbCommissioningState == BDB_COMMISSIONING_STATE_TC_LINK_KEY_EXCHANGE)
      {
        bdb_reportCommissioningState(BDB_COMMISSIONING_STATE_TC_LINK_KEY_EXCHANGE, FALSE);
        return;
      }
      bdbCommissioningProcedureState.bdb_ParentLostSavedState = bdbCommissioningProcedureState.bdbCommissioningState;

    }
    bdbCommissioningProcedureState.bdbCommissioningState = BDB_PARENT_LOST;
    NLME_OrphanStateSet();
    ZDApp_ChangeState( DEV_NWK_ORPHAN );

    //Stop keep alive until we rejoin
    OsalPortTimers_stopTimer(NWK_TaskID,NWK_ZED_KEEP_ALIVE_EVT);
    nwk_SetCurrentPollRateType(POLL_RATE_KEEP_ALIVE,FALSE);

    // turn receiver off while in orphan state
    byte temp = FALSE;
    ZMacSetReq(ZMacRxOnIdle, &temp);

    bdb_reportCommissioningState(BDB_PARENT_LOST,FALSE);
  }
#endif
}



 /*********************************************************************
 * @fn          bdb_NetworkRestoredResumeState
 *
 * @brief       Restore the state of child device after parent lost
 *
 * @return      none
 */
void bdb_NetworkRestoredResumeState(void)
{
#if (BDB_FINDING_BINDING_CAPABILITY_ENABLED==1)
#if ZG_BUILD_ENDDEVICE_TYPE
  if(ZG_DEVICE_ENDDEVICE_TYPE)
  {
    uint8_t restoreSimpleDesc = FALSE;
    //If restored when F&B still enabled, then restore the simple descriptors attempts
    if(bdbCommissioningProcedureState.bdbCommissioningState == BDB_COMMISSIONING_STATE_FINDING_BINDING)
    {
      bdbFindingBindingRespondent_t  *pRespondentTemp = NULL;

      pRespondentTemp = pRespondentHead;

      while(pRespondentTemp != NULL)
      {
        if(pRespondentTemp->attempts & FINDING_AND_BINDING_PARENT_LOST)
        {
          pRespondentTemp->attempts &= ~FINDING_AND_BINDING_PARENT_LOST;
          restoreSimpleDesc = TRUE;
        }
        pRespondentTemp = pRespondentTemp->pNext;
      }
    }
    if(restoreSimpleDesc)
    {
      //Restore the simple Descriptor sending after 1 second of restoring the network
      OsalPortTimers_startTimer(bdb_TaskID,BDB_RESPONDENT_PROCESS_TIMEOUT, 1000);
    }
  }
#endif
#endif
}

#if ZG_BUILD_JOINING_TYPE
 /*********************************************************************
 * @fn          bdb_recoverNwk
 *
 * @brief       Instruct a joiner to try to rejoin its previous network
 *
 * @return      success if the attempt is being executed
 *              False if device do not have nwk parameters to perform this action
 */
uint8_t bdb_recoverNwk(void)
{
  if(bdbAttributes.bdbNodeIsOnANetwork)
  {
    if(ZDOInitDevice(0) == ZDO_INITDEV_RESTORED_NETWORK_STATE)
    {
      return ZSuccess;
    }
  }
  return ZFailure;
}

#endif

 /*********************************************************************
 * @fn          bdb_reportCommissioningState
 *
 * @brief       Process the result of a BDB main state attempt.
 *
 * @param       bdbCommissioningState - MainState that is issuing fail
 * @param       didSuccess - TRUE if the main state were success, FALSE otherwise
 *
 * @return      none
 */
void bdb_reportCommissioningState(uint8_t bdbCommissioningState,bool didSuccess)
{
  bdbCommissioningModeMsg_t bdbCommissioningModeMsg;

    switch(bdbCommissioningState)
    {
#if (ZG_BUILD_JOINING_TYPE)
      case BDB_COMMISSIONING_STATE_JOINING:
        if(ZG_DEVICE_JOINING_TYPE)
        {
          //Prepare for the next state or commissioning mode to be excecuted
          OsalPortTimers_startTimer(bdb_TaskID,BDB_CHANGE_COMMISSIONING_STATE,50);

          if(didSuccess)
          {
            //Next state is TC link key exchange
            bdbCommissioningProcedureState.bdbCommissioningState = BDB_COMMISSIONING_STATE_TC_LINK_KEY_EXCHANGE;
            //Free the list of nwk discovered
            while(pBDBListNwk)
            {
              bdb_nwkDescFree(pBDBListNwk);
            }

            //No notification in this step
            return;
          }
          else
          {
            uint8_t temp = FALSE;

            bdbAttributes.bdbCommissioningStatus = BDB_COMMISSIONING_NO_NETWORK;
            bdbCommissioningModeMsg.bdbCommissioningMode = BDB_COMMISSIONING_NWK_STEERING;
            bdbCommissioningProcedureState.bdbCommissioningState = BDB_COMMISSIONING_STATE_START_RESUME;
            bdbAttributes.bdbCommissioningMode &= ~BDB_COMMISSIONING_MODE_NWK_STEERING;

            //Turn off the radio
            ZMacSetReq(ZMacRxOnIdle, &temp);
            //Set the device to FN, to start as new for subsequent attempts
            bdb_setFN();
            NLME_ResetRequest();
            ZDApp_ChangeState( DEV_HOLD );

            //Free the list of nwk discovered
            while(pBDBListNwk)
            {
              bdb_nwkDescFree(pBDBListNwk);
            }
          }
        }
      break;

      case BDB_COMMISSIONING_STATE_TC_LINK_KEY_EXCHANGE:
        if(ZG_DEVICE_JOINING_TYPE)
        {
          if(didSuccess)
          {
            //Clear any setting that would set the device as FN
            zgWriteStartupOptions(ZG_STARTUP_CLEAR, ZCD_STARTOPT_DEFAULT_CONFIG_STATE | ZCD_STARTOPT_DEFAULT_NETWORK_STATE);

            //Next state is nwk steering on the nwk (permit joining)
            bdbCommissioningProcedureState.bdbCommissioningState = BDB_COMMISSIONING_STATE_STEERING_ON_NWK;
            OsalPortTimers_startTimer(bdb_TaskID,BDB_CHANGE_COMMISSIONING_STATE, 50);

            //Disable Join poll rate as it was successful
            nwk_SetCurrentPollRateType(POLL_RATE_TYPE_JOIN_REJOIN,FALSE);

            // Reset the state back to Not-active as the process has completed
            bdbCommissioningProcedureState.bdbTCExchangeState = BDB_TC_LINK_KEY_EXCHANGE_NOT_ACTIVE;

            OsalPortTimers_stopTimer( bdb_TaskID, BDB_PROCESS_TIMEOUT );
            //No notification to the user is needed
            return;
          }
          else
          {
            bdbCommissioningModeMsg.bdbCommissioningMode = BDB_COMMISSIONING_NWK_STEERING;
            bdbAttributes.bdbCommissioningStatus = BDB_COMMISSIONING_TCLK_EX_FAILURE;

            OsalPortTimers_stopTimer( bdb_TaskID, BDB_PROCESS_TIMEOUT);

            bdb_setNodeIsOnANetwork(FALSE);

            //No process shall be attempted after this fail
            bdbAttributes.bdbCommissioningMode = 0;

            //Fill the context for the user notification
            OsalPortTimers_startTimer(bdb_TaskID,BDB_TC_LINK_KEY_EXCHANGE_FAIL,BDB_TC_LINK_KEY_EXCHANGE_FAIL_LEAVE_TIMEOUT);
          }
        }
      break;
#endif

      case BDB_COMMISSIONING_STATE_STEERING_ON_NWK:
        bdbCommissioningModeMsg.bdbCommissioningMode = BDB_COMMISSIONING_NWK_STEERING;
        if(didSuccess)
        {
          bdbAttributes.bdbCommissioningStatus = BDB_COMMISSIONING_SUCCESS;

          zgConfigPANID = _NIB.nwkPanId;
          osal_nv_write( ZCD_NV_PANID, sizeof(zgConfigPANID), &zgConfigPANID );
#if (ZG_BUILD_RTR_TYPE)
          //Update ZDApp state
          if(ZG_DEVICE_RTRONLY_TYPE)
          {
            ZDApp_ChangeState( DEV_ROUTER );
          }
#endif
#if (ZG_BUILD_ENDDEVICE_TYPE)
          if(ZG_DEVICE_ENDDEVICE_TYPE)
          {
            ZDApp_ChangeState( DEV_END_DEVICE );
          }
#endif
        }
#if (ZG_BUILD_COORDINATOR_TYPE)
        else
        {
          if(ZG_DEVICE_COORDINATOR_TYPE)
          {
            bdbAttributes.bdbCommissioningStatus = BDB_COMMISSIONING_NO_NETWORK;
          }
        }
#endif

        bdbCommissioningProcedureState.bdbCommissioningState = BDB_COMMISSIONING_STATE_START_RESUME;
        OsalPortTimers_startTimer(bdb_TaskID,BDB_CHANGE_COMMISSIONING_STATE,50);
        bdbAttributes.bdbCommissioningMode &= ~BDB_COMMISSIONING_MODE_NWK_STEERING;
      break;

      case BDB_COMMISSIONING_STATE_FORMATION:
        bdbCommissioningModeMsg.bdbCommissioningMode = BDB_COMMISSIONING_FORMATION;

        if(didSuccess)
        {
          bdbAttributes.bdbCommissioningStatus = BDB_COMMISSIONING_SUCCESS;
          zgConfigPANID = _NIB.nwkPanId;
          osal_nv_write( ZCD_NV_PANID, sizeof(zgConfigPANID), &zgConfigPANID );

          //Clear any setting that would set the device as FN
          zgWriteStartupOptions(ZG_STARTUP_CLEAR, ZCD_STARTOPT_DEFAULT_CONFIG_STATE | ZCD_STARTOPT_DEFAULT_NETWORK_STATE);

           //Update ZDApp State
#if (ZG_BUILD_RTR_TYPE)
          if(ZG_DEVICE_RTRONLY_TYPE)
          {
            ZDApp_ChangeState( DEV_ROUTER );
          }
#endif
#if (ZG_BUILD_COORDINATOR_TYPE)
          if(ZG_DEVICE_COORDINATOR_TYPE)
          {
            ZDApp_ChangeState( DEV_ZB_COORD );
          }
#endif
        }
        else
        {
          bdbAttributes.bdbCommissioningStatus = BDB_COMMISSIONING_FORMATION_FAILURE;
          //If not on the nwk, then restart the nwk parameters
#if (ZG_BUILD_RTR_TYPE)
          if(ZG_DEVICE_RTR_TYPE)
          {
            if(!bdbAttributes.bdbNodeIsOnANetwork)
            {
              uint8_t temp = FALSE;
              //Turn off the radio
              ZMacSetReq(ZMacRxOnIdle, &temp);
              //Set the device to FN, to start as new for subsequent attempts
              bdb_setFN();
              NLME_ResetRequest();
              ZDApp_ChangeState( DEV_HOLD );
            }
          }
#endif
        }
        bdbCommissioningProcedureState.bdbCommissioningState = BDB_COMMISSIONING_STATE_START_RESUME;
        OsalPortTimers_startTimer(bdb_TaskID,BDB_CHANGE_COMMISSIONING_STATE,50);
        bdbAttributes.bdbCommissioningMode &= ~BDB_COMMISSIONING_MODE_NWK_FORMATION;
      break;


      case BDB_COMMISSIONING_STATE_FINDING_BINDING:
#if (BDB_FINDING_BINDING_CAPABILITY_ENABLED==1)
        bdbCommissioningModeMsg.bdbCommissioningMode = BDB_COMMISSIONING_FINDING_BINDING;

        //Do not notify the status if we have another identify query to send
        if(bdbAttributes.bdbCommissioningStatus == BDB_COMMISSIONING_SUCCESS)
        {
          //Success at least once during F&B as initiator, mark it
          bdb_FBStateSuccessLatch = TRUE;
        }

        //Will we process another indentify?
        if(((FINDING_AND_BINDING_PERIODIC_ENABLE == FALSE)    ||
            (bdb_FB_InitiatorCurrentCyclesNumber == 0))       &&
            (bdb_getRespondentRetry(pRespondentHead) == NULL) &&
            (OsalPortTimers_getTimerTimeout( bdb_TaskID, BDB_RESPONDENT_PROCESS_TIMEOUT) == 0))
        {
          // Dealocate respondent list and clean all the F&B process
          pRespondentCurr = NULL;
          pRespondentNext = NULL;
          bdb_zclRespondentListClean( &pRespondentHead );
          OsalPortTimers_stopTimer( bdb_TaskID, BDB_RESPONDENT_PROCESS_TIMEOUT );

          //Report success if in any of the attempts we got success, regardless that we did receive no rsp on the last attempt
          if(bdb_FBStateSuccessLatch && (bdbAttributes.bdbCommissioningStatus == BDB_COMMISSIONING_FB_NO_IDENTIFY_QUERY_RESPONSE))
          {
            bdbAttributes.bdbCommissioningStatus = BDB_COMMISSIONING_SUCCESS;
          }

          //Set default state
          bdb_FBStateSuccessLatch = FALSE;

          //Resume BDB machine state only if we were in F&B, if we were on parent lost, only clean the commissioning mode and remove from bdb_ParentLostSavedState
          if(bdbCommissioningProcedureState.bdbCommissioningState == BDB_COMMISSIONING_STATE_FINDING_BINDING)
          {
            bdbCommissioningProcedureState.bdbCommissioningState = BDB_COMMISSIONING_STATE_START_RESUME;
            OsalPortTimers_startTimer(bdb_TaskID,BDB_CHANGE_COMMISSIONING_STATE,50);
          }
          else if(bdbCommissioningProcedureState.bdb_ParentLostSavedState == BDB_COMMISSIONING_STATE_FINDING_BINDING)
          {
            bdbCommissioningProcedureState.bdb_ParentLostSavedState = BDB_COMMISSIONING_STATE_START_RESUME;
          }

          bdbAttributes.bdbCommissioningMode &= ~BDB_COMMISSIONING_MODE_FINDING_BINDING;
        }
        else
        {
          return;
        }

#endif
      break;
      case BDB_COMMISSIONING_STATE_TL:
        // Set NWK task to run
        nwk_setStateIdle( FALSE );
        zgAllowRadioRxOff = TRUE; // allow radio to be turned off
        bdbCommissioningModeMsg.bdbCommissioningMode = BDB_COMMISSIONING_TOUCHLINK;
        if(didSuccess)
        {
          bdbAttributes.bdbCommissioningStatus = BDB_COMMISSIONING_SUCCESS;
          bdbAttributes.bdbCommissioningMode = BDB_COMMISSIONING_MODE_IDDLE;

          //Update ZDApp state
#if (ZG_BUILD_RTR_TYPE)
          if(ZG_DEVICE_RTRONLY_TYPE)
          {
            ZDApp_ChangeState( DEV_ROUTER );
          }
#endif
#if (ZG_BUILD_ENDDEVICE_TYPE)
          if(ZG_DEVICE_ENDDEVICE_TYPE)
          {
            ZDApp_ChangeState( DEV_END_DEVICE );
          }
#endif
#if defined ( BDB_TL_INITIATOR )
          // Establish bind links
          if(pRespondentHead != NULL)
          {
            AddrMgrEntry_t entry;

            // add the device's address information
            entry.user    = ADDRMGR_USER_BINDING;
            entry.nwkAddr = selectedTargetNwkAddr;
            osal_cpyExtAddr(entry.extAddr, selectedTargetIEEEAddr);
            AddrMgrEntryUpdate( &entry );
            pRespondentHead->data.addr.shortAddr = selectedTargetNwkAddr;
            pRespondentHead->data.addrMode = afAddr16Bit;
            pRespondentHead->data.panId = _NIB.nwkPanId;
            pRespondentHead->attempts = FINDING_AND_BINDING_FOR_TOUCHLINK_ADDED;
#if ZG_BUILD_ENDDEVICE_TYPE
            if(ZG_DEVICE_ENDDEVICE_TYPE)
            {
#if (BDB_FINDING_BINDING_CAPABILITY_ENABLED==1)
              bdb_ProcessRespondentList();
#endif // BDB_FINDING_BINDING_CAPABILITY_ENABLED
              OsalPortTimers_stopTimer( bdb_TaskID, BDB_RESPONDENT_PROCESS_TIMEOUT );
            }
#else
            if(ZG_DEVICE_RTRONLY_TYPE)
            {
              //wait until both devices are in the nwk and have established neighbor links
              OsalPortTimers_startTimer(bdb_TaskID,BDB_RESPONDENT_PROCESS_TIMEOUT, 16000);
            }
#endif // ZG_BUILD_ENDDEVICE_TYPE
          }
#endif // defined ( BDB_TL_INITIATOR )
        }

#if defined ( BDB_TL_INITIATOR )
#if (ZG_BUILD_ENDDEVICE_TYPE)
        if(ZG_DEVICE_ENDDEVICE_TYPE)
        {
            initiatorSetNwkToInitState();
        }
#endif // ZG_BUILD_ENDDEVICE_TYPE
#endif // defined ( BDB_TL_INITIATOR )

        //The fail status is already set from the calling function to report commissioning process
        // The commissioning FAIL status is set before calling the bdb_reportCommissioningState
#if defined( BDB_TL_INITIATOR ) || defined( BDB_TL_TARGET )
        touchLinkTransID = 0;
#if defined( BDB_TL_INITIATOR )
        initiatorClearSelectedTarget();
        OsalPortTimers_stopTimer(touchLinkInitiator_TaskID, TOUCHLINK_TRANS_LIFETIME_EXPIRED_EVT);
#endif
#endif
        bdbCommissioningProcedureState.bdbCommissioningState = BDB_COMMISSIONING_STATE_START_RESUME;
        OsalPortTimers_startTimer(bdb_TaskID,BDB_CHANGE_COMMISSIONING_STATE,50);
        //Clear the event
        bdbAttributes.bdbCommissioningMode &= ~BDB_COMMISSIONING_MODE_INITIATOR_TL;

      break;

      case BDB_INITIALIZATION:
        //Notify user about successfull initialization
        bdbCommissioningModeMsg.bdbCommissioningMode = BDB_COMMISSIONING_INITIALIZATION;
        if(didSuccess)
        {
          //Update ZDApp state
#if (ZG_BUILD_COORDINATOR_TYPE)
          if(ZG_DEVICE_COORDINATOR_TYPE)
          {
            ZDApp_ChangeState( DEV_ZB_COORD );
          }
#endif
#if (ZG_BUILD_ENDDEVICE_TYPE)
          if(ZG_DEVICE_ENDDEVICE_TYPE)
          {

            //Disable rejoin poll rate as initialization was successful
            nwk_SetCurrentPollRateType(POLL_RATE_TYPE_JOIN_REJOIN,FALSE);
            ZDApp_ChangeState( DEV_NWK_SEC_REJOIN_CURR_CHANNEL);

          }
#endif
          ZDApp_RestoreNwkSecMaterial();
          bdbAttributes.bdbCommissioningStatus = BDB_COMMISSIONING_NETWORK_RESTORED;
          bdbCommissioningProcedureState.bdbCommissioningState = BDB_COMMISSIONING_STATE_START_RESUME;
          OsalPortTimers_startTimer(bdb_TaskID,BDB_CHANGE_COMMISSIONING_STATE,200);
        }
        else
        {
#if (ZG_BUILD_ENDDEVICE_TYPE)
          if(ZG_DEVICE_ENDDEVICE_TYPE)
          {
            if(bdb_isDeviceNonFactoryNew())
            {
              //Notify the user about losing parent
              bdbCommissioningModeMsg.bdbCommissioningMode = BDB_COMMISSIONING_PARENT_LOST;
              bdbAttributes.bdbCommissioningMode |= BDB_COMMISSIONING_MODE_PARENT_LOST;
              //Rejoin unsuccessful, clean Rejoin pollrate
              nwk_SetCurrentPollRateType(POLL_RATE_TYPE_JOIN_REJOIN,FALSE);

              //Update ZDApp state
              ZDApp_ChangeState( DEV_NWK_ORPHAN );
            }
          }
#endif
          bdbAttributes.bdbCommissioningStatus = BDB_COMMISSIONING_NO_NETWORK;
        }
        bdbAttributes.bdbCommissioningMode &= ~BDB_COMMISSIONING_MODE_INITIALIZATION;

      break;
#if (ZG_BUILD_ENDDEVICE_TYPE)
      case BDB_PARENT_LOST:
        bdbCommissioningModeMsg.bdbCommissioningMode = BDB_COMMISSIONING_PARENT_LOST;
        if(ZG_DEVICE_ENDDEVICE_TYPE)
        {
          if(didSuccess)
          {
            bdbCommissioningProcedureState.bdbCommissioningState = bdbCommissioningProcedureState.bdb_ParentLostSavedState;
            bdbCommissioningProcedureState.bdb_ParentLostSavedState = 0;

            //Disable rejoin and "Disabled" pollrate
            nwk_SetCurrentPollRateType(POLL_RATE_DISABLED | POLL_RATE_TYPE_JOIN_REJOIN,FALSE);
            bdbAttributes.bdbCommissioningMode &= ~BDB_COMMISSIONING_MODE_PARENT_LOST;
            bdbAttributes.bdbCommissioningStatus = BDB_COMMISSIONING_NETWORK_RESTORED;
            //Update ZDApp state to type of rejoin device performed
            if(bdb_performingTCRejoin == TRUE)
            {
              ZDApp_ChangeState( DEV_NWK_TC_REJOIN_CURR_CHANNEL );
            }
            else
            {
              ZDApp_ChangeState( DEV_NWK_SEC_REJOIN_CURR_CHANNEL );
            }


            bdb_NetworkRestoredResumeState();
          }
          else
          {
            bdbAttributes.bdbCommissioningMode |= BDB_COMMISSIONING_MODE_PARENT_LOST;
            bdbAttributes.bdbCommissioningStatus = BDB_COMMISSIONING_NO_NETWORK;
          }
        }
      break;
#endif
    }
#ifdef MT_APP_CNF_FUNC
    //Notify the user about the status, the main state which has failed
    bdbCommissioningModeMsg.bdbCommissioningStatus = bdbAttributes.bdbCommissioningStatus;

    bdb_NotifyApp((uint8_t*)&bdbCommissioningModeMsg);
#else
    if(pfnCommissioningStatusCB)
    {
      //Notify the user about the status, the main state which has failed
      bdbCommissioningModeMsg.bdbCommissioningStatus = bdbAttributes.bdbCommissioningStatus;

      bdb_NotifyApp((uint8_t*)&bdbCommissioningModeMsg);
    }
#endif
}


 /*********************************************************************
 * @fn          bdb_nwkFormationAttempt
 *
 * @brief       Process a nwk formation attempt.
 *
 * @param       didSuccess - TRUE if the nwk formation was success, FALSE
 *                         otherwise and try secondary channel
 *
 * @return      none
 */
void bdb_nwkFormationAttempt(bool didSuccess)
{
  if(didSuccess)
  {
    bdb_reportCommissioningState(BDB_COMMISSIONING_STATE_FORMATION,TRUE);
  }
  else
  {
    //Can we try the secondary channel set?
    if((vDoPrimaryScan) && (bdbAttributes.bdbSecondaryChannelSet))
    {
      vDoPrimaryScan = FALSE;
      bdb_nwkJoiningFormation(FALSE);
    }
    else
    {
      bdb_reportCommissioningState(BDB_COMMISSIONING_STATE_FORMATION, FALSE);
    }
  }
}



/*********************************************************************
 * @fn          bdb_isDeviceNonFactoryNew
 *
 * @brief       Returns the state of bdbNodeIsOnANetwork attribute
 *
 * @param       none
 *
 * @return      bdbNodeIsOnANetwork
 */
bool bdb_isDeviceNonFactoryNew(void)
{
  return bdbAttributes.bdbNodeIsOnANetwork;
}


/*********************************************************************
 * @fn          bdb_doTrustCenterRequireKeyExchange
 *
 * @brief       Returns the state of bdbTrustCenterRequireKeyExchange attribute
 *
 * @param       none
 *
 * @return      bdbTrustCenterRequireKeyExchange
 */
bool bdb_doTrustCenterRequireKeyExchange(void)
{
#if (ZG_BUILD_COORDINATOR_TYPE)
  return bdbAttributes.bdbTrustCenterRequireKeyExchange;
#else
  return 0;
#endif
}

/*********************************************************************
 * @fn      bdb_rejoinNwk
 *
 * @brief   Attempt to rejoin/resume a nwk from nv parameters
 *
 * @param   none
 *
 * @return  ZStatus_t
 */
ZStatus_t bdb_rejoinNwk(void)
{
  ZStatus_t rejoinStatus = ZSuccess;

  //Update the seq number
  _NIB.SequenceNum ++;
  nwkIB_t _NIBCpy;

  osal_nv_read(ZCD_NV_NIB, 0, sizeof(nwkIB_t), &_NIBCpy);
  _NIBCpy.SequenceNum = _NIB.SequenceNum;

  osal_nv_write(ZCD_NV_NIB, sizeof(nwkIB_t), &_NIBCpy );

  // Transition state machine to correct rejoin state based on nwk key
  if ( (bdbSecureRejoinAttempts < zgBdbMaxSecureRejoinAttempts) && (ZDApp_RestoreNwkKey( FALSE ) == TRUE) )
  {
    ZDApp_ChangeState( DEV_NWK_SEC_REJOIN_CURR_CHANNEL );
  }
  else
  {
    ZDApp_ChangeState( DEV_NWK_TC_REJOIN_CURR_CHANNEL );
  }

  // Before trying to do rejoin, check if the device has a valid short address
  // If not, generate a random short address for itself
  if ( _NIB.nwkDevAddress == INVALID_NODE_ADDR )
  {
    rejoinStatus = ZFailure;
  }

  // Check if the device has a valid PanID, if not, set it to the discovered Pan
  if ( _NIB.nwkPanId == 0xFFFF )
  {
    rejoinStatus = ZFailure;
  }

  if(rejoinStatus == ZSuccess)
  {
#if ( RFD_RX_ALWAYS_ON_CAPABLE == TRUE )
    if ( ZG_DEVICE_RTR_TYPE || zgRxAlwaysOn == TRUE )
    {
      uint8_t tmp = true;
      ZMacSetReq( ZMacRxOnIdle, &tmp ); // Set receiver always on during rejoin
    }
    else
#endif
    // disable polling during rejoin scan
    {
      nwk_SetCurrentPollRateType(POLL_RATE_DISABLED, TRUE);
    }

    // Perform Secure or Unsecure Rejoin depending on available configuration
    if ( ((zgBdbAttemptUnsecureRejoin == FALSE) ||
         (bdbSecureRejoinAttempts < zgBdbMaxSecureRejoinAttempts)) &&
           (ZDApp_RestoreNwkKey( TRUE ) == TRUE) )
    {
      bdb_performingTCRejoin = FALSE;
      rejoinStatus = NLME_ReJoinRequest( ZDO_UseExtendedPANID, _NIB.nwkLogicalChannel);
      bdbSecureRejoinAttempts++;
    }
    else
    {
      bdb_performingTCRejoin = TRUE;
      rejoinStatus = NLME_ReJoinRequestUnsecure( ZDO_UseExtendedPANID, _NIB.nwkLogicalChannel);
      bdbSecureRejoinAttempts = 0;
    }
  }

  return rejoinStatus;
}

#if (ZG_BUILD_JOINING_TYPE)
 /*********************************************************************
 * @fn          bdb_nwkDiscoveryAttempt
 *
 * @brief       Process a nwk discovery attempt
 *
 * @param       didSuccess - TRUE we found nwk in the scanned channels, FALSE if
 *                           no suitable nwks were found, try secondary channel
 *
 * @return      none
 */
void bdb_nwkDiscoveryAttempt(bool didSuccess)
{
  uint8_t bdbJoinEvent = BDB_JOIN_EVENT_NWK_DISCOVERY;

  if(didSuccess)
  {
    bdb_SendMsg(bdb_TaskID, BDB_COMMISSIONING_STATE_JOINING, BDB_MSG_EVENT_SUCCESS,sizeof(bdbJoinEvent),(uint8_t*)&bdbJoinEvent);
  }
  else
  {
    //Can we try the secondary channel set?
    if((vDoPrimaryScan) && (bdbAttributes.bdbSecondaryChannelSet))
    {
      vDoPrimaryScan = FALSE;
      bdb_setChannel(bdbAttributes.bdbSecondaryChannelSet);

      ZDApp_NetworkInit( 50 );
    }
    else
    {
      bdb_reportCommissioningState(BDB_COMMISSIONING_STATE_JOINING, FALSE);
    }
  }
}

 /*********************************************************************
 * @fn          bdb_filterNwkDisc
 *
 * @brief       Filter the nwks found and attempt to join the suitable nwks
 *              Here the application can include nwk filters
 *
 * @param       none
 *
 * @return      none
 */
void bdb_filterNwkDisc(void)
{
  networkDesc_t* pNwkDesc;
  uint8_t i = 0;
  uint8_t ResultCount = 0, ResultTotal = 0;

  pBDBListNwk  = nwk_getNwkDescList();
  nwk_desc_list_release();

  pNwkDesc = pBDBListNwk;
  while (pNwkDesc)
  {
    ResultCount++;
    pNwkDesc = pNwkDesc->nextDesc;
  }

  ResultTotal = ResultCount;

  if(pBDBListNwk)
  {
    pNwkDesc = pBDBListNwk;

    if(pNwkDesc)
    {
      for ( i = 0; i < ResultTotal; i++, pNwkDesc = pNwkDesc->nextDesc )
      {
        if ( nwk_ExtPANIDValid( ZDO_UseExtendedPANID ) == true )
        {
          // If the extended Pan ID is commissioned to a non zero value
          // Only join the Pan that has match EPID
          if ( osal_ExtAddrEqual( ZDO_UseExtendedPANID, pNwkDesc->extendedPANID) == false )
          {
            //Remove from the list
            bdb_nwkDescFree(pNwkDesc);
            ResultCount--;
            continue;
          }
        }
        else if ( zgConfigPANID != 0xFFFF )
        {
          // PAN Id is preconfigured. check if it matches
          if ( pNwkDesc->panId != zgConfigPANID )
          {
            //Remove from the list
            bdb_nwkDescFree(pNwkDesc);
            ResultCount--;
            continue;
          }
        }

        if ( pNwkDesc->chosenRouter != _NIB.nwkCoordAddress || _NIB.nwkCoordAddress == INVALID_NODE_ADDR )
        {
          // check that network is allowing joining
          if ( ZSTACK_ROUTER_BUILD )
          {
            if ( !pNwkDesc->routerCapacity )
            {
              //Remove from the list
              bdb_nwkDescFree(pNwkDesc);
              ResultCount--;
              continue;
            }
          }
          else if ( ZSTACK_END_DEVICE_BUILD )
          {
            if ( !pNwkDesc->deviceCapacity )
            {
              //Remove from the list
              bdb_nwkDescFree(pNwkDesc);
              ResultCount--;
              continue;
            }
          }
        }

        // check version of zigbee protocol
        if ( pNwkDesc->version != _NIB.nwkProtocolVersion )
          continue;

        // check version of stack profile, only matching profiles are supported
        if ( pNwkDesc->stackProfile != zgStackProfile  )
        {
            //Remove from the list
            bdb_nwkDescFree(pNwkDesc);
            ResultCount--;
            continue;
        }
      }
    }
  }

  //Notify the application about the remaining valid networks to join
  if(pfnFilterNwkDesc)
  {
    pfnFilterNwkDesc(pBDBListNwk, ResultCount);
  }
}

 /*********************************************************************
 * @fn          bdb_tryNwkAssoc
 *
 * @brief       Try to associate to the first network in the network descriptor list
 *
 * @param       none
 *
 * @return      none
 */
void bdb_tryNwkAssoc(void)
{
  if(pBDBListNwk)
  {
    bdbCommissioningProcedureState.bdbJoinState = BDB_JOIN_STATE_ASSOC;

    //Try the first in the list after the filtering
    if(ZSuccess != bdb_joinProcess(pBDBListNwk))
    {
      //If fail, free the first in the list and prepare for futher processing, either next nwk or discover again
      uint8_t bdbJoinEvent = BDB_JOIN_EVENT_ASSOCIATION;
      bdb_nwkDescFree(pBDBListNwk);
      bdb_SendMsg(bdb_TaskID,BDB_COMMISSIONING_STATE_JOINING,BDB_MSG_EVENT_FAIL,sizeof(uint8_t),&bdbJoinEvent);
    }
  }
  else
  {
    bdbCommissioningProcedureState.bdbJoinState = BDB_JOIN_STATE_NWK_DISC;
    uint8_t bdbJoinEvent = BDB_JOIN_EVENT_NWK_DISCOVERY;

    bdb_SendMsg(bdb_TaskID,BDB_COMMISSIONING_STATE_JOINING,BDB_MSG_EVENT_FAIL,sizeof(uint8_t),&bdbJoinEvent);
  }
}



 /*********************************************************************
 * @fn          bdb_nwkAssocAttemt
 *
 * @brief       Process the result of an attempt to associate to a network
 *
 * @param       didSuccess - bool
 *
 * @return      none
 */
void bdb_nwkAssocAttemt(bool didSuccess)
{
  bdbAttributes.bdbCommissioningStatus = BDB_COMMISSIONING_STATE_JOINING;
  uint8_t bdbJoinEvent = BDB_JOIN_EVENT_ASSOCIATION;
  uint8_t status;

  if(didSuccess)
  {
    status = BDB_MSG_EVENT_SUCCESS;
  }
  else
  {
    if(bdb_nwkAssocRetriesCount < BDBC_REC_SAME_NETWORK_RETRY_ATTEMPS)
    {
      bdb_nwkAssocRetriesCount++;
    }
    else
    {
      //Free the first in the list and prepare for futher processing
      bdb_nwkDescFree(pBDBListNwk);
      bdb_nwkAssocRetriesCount = 0;
    }
    status = BDB_MSG_EVENT_FAIL;
  }
  bdb_SendMsg(bdb_TaskID,BDB_COMMISSIONING_STATE_JOINING,status,sizeof(uint8_t),&bdbJoinEvent);
}


/****************************************************************************
 * @fn          bdb_nwkDescFree
 *
 * @brief       This function frees one network discovery data.
 *
 * @param       ZSuccess - If the device was found and erased
 * @param       ZInvalidParameter - Not found
 *
 * @return      none
 */
ZStatus_t bdb_nwkDescFree(networkDesc_t* nodeDescToRemove)
{
  networkDesc_t* current_desc;
  networkDesc_t* prev_desc;

  current_desc = pBDBListNwk;

  while(current_desc != NULL)
  {
    if(current_desc == nodeDescToRemove)
    {
      if (current_desc == pBDBListNwk)
      {
        pBDBListNwk = pBDBListNwk->nextDesc;
      }
      else
      {
        prev_desc->nextDesc = current_desc->nextDesc;
      }

      OsalPort_free( current_desc );

      return ZSuccess;
    }

    prev_desc = current_desc;
    current_desc = current_desc->nextDesc;
  }

  return ZInvalidParameter;
}

/*********************************************************************
* @fn          bdb_joinProcess
*
* @brief       Start the joining process for the selected nwk
*
* @return      ZStatus_t
*/
ZStatus_t bdb_joinProcess(networkDesc_t *pChosenNwk)
{
  ZStatus_t status;

  ZDApp_ChangeState( DEV_NWK_JOINING );
  ZDApp_NodeProfileSync( pChosenNwk->stackProfile);

  status =  NLME_JoinRequest( pChosenNwk->extendedPANID, pChosenNwk->panId,
                        pChosenNwk->logicalChannel,
                        ZDO_Config_Node_Descriptor.CapabilityFlags,
                        pChosenNwk->chosenRouter, pChosenNwk->chosenRouterDepth );

  if(status == ZSuccess)
  {
    // The receiver is on, turn network layer polling off.
    if ( ZDO_Config_Node_Descriptor.CapabilityFlags & CAPINFO_RCVR_ON_IDLE )
    {
      nwk_SetCurrentPollRateType(POLL_RATE_RX_ON_TRUE,TRUE);
    }
    else
    {
      if (devStartMode == MODE_JOIN)
      {
        nwk_SetCurrentPollRateType(POLL_RATE_TYPE_JOIN_REJOIN,TRUE);
      }
    }
  }
  return status;
}
#endif


 /*********************************************************************
 * @fn          bdb_setChannelAttribute
 *
 * @brief       Set the primary or secondary channel for discovery or formation procedure
 *
 * @param       isPrimaryChannel - True if channel to set is primary,
 *                                 False if the channel to set is secondary
 *
 * @param       channel - Channel mask
 *
 * @return      none
 */
void bdb_setChannelAttribute(bool isPrimaryChannel, uint32_t channel)
{
  if(isPrimaryChannel)
  {
    bdbAttributes.bdbPrimaryChannelSet = channel;
  }
  else
  {
    bdbAttributes.bdbSecondaryChannelSet = channel;
  }
}

 /*********************************************************************
 * @fn          bdb_setChannel
 *
 * @brief       Set channel and save it in Nv for joining/formation operations
 *
 * @param       channel - Channel mask
 *
 * @return      none
 */
void bdb_setChannel(uint32_t channel)
{
  //Assign the channel and save it into nv

  vScanChannels = channel;
  runtimeChannel = channel;

  osal_nv_write(ZCD_NV_CHANLIST, sizeof(uint32_t), &vScanChannels);     //Zstackapi_sysConfigWriteReq
}


 /*********************************************************************
 * @fn          bdb_nwkJoiningFormation
 *
 * @brief       Performs Joining/Formation operation on primary or secondary channel
 *
 * @param       isJoining - TRUE if the device is performing joining, FALSE is performing Formation
 *
 * @return      none
 */
void bdb_nwkJoiningFormation(bool isJoining)
{

  if((vDoPrimaryScan) && (bdbAttributes.bdbPrimaryChannelSet))
  {
    bdb_setChannel(bdbAttributes.bdbPrimaryChannelSet);
  }
  else
  {
    vDoPrimaryScan = FALSE;
    bdb_setChannel(bdbAttributes.bdbSecondaryChannelSet);
  }

  if(vScanChannels)
  {
    if(ZG_DEVICE_RTRONLY_TYPE)
    {
      if(isJoining)
      {
        ZDOInitDeviceEx(100,0);
      }
      else
      {
        ZDOInitDeviceEx(100,1);
      }
    }
    //ZED can only join, and ZC can only create
    else
    {
      ZDOInitDeviceEx(100,0);
    }
  }
  else
  {
    if(isJoining)
    {
      bdb_reportCommissioningState(BDB_COMMISSIONING_STATE_JOINING, FALSE);
    }
    else
    {
      bdb_reportCommissioningState(BDB_COMMISSIONING_STATE_FORMATION, FALSE);
    }
  }
}

#if (ZG_BUILD_JOINING_TYPE)
void bdb_changeTCExchangeState( uint8_t bdbTCExchangeState )
{
  bdbCommissioningProcedureState.bdbTCExchangeState = bdbTCExchangeState;
}

 /*********************************************************************
 * @fn          bdb_tcLinkKeyExchangeAttempt
 *
 * @brief       Generic send msg for TC link key exchange process attempts
 *
 * @param       didSuccess - FALSE if the step failed/timeout, TRUE otherwise
 * @param       bdbTCExchangeState - Step in which the attemp was done
 *
 * @return      none
 */
void bdb_tcLinkKeyExchangeAttempt(bool didSuccess, uint8_t bdbTCExchangeState)
{
  bool bdbEventStatus = BDB_MSG_EVENT_SUCCESS;
  uint8_t dummy;
  bdbCommissioningProcedureState.bdbTCExchangeState = bdbTCExchangeState;
  if(didSuccess)
  {
    //Allow try since we are performing a new step.
    OsalPortTimers_stopTimer(bdb_TaskID, BDB_PROCESS_TIMEOUT);
    bdbAttributes.bdbTCLinkKeyExchangeAttempts = 0;
  }
  else
  {
    bdb_acceptNewTrustCenterLinkKey = FALSE;
    bdbEventStatus = BDB_MSG_EVENT_FAIL;
  }
  bdb_SendMsg(bdb_TaskID,BDB_COMMISSIONING_STATE_TC_LINK_KEY_EXCHANGE,bdbEventStatus,1, &dummy);
}


 /*********************************************************************
 * @fn          bdb_requestVerifyTCLinkKey
 *
 * @brief       Attempt to verify the TC link key by sending Verify Key Request
 *
 * @param       none
 *
 * @return      none
 */
void bdb_requestVerifyTCLinkKey(void)
{
    uint8_t TC_ExtAddr[Z_EXTADDR_LEN];
    APSME_VerifyKeyReq_t vKey;

    APSME_GetRequest( apsTrustCenterAddress,0, TC_ExtAddr );

    vKey.tcExtAddr = TC_ExtAddr;
    vKey.keyType = KEY_TYPE_TC_LINK;

    APSME_VerifyKeyReq( &vKey );

    OsalPortTimers_stopTimer(bdb_TaskID,BDB_PROCESS_TIMEOUT);
    OsalPortTimers_startTimer(bdb_TaskID,BDB_PROCESS_TIMEOUT,BDBC_TC_LINK_KEY_EXANGE_TIMEOUT);

}

/*********************************************************************
 * @fn          bdb_requestTCLinkKey
 *
 * @brief       Attempt to request a TC link key
 *
 * @param       none
 *
 * @return      none
 */
void bdb_requestTCLinkKey(void)
{
  zAddrType_t destAddr;
  APSME_RequestKeyReq_t req;

  destAddr.addrMode = Addr16Bit;
  destAddr.addr.shortAddr = 0x0000;

  req.dstAddr = destAddr.addr.shortAddr;
  req.keyType = KEY_TYPE_TC_LINK;

  // joiner is requesting a key, so we expect an incoming response
  bdb_acceptNewTrustCenterLinkKey = TRUE;

  APSME_RequestKeyReq(&req);

  OsalPortTimers_stopTimer(bdb_TaskID,BDB_PROCESS_TIMEOUT);

  OsalPortTimers_startTimer(bdb_TaskID,BDB_PROCESS_TIMEOUT,(uint32_t)requestLinkKeyTimeout);
}


/*********************************************************************
 * @fn          bdb_requestTCStackVersion
 *
 * @brief       Attempt to request the TC stack version using ZDP Node desc if
 *              join a Centralized nwk
 *
 * @param       none
 *
 * @return      none
 */
void bdb_requestTCStackVersion(void)
{
  if(requestNewTrustCenterLinkKey)
  {
    if(!APSME_IsDistributedSecurity())
    {
      if(bdbAttributes.bdbTCLinkKeyExchangeMethod == BDB_TC_LINK_KEY_EXCHANGE_APS_KEY)
      {
        zAddrType_t destAddr;

        destAddr.addrMode = Addr16Bit;
        destAddr.addr.shortAddr = 0x0000;

        ZDP_NodeDescReq( &destAddr, destAddr.addr.shortAddr, 0);

        OsalPortTimers_stopTimer(bdb_TaskID,BDB_PROCESS_TIMEOUT);
        OsalPortTimers_startTimer( bdb_TaskID, BDB_PROCESS_TIMEOUT, BDBC_TC_LINK_KEY_EXANGE_TIMEOUT );
        return;
      }
      else
      {
        if(pfnCBKETCLinkKeyExchange)
        {
          pfnCBKETCLinkKeyExchange();
        }
        return;
      }
    }
    else
    {
      bdb_setNodeJoinLinkKeyType(BDB_DISTRIBUTED_SECURITY_GLOBAL_LINK_KEY);
    }
  }
  else
  {
    //Key not required, set default which is global
    bdb_setNodeJoinLinkKeyType(BDB_DEFAULT_GLOBAL_TRUST_CENTER_LINK_KEY);
  }
  //TC link key not required or join distributed nwk
  bdb_reportCommissioningState(BDB_COMMISSIONING_STATE_TC_LINK_KEY_EXCHANGE,TRUE);
}
#endif


/*********************************************************************
 * @fn          bdb_nwkSteeringDeviceOnNwk
 *
 * @brief       Send ZDP mgmt permit joining
 *
 * @param       none
 *
 * @return      none
 */
void bdb_nwkSteeringDeviceOnNwk(void)
{
  zAddrType_t dstAddr;
  dstAddr.addr.shortAddr = NWK_BROADCAST_SHORTADDR_DEVZCZR;
  dstAddr.addrMode = AddrBroadcast;
  // Trust Center significance is always true
  ZDP_MgmtPermitJoinReq( &dstAddr, BDBC_MIN_COMMISSIONING_TIME, TRUE, FALSE );
}


/*********************************************************************
 * @fn          bdb_startResumeCommissioningProcess
 *
 * @brief       Starts or resume the commissioning operations sets in the
 *              commissioningMode attribute
 *
 * @param       none
 *
 * @return      none
 */
void bdb_startResumeCommissioningProcess(void)
{

#if defined BDB_TL_INITIATOR
  if(bdbAttributes.bdbCommissioningMode & BDB_COMMISSIONING_MODE_INITIATOR_TL)
  {
    uint16_t nwkAddr;

    //Does the device supports this commissioning mode?
    if(bdbAttributes.bdbNodeCommissioningCapability & BDB_TOUCHLINK_CAPABILITY)
    {
      //Clear previous state and substates
      memset(&bdbCommissioningProcedureState,0,sizeof(bdbCommissioningProcedureState));
      bdbCommissioningProcedureState.bdbCommissioningState = BDB_COMMISSIONING_STATE_TL;

      // Get our short address
      ZMacGetReq( ZMacShortAddress, (byte*)&nwkAddr );

      // Check if address is invalid
      if ( nwkAddr >= NWK_BROADCAST_SHORTADDR_DEVZCZR )
      {
        initiatorSelectNwkParams();
      }

      touchLinkInitiator_StartDevDisc( );

      bdb_NotifyCommissioningModeStart(BDB_COMMISSIONING_TOUCHLINK);
    }
    else
    {
      //Process the next commissioning mode
      bdb_reportCommissioningState( BDB_COMMISSIONING_STATE_TL, FALSE );
    }
    return;
  }
#endif // BDB_TOUCHLINK_CAPABILITY_ENABLED

#if ZG_BUILD_ENDDEVICE_TYPE
  if(ZG_DEVICE_ENDDEVICE_TYPE)
  {
    if(bdbAttributes.bdbCommissioningMode & BDB_COMMISSIONING_MODE_PARENT_LOST)
    {
      //No commissioning process can be performed if the ZED has lost its parent
      return;
    }
  }
#endif

  if(bdbAttributes.bdbCommissioningMode & BDB_COMMISSIONING_MODE_NWK_FORMATION)
  {
    bdbCommissioningProcedureState.bdbCommissioningState = BDB_COMMISSIONING_STATE_FORMATION;

    if(bdbAttributes.bdbNodeCommissioningCapability & BDB_NETWORK_FORMATION_CAPABILITY)
    {
      if(!bdbAttributes.bdbNodeIsOnANetwork)
      {
#if (BDB_TOUCHLINK_CAPABILITY_ENABLED == TRUE)
      bdb_ClearNetworkParams();
#endif
        vDoPrimaryScan = TRUE;

        memset(&bdbCommissioningProcedureState,0,sizeof(bdbCommissioningProcedureState));
        bdbCommissioningProcedureState.bdbCommissioningState = BDB_COMMISSIONING_STATE_FORMATION;

        bdb_nwkJoiningFormation(FALSE);
        bdb_NotifyCommissioningModeStart(BDB_COMMISSIONING_FORMATION);
        return;
      }
    }
    bdb_reportCommissioningState(BDB_COMMISSIONING_STATE_FORMATION, FALSE);
    return;
  }

  if(bdbAttributes.bdbCommissioningMode & BDB_COMMISSIONING_MODE_NWK_STEERING)
  {
    bdbCommissioningProcedureState.bdbCommissioningState = BDB_COMMISSIONING_STATE_STEERING_ON_NWK;

    if(bdbAttributes.bdbNodeCommissioningCapability & BDB_NETWORK_STEERING_CAPABILITY)
    {
#if (BDB_TOUCHLINK_CAPABILITY_ENABLED == TRUE)
      bdb_ClearNetworkParams();
#endif
      if(bdbAttributes.bdbNodeIsOnANetwork)
      {
        bdb_nwkSteeringDeviceOnNwk();
        bdb_reportCommissioningState(BDB_COMMISSIONING_STATE_STEERING_ON_NWK, TRUE);
      }
#if (ZG_BUILD_JOINING_TYPE)
      else
      {
        if(ZG_DEVICE_JOINING_TYPE)
        {
          vDoPrimaryScan = TRUE;

          //Initialize the commissioning procedure state, bdbJoinState to nwk discovery and TCLinkKeyExchange to not active
          memset(&bdbCommissioningProcedureState,0,sizeof(bdbCommissioningProcedureState_t));
          bdbCommissioningProcedureState.bdbCommissioningState = BDB_COMMISSIONING_STATE_JOINING;
          bdb_nwkJoiningFormation(TRUE);
          bdb_NotifyCommissioningModeStart(BDB_COMMISSIONING_NWK_STEERING);
        }
      }
#endif
#if (ZG_BUILD_COORDINATOR_TYPE)
      if(ZG_DEVICE_COORDINATOR_TYPE)
      {
        bdb_reportCommissioningState(BDB_COMMISSIONING_STATE_STEERING_ON_NWK, FALSE);
      }
#endif
    }
    return;
  }

#if (BDB_FINDING_BINDING_CAPABILITY_ENABLED==1)
  if(bdbAttributes.bdbCommissioningMode & BDB_COMMISSIONING_MODE_FINDING_BINDING)
  {
    bdbCommissioningProcedureState.bdbCommissioningState = BDB_COMMISSIONING_STATE_FINDING_BINDING;

    //Is the device on a network?
    if(bdb_isDeviceNonFactoryNew())
    {
      zclAttrRec_t attrRec;

      endPointDesc_t *bdb_CurrEpDescriptor = NULL;

      bdb_CurrEpDescriptor = bdb_setEpDescListToActiveEndpoint();

      //If not found endpoint with Identify cluster is found, then report fail
      if(bdb_CurrEpDescriptor == NULL)
      {
        bdb_exitFindingBindingWStatus(BDB_COMMISSIONING_FAILURE);
        return;
      }

      if( bdb_CurrEpDescriptorList->epDesc->epType & BDB_FINDING_AND_BINDING_TARGET)  //F&B as Target
      {
        if (zclFindAttrRec( bdb_CurrEpDescriptor->endPoint, ZCL_CLUSTER_ID_GENERAL_IDENTIFY,
                  ATTRID_IDENTIFY_IDENTIFY_TIME, &attrRec ) )
        {
          //Set it to at less 180
          if ( *((uint16_t*)attrRec.attr.dataPtr) <= BDBC_MIN_COMMISSIONING_TIME )
          {
            *((uint16_t*)attrRec.attr.dataPtr) = BDBC_MIN_COMMISSIONING_TIME;
             OsalPortTimers_startTimer( bdb_TaskID, BDB_IDENTIFY_TIMEOUT, 1000 );

            if(pfnIdentifyTimeChangeCB != NULL)
            {
              if(bdbIndentifyActiveEndpoint == 0xFF)
              {
                pfnIdentifyTimeChangeCB(bdbIndentifyActiveEndpoint);
              }
              else
              {
                pfnIdentifyTimeChangeCB(bdb_CurrEpDescriptor->endPoint);
              }
            }
          }
          //Attribute found and set, report success
          if(!(bdb_CurrEpDescriptorList->epDesc->epType & BDB_FINDING_AND_BINDING_INITIATOR))
          {
            bdb_exitFindingBindingWStatus(BDB_COMMISSIONING_FB_TARGET_IN_PROGRESS);
          }
          else
          {
            bdbCommissioningModeMsg_t bdbCommissioningModeMsg;

            bdbCommissioningModeMsg.bdbCommissioningMode = BDB_COMMISSIONING_FINDING_BINDING;
            bdbCommissioningModeMsg.bdbCommissioningStatus = BDB_COMMISSIONING_FB_TARGET_IN_PROGRESS;

            bdb_NotifyApp((uint8_t*)&bdbCommissioningModeMsg);
          }
        }
        else
        {
          //Attribute not found and no initiator process, report fail
          if(!(bdb_CurrEpDescriptorList->epDesc->epType & BDB_FINDING_AND_BINDING_INITIATOR))
          {
            bdb_exitFindingBindingWStatus(BDB_COMMISSIONING_FAILURE);
          }
        }
      }  //F&B Target

      if( bdb_CurrEpDescriptorList->epDesc->epType & BDB_FINDING_AND_BINDING_INITIATOR)  //F&B as Initiator
      {
        bdbCommissioningModeMsg_t bdbCommissioningModeMsg;

        //If no function to add binds is available then do not process Initiator
        if(pbindAddEntry == NULL)
        {
          //If no target process, then report fail
          if(!(bdb_CurrEpDescriptorList->epDesc->epType & BDB_FINDING_AND_BINDING_TARGET))
          {
            bdb_exitFindingBindingWStatus(BDB_COMMISSIONING_FAILURE);
          }
        }
        else
        {
          //Send identify query with the endpoint requested
          if(bdb_SendIdentifyQuery(bdb_CurrEpDescriptor->endPoint) != ZSuccess)
          {
            bdb_exitFindingBindingWStatus(BDB_COMMISSIONING_FAILURE);
          }

          //If periodic F&B is enabled
          if ( FINDING_AND_BINDING_PERIODIC_ENABLE == TRUE )
          {
            // total F&B time will be at least BDBC_MIN_COMMISSIONING_TIME, and at most (BDBC_MIN_COMMISSIONING_TIME + FINDING_AND_BINDING_PERIODIC_TIME - 1)
            bdb_FB_InitiatorCurrentCyclesNumber = (BDBC_MIN_COMMISSIONING_TIME + (FINDING_AND_BINDING_PERIODIC_TIME - 1)) / FINDING_AND_BINDING_PERIODIC_TIME;

            OsalPortTimers_startTimer(bdb_TaskID, BDB_FINDING_AND_BINDING_PERIOD_TIMEOUT, FINDING_AND_BINDING_PERIODIC_TIME * 1000);
          }

          bdbCommissioningModeMsg.bdbCommissioningMode = BDB_COMMISSIONING_FINDING_BINDING;
          bdbCommissioningModeMsg.bdbCommissioningStatus = BDB_COMMISSIONING_FB_INITITATOR_IN_PROGRESS;

          bdb_NotifyApp((uint8_t*)&bdbCommissioningModeMsg);
        }
      } //F&B Initiator
    }
    //Not in the network
    else
    {
      bdb_exitFindingBindingWStatus(BDB_COMMISSIONING_FAILURE);
    }

    return;
  }
#endif

}

/*********************************************************************
 * @fn          bdb_event_loop
 *
 * @brief       Main event loop bdb tasks.
 *
 * @param       task_id - task id
 * @param       events - event bitmap
 *
 * @return      unprocessed events
 */
uint32_t bdb_event_loop(byte task_id, uint32_t events)
{
  (void)task_id;  // Intentionally unreferenced parameter

#if (BDB_FINDING_BINDING_CAPABILITY_ENABLED==1)
  endPointDesc_t * bdb_CurrEpDescriptor;
#endif

  if(events & BDB_CHANGE_COMMISSIONING_STATE)
  {
    switch(bdbCommissioningProcedureState.bdbCommissioningState)
    {
      case BDB_COMMISSIONING_STATE_START_RESUME:
        bdb_startResumeCommissioningProcess();
      break;

      case BDB_COMMISSIONING_STATE_TC_LINK_KEY_EXCHANGE:
        if (ZG_BUILD_JOINING_TYPE)
        {
          bdb_tcLinkKeyExchangeAttempt(TRUE,BDB_REQ_TC_STACK_VERSION);
        }
      break;

      case BDB_COMMISSIONING_STATE_STEERING_ON_NWK:
        bdb_nwkSteeringDeviceOnNwk();

        bdb_reportCommissioningState(BDB_COMMISSIONING_STATE_STEERING_ON_NWK, TRUE);
      break;

      case BDB_COMMISSIONING_STATE_FINDING_BINDING:
        bdbAttributes.bdbCommissioningStatus = BDB_COMMISSIONING_SUCCESS;
        bdbCommissioningProcedureState.bdbCommissioningState = BDB_COMMISSIONING_STATE_START_RESUME;
        OsalPortTimers_startTimer(bdb_TaskID,BDB_CHANGE_COMMISSIONING_STATE,50);
      break;

    }
    return (events ^ BDB_CHANGE_COMMISSIONING_STATE);
  }

  if ( events & SYS_EVENT_MSG )
  {
    uint8_t *msg_ptr;

    while ( (msg_ptr = OsalPort_msgReceive( bdb_TaskID )) )
    {
      //Process the Incomming ZDO messages used by BDB commissioning methods
      if(((bdbInMsg_t*)msg_ptr)->hdr.event == BDB_ZDO_CB_MSG)
      {
        bdb_processZDOMgs((zdoIncomingMsg_t *)msg_ptr);
      }

      //Validate the is receive on the right process
      else if(((bdbInMsg_t*)msg_ptr)->hdr.event == bdbCommissioningProcedureState.bdbCommissioningState)
      {
        bdb_ProcessOSALMsg( (bdbInMsg_t *)msg_ptr );
      }
      //Notify the user
      else if(((bdbInMsg_t*)msg_ptr)->hdr.event == BDB_NOTIFY_USER)
      {
        ((bdbCommissioningModeMsg_t*) ((bdbInMsg_t*)msg_ptr)->buf)->bdbRemainingCommissioningModes = bdbAttributes.bdbCommissioningMode;
        if(pfnCommissioningStatusCB)
        {
          pfnCommissioningStatusCB((bdbCommissioningModeMsg_t*) (((bdbInMsg_t*)msg_ptr)->buf));
        }
#ifdef MT_APP_CNF_FUNC
        //Notify the host processor about the event
        MT_AppCnfCommissioningNotification((bdbCommissioningModeMsg_t*) (((bdbInMsg_t*)msg_ptr)->buf));
#endif
      }
#if (ZG_BUILD_COORDINATOR_TYPE)
      else
      {
        if(ZG_DEVICE_COORDINATOR_TYPE)
        {
          //Notify the status
          if(((bdbInMsg_t*)msg_ptr)->hdr.event == BDB_TC_LINK_KEY_EXCHANGE_PROCESS)
          {
            pfnTCLinkKeyExchangeProcessCB( (bdb_TCLinkKeyExchProcess_t*) ((bdbInMsg_t*)msg_ptr)->buf);
          }
        }
      }
#endif
      // Release the memory
      OsalPort_msgDeallocate( msg_ptr );
    }

    // Return unprocessed events
    return (events ^ SYS_EVENT_MSG);
  }


  if(events & BDB_PROCESS_TIMEOUT)
  {
    bdb_processTimeout();
    // Return unprocessed events
    return (events ^ BDB_PROCESS_TIMEOUT);
  }

  if(events &  BDB_REPORT_TIMEOUT){
#ifdef BDB_REPORTING
    bdb_RepProcessEvent();
#endif
    // Return unprocessed events
    return (events ^ BDB_REPORT_TIMEOUT);
  }

#if (ZG_BUILD_JOINING_TYPE)
  if(events & BDB_TC_LINK_KEY_EXCHANGE_FAIL)
  {
    if(ZG_DEVICE_JOINING_TYPE)
    {
      NLME_LeaveReq_t leaveReq;
      // Set every field to 0
      memset( &leaveReq, 0, sizeof( NLME_LeaveReq_t ) );

      if ( NLME_LeaveReq( &leaveReq ) != ZSuccess )
      {
        OsalPort_setEvent( bdb_TaskID,BDB_TC_LINK_KEY_EXCHANGE_FAIL);
      }
    }
    // Return unprocessed events
    return (events ^ BDB_TC_LINK_KEY_EXCHANGE_FAIL);
  }
#endif

  if(events & BDB_TC_JOIN_TIMEOUT)
  {
#if (ZG_BUILD_COORDINATOR_TYPE)
    if(ZG_DEVICE_COORDINATOR_TYPE)
    {
      bdb_TCProcessJoiningList();
    }
#endif
    return (events ^ BDB_TC_JOIN_TIMEOUT);
  }

#if (BDB_FINDING_BINDING_CAPABILITY_ENABLED==1)

  if(events & BDB_FINDING_AND_BINDING_PERIOD_TIMEOUT)
  {
    if ( FINDING_AND_BINDING_PERIODIC_ENABLE == TRUE )
    {
      bdb_CurrEpDescriptor = bdb_setEpDescListToActiveEndpoint();

      //If we have endpoint from which to send the identify command, then proceed, otherwise finish
      if(bdb_CurrEpDescriptor != NULL) //just a safty check. The fact that we got to this functuon at all means that this cannot be NULL
      {
        //Substract an attempt
        bdb_FB_InitiatorCurrentCyclesNumber--;

        if(bdb_FB_InitiatorCurrentCyclesNumber > 0)
        {
          //Only send Identify Query if there is no pending responses from a previous identify query
          if ((OsalPortTimers_getTimerTimeout(bdb_TaskID, BDB_RESPONDENT_PROCESS_TIMEOUT) == 0) && (bdb_getRespondentRetry(pRespondentHead) == NULL))
          {
            //Send identify query with the endpoint requested
            bdb_SendIdentifyQuery(bdb_CurrEpDescriptor->endPoint);
          }
          OsalPortTimers_startTimer(bdb_TaskID, BDB_FINDING_AND_BINDING_PERIOD_TIMEOUT, FINDING_AND_BINDING_PERIODIC_TIME * 1000);
        }
      }
    }

    if (bdb_FB_InitiatorCurrentCyclesNumber == 0)
    {
      bdb_exitFindingBindingWStatus( BDB_COMMISSIONING_FB_NO_IDENTIFY_QUERY_RESPONSE );
    }

    return (events ^ BDB_FINDING_AND_BINDING_PERIOD_TIMEOUT);
  }

  if(events & BDB_IDENTIFY_TIMEOUT)
  {
    zclAttrRec_t identifyAttrRec;
    epList_t *bdb_CurrEpDescriptorNextInList = NULL;
    bdb_CurrEpDescriptorNextInList = bdb_HeadEpDescriptorList;

    bool KeepIdentifyTimerRunning = FALSE;

    while(bdb_CurrEpDescriptorNextInList != NULL )
    {
      endPointDesc_t *bdb_EpDescriptor = NULL;
      bdb_EpDescriptor = bdb_CurrEpDescriptorNextInList->epDesc;

      //Do not check ZDO or Zigbee reserved endpoints
      if((bdb_CurrEpDescriptorNextInList->epDesc->endPoint == 0) || (bdb_CurrEpDescriptorNextInList->epDesc->endPoint >= BDB_ZIGBEE_RESERVED_ENDPOINTS_START))
      {
        bdb_CurrEpDescriptorNextInList = bdb_CurrEpDescriptorNextInList->nextDesc;
        continue;
      }

      if ( zclFindAttrRec( bdb_EpDescriptor->endPoint, ZCL_CLUSTER_ID_GENERAL_IDENTIFY,
                        ATTRID_IDENTIFY_IDENTIFY_TIME, &identifyAttrRec ) )
      {
        uint16_t *data = (uint16_t*)(identifyAttrRec.attr.dataPtr);
        if ( (*data) > 0)
        {
          (*data)--;
          KeepIdentifyTimerRunning = TRUE;
        }
        else
        {
          bdbCommissioningModeMsg_t bdbCommissioningModeMsg;

          //Notify the application
          bdbAttributes.bdbCommissioningStatus = BDB_COMMISSIONING_SUCCESS;
          bdbCommissioningModeMsg.bdbCommissioningMode = BDB_COMMISSIONING_FINDING_BINDING;
          bdbCommissioningModeMsg.bdbCommissioningStatus = BDB_COMMISSIONING_SUCCESS;
          bdb_NotifyApp((uint8_t*)&bdbCommissioningModeMsg);
          // Use bdb success main state
          bdbAttributes.bdbCommissioningStatus = BDB_COMMISSIONING_SUCCESS;
          if(pfnIdentifyTimeChangeCB != NULL)
          {
            pfnIdentifyTimeChangeCB(bdb_EpDescriptor->endPoint);
          }
        }
      }
      bdb_CurrEpDescriptorNextInList = bdb_CurrEpDescriptorNextInList->nextDesc;
    }

    //If any endpoint has identify running, keep the timer on
    if(KeepIdentifyTimerRunning)
    {
      OsalPortTimers_startTimer( bdb_TaskID, BDB_IDENTIFY_TIMEOUT, 1000 );
    }
    else
    {
      OsalPortTimers_stopTimer( bdb_TaskID, BDB_IDENTIFY_TIMEOUT );
    }

    // Return unprocessed events
    return (events ^ BDB_IDENTIFY_TIMEOUT);
  }

  if(events & BDB_RESPONDENT_PROCESS_TIMEOUT)
  {
    // Send Simple Descriptor request to a respondent node
    bdb_ProcessRespondentList();

    return (events ^ BDB_RESPONDENT_PROCESS_TIMEOUT);
  }
#endif

  return 0;
}

/*********************************************************************
 * @fn          bdb_processZDOMgs
 *
 * @brief       Process ZDO messages used as part of BDB commissioning methods
 *
 * @param       zdoIncomingMsg_t - ZDO message
 *
 * @return
 */
static void bdb_processZDOMgs(zdoIncomingMsg_t *pMsg)
{
  switch ( pMsg->clusterID )
  {
#if (BDB_FINDING_BINDING_CAPABILITY_ENABLED==1)
    case IEEE_addr_rsp:
       bdb_ProcessIEEEAddrRsp(pMsg);
    break;
    case Simple_Desc_rsp:
      bdb_ProcessSimpleDesc(pMsg);
    break;
#endif

#if (ZG_BUILD_JOINING_TYPE)
    case Node_Desc_rsp:
      bdb_ProcessNodeDescRsp(pMsg);
    break;
#endif

    default:
    break;
  }
}

#if (ZG_BUILD_JOINING_TYPE)
/*********************************************************************
 * @fn      bdb_ProcessNodeDescRsp
 *
 * @brief   Process Node Descriptor response to validate the stack version of the
 *
 * @param   zdoIncomingMsg_t *pMsg
 *
 * @return  none
 */
static void bdb_ProcessNodeDescRsp(zdoIncomingMsg_t *pMsg)
{
  //Avoid processing unintended messages
  if(requestNewTrustCenterLinkKey &&
    (bdbCommissioningProcedureState.bdbCommissioningState == BDB_COMMISSIONING_STATE_TC_LINK_KEY_EXCHANGE))
  {
    if(!APSME_IsDistributedSecurity())
    {
      //Is this from the coordinator?
      if(pMsg->srcAddr.addr.shortAddr == 0x0000)
      {
        ZDO_NodeDescRsp_t NDRsp = {0};
        uint8_t StackComplianceRev;

        //Stop timer to avoid unintended resets
        OsalPortTimers_stopTimer( bdb_TaskID, BDB_PROCESS_TIMEOUT);

        ZDO_ParseNodeDescRsp(pMsg, &NDRsp);

        StackComplianceRev = NDRsp.nodeDesc.ServerMask >> STACK_COMPLIANCE_CURRENT_REV_POS;

#if defined ( APP_TP2_TEST_MODE )
        if( guEnableKeyExchange && StackComplianceRev >= STACK_COMPL_REV_21 )
#else
        if( StackComplianceRev >= STACK_COMPL_REV_21 )
#endif
        {
          bdb_tcLinkKeyExchangeAttempt(TRUE,BDB_REQ_TC_LINK_KEY);
        }
        else
        {
          APSME_TCLinkKeyNVEntry_t TCLKDevEntry;
          uint8_t entryFound;
          uint16_t entryIndex;
          // Find TCLK entry with TC extAddr
          entryIndex = APSME_SearchTCLinkKeyEntry(AIB_apsTrustCenterAddress,&entryFound,&TCLKDevEntry);

          if(entryIndex < gZDSECMGR_TC_DEVICE_MAX)
          {
            if( TCLKDevEntry.keyAttributes != ZG_PROVISIONAL_KEY )
            {
              TCLKDevEntry.keyAttributes = ZG_NON_R21_NWK_JOINED;
            }

            //Save the KeyAttribute for joining device that it has joined non-R21 nwk
            osal_nv_write_ex(ZCD_NV_EX_TCLK_TABLE, entryIndex,
                            sizeof(APSME_TCLinkKeyNVEntry_t),
                            &TCLKDevEntry);

            TCLinkKeyRAMEntry[entryIndex].entryUsed = TRUE;
          }

          bdb_setNodeJoinLinkKeyType(BDB_DEFAULT_GLOBAL_TRUST_CENTER_LINK_KEY);
          bdb_reportCommissioningState(BDB_COMMISSIONING_STATE_TC_LINK_KEY_EXCHANGE, TRUE);
        }
      }
    }
  }
}
#endif

/*********************************************************************
 * @fn          bdb_touchlinkSendFNReset
 *
 * @brief       Starts the Factory New Procedure for Initiator
 *
 * @param       isOnANetwork - TRUE if the devices is not FN, FALSE otherwise
 *
 * @return      none
 */
void bdb_touchlinkSendFNReset( void )
{
#ifdef BDB_TL_INITIATOR
  touchLinkInitiator_ResetToFNProcedure( );
#endif
}


/*********************************************************************
 * @fn          bdb_setNodeIsOnANetwork
 *
 * @brief       Sets and saves in Nv bdbNodeIsOnANetwork attribute
 *
 * @param       isOnANetwork - TRUE if the devices is not FN, FALSE otherwise
 *
 * @return      none
 */
void bdb_setNodeIsOnANetwork(bool isOnANetwork)
{
  if((bdbAttributes.bdbNodeIsOnANetwork != isOnANetwork) || (!bdb_initialization))
  {
    //We lose our network
    if(!isOnANetwork)
    {
      bdbAttributes.bdbCommissioningMode = 0;
    }

    bdbAttributes.bdbNodeIsOnANetwork = isOnANetwork;

    osal_nv_write(ZCD_NV_BDBNODEISONANETWORK, sizeof(bdbAttributes.bdbNodeIsOnANetwork), &bdbAttributes.bdbNodeIsOnANetwork);
  }
}

/*********************************************************************
 * @fn          bdb_setCommissioningGroupID
 *
 * @brief       Sets the commissioning groupd ID
 *
 * @param       groupID
 *
 * @return      none
 */
void bdb_setCommissioningGroupID(uint16_t groupID)
{
  bdbAttributes.bdbCommissioningGroupID = groupID;
}

/*********************************************************************
 * @fn      bdb_CreateRespondentList
 *
 * @brief   Create respondent list for finding and binding if empty
 *
 * @param   pHead - pointer to a pointer of the list head
 *
 * @return  none
 */
void bdb_CreateRespondentList( bdbFindingBindingRespondent_t **pHead )
{

  // Create the list if empty
  if ( *pHead == NULL )
  {
    *pHead = ( bdbFindingBindingRespondent_t* )OsalPort_malloc( sizeof( bdbFindingBindingRespondent_t ) );

    if ( *pHead != NULL )
    {
      (*pHead)->pNext = NULL;
    }
  }
  return;
}

/*********************************************************************
 * @fn      bdb_AddRespondentNode
 *
 * @brief   Add node to respondent list for finding and binding
 *
 * @param   pHead - pointer to a pointer of the list head
 *
 * @return  pointer to new node
 */
bdbFindingBindingRespondent_t* bdb_AddRespondentNode( bdbFindingBindingRespondent_t **pHead, zclIdentifyQueryRsp_t *pCmd )
{
  bdbFindingBindingRespondent_t **pCurr;
  bdbFindingBindingRespondent_t *temp;

    // Create respondent list if empty
  if ( *pHead == NULL )
  {
    bdb_CreateRespondentList( pHead );
    return *pHead;
  }
  else
  {
    // if pCmd is equal to NULL, don't look for duplicates
    if( pCmd != NULL )
    {
      //Find if any duplicate in the list
      temp = *pHead;

      while(temp != NULL)
      {
        if((temp->data.endPoint == pCmd->srcAddr->endPoint) && (temp->data.panId == pCmd->srcAddr->panId))
        {
          //Duplicate
          if(temp->data.addr.shortAddr == pCmd->srcAddr->addr.shortAddr)
          {
            return NULL;
          }
        }
        temp = temp->pNext;
      }
    }

    pCurr = &((*pHead)->pNext);

    while ( *pCurr != NULL )
    {
      pCurr = &((*pCurr)->pNext);
    }

    *pCurr = ( bdbFindingBindingRespondent_t* )OsalPort_malloc( sizeof( bdbFindingBindingRespondent_t ) );

    if(*pCurr == NULL)
    {
      //No memory
      return NULL;
    }

    (*pCurr)->pNext = NULL;
  }

  return *pCurr;
}

/*********************************************************************
 * @fn      bdb_zclRespondentListClean
 *
 * @brief   This function free reserved memory for respondent list
 *
 * @param   pHead - begin of the respondent list
 *
 * @return  status
 */
void bdb_zclRespondentListClean( bdbFindingBindingRespondent_t **pHead )
{
  bdbFindingBindingRespondent_t **pCurr;
  bdbFindingBindingRespondent_t **pNext;

  if ( *pHead == NULL )
  {
    return;
  }

  pCurr = pHead;

  while( *pCurr != NULL )
  {
    pNext = &((*pCurr)->pNext);
    OsalPort_free( *pCurr );
    *pCurr = ( bdbFindingBindingRespondent_t* )NULL;
    pCurr = pNext;
  }
  *pHead = NULL;
}

 /*********************************************************************
 * PRIVATE FUNCTIONS
 *********************************************************************/

/*********************************************************************
 * @fn      bdb_ProcessOSALMsg
 *
 * @brief   Process the incoming task message.
 *
 * @param   msgPtr - message to process
 *
 * @return  none
 */
void bdb_ProcessOSALMsg( bdbInMsg_t *msgPtr )
{

  switch(msgPtr->hdr.event)
  {
#if (ZG_BUILD_JOINING_TYPE)
    case BDB_COMMISSIONING_STATE_JOINING:
      if(ZG_DEVICE_JOINING_TYPE)
      {
        switch(msgPtr->buf[0])
        {
          case BDB_JOIN_EVENT_NWK_DISCOVERY:
            if(msgPtr->hdr.status == BDB_MSG_EVENT_SUCCESS)
            {
              bdb_filterNwkDisc();

              if(pfnFilterNwkDesc == NULL)
              {
                //If the callback to filter the network descriptor is registered, then the joining is now
                //handled by application call after filter the nwk descriptors, see zstackmsg_CmdIDs_BDB_FILTER_NWK_DESC_COMPLETE_REQ
                //Otherwise, try to join all the valid networks
                bdb_tryNwkAssoc();
              }
            }
            else
            {
              bdb_nwkDiscoveryAttempt(FALSE);
            }
          break;

          case BDB_JOIN_EVENT_ASSOCIATION:
            if(msgPtr->hdr.status == BDB_MSG_EVENT_SUCCESS)
            {
              bdbCommissioningProcedureState.bdbJoinState = BDB_JOIN_STATE_WAITING_NWK_KEY;
              //Nwk key timeout get right timing
              OsalPortTimers_startTimer(bdb_TaskID,BDB_PROCESS_TIMEOUT, BDB_DEFAULT_DEVICE_UNAUTH_TIMEOUT);
            }
            else
            {
              if ( (NLME_GetShortAddr() != INVALID_NODE_ADDR) ||
                   (_NIB.nwkDevAddress != INVALID_NODE_ADDR) )
              {
                uint16_t addr = INVALID_NODE_ADDR;
                // Invalidate nwk addr so end device does not use in its data reqs.
                _NIB.nwkDevAddress = INVALID_NODE_ADDR;
                ZMacSetReq( ZMacShortAddress, (uint8_t *)&addr );
              }

              //Clear the neighbor Table and network discovery tables.
              nwkNeighborInitTable();
              NLME_NwkDiscTerm();
              _NIB.nwkState = NWK_INIT;

              bdb_tryNwkAssoc();
            }
          break;
        }
      }
    break;

    case BDB_COMMISSIONING_STATE_TC_LINK_KEY_EXCHANGE:
      if(ZG_DEVICE_JOINING_TYPE)
      {
        if(msgPtr->hdr.status != BDB_MSG_EVENT_SUCCESS)
        {
          bdbAttributes.bdbTCLinkKeyExchangeAttempts++;
          if(bdbAttributes.bdbTCLinkKeyExchangeAttempts > bdbAttributes.bdbTCLinkKeyExchangeAttemptsMax)
          {
            //TCLK process fail due to many attempts fails
            bdb_reportCommissioningState(BDB_COMMISSIONING_STATE_TC_LINK_KEY_EXCHANGE, FALSE);
            return;
          }
        }
        switch(bdbCommissioningProcedureState.bdbTCExchangeState)
        {
          case BDB_REQ_TC_STACK_VERSION:
            bdb_requestTCStackVersion();
          break;
          case BDB_REQ_TC_LINK_KEY:
            bdb_requestTCLinkKey();
          break;
          case BDB_REQ_VERIFY_TC_LINK_KEY:
            bdb_requestVerifyTCLinkKey();
          break;
        }
      }
   break;
#endif
   }
}


/*********************************************************************
 * @fn      bdb_processTimeout
 *
 * @brief   Handles timeout of the bdb process
 *
 * @param   msgPtr - message to process
 *
 * @return  none
 */
void bdb_processTimeout(void)
{
#if (ZG_BUILD_JOINING_TYPE)
  if(ZG_DEVICE_JOINING_TYPE)
  {
    switch(bdbCommissioningProcedureState.bdbCommissioningState)
    {
      case BDB_COMMISSIONING_STATE_TC_LINK_KEY_EXCHANGE:

        bdb_tcLinkKeyExchangeAttempt(FALSE,bdbCommissioningProcedureState.bdbTCExchangeState);
      break;
      case BDB_COMMISSIONING_STATE_JOINING:
        if(bdbCommissioningProcedureState.bdbJoinState == BDB_JOIN_STATE_WAITING_NWK_KEY)
        {
          //If nwk key fails, then try association again
          bdbCommissioningProcedureState.bdbJoinState = BDB_JOIN_STATE_ASSOC;
          bdb_nwkAssocAttemt(FALSE);
        }
      break;
    }
  }
#endif

}


/*********************************************************************
 * @fn      bdb_SendMsg
 *
 * @brief   Send messages to bdb processing with the expected format
 *
 * @param   msgPtr - message to process
 *
 * @return  none
 */
void bdb_SendMsg(uint8_t taskID, uint8_t toCommissioningState,uint8_t status, uint8_t len, uint8_t *buf)
{
  bdbInMsg_t *msgPtr = NULL;

  if ( (len > 0) && (buf != NULL) )
  {
    uint8_t tmpLength;
    tmpLength = len;
    tmpLength += sizeof(OsalPort_EventHdr);

    msgPtr = (bdbInMsg_t *)OsalPort_msgAllocate( tmpLength );

    if ( msgPtr )
    {
      OsalPort_memcpy( msgPtr->buf, buf, len );

      msgPtr->hdr.event = toCommissioningState;
      msgPtr->hdr.status = status;
      OsalPort_msgSend( taskID, (uint8_t *)msgPtr );
    }
  }
}


/*********************************************************************
 * @fn      bdb_RegisterCommissioningStatusCB
 *
 * @brief   Register a callback in which the status of the procedures done in
 *          BDB commissioning process will be reported
 *
 * @param   bdbGCB_CommissioningStatus - application callback
 *
 * @return  none
 */
void bdb_RegisterCommissioningStatusCB(bdbGCB_CommissioningStatus_t bdbGCB_CommissioningStatus)
{
  pfnCommissioningStatusCB = bdbGCB_CommissioningStatus;
}

/*********************************************************************
 * @fn      bdb_ClearNetworkParams
 *
 * @brief   Restore nwk parameters to invalid if the device is not on a network
 *
 * @param   void
 *
 * @return  void
 */
void bdb_ClearNetworkParams(void)
{
#if (BDB_TOUCHLINK_CAPABILITY_ENABLED == TRUE)
  uint8_t cntChannel = 0;
  uint32_t tmpChanMask = 0x00000001;

  if ( bdbAttributes.bdbNodeIsOnANetwork == FALSE )
  {
    //Clear the event
    _NIB.nwkPanId = INVALID_NODE_ADDR;

    // Place the channel on the lowest one of the primary channel set
    while( !(BDB_DEFAULT_PRIMARY_CHANNEL_SET & tmpChanMask) )
    {
        cntChannel++;
        tmpChanMask <<= 1;
    }

    _NIB.nwkLogicalChannel = cntChannel;
    _NIB.nwkDevAddress = INVALID_NODE_ADDR;
    touchLink_SetMacNwkParams( _NIB.nwkDevAddress, _NIB.nwkPanId, _NIB.nwkLogicalChannel );
  }
#endif
}

#if (ZG_BUILD_JOINING_TYPE)
/*********************************************************************
 * @fn      bdb_RegisterCBKETCLinkKeyExchangeCB
 *
 * @brief   Register a callback in which the TC link key exchange procedure will
 *          be performed by application.
 *          Upon fail or success bdb must be notified, see bdb_CBKETCLinkKeyExchangeAttempt
 *
 * @param   bdbGCB_TCLinkKeyExchangeMethod - application callback
 *
 * @return  none
 */
void bdb_RegisterCBKETCLinkKeyExchangeCB(bdbGCB_CBKETCLinkKeyExchange_t bdbGCB_CBKETCLinkKeyExchange)
{
  if(bdbGCB_CBKETCLinkKeyExchange)
  {
    pfnCBKETCLinkKeyExchange = bdbGCB_CBKETCLinkKeyExchange;
    bdbAttributes.bdbTCLinkKeyExchangeMethod = BDB_TC_LINK_KEY_EXCHANGE_CBKE;
  }
  else
  {
    pfnCBKETCLinkKeyExchange = NULL;
    bdbAttributes.bdbTCLinkKeyExchangeMethod = BDB_TC_LINK_KEY_EXCHANGE_APS_KEY;
  }
}

/*********************************************************************
 * @fn      bdb_RegisterForFilterNwkDescCB
 *
 * @brief   Register a callback in which the application gets the list of network
 *          descriptors got from active scan.
 *          Use bdb_nwkDescFree to release the network descriptors that are not
 *          of interest and leave those which are to be attempted.
 *
 * @param   bdbGCB_FilterNwkDesc - application callback
 *
 * @return  none
 */
void bdb_RegisterForFilterNwkDescCB(bdbGCB_FilterNwkDesc_t bdbGCB_FilterNwkDesc)
{
  if(bdbGCB_FilterNwkDesc)
  {
    pfnFilterNwkDesc = bdbGCB_FilterNwkDesc;
  }
}


/*********************************************************************
 * @fn          bdb_CBKETCLinkKeyExchangeAttempt
 *
 * @brief       Tell BDB module the result of the TC link key exchange, to try
 *              the default process or to keep going with the joining process.
 *
 * @param       didSuccess - TRUE if the process was succes, False otherwise
 *
 * @return      none
 */
void bdb_CBKETCLinkKeyExchangeAttempt(bool didSuccess)
{
  if(didSuccess)
  {
    bdb_setNodeJoinLinkKeyType(BDB_DEFAULT_GLOBAL_TRUST_CENTER_LINK_KEY);
    bdb_reportCommissioningState(BDB_COMMISSIONING_STATE_TC_LINK_KEY_EXCHANGE, TRUE);
  }
  else
  {
    bdbAttributes.bdbTCLinkKeyExchangeMethod = BDB_TC_LINK_KEY_EXCHANGE_APS_KEY;
    //We are going back one state to try it again
    bdbCommissioningProcedureState.bdbTCExchangeState -= BDB_TC_EXCHANGE_NEXT_STATE;
    bdb_tcLinkKeyExchangeAttempt(TRUE,BDB_REQ_TC_STACK_VERSION);
  }

}
#endif

#if !defined (DISABLE_GREENPOWER_BASIC_PROXY) && (ZG_BUILD_RTR_TYPE)

/*********************************************************************
 * @fn      gp_ChangeChannelReq
 *
 * @brief   Callback function to notify the BDB about a GP commissioning
 * request that will change the current channel for at most
 * gpBirectionalCommissioningChangeChannelTimeout ms
 *
 * @param   channel - Channel in which the commissioning will take place
 *
 * @return  TRUE to allow change channel, FALSE to do not allow
 */
static uint8_t gp_ChangeChannelReq(void)
{
  uint8_t allowChangeChannel = TRUE;

  //Do not allow changes of channel if any process is in place
  if(bdbAttributes.bdbCommissioningMode)
  {
    allowChangeChannel = FALSE;
  }

  //Check application state to decide if allow change channel or not

  return allowChangeChannel;
}


/*********************************************************************
 * @fn          gp_CBInit
 *
 * @brief       Register the callbacks for GP endpoint
 *
 * @param       none
 *
 * @return      none
 */
void gp_CBInit(void)
{
  GP_DataCnfGCB = GP_DataCnf;
  GP_stopCommissioningModeGCB = gp_stopCommissioningMode;
#if ((!defined USE_ICALL) && (!defined OSAL_PORT2TIRTOS))
  GP_expireDuplicateFilteringGCB = gp_expireDuplicateFiltering;
  GP_DataIndGCB = gp_dataIndProxy;
  GP_SecReqGCB = GP_SecReq;
  GP_CheckAnnouncedDeviceGCB = gp_CheckAnnouncedDevice;
  GP_aliasConflictAnnce = &aliasConflictAnnce;
#endif
}

#endif

/*********************************************************************
*********************************************************************/

/******************************************************************************
 * @fn          bdb_GenerateInstallCodeCRC
 *
 * @brief       Creates a CRC for the install code passed.
 *
 * @param       installCode - install code from which CRC will be generated
 *
 * @return      CRC
 */
uint16_t bdb_GenerateInstallCodeCRC(uint8_t *installCode)
{
  uint16_t CRC;

  bdb_calculateCCITT_CRC(installCode, INSTALL_CODE_LEN, &CRC);

  return CRC;
}

/******************************************************************************
 * @fn          bdb_calculateCCITT_CRC
 *
 * @brief       Creates a CRC for the install code passed.
 *
 * @param       Mb - install code from which CRC will be generated
 * @param       msglen - install code length
 * @param       crc -
 *
 * @return      none
 */
void bdb_calculateCCITT_CRC (uint8_t *Mb, uint32_t msglen, uint16_t *crc)
{
  uint16_t crcinit_direct;
  uint16_t crcinit_nondirect;
  bdb_crcInit(crc, &crcinit_direct, &crcinit_nondirect);
  *crc = bdb_crcBitByBitFast(Mb, msglen, crcinit_direct, crcinit_nondirect);
}


/******************************************************************************
 * @fn          bdb_crcInit
 *
 * @brief       Initialize CRC calculation
 *
 * @param       crc -
 * @param       crcinit_direct -
 * @param       crcinit_nondirect -
 *
 * @return      none
 */
void bdb_crcInit(uint16_t *crc, uint16_t *crcinit_direct, uint16_t *crcinit_nondirect)
{

  uint16_t i;
  uint16_t bit;

  *crcinit_direct = CRC_INIT;
  *crc = CRC_INIT;
  for (i=0; i<CRC_ORDER; i++)
  {
    bit = *crc & 1;
    if (bit) *crc^= CRC_POLYNOM;
    *crc >>= 1;
    if (bit) *crc|= CRC_HIGHBIT;
  }
  *crcinit_nondirect = *crc;

}


/******************************************************************************
 * @fn          bdb_crcReflect
 *
 * @brief
 *
 * @param       crc -
 * @param       bitnum -
 *
 * @return      none
 */
uint16_t bdb_crcReflect (uint16_t crc, uint16_t bitnum)
{

  // reflects the lower 'bitnum' bits of 'crc'

  uint16_t i, j=1, crcout=0;

  for (i=(uint16_t)1<<(bitnum-1); i; i>>=1) {
    if (crc & i) crcout|=j;
    j<<= 1;
  }
  return (crcout);
}


/******************************************************************************
 * @fn          bdb_crcBitByBitFast
 *
 * @brief
 *
 * @param       p -
 * @param       len -
 * @param       crcinit_direct -
 * @param       crcinit_nondirect -
 *
 * @return      crc
 */
uint16_t bdb_crcBitByBitFast(uint8_t * p, uint32_t len, uint16_t crcinit_direct, uint16_t crcinit_nondirect)
{
  // fast bit by bit algorithm without augmented zero bytes.
  // does not use lookup table, suited for polynom orders between 1...32.

  uint16_t i, j, c, bit;
  uint16_t crc = crcinit_direct;

  for (i=0; i<len; i++) {

    c = (uint16_t)*p++;
    c = bdb_crcReflect(c, 8);

    for (j=0x80; j; j>>=1) {

      bit = crc & CRC_HIGHBIT;
      crc<<= 1;
      if (c & j) bit^= CRC_HIGHBIT;
      if (bit) crc^= CRC_POLYNOM;
    }
  }

  crc=bdb_crcReflect(crc, CRC_ORDER);
  crc^= CRC_XOR;

  return(crc);
}

/******************************************************************************
 * @fn          bdb_resetStateMachine
 *
 * @brief       reinitialize the BDB state machine without power cycling
 *              the device
 *
 * @return      crc
 */
void bdb_resetStateMachine(void)
{
    bdb_initialization = 0;

#if (BDB_FINDING_BINDING_CAPABILITY_ENABLED==1)
    bdb_StopInitiatorFindingBinding();
#endif

    OsalPortTimers_stopTimer(bdb_TaskID, BDB_CHANGE_COMMISSIONING_STATE);
    OsalPort_clearEvent(bdb_TaskID, BDB_CHANGE_COMMISSIONING_STATE);
    bdbAttributes.bdbCommissioningMode = 0;
    bdb_StartCommissioning(0);
}

