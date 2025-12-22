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

#ifndef RCP_QUEUE_H_
#define RCP_QUEUE_H_

#include "ns_list.h"
#include "rcp_types.h"
#include "mac_api.h"

/* Undefine this to turn off unicast queuing in rcp_queue. */
#define RCP_QUEUE_CONFIG_ENQUEUE_UNICAST

#define RCP_QUEUE_MAX_SIZE 20
#define RCP_QUEUE_DATA_REQ_TIMEOUT_DISABLE 0xFFFF
#define RCP_QUEUE_DATA_REQ_TIMEOUT_SEC 3
#define RCP_QUEUE_DATA_REQ_TIMEOUT_PENDING_SEC 10

#define RCP_QUEUE_DATA_REQ_CONTINUE 0
#define RCP_QUEUE_DATA_REQ_QUEUED   1
#define RCP_QUEUE_DATA_REQ_REJECT   2

/*  RCP TX data request buffer structure 
    since RCP LMAC only support 
        one unicast data request
        one broadcast data request
        one Async request

    if there is more than data request is to RCP LMAC, we need to push all 
    data request into queue. Once the data confirm is received and processed
    we will check if there is any more data request in the queue. If there is one,
    deque the data request and send to RCP LMAC
*/
typedef struct rcp_tx_data_req_buffer_s {
    ns_list_link_t      link;                   /*!< linked list */
    /* save all data request parameters */
    rcp_data_req_type           req_type;           /*!< data request type */
    mcps_data_req_t             data_req;           /*!< data request */
    mcps_data_req_ie_list_t     ie_ext;             /*!< ie list */
    mac_data_priority_t         priority;           /*!< data priority */
    uint16_t                    timeout;            /*!< timeout value entry (sec) */
} rcp_tx_data_req_buffer_t;

void rcp_tx_data_queue_init();
void rcp_tx_data_queue_tx_confirm_handler(uint8_t msdu_handle);
uint8_t rcp_tx_data_queue_tx_request_handler(const mcps_data_req_t *data, const mcps_data_req_ie_list_t *ie_ext, 
    const channel_list_s *asynch_channel_list, mac_data_priority_t priority);
void rcp_queue_slow_timer(uint16_t seconds);
    
#endif
