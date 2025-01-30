/*
 * Copyright (c) 2019-2024, Texas Instruments Incorporated - http://www.ti.com
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
 *  ======== LogSinkBuf.c ========
 */

#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <ti/drivers/dpl/HwiP.h>
#include <ti/drivers/dpl/TimestampP.h>

#include <ti/log/LogSinkBuf.h>

#define LogSinkBuf_FULL     -1
#define LogSinkBuf_MAX_ARGS (LogSinkBuf_WORDS_PER_RECORD - 1)

/* Global LogSinkBuf instance reference for use with singleton implementations
 * of printf.
 */
Log_SINK_BUF_USE(CONFIG_ti_log_LogSinkBuf_0);

static void findNextRecord(LogSinkBuf_Handle inst, LogSinkBuf_Rec **rec)
{
    /* compute next available record */
    *rec = inst->curEntry;
    if (*rec == inst->endEntry)
    {
        if (inst->advance == LogSinkBuf_Type_CIRCULAR)
        {
            inst->curEntry = inst->buffer;
        }
        else
        {
            inst->advance = LogSinkBuf_FULL;
        }
    }
    else
    {
        inst->curEntry = (LogSinkBuf_Rec *)((char *)*rec + sizeof(LogSinkBuf_Rec));
    }
}

/*
 *  ======== LogSinkBuf_printf ========
 */
void LogSinkBuf_printf(LogSinkBuf_Handle inst, uint32_t header, uint32_t index, uint32_t numArgs, va_list argptr)
{
    uintptr_t key;
    uint32_t serial;
    LogSinkBuf_Rec *rec;
    uint32_t argsToCopy = numArgs;

    /* disable interrupts */
    key = HwiP_disable();

    /* increment serial even when full */
    serial = ++(inst->serial);

    if (inst->advance == LogSinkBuf_FULL)
    {
        HwiP_restore(key);
        return;
    }

    /* compute next available record */
    findNextRecord(inst, &rec);

    rec->timestampLow = TimestampP_getNative32();

    /* enable interrupts */
    HwiP_restore(key);

    /* write data to record */
    rec->serial = serial;
    rec->type   = LogSinkBuf_PRINTF;

    rec->data[0] = header;

    uint32_t i;
    for (i = 0; i < argsToCopy; i++)
    {
        rec->data[1 + i] = va_arg(argptr, uintptr_t);
    }

    return;
}

/*
 *  ======== LogSinkBuf_printfDepInjection ========
 *
 *  This printf implementation is for use when multiple LogSinkBuf_Instance
 *  instances should be supported within an application.
 *
 *  The implementation will read out the LogSinkBuf_Instance address at runtime
 *  from the handle argument.
 */
void LogSinkBuf_printfDepInjection(const Log_Module *handle, uint32_t header, uint32_t index, uint32_t numArgs, ...)
{
    va_list argptr;

    /* Guard against more arguments being passed in than supported */
    if (numArgs > LogSinkBuf_MAX_ARGS)
    {
        numArgs = LogSinkBuf_MAX_ARGS;
    }

    LogSinkBuf_Handle inst = (LogSinkBuf_Handle)handle->sinkConfig;

    /* Get the VA args pointer in the initial wrapper since you cannot pass VA
     * args to further functions using elipses (...) syntax.
     *
     * All va_start() does is get us the pointer to the first VA arg on the
     * stack. That value will still be valid when passed on further.
     */
    va_start(argptr, numArgs);

    LogSinkBuf_printf(inst, header, index, numArgs, argptr);

    va_end(argptr);
}

/*
 *  ======== LogSinkBuf_printfDepInjection0 ========
 */
void LogSinkBuf_printfDepInjection0(const Log_Module *handle, uint32_t header, uint32_t index, ...)
{
    va_list argptr;

    va_start(argptr, index);
    LogSinkBuf_printf((LogSinkBuf_Handle)handle->sinkConfig, header, index, 0, argptr);
    va_end(argptr);
}

/*
 *  ======== LogSinkBuf_printfDepInjection1 ========
 */
void LogSinkBuf_printfDepInjection1(const Log_Module *handle, uint32_t header, uint32_t index, ...)
{
    va_list argptr;

    va_start(argptr, index);
    LogSinkBuf_printf((LogSinkBuf_Handle)handle->sinkConfig, header, index, 1, argptr);
    va_end(argptr);
}

/*
 *  ======== LogSinkBuf_printfDepInjection2 ========
 */
void LogSinkBuf_printfDepInjection2(const Log_Module *handle, uint32_t header, uint32_t index, ...)
{
    va_list argptr;

    va_start(argptr, index);
    LogSinkBuf_printf((LogSinkBuf_Handle)handle->sinkConfig, header, index, 2, argptr);
    va_end(argptr);
}

/*
 *  ======== LogSinkBuf_printfDepInjection3 ========
 */
void LogSinkBuf_printfDepInjection3(const Log_Module *handle, uint32_t header, uint32_t index, ...)
{
    va_list argptr;

    va_start(argptr, index);
    LogSinkBuf_printf((LogSinkBuf_Handle)handle->sinkConfig, header, index, 3, argptr);
    va_end(argptr);
}

/*
 *  ======== LogSinkBuf_printfSingleton ========
 *
 *  This implementation purposefully does not use the handle argument but
 *  instead references a single, global LogSinkBuf_Instance structure. When LTO
 *  is enabled, this allows the compiler to avoid loading the handle at the
 *  call site, saving on flash.
 *
 *  Use of this printf implementation has the limitation that only one
 *  LogSinkBuf instance may be used and it will be assigned the
 *  LogsinkBuf_Instance name LogSinkBuf_CONFIG_ti_log_LogSinkBuf_0_config
 */
void LogSinkBuf_printfSingleton(const Log_Module *handle, uint32_t header, uint32_t index, uint32_t numArgs, ...)
{
    va_list argptr;

    /* Guard against more arguments being passed in than supported */
    if (numArgs > LogSinkBuf_MAX_ARGS)
    {
        numArgs = LogSinkBuf_MAX_ARGS;
    }

    /* Get the VA args pointer in the initial wrapper since you cannot pass VA
     * args to further functions using elipses (...) syntax.
     *
     * All va_start() does is get us the pointer to the first VA arg on the
     * stack. That value will still be valid when passed on further.
     */
    va_start(argptr, numArgs);
    LogSinkBuf_printf(&LogSinkBuf_CONFIG_ti_log_LogSinkBuf_0_config, header, index, numArgs, argptr);
    va_end(argptr);
}

/*
 *  ======== LogSinkBuf_printfSingleton0 ========
 */
void LogSinkBuf_printfSingleton0(const Log_Module *handle, uint32_t header, uint32_t index, ...)
{
    va_list argptr;

    va_start(argptr, index);
    LogSinkBuf_printf(&LogSinkBuf_CONFIG_ti_log_LogSinkBuf_0_config, header, index, 0, argptr);
    va_end(argptr);
}

/*
 *  ======== LogSinkBuf_printfSingleton1 ========
 */
void LogSinkBuf_printfSingleton1(const Log_Module *handle, uint32_t header, uint32_t index, ...)
{
    va_list argptr;

    va_start(argptr, index);
    LogSinkBuf_printf(&LogSinkBuf_CONFIG_ti_log_LogSinkBuf_0_config, header, index, 1, argptr);
    va_end(argptr);
}

/*
 *  ======== LogSinkBuf_printfSingleton2 ========
 */
void LogSinkBuf_printfSingleton2(const Log_Module *handle, uint32_t header, uint32_t index, ...)
{
    va_list argptr;

    va_start(argptr, index);
    LogSinkBuf_printf(&LogSinkBuf_CONFIG_ti_log_LogSinkBuf_0_config, header, index, 2, argptr);
    va_end(argptr);
}

/*
 *  ======== LogSinkBuf_printfSingleton3 ========
 */
void LogSinkBuf_printfSingleton3(const Log_Module *handle, uint32_t header, uint32_t index, ...)
{
    va_list argptr;

    va_start(argptr, index);
    LogSinkBuf_printf(&LogSinkBuf_CONFIG_ti_log_LogSinkBuf_0_config, header, index, 3, argptr);
    va_end(argptr);
}

/*
 *  ======== LogSinkBuf_buf ========
 */
void LogSinkBuf_bufDepInjection(const Log_Module *handle, uint32_t header, uint32_t index, uint8_t *data, size_t size)
{
    uintptr_t key;
    uint32_t serial;
    LogSinkBuf_Rec *rec;
    uint32_t numRecords = ((size) / LogSinkBuf_SIZEOF_RECORD) + ((size % LogSinkBuf_SIZEOF_RECORD) != 0);
    numRecords += 1;
    uint32_t i;

    if (handle == NULL)
    {
        return;
    }

    LogSinkBuf_Handle inst = (LogSinkBuf_Handle)handle->sinkConfig;

    /* disable interrupts */
    key = HwiP_disable();

    /* Here we acquire records as contiguous memory.
     * This approach leads to long critical sections for large buffers
     * and can be improved in the future once all hosts support fragmentation
     * of packets
     */
    for (i = 0; i < numRecords; i++)
    {
        /* increment serial even when full */
        serial = ++(inst->serial);

        if (inst->advance == LogSinkBuf_FULL)
        {
            HwiP_restore(key);
            return;
        }

        /* compute next available record */
        findNextRecord(inst, &rec);

        rec->timestampLow = TimestampP_getNative32();

        /* write data to record */
        rec->serial = serial;
        if (i == 0)
        {
            rec->type    = LogSinkBuf_BUFFER_START;
            rec->data[0] = header;
            rec->data[1] = size;
        }
        else
        {
            rec->type = LogSinkBuf_BUFFER_CONTINUED;
            memcpy(rec->data, &data[(i - 1) * LogSinkBuf_SIZEOF_RECORD], LogSinkBuf_SIZEOF_RECORD);
        }
    }

    /* enable interrupts */
    HwiP_restore(key);
}
