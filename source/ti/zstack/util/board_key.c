/******************************************************************************

 @file board_key.c

 @brief This file contains the interface to the Launchpad Key Service

 <BR>
 NOTE:  The launchpad only has 2 buttons, so only KEY_LEFT and KEY_RIGHT are
        enabled.
 <BR>
 Group: WCS LPC
 Target Device: CC2652

 ******************************************************************************

 Copyright (c) 2016-2018, Texas Instruments Incorporated
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
 Release Name: simplelink_cc26x2_sdk_2_10_00_44_s
 Release Date: 2018-04-09 12:59:57
 *****************************************************************************/

/******************************************************************************
 Includes
 *****************************************************************************/

#include <stdbool.h>
#include <chipinfo.h>
#include "util_timer.h"
#include "board_key.h"

/******************************************************************************
 Local Variables
 *****************************************************************************/

/* Value of keys Pressed */
static uint8_t keysPressed;

/* Key debounce clock */
static ClockP_Struct keyChangeClock;

/* Pointer to application callback */
static Board_Key_keysPressedCB_t appKeyChangeHandler = NULL;

/*
 Create the MSA KEY pin table. This will override the key attributes in
 BoardGpioInitTable[].
 */
static PIN_Config keyPinTable[] = {
      CONFIG_PIN_BTN1 | PIN_INPUT_EN | PIN_PULLUP | PIN_IRQ_NEGEDGE,
      CONFIG_PIN_BTN2 | PIN_INPUT_EN | PIN_PULLUP | PIN_IRQ_NEGEDGE,
      PIN_TERMINATE /* Terminate list */
    };

#if defined(CC13X2R1_LAUNCHXL) && !defined(NO_CC1312R1_SUPPORT)
static PIN_Config keyPinTable_CC1312R1[] = {
      Board_PIN_BUTTON0_CC1312R1 | PIN_INPUT_EN | PIN_PULLUP | PIN_IRQ_NEGEDGE,
      Board_PIN_BUTTON1_CC1312R1 | PIN_INPUT_EN | PIN_PULLUP | PIN_IRQ_NEGEDGE,
      PIN_TERMINATE /* Terminate list */
    };
#endif

/* KEY pin state */
static PIN_State keyPinState;

/* KEY Pin Handle */
static PIN_Handle keyPinHandle;

/* KEY Pid Id */
static PIN_Id keyButton0;
static PIN_Id keyButton1;

/******************************************************************************
 Local Functions
 *****************************************************************************/
static void board_key_changeHandler(UArg a0);
static void board_key_keyFxn(PIN_Handle keyPinHandle, PIN_Id keyPinId);


/******************************************************************************
 Public Functions
 *****************************************************************************/

/*!
 Enable interrupts for keys on PIN.

 Public function defined in board_key.h
 */
uint8_t Board_Key_initialize(Board_Key_keysPressedCB_t appKeyCB)
{
    uint8_t keyState;

    /* Setup KEY ISR */
#if defined(CC13X2R1_LAUNCHXL) && !defined(NO_CC1312R1_SUPPORT)
#ifndef TIMAC_AGAMA_FPGA
    if (ChipInfo_GetChipType() == CHIP_TYPE_CC1312)
    {
        keyButton0 = Board_PIN_BUTTON0_CC1312R1;
        keyButton1 = Board_PIN_BUTTON1_CC1312R1;
        keyPinHandle = PIN_open(&keyPinState, keyPinTable_CC1312R1);
    }
    else
#endif
    {
        keyButton0 = CONFIG_PIN_BTN1;
        keyButton1 = CONFIG_PIN_BTN2;
        keyPinHandle = PIN_open(&keyPinState, keyPinTable);
    }
#else
    keyButton0 = CONFIG_PIN_BTN1;
    keyButton1 = CONFIG_PIN_BTN2;
    keyPinHandle = PIN_open(&keyPinState, keyPinTable);
#endif
    /* Get current key state */
    keyState = board_key_getValues();

    /* Register Callbacks */
    PIN_registerIntCb(keyPinHandle, board_key_keyFxn);

    /* Setup keycallback for keys */
    UtilTimer_construct(&keyChangeClock, board_key_changeHandler,
                    (KEY_DEBOUNCE_TIMEOUT),
                    0, false, 0);

    /* Set the application callback */
    appKeyChangeHandler = appKeyCB;

    return(keyState);
}

/*!
 * @brief       Interrupt handler for a Key press
 *
 * @param       keyPinHandle - PIN Handle
 * @param       keyPinId - PIN ID
 */
static void board_key_keyFxn(PIN_Handle keyPinHandle, PIN_Id keyPinId)
{
    (void)keyPinHandle;

    if(keyPinId == keyButton0)
    {
        keysPressed |= KEY_LEFT;
    }
    else if(keyPinId == keyButton1)
    {
        keysPressed |= KEY_RIGHT;
    }

    if(UtilTimer_isActive(&keyChangeClock) != true)
    {
        UtilTimer_start(&keyChangeClock);
    }
}

/*!
 * @brief       Handler for key change
 *
 * @param       UArg a0 - ignored
 */
static void board_key_changeHandler(UArg a0)
{
    if(appKeyChangeHandler != NULL)
    {
        /* Notify the application */
        (*appKeyChangeHandler)(keysPressed);

        /* Clear keys */
        keysPressed = 0;
    }
}

/*!
 * @brief       Get the current value for all the keys
 *
 * @return      bit mapped representation of all keys
 */
uint8_t board_key_getValues(void)
{
    uint8_t keys = 0;

    if(PIN_getInputValue(keyButton0) == false)
    {
        keys |= KEY_LEFT;
    }

    if(PIN_getInputValue(keyButton1) == false)
    {
        keys |= KEY_RIGHT;
    }

    return(keys);
}

