/*
 * Copyright (c) 2022-2023, Texas Instruments Incorporated
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
/** ===========================================================================
 *  @file       TimestampP.h
 *
 *  @brief      Timestamp module for the RTOS Porting Interface
 *
 *  Utility functions to get device timestamp
 *
 *  ===========================================================================
 */

#ifndef ti_dpl_TimestampP__include
#define ti_dpl_TimestampP__include

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TimestampP_Exponent_Seconds      0
#define TimestampP_Exponent_Miliseconds  3
#define TimestampP_Exponent_Microseconds 6
#define TimestampP_Exponent_Nanoseconds  9

/* @brief Specification for parsing native device timestamp
 *
 * When provided with a native timestamp, however many bits, a peer device
 * may perform the calculation:
 *
 * @code
 *   double fractional = (ts & ((1 << fracBytes*8) - 1)) / 2^fracBytes*8
 *   double integral   = (ts >> fracBytes*8) & ((1 << intBytes*8) - 1)
 *   // if multiplier > 0:
 *   double time       =   abs(multiplier) * (integral + fractional) * 10^-exponent
 *   // if multiplier < 0:
 *   double time       = 1/abs(multiplier) * (integral + fractional) * 10^-exponent
 * @endcode
 *
 * For example, if the native format is a 32-bit wide fixed point fractional
 * value with 16 bits for seconds and subseconds, the setting would be:
 * fracBytes = 2, intBytes = 2, multiplier = 1, exponent = 0
 *
 * On the other hand if the native format is a 32-bit wide integral value
 * where each tick is worth 8 microseconds, you get this:
 * fracBytes = 0, intBytes = 4, multiplier = 8, exponent = 6
 *
 * If the native format is a 48-bit wide 32786 Hz counter where each
 * tick is worth 30.5175 etc us, the setting would be:
 * fracBytes = 0, intBytes = 6, multiplier = -32768, exponent = 0
 *
 * It is expected that if there is a fractional part, it is in the LSB end.
 */
typedef union TimestampP_Format
{
    struct
    {
        uint32_t fracBytes:4;  //<! Octets (LSB) used for fractional part (if any)
        uint32_t intBytes:4;   //<! Octets (MSB) used for integer part
        uint32_t exponent:8;   //<! How much to scale native time to get seconds.
        int32_t multiplier:16; //<! Signed 16-bit multiplier, eg 8 if one tick is 8 time units
    } format;
    uint32_t value;
} TimestampP_Format;

extern uint32_t TimestampP_getNative32(void);
extern uint64_t TimestampP_getNative64(void);
extern const TimestampP_Format TimestampP_nativeFormat64;
extern const TimestampP_Format TimestampP_nativeFormat32;

#ifdef __cplusplus
}
#endif

#endif /* ti_dpl_TimestampP__include */
