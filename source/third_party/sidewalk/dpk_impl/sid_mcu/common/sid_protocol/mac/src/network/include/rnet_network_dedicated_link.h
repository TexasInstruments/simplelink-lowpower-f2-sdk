/*
 * Copyright 2020 Amazon.com, Inc. or its affiliates. All rights reserved.
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

#ifndef RNET_NETWORK_DEDICATED_LINK_H
#define RNET_NETWORK_DEDICATED_LINK_H
#include <sid_protocol_opts.h>

#if defined(GW_SUPPORT) && GW_SUPPORT

#include <stdint.h>
#include "rnet_config.h"
#include "rnet_nw_layer.h"
#include "rnet_transact_store.h"
#include "sid_mac_controller.h"

typedef struct {
    uint8_t ch;
} rnet_dedicated_link_hnd_shake_param_t;

typedef enum {
    NOT_INIT_STATE = 0,
    DEDICATED_LINK_INIT_STATE,
    KEY_EXCHANGE_STATE,
    HND_SHAKE_STATE,
} rnet_dedicated_link_setup_state_t;

/*!
 * @brief: function to terminate the dedicated link
 */
void rnet_terminate_dedicated_link(void);

/*!
 * @brief: API to call to trigger the state machine
 * @param [in]: new state variable
 * @param [in] pointer to data
 */
void rnet_process_dedicated_state_machine(rnet_dedicated_link_setup_state_t state, void *data);
/*!
 * @brief: API to call when the link ready command is received from the distress gw/client dev
 * @param[in]: client dev ID
 * @return[out] error if the the distress GW/Client id validation fails.
 */
rnet_error_t rnet_process_dev_link_ready_announcement(uint8_t* src_id);
/*!
 * @brief: API to call when the ack is received from the assisting GW
 * @param[in]: Assisting GW ID
 * @return[out] error if the the Assisting GW ID validation fails.
 */
rnet_error_t rnet_process_dev_link_ready_announcement_ack(uint8_t* src_id);
/*!
 * @brief: function to force start dedicated link through cli.
 * @param[in] dedicated link initialization parameter.
 */
void rnet_force_trigger_dedicated_link(rnet_dedicated_link_init_param_t* param);

#endif //GW_SUPPORT

#endif /* RNET_NETWORK_DEDICATED_LINK_H */
