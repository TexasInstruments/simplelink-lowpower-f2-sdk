/******************************************************************************

 @file  app_ble_config.c

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

#include "app_ble_config.h"

#define countof(array_) \
    (1 \
        ? sizeof(array_)/sizeof((array_)[0]) \
        : sizeof(struct { int do_not_use_countof_for_pointers : ((void*)(array_) == (void*)&array_);}) \
        )

static const sid_ble_cfg_service_t ble_service = {
    .type = AMA_SERVICE,
    .id = {
        .type = UUID_TYPE_16,
        .uu = { 0xFE, 0x03 },
    },
};

static const sid_ble_cfg_descriptor_t ble_desc[] = {
    {
        .id = {
            .type = UUID_TYPE_16,
            .uu = { 0x29, 0x02 },
        },
        .perm = {
            .is_write = true,
        },
    }
};

static const sid_ble_cfg_characteristics_t ble_characteristics[] = {
    {
        .id = {
            .type = UUID_TYPE_128,
            .uu = { 0x3C, 0xC5, 0x61, 0xAB, 0x27, 0x04, 0x32, 0x92,
                        0x58, 0x4D, 0x6C, 0x7D, 0xC9, 0x96, 0xF9, 0x74 },
        },
        .properties = {
            .is_write_no_resp = true,
        },
        .perm = {
            .is_write = true,
        },
    },
    {
        .id = {
            .type = UUID_TYPE_128,
            .uu = { 0xFE, 0xD2, 0xF0, 0xE7, 0xB7, 0x53, 0x15, 0x90,
                        0xC1, 0x47, 0xCE, 0xFE, 0xC0, 0x83, 0x2E, 0xB3 },
        },
        .properties = {
            .is_notify = true,
        },
        .perm = {
            .is_none = true,
        },
    },
};

static const sid_ble_cfg_adv_param_t adv_param = {
    .type = AMA_SERVICE,
    .fast_enabled = true,
    .slow_enabled = true,
    .fast_interval = 256,
    .fast_timeout = 1600,
    .slow_interval = 3000,
    .slow_timeout = 0,
};
static const sid_ble_cfg_conn_param_t conn_param = {
    .min_conn_interval = 16,
    .max_conn_interval = 60,
    .slave_latency = 0,
    .conn_sup_timeout = 400,
};
static const sid_ble_cfg_gatt_profile_t ble_profile[] = {
    {
        .service = ble_service,
        .char_count = countof(ble_characteristics),
        .characteristic = ble_characteristics,
        .desc_count = countof(ble_desc),
        .desc = ble_desc,
    },
};

static const sid_ble_config_t ble_cfg = {
    .name = "ti-cc13x2",
    .mtu = 247,
    .is_adv_available = true,
    .mac_addr_type = SID_BLE_CFG_MAC_ADDRESS_TYPE_PUBLIC,
    .adv_param = adv_param,
    .is_conn_available = true,
    .conn_param = conn_param,
    .num_profile = countof(ble_profile),
    .profile = ble_profile,
};

static const sid_ble_link_config_t ble_config = {
    .create_ble_adapter = sid_pal_ble_adapter_create,
    .config = &ble_cfg,
};

const sid_ble_link_config_t* app_get_ble_config(void)
{
    return &ble_config;
}
