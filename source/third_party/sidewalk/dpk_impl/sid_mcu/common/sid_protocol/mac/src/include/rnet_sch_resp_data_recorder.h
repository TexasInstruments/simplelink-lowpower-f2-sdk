/*
 * Copyright 2022 Amazon.com, Inc. or its affiliates. All rights reserved.
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

#ifndef RNET_SCH_RESPONSE_DATA_RECORDER_H
#define RNET_SCH_RESPONSE_DATA_RECORDER_H

#include <rnet_errors.h>
#include <stdint.h>
#include <stdbool.h>

/*!
 * @brief Records the DL schedule information of a given node to nodeDB.
 * @param offset: The first slot number after frame boundary in schedule.
 * @param period: The period of slots in schedule.
 * @param[in] node_id: The ID of the node following the wakeup schedule.
 *                     The size of the ID must be compatible with the nodedb implementation,
 *                     which uses RNET_DEV_ID_SZ.
 * returns An error code indicating SUCESS/FAILURE.
 */
rnet_error_t rnet_fsk_wan_record_sch_info(uint8_t offset, uint8_t period, uint8_t *node_id);

/*!
 * @brief Enable/disable DL schedule for a given node
 * @param enable: Set to true to use the allocated DL schedule, set to false to use CS slot
 * @param[in] node_id: The ID of the node following the wakeup schedule.
 *                     The size of the ID must be compatible with the nodedb implementation,
 *                     which uses RNET_DEV_ID_SZ.
 * returns An error code indicating SUCESS/FAILURE.
 */
rnet_error_t rnet_fsk_wan_sch_enable(bool enable, uint8_t *node_id);

/*!
 * @brief Records the join response code of a given node to nodeDB
 * @param jp_resp_code: The response code represents whether the join is accpected or rejected
 * @param[in] node_id: The ID of the node following the wakeup schedule.
 *                     The size of the ID must be compatible with the nodedb implementation,
 *                     which uses RNET_DEV_ID_SZ.
 * returns An error code indicating SUCESS/FAILURE.
 */
rnet_error_t rnet_fsk_wan_record_jp_resp_code(uint8_t jp_resp_code, uint8_t *node_id);

/*!
 * @brief Get the join response code record of a given node in nodeDB
 * @param[out] jp_resp_code: The response code represents whether the join is accpected or rejected
 * @param[in] node_id: The ID of the node following the wakeup schedule.
 *                     The size of the ID must be compatible with the nodedb implementation,
 *                     which uses RNET_DEV_ID_SZ.
 * returns An error code indicating SUCESS/FAILURE.
 */
rnet_error_t rnet_fsk_wan_get_last_jp_resp_code(uint8_t *jp_resp_code, uint8_t *node_id);

#endif /* RNET_SCH_RESPONSE_DATA_RECORDER_H */
