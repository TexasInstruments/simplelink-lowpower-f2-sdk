/*
 * Copyright (c) 2016-2018, Arm Limited and affiliates.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "nsconfig.h"
#include <string.h>
#include "ns_types.h"
#include "fhss_api.h"
#include "fhss_config.h"
#include "net_fhss.h"
#include "nsdynmemLIB.h"
#include "Service_Libs/fhss/fhss.h"
#include "Service_Libs/fhss/fhss_common.h"
#include "Service_Libs/fhss/fhss_ws.h"
#include "Service_Libs/fhss/fhss_statistics.h"
#include "Service_Libs/fhss/channel_list.h"
#include <arm_hal_interrupt.h>
#include "ns_trace.h"
#include "mac_assert.h"

#define TRACE_GROUP "fhss"

static fhss_structure_t *fhss_struct = NULL;

fhss_structure_t *fhss_allocate_instance(fhss_api_t *fhss_api, const fhss_timer_t *fhss_timer)
{
    if (fhss_struct || !fhss_api || !fhss_timer) {
        return NULL;
    }
    fhss_struct = ns_dyn_mem_alloc(sizeof(fhss_structure_t));
    if (!fhss_struct) {
        return NULL;
    }
    memset(fhss_struct, 0, sizeof(fhss_structure_t));
    fhss_struct->fhss_api = fhss_api;
    fhss_struct->platform_functions = *fhss_timer;
    if (!fhss_struct->platform_functions.fhss_resolution_divider) {
        fhss_struct->platform_functions.fhss_resolution_divider = 1;
    }
    return fhss_struct;
}

int8_t fhss_free_instance(fhss_api_t *fhss_api)
{
    if (!fhss_struct || fhss_struct->fhss_api != fhss_api) {
        return -1;
    }
    ns_dyn_mem_free(fhss_struct);
    fhss_struct = NULL;
    return 0;
}

int fhss_ws_set_hop_count(fhss_structure_t *fhss_structure, const uint8_t hop_count)
{
    fhss_structure->own_hop = hop_count;
//    fhss_stats_update(fhss_structure, STATS_FHSS_HOP_COUNT, fhss_structure->own_hop);
    return 0;
}

int fhss_ws_set_tx_allowance_level(fhss_structure_t *fhss_structure, const fhss_ws_tx_allow_level global_level, const fhss_ws_tx_allow_level ef_level)
{
    fhss_structure->ws->tx_level = global_level;
    fhss_structure->ws->ef_tx_level = ef_level;
    return 0;
}

fhss_structure_t *fhss_ws_enable(fhss_api_t *fhss_api, const fhss_ws_configuration_t *fhss_configuration, const fhss_timer_t *fhss_timer)
{
    if (!fhss_api || !fhss_configuration || !fhss_timer) {
        tr_err("Invalid FHSS enable configuration");
        return NULL;
    }
    int channel_count = channel_list_count_channels(fhss_configuration->channel_mask);
    if (channel_count <= 0) {
        // There must be at least one configured channel in channel list
        return NULL;
    }
    fhss_structure_t *fhss_struct = fhss_allocate_instance(fhss_api, fhss_timer);
    if (!fhss_struct) {
        return NULL;
    }
    fhss_struct->ws = ns_dyn_mem_alloc(sizeof(fhss_ws_t));
    if (!fhss_struct->ws) {
        fhss_free_instance(fhss_api);
        return NULL;
    }
    memset(fhss_struct->ws, 0, sizeof(fhss_ws_t));

    fhss_struct->ws->fhss_configuration = *fhss_configuration;
    fhss_struct->number_of_channels = channel_count;
    fhss_ws_set_hop_count(fhss_struct, 0xff);
    fhss_struct->rx_channel = fhss_configuration->unicast_fixed_channel;
    fhss_struct->ws->min_synch_interval = DEFAULT_MIN_SYNCH_INTERVAL;

    ns_list_init(&fhss_struct->fhss_failed_tx_list);
    return fhss_struct;
}

int8_t fhss_disable(fhss_structure_t *fhss_structure)
{
    if (!fhss_structure) {
        return -1;
    }
    fhss_structure->fhss_api->synch_state_set(fhss_structure->fhss_api, FHSS_UNSYNCHRONIZED, 0);
    ns_dyn_mem_free(fhss_structure->bs);
    ns_dyn_mem_free(fhss_structure->ws->tr51_channel_table);
    ns_dyn_mem_free(fhss_structure->ws->tr51_output_table);
    ns_dyn_mem_free(fhss_structure->ws);
//    fhss_failed_list_free(fhss_structure);
    ns_dyn_mem_free(fhss_structure);
    fhss_struct = 0;
    return 0;
}

int ns_fhss_delete(fhss_api_t *fhss_api)
{
    fhss_structure_t *fhss_structure = fhss_get_object_with_api(fhss_api);
    if (!fhss_structure) {
        return -1;
    }
    if (fhss_disable(fhss_structure)) {
        return -1;
    }
    ns_dyn_mem_free(fhss_api);
    return 0;
}

fhss_api_t *ns_fhss_ws_create(const fhss_ws_configuration_t *fhss_configuration, const fhss_timer_t *fhss_timer)
{
    fhss_api_t *this = ns_dyn_mem_alloc(sizeof(fhss_api_t));
    if (!this) {
        return NULL;
    }
    // Create FHSS object
    fhss_structure_t *fhss_struct = fhss_ws_enable(this, fhss_configuration, fhss_timer);
    if (!fhss_struct) {
        tr_err("Failed to enable FHSS");
        ns_dyn_mem_free(this);
        return NULL;
    }

    return this;
}

int ns_fhss_ws_set_parent(const fhss_api_t *fhss_api, const uint8_t eui64[8], const broadcast_timing_info_t *bc_timing_info, const bool force_synch)
{
    /* Set Parent to track broadcast schedule */
    timacSetTrackParent((uint8_t *) eui64);
    (void) fhss_api;
    (void) bc_timing_info;
    (void) force_synch;
    return 0;
}

int ns_fhss_ws_remove_parent(const fhss_api_t *fhss_api, const uint8_t eui64[8])
{
    (void) fhss_api;
    (void) eui64[8];
    MAC_ASSERT(0);
    return 0;
}

int ns_fhss_set_neighbor_info_fp(const fhss_api_t *fhss_api, fhss_get_neighbor_info *get_neighbor_info)
{
    (void) fhss_api;
    (void) get_neighbor_info;
    MAC_ASSERT(0);
    return 0;
}

const fhss_ws_configuration_t *ns_fhss_ws_configuration_get(const fhss_api_t *fhss_api)
{
    (void) fhss_api;
    if (!fhss_struct || !fhss_struct->ws) {
        return NULL;
    }
    return &fhss_struct->ws->fhss_configuration;
}

fhss_structure_t *fhss_get_object_with_api(const fhss_api_t *fhss_api)
{
    if (!fhss_api || !fhss_struct) {
        return NULL;
    }
    if (fhss_struct->fhss_api == fhss_api) {
        return fhss_struct;
    }
    return NULL;
}

int fhss_ws_configuration_set(fhss_structure_t *fhss_structure, const fhss_ws_configuration_t *fhss_configuration)
{
    int channel_count = channel_list_count_channels(fhss_configuration->channel_mask);
    if (channel_count <= 0) {
        return -1;
    }
    platform_enter_critical();
    fhss_structure->ws->fhss_configuration = *fhss_configuration;
    fhss_structure->number_of_channels = channel_count;
    if (fhss_configuration->ws_uc_channel_function == WS_FIXED_CHANNEL) {
        fhss_structure->rx_channel = fhss_configuration->unicast_fixed_channel;
    }
    platform_exit_critical();
    return 0;
}

int ns_fhss_ws_configuration_set(const fhss_api_t *fhss_api, const fhss_ws_configuration_t *fhss_configuration)
{
    fhss_structure_t *fhss_structure = fhss_get_object_with_api(fhss_api);
    if (!fhss_structure || !fhss_structure->ws) {
        return -1;
    }
    return fhss_ws_configuration_set(fhss_structure, fhss_configuration);
}

int ns_fhss_ws_set_hop_count(const fhss_api_t *fhss_api, const uint8_t hop_count)
{
    fhss_structure_t *fhss_structure = fhss_get_object_with_api(fhss_api);
    if (!fhss_structure || !fhss_structure->ws) {
        return -1;
    }
    return fhss_ws_set_hop_count(fhss_structure, hop_count);
}

int ns_fhss_ws_set_tx_allowance_level(const fhss_api_t *fhss_api, const fhss_ws_tx_allow_level global_level, const fhss_ws_tx_allow_level ef_level)
{
    fhss_structure_t *fhss_structure = fhss_get_object_with_api(fhss_api);
    if (!fhss_structure || !fhss_structure->ws) {
        return -1;
    }
    return fhss_ws_set_tx_allowance_level(fhss_structure, global_level, ef_level);
}

