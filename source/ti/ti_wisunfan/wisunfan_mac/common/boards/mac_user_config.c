/******************************************************************************

 @file  mac_user_config.c

 @brief User configurable variables for the TIMAC radio.

 Group: WCS LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2016-2025, Texas Instruments Incorporated
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
#if !defined(DeviceFamily_CC13X2) && !defined(DeviceFamily_CC26X2) && !defined(DeviceFamily_CC13X2X7) && !defined(DeviceFamily_CC13X4)
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

/* CC1352R1/CC1352P1/CC1352P_2/CC1352P7_1/CC1354R10/CC1354P10 Configuration */
#if defined(LAUNCHXL_CC1352R1) || defined(LAUNCHXL_CC1352P1) \
    || defined(LAUNCHXL_CC1352P_2) || defined(LP_CC1352P7_1) \
    || defined (LP_CC1354R10_RGZ) || defined (LP_EM_CC1354P10_1) \
    || defined (LP_EM_CC1354P10_6) || defined (LP_CC1354P10_1_RGZ) \
    || defined (LP_CC1354P10_6_RGZ)
#if defined(COPROCESSOR)
void rfSelect(uint16_t phyId)
{
    macRfCfg_ieee_t RfCfg0 = RF_CONFIG_250KBPS_IEEE_PHY_0;
    macRfCfg_prop_t RfCfg1 = RF_CONFIG_50KBPS_915MHZ_PHY_1;
    macRfCfg_prop_t RfCfg3 = RF_CONFIG_50KBPS_868MHZ_PHY_3;
    macRfCfg_prop_t RfCfg129 = RF_CONFIG_5KBPS_915MHZ_PHY_129;
    macRfCfg_prop_t RfCfg131 = RF_CONFIG_5KBPS_868MHZ_PHY_131;
    macRfCfg_prop_t RfCfg132 = RF_CONFIG_200KBPS_915MHZ_PHY_132;
    macRfCfg_prop_t RfCfg133 = RF_CONFIG_200KBPS_868MHZ_PHY_133;
    macRfCfg_prop_t *pRfCfg;
    if(phyId == APIMAC_250KBPS_IEEE_PHY_0)
    {
#if defined(LAUNCHXL_CC1352P_2)
        RfCfg0.data = (void *)&maxPowerIeee;
#endif
        rfSetConfigIeee(&RfCfg0);
    }
    else
    {
        switch(phyId)
        {
        case APIMAC_50KBPS_915MHZ_PHY_1:
            pRfCfg = &RfCfg1;
            break;
        case APIMAC_50KBPS_868MHZ_PHY_3:
            pRfCfg = &RfCfg3;
            break;
        case APIMAC_5KBPS_915MHZ_PHY_129:
            pRfCfg = &RfCfg129;
            break;
        case APIMAC_5KBPS_868MHZ_PHY_131:
            pRfCfg = &RfCfg131;
            break;
        case APIMAC_200KBPS_915MHZ_PHY_132:
            pRfCfg = &RfCfg132;
            break;
        case APIMAC_200KBPS_868MHZ_PHY_133:
            pRfCfg = &RfCfg133;
            break;
        default:
            pRfCfg = &RfCfg1;
            break;
        }
        rfSetConfigSubG(pRfCfg);
    }
}
#else
void rfSelect(uint16_t phyId)
{
    (void)phyId;
#if (CONFIG_PHY_ID == APIMAC_250KBPS_IEEE_PHY_0)
    macRfCfg_ieee_t RfCfg = RF_CONFIG_250KBPS_IEEE_PHY_0;
#if defined(LAUNCHXL_CC1352P_2)
    RfCfg.data = (void *)&maxPowerIeee;
#endif
    rfSetConfigIeee(&RfCfg);
#else
#if (CONFIG_PHY_ID == APIMAC_50KBPS_1a)
    macRfCfg_prop_t RfCfg = RF_CONFIG_50KBPS_915MHZ_PHY_1;
/* #endif for CONFIG_CHANNEL_PLAN_ID */
#elif (CONFIG_PHY_ID == APIMAC_50KBPS_1b)
    macRfCfg_prop_t RfCfg = RF_CONFIG_50KBPS_915MHZ_PHY_2;
/* #endif for CONFIG_CHANNEL_PLAN_ID */
#elif (CONFIG_PHY_ID == APIMAC_100KBPS_2a)
    macRfCfg_prop_t RfCfg = RF_CONFIG_100KBPS_915MHZ_PHY_3;
/* #endif for CONFIG_CHANNEL_PLAN_ID */
#elif (CONFIG_PHY_ID == APIMAC_100KBPS_2b)
    macRfCfg_prop_t RfCfg = RF_CONFIG_100KBPS_915MHZ_PHY_4;
/* #endif for CONFIG_CHANNEL_PLAN_ID */
#elif (CONFIG_PHY_ID == APIMAC_150KBPS_3)
    macRfCfg_prop_t RfCfg = RF_CONFIG_150KBPS_915MHZ_PHY_5;
#elif (CONFIG_PHY_ID == APIMAC_200KBPS_4a)
    macRfCfg_prop_t RfCfg = RF_CONFIG_200KBPS_915MHZ_PHY_6;
#elif (CONFIG_PHY_ID == APIMAC_200KBPS_4b)
    macRfCfg_prop_t RfCfg = RF_CONFIG_200KBPS_915MHZ_PHY_7;
#elif (CONFIG_PHY_ID == APIMAC_300KBPS_5)
    macRfCfg_prop_t RfCfg = RF_CONFIG_300KBPS_915MHZ_PHY_8;
#else
    macRfCfg_prop_t RfCfg = RF_CONFIG_50KBPS_915MHZ_PHY_1;
#endif
    rfSetConfigSubG(&RfCfg);
#endif
}
#endif
#endif
/* end of CC1352R1/CC1352P1/CC1352P_2 Configuration */

/* CC1352P_4 Configuration */
#if defined(LAUNCHXL_CC1352P_4)
#if defined(COPROCESSOR)
void rfSelect(uint16_t phyId)
{
    macRfCfg_ieee_t RfCfg0 = RF_CONFIG_250KBPS_IEEE_PHY_0;
    macRfCfg_prop_t RfCfg128 = RF_CONFIG_50KBPS_433MHZ_PHY_128;
    macRfCfg_prop_t RfCfg130 = RF_CONFIG_5KBPS_433MHZ_PHY_130;
    macRfCfg_prop_t *pRfCfg;
    if(phyId == APIMAC_250KBPS_IEEE_PHY_0)
    {
        rfSetConfigIeee(&RfCfg0);
    }
    else
    {
        switch(phyId)
        {
        case APIMAC_50KBPS_433MHZ_PHY_128:
            pRfCfg = &RfCfg128;
            break;
        case APIMAC_5KBPS_433MHZ_PHY_130:
            pRfCfg = &RfCfg130;
            break;
        default:
            pRfCfg = &RfCfg128;
            break;
        }
        rfSetConfigSubG(pRfCfg);
    }
}
#else
void rfSelect(uint16_t phyId)
{
    (void)phyId;
#if (CONFIG_PHY_ID == APIMAC_250KBPS_IEEE_PHY_0)
    macRfCfg_ieee_t RfCfg = RF_CONFIG_250KBPS_IEEE_PHY_0;
    rfSetConfigIeee(&RfCfg);
#else
#if (CONFIG_PHY_ID == APIMAC_50KBPS_433MHZ_PHY_128)
    macRfCfg_prop_t RfCfg = RF_CONFIG_50KBPS_433MHZ_PHY_128;
#elif (CONFIG_PHY_ID == APIMAC_5KBPS_433MHZ_PHY_130)
    macRfCfg_prop_t RfCfg = RF_CONFIG_5KBPS_433MHZ_PHY_130;
#else
    macRfCfg_prop_t RfCfg = RF_CONFIG_50KBPS_433MHZ_PHY_128;
#endif
    rfSetConfigSubG(&RfCfg);
#endif
}
#endif
#endif
/* end of CC1352P_4 Configuration */

/* CC1312R1 Configuration */
#if defined(LAUNCHXL_CC1312R1) || defined(LP_CC1312R7) \
    || defined (LP_EM_CC1314R10) || defined (LP_CC1314R10_RGZ) || defined(LP_EM_CC1312PSIP)
#if defined(COPROCESSOR)
void rfSelect(uint16_t phyId)
{
    macRfCfg_prop_t RfCfg1 = RF_CONFIG_50KBPS_915MHZ_PHY_1;
    macRfCfg_prop_t RfCfg3 = RF_CONFIG_50KBPS_868MHZ_PHY_3;
    macRfCfg_prop_t RfCfg129 = RF_CONFIG_5KBPS_915MHZ_PHY_129;
    macRfCfg_prop_t RfCfg131 = RF_CONFIG_5KBPS_868MHZ_PHY_131;
    macRfCfg_prop_t RfCfg132 = RF_CONFIG_200KBPS_915MHZ_PHY_132;
    macRfCfg_prop_t RfCfg133 = RF_CONFIG_200KBPS_868MHZ_PHY_133;
    macRfCfg_prop_t *pRfCfg;
    switch(phyId)
    {
    case APIMAC_50KBPS_915MHZ_PHY_1:
        pRfCfg = &RfCfg1;
        break;
    case APIMAC_50KBPS_868MHZ_PHY_3:
        pRfCfg = &RfCfg3;
        break;
    case APIMAC_5KBPS_915MHZ_PHY_129:
        pRfCfg = &RfCfg129;
        break;
    case APIMAC_5KBPS_868MHZ_PHY_131:
        pRfCfg = &RfCfg131;
        break;
    case APIMAC_200KBPS_915MHZ_PHY_132:;
        pRfCfg = &RfCfg132;
        break;
    case APIMAC_200KBPS_868MHZ_PHY_133:
        pRfCfg = &RfCfg133;
        break;
    default:
        pRfCfg = &RfCfg1;
        break;
    }
    rfSetConfigSubG(pRfCfg);
}
#else
void rfSelect(uint16_t phyId)
{
    (void)phyId;

#if (CONFIG_PHY_ID == APIMAC_50KBPS_1a)
    macRfCfg_prop_t RfCfg = RF_CONFIG_50KBPS_915MHZ_PHY_1;
/* #endif for CONFIG_CHANNEL_PLAN_ID */
#elif (CONFIG_PHY_ID == APIMAC_50KBPS_1b)
    macRfCfg_prop_t RfCfg = RF_CONFIG_50KBPS_915MHZ_PHY_2;
/* #endif for CONFIG_CHANNEL_PLAN_ID */
#elif (CONFIG_PHY_ID == APIMAC_100KBPS_2a)
    macRfCfg_prop_t RfCfg = RF_CONFIG_100KBPS_915MHZ_PHY_3;
/* #endif for CONFIG_CHANNEL_PLAN_ID */
#elif (CONFIG_PHY_ID == APIMAC_100KBPS_2b)
    macRfCfg_prop_t RfCfg = RF_CONFIG_100KBPS_915MHZ_PHY_4;
/* #endif for CONFIG_CHANNEL_PLAN_ID */
#elif (CONFIG_PHY_ID == APIMAC_150KBPS_3)
    macRfCfg_prop_t RfCfg = RF_CONFIG_150KBPS_915MHZ_PHY_5;
#elif (CONFIG_PHY_ID == APIMAC_200KBPS_4a)
    macRfCfg_prop_t RfCfg = RF_CONFIG_200KBPS_915MHZ_PHY_6;
#elif (CONFIG_PHY_ID == APIMAC_200KBPS_4b)
    macRfCfg_prop_t RfCfg = RF_CONFIG_200KBPS_915MHZ_PHY_7;
#elif (CONFIG_PHY_ID == APIMAC_300KBPS_5)
    macRfCfg_prop_t RfCfg = RF_CONFIG_300KBPS_915MHZ_PHY_8;
#endif
    rfSetConfigSubG(&RfCfg);
}
#endif
#endif
/* end of CC1312R1 Configuration */

/* CC26X2R1/CC2652RB Configuration */
#if defined(LAUNCHXL_CC26X2R1) || defined(LAUNCHXL_CC2652RB)
void rfSelect(uint16_t phyId)
{
    (void)phyId;
    macRfCfg_ieee_t RfCfg = RF_CONFIG_250KBPS_IEEE_PHY_0;
    rfSetConfigIeee(&RfCfg);
}
#endif
/* end of CC26X2R1/CC2652RB Configuration */
/******************************************************************************
 */

