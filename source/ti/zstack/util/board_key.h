/******************************************************************************

 @file board_key.h

 @brief This file contains the Key Press Service definitions and prototypes.

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
#ifndef BOARD_KEY_H
#define BOARD_KEY_H

/******************************************************************************
 Includes
 *****************************************************************************/

#include "ti_drivers_config.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*!
 \defgroup BoardKey Keypress Functions
 <BR>
 This module handles the definition, initilization and key bounce for key
 presses.
 To use this module, call Board_Key_initialize() with a callback function as
 the parameter, then when a key or keys are pressed, the callback function
 will be called (after a debounce period).  Single or multiple key presses are
 detected and passed to the callback function.
 <BR>
 */

/******************************************************************************
 Constants
 *****************************************************************************/

/*!
 * \ingroup BoardKey
 * @{
 */

/*! Select Key ID */
#define KEY_SELECT            0x01
/*! Up Key ID */
#define KEY_UP                0x02
/*! Down Key ID */
#define KEY_DOWN              0x04
/*! Left Key ID */
#define KEY_LEFT              0x08
/*! Right Key ID */
#define KEY_RIGHT             0x10

/*! Debounce timeout in milliseconds */
#define KEY_DEBOUNCE_TIMEOUT  200

/******************************************************************************
 Typedefs
 *****************************************************************************/

/*! Key Press Callback function typedef */
typedef void (*Board_Key_keysPressedCB_t)(uint8_t keysPressed);

/******************************************************************************
 API Functions
 *****************************************************************************/

/*!
 * @brief   Enable interrupts for keys on GPIOs.
 *
 * @param   appKeyCB - application key pressed callback
 *
 * @return  current state of all keys, check for bit masked
 *          KEY_SELECT, KEY_UP, etc.
 */
uint8_t Board_Key_initialize(Board_Key_keysPressedCB_t appKeyCB);


/*!
 * @brief   Get the current value for all the keys
 *
 * @param   none
 *
 * @return  bit mapped representation of all keys
 */
uint8_t board_key_getValues(void);

/*! @} end group BoardKey */

#ifdef __cplusplus
}
#endif

#endif /* BOARD_KEY_H */
