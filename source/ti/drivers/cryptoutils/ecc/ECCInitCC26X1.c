/*
 * Copyright (c) 2021, Texas Instruments Incorporated
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

#include <stdint.h>

#include <ti/drivers/cryptoutils/ecc/ECCInitCC26X1.h>
#include <ti/drivers/cryptoutils/ecc/ECCParams.h>

#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(driverlib/rom_ecc.h)

/*
 *  ======== ECCInitCC26X1_NISTP256 ========
 */
void ECCInitCC26X1_NISTP256(ECC_State *state, uint8_t windowSize, uint32_t *workZone)
{
    state->data_Gx     = ECC_NISTP256_generatorX.word;
    state->data_Gy     = ECC_NISTP256_generatorY.word;
    state->data_p      = ECC_NISTP256_prime.word;
    state->data_r      = ECC_NISTP256_order.word;
    state->data_a      = ECC_NISTP256_a.word;
    state->data_b      = ECC_NISTP256_b.word;
    state->data_a_mont = ECC_NISTP256_a_mont.word;
    state->data_b_mont = ECC_NISTP256_b_mont.word;
    state->data_k_mont = ECC_NISTP256_k_mont.word;
    state->win         = windowSize;

    state->workzone = workZone;
}

/*
 *  ======== ECCInitCC26X1_Curve25519 ========
 */
void ECCInitCC26X1_Curve25519(ECC_State *state, uint8_t windowSize, uint32_t *workZone)
{
    state->data_Gx     = ECC_Curve25519_generatorX.word;
    state->data_Gy     = ECC_Curve25519_generatorY.word;
    state->data_p      = ECC_Curve25519_prime.word;
    state->data_r      = ECC_Curve25519_order.word;
    state->data_a      = ECC_Curve25519_a.word;
    state->data_b      = ECC_Curve25519_b.word;
    /* The following montgomery curve params are not used for any Curve25519 functions */
    state->data_a_mont = NULL;
    state->data_b_mont = NULL;
    state->data_k_mont = NULL;
    state->win         = windowSize;

    state->workzone = workZone;
}
