/*
 * Copyright (c) 2022, Arm Limited. All rights reserved.
 * Copyright (c) 2023 Cypress Semiconductor Corporation (an Infineon company)
 * or an affiliate of Cypress Semiconductor Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "rss_comms.h"

#include <stddef.h>
#include <stdint.h>

#include "rss_comms_hal.h"
#include "rss_comms_queue.h"
#include "tfm_rpc.h"
#include "tfm_multi_core.h"
#include "tfm_psa_call_pack.h"
#include "tfm_spm_log.h"
#include "rss_comms_permissions_hal.h"

static struct client_request_t *req_to_process;

static psa_status_t message_dispatch(struct client_request_t *req)
{
    enum tfm_plat_err_t plat_err;

    /* Create the call parameters */
    struct client_params_t params = {
        .ns_client_id_stateless = -((int32_t)req->client_id),
        .p_invecs = req->in_vec,
        .p_outvecs = req->out_vec,
    };

    SPMLOG_DBGMSG("[RSS-COMMS] Dispatching message\r\n");
    SPMLOG_DBGMSGVAL("handle=", req->handle);
    SPMLOG_DBGMSGVAL("type=", req->type);
    SPMLOG_DBGMSGVAL("in_len=", req->in_len);
    SPMLOG_DBGMSGVAL("out_len=", req->out_len);
    if (req->in_len > 0) {
        SPMLOG_DBGMSGVAL("in_vec[0].len=", req->in_vec[0].len);
    }
    if (req->in_len > 1) {
        SPMLOG_DBGMSGVAL("in_vec[1].len=", req->in_vec[1].len);
    }
    if (req->in_len > 2) {
        SPMLOG_DBGMSGVAL("in_vec[2].len=", req->in_vec[2].len);
    }
    if (req->in_len > 3) {
        SPMLOG_DBGMSGVAL("in_vec[3].len=", req->in_vec[3].len);
    }
    if (req->out_len > 0) {
        SPMLOG_DBGMSGVAL("out_vec[0].len=", req->out_vec[0].len);
    }
    if (req->out_len > 1) {
        SPMLOG_DBGMSGVAL("out_vec[1].len=", req->out_vec[1].len);
    }
    if (req->out_len > 2) {
        SPMLOG_DBGMSGVAL("out_vec[2].len=", req->out_vec[2].len);
    }
    if (req->out_len > 3) {
        SPMLOG_DBGMSGVAL("out_vec[3].len=", req->out_vec[3].len);
    }

    plat_err = comms_permissions_service_check(req->handle,
                                               req->in_vec,
                                               req->in_len,
                                               req->type);
    if (plat_err != TFM_PLAT_ERR_SUCCESS) {
        SPMLOG_ERRMSG("[RSS-COMMS] Call not permitted\r\n");
        return PSA_ERROR_NOT_PERMITTED;
    }

    return tfm_rpc_psa_call(req->handle,
                            PARAM_PACK(req->type,
                                       req->in_len,
                                       req->out_len),
                            &params,
                            NULL);
}

static void rss_comms_reply(const void *owner, int32_t ret)
{
    struct client_request_t *req = (struct client_request_t *)owner;

    req->return_val = ret;

    SPMLOG_DBGMSG("[RSS-COMMS] Sending reply\r\n");
    SPMLOG_DBGMSGVAL("protocol_ver=", req->protocol_ver);
    SPMLOG_DBGMSGVAL("seq_num=", req->seq_num);
    SPMLOG_DBGMSGVAL("client_id=", req->client_id);
    SPMLOG_DBGMSGVAL("return_val=", req->return_val);
    SPMLOG_DBGMSGVAL("out_vec[0].len=", req->out_vec[0].len);
    SPMLOG_DBGMSGVAL("out_vec[1].len=", req->out_vec[1].len);
    SPMLOG_DBGMSGVAL("out_vec[2].len=", req->out_vec[2].len);
    SPMLOG_DBGMSGVAL("out_vec[3].len=", req->out_vec[3].len);

    if (tfm_multi_core_hal_reply(req) != TFM_PLAT_ERR_SUCCESS) {
        SPMLOG_DBGMSG("[RSS-COMMS] Sending reply failed!\r\n");
    }
}

static void rss_comms_handle_req(void)
{
    psa_status_t status;
    void *queue_entry;

    /* FIXME: consider memory limitations that may prevent dispatching all
     * messages in one go.
     */
    while (queue_dequeue(&queue_entry) == 0) {
        /* Deliver PSA Client call request to handler in SPM. */
        req_to_process = queue_entry;
        status = message_dispatch(req_to_process);
#if CONFIG_TFM_SPM_BACKEND_IPC == 1
        /*
         * If status == PSA_SUCCESS, peer will be replied when mailbox agent
         * partition receives a 'ASYNC_MSG_REPLY' signal from the requested
         * service partition.
         * If status != PSA_SUCCESS, the service call has been finished.
         * Reply to the peer directly.
         */
        if (status != PSA_SUCCESS) {
            SPMLOG_DBGMSGVAL("[RSS-COMMS] Message dispatch failed: ", status);
            rss_comms_reply(req_to_process, status);
        }
#else
        /* In SFN model, the service call has been finished. Reply to the peer directly. */
        rss_comms_reply(req_to_process, status);
#endif
    }
}

static const void *rss_comms_get_caller_data(int32_t client_id)
{
    (void)client_id;

    return req_to_process;
}

static struct tfm_rpc_ops_t rpc_ops = {
    .handle_req = rss_comms_handle_req,
    .reply = rss_comms_reply,
    .get_caller_data = rss_comms_get_caller_data,
};

int32_t tfm_inter_core_comm_init(void)
{
    int32_t ret;

    /* Register RPC callbacks */
    ret = tfm_rpc_register_ops(&rpc_ops);
    if (ret != TFM_RPC_SUCCESS) {
        return ret;
    }

    /* Platform specific initialization */
    ret = tfm_multi_core_hal_init();
    if (ret != TFM_PLAT_ERR_SUCCESS) {
        tfm_rpc_unregister_ops();
        return ret;
    }

    return TFM_RPC_SUCCESS;
}
