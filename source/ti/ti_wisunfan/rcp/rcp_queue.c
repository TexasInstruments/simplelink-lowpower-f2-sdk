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
 *  ======== rcp_queue.c ========
 */

#include "rcp_queue.h"
#include "nsdynmemLIB.h"

#define TRACE_GROUP "RCPQ"

typedef NS_LIST_HEAD(rcp_tx_data_req_buffer_t, link)  rcp_buffer_list_t;

typedef struct rcp_tx_queue_s {
    rcp_buffer_list_t   tx_queue;
    uint16_t        tx_queue_size;
    uint16_t        tx_queue_size_high_mark;
    uint16_t        tx_queue_unicast_size;
    uint16_t        tx_queue_unicast_high_mark;
    bool            unicast_tx_active;

    uint16_t    num_tx_unicast_enque;
    uint16_t    num_tx_unicast_deque;
    uint16_t    num_error_alloc;
    uint16_t    num_reject_queue_full;
    uint16_t    num_reject_unicast_in_progress;
    uint16_t    num_reject_broadcast_in_progress;
    uint16_t    num_reject_async_in_progress;
    uint16_t    num_data_req_timeout;
    uint16_t    num_unicast_timeout;
    uint16_t    num_broadcast_timeout;
    uint16_t    num_async_timeout;
    uint16_t    num_msdu_handle_not_found;
} rcp_tx_queue_t;

struct rcp_tx_queue_s rcp_tx_queue;

extern void rcp_process_data_request(const mcps_data_req_t *data, const mcps_data_req_ie_list_t *ie_ext, const channel_list_s *asynch_channel_list,
                                    mac_data_priority_t priority);
extern int8_t mcps_sap_rcp_data_cnf(rcp_data_cnf_t *data_cnf);

static bool rcp_tx_data_queue_unicast_allow();
static void rcp_tx_data_queue_set_unicast_state(bool active);
static rcp_data_req_type rcp_tx_data_packet_type(const mcps_data_req_t *data_req, const channel_list_s *pAsynCh);
static rcp_tx_data_req_buffer_t *rcp_tx_data_queue_read();

static bool rcp_tx_data_queue_unicast_allow()
{
    return !(rcp_tx_queue.unicast_tx_active);
}

static void rcp_tx_data_queue_set_unicast_state(bool active)
{
    rcp_tx_queue.unicast_tx_active = active;
}

static rcp_data_req_type rcp_tx_data_packet_type(const mcps_data_req_t *data_req, const channel_list_s *pAsynCh)
{
    rcp_data_req_type data_req_type;

    if (pAsynCh != NULL) {
        data_req_type = RCP_DATA_REQ_ASYNC;
    } else {
        if (data_req->TxAckReq) {
            data_req_type = RCP_DATA_REQ_UNICAST;
        } else {
            data_req_type = RCP_DATA_REQ_BROADCAST;
        }
    }
    return data_req_type;
}

void rcp_tx_data_queue_init()
{
    ns_list_init(&(rcp_tx_queue.tx_queue));
}

void rcp_tx_data_queue_tx_confirm_handler(uint8_t msdu_handle)
{
    rcp_tx_data_req_buffer_t *tx_req_match = NULL;
    rcp_tx_data_req_buffer_t *tx_req_new = NULL;

    /* Dequeue packet after CNF received */
    ns_list_foreach_safe(rcp_tx_data_req_buffer_t, buf, &rcp_tx_queue.tx_queue) {
        if (buf->data_req.msduHandle == msdu_handle) {
            tx_req_match = buf;
            ns_list_remove(&rcp_tx_queue.tx_queue, buf);
            break;
        }
    }

    if (tx_req_match == NULL) {
        rcp_tx_queue.num_msdu_handle_not_found++;
    }

#ifdef RCP_QUEUE_CONFIG_ENQUEUE_UNICAST
    if (tx_req_match != NULL && (tx_req_match->req_type == RCP_DATA_REQ_UNICAST)) {
        rcp_tx_queue.num_tx_unicast_deque++;
        rcp_tx_queue.tx_queue_unicast_size--;

        /* set unicast active to false */
        rcp_tx_data_queue_set_unicast_state(false);

        /* check RCP TX queue to see if TX is allowed */
        // Get the first unicast packet from the queue
        ns_list_foreach_safe(rcp_tx_data_req_buffer_t, buf, &rcp_tx_queue.tx_queue) {
            if (buf->req_type == RCP_DATA_REQ_UNICAST) {
                // Do not dequeue immediately. Dequeue only after CNF handler is received (triggered by failure event)
                tx_req_new = buf;
                tx_req_new->timeout = RCP_QUEUE_DATA_REQ_TIMEOUT_SEC; // Reset timeout after TX
                break;
            }
        }
        if (tx_req_new) {
            rcp_tx_data_queue_set_unicast_state(true);
            // send data request (pending) to RCP
            rcp_process_data_request(&(tx_req_new->data_req), &(tx_req_new->ie_ext), NULL, tx_req_new->priority);
        }
    }
#endif

    // free tx_req_match buffer
    if (tx_req_match) {
        ns_dyn_mem_free(tx_req_match);
    }
    return;
}

uint8_t rcp_tx_data_queue_tx_request_handler(const mcps_data_req_t *data,
                                        const mcps_data_req_ie_list_t *ie_ext,
                                        const channel_list_s *asynch_channel_list, mac_data_priority_t priority)
{
    uint8_t ret = RCP_QUEUE_DATA_REQ_CONTINUE;
    rcp_tx_data_req_buffer_t *txReq;

    rcp_data_req_type data_req_type = rcp_tx_data_packet_type(data, asynch_channel_list);

    if (ns_list_count(&rcp_tx_queue.tx_queue) >= RCP_QUEUE_MAX_SIZE) {
        rcp_tx_queue.num_reject_queue_full++;
        return RCP_QUEUE_DATA_REQ_REJECT;
    }

    /* Enqueue all packets, no matter req type */
    txReq = ns_dyn_mem_temporary_alloc(sizeof(rcp_tx_data_req_buffer_t) );
    if (!txReq) {
        // there is no heap space, can not push into queue
        rcp_tx_queue.num_error_alloc++;
        return RCP_QUEUE_DATA_REQ_REJECT;
    }
    txReq->req_type = data_req_type;
    txReq->data_req = *data;
    txReq->ie_ext   = *ie_ext;
    txReq->priority = priority;
    txReq->timeout = RCP_QUEUE_DATA_REQ_TIMEOUT_SEC;

    /* rcp TX queue is implemented as FIFO
       if we need to handle priority in RCP TX queue, we need to sort the buffer
       see function: lowpan_adaptation_tx_queue_write
    */
#ifdef RCP_QUEUE_CONFIG_ENQUEUE_UNICAST
    /* Only block TX if unicast */
    if (txReq->req_type == RCP_DATA_REQ_UNICAST) {
        /* check RCP TX queue to see if TX is allowed, there is no pending packet*/
        if ( (rcp_tx_data_queue_unicast_allow()) && (rcp_tx_queue.tx_queue_unicast_size == 0 ) )
        {
            // there is no pending packet
            rcp_tx_data_queue_set_unicast_state(true);
            ret = RCP_QUEUE_DATA_REQ_CONTINUE;
        } else {
            txReq->timeout = RCP_QUEUE_DATA_REQ_TIMEOUT_PENDING_SEC;
            ret = RCP_QUEUE_DATA_REQ_QUEUED;
        }

        /* Update debug stats */
        rcp_tx_queue.num_tx_unicast_enque++;
        rcp_tx_queue.tx_queue_unicast_size++;
        /* update the TX queue high water mark */
        if (rcp_tx_queue.tx_queue_unicast_size > rcp_tx_queue.tx_queue_unicast_high_mark) {
            rcp_tx_queue.tx_queue_unicast_high_mark = rcp_tx_queue.tx_queue_unicast_size;
        }
    }
#endif

    ns_list_add_to_end(&(rcp_tx_queue.tx_queue), txReq);
    rcp_tx_queue.tx_queue_size = ns_list_count(&(rcp_tx_queue.tx_queue));
    if (rcp_tx_queue.tx_queue_size > rcp_tx_queue.tx_queue_size_high_mark) {
        rcp_tx_queue.tx_queue_size_high_mark = rcp_tx_queue.tx_queue_size;
    }
    return ret;
}

void rcp_queue_slow_timer(uint16_t seconds)
{
    rcp_data_cnf_t data_cnf_reject = {0};

    ns_list_foreach_safe(rcp_tx_data_req_buffer_t, buf, &rcp_tx_queue.tx_queue) {
        if ((buf->timeout != RCP_QUEUE_DATA_REQ_TIMEOUT_DISABLE) && (buf->timeout > seconds)) {
            buf->timeout -= seconds;
        } else if ((buf->timeout != RCP_QUEUE_DATA_REQ_TIMEOUT_DISABLE)) {
            buf->timeout = RCP_QUEUE_DATA_REQ_TIMEOUT_DISABLE; // Prevent retriggering timeout
            /* Do not dequeue immediately. Dequeue only after CNF handler is received (triggered by failure event) */
            rcp_tx_queue.num_data_req_timeout++;
            if (buf->req_type == RCP_DATA_REQ_UNICAST) {
                rcp_tx_queue.num_unicast_timeout++;
            } else if (buf->req_type == RCP_DATA_REQ_BROADCAST) {
                rcp_tx_queue.num_broadcast_timeout++;
            } else if (buf->req_type == RCP_DATA_REQ_ASYNC) {
                rcp_tx_queue.num_async_timeout++;
            }

            data_cnf_reject.cnf_type = buf->req_type;
            data_cnf_reject.status = MLME_RCP_QUEUE_TIMEOUT;
            data_cnf_reject.msdu_handle = buf->data_req.msduHandle;
            data_cnf_reject.key_index = INVALID_KEY_INDEX;
            mcps_sap_rcp_data_cnf(&data_cnf_reject);
        }
    }
}