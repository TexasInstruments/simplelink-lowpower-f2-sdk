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

#ifndef SID_PROTOCOL_TRAFFIC_GEN_H
#define SID_PROTOCOL_TRAFFIC_GEN_H

#ifdef __cplusplus
extern "C" {
#endif

#include <sid_error.h>
#include <stdint.h>
#include <stdbool.h>

#define DEVICE_ID_SZ         5

enum traffic_gen_dst {
   DEVICE = 0,
   CLOUD = 1,
   GROUP = 2
};

typedef struct traffic_gen_cfg {
    unsigned char  dst_id[DEVICE_ID_SZ];
    uint8_t        seq_no;
    uint8_t        group_no;
    int32_t        start_delay;
    uint32_t       tx_interval;
    uint32_t       min_random;
    uint32_t       max_random;
    uint16_t       count;
    int8_t         id;
    uint8_t        dst;
    uint8_t        retries;
    bool           ack;
    bool           response;
    bool           event_based;
    uint8_t        pl_size;
    int16_t        ll_seq_num;
    uint32_t       cld_addr;
} traffic_gen_cfg_t;

enum traffic_gen_cfg_cmd {
   START = 0,
   STOP = 1,
   SHOW = 2,
   ADD = 3,
   DELETE = 4,
   DELETE_ALL = 5
};

enum traffic_gen_metrics_cmd {
   METRICS_SHOW = 0,
   METRICS_CLEAR = 1,
   METRICS_DELETE = 2,
   METRICS_ENABLE = 3,
   METRICS_DISABLE = 4
};

/*
 * @brief   Process traffic gen cfg cmd
 *
 * @return  SID_ERROR_NONE if success
 */
sid_error_t sid_traffic_gen_cfg_exec(enum traffic_gen_cfg_cmd cmd, traffic_gen_cfg_t *pcfg);

/*
 * @brief   Send a ping packet
 *
 * @return  SID_ERROR_NONE if success
 */
sid_error_t sid_traffic_gen_ping_send(enum traffic_gen_dst dst, traffic_gen_cfg_t *pcfg);

/**
 * @brief   Process traffic gen metrics cmd
 *
 * @return  SID_ERROR_NONE if success
 */
sid_error_t sid_traffic_gen_metrics_exec(enum traffic_gen_metrics_cmd cmd);

/**
 * @brief   Get a traffic gen config for a specific ID
 *
 * @return  None
 */
struct traffic_gen_cfg *sid_traffic_gen_get_config(int8_t id);

/*
 * @brief   Inits traffic gen cli
 *
 * @return  0 if success
 */

sid_error_t sid_traffic_gen_init();

/**
 * @brief   Process traffic gen events
 *
 * @return  None
 */
void sid_traffic_gen_process();

/**
 * @brief   De-init traffic gen cli
 *
 * @return  None
 */
void sid_traffic_gen_deinit();

#ifdef __cplusplus
}
#endif

#endif
