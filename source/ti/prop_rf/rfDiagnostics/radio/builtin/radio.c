/*
 * Copyright (c) 2015-2022, Texas Instruments Incorporated
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
/* Include Files*/
#include <stdlib.h>
#include <unistd.h>

/* DriverLib Header Files */
#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(driverlib/rf_data_entry.h)
#include DeviceFamily_constructPath(driverlib/rf_prop_mailbox.h)
#include DeviceFamily_constructPath(driverlib/rf_ble_mailbox.h)
#include DeviceFamily_constructPath(driverlib/rf_ieee_mailbox.h)
#include DeviceFamily_constructPath(driverlib/osc.h)

/* Application Specific Header Files*/
#include <TestMode.h>
#include "RFQueue.h"
#include "radio/radio.h"
#include "radio/radio_pvt.h"
#include "radio/builtin/PhySettings.h"


/* Board Header files */
#include "ti_drivers_config.h"
#include <ti/drivers/GPIO.h>

#define RADIO_MAX_LENGTH        255 // Max length byte the radio will accept
#define NUM_DATA_ENTRIES        2   // NOTE: Only two data entries supported at the moment
#define NUM_APPENDED_BYTES      11  // Max: CRC (4) + RSSI (1) + status (2) + timestamp (4)
#define ACTIVE_STATE            2
#define WHITENING_BLE_CH_17     0xD1
#define CHANNEL_BLE_CH_17       0x8C
#define MIN_BLE_ADV_PAYLOAD_LENGTH  12
#define MAX_BLE_ADV_PAYLOAD_LENGTH  37
#define SEQUENCE_NUMBER_SIZE    2

/* Function Prototypes */
static void rxCallback(RF_Handle h, RF_CmdHandle ch, RF_EventMask e);
static void rxCallbackAdvanced(RF_Handle h, RF_CmdHandle ch, RF_EventMask e);
static void rxCallbackBle(RF_Handle h, RF_CmdHandle ch, RF_EventMask e);
static uint8_t lsbFirst(uint8_t b);

/*
 * Buffer which contains all Data Entries for receiving data.
 * Pragma's are needed to make sure this buffer is 4 byte aligned (requirement from the RF Core)
 */
#if defined(__TI_COMPILER_VERSION__)
#pragma DATA_ALIGN (rxDataEntryBuffer, 4);
static uint8_t rxDataEntryBuffer[RF_QUEUE_DATA_ENTRY_BUFFER_SIZE(NUM_DATA_ENTRIES,
                                                                 RADIO_MAX_LENGTH,
                                                                 NUM_APPENDED_BYTES)];

#pragma DATA_ALIGN (advPktInfo, 4);
static rfc_ble5ExtAdvEntry_t advPktInfo;

#pragma DATA_ALIGN (txPacket, 4);
static uint8_t txPacket[RADIO_MAX_LENGTH];

#elif defined(__IAR_SYSTEMS_ICC__)
#pragma data_alignment = 4
static uint8_t rxDataEntryBuffer[RF_QUEUE_DATA_ENTRY_BUFFER_SIZE(NUM_DATA_ENTRIES,
                                                                 RADIO_MAX_LENGTH,
                                                                 NUM_APPENDED_BYTES)];

#pragma data_alignment = 4
static rfc_ble5ExtAdvEntry_t advPktInfo;

#pragma data_alignment = 4
static uint8_t txPacket[RADIO_MAX_LENGTH];

#elif defined(__GNUC__)
static uint8_t rxDataEntryBuffer[RF_QUEUE_DATA_ENTRY_BUFFER_SIZE(NUM_DATA_ENTRIES,
                                                                 RADIO_MAX_LENGTH,
                                                                NUM_APPENDED_BYTES)]
                                                                __attribute__((aligned(4)));

static rfc_ble5ExtAdvEntry_t advPktInfo __attribute__((aligned(4)));

static uint8_t txPacket[RADIO_MAX_LENGTH] __attribute__((aligned(4)));

#else
#error This compiler is not supported.
#endif

static uint8_t rxPacket[RADIO_MAX_LENGTH + NUM_APPENDED_BYTES - 1];       // The length byte is stored in a separate variable
static uint16_t extHeader[6] = {0x0201, 0x0403, 0x0605, 0xBEBA};    // Used when sending BLE packets to SmartRF Studio

static RF_Object rfObject;
static RF_Handle rfHandle;
static RF_Params rfParams;
static RF_ScheduleCmdParams schParams;
static bool isOpen = false;
static uint8_t currentPhyNumber = 0;

static rfc_CMD_PROP_TX_t RF_cmdPropTx;
static rfc_CMD_PROP_TX_ADV_t RF_cmdPropTxAdv;
static rfc_CMD_PROP_RX_t RF_cmdPropRx;
static rfc_CMD_PROP_RX_ADV_t RF_cmdPropRxAdv;
static rfc_CMD_BLE5_ADV_AUX_t RF_cmdBle5AdvAux;
static rfc_CMD_BLE5_GENERIC_RX_t RF_cmdBle5GenericRx;
static rfc_CMD_IEEE_TX_t RF_cmdIeeeTx;
static rfc_CMD_IEEE_RX_t RF_cmdIeeeRx;
static rfc_CMD_TX_TEST_t RF_cmdTxTest;
static rfc_CMD_RX_TEST_t RF_cmdRxTest;

static RF_CmdHandle rxCmdhandle;
static RF_CmdHandle txCmdhandle;
static dataQueue_t dataQueue;
static rfc_dataEntryGeneral_t* currentDataEntry;
static uint8_t* packetDataPointer;
static uint8_t packetLength;
static rfc_propRxOutput_t rxStatisticsProp;
static rfc_bleGenericRxOutput_t rxStatisticsBle;
static rfc_ieeeRxOutput_t rxStatisticsIeee;

static int32_t rssiAvg;
static int8_t rssiMin;
static int8_t rssiMax;

static uint16_t ieeeRxPackets = 0;
static uint16_t ieeeRxPacketsNOk = 0;
static uint16_t numRxSynOk = 0;

/*
 * Default channel and whitening settings in SmartRF Studio (for BLE channel 17).
 * Use this if the selected freq. is not a valid BLE channel, and
 * PhySettings_testType = PhySettings_TEST_STUDIO_COMPL
 */
static uint8_t whiteningSetting = WHITENING_BLE_CH_17;
static uint8_t channelSetting = CHANNEL_BLE_CH_17;

/* Init value for whitening for BLE channel  0 - 39 */
static int16_t bleWhitening[40][2] = {{2402, 0xE5}, {2404, 0xC0}, {2406, 0xC1}, {2408, 0xC2},
                                      {2410, 0xC3}, {2412, 0xC4}, {2414, 0xC5}, {2416, 0xC6},
                                      {2418, 0xC7}, {2420, 0xC8}, {2422, 0xC9}, {2424, 0xCA},
                                      {2426, 0xE6}, {2428, 0xCB}, {2430, 0xCC}, {2432, 0xCD},
                                      {2434, 0xCE}, {2436, 0xCF}, {2438, 0xD0}, {2440, 0xD1},
                                      {2442, 0xD2}, {2444, 0xD3}, {2446, 0xD4}, {2448, 0xD5},
                                      {2450, 0xD6}, {2452, 0xD7}, {2454, 0xD8}, {2456, 0xD9},
                                      {2458, 0xDA}, {2460, 0xDB}, {2462, 0xDC}, {2464, 0xDD},
                                      {2466, 0xDE}, {2468, 0xDF}, {2470, 0xE0}, {2472, 0xE1},
                                      {2474, 0xE2}, {2476, 0xE3}, {2478, 0xE4}, {2480, 0xE7}};

static void RF_runCmdSafe(RF_Handle h, RF_Op *pOp, RF_Priority ePri, RF_Callback pCb, RF_EventMask bmEvent)
{
    RF_EventMask terminationReason = RF_EventCmdAborted | RF_EventCmdPreempted;
    // Re-run if command was aborted due to SW TCXO compensation
    while(( terminationReason & RF_EventCmdAborted ) && ( terminationReason & RF_EventCmdPreempted ))
    {
        terminationReason = RF_runCmd(h, pOp, ePri, pCb, bmEvent);
    }
}

void Radio_Builtin_init(void)
{
    RF_Params_init(&rfParams);
    RF_ScheduleCmdParams_init(&schParams);

    // Initialize the test commands as these will be the same for all PHYs
    // CMD_TX_TEST is used for TestMode_CARRIER_WAVE and TEST_MODE_MODULATED
    RF_cmdTxTest.commandNo = 0x0808;
    RF_cmdTxTest.condition.rule = 0x1;
    RF_cmdTxTest.config.bFsOff = 0x1;
    RF_cmdTxTest.config.whitenMode = 0x2;
    RF_cmdTxTest.txWord = 0xAAAA;
    RF_cmdTxTest.endTrigger.triggerType = 0x1;
    RF_cmdTxTest.syncWord = 0x930B51DE;

    // CMD_RX_TEST is used for TEST_MODE_CONT_RX (must be called before reading the RSSI)
    RF_cmdRxTest.commandNo = 0x0807;
    RF_cmdRxTest.condition.rule = 0x1;
    RF_cmdRxTest.config.bFsOff = 0x1;
    RF_cmdRxTest.config.bNoSync = 0x1;
    RF_cmdRxTest.endTrigger.triggerType = 0x1;
}

void Radio_Builtin_cancelRx(void)
{
    RF_cancelCmd(rfHandle, rxCmdhandle, 0);
    RF_pendCmd(rfHandle, rxCmdhandle, RF_EventLastCmdDone);
}

void Radio_Builtin_cancelTx(void)
{
    RF_cancelCmd(rfHandle, txCmdhandle, 0);
    RF_pendCmd(rfHandle, txCmdhandle, RF_EventLastCmdDone);
}

bool Radio_Builtin_setupPhy(uint8_t phyIndex, uint8_t phyIndex2)
{
    // Close the rfHandler (if open)
    if (isOpen)
    {
        RF_yield(rfHandle);
        RF_close(rfHandle);
        isOpen = false;
    }

    // Find the correct setup command
    switch (PhySettings_supportedPhys[phyIndex].PhySettings_setupType)
    {
        case (PhySettings_SETUP_PROP):

            rfHandle = RF_open(&rfObject,
                               PhySettings_supportedPhys[phyIndex].RF_pMode,
                               (RF_RadioSetup*)(PhySettings_supportedPhys[phyIndex].RF_pCmdPropRadioDivSetup),
                               &rfParams);

            if (rfHandle == NULL)
            {
                while(1);
            }
            break;

        case (PhySettings_SETUP_PROP_PA):

            rfHandle = RF_open(&rfObject,
                               PhySettings_supportedPhys[phyIndex].RF_pMode,
                               (RF_RadioSetup*)(PhySettings_supportedPhys[phyIndex].RF_pCmdPropRadioDivSetupPa),
                               &rfParams);
            if (rfHandle == NULL)
            {
                while(1);
            }
            break;

        case (PhySettings_SETUP_BLE):

            rfHandle = RF_open(&rfObject,
                               PhySettings_supportedPhys[phyIndex].RF_pMode,
                               (RF_RadioSetup*)(PhySettings_supportedPhys[phyIndex].RF_pCmdBle5RadioSetup),
                               &rfParams);

            if (rfHandle == NULL)
            {
                while(1);
            }
            break;

        case (PhySettings_SETUP_BLE_PA):

            rfHandle = RF_open(&rfObject,
                               PhySettings_supportedPhys[phyIndex].RF_pMode,
                               (RF_RadioSetup*)(PhySettings_supportedPhys[phyIndex].RF_pCmdBle5RadioSetupPa),
                               &rfParams);
            if (rfHandle == NULL)
            {
                while(1);
            }
            break;

        case (PhySettings_SETUP_IEEE):

            rfHandle = RF_open(&rfObject,
                               PhySettings_supportedPhys[phyIndex].RF_pMode,
                               (RF_RadioSetup*)(PhySettings_supportedPhys[phyIndex].RF_pCmdRadioSetup),
                               &rfParams);
            if (rfHandle == NULL)
            {
                while(1);
            }
            break;

        case (PhySettings_SETUP_IEEE_PA):

            rfHandle = RF_open(&rfObject,
                               PhySettings_supportedPhys[phyIndex].RF_pMode,
                               (RF_RadioSetup*)(PhySettings_supportedPhys[phyIndex].RF_pCmdRadioSetupPa),
                               &rfParams);
            if (rfHandle == NULL)
            {
                while(1);
            }
            break;

        default:
            break;
    }
    currentPhyNumber = phyIndex;
    isOpen = true;
    return true;
}

bool Radio_Builtin_packetTx(uint16_t numPkts, uint32_t *pktLen)
{
    if (isOpen)
    {
        // Clear all commands
        memset(&RF_cmdPropTx, 0, sizeof(rfc_CMD_PROP_TX_t));
        memset(&RF_cmdPropTxAdv, 0, sizeof(rfc_CMD_PROP_TX_ADV_t));
        memset(&RF_cmdBle5AdvAux, 0, sizeof(rfc_CMD_BLE5_ADV_AUX_t));
        memset(&RF_cmdIeeeTx, 0, sizeof(rfc_CMD_IEEE_TX_t));

        uint16_t i;

        // Determine how to configure the different commands for the applications needs
        switch (PhySettings_supportedPhys[currentPhyNumber].PhySettings_apiType)
        {
            case (PhySettings_API_PROP):

                if(PhySettings_supportedPhys[currentPhyNumber].PhySettings_cmdType == PhySettings_CMD_STANDARD)
                {
                    //------------------------------------------------------------------------------
                    // PhySettings_API_PROP + PhySettings_CMD_STANDARD + PhySettings_TEST_RF_PERF
                    // PhySettings_API_PROP + PhySettings_CMD_STANDARD + PhySettings_TEST_STUDIO_COMPL
                    //
                    // Standard TX command for Proprietary Phy's used for both
                    // RF performance testing and for packet test with SmartRF Studio
                    //------------------------------------------------------------------------------
                    memcpy(&RF_cmdPropTx,
                           PhySettings_supportedPhys[currentPhyNumber].RF_pCmdPropTx,
                           sizeof(rfc_CMD_PROP_TX_t));

                    // Modify application specific settings
                    RF_cmdPropTx.startTrigger.triggerType = TRIG_NOW;
                    RF_cmdPropTx.pktLen = ((uint8_t)(*pktLen));
                    RF_cmdPropTx.pPkt = txPacket;

                    // Create the packet content (same packet for every iteration)
                    // 2 bytes seq. number/packet counter added later
                    for (i = SEQUENCE_NUMBER_SIZE; i < ((uint8_t)(*pktLen)); i++)
                    {
                        txPacket[i] = rand();
                    }
                }
                else // PhySettings_CMD_ADVANCED
                {
                    memcpy(&RF_cmdPropTxAdv,
                           PhySettings_supportedPhys[currentPhyNumber].RF_pCmdPropTxAdv,
                           sizeof(rfc_CMD_PROP_TX_ADV_t));

                    if(PhySettings_supportedPhys[currentPhyNumber].PhySettings_testType == PhySettings_TEST_RF_PERF)
                    {
                        //--------------------------------------------------------------------------
                        // PhySettings_API_PROP + PhySettings_CMD_ADVANCED + PhySettings_TEST_RF_PERF
                        //
                        // Advanced TX command for Proprietary Phy's used for RF performance testing.
                        // The packet format is altered compared to what is imported from sysConfig,
                        // to utilize "standard" packet format with a 8 bits length field following
                        // the sync word
                        //--------------------------------------------------------------------------

                        // Modify application specific settings
                        RF_cmdPropTxAdv.startTrigger.triggerType = TRIG_NOW;
                        RF_cmdPropTxAdv.numHdrBits = 0;
                        RF_cmdPropTxAdv.pktLen = ((uint8_t)(*pktLen)) + 1;
                        RF_cmdPropTxAdv.pPkt = txPacket;

                        // Create the packet content (same packet for every iteration)
                        // For the advanced TX command the length must manually be written
                        // to the packet.
                        // 2 bytes seq. number/packet number added later
                        txPacket[0] = ((uint8_t)(*pktLen));

                        for (i = (SEQUENCE_NUMBER_SIZE + 1); i < (((uint8_t)(*pktLen)) + 1); i++)
                        {
                            txPacket[i] = rand();
                        }
                    }
                    else // PhySettings_TEST_STUDIO_COMPL
                    {
                        //--------------------------------------------------------------------------
                        // PhySettings_API_PROP + PhySettings_CMD_ADVANCED + PhySettings_TEST_STUDIO_COMPL
                        //
                        // Advanced TX format for Proprietary PHy's used for testing towards SmartRF
                        // Studio. Settings imported from sysConfig uses a 2 bytes long header with an
                        // 11 bits long length "byte" (802.15.4g compliant). Packet header needs to
                        // contain info regarding whitening and CRC. Default here is whitening
                        // enabled and 2 bytes CRC. The payload is also being written to the packet
                        // LSB first
                        //--------------------------------------------------------------------------

                        // Modify application specific settings
                        RF_cmdPropTxAdv.startTrigger.triggerType = TRIG_NOW;
                        RF_cmdPropTxAdv.pktLen = ((uint8_t)(*pktLen));
                        RF_cmdPropTxAdv.pPkt = txPacket;

                        // Create the packet content (same packet for every iteration)
                        // For the advanced TX command the length must manually be written to the
                        // packet. 2 bytes seq. number/packet number added later
                        txPacket[0] = ((uint8_t)(*pktLen));
                        txPacket[1] = 0x18; // Whitening and 2 bytes CRC enabled
                                            // (default in SmartRF Studio for 802.15.4g mode)
                        for (i = (SEQUENCE_NUMBER_SIZE + 2); i < ((uint8_t)(*pktLen)); i++)
                        {
                            // For the payload, SmartRF Studio uses LSbit first by default for
                            // 802.15.4g mode
                            txPacket[i] = lsbFirst(rand());
                        }
                    }
                }
                break;

            case (PhySettings_API_IEEE):

                //------------------------------------------------------------------------------
                // PhySettings_API_IEEE +
                // PhySettings_CMD_STANDARD/PhySettings_CMD_ADVANCED +
                // PhySettings_TEST_RF_PERF/PhySettings_TEST_STUDIO_COMPL
                //
                // IEEE PHY is not supported by the PROP API, so both RF performance testing and
                // packet test with SmartRF Studio is done sing the IEEE API.
                // Since PROP API is not supported, PhySettings_CMD_STANDARD/PhySettings_CMD_ADVANCED are don't care
                //------------------------------------------------------------------------------
                memcpy(&RF_cmdIeeeTx,
                       PhySettings_supportedPhys[currentPhyNumber].RF_pCmdIeeeTx,
                       sizeof(rfc_CMD_IEEE_TX_t));

                // Modify application specific settings
                RF_cmdIeeeTx.startTrigger.triggerType = TRIG_NOW;
                RF_cmdIeeeTx.payloadLen = ((uint8_t)(*pktLen));
                RF_cmdIeeeTx.pPayload = txPacket;

                // Create the packet content (same packet for every iteration)
                // 2 bytes seq. number/packet counter added later
                for (i = SEQUENCE_NUMBER_SIZE; i < ((uint8_t)(*pktLen)); i++)
                {
                    txPacket[i] = rand();
                }
                break;

            case (PhySettings_API_BLE):

                if(PhySettings_supportedPhys[currentPhyNumber].PhySettings_testType ==  PhySettings_TEST_RF_PERF)
                {
                    //------------------------------------------------------------------------------
                    // PhySettings_API_BLE + PhySettings_TEST_RF_PERF
                    //
                    // Standard TX command used for BLE Phy's for RF testing
                    //------------------------------------------------------------------------------
                    memcpy(&RF_cmdPropTx,
                           PhySettings_supportedPhys[currentPhyNumber].RF_pCmdPropTx,
                           sizeof(rfc_CMD_PROP_TX_t));

                    // Modify application specific settings
                    RF_cmdPropTx.startTrigger.triggerType = TRIG_NOW;
                    RF_cmdPropTx.pktLen = ((uint8_t)(*pktLen));
                    RF_cmdPropTx.pPkt = txPacket;

                    // Create the packet content (same packet for every iteration)
                    // 2 bytes seq. number/packet counter added later
                    for (i = SEQUENCE_NUMBER_SIZE; i < ((uint8_t)(*pktLen)); i++)
                    {
                        txPacket[i] = rand();
                    }
                }
                else // PhySettings_TEST_STUDIO_COMPL
                {
                    //------------------------------------------------------------------------------
                    // PhySettings_API_BLE + PhySettings_TEST_STUDIO_COMPL
                    //------------------------------------------------------------------------------
                    memcpy(&RF_cmdBle5AdvAux,
                           PhySettings_supportedPhys[currentPhyNumber].RF_pCmdBle5AdvAux,
                           sizeof(rfc_CMD_BLE5_ADV_AUX_t));

                    RF_cmdBle5AdvAux.startTrigger.triggerType = TRIG_NOW;

                    if ((((uint8_t)(*pktLen)) >= MIN_BLE_ADV_PAYLOAD_LENGTH) && (((uint8_t)(*pktLen)) <= MAX_BLE_ADV_PAYLOAD_LENGTH))
                    {
                        advPktInfo.advDataLen = ((uint8_t)(*pktLen)) - 10;
                    }
                    else
                    {
                        advPktInfo.advDataLen = MAX_BLE_ADV_PAYLOAD_LENGTH - 10;
                    }
                    advPktInfo.extHdrFlags = 9;                     // Extended header flags as
                                                                    // defines in BLE
                    advPktInfo.extHdrInfo.advMode = 0;              // Advertiser mode: Non-
                                                                    // connectable, non-scannable
                    advPktInfo.extHdrInfo.length = 9;               // Extended header length
                    advPktInfo.pAdvData = (uint8_t*)&txPacket;      // Pointer to buffer containing
                                                                    // advData. If advDataLen = 0,
                                                                    // pointer may be NULL.
                    advPktInfo.pExtHeader = (uint8_t*)&extHeader;   // Pointer to buffer containing
                                                                    // extended header

                    RF_cmdBle5AdvAux.pParams->pAdvPkt = (uint8_t*)&advPktInfo;

                    // Create the packet content (same packet for every iteration)
                    // 2 bytes seq. number/packet counter added later
                    for (i = SEQUENCE_NUMBER_SIZE; i < (((uint8_t)(*pktLen)) - 10); i++)
                    {
                        txPacket[i] = rand();
                    }
                    //------------------------------------------------------------------------------
                }
                break;

            default:
                break;
        }

        // Set the frequency
        RF_runCmdSafe(rfHandle, (RF_Op*)PhySettings_supportedPhys[currentPhyNumber].RF_pCmdFs, RF_PriorityNormal, NULL, 0);

        uint16_t packetCounter;
        uint32_t cmdStatus = 0;

        for(packetCounter = 1; packetCounter <= numPkts; packetCounter++)
        {
            switch (PhySettings_supportedPhys[currentPhyNumber].PhySettings_apiType)
            {
                case (PhySettings_API_PROP):

                    if (PhySettings_supportedPhys[currentPhyNumber].PhySettings_cmdType == PhySettings_CMD_STANDARD)
                    {
                        //--------------------------------------------------------------------------
                        // PhySettings_API_PROP + PhySettings_CMD_STANDARD + PhySettings_TEST_RF_PERF
                        // PhySettings_API_PROP + PhySettings_CMD_STANDARD + PhySettings_TEST_STUDIO_COMPL
                        //
                        // When using CMD_PROP_TX, the length is sent automatically,
                        // hence the sequence numbers starts at index 0
                        //--------------------------------------------------------------------------
                        txPacket[0] = (uint8_t)(packetCounter >> 8);
                        txPacket[1] = (uint8_t)(packetCounter);

                        RF_runCmdSafe(rfHandle, (RF_Op*)&RF_cmdPropTx, RF_PriorityNormal, NULL, 0);
                        cmdStatus = ((volatile RF_Op*)&RF_cmdPropTx)->status;

                        // Add a delay for SmartRF Studio to keep up
                        usleep(10000);
                    }
                    else // PhySettings_CMD_ADVANCED
                    {
                        if (PhySettings_supportedPhys[currentPhyNumber].PhySettings_testType == PhySettings_TEST_RF_PERF)
                        {
                            //----------------------------------------------------------------------
                            // PhySettings_API_PROP + PhySettings_CMD_ADVANCED + PhySettings_TEST_RF_PERF
                            //
                            // When using CMD_PROP_TX_ADV, the length must be written to the packet
                            // manually, hence the sequence numbers starts at index 1
                            //----------------------------------------------------------------------
                            txPacket[1] = (uint8_t)(packetCounter >> 8);
                            txPacket[2] = (uint8_t)(packetCounter);
                        }
                        else // PhySettings_TEST_STUDIO_COMPL
                        {
                            //----------------------------------------------------------------------
                            // PhySettings_API_PROP + PhySettings_CMD_ADVANCED + PhySettings_TEST_STUDIO_COMPL
                            //
                            // When using CMD_PROP_TX_ADV (and 802.15.4g mode), the length must be
                            // written to the 2 bytes long header in the packet manually, hence the
                            // sequence numbers starts at index 2. Payload needs to be written
                            // LSB first to be compliant with SmartRF Studio
                            //----------------------------------------------------------------------
                            txPacket[2] = lsbFirst((uint8_t)(packetCounter >> 8));
                            txPacket[3] = lsbFirst((uint8_t)(packetCounter));
                        }
                        RF_runCmdSafe(rfHandle, (RF_Op*)&RF_cmdPropTxAdv, RF_PriorityNormal, NULL, 0);
                        cmdStatus = ((volatile RF_Op*)&RF_cmdPropTxAdv)->status;

                        // Add a delay for SmartRF Studio to keep up
                        usleep(10000);
                    }
                    break;

                case (PhySettings_API_IEEE):

                    //--------------------------------------------------------------------------
                    // PhySettings_API_IEEE +
                    // PhySettings_CMD_STANDARD/PhySettings_CMD_ADVANCED +
                    // PhySettings_TEST_RF_PERF/PhySettings_TEST_STUDIO_COMPL
                    //
                    // When using CMD_IEEE_TX, the length is sent automatically,
                    // hence the sequence numbers starts at index 0
                    //--------------------------------------------------------------------------
                    txPacket[0] = (uint8_t)(packetCounter >> 8);
                    txPacket[1] = (uint8_t)(packetCounter);

                    txCmdhandle = RF_scheduleCmd(rfHandle, (RF_Op*)&RF_cmdIeeeTx, &schParams, NULL, RF_EventLastFGCmdDone);
                    RF_pendCmd(rfHandle, txCmdhandle, RF_EventLastFGCmdDone);
                    cmdStatus = ((volatile RF_Op*)&RF_cmdIeeeTx)->status;

                    // Add a delay for SmartRF Studio to keep up
                    usleep(10000);
                    break;

                case (PhySettings_API_BLE):

                    if (PhySettings_supportedPhys[currentPhyNumber].PhySettings_testType == PhySettings_TEST_RF_PERF)
                    {
                        //--------------------------------------------------------------------------
                        // PhySettings_API_BLE + PhySettings_TEST_RF_PERF
                        //
                        // When using CMD_PROP_TX, the length is sent automatically,
                        // so the sequence numbers starts at index 0
                        //--------------------------------------------------------------------------
                        txPacket[0] = (uint8_t)(packetCounter >> 8);
                        txPacket[1] = (uint8_t)(packetCounter);

                        RF_runCmdSafe(rfHandle, (RF_Op*)&RF_cmdPropTx, RF_PriorityNormal, NULL, 0);
                        cmdStatus = ((volatile RF_Op*)&RF_cmdPropTx)->status;
                    }
                    else // PhySettings_TEST_STUDIO_COMPL
                    {
                        //--------------------------------------------------------------------------
                        // PhySettings_API_BLE + PhySettings_TEST_STUDIO_COMPL
                        //--------------------------------------------------------------------------
                        txPacket[0] = (uint8_t)(packetCounter >> 8);
                        txPacket[1] = (uint8_t)(packetCounter);

                        RF_runCmdSafe(rfHandle, (RF_Op*)&RF_cmdBle5AdvAux, RF_PriorityNormal, NULL, 0);
                        cmdStatus = ((volatile RF_Op*)&RF_cmdBle5AdvAux)->status;

                        // Add a delay for SmartRF Studio to keep up
                        // (10 ms packet interval is used for BLE)
                        usleep(10000);
                        //--------------------------------------------------------------------------
                    }
                    break;

                default:
                    break;
            }

            // Exit packet TX if the TX commands does not run properly
            if ((cmdStatus != PROP_DONE_OK) && (cmdStatus != IEEE_DONE_OK) && (cmdStatus != BLE_DONE_OK))
            {
                return false;
            }

            // Print the packet number (starting at 1)
            TestMode_perTxPrintMsg(packetCounter);

            GPIO_toggle(CONFIG_GPIO_GLED);
        }
        return true;
    }
    else
    {
        return false;
    }
}

bool Radio_Builtin_packetRx(uint16_t pktLen)
{
    ieeeRxPackets = 0;
    ieeeRxPacketsNOk = 0;
    numRxSynOk = 0;

    if (isOpen)
    {
        // Clear all commands
        memset(&RF_cmdPropRx, 0, sizeof(rfc_CMD_PROP_RX_t));
        memset(&RF_cmdPropRxAdv, 0, sizeof(rfc_CMD_PROP_RX_ADV_t));
        memset(&RF_cmdBle5GenericRx, 0, sizeof(rfc_CMD_BLE5_GENERIC_RX_t));
        memset(&RF_cmdIeeeRx, 0, sizeof(rfc_CMD_IEEE_RX_t));

        memset(&rxStatisticsProp, 0, sizeof(rfc_propRxOutput_t));
        memset(&rxStatisticsBle, 0, sizeof(rfc_bleGenericRxOutput_t));
        memset(&rxStatisticsIeee, 0, sizeof(rfc_ieeeRxOutput_t));

        rssiAvg = 0;
        rssiMin = 127;
        rssiMax = -127;

        if(RFQueue_defineQueue(&dataQueue,
                               rxDataEntryBuffer,
                               sizeof(rxDataEntryBuffer),
                               NUM_DATA_ENTRIES,
                               RADIO_MAX_LENGTH + NUM_APPENDED_BYTES,
                               0))
        {
             while(1);
        }

        // Determine how to configure the different commands for the applications needs
        switch (PhySettings_supportedPhys[currentPhyNumber].PhySettings_apiType)
        {
            case (PhySettings_API_PROP):

                if(PhySettings_supportedPhys[currentPhyNumber].PhySettings_cmdType == PhySettings_CMD_STANDARD)
                {
                    //-------------------------------------------------------------------------------
                    // PhySettings_API_PROP + PhySettings_CMD_STANDARD + PhySettings_TEST_RF_PERF
                    // PhySettings_API_PROP + PhySettings_CMD_STANDARD + PhySettings_TEST_STUDIO_COMPL
                    //-------------------------------------------------------------------------------
                    memcpy(&RF_cmdPropRx,
                           PhySettings_supportedPhys[currentPhyNumber].RF_pCmdPropRx,
                           sizeof(rfc_CMD_PROP_RX_t));

                    RF_cmdPropRx.pQueue = &dataQueue;
                    RF_cmdPropRx.rxConf.bAutoFlushIgnored = 1;
                    RF_cmdPropRx.rxConf.bAutoFlushCrcErr = 1;
                    RF_cmdPropRx.maxPktLen = pktLen;
                    RF_cmdPropRx.pktConf.bRepeatOk = 1;
                    RF_cmdPropRx.pktConf.bRepeatNok = 1;
                    RF_cmdPropRx.pOutput = (uint8_t*)&rxStatisticsProp;
                }
                else // PhySettings_CMD_ADVANCED
                {
                    //-------------------------------------------------------------------------------
                    // PhySettings_API_PROP + PhySettings_CMD_ADVANCED + PhySettings_TEST_RF_PERF
                    // PhySettings_API_PROP + PhySettings_CMD_ADVANCED + PhySettings_TEST_STUDIO_COMPL
                    //
                    // Common setting for PhySettings_TEST_RF_PERF and PhySettings_TEST_STUDIO_COMPL
                    //-------------------------------------------------------------------------------
                    memcpy(&RF_cmdPropRxAdv,
                           PhySettings_supportedPhys[currentPhyNumber].RF_pCmdPropRxAdv,
                           sizeof(rfc_CMD_PROP_RX_ADV_t));

                    RF_cmdPropRxAdv.pQueue = &dataQueue;
                    RF_cmdPropRxAdv.rxConf.bAutoFlushIgnored = 1;
                    RF_cmdPropRxAdv.rxConf.bAutoFlushCrcErr = 1;
                    RF_cmdPropRxAdv.maxPktLen = pktLen;
                    RF_cmdPropRxAdv.pktConf.bRepeatOk = 1;
                    RF_cmdPropRxAdv.pktConf.bRepeatNok = 1;
                    RF_cmdPropRxAdv.pktConf.filterOp = 0;
                    RF_cmdPropRxAdv.pOutput = (uint8_t*)&rxStatisticsProp;

                    // Addition settings for PhySettings_TEST_RF_PERF
                    if(PhySettings_supportedPhys[currentPhyNumber].PhySettings_testType == PhySettings_TEST_RF_PERF)
                    {
                        //---------------------------------------------------------------------------
                        // PhySettings_API_PROP + PhySettings_CMD_ADVANCED + PhySettings_TEST_RF_PERF
                        //---------------------------------------------------------------------------
                        RF_cmdPropRxAdv.pktConf.bCrcIncHdr = 1;
                        RF_cmdPropRxAdv.hdrConf.numHdrBits = 8;
                        RF_cmdPropRxAdv.hdrConf.numLenBits = 8;
                        RF_cmdPropRxAdv.lenOffset = 0;
                    }
                }
                break;

            case (PhySettings_API_IEEE):

                // Define a queue for the IEEE PHY
                if(RFQueue_defineQueue(&dataQueue,
                                       rxDataEntryBuffer,
                                       sizeof(rxDataEntryBuffer),
                                       NUM_DATA_ENTRIES,
                                       RADIO_MAX_LENGTH + NUM_APPENDED_BYTES,
                                       1))
                {
                     while(1);
                }

                //----------------------------------------------------------------------------------
                // PhySettings_API_IEEE +
                // PhySettings_CMD_STANDARD/PhySettings_CMD_ADVANCED +
                // PhySettings_TEST_RF_PERF/PhySettings_TEST_STUDIO_COMPL
                //
                // IEEE PHY is not supported by the PROP API, so both RF performance testing and
                // packet test with SmartRF Studio is done sing the IEEE API.
                // Since PROP API is not supported, PhySettings_CMD_STANDARD/PhySettings_CMD_ADVANCED are don't care
                //----------------------------------------------------------------------------------
                memcpy(&RF_cmdIeeeRx,
                       PhySettings_supportedPhys[currentPhyNumber].RF_pCmdIeeeRx,
                       sizeof(rfc_CMD_IEEE_RX_t));

                RF_cmdIeeeRx.pRxQ = &dataQueue;
                RF_cmdIeeeRx.pOutput = &rxStatisticsIeee;
                break;

            case (PhySettings_API_BLE):

                if(PhySettings_supportedPhys[currentPhyNumber].PhySettings_testType ==  PhySettings_TEST_RF_PERF)
                {
                    //-------------------------------------------------------------------------------
                    // PhySettings_API_BLE + PhySettings_TEST_RF_PERF
                    //-------------------------------------------------------------------------------
                    memcpy(&RF_cmdPropRx,
                           PhySettings_supportedPhys[currentPhyNumber].RF_pCmdPropRx,
                           sizeof(rfc_CMD_PROP_RX_t));

                    RF_cmdPropRx.pQueue = &dataQueue;
                    RF_cmdPropRx.rxConf.bAutoFlushIgnored = 1;
                    RF_cmdPropRx.rxConf.bAutoFlushCrcErr = 1;
                    RF_cmdPropRx.maxPktLen = pktLen;
                    RF_cmdPropRx.pktConf.bRepeatOk = 1;
                    RF_cmdPropRx.pktConf.bRepeatNok = 1;
                    RF_cmdPropRx.pOutput = (uint8_t*)&rxStatisticsProp;
                }
                else
                {
                    //-------------------------------------------------------------------------------
                    // PhySettings_API_BLE PhySettings_TEST_STUDIO_COMPL
                    //-------------------------------------------------------------------------------
                    memcpy(&RF_cmdBle5GenericRx,
                           PhySettings_supportedPhys[currentPhyNumber].RF_pCmdBle5GenericRx,
                           sizeof(rfc_CMD_BLE5_GENERIC_RX_t));

                    RF_cmdBle5GenericRx.pParams->pRxQ = &dataQueue;
                    RF_cmdBle5GenericRx.pParams->bRepeat = 1;
                    RF_cmdBle5GenericRx.pParams->rxConfig.bAutoFlushCrcErr = 1;
                    RF_cmdBle5GenericRx.pParams->rxConfig.bAutoFlushIgnored = 1;
                    RF_cmdBle5GenericRx.pParams->rxConfig.bAutoFlushEmpty = 1;
                    RF_cmdBle5GenericRx.pOutput = &rxStatisticsBle;
                }
                break;

            default:
                break;
        }

        uint32_t cmdStatus = 0;

        // Set the frequency
        RF_postCmd(rfHandle, (RF_Op*)PhySettings_supportedPhys[currentPhyNumber].RF_pCmdFs, RF_PriorityNormal, NULL, 0);

        switch (PhySettings_supportedPhys[currentPhyNumber].PhySettings_apiType)
        {
            case (PhySettings_API_PROP):

                if (PhySettings_supportedPhys[currentPhyNumber].PhySettings_cmdType == PhySettings_CMD_STANDARD)
                {
                    //-------------------------------------------------------------------------------
                    // PhySettings_API_PROP + PhySettings_CMD_STANDARD + PhySettings_TEST_RF_PERF
                    // PhySettings_API_PROP + PhySettings_CMD_STANDARD + PhySettings_TEST_STUDIO_COMPL
                    //-------------------------------------------------------------------------------
                    rxCmdhandle = RF_postCmd(rfHandle,
                                             (RF_Op*)&RF_cmdPropRx,
                                             RF_PriorityNormal,
                                             &rxCallback,
                                             (RF_EventRxEntryDone|RF_EventMdmSoft));

                    // Allow time for the radio to enter RX
                    CPUdelay(20000);
                    cmdStatus = ((volatile RF_Op*)&RF_cmdPropRx)->status;
                }
                else // PhySettings_CMD_ADVANCED
                {
                    //-------------------------------------------------------------------------------
                    // PhySettings_API_PROP + PhySettings_CMD_ADVANCED + PhySettings_TEST_RF_PERF
                    // PhySettings_API_PROP + PhySettings_CMD_ADVANCED + PhySettings_TEST_STUDIO_COMPL
                    //
                    // The difference between the PhySettings_TEST_RF_PERF and PhySettings_TEST_STUDIO_COMPL
                    // is in the callback
                    //-------------------------------------------------------------------------------
                    rxCmdhandle = RF_postCmd(rfHandle,
                                             (RF_Op*)&RF_cmdPropRxAdv,
                                             RF_PriorityNormal,
                                             &rxCallbackAdvanced,
                                             (RF_EventRxEntryDone | RF_EventMdmSoft));

                    // Allow time for the radio to enter RX
                    CPUdelay(20000);
                    cmdStatus = ((volatile RF_Op*)&RF_cmdPropRxAdv)->status;
                }
                break;

            case (PhySettings_API_IEEE):

                //----------------------------------------------------------------------------------
                // PhySettings_API_IEEE +
                // PhySettings_CMD_STANDARD/PhySettings_CMD_ADVANCED +
                // PhySettings_TEST_RF_PERF/PhySettings_TEST_STUDIO_COMPL
                //----------------------------------------------------------------------------------
                rxCmdhandle = RF_postCmd(rfHandle,
                                         (RF_Op*)&RF_cmdIeeeRx,
                                         RF_PriorityNormal,
                                         &rxCallback,
                                         (RF_EventRxEntryDone | RF_EventRxOk | RF_EventRxNOk | RF_EventMdmSoft));

                // Allow time for the radio to enter RX
                CPUdelay(20000);
                cmdStatus = ((volatile RF_Op*)&RF_cmdIeeeRx)->status;
                break;

            case (PhySettings_API_BLE):

                if (PhySettings_supportedPhys[currentPhyNumber].PhySettings_testType == PhySettings_TEST_RF_PERF)
                {
                    //-------------------------------------------------------------------------------
                    // PhySettings_API_BLE + PhySettings_TEST_RF_PERF
                    //
                    // Using PhySettings_CMD_STANDARD, so same code as for PhySettings_API_PROP
                    //-------------------------------------------------------------------------------
                    rxCmdhandle = RF_postCmd(rfHandle,
                                             (RF_Op*)&RF_cmdPropRx,
                                             RF_PriorityNormal,
                                             &rxCallback,
                                             (RF_EventRxEntryDone | RF_EventMdmSoft));

                    // Allow time for the radio to enter RX
                    CPUdelay(20000);
                    cmdStatus = ((volatile RF_Op*)&RF_cmdPropRx)->status;
                }
                else
                {
                    //-------------------------------------------------------------------------------
                    // PhySettings_API_BLE + PhySettings_TEST_STUDIO_COMPL
                    //-------------------------------------------------------------------------------
                    rxCmdhandle = RF_postCmd(rfHandle,
                                             (RF_Op*)&RF_cmdBle5GenericRx,
                                             RF_PriorityNormal,
                                             &rxCallbackBle,
                                             (RF_EventRxEntryDone | RF_EventMdmSoft));

                    // Allow time for the radio to enter RX
                    CPUdelay(20000);
                    cmdStatus = ((volatile RF_Op*)&RF_cmdBle5GenericRx)->status;
                }
                break;

            default:
               break;
        }

        if (cmdStatus != ACTIVE_STATE)
        {
            return false;
        }
        else
        {
            return true;
        }

    }
    else
    {
        return false;
    }
}

bool Radio_Builtin_contTx(bool cw)
{
    if (isOpen)
    {
        uint32_t cmdStatus;

        // Set the frequency
        RF_runCmdSafe(rfHandle, (RF_Op*)PhySettings_supportedPhys[currentPhyNumber].RF_pCmdFs, RF_PriorityNormal, NULL, 0);

        if (cw)
        {
            RF_cmdTxTest.config.bUseCw = 1;
        }
        else
        {
            RF_cmdTxTest.config.bUseCw = 0;
        }
        txCmdhandle = RF_postCmd(rfHandle, (RF_Op*)&RF_cmdTxTest, RF_PriorityNormal, NULL, 0);

        cmdStatus = ((volatile RF_Op*)&RF_cmdTxTest)->status;

        if (cmdStatus != ACTIVE_STATE)
        {
            return false;
        }
        return true;
    }
    return false;
}

bool Radio_Builtin_contRx(void)
{
    if (isOpen)
    {
        uint32_t cmdStatus;

        // Set the frequency
        RF_runCmdSafe(rfHandle, (RF_Op*)PhySettings_supportedPhys[currentPhyNumber].RF_pCmdFs, RF_PriorityNormal, NULL, 0);

        rxCmdhandle = RF_postCmd(rfHandle, (RF_Op*)&RF_cmdRxTest, RF_PriorityNormal, NULL, 0);

        cmdStatus = RF_cmdRxTest.status;

        if (cmdStatus != ACTIVE_STATE)
        {
            return false;
        }
        return true;
    }
    return false;
}

void Radio_Builtin_setFreq(uint32_t freq, uint32_t mdrFreq)
{
    uint16_t centerFreq;
    uint16_t fractFreq;
    (void)mdrFreq;

    centerFreq = (uint16_t)(freq / 1000000);
    fractFreq  = (uint16_t)(((uint64_t)freq-((uint64_t)centerFreq*1000000))*65536/1000000);

    PhySettings_supportedPhys[currentPhyNumber].RF_pCmdFs->frequency = centerFreq;
    PhySettings_supportedPhys[currentPhyNumber].RF_pCmdFs->fractFreq = fractFreq;

    // Find the correct setup command and change the centerFreq of the setup command as well
    switch (PhySettings_supportedPhys[currentPhyNumber].PhySettings_setupType)
    {
        case (PhySettings_SETUP_PROP):
            PhySettings_supportedPhys[currentPhyNumber].RF_pCmdPropRadioDivSetup->centerFreq = centerFreq;
            break;

        case (PhySettings_SETUP_PROP_PA):
            PhySettings_supportedPhys[currentPhyNumber].RF_pCmdPropRadioDivSetupPa->centerFreq = centerFreq;
            break;

        case (PhySettings_SETUP_IEEE_PA):
            break;

        case (PhySettings_SETUP_IEEE):
            break;

        default: // PhySettings_SETUP_BLE and PhySettings_SETUP_BLE_PA

            // For BLE, the frequency is not dependent on the setup command.
            // Instead, the BLE RX and TX commands needs to be modified.
            // This is only necessary if the application is to communicate with SmartRF Studio
            // using CMD_BLE5_ADV_AUX and CMD_BLE5_GENERIC_RX
            if(PhySettings_supportedPhys[currentPhyNumber].PhySettings_testType == PhySettings_TEST_STUDIO_COMPL)
            {
                uint8_t j;
                whiteningSetting = WHITENING_BLE_CH_17;
                channelSetting = CHANNEL_BLE_CH_17;
                for (j = 0; j < 40; j++)
                {
                    if(bleWhitening[j][0] == (freq/1000000))
                    {
                        whiteningSetting = bleWhitening[j][1];
                        channelSetting = (freq/1000000) - 2400 + 100;
                        break;
                    }
                }
                PhySettings_supportedPhys[currentPhyNumber].RF_pCmdBle5AdvAux->channel = channelSetting;
                PhySettings_supportedPhys[currentPhyNumber].RF_pCmdBle5GenericRx->channel = channelSetting;
                PhySettings_supportedPhys[currentPhyNumber].RF_pCmdBle5AdvAux->whitening.bOverride = 1;
                PhySettings_supportedPhys[currentPhyNumber].RF_pCmdBle5GenericRx->whitening.bOverride = 1;
                PhySettings_supportedPhys[currentPhyNumber].RF_pCmdBle5AdvAux->whitening.init = (whiteningSetting & 0x7F);
                PhySettings_supportedPhys[currentPhyNumber].RF_pCmdBle5GenericRx->whitening.init = (whiteningSetting & 0x7F);
            }
            break;
    }

    if (isOpen)
    {
        RF_yield(rfHandle);
    }
}

void Radio_Builtin_getFreq(RF_Frequency *freqs)
{
    freqs->freq = (uint32_t)RADIO_ERROR_VALUE;
    freqs->mdrFreq = 0;

    if (isOpen)
    {
        freqs->freq = (PhySettings_supportedPhys[currentPhyNumber].RF_pCmdFs->frequency)*1000000;
        freqs->freq += ((((uint64_t)(PhySettings_supportedPhys[currentPhyNumber].RF_pCmdFs->fractFreq)*1000000))/65536);
    }
}

bool Radio_Builtin_setPower(int8_t i8TxPowerDbm)
{
    RF_TxPowerTable_Value newValue;

    newValue =
        RF_TxPowerTable_findValue((RF_TxPowerTable_Entry *)PhySettings_supportedPhys[currentPhyNumber].RF_pTxPowerTable,
        i8TxPowerDbm);
    if(newValue.rawValue == RF_TxPowerTable_INVALID_VALUE)
    {
        return false;
    }

    RF_Stat rfStatus = RF_setTxPower(rfHandle, newValue);

    if(rfStatus == RF_StatSuccess)
    {
        return true;
    }
    else
    {
        return false;
    }
}

int8_t Radio_Builtin_getPower(void)
{
    int8_t txPowerDbm = RADIO_ERROR_VALUE;
    if (isOpen)
    {
        RF_TxPowerTable_Entry *rfPowerTable = NULL;
        RF_TxPowerTable_Value currValue = RF_getTxPower(rfHandle);

        if(currValue.rawValue != RF_TxPowerTable_INVALID_VALUE)
        {
            rfPowerTable = (RF_TxPowerTable_Entry *)PhySettings_supportedPhys[currentPhyNumber].RF_pTxPowerTable;
            txPowerDbm = RF_TxPowerTable_findPowerLevel(rfPowerTable, currValue);
        }
    }
    return txPowerDbm;
}

int8_t Radio_Builtin_getCurrentRssi(void)
{
    int8_t currentRSSI = RADIO_ERROR_VALUE;
    if (isOpen)
    {
        currentRSSI = RF_getRssi(rfHandle);
    }
    return currentRSSI;
}

int32_t Radio_Builtin_getAvgRssi(void)
{
    int32_t avgRSSI;

    if((PhySettings_supportedPhys[currentPhyNumber].PhySettings_apiType == PhySettings_API_BLE) &
       (PhySettings_supportedPhys[currentPhyNumber].PhySettings_testType == PhySettings_TEST_STUDIO_COMPL))
    {
        avgRSSI = (int32_t)(rssiAvg/rxStatisticsBle.nRxOk);
    }
    else if(PhySettings_supportedPhys[currentPhyNumber].PhySettings_apiType == PhySettings_API_IEEE)
    {
        avgRSSI = (int32_t)(rssiAvg/ieeeRxPackets);
    }
    else
    {
        avgRSSI = (int32_t)(rssiAvg/rxStatisticsProp.nRxOk);
    }

    return avgRSSI;
}

int8_t Radio_Builtin_getMinRssi(void)
{
    return rssiMin;
}

int8_t Radio_Builtin_getMaxRssi(void)
{
    return rssiMax;
}

uint16_t Radio_Builtin_getNumRxPackets(void)
{
    int16_t numRxPackets;

    if((PhySettings_supportedPhys[currentPhyNumber].PhySettings_apiType == PhySettings_API_BLE) &
       (PhySettings_supportedPhys[currentPhyNumber].PhySettings_testType == PhySettings_TEST_STUDIO_COMPL))
    {
        numRxPackets = rxStatisticsBle.nRxOk;
    }
    else if(PhySettings_supportedPhys[currentPhyNumber].PhySettings_apiType == PhySettings_API_IEEE)
    {
        numRxPackets = (int16_t)ieeeRxPackets;
    }
    else
    {
        numRxPackets = rxStatisticsProp.nRxOk;
    }

    return numRxPackets;
}

uint16_t Radio_Builtin_getNumRxPacketsNok(void)
{
    uint16_t numRxPacketsNok;

    if((PhySettings_supportedPhys[currentPhyNumber].PhySettings_apiType == PhySettings_API_BLE) &
       (PhySettings_supportedPhys[currentPhyNumber].PhySettings_testType == PhySettings_TEST_STUDIO_COMPL))
    {
        numRxPacketsNok = rxStatisticsBle.nRxNok;
    }
    else if(PhySettings_supportedPhys[currentPhyNumber].PhySettings_apiType == PhySettings_API_IEEE)
    {
        numRxPacketsNok = ieeeRxPacketsNOk;
    }
    else
    {
        numRxPacketsNok = rxStatisticsProp.nRxNok;
    }

    return numRxPacketsNok;
}

uint16_t Radio_Builtin_getNumRxSync(void)
{
    return numRxSynOk;
}

uint8_t Radio_Builtin_getRxPhyIndex(void)
{
    return currentPhyNumber;
}

bool Radio_Builtin_checkPacketLength(uint32_t *perPktLen)
{
    uint32_t pktLen = *perPktLen;
    if((pktLen > 1) && (pktLen <= RADIO_MAX_LENGTH))
    {
        if(PhySettings_supportedPhys[currentPhyNumber].PhySettings_apiType == PhySettings_API_BLE &&
           PhySettings_supportedPhys[currentPhyNumber].PhySettings_testType == PhySettings_TEST_STUDIO_COMPL)
        {
            if(pktLen > MAX_BLE_ADV_PAYLOAD_LENGTH)
            {
                *perPktLen = MAX_BLE_ADV_PAYLOAD_LENGTH;
            }
            else if(pktLen < MIN_BLE_ADV_PAYLOAD_LENGTH)
            {
                *perPktLen = MIN_BLE_ADV_PAYLOAD_LENGTH;
            }
        }

        return true;
    }
    else
    {
        return false;
    }
}

/*
 * Called for every received packet and command done
 * Used with the standard RX command (CMD_PROP_RX) for both PhySettings_API_PROP and PhySettings_API_BLE,
 * as long as PhySettingsExample_cmdType = PhySettings_CMD_STANDARD. Also used for all PhySettings_API_IEEE
 */
void rxCallback(RF_Handle h, RF_CmdHandle ch, RF_EventMask e)
{
    if (e & RF_EventRxEntryDone)
    {
        // Get current un-handled data entry
        currentDataEntry = RFQueue_getDataEntry();

        packetLength = *(uint8_t*)(&currentDataEntry->data);
        packetDataPointer = (uint8_t*)(&currentDataEntry->data + 1);

        // Copy the payload to the rxPacket variable
        // If this is an IEEE packet (IEEE API) the packetLength includes
        // the optional appended status bytes as well (default 2 bytes if not
        // doing any changes to the default IEEE settings)
        memcpy(rxPacket, packetDataPointer, packetLength);

        RFQueue_nextEntry();

        if (PhySettings_supportedPhys[currentPhyNumber].PhySettings_apiType == PhySettings_API_PROP)
        {
            rssiAvg += (int32_t)(rxStatisticsProp.lastRssi);
            if(rxStatisticsProp.lastRssi > rssiMax)
            {
                rssiMax = rxStatisticsProp.lastRssi;
            }
            if(rxStatisticsProp.lastRssi < rssiMin)
            {
                rssiMin = rxStatisticsProp.lastRssi;
            }
        }
        else // PhySettings_API_IEEE
        {
            rssiAvg += (int32_t)(rxStatisticsIeee.lastRssi);
            if(rxStatisticsIeee.lastRssi > rssiMax)
            {
                rssiMax = rxStatisticsIeee.lastRssi;
            }
            if(rxStatisticsIeee.lastRssi < rssiMin)
            {
                rssiMin = rxStatisticsIeee.lastRssi;
            }
        }

        GPIO_toggle(CONFIG_GPIO_RLED);
    }

    if(e & RF_EventRxOk)
    {
        ieeeRxPackets++;
    }

    if(e & RF_EventRxNOk)
    {
        ieeeRxPacketsNOk++;
    }

    if(e & RF_EventMdmSoft)
    {
        numRxSynOk++;
    }
}

/*
 * Called for every received packet and command done
 * Used with the advanced RX command (CMD_PROP_RX_ADV) for PhySettings_API_PROP, and supports
 * both RF Performance testing (PhySettings_TEST_RF_PERF) and SmartRF Studio compliance
 * testing (PhySettings_TEST_STUDIO_COMPL)
 */
void rxCallbackAdvanced(RF_Handle h, RF_CmdHandle ch, RF_EventMask e)
{
    uint8_t i;

    if (e & RF_EventRxEntryDone)
    {
        // Get current un-handled data entry
        currentDataEntry = RFQueue_getDataEntry();

        packetLength = *(uint8_t*)(&currentDataEntry->data);

        if (PhySettings_supportedPhys[currentPhyNumber].PhySettings_testType == PhySettings_TEST_RF_PERF)
        {
            packetDataPointer = (uint8_t*)(&currentDataEntry->data + 1);

            // Copy the payload to the rxPacket variable
            memcpy(rxPacket, packetDataPointer, packetLength);
        }
        else // PhySettings_TEST_STUDIO_COMPL
        {
            packetDataPointer = (uint8_t*)(&currentDataEntry->data + 2);

            // Copy the payload to the rxPacket variable
            memcpy(rxPacket, packetDataPointer, packetLength - 2);
            for(i = 0; i < (packetLength - 2); i++)
            {
                rxPacket[i] = lsbFirst(rxPacket[i]);
            }
        }
        RFQueue_nextEntry();

        rssiAvg += (int32_t)(rxStatisticsProp.lastRssi);
        if(rxStatisticsProp.lastRssi > rssiMax)
        {
            rssiMax = rxStatisticsProp.lastRssi;
        }
        if(rxStatisticsProp.lastRssi < rssiMin)
        {
            rssiMin = rxStatisticsProp.lastRssi;
        }

        GPIO_toggle(CONFIG_GPIO_RLED);
    }
}

/*
 * Called for every received packet and command done
 * Used with the BLE RX command (CMD_BLE5_GENERIC_RX) for PhySettings_API_BLE when performing
 * SmartRF Studio compliance testing (PhySettings_TEST_STUDIO_COMPL)
 */
void rxCallbackBle(RF_Handle h, RF_CmdHandle ch, RF_EventMask e)
{
    if (e & RF_EventRxEntryDone)
    {
        // Get current unhandled data entry
        currentDataEntry = RFQueue_getDataEntry();

        packetLength = *(uint8_t*)(&currentDataEntry->data + 1) - 10;
        packetDataPointer = (uint8_t*)(&currentDataEntry->data + 12);

        // Copy the payload to the rxPacket variable
        memcpy(rxPacket, packetDataPointer, packetLength);

        RFQueue_nextEntry();

        rssiAvg += (int32_t)(rxStatisticsBle.lastRssi);
        if(rxStatisticsProp.lastRssi > rssiMax)
        {
            rssiMax = rxStatisticsBle.lastRssi;
        }
        if(rxStatisticsProp.lastRssi < rssiMin)
        {
            rssiMin = rxStatisticsBle.lastRssi;
        }

        GPIO_toggle(CONFIG_GPIO_RLED);
    }
}

/*
 * Function to convert from MSB to LSB first.
 * Necessary to be compliant with the default SmartRF Studio implementation for receiving
 * and transmitting 802.15.4g packets
 */
uint8_t lsbFirst(uint8_t b)
{
    b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
    b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
    b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
    return b;
}

uint8_t Radio_Builtin_getNumSupportedPhys(void)
{
    return PhySettings_numSupportedPhys;
}

char *Radio_Builtin_getPhyName(uint8_t phyIndex)
{
    if (phyIndex < PhySettings_numSupportedPhys)
        return PhySettings_supportedPhys[phyIndex].PhySettings_phyName;
    else
        return "Bad Index.";
}

void Radio_Builtin_registerFxns(Radio_Fxns *fxns){
    fxns->init = Radio_Builtin_init;
    fxns->setupPhy = Radio_Builtin_setupPhy;
    fxns->enableMdr = NULL;
    fxns->packetTx = Radio_Builtin_packetTx;
    fxns->packetRx = Radio_Builtin_packetRx;
    fxns->packetMdrTx = NULL;
    fxns->packetMdrRx = NULL;
    fxns->packetMdrCsTx = NULL;
    fxns->packetCsTx = NULL;
    fxns->contTx = Radio_Builtin_contTx;
    fxns->contRx = Radio_Builtin_contRx;
    fxns->setFreq = Radio_Builtin_setFreq;
    fxns->getFreq = Radio_Builtin_getFreq;
    fxns->setPower = Radio_Builtin_setPower;
    fxns->getPower = Radio_Builtin_getPower;
    fxns->getCurrentRssi = Radio_Builtin_getCurrentRssi;
    fxns->getAvgRssi = Radio_Builtin_getAvgRssi;
    fxns->getMinRssi = Radio_Builtin_getMinRssi;
    fxns->getMaxRssi = Radio_Builtin_getMaxRssi;
    fxns->cancelRx = Radio_Builtin_cancelRx;
    fxns->cancelTx = Radio_Builtin_cancelTx;
    fxns->getNumRxPackets = Radio_Builtin_getNumRxPackets;
    fxns->getNumRxPacketsNok = Radio_Builtin_getNumRxPacketsNok;
    fxns->getNumRxSync = Radio_Builtin_getNumRxSync;
    fxns->getNumRxPackets = Radio_Builtin_getNumRxPackets;
    fxns->getRxPhyIndex = Radio_Builtin_getRxPhyIndex;
    fxns->checkPacketLength = Radio_Builtin_checkPacketLength;
    fxns->getNumSupportedPhys = Radio_Builtin_getNumSupportedPhys;
    fxns->getPhyName = Radio_Builtin_getPhyName;
    fxns->getRadioVersion = NULL;
}
