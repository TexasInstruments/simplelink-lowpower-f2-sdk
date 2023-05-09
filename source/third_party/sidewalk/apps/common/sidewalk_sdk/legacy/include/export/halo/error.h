/*
 * Copyright 2020-2021 Amazon.com, Inc. or its affiliates. All rights reserved.
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

#ifndef HALO_LIB_HALO_EXPORT_HALO_ERROR_H
#define HALO_LIB_HALO_EXPORT_HALO_ERROR_H

typedef enum {
    HALO_ERROR_NONE                     =  0,
    HALO_ERROR_GENERIC                  = -1,
    HALO_ERROR_TIMEOUT                  = -2,
    /** Resource not available */
    HALO_ERROR_OUT_OF_RESOURCES         = -3,
    HALO_ERROR_OOM                      = -4,
    /** Out of file handles */
    HALO_ERROR_OUT_OF_HANDLES           = -5,
    HALO_ERROR_NOSUPPORT                = -6,
    /** No permission for operation */
    HALO_ERROR_NO_PERMISSION            = -7,
    /** Indicated resource not found */
    HALO_ERROR_NOT_FOUND                = -8,
    /** Null pointer provided */
    HALO_ERROR_NULL_POINTER             = -9,
    /** Parameter out of range */
    HALO_ERROR_PARAM_OUT_OF_RANGE       = -10,
    HALO_ERROR_INVALID_ARGS             = -11,
    /** Parameters form incompatible set */
    HALO_ERROR_INCOMPATIBLE_PARAMS      = -12,
    /** Input/Output error */
    HALO_ERROR_IO_ERROR                 = -13,
    /** Safe to try again */
    HALO_ERROR_TRY_AGAIN                = -14,
    /** Resource busy */
    HALO_ERROR_BUSY                     = -15,
    /** Mutex in dead lock */
    HALO_ERROR_DEAD_LOCK                = -16,
    /** Defined data type overflowed */
    HALO_ERROR_DATA_TYPE_OVERFLOW       = -17,
    /** Destination buffer overflowed */
    HALO_ERROR_BUFFER_OVERFLOW          = -18,
    /** Operation already in progress */
    HALO_ERROR_IN_PROGRESS              = -19,
    /** Operation canceled */
    HALO_ERROR_CANCELED                 = -20,
    /** Owner of resource died */
    HALO_ERROR_OWNER_DEAD               = -21,
    /** Unrecoverable error */
    HALO_ERROR_UNRECOVERABLE            = -22,
    /** Invalid port */
    HALO_ERROR_PORT_INVALID             = -23,
    /** Device port not opened */
    HALO_ERROR_PORT_NOT_OPEN            = -24,
    /** Resource uninitialized */
    HALO_ERROR_UNINITIALIZED            = -25,
    /** Resource already initialized */
    HALO_ERROR_ALREADY_INITIALIZED      = -26,
    /** Resource already exists */
    HALO_ERROR_ALREADY_EXISTS           = -27,
    /** Parameter below acceptable threshold */
    HALO_ERROR_BELOW_THRESHOLD          = -28,
    /** Resource stopped */
    HALO_ERROR_STOPPED                  = -29,
    /** Storage read failure */
    HALO_ERROR_STORAGE_READ_FAIL        = -30,
    /** Storage write failure */
    HALO_ERROR_STORAGE_WRITE_FAIL       = -31,
    /** Storage erase failure */
    HALO_ERROR_STORAGE_ERASE_FAIL       = -32,
    /** Storage is full */
    HALO_ERROR_STORAGE_FULL             = -33,
    /** Authentication failure */
    HALO_ERROR_AUTHENTICATION_FAIL      = -34,
    /** Encryption failure */
    HALO_ERROR_ENCRYPTION_FAIL          = -35,
    /** Decryption failure */
    HALO_ERROR_DECRYPTION_FAIL          = -36,
    /** Device ID obfuscation failure */
    HALO_ERROR_ID_OBFUSCATION_FAIL      = -37,
    /** Message routing failed: no route to destination with requested MTU size*/
    HALO_ERROR_NO_ROUTE_AVAILABLE       = -38,
    /** Invalid Response */
    HALO_ERROR_INVALID_RESPONSE         = -39,
    /** Invalid State */
    HALO_ERROR_INVALID_STATE            = -40,
} halo_error_t;

#endif /* !HALO_LIB_HALO_EXPORT_HALO_ERROR_H */
