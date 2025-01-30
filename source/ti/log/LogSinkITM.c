/*
 * Copyright (c) 2022-2024, Texas Instruments Incorporated - http://www.ti.com
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
 *  ======== LogSinkITM.c ========
 */

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>

#include <ti/log/Log.h>
#include <ti/log/LogSinkITM.h>

#include <ti/drivers/dpl/TimestampP.h>
#include <ti/drivers/dpl/HwiP.h>
#include <ti/drivers/ITM.h>

#define LogSinkITM_RESET_FRAME (0xBBBBBBBB)

/*
 *  ======== LogSinkITM_sendTimeSync ========
 */
void LogSinkITM_sendTimeSync(void)
{
    uint64_t ts = TimestampP_getNative64();
    ITM_send32Polling(LogSinkITM_STIM_TIME_SYNC, ts & 0xffffffff);
    ITM_send32Polling(LogSinkITM_STIM_TIME_SYNC, ts >> 32);
}

/*
 *  ======== LogSinkITM_init ========
 */
void LogSinkITM_init(void)
{
    /* disable interrupts */
    uint32_t key   = HwiP_disable();
    /* Setup and enable ITM driver */
    bool itmOpened = ITM_open();

    if (itmOpened)
    {
        /* Send the reset sequence */
        ITM_send32Atomic(LogSinkITM_STIM_INFO, LogSinkITM_RESET_FRAME);
        /* Enable generation of time stamps based on system CPU */
        ITM_enableTimestamps(ITM_TS_DIV_16, false);

        /* Send information about timer resync */
        ITM_send16Polling(LogSinkITM_STIM_INFO, LogSinkITM_Info_Timing | ITM_TS_DIV_16 << 8);
        ITM_send32Polling(LogSinkITM_STIM_INFO, TimestampP_nativeFormat64.value);

        /* Initial timestamp sync  */
        LogSinkITM_sendTimeSync();
    }

    /* enable interrupts */
    HwiP_restore(key);
}

/*
 *  ======== LogSinkITM_printf ========
 */
void LogSinkITM_printf(const Log_Module *handle, uint32_t header, uint32_t headerPtr, uint32_t numArgs, va_list argptr)
{
    uint32_t key;

    /* disable interrupts */
    key = HwiP_disable();

    /* Send header */
    ITM_send32Polling(LogSinkITM_STIM_HEADER, headerPtr);

    uint32_t i;
    for (i = 0; i < numArgs; ++i)
    {
        uintptr_t arg = va_arg(argptr, uintptr_t);
        ITM_send32Polling(LogSinkITM_STIM_TRACE, arg);
    }

    /* enable interrupts */
    HwiP_restore(key);
}

/*
 *  ======== LogSinkITM_printfSingleton0 ========
 */
void LogSinkITM_printfSingleton0(const Log_Module *handle, uint32_t header, uint32_t headerPtr, ...)
{
    va_list argptr;

    va_start(argptr, headerPtr);
    LogSinkITM_printf(handle, header, headerPtr, 0, argptr);
    va_end(argptr);
}

/*
 *  ======== LogSinkITM_printfSingleton1 ========
 */
void LogSinkITM_printfSingleton1(const Log_Module *handle, uint32_t header, uint32_t headerPtr, ...)
{
    va_list argptr;

    va_start(argptr, headerPtr);
    LogSinkITM_printf(handle, header, headerPtr, 1, argptr);
    va_end(argptr);
}

/*
 *  ======== LogSinkITM_printfSingleton2 ========
 */
void LogSinkITM_printfSingleton2(const Log_Module *handle, uint32_t header, uint32_t headerPtr, ...)
{
    va_list argptr;

    va_start(argptr, headerPtr);
    LogSinkITM_printf(handle, header, headerPtr, 2, argptr);
    va_end(argptr);
}

/*
 *  ======== LogSinkITM_printfSingleton1 ========
 */
void LogSinkITM_printfSingleton3(const Log_Module *handle, uint32_t header, uint32_t headerPtr, ...)
{
    va_list argptr;

    va_start(argptr, headerPtr);
    LogSinkITM_printf(handle, header, headerPtr, 3, argptr);
    va_end(argptr);
}

/*
 *  ======== LogSinkITM_printfSingleton ========
 */
void LogSinkITM_printfSingleton(const Log_Module *handle, uint32_t header, uint32_t headerPtr, uint32_t numArgs, ...)
{
    va_list argptr;

    va_start(argptr, numArgs);
    LogSinkITM_printf(handle, header, headerPtr, numArgs, argptr);
    va_end(argptr);
}

/*
 *  ======== LogSinkITM_buf ========
 */
void LogSinkITM_bufSingleton(const Log_Module *handle, uint32_t header, uint32_t headerPtr, uint8_t *data, size_t size)
{
    uint32_t key;

    /* disable interrupts */
    key = HwiP_disable();

    /* Send header */
    ITM_send32Polling(LogSinkITM_STIM_HEADER, headerPtr);
    /* We always send the size of the expected buffer */
    ITM_send32Polling(LogSinkITM_STIM_TRACE, size);
    /* Send out the actual data */
    ITM_sendBufferAtomic(LogSinkITM_STIM_TRACE, (const char *)data, size);

    /* enable interrupts */
    HwiP_restore(key);
}

/*
 *  ======== LogSinkITM_finalize ========
 */
void LogSinkITM_finalize(void)
{
    /* disable interrupts */
    uint32_t key = HwiP_disable();

    ITM_close();

    /* enable interrupts */
    HwiP_restore(key);
}
