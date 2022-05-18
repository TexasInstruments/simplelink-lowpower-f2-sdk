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

#ifndef ti_drivers_cryptoutils_ecc_ECCInitCC26X1__include
#define ti_drivers_cryptoutils_ecc_ECCInitCC26X1__include

#include <stdint.h>

#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(driverlib/rom_ecc.h)

#ifdef __cplusplus
extern "C" {
#endif

/*!
 *  @brief Initializes the ECC state with NIST P256 curve params.
 *
 *  @note  If a windowSize of 3 is selected, ECC_initialize() ROM function should
 *         be called instead of this function to reduce code size.
 *
 *  @param  state       Points to ECC state.
 *  @param  windowSize  ECC SW window size used for computations.
 *  @param  workZone Points to ECC workzone buffer.
 */
void ECCInitCC26X1_NISTP256(ECC_State *state, uint8_t windowSize, uint32_t *workZone);

/*!
 *  @brief Initializes the ECC state with Curve25519 curve params.
 *
 *  @param  state       Points to ECC state.
 *  @param  windowSize  ECC SW window size used for computations.
 *  @param  workZone Points to ECC workzone buffer.
 */
void ECCInitCC26X1_Curve25519(ECC_State *state, uint8_t windowSize, uint32_t *workZone);

#ifdef __cplusplus
}
#endif

#endif /* ti_drivers_cryptoutils_ecc_ECCInitCC26X1__include */
