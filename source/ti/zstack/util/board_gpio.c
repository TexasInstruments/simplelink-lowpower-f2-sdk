/******************************************************************************

 @file board_gpio.c

 @brief This file contains the interface to the LaunchPad GPIO Service

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
#include "board_gpio.h"


/******************************************************************************
 Constants
 *****************************************************************************/

/******************************************************************************
 Typedefs
 *****************************************************************************/

/******************************************************************************
 Local Variables
 *****************************************************************************/
#if defined(CC13XX_LAUNCHXL) || defined(CC13X2R1_LAUNCHXL)
#if defined(Board_DIO1_RFSW)
#define Board_DIO_RFSW Board_DIO1_RFSW
#elif defined(Board_DIO30_RFSW)
#define Board_DIO_RFSW Board_DIO30_RFSW
#endif
#endif

static PIN_Config gpioPinTable[] =
{
#if defined(CC13XX_LAUNCHXL) || defined(CC13X2R1_LAUNCHXL)

#if !defined(FREQ_2_4G)
    Board_DIO_RFSW   | PIN_GPIO_OUTPUT_EN | PIN_GPIO_HIGH | PIN_PUSHPULL | PIN_DRVSTR_MAX,   /* RF SW Switch defaults to sub-1GHz path*/
#else
    Board_DIO_RFSW   | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX,   /* RF SW Switch defaults to 2GHz path*/
#endif

#if defined(CC13XX_LAUNCHXL) && !defined(COPROCESSOR)
#if (CONFIG_RANGE_EXT_MODE == APIMAC_NO_EXTENDER)
    Board_DIO30_SWPWR | PIN_GPIO_OUTPUT_EN | PIN_GPIO_HIGH | PIN_PUSHPULL | PIN_DRVSTR_MAX,    /* External RF Switch is powered on by default */
#endif
#endif

#endif
    PIN_TERMINATE /* Terminate list     */
};

/* GPIO pin state */
static PIN_State gpioPinState;

/* GPIO Pin Handle */
static PIN_Handle gpioPinHandle;

/******************************************************************************
 Local Function Prototypes
 *****************************************************************************/

/******************************************************************************
 Public Functions
 *****************************************************************************/

/*!
 Initialize GPIOs

 Public function defined in board_gpio.h
 */
void Board_Gpio_initialize(void)
{
    /* Open GPIO PIN driver */
    gpioPinHandle = PIN_open(&gpioPinState, gpioPinTable);
}

/*!
 Control the state of an GPIO

 Public function defined in board_gpio.h
 */
void Board_Gpio_control(board_gpio_type gpio, uint32_t value)
{
#if defined(CC13XX_LAUNCHXL) || defined(CC13X2R1_LAUNCHXL)
    uint32_t key;
#endif

    if (gpioPinHandle == NULL)
    {
        return;
    }

#if defined(CC13XX_LAUNCHXL) || defined(CC13X2R1_LAUNCHXL)
    if (gpio == board_rfSwitch_select)
    {
        key = HwiP_disable();
        PIN_setOutputValue(gpioPinHandle, Board_DIO_RFSW, value);
        HwiP_restore(key);
        return;
    }

#if defined(CC13XX_LAUNCHXL) && !defined(COPROCESSOR)
#if (CONFIG_RANGE_EXT_MODE == APIMAC_NO_EXTENDER)
    if (gpio == board_rfSwitch_pwr)
    {
        key = HwiP_disable();
        PIN_setOutputValue(gpioPinHandle, Board_DIO30_SWPWR, value);
        HwiP_restore(key);
        return;
    }
#endif
#endif

#endif
}
