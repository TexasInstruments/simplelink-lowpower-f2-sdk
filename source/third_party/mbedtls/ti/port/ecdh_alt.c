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
#include <mbedtls/build_info.h>
#include <mbedtls/ecdh.h>
#include <mbedtls/ecp.h>
#include <mbedtls/error.h>
#include <mbedtls/platform.h>
#include <mbedtls/platform_util.h>

#if defined MBEDTLS_ECDH_COMPUTE_SHARED_ALT || defined MBEDTLS_ECDH_GEN_PUBLIC_ALT

#include <stdint.h>

#include <ti/drivers/ECDH.h>
#include <ti/drivers/cryptoutils/cryptokey/CryptoKeyPlaintext.h>
#include <ti/devices/DeviceFamily.h>

#if (DeviceFamily_PARENT == DeviceFamily_PARENT_CC13X4_CC26X3_CC26X4)
    #include <ti/drivers/ecdh/ECDHCC26X2.h>
#else
    #error "No valid DeviceFamily found for the ECDH alternate implementation!"
#endif

    /* Parameter validation macros based on platform_util.h */
#define ECDH_VALIDATE_RET(cond) MBEDTLS_INTERNAL_VALIDATE_RET(cond, MBEDTLS_ERR_ECP_BAD_INPUT_DATA)
#define ECDH_VALIDATE(cond)     MBEDTLS_INTERNAL_VALIDATE(cond)

#if defined MBEDTLS_ECDH_GEN_PUBLIC_ALT

/**
 * Size in bytes of the identifier at the beginning of the point in big-endian format
 */
#define OCTET_STRING_OFFSET 1

static bool ecdhInitialized = false;
/*
 * Generate public key (restartable version)
 */
static int ecdh_gen_public_restartable(mbedtls_ecp_group *grp,
                                       mbedtls_mpi *d,
                                       mbedtls_ecp_point *Q,
                                       int (*f_rng)(void *, unsigned char *, size_t),
                                       void *p_rng,
                                       mbedtls_ecp_restart_ctx *rs_ctx)
{
    int ret        = 0;
    size_t plen = (grp->nbits + 7)/8;
    uint8_t *q_buf = (uint8_t *)mbedtls_calloc(1, (plen * 2U) + OCTET_STRING_OFFSET);
    uint8_t *d_buf = (uint8_t *)mbedtls_calloc(1, plen);
    CryptoKey myPrivateKey;
    CryptoKey myPublicKey;
    ECDH_Config config = {0};
    ECDH_Handle handle = NULL;
    ECDH_Params ecdhGenPubKeyParams;
    ECDH_OperationGeneratePublicKey operationGeneratePublicKey;
    ECDHCC26X2_HWAttrs hwAttrs = {0};
    ECDHCC26X2_Object object   = {0};

    if ((NULL == q_buf) || (NULL == d_buf))
    {
        ret = MBEDTLS_ERR_ECP_ALLOC_FAILED;
        goto cleanup;
    }

    if (ecdhInitialized == false)
    {
        ECDH_init();
        ecdhInitialized = true;
    }

    hwAttrs.intPriority = (4 << 5);

    config.object  = (void *)&object;
    config.hwAttrs = (void *)&hwAttrs;

    ECDH_Params_init(&ecdhGenPubKeyParams);
    ecdhGenPubKeyParams.returnBehavior = ECDH_RETURN_BEHAVIOR_POLLING;

    /* Zeroise the dynamic object before it is used by ECDH_construct */
    memset(&object, 0x00, sizeof(object));

    handle = ECDH_construct(&config, &ecdhGenPubKeyParams);

    if (NULL == handle)
    {
        ret = MBEDTLS_ERR_PLATFORM_HW_ACCEL_FAILED;
        goto cleanup;
    }

    ECDH_OperationGeneratePublicKey_init(&operationGeneratePublicKey);

#if defined(MBEDTLS_ECP_MONTGOMERY_ENABLED)
    if (mbedtls_ecp_get_type(grp) == MBEDTLS_ECP_TYPE_MONTGOMERY)
    {
        /* ECDH ALT layer only supports Montgomery keys in little endian format */
        (void)CryptoKeyPlaintext_initKey(&myPublicKey, q_buf, plen);
        operationGeneratePublicKey.keyMaterialEndianness = ECDH_LITTLE_ENDIAN_KEY;
        /* The driver consumes little-endian order for Montgomery curves */
        MBEDTLS_MPI_CHK(mbedtls_mpi_write_binary_le(d, d_buf, plen));
    }
#endif
#if defined(MBEDTLS_ECP_SHORT_WEIERSTRASS_ENABLED)
    if (mbedtls_ecp_get_type(grp) == MBEDTLS_ECP_TYPE_SHORT_WEIERSTRASS)
    {
        /* ECDH ALT layer only supports Weierstrass keys in big-endian format */
        (void)CryptoKeyPlaintext_initKey(&myPublicKey, q_buf, (2U * plen) + OCTET_STRING_OFFSET);
        operationGeneratePublicKey.keyMaterialEndianness = ECDH_BIG_ENDIAN_KEY;

        /* The driver consumes big-endian order for Weierstrass curves. */
        MBEDTLS_MPI_CHK(mbedtls_mpi_write_binary(d, d_buf, plen));
    }
#endif

    (void)CryptoKeyPlaintext_initKey(&myPrivateKey, d_buf, plen);

    if (0 != f_rng(p_rng, d_buf, plen))
    {
        ret = MBEDTLS_ERR_ECP_RANDOM_FAILED;
        goto cleanup;
    }
    operationGeneratePublicKey.myPrivateKey = &myPrivateKey;
    operationGeneratePublicKey.myPublicKey  = &myPublicKey;

    /* Load the ROM curve params */
    switch (grp->id)
    {
        case MBEDTLS_ECP_DP_SECP224R1:
            operationGeneratePublicKey.curve = &ECCParams_NISTP224;
            break;

        case MBEDTLS_ECP_DP_SECP256R1:
            operationGeneratePublicKey.curve = &ECCParams_NISTP256;
            break;

        case MBEDTLS_ECP_DP_SECP384R1:
            operationGeneratePublicKey.curve = &ECCParams_NISTP384;
            break;

        case MBEDTLS_ECP_DP_SECP521R1:
            operationGeneratePublicKey.curve = &ECCParams_NISTP521;
            break;

        case MBEDTLS_ECP_DP_BP256R1:
            operationGeneratePublicKey.curve = &ECCParams_BrainpoolP256R1;
            break;

        case MBEDTLS_ECP_DP_BP384R1:
            operationGeneratePublicKey.curve = &ECCParams_BrainpoolP384R1;
            break;

        case MBEDTLS_ECP_DP_BP512R1:
            operationGeneratePublicKey.curve = &ECCParams_BrainpoolP512R1;
            break;

        case MBEDTLS_ECP_DP_CURVE25519:
            operationGeneratePublicKey.curve = &ECCParams_Curve25519;
            break;

        default:
            /* Possible extension to load an arbitrary curve */
            ret = MBEDTLS_ERR_ECP_FEATURE_UNAVAILABLE;
            goto cleanup;
    }

    if (ECDH_STATUS_SUCCESS == ECDH_generatePublicKey(handle, &operationGeneratePublicKey))
    {
        ret = 0;
    }
    else
    {
        ret = MBEDTLS_ERR_PLATFORM_HW_ACCEL_FAILED;
        goto cleanup;
    }

#if defined(MBEDTLS_ECP_MONTGOMERY_ENABLED)
    if (mbedtls_ecp_get_type(grp) == MBEDTLS_ECP_TYPE_MONTGOMERY)
    {
        /* ECDH ALT layer only supports Montgomery keys in little endian format without octet offset byte */
        mbedtls_ecp_point_read_binary(grp, Q, q_buf, plen);
    }
#endif
#if defined(MBEDTLS_ECP_SHORT_WEIERSTRASS_ENABLED)
    if (mbedtls_ecp_get_type(grp) == MBEDTLS_ECP_TYPE_SHORT_WEIERSTRASS)
    {
        /* ECDH ALT layer only supports Weierstrass keys in big-endian format with octet offset bytes
         * Do not copy the octet offset in the mbedtls shared secret buffer */
        mbedtls_ecp_point_read_binary(grp, Q, q_buf, ((plen * 2U) + OCTET_STRING_OFFSET));
    }
#endif

cleanup:
    mbedtls_free(q_buf);
    mbedtls_free(d_buf);

    if (NULL != handle)
    {
        ECDH_close(handle);
    }

    return (ret);
}

/*
 * Generate public key
 */
int mbedtls_ecdh_gen_public(mbedtls_ecp_group *grp,
                            mbedtls_mpi *d,
                            mbedtls_ecp_point *Q,
                            int (*f_rng)(void *, unsigned char *, size_t),
                            void *p_rng)
{
    ECDH_VALIDATE_RET(grp != NULL);
    ECDH_VALIDATE_RET(d != NULL);
    ECDH_VALIDATE_RET(Q != NULL);
    ECDH_VALIDATE_RET(f_rng != NULL);
    return (ecdh_gen_public_restartable(grp, d, Q, f_rng, p_rng, NULL));
}
#endif /* MBEDTLS_ECDH_GEN_PUBLIC_ALT */

#if defined MBEDTLS_ECDH_COMPUTE_SHARED_ALT
/*
 * Compute shared secret (SEC1 3.3.1)
 */
static int ecdh_compute_shared_restartable(mbedtls_ecp_group *grp,
                                           mbedtls_mpi *z,
                                           const mbedtls_ecp_point *Q,
                                           const mbedtls_mpi *d,
                                           int (*f_rng)(void *, unsigned char *, size_t),
                                           void *p_rng,
                                           mbedtls_ecp_restart_ctx *rs_ctx)
{
    int ret     = 0;
    size_t plen = (grp->nbits + 7)/8;
    size_t olen;
    uint8_t *q_buf = (uint8_t *)mbedtls_calloc(1, (plen * 2U) + OCTET_STRING_OFFSET);
    uint8_t *s_buf = (uint8_t *)mbedtls_calloc(1, (plen * 2U) + OCTET_STRING_OFFSET);
    uint8_t *d_buf = (uint8_t *)mbedtls_calloc(1, plen);
    CryptoKey myPrivateKey;
    CryptoKey theirPublicKey;
    CryptoKey sharedSecret;
    ECDH_Config config = {0};
    ECDH_Handle handle = NULL;
    ECDH_Params ecdhCompShSecParams;
    ECDH_OperationComputeSharedSecret operationComputeSharedSecret;
    ECDHCC26X2_HWAttrs hwAttrs = {0};
    ECDHCC26X2_Object object   = {0};

    if ((NULL == q_buf) || (NULL == s_buf) || (NULL == d_buf))
    {
        ret = MBEDTLS_ERR_ECP_ALLOC_FAILED;
        goto cleanup;
    }

    if (ecdhInitialized == false)
    {
        ECDH_init();
        ecdhInitialized = true;
    }

    hwAttrs.intPriority = (4 << 5);

    config.object  = (void *)&object;
    config.hwAttrs = (void *)&hwAttrs;

    ECDH_Params_init(&ecdhCompShSecParams);
    ecdhCompShSecParams.returnBehavior = ECDH_RETURN_BEHAVIOR_POLLING;

    /* Zeroise the dynamic object before it is used by ECDH_construct */
    memset(&object, 0x00, sizeof(object));

    handle = ECDH_construct(&config, &ecdhCompShSecParams);

    if (NULL == handle)
    {
        ret = MBEDTLS_ERR_PLATFORM_HW_ACCEL_FAILED;
        goto cleanup;
    }

    ECDH_OperationComputeSharedSecret_init(&operationComputeSharedSecret);

#if defined(MBEDTLS_ECP_MONTGOMERY_ENABLED)
    if (mbedtls_ecp_get_type(grp) == MBEDTLS_ECP_TYPE_MONTGOMERY)
    {
        /* ECDH ALT layer only supports Montgomery keys in little endian format */
        ret = mbedtls_ecp_point_write_binary(grp, Q, MBEDTLS_ECP_PF_UNCOMPRESSED, &olen, q_buf, plen);
        CryptoKeyPlaintext_initBlankKey(&sharedSecret, s_buf, plen);
        (void)CryptoKeyPlaintext_initKey(&theirPublicKey, q_buf, plen);
        operationComputeSharedSecret.keyMaterialEndianness = ECDH_LITTLE_ENDIAN_KEY;
        /* The driver consumes little-endian order for Montgomery curves */
        MBEDTLS_MPI_CHK(mbedtls_mpi_write_binary_le(d, d_buf, plen));
    }
#endif
#if defined(MBEDTLS_ECP_SHORT_WEIERSTRASS_ENABLED)
    if (mbedtls_ecp_get_type(grp) == MBEDTLS_ECP_TYPE_SHORT_WEIERSTRASS)
    {
        /* ECDH ALT layer only supports Weierstrass keys in big-endian format */
        ret = mbedtls_ecp_point_write_binary(grp, Q, MBEDTLS_ECP_PF_UNCOMPRESSED, &olen, q_buf, ((plen * 2U) + OCTET_STRING_OFFSET));
        CryptoKeyPlaintext_initBlankKey(&sharedSecret, s_buf, (2U * plen) + OCTET_STRING_OFFSET);
        (void)CryptoKeyPlaintext_initKey(&theirPublicKey, q_buf, (2U * plen) + OCTET_STRING_OFFSET);
        operationComputeSharedSecret.keyMaterialEndianness = ECDH_BIG_ENDIAN_KEY;

        /* The driver consumes big-endian order for Weierstrass curves. */
        MBEDTLS_MPI_CHK(mbedtls_mpi_write_binary(d, d_buf, plen));
    }
#endif
    if (ret != 0)
    {
        goto cleanup;
    }

    (void)CryptoKeyPlaintext_initKey(&myPrivateKey, d_buf, plen);

    operationComputeSharedSecret.myPrivateKey   = &myPrivateKey;
    operationComputeSharedSecret.theirPublicKey = &theirPublicKey;
    operationComputeSharedSecret.sharedSecret   = &sharedSecret;

    /* Load the ROM curve params */
    switch (grp->id)
    {
        case MBEDTLS_ECP_DP_SECP224R1:
            operationComputeSharedSecret.curve = &ECCParams_NISTP224;
            break;

        case MBEDTLS_ECP_DP_SECP256R1:
            operationComputeSharedSecret.curve = &ECCParams_NISTP256;
            break;

        case MBEDTLS_ECP_DP_SECP384R1:
            operationComputeSharedSecret.curve = &ECCParams_NISTP384;
            break;

        case MBEDTLS_ECP_DP_SECP521R1:
            operationComputeSharedSecret.curve = &ECCParams_NISTP521;
            break;

        case MBEDTLS_ECP_DP_BP256R1:
            operationComputeSharedSecret.curve = &ECCParams_BrainpoolP256R1;
            break;

        case MBEDTLS_ECP_DP_BP384R1:
            operationComputeSharedSecret.curve = &ECCParams_BrainpoolP384R1;
            break;

        case MBEDTLS_ECP_DP_BP512R1:
            operationComputeSharedSecret.curve = &ECCParams_BrainpoolP512R1;
            break;

        case MBEDTLS_ECP_DP_CURVE25519:
            operationComputeSharedSecret.curve = &ECCParams_Curve25519;
            break;

        default:
            /* Possible extension to load an arbitrary curve */
            ret = MBEDTLS_ERR_ECP_FEATURE_UNAVAILABLE;
            goto cleanup;
    }

    if (ECDH_STATUS_SUCCESS == ECDH_computeSharedSecret(handle, &operationComputeSharedSecret))
    {
        ret = 0;
    }
    else
    {
        ret = MBEDTLS_ERR_PLATFORM_HW_ACCEL_FAILED;
        goto cleanup;
    }

#if defined(MBEDTLS_ECP_MONTGOMERY_ENABLED)
    if (mbedtls_ecp_get_type(grp) == MBEDTLS_ECP_TYPE_MONTGOMERY)
    {
        /* ECDH ALT layer only supports Montgomery keys in little endian format without octet offset byte */
        MBEDTLS_MPI_CHK(mbedtls_mpi_read_binary_le(z, s_buf, plen));
    }
#endif
#if defined(MBEDTLS_ECP_SHORT_WEIERSTRASS_ENABLED)
    if (mbedtls_ecp_get_type(grp) == MBEDTLS_ECP_TYPE_SHORT_WEIERSTRASS)
    {
        /* ECDH ALT layer only supports Weierstrass keys in big-endian format with octet offset bytes
         * Do not copy the octet offset in the mbedtls shared secret buffer */
        MBEDTLS_MPI_CHK(mbedtls_mpi_read_binary(z, s_buf + OCTET_STRING_OFFSET, plen));
    }
#endif

cleanup:
    mbedtls_free(q_buf);
    mbedtls_free(d_buf);
    mbedtls_free(s_buf);

    if (NULL != handle)
    {
        ECDH_close(handle);
    }

    return (ret);
}

/*
 * Compute shared secret (SEC1 3.3.1)
 */
int mbedtls_ecdh_compute_shared(mbedtls_ecp_group *grp,
                                mbedtls_mpi *z,
                                const mbedtls_ecp_point *Q,
                                const mbedtls_mpi *d,
                                int (*f_rng)(void *, unsigned char *, size_t),
                                void *p_rng)
{
    ECDH_VALIDATE_RET(grp != NULL);
    ECDH_VALIDATE_RET(Q != NULL);
    ECDH_VALIDATE_RET(d != NULL);
    ECDH_VALIDATE_RET(z != NULL);
    return (ecdh_compute_shared_restartable(grp, z, Q, d, f_rng, p_rng, NULL));
}
#endif /* MBEDTLS_ECDH_COMPUTE_SHARED_ALT */

#endif /* MBEDTLS_ECDH_COMPUTE_SHARED_ALT || MBEDTLS_ECDH_GEN_PUBLIC_ALT */