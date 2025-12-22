/*
 * Copyright (c) 2024, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,

 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ======== rcp_lmac.h ========
 */

#ifndef RCP_LMAC_H_
#define RCP_LMAC_H_

#include <stddef.h>
#include <stdint.h>
#include "rcp_types.h"
#include "timac_api.h"

typedef struct rcp_lmac_internal_s {
    uint8_t async_channel_list[MAC_154G_CHANNEL_BITMAP_SIZ];
    keyDescriptor_t key_descriptors[MAC_NUM_KEY_DESCRIPTORS];
    macMRFSKPHYDesc_t phy_descriptor; // Current LMAC PHY parameters
    bool ffd; // Full function device (BR) or reduced function device (RN)
    bool start_flag; // Flag to indicate ready to start to the MAC thread
    uint8_t app_task_id; // Application task ID
} rcp_lmac_internal_t;

extern rcp_lmac_internal_t rcp_lmac_store;

void rcp_lmac_data_ind_to_host(macCbackEvent_t *pData);

void rcp_lmac_data_cnf_to_host(macCbackEvent_t *pData);

rcp_status_t rcp_lmac_from_host(rcp_cmd_t *rcp_tx_cmd);

rcp_status_t handle_rcp_data_req(rcp_data_req_t *data_req);

#endif // RCP_LMAC_H_

