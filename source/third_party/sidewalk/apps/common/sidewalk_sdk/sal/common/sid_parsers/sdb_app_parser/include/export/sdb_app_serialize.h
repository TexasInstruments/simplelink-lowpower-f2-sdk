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

#ifndef SDB_APP_SERIALIZE_H
#define SDB_APP_SERIALIZE_H

#include <sdb_app_types.h>
#include <sid_error.h>

#include <stdbool.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

//TODO: Add doxygen comments
sid_error_t sdb_control_serialize(uint8_t *buffer, size_t len, const struct sdb_ctrl *data, uint16_t *out_size);

sid_error_t sdb_open_serialize(uint8_t *buffer, size_t len, const struct sdb_open *data, uint16_t *out_size);

sid_error_t sdb_maintain_serialize(uint8_t *buffer, size_t len, const struct sdb_maintain *data, uint16_t *out_size);

sid_error_t sdb_ka_serialize(uint8_t *buffer, size_t len, const struct sdb_ka *data, uint16_t *out_size);

sid_error_t sdb_link_status_serialize(uint8_t *buffer, size_t len, const struct sdb_app_link_status *data, uint16_t *out_size);

sid_error_t sdb_close_serialize(uint8_t *buffer, size_t len, const struct sdb_close *data, uint16_t *out_size);

#ifdef __cplusplus
}
#endif
#endif

