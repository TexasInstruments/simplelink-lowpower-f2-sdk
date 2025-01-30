/*
 * Copyright (c) 2019 Laczen
 * Copyright (c) 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifdef CONFIG_BT_SETTINGS

#include <errno.h>
#include <string.h>

#include "settings/settings.h"
#include "settings/settings_nvs.h"
//#include "settings_priv.h"
#include <storage/flash_map.h>

#include <logging/log.h>
LOG_MODULE_DECLARE(settings, CONFIG_SETTINGS_LOG_LEVEL);

typedef struct
{
  uint8_t state;    // page state
  uint8_t cycle;    // page compaction cycle count. Used to select the 'newest' active page
                    // at device reset, in the very unlikely scenario that both pages are active.
  uint8_t mode;     // compact mode
  uint8_t allActive;  //all items are active or not
  uint8_t sPage;
  uint8_t ePage;
  uint16_t offset;  // page offset
  uint16_t sOffset;
  uint16_t eOffset;
} NVOCMP_pageInfo_t;

typedef struct
{
  uint8_t xDstPage;         // xdst page
  uint8_t xSrcSPage;    // xsrc start page
  uint8_t xSrcEPage;      // xsrc end page
  uint8_t xSrcPages;    // no of xsrc pages
  uint16_t xDstOffset;      // xdst offset
  uint16_t xSrcSOffset;    // xsrc start offset
  uint16_t xSrcEOffset;    // xsrc end offset
} NVOCMP_compactInfo_t;

typedef struct
{
  uint8_t nvSize;       // no of NV pages
  uint8_t headPage;     // head active page
  uint8_t tailPage;     // transfer destination page
  uint8_t actPage;      // current active page
  uint8_t xsrcPage;     // transfer source page
  uint8_t forceCompact; // force compaction to happen
  uint16_t actOffset;   // active page offset
  uint16_t xsrcOffset;  // transfer source page offset
  uint16_t xdstOffset;  // transfer destination page offset
  NVOCMP_compactInfo_t compactInfo;
  NVOCMP_pageInfo_t pageInfo[2];
} NVOCMP_nvHandle_t;

extern NVOCMP_nvHandle_t NVOCMP_nvHandle;
//static struct settings_nvs default_settings_nvs;

struct settings_nvs_read_fn_arg {
	struct nvs_fs *fs;
	uint16_t id;
};

static int settings_nvs_load(struct settings_store *cs,
			     const struct settings_load_arg *arg);
static int settings_nvs_save(struct settings_store *cs, const char *name,
			     const char *value, size_t val_len);

static struct settings_store_itf settings_nvs_itf = {
	.csi_load = settings_nvs_load,
	.csi_save = settings_nvs_save,
};

static ssize_t settings_nvs_read_fn(void *back_end, void *data, size_t len)
{
	struct settings_nvs_read_fn_arg *rd_fn_arg;
	ssize_t rc;

	rd_fn_arg = (struct settings_nvs_read_fn_arg *)back_end;

	rc = nvs_read(rd_fn_arg->fs, rd_fn_arg->id, data, len);
	if (rc > (ssize_t)len) {
		/* nvs_read signals that not all bytes were read
		 * align read len to what was requested
		 */
		rc = len;
	}
	return rc;
}

int settings_nvs_src(struct settings_nvs *cf)
{
	cf->cf_store.cs_itf = &settings_nvs_itf;
	settings_src_register(&cf->cf_store);

	return 0;
}

int settings_nvs_dst(struct settings_nvs *cf)
{
	cf->cf_store.cs_itf = &settings_nvs_itf;
	settings_dst_register(&cf->cf_store);

	return 0;
}

static int settings_nvs_load(struct settings_store *cs,
			     const struct settings_load_arg *arg)
{
	int ret = 0;
	struct settings_nvs *cf = (struct settings_nvs *)cs;
	struct settings_nvs_read_fn_arg read_fn_arg;
	char name[SETTINGS_MAX_NAME_LEN + SETTINGS_EXTRA_LEN + 1];
	char buf[SETTINGS_MAX_NAME_LEN + SETTINGS_EXTRA_LEN + 1];
	ssize_t rc1, rc2;
	uint16_t name_id = NVS_NAMECNT_ID;

	name_id = cf->last_name_id + 1;

	while (1) {

		name_id--;
		if (name_id == NVS_NAMECNT_ID) {
			break;
		}

		/* In the NVS backend, each setting item is stored in two NVS
		 * entries one for the setting's name and one with the
		 * setting's value.
		 */
		rc1 = nvs_read(&cf->cf_nvs, name_id, &name, sizeof(name));
		rc2 = nvs_read(&cf->cf_nvs, name_id + NVS_NAME_ID_OFFSET,
			       &buf, sizeof(buf));

		if ((rc1 <= 0) && (rc2 <= 0)) {
			continue;
		}

		if ((rc1 <= 0) || (rc2 <= 0)) {
			/* Settings item is not stored correctly in the NVS.
			 * NVS entry for its name or value is either missing
			 * or deleted. Clean dirty entries to make space for
			 * future settings item.
			 */
			if (name_id == cf->last_name_id) {
				cf->last_name_id--;
				nvs_write(&cf->cf_nvs, NVS_NAMECNT_ID,
					  &cf->last_name_id, sizeof(uint16_t));
			}
			nvs_delete(&cf->cf_nvs, name_id);
			nvs_delete(&cf->cf_nvs, name_id + NVS_NAME_ID_OFFSET);
			continue;
		}

		/* Found a name, this might not include a trailing \0 */
		name[rc1] = '\0';
		read_fn_arg.fs = &cf->cf_nvs;
		read_fn_arg.id = name_id + NVS_NAME_ID_OFFSET;

		ret = settings_call_set_handler(
			name, rc2,
			settings_nvs_read_fn, &read_fn_arg,
			(void *)arg);
		if (ret) {
			break;
		}
	}
	return ret;
}

static int settings_nvs_save(struct settings_store *cs, const char *name,
			     const char *value, size_t val_len)
{
	struct settings_nvs *cf = (struct settings_nvs *)cs;
	char rdname[SETTINGS_MAX_NAME_LEN + SETTINGS_EXTRA_LEN + 1];
	uint16_t name_id, write_name_id;
	bool delete, write_name;
	int rc = 0;

	if (!name) {
		return -EINVAL;
	}

	/* Find out if we are doing a delete */
	delete = ((value == NULL) || (val_len == 0));

	name_id = cf->last_name_id + 1;
	write_name_id = cf->last_name_id + 1;
	write_name = true;

	while (1) {
		name_id--;
		if (name_id == NVS_NAMECNT_ID) {
			break;
		}

		rc = nvs_read(&cf->cf_nvs, name_id, &rdname, sizeof(rdname));

		if (rc < 0) {
			/* Error or entry not found */
			if (rc == -ENOENT) {
				write_name_id = name_id;
			}
			continue;
		}

		rdname[rc] = '\0';

		if (strcmp(name, rdname)) {
			continue;
		}

		if ((delete) && (name_id == cf->last_name_id)) {
			cf->last_name_id--;
			rc = nvs_write(&cf->cf_nvs, NVS_NAMECNT_ID,
				       &cf->last_name_id, sizeof(uint16_t));
			if (rc < 0) {
				/* Error: can't to store
				 * the largest name ID in use.
				 */
				return rc;
			}
		}

		if (delete) {
			rc = nvs_delete(&cf->cf_nvs, name_id);

			if (rc >= 0) {
				rc = nvs_delete(&cf->cf_nvs, name_id +
					NVS_NAME_ID_OFFSET);
			}

			if (rc < 0) {
				return rc;
			}

			return 0;
		}
		write_name_id = name_id;
		write_name = false;
		break;
	}

	if (delete) {
		return 0;
	}

	/* No free IDs left. */
	if (write_name_id == NVS_NAMECNT_ID + NVS_NAME_ID_OFFSET) {
		return -ENOMEM;
	}

	/* write the value */
	rc = nvs_write(&cf->cf_nvs, write_name_id + NVS_NAME_ID_OFFSET,
		       value, val_len);
	if (rc < 0) {
		return rc;
	}

	/* write the name if required */
	if (write_name) {
		rc = nvs_write(&cf->cf_nvs, write_name_id, name, strlen(name));
		if (rc < 0) {
			return rc;
		}
	}

	/* update the last_name_id and write to flash if required*/
	if (write_name_id > cf->last_name_id) {
		cf->last_name_id = write_name_id;
		rc = nvs_write(&cf->cf_nvs, NVS_NAMECNT_ID, &cf->last_name_id,
			       sizeof(uint16_t));
	}

	if (rc < 0) {
		return rc;
	}

	return 0;
}

/* Initialize the nvs backend. */
int settings_nvs_backend_init(struct settings_nvs *cf)
{
	int rc;
	uint16_t last_name_id;

	rc = nvs_init(&cf->cf_nvs, cf->flash_dev_name);
	if (rc) {
		return rc;
	}

	rc = nvs_read(&cf->cf_nvs, NVS_NAMECNT_ID, &last_name_id,
		      sizeof(last_name_id));
	if (rc < 0) {
		cf->last_name_id = NVS_NAMECNT_ID;
	} else {
		cf->last_name_id = last_name_id;
	}

	LOG_DBG("Initialized");
	return 0;
}

int settings_backend_init(void)
{
	static struct settings_nvs default_settings_nvs;
	int rc;
	uint16_t cnt = 0;
	size_t nvs_sector_size, nvs_size = 0;
	const struct flash_area *fa;
	struct flash_sector hw_flash_sector;
	uint32_t sector_cnt = 1;

	rc = flash_area_open(DT_FLASH_AREA_STORAGE_ID, &fa);
	if (rc) {
		return rc;
	}

	rc = flash_area_get_sectors(DT_FLASH_AREA_STORAGE_ID, &sector_cnt,
				    &hw_flash_sector);
	if (rc == -ENODEV) {
		return rc;
	} else if (rc != 0 && rc != -ENOMEM) {
		k_panic();
	}

	nvs_sector_size = CONFIG_SETTINGS_NVS_SECTOR_SIZE_MULT *
			  hw_flash_sector.fs_size;

	if (nvs_sector_size > UINT16_MAX) {
		return -EDOM;
	}

	while (cnt < CONFIG_SETTINGS_NVS_SECTOR_COUNT) {
		nvs_size += nvs_sector_size;
		if (nvs_size > fa->fa_size) {
			break;
		}
		cnt++;
	}

	/* define the nvs file system using the page_info */
	default_settings_nvs.cf_nvs.sector_size = nvs_sector_size;
	default_settings_nvs.cf_nvs.sector_count = cnt;
	default_settings_nvs.cf_nvs.offset = fa->fa_off + NVOCMP_nvHandle.pageInfo[0].offset;
	default_settings_nvs.flash_dev_name = fa->fa_dev_name;

	rc = settings_nvs_backend_init(&default_settings_nvs);
	if (rc) {
		return rc;
	}

	rc = settings_nvs_src(&default_settings_nvs);

	if (rc) {
		return rc;
	}

	rc = settings_nvs_dst(&default_settings_nvs);

	return rc;
}

#endif /* CONFIG_BT_SETTINGS */
