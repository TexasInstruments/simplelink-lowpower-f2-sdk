/******************************************************************************

 @file  sp_ble_user_config.c

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
#include <ti/sysbios/BIOS.h>

#include "ti_ble_config.h"
#include "ti_radio_config.h"
#include "ble_overrides.h"
#ifndef USE_DMM
#include <ti/drivers/rf/RF.h>
#else
#include <dmm/dmm_rfmap.h>
#endif //USE_DMM

#include "ecc/ECCROMCC26XX.h"

#include <ti/drivers/AESCCM.h>
#include <ti/drivers/aesccm/AESCCMCC26XX.h>
#include <ti/drivers/AESECB.h>
#include <ti/drivers/aesecb/AESECBCC26XX.h>
#include <ti/drivers/cryptoutils/cryptokey/CryptoKeyPlaintext.h>
#include <ti/sysbios/knl/Swi.h>
#include <ti/drivers/utils/Random.h>
#include <ti/drivers/TRNG.h>

#include <driverlib/pka.h>
#include <driverlib/rf_bt5_coex.h>

#include "simple_peripheral.h"

/*******************************************************************************
 * MACROS
 */

/*******************************************************************************
 * CONSTANTS
 */

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

// Tx Power Table
txPwrTbl_t appTxPwrTbl = {
                           (txPwrVal_t*)RF_BLE_txPowerTable,
                           NUM_TX_POWER_VALUES,  // max
                           DEFAULT_TX_POWER};    // default

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

// BOOSTXL-AOA configurations
// Maximum number of antennas
#define ANTENNA_TABLE_SIZE 6
// BitMask of all the relevant GPIOs which needed for the antennas
#define ANTENNA_IO_MASK  BV(27)|BV(28)|BV(29)|BV(30)

// Antenna GPIO configuration per id (relevant for BOOSTXL-AOA)
antennaIOEntry_t antennaTbl[ANTENNA_TABLE_SIZE] = {
                               BV(27)|BV(28),  // id 0, A1.1
                               BV(27)|BV(29),  // id 1, A1.2
                               BV(27)|BV(30),  // id 2, A1.3
                               BV(28),         // id 3, A2.1
                               BV(29),         // id 4, A2.2
                               BV(30)};        // id 5  A2.3

// Antenna properties passes to the stack
cteAntProp_t  appCTEAntProp = {ANTENNA_IO_MASK,
                               ANTENNA_TABLE_SIZE,
                               antennaTbl};

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
#endif

#ifdef ICALL_JT
#include <icall.h>

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

// Swi APIs needed by BLE controller
rtosApiTblPtr_t rtosApiTable[] =
{
  (uint32_t) Swi_disable,
  (uint32_t) Swi_restore
};

// BLE Stack Configuration Structure
const stackSpecific_t bleStackConfig =
{
  .maxNumConns                          = MAX_NUM_BLE_CONNS,
  .maxNumPDUs                           = MAX_NUM_PDU,
  .maxPduSize                           = MAX_PDU_SIZE,
  .maxNumPSM                            = L2CAP_NUM_PSM,
  .maxNumCoChannels                     = L2CAP_NUM_CO_CHANNELS,
  //! Naming discrepancy between ble5stack and ble5stack_flash
#if defined (DeviceFamily_CC13X4) || defined (DeviceFamily_CC26X4)
  .maxAcceptListElems                    = MAX_NUM_AL_ENTRIES,
#else
  .maxWhiteListElems                    = MAX_NUM_WL_ENTRIES,
#endif
  .maxResolvListElems                   = CFG_MAX_NUM_RL_ENTRIES,
  .pfnBMAlloc                           = &pfnBMAlloc,
  .pfnBMFree                            = &pfnBMFree,
  .rfDriverParams.powerUpDurationMargin = RF_POWER_UP_DURATION_MARGIN,
  .rfDriverParams.inactivityTimeout     = RF_INACTIVITY_TIMEOUT,
  .rfDriverParams.powerUpDuration       = RF_POWER_UP_DURATION,
  .rfDriverParams.pErrCb                = &(RF_ERR_CB),
  .eccParams                            = &eccParams_NISTP256,
  .fastStateUpdateCb                    = SimplePeripheral_bleFastStateUpdateCb,
  .bleStackType                         = DMMPolicy_StackRole_BlePeripheral,
#ifdef CC2652RB_LAUNCHXL
  .extStackSettings                     = EXTENDED_STACK_SETTINGS | CC2652RB_OVERRIDE_USED,
#else
  .extStackSettings                     = EXTENDED_STACK_SETTINGS,
#endif
  .maxNumCteBuffers                     = MAX_NUM_CTE_BUFS
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

// Table for Driver can be found in icall_user_config.c
// if a driver is not to be used, then the pointer shoul dbe set to NULL,
// for this example, this is done in ble_user_config.h
const drvTblPtr_t driverTable =
{
  .rfDrvTbl         = rfDriverTableBLE,
  .eccDrvTbl        = eccDriverTable,
  .cryptoDrvTbl     = cryptoDriverTableBLE,
  .trngDrvTbl       = trngDriverTable,
  .rtosApiTbl       = rtosApiTable,
  .nvintfStructPtr  = &nvintfFncStruct,
#ifdef OSAL_SNV_EXTFLASH
  .extflashDrvTbl = extflashDriverTable,
#endif // OSAL_SNV_EXTFLASH
};

const boardConfig_t boardConfig =
{
  .rfFeModeBias = RF_FE_MODE_AND_BIAS,
  .rfRegTbl      = (regOverride_t*)pOverrides_bleCommon,
  .rfRegTbl1M    = (regOverride_t*)pOverrides_ble1Mbps,
#if defined(BLE_V50_FEATURES) && (BLE_V50_FEATURES & (PHY_2MBPS_CFG | PHY_LR_CFG))
  // Currently, no overrides for 2M and Coded, so exclude from build.
  .rfRegTbl2M    = (regOverride_t*)pOverrides_ble2Mbps,
  .rfRegTblCoded = (regOverride_t*)pOverrides_bleCoded,
#endif // PHY_2MBPS_CFG | PHY_LR_CFG
  .txPwrTbl      = &appTxPwrTbl,
#if defined(CC13X2P)
#if defined(CC13X2P_2_LAUNCHXL)
  .txPwrBackoffTbl = &appTxPwrBackoffTbl,
#else
  .txPwrBackoffTbl = NULL,
#endif // defined(CC13X2P_2_LAUNCHXL)
  .rfRegOverrideTxStdTblptr  = (regOverride_t*)pOverrides_bleTxStd,   // Default PA
  .rfRegOverrideTx20TblPtr   = (regOverride_t*)pOverrides_bleTx20   ,// High power PA
#endif //CC13X2P
#if defined(RTLS_CTE)
  .rfRegOverrideCtePtr = (regOverride_t*)(pOverrides_bleCommon + BLE_STACK_OVERRIDES_OFFSET + CTE_OVERRIDES_OFFSET),
  .cteAntennaPropPtr   = &appCTEAntProp,
#else
  .rfRegOverrideCtePtr = NULL,
  .cteAntennaPropPtr   = NULL,
#endif
  .privOverrideOffset = BLE_STACK_OVERRIDES_OFFSET + PRIVACY_OVERRIDE_OFFSET,
#if defined(USE_COEX)
  .coexUseCaseConfigPtr = &coexSysConfig,
#else
  .coexUseCaseConfigPtr = NULL,
#endif
};

#else /* !(ICALL_JT) */

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
