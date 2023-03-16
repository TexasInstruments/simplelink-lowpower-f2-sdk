/*
 * Copyright (c) 2022, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <ti/drivers/attestation/Attestation_PSA.h>
#include <ti/drivers/crypto/CryptoCC26X4_ns.h>
#include <ti/drivers/Power.h>
#include <ti/drivers/power/PowerCC26X2.h>
#include <ti/drivers/power/PowerCC26XX.h>
#include <ti/drivers/TRNG.h>

#include <psa/client.h>
#include <psa/initial_attestation.h>

/* Static globals */
static bool isInitialized = false;

/*
 *  ======== Attestation_PSA_mapPSAToAttestationPSAStatus ========
 */
static int_fast16_t Attestation_PSA_mapPSAToAttestationPSAStatus(psa_status_t psaStatus)
{
    int_fast16_t status = ATTESTATION_PSA_STATUS_ERROR;

    if (psaStatus == PSA_SUCCESS)
    {
        status = ATTESTATION_PSA_STATUS_SUCCESS;
    }
    else if (psaStatus == PSA_ERROR_INVALID_ARGUMENT)
    {
        status = ATTESTATION_PSA_STATUS_INVALID_INPUTS;
    }

    return status;
}

/*
 *  ======== Attestation_PSA_setup ========
 */
static int_fast16_t Attestation_PSA_setup(void)
{
    int_fast16_t status = ATTESTATION_PSA_STATUS_SUCCESS;

    /* Set power dependency for SHA2 */
    (void)Power_setDependency(PowerCC26XX_PERIPH_CRYPTO);

    /* Set power dependency for PKA for ECDSA driver */
    (void)Power_setDependency(PowerCC26X2_PERIPH_PKA);

    /* Try to get semaphore for Crypto PSA access */
    if (CryptoPSACC26X4_acquireLock(SemaphoreP_NO_WAIT) == false)
    {
        status = ATTESTATION_PSA_STATUS_RESOURCE_UNAVAILABLE;
    }

    return status;
}

/*
 *  ======== Attestation_PSA_cleanup ========
 */
static void Attestation_PSA_cleanup(void)
{
    /* Release power dependencies */
    (void)Power_releaseDependency(PowerCC26XX_PERIPH_CRYPTO);
    (void)Power_releaseDependency(PowerCC26X2_PERIPH_PKA);

    CryptoPSACC26X4_releaseLock();
}

/*
 *  ======== Attestation_PSA_getToken ========
 */
int_fast16_t Attestation_PSA_getToken(const uint8_t *auth_challenge,
                                      size_t challenge_size,
                                      uint8_t *token_buf,
                                      size_t token_buf_size,
                                      size_t *token_size)
{
    int_fast16_t status = ATTESTATION_PSA_STATUS_ERROR;
    psa_status_t psaStatus;

    status = Attestation_PSA_setup();

    if (status != ATTESTATION_PSA_STATUS_SUCCESS)
    {
        return status;
    }

    if (token_buf_size > ATTESTATION_PSA_MAX_TOKEN_SIZE)
    {
        return ATTESTATION_PSA_STATUS_INVALID_INPUTS;
    }

    psaStatus = psa_initial_attest_get_token(auth_challenge, challenge_size, token_buf, token_buf_size, token_size);

    Attestation_PSA_cleanup();

    return Attestation_PSA_mapPSAToAttestationPSAStatus(psaStatus);
}

/*
 *  ======== Attestation_PSA_getTokenSize ========
 */
int_fast16_t Attestation_PSA_getTokenSize(size_t challenge_size, size_t *token_size)
{
    int_fast16_t status;
    psa_status_t psaStatus;

    status = Attestation_PSA_setup();

    if (status != ATTESTATION_PSA_STATUS_SUCCESS)
    {
        return status;
    }

    psaStatus = psa_initial_attest_get_token_size(challenge_size, token_size);

    Attestation_PSA_cleanup();

    return Attestation_PSA_mapPSAToAttestationPSAStatus(psaStatus);
}

/*
 *  ======== Attestation_PSA_init ========
 */
void Attestation_PSA_init(void)
{
    if (!isInitialized)
    {
        /* TRNG is required for ECDSA signature */
        TRNG_init();

        /* Initialize CryptoPSA semaphores */
        CryptoPSACC26X4_init();

        isInitialized = true;
    }
}
