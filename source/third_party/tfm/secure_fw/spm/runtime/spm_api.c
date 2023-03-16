/*
 * Copyright (c) 2017-2020, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

/* All the APIs defined in this file are common for library and IPC model. */

#include "spm_api.h"
#include "spm_db.h"

/* Extern SPM variable */
extern struct spm_partition_db_t g_spm_partition_db;

uint32_t get_partition_idx(uint32_t partition_id)
{
    uint32_t i;

    if (partition_id == INVALID_PARTITION_ID) {
        return SPM_INVALID_PARTITION_IDX;
    }

    for (i = 0; i < g_spm_partition_db.partition_count; ++i) {
        if (g_spm_partition_db.partitions[i].static_data->partition_id ==
                partition_id) {
            return i;
        }
    }
    return SPM_INVALID_PARTITION_IDX;
}

uint32_t tfm_spm_partition_get_partition_id(uint32_t partition_idx)
{
    return g_spm_partition_db.partitions[partition_idx].static_data->
            partition_id;
}

uint32_t tfm_spm_partition_get_flags(uint32_t partition_idx)
{
    return g_spm_partition_db.partitions[partition_idx].static_data->
            partition_flags;
}

uint32_t tfm_spm_partition_get_privileged_mode(uint32_t partition_flags)
{
    if (partition_flags & SPM_PART_FLAG_PSA_ROT) {
        return TFM_PARTITION_PRIVILEGED_MODE;
    } else {
        return TFM_PARTITION_UNPRIVILEGED_MODE;
    }
}

bool tfm_is_partition_privileged(uint32_t partition_idx)
{
    uint32_t flags = tfm_spm_partition_get_flags(partition_idx);

    return tfm_spm_partition_get_privileged_mode(flags) ==
        TFM_PARTITION_PRIVILEGED_MODE;
}

__attribute__((section("SFN")))
void tfm_spm_partition_change_privilege(uint32_t privileged)
{
    CONTROL_Type ctrl;

    ctrl.w = __get_CONTROL();

    if (privileged == TFM_PARTITION_PRIVILEGED_MODE) {
        ctrl.b.nPRIV = 0;
    } else {
        ctrl.b.nPRIV = 1;
    }

    __set_CONTROL(ctrl.w);
}
