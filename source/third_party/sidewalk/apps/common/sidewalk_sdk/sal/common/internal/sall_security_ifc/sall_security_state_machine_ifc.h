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

#ifndef SALL_SECURITY_STATE_MACHINE_IFC_H
#define SALL_SECURITY_STATE_MACHINE_IFC_H

#include <sid_security_crypt_mask_ifc.h>
#include <sid_flex_types.h>
#include <sid_asl_types.h>

#include <sid_network_address.h>
#include <sid_error.h>

#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

static const struct sid_address chimera_service = {
    .type = SID_LL_ATYPE_CLOUD,
    .addr = {0x26},
};

static const struct sid_address mobile_gateway = {
    .type = SID_LL_ATYPE_CLOUD,
    .addr = {0x27},
};

static const struct sid_address prov_service = {
    .type = SID_LL_ATYPE_CLOUD,
    .addr = {0x24},
};

static const struct sid_address metrics_service = {
    .type = SID_LL_ATYPE_CLOUD,
    .addr = {0x22},
};

static const struct sid_address network_server = {
    .type = SID_LL_ATYPE_CLOUD,
    .addr = {0x21},
};

/**
 * @brief Structure that holds Input params for decrypting a received buffer
 */
struct sall_security_state_machine_decrypt_params {
    const uint8_t *in;                    /**< Buffer with data to decrypt */
    const size_t in_size;                 /**< Size of input buffer */
    uint8_t *out;                         /**< Buffer to hold decrypted data */
    size_t out_size;                      /**< Output buffer size, will return decrypted buffer size */
    struct sid_flex *flex;                /**< Input flex buffer for the state machine to fill */
    sid_security_crypt_mask_t crypt_mask; /**< Crypt mask used to decrypt data */
};

/**
 * @brief Structure that holds Input params for encrypting a buffer that should be transmitted
 */
struct sall_security_state_machine_encrypt_params {
    const uint8_t *in;     /**< Buffer with data to encrypt */
    const size_t in_size;  /**< Size of the input buffer */
    uint8_t *out;          /**< Buffer to hold encrypted data */
    size_t out_size;       /**< Output buffer size, will hold the size of encrypted data */
    uint16_t class;        /**< Class of the response to be encrypted */
    uint16_t id;           /**< Class ID of the response to be encrypted */
    struct sid_flex *flex; /**< Outgoing Flex structure for the state machine to fill */
};

/**
 * @brief Structure that holds Input asl params for decrypting a received buffer
 */
struct sall_security_state_machine_asl_decrypt_params {
    const uint8_t *in;                               /**< Buffer with data to decrypt */
    const size_t in_size;                            /**< Size of input buffer */
    uint8_t *scratch_buf;                            /**< Scratch buffer to hold decrypted data */
    size_t scratch_size;                             /**< Size of scratch buffer */
    struct sid_flex *flex;                           /**< Input flex buffer for the state machine to fill */
    struct app_pl_frame_deserialized_buffer *buffer; /**< Buffer to hold deserialized data */
    sid_security_crypt_mask_t crypt_mask;            /**< Crypt mask used to decrypt data */
};

/**
 * @brief Structure that holds Input params for encrypting a buffer that should be transmitted
 */
struct sall_security_state_machine_asl_encrypt_params {
    uint8_t *scratch_buf;                       /**< Scratch buffer to hold intermediate encrypted data */
    size_t scratch_size;                        /**< Size of scratch buffer */
    uint16_t class;                             /**< Class of the response to be encrypted */
    uint16_t id;                                /**< Class ID of the response to be encrypted */
    struct sid_flex *flex;                      /**< Outgoing Flex structure for the state machine to fill */
    struct app_pl_frame *pl_frame;              /**< Presentation content to encrypt */
    struct app_cmd_frame *cmd_frame;            /**< Application content to encrypt */
    struct app_pl_frame_serialized_buffer *out; /**< Buffer to hold encrypted data */
};

/**
 * Decrypt received buffer
 *
 * @return result of execution
 *
 * @param[in, out] params pointer to sall_security_state_machine_decrypt_params_t
 */
sid_error_t sall_security_state_machine_decrypt(struct sall_security_state_machine_decrypt_params *params);

/**
 * Encrypt buffer before transmitting
 *
 * @return result of execution
 *
 * @param[in, out] params pointer to sall_security_state_machine_encrypt_params_t
 */
sid_error_t sall_security_state_machine_encrypt(struct sall_security_state_machine_encrypt_params *params);

/**
 * Decrypt received asl buffer
 *
 * @return result of execution
 *
 * @param[in, out] params pointer to sall_security_state_machine_asl_decrypt_params_t
 */
sid_error_t sall_security_state_machine_asl_decrypt(struct sall_security_state_machine_asl_decrypt_params *params);

/**
 * Encrypt asl buffer before transmitting
 *
 * @return result of execution
 *
 * @param[in, out] params pointer to sall_security_state_machine_asl_encrypt_params_t
 */
sid_error_t sall_security_state_machine_asl_encrypt(struct sall_security_state_machine_asl_encrypt_params *params);

#ifdef __cplusplus
}
#endif

#endif   // SALL_SECURITY_STATE_MACHINE_IFC_H
