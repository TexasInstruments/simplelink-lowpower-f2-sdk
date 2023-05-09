
#ifdef CONFIG_BT_SETTINGS
#include <string.h>
#include <stdio.h>

#include <zephyr/types.h>
#include <stddef.h>
#include <sys/types.h>
#include <errno.h>
#include <kernel.h>

#include "settings/settings.h"

#include <logging/log.h>
#include "sys/__assert.h"

#ifdef __IAR_SYSTEMS_ICC__
#pragma section="settings_handler_static_area"
#endif /* __IAR_SYSTEMS_ICC__ */

LOG_MODULE_DECLARE(settings, CONFIG_SETTINGS_LOG_LEVEL);

sys_slist_t settings_load_srcs;
struct settings_store *settings_save_dst;
extern struct k_mutex settings_lock;

void settings_src_register(struct settings_store *cs)
{
	sys_slist_append(&settings_load_srcs, &cs->cs_next);
}

void settings_dst_register(struct settings_store *cs)
{
	settings_save_dst = cs;
}

int settings_load(void)
{
	return settings_load_subtree(NULL);
}

int settings_load_subtree(const char *subtree)
{
	struct settings_store *cs;
	int rc;
	const struct settings_load_arg arg = {
		.subtree = subtree
	};

	/*
	 * for every config store
	 *    load config
	 *    apply config
	 *    commit all
	 */
	k_mutex_lock(&settings_lock, K_FOREVER);
#if defined(__IAR_SYSTEMS_ICC__)
	for (cs = ((sys_slist_peek_head(&settings_load_srcs)) ? ((struct settings_store *)(((char *)((sys_slist_peek_head(&settings_load_srcs)))) - ((size_t)__INTADDR__(&(((struct settings_store *)0)->cs_next))))) : 0);
	     cs != 0;
	     cs = ((cs) ? ((sys_slist_peek_next(&((cs)->cs_next))) ? ((struct settings_store *)(((char *)((sys_slist_peek_next(&((cs)->cs_next))))) - ((size_t)__INTADDR__(&(((struct settings_store *)0)->cs_next))))) : 0) : 0))
        {
		cs->cs_itf->csi_load(cs, &arg);
	}
#else /* defined(__IAR_SYSTEMS_ICC__) */
	SYS_SLIST_FOR_EACH_CONTAINER(&settings_load_srcs, cs, cs_next) {
		cs->cs_itf->csi_load(cs, &arg);
	}
#endif /* defined(__IAR_SYSTEMS_ICC__) */
	rc = settings_commit_subtree(subtree);
	k_mutex_unlock(&settings_lock);
	return rc;
}

int settings_load_subtree_direct(
	const char             *subtree,
	settings_load_direct_cb cb,
	void                   *param)
{
	struct settings_store *cs;

	const struct settings_load_arg arg = {
		.subtree = subtree,
		.cb = cb,
		.param = param
	};
	/*
	 * for every config store
	 *    load config
	 *    apply config
	 *    commit all
	 */
	k_mutex_lock(&settings_lock, K_FOREVER);
#if defined(__IAR_SYSTEMS_ICC__)
	for (cs = ((sys_slist_peek_head(&settings_load_srcs)) ? ((struct settings_store *)(((char *)((sys_slist_peek_head(&settings_load_srcs)))) - ((size_t)__INTADDR__(&(((struct settings_store *)0)->cs_next))))) : 0);
	     cs != 0;
	     cs = ((cs) ? ((sys_slist_peek_next(&((cs)->cs_next))) ? ((struct settings_store *)(((char *)((sys_slist_peek_next(&((cs)->cs_next))))) - ((size_t)__INTADDR__(&(((struct settings_store *)0)->cs_next))))) : 0) : 0))
        {
		cs->cs_itf->csi_load(cs, &arg);
	}
#else /* defined(__IAR_SYSTEMS_ICC__) */
	SYS_SLIST_FOR_EACH_CONTAINER(&settings_load_srcs, cs, cs_next) {
		cs->cs_itf->csi_load(cs, &arg);
	}
#endif /* defined(__IAR_SYSTEMS_ICC__) */
	k_mutex_unlock(&settings_lock);
	return 0;
}

/*
 * Append a single value to persisted config. Don't store duplicate value.
 */
int settings_save_one(const char *name, const void *value, size_t val_len)
{
	int rc;
	struct settings_store *cs;

	cs = settings_save_dst;
	if (!cs) {
		return -ENOENT;
	}

	k_mutex_lock(&settings_lock, K_FOREVER);

	rc = cs->cs_itf->csi_save(cs, name, (char *)value, val_len);

	k_mutex_unlock(&settings_lock);

	return rc;
}

int settings_delete(const char *name)
{
	return settings_save_one(name, NULL, 0);
}

int settings_save(void)
{
	struct settings_store *cs;
	int rc;
	int rc2;

	cs = settings_save_dst;
	if (!cs) {
		return -ENOENT;
	}

	if (cs->cs_itf->csi_save_start) {
		cs->cs_itf->csi_save_start(cs);
	}
	rc = 0;

	Z_STRUCT_SECTION_FOREACH(settings_handler_static, ch) {
		if (ch->h_export) {
			rc2 = ch->h_export(settings_save_one);
			if (!rc) {
				rc = rc2;
			}
		}
	}

	if (cs->cs_itf->csi_save_end) {
		cs->cs_itf->csi_save_end(cs);
	}
	return rc;
}

void settings_store_init(void)
{
	sys_slist_init(&settings_load_srcs);
}
#endif //CONFIG_BT_SETTINGS
