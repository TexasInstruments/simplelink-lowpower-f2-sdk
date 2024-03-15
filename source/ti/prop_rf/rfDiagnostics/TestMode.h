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
#ifndef TestMode__include
#define TestMode__include

/***************************************************************************************************
 * DEFINES
 ***************************************************************************************************/
#define TestMode_EXIT               0
#define TestMode_CARRIER_WAVE       1
#define TestMode_MODULATED          2
#define TestMode_CONT_RX            3
#define TestMode_PER_TX             4
#define TestMode_PER_RX             5
#define TestMode_MDR_TX             6
#define TestMode_MDR_RX             7
#define TestMode_MDR_CS_TX          8
#define TestMode_CS_TX              9
#define TestMode_NUMBER_OF_MODES    10

#define TestMode_SELECT_CW                 1
#define TestMode_SELECT_MODULATED          0

// Test status
typedef enum
{
    TestMode_Status_Fail                = 0,
    TestMode_Status_Success             = 1,
} TestMode_Status;

// Function type to print messages
typedef void (*printMsgFxn)(uint32_t mode, uint16_t packetCount, uint16_t packetCountNok, uint16_t rxSyncCount, bool txDone, int32_t avgRssi, int32_t minRssi, int32_t maxRssi, char *phyName);

/***************************************************************************************************
 *
 * Runs/exits a test mode
 *
 * This function runs or exits a specified test mode
 *
 * param newMode     - The test mode to set and run
 * param perNumPkts  - Number of PER packets for PER TX or PER RX test modes
 * param perPktLen   - PER packet length for PER TX or PER RX test modes
 * param pMsgFxn     - Pointer to print function
 *
 *return Status of setting test mode
 *
 ***************************************************************************************************/
TestMode_Status TestMode_write(uint32_t newMode, uint32_t perNumPkts, uint32_t perPktLen, printMsgFxn pMsgFxn);

/***************************************************************************************************
 *
 * Prints the number of transmitted packets
 *
 * param numPackets - Number of transmitted packets
 *
 * return none
 *
 ***************************************************************************************************/
void TestMode_perTxPrintMsg(uint16_t numPackets);

/***************************************************************************************************
 *
 * Get the active test mode
 *
 * param none
 *
 * return active test mode
 *
 ***************************************************************************************************/
uint8_t TestMode_read(void);

#endif /* TestMode__include */
