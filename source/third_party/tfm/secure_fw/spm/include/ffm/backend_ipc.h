/*
 * Copyright (c) 2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef __BACKEND_IPC_H__
#define __BACKEND_IPC_H__

#include <stdint.h>

/* Calculate the service setting. In IPC it is the signal set. */
#define BACKEND_SERVICE_SET(set, p_service) ((set) |= (p_service)->signal)

/*
 * Actions done before entering SPM.
 *
 * Executes with interrupt unmasked. Check the necessity of switching to SPM
 * stack and lock scheduler. Return value is the pair of SPM SP and PSPLIM if
 * necessary. Otherwise, zeros.
 */
uint64_t backend_abi_entering_spm(void);

/*
 * Actions done after leaving SPM and before entering other components.
 *
 * Executes with interrupt masked.
 * Check return value from backend and trigger scheduler in PendSV if necessary.
 */
uint32_t backend_abi_leaving_spm(uint32_t result);

#endif /* __BACKEND_IPC_H__ */
