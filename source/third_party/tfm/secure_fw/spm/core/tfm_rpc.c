/*
 * Copyright (c) 2019-2022, Arm Limited. All rights reserved.
 * Copyright (c) 2023 Cypress Semiconductor Corporation (an Infineon company)
 * or an affiliate of Cypress Semiconductor Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "async.h"
#include "config_impl.h"
#include "spm.h"
#include "ffm/agent_api.h"
#include "ffm/psa_api.h"
#include "tfm_rpc.h"
#include "utilities.h"
#include "load/partition_defs.h"
#include "tfm_psa_call_pack.h"

static void default_handle_req(void)
{
}

static void default_mailbox_reply(const void *owner, int32_t ret)
{
    (void)owner;
    (void)ret;
}

static const void *default_get_caller_data(int32_t client_id)
{
    (void)client_id;

    return NULL;
}

static struct tfm_rpc_ops_t rpc_ops = {
    .handle_req = default_handle_req,
    .reply      = default_mailbox_reply,
    .get_caller_data = default_get_caller_data,
};

uint32_t tfm_rpc_psa_framework_version(void)
{
    return tfm_spm_client_psa_framework_version();
}

uint32_t tfm_rpc_psa_version(uint32_t sid)
{
    return tfm_spm_client_psa_version(sid);
}

psa_status_t tfm_rpc_psa_call(psa_handle_t handle, uint32_t control,
                              const struct client_params_t *params,
                              const void *client_data_stateless)
{
    SPM_ASSERT(params != NULL);

    return agent_psa_call(handle, control, params, client_data_stateless);
}

/* Following PSA APIs are only needed by connection-based services */
#if CONFIG_TFM_CONNECTION_BASED_SERVICE_API == 1

psa_status_t tfm_rpc_psa_connect(uint32_t sid,
                                 uint32_t version,
                                 int32_t ns_client_id,
                                 const void *client_data)
{
    return agent_psa_connect(sid, version, ns_client_id, client_data);
}

void tfm_rpc_psa_close(psa_handle_t handle)
{
    psa_close(handle);
}

#endif /* CONFIG_TFM_CONNECTION_BASED_SERVICE_API */

int32_t tfm_rpc_register_ops(const struct tfm_rpc_ops_t *ops_ptr)
{
    if (!ops_ptr) {
        return TFM_RPC_INVAL_PARAM;
    }

    if (!ops_ptr->handle_req || !ops_ptr->reply || !ops_ptr->get_caller_data) {
        return TFM_RPC_INVAL_PARAM;
    }

    /* Currently, one and only one mailbox implementation is supported. */
    if ((rpc_ops.handle_req != default_handle_req) ||
        (rpc_ops.reply != default_mailbox_reply) ||
        (rpc_ops.get_caller_data != default_get_caller_data)) {
        return TFM_RPC_CONFLICT_CALLBACK;
    }

    rpc_ops.handle_req = ops_ptr->handle_req;
    rpc_ops.reply = ops_ptr->reply;
    rpc_ops.get_caller_data = ops_ptr->get_caller_data;

    return TFM_RPC_SUCCESS;
}

void tfm_rpc_unregister_ops(void)
{
    rpc_ops.handle_req = default_handle_req;
    rpc_ops.reply = default_mailbox_reply;
    rpc_ops.get_caller_data = default_get_caller_data;
}

void tfm_rpc_client_call_handler(void)
{
    rpc_ops.handle_req();
}

#if CONFIG_TFM_SPM_BACKEND_IPC == 1
void tfm_rpc_client_call_reply(void)
{
    psa_msg_t msg;
    psa_status_t status = psa_get(ASYNC_MSG_REPLY, &msg);
    struct connection_t *handle = (struct connection_t *)msg.rhandle;

    rpc_ops.reply(handle->caller_data, status);

    if (handle->status == TFM_HANDLE_STATUS_TO_FREE) {
        spm_free_connection(handle);
    } else {
        handle->status = TFM_HANDLE_STATUS_IDLE;
    }
}
#endif /* CONFIG_TFM_SPM_BACKEND_IPC == 1 */

void tfm_rpc_set_caller_data(struct connection_t *handle, int32_t client_id)
{
    handle->caller_data = rpc_ops.get_caller_data(client_id);
}
