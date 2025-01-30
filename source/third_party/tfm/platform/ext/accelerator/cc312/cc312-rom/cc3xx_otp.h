/*
 * Copyright (c) 2021-2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef CC3XX_OTP_H
#define CC3XX_OTP_H

#include "cc3xx_error.h"

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Write contents from the OTP memory
 *
 * @param[out] otp_addr Address to write to
 * @param[in]  size     Size in bytes of the data to write
 * @param[in]  buf      Buffer containing the data to be written
 *
 * @return cc3xx_err_t
 */
cc3xx_err_t cc3xx_otp_write(uint8_t *otp_addr, size_t size, const uint8_t *buf);

/**
 * @brief Read contents from the OTP memory
 *
 * @param[in]  otp_addr Address to read from
 * @param[in]  size     Size in bytes of the data to read
 * @param[out] buf      Buffer that will be holding the retrieved data from OTP
 *
 * @return cc3xx_err_t
 */
cc3xx_err_t cc3xx_otp_read(const uint8_t *otp_addr, size_t size, uint8_t *buf);

#ifdef __cplusplus
}
#endif

#endif /* CC3XX_OTP_H */
