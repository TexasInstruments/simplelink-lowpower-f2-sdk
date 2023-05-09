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
/**
 *
 * @file ace_status.h
 * @ingroup ACE_STATUS
 * @brief Definition of ACE status codes.
 *      - Bluetooth Status Codes @ref aceBT_status_t.
 *      - Wifi Status Codes @ref aceWifiMgr_error_t.
 */

#ifndef ACE_STATUS_H
#define ACE_STATUS_H

typedef enum {
    /**
     * @name General
     * @brief Basic status returns.
     *        Reserved value[0 to -99]
     * @{
     */

    /** Operation completed successfully */
    ACE_STATUS_OK = 0,
    /** Unspecified run-time error */
    ACE_STATUS_GENERAL_ERROR = -1,
    /** Operation timed out */
    ACE_STATUS_TIMEOUT = -2,
    /** Resource not available */
    ACE_STATUS_OUT_OF_RESOURCES = -3,
    /** Failed to allocate memory */
    ACE_STATUS_OUT_OF_MEMORY = -4,
    /** Out of file handles */
    ACE_STATUS_OUT_OF_HANDLES = -5,
    /** Not supported on this platform */
    ACE_STATUS_NOT_SUPPORTED = -6,
    /** No permission for operation */
    ACE_STATUS_NO_PERMISSION = -7,
    /** Indicated resource not found */
    ACE_STATUS_NOT_FOUND = -8,
    /** Null pointer provided */
    ACE_STATUS_NULL_POINTER = -9,
    /** Parameter out of range */
    ACE_STATUS_PARAM_OUT_OF_RANGE = -10,
    /** Parameter value bad */
    ACE_STATUS_BAD_PARAM = -11,
    /** Parameters form incompatible set */
    ACE_STATUS_INCOMPATIBLE_PARAMS = -12,
    /** Input/Output error */
    ACE_STATUS_IO_ERROR = -13,
    /** Safe to try again */
    ACE_STATUS_TRY_AGAIN = -14,
    /** Resource busy */
    ACE_STATUS_BUSY = -15,
    /** Mutex in dead lock */
    ACE_STATUS_DEAD_LOCK = -16,
    /** Defined data type overflowed */
    ACE_STATUS_DATA_TYPE_OVERFLOW = -17,
    /** Destination buffer overflowed */
    ACE_STATUS_BUFFER_OVERFLOW = -18,
    /** Operation already in progress */
    ACE_STATUS_IN_PROGRESS = -19,
    /** Operation canceled */
    ACE_STATUS_CANCELED = -20,
    /** Owner of resource died */
    ACE_STATUS_OWNER_DEAD = -21,
    /** Unrecoverable error */
    ACE_STATUS_UNRECOVERABLE = -22,
    /** Invalid port */
    ACE_STATUS_PORT_INVALID = -23,
    /** Device port not opened */
    ACE_STATUS_PORT_NOT_OPEN = -24,
    /** Resource uninitialized */
    ACE_STATUS_UNINITIALIZED = -25,
    /** Resource already initialized */
    ACE_STATUS_ALREADY_INITIALIZED = -26,
    /** Resource already exists */
    ACE_STATUS_ALREADY_EXISTS = -27,
    /** Parameter below acceptable threshold */
    ACE_STATUS_BELOW_THRESHOLD = -28,
    /** Resource stopped */
    ACE_STATUS_STOPPED = -29,
    /** Storage read failure */
    ACE_STATUS_STORAGE_READ_FAIL = -30,
    /** Storage write failure */
    ACE_STATUS_STORAGE_WRITE_FAIL = -31,
    /** Storage erase failure */
    ACE_STATUS_STORAGE_ERASE_FAIL = -32,
    /** Storage is full */
    ACE_STATUS_STORAGE_FULL = -33,
    /** API/Operation is not implemented */
    ACE_STATUS_NOT_IMPLEMENTED = -34,
    /** Resource can be reclaimed */
    ACE_STATUS_RESOURCE_RECLAIMABLE = -35,
    /** Data is corrupted */
    ACE_STATUS_DATA_CORRUPTED = -36,
    /** Connected */
    ACE_STATUS_CONNECTED = -37,
    /** Disconnected */
    ACE_STATUS_DISCONNECTED = -38,
    /** Reset occured */
    ACE_STATUS_RESET = -39,
    /* @} */

    /**
     * @name File operation
     * @brief Errors related to file operations
     *        Reserved value[-100 to -199]
     * @{
     */

    /** Filesystem not integrated with the system */
    ACE_STATUS_FAILURE_UNKNOWN_FILESYSTEM = -100,
    /** System already configured with the maximum number of filesystems */
    ACE_STATUS_FAILURE_MAX_FILESYSTEMS = -101,
    /** Flle operation not compatible */
    ACE_STATUS_FAILURE_INCOMPATIBLE_FILE = -102,
    /** File not open */
    ACE_STATUS_FAILURE_FILE_NOT_OPEN = -103,
    /** File pointer has reached end of file */
    ACE_STATUS_EOF = -104,
    /** Requested size not supported */
    ACE_STATUS_MAX_FILE_SIZE_REACHED = -105,
    /** Operation not supported */
    ACE_STATUS_FAILURE_UNKNOWN_FILE = -106,
    /** Directory already exists */
    ACE_STATUS_DIR_EXISTS = -107,
    /** Filesystem does not support directories */
    ACE_STATUS_DIR_NOT_SUPPORTED = -108,
    /** Invalid path */
    ACE_STATUS_INVALID_PATH = -109,

    /* @} */

    /**
     * @name Network
     * @brief Errors related to network or sockets
     *        Reserved [-200 to -299]
     * @{
     */

    /** Invalid data length received */
    ACE_STATUS_DATA_LEN_INVALID = -200,
    /** No network available */
    ACE_STATUS_NO_NET = -201,
    /** Network connection error */
    ACE_STATUS_NET_CONNECTION_ERROR = -202,
    /** Network connection timeout error */
    ACE_STATUS_NET_CONNECTION_TIMEOUT_ERROR = -203,
    /** Network transmit abort error */
    ACE_STATUS_NET_TRANSMIT_ABORT_ERROR = -204,
    /** Network transmit abort error */
    ACE_STATUS_NET_RECEIVE_ABORT_ERROR = -205,
    /** Connection failed due to authentication failure */
    ACE_STATUS_NET_AUTH_FAILURE = -206,

    /* @} */

    /**
     * @name Crash manager
     * @brief Errors related to crash manager module
     *        Reserved value[-300 to -399]
     * @{
     */

    /* @} */

    /**
     * @name Metric
     * @brief Errors related to metric module
     *        Reserved value[-400 to -499]
     * @{
     */

    /* @} */

    /**
     * @name AIPC
     * @brief Errors related to AIPC module
     *        Reserved value[-500 to -599]
     * @{
     */

    /* @} */

    /**
     * @name CLI
     * @brief Errors related to CLI module
     *        Reserved value[-600 to -699]
     * @{
     */

    /** Command passed to print help */
    ACE_STATUS_CLI_HELP_COMMAND = -600,

    /** CLI function failed */
    ACE_STATUS_CLI_FUNC_ERROR = -601,

    /* @} */

    /**
     * @name CMSIS
     * @brief Errors related to CMSIS module
     *        Reserved value[-700 to -799]
     * @{
     */

    /* @} */

    /**
     * @name Dispatcher
     * @brief Errors related to dispatcher module
     *        Reserved value[-800 to -899]
     * @{
     */

    /* @} */

    /**
     * @name Shared memory
     * @brief Errors related to shared memory module
     *        Reserved value[-900 to -999]
     * @{
     */

    /* @} */

    /**
     * @name Dropbox
     * @brief Errors related to dropbox module
     *        Reserved value[-1000 to -1099]
     * @{
     */

    /* @} */

    /**
     * @name Events
     * @brief Errors related to events module
     *        Reserved value[-1100 to -1199]
     * @{
     */
    /** Events exceeded maximum number of subscribers */
    ACE_STATUS_EVENTS_MAX_SUBSCRIBERS = -1100,

    /* @} */

    /* Range: 1200-1299 unused */

    /**
     * @name Logging
     * @brief Errors related to logging module
     *        Reserved value[-1300 to -1399]
     * @{
     */

    /* @} */

    /**
     * @name Automatic time zone service
     * @brief Errors related to ATZ module
     *        Reserved value[-1400 to -1499]
     * @{
     */

    /** ACE ATZ internal error */
    ACE_STATUS_ATZ_INTERNAL_ERROR = -1400,

    /* @} */

    /**
     * @name Bluetooth middleware service
     * @brief Errors related to bluetooth module
     *        Reserved value[-1500 to -1599]
     *        Bluetooth Status Codes @ref aceBT_status_t.
     * @{
     */

    /** BT Error codes below will be deprecated.
    * They are defined here for backward compatiblity
    */
    ACE_STATUS_BT_JNI_ENVIRONMENT_ERROR = -1501,

    ACE_STATUS_BT_JNI_THREAD_ATTACH_ERROR = -1502,

    ACE_STATUS_BT_WAKELOCK_ERROR = -1503,

    ACE_STATUS_BT_CONN_PENDING = -1504,

    ACE_STATUS_BT_AUTH_FAIL_CONN_TIMEOUT = -1505,

    ACE_STATUS_BT_RMT_DEV_DOWN = -1506,

    ACE_STATUS_BT_DONE = -1507,

    ACE_STATUS_BT_UNHANDLED = -1508,

    ACE_STATUS_BT_AUTH_REJECTED = -1509,

    ACE_STATUS_BT_AUTH_FAIL_SMP_FAIL = -1510,
    /* End of BT errors which will be deprecated.*/

    /* @} */

    /**
     * @name Button manager service
     * @brief Errors related to button manager module
     *        Reserved value[-1600 to -1699]
     * @{
     */

    /* @} */

    /**
     * @name Device information module
     * @brief Errors related to device information module
     *        Reserved value[-1700 to -1799]
     * @{
     */

    /** Error internal to device information middleware module */
    ACE_STATUS_DEVICE_INFO_INTERNAL_ERROR = -1700,

    /** Device information entry not supported */
    ACE_STATUS_DEVICE_INFO_ENTRY_NOT_SUPPORTED = -1701,

    /* @} */

    /**
     * @name Factory reset module
     * @brief Errors related to factory reset module
     *        Reserved value[-1800 to -1899]
     * @{
     */

    /* @} */

    /**
     * @name Frustration free setup service
     * @brief Errors related to FFS module
     *        Reserved value[-1900 to -1999]
     * @{
     */

    /* @} */

    /**
     * @name HTTP client
     * @brief Errors related to HTTP client module
     *        Reserved value[-2000 to -2099]
     * @{
     */

    /* @} */

    /**
     * @name TLS store
     * @brief Errors related to TLS store module
     *        Reserved value[-2100 to -2199]
     * @{
     */

    /* @} */

    /**
     * @name Input manager
     * @brief Errors related to input manager module
     *        Reserved value[-2200 to -2299]
     * @{
     */

    /* @} */

    /**
     * @name LED manager
     * @brief Errors related to LED manager module
     *        Reserved value[-2300 to -2399]
     * @{
     */

    /* @} */

    /**
     * @name Maplite
     * @brief Errors related to maplite module
     *        Reserved value[-2400 to -2499]
     * @{
     */

    /** Module initialization error */
    ACE_STATUS_MODULE_INIT_ERROR = -2400,

    /** Registration request error */
    ACE_STATUS_REGISTRATION_REQUEST_ERROR = -2401,

    /** Registration response error */
    ACE_STATUS_REGISTRATION_RESPONSE_ERROR = -2402,

    /** Device not registered error */
    ACE_STATUS_NOT_REGISTERED_ERROR = -2403,

    /** Invalid registration info error */
    ACE_STATUS_REGISTRATION_INVALID_INFO = -2404,

    /** Registration request payload error */
    ACE_STATUS_REGISTRATION_REQUEST_PAYLOAD_ERROR = -2405,

    /** Registration invalid response error */
    ACE_STATUS_REGISTRATION_INVALID_RESPONSE = -2406,

    /** Registration internal error */
    ACE_STATUS_REGISTRATION_INTERNAL_ERROR = -2407,

    /** Access Token Expired */
    ACE_STATUS_ACCESS_TOKEN_EXPIRED = -2408,

    /** Invalid refresh token */
    ACE_STATUS_INVALID_REFRESH_TOKEN = -2409,

    /** ACM connection error */
    ACE_STATUS_ACM_CONNECTION_ERROR = -2410,

    /* @} */

    /**
     * @name OTA
     * @brief Errors related to OTA module
     *        Reserved value[-2500 to -2599]
     * @{
     */

    /* @} */

    /**
     * @name Power
     * @brief Errors related to power manager
     *        Reserved value[-2600 to -2699]
     * @{
     */
    /** Resource reference count already zero */
    ACE_STATUS_PWR_ZERO_REF_COUNT = -2600,

    /* @} */

    /**
     * @name Thermal
     * @brief Errors related to thermal module
     *        Reserved value[-2700 to -2799]
     * @{
     */

    /** Thermal get data error */
    ACE_STATUS_THERMAL_GETDATA_ERR = -2700,

    /** Thermal load policy error */
    ACE_STATUS_THERMAL_LOAD_POLICY_ERR = -2701,

    /** Thernal cli command execution error */
    ACE_STATUS_THERMAL_FUNC_ERR = -2702,

    /* @} */

    /**
     * @name WIFI
     * @brief Errors related to WIFI module
     *        Reserved value[-2800 to -2899]
     *        Wifi Status Codes @ref aceWifiMgr_error_t.
     * @{
     */
    /** Error indicating violation of the agreed
        exchange protocol */
    ACE_STATUS_PROTOCOL_ERROR = -2800,

    /** Indicating sender will send more data */
    ACE_STATUS_MORE_DATA = -2801,
    /* @} */

    /* Range: 2900-2999 unused */

    /**
     * @name HAL
     * @brief Errors related to hardware abstraction layer
     *        Reserved value[-10000 to -10999]
     * @{
     */

    /** Invalid baud rate selected */
    ACE_STATUS_BAUDRATE_INVALID = -10001,
    /** Bad parity */
    ACE_STATUS_PARITY_INVALID = -10002,
    /** Device returned bad stop bits */
    ACE_STATUS_STOP_BITS_INVALID = -10003,
    /** Device has bad flow control */
    ACE_STATUS_FLOW_CONTROL_INVALID = -10004,
    /** Device/SW state machine invalid */
    ACE_STATUS_DEVICE_STATE_INVALID = -10005,
    /** Unknown hardware failure */
    ACE_STATUS_HW_FAILURE = -10006,
    /** Device operation error */
    ACE_STATUS_DEVICE_OPERATION_ERROR = -10007,
    /** Device initialization error */
    ACE_STATUS_INIT_ERROR = -10008,
    /** Power/Thermal policy write error */
    ACE_STATUS_POLICY_WRITE_INVALID = -10009,
    /** Device not found */
    ACE_STATUS_DEVICE_NOT_FOUND = -10010,
    /** Device not configured */
    ACE_STATUS_DEVICE_NO_CONFIG = -10011,

    /** KV storage database open error */
    ACE_STATUS_DB_OPEN_ERROR = -10012,

    /* @} */
} ace_status_t;

#endif /* ACE_STATUS_H */
