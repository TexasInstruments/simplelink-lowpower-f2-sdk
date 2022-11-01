/*
 * Copyright (c) 2016-2017, Texas Instruments Incorporated
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
//******************************************************************************

#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(driverlib/rf_common_cmd.h)
#include DeviceFamily_constructPath(driverlib/rf_prop_cmd.h)
#include "application_settings.h"

// CMD_NOP
rfc_CMD_NOP_t RF_cmdNop =
{
    .commandNo                = CMD_NOP,
    .status                   = 0x0000,
    .pNextOp                  = 0, // Set this to (uint8_t*)&RF_cmdPropCs in the application
    .startTime                = 0x00000000,
    .startTrigger.triggerType = TRIG_ABSTIME, // Trigs at an absolute time
    .startTrigger.bEnaCmd     = 0x0,
    .startTrigger.triggerNo   = 0x0,
    .startTrigger.pastTrig    = 0x0,
    .condition.rule           = COND_ALWAYS, // Always run next command (except in case of Abort)
    .condition.nSkip          = 0x0,
};

// CMD_PROP_CS
rfc_CMD_PROP_CS_t RF_cmdPropCs =
{
    .commandNo                = CMD_PROP_CS,
    .status                   = 0x0000,
    .pNextOp                  = 0, // Set this to (uint8_t*)&RF_cmdCountBranch in the application
    .startTime                = 0x00000000,
    .startTrigger.triggerType = TRIG_NOW,
    .startTrigger.bEnaCmd     = 0x0,
    .startTrigger.triggerNo   = 0x0,
    .startTrigger.pastTrig    = 0x0,
    .condition.rule           = COND_SKIP_ON_FALSE, // Run next command if this command returned TRUE,
                                                    // skip a number of commands (.condition.nSkip - 1) if it returned FALSE
                                                    // End causes for the CMD_PROP_CS command:
                                                    // Observed channel state Busy with csConf.busyOp = 1:                            PROP_DONE_BUSY        TRUE
                                                    // 0bserved channel state Idle with csConf.idleOp = 1:                            PROP_DONE_IDLE        FALSE
                                                    // Timeout trigger observed with channel state Busy:                              PROP_DONE_BUSY        TRUE
                                                    // Timeout trigger observed with channel state Idle:                              PROP_DONE_IDLE        FALSE
                                                    // Timeout trigger observed with channel state Invalid and csConf.timeoutRes = 0: PROP_DONE_BUSYTIMEOUT TRUE
                                                    // Timeout trigger observed with channel state Invalid and csConf.timeoutRes = 1: PROP_DONE_IDLETIMEOUT FALSE
                                                    // Received CMD_STOP after command started:                                       PROP_DONE_STOPPED     FALSE
    .condition.nSkip          = 0x2, // Number of skips + 1 if the rule involves skipping. 0: Same, 1: Next, 2: Skip next
    .csFsConf.bFsOffIdle      = 0x0, // Keep synthesizer running if command ends with channel Idle
    .csFsConf.bFsOffBusy      = 0x0, // Keep synthesizer running if command ends with Busy
    .__dummy0                 = 0x00,
    .csConf.bEnaRssi          = 0x1, // Enable RSSI as a criterion
    .csConf.bEnaCorr          = 0x0, // Disable correlation (PQT) as a criterion
    .csConf.operation         = 0x0, // Busy if either RSSI or correlation indicates Busy
    .csConf.busyOp            = 0x1, // End carrier sense on channel Busy
    .csConf.idleOp            = 0x0, // Continue on channel Idle
    .csConf.timeoutRes        = 0x0, // Timeout with channel state Invalid treated as Busy
    .rssiThr                  = 0x0, // Set the RSSI threshold in the application
    .numRssiIdle              = 0x0, // Number of consecutive RSSI measurements - 1 below the threshold
                                     // needed before the channel is declared Idle
    .numRssiBusy              = 0x0, // Number of consecutive RSSI measurements -1 above the threshold
                                     // needed before the channel is declared Busy
    .corrPeriod               = 0x0000, // N/A since .csConf.bEnaCorr = 0
    .corrConfig.numCorrInv    = 0x0, // N/A since .csConf.bEnaCorr = 0
    .corrConfig.numCorrBusy   = 0x0, // N/A since .csConf.bEnaCorr = 0
    .csEndTrigger.triggerType = TRIG_REL_START, // Trigs at a time relative to the command started
    .csEndTrigger.bEnaCmd     = 0x0,
    .csEndTrigger.triggerNo   = 0x0,
    .csEndTrigger.pastTrig    = 0x0,
    .csEndTime                = 0x00000000, // Set the CS end time in the application
};

// CMD_COUNT_BRANCH
rfc_CMD_COUNT_BRANCH_t RF_cmdCountBranch =
{
    .commandNo                = CMD_COUNT_BRANCH,
    .status                   = 0x0000,
    .pNextOp                  = 0, // Set this to (uint8_t*)&RF_cmdPropTx in the application
    .startTime                = 0x00000000,
    .startTrigger.triggerType = TRIG_NOW, // Triggers immediately
    .startTrigger.bEnaCmd     = 0x0,
    .startTrigger.triggerNo   = 0x0,
    .startTrigger.pastTrig    = 0x0,
    .condition.rule           = COND_STOP_ON_FALSE, // Run next command if this command returned TRUE, stop if it returned FALSE
                                                    // End causes for the CMD_COUNT_BRANCH command:
                                                    // Finished operation with counter = 0 when being started: DONE_OK         TRUE
                                                    // Finished operation with counter > 0 after decrementing: DONE_OK         TRUE
                                                    // Finished operation with counter = 0 after decrementing: DONE_COUNTDOWN  FALSE
    .condition.nSkip          = 0x0,
    .counter                  = 0, // On start, the radio CPU decrements the value, and the end status of the operation differs if the result is zero
                                   // This number is set in the application (CS_RETRIES_WHEN_BUSY) and determines how many times the CMD_PROP_CS should run
                                   // in the case where the channel i Busy
    .pNextOpIfOk              = 0, // Set this to (uint8_t*)&RF_cmdPropCs in the application
};
