/*
 * Copyright (c) 2024 Texas Instruments Incorporated - http://www.ti.com
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
 *  ======== ti_posix_freertos_time_defines.h ========
 *  FREERTOS time defines
 */

#ifndef ti_posix_freertos_time_defines__include
#define ti_posix_freertos_time_defines__include

#ifdef __cplusplus
extern "C" {
#endif

/*
 *  The maximum number of ticks before the tick count rolls over.  We use
 *  0xFFFFFFFF instead of 0x100000000 to avoid 64-bit math.
 */
#define FREERTOS_MAX_TICKS UINT32_MAX

/* The integral number of seconds in a period of FREERTOS_MAX_TICKS */
#define FREERTOS_MAX_SECONDS (FREERTOS_MAX_TICKS / configTICK_RATE_HZ)

/*
 *  FREERTOS_MAX_TICKS - FREERTOS_MAX_SECONDS is the number of ticks left over that
 *  don't make up a whole second.  We add 1 to get the remaining number
 *  of ticks when the tick count wraps back to 0.  REM_TICKS could
 *  theoritically be equivalent to 1 second (when the tick period divides
 *  0x100000000 evenly), so it is not really a "remainder", since it ranges
 *  from 1 to configTICK_RATE_HZ, instead of from 0 to configTICK_RATE_HZ - 1.
 *  However, this will not affect the seconds calculation in clock_gettime(),
 *  so we can ignore this special case.
 */
#define FREERTOS_REM_TICKS ((FREERTOS_MAX_TICKS - FREERTOS_MAX_SECONDS) + 1)

/* number of microseconds per tick */
#define FREERTOS_TICK_PERIOD_USECS (1000000 / configTICK_RATE_HZ)

/* number of seconds from 1900 (TI Epoch) to 1970 (POSIX Epoch) */
#define TI_EPOCH_OFFSET 2208988800

#define NSEC_PER_SEC 1000000000
#ifdef __cplusplus
}
#endif

#endif /* ti_posix_freertos_time_defines__include */