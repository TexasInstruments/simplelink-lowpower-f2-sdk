/*
 * Copyright 2020-2022 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.TXT file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef SID_PAL_LOG_IFC_H
#define SID_PAL_LOG_IFC_H

/** @file
 *
 * @defgroup sid_pal_timers SID Timer API
 * @{
 *
 * @details Interface for timers for sidewalk SDK
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

typedef enum
{
    SID_PAL_LOG_SEVERITY_ERROR =    0,
    SID_PAL_LOG_SEVERITY_WARNING =  1,
    SID_PAL_LOG_SEVERITY_INFO =     2,
    SID_PAL_LOG_SEVERITY_DEBUG =    3
} sid_pal_log_severity_t;

#ifndef SID_PAL_LOG_LEVEL
#define SID_PAL_LOG_LEVEL  SID_PAL_LOG_SEVERITY_INFO
#endif

#ifndef SID_PAL_LOG_ENABLED
#define SID_PAL_LOG_ENABLED 1
#endif

/**
 * Printf style logging function
 *
 * @param[in]   serverity       Severity of the log
 * @param[in]   num_args        Number of arguments to be logged
 * @param[in]   fmt             Format string to print with variables
 */
void sid_pal_log(sid_pal_log_severity_t severity, uint32_t num_args, const char* fmt, ...);

/**
 * The function returns current logging level. Implemented in sid_log_control
 *
 */
sid_pal_log_severity_t sid_log_control_get_current_log_level();

/**
 * Flush log function
 */
void sid_pal_log_flush(void);

/**
 * String log pushing operation - in the case of JLink RTT logs since these are deferred,
 * they require that a special function be used in order to ensure that strings are
 * copied correctly in order for pushing then at a later time.
 *
 * For platforms that do not use deferred logging, this can remain unimplemented.
 */
char const *sid_pal_log_push_str(char *string);

/**
 * Describes the log buffer that is retrieved from sid_pal_log_ifc implementation
 */
struct sid_pal_log_buffer {
    /** Raw buffer that the sid_pal_log implementation will copy the log string
     * into
     **/
    uint8_t *buf;
    /** Size of the above Raw buffer, @sid_pal_log_get_log_buffer will replace
     * the size value with the actual size of the log string that was copied in
     * bytes
     */
    uint8_t size;
    /** Index of log string
     */
    uint8_t idx;
};

/**
 * Allows for retrival of log buffers from sid_pal_log implementation
 *
 * OPTIONAL If business logic wants to send logs over an external connection
 * they can use this api to pull out logs from sid_pal_log implementation if
 * implemented.
 * Internally the implementation can use a circular list of buffers to store
 * the logs as they come.
 *
 * @param[in] log_buffer Pointer to log buffer descriptor
 *
 * @return true if log strings were copied into log_buffer, false otherwise
 */
bool sid_pal_log_get_log_buffer(struct sid_pal_log_buffer *const log_buffer);

#define SID_PAL_VA_NARG(...) \
        (SID_PAL_VA_NARG_(_0, ## __VA_ARGS__, SID_PAL_RSEQ_N()))
#define SID_PAL_VA_NARG_(...) \
        SID_PAL_VA_ARG_N(__VA_ARGS__)
#define SID_PAL_VA_ARG_N( \
         _1, _2, _3, _4, _5, _6, _7, _8, _9,_10, \
        _11,_12,_13,_14,_15,_16,_17,_18,_19,_20, \
        _21,_22,_23,_24,_25,_26,_27,_28,_29,_30, \
        _31,_32,_33,_34,_35,_36,_37,_38,_39,_40, \
        _41,_42,_43,_44,_45,_46,_47,_48,_49,_50, \
        _51,_52,_53,_54,_55,_56,_57,_58,_59,_60, \
        _61,_62,_63,N,...) N
#define SID_PAL_RSEQ_N() \
        62, 61, 60,                             \
        59, 58, 57, 56, 55, 54, 53, 52, 51, 50, \
        49, 48, 47, 46, 45, 44, 43, 42, 41, 40, \
        39, 38, 37, 36, 35, 34, 33, 32, 31, 30, \
        29, 28, 27, 26, 25, 24, 23, 22, 21, 20, \
        19, 18, 17, 16, 15, 14, 13, 12, 11, 10, \
         9,  8,  7,  6,  5,  4,  3,  2,  1,  0

#if SID_PAL_LOG_ENABLED
#define SID_PAL_LOG_FLUSH()  sid_pal_log_flush()
#define SID_PAL_LOG_PUSH_STR(x)  sid_pal_log_push_str(x)

#define SID_PAL_LOG_HIGHEST_SEVIRITY(level, fmt_, ...) \
            sid_pal_log(level, SID_PAL_VA_NARG(__VA_ARGS__), fmt_, ##__VA_ARGS__);

#define SID_PAL_LOG(level, fmt_, ...)                                                           \
    do {                                                                                        \
        if (level <= sid_log_control_get_current_log_level())  {  \
            sid_pal_log(level, SID_PAL_VA_NARG(__VA_ARGS__), fmt_, ##__VA_ARGS__);              \
        }                                                                                       \
    } while(0)

#define SID_PAL_HEXDUMP_MAX           (8)
#define SID_PAL_HEXDUMP(level, data, len)                                     \
    do {                                                                      \
        if (level <= SID_PAL_LOG_LEVEL) {                                     \
            char const digit[16] = "0123456789ABCDEF";                        \
            uint8_t idx=0;                                                    \
            char hex_buf[SID_PAL_HEXDUMP_MAX*3 + 1] = {0};                    \
            for (int i=0; i<len; i++) {                                       \
                if (idx && ((i % SID_PAL_HEXDUMP_MAX) == 0)) {                \
                    SID_PAL_LOG(level, "%s", SID_PAL_LOG_PUSH_STR(hex_buf));  \
                    idx = 0;                                                  \
                }                                                             \
                hex_buf[idx++] = digit[(data[i] >> 4) & 0x0f];                \
                hex_buf[idx++] = digit[(data[i] >> 0) & 0x0f];                \
                hex_buf[idx++] = ' ';                                         \
                hex_buf[idx] = '\0';                                          \
            }                                                                 \
            if (idx) {                                                        \
                SID_PAL_LOG(level, "%s", SID_PAL_LOG_PUSH_STR(hex_buf));      \
            }                                                                 \
        }                                                                     \
    } while(0)

#else
#define SID_PAL_LOG_HIGHEST_SEVIRITY(level, fmt_, ...)
#define SID_PAL_LOG(level_, fmt_, ...)
#define SID_PAL_HEXDUMP(level_, data_, len_)
#define SID_PAL_LOG_FLUSH()
#define SID_PAL_LOG_PUSH_STR(x) (x)
#endif


/* Logging helpers to simplify logging APIs */
#define SID_PAL_LOG_ERROR(fmt_, ...)           SID_PAL_LOG_HIGHEST_SEVIRITY(SID_PAL_LOG_SEVERITY_ERROR, fmt_, ##__VA_ARGS__)
#if SID_PAL_LOG_LEVEL >= SID_PAL_LOG_SEVERITY_WARNING
#define SID_PAL_LOG_WARNING(fmt_, ...)         SID_PAL_LOG(SID_PAL_LOG_SEVERITY_WARNING, fmt_, ##__VA_ARGS__)
#if SID_PAL_LOG_LEVEL >= SID_PAL_LOG_SEVERITY_INFO
#define SID_PAL_LOG_INFO(fmt_, ...)            SID_PAL_LOG(SID_PAL_LOG_SEVERITY_INFO,    fmt_, ##__VA_ARGS__)
#if SID_PAL_LOG_LEVEL >= SID_PAL_LOG_SEVERITY_DEBUG
#define SID_PAL_LOG_DEBUG(fmt_, ...)           SID_PAL_LOG(SID_PAL_LOG_SEVERITY_DEBUG,   fmt_, ##__VA_ARGS__)
#endif /* WARNING */
#endif /* INFO */
#endif /* DEBUG */

#define SID_PAL_LOG_TRACE()                    SID_PAL_LOG_INFO("%s:%i %s() TRACE --", __FILENAME__, __LINE__, __FUNCTION__)

#ifdef __cplusplus
}
#endif

#endif
