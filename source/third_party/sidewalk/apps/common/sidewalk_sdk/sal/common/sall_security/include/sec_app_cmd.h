/*
* Copyright 2021-2022 Amazon.com, Inc. or its affiliates.  All rights reserved.
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

#ifndef SEC_APP_CMD_H
#define SEC_APP_CMD_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum app_cmd_type {
    APP_CMD_READ     = 0x0,
    APP_CMD_WRITE    = 0x1,
    APP_CMD_NOTIFY   = 0x2,
    APP_CMD_RESPONSE = 0x3,
    APP_CMD_ALL      = 0x4,
};

enum {
    APP_CMD_ID_OFFSET = 0x2,
    APP_CMD_ID_MASK   = 0x03,
};

static inline uint16_t merge_app_cmd_id_code(uint16_t cmd_id, enum app_cmd_type type)
{
    return (cmd_id << APP_CMD_ID_OFFSET) | (type & APP_CMD_ID_MASK);
}

static inline uint16_t app_app_cmd_id_read(uint16_t cmd_id)
{
    return merge_app_cmd_id_code(cmd_id, APP_CMD_READ);
}

static inline uint16_t app_cmd_id_write(uint16_t cmd_id)
{
    return merge_app_cmd_id_code(cmd_id, APP_CMD_WRITE);
}

static inline uint16_t app_cmd_id_notify(uint16_t cmd_id)
{
    return merge_app_cmd_id_code(cmd_id, APP_CMD_NOTIFY);
}

static inline uint16_t app_cmd_id_response(uint16_t cmd_id)
{
    return merge_app_cmd_id_code(cmd_id, APP_CMD_RESPONSE);
}

static inline uint16_t get_app_cmd_id(uint16_t cmd_id)
{
    return cmd_id >> APP_CMD_ID_OFFSET;
}

static inline uint8_t get_app_cmd_type(uint16_t cmd_id)
{
    return cmd_id & APP_CMD_ID_MASK;
}

#ifdef __cplusplus
}
#endif

#endif /*! SEC_APP_CMD_H */
