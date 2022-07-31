/*
 * Copyright (c) 2019-2020, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

/*********** WARNING: This is an auto-generated file. Do not edit! ***********/

#ifndef __TFM_SPM_DB_FUNC_INC__
#define __TFM_SPM_DB_FUNC_INC__

#include "spm_api.h"
#include "psa_manifest/sid.h"

/**************************************************************************/
/** IRQ count per partition */
/**************************************************************************/
#ifdef TFM_PARTITION_PROTECTED_STORAGE
#define TFM_PARTITION_TFM_SP_PS_IRQ_COUNT 0
#endif /* TFM_PARTITION_PROTECTED_STORAGE */

#ifdef TFM_PARTITION_INTERNAL_TRUSTED_STORAGE
#define TFM_PARTITION_TFM_SP_ITS_IRQ_COUNT 0
#endif /* TFM_PARTITION_INTERNAL_TRUSTED_STORAGE */

#ifdef TFM_PARTITION_AUDIT_LOG
#define TFM_PARTITION_TFM_SP_AUDIT_LOG_IRQ_COUNT 0
#endif /* TFM_PARTITION_AUDIT_LOG */

#ifdef TFM_PARTITION_CRYPTO
#define TFM_PARTITION_TFM_SP_CRYPTO_IRQ_COUNT 0
#endif /* TFM_PARTITION_CRYPTO */

#ifdef TFM_PARTITION_PLATFORM
#define TFM_PARTITION_TFM_SP_PLATFORM_IRQ_COUNT 0
#endif /* TFM_PARTITION_PLATFORM */

#ifdef TFM_PARTITION_INITIAL_ATTESTATION
#define TFM_PARTITION_TFM_SP_INITIAL_ATTESTATION_IRQ_COUNT 0
#endif /* TFM_PARTITION_INITIAL_ATTESTATION */

#ifdef TFM_PARTITION_TEST_CORE
#define TFM_PARTITION_TFM_SP_CORE_TEST_IRQ_COUNT 0
#endif /* TFM_PARTITION_TEST_CORE */

#ifdef TFM_PARTITION_TEST_CORE
#define TFM_PARTITION_TFM_SP_CORE_TEST_2_IRQ_COUNT 0
#endif /* TFM_PARTITION_TEST_CORE */

#ifdef TFM_PARTITION_TEST_SECURE_SERVICES
#define TFM_PARTITION_TFM_SP_SECURE_TEST_PARTITION_IRQ_COUNT 0
#endif /* TFM_PARTITION_TEST_SECURE_SERVICES */

#ifdef TFM_PARTITION_TEST_CORE_IPC
#define TFM_PARTITION_TFM_SP_IPC_SERVICE_TEST_IRQ_COUNT 0
#endif /* TFM_PARTITION_TEST_CORE_IPC */

#ifdef TFM_PARTITION_TEST_CORE_IPC
#define TFM_PARTITION_TFM_SP_IPC_CLIENT_TEST_IRQ_COUNT 0
#endif /* TFM_PARTITION_TEST_CORE_IPC */

#ifdef TFM_ENABLE_IRQ_TEST
#define TFM_PARTITION_TFM_IRQ_TEST_1_IRQ_COUNT 1
#endif /* TFM_ENABLE_IRQ_TEST */

#ifdef TFM_PARTITION_TEST_PS
#define TFM_PARTITION_TFM_SP_PS_TEST_IRQ_COUNT 0
#endif /* TFM_PARTITION_TEST_PS */

#ifdef TFM_PARTITION_TEST_SECURE_SERVICES
#define TFM_PARTITION_TFM_SP_SECURE_CLIENT_2_IRQ_COUNT 0
#endif /* TFM_PARTITION_TEST_SECURE_SERVICES */

#ifdef TFM_MULTI_CORE_TEST
#define TFM_PARTITION_TFM_SP_MULTI_CORE_TEST_IRQ_COUNT 0
#endif /* TFM_MULTI_CORE_TEST */

/**************************************************************************/
/** Declarations of partition init functions */
/**************************************************************************/
#ifdef TFM_PARTITION_PROTECTED_STORAGE
extern void tfm_ps_req_mngr_init(void);
#endif /* TFM_PARTITION_PROTECTED_STORAGE */

#ifdef TFM_PARTITION_INTERNAL_TRUSTED_STORAGE
extern void tfm_its_req_mngr_init(void);
#endif /* TFM_PARTITION_INTERNAL_TRUSTED_STORAGE */

#ifdef TFM_PARTITION_AUDIT_LOG
extern void audit_core_init(void);
#endif /* TFM_PARTITION_AUDIT_LOG */

#ifdef TFM_PARTITION_CRYPTO
extern void tfm_crypto_init(void);
#endif /* TFM_PARTITION_CRYPTO */

#ifdef TFM_PARTITION_PLATFORM
extern void platform_sp_init(void);
#endif /* TFM_PARTITION_PLATFORM */

#ifdef TFM_PARTITION_INITIAL_ATTESTATION
extern void attest_partition_init(void);
#endif /* TFM_PARTITION_INITIAL_ATTESTATION */

#ifdef TFM_PARTITION_TEST_CORE
extern void core_test_init(void);
#endif /* TFM_PARTITION_TEST_CORE */

#ifdef TFM_PARTITION_TEST_CORE
extern void core_test_2_init(void);
#endif /* TFM_PARTITION_TEST_CORE */

#ifdef TFM_PARTITION_TEST_SECURE_SERVICES
extern void tfm_secure_client_service_init(void);
#endif /* TFM_PARTITION_TEST_SECURE_SERVICES */

#ifdef TFM_PARTITION_TEST_CORE_IPC
extern void ipc_service_test_main(void);
#endif /* TFM_PARTITION_TEST_CORE_IPC */

#ifdef TFM_PARTITION_TEST_CORE_IPC
extern void ipc_client_test_main(void);
#endif /* TFM_PARTITION_TEST_CORE_IPC */

#ifdef TFM_ENABLE_IRQ_TEST
extern void tfm_irq_test_1_init(void);
#endif /* TFM_ENABLE_IRQ_TEST */

#ifdef TFM_PARTITION_TEST_PS
extern void tfm_ps_test_init(void);
#endif /* TFM_PARTITION_TEST_PS */

#ifdef TFM_PARTITION_TEST_SECURE_SERVICES
extern void tfm_secure_client_2_init(void);
#endif /* TFM_PARTITION_TEST_SECURE_SERVICES */

#ifdef TFM_MULTI_CORE_TEST
extern void multi_core_test_main(void);
#endif /* TFM_MULTI_CORE_TEST */

/**************************************************************************/
/** Context stacks for IRQ handling */
/**************************************************************************/
/* The max size of the context stack can be calculated as a function of the IRQ
 * count of the secure partition:
 *
 * max_stack_size = intr_ctx_size + (IRQ_CNT * (intr_ctx_size + hndl_ctx_size))
 *
 * where:
 *   intr_ctx: Frame pushed when the partition is interrupted
 *   hndl_ctx: Frame pushed when the partition is handling an interrupt
 */
static uint32_t ns_interrupt_ctx_stack[
        sizeof(struct interrupted_ctx_stack_frame_t) / sizeof(uint32_t)];

static uint32_t tfm_core_interrupt_ctx_stack[
        sizeof(struct interrupted_ctx_stack_frame_t) / sizeof(uint32_t)];

#ifdef TFM_PARTITION_PROTECTED_STORAGE
static uint32_t ctx_stack_TFM_SP_PS[
        (sizeof(struct interrupted_ctx_stack_frame_t) +
            (TFM_PARTITION_TFM_SP_PS_IRQ_COUNT) * (
                sizeof(struct interrupted_ctx_stack_frame_t) +
                sizeof(struct handler_ctx_stack_frame_t)
        )) / sizeof(uint32_t)];
#endif /* TFM_PARTITION_PROTECTED_STORAGE */

#ifdef TFM_PARTITION_INTERNAL_TRUSTED_STORAGE
static uint32_t ctx_stack_TFM_SP_ITS[
        (sizeof(struct interrupted_ctx_stack_frame_t) +
            (TFM_PARTITION_TFM_SP_ITS_IRQ_COUNT) * (
                sizeof(struct interrupted_ctx_stack_frame_t) +
                sizeof(struct handler_ctx_stack_frame_t)
        )) / sizeof(uint32_t)];
#endif /* TFM_PARTITION_INTERNAL_TRUSTED_STORAGE */

#ifdef TFM_PARTITION_AUDIT_LOG
static uint32_t ctx_stack_TFM_SP_AUDIT_LOG[
        (sizeof(struct interrupted_ctx_stack_frame_t) +
            (TFM_PARTITION_TFM_SP_AUDIT_LOG_IRQ_COUNT) * (
                sizeof(struct interrupted_ctx_stack_frame_t) +
                sizeof(struct handler_ctx_stack_frame_t)
        )) / sizeof(uint32_t)];
#endif /* TFM_PARTITION_AUDIT_LOG */

#ifdef TFM_PARTITION_CRYPTO
static uint32_t ctx_stack_TFM_SP_CRYPTO[
        (sizeof(struct interrupted_ctx_stack_frame_t) +
            (TFM_PARTITION_TFM_SP_CRYPTO_IRQ_COUNT) * (
                sizeof(struct interrupted_ctx_stack_frame_t) +
                sizeof(struct handler_ctx_stack_frame_t)
        )) / sizeof(uint32_t)];
#endif /* TFM_PARTITION_CRYPTO */

#ifdef TFM_PARTITION_PLATFORM
static uint32_t ctx_stack_TFM_SP_PLATFORM[
        (sizeof(struct interrupted_ctx_stack_frame_t) +
            (TFM_PARTITION_TFM_SP_PLATFORM_IRQ_COUNT) * (
                sizeof(struct interrupted_ctx_stack_frame_t) +
                sizeof(struct handler_ctx_stack_frame_t)
        )) / sizeof(uint32_t)];
#endif /* TFM_PARTITION_PLATFORM */

#ifdef TFM_PARTITION_INITIAL_ATTESTATION
static uint32_t ctx_stack_TFM_SP_INITIAL_ATTESTATION[
        (sizeof(struct interrupted_ctx_stack_frame_t) +
            (TFM_PARTITION_TFM_SP_INITIAL_ATTESTATION_IRQ_COUNT) * (
                sizeof(struct interrupted_ctx_stack_frame_t) +
                sizeof(struct handler_ctx_stack_frame_t)
        )) / sizeof(uint32_t)];
#endif /* TFM_PARTITION_INITIAL_ATTESTATION */

#ifdef TFM_PARTITION_TEST_CORE
static uint32_t ctx_stack_TFM_SP_CORE_TEST[
        (sizeof(struct interrupted_ctx_stack_frame_t) +
            (TFM_PARTITION_TFM_SP_CORE_TEST_IRQ_COUNT) * (
                sizeof(struct interrupted_ctx_stack_frame_t) +
                sizeof(struct handler_ctx_stack_frame_t)
        )) / sizeof(uint32_t)];
#endif /* TFM_PARTITION_TEST_CORE */

#ifdef TFM_PARTITION_TEST_CORE
static uint32_t ctx_stack_TFM_SP_CORE_TEST_2[
        (sizeof(struct interrupted_ctx_stack_frame_t) +
            (TFM_PARTITION_TFM_SP_CORE_TEST_2_IRQ_COUNT) * (
                sizeof(struct interrupted_ctx_stack_frame_t) +
                sizeof(struct handler_ctx_stack_frame_t)
        )) / sizeof(uint32_t)];
#endif /* TFM_PARTITION_TEST_CORE */

#ifdef TFM_PARTITION_TEST_SECURE_SERVICES
static uint32_t ctx_stack_TFM_SP_SECURE_TEST_PARTITION[
        (sizeof(struct interrupted_ctx_stack_frame_t) +
            (TFM_PARTITION_TFM_SP_SECURE_TEST_PARTITION_IRQ_COUNT) * (
                sizeof(struct interrupted_ctx_stack_frame_t) +
                sizeof(struct handler_ctx_stack_frame_t)
        )) / sizeof(uint32_t)];
#endif /* TFM_PARTITION_TEST_SECURE_SERVICES */

#ifdef TFM_PARTITION_TEST_CORE_IPC
static uint32_t ctx_stack_TFM_SP_IPC_SERVICE_TEST[
        (sizeof(struct interrupted_ctx_stack_frame_t) +
            (TFM_PARTITION_TFM_SP_IPC_SERVICE_TEST_IRQ_COUNT) * (
                sizeof(struct interrupted_ctx_stack_frame_t) +
                sizeof(struct handler_ctx_stack_frame_t)
        )) / sizeof(uint32_t)];
#endif /* TFM_PARTITION_TEST_CORE_IPC */

#ifdef TFM_PARTITION_TEST_CORE_IPC
static uint32_t ctx_stack_TFM_SP_IPC_CLIENT_TEST[
        (sizeof(struct interrupted_ctx_stack_frame_t) +
            (TFM_PARTITION_TFM_SP_IPC_CLIENT_TEST_IRQ_COUNT) * (
                sizeof(struct interrupted_ctx_stack_frame_t) +
                sizeof(struct handler_ctx_stack_frame_t)
        )) / sizeof(uint32_t)];
#endif /* TFM_PARTITION_TEST_CORE_IPC */

#ifdef TFM_ENABLE_IRQ_TEST
static uint32_t ctx_stack_TFM_IRQ_TEST_1[
        (sizeof(struct interrupted_ctx_stack_frame_t) +
            (TFM_PARTITION_TFM_IRQ_TEST_1_IRQ_COUNT) * (
                sizeof(struct interrupted_ctx_stack_frame_t) +
                sizeof(struct handler_ctx_stack_frame_t)
        )) / sizeof(uint32_t)];
#endif /* TFM_ENABLE_IRQ_TEST */

#ifdef TFM_PARTITION_TEST_PS
static uint32_t ctx_stack_TFM_SP_PS_TEST[
        (sizeof(struct interrupted_ctx_stack_frame_t) +
            (TFM_PARTITION_TFM_SP_PS_TEST_IRQ_COUNT) * (
                sizeof(struct interrupted_ctx_stack_frame_t) +
                sizeof(struct handler_ctx_stack_frame_t)
        )) / sizeof(uint32_t)];
#endif /* TFM_PARTITION_TEST_PS */

#ifdef TFM_PARTITION_TEST_SECURE_SERVICES
static uint32_t ctx_stack_TFM_SP_SECURE_CLIENT_2[
        (sizeof(struct interrupted_ctx_stack_frame_t) +
            (TFM_PARTITION_TFM_SP_SECURE_CLIENT_2_IRQ_COUNT) * (
                sizeof(struct interrupted_ctx_stack_frame_t) +
                sizeof(struct handler_ctx_stack_frame_t)
        )) / sizeof(uint32_t)];
#endif /* TFM_PARTITION_TEST_SECURE_SERVICES */

#ifdef TFM_MULTI_CORE_TEST
static uint32_t ctx_stack_TFM_SP_MULTI_CORE_TEST[
        (sizeof(struct interrupted_ctx_stack_frame_t) +
            (TFM_PARTITION_TFM_SP_MULTI_CORE_TEST_IRQ_COUNT) * (
                sizeof(struct interrupted_ctx_stack_frame_t) +
                sizeof(struct handler_ctx_stack_frame_t)
        )) / sizeof(uint32_t)];
#endif /* TFM_MULTI_CORE_TEST */

uint32_t *ctx_stack_list[] =
{
    ns_interrupt_ctx_stack,
    tfm_core_interrupt_ctx_stack,
#ifdef TFM_PARTITION_PROTECTED_STORAGE
    ctx_stack_TFM_SP_PS,
#endif /* TFM_PARTITION_PROTECTED_STORAGE */
#ifdef TFM_PARTITION_INTERNAL_TRUSTED_STORAGE
    ctx_stack_TFM_SP_ITS,
#endif /* TFM_PARTITION_INTERNAL_TRUSTED_STORAGE */
#ifdef TFM_PARTITION_AUDIT_LOG
    ctx_stack_TFM_SP_AUDIT_LOG,
#endif /* TFM_PARTITION_AUDIT_LOG */
#ifdef TFM_PARTITION_CRYPTO
    ctx_stack_TFM_SP_CRYPTO,
#endif /* TFM_PARTITION_CRYPTO */
#ifdef TFM_PARTITION_PLATFORM
    ctx_stack_TFM_SP_PLATFORM,
#endif /* TFM_PARTITION_PLATFORM */
#ifdef TFM_PARTITION_INITIAL_ATTESTATION
    ctx_stack_TFM_SP_INITIAL_ATTESTATION,
#endif /* TFM_PARTITION_INITIAL_ATTESTATION */
#ifdef TFM_PARTITION_TEST_CORE
    ctx_stack_TFM_SP_CORE_TEST,
#endif /* TFM_PARTITION_TEST_CORE */
#ifdef TFM_PARTITION_TEST_CORE
    ctx_stack_TFM_SP_CORE_TEST_2,
#endif /* TFM_PARTITION_TEST_CORE */
#ifdef TFM_PARTITION_TEST_SECURE_SERVICES
    ctx_stack_TFM_SP_SECURE_TEST_PARTITION,
#endif /* TFM_PARTITION_TEST_SECURE_SERVICES */
#ifdef TFM_PARTITION_TEST_CORE_IPC
    ctx_stack_TFM_SP_IPC_SERVICE_TEST,
#endif /* TFM_PARTITION_TEST_CORE_IPC */
#ifdef TFM_PARTITION_TEST_CORE_IPC
    ctx_stack_TFM_SP_IPC_CLIENT_TEST,
#endif /* TFM_PARTITION_TEST_CORE_IPC */
#ifdef TFM_ENABLE_IRQ_TEST
    ctx_stack_TFM_IRQ_TEST_1,
#endif /* TFM_ENABLE_IRQ_TEST */
#ifdef TFM_PARTITION_TEST_PS
    ctx_stack_TFM_SP_PS_TEST,
#endif /* TFM_PARTITION_TEST_PS */
#ifdef TFM_PARTITION_TEST_SECURE_SERVICES
    ctx_stack_TFM_SP_SECURE_CLIENT_2,
#endif /* TFM_PARTITION_TEST_SECURE_SERVICES */
#ifdef TFM_MULTI_CORE_TEST
    ctx_stack_TFM_SP_MULTI_CORE_TEST,
#endif /* TFM_MULTI_CORE_TEST */
};

/**************************************************************************/
/** Dependencies array for Secure Partition */
/**************************************************************************/
#ifdef TFM_PARTITION_PROTECTED_STORAGE
static int32_t dependencies_TFM_SP_PS[] =
{
    TFM_CRYPTO_SID,
    TFM_ITS_SET_SID,
    TFM_ITS_GET_SID,
    TFM_ITS_GET_INFO_SID,
    TFM_ITS_REMOVE_SID,
    TFM_SP_PLATFORM_NV_COUNTER_SID,
};
#endif /* TFM_PARTITION_PROTECTED_STORAGE */

#ifdef TFM_PARTITION_CRYPTO
static int32_t dependencies_TFM_SP_CRYPTO[] =
{
    TFM_ITS_SET_SID,
    TFM_ITS_GET_SID,
    TFM_ITS_GET_INFO_SID,
    TFM_ITS_REMOVE_SID,
};
#endif /* TFM_PARTITION_CRYPTO */

#ifdef TFM_PARTITION_INITIAL_ATTESTATION
static int32_t dependencies_TFM_SP_INITIAL_ATTESTATION[] =
{
    TFM_CRYPTO_SID,
};
#endif /* TFM_PARTITION_INITIAL_ATTESTATION */

#ifdef TFM_PARTITION_TEST_CORE
static int32_t dependencies_TFM_SP_CORE_TEST[] =
{
    SPM_CORE_TEST_2_INVERT_SID,
    SPM_CORE_TEST_2_GET_EVERY_SECOND_BYTE_SID,
    SPM_CORE_TEST_2_SLAVE_SERVICE_SID,
};
#endif /* TFM_PARTITION_TEST_CORE */

#ifdef TFM_PARTITION_TEST_SECURE_SERVICES
static int32_t dependencies_TFM_SP_SECURE_TEST_PARTITION[] =
{
    TFM_SECURE_CLIENT_2_SID,
    TFM_CRYPTO_SID,
    TFM_PS_SET_SID,
    TFM_PS_GET_SID,
    TFM_PS_GET_INFO_SID,
    TFM_PS_REMOVE_SID,
    TFM_PS_GET_SUPPORT_SID,
    TFM_ITS_SET_SID,
    TFM_ITS_GET_SID,
    TFM_ITS_GET_INFO_SID,
    TFM_ITS_REMOVE_SID,
    TFM_ATTEST_GET_TOKEN_SID,
    TFM_ATTEST_GET_TOKEN_SIZE_SID,
    TFM_ATTEST_GET_PUBLIC_KEY_SID,
    TFM_PS_TEST_PREPARE_SID,
    TFM_SP_PLATFORM_SYSTEM_RESET_SID,
    TFM_SP_PLATFORM_IOCTL_SID,
};
#endif /* TFM_PARTITION_TEST_SECURE_SERVICES */

#ifdef TFM_PARTITION_TEST_CORE_IPC
static int32_t dependencies_TFM_SP_IPC_CLIENT_TEST[] =
{
    IPC_SERVICE_TEST_PSA_ACCESS_APP_READ_ONLY_MEM_SID,
    IPC_SERVICE_TEST_PSA_ACCESS_APP_MEM_SID,
    IPC_SERVICE_TEST_BASIC_SID,
    IPC_SERVICE_TEST_APP_ACCESS_PSA_MEM_SID,
};
#endif /* TFM_PARTITION_TEST_CORE_IPC */

#ifdef TFM_PARTITION_TEST_PS
static int32_t dependencies_TFM_SP_PS_TEST[] =
{
    TFM_CRYPTO_SID,
    TFM_ITS_GET_SID,
    TFM_ITS_REMOVE_SID,
};
#endif /* TFM_PARTITION_TEST_PS */

#ifdef TFM_PARTITION_TEST_SECURE_SERVICES
static int32_t dependencies_TFM_SP_SECURE_CLIENT_2[] =
{
    TFM_ITS_GET_SID,
    TFM_CRYPTO_SID,
};
#endif /* TFM_PARTITION_TEST_SECURE_SERVICES */

/**************************************************************************/
/** The static data of the partition list */
/**************************************************************************/
const struct spm_partition_static_data_t static_data_list[] =
{
    {
        .partition_id         = TFM_SP_NON_SECURE_ID,
        .partition_flags      = 0,
    },

    {
        .partition_id         = TFM_SP_CORE_ID,
        .partition_flags      = SPM_PART_FLAG_APP_ROT | SPM_PART_FLAG_PSA_ROT,
    },

#ifdef TFM_PARTITION_PROTECTED_STORAGE
    {
        .partition_id         = TFM_SP_PS,
        .partition_flags      = SPM_PART_FLAG_IPC
                              | SPM_PART_FLAG_APP_ROT
                              ,
        .partition_priority   = TFM_PRIORITY(NORMAL),
        .partition_init       = tfm_ps_req_mngr_init,
        .dependencies_num     = 6,
        .p_dependencies       = dependencies_TFM_SP_PS,
    },
#endif /* TFM_PARTITION_PROTECTED_STORAGE */

#ifdef TFM_PARTITION_INTERNAL_TRUSTED_STORAGE
    {
        .partition_id         = TFM_SP_ITS,
        .partition_flags      = SPM_PART_FLAG_IPC
                              | SPM_PART_FLAG_PSA_ROT | SPM_PART_FLAG_APP_ROT
                              ,
        .partition_priority   = TFM_PRIORITY(NORMAL),
        .partition_init       = tfm_its_req_mngr_init,
        .dependencies_num     = 0,
        .p_dependencies       = NULL,
    },
#endif /* TFM_PARTITION_INTERNAL_TRUSTED_STORAGE */

#ifdef TFM_PARTITION_AUDIT_LOG
    {
        .partition_id         = TFM_SP_AUDIT_LOG,
        .partition_flags      = 0
                              | SPM_PART_FLAG_PSA_ROT | SPM_PART_FLAG_APP_ROT
                              ,
        .partition_priority   = TFM_PRIORITY(NORMAL),
        .partition_init       = audit_core_init,
        .dependencies_num     = 0,
        .p_dependencies       = NULL,
    },
#endif /* TFM_PARTITION_AUDIT_LOG */

#ifdef TFM_PARTITION_CRYPTO
    {
        .partition_id         = TFM_SP_CRYPTO,
        .partition_flags      = SPM_PART_FLAG_IPC
                              | SPM_PART_FLAG_PSA_ROT | SPM_PART_FLAG_APP_ROT
                              ,
        .partition_priority   = TFM_PRIORITY(NORMAL),
        .partition_init       = tfm_crypto_init,
        .dependencies_num     = 4,
        .p_dependencies       = dependencies_TFM_SP_CRYPTO,
    },
#endif /* TFM_PARTITION_CRYPTO */

#ifdef TFM_PARTITION_PLATFORM
    {
        .partition_id         = TFM_SP_PLATFORM,
        .partition_flags      = SPM_PART_FLAG_IPC
                              | SPM_PART_FLAG_PSA_ROT | SPM_PART_FLAG_APP_ROT
                              ,
        .partition_priority   = TFM_PRIORITY(NORMAL),
        .partition_init       = platform_sp_init,
        .dependencies_num     = 0,
        .p_dependencies       = NULL,
    },
#endif /* TFM_PARTITION_PLATFORM */

#ifdef TFM_PARTITION_INITIAL_ATTESTATION
    {
        .partition_id         = TFM_SP_INITIAL_ATTESTATION,
        .partition_flags      = SPM_PART_FLAG_IPC
                              | SPM_PART_FLAG_PSA_ROT | SPM_PART_FLAG_APP_ROT
                              ,
        .partition_priority   = TFM_PRIORITY(NORMAL),
        .partition_init       = attest_partition_init,
        .dependencies_num     = 1,
        .p_dependencies       = dependencies_TFM_SP_INITIAL_ATTESTATION,
    },
#endif /* TFM_PARTITION_INITIAL_ATTESTATION */

#ifdef TFM_PARTITION_TEST_CORE
    {
        .partition_id         = TFM_SP_CORE_TEST,
        .partition_flags      = SPM_PART_FLAG_IPC
                              | SPM_PART_FLAG_PSA_ROT | SPM_PART_FLAG_APP_ROT
                              ,
        .partition_priority   = TFM_PRIORITY(NORMAL),
        .partition_init       = core_test_init,
        .dependencies_num     = 3,
        .p_dependencies       = dependencies_TFM_SP_CORE_TEST,
    },
#endif /* TFM_PARTITION_TEST_CORE */

#ifdef TFM_PARTITION_TEST_CORE
    {
        .partition_id         = TFM_SP_CORE_TEST_2,
        .partition_flags      = SPM_PART_FLAG_IPC
                              | SPM_PART_FLAG_APP_ROT
                              ,
        .partition_priority   = TFM_PRIORITY(NORMAL),
        .partition_init       = core_test_2_init,
        .dependencies_num     = 0,
        .p_dependencies       = NULL,
    },
#endif /* TFM_PARTITION_TEST_CORE */

#ifdef TFM_PARTITION_TEST_SECURE_SERVICES
    {
        .partition_id         = TFM_SP_SECURE_TEST_PARTITION,
        .partition_flags      = SPM_PART_FLAG_IPC
                              | SPM_PART_FLAG_PSA_ROT | SPM_PART_FLAG_APP_ROT
                              ,
        .partition_priority   = TFM_PRIORITY(NORMAL),
        .partition_init       = tfm_secure_client_service_init,
        .dependencies_num     = 17,
        .p_dependencies       = dependencies_TFM_SP_SECURE_TEST_PARTITION,
    },
#endif /* TFM_PARTITION_TEST_SECURE_SERVICES */

#ifdef TFM_PARTITION_TEST_CORE_IPC
    {
        .partition_id         = TFM_SP_IPC_SERVICE_TEST,
        .partition_flags      = SPM_PART_FLAG_IPC
                              | SPM_PART_FLAG_PSA_ROT | SPM_PART_FLAG_APP_ROT
                              ,
        .partition_priority   = TFM_PRIORITY(HIGH),
        .partition_init       = ipc_service_test_main,
        .dependencies_num     = 0,
        .p_dependencies       = NULL,
    },
#endif /* TFM_PARTITION_TEST_CORE_IPC */

#ifdef TFM_PARTITION_TEST_CORE_IPC
    {
        .partition_id         = TFM_SP_IPC_CLIENT_TEST,
        .partition_flags      = SPM_PART_FLAG_IPC
                              | SPM_PART_FLAG_APP_ROT
                              ,
        .partition_priority   = TFM_PRIORITY(NORMAL),
        .partition_init       = ipc_client_test_main,
        .dependencies_num     = 4,
        .p_dependencies       = dependencies_TFM_SP_IPC_CLIENT_TEST,
    },
#endif /* TFM_PARTITION_TEST_CORE_IPC */

#ifdef TFM_ENABLE_IRQ_TEST
    {
        .partition_id         = TFM_IRQ_TEST_1,
        .partition_flags      = SPM_PART_FLAG_IPC
                              | SPM_PART_FLAG_APP_ROT
                              ,
        .partition_priority   = TFM_PRIORITY(NORMAL),
        .partition_init       = tfm_irq_test_1_init,
        .dependencies_num     = 0,
        .p_dependencies       = NULL,
    },
#endif /* TFM_ENABLE_IRQ_TEST */

#ifdef TFM_PARTITION_TEST_PS
    {
        .partition_id         = TFM_SP_PS_TEST,
        .partition_flags      = SPM_PART_FLAG_IPC
                              | SPM_PART_FLAG_PSA_ROT | SPM_PART_FLAG_APP_ROT
                              ,
        .partition_priority   = TFM_PRIORITY(NORMAL),
        .partition_init       = tfm_ps_test_init,
        .dependencies_num     = 3,
        .p_dependencies       = dependencies_TFM_SP_PS_TEST,
    },
#endif /* TFM_PARTITION_TEST_PS */

#ifdef TFM_PARTITION_TEST_SECURE_SERVICES
    {
        .partition_id         = TFM_SP_SECURE_CLIENT_2,
        .partition_flags      = SPM_PART_FLAG_IPC
                              | SPM_PART_FLAG_APP_ROT
                              ,
        .partition_priority   = TFM_PRIORITY(NORMAL),
        .partition_init       = tfm_secure_client_2_init,
        .dependencies_num     = 2,
        .p_dependencies       = dependencies_TFM_SP_SECURE_CLIENT_2,
    },
#endif /* TFM_PARTITION_TEST_SECURE_SERVICES */

#ifdef TFM_MULTI_CORE_TEST
    {
        .partition_id         = TFM_SP_MULTI_CORE_TEST,
        .partition_flags      = SPM_PART_FLAG_IPC
                              | SPM_PART_FLAG_APP_ROT
                              ,
        .partition_priority   = TFM_PRIORITY(NORMAL),
        .partition_init       = multi_core_test_main,
        .dependencies_num     = 0,
        .p_dependencies       = NULL,
    },
#endif /* TFM_MULTI_CORE_TEST */

};

/**************************************************************************/
/** The platform data of the partition list */
/**************************************************************************/
#ifdef TFM_PARTITION_AUDIT_LOG
const struct tfm_spm_partition_platform_data_t *
    platform_data_list_TFM_SP_AUDIT_LOG[] =
{
#ifdef AUDIT_UART_REDIRECTION
    TFM_PERIPHERAL_UART1,
#endif /* AUDIT_UART_REDIRECTION */
    NULL
};
#endif /* TFM_PARTITION_AUDIT_LOG */

#ifdef TFM_PARTITION_TEST_CORE
const struct tfm_spm_partition_platform_data_t *
    platform_data_list_TFM_SP_CORE_TEST[] =
{
    TFM_PERIPHERAL_FPGA_IO,
    NULL
};
#endif /* TFM_PARTITION_TEST_CORE */

#ifdef TFM_PARTITION_TEST_SECURE_SERVICES
const struct tfm_spm_partition_platform_data_t *
    platform_data_list_TFM_SP_SECURE_TEST_PARTITION[] =
{
    TFM_PERIPHERAL_STD_UART,
    NULL
};
#endif /* TFM_PARTITION_TEST_SECURE_SERVICES */

#ifdef TFM_ENABLE_IRQ_TEST
const struct tfm_spm_partition_platform_data_t *
    platform_data_list_TFM_IRQ_TEST_1[] =
{
    TFM_PERIPHERAL_TIMER0,
    NULL
};
#endif /* TFM_ENABLE_IRQ_TEST */

const struct tfm_spm_partition_platform_data_t **platform_data_list_list[] =
{
    NULL,
    NULL,

#ifdef TFM_PARTITION_PROTECTED_STORAGE
    NULL,
#endif /* TFM_PARTITION_PROTECTED_STORAGE */

#ifdef TFM_PARTITION_INTERNAL_TRUSTED_STORAGE
    NULL,
#endif /* TFM_PARTITION_INTERNAL_TRUSTED_STORAGE */

#ifdef TFM_PARTITION_AUDIT_LOG
    platform_data_list_TFM_SP_AUDIT_LOG,
#endif /* TFM_PARTITION_AUDIT_LOG */

#ifdef TFM_PARTITION_CRYPTO
    NULL,
#endif /* TFM_PARTITION_CRYPTO */

#ifdef TFM_PARTITION_PLATFORM
    NULL,
#endif /* TFM_PARTITION_PLATFORM */

#ifdef TFM_PARTITION_INITIAL_ATTESTATION
    NULL,
#endif /* TFM_PARTITION_INITIAL_ATTESTATION */

#ifdef TFM_PARTITION_TEST_CORE
    platform_data_list_TFM_SP_CORE_TEST,
#endif /* TFM_PARTITION_TEST_CORE */

#ifdef TFM_PARTITION_TEST_CORE
    NULL,
#endif /* TFM_PARTITION_TEST_CORE */

#ifdef TFM_PARTITION_TEST_SECURE_SERVICES
    platform_data_list_TFM_SP_SECURE_TEST_PARTITION,
#endif /* TFM_PARTITION_TEST_SECURE_SERVICES */

#ifdef TFM_PARTITION_TEST_CORE_IPC
    NULL,
#endif /* TFM_PARTITION_TEST_CORE_IPC */

#ifdef TFM_PARTITION_TEST_CORE_IPC
    NULL,
#endif /* TFM_PARTITION_TEST_CORE_IPC */

#ifdef TFM_ENABLE_IRQ_TEST
    platform_data_list_TFM_IRQ_TEST_1,
#endif /* TFM_ENABLE_IRQ_TEST */

#ifdef TFM_PARTITION_TEST_PS
    NULL,
#endif /* TFM_PARTITION_TEST_PS */

#ifdef TFM_PARTITION_TEST_SECURE_SERVICES
    NULL,
#endif /* TFM_PARTITION_TEST_SECURE_SERVICES */

#ifdef TFM_MULTI_CORE_TEST
    NULL,
#endif /* TFM_MULTI_CORE_TEST */

};

/**************************************************************************/
/** The partition list for the DB */
/**************************************************************************/
static struct spm_partition_desc_t partition_list [] =
{
    {{0}}, /* placeholder for Non-secure internal partition */
    {{0}},   /* placeholder for TF-M Core internal partition */

    /* -----------------------------------------------------------------------*/
    /* - Partition DB record for TFM_SP_PS */
    /* -----------------------------------------------------------------------*/
#ifdef TFM_PARTITION_PROTECTED_STORAGE
    {
    /* Runtime data */
        .runtime_data             = {0},
        .static_data              = NULL,
        .platform_data_list       = NULL,
    },
#endif /* TFM_PARTITION_PROTECTED_STORAGE */

    /* -----------------------------------------------------------------------*/
    /* - Partition DB record for TFM_SP_ITS */
    /* -----------------------------------------------------------------------*/
#ifdef TFM_PARTITION_INTERNAL_TRUSTED_STORAGE
    {
    /* Runtime data */
        .runtime_data             = {0},
        .static_data              = NULL,
        .platform_data_list       = NULL,
    },
#endif /* TFM_PARTITION_INTERNAL_TRUSTED_STORAGE */

    /* -----------------------------------------------------------------------*/
    /* - Partition DB record for TFM_SP_AUDIT_LOG */
    /* -----------------------------------------------------------------------*/
#ifdef TFM_PARTITION_AUDIT_LOG
    {
    /* Runtime data */
        .runtime_data             = {0},
        .static_data              = NULL,
        .platform_data_list       = NULL,
    },
#endif /* TFM_PARTITION_AUDIT_LOG */

    /* -----------------------------------------------------------------------*/
    /* - Partition DB record for TFM_SP_CRYPTO */
    /* -----------------------------------------------------------------------*/
#ifdef TFM_PARTITION_CRYPTO
    {
    /* Runtime data */
        .runtime_data             = {0},
        .static_data              = NULL,
        .platform_data_list       = NULL,
    },
#endif /* TFM_PARTITION_CRYPTO */

    /* -----------------------------------------------------------------------*/
    /* - Partition DB record for TFM_SP_PLATFORM */
    /* -----------------------------------------------------------------------*/
#ifdef TFM_PARTITION_PLATFORM
    {
    /* Runtime data */
        .runtime_data             = {0},
        .static_data              = NULL,
        .platform_data_list       = NULL,
    },
#endif /* TFM_PARTITION_PLATFORM */

    /* -----------------------------------------------------------------------*/
    /* - Partition DB record for TFM_SP_INITIAL_ATTESTATION */
    /* -----------------------------------------------------------------------*/
#ifdef TFM_PARTITION_INITIAL_ATTESTATION
    {
    /* Runtime data */
        .runtime_data             = {0},
        .static_data              = NULL,
        .platform_data_list       = NULL,
    },
#endif /* TFM_PARTITION_INITIAL_ATTESTATION */

    /* -----------------------------------------------------------------------*/
    /* - Partition DB record for TFM_SP_CORE_TEST */
    /* -----------------------------------------------------------------------*/
#ifdef TFM_PARTITION_TEST_CORE
    {
    /* Runtime data */
        .runtime_data             = {0},
        .static_data              = NULL,
        .platform_data_list       = NULL,
    },
#endif /* TFM_PARTITION_TEST_CORE */

    /* -----------------------------------------------------------------------*/
    /* - Partition DB record for TFM_SP_CORE_TEST_2 */
    /* -----------------------------------------------------------------------*/
#ifdef TFM_PARTITION_TEST_CORE
    {
    /* Runtime data */
        .runtime_data             = {0},
        .static_data              = NULL,
        .platform_data_list       = NULL,
    },
#endif /* TFM_PARTITION_TEST_CORE */

    /* -----------------------------------------------------------------------*/
    /* - Partition DB record for TFM_SP_SECURE_TEST_PARTITION */
    /* -----------------------------------------------------------------------*/
#ifdef TFM_PARTITION_TEST_SECURE_SERVICES
    {
    /* Runtime data */
        .runtime_data             = {0},
        .static_data              = NULL,
        .platform_data_list       = NULL,
    },
#endif /* TFM_PARTITION_TEST_SECURE_SERVICES */

    /* -----------------------------------------------------------------------*/
    /* - Partition DB record for TFM_SP_IPC_SERVICE_TEST */
    /* -----------------------------------------------------------------------*/
#ifdef TFM_PARTITION_TEST_CORE_IPC
    {
    /* Runtime data */
        .runtime_data             = {0},
        .static_data              = NULL,
        .platform_data_list       = NULL,
    },
#endif /* TFM_PARTITION_TEST_CORE_IPC */

    /* -----------------------------------------------------------------------*/
    /* - Partition DB record for TFM_SP_IPC_CLIENT_TEST */
    /* -----------------------------------------------------------------------*/
#ifdef TFM_PARTITION_TEST_CORE_IPC
    {
    /* Runtime data */
        .runtime_data             = {0},
        .static_data              = NULL,
        .platform_data_list       = NULL,
    },
#endif /* TFM_PARTITION_TEST_CORE_IPC */

    /* -----------------------------------------------------------------------*/
    /* - Partition DB record for TFM_IRQ_TEST_1 */
    /* -----------------------------------------------------------------------*/
#ifdef TFM_ENABLE_IRQ_TEST
    {
    /* Runtime data */
        .runtime_data             = {0},
        .static_data              = NULL,
        .platform_data_list       = NULL,
    },
#endif /* TFM_ENABLE_IRQ_TEST */

    /* -----------------------------------------------------------------------*/
    /* - Partition DB record for TFM_SP_PS_TEST */
    /* -----------------------------------------------------------------------*/
#ifdef TFM_PARTITION_TEST_PS
    {
    /* Runtime data */
        .runtime_data             = {0},
        .static_data              = NULL,
        .platform_data_list       = NULL,
    },
#endif /* TFM_PARTITION_TEST_PS */

    /* -----------------------------------------------------------------------*/
    /* - Partition DB record for TFM_SP_SECURE_CLIENT_2 */
    /* -----------------------------------------------------------------------*/
#ifdef TFM_PARTITION_TEST_SECURE_SERVICES
    {
    /* Runtime data */
        .runtime_data             = {0},
        .static_data              = NULL,
        .platform_data_list       = NULL,
    },
#endif /* TFM_PARTITION_TEST_SECURE_SERVICES */

    /* -----------------------------------------------------------------------*/
    /* - Partition DB record for TFM_SP_MULTI_CORE_TEST */
    /* -----------------------------------------------------------------------*/
#ifdef TFM_MULTI_CORE_TEST
    {
    /* Runtime data */
        .runtime_data             = {0},
        .static_data              = NULL,
        .platform_data_list       = NULL,
    },
#endif /* TFM_MULTI_CORE_TEST */

};

struct spm_partition_db_t g_spm_partition_db = {
    .is_init = 0,
    .partition_count = sizeof(partition_list) / sizeof(partition_list[0]),
    .running_partition_idx = 0,
    .partitions = partition_list,
};

#endif /* __TFM_SPM_DB_FUNC_INC__ */
