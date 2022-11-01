/*
 * Copyright (c) 2019, Texas Instruments Incorporated
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

/***** Includes *****/

/* Application Header files */
#include "StateMachine.h"
#include "rfSynchronizedPacket.h"
#include <ti_radio_config.h>

/* Standard C Libraries */
#include <stdlib.h>
#include <stdint.h>

/*Board Header Files*/
 #include "ti_drivers_config.h"

/* TI Drivers */
#include <ti/drivers/rf/RF.h>
#include <ti/drivers/GPIO.h>

#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(driverlib/rf_common_cmd.h)
#include DeviceFamily_constructPath(driverlib/rf_mailbox.h)
#include DeviceFamily_constructPath(driverlib/rf_prop_cmd.h)
#include DeviceFamily_constructPath(driverlib/rf_prop_mailbox.h)
#include DeviceFamily_constructPath(driverlib/cpu.h)
#include DeviceFamily_constructPath(driverlib/ioc.h)

/* Define events that can be posted to the application state machine */
typedef enum {
    Event_TxModeButtonPushed = StateMachine_Event00,
    Event_LedButtonPushed = StateMachine_Event01,
} Event;

/* Declare state handler functions for the application. */
StateMachine_DECLARE_STATE(SetupState);
StateMachine_DECLARE_STATE(PeriodicBeaconState);
StateMachine_DECLARE_STATE(SpontaneousBeaconState);


/***** Defines *****/
#define BEACON_INTERVAL_MS  500
#define MAIN_TASK_STACK_SIZE 1024
#define MAIN_TASK_PRIORITY   1        /* Lowest priority by default */

/***** Prototypes *****/
void buttonCallbackFunction(uint_least8_t index);

void PeriodicBeaconState_txCallback(RF_Handle h, RF_CmdHandle ch, RF_EventMask e);

/***** Variable declarations *****/

static RF_Object rfObject;
static RF_Handle rfHandle;

static BeaconPacket message;
static StateMachine_Struct stateMachine;

/***** Function definitions *****/

void *mainThread(void *arg0)
{
    GPIO_setConfig(CONFIG_GPIO_RLED, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);
    GPIO_setConfig(CONFIG_GPIO_GLED, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);
    GPIO_setConfig(CONFIG_GPIO_BTN1, GPIO_CFG_IN_PU | GPIO_CFG_IN_INT_FALLING);
    GPIO_setConfig(CONFIG_GPIO_BTN2, GPIO_CFG_IN_PU | GPIO_CFG_IN_INT_FALLING);

    /* Install Button callback */
    GPIO_setCallback(CONFIG_GPIO_BTN1, buttonCallbackFunction);
    GPIO_setCallback(CONFIG_GPIO_BTN2, buttonCallbackFunction);

    /* Enable interrupts */
    GPIO_enableInt(CONFIG_GPIO_BTN1);
    GPIO_enableInt(CONFIG_GPIO_BTN2);

    // Initialise the application state machine.
    StateMachine_construct(&stateMachine);

    /* Execute the state machine StateMachine_exec() function */
    StateMachine_exec(&stateMachine, SetupState);

    return (0);
}


/* GPIO interrupt Callback function for board buttons. */
void buttonCallbackFunction(uint_least8_t index) {

    /* Debounce the button with a short delay */
    CPUdelay(CPU_convertMsToDelayCycles(5));
    if (GPIO_read(index) == 1)
    {
        return;
    }

    switch (index)
    {
    case CONFIG_GPIO_BTN1:
        StateMachine_postEvents(&stateMachine, Event_LedButtonPushed);
        GPIO_toggle(CONFIG_GPIO_GLED);
        break;
    case CONFIG_GPIO_BTN2:
        StateMachine_postEvents(&stateMachine, Event_TxModeButtonPushed);
        break;
    }
}



void SetupState_function()
{
    /* Prepare the packet */
    RF_cmdPropTx.pktLen = sizeof(message);
    RF_cmdPropTx.pPkt = (uint8_t*)&message;
    RF_cmdPropTx.startTrigger.triggerType = TRIG_ABSTIME;
    RF_cmdPropTx.startTime = 0;

    message.beaconInterval = RF_convertMsToRatTicks(BEACON_INTERVAL_MS);

    /* Request access to the radio. This does not power-up the RF core, but only initialise
     * the driver and cache the setup command. */
    RF_Params rfParams;
    RF_Params_init(&rfParams);
#if defined(DeviceFamily_CC26X0R2)
    rfHandle = RF_open(&rfObject, &RF_prop, (RF_RadioSetup*)&RF_cmdPropRadioSetup, &rfParams);
#else
    rfHandle = RF_open(&rfObject, &RF_prop, (RF_RadioSetup*)&RF_cmdPropRadioDivSetup, &rfParams);
#endif// DeviceFamily_CC26X0R2
    if (rfHandle == NULL)
    {
        while(1);
    }

    /* Increase the SWI priority of the internal state machine in the RF driver. The default
     * value is 0. This is necessary because the button callback blocks for a long time and
     * might disturb the RF driver state machine.
     */
    uint32_t swiPriority = 1;
    RF_control(rfHandle, RF_CTRL_SET_SWI_PRIORITY, &swiPriority);

    /* Set the frequency. Now the RF driver powers the RF core up and runs the setup command from above.
     * The FS command is executed and also cached for later use when the RF driver does an automatic
     * power up. */
    RF_EventMask result = RF_runCmd(rfHandle, (RF_Op*)&RF_cmdFs, RF_PriorityNormal, NULL, 0);
    if ((result != RF_EventLastCmdDone) || ((volatile RF_Op*)&RF_cmdFs)->status != DONE_OK)
    {
        while(1);
    }

    /* Use the current time as an anchor point for future time stamps.
     * The Nth transmission in the future will be exactly N * 500ms after
     * this time stamp.  */
    RF_cmdPropTx.startTime = RF_getCurrentTime();

    /* A trigger in the past is triggered as soon as possible.
     * No error is given.
     * This avoids assertion when button debouncing causes delay in TX trigger.  */
    RF_cmdPropTx.startTrigger.pastTrig = 1;

    /* Route the PA signal to an LED to indicate ongoing transmissions.
     * Available signals are listed in the proprietary RF user's guide.
     */
    GPIO_setMux(CONFIG_GPIO_RLED, IOC_PORT_RFC_GPO1);

    StateMachine_setNextState(&stateMachine, PeriodicBeaconState);
}

void PeriodicBeaconState_function()
{
    /* Set absolute TX time in the future to utilise "deferred dispatching of commands with absolute timing".
     * This is explained in the proprietary RF user's guide. */
    RF_cmdPropTx.startTime += RF_convertMsToRatTicks(BEACON_INTERVAL_MS);

    message.txTime = RF_cmdPropTx.startTime;
    message.ledState = GPIO_read(CONFIG_GPIO_GLED);

    /* Because the TX command is due in 500ms and we use TRIG_ABSTIME as start trigger type,
     * the RF driver will now power down the RF core and and wait until ~1.5ms before RF_cmdPropTx.startTime.
     * Then the RF driver will power-up the RF core, re-synchronise the RAT and re-run the setup procedure.
     * The setup procedure includes RF_cmdPropRadioDivSetup and RF_cmdFs from above.
     * This will guarantee that RF_cmdPropTx is delivered to the RF core right before it has
     * to start. This is fully transparent to the application. It appears as the RF core was
     * never powered down.
     * This concept is explained in the proprietary RF user's guide. */
    RF_EventMask result = RF_runCmd(rfHandle, (RF_Op*)&RF_cmdPropTx, RF_PriorityNormal, NULL, 0);
    if ((result != RF_EventLastCmdDone) || ((volatile RF_Op*)&RF_cmdPropTx)->status != PROP_DONE_OK)
    {
        while(1);
    }

    if (StateMachine_pendEvents(&stateMachine, Event_TxModeButtonPushed, STATEMACHINE_PEND_NON_BLOCKING) & Event_TxModeButtonPushed)
    {
        StateMachine_setNextState(&stateMachine, SpontaneousBeaconState);
    }
    else
    {
        // PeriodicBeaconState_function() will be entered again.
        StateMachine_setNextState(&stateMachine, PeriodicBeaconState);
    }
}


void SpontaneousBeaconState_function()
{
    StateMachine_EventMask events = StateMachine_pendEvents(&stateMachine, Event_TxModeButtonPushed | Event_LedButtonPushed, STATEMACHINE_PEND_BLOCKING);

    /* We need to find the next synchronized time slot that is far enough
     * in the future to allow the RF driver to power up the RF core.
     * We use 2 ms as safety margin. */
    uint32_t currentTime = RF_getCurrentTime() + RF_convertMsToRatTicks(2);
    uint32_t intervalsSinceLastPacket = DIV_INT_ROUND_UP(currentTime - RF_cmdPropTx.startTime, RF_convertMsToRatTicks(BEACON_INTERVAL_MS));
    RF_cmdPropTx.startTime += intervalsSinceLastPacket * RF_convertMsToRatTicks(BEACON_INTERVAL_MS);

    if (events & Event_TxModeButtonPushed)
    {
        StateMachine_setNextState(&stateMachine, PeriodicBeaconState);
    }

    if (events & Event_LedButtonPushed)
    {
        message.txTime = RF_cmdPropTx.startTime;
        message.ledState = GPIO_read(CONFIG_GPIO_GLED);

        /* Because the TX command is due in 500ms and we use TRIG_ABSTIME as start trigger type,
         * the RF driver will now power down the RF core and and wait until ~1.5ms before RF_cmdPropTx.startTime.
         * Then the RF driver will power-up the RF core, re-synchronise the RAT and re-run the setup procedure.
         * The setup procedure includes RF_cmdPropRadioDivSetup and RF_cmdFs from above.
         * This will guarantee that RF_cmdPropTx is delivered to the RF core right before it has
         * to start. This is fully transparent to the application. It appears as the RF core was
         * never powered down.
         * This concept is explained in the proprietary RF user's guide. */
        RF_EventMask result = RF_runCmd(rfHandle, (RF_Op*)&RF_cmdPropTx, RF_PriorityNormal, NULL, 0);
        if ((result != RF_EventLastCmdDone) || ((volatile RF_Op*)&RF_cmdPropTx)->status != PROP_DONE_OK)
        {
            while(1);
        }
    }
}
