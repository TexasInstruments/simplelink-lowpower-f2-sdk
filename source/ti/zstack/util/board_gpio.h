/******************************************************************************

 @file board_gpio.h

 @brief This file contains the RF GPIO Service definitions and prototypes.

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
#ifndef BOARD_GPIO_H
#define BOARD_GPIO_H

/******************************************************************************
 Includes
 *****************************************************************************/
#if !defined(COPROCESSOR)
#include "config.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/*!
 \defgroup BoardGPIO Board GPIO Functions
 <BR>
 This module is a collection of functions to control GPIOs.
 <BR>
 */

/******************************************************************************
 Typedefs
 *****************************************************************************/

/*!
 * \ingroup Board GPIOs
 * @{
 */

/*! GPIO Identifications */
typedef enum
{
#if defined(CC13XX_LAUNCHXL) || defined(CC13X2R1_LAUNCHXL)
    board_rfSwitch_select,
#if defined(CC13XX_LAUNCHXL) && !defined(COPROCESSOR)
#if (CONFIG_RANGE_EXT_MODE == APIMAC_NO_EXTENDER)
    board_rfSwitch_pwr,
#endif
#endif
#endif
    NO_OF_GPIOS
} board_gpio_type;

/******************************************************************************
 API Functions
 *****************************************************************************/

/*!
 * @brief   Initialize GPIO System
 */
void Board_Gpio_initialize(void);

/*!
 * @brief   Control the state of an GPIO
 *
 * @param   gpio - Which GPIO
 * @param   state - set to ON or OFF,
 *
 */
void Board_Gpio_control(board_gpio_type gpio, uint32_t value);

/*! @} end group BoardGPIO */


#ifdef __cplusplus
}
#endif

#endif /* BOARD_LED_H */
