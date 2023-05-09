/*
 * Copyright 2017-2020 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.txt file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef RNET_SPIS_H
#define RNET_SPIS_H

#include <sid_protocol_opts.h>
#if RNET_SRL_HOST_INTERFACE || RNET_SPI_MASTER_DEBUG_BUILD
#if RNET_HOST_SPI_MCU

#include "rnet_host_interface.h"
#include <sid_protocol_defs.h>

#ifdef __cplusplus
extern "C" {
#endif

#define HOST_MCU_SPIS_INSTANCE 1
#define SPIS_DEFAULT_CHAR 0xC0

#define SPIS_XFER_SZ    128

#define SPIS_DF_HDR_SZ 3
#define SPIS_PAYLOAD_IN_XFER (SPIS_XFER_SZ-SPIS_DF_HDR_SZ-2)

#define SPIS_FT_DF      0x2
#define SPIS_FT_ACK     0x3
#define SPIS_FT_NACK    0x4
#define SPIS_FT_STATUS  0x5

#define SPIS_FT_LOG     0xD0

#define SPIS_ACK_FRAME_LENGTH 4
#define SPIS_NACK_FRAME_LENGTH 5

typedef void(*SpisTxDoneCb)(void);
typedef S8(*SpisRxDoneCb)(U8*, U16);

void rnet_host_spi_init(SpisTxDoneCb tx_cb, SpisRxDoneCb rx_cb);

S8 rnet_host_spi_send(U8* frame_buf, U16 len);
void rnet_spim_debug_process(void);
void rnet_spis_process(void);

#ifdef __cplusplus
}
#endif

#endif // RNET_HOST_SPI_MCU
#endif // RNET_SRL_HOST_INTERRACE

#endif
