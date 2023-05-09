/*
 * Copyright 2022 Amazon.com, Inc. or its affiliates.  All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and conditions
 * set forth in the accompanying LICENSE.TXT file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef TI_SPI_RTT_IFC_H
#define TI_SPI_RTT_IFC_H

#include <stdarg.h>
#include <stdint.h>

/**
 *  Print via RTT module and store logs to send via SPI
 *
 * @param[in]   level           Severity of the log
 * @param[in]   fmt             String to be sent
 * @param[in]   ap              Argument list.
 */
void spi_rtt_vprint(uint8_t level, const char *fmt, va_list ap);

/**
 * Allows for retrival of log buffers
 *
 * @param[in] log_buffer Pointer to log buffer descriptor
 *
 * @return true if log strings were copied into log_buffer, false otherwise
 */
bool spi_rtt_get_log_buffer(struct sid_pal_log_buffer *const log_buffer);

#endif   // TI_SPI_RTT_IFC_H
