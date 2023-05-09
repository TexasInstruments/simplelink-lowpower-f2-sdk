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

#ifndef SEC_GW_STATE_MACHINE_H
#define SEC_GW_STATE_MACHINE_H

#include <sid_network_address.h>
#include <sid_error.h>

#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SEC_SM_GCM_KEY_SIZE 16
#define SEC_SM_EXTENDED_NONCE_SIZE 12

enum sec_gw_sm_key_info_op {
    SEC_GW_SM_KEY_INFO_OP_TRANSIENT_STORE = 0,       /**< Store keys to ram */
    SEC_GW_SM_KEY_INFO_OP_FLUSH_TRANSIENT_STORE = 1, /**< Flush stored keys to flash */
    SEC_GW_SM_KEY_INFO_OP_CLEAR_STORE = 2,           /**< Clear keys from ram, flash saved keys will not be removed */
};

/**
 * @brief Structure that holds the info for initializing state machine
 */
struct sec_gw_sm_info {
    uint8_t gcm_key[SEC_SM_GCM_KEY_SIZE];
    uint8_t peer_nonce[SEC_SM_EXTENDED_NONCE_SIZE];
};

/**
 * @brief Structure that holds the config for state machine initializing
 */
struct sec_gw_config {
    struct sid_address address;
};

/**
 * Save generated WAN keys
 *
 * @return result of execution
 *
 * @param[in] keys pointer to struct sec_gw_sm_info that holds the generated keys
 * @param[in] type of enum sec_gw_sm_key_info_op operation
 *
 * @note The info field has to be given for SEC_GW_SM_KEY_INFO_OP_TRANSIENT_STORE,
 *       info field is ignored for the other options. This api is called to save
 *       the derived WAN key into flash, the api contains can be called
 *       consecutively to store network server derived wan key and then the
 *       application server derived key.
 */
sid_error_t sec_gw_sm_save_key_info(const struct sec_gw_sm_info *info, enum sec_gw_sm_key_info_op op_code);

/**
 * Set device address for state machine to use
 *
 * @param[in] address to set in the state machine
 */
void sec_gw_sm_set_address(const struct sid_address *const address);

const struct sid_address *const sec_gw_sm_get_address(void);

/**
 * Reset security state machine
 */
void sec_gw_sm_reset(void);

/**
 * Initialize security state machine
 *
 * @param[in] config for the security state machine
 */
void sec_gw_sm_init(const struct sec_gw_config *config);

#ifdef __cplusplus
}
#endif

#endif /*! SEC_GW_STATE_MACHINE_H */
