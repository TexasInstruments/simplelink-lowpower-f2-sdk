/*
 * Copyright (c) 2019-2023, Arm Limited. All rights reserved.
 * Copyright (c) 2022-2023 Cypress Semiconductor Corporation (an Infineon
 * company) or an affiliate of Cypress Semiconductor Corporation. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <stdint.h>
#include "async.h"
#include "bitops.h"
#include "config_impl.h"
#include "config_spm.h"
#include "critical_section.h"
#include "internal_status_code.h"
#include "psa/lifecycle.h"
#include "psa/service.h"
#include "spm.h"
#include "tfm_arch.h"
#include "load/partition_defs.h"
#include "load/service_defs.h"
#include "load/interrupt_defs.h"
#include "utilities.h"
#include "ffm/backend.h"
#include "ffm/psa_api.h"
#include "tfm_rpc.h"
#include "tfm_hal_platform.h"
#include "tfm_psa_call_pack.h"
#include "tfm_hal_isolation.h"



void spm_handle_programmer_errors(psa_status_t status)
{
    if (status == PSA_ERROR_PROGRAMMER_ERROR ||
        status == PSA_ERROR_CONNECTION_REFUSED) {
        if (!tfm_spm_is_ns_caller()) {
            tfm_core_panic();
        }
    }
}

uint32_t tfm_spm_get_lifecycle_state(void)
{
    /*
     * FixMe: return PSA_LIFECYCLE_UNKNOWN to the caller directly. It will be
     * implemented in the future.
     */
    return PSA_LIFECYCLE_UNKNOWN;
}

/* PSA Partition API function body */

#if CONFIG_TFM_SPM_BACKEND_IPC == 1 \
    || CONFIG_TFM_FLIH_API == 1 || CONFIG_TFM_SLIH_API == 1
psa_signal_t tfm_spm_partition_psa_wait(psa_signal_t signal_mask,
                                        uint32_t timeout)
{
    struct partition_t *partition = NULL;
    psa_signal_t signal;

    /*
     * Timeout[30:0] are reserved for future use.
     * SPM must ignore the value of RES.
     */
    timeout &= PSA_TIMEOUT_MASK;

    partition = GET_CURRENT_COMPONENT();

    /*
     * signals_allowed can be 0 for TF-M internal partitions for special usages.
     * Regular Secure Partitions should have at least one signal.
     * This is gauranteed by the manifest tool.
     * It is a PROGRAMMER ERROR if the signal_mask does not include any assigned
     * signals.
     */
    if ((partition->signals_allowed) &&
        (partition->signals_allowed & signal_mask) == 0) {
        tfm_core_panic();
    }

    /*
     * After new signal(s) are available, the return value will be updated in
     * PendSV and blocked thread gets to run.
     */
    if (timeout == PSA_BLOCK) {
        signal = backend_wait_signals(partition, signal_mask);
        if (signal == (psa_signal_t)0) {
            signal = (psa_signal_t)STATUS_NEED_SCHEDULE;
        }
    } else {
        signal = partition->signals_asserted & signal_mask;
    }

    return signal;
}
#endif

#if CONFIG_TFM_SPM_BACKEND_IPC == 1
psa_status_t tfm_spm_partition_psa_get(psa_signal_t signal, psa_msg_t *msg)
{
    psa_status_t ret = PSA_ERROR_GENERIC_ERROR;
    struct connection_t *handle = NULL;
    struct partition_t *partition = NULL;
    fih_int fih_rc = FIH_FAILURE;

    /*
     * Only one message could be retrieved every time for psa_get(). It is a
     * fatal error if the input signal has more than a signal bit set.
     */
    if (!IS_ONLY_ONE_BIT_IN_UINT32(signal)) {
        tfm_core_panic();
    }

    partition = GET_CURRENT_COMPONENT();

    /*
     * Write the message to the service buffer. It is a fatal error if the
     * input msg pointer is not a valid memory reference or not read-write.
     */
    FIH_CALL(tfm_hal_memory_check, fih_rc,
             partition->boundary, (uintptr_t)msg,
             sizeof(psa_msg_t), TFM_HAL_ACCESS_READWRITE);
    if (fih_not_eq(fih_rc, fih_int_encode(PSA_SUCCESS))) {
        tfm_core_panic();
    }

    /*
     * It is a fatal error if the caller call psa_get() when no message has
     * been set. The caller must call this function after an RoT Service signal
     * is returned by psa_wait().
     */
    if (partition->signals_asserted == 0) {
        tfm_core_panic();
    }

    /*
     * It is a fatal error if the RoT Service signal is not currently asserted.
     */
    if ((partition->signals_asserted & signal) == 0) {
        tfm_core_panic();
    }

    if (signal == ASYNC_MSG_REPLY) {
        struct connection_t **pr_handle_iter, **prev = NULL;
        struct critical_section_t cs_assert = CRITICAL_SECTION_STATIC_INIT;

        /* Remove tail of the list, which is the first item added */
        CRITICAL_SECTION_ENTER(cs_assert);
        if (!partition->p_handles) {
            tfm_core_panic();
        }
        UNI_LIST_FOREACH_NODE_PNODE(pr_handle_iter, handle,
                                    partition, p_handles) {
            prev = pr_handle_iter;
        }
        handle = *prev;
        UNI_LIST_REMOVE_NODE_BY_PNODE(prev, p_handles);
        ret = handle->reply_value;
        /* Clear the signal if there are no more asynchronous responses waiting */
        if (!partition->p_handles) {
            partition->signals_asserted &= ~ASYNC_MSG_REPLY;
        }
        CRITICAL_SECTION_LEAVE(cs_assert);
    } else {
        /*
         * Get message by signal from partition. It is a fatal error if getting
         * failed, which means the input signal does not correspond to an RoT service.
         */
        handle = spm_get_handle_by_signal(partition, signal);
        if (handle) {
            ret = PSA_SUCCESS;
        } else {
            return PSA_ERROR_DOES_NOT_EXIST;
        }
    }

    spm_memcpy(msg, &handle->msg, sizeof(psa_msg_t));

    return ret;
}
#endif

psa_status_t tfm_spm_partition_psa_reply(psa_handle_t msg_handle,
                                         psa_status_t status)
{
    struct service_t *service;
    struct connection_t *handle;
    psa_status_t ret = PSA_SUCCESS;
    struct critical_section_t cs_assert = CRITICAL_SECTION_STATIC_INIT;

    /* It is a fatal error if message handle is invalid */
    handle = spm_msg_handle_to_connection(msg_handle);
    if (!handle) {
        tfm_core_panic();
    }

    /*
     * RoT Service information is needed in this function, stored it in message
     * body structure. Only two parameters are passed in this function: handle
     * and status, so it is useful and simply to do like this.
     */
    service = handle->service;
    if (!service) {
        tfm_core_panic();
    }

    switch (handle->msg.type) {
    case PSA_IPC_CONNECT:
        /*
         * Reply to PSA_IPC_CONNECT message. Connect handle is returned if the
         * input status is PSA_SUCCESS. Others return values are based on the
         * input status.
         */
        if (status == PSA_SUCCESS) {
            ret = msg_handle;
        } else if (status == PSA_ERROR_CONNECTION_REFUSED) {
            /* Refuse the client connection, indicating a permanent error. */
            ret = PSA_ERROR_CONNECTION_REFUSED;
            handle->status = TFM_HANDLE_STATUS_TO_FREE;
        } else if (status == PSA_ERROR_CONNECTION_BUSY) {
            /* Fail the client connection, indicating a transient error. */
            ret = PSA_ERROR_CONNECTION_BUSY;
        } else {
            tfm_core_panic();
        }
        break;
    case PSA_IPC_DISCONNECT:
        /* Service handle is not used anymore */
        handle->status = TFM_HANDLE_STATUS_TO_FREE;

        /*
         * If the message type is PSA_IPC_DISCONNECT, then the status code is
         * ignored
         */
        break;
    default:
        if (handle->msg.type >= PSA_IPC_CALL) {

#if PSA_FRAMEWORK_HAS_MM_IOVEC
            /*
             * Any output vectors that are still mapped will report that
             * zero bytes have been written.
             */
            for (int i = OUTVEC_IDX_BASE; i < PSA_MAX_IOVEC * 2; i++) {
                if (IOVEC_IS_MAPPED(handle, i) && (!IOVEC_IS_UNMAPPED(handle, i))) {
                    handle->outvec_written[i - OUTVEC_IDX_BASE] = 0;
                }
            }
#endif
            /* Reply to a request message. Return values are based on status */
            ret = status;
            /*
             * The total number of bytes written to a single parameter must be
             * reported to the client by updating the len member of the
             * psa_outvec structure for the parameter before returning from
             * psa_call().
             */
            update_caller_outvec_len(handle);
            if (SERVICE_IS_STATELESS(service->p_ldinf->flags)) {
                handle->status = TFM_HANDLE_STATUS_TO_FREE;
            }
        } else {
            tfm_core_panic();
        }
    }

    if (ret == PSA_ERROR_PROGRAMMER_ERROR) {
        /*
         * If the source of the programmer error is a Secure Partition, the SPM
         * must panic the Secure Partition in response to a PROGRAMMER ERROR.
         */
        if (!TFM_CLIENT_ID_IS_NS(handle->msg.client_id)) {
            tfm_core_panic();
        }
    }

    /*
     * TODO: It can be optimized further by moving critical section protection
     * to mailbox. Also need to check implementation when secure context is
     * involved.
     */
    CRITICAL_SECTION_ENTER(cs_assert);
    ret = backend_replying(handle, ret);
    CRITICAL_SECTION_LEAVE(cs_assert);

    /*
     * When IPC model is using the asynchronous agent API, retain the handle
     * until the response has been collected by the agent.
     */
#if CONFIG_TFM_SPM_BACKEND_IPC == 1
    if (is_tfm_rpc_msg(handle)) {
        return ret;
    }
#endif

    /*
     * When the asynchronous agent API is not used or when in SFN model, free
     * the connection handle immediately.
     */
    if (handle->status == TFM_HANDLE_STATUS_TO_FREE) {
        spm_free_connection(handle);
    } else {
        handle->status = TFM_HANDLE_STATUS_IDLE;
    }

    return ret;
}

#if CONFIG_TFM_DOORBELL_API == 1
psa_status_t tfm_spm_partition_psa_notify(int32_t partition_id)
{
    struct partition_t *p_pt = tfm_spm_get_partition_by_id(partition_id);

    return backend_assert_signal(p_pt, PSA_DOORBELL);
}

psa_status_t tfm_spm_partition_psa_clear(void)
{
    struct critical_section_t cs_assert = CRITICAL_SECTION_STATIC_INIT;
    struct partition_t *partition = NULL;

    partition = GET_CURRENT_COMPONENT();

    /*
     * It is a fatal error if the Secure Partition's doorbell signal is not
     * currently asserted.
     */
    if ((partition->signals_asserted & PSA_DOORBELL) == 0) {
        tfm_core_panic();
    }

    CRITICAL_SECTION_ENTER(cs_assert);
    partition->signals_asserted &= ~PSA_DOORBELL;
    CRITICAL_SECTION_LEAVE(cs_assert);

    return PSA_SUCCESS;
}
#endif /* CONFIG_TFM_DOORBELL_API == 1 */

psa_status_t tfm_spm_partition_psa_panic(void)
{
#ifdef CONFIG_TFM_HALT_ON_CORE_PANIC
    tfm_hal_system_halt();
#else
    /*
     * PSA FF recommends that the SPM causes the system to restart when a secure
     * partition panics.
     */
    tfm_hal_system_reset();
#endif

    /* Execution should not reach here */
    return PSA_ERROR_GENERIC_ERROR;
}
