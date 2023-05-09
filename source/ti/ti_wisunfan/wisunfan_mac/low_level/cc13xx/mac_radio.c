/******************************************************************************

 @file  mac_radio.c

 @brief Describe the purpose and contents of the file.

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

/* hal */
#include "hal_types.h"

/* high-level */
#include "mac_pib.h"
#include "mac_main.h"
#include "mac_mgmt.h"
#include "mac_hl_patch.h"

/* exported low-level */
#include "mac_low_level.h"

/* radio specific */
#include <hw_rfc_dbell.h>
#include "rf_mac_api.h"

/* low-level specific */
#include "mac_radio.h"
#include "mac_tx.h"
#include "mac_rx.h"
#include "mac_rx_onoff.h"
#include "mac.h"
#include "mac_settings.h"

/* target specific */
#include "mac_radio_defs.h"
#include "mac_symbol_timer.h"
#include "mac_user_config.h"

/* Chip specific */
#include <hw_memmap.h>
#include <hw_fcfg1.h>
#include <chipinfo.h>

/* Activity Tracking */
#include "mac_activity.h"

/* debug */
#include "mac_assert.h"

// SW Tracer
#include "dbg.h"
#ifdef DEBUG_SW_TRACE
#define DBG_ENABLE
#ifndef _DBGFILE
#define _DBGFILE mac_radio_c
#endif
#include "dbgid_sys_mst.h"
#endif // DEBUG_SW_TRACE

#include "rom_jt_154.h"

/* ------------------------------------------------------------------------------------------------
 *                                         Defines
 * ------------------------------------------------------------------------------------------------
 */
#define ED_RF_POWER_MIN_DBM   (MAC_RADIO_RECEIVER_SENSITIVITY_DBM + MAC_SPEC_ED_MIN_DBM_ABOVE_RECEIVER_SENSITIVITY)
#define ED_RF_POWER_MAX_DBM   MAC_RADIO_RECEIVER_SATURATION_DBM

/* register value table offset */
#define MAC_RADIO_DEFS_TBL_TXPWR_FIRST_ENTRY   0
#define MAC_RADIO_DEFS_TBL_TXPWR_LAST_ENTRY    1
#define MAC_RADIO_DEFS_TBL_TXPWR_ENTRIES       2

/* 433 MHz Override and Power table offset */
#define MAC_FREQ_OVRRIDE_TBL_OFFSET_433      3
#define MAC_FREQ_PWR_TBL_OFFSET_433  1

/* Dynamic PA Override table offset */
#define MAC_DYNAMIC_PA_STD_TBL_OFFSET  1
#define MAC_DYNAMIC_PA_TX20_TBL_OFFSET 2

/* Chip user ID */
#define MAC_RADIO_CC1350_MULTI_PROTOCOL_7X7    0x2002F000
#define MAC_RADIO_CC1350_MULTI_PROTOCOL_5x5    0x2001F000
#define MAC_RADIO_CC1350_MULTI_PROTOCOL_4x4    0x2000F000
#define MAC_RADIO_CC1310_PROPRIETARY_7X7       0x20028000
#define MAC_RADIO_CC1310_PROPRIETARY_5x5       0x20018000
#define MAC_RADIO_CC1310_PROPRIETARY_4x4       0x20008000

#define MAC_RADIO_CC1352_MULTI_PROTOCOL_7X7    0x0082F000
#define MAC_RADIO_CC1352_PG1_1_MULTI_PROTOCOL_7X7    0x2082F000
#define MAC_RADIO_CC1312_PG1_1_MULTI_PROTOCOL_7X7    0x20828000
/* ------------------------------------------------------------------------------------------------
 *                                        Global Variables
 * ------------------------------------------------------------------------------------------------
 */
uint32 macPhyTxPower;
/**
    Activity Tracking Object.
    Holds the function pointers related
    to Activity tracking module.
*/
 __attribute__((weak)) const activityObject_t activityObject = {
    .pSetActivityTrackingTxFn = NULL,
    .pSetActivityTrackingRxFn = NULL,
    .pGetActivityPriorityTxFn = NULL,
    .pGetActivityPriorityRxFn = NULL,
    .pSetActivityTxFn = NULL,
    .pSetActivityRxFn = NULL,
    .pGetActivityTxFn = NULL,
    .pGetActivityRxFn = NULL,
  #ifdef MAC_ACTIVITY_PROFILING
    .pStartActivityProfilingTimerFn = NULL,
    .pPrintActivityInfoFn = NULL
  #endif
};

/* ------------------------------------------------------------------------------------------------
 *                                        Local Variables
 * ------------------------------------------------------------------------------------------------
 */
static uint8 reqChannel;
static uint32 reqTxPower;
static uint16 macSavedPanID;

RF_Mode *pRfMode;
RF_TxPowerTable_Entry *pRfPowerTable;
macRfCfg_maxPower_t *pMaxPower;

#if defined(COMBO_MAC) || !defined(FREQ_2_4G)
static uint8 phyIdOld = 0xFF;
#endif
/* RF Object for TIMAC */
static volatile RF_Object RF_object;


/* RF handle for TIMAC */
RF_Handle RF_handle = NULL;

/* RF Parameters for TIMAC */
RF_Params RF_params;

/* ------------------------------------------------------------------------------------------------
 *                                        Local Functions
 * ------------------------------------------------------------------------------------------------
 */
static void macRadioSelectConfig(void);
#if defined(COMBO_MAC) || !defined(FREQ_2_4G)
static bool  macRadioIdCheckOk(void);

/**************************************************************************************************
 * @fn          macRadioIdCheckOk
 *
 * @brief       Radio ID check.
 *
 * @param       none
 *
 * @return      TRUE if chip ID is okay to run TIMAC2.0
 *              FALSE otherwise
 **************************************************************************************************
 */
static bool macRadioIdCheckOk(void)
{
#ifndef TIMAC_AGAMA_FPGA
  if (ChipInfo_GetChipType() == CHIP_TYPE_CC1310 ||
      ChipInfo_GetChipType() == CHIP_TYPE_CC1350 ||
      ChipInfo_GetChipType() == CHIP_TYPE_CC2650 ||
      ChipInfo_GetChipType() == CHIP_TYPE_CC1312 ||
      ChipInfo_GetChipType() == CHIP_TYPE_CC1312P ||
      ChipInfo_GetChipType() == CHIP_TYPE_CC1352 ||
      ChipInfo_GetChipType() == CHIP_TYPE_CC2652 ||
      ChipInfo_GetChipType() == CHIP_TYPE_CC1352P ||
      ChipInfo_GetChipType() == CHIP_TYPE_CC1312R7 ||
#if defined(DeviceFamily_CC13X4) || defined(DeviceFamily_CC26X4)
      ChipInfo_GetChipType() == CHIP_TYPE_CC1314R10 ||
      ChipInfo_GetChipType() == CHIP_TYPE_CC1354R10 ||
      ChipInfo_GetChipType() == CHIP_TYPE_CC2674R10 ||
      ChipInfo_GetChipType() == CHIP_TYPE_CC1354P10 ||
#endif
      ChipInfo_GetChipType() == CHIP_TYPE_CC1352R7 ||
      ChipInfo_GetChipType() == CHIP_TYPE_CC1352P7)
  {
    return TRUE;
  }
  else
  {
    return FALSE;
  }
#else
  return TRUE;
#endif
}
#endif

/**************************************************************************************************
 * @fn          macGetRadioState
 *
 * @brief       Returns Radio State.
 *
 * @param       none
 *
 * @return      TRUE if Radio is ON
 *              FALSE otherwise
 **************************************************************************************************
 */
bool macGetRadioState(void)
{
  RF_InfoVal info;

  RF_getInfo(RF_handle, RF_GET_RADIO_STATE, &info);
  return (info.bRadioState);
}
/**************************************************************************************************
 * @fn          macRadioSelectConfig
 *
 * @brief       Radio config selection.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
static void macRadioSelectConfig(void)
{
    if(pMacRadioConfig->pRfSelectFP)
    {
        pMacRadioConfig->pRfSelectFP(pMacPib->curPhyID);
    }
}

#if defined(COMBO_MAC) || !defined(FREQ_2_4G)
/**************************************************************************************************
 * @fn          macRadioInitSubG
 *
 * @brief       (Re-)Initialize radio hardware.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macRadioInitSubG(void)
{
  macMRFSKPHYDesc_t *pPhyDesc;
  uint32 txPower;

  RF_ScheduleCmdParams RF_schParams;
  RF_ScheduleCmdParams_init(&RF_schParams);
  RF_schParams.endTime = 0;

  /* Acquire Activity priority */
  if (activityObject.pGetActivityPriorityTxFn)
  {
      RF_schParams.activityInfo = activityObject.pGetActivityPriorityTxFn();
  }

  if (RF_handle == NULL)
  {
    MAC_ASSERT(macRadioIdCheckOk());

    /* RF driver function table should be prepared */
    MAC_ASSERT(macRfDrvTblPtr != NULL);

    /* Initialize RF Driver parameters */
    RF_Params_init(&RF_params);
	RF_params.nID = RF_STACK_ID_154;
  }

  macRadioSelectConfig();

  pPhyDesc = MAP_macMRFSKGetPhyDesc(pMacPib->curPhyID);

  txPower = macGetRadioTxPowerReg(pMacPib->phyTransmitPower);
  if (txPower != MAC_RADIO_TX_POWER_INVALID)
  {
    RF_TxPowerTable_Value value = *(RF_TxPowerTable_Value *)&txPower;
    if (value.paType == RF_TxPowerTable_DefaultPA)
    {
        RF_cmdPropRadioDivSetup.txPower = (uint16)value.rawValue;
    }
    else
    {
        RF_cmdPropRadioDivSetup.txPower = (uint16)0xFFFF;
    }
  }

  RF_cmdPropRadioDivSetup.centerFreq = (pPhyDesc->firstChCentrFreq +
    ((pPhyDesc->noOfChannels - 1) * pPhyDesc->channelSpacing) / 2) / 1000;

  RF_cmdPropRadioDivSetup.status = IDLE;
  RF_cmdPropRadioDivSetup.pNextOp = NULL;
  RF_cmdPropRadioDivSetup.condition.rule = COND_NEVER;
  RF_cmdPropRadioDivSetup.startTrigger.pastTrig = 1;

  // apply pib value for fskPreambleLen to set the actual RF_cmdPropRadioDivSetup setting
  RF_cmdPropRadioDivSetup.preamConf.nPreamBytes = pMacPib->fskPreambleLen;

  /* don't change the below variables if csm is in operation */
  if ( FALSE == MAP_macIsCsmOperational() )
  {
    /* Use the correct symbol timing per PIB configuration */
    macSpecUsecsPerSymbol = 1000 / pPhyDesc->symbolRate;

    /* Use the symbol timing per macUnitBackoffPeriod */
    macUnitBackoffPeriod = 1000 / macSpecUsecsPerSymbol;
  }


  if (RF_handle == NULL)
  {
    /* Request access to the radio */
    RF_handle = RF_open((RF_Object *)&RF_object, pRfMode, (RF_RadioSetup*) &RF_cmdPropRadioDivSetup, &RF_params);

#ifdef MAC_ACTIVITY_PROFILING
    /* Successful transmission, reset tracking data */
    if (activityObject.pStartActivityProfilingTimerFn)
    {
        activityObject.pStartActivityProfilingTimerFn(RF_handle);
    }
#endif
  }
  else
  {
    macStopCmd(FALSE);

    if (pMacPib->curPhyID == phyIdOld)
    {
      RF_scheduleCmd(RF_handle, (RF_Op*) &RF_cmdPropRadioDivSetup, &RF_schParams,
                     NULL, RF_EventLastCmdDone);
      lastPostCmd = (uint32_t*)&RF_cmdPropRadioDivSetup;
    }
    else
    {
      /* RF_ratDisableChannel is needed ? */
      RF_close(RF_handle);
      macRadioYielded = MAC_RADIO_YIELDED;
      RF_handle = RF_open((RF_Object *)&RF_object, pRfMode, (RF_RadioSetup*) &RF_cmdPropRadioDivSetup, &RF_params);

#ifdef MAC_ACTIVITY_PROFILING
      /* Successful transmission, reset tracking data */
      if (activityObject.pStartActivityProfilingTimerFn)
      {
          activityObject.pStartActivityProfilingTimerFn(RF_handle);
      }
#endif
    }
  }
  phyIdOld = pMacPib->curPhyID;
}
#endif
/**************************************************************************************************
 * @fn          macRadioInit
 *
 * @brief       (Re-)Initialize radio hardware.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macRadioInit(void)
{
    SELECT_CALL(macRadioInit);
}

/**************************************************************************************************
 * @fn          macRadioSwInit
 *
 * @brief       Initialize radio software.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macRadioSwInit(void)
{
  /* Variable initialization for this module */
  reqChannel    = MAC_RADIO_CHANNEL_DEFAULT;
  macPhyChannel = MAC_RADIO_CHANNEL_INVALID;
  reqTxPower    = MAC_RADIO_TX_POWER_INVALID;
  macPhyTxPower = MAC_RADIO_TX_POWER_INVALID;
}


/**************************************************************************************************
 * @fn          macRadioReset
 *
 * @brief       Resets the radio module.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macRadioReset(void)
{
}

/**************************************************************************************************
 * @fn          macRadioSetRE
 *
 * @brief       Set Range Extender.
 *
 * @param       hgm - high gain mode (0 : off, 1 : high gain, 2 : low gain)
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macRadioSetRE(uint32 hgm)
{
    // This function is not used anymore in Agama.
}

/**************************************************************************************************
 * @fn          macRadioRandomByte
 *
 * @brief       Return a random byte derived from previously set random seed.
 *
 * @param       none
 *
 * @return      a random byte
 **************************************************************************************************
 */
MAC_INTERNAL_API uint8 macRadioRandomByte(void)
{
  return(MAC_RADIO_RANDOM_BYTE());
}


/**************************************************************************************************
 * @fn          macRadioSetPanCoordinator
 *
 * @brief       Configure the pan coordinator status of the radio
 *
 * @param       panCoordFlag - non-zero to configure radio to be pan coordinator
 *                             zero to configure radio as NON pan coordinator
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macRadioSetPanCoordinator(uint8 panCoordFlag)
{
  /* abstracted radio configuration */
  MAC_RADIO_SET_PAN_COORDINATOR(panCoordFlag);
}


/**************************************************************************************************
 * @fn          macRadioSetPanID
 *
 * @brief       Set the pan ID on the radio.
 *
 * @param       panID - 16 bit PAN identifier
 *
 * @return      none
 **************************************************************************************************
 */
void macRadioSetPanID(uint16 panID)
{
  /* abstracted radio configuration */
  MAC_RADIO_SET_PAN_ID(panID);
}


/**************************************************************************************************
 * @fn          macRadioSetShortAddr
 *
 * @brief       Set the short address on the radio.
 *
 * @param       shortAddr - 16 bit short address
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macRadioSetShortAddr(uint16 shortAddr)
{
  /* abstracted radio configuration */
  MAC_RADIO_SET_SHORT_ADDR(shortAddr);
}


/**************************************************************************************************
 * @fn          macRadioSetIEEEAddr
 *
 * @brief       Set the IEEE address on the radio.
 *
 * @param       pIEEEAddr - pointer to array holding 64 bit IEEE address; array must be little
 *                          endian format (starts with lowest signficant byte)
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macRadioSetIEEEAddr(uint8 * pIEEEAddr)
{
  /* abstracted radio configuration */
  MAC_RADIO_SET_EXT_ADDR(pIEEEAddr);
}


/**************************************************************************************************
 * @fn          macRadioSetTxPower
 *
 * @brief       Set transmitter power of the radio.
 *
 * @param       txPower - TX power in dBm
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API uint8 macRadioSetTxPower(int8 txPower)
{
  if (macGetRadioTxPowerReg(txPower) == MAC_RADIO_TX_POWER_INVALID)
  {
    return MAC_INVALID_PARAMETER;
  }

  /* update the radio power setting */
  macRadioUpdateTxPower();

  return MAC_SUCCESS;
}

/**************************************************************************************************
 * @fn          macRadioNeedAdjustPower
 *
 * @brief       Decide whether power adjustment is needed or not.
 *
 * @param       reqCh - new channel
 *              oldCh - old channel
 *
 * @return      TRUE if adjustment is needed
 *              FALSE otherwise
 **************************************************************************************************
 */
MAC_INTERNAL_API bool macRadioNeedAdjustPower(uint8_t reqCh, uint8_t oldCh)
{
    if(pMaxPower)
    {
        uint8_t reqChIdx;
        uint8_t oldChIdx;

        reqChIdx = reqCh - pMaxPower->first;
        oldChIdx = oldCh - pMaxPower->first;
        if((oldCh == MAC_RADIO_CHANNEL_INVALID)
        || ((oldCh < pMaxPower->first) || (oldCh >= pMaxPower->first + pMaxPower->numbers))
        || (pMaxPower->pTable[reqChIdx] != pMaxPower->pTable[oldChIdx]))
        {
            return(TRUE);
        }
    }
    return(FALSE);
}

/**************************************************************************************************
 * @fn          macRadioLimitTxPower
 *
 * @brief       Limit tx power
 *
 * @param       reqCh - current channel
 *              pTxPower - pointer of power value
 *
 * @return      none
 *
 **************************************************************************************************
 */
MAC_INTERNAL_API void macRadioLimitTxPower(uint8_t reqCh, int8_t *pTxPower)
{
    if(pMaxPower)
    {
        uint8_t reqChIdx;
        int8_t txPower = *pTxPower;

        if((reqCh >= pMaxPower->first) && (reqCh < pMaxPower->first + pMaxPower->numbers))
        {
            reqChIdx = reqCh - pMaxPower->first;
            if(txPower > pMaxPower->pTable[reqChIdx])
            {
                txPower = pMaxPower->pTable[reqChIdx];
                *pTxPower = txPower;
            }
        }
    }
}

/**************************************************************************************************
 * @fn          macGetRadioTxPowerReg
 *
 * @brief       Find transmitter power register value.
 *
 * @param       txPower - TX power in dBm.
 *
 * @return      TX Power register setting if found.
 *              0xFFFFFFFF if there is no register value that matches txPower
 **************************************************************************************************
 */
MAC_INTERNAL_API uint32 macGetRadioTxPowerReg(int8 txPower)
{
  RF_TxPowerTable_Entry *pTxPwrTbl;
  RF_TxPowerTable_Value value;

  macRadioLimitTxPower(macPhyChannel, &txPower);

  pTxPwrTbl = pRfPowerTable;

  value = RF_TxPowerTable_findValue(pTxPwrTbl, txPower);
  if (value.rawValue != RF_TxPowerTable_INVALID_VALUE)
  {
      reqTxPower = *(uint32 *)&value;
      return reqTxPower;
  }

  return MAC_RADIO_TX_POWER_INVALID;
}


/**************************************************************************************************
 * @fn          macRadioUpdateTxPower
 *
 * @brief       Update the radio's transmit power if a new power level has been requested
 *
 * @param       reqTxPower - file scope variable that holds the last request power level
 *              macPhyTxPower - global variable that holds radio's set power level
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macRadioUpdateTxPower(void)
{
  halIntState_t  s;

  /*
   *  If the requested power setting is different from the actual radio setting,
   *  attempt to udpate to the new power setting.
   */
  HAL_ENTER_CRITICAL_SECTION(s);
  if (reqTxPower != macPhyTxPower)
  {
    /*
     *  Radio power cannot be updated when the radio is physically transmitting.
     *  If there is a possibility radio is transmitting, do not change the power
     *  setting.  This function will be called again after the current transmit
     *  completes.
     */
    if (!macRxOutgoingAckFlag && !MAC_TX_IS_PHYSICALLY_ACTIVE())
    {

      /*
       *  Set new power level;  update the shadow value and write
       *  the new value to the radio hardware.
       */
      macPhyTxPower = reqTxPower;
      HAL_EXIT_CRITICAL_SECTION(s);

      macSetTxPowerVal(macPhyTxPower);

      return;
    }
  }
  HAL_EXIT_CRITICAL_SECTION(s);
}


/**************************************************************************************************
 * @fn          macRadioSetChannel
 *
 * @brief       Set radio channel.
 *
 * @param       channel - channel number, valid range is 11 through 26. Allow
 *              channels 27 and 28 for some Japanese customers.
 *
 * @return      TRUE if successful, FALSE otherwise
 **************************************************************************************************
 */
MAC_INTERNAL_API bool macRadioSetChannel(uint8 channel)
{
  halIntState_t  s;

  /* illegal channel */
#if (MAC_CHAN_LOWEST == 0)
  /* No need to check MAC_CHAN_LOWEST if it is zero */
  MAC_ASSERT(channel <= MAC_CHAN_HIGHEST);
#else
  MAC_ASSERT((channel >= MAC_CHAN_LOWEST) && (channel <= MAC_CHAN_HIGHEST));
#endif

  /* critical section to make sure transmit does not start while updating channel */
  HAL_ENTER_CRITICAL_SECTION(s);

  /* set requested channel */
  reqChannel = channel;

  /*
   *  If transmit is not active, update the radio hardware immediately.  If transmit is active,
   *  the channel will be updated at the end of the current transmit.
   */
    if (!(MAC_TX_IS_PHYSICALLY_ACTIVE()))
    {
      HAL_EXIT_CRITICAL_SECTION(s);
      return macRadioUpdateChannel();
    }

  HAL_EXIT_CRITICAL_SECTION(s);

  return TRUE;
}

MAC_INTERNAL_API bool macRadioSetChannelRx(uint8 channel)
{
  halIntState_t  s;

  /* illegal channel */
#if (MAC_CHAN_LOWEST == 0)
  /* No need to check MAC_CHAN_LOWEST if it is zero */
  MAC_ASSERT(channel <= MAC_CHAN_HIGHEST);
#else
  MAC_ASSERT((channel >= MAC_CHAN_LOWEST) && (channel <= MAC_CHAN_HIGHEST));
#endif

  /* critical section to make sure transmit does not start while updating channel */
  HAL_ENTER_CRITICAL_SECTION(s);

  /* set requested channel */
  reqChannel = channel;

  /*
   *  If transmit is not active, update the radio hardware immediately.  If transmit is active,
   *  the channel will be updated at the end of the current transmit.
   */
    if (!(MAC_TX_IS_PHYSICALLY_ACTIVE()))
    {
      HAL_EXIT_CRITICAL_SECTION(s);
      return macRadioUpdateChannelRx();
    }

  HAL_EXIT_CRITICAL_SECTION(s);

  return TRUE;
}
/**************************************************************************************************
 * @fn          macRadioUpdateChannel
 *
 * @brief       Update the radio channel if a new channel has been requested.
 *
 * @param       none
 *
 * @return      TRUE if successful, FALSE otherwise
 **************************************************************************************************
 */
MAC_INTERNAL_API bool macRadioUpdateChannel(void)
{
  halIntState_t  s;

  MAC_ASSERT(!(MAC_TX_IS_PHYSICALLY_ACTIVE()));
  /* if the channel has changed or a scanning is in process,
     set the radio to the new channel */
  HAL_ENTER_CRITICAL_SECTION(s);
  if ((reqChannel != macPhyChannel) ||
      (macRxEnableFlags & MAC_RX_SCAN))
  {
    /* Changing the channel stops any receive in progress.
     * Also note that this RX off and on sequence must be
     * executed for the coordinator since the radio may be
     * waiting for the prior RX on a different channel.
     * If the sequence is not executed, the radio will remain
     * stuck on the prior channel and nothing will be received.
     */
    HAL_EXIT_CRITICAL_SECTION(s);

    macRxOff();

    DBG_PRINT1(DBGSYS, "MAC_RADIO_SET_CHANNEL(%u)", reqChannel);

    if (MAC_RADIO_SET_CHANNEL(reqChannel) == TRUE)
    {
      if(macRadioNeedAdjustPower(reqChannel, macPhyChannel))
      {
          macPhyChannel = reqChannel;
          macRadioSetTxPower(macPib.phyTransmitPower);
      }
      else
      {
          macPhyChannel = reqChannel;
      }
    }

    /* Request RX to be on */
    return macRxOnRequest();
  }

  HAL_EXIT_CRITICAL_SECTION(s);

  return TRUE;
}

MAC_INTERNAL_API bool macRadioUpdateChannelRx(void)
{
  halIntState_t  s;

  MAC_ASSERT(!(MAC_TX_IS_PHYSICALLY_ACTIVE()));
  /* if the channel has changed or a scanning is in process,
     set the radio to the new channel */
  HAL_ENTER_CRITICAL_SECTION(s);
  if ((reqChannel != macPhyChannel) ||
      (macRxEnableFlags & MAC_RX_SCAN))
  {
    /* Changing the channel stops any receive in progress.
     * Also note that this RX off and on sequence must be
     * executed for the coordinator since the radio may be
     * waiting for the prior RX on a different channel.
     * If the sequence is not executed, the radio will remain
     * stuck on the prior channel and nothing will be received.
     */
    HAL_EXIT_CRITICAL_SECTION(s);

    macRxOff();

    DBG_PRINT1(DBGSYS, "MAC_RADIO_SET_CHANNEL(%u)", reqChannel);

    if (MAC_RADIO_SET_CHANNEL(reqChannel) == TRUE)
    {
        if(macRadioNeedAdjustPower(reqChannel, macPhyChannel))
        {
            macPhyChannel = reqChannel;
            macRadioSetTxPower(macPib.phyTransmitPower);
        }
        else
        {
            macPhyChannel = reqChannel;
        }
    }
  }
  else
  {
    HAL_EXIT_CRITICAL_SECTION(s);
  }

  /* Request RX to be on */
  macRxOnRequest();

  return TRUE;
}

/**************************************************************************************************
 * @fn          macRadioStartScan
 *
 * @brief       Puts radio into selected scan mode.
 *
 * @param       scanMode - scan mode, see #defines in .h file
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macRadioStartScan(uint8 scanMode)
{
  MAC_ASSERT(macRxFilter == RX_FILTER_OFF); /* all filtering must be off to start scan */

  /* Save the current local PAN ID before any scan */
  macSavedPanID = pMacPib->panId;

  /* set the receive filter based on the selected scan mode */
  if (scanMode == MAC_SCAN_ED)
  {
    macRxFilter = RX_FILTER_ALL;
  }
  else if (scanMode == MAC_SCAN_ORPHAN)
  {
    /* if Energy detection ran before, then the radio is setup to do
     * CMD_IEEE_ED_SCAN. Setup radio to default of CMD_IEEE_RX instead.
     */
    macSetupReceiveCmd();
    macRxFilter = RX_FILTER_NON_COMMAND_FRAMES;
  }
  else
  {
    MAC_ASSERT((scanMode == MAC_SCAN_ACTIVE_ENHANCED) || (scanMode == MAC_SCAN_ACTIVE) ||
               (scanMode == MAC_SCAN_PASSIVE)); /* invalid scan type */

    macRxFilter = RX_FILTER_NON_BEACON_FRAMES;

    /* if Energy detection ran before, then the radio is setup to do
     * CMD_IEEE_ED_SCAN. Setup radio to default of CMD_IEEE_RX instead.
     */
    macSetupReceiveCmd();

    /* for active and passive scans, per spec the pan ID must be 0xFFFF */
    pMacPib->panId = 0xFFFF;
    MAC_RADIO_SET_PAN_ID(pMacPib->panId);
  }
}


/**************************************************************************************************
 * @fn          macRadioStopScan
 *
 * @brief       Takes radio out of scan mode.  Note can be called if
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macRadioStopScan(void)
{
  macRxFilter = RX_FILTER_OFF;

  /* restore the pan ID (passive and active scans set pan ID to 0xFFFF) */
  pMacPib->panId = macSavedPanID;
  MAC_RADIO_SET_PAN_ID(pMacPib->panId);
}


/**************************************************************************************************
 * @fn          macRadioEnergyDetectStop
 *
 * @brief       Called at completion of an energy detect.  Note: can be called even if energy
 *              detect is already stopped (needed by reset).
 *
 * @param       none
 *
 * @return      highest energy detect measurement
 **************************************************************************************************
 */
uint8 macRadioEnergyDetectStop(void)
{
  int8 rssiDbm;
  uint8 energyDetectMeasurement;

  rssiDbm = RF_getRssi(RF_handle);

  energyDetectMeasurement = radioComputeED(rssiDbm);

  DBG_PRINT1(DBGSYS, "ED: normalized energy level = %u", energyDetectMeasurement);

  return(energyDetectMeasurement);
}

/*=================================================================================================
 * @fn          radioComputeED
 *
 * @brief       Compute energy detect measurement.
 *
 * @param       rssi - raw RSSI value from radio hardware
 *
 * @return      energy detect measurement in the range of 0x00-0xFF
 *=================================================================================================
 */
uint8 radioComputeED(int8 rssiDbm)
{
  uint8 ed;
  int8 min, max;
  /*
   *  Keep RF power between minimum and maximum values.
   *  This min/max range is derived from datasheet and specification.
   */
  min = ED_RF_POWER_MIN_DBM;
  max = ED_RF_POWER_MAX_DBM;


  if (rssiDbm < min)
  {
    rssiDbm = min;
  }
  else if (rssiDbm > max)
  {
    rssiDbm = max;
  }

  /*
   *  Create energy detect measurement by normalizing and scaling RF power level.
   *
   *  Note : The division operation below is designed for maximum accuracy and
   *         best granularity.  This is done by grouping the math operations to
   *         compute the entire numerator before doing any division.
   */
  ed = (MAC_SPEC_ED_MAX * (rssiDbm - ED_RF_POWER_MIN_DBM)) / (ED_RF_POWER_MAX_DBM - ED_RF_POWER_MIN_DBM);
  return(ed);
}


/**************************************************************************************************
 * @fn          macRadioComputeLQI
 *
 * @brief       Compute link quality indication.
 *
 * @param       rssi - raw RSSI value from radio hardware
 *              corr - correlation value from radio hardware
 *
 * @return      link quality indicator value
 **************************************************************************************************
 */
MAC_INTERNAL_API uint8 macRadioComputeLQI(int8 rssiDbm, uint8 corr)
{
  (void) corr; /* suppress compiler warning of unused parameter */

  /*
   *  Note : Currently the LQI value is simply the energy detect measurement.
   *         A more accurate value could be derived by using the correlation
   *         value along with the RSSI value.
   */
  return(radioComputeED(rssiDbm));
}


/**************************************************************************************************
 * @fn          macRadioPowerDown
 *
 * @brief       Power down the radio by CM0 by sending an RF_yield command.
 *
 * @param       bypassRAT - Indicates if RAT disabling needs to be avoided
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macRadioPowerDown(bool bypassRAT)
{
}


/**************************************************************************************************
 * @fn          macRadioPowerUp
 *
 * @brief       Start the radio by sending a NOP command.
 *
 * @param       bypassRAT - Indicates if RAT restoration needs to be avoided
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macRadioPowerUp(bool bypassRAT)
{
}
