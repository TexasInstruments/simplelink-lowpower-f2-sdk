/*
 * Copyright 2021-2022 Amazon.com, Inc. or its affiliates.  All rights reserved.
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

#ifndef SEC_GW_CREDENTIALS_H
#define SEC_GW_CREDENTIALS_H

#include <sec_defines.h>

#include <sid_error.h>

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct sec_gw_credentials {
    uint8_t cloud_25519_public_key[SEC_PUK_LEN];
    uint8_t cloud_25519_serial_number[SEC_CLOUD_CERT_SERIAL_LEN];
    uint8_t manufacturer_25519_public_key[SEC_PUK_LEN];
    uint8_t manufacturer_25519_serial_number[SEC_SERIAL_NUMBER_LEN];
    uint8_t product_25519_public_key[SEC_PUK_LEN];
    uint8_t product_25519_signature[SEC_SIG_LEN];
    uint8_t product_25519_serial_number[SEC_SERIAL_NUMBER_LEN];
    uint8_t device_25519_private_key[SEC_PRK_LEN];
    uint8_t device_25519_public_key[SEC_PUK_LEN];
    uint8_t device_25519_signature[SEC_SIG_LEN];
    uint8_t device_25519_serial_number[SEC_DEVICE_SERIAL_NUMBER_LEN];
};

sid_error_t sec_gw_get_prod_credentials(const struct sec_gw_credentials **credentials);

void sec_gw_release_prod_credentials(const struct sec_gw_credentials *credentials);

#ifdef __cplusplus
}
#endif

#endif /*! SEC_GW_CREDENTIALS_H */

