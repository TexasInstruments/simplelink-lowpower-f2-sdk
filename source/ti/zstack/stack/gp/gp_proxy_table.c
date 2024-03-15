/**************************************************************************************************
  Filename:       gp_proxy_table.c
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
#include "gp_bit_fields.h"
#include "gp_common.h"
#include "gp_proxy.h"
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
ZDO_DeviceAnnce_t aliasConflictAnnce;

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
static uint8_t pt_getAlias( uint8_t* pNew );
static uint8_t pt_getSecurity( uint8_t* pNew );
static uint8_t pt_getSecFrameCounterCapabilities( uint8_t* pNew );
static uint8_t pt_updateLightweightUnicastSink( uint8_t* pNew, uint8_t* pCurr );
static uint8_t pt_removeLightweightUnicastSink( uint8_t* pEntry, uint8_t* pAddr );
static uint8_t pt_addProxyGroup( uint8_t* pNew, uint8_t* pCurr );
static uint8_t pt_removeProxyGroup( uint8_t* pEntry, uint16_t groupAddr );
static uint16_t gp_pairingSetProxyTblOptions( uint32_t pairingOpt );

/*********************************************************************
 * PUBLIC FUNCTIONS
 *********************************************************************/

/*********************************************************************
 * @fn          pt_ZclReadGetProxyEntry
 *
 * @brief       Populate the given item data
 *
 * @param       nvId - NV entry to read
 *              pData - Pointer to OTA message payload
 *              len - Lenght of the payload
 *
 * @return
 */
uint8_t pt_ZclReadGetProxyEntry( uint16_t nvId, uint8_t* pData, uint8_t* len )
{
  uint8_t  freeSinkEntry[LSINK_ADDR_LEN] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  uint8_t  currEntry[PROXY_TBL_LEN];
  uint8_t  gpdEntry[PROXY_TBL_LEN];
  uint8_t *pEntry;
  uint8_t  msgLen = 0;
  uint8_t  status;
  uint16_t options;

  status = gp_getProxyTableByIndex( nvId, currEntry );
  pEntry = gpdEntry;

  // if FAIL or Empty
  if((status == NV_OPER_FAILED) || (status == NV_INVALID_DATA))
  {
    // if empty and nvID 0, then poxxy table is empty.
    if((status == NV_INVALID_DATA) && (nvId == 0)){
      pData = NULL;
    }
    // FAIL
    return status;
  }

  pEntry = zcl_memcpy(pEntry, &currEntry[PROXY_TBL_OPT], sizeof(uint16_t));
  zcl_memcpy( &options, &currEntry[PROXY_TBL_OPT], sizeof(uint16_t));
  msgLen += sizeof(uint16_t);

  // Options bitfield
  // If Application Id bitfield is 0b000
  if(GP_PAIRING_OPT_IS_APPLICATION_ID_GPD(options))
  {
    // Populate GPD ID
      pEntry = zcl_memcpy(pEntry, &currEntry[GP_TBL_SRC_ID], sizeof(uint32_t));
      msgLen += sizeof(uint32_t);

  }
  // If Application Id bitfield is 0b010
  else if(GP_PAIRING_OPT_IS_APPLICATION_ID_IEEE(options))
  {
      pEntry = zcl_memcpy(pEntry, &currEntry[PROXY_TBL_GPD_ID], Z_EXTADDR_LEN);
      pEntry = zcl_memcpy(pEntry, &currEntry[PROXY_TBL_GPD_EP], sizeof(uint8_t));
      msgLen += Z_EXTADDR_LEN + sizeof(uint8_t);
  }

  if(PROXY_TBL_GET_ASSIGNED_ALIAS(currEntry[PROXY_TBL_OPT + 1]))
  {
      pEntry = zcl_memcpy(pEntry, &currEntry[PROXY_TBL_ALIAS], sizeof(uint16_t));
      msgLen += sizeof(uint16_t);
  }

  if(PROXY_TBL_GET_SEC_USE(currEntry[PROXY_TBL_OPT + 1]))
  {
      pEntry = zcl_memcpy(pEntry, &currEntry[PROXY_TBL_SEC_OPT], sizeof(uint8_t));
      msgLen += sizeof(uint8_t);
  }

  if((PROXY_TBL_GET_SEC_USE(currEntry[PROXY_TBL_OPT + 1]) ||
     (PROXY_TBL_GET_SEC_CAP( currEntry[PROXY_TBL_OPT]))))
  {
    pEntry = zcl_memcpy(pEntry, &currEntry[PROXY_TBL_SEC_FRAME], sizeof(uint32_t));
    msgLen += sizeof(uint32_t);
  }

  if(PROXY_TBL_GET_SEC_USE(currEntry[PROXY_TBL_OPT + 1]))
  {
      pEntry = zcl_memcpy(pEntry, &currEntry[PROXY_TBL_GPD_KEY], SEC_KEY_LEN);
      msgLen += SEC_KEY_LEN;
  }

  if(PROXY_TBL_GET_LIGHTWIGHT_UNICAST(currEntry[PROXY_TBL_OPT]))
  {
    uint8_t sinkCount = 0;
    uint8_t sinkCountByte = msgLen;

    msgLen += sizeof(sinkCountByte);

    if(!zcl_memcmp(freeSinkEntry, &currEntry[PROXY_TBL_1ST_LSINK_ADDR], LSINK_ADDR_LEN))
    {
      sinkCount += 1;
      pEntry = zcl_memcpy(&gpdEntry[sinkCountByte], &sinkCount, sizeof(uint8_t));
      pEntry = zcl_memcpy(pEntry, &currEntry[PROXY_TBL_1ST_LSINK_ADDR], LSINK_ADDR_LEN);
      msgLen += LSINK_ADDR_LEN;
    }
    if(!zcl_memcmp(freeSinkEntry, &currEntry[PROXY_TBL_2ND_LSINK_ADDR], LSINK_ADDR_LEN))
    {
      sinkCount += 1;
      zcl_memcpy(&gpdEntry[sinkCountByte], &sinkCount, sizeof(uint8_t));
      pEntry = zcl_memcpy(pEntry, &currEntry[PROXY_TBL_2ND_LSINK_ADDR], LSINK_ADDR_LEN);
      msgLen += LSINK_ADDR_LEN;
    }
  }

  if(PROXY_TBL_GET_CGROUP(currEntry[PROXY_TBL_OPT + 1]))
  {
    uint8_t groupCount = 0;
    uint8_t groupCountByte = msgLen;

    msgLen++;
    if(GET_BIT(&currEntry[PROXY_TBL_GRP_TBL_ENTRIES], PROXY_TBL_1ST_GRP_BIT))
    {
      groupCount += 1;
      pEntry = zcl_memcpy(&gpdEntry[groupCountByte], &groupCount, sizeof(uint8_t));
      pEntry = zcl_memcpy(pEntry, &currEntry[PROXY_TBL_1ST_GRP_ADDR], sizeof(uint32_t));
      msgLen += sizeof(uint32_t);
    }
    if(GET_BIT(&currEntry[PROXY_TBL_GRP_TBL_ENTRIES], PROXY_TBL_2ND_GRP_BIT))
    {
      groupCount += 1;
      zcl_memcpy(&gpdEntry[groupCountByte], &groupCount, sizeof(uint8_t));
      pEntry = zcl_memcpy(pEntry, &currEntry[PROXY_TBL_2ND_GRP_ADDR], sizeof(uint32_t));
      msgLen += sizeof(uint32_t);
    }
  }

  pEntry = zcl_memcpy(pEntry, &currEntry[PROXY_TBL_RADIUS], sizeof(uint8_t));
  msgLen += sizeof(uint8_t);

  if((PROXY_TBL_GET_ENTRY_ACTIVE(currEntry[PROXY_TBL_OPT] ) == 0) ||
     (PROXY_TBL_GET_ENTRY_VALID(currEntry[PROXY_TBL_OPT]) == 0))
  {
      pEntry = zcl_memcpy(pEntry, &currEntry[PROXY_TBL_SEARCH_COUNTER], sizeof(uint8_t));
      msgLen += sizeof(uint8_t);
  }

  if (pData != NULL)
  {
    uint8_t sinkEntryLen;
    uint16_t totalLen;

    sinkEntryLen = msgLen;
    if (*len != 0)
    {
      msgLen -= sizeof(uint16_t);
    }
    totalLen = msgLen;
    totalLen += *len;
    zcl_memcpy(pData, &totalLen, sizeof(uint16_t));
    if(*len == 0 )
    {
      pData += sizeof(uint16_t);
    }
    msgLen += sizeof(uint16_t);

    pData += *len;
    if(sinkEntryLen <= PROXY_TBL_LEN)
    {
      zcl_memcpy(pData, gpdEntry, sinkEntryLen);
    }
    else
    {
      return FAILURE;
    }

    if(*len != 0)
    {
      *len = totalLen;
      return status;
    }
    *len += msgLen;
    return status;
  }
  else
  {
    if(*len == 0)
    {
        msgLen += sizeof(uint16_t);
    }
    *len += msgLen;
    return status;
  }
}

/*********************************************************************
 * @fn          gp_UpdateProxyTbl
 *
 * @brief       To update the proxy table NV vectors
 *
 * @param       pEntry - New entry to be added
 *              options - pairing options bitfield
 *              conflictResolution - Conflict resolution flag
 *
 * @return
 */
uint8_t gp_UpdateProxyTbl( uint8_t* pEntry, uint32_t options, uint8_t conflictResolution )
{
  uint8_t i;
  uint8_t newEntry[PROXY_TBL_LEN];
  uint8_t currEntry[PROXY_TBL_LEN];
  uint16_t proxyTableIndex;
  uint8_t status;

  // Copy the new entry pointer to array
  proxyTableCpy( &newEntry, pEntry );

  for(i = 0; i < GPP_MAX_PROXY_TABLE_ENTRIES ; i++)
  {
    proxyTableIndex = i;
    status = gp_getProxyTableByIndex(proxyTableIndex, currEntry);
    if(status == NV_OPER_FAILED)
    {
      // FAIL
      return status;
    }

    // if the entry is empty
    if((status == NV_INVALID_DATA) && (GP_PAIRING_OPT_ADD_SINK(options) == TRUE))
    {
      // Save new entry
      status = zclport_writeNV( ZCL_PORT_PROXY_TABLE_NV_ID, proxyTableIndex,
                                PROXY_TBL_LEN,
                                newEntry );

      // Perform address conflict resolution
      if(zcl_memcmp(&_NIB.nwkDevAddress, &newEntry[PROXY_TBL_ALIAS], sizeof(uint16_t))        ||
         zcl_memcmp(&_NIB.nwkDevAddress, &newEntry[PROXY_TBL_1ST_GRP_ADDR], sizeof(uint16_t)) ||
         zcl_memcmp(&_NIB.nwkDevAddress, &newEntry[PROXY_TBL_2ND_GRP_ADDR], sizeof(uint16_t))   )
      {
#if (defined (USE_ICALL) || defined (OSAL_PORT2TIRTOS))
        zstack_gpAddrConflict_t *pMsg;

        pMsg = (zstack_gpAddrConflict_t*)zcl_mem_alloc( sizeof(zstack_gpAddrConflict_t));
        if(pMsg != NULL)
        {
            zcl_memset(pMsg, 0, sizeof(zstack_gpAddrConflict_t));

            pMsg->conflictResolution = conflictResolution;

            Zstackapi_gpAliasConflict(gpAppEntity, pMsg);
            zcl_mem_free(pMsg);
        }
        else
        {
            return ZMemError;
        }
#else
        NLME_ReportAddressConflict(_NIB.nwkDevAddress, TRUE);
#endif
      }
      return status;
    }

    if(gpLookForGpd(proxyTableIndex, newEntry))
    {
      // Entry found
      break;
    }
  }

  if(i >= GPP_MAX_PROXY_TABLE_ENTRIES)
  {
    // No space for new entries
    return FAILURE;
  }

  // Remove the entry
  if(GP_PAIRING_OPT_ADD_SINK(options) == FALSE)
  {
    uint16_t addr;
    zcl_memcpy(&addr, &newEntry[PROXY_TBL_1ST_GRP_ADDR], sizeof(uint16_t));
    if((pt_removeProxyGroup(currEntry, addr) == TRUE &&
        pt_removeLightweightUnicastSink(currEntry, &newEntry[PROXY_TBL_1ST_LSINK_ADDR]) == TRUE))
    {
      gp_ResetProxyTblEntry(currEntry);
    }
    status = zclport_writeNV(ZCL_PORT_PROXY_TABLE_NV_ID, proxyTableIndex,
                             PROXY_TBL_LEN,
                             currEntry);
    return status;
  }

  if(PROXY_TBL_GET_LIGHTWIGHT_UNICAST(newEntry[PROXY_TBL_OPT]))
  {
    if((!pt_updateLightweightUnicastSink(newEntry, currEntry)))
    {
      // The entry is full
      return NV_BAD_ITEM_LEN;
    }
    PROXY_TBL_SET_LIGHTWIGHT_UNICAST(&currEntry[PROXY_TBL_OPT], options);
  }
  else if(PROXY_TBL_GET_CGROUP(newEntry[PROXY_TBL_OPT + 1]))
  {
    if((!pt_addProxyGroup(newEntry, currEntry)))
    {
      // The entry is full
      return NV_BAD_ITEM_LEN;
    }
    PROXY_TBL_SET_CGROUP(&currEntry[PROXY_TBL_OPT +1], options);
  }

  if(pt_getAlias(newEntry))
  {
    zcl_memcpy(&currEntry[PROXY_TBL_ALIAS], &newEntry[PROXY_TBL_ALIAS], sizeof(uint16_t));
  }
  if(pt_getSecurity(newEntry))
  {
    PROXY_TBL_SET_SEC_USE(&currEntry[PROXY_TBL_OPT + 1], TRUE);
    zcl_memcpy(&currEntry[PROXY_TBL_SEC_OPT], &newEntry[PROXY_TBL_SEC_OPT], sizeof(uint8_t));
    zcl_memcpy(&currEntry[PROXY_TBL_GPD_KEY], &newEntry[PROXY_TBL_GPD_KEY], SEC_KEY_LEN);
  }
  if(pt_getSecFrameCounterCapabilities(newEntry))
  {
    PROXY_TBL_SET_SEC_CAP(&currEntry[PROXY_TBL_OPT], TRUE);
  }

  zcl_memcpy(&currEntry[PROXY_TBL_SEC_FRAME], &newEntry[PROXY_TBL_SEC_FRAME], sizeof(uint32_t));
  currEntry[PROXY_TBL_RADIUS] = newEntry[PROXY_TBL_RADIUS];
  currEntry[PROXY_TBL_SEARCH_COUNTER] = newEntry[PROXY_TBL_SEARCH_COUNTER];
  status = zclport_writeNV(ZCL_PORT_PROXY_TABLE_NV_ID, proxyTableIndex,
                           PROXY_TBL_LEN,
                           currEntry);

  if (zcl_memcmp(&_NIB.nwkDevAddress, &currEntry[PROXY_TBL_ALIAS], sizeof(uint16_t))        ||
      zcl_memcmp(&_NIB.nwkDevAddress, &currEntry[PROXY_TBL_1ST_GRP_ADDR], sizeof(uint16_t)) ||
      zcl_memcmp(&_NIB.nwkDevAddress, &currEntry[PROXY_TBL_2ND_GRP_ADDR], sizeof(uint16_t))   )
  {
#if (defined (USE_ICALL) || defined (OSAL_PORT2TIRTOS))
      zstack_gpAddrConflict_t *pMsg;

      pMsg = (zstack_gpAddrConflict_t*)zcl_mem_alloc(sizeof(zstack_gpAddrConflict_t));
      if(pMsg != NULL)
      {
          zcl_memset(pMsg, 0, sizeof(zstack_gpAddrConflict_t));

          pMsg->conflictResolution = TRUE;

          Zstackapi_gpAliasConflict(gpAppEntity, pMsg);
          zcl_mem_free(pMsg);
      }
      else
      {
          return ZMemError;
      }
#else
    NLME_ReportAddressConflict(_NIB.nwkDevAddress, TRUE);
#endif
  }

  return status;
}

 /*********************************************************************
 * @fn          gp_pairingSetProxyTblOptions
 *
 * @brief       Function to fill the options pramenter in a Proxy Table entry
 *              from a GP Pairing Command
 *
 * @param       pBitField - pointer to the bit field.
 *              bit       - position of the bit to set in the given bitfield.
 *
 * @return
 */
static uint16_t gp_pairingSetProxyTblOptions ( uint32_t pairingOpt )
{
  uint8_t  proxyOpt[2] = {0x00, 0x00};
  uint16_t opt = 0;

  PAIRING_PROXY_TBL_SET_APPLICATION_ID((uint8_t*)&proxyOpt[0], pairingOpt);
  PROXY_TBL_SET_ENTRY_ACTIVE(&proxyOpt[0]);
  PROXY_TBL_SET_ENTRY_VALID(&proxyOpt[0]);
  PAIRING_PROXY_TBL_SET_MAC_SEQ_CAP((uint16_t*)&proxyOpt, pairingOpt);

  PROXY_TBL_SET_LIGHTWIGHT_UNICAST(&proxyOpt[0], pairingOpt);
  PROXY_TBL_SET_DGROUP(&proxyOpt[0], pairingOpt);
  PROXY_TBL_SET_CGROUP(&proxyOpt[1], pairingOpt);

  // FALSE by default
  PROXY_TBL_SET_FIRST_TO_FORWARD(&proxyOpt[1], FALSE);

  // The InRange sub-field, if set to 0b1, indicates that this GPD is in range
  // of this proxy. The default value is FALSE
  PROXY_TBL_SET_IN_RANGE(&proxyOpt[1], FALSE);
  // The GPDfixed sub-field, if set to 0b1, indicates portability capabilities
  PROXY_TBL_SET_GPD_FIXED(&proxyOpt[1], pairingOpt);
  PROXY_TBL_SET_HAS_ALL_ROUTES(&proxyOpt[1], pairingOpt);
  if(GP_PAIRING_IS_GRPCAST_CGROUP(pairingOpt) == FALSE)
  {
    PROXY_TBL_SET_ASSIGNED_ALIAS(&proxyOpt[1], GP_PAIRING_ALIAS(pairingOpt));
  }
  if(GP_GET_SEC_LEVEL(pairingOpt) >= GP_SECURITY_LVL_4FC_4MIC)
  {
    PROXY_TBL_SET_SEC_USE(&proxyOpt[1], TRUE);
  }
  else
  {
    PROXY_TBL_SET_SEC_USE(&proxyOpt[1], FALSE);
  }
  PROXY_TBL_SET_OPT_EXT(&proxyOpt[1], FALSE);

  opt |= ((proxyOpt[0]) & 0x00FF);
  opt |= (((proxyOpt[1]) << 8) & 0xFF00);

  return opt;
}

/*********************************************************************
 * @fn          gp_PairingUpdateProxyTbl
 *
 * @brief       General function fill the proxy table vector
 *
 * @param
 *
 * @return
 */
uint8_t gp_PairingUpdateProxyTbl( gpPairingCmd_t* payload )
{
  gpdID_t gpdID;
  uint8_t  proxyEntry[PROXY_TBL_LEN];
  uint8_t  newSinkGroup = 0;
  uint16_t entryOptions = 0;
  uint8_t  securityOpt = 0;
  uint8_t  searchCounter = 0x0A;
  uint16_t invalidAlias = 0xFFFF;

  entryOptions = gp_pairingSetProxyTblOptions(payload->options);
  zcl_memcpy(&proxyEntry[PROXY_TBL_OPT], (uint8_t*)&entryOptions, sizeof(uint16_t));

  // Options bitfield
  // If Application Id bitfield is 0b000
  if(GP_PAIRING_OPT_IS_APPLICATION_ID_GPD(payload->options))
  {
    // Populate GPD ID
    zcl_memcpy(&proxyEntry[GP_TBL_SRC_ID], (uint8_t*)&payload->gpdId, sizeof(payload->gpdId));
    gpdID.appID = GP_OPT_APP_ID_GPD;
    gpdID.id.srcID = payload->gpdId;
  }
  // If Application Id bitfield is 0b010
  else if(GP_PAIRING_OPT_IS_APPLICATION_ID_IEEE(payload->options))
  {
    zcl_memcpy(&proxyEntry[GP_TBL_GPD_ID], payload->gpdIEEE, Z_EXTADDR_LEN);
    gpdID.appID = GP_OPT_APP_ID_IEEE;
    zcl_memcpy(gpdID.id.gpdExtAddr, payload->gpdIEEE, Z_EXTADDR_LEN);
  }

  zcl_memcpy(&proxyEntry[PROXY_TBL_GPD_EP], (uint8_t*)&payload->ep, sizeof(uint8_t));

  if(GP_PAIRING_ALIAS(payload->options))
  {
    zcl_memcpy(&proxyEntry[PROXY_TBL_ALIAS], (uint8_t*)&payload->assignedAlias, sizeof(uint16_t));
  }
  else
  {
    zcl_memcpy(&proxyEntry[PROXY_TBL_ALIAS], (uint8_t*)&invalidAlias, sizeof(uint16_t));
  }


  //Security options
  securityOpt |= GP_GET_SEC_LEVEL(payload->options);
  securityOpt |= GP_GET_SEC_KEY_TYPE (payload->options) << GP_OPT_SEC_LEVEL_LEN;
  zcl_memcpy( &proxyEntry[PROXY_TBL_SEC_OPT], &securityOpt, sizeof(uint8_t));

  zcl_memcpy(&proxyEntry[PROXY_TBL_SEC_FRAME], (uint8_t*)&payload->gpdSecCounter, sizeof(uint32_t));
  zcl_memcpy(&proxyEntry[PROXY_TBL_GPD_KEY],(uint8_t*)&payload->gpdKey, SEC_KEY_LEN);

  zcl_memcpy(&proxyEntry[PROXY_TBL_1ST_LSINK_ADDR], payload->sinkIEEE, Z_EXTADDR_LEN );
  zcl_memcpy(&proxyEntry[PROXY_TBL_1ST_LSINK_ADDR + Z_EXTADDR_LEN], (uint8_t*)&payload->sinkNwkAddr, sizeof(uint16_t));
  // to add the invalid entries
  zcl_memset(&proxyEntry[PROXY_TBL_2ND_LSINK_ADDR], 0xFF, 0x10);

  if(GP_PAIRING_IS_GRPCAST_DGROUP(payload->options) || GP_PAIRING_IS_GRPCAST_CGROUP(payload->options))
  {
    uint16_t alias;

    if(GP_PAIRING_IS_GRPCAST_CGROUP(payload->options))
    {
      newSinkGroup = PROXY_TBL_GET_CGROUP(HI_UINT16(entryOptions));
    }
    else
    {
      newSinkGroup = PROXY_TBL_GET_DGROUP(HI_UINT16(entryOptions));
    }

    zcl_memset(&proxyEntry[PROXY_TBL_GRP_TBL_ENTRIES], newSinkGroup, sizeof(uint8_t));
    zcl_memcpy(&proxyEntry[PROXY_TBL_1ST_GRP_ADDR], (uint8_t*)&payload->sinkGroupID, sizeof(uint16_t));
    if(GP_PAIRING_ALIAS(payload->options))
    {
      zcl_memcpy(&proxyEntry[PROXY_TBL_1ST_GRP_ADDR + sizeof(uint16_t)], (uint8_t*)&payload->assignedAlias, sizeof(uint16_t));
    }
    else
    {
      alias = gp_aliasDerivation(&gpdID);
      zcl_memcpy(&proxyEntry[PROXY_TBL_1ST_GRP_ADDR + sizeof(uint16_t)], (uint8_t*)&alias, sizeof(uint16_t));
    }
  }
  else
  {
    zcl_memset(&proxyEntry[PROXY_TBL_GRP_TBL_ENTRIES], 0x00, sizeof(uint8_t));
    zcl_memset(&proxyEntry[PROXY_TBL_1ST_GRP_ADDR + sizeof(uint16_t)], 0xFF, sizeof(uint16_t));
  }
  // to add the invalid entries
  zcl_memset(&proxyEntry[PROXY_TBL_2ND_GRP_ADDR], 0xFF, sizeof(uint32_t));


  zcl_memcpy(&proxyEntry[PROXY_TBL_RADIUS], &payload->forwardingRadius, sizeof(uint8_t));

  if((PROXY_TBL_GET_ENTRY_ACTIVE(proxyEntry[0] ) == 0) && (PROXY_TBL_GET_ENTRY_VALID(proxyEntry[0]) == 0))
  {
    zcl_memcpy(&proxyEntry[PROXY_TBL_SEARCH_COUNTER], &searchCounter, sizeof(uint8_t));
  }
  else
  {
    zcl_memset(&proxyEntry[PROXY_TBL_SEARCH_COUNTER], 0xFF, sizeof(uint8_t));
  }
  // Update the proxy table
  return gp_UpdateProxyTbl(proxyEntry, payload->options, TRUE);
}

/*********************************************************************
 * @fn          gp_CheckAnnouncedDevice
 *
 * @brief       General function to check if it has the announced device
 *              listed in the SinkAddressList and look for address conflict
 *              resolution.
 *
 * @param
 *
 * @return
 */
void gp_CheckAnnouncedDevice ( uint8_t *sinkIEEE, uint16_t sinkNwkAddr )
{
  uint8_t i;
  uint8_t status;
  uint8_t ProxyTableEntry[PROXY_TBL_LEN];

#if !(defined (USE_ICALL) || defined (OSAL_PORT2TIRTOS))
  uint8_t annceDelay;
#endif

  for(i = 0; i < GPP_MAX_PROXY_TABLE_ENTRIES ; i++)
  {
    status = gp_getProxyTableByIndex(i, ProxyTableEntry);

    if(status == NV_OPER_FAILED)
    {
      // FAIL
      return;
    }

    // if the entry is empty
    if(status == NV_INVALID_DATA)
    {
      continue;
    }

    // Compare for nwk alias address conflict
    if(zcl_memcmp(&sinkNwkAddr, &ProxyTableEntry[PROXY_TBL_ALIAS], sizeof(uint16_t))        ||
       zcl_memcmp(&sinkNwkAddr, &ProxyTableEntry[PROXY_TBL_1ST_GRP_ADDR], sizeof(uint16_t)) ||
       zcl_memcmp(&sinkNwkAddr, &ProxyTableEntry[PROXY_TBL_2ND_GRP_ADDR], sizeof(uint16_t)) ||
       zcl_memcmp(&sinkNwkAddr, &ProxyTableEntry[PROXY_TBL_1ST_GRP_ADDR], sizeof(uint16_t)) ||
       zcl_memcmp(&sinkNwkAddr, &ProxyTableEntry[PROXY_TBL_2ND_GRP_ADDR], sizeof(uint16_t))   )
    {
#if (defined (USE_ICALL) || defined (OSAL_PORT2TIRTOS))
      zstack_gpAddrConflict_t *pMsg;

      pMsg = (zstack_gpAddrConflict_t*)zcl_mem_alloc(sizeof(zstack_gpAddrConflict_t));
      if(pMsg != NULL)
      {
          zcl_memset(pMsg, 0, sizeof(zstack_gpAddrConflict_t));

          pMsg->nwkAddr = sinkNwkAddr;
          zcl_memset( pMsg->extAddr, 0xFF, Z_EXTADDR_LEN );
          pMsg->conflictResolution = FALSE;

          Zstackapi_gpAliasConflict(gpAppEntity, pMsg);
          zcl_mem_free(pMsg);
      }
      else
      {
          // handle malloc error
      }
#else
      aliasConflictAnnce.nwkAddr = sinkNwkAddr;
      zcl_memset(aliasConflictAnnce.extAddr, 0xFF, Z_EXTADDR_LEN);
      aliasConflictAnnce.capabilities = 0;

      annceDelay = ( OsalPort_rand()/650 ) + 5; // ( 65535/650 = 100 ) + 5
      // where:
      // 100 is Dmax
      // 5   is Dmin
      OsalPortTimers_startTimer(gp_TaskID, GP_PROXY_ALIAS_CONFLICT_TIMEOUT, annceDelay);
#endif
    }

    if(sinkIEEE)
    {
      //Check if one of the Sink IEEE is the same
      if(zcl_memcmp(sinkIEEE, &ProxyTableEntry[PROXY_TBL_1ST_LSINK_ADDR], Z_EXTADDR_LEN))
      {
        // If Nwk address is different, then update the new address
        if(!zcl_memcmp(&sinkNwkAddr, &ProxyTableEntry[PROXY_TBL_1ST_LSINK_ADDR + Z_EXTADDR_LEN], sizeof(uint16_t)))
        {
          zcl_memcpy( &ProxyTableEntry[PROXY_TBL_1ST_LSINK_ADDR + Z_EXTADDR_LEN], &sinkNwkAddr, sizeof(uint16_t));
        }
      }
      else if(zcl_memcmp(sinkIEEE, &ProxyTableEntry[PROXY_TBL_2ND_LSINK_ADDR], Z_EXTADDR_LEN))
      {
        // If Nwk address is different, then update the new address
        if (!zcl_memcmp(&sinkNwkAddr, &ProxyTableEntry[PROXY_TBL_2ND_LSINK_ADDR + Z_EXTADDR_LEN], sizeof(uint16_t)))
        {
          zcl_memcpy(&ProxyTableEntry[PROXY_TBL_2ND_LSINK_ADDR + Z_EXTADDR_LEN], &sinkNwkAddr, sizeof(uint16_t));
        }
      }
    }
  }
  return;
}

 /*********************************************************************
 * PRIVATE FUNCTIONS
 *********************************************************************/

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

  newAlias = PROXY_TBL_GET_ASSIGNED_ALIAS(pNew[PROXY_TBL_OPT + 1]);

  return newAlias;
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

  secUse = PROXY_TBL_GET_SEC_USE(pNew[PROXY_TBL_OPT + 1]);

  return secUse;
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

  secCap = PROXY_TBL_GET_SEC_CAP(pNew[PROXY_TBL_OPT]);

  return secCap;
}

/*********************************************************************
 * @fn          pt_updateLightweightUnicastSink
 *
 * @brief       To add new sink entry
 *
 * @param       pNew - New entry array to be added
 *              pCurr - The current entry array in the proxy table
 *
 * @return      TRUE new entry added, FALSE if not
 */
static uint8_t pt_updateLightweightUnicastSink( uint8_t* pNew, uint8_t* pCurr )
{
  uint8_t freeSinkEntry[LSINK_ADDR_LEN] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

  // if the current entry doesn't have entries for lightweight unicast
  if(zcl_memcmp( freeSinkEntry, &pCurr[PROXY_TBL_1ST_LSINK_ADDR], LSINK_ADDR_LEN))
  {
    zcl_memcpy( &pCurr[PROXY_TBL_1ST_LSINK_ADDR], &pNew[PROXY_TBL_1ST_LSINK_ADDR], LSINK_ADDR_LEN);
    return TRUE;
  }
  // if the incoming Sink entry is different to the current entry
  else if(!zcl_memcmp( &pCurr[PROXY_TBL_1ST_LSINK_ADDR], &pNew[PROXY_TBL_1ST_LSINK_ADDR], LSINK_ADDR_LEN))
  {
    // add the sink entry in the next slot if empty
    if(zcl_memcmp(freeSinkEntry, &pCurr[PROXY_TBL_2ND_LSINK_ADDR], LSINK_ADDR_LEN))
    {
      zcl_memcpy(&pCurr[PROXY_TBL_2ND_LSINK_ADDR], &pNew[PROXY_TBL_1ST_LSINK_ADDR], LSINK_ADDR_LEN);
      return TRUE;
    }
  }
  else
  {
    return TRUE;
  }

  // if the incoming Sink entry is different to the current entry
  if(!zcl_memcmp(&pCurr[PROXY_TBL_2ND_LSINK_ADDR], &pNew[PROXY_TBL_1ST_LSINK_ADDR], LSINK_ADDR_LEN))
  {
    return FALSE;
  }

  return FALSE;
}

/*********************************************************************
 * @fn          pt_removeLightweightUnicastSink
 *
 * @brief       To remove a LightweightUnicast sink entry
 *
 * @param       pEntry - Entry array to be removed
 *              pAddr - LightweightUnicast address to be removed
 *
 * @return      TRUE new entry added, FALSE if not
 */
static uint8_t pt_removeLightweightUnicastSink( uint8_t* pEntry, uint8_t* pAddr )
{
  uint8_t freeSinkEntry[LSINK_ADDR_LEN] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

  // if the incoming Sink entry is the same to the current entry
  if(zcl_memcmp(&pEntry[PROXY_TBL_1ST_LSINK_ADDR], pAddr, LSINK_ADDR_LEN))
  {
      // Move the next sink entry to current
      zcl_memcpy(&pEntry[PROXY_TBL_1ST_LSINK_ADDR], &pEntry[PROXY_TBL_2ND_LSINK_ADDR], LSINK_ADDR_LEN);
      // Invalidate the sink entry
      zcl_memcpy(&pEntry[PROXY_TBL_2ND_LSINK_ADDR], freeSinkEntry, LSINK_ADDR_LEN);

      // if the sink entry is empty then remove the proxy table entry
      if(zcl_memcmp(freeSinkEntry, &pEntry[PROXY_TBL_1ST_LSINK_ADDR], LSINK_ADDR_LEN))
      {
        return TRUE;
      }
      return FALSE;
  }
  // if the incoming Sink entry is the same to the current entry
  else if(zcl_memcmp(&pEntry[PROXY_TBL_2ND_LSINK_ADDR], pAddr, LSINK_ADDR_LEN))
  {
      // Invalidate the sink entry
      zcl_memcpy(&pEntry[PROXY_TBL_2ND_LSINK_ADDR], freeSinkEntry, LSINK_ADDR_LEN);
      return FALSE;
  }

  // if the sink entry is empty then remove the proxy table entry
  if(zcl_memcmp(freeSinkEntry, &pEntry[PROXY_TBL_1ST_LSINK_ADDR], LSINK_ADDR_LEN) == TRUE &&
     zcl_memcmp(freeSinkEntry, &pEntry[PROXY_TBL_2ND_LSINK_ADDR], LSINK_ADDR_LEN) == TRUE)
  {
    return TRUE;
  }

  return FALSE;
}

/*********************************************************************
 * @fn          pt_addProxyGroup
 *
 * @brief       To add new proxy group entry
 *
 * @param       pNew - New entry array to be added
 *              pCurr - The current entry array in the proxy table
 *
 * @return      TRUE new entry added, FALSE if not
 */
static uint8_t pt_addProxyGroup( uint8_t* pNew, uint8_t* pCurr )
{
  uint8_t newEntry[PROXY_TBL_LEN];
  uint8_t currEntry[PROXY_TBL_LEN];

  proxyTableCpy(&newEntry, pNew);
  proxyTableCpy(&currEntry, pCurr);

  if(!GET_BIT( &currEntry[PROXY_TBL_GRP_TBL_ENTRIES], PROXY_TBL_2ND_GRP_BIT))
  {
    if(!GET_BIT( &currEntry[PROXY_TBL_GRP_TBL_ENTRIES], PROXY_TBL_1ST_GRP_BIT))
    {
      zcl_memcpy( &currEntry[PROXY_TBL_1ST_GRP_ADDR], &newEntry[PROXY_TBL_1ST_GRP_ADDR], sizeof(uint32_t));
      SET_BIT( &currEntry[PROXY_TBL_GRP_TBL_ENTRIES], PROXY_TBL_1ST_GRP_BIT);
      proxyTableCpy( pCurr, &currEntry );
      return TRUE;
    }
    else
    {
      zcl_memcpy( &currEntry[PROXY_TBL_2ND_GRP_ADDR], &newEntry[PROXY_TBL_1ST_GRP_ADDR], sizeof(uint32_t));
      SET_BIT( &currEntry[PROXY_TBL_GRP_TBL_ENTRIES], PROXY_TBL_2ND_GRP_BIT);
      proxyTableCpy( pCurr, &currEntry );
      return TRUE;
    }
  }
  return FALSE;
}

/*********************************************************************
 * @fn          pt_removeProxyGroup
 *
 * @brief       To remove proxy group entry
 *
 * @param       pEntry -Entry array to be removed
 *              groupAddr - The group address to be removed
 *
 * @return      TRUE remove entire entry, FALSE if not
 */
static uint8_t pt_removeProxyGroup( uint8_t* pEntry, uint16_t groupAddr )
{

  if(GET_BIT(&pEntry[PROXY_TBL_GRP_TBL_ENTRIES], PROXY_TBL_1ST_GRP_BIT))
  {
    if(zcl_memcmp(&pEntry[PROXY_TBL_1ST_GRP_ADDR], &groupAddr, sizeof(uint16_t)) == TRUE)
    {
      CLR_BIT(&pEntry[PROXY_TBL_GRP_TBL_ENTRIES], PROXY_TBL_1ST_GRP_BIT);
      zcl_memset(&pEntry[PROXY_TBL_1ST_GRP_ADDR], 0xFF, sizeof(uint32_t));
      return TRUE;
    }
  }
  if(GET_BIT(&pEntry[PROXY_TBL_GRP_TBL_ENTRIES], PROXY_TBL_2ND_GRP_BIT))
  {
    if(zcl_memcmp(&pEntry[PROXY_TBL_2ND_GRP_ADDR], &groupAddr, sizeof(uint16_t)) == TRUE)
    {
      CLR_BIT(&pEntry[PROXY_TBL_GRP_TBL_ENTRIES], PROXY_TBL_2ND_GRP_BIT);
      zcl_memset(&pEntry[PROXY_TBL_2ND_GRP_ADDR], 0xFF, sizeof(uint32_t));
      return FALSE;
    }
    else
    {
      SET_BIT(&pEntry[PROXY_TBL_GRP_TBL_ENTRIES], PROXY_TBL_1ST_GRP_BIT);
      zcl_memcpy(&pEntry[PROXY_TBL_1ST_GRP_ADDR], &pEntry[PROXY_TBL_2ND_GRP_ADDR], sizeof (uint32_t));
      return FALSE;
    }
  }
  return TRUE;
}

#endif
/*********************************************************************
*********************************************************************/

