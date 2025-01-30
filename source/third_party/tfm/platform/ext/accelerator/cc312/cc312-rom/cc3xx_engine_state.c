/*
 * Copyright (c) 2021-2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "cc3xx_engine_state.h"
#include "cc3xx_dev.h"

enum cc3xx_engine_t cc3xx_engine_in_use = CC3XX_ENGINE_NONE;

void cc3xx_set_engine(enum cc3xx_engine_t engine)
{
    /* Wait for the crypto engine to be ready */
    while (P_CC3XX->cc_ctl.crypto_busy) {}

    /* Set the crypto engine to the requested engine */
    P_CC3XX->cc_ctl.crypto_ctl = engine;

    /* crypto_ctl is WO, so mirror the state */
    cc3xx_engine_in_use = engine;

    /* Wait for the crypto engine to be ready */
    while (P_CC3XX->cc_ctl.crypto_busy) {}
}
