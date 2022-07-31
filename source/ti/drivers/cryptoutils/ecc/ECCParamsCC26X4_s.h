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

#ifndef ti_drivers_cryptoutils_ecc_ECCParamsCC26X4_s__include
#define ti_drivers_cryptoutils_ecc_ECCParamsCC26X4_s__include

#include <ti/drivers/cryptoutils/ecc/ECCParams.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 *  @brief  Returns a pointer to curve params stored in secure memory
 *
 *  @param  curveParams  Points to curve param struct provided by non-secure
 *                       code which references a curve by its name.
 *  @return Pointer to the curve params stored in secure memory.
 */
const ECCParams_CurveParams *ECCParams_s_getCurveParams(const ECCParams_CurveParams *curveParams);

/*!
 *  @brief  Veneer to extract the curve generator point from an ecliptic curve
 *          description.
 *
 *  @note   See ECCParams_s_getUncompressedGeneratorPoint() in ECCParam.h for full
 *          description, parameters, and return values. PSA_ERROR_PROGRAMMER_ERROR
 *          is an additional return value.
 *
 *  @retval PSA_ERROR_PROGRAMMER_ERROR if any args point to secure addresses.
 */
int_fast16_t ECCParams_s_getUncompressedGeneratorPoint(const ECCParams_CurveParams *curveParams,
                                                       uint8_t *buffer,
                                                       size_t length);

#ifdef __cplusplus
}
#endif

#endif /* ti_drivers_cryptoutils_ecc_ECCParamsCC26X4_s__include */
