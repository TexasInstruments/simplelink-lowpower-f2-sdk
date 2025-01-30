/*
 * Copyright (c) 2017 Nordic Semiconductor ASA
 * Copyright (c) 2015 Runtime Inc
 * Copyright (c) 2017 Linaro Ltd
 * Copyright (c) 2020 Gerson Fernando Budke <nandojve@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifdef CONFIG_BT_SETTINGS
#include <zephyr/types.h>
#include <stddef.h>
#include <sys/types.h>
#include <device.h>
#include <storage/flash_map.h>
#include <drivers/flash.h>
#include <zephyr.h>


struct layout_data {
	uint32_t area_idx;
	uint32_t area_off;
	uint32_t area_len;
	void *ret;        /* struct flash_area* or struct flash_sector* */
	uint32_t ret_idx;
	uint32_t ret_len;
	int status;
};

extern const struct flash_area *flash_map;
extern const int flash_map_entries;

static struct flash_area const *get_flash_area_from_id(int idx)
{
	for (int i = 0; i < flash_map_entries; i++) {
		if (flash_map[i].fa_id == idx) {
			return &flash_map[i];
		}
	}

	return NULL;
}

/*
 * Check if a flash_page_foreach() callback should exit early, due to
 * one of the following conditions:
 *
 * - The flash page described by "info" is before the area of interest
 *   described in "data"
 * - The flash page is after the end of the area
 * - There are too many flash pages on the device to fit in the array
 *   held in data->ret. In this case, data->status is set to -ENOMEM.
 *
 * The value to return to flash_page_foreach() is stored in
 * "bail_value" if the callback should exit early.
 */
static bool should_bail(const struct flash_pages_info *info,
						struct layout_data *data,
						bool *bail_value)
{
	if (info->start_offset < data->area_off) {
		*bail_value = true;
		return true;
	} else if (info->start_offset >= data->area_off + data->area_len) {
		*bail_value = false;
		return true;
	} else if (data->ret_idx >= data->ret_len) {
		data->status = -ENOMEM;
		*bail_value = false;
		return true;
	}

	return false;
}


// Iterate over all flash pages on a device
static void flash_page_foreach(const struct device *dev, flash_page_cb cb, void *data)
{
  const struct flash_driver_api *api = dev->api;
  const struct flash_pages_layout *layout;
  struct flash_pages_info page_info;
  size_t block, num_blocks, page = 0, i;
  off_t off = 0;

  api->page_layout(dev, &layout, &num_blocks);

  for (block = 0; block < num_blocks; block++) {
    const struct flash_pages_layout *l = &layout[block];
    page_info.size = l->pages_size;

    for (i = 0; i < l->pages_count; i++) {
      page_info.start_offset = off;
      page_info.index = page;

      if (!cb(&page_info, data)) {
        return;
      }

      off += page_info.size;
      page++;
    }
  }
}

/*
 * Generic page layout discovery routine. This is kept separate to
 * support both the deprecated flash_area_to_sectors() and the current
 * flash_area_get_sectors(). A lot of this can be inlined once
 * flash_area_to_sectors() is removed.
 */
static int flash_area_layout(int idx, uint32_t *cnt, void *ret,
flash_page_cb cb, struct layout_data *cb_data)
{
	const struct device *flash_dev;

	cb_data->area_idx = idx;

	const struct flash_area *fa;

	fa = get_flash_area_from_id(idx);
	if (fa == NULL) {
		return -EINVAL;
	}

	cb_data->area_idx = idx;
	cb_data->area_off = fa->fa_off;
	cb_data->area_len = fa->fa_size;

	cb_data->ret = ret;
	cb_data->ret_idx = 0U;
	cb_data->ret_len = *cnt;
	cb_data->status = 0;

	flash_dev = device_get_binding(fa->fa_dev_name);
	if (flash_dev == NULL) {
		return -ENODEV;
	}

	flash_page_foreach(flash_dev, cb, cb_data);

	if (cb_data->status == 0) {
		*cnt = cb_data->ret_idx;
	}

	return cb_data->status;
}

static bool get_sectors_cb(const struct flash_pages_info *info, void *datav)
{
	struct layout_data *data = datav;
	struct flash_sector *ret = data->ret;
	bool bail;

	if (should_bail(info, data, &bail)) {
		return bail;
	}

	ret[data->ret_idx].fs_off = info->start_offset - data->area_off;
	ret[data->ret_idx].fs_size = info->size;
	data->ret_idx++;

	return true;
}


int flash_area_open(uint8_t id, const struct flash_area **fap)
{
	const struct flash_area *area;

	if (flash_map == NULL) {
		return -EACCES;
	}

	area = get_flash_area_from_id(id);
	if (area == NULL) {
		return -ENOENT;
	}

	*fap = area;

	return 0;
}

int flash_area_get_sectors(int idx, uint32_t *cnt, struct flash_sector *ret)
{
	struct layout_data data;
	int err = flash_area_layout(idx, cnt, ret, get_sectors_cb, &data);
	return err;
}
#endif //CONFIG_BT_SETTINGS
