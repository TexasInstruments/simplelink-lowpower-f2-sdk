/*
 * Copyright (c) 2019-2023, Arm Limited. All rights reserved.
 * Copyright (c) 2020-2023 Cypress Semiconductor Corporation (an Infineon company)
 * or an affiliate of Cypress Semiconductor Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef __TFM_PERIPHERALS_DEF_H__
#define __TFM_PERIPHERALS_DEF_H__

#include "platform_irq.h"
#include "common_target_cfg.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Quantized default IRQ priority, the value is:
 * (Number of configurable priority) / 4: (1UL << __NVIC_PRIO_BITS) / 4
 */
#define DEFAULT_IRQ_PRIORITY    (1UL << (__NVIC_PRIO_BITS - 2))

#define TFM_TIMER0_IRQ           (TIMER0_IRQn)
#define TFM_DMA0_COMBINED_S_IRQ  (DMA_Combined_S_IRQn)

extern struct platform_data_t tfm_peripheral_std_uart;
extern struct platform_data_t tfm_peripheral_timer0;
extern struct platform_data_t tfm_peripheral_dma0_ch0;

#define TFM_PERIPHERAL_STD_UART  (&tfm_peripheral_std_uart)
#define TFM_PERIPHERAL_TIMER0    (&tfm_peripheral_timer0)
#define TFM_PERIPHERAL_DMA0_CH0  (&tfm_peripheral_dma0_ch0)

/* AP to RSS MHU receiver interrupt */
#define MAILBOX_IRQ CMU_MHU0_Receiver_IRQn

/* Append RSS-specific static MPU regions to the standard ones in
 * tfm_hal_isolation_v8m.c.
 */
#define PLATFORM_STATIC_MPU_REGIONS \
    /* Keep ROM as no-execute, and prevent unpriv from accessing */ \
    { \
        ARM_MPU_RBAR(ROM_BASE_S, \
                     ARM_MPU_SH_NON, \
                     ARM_MPU_READ_ONLY, \
                     ARM_MPU_PRIVILEGED, \
                     ARM_MPU_EXECUTE_NEVER), \
        ARM_MPU_RLAR_PXN(ROM_BASE_S + ROM_SIZE - 1, \
                         ARM_MPU_PRIVILEGE_EXECUTE_NEVER, \
                         0), \
    },

#ifdef __cplusplus
}
#endif

#endif /* __TFM_PERIPHERALS_DEF_H__ */
