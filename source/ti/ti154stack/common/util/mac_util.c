/******************************************************************************

 @file  mac_util.c

 @brief Utility functions commonly used by TIMAC applications

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

/******************************************************************************
 Includes
 *****************************************************************************/

#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#ifndef __unix__
#include <ti/drivers/dpl/HwiP.h>
#else
#include "stdlib.h"
#include "compiler.h"
#include "hlos_specific.h"
#endif

#include "mac_util.h"
#include "api_mac.h"

/******************************************************************************
 Typedefs
 *****************************************************************************/
#define UTIL_SADDR_EXT_LEN  8

/******************************************************************************
 Public Functions
 *****************************************************************************/

/*!
 Converts from a uint16 to ascii hex string.

 Public function defined in mac_util.h
 */
void Util_uint16toa(uint16_t u, char *string)
{
    if(string == NULL)
    {
        return;
    }

    /* add preceding zeros */
    if(u < 0x1000)
    {
        *string++ = '0';
    }

    if(u < 0x0100)
    {
        *string++ = '0';
    }

    if(u < 0x0010)
    {
        *string++ = '0';
    }

    Util_ltoa( (unsigned long)u, (unsigned char *)string, 16 );
}

/*!
 Convert a 16bit number to ASCII

 Public function defined in mac_util.h
 */
void Util_itoa(uint16_t num, uint8_t *buf, uint8_t radix)
{
    char c, i;
    uint8_t *p, rst[5];

    p = rst;
    for(i = 0; i < 5; i++, p++)
    {
        c = num % radix;  /* Isolate a digit */
        *p = c + ( (c < 10) ? '0' : '7' );  /* Convert to Ascii */
        num /= radix;
        if(!num)
        {
            break;
        }
    }

    for(c = 0; c <= i; c++)
    {
        *buf++ = *p--;  /* Reverse character order */
    }

    *buf = '\0';
}

/*!
 Convert a long unsigned int to a string.

 Public function defined in mac_util.h
 */
unsigned char *Util_ltoa(uint32_t l, uint8_t *buf, uint8_t radix)
{
#if defined (__GNUC__) && !defined(__unix__) && !defined(__clang__)
    return( (char *)ltoa(l, buf, radix) );
#else
    unsigned char tmp1[10] = "", tmp2[10] = "", tmp3[10] = "";
    unsigned short num1, num2, num3;
    unsigned char i;

    buf[0] = '\0';

    if(radix == 10)
    {
        num1 = l % 10000;
        num2 = (l / 10000) % 10000;
        num3 = (unsigned short)(l / 100000000);

        if(num3)
        {
            Util_itoa(num3, tmp3, 10);
        }
        if(num2)
        {
            Util_itoa(num2, tmp2, 10);
        }
        if(num1)
        {
            Util_itoa(num1, tmp1, 10);
        }

        if(num3)
        {
            strcpy( (char *)buf, (char const *)tmp3 );
            for(i = 0; i < 4 - strlen( (char const *)tmp2 ); i++)
            {
                strcat( (char *)buf, "0" );
            }
        }
        strcat( (char *)buf, (char const *)tmp2 );
        if(num3 || num2)
        {
            for(i = 0; i < 4 - strlen( (char const *)tmp1 ); i++)
            {
                strcat( (char *)buf, "0" );
            }
        }
        strcat( (char *)buf, (char const *)tmp1 );
        if(!num3 && !num2 && !num1)
        {
            strcpy( (char *)buf, "0" );
        }
    }
    else if(radix == 16)
    {
        num1 = l & 0x0000FFFF;
        num2 = l >> 16;

        if(num2)
        {
            Util_itoa(num2, tmp2, 16);
        }
        if(num1)
        {
            Util_itoa(num1, tmp1, 16);
        }

        if(num2)
        {
            strcpy( (char *)buf, (char const *)tmp2 );
            for(i = 0; i < 4 - strlen( (char const *)tmp1 ); i++)
            {
                strcat( (char *)buf, "0" );
            }
        }
        strcat( (char *)buf, (char const *)tmp1 );
        if(!num2 && !num1)
        {
            strcpy( (char *)buf, "0" );
        }
    }
    else
    {
        return(NULL);
    }

    return(buf);
#endif
}

/*!
 Get the high byte of a uint16_t variable

 Public function defined in mac_util.h
 */
uint8_t Util_hiUint16(uint16_t a)
{
    return((a >> 8) & 0xFF);
}

/*!
 Get the low byte of a uint16_t variable

 Public function defined in mac_util.h
 */
uint8_t Util_loUint16(uint16_t a)
{
    return((a) & 0xFF);
}

/*!
 Build a uint16_t out of 2 uint8_t variables

 Public function defined in mac_util.h
 */
uint16_t Util_buildUint16(uint8_t loByte, uint8_t hiByte)
{
    return((uint16_t)(((loByte) & 0x00FF) + (((hiByte) & 0x00FF) << 8)));
}

/*!
 Build a uint32_t out of 4 uint8_t variables

 Public function defined in mac_util.h
 */
uint32_t Util_buildUint32(uint8_t byte0, uint8_t byte1, uint8_t byte2,
                            uint8_t byte3)
{
    return((uint32_t)((uint32_t)((byte0) & 0x00FF) +
                     ((uint32_t)((byte1) & 0x00FF) << 8) +
                     ((uint32_t)((byte2) & 0x00FF) << 16) +
                     ((uint32_t)((byte3) & 0x00FF) << 24)));
}

/*!
 Pull 1 uint8_t out of a uint32_t

 Public function defined in mac_util.h
 */
uint8_t Util_breakUint32(uint32_t var, int byteNum)
{
    return(uint8_t)((uint32_t)(((var) >> ((byteNum) * 8)) & 0x00FF));
}

/*!
 Build a uint16_t from a uint8_t array

 Public function defined in mac_util.h
 */
uint16_t Util_parseUint16(uint8_t *pArray)
{
    return(Util_buildUint16(pArray[0], pArray[1]));
}

/*!
 Build a uint32_t from a uint8_t array

 Public function defined in mac_util.h
 */
uint32_t Util_parseUint32(uint8_t *pArray)
{
    return(Util_buildUint32(pArray[0], pArray[1], pArray[2], pArray[3]));
}

/*!
 Break and buffer a uint16_t value - LSB first

 Public function defined in mac_util.h
 */
uint8_t *Util_bufferUint16(uint8_t *pBuf, uint16_t val)
{
    *pBuf++ = Util_loUint16(val);
    *pBuf++ = Util_hiUint16(val);

    return(pBuf);
}

/*!
 Break and buffer a uint32_t value - LSB first

 Public function defined in mac_util.h
 */
uint8_t *Util_bufferUint32(uint8_t *pBuf, uint32_t val)
{
    *pBuf++ = Util_breakUint32(val, 0);
    *pBuf++ = Util_breakUint32(val, 1);
    *pBuf++ = Util_breakUint32(val, 2);
    *pBuf++ = Util_breakUint32(val, 3);

    return(pBuf);
}

/*!
  Utility function to clear an event

 Public function defined in mac_util.h
 */
void Util_clearEvent(uint16_t *pEvent, uint16_t event)
{
#ifndef __unix__
    uint32_t key;

    /* Enter critical section */
    key = HwiP_disable();
#else
    _ATOMIC_global_lock();
#endif

    /* Clear the event */
    *pEvent &= ~(event);

    /* Exit critical section */
#ifndef __unix__
    HwiP_restore(key);
#else
    _ATOMIC_global_unlock();
#endif
}

/*!
  Utility function to set an event

 Public function defined in mac_util.h
 */
void Util_setEvent(uint16_t *pEvent, uint16_t event)
{
#ifndef __unix__
    uint32_t key;

    /* Enter critical section */
    key = HwiP_disable();
#else
    _ATOMIC_global_lock();
#endif

    /* Set the event */
    *pEvent |= event;

    /* Exit critical section */
#ifndef __unix__
    HwiP_restore(key);
#else
    _ATOMIC_global_unlock();
#endif
}

/*!
  Utility function to copy the extended address

 Public function defined in mac_util.h
 */
void Util_copyExtAddr(void *pSrcAddr, void *pDstAddr)
{
    memcpy(pSrcAddr, pDstAddr, (UTIL_SADDR_EXT_LEN));
}
