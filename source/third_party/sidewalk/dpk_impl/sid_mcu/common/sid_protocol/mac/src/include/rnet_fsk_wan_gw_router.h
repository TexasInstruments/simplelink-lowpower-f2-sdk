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

#ifndef RNET_FSK_WAN_ROUTER_H
#define RNET_FSK_WAN_ROUTER_H

#include <rnet_transact_store.h>
#include <rnet_frames.h>
#include <sid_error.h>
#include <sid_jp_fsk_wan_gw.h>
#include <stdint.h>
#include <stdbool.h>

/*!
 * @brief Consumes an incoming cmd payload containing slot lookup response.
 * @param[in] fsk_wan_info: Pointer to FSK-WAN info to be consumed
 * returns An error code indicating SUCESS/FAILURE.
 */
sid_error_t sid_fsk_wan_consume_slot_lookup_resp(struct sid_jp_generic_data_holder *fsk_wan_info);

/*!
 * @brief Determines whether there is a pending expected response for FSK-WAN schedule.
 * @param[in] rnet: Pointer to the transaction store entry having the packet.
 * @param[in] node: Pointer to the node in nodedb contatining the destination nodes entry.
 *                  If null, node database will be queried for the destination. If a prior
 *                  entry does not exist, a new entry will be created.
 * @return true for pending response, and false otherwise.
 */
bool is_node_sch_info_query_pending(rnet_transact_store_t *rnet, rnet_node_t *node);

#endif /* RNET_FSK_WAN_ROUTER_H */
