/*
 * Copyright 2021 Amazon.com, Inc. or its affiliates. All rights reserved.
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

#ifndef HALO_LIB_HALL_INCLUDE_EXPORT_HALO_LIB_HALL_CORE_STATS_H_
#define HALO_LIB_HALL_INCLUDE_EXPORT_HALO_LIB_HALL_CORE_STATS_H_

#include <sid_network_data_ifc.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef const struct halo_hall_stats_ifc_t * halo_hall_stats_ifc;

struct halo_hall_stats_ifc_t {
    void (*destroy)(const halo_hall_stats_ifc* instance);

    /**
     * Function will notified each time when MDM will detect duplicates
     * @param instance - reference to object instance
     */
    void (*messages_dropped)(const halo_hall_stats_ifc* instance,  const network_interface_data_ifc *nif);
};

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* HALO_LIB_HALL_INCLUDE_EXPORT_HALO_LIB_HALL_CORE_STATS_H_ */
