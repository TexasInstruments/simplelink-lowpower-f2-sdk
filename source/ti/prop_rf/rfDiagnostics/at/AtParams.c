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
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>
#include <string.h>
#include <TestMode.h>

#include "at/AtParams.h"
#include "at/AtProcess.h"
#include "at/platform/tirtos/DbgPrint.h"
#include "at/platform/inc/AtTerm.h"
#include "radio/radio.h"
#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(inc/hw_ccfg.h)
#include DeviceFamily_constructPath(inc/hw_ccfg_simple_struct.h)

uint8_t AtParams_echoEnabled = 1;

static uint32_t perNumPkts = NUMBER_OF_PACKETS;
static uint32_t perPktLen = PACKET_LENGTH;
static uint16_t packetTxCount = 0;
RF_Frequency frequency = {
    .freq = 0,
    .mdrFreq = 0
};

/***************************************************************************************************
 * LOCAL FUNCTIONS
 ***************************************************************************************************/
static AtProcess_Status atParamReadPerNumPackets(char* paramStr, uint32_t paramLen);
static AtProcess_Status atParamWritePerNumPackets(char* paramStr, uint32_t paramLen);
static AtProcess_Status atParamReadPerPacketLen(char* paramStr, uint32_t paramLen);
static AtProcess_Status atParamWritePerPacketLen(char* paramStr, uint32_t paramLen);
static AtProcess_Status atParamReadTestMode(char* paramStr, uint32_t paramLen);
static AtProcess_Status atParamWriteTestMode(char* paramStr, uint32_t paramLen);
static AtProcess_Status atParamWriteAtEcho(char* paramStr, uint32_t paramLen);
static AtProcess_Status atParamReadAtEcho(char* paramStr, uint32_t paramLen);
static AtProcess_Status atParamReadFreq(char* paramStr, uint32_t paramLen);
static AtProcess_Status atParamWriteFreq(char* paramStr, uint32_t paramLen);
static AtProcess_Status atParamReadPower(char* paramStr, uint32_t paramLen);
static AtProcess_Status atParamWritePower(char* paramStr, uint32_t paramLen);
static AtProcess_Status atParamReadCurrRssi(char* paramStr, uint32_t paramLen);
static AtProcess_Status atParamReadAvgRssi(char* paramStr, uint32_t paramLen);
static AtProcess_Status atParamReadMinRssi(char* paramStr, uint32_t paramLen);
static AtProcess_Status atParamReadMaxRssi(char* paramStr, uint32_t paramLen);
static AtProcess_Status atParamReadPHyNames(char* paramStr, uint32_t paramLen);
static AtProcess_Status atParamReadRadioVersion(char* paramStr, uint32_t paramLen);

AtCommand_t atParamsters[] =
{
    {"PP?", atParamReadPerNumPackets}, // PP: Number of packets
    {"pp?", atParamReadPerNumPackets},
    {"PP=", atParamWritePerNumPackets},
    {"pp=", atParamWritePerNumPackets},
    {"PL?", atParamReadPerPacketLen}, // PL: Payload length
    {"pl?", atParamReadPerPacketLen},
    {"PL=", atParamWritePerPacketLen},
    {"pl=", atParamWritePerPacketLen},
    {"TM?", atParamReadTestMode}, // TM: Test mode
    {"tm?", atParamReadTestMode},
    {"TM=", atParamWriteTestMode},
    {"tm=", atParamWriteTestMode},
    {"FR?", atParamReadFreq}, // FR: Frequency
    {"fr?", atParamReadFreq},
    {"FR=", atParamWriteFreq},
    {"fr=", atParamWriteFreq},
    {"PW?", atParamReadPower}, // PW: Output power
    {"pw?", atParamReadPower},
    {"PW=", atParamWritePower},
    {"pw=", atParamWritePower},
    {"CR?", atParamReadCurrRssi}, // CR: Current RSSI
    {"cr?", atParamReadCurrRssi},
    {"AR?", atParamReadAvgRssi}, // AR: Average RSSI
    {"ar?", atParamReadAvgRssi},
    {"LR?", atParamReadMinRssi}, //LR: Lowest/Minimum RSSI
    {"lr?", atParamReadMinRssi},
    {"MR?", atParamReadMaxRssi}, // MR: Maximum RSSI
    {"mr?", atParamReadMaxRssi},
    {"PN?", atParamReadPHyNames}, // PN: Phy Name
    {"pn?", atParamReadPHyNames},
    {"AE?", atParamReadAtEcho}, // AE: Echo
    {"ae?", atParamReadAtEcho},
    {"AE=", atParamWriteAtEcho},
    {"ae=", atParamWriteAtEcho},
    {"RV?", atParamReadRadioVersion}, // RV: Radio Version
    {"rv?", atParamReadRadioVersion}
};

static AtProcess_Status atParamReadPHyNames(char* paramStr, uint32_t paramLen)
{
    (void)paramLen;

    uint8_t radioId;
    AtTerm_getIdAndParam(paramStr, &radioId, (uintptr_t)NULL, (uintptr_t)NULL, 0);

    uint8_t i;

    for (i = 0; i < Radio_getNumSupportedPhys(); i++)
    {
        AtTerm_sendString(Radio_getPhyName(i));
        AtTerm_sendString("\r\n");
    }
    return AtProcess_Status_Success;
}


static AtProcess_Status atParamReadPerNumPackets(char* paramStr, uint32_t paramLen)
{
    (void)paramStr;
    (void)paramLen;

    AtTerm_sendStringUi32Value("", perNumPkts, 10);
    AtTerm_sendString("\r");
    return AtProcess_Status_Success;
}

static AtProcess_Status atParamWritePerNumPackets(char* paramStr, uint32_t paramLen)
{
    (void)paramLen;

    AtProcess_Status status = AtProcess_Status_ParamError;
    uint32_t temp = perNumPkts;

    if(sscanf(paramStr, "%d", (int *)&perNumPkts) == 1)
    {
        if(perNumPkts <= MAX_NUMBER_OF_PACKETS)
        {
            status = AtProcess_Status_Success;
        }
        else
        {
            perNumPkts = temp;
        }
    }
    return status;
}

static AtProcess_Status atParamReadPerPacketLen(char* paramStr, uint32_t paramLen)
{
    (void)paramStr;
    (void)paramLen;

    AtTerm_sendStringUi32Value("", perPktLen, 10);
    AtTerm_sendString("\r");
    return AtProcess_Status_Success;
}

static AtProcess_Status atParamWritePerPacketLen(char* paramStr, uint32_t paramLen)
{
    (void)paramLen;

    AtProcess_Status status = AtProcess_Status_ParamError;

    uint32_t temp = perPktLen;

    if(sscanf(paramStr, "%d", (int *)&perPktLen) == 1)
    {
        // Packet length must be between 2 (to make room for seq. #) and 255.
        // If PhySettings_apiType = PhySettings_API_BLE when PhySettings_testType = PhySettings_TEST_STUDIO_COMPL,
        // the length will be set to MAX_BLE_ADV_PAYLOAD_LENGTH (37) if the new length is higher than 37 and set to
        // MIN_BLE_ADV_PAYLOAD_LENGTH (12) if it is lower than 12.
        if(Radio_checkPacketLength(&perPktLen))
        {
            status = AtProcess_Status_Success;
        }
        else
        {
            perPktLen = temp;
        }
    }
    return status;
}

static AtProcess_Status atParamReadTestMode(char* paramStr, uint32_t paramLen)
{
    (void)paramStr;
    (void)paramLen;

    uint8_t testMode = TestMode_read();

    AtTerm_sendStringUi8Value("      ", testMode, 10);
    AtTerm_sendString("\r");

    return AtProcess_Status_Success;
}

static AtProcess_Status atParamWriteTestMode(char* paramStr, uint32_t paramLen)
{
    (void)paramLen;

    TestMode_Status testStatus = TestMode_Status_Fail;

    uint32_t mode = atoi(paramStr);

    testStatus = TestMode_write(mode, perNumPkts, perPktLen, AtParams_printTestMsg);

    if(testStatus == TestMode_Status_Success)
    {
        return AtProcess_Status_Success;
    }
    else
    {
        return AtProcess_Status_Error;
    }
}

static AtProcess_Status atParamReadFreq(char* paramStr, uint32_t paramLen)
{
    (void)paramStr;
    (void)paramLen;

    Radio_getFreq(&frequency);

    if(frequency.freq != (uint32_t)RADIO_ERROR_VALUE)
    {
        AtTerm_sendStringUi32Value("    ", frequency.freq, 10);
        AtTerm_sendString(" Hz\r");
        if(frequency.mdrFreq != 0)
        {
            AtTerm_sendStringUi32Value("    ", frequency.mdrFreq, 10);
            AtTerm_sendString(" Hz\r");
        }
        return AtProcess_Status_Success;
    }
    else
    {
        return AtProcess_Status_Error;
    }
}

static AtProcess_Status atParamWriteFreq(char* paramStr, uint32_t paramLen)
{
    (void)paramLen;

    AtProcess_Status status = AtProcess_Status_ParamError;
    uint32_t newFreq;
    uint32_t newMdrFreq;
    uint8_t testMode = TestMode_read();

    // Do not allow for changing frequency when a test is running
    if (testMode == TestMode_EXIT)
    {
        char *token;
        char delimiter[] = " ";
        token = strtok(paramStr, delimiter);
        if(NULL != token)
        {
            status = AtProcess_Status_Success;
            newFreq = atoi(token);
            token = strtok(NULL, delimiter);
            if(NULL != token)
            {
                newMdrFreq = atoi(token);
            }
            else
            {
                newMdrFreq = NO_MDR_FREQ;
            }
            Radio_setFreq(newFreq,newMdrFreq);
        }
        else
        {
            status = AtProcess_Status_Error;
        }
    }
    else
    {
        status = AtProcess_Status_Error;
    }
    return status;
}


static AtProcess_Status atParamReadPower(char* paramStr, uint32_t paramLen)
{
    (void)paramStr;
    (void)paramLen;

    int8_t power = Radio_getPower();

    if(power != RADIO_ERROR_VALUE)
    {
        AtTerm_sendStringI8Value("      ", power, 10);
        AtTerm_sendString(" dBm\r");
        return AtProcess_Status_Success;
    }
    else
    {
        return AtProcess_Status_Error;
    }
}

static AtProcess_Status atParamWritePower(char* paramStr, uint32_t paramLen)
{
    (void)paramLen;

    AtProcess_Status status = AtProcess_Status_ParamError;
    int32_t newPower;
    uint8_t testMode = TestMode_read();

    // Do not allow for changing output power when a test is running
    if (testMode == TestMode_EXIT)
    {
        if(sscanf(paramStr, "%d", (int *)&newPower) == 1)
        {
            if (Radio_setPower(newPower))
            {
                status = AtProcess_Status_Success;
            }
            else
            {
                status = AtProcess_Status_Error;
            }
        }
    }
    else
    {
        status = AtProcess_Status_Error;
    }
    return status;
}

static AtProcess_Status atParamReadCurrRssi(char* paramStr, uint32_t paramLen)
{
    (void)paramStr;
    (void)paramLen;

    AtProcess_Status status = AtProcess_Status_Error;

    int8_t rssiDbm = Radio_getCurrentRssi();

    if(rssiDbm != RADIO_ERROR_VALUE)
    {
        AtTerm_sendStringI8Value("      ", rssiDbm, 10);
        AtTerm_sendString(" dBm\r");
        status = AtProcess_Status_Success;
    }

    return status;

}

static AtProcess_Status atParamReadAvgRssi(char* paramStr, uint32_t paramLen)
{
    (void)paramStr;
    (void)paramLen;

    AtProcess_Status status = AtProcess_Status_Error;
    uint8_t testMode = TestMode_read();

    if(testMode == TestMode_PER_RX)
    {
        int32_t avgRssi = Radio_getAvgRssi();

        AtTerm_sendStringI32Value("  ", avgRssi, 10);
        AtTerm_sendString(" dBm\r");
        status = AtProcess_Status_Success;
    }

    return status;
}

static AtProcess_Status atParamReadMinRssi(char* paramStr, uint32_t paramLen)
{
    (void)paramStr;
    (void)paramLen;

    AtProcess_Status status = AtProcess_Status_Error;
    uint8_t testMode = TestMode_read();

    if(testMode == TestMode_PER_RX)
    {
        int8_t minRssi = Radio_getMinRssi();
        AtTerm_sendStringI8Value("      ", minRssi, 10);
        AtTerm_sendString(" dBm\r");
        status = AtProcess_Status_Success;
    }

    return status;
}

static AtProcess_Status atParamReadMaxRssi(char* paramStr, uint32_t paramLen)
{
    (void)paramStr;
    (void)paramLen;

    AtProcess_Status status = AtProcess_Status_Error;
    uint8_t testMode = TestMode_read();

    if(testMode == TestMode_PER_RX)
    {
        int8_t maxRssi = Radio_getMaxRssi();
        AtTerm_sendStringI8Value("      ", maxRssi, 10);
        AtTerm_sendString(" dBm\r");
        status = AtProcess_Status_Success;
    }

    return status;
}

static AtProcess_Status atParamWriteAtEcho(char* paramStr, uint32_t paramLen)
{
    (void)paramLen;

    AtProcess_Status status = AtProcess_Status_ParamError;

    AtParams_echoEnabled = atoi(paramStr);
    status = AtProcess_Status_Success;

    return status;
}

static AtProcess_Status atParamReadAtEcho(char* paramStr, uint32_t paramLen)
{
    (void)paramStr;
    (void)paramLen;

    AtProcess_Status status = AtProcess_Status_ParamError;

    AtTerm_sendStringUi8Value("      ",AtParams_echoEnabled, 10);
    AtTerm_sendString("\r");
    status = AtProcess_Status_Success;

    return status;
}

static AtProcess_Status atParamReadRadioVersion(char* paramStr, uint32_t paramLen)
{
    (void)paramStr;
    (void)paramLen;
    char radioVersion[RADIO_VERSION_LENGTH] = {0};

    AtProcess_Status status = AtProcess_Status_ParamError;

    strcpy(radioVersion, Radio_getRadioVersion());

    if(strcmp(radioVersion, RADIO_UNSUPPORTED_CMD))
    {
        AtTerm_sendString(radioVersion);
        AtTerm_sendString("\r");
        status = AtProcess_Status_Success;
    }

    return status;
}

/***************************************************************************************************
 * INTERFACE FUNCTIONS
 ***************************************************************************************************/

AtProcess_Status AtParams_parseIncoming(char *param, uint8_t paramLen)
{
    uint8_t cmdIdx, paramOffSet;
    AtProcess_Status status = AtProcess_Status_CmdIdError;

    for(cmdIdx = 0; cmdIdx < (sizeof(atParamsters) / sizeof(AtCommand_t)); cmdIdx++)
    {
        if((paramOffSet = AtProcess_cmdCmpAndSetRadioId(atParamsters[cmdIdx].cmdStr, param)) && (atParamsters[cmdIdx].cmdFxn != NULL))
        {
            status = atParamsters[cmdIdx].cmdFxn(&(param[paramOffSet]), paramLen - 1);
            break;
        }
    }

    return status;
}

void AtParams_printTestMsg(uint32_t mode, uint16_t packetCount, uint16_t packetCountNok, uint16_t rxSyncCount, bool txDone, int32_t avgRssi, int32_t minRssi, int32_t maxRssi, char *phyName)
{
    if(mode == TestMode_PER_TX)
    {
        if(txDone)
        {
            AtTerm_sendStringUi16Value("Packets Transmitted: ", packetTxCount, 10);
            AtTerm_sendString("\r");
            packetTxCount = 0;
        }
        else
        {
            packetTxCount = packetCount;

            if(packetTxCount == 1)
            {
                AtTerm_sendString("Sending packets....\r");
            }
        }
    }
    else if(mode == TestMode_MDR_TX)
    {
        if(txDone)
        {
            AtTerm_sendStringUi16Value("MDR Packets Transmitted: ", packetTxCount, 10);
            AtTerm_sendString("\r");
            packetTxCount = 0;
        }
        else
        {
            packetTxCount = packetCount;

            if(packetTxCount == 1)
            {
                AtTerm_sendString("Sending packets....\r");
            }
        }
    }
    else if(mode == TestMode_MDR_CS_TX)
    {
        if(txDone)
        {
            AtTerm_sendStringUi16Value("MDR Packets Transmitted: ", packetTxCount, 10);
            AtTerm_sendString("\r");
            packetTxCount = 0;
        }
        else
        {
            packetTxCount = packetCount;

            if(packetTxCount == 1)
            {
                AtTerm_sendString("Sending packets....\r");
            }
        }
    }
    else if(mode == TestMode_CS_TX)
    {
        if(txDone)
        {
            AtTerm_sendStringUi16Value("CS+TX Packets Transmitted: ", packetTxCount, 10);
            AtTerm_sendString("\r");
            packetTxCount = 0;
        }
        else
        {
            packetTxCount = packetCount;

            if(packetTxCount == 1)
            {
                AtTerm_sendString("Sending packets....\r");
            }
        }
    }
    else if(mode == TestMode_PER_RX)
    {
        AtTerm_sendString("RX in Progress....\r\n");
        AtTerm_sendString("Exit by setting Test Mode = 0 (ATPTM=0)\r\n");
        AtTerm_sendString("Note: Packet Length filtering is utilized, so the packet length\r\n");
        AtTerm_sendString("on the receiver must be larger or equal to the packet length on the transmitter\r\n");
    }
    else if(mode == TestMode_MDR_RX)
    {
        AtTerm_sendString("MDR RX in Progress....\r\n");
        AtTerm_sendString("Exit by setting Test Mode = 0 (ATPTM=0)\r\n");
        AtTerm_sendString("Note: Packet Length filtering is utilized, so the packet length\r\n");
        AtTerm_sendString("on the receiver must be larger or equal to the packet length on the transmitter\r\n");
    }
    else
    {
        uint32_t totalPackets = (uint32_t)(packetCount + packetCountNok);

        /* Avoid a 0.0/0.0 (NaN) or a x/0.0 (+Inf) condition */
        uint32_t per = 0;
        if(totalPackets > 0)
        {
            per = ((uint32_t)packetCountNok * 100)/(totalPackets);
        }

        AtTerm_sendStringUi16Value("Packets Received: ", (packetCount + packetCountNok), 10);
        AtTerm_sendString("\r\n");
        AtTerm_sendStringUi16Value("CRC Ok:           ", packetCount, 10);
        AtTerm_sendString("\r\n");
        AtTerm_sendStringUi16Value("Sync Ok:          ", rxSyncCount, 10);
        AtTerm_sendString("\r\n");
        AtTerm_sendStringUi16Value("PER:              ", (int16_t)per, 10);
        AtTerm_sendString("%\r\n");
        AtTerm_sendStringI32Value("Average RSSI: ", avgRssi, 10);
        AtTerm_sendString(" dBm\r\n");
        AtTerm_sendStringI32Value("Max RSSI:     ", maxRssi, 10);
        AtTerm_sendString(" dBm\r\n");
        AtTerm_sendStringI32Value("Min RSSI:     ", minRssi, 10);
        AtTerm_sendString(" dBm\r\n");
        AtTerm_sendString("PHY RX'd: ");
        AtTerm_sendString(phyName);
        AtTerm_sendString("\r\n");
    }
}

/***************************************************************************************************
 ***************************************************************************************************/
