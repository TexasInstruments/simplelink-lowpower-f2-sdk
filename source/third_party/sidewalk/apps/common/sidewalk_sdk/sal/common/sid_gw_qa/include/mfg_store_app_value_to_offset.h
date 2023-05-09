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

#ifndef MFG_STORE_APP_VALUE_TO_OFFSET_H
#define MFG_STORE_APP_VALUE_TO_OFFSET_H

#include <sid_pal_mfg_store_ifc.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum mfg_store_app_value {
    MFG_STORE_APP_VALUE_WIFI_MAC =   // This value is for gateways only. It is removed in the V7 spec.
    SID_PAL_MFG_STORE_CORE_VALUE_MAX,
    MFG_STORE_APP_VALUE_ETH_MAC,                   // This value is for gateways only. It is removed in the V7 spec.
    MFG_STORE_APP_VALUE_HW_MODEL_AND_VERSION,      // This is a Nordic specific value.
    MFG_STORE_APP_VALUE_BRIDGE_CLOUD_AUTH_TOKEN,   // This is a bridge specific value. It is removed in the V7 spec.
    MFG_STORE_APP_VALUE_SMTC_TRIM_CAP,             // This is a Semtech radio specific value.
    MFG_STORE_APP_VALUE_PROXY_FLAG,   // This is for the factory / manufacturing tests. It is removed in the V7 spec.
    MFG_STORE_APP_VALUE_CRYSTAL_32MHZ,
    MFG_STORE_APP_VALUE_CRYSTAL_32KHZ,
    MFG_STORE_APP_VALUE_ADC,        // This is for Nordic diag only. It is removed in the V7 spec.
    MFG_STORE_APP_VALUE_BOARD_ID,   // This is for Nordic diags only, it is in the V7 spec
    MFG_STORE_APP_VALUE_BLE_MAC,

    // These values are for the manufacturing store implementation,
    // but are not part of the public interface
    // TODO: Move these to the sid_pal_mfg_store_value_t, but these are optional keys
    MFG_STORE_APP_VALUE_DHAV2_REGION_ADDRESS,
    MFG_STORE_APP_VALUE_DHAV2_REGION_SIZE
};

enum mfg_store_app_value_size {
    MFG_STORE_APP_VALUE_BOARD_ID_SIZE = 16,
    MFG_STORE_APP_VALUE_WIFI_MAC_SIZE = 17,   // NOTE: Does not match V5 spec, but this was the old value used
    MFG_STORE_APP_VALUE_ETH_MAC_SIZE = 17,    // NOTE: Does not match V5 spec, but this was the old value used
    MFG_STORE_APP_VALUE_BLE_MAC_SIZE = 6,
    MFG_STORE_APP_VALUE_HW_MODEL_AND_VERSION_SIZE = 4,
    MFG_STORE_APP_VALUE_BRIDGE_CLOUD_AUTH_TOKEN_SIZE = 37,
    MFG_STORE_APP_VALUE_SMTC_TRIM_CAP_SIZE = 4,
    MFG_STORE_APP_VALUE_PROXY_FLAG_SIZE = 4,
    MFG_STORE_APP_VALUE_CRYSTAL_32MHZ_SIZE = 4,
    MFG_STORE_APP_VALUE_CRYSTAL_32KHZ_SIZE = 4,
    MFG_STORE_APP_VALUE_ADC_SIZE = 4,

    MFG_STORE_APP_VALUE_DHAV2_REGION_ADDRESS_SIZE = 4,
    MFG_STORE_APP_VALUE_DHAV2_REGION_SIZE_SIZE = 4,
};

uint32_t mfg_store_internal_value_to_offset(int value);

#ifdef __cplusplus
}
#endif

#endif /*! MFG_STORE_APP_VALUE_TO_OFFSET_H */
