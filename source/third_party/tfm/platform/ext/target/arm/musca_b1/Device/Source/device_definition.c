/*
 * Copyright (c) 2017-2021 Arm Limited. All rights reserved.
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

#include "device_cfg.h"
#include "device_definition.h"
#include "platform_base_address.h"
#include "tfm_plat_defs.h"

/* ======= Peripheral configuration structure definitions ======= */
/* MUSCA B1 SCC driver structures */
#ifdef MUSCA_B1_SCC_S
static const struct musca_b1_scc_dev_cfg_t MUSCA_B1_SCC_DEV_CFG_S = {
    .base = MUSCA_B1_SCC_S_BASE};
struct musca_b1_scc_dev_t MUSCA_B1_SCC_DEV_S = {&(MUSCA_B1_SCC_DEV_CFG_S)};
#endif

/* Arm PPC SSE 200 driver structures */
#ifdef AHB_PPC0_S
static struct ppc_sse200_dev_cfg_t AHB_PPC0_DEV_CFG_S = {
    .spctrl_base  = MUSCA_B1_SPCTRL_S_BASE,
    .nspctrl_base = MUSCA_B1_NSPCTRL_NS_BASE };
static struct ppc_sse200_dev_data_t AHB_PPC0_DEV_DATA_S = {
    .p_ns_ppc  = 0,
    .p_sp_ppc  = 0,
    .p_nsp_ppc = 0,
    .int_bit_mask = 0,
    .state = 0 };
struct ppc_sse200_dev_t AHB_PPC0_DEV_S = {
    &AHB_PPC0_DEV_CFG_S, &AHB_PPC0_DEV_DATA_S };
#endif

#ifdef AHB_PPCEXP0_S
static struct ppc_sse200_dev_cfg_t AHB_PPCEXP0_DEV_CFG_S = {
    .spctrl_base  = MUSCA_B1_SPCTRL_S_BASE,
    .nspctrl_base = MUSCA_B1_NSPCTRL_NS_BASE };
static struct ppc_sse200_dev_data_t AHB_PPCEXP0_DEV_DATA_S = {
    .p_ns_ppc  = 0,
    .p_sp_ppc  = 0,
    .p_nsp_ppc = 0,
    .int_bit_mask = 0,
    .state = 0 };
struct ppc_sse200_dev_t AHB_PPCEXP0_DEV_S = {
    &AHB_PPCEXP0_DEV_CFG_S, &AHB_PPCEXP0_DEV_DATA_S };
#endif

#ifdef AHB_PPCEXP1_S
static struct ppc_sse200_dev_cfg_t AHB_PPCEXP1_DEV_CFG_S = {
    .spctrl_base  = MUSCA_B1_SPCTRL_S_BASE,
    .nspctrl_base = MUSCA_B1_NSPCTRL_NS_BASE };
static struct ppc_sse200_dev_data_t AHB_PPCEXP1_DEV_DATA_S = {
    .p_ns_ppc  = 0,
    .p_sp_ppc  = 0,
    .p_nsp_ppc = 0,
    .int_bit_mask = 0,
    .state = 0 };
struct ppc_sse200_dev_t AHB_PPCEXP1_DEV_S = {
    &AHB_PPCEXP1_DEV_CFG_S, &AHB_PPCEXP1_DEV_DATA_S };
#endif

#ifdef AHB_PPCEXP2_S
static struct ppc_sse200_dev_cfg_t AHB_PPCEXP2_DEV_CFG_S = {
    .spctrl_base  = MUSCA_B1_SPCTRL_S_BASE,
    .nspctrl_base = MUSCA_B1_NSPCTRL_NS_BASE };
static struct ppc_sse200_dev_data_t AHB_PPCEXP2_DEV_DATA_S = {
    .p_ns_ppc  = 0,
    .p_sp_ppc  = 0,
    .p_nsp_ppc = 0,
    .int_bit_mask = 0,
    .state = 0 };
struct ppc_sse200_dev_t AHB_PPCEXP2_DEV_S = {
    &AHB_PPCEXP2_DEV_CFG_S, &AHB_PPCEXP2_DEV_DATA_S };
#endif

#ifdef AHB_PPCEXP3_S
static struct ppc_sse200_dev_cfg_t AHB_PPCEXP3_DEV_CFG_S = {
    .spctrl_base  = MUSCA_B1_SPCTRL_S_BASE,
    .nspctrl_base = MUSCA_B1_NSPCTRL_NS_BASE };
static struct ppc_sse200_dev_data_t AHB_PPCEXP3_DEV_DATA_S = {
    .p_ns_ppc  = 0,
    .p_sp_ppc  = 0,
    .p_nsp_ppc = 0,
    .int_bit_mask = 0,
    .state = 0 };
struct ppc_sse200_dev_t AHB_PPCEXP3_DEV_S = {
    &AHB_PPCEXP3_DEV_CFG_S, &AHB_PPCEXP3_DEV_DATA_S };
#endif

#ifdef APB_PPC0_S
static struct ppc_sse200_dev_cfg_t APB_PPC0_DEV_CFG_S = {
    .spctrl_base  = MUSCA_B1_SPCTRL_S_BASE,
    .nspctrl_base = MUSCA_B1_NSPCTRL_NS_BASE };
static struct ppc_sse200_dev_data_t APB_PPC0_DEV_DATA_S = {
    .p_ns_ppc  = 0,
    .p_sp_ppc  = 0,
    .p_nsp_ppc = 0,
    .int_bit_mask = 0,
    .state = 0 };
struct ppc_sse200_dev_t APB_PPC0_DEV_S = {
    &APB_PPC0_DEV_CFG_S, &APB_PPC0_DEV_DATA_S };
#endif

#ifdef APB_PPC1_S
static struct ppc_sse200_dev_cfg_t APB_PPC1_DEV_CFG_S = {
    .spctrl_base  = MUSCA_B1_SPCTRL_S_BASE,
    .nspctrl_base = MUSCA_B1_NSPCTRL_NS_BASE };
static struct ppc_sse200_dev_data_t APB_PPC1_DEV_DATA_S = {
    .p_ns_ppc  = 0,
    .p_sp_ppc  = 0,
    .p_nsp_ppc = 0,
    .int_bit_mask = 0,
    .state = 0 };
struct ppc_sse200_dev_t APB_PPC1_DEV_S = {
    &APB_PPC1_DEV_CFG_S, &APB_PPC1_DEV_DATA_S};
#endif

#ifdef APB_PPCEXP0_S
static struct ppc_sse200_dev_cfg_t APB_PPCEXP0_DEV_CFG_S = {
    .spctrl_base  = MUSCA_B1_SPCTRL_S_BASE,
    .nspctrl_base = MUSCA_B1_NSPCTRL_NS_BASE };
static struct ppc_sse200_dev_data_t APB_PPCEXP0_DEV_DATA_S = {
    .p_ns_ppc  = 0,
    .p_sp_ppc  = 0,
    .p_nsp_ppc = 0,
    .int_bit_mask = 0,
    .state = 0 };
struct ppc_sse200_dev_t APB_PPCEXP0_DEV_S = {
    &APB_PPCEXP0_DEV_CFG_S, &APB_PPCEXP0_DEV_DATA_S };
#endif

#ifdef APB_PPCEXP1_S
static struct ppc_sse200_dev_cfg_t APB_PPCEXP1_DEV_CFG = {
    .spctrl_base  = MUSCA_B1_SPCTRL_S_BASE,
    .nspctrl_base = MUSCA_B1_NSPCTRL_NS_BASE };
static struct ppc_sse200_dev_data_t APB_PPCEXP1_DEV_DATA_S = {
    .p_ns_ppc  = 0,
    .p_sp_ppc  = 0,
    .p_nsp_ppc = 0,
    .int_bit_mask = 0,
    .state = 0 };
struct ppc_sse200_dev_t APB_PPCEXP1_DEV_S = {
    &APB_PPCEXP1_DEV_CFG, &APB_PPCEXP1_DEV_DATA_S };
#endif

#ifdef APB_PPCEXP2_S
static struct ppc_sse200_dev_cfg_t APB_PPCEXP2_DEV_CFG = {
    .spctrl_base  = MUSCA_B1_SPCTRL_S_BASE,
    .nspctrl_base = MUSCA_B1_NSPCTRL_NS_BASE };
static struct ppc_sse200_dev_data_t APB_PPCEXP2_DEV_DATA_S = {
    .p_ns_ppc  = 0,
    .p_sp_ppc  = 0,
    .p_nsp_ppc = 0,
    .int_bit_mask = 0,
    .state = 0 };
struct ppc_sse200_dev_t APB_PPCEXP2_DEV_S = {
    &APB_PPCEXP2_DEV_CFG, &APB_PPCEXP2_DEV_DATA_S };
#endif

#ifdef APB_PPCEXP3_S
static struct ppc_sse200_dev_cfg_t APB_PPCEXP3_DEV_CFG = {
    .spctrl_base  = MUSCA_B1_SPCTRL_S_BASE,
    .nspctrl_base = MUSCA_B1_NSPCTRL_NS_BASE };
static struct ppc_sse200_dev_data_t APB_PPCEXP3_DEV_DATA_S = {
    .p_ns_ppc  = 0,
    .p_sp_ppc  = 0,
    .p_nsp_ppc = 0,
    .int_bit_mask = 0,
    .state = 0 };
struct ppc_sse200_dev_t APB_PPCEXP3_DEV_S = {
    &APB_PPCEXP3_DEV_CFG, &APB_PPCEXP3_DEV_DATA_S };
#endif

/* Arm MPC SIE 200 driver structures */
#ifdef MPC_ISRAM0_S
static const struct mpc_sie200_dev_cfg_t MPC_ISRAM0_DEV_CFG_S = {
    .base = MUSCA_B1_MPC_SRAM0_S_BASE};
static struct mpc_sie200_dev_data_t MPC_ISRAM0_DEV_DATA_S = {
    .range_list = 0,
    .nbr_of_ranges = 0,
    .state = 0,
    .reserved = 0};
struct mpc_sie200_dev_t MPC_ISRAM0_DEV_S = {
    &(MPC_ISRAM0_DEV_CFG_S),
    &(MPC_ISRAM0_DEV_DATA_S)};
#endif

#ifdef MPC_ISRAM1_S
static const struct mpc_sie200_dev_cfg_t MPC_ISRAM1_DEV_CFG_S = {
    .base = MUSCA_B1_MPC_SRAM1_S_BASE};
static struct mpc_sie200_dev_data_t MPC_ISRAM1_DEV_DATA_S = {
    .range_list = 0,
    .nbr_of_ranges = 0,
    .state = 0,
    .reserved = 0};
struct mpc_sie200_dev_t MPC_ISRAM1_DEV_S = {
    &(MPC_ISRAM1_DEV_CFG_S),
    &(MPC_ISRAM1_DEV_DATA_S)};
#endif

#ifdef MPC_ISRAM2_S
static const struct mpc_sie200_dev_cfg_t MPC_ISRAM2_DEV_CFG_S = {
    .base = MUSCA_B1_MPC_SRAM2_S_BASE};
static struct mpc_sie200_dev_data_t MPC_ISRAM2_DEV_DATA_S = {
    .range_list = 0,
    .nbr_of_ranges = 0,
    .state = 0,
    .reserved = 0};
struct mpc_sie200_dev_t MPC_ISRAM2_DEV_S = {
    &(MPC_ISRAM2_DEV_CFG_S),
    &(MPC_ISRAM2_DEV_DATA_S)};
#endif

#ifdef MPC_ISRAM3_S
static const struct mpc_sie200_dev_cfg_t MPC_ISRAM3_DEV_CFG_S = {
    .base = MUSCA_B1_MPC_SRAM3_S_BASE};
static struct mpc_sie200_dev_data_t MPC_ISRAM3_DEV_DATA_S = {
    .range_list = 0,
    .nbr_of_ranges = 0,
    .state = 0,
    .reserved = 0};
struct mpc_sie200_dev_t MPC_ISRAM3_DEV_S = {
    &(MPC_ISRAM3_DEV_CFG_S),
    &(MPC_ISRAM3_DEV_DATA_S)};
#endif

#ifdef MPC_CODE_SRAM_S
static const struct mpc_sie200_dev_cfg_t MPC_CODE_SRAM_DEV_CFG_S = {
    .base = MUSCA_B1_CODE_SRAM_MPC_S_BASE};
static struct mpc_sie200_dev_data_t MPC_CODE_SRAM_DEV_DATA_S = {
    .range_list = 0,
    .nbr_of_ranges = 0,
    .state = 0,
    .reserved = 0};
struct mpc_sie200_dev_t MPC_CODE_SRAM_DEV_S = {
    &(MPC_CODE_SRAM_DEV_CFG_S),
    &(MPC_CODE_SRAM_DEV_DATA_S)};
#endif

#ifdef MPC_QSPI_S
static const struct mpc_sie200_dev_cfg_t MPC_QSPI_DEV_CFG_S = {
    .base = MUSCA_B1_QSPI_MPC_S_BASE};
static struct mpc_sie200_dev_data_t MPC_QSPI_DEV_DATA_S = {
    .range_list = 0,
    .nbr_of_ranges = 0,
    .state = 0,
    .reserved = 0};
struct mpc_sie200_dev_t MPC_QSPI_DEV_S = {
    &(MPC_QSPI_DEV_CFG_S),
    &(MPC_QSPI_DEV_DATA_S)};
#endif

#ifdef MPC_EFLASH0_S
static const struct mpc_sie200_dev_cfg_t MPC_EFLASH0_DEV_CFG_S = {
    .base = MUSCA_B1_EFLASH0_MPC_S_BASE};
static struct mpc_sie200_dev_data_t MPC_EFLASH0_DEV_DATA_S = {
    .range_list = 0,
    .nbr_of_ranges = 0,
    .state = 0,
    .reserved = 0};
struct mpc_sie200_dev_t MPC_EFLASH0_DEV_S = {
    &(MPC_EFLASH0_DEV_CFG_S),
    &(MPC_EFLASH0_DEV_DATA_S)};
#endif

#ifdef MPC_EFLASH1_S
static const struct mpc_sie200_dev_cfg_t MPC_EFLASH1_DEV_CFG_S = {
    .base = MUSCA_B1_EFLASH1_MPC_S_BASE};
static struct mpc_sie200_dev_data_t MPC_EFLASH1_DEV_DATA_S = {
    .range_list = 0,
    .nbr_of_ranges = 0,
    .state = 0,
    .reserved = 0};
struct mpc_sie200_dev_t MPC_EFLASH1_DEV_S = {
    &(MPC_EFLASH1_DEV_CFG_S),
    &(MPC_EFLASH1_DEV_DATA_S)};
#endif

/** CMSDK GPIO driver structures */
#ifdef GPIO0_CMSDK_S
static const struct gpio_cmsdk_dev_cfg_t GPIO0_CMSDK_DEV_CFG_S = {
    .base = MUSCA_B1_GPIO_S_BASE};
struct gpio_cmsdk_dev_t GPIO0_CMSDK_DEV_S = {&(GPIO0_CMSDK_DEV_CFG_S)};
#endif

/** Arm UART PL011 driver structures */
#ifdef UART0_PL011_S
static const struct uart_pl011_dev_cfg_t UART0_PL011_DEV_CFG_S = {
    .base = MUSCA_B1_UART0_S_BASE,
    .def_baudrate = DEFAULT_UART_BAUDRATE,
    .def_wlen = UART_PL011_WLEN_8,
    .def_parity = UART_PL011_PARITY_DISABLED,
    .def_stopbit = UART_PL011_STOPBIT_1};
static struct uart_pl011_dev_data_t UART0_PL011_DEV_DATA_S = {
    .state = UART_PL011_UNINITIALIZED,
    .uart_clk = 0,
    .baudrate = 0};
struct uart_pl011_dev_t UART0_PL011_DEV_S = {&(UART0_PL011_DEV_CFG_S),
                                             &(UART0_PL011_DEV_DATA_S)};
#endif
#ifdef UART0_PL011_NS
static const struct uart_pl011_dev_cfg_t UART0_PL011_DEV_CFG_NS = {
    .base = MUSCA_B1_UART0_NS_BASE,
    .def_baudrate = DEFAULT_UART_BAUDRATE,
    .def_wlen = UART_PL011_WLEN_8,
    .def_parity = UART_PL011_PARITY_DISABLED,
    .def_stopbit = UART_PL011_STOPBIT_1};
static struct uart_pl011_dev_data_t UART0_PL011_DEV_DATA_NS = {
    .state = UART_PL011_UNINITIALIZED,
    .uart_clk = 0,
    .baudrate = 0};
struct uart_pl011_dev_t UART0_PL011_DEV_NS = {&(UART0_PL011_DEV_CFG_NS),
                                              &(UART0_PL011_DEV_DATA_NS)};
#endif

#ifdef UART1_PL011_S
static const struct uart_pl011_dev_cfg_t UART1_PL011_DEV_CFG_S = {
    .base = MUSCA_B1_UART1_S_BASE,
    .def_baudrate = DEFAULT_UART_BAUDRATE,
    .def_wlen = UART_PL011_WLEN_8,
    .def_parity = UART_PL011_PARITY_DISABLED,
    .def_stopbit = UART_PL011_STOPBIT_1};
static struct uart_pl011_dev_data_t UART1_PL011_DEV_DATA_S = {
    .state = UART_PL011_UNINITIALIZED,
    .uart_clk = 0,
    .baudrate = 0};
struct uart_pl011_dev_t UART1_PL011_DEV_S = {&(UART1_PL011_DEV_CFG_S),
                                             &(UART1_PL011_DEV_DATA_S)};
#endif
#ifdef UART1_PL011_NS
static const struct uart_pl011_dev_cfg_t UART1_PL011_DEV_CFG_NS = {
    .base = MUSCA_B1_UART1_NS_BASE,
    .def_baudrate = DEFAULT_UART_BAUDRATE,
    .def_wlen = UART_PL011_WLEN_8,
    .def_parity = UART_PL011_PARITY_DISABLED,
    .def_stopbit = UART_PL011_STOPBIT_1};
static struct uart_pl011_dev_data_t UART1_PL011_DEV_DATA_NS = {
    .state = UART_PL011_UNINITIALIZED,
    .uart_clk = 0,
    .baudrate = 0};
struct uart_pl011_dev_t UART1_PL011_DEV_NS = {&(UART1_PL011_DEV_CFG_NS),
                                              &(UART1_PL011_DEV_DATA_NS)};
#endif

/** CMSDK Timers driver structures */
#ifdef CMSDK_TIMER0_S
static const struct timer_cmsdk_dev_cfg_t CMSDK_TIMER0_DEV_CFG_S
#ifdef TFM_PARTITION_SLIH_TEST
    TFM_LINK_SET_RO_IN_PARTITION_SECTION("TFM_SP_SLIH_TEST", "APP-ROT")
#endif
    = {.base = MUSCA_B1_CMSDK_TIMER0_S_BASE};
static struct timer_cmsdk_dev_data_t CMSDK_TIMER0_DEV_DATA_S
#ifdef TFM_PARTITION_SLIH_TEST
    TFM_LINK_SET_RW_IN_PARTITION_SECTION("TFM_SP_SLIH_TEST", "APP-ROT")
#endif
    = {.is_initialized = 0};
struct timer_cmsdk_dev_t CMSDK_TIMER0_DEV_S
#ifdef TFM_PARTITION_SLIH_TEST
    TFM_LINK_SET_RW_IN_PARTITION_SECTION("TFM_SP_SLIH_TEST", "APP-ROT")
#endif
    = {&(CMSDK_TIMER0_DEV_CFG_S), &(CMSDK_TIMER0_DEV_DATA_S)};
#endif
#ifdef CMSDK_TIMER0_NS
static const struct timer_cmsdk_dev_cfg_t CMSDK_TIMER0_DEV_CFG_NS = {
    .base = MUSCA_B1_CMSDK_TIMER0_NS_BASE};
static struct timer_cmsdk_dev_data_t CMSDK_TIMER0_DEV_DATA_NS = {
    .is_initialized = 0};
struct timer_cmsdk_dev_t CMSDK_TIMER0_DEV_NS = {&(CMSDK_TIMER0_DEV_CFG_NS),
                                                &(CMSDK_TIMER0_DEV_DATA_NS)};
#endif

#ifdef CMSDK_TIMER1_S
static const struct timer_cmsdk_dev_cfg_t CMSDK_TIMER1_DEV_CFG_S = {
    .base = MUSCA_B1_CMSDK_TIMER1_S_BASE};
static struct timer_cmsdk_dev_data_t CMSDK_TIMER1_DEV_DATA_S = {
    .is_initialized = 0};
struct timer_cmsdk_dev_t CMSDK_TIMER1_DEV_S = {&(CMSDK_TIMER1_DEV_CFG_S),
                                               &(CMSDK_TIMER1_DEV_DATA_S)};
#endif
#ifdef CMSDK_TIMER1_NS
static const struct timer_cmsdk_dev_cfg_t CMSDK_TIMER1_DEV_CFG_NS = {
    .base = MUSCA_B1_CMSDK_TIMER1_NS_BASE};
static struct timer_cmsdk_dev_data_t CMSDK_TIMER1_DEV_DATA_NS = {
    .is_initialized = 0};
struct timer_cmsdk_dev_t CMSDK_TIMER1_DEV_NS = {&(CMSDK_TIMER1_DEV_CFG_NS),
                                                &(CMSDK_TIMER1_DEV_DATA_NS)};
#endif

/* QSPI IP6514E driver structures */
#ifdef QSPI_IP6514E_S
static const struct qspi_ip6514e_dev_cfg_t QSPI_DEV_CFG_S = {
    .base = MUSCA_B1_QSPI_REG_S_BASE,
    .addr_mask = (1U << 23) - 1, /* 8MiB minus 1 byte */
};
struct qspi_ip6514e_dev_t QSPI_DEV_S = {
    &QSPI_DEV_CFG_S
};
#endif

#ifdef QSPI_IP6514E_NS
static const struct qspi_ip6514e_dev_cfg_t QSPI_DEV_CFG_NS = {
    .base = MUSCA_B1_QSPI_REG_NS_BASE,
    .addr_mask = (1U << 23) - 1, /* 8MiB minus 1 byte */
};
struct qspi_ip6514e_dev_t QSPI_DEV_NS = {
    &QSPI_DEV_CFG_NS
};
#endif

/* SE MHU */
#ifdef SE_MHU_SENDER_S
struct mhu_v2_x_dev_t SE_MHU_SENDER_DEV_S = {
        .base = MUSCA_B1_SE_MHU_SND_S_BASE,
        .frame = MHU_V2_X_SENDER_FRAME
};
#endif
#ifdef SE_MHU_SENDER_NS
struct mhu_v2_x_dev_t SE_MHU_SENDER_DEV_NS = {
        .base = MUSCA_B1_SE_MHU_SND_NS_BASE,
        .frame = MHU_V2_X_SENDER_FRAME
};
#endif

#ifdef SE_MHU_RECEIVER_S
struct mhu_v2_x_dev_t SE_MHU_RECEIVER_DEV_S = {
        .base = MUSCA_B1_SE_MHU_RCV_S_BASE,
        .frame = MHU_V2_X_RECEIVER_FRAME
};
#endif
#ifdef SE_MHU_RECEIVER_NS
struct mhu_v2_x_dev_t SE_MHU_RECEIVER_DEV_NS = {
        .base = MUSCA_B1_SE_MHU_RCV_NS_BASE,
        .frame = MHU_V2_X_RECEIVER_FRAME
};
#endif

/* ======= External peripheral configuration structure definitions ======= */

/* MT25QL Flash memory library structures */
#if (defined(MT25QL_S) && defined(QSPI_IP6514E_S))
struct mt25ql_dev_t MT25QL_DEV_S = {
    .controller = &QSPI_DEV_S,
    .direct_access_start_addr = MUSCA_B1_QSPI_FLASH_S_BASE,
    .baud_rate_div = 4U,
    /*
     * 8 MiB flash memory are advertised in the Arm Musca-B1 Test Chip and Board
     * Technical Reference Manual. The MT25QL Flash device may however contain
     * more.
     */
    .size = 0x00800000U, /* 8 MiB */
    .config_state = { 0 },
};
#endif

#if (defined(MT25QL_NS) && defined(QSPI_IP6514E_NS))
struct mt25ql_dev_t MT25QL_DEV_NS = {
    .controller = &QSPI_DEV_NS,
    .direct_access_start_addr = MUSCA_B1_QSPI_FLASH_NS_BASE,
    .baud_rate_div = 4U,
    /*
     * 8 MiB flash memory are advertised in the Arm Musca-B1 Test Chip and Board
     * Technical Reference Manual. The MT25QL Flash device may however contain
     * more.
     */
    .size = 0x00800000U, /* 8 MiB */
    .config_state = { 0 },
};
#endif
