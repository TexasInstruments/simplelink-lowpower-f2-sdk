/******************************************************************************

 @file board_lcd.h

 @brief This file contains the LCD Service definitions and prototypes.

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
#ifndef BOARD_LCD_H
#define BOARD_LCD_H

/******************************************************************************
 Includes
 *****************************************************************************/
#ifndef IS_HLOS
#include <ti/display/lcd/LCDDogm1286.h>
#endif //!IS_HLOS

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef IS_HLOS

/*!
 \defgroup BoardLCD Board LCD Functions
 <BR>
 This module is a collection of functions to control the LCD.
 <BR>
 */

/*!
 * \ingroup BoardLCD
 * @{
 */

#if defined(BOARD_DISPLAY_USE_LCD) && defined(BOARD_DISPLAY_USE_UART)
#error "can't enable both BOARD_DISPLAY_USE_LCD and BOARD_DISPLAY_USE_UART"
#endif

/*! LCD macros */
#if defined(BOARD_DISPLAY_USE_LCD) || defined(BOARD_DISPLAY_USE_UART)
/*! Macro definition to write a string to the LCD */
#define LCD_WRITE_STRING(str, line) Board_Lcd_writeString(str, line)
/*! Macro definition to write a string with a value to the LCD */
#define LCD_WRITE_STRING_VALUE(str, value, format, line) \
    Board_Lcd_writeStringValue(str, value, format, line)
#else
/*! Macro definition to write a string to the LCD */
#define LCD_WRITE_STRING(str, line)
/*! Macro definition to write a string with a value to the LCD */
#define LCD_WRITE_STRING_VALUE(str, value, format, line)
#endif

/******************************************************************************
 API Functions
 *****************************************************************************/

#if defined(BOARD_DISPLAY_USE_LCD) || defined(BOARD_DISPLAY_USE_UART)
/*!
 * @brief   Open LCD for launch pad only
 */
extern void Board_LCD_open(void);
#else
#define Board_LCD_open()
#endif

/*!
 * @brief   Write a string on the LCD display.
 *
 * @param   str - string to print
 * @param   line - line (page) to write (0-7)
 */
extern void Board_Lcd_writeString(char *str, uint8_t line);

/*!
 * @brief   Write a string and value on the LCD display.
 *
 * @param   str - string to print
 * @param   value - value to print
 * @param   format - base of the value to print (2,8,16 etc)
 * @param   line - line (page) to write (0-7)
 */
extern void Board_Lcd_writeStringValue(char *str, uint16_t value,
                                       uint8_t format,
                                       uint8_t line);

#else //!IS_HLOS

#define Board_Lcd_printf(line, ...) \
/*move curser to line*/ \
fprintf(stderr,"\033[%d;0H", line+1); \
/*clear line */ \
fprintf(stderr,"\033[2K"); \
fprintf(stderr,__VA_ARGS__); \
fprintf(stderr,"\n");

#define Board_LCD_open() \
fprintf(stderr,"\033[2J");

#endif //!IS_HLOS

/*! @} end group BoardLCD */

#ifdef __cplusplus
}
#endif

#endif /* BOARD_LCD_H */
