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

#ifndef SEC_DEFINES_H
#define SEC_DEFINES_H

#ifdef __cplusplus
extern "C" {
#endif

#define SEC_DEVICE_SERIAL_NUMBER_LEN 5
#define SEC_SERIAL_NUMBER_LEN        4
#define SEC_CLOUD_CERT_SERIAL_LEN    12

#define SEC_SMSN_LEN                 32
#define SEC_PRK_LEN                  32
#define SEC_PUK_LEN                  32
#define SEC_P256R1_PUK_LEN           64
#define SEC_SIG_LEN                  64
#define SEC_SHA256_LEN               32

#define SEC_AUTH_TAG_SIZE            4
#define SEC_ECDH_SHARED_SECRET_SIZE  32
#define SEC_GCM_KEY_SIZE             16
#define SEC_EXTENDED_NONCE_SIZE      12

#define SEC_BEAMS_NONCE_SIZE         8
#define SEC_BEAMS_COUNTER_SIZE       6
#define SEC_BEAMS_IV_SIZE            12
#define SEC_BEAMS_AAD_SIZE           8
#define SEC_BEAMS_AUTH_TAG_SIZE      12
#define SEC_BEAMS_FIXED_DISTINCT_LEN 6
#define SEC_BEAMS_CONTEXT_SIZE       48

#define SEC_AUTH_TAG_MAX             16

#define SEC_SCRATCH_BUF_SIZE         326

#define SEC_TEST_MFG_DATA_VERSION    0x83

#ifdef __cplusplus
}
#endif

#endif /*! SEC_DEFINES_H */
