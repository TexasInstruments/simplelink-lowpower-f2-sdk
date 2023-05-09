/******************************************************************************

 @file  sid_pal_spi_client_legacy.h

 @brief Describe the purpose and contents of the file.

 Group: WCS, LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************

 Copyright (c) 2023, Texas Instruments Incorporated

 All rights reserved not granted herein.
 Limited License.

 Texas Instruments Incorporated grants a world-wide, royalty-free,
 non-exclusive license under copyrights and patents it now or hereafter
 owns or controls to make, have made, use, import, offer to sell and sell
 ("Utilize") this software subject to the terms herein. With respect to the
 foregoing patent license, such license is granted solely to the extent that
 any such patent is necessary to Utilize the software alone. The patent
 license shall not apply to any combinations which include this software,
 other than combinations with devices manufactured by or for TI ("TI
 Devices"). No hardware patent is licensed hereunder.

 Redistributions must preserve existing copyright notices and reproduce
 this license (including the above copyright notice and the disclaimer and
 (if applicable) source code license limitations below) in the documentation
 and/or other materials provided with the distribution.

 Redistribution and use in binary form, without modification, are permitted
 provided that the following conditions are met:

   * No reverse engineering, decompilation, or disassembly of this software
     is permitted with respect to any software provided in binary form.
   * Any redistribution and use are licensed by TI for use only with TI Devices.
   * Nothing shall obligate TI to provide you with source code for the software
     licensed and provided to you in object code.

 If software source code is provided to you, modification and redistribution
 of the source code are permitted provided that the following conditions are
 met:

   * Any redistribution and use of the source code, including any resulting
     derivative works, are licensed by TI for use only with TI Devices.
   * Any redistribution and use of any object code compiled from the source
     code and any resulting derivative works, are licensed by TI for use
     only with TI Devices.

 Neither the name of Texas Instruments Incorporated nor the names of its
 suppliers may be used to endorse or promote products derived from this
 software without specific prior written permission.

 DISCLAIMER.

 THIS SOFTWARE IS PROVIDED BY TI AND TI'S LICENSORS "AS IS" AND ANY EXPRESS
 OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 IN NO EVENT SHALL TI AND TI'S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 ******************************************************************************


 *****************************************************************************/

/*
 * Copyright 2022 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.TXT file. This file is a
 * Modifiable File, as defined in the accompanying LICENSE.TXT file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef SID_PAL_SPI_CLIENT_H
#define SID_PAL_SPI_CLIENT_H

#include <sid_error.h>
#include <sid_time_types.h>
#include <sid_protocol_defs.h>
#include <sid_pal_serial_client_ifc.h>

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// TODO: These configs must be passed to init function as structure.
#define SRL_HOST_MAX_FRAME_SZ 256
#define SRL_FRAME_TYPE_TEST_MODE 0x40
#define SRL_FRAME_TYPE_NORMAL_MODE 0x80

#define SRL_FRAME_TEST_MODE_VERSION 0x1
#define SRL_FRAME_NORMAL_MODE_VERSION 0x1

#define SRL_FLAGS_WAIT_FOR_ACK 0x01
#define SRL_FLAGS_CRC_CHECK    0x02
#define SRL_FLAGS_SEQN_CHECK   0x04

#define SPIS_DEFAULT_CHAR 0xC0

#define SPIS_XFER_SZ    128

#define SPIS_DF_HDR_SZ 3
#define SPIS_PAYLOAD_IN_XFER (SPIS_XFER_SZ - SPIS_DF_HDR_SZ - 2)

#define SPIS_FT_DF      0x2
#define SPIS_FT_ACK     0x3
#define SPIS_FT_NACK    0x4
#define SPIS_FT_STATUS  0x5

#define SPIS_FT_LOG     0xD0

#define SPIS_ACK_FRAME_LENGTH 4
#define SPIS_NACK_FRAME_LENGTH 5

typedef struct {
    volatile uint8_t frame_to_srl_vld;
    volatile uint16_t frame_to_srl_len;
    uint8_t frame_to_srl[RNET_MAX_FRAME_SZ];
    uint8_t frame_from_srl_vld:1;
    uint16_t frame_from_srl_len;
    uint8_t frame_from_srl[RNET_MAX_FRAME_SZ];
    uint8_t srl_flags;
    uint8_t srl_wait_for_acks_ena:1;
    uint8_t srl_crc_check_ena:1;
    uint8_t srl_seqn_check_ena:1;
    volatile uint8_t requesting_xfer;
    uint8_t srl_b2b_xfer_dly_ms;
    uint16_t resend_tmout_ms;
    uint8_t inj_err_prob;
    uint8_t inj_dropped_prob;
    uint8_t dbg_back_pressure;
    struct sid_timespec last_xfer_done_ts;
    struct sid_timespec last_int_assert_ts;
    uint8_t tm_on;
} serial_host_st_t;

sid_error_t sid_pal_spi_client_create(sid_pal_serial_ifc_t const **_this, const void *config, const sid_pal_serial_params_t *params);

#ifdef __cplusplus
}
#endif


#endif /* SID_PAL_SPI_CLIENT_H */
