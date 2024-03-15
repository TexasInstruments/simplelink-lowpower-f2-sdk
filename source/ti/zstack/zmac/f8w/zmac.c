/******************************************************************************

 @file  zmac.c

 @brief This file contains the ZStack MAC Porting Layer

 Group: CMCU, LPRF
 Target Device: CC2652

 ******************************************************************************

 Copyright (c) 2005-2017, Texas Instruments Incorporated

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
 Release Name: simplelink_zigbee_sdk_plugin_0_95_00_18_s
 Release Date: 2017-11-10 13:43:40
 *****************************************************************************/

/********************************************************************************************************
 *                                               INCLUDES
 ********************************************************************************************************/


#include "rom_jt_154.h"
#include "zmac.h"
#include "mac_main.h"
#include "mac_data.h"
#include "mac_mgmt.h"
#include "zcomdef.h"
#include "ti_zstack_config.h"

#ifndef ZSTACK_GPD
#include "zglobals.h"
#endif
#include "mac_api.h"

/********************************************************************************************************
 *                                                 MACROS
 ********************************************************************************************************/

/********************************************************************************************************
 *                                               CONSTANTS
 ********************************************************************************************************/
#define MAX_SECURITY_PIB_SET_ENTRY  sizeof(deviceDescriptor_t)

// This is the maximum value that can be returned in energy detect, but it should be replaced with
// a best case value.
#if !defined ( MAX_ED_THRESHOLD )
  #define MAX_ED_THRESHOLD 0xEB
#endif

// TBD: these need to be set to the 2.4G settings
#define DEFAULT_PHYID 0
#define DEFAULT_CHANNELPAGE 0


/********************************************************************************************************
 *                                               GLOBALS
 ********************************************************************************************************/
uint32_t _ScanChannels;

 /*! Transmit Delay for Green Power. Defined by the ZStack but used by the Ti 15.4 library */
bool ApiMac_mtDataReqTxOptionGp = false;

extern uint8_t aExtendedAddress[];

static void convertCapInfo(ApiMac_capabilityInfo_t *pDst, uint8_t srcCapInfo);
static void convertToTxOptions(ApiMac_txOptions_t *pDst, uint16_t srcTxOptions);

/**************************************************************************************************
 * @fn          MAC_SetRandomSeedCB
 *
 * @brief       MAC function: Set the function pointer for the random seed callback.
 *
 * input parameters
 *
 * @param       pCBFcn - function pointer of the random seed callback
 *
 * output parameters
 *
 * None.
 *
 * @return      none
 **************************************************************************************************
 */
extern void MAC_SetRandomSeedCB(macRNGFcn_t pCBFcn);


/**************************************************************************************************
 * @fn          MAP_macMgmtReset
 *
 * @brief       This function initializes the data structures for the mgmt module.
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
#if !defined(TIMAC_ROM_IMAGE_BUILD)
extern void MAP_macMgmtReset(void);
#endif

/********************************************************************************************************
 *                                               LOCALS
 ********************************************************************************************************/

/********************************************************************************************************
 * FUNCTION PROTOTYPES
 ********************************************************************************************************/
extern void NLME_SetEnergyThreshold( uint8_t value );
/********************************************************************************************************
 *                                                TYPEDEFS
 ********************************************************************************************************/


/********************************************************************************************************
 *                                                FUNCTIONS
 ********************************************************************************************************/

/* Capability Information */
#define CAPABLE_PAN_COORD       0x01  /* Device is capable of becoming a PAN
                                        coordinator */
#define CAPABLE_FFD             0x02  /* Device is an FFD */
#define CAPABLE_MAINS_POWER     0x04  /* Device is mains powered rather than
                                        battery powered */
#define CAPABLE_RX_ON_IDLE      0x08  /* Device has its receiver on when idle
                                        */
#define CAPABLE_SECURITY        0x40  /* Device is capable of sending and
                                        receiving secured frames */
#define CAPABLE_ALLOC_ADDR      0x80  /* Request allocation of a short address
                                        in the associate procedure */
/*!
 Convert ApiMac_capabilityInfo_t data type to uint8_t capInfo

 Public function defined in api_mac.h
 */
uint8_t ApiMac_convertCapabilityInfo(ApiMac_capabilityInfo_t *pMsgcapInfo)
{
    uint8_t capInfo = 0;

    if(pMsgcapInfo->panCoord)
    {
        capInfo |= CAPABLE_PAN_COORD;
    }

    if(pMsgcapInfo->ffd)
    {
        capInfo |= CAPABLE_FFD;
    }

    if(pMsgcapInfo->mainsPower)
    {
        capInfo |= CAPABLE_MAINS_POWER;
    }

    if(pMsgcapInfo->rxOnWhenIdle)
    {
        capInfo |= CAPABLE_RX_ON_IDLE;
    }

    if(pMsgcapInfo->security)
    {
        capInfo |= CAPABLE_SECURITY;
    }

    if(pMsgcapInfo->allocAddr)
    {
        capInfo |= CAPABLE_ALLOC_ADDR;
    }

    return (capInfo);
}

/*!
 * @brief       Convert API txOptions to bitmasked txOptions.
 *
 * @param       txOptions - tx options structure
 *
 * @return      bitmasked txoptions
 */
uint16_t convertTxOptions(ApiMac_txOptions_t txOptions)
{
    uint16_t retVal = 0;

    if(txOptions.ack == true)
    {
        retVal |= MAC_TXOPTION_ACK;
    }
    if(txOptions.indirect == true)
    {
        retVal |= MAC_TXOPTION_INDIRECT;
    }
    if(txOptions.pendingBit == true)
    {
        retVal |= MAC_TXOPTION_PEND_BIT;
    }
    if(txOptions.noRetransmits == true)
    {
        retVal |= MAC_TXOPTION_NO_RETRANS;
    }
    if(txOptions.noConfirm == true)
    {
        retVal |= MAC_TXOPTION_NO_CNF;
    }
    if(txOptions.useAltBE == true)
    {
        retVal |= MAC_TXOPTION_ALT_BE;
    }
    if(txOptions.usePowerAndChannel == true)
    {
        retVal |= MAC_TXOPTION_PWR_CHAN;
    }
    if(txOptions.useGreenPower == true)
    {
        retVal |= MAC_TXOPTION_GREEN_PWR;
    }
    return (retVal);
}

/*!
 * @brief       Copy the common address type from App type to Mac Stack type.
 *
 * @param       pDst - pointer to the mac stack type
 * @param       pSrc - pointer to the application type
 */
void copyApiMacAddrToMacAddr(sAddr_t *pDst, ApiMac_sAddr_t *pSrc)
{
    /* Copy each element of the structure */
    pDst->addrMode = pSrc->addrMode;
    if(pSrc->addrMode == ApiMac_addrType_short)
    {
        pDst->addr.shortAddr = pSrc->addr.shortAddr;
    }
    else
    {
        OsalPort_memcpy(pDst->addr.extAddr, pSrc->addr.extAddr, sizeof(sAddrExt_t));
    }
}

/**************************************************************************************************
 * @fn          ZMacEventLoop
 *
 * @brief       This function is the main event handling function of the MAC executing
 *              in task context.  This function is called by OSAL when an event or message
 *              is pending for the MAC.
 *
 * input parameters
 *
 * @param       taskId - OSAL task ID of this task.
 * @param       events - OSAL event mask.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
uint32_t ZMacEventLoop(uint8_t taskId, uint32_t events)
{
  macEvent_t          *pMsg;
  macEventHdr_t       hdr;
  halIntState_t       intState;
  uint8_t               status;

  (void)taskId;  // Intentionally unreferenced parameter

  /* handle mac symbol timer events */
  /// TODO? ??? ???
  macSymbolTimerEventHandler();

  /* handle events on rx queue */
  if (events & MAC_RX_QUEUE_TASK_EVT)
  {
    while ((pMsg = (macEvent_t *) OsalPort_msgDequeue(&macData.rxQueue)) != NULL)
    {
      HAL_ENTER_CRITICAL_SECTION(intState);
      macData.rxCount--;
      HAL_EXIT_CRITICAL_SECTION(intState);

      hdr.event = pMsg->hdr.event;

      macMain.pBuf = (uint8_t *)pMsg;

      /* Check security processing result from the LMAC */
      status = pMsg->hdr.status;

      if (status == MAC_SUCCESS)
      {
        MAP_macExecute(pMsg);
      }
#ifdef FEATURE_MAC_SECURITY
      else if ((status  == MAC_IMPROPER_KEY_TYPE) ||
               (status  == MAC_IMPROPER_SECURITY_LEVEL) ||
               (status  == MAC_SECURITY_ERROR) ||
               (status  == MAC_UNAVAILABLE_KEY) ||
               (status  == MAC_UNSUPPORTED_LEGACY) ||
               (status  == MAC_UNSUPPORTED_SECURITY) ||
               (status  == MAC_INVALID_PARAMETER) ||
               (status  == MAC_COUNTER_ERROR))
      {
        MAP_macCommStatusInd(pMsg);

        /* discard invalid DATA_IND frame */
        if (hdr.event == MAC_RX_DATA_IND_EVT)
        {
          MAP_mac_msg_deallocate((uint8_t **)&pMsg);
        }
      }
#else
      else
      {
        /* discard invalid DATA_IND frame */
        if (hdr.event == MAC_RX_DATA_IND_EVT)
        {
          MAP_mac_msg_deallocate((uint8_t **)&pMsg);
        }
      }
#endif /* FEATURE_MAC_SECURITY */

      /* discard all frames except DATA_IND.
       * do not free data frames for app
       */
      if (hdr.event != MAC_RX_DATA_IND_EVT)
      {
        MAP_mac_msg_deallocate(&macMain.pBuf);
      }
    }
  }

  /* handle events on osal msg queue */
  if (events & SYS_EVENT_MSG)
  {
    while ((pMsg = (macEvent_t *) OsalPort_msgReceive(macTaskId)) != NULL)
    {
      macMain.pBuf = (uint8_t *)pMsg;
      /* execute state machine */
      MAP_macExecute(pMsg);
      MAP_mac_msg_deallocate(&macMain.pBuf);
    }
  }

  /* handle tx complete */
  if (events & MAC_TX_COMPLETE_TASK_EVT)
  {
    hdr.event = MAC_INT_TX_COMPLETE_EVT;
    MAP_macExecute((macEvent_t *) &hdr);
  }

  /* handle response wait timer */
  if (events & MAC_RESPONSE_WAIT_TASK_EVT)
  {
    hdr.event = MAC_TIM_RESPONSE_WAIT_EVT;
    MAP_macExecute((macEvent_t *) &hdr);
  }

  /* handle frame response timer */
  if (events & MAC_FRAME_RESPONSE_TASK_EVT)
  {
    hdr.event = MAC_TIM_FRAME_RESPONSE_EVT;
    MAP_macExecute((macEvent_t *) &hdr);
  }

  /* handle scan timer */
  if (events & MAC_SCAN_TASK_EVT)
  {
    hdr.event = MAC_TIM_SCAN_EVT;
    MAP_macExecute((macEvent_t *) &hdr);
  }

  /* handle indirect timer */
  if (events & MAC_EXP_INDIRECT_TASK_EVT)
  {
    hdr.event = MAC_TIM_EXP_INDIRECT_EVT;
    MAP_macExecute((macEvent_t *) &hdr);
  }

  /* handle start complete */
  if (events & MAC_START_COMPLETE_TASK_EVT)
  {
    hdr.status = MAC_SUCCESS;
    hdr.event = MAC_INT_START_COMPLETE_EVT;
    MAP_macExecute((macEvent_t *) &hdr);
  }

  /* handle pending broadcast */
  if (events & MAC_BROADCAST_PEND_TASK_EVT)
  {
    hdr.status = MAC_SUCCESS;
    hdr.event = MAC_INT_BROADCAST_PEND_EVT;
    MAP_macExecute((macEvent_t *) &hdr);
  }

  if (events & MAC_CSMA_TIM_TASK_EVT)
  {
    hdr.status = MAC_SUCCESS;
    hdr.event = MAC_CSMA_TIM_EXP_EVT ;
    MAP_macExecute((macEvent_t *) &hdr);
  }

  if (events & MAC_TX_BACKOFF_TIM_TASK_EVT)
  {
    hdr.status = MAC_SUCCESS;
    hdr.event = TX_BACKOFF_TIM_EXP_EVT;
    macExecute((macEvent_t *) &hdr);
  }

  if (events & MAC_RX_BACKOFF_TIM_TASK_EVT)
  {
    hdr.status = MAC_SUCCESS;
    hdr.event =  RX_BACKOFF_TIM_EXP_EVT;
    macExecute((macEvent_t *) &hdr);
  }

  /* handle pending message, if any */
  if (macMain.pPending != NULL)
  {
    /* Hold off interrupts */
    HAL_ENTER_CRITICAL_SECTION(intState);

    pMsg = macMain.pPending;
    macMain.pPending = NULL;

    /* Allow interrupts */
    HAL_EXIT_CRITICAL_SECTION(intState);

    macMain.pBuf = (uint8_t *)pMsg;
    /* execute state machine */
    MAP_macExecute(pMsg);
    MAP_mac_msg_deallocate(&macMain.pBuf);
  }

  return 0;
}


/********************************************************************************************************
 * @fn      ZMacInit
 *
 * @brief   Initialize MAC.
 *
 * @param   none.
 *
 * @return  status.
 ********************************************************************************************************/
uint8_t ZMacInit( void )
{
  uint8_t i;

  MAP_MAC_Init();

  if ( ZG_BUILD_RTR_TYPE )
  {
    MAP_MAC_InitCoord();
  }
  if( ZG_BUILD_JOINING_TYPE )
  {
    MAP_MAC_InitDevice();
  }

  /* reset run-time configurable high level modules */
  for (i = 0; i < MAC_FEAT_MAX; i++)
  {
    if (macReset[i] != NULL)
    {
      (*macReset[i])();
    }
  }
  MAP_macMgmtReset();

  // Turn off interrupts
  //osal_int_disable( INTS_ALL );

  // Set the Energy Detect Ceiling
  NLME_SetEnergyThreshold( MAX_ED_THRESHOLD );

  return TRUE;

}

/********************************************************************************************************
 * @fn      ZMacReset
 *
 * @brief   Reset the MAC.
 *
 * @param   Default to PIB defaults.
 *
 * @return  status.
 ********************************************************************************************************/
uint8_t ZMacReset( uint8_t SetDefaultPIB )
{
  byte stat;
  byte value;

  stat = MAP_MAC_MlmeResetReq( SetDefaultPIB );

  // Don't send PAN ID conflict
  value = FALSE;
  MAP_MAC_MlmeSetReq( MAC_ASSOCIATED_PAN_COORD, &value );
  MAP_MAC_MlmeSetReq( MAC_EXTENDED_ADDRESS, &aExtendedAddress );

  return ( stat );
}


/********************************************************************************************************
 * @fn      ZMacGetReq
 *
 * @brief   Read a MAC PIB attribute.
 *
 * @param   attr - PIB attribute to get
 * @param   value - pointer to the buffer to store the attribute
 *
 * @return  status
 ********************************************************************************************************/
uint8_t ZMacGetReq( uint8_t attr, uint8_t *value )
{
  if ( attr == ZMacExtAddr )
  {
    osal_cpyExtAddr( value, &aExtendedAddress );
    return ZMacSuccess;
  }
#ifdef IEEE_COEX_ENABLED
  else if ( attr == ZMacCoexPIBMetrics )
  {
      ((coexMetricsStruct_t *)value)->dbgCoexGrants = coexMetricsStruct.dbgCoexGrants;
      ((coexMetricsStruct_t *)value)->dbgCoexRejects = coexMetricsStruct.dbgCoexRejects;
      ((coexMetricsStruct_t *)value)->dbgCoexContRejects = coexMetricsStruct.dbgCoexContRejects;
      ((coexMetricsStruct_t *)value)->dbgCoexMaxContRejects = coexMetricsStruct.dbgCoexMaxContRejects;
      return ZMacSuccess;
  }
#endif /* IEEE_COEX_ENABLED */

  return (ZMacStatus_t) MAP_MAC_MlmeGetReq( attr, value );
}


/********************************************************************************************************
 * @fn      ZMacSetReq
 *
 * @brief   Write a MAC PIB attribute.
 *
 * @param   attr - PIB attribute to Set
 * @param   value - pointer to the data
 *
 * @return  status
 ********************************************************************************************************/
uint8_t ZMacSetReq( uint8_t attr, byte *value )
{
  if ( attr == ZMacExtAddr )
  {
    osal_cpyExtAddr( aExtendedAddress, value );
  }
  if(attr == ZMacRxOnIdle)
  {
    if ((*value == FALSE) && (zgAllowRadioRxOff == FALSE))
    {
      return ZFailure;
    }
  }
#ifdef IEEE_COEX_ENABLED
  else if ( attr == ZMacCoexPIBMetrics )
  {
      coexMetricsStruct.dbgCoexGrants = ((coexMetricsStruct_t *)value)->dbgCoexGrants;
      coexMetricsStruct.dbgCoexRejects = ((coexMetricsStruct_t *)value)->dbgCoexRejects;
      coexMetricsStruct.dbgCoexContRejects = ((coexMetricsStruct_t *)value)->dbgCoexContRejects;
      coexMetricsStruct.dbgCoexMaxContRejects = ((coexMetricsStruct_t *)value)->dbgCoexMaxContRejects;
      return ZMacSuccess;
  }
#endif /* IEEE_COEX_ENABLED */

  return (ZMacStatus_t) MAP_MAC_MlmeSetReq( attr, value );
}

#ifdef FEATURE_MAC_SECURITY
/********************************************************************************************************
 * @fn      ZMacSecurityGetReq
 *
 * @brief   Read a MAC Security PIB attribute.
 *
 * @param   attr - PIB attribute to get
 * @param   value - pointer to the buffer to store the attribute
 *
 * @return  status
 ********************************************************************************************************/
uint8_t ZMacSecurityGetReq( uint8_t attr, uint8_t *value )
{
  uint8_t         keyIndex, entry, status;
  uint8_t        *ptr;

  ptr = (uint8_t *) value;
  switch (attr)
  {
    case ZMacKeyIdLookupEntry:
      keyIndex = *ptr++; /* key_index */
      entry    = *ptr++; /* key_id_lookup_index */

      /* Transform byte array to propriety PIB struct */
      ((macSecurityPibKeyIdLookupEntry_t *)value)->key_id_lookup_index = entry;
      ((macSecurityPibKeyIdLookupEntry_t *)value)->key_index = keyIndex;
      break;

    case ZMacKeyDeviceEntry:
      keyIndex = *ptr++; /* key_index */
      entry    = *ptr++; /* key_device_index */

      /* Transform byte array to propriety PIB struct */
      ((macSecurityPibKeyDeviceEntry_t *)value)->key_device_index = entry;
      ((macSecurityPibKeyDeviceEntry_t *)value)->key_index = keyIndex;
      break;

    case ZMacKeyUsageEntry:
      keyIndex = *ptr++; /* key_index */
      entry    = *ptr++; /* key_key_usage_index */

      /* Transform byte array to propriety PIB struct */
      ((macSecurityPibKeyUsageEntry_t *)value)->key_key_usage_index = entry;
      ((macSecurityPibKeyUsageEntry_t *)value)->key_index = keyIndex;
      break;

    case ZMacKeyEntry:
      keyIndex = *ptr++; /* key_index */

      /* Transform byte array to propriety PIB struct */
      ((macSecurityPibKeyEntry_t *)value)->key_index = keyIndex;
      break;

    case ZMacDeviceEntry:
      entry = *ptr++; /* device_index */

      /* Transform byte array to propriety PIB struct */
      ((macSecurityPibDeviceEntry_t *)value)->device_index = entry;
      break;

    case ZMacSecurityLevelEntry:
      entry = *ptr++; /* security_level_index */

      /* Transform byte array to propriety PIB struct */
      ((macSecurityPibSecurityLevelEntry_t *)value)->security_level_index = entry;
      break;
  }

  status = (ZMacStatus_t) MAP_MAC_MlmeGetSecurityReq( attr, value );

  ptr = (uint8_t *) value;
  switch (attr)
  {
    case ZMacKeyIdLookupEntry:
      *ptr++ = keyIndex; /* key_index */
      *ptr++ = entry;    /* key_id_lookup_index */

      /* Transform propriety PIB struct to byte array */
      OsalPort_memcpy(ptr, &((macSecurityPibKeyIdLookupEntry_t *)value)->macKeyIdLookupEntry, sizeof(keyIdLookupDescriptor_t));
      break;

    case ZMacKeyDeviceEntry:
      *ptr++ = keyIndex; /* key_index */
      *ptr++ = entry;    /* key_device_index */

      /* Transform propriety PIB struct to byte array */
      OsalPort_memcpy(ptr, &((macSecurityPibKeyDeviceEntry_t *)value)->macKeyDeviceEntry, sizeof(keyDeviceDescriptor_t));
      break;

    case ZMacKeyUsageEntry:
      *ptr++ = keyIndex; /* key_index */
      *ptr++ = entry;    /* key_key_usage_index */

      /* Transform propriety PIB struct to byte array */
      OsalPort_memcpy(ptr, &((macSecurityPibKeyUsageEntry_t *)value)->macKeyUsageEntry, sizeof(keyUsageDescriptor_t));
      break;

    case ZMacKeyEntry:
      *ptr++ = keyIndex; /* key_index */

      /* Transform propriety PIB struct to byte array */
      OsalPort_memcpy(ptr, &((macSecurityPibKeyEntry_t *)value)->keyEntry, MAC_KEY_MAX_LEN);
      break;

    case ZMacDeviceEntry:
      *ptr++ = entry; /* device_index */

      /* Transform propriety PIB struct to byte array */
      OsalPort_memcpy(ptr, &((macSecurityPibDeviceEntry_t *)value)->macDeviceEntry, sizeof(deviceDescriptor_t));
      break;

    case ZMacSecurityLevelEntry:
      *ptr++ = entry; /* security_level_index */

      /* Transform propriety PIB struct to byte array */
      OsalPort_memcpy(ptr, &((macSecurityPibSecurityLevelEntry_t *)value)->macSecurityLevelEntry, sizeof(securityLevelDescriptor_t));
      break;
  }

  return status;
}


/********************************************************************************************************
 * @fn      ZMacSecuritySetReq
 *
 * @brief   Write a MAC Security PIB attribute.
 *
 * @param   attr - PIB attribute to Set
 * @param   value - pointer to the data
 *
 * @return  status
 ********************************************************************************************************/
uint8_t ZMacSecuritySetReq( uint8_t attr, byte *value )
{
  uint8_t         keyIndex, entry;
  uint32_t        frameCounter;
  uint8_t        *ptr;
  uint8_t         tmp[MAX_SECURITY_PIB_SET_ENTRY]; /* This must cover the largest single PIB entry */
  uint8_t         temp[MAX_SECURITY_PIB_SET_ENTRY]; /* This must cover the largest single PIB entry */

  /* Note that the Transform byte array to propriety PIB struct below works
   * because the members of the structures are all defined manually in a way,
   * where OsalPort_memcpy would work, without usage of explicit 'pack', they fit
   * such that OsalPort_memcpy would work
   */
  ptr = (uint8_t *) value;
  switch (attr)
  {
    case ZMacKeyIdLookupEntry:
      keyIndex = *ptr++; /* key_index */
      entry    = *ptr++; /* key_id_lookup_index */

      /* Transform byte array to propriety PIB struct */
      OsalPort_memcpy(tmp, ptr, sizeof(keyIdLookupDescriptor_t));
      OsalPort_memcpy(&((macSecurityPibKeyIdLookupEntry_t *)temp)->macKeyIdLookupEntry, tmp, sizeof(keyIdLookupDescriptor_t));
      ((macSecurityPibKeyIdLookupEntry_t *)temp)->key_id_lookup_index = entry;
      ((macSecurityPibKeyIdLookupEntry_t *)temp)->key_index = keyIndex;
      return (ZMacStatus_t) MAP_MAC_MlmeSetSecurityReq( attr, temp );

    case ZMacKeyDeviceEntry:
      keyIndex = *ptr++; /* key_index */
      entry    = *ptr++; /* key_device_index */

      /* Transform byte array to propriety PIB struct */
      OsalPort_memcpy(tmp, ptr, sizeof(keyDeviceDescriptor_t));
      OsalPort_memcpy(&((macSecurityPibKeyDeviceEntry_t *)temp)->macKeyDeviceEntry, tmp, sizeof(keyDeviceDescriptor_t));
      ((macSecurityPibKeyDeviceEntry_t *)temp)->key_device_index = entry;
      ((macSecurityPibKeyDeviceEntry_t *)temp)->key_index = keyIndex;
      return (ZMacStatus_t) MAP_MAC_MlmeSetSecurityReq( attr, temp );

    case ZMacKeyUsageEntry:
      keyIndex = *ptr++; /* key_index */
      entry    = *ptr++; /* key_key_usage_index */

      /* Transform byte array to propriety PIB struct */
      OsalPort_memcpy(tmp, ptr, sizeof(keyUsageDescriptor_t));
      OsalPort_memcpy(&((macSecurityPibKeyUsageEntry_t *)temp)->macKeyUsageEntry, tmp, sizeof(keyUsageDescriptor_t));
      ((macSecurityPibKeyUsageEntry_t *)temp)->key_key_usage_index = entry;
      ((macSecurityPibKeyUsageEntry_t *)temp)->key_index = keyIndex;
      return (ZMacStatus_t) MAP_MAC_MlmeSetSecurityReq( attr, temp );

    case ZMacKeyEntry:
      keyIndex = *ptr++; /* key_index */

      /* Transform byte array to propriety PIB struct */
      OsalPort_memcpy(tmp, ptr, MAC_KEY_MAX_LEN);
      OsalPort_memcpy(&((macSecurityPibKeyEntry_t *)temp)->keyEntry, tmp, MAC_KEY_MAX_LEN);
      ptr+=MAC_KEY_MAX_LEN;

      OsalPort_memcpy(&frameCounter, ptr, sizeof(frameCounter));

      ((macSecurityPibKeyEntry_t *)temp)->frameCounter = frameCounter;
      ((macSecurityPibKeyEntry_t *)temp)->key_index = keyIndex;
      return (ZMacStatus_t) MAP_MAC_MlmeSetSecurityReq( attr, temp );

    case ZMacDeviceEntry:
       entry = *ptr++;

       /* Transform byte array to propriety PIB struct */
       OsalPort_memcpy(tmp, ptr, sizeof(deviceDescriptor_t));
       OsalPort_memcpy(&((macSecurityPibDeviceEntry_t *)temp)->macDeviceEntry, tmp, sizeof(deviceDescriptor_t));

      ((macSecurityPibDeviceEntry_t *)temp)->device_index = entry;
      return (ZMacStatus_t) MAP_MAC_MlmeSetSecurityReq( attr, temp );

    case ZMacSecurityLevelEntry:
      entry = *ptr++; /* security_level_index */

      /* Transform byte array to propriety PIB struct */
      OsalPort_memcpy(tmp, ptr, sizeof(securityLevelDescriptor_t));
      OsalPort_memcpy(&((macSecurityPibSecurityLevelEntry_t *)temp)->macSecurityLevelEntry, tmp, sizeof(securityLevelDescriptor_t));

      ((macSecurityPibSecurityLevelEntry_t *)temp)->security_level_index = entry;
      return (ZMacStatus_t) MAP_MAC_MlmeSetSecurityReq( attr, temp );
  }

  return (ZMacStatus_t) MAP_MAC_MlmeSetSecurityReq( attr, value );
}
#endif /* FEATURE_MAC_SECURITY */

/********************************************************************************************************
 * @fn      ZMacAssociateReq
 *
 * @brief   Request an association with a coordinator.
 *
 * @param   structure with info need to associate.
 *
 * @return  status
 ********************************************************************************************************/
uint8_t ZMacAssociateReq( ZMacAssociateReq_t *pData )
{
  ApiMac_mlmeAssociateReq_t assocReq;

  /* Right now, set security to zero */
  pData->Sec.SecurityLevel = false;

  memset(&assocReq, 0, sizeof(ApiMac_mlmeAssociateReq_t));

  OsalPort_memcpy(&assocReq.sec, &pData->Sec, sizeof(ApiMac_sec_t));

  assocReq.logicalChannel = pData->LogicalChannel;
  assocReq.channelPage = pData->ChannelPage;
  assocReq.phyID = 0;

  OsalPort_memcpy(&assocReq.coordAddress, &pData->CoordAddress, sizeof(ApiMac_sAddr_t));
  convertCapInfo(&assocReq.capabilityInformation, pData->CapabilityFlags);

  assocReq.phyID = DEFAULT_PHYID;
  assocReq.channelPage = DEFAULT_CHANNELPAGE;


  assocReq.coordPanId = pData->CoordPANId;

  MAP_MAC_MlmeAssociateReq(&assocReq);

  return ( ZMacSuccess );
}

/********************************************************************************************************
 * @fn      ZMacAssociateRsp
 *
 * @brief   Request to send an association response message.
 *
 * @param   structure with associate response and info needed to send it.
 *
 * @return  MAC_SUCCESS or MAC error code
 ********************************************************************************************************/
uint8_t ZMacAssociateRsp( ZMacAssociateRsp_t *pData )
{
  ApiMac_mlmeAssociateRsp_t assocRsp;

  /* TBD: set security to zero for now. Require Ztool change */
  pData->Sec.SecurityLevel = false;

  memset(&assocRsp, 0, sizeof(ApiMac_mlmeAssociateRsp_t));

  OsalPort_memcpy(&assocRsp.sec, &pData->Sec, sizeof(ApiMac_sec_t));
  OsalPort_memcpy(&assocRsp.deviceAddress, pData->DeviceAddress, sizeof(ApiMac_sAddrExt_t));
  assocRsp.assocShortAddress = pData->AssocShortAddress;
  assocRsp.status = (ApiMac_assocStatus_t)pData->Status;

  return ( MAP_MAC_MlmeAssociateRsp(&assocRsp) );
}

/********************************************************************************************************
 * @fn      ZMacDisassociateReq
 *
 * @brief   Request to send a disassociate request message.
 *
 * @param   structure with info need send it.
 *
 * @return  status
 ********************************************************************************************************/
uint8_t ZMacDisassociateReq( ZMacDisassociateReq_t *pData )
{
  ApiMac_mlmeDisassociateReq_t disassocReq;

  /* Right now, set security to zero */
  pData->Sec.SecurityLevel = false;

  memset(&disassocReq, 0, sizeof(ApiMac_mlmeDisassociateReq_t));

  OsalPort_memcpy(&disassocReq.sec, &pData->Sec, sizeof(ApiMac_sec_t));
  OsalPort_memcpy(&disassocReq.deviceAddress, &pData->DeviceAddress, sizeof(ApiMac_sAddrExt_t));

  disassocReq.devicePanId = pData->DevicePanId;
  disassocReq.disassociateReason = (ApiMac_disassocateReason_t)pData->DisassociateReason;
  disassocReq.txIndirect = pData->TxIndirect;

  MAP_MAC_MlmeDisassociateReq(&disassocReq);
  return ( ZMacSuccess );
}

/********************************************************************************************************
 * @fn      ZMacOrphanRsp
 *
 * @brief   Allows next higher layer to respond to an orphan indication message.
 *
 * @param   structure with info need send it.
 *
 * @return  status
 ********************************************************************************************************/
uint8_t ZMacOrphanRsp( ZMacOrphanRsp_t *pData )
{
  ApiMac_mlmeOrphanRsp_t orphanRsp;

  /* Right now, set security to zero */
  pData->Sec.SecurityLevel = false;

  memset(&orphanRsp, 0, sizeof(ApiMac_mlmeOrphanRsp_t));

  OsalPort_memcpy(&orphanRsp.sec, &pData->Sec, sizeof(ApiMac_sec_t));
  OsalPort_memcpy(&orphanRsp.orphanAddress, pData->OrphanAddress, sizeof(ApiMac_sAddrExt_t));

  orphanRsp.shortAddress = pData->ShortAddress;
  orphanRsp.associatedMember = pData->AssociatedMember;

  MAP_MAC_MlmeOrphanRsp(&orphanRsp);
  return ( ZMacSuccess );
}

/********************************************************************************************************
 * @fn      ZMacScanReq
 *
 * @brief   This function is called to perform a network scan.
 *
 * @param   param - structure with info need send it.
 *
 * @return  status
 ********************************************************************************************************/
uint8_t ZMacScanReq( ZMacScanReq_t *pData )
{
  ApiMac_mlmeScanReq_t scanReq;

  /* Channel Page */
  pData->ChannelPage = 0x00;

  memset(&scanReq, 0, sizeof(ApiMac_mlmeScanReq_t));
  OsalPort_memcpy(&scanReq.sec, &pData->Sec, sizeof(ApiMac_sec_t));

  _ScanChannels = pData->ScanChannels;
  OsalPort_bufferUint32(scanReq.scanChannels, pData->ScanChannels);
  scanReq.scanType = (ApiMac_scantype_t)pData->ScanType;
  scanReq.scanDuration = pData->ScanDuration;
  scanReq.maxResults = pData->MaxResults;
  scanReq.permitJoining = pData->PermitJoining;
  scanReq.linkQuality = pData->LinkQuality;
  scanReq.percentFilter = pData->PercentFilter;
  scanReq.phyID = DEFAULT_PHYID;
  scanReq.channelPage = DEFAULT_CHANNELPAGE;

  MAP_MAC_MlmeScanReq(&scanReq);

  return ( ZMacSuccess );
}


/********************************************************************************************************
 * @fn      ZMacStartReq
 *
 * @brief   This function is called to tell the MAC to transmit beacons
 *          and become a coordinator.
 *
 * @param   structure with info need send it.
 *
 * @return  status
 ********************************************************************************************************/
uint8_t ZMacStartReq( ZMacStartReq_t *pData )
{
  ApiMac_mlmeStartReq_t startReq;
  uint8_t rxOnIdle = TRUE;

  /* Probably want to keep the receiver on */
  MAP_MAC_MlmeSetReq( MAC_RX_ON_WHEN_IDLE, &rxOnIdle );

  /* Right now, set security to zero */
  pData->RealignSec.SecurityLevel = FALSE;
  pData->BeaconSec.SecurityLevel = FALSE;

  memset(&startReq, 0, sizeof(ApiMac_mlmeStartReq_t));

  startReq.startTime = pData->StartTime;
  startReq.panId = pData->PANID;
  startReq.logicalChannel = pData->LogicalChannel;
  startReq.channelPage = pData->ChannelPage;
  startReq.beaconOrder = pData->BeaconOrder;
  startReq.superframeOrder = pData->SuperframeOrder;
  startReq.panCoordinator = pData->PANCoordinator;
  startReq.batteryLifeExt = pData->BatteryLifeExt;
  startReq.coordRealignment = pData->CoordRealignment;

  startReq.phyID = DEFAULT_PHYID;
  startReq.channelPage = DEFAULT_CHANNELPAGE;

  // Shouldn't have to set these
  startReq.mpmParams.eBeaconOrder = 15;
  startReq.mpmParams.NBPANEBeaconOrder = 16383;


  if(zgDeviceLogicalType == ZG_DEVICETYPE_ROUTER)
  {
      MAP_MAC_MlmeSetReq(MAC_PAN_ID, &pData->PANID);
      MAP_MAC_MlmeSetReq(MAC_LOGICAL_CHANNEL, &pData->LogicalChannel);
  }

  MAP_MAC_MlmeStartReq(&startReq);

  // MAC does not issue mlmeStartConfirm(), so we have to
  // mlmeStartConfirm( stat );  This needs to be addressed some how

  return ( ZMacSuccess );
}

/********************************************************************************************************
 * @fn      ZMacSyncReq
 *
 * @brief   This function is called to request a sync to the current
 *          networks beacons.
 *
 * @param   pData - pointer to structure
 *
 * @return  ZMacSuccess
 ********************************************************************************************************/
uint8_t ZMacSyncReq( ZMacSyncReq_t *pData )
{
  ApiMac_mlmeSyncReq_t syncReq =
                  {
                   .logicalChannel = pData->LogicalChannel,
                   .channelPage = pData->ChannelPage,
                   .phyID = 0,
                   .trackBeacon = pData->TrackBeacon
                  };

  /*syncReq.logicalChannel = pData->LogicalChannel;
  syncReq.channelPage = pData->ChannelPage;
  syncReq.phyID = 0;
  syncReq.trackBeacon = pData->TrackBeacon; */

  syncReq.phyID = DEFAULT_PHYID;
  syncReq.channelPage = DEFAULT_CHANNELPAGE;

  MAP_MAC_MlmeSyncReq(&syncReq);

  return ( ZMacSuccess );
}

/********************************************************************************************************
 * @fn      ZMacPollReq
 *
 * @brief   This function is called to request MAC data request poll.
 *
 * @param   pData - pointer to structure
 *
 * @return  ZMacSuccess
 ********************************************************************************************************/
uint8_t ZMacPollReq( ZMacPollReq_t *pData )
{
  ApiMac_mlmePollReq_t pollReq;

  /* Right now, set security to zero */
  pData->Sec.SecurityLevel = false;

  memset(&pollReq, 0, sizeof(ApiMac_mlmePollReq_t));

  OsalPort_memcpy(&pollReq.coordAddress, &pData->CoordAddress, sizeof(ApiMac_sAddr_t));
  pollReq.coordPanId = pData->CoordPanId;

  MAP_MAC_MlmePollReq(&pollReq);

  return ( ZMacSuccess );
}

/********************************************************************************************************
 * @fn      ZMacDataReqSec
 *
 * @brief   Send a MAC Data Frame packet, calls the passed in function to apply non-MAC security
 *          on the MAC data field after the MAC buffer allocation.
 *
 * @param   pData - structure containing data and where to send it.
 * @param   secCB - callback function to apply security, NULL indicates no security
 *
 * @return  status
 ********************************************************************************************************/
uint8_t ZMacDataReqSec( ZMacDataReq_t *pData, applySecCB_t secCB )
{
  ApiMac_mcpsDataReq_t dataReq;
  uint8_t stat = MAC_NO_RESOURCES;

  memset(&dataReq, 0, sizeof(ApiMac_mcpsDataReq_t));

  OsalPort_memcpy( &dataReq.dstAddr, &pData->DstAddr, sizeof(ApiMac_sAddr_t) );
  dataReq.dstPanId = pData->DstPANId;
  dataReq.srcAddrMode = (ApiMac_addrType_t)pData->SrcAddrMode;

  dataReq.msduHandle = pData->Handle;

  convertToTxOptions(&dataReq.txOptions, pData->TxOptions);

  dataReq.channel = pData->Channel;
  dataReq.power = pData->Power;

  dataReq.gpOffset = pData->GpOffset;
  dataReq.gpDuration = pData->GpDuration;

  dataReq.msdu.len = pData->msduLength;
  dataReq.msdu.p = OsalPort_malloc(dataReq.msdu.len);

  if (dataReq.msdu.p)
  {
      /* Copy data */
      OsalPort_memcpy( dataReq.msdu.p, pData->msdu, pData->msduLength );

      /* Encrypt in place */
      if ( secCB )
      {
        if ( secCB( dataReq.msdu.len, dataReq.msdu.p ) != ZSuccess )
        {
          /* Deallocate the buffer */
          OsalPort_free( dataReq.msdu.p );

          return ( MAC_NO_RESOURCES );
        }
      }
  }

  /* Call Mac Data Request */
  stat = MAP_MAC_McpsDataReq( &dataReq );

  if (dataReq.msdu.p)
  {
    OsalPort_free( dataReq.msdu.p );
  }

  return ( stat );
}

/********************************************************************************************************
 * @fn      ZMacDataReq
 *
 * @brief   Send a MAC Data Frame packet.
 *
 * @param   structure containing data and where to send it.
 *
 * @return  status
 ********************************************************************************************************/
uint8_t ZMacDataReq( ZMacDataReq_t *pData )
{
  return ZMacDataReqSec( pData, NULL );
}

/********************************************************************************************************
 * @fn      ZMacPurgeReq
 *
 * @brief   Purge a MAC Data Frame packet.
 *
 * @param   MSDU data handle.
 *
 * @return  status
 ********************************************************************************************************/
uint8_t ZMacPurgeReq( byte Handle )
{
  MAP_MAC_McpsPurgeReq( Handle );
  return ZMacSuccess;
}

/********************************************************************************************************
 * @fn      ZMacSrcMatchEnable
 *
 * @brief   This function is call to enable AUTOPEND and source address matching.
 *
 * @param   addressType - address type that the application uses
 *                        SADDR_MODE_SHORT or SADDR_MODE_EXT.
 *          numEntries  - number of source address table entries to be used
 *
 * @return  status
 ********************************************************************************************************/
ZMacStatus_t ZMacSrcMatchEnable (void)
{
  MAC_SrcMatchEnable();
  return ZMacSuccess;
}

/********************************************************************************************************
 * @fn      ZMacSrcMatchAddEntry
 *
 * @brief   This function is called to add a short or extended address to source address table.
 *
 * @param   addr - a pointer to sAddr_t which contains addrMode
 *                     and a union of a short 16-bit MAC address or an extended
 *                     64-bit MAC address to be added to the source address table.
 *          panID - the device PAN ID. It is only used when the addr is
 *                      using short address
 *
 * @return  status
 ********************************************************************************************************/
ZMacStatus_t ZMacSrcMatchAddEntry (zAddrType_t *addr, uint16_t panID)
{
    uint8 status;

    status = MAC_SrcMatchAddEntry ((sAddr_t*)addr, panID);

    /* if entry is already present in table or was successfully added */
    if((status == MAC_DUPLICATED_ENTRY) || (status == MAC_SUCCESS))
    {
        MAC_SrcMatchSetPend ((sAddr_t *)addr, panID, TRUE);
    }

    return (status);
}

/********************************************************************************************************
 * @fn      ZMacSrcMatchDeleteEntry
 *
 * @brief   This function is called to delete a short or extended address from source address table.
 *
 * @param   addr - a pointer to sAddr_t which contains addrMode
 *                     and a union of a short 16-bit MAC address or an extended
 *                     64-bit MAC address to be added to the source address table.
 *          panID - the device PAN ID. It is only used when the addr is
 *                      using short address
 *
 * @return  status
 ********************************************************************************************************/
ZMacStatus_t ZMacSrcMatchDeleteEntry (zAddrType_t *addr, uint16_t panID)
{
  return (MAC_SrcMatchDeleteEntry ((sAddr_t*)addr, panID));
}

/********************************************************************************************************
 * @fn       ZMacSrcMatchAckAllPending
 *
 * @brief    Enabled/disable acknowledging all packets with pending bit set
 *           It is normally enabled when adding new entries to
 *           the source address table fails due to the table is full, or
 *           disabled when more entries are deleted and the table has
 *           empty slots.
 *
 * @param    option - true (acknowledging all packets with pending field set)
 *                    false (acknowledging all packets with pending field cleared)
 *
 * @return   status
 ********************************************************************************************************/
ZMacStatus_t ZMacSrcMatchAckAllPending (uint8_t option)
{
  MAC_SrcMatchAckAllPending (option);

  return ZMacSuccess;
}

/********************************************************************************************************
 * @fn       ZMacSrcMatchCheckAllPending
 *
 * @brief    This function is called to check if acknowledging all packets with pending bit set is enabled.
 *
 * @param    none
 *
 * @return   status
 ********************************************************************************************************/
ZMacStatus_t ZMacSrcMatchCheckAllPending (void)
{
  return (MAC_SrcMatchCheckAllPending ());
}

/********************************************************************************************************
 * @fn      - ZMACPwrOnReq
 *
 * @brief   - This function requests the MAC to power on the radio hardware
 *            and wake up.  When the power on procedure is complete the MAC
 *            will send a MAC_PWR_ON_CNF to the application.
 *
 * @input   - None.
 *
 * @output  - None.
 *
 * @return  - None.
 ********************************************************************************************************/
void ZMacPwrOnReq ( void )
{
  MAP_MAC_PwrOnReq();
}

/********************************************************************************************************
 * @fn          MAP_MAC_PwrMode
 *
 * @brief       This function returns the current power mode of the MAC.
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      The current power mode of the MAC.
 ********************************************************************************************************/
uint8_t ZMac_PwrMode(void)
{
  return (MAP_MAC_PwrMode());
}

/********************************************************************************************************
 * @fn      ZMacSetTransmitPower
 *
 * @brief   Set the transmitter power according to the level setting param.
 *
 * @param   Valid power level setting as defined in ZMAC.h.
 *
 * @return  ZMacSuccess if PHY_TRANSMIT_POWER found or ZMacUnsupportedAttribute.
 ********************************************************************************************************/
uint8_t ZMacSetTransmitPower( ZMacTransmitPower_t level )
{
  return MAP_MAC_MlmeSetReq( ZMacPhyTransmitPowerSigned, &level );
}

/********************************************************************************************************
 * @fn      ZMacSendNoData
 *
 * @brief   This function sends an empty msg
 *
 * @param   DstAddr   - destination short address
 *          DstPANId  - destination pan id
 *
 * @return  None
 ********************************************************************************************************/
void ZMacSendNoData( uint16_t DstAddr, uint16_t DstPANId )
{
  ApiMac_mcpsDataReq_t dataReq;

  memset(&dataReq, 0, sizeof(ApiMac_mcpsDataReq_t));

  /* Fill in src information */
  dataReq.srcAddrMode = (ApiMac_addrType_t)SADDR_MODE_SHORT;

  /* Fill in dst information */
  dataReq.dstAddr.addr.shortAddr = DstAddr;
  dataReq.dstAddr.addrMode = (ApiMac_addrType_t)SADDR_MODE_SHORT;
  dataReq.dstPanId = DstPANId;

  /* Misc information */
  convertToTxOptions(&dataReq.txOptions,
                   (ZMAC_TXOPTION_ACK | ZMAC_TXOPTION_NO_RETRANS | ZMAC_TXOPTION_NO_CNF));

  /* Call Mac Data Request */
  MAP_MAC_McpsDataReq(&dataReq);
}

/********************************************************************************************************
 * @fn      ZMacStateIdle
 *
 * @brief   This function returns true if the MAC state is idle.
 *
 * @param   none
 *
 * @return  TRUE if the MAC state is idle, FALSE otherwise.
 ********************************************************************************************************/
uint8_t ZMacStateIdle( void )
{
  return MAP_macStateIdle();
}

/********************************************************************************************************
 * @fn      ZMacEnhancedActiveScanReq
 *
 * @brief   This function is called to perform a network scan.
 *
 * @param   param - structure with info need send it.
 *
 * @return  status
 ********************************************************************************************************/
uint8_t ZMacEnhancedActiveScanReq( ZMacScanReq_t *pData )
{
  return ZMacScanReq(pData);
}

/********************************************************************************************************
 * @fn      convertCapInfo
 *
 * @brief   Convert the bit oriented capability info to the structure based capability info
 *
 * @param   pDst - pointer to destination structure
 * @param   srcCapInfo - bit oriented byte
 *
 * @return  None
 ********************************************************************************************************/
static void convertCapInfo(ApiMac_capabilityInfo_t *pDst, uint8_t srcCapInfo)
{
    memset(pDst, 0, sizeof(ApiMac_capabilityInfo_t));

    if (srcCapInfo & MAC_CAPABLE_PAN_COORD)
    {
        pDst->panCoord = TRUE;
    }
    if (srcCapInfo & MAC_CAPABLE_FFD)
    {
        pDst->ffd = TRUE;
    }
    if (srcCapInfo & MAC_CAPABLE_MAINS_POWER)
    {
        pDst->mainsPower = TRUE;
    }
    if (srcCapInfo & MAC_CAPABLE_RX_ON_IDLE)
    {
        pDst->rxOnWhenIdle = TRUE;
    }
    if (srcCapInfo & MAC_CAPABLE_SECURITY)
    {
        pDst->security = TRUE;
    }
    if (srcCapInfo & MAC_CAPABLE_ALLOC_ADDR)
    {
        pDst->allocAddr = TRUE;
    }
}

/********************************************************************************************************
 * @fn      convertToTxOptions
 *
 * @brief   Convert the bit oriented txOptions to the structure based txOptions
 *
 * @param   pDst - pointer to destination structure
 * @param   srcTxOptions - bit oriented byte
 *
 * @return  None
 ********************************************************************************************************/
static void convertToTxOptions(ApiMac_txOptions_t *pDst, uint16_t srcTxOptions)
{
  if ( srcTxOptions & ZMAC_TXOPTION_ACK)
  {
    pDst->ack = true;
  }
  if ( srcTxOptions & ZMAC_TXOPTION_INDIRECT)
  {
    pDst->indirect = true;
  }
  if ( srcTxOptions & ZMAC_TXOPTION_NO_RETRANS)
  {
    pDst->noRetransmits = true;
  }
  if ( srcTxOptions & ZMAC_TXOPTION_NO_CNF)
  {
    pDst->noConfirm = true;
  }
  if ( srcTxOptions & ZMAC_TXOPTION_GREEN_PWR)
  {
    pDst->useGreenPower = true;
  }
}

/********************************************************************************************************
 * @fn      ZMacSetZigbeeMACParams
 *
 * @brief   Sets MAC PIB values specific for Z-Stack (after calling ZMacReset)
 *
 * @return  None
 ********************************************************************************************************/
void ZMacSetZigbeeMACParams(void)
{
    uint8_t responseWaitTime = 16;
    ZMacSetReq(MAC_RESPONSE_WAIT_TIME, &responseWaitTime);

    uint16_t transactionPersistenceTime = 500;
    ZMacSetReq(MAC_TRANSACTION_PERSISTENCE_TIME, (byte *)&transactionPersistenceTime);

    uint8_t macFrameRetries = ZMAC_MAX_FRAME_RETRIES;
    ZMacSetReq(MAC_MAX_FRAME_RETRIES, &macFrameRetries);

    ZMacSetTransmitPower( (ZMacTransmitPower_t)TXPOWER );
}
