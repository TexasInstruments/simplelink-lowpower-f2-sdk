/******************************************************************************

 @file thermostat.h

 @brief Thermostat example application

 Group: CMCU, LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2017-2025, Texas Instruments Incorporated
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions
 are met:

 *  Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.

 *  Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.

 *  Neither the name of Texas Instruments Incorporated nor the names of
    its contributors may be used to endorse or promote products derived
    from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 ******************************************************************************
 
 
 *****************************************************************************/

#ifndef _THERMOSTAT_H_
#define _THERMOSTAT_H_

#ifdef __cplusplus
extern "C"
{
#endif

/******************************************************************************
 Constants and definitions
 *****************************************************************************/
/* Thermostat temp URI string */
#define THERMOSTAT_TEMP_URI    "thermostat/temperature"
/* Thermostat set point URI string */
#define THERMOSTAT_SET_PT_URI  "thermostat/setpoint"

/**
 * Thermostat events.
 */
typedef enum
{
    Thermostat_evtTempChange,         /* Temperature change event received */
    Thermostat_evtSetPtChange,        /* set point change event received */
    Thermostat_evtNwkSetup,           /* openthread network is setup */
    Thermostat_evtDevRoleChanged,     /* Events for Device State */
    Thermostat_evtGraphicExt,         /* graphics event received */
    Thermostat_evtKeyRight,           /* Right key is pressed */
    Thermostat_evtNwkJoined,          /* Joined the network */
    Thermostat_evtNwkJoinFailure,     /* Failed joining network */
#if TIOP_CUI
    Thermostat_evtProcessMenuUpdate,  /* CUI Menu Event is triggered */
    Thermostat_evtNwkAttach,          /* CUI Menu Attach option is selected */
    Thermostat_evtNwkJoin,            /* CUI Menu Join option is selected */
#endif /* TIOP_CUI */
#ifdef USE_DMM
    Thermostat_evtNwkParamUpdate,
#endif /* USE_DMM */
} Thermostat_evt;

/******************************************************************************
 External functions
 *****************************************************************************/

/**
 * @brief Posts an event to the Thermostat task.
 *
 * @param event event to post.
 * @return None
 */
extern void Thermostat_postEvt(Thermostat_evt event);

#ifdef __cplusplus
}
#endif

#endif /* _THERMOSTAT_H_ */
