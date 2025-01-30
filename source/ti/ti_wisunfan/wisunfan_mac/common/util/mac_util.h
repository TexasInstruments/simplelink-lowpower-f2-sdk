/******************************************************************************

 @file  mac_util.h

 @brief Declaration of utility functions common to TIMAC Applications.

 Group: WCS LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2016-2025, Texas Instruments Incorporated
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
#ifndef UTIL_H
#define UTIL_H

/******************************************************************************
 Includes
 *****************************************************************************/
#include <stdint.h>

#ifndef COPROCESSOR
//#include "ti_wisunfan_config.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/*!
 \defgroup UtilMisc Utility and Miscellaneous
 <BR>
 Miscellaneous conversion functions.
 <BR>
 */

/******************************************************************************
 Constants and definitions
 *****************************************************************************/
/* aBaseSuperFrame Duration in slots */
#define BASE_SUPER_FRAME_DURATION   960

/* symbol duration for 50 kbps mode in micro seconds*/
#define SYMBOL_DURATION_50_kbps 20

/* symbol duration for 200 kbps mode in micro seconds*/
#define SYMBOL_DURATION_200_kbps 5

/* symbol duration for 250 kbps mode in micro seconds*/
#define SYMBOL_DURATION_250_kbps 16

/* symbol duration for LRM mode in micro seconds*/
#define SYMBOL_DURATION_LRM 50

#ifdef POWER_MEAS
/* POLL only profile */
#define POLL_ACK    1
/* sensor data only profile */
#define DATA_ACK    2
/* Poll + Data */
#define POLL_DATA   3
/* SLEEP */
#define SLEEP   4
#endif

#if !defined(STATIC)
#if defined(UNIT_TEST)
/*! Allow access to the local variables to test code by making them public*/
#define STATIC
#else
/*! Define STATIC as static for local variables */
#define STATIC static
#endif
#endif

#if !defined(CONST)
#if defined(UNIT_TEST)
/*! Allow constant to be used in different context*/
#define CONST
#else
/*! Define CONST as const for local variables */
#define CONST const
#endif
#endif
/*!
 * \ingroup UtilMisc
 * @{
 */

/******************************************************************************
 Function Prototypes
 *****************************************************************************/

/*!
 * @brief      Converts from a uint16 to ascii hex string.
 *             The # will be exactly 4 hex digits (e.g. 0x0000 or 0x1E3F).
 *             NULL terminates the string.
 *
 * @param      u - Number to be converted
 * @param      string - pointer to coverted string
 */
extern void Util_uint16toa(uint16_t u, char *string);

/*!
 * @brief      Convert a 16bit number to ASCII
 *
 * @param      num - number to convert
 * @param      buf - buffer to write ASCII
 * @param      radix - base to convert to (ie. 10 or 16)
 */
extern void Util_itoa(uint16_t num, uint8_t *buf, uint8_t radix);

/*!
 * @brief      Convert a long unsigned int to a string.
 *
 * @param      l - long to convert
 * @param      buf - buffer to convert to
 * @param      radix - 10 dec, 16 hex
 *
 * @return     pointer to buffer
 */
extern unsigned char *Util_ltoa(uint32_t l, uint8_t *buf, uint8_t radix);

/*!
 * @brief      Get the high byte of a uint16_t variable
 *
 * @param      a - uint16_t variable
 *
 * @return     high byte
 */
extern uint8_t Util_hiUint16(uint16_t a);

/*!
 * @brief      Get the low byte of a uint16_t variable
 *
 * @param      a - uint16_t variable
 *
 * @return     low byte
 */
extern uint8_t Util_loUint16(uint16_t a);

/*!
 * @brief      Build a uint16_t out of 2 uint8_t variables
 *
 * @param      loByte - low byte
 * @param      hiByte - high byte
 *
 * @return     combined uint16_t
 */
extern uint16_t Util_buildUint16(uint8_t loByte, uint8_t hiByte);

/*!
 * @brief      Build a uint32_t out of 4 uint8_t variables
 *
 * @param      byte0 - byte - 0
 * @param      byte1 - byte - 1
 * @param      byte2 - byte - 2
 * @param      byte3 - byte - 3
 *
 * @return     combined uint32_t
 */
extern uint32_t Util_buildUint32(uint8_t byte0, uint8_t byte1, uint8_t byte2,
                                 uint8_t byte3);

/*!
 * @brief      Pulls 1 uint8_t out of a uint32_t
 *
 * @param      var - uint32_t variable
 * @param      byteNum - what byte to pull out (0-3)
 *
 * @return     uint8_t
 */
extern uint8_t Util_breakUint32(uint32_t var, int byteNum);

/*!
 * @brief      Build a uint16_t from a uint8_t array
 *
 * @param      pArray - pointer to uint8_t array
 *
 * @return     combined uint16_t
 */

extern uint16_t Util_parseUint16(uint8_t *pArray);

/*!
 * @brief      Build a uint32_t from a uint8_t array
 *
 * @param      pArray - pointer to uint8_t array
 *
 * @return     combined uint32_t
 */
extern uint32_t Util_parseUint32(uint8_t *pArray);

/*!
 * @brief      Break and buffer a uint16 value - LSB first
 *
 * @param      pBuf - ptr to next available buffer location
 * @param      val  - 16-bit value to break/buffer
 *
 * @return     pBuf - ptr to next available buffer location
 */
extern uint8_t *Util_bufferUint16(uint8_t *pBuf, uint16_t val);

/*!
 * @brief      Break and buffer a uint32 value - LSB first
 *
 * @param      pBuf - ptr to next available buffer location
 * @param      val  - 32-bit value to break/buffer
 *
 * @return     pBuf - ptr to next available buffer location
 */
extern uint8_t *Util_bufferUint32(uint8_t *pBuf, uint32_t val);

/*!
 * @brief       Utility function to clear an event
 *
 * @param       pEvent - pointer to event variable
 * @param       event - event(s) to clear
 */
extern void Util_clearEvent(uint16_t *pEvent, uint16_t event);

/*!
 * @brief       Utility function to set an event
 *
 * @param       pEvent - pointer to event variable
 * @param       event - event(s) to clear
 */
extern void Util_setEvent(uint16_t *pEvent, uint16_t event);

/*!
 * @brief       Utility function to copy the extended address
 *
 * @param       pSrcAddr - pointer to source from which to be copied
 * @param       pDstAddr - pointer to destination to copy to
 */
extern void Util_copyExtAddr(void *pSrcAddr, void *pDstAddr);

/*! @} end group UtilMisc */

#ifdef __cplusplus
}
#endif

#endif /* UTIL_H */
