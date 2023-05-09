/*
 * Copyright 2019-2021 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.TXT file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef LIB_HALL_INCLUDE_EXPORT_HALO_LIB_HALL_INTERFACE_H_
#define LIB_HALL_INCLUDE_EXPORT_HALO_LIB_HALL_INTERFACE_H_


#include <stdbool.h>
#include <sid_memory_pool.h>

#include <halo/lib/hall/hallerr.h>
#include <halo/lib/hall/core/router.h>
#include <halo/lib/hall/core/dispatcher.h>
#include <halo/lib/hall/core/mdm.h>
#include <halo/lib/hall/core/resource_manager.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void* halo_hall_lib_ctx_t;

typedef struct {
    const halo_lib_sequence_number_ifc * sequence_number;
    const halo_hall_stats_ifc * mdm_stats_ifc;
    struct sid_memory_pool * memory_pool;
    set_timer_t mdm_set_timer_callback;
    timer_ctx_t mdm_timer_context;
    struct sid_address local_addr;
} halo_hall_lib_params_t;

typedef struct {
    halo_hall_router_config_t router_config;
    halo_hall_mdm_config_t mdm_config;
    uint32_t mem_pool_size;
} halo_hall_lib_config_t;

const halo_hall_lib_config_t* halo_hall_get_default_config(void);

sid_error_t halo_hall_init(halo_hall_lib_ctx_t* lib_ctx,
                            const halo_hall_lib_params_t* params,
                            const halo_hall_lib_config_t* config);
sid_error_t halo_hall_deinit(halo_hall_lib_ctx_t lib_ctx);

sid_error_t halo_hall_get_mdm(void* lib_ctx, const halo_hall_mdm_ifc** mdm);
sid_error_t halo_hall_get_router(void* lib_ctx, const halo_hall_router_ext_ifc** router);
sid_error_t halo_hall_get_dispatcher(void* lib_ctx, const halo_hall_dispatcher_ifc** dispatcher);
sid_error_t halo_hall_get_resource_manager(void* lib_ctx, const halo_hall_resource_manager_ifc** resource_manager);

// functions may return NULL
const halo_hall_mdm_ifc* halo_hall_ref_mdm(const void* lib_ctx);
const halo_hall_router_ext_ifc* halo_hall_ref_router(const void* lib_ctx);
const halo_hall_dispatcher_ifc* halo_hall_ref_dispatcher(const void* lib_ctx);
const halo_hall_resource_manager_ifc* halo_hall_ref_resource_manager(const void* lib_ctx);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* LIB_HALL_INCLUDE_EXPORT_HALO_LIB_HALL_INTERFACE_H_ */
