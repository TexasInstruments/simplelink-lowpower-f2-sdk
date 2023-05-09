/*
 * Copyright 2018-2019 Amazon.com, Inc. or its affiliates. All rights reserved.
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
 * @file hal_device_info.h
 *
 * @brief    Device Info HAL provides access to Device Info data.
 * @addtogroup  ACE_HAL_DEVICE_INFO
 * @{
 * @details  For porting suggestions, guides or typical implementation details, check
 *           @ref DEVICE_INFO_PORTING_GUIDE "Device Info Porting Guide".
 */

#ifndef ACE_HAL_DEVICE_INFO_H_
#define ACE_HAL_DEVICE_INFO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <ace/ace_config.h>
#include <ace/ace_status.h>
#include <stdint.h>

/**
 * @defgroup ACE_HAL_DEVICE_INFO_DS Data Structures and Enums
 * @brief Definitions for Device Info data entries and size limits.
 * @details %Entry can be set to return ACE_STATUS_DEVICE_INFO_ENTRY_NOT_SUPPORTED if it is not applicable.
 * Unimplemented entry should return ACE_STATUS_DEVICE_INFO_ENTRY_NOT_SUPPORTED.
 *
 * Run Device Info HAL test to make sure all the Device Info implementation is OK.
 * Need to pass all @ref ACE_HAL_TESTS_DEVICE_INFO "Device Info HAL tests".
 * @ingroup ACE_HAL_DEVICE_INFO
 * @{
 */

/**
 * @brief Device Info entry type.
 *
 * Legend for comments for acehal_device_info_entry_t:
 *
 * [description] <br>
 * scope: [scope - 1P, 1P and 3P] <br>
 * used by components: [component] <br>
 * format: [format] <br>
 * example: [example] <br>
 * notes: [notes] (optional)
 */
typedef enum {
    /**
     * [OS version number] <br>
     * scope: [1P and 3P] <br>
     * used by components: [FFS, Logging, Metrics] <br>
     * format: [xxx.xxx.xxx] <br>
     * example: [0.1.2] <br>
     * notes: [operating system version number; stored as string; used in FFS metrics upload]
     */
    ACE_DEVICE_INFO_OS_VERSION = 0,
    OS_VERSION = 0, /**< deprecated */

    /**
     * [software version number] <br>
     * scope: [1P] <br>
     * used by components: [OTA, Logging, Metrics] <br>
     * format: [xxxx] <br>
     * example: [0123456789] <br>
     * notes: [a decimal number usually uint64; stored as string]
     */
    ACE_DEVICE_INFO_VERSION_NUMBER = 1,
    VERSION_NUMBER = 1, /**< deprecated */

    /**
     * [build fingerprint] <br>
     * scope: [1P] <br>
     * used by components: [Logging, Metrics] <br>
     * format: [$(BRAND)/$(PRODUCT)/$(DEVICE)/$(BOARD):$(VERSION.RELEASE)/$(ID)/$(VERSION.INCREMENTAL):$(TYPE)/$(TAGS)] <br>
     * example: [ABC/abc/abc/abc:1.0/ABC/01234/:userdebug/dev-keys]
     */
    ACE_DEVICE_INFO_BUILD_FINGERPRINT = 2,
    BUILD_FINGERPRINT = 2, /**< deprecated */

    /**
     * [product name] <br>
     * scope: [1P] <br>
     * used by components: [Logging, Metrics] <br>
     * format: [xxxx] <br>
     * example: [productABC] <br>
     * notes: [name string to describe the product]
     */
    ACE_DEVICE_INFO_PRODUCT_NAME = 3,
    PRODUCT_NAME = 3, /**< deprecated */

    /**
     * [platform build variant] <br>
     * scope: [1P] <br>
     * used by components: [Logging, Metrics] <br>
     * format: [xxxx] <br>
     * example: [userdebug]/[release]
     */
    ACE_DEVICE_INFO_BUILD_VARIANT = 4,
    BUILD_VARIANT = 4, /**< deprecated */

    /**
     * [build tags] <br>
     * scope: [1P] <br>
     * used by components: [OTA] <br>
     * format: [xxx-keys] <br>
     * example: [release-keys]/[dev-keys]
     */
    ACE_DEVICE_INFO_BUILD_TAGS = 5,
    BUILD_TAGS = 5, /**< deprecated */

    /**
     * [package name] <br>
     * scope: [1P] <br>
     * used by components: [OTA] <br>
     * format: [com.xxx.xxx] <br>
     * example: [com.amazon.abc] <br>
     * notes: [reverse domain-style identifier]
     */
    ACE_DEVICE_INFO_PACKAGE_NAME = 6,
    PACKAGE_NAME = 6, /**< deprecated */

    /**
     * [hardware platform (chipset) name] <br>
     * scope: [1P] <br>
     * used by components: [Logging, Metrics] <br>
     * format: [xxxx] <br>
     * example: [mt8167] <br>
     * notes: [reverse domain-style identifier]
     */
    ACE_DEVICE_INFO_HARDWARE_NAME = 7,
    HARDWARE_NAME = 7, /**< deprecated */

    /**
     * [bootloader version] <br>
     * scope: [1P] <br>
     * used by components: [Logging, Metrics] <br>
     * format: [xxxx] <br>
     * example: [unknown] <br>
     * notes: [value can be set to "unknown"]
     */
    ACE_DEVICE_INFO_BOOTLOADER = 8,
    BOOTLOADER = 8, /**< deprecated */

    /**
     * [build description] <br>
     * scope: [1P] <br>
     * used by components: [] <br>
     * format: [$(PRODUCT)-$(TYPE) $(VERSION.RELEASE) $(ID) $(VERSION.INCREMENTAL) $(TAGS)] <br>
     * example: [ABC-userdebug 1.0 ABC 12345 dev-keys]
     */
    ACE_DEVICE_INFO_BUILD_DESC = 9,
    BUILD_DESC = 9, /**< deprecated */

    /**
     * [device serial number (DSN)] <br>
     * scope: [1P and 3P] <br>
     * used by components: [FFS, Logging, Metrics] <br>
     * format: [xxxx] <br>
     * example: [0123456789ABC]
     */
    ACE_DEVICE_INFO_DEVICE_SERIAL = 10,
    DEVICE_SERIAL = 10, /**< deprecated */

    /**
     * [device type id] <br>
     * scope: [1P and 3P] <br>
     * used by components: [FFS, Logging, Metrics] <br>
     * format: [xxxx] <br>
     * example: [0123456789ABC]
     */
    ACE_DEVICE_INFO_DEVICE_TYPE_ID = 11,
    DEVICE_TYPE_ID = 11, /**< deprecated */

    /**
     * [WiFi MAC address] <br>
     * scope: [1P and 3P] <br>
     * used by components: [FFS] <br>
     * format: [xxxxxxxxxxxx] <br>
     * example: [AABBCC112233]
     */
    ACE_DEVICE_INFO_WIFI_MAC = 12,
    WIFI_MAC = 12, /**< deprecated */

    /**
     * [BT MAC address] <br>
     * scope: [1P] <br>
     * used by components: [] <br>
     * format: [xxxxxxxxxxxx] <br>
     * example: [AABBCC112233]
     */
    ACE_DEVICE_INFO_BT_MAC = 13,
    BT_MAC = 13, /**< deprecated */

    /**
     * [MAC secret] <br>
     * scope: [1P] <br>
     * used by components: [] <br>
     * format: [xxxx] <br>
     * example: [0] <br>
     * notes: [value can be set to "0"; MAC secret is optional]
     */
    ACE_DEVICE_INFO_MAC_SECRET = 14,
    MAC_SECRET = 14, /**< deprecated */

    /**
     * [manufacturing name] <br>
     * scope: [1P and 3P] <br>
     * used by components: [FFS] <br>
     * format: [xxxx] <br>
     * example: [0123456789ABC] <br>
     * notes: [used in FFS metrics upload]
     */
    ACE_DEVICE_INFO_MANUFACTURING = 15,
    MANUFACTURING = 15, /**< deprecated */

    /**
     * [product id] <br>
     * scope: [1P and 3P] <br>
     * used by components: [FFS] <br>
     * format: [xxxx] <br>
     * example: [AB12] <br>
     * notes: [used in FFS Bluetooth beacon data]
     */
    ACE_DEVICE_INFO_PRODUCT_ID = 16,
    PRODUCT_ID = 16, /**< deprecated */

    /**
     * [FFS pin] <br>
     * scope: [1P and 3P] <br>
     * used by components: [FFS] <br>
     * format: [xxxx] <br>
     * example: [ABCDE1234] <br>
     * notes: [9 digit pin; needed for FFS 2D barcode (if supported for product)]
     */
    ACE_DEVICE_INFO_FFS_PIN = 17,
    FFS_PIN = 17, /**< deprecated */

    /**
     * [device hardware revision] <br>
     * scope: [1P and 3P] <br>
     * used by components: [FFS] <br>
     * format: [xxxx] <br>
     * example: [0] <br>
     * notes: [value can be set to "0"; (optional for FFS, used in FFS metrics upload)]
     */
    ACE_DEVICE_INFO_DEV_HW_REV = 18,
    DEV_HW_REV = 18, /**< deprecated */

    /**
     * [Digital Signature Standard public key] <br>
     * scope: [1P and 3P] <br>
     * used by components: [FFS] <br>
     * format: [xxxx] <br>
     * example: [] <br>
     * notes: [DSS public key in PEM format]
     */
    ACE_DEVICE_INFO_DSS_PUB_KEY = 19,
    DSS_PUB_KEY = 19, /**< deprecated */

    /* @internal */

    CAP_SET_INDEX_KEY = 20, /**< reserved */
    SIGN_TYPE = 21,         /**< reserved */
    DEVICEID = 22,          /**< reserved */
    BOARDID = 23,           /**< reserved */
    PRODUCTIDX = 24,        /**< reserved */
    FWVERSION = 25,         /**< reserved */
    HWVERSION = 26,         /**< reserved */
    MODEL = 27,             /**< reserved */
    MFGNAME = 28,           /**< reserved */
    PLATFORM_NAME = 29,     /**< reserved */
    BT_MFG = 30,            /**< reserved */
    WIFI_MFG = 31,          /**< reserved */
    PRODUCT_ID2 = 32,       /**< reserved */
    BOOTMODE = 33,          /**< reserved */
    POSTMODE = 34,          /**< reserved */
    BOOTCOUNT = 35,         /**< reserved */
    UNLOCK_CODE = 36,       /**< reserved */
    MICCAL0 = 37,           /**< reserved */
    MICCAL1 = 38,           /**< reserved */
    MICCAL2 = 39,           /**< reserved */
    MICCAL3 = 40,           /**< reserved */
    MICCAL4 = 41,           /**< reserved */
    MICCAL5 = 42,           /**< reserved */
    MICCAL6 = 43,           /**< reserved */
    ALSCAL = 44,            /**< reserved */
    KB = 45,                /**< reserved */
    DKB = 46,               /**< reserved */
    DEV_FLAGS = 47,         /**< reserved */
    FOS_FLAGS = 48,         /**< reserved */
    USR_FLAGS = 49,         /**< reserved */
    LOCALE = 50,            /**< reserved */
    CHARGER_DET = 51,       /**< reserved */
    ALSPARAMS = 52,         /**< reserved */
    ETH_MAC_ADDR = 53,      /**< reserved */
    LEDPARAMS = 54,         /**< reserved */
    T_UNLOCK_CODE = 55,     /**< reserved */
    T_UNLOCK_CERT = 56,     /**< reserved */

    /* @endinternal */

    /**
     * [product name for Bluetooth advertising] <br>
     * scope: [1P and 3P] <br>
     * used by components: [FFS] <br>
     * format: [xxxx] <br>
     * example: [ABC Product] <br>
     * notes: [used for FFS Bluetooth advertising]
     */
    ACE_DEVICE_INFO_BT_DEVICE_NAME = 57,
    BT_DEVICE_NAME = 57, /**< deprecated */

    /**
     * [client id] <br>
     * scope: [1P and 3P] <br>
     * used by components: [FFS] <br>
     * format: [xxxx] <br>
     * example: [xxxx.xxxx-xxxx-xxxx.xxxx] <br>
     * notes: [the product's client ID. To access this information, go to Amazon's Developer Console]
     */
    ACE_DEVICE_INFO_CLIENT_ID = 58,
    CLIENT_ID = 58,  /**< deprecated */

    INVALID_ENTRY,
} acehal_device_info_entry_t;

/**
 * @defgroup ACE_HAL_DEVICE_INFO_DS_MAX_LENGTH Data Info Entry Length Limits
 * @brief Device Info max length default value.
 * @details Max length can be defined in config file if different from default.
 * Example definition in config file : <br>
 * @code
 * #define DEVICE_INFO_OS_VERSION_MAX_LEN 32
 * @endcode
 * @ingroup ACE_HAL_DEVICE_INFO_DS
 * @{
 */

/** Maximum length for entry os version */
#ifndef DEVICE_INFO_OS_VERSION_MAX_LEN
#define DEVICE_INFO_OS_VERSION_MAX_LEN 16
#endif

/** Maximum length for version number */
#ifndef DEVICE_INFO_VERSION_NUMBER_MAX_LEN
#define DEVICE_INFO_VERSION_NUMBER_MAX_LEN 128
#endif

/** Maximum length for build fingerprint */
#ifndef DEVICE_INFO_BUILD_FINGERPRINT_MAX_LEN
#define DEVICE_INFO_BUILD_FINGERPRINT_MAX_LEN 128
#endif

/** Maximum length for product name */
#ifndef DEVICE_INFO_PRODUCT_NAME_MAX_LEN
#define DEVICE_INFO_PRODUCT_NAME_MAX_LEN 32
#endif

/** Maximum length for build variant */
#ifndef DEVICE_INFO_BUILD_VARIANT_MAX_LEN
#define DEVICE_INFO_BUILD_VARIANT_MAX_LEN 32
#endif

/** Maximum length for build tags */
#ifndef DEVICE_INFO_BUILD_TAGS_MAX_LEN
#define DEVICE_INFO_BUILD_TAGS_MAX_LEN 48
#endif

/** Maximum length for package name */
#ifndef DEVICE_INFO_PACKAGE_NAME_MAX_LEN
#define DEVICE_INFO_PACKAGE_NAME_MAX_LEN 64
#endif

/** Maximum length for hardware name */
#ifndef DEVICE_INFO_HARDWARE_NAME_MAX_LEN
#define DEVICE_INFO_HARDWARE_NAME_MAX_LEN 32
#endif

/** Maximum length for bootloader */
#ifndef DEVICE_INFO_BOOTLOADER_MAX_LEN
#define DEVICE_INFO_BOOTLOADER_MAX_LEN 32
#endif

/** Maximum length for build desc */
#ifndef DEVICE_INFO_BUILD_DESC_MAX_LEN
#define DEVICE_INFO_BUILD_DESC_MAX_LEN 64
#endif

/** Maximum length for device serial */
#ifndef DEVICE_INFO_DEVICE_SERIAL_MAX_LEN
#define DEVICE_INFO_DEVICE_SERIAL_MAX_LEN 32
#endif

/** Maximum length for device type */
#ifndef DEVICE_INFO_DEVICE_TYPE_ID_MAX_LEN
#define DEVICE_INFO_DEVICE_TYPE_ID_MAX_LEN 16
#endif

/** Maximum length for wifi mac */
#ifndef DEVICE_INFO_WIFI_MAC_MAX_LEN
#define DEVICE_INFO_WIFI_MAC_MAX_LEN 32
#endif

/** Maximum length for bt mac */
#ifndef DEVICE_INFO_BT_MAC_MAX_LEN
#define DEVICE_INFO_BT_MAC_MAX_LEN 32
#endif

/** Maximum length for manufacturing */
#ifndef DEVICE_INFO_MANUFACTURING_MAX_LEN
#define DEVICE_INFO_MANUFACTURING_MAX_LEN 32
#endif

/** Maximum length for product id */
#ifndef DEVICE_INFO_PRODUCT_ID_MAX_LEN
#define DEVICE_INFO_PRODUCT_ID_MAX_LEN 16
#endif

/** Maximum length for ffs pin */
#ifndef DEVICE_INFO_FFS_PIN_MAX_LEN
#define DEVICE_INFO_FFS_PIN_MAX_LEN 16
#endif

/** Maximum length for dev hw rev */
#ifndef DEVICE_INFO_DEV_HW_REV_MAX_LEN
#define DEVICE_INFO_DEV_HW_REV_MAX_LEN 32
#endif

/** Maximum length for dss pub key */
#ifndef DEVICE_INFO_DSS_PUB_KEY_MAX_LEN
#define DEVICE_INFO_DSS_PUB_KEY_MAX_LEN 384
#endif

/** Maximum length for cap set index key */
#ifndef DEVICE_INFO_CAP_SET_INDEX_KEY_MAX_LEN
#define DEVICE_INFO_CAP_SET_INDEX_KEY_MAX_LEN 8
#endif

/** Maximum length for sign type */
#ifndef DEVICE_INFO_SIGN_TYPE_MAX_LEN
#define DEVICE_INFO_SIGN_TYPE_MAX_LEN 8
#endif

/** Maximum length for bt device name */
#ifndef DEVICE_INFO_BT_DEVICE_NAME_MAX_LEN
#define DEVICE_INFO_BT_DEVICE_NAME_MAX_LEN 64
#endif

/** Maximum length for client id name */
#ifndef DEVICE_INFO_CLIENT_ID_NAME_MAX_LEN
#define DEVICE_INFO_CLIENT_ID_NAME_MAX_LEN 128
#endif

/** Maximum length for firmware version */
#ifndef DEVICE_INFO_FIRMWARE_VERSION_MAX_LEN
#define DEVICE_INFO_FIRMWARE_VERSION_MAX_LEN DEVICE_INFO_VERSION_NUMBER_MAX_LEN
#endif

/** @} */
/** @} */

/**
 * @brief Get Device Info entry
 *
 * Get the content for the specified Device Info entry. The caller must allocate
 * the required memory based on the size of the entry by @ref aceDeviceInfoDsHal_getEntrySize
 * or use @ref ACE_HAL_DEVICE_INFO_DS_MAX_LENGTH "Device Info Entry Length Limits"
 *
 * @param[in] entry: Enum type of specified Device Info entry.
 * @param[in,out] data: Pointer to the Device Info entry content output buffer (caller allocated).
 * @param[in] data_size: Buffer size of the content output buffer.
 * Mimimum buffer size is content length + 1. Refer to @ref aceDeviceInfoDsHal_getEntrySize
 * to get the content length.
 *
 * @return String length of the specified entry on success, or an ace_status_t error otherwise.
 * @retval ACE_STATUS_DEVICE_INFO_ENTRY_NOT_SUPPORTED %Entry is not supported on this platform.
 * @retval ACE_STATUS_BAD_PARAM Input paramter is invalid.
 * @retval ACE_STATUS_BUFFER_OVERFLOW Content output buffer is smaller than the entry data size.
 * @retval ACE_STATUS_DEVICE_INFO_INTERNAL_ERROR Other internal errors.
 */
int aceDeviceInfoDsHal_getEntry(const acehal_device_info_entry_t entry, char* data, const uint32_t data_size);

/**
 * @brief Get Device Info entry size
 *
 * Get the size of a specified Device Info entry.
 *
 * @param[in] entry: Enum type of specified Device Info entry.
 *
 * @return String length of the specified entry on success, or an ace_status_t error otherwise.
 * Length returned is the content length without NULL terminator.
 * When allocating memory for aceDeviceInfoDsHal_getEntry data output buffer, need to use length + 1.
 * @retval ACE_STATUS_DEVICE_INFO_ENTRY_NOT_SUPPORTED %Entry is not supported on this platform.
 * @retval ACE_STATUS_BAD_PARAM Input paramter is invalid.
 * @retval ACE_STATUS_DEVICE_INFO_INTERNAL_ERROR Other internal errors.
 */
int aceDeviceInfoDsHal_getEntrySize(const acehal_device_info_entry_t entry);

#ifdef __cplusplus
}
#endif

#endif /* ACE_HAL_DEVICE_INFO_H_ */
/** @} */
