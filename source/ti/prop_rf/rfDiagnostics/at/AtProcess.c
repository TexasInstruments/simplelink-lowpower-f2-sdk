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

/***************************************************************************************************
 * INCLUDES
 ***************************************************************************************************/
#include "at/platform/inc/AtTerm.h"
#include "at/AtProcess.h"
#include "at/AtControl.h"
#include "at/AtParams.h"
#include "at/platform/tirtos/DbgPrint.h"
#include "radio/radio.h"

#include <string.h>

/***************************************************************************************************
 * CONSTANTS
 ***************************************************************************************************/
#define ATPROCESS_STATE_A                0
#define ATPROCESS_STATE_T                1
#define ATPROCESS_STATE_CMD              2
#define ATPROCESS_BACKSPACE_CHAR         0x7f

static void processCmd(char* cmdBuffer, uint8_t cmdLen)
{
    AtProcess_Status status = AtProcess_Status_CmdIdError;

    switch(cmdBuffer[0])
    {
    case '+':

        // Process RF command
        status = AtControl_parseIncoming(&(cmdBuffer[1]), cmdLen-1);
        break;

    case '&':

        // Process peripheral command
        break;

    case 'P':
    case 'p':

        // Process Parameter command
        status = AtParams_parseIncoming(&(cmdBuffer[1]), cmdLen-1);
        break;
    }

    if(status == AtProcess_Status_Success)
    {
        AtProcess_printSuccess();
    }
    else
    {
        AtProcess_printErrorCode(status);
    }
    return;
}

/***************************************************************************************************
 * INTERFACE FUNCTIONS
 ***************************************************************************************************/
void AtProcess_processingLoop(void)
{
    char ch;
    static char cmdBuffer[ATPROCESS_MAX_AT_MSG_LEN];
    static uint8_t state = ATPROCESS_STATE_A;
    static uint8_t cmdLen = 0;

#ifndef AT_SPI
    if(AtTerm_getChar(&ch) > 0)
    {
#else
    char spiCmdBuffer[ATPROCESS_MAX_AT_MSG_LEN];
    uint8_t spiCmdLen = 0;
    uint8_t cmdIdx = 0;
    spiCmdLen = AtTerm_getChar(spiCmdBuffer);
    for(cmdIdx = 0; cmdIdx < spiCmdLen; cmdIdx++)
    {
        ch = spiCmdBuffer[cmdIdx];
#endif
        if(AtParams_echoEnabled)
        {
            AtTerm_putChar(ch);
        }

        switch(state)
        {
        case ATPROCESS_STATE_A:

            memset(cmdBuffer, 0, ATPROCESS_MAX_AT_MSG_LEN);
            cmdLen = 0;
            if((ch == 'a') || (ch == 'A'))
            {
                state = ATPROCESS_STATE_T;
            }
            break;

        case ATPROCESS_STATE_T:

            if((ch == 't') || (ch == 'T'))
            {
                state = ATPROCESS_STATE_CMD;
            }
            else if(ch == ATPROCESS_BACKSPACE_CHAR)
            {
                state = ATPROCESS_STATE_A;
            }
            break;

        case ATPROCESS_STATE_CMD:

            if((ch == '\n') || (ch == '\r'))
            {
                // Command entry complete, process it
                processCmd(cmdBuffer, cmdLen);
                state = ATPROCESS_STATE_A;
                cmdLen = 0;
            }
            else if(ch == ATPROCESS_BACKSPACE_CHAR)
            {
                if(cmdLen > 0)
                {
                    cmdLen--;
                }
                else
                {
                    state = ATPROCESS_STATE_T;
                }
            }
            else
            {
                if(cmdLen < ATPROCESS_MAX_AT_MSG_LEN)
                {
                    // Add this char to the command string
                    cmdBuffer[cmdLen++] = ch;
                }
                else
                {
                    // The command entered was too long we have overrun the command buffer
                    AtProcess_printErrorCode(AtProcess_Status_CmdIdError);
                    state = ATPROCESS_STATE_A;
                    cmdLen = 0;
                }
            }
            break;

        default:
            state = ATPROCESS_STATE_A;
            cmdLen = 0;
            break;
        }
    }
}

void AtProcess_printSuccess(void)
{
    AtTerm_sendString("OK\r\r");
}

void AtProcess_printErrorCode(AtProcess_Status errorCode)
{
    if(errorCode == AtProcess_Status_CmdIdError)
    {
        AtTerm_sendString("AtProcess_Status_CmdIdError\r\r");
    }

    if(errorCode == AtProcess_Status_ParamError)
    {
        AtTerm_sendString("AtProcess_Status_ParamError\r\r");
    }

    if(errorCode == AtProcess_Status_Error)
    {
        AtTerm_sendString("AtProcess_Status_Error\r\r");
    }
}

uint8_t AtProcess_cmdCmpAndSetRadioId(char* cmdStr, char* inComingCmdStr)
{
    uint8_t index = 0, match = 0, radioId = RADIO_ID_DEFAULT;

    char *p = cmdStr;
    char *s = inComingCmdStr;

    // Find how many chars are required for a match
    // e.g FR? -> 3, I -> 1
    // Either find an exact match or a modified match, i.e. one with a radio id
    // e.g FR? <=> FR?
    //     FR? <=> FR1?
    //     FR? <=> FR2?
    //     FR? <X> FR0? --> 0 is an invalid radio id
    // Similarly,
    //      I  <=> I 0  --> matches the control command 'I'
    //      I  <=> I1 0 --> matches the control command 'I'
    //      I  <=> I2 0 --> matches the control command 'I'
    //      I  <=> I0 0 --> matches the control command 'I' even though an invlaid radio id
    //                      is entered; this is caught (and rejected) later on in the chain.
//    uint8_t reqMatch = 0;
//    while(p[reqMatch]){++reqMatch;} --> COMPILER BUG: does an additional inc
    uint8_t reqMatch = strlen(p);

    for(index = 0, match = 0; index < ATPROCESS_MAX_AT_CMD_LEN; index++)
    {
        // found all the characters
        if(match == reqMatch)
            break;

        if(s[index] == p[index]){
            // characters match
            match++;
        }else{
            // characters dont match
            if ((p[index] == '=') || (p[index] == '?')){
                // tried to match to '='|'?', maybe this is the
                // radio ID and the next char will match?
                if(Radio_verifyRadioId((uint8_t)(s[index]-'0')) && (s[index+1] == p[index]))
                {
                    match++;
                    radioId = (uint8_t)(s[index]-'0');
                    index++;
                }
            }else{
                break;
            }
        }
    }

    if (match != reqMatch)
        index = 0;
    else
        Radio_setCurrentRadio(radioId);

    return index;
}

/***************************************************************************************************
 ***************************************************************************************************/
