/*
 * Copyright 2018-2020 Amazon.com, Inc. or its affiliates. All rights reserved.
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
#ifndef _ACE_LOG_H
#define _ACE_LOG_H

#include <stdarg.h>
#include <ace/ace_status.h>
#include <ace/uthash.h>
#include <ace/ace_config.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Log buffer definitions.
 * @ingroup ACE_LOG_API_DEFINE
 *
 * These are the possible log buffers available to ACE.
 * At a minimum, the platform must support one log buffer and
 * in such a case, messages sent to other buffers should be
 * recorded in that same buffer. If the platform supports multiple
 * buffers but a smaller set than defined by ACE, the messages
 * will be routed to the most appropriate buffer. For example,
 * system related messages will be recorded in the system buffer and
 * all other messages will be recorded in the main buffer.
 */
typedef enum {
    ACE_LOG_ID_MIN = 0,
    /** MAIN log buffer, this buffer will always be available */
    ACE_LOG_ID_MAIN = ACE_LOG_ID_MIN,
    /** Radio log buffer, this buffer is optional.
     * If it doesn't exist, logs will be stored in the MAIN buffer */
    ACE_LOG_ID_RADIO = 1,
    /** SYSTEM log buffer, this buffer is optional.
     * If it doesn't exist, logs will be stored in the MAIN buffer */
    ACE_LOG_ID_SYSTEM = 2,
    /** CRASH log buffer, this buffer is optional.
     * If it doesn't exist, logs will be stored in the MAIN buffer */
    ACE_LOG_ID_CRASH = 3,
    /** DM main log, this is used by device maker to record and upload private
       logs */
    ACE_LOG_ID_DM_MAIN = 4,
    ACE_LOG_ID_MAX
} aceLog_buffer_t;

/**
 * @brief Log level definitions.
 * @ingroup ACE_LOG_API_DEFINE
 *
 * Different log levels can be leveraged to filter log output in various
 * scenarios. For example, for development all messages including VERBOSE
 * messages should be logged, while in a production environment only
 * WARN messages and above should be logged.
 */
typedef enum {
    ACE_LOG_MIN = 0,
    /** Log message is a verbose message. */
    ACE_LOG_VERBOSE = ACE_LOG_MIN,
    /** Log message is a debug message. */
    ACE_LOG_DEBUG = 1,
    /** Log message is a info message. */
    ACE_LOG_INFO = 2,
    /** Log message is a warning message. */
    ACE_LOG_WARN = 3,
    /** Log message is a error message. */
    ACE_LOG_ERROR = 4,
    /** Log message is a fatal message. */
    ACE_LOG_FATAL = 5,
    ACE_LOG_MAX = ACE_LOG_FATAL
} aceLog_level_t;

/**
 * @brief Log levels for compile flag ACE_LOG_MIN_LVL
 * @ingroup ACE_LOG_API_DEFINE
 *
 * Define ACE_LOG_MIN_LVL to the minimal level of logs to be compiled
 * ACE_LOG_MIN_LVL is set to 0 by default if not set
 *
 * Log levels below the ACE_LOG_MIN_LVL will not be compiled
 *
 * ACE log levels for ACE_LOG_MIN_LVL are defined as follows:
 *     0 - Verbose
 *     1 - Debug
 *     2 - Info
 *     3 - Warning
 *     4 - Error
 *     5 - Fatal
 *
 */
#ifndef ACE_LOG_MIN_LVL
#ifdef ACE_LOG_CONFIG_MIN_LVL
#define ACE_LOG_MIN_LVL ACE_LOG_CONFIG_MIN_LVL
#else
#define ACE_LOG_MIN_LVL 0
#endif
#endif

/**
 * @brief Used to store a tag and its filtered level in a hashtable for log
 *        filtering.
 * @ingroup ACE_LOG_API_DEFINE
 *
 * Used to define log level filtering.
 */
typedef struct aceLog_filter_struct {
    const char* tag;
    int filter_level;
    UT_hash_handle hh;
} aceLog_filter_t;

/**
 * @brief Write a VERBOSE log message to target buffer.
 * @ingroup ACE_LOG_API_DEFINE
 *
 * @param[in] log_id Which log buffer to write to.
 * @param[in] tag    Log tag string.
 * @param[in] fmt    Log message
 * @param[in] ...    Arguments for format specification.
 */
#ifndef ACE_LOGV
#if ACE_LOG_MIN_LVL <= 0
#define ACE_LOGV(log_id, tag, fmt, ...) \
    ace_log(log_id, ACE_LOG_VERBOSE, tag, fmt, ##__VA_ARGS__)
#else
#define ACE_LOGV(log_id, tag, fmt, ...)
#endif
#endif

/**
 * @brief Write a DEBUG log message to target buffer.
 * @ingroup ACE_LOG_API_DEFINE
 *
 * @param[in] log_id  Which log buffer to write to.
 * @param[in] tag     Log tag string.
 * @param[in] fmt     Log message
 * @param[in] ...     Arguments for format specification.
 */
#ifndef ACE_LOGD
#if ACE_LOG_MIN_LVL <= 1
#define ACE_LOGD(log_id, tag, fmt, ...) \
    ace_log(log_id, ACE_LOG_DEBUG, tag, fmt, ##__VA_ARGS__)
#else
#define ACE_LOGD(log_id, tag, fmt, ...)
#endif
#endif

/**
 * @brief Write an INFO log message to target buffer.
 * @ingroup ACE_LOG_API_DEFINE
 *
 * @param[in] log_id Which log buffer to write to.
 * @param[in] tag    Log tag string.
 * @param[in] fmt    Log message
 * @param[in] ...    Arguments for format specification.
 */
#ifndef ACE_LOGI
#if ACE_LOG_MIN_LVL <= 2
#define ACE_LOGI(log_id, tag, fmt, ...) \
    ace_log(log_id, ACE_LOG_INFO, tag, fmt, ##__VA_ARGS__)
#else
#define ACE_LOGI(log_id, tag, fmt, ...)
#endif
#endif

/**
 * @brief Write a WARNING log message to target buffer.
 * @ingroup ACE_LOG_API_DEFINE
 *
 * @param[in] log_id Which log buffer to write to.
 * @param[in] tag    Log tag string.
 * @param[in] fmt    Log message
 * @param[in] ...    Arguments for format specification.
 */
#ifndef ACE_LOGW
#if ACE_LOG_MIN_LVL <= 3
#define ACE_LOGW(log_id, tag, fmt, ...) \
    ace_log(log_id, ACE_LOG_WARN, tag, fmt, ##__VA_ARGS__)
#else
#define ACE_LOGW(log_id, tag, fmt, ...)
#endif
#endif

/**
 * @brief Write an ERROR log message to target buffer.
 * @ingroup ACE_LOG_API_DEFINE
 *
 * @param[in] log_id Which log buffer to write to.
 * @param[in] tag    Log tag string.
 * @param[in] fmt    Log message
 * @param[in] ...    Arguments for format specification.
 */
#ifndef ACE_LOGE
#if ACE_LOG_MIN_LVL <= 4
#define ACE_LOGE(log_id, tag, fmt, ...) \
    ace_log(log_id, ACE_LOG_ERROR, tag, fmt, ##__VA_ARGS__)
#else
#define ACE_LOGE(log_id, tag, fmt, ...)
#endif
#endif

/**
 * @brief Write a FATAL log message to target buffer.
 * @ingroup ACE_LOG_API_DEFINE
 *
 * @param[in] log_id Which log buffer to write to.
 * @param[in] tag    Log tag string.
 * @param[in] fmt    Log message
 * @param[in] ...    Arguments for format specification.
 */
#ifndef ACE_LOGF
#if ACE_LOG_MIN_LVL <= 5
#define ACE_LOGF(log_id, tag, fmt, ...) \
    ace_log(log_id, ACE_LOG_FATAL, tag, fmt, ##__VA_ARGS__)
#else
#define ACE_LOGF(log_id, tag, fmt, ...)
#endif
#endif

/**
 * @brief Write a log message to target buffer.
 * @ingroup ACE_LOG_API_DEFINE
 *
 * @param[in] log_id   Which log buffer to write to.
 * @param[in] priority priority such as ACE_LOG_VERBOSE or ACE_LOG_ERROR
 * @param[in] tag      Log tag string.
 * @param[in] fmt      Log message
 * @param[in] ...      Arguments for format specification.
 */
#ifndef ACE_LOG_BASE
#define ACE_LOG_BASE(log_id, priority, tag, fmt, ...) \
    ace_log(log_id, priority, tag, fmt, ##__VA_ARGS__)
#endif

/**
 * @brief Register a filter for a certain log tag. Any subsequent log message
 * using the specified tag and a priority less than the one specified in the
 * filter will not be saved to the log.
 * @ingroup ACE_LOG_API
 *
 * @param[in] filter_tag    log tag the filter applies to.
 * @param[in] priority      log level for the filter.
 * @param[out] filter       Pointer to memory where the filter is initialized.
 * @return ACE_STATUS_SUCCESS if successful and an error code upon failure
 * @retval ACE_STATUS_NULL_POINTER if filtertag is NULL
 * @retval ACE_STATUS_UNINITIALIZED if mutex is not initialized
 * @retval ACE_STATUS_BUSY if unable to grab mutex
 * @retval ACE_STATUS_NOT_IMPLEMENTED if not implemented for this platform
 */
ace_status_t aceLog_registerFilter(const char* filter_tag,
                                     aceLog_level_t priority,
                                     aceLog_filter_t* filter);

/**
 * @brief Un-register a filter for a certain log tag.
 * @ingroup ACE_LOG_API
 *
 * @param[in] filter_tag     log tag the filter applies to.
 * @return ACE_STATUS_SUCCESS if success or error code upon failure
 * @retval ACE_STATUS_NULL_POINTER if filtertag is NULL
 * @retval ACE_STATUS_UNINITIALIZED if mutex is not initialized
 * @retval ACE_STATUS_BUSY if unable to grab mutex
 * @retval ACE_STATUS_NOT_IMPLEMENTED if not implemented for this platform
 */
ace_status_t aceLog_unregisterFilter(const char* filter_tag);

/* Generally for internal use; prefer helper macros above. */
void aceLog_log(aceLog_buffer_t log_id, aceLog_level_t priority, const char* tag,
             const char* fmt, ...);
void aceLog_va(aceLog_buffer_t log_id, aceLog_level_t priority, const char* tag,
                const char* fmt, va_list ap);

/**
 * @param[in] log_id
 * @param[in] priority
 * @param[in] tag
 * @param[in] str
 */
void aceLog_str(aceLog_buffer_t log_id, aceLog_level_t priority, const char* tag,
                 const char* str);

/**
 *
 * @brief Internal log API for writing binary logs (only used by metrics)
 *
 * @param[in] buf       Data buffer holding data to be written to log
 * @param[len] len      @ATT_META{size: len} length of data to write
 * @return int          status, 0 = OK, < 0 = Fail
 */
int aceLog_writeBin(const void* buf, unsigned int len);

/**
 * @brief Internal log API for reading binary logs (only used by metrics)
 *
 * @param[out] dst      @ATT_META{size: len, return_is_size} Pointer to
 *                      allocated data buffer where the log data should be
 *                      stored.
 * @param[in] len       Total size available in buffer
 * @return int          Amount of data written to buffer or ace_status_t if
 *                      failure
 */
int aceLog_readBin(void* dst, unsigned int len);


//========================== Deprecated APIs/Types ============================

typedef aceLog_level_t ace_loglevel;
typedef aceLog_buffer_t ace_logbuffer;
typedef aceLog_filter_t aceLogFilter_t;

/**
 * @deprecated Use aceLog_registerFilter()
 */
ace_status_t ace_log_register_filter(const char* filter_tag,
                                     ace_loglevel priority,
                                     aceLogFilter_t* filter);

/**
 * @deprecated Use aceLog_unregisterFilter()
 */
ace_status_t ace_log_unregister_filter(const char* filter_tag);

/* Generally for internal use; prefer helper macros above. */
void ace_log(ace_logbuffer log_id, ace_loglevel priority, const char* tag,
             const char* fmt, ...);
void ace_log_va(ace_logbuffer log_id, ace_loglevel priority, const char* tag,
                const char* fmt, va_list ap);

/**
 * @deprecated Use aceLog_logStr()
 */
void ace_log_str(ace_logbuffer log_id, ace_loglevel priority, const char* tag,
                 const char* str);

/**
 * @deprecated Use aceLog_writeBin()
 */
int ace_log_write_bin(const void* buf, unsigned int len);

/**
 * @deprecated Use aceLog_readBin()
 */
int ace_log_read_bin(void* dst, unsigned int len);


#ifdef __cplusplus
}
#endif

#endif /* _ACE_LOG_H */
