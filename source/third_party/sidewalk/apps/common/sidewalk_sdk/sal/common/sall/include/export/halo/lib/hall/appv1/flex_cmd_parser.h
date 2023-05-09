/*
* Copyright 2020 Amazon.com, Inc. or its affiliates.  All rights reserved.
*
* AMAZON PROPRIETARY/CONFIDENTIAL
*
* You may not use this file except in compliance with the terms and conditions
* set forth in the accompanying LICENSE.TXT file.
*
* THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
* DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
* IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
*/

#ifndef HALO_LIB_HALL_INCLUDE_EXPORT_HALO_LIB_HALL_APPV1_FLEX_CMD_PARSER_H
#define HALO_LIB_HALL_INCLUDE_EXPORT_HALO_LIB_HALL_APPV1_FLEX_CMD_PARSER_H

#include <halo/lib/hall/appv1/cmd.h>
#include <halo/lib/hall/hallerr.h>

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

sid_error_t halo_hall_appv1_cmd_descriptor_from_flex_deserialize(const uint8_t* buffer, const size_t len, halo_appv1_cmd_descriptor_t* appv1_cmd, size_t* offset);

sid_error_t halo_hall_appv1_cmd_descriptor_to_flex_serialize(uint8_t* buffer, const size_t len, const halo_appv1_cmd_descriptor_t* appv1_cmd, size_t* offset);

#ifdef __cplusplus
}
#endif

#endif /*! HALO_LIB_HALL_INCLUDE_EXPORT_HALO_LIB_HALL_APPV1_FLEX_CMD_PARSER_H */
