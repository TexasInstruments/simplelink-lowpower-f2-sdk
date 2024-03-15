/*
 * Copyright (c) 2023, Texas Instruments Incorporated
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
/**
 *  @file       Math.h
 *
 *  @brief      Math utility functions
 *
 */

#ifndef ti_drivers_utils_Math__include
#define ti_drivers_utils_Math__include

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 *  @brief      Macro to determine the minimum of two numbers.
 *
 *  @warning    Do not use arguments that have a side effect. For example do not
 *              use pre- and post-increment operators.
 *
 *  @param      x  The first number. Either an integer or a floating point type.
 *  @param      y  The second number. Must be the same type as  @c x.
 *
 *  @return     The minimum of  @c x and  @c y
 */
#define Math_MIN(x, y) (((x) < (y)) ? (x) : (y))

/*!
 *  @brief      Macro to determine the maximum of two numbers.
 *
 *  @warning    Do not use arguments that have a side effect. For example do not
 *              use pre- and post-increment operators.
 *
 *  @param      x  The first number. Either an integer or a floating point type.
 *  @param      y  The second number. Must be the same type as  @c x.
 *
 *  @return     The maximum of  @c x and  @c y
 */
#define Math_MAX(x, y) (((x) > (y)) ? (x) : (y))

/*!
 *  @brief      Macro to calculate the absolute value of a numbers.
 *
 *  @warning    Do not use arguments that have a side effect. For example do not
 *              use pre- and post-increment operators.
 *
 *  @param      x  The number to calculate the absolute value of.
 *                 Either a signed integer or a floating point type.
 *
 *  @return     The absolute value of  @c x
 */
#define Math_ABS(x) ((x) < 0 ? -(x) : (x))

/*!
 *  @brief      Divide a number by 1000
 *
 *  This function is intended for devices without a hardware divider (for example CC23X0)
 *  that must run divisions (that are not a power of 2) in software.
 *  The generic software division implementations provided by compilers are
 *  relatively slow. This function only supports dividing by 1000, but
 *  does so in ~16 cycles vs. ~95 cycles for the generic implementations.
 *
 *  @warning    Limitations: The division is only accurate for
 *              @c dividend < 754515999, and off by 1 for values of
 *              @c dividend = 754515999 + 1000*n.
 *
 *  @param      dividend  The dividend to be divided by 1000. Must be below
 *                        754515999 for division to be accurate.
 *
 *  @return     Returns  @c dividend / 1000 (see limitations)
 */
extern uint32_t Math_divideBy1000(uint32_t dividend);

#ifdef __cplusplus
}
#endif

#endif /* ti_drivers_utils_Math__include */