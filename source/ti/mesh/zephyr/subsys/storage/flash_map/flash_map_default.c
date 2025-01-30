/*
 * Copyright (c) 2017 Nordic Semiconductor ASA
 * Copyright (c) 2015 Runtime Inc
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifdef CONFIG_BT_SETTINGS
#include <zephyr.h>
#include <storage/flash_map.h>

void dev_page_layout(struct device *dev,
                    const struct flash_pages_layout **layout,
                    size_t *layout_size);

// Flash Area Structure
const struct flash_area default_flash_map[] =
{
 {.fa_id = DT_FLASH_AREA_STORAGE_ID,
  .fa_off = DT_FLASH_AREA_0_OFFSET,
  .fa_dev_name = DT_FLASH_AREA_0_DEV,
  .fa_size = DT_FLASH_AREA_0_SIZE,},
};

// Number of Flash Areas
const int flash_map_entries = ARRAY_SIZE(default_flash_map);

// Flash Area Structure Array
const struct flash_area *flash_map = default_flash_map;

// Device Layout
static const struct flash_pages_layout device_flash_layout =
{
 .pages_count = CONFIG_SETTINGS_NVS_PAGE_COUNT,
 .pages_size = CONFIG_SETTINGS_NVS_PAGE_SIZE
};

// Device Layout Size
static const size_t flash_area_cnt = CONFIG_SETTINGS_NVS_SECTOR_COUNT;

// Device Flash Driver API Structure
static struct flash_driver_api dev_driver_api =
{
 .page_layout = (flash_api_pages_layout)dev_page_layout,
 .write       = (flash_api_write)nvs_write,
 .erase       = (flash_api_erase)nvs_delete,
 .read        = (flash_api_read)nvs_read,
 .write_block_size = CONFIG_SETTINGS_NVS_PAGE_SIZE
};

// Top Level Device Structure
struct device dev_struct = {
  .api = (void *)&dev_driver_api,
  .config = NULL, //&dev_config;
};

void dev_page_layout(struct device *dev,
                    const struct flash_pages_layout **layout,
                    size_t *layout_size)
{
  *layout      = &device_flash_layout;
  *layout_size = flash_area_cnt;
}
#endif //CONFIG_BT_SETTINGS
