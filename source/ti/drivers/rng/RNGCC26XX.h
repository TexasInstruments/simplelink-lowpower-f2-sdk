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
/** ============================================================================
 *  @file       RNGCC26XX.h
 *
 *  @brief      RNG driver implementation for the CC26XX family
 *
 *  This file should only be included in the board file to fill the RNG_config
 *  structure.
 *
 *  The CC26XX family has a general purpose TRNG. This implementation of the
 *  RNG driver for the CC26XX family uses the TRNG for all entropy generation.
 *
 */

#ifndef ti_drivers_rnd_RNGCC26XX__include
#define ti_drivers_rnd_RNGCC26XX__include

#include <stdint.h>
#include <stdbool.h>

#include <ti/drivers/RNG.h>
#include <ti/drivers/TRNG.h>
#include <ti/drivers/trng/TRNGCC26XX.h>

#ifdef __cplusplus
extern "C" {
#endif

/*! \cond Internal APIs */

/*!
 *  @brief RNGCC26XX Object
 *
 *  \note The application must not access any member variables of this structure!
 */
typedef struct
{
    TRNG_Config trngConfig;
    TRNGCC26XX_Object trngObject;
    RNG_Params rngParams;
    volatile bool operationInProgress;
    bool operationCanceled;
} RNGCC26XX_Object;

/*! \endcond */

#ifdef __cplusplus
}
#endif

#endif /* ti_drivers_rnd_RNGCC26XX__include */
