/*
 * Copyright 2022 Amazon.com, Inc. or its affiliates. All rights reserved.
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

#ifndef HALO_HALL_APPV1_SIDEBAND_TYPES_H_
#define HALO_HALL_APPV1_SIDEBAND_TYPES_H_

#include <sdb_app_types.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
/* all the structures are defined in projects/sid/sal/common/sid_parsers/sdb_app_parser/include/export/sdb_app_types.h */
typedef struct sdb_ctrl halo_hall_appv1_sdb_control_t;

typedef struct sdb_open halo_hall_appv1_sdb_open_t;

typedef struct sdb_maintain halo_hall_appv1_sdb_maintain_t;

typedef struct sdb_ka halo_hall_appv1_sdb_keep_alive_t;

typedef struct sdb_app_link_status halo_hall_appv1_sdb_link_status_t;

typedef struct sdb_close halo_hall_appv1_sdb_close_t;

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* HALO_HALL_APPV1_SIDEBAND_TYPES_H_ */
