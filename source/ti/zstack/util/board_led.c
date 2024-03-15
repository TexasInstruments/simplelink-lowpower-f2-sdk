/******************************************************************************

 @file board_led.c

 @brief This file contains the interface to the SRF06EB LED Service

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

#include <xdc/std.h>

#include <ti/drivers/PIN.h>

#include <ti/drivers/dpl/HwiP.h>

#include "ti_drivers_config.h"

#include "util_timer.h"
#include "board_led.h"

/******************************************************************************
 Constants
 *****************************************************************************/
#if !defined(BOARD_LED_BLINK_PERIOD)
#define BOARD_LED_BLINK_PERIOD 500     /* in milliseconds */
#endif

typedef enum
{
    BLINKING_STATUS_ON,
    BLINKING_STATUS_OFF,
    BLINKING_STATUS_DONE
} blinkStatus;

/******************************************************************************
 Typedefs
 *****************************************************************************/

typedef struct
{
    board_led_state state; /* Off, On or Blink */
    blinkStatus status; /* What is led status (on, off, or done) */
} board_led_status_t;

/******************************************************************************
 Local Variables
 *****************************************************************************/

static ClockP_Struct blinkClkStruct;

static board_led_status_t ledStatus[NO_OF_LEDS];

static PIN_Config ledPinTable[] =
{
    CONFIG_PIN_RLED | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL
            | PIN_DRVSTR_MAX, /* LED1 initially off */
    CONFIG_PIN_GLED | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL
            | PIN_DRVSTR_MAX, /* LED2 initially off */
    /* To Do */
    /* LED2 and LED3 should be added later */
    PIN_TERMINATE /* Terminate list     */
};

/* LED pin state */
static PIN_State ledPinState;

/* LED Pin Handle */
static PIN_Handle ledPinHandle;

/******************************************************************************
 Local Function Prototypes
 *****************************************************************************/
static void board_led_blinkTimeoutCB(UArg a0);
static bool board_led_anyBlinking(void);
static void board_led_blinkLed(void);
static unsigned int board_led_convertLedType(board_led_type led);
static uint32_t board_led_convertLedValue(board_led_state state);

/******************************************************************************
 Public Functions
 *****************************************************************************/

/*!
 Initialize LEDs

 Public function defined in board_led.h
 */
void Board_Led_initialize(void)
{
    uint8_t x;
    unsigned int index;
    uint32_t value;

    /* Open LED PIN driver */
    ledPinHandle = PIN_open(&ledPinState, ledPinTable);

    value = board_led_convertLedValue(board_led_state_OFF);

    for(x = 0; x < NO_OF_LEDS; x++)
    {
        ledStatus[x].state = board_led_state_OFF;
        ledStatus[x].status = BLINKING_STATUS_DONE;

        index = board_led_convertLedType((board_led_type) x);

        PIN_setOutputValue(ledPinHandle, index, value);
    }

    UtilTimer_construct(&blinkClkStruct, board_led_blinkTimeoutCB,
    BOARD_LED_BLINK_PERIOD,
                        0, false, 0);
}

/*!
 Control the state of an LED

 Public function defined in board_led.h
 */
void Board_Led_control(board_led_type led, board_led_state state)
{
    unsigned int gpioType;
    uint32_t value;
    uint32_t key;

    if (ledPinHandle == NULL)
    {
        return;
    }

    /* Convert to GPIO types */
    gpioType = board_led_convertLedType(led);
    value = board_led_convertLedValue(state);

    /* Look for invalid parameters */
    if((led >= NO_OF_LEDS) || (state > board_led_state_BLINKING))
    {
        return;
    }

    /* Save state and status */
    ledStatus[led].state = state;
    if((state == board_led_state_BLINK) || (state == board_led_state_BLINKING))
    {
        ledStatus[led].status = BLINKING_STATUS_ON;
    }

    /* Enter critical section so this function is thread safe*/
    key = HwiP_disable();

    /* Update hardware LEDs */
    PIN_setOutputValue(ledPinHandle, gpioType, value);

    /* Exit critical section */
    HwiP_restore(key);


    /* Are any LEDs are blinking? */
    if(board_led_anyBlinking())
    {
        if(UtilTimer_isActive(&blinkClkStruct) == false)
        {
            UtilTimer_start(&blinkClkStruct);
        }
    }
    else
    {
        if(UtilTimer_isActive(&blinkClkStruct) == true)
        {
            UtilTimer_stop(&blinkClkStruct);
        }
    }
}

/*!
 Toggle the state of an LED

 Public function defined in board_led.h
 */
void Board_Led_toggle(board_led_type led)
{
    board_led_state newState = board_led_state_OFF;

    /* Look for invalid parameter */
    if(led < NO_OF_LEDS)
    {
        /* Toggle state */
        if(ledStatus[led].state == board_led_state_OFF)
        {
            newState = board_led_state_ON;
        }

        /* Set new state */
        Board_Led_control(led, newState);
    }
}

/******************************************************************************
 Local Functions
 *****************************************************************************/

/*!
 * @brief       Timeout handler function
 *
 * @param       a0 - ignored
 */
static void board_led_blinkTimeoutCB(UArg a0)
{
    /* Update blinking LEDs */
    board_led_blinkLed();

    if(board_led_anyBlinking())
    {
        /* Setup for next time */
        UtilTimer_start(&blinkClkStruct);
    }
}

/*!
 * @brief       Are there any blinking LEDs?
 *
 * @return      true, yes at least one.  false if none
 */
static bool board_led_anyBlinking(void)
{
    uint8_t x;

    for(x = 0; x < NO_OF_LEDS; x++)
    {
        if((ledStatus[x].state == board_led_state_BLINKING) ||
                        ((ledStatus[x].state == board_led_state_BLINK)
                        && (ledStatus[x].status != BLINKING_STATUS_DONE)))
        {
            return (true);
        }
    }

    return (false);
}

/*!
 * @brief       Blink LEDs
 */
static void board_led_blinkLed(void)
{
    uint8_t x;
    uint32_t key;

    for(x = 0; x < NO_OF_LEDS; x++)
    {
        unsigned int index;
        uint32_t value;

        if(ledStatus[x].state == board_led_state_BLINKING)
        {
            index = board_led_convertLedType((board_led_type) x);

            if(ledStatus[x].status == BLINKING_STATUS_OFF)
            {
                value = board_led_convertLedValue(board_led_state_ON);
                ledStatus[x].status = BLINKING_STATUS_ON;
            }
            else
            {
                value = board_led_convertLedValue(board_led_state_OFF);
                ledStatus[x].status = BLINKING_STATUS_OFF;
            }

            /* Enter critical section so this function is thread safe*/
            key = HwiP_disable();

            PIN_setOutputValue(ledPinHandle, index, value);

            /* Exit critical section */
            HwiP_restore(key);
        }
        else if((ledStatus[x].state == board_led_state_BLINK) && (ledStatus[x]
                        .status
                                                                  != BLINKING_STATUS_DONE))
        {
            index = board_led_convertLedType((board_led_type) x);
            value = board_led_convertLedValue(board_led_state_OFF);
            ledStatus[x].status = BLINKING_STATUS_DONE;

            /* Enter critical section so this function is thread safe*/
            key = HwiP_disable();

            PIN_setOutputValue(ledPinHandle, index, value);

            /* Exit critical section */
            HwiP_restore(key);
        }
    }
}

/*!
 * @brief       Convert from board_led type to PIN led type
 *
 * @param       led - board_led type
 *
 * @return      PIN Led Type
 */
static unsigned int board_led_convertLedType(board_led_type led)
{
    if (led == board_led_type_LED1)
    {
        return(CONFIG_PIN_RLED);
    }
    return(CONFIG_PIN_GLED);
}

/*!
 * @brief       Convert from board_led value to GPIO value
 *
 * @param       led - board_led type
 *
 * @return      GPIO value
 */
static uint32_t board_led_convertLedValue(board_led_state state)
{
    uint32_t value;

    switch(state)
    {
        case board_led_state_ON:
        case board_led_state_BLINK:
        case board_led_state_BLINKING:
            value = CONFIG_GPIO_LED_ON;
            break;

        default:
            value = CONFIG_GPIO_LED_OFF;
            break;
    }

    return (value);
}
