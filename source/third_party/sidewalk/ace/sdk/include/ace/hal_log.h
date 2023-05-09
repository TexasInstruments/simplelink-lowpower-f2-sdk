/*
 * Copyright 2019-2020 Amazon.com, Inc. or its affiliates. All rights reserved.
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

/**
 * @file hal_log.h
 * @brief ACE Log Hal defines the required API implmentations from the platform
 * to enable the ACE logging capability.
 * Definition of the terms:
 * *log buffer: a repository where logs with common log buffer ID are collected.
 * *log message: one text or binary log in the raw format without metadata.
 * *log entry: a log with meta datas such as timestamp, uid, tid.
 * @addtogroup ACE_HAL_LOG
 * @{
 */

#ifndef ACE_HAL_LOG_H_
#define ACE_HAL_LOG_H_

#include <stdint.h>
#include <stdlib.h>
#include <ace/ace_status.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup ACE_HAL_LOG_DS Data Structures and Enum
 * @{
 * @ingroup ACE_HAL_LOG
 */

/**
 * @brief LOGHAL API Version.
 */
#define ACE_LOGHAL_API_VERSION 1

/**
 * @brief Log buffer ID.
 * It's up to the platform on how to map buffer ID to its implementation.
 * They can be mapped to one buffer or file, or individual buffers or files.
 * On platforms that have sufficient resources, the implementation can map each
 * log buffer ID to an individual log buffer or file. On platforms that have
 * limited resources, the implementation can map multiple buffer IDs to one or a
 * few buffers or files.
 */
typedef enum {
    ACE_LOGHAL_ID_MAIN = 0, /**< For apps and middlewares. */
    ACE_LOGHAL_ID_SYSTEM,   /**< For middleware, dpk and platform modules. */
    ACE_LOGHAL_ID_CRASH,    /**< For crash log from the platform or run time
                             * systems. */
    ACE_LOGHAL_ID_DM_MAIN,  /**< For device manufacture modules and apps. */
    ACE_LOGHAL_ID_BIN,      /**< For binary logs. The support for the binary
                             * log is optional. */
    ACE_LOGHAL_ID_REMOTE,   /**< Reserved for remote devices. */
    ACE_LOGHAL_ID_MAX       /**< Number of the logs. */
} aceLogHal_id_t;

/**
 * @brief Log severity levels.
 */
typedef enum {
    /** Even more detailed informational messages than the #ACE_LOGHAL_DEBUG. */
    ACE_LOGHAL_VERBOSE = 0,
    /** Detailed informational messages that's useful for debugging an
       application. */
    ACE_LOGHAL_DEBUG,
    /** Informational messages that highlight the progress of the application.
     */
    ACE_LOGHAL_INFO,
    /** Recoverable conditions that put the application in an undesirable state.
     */
    ACE_LOGHAL_WARN,
    /** Unrecoverable error conditions that still allow the application to
       continue. */
    ACE_LOGHAL_ERROR,
    /** Severe error condition that will likely lead the application to abort.
     */
    ACE_LOGHAL_FATAL
} aceLogHal_level_t;

/**
 * @brief Modes for aceLogHal_read().
 */
typedef enum {
    /** Read one log message or entry. Support for this is required
     * if aceLogHal_read() is supported */
    ACE_LOGHAL_READ_SINGLE = 1U << 0U,
    /** Fill the read buffer with as many complete log entries as the
     * buffer will allow. Support for this is optional. */
    ACE_LOGHAL_READ_FILL = 1U << 1U
} aceLogHal_readMode_t;

/**
 * @brief Properties of the platform implementation.
 * These are used as keys to the aceLogHal_getPlatformProperty() API.
 */
typedef enum {
    /** API version. Implementation should return #ACE_LOGHAL_API_VERSION */
    ACE_LOGHAL_PROPERTY_API_VER = 0,
    /** Max size of the binary log message. Returns 0 for no limit. */
    ACE_LOGHAL_PROPERTY_LOG_BIN_MSG_MAX_LEN,
    /** Max size of the log message. Log messages larger than this may be
     * truncated. This value will be used by the ace_log module to allocate
     * memory for the log from the caller's stack. */
    ACE_LOGHAL_PROPERTY_LOG_TXT_MSG_MAX_LEN,
    /** Max size of the formatted log entry. This value can be used by the
     * application to allocate memory to read log entries. */
    ACE_LOGHAL_PROPERTY_LOG_TXT_ENTRY_MAX_LEN,
    /** Bitmask of the supported ::aceLogHal_readMode_t. */
    ACE_LOGHAL_PROPERTY_SUPPORTED_READ_MODES
} aceLogHal_platformProperty_t;

/**
 * @brief Log operation and attribute flags.
 */
typedef enum {
    /** Indicate data or operation is text */
    ACE_LOGHAL_TYPE_TXT = 0,
    /** Indicate data or operation is binary. The support for the binary
     * read/write is optional. */
    ACE_LOGHAL_TYPE_BIN = 1
} aceLogHal_flags_t;

/**
 * @brief Log file handler for the aceLogHal_read() API.
 */
typedef void* aceLogHal_readHandle_t;

/** @} */

/**
 * @defgroup ACE_HAL_LOG_APIs APIs
 * @{
 * @ingroup ACE_HAL_LOG
 */

/**
 * @brief Get platform property.
 * The platform is required to support this API.
 * Some of the return values have an impact on the memory allocation or
 * impose a log length limitation.
 * @param property_id ID of the property as defined in
 * ::aceLogHal_platformProperty_t
 * @return Value of the property, otherwise one of the error codes of
 * ace_status_t.
 */
int32_t aceLogHal_getPlatformProperty(aceLogHal_platformProperty_t property_id);

/**
 * @brief Write a text or binary log.
 * @param[in] log_buffer_id Log buffer this log writes to
 * @param[in] prio Severity level of the log
 * @param[in] tag Tag of the log
 * @param[in] payload Pointer to the memory of the log message owned by the
 * caller
 * @param[in] len Length of the payload. If len is zero and flags is
 * #ACE_LOGHAL_TYPE_TXT, payload is treated as a string.
 * @param[in] flags Indicates the type of the log as defined in
 * ::aceLogHal_flags_t The support for #ACE_LOGHAL_TYPE_BIN is optional.
 * @return number of bytes written on success, otherwise one of the error codes
 * of ace_status_t. ACE_STATUS_INCOMPATIBLE_PARAMS should be returned if the
 * type indicated by the flags is not compatible with the log buffer.
 */
int32_t aceLogHal_write(aceLogHal_id_t log_buffer_id, aceLogHal_level_t prio,
                        const char* tag, const void* payload, size_t len,
                        int flags);

/**
 * @brief Open a read handle on a log buffer.
 * The support for this API is required only if the platform supports
 * aceLogHal_read().
 * @param[in] log_id ID of the log buffer to open
 * @param[in] mode Read mode as defined in ::aceLogHal_readMode_t
 * @return the handle of the log reader, or NULL on failure such as open a
 * buffer with unsupported mode or invalid log_id
 */
aceLogHal_readHandle_t aceLogHal_readOpen(aceLogHal_id_t log_id,
                                          aceLogHal_readMode_t mode);
/**
 * @brief Read one or multiple formatted text log entry or one binary log
 * message. Read should begin with the head of the log buffer and move on to the
 * next unread log entry with every aceLogHal_read() call.
 * The support for this API is optional and depends if the product includes
 * modules that reads logs using this API.
 * @param[in] handle The log buffer handler returned by aceLogHal_readOpen() to
 * read from
 * @param[in,out] log_mem Memory buffer to store the log. Caller needs to
 * provide the memory. The read log entry or message will be carried back via
 * this parameter. The number of the returned log and type (log message or log
 * entry) depends on the ::aceLogHal_readMode_t of the handle when it's opened.
 * Text log returns one or more formatted log depending on the
 * ::aceLogHal_readMode_t. If log_mem is not big enough to hold single complete
 * log, logs should be truncated from the end and partial log should be
 * returned. Partial text log should be terminated by null. Next read will
 * start from the next log entry.
 * @param[in] len Size of the log_mem buffer in bytes
 * @return an integer greater than 0 for the actual bytes read, otherwise one of
 * the error codes of ace_status_t. When reading text logs, return
 * ACE_STATUS_EOF when the reading position reaches the end of the log buffer.
 * When reading binary logs and no more log is available, aceLogHal_read()
 * should blocks and wait until logs become available. Returns
 * ACE_STATUS_BUFFER_OVERFLOW if log_mem is not big enough to hold the entry.
 */
int32_t aceLogHal_read(aceLogHal_readHandle_t handle, char* log_mem,
                       size_t len);

/**
 * @brief Close the read handle created using aceLogHal_readOpen().
 * This is required if the platform supports aceLogHal_readOpen().
 * @param[in] handle the file handle to be closed
 * @return ACE_STATUS_OK on success, otherwise one of the error codes of
 * ace_status_t.
 */
ace_status_t aceLogHal_readClose(aceLogHal_readHandle_t handle);
/** @} */

#ifdef __cplusplus
}
#endif

#endif  // ACE_HAL_LOG_H_
/** @} */
