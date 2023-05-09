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

#ifndef SID_ERROR_H
#define SID_ERROR_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Maps to common error types, sid_error_t, etc
 */
typedef enum {
    SID_ERROR_NONE                     =  0,
    SID_ERROR_GENERIC                  = -1,
    SID_ERROR_TIMEOUT                  = -2,
    /** Resource not available */
    SID_ERROR_OUT_OF_RESOURCES         = -3,
    SID_ERROR_OOM                      = -4,
    /** Out of file handles */
    SID_ERROR_OUT_OF_HANDLES           = -5,
    SID_ERROR_NOSUPPORT                = -6,
    /** No permission for operation */
    SID_ERROR_NO_PERMISSION            = -7,
    /** Indicated resource not found */
    SID_ERROR_NOT_FOUND                = -8,
    /** Null pointer provided */
    SID_ERROR_NULL_POINTER             = -9,
    /** Parameter out of range */
    SID_ERROR_PARAM_OUT_OF_RANGE       = -10,
    SID_ERROR_INVALID_ARGS             = -11,
    /** Parameters form incompatible set */
    SID_ERROR_INCOMPATIBLE_PARAMS      = -12,
    /** Input/Output error */
    SID_ERROR_IO_ERROR                 = -13,
    /** Safe to try again */
    SID_ERROR_TRY_AGAIN                = -14,
    /** Resource busy */
    SID_ERROR_BUSY                     = -15,
    /** Mutex in dead lock */
    SID_ERROR_DEAD_LOCK                = -16,
    /** Defined data type overflowed */
    SID_ERROR_DATA_TYPE_OVERFLOW       = -17,
    /** Destination buffer overflowed */
    SID_ERROR_BUFFER_OVERFLOW          = -18,
    /** Operation already in progress */
    SID_ERROR_IN_PROGRESS              = -19,
    /** Operation canceled */
    SID_ERROR_CANCELED                 = -20,
    /** Owner of resource died */
    SID_ERROR_OWNER_DEAD               = -21,
    /** Unrecoverable error */
    SID_ERROR_UNRECOVERABLE            = -22,
    /** Invalid port */
    SID_ERROR_PORT_INVALID             = -23,
    /** Device port not opened */
    SID_ERROR_PORT_NOT_OPEN            = -24,
    /** Resource uninitialized */
    SID_ERROR_UNINITIALIZED            = -25,
    /** Resource already initialized */
    SID_ERROR_ALREADY_INITIALIZED      = -26,
    /** Resource already exists */
    SID_ERROR_ALREADY_EXISTS           = -27,
    /** Parameter below acceptable threshold */
    SID_ERROR_BELOW_THRESHOLD          = -28,
    /** Resource stopped */
    SID_ERROR_STOPPED                  = -29,
    /** Storage read failure */
    SID_ERROR_STORAGE_READ_FAIL        = -30,
    /** Storage write failure */
    SID_ERROR_STORAGE_WRITE_FAIL       = -31,
    /** Storage erase failure */
    SID_ERROR_STORAGE_ERASE_FAIL       = -32,
    /** Storage is full */
    SID_ERROR_STORAGE_FULL             = -33,
    /** Authentication failure */
    SID_ERROR_AUTHENTICATION_FAIL      = -34,
    /** Encryption failure */
    SID_ERROR_ENCRYPTION_FAIL          = -35,
    /** Decryption failure */
    SID_ERROR_DECRYPTION_FAIL          = -36,
    /** Device ID obfuscation failure */
    SID_ERROR_ID_OBFUSCATION_FAIL      = -37,
    /** Message routing failed: no route to destination with requested MTU size*/
    SID_ERROR_NO_ROUTE_AVAILABLE       = -38,
    /** Invalid Response */
    SID_ERROR_INVALID_RESPONSE         = -39,
    /** Invalid State */
    SID_ERROR_INVALID_STATE            = -40,
} sid_error_t;

#ifdef __cplusplus
}
#endif

#endif
