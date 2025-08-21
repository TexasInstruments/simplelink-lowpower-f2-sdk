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
 *  ======== rcp_host.h ========
 */

#ifndef RCP_HOST_H_
#define RCP_HOST_H_

#include <stddef.h>
#include <stdint.h>
#include "rcp_types.h"
#include "mac_api.h"

#include "MAC/IEEE802_15_4/mac_defines.h"
#include "MAC/IEEE802_15_4/mac_mcps_sap.h"
#include "MAC/IEEE802_15_4/mac_security_mib.h"
#include "MAC/IEEE802_15_4/mac_header_helper_functions.h"
#include "sw_mac.h"

typedef struct rcp_host_internal_s {
    mac_api_t *mac_api;
    struct protocol_interface_rf_mac_setup *setup;
    bool async_in_progress;
    bool uc_in_progress;
    bool bc_in_progress;
} rcp_host_internal_t;

void handle_rcp_data_ind(rcp_data_ind_t * data_ind);
void handle_rcp_data_cnf(rcp_data_cnf_t * data_cnf);

void rcp_host_init(mac_api_t* pMac_api, struct protocol_interface_rf_mac_setup *pMac_setup);

rcp_status_t rcp_host_data_req(rcp_data_req_type data_req_type,
                          uint8_t *data, uint16_t data_len, uint8_t msdu_handle,
                          mlme_security_t *sec, uint16_t mdata_offset,
                          uint32_t frame_count, uint8_t frame_count_offset,
                          uint16_t utie_offset, uint16_t btie_offset, fhnt_entry_t *fhnt_entry);

void rcp_init();
void rcp_host_mac_init(mlme_start_t *start_req);
void rcp_host_fh_init();
void rcp_host_mac_reset();
void rcp_host_set_bc_timing(broadcast_timing_info_t *bc_timing_info, bool force_synch);
void rcp_host_set_sec_key(uint8_t index, uint8_t key[16]);
void rcp_host_set_tx_frame_count(uint8_t index, uint32_t frame_count);
void processIncomingAresp(void);

// void rcp_host_get_config(uint8_t attr);
void rcp_host_set_config(uint16_t attr, void *val, uint16_t val_len);

void rcp_host_from_lmac(rcp_cmd_t *rcp_cmd);

#endif // RCP_HOST_H_
