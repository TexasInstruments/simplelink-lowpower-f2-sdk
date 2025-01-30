/*
 * Copyright (c) 2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef __RSS_KMU_SLOT_IDS_H__
#define __RSS_KMU_SLOT_IDS_H__

#include "kmu_drv.h"

#ifdef __cplusplus
extern "C" {
#endif

enum rss_kmu_slot_id_t {
    RSS_KMU_SLOT_TRAM_KEY = KMU_USER_SLOT_MIN,
    RSS_KMU_SLOT_VHUK,
    RSS_KMU_SLOT_CPAK_SEED,
#ifdef RSS_BOOT_KEYS_CCA
    RSS_KMU_SLOT_DAK_SEED,
#endif
#ifdef RSS_BOOT_KEYS_DPE
    RSS_KMU_SLOT_ROT_CDI,
#endif
    /* The session key is used for AEAD, so requires two contiguous slots. Only
     * the first should be used for calls, the key loader and derivation code
     * will transparently use the second where necessary.
     */
    RSS_KMU_SLOT_SESSION_KEY_0,
    _RSS_KMU_AEAD_RESERVED_SLOT_SESSION_KEY,
    /* The CM provisioning key is used for AEAD, so requires two contiguous
     * slots. Only the first should be used for calls, the key loader and
     * derivation code will transparently use the second where necessary.
     */
    RSS_KMU_SLOT_CM_PROVISIONING_KEY,
    _RSS_KMU_AEAD_RESERVED_SLOT_CM_PROVISIONING_KEY,
    /* The DM provisioning key is used for AEAD, so requires two contiguous
     * slots. Only the first should be used for calls, the key loader and
     * derivation code will transparently use the second where necessary.
     */
    RSS_KMU_SLOT_DM_PROVISIONING_KEY,
    _RSS_KMU_AEAD_RESERVED_SLOT_DM_PROVISIONING_KEY,
    RSS_KMU_SLOT_SECURE_ENCRYPTION_KEY,
    RSS_KMU_SLOT_NON_SECURE_ENCRYPTION_KEY,
};

#ifdef __cplusplus
}
#endif

#endif /* __RSS_KMU_SLOT_IDS_H__ */
