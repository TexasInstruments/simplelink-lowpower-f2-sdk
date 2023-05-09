/*
 * Copyright 2020 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.txt file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef RNET_ERRORS_H
#define RNET_ERRORS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

// values the same as NRF_ERRORs
typedef enum {
    RNET_SUCCESS                        = 0,  ///< Successful command
    RNET_ERROR_SVC_HANDLER_MISSING      = 1,  ///< SVC handler is missing
    RNET_ERROR_SOFTDEVICE_NOT_ENABLED   = 2,  ///< SoftDevice has not been enabled
    RNET_ERROR_INTERNAL                 = 3,  ///< Internal Error
    RNET_ERROR_NO_MEM                   = 4,  ///< No Memory for operation
    RNET_ERROR_NOT_FOUND                = 5,  ///< Not found
    RNET_ERROR_NOT_SUPPORTED            = 6,  ///< Not supported
    RNET_ERROR_INVALID_PARAM            = 7,  ///< Invalid Parameter
    RNET_ERROR_INVALID_STATE            = 8,  ///< Invalid state, operation disallowed in this state
    RNET_ERROR_INVALID_LENGTH           = 9,  ///< Invalid Length
    RNET_ERROR_INVALID_FLAGS            = 10, ///< Invalid Flags
    RNET_ERROR_INVALID_DATA             = 11, ///< Invalid Data
    RNET_ERROR_DATA_SIZE                = 12, ///< Data size exceeds limit
    RNET_ERROR_TIMEOUT                  = 13, ///< Operation timed out
    RNET_ERROR_NULL                     = 14, ///< Null Pointer
    RNET_ERROR_FORBIDDEN                = 15, ///< Forbidden Operation
    RNET_ERROR_INVALID_ADDR             = 16, ///< Bad Memory Address
    RNET_ERROR_BUSY                     = 17, ///< Busy
    RNET_ERROR_CONN_COUNT               = 18, ///< Maximum connection count exceeded.
    RNET_ERROR_RESOURCES                = 19, ///< Not enough resources for operation
    RNET_ERROR_HANDSHAKE_FAILED         = 20, ///< Handshake Failed
    RNET_ERROR_DECRYPT_FAILED           = 21, ///< Decrypt Error
    RNET_ERROR_ENCRYPT_FAILED           = 22, ///< Encrypt Error
    RNET_ERROR_CLASS_NOT_SUPPORTED      = 23, ///< Not supported
    RNET_ERROR_CMD_NOT_SUPPORTED        = 24, ///< Not supported
    RNET_ERROR_HARDWARE_ERROR           = 25, ///< Hardware Failure
    RNET_ERROR_SECURITY_ERROR           = 26, ///< Security Error
    RNET_ERROR_PROTOCOL_MISMATCH        = 27, ///< Protocol Mismatch
    RNET_ERROR_UNKNOWN                  = 28, ///< Unknown Error
    RNET_ERROR_TR_STORE_OVERFLOW        = 29, ///< Transaction storage overflow
    RNET_ERROR_AUTH_ERROR               = 30, ///< Failed cmac authentication
    RNET_ERROR_MAX                            ///< Max error Value. Add new errors on top>
} rnet_error_t;

typedef uint32_t ret_code_t;

#ifdef __cplusplus
}
#endif

#endif
