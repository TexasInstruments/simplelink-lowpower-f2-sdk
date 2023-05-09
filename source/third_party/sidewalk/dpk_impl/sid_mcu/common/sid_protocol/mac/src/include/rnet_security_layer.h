/*
 * Copyright 2018-2020 Amazon.com, Inc. or its affiliates. All rights reserved.
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

#ifndef RNET_SECURITY_LAYER_H
#define RNET_SECURITY_LAYER_H

#include <rnet_errors.h>
#include <sid_protocol_opts.h>
#include "rnet_frames.h"
#include "rnet_time.h"
#include "rnet_app_layer.h"
#include "rnet_app_sup_layer.h"
#include <sid_protocol_defs.h>
#include "rnet_nw_layer.h"
#include "rnet_dev_addr_def.h"

#define IV_SIZE                     12
#define AAD_SIZE                    8
#define MAX_AUTH_TAG_SIZE           16
#define MAX_AAD_SIZE                15
#define PSEUDO_RAND_SIZE            16

typedef enum {
  UNKNOWN = 0,
  ENCRYPT,
  DECRYPT,
  AUTHENTICATION,
  SECURITY,
} rnet_crypto_error_t;

#if RNET_SECURITY_BUILD
#define ECDH_SHARED_SECRET_SIZE     32
#define ECDH_25519_PUK_SIZE         32
#define ECDH_256R1_PUK_SIZE         64
#define ECDH_PRK_SIZE               32
#define SIGNATURE_SIZE              64
#define GCM_KEY_SIZE                16
#define RNET_NONCE_SIZE             8
#define RNET_NONCE_EXTENDED_SIZE    12
#define SECURE_CONTEXT_SIZE         48
#define MAX_SIGNATURE_VERIFY_HASH_SIZE  90

#define CHECK_CERT_SIZE             5
#define CLOUD_CERT_SERIAL_SIZE      12
#define SERIAL_NUMBER_SIZE          4

#define IKM_A_SIZE                  80
#define AUTH_TAG_SIZE               12
#define COUNTER_SIZE                6
#define RANDOM_SIZE                 8
#define FIXED_DISTINCT_LEN          6
#define REMOTE_CERTCOUNT_NONCE_ECDH_PUK 42
#if HALO_FFS_PROVISIONING
#define RESP_PROV_INIT_REQ_SIZE     46
#define PROV_REQ_HASH_SIZE          5
#define MAX_SECURITY_PAYLOAD_SIZE   64
#define SEC_CAPABILITIES_SIZE       2
#define APID_SIZE                   4
#endif
typedef enum {
  SECURITY_DISABLED = 0,
  ECDH_25519_ED_25519_AES_CMAC_GCM_128,
  ECDH_256R1_ED_25519_AES_CMAC_GCM_128 // Not Supported
} rnet_security_mode_t;

typedef enum {
  DEVICE = 0,
  MODEL,
  MANUFACTURER,
  ROOT          // AMZN
} rnet_intermediate_cert_mode_t;

typedef enum {
  CLOUD_SIGNATURE = 0,
  ECDH_CLOUD_SIGNATURE,
  ECDH_CLOUD_SIG_WITH_DEVICE_CLOUD_CERT
} rnet_verify_mode_t;

typedef enum {
  NOT_APPLICABLE = 0,
  BLE,
  CLOUD,
  CLOUD_FFS
} rnet_operating_mode_t;

typedef enum {
  DEVICE_NOT_CONFIGURED = 0,
  START_BLE_SECURE_SETUP,
  SECURE_BLE_SETUP_COMPLETE,
  START_CLOUD_SECURE_SETUP,
  CLOUD_SECURE_SETUP_COMPLETE,
  START_CLOUD_FFS_SECURE_SETUP,
  CLOUD_FFS_SECURE_SETUP_COMPLETE,
  DEVICE_ONLINE
} rnet_setup_state_t;

typedef struct {
  U8    intermediate_cert;
  U8    frame_count;
  union {
    U8  nonce[RNET_NONCE_SIZE];
    U8  nonce_extended[RNET_NONCE_EXTENDED_SIZE];
  };
  U8    cloud_cert_serial_number_count;
  U8    intermediate_cert_serial[SERIAL_NUMBER_SIZE];
} rnet_device_cred_t;

typedef struct {
  bool  is_complete;
  U8    frame_count;
  U8    cert_count;
  union {
    U8  nonce[RNET_NONCE_SIZE];
    U8  nonce_extended[RNET_NONCE_EXTENDED_SIZE];
  };
  U8    cert_serial_number[SERIAL_NUMBER_SIZE];
  U8    cert_puk[ECDH_25519_PUK_SIZE];
  U8    cert_sig[SIGNATURE_SIZE];
} rnet_remote_cred_t;

typedef struct {
  rnet_device_cred_t    self;
  rnet_remote_cred_t    peer;
} rnet_session_cred_t;

typedef struct {
  U64   counter;
} rnet_enc_context_t;

typedef struct {
  rnet_enc_context_t        e_ctx;
  U8    shared_secret[ECDH_SHARED_SECRET_SIZE];
  U8    gcm_key[GCM_KEY_SIZE];
  bool  gcm_key_valid;
} rnet_key_management_t;

typedef struct {
  // Device ECDH{Pub, Prk, Signature }
  U8    self_puk[ECDH_25519_PUK_SIZE];
  U8    self_prk[ECDH_PRK_SIZE];
  U8    self_sig[SIGNATURE_SIZE];

  // Remote ECDH{puk, Signature}
  U8    remote_puk[ECDH_25519_PUK_SIZE];
  U8    remote_sig[SIGNATURE_SIZE];
} rnet_ecdh_cred_t;

typedef struct {
  rnet_ecdh_cred_t      ecdh;
  rnet_session_cred_t   session;
  rnet_key_management_t km;
} rnet_cred_t;

typedef struct {
  rnet_operating_mode_t op_mode;
  rnet_setup_state_t    state;
  rnet_cred_t           ble;
  rnet_cred_t           cloud;
  rnet_cred_t           cloud_ffs;
} rnet_context_t;

typedef struct {
  U8    device_serial[RNET_DEV_ID_SZ];
  U8    device_25519_prk[ECDH_PRK_SIZE];
  U8    device_25519_puk[ECDH_25519_PUK_SIZE];
  U8    device_25519_sig[SIGNATURE_SIZE];

  U8    product_25519_serial[SERIAL_NUMBER_SIZE];
  U8    product_25519_prk[ECDH_PRK_SIZE];
  U8    product_25519_puk[ECDH_25519_PUK_SIZE];
  U8    product_25519_sig[SIGNATURE_SIZE];

  U8    manu_25519_serial[SERIAL_NUMBER_SIZE];
  U8    manu_25519_prk[ECDH_PRK_SIZE];
  U8    manu_25519_puk[ECDH_25519_PUK_SIZE];
  U8    manu_25519_sig[SIGNATURE_SIZE];

  U8    amz_25519_serial[SERIAL_NUMBER_SIZE];
  U8    amz_25519_prk[ECDH_PRK_SIZE];
  U8    amz_25519_puk[ECDH_25519_PUK_SIZE];
  U8    amz_25519_sig[SIGNATURE_SIZE];

  // Embedded into Device Firmware
  U8    cloud_25519_serial[CLOUD_CERT_SERIAL_SIZE]; // cert[4(end-cert),4(intermediate),4(root)]
  U8    cloud_25519_puk[ECDH_25519_PUK_SIZE];
  U8    cloud_25519_sig[SIGNATURE_SIZE];

  U8    device_p256r1_prk[ECDH_PRK_SIZE];
  U8    device_p256r1_puk[ECDH_256R1_PUK_SIZE];
  U8    device_p256r1_sig[SIGNATURE_SIZE];
} rnet_factory_cred_t;

void rnet_read_factory_sec_settings();
void rnet_security_init();
bool rnet_prod_device();
rnet_setup_state_t rnet_get_state();
bool rnet_honor_cmd(U8* buf);
void rnet_error_from_remote(U8* data, rnet_link_t rcv_link);
void rnet_cleanup(rnet_crypto_error_t type, U8 error, rnet_link_t dest);
bool rnet_block_cmds_over_ble(U16 class_id, U16 cmd_id);
bool rnet_command_need_security(U16 class_id, U16 cmd_id);
void rnet_state_transition(rnet_setup_state_t state);
void rnet_set_state(bool status);
rnet_cred_t* rnet_get_cred();
void rnet_set_op_mode(rnet_operating_mode_t mode);
rnet_operating_mode_t rnet_get_op_mode();
rnet_error_t rnet_save_context(rnet_operating_mode_t mode, bool flag);
rnet_error_t rnet_load_context(rnet_operating_mode_t mode);
rnet_error_t rnet_get_sec_cap(U8 mode, U8* sec_cap, U8* idx);
rnet_error_t rnet_verify_intermediate_cert_serial(U8* data);
rnet_error_t rnet_get_dev_credentials(U16 cmd_id, U8* data, U8* idx);
rnet_error_t rnet_get_ecdh_puk_sig(U16 cmd_id, U8* data, U8* idx);
rnet_error_t rnet_set_remote_credentials(cmd_frame_t* cmd, U8 req_idx);
rnet_error_t rnet_verify_remote_cert(rnet_cred_t* cred);
rnet_error_t rnet_eddsa_signature_verify(rnet_cred_t* cred, rnet_verify_mode_t mode);
rnet_error_t rnet_gen_device_ecdh_keypair(rnet_cred_t* cred);
rnet_error_t rnet_sign_device_puk(rnet_cred_t* cred);
rnet_error_t rnet_random_num_generate(U8* rand_num, size_t size);
rnet_error_t rnet_gen_shared_secret(rnet_cred_t* cred);
rnet_error_t rnet_gen_gcm_key(rnet_cred_t* cred);
rnet_error_t rnet_hand_shake(cmd_frame_t* cmd, U8* resp_data, U8* idx, U8 req_idx);
void rnet_resp_hand_shake(cmd_frame_t* cmd, U8 req_idx);
rnet_error_t rnet_decrypt(U8 mode, U8 *in_buf, size_t in_len, U8* out_buf, size_t* out_buf_len);
rnet_error_t rnet_encrypt(U8 mode, U8 *in_buf, size_t in_len, U8* out_buf, size_t* out_buf_len);
void rnet_encode_data_for_encrypt(U8 mode, size_t in_len, U8* iv, U8* aad);
void rnet_parse_data_for_decrypt(U8 mode, U8* in_data, U8* iv, U8* aad, U8* auth_tag);
void rnet_sec_set_peer_nonce(const uint8_t * buffer, size_t len);
void rnet_sec_set_gcm_key(const uint8_t * buffer, size_t len);
void rnet_sec_gcm_key_set_valid(bool valid);
bool rnet_sec_gcm_key_valid(void);

#if HALO_FFS_PROVISIONING
/**
 * @brief Function to handle provisioning init commands
 *
 * @param[in]  cmd        pointer to received command.
 * @param[in]  req_idx    index of command data start
 *
 * @return                execution status
 */
rnet_error_t rnet_prov_handle(cmd_frame_t* cmd, U8 req_idx);
/**
 * @brief Function to populate PROV_INIT_REQ command
 *
 * @param[out] req      pointer to prov_init_req command
 *
 * @return              execution status
 */
rnet_error_t rnet_get_prov_init_req(rnet_cmd_t *req);
/**
 * @brief Function to poulate NTFY_PROV_INIT_REQ_SIG command
 *
 * @param[out] sig    pointer to ntfy_prov_init_req_sig command.
 * @param[in] req     pointer to prov_init_req command.
 *
 * @return            execution status
 */
rnet_error_t rnet_get_prov_init_req_sig(rnet_cmd_t *sig, rnet_cmd_t *req);
#endif
#endif /* RNET_SECURITY_BUILD */

#endif
