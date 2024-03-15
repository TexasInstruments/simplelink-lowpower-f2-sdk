/**************************************************************************************************
  Filename:       gp_sink_table.c
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
#include "zd_object.h"
#include "nwk_util.h"
#include "zcomdef.h"
#include "cgp_stub.h"
#include "gp_common.h"
#include "gp_sink.h"
#include "gp_interface.h"
#include "zcl_port.h"
#include "zcl.h"

#if (defined (USE_ICALL) || defined (OSAL_PORT2TIRTOS))
#include "zstackapi.h"
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

 /*********************************************************************
 * EXTERNAL VARIABLES
 */
extern uint8_t gpAppEntity;

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */

 /*********************************************************************
 * LOCAL FUNCTIONS
 */
static uint16_t gp_UpdateSinkTbl( uint8_t* pEntry, uint8_t actions );
static uint8_t pt_addSinkGroup( uint8_t* pNew, uint8_t* pCurr );
static uint8_t pt_getSecurity( uint8_t* pNew );
static uint8_t pt_getAlias( uint8_t* pNew );
static uint8_t pt_getSecFrameCounterCapabilities( uint8_t* pNew );

/*********************************************************************
 * PUBLIC FUNCTIONS
 *********************************************************************/

/*********************************************************************
 * @fn          pt_ZclReadGetSinkEntry
 *
 * @brief       Populate the given item data
 *
 * @param       nvId - NV entry to read
 *              pData - Pointer to OTA message payload
 *              len - Lenght of the payload
 *
 * @return
 */
uint8_t pt_ZclReadGetSinkEntry( uint16_t nvId, uint8_t* pData, uint8_t* len )
{
  uint8_t  currEntry[SINK_TBL_ENTRY_LEN];
  uint8_t  gpdEntry[SINK_TBL_ENTRY_LEN];
  uint8_t *pEntry;
  uint8_t  status;
  uint8_t  msgLen;
  uint16_t options;

  status = gp_getSinkTableByIndex( nvId, currEntry );
  pEntry = gpdEntry;

  // if FAIL or Empty
  if ( ( status == NV_OPER_FAILED ) || ( status == NV_INVALID_DATA ) )
  {
    // FAIL
    return status;
  }
  pEntry = zcl_memcpy( pEntry, &currEntry[SINK_TBL_OPT], sizeof( uint16_t ) );
  zcl_memcpy( &options, &currEntry[SINK_TBL_OPT], sizeof( uint16_t ) );
  msgLen = sizeof( uint16_t );

  // Options bitfield
  // If Application Id bitfield is 0b000
  if( GP_PAIRING_OPT_IS_APPLICATION_ID_GPD( options ) )
  {
    // Populate GPD ID
    pEntry = zcl_memcpy( pEntry, &currEntry[SINK_TBL_SRC_ID], sizeof( uint32_t ) );
    msgLen += sizeof( uint32_t );
  }
  // If Application Id bitfield is 0b010
  else if( GP_PAIRING_OPT_IS_APPLICATION_ID_IEEE( options ) )
  {
    pEntry = zcl_memcpy( pEntry, &currEntry[SINK_TBL_GPD_ID], Z_EXTADDR_LEN );
    pEntry = zcl_memcpy( pEntry, &currEntry[SINK_TBL_GPD_EP], sizeof( uint8_t ) );
    msgLen += Z_EXTADDR_LEN + sizeof( uint8_t );
  }

  pEntry = zcl_memcpy( pEntry, &currEntry[SINK_TBL_DEVICE_ID], sizeof( uint8_t ) );
  msgLen += sizeof( uint8_t );

  if ( SINK_TBL_GET_COMMUNICATION_MODE( currEntry[SINK_TBL_OPT] ) == GP_OPT_COMMUNICATION_MODE_GRPCAST_GROUP_ID )
  {
    uint8_t groupCount;

    uint8_t groupCountByte;
    msgLen++;

    pEntry = zcl_memcpy( pEntry, &currEntry[SINK_GROUP_COUNT], sizeof( uint8_t ) );
    msgLen += sizeof( uint8_t );

    for(groupCount = 0; groupCount < 2; groupCount++)
    {
      groupCountByte = SINK_TBL_1ST_GRP_ADDR + (groupCount * SINK_TBL_GRP_ADDR_LEN);
      pEntry = zcl_memcpy( pEntry, &currEntry[groupCountByte], SINK_TBL_GRP_ADDR_LEN );
      msgLen += groupCountByte;
    }
  }

  if ( SINK_TBL_GET_ASSIGNED_ALIAS( currEntry[SINK_TBL_OPT + 1] ) )
  {
    pEntry = zcl_memcpy( pEntry, &currEntry[SINK_TBL_ALIAS], sizeof( uint16_t ) );
    msgLen += sizeof( uint16_t );
  }

  pEntry = zcl_memcpy( pEntry, &currEntry[SINK_TBL_RADIUS], sizeof( uint8_t ) );
  msgLen += sizeof( uint8_t );

  if ( SINK_TBL_GET_SEC_USE( currEntry[PROXY_TBL_OPT + 1] ) )
  {
    pEntry = zcl_memcpy( pEntry, &currEntry[SINK_TBL_SEC_OPT], sizeof( uint8_t ) );
    pEntry = zcl_memcpy( pEntry, &currEntry[SINK_TBL_SEC_FRAME], sizeof( uint32_t ) );
    pEntry = zcl_memcpy( pEntry, &currEntry[SINK_TBL_GPD_KEY], SEC_KEY_LEN );
    msgLen += sizeof( uint8_t ) + sizeof( uint32_t ) + SEC_KEY_LEN;
  }
  else if ( ( SINK_TBL_GET_MAC_SEQ_CAP( currEntry[PROXY_TBL_OPT] ) ) )
  {
    pEntry = zcl_memcpy( pEntry, &currEntry[SINK_TBL_SEC_FRAME], sizeof( uint32_t ) );
    msgLen += sizeof( uint32_t );
  }

  if ( pData != NULL )
  {
    uint8_t sinkEntryLen;
    uint16_t totalLen;

    sinkEntryLen = msgLen;
    if ( *len != 0 )
    {
      msgLen -= sizeof( uint16_t );
    }
    totalLen = msgLen;
    totalLen += *len;
    zcl_memcpy( pData, &totalLen, sizeof( uint16_t ) );
    if ( *len == 0 )
    {
      pData += sizeof( uint16_t );
    }
    msgLen += sizeof( uint16_t );

    pData += *len;
    if(sinkEntryLen <= SINK_TBL_ENTRY_LEN)
    {
      zcl_memcpy( pData, gpdEntry, sinkEntryLen );
    }
    else
    {
      return FAILURE;
    }
    if ( *len != 0 )
    {
      *len = totalLen;
      return status;
    }
    *len += msgLen;
    return status;
  }
  else
  {
    if ( *len == 0 )
    {

        msgLen += sizeof( uint16_t );
    }
    *len += msgLen;
    return status;
  }
}

/*********************************************************************
 * @fn          gp_autoCommissioningGpdfSinkTblUpdate
 *
 * @brief       Populate the given item data with Auto-Commissioning GPDF
 *
 * @param       pEntry - Pointer to Sink entry
 *              nvIndex - NV index for the Sink entry
 *
 * @return
 */
ZStatus_t gp_autoCommissioningGpdfSinkTblUpdate(gpdID_t *gpdId, gp_DataInd_t *gpDataInd, uint8_t *pEntry, uint16_t nvIndex)
{
    gpSinkTableOptions_t sinkOptions;
    uint8_t cmdId;
    uint8_t deviceId = 0x00;
    uint8_t gpCommunicationMode;

    // Create a new entry
    cmdId = *(gpDataInd->GPDasdu);
    if( (cmdId == 0x20) || (cmdId == 0x21) || (cmdId == 0x22) )
    {
      deviceId = 0x02;
    }

    // Sink Options
    sinkOptions.appId = gpdId->appID;
    zcl_ReadAttrData(GREEN_POWER_INTERNAL_ENDPOINT, ZCL_CLUSTER_ID_GREEN_POWER, ATTRID_GP_GPS_COMMUNICATION_MODE,
                     &gpCommunicationMode, NULL);
    sinkOptions.communicationMode = gpCommunicationMode;
    sinkOptions.sequenceNumberCap = FALSE;
    sinkOptions.rxOnCapability = FALSE;
    sinkOptions.gpdFixed = FALSE;
    sinkOptions.assignedAlias = FALSE;
    sinkOptions.securityUse = FALSE;

    return gp_commissioningSinkTblUpdate(gpdId, gpDataInd->EndPoint, deviceId, pEntry, nvIndex, sinkOptions, NULL);
}

/*********************************************************************
 * @fn          gp_commissioningGpdfSinkTblUpdate
 *
 * @brief       Populate the given item data with commissioning GPDF
 *
 * @param       pEntry - Pointer to Sink entry
 *              nvIndex - NV index for the Sink entry
 *
 * @return
 */
ZStatus_t gp_commissioningGpdfSinkTblUpdate(gpdID_t *gpdId, gp_DataInd_t *gpDataInd, uint8_t *pEntry, uint16_t nvIndex)
{
    gpdCommissioningCmd_t commissioningCmdPayload;
    gpSinkTableOptions_t sinkOptions;
    uint8_t gpCommunicationMode;

    // Create a new entry
    gp_parseCommisioningCmd(gpDataInd->GPDasdu, &commissioningCmdPayload);

    // Sink Options
    sinkOptions.appId = gpdId->appID;
    zcl_ReadAttrData(GREEN_POWER_INTERNAL_ENDPOINT, ZCL_CLUSTER_ID_GREEN_POWER, ATTRID_GP_GPS_COMMUNICATION_MODE,
                     &gpCommunicationMode, NULL);
    sinkOptions.communicationMode = gpCommunicationMode;
    sinkOptions.sequenceNumberCap = commissioningCmdPayload.options.sequenceNumberCap;
    sinkOptions.rxOnCapability = commissioningCmdPayload.options.rxOnCap;
    sinkOptions.gpdFixed = commissioningCmdPayload.options.fixedLocation;
    sinkOptions.assignedAlias = FALSE;
    if( (commissioningCmdPayload.options.extendedOptionsField == TRUE) && (commissioningCmdPayload.extOptions.securityLevel >= GP_SECURITY_LVL_4FC_4MIC) )
    {
      sinkOptions.securityUse = TRUE;
    }
    else
    {
      sinkOptions.securityUse = FALSE;
    }

    //If GPD key requires encryption or decryption
    if( commissioningCmdPayload.extOptions.keyEncryption == TRUE )
    {
        zstack_gpEncryptDecryptCommissioningKeyRsp_t keyRsp;

        zcl_memset(&keyRsp, 0x00, sizeof(zstack_gpEncryptDecryptCommissioningKeyRsp_t));
        gp_processCommissioningKey(gpdId, &commissioningCmdPayload, &keyRsp);

        //Key was delivered encrypted on commissioning command
        if( commissioningCmdPayload.extOptions.gpdKeyPresent == TRUE )
        {
            // Save decrypted network key to sink table entry
            zcl_memcpy( commissioningCmdPayload.gpdKey, keyRsp.key, SEC_KEY_LEN );
        }
    }
    return gp_commissioningSinkTblUpdate(gpdId, gpDataInd->EndPoint, commissioningCmdPayload.deviceId, pEntry, nvIndex, sinkOptions, &commissioningCmdPayload);
}

/*********************************************************************
 * @fn          gp_PairingConfigUpdateSinkTbl
 *
 * @brief       General function fill the sink table vector
 *
 * @param
 *
 * @return     uint16_t - NV index of updated sink entry
 */
uint16_t gp_PairingConfigUpdateSinkTbl( gpPairingConfigCmd_t* payload )
{
  uint8_t  sinkEntry[SINK_TBL_ENTRY_LEN];

  zcl_memcpy(&sinkEntry[SINK_TBL_OPT], (uint8_t*)&payload->options, sizeof(payload->options));

  // Options bitfield
  // If Application Id bitfield is 0b000
  if(GP_PAIRING_CONFIG_OPT_IS_APPLICATION_ID_GPD(payload->options))
  {
    // Populate GPD ID
    zcl_memcpy(&sinkEntry[SINK_TBL_GPD_ID + 4], (uint8_t*)&payload->gpdId, sizeof(payload->gpdId));

  }
  // If Application Id bitfield is 0b010
  else if( GP_PAIRING_CONFIG_OPT_IS_APPLICATION_ID_IEEE(payload->options))
  {
    zcl_memcpy(&sinkEntry[SINK_TBL_GPD_ID], (uint8_t*)&payload->gpdIEEE, Z_EXTADDR_LEN);
  }

  zcl_memcpy(&sinkEntry[SINK_TBL_GPD_EP], (uint8_t*)&payload->ep, sizeof(payload->ep));
  zcl_memcpy(&sinkEntry[SINK_TBL_DEVICE_ID], (uint8_t*)&payload->deviceId, sizeof(payload->deviceId));

  if(GP_PAIRING_CONFIG_IS_GRPCAST_GROUP( payload->options))
  {
    zcl_memcpy(&sinkEntry[SINK_TBL_1ST_GRP_ADDR], (uint8_t*)&payload->deviceId, sizeof(payload->groupEntriesCount * SINK_TBL_GRP_ADDR_LEN));
  }

  if(GP_PAIRING_CONFIG_ALIAS( payload->options))
  {
    zcl_memcpy(&sinkEntry[SINK_TBL_ALIAS], (uint8_t*)&payload->gpdAssignedAlias, sizeof(payload->gpdAssignedAlias));
  }

  zcl_memcpy(&sinkEntry[SINK_TBL_RADIUS], (uint8_t*)&payload->forwardingRadius, sizeof(payload->forwardingRadius));

  if(GP_PAIRING_CONFIG_SECURITY_USE(payload->options))
  {
    zcl_memcpy(&sinkEntry[SINK_TBL_SEC_OPT], (uint8_t*)&payload->securityOptions, sizeof(payload->securityOptions));
    zcl_memcpy(&sinkEntry[SINK_TBL_SEC_FRAME], (uint8_t*)&payload->gpdSecurityFrameCounter, sizeof(payload->gpdSecurityFrameCounter));
    zcl_memcpy(&sinkEntry[SINK_TBL_GPD_KEY], (uint8_t*)&payload->gpdSecurityKey, sizeof(payload->gpdSecurityKey));
  }
  else
  {
    uint8_t securityOptions = 0;
    zcl_memcpy(&sinkEntry[SINK_TBL_SEC_OPT], &securityOptions, sizeof(securityOptions));
  }

  if( GP_GET_GPD_SEQ_CAP_BIT(payload->options))
  {
    zcl_memcpy(&sinkEntry[SINK_TBL_SEC_FRAME], (uint8_t*)&payload->gpdSecurityFrameCounter, sizeof(payload->gpdSecurityFrameCounter));
  }

  // Update the sink table
  return gp_UpdateSinkTbl(sinkEntry, payload->actions);
}

/*********************************************************************
 * @fn          gp_commissioningSinkTblUpdate
 *
 * @brief       Create new entry from commissioning GPDF
 *
 *
 * @return      ZStatus_t
 */
ZStatus_t gp_commissioningSinkTblUpdate(gpdID_t *gpdId, uint8_t ep, uint8_t deviceId, uint8_t *pEntry, uint16_t nvIndex, gpSinkTableOptions_t sinkOptions, gpdCommissioningCmd_t *pCommissioningCmd)
{
    gp_commissioningSinkEntryParse(gpdId, ep, deviceId, pEntry, sinkOptions, pCommissioningCmd);
    return zclport_writeNV( ZCL_PORT_SINK_TABLE_NV_ID, nvIndex,
                            SINK_TBL_ENTRY_LEN, pEntry );
}

/*********************************************************************
 * @fn          gp_commissioningSinkEntryParse
 *
 * @brief       Fill a sink table entry from a commissioning command
 *
 *
 * @return      ZStatus_t
 */
void gp_commissioningSinkEntryParse(gpdID_t *gpdId, uint8_t ep, uint8_t deviceId, uint8_t *pEntry, gpSinkTableOptions_t sinkOptions, gpdCommissioningCmd_t *pCommissioningCmd)
{
    zcl_memcpy( &pEntry[SINK_TBL_OPT], ( uint8_t* )&sinkOptions, sizeof( uint16_t ) );

    // If Application Id bitfield is 0b000
    if( gpdId->appID == GP_OPT_APP_ID_GPD )
    {
      zcl_memcpy( &pEntry[SINK_TBL_SRC_ID], ( uint8_t* )&gpdId->id.srcID, sizeof( uint32_t ) );
      zcl_memcpy( &pEntry[SINK_TBL_ALIAS], ( uint8_t* )&gpdId->id.srcID, sizeof( uint16_t ) );
    }
    // If Application Id bitfield is 0b010
    else if( gpdId->appID == GP_OPT_APP_ID_IEEE )
    {
      zcl_memcpy( &pEntry[SINK_TBL_GPD_ID], gpdId->id.gpdExtAddr, Z_EXTADDR_LEN );
      zcl_memcpy( &pEntry[SINK_TBL_ALIAS], gpdId->id.gpdExtAddr, sizeof( uint16_t ) );
    }

    pEntry[SINK_TBL_GPD_EP] = ep;

    pEntry[SINK_TBL_DEVICE_ID] = deviceId;

    if(sinkOptions.communicationMode == GP_OPT_COMMUNICATION_MODE_GRPCAST_DGROUP_ID)
    {
       aps_Group_t grp;

       pEntry[SINK_GROUP_COUNT] = 1;
       if(gpdId->appID == GP_OPT_APP_ID_GPD)
       {
         zcl_memcpy( &grp.ID, &gpdId->id.srcID, sizeof( uint16_t ) );
         zcl_memcpy( &pEntry[SINK_TBL_1ST_GRP_ADDR], ( uint8_t* )&gpdId->id.srcID, sizeof(uint16_t) );
       }
       else if(gpdId->appID == GP_OPT_APP_ID_IEEE)
       {
         zcl_memcpy( &grp.ID, gpdId->id.gpdExtAddr, sizeof( uint16_t ) );
         zcl_memcpy( &pEntry[SINK_TBL_1ST_GRP_ADDR], gpdId->id.gpdExtAddr, sizeof(uint16_t) );
       }
#if defined (ZCL_GROUPS)
       zclport_aps_AddGroup(GREEN_POWER_INTERNAL_ENDPOINT, &grp );
#endif
    }
    else if( sinkOptions.communicationMode == GP_OPT_COMMUNICATION_MODE_GRPCAST_GROUP_ID )
    {
#if defined (ZCL_GROUPS)
      uint16_t *pGroupList = NULL;
      //uint8_t   numGroups;

      pGroupList = zcl_mem_alloc(APS_MAX_GROUPS * 2);
      //numGroups = zclport_aps_FindAllGroupsForEndpoint(GREEN_POWER_INTERNAL_ENDPOINT, pGroupList);

      if(pGroupList != NULL)
      {
        zcl_memcpy( &pEntry[SINK_TBL_1ST_GRP_ADDR], pGroupList, sizeof(uint16_t) );
      }
      else
      {
        // TODO: handle error. add return status to function?
      }
      if( gpdId->appID == GP_OPT_APP_ID_GPD )
      {
        zcl_memcpy( &pEntry[SINK_TBL_1ST_GRP_ADDR +2], ( uint8_t* )&gpdId->id.srcID, sizeof(uint16_t) );
      }
      else if( gpdId->appID == GP_OPT_APP_ID_IEEE )
      {
        zcl_memcpy( &pEntry[SINK_TBL_1ST_GRP_ADDR +2], gpdId->id.gpdExtAddr, sizeof(uint16_t) );
      }
#endif
    }

    pEntry[SINK_TBL_RADIUS] =  GPS_RADIUS;
    if( (sinkOptions.securityUse == TRUE) && (pCommissioningCmd != NULL) )
    {
        pEntry[SINK_TBL_SEC_OPT] = ( pCommissioningCmd->extOptions.securityLevel | ( pCommissioningCmd->extOptions.keyType << 2) ) & 0x1F;
        zcl_memcpy( &pEntry[SINK_TBL_SEC_FRAME], ( uint8_t* )&pCommissioningCmd->gpdOutCounter, sizeof( uint32_t ) );

        if( pCommissioningCmd->extOptions.gpdKeyPresent == TRUE )
        {
            //Copy received key, that can be encrypted
            zcl_memcpy( &pEntry[SINK_TBL_GPD_KEY], pCommissioningCmd->gpdKey, SEC_KEY_LEN );
        }
        else  // Key is requested from GPD
        {
            uint8_t gpSharedKeyType;
            uint8_t gpSharedKey[SEC_KEY_LEN];
            uint16_t gpSharedKeyTypeLen;
            uint16_t gpSharedKeyLen;

            //Read the key
            zcl_ReadAttrData(GREEN_POWER_INTERNAL_ENDPOINT, ZCL_CLUSTER_ID_GREEN_POWER, ATTRID_GP_SHARED_SEC_KEY_TYPE,
                             &gpSharedKeyType, &gpSharedKeyTypeLen);
            zcl_ReadAttrData(GREEN_POWER_INTERNAL_ENDPOINT, ZCL_CLUSTER_ID_GREEN_POWER, ATTRID_GP_SHARED_SEC_KEY,
                             gpSharedKey, &gpSharedKeyLen);

            // Save network key to sink table entry
            zcl_memcpy( &pEntry[SINK_TBL_GPD_KEY], gpSharedKey, SEC_KEY_LEN );
        }
    }
    else if( (sinkOptions.sequenceNumberCap == TRUE) && (pCommissioningCmd != NULL) )
    {
        zcl_memcpy( &pEntry[SINK_TBL_SEC_FRAME], ( uint8_t* )&pCommissioningCmd->gpdOutCounter, GP_SECURITY_MIC_SIZE );
    }
    else if( sinkOptions.securityUse == FALSE )
    {
        pEntry[SINK_TBL_SEC_OPT] = 0x00;
    }
}

 /*********************************************************************
 * PRIVATE FUNCTIONS
 *********************************************************************/

/*********************************************************************
 * @fn          gp_UpdateSinkTbl
 *
 * @brief       To update the sink table NV vectors
 *
 * @param       pNpEntryew - New entry array to be added
 *
 * @return      uint16_t - NV sink table entry index
 */

static uint16_t gp_UpdateSinkTbl( uint8_t* pEntry, uint8_t actions )
{
  uint8_t i;
  uint8_t newEntry[SINK_TBL_ENTRY_LEN];
  uint8_t currEntry[SINK_TBL_ENTRY_LEN];
  uint16_t sinkTableIndex;
  uint8_t status;

  // Copy the new entry pointer to array
  sinkTableCpy(newEntry, pEntry);

  for (i = 0; i < GPS_MAX_SINK_TABLE_ENTRIES ; i++)
  {
    sinkTableIndex = i;
    status = gp_getSinkTableByIndex(sinkTableIndex, currEntry);

    if (status == NV_OPER_FAILED)
    {
      // FAIL
      return ZCD_NV_INVALID_INDEX;
    }

    // if the entry is empty
    if((status == NV_INVALID_DATA) &&
       (GP_PAIRING_CONFIG_ACTION_IS_EXTEND(actions) ||
        GP_PAIRING_CONFIG_ACTION_IS_REPLACE(actions)))
    {
      status = zclport_writeNV( ZCL_PORT_SINK_TABLE_NV_ID, sinkTableIndex,
                                SINK_TBL_ENTRY_LEN,
                                newEntry );
      return sinkTableIndex;
    }

    if ( gpLookForGpd( sinkTableIndex, newEntry ) )
    {
      // Entry found
      break;
    }

    if ( i >= GPS_MAX_SINK_TABLE_ENTRIES )
    {
      // No space for new entries
      return ZCD_NV_INVALID_INDEX;
    }
  }

  // Remove the entry
  if(GP_PAIRING_CONFIG_ACTION_IS_REMOVE_PAIRING(actions) ||
     GP_PAIRING_CONFIG_ACTION_IS_REMOVE_GPD(actions))
  {
      gp_ResetSinkTblEntry( currEntry );

    status = zclport_writeNV(ZCL_PORT_SINK_TABLE_NV_ID, sinkTableIndex,
                             SINK_TBL_ENTRY_LEN,
                             currEntry);
    return sinkTableIndex;
  }

  if(SINK_TBL_GET_COMMUNICATION_MODE( newEntry[SINK_TBL_OPT]) ==
     GP_OPT_COMMUNICATION_MODE_GRPCAST_GROUP_ID)
  {
    if((!pt_addSinkGroup( newEntry, currEntry)))
    {
      // The entry is full
      return ZCD_NV_INVALID_INDEX;
    }
  }

  if(pt_getAlias(newEntry))
  {
    zcl_memcpy(&currEntry[SINK_TBL_ALIAS], &newEntry[SINK_TBL_ALIAS], sizeof(uint16_t));
  }

  currEntry[SINK_TBL_RADIUS] = newEntry[SINK_TBL_RADIUS];
  if(pt_getSecurity( newEntry ) ||
     pt_getSecFrameCounterCapabilities(newEntry))
  {
    zcl_memcpy(&currEntry[SINK_TBL_SEC_FRAME], &newEntry[SINK_TBL_SEC_FRAME], sizeof(uint32_t));
  }

  status = zclport_writeNV(ZCL_PORT_SINK_TABLE_NV_ID, sinkTableIndex,
                           PROXY_TBL_LEN,
                           currEntry);

  return sinkTableIndex;
}

/*********************************************************************
 * @fn          pt_addSinkGroup
 *
 * @brief       To add new sink group entry
 *
 * @param       pNew - New entry array to be added
 *              pCurr - The current entry array in the sink table
 *
 * @return      TRUE new entry added, FALSE if not
 */
static uint8_t pt_addSinkGroup( uint8_t* pNew, uint8_t* pCurr )
{
  uint8_t newEntry[SINK_TBL_ENTRY_LEN];
  uint8_t currEntry[SINK_TBL_ENTRY_LEN];

  proxyTableCpy( &newEntry, pNew );
  proxyTableCpy( &currEntry, pCurr );


  if(zcl_memcmp(&currEntry[SINK_TBL_1ST_GRP_ADDR], &newEntry[SINK_TBL_1ST_GRP_ADDR], sizeof(uint32_t)))
  {
    zcl_memcpy(&currEntry[SINK_TBL_1ST_GRP_ADDR], &newEntry[SINK_TBL_1ST_GRP_ADDR], sizeof(uint32_t));
    proxyTableCpy(pCurr, &currEntry);
    return TRUE;
  }
  else if(!zcl_memcmp(&currEntry[SINK_TBL_2ND_GRP_ADDR], &newEntry[SINK_TBL_1ST_GRP_ADDR], sizeof(uint32_t)))
  {
    zcl_memcpy(&currEntry[SINK_TBL_2ND_GRP_ADDR], &newEntry[SINK_TBL_1ST_GRP_ADDR], sizeof(uint32_t));
    proxyTableCpy(pCurr, &currEntry);
    return TRUE;
  }
  return FALSE;
}

/*********************************************************************
 * @fn          pt_getSecurity
 *
 * @brief       Get Bit to update security options field or not
 *
 * @param       pNew - New entry array to be added
 *
 * @return      secUse - TRUE if security use bit is set
 */
static uint8_t pt_getSecurity( uint8_t* pNew )
{
  bool secUse;

  secUse = SINK_TBL_GET_SEC_USE(pNew[SINK_TBL_OPT + 1]);

  return secUse;
}

/*********************************************************************
 * @fn          pt_getAlias
 *
 * @brief       Get Bit to update assigned alias field or not
 *
 * @param       pNew - New entry array to be added
 *
 * @return      newAlias - TRUE if assigned alias bit is set
 */
static uint8_t pt_getAlias( uint8_t* pNew )
{
  bool newAlias;

  newAlias = SINK_TBL_GET_ASSIGNED_ALIAS(pNew[SINK_TBL_OPT + 1]);

  return newAlias;
}

/*********************************************************************
 * @fn          pt_getSecFrameCounterCapabilities
 *
 * @brief       Get Bit to update security frame counter capabilities
 *
 * @param       pNew - New entry array to be added
 *
 * @return      secCap - TRUE if security frame counter capabilities bit is set
 */
static uint8_t pt_getSecFrameCounterCapabilities( uint8_t* pNew )
{
  bool secCap;

  secCap = SINK_TBL_GET_RX_ON_CAPABILITIES(pNew[SINK_TBL_OPT]);

  return secCap;
}

#endif
/*********************************************************************
*********************************************************************/

