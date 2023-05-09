/*
 * Copyright (c) 2017 Nordic Semiconductor ASA
 * Copyright (c) 2015 Runtime Inc
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Public API for flash map
 */

#ifndef ZEPHYR_INCLUDE_STORAGE_FLASH_MAP_H_
#define ZEPHYR_INCLUDE_STORAGE_FLASH_MAP_H_

/**
 * @brief Abstraction over flash partitions/areas and their drivers
 *
 * @defgroup flash_area_api flash area Interface
 * @{
 */

/*
 * This API makes it possible to operate on flash areas easily and
 * effectively.
 *
 * The system contains global data about flash areas. Every area
 * contains an ID number, offset, and length.
 */

/**
 *
 */
#include <zephyr/types.h>
#include <stddef.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Provided for compatibility with MCUboot */
#define SOC_FLASH_0_ID 0
/** Provided for compatibility with MCUboot */
#define SPI_FLASH_0_ID 1

/**
 * @brief Flash partition
 *
 * This structure represents a fixed-size partition on a flash device.
 * Each partition contains one or more flash sectors.
 */
struct flash_area {
	/** ID number */
	uint8_t fa_id;
	/** Provided for compatibility with MCUboot */
	uint8_t fa_device_id;
	uint16_t pad16;
	/** Start offset from the beginning of the flash device */
	off_t fa_off;
	/** Total size */
	size_t fa_size;
	/**
	 * Name of the flash device, suitable for passing to
	 * device_get_binding().
	 */
	const char *fa_dev_name;
};

/**
 * @brief Structure for transfer flash sector boundaries
 *
 * This template is used for presentation of flash memory structure. It
 * consumes much less RAM than @ref flash_area
 */
struct flash_sector {
	/** Sector offset from the beginning of the flash device */
	off_t fs_off;
	/** Sector size in bytes */
	size_t fs_size;
};

/**
 * @brief Retrieve partitions flash area from the flash_map.
 *
 * Function Retrieves flash_area from flash_map for given partition.
 *
 * @param[in]  id ID of the flash partition.
 * @param[out] fa Pointer which has to reference flash_area. If
 * @p ID is unknown, it will be NULL on output.
 *
 * @return  0 on success, -EACCES if the flash_map is not available ,
 * -ENOENT if @p ID is unknown.
 */
int flash_area_open(uint8_t id, const struct flash_area **fa);

/**
 * Retrieve info about sectors within the area.
 *
 * @param[in]  fa_id    Given flash area ID
 * @param[out] sectors  buffer for sectors data
 * @param[in,out] count On input Capacity of @p sectors, on output number of
 * sectors Retrieved.
 *
 * @return  0 on success, negative errno code on fail. Especially returns
 * -ENOMEM if There are too many flash pages on the flash_area to fit in the
 * array.
 */
int flash_area_get_sectors(int fa_id, uint32_t *count,
			   struct flash_sector *sectors);

/**
 * Flash map iteration callback
 *
 * @param fa flash area
 * @param user_data User supplied data
 *
 */
typedef void (*flash_area_cb_t)(const struct flash_area *fa,
				void *user_data);

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif /* ZEPHYR_INCLUDE_STORAGE_FLASH_MAP_H_ */
