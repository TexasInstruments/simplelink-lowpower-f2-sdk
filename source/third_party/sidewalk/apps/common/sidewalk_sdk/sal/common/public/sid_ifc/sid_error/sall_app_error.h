/*
 * Copyright 2019-2021 Amazon.com, Inc. or its affiliates. All rights reserved.
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

#ifndef SALL_APP_ERROR_H
#define SALL_APP_ERROR_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

static const uint32_t SALL_APP_ERR = 0xf0100000;

typedef enum {
    SALL_APP_NO_ERROR = 0,
    SALL_APP_ERR_SVC_HANDLER_MISSING = 1, ///< SVC handler is missing
    SALL_APP_ERR_SOFTDEVICE_NOT_ENABLED = 2, ///< SoftDevice has not been enabled
    SALL_APP_ERR_INTERNAL = 3, ///< Internal Error
    SALL_APP_ERR_NO_MEM = 4, ///< No Memory for operation
    SALL_APP_ERR_NOT_FOUND = 5, ///< Not found
    SALL_APP_ERR_NOT_SUPPORTED = 6, ///< Not supported
    SALL_APP_ERR_INVALID_PARAM = 7, ///< Invalid Parameter
    SALL_APP_ERR_INVALID_STATE = 8, ///< Invalid state, operation disallowed in this state
    SALL_APP_ERR_INVALID_LENGTH = 9, ///< Invalid Length
    SALL_APP_ERR_INVALID_FLAGS = 10, ///< Invalid Flags
    SALL_APP_ERR_INVALID_DATA = 11, ///< Invalid Data
    SALL_APP_ERR_DATA_SIZE = 12, ///< Data size exceeds limit
    SALL_APP_ERR_TIMEOUT = 13, ///< Operation timed out
    SALL_APP_ERR_NULL = 14, ///< Null Pointer
    SALL_APP_ERR_FORBIDDEN = 15, ///< Forbidden Operation
    SALL_APP_ERR_INVALID_ADDR = 16, ///< Bad Memory Address
    SALL_APP_ERR_BUSY = 17, ///< Busy
    SALL_APP_ERR_CONN_COUNT = 18, ///< Maximum connection count exceeded.
    SALL_APP_ERR_RESOURCES = 19, ///< Not enough resources for operation
    SALL_APP_ERR_HANDSHAKE_FAILED = 20, ///< Handshake Failed
    SALL_APP_ERR_DECRYPT_FAILED = 21, ///< Decrypt Error
    SALL_APP_ERR_ENCRYPT_FAILED = 22, ///< Encrypt Error
    SALL_APP_ERR_CLASS_NOT_SUPPORTED = 23, ///< Not supported
    SALL_APP_ERR_CMD_NOT_SUPPORTED = 24, ///< Not supported
    SALL_APP_ERR_HARDWARE_ERROR = 25, ///< Hardware Failure
    SALL_APP_ERR_SECURITY_ERROR = 26, ///< Security Error
    SALL_APP_ERR_PROTOCOL_MISMATCH = 27, ///< Protocol Mismatch
    SALL_APP_ERR_UNKNOWN = 28, ///< Unknown Error
    SALL_APP_ERR_TR_STORE_OVERFLOW = 29, ///< Transaction storage overflow
    SALL_APP_ERR_AUTH_ERROR = 30, ///< Failed cmac authentication
    SALL_APP_ERR_INVALID_VERSION = 31, ///< invalid version (not supported)
    SALL_APP_ERR_INVALID_RNET_ADDR = 32, ///<  RNet address is invalid
    SALL_APP_ERR_ALREADY_IN_PROGRESS = 33, /// < Already in Progress

    SALL_APP_ERR_MAX ///< Max error Value. Add new errors on top>
} sall_app_error_t;

#ifdef __cplusplus
}
#endif

#endif
