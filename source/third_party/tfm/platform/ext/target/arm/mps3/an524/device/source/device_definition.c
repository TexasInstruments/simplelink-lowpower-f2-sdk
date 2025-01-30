/*
 * Copyright (c) 2019-2023 Arm Limited. All rights reserved.
 *
 * Licensed under the Apache License Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing software
 * distributed under the License is distributed on an "AS IS" BASIS
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * \file device_definition.c
 * \brief This file defines exports the structures based on the peripheral
 * definitions from device_cfg.h.
 * This file is meant to be used as a helper for baremetal
 * applications and/or as an example of how to configure the generic
 * driver structures.
 */

#include "device_definition.h"
#include "platform_base_address.h"
#include "tfm_plat_defs.h"

/* UART CMSDK driver structures */
#ifdef UART0_CMSDK_S
static const struct uart_cmsdk_dev_cfg_t UART0_CMSDK_DEV_CFG_S = {
    .base = UART0_BASE_S,
    .default_baudrate = DEFAULT_UART_BAUDRATE
};
static struct uart_cmsdk_dev_data_t UART0_CMSDK_DEV_DATA_S = {
    .state = 0,
    .system_clk = 0,
    .baudrate = 0
};
struct uart_cmsdk_dev_t ARM_UART0_DEV_S = {
    &(UART0_CMSDK_DEV_CFG_S),
    &(UART0_CMSDK_DEV_DATA_S)
};
#endif
#ifdef UART0_CMSDK_NS
static const struct uart_cmsdk_dev_cfg_t UART0_CMSDK_DEV_CFG_NS = {
    .base = UART0_BASE_NS,
    .default_baudrate = DEFAULT_UART_BAUDRATE
};
static struct uart_cmsdk_dev_data_t UART0_CMSDK_DEV_DATA_NS = {
    .state = 0,
    .system_clk = 0,
    .baudrate = 0
};
struct uart_cmsdk_dev_t ARM_UART0_DEV_NS = {
    &(UART0_CMSDK_DEV_CFG_NS),
    &(UART0_CMSDK_DEV_DATA_NS)
};
#endif

#ifdef UART1_CMSDK_S
static const struct uart_cmsdk_dev_cfg_t UART1_CMSDK_DEV_CFG_S = {
    .base = UART1_BASE_S,
    .default_baudrate = DEFAULT_UART_BAUDRATE
};
static struct uart_cmsdk_dev_data_t UART1_CMSDK_DEV_DATA_S = {
    .state = 0,
    .system_clk = 0,
    .baudrate = 0
};
struct uart_cmsdk_dev_t ARM_UART1_DEV_S = {
    &(UART1_CMSDK_DEV_CFG_S),
    &(UART1_CMSDK_DEV_DATA_S)
};
#endif
#ifdef UART1_CMSDK_NS
static const struct uart_cmsdk_dev_cfg_t UART1_CMSDK_DEV_CFG_NS = {
    .base = UART1_BASE_NS,
    .default_baudrate = DEFAULT_UART_BAUDRATE
};
static struct uart_cmsdk_dev_data_t UART1_CMSDK_DEV_DATA_NS = {
    .state = 0,
    .system_clk = 0,
    .baudrate = 0
};
struct uart_cmsdk_dev_t ARM_UART1_DEV_NS = {
    &(UART1_CMSDK_DEV_CFG_NS),
    &(UART1_CMSDK_DEV_DATA_NS)
};
#endif

#ifdef UART2_CMSDK_S
static const struct uart_cmsdk_dev_cfg_t UART2_CMSDK_DEV_CFG_S = {
    .base = UART2_BASE_S,
    .default_baudrate = DEFAULT_UART_BAUDRATE
};
static struct uart_cmsdk_dev_data_t UART2_CMSDK_DEV_DATA_S = {
    .state = 0,
    .system_clk = 0,
    .baudrate = 0
};
struct uart_cmsdk_dev_t ARM_UART2_DEV_S = {
    &(UART2_CMSDK_DEV_CFG_S),
    &(UART2_CMSDK_DEV_DATA_S)
};
#endif
#ifdef UART2_CMSDK_NS
static const struct uart_cmsdk_dev_cfg_t UART2_CMSDK_DEV_CFG_NS = {
    .base = UART2_BASE_NS,
    .default_baudrate = DEFAULT_UART_BAUDRATE
};
static struct uart_cmsdk_dev_data_t UART2_CMSDK_DEV_DATA_NS = {
    .state = 0,
    .system_clk = 0,
    .baudrate = 0
};
struct uart_cmsdk_dev_t ARM_UART2_DEV_NS = {
    &(UART2_CMSDK_DEV_CFG_NS),
    &(UART2_CMSDK_DEV_DATA_NS)
};
#endif

#ifdef UART3_CMSDK_S
static const struct uart_cmsdk_dev_cfg_t UART3_CMSDK_DEV_CFG_S = {
    .base = UART3_BASE_S,
    .default_baudrate = DEFAULT_UART_BAUDRATE
};
static struct uart_cmsdk_dev_data_t UART3_CMSDK_DEV_DATA_S = {
    .state = 0,
    .system_clk = 0,
    .baudrate = 0
};
struct uart_cmsdk_dev_t ARM_UART3_DEV_S = {
    &(UART3_CMSDK_DEV_CFG_S),
    &(UART3_CMSDK_DEV_DATA_S)
};
#endif
#ifdef UART3_CMSDK_NS
static const struct uart_cmsdk_dev_cfg_t UART3_CMSDK_DEV_CFG_NS = {
    .base = UART3_BASE_NS,
    .default_baudrate = DEFAULT_UART_BAUDRATE
};
static struct uart_cmsdk_dev_data_t UART3_CMSDK_DEV_DATA_NS = {
    .state = 0,
    .system_clk = 0,
    .baudrate = 0
};
struct uart_cmsdk_dev_t ARM_UART3_DEV_NS = {
    &(UART3_CMSDK_DEV_CFG_NS),
    &(UART3_CMSDK_DEV_DATA_NS)
};
#endif

#ifdef UART4_CMSDK_S
static const struct uart_cmsdk_dev_cfg_t UART4_CMSDK_DEV_CFG_S = {
    .base = UART4_BASE_S,
    .default_baudrate = DEFAULT_UART_BAUDRATE
};
static struct uart_cmsdk_dev_data_t UART4_CMSDK_DEV_DATA_S = {
    .state = 0,
    .system_clk = 0,
    .baudrate = 0
};
struct uart_cmsdk_dev_t ARM_UART4_DEV_S = {
    &(UART4_CMSDK_DEV_CFG_S),
    &(UART4_CMSDK_DEV_DATA_S)
};
#endif
#ifdef UART4_CMSDK_NS
static const struct uart_cmsdk_dev_cfg_t UART4_CMSDK_DEV_CFG_NS = {
    .base = UART4_BASE_NS,
    .default_baudrate = DEFAULT_UART_BAUDRATE
};
static struct uart_cmsdk_dev_data_t UART4_CMSDK_DEV_DATA_NS = {
    .state = 0,
    .system_clk = 0,
    .baudrate = 0
};
struct uart_cmsdk_dev_t ARM_UART4_DEV_NS = {
    &(UART4_CMSDK_DEV_CFG_NS),
    &(UART4_CMSDK_DEV_DATA_NS)
};
#endif

#ifdef UART5_CMSDK_S
static const struct uart_cmsdk_dev_cfg_t UART5_CMSDK_DEV_CFG_S = {
    .base = UART5_BASE_S,
    .default_baudrate = DEFAULT_UART_BAUDRATE
};
static struct uart_cmsdk_dev_data_t UART5_CMSDK_DEV_DATA_S = {
    .state = 0,
    .system_clk = 0,
    .baudrate = 0
};
struct uart_cmsdk_dev_t ARM_UART5_DEV_S = {
    &(UART5_CMSDK_DEV_CFG_S),
    &(UART5_CMSDK_DEV_DATA_S)
};
#endif
#ifdef UART5_CMSDK_NS
static const struct uart_cmsdk_dev_cfg_t UART5_CMSDK_DEV_CFG_NS = {
    .base = UART5_BASE_NS,
    .default_baudrate = DEFAULT_UART_BAUDRATE
};
static struct uart_cmsdk_dev_data_t UART5_CMSDK_DEV_DATA_NS = {
    .state = 0,
    .system_clk = 0,
    .baudrate = 0
};
struct uart_cmsdk_dev_t ARM_UART5_DEV_NS = {
    &(UART5_CMSDK_DEV_CFG_NS),
    &(UART5_CMSDK_DEV_DATA_NS)
};
#endif

/* ARM PPC SIE 200 driver structures */
#ifdef AHB_PPC0_S
static struct ppc_sse200_dev_cfg_t AHB_PPC0_DEV_CFG_S = {
    .spctrl_base  = CMSDK_SPCTRL_BASE_S,
    .nspctrl_base = CMSDK_NSPCTRL_BASE_NS
};
static struct ppc_sse200_dev_data_t AHB_PPC0_DEV_DATA_S = {
    .p_ns_ppc  = 0,
    .p_sp_ppc  = 0,
    .p_nsp_ppc = 0,
    .int_bit_mask = 0,
    .state = 0
};
struct ppc_sse200_dev_t AHB_PPC0_DEV_S = {
    &AHB_PPC0_DEV_CFG_S,
    &AHB_PPC0_DEV_DATA_S
};
#endif

#ifdef AHB_PPCEXP0_S
static struct ppc_sse200_dev_cfg_t AHB_PPCEXP0_DEV_CFG_S = {
    .spctrl_base  = CMSDK_SPCTRL_BASE_S,
    .nspctrl_base = CMSDK_NSPCTRL_BASE_NS
};
static struct ppc_sse200_dev_data_t AHB_PPCEXP0_DEV_DATA_S = {
    .p_ns_ppc  = 0,
    .p_sp_ppc  = 0,
    .p_nsp_ppc = 0,
    .int_bit_mask = 0,
    .state = 0
};
struct ppc_sse200_dev_t AHB_PPCEXP0_DEV_S = {
    &AHB_PPCEXP0_DEV_CFG_S,
    &AHB_PPCEXP0_DEV_DATA_S
};
#endif

#ifdef APB_PPC0_S
static struct ppc_sse200_dev_cfg_t APB_PPC0_DEV_CFG_S = {
    .spctrl_base  = CMSDK_SPCTRL_BASE_S,
    .nspctrl_base = CMSDK_NSPCTRL_BASE_NS
};
static struct ppc_sse200_dev_data_t APB_PPC0_DEV_DATA_S = {
    .p_ns_ppc  = 0,
    .p_sp_ppc  = 0,
    .p_nsp_ppc = 0,
    .int_bit_mask = 0,
    .state = 0
};
struct ppc_sse200_dev_t APB_PPC0_DEV_S = {
    &APB_PPC0_DEV_CFG_S,
    &APB_PPC0_DEV_DATA_S
};
#endif

#ifdef APB_PPC1_S
static struct ppc_sse200_dev_cfg_t APB_PPC1_DEV_CFG_S = {
    .spctrl_base  = CMSDK_SPCTRL_BASE_S,
    .nspctrl_base = CMSDK_NSPCTRL_BASE_NS
};
static struct ppc_sse200_dev_data_t APB_PPC1_DEV_DATA_S = {
    .p_ns_ppc  = 0,
    .p_sp_ppc  = 0,
    .p_nsp_ppc = 0,
    .int_bit_mask = 0,
    .state = 0
};
struct ppc_sse200_dev_t APB_PPC1_DEV_S = {
    &APB_PPC1_DEV_CFG_S,
    &APB_PPC1_DEV_DATA_S
};
#endif

#ifdef APB_PPCEXP0_S
static struct ppc_sse200_dev_cfg_t APB_PPCEXP0_DEV_CFG_S = {
    .spctrl_base  = CMSDK_SPCTRL_BASE_S,
    .nspctrl_base = CMSDK_NSPCTRL_BASE_NS
};
static struct ppc_sse200_dev_data_t APB_PPCEXP0_DEV_DATA_S = {
    .p_ns_ppc  = 0,
    .p_sp_ppc  = 0,
    .p_nsp_ppc = 0,
    .int_bit_mask = 0,
    .state = 0
};
struct ppc_sse200_dev_t APB_PPCEXP0_DEV_S = {
    &APB_PPCEXP0_DEV_CFG_S,
    &APB_PPCEXP0_DEV_DATA_S
};
#endif

#ifdef APB_PPCEXP1_S
static struct ppc_sse200_dev_cfg_t APB_PPCEXP1_DEV_CFG = {
    .spctrl_base  = CMSDK_SPCTRL_BASE_S,
    .nspctrl_base = CMSDK_NSPCTRL_BASE_NS
};
static struct ppc_sse200_dev_data_t APB_PPCEXP1_DEV_DATA_S = {
    .p_ns_ppc  = 0,
    .p_sp_ppc  = 0,
    .p_nsp_ppc = 0,
    .int_bit_mask = 0,
    .state = 0
};
struct ppc_sse200_dev_t APB_PPCEXP1_DEV_S = {
    &APB_PPCEXP1_DEV_CFG,
    &APB_PPCEXP1_DEV_DATA_S
};
#endif

#ifdef APB_PPCEXP2_S
static struct ppc_sse200_dev_cfg_t APB_PPCEXP2_DEV_CFG = {
    .spctrl_base  = CMSDK_SPCTRL_BASE_S,
    .nspctrl_base = CMSDK_NSPCTRL_BASE_NS
};
static struct ppc_sse200_dev_data_t APB_PPCEXP2_DEV_DATA_S = {
    .p_ns_ppc  = 0,
    .p_sp_ppc  = 0,
    .p_nsp_ppc = 0,
    .int_bit_mask = 0,
    .state = 0
};
struct ppc_sse200_dev_t APB_PPCEXP2_DEV_S = {
    &APB_PPCEXP2_DEV_CFG,
    &APB_PPCEXP2_DEV_DATA_S
};
#endif

/* CMSDK Timer driver structures */
#ifdef CMSDK_TIMER0_S
static const struct timer_cmsdk_dev_cfg_t CMSDK_TIMER0_DEV_CFG_S
#ifdef TFM_PARTITION_SLIH_TEST
    TFM_LINK_SET_RO_IN_PARTITION_SECTION("TFM_SP_SLIH_TEST", "APP-ROT")
#endif
  = {
        .base = CMSDK_TIMER0_BASE_S
    };
static struct timer_cmsdk_dev_data_t CMSDK_TIMER0_DEV_DATA_S
#ifdef TFM_PARTITION_SLIH_TEST
    TFM_LINK_SET_RW_IN_PARTITION_SECTION("TFM_SP_SLIH_TEST", "APP-ROT")
#endif
  = {
        .is_initialized = 0
    };
struct timer_cmsdk_dev_t CMSDK_TIMER0_DEV_S
#ifdef TFM_PARTITION_SLIH_TEST
    TFM_LINK_SET_RW_IN_PARTITION_SECTION("TFM_SP_SLIH_TEST", "APP-ROT")
#endif
  = {
        &(CMSDK_TIMER0_DEV_CFG_S),
        &(CMSDK_TIMER0_DEV_DATA_S)
    };
#endif
#ifdef CMSDK_TIMER0_NS
static const struct timer_cmsdk_dev_cfg_t CMSDK_TIMER0_DEV_CFG_NS = {
    .base = CMSDK_TIMER0_BASE_NS
};
static struct timer_cmsdk_dev_data_t CMSDK_TIMER0_DEV_DATA_NS = {
    .is_initialized = 0
};
struct timer_cmsdk_dev_t CMSDK_TIMER0_DEV_NS = {
    &(CMSDK_TIMER0_DEV_CFG_NS),
    &(CMSDK_TIMER0_DEV_DATA_NS)
};
#endif

#ifdef CMSDK_TIMER1_S
static const struct timer_cmsdk_dev_cfg_t CMSDK_TIMER1_DEV_CFG_S = {
    .base = CMSDK_TIMER1_BASE_S
};
static struct timer_cmsdk_dev_data_t CMSDK_TIMER1_DEV_DATA_S = {
    .is_initialized = 0
};
struct timer_cmsdk_dev_t CMSDK_TIMER1_DEV_S = {
    &(CMSDK_TIMER1_DEV_CFG_S),
    &(CMSDK_TIMER1_DEV_DATA_S)
};
#endif
#ifdef CMSDK_TIMER1_NS
static const struct timer_cmsdk_dev_cfg_t CMSDK_TIMER1_DEV_CFG_NS = {
    .base = CMSDK_TIMER1_BASE_NS
};
static struct timer_cmsdk_dev_data_t CMSDK_TIMER1_DEV_DATA_NS = {
    .is_initialized = 0
};
struct timer_cmsdk_dev_t CMSDK_TIMER1_DEV_NS = {
    &(CMSDK_TIMER1_DEV_CFG_NS),
    &(CMSDK_TIMER1_DEV_DATA_NS)
};
#endif

/* ARM MPC SSE 200 driver structures */
#ifdef MPC_ISRAM0_S
static const struct mpc_sie_memory_range_t MPC_ISRAM0_RANGE_S = {
    .base         = MPC_ISRAM0_RANGE_BASE_S,
    .limit        = MPC_ISRAM0_RANGE_LIMIT_S,
    .range_offset = 0,
    .attr         = MPC_SIE_SEC_ATTR_SECURE
};

static const struct mpc_sie_memory_range_t MPC_ISRAM0_RANGE_NS = {
    .base         = MPC_ISRAM0_RANGE_BASE_NS,
    .limit        = MPC_ISRAM0_RANGE_LIMIT_NS,
    .range_offset = 0,
    .attr         = MPC_SIE_SEC_ATTR_NONSECURE
};

#define MPC_ISRAM0_RANGE_LIST_LEN  2u
static const struct mpc_sie_memory_range_t*
    MPC_ISRAM0_RANGE_LIST[MPC_ISRAM0_RANGE_LIST_LEN] = {
        &MPC_ISRAM0_RANGE_S,
        &MPC_ISRAM0_RANGE_NS
};

static struct mpc_sie_dev_cfg_t MPC_ISRAM0_DEV_CFG_S = {
    .base = MPC_ISRAM0_BASE_S,
    .range_list = MPC_ISRAM0_RANGE_LIST,
    .nbr_of_ranges = MPC_ISRAM0_RANGE_LIST_LEN};
static struct mpc_sie_dev_data_t MPC_ISRAM0_DEV_DATA_S = {
    .is_initialized = false};

struct mpc_sie_dev_t MPC_ISRAM0_DEV_S = {
    &(MPC_ISRAM0_DEV_CFG_S),
    &(MPC_ISRAM0_DEV_DATA_S)
};
#endif

#ifdef MPC_ISRAM1_S
static const struct mpc_sie_memory_range_t MPC_ISRAM1_RANGE_S = {
    .base         = MPC_ISRAM1_RANGE_BASE_S,
    .limit        = MPC_ISRAM1_RANGE_LIMIT_S,
    .range_offset = 0,
    .attr         = MPC_SIE_SEC_ATTR_SECURE
};

static const struct mpc_sie_memory_range_t MPC_ISRAM1_RANGE_NS = {
    .base         = MPC_ISRAM1_RANGE_BASE_NS,
    .limit        = MPC_ISRAM1_RANGE_LIMIT_NS,
    .range_offset = 0,
    .attr         = MPC_SIE_SEC_ATTR_NONSECURE
};

#define MPC_ISRAM1_RANGE_LIST_LEN  2u
static const struct mpc_sie_memory_range_t*
    MPC_ISRAM1_RANGE_LIST[MPC_ISRAM1_RANGE_LIST_LEN] = {
        &MPC_ISRAM1_RANGE_S,
        &MPC_ISRAM1_RANGE_NS
};

static struct mpc_sie_dev_cfg_t MPC_ISRAM1_DEV_CFG_S = {
    .base = MPC_ISRAM1_BASE_S,
    .range_list = MPC_ISRAM1_RANGE_LIST,
    .nbr_of_ranges = MPC_ISRAM1_RANGE_LIST_LEN};
static struct mpc_sie_dev_data_t MPC_ISRAM1_DEV_DATA_S = {
    .is_initialized = false};

struct mpc_sie_dev_t MPC_ISRAM1_DEV_S = {
    &(MPC_ISRAM1_DEV_CFG_S),
    &(MPC_ISRAM1_DEV_DATA_S)
};
#endif

#ifdef MPC_ISRAM2_S
static const struct mpc_sie_memory_range_t MPC_ISRAM2_RANGE_S = {
    .base         = MPC_ISRAM2_RANGE_BASE_S,
    .limit        = MPC_ISRAM2_RANGE_LIMIT_S,
    .range_offset = 0,
    .attr         = MPC_SIE_SEC_ATTR_SECURE
};

static const struct mpc_sie_memory_range_t MPC_ISRAM2_RANGE_NS = {
    .base         = MPC_ISRAM2_RANGE_BASE_NS,
    .limit        = MPC_ISRAM2_RANGE_LIMIT_NS,
    .range_offset = 0,
    .attr         = MPC_SIE_SEC_ATTR_NONSECURE
};

#define MPC_ISRAM2_RANGE_LIST_LEN  2u
static const struct mpc_sie_memory_range_t*
    MPC_ISRAM2_RANGE_LIST[MPC_ISRAM2_RANGE_LIST_LEN] = {
        &MPC_ISRAM2_RANGE_S,
        &MPC_ISRAM2_RANGE_NS
};

static struct mpc_sie_dev_cfg_t MPC_ISRAM2_DEV_CFG_S = {
    .base = MPC_ISRAM2_BASE_S,
    .range_list = MPC_ISRAM2_RANGE_LIST,
    .nbr_of_ranges = MPC_ISRAM2_RANGE_LIST_LEN};
static struct mpc_sie_dev_data_t MPC_ISRAM2_DEV_DATA_S = {
    .is_initialized = false};

struct mpc_sie_dev_t MPC_ISRAM2_DEV_S = {
    &(MPC_ISRAM2_DEV_CFG_S),
    &(MPC_ISRAM2_DEV_DATA_S)
};
#endif

#ifdef MPC_ISRAM3_S
static const struct mpc_sie_memory_range_t MPC_ISRAM3_RANGE_S = {
    .base         = MPC_ISRAM3_RANGE_BASE_S,
    .limit        = MPC_ISRAM3_RANGE_LIMIT_S,
    .range_offset = 0,
    .attr         = MPC_SIE_SEC_ATTR_SECURE
};

static const struct mpc_sie_memory_range_t MPC_ISRAM3_RANGE_NS = {
    .base         = MPC_ISRAM3_RANGE_BASE_NS,
    .limit        = MPC_ISRAM3_RANGE_LIMIT_NS,
    .range_offset = 0,
    .attr         = MPC_SIE_SEC_ATTR_NONSECURE
};

#define MPC_ISRAM3_RANGE_LIST_LEN  2u
static const struct mpc_sie_memory_range_t*
    MPC_ISRAM3_RANGE_LIST[MPC_ISRAM3_RANGE_LIST_LEN] = {
        &MPC_ISRAM3_RANGE_S,
        &MPC_ISRAM3_RANGE_NS
};

static struct mpc_sie_dev_cfg_t MPC_ISRAM3_DEV_CFG_S = {
    .base = MPC_ISRAM3_BASE_S,
    .range_list = MPC_ISRAM3_RANGE_LIST,
    .nbr_of_ranges = MPC_ISRAM3_RANGE_LIST_LEN};
static struct mpc_sie_dev_data_t MPC_ISRAM3_DEV_DATA_S = {
    .is_initialized = false};

struct mpc_sie_dev_t MPC_ISRAM3_DEV_S = {
    &(MPC_ISRAM3_DEV_CFG_S),
    &(MPC_ISRAM3_DEV_DATA_S)
};
#endif

#ifdef MPC_DDR4_S
static const struct mpc_sie_memory_range_t MPC_DDR4_RANGE_S = {
    .base         = MPC_DDR4_RANGE_BASE_S,
    .limit        = MPC_DDR4_RANGE_LIMIT_S,
    .range_offset = 0,
    .attr         = MPC_SIE_SEC_ATTR_SECURE
};

static const struct mpc_sie_memory_range_t MPC_DDR4_RANGE_NS = {
    .base         = MPC_DDR4_RANGE_BASE_NS,
    .limit        = MPC_DDR4_RANGE_LIMIT_NS,
    .range_offset = 0,
    .attr         = MPC_SIE_SEC_ATTR_NONSECURE
};

#define MPC_DDR4_RANGE_LIST_LEN  2u
static const struct mpc_sie_memory_range_t*
    MPC_DDR4_RANGE_LIST[MPC_DDR4_RANGE_LIST_LEN] = {
        &MPC_DDR4_RANGE_S,
        &MPC_DDR4_RANGE_NS
};

static struct mpc_sie_dev_cfg_t MPC_DDR4_DEV_CFG_S = {
    .base = MPC_DDR4_BASE_S,
    .range_list = MPC_DDR4_RANGE_LIST,
    .nbr_of_ranges = MPC_DDR4_RANGE_LIST_LEN};
static struct mpc_sie_dev_data_t MPC_DDR4_DEV_DATA_S = {
    .is_initialized = false};

struct mpc_sie_dev_t MPC_DDR4_DEV_S = {
    &(MPC_DDR4_DEV_CFG_S),
    &(MPC_DDR4_DEV_DATA_S)
};
#endif

#ifdef MPC_QSPI_S
static const struct mpc_sie_memory_range_t MPC_QSPI_RANGE_S = {
    .base         = MPC_QSPI_RANGE_BASE_S,
    .limit        = MPC_QSPI_RANGE_LIMIT_S,
    .range_offset = 0,
    .attr         = MPC_SIE_SEC_ATTR_SECURE
};

static const struct mpc_sie_memory_range_t MPC_QSPI_RANGE_NS = {
    .base         = MPC_QSPI_RANGE_BASE_NS,
    .limit        = MPC_QSPI_RANGE_LIMIT_NS,
    .range_offset = 0,
    .attr         = MPC_SIE_SEC_ATTR_NONSECURE
};

#define MPC_QSPI_RANGE_LIST_LEN  2u
static const struct mpc_sie_memory_range_t*
    MPC_QSPI_RANGE_LIST[MPC_QSPI_RANGE_LIST_LEN] = {
        &MPC_QSPI_RANGE_S,
        &MPC_QSPI_RANGE_NS
};

static struct mpc_sie_dev_cfg_t MPC_QSPI_DEV_CFG_S = {
    .base = MPC_QSPI_BASE_S,
    .range_list = MPC_QSPI_RANGE_LIST,
    .nbr_of_ranges = MPC_QSPI_RANGE_LIST_LEN};
static struct mpc_sie_dev_data_t MPC_QSPI_DEV_DATA_S = {
    .is_initialized = false};

struct mpc_sie_dev_t MPC_QSPI_DEV_S = {
    &(MPC_QSPI_DEV_CFG_S),
    &(MPC_QSPI_DEV_DATA_S)
};
#endif

#ifdef MPS3_IO_S
static struct arm_mps3_io_dev_cfg_t MPS3_IO_DEV_CFG_S = {
    .base = MPS3_IO_FPGAIO_BASE_S
};
struct arm_mps3_io_dev_t MPS3_IO_DEV_S = {
    .cfg = &(MPS3_IO_DEV_CFG_S)
};
#endif

#ifdef MPS3_IO_NS
static struct arm_mps3_io_dev_cfg_t MPS3_IO_DEV_CFG_NS = {
    .base = MPS3_IO_FPGAIO_BASE_NS
};
struct arm_mps3_io_dev_t MPS3_IO_DEV_NS = {
    .cfg = &(MPS3_IO_DEV_CFG_NS)
};
#endif
