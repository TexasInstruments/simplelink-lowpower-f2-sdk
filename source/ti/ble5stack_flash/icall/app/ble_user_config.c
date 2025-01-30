/******************************************************************************

 @file  ble_user_config.c

 @brief This file contains user configurable variables for the BLE
        Application.

 Group: WCS, BTS
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

/*******************************************************************************
 * INCLUDES
 */
#include "hal_types.h"
#include "ble_user_config.h"
#include "hci.h"
#include <ti/drivers/AESCCM.h>
#include <ti/drivers/AESECB.h>
#include <ti/drivers/cryptoutils/cryptokey/CryptoKeyPlaintext.h>
#include <ti/drivers/utils/Random.h>

#if !defined(CC23X0) && !defined(CC33xx)
#include "ble_overrides.h"
#include "ecc/ECCROMCC26XX.h"
#endif // !defined(CC23X0) && !defined(CC33xx)

#ifndef CC23X0
#include "ti_radio_config.h"
#include <ti/drivers/rf/RF.h>
#include <ti/drivers/aesccm/AESCCMCC26XX.h>
#include <ti/drivers/aesecb/AESECBCC26XX.h>
#include <ti/drivers/TRNG.h>
#else
#include <ti/drivers/aesccm/AESCCMLPF3.h>
#include <ti/drivers/aesecb/AESECBLPF3.h>
#include <ti/drivers/RNG.h>
#endif // CC23X0

#ifdef SYSCFG
#include "ti_ble_config.h"
#else
#ifndef CONTROLLER_ONLY
#include <gapbondmgr.h>
#endif // CONTROLLER_ONLY
#endif // SYSCFG

#if defined(FREERTOS) || defined(CC33xx)
#define Swi_restore SwiP_restore
#define Swi_disable SwiP_disable
#include <ti/drivers/dpl/SwiP.h>
#else
#include <ti/sysbios/knl/Swi.h>
#include <ti/sysbios/BIOS.h>
#endif // FREERTOS || CC33xx

#ifndef CC23X0
#if !defined(DeviceFamily_CC26X1)
#include <driverlib/pka.h>
#if !defined(DeviceFamily_CC13X4) && !defined(DeviceFamily_CC26X4)
#include <driverlib/rf_bt5_coex.h>
#endif // !defined(DeviceFamily_CC13X4) && !defined(DeviceFamily_CC26X4)
#else
#include <driverlib/rom_ecc.h>
#endif // !defined(DeviceFamily_CC26X1)
#endif // CC23X0

/*******************************************************************************
 * MACROS
 */

/*******************************************************************************
 * CONSTANTS
 */

#ifndef SYSCFG
#ifdef CC23X0
// Default Tx Power dBm value
#define DEFAULT_TX_POWER               0
#else
// Default Tx Power Index
#define DEFAULT_TX_POWER               HCI_EXT_TX_POWER_0_DBM
#endif // CC23X0
#endif // SYSCFG

// Tx Power
#define NUM_TX_POWER_VALUES (RF_BLE_TX_POWER_TABLE_SIZE - 1)

// Override NOP
#define OVERRIDE_NOP                   0xC0000001


/*******************************************************************************
 * TYPEDEFS
 */

/*******************************************************************************
 * LOCAL VARIABLES
 */

/*********************************************************************
 * LOCAL FUNCTIONS
 */

void driverTable_fnSpinlock(void);

/*******************************************************************************
 * GLOBAL VARIABLES
 */

#ifndef CC23X0
// Tx Power Table
txPwrTbl_t appTxPwrTbl = {
                           (txPwrVal_t*)RF_BLE_txPowerTable,
                           NUM_TX_POWER_VALUES,  // max
                           DEFAULT_TX_POWER};    // default
#else
// The Tx Power value
int8 defaultTxPowerDbm = DEFAULT_TX_POWER;
#endif

#if defined(CC13X2P) && defined(CC13X2P_2_LAUNCHXL)

// Tx Power Backoff Values (txPwrBackoff1MPhy,txPwrBackoff2MPhy ,txPwrBackoffCoded)
 const txPwrBackoffVal_t TxPowerBackoffTable[] =
   { {HCI_EXT_TX_POWER_P2_20_DBM,HCI_EXT_TX_POWER_P2_20_DBM,HCI_EXT_TX_POWER_P2_20_DBM},   //max tx power for channel 0
     {HCI_EXT_TX_POWER_P2_20_DBM,HCI_EXT_TX_POWER_P2_20_DBM,HCI_EXT_TX_POWER_P2_20_DBM},   //max tx power for channel 1
     {HCI_EXT_TX_POWER_P2_20_DBM,HCI_EXT_TX_POWER_P2_20_DBM,HCI_EXT_TX_POWER_P2_20_DBM},   //max tx power for channel 2
     {HCI_EXT_TX_POWER_P2_20_DBM,HCI_EXT_TX_POWER_P2_20_DBM,HCI_EXT_TX_POWER_P2_20_DBM},   //max tx power for channel 3
     {HCI_EXT_TX_POWER_P2_20_DBM,HCI_EXT_TX_POWER_P2_20_DBM,HCI_EXT_TX_POWER_P2_20_DBM},   //max tx power for channel 4
     {HCI_EXT_TX_POWER_P2_20_DBM,HCI_EXT_TX_POWER_P2_20_DBM,HCI_EXT_TX_POWER_P2_20_DBM},   //max tx power for channel 5
     {HCI_EXT_TX_POWER_P2_20_DBM,HCI_EXT_TX_POWER_P2_20_DBM,HCI_EXT_TX_POWER_P2_20_DBM},   //max tx power for channel 6
     {HCI_EXT_TX_POWER_P2_20_DBM,HCI_EXT_TX_POWER_P2_20_DBM,HCI_EXT_TX_POWER_P2_20_DBM},   //max tx power for channel 7
     {HCI_EXT_TX_POWER_P2_20_DBM,HCI_EXT_TX_POWER_P2_20_DBM,HCI_EXT_TX_POWER_P2_20_DBM},   //max tx power for channel 8
     {HCI_EXT_TX_POWER_P2_20_DBM,HCI_EXT_TX_POWER_P2_20_DBM,HCI_EXT_TX_POWER_P2_20_DBM},   //max tx power for channel 9
     {HCI_EXT_TX_POWER_P2_20_DBM,HCI_EXT_TX_POWER_P2_20_DBM,HCI_EXT_TX_POWER_P2_20_DBM},   //max tx power for channel 10
     {HCI_EXT_TX_POWER_P2_20_DBM,HCI_EXT_TX_POWER_P2_20_DBM,HCI_EXT_TX_POWER_P2_20_DBM},   //max tx power for channel 11
     {HCI_EXT_TX_POWER_P2_20_DBM,HCI_EXT_TX_POWER_P2_20_DBM,HCI_EXT_TX_POWER_P2_20_DBM},   //max tx power for channel 12
     {HCI_EXT_TX_POWER_P2_20_DBM,HCI_EXT_TX_POWER_P2_20_DBM,HCI_EXT_TX_POWER_P2_20_DBM},   //max tx power for channel 13
     {HCI_EXT_TX_POWER_P2_20_DBM,HCI_EXT_TX_POWER_P2_20_DBM,HCI_EXT_TX_POWER_P2_20_DBM},   //max tx power for channel 14
     {HCI_EXT_TX_POWER_P2_20_DBM,HCI_EXT_TX_POWER_P2_20_DBM,HCI_EXT_TX_POWER_P2_20_DBM},   //max tx power for channel 15
     {HCI_EXT_TX_POWER_P2_20_DBM,HCI_EXT_TX_POWER_P2_20_DBM,HCI_EXT_TX_POWER_P2_20_DBM},   //max tx power for channel 16
     {HCI_EXT_TX_POWER_P2_20_DBM,HCI_EXT_TX_POWER_P2_20_DBM,HCI_EXT_TX_POWER_P2_20_DBM},   //max tx power for channel 17
     {HCI_EXT_TX_POWER_P2_20_DBM,HCI_EXT_TX_POWER_P2_20_DBM,HCI_EXT_TX_POWER_P2_20_DBM},   //max tx power for channel 18
     {HCI_EXT_TX_POWER_P2_20_DBM,HCI_EXT_TX_POWER_P2_20_DBM,HCI_EXT_TX_POWER_P2_20_DBM},   //max tx power for channel 19
     {HCI_EXT_TX_POWER_P2_20_DBM,HCI_EXT_TX_POWER_P2_20_DBM,HCI_EXT_TX_POWER_P2_20_DBM},   //max tx power for channel 20
     {HCI_EXT_TX_POWER_P2_20_DBM,HCI_EXT_TX_POWER_P2_20_DBM,HCI_EXT_TX_POWER_P2_20_DBM},   //max tx power for channel 21
     {HCI_EXT_TX_POWER_P2_20_DBM,HCI_EXT_TX_POWER_P2_20_DBM,HCI_EXT_TX_POWER_P2_20_DBM},   //max tx power for channel 22
     {HCI_EXT_TX_POWER_P2_20_DBM,HCI_EXT_TX_POWER_P2_20_DBM,HCI_EXT_TX_POWER_P2_20_DBM},   //max tx power for channel 23
     {HCI_EXT_TX_POWER_P2_20_DBM,HCI_EXT_TX_POWER_P2_20_DBM,HCI_EXT_TX_POWER_P2_20_DBM},   //max tx power for channel 24
     {HCI_EXT_TX_POWER_P2_20_DBM,HCI_EXT_TX_POWER_P2_20_DBM,HCI_EXT_TX_POWER_P2_20_DBM},   //max tx power for channel 25
     {HCI_EXT_TX_POWER_P2_20_DBM,HCI_EXT_TX_POWER_P2_20_DBM,HCI_EXT_TX_POWER_P2_20_DBM},   //max tx power for channel 26
     {HCI_EXT_TX_POWER_P2_20_DBM,HCI_EXT_TX_POWER_P2_20_DBM,HCI_EXT_TX_POWER_P2_20_DBM},   //max tx power for channel 27
     {HCI_EXT_TX_POWER_P2_20_DBM,HCI_EXT_TX_POWER_P2_20_DBM,HCI_EXT_TX_POWER_P2_20_DBM},   //max tx power for channel 28
     {HCI_EXT_TX_POWER_P2_20_DBM,HCI_EXT_TX_POWER_P2_20_DBM,HCI_EXT_TX_POWER_P2_20_DBM},   //max tx power for channel 29
     {HCI_EXT_TX_POWER_P2_20_DBM,HCI_EXT_TX_POWER_P2_20_DBM,HCI_EXT_TX_POWER_P2_20_DBM},   //max tx power for channel 30
     {HCI_EXT_TX_POWER_P2_20_DBM,HCI_EXT_TX_POWER_P2_20_DBM,HCI_EXT_TX_POWER_P2_20_DBM},   //max tx power for channel 31
     {HCI_EXT_TX_POWER_P2_20_DBM,HCI_EXT_TX_POWER_P2_20_DBM,HCI_EXT_TX_POWER_P2_20_DBM},   //max tx power for channel 32
     {HCI_EXT_TX_POWER_P2_20_DBM,HCI_EXT_TX_POWER_P2_20_DBM,HCI_EXT_TX_POWER_P2_20_DBM},   //max tx power for channel 33
     {HCI_EXT_TX_POWER_P2_20_DBM,HCI_EXT_TX_POWER_P2_17_DBM_P4_9_DBM,HCI_EXT_TX_POWER_P2_20_DBM},    //max tx power for channel 34
     {HCI_EXT_TX_POWER_P2_20_DBM,HCI_EXT_TX_POWER_P2_18_DBM_P4_10_DBM,HCI_EXT_TX_POWER_P2_20_DBM},   //max tx power for channel 35
     {HCI_EXT_TX_POWER_P2_19_DBM,HCI_EXT_TX_POWER_P2_18_DBM_P4_10_DBM,HCI_EXT_TX_POWER_P2_20_DBM},   //max tx power for channel 36
     {HCI_EXT_TX_POWER_P2_20_DBM,HCI_EXT_TX_POWER_P2_20_DBM,HCI_EXT_TX_POWER_P2_20_DBM},   //max tx power for channel 37
     {HCI_EXT_TX_POWER_P2_20_DBM,HCI_EXT_TX_POWER_P2_20_DBM,HCI_EXT_TX_POWER_P2_20_DBM},   //max tx power for channel 38
     {HCI_EXT_TX_POWER_P2_14_DBM_P4_6_DBM,HCI_EXT_TX_POWER_P2_20_DBM,HCI_EXT_TX_POWER_P2_20_DBM}};  //max tx power for channel 39

txPwrBackoffTbl_t appTxPwrBackoffTbl = {TxPowerBackoffTable};

#endif // defined(CC13X2P) && defined(CC13X2P_2_LAUNCHXL)


// Antenna board configurations (example for a 12-antenna board)
// Maximum number of antennas
#define ANTENNA_TABLE_SIZE 12
// BitMask of all the relevant GPIOs which needed for the antennas
#define ANTENNA_IO_MASK  BV(27)|BV(28)|BV(29)|BV(30)

// Antenna GPIO configuration (should be adapted to the antenna board design)
antennaIOEntry_t antennaTbl[ANTENNA_TABLE_SIZE] = {
                                   0,                        // antenna 0 GPIO configuration (all GPIOs in ANTENNA_IO_MASK are LOW)
                                   BV(28),                   // antenna 1
                                   BV(29),                   // antenna 2
                                   BV(28) | BV(29),          // antenna 3
                                   BV(30),                   // antenna 4
                                   BV(28) | BV(30),          // antenna 5
                                   BV(27),                   // antenna 6
                                   BV(27) | BV(28),          // antenna 7
                                   BV(27) | BV(29),          // antenna 8
                                   BV(27) | BV(28) | BV(29), // antenna 9
                                   BV(27) | BV(30),          // antenna 10
                                   BV(27) | BV(28) | BV(30)  // antenna 11
};

#ifdef RTLS_CTE
// Antenna properties passes to the stack
cteAntProp_t  appCTEAntProp = {ANTENNA_IO_MASK,
                               ANTENNA_TABLE_SIZE,
                               antennaTbl};
#endif // RTLS_CTE

#ifdef CC23X0
ECCParams_CurveParams eccParams_NISTP256 = {
    .curveType      = ECCParams_CURVE_TYPE_SHORT_WEIERSTRASS_AN3,
    .length         = ECCParams_NISTP256_LENGTH,
    .prime          = ECC_NISTP256_prime.byte,
    .order          = ECC_NISTP256_order.byte,
    .a              = ECC_NISTP256_a.byte,
    .b              = ECC_NISTP256_b.byte,
    .generatorX     = ECC_NISTP256_generatorX.byte,
    .generatorY     = ECC_NISTP256_generatorY.byte,
    .cofactor       = 1
};
#elif !defined(DeviceFamily_CC26X1)
ECCParams_CurveParams eccParams_NISTP256 = {
    .curveType      = ECCParams_CURVE_TYPE_SHORT_WEIERSTRASS_AN3,
    .length         = NISTP256_PARAM_SIZE_BYTES,
    .prime          = NISTP256_prime.byte,
    .order          = NISTP256_order.byte,
    .a              = NISTP256_a.byte,
    .b              = NISTP256_b.byte,
    .generatorX     = NISTP256_generator.x.byte,
    .generatorY     = NISTP256_generator.y.byte,
    .cofactor       = 1
};
#else
ECCParams_CurveParams eccParams_NISTP256 = {
    .curveType      = ECCParams_CURVE_TYPE_SHORT_WEIERSTRASS_AN3,
    .length         = ECC_NISTP256_PARAM_LENGTH_BYTES,
    .prime          = ECC_NISTP256_prime.byte,
    .order          = ECC_NISTP256_order.byte,
    .a              = ECC_NISTP256_a.byte,
    .b              = ECC_NISTP256_b.byte,
    .generatorX     = ECC_NISTP256_generatorX.byte,
    .generatorY     = ECC_NISTP256_generatorY.byte,
    .cofactor       = 1
};
#endif // CC23X0

#if defined(USE_COEX)
// 2 structures generated by the sysconfig RF for coexistence configuration
extern rfCoreHal_bleCoExConfig_t coexConfig;
extern RF_CoexOverride_BLEUseCases coexConfigBle;
// from coexConfig struct we will get the coex enable and coex type (3 or 1 wire)
// from coexConfigBle struct we will get the priority and rx request per use case:
// initiator, connected, Broadcaster and Observer.
coexUseCaseConfig_t coexSysConfig = {
    &coexConfig,
    &coexConfigBle
};
#endif // USE_COEX

#ifdef ICALL_JT
#include <icall.h>

#ifndef CC23X0
// RF Driver API Table
rfDrvTblPtr_t rfDriverTableBLE[] =
  { (uint32)RF_open,
    (uint32)driverTable_fnSpinlock, // RF_close
#ifdef RF_SINGLEMODE
    (uint32)RF_postCmd,
#else // !RF_SINGLEMODE
    (uint32)driverTable_fnSpinlock, // RF_postCmd
#endif // RF_SINGLEMODE
    (uint32)driverTable_fnSpinlock, // RF_pendCmd
#ifdef RF_SINGLEMODE
    (uint32)RF_runCmd,
#else // !RF_SINGLEMODE
    (uint32)driverTable_fnSpinlock, // RF_runCmd
#endif // RF_SINGLEMODE
    (uint32)RF_cancelCmd,
    (uint32)RF_flushCmd,
    (uint32)driverTable_fnSpinlock, // RF_yield
    (uint32)RF_Params_init,
    (uint32)RF_runImmediateCmd,
    (uint32)RF_runDirectCmd,
    (uint32)RF_ratCompare,
    (uint32)driverTable_fnSpinlock, // RF_ratCapture
    (uint32)RF_ratDisableChannel,
    (uint32)RF_getCurrentTime,
    (uint32)RF_getRssi,
    (uint32)RF_getInfo,
    (uint32)RF_getCmdOp,
    (uint32)RF_control,
    (uint32)driverTable_fnSpinlock, // RF_getTxPower
    (uint32)RF_setTxPower, // RF_setTxPower
    (uint32)driverTable_fnSpinlock, // RF_TxPowerTable_findPowerLevel
    (uint32)driverTable_fnSpinlock, // RF_TxPowerTable_findValue
#ifndef RF_SINGLEMODE
    (uint32)RF_scheduleCmd,
    (uint32)RF_runScheduleCmd,
    (uint32)driverTable_fnSpinlock, // RF_requestAccess
#endif // !RF_SINGLEMODE
  };

#ifndef CC33xx
cryptoDrvTblPtr_t cryptoDriverTableBLE[] =
  { (uint32)AESCCM_init,
    (uint32)AESCCM_open,
    (uint32)AESCCM_close,
    (uint32)AESCCM_Params_init,
    (uint32)AESCCM_Operation_init,
    (uint32)AESCCM_oneStepEncrypt,
    (uint32)AESCCM_oneStepDecrypt,
    (uint32)AESECB_init,
    (uint32)AESECB_open,
    (uint32)AESECB_close,
    (uint32)AESECB_Params_init,
    (uint32)AESECB_Operation_init,
    (uint32)AESECB_oneStepEncrypt,
    (uint32)AESECB_oneStepDecrypt,
    (uint32)CryptoKeyPlaintext_initKey,
    (uint32)CryptoKeyPlaintext_initBlankKey};
#endif // CC33xx

// Swi APIs needed by BLE controller
rtosApiTblPtr_t rtosApiTable[] =
{
  (uint32_t) Swi_disable,
  (uint32_t) Swi_restore
};
#endif //CC23X0

// BLE Stack Configuration Structure
const stackSpecific_t bleStackConfig =
{
  .maxNumConns                          = MAX_NUM_BLE_CONNS,
  .maxNumPDUs                           = MAX_NUM_PDU,
  .maxPduSize                           = 0,
  .maxNumPSM                            = L2CAP_NUM_PSM,
  .maxNumCoChannels                     = L2CAP_NUM_CO_CHANNELS,
  .maxAcceptListElems                   = MAX_NUM_AL_ENTRIES,
  .maxResolvListElems                   = CFG_MAX_NUM_RL_ENTRIES,
  .pfnBMAlloc                           = &pfnBMAlloc,
  .pfnBMFree                            = &pfnBMFree,
#ifndef CC23X0
  .rfDriverParams.powerUpDurationMargin = RF_POWER_UP_DURATION_MARGIN,
  .rfDriverParams.inactivityTimeout     = RF_INACTIVITY_TIMEOUT,
  .rfDriverParams.powerUpDuration       = RF_POWER_UP_DURATION,
  .rfDriverParams.pErrCb                = &(RF_ERR_CB),
#endif //!CC23X0
  .eccParams                            = &eccParams_NISTP256,
  .fastStateUpdateCb                    = NULL,
  .bleStackType                         = 0,
#ifdef CC2652RB_LAUNCHXL
  .extStackSettings                     = EXTENDED_STACK_SETTINGS | CC2652RB_OVERRIDE_USED,
#elif defined(CC33xx)
  .extStackSettings                     = EXTENDED_STACK_SETTINGS | CENTRAL_GUARD_TIME_ENABLE,
#else
  .extStackSettings                     = EXTENDED_STACK_SETTINGS,
#endif
#ifndef CC23X0
  .maxNumCteBuffers                     = MAX_NUM_CTE_BUFS,
#endif
  .advReportIncChannel                  = ADV_RPT_INC_CHANNEL
};

uint16_t bleUserCfg_maxPduSize = MAX_PDU_SIZE;

#ifdef OSAL_SNV_EXTFLASH
const extflashDrvTblPtr_t extflashDriverTable[] =
{
  (uint32) ExtFlash_open,
  (uint32) ExtFlash_close,
  (uint32) ExtFlash_read,
  (uint32) ExtFlash_write,
  (uint32) ExtFlash_erase
};
#endif // OSAL_SNV_EXTFLASH

#ifdef SDAA_ENABLE
sdaaUsrCfg_t sdaaCfgTable =
{
  .rxWindowDuration        = SDAA_RX_WINDOW_DURATION,
  .txUsageTresh            = SDAA_MAX_THRESHOLD,
  .rssithreshold           = SDAA_RSSI_THRESHOLD,
  .numberofnoisysamples    = SDAA_MAX_ALLOWED_NOISY_SAMPLES,
  .blockingchanneltime     = SDAA_MAX_BLOCKED_CHANNEL_TIME,
  .constobservtime         = SDAA_CONST_OBSERV_TIME,
  .observationtime         = SDAA_OBSERVATION_TIME
};
#endif //SDAA_ENABLE

#ifndef CC23X0
// Table for Driver can be found in icall_user_config.c
// if a driver is not to be used, then the pointer should be set to NULL,
// for this example, this is done in ble_user_config.h
const drvTblPtr_t driverTable =
{
  .rfDrvTbl         = rfDriverTableBLE,
#ifndef CC33xx
  .eccDrvTbl        = eccDriverTable,
  .cryptoDrvTbl     = cryptoDriverTableBLE,
  .trngDrvTbl       = trngDriverTable,
#else
  .eccDrvTbl        = NULL,
  .cryptoDrvTbl     = NULL,
  .trngDrvTbl       = NULL,
#endif
  .rtosApiTbl       = rtosApiTable,
#ifndef CC33xx
  .nvintfStructPtr  = &nvintfFncStruct,
#else
  .nvintfStructPtr  = NULL,
#endif
#ifdef OSAL_SNV_EXTFLASH
  .extflashDrvTbl = extflashDriverTable,
#endif // OSAL_SNV_EXTFLASH
};
#endif // !CC23X0

const boardConfig_t boardConfig =
{
#ifndef CC23X0
#ifndef CC33xx
  .rfFeModeBias  = RF_FE_MODE_AND_BIAS,
  .rfRegTbl      = (regOverride_t*)pOverrides_bleCommon,
  .rfRegTbl1M    = (regOverride_t*)pOverrides_ble1Mbps,
  .rfRegTbl2M    = (regOverride_t*)pOverrides_ble2Mbps,
  .rfRegTblCoded = (regOverride_t*)pOverrides_bleCoded,
#else
  .rfFeModeBias  = 0,
  .rfRegTbl      = NULL,
  .rfRegTbl1M    = NULL,
  .rfRegTbl2M    = NULL,
  .rfRegTblCoded = NULL,
#endif // CC33xx
  .txPwrTbl      = &appTxPwrTbl,
  /* The define EM_CC1354P10_1_LP is needed since it is High PA device for
     other stacks (not for BLE) and thus needed to be defined */
#if defined(CC13X2P) || defined(EM_CC1354P10_1_LP) || defined(CC2674R10_RGZ_LP) || defined(CC2674P10_RGZ_LP)
#if defined(CC13X2P_2_LAUNCHXL)
  .txPwrBackoffTbl = &appTxPwrBackoffTbl,
#else
  .txPwrBackoffTbl = NULL,
#endif // defined(CC13X2P_2_LAUNCHXL)
/* The define EM_CC1354P10_1_LP is needed since it is High PA device for
   other stacks (not for BLE) and thus needed to be defined */
#if defined(EM_CC1354P10_1_LP) || defined(CC33xx) || defined(CC2674R10_RGZ_LP)
  .rfRegOverrideTxStdTblptr  = NULL,
  .rfRegOverrideTx20TblPtr   = NULL,
#else
  .rfRegOverrideTxStdTblptr  = (regOverride_t*)pOverrides_bleTxStd,   // Default PA
  .rfRegOverrideTx20TblPtr   = (regOverride_t*)pOverrides_bleTx20   ,// High power PA
#endif // EM_CC1354P10_1_LP || CC2674R10_RGZ_LP
#endif // defined(CC13X2P) || defined(EM_CC1354P10_1_LP) || defined(CC2674R10_RGZ_LP) || defined(CC2674P10_RGZ_LP)
#if defined(RTLS_CTE)
  .rfRegOverrideCtePtr = (regOverride_t*)(pOverrides_bleCommon + BLE_STACK_OVERRIDES_OFFSET + CTE_OVERRIDES_OFFSET),
  .cteAntennaPropPtr   = &appCTEAntProp,
#else
  .rfRegOverrideCtePtr = NULL,
  .cteAntennaPropPtr   = NULL,
#endif // RTLS_CTE
#ifndef CC33xx
  .privOverrideOffset = BLE_STACK_OVERRIDES_OFFSET + PRIVACY_OVERRIDE_OFFSET,
#else
  .privOverrideOffset = 0,
#endif // CC33xx
#if defined(USE_COEX)
  .coexUseCaseConfigPtr = &coexSysConfig,
#else
  .coexUseCaseConfigPtr = NULL,
#endif // USE_COEX
#else // !CC23X0
    .rfFeModeBias  = 0,
#endif
};

#else /* !(ICALL_JT) */

#ifndef CC23X0
// RF Driver API Table
rfDrvTblPtr_t rfDriverTable[] =
{
  (uint32) RF_open,
  (uint32) driverTable_fnSpinlock, // RF_close
#ifdef RF_SINGLEMODE
  (uint32) RF_postCmd,
#else // !RF_SINGLEMODE
  (uint32) driverTable_fnSpinlock, // RF_postCmd
#endif// RF_SINGLEMODE
  (uint32) driverTable_fnSpinlock, // RF_pendCmd
#ifdef RF_SINGLEMODE
  (uint32) RF_runCmd,
#else // !RF_SINGLEMODE
  (uint32) driverTable_fnSpinlock, // RF_runCmd
#endif// RF_SINGLEMODE
  (uint32) RF_cancelCmd,
  (uint32) RF_flushCmd,
  (uint32) driverTable_fnSpinlock, // RF_yield
  (uint32) RF_Params_init,
  (uint32) RF_runImmediateCmd,
  (uint32) RF_runDirectCmd,
  (uint32) RF_ratCompare
  (uint32) driverTable_fnSpinlock, // RF_ratCapture
  (uint32) driverTable_fnSpinlock, // RF_ratDisableChannel
  (uint32) RF_getCurrentTime,
  (uint32) RF_getRssi,
  (uint32) RF_getInfo,
  (uint32) RF_getCmdOp,
  (uint32) RF_control,
  (uint32) driverTable_fnSpinlock, // RF_getTxPower
  (uint32) RF_setTxPower, // RF_setTxPower
  (uint32) driverTable_fnSpinlock, // RF_TxPowerTable_findPowerLevel
  (uint32) driverTable_fnSpinlock, // RF_TxPowerTable_findValue
#ifndef RF_SINGLEMODE
  (uint32) RF_scheduleCmd,
  (uint32) RF_runScheduleCmd,
  (uint32) driverTable_fnSpinlock  // RF_requestAccess
#endif // !RF_SINGLEMODE
};

// ECC Driver API Table
eccDrvTblPtr_t eccDriverTable[] =
{
  (uint32) ECDH_init,
  (uint32) ECDH_Params_init,
  (uint32) ECDH_open,
  (uint32) ECDH_close,
  (uint32) ECDH_OperationGeneratePublicKey_init,
  (uint32) ECDH_OperationComputeSharedSecret_init,
  (uint32) ECDH_generatePublicKey,
  (uint32) ECDH_computeSharedSecret
};

// Crypto Driver API Table
cryptoDrvTblPtr_t cryptoDriverTable[] =
{
  (uint32)AESCCM_init,
  (uint32)AESCCM_open,
  (uint32)AESCCM_close,
  (uint32)AESCCM_Params_init,
  (uint32)AESCCM_Operation_init,
  (uint32)AESCCM_oneStepEncrypt,
  (uint32)AESCCM_oneStepDecrypt,
  (uint32)AESECB_init,
  (uint32)AESECB_open,
  (uint32)AESECB_close,
  (uint32)AESECB_Params_init,
  (uint32)AESECB_Operation_init,
  (uint32)AESECB_oneStepEncrypt,
  (uint32)AESECB_oneStepDecrypt,
  (uint32)CryptoKeyPlaintext_initKey,
  (uint32)CryptoKeyPlaintext_initBlankKey
};

trngDrvTblPtr_t trngDriverTable[] =
{
  (uint32) TRNG_init,
  (uint32) TRNG_open,
  (uint32) TRNG_generateEntropy,
  (uint32) TRNG_close
};
#endif

#endif /* ICALL_JT */

/*******************************************************************************
 * @fn          RegisterAssertCback
 *
 * @brief       This routine registers the Application's assert handler.
 *
 * input parameters
 *
 * @param       appAssertHandler - Application's assert handler.
 *
 * output parameters
 *
 * @param       None.
 *
 * @return      None.
 */
void RegisterAssertCback(assertCback_t appAssertHandler)
{
  appAssertCback = appAssertHandler;

#ifdef EXT_HAL_ASSERT
  // also set the Assert callback pointer used by halAssertHandlerExt
  // Note: Normally, this pointer will be intialized by the stack, but in the
  //       event HAL_ASSERT is used by the Application, we initialize it
  //       directly here.
  halAssertCback = appAssertHandler;
#endif // EXT_HAL_ASSERT

  return;
}

/*******************************************************************************
 * @fn          driverTable_fnSpinLock
 *
 * @brief       This routine is used to trap calls to unpopulated indexes of
 *              driver function pointer tables.
 *
 * input parameters
 *
 * @param       None.
 *
 * output parameters
 *
 * @param       None.
 *
 * @return      None.
 */
void driverTable_fnSpinlock(void)
{
  volatile uint8 i = 1;

  while(i);
}

/*******************************************************************************
 * @fn          DefaultAssertCback
 *
 * @brief       This is the Application default assert callback, in the event
 *              none is registered.
 *
 * input parameters
 *
 * @param       assertCause    - Assert cause as defined in hal_assert.h.
 * @param       assertSubcause - Optional assert subcause (see hal_assert.h).
 *
 * output parameters
 *
 * @param       None.
 *
 * @return      None.
 */
void DefaultAssertCback(uint8 assertCause, uint8 assertSubcause)
{
#ifdef HAL_ASSERT_SPIN
  driverTable_fnSpinlock();
#endif // HAL_ASSERT_SPIN

  return;
}

// Application Assert Callback Function Pointer
assertCback_t appAssertCback = DefaultAssertCback;

/*******************************************************************************
 */
