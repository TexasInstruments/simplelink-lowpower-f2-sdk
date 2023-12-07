/*
 * Copyright (c) 2015-2022, Texas Instruments Incorporated
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

#ifndef atTerm__include
#define atTerm__include

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>


#define RADIO_NO_PHY (0xFF)

/***************************************************************************************************
 *
 * Initializes the AT terminal
 *
 * This function configures the UART for use as an AT Terminal. If opening UART fails,
 * system abort is performed.
 *
 * param none
 *
 * return 0 for success
 *
 ***************************************************************************************************/
extern int32_t AtTerm_init(void);

/***************************************************************************************************
 *
 * Gets a char from the AT Terminal. In SPI mode this will block until a SPI transaction
 * occurs then return the entirety of the received command
 *
 * param ch - Pointer to an uint8_t where the character will be read in to
 *
 * return bytes read
 *
 ***************************************************************************************************/
extern int32_t AtTerm_getChar(char* ch);

/***************************************************************************************************
 *
 * Puts a char to the AT Terminal
 *
 * param ch - The char to be written
 *
 * return none
 *
 ***************************************************************************************************/
extern void AtTerm_putChar(char ch);

/***************************************************************************************************
 *
 * Sends a string to the AT Terminal
 *
 * param string - The NULL terminated string to be written
 *
 * return none
 *
 ***************************************************************************************************/
extern void AtTerm_sendString(char* string);

/***************************************************************************************************
 *
 * Sends a string and an uint8_t to the AT Terminal
 *
 * param string - The NULL terminated string to be written
 * param value -  The uint8_t value to be written
 * param format - Base 10 or 16 (Dec or Hex)
 *
 * return none
 *
 ***************************************************************************************************/
extern void AtTerm_sendStringUi8Value(char *string, uint8_t value, uint8_t format);

/***************************************************************************************************
 *
 * Sends a string and an int8_t to the AT Terminal
 *
 * param string - The NULL terminated string to be written
 * param value -  The int8_t value to be written
 * param format - Base 10 or 16 (Dec or Hex)
 *
 *return none
 *
 ***************************************************************************************************/
extern void AtTerm_sendStringI8Value(char *string, int8_t value, uint8_t format);

/***************************************************************************************************
 *
 * Sends a string and an uint16_t to the AT Terminal
 *
 * param string - The NULL terminated string to be written
 * param value -  The uint16_t value to be written
 * param format - Base 10 or 16 (Dec or Hex)
 *
 * return none
 *
 ***************************************************************************************************/
extern void AtTerm_sendStringUi16Value(char* string, uint16_t value, uint8_t format);

/***************************************************************************************************
 *
 * Sends a string and an int16_t to the AT Terminal
 *
 * param string - The NULL terminated string to be written
 * param value -  The int16_t value to be written
 * param format - Base 10 or 16 (Dec or Hex)
 *
 * return none
 *
 ***************************************************************************************************/
extern void AtTerm_sendStringI16Value(char *string, int16_t value, uint8_t format);

/***************************************************************************************************
 *
 * Sends a string and an uint32_t to the AT Terminal
 *
 * param string - The NULL terminated string to be written
 * param value -  The uint32_t value to be written
 * param format - Base 10 or 16 (Dec or Hex)
 *
 * return none
 *
 ***************************************************************************************************/
extern void AtTerm_sendStringUi32Value( char *string, uint32_t value, uint8_t format);

/***************************************************************************************************
 *
 * Sends a string and an int32_t to the AT Terminal
 *
 * param string - The NULL terminated string to be written
 * param value -  The int32_t value to be written
 * param format - Base 10 or 16 (Dec or Hex)
 *
 * return none
 *
 ***************************************************************************************************/
extern void AtTerm_sendStringI32Value(char *string, int32_t value, uint8_t format);

/***************************************************************************************************
 *
 * Sends a FF (NP form feed, new page) AT Terminal
 *
 * param none
 *
 * return none
 *
 ***************************************************************************************************/
extern void AtTerm_clearTerm(void);

/***************************************************************************************************
 *
 * Extract the radio ID and AT command parameter from the string entered into the terminal
 *
 * param paramStr terminal string
 * param radioId  radio ID extracted from terminal string is stored here. Can be NULL; the current
 *                radio may have been set somewhere else.
 * param fxnParam AT command parameter extracted from terminal string is stored here. Can be NULL.
 * param fxnParam2 AT command parameter extracted from terminal string is stored here. Can be NULL.
 * param fxnParamLen length (in bytes) of the AT command parameter. 0 if fxnParam is NULL
 *
 * return none
 *
 ***************************************************************************************************/
extern void AtTerm_getIdAndParam(char *paramStr, uint8_t *radioId, uintptr_t fxnParam, uintptr_t fxnParam2, size_t fxnParamLen);


#ifdef __cplusplus
}
#endif

#endif /* atTerm__include */
