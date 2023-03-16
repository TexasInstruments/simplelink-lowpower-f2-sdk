/*
 * Copyright (c) 2019-2023 Texas Instruments Incorporated - http://www.ti.com
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
 *  ======== LogSinkBuf.h ========
 */

#ifndef ti_loggers_utils_LogSinkBuf__include
#define ti_loggers_utils_LogSinkBuf__include

#include <ti/log/Log.h>
#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif

#define Log_TI_LOG_SINK_BUF_VERSION 0.1.0

#define LogSinkBuf_Type_LINEAR      (1)
#define LogSinkBuf_Type_CIRCULAR    (2)
#define LogSinkBuf_WORDS_PER_RECORD (5)
#define LogSinkBuf_BUF_HEADER_SIZE  sizeof(LogSinkBuf_RecordType) + sizeof(uint32_t)
#define LogSinkBuf_SIZEOF_RECORD    (sizeof(uint32_t) * LogSinkBuf_WORDS_PER_RECORD)

typedef enum
{
    LogSinkBuf_EVENT            = 0,
    LogSinkBuf_PRINTF           = 1,
    LogSinkBuf_BUFFER_START     = 2,
    LogSinkBuf_BUFFER_CONTINUED = 3
} LogSinkBuf_RecordType;

/*
 *  ======== LogSinkBuf_Rec ========
 */
typedef struct LogSinkBuf_Rec
{
    uint32_t serial;
    uint32_t timestampHigh; /* Upper 32 bits of timestamp */
    uint32_t timestampLow;  /* Lower 32 bits of timestamp */
    LogSinkBuf_RecordType type;
    uint32_t data[LogSinkBuf_WORDS_PER_RECORD];
} LogSinkBuf_Rec;

/*
 *  ======== LogSinkBuf_Instance ========
 */
typedef struct LogSinkBuf_Instance
{
    uint8_t bufType;
    int8_t advance;
    uint16_t numEntries;
    uint32_t serial;
    LogSinkBuf_Rec *buffer;
    LogSinkBuf_Rec *curEntry; /* next record to write */
    LogSinkBuf_Rec *endEntry;
} LogSinkBuf_Instance;

/*
 *  ======== LogSinkBuf_Handle ========
 */
typedef LogSinkBuf_Instance *LogSinkBuf_Handle;

/*
 *  ======== LogSinkBuf_printf ========
 */
extern void ti_log_LogSinkBuf_printf(const Log_Module *handle, uint32_t header, uint32_t index, uint32_t numArgs, ...);

/*
 *  ======== LogSinkBuf_buf ========
 */
extern void ti_log_LogSinkBuf_buf(const Log_Module *handle,
                                  uint32_t header,
                                  uint32_t index,
                                  uint8_t *data,
                                  size_t size);

/*
 * Helpers to define/use instance. ITM is a singleton so no arguments are taken.
 */
#define Log_SINK_BUF_DEFINE(name, type, num_entries)                                            \
    static LogSinkBuf_Rec logSinkBuf_##name##_buffer[num_entries];                              \
    LogSinkBuf_Instance LogSinkBuf_##name##_Config = {.serial     = 0,                          \
                                                      .bufType    = type,                       \
                                                      .advance    = type,                       \
                                                      .numEntries = num_entries,                \
                                                      .buffer     = logSinkBuf_##name##_buffer, \
                                                      .curEntry   = logSinkBuf_##name##_buffer, \
                                                      .endEntry   = logSinkBuf_##name##_buffer + (num_entries - 1)}
#define Log_SINK_BUF_USE(name) extern LogSinkBuf_Instance LogSinkBuf_##name##_Config
#define Log_MODULE_INIT_SINK_BUF(name, _levels)                                                                      \
    {                                                                                                                \
        .sinkConfig = &LogSinkBuf_##name##_Config, .printf = ti_log_LogSinkBuf_printf, .buf = ti_log_LogSinkBuf_buf, \
        .levels = _levels,                                                                                           \
    }

_Log_DEFINE_LOG_VERSION(LogSinkBuf, Log_TI_LOG_SINK_BUF_VERSION);

#if defined(__cplusplus)
}
#endif

#endif /* ti_loggers_utils_LogSinkBuf__include */
