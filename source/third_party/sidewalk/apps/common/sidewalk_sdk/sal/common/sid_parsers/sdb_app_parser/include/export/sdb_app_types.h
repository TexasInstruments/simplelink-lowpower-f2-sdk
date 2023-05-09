/*
 * Copyright 2021 Amazon.com, Inc. or its affiliates. All rights reserved.
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

#ifndef SDB_APP_TYPES_H
#define SDB_APP_TYPES_H

#include <sid_network_address.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SDB_MINOR_VERSION 0
#define SDB_MAJOR_VERSION 1

#define SDB_TAG_VALUE(x) (1 << x)

enum sdb_app_cmd_type {
    SDB_CMD_READ = 0,
    SDB_CMD_WRITE = 1,
    SDB_CMD_NTFY = 2,
    SDB_CMD_RESP = 3,
};

enum sdb_payload_tag {
    SDB_TAG_ID_VERSION = 0,
    SDB_TAG_ID_STATUS = 1,
    SDB_TAG_ID_HANDLE = 2,
    SDB_TAG_ID_KA_PERIOD = 3,
    SDB_TAG_ID_HOP_SPEED = 4,
    SDB_TAG_ID_EPOCH_MS = 5,
    SDB_TAG_ID_DEV_ID = 6,
    SDB_TAG_ID_SDB_T_REMAIN = 7,
    SDB_TAG_ID_CTRL_COMMAND = 8,
};

enum sdb_linkstatus {
    SDB_LINK_STATUS_OK = 0,
    SDB_LINK_STATUS_FAIL = 0xFF,
};

struct sdb_ctrl {
    uint32_t tag_mask;
    uint8_t cmd_type;
    uint8_t sub_cmd;
    uint32_t handle;
    struct sid_address addr;
};

struct sdb_open {
    uint32_t tag_mask;
    uint8_t cmd_type;
    uint8_t major_version;
    uint8_t minor_version;
};

struct sdb_maintain {
    uint32_t tag_mask;
    uint8_t cmd_type;
    uint32_t handle;
};

struct sdb_ka {
    uint32_t tag_mask;
    uint8_t cmd_type;
    uint32_t epoch_ms;
    uint8_t hop_speed;
    uint8_t period;
};

struct sdb_app_link_status {
    uint32_t tag_mask;
    uint8_t cmd_type;
    uint8_t status_code;
    uint32_t handle;
};

struct sdb_close {
    uint32_t tag_mask;
    uint8_t cmd_type;
    uint16_t time_remaining_secs;
};

#ifdef __cplusplus
}
#endif
#endif
