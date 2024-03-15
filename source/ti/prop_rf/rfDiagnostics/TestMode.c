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
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include <stdbool.h>

#include "radio/radio.h"
#include "TestMode.h"
#include "radio/builtin/PhySettings.h"
#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(inc/hw_ccfg.h)
#include DeviceFamily_constructPath(inc/hw_ccfg_simple_struct.h)

#if defined(DUAL_RADIOS)
static uint8_t currentMode[2] = {0};
#else
static uint8_t currentMode[1] = {0};
#endif

static printMsgFxn pTestMsgFxn;


static void TestMode_Exit(uint8_t testMode)
{
    if ((testMode == TestMode_MODULATED) || (testMode == TestMode_CARRIER_WAVE))
    {
        Radio_cancelTx();
    }
    else // TestMode_PER_RX or TestMode_CONT_RX
    {
        Radio_cancelRx();

        if(testMode == TestMode_PER_RX)
        {
            int8_t rssiMin = Radio_getMinRssi();
            int8_t rssiMax = Radio_getMaxRssi();
            int32_t rssiAvg = Radio_getAvgRssi();
            uint16_t packetCount = Radio_getNumRxPackets();
            uint16_t packetCountNok = Radio_getNumRxPacketsNok();
            uint16_t rxSyncCount = Radio_getNumRxSync();
            uint8_t rxPhyId = Radio_getRxPhyIndex();
            char *phyName = Radio_getPhyName(rxPhyId);

            pTestMsgFxn(TestMode_EXIT, packetCount, packetCountNok, rxSyncCount, false, rssiAvg, (int32_t)(rssiMin), (int32_t)(rssiMax), phyName);
        }
        else if(testMode == TestMode_MDR_RX)
        {
            int8_t rssiMin = Radio_getMinRssi();
            int8_t rssiMax = Radio_getMaxRssi();
            int32_t rssiAvg = Radio_getAvgRssi();
            uint16_t packetCount = Radio_getNumRxPackets();
            uint16_t packetCountNok = Radio_getNumRxPacketsNok();
            uint16_t rxSyncCount = Radio_getNumRxSync();
            uint8_t rxPhyId = Radio_getRxPhyIndex();
            char *phyName = Radio_getPhyName(rxPhyId);

            pTestMsgFxn(TestMode_EXIT, packetCount, packetCountNok, rxSyncCount, false, rssiAvg, (int32_t)(rssiMin), (int32_t)(rssiMax), phyName);
        }
    }
}

void TestMode_perTxPrintMsg(uint16_t numPackets)
{
    pTestMsgFxn(TestMode_PER_TX, numPackets, 0, 0, false, 0, 0, 0, NULL);
}

uint8_t TestMode_read(void)
{
    return currentMode[Radio_getCurrentRadio()-1U];
}


TestMode_Status TestMode_write(uint32_t newMode, uint32_t perNumPkts, uint32_t perPktLen, printMsgFxn pMsgFxn)
{
    TestMode_Status status = TestMode_Status_Fail;
    pTestMsgFxn = pMsgFxn;

    uint8_t *currentRadioTestMode = &currentMode[Radio_getCurrentRadio()-1U];

    if(newMode < TestMode_NUMBER_OF_MODES)
    {
        //------------------------------------------------------------------------------------------
        // TestMode_CARRIER_WAVE
        //------------------------------------------------------------------------------------------
        if((*currentRadioTestMode == TestMode_EXIT) && (newMode == TestMode_CARRIER_WAVE))
        {
            // Call function to transmit CW
            if (Radio_contTx(TestMode_SELECT_CW))
            {
                *currentRadioTestMode = newMode;
                status = TestMode_Status_Success;
            }
            else
            {
                status = TestMode_Status_Fail;
            }
        }

        //------------------------------------------------------------------------------------------
        // TEST_MODE_MODULATED
        //------------------------------------------------------------------------------------------
        else if((*currentRadioTestMode == TestMode_EXIT) && (newMode == TestMode_MODULATED))
        {
            // Call function to transmit modulated
            if (Radio_contTx(TestMode_SELECT_MODULATED))
            {
                *currentRadioTestMode = newMode;
                status = TestMode_Status_Success;
            }
            else
            {
                status = TestMode_Status_Fail;
            }
        }

        //------------------------------------------------------------------------------------------
        // TEST_MODE_CONT_RX
        //------------------------------------------------------------------------------------------
        else if((*currentRadioTestMode == TestMode_EXIT) && (newMode == TestMode_CONT_RX))
        {
            // Call function to enter continuous RX
            if (Radio_contRx())
            {
                *currentRadioTestMode = newMode;
                status = TestMode_Status_Success;
            }
            else
            {
                status = TestMode_Status_Fail;
            }
        }

        //------------------------------------------------------------------------------------------
        // TEST_MODE_PER_TX
        //------------------------------------------------------------------------------------------
        else if((*currentRadioTestMode == TestMode_EXIT) && (newMode == TestMode_PER_TX))
        {
            // Start Transmitting perNumPkts packets of length perPktLen
            if (Radio_packetTx((uint16_t)perNumPkts, &perPktLen))
            {
                // Set test mode back to 0 when returning
                *currentRadioTestMode = TestMode_EXIT;
                status = TestMode_Status_Success;
            }
            else
            {
                status = TestMode_Status_Fail;
            }

            pTestMsgFxn(TestMode_PER_TX, 0, 0, 0, true, 0, 0, 0, NULL);
        }

        //------------------------------------------------------------------------------------------
        // TEST_MODE_PER_RX
        //------------------------------------------------------------------------------------------
        else if((*currentRadioTestMode == TestMode_EXIT) && (newMode == TestMode_PER_RX))
        {
            // Start PacketRx
            if (Radio_packetRx((uint16_t)perPktLen))
            {
                *currentRadioTestMode = TestMode_PER_RX;
                status = TestMode_Status_Success;
                pTestMsgFxn(TestMode_PER_RX, 0, 0, 0, false, 0, 0, 0, NULL);
            }
            else
            {
                status = TestMode_Status_Fail;
            }

        }

        //------------------------------------------------------------------------------------------
        // TEST_MODE_MDR_TX
        //------------------------------------------------------------------------------------------
        else if((*currentRadioTestMode == TestMode_EXIT) && (newMode == TestMode_MDR_TX))
        {
            // Start Transmitting perNumPkts packets of length perPktLen
            if (Radio_packetMdrTx((uint16_t)perNumPkts, &perPktLen))
            {
                // Set test mode back to 0 when returning
                *currentRadioTestMode = TestMode_EXIT;
                status = TestMode_Status_Success;
            }
            else
            {
                status = TestMode_Status_Fail;
            }

            pTestMsgFxn(TestMode_MDR_TX, 0, 0, 0, true, 0, 0, 0, NULL);
        }

        //------------------------------------------------------------------------------------------
        // TEST_MODE_MDR_RX
        //------------------------------------------------------------------------------------------
        else if((*currentRadioTestMode == TestMode_EXIT) && (newMode == TestMode_MDR_RX))
        {
            // Start PacketRx
            if (Radio_packetMdrRx((uint16_t)perPktLen))
            {
                *currentRadioTestMode = TestMode_MDR_RX;
                status = TestMode_Status_Success;
                pTestMsgFxn(TestMode_MDR_RX, 0, 0, 0, false, 0, 0, 0, NULL);
            }
            else
            {
                status = TestMode_Status_Fail;
            }

        }

        //------------------------------------------------------------------------------------------
        // TEST_MODE_MDR_CS_TX
        //------------------------------------------------------------------------------------------
        else if((*currentRadioTestMode == TestMode_EXIT) && (newMode == TestMode_MDR_CS_TX))
        {
            // Start Transmitting perNumPkts packets of length perPktLen
            if (Radio_packetMdrCsTx((uint16_t)perNumPkts, &perPktLen))
            {
                // Set test mode back to 0 when returning
                *currentRadioTestMode = TestMode_EXIT;
                status = TestMode_Status_Success;
            }
            else
            {
                status = TestMode_Status_Fail;
            }

            pTestMsgFxn(TestMode_MDR_CS_TX, 0, 0, 0, true, 0, 0, 0, NULL);
        }

        //------------------------------------------------------------------------------------------
        // TEST_MODE_CS_TX
        //------------------------------------------------------------------------------------------
        else if((*currentRadioTestMode == TestMode_EXIT) && (newMode == TestMode_CS_TX))
        {
            // Start Transmitting perNumPkts packets of length perPktLen
            if (Radio_packetCsTx((uint16_t)perNumPkts, &perPktLen))
            {
                // Set test mode back to 0 when returning
                *currentRadioTestMode = TestMode_EXIT;
                status = TestMode_Status_Success;
            }
            else
            {
                status = TestMode_Status_Fail;
            }

            pTestMsgFxn(TestMode_CS_TX, 0, 0, 0, true, 0, 0, 0, NULL);
        }

        //------------------------------------------------------------------------------------------
        // EXIT_TEST_MODE
        //------------------------------------------------------------------------------------------
        else if(((*currentRadioTestMode == TestMode_CARRIER_WAVE) ||
                 (*currentRadioTestMode == TestMode_MODULATED)    ||
                 (*currentRadioTestMode == TestMode_PER_RX)       ||
                 (*currentRadioTestMode == TestMode_MDR_RX)       ||
                 (*currentRadioTestMode == TestMode_CONT_RX))     &&
                 (newMode == TestMode_EXIT))
        {
            // Function to abort the different modes
            TestMode_Exit(*currentRadioTestMode);

            *currentRadioTestMode = TestMode_EXIT;
            status = TestMode_Status_Success;
        }
        else
        {
            status = TestMode_Status_Fail;
        }
    }
    return status;
}

