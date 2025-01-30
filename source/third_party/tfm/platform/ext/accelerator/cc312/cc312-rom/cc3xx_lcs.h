/*
 * Copyright (c) 2021, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef CC3XX_LCS_H
#define CC3XX_LCS_H

#include "cc3xx_lcs_defs.h"
#include "cc3xx_error.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Gets the current value of the lifecycle state
 *
 * @param[out]  lcs  Buffer of type \ref cc3xx_lcs_t to hold the retrieved LCS
 *
 * @return cc3xx_err_t
 */
cc3xx_err_t cc3xx_lcs_get(cc3xx_lcs_t* lcs);

/**
 * @brief Returns a string which contains the name of the LCS value passed as input
 *
 * @param[in] lcs  Value of the LCS for which the function returns the name
 *
 * @return const char*
 */
const char* cc3xx_lcs_get_name(cc3xx_lcs_t lcs);

#ifdef __cplusplus
}
#endif

#endif /* CC3XX_LCS_H */
