/******************************************************************************
 Copyright (c) 2022-2023, Texas Instruments Incorporated
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions
 are met:

 *  Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.

 *  Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.

 *  Neither the name of Texas Instruments Incorporated nor the names of
    its contributors may be used to endorse or promote products derived
    from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************/

#include <mbedtls/bignum.h>
#include "mbedtls/build_info.h"
#include <mbedtls/ecdsa.h>
#include <mbedtls/error.h>
#include <mbedtls/platform.h>
#include <mbedtls/platform_util.h>

#if defined MBEDTLS_ECDSA_VERIFY_ALT || defined MBEDTLS_ECDSA_SIGN_ALT

/* NOTE: The TI drivers consume these points and big numbers in network byte
 *       order. This is in contrast to the mbedtls_mpi structures which story
 *       these numbers and points in little endian byte order. This file uses
 *       the mpi functions to re-write the buffers into network byte order.
 */

#include <stdint.h>
#include <string.h>

#include <ti/drivers/ECDSA.h>
#include <ti/drivers/cryptoutils/cryptokey/CryptoKeyPlaintext.h>
#include <ti/devices/DeviceFamily.h>

#if (DeviceFamily_PARENT == DeviceFamily_PARENT_CC13X4_CC26X3_CC26X4)
    #include <ti/drivers/ecdsa/ECDSACC26X2.h>
#else
    #error "No valid DeviceFamily found for the ECDSA alternate implementation!"
#endif
#ifdef MBEDTLS_ECDSA_VERIFY_ALT

static bool ecdsaInitialized = false;

/*
 * Verify ECDSA signature of hashed message
 */
int mbedtls_ecdsa_verify(mbedtls_ecp_group *grp,
                         const unsigned char *buf,
                         size_t blen,
                         const mbedtls_ecp_point *Q,
                         const mbedtls_mpi *r,
                         const mbedtls_mpi *s)
{
    int ret     = 0;
    size_t plen = (grp->nbits + 7)/8;
    size_t olen;
    uint8_t *r_buf = (uint8_t *)mbedtls_calloc(1, plen);
    uint8_t *s_buf = (uint8_t *)mbedtls_calloc(1, plen);
    uint8_t *q_buf = (uint8_t *)mbedtls_calloc(1, (plen * 2U) + 1);
    CryptoKey theirPublicKey;
    ECDSA_Config config = {0};
    ECDSA_Handle handle = NULL;
    ECDSA_Params ecdsaVerifyParams;
    ECDSA_OperationVerify operationVerify;
    ECDSACC26X2_HWAttrs hwAttrs = {0};
    ECDSACC26X2_Object object   = {0};

    if ((NULL == r_buf) || (NULL == s_buf) || (NULL == q_buf))
    {
        ret = MBEDTLS_ERR_ECP_ALLOC_FAILED;
        goto cleanup;
    }
    
    if (ecdsaInitialized == false)
    {
        ECDSA_init();
        ecdsaInitialized = true;
    }

    hwAttrs.intPriority     = (4 << 5);
    hwAttrs.trngIntPriority = (5 << 5);

    config.object  = (void *)&object;
    config.hwAttrs = (void *)&hwAttrs;

    ECDSA_Params_init(&ecdsaVerifyParams);
    ecdsaVerifyParams.returnBehavior = ECDSA_RETURN_BEHAVIOR_POLLING;

    /* Zeroise the dynamic object before it is used by ECDSA_construct */
    memset(&object, 0x00, sizeof(object));

    handle = ECDSA_construct(&config, &ecdsaVerifyParams);

    if (NULL == handle)
    {
        ret = MBEDTLS_ERR_PLATFORM_HW_ACCEL_FAILED;
        goto cleanup;
    }

    /* The driver consumes an octet string in network byte order. Copy this
     * point and reverse them again. 
     * Also, the driver only supports NIST P-256 curves for ECDSA, so the length of public key
     * is 2*plen + 1
     */
    ret = mbedtls_ecp_point_write_binary(grp, Q, MBEDTLS_ECP_PF_UNCOMPRESSED, &olen, q_buf, ((plen * 2U) + 1));
    if (ret != 0)
    {
        goto cleanup;
    }

    CryptoKeyPlaintext_initKey(&theirPublicKey, q_buf, (2U * plen) + 1);

    /* The driver also consumes the r and s in network byte order. Copy these
     * buffers and them reverse them again */
    MBEDTLS_MPI_CHK(mbedtls_mpi_write_binary(r, r_buf, plen));
    MBEDTLS_MPI_CHK(mbedtls_mpi_write_binary(s, s_buf, plen));

    /* prepare the operation */
    ECDSA_OperationVerify_init(&operationVerify);
    operationVerify.theirPublicKey = &theirPublicKey;
    operationVerify.hash           = buf;
    operationVerify.r              = r_buf;
    operationVerify.s              = s_buf;

    /* Load the ROM curve params */
    switch (grp->id)
    {
        case MBEDTLS_ECP_DP_SECP224R1:
            operationVerify.curve = &ECCParams_NISTP224;
            break;

        case MBEDTLS_ECP_DP_SECP256R1:
            operationVerify.curve = &ECCParams_NISTP256;
            break;

        case MBEDTLS_ECP_DP_SECP384R1:
            operationVerify.curve = &ECCParams_NISTP384;
            break;

        case MBEDTLS_ECP_DP_SECP521R1:
            operationVerify.curve = &ECCParams_NISTP521;
            break;

        case MBEDTLS_ECP_DP_BP256R1:
            operationVerify.curve = &ECCParams_BrainpoolP256R1;
            break;

        case MBEDTLS_ECP_DP_BP384R1:
            operationVerify.curve = &ECCParams_BrainpoolP384R1;
            break;

        case MBEDTLS_ECP_DP_BP512R1:
            operationVerify.curve = &ECCParams_BrainpoolP512R1;
            break;

        case MBEDTLS_ECP_DP_CURVE25519:
            operationVerify.curve = &ECCParams_Curve25519;
            break;

        default:
            /* Possible extension to load an arbitrary curve */
            ret = MBEDTLS_ERR_ECP_FEATURE_UNAVAILABLE;
            goto cleanup;
    }

    if (ECDSA_STATUS_SUCCESS == ECDSA_verify(handle, &operationVerify))
    {
        ret = 0;
    }
    else
    {
        ret = MBEDTLS_ERR_ECP_VERIFY_FAILED;
    }

cleanup:
    mbedtls_free(r_buf);
    mbedtls_free(s_buf);
    mbedtls_free(q_buf);

    if (NULL != handle)
    {
        ECDSA_close(handle);
    }

    return ret;
}

#endif /* MBEDTLS_ECDSA_VERIFY_ALT */

#ifdef MBEDTLS_ECDSA_SIGN_ALT

/*
 * Compute ECDSA signature of a hashed message
 */
int mbedtls_ecdsa_sign(mbedtls_ecp_group *grp,
                       mbedtls_mpi *r,
                       mbedtls_mpi *s,
                       const mbedtls_mpi *d,
                       const unsigned char *buf,
                       size_t blen,
                       int (*f_rng)(void *, unsigned char *, size_t),
                       void *p_rng)
{
    int ret        = 0;
    size_t plen = (grp->nbits + 7)/8;
    uint8_t *r_buf = (uint8_t *)mbedtls_calloc(1, plen);
    uint8_t *s_buf = (uint8_t *)mbedtls_calloc(1, plen);
    uint8_t *d_buf = (uint8_t *)mbedtls_calloc(1, plen);
    CryptoKey myPrivateKey;
    ECDSA_Config config = {0};
    ECDSA_Params ecdsaSignParams;
    ECDSA_Handle handle = NULL;
    ECDSA_OperationSign operationSign;
    ECDSACC26X2_HWAttrs hwAttrs = {0};
    ECDSACC26X2_Object object   = {0};

    if (NULL == r_buf || NULL == s_buf || NULL == d_buf)
    {
        ret = MBEDTLS_ERR_ECP_ALLOC_FAILED;
        goto cleanup;
    }

    if (ecdsaInitialized == false)
    {
        ECDSA_init();
        ecdsaInitialized = true;
    }

    hwAttrs.intPriority     = (4 << 5);
    hwAttrs.trngIntPriority = (5 << 5);

    config.object  = (void *)&object;
    config.hwAttrs = (void *)&hwAttrs;

    ECDSA_Params_init(&ecdsaSignParams);
    ecdsaSignParams.returnBehavior = ECDSA_RETURN_BEHAVIOR_POLLING;

    /* Zeroise the dynamic object before it is used by ECDSA_construct */
    memset(&object, 0x00, sizeof(object));

    handle = ECDSA_construct(&config, &ecdsaSignParams);

    if (NULL == handle)
    {
        ret = MBEDTLS_ERR_PLATFORM_HW_ACCEL_FAILED;
        goto cleanup;
    }

    /* The driver consumes numbers in network byte order */
    MBEDTLS_MPI_CHK(mbedtls_mpi_write_binary(d, d_buf, plen));

    CryptoKeyPlaintext_initKey(&myPrivateKey, d_buf, plen);
    ECDSA_OperationSign_init(&operationSign);
    operationSign.myPrivateKey = &myPrivateKey;
    operationSign.hash         = buf;
    operationSign.r            = r_buf;
    operationSign.s            = s_buf;

    /* Load the ROM curve params */
    switch (grp->id)
    {
        case MBEDTLS_ECP_DP_SECP224R1:
            operationSign.curve = &ECCParams_NISTP224;
            break;

        case MBEDTLS_ECP_DP_SECP256R1:
            operationSign.curve = &ECCParams_NISTP256;
            break;

        case MBEDTLS_ECP_DP_SECP384R1:
            operationSign.curve = &ECCParams_NISTP384;
            break;

        case MBEDTLS_ECP_DP_SECP521R1:
            operationSign.curve = &ECCParams_NISTP521;
            break;

        case MBEDTLS_ECP_DP_BP256R1:
            operationSign.curve = &ECCParams_BrainpoolP256R1;
            break;

        case MBEDTLS_ECP_DP_BP384R1:
            operationSign.curve = &ECCParams_BrainpoolP384R1;
            break;

        case MBEDTLS_ECP_DP_BP512R1:
            operationSign.curve = &ECCParams_BrainpoolP512R1;
            break;

        case MBEDTLS_ECP_DP_CURVE25519:
            operationSign.curve = &ECCParams_Curve25519;
            break;

        default:
            /* Possible extension to load an arbitrary curve */
            ret = MBEDTLS_ERR_ECP_FEATURE_UNAVAILABLE;
            goto cleanup;
    }

    if (ECDSA_STATUS_SUCCESS == ECDSA_sign(handle, &operationSign))
    {
        ret = 0;
    }
    else
    {
        ret = MBEDTLS_ERR_PLATFORM_HW_ACCEL_FAILED;
    }

    /* The driver produces r and s in network byte order. copy into mbedtls mpi
     * format. This incurs an extra byte reversal when written to ASN1. */
    MBEDTLS_MPI_CHK(mbedtls_mpi_read_binary(r, r_buf, plen));
    MBEDTLS_MPI_CHK(mbedtls_mpi_read_binary(s, s_buf, plen));

cleanup:
    mbedtls_free(r_buf);
    mbedtls_free(s_buf);
    mbedtls_free(d_buf);

    if (NULL != handle)
    {
        ECDSA_close(handle);
    }

    return ret;
}
#endif /* MBEDTLS_ECDSA_SIGN_ALT */

#endif /* defined MBEDTLS_ECDSA_VERIFY_ALT || defined MBEDTLS_ECDSA_SIGN_ALT */