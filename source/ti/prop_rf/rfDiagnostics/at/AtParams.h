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

#ifndef atParams__include
#define atParams__include

#ifdef __cplusplus
extern "C"
{
#endif

/***************************************************************************************************
 * INCLUDES
 ***************************************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include "at/AtProcess.h"

/***************************************************************************************************
 * DEFINES
 ***************************************************************************************************/

#define MAX_NUMBER_OF_PACKETS   65535

#define NO_MDR_FREQ 0xFFFFFFFF

#define NUMBER_OF_PACKETS       100
#define PACKET_LENGTH           12

/***************************************************************************************************
 * MACROS
 ***************************************************************************************************/

//! \brief At Parameter containing the state of the UART echo used to echo incoming
//! characters
extern uint8_t AtParams_echoEnabled;


/***************************************************************************************************
 *
 * Parses incoming AT Parameter commands ("ATP")
 *
 * This function parses incoming AT Parameter commands (setting/getting).
 * These commands are of the format "ATPxx=yy"/ "ATPxx?" respectively
 *
 * param *param - The string containing the command to be parsed
 * param len -    The length of the command
 *
 *return AT status code
 *
 ***************************************************************************************************/
AtProcess_Status AtParams_parseIncoming( char *param, uint8_t len );


/***************************************************************************************************
 *
 * Print messages using AtTerm
 *
 * This function prints messages via UART during or after test modes
 *
 * param mode        - The test mode that needs a message printed
 * param packetCount - Number of packets
 * param txDone      - Flag for if tx test mode is done
 * param avgRssi     - Average RSSI
 * param minRssi     - Minimum RSSI
 * param maxRSSI     - Maximum RSSI
 * param phyName     - Phy Packets were received on, NULL if N/A
 *
 *return none
 *
 ***************************************************************************************************/
void AtParams_printTestMsg(uint32_t mode, uint16_t packetCount, uint16_t packetCountNok, uint16_t rxSyncCount, bool txDone, int32_t avgRssi, int32_t minRssi, int32_t maxRssi, char *phyName);

/***************************************************************************************************
 ***************************************************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* atParams__include */
