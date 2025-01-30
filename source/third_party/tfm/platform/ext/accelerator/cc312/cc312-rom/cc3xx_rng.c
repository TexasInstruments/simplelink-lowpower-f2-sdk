/*
 * Copyright (c) 2021-2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "cc3xx_rng.h"

#include "cc3xx_error.h"
#include "cc3xx_dev.h"

#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#ifdef CC3XX_CONFIG_RNG_EXTERNAL_TRNG
#include "cc3xx_rng_external_trng.h"
#endif /* CC3XX_CONFIG_RNG_EXTERNAL_TRNG */

static void rng_init(void)
{
    /* Enable clock */
    P_CC3XX->rng.rng_clk_enable = 0x1U;

    /* reset trng */
    P_CC3XX->rng.rng_sw_reset = 0x1U;

    /* Apparently there's no way to tell that the reset has finished, so just do
     * these things repeatedly until they succeed (and hence the reset has
     * finished). Works because the reset value of SAMPLE_CNT1 is 0xFFFF.
     */
    do {
        /* Enable clock */
        P_CC3XX->rng.rng_clk_enable = 0x1U;

        /* Set subsampling ratio */
        P_CC3XX->rng.sample_cnt1 = CC3XX_CONFIG_RNG_SUBSAMPLING_RATE;

    } while (P_CC3XX->rng.sample_cnt1 != CC3XX_CONFIG_RNG_SUBSAMPLING_RATE);

    /* Temporarily disable the random source */
    P_CC3XX->rng.rnd_source_enable = 0x0U;

    /* Clear the interrupts */
    P_CC3XX->rng.rng_icr = 0x3FU;

    /* Mask all interrupts except EHR_VALID */
    P_CC3XX->rng.rng_imr = 0x3EU;

    /* Select the oscillator ring (And set SOP_SEL to 0x1 as is mandatory) */
    P_CC3XX->rng.trng_config = CC3XX_CONFIG_RNG_RING_OSCILLATOR_ID | (0x1U << 2);

    /* Set debug control register to no bypasses */
    P_CC3XX->rng.trng_debug_control = 0x0U;

    /* Enable the random source */
    P_CC3XX->rng.rnd_source_enable = 0x1U;
}

static void rng_finish(void)
{
    /* Disable the random source */
    P_CC3XX->rng.rnd_source_enable = 0x0U;

    /* Disable clock */
    P_CC3XX->rng.rng_clk_enable = 0x0U;
}

static cc3xx_err_t fill_entropy_buf(uint32_t *buf) {
    uint32_t attempt_count = 0;
    uint32_t idx;

    /* Wait until the RNG has finished. Any status other than 0x1 indicates
     * that either the RNG hasn't finished or a statistical test has been
     * failed.
     */
    do {
        if (P_CC3XX->rng.rng_isr & 0xEU) {
            /* At least one test has failed - the buffer contents aren't
             * random.
             */

            /* Reset EHR registers */
            P_CC3XX->rng.rst_bits_counter = 0x1U;

            /* Clear the interrupt bits to restart generator */
            P_CC3XX->rng.rng_icr = 0x3FU;

            attempt_count++;
        }
    } while ((! (P_CC3XX->rng.rng_isr & 0x1U))
             && attempt_count < CC3XX_CONFIG_RNG_MAX_ATTEMPTS);

    if (attempt_count == CC3XX_CONFIG_RNG_MAX_ATTEMPTS) {
        rng_finish();
        return CC3XX_ERR_RNG_TOO_MANY_ATTEMPTS;
    }

    /* Reset EHR register */
    P_CC3XX->rng.rst_bits_counter = 0x1U;

    /* Make sure the interrupt is cleared before the generator is
     * restarted, to avoid a race condition with the hardware
     */
    P_CC3XX->rng.rng_icr = 0xFFFFFFFF;

    /* Reading the EHR_DATA restarts the generator */
    for (idx = 0; idx < 6; idx++) {
        buf[idx] = P_CC3XX->rng.ehr_data[idx];
    }

    return CC3XX_ERR_SUCCESS;
}

#ifdef CC3XX_CONFIG_RNG_ENABLE
#ifndef CC3XX_CONFIG_RNG_EXTERNAL_TRNG
cc3xx_err_t cc3xx_rng_get_random(uint8_t* buf, size_t length)
{
    static uint32_t entropy_buf[6];
    static size_t entropy_buf_used_idx = sizeof(entropy_buf);
    size_t copy_size;
    cc3xx_err_t err;

    rng_init();

    while(length > 0) {
        copy_size = sizeof(entropy_buf) - entropy_buf_used_idx < length ?
                    sizeof(entropy_buf) - entropy_buf_used_idx : length;

        /* Fill from entropy buffer if we still have some */
        memcpy(buf, ((uint8_t*)entropy_buf) + entropy_buf_used_idx, copy_size);
        length -= copy_size;
        buf += copy_size;
        entropy_buf_used_idx += copy_size;

        if (length == 0) {
            err = CC3XX_ERR_SUCCESS;
            goto out;
        }

        err = fill_entropy_buf(entropy_buf);
        if (err != CC3XX_ERR_SUCCESS) {
            goto out;
        }
        entropy_buf_used_idx = 0;
    }

    err = CC3XX_ERR_SUCCESS;
out:
    rng_finish();

    return err;
}

/* As per NIST SP800-90A A.5.1 */
cc3xx_err_t cc3xx_rng_get_random_uint(uint32_t bound, uint32_t *uint)
{
    uint32_t value;
    uint32_t attempts = 0;
    cc3xx_err_t err;
    uint32_t mask;

    /* Zero is not a sane bound */
    assert(bound != 0);

    /* There are two cases that we need to handle differently, the one where we
     * have a single bit set, and the one where we have multiple. First check
     * which we have.
     */
    if ((bound & (bound - 1)) == 0) {
        /* If a single bit is set, we can get the mask by subtracting one */
        mask = bound - 1;
    } else {
        /* Else, we shift the all-one word right until it matches the offset of
         * the leading one-bit in the bound.
         */
        mask = UINT32_MAX >> __builtin_clz(bound);
    }

    do {
        /* Only pull as much entropy as we need, as RNG reseeding is slow */
        err = cc3xx_rng_get_random((uint8_t *)&value,
                                   (32 - __builtin_clz(bound) + 7) / 8);
        if (err != CC3XX_ERR_SUCCESS) {
            return err;
        }

        value &= mask;

        attempts += 1;
        if (attempts >= CC3XX_CONFIG_RNG_MAX_ATTEMPTS) {
            return CC3XX_ERR_RNG_TOO_MANY_ATTEMPTS;
        }
    } while (value >= bound);

    *uint = value;

    return CC3XX_ERR_SUCCESS;
}
#else
cc3xx_err_t cc3xx_rng_get_random(uint8_t* buf, size_t length)
{
    return rng_get_random(buf, length);
}
#endif /* !CC3XX_CONFIG_RNG_EXTERNAL_TRNG */
#endif /* CC3XX_CONFIG_RNG_ENABLE */
