/*
 * Copyright (c) 2023, Arm Limited. All rights reserved.
 * Copyright (c) 2023 Cypress Semiconductor Corporation (an Infineon company)
 * or an affiliate of Cypress Semiconductor Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "current.h"
#include "fih.h"
#include "internal_status_code.h"
#include "spm.h"
#include "tfm_hal_isolation.h"
#include "tfm_multi_core.h"
#include "ffm/agent_api.h"
#include "ffm/backend.h"
#include "ffm/psa_api.h"
#include "psa/error.h"

psa_status_t tfm_spm_agent_psa_call(psa_handle_t handle,
                                    uint32_t control,
                                    const struct client_params_t *params,
                                    const void *client_data_stateless)
{
    struct connection_t *p_connection;
    struct partition_t *curr_partition = GET_CURRENT_COMPONENT();
    fih_int fih_rc = FIH_FAILURE;
    psa_status_t status;

    (void)client_data_stateless;

    FIH_CALL(tfm_hal_memory_check, fih_rc,
             curr_partition->boundary, (uintptr_t)params,
             sizeof(struct client_params_t),
             TFM_HAL_ACCESS_READABLE);
    if (fih_not_eq(fih_rc, fih_int_encode(PSA_SUCCESS))) {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    status = spm_get_connection(&p_connection, handle, params->ns_client_id_stateless);
    if (status != PSA_SUCCESS) {
        return status;
    }

    status = spm_associate_call_params(p_connection, control, params->p_invecs, params->p_outvecs);
    if (status != PSA_SUCCESS) {
        return status;
    }

    return backend_messaging(p_connection);
}

#if CONFIG_TFM_CONNECTION_BASED_SERVICE_API == 1

psa_handle_t tfm_spm_agent_psa_connect(uint32_t sid, uint32_t version,
                                       int32_t ns_client_id,
                                       const void *client_data)
{
    (void)ns_client_id;
    (void)client_data;

    struct partition_t *curr_partition = GET_CURRENT_COMPONENT();

    if (!IS_NS_AGENT_MAILBOX(curr_partition->p_ldinf)) {
        tfm_core_panic();
    }

    return tfm_spm_client_psa_connect(sid, version);
}

#endif /* CONFIG_TFM_CONNECTION_BASED_SERVICE_API == 1 */
