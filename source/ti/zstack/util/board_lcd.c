/******************************************************************************

 @file board_lcd.c

 @brief This file contains the interface to the LaunchPad LCD Service

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

#include <string.h>
#include <xdc/runtime/System.h>

/* TI-RTOS Header files */
#include <ti/drivers/PIN.h>
#include <ti/display/Display.h>
#include <ti/display/DisplaySharp.h>
#include <ti/display/DisplayExt.h>

#include "ti_drivers_config.h"
#include "board_lcd.h"
#include "mac_util.h"
#include "osal_port.h"

/******************************************************************************
 Local Constants
 *****************************************************************************/

#define MAX_LCD_BUF 40

/******************************************************************************
 Local Variables
 *****************************************************************************/
#if defined(BOARD_DISPLAY_USE_LCD) || defined(BOARD_DISPLAY_USE_UART)

static uint8_t lcdBuf[MAX_LCD_BUF];

#if defined(BOARD_DISPLAY_USE_LCD)
/* LCD parameters */
static Display_Params params;
static Display_Handle hDisp;
static DisplaySharpColor_t colors = {.fg = ClrWhite, .bg = ClrBlack};

#endif

/******************************************************************************
 Public Functions
 *****************************************************************************/


/*!
 Open LCD peripheral on Launchpad.

 Public function defined in board_lcd.h
 */
void Board_LCD_open(void)
{
#if defined(BOARD_DISPLAY_USE_LCD)
    Display_Params_init(&params);
    params.lineClearMode = DISPLAY_CLEAR_NONE;

    hDisp = Display_open(Display_Type_LCD, &params);

    Display_control(hDisp, DISPLAYSHARP_CMD_SET_COLORS, &colors );
#endif
}


/*!
 * Write a string on the LCD display.
 *
 * Public function defined in board_lcd.h
 */
void Board_Lcd_writeString(char *str, uint8_t line)
{
#if defined(BOARD_DISPLAY_USE_UART)
    System_printf(str);
    System_printf("\r\n");
#endif

#if defined(BOARD_DISPLAY_USE_LCD)
    if(hDisp != NULL)
    {
        Display_control(hDisp, DISPLAY_CMD_TRANSPORT_OPEN, NULL);
        Display_print0(hDisp, line, 0, str);
        Display_control(hDisp, DISPLAY_CMD_TRANSPORT_CLOSE, NULL);
    }
#endif /* BOARD_DISPLAY_USE_LCD */
}

/*!
 * Write a string and value on the LCD display.
 *
 * Public function defined in board_lcd.h
 */
void Board_Lcd_writeStringValue(char *str, uint16_t value, uint8_t format,
                                uint8_t line)
{
    int len = strlen(str);
    memset(lcdBuf, 0, MAX_LCD_BUF);
    OsalPort_memcpy(lcdBuf, str, len);
    Util_itoa(value, &lcdBuf[len], format);

#if defined(BOARD_DISPLAY_USE_UART)
    System_printf((xdc_CString)lcdBuf);
    System_printf("\r\n");
#endif

#if defined(BOARD_DISPLAY_USE_LCD)
    if(hDisp != NULL)
    {
        Display_control(hDisp, DISPLAY_CMD_TRANSPORT_OPEN, NULL);
        Display_print0(hDisp, line, 0, lcdBuf);
        Display_control(hDisp, DISPLAY_CMD_TRANSPORT_CLOSE, NULL);
    }
#endif /* BOARD_DISPLAY_USE_LCD */
}

#endif
