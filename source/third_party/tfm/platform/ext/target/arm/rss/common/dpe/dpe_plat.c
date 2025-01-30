/*
 * Copyright (c) 2023 Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "dpe_plat.h"
#include "device_definition.h"
#include "rss_kmu_slot_ids.h"
#include "tfm_crypto_defs.h"

int dpe_plat_get_rot_cdi(uint8_t *buf, size_t buf_len)
{
    enum kmu_error_t err;

    err = kmu_get_key(&KMU_DEV_S, RSS_KMU_SLOT_ROT_CDI, buf, buf_len);
    if (err != KMU_ERROR_NONE) {
        return -1;
    }

    return 0;
}

psa_key_id_t dpe_plat_get_root_attest_key_id(void)
{
    return TFM_BUILTIN_KEY_ID_IAK;
}
