/*
 * Copyright (c) 2018-2020 Arm Limited. All rights reserved.
 * Copyright (c) 2020 Nordic Semiconductor ASA.
 * Copyright (c) 2021 Laird Connectivity.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "target_cfg.h"
#include "region_defs.h"
#include "tfm_plat_defs.h"
#include "tfm_peripherals_config.h"
#include "region.h"

#include <spu.h>
#include <nrfx.h>
#include <nrfx_nvmc.h>
#include <hal/nrf_nvmc.h>
#include <hal/nrf_gpio.h>
#include <hal/nrf_spu.h>

#define PIN_XL1 0
#define PIN_XL2 1

#if !(defined(NRF91_SERIES) || defined(NRF53_SERIES))
#error "Invalid configuration"
#endif


#if TFM_PERIPHERAL_DCNF_SECURE
struct platform_data_t tfm_peripheral_dcnf = {
    NRF_DCNF_S_BASE,
    NRF_DCNF_S_BASE + (sizeof(NRF_DCNF_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_FPU_SECURE
struct platform_data_t tfm_peripheral_fpu = {
    NRF_FPU_S_BASE,
    NRF_FPU_S_BASE + (sizeof(NRF_FPU_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_OSCILLATORS_SECURE
struct platform_data_t tfm_peripheral_oscillators = {
    NRF_OSCILLATORS_S_BASE,
    NRF_OSCILLATORS_S_BASE + (sizeof(NRF_OSCILLATORS_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_REGULATORS_SECURE
struct platform_data_t tfm_peripheral_regulators = {
    NRF_REGULATORS_S_BASE,
    NRF_REGULATORS_S_BASE + (sizeof(NRF_REGULATORS_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_CLOCK_SECURE
struct platform_data_t tfm_peripheral_clock = {
    NRF_CLOCK_S_BASE,
    NRF_CLOCK_S_BASE + (sizeof(NRF_CLOCK_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_POWER_SECURE
struct platform_data_t tfm_peripheral_power = {
    NRF_POWER_S_BASE,
    NRF_POWER_S_BASE + (sizeof(NRF_POWER_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_RESET_SECURE
struct platform_data_t tfm_peripheral_reset = {
    NRF_RESET_S_BASE,
    NRF_RESET_S_BASE + (sizeof(NRF_RESET_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_SPIM0_SECURE
struct platform_data_t tfm_peripheral_spim0 = {
    NRF_SPIM0_S_BASE,
    NRF_SPIM0_S_BASE + (sizeof(NRF_SPIM_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_SPIS0_SECURE
struct platform_data_t tfm_peripheral_spis0 = {
    NRF_SPIS0_S_BASE,
    NRF_SPIS0_S_BASE + (sizeof(NRF_SPIS_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_TWIM0_SECURE
struct platform_data_t tfm_peripheral_twim0 = {
    NRF_TWIM0_S_BASE,
    NRF_TWIM0_S_BASE + (sizeof(NRF_TWIM_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_TWIS0_SECURE
struct platform_data_t tfm_peripheral_twis0 = {
    NRF_TWIS0_S_BASE,
    NRF_TWIS0_S_BASE + (sizeof(NRF_TWIS_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_UARTE0_SECURE
struct platform_data_t tfm_peripheral_uarte0 = {
    NRF_UARTE0_S_BASE,
    NRF_UARTE0_S_BASE + (sizeof(NRF_UARTE_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_SPIM1_SECURE
struct platform_data_t tfm_peripheral_spim1 = {
    NRF_SPIM1_S_BASE,
    NRF_SPIM1_S_BASE + (sizeof(NRF_SPIM_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_SPIS1_SECURE
struct platform_data_t tfm_peripheral_spis1 = {
    NRF_SPIS1_S_BASE,
    NRF_SPIS1_S_BASE + (sizeof(NRF_SPIS_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_TWIM1_SECURE
struct platform_data_t tfm_peripheral_twim1 = {
    NRF_TWIM1_S_BASE,
    NRF_TWIM1_S_BASE + (sizeof(NRF_TWIM_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_TWIS1_SECURE
struct platform_data_t tfm_peripheral_twis1 = {
    NRF_TWIS1_S_BASE,
    NRF_TWIS1_S_BASE + (sizeof(NRF_TWIS_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_UARTE1_SECURE
struct platform_data_t tfm_peripheral_uarte1 = {
    NRF_UARTE1_S_BASE,
    NRF_UARTE1_S_BASE + (sizeof(NRF_UARTE_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_SPIM4_SECURE
struct platform_data_t tfm_peripheral_spim4 = {
    NRF_SPIM4_S_BASE,
    NRF_SPIM4_S_BASE + (sizeof(NRF_SPIM_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_SPIM2_SECURE
struct platform_data_t tfm_peripheral_spim2 = {
    NRF_SPIM2_S_BASE,
    NRF_SPIM2_S_BASE + (sizeof(NRF_SPIM_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_SPIS2_SECURE
struct platform_data_t tfm_peripheral_spis2 = {
    NRF_SPIS2_S_BASE,
    NRF_SPIS2_S_BASE + (sizeof(NRF_SPIS_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_TWIM2_SECURE
struct platform_data_t tfm_peripheral_twim2 = {
    NRF_TWIM2_S_BASE,
    NRF_TWIM2_S_BASE + (sizeof(NRF_TWIM_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_TWIS2_SECURE
struct platform_data_t tfm_peripheral_twis2 = {
    NRF_TWIS2_S_BASE,
    NRF_TWIS2_S_BASE + (sizeof(NRF_TWIS_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_UARTE2_SECURE
struct platform_data_t tfm_peripheral_uarte2 = {
    NRF_UARTE2_S_BASE,
    NRF_UARTE2_S_BASE + (sizeof(NRF_UARTE_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_SPIM3_SECURE
struct platform_data_t tfm_peripheral_spim3 = {
    NRF_SPIM3_S_BASE,
    NRF_SPIM3_S_BASE + (sizeof(NRF_SPIM_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_SPIS3_SECURE
struct platform_data_t tfm_peripheral_spis3 = {
    NRF_SPIS3_S_BASE,
    NRF_SPIS3_S_BASE + (sizeof(NRF_SPIS_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_TWIM3_SECURE
struct platform_data_t tfm_peripheral_twim3 = {
    NRF_TWIM3_S_BASE,
    NRF_TWIM3_S_BASE + (sizeof(NRF_TWIM_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_TWIS3_SECURE
struct platform_data_t tfm_peripheral_twis3 = {
    NRF_TWIS3_S_BASE,
    NRF_TWIS3_S_BASE + (sizeof(NRF_TWIS_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_UARTE3_SECURE
struct platform_data_t tfm_peripheral_uarte3 = {
    NRF_UARTE3_S_BASE,
    NRF_UARTE3_S_BASE + (sizeof(NRF_UARTE_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_SAADC_SECURE
struct platform_data_t tfm_peripheral_saadc = {
    NRF_SAADC_S_BASE,
    NRF_SAADC_S_BASE + (sizeof(NRF_SAADC_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_TIMER0_SECURE
struct platform_data_t tfm_peripheral_timer0 = {
    NRF_TIMER0_S_BASE,
    NRF_TIMER0_S_BASE + (sizeof(NRF_TIMER_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_TIMER1_SECURE
struct platform_data_t tfm_peripheral_timer1 = {
    NRF_TIMER1_S_BASE,
    NRF_TIMER1_S_BASE + (sizeof(NRF_TIMER_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_TIMER2_SECURE
struct platform_data_t tfm_peripheral_timer2 = {
    NRF_TIMER2_S_BASE,
    NRF_TIMER2_S_BASE + (sizeof(NRF_TIMER_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_RTC0_SECURE
struct platform_data_t tfm_peripheral_rtc0 = {
    NRF_RTC0_S_BASE,
    NRF_RTC0_S_BASE + (sizeof(NRF_RTC_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_RTC1_SECURE
struct platform_data_t tfm_peripheral_rtc1 = {
    NRF_RTC1_S_BASE,
    NRF_RTC1_S_BASE + (sizeof(NRF_RTC_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_DPPI_SECURE
struct platform_data_t tfm_peripheral_dppi = {
    NRF_DPPIC_S_BASE,
    NRF_DPPIC_S_BASE + (sizeof(NRF_DPPIC_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_WDT_SECURE
struct platform_data_t tfm_peripheral_wdt = {
    NRF_WDT_S_BASE,
    NRF_WDT_S_BASE + (sizeof(NRF_WDT_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_WDT0_SECURE
struct platform_data_t tfm_peripheral_wdt0 = {
    NRF_WDT0_S_BASE,
    NRF_WDT0_S_BASE + (sizeof(NRF_WDT_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_WDT1_SECURE
struct platform_data_t tfm_peripheral_wdt1 = {
    NRF_WDT1_S_BASE,
    NRF_WDT1_S_BASE + (sizeof(NRF_WDT_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_COMP_SECURE
struct platform_data_t tfm_peripheral_comp = {
    NRF_COMP_S_BASE,
    NRF_COMP_S_BASE + (sizeof(NRF_COMP_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_LPCOMP_SECURE
struct platform_data_t tfm_peripheral_lpcomp = {
    NRF_LPCOMP_S_BASE,
    NRF_LPCOMP_S_BASE + (sizeof(NRF_LPCOMP_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_EGU0_SECURE
struct platform_data_t tfm_peripheral_egu0 = {
    NRF_EGU0_S_BASE,
    NRF_EGU0_S_BASE + (sizeof(NRF_EGU_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_EGU1_SECURE
struct platform_data_t tfm_peripheral_egu1 = {
    NRF_EGU1_S_BASE,
    NRF_EGU1_S_BASE + (sizeof(NRF_EGU_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_EGU2_SECURE
struct platform_data_t tfm_peripheral_egu2 = {
    NRF_EGU2_S_BASE,
    NRF_EGU2_S_BASE + (sizeof(NRF_EGU_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_EGU3_SECURE
struct platform_data_t tfm_peripheral_egu3 = {
    NRF_EGU3_S_BASE,
    NRF_EGU3_S_BASE + (sizeof(NRF_EGU_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_EGU4_SECURE
struct platform_data_t tfm_peripheral_egu4 = {
    NRF_EGU4_S_BASE,
    NRF_EGU4_S_BASE + (sizeof(NRF_EGU_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_EGU5_SECURE
struct platform_data_t tfm_peripheral_egu5 = {
    NRF_EGU5_S_BASE,
    NRF_EGU5_S_BASE + (sizeof(NRF_EGU_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_PWM0_SECURE
struct platform_data_t tfm_peripheral_pwm0 = {
    NRF_PWM0_S_BASE,
    NRF_PWM0_S_BASE + (sizeof(NRF_PWM_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_PWM1_SECURE
struct platform_data_t tfm_peripheral_pwm1 = {
    NRF_PWM1_S_BASE,
    NRF_PWM1_S_BASE + (sizeof(NRF_PWM_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_PWM2_SECURE
struct platform_data_t tfm_peripheral_pwm2 = {
    NRF_PWM2_S_BASE,
    NRF_PWM2_S_BASE + (sizeof(NRF_PWM_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_PWM3_SECURE
struct platform_data_t tfm_peripheral_pwm3 = {
    NRF_PWM3_S_BASE,
    NRF_PWM3_S_BASE + (sizeof(NRF_PWM_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_PDM0_SECURE
struct platform_data_t tfm_peripheral_pdm0 = {
    NRF_PDM0_S_BASE,
    NRF_PDM0_S_BASE + (sizeof(NRF_PDM_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_PDM_SECURE
struct platform_data_t tfm_peripheral_pdm = {
    NRF_PDM_S_BASE,
    NRF_PDM_S_BASE + (sizeof(NRF_PDM_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_I2S0_SECURE
struct platform_data_t tfm_peripheral_i2s0 = {
    NRF_I2S0_S_BASE,
    NRF_I2S0_S_BASE + (sizeof(NRF_I2S_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_I2S_SECURE
struct platform_data_t tfm_peripheral_i2s = {
    NRF_I2S_S_BASE,
    NRF_I2S_S_BASE + (sizeof(NRF_I2S_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_IPC_SECURE
struct platform_data_t tfm_peripheral_ipc = {
    NRF_IPC_S_BASE,
    NRF_IPC_S_BASE + (sizeof(NRF_IPC_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_FPU_SECURE
struct platform_data_t tfm_peripheral_fpu = {
    NRF_FPU_S_BASE,
    NRF_FPU_S_BASE + (sizeof(NRF_FPU_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_QSPI_SECURE
struct platform_data_t tfm_peripheral_qspi = {
    NRF_QSPI_S_BASE,
    NRF_QSPI_S_BASE + (sizeof(NRF_QSPI_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_NFCT_SECURE
struct platform_data_t tfm_peripheral_nfct = {
    NRF_NFCT_S_BASE,
    NRF_NFCT_S_BASE + (sizeof(NRF_NFCT_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_MUTEX_SECURE
struct platform_data_t tfm_peripheral_mutex = {
    NRF_MUTEX_S_BASE,
    NRF_MUTEX_S_BASE + (sizeof(NRF_MUTEX_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_QDEC0_SECURE
struct platform_data_t tfm_peripheral_qdec0 = {
    NRF_QDEC0_S_BASE,
    NRF_QDEC0_S_BASE + (sizeof(NRF_QDEC_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_QDEC1_SECURE
struct platform_data_t tfm_peripheral_qdec1 = {
    NRF_QDEC1_S_BASE,
    NRF_QDEC1_S_BASE + (sizeof(NRF_QDEC_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_USBD_SECURE
struct platform_data_t tfm_peripheral_usbd = {
    NRF_USBD_S_BASE,
    NRF_USBD_S_BASE + (sizeof(NRF_USBD_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_USBREG_SECURE
struct platform_data_t tfm_peripheral_usbreg = {
    NRF_USBREGULATOR_S_BASE,
    NRF_USBREGULATOR_S_BASE + (sizeof(NRF_USBREG_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_NVMC_SECURE
struct platform_data_t tfm_peripheral_nvmc = {
    NRF_NVMC_S_BASE,
    NRF_NVMC_S_BASE + (sizeof(NRF_NVMC_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_GPIO0_SECURE
struct platform_data_t tfm_peripheral_gpio0 = {
    NRF_P0_S_BASE,
    NRF_P0_S_BASE + (sizeof(NRF_GPIO_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_GPIO1_SECURE
struct platform_data_t tfm_peripheral_gpio1 = {
    NRF_P1_S_BASE,
    NRF_P1_S_BASE + (sizeof(NRF_GPIO_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_VMC_SECURE
struct platform_data_t tfm_peripheral_vmc = {
    NRF_VMC_S_BASE,
    NRF_VMC_S_BASE + (sizeof(NRF_VMC_Type) - 1),
};
#endif

#ifdef PSA_API_TEST_IPC
struct platform_data_t
    tfm_peripheral_FF_TEST_SERVER_PARTITION_MMIO = {
        FF_TEST_SERVER_PARTITION_MMIO_START,
        FF_TEST_SERVER_PARTITION_MMIO_END
};

struct platform_data_t
    tfm_peripheral_FF_TEST_DRIVER_PARTITION_MMIO = {
        FF_TEST_DRIVER_PARTITION_MMIO_START,
        FF_TEST_DRIVER_PARTITION_MMIO_END
};

/* This platform implementation uses PSA_TEST_SCRATCH_AREA for
 * storing the state between resets, but the FF_TEST_NVMEM_REGIONS
 * definitons are still needed for tests to compile.
 */
#define FF_TEST_NVMEM_REGION_START  0xFFFFFFFF
#define FF_TEST_NVMEM_REGION_END    0xFFFFFFFF
struct platform_data_t
    tfm_peripheral_FF_TEST_NVMEM_REGION = {
        FF_TEST_NVMEM_REGION_START,
        FF_TEST_NVMEM_REGION_END
};
#endif /* PSA_API_TEST_IPC */


/* The section names come from the scatter file */
REGION_DECLARE(Load$$LR$$, LR_NS_PARTITION, $$Base);
REGION_DECLARE(Image$$, ER_VENEER, $$Base);
REGION_DECLARE(Image$$, VENEER_ALIGN, $$Limit);

const struct memory_region_limits memory_regions = {
    .non_secure_code_start =
        (uint32_t)&REGION_NAME(Load$$LR$$, LR_NS_PARTITION, $$Base) +
        BL2_HEADER_SIZE,

    .non_secure_partition_base =
        (uint32_t)&REGION_NAME(Load$$LR$$, LR_NS_PARTITION, $$Base),

    .non_secure_partition_limit =
        (uint32_t)&REGION_NAME(Load$$LR$$, LR_NS_PARTITION, $$Base) +
        NS_PARTITION_SIZE - 1,

    .veneer_base =
        (uint32_t)&REGION_NAME(Image$$, ER_VENEER, $$Base),

    .veneer_limit =
        (uint32_t)&REGION_NAME(Image$$, VENEER_ALIGN, $$Limit),

#ifdef NRF_NS_SECONDARY
    .secondary_partition_base = SECONDARY_PARTITION_START,
    .secondary_partition_limit = SECONDARY_PARTITION_START +
        SECONDARY_PARTITION_SIZE - 1,
#endif /* NRF_NS_SECONDARY */

#ifdef NRF_NS_STORAGE_PARTITION_START
    .non_secure_storage_partition_base = NRF_NS_STORAGE_PARTITION_START,
    .non_secure_storage_partition_limit = NRF_NS_STORAGE_PARTITION_START +
        NRF_NS_STORAGE_PARTITION_SIZE - 1,
#endif /* NRF_NS_STORAGE_PARTITION_START */
};

/* To write into AIRCR register, 0x5FA value must be write to the VECTKEY field,
 * otherwise the processor ignores the write.
 */
#define SCB_AIRCR_WRITE_MASK ((0x5FAUL << SCB_AIRCR_VECTKEY_Pos))

enum tfm_plat_err_t enable_fault_handlers(void)
{
    /* Explicitly set secure fault priority to the highest */
    NVIC_SetPriority(SecureFault_IRQn, 0);

    /* Enables BUS, MEM, USG and Secure faults */
    SCB->SHCSR |= SCB_SHCSR_USGFAULTENA_Msk |
                  SCB_SHCSR_BUSFAULTENA_Msk |
                  SCB_SHCSR_MEMFAULTENA_Msk |
                  SCB_SHCSR_SECUREFAULTENA_Msk;

    return TFM_PLAT_ERR_SUCCESS;
}

enum tfm_plat_err_t system_reset_cfg(void)
{
    uint32_t reg_value = SCB->AIRCR;

    /* Clear SCB_AIRCR_VECTKEY value */
    reg_value &= ~(uint32_t)(SCB_AIRCR_VECTKEY_Msk);

    /* Enable system reset request only to the secure world */
    reg_value |= (uint32_t)(SCB_AIRCR_WRITE_MASK | SCB_AIRCR_SYSRESETREQS_Msk);

    SCB->AIRCR = reg_value;

    return TFM_PLAT_ERR_SUCCESS;
}

enum tfm_plat_err_t init_debug(void)
{
#if defined(NRF91_SERIES)

#if !defined(DAUTH_CHIP_DEFAULT)
#error "Debug access on this platform can only be configured by programming the corresponding registers in UICR."
#endif

#elif defined(NRF53_SERIES)

#if defined(DAUTH_NONE)
    /* Disable debugging */
    NRF_CTRLAP->APPROTECT.DISABLE = 0;
    NRF_CTRLAP->SECUREAPPROTECT.DISABLE = 0;
#elif defined(DAUTH_NS_ONLY)
    /* Allow debugging Non-Secure only */
    NRF_CTRLAP->APPROTECT.DISABLE = NRF_UICR->APPROTECT;
    NRF_CTRLAP->SECUREAPPROTECT.DISABLE = 0;
#elif defined(DAUTH_FULL) || defined(DAUTH_CHIP_DEFAULT)
    /* Allow debugging */
    /* Use the configuration in UICR. */
    NRF_CTRLAP->APPROTECT.DISABLE = NRF_UICR->APPROTECT;
    NRF_CTRLAP->SECUREAPPROTECT.DISABLE = NRF_UICR->SECUREAPPROTECT;
#else
#error "No debug authentication setting is provided."
#endif

    /* Lock access to APPROTECT, SECUREAPPROTECT */
    NRF_CTRLAP->APPROTECT.LOCK = CTRLAPPERI_APPROTECT_LOCK_LOCK_Locked <<
        CTRLAPPERI_APPROTECT_LOCK_LOCK_Msk;
    NRF_CTRLAP->SECUREAPPROTECT.LOCK = CTRLAPPERI_SECUREAPPROTECT_LOCK_LOCK_Locked <<
        CTRLAPPERI_SECUREAPPROTECT_LOCK_LOCK_Msk;

#endif

    return TFM_PLAT_ERR_SUCCESS;
}

/*----------------- NVIC interrupt target state to NS configuration ----------*/
enum tfm_plat_err_t nvic_interrupt_target_state_cfg(void)
{
    /* Target every interrupt to NS; unimplemented interrupts will be Write-Ignored */
    for (uint8_t i = 0; i < sizeof(NVIC->ITNS) / sizeof(NVIC->ITNS[0]); i++) {
        NVIC->ITNS[i] = 0xFFFFFFFF;
    }

    /* Make sure that the SPU is targeted to S state */
    NVIC_ClearTargetState(NRFX_IRQ_NUMBER_GET(NRF_SPU));

#ifdef SECURE_UART1
    /* UARTE1 is a secure peripheral, so its IRQ has to target S state */
    NVIC_ClearTargetState(NRFX_IRQ_NUMBER_GET(NRF_UARTE1));
#endif

    return TFM_PLAT_ERR_SUCCESS;
}

/*----------------- NVIC interrupt enabling for S peripherals ----------------*/
enum tfm_plat_err_t nvic_interrupt_enable(void)
{
    /* SPU interrupt enabling */
    spu_enable_interrupts();

    NVIC_ClearPendingIRQ(NRFX_IRQ_NUMBER_GET(NRF_SPU));
    NVIC_EnableIRQ(NRFX_IRQ_NUMBER_GET(NRF_SPU));

    return TFM_PLAT_ERR_SUCCESS;
}

/*------------------- SAU/IDAU configuration functions -----------------------*/

void sau_and_idau_cfg(void)
{
    /* IDAU (SPU) is always enabled. SAU is non-existent.
     * Allow SPU to have precedence over (non-existing) ARMv8-M SAU.
     */
    TZ_SAU_Disable();
    SAU->CTRL |= SAU_CTRL_ALLNS_Msk;
}

enum tfm_plat_err_t spu_init_cfg(void)
{
    /*
     * Configure SPU Regions for Non-Secure Code and SRAM (Data)
     * Configure SPU for Peripheral Security
     * Configure Non-Secure Callable Regions
     * Configure Secondary Image Partition
     * Configure Non-Secure Storage Partition
     */

    /* Reset Flash and SRAM configuration of regions that are not owned by
     * the bootloader(s) to all-Secure.
     */
    spu_regions_reset_unlocked_secure();

    uint32_t perm;

    /* Configure Secure Code to be secure and RX */
    perm = 0;
    perm |= NRF_SPU_MEM_PERM_READ;
    /* Do not permit writes to secure flash */
    perm |= NRF_SPU_MEM_PERM_EXECUTE;

    spu_regions_flash_config(S_CODE_START, S_CODE_LIMIT, SPU_SECURE_ATTR_SECURE, perm,
			     SPU_LOCK_CONF_LOCKED);

    /* Configure Secure RAM to be secure and RWX */
    perm = 0;
    perm |= NRF_SPU_MEM_PERM_READ;
    perm |= NRF_SPU_MEM_PERM_WRITE;
    /* Permit execute from Secure RAM because otherwise Crypto fails
     * to initialize. */
    perm |= NRF_SPU_MEM_PERM_EXECUTE;

    spu_regions_sram_config(S_DATA_START, S_DATA_LIMIT, SPU_SECURE_ATTR_SECURE, perm,
			    SPU_LOCK_CONF_LOCKED);

    /* Configures SPU Code and Data regions to be non-secure */
    perm = 0;
    perm |= NRF_SPU_MEM_PERM_READ;
    perm |= NRF_SPU_MEM_PERM_WRITE;
    perm |= NRF_SPU_MEM_PERM_EXECUTE;

    spu_regions_flash_config(memory_regions.non_secure_partition_base,
			     memory_regions.non_secure_partition_limit, SPU_SECURE_ATTR_NONSECURE,
			     perm, SPU_LOCK_CONF_LOCKED);

    spu_regions_sram_config(NS_DATA_START, NS_DATA_LIMIT, SPU_SECURE_ATTR_NONSECURE, perm,
			    SPU_LOCK_CONF_LOCKED);

    /* Configures veneers region to be non-secure callable */
    spu_regions_flash_config_non_secure_callable(memory_regions.veneer_base,
						 memory_regions.veneer_limit - 1);

#ifdef NRF_NS_SECONDARY
	perm = 0;
	perm |= NRF_SPU_MEM_PERM_READ;
	perm |= NRF_SPU_MEM_PERM_WRITE;

    /* Secondary image partition */
    spu_regions_flash_config(memory_regions.secondary_partition_base,
			     memory_regions.secondary_partition_limit, SPU_SECURE_ATTR_NONSECURE,
			     perm, SPU_LOCK_CONF_LOCKED);
#endif /* NRF_NS_SECONDARY */

#ifdef NRF_NS_STORAGE_PARTITION_START
    /* Configures storage partition to be non-secure */
    perm = 0;
    perm |= NRF_SPU_MEM_PERM_READ;
    perm |= NRF_SPU_MEM_PERM_WRITE;

    spu_regions_flash_config(memory_regions.non_secure_storage_partition_base,
			     memory_regions.non_secure_storage_partition_limit,
			     SPU_SECURE_ATTR_NONSECURE, perm, SPU_LOCK_CONF_LOCKED);
#endif /* NRF_NS_STORAGE_PARTITION_START */

    return TFM_PLAT_ERR_SUCCESS;
}

enum tfm_plat_err_t spu_periph_init_cfg(void)
{
    /* Peripheral configuration */

    /* The following peripherals share ID:
     * - FPU (FPU cannot be configured in NRF91 series, it's always NS)
     * - DCNF (On 53, but not 91)
     */
#ifndef NRF91_SERIES
    spu_peripheral_config_non_secure((uint32_t)NRF_FPU, false);
#endif

    /* The following peripherals share ID:
     * - REGULATORS
     * - OSCILLATORS
     */
    spu_peripheral_config_non_secure((uint32_t)NRF_REGULATORS, false);
    /* The following peripherals share ID:
     * - CLOCK
     * - POWER
     * - RESET (On 53, but not 91)
     */
    spu_peripheral_config_non_secure((uint32_t)NRF_CLOCK, false);
    /* The following peripherals share ID: (referred to as Serial-Box)
     * - SPIMx
     * - SPISx
     * - TWIMx
     * - TWISx
     * - UARTEx
     */
    spu_peripheral_config_non_secure((uint32_t)NRF_SPIM0, false);
#ifndef SECURE_UART1
    /* UART1 is a secure peripheral, so we need to leave Serial-Box 1 as Secure */
    spu_peripheral_config_non_secure((uint32_t)NRF_SPIM1, false);
#endif
    spu_peripheral_config_non_secure((uint32_t)NRF_SPIM2, false);
    spu_peripheral_config_non_secure((uint32_t)NRF_SPIM3, false);
#ifdef NRF_SPIM4
    spu_peripheral_config_non_secure((uint32_t)NRF_SPIM4, false);
#endif
    spu_peripheral_config_non_secure((uint32_t)NRF_SAADC, false);
    spu_peripheral_config_non_secure((uint32_t)NRF_TIMER0, false);
    spu_peripheral_config_non_secure((uint32_t)NRF_TIMER1, false);
    spu_peripheral_config_non_secure((uint32_t)NRF_TIMER2, false);
    spu_peripheral_config_non_secure((uint32_t)NRF_RTC0, false);
    spu_peripheral_config_non_secure((uint32_t)NRF_RTC1, false);
    spu_peripheral_config_non_secure((uint32_t)NRF_DPPIC, false);
#ifndef PSA_API_TEST_IPC
#ifdef NRF_WDT0
    /* WDT0 is used as a secure peripheral in PSA FF tests */
    spu_peripheral_config_non_secure((uint32_t)NRF_WDT0, false);
#endif
#ifdef NRF_WDT
    spu_peripheral_config_non_secure((uint32_t)NRF_WDT, false);
#endif
#endif /* PSA_API_TEST_IPC */
#ifdef NRF_WDT1
    spu_peripheral_config_non_secure((uint32_t)NRF_WDT1, false);
#endif
    /* The following peripherals share ID:
     * - COMP
     * - LPCOMP
     */
#ifdef NRF_COMP
    spu_peripheral_config_non_secure((uint32_t)NRF_COMP, false);
#endif
    spu_peripheral_config_non_secure((uint32_t)NRF_EGU0, false);
    spu_peripheral_config_non_secure((uint32_t)NRF_EGU1, false);
    spu_peripheral_config_non_secure((uint32_t)NRF_EGU2, false);
    spu_peripheral_config_non_secure((uint32_t)NRF_EGU3, false);
    spu_peripheral_config_non_secure((uint32_t)NRF_EGU4, false);
#ifndef PSA_API_TEST_IPC
    /* EGU5 is used as a secure peripheral in PSA FF tests */
    spu_peripheral_config_non_secure((uint32_t)NRF_EGU5, false);
#endif
    spu_peripheral_config_non_secure((uint32_t)NRF_PWM0, false);
    spu_peripheral_config_non_secure((uint32_t)NRF_PWM1, false);
    spu_peripheral_config_non_secure((uint32_t)NRF_PWM2, false);
    spu_peripheral_config_non_secure((uint32_t)NRF_PWM3, false);
#ifdef NRF_PDM
    spu_peripheral_config_non_secure((uint32_t)NRF_PDM, false);
#endif
#ifdef NRF_PDM0
    spu_peripheral_config_non_secure((uint32_t)NRF_PDM0, false);
#endif
#ifdef NRF_I2S
    spu_peripheral_config_non_secure((uint32_t)NRF_I2S, false);
#endif
#ifdef NRF_I2S0
    spu_peripheral_config_non_secure((uint32_t)NRF_I2S0, false);
#endif
    spu_peripheral_config_non_secure((uint32_t)NRF_IPC, false);
#ifndef SECURE_QSPI
#ifdef NRF_QSPI
    spu_peripheral_config_non_secure((uint32_t)NRF_QSPI, false);
#endif
#endif
#ifdef NRF_NFCT
    spu_peripheral_config_non_secure((uint32_t)NRF_NFCT, false);
#endif
#ifdef NRF_MUTEX
    spu_peripheral_config_non_secure((uint32_t)NRF_MUTEX, false);
#endif
#ifdef NRF_QDEC0
    spu_peripheral_config_non_secure((uint32_t)NRF_QDEC0, false);
#endif
#ifdef NRF_QDEC1
    spu_peripheral_config_non_secure((uint32_t)NRF_QDEC1, false);
#endif
#ifdef NRF_USBD
    spu_peripheral_config_non_secure((uint32_t)NRF_USBD, false);
#endif
#ifdef NRF_USBREGULATOR
    spu_peripheral_config_non_secure((uint32_t)NRF_USBREGULATOR, false);
#endif
    spu_peripheral_config_non_secure((uint32_t)NRF_NVMC, false);
    spu_peripheral_config_non_secure((uint32_t)NRF_P0, false);
#ifdef NRF_P1
    spu_peripheral_config_non_secure((uint32_t)NRF_P1, false);
#endif
    spu_peripheral_config_non_secure((uint32_t)NRF_VMC, false);

    /* DPPI channel configuration */
    spu_dppi_config_non_secure(TFM_PERIPHERAL_DPPI_CHANNEL_MASK_SECURE, SPU_LOCK_CONF_LOCKED);

    /* GPIO pin configuration */
    spu_gpio_config_non_secure(0, TFM_PERIPHERAL_GPIO0_PIN_MASK_SECURE, SPU_LOCK_CONF_LOCKED);
#ifdef TFM_PERIPHERAL_GPIO1_PIN_MASK_SECURE
    spu_gpio_config_non_secure(1, TFM_PERIPHERAL_GPIO1_PIN_MASK_SECURE, SPU_LOCK_CONF_LOCKED);
#endif

#ifdef NRF53_SERIES
    /* Configure properly the XL1 and XL2 pins so that the low-frequency crystal
     * oscillator (LFXO) can be used.
     * This configuration can be done only from secure code, as otherwise those
     * register fields are not accessible. That's why it is placed here.
     */
    nrf_gpio_pin_control_select(PIN_XL1, NRF_GPIO_PIN_SEL_PERIPHERAL);
    nrf_gpio_pin_control_select(PIN_XL2, NRF_GPIO_PIN_SEL_PERIPHERAL);
#endif

	/*
	 * 91 has an instruction cache.
	 * 53 has both instruction cache and a data cache.
	 *
	 * 53's instruction cache has an nrfx driver, but 91's cache is
	 * not supported by nrfx at time of writing.
	 *
	 * We enable all caches available here because non-secure cannot
	 * configure caches.
	 */
#if defined(NVMC_FEATURE_CACHE_PRESENT) // From MDK
	nrfx_nvmc_icache_enable();
#elif defined(CACHE_PRESENT) // From MDK
	NRF_CACHE->ENABLE = CACHE_ENABLE_ENABLE_Enabled;
#endif

    /* Enforce that the nRF5340 Network MCU is in the Non-Secure
     * domain. Non-secure is the HW reset value for the network core
     * so configuring this should not be necessary, but we want to
     * make sure that the bootloader has not accidentally configured
     * it to be secure. Additionally we lock the register to make sure
     * it doesn't get changed by accident.
     */
    nrf_spu_extdomain_set(NRF_SPU, 0, false, true);

    return TFM_PLAT_ERR_SUCCESS;
}

void spu_periph_configure_to_secure(uint32_t periph_num)
{
    spu_peripheral_config_secure(periph_num, true);
}

void spu_periph_configure_to_non_secure(uint32_t periph_num)
{
    spu_peripheral_config_non_secure(periph_num, true);
}
