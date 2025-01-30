/*
 * Copyright (c) 2019-2023, Arm Limited. All rights reserved.
 * Copyright (c) 2022-2023 Cypress Semiconductor Corporation (an Infineon company)
 * or an affiliate of Cypress Semiconductor Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

/*
 * Definitions of Remote Procedure Call (RPC) functionalities in TF-M, which
 * sits between upper TF-M SPM and underlying mailbox implementation.
 */

#ifndef __TFM_RPC_H__
#define __TFM_RPC_H__

#ifdef TFM_PARTITION_NS_AGENT_MAILBOX

#include <stdbool.h>
#include <stdint.h>
#include "cmsis_compiler.h"
#include "psa/client.h"
#include "psa/service.h"
#include "thread.h"
#include "spm.h"
#include "ffm/agent_api.h"

#define TFM_RPC_SUCCESS             (0)
#define TFM_RPC_INVAL_PARAM         (INT32_MIN + 1)
#define TFM_RPC_CONFLICT_CALLBACK   (INT32_MIN + 2)

/*
 * The underlying mailbox communication implementation should provide
 * the specific operations to complete the RPC functionalities.
 *
 * It includes the following operations:
 * handle_req() - Handle PSA client call request from NSPE
 * reply()      - Reply PSA client call return result to NSPE. The parameter
 *                owner identifies the owner of the PSA client call.
 * get_caller_data() - Get the private data of NSPE client from mailbox to
 *                     identify the PSA client call.
 */
struct tfm_rpc_ops_t {
    void (*handle_req)(void);
    void (*reply)(const void *owner, int32_t ret);
    const void * (*get_caller_data)(int32_t client_id);
};

/**
 * \brief RPC handler for \ref psa_framework_version.
 *
 * \return version              The version of the PSA Framework implementation
 *                              that is providing the runtime services.
 */
uint32_t tfm_rpc_psa_framework_version(void);

/**
 * \brief RPC handler for \ref psa_version.
 *
 * \param[in] sid               RoT Service identity.
 *
 * \retval PSA_VERSION_NONE     The RoT Service is not implemented, or the
 *                              caller is not permitted to access the service.
 * \retval > 0                  The version of the implemented RoT Service.
 */
uint32_t tfm_rpc_psa_version(uint32_t sid);

/**
 * \brief RPC handler for \ref psa_connect.
 *
 * \param[in] sid               RoT Service identity.
 * \param[in] version           The version of the RoT Service.
 * \param[in] ns_client_id      Agent representing NS client's identifier.
 * \param[in] client_data       Client data, treated as opaque by SPM.
 *
 * \retval PSA_SUCCESS          Success.
 * \retval PSA_CONNECTION_BUSY  The SPM cannot make the connection
 *                              at the moment.
 * \retval "Does not return"    The RoT Service ID and version are not
 *                              supported, or the caller is not permitted to
 *                              access the service.
 */
psa_status_t tfm_rpc_psa_connect(uint32_t sid,
                                 uint32_t version,
                                 int32_t ns_client_id,
                                 const void *client_data);

/**
 * \brief RPC handler for \ref psa_call.
 *
 * \param[in] handle                 Handle to the service being accessed.
 * \param[in] control                A composited uint32_t value for controlling purpose,
 *                                   containing call types, numbers of in/out vectors and
 *                                   attributes of vectors.
 * \param[in] params                 Combines the psa_invec and psa_outvec params
 *                                   for the psa_call() to be made, as well as
 *                                   NS agent's client identifier, which is ignored
 *                                   for connection-based services.
 * \param[in] client_data_stateless  Client data, treated as opaque by SPM.
 *
 * \retval PSA_SUCCESS               Success.
 * \retval "Does not return"         The call is invalid, one or more of the
 *                                   following are true:
 * \arg                                An invalid handle was passed.
 * \arg                                The connection is already handling a request.
 * \arg                                An invalid memory reference was provided.
 * \arg                                in_num + out_num > PSA_MAX_IOVEC.
 * \arg                                The message is unrecognized by the RoT
 *                                     Service or incorrectly formatted.
 */
psa_status_t tfm_rpc_psa_call(psa_handle_t handle, uint32_t control,
                              const struct client_params_t *params,
                              const void *client_data_stateless);

/**
 * \brief RPC handler for \ref psa_close.
 *
 * \param[in] handle            A handle to an established connection, or the null handle.
 *
 * \retval void                 Success.
 * \retval "Does not return"    The call is invalid, one or more of the
 *                              following are true:
 * \arg                           An invalid handle was provided that is not
 *                                the null handle..
 */
void tfm_rpc_psa_close(psa_handle_t handle);

/**
 * \brief Register underlying mailbox communication operations.
 *
 * \param[in] ops_ptr           Pointer to the specific operation structure.
 *
 * \retval TFM_RPC_SUCCESS      Mailbox operations are successfully registered.
 * \retval Other error code     Fail to register mailbox operations.
 */
int32_t tfm_rpc_register_ops(const struct tfm_rpc_ops_t *ops_ptr);

/**
 * \brief Unregister underlying mailbox communication operations.
 *
 * Currently one and only one underlying mailbox communication implementation is
 * allowed in runtime. Thus it is unnecessary to specify the mailbox
 * communication operation callbacks to be unregistered.
 *
 * \param[in] void
 */
void tfm_rpc_unregister_ops(void);

/**
 * \brief Handling PSA client call request
 *
 * \param void
 */
void tfm_rpc_client_call_handler(void);

#if CONFIG_TFM_SPM_BACKEND_IPC == 1
/**
 * \brief Reply PSA client call return result
 *
 * \param[in] void
 */
void tfm_rpc_client_call_reply(void);
#endif /* CONFIG_TFM_SPM_BACKEND_IPC == 1 */

/*
 * Check if the message was allocated for a non-secure request via RPC
 *
 * \param[in] handle        The connection handle context pointer
 *                          \ref connection_t structures
 *
 * \retval true             The message was allocated for a NS request via RPC.
 * \retval false            Otherwise.
 */
__STATIC_INLINE bool is_tfm_rpc_msg(const struct connection_t *handle)
{
    /*
     * FIXME
     * The ID should be smaller than 0 if the message is allocated by a
     * non-secure caller.
     * However, current TF-M implementation use 0 as the default non-secure
     * caller ID. Therefore, treat the caller as non-secure when client_id == 0.
     *
     * This condition check should be improved after TF-M non-secure client ID
     * management is implemented.
     */
    if (handle && (handle->caller_data) && (handle->msg.client_id <= 0)) {
        return true;
    }

    return false;
}

/*
 * \brief Set the private data of the NS caller in \ref connection_t, to
 *        identify the caller after PSA client call is compeleted.
 *
 * \param[in] handle        The address of \ref connection_t structure
 * \param[in] client_id     The client ID of the NS caller.
 */
void tfm_rpc_set_caller_data(struct connection_t *handle, int32_t client_id);

#else /* TFM_PARTITION_NS_AGENT_MAILBOX */

/* RPC is only available in multi-core scenario */
#define is_tfm_rpc_msg(x)                       (false)

#define tfm_rpc_client_call_reply(owner, ret)   do {} while (0)

#endif /* TFM_PARTITION_NS_AGENT_MAILBOX */
#endif /* __TFM_RPC_H__ */
