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
 * @file hal_circularDs.h
 * @brief ACS CircularDs HAL implements a circular buffer based storage.
 * @addtogroup ACE_HAL_CIRCULARDS
 * @{
 */

#ifndef ACE_HAL_CIRCULAR_DS_H_
#define ACE_HAL_CIRCULAR_DS_H_

#include <ace/ace_status.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Structure for a CircularDs storage.
 * @note @ref aceCircularDs_s needs to be defined by the implementation.
 */
typedef struct aceCircularDs_s aceCircularDs_t;

/**
 * @brief Information provided by aceCircularDsHal_getInfo.
 */
typedef struct {
    /** Total space in bytes */
    uint32_t total_space_in_bytes;
    /** Available space in bytes */
    uint32_t available_space_in_bytes;
} aceCircularDsInfo_t;

/**
 * @brief Enum types used by aceCircularDsHal_sync.
 */
typedef enum {
    /** Reading is completed */
    ACE_CIRCULARDS_READ_COMPLETE = 0,
    /** Writing is completed */
    ACE_CIRCULARDS_WRITE_COMPLETE = 1
} aceCircularDsSync_t;

/**
 * @brief Create a CircularDs instance based on a storage.
 * @param[in] name storage name
 * @return storage handle or NULL on failure
 * @note This is a thread-safe API
 */
aceCircularDs_t* aceCircularDsHal_open(const char* name);

/**
 * @brief Close a CircularDs instance.
 * This does not erase any data in the storage, but only frees up memory.
 * @param[in] storage storage handle
 * @return ACE_STATUS_OK on success, else one of the error codes in @ref
 * ace_status_t
 * @note This is a thread-safe API.
 */
ace_status_t aceCircularDsHal_close(aceCircularDs_t* storage);

/**
 * @brief Read data from the current unread position.
 * Call @ref aceCircularDsHal_sync with @ref ACE_CIRCULARDS_READ_COMPLETE to
 * mark the data entry as read and move the unread position forward.
 * @param[in]     storage storage handle returned by @ref aceCircularDsHal_open
 * @param[out]    buf     pointer to the buffer where data is read
 * @param[in/out] len     bytes to read (it returns actual read bytes)
 * @return ACE_STATUS_OK on success, else one of the error codes in @ref
 * ace_status_t
 * @note This is a thread-safe API.
 */
ace_status_t aceCircularDsHal_getData(aceCircularDs_t* storage, void* buf,
                                      uint32_t* len);

/**
 * @brief Write data to the current write position.
 * Call @ref aceCircularDsHal_sync with @ref ACE_CIRCULARDS_WRITE_COMPLETE after
 * a data entry is written.
 * @param[in]     storage storage handle returned by @ref aceCircularDsHal_open
 * @param[in]     buf     pointer to the buffer where data is to be written
 * @param[in/out] len     bytes to write (it returns bytes actually written)
 * @return ACE_STATUS_OK on success, else one of the error codes in @ref
 * ace_status_t
 * @note This is a thread-safe API.
 */
ace_status_t aceCircularDsHal_setData(aceCircularDs_t* storage, const void* buf,
                                      uint32_t len);

/**
 * @brief Mark the current data entry in the storage as read/write complete.
 * Users should call this API with type @ref ACE_CIRCULARDS_READ_COMPLETE (after
 * aceCircularDsHal_getData) or @ref ACE_CIRCULARDS_WRITE_COMPLETE (after
 * aceCircularDsHal_setData) once they are done reading/writing.
 * @param[in] storage storage handle returned by @ref aceCircularDsHal_open
 * @param[in] type    read/write complete enum @ref aceCircularDsSync_t
 * @return ACE_STATUS_OK on success, else one of the error codes in @ref
 * ace_status_t
 * @note This is a thread-safe API.
 */
ace_status_t aceCircularDsHal_sync(aceCircularDs_t* storage,
                                   aceCircularDsSync_t type);

/**
 * @brief Get information (total space and available space) about a storage.
 * @param[in]  storage storage handle returned by @ref aceCircularDsHal_open
 * @param[out] info    info @ref aceCircularDsInfo_t about the storage
 * @return ACE_STATUS_OK on success, else one of the error codes in @ref
 * ace_status_t
 * @note This is a thread-safe API.
 */
ace_status_t aceCircularDsHal_getInfo(aceCircularDs_t* storage,
                                      aceCircularDsInfo_t* info);

/**
 * @brief Reclaim space for a storage by erasing all previously read data.
 * This works if there is enough data to be cleaned up i.e a block for block
 * based devices.
 * @param[in] storage storage handle returned by @ref aceCircularDsHal_open
 * @return ACE_STATUS_OK on success, else one of the error codes in @ref
 * ace_status_t
 * @note This is a thread-safe API.
 */
ace_status_t aceCircularDsHal_reclaim(aceCircularDs_t* storage);

/**
 * @brief Erase all data from a storage.
 * @param[in] storage storage handle returned by @ref aceCircularDsHal_open
 * @return ACE_STATUS_OK on success, else one of the error codes in @ref
 * ace_status_t
 * @note This is a thread-safe API. Costly operation and should be used
 * sparingly.
 */
ace_status_t aceCircularDsHal_erase(aceCircularDs_t* storage);

#ifdef __cplusplus
}
#endif

#endif /* ACE_HAL_CIRCULAR_DS_H_ */
/** @} */
