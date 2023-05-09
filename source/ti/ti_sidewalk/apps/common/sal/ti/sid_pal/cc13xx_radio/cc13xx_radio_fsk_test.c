/******************************************************************************

 @file  cc13xx_radio_fsk_test.c

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

#include <sid_pal_radio_ifc.h>
#include "ti/cc13xx.h"
#include <halo/utils.h>

static const uint8_t fsk_mod_shaping[] = {CC13XX_GFSK_MOD_SHAPE_OFF, CC13XX_GFSK_MOD_SHAPE_BT_03,
                                          CC13XX_GFSK_MOD_SHAPE_BT_05, CC13XX_GFSK_MOD_SHAPE_BT_07,
                                          CC13XX_GFSK_MOD_SHAPE_BT_1};

static const uint8_t fsk_bw[] = {CC13XX_GFSK_BW_4800,   CC13XX_GFSK_BW_5800,  CC13XX_GFSK_BW_7300,
                                 CC13XX_GFSK_BW_9700,   CC13XX_GFSK_BW_11700,
                                 CC13XX_GFSK_BW_14600,  CC13XX_GFSK_BW_19500,
                                 CC13XX_GFSK_BW_23400,  CC13XX_GFSK_BW_29300,
                                 CC13XX_GFSK_BW_39000,  CC13XX_GFSK_BW_46900,
                                 CC13XX_GFSK_BW_58600,  CC13XX_GFSK_BW_78200,
                                 CC13XX_GFSK_BW_93800,  CC13XX_GFSK_BW_117300,
                                 CC13XX_GFSK_BW_156200, CC13XX_GFSK_BW_187200,
                                 CC13XX_GFSK_BW_234300, CC13XX_GFSK_BW_312000,
                                 CC13XX_GFSK_BW_373600, CC13XX_GFSK_BW_467000};

static const uint8_t fsk_addr_comp[] = {CC13XX_GFSK_ADDR_CMP_FILT_OFF,
                                        CC13XX_GFSK_ADDR_CMP_FILT_NODE,
                                        CC13XX_GFSK_ADDR_CMP_FILT_NODE_BROAD};

static const uint8_t fsk_preamble_detect[] = {CC13XX_GFSK_PBL_DET_OFF,
                                              CC13XX_GFSK_PBL_DET_08_BITS,
                                              CC13XX_GFSK_PBL_DET_16_BITS,
                                              CC13XX_GFSK_PBL_DET_24_BITS,
                                              CC13XX_GFSK_PBL_DET_32_BITS};

static const uint8_t fsk_crc_type[] = {CC13XX_GFSK_CRC_OFF, CC13XX_GFSK_CRC_1_BYTE,
                                       CC13XX_GFSK_CRC_2_BYTES, CC13XX_GFSK_CRC_1_BYTE_INV,
                                       CC13XX_GFSK_CRC_2_BYTES_INV};

#define FSK_MOD_SHAPING_PARAMS_NUM     countof(fsk_mod_shaping)
#define FSK_BW_NUM                     countof(fsk_bw)
#define FSK_ADDR_COMP_NUM              countof(fsk_addr_comp)
#define FSK_PREAMBLE_DETECT_NUM        countof(fsk_preamble_detect)
#define FSK_CRC_TYPES_NUM              countof(fsk_crc_type)

/*
 * Functions here are place holders for future MAC operation
 */
int32_t sid_pal_radio_get_fsk_mod_shaping(uint8_t idx, uint8_t *ms)
{
    if (idx >= FSK_MOD_SHAPING_PARAMS_NUM) {
        return RADIO_ERROR_INVALID_PARAMS;
    }
    *ms = fsk_mod_shaping[idx];
    return RADIO_ERROR_NONE;
}

int16_t sid_pal_radio_get_fsk_mod_shaping_idx(uint8_t ms)
{
    for (int i = 0; i < FSK_MOD_SHAPING_PARAMS_NUM; i++) {
        if (fsk_mod_shaping[i] == ms) {
            return i;
        }
    }
    return -1;
}

int32_t sid_pal_radio_get_fsk_bw(uint8_t idx, uint8_t *bw)
{
    if (idx >= FSK_BW_NUM) {
        return RADIO_ERROR_INVALID_PARAMS;
    }
    *bw = fsk_bw[idx];
    return RADIO_ERROR_NONE;
}

int16_t sid_pal_radio_get_fsk_bw_idx(uint8_t bw)
{
    for (int i = 0; i < FSK_BW_NUM; i++) {
        if (fsk_bw[i] == bw) {
            return i;
        }
    }
    return -1;
}

int32_t sid_pal_radio_get_fsk_addr_comp(uint8_t idx, uint8_t *ac)
{
    if (idx >= FSK_ADDR_COMP_NUM) {
        return RADIO_ERROR_INVALID_PARAMS;
    }
    *ac = fsk_addr_comp[idx];
    return RADIO_ERROR_NONE;
}


int16_t sid_pal_radio_get_fsk_addr_comp_idx(uint8_t ac)
{
    for (int i = 0; i < FSK_ADDR_COMP_NUM; i++) {
        if (fsk_addr_comp[i] == ac) {
            return i;
        }
    }
    return -1;
}

int32_t sid_pal_radio_get_fsk_preamble_detect(uint8_t idx, uint8_t *pd)
{
    if (idx >= FSK_PREAMBLE_DETECT_NUM) {
        return RADIO_ERROR_INVALID_PARAMS;
    }
    *pd = fsk_preamble_detect[idx];
    return RADIO_ERROR_NONE;
}

int16_t sid_pal_radio_get_fsk_preamble_detect_idx(uint8_t pd)
{
    for (int i = 0; i < FSK_PREAMBLE_DETECT_NUM; i++) {
        if (fsk_preamble_detect[i] == pd) {
            return i;
        }
    }
    return -1;
}

int32_t sid_pal_radio_get_fsk_crc_type(uint8_t idx, uint8_t *crc)
{
    if (idx >= FSK_CRC_TYPES_NUM) {
        return RADIO_ERROR_INVALID_PARAMS;
    }
    *crc = fsk_crc_type[idx];
    return RADIO_ERROR_NONE;
}


int16_t sid_pal_radio_get_fsk_crc_type_idx(uint8_t crc)
{
    for (int i = 0; i < FSK_CRC_TYPES_NUM; i++) {
        if (fsk_crc_type[i] == crc) {
            return i;
        }
    }
    return -1;
}
