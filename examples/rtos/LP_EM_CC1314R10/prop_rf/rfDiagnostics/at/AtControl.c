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
#include <stdlib.h>
#include <string.h>

#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Task.h>
#include <xdc/runtime/Error.h>

#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(inc/hw_ccfg.h)
#include DeviceFamily_constructPath(inc/hw_ccfg_simple_struct.h)
#include DeviceFamily_constructPath(driverlib/sys_ctrl.h)

#include "at/AtProcess.h"
#include "at/AtControl.h"
#include "at/platform/inc/AtTerm.h"
#include "at/platform/tirtos/DbgPrint.h"
#include "at/AtParams.h"
#include "radio/radio.h"
#include <TestMode.h>

static AtProcess_Status atCtrlrFInit(char *paramStr, uint32_t paramLen);
static AtProcess_Status atCtrlReset(char *paramStr, uint32_t paramLen);

static AtCommand_t atControlCmds[] =
    {
        {"I", atCtrlrFInit}, // I: Initialize PHY
        {"i", atCtrlrFInit},
        {"RS", atCtrlReset}, // RS: reset device
        {"rs", atCtrlReset}
};

static AtProcess_Status atCtrlReset(char *paramStr, uint32_t paramLen)
{
    (void)paramStr;
    (void)paramLen;

    SysCtrlSystemReset();
    return AtProcess_Status_Success;
}

static AtProcess_Status atCtrlrFInit(char *paramStr, uint32_t paramLen)
{
    (void)paramLen;
    uint8_t phyIndex = 0;
    uint8_t radioId;
    uint8_t testMode = TestMode_read();

    AtProcess_Status status = AtProcess_Status_Error;

    // Do not allow for changing PHY when a test is running
    if (testMode == TestMode_EXIT)
    {
        AtTerm_getIdAndParam(paramStr, &radioId, (uintptr_t)&phyIndex, sizeof(phyIndex));
        if(Radio_verifyRadioId(radioId))
        {
            // verify and set the current radio
            Radio_setCurrentRadio(radioId);

            // Check that this PHY is available
            if (phyIndex < Radio_getNumSupportedPhys())
            {
                // Call a function that will setup the right PHY
                Radio_setupPhy(phyIndex);

                AtTerm_sendString("Init State \v");

                status = AtProcess_Status_Success;
            }
        }
    }

    return status;
}

/***************************************************************************************************
 * INTERFACE FUNCTIONS
 ***************************************************************************************************/

AtProcess_Status AtControl_parseIncoming(char *command, uint8_t cmdLen)
{
    uint8_t cmdIdx, paramOffSet;
    AtProcess_Status status = AtProcess_Status_CmdIdError;

    for (cmdIdx = 0; cmdIdx < (sizeof(atControlCmds) / sizeof(AtCommand_t)); cmdIdx++)
    {
        if ((paramOffSet = AtProcess_cmdCmpAndSetRadioId(atControlCmds[cmdIdx].cmdStr, command)) && (atControlCmds[cmdIdx].cmdFxn != NULL))
        {
            status = atControlCmds[cmdIdx].cmdFxn(&(command[paramOffSet]), cmdLen - paramOffSet);
            break;
        }
    }
    return status;
}

/***************************************************************************************************
 ***************************************************************************************************/
