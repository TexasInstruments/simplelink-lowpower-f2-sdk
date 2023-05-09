/*
 * Copyright 2021 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.txt file. This file is a
 * Modifiable File, as defined in the accompanying LICENSE.TXT file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef SID_RCP_INTERFACE_H
#define SID_RCP_INTERFACE_H

/** @file
 *
 * @defgroup SRCP_LIB Sidewalk Remote Certificate Provisioning (SRCP) Library
 *
 * This is a reference implementation of an SRCP library. This provides
 * functions to help users provision Sidewalk credentials onto devices which
 * are already deployed in the field.
 *
 * @{
 * @ingroup  SRCP_LIB
 */

#include <sid_error.h>
#include <stddef.h>
#include <stdint.h>

#define SID_RCP_APID_SIZE 4

#define SID_RCP_ED25519_PRK_SIZE 32
#define SID_RCP_ED25519_PUK_SIZE 32
#define SID_RCP_ED25519_SIG_SIZE 64

#define SID_RCP_P256R1_PRK_SIZE 32
#define SID_RCP_P256R1_PUK_SIZE 64
#define SID_RCP_P256R1_SIG_SIZE 64

#define SID_RCP_SHA256_SIZE 32

#define SID_RCP_SMSN_SIZE SID_RCP_SHA256_SIZE
#define SID_RCP_TR_ID_SIZE SID_RCP_SHA256_SIZE
#define SID_RCP_ATTESTED_CSR_HASH_SIZE SID_RCP_SHA256_SIZE

#define SID_RCP_CSR_STATUS_SIZE 1

#define SID_RCP_ED25519_CERT_SIZE 500
#define SID_RCP_P256R1_CERT_SIZE 660

/**
 * Sidewalk Device CSR (Certificate signing request)
 */
struct sid_rcp_csr {
    /* Inputs */

    /**
     * Sidewalk Manufacturing Serial Number
     * @see sid_rcp_create_smsn()
     */
    const uint8_t *smsn;

    /* Outputs */
    /** Transaction id */
    uint8_t *tr_id; // SID_RCP_TR_ID_SIZE

    /** ED25519 key pair */
    uint8_t *ed25519_prk; // SID_RCP_ED25519_PRK_SIZE
    uint8_t *ed25519_puk; // SID_RCP_ED25519_PUK_SIZE

    /** P256R1 key pair */
    uint8_t *p256r1_prk; // SID_RCP_P256R1_PRK_SIZE
    uint8_t *p256r1_puk; // SID_RCP_P256R1_PUK_SIZE

    /** Output signature */
    uint8_t *signature; /** type/length defined by callback implementation */
};

/**
 * Parameters for a signing implementation
 */
struct sid_rcp_sign_param {
    /* Inputs */

    const uint8_t *message;
    size_t msg_size;

    /** Context passed to callback from original caller */
    void *context;

    /* Outputs */

    /** Output signature */
    uint8_t *signature; /** type/length defined by callback implementation */
};

/**
 * Input parameters to sid_rcp_create_smsn()
 *
 */
struct sid_rcp_dev_info {
    /* Inputs */

    const uint8_t *dev_type; /** Device type */
    size_t dev_type_size;

    const uint8_t *apid; /** Advertised product id */

    const uint8_t *dsn; /** Device serial number */
    size_t dsn_size;
};

struct sid_rcp_param {
    /* Inputs */
    void *context; /** passed via sid_rcp_sign_param */

    /**
     * Signing implementation used by sid_rcp_start_srcp
     */
    sid_error_t (*sign_cb)(struct sid_rcp_sign_param *param);
};

/**
 * Signed device CSR response from Sidewalk SRCP service passed to
 * sid_rcp_finalize_srcp()
 */
struct sid_rcp_csr_resp {
    /* input */
    const uint8_t *ed25519_sig;
    const uint8_t *p256r1_sig;
    const uint8_t *tr_id;
    const uint8_t *attested_csr_hash;
    const uint8_t *csr_resp_sig;
    size_t csr_resp_sig_size;
};

/**
 * Flag for a 3P service or mobile app to understand whether the device
 * successfully processed the signed device CSR response from the Sidewalk SRCP
 * service.
 *
 * In the reference implementation in the Sidewalk Device MCU SDK this needs to
 * match the values expected by the device registration scripts, which includes
 * an emulation of the 3P service.
 *
 * @see sid_rcp_finalize_srcp()
 * @see sid_rcp_resp_param.csr_status
 */
enum sid_rcp_resp_csr_status {
    SRCP_RESP_CSR_DEVICE_SUCCESS = 0,
    SRCP_RESP_CSR_DEVICE_FAILURE = 1,
};

/**
 * Input and output parameters for sid_rcp_finalize_srcp()
 */
struct sid_rcp_resp_param {
    /* Inputs */
    const uint8_t *ed25519_prk;
    const uint8_t *ed25519_puk;
    const uint8_t *p256r1_prk;
    const uint8_t *p256r1_puk;
    /** @see sid_rcp_create_smsn() */
    const uint8_t *smsn;
    /** Sidewalk SRCP service public key, it is different across cloud stages */
    const uint8_t *sidewalk_puk;
    /** ed25519 certificate chain as a byte array */
    const uint8_t *ed25519_cert;
    /** p256r1 certificate chain as a byte array */
    const uint8_t *p256r1_cert;
    /** Advertised product id */
    const uint8_t *apid;
    /** Sidewalk Application server ed25519 pub key */
    const uint8_t *app_server_puk;
    /** context passed to sign_cb() */
    void *context;

    /**
     * Signing implementation which will generate csr_status_sig
     * @param[in,out] param Signing callback
     */
    sid_error_t (*sign_cb)(struct sid_rcp_sign_param *param);

    /* Outputs */

    /**
     * Output containing result of processing the signed device CSR response.
     * @see sid_rcp_resp_csr_status for values
     * @see SID_RCP_CSR_STATUS_SIZE
     */
    uint8_t *csr_status;
    /**
     * Output signature generated by sign_cb()
     */
    uint8_t *csr_status_sig;
};

/**
 * Create Sidewalk Manufacturing Serial Number (SMSN)
 *
 * SMSN = SHA256(Device Type|DSN|APID)
 *
 * If using the Sidewalk Developer Service aka ACS console, the device type and
 * APID are part of the JSON payload obtained when you generate a new device
 * certificate.
 *
 * The DSN should uniquely identify an instance of what will become a Sidewalk
 * Edge device within a Device Type family
 */
sid_error_t sid_rcp_create_smsn(struct sid_rcp_dev_info *dev_info, uint8_t *smsn);

/**
 * Generate a CSR to be passed to the 3P and Sidewalk SRCP cloud services
 *
 * @param[in,out] csr       SRCP payload containing generated CSR
 * @param[in]     req_param @see sid_rcp_param
 */
sid_error_t sid_rcp_start_srcp(struct sid_rcp_csr *csr, const struct sid_rcp_param *req_param);

/**
 * Finalize SRCP device processing.
 *
 * Verify receiving SRCP response with the Sidewalk SRCP public key.
 *
 * Verify incoming Sidewalk device ed25519 signature.
 *
 * Verify incoming Sidewalk device p256r1 signature.
 *
 * Generate SRCP status signature @see sid_rcp_resp_param.csr_status_sig and
 * sid_rcp_resp_param.sign_cb().
 *
 * sid_rcp_resp_param.csr_status contains the state of the SRCP payload after
 * processing.
 *
 * @param[in]     csr_resp   Signed device CSR response from Sidewalk SRCP service
 * @param[in,out] resp_param Contains keys, SMSN, and output signature
 */
sid_error_t sid_rcp_finalize_srcp(const struct sid_rcp_csr_resp *csr_resp, struct sid_rcp_resp_param *resp_param);

/** @} */

#endif // SID_RCP_INTERFACE_H
