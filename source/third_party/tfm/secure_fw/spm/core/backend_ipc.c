/*
 * Copyright (c) 2021-2023, Arm Limited. All rights reserved.
 * Copyright (c) 2021-2023 Cypress Semiconductor Corporation (an Infineon
 * company) or an affiliate of Cypress Semiconductor Corporation. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <stdint.h>
#include "aapcs_local.h"
#include "async.h"
#include "critical_section.h"
#include "compiler_ext_defs.h"
#include "config_spm.h"
#include "ffm/psa_api.h"
#include "runtime_defs.h"
#include "stack_watermark.h"
#include "spm.h"
#include "tfm_hal_isolation.h"
#include "tfm_hal_platform.h"
#include "tfm_rpc.h"
#include "ffm/backend.h"
#include "utilities.h"
#include "memory_symbols.h"
#include "load/partition_defs.h"
#include "load/service_defs.h"
#include "load/spm_load_api.h"
#include "psa/error.h"
#include "internal_status_code.h"

/* Declare the global component list */
struct partition_head_t partition_listhead;

#if TFM_ISOLATION_LEVEL > 1
extern uintptr_t spm_boundary;
#endif

#ifdef CONFIG_TFM_USE_TRUSTZONE
/* Instance for SPM_THREAD_CONTEXT */
struct context_ctrl_t *p_spm_thread_context;
#else
/* If ns_agent_tz isn't used, we need to provide a stack for SPM to use */
static uint8_t spm_thread_stack[CONFIG_TFM_SPM_THREAD_STACK_SIZE] __aligned(8);
ARCH_CLAIM_CTXCTRL_INSTANCE(spm_thread_context,
                            spm_thread_stack,
                            sizeof(spm_thread_stack));

struct context_ctrl_t *p_spm_thread_context = &spm_thread_context;
#endif

/* Indicator point to the partition meta */
uintptr_t *partition_meta_indicator_pos;

/*
 * Query the state of current thread.
 */
static uint32_t query_state(struct thread_t *p_thrd, uint32_t *p_retval)
{
    struct critical_section_t cs_signal = CRITICAL_SECTION_STATIC_INIT;
    struct partition_t *p_pt = NULL;
    uint32_t state = p_thrd->state;
    psa_signal_t retval_signals = 0;

    /* Get current partition of thread. */
    p_pt = TO_CONTAINER(p_thrd->p_context_ctrl,
                        struct partition_t, ctx_ctrl);

    CRITICAL_SECTION_ENTER(cs_signal);

    retval_signals = p_pt->signals_waiting & p_pt->signals_asserted;

    if (retval_signals) {
        /*
         * Signal "ASYNC_MSG_REPLY" can only be waited in one of cases below:
         *
         *   - A FF-M Secure Partition is calling the Client API and
         *     expecting a replied "handle/status" from RoT Services.
         *     FF-M Secure Partitions cannot use 'psa_wait' to wait
         *     on this signal because the signal is not set in FF-M
         *     Secure Partitions' "signals_allowed".
         *
         *   - A Mailbox NS Agent is calling "psa_wait" with a pattern
         *     containing "ASYNC_MSG_REPLY". The signal is set in
         *     Mailbox NS Agents' "signals_allowed".
         *
         * Here uses "signals_allowed" to check if the calling target is a
         * FF-M Secure Partition or a Mailbox NS Agent.
         */
        if ((retval_signals ==  ASYNC_MSG_REPLY) &&
            ((p_pt->signals_allowed & ASYNC_MSG_REPLY) != ASYNC_MSG_REPLY)) {
            p_pt->signals_asserted &= ~ASYNC_MSG_REPLY;
            *p_retval = (uint32_t)p_pt->reply_value;
        } else {
            *p_retval = retval_signals;
        }

        /* Clear 'signals_waiting' to indicate the component is not waiting. */
        p_pt->signals_waiting = 0;
        state = THRD_STATE_RET_VAL_AVAIL;
    } else if (p_pt->signals_waiting != 0) {
        /*
         * If the thread is waiting some signals but none of them is asserted,
         * block the thread.
         */
        state = THRD_STATE_BLOCK;
    }

    CRITICAL_SECTION_LEAVE(cs_signal);
    return state;
}

extern struct psa_api_tbl_t psa_api_thread_fn_call;
extern struct psa_api_tbl_t psa_api_svc;

static void prv_process_metadata(struct partition_t *p_pt)
{
    const struct partition_load_info_t *p_pt_ldi;
    const struct service_load_info_t *p_srv_ldi;
    struct context_ctrl_t *ctx_ctrl;
    struct runtime_metadata_t *p_rt_meta;
    service_fn_t *p_sfn_table;
    uint32_t allocate_size;

    p_pt_ldi = p_pt->p_ldinf;
    p_srv_ldi = LOAD_INFO_SERVICE(p_pt_ldi);
    ctx_ctrl = &p_pt->ctx_ctrl;

    /* common runtime metadata */
    allocate_size = sizeof(*p_rt_meta);

    if (!IS_IPC_MODEL(p_pt_ldi)) {
        /* SFN specific metadata - SFN function table */
        allocate_size += sizeof(service_fn_t) * p_pt_ldi->nservices;
    }

    ARCH_CTXCTRL_ALLOCATE_STACK(ctx_ctrl, allocate_size);
    p_rt_meta = (struct runtime_metadata_t *)
                                    ARCH_CTXCTRL_ALLOCATED_PTR(ctx_ctrl);

#if TFM_ISOLATION_LEVEL == 1
    p_rt_meta->psa_fns = &psa_api_thread_fn_call;
#else
    if (tfm_hal_boundary_need_switch(spm_boundary, p_pt->boundary)) {
        p_rt_meta->psa_fns = &psa_api_svc;
    } else {
        p_rt_meta->psa_fns = &psa_api_thread_fn_call;
    }
#endif

    p_rt_meta->entry = p_pt_ldi->entry;
    p_rt_meta->n_sfn = 0;
    p_sfn_table = p_rt_meta->sfn_table;

    if (!IS_IPC_MODEL(p_pt_ldi)) {
        /* SFN table. The signal bit of the service is the same index of SFN. */
        for (int i = 0; i < p_pt_ldi->nservices; i++) {
            p_sfn_table[i] = (service_fn_t)p_srv_ldi[i].sfn;
        }

        p_rt_meta->n_sfn = p_pt_ldi->nservices;
    }

    p_pt->p_metadata = (void *)p_rt_meta;
}

/*
 * Send message and wake up the SP who is waiting on message queue, block the
 * current thread and trigger scheduler.
 */
psa_status_t backend_messaging(struct connection_t *p_connection)
{
    struct partition_t *p_owner = NULL;
    psa_signal_t signal = 0;
    psa_status_t ret = PSA_SUCCESS;

    if (!p_connection || !p_connection->service ||
        !p_connection->service->p_ldinf         ||
        !p_connection->service->partition) {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    p_owner = p_connection->service->partition;
    signal = p_connection->service->p_ldinf->signal;

    UNI_LIST_INSERT_AFTER(p_owner, p_connection, p_handles);

    /* Messages put. Update signals */
    ret = backend_assert_signal(p_owner, signal);

    /*
     * If it is a NS request via RPC, it is unnecessary to block current
     * thread.
     */
    if (is_tfm_rpc_msg(p_connection)) {
        ret = PSA_SUCCESS;
    } else {
        signal = backend_wait_signals(p_connection->p_client, ASYNC_MSG_REPLY);
        if (signal == (psa_signal_t)0) {
            ret = STATUS_NEED_SCHEDULE;
        }
    }

    p_connection->status = TFM_HANDLE_STATUS_ACTIVE;

    return ret;
}

psa_status_t backend_replying(struct connection_t *handle, int32_t status)
{
    struct partition_t *client = handle->p_client;

    if (is_tfm_rpc_msg(handle)) {
        /*
         * Add to the list of outstanding responses.
         * Note that we use the partition's p_handles pointer.
         * This assumes that partitions using the agent API will process all requests
         * asynchronously and will not also provide services of their own.
         */
        handle->reply_value = (uintptr_t)status;
        handle->msg.rhandle = handle;
        UNI_LIST_INSERT_AFTER(client, handle, p_handles);
        return backend_assert_signal(handle->p_client, ASYNC_MSG_REPLY);
    } else {
        handle->p_client->reply_value = (uintptr_t)status;
        return backend_assert_signal(handle->p_client, ASYNC_MSG_REPLY);
    }
}

extern void common_sfn_thread(void *param);

static thrd_fn_t partition_init(struct partition_t *p_pt,
                                uint32_t service_setting, uint32_t *param)
{
    thrd_fn_t thrd_entry;

    (void)param;
    SPM_ASSERT(p_pt);

#if CONFIG_TFM_DOORBELL_API == 1
    p_pt->signals_allowed |= PSA_DOORBELL;
#endif /* CONFIG_TFM_DOORBELL_API == 1 */

    p_pt->signals_allowed |= service_setting;

    /* Allow 'ASYNC_MSG_REPLY' for Mailbox NS Agent. */
    if (IS_NS_AGENT_MAILBOX(p_pt->p_ldinf)) {
        p_pt->signals_allowed |= ASYNC_MSG_REPLY;
    }

    UNI_LISI_INIT_NODE(p_pt, p_handles);

    if (IS_IPC_MODEL(p_pt->p_ldinf)) {
        /* IPC Partition */
        thrd_entry = POSITION_TO_ENTRY(p_pt->p_ldinf->entry, thrd_fn_t);
    } else {
        /* SFN Partition */
        thrd_entry = (thrd_fn_t)common_sfn_thread;
    }
    return thrd_entry;
}

#ifdef CONFIG_TFM_USE_TRUSTZONE
static thrd_fn_t ns_agent_tz_init(struct partition_t *p_pt,
                                  uint32_t service_setting, uint32_t *param)
{
    thrd_fn_t thrd_entry;

    (void)service_setting;
    SPM_ASSERT(p_pt);
    SPM_ASSERT(param);

    /* Get the context from ns_agent_tz */
    SPM_THREAD_CONTEXT = &p_pt->ctx_ctrl;

    thrd_entry = POSITION_TO_ENTRY(p_pt->p_ldinf->entry, thrd_fn_t);

    /* NS agent TZ expects NSPE entry point as the parameter */
    *param = tfm_hal_get_ns_entry_point();
    return thrd_entry;
}
#else
static thrd_fn_t ns_agent_tz_init(struct partition_t *p_pt,
                                  uint32_t service_setting, uint32_t *param)
{
    (void)p_pt;
    (void)service_setting;
    (void)param;
}
#endif

typedef thrd_fn_t (*comp_init_fn_t)(struct partition_t *, uint32_t, uint32_t *);
comp_init_fn_t comp_init_fns[] = {partition_init, ns_agent_tz_init};

/* Parameters are treated as assuredly */
void backend_init_comp_assuredly(struct partition_t *p_pt, uint32_t service_setting)
{
    const struct partition_load_info_t *p_pldi = p_pt->p_ldinf;
    thrd_fn_t thrd_entry;
    uint32_t param;
    int32_t index = PARTITION_TYPE_TO_INDEX(p_pldi->flags);

    ARCH_CTXCTRL_INIT(&p_pt->ctx_ctrl,
                      LOAD_ALLOCED_STACK_ADDR(p_pldi),
                      p_pldi->stack_size);

    watermark_stack(p_pt);

    THRD_INIT(&p_pt->thrd, &p_pt->ctx_ctrl,
              TO_THREAD_PRIORITY(PARTITION_PRIORITY(p_pldi->flags)));

    thrd_entry = (comp_init_fns[index])(p_pt, service_setting, &param);

    prv_process_metadata(p_pt);

    thrd_start(&p_pt->thrd, thrd_entry, THRD_GENERAL_EXIT, (void *)param);
}

uint32_t backend_system_run(void)
{
    uint32_t control;
    struct partition_t *p_cur_pt;
    fih_int fih_rc = FIH_FAILURE;

    SPM_ASSERT(SPM_THREAD_CONTEXT);

#ifndef CONFIG_TFM_USE_TRUSTZONE
    /*
     * TZ NS Agent is mandatory when Trustzone is enabled. SPM borrows its
     * stack to improve the stack usage efficiency.
     * Hence SPM needs to have a dedicated stack when Trustzone is not enabled,
     * and this stack needs to be sealed before upcoming usage.
     */
    ARCH_CTXCTRL_ALLOCATE_STACK(SPM_THREAD_CONTEXT, sizeof(uint64_t));
    arch_seal_thread_stack(ARCH_CTXCTRL_ALLOCATED_PTR(SPM_THREAD_CONTEXT));
#endif

    /* Init thread callback function. */
    thrd_set_query_callback(query_state);

    partition_meta_indicator_pos = (uintptr_t *)PART_LOCAL_STORAGE_PTR_POS;
    control = thrd_start_scheduler(&CURRENT_THREAD);

    p_cur_pt = TO_CONTAINER(CURRENT_THREAD->p_context_ctrl,
                            struct partition_t, ctx_ctrl);

    FIH_CALL(tfm_hal_activate_boundary, fih_rc, p_cur_pt->p_ldinf, p_cur_pt->boundary);
    if (fih_not_eq(fih_rc, fih_int_encode(TFM_HAL_SUCCESS))) {
        tfm_core_panic();
    }

    return control;
}

psa_signal_t backend_wait_signals(struct partition_t *p_pt, psa_signal_t signals)
{
    struct critical_section_t cs_signal = CRITICAL_SECTION_STATIC_INIT;
    psa_signal_t ret;

    if (!p_pt) {
        tfm_core_panic();
    }

    CRITICAL_SECTION_ENTER(cs_signal);

    ret = p_pt->signals_asserted & signals;
    if (ret == (psa_signal_t)0) {
        p_pt->signals_waiting = signals;
    }

    CRITICAL_SECTION_LEAVE(cs_signal);

    return ret;
}

psa_status_t backend_assert_signal(struct partition_t *p_pt, psa_signal_t signal)
{
    struct critical_section_t cs_signal = CRITICAL_SECTION_STATIC_INIT;
    psa_status_t ret = PSA_SUCCESS;

    if (!p_pt) {
        tfm_core_panic();
    }

    CRITICAL_SECTION_ENTER(cs_signal);
    p_pt->signals_asserted |= signal;

    if (p_pt->signals_asserted & p_pt->signals_waiting) {
        ret = STATUS_NEED_SCHEDULE;
    }
    CRITICAL_SECTION_LEAVE(cs_signal);

    return ret;
}

uint64_t backend_abi_entering_spm(void)
{
    struct partition_t *caller = GET_CURRENT_COMPONENT();
    uint32_t sp = 0;
    uint32_t sp_limit = 0;
    AAPCS_DUAL_U32_T spm_stack_info;

#if TFM_ISOLATION_LEVEL == 1
    /* PSA APIs must be called from Thread mode */
    if (__get_active_exc_num() != EXC_NUM_THREAD_MODE) {
        tfm_core_panic();
    }
#endif

    /*
     * Check if caller stack is within SPM stack. If not, then stack needs to
     * switch. Otherwise, return zeros.
     */
    if ((caller->ctx_ctrl.sp <= SPM_THREAD_CONTEXT->sp_limit) ||
        (caller->ctx_ctrl.sp >  SPM_THREAD_CONTEXT->sp_base)) {
        sp       = SPM_THREAD_CONTEXT->sp;
        sp_limit = SPM_THREAD_CONTEXT->sp_limit;
    }

    AAPCS_DUAL_U32_SET(spm_stack_info, sp, sp_limit);

    arch_acquire_sched_lock();

    return AAPCS_DUAL_U32_AS_U64(spm_stack_info);
}

uint32_t backend_abi_leaving_spm(uint32_t result)
{
    uint32_t sched_attempted;

    spm_handle_programmer_errors(result);

    /* Release scheduler lock and check the record of schedule attempt. */
    sched_attempted = arch_release_sched_lock();

    /* Interrupt is masked, PendSV will not happen immediately. */
    if (result == STATUS_NEED_SCHEDULE ||
        sched_attempted == SCHEDULER_ATTEMPTED) {
        arch_attempt_schedule();
    }

    return result;
}

uint64_t ipc_schedule(void)
{
    fih_int fih_rc = FIH_FAILURE;
    AAPCS_DUAL_U32_T ctx_ctrls;
    struct partition_t *p_part_curr, *p_part_next;
    struct context_ctrl_t *p_curr_ctx;
    struct thread_t *pth_next;
    struct critical_section_t cs = CRITICAL_SECTION_STATIC_INIT;

    /* Protect concurrent access to current thread/component and thread status */
    CRITICAL_SECTION_ENTER(cs);

    pth_next = thrd_next();
    p_curr_ctx = (struct context_ctrl_t *)(CURRENT_THREAD->p_context_ctrl);

    AAPCS_DUAL_U32_SET(ctx_ctrls, (uint32_t)p_curr_ctx, (uint32_t)p_curr_ctx);

    p_part_curr = GET_CURRENT_COMPONENT();
    p_part_next = GET_THRD_OWNER(pth_next);

    if (pth_next != NULL && p_part_curr != p_part_next) {
        /* Check if there is enough room on stack to save more context */
        if ((p_curr_ctx->sp_limit +
                sizeof(struct tfm_additional_context_t)) > __get_PSP()) {
            tfm_core_panic();
        }

        /*
         * If required, let the platform update boundary based on its
         * implementation. Change privilege, MPU or other configurations.
         */
        if (tfm_hal_boundary_need_switch(p_part_curr->boundary,
                                         p_part_next->boundary)) {
            FIH_CALL(tfm_hal_activate_boundary, fih_rc,
                     p_part_next->p_ldinf, p_part_next->boundary);
            if (fih_not_eq(fih_rc, fih_int_encode(TFM_HAL_SUCCESS))) {
                tfm_core_panic();
            }
        }
        ARCH_FLUSH_FP_CONTEXT();

        AAPCS_DUAL_U32_SET_A1(ctx_ctrls, (uint32_t)pth_next->p_context_ctrl);

        CURRENT_THREAD = pth_next;
    }

    /* Update meta indicator */
    if (partition_meta_indicator_pos && (p_part_next->p_metadata)) {
        *partition_meta_indicator_pos = (uintptr_t)(p_part_next->p_metadata);
    }
    CRITICAL_SECTION_LEAVE(cs);

    return AAPCS_DUAL_U32_AS_U64(ctx_ctrls);
}
