/*
 * Copyright (c) 2022-2024, Texas Instruments Incorporated
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
/*
 *  ======== TimestampPCC26XX.c ========
 */
#include <stdint.h>

#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(driverlib/aon_rtc.h)
#include <ti/drivers/dpl/TimestampP.h>

#define TIMESTAMPP_NATIVEFORMAT32_INITIALIZER                                                                \
    {                                                                                                        \
        .format = {.exponent = TimestampP_Exponent_Seconds, .fracBytes = 2, .intBytes = 2, .multiplier = 1 } \
    }
#define TIMESTAMPP_NATIVEFORMAT64_INITIALIZER                                                                \
    {                                                                                                        \
        .format = {.exponent = TimestampP_Exponent_Seconds, .fracBytes = 4, .intBytes = 4, .multiplier = 1 } \
    }

#if defined(__IAR_SYSTEMS_ICC__)
__root const TimestampP_Format
#elif defined(__TI_COMPILER_VERSION__) || (defined(__clang__) && defined(__ti_version__))
const TimestampP_Format __attribute__((used))
#elif defined(__GNUC__)
const TimestampP_Format __attribute__((section(".timestampPFormat"), used))
#endif
    TimestampP_nativeFormat64 = TIMESTAMPP_NATIVEFORMAT64_INITIALIZER;

#if defined(__IAR_SYSTEMS_ICC__)
__root const TimestampP_Format
#elif defined(__TI_COMPILER_VERSION__) || (defined(__clang__) && defined(__ti_version__))
const TimestampP_Format __attribute__((used))
#elif defined(__GNUC__)
const TimestampP_Format __attribute__((section(".timestampPFormat"), used))
#endif
    TimestampP_nativeFormat32 = TIMESTAMPP_NATIVEFORMAT32_INITIALIZER;

/* Copy of timestamp native formats in .log_data section */
#if defined(__IAR_SYSTEMS_ICC__)
    #pragma location = ".log_data"
__root const TimestampP_Format
#elif defined(__TI_COMPILER_VERSION__) || (defined(__clang__) && defined(__ti_version__)) || defined(__GNUC__)
const TimestampP_Format __attribute__((section(".log_data"), used))
#endif
    TimestampP_nativeFormat64_copy = TIMESTAMPP_NATIVEFORMAT64_INITIALIZER;

#if defined(__IAR_SYSTEMS_ICC__)
    #pragma location = ".log_data"
__root const TimestampP_Format
#elif defined(__TI_COMPILER_VERSION__) || (defined(__clang__) && defined(__ti_version__)) || defined(__GNUC__)
const TimestampP_Format __attribute__((section(".log_data"), used))
#endif
    TimestampP_nativeFormat32_copy = TIMESTAMPP_NATIVEFORMAT32_INITIALIZER;

uint64_t TimestampP_getNative64()
{
    return AONRTCCurrent64BitValueGet();
}

uint32_t TimestampP_getNative32()
{
    return AONRTCCurrentCompareValueGet();
}
