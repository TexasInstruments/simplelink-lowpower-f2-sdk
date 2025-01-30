/*
 * Copyright (c) 2023 Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef __DPE_PLAT_H__
#define __DPE_PLAT_H__

#include <stddef.h>
#include <stdint.h>

#include "psa/crypto.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Get the RoT CDI value generated at boot time.
 *
 * \param[out] buf      Pointer to buffer to be written
 * \param[in]  buf_len  Length of data requested
 *
 * \return Returns 0 on success or a negative integer on failure.
 */
int dpe_plat_get_rot_cdi(uint8_t *buf, size_t buf_len);

/**
 * \brief Get the key ID of the root attestation key.
 *
 * \return Returns key ID.
 */
psa_key_id_t dpe_plat_get_root_attest_key_id(void);

#ifdef __cplusplus
}
#endif

#endif /* __DPE_PLAT_H__ */
