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
* @file    hal_block_device.h
* @brief   Block Device HAL provides an interface for access to block-based
*          storage
* @details Block Device HAL APIs provide the basic read, write and erase I/O
*          operations on a block-based storage.
* @addtogroup ACE_HAL_BLOCK_DEVICE
*
* DEFINITION
* -----------
*
* block device: a bounded continuous byte addressable block-based I/O storage
*
* @{
*/

#ifndef ACE_HAL_BLOCK_DEVICE_H_
#define ACE_HAL_BLOCK_DEVICE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <ace/ace_status.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/**
 * @name Max number of chars in block device name, excluding trailing \0.
 */
#define ACE_BLOCK_DEVICE_NAME_MAX 16

/**
 * @brief Block device information.
 */
typedef struct {
    /** Device name. Must be unique among all block devices */
    char name[ACE_BLOCK_DEVICE_NAME_MAX + 1];

    /** Device storage size in bytes */
    uint32_t size;

    /** Size of a block in bytes */
    uint32_t block_size;

    /** If async mode is supported */
    bool async_supported;
} aceBlockDeviceHal_deviceInfo_t;

/**
 * @brief Block device handle.
 */
typedef void* aceBlockDeviceHal_handle_t;

/**
 * @brief Get the @ref aceBlockDeviceHal_handle_t handle for a device from
 *        device name.
 *
 * @param[in] name The name of the device to get
 *
 * @return a valid @ref aceBlockDeviceHal_handle_t on success or NULL on failure
 */
aceBlockDeviceHal_handle_t aceBlockDeviceHal_getDevice(const char* name);

/**
 * @brief Get device info.
 *
 * @param[in]  hdl  The handle to the device
 * @param[out] info The info of the device
 *
 * @return ACE_STATUS_OK on success or a negative error code from @ref
 *         ace_status_t on failure
 */
ace_status_t aceBlockDeviceHal_getDeviceInfo(
    aceBlockDeviceHal_handle_t hdl, aceBlockDeviceHal_deviceInfo_t* info);

/**
 * @brief Read from a device.
 *
 * @param[in]  hdl  The handle to the device to perform the read on
 * @param[in]  from The offset from device base address to read data from
 * @param[in]  len  Number of bytes to read. Max allowed value is INT32_MAX
 * @param[out] buf  Buffer to hold the read data
 *
 * @return number of bytes read or a negative error code from @ref ace_status_t
 */
int32_t aceBlockDeviceHal_read(aceBlockDeviceHal_handle_t hdl, uint32_t from,
                               uint32_t len, uint8_t* buf);

/**
 * @brief Write to a device.
 *
 * @param[in] hdl The handle to the device to perform the write on
 * @param[in] to  The offset from device base address to write data to
 * @param[in] len Number of bytes to write. Max allowed value is INT32_MAX
 * @param[in] buf Holds the data to write
 *
 * @return number of bytes written or a negative error code from @ref
 *         ace_status_t
 */
int32_t aceBlockDeviceHal_write(aceBlockDeviceHal_handle_t hdl, uint32_t to,
                                uint32_t len, const uint8_t* buf);

/**
 * @brief Erase a device synchronously.
 *
 * @param[in] hdl The handle to the device to perform the erase on
 *
 * @return ACE_STATUS_OK on success or a negative error code from @ref
 *         ace_status_t
 */
ace_status_t aceBlockDeviceHal_eraseAll(aceBlockDeviceHal_handle_t hdl);

/**
 * @brief Erase a region (number of blocks) synchronously.
 *
 * @param[in] hdl    The handle to the device to perform the erase on
 * @param[in] offset The offset within the device you want to start
 *                   erasing from. The offset must be aligned to block size
 * @param[in] len    The size of the area you want to erase.
 *                   @ref len must be aligned to block size.
 *
 * @return ACE_STATUS_OK on success or a negative error code from @ref
 *         ace_status_t
 */
ace_status_t aceBlockDeviceHal_eraseBlocks(aceBlockDeviceHal_handle_t hdl,
                                           size_t offset, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* ACE_HAL_BLOCK_DEVICE_H_ */
/** @} */
