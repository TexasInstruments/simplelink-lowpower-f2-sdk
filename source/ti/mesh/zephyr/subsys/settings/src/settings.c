/*
 * Copyright (c) 2018 Nordic Semiconductor ASA
 * Copyright (c) 2015 Runtime Inc
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifdef CONFIG_BT_SETTINGS
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <errno.h>
#include <kernel.h>

#include "settings/settings_nvs.h"
#include "settings/settings.h"
#include <zephyr/types.h>

#include <logging/log.h>
LOG_MODULE_REGISTER(settings, CONFIG_SETTINGS_LOG_LEVEL);

K_MUTEX_DEFINE(settings_lock);

#ifdef __IAR_SYSTEMS_ICC__
#pragma section="settings_handler_static_area"
#endif /* __IAR_SYSTEMS_ICC__ */

extern const struct settings_handler_static settings_handler_bt_mesh;

void settings_store_init(void);

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

void settings_init(void)
{
	settings_store_init();
}

int settings_name_steq(const char *name, const char *key, const char **next)
{
	if (next) {
		*next = NULL;
	}

	if ((!name) || (!key)) {
		return 0;
	}

	/* name might come from flash directly, in flash the name would end
	 * with '=' or '\0' depending how storage is done. Flash reading is
	 * limited to what can be read
	 */

	while ((*key != '\0') && (*key == *name) &&
	       (*name != '\0') && (*name != SETTINGS_NAME_END)) {
		key++;
		name++;
	}

	if (*key != '\0') {
		return 0;
	}

	if (*name == SETTINGS_NAME_SEPARATOR) {
		if (next) {
			*next = name + 1;
		}
		return 1;
	}

	if ((*name == SETTINGS_NAME_END) || (*name == '\0')) {
		return 1;
	}

	return 0;
}

int settings_name_next(const char *name, const char **next)
{
	int rc = 0;

	if (next) {
		*next = NULL;
	}

	if (!name) {
		return 0;
	}

	/* name might come from flash directly, in flash the name would end
	 * with '=' or '\0' depending how storage is done. Flash reading is
	 * limited to what can be read
	 */
	while ((*name != '\0') && (*name != SETTINGS_NAME_END) &&
	       (*name != SETTINGS_NAME_SEPARATOR)) {
		rc++;
		name++;
	}

	if (*name == SETTINGS_NAME_SEPARATOR) {
		if (next) {
			*next = name + 1;
		}
		return rc;
	}

	return rc;
}

struct settings_handler_static *settings_parse_and_lookup(const char *name,
							const char **next)
{
	struct settings_handler_static *bestmatch;
	const char *tmpnext;

	bestmatch = NULL;
	if (next) {
		*next = NULL;
	}

	Z_STRUCT_SECTION_FOREACH(settings_handler_static, ch) {
		if (!settings_name_steq(name, ch->name, &tmpnext)) {
			continue;
		}
		if (!bestmatch) {
			bestmatch = ch;
			if (next) {
				*next = tmpnext;
			}
			continue;
		}
		if (settings_name_steq(ch->name, bestmatch->name, NULL)) {
			bestmatch = ch;
			if (next) {
				*next = tmpnext;
			}
		}
	}

#if defined(CONFIG_SETTINGS_DYNAMIC_HANDLERS)
	struct settings_handler *ch;

	SYS_SLIST_FOR_EACH_CONTAINER(&settings_handlers, ch, node) {
		if (!settings_name_steq(name, ch->name, &tmpnext)) {
			continue;
		}
		if (!bestmatch) {
			bestmatch = (struct settings_handler_static *)ch;
			if (next) {
				*next = tmpnext;
			}
			continue;
		}
		if (settings_name_steq(ch->name, bestmatch->name, NULL)) {
			bestmatch = (struct settings_handler_static *)ch;
			if (next) {
				*next = tmpnext;
			}
		}
	}
#endif /* CONFIG_SETTINGS_DYNAMIC_HANDLERS */
	return bestmatch;
}

int settings_call_set_handler(const char *name,
			      size_t len,
			      settings_read_cb read_cb,
			      void *read_cb_arg,
			      const struct settings_load_arg *load_arg)
{
	int rc;
	const char *name_key = name;

	if (load_arg && load_arg->subtree &&
	    !settings_name_steq(name, load_arg->subtree, &name_key)) {
		return 0;
	}

	if (load_arg && load_arg->cb) {
		rc = load_arg->cb(name_key, len, read_cb, read_cb_arg,
				  load_arg->param);
	} else {
		struct settings_handler_static *ch;

		ch = settings_parse_and_lookup(name, &name_key);
		if (!ch) {
			return 0;
		}

		rc = ch->h_set(name_key, len, read_cb, read_cb_arg);

		if (rc != 0) {
			LOG_ERR("set-value failure. key: %s error(%d)",
				log_strdup(name), rc);
			/* Ignoring the error */
			rc = 0;
		} else {
			LOG_DBG("set-value OK. key: %s",
				log_strdup(name));
		}
	}
	return rc;
}

int settings_commit(void)
{
	return settings_commit_subtree(NULL);
}


int settings_commit_subtree(const char *subtree)
{
	int rc;
	int rc2;

	rc = 0;
	const struct settings_handler_static *ch = &settings_handler_bt_mesh;

	if (ch->h_commit) {
		rc2 = ch->h_commit();
		if (!rc) {
			rc = rc2;
		}
	}

	return rc;
}
#endif //CONFIG_BT_SETTINGS
