/**************************************************************************************************
  Filename:       gp_sink.c
  Revised:        $Date: 2016-02-25 11:51:49 -0700 (Thu, 25 Feb 2016) $
  Revision:       $Revision: - $

  Description:    This file contains the implementation of the cGP stub.


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

#include "zglobals.h"
#if !defined (DISABLE_GREENPOWER_BASIC_PROXY) && (ZG_BUILD_RTR_TYPE)
#include "zcomdef.h"
#include "zcl_general.h"
#include "af.h"
#include "gp_common.h"
#include "gp_sink.h"
#include "zcl_port.h"
#include "zcl.h"
#include "dgp_stub.h"
#include "mac_api.h"
#include "zd_sec_mgr.h"
#include "bdb.h"

#if (defined (USE_ICALL) || defined (OSAL_PORT2TIRTOS))
#include "zstackapi.h"
#include "util_timer.h"
#endif

 /*********************************************************************
 * MACROS
 */


 /*********************************************************************
 * CONSTANTS
 */

/*********************************************************************
 * TYPEDEFS
 */

 /*********************************************************************
 * GLOBAL VARIABLES
 */
uint8_t  zgGP_SinkCommissioningMode = FALSE;  // Global flag that states if in commissioning mode or in operational mode.


 /*********************************************************************
 * EXTERNAL VARIABLES
 */
extern uint8_t gpAppEntity;

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

 /*********************************************************************
 * LOCAL FUNCTIONS
 */
static void gp_ZclPairingConfigurationParse( zclGpPairingConfig_t* pCmd, gpPairingConfigCmd_t* payload );
static void gp_ZclSinkTableReqParse( zclGpTableRequest_t* pCmd, gpTableReqCmd_t* payload );
static ZStatus_t zclGp_GpChannelRequestProcess( zclGpCommissioningNotification_t *pCmd );
static ZStatus_t zclGp_GpCommissioningNotificationProcess( zclGpCommissioningNotification_t *pCmd );
static ZStatus_t zclGp_GpSuccessNotificationProcess( zclGpCommissioningNotification_t *pCmd );
static uint8_t* gp_processCommissioning(gpdID_t *pGPDId, uint8_t *pAsdu, gpdCommissioningCmd_t *pCommissioningCmdPayload, uint8_t* pLen);

/*********************************************************************
 * LOCAL VARIABLES
 */

GpSink_AppCallbacks_t *GpSink_AppCallbacks = NULL;
uint8_t* pNewSinkEntry = NULL;


/*********************************************************************
 * PUBLIC FUNCTIONS
 *********************************************************************/

/*********************************************************************
 * @fn      zclGp_RegisterCBForGPDCommand
 *
 * @brief   Register an applications command callbacks for GPDF
 *
 * @param   callbacks - pointer to the callback record.
 *
 * @return  ZInvalidParameter if no valid pointer is providedm otherwise ZSuccess
 */

ZStatus_t zclGp_RegisterCBForGPDCommand(GpSink_AppCallbacks_t *pGpSink_AppCallbacks)
{
  ZStatus_t status = ZSuccess;

  if(pGpSink_AppCallbacks != NULL)
  {
    GpSink_AppCallbacks = pGpSink_AppCallbacks;
  }
  else
  {
    status = ZInvalidParameter;
  }
  return status;
}


/*********************************************************************
 * @fn      zclGp_GpNotificationCommandCB
 *
 * @brief   Callback from the ZCL GreenPower Cluster Library when
 *          it received an Gp Notification Command for this application.
 *
 * @param   pCmd - command payload
 *
 * @return  none
 */
void zclGp_GpNotificationCommandCB( zclGpNotification_t *pCmd )
{
  if(GpSink_AppCallbacks != NULL)
  {
    switch(pCmd->gpCmdId)
    {
#ifdef ZCL_IDENTIFY
    case GP_COMMAND_IDENTIFY_IDENTIFY:
      if( GpSink_AppCallbacks->pfnGpdfIdentifyCmd )
      {
        GpSink_AppCallbacks->pfnGpdfIdentifyCmd(pCmd);
      }
    break;
#endif
#ifdef ZCL_SCENES
    case GP_COMMAND_RECALL_SCENE0:
    case GP_COMMAND_RECALL_SCENE1:
    case GP_COMMAND_RECALL_SCENE2:
    case GP_COMMAND_RECALL_SCENE3:
    case GP_COMMAND_RECALL_SCENE4:
    case GP_COMMAND_RECALL_SCENE5:
    case GP_COMMAND_RECALL_SCENE6:
    case GP_COMMAND_RECALL_SCENE7:
      if( GpSink_AppCallbacks->pfnGpdfRecallSceneCmd )
      {
        GpSink_AppCallbacks->pfnGpdfRecallSceneCmd(pCmd);
      }
    break;
    case GP_COMMAND_STORE_SCENE0:
    case GP_COMMAND_STORE_SCENE1:
    case GP_COMMAND_STORE_SCENE2:
    case GP_COMMAND_STORE_SCENE3:
    case GP_COMMAND_STORE_SCENE4:
    case GP_COMMAND_STORE_SCENE5:
    case GP_COMMAND_STORE_SCENE6:
    case GP_COMMAND_STORE_SCENE7:
      if( GpSink_AppCallbacks->pfnGpdfStoreSceneCmd )
      {
        GpSink_AppCallbacks->pfnGpdfStoreSceneCmd(pCmd);
      }
    break;
#endif
#ifdef ZCL_ON_OFF
    case GP_COMMAND_OFF:
      if( GpSink_AppCallbacks->pfnGpdfOffCmd )
      {
        GpSink_AppCallbacks->pfnGpdfOffCmd(pCmd);
      }
    break;
    case GP_COMMAND_ON:
      if( GpSink_AppCallbacks->pfnGpdfOnCmd )
      {
        GpSink_AppCallbacks->pfnGpdfOnCmd(pCmd);
      }
    break;
    case GP_COMMAND_TOGGLE:
      if( GpSink_AppCallbacks->pfnGpdfToggleCmd )
      {
        GpSink_AppCallbacks->pfnGpdfToggleCmd(pCmd);
      }
    break;
#endif
#ifdef ZCL_LEVEL_CTRL
    case GP_COMMAND_LEVEL_CONTROL_MOVE_UP:
      if( GpSink_AppCallbacks->pfnGpdfMoveUpCmd )
      {
        GpSink_AppCallbacks->pfnGpdfMoveUpCmd(pCmd);
      }
    break;
    case GP_COMMAND_LEVEL_CONTROL_MOVE_DOWN:
      if( GpSink_AppCallbacks->pfnGpdfMoveDownCmd )
      {
        GpSink_AppCallbacks->pfnGpdfMoveDownCmd(pCmd);
      }
    break;
    case GP_COMMAND_LEVEL_CONTROL_STEP_UP:
      if( GpSink_AppCallbacks->pfnGpdfStepUpCmd )
      {
        GpSink_AppCallbacks->pfnGpdfStepUpCmd(pCmd);
      }
    break;
    case GP_COMMAND_LEVEL_CONTROL_STEP_DOWN:
      if( GpSink_AppCallbacks->pfnGpdfStepDownCmd )
      {
        GpSink_AppCallbacks->pfnGpdfStepDownCmd(pCmd);
      }
    break;
    case GP_COMMAND_LEVEL_CONTROL_STOP:
      if( GpSink_AppCallbacks->pfnGpdfLevelControlStopCmd )
      {
        GpSink_AppCallbacks->pfnGpdfLevelControlStopCmd(pCmd);
      }
    break;
    case GP_COMMAND_LEVEL_CONTROL_MOVE_UP_WITH_ON_OFF:
      if( GpSink_AppCallbacks->pfnGpdfMoveUpWithOnOffCmd )
      {
        GpSink_AppCallbacks->pfnGpdfMoveUpWithOnOffCmd(pCmd);
      }
    break;
    case GP_COMMAND_LEVEL_CONTROL_MOVE_DOWN_UP_WITH_ON_OFF:
      if( GpSink_AppCallbacks->pfnGpdfMoveDownWithOnOffCmd )
      {
        GpSink_AppCallbacks->pfnGpdfMoveDownWithOnOffCmd(pCmd);
      }
    break;
    case GP_COMMAND_LEVEL_CONTROL_STEP_UP_UP_WITH_ON_OFF:
      if( GpSink_AppCallbacks->pfnGpdfStepUpWithOnOffCmd )
      {
        GpSink_AppCallbacks->pfnGpdfStepUpWithOnOffCmd(pCmd);
      }
    break;
    case GP_COMMAND_LEVEL_CONTROL_STEP_DOWN_UP_WITH_ON_OFF:
      if( GpSink_AppCallbacks->pfnGpdfStepDownWithOnOffCmd )
      {
        GpSink_AppCallbacks->pfnGpdfStepDownWithOnOffCmd(pCmd);
      }
    break;
#endif
    case GP_COMMAND_MOVE_HUE_STOP:
      if( GpSink_AppCallbacks->pfnGpdfMoveHueStopCmd )
      {
        GpSink_AppCallbacks->pfnGpdfMoveHueStopCmd(pCmd);
      }
    break;
    case GP_COMMAND_MOVE_HUE_UP:
      if( GpSink_AppCallbacks->pfnGpdfMoveHueUpCmd )
      {
        GpSink_AppCallbacks->pfnGpdfMoveHueUpCmd(pCmd);
      }
    break;
    case GP_COMMAND_MOVE_HUE_DOWN:
      if( GpSink_AppCallbacks->pfnGpdfMoveHueDownCmd )
      {
        GpSink_AppCallbacks->pfnGpdfMoveHueDownCmd(pCmd);
      }
    break;
    case GP_COMMAND_STEP_HUE_UP:
      if( GpSink_AppCallbacks->pfnGpdfStepHueUpCmd )
      {
        GpSink_AppCallbacks->pfnGpdfStepHueUpCmd(pCmd);
      }
    break;
    case GP_COMMAND_STEP_HUE_DOWN:
      if( GpSink_AppCallbacks->pfnGpdfStepHueDownCmd )
      {
        GpSink_AppCallbacks->pfnGpdfStepHueDownCmd(pCmd);
      }
    break;
    case GP_COMMAND_MOVE_SATURATION_STOP:
      if( GpSink_AppCallbacks->pfnGpdfMoveSaturationStopCmd )
      {
        GpSink_AppCallbacks->pfnGpdfMoveSaturationStopCmd(pCmd);
      }
    break;
    case GP_COMMAND_MOVE_SATURATION_UP:
      if( GpSink_AppCallbacks->pfnGpdfMoveSaturationUpCmd )
      {
        GpSink_AppCallbacks->pfnGpdfMoveSaturationUpCmd(pCmd);
      }
    break;
    case GP_COMMAND_MOVE_SATURATION_DOWN:
      if( GpSink_AppCallbacks->pfnGpdfMoveSaturationDownCmd )
      {
        GpSink_AppCallbacks->pfnGpdfMoveSaturationDownCmd(pCmd);
      }
    break;
    case GP_COMMAND_STEP_SATURATION_UP:
      if( GpSink_AppCallbacks->pfnGpdfStepSaturationUpCmd )
      {
        GpSink_AppCallbacks->pfnGpdfStepSaturationUpCmd(pCmd);
      }
    break;
    case GP_COMMAND_STEP_SATURATION_DOWN:
      if( GpSink_AppCallbacks->pfnGpdfStepSaturationDownCmd )
      {
        GpSink_AppCallbacks->pfnGpdfStepSaturationDownCmd(pCmd);
      }
    break;
    case GP_COMMAND_MOVE_COLOR:
      if( GpSink_AppCallbacks->pfnGpdfMoveColorCmd )
      {
        GpSink_AppCallbacks->pfnGpdfMoveColorCmd(pCmd);
      }
    break;
    case GP_COMMAND_STEP_COLOR:
      if( GpSink_AppCallbacks->pfnGpdfStepColorCmd )
      {
        GpSink_AppCallbacks->pfnGpdfStepColorCmd(pCmd);
      }
    break;
#ifdef ZCL_DOORLOCK
    case GP_COMMAND_LOCK_DOOR:
      if( GpSink_AppCallbacks->pfnGpdfLockDoorCmd )
      {
        GpSink_AppCallbacks->pfnGpdfLockDoorCmd(pCmd);
      }
    break;
    case GP_COMMAND_UNLOCK_DOOR:
      if( GpSink_AppCallbacks->pfnGpdfUnlockDoorCmd )
      {
        GpSink_AppCallbacks->pfnGpdfUnlockDoorCmd(pCmd);
      }
    break;
#endif
    case GP_COMMAND_ATTR_REPORTING:
      if( GpSink_AppCallbacks->pfnGpdfAttributeReportingCmd )
      {
        GpSink_AppCallbacks->pfnGpdfAttributeReportingCmd(pCmd);
      }
    break;
    case GP_COMMAND_MFR_SPECIFIC_ATTR_REPORTING:
      if( GpSink_AppCallbacks->pfnGpdfMfrSpecificReportingCmd )
      {
        GpSink_AppCallbacks->pfnGpdfMfrSpecificReportingCmd(pCmd);
      }
    break;
    case GP_COMMAND_MULTICLUSTER_REPORTING:
      if( GpSink_AppCallbacks->pfnGpdfMultiClusterReportingCmd )
      {
        GpSink_AppCallbacks->pfnGpdfMultiClusterReportingCmd(pCmd);
      }
    break;
    case GP_COMMAND_MULTICLUSTER_MFR_SPECIFIC_REPORTING:
      if( GpSink_AppCallbacks->pfnGpdfMfrSpecificMultiReportingCmd )
      {
        GpSink_AppCallbacks->pfnGpdfMfrSpecificMultiReportingCmd(pCmd);
      }
    break;
    case GP_COMMAND_REQUEST_ATTR:
      if( GpSink_AppCallbacks->pfnGpdfRequestAttributesCmd )
      {
        GpSink_AppCallbacks->pfnGpdfRequestAttributesCmd(pCmd);
      }
    break;
    case GP_COMMAND_READ_ATTR_RSP:
      if( GpSink_AppCallbacks->pfnGpdfReadAttributeRspCmd )
      {
        GpSink_AppCallbacks->pfnGpdfReadAttributeRspCmd(pCmd);
      }
    break;
    case GP_COMMAND_ZCL_TUNNELING:
      if( GpSink_AppCallbacks->pfnGpdfzclTunnelingCmd )
      {
        GpSink_AppCallbacks->pfnGpdfzclTunnelingCmd(pCmd);
      }
    break;
    default:
      //What to do
    break;
    }
  }
}

/*********************************************************************
 * @fn      zclGp_DataIndParse
 *

 * @brief   Parse the Gp Data indication to Gp Notification command
 *
 * @param   pInd - Pointer to the incoming data
 *
 * @return  ZStatus_t
 */
ZStatus_t zclGp_DataIndParse( gp_DataInd_t *pInd, gpNotificationCmd_t *pGpNotification )
{
  uint8_t currEntry[PROXY_TBL_LEN];
  uint8_t  ntfOpt[2] = {0x00, 0x00};
  uint8_t i;
  int8_t RSSI;
  uint8_t LQI;
  ZStatus_t status;

  for (i = 0; i < GPP_MAX_PROXY_TABLE_ENTRIES ; i++)
  {
    status = gp_getProxyTableByIndex(i, currEntry);

    if(status == NV_OPER_FAILED)
    {
      return status;
    }

    // if the entry is empty
    if (status == NV_INVALID_DATA)
    {
      continue;
    }

    if(GP_TBL_COMP_APPLICATION_ID( pInd->appID, currEntry[PROXY_TBL_OPT]))
    {
      if((pInd->appID == GP_OPT_APP_ID_GPD) &&
         zcl_memcmp( &pInd->SrcId, &currEntry[PROXY_TBL_GPD_ID + 4], sizeof (uint32_t)))
      {
        // Entry found
        pGpNotification->gpdId = pInd->SrcId;
        ntfOpt[0] = GP_OPT_APP_ID_GPD;
        status = SUCCESS;
        break;
      }
      else if((pInd->appID == GP_OPT_APP_ID_IEEE ) &&
               zcl_memcmp( &pInd->srcAddr, &currEntry[PROXY_TBL_GPD_ID], Z_EXTADDR_LEN))
      {
        // Entry found
        zcl_memcpy(pGpNotification->gpdIEEE, &(pInd->srcAddr.addr.extAddr), Z_EXTADDR_LEN);
        pGpNotification->ep = pInd->EndPoint;
        ntfOpt[0] = GP_OPT_APP_ID_IEEE;
        status = SUCCESS;
        break;
      }
    }
    else
    {
      status = INVALIDPARAMETER;
      continue;
    }
  }

  if ( status == SUCCESS )
  {
      // Set the options bit field
      GP_NTF_SET_ALSO_UNICAST((uint8_t*)&ntfOpt[0], currEntry[PROXY_TBL_OPT]);
      GP_NTF_SET_ALSO_DGROUP((uint8_t*)&ntfOpt[0], currEntry[PROXY_TBL_OPT]);
      GP_NTF_SET_ALSO_CGROUP((uint8_t*)&ntfOpt[0], currEntry[PROXY_TBL_OPT]);
      GP_NTF_SET_SEC_LEVEL((uint8_t*)&ntfOpt[0], currEntry[PROXY_TBL_SEC_OPT]);
      GP_NTF_SET_SEC_KEY_TYPE((uint8_t*)&ntfOpt[1], currEntry[PROXY_TBL_SEC_OPT]);
      pInd->RxAfterTx ?GP_SET_RX_AFTER_TX((uint8_t*)&ntfOpt[1]) : GP_CLR_RX_AFTER_TX((uint8_t*)&ntfOpt[1]);

      // Mandatory to set this sub fields to 0b1 by the Green Power Basic Proxy Spec in A.3.3.4.1
      PROXY_TBL_CLR_TX_QUEUE_FULL((uint8_t*)&ntfOpt[1]);
      PROXY_TBL_CLR_BIDIRECTIONAL_CAP((uint8_t*)&ntfOpt[1]);
      PROXY_TBL_SET_PROXY_INFO((uint8_t*)&ntfOpt[1]);

      pGpNotification->options |= ((ntfOpt[0]) & 0x00FF);
      pGpNotification->options |= (((ntfOpt[1]) <<  8) & 0xFF00);

      pGpNotification->gpdSecCounter = pInd->GPDSecFrameCounter;
      pGpNotification->cmdId = pInd->GPDCmmdID;

      if ( pInd->GPDasduLength > 0 )
      {
          pGpNotification->payloadLen = pInd->GPDasduLength;
          pGpNotification->cmdPayload = zcl_mem_alloc(pGpNotification->payloadLen);
          if(pGpNotification->cmdPayload != NULL)
          {
            zcl_memcpy(pGpNotification->cmdPayload, pInd->GPDasdu, pInd->GPDasduLength);
          }
      }

      pGpNotification->gppShortAddr = _NIB.nwkDevAddress;

      RSSI = pInd->Rssi;
      ( RSSI > 8 ) ?RSSI = 8 : ( RSSI < -109 ) ?RSSI = -109 : 0;
      RSSI += 110;
      RSSI /= 2;

      ( pInd->LinkQuality == 0 ) ?LQI = 0 : ( pInd->LinkQuality > 0 ) ?LQI = 2 : 0;

      pGpNotification->gppGpdLink = RSSI;
      pGpNotification->gppGpdLink |= ( LQI << 6 );
  }

  return status;
}

/*********************************************************************
 * @fn      zclGp_GpCommissioningNotificationCommandCB
 *
 * @brief   Callback from the ZCL GreenPower Cluster Library when
 *          it received an Gp Commissioning Notification Command for this application.
 *
 * @param   pCmd - command payload
 *
 * @return  none
 */
void zclGp_GpCommissioningNotificationCommandCB( zclGpCommissioningNotification_t *pCmd )
{
    switch(pCmd->gpCmdId)
    {
      case GP_CHANNEL_REQ_COMMAND_ID:
        zclGp_GpChannelRequestProcess(pCmd);
      break;
      case GP_COMMISSIONING_COMMAND_ID:
          zclGp_GpCommissioningNotificationProcess(pCmd);
      break;
      case GP_SUCCESS_COMMAND_ID:
          zclGp_GpSuccessNotificationProcess(pCmd);
      break;
      default:
        // Auto-commissioning frame notification
      break;
    }
}

/*********************************************************************
 * @fn      zclGp_GpPairingConfigCommandCB
 *
 * @brief   Callback from the ZCL GreenPower Cluster Library when
 *          it received an Gp Pairing Configuration Command for this application.
 *
 * @param   pCmd - command payload
 *
 * @return  none
 */
void zclGp_GpPairingConfigurationCommandCB( zclGpPairingConfig_t *pCmd )
{
  gpPairingConfigCmd_t pairingConfigPayload;
  uint16_t sinkTableIndex;

  zcl_memset( &pairingConfigPayload, 0x00, sizeof(gpPairingConfigCmd_t) );
  gp_ZclPairingConfigurationParse( pCmd, &pairingConfigPayload );
  sinkTableIndex = gp_PairingConfigUpdateSinkTbl(&pairingConfigPayload);

  if( GP_PAIRING_CONFIG_ACTION_SEND_PAIRING(pCmd->actions))
  {
      uint8_t pEntry[SINK_TBL_ENTRY_LEN];

      if ( gp_getSinkTableByIndex( sinkTableIndex, pEntry ) == SUCCESS )
      {
        uint32_t secFrameCnt;
        secFrameCnt = BUILD_UINT32(pEntry[SINK_TBL_SEC_FRAME], pEntry[SINK_TBL_SEC_FRAME+1],
                                   pEntry[SINK_TBL_SEC_FRAME+2], pEntry[SINK_TBL_SEC_FRAME+3]);
        zclGp_SendGpPairing(pEntry, pCmd->actions, secFrameCnt, zcl_InSeqNum);
      }
  }


 // if(gp_commissioningOptions & GP_COMM_OPT_EXIT_ON_PAIRING_SUCCESS_MASK)
 // {
 //   gp_stopCommissioningMode();
 // }
}

/*********************************************************************
 * @fn      zclGp_GpSinkTableReqCB
 *
 * @brief   Callback from the ZCL GreenPower Cluster Library when
 *          it received a Gp proxy table req.
 *
 * @param   pCmd - command payload
 *
 * @return  none
 */
void zclGp_GpSinkTableReqCB( zclGpTableRequest_t *pCmd )
{
  uint16_t i;
  uint8_t  entryLen = 0;
  uint8_t  maxEntryLen = 0;
  uint8_t* buf = NULL;
  uint8_t  currEntry[SINK_TBL_ENTRY_LEN];
  zclGpTableResponse_t sinkTblRsp = {0};
  gpTableReqCmd_t sinkTblReqPayload = {0};

  sinkTblRsp.entry = NULL;
  gp_ZclSinkTableReqParse( pCmd, &sinkTblReqPayload );

  // To get total entries for "Total number of non-empty Proxy Table entries" field
  for(i = 0; i < GPS_MAX_SINK_TABLE_ENTRIES; i++)
  {
    if(gp_getSinkTableByIndex(i, currEntry) == SUCCESS)
    {
      sinkTblRsp.tableEntriesTotal += 1;
    }
  }

  // for Request Table Entries by Index
  if(GP_PAIRING_OPT_IS_REQ_TYPE_INDEX( sinkTblReqPayload.options ))
  {
    if((sinkTblRsp.tableEntriesTotal == 0x00) ||
       (sinkTblRsp.tableEntriesTotal < sinkTblReqPayload.index))
    {
      sinkTblRsp.status = SUCCESS;
      sinkTblRsp.startIndex = sinkTblReqPayload.index;
      sinkTblRsp.entriesCount = 0x00;
      // Send response and exit
      zclGp_SendGpSinkTableResponse(pCmd->srcAddr, &sinkTblRsp, zcl_InSeqNum);
      return;
    }
    for(i = sinkTblReqPayload.index; i <= GPS_MAX_SINK_TABLE_ENTRIES; i++)
    {
      maxEntryLen = entryLen;    // Save the last iteration Lenght

      // Get the length of the payload
      if(pt_ZclReadGetSinkEntry(i, NULL, &entryLen) != SUCCESS)
      {
        break;
      }

      if(entryLen >= 75)   // max payload for entries
      {
        entryLen = maxEntryLen;   // The last valid lenght is the MAX for this packet
        break;
      }
    }

    if(entryLen)
    {
      buf = zcl_mem_alloc( entryLen );
      if ( buf != NULL )
      {
        // Load the buffer - serially
        uint8_t *pBuf = buf;

        entryLen = 0;
        sinkTblRsp.entriesCount = 0;
        // JASON -- what the hell does this for loop do?
        for (i = sinkTblReqPayload.index; i <= GPS_MAX_SINK_TABLE_ENTRIES ; i++)
        {
          if(pt_ZclReadGetSinkEntry(i, pBuf, &entryLen) != SUCCESS)
          {
            break;
          }

          sinkTblRsp.entriesCount += 1;
          if(entryLen >= maxEntryLen)   // to see if we reched the MAX calculated payload
          {
            break;
          }
        }
      }
    }
  }

    uint8_t lookForEntry = 0;
    gpdID_t gpdID;
    uint16_t NvProxyTableIndex;
    uint8_t  ProxyTableEntryTemp[PROXY_TBL_LEN];

    if(GP_OPT_IS_APPLICATION_ID_IEEE( sinkTblReqPayload.options ))
    {
      gpdID.appID = GP_OPT_APP_ID_GPD;
      gpdID.id.srcID = sinkTblReqPayload.gpdId;
      lookForEntry = 1;
      sinkTblRsp.startIndex = 0xFF;
    }
    else if(GP_OPT_IS_APPLICATION_ID_GPD( sinkTblReqPayload.options ))
    {
      gpdID.appID = GP_OPT_APP_ID_IEEE;
      zcl_memcpy( gpdID.id.gpdExtAddr, sinkTblReqPayload.gpdIEEE, Z_EXTADDR_LEN );
      lookForEntry = 1;
      sinkTblRsp.startIndex = 0xFF;
    }

    if(lookForEntry)
    {
      if(gp_getSinkTableByGpId(&gpdID, ProxyTableEntryTemp, &NvProxyTableIndex) == ZSuccess)
      {
        maxEntryLen = entryLen;    // Save the last iteration Lenght

        // Get the length of the payload
        if(pt_ZclReadGetSinkEntry(NvProxyTableIndex, NULL, &entryLen) != ZSuccess)
        {
          entryLen = 0;
        }

        if(entryLen >= 75)   // max payload for entries
        {
          entryLen = maxEntryLen;   // The last valid lenght is the MAX for this packet
        }
      }
      else
      {
        sinkTblRsp.status = ZCL_STATUS_NOT_FOUND;
      }

      if(entryLen)
      {
        buf = zcl_mem_alloc(entryLen);
        if ( buf != NULL )
        {
          // Load the buffer - serially
          uint8_t *pBuf = buf;

          entryLen = 0;
          sinkTblRsp.entriesCount = 0;

          if(pt_ZclReadGetSinkEntry(NvProxyTableIndex, pBuf, &entryLen) == SUCCESS)
          {
            sinkTblRsp.entriesCount = 1;
          }
        }
      }
    }

  // Send response and exit
  sinkTblRsp.startIndex = sinkTblReqPayload.index;
  sinkTblRsp.entry = buf;
  zclGp_SendGpSinkTableResponse(pCmd->srcAddr, &sinkTblRsp, zcl_InSeqNum);
  zcl_mem_free( buf );
}

/*********************************************************************
* @fn          gp_SinkTblInit
*
* @brief       General function to init the NV items for sink table
*
* @param
*
* @return
*/
uint8_t gp_SinkTblInit(uint8_t resetTable)
{
  uint8_t i;
  uint8_t status;
  uint8_t emptyEntry[SINK_TBL_ENTRY_LEN];

  gp_ResetSinkTblEntry(emptyEntry);

  for(i = 0; i < GPS_MAX_SINK_TABLE_ENTRIES ; i++)
  {
    status = zclport_initializeNVItem(ZCL_PORT_SINK_TABLE_NV_ID, i,
                                      SINK_TBL_ENTRY_LEN, emptyEntry );

    if((status != SUCCESS) && (status != NV_ITEM_UNINIT))
    {
      return status;
    }
    else if((status == SUCCESS) && (resetTable == TRUE))
    {
      status = zclport_writeNV(ZCL_PORT_SINK_TABLE_NV_ID, i,
                               SINK_TBL_ENTRY_LEN, emptyEntry );
    }
  }
  return status;
}

/*********************************************************************
* @fn          gp_getSinkTableByGpId
*
* @brief       General function to get sink table entry by gpdID (GP Src ID or Extended Adddress)
*
* @param       gpdID  - address to look for in the table
*              pEntry  - buffer in which the entry of the table will be copied
*
* @return
*/
uint8_t gp_getSinkTableByGpId(gpdID_t *gpdID, uint8_t *pEntry, uint16_t *NvSinkTableIndex)
{
 uint8_t i;
 uint8_t status;
 if((pEntry == NULL) || (gpdID == NULL) || (NvSinkTableIndex == NULL))
 {
   return ZFailure;
 }

 // Initialize index as invalid
 *NvSinkTableIndex = ZCD_NV_INVALID_INDEX;

 for(i = 0; i < GPS_MAX_SINK_TABLE_ENTRIES ; i++)
 {
   status = gp_getSinkTableByIndex(i, pEntry);

   if(status == NV_OPER_FAILED)
   {
     // FAIL
     return ZMemError;
   }

   // if the entry is empty
   if(status == NV_INVALID_DATA)
   {
     if(*NvSinkTableIndex == ZCD_NV_INVALID_INDEX)
     {
       *NvSinkTableIndex = i;
     }
     continue;
   }

   //Check that App ID is the same
   if(GP_TBL_COMP_APPLICATION_ID( gpdID->appID, pEntry[SINK_TBL_OPT]))
   {
     if((gpdID->appID == GP_OPT_APP_ID_GPD) &&
         zcl_memcmp(&gpdID->id.srcID, &pEntry[SINK_TBL_SRC_ID], sizeof(uint32_t)))
     {
       // Entry found
       *NvSinkTableIndex = i;
       return ZSuccess;
     }
     else if((gpdID->appID == GP_OPT_APP_ID_IEEE) &&
              zcl_memcmp(&gpdID->id.gpdExtAddr, &pEntry[SINK_TBL_GPD_ID], Z_EXTADDR_LEN))
     {
       // Entry found
       *NvSinkTableIndex = i;
       return ZSuccess;
     }
   }
   else
   {
     continue;
   }
 }
 return ZInvalidParameter;
}

/*********************************************************************
* @fn          gp_getSinkTableByIndex
*
* @brief       General function to get sink table entry by NV index
*
* @param       nvIndex - NV Id of sink table
*              pEntry  - pointer to SINK_TBL_LEN array
*
* @return
*/
uint8_t gp_getSinkTableByIndex( uint16_t nvIndex, uint8_t *pEntry )
{
 uint8_t status;
 uint16_t emptyEntry = 0xFFFF;

 status = zclport_readNV(ZCL_PORT_SINK_TABLE_NV_ID, nvIndex,
                           0, SINK_TBL_ENTRY_LEN, pEntry);
 if(status != SUCCESS)
 {
   // Return the failure status of NV read procedure
   return status;
 }

 // if the entry is empty
 if(zcl_memcmp(pEntry, &emptyEntry, sizeof(uint16_t)))
 {
   return NV_INVALID_DATA;
 }
 return status;
}

 /*********************************************************************
  * @fn      GP_SinkProcessDataIndicationCommissioningGpdf
  *
  * @brief   Sink process a commissioning GPDF
  *
  * @param   gpdId - Green Power Device id
  *          gp_DataInd - Data indication from Green Power Stub
  *
  * @return  ZStatus_t
  */
ZStatus_t GP_SinkProcessDataIndicationCommissioningGpdf( gpdID_t *gpdId, gp_DataInd_t *gp_DataInd, uint16_t index )
 {
   uint8_t pSinkEntry[SINK_TBL_ENTRY_LEN];
   ZStatus_t status  = ZFailure;

   if(gp_DataInd->GPDCmmdID == GP_COMMISSIONING_COMMAND_ID)
   {
       status = gp_commissioningGpdfSinkTblUpdate(gpdId, gp_DataInd, pSinkEntry, index);
   }
   else if(gp_DataInd->AutoCommissioning == TRUE)
   {
       status = gp_autoCommissioningGpdfSinkTblUpdate(gpdId, gp_DataInd, pSinkEntry, index);
   }
   // Add entry to proxy side of the Sink
   gp_sinkAddProxyEntry(pSinkEntry);

   return ( status );
 }

/*********************************************************************
* @fn          gp_SecurityOperationSink
*
* @brief       Performs Security Operations according to Sink
*
* @param       ind - pointer to gp data indication
* @param       pFrameCounter
* @param       pKeyType
* @param       pkey Key to be used to decript the packet (if applies)
*
* @return      GP_SEC_RSP status
*/
uint8_t gp_SecurityOperationSink( gp_SecReq_t* pInd, uint8_t* pKeyType, void* pKey)
{
  gp_DataInd_t *gpDataInd = NULL;
  uint8_t         currEntry[SINK_TBL_ENTRY_LEN];
  uint8_t         status;
  uint32_t        secFrameCounter = 0;
  uint8_t         securityCheckFail = FALSE;
  uint8_t         endpointCheckFail = FALSE;
  uint16_t        nvSinkTableIndex = 0;
  gpdID_t       gpdID;


  if((pKeyType == NULL) || (pKey == NULL) || (pInd == NULL))
  {
    return GP_SEC_RSP_ERROR;
  }

  *pKeyType = 0;
  zcl_memset(pKey, 0, SEC_KEY_LEN);
  gpdID.appID = pInd->gpdID.appID;
  if(gpdID.appID == GP_APP_ID_GPID)
  {
    gpdID.id.srcID = pInd->gpdID.id.srcID;
  }
  else
  {
    zcl_memcpy(gpdID.id.gpdExtAddr, pInd->gpdID.id.gpdExtAddr, Z_EXTADDR_LEN);
  }

  gpDataInd = gp_DataIndGet(pInd->dGPStubHandle);
  status = gp_getSinkTableByGpId(&gpdID, currEntry, &nvSinkTableIndex);
  // Look for success command
  if(((status == ZInvalidParameter) && (zgGP_SinkCommissioningMode == TRUE)) &&
      (pInd->gp_SecData.GPDFSecLvl >= GP_SECURITY_LVL_4FC_4MIC))
  {
    zstack_gpDecryptDataInd_t req;
    zstack_gpDecryptDataIndRsp_t rsp;
    gp_DataInd_t *pDataIndEncrypted = NULL;
    uint8_t dataIndLen;

    if(pNewSinkEntry == NULL)
    {
      return GP_SEC_RSP_ERROR;
    }

    // Alloc memory for encrypted data indication
    dataIndLen = sizeof(gp_DataInd_t) + gpDataInd->GPDasduLength;
    pDataIndEncrypted = zcl_mem_alloc(dataIndLen);
    if(pDataIndEncrypted == NULL)
    {
      return GP_SEC_RSP_ERROR;
    }

    // Decrypt data indication to analyze the payload before
    // green power security validation
    zcl_memcpy(pDataIndEncrypted, gpDataInd, dataIndLen);
    req.pGpDataInd = pDataIndEncrypted;
    rsp.pGpDataInd = NULL;
    zcl_memcpy(req.key, &pNewSinkEntry[SINK_TBL_GPD_KEY], SEC_KEY_LEN);
    Zstackapi_gptDecryptDataInd( gpAppEntity, &req, &rsp );
    if((rsp.status == SUCCESS) && (rsp.pGpDataInd))
    {
      if(rsp.pGpDataInd->GPDasdu[0] == GP_SUCCESS_COMMAND_ID)
      {
        uint8_t i;

        // Set status as success because temporal
        // entry is in place
        status = ZSuccess;
        zgGP_SinkCommissioningMode = FALSE;
        for (i = 0; i < gGP_TX_QUEUE_MAX_ENTRY; i++)
        {
          if(gpTxQueueList[i].gp_DataReq != NULL)
          {
            OsalPort_msgDeallocate((void*)gpTxQueueList[i].gp_DataReq);
            gpTxQueueList[i].gp_DataReq = NULL;
          }
        }
        zcl_memcpy(currEntry, pNewSinkEntry, SINK_TBL_ENTRY_LEN);
      }
    }
    else
    {
      zcl_mem_free(pDataIndEncrypted);
      return GP_SEC_RSP_ERROR;
     }
   //Copy decrypted data
   zcl_memcpy(gpDataInd, rsp.pGpDataInd, dataIndLen);
   zcl_mem_free(pDataIndEncrypted);
   return GP_SEC_RSP_PASS_UNPROCESSED;
   }

  // Unsecured frames in commissioning mode
  if((((status == ZInvalidParameter) || (status == ZSuccess)) &&
  (zgGP_SinkCommissioningMode == TRUE)) &&
  ((pInd->gp_SecData.GPDFSecLvl == GP_SECURITY_LVL_NO_SEC)))
  {
    gpdCommissioningCmd_t cmd;
    uint8_t gpsSecurityLevel;
    zcl_ReadAttrData(GREEN_POWER_INTERNAL_ENDPOINT, ZCL_CLUSTER_ID_GREEN_POWER,
                     ATTRID_GP_GPS_SECURITY_LEVEL, &gpsSecurityLevel, NULL);

    //Section A.3.9.2.1.2 GPD command used for commissioning
    if(gpDataInd->GPDasdu[0] == GP_COMMISSIONING_COMMAND_ID)
    {
      gp_parseCommisioningCmd((uint8_t*)&gpDataInd->GPDasdu[1], &cmd);
      if(((cmd.options.securityKeyRequest == TRUE) &&
      (gpDataInd->RxAfterTx == FALSE)) ||
      (cmd.options.securityKeyRequest == TRUE) &&
      (cmd.extOptions.securityLevel == GP_SECURITY_LVL_NO_SEC))
      {
        return GP_SEC_RSP_DROP_FRAME;
      }

      if(cmd.extOptions.securityLevel >=
        SEC_PARAMETERS_GET_SEC_OPT_SECURITY_LVL(gpsSecurityLevel))
      {
        if(cmd.extOptions.outgoingCounterPresent == TRUE)
        {
          if((cmd.extOptions.keyType == GP_SECURITY_KEY_TYPE_ZIGBEE_NWK_KEY) ||
             (cmd.extOptions.keyType == GP_SECURITY_KEY_TYPE_GPD_GROUP_KEY)  ||
             (cmd.extOptions.keyType == GP_SECURITY_KEY_TYPE_NWK_KEY_DERIVED_GPD_GROUP))
          {
            return GP_SEC_RSP_PASS_UNPROCESSED;
          }
          else if( ( (cmd.extOptions.keyType == GP_SECURITY_KEY_TYPE_OUT_OF_BOX_GPD_KEY) ||
          (cmd.extOptions.keyType == GP_SECURITY_KEY_TYPE_DERIVED_IND_GPD_KEY)) &&
          (cmd.extOptions.gpdKeyPresent == TRUE))
          {
            return GP_SEC_RSP_PASS_UNPROCESSED;
          }
        }
        else if((cmd.extOptions.keyType == GP_SECURITY_KEY_TYPE_NO_KEY) &&
                (cmd.options.securityKeyRequest == TRUE) &&
                (gpDataInd->RxAfterTx == TRUE) )
        {
          return GP_SEC_RSP_PASS_UNPROCESSED;
        }
      }
    }
    else if((gpDataInd->AutoCommissioning == TRUE) ||
            (SEC_PARAMETERS_GET_SEC_OPT_SECURITY_LVL(gpsSecurityLevel) == GP_SECURITY_LVL_NO_SEC))
    {
      return GP_SEC_RSP_PASS_UNPROCESSED;
    }
    else
    {
      return GP_SEC_RSP_DROP_FRAME;
    }
  }
  //Found and in operational mode
  else if((status == ZSuccess) && (zgGP_SinkCommissioningMode == FALSE))
  {
    //If security level is zero then don't check and pass unprocessed
    if((pInd->gp_SecData.GPDFSecLvl == SEC_PARAMETERS_GET_SEC_OPT_SECURITY_LVL(currEntry[SINK_TBL_SEC_OPT])) &&
       (GP_SECURITY_LVL_NO_SEC == SEC_PARAMETERS_GET_SEC_OPT_SECURITY_LVL(currEntry[SINK_TBL_SEC_OPT])))
    {
      return GP_SEC_RSP_PASS_UNPROCESSED;
    }
    //Check security Section A.3.7.3.3
    //Check framecounter freshness
    zcl_memcpy(&secFrameCounter, &currEntry[SINK_TBL_SEC_FRAME], sizeof(uint32_t));
    if(secFrameCounter >= pInd->gp_SecData.GPDSecFrameCounter)
    {
      securityCheckFail = TRUE;
    }
    //Compare the security level
    else if(pInd->gp_SecData.GPDFSecLvl !=
            SEC_PARAMETERS_GET_SEC_OPT_SECURITY_LVL(currEntry[SINK_TBL_SEC_OPT]))
    {
      securityCheckFail = TRUE;
    }
    //Mapping of security key type (section A.1.4.1.3 Table 12)
    else if((SEC_PARAMETERS_GET_SEC_OPT_SECURITY_KEY_TYP(currEntry[SINK_TBL_SEC_OPT]) <=
             GP_SECURITY_KEY_TYPE_NWK_KEY_DERIVED_GPD_GROUP) &&
            (pInd->gp_SecData.GPDFKeyType == 1))
    {
      securityCheckFail = TRUE;
    }
    else if(((SEC_PARAMETERS_GET_SEC_OPT_SECURITY_KEY_TYP(currEntry[SINK_TBL_SEC_OPT]) == GP_SECURITY_KEY_TYPE_OUT_OF_BOX_GPD_KEY)   ||
             (SEC_PARAMETERS_GET_SEC_OPT_SECURITY_KEY_TYP(currEntry[SINK_TBL_SEC_OPT]) == GP_SECURITY_KEY_TYPE_DERIVED_IND_GPD_KEY)) &&
             (pInd->gp_SecData.GPDFKeyType == 0) )
    {
       securityCheckFail = TRUE;
    }
    else if( (SEC_PARAMETERS_GET_SEC_OPT_SECURITY_KEY_TYP(currEntry[SINK_TBL_SEC_OPT]) == 0x05) ||
    (SEC_PARAMETERS_GET_SEC_OPT_SECURITY_KEY_TYP(currEntry[SINK_TBL_SEC_OPT]) == 0x06) )
    {
      //keytype reserved
      securityCheckFail = TRUE;
    }

    if(securityCheckFail == TRUE)
    {
      if(zgGP_ProxyCommissioningMode == FALSE)
      {
        return GP_SEC_RSP_DROP_FRAME;
      }
      else
      {
        return GP_SEC_RSP_PASS_UNPROCESSED;
      }
    }
    else
    {
      //Security check success
      zcl_memcpy(pKey, &currEntry[SINK_TBL_GPD_KEY], SEC_KEY_LEN);
      *pKeyType = SEC_PARAMETERS_GET_SEC_OPT_SECURITY_KEY_TYP(currEntry[SINK_TBL_SEC_OPT]);

      //compare the endpoint
      if(endpointCheckFail)
      {
        return GP_SEC_RSP_TX_THEN_DROP;
      }
      else
      {
        return GP_SEC_RSP_MATCH;
      }
    }
  }
  //Section A.3.7.3.1.2 in operational mode drop frames if no
  //entry was found
  else if((status == ZInvalidParameter) && (zgGP_SinkCommissioningMode == FALSE))
  {
  // Pass frame to proxy
  return GP_SEC_RSP_NO_ENTRY;
  }
  //error
  else if(status == ZFailure)
  {
    //Not found, or error, drop the frame
    return GP_SEC_RSP_DROP_FRAME;
  }
  return GP_SEC_RSP_DROP_FRAME;
}

/*********************************************************************
 * @fn          gp_dataIndSink
 *
 * @brief       This passes the MCPS data indications received in MAC to the application
 *
 * @param       gp_DataInd
 *
 * @return
 */
void gp_dataIndSink(gp_DataInd_t *gpDataInd)
{
  uint8_t    status;
  uint32_t   timeout;
  gpdID_t  gpdID;
  uint8_t    sinkTableEntry[SINK_TBL_ENTRY_LEN];
  uint16_t   sinkTableEntryIndex = 0;

  //If authentication/decryption fail, then drop the frame and stop processing
  //GP Basic proxy A.3.5.2.3
  if(gpDataInd->status == GP_DATA_IND_STATUS_COUNTER_FAILURE ||
     gpDataInd->status == GP_DATA_IND_STATUS_AUTH_FAILURE)
  {
    return;
  }

  gpdID.appID = gpDataInd->appID;
  if(gpDataInd->appID == GP_OPT_APP_ID_IEEE)
  {
    if(gpDataInd->status == GP_DATA_IND_STATUS_SECURITY_SUCCESS ||
       gpDataInd->status == GP_DATA_IND_STATUS_NO_SECURITY)
    {
      uint8_t invalidIEEE[Z_EXTADDR_LEN];
      uint8_t result;

      zcl_memset(invalidIEEE, 0x00, Z_EXTADDR_LEN);
      result = zcl_memcmp(gpDataInd->srcAddr.addr.extAddr, invalidIEEE, Z_EXTADDR_LEN);

      // If IEEE addr is equal to 0x0000000000000000, silently drop the frame
      if(result == TRUE)
      {
        return;
      }
    }
    zcl_memcpy(gpdID.id.gpdExtAddr, gpDataInd->srcAddr.addr.extAddr, Z_EXTADDR_LEN);
  }
  else if (gpDataInd->appID == GP_OPT_APP_ID_GPD)
  {
    if(gpDataInd->status == GP_DATA_IND_STATUS_SECURITY_SUCCESS ||
       gpDataInd->status == GP_DATA_IND_STATUS_NO_SECURITY)
    {
      // If GPD source id is equal to 0x00000000, silently drop the frame
      if(gpDataInd->SrcId == (uint32_t)0x00000000)
      {
        return;
      }
    }
    gpdID.id.srcID = gpDataInd->SrcId;
  }

  //Find duplicates A.3.6.1.2 Duplicate filtering
  if(gp_DataIndFindDuplicate(gpDataInd->SecReqHandling.dGPStubHandle, gpDataInd->GPDFSecLvl))
  {
    return;
  }

  gpDataInd->SecReqHandling.timeout = gpDuplicateTimeout;
  //Consider the current time elapsed to the next timeout
#if (defined (USE_ICALL) || defined (OSAL_PORT2TIRTOS))
  timeout =  UtilTimer_getTimeout(gpAppExpireDuplicateClkHandle);
#else
  timeout = OsalPortTimers_getTimerTimeout(gp_TaskID, GP_DUPLICATE_FILTERING_TIMEOUT_EVENT);
#endif

  if(timeout)
  {
    gpDataInd->SecReqHandling.timeout += timeout;
  }
  else
  {
#if (defined (USE_ICALL) || defined (OSAL_PORT2TIRTOS))
  UtilTimer_setTimeout(gpAppExpireDuplicateClkHandle, gpDataInd->SecReqHandling.timeout );
  UtilTimer_start(&gpAppExpireDuplicateClk);
#else
  OsalPortTimers_startTimer(gp_TaskID, GP_DUPLICATE_FILTERING_TIMEOUT_EVENT, gp_DataInd->SecReqHandling.timeout);
#endif
  }

  status = gp_getSinkTableByGpId(&gpdID, sinkTableEntry, &sinkTableEntryIndex);
  if((status == ZSuccess) &&
     (zgGP_SinkCommissioningMode == FALSE))
  {
    uint32_t secNumber = 0;
    zclGpNotification_t cmd;

    if((gpDataInd->AutoCommissioning == FALSE) &&
       (gpDataInd->GPDFSecLvl >= GP_SECURITY_LVL_4FC_4MIC))
    {
      secNumber = zcl_build_uint32(&sinkTableEntry[SINK_TBL_SEC_FRAME], sizeof(uint32_t));
      //A.3.6.1.3 Freshness check
      if(secNumber >= gpDataInd->GPDSecFrameCounter)
      {
        return;
      }
      zcl_buffer_uint32(&sinkTableEntry[SINK_TBL_SEC_FRAME], gpDataInd->GPDSecFrameCounter);

      //Update Sec Frame counter to sink table entry
      zclport_writeNV( ZCL_PORT_SINK_TABLE_NV_ID, sinkTableEntryIndex,
                       SINK_TBL_ENTRY_LEN, sinkTableEntry );
      secNumber = gpDataInd->GPDSecFrameCounter;
    }

    zcl_memcpy(&cmd.gpdId, &gpdID, sizeof(gpdID_t));
    cmd.GPDEndpoint = gpDataInd->EndPoint;
    cmd.securityFrameCounter = secNumber;
    cmd.gpCmdId = gpDataInd->GPDCmmdID;
    cmd.DataLen = gpDataInd->GPDasduLength;
    cmd.pData = gpDataInd->GPDasdu;

    zclGp_GpNotificationCommandCB(&cmd);
  }
  else if((status == ZInvalidParameter) &&
          (zgGP_SinkCommissioningMode == FALSE))
  {
    if(gpDataInd->GPDCmmdID == GP_SUCCESS_COMMAND_ID)
    {
      zstack_gpCommissioningSuccess_t msg;
      gpSinkTableOptions_t sinkOptions;
      uint16_t sinkTableEntryIndex = 0;

      if(pNewSinkEntry == NULL)
      {
        return;
      }

      //Look at temporal entry to complete commissioning
      sinkOptions = *(gpSinkTableOptions_t*)&pNewSinkEntry[SINK_TBL_OPT];
      //Compare device ID's
      if(sinkOptions.appId == gpdID.appID)
      {
        uint8_t res = FALSE;

        // If Application Id bitfield is 0b000
        if(gpdID.appID == GP_OPT_APP_ID_GPD)
        {
          res = zcl_memcmp(&pNewSinkEntry[SINK_TBL_SRC_ID], (uint8_t*)&gpdID.id.srcID, sizeof(uint32_t));
        }
        // If Application Id bitfield is 0b010
        else if(gpdID.appID == GP_OPT_APP_ID_IEEE)
        {
          res = zcl_memcmp(&pNewSinkEntry[SINK_TBL_GPD_ID], gpdID.id.gpdExtAddr, Z_EXTADDR_LEN);
        }

        //If a gp success command arrives with other source ID than temporal entry
        //then free temporal entry
        if(res == FALSE)
        {
          zcl_mem_free(pNewSinkEntry);
          return;
        }
      }
      //If a gp success command arrives with other source ID than temporal entry
      //then free temporal entry
      else
      {
        zcl_mem_free(pNewSinkEntry);
        return;
      }

      zcl_memcpy(&msg.alias, &pNewSinkEntry[SINK_TBL_ALIAS], sizeof( uint16_t ));
      zcl_memcpy(&msg.gpdID, &gpdID, sizeof(gpdID_t));
      msg.index = sinkTableEntryIndex;

      //Send success message
      Zstackapi_gpCommissioningSucess(gpAppEntity, &msg);

      zclGp_SendGpPairing(pNewSinkEntry, GP_ACTION_EXTEND, gpDataInd->GPDSecFrameCounter, zcl_InSeqNum);
      gp_sinkAddProxyEntry(pNewSinkEntry);
      status = zclport_writeNV( ZCL_PORT_SINK_TABLE_NV_ID, sinkTableEntryIndex,
                                SINK_TBL_ENTRY_LEN, pNewSinkEntry );

      zcl_mem_free(pNewSinkEntry);
      // To not process this indication in the proxy side
      gpDataInd->status = GP_DATA_IND_STATUS_UNPROCESSED;
      }
    }
    else if((status == ZInvalidParameter) &&
          (zgGP_SinkCommissioningMode == TRUE))
    {
      if(gpDataInd->GPDCmmdID == GP_CHANNEL_REQ_COMMAND_ID)
      {

#if (defined (USE_ICALL) || defined (OSAL_PORT2TIRTOS))
      UtilTimer_setTimeout(gpAppTempMasterTimeoutClkHandle, gpBirectionalCommissioningChangeChannelTimeout);
      UtilTimer_start(&gpAppTempMasterTimeoutClk);
#else
      OsalPortTimers_startTimer(gp_TaskID, GP_CHANNEL_CONFIGURATION_TIMEOUT, gpBirectionalCommissioningChangeChannelTimeout);
#endif
    }
    else if((gpDataInd->GPDCmmdID == GP_COMMISSIONING_COMMAND_ID) ||
            (gpDataInd->AutoCommissioning == TRUE))
    {
      if(gpDataInd->RxAfterTx == TRUE)
      {
        uint8_t payloadLen;
        uint8_t *pGpCommissioningReply = NULL;
        gpdCommissioningCmd_t commissioningCmdPayload;

        // Parse commissioning payload and get length
        pGpCommissioningReply = gp_processCommissioning(&gpdID, gpDataInd->GPDasdu, &commissioningCmdPayload, &payloadLen);
        if ( pGpCommissioningReply != NULL)
        {
          gp_DataReq_t *pDataReq;
          uint8_t gpCommunicationMode;
          gpSinkTableOptions_t sinkOptions;

          pDataReq = (gp_DataReq_t*)OsalPort_msgAllocate(sizeof(gp_DataReq_t) + payloadLen);
          if(pDataReq == NULL)
          {
            //FAIL no memory
            return;
          }

          // Sink Options
          sinkOptions.appId = gpdID.appID;
          zcl_ReadAttrData(GREEN_POWER_INTERNAL_ENDPOINT, ZCL_CLUSTER_ID_GREEN_POWER,
                           ATTRID_GP_GPS_COMMUNICATION_MODE, &gpCommunicationMode, NULL);
          sinkOptions.communicationMode = gpCommunicationMode;
          sinkOptions.sequenceNumberCap = commissioningCmdPayload.options.sequenceNumberCap;
          sinkOptions.rxOnCapability = commissioningCmdPayload.options.rxOnCap;
          sinkOptions.gpdFixed = commissioningCmdPayload.options.fixedLocation;
          sinkOptions.assignedAlias = FALSE;
          if((commissioningCmdPayload.options.extendedOptionsField == TRUE) &&
             (commissioningCmdPayload.extOptions.securityLevel >= GP_SECURITY_LVL_4FC_4MIC))
          {
            sinkOptions.securityUse = TRUE;
          }
          else
          {
            sinkOptions.securityUse = FALSE;
          }


          pNewSinkEntry = zcl_mem_alloc(SINK_TBL_ENTRY_LEN);
          if(pNewSinkEntry == NULL)
          {
            return;
          }
          gp_commissioningSinkEntryParse(&gpdID, gpDataInd->EndPoint, commissioningCmdPayload.deviceId,
                                         pNewSinkEntry, sinkOptions, &commissioningCmdPayload);

          pDataReq->hdr.event = GP_DATA_REQ;
          pDataReq->hdr.status = 0;
          pDataReq->GPEPhandle = gp_GetHandle(DGP_HANDLE_TYPE);
          pDataReq->Action = TRUE;
          zcl_memcpy(&pDataReq->gpdID, &gpdID, sizeof(gpdID_t));
          pDataReq->EndPoint = gpDataInd->EndPoint;
          pDataReq->TxOptions = GP_OPT_USE_TX_QUEUE_MASK;
          pDataReq->GPDCmmdId = GP_COMMISSIONING_REPLY_COMMAND_ID;
          pDataReq->GPDasduLength = payloadLen;
          zcl_memcpy(pDataReq->GPDasdu, pGpCommissioningReply, payloadLen);

          // Send Green Power data request message
          OsalPort_msgSend(gp_TaskID, (uint8_t*)pDataReq);
          zcl_mem_free(pGpCommissioningReply);
        }
      }
      else  // RxAfterTx == FALSE
      {
        zstack_gpAliasNwkAddress_t msg;
        uint8_t status;

        status = GP_SinkProcessDataIndicationCommissioningGpdf(&gpdID, gpDataInd, sinkTableEntryIndex);
        if(status == SUCCESS)
        {
          status = gp_getSinkTableByIndex(sinkTableEntryIndex, sinkTableEntry);
          if(status == SUCCESS)
          {
            zclGp_SendGpPairing(sinkTableEntry, GP_ACTION_EXTEND, gpDataInd->GPDSecFrameCounter, zcl_InSeqNum);

            if(gpdID.appID == GP_OPT_APP_ID_GPD)
            {
              zcl_memcpy(&msg.alias, (uint8_t*)&gpdID.id.srcID, sizeof(uint16_t));
            }
            else if(gpdID.appID == GP_OPT_APP_ID_IEEE)
            {
              zcl_memcpy(&msg.alias, gpdID.id.gpdExtAddr, sizeof(uint16_t));
            }

            //Send success message
            Zstackapi_gpSendDeviceAnnounce(gpAppEntity, &msg);
          }
        }
      }
    }
  }
}

/*********************************************************************
* @fn          gp_parseCommisioningCmd
*
* @brief       Parse Green Power Commissioning command payload
*
* @param       gpdASDU - payload of commissioning command
*
* @return      pointer to gpdCommissioningCmd_t struct
*/
void gp_parseCommisioningCmd(uint8_t *gpdASDU, gpdCommissioningCmd_t *pCmd)
{
  uint8_t *pPayload;

  pPayload = gpdASDU;
  pCmd->deviceId = *pPayload++;
  pCmd->options = *(gpdCommissioningOptions_t*)pPayload++;
  if(pCmd->options.extendedOptionsField == TRUE)
  {
    pCmd->extOptions = *(gpdCommissioningExtOptions_t*)pPayload++;

    if(pCmd->extOptions.gpdKeyPresent == TRUE)
    {
      zcl_memcpy(pCmd->gpdKey, pPayload, SEC_KEY_LEN);
      pPayload += SEC_KEY_LEN;
      if(pCmd->extOptions.keyEncryption == TRUE)
      {
        zcl_memcpy(&pCmd->gpdKeyMic, pPayload, sizeof(uint32_t));
        pPayload += sizeof(uint32_t);
      }
      zcl_memcpy(&pCmd->gpdOutCounter, pPayload, sizeof(uint32_t));
      pPayload += sizeof(uint32_t);
    }
    else if(pCmd->extOptions.outgoingCounterPresent == TRUE)
    {
      zcl_memcpy(&pCmd->gpdOutCounter, pPayload, sizeof(uint32_t));
      pPayload += sizeof(uint32_t);
    }
  }
}

/*********************************************************************
* PRIVATE FUNCTIONS
*********************************************************************/

/*********************************************************************
* @fn          gp_ZclPairingConfigurationParse
*
* @brief       Parse the pairing configuration data message payload
*
* @param       pCmd  - Pointer to received pairing configuration message
*              payload  - Pointer to pairing configuration command
*
* @return
*/
static void gp_ZclPairingConfigurationParse( zclGpPairingConfig_t* pCmd, gpPairingConfigCmd_t* payload )
{
  payload->actions = pCmd->actions;
  payload->options = pCmd->options ;

  // Options bitfield
  // If Application Id bitfield is 0b000
  if(GP_PAIRING_CONFIG_OPT_IS_APPLICATION_ID_GPD(payload->options))
  {
    // Populate GPD ID
    gp_PopulateField((uint8_t*)&payload->gpdId, &pCmd->pData, sizeof(payload->gpdId));
  }
  // If Application Id bitfield is 0b010
  else if(GP_PAIRING_CONFIG_OPT_IS_APPLICATION_ID_IEEE(payload->options))
  {
    // Populate GPD IEEE
    gp_PopulateField((uint8_t*)&payload->gpdIEEE, &pCmd->pData, Z_EXTADDR_LEN);
    // Populate EP
    gp_PopulateField((uint8_t*)&payload->ep, &pCmd->pData, sizeof(payload->ep));
  }

  if(GP_PAIRING_CONFIG_OPT_REMOVE_GPD( payload->actions))
  {
    uint8_t i;
    uint8_t status;
    uint8_t currEntry[PROXY_TBL_LEN];

    // Remove
    for (i = 0; i < GPP_MAX_PROXY_TABLE_ENTRIES ; i++)
    {
      status = gp_getProxyTableByIndex(i, currEntry);

      if(status == NV_OPER_FAILED)
      {
        // FAIL
        return;
      }

      // if the entry is empty
      if (status == NV_INVALID_DATA)
      {
        // Look for the next entry
        continue;
      }

      if((GP_PAIRING_CONFIG_OPT_IS_APPLICATION_ID_GPD(payload->options)) &&
          zcl_memcmp(&currEntry[PROXY_TBL_GPD_ID + 4], &payload->gpdId, sizeof(uint32_t)))
      {
        // Remove this GPD entry
        gp_ResetSinkTblEntry(currEntry);
      }
      else if((GP_PAIRING_CONFIG_OPT_IS_APPLICATION_ID_IEEE(payload->options)) &&
               zcl_memcmp(&currEntry[PROXY_TBL_GPD_ID ], payload->gpdIEEE, Z_EXTADDR_LEN))
      {
        // Remove this GPD entry
        gp_ResetSinkTblEntry(currEntry);
      }
    }
    return;
  }

  // If Extend Sink Table entry is 0b1
  if((GP_PAIRING_CONFIG_OPT_EXTEND_ENTRY( payload->actions)) ||
      GP_PAIRING_CONFIG_OPT_REPLACE_ENTRY(payload->actions))
  {
        // Populate Device ID
    gp_PopulateField((uint8_t*)&payload->deviceId, &pCmd->pData, sizeof(payload->deviceId));
    gp_PopulateField((uint8_t*)&payload->forwardingRadius, &pCmd->pData, sizeof(payload->forwardingRadius));

    if(GP_PAIRING_CONFIG_IS_GRPCAST_GROUP(payload->options))
    {
        // Get Group entries from pairing configuration and alloc memory
        payload->groupEntriesCount = *pCmd->pData++;
        payload->pGroupList = OsalPort_msgAllocate(payload->groupEntriesCount * SINK_TBL_GRP_ADDR_LEN);

        zcl_memcpy(payload->pGroupList, pCmd->pData, (payload->groupEntriesCount * SINK_TBL_GRP_ADDR_LEN));
    }

    if(GP_PAIRING_CONFIG_ALIAS(payload->options))
    {
      gp_PopulateField((uint8_t*)&payload->gpdAssignedAlias, &pCmd->pData, sizeof(payload->gpdAssignedAlias));
    }

    if ( GP_PAIRING_CONFIG_SECURITY_USE(payload->options))
    {
      gp_PopulateField((uint8_t*)&payload->securityOptions, &pCmd->pData, sizeof(payload->securityOptions));
      gp_PopulateField((uint8_t*)&payload->gpdSecurityFrameCounter, &pCmd->pData, sizeof(payload->gpdSecurityFrameCounter));
      gp_PopulateField((uint8_t*)&payload->gpdSecurityKey, &pCmd->pData, SEC_KEY_LEN);
    }
    else
    {
      if(GP_GET_GPD_SEQ_CAP_BIT(payload->options))
      {
        // Populate GPD security frame counter
        gp_PopulateField((uint8_t*)&payload->gpdSecurityFrameCounter, &pCmd->pData, sizeof(payload->gpdSecurityFrameCounter));
      }
    }

    gp_PopulateField((uint8_t*)&payload->numOfPairedEndpoints, &pCmd->pData, sizeof(payload->numOfPairedEndpoints));

    if((payload->numOfPairedEndpoints > 0x00) && (payload->numOfPairedEndpoints < 0xFE))
    {
      gp_PopulateField((uint8_t*)&payload->pPairedEndpoints, &pCmd->pData, payload->numOfPairedEndpoints);
    }
    else
    {
      payload->pPairedEndpoints = NULL;
    }

    if(GP_PAIRING_CONFIG_APP_INFO(payload->options))
    {
      // Populate GPD App Info
      gp_PopulateField((uint8_t*)&payload->appInfo, &pCmd->pData, sizeof(payload->appInfo));

      if(APP_INFO_GET_MANUFACTURER_ID_PRESENT(payload->appInfo))
      {
        gp_PopulateField((uint8_t*)&payload->manufacturerId, &pCmd->pData, sizeof(payload->manufacturerId));
      }
      else if(APP_INFO_GET_MODE_ID_PRESENT(payload->appInfo))
      {
        gp_PopulateField((uint8_t*)&payload->modeId, &pCmd->pData, sizeof(payload->modeId));
      }
      else if(APP_INFO_GET_GPD_COMMANDS_PRESENT(payload->appInfo))
      {
        gp_PopulateField((uint8_t*)&payload->numberOfGpdCmd, &pCmd->pData, sizeof(payload->numberOfGpdCmd));
        gp_PopulateField((uint8_t*)&payload->pGpdCmdList, &pCmd->pData, payload->numberOfGpdCmd);
      }
      else if(APP_INFO_GET_CLUSTER_LIST_PRESENT(payload->appInfo))
      {
        uint8_t nClusters;
        uint8_t clusterCount;

        nClusters = *pCmd->pData;
        clusterCount = APP_INFO_NUMBER_OF_SERVER_CLUSTERS(nClusters) + APP_INFO_NUMBER_OF_CLIENT_CLUSTERS(nClusters) + 1;
        gp_PopulateField((uint8_t*)&payload->pClusterList, &pCmd->pData, clusterCount);
      }
    }
  }
}

/*********************************************************************
* @fn          gp_ZclSinkTableReqParse
*
* @brief       Parse the sink table request data message payload
*
* @param       pCmd  - Pointer to received Sink table request message
*              payload  - Pointer to Sinl table request command
*
* @return
*/
static void gp_ZclSinkTableReqParse( zclGpTableRequest_t* pCmd, gpTableReqCmd_t* payload )
{
 payload->options =  pCmd->options;

 // If Request type bitfield is 0b00
 if(GP_OPT_IS_APPLICATION_ID_GPD(payload->options))
 {
   // If Application Id bitfield is 0b000
   if(GP_PAIRING_OPT_IS_APPLICATION_ID_GPD(payload->options))
   {
     // Populate GPD ID
     gp_PopulateField((uint8_t*)&payload->gpdId, &pCmd->pData, sizeof(payload->gpdId));

     // Populate GPD IEEE Invalid
     zcl_memset (&payload->gpdIEEE, 0xFF, Z_EXTADDR_LEN);
     // Populate EP Invalid
     payload->ep = 0xFF;
   }
   // If Application Id bitfield is 0b010
   else if( GP_OPT_IS_APPLICATION_ID_IEEE(payload->options))
   {
     // Populate GPD ID Invalid
     payload->gpdId = 0xFFFFFFFF;

     // Populate GPD IEEE
     gp_PopulateField((uint8_t*)&payload->gpdIEEE, &pCmd->pData, Z_EXTADDR_LEN);
     // Populate EP
     gp_PopulateField((uint8_t*)&payload->ep, &pCmd->pData, sizeof(payload->ep));
   }
   // Populate Index Invalid
   payload->index = 0xFF;
 }
 // If Request type bitfield is 0b01
 else if(GP_PAIRING_OPT_IS_REQ_TYPE_INDEX( payload->options))
 {
   // Populate GPD ID Invalid
   payload->gpdId = 0xFFFFFFFF;

   // Populate GPD IEEE Invalid
   zcl_memset( &payload->gpdIEEE, 0xFF, Z_EXTADDR_LEN );

   // Populate EP Invalid
   payload->ep = 0xFF;

   // Populate index
   gp_PopulateField((uint8_t*)&payload->index, &pCmd->pData, sizeof(payload->index));
 }
}

/*********************************************************************
 * @fn      zclGp_GpChannelRequestProcess
 *
 * @brief   Process a commissioning notification generated from a
 *          channel request GPDF
 *
 * @param   pCmd - command payload
 *
 * @return  none
 */
static ZStatus_t zclGp_GpChannelRequestProcess( zclGpCommissioningNotification_t *pCmd )
{
  ZStatus_t status = ZFailure;
  afAddrType_t dstAddr;
  gpResponseCmd_t cmd;

  cmd.options = pCmd->gpdId.appID;
  cmd.tempMasterShortAddr = pCmd->GPPAddress;

  if(GP_PAIRING_OPT_IS_APPLICATION_ID_GPD( pCmd->gpdId.appID))
  {
     cmd.gpdSrcId = pCmd->gpdId.id.srcID;
  }
  else if(GP_PAIRING_OPT_IS_APPLICATION_ID_IEEE(pCmd->gpdId.appID))
  {
    zcl_memcpy(cmd.gpdIEEE, pCmd->gpdId.id.gpdExtAddr, Z_EXTADDR_LEN);
    cmd.ep = pCmd->GPDEndpoint;
  }

  cmd.cmdId = GP_CHANNEL_CCONFIG_COMMAND_ID;
  cmd.tempMasterChannel = pCmd->pData[0] & 0x0F;
  cmd.payloadLen = sizeof(gpdChannelConfiguration_t);
  cmd.cmdPayload = zcl_mem_alloc( cmd.payloadLen );

  if (cmd.cmdPayload)
  {
    dstAddr.addr.shortAddr = pCmd->GPPAddress;
    dstAddr.addrMode = afAddr16Bit;
    dstAddr.endPoint = GREEN_POWER_INTERNAL_ENDPOINT;

    (*(gpdChannelConfiguration_t*)cmd.cmdPayload).operationalChannel =_NIB.nwkLogicalChannel - 11;
    (*(gpdChannelConfiguration_t*)cmd.cmdPayload).basic = TRUE;
    (*(gpdChannelConfiguration_t*)cmd.cmdPayload).reserved = 0;

    zclGp_SendGpResponseCommand(&dstAddr, &cmd);
    zcl_mem_free(cmd.cmdPayload);
  }
  else
    status = ZMemError;

  return ( status );
}

/*********************************************************************
 * @fn      zclGp_GpCommissioningNotificationProcess
 *
 * @brief   Process a commissioning notification generated from a
 *          commissioning GPDF
 *
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclGp_GpCommissioningNotificationProcess( zclGpCommissioningNotification_t *pCmd )
{
  gpdCommissioningCmd_t commissioningCmdPayload;
  ZStatus_t status;
  uint8_t pEntry[SINK_TBL_ENTRY_LEN];
  uint16_t index;

  status = gp_getSinkTableByGpId(&pCmd->gpdId, pEntry, &index);
  // If the GPD is not found but the table is not full
  if((status == ZApsTableFull) ||
     (status == ZFailure))
  {
    return status;
  }

  //Entry found for GPD
  if(status == ZSuccess)
  {
    uint32_t frameCounter;

    gp_parseCommisioningCmd(pCmd->pData, &commissioningCmdPayload);
    frameCounter = zcl_build_uint32(&pEntry[SINK_TBL_SEC_FRAME], FRAME_COUNTER_LEN);
    if((commissioningCmdPayload.extOptions.outgoingCounterPresent == TRUE) &&
       (commissioningCmdPayload.gpdOutCounter > frameCounter))
    {
      zcl_memcpy(&pEntry[SINK_TBL_SEC_FRAME], &commissioningCmdPayload.gpdOutCounter, FRAME_COUNTER_LEN);
      zclport_writeNV( ZCL_PORT_SINK_TABLE_NV_ID, index,
                       SINK_TBL_ENTRY_LEN,
                       pEntry );
    }
  }
  // No matches and an empty entry was found
  else if((status == ZInvalidParameter) &&
          (pCmd->options.rxAfterTx == TRUE))
  {
    gpResponseCmd_t gpResponseCmd;
    uint8_t len;

    pNewSinkEntry = zcl_mem_alloc(SINK_TBL_ENTRY_LEN);
    if(pNewSinkEntry == NULL)
    {
      return ZFailure;
    }

    // Get Commissioning reply
    gpResponseCmd.cmdPayload = gp_processCommissioning(&pCmd->gpdId, pCmd->pData, &commissioningCmdPayload, &len);
    if(gpResponseCmd.cmdPayload != NULL)
    {
      gpSinkTableOptions_t sinkOptions;
      afAddrType_t dstAddr;
      uint8_t gpCommunicationMode;

      // Sink Options
      sinkOptions.appId = pCmd->gpdId.appID;
      zcl_ReadAttrData(GREEN_POWER_INTERNAL_ENDPOINT, ZCL_CLUSTER_ID_GREEN_POWER,
                       ATTRID_GP_GPS_COMMUNICATION_MODE, &gpCommunicationMode, NULL);
      sinkOptions.communicationMode = gpCommunicationMode;
      sinkOptions.sequenceNumberCap = commissioningCmdPayload.options.sequenceNumberCap;
      sinkOptions.rxOnCapability = commissioningCmdPayload.options.rxOnCap;
      sinkOptions.gpdFixed = commissioningCmdPayload.options.fixedLocation;
      sinkOptions.assignedAlias = FALSE;

      if((commissioningCmdPayload.options.extendedOptionsField == TRUE) &&
         (commissioningCmdPayload.extOptions.securityLevel >= GP_SECURITY_LVL_4FC_4MIC))
      {
        sinkOptions.securityUse = TRUE;
      }
      else
      {
        sinkOptions.securityUse = FALSE;
      }


      gp_commissioningSinkEntryParse(&pCmd->gpdId, pCmd->GPDEndpoint, commissioningCmdPayload.deviceId,
                                     pEntry, sinkOptions, &commissioningCmdPayload);

      //Save the new entry temporarily
      zcl_memcpy(pNewSinkEntry, pEntry, SINK_TBL_ENTRY_LEN);
      gpResponseCmd.options = pCmd->gpdId.appID;
      gpResponseCmd.tempMasterShortAddr = pCmd->GPPAddress;
      if(pCmd->gpdId.appID == GP_OPT_APP_ID_GPD)
      {
        gpResponseCmd.gpdSrcId = pCmd->gpdId.id.srcID;
      }
      else if( pCmd->gpdId.appID == GP_OPT_APP_ID_IEEE )
      {
        zcl_memcpy(gpResponseCmd.gpdIEEE, pCmd->gpdId.id.gpdExtAddr, Z_EXTADDR_LEN);
                   gpResponseCmd.ep = pCmd->GPDEndpoint;
      }

      gpResponseCmd.cmdId = GP_COMMISSIONING_REPLY_COMMAND_ID;
      gpResponseCmd.tempMasterChannel = pCmd->pData[0] & 0x0F;
      gpResponseCmd.payloadLen = len;

      //Send GP Response command
      dstAddr.addr.shortAddr = pCmd->GPPAddress;
      dstAddr.addrMode = afAddr16Bit;
      dstAddr.endPoint = GREEN_POWER_INTERNAL_ENDPOINT;
      zclGp_SendGpResponseCommand(&dstAddr, &gpResponseCmd);
      zcl_mem_free( gpResponseCmd.cmdPayload );
    }
    else  //No memory
    {
      status = ZFailure;
    }
  }
  return status;
}

/*********************************************************************
 * @fn      zclGp_GpSuccessNotificationProcess
 *
 * @brief   Process a success notification generated from a
 *          success GPDF
 *
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclGp_GpSuccessNotificationProcess( zclGpCommissioningNotification_t *pCmd )
{
  gpSinkTableOptions_t sinkOptions;
  ZStatus_t status;
  uint8_t pEntry[PROXY_TBL_LEN];
  uint16_t index;

  if(pNewSinkEntry == NULL)
  {
     return ZFailure;
  }

  status = gp_getSinkTableByGpId(&pCmd->gpdId, pEntry, &index);
  // This GPD alredy exist or there is an error
  if((status == ZApsTableFull) || (status == ZFailure) ||
     (status == ZSuccess))
  {
    zcl_mem_free(pNewSinkEntry);
    return status;
  }

  //Look at temporal entry to complete commissioning
  sinkOptions = *(gpSinkTableOptions_t*)&pNewSinkEntry[SINK_TBL_OPT];

  //Compare device ID's
  if(sinkOptions.appId == pCmd->options.appId)
  {
    uint8_t res = FALSE;
    // If Application Id bitfield is 0b000
    if(pCmd->options.appId == GP_OPT_APP_ID_GPD)
    {
      res = zcl_memcmp(&pNewSinkEntry[SINK_TBL_SRC_ID], (uint8_t*)&pCmd->gpdId.id.srcID, sizeof(uint32_t));
    }
    // If Application Id bitfield is 0b010
    else if(pCmd->options.appId == GP_OPT_APP_ID_IEEE)
    {
      res = zcl_memcmp(&pNewSinkEntry[SINK_TBL_GPD_ID], pCmd->gpdId.id.gpdExtAddr, Z_EXTADDR_LEN);
    }

    //If a gp success command arrives with other source ID than temporal entry
    //then free temporal entry
    if(res == FALSE)
    {
      zcl_mem_free(pNewSinkEntry);
      return ZInvalidParameter;
    }
  }
  //If a gp success command arrives with other source ID than temporal entry
  //then free temporal entry
  else
  {
    zcl_mem_free(pNewSinkEntry);
    return ZInvalidParameter;
  }

  if(pCmd->options.securityProcessingFailed == FALSE)
  {
    zstack_gpCommissioningSuccess_t msg;

    zcl_memcpy(&msg.alias, &pNewSinkEntry[SINK_TBL_ALIAS], sizeof( uint16_t ));
    zcl_memcpy(&msg.gpdID, &pCmd->gpdId, sizeof(gpdID_t));
    msg.index = index;

    //Send success message
    Zstackapi_gpCommissioningSucess(gpAppEntity, &msg);
    zclGp_SendGpPairing(pNewSinkEntry, GP_ACTION_EXTEND, pCmd->securityFrameCounter, zcl_InSeqNum);
    gp_sinkAddProxyEntry(pNewSinkEntry);
    status = zclport_writeNV(ZCL_PORT_SINK_TABLE_NV_ID, index,
                             SINK_TBL_ENTRY_LEN, pNewSinkEntry);

     zcl_mem_free(pNewSinkEntry);
  }
  return ZSuccess;
}

/*********************************************************************
 * @fn      gp_getCommissioningReply
 *
 * @brief   Parse commissioning command payload and get commissioning reply
 *          buffer
 *
 * @param   pAsdu - pointer to incoming green power commissioning command payload
 *          pCommissioningCmdPayload - Pointer to commissioning payload struct
 *
 * @return  uint8_t - length of commissioning reply
 */
static uint8_t* gp_processCommissioning(gpdID_t *pGPDId, uint8_t *pAsdu, gpdCommissioningCmd_t *pCommissioningCmdPayload, uint8_t* pLen)
{
  uint8_t *pGpCommissioningReply = NULL;
  uint8_t len = 1; // Commissioning reply options

  if((pAsdu == NULL) || (pCommissioningCmdPayload == NULL))
  {
    // Return zero length if pointers are invalid
    return NULL;
  }
  gp_parseCommisioningCmd(pAsdu, pCommissioningCmdPayload);
  if(pCommissioningCmdPayload->options.panIdRequest == TRUE)
  {
    len += sizeof(uint16_t);
  }
  if(pCommissioningCmdPayload->options.extendedOptionsField == TRUE)
  {
    if(pCommissioningCmdPayload->extOptions.securityLevel <
       GP_SECURITY_LVL_4FC_4MIC)
    {
      // Return zero length if security check fails
      return NULL;
    }

    if((pCommissioningCmdPayload->options.securityKeyRequest == TRUE) &&
       (pCommissioningCmdPayload->extOptions.gpdKeyPresent == FALSE))
    {
      len += SEC_KEY_LEN;
      if( pCommissioningCmdPayload->extOptions.keyEncryption == TRUE )
      {
        len += 4; // MIC
      }
      if((pCommissioningCmdPayload->extOptions.securityLevel & 0x02) >=
          GP_SECURITY_LVL_4FC_4MIC)
      {
        len += 4; // Security frame Counter
      }
    }
  }

  if(pLen != NULL)
  {
    *pLen = len;
  }

  pGpCommissioningReply = zcl_mem_alloc(len);
  if ( pGpCommissioningReply != NULL)
  {
    uint8_t *pGpResponsePayload = pGpCommissioningReply;
    gpCommissioningReplyOptions_t *pCommissioningReplyOptions;

    pCommissioningReplyOptions = ((gpCommissioningReplyOptions_t*)pGpResponsePayload++);
    pCommissioningReplyOptions->panIdPresent = pCommissioningCmdPayload->options.panIdRequest;
    if(pCommissioningCmdPayload->options.extendedOptionsField == TRUE)
    {
      pCommissioningReplyOptions->gpdSecurityKeyPresent = !pCommissioningCmdPayload->extOptions.gpdKeyPresent |
                                                           pCommissioningCmdPayload->options.securityKeyRequest;
      pCommissioningReplyOptions->gpdKeyEncryption = pCommissioningCmdPayload->extOptions.keyEncryption;
      pCommissioningReplyOptions->securityLevel = pCommissioningCmdPayload->extOptions.securityLevel;
      pCommissioningReplyOptions->keyType = pCommissioningCmdPayload->extOptions.keyType;
    }
    if(pCommissioningReplyOptions->panIdPresent == TRUE)
    {
      pGpResponsePayload = zcl_memcpy(pGpResponsePayload, &_NIB.nwkPanId, sizeof(uint16_t));
    }

    //If GPD key requires encryption or decryption
    if( pCommissioningCmdPayload->extOptions.keyEncryption == TRUE )
    {
      zstack_gpEncryptDecryptCommissioningKeyRsp_t keyRsp;

      zcl_memset(&keyRsp, 0x00, sizeof(zstack_gpEncryptDecryptCommissioningKeyRsp_t));
      gp_processCommissioningKey(pGPDId, pCommissioningCmdPayload, &keyRsp);

      //Key was delivered encrypted on commissioning command
      if( pCommissioningCmdPayload->extOptions.gpdKeyPresent == TRUE )
      {
        // Save decrypted network key
        zcl_memcpy( pCommissioningCmdPayload->gpdKey, keyRsp.key, SEC_KEY_LEN );
      }
      //Key was requested and requires encryption for gp response
      else if ( 0 == pCommissioningCmdPayload->extOptions.gpdKeyPresent ||
                0 != pCommissioningCmdPayload->options.securityKeyRequest )
      {
        pGpResponsePayload = zcl_memcpy(pGpResponsePayload, keyRsp.key, SEC_KEY_LEN);
        pGpResponsePayload = zcl_memcpy(pGpResponsePayload, &keyRsp.keyMic, GP_SECURITY_MIC_SIZE);

        //If securityLevel sub-field to 0b10 or 0b11, GPDsecurityKeyPresent sub-field to 0b1 and the GPDkeyEncryption sub-field to 0b1;
        if((pCommissioningCmdPayload->extOptions.securityLevel & 0x02 ) >= GP_SECURITY_LVL_4FC_4MIC)
        {
          pGpResponsePayload = zcl_memcpy(pGpResponsePayload, &keyRsp.secFrameCounter, FRAME_COUNTER_LEN);
        }
      }
    }
    //If Key is required for gp response in plain text
    if ( 0 == pCommissioningCmdPayload->extOptions.gpdKeyPresent ||
         0 != pCommissioningCmdPayload->options.securityKeyRequest )
    {
      uint8_t gpSharedKey[SEC_KEY_LEN];

      zcl_ReadAttrData(GREEN_POWER_INTERNAL_ENDPOINT, ZCL_CLUSTER_ID_GREEN_POWER,
                       ATTRID_GP_SHARED_SEC_KEY, gpSharedKey, NULL);

      //Copy the shared key in plain text
      pGpResponsePayload = zcl_memcpy(pGpResponsePayload, gpSharedKey, SEC_KEY_LEN);
    }
  }
  return pGpCommissioningReply;
}

/*********************************************************************
 * @fn          gp_GetSinkCommissioningMode
 *
 * @brief       Get Green Power Proxy commissioning mode
 *
 * @param       enabled
 *
 * @return
 */
bool gp_GetSinkCommissioningMode(void)
{
  return zgGP_SinkCommissioningMode;
}

/*********************************************************************
 * @fn          gp_SetSinkCommissioningMode
 *
 * @brief       Enable or disable Green Power Proxy commissioning mode
 *
 * @param       enabled
 *
 * @return
 */
void gp_SetSinkCommissioningMode(bool enabled)
{
  zgGP_SinkCommissioningMode = enabled;
}
#endif

/*********************************************************************
*********************************************************************/
