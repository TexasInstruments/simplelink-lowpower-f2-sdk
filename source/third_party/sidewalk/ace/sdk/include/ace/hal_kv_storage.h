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
* @file hal_kv_storage.h
* @brief Key-Value Storage (KVS) HAL provides an interface to persistently store
* and retrieve data on the target platform.
* @addtogroup ACE_HAL_KV_STORAGE
*
* Glossary
* ----------
*  Key - a null-terminated string used to save and retrieve data associated with
* it. The format of the key is defined below.
*
*  Data entry - the combination of the key and data associated with the key.
*
*  Group - container for data entry or entries with the same group name.
*  * Multiple groups are allowed in one system.
*  * One default group must be defined in the system.
*  * Group names are unique within a system.
*
* Key format
* ----------
*
* KVS keys have the format of X.Y where X is the group name and Y is the sub-key
* name.
* If X.Y key format is provided to KVS HAL APIs, it assumes that user would like
* to access the sub-key Y in group X.
* If no group name is specified, the assumption is that the sub-key Y is to be
* accessed from default group (pre-defined).
* Note that, the entire string after the first '.' should be treated as the
* sub-key name.
* For example "somegroup.somekey_subkey", "somegroup" is the group name and
* "somekey_subkey" should be treated as the sub-key name.
* No "."(dot) is allowed in group key and sub-key name.
*
* @note The length limit of KVS group name is ACE_KV_STORAGE_HAL_MAX_GROUP_LEN.
* @note The length limit of KVS key name is ACE_KV_STORAGE_HAL_MAX_KEY_LEN.
*
* @{
*/

#ifndef ACE_HAL_KV_STORAGE_H_
#define ACE_HAL_KV_STORAGE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <ace/ace_config.h>
#include <stdint.h>
#include <stddef.h>
#include <ace/ace_status.h>

/**
 * @cond DEPRECATED
 * @deprecated Please use the new symbols.
 * @{
 */
#define aceKeyValueDsHal_db_init aceKeyValueDsHal_init
/**
 * @}
 * @endcond
 */

/** The maximum length of key name */
#define ACE_KV_STORAGE_HAL_MAX_KEY_LEN 48

/** The maximum length of group name */
#define ACE_KV_STORAGE_HAL_MAX_GROUP_LEN 16

/**
 * The maximum value length that ACE will try to store.
 * KVS implementation must at least support value stores of this size.
 */
#define ACE_KV_STORAGE_HAL_MAX_VAL_LEN 7680

/**
 * @brief Define shutdown reason
 * LCR (Life Cycle Reason) need to read shutdown reason through KVS.
 */
#define ACE_KV_STORAGE_KEY_SHUTDOWN_REASON "shutdown_reason"

/** The default KVS group name */
#ifndef ACE_KVS_DS_DEFAULT_GROUP
#error Please define ACONF_ACE_KVS_DS_DEFAULT_GROUP in ACE config file. \
       Default KVS data storage group is a KVS group for all the KVS data \
       that have no group name defined.
#endif

/**
 * @brief Call back used when aceKeyValueDsHal_listKeys API get each key name.
 *
 * @param key_name[in]: Pointer to the current key name string filled by
 *                       aceKeyValueDsHal_listKeys.
 * @return Return 0 on success. Return -1 if caller want to stop the iteration.
 */
typedef int (*aceKeyValueDsHal_listKeys_cb)(const char* key_name);

/**
 * @brief Perform KVS initialization.
 *        This API can be called during the system boot up to configure KVS if
 * needed.
 *        For example, this API can be used to config storage or filesystem for
 * low level KVS implementation.
 *
 * @param[in] name: Pointer to a comma separated string for the group names.
 *
 */
void aceKeyValueDsHal_init(const char* name);

/**
 * @brief Set (save) data to storage for the key.
 *        A new data entry will be created if the key does not exist, or
 *        the existing data will be overwritten.
 *
 * @param[in] key: Pointer to a null-terminated string for the key.
 * @param[in] value: Pointer to the data to be set.
 * @param[in] len: Length of the data to be set.
 * @return Return ACE_STATUS_OK if success, or negative
 * ace_status_t error code.
 * @retval ACE_STATUS_BAD_PARAM invalid key
 * @retval ACE_STATUS_BAD_PARAM value pointer is NULL
 * @retval ACE_STATUS_BAD_PARAM if data size is bigger than
 * ACE_KV_STORAGE_HAL_MAX_VAL_LEN
 * @retval ACE_STATUS_NOT_FOUND group does not exist
 * @retval ACE_STATUS_OUT_OF_MEMORY if no space to hold the data in the group
 * or other negative ace_status_t error code
 */
ace_status_t aceKeyValueDsHal_set(const char* key, const void* value,
                                  const size_t len);

/**
 * @brief Set (save) data to storage for the key with hardware-backed
 * encryption.
 *
 * @param[in] key: Pointer to a null-terminated string for the key.
 * @param[in] value: Pointer to the data to be set
 * @param[in] len: Length of the data to be set.
 * @return Return ACE_STATUS_OK if successful, or negative
 * ace_status_t error code.
 * @retval ACE_STATUS_BAD_PARAM invalid key
 * @retval ACE_STATUS_BAD_PARAM value pointer is NULL
 * @retval ACE_STATUS_BAD_PARAM if data size is bigger than
 * ACE_KV_STORAGE_HAL_MAX_VAL_LEN
 * @retval ACE_STATUS_NOT_FOUND group does not exist
 * @retval ACE_STATUS_OUT_OF_MEMORY if no space to hold the data in the group
 * or other negative ace_status_t error code
 */
ace_status_t aceKeyValueDsHal_setWithEncryption(const char* key,
                                                const void* value,
                                                const size_t len);

/**
 * @brief Remove (clear) data associated with the key.
 *
 * @param[in] key: Pointer to a null-terminated string for the key.
 * @return ace_status_t: Return ACE_STATUS_OK if the operation succeeds or
 * the sub-key does not exist within an existing group, or negative
 * ace_status_t error code.
 * @retval ACE_STATUS_BAD_PARAM invalid key
 * @retval ACE_STATUS_NOT_FOUND group does not exist
 * or other negative ace_status_t error code
 */
ace_status_t aceKeyValueDsHal_remove(const char* key);

/**
 * @brief Get (read) data associated with the key name.
 *
 * @param[in] key: Pointer to a null-terminated string for the key.
 * @param[in,out] value: Pointer to the data buffer to read in the data.
 *                       The data buffer will not be cleared by KVS before read.
 * @param[in] len: Length of the data buffer.
 * @return Return the length of data (in bytes) if success, or negative
 * ace_status_t error code.
 * @retval ACE_STATUS_BAD_PARAM invalid key
 * @retval ACE_STATUS_BAD_PARAM value pointer is NULL
 * @retval ACE_STATUS_NOT_FOUND key does not exist
 * @retval ACE_STATUS_BUFFER_OVERFLOW content output is smaller than the
 * data entry data_size
 * or other negative ace_status_t error code
 */
int aceKeyValueDsHal_get(const char* key, void* const value, const size_t len);

/**
 * @brief Get data size associated with the key.
 *
 * @param[in] key: Pointer to a null-terminated string for the key.
 * @return Return the length of data (in bytes) if success, or negative
 * ace_status_t error code.
 * @retval ACE_STATUS_BAD_PARAM invalid key
 * @retval ACE_STATUS_NOT_FOUND key does not exist
 * or other negative ace_status_t error code
 */
int aceKeyValueDsHal_getValueSize(const char* key);

/**
 * @brief Erase (clean) a KVS group.
 *        All the Key-Value pairs will be erased within the group.
 *
 * @param[in] group_name: Pointer to a null-terminated string for the group
 * name.
 * @return Return ACE_STATUS_OK if success, or negative ace_status_t error code.
 * @retval ACE_STATUS_BAD_PARAM invalid group name
 * @retval ACE_STATUS_NOT_FOUND group does not exist
 * or other negative ace_status_t error code
 */
ace_status_t aceKeyValueDsHal_eraseGroup(const char* group_name);

/**
 * @brief List key names within a KVS group.
 *
 * Caller needs to provide a callback (aceKeyValueDsHal_listKeys_cb type).
 * This is a synchronous call.
 * The callback will NOT be triggered after the function returns.
 *
 * Example:
 * @code{.c}
 * // Define the callback function
 * static int listKeys_cb(const char* key_name) {
 *     printf("%s\n", key_name);
 *     return 0;
 * }
 * // Call aceKeyValueDsHal_listKeys function with callback specified
 * ace_status_t rc = aceKeyValueDsHal_listKeys(group_name, &listKeys_cb);
 * @endcode
 *
 * @param group_name[in]: Pointer to string for the group name.
 * @param list_keys_cb[in]: Callback used when aceKeyValueDsHal_listKeys API
 * gets each key name.
 * @return Return ACE_STATUS_OK if successful, or negative ace_status_t
 * error code.
 * @retval ACE_STATUS_BAD_PARAM invalid group name
 * @retval ACE_STATUS_BAD_PARAM list_keys_cb is invalid callback
 * @retval ACE_STATUS_NOT_FOUND group with the name does not exist
 * or other negative ace_status_t error code
 */
ace_status_t aceKeyValueDsHal_listKeys(
    const char* group_name, aceKeyValueDsHal_listKeys_cb list_keys_cb);

#ifdef __cplusplus
}
#endif

#endif /* ACE_HAL_KV_STORAGE_H_ */
/** @} */
