/*
 * Copyright (c) 2016-2019, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


/***** Includes *****/
/* Standard C Libraries */
#include <stdlib.h>

/* TI-RTOS Header files */
#include <ti/drivers/rf/RF.h>
#include <ti/drivers/GPIO.h>

/* Board Header files */
#include "ti_drivers_config.h"

/* Application specific Header files */
#include "menu.h"
#include "config.h"
#include "RFQueue.h"

#include <ti_radio_config.h>

/***** Defines *****/
#define MAX_PAYLOAD_LENGTH      254 // Maximum length of the packet to send (Even due to HS requirement)
#define MAX_BLE_PAYLOAD_LENGTH  30  // Maximum length of the BLE4/5 packet to send
#define DATA_ENTRY_HEADER_SIZE  8   // Constant header size of a Generic Data Entry
#define MAX_LENGTH              254 // Set the length of the data entry
#define NUM_DATA_ENTRIES        1
#define NUM_APPENDED_BYTES      0

#define EXTENDED_HEADER_LENGTH  9
#define BLE_BASE_FREQUENCY      2300 // When programming the channel in the BLE TX command it is the
                                     // offset from 2300 MHz

#define MAX_BLE_PWR_LEVEL_DBM         5
#define MAX_2_4_GHZ_PWR_LEVEL_DBM     5
#define MAX_SUB1_PWR_LEVEL_DBM        13
#define MAX_SUB1_BOOST_PWR_LEVEL_DBM  14

#define ABORT_GRACEFUL          1   // Option for the RF cancel command
#define ABORT_ABRUPT            0   // Option for the RF cancel command

/* Inter-packet intervals for each phy mode in ms*/
#define PKT_INTERVAL_MS_2GFSK   60
#define PKT_INTERVAL_MS_CUSTOM  60
#define PKT_INTERVAL_MS_SLR     80
#define PKT_INTERVAL_MS_LRM     500
#define PKT_INTERVAL_MS_OOK     100
#define PKT_INTERVAL_MS_HSM     50
#define PKT_INTERVAL_MS_BLE     100

#define RF_TX20_ENABLED         0xFFFF // Tx power setting when high PA is in use
#define CENTER_FREQ_EU          0x0364 // Center Frequency 868 MHz
#define CENTER_FREQ_US          0x0393 // Center Frequency 915 MHz

/* IEEE 802.15.4g Header Configuration
 * _S indicates the shift for a given bit field
 * _M indicates the mask required to isolate a given bit field
 */
#define IEEE_HDR_LEN_S          0U
#define IEEE_HDR_LEN_M          0x00FFU
#define IEEE_HDR_CRC_S          12U
#define IEEE_HDR_CRC_M          0x1000U
#define IEEE_HDR_WHTNG_S        11U
#define IEEE_HDR_WHTNG_M        0x0800U
#define IEEE_HDR_CRC_2BYTE      1U
#define IEEE_HDR_CRC_4BYTE      0U
#define IEEE_HDR_WHTNG_EN       1U
#define IEEE_HDR_WHTNG_DIS      0U

#define IEEE_HDR_CREATE(crc, whitening, length) {            \
    (crc << IEEE_HDR_CRC_S | whitening << IEEE_HDR_WHTNG_S | \
    ((length << IEEE_HDR_LEN_S) & IEEE_HDR_LEN_M))           \
}


/***** Prototypes *****/
static void tx_callback(RF_Handle h, RF_CmdHandle ch, RF_EventMask e);

/***** Variable declarations *****/
static RF_Object rfObject;
static RF_Handle rfHandle;

static uint8_t packet[MAX_PAYLOAD_LENGTH];
static uint8_t dataOffset = 0;
static volatile uint16_t seqNumber = 0;
static volatile uint32_t packetCount = 0;
static ApplicationConfig localConfig;
static volatile uint32_t time = 0;
static volatile bool bPacketTxDone = false;
static volatile RF_CmdHandle cmdHandle;

static uint8_t triggerType = TRIG_NOW;

static tx_metrics txMetrics = {
    .transmitPowerDbm = RF_TxPowerTable_INVALID_DBM,
    .dataRateBps      = 0,
    .packetIntervalMs = 0
};

/*
This interval is dependent on data rate and packet length, and might need to be changed
if any of these parameter changes
*/
uint32_t packetInterval;

#if defined(__TI_COMPILER_VERSION__)
#pragma DATA_ALIGN (txDataEntryBuffer, 4);
#elif defined(__IAR_SYSTEMS_ICC__)
#pragma data_alignment = 4
#elif defined(__GNUC__)
__attribute__ ((aligned (4)))
#else
#error This compiler is not supported.
#endif
static uint8_t txDataEntryBuffer[RF_QUEUE_DATA_ENTRY_BUFFER_SIZE(NUM_DATA_ENTRIES,
                                                                 MAX_LENGTH,
                                                                 NUM_APPENDED_BYTES)];

/* TX queue or RF Core to read data from */
static dataQueue_t dataQueue;
static rfc_dataEntryGeneral_t* currentDataEntry;
static uint8_t *pPacket;
#if (defined SUPPORT_BLE)
rfc_ble5ExtAdvEntry_t ble5ExtAdvPacket;
#endif

/* Runs the transmitting part of the test application and returns a result. */
TestResult tx_runTxTest(const ApplicationConfig* config)
{
    uint32_t lastpacketCount = 0;
    uint16_t cmdTxPower      = 0;
    RF_TxPowerTable_Entry *rfPowerTable = NULL;
    uint8_t rfPowerTableSize = 0;
    dataOffset       = 0;

    if(config == NULL)
    {
        while(1);
    }
    memcpy((void *)&localConfig, config, sizeof(ApplicationConfig));

    RF_Params rfParams;
    RF_Params_init(&rfParams);
    if(localConfig.intervalMode == IntervalMode_Yes)
    {
        triggerType = TRIG_ABSTIME;
    }
    else
    {
        triggerType = TRIG_NOW;
    }

    rfc_CMD_PROP_TX_ADV_t *pAdvTxCmd = NULL;
    rfc_CMD_PROP_TX_t *pTxCmd = NULL;

    // Setup RX Commands
    switch (config->rfSetup)
    {
        case RfSetup_Custom:
            pTxCmd = &RF_cmdPropTx;
            break;
#if (defined SUPPORT_FSK_50KBPS)
        case RfSetup_Fsk:
            pTxCmd = &RF_cmdPropTx_fsk_50kbps;
            break;
#endif
#if (defined SUPPORT_PROP_2_4_GHZ)
#if !((defined CONFIG_LP_EM_CC2653P10)   || (defined CONFIG_CC2653P10RSL))
        case RfSetup_2_4_Fsk_250:
            pTxCmd = &RF_cmdPropTx_2_4G_fsk_250kbps;
            break;
#endif
        case RfSetup_2_4_Fsk_100:
            pTxCmd = &RF_cmdPropTx_2_4G_fsk_100kbps;
            break;

#endif
#if (defined SUPPORT_SLR)
        case RfSetup_Sl_lr:
            pTxCmd = &RF_cmdPropTx_sl_lr;
            break;
#endif
#if (defined SUPPORT_BLE)
        case RfSetup_Ble:
        case RfSetup_Ble5:
            RF_ble_cmdBle5AdvAux.pParams->pAdvPkt = (uint8_t *)&ble5ExtAdvPacket;
            ble5ExtAdvPacket.extHdrInfo.length = EXTENDED_HEADER_LENGTH;
            ble5ExtAdvPacket.advDataLen = MAX_BLE_PAYLOAD_LENGTH - EXTENDED_HEADER_LENGTH - 1;
            ble5ExtAdvPacket.pAdvData = packet;
            RF_ble_cmdBle5AdvAux.startTrigger.triggerType  = triggerType;
            RF_ble_cmdBle5AdvAux.startTrigger.pastTrig = 1;
            RF_ble_cmdBle5AdvAux.channel = 0xFF;
            RF_ble_cmdBle5AdvAux.whitening.bOverride = 1;
            RF_ble_cmdBle5AdvAux.whitening.init = config->frequencyTable[config->frequency].whitening;
            RF_ble_cmdBle5AdvAux.startTime = 0;
            RF_ble_cmdBle5AdvAux.txPower = 0;
            RF_ble_cmdBleAdvNc.pParams->pAdvData = packet;
            RF_ble_cmdBleAdvNc.startTrigger.triggerType  = triggerType;
            RF_ble_cmdBleAdvNc.startTrigger.pastTrig = 1;
            RF_ble_cmdBleAdvNc.channel = 0xFF;
            RF_ble_cmdBleAdvNc.whitening.bOverride = 1;
            RF_ble_cmdBleAdvNc.whitening.init = config->frequencyTable[config->frequency].whitening;
            RF_ble_cmdBleAdvNc.startTime = 0;
            break;
#endif
#if (defined SUPPORT_FSK_200KBPS)
        case RfSetup_Fsk_200kbps:
            pAdvTxCmd = &RF_cmdPropTxAdv_fsk_200kbps;
            break;
#endif
        default:
            break;
    }

    if(NULL != pTxCmd)
    {
        pTxCmd->pktLen = config->payloadLength;
        pTxCmd->pPkt = packet;
        pTxCmd->startTrigger.triggerType = triggerType;
        pTxCmd->startTrigger.pastTrig = 1;
        pTxCmd->startTime = 0;
    }
    else if (NULL != pAdvTxCmd)
    {
        pAdvTxCmd->pktLen = config->payloadLength;
        pAdvTxCmd->pPkt = packet;
        pAdvTxCmd->startTrigger.triggerType = triggerType;
        pAdvTxCmd->startTrigger.pastTrig = 1;
        pAdvTxCmd->startTime = 0;
    }

    if( RFQueue_defineQueue(&dataQueue,
                            txDataEntryBuffer,
                            sizeof(txDataEntryBuffer),
                            NUM_DATA_ENTRIES,
                            MAX_LENGTH + NUM_APPENDED_BYTES))
    {
        /* Failed to allocate space for all data entries */
        while(true);
    }

    currentDataEntry = (rfc_dataEntryGeneral_t*)&txDataEntryBuffer;
    currentDataEntry->length = config->payloadLength;
    pPacket = &currentDataEntry->data;

    /* Request access to the radio based on test case */
    switch (config->rfSetup)
    {
        case RfSetup_Custom:
            rfHandle = RF_open(&rfObject, &RF_prop, (RF_RadioSetup*)&RF_cmdPropRadioDivSetup, &rfParams);
            cmdTxPower = RF_cmdPropRadioDivSetup.txPower;
            packetInterval = (uint32_t)(RF_convertMsToRatTicks(PKT_INTERVAL_MS_CUSTOM)); // Set packet interval to 60 ms
            break;
#if (defined SUPPORT_FSK_50KBPS)
        case RfSetup_Fsk:
            RF_cmdPropRadioDivSetup_fsk_50kbps.centerFreq = config->frequencyTable[config->frequency].frequency;
            rfHandle = RF_open(&rfObject, &RF_prop_fsk_50kbps, (RF_RadioSetup*)&RF_cmdPropRadioDivSetup_fsk_50kbps, &rfParams);
            cmdTxPower     = RF_cmdPropRadioDivSetup_fsk_50kbps.txPower;
            packetInterval = (uint32_t)(RF_convertMsToRatTicks(PKT_INTERVAL_MS_2GFSK)); // Set packet interval to 60 ms
            break;
#endif
#if (defined SUPPORT_PROP_2_4_GHZ)
#if !((defined CONFIG_LP_EM_CC2653P10)   || (defined CONFIG_CC2653P10RSL))
        case RfSetup_2_4_Fsk_250:
            RF_cmdPropRadioDivSetup_2_4G_fsk_250kbps.centerFreq = config->frequencyTable[config->frequency].frequency; //TODO: Is this line needed?
            rfHandle = RF_open(&rfObject, &RF_prop_2_4G_fsk_250kbps, (RF_RadioSetup*)&RF_cmdPropRadioDivSetup_2_4G_fsk_250kbps, &rfParams);
            cmdTxPower = RF_cmdPropRadioDivSetup_2_4G_fsk_250kbps.txPower;
            packetInterval = (uint32_t)(RF_convertMsToRatTicks(PKT_INTERVAL_MS_2GFSK)); // Set packet interval to 60 ms
            break;
#endif
        case RfSetup_2_4_Fsk_100:
            RF_cmdPropRadioDivSetup_2_4G_fsk_100kbps.centerFreq = config->frequencyTable[config->frequency].frequency; //TODO: Is this line needed?
            rfHandle = RF_open(&rfObject, &RF_prop_2_4G_fsk_100kbps, (RF_RadioSetup*)&RF_cmdPropRadioDivSetup_2_4G_fsk_100kbps, &rfParams);
            cmdTxPower = RF_cmdPropRadioDivSetup_2_4G_fsk_100kbps.txPower;
            packetInterval = (uint32_t)(RF_convertMsToRatTicks(PKT_INTERVAL_MS_2GFSK)); // Set packet interval to 60 ms
            break;
#endif
#if (defined SUPPORT_SLR)
        case RfSetup_Sl_lr:
            RF_cmdPropRadioDivSetup_sl_lr.centerFreq = config->frequencyTable[config->frequency].frequency;
            rfHandle = RF_open(&rfObject, &RF_prop_sl_lr, (RF_RadioSetup*)&RF_cmdPropRadioDivSetup_sl_lr, &rfParams);
            packetInterval = (uint32_t)(RF_convertMsToRatTicks(PKT_INTERVAL_MS_SLR)); // Set packet interval to 80 ms
            cmdTxPower     = RF_cmdPropRadioDivSetup_sl_lr.txPower;
            break;
#endif
#if (defined SUPPORT_BLE)
        case RfSetup_Ble:
        case RfSetup_Ble5:
            rfHandle = RF_open(&rfObject, &RF_modeBle, (RF_RadioSetup*)&RF_ble_cmdRadioSetup, &rfParams);
            packetInterval = (uint32_t)(RF_convertMsToRatTicks(PKT_INTERVAL_MS_BLE)); // Set packet interval to 100 ms
            cmdTxPower     = RF_ble_cmdRadioSetup.txPower;
            break;
#endif
#if (defined SUPPORT_FSK_200KBPS)
        case RfSetup_Fsk_200kbps:
            rfHandle = RF_open(&rfObject, &RF_prop_fsk_200kbps, (RF_RadioSetup*)&RF_cmdPropRadioDivSetup_fsk_200kbps, &rfParams);
            cmdTxPower = RF_cmdPropRadioDivSetup_fsk_200kbps.txPower;
            dataOffset = 2;
            packetInterval = (uint32_t)(RF_convertMsToRatTicks(PKT_INTERVAL_MS_2GFSK)); // Set packet interval to 60 ms
            break;
#endif
        default:
            break;
    }

    /* Set the packet interval for display purposes */
    if(config->intervalMode == IntervalMode_Yes)
    {
        txMetrics.packetIntervalMs = RF_convertRatTicksToMs(packetInterval);
    }
    else
    {
        // packets sent back-to-back
        txMetrics.packetIntervalMs = 0;
    }

    /* Determine which TX power table and size to use */
    switch (config->rfSetup)
    {
        case RfSetup_Custom:
            rfPowerTable = (RF_TxPowerTable_Entry *)txPowerTable;
            rfPowerTableSize = TX_POWER_TABLE_SIZE;
            break;
#if (defined SUPPORT_FSK_50KBPS)
        case RfSetup_Fsk:
            rfPowerTable = (RF_TxPowerTable_Entry *)RF_PROP_txPowerTable_fsk_50kbps;
            rfPowerTableSize = RF_PROP_TX_POWER_TABLE_SIZE_fsk_50kbps;
            break;
#endif
#if (defined SUPPORT_PROP_2_4_GHZ)
#if !((defined CONFIG_LP_EM_CC2653P10)   || (defined CONFIG_CC2653P10RSL))
        case RfSetup_2_4_Fsk_250:
            rfPowerTable = (RF_TxPowerTable_Entry *)RF_PROP_txPowerTable_2_4G_fsk_250kbps;
            rfPowerTableSize = RF_PROP_TX_POWER_TABLE_SIZE_2_4G_fsk_100kbps;
            break;
#endif
        case RfSetup_2_4_Fsk_100:
            rfPowerTable = (RF_TxPowerTable_Entry *)RF_PROP_txPowerTable_2_4G_fsk_100kbps;
            rfPowerTableSize = RF_PROP_TX_POWER_TABLE_SIZE_2_4G_fsk_100kbps;
            break;
#endif
#if (defined SUPPORT_FSK_200KBPS)
        case RfSetup_Fsk_200kbps:
            rfPowerTable = (RF_TxPowerTable_Entry *)RF_PROP_txPowerTable_fsk_200kbps;
            rfPowerTableSize = RF_PROP_TX_POWER_TABLE_SIZE_fsk_200kbps;
            break;
#endif
#if  (defined SUPPORT_SLR)
        case RfSetup_Sl_lr:
            rfPowerTable = (RF_TxPowerTable_Entry *)PROP_RF_txPowerTable_sl_lr;
            rfPowerTableSize = RF_PROP_TX_POWER_TABLE_SIZE_SL_LR;
            break;
#endif
#if (defined SUPPORT_BLE)
    case RfSetup_Ble:
    case RfSetup_Ble5:
        rfPowerTable = (RF_TxPowerTable_Entry *)RF_BLE_txPowerTable;
        rfPowerTableSize = RF_BLE_TX_POWER_TABLE_SIZE;
        break;
#endif
    default:
        break;
    }

    /* Set the power level based on the choice of high or default PA */
    int8_t powerLevelDbm;
#if (defined SUPPORT_HIGH_PA)
    if(config->highPa == HighPa_En)
    {
        powerLevelDbm = RF_TxPowerTable_MAX_DBM;
    }
    else
    {

#if (defined SUPPORT_PROP_2_4_GHZ) || (defined SUPPORT_BLE)
        if((config->rfSetup == RfSetup_Ble) || (config->rfSetup == RfSetup_Ble5))
        {
            powerLevelDbm = MAX_BLE_PWR_LEVEL_DBM;
        }
#if defined SUPPORT_PROP_2_4_GHZ
        else if((config->rfSetup == RfSetup_2_4_Fsk_250) || (config->rfSetup == RfSetup_2_4_Fsk_100))
        {
            powerLevelDbm = MAX_2_4_GHZ_PWR_LEVEL_DBM;
        }
#endif
        else
        {
            powerLevelDbm = MAX_SUB1_PWR_LEVEL_DBM;
        }
#else
        powerLevelDbm = MAX_SUB1_PWR_LEVEL_DBM;
#endif // (defined SUPPORT_PROP_2_4_GHZ) || (defined SUPPORT_BLE)

    }
#else
    if(ccfgForceVddrHH)
    {
        powerLevelDbm = MAX_SUB1_BOOST_PWR_LEVEL_DBM;
    }
#if (defined SUPPORT_PROP_2_4_GHZ)
    else if((config->rfSetup == RfSetup_2_4_Fsk_250) || (config->rfSetup == RfSetup_2_4_Fsk_100))
    {
        powerLevelDbm = MAX_2_4_GHZ_PWR_LEVEL_DBM;
    }
#endif // (defined SUPPORT_PROP_2_4_GHZ)
    else
    {
        powerLevelDbm = MAX_SUB1_PWR_LEVEL_DBM;
    }
#endif

    RF_TxPowerTable_Value powerTableValue =  RF_TxPowerTable_findValue(rfPowerTable, powerLevelDbm);
    RF_Stat status = RF_setTxPower(rfHandle, powerTableValue);
    if(status != RF_StatSuccess)
    {
        txMetrics.transmitPowerDbm = RF_TxPowerTable_INVALID_DBM;
    }

    /*
     * Exceptions for
     *   1. BLE: Tx power is set to the second highest entry when boost mode
     *      is turned off (CCFG_FORCE_VDDR_HH = 0)
     *   2. High Speed Mode: The Tx power is set to the highest entry in the
     *      power table when boost mode is turned on (CCFG_FORCE_VDDR_HH = 1)
     *      or the second highest entry when boost is turned off
     *      (CCFG_FORCE_VDDR_HH = 0)
     */
    if((rfPowerTable == NULL) || (cmdTxPower == 0))
    {
        txMetrics.transmitPowerDbm = RF_TxPowerTable_INVALID_DBM;
    }
    else
    {
        RF_TxPowerTable_Value currValue = RF_getTxPower(rfHandle);
        txMetrics.transmitPowerDbm = RF_TxPowerTable_findPowerLevel(rfPowerTable, currValue);

        //if CCFG_FORCE_VDDR_HH is not set max power cannot be achieved in Sub-1GHz
        //mode; this does not apply to 2.4GHz proprietary modes either
        if(ccfgForceVddrHH)
        {
            if((currValue.paType == RF_TxPowerTable_DefaultPA) &&
                (txMetrics.transmitPowerDbm == rfPowerTable[rfPowerTableSize-2].power))
            {
#if (defined SUPPORT_BLE)
                if ((config->rfSetup != RfSetup_Ble) && (config->rfSetup != RfSetup_Ble5))
#endif
                {
                    txMetrics.transmitPowerDbm = rfPowerTable[rfPowerTableSize-3].power;
                }
            }
        }
    }

    /* Determine the data rate in bits per seconds */
    txMetrics.dataRateBps = config_dataRateTable_Lut[config->rfSetup];

    menu_updateTxMetricScreen(&txMetrics);

    /* Set the frequency */
    switch (config->rfSetup)
    {
        case RfSetup_Custom:
            /* Custom settings exported from SmartRf studio shall use the exported frequency */
            RF_runCmd(rfHandle, (RF_Op*)&RF_cmdFs, RF_PriorityNormal, NULL, 0);
            break;
#if (defined SUPPORT_FSK_50KBPS)
        case RfSetup_Fsk:
            RF_cmdFs_fsk_50kbps.frequency = config->frequencyTable[config->frequency].frequency;
            RF_cmdFs_fsk_50kbps.fractFreq = config->frequencyTable[config->frequency].fractFreq;
            RF_runCmd(rfHandle, (RF_Op*)&RF_cmdFs_fsk_50kbps, RF_PriorityNormal, NULL, 0);
            break;
#endif
#if (defined SUPPORT_PROP_2_4_GHZ)
#if !((defined CONFIG_LP_EM_CC2653P10)   || (defined CONFIG_CC2653P10RSL))
        case RfSetup_2_4_Fsk_250:
            RF_cmdFs_2_4G_fsk_250kbps.frequency = config->frequencyTable[config->frequency].frequency;
            RF_cmdFs_2_4G_fsk_250kbps.fractFreq = config->frequencyTable[config->frequency].fractFreq;
            RF_runCmd(rfHandle, (RF_Op*)&RF_cmdFs_2_4G_fsk_250kbps, RF_PriorityNormal, NULL, 0);
            break;
#endif
        case RfSetup_2_4_Fsk_100:
            RF_cmdFs_2_4G_fsk_100kbps.frequency = config->frequencyTable[config->frequency].frequency;
            RF_cmdFs_2_4G_fsk_100kbps.fractFreq = config->frequencyTable[config->frequency].fractFreq;
            RF_runCmd(rfHandle, (RF_Op*)&RF_cmdFs_2_4G_fsk_100kbps, RF_PriorityNormal, NULL, 0);
            break;
#endif
#if (defined SUPPORT_FSK_200KBPS)
        case RfSetup_Fsk_200kbps:
            RF_cmdFs_fsk_200kbps.frequency = config->frequencyTable[config->frequency].frequency;
            RF_cmdFs_fsk_200kbps.fractFreq = config->frequencyTable[config->frequency].fractFreq;
            RF_runCmd(rfHandle, (RF_Op*)&RF_cmdFs_fsk_200kbps, RF_PriorityNormal, NULL, 0);
            break;
#endif
#if  (defined SUPPORT_SLR)
        case RfSetup_Sl_lr:
            RF_cmdFs_sl_lr.frequency = config->frequencyTable[config->frequency].frequency;
            RF_cmdFs_sl_lr.fractFreq = config->frequencyTable[config->frequency].fractFreq;
            RF_runCmd(rfHandle, (RF_Op*)&RF_cmdFs_sl_lr, RF_PriorityNormal, NULL, 0);
            break;
#endif
#if (defined SUPPORT_BLE)
    case RfSetup_Ble:
    case RfSetup_Ble5:
        RF_ble_cmdFs.frequency = config->frequencyTable[config->frequency].frequency;
        RF_ble_cmdFs.fractFreq = config->frequencyTable[config->frequency].fractFreq;
        RF_runCmd(rfHandle, (RF_Op*)&RF_ble_cmdFs, RF_PriorityNormal, NULL, 0);
        if(config->intervalMode == IntervalMode_No)
        {
            /* If BLE packets are sent back-to-back the synthesizer is turned
             * off after the first transmission if the advertisement channel is
             * set to 255. The channel must be directly written in the
             * advertisement command itself; it is an offset from 2300 MHz.
             */
            RF_ble_cmdBleAdvNc.channel = config->frequencyTable[config->frequency].frequency - BLE_BASE_FREQUENCY;
            RF_ble_cmdBle5AdvAux.channel = config->frequencyTable[config->frequency].frequency - BLE_BASE_FREQUENCY;
        }
        break;
#endif
    default:
        break;
    }

    /* Get current time */
    time = RF_getCurrentTime();

    /* Create packet with incrementing sequence number and random payload */
#if (defined SUPPORT_FSK_200KBPS)
    if(config->rfSetup == RfSetup_Fsk_200kbps)
    {
        /* Compose the IEEE 802.15.4g header
         * Use a 2-byte CRC with whitening enabled
         */
        uint16_t ieeeHdr = IEEE_HDR_CREATE(IEEE_HDR_CRC_2BYTE, IEEE_HDR_WHTNG_EN, config->payloadLength);
        pPacket[0] = packet[0] = (uint8_t)(ieeeHdr & 0x00FF);
        pPacket[1] = packet[1] = (uint8_t)((ieeeHdr & 0xFF00) >> 8);
    }
#endif
    pPacket[dataOffset + 0] = packet[dataOffset + 0] = (uint8_t)(seqNumber >> 8);
    pPacket[dataOffset + 1] = packet[dataOffset + 1] = (uint8_t)(seqNumber++);
    uint16_t i;
    for (i = dataOffset + 2; i < config->payloadLength; i++)
    {
        pPacket[i] = packet[i] = rand();
    }

    /* Set absolute TX time to utilize automatic power management */
    time += packetInterval;

    /* Send packet */
    switch (config->rfSetup)
    {
        case RfSetup_Custom:
            RF_cmdPropTx.startTime = time;
            cmdHandle = RF_postCmd(rfHandle, (RF_Op*)&RF_cmdPropTx, RF_PriorityNormal, &tx_callback, 0);
            break;
#if (defined SUPPORT_FSK_50KBPS)
        case RfSetup_Fsk:
            RF_cmdPropTx_fsk_50kbps.startTime = time;
            cmdHandle = RF_postCmd(rfHandle, (RF_Op*)&RF_cmdPropTx_fsk_50kbps, RF_PriorityNormal, &tx_callback, 0);
            break;
#endif
#if (defined SUPPORT_PROP_2_4_GHZ)
#if !((defined CONFIG_LP_EM_CC2653P10)   || (defined CONFIG_CC2653P10RSL))
        case RfSetup_2_4_Fsk_250:
            RF_cmdPropTx_2_4G_fsk_250kbps.startTime = time;
            cmdHandle = RF_postCmd(rfHandle, (RF_Op*)&RF_cmdPropTx_2_4G_fsk_250kbps, RF_PriorityNormal, &tx_callback, 0);
            break;
#endif
        case RfSetup_2_4_Fsk_100:
            RF_cmdPropTx_2_4G_fsk_100kbps.startTime = time;
            cmdHandle = RF_postCmd(rfHandle, (RF_Op*)&RF_cmdPropTx_2_4G_fsk_100kbps, RF_PriorityNormal, &tx_callback, 0);
            break;
#endif
#if (defined SUPPORT_SLR)
        case RfSetup_Sl_lr:
            RF_cmdPropTx_sl_lr.startTime = time;
            cmdHandle = RF_postCmd(rfHandle, (RF_Op*)&RF_cmdPropTx_sl_lr, RF_PriorityNormal, &tx_callback, 0);
            break;
#endif
#if (defined SUPPORT_BLE)
        case RfSetup_Ble:
            RF_ble_cmdBleAdvNc.startTime = time;
            cmdHandle = RF_postCmd(rfHandle, (RF_Op*)&RF_ble_cmdBleAdvNc, RF_PriorityNormal, &tx_callback, 0);
            break;
        case RfSetup_Ble5:
            RF_ble_cmdBle5AdvAux.startTime = time;
            cmdHandle = RF_postCmd(rfHandle, (RF_Op*)&RF_ble_cmdBle5AdvAux, RF_PriorityNormal, &tx_callback, 0);
            break;
#endif
#if (defined SUPPORT_FSK_200KBPS)
        case RfSetup_Fsk_200kbps:
            RF_cmdPropTxAdv_fsk_200kbps.startTime = time;
            cmdHandle = RF_postCmd(rfHandle, (RF_Op*)&RF_cmdPropTxAdv_fsk_200kbps, RF_PriorityNormal, &tx_callback, 0);
            break;
#endif
        default:
            break;
    }

    while (!bPacketTxDone)
    {
        /* Check, whether a button has been pressed */
        if (menu_isButtonPressed())
        {
            /* If there is an ongoing Tx command, cancel it */
            (void)RF_cancelCmd(rfHandle, cmdHandle, ABORT_GRACEFUL);
            RF_pendCmd(rfHandle, cmdHandle, (RF_EventCmdCancelled | RF_EventCmdStopped | RF_EventCmdAborted));
            RF_close(rfHandle);

            /* Do a final update to indicate #packets sent*/
            menu_updateTxScreen(packetCount);

            bPacketTxDone = false;
            packetCount = 0;
            seqNumber = 0;
            return TestResult_Aborted;
        }
        else if(packetCount != lastpacketCount)
        {
            /* Update the display */
            menu_updateTxScreen(packetCount);
            lastpacketCount = packetCount;
        }
    }

    if(packetCount == config->packetCount)
    {
        /* Do a final update to indicate all packets were sent*/
        menu_updateTxScreen(packetCount);
    }

    bPacketTxDone = false;
    packetCount = 0;
    seqNumber = 0;
    RF_close(rfHandle);
    return TestResult_Finished;
}

void tx_callback(RF_Handle h, RF_CmdHandle ch, RF_EventMask e)
{
    if(e & RF_EventLastCmdDone)
    {
        /* Increment the packet counter */
        packetCount++;

        if(packetCount < localConfig.packetCount)
        {
            /* Increment the sequence number for the next packet but keep
             * the same data */
            pPacket[dataOffset + 0] = packet[dataOffset + 0] = (uint8_t)(seqNumber >> 8);
            pPacket[dataOffset + 1] = packet[dataOffset + 1] = (uint8_t)(seqNumber++);

            /* Set absolute TX time to utilize automatic power management */
            time += packetInterval;

            switch (localConfig.rfSetup)
            {
                case RfSetup_Custom:
                    RF_cmdPropTx.startTime = time;
                    cmdHandle = RF_postCmd(rfHandle, (RF_Op*)&RF_cmdPropTx, RF_PriorityNormal, &tx_callback, 0);
                    break;
#if (defined SUPPORT_FSK_50KBPS)
                case RfSetup_Fsk:
                    RF_cmdPropTx_fsk_50kbps.startTime = time;
                    cmdHandle = RF_postCmd(rfHandle, (RF_Op*)&RF_cmdPropTx_fsk_50kbps, RF_PriorityNormal, &tx_callback, 0);
                    break;
#endif
#if (defined SUPPORT_PROP_2_4_GHZ)
#if !((defined CONFIG_LP_EM_CC2653P10)   || (defined CONFIG_CC2653P10RSL))
                case RfSetup_2_4_Fsk_250:
                    RF_cmdPropTx_2_4G_fsk_250kbps.startTime = time;
                    cmdHandle = RF_postCmd(rfHandle, (RF_Op*)&RF_cmdPropTx_2_4G_fsk_250kbps, RF_PriorityNormal, &tx_callback, 0);
                    break;
#endif
                case RfSetup_2_4_Fsk_100:
                    RF_cmdPropTx_2_4G_fsk_100kbps.startTime = time;
                    cmdHandle = RF_postCmd(rfHandle, (RF_Op*)&RF_cmdPropTx_2_4G_fsk_100kbps, RF_PriorityNormal, &tx_callback, 0);
                    break;
#endif
#if (defined SUPPORT_SLR)
                case RfSetup_Sl_lr:
                    RF_cmdPropTx_sl_lr.startTime = time;
                    cmdHandle = RF_postCmd(rfHandle, (RF_Op*)&RF_cmdPropTx_sl_lr, RF_PriorityNormal, &tx_callback, 0);
                    break;
#endif
#if (defined SUPPORT_BLE)
                case RfSetup_Ble:
                    RF_ble_cmdBleAdvNc.startTime = time;
                    cmdHandle = RF_postCmd(rfHandle, (RF_Op*)&RF_ble_cmdBleAdvNc, RF_PriorityNormal, &tx_callback, 0);
                    break;
                case RfSetup_Ble5:
                    RF_ble_cmdBle5AdvAux.startTime = time;
                    cmdHandle = RF_postCmd(rfHandle, (RF_Op*)&RF_ble_cmdBle5AdvAux, RF_PriorityNormal, &tx_callback, 0);
                    break;
#endif
#if (defined SUPPORT_FSK_200KBPS)
                case RfSetup_Fsk_200kbps:
                    RF_cmdPropTxAdv_fsk_200kbps.startTime = time;
                    cmdHandle = RF_postCmd(rfHandle, (RF_Op*)&RF_cmdPropTxAdv_fsk_200kbps, RF_PriorityNormal, &tx_callback, 0);
                    break;
#endif
                default:
                    break;
            }
        }
        else
        {
            bPacketTxDone = true;
        }
    }
}
