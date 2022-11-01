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
/* TI-RTOS Header files */
#include <ti/drivers/rf/RF.h>
#include <ti/drivers/GPIO.h>
#if defined(__IAR_SYSTEMS_ICC__)
#include <intrinsics.h>
#endif
/* Board Header files */
#include "ti_drivers_config.h"

/* Application specific Header files */
#include "menu.h"
#include "config.h"
#include "RFQueue.h"

#include <ti_radio_config.h>

/***** Defines *****/
#define RX_DEBUG_COLLECT_METRICS 0  // Set to 1 to collect rx metrics (debugging mode)
#define DATA_ENTRY_HEADER_SIZE  8   // Constant header size of a Generic Data Entry
#define MAX_LENGTH              254 // Max length byte the radio will accept (Even due to HS requirement)
#define NUM_DATA_ENTRIES        2   // NOTE: Only two data entries supported at the moment
#define NUM_APPENDED_BYTES      6   // -- For the HS command (6 bytes total),
                                    // packet length (2 bytes) + Timestamp (4 bytes)
                                    // -- For other Sub-1 GHz commands (6 bytes total),
                                    // packet length (1 bytes) + Timestamp (4 bytes) + status (1 byte)
                                    // -- For 200 Kbps IEEE 802.15.4g commands (5 bytes total),
                                    // Timestamp (4 bytes) + status (1 byte)
                                    // -- For BLE (9 bytes total), but max payload is well under
                                    // the max length supported by the sub-1 phys
                                    // Timestamp (4 bytes) + status (1 byte) + RSSI (1 byte) + CRC (3 bytes)
#define NUM_LEN_BITS            0x8 // For header of 200 Kbps IEEE 802.15.4g commands
#define NFRACBITS               9   // 32 - ceil(log2(4000000)) - 1
#define RX_TIMEOUT_NPKTS        5   // Specifies how many packet intervals constitute a RAT Timer
                                    // timeout interval
#define PKT_INTERVAL_TOL        2   // Specifies the tolerance, in percentage, of the permissible
                                    // drift in packet interval
#define ADV_NONCONN_IND         2   //Packet type for BLE4
#define AUX_ADV_IND             7   //Packet type for BLE5

#define ABORT_GRACEFUL          1   // Option for the RF cancel command
#define ABORT_ABRUPT            0   // Option for the RF cancel command

#define LOG2_100MILLION         27  // ceil(log2(10^8)=26.575424759098897) = 27

/*
 * PROPRIETARY RX FRAME (2-GFSK, LRM, SLR, OOK)
 *
 * +-------------------------------------------------------------+
 * |_LEN_|_____________PAYLOAD(sz)__________|_TIMESTAMP_|_STATUS_|
 * |     |_SERIAL_|__________DATA___________|           |        |
 * |1B   |2B      | Upto 252B               | 4B        | 1B     |
 * +-------------------------------------------------------------+
 */
#define RX_FRAME_PROP_OFFSET_LEN            0
#define RX_FRAME_PROP_OFFSET_SERIAL         1
#define RX_FRAME_PROP_OFFSET_DATA           3
#define RX_FRAME_PROP_OFFSET_TIMESTAMP(sz)  (RX_FRAME_PROP_OFFSET_SERIAL + sz)

/*
 * PROPRIETARY RX FRAME (2-GFSK IEEE 802.15.4g)
 *
 * +-------------------------------------------------------------+
 * |_HDR___|_____________PAYLOAD(sz)________|_TIMESTAMP_|_STATUS_|
 * |LEN|FLG|_SERIAL_|__________DATA_________|           |        |
 * |1B |1B |2B      | Upto 252B             | 4B        | 1B     |
 * +-------------------------------------------------------------+
 */
#define RX_FRAME_PROP_2GFSK200K_OFFSET_LEN            0
#define RX_FRAME_PROP_2GFSK200K_OFFSET_SERIAL         2
#define RX_FRAME_PROP_2GFSK200K_OFFSET_DATA           4
#define RX_FRAME_PROP_2GFSK200K_OFFSET_TIMESTAMP(sz)  (RX_FRAME_PROP_2GFSK200K_OFFSET_LEN + sz)

/*
 * BLE4 (ADV_NONCONN_IND Packet) RX FRAME
 *
 * +-------------------------------------------------------------------------------------------+
 * |____PDU____|_______________PAYLOAD(sz)___________________|_CRC_|_RSSI_|_STATUS_|_TIMESTAMP_|
 * |_ADV_|_LEN_|_ADV_ADDR_|_SERIAL_|__________DATA___________|     |      |        |           |
 * |1B   |1B   |6B        |2B      |Upto 22B                 |3B   |1B    |1B/2B   |4B         |
 * +-------------------------------------------------------------------------------------------+
 *
 * Note that BLE4 frames can transfer up to 37B of payload but are hard-coded
 * for 30B in this example. Also if BLE4 Rx is run on an 13X2 device then it
 * is actually the BLE5 Rx command that is run
 */
#define RX_FRAME_BLE4_OFFSET_ADV_TYPE      0
#define RX_FRAME_BLE4_OFFSET_LEN           1
#define RX_FRAME_BLE4_OFFSET_ADV_ADDR      2
#define RX_FRAME_BLE4_OFFSET_SERIAL        8
#define RX_FRAME_BLE4_OFFSET_DATA          10
#define RX_FRAME_BLE4_OFFSET_CRC(sz)       (RX_FRAME_BLE4_OFFSET_ADV_ADDR + sz)
#define RX_FRAME_BLE4_OFFSET_RSSI(sz)      (RX_FRAME_BLE4_OFFSET_CRC(sz) + 3)
#define RX_FRAME_BLE4_OFFSET_STATUS(sz)    (RX_FRAME_BLE4_OFFSET_RSSI(sz) + 1)
#define RX_FRAME_BLE4_OFFSET_TIMESTAMP(sz) (RX_FRAME_BLE4_OFFSET_STATUS(sz) + 2)

/*
 * BLE5 (AUX_ADV_IND Packet) RX FRAME
 *
 * +-------------------------------------------------------------------------------------------+
 * |____PDU____|_______________PAYLOAD(sz)___________________|_CRC_|_RSSI_|_STATUS_|_TIMESTAMP_|
 * |_ADV_|_LEN_|_ADV_ADDR_|_SERIAL_|__________DATA___________|     |      |        |           |
 * |1B   |1B   |10B       |2B      |Upto 18B                 |3B   |1B    |1B/2B   |4B         |
 * +-------------------------------------------------------------------------------------------+
 *
 * Note that BLE4 frames can transfer up to 37B of payload but are hard-coded
 * for 30B in this example
 */
#define RX_FRAME_BLE5_OFFSET_ADV_TYPE      0
#define RX_FRAME_BLE5_OFFSET_LEN           1
#define RX_FRAME_BLE5_OFFSET_ADV_ADDR      2
#define RX_FRAME_BLE5_OFFSET_SERIAL        12
#define RX_FRAME_BLE5_OFFSET_DATA          14
#define RX_FRAME_BLE5_OFFSET_CRC(sz)       (RX_FRAME_BLE5_OFFSET_ADV_ADDR + sz)
#define RX_FRAME_BLE5_OFFSET_RSSI(sz)      (RX_FRAME_BLE5_OFFSET_CRC(sz) + 3)
#define RX_FRAME_BLE5_OFFSET_STATUS(sz)    (RX_FRAME_BLE5_OFFSET_RSSI(sz) + 1)
#define RX_FRAME_BLE5_OFFSET_TIMESTAMP(sz) (RX_FRAME_BLE5_OFFSET_STATUS(sz) + 2)

/***** Prototypes *****/
/* Radio Receive operation callback */
static void rx_callback(RF_Handle h, RF_CmdHandle ch, RF_EventMask e);
/* RAT Timer timeout callback function */
static void rx_timeoutCb(RF_Handle h, RF_RatHandle rh, RF_EventMask e, uint32_t compareCaptureTime);

/***** Variable declarations *****/
static uint8_t packetReceived = false;
static uint16_t* crcOk;
static int8_t* rssi;

static RF_Object rfObject;
static RF_Handle rfHandle;
static RF_CmdHandle rxCmdHndl = 0; // Handle needed to abort the RX command
static volatile RF_RatConfigCompare ratCompareConfig;
static volatile RF_RatHandle ratHandle = RF_ALLOC_ERROR;
static volatile RF_Stat ratStatus;

static ApplicationConfig localConfig;

static volatile bool     bFirstPacket = true, bSecondPacket = true;
static volatile bool     bPacketsLost = false;
static volatile uint16_t nRxPkts = 0, nMissPkts = 0, nExpPkts = 0;
static volatile uint32_t throughputI = 0, throughputQ = 0;
static uint16_t timestampOffset = 0;
static uint16_t nRxTimeouts = 0;
static ratmr_t  startTime = 0, endTime = 0;
static ratmr_t  currTimerVal = 0, rxTimeoutVal = 0;
static uint32_t deltaTimeUs = 0;
static uint32_t deltaTimePacket = 0;
static uint32_t deltaTimePacketUs = 0;
static uint32_t pktIntervalEstUs  = 0;
static uint32_t nBits = 0;

#if RX_DEBUG_COLLECT_METRICS == 1
uint32_t deltaTimeRxPacketUsMin = 0xFFFFFFFF;
uint32_t deltaTimeRxPacketUsMax = 0;
uint32_t throughputMin = 0xFFFFFFFF;
uint32_t nBitsAtTpMin = 0, nBitsAtTpMax = 0;
uint32_t deltaTimeUsAtTpMin = 0, deltaTimeUsAtTpMax = 0;
uint32_t throughputMax = 0;
#endif

static rx_metrics rxMetrics = {
    .packetsReceived = 0,
    .packetsMissed   = 0,
    .packetsExpected = 0,
    .nRxTimeouts     = 0,
    .nPktsPerTimeout = RX_TIMEOUT_NPKTS,
    .rssi            = 0,
    .crcOK           = 0,
    .throughput      = 0
};

/*
Buffer which contains all Data Entries for receiving data.
Pragmas are needed to make sure this buffer is 4 byte aligned (requirement from the RF Core)
*/
#if defined(__TI_COMPILER_VERSION__)
#pragma DATA_ALIGN (rxDataEntryBuffer, 4);
#elif defined(__IAR_SYSTEMS_ICC__)
#pragma data_alignment = 4
#elif defined(__GNUC__)
__attribute__ ((aligned (4)))
#else
#error This compiler is not supported.
#endif
static uint8_t rxDataEntryBuffer[RF_QUEUE_DATA_ENTRY_BUFFER_SIZE(NUM_DATA_ENTRIES,
                                                                 MAX_LENGTH,
                                                                 NUM_APPENDED_BYTES)];

/* Receive queue for the RF Code to fill in data */
static dataQueue_t dataQueue;

/* General data entry structure (type = 0) */
rfc_dataEntryGeneral_t* currentDataEntry;

#if (defined SUPPORT_BLE)
rfc_bleGenericRxOutput_t rxStatistics_ble; // Output structure for RF_ble_cmdBleGenericRx
#endif

rfc_propRxOutput_t rxStatistics_prop; // Output structure for CMD_PROP_RX

/* Reset all the volatile variables */
static void rx_resetVariables(void)
{
    packetReceived = false;
    bFirstPacket = bSecondPacket =  true;
    bPacketsLost = false;
    nBits = nRxPkts = nMissPkts = nExpPkts = 0;
    nRxTimeouts = 0;
    startTime = endTime = 0;
    deltaTimeUs = deltaTimePacket = deltaTimePacketUs = 0;
    pktIntervalEstUs = 0;
    throughputI = throughputQ = 0;
}

/* Runs the receiving part of the test application and returns a result */
TestResult rx_runRxTest(const ApplicationConfig* config)
{
    TestResult testResult;

    if (config == NULL)
    {
        while(1);
    }
    memcpy((void *)&localConfig, config, sizeof(ApplicationConfig));

    RF_Params rfParams;
    RF_Params_init(&rfParams);

    if( RFQueue_defineQueue(&dataQueue,
                            rxDataEntryBuffer,
                            sizeof(rxDataEntryBuffer),
                            NUM_DATA_ENTRIES,
                            MAX_LENGTH + NUM_APPENDED_BYTES))
    {
        /* Failed to allocate space for all data entries */
        while(true);
    }

    RF_RatConfigCompare_init((RF_RatConfigCompare *)&ratCompareConfig);
    ratCompareConfig.callback = (RF_RatCallback)&rx_timeoutCb;
    ratCompareConfig.channel  = RF_RatChannelAny;

    rfc_CMD_PROP_RX_ADV_t *pAdvRxCmd = NULL;
    rfc_CMD_PROP_RX_t *pRxCmd = NULL;

    // Setup RX Commands
    switch (config->rfSetup)
    {
        case RfSetup_Custom:
            pRxCmd = &RF_cmdPropRx;
            break;
#if (defined SUPPORT_FSK_50KBPS)
        case RfSetup_Fsk:
            pRxCmd = &RF_cmdPropRx_fsk_50kbps;
            break;
#endif
#if (defined SUPPORT_PROP_2_4_GHZ)
#if !((defined CONFIG_LP_EM_CC2653P10)   || (defined CONFIG_CC2653P10RSL))
        case RfSetup_2_4_Fsk_250:
            pRxCmd = &RF_cmdPropRx_2_4G_fsk_250kbps;
            break;
#endif
        case RfSetup_2_4_Fsk_100:
            pRxCmd = &RF_cmdPropRx_2_4G_fsk_100kbps;
            break;
#endif
#if (defined SUPPORT_SLR)
        case RfSetup_Sl_lr:
            pRxCmd = &RF_cmdPropRx_sl_lr;
            break;
#endif
#if (defined SUPPORT_BLE)
        case RfSetup_Ble:
        case RfSetup_Ble5:
            RF_ble_cmdBleGenericRx.pOutput = &rxStatistics_ble;
            RF_ble_cmdBleGenericRx.pParams->pRxQ = &dataQueue;
            RF_ble_cmdBleGenericRx.pParams->bRepeat = 1;
            RF_ble_cmdBleGenericRx.pParams->rxConfig.bAutoFlushCrcErr = 1;
            RF_ble_cmdBleGenericRx.pParams->rxConfig.bAppendTimestamp = 1;
            RF_ble_cmdBleGenericRx.channel = 0xFF;
            RF_ble_cmdBleGenericRx.whitening.bOverride = 1;
            RF_ble_cmdBleGenericRx.whitening.init = config->frequencyTable[config->frequency].whitening;
            break;
#endif
#if (defined SUPPORT_FSK_200KBPS)
        case RfSetup_Fsk_200kbps:
            pAdvRxCmd = &RF_cmdPropRxAdv_fsk_200kbps;
            break;
#endif
        default:
            break;
    }

    if(NULL != pRxCmd)
    {
        pRxCmd->pQueue = &dataQueue;
        pRxCmd->pOutput = (uint8_t*)&rxStatistics_prop;
        pRxCmd->maxPktLen = MAX_LENGTH;
        pRxCmd->pktConf.bRepeatOk = 1;
        pRxCmd->pktConf.bRepeatNok = 1;
        pRxCmd->rxConf.bAutoFlushCrcErr = 1;
        pRxCmd->rxConf.bAutoFlushIgnored = 1;
        pRxCmd->rxConf.bAppendTimestamp = 1;
        pRxCmd->rxConf.bAppendStatus = 0x1;
    }
    else if (NULL != pAdvRxCmd)
    {
        pAdvRxCmd->pQueue = &dataQueue;
        pAdvRxCmd->pOutput = (uint8_t*)&rxStatistics_prop;
        pAdvRxCmd->maxPktLen = MAX_LENGTH;
        pAdvRxCmd->pktConf.bRepeatOk = 1;
        pAdvRxCmd->pktConf.bRepeatNok = 1;
        pAdvRxCmd->rxConf.bAutoFlushCrcErr = 1;
        pAdvRxCmd->rxConf.bAutoFlushIgnored = 1;
        pAdvRxCmd->rxConf.bAppendTimestamp = 1;
        pAdvRxCmd->rxConf.bAppendStatus = 1;
        pAdvRxCmd->rxConf.bIncludeCrc = 0x0;
        pAdvRxCmd->rxConf.bAppendRssi = 0x0;
        pAdvRxCmd->hdrConf.numLenBits = NUM_LEN_BITS;
    }

    /* Request access to the radio based on test case */
    switch (config->rfSetup)
    {
        case RfSetup_Custom:
            rfHandle = RF_open(&rfObject, &RF_prop, (RF_RadioSetup*)&RF_cmdPropRadioDivSetup, &rfParams);
            break;
#if (defined SUPPORT_FSK_50KBPS)
        case RfSetup_Fsk:
            RF_cmdPropRadioDivSetup_fsk_50kbps.centerFreq = config->frequencyTable[config->frequency].frequency;
            rfHandle = RF_open(&rfObject, &RF_prop_fsk_50kbps, (RF_RadioSetup*)&RF_cmdPropRadioDivSetup_fsk_50kbps, &rfParams);
            break;
#endif
#if (defined SUPPORT_PROP_2_4_GHZ)
#if !((defined CONFIG_LP_EM_CC2653P10)   || (defined CONFIG_CC2653P10RSL))
        case RfSetup_2_4_Fsk_250:
            RF_cmdPropRadioDivSetup_2_4G_fsk_250kbps.centerFreq = config->frequencyTable[config->frequency].frequency; //TODO: Is this line needed?
            rfHandle = RF_open(&rfObject, &RF_prop_2_4G_fsk_250kbps, (RF_RadioSetup*)&RF_cmdPropRadioDivSetup_2_4G_fsk_250kbps, &rfParams);
            break;
#endif
        case RfSetup_2_4_Fsk_100:
            RF_cmdPropRadioDivSetup_2_4G_fsk_100kbps.centerFreq = config->frequencyTable[config->frequency].frequency; //TODO: Is this line needed?
            rfHandle = RF_open(&rfObject, &RF_prop_2_4G_fsk_100kbps, (RF_RadioSetup*)&RF_cmdPropRadioDivSetup_2_4G_fsk_100kbps, &rfParams);
            break;
#endif
#if (defined SUPPORT_SLR)
        case RfSetup_Sl_lr:
            RF_cmdPropRadioDivSetup_sl_lr.centerFreq = config->frequencyTable[config->frequency].frequency;
            rfHandle = RF_open(&rfObject, &RF_prop_sl_lr, (RF_RadioSetup*)&RF_cmdPropRadioDivSetup_sl_lr, &rfParams);
            break;
#endif
#if (defined SUPPORT_BLE)
        case RfSetup_Ble:
        case RfSetup_Ble5:
            rfHandle = RF_open(&rfObject, &RF_modeBle, (RF_RadioSetup*)&RF_ble_cmdRadioSetup, &rfParams);
            break;
#endif
#if (defined SUPPORT_FSK_200KBPS)
        case RfSetup_Fsk_200kbps:
            rfHandle = RF_open(&rfObject, &RF_prop_fsk_200kbps, (RF_RadioSetup*)&RF_cmdPropRadioDivSetup_fsk_200kbps, &rfParams);
            break;
#endif
        default:
            break;
    }

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
        break;
#endif
    default:
        break;
    }

    /* Enter RX mode and stay forever in RX */
    switch (config->rfSetup)
    {
        case RfSetup_Custom:
            rxCmdHndl = RF_postCmd(rfHandle, (RF_Op*)&RF_cmdPropRx, RF_PriorityNormal, &rx_callback, RF_EventRxEntryDone);
            crcOk = &rxStatistics_prop.nRxOk;
            rssi = &rxStatistics_prop.lastRssi;
            break;
#if (defined SUPPORT_FSK_50KBPS)
        case RfSetup_Fsk:
            rxCmdHndl = RF_postCmd(rfHandle, (RF_Op*)&RF_cmdPropRx_fsk_50kbps, RF_PriorityNormal, &rx_callback, RF_EventRxEntryDone);
            crcOk = &rxStatistics_prop.nRxOk;
            rssi = &rxStatistics_prop.lastRssi;
            break;
#endif
#if (defined SUPPORT_PROP_2_4_GHZ)
#if !((defined CONFIG_LP_EM_CC2653P10)   || (defined CONFIG_CC2653P10RSL))
        case RfSetup_2_4_Fsk_250:
            rxCmdHndl = RF_postCmd(rfHandle, (RF_Op*)&RF_cmdPropRx_2_4G_fsk_250kbps, RF_PriorityNormal, &rx_callback, RF_EventRxEntryDone);
            crcOk = &rxStatistics_prop.nRxOk;
            rssi = &rxStatistics_prop.lastRssi;
            break;
#endif
        case RfSetup_2_4_Fsk_100:
            rxCmdHndl = RF_postCmd(rfHandle, (RF_Op*)&RF_cmdPropRx_2_4G_fsk_100kbps, RF_PriorityNormal, &rx_callback, RF_EventRxEntryDone);
            crcOk = &rxStatistics_prop.nRxOk;
            rssi = &rxStatistics_prop.lastRssi;
            break;
#endif
#if (defined SUPPORT_FSK_200KBPS)
        case RfSetup_Fsk_200kbps:
            rxCmdHndl = RF_postCmd(rfHandle, (RF_Op*)&RF_cmdPropRxAdv_fsk_200kbps, RF_PriorityNormal, &rx_callback, RF_EventRxEntryDone);
            crcOk = &rxStatistics_prop.nRxOk;
            rssi = &rxStatistics_prop.lastRssi;
            break;
#endif
#if  (defined SUPPORT_SLR)
        case RfSetup_Sl_lr:
            rxCmdHndl = RF_postCmd(rfHandle, (RF_Op*)&RF_cmdPropRx_sl_lr, RF_PriorityNormal, &rx_callback, RF_EventRxEntryDone);
            crcOk = &rxStatistics_prop.nRxOk;
            rssi = &rxStatistics_prop.lastRssi;
            break;
#endif
#if (defined SUPPORT_BLE)
        case RfSetup_Ble:
        case RfSetup_Ble5:
            rxCmdHndl = RF_postCmd(rfHandle, (RF_Op*)&RF_ble_cmdBleGenericRx, RF_PriorityNormal, &rx_callback, RF_EventRxEntryDone);
            crcOk = &rxStatistics_ble.nRxOk;
            rssi = &rxStatistics_ble.lastRssi;
            break;
#endif
        default:
            break;
    }

    *crcOk = 0;
    *rssi = 0;
    while(true)
    {
        if(packetReceived || bPacketsLost)
        {
            bPacketsLost   = false;
            packetReceived = false;

            rxMetrics.packetsReceived = nRxPkts;
            rxMetrics.packetsMissed   = nMissPkts;
            rxMetrics.packetsExpected = nExpPkts;
            rxMetrics.nRxTimeouts     = nRxTimeouts;
            rxMetrics.rssi            = *rssi;
            rxMetrics.crcOK           = *crcOk;
            rxMetrics.throughput      = throughputI;
            menu_updateRxScreen(&rxMetrics);
        }

        if (menu_isButtonPressed() || (nExpPkts >= localConfig.packetCount))
        {
            /* Stop the RAT Compare */
            (void)RF_ratDisableChannel(rfHandle, ratHandle);
            ratHandle = RF_ALLOC_ERROR;

            /* Force abort */
            (void)RF_cancelCmd(rfHandle, rxCmdHndl, ABORT_ABRUPT);
            (void)RF_pendCmd(rfHandle, rxCmdHndl, 0);
            RF_close(rfHandle);

            if((nExpPkts >= localConfig.packetCount))
            {
                rxMetrics.packetsReceived = nRxPkts;
                rxMetrics.packetsMissed   = nMissPkts;
                rxMetrics.packetsExpected = nExpPkts;
                rxMetrics.nRxTimeouts     = nRxTimeouts;
                rxMetrics.rssi            = *rssi;
                rxMetrics.crcOK           = *crcOk;
                rxMetrics.throughput      = throughputI;
                menu_updateRxScreen(&rxMetrics);

                testResult = TestResult_Finished;
            }
            else
            {
                testResult = TestResult_Aborted;
            }

            /* Reset all variables */
            rx_resetVariables();

            return(testResult);
        }
    }
}

void rx_callback(RF_Handle h, RF_CmdHandle ch, RF_EventMask e)
{
    uint16_t payloadLength    = 0;
    uint16_t pktSeqNum        = 0;
    uint16_t secondPktSeqNum  = 0;
    uint16_t nExpPktsLocal    = nExpPkts;
    uint32_t nFracBits        = 0;
    uint32_t nBitsDeltaTimeUs = 0;
    static uint16_t firstPktSeqNum = 0;
#if (defined SUPPORT_BLE)
    uint16_t pduAdvType = 0;
#endif

    /* For TC_HSM, the packet length and a pointer to the first byte in the payload can be found as follows:
     *
     * uint8_t payloadLength      = ((*(uint8_t*)(&currentDataEntry->data + 1)) << 8) | (*(uint8_t*)(&currentDataEntry->data));
     * uint8_t* packetDataPointer = (uint8_t*)(&currentDataEntry->data + 2);
     *
     * For the other test cases (TC_LRM, TC_OOK and TC_FSK), the packet length and first payload byte is found here:
     *
     * uint8_t payloadLength      = *(uint8_t*)(&currentDataEntry->data);
     * uint8_t* packetDataPointer = (uint8_t*)(&currentDataEntry->data + 1);
     */

    if (e & RF_EventRxEntryDone)
    {

        /* Get current unhandled data entry, point to next entry */
        currentDataEntry = RFQueue_getDataEntry();
        RFQueue_nextEntry();

        switch(localConfig.rfSetup)
        {
#if (defined SUPPORT_BLE)
            case RfSetup_Ble:
                pduAdvType    = *(uint8_t*)(&currentDataEntry->data + RX_FRAME_BLE4_OFFSET_ADV_TYPE);
                payloadLength = *(uint8_t*)(&currentDataEntry->data + RX_FRAME_BLE4_OFFSET_LEN);
                if(pduAdvType == ADV_NONCONN_IND)
                {
                    // Received a BLE4 (ADV_NONCONN_IND) packet
                    pktSeqNum     = (((*(uint8_t*)(&currentDataEntry->data + RX_FRAME_BLE4_OFFSET_SERIAL)) << 8) |
                                      (*(uint8_t*)(&currentDataEntry->data + (RX_FRAME_BLE4_OFFSET_SERIAL + 1))));
                    timestampOffset = RX_FRAME_BLE4_OFFSET_TIMESTAMP(payloadLength);
                }
                else // pduAdvType == AUX_ADV_IND
                {  // Received a BLE5 (AUX_ADV_IND) packet
                    pktSeqNum     = (((*(uint8_t*)(&currentDataEntry->data + RX_FRAME_BLE5_OFFSET_SERIAL)) << 8) |
                                      (*(uint8_t*)(&currentDataEntry->data + (RX_FRAME_BLE5_OFFSET_SERIAL + 1))));
                    timestampOffset = RX_FRAME_BLE5_OFFSET_TIMESTAMP(payloadLength);
                }
                break;
            case RfSetup_Ble5:
                pduAdvType    = *(uint8_t*)(&currentDataEntry->data + RX_FRAME_BLE5_OFFSET_ADV_TYPE);
                payloadLength = *(uint8_t*)(&currentDataEntry->data + RX_FRAME_BLE5_OFFSET_LEN);
                if(pduAdvType == ADV_NONCONN_IND)
                {
                    // Received a BLE4 (ADV_NONCONN_IND) packet
                    pktSeqNum     = (((*(uint8_t*)(&currentDataEntry->data + RX_FRAME_BLE4_OFFSET_SERIAL)) << 8) |
                                      (*(uint8_t*)(&currentDataEntry->data + (RX_FRAME_BLE4_OFFSET_SERIAL + 1))));
                    timestampOffset = RX_FRAME_BLE4_OFFSET_TIMESTAMP(payloadLength);
                }
                else // pduAdvType == AUX_ADV_IND
                {  // Received a BLE5 (AUX_ADV_IND) packet
                    pktSeqNum     = (((*(uint8_t*)(&currentDataEntry->data + RX_FRAME_BLE5_OFFSET_SERIAL)) << 8) |
                                      (*(uint8_t*)(&currentDataEntry->data + (RX_FRAME_BLE5_OFFSET_SERIAL + 1))));
                    timestampOffset = RX_FRAME_BLE5_OFFSET_TIMESTAMP(payloadLength);
                }
                break;
#endif
#if (defined SUPPORT_FSK_200KBPS)
            case RfSetup_Fsk_200kbps:
                payloadLength = *(uint8_t*)(&currentDataEntry->data + RX_FRAME_PROP_2GFSK200K_OFFSET_LEN);
                pktSeqNum     = (((*(uint8_t*)(&currentDataEntry->data + RX_FRAME_PROP_2GFSK200K_OFFSET_SERIAL)) << 8) |
                                  (*(uint8_t*)(&currentDataEntry->data + (RX_FRAME_PROP_2GFSK200K_OFFSET_SERIAL + 1))));
                timestampOffset = RX_FRAME_PROP_2GFSK200K_OFFSET_TIMESTAMP(payloadLength);
                break;
#endif
            default:
                payloadLength = *(uint8_t*)(&currentDataEntry->data + RX_FRAME_PROP_OFFSET_LEN);
                pktSeqNum     = (((*(uint8_t*)(&currentDataEntry->data + RX_FRAME_PROP_OFFSET_SERIAL)) << 8) |
                                  (*(uint8_t*)(&currentDataEntry->data + (RX_FRAME_PROP_OFFSET_SERIAL + 1))));
                timestampOffset = RX_FRAME_PROP_OFFSET_TIMESTAMP(payloadLength);
                break;
        }

        if(pktSeqNum > localConfig.packetCount)
        {
            /* TX off, Spurious packets received greater than the configured
             * packet count
             * _          _            _
             * V  OFF     V  ON <----  V  ON
             * |          |            |
             * TX         RX (PC=10)   Rogue TX
             *            ^
             *            +---+---+---+---+---+
             *            | 11| 12| 13| 14| 15| <---RX should ignore these
             *            +---+---+---+---+---+     packets
             */
            return;
        }
        if(pktSeqNum <= (nExpPktsLocal + RX_TIMEOUT_NPKTS))
        {
            /* No missing packets (pktSeqNum == nExpPkts)
             * +---+---+---+---+---+
             * | 0 | 1 | 2 | 3 | 4 |
             * +---+---+---+---+---+
             *          ^
             *          |
             *          pktSeqNum = nExpPkts = 2
             *          nExpPkts   = pktSeqNum + 1  = 3
             *
             * Missing packets
             * ((pktSeqNum < (nExpPkts + RX_TIMEOUT_NPKTS (5))) &&
             *  (pktSeqNum > nExpPkts))
             *
             * +---+---+                     +---+---+---+
             * | 0 | 1 |<---miss #2 to 4 --> | 5 | 6 | 7 |
             * +---+---+                     +---+---+---+
             *                               ^
             *                               |
             *                               pktSeqNum = 5
             *                               nExpPkts = 2
             *                               pktSeqNum < (nExpPkts + 5)
             *                               { nMissPkts += pktSeqNum - nExpPkts
             *                                  nExpPkts   = pktSeqNum + 1 }
             *
             * Catch duplicate packets interferer
             * ((pktSeqNum < (nExpPkts + RX_TIMEOUT_NPKTS (5))) &&
             *  (pktSeqNum < nExpPkts))
             * +---+---+---+---+---+---+---+---+---+
             * | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 6 | 8 |
             * +---+---+---+---+---+---+---+---+---+
             *                              ^
             *                              |
             *                              pktSeqNum = 6
             *                              nExpPkts  = 7
             *                              pktSeqNum < (nExpPkts + 5)
             *                              { nMissPkts  = nMissPkts + 1
             *                                nExpPkts   = nExpPkts + 1 }
             */
            if(pktSeqNum >= nExpPktsLocal)
            {
                nMissPkts += (pktSeqNum - nExpPktsLocal);
                nExpPktsLocal = pktSeqNum + 1;
            }
            else // pktSeqNum < nExpPkts
            {
                nMissPkts = nMissPkts + 1;
                nExpPktsLocal = nExpPktsLocal + 1;
            }
        }
        else if (pktSeqNum > (nExpPktsLocal + RX_TIMEOUT_NPKTS))
        {
            /* Catch spurious packets
             * (pktSeqNum > nExpPkts + RX_TIMEOUT_NPKTS(5))
             * +---+---+                      +----+---+---+
             * | 0 | 1 | <---miss #2 to 5 --> | 255| 7 | 8 |
             * +---+---+                      +----+---+---+
             *                                ^     ^
             *                                |     |
             *                                pktSeqNum = 255
             *                                nExpPkts  = 2
             *                                pktSeqNum > (nExpPkts + RX_TIMEOUT_NPKTS(5))
             *                                { nMissPkts++ (1)
             *                                  nExpPkts++  (3) }
             *                                      |
             *                                      |
             *                                      pktSeqNum = 7
             *                                      nExpPkts  = 3
             *                                      pktSeqNum < (nExpPkts + RX_TIMEOUT_NPKTS(5))
             *                                      { nMissPkts += (pktSeqNum - nExpPkts) (5)
             *                                        nExpPkts   = pktSeqNum +  1 (8) }
             *
             * In this case a spurious packet #255 replaced packet #6, this
             * packet was discarded and the number of missing packets
             * incremented by 1. However, the packets #2 to 5 that were lost to
             * interference were not counted. When the next legitimate packet
             * in sequence, #7, arrives the packets missing due to interference
             * will be accounted for
             *
             * Look for Drift in expected Sequence Number
             * If the total accumulated time (for N-1 packets up to this point)
             * divided by the packet sequence number (N-1) is less than 1
             * packet interval (+PKT_INTERVAL_TOL%) then the packet is not
             * spurious
             *
             */
            if((deltaTimeUs / pktSeqNum) > pktIntervalEstUs)
            {
                nExpPktsLocal++;
                nMissPkts++;
                bPacketsLost = true;

                /* Set the RAT compare value if all packets have not been
                 * received. Only do this after two packets were received, and
                 * the packet interval is determined; additionally if the RAT
                 * timer were already started it must be disabled before
                 * starting a new compare
                 */
                if((nExpPktsLocal < localConfig.packetCount) && (bSecondPacket == false))
                {
                    if(ratHandle >= 0)
                    {
                        (void)RF_ratDisableChannel(rfHandle, ratHandle);
                        ratHandle = RF_ALLOC_ERROR;
                    }
                    currTimerVal = RF_getCurrentTime();
                    ratCompareConfig.timeout = currTimerVal + rxTimeoutVal;
                    ratHandle = RF_ratCompare(rfHandle, (RF_RatConfigCompare *)&ratCompareConfig, NULL);
                    if(ratHandle == RF_ALLOC_ERROR)
                    {
                        // Issue with the RAT compare
                        while(1);
                    }
                }
                nExpPkts = nExpPktsLocal;
                return;
            }
            else
            {
                nExpPktsLocal   = pktSeqNum + 1;
            }
        }

        /* Stop the RAT Compare only if a legitimate packet is received */
        if(ratHandle >= 0)
        {
            ratStatus = RF_ratDisableChannel(rfHandle, ratHandle);
            ratHandle = RF_ALLOC_ERROR;
            if(ratStatus != RF_StatCmdDoneSuccess)
            {
                /* error disabling RAT channel */
                while(1);
            }
        }

        if(bFirstPacket)
        {
            firstPktSeqNum = pktSeqNum;
            startTime = *(uint32_t *)((uint8_t *)&(currentDataEntry->data) + timestampOffset);
            /* Lock out this read after the first packet */
            bFirstPacket = false;
#if RX_DEBUG_COLLECT_METRICS == 1
            // Reset the debug metrics
            deltaTimeRxPacketUsMin = 0xFFFFFFFF;
            deltaTimeRxPacketUsMax = 0;
            throughputMin = 0xFFFFFFFF;
            nBitsAtTpMin = 0, nBitsAtTpMax = 0;
            deltaTimeUsAtTpMin = 0, deltaTimeUsAtTpMax = 0;
            throughputMax = 0;
#endif
        }
        else
        {
            endTime = *(uint32_t *)((uint8_t *)&(currentDataEntry->data) + timestampOffset);
            /* Calculate the delta between two consecutive packets */
            deltaTimePacket   = endTime - startTime;
            deltaTimePacketUs = deltaTimePacket/RF_RAT_TICKS_PER_US;
#if RX_DEBUG_COLLECT_METRICS == 1
            if(deltaTimePacketUs < deltaTimeRxPacketUsMin)
            {
                deltaTimeRxPacketUsMin = deltaTimePacketUs;
            }
            else if(deltaTimePacketUs > deltaTimeRxPacketUsMax)
            {
                deltaTimeRxPacketUsMax = deltaTimePacketUs;
            }
#endif
            /* Set current packet time stamp as the start time for the next
             * delta calculation
             */
            startTime    = endTime;
            deltaTimeUs += deltaTimePacketUs;
#if defined(__TI_COMPILER_VERSION__)
            nFracBits   = _norm(deltaTimeUs);
#elif defined(__IAR_SYSTEMS_ICC__)
            nFracBits   = __CLZ(deltaTimeUs);
#elif defined(__GNUC__)
            nFracBits   = __builtin_clz(deltaTimeUs);
#else
#error This compiler is not supported.
#endif
            nBitsDeltaTimeUs = 32UL - nFracBits;

            if(nBitsDeltaTimeUs >= LOG2_100MILLION)
            {
                // deltaTimeUs is two orders of magnitude larger that 10^6
                // Throughput_I = (N_bits * 2^nFracBits)/(delT_us/10^6))
                //              = Throughput_I / 2^nFracBits
                // Shift N_bits up to occupy the MSbs and then divide by
                // (delT_us/10^6) which is at least 6 bits wide
                //   log2(1e8)-log2(1e6) = 6.643856189774724
                throughputI = (nBits << nFracBits)/ (deltaTimeUs / 1000000UL);
                throughputQ = throughputI & ((1 << nFracBits) - 1);
                throughputI = throughputI >> nFracBits;
            }
            else
            {
                // deltaTimeUs is smaller or comparable to 10^8
                // Throughput_I = (N_bits * 2^NFRAC)/(delT_us/10^6)
                //              = (N_bits)* (round(10^6*2^NFRAC)/delT_us)
                //              = (N_bits)* ((10^6*2^NFRAC + delT_us/2)/delT_us)
                //              = Throughput_I / 2^nFracBits
                throughputI = nBits * (((1000000UL << NFRACBITS) + (deltaTimeUs >> 1))/deltaTimeUs);
                throughputQ = throughputI & ((1 << NFRACBITS) - 1);
                throughputI = throughputI >> NFRACBITS;
            }
#if RX_DEBUG_COLLECT_METRICS == 1
            if(throughputI < throughputMin)
            {
                throughputMin = throughputI;
                nBitsAtTpMin  = nBits;
                deltaTimeUsAtTpMin = deltaTimeUs;
            }
            else if(throughputI > throughputMax )
            {
                throughputMax = throughputI;
                nBitsAtTpMax  = nBits;
                deltaTimeUsAtTpMax = deltaTimeUs;
            }
#endif
            /* Start the RAT Timer to timeout after NPKTS_TIMEOUT packet
             * intervals
             */
            if(bSecondPacket)
            {
                /* the division by (secondPktSeqNum - firstPktSeqNum) ensures
                 * that we get the delta time for one packet, even if we miss
                 * any of the starting packets
                 *
                 * +---+---+---+      +---+---+---+---+    +---+---+---+
                 * | 0 |   | 2 |   OR | 0 |   |   | 3 | OR |   | 1 | 2 |
                 * +---+---+---+      +---+---+---+---+    +---+---+---+
                 * <--delT->   ^      <--delT----->   ^        <dT>    ^
                 *             |                      |                |
                 *       packet interval = delT/2     |                |
                 *                    packet interval = delT/3         |
                 *                                     packet interval = delT/1
                 */
                secondPktSeqNum = pktSeqNum;
                rxTimeoutVal = RX_TIMEOUT_NPKTS * deltaTimePacket/(secondPktSeqNum - firstPktSeqNum);
                pktIntervalEstUs  = (uint32_t)((1 + PKT_INTERVAL_TOL/100.0) * (float)deltaTimePacketUs);
                /* Lock out this calculation after the second packet */
                bSecondPacket = false;
            }

            /* Set the RAT compare value if all packets have not been received*/
            if(nExpPktsLocal < localConfig.packetCount)
            {
                currTimerVal = RF_getCurrentTime();
                ratCompareConfig.timeout = currTimerVal + rxTimeoutVal;
                ratHandle = RF_ratCompare(rfHandle, (RF_RatConfigCompare *)&ratCompareConfig, NULL);
                if(ratHandle == RF_ALLOC_ERROR)
                {
                    // Issue with the RAT compare
                    while(1);
                }
            }
        }
        nBits        += (payloadLength << 3);

        /* Update packet metrics each time a good packet is received */
        nRxPkts++;
        nMissPkts = (nExpPktsLocal - nRxPkts);
        nExpPkts = nExpPktsLocal;
        packetReceived = true;
    }
}

void rx_timeoutCb(RF_Handle h, RF_RatHandle rh, RF_EventMask e, uint32_t compareCaptureTime)
{
    nExpPkts  += RX_TIMEOUT_NPKTS;
    nMissPkts += RX_TIMEOUT_NPKTS;
    nRxTimeouts++;
    bPacketsLost = true;

    /* Update timeout metrics */
    if(nExpPkts < localConfig.packetCount)
    {
        /*
         * The RAT compare event is one-shot, the channel deallocated prior to
         * this callback being issued. It is safe to issue the next RAT compare
         * event
         */
        ratCompareConfig.timeout = compareCaptureTime + rxTimeoutVal;
        ratHandle = RF_ratCompare(rfHandle, (RF_RatConfigCompare *)&ratCompareConfig, NULL);
        if(ratHandle == RF_ALLOC_ERROR)
        {
            // Issue with the RAT compare
            while(1);
        }
    }
    else // nExpPkts >= localConfig.packetCount
    {
        /* There should be no more packets beyond this point, these are
         * the final metrics
         */
        nMissPkts  = localConfig.packetCount - nRxPkts;
        nExpPkts   = localConfig.packetCount;
    }
}

