/**************************************************************************************************
  Filename:       gp_proxy.c
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
#include "gp_bit_fields.h"
#include "gp_common.h"
#include "gp_proxy.h"
#include "gp_interface.h"
#include "zcl_port.h"
#include "zcl.h"
#include "dgp_stub.h"
#include "mac_api.h"
#include "zd_sec_mgr.h"
#include "bdb.h"

#if defined (ENABLE_GREENPOWER_COMBO_BASIC)
#include "gp_sink.h"
#endif

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
uint8_t  gpTempMaster = FALSE;
uint8_t  gp_commissioningOptions = 0;            //Commissioning Options from the ommissioningMode command
uint16_t gp_commissioningUnicastAddress = 0;     //address of the device to which send the notifications

uint8_t  zgGP_ProxyCommissioningMode = FALSE;    //Global flag that states if in commissioning mode or in operational mode.
uint16_t gp_commissionerAddress = 0xFFFF;        //Address to which send the notifications during commissioning mode
uint8_t  gp_unicastCommunication = 0;            //Flag to indicate if the commissioner requested unicast communication or not.
byte   gp_OperationalChannel = 0xFF;              //Holder of the operational nwk channel

 /*********************************************************************
 * EXTERNAL VARIABLES
 */


/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

 /*********************************************************************
 * LOCAL FUNCTIONS
 */
static ZStatus_t zclGp_CommissioningModeDataIndParse(gp_DataInd_t *pInd, gpCommissioningNotificationCmd_t *pGpNotification, uint8_t secKeyType);
static ZStatus_t zclGp_DataIndParse(gp_DataInd_t *pInd, gpNotificationCmd_t *pGpNotification);
static void gp_ZclPairingParse(zclGpPairing_t* pCmd, gpPairingCmd_t* payload);
static void gp_ZclProxyTableReqParse(zclGpTableRequest_t* pCmd, gpTableReqCmd_t* payload);

/*********************************************************************
 * LOCAL VARIABLES
 */
static gpCommissioningMode_t  pfnCommissioningMode = NULL;
static gpChangeChannelReq_t   pfnChangeChannelReq = NULL;
static gpChangeChannelReq_t   pfnChangeChannelReqForBDB = NULL;


/*********************************************************************
 * PUBLIC FUNCTIONS
 *********************************************************************/

/*********************************************************************
 * @fn      zclGp_GpPairingCommandCB
 *
 * @brief   Callback from the ZCL GreenPower Cluster Library when
 *          it received an Gp Pairing Command for this application.
 *
 * @param   pCmd - command payload
 *
 * @return  none
 */
void zclGp_GpPairingCommandCB( zclGpPairing_t *pCmd )
{
  gpPairingCmd_t pairingPayload;

  zcl_memset(&pairingPayload, 0x00, sizeof(gpPairingCmd_t));
  gp_ZclPairingParse(pCmd, &pairingPayload);

  if((GP_PAIRING_OPT_REMOVE_GPD(pairingPayload.options) == 0) &&
     (GP_GET_SEC_LEVEL(pairingPayload.options) == GP_SECURITY_LVL_RESERVED))
  {
    // A.3.5.2.3 line 4657
    // If RemoveGPD is 0 and SecurityLevel is 0b01, then the proxy shall not update existing or add new proxy table entry.
    return;
  }

  if(GP_PAIRING_OPT_REMOVE_GPD(pairingPayload.options))
  {
    uint16_t proxyTableIndex;
    uint8_t currEntry[PROXY_TBL_LEN];
    gpdID_t gpdID;

    if(GP_PAIRING_OPT_IS_APPLICATION_ID_GPD(pairingPayload.options))
    {
      gpdID.appID = GP_OPT_APP_ID_GPD;
      gpdID.id.srcID = pairingPayload.gpdId;
    }
    else if(GP_PAIRING_OPT_IS_APPLICATION_ID_IEEE(pairingPayload.options))
    {
      gpdID.appID = GP_OPT_APP_ID_IEEE;
      zcl_memcpy(gpdID.id.gpdExtAddr, pairingPayload.gpdIEEE, Z_EXTADDR_LEN);
    }

    if(gp_getProxyTableByGpId(&gpdID, currEntry, &proxyTableIndex) == ZSuccess)
    {
      gp_ResetProxyTblEntry(currEntry);
      zclport_writeNV(ZCL_PORT_PROXY_TABLE_NV_ID, proxyTableIndex,
                               PROXY_TBL_LEN,
                               currEntry);
    }
    return;
  }

  // Full unicast is not supported by GPP basic, not add the entry
  // as it would only interfere
  if(GP_PAIRING_IS_FULL_UNICAST(pairingPayload.options) == TRUE)
  {
    // A.3.5.2.3 When GP Pairing was received in unicast with unsupported communication mode
    // Proxy shall rsp w/ ZCL Default rsp w/ status ZCL_STATUS_INVALID_FIELD.
    // Silently drop if received in broadcast.
    if(pCmd->wasBroadcast == FALSE)
    {
      zclDefaultRspCmd_t defaultRspCmd;

      defaultRspCmd.commandID = COMMAND_GP_PAIRING;
      defaultRspCmd.statusCode = ZCL_STATUS_INVALID_FIELD;
      zcl_SendDefaultRspCmd( GREEN_POWER_INTERNAL_ENDPOINT, pCmd->srcAddr,
                              ZCL_CLUSTER_ID_GREEN_POWER, &defaultRspCmd,
                              ZCL_FRAME_CLIENT_SERVER_DIR, TRUE, 0, zcl_InSeqNum );
    }
    return;
  }

  if(gp_PairingUpdateProxyTbl(&pairingPayload) == FAILURE)
  {
    // A.3.5.2.3 When GP Pairing was received in unicast and entry could not be created due to lack of space,
    // the proxy shall rsp w/ ZCL Default rsp w/ status INSUFFICIENT_SPACE
    // Silently drop if received in broadcast.
    if(pCmd->wasBroadcast == FALSE){
      zclDefaultRspCmd_t defaultRspCmd;

      defaultRspCmd.commandID = COMMAND_GP_PAIRING;
      defaultRspCmd.statusCode = ZCL_STATUS_INSUFFICIENT_SPACE;
      zcl_SendDefaultRspCmd( GREEN_POWER_INTERNAL_ENDPOINT, pCmd->srcAddr,
                              ZCL_CLUSTER_ID_GREEN_POWER, &defaultRspCmd,
                              ZCL_FRAME_CLIENT_SERVER_DIR, TRUE, 0, zcl_InSeqNum );
    }
    return;
  }

  if(gp_commissioningOptions & GP_COMM_OPT_EXIT_ON_PAIRING_SUCCESS_MASK)
  {
    gp_stopCommissioningMode();
  }

}

/*********************************************************************
 * @fn      zclGp_GpProxyTableReqCB
 *
 * @brief   Callback from the ZCL GreenPower Cluster Library when
 *          it received a Gp proxy table req.
 *
 * @param   pCmd - command payload
 *
 * @return  none
 */
void zclGp_GpProxyTableReqCB(zclGpTableRequest_t *pCmd)
{
  uint8_t i;
  uint8_t entryLen = 0;
  uint8_t maxEntryLen = 0;
  uint8_t* buf = NULL;
  uint8_t currEntry[PROXY_TBL_LEN];
  zclGpTableResponse_t proxyTblRsp = {0};
  gpTableReqCmd_t proxyTblReqPayload = {0};

  proxyTblRsp.entry = NULL;
  gp_ZclProxyTableReqParse(pCmd, &proxyTblReqPayload);

  // To get total entries for "Total number of non-empty Proxy Table entries" field
  for(i = 0; i < GPP_MAX_PROXY_TABLE_ENTRIES; i++)
  {
    if(gp_getProxyTableByIndex(i, currEntry) == SUCCESS)
    {
      proxyTblRsp.tableEntriesTotal += 1;
    }
  }

  // for Request Table Entries by Index
  if (GP_PAIRING_OPT_IS_REQ_TYPE_INDEX(proxyTblReqPayload.options))
  {
    if((proxyTblRsp.tableEntriesTotal == 0x00) ||
       (proxyTblRsp.tableEntriesTotal < proxyTblReqPayload.index))
    {
      proxyTblRsp.status = ZCL_STATUS_NOT_FOUND;
      proxyTblRsp.startIndex = proxyTblReqPayload.index;
      proxyTblRsp.entriesCount = 0x00;
      // Send response and exit
      zclGp_SendGpProxyTableResponse(pCmd->srcAddr, &proxyTblRsp, zcl_InSeqNum);
      return;
    }
    for(i = proxyTblReqPayload.index; i <= GPP_MAX_PROXY_TABLE_ENTRIES; i++)
    {
      maxEntryLen = entryLen;    // Save the last iteration Lenght

      // Get the length of the payload
      if(pt_ZclReadGetProxyEntry(i, NULL, &entryLen) != SUCCESS)
      {
        break;
      }

      if(entryLen >= 75)   // max payload for entries
      {
        entryLen = maxEntryLen;   // The last valid lenght is the MAX for this packet
        break;
      }
    }

    if(entryLen > 0)
    {
      buf = zcl_mem_alloc(entryLen);
      if ( buf != NULL )
      {
        // Load the buffer - serially
        uint8_t *pBuf = buf;

        entryLen = 0;
        proxyTblRsp.entriesCount = 0;
        for(i = proxyTblReqPayload.index; i <= GPP_MAX_PROXY_TABLE_ENTRIES ; i++)
        {
          if(pt_ZclReadGetProxyEntry(i, pBuf, &entryLen) != SUCCESS)
          {
            break;
          }

          proxyTblRsp.entriesCount += 1;
          if(entryLen >= maxEntryLen)   // to see if we reched the MAX calculated payload
          {
            break;
          }
        }
      }
    }
  }

  // for Request Table Entries by GPD ID
  if(GP_PAIRING_OPT_IS_REQ_TYPE_GPD(proxyTblReqPayload.options))
  {
    uint8_t lookForEntry = 0;
    gpdID_t gpdID;
    uint16_t NvProxyTableIndex;
    uint8_t  ProxyTableEntryTemp[PROXY_TBL_LEN];

    if(GP_PAIRING_OPT_IS_APPLICATION_ID_GPD(proxyTblReqPayload.options))
    {
      gpdID.appID = GP_OPT_APP_ID_GPD;
      gpdID.id.srcID = proxyTblReqPayload.gpdId;
      lookForEntry = 1;
      proxyTblRsp.startIndex = 0xFF;
    }
    else if(GP_PAIRING_OPT_IS_APPLICATION_ID_IEEE(proxyTblReqPayload.options))
    {
      gpdID.appID = GP_OPT_APP_ID_IEEE;
      zcl_memcpy(gpdID.id.gpdExtAddr, proxyTblReqPayload.gpdIEEE, Z_EXTADDR_LEN);
      lookForEntry = 1;
      proxyTblRsp.startIndex = 0xFF;
    }

    if(lookForEntry)
    {
      if(gp_getProxyTableByGpId(&gpdID, ProxyTableEntryTemp, &NvProxyTableIndex) == ZSuccess)
      {
        maxEntryLen = entryLen;    // Save the last iteration Lenght

        // Get the length of the payload
        if(pt_ZclReadGetProxyEntry(NvProxyTableIndex, NULL, &entryLen) != ZSuccess)
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
        proxyTblRsp.status = ZCL_STATUS_NOT_FOUND;
      }

      if(entryLen > 0)
      {
        buf = zcl_mem_alloc(entryLen);
        if ( buf != NULL )
        {
          // Load the buffer - serially
          uint8_t *pBuf = buf;

          entryLen = 0;
          proxyTblRsp.entriesCount = 0;
          if(pt_ZclReadGetProxyEntry(NvProxyTableIndex, pBuf, &entryLen) == SUCCESS)
          {
            proxyTblRsp.entriesCount = 1;
          }
        }
      }
    }
  }

  // Send response and exit
  proxyTblRsp.startIndex = proxyTblReqPayload.index;
  proxyTblRsp.entry = buf;
  zclGp_SendGpProxyTableResponse(pCmd->srcAddr, &proxyTblRsp, zcl_InSeqNum);
  zcl_mem_free( buf );
}


/*********************************************************************
 * @fn      zclGp_GpResponseCommandCB
 *
 * @brief   Callback from the ZCL GreenPower Cluster Library when
 *          it received a Gp Response command. (Ref A.3.5.2.1)
 *
 * @param   pCmd - command payload
 *
 * @return  none
 */
void zclGp_GpResponseCommandCB(zclGpResponse_t *pCmd)
{
  gp_DataReq_t  *gp_DataReq = NULL;
  gpdID_t       gpdID;
  uint8_t         endpoint = 0;
  uint8_t         gpdCmdId;
  uint8_t         payloadLen = 0;

  //No bidirectional communication in operational mode
  //A.3.2.8
  if(zgGP_ProxyCommissioningMode == FALSE)
  {
    return;
  }

  gpdID.appID = pCmd->options & GP_RSP_CMD_OPT_APP_ID_MASK;
  if(gpdID.appID == GP_APP_ID_GPID)
  {
    gpdID.id.srcID = zcl_build_uint32(pCmd->pData,sizeof(uint32_t));
    pCmd->pData += sizeof(uint32_t);
  }
  else if(gpdID.appID == GP_APP_ID_IEEE)
  {
    zcl_cpyExtAddr(gpdID.id.gpdExtAddr, pCmd->pData);
    pCmd->pData += Z_EXTADDR_LEN;

    endpoint = *pCmd->pData;
    pCmd->pData++;
  }
  else
  {
    //Invalid app ID
    return;
  }

  // CmdId
  gpdCmdId  = *pCmd->pData++;
  // Payload length
  payloadLen = *pCmd->pData++;

  if(gpdCmdId == GP_CHANNEL_REQ_COMMAND_ID || gpdCmdId == GP_CHANNEL_CCONFIG_COMMAND_ID )
  {
      if(gpdID.appID != 0 || gpdID.id.srcID != 0)
      {
          return;
      }
  }


  gp_DataReq = (gp_DataReq_t*)OsalPort_msgAllocate(sizeof(gp_DataReq_t) + payloadLen);
  if(gp_DataReq == NULL)
  {
    //FAIL no memory
    return;
  }
  if(payloadLen == 0xFF)
  {
    payloadLen = 0;
  }
  gp_DataReq->TxOptions = GP_OPT_USE_TX_QUEUE_MASK;
  gp_DataReq->EndPoint = endpoint;
  zcl_memcpy(gp_DataReq->GPDasdu, pCmd->pData, payloadLen);
  gp_DataReq->GPDasduLength = payloadLen;
  gp_DataReq->GPDCmmdId = gpdCmdId;
  zcl_memcpy(&gp_DataReq->gpdID, &gpdID, sizeof(gpdID_t));

  gp_DataReq->GPEPhandle = gp_GetHandle(GPEP_HANDLE_TYPE);

  gp_DataReq->hdr.event = GP_DATA_REQ;
  gp_DataReq->hdr.status = 0;

  if(pCmd->options & GP_RSP_CMD_OPT_TRANSMIT_ON_ENDPOINT_MATCH_MASK)
  {
    gp_DataReq->TxOptions |= GP_OPT_TX_ON_MATCHING_ENDPOINT_MASK;
  }

  //Validate the command being send as unicast and we are the tempMaster
  if(pCmd->tempMasterShortAddr == _NIB.nwkDevAddress)
  {
    uint16_t NvProxyTableIndex;
    uint8_t  ProxyTableEntryTemp[PROXY_TBL_LEN];

    if(pfnChangeChannelReqForBDB)
    {
      //Check if we got permission from BDB
      if(!pfnChangeChannelReqForBDB())
      {
        //No permisssion
        return;
      }
    }
    if(pfnChangeChannelReq)
    {
      //Check if we got permission from BDB
      if(!pfnChangeChannelReq())
      {
        //No permisssion
        return;
      }
    }

    gpTempMaster = TRUE;

    //Check if the entry exist, if so, set the first to forward flag to 1
    if(gp_getProxyTableByGpId(&gpdID,ProxyTableEntryTemp,&NvProxyTableIndex) == ZSuccess)
    {
      //Update FirstToForward to 1
      if(PROXY_TBL_GET_FIRST_TO_FORWARD(ProxyTableEntryTemp[PROXY_TBL_OPT]) == 0)
      {
        PROXY_TBL_SET_FIRST_TO_FORWARD(&ProxyTableEntryTemp[PROXY_TBL_OPT], TRUE);
        zclport_writeNV(ZCL_PORT_PROXY_TABLE_NV_ID,
                        NvProxyTableIndex,
                        PROXY_TBL_LEN,
                        &ProxyTableEntryTemp);
      }
    }
    //Depends on TempMasterAddress
    gp_DataReq->Action = TRUE;

    //Save the operational channel.
    gp_OperationalChannel = _NIB.nwkLogicalChannel;

    //Step 6 of Section A.3.9.1 The Procedure
    //Check if we are in a different channel
    if((pCmd->tempMasterTxChannel + 11) != _NIB.nwkLogicalChannel)
    {
      //did we got permission to attend channel request?
#if (defined (USE_ICALL) || defined (OSAL_PORT2TIRTOS))
      if(UtilTimer_getTimeout(gpAppTempMasterTimeoutClkHandle) == 0)
#else
      if(OsalPortTimers_getTimerTimeout(gp_TaskID, GP_CHANNEL_CONFIGURATION_TIMEOUT))
#endif
      {
        UtilTimer_setTimeout(gpAppTempMasterTimeoutClkHandle, gpBirectionalCommissioningChangeChannelTimeout);
        UtilTimer_start(&gpAppTempMasterTimeoutClk);
      }
      _NIB.nwkLogicalChannel = pCmd->tempMasterTxChannel + 11;
      ZMacSetReq( ZMacChannel, &(_NIB.nwkLogicalChannel) );
    }

  }
  else
  {
    uint16_t NvProxyTableIndex;
    uint8_t  ProxyTableEntryTemp[PROXY_TBL_LEN];

    //We are not the tempMaster or this was not a unicast to us, set the first to forward flag to 0
    if((gp_getProxyTableByGpId(&gpdID,ProxyTableEntryTemp,&NvProxyTableIndex) == ZSuccess) &&
       (PROXY_TBL_GET_FIRST_TO_FORWARD(ProxyTableEntryTemp[PROXY_TBL_OPT]) == 1))
    {
        PROXY_TBL_SET_FIRST_TO_FORWARD(&ProxyTableEntryTemp[PROXY_TBL_OPT], FALSE);
        zclport_writeNV( ZCL_PORT_PROXY_TABLE_NV_ID,
                         NvProxyTableIndex,
                         PROXY_TBL_LEN,
                         ProxyTableEntryTemp );
    }
    //Also remove any packet to the GPD
    gp_DataReq->Action = 0;
  }
  OsalPort_msgSend(gp_TaskID,(uint8_t*)gp_DataReq);
}

/*********************************************************************
 * @fn      zclGp_GpProxyCommissioningModeCB
 *
 * @brief   Callback from the ZCL GreenPower Cluster Library when
 *          it received a Gp Commissioning Mode command.
 *
 * @param   pCmd - command payload
 *
 * @return  none
 */
void zclGp_GpProxyCommissioningModeCB(zclGpProxyCommissioningMode_t* pCmd)
{
  gp_commissioningOptions = pCmd->options;
  bool commissioningWindowFlag = FALSE;
  uint32_t commissioningWindow = ppgCommissioningWindow;

  if((zgGP_ProxyCommissioningMode == TRUE) &&
     (gp_commissionerAddress != pCmd->srcAddr))
  {
    // If is in commissioning mode and the soruce address is different from
    // the device that set the proxy in commissioning mode, then drop
    // the request.
    return;
  }

  //Enter in commissioning mode
  if(gp_commissioningOptions & GP_COMM_OPT_ACTION_MASK)
  {

    if(gp_commissioningOptions & GP_COMM_OPT_EXIT_ON_WINDOW_EXPIRATION_MASK)
    {
      commissioningWindow = OsalPort_buildUint16(pCmd->pData);
      pCmd->pData += sizeof(uint16_t);
      commissioningWindowFlag = TRUE;
    }

    gp_commissionerAddress = pCmd->srcAddr;

    if(gp_commissioningOptions & GP_COMM_OPT_UNICAST_COMM_MASK)
    {
      gp_unicastCommunication = TRUE;
    }
    else
    {
      gp_unicastCommunication = FALSE;
    }

    //Exit upon expire
    zgGP_ProxyCommissioningMode = TRUE;

    //Notify the user about entering in commissioning mode
    if(pfnCommissioningMode)
    {
      pfnCommissioningMode(TRUE, commissioningWindowFlag, commissioningWindow);
    }
  }
  //Exit commissioning mode
  else
  {
    gp_stopCommissioningMode();
  }
}

/*********************************************************************
* @fn          gp_ProxyTblInit
*
* @brief       General function to init the NV items for proxy table
*
* @param
*
* @return
*/
uint8_t gp_ProxyTblInit(uint8_t resetTable)
{
 uint8_t i;
 uint8_t status;
 uint8_t emptyEntry[PROXY_TBL_LEN];

 gp_ResetProxyTblEntry(emptyEntry);

 for(i = 0; i < GPP_MAX_PROXY_TABLE_ENTRIES ; i++)
 {
   status = zclport_initializeNVItem(ZCL_PORT_PROXY_TABLE_NV_ID, i,
                                     PROXY_TBL_LEN,
                                     emptyEntry);

   if((status != SUCCESS) && (status != NV_ITEM_UNINIT))
   {
     return status;
   }
   else if((status == SUCCESS) && (resetTable == TRUE))
   {
     status = zclport_writeNV(ZCL_PORT_PROXY_TABLE_NV_ID, i,
                              PROXY_TBL_LEN,
                              emptyEntry );
   }
 }
 return status;
}

/*********************************************************************
 * @fn      gp_stopCommissioningMode
 *
 * @brief   Stops the commissioning mode
 *
 * @param   none
 *
 * @return  none
 */
void gp_stopCommissioningMode(void)
{
  uint8_t i;
  //Callback to notify about the end of the commissioning mode
  gp_commissioningOptions = 0;
  zgGP_ProxyCommissioningMode = FALSE;
  gp_commissionerAddress = 0xFFFF;
  gp_unicastCommunication = 0;

  // Release the gpTxQueue, proxy does not support bidirectional
  // communication on operational mode
  // A.3.2.8
  for (i = 0; i < gGP_TX_QUEUE_MAX_ENTRY; i++)
  {
    if(gpTxQueueList[i].gp_DataReq != NULL)
    {
      OsalPort_msgDeallocate((void*)gpTxQueueList[i].gp_DataReq);
      gpTxQueueList[i].gp_DataReq = NULL;
    }
  }

  //Notify the user that we have exited the commissioning mode
  if(pfnCommissioningMode)
  {
    pfnCommissioningMode(FALSE, FALSE, 0);
  }
}

 /*********************************************************************
 * @fn          gp_getProxyTableByGpId
 *
 * @brief       General function to get proxy table entry by gpdID (GP Src ID or Extended Adddress)
 *
 * @param       gpdID  - address to look for in the table
 *              pEntry  - buffer in which the entry of the table will be copied
 *
 * @return
 */
uint8_t gp_getProxyTableByGpId(gpdID_t *gpdID, uint8_t *pEntry, uint16_t *NvProxyTableIndex)
{
  uint8_t i;
  uint8_t status;

  if((pEntry == NULL) || (gpdID == NULL) || (NvProxyTableIndex == NULL))
  {
    return ZFailure;
  }

  for(i = 0; i < GPP_MAX_PROXY_TABLE_ENTRIES ; i++)
  {
    status = gp_getProxyTableByIndex(i, pEntry);
    if(status == NV_OPER_FAILED)
    {
      // FAIL
      return ZFailure;
    }

    // if the entry is empty
    if(status == NV_INVALID_DATA)
    {
      continue;
    }

    //Check that App ID is the same
    if(GP_TBL_COMP_APPLICATION_ID(gpdID->appID, pEntry[PROXY_TBL_OPT]))
    {
      if((gpdID->appID == GP_OPT_APP_ID_GPD) &&
        zcl_memcmp( &gpdID->id.srcID, &pEntry[PROXY_TBL_GPD_ID + 4], sizeof(uint32_t)))
      {
        // Entry found
        *NvProxyTableIndex = i;
        return ZSuccess;
      }
      else if((gpdID->appID == GP_OPT_APP_ID_IEEE) &&
              zcl_memcmp(&gpdID->id.gpdExtAddr, &pEntry[PROXY_TBL_GPD_ID], Z_EXTADDR_LEN))
      {
        // Entry found
        *NvProxyTableIndex = i;
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
 * @fn          gp_getProxyTableByIndex
 *
 * @brief       General function to get proxy table entry by NV index
 *
 * @param       nvIndex - NV Id of proxy table
 *              pEntry  - pointer to PROXY_TBL_LEN array
 *
 * @return
 */
uint8_t gp_getProxyTableByIndex( uint16_t nvIndex, uint8_t *pEntry )
{
  uint8_t status;
  uint16_t emptyEntry = 0xFFFF;

  status = zclport_readNV(ZCL_PORT_PROXY_TABLE_NV_ID, nvIndex,
                            0,
                            PROXY_TBL_LEN,
                            pEntry);

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
 * @fn          gp_dataIndProxy
 *
 * @brief       This passes the MCPS data indications received in MAC to the application
 *
 * @param       gp_DataInd
 *
 * @return
 */
void gp_dataIndProxy(gp_DataInd_t *gp_DataInd)
{
  uint32_t   timeout;
  gpdID_t  gpdID;
  uint8_t    pProxyTableEntry[PROXY_TBL_LEN];
  uint16_t   nvIndex = 0;
  uint8_t proxyTableStatus = ZFailure;

  if((gp_DataInd->frameType == GP_NWK_FRAME_TYPE_MAINTENANCE ) &&
    ((gp_DataInd->AutoCommissioning == TRUE) &&
     (zgGP_ProxyCommissioningMode == FALSE)))
  {
    return;
  }

  //If authentication/decryption fail, then drop the frame and stop processing
  //GP Basic proxy A.3.5.2.3
  if( zgGP_ProxyCommissioningMode == FALSE &&
     (gp_DataInd->status == GP_DATA_IND_STATUS_COUNTER_FAILURE ||
      gp_DataInd->status == GP_DATA_IND_STATUS_AUTH_FAILURE
#if defined (ENABLE_GREENPOWER_COMBO_BASIC)
      || gp_DataInd->status == GP_DATA_IND_STATUS_UNPROCESSED
#endif
      ))
  {
    gp_expireDuplicateFiltering();
    return;
  }

  //According to A.3.9.1 step 9a, no other frame can be received.
  //If the TempMaster receives any other GPDF than Channel Request
  //GPDF on TransmitChannel
  if((gpTempMaster == TRUE) &&
     (gp_OperationalChannel != _NIB.nwkLogicalChannel))
  {
     if(gp_DataInd->GPDCmmdID != GP_CHANNEL_REQ_COMMAND_ID)
     {
         return;
     }
     //We are on a different channel, and received the GP channel request command,
     //no need to wait, send the frame and get back to our channel.
     //Frame is pending to send in less than 20ms, so wait 200 to be sure that
     //it got send before changing channel.

     if(UtilTimer_isActive(&gpAppTempMasterTimeoutClk) == TRUE)
     {
         UtilTimer_stop(&gpAppTempMasterTimeoutClk);
         UtilTimer_setTimeout(gpAppTempMasterTimeoutClkHandle, 200);
         UtilTimer_start(&gpAppTempMasterTimeoutClk);
         return;
     }

  }
  gp_DataInd->SecReqHandling.timeout = gpDuplicateTimeout;
  //Consider the current time elapsed to the next timeout
#if (defined (USE_ICALL) || defined (OSAL_PORT2TIRTOS))
  timeout =  UtilTimer_getTimeout(gpAppExpireDuplicateClkHandle);
#else
  timeout = OsalPortTimers_getTimerTimeout(gp_TaskID, GP_DUPLICATE_FILTERING_TIMEOUT_EVENT);
#endif

  if(timeout > 0x00)
  {
    gp_DataInd->SecReqHandling.timeout += timeout;
  }
  else
  {
#if (defined (USE_ICALL) || defined (OSAL_PORT2TIRTOS))
  UtilTimer_setTimeout(gpAppExpireDuplicateClkHandle, gp_DataInd->SecReqHandling.timeout);
  UtilTimer_start(&gpAppExpireDuplicateClk);
#else
  OsalPortTimers_startTimer(gp_TaskID,
                         GP_DUPLICATE_FILTERING_TIMEOUT_EVENT,
                         gp_DataInd->SecReqHandling.timeout);
#endif
  }

  gpdID.appID = gp_DataInd->appID;
  if(gp_DataInd->appID == GP_OPT_APP_ID_IEEE)
  {
    zcl_memcpy(gpdID.id.gpdExtAddr, gp_DataInd->srcAddr.addr.extAddr, Z_EXTADDR_LEN);
  }
  else if(gp_DataInd->appID == GP_OPT_APP_ID_GPD)
  {
    gpdID.id.srcID = gp_DataInd->SrcId;
  }

  proxyTableStatus = gp_getProxyTableByGpId(&gpdID, pProxyTableEntry, &nvIndex);
  if(proxyTableStatus == ZSuccess)
  {
    // A3.9.1 Step 12. d.
    // When recieving unprotectred GPDF for GPD in proxy table w/ non-zero SecurityLevel, Proxy will not update
    // GPD SecFrameCounter for this proxy table entry.
    if(SEC_PARAMETERS_GET_SEC_OPT_SECURITY_LVL(pProxyTableEntry[PROXY_TBL_SEC_OPT]) == gp_DataInd->GPDFSecLvl)
    {
      //Update Sec Frame counter to proxy table  A.3.6.1.3
      zcl_memcpy(&pProxyTableEntry[PROXY_TBL_SEC_FRAME],
                (uint8_t*)&gp_DataInd->GPDSecFrameCounter,
                sizeof(uint32_t));

      zclport_writeNV(ZCL_PORT_PROXY_TABLE_NV_ID, nvIndex,
                        PROXY_TBL_LEN,
                        pProxyTableEntry);
    }


    // When Groupcast Radius is set to 0, this mean unspecified and to set radius to AF_DEFAULT_RADIUS (2 * nwkMaxDepth).
    // If Groupcast Radius is set to greater than 0, then use this as the radius when forwarding GPDF packet.
    if(pProxyTableEntry[PROXY_TBL_RADIUS] > 0)
    {
        zcl_radius = pProxyTableEntry[PROXY_TBL_RADIUS];
    }
  }

  if(zgGP_ProxyCommissioningMode == TRUE)
  {
    if(gp_DataInd->GPDCmmdID == GP_CHANNEL_REQ_COMMAND_ID)
    {
      if(pfnChangeChannelReqForBDB)
      {
        //Check if we got permission from BDB
        if(!pfnChangeChannelReqForBDB())
        {
          //No permisssion
          return;
        }
      }
      if(pfnChangeChannelReq)
      {
        //Check if we got permission from BDB
        if(!pfnChangeChannelReq())
        {
          //No permisssion
          return;
        }
      }
    }

    // A.3.9.1 step 17 a
    // When a proxy recieves a Success GPDF with Auto Commissioning True, the frame is silently dropped.
    if((gp_DataInd->GPDCmmdID == GP_SUCCESS_COMMAND_ID) && (gp_DataInd->AutoCommissioning == TRUE))
    {
      return;
    }

    gpCommissioningNotificationCmd_t gpNotification;
    uint8_t* pEntry = NULL;
    uint8_t secKeyType = GP_SECURITY_KEY_TYPE_NO_KEY;

    if(proxyTableStatus == ZSuccess)
    {
        pEntry = pProxyTableEntry;
        secKeyType = SEC_PARAMETERS_GET_SEC_OPT_SECURITY_KEY_TYP(pEntry[PROXY_TBL_SEC_OPT]);
    }

    secKeyType = SEC_PARAMETERS_GET_SEC_OPT_SECURITY_KEY_TYP(pEntry[PROXY_TBL_SEC_OPT]);
    zcl_memset(&gpNotification, 0, sizeof(gpCommissioningNotificationCmd_t));
    zclGp_CommissioningModeDataIndParse(gp_DataInd, &gpNotification, secKeyType);
    zclGp_SendGpCommissioningNotificationCommand(&gpNotification, gp_DataInd->SeqNumber, &gpdID, pEntry);
  }
  // Send Green Power Notification
  else if((zgGP_ProxyCommissioningMode == FALSE) &&
          ((gp_DataInd->status == GP_DATA_IND_STATUS_SECURITY_SUCCESS) ||
           (gp_DataInd->status == GP_DATA_IND_STATUS_NO_SECURITY)
#if !defined (ENABLE_GREENPOWER_COMBO_BASIC)
         ||(gp_DataInd->status == GP_DATA_IND_STATUS_UNPROCESSED)
#endif
         ))
  {
    gpNotificationCmd_t gpNotification;

    zcl_memset(&gpNotification, 0, sizeof(gpNotificationCmd_t));
    zclGp_DataIndParse(gp_DataInd, &gpNotification);
    zclGp_SendGpNotificationCommand(&gpNotification, gp_DataInd->SeqNumber);
  }
}

/*********************************************************************
* @fn          gp_SecurityOperationProxy
*
* @brief       Performs Security Operations according to Proxy
*
* @param       ind - pointer to gp data indication
* @param       pFrameCounter
* @param       pKeyType
* @param       pkey Key to be used to decript the packet (if applies)
*
* @return      GP_SEC_RSP status
*/
uint8_t gp_SecurityOperationProxy( gp_SecReq_t* pInd, uint8_t* pKeyType, void* pKey)
{
 uint8_t    currEntry[PROXY_TBL_LEN];
 uint8_t    status;
 uint32_t   SecFrameCounter = 0;
 uint8_t    securityCheckFail = FALSE;
 uint8_t    endpointCheckFail = FALSE;
 uint16_t   NvProxyTableIndex = 0;
 gpdID_t  gpdID;


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

 status = gp_getProxyTableByGpId(&gpdID, currEntry, &NvProxyTableIndex);

 //Not found
 if(status == ZInvalidParameter)
 {
   //Section A.3.5.2.1 if not in commissioning mode and proxy
   //have an entry for GPD, then drop frame
   if(zgGP_ProxyCommissioningMode == FALSE)
   {
     return GP_SEC_RSP_DROP_FRAME;
   }
   if(pInd->gp_SecData.GPDFSecLvl >= GP_SECURITY_LVL_4FC_4MIC)
   {
     return GP_SEC_RSP_TX_THEN_DROP;
   }
   if(pInd->gp_SecData.GPDFKeyType == 0)
   {
     //If there is no shared key, then pass unprocessed
     if(zcl_memcmp(pKey, (void*)zgpSharedKey, SEC_KEY_LEN))
     {
       return GP_SEC_RSP_PASS_UNPROCESSED;
     }
     zcl_memcpy(pKey, (void*)zgpSharedKey, SEC_KEY_LEN);
     return GP_SEC_RSP_PASS_UNPROCESSED;
   }
   else
   {
     return GP_SEC_RSP_PASS_UNPROCESSED;
   }
 }

 //error
 else if(status == ZFailure)
 {
   //Not found, or error, drop the frame
   return GP_SEC_RSP_DROP_FRAME;
 }

 //Found
 else
 {
     if(gpdID.appID == GP_APP_ID_IEEE)
     {
       // Not the same Ep, then drop the frame
       if((pInd->EndPoint != currEntry[PROXY_TBL_GPD_EP]) &&
          (pInd->EndPoint != 0x00) &&
          (pInd->EndPoint != 0xFF))
       {
         return GP_SEC_RSP_DROP_FRAME;
       }
     }

   //Active/Inactive entries in the proxy table are not supported
   //If security level is zero then don't check and pass unprocessed
   if((pInd->gp_SecData.GPDFSecLvl == SEC_PARAMETERS_GET_SEC_OPT_SECURITY_LVL(currEntry[PROXY_TBL_SEC_OPT])) &&
       (GP_SECURITY_LVL_NO_SEC == SEC_PARAMETERS_GET_SEC_OPT_SECURITY_LVL(currEntry[PROXY_TBL_SEC_OPT])))
   {
     return GP_SEC_RSP_PASS_UNPROCESSED;
   }
   //Check security Section A.3.7.3.3
   //Check framecounter freshness
   zcl_memcpy(&SecFrameCounter, &currEntry[PROXY_TBL_SEC_FRAME], sizeof(uint32_t));
   if(SecFrameCounter >= pInd->gp_SecData.GPDSecFrameCounter)
   {
     securityCheckFail = TRUE;
   }
   //Compare the security level
   else if(pInd->gp_SecData.GPDFSecLvl !=
           SEC_PARAMETERS_GET_SEC_OPT_SECURITY_LVL(currEntry[PROXY_TBL_SEC_OPT]))
   {
     securityCheckFail = TRUE;
   }
   //Mapping of security key type (section A.1.4.1.3 Table 12)
   else if((SEC_PARAMETERS_GET_SEC_OPT_SECURITY_KEY_TYP(currEntry[PROXY_TBL_SEC_OPT]) <= 0x03) &&
           (pInd->gp_SecData.GPDFKeyType == 1))
   {
     securityCheckFail = TRUE;
   }
   else if(((SEC_PARAMETERS_GET_SEC_OPT_SECURITY_KEY_TYP(currEntry[PROXY_TBL_SEC_OPT]) == 0x07) ||
            (SEC_PARAMETERS_GET_SEC_OPT_SECURITY_KEY_TYP(currEntry[PROXY_TBL_SEC_OPT]) == 0x04)) &&
            (pInd->gp_SecData.GPDFKeyType == 0))
   {
     securityCheckFail = TRUE;

   }
   else if((SEC_PARAMETERS_GET_SEC_OPT_SECURITY_KEY_TYP(currEntry[PROXY_TBL_SEC_OPT]) == 0x05) ||
           (SEC_PARAMETERS_GET_SEC_OPT_SECURITY_KEY_TYP(currEntry[PROXY_TBL_SEC_OPT]) == 0x06))
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
     zcl_memcpy(pKey, &currEntry[PROXY_TBL_GPD_KEY], SEC_KEY_LEN);
     *pKeyType = SEC_PARAMETERS_GET_SEC_OPT_SECURITY_KEY_TYP(currEntry[PROXY_TBL_SEC_OPT]);

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
}

/*********************************************************************
 * @fn          GP_DataCnf
 *
 * @brief       Primitive to notify GP EndPoint the status of a previews DataReq
 *
 * @param       gp_DataCnf
 *
 * @return      none
 */
void GP_DataCnf(gp_DataCnf_t *gp_DataCnf)
{
  //Current spec does not mandates to do anything with this.
  switch(gp_DataCnf->status)
  {
    case GP_DATA_CNF_TX_QUEUE_FULL:
    case GP_DATA_CNF_ENTRY_REPLACED:
    case GP_DATA_CNF_ENTRY_ADDED:
    case GP_DATA_CNF_ENTRY_EXPIRED:
    case GP_DATA_CNF_ENTRY_REMOVED:
    case GP_DATA_CNF_GPDF_SENDING_FINALIZED:
    break;
  }
}

/*********************************************************************
 * @fn          gp_RegisterCommissioningModeCB
 *
 * @brief       Register a callback in which the application will be notified about
 *              commissioning mode indication
 *
 * @param       gpCommissioningModeCB
 *
 * @return      none
 */
void gp_RegisterCommissioningModeCB(gpCommissioningMode_t gpCommissioningModeCB)
{
  pfnCommissioningMode = gpCommissioningModeCB;
}

/*********************************************************************
 * @fn          gp_RegisterGPChangeChannelReqCB
 *
 * @brief       Register a callback in which the application will be notified about a change
 *              of channel for at most gpBirectionalCommissioningChangeChannelTimeout ms
 *              to perform GP bidirectional commissioning in the channel parameter.
 *
 * @param       gpChangeChannelReq
 *
 * @return      none
 */
void gp_RegisterGPChangeChannelReqCB(gpChangeChannelReq_t gpChangeChannelReqCB)
{
  pfnChangeChannelReq = gpChangeChannelReqCB;
}

/*********************************************************************
 * @fn          gp_RegisterGPChangeChannelReqForBDBCB
 *
 * @brief       Register a callback in which the bdb will be notified about a change
 *              of channel for at most gpBirectionalCommissioningChangeChannelTimeout ms
 *              to perform GP bidirectional commissioning in the channel parameter.
 *
 * @param       gpChangeChannelReq
 *
 * @return      none
 */
void gp_RegisterGPChangeChannelReqForBDBCB(gpChangeChannelReq_t gpChangeChannelReqCB)
{
  pfnChangeChannelReqForBDB = gpChangeChannelReqCB;
}

/*********************************************************************
 * @fn          gp_returnOperationalChannel
 *
 * @brief       Return to the operational channel after bidirectional commissioning
 *
 * @param       none
 *
 * @return      none
 */
void gp_returnOperationalChannel(void)
{
  gp_DataReq_t gp_DataReq;
#if (defined (USE_ICALL) || defined (OSAL_PORT2TIRTOS))
  if(UtilTimer_isActive(&gpAppTempMasterTimeoutClk) == TRUE)
  {
    UtilTimer_stop(&gpAppTempMasterTimeoutClk);
  }
#else
  OsalPortTimers_stopTimer(gp_TaskID, GP_CHANNEL_CONFIGURATION_TIMEOUT);
#endif

  gp_DataReq.Action = 0;
  gp_DataReq.gpdID.appID = GP_APP_ID_GPID;
  gp_DataReq.gpdID.id.srcID = 0;
  gp_DataReq.TxOptions = GP_OPT_USE_TX_QUEUE_MASK;
  gpTempMaster = FALSE;

  GP_DataReq(&gp_DataReq);
  _NIB.nwkLogicalChannel = gp_OperationalChannel;
  gp_OperationalChannel = 0xFF;
  ZMacSetReq( ZMacChannel, &(_NIB.nwkLogicalChannel) );

}

/*********************************************************************
 * @fn          gp_GetProxyCommissioningMode
 *
 * @brief       Get Green Power Proxy commissioning mode
 *
 * @param       enabled
 *
 * @return
 */
bool gp_GetProxyCommissioningMode(void)
{
    return zgGP_ProxyCommissioningMode;
}

/*********************************************************************
 * @fn          gp_SetProxyCommissioningMode
 *
 * @brief       Enable or disable Green Power Proxy commissioning mode
 *
 * @param       enabled
 *
 * @return
 */
void gp_SetProxyCommissioningMode(bool enabled)
{
    zgGP_ProxyCommissioningMode = enabled;
}

 /*********************************************************************
 * PRIVATE FUNCTIONS
 *********************************************************************/

/*********************************************************************
 * @fn      zclGp_CommissioningModeDataIndParse
 *
 * @brief   Parse the Gp Data indication to Gp Commissioning Notification command
 *
 * @param   pInd - Pointer to the incoming data
 *          pGpNotification - Pointer to out going data
 *          secKeyType - security key type of GPD from the proxy table
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclGp_CommissioningModeDataIndParse( gp_DataInd_t *pInd, gpCommissioningNotificationCmd_t *pGpNotification, uint8_t secKeyType )
{
  gpCommissioningNotificationOptions_t commissioningNtfOptions;
  int8_t RSSI;
  uint8_t LQI;

  zcl_memset(&commissioningNtfOptions, 0x00, sizeof(gpCommissioningNotificationOptions_t));
  if(pInd->GPDCmmdID != GP_CHANNEL_REQ_COMMAND_ID)
  {
    if(GP_OPT_IS_APPLICATION_ID_GPD( pInd->appID ))
    {
      pGpNotification->gpdId = pInd->SrcId;
      commissioningNtfOptions.appId = GP_OPT_APP_ID_GPD;
    }
    else if(GP_OPT_IS_APPLICATION_ID_IEEE(pInd->appID))
    {
      zcl_memcpy(pGpNotification->gpdIEEE, &(pInd->srcAddr.addr.extAddr), Z_EXTADDR_LEN);
      pGpNotification->ep = pInd->EndPoint;
      commissioningNtfOptions.appId = GP_OPT_APP_ID_IEEE;
    }
  }

  // Set the options bit field
  if(pInd->GPDCmmdID == GP_CHANNEL_REQ_COMMAND_ID)
  {
    commissioningNtfOptions.rxAfterTx = TRUE;
  }
  else
  {
    commissioningNtfOptions.rxAfterTx = pInd->RxAfterTx;
  }

  // A.3.3.4.3 GP Commissioning Notification command
  // From specification v1.1. All proxy basic devices implementing the current specification
  // SHALL always set the BidirectionalCommunicationCapability sub-field to 0b0
  commissioningNtfOptions.bidirectionalCapability = FALSE;
  commissioningNtfOptions.securityLevel = pInd->GPDFSecLvl;
  commissioningNtfOptions.proxyInfoPresent = TRUE;
  commissioningNtfOptions.reserved = 0;

  if(pInd->GPDFSecLvl >= GP_SECURITY_LVL_4FC_4MIC)
  {
    if(pInd->GPDFKeyType == GP_SHARED_KEY_TYPE)
    {
        if ((secKeyType == GP_SECURITY_KEY_TYPE_ZIGBEE_NWK_KEY) || (secKeyType == GP_SECURITY_KEY_TYPE_GPD_GROUP_KEY)
             || (secKeyType == GP_SECURITY_KEY_TYPE_OUT_OF_BOX_GPD_KEY))
        {
            commissioningNtfOptions.securityKeyType = secKeyType;
        }
        else
        {
            commissioningNtfOptions.securityKeyType = GP_SECURITY_KEY_TYPE_NO_KEY;
        }

    }
    else if(pInd->GPDFKeyType == GP_INDIVIDUAL_KEY_TYPE)
    {
      commissioningNtfOptions.securityKeyType = GP_SECURITY_KEY_TYPE_OUT_OF_BOX_GPD_KEY;
    }
  }
  else if(pInd->GPDFSecLvl == GP_SECURITY_LVL_NO_SEC)
  {
    commissioningNtfOptions.securityKeyType = GP_SECURITY_KEY_TYPE_NO_KEY;
  }

  if( pInd->status == GP_DATA_IND_STATUS_AUTH_FAILURE )
  {
    commissioningNtfOptions.securityProcessingFailed = TRUE;
    pGpNotification->mic = pInd->MIC;
  }

  pGpNotification->options = 0x00;
  pGpNotification->options |= commissioningNtfOptions.appId;
  pGpNotification->options |= commissioningNtfOptions.rxAfterTx <<3;
  pGpNotification->options |= commissioningNtfOptions.securityLevel << 4;
  pGpNotification->options |= commissioningNtfOptions.securityKeyType << 6;
  pGpNotification->options |= commissioningNtfOptions.securityProcessingFailed << 9;
  pGpNotification->options |= commissioningNtfOptions.bidirectionalCapability << 10;
  pGpNotification->options |= commissioningNtfOptions.proxyInfoPresent << 11;

  pGpNotification->gpdSecCounter = pInd->GPDSecFrameCounter;
  pGpNotification->cmdId = pInd->GPDCmmdID;

  if ( pInd->GPDasduLength > 0 )
  {
    pGpNotification->payloadLen = pInd->GPDasduLength;
    pGpNotification->cmdPayload = pInd->GPDasdu;
  }

  pGpNotification->gppShortAddr = _NIB.nwkDevAddress;

  RSSI = pInd->Rssi;
  (RSSI > 8) ?RSSI = 8 : (RSSI < -109) ?RSSI = -109 : 0;
  RSSI += 110;
  RSSI /= 2;

  (pInd->LinkQuality == 0) ?LQI = 0 : (pInd->LinkQuality > 0) ?LQI = 2 : 0;

  pGpNotification->gppGpdLink = RSSI;
  pGpNotification->gppGpdLink |= (LQI<<6);

  return SUCCESS;
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
static ZStatus_t zclGp_DataIndParse( gp_DataInd_t *pInd, gpNotificationCmd_t *pGpNotification )
{
  uint8_t currEntry[PROXY_TBL_LEN];
  uint8_t  ntfOpt[2] = {0x00, 0x00};
  uint8_t i;
  int8_t RSSI;
  uint8_t LQI;
  ZStatus_t status;

  for(i = 0; i < GPP_MAX_PROXY_TABLE_ENTRIES ; i++)
  {
    status = gp_getProxyTableByIndex(i, currEntry);

    if(status == NV_OPER_FAILED)
    {
      return status;
    }

    // if the entry is empty
    if(status == NV_INVALID_DATA)
    {
      continue;
    }

    if(GP_TBL_COMP_APPLICATION_ID( pInd->appID, currEntry[PROXY_TBL_OPT]))
    {
      if((pInd->appID == GP_OPT_APP_ID_GPD) &&
          (zcl_memcmp( &pInd->SrcId, &currEntry[PROXY_TBL_GPD_ID + 4], sizeof(uint32_t))))
      {
        // Entry found
        pGpNotification->gpdId = pInd->SrcId;
        ntfOpt[0] = GP_OPT_APP_ID_GPD;
        status = SUCCESS;
        break;
      }
      else if((pInd->appID == GP_OPT_APP_ID_IEEE) &&
              (zcl_memcmp(&pInd->srcAddr, &currEntry[PROXY_TBL_GPD_ID], Z_EXTADDR_LEN)))
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

  if(status == SUCCESS)
  {
      // Set the options bit field
      GP_NTF_SET_ALSO_UNICAST((uint8_t*)&ntfOpt[0], currEntry[PROXY_TBL_OPT]);
      GP_NTF_SET_ALSO_DGROUP((uint8_t*)&ntfOpt[0], currEntry[PROXY_TBL_OPT]);
      GP_NTF_SET_ALSO_CGROUP((uint8_t*)&ntfOpt[0], currEntry[PROXY_TBL_OPT + 1]);
      GP_NTF_SET_SEC_LEVEL((uint8_t*)&ntfOpt[0], currEntry[PROXY_TBL_SEC_OPT]);
      GP_NTF_SET_SEC_KEY_TYPE((uint8_t*)&ntfOpt[1], currEntry[PROXY_TBL_SEC_OPT]);
      pInd->RxAfterTx ?GP_SET_RX_AFTER_TX((uint8_t*)&ntfOpt[1]) : GP_CLR_RX_AFTER_TX((uint8_t*)&ntfOpt[1]);

      // Mandatory to set this sub fields to 0b1 by the Green Power Basic Proxy Spec in A.3.3.4.1
      PROXY_TBL_SET_TX_QUEUE_FULL((uint8_t*)&ntfOpt[1]);
      PROXY_TBL_CLR_BIDIRECTIONAL_CAP((uint8_t*)&ntfOpt[1]);
      PROXY_TBL_SET_PROXY_INFO((uint8_t* )&ntfOpt[1]);

      pGpNotification->options |= ((ntfOpt[0]) & 0x00FF);
      pGpNotification->options |= (((ntfOpt[1]) << 8) & 0xFF00);

      pGpNotification->gpdSecCounter = pInd->GPDSecFrameCounter;
      pGpNotification->cmdId = pInd->GPDCmmdID;

      if(pInd->GPDasduLength > 0)
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
      (RSSI > 8) ?RSSI = 8 : (RSSI < -109) ?RSSI = -109 : 0;
      RSSI += 110;
      RSSI /= 2;
      (pInd->LinkQuality == 0) ?LQI = 0 : (pInd->LinkQuality > 0) ?LQI = 2 : 0;
      pGpNotification->gppGpdLink = RSSI;
      pGpNotification->gppGpdLink |= (LQI<<6);
  }
  return status;
}

 /*********************************************************************
 * @fn          gp_ZclPairingParse
 *
 * @brief       Parse the pairing data message payload
 *
 * @param
 *
 * @return
 */
static void gp_ZclPairingParse( zclGpPairing_t* pCmd, gpPairingCmd_t* payload )
{

  payload->options |= (((uint32_t)pCmd->options[2] << 16) & 0x00FF0000);
  payload->options |= (((uint32_t)pCmd->options[1] <<  8) & 0x0000FF00);
  payload->options |= (((uint32_t)pCmd->options[0])       & 0x000000FF);

  // Options bitfield
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
  else if(GP_PAIRING_OPT_IS_APPLICATION_ID_IEEE(payload->options))
  {
    // Populate GPD ID Invalid
    payload->gpdId = 0xFFFFFFFF;

    // Populate GPD IEEE
    gp_PopulateField((uint8_t*)&payload->gpdIEEE, &pCmd->pData, Z_EXTADDR_LEN);
    // Populate EP
    gp_PopulateField((uint8_t*)&payload->ep, &pCmd->pData, sizeof(payload->ep));
  }

  // If Remove GPD bit is 0b0
  if(!GP_PAIRING_OPT_REMOVE_GPD(payload->options))
  {
    // Communication Mode 0b00 or 0b11
    if(GP_PAIRING_IS_FULL_UNICAST(payload->options) ||
       GP_PAIRING_IS_LIGHT_UNICAST(payload->options))
    {
      // Populate Sink Addesses
      gp_PopulateField((uint8_t*)&payload->sinkIEEE, &pCmd->pData, Z_EXTADDR_LEN);
      gp_PopulateField((uint8_t*)&payload->sinkNwkAddr, &pCmd->pData, sizeof(payload->sinkNwkAddr));

      // Populate Grp Address Ivalid
      payload->sinkGroupID = 0xFFFF;
    }
    // Communication Mode Groupcast Forwarding Pre-Commissioned Group ID
    else if((GP_PAIRING_IS_GRPCAST_DGROUP(payload->options)) ||
            (GP_PAIRING_IS_GRPCAST_CGROUP(payload->options )))
    {
      // Populate Sink Addresses Invalid
      zcl_memset(&payload->sinkIEEE, 0xFF, Z_EXTADDR_LEN);
      payload->sinkNwkAddr = 0xFFFF;

      // Populate Grp Address
      gp_PopulateField((uint8_t*)&payload->sinkGroupID, &pCmd->pData, sizeof(payload->sinkGroupID));
    }
  }
  else
  {
    uint8_t i;
    uint8_t status;
    uint8_t currEntry[PROXY_TBL_LEN];

    // Remove
    for(i = 0; i < GPP_MAX_PROXY_TABLE_ENTRIES ; i++)
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

      if((GP_PAIRING_OPT_IS_APPLICATION_ID_GPD(payload->options)) &&
         (zcl_memcmp(&currEntry[PROXY_TBL_GPD_ID + 4], &payload->gpdId, sizeof(uint32_t))))
      {
        // Remove this GPD entry
          gp_ResetProxyTblEntry( currEntry );
      }
      else if((GP_PAIRING_OPT_IS_APPLICATION_ID_IEEE(payload->options)) &&
              (zcl_memcmp( &currEntry[PROXY_TBL_GPD_ID ], payload->sinkIEEE, Z_EXTADDR_LEN)))
      {
        // Remove this GPD entry
          gp_ResetProxyTblEntry(currEntry);
      }
    }
    return;
  }

  // If Add Sink bit is 0b1
  if(GP_PAIRING_OPT_ADD_SINK(payload->options))
  {
    // Populate Device ID
    gp_PopulateField((uint8_t*)&payload->deviceId, &pCmd->pData, sizeof(payload->deviceId));

    if(GP_SEC_COUNTER(payload->options))
    {
      // Populate GPD security frame counter
      gp_PopulateField((uint8_t*)&payload->gpdSecCounter, &pCmd->pData, sizeof(payload->gpdSecCounter));
    }
    else
    {
      payload->gpdSecCounter = 0xFFFFFFFF;
    }

    if(GP_SEC_KEY(payload->options))
    {
      gp_PopulateField((uint8_t* )&payload->gpdKey, &pCmd->pData, SEC_KEY_LEN);
    }
    else
    {
      zcl_memset(&payload->gpdKey, 0xFF, SEC_KEY_LEN);
    }

    if(GP_PAIRING_ALIAS(payload->options))
    {
      gp_PopulateField((uint8_t* )&payload->assignedAlias, &pCmd->pData, sizeof( payload->assignedAlias));
    }
    else
    {
      payload->assignedAlias = 0xFFFF;
    }
    if (GP_FORWARDING_RADIUS(payload->options))
    {
      gp_PopulateField((uint8_t*)&payload->forwardingRadius, &pCmd->pData, sizeof(payload->forwardingRadius));
    }
    else
    {
      payload->forwardingRadius = 0x1E; // ZUTH Expects this by deault, but can't find in spec
    }
  }
  else
  {
    // Ivalidate every field
    payload->deviceId = 0xFF;
    payload->gpdSecCounter = 0xFFFFFFFF;
    payload->assignedAlias = 0xFFFF;
    payload->forwardingRadius = 0x1E; // ZUTH Expects this by deault, but can't find in spec
  }
}

 /*********************************************************************
 * @fn          gp_ZclProxyTableReqParse
 *
 * @brief       Parse the proxy table request data message payload
 *
 * @param
 *
 * @return
 */
static void gp_ZclProxyTableReqParse( zclGpTableRequest_t* pCmd, gpTableReqCmd_t* payload )
{
  payload->options =  pCmd->options;

  // If Request type bitfield is 0b00
  if(GP_PAIRING_OPT_IS_REQ_TYPE_GPD(payload->options))
  {
    // If Application Id bitfield is 0b000
    if(GP_PAIRING_OPT_IS_APPLICATION_ID_GPD( payload->options))
    {
      // Populate GPD ID
      gp_PopulateField((uint8_t* )&payload->gpdId, &pCmd->pData, sizeof(payload->gpdId));

      // Populate GPD IEEE Invalid
      zcl_memset (&payload->gpdIEEE, 0xFF, Z_EXTADDR_LEN);
      // Populate EP Invalid
      payload->ep = 0xFF;
    }
    // If Application Id bitfield is 0b010
    else if(GP_PAIRING_OPT_IS_APPLICATION_ID_IEEE(payload->options))
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
  else if(GP_PAIRING_OPT_IS_REQ_TYPE_INDEX(payload->options))
  {
    // Populate GPD ID Invalid
    payload->gpdId = 0xFFFFFFFF;

    // Populate GPD IEEE Invalid
    zcl_memset(&payload->gpdIEEE, 0xFF, Z_EXTADDR_LEN);

    // Populate EP Invalid
    payload->ep = 0xFF;

    // Populate index
    gp_PopulateField((uint8_t*)&payload->index, &pCmd->pData, sizeof(payload->index));
  }
}

#endif
/*********************************************************************
*********************************************************************/
