/*
 * Copyright (c) 2021-2022, Arm Limited. All rights reserved.
 * Copyright (c) 2022 Cypress Semiconductor Corporation (an Infineon
 * company) or an affiliate of Cypress Semiconductor Corporation. All rights
 * reserved.
 * Copyright (c) 2023-2024, Texas Instruments Incorporated. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "cmsis.h"
#include "spm.h"
#include "tfm_hal_interrupt.h"
#include "tfm_peripherals_def.h"
#include "interrupt.h"
#include "load/interrupt_defs.h"
#include "platform_irq.h"

/* Globals to store interrupt info */
static struct irq_t ff_crypto_result_irq;
static struct irq_t ff_pka_irq;
static struct irq_t ff_trng_irq;

/*
 * Quantized default IRQ priority, the value is:
 * (Number of configurable priority) / 4: (1UL << __NVIC_PRIO_BITS) / 4
 */
#define DEFAULT_IRQ_PRIORITY (1UL << (__NVIC_PRIO_BITS - 2))

enum tfm_hal_status_t crypto_result_avail_irq_irqn_init(void *p_pt, const struct irq_load_info_t *p_ildi)
{
    /* Store interrupt info */
    ff_crypto_result_irq.p_ildi = p_ildi;
    ff_crypto_result_irq.p_pt   = p_pt;

    NVIC_SetPriority(CRYPTO_RESULT_AVAIL_IRQ_IRQn, DEFAULT_IRQ_PRIORITY);
    /* Set interrupt as Secure */
    NVIC_ClearTargetState(CRYPTO_RESULT_AVAIL_IRQ_IRQn);
    NVIC_DisableIRQ(CRYPTO_RESULT_AVAIL_IRQ_IRQn);

    return TFM_HAL_SUCCESS;
}

enum tfm_hal_status_t pka_irq_irqn_init(void *p_pt, const struct irq_load_info_t *p_ildi)
{
    /* Store interrupt info */
    ff_pka_irq.p_ildi = p_ildi;
    ff_pka_irq.p_pt   = p_pt;

    NVIC_SetPriority(PKA_IRQ_IRQn, DEFAULT_IRQ_PRIORITY);
    /* Set interrupt as Secure */
    NVIC_ClearTargetState(PKA_IRQ_IRQn);
    NVIC_DisableIRQ(PKA_IRQ_IRQn);

    return TFM_HAL_SUCCESS;
}

enum tfm_hal_status_t trng_irq_irqn_init(void *p_pt, const struct irq_load_info_t *p_ildi)
{
    /* Store interrupt info */
    ff_trng_irq.p_ildi = p_ildi;
    ff_trng_irq.p_pt   = p_pt;

    NVIC_SetPriority(TRNG_IRQ_IRQn, DEFAULT_IRQ_PRIORITY);
    /* Set interrupt as Secure */
    NVIC_ClearTargetState(TRNG_IRQ_IRQn);
    NVIC_DisableIRQ(TRNG_IRQ_IRQn);

    return TFM_HAL_SUCCESS;
}

void INT_CRYPTO_RESULT_AVAIL_IRQ_Handler(void)
{
    spm_handle_interrupt(ff_crypto_result_irq.p_pt, ff_crypto_result_irq.p_ildi);
}

void INT_PKA_IRQ_Handler(void)
{
    spm_handle_interrupt(ff_pka_irq.p_pt, ff_pka_irq.p_ildi);
}

void INT_TRNG_IRQ_Handler(void)
{
    spm_handle_interrupt(ff_trng_irq.p_pt, ff_trng_irq.p_ildi);
}