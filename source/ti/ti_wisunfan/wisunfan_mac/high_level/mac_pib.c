/******************************************************************************

 @file  mac_pib.c

 @brief This module contains procedures for the MAC PIB.

 Group: WCS, LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2006-2023, Texas Instruments Incorporated

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

 ******************************************************************************
 
 
 *****************************************************************************/

/* ------------------------------------------------------------------------------------------------
 *                                          Includes
 * ------------------------------------------------------------------------------------------------
 */

#ifndef LIBRARY
#include "ti_wisunfan_features.h"
#endif

#include "hal_mcu.h"
#include "timac_api.h"
#include "mac_spec.h"
#include "mac_radio.h"
#include "mac_low_level.h"
#include "mac_radio_defs.h"
#include "mac_main.h"
#include "mac_pib.h"
#include "mac_mgmt.h"
#include "mac_assert.h"
#include "mac_hl_patch.h"
#include <stddef.h>

/******************************************************************************
  ROM jump table will support
  1. TIMAC ROM image build
  2. TIMAC stack build without ROM image
  3. TIMAC stack with calling ROM image
 *****************************************************************************/
#include "rom_jt_154.h"


/* ------------------------------------------------------------------------------------------------
 *                                           Local Variables
 * ------------------------------------------------------------------------------------------------
 */


extern uint8 MAC_MlmeGetReqSize( uint8 pibAttribute );

#if defined( FEATURE_MAC_PIB_PTR )

/* Pointer to the MAC PIB */
macPib_t* pMacPib = &macPib;

/**************************************************************************************************
 * @fn          MAC_MlmeSetActivePib
 *
 * @brief       This function initializes the PIB.
 *
 * input parameters
 *
 * @param       None.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
void MAC_MlmeSetActivePib( void* pPib )
{
  halIntState_t intState;
  HAL_ENTER_CRITICAL_SECTION(intState);
  pMacPib = (macPib_t *)pPib;
  HAL_EXIT_CRITICAL_SECTION(intState);
}
#endif /* FEATURE_MAC_PIB_PTR */

/**************************************************************************************************
 * @fn          macPibReset
 *
 * @brief       This function initializes the PIB.
 *
 * input parameters
 *
 * @param       None.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
MAC_INTERNAL_API void macPibReset(void)
{
  /* copy PIB defaults */
#if defined( FEATURE_MAC_PIB_PTR )
  *pMacPib = macPibDefaults;
#else
  //macPib = macPibDefaults;
  MAP_osal_memcpy((void *)&macPib, (const void *)&macPibDefaults, sizeof(macPib));
#endif /* FEATURE_MAC_PIB_PTR */

  /* initialize random sequence numbers */
  pMacPib->dsn = MAP_macRadioRandomByte();
  pMacPib->bsn = MAP_macRadioRandomByte();
}

/**************************************************************************************************
 * @fn          MAC_MlmeGetReq
 *
 * @brief       This direct execute function retrieves an attribute value
 *              from the MAC PIB.
 *
 * input parameters
 *
 * @param       pibAttribute - The attribute identifier.
 * @param       pValue - pointer to the attribute value.
 *
 * output parameters
 *
 * @param       pValue - pointer to the attribute value.
 *
 * @return      The status of the request, as follows:
 *              MAC_SUCCESS Operation successful.
 *              MAC_UNSUPPORTED_ATTRIBUTE Attribute not found.
 *
 **************************************************************************************************
 */
uint8 MAC_MlmeGetReq(uint8 pibAttribute, void *pValue)
{
  uint8         i;
  halIntState_t intState;

  if ((i = MAP_macPibIndex(pibAttribute)) == MAC_PIB_INVALID)
  {
    return MAC_UNSUPPORTED_ATTRIBUTE;
  }

  HAL_ENTER_CRITICAL_SECTION(intState);
  MAP_osal_memcpy(pValue, (uint8 *) pMacPib + macPibTbl[i].offset, macPibTbl[i].len);
  HAL_EXIT_CRITICAL_SECTION(intState);
  return MAC_SUCCESS;
}

/**************************************************************************************************
 * @fn          MAC_MlmeGetReqSize
 *
 * @brief       This direct execute function gets the MAC PIB attribute size
 *
 * input parameters
 *
 * @param       pibAttribute - The attribute identifier.
 *
 * output parameters
 *
 * None.
 *
 * @return      size in bytes
 *
 **************************************************************************************************
 */
uint8 MAC_MlmeGetReqSize( uint8 pibAttribute )
{
  uint8 index;

  if ((index = MAP_macPibIndex(pibAttribute)) == MAC_PIB_INVALID)
  {
    return 0;
  }

  return ( macPibTbl[index].len );
}

/**************************************************************************************************
 * @fn          MAC_MlmeSetReq
 *
 * @brief       This direct execute function sets an attribute value
 *              in the MAC PIB.
 *
 * input parameters
 *
 * @param       pibAttribute - The attribute identifier.
 * @param       pValue - pointer to the attribute value.
 *
 * output parameters
 *
 * None.
 *
 * @return      The status of the request, as follows:
 *              MAC_SUCCESS Operation successful.
 *              MAC_UNSUPPORTED_ATTRIBUTE Attribute not found.
 *
 **************************************************************************************************
 */
uint8 MAC_MlmeSetReq(uint8 pibAttribute, void *pValue)
{
  uint8         i;
  halIntState_t intState;

  if (MAP_macPibCheckByPatch(pibAttribute, pValue) == MAC_PIB_PATCH_DONE)
  {
      return MAC_SUCCESS;
  }

  if (pibAttribute == MAC_BEACON_PAYLOAD)
  {
    return MAC_UNSUPPORTED_ATTRIBUTE;  
  }

  /* look up attribute in PIB table */
  if ((i = MAP_macPibIndex(pibAttribute)) == MAC_PIB_INVALID)
  {
    return MAC_UNSUPPORTED_ATTRIBUTE;
  }

  /* do range check; no range check if min and max are zero */
  if ((macPibTbl[i].min != 0) || (macPibTbl[i].max != 0))
  {
    /* if min == max, this is a read-only attribute */
    if (macPibTbl[i].min == macPibTbl[i].max)
    {
      return MAC_READ_ONLY;
    }

    /* check for special cases */
    if (pibAttribute == MAC_MAX_FRAME_TOTAL_WAIT_TIME)
    {
      if ((*((uint16 *) pValue) < MAC_MAX_FRAME_RESPONSE_MIN) ||
          (*((uint16 *) pValue) > MAC_MAX_FRAME_RESPONSE_MAX))
      {
        return MAC_INVALID_PARAMETER;
      }
    }
    else if (pibAttribute == MAC_EBEACON_ORDER_NBPAN)
    {
      if ((*((uint16 *) pValue) < MAC_EBEACON_ORDER_NBPAN_MIN) ||
          (*((uint16 *) pValue) > MAC_EBEACON_ORDER_NBPAN_MAX))
      {
        return MAC_INVALID_PARAMETER;
      }
    }
    else if (pibAttribute == MAC_PHY_TRANSMIT_POWER_SIGNED)
    {
      if (MAP_macGetRadioTxPowerReg(*((int8*) pValue)) == MAC_RADIO_TX_POWER_INVALID)
      {
        return MAC_INVALID_PARAMETER;
      }
    }
    else if (pibAttribute == MAC_PHY_CURRENT_DESCRIPTOR_ID)
    {
      /* Only if the PhyID changes, reconfigure radio per PhyID */
      if (pMacPib->curPhyID == *((uint8*) pValue))
      {
        MAP_macSetDefaultsByPhyID();
        return MAC_SUCCESS;
      }
    }
    // Added preamble setting -> changes based on phy
    else if (pibAttribute == MAC_PHY_FSK_PREAMBLE_LEN)
    {
        pMacPib->fskPreambleLen = *((uint8*) pValue);
    }
    /* range check for general case */
    else if ((*((uint8 *) pValue) < macPibTbl[i].min) || (*((uint8 *) pValue) > macPibTbl[i].max))
    {
      return MAC_INVALID_PARAMETER;
    }

  }

  /* set value in PIB */
  HAL_ENTER_CRITICAL_SECTION(intState);
  MAP_osal_memcpy((uint8 *) pMacPib + macPibTbl[i].offset, pValue, macPibTbl[i].len);
  HAL_EXIT_CRITICAL_SECTION(intState);

  /* handle special cases */
  switch (pibAttribute)
  {
    case MAC_PAN_ID:
      /* set pan id in radio */
      MAP_macRadioSetPanID(pMacPib->panId);
      break;

    case MAC_SHORT_ADDRESS:
      /* set short address in radio */
      MAP_macRadioSetShortAddr(pMacPib->shortAddress);
      break;

    case MAC_RX_ON_WHEN_IDLE:
      /* turn rx on or off */
      if (pMacPib->rxOnWhenIdle)
      {
        MAP_macRxEnable(MAC_RX_WHEN_IDLE);
      }
      else
      {
        MAP_macRxDisable(MAC_RX_WHEN_IDLE);
      }
      break;

    case MAC_LOGICAL_CHANNEL:
      if (TRUE != MAP_macRadioSetChannel(pMacPib->logicalChannel))
      {
        return MAC_NO_RESOURCES;
      }
      break;

    case MAC_EXTENDED_ADDRESS:
      /* set ext address in radio */
      MAP_macRadioSetIEEEAddr(pMacPib->extendedAddress.addr.extAddr);
      break;

    case MAC_PHY_TRANSMIT_POWER_SIGNED:
      MAP_macRadioSetTxPower(pMacPib->phyTransmitPower);
      break;

    case MAC_PHY_CURRENT_DESCRIPTOR_ID:
      MAP_macSetDefaultsByPhyID();
      MAP_MAC_ResumeReq();
      break;

    case MAC_RANGE_EXTENDER:
      MAP_macSetDefaultsByRE();
      MAP_macRadioSetRE(pMacPib->rangeExt);
      if (pMacPib->rangeExt)
      {
        MAP_MAC_ResumeReq();
      }
      break;

    default:
      MAP_macSetDefaultsByPatch(pibAttribute);
      break;
  }

  return MAC_SUCCESS;
}

/**************************************************************************************************
 * @fn          MAC_GetPHYParamReq
 *
 * @brief       This direct execute function retrieves an attribute value
 *              from the PHY descriptor related entries
 *
 * @param       phyAttribute - The attribute identifier.
 * @param       phyID - phyID corresponding to the attribute
 * @param       pValue - pointer to the attribute value.
 *
 * @return      The status of the request, as follows:
 *              MAC_SUCCESS Operation successful.
 *              MAC_UNSUPPORTED_ATTRIBUTE Attribute not found.
 *
 **************************************************************************************************
 */
uint8 MAC_GetPHYParamReq(uint8 phyAttribute, uint8 phyID, void *pValue)
{
  /* phyAttribute can only be MAC_PHY_DESCRIPTOR */
  macPHYDesc_t *pPhyDesc = NULL;
  uint8 status = MAC_SUCCESS;

  MAC_PARAM_STATUS( pValue != NULL, status );

  if ( MAC_SUCCESS != status )
  {
    return status;
  }

  switch (phyAttribute)
  {
    case MAC_PHY_DESCRIPTOR:
      pPhyDesc = MAP_macMRFSKGetPhyDesc(phyID);

      if ( pPhyDesc != NULL )
      {
        MAP_osal_memcpy(((macPHYDesc_t *)pValue), pPhyDesc, sizeof(macPHYDesc_t) );
      }
      else
      {
        return MAC_INVALID_PARAMETER;
      }

      break;

    default:
      return MAC_UNSUPPORTED_ATTRIBUTE;
  }
  return status;
}

/**************************************************************************************************
 * @fn          MAC_SetPHYParamReq
 *
 * @brief       This direct execute function sets an attribute value
 *              from the PHY descriptor related entries
 *              Note: this function will cause a radio restart.
 *
 * input parameters
 *
 * @param       phyAttribute - The attribute identifier.
*  @param       phyID corresponding to the attribute
 * @param       pValue - pointer to the attribute value.
 *
 * @return      The status of the request, as follows:
 *              MAC_SUCCESS Operation successful.
 *              MAC_UNSUPPORTED_ATTRIBUTE Attribute not found.
 *
 **************************************************************************************************
 */
uint8 MAC_SetPHYParamReq(uint8 phyAttribute, uint8 phyID, void *pValue)
{
  uint8 status = MAC_SUCCESS;
  MAC_PARAM_STATUS( pValue != NULL, status );

  if ( MAC_SUCCESS != status )
  {
    return status;
  }
#if defined(COMBO_MAC) || !defined(FREQ_2_4G)
  /* phyAttribute can only be MAC_PHY_DESCRIPTOR */
  switch (phyAttribute)
  {
    case MAC_PHY_DESCRIPTOR:

       if ( ( phyID >= MAC_MRFSK_GENERIC_PHY_ID_BEGIN ) &&
            ( phyID <= MAC_MRFSK_GENERIC_PHY_ID_END) )
       {
         MAP_osal_memcpy(&macMRFSKGenPhyTable[phyID - MAC_MRFSK_GENERIC_PHY_ID_BEGIN] , pValue,
                     sizeof(macPHYDesc_t));

         /* Reconfigure radio per phyID */
       }
       else
       {
         /* TBD if standard PHY descriptors can be overwritten and/or need to
          * to set valid flag for them */
         return MAC_INVALID_PARAMETER;
       }

       break;

    default:
      return MAC_UNSUPPORTED_ATTRIBUTE;
  }
#endif
  return status;
}
#if defined(COMBO_MAC) || !defined(FREQ_2_4G)
/**************************************************************************************************
 * @fn          macMRFSKGetPhyDesc
 *
 * @brief       This function selects the indexed MR-FSK PHY descriptor
 *
 * input parameters
 *
 * @param       phyID - index of the PHY descriptor
 *
 * @return      pValue - pointer to the PHY descriptor
 *
 **************************************************************************************************
 */
MAC_INTERNAL_API macMRFSKPHYDesc_t* macMRFSKGetPhyDesc(uint8 phyID)
{
  macMRFSKPHYDesc_t *pMRFSKDesc = NULL;

  if (MAP_macCheckPhyMode(phyID) == PHY_MODE_STD)
  {
    pMRFSKDesc = (macMRFSKPHYDesc_t*) &macMRFSKStdPhyTable[phyID - MAC_MRFSK_STD_PHY_ID_BEGIN];
  }
  else if (MAP_macCheckPhyMode(phyID) == PHY_MODE_GEN)
  {
    pMRFSKDesc = (macMRFSKPHYDesc_t*) &macMRFSKGenPhyTable[phyID - MAC_MRFSK_GENERIC_PHY_ID_BEGIN];
  }
  else
  {
    MAC_ASSERT_FORCED();
  }
  return pMRFSKDesc;
}
#endif
