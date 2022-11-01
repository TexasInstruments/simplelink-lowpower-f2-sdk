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

#ifndef atProcess__include
#define atProcess__include

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>

#define ATPROCESS_MAX_AT_MSG_LEN     128
#define ATPROCESS_MAX_AT_CMD_LEN       5

// AtProcess and error codes
typedef enum
{
    AtProcess_Status_Success        = 0,
    AtProcess_Status_CmdIdError     = 1,
    AtProcess_Status_ParamError     = 2,
    AtProcess_Status_Error          = 3,
} AtProcess_Status;

// AT command processing function type
typedef AtProcess_Status (*AtCmdFnx)(char* paramStr, uint32_t paramLen);

// AT command processing type, matching command string to processing function
typedef struct
{
    char cmdStr[ATPROCESS_MAX_AT_CMD_LEN];
    AtCmdFnx cmdFxn;
} AtCommand_t;

/***************************************************************************************************
 *
 * Processing loop for reading and parsing UART characters
 *
 * This function parses in coming UART character to identify the "AT" start
 * of frame and the "/r"/CR end of frame. The command frame is then parsed to
 * identify the command type before passing to the appropriate sub module
 *
 * param none
 *
 * return none
 *
****************************************************************************************************/
extern void AtProcess_processingLoop(void);

/***************************************************************************************************
 *
 * Print error code to AT terminal
 *
 * This function identifies the atProcess_Status error code and prints the
 * ASCI to the AT terminal
 *
 * param errorCode the atProcess_Status error code
 *
 * return none
 *
****************************************************************************************************/
extern void AtProcess_printErrorCode(AtProcess_Status errorCode);

/***************************************************************************************************
 *
 * Print success to AT terminal
 *
 * This function Prints success to AT terminal
 *
 * param none
 *
 * return none
 *
*****************************************************************************************************/
extern void AtProcess_printSuccess(void);

/***************************************************************************************************
 *
 * Compare AT commands and (optionally) extract and set the current radio id
 *
 * This function compares 2 AT commands and returns 0 for no match or index to
 * parameter if they do match. If the command has a radio identifier, it will extract and set
 * the current radio to it.
 *
 * param cmd1 - The string containing first command
 * param cmd2 - The string containing second command
 *
 * return index to parameters if commands match, 0 if not
 *
*****************************************************************************************************/
extern uint8_t AtProcess_cmdCmpAndSetRadioId(char* cmd1, char* cmd2);

/***************************************************************************************************
 *
 * Compare AT parameter set/get commands
 *
 * This function compares 2 AT parameter set/get commands and returns 0 for no
 * match or index to opertator (=/?) if they do match
 *
 * param paramCmd1 - The string containing first param command
 * param paramCmd2 - The string containing second command
 *
 * return index to parameters if commands match, 0 if not
 *
*****************************************************************************************************/
extern uint8_t AtProcess_paramCmp(char* paramCmd1, char* paramCmd2);

#ifdef __cplusplus
}
#endif

#endif /* atProcess__include */
