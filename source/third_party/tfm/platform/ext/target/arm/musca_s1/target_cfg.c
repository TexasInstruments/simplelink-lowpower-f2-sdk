/*
 * Copyright (c) 2018-2023 Arm Limited. All rights reserved.
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
#include "Driver_MPC.h"
#include "Driver_PPC.h"
#include "platform_description.h"
#include "device_definition.h"
#include "region_defs.h"
#include "tfm_plat_defs.h"
#include "region.h"
#include "cmsis_driver_config.h"

#define MIN(A, B) (((A) < (B)) ? (A) : (B))
#define MAX(A, B) (((A) > (B)) ? (A) : (B))

/* The section names come from the scatter file */
REGION_DECLARE(Load$$LR$$, LR_NS_PARTITION, $$Base);
REGION_DECLARE(Image$$, ER_VENEER, $$Base);
REGION_DECLARE(Image$$, VENEER_ALIGN, $$Limit);
REGION_DECLARE(Load$$LR$$, LR_SECONDARY_PARTITION, $$Base);

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
        (uint32_t)&REGION_NAME(Image$$, VENEER_ALIGN, $$Limit) - 1,
};

/* Allows software, via SAU, to define the code region as a NSC */
#define NSCCFG_CODENSC  1

/* Import MPC driver */
extern ARM_DRIVER_MPC Driver_MRAM_MPC;
extern ARM_DRIVER_MPC Driver_ISRAM0_MPC, Driver_ISRAM1_MPC;
extern ARM_DRIVER_MPC Driver_ISRAM2_MPC, Driver_ISRAM3_MPC;

/* Import PPC driver */
extern ARM_DRIVER_PPC Driver_APB_PPC0, Driver_APB_PPC1;
extern ARM_DRIVER_PPC Driver_AHB_PPCEXP0, Driver_AHB_PPCEXP1;
extern ARM_DRIVER_PPC Driver_APB_PPCEXP0, Driver_APB_PPCEXP1;
extern ARM_DRIVER_PPC Driver_APB_PPCEXP2, Driver_APB_PPCEXP3;

/* Define Peripherals NS address range for the platform */
#define PERIPHERALS_BASE_NS_START (0x40000000)
#define PERIPHERALS_BASE_NS_END   (0x4FFFFFFF)

/* Enable system reset request for CPU 0 */
#define ENABLE_CPU0_SYSTEM_RESET_REQUEST (1U << 4U)

/* To write into AIRCR register, 0x5FA value must be write to the VECTKEY field,
 * otherwise the processor ignores the write.
 */
#define SCB_AIRCR_WRITE_MASK ((0x5FAUL << SCB_AIRCR_VECTKEY_Pos))

/* Debug configuration flags */
#define SPNIDEN_SEL_STATUS (0x01u << 7)
#define SPNIDEN_STATUS     (0x01u << 6)
#define SPIDEN_SEL_STATUS  (0x01u << 5)
#define SPIDEN_STATUS      (0x01u << 4)
#define NIDEN_SEL_STATUS   (0x01u << 3)
#define NIDEN_STATUS       (0x01u << 2)
#define DBGEN_SEL_STATUS   (0x01u << 1)
#define DBGEN_STATUS       (0x01u << 0)

#define All_SEL_STATUS (SPNIDEN_SEL_STATUS | SPIDEN_SEL_STATUS | \
                        NIDEN_SEL_STATUS | DBGEN_SEL_STATUS)

static ARM_DRIVER_PPC *const ppc_bank_drivers[] = {
    0,                      /* AHB PPC0 */
    0,                      /* Reserved */
    0,                      /* Reserved */
    0,                      /* Reserved */
    &Driver_AHB_PPCEXP0,    /* AHB PPCEXP0 */
    0,                      /* AHB PPCEXP1 */
    0,                      /* AHB PPCEXP2 */
    0,                      /* AHB PPCEXP3 */
    &Driver_APB_PPC0,       /* APB PPC0 */
    &Driver_APB_PPC1,       /* APB PPC1 */
    0,                      /* Reserved */
    0,                      /* Reserved */
    &Driver_APB_PPCEXP0,    /* APB PPCEXP0 */
    &Driver_APB_PPCEXP1,    /* APB PPCEXP1 */
};

#define PPC_BANK_COUNT \
    (sizeof(ppc_bank_drivers)/sizeof(ppc_bank_drivers[0]))


struct platform_data_t tfm_peripheral_std_uart = {
        MUSCA_S1_UART1_NS_BASE,
        MUSCA_S1_UART1_NS_BASE + 0xFFF,
        PPC_SP_DO_NOT_CONFIGURE,
        -1
};

struct platform_data_t tfm_peripheral_timer0 = {
        MUSCA_S1_CMSDK_TIMER0_S_BASE,
        MUSCA_S1_CMSDK_TIMER1_S_BASE - 1,
        PPC_SP_APB_PPC0,
        CMSDK_TIMER0_APB_PPC_POS
};

#ifdef PSA_API_TEST_IPC

/* Below data structure are only used for PSA FF tests, and this pattern is
 * definitely not to be followed for real life use cases, as it can break
 * security.
 */

struct platform_data_t
    tfm_peripheral_FF_TEST_UART_REGION = {
        MUSCA_S1_UART1_NS_BASE,
        MUSCA_S1_UART1_NS_BASE + 0xFFF,
        PPC_SP_DO_NOT_CONFIGURE,
        -1
};

struct platform_data_t
    tfm_peripheral_FF_TEST_WATCHDOG_REGION = {
        MUSCA_S1_CMSDK_WATCHDOG_S_BASE,
        MUSCA_S1_CMSDK_WATCHDOG_S_BASE + 0xFFF,
        PPC_SP_DO_NOT_CONFIGURE,
        -1
};

#define FF_TEST_NVMEM_REGION_START 0x3003F800
#define FF_TEST_NVMEM_REGION_END 0x3003FBFF
#define FF_TEST_SERVER_PARTITION_MMIO_START 0x3003FC00
#define FF_TEST_SERVER_PARTITION_MMIO_END 0x3003FCFF
#define FF_TEST_DRIVER_PARTITION_MMIO_START 0x3003FE00
#define FF_TEST_DRIVER_PARTITION_MMIO_END 0x3003FEFF

struct platform_data_t
    tfm_peripheral_FF_TEST_NVMEM_REGION = {
        FF_TEST_NVMEM_REGION_START,
        FF_TEST_NVMEM_REGION_END,
        PPC_SP_DO_NOT_CONFIGURE,
        -1
};

struct platform_data_t
    tfm_peripheral_FF_TEST_SERVER_PARTITION_MMIO = {
        FF_TEST_SERVER_PARTITION_MMIO_START,
        FF_TEST_SERVER_PARTITION_MMIO_END,
        PPC_SP_DO_NOT_CONFIGURE,
        -1
};

struct platform_data_t
    tfm_peripheral_FF_TEST_DRIVER_PARTITION_MMIO = {
        FF_TEST_DRIVER_PARTITION_MMIO_START,
        FF_TEST_DRIVER_PARTITION_MMIO_END,
        PPC_SP_DO_NOT_CONFIGURE,
        -1
};

#endif

enum tfm_plat_err_t enable_fault_handlers(void)
{
    /* Explicitly set secure fault priority to the highest */
    NVIC_SetPriority(SecureFault_IRQn, 0);

    /* Enables BUS, MEM, USG and Secure faults */
    SCB->SHCSR |= SCB_SHCSR_USGFAULTENA_Msk
                  | SCB_SHCSR_BUSFAULTENA_Msk
                  | SCB_SHCSR_MEMFAULTENA_Msk
                  | SCB_SHCSR_SECUREFAULTENA_Msk;
    return TFM_PLAT_ERR_SUCCESS;
}

enum tfm_plat_err_t system_reset_cfg(void)
{
    struct sysctrl_t *sysctrl = (struct sysctrl_t *)CMSDK_SYSCTRL_BASE_S;
    uint32_t reg_value = SCB->AIRCR;

    /* Enable system reset request for CPU 0, to be triggered via
     * NVIC_SystemReset function.
     */
    sysctrl->resetmask |= ENABLE_CPU0_SYSTEM_RESET_REQUEST;

    /* Clear SCB_AIRCR_VECTKEY value */
    reg_value &= ~(uint32_t)(SCB_AIRCR_VECTKEY_Msk);

    /* Enable system reset request only to the secure world */
    reg_value |= (uint32_t)(SCB_AIRCR_WRITE_MASK | SCB_AIRCR_SYSRESETREQS_Msk);

    SCB->AIRCR = reg_value;

    return TFM_PLAT_ERR_SUCCESS;
}

enum tfm_plat_err_t init_debug(void)
{
    volatile struct sysctrl_t *sys_ctrl =
                                       (struct sysctrl_t *)CMSDK_SYSCTRL_BASE_S;

#if defined(DAUTH_NONE)
    /* Set all the debug enable selector bits to 1 */
    sys_ctrl->secdbgset = All_SEL_STATUS;
    /* Set all the debug enable bits to 0 */
    sys_ctrl->secdbgclr = DBGEN_STATUS |
                          NIDEN_STATUS |
                          SPIDEN_STATUS |
                          SPNIDEN_STATUS;

#elif defined(DAUTH_NS_ONLY)
    /* Set all the debug enable selector bits to 1 */
    sys_ctrl->secdbgset = All_SEL_STATUS;
    /* Set the debug enable bits to 1 for NS, and 0 for S mode */
    sys_ctrl->secdbgset = DBGEN_STATUS | NIDEN_STATUS;
    sys_ctrl->secdbgclr = SPIDEN_STATUS | SPNIDEN_STATUS;
#elif defined(DAUTH_FULL)
    /* Set all the debug enable selector bits to 1 */
    sys_ctrl->secdbgset = All_SEL_STATUS;
    /* Set all the debug enable bits to 1 */
    sys_ctrl->secdbgset = DBGEN_STATUS |
                          NIDEN_STATUS |
                          SPIDEN_STATUS |
                          SPNIDEN_STATUS;
#else

#if !defined(DAUTH_CHIP_DEFAULT)
#error "No debug authentication setting is provided."
#endif

    /* Set all the debug enable selector bits to 0 */
    sys_ctrl->secdbgclr = All_SEL_STATUS;

    /* No need to set any enable bits because the value depends on
     * input signals.
     */
#endif
    return TFM_PLAT_ERR_SUCCESS;
}

/*----------------- NVIC interrupt target state to NS configuration ----------*/
enum tfm_plat_err_t nvic_interrupt_target_state_cfg(void)
{
    /* Target every interrupt to NS; unimplemented interrupts will be WI */
    for (uint8_t i=0; i<sizeof(NVIC->ITNS)/sizeof(NVIC->ITNS[0]); i++) {
        NVIC->ITNS[i] = 0xFFFFFFFF;
    }

    /* Make sure that MPC and PPC are targeted to S state */
    NVIC_ClearTargetState(S_MPC_COMBINED_IRQn);
    NVIC_ClearTargetState(S_PPC_COMBINED_IRQn);

    return TFM_PLAT_ERR_SUCCESS;
}

/*----------------- NVIC interrupt enabling for S peripherals ----------------*/
enum tfm_plat_err_t nvic_interrupt_enable()
{
    int32_t ret = ARM_DRIVER_OK;

    /* MPC interrupt enabling */
    ret = Driver_MRAM_MPC.EnableInterrupt();
    if (ret != ARM_DRIVER_OK) {
        return TFM_PLAT_ERR_SYSTEM_ERR;
    }
    NVIC_EnableIRQ(S_MPC_COMBINED_IRQn);

    /* PPC interrupt enabling */
    /* Clear pending PPC interrupts */
    /* In the PPC configuration function, we have used the Non-Secure
     * Privilege Control Block to grant unprivilged NS access to some
     * peripherals used by NS. That triggers a PPC0 exception as that
     * register is meant for NS privileged access only. Clear it here
     */

    /* Enable PPC interrupts for APB PPC */
    Driver_APB_PPC0.ClearInterrupt();

    ret = Driver_APB_PPC0.EnableInterrupt();
    if (ret != ARM_DRIVER_OK) {
        return TFM_PLAT_ERR_SYSTEM_ERR;
    }
    ret = Driver_APB_PPC1.EnableInterrupt();
    if (ret != ARM_DRIVER_OK) {
        return TFM_PLAT_ERR_SYSTEM_ERR;
    }
    ret = Driver_AHB_PPCEXP0.EnableInterrupt();
    if (ret != ARM_DRIVER_OK) {
        return TFM_PLAT_ERR_SYSTEM_ERR;
    }

    ret = Driver_APB_PPCEXP0.EnableInterrupt();
    if (ret != ARM_DRIVER_OK) {
        return TFM_PLAT_ERR_SYSTEM_ERR;
    }
    ret = Driver_APB_PPCEXP1.EnableInterrupt();
    if (ret != ARM_DRIVER_OK) {
        return TFM_PLAT_ERR_SYSTEM_ERR;
    }

    NVIC_EnableIRQ(S_PPC_COMBINED_IRQn);

#ifdef PSA_API_TEST_IPC
    NVIC_EnableIRQ(FF_TEST_UART_IRQ);
#endif

    return TFM_PLAT_ERR_SUCCESS;
}

/*------------------- SAU/IDAU configuration functions -----------------------*/

void sau_and_idau_cfg(void)
{
    /* Ensure all memory accesses are completed */
    __DMB();

    /* Enables SAU */
    TZ_SAU_Enable();

    /* Configures SAU regions to be non-secure */
    SAU->RNR  = 0U;
    SAU->RBAR = (memory_regions.non_secure_partition_base
                & SAU_RBAR_BADDR_Msk);
    SAU->RLAR = (memory_regions.non_secure_partition_limit
                & SAU_RLAR_LADDR_Msk)
                | SAU_RLAR_ENABLE_Msk;

    SAU->RNR  = 1U;
    SAU->RBAR = (NS_DATA_START & SAU_RBAR_BADDR_Msk);
    SAU->RLAR = (NS_DATA_LIMIT & SAU_RLAR_LADDR_Msk) | SAU_RLAR_ENABLE_Msk;

    /* Configures veneers region to be non-secure callable */
    SAU->RNR  = 2U;
    SAU->RBAR = (memory_regions.veneer_base  & SAU_RBAR_BADDR_Msk);
    SAU->RLAR = (memory_regions.veneer_limit & SAU_RLAR_LADDR_Msk)
                | SAU_RLAR_ENABLE_Msk
                | SAU_RLAR_NSC_Msk;

    /* Configure the peripherals space */
    SAU->RNR  = 3U;
    SAU->RBAR = (PERIPHERALS_BASE_NS_START & SAU_RBAR_BADDR_Msk);
    SAU->RLAR = (PERIPHERALS_BASE_NS_END & SAU_RLAR_LADDR_Msk)
                | SAU_RLAR_ENABLE_Msk;

    /* Allows SAU to define the code region as a NSC */
    struct spctrl_def* spctrl = CMSDK_SPCTRL;
    spctrl->nsccfg |= NSCCFG_CODENSC;

    /* Ensure the write is completed and flush pipeline */
    __DSB();
    __ISB();
}

/*------------------- Memory configuration functions -------------------------*/

int32_t mpc_init_region_with_attr(ARM_DRIVER_MPC *region,
                         uintptr_t region_base, uintptr_t region_limit,
                         uintptr_t attr_range_start, uintptr_t attr_range_limit,
                         ARM_MPC_SEC_ATTR attr)
{
    uintptr_t range_start = MAX(region_base, attr_range_start);
    uintptr_t range_limit = MIN(region_limit, attr_range_limit);

    if (range_start < range_limit) {
        /* ConfigRegion checks whether the range addresses are aligned at MPC
         * block border
         */
        return region->ConfigRegion(range_start, range_limit, attr);
    }
    return ARM_DRIVER_OK;
}

int32_t mpc_init_cfg(void)
{
    ARM_DRIVER_MPC* mpc_data_region0 = &Driver_ISRAM0_MPC;
    ARM_DRIVER_MPC* mpc_data_region1 = &Driver_ISRAM1_MPC;
    ARM_DRIVER_MPC* mpc_data_region2 = &Driver_ISRAM2_MPC;
    ARM_DRIVER_MPC* mpc_data_region3 = &Driver_ISRAM3_MPC;

    int32_t ret = ARM_DRIVER_OK;

    ret = Driver_MRAM_MPC.Initialize();
    if (ret != ARM_DRIVER_OK) {
        return ret;
    }

    ret = Driver_MRAM_MPC.ConfigRegion(memory_regions.non_secure_partition_base,
                                 memory_regions.non_secure_partition_limit,
                                 ARM_MPC_ATTR_NONSECURE);
    if (ret != ARM_DRIVER_OK) {
        return ret;
    }

    ret = mpc_data_region0->Initialize();
    if (ret != ARM_DRIVER_OK) {
        return ret;
    }

    ret = mpc_data_region0->ConfigRegion(MPC_ISRAM0_RANGE_BASE_S,
                                   MPC_ISRAM0_RANGE_LIMIT_S,
                                   ARM_MPC_ATTR_SECURE);
    if (ret != ARM_DRIVER_OK) {
        return ret;
    }

    ret = mpc_data_region1->Initialize();
    if (ret != ARM_DRIVER_OK) {
        return ret;
    }

    ret = mpc_data_region1->ConfigRegion(MPC_ISRAM1_RANGE_BASE_S,
                                   MPC_ISRAM1_RANGE_LIMIT_S,
                                   ARM_MPC_ATTR_SECURE);
    if (ret != ARM_DRIVER_OK) {
        return ret;
    }

    ret = mpc_data_region2->Initialize();
    if (ret != ARM_DRIVER_OK) {
        return ret;
    }

    ret = mpc_data_region2->ConfigRegion(MPC_ISRAM2_RANGE_BASE_NS,
                                   MPC_ISRAM2_RANGE_LIMIT_NS,
                                   ARM_MPC_ATTR_NONSECURE);
    if (ret != ARM_DRIVER_OK) {
        return ret;
    }

    ret = mpc_data_region3->Initialize();
    if (ret != ARM_DRIVER_OK) {
        return ret;
    }

    ret = mpc_data_region3->ConfigRegion(MPC_ISRAM3_RANGE_BASE_NS,
                                   MPC_ISRAM3_RANGE_LIMIT_NS,
                                   ARM_MPC_ATTR_NONSECURE);
    if (ret != ARM_DRIVER_OK) {
        return ret;
    }

    /* NOTE: The recommended and expected way of programming MPCs requires to
     * lock each MPC at this point, so no further configuration is allowed.
     * However there is a hardware issue in Musca-S1, that makes it necessary to
     * allow re-configuration before reset. Therefore locking is skipped here.
     */

    /* Add barriers to assure the MPC configuration is done before continue
     * the execution.
     */
    __DSB();
    __ISB();

    return ARM_DRIVER_OK;
}

/*  Due to a hardware issue NVIC_SystemReset() does not reset all the MPCs,
 *  and these retain incorrect settings after reset. This can block the
 *  boot process.
 *  To avoid such cases mpc_revert_non_secure_to_secure_cfg() is implemented
 *  to revert the MPC settings back to secure.
 */
void mpc_revert_non_secure_to_secure_cfg(void)
{
    ARM_DRIVER_MPC* mpc_data_region2 = &Driver_ISRAM2_MPC;
    ARM_DRIVER_MPC* mpc_data_region3 = &Driver_ISRAM3_MPC;

    Driver_MRAM_MPC.ConfigRegion(MPC_MRAM_RANGE_BASE_S,
                                 MPC_MRAM_RANGE_LIMIT_S,
                                 ARM_MPC_ATTR_SECURE);

    mpc_data_region2->ConfigRegion(MPC_ISRAM2_RANGE_BASE_S,
                                   MPC_ISRAM2_RANGE_LIMIT_S,
                                   ARM_MPC_ATTR_SECURE);

    mpc_data_region3->ConfigRegion(MPC_ISRAM3_RANGE_BASE_S,
                                   MPC_ISRAM3_RANGE_LIMIT_S,
                                   ARM_MPC_ATTR_SECURE);

    /* Add barriers to assure the MPC configuration is done before continue
     * the execution.
     */
    __DSB();
    __ISB();
}

/*---------------------- PPC configuration functions -------------------------*/

int32_t ppc_init_cfg(void)
{
    struct spctrl_def* spctrl = CMSDK_SPCTRL;

    /* Grant non-secure access to peripherals in the PPC0
     * (timer0 and 1, dualtimer, mhu 0 and 1)
     */
    int32_t ret = ARM_DRIVER_OK;

    ret = Driver_APB_PPC0.Initialize();
    if (ret != ARM_DRIVER_OK) {
        return ret;
    }
    ret = Driver_APB_PPC0.ConfigPeriph(CMSDK_TIMER0_APB_PPC_POS,
                                 ARM_PPC_NONSECURE_ONLY,
                                 ARM_PPC_PRIV_ONLY);
    if (ret != ARM_DRIVER_OK) {
        return ret;
    }
    ret = Driver_APB_PPC0.ConfigPeriph(CMSDK_TIMER1_APB_PPC_POS,
                                 ARM_PPC_NONSECURE_ONLY,
                                 ARM_PPC_PRIV_ONLY);
    if (ret != ARM_DRIVER_OK) {
        return ret;
    }
    ret = Driver_APB_PPC0.ConfigPeriph(CMSDK_DTIMER_APB_PPC_POS,
                                 ARM_PPC_NONSECURE_ONLY,
                                 ARM_PPC_PRIV_ONLY);
    if (ret != ARM_DRIVER_OK) {
        return ret;
    }
    ret = Driver_APB_PPC0.ConfigPeriph(CMSDK_MHU0_APB_PPC_POS,
                                 ARM_PPC_NONSECURE_ONLY,
                                 ARM_PPC_PRIV_ONLY);
    if (ret != ARM_DRIVER_OK) {
        return ret;
    }
    ret = Driver_APB_PPC0.ConfigPeriph(CMSDK_MHU1_APB_PPC_POS,
                                 ARM_PPC_NONSECURE_ONLY,
                                 ARM_PPC_PRIV_ONLY);
    if (ret != ARM_DRIVER_OK) {
        return ret;
    }

    ret = Driver_APB_PPC1.Initialize();
    if (ret != ARM_DRIVER_OK) {
        return ret;
    }

    /* Grant non-secure access for all APB peripherals on EXP1.
     * For the specific APB PPC bit definitions on EXP1 see region_defs.h
     */
    ret = Driver_AHB_PPCEXP0.Initialize();
    if (ret != ARM_DRIVER_OK) {
        return ret;
    }
    ret = Driver_AHB_PPCEXP0.ConfigPeriph(MUSCA_S1_GPIO_AHB_PPC_POS,
                                 ARM_PPC_NONSECURE_ONLY,
                                 ARM_PPC_PRIV_AND_NONPRIV);
    if (ret != ARM_DRIVER_OK) {
        return ret;
    }

    ret = Driver_APB_PPCEXP0.Initialize();
    if (ret != ARM_DRIVER_OK) {
        return ret;
    }

    ret = Driver_APB_PPCEXP1.Initialize();
    if (ret != ARM_DRIVER_OK) {
        return ret;
    }

    ret = Driver_APB_PPCEXP1.ConfigPeriph(MUSCA_S1_UART0_APB_PPC_POS,
                                 ARM_PPC_NONSECURE_ONLY,
                                 ARM_PPC_PRIV_AND_NONPRIV);
    if (ret != ARM_DRIVER_OK) {
        return ret;
    }
    ret = Driver_APB_PPCEXP1.ConfigPeriph(MUSCA_S1_UART1_APB_PPC_POS,
                                 ARM_PPC_NONSECURE_ONLY,
                                 ARM_PPC_PRIV_AND_NONPRIV);
    if (ret != ARM_DRIVER_OK) {
        return ret;
    }
    ret = Driver_APB_PPCEXP1.ConfigPeriph(MUSCA_S1_SPI_APB_PPC_POS,
                                 ARM_PPC_NONSECURE_ONLY,
                                 ARM_PPC_PRIV_AND_NONPRIV);
    if (ret != ARM_DRIVER_OK) {
        return ret;
    }
    ret = Driver_APB_PPCEXP1.ConfigPeriph(MUSCA_S1_I2C0_APB_PPC_POS,
                                 ARM_PPC_NONSECURE_ONLY,
                                 ARM_PPC_PRIV_AND_NONPRIV);
    if (ret != ARM_DRIVER_OK) {
        return ret;
    }
    ret = Driver_APB_PPCEXP1.ConfigPeriph(MUSCA_S1_I2C1_APB_PPC_POS,
                                 ARM_PPC_NONSECURE_ONLY,
                                 ARM_PPC_PRIV_AND_NONPRIV);
    if (ret != ARM_DRIVER_OK) {
        return ret;
    }
    ret = Driver_APB_PPCEXP1.ConfigPeriph(MUSCA_S1_I2S_APB_PPC_POS,
                                 ARM_PPC_NONSECURE_ONLY,
                                 ARM_PPC_PRIV_AND_NONPRIV);
    if (ret != ARM_DRIVER_OK) {
        return ret;
    }
    ret = Driver_APB_PPCEXP1.ConfigPeriph(MUSCA_S1_PWM0_APB_PPC_POS,
                                 ARM_PPC_NONSECURE_ONLY,
                                 ARM_PPC_PRIV_AND_NONPRIV);
    if (ret != ARM_DRIVER_OK) {
        return ret;
    }
    ret = Driver_APB_PPCEXP1.ConfigPeriph(MUSCA_S1_RTC_APB_PPC_POS,
                                 ARM_PPC_NONSECURE_ONLY,
                                 ARM_PPC_PRIV_AND_NONPRIV);
    if (ret != ARM_DRIVER_OK) {
        return ret;
    }
    ret = Driver_APB_PPCEXP1.ConfigPeriph(MUSCA_S1_PVT_APB_PPC_POS,
                                 ARM_PPC_NONSECURE_ONLY,
                                 ARM_PPC_PRIV_AND_NONPRIV);
    if (ret != ARM_DRIVER_OK) {
        return ret;
    }
    ret = Driver_APB_PPCEXP1.ConfigPeriph(MUSCA_S1_GPTIMER0_APB_PPC_POS,
                                 ARM_PPC_NONSECURE_ONLY,
                                 ARM_PPC_PRIV_AND_NONPRIV);
    if (ret != ARM_DRIVER_OK) {
        return ret;
    }
    ret = Driver_APB_PPCEXP1.ConfigPeriph(MUSCA_S1_GPTIMER1_APB_PPC_POS,
                                 ARM_PPC_NONSECURE_ONLY,
                                 ARM_PPC_PRIV_AND_NONPRIV);
    if (ret != ARM_DRIVER_OK) {
        return ret;
    }
    ret = Driver_APB_PPCEXP1.ConfigPeriph(MUSCA_S1_PWM1_APB_PPC_POS,
                                 ARM_PPC_NONSECURE_ONLY,
                                 ARM_PPC_PRIV_AND_NONPRIV);
    if (ret != ARM_DRIVER_OK) {
        return ret;
    }
    ret = Driver_APB_PPCEXP1.ConfigPeriph(MUSCA_S1_PWM2_APB_PPC_POS,
                                 ARM_PPC_NONSECURE_ONLY,
                                 ARM_PPC_PRIV_AND_NONPRIV);
    if (ret != ARM_DRIVER_OK) {
        return ret;
    }

    /* Configure the response to a security violation as a
     * bus error instead of RAZ/WI
     */
    spctrl->secrespcfg |= 1U;

    return ARM_DRIVER_OK;
}

void ppc_configure_to_non_secure(enum ppc_bank_e bank, uint16_t pos)
{
    /* Setting NS flag for peripheral to enable NS access */
    ARM_DRIVER_PPC *ppc_driver;

    if (bank >= PPC_BANK_COUNT) {
        return;
    }

    ppc_driver = ppc_bank_drivers[bank];
    if (ppc_driver) {
        ppc_driver->ConfigPeriph(pos, ARM_PPC_NONSECURE_ONLY,
                                 ARM_PPC_PRIV_ONLY);
    }
}

void ppc_configure_to_secure(enum ppc_bank_e bank, uint16_t pos)
{
    /* Clear NS flag for peripheral to prevent NS access */
    ARM_DRIVER_PPC *ppc_driver;

    if (bank >= PPC_BANK_COUNT) {
        return;
    }

    ppc_driver = ppc_bank_drivers[bank];
    if (ppc_driver) {
        ppc_driver->ConfigPeriph(pos, ARM_PPC_SECURE_ONLY,
                                 ARM_PPC_PRIV_ONLY);
    }
}

void ppc_en_secure_unpriv(enum ppc_bank_e bank, uint16_t pos)
{
    ARM_DRIVER_PPC *ppc_driver;

    if (bank >= PPC_BANK_COUNT) {
        return;
    }

    ppc_driver = ppc_bank_drivers[bank];
    if (ppc_driver) {
        ppc_driver->ConfigPeriph(pos, ARM_PPC_SECURE_ONLY,
                                 ARM_PPC_PRIV_AND_NONPRIV);
    }
}

void ppc_clr_secure_unpriv(enum ppc_bank_e bank, uint16_t pos)
{
    ARM_DRIVER_PPC *ppc_driver;

    if (bank >= PPC_BANK_COUNT) {
        return;
    }

    ppc_driver = ppc_bank_drivers[bank];
    if (ppc_driver) {
        ppc_driver->ConfigPeriph(pos, ARM_PPC_SECURE_ONLY,
                                 ARM_PPC_PRIV_ONLY);
    }
}

void ppc_clear_irq(void)
{
    Driver_AHB_PPCEXP0.ClearInterrupt();
    Driver_APB_PPC0.ClearInterrupt();
    Driver_APB_PPC1.ClearInterrupt();
    Driver_APB_PPCEXP0.ClearInterrupt();
    Driver_APB_PPCEXP1.ClearInterrupt();
}
