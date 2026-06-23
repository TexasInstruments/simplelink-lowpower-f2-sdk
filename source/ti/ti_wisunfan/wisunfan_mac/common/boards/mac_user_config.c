/******************************************************************************

 @file  mac_user_config.c

 @brief User configurable variables for the TIMAC radio.

 Group: WCS LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2016-2026, Texas Instruments Incorporated
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions
 are met:

 *  Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.

 *  Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.

 *  Neither the name of Texas Instruments Incorporated nor the names of
    its contributors may be used to endorse or promote products derived
    from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 ******************************************************************************
 
 
 *****************************************************************************/

/******************************************************************************
 * INCLUDES
 */

#if (defined(USE_DMM) && !defined(SYSCFG))
#include "Board.h"
#else
#include "ti_drivers_config.h"
#endif
#include "api_mac.h"
#include "ti_wisunfan_config.h"
#include "ti_radio_config.h"
#include "mac_user_config.h"
#if defined(FEATURE_MAC_SECURITY) || defined(ZSTACK_SECURITY)
#include <ti/drivers/AESCCM.h>
#endif
#if defined(FEATURE_SECURE_COMMISSIONING) || defined(USE_DMM) || defined(ZSTACK_SECURITY)
#include <ti/drivers/AESECB.h>
#endif
#include <ti/drivers/cryptoutils/cryptokey/CryptoKeyPlaintext.h>
/******************************************************************************
 * MACROS
 */

/******************************************************************************
 * CONSTANTS
 */
#define RF_CONFIG_IEEE_PL_FIRST_CHANNEL     11
#define RF_CONFIG_IEEE_PL_NO_CHANNELS       16
/******************************************************************************
 * TYPEDEFS
 */

/******************************************************************************
 * LOCAL VARIABLES
 */

/******************************************************************************
 * GLOBAL VARIABLES
 */
int8_t PLTableIeee[RF_CONFIG_IEEE_PL_NO_CHANNELS] =
{
     20,    //20dBm max for Channel = 11
     20,    //20dBm max for Channel = 12
     20,    //20dBm max for Channel = 13
     20,    //20dBm max for Channel = 14
     20,    //20dBm max for Channel = 15
     20,    //20dBm max for Channel = 16
     20,    //20dBm max for Channel = 17
     20,    //20dBm max for Channel = 18
     20,    //20dBm max for Channel = 19
     20,    //20dBm max for Channel = 20
     20,    //20dBm max for Channel = 21
     20,    //20dBm max for Channel = 22
     20,    //20dBm max for Channel = 23
     20,    //20dBm max for Channel = 24
     19,    //19dBm max for Channel = 25
     15,    //15dBm max for Channel = 26
};

macRfCfg_maxPower_t maxPowerIeee =
{
    RF_CONFIG_IEEE_PL_FIRST_CHANNEL,
    RF_CONFIG_IEEE_PL_NO_CHANNELS,
    PLTableIeee
};

// RF Driver API Table
const uint32_t macRfDriverTable[] =
  { (uint32_t) RF_open,
    (uint32_t) RF_close,
    (uint32_t) NULL,
    (uint32_t) NULL,
    (uint32_t) NULL,
    (uint32_t) RF_cancelCmd,
    (uint32_t) RF_flushCmd,
    (uint32_t) RF_yield,
    (uint32_t) RF_Params_init,
    (uint32_t) RF_runImmediateCmd,
    (uint32_t) RF_runDirectCmd,
    (uint32_t) RF_ratCompare,
    (uint32_t) NULL,
    (uint32_t) RF_RatConfigOutput_init,
    (uint32_t) RF_ratDisableChannel,
    (uint32_t) RF_getCurrentTime,
    (uint32_t) RF_getRssi,
    (uint32_t) RF_getInfo,
    (uint32_t) RF_getCmdOp,
    (uint32_t) RF_control,
    (uint32_t) RF_scheduleCmd,
    (uint32_t) RF_getTxPower,
    (uint32_t) RF_setTxPower,
    (uint32_t) NULL,
    (uint32_t) RF_TxPowerTable_findValue,
    (uint32_t) RF_ScheduleCmdParams_init,
    (uint32_t) RF_requestAccess };

// Crypto Driver API Table
#ifdef CC13X0_SUPPORT
const uint32_t macCryptoDriverTable[] =
  { (uint32_t) CryptoCC26XX_close,
    (uint32_t) CryptoCC26XX_init,
    (uint32_t) CryptoCC26XX_open,
    (uint32_t) CryptoCC26XX_Params_init,
    (uint32_t) CryptoCC26XX_Transac_init,
    (uint32_t) CryptoCC26XX_allocateKey,
    (uint32_t) CryptoCC26XX_releaseKey,
    (uint32_t) CryptoCC26XX_transact,
    (uint32_t) CryptoCC26XX_transactPolling,
    (uint32_t) CryptoCC26XX_transactCallback };
#else

const uint32_t macCryptoDriverTable[] =
{
#if defined(FEATURE_MAC_SECURITY) || defined(ZSTACK_SECURITY)
      (uint32_t) AESCCM_init,
      (uint32_t) AESCCM_open,
      (uint32_t) AESCCM_close,
      (uint32_t) AESCCM_Params_init,
      (uint32_t) AESCCM_Operation_init,
      (uint32_t) AESCCM_oneStepEncrypt,
      (uint32_t) AESCCM_oneStepDecrypt,
#else
      (uint32_t) NULL,
      (uint32_t) NULL,
      (uint32_t) NULL,
      (uint32_t) NULL,
      (uint32_t) NULL,
      (uint32_t) NULL,
      (uint32_t) NULL,
#endif
#if defined(FEATURE_SECURE_COMMISSIONING) || defined(USE_DMM) || defined(ZSTACK_SECURITY)
      (uint32_t) AESECB_init,
      (uint32_t) AESECB_open,
      (uint32_t) AESECB_close,
      (uint32_t) AESECB_Params_init,
      (uint32_t) AESECB_Operation_init,
      (uint32_t) AESECB_oneStepEncrypt,
      (uint32_t) AESECB_oneStepDecrypt,
#else
      (uint32_t) NULL,
      (uint32_t) NULL,
      (uint32_t) NULL,
      (uint32_t) NULL,
      (uint32_t) NULL,
      (uint32_t) NULL,
      (uint32_t) NULL,
#endif
      (uint32_t) CryptoKeyPlaintext_initKey,
      (uint32_t) CryptoKeyPlaintext_initBlankKey
  };
#endif

void rfSelect(uint16_t phyId)
{
    macRfCfg_prop_t RfCfg1a = RF_CONFIG_50KBPS_915MHZ_PHY_1;
    macRfCfg_prop_t RfCfg1b = RF_CONFIG_50KBPS_915MHZ_PHY_2;
    macRfCfg_prop_t RfCfg2a = RF_CONFIG_100KBPS_915MHZ_PHY_3;
    macRfCfg_prop_t RfCfg2b = RF_CONFIG_100KBPS_915MHZ_PHY_4;
    macRfCfg_prop_t RfCfg3 = RF_CONFIG_150KBPS_915MHZ_PHY_5;
    macRfCfg_prop_t RfCfg4a = RF_CONFIG_200KBPS_915MHZ_PHY_6;
    macRfCfg_prop_t RfCfg4b = RF_CONFIG_200KBPS_915MHZ_PHY_7;
    macRfCfg_prop_t RfCfg5 = RF_CONFIG_300KBPS_915MHZ_PHY_8;

    macRfCfg_prop_t *pRfCfg;
    switch(phyId)
    {
    case APIMAC_50KBPS_1a:
        pRfCfg = &RfCfg1a;
        break;
    case APIMAC_50KBPS_1b:
        pRfCfg = &RfCfg1b;
        break;
    case APIMAC_100KBPS_2a:
        pRfCfg = &RfCfg2a;
        break;
    case APIMAC_100KBPS_2b:
        pRfCfg = &RfCfg2b;
        break;
    case APIMAC_150KBPS_3:;
        pRfCfg = &RfCfg3;
        break;
    case APIMAC_200KBPS_4a:
        pRfCfg = &RfCfg4a;
        break;
    case APIMAC_200KBPS_4b:
        pRfCfg = &RfCfg4b;
        break;
    case APIMAC_300KBPS_5:
        pRfCfg = &RfCfg5;
        break;
    default:
        pRfCfg = &RfCfg1b;
        break;
    }
    rfSetConfigSubG(pRfCfg);
}
/******************************************************************************
 */

