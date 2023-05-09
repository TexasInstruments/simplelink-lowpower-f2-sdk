/*
 * Copyright 2020-2022 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.txt file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef _RNET_SDB_H_
#define _RNET_SDB_H_

#include <sid_error.h>

#include <stdbool.h>
#include <stdint.h>

#include "rnet_dev_addr_def.h"

struct sdb_app_handle;

struct sdb_cmd {
  uint16_t cmd_id;
  uint8_t type;
  uint8_t *cmd_data;
  uint8_t cmd_len;
  uint8_t channel;
  uint8_t dst[RNET_DEV_ID_SZ];
  uint8_t dst_frmt;
  uint32_t seqn;
};

sid_error_t sdb_init(struct sdb_app_handle **handle);

sid_error_t process_sdb_command(struct sdb_app_handle *handle, struct sdb_cmd *req);

void sdb_process(struct sdb_app_handle *handle);

sid_error_t sdb_deinit(struct sdb_app_handle *handle);

#if !(SID_GW_BUILD)
sid_error_t sdb_tx_prepare(void *rnet_tr, int32_t tx_airtime_ms);
#endif

#endif //_RNET_SDB_H_
