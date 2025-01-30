/*
 * Copyright (c) 2019-2023, Arm Limited. All rights reserved.
 * Copyright (c) 2020, Cypress Semiconductor Corporation. All rights reserved.
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
#define FF_TEST_UART_IRQ         (UARTTX2_IRQn)
#define FF_TEST_UART_IRQ_Handler UARTTX2_Handler

extern struct platform_data_t tfm_peripheral_std_uart;
extern struct platform_data_t tfm_peripheral_timer0;

#define TFM_PERIPHERAL_STD_UART  (&tfm_peripheral_std_uart)
#define TFM_PERIPHERAL_TIMER0    (&tfm_peripheral_timer0)

#ifdef CORSTONE310_FVP
#define TFM_DMA0_CH0_IRQ         (DMA_CHANNEL_0_IRQn)
#define TFM_DMA0_CH1_IRQ         (DMA_CHANNEL_1_IRQn)

extern struct platform_data_t tfm_peripheral_dma0_ch0;
extern struct platform_data_t tfm_peripheral_dma0_ch1;

#define TFM_PERIPHERAL_DMA0_CH0  (&tfm_peripheral_dma0_ch0)
#define TFM_PERIPHERAL_DMA0_CH1  (&tfm_peripheral_dma0_ch1)
#endif

#ifdef PSA_API_TEST_IPC
extern struct platform_data_t tfm_peripheral_FF_TEST_UART_REGION;
extern struct platform_data_t tfm_peripheral_FF_TEST_WATCHDOG_REGION;
extern struct platform_data_t tfm_peripheral_FF_TEST_NVMEM_REGION;
extern struct platform_data_t tfm_peripheral_FF_TEST_SERVER_PARTITION_MMIO;
extern struct platform_data_t tfm_peripheral_FF_TEST_DRIVER_PARTITION_MMIO;
#define FF_TEST_UART_REGION           (&tfm_peripheral_FF_TEST_UART_REGION)
#define FF_TEST_WATCHDOG_REGION       (&tfm_peripheral_FF_TEST_WATCHDOG_REGION)
#define FF_TEST_NVMEM_REGION          (&tfm_peripheral_FF_TEST_NVMEM_REGION)
#define FF_TEST_SERVER_PARTITION_MMIO (&tfm_peripheral_FF_TEST_SERVER_PARTITION_MMIO)
#define FF_TEST_DRIVER_PARTITION_MMIO (&tfm_peripheral_FF_TEST_DRIVER_PARTITION_MMIO)
#endif /* PSA_API_TEST_IPC */

#ifdef __cplusplus
}
#endif

#endif /* __TFM_PERIPHERALS_DEF_H__ */
