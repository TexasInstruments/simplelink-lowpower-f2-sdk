/******************************************************************************

 @file  app_900_config.h

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

#ifndef APP_900_CONFIG_H
#define APP_900_CONFIG_H

#include <cc13xx_config.h>

#define RADIO_MAX_TX_POWER_NA        (20)
#define RADIO_MAX_TX_POWER_EU        (14)

#if defined (REGION_ALL)
#define RADIO_REGION                 RADIO_REGION_NONE
#elif defined (REGION_US915)
#define RADIO_REGION                 RADIO_REGION_NA
#elif defined (REGION_EU868)
#define RADIO_REGION                 RADIO_REGION_EU
#endif
#define RADIO_ANT_GAIN(X)            ((X) * 100)

const radio_cc13xx_regional_param_t radio_cc13xx_regional_param[] =
{
    #if defined (REGION_ALL) || defined (REGION_US915)
    {
        .param_region = RADIO_REGION_NA,
        .max_tx_power = RADIO_MAX_TX_POWER_NA,
        .cca_level_adjust = 0,
        .ant_dbi = RADIO_ANT_GAIN(2.15)
    },
    #endif
    #if defined (REGION_ALL) || defined (REGION_EU868)
    {
        .param_region = RADIO_REGION_EU,
        .max_tx_power = RADIO_MAX_TX_POWER_EU,
        .cca_level_adjust = 0,
        .ant_dbi = RADIO_ANT_GAIN(2.15)
    },
    #endif
};

const radio_cc13xx_device_config_t radio_cc13xx_default_cfg = {
    .id               = TI_ID_CC13x2P,

    .regional_config = {
        .radio_region = RADIO_REGION,
        .reg_param_table_size = sizeof(radio_cc13xx_regional_param) / sizeof(radio_cc13xx_regional_param[0]),
        .reg_param_table = radio_cc13xx_regional_param,
    },

    .state_timings = {
        .sleep_to_full_power_us = 0,
        .full_power_to_sleep_us = 0,
        .rx_to_tx_us = 0,
        .tx_to_rx_us = 0,
    },
};

const struct sid_sub_ghz_links_config sub_ghz_link_config = {
    .enable_link_metrics = true,
    .registration_config = {
        .enable = true,
        .periodicity_s = UINT32_MAX,
    },
};

const struct sid_sub_ghz_links_config* app_get_sub_ghz_config(void)
{
    return &sub_ghz_link_config;
}
#endif
