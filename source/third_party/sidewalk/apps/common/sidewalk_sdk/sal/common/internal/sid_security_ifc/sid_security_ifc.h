/*
 * Copyright 2020-2022 Amazon.com, Inc. or its affiliates.  All rights reserved.
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

#ifndef SID_SECURITY_IFC_H
#define SID_SECURITY_IFC_H

#include <sid_error.h>

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// FOR GATEWAY_CLASS
#define SID_SEC_GW_AAD_MAX_CLD_ID_SZ 3 // maximum size for cloud address which form AAD

#define SID_SEC_AES_KEY_SZ 16
#define SID_SEC_MAX_KEY_STR_LENGTH 48
#define SID_SEC_REF_SZ 4
#define SID_SEC_TX_UUID_PARAM_SZ 2
#define SID_SEC_DEFAULT_KEY_TTL_SECS 3600 // 1 hour
#define SID_SEC_DEFAULT_TX_UUID_TTL_SECS (SID_SEC_DEFAULT_KEY_TTL_SECS / 4) // 900 secs (15 min)
#define SID_SEC_MIN_TX_UUID_TTL_SECS 30 // 30 secs

#define SID_SEC_CMAC_MIN_TAG_SZ 16
#define SID_SEC_CMAC_MAX_TAG_SZ 16
#define SID_SEC_WAN_CMAC_TAG_SZ 4

//Expected maximum clock drift in sec
#define SID_SEC_TIME_TOLERANCE 30

#define SID_SEC_WAN_KEY_PARAM_SZ 2
#define SID_SEC_WAN_KEY_PARAM_NUM 3
#define SID_SEC_WAN_KEY_PARAMS_TOTAL_SZ (SID_SEC_WAN_KEY_PARAM_SZ * SID_SEC_WAN_KEY_PARAM_NUM)
#define SID_SEC_SET_WAN_PARAMS_PKT_SZ (SID_SEC_REF_SZ              +\
                                    SID_SEC_TX_UUID_PARAM_SZ        +\
                                    SID_SEC_WAN_KEY_PARAMS_TOTAL_SZ)

#define SID_SEC_APP_KEY_PARAM_SZ 2
#define SID_SEC_APP_KEY_PARAM_NUM 1
#define SID_SEC_APP_KEY_PARAMS_TOTAL_SZ (SID_SEC_APP_KEY_PARAM_SZ * SID_SEC_APP_KEY_PARAM_NUM)
#define SID_SEC_SET_APP_PARAMS_PKT_SZ (SID_SEC_APP_KEY_PARAMS_TOTAL_SZ)

#define SID_SEC_GCM_MAC_SZ 4
#define SID_LL_MAX_SEQ_NUM_SZ 3
#define SID_SEC_DEV_ID_SZ 5
#define SID_SEC_IV_SZ (SID_SEC_DEV_ID_SZ + SID_LL_MAX_SEQ_NUM_SZ + SID_SEC_REF_SZ)
#define SID_SEC_AAD_SZ 8
#define SID_SEC_CMAC_AAD_SZ 16
#define SID_SEC_MAX_RF_AAD_SZ 13
#define SID_SEC_MAX_RF_CMAC_AAD_SZ 17

//TTL for the time reference in encryption IV
//During this time, IV has same time reference
#define SID_SEC_IV_REF_TTL 60

//Number of possible key and reference combination
#define SID_SEC_MAX_KEY_REF_LIST_SZ 3

#define SID_SEC_TX_UUID_SZ 5
#define SID_SEC_TX_UUID_PARAMS_SZ 2
#define SID_SEC_SHA256_OUT_SZ 32
#define SID_SEC_TX_UUID_HASH_SZ (SID_SEC_DEV_ID_SZ + SID_SEC_REF_SZ)

typedef enum {
    SID_SEC_KEY_WAN_GCM_128 = 0,
    SID_SEC_KEY_WAN_CMAC_128 = 1,
    SID_SEC_KEY_WAN_TX_UUID_128 = 2,
    SID_SEC_KEY_WAN_TSYNC_RESP_128 = 3,
    SID_SEC_KEY_WAN_JOIN_RESP_GCM_128 = 4,
    SID_SEC_KEY_PAN_GCM_128 = 5,
    SID_SEC_KEY_PAN_CMAC_128 = 6,
    SID_SEC_KEY_APP_CTR_128 = 7,
    SID_SEC_KEY_D2D_GCM_128 = 8,
    SID_SEC_KEY_DERIVED_TIMEREF_LAST = 9,

    SID_SEC_KEY_WAN_GCM_CTRREF_128 = SID_SEC_KEY_DERIVED_TIMEREF_LAST,
    SID_SEC_KEY_WAN_CMAC_CTRREF_128 = 10,
    SID_SEC_KEY_WAN_TX_UUID_CTRREF_128 = 11,
    SID_SEC_KEY_WAN_TSYNC_RESP_CTRREF_128 = 12,
    SID_SEC_KEY_PAN_GCM_CTRREF_128 = 13,
    SID_SEC_KEY_PAN_CMAC_CTRREF_128 = 14,
    SID_SEC_KEY_APP_GCM_CTRREF_128 = 15,
    SID_SEC_KEY_D2D_GCM_CTRREF_128 = 16,
    SID_SEC_KEY_DERIVED_LAST = 17,

    SID_SEC_KEY_PAN_MASTER_128 = SID_SEC_KEY_DERIVED_LAST,
    SID_SEC_KEY_WAN_MASTER_128 = 18,
    SID_SEC_KEY_APP_MASTER_128 = 19,
    SID_SEC_KEY_D2D_MASTER_128 = 20,

    SID_SEC_KEY_LAST = 21,
    SID_SEC_KEY_EXTERNAL_128 = SID_SEC_KEY_LAST
} sid_sec_key_type_t;

#define SID_SEC_NUM_MASTER_KEY (SID_SEC_KEY_LAST - SID_SEC_KEY_DERIVED_LAST)
#define SID_SEC_NUM_DERIVED_TIMEREF_KEY SID_SEC_KEY_DERIVED_TIMEREF_LAST
#define SID_SEC_NUM_DERIVED_CTRREF_KEY (SID_SEC_KEY_DERIVED_LAST - SID_SEC_KEY_DERIVED_TIMEREF_LAST)

typedef enum {
    SID_SEC_TX_UUID_TIME = 0,
    SID_SEC_TX_UUID_COUNTER,
    SID_SEC_TX_UUID_LAST,
} sid_sec_tx_uuid_type_t;

typedef enum {
    SID_SEC_APP_ULINK_ENCRYPT = 0,
    SID_SEC_APP_DLINK_DECRYPT,
    SID_SEC_APP_DLINK_ENCRYPT,
    SID_SEC_APP_ULINK_DECRYPT
} sid_sec_app_crypt_mode_t;

typedef struct {
    sid_sec_key_type_t key_type;
    uint32_t seqn;
    uint8_t seqn_size;
    uint8_t src_id[SID_SEC_DEV_ID_SZ];
    uint8_t const *in_data;
    size_t in_size;
    uint8_t const *aad;
    size_t aad_size;
    uint32_t *reference;
    uint8_t const *ext_key;
    size_t ext_key_size;
} sid_sec_enc_params_t;

typedef struct {
    sid_sec_key_type_t key_type;
    sid_sec_app_crypt_mode_t mode;
    uint32_t seqn;
    uint8_t seqn_size;
    uint8_t src_id[SID_SEC_DEV_ID_SZ];
    uint8_t const *in_data;
    size_t in_size;
    uint32_t *reference;
    uint8_t const *ext_key;
    size_t ext_key_size;
} sid_sec_app_enc_params_t;

typedef struct {
    sid_sec_key_type_t key_type;
    uint8_t const *in_data;
    size_t in_size;
    uint8_t const *ext_ikm;
    size_t ext_ikm_size;
    uint32_t *counter;
    uint32_t seqn;
} sid_sec_sec_auth_params_t;

typedef struct {
    uint8_t const *dev_id;
    size_t id_size;
    sid_sec_tx_uuid_type_t type;
    size_t tx_uuid_size;
    uint32_t ext_reference;
} sid_sec_tx_uuid_params_t;

/**
 * @brief Deinit master key info structure.
 *
 * @param[in]  key_type   type of master key
 *
 * @retval  SID_ERROR_NONE   If the function completed successfully.
 *                            Otherwise, an error code is returned.
 */
sid_error_t sid_sec_deinit_master_key_info(sid_sec_key_type_t key_type);

/**
 * @brief Store master key to persistent memory.
 *
 * @param[in]  key_type   type of master key
 * @param[in]  key        Pointer to the key
 * @param[in]  flush      if true, flush key to persistent storage
 *
 * @retval  SID_ERROR_NONE   If the function completed successfully.
 *                            Otherwise, an error code is returned.
 *
 * @note If flush is true and key is false, the key stored in transient
 *       storage (ram) will be saved to persistent storage
 */
sid_error_t sid_sec_store_master_key(sid_sec_key_type_t key_type, const uint8_t *key, bool flush);

/**
 * @brief Get master key from persistent memory.
 *
 * @param[in]   key_type   type of master key
 * @param[out]  key        Pointer to the key
 * @param[in]   length     key length
 * @retval  SID_ERROR_NONE   If the function completed successfully.
 *                            Otherwise, an error code is returned.
 *
 */
sid_error_t sid_sec_get_master_key(sid_sec_key_type_t key_type, uint8_t *key, size_t length);

/**
 * @brief Store WAN key parameters to persistent memory.
 *
 * @param[in]  params   Pointer to the parameters
 * @param[in]  length   Length of paramters
 *
 * @retval  SID_ERROR_NONE   If the function completed successfully.
 *                            Otherwise, an error code is returned.
 */
sid_error_t sid_sec_store_wan_params(uint8_t *params, size_t length);

/**
 * @brief Store Applicaton key parameters to persistent memory.
 *
 * @param[in]  params   Pointer to the parameters
 * @param[in]  length   Length of paramters
 *
 * @retval  SID_ERROR_NONE   If the function completed successfully.
 *                            Otherwise, an error code is returned.
 */
sid_error_t sid_sec_store_app_params(uint8_t *params, size_t length);

/**
 * @brief Store D2D key parameters to persistent memory.
 *
 * @param[in]  params   Pointer to the parameters
 * @param[in]  length   Length of paramters
 *
 * @retval  SID_ERROR_NONE   If the function completed successfully.
 *                            Otherwise, an error code is returned.
 */
sid_error_t sid_sec_store_d2d_params(uint8_t *params, size_t length);

/**
 * @brief Function for time sync counter increment
 *
 * @retval  SID_ERROR_NONE   If the function completed successfully.
 *                            Otherwise, an error code is returned.
 */
sid_error_t sid_sec_inc_tsync_counter(void);

/**
 * @brief Function for getting time sync counter.
 *
 * @param[out]   counter      pointer to counter
 *
 * @retval  SID_ERROR_NONE   If the function completed successfully.
 *                            Otherwise, an error code is returned.
 */
sid_error_t sid_sec_get_tsync_counter(uint32_t *counter);

#if SID_SECURITY_STACK_TEST
/**
 * @brief  Get time sync counter without seed.
 *         Counter simulation purpose only.
 *
 * @retval  uint32_t   time sync counter value.
 */
uint32_t sid_sec_get_tsync_counter_raw(void);

/**
 * @brief  Set time sync counter.
 *         Counter simulation purpose only.
 *
 * @param[in]   val   value for new counter value
 */
void sid_sec_set_tsync_counter_raw(uint32_t val);
#endif

/**
 * @brief Generate tx_uuid
 *
 * @param[in]    params         Pointer to tx_uuid paramters
 * @param[out]   tx_uuid        Pointer to tx_uuid buffer
 * @param[out]   reference      Pointer to reference used for tx_uuid
 * @param[out]   intvl_secs     Pointer to tx_uuid refresh interval in sec,
 *
 * @retval  SID_ERROR_NONE   If the function completed successfully.
 *                            Otherwise, an error code is returned.
 */
sid_error_t sid_sec_get_tx_uuid(sid_sec_tx_uuid_params_t *params,
                                  uint8_t *tx_uuid,
                                  uint32_t *reference,
                                  uint32_t *intvl_secs);

/**
 * @brief Verify if the tx_uuid is mine
 *
 * @param[in]  params         Pointer to tx_uuid parameters
 * @param[in]  tx_uuid        Pointer to tx_uuid buffer
 *
 * @retval  SID_ERROR_NONE   If the function completed successfully.
 *                            Otherwise, an error code is returned.
 */
sid_error_t sid_sec_verify_tx_uuid(sid_sec_tx_uuid_params_t *params,
                                   uint8_t const *tx_uuid);

/**
 * @brief  Encrypt/Decrypt application data by AES-CTR
 *
 * @param[in]      params    Parameters for app layer encryption
 * @param[out]     out       Pointer to the output data
 *
 * @retval  SID_ERROR_NONE   If the function completed successfully.
 *                            Otherwise, an error code is returned.
 */
sid_error_t sid_sec_app_layer_crypt(sid_sec_app_enc_params_t params,
                                    uint8_t *out,
                                    size_t out_size);

/**
 * @brief  Encrypt data by AEAD
 *
 * @param[in]      params    Parameters for encryption
 * @param[out]     out       Pointer to the output data
 * @param[in]      out_size  Size of output data
 * @param[out]     mac       Pointer to the generated MAC
 * @param[in]      mac_size  Size of mac
 *
 * @retval  SID_ERROR_NONE   If the function completed successfully.
 *                            Otherwise, an error code is returned.
 */
sid_error_t sid_sec_aead_encrypt(sid_sec_enc_params_t params,
                                uint8_t *out,
                                size_t out_size,
                                uint8_t *mac,
                                size_t mac_size);

/**
 * @brief  Decrypt data by AEAD
 *
 * @param[in]      params    Parameters for decryption
 * @param[out]     out       Pointer to the output data
 * @param[in]      out_size  Size of output data
 * @param[in]      mac       Pointer to the MAC
 * @param[in]      mac_size  Size of mac
 *
 * @retval  SID_ERROR_NONE   If the function completed successfully.
 *                            Otherwise, an error code is returned.
 */
sid_error_t sid_sec_aead_decrypt(sid_sec_enc_params_t params,
                                uint8_t *out,
                                size_t out_size,
                                uint8_t *mac,
                                size_t mac_size);

/**
 * @brief Generate MAC of message using AES-CMAC
 *
 * @param[in]   params        Parameters for authentication
 * @param[out]  mac           Pointer to the generated MAC
 * @param[in]   mac_size      Size of mac buffer
 *
 * @retval  SID_ERROR_NONE   If the function completed successfully.
 *                            Otherwise, an error code is returned.
 */
sid_error_t sid_sec_generate_mac(sid_sec_sec_auth_params_t params,
                                    uint8_t *mac,
                                    size_t mac_size);

/**
 * @brief Verify MAC of message using AES-CMAC
 *
 * @param[in]   params        Parameters for authentication
 * @param[in]   mac           Pointer to the generated MAC
 * @param[in]   mac_size      Size of mac buffer
 *
 * @retval  SID_ERROR_NONE   If the function completed successfully.
 *                            Otherwise, an error code is returned.
 */
sid_error_t sid_sec_verify_mac(sid_sec_sec_auth_params_t params,
                                    uint8_t const *mac,
                                    size_t mac_size);

#ifdef __cplusplus
}
#endif

#endif /* SID_SECURITY_IFC_H */
