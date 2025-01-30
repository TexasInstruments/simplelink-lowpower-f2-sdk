/*
 * Copyright (c) 2021-2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef CC3XX_ENGINE_STATE_H
#define CC3XX_ENGINE_STATE_H

#include "cc3xx_error.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

enum cc3xx_engine_t {
    CC3XX_ENGINE_NONE                 = 0b00000u,
    CC3XX_ENGINE_AES                  = 0b00001U,
    CC3XX_ENGINE_AES_TO_HASH          = 0b00010U,
    CC3XX_ENGINE_AES_AND_HASH         = 0b00011U,
    CC3XX_ENGINE_HASH                 = 0b00111U,
    CC3XX_ENGINE_AES_MAC_AND_BYPASS   = 0b01001U,
    CC3XX_ENGINE_AES_TO_HASH_AND_DOUT = 0b01010U,
    CC3XX_ENGINE_CHACHA               = 0b10000U,
};

extern enum cc3xx_engine_t cc3xx_engine_in_use;

/**
 * @brief Sets the engine being currently used
 *
 * @param engine Value of type \ref enum cc3xx_engine_t to be set
 */
void cc3xx_set_engine(enum cc3xx_engine_t engine);

#ifdef __cplusplus
}
#endif

#endif /* CC3XX_ENGINE_STATE_H */
