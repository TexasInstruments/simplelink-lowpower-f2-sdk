/*
 * Copyright (c) 2021-2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "tfm_peripherals_def.h"
#include "array.h"
#include "cmsis.h"

#ifdef RSS_DEBUG_UART
struct platform_data_t tfm_peripheral_std_uart = {
        RSS_DEBUG_UART0_BASE_NS,
        RSS_DEBUG_UART0_BASE_NS + 0xFFF,
        PPC_SP_DO_NOT_CONFIGURE,
        -1
};
#endif /* RSS_DEBUG_UART */

#ifdef RSS_USE_HOST_UART
struct platform_data_t tfm_peripheral_std_uart = {
        HOST_UART0_BASE_NS,
        HOST_UART0_BASE_NS + 0xFFF,
        PPC_SP_DO_NOT_CONFIGURE,
        -1
};
#endif /* RSS_USE_HOST_UART */

struct platform_data_t tfm_peripheral_timer0 = {
        SYSTIMER0_ARMV8_M_BASE_S,
        SYSTIMER0_ARMV8_M_BASE_S + 0xFFF,
        PPC_SP_PERIPH0,
        SYSTEM_TIMER0_PERIPH_PPC0_POS_MASK
};

struct platform_data_t tfm_peripheral_dma0_ch0 = {
        DMA_350_BASE_S + 0x1000UL, /* The DMA0 CH0 starts at 0x1000 offset */
        DMA_350_BASE_S + 0x1FFFUL, /* and its size is 0x1000.              */
        PPC_SP_DO_NOT_CONFIGURE,
        -1
};

/* Allowed named MMIO of this platform.
 * Platform RSS only has named MMIO.
 */
static const uintptr_t partition_named_mmio_list[] = {
    (uintptr_t)TFM_PERIPHERAL_STD_UART,
    (uintptr_t)TFM_PERIPHERAL_TIMER0,
    (uintptr_t)TFM_PERIPHERAL_DMA0_CH0,
};

void get_partition_named_mmio_list(const uintptr_t **list, size_t *length)
{
    *list = partition_named_mmio_list;
    *length = ARRAY_SIZE(partition_named_mmio_list);
}
