/*
 * Copyright (c) 2022, Texas Instruments Incorporated - http://www.ti.com
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * These PSA ITS functions facilitate direct calls to the corresponding TF-M ITS
 * functions without requiring PSA calls (IPC model) or using a secure gateway
 * veneer (Library or SFN model).
 *
 */

#include <interface/include/psa/internal_trusted_storage.h>
#include <psa_manifest/pid.h>
#include <secure_fw/partitions/internal_trusted_storage/tfm_internal_trusted_storage.h>
#include <third_party/tfm/secure_fw/spm/include/utilities.h>

#include <stdint.h>

#if TFM_ENABLED
    /* Set the client ID to the PID of the Crypto SP */
    #define CLIENT_ID (int32_t)CRYPTO_SP
#else
    #ifndef DEFAULT_NS_CLIENT_ID
        #define DEFAULT_NS_CLIENT_ID (-1)
    #endif
    #define CLIENT_ID (int32_t)DEFAULT_NS_CLIENT_ID
#endif

static uint8_t *data;

psa_status_t psa_its_remove(psa_storage_uid_t uid)
{
    return tfm_its_remove(CLIENT_ID, uid);
}

psa_status_t psa_its_get_info(psa_storage_uid_t uid, struct psa_storage_info_t *p_info)
{
    return tfm_its_get_info(CLIENT_ID, uid, p_info);
}

psa_status_t psa_its_get(psa_storage_uid_t uid,
                         size_t data_offset,
                         size_t data_size,
                         void *p_data,
                         size_t *p_data_length)
{
    data = (uint8_t *)p_data;

    return tfm_its_get(CLIENT_ID, uid, data_offset, data_size, p_data_length);
}

psa_status_t psa_its_set(psa_storage_uid_t uid,
                         size_t data_length,
                         const void *p_data,
                         psa_storage_create_flags_t create_flags)
{
    data = (uint8_t *)p_data;

    return tfm_its_set(CLIENT_ID, uid, data_length, create_flags);
}

size_t its_req_mngr_read(uint8_t *buf, size_t num_bytes)
{
    (void)spm_memcpy(buf, data, num_bytes);
    data += num_bytes;
    return num_bytes;
}

void its_req_mngr_write(const uint8_t *buf, size_t num_bytes)
{
    (void)spm_memcpy(data, buf, num_bytes);
    data += num_bytes;
}
