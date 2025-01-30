/*
 * Copyright (c) 2019-2023 Arm Limited. All rights reserved.
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

#include "common_target_cfg.h"
#include "Driver_MPC.h"
#include "Driver_PPC.h"
#include "device_definition.h"
#include "platform_description.h"
#include "region_defs.h"
#include "mpu_armv8m_drv.h"
#include "tfm_plat_defs.h"
#include "region.h"

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

static struct mpu_armv8m_dev_t dev_mpu = { MPU_BASE };

/* Allows software, via SAU, to define the code region as a NSC */
#define NSCCFG_CODENSC  1

extern const struct memory_region_limits memory_regions;

/* Import MPC drivers */
extern ARM_DRIVER_MPC Driver_QSPI_MPC;
extern ARM_DRIVER_MPC Driver_ISRAM0_MPC;
extern ARM_DRIVER_MPC Driver_ISRAM1_MPC;
extern ARM_DRIVER_MPC Driver_ISRAM2_MPC;
extern ARM_DRIVER_MPC Driver_ISRAM3_MPC;

/* Import PPC drivers */
extern ARM_DRIVER_PPC Driver_AHB_PPCEXP0;
extern ARM_DRIVER_PPC Driver_APB_PPC0;
extern ARM_DRIVER_PPC Driver_APB_PPC1;
extern ARM_DRIVER_PPC Driver_APB_PPCEXP0;
extern ARM_DRIVER_PPC Driver_APB_PPCEXP1;
extern ARM_DRIVER_PPC Driver_APB_PPCEXP2;

/* Define Peripherals NS address range for the platform */
#define PERIPHERALS_BASE_NS_START      (0x40000000)
#define PERIPHERALS_BASE_NS_END        (0x4FFFFFFF)

/* Enable system reset request for CPU 0 */
#define ENABLE_CPU0_SYSTEM_RESET_REQUEST (1U << 4U)

/* To write into AIRCR register, 0x5FA value must be write to the VECTKEY field,
 * otherwise the processor ignores the write.
 */
#define SCB_AIRCR_WRITE_MASK ((0x5FAUL << SCB_AIRCR_VECTKEY_Pos))

struct platform_data_t tfm_peripheral_std_uart = {
        UART1_BASE_S,
        UART1_BASE_S + 0xFFF,
        PPC_SP_APB_PPC_EXP2,
        CMSDK_UART1_APB_PPC_POS
};

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
    &Driver_APB_PPCEXP2,    /* APB PPCEXP2 */
};

#define PPC_BANK_COUNT (sizeof(ppc_bank_drivers)/sizeof(ppc_bank_drivers[0]))

struct platform_data_t tfm_peripheral_timer0 = {
        CMSDK_TIMER0_BASE_S,
        CMSDK_TIMER1_BASE_S - 1,
        PPC_SP_APB_PPC0,
        CMSDK_TIMER0_APB_PPC_POS
};

enum tfm_plat_err_t enable_fault_handlers(void)
{
    /* Explicitly set secure fault priority to the highest */
    NVIC_SetPriority(SecureFault_IRQn, 0);

    /* Enables BUS, MEM, USG and Secure faults */
    SCB->SHCSR |= (SCB_SHCSR_USGFAULTENA_Msk | SCB_SHCSR_BUSFAULTENA_Msk |
                   SCB_SHCSR_MEMFAULTENA_Msk | SCB_SHCSR_SECUREFAULTENA_Msk);
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
    sys_ctrl->secdbgclr =
                   DBGEN_STATUS | NIDEN_STATUS | SPIDEN_STATUS | SPNIDEN_STATUS;
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
    sys_ctrl->secdbgset =
                   DBGEN_STATUS | NIDEN_STATUS | SPIDEN_STATUS | SPNIDEN_STATUS;
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

/*--------------------- NVIC interrupt NS/S configuration --------------------*/
enum tfm_plat_err_t nvic_interrupt_target_state_cfg(void)
{
    uint8_t i;

    /* Target every interrupt to NS; unimplemented interrupts will be WI */
    for (i = 0; i < (sizeof(NVIC->ITNS) / sizeof(NVIC->ITNS[0])); i++) {
        NVIC->ITNS[i] = 0xFFFFFFFF;
    }

    /* Make sure that MPC and PPC are targeted to S state */
    NVIC_ClearTargetState(MPC_IRQn);
    NVIC_ClearTargetState(PPC_IRQn);

    return TFM_PLAT_ERR_SUCCESS;
}

/*----------------- NVIC interrupt enabling for S peripherals ----------------*/
enum tfm_plat_err_t nvic_interrupt_enable(void)
{
    int32_t ret = ARM_DRIVER_OK;
    /* MPC interrupt enabling */
    ret = Driver_QSPI_MPC.EnableInterrupt();
    if (ret != ARM_DRIVER_OK) {
        return TFM_PLAT_ERR_SYSTEM_ERR;
    }
    ret = Driver_ISRAM2_MPC.EnableInterrupt();
    if (ret != ARM_DRIVER_OK) {
        return TFM_PLAT_ERR_SYSTEM_ERR;
    }
    ret = Driver_ISRAM3_MPC.EnableInterrupt();
    if (ret != ARM_DRIVER_OK) {
        return TFM_PLAT_ERR_SYSTEM_ERR;
    }
    NVIC_EnableIRQ(MPC_IRQn);

    /* PPC interrupt enabling */
    /* Clear pending PPC interrupts */
    /* In the PPC configuration function, we have used the Non-Secure
     * Privilege Control Block to grant unprivilged NS access to some
     * peripherals used by NS. That triggers a PPC0 exception as that
     * register is meant for NS privileged access only. Clear it here
     */
    Driver_APB_PPC0.ClearInterrupt();

    /* Enable PPC interrupts for APB PPC */
    ret = Driver_APB_PPC0.EnableInterrupt();
    if (ret != ARM_DRIVER_OK) {
        return TFM_PLAT_ERR_SYSTEM_ERR;
    }
    ret = Driver_APB_PPC1.EnableInterrupt();
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
    ret = Driver_APB_PPCEXP2.EnableInterrupt();
    if (ret != ARM_DRIVER_OK) {
        return TFM_PLAT_ERR_SYSTEM_ERR;
    }

    NVIC_EnableIRQ(PPC_IRQn);

    return (enum tfm_plat_err_t) ARM_DRIVER_OK;
}

/*------------------- SAU/IDAU configuration functions -----------------------*/
void sau_and_idau_cfg(void)
{
    struct spctrl_def *spctrl = CMSDK_SPCTRL;

    /* Ensure all memory accesses are completed */
    __DMB();

    /* Enables SAU */
    TZ_SAU_Enable();

    /* Configures SAU regions to be non-secure */
    SAU->RNR  = 0U;
    SAU->RBAR = (memory_regions.non_secure_partition_base
                 & SAU_RBAR_BADDR_Msk);
    SAU->RLAR = (memory_regions.non_secure_partition_limit
                  & SAU_RLAR_LADDR_Msk) | SAU_RLAR_ENABLE_Msk;

    SAU->RNR  = 1U;
    SAU->RBAR = (NS_DATA_START & SAU_RBAR_BADDR_Msk);
    SAU->RLAR = (NS_DATA_LIMIT & SAU_RLAR_LADDR_Msk) | SAU_RLAR_ENABLE_Msk;

    /* Configures veneers region to be non-secure callable */
    SAU->RNR  = 2U;
    SAU->RBAR = (memory_regions.veneer_base & SAU_RBAR_BADDR_Msk);
    SAU->RLAR = (memory_regions.veneer_limit & SAU_RLAR_LADDR_Msk)
                 | SAU_RLAR_ENABLE_Msk | SAU_RLAR_NSC_Msk;

    /* Configure the peripherals space */
    SAU->RNR  = 3U;
    SAU->RBAR = (PERIPHERALS_BASE_NS_START & SAU_RBAR_BADDR_Msk);
    SAU->RLAR = (PERIPHERALS_BASE_NS_END & SAU_RLAR_LADDR_Msk)
                  | SAU_RLAR_ENABLE_Msk;
#ifdef BL2
    /* Secondary image partition */
    SAU->RNR  = 4U;
    /* TODO */
    SAU->RBAR = (memory_regions.secondary_partition_base
                 & SAU_RBAR_BADDR_Msk);
    SAU->RLAR = (memory_regions.secondary_partition_limit
                 & SAU_RLAR_LADDR_Msk) | SAU_RLAR_ENABLE_Msk;
#endif /* BL2 */

    /* Allows SAU to define the code region as a NSC */
    spctrl->nsccfg |= NSCCFG_CODENSC;

    /* Ensure the write is completed and flush pipeline */
    __DSB();
    __ISB();
}

/*------------------- Memory configuration functions -------------------------*/
enum tfm_plat_err_t mpc_init_cfg(void)
{
    int32_t ret = ARM_DRIVER_OK;

    ret = Driver_QSPI_MPC.Initialize();
    if (ret != ARM_DRIVER_OK) {
        return ret;
    }

    ret = Driver_QSPI_MPC.ConfigRegion(
                    memory_regions.non_secure_partition_base,
                    memory_regions.non_secure_partition_limit,
                    ARM_MPC_ATTR_NONSECURE);
    if (ret != ARM_DRIVER_OK) {
        return ret;
    }
#ifdef BL2
    ret = Driver_QSPI_MPC.ConfigRegion(
                    memory_regions.secondary_partition_base,
                    memory_regions.secondary_partition_limit,
                    ARM_MPC_ATTR_NONSECURE);
    if (ret != ARM_DRIVER_OK) {
        return ret;
    }
#endif /* BL2 */

    /* SPE uses the first 96kB (3 ISRAM banks) for data */
    ret = Driver_ISRAM0_MPC.Initialize();
    if (ret != ARM_DRIVER_OK) {
        return ret;
    }

    ret = Driver_ISRAM0_MPC.ConfigRegion(
                        MPC_ISRAM0_RANGE_BASE_S,
                        MPC_ISRAM0_RANGE_LIMIT_S,
                        ARM_MPC_ATTR_SECURE);
    if (ret != ARM_DRIVER_OK) {
        return ret;
    }

    ret = Driver_ISRAM1_MPC.Initialize();
    if (ret != ARM_DRIVER_OK) {
        return ret;
    }

    ret = Driver_ISRAM1_MPC.ConfigRegion(
                        MPC_ISRAM1_RANGE_BASE_S,
                        MPC_ISRAM1_RANGE_LIMIT_S,
                        ARM_MPC_ATTR_SECURE);
    if (ret != ARM_DRIVER_OK) {
        return ret;
    }

    ret = Driver_ISRAM2_MPC.Initialize();
    if (ret != ARM_DRIVER_OK) {
        return ret;
    }

    ret = Driver_ISRAM2_MPC.ConfigRegion(
                        MPC_ISRAM2_RANGE_BASE_S,
                        MPC_ISRAM2_RANGE_LIMIT_S,
                        ARM_MPC_ATTR_SECURE);
    if (ret != ARM_DRIVER_OK) {
        return ret;
    }

    /* NSPE use the last 32KB (ISRAM 3) */
    ret = Driver_ISRAM3_MPC.Initialize();
    if (ret != ARM_DRIVER_OK) {
        return ret;
    }

    ret = Driver_ISRAM3_MPC.ConfigRegion(
                        MPC_ISRAM3_RANGE_BASE_NS,
                        MPC_ISRAM3_RANGE_LIMIT_NS,
                        ARM_MPC_ATTR_NONSECURE);
    if (ret != ARM_DRIVER_OK) {
        return ret;
    }

    /* Lock down the MPC configuration */
    ret = Driver_QSPI_MPC.LockDown();
    if (ret != ARM_DRIVER_OK) {
        return ret;
    }

    ret = Driver_ISRAM0_MPC.LockDown();
    if (ret != ARM_DRIVER_OK) {
        return ret;
    }

    ret = Driver_ISRAM1_MPC.LockDown();
    if (ret != ARM_DRIVER_OK) {
        return ret;
    }

    ret = Driver_ISRAM2_MPC.LockDown();
    if (ret != ARM_DRIVER_OK) {
        return ret;
    }

    ret = Driver_ISRAM3_MPC.LockDown();
    if (ret != ARM_DRIVER_OK) {
        return ret;
    }

    /* Add barriers to assure the MPC configuration is done before continue
     * the execution.
     */
    __DSB();
    __ISB();

    return TFM_PLAT_ERR_SUCCESS;
}

void mpc_revert_non_secure_to_secure_cfg(void)
{
    Driver_ISRAM3_MPC.ConfigRegion(MPC_ISRAM3_RANGE_BASE_S,
                                   MPC_ISRAM3_RANGE_LIMIT_S,
                                   ARM_MPC_ATTR_SECURE);

    Driver_QSPI_MPC.ConfigRegion(MPC_QSPI_RANGE_BASE_S,
                                 MPC_QSPI_RANGE_LIMIT_S,
                                 ARM_MPC_ATTR_SECURE);

    /* Add barriers to assure the MPC configuration is done before continue
     * the execution.
     */
    __DSB();
    __ISB();
}

void mpc_clear_irq(void)
{
    Driver_QSPI_MPC.ClearInterrupt();
    Driver_ISRAM0_MPC.ClearInterrupt();
    Driver_ISRAM1_MPC.ClearInterrupt();
    Driver_ISRAM2_MPC.ClearInterrupt();
    Driver_ISRAM3_MPC.ClearInterrupt();
}

/*------------------- PPC configuration functions -------------------------*/
enum tfm_plat_err_t ppc_init_cfg(void)
{
    struct spctrl_def *spctrl = CMSDK_SPCTRL;
    int32_t ret = ARM_DRIVER_OK;

    /* Grant non-secure access to peripherals in the PPC0
     * (timer0 and 1, dualtimer, watchdog, mhu 0 and 1)
     */
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
    ret = Driver_APB_PPC0.ConfigPeriph(CMSDK_MHU0_APB_PPC_POS,
                                    ARM_PPC_NONSECURE_ONLY,
                                    ARM_PPC_PRIV_ONLY);

    ret = Driver_APB_PPC1.Initialize();
    if (ret != ARM_DRIVER_OK) {
        return ret;
    }

    /* No peripherals are configured on APB PPC EXP0 but device needs to be
     * initialialized so that the interrupt can be enabled later.
     */
    ret = Driver_APB_PPCEXP0.Initialize();
    if (ret != ARM_DRIVER_OK) {
        return ret;
    }

    /* Grant non-secure access for APB peripherals on EXP1 */
    ret = Driver_APB_PPCEXP1.Initialize();
    if (ret != ARM_DRIVER_OK) {
        return ret;
    }
    ret = Driver_APB_PPCEXP1.ConfigPeriph(CMSDK_I2C0_APB_PPC_POS,
                                        ARM_PPC_NONSECURE_ONLY,
                                        ARM_PPC_PRIV_ONLY);
    if (ret != ARM_DRIVER_OK) {
        return ret;
    }
    ret = Driver_APB_PPCEXP1.ConfigPeriph(CMSDK_I2C1_APB_PPC_POS,
                                        ARM_PPC_NONSECURE_ONLY,
                                        ARM_PPC_PRIV_ONLY);
    if (ret != ARM_DRIVER_OK) {
        return ret;
    }
    ret = Driver_APB_PPCEXP1.ConfigPeriph(CMSDK_SPI0_APB_PPC_POS,
                                        ARM_PPC_NONSECURE_ONLY,
                                        ARM_PPC_PRIV_ONLY);
    if (ret != ARM_DRIVER_OK) {
        return ret;
    }
    ret = Driver_APB_PPCEXP1.ConfigPeriph(CMSDK_SPI1_APB_PPC_POS,
                                        ARM_PPC_NONSECURE_ONLY,
                                        ARM_PPC_PRIV_ONLY);
    if (ret != ARM_DRIVER_OK) {
        return ret;
    }
    ret = Driver_APB_PPCEXP1.ConfigPeriph(CMSDK_SPI2_APB_PPC_POS,
                                        ARM_PPC_NONSECURE_ONLY,
                                        ARM_PPC_PRIV_ONLY);
    if (ret != ARM_DRIVER_OK) {
        return ret;
    }
    ret = Driver_APB_PPCEXP1.ConfigPeriph(CMSDK_I2C2_APB_PPC_POS,
                                        ARM_PPC_NONSECURE_ONLY,
                                        ARM_PPC_PRIV_ONLY);
    if (ret != ARM_DRIVER_OK) {
        return ret;
    }
    ret = Driver_APB_PPCEXP1.ConfigPeriph(CMSDK_I2C3_APB_PPC_POS,
                                        ARM_PPC_NONSECURE_ONLY,
                                        ARM_PPC_PRIV_ONLY);
    if (ret != ARM_DRIVER_OK) {
        return ret;
    }
    ret = Driver_APB_PPCEXP1.ConfigPeriph(CMSDK_I2C4_APB_PPC_POS,
                                        ARM_PPC_NONSECURE_ONLY,
                                        ARM_PPC_PRIV_ONLY);
    if (ret != ARM_DRIVER_OK) {
        return ret;
    }

    /* Grant non-secure access for APB peripherals on EXP2 */
    ret = Driver_APB_PPCEXP2.Initialize();
    if (ret != ARM_DRIVER_OK) {
        return ret;
    }
    ret = Driver_APB_PPCEXP2.ConfigPeriph(CMSDK_FPGA_SCC_PPC_POS,
                                        ARM_PPC_NONSECURE_ONLY,
                                        ARM_PPC_PRIV_ONLY);
    if (ret != ARM_DRIVER_OK) {
        return ret;
    }
    ret = Driver_APB_PPCEXP2.ConfigPeriph(CMSDK_FPGA_AUDIO_PPC_POS,
                                        ARM_PPC_NONSECURE_ONLY,
                                        ARM_PPC_PRIV_ONLY);
    if (ret != ARM_DRIVER_OK) {
        return ret;
    }
    ret = Driver_APB_PPCEXP2.ConfigPeriph(CMSDK_FPGA_IO_PPC_POS,
                                        ARM_PPC_NONSECURE_ONLY,
                                        ARM_PPC_PRIV_AND_NONPRIV);
    if (ret != ARM_DRIVER_OK) {
        return ret;
    }
    ret = Driver_APB_PPCEXP2.ConfigPeriph(CMSDK_UART0_APB_PPC_POS,
                                        ARM_PPC_NONSECURE_ONLY,
                                        ARM_PPC_PRIV_ONLY);
    if (ret != ARM_DRIVER_OK) {
        return ret;
    }
    ret = Driver_APB_PPCEXP2.ConfigPeriph(CMSDK_UART1_APB_PPC_POS,
                                        ARM_PPC_NONSECURE_ONLY,
                                        ARM_PPC_PRIV_ONLY);
    if (ret != ARM_DRIVER_OK) {
        return ret;
    }
    ret = Driver_APB_PPCEXP2.ConfigPeriph(CMSDK_UART2_APB_PPC_POS,
                                        ARM_PPC_NONSECURE_ONLY,
                                        ARM_PPC_PRIV_ONLY);
    if (ret != ARM_DRIVER_OK) {
        return ret;
    }
    ret = Driver_APB_PPCEXP2.ConfigPeriph(CMSDK_UART3_APB_PPC_POS,
                                        ARM_PPC_NONSECURE_ONLY,
                                        ARM_PPC_PRIV_ONLY);
    if (ret != ARM_DRIVER_OK) {
        return ret;
    }
    ret = Driver_APB_PPCEXP2.ConfigPeriph(CMSDK_UART4_APB_PPC_POS,
                                        ARM_PPC_NONSECURE_ONLY,
                                        ARM_PPC_PRIV_ONLY);
    if (ret != ARM_DRIVER_OK) {
        return ret;
    }
    ret = Driver_APB_PPCEXP2.ConfigPeriph(CMSDK_UART5_APB_PPC_POS,
                                        ARM_PPC_NONSECURE_ONLY,
                                        ARM_PPC_PRIV_ONLY);
    if (ret != ARM_DRIVER_OK) {
        return ret;
    }
    ret = Driver_APB_PPCEXP2.ConfigPeriph(CMSDK_CLCD_APB_PPC_POS,
                                        ARM_PPC_NONSECURE_ONLY,
                                        ARM_PPC_PRIV_ONLY);
    if (ret != ARM_DRIVER_OK) {
        return ret;
    }
    ret = Driver_APB_PPCEXP2.ConfigPeriph(CMSDK_RTC_APB_PPC_POS,
                                        ARM_PPC_NONSECURE_ONLY,
                                        ARM_PPC_PRIV_ONLY);
    if (ret != ARM_DRIVER_OK) {
        return ret;
    }

    /*
     * Grant non-secure access to all peripherals on AHB EXP0:
     * Make sure that all possible peripherals are enabled by default
     */
    ret = Driver_AHB_PPCEXP0.Initialize();
    if (ret != ARM_DRIVER_OK) {
        return ret;
    }
    ret = Driver_AHB_PPCEXP0.ConfigPeriph(CMSDK_GPIO0_PPC_POS,
                                        ARM_PPC_NONSECURE_ONLY,
                                        ARM_PPC_PRIV_ONLY);
    if (ret != ARM_DRIVER_OK) {
        return ret;
    }
    ret = Driver_AHB_PPCEXP0.ConfigPeriph(CMSDK_GPIO1_PPC_POS,
                                        ARM_PPC_NONSECURE_ONLY,
                                        ARM_PPC_PRIV_ONLY);
    if (ret != ARM_DRIVER_OK) {
        return ret;
    }
    ret = Driver_AHB_PPCEXP0.ConfigPeriph(CMSDK_GPIO2_PPC_POS,
                                        ARM_PPC_NONSECURE_ONLY,
                                        ARM_PPC_PRIV_ONLY);
    if (ret != ARM_DRIVER_OK) {
        return ret;
    }
    ret = Driver_AHB_PPCEXP0.ConfigPeriph(CMSDK_GPIO3_PPC_POS,
                                        ARM_PPC_NONSECURE_ONLY,
                                        ARM_PPC_PRIV_ONLY);
    if (ret != ARM_DRIVER_OK) {
        return ret;
    }
    ret = Driver_AHB_PPCEXP0.ConfigPeriph(CMSDK_USB_ETHERNET_PPC_POS,
                                        ARM_PPC_NONSECURE_ONLY,
                                        ARM_PPC_PRIV_ONLY);
    if (ret != ARM_DRIVER_OK) {
        return ret;
    }
    ret = Driver_AHB_PPCEXP0.ConfigPeriph(CMSDK_USER0_PPC_POS,
                                        ARM_PPC_NONSECURE_ONLY,
                                        ARM_PPC_PRIV_ONLY);
    if (ret != ARM_DRIVER_OK) {
        return ret;
    }
    ret = Driver_AHB_PPCEXP0.ConfigPeriph(CMSDK_USER1_PPC_POS,
                                        ARM_PPC_NONSECURE_ONLY,
                                        ARM_PPC_PRIV_ONLY);
    if (ret != ARM_DRIVER_OK) {
        return ret;
    }
    ret = Driver_AHB_PPCEXP0.ConfigPeriph(CMSDK_USER2_PPC_POS,
                                        ARM_PPC_NONSECURE_ONLY,
                                        ARM_PPC_PRIV_ONLY);
    if (ret != ARM_DRIVER_OK) {
        return ret;
    }

    /*
     * Configure the response to a security violation as a
     * bus error instead of RAZ/WI
     */
    spctrl->secrespcfg |= CMSDK_SECRESPCFG_BUS_ERR_MASK;

    return TFM_PLAT_ERR_SUCCESS;
}

void ppc_configure_to_secure(ppc_bank_t bank, uint32_t pos)
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

void ppc_en_secure_unpriv(ppc_bank_t bank, uint32_t pos)
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

void ppc_clr_secure_unpriv(ppc_bank_t bank, uint32_t pos)
{
    ppc_configure_to_secure(bank, pos);
}

void ppc_clear_irq(void)
{
    Driver_AHB_PPCEXP0.ClearInterrupt();
    Driver_APB_PPC0.ClearInterrupt();
    Driver_APB_PPC1.ClearInterrupt();
    Driver_APB_PPCEXP0.ClearInterrupt();
    Driver_APB_PPCEXP1.ClearInterrupt();
    Driver_APB_PPCEXP2.ClearInterrupt();
}
