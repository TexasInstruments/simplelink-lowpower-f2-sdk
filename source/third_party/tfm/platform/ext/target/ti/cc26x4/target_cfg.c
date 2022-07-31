/*
 * Copyright (c) 2017-2018 ARM Limited
 * Copyright (c) 2020 Texas Instruments Incorporated
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
#include "cmsis.h"
#include "platform_retarget_dev.h"
#include "region_defs.h"
#include "tfm_secure_api.h"
/* TI CC13x4/CC26x4 SDK include(s) */
#include "ti/devices/cc13x4_cc26x4/driverlib/aon_rtc.h"
#include "ti/devices/cc13x4_cc26x4/driverlib/setup.h" /* SetupTrimDevice() */
#include "ti/devices/cc13x4_cc26x4/driverlib/sys_ctrl.h"
#include "ti/devices/cc13x4_cc26x4/inc/hw_memmap.h"
#include "ti/devices/cc13x4_cc26x4/inc/hw_prcm.h"

/* Macros to pick linker symbols */
#define REGION(a, b, c) a##b##c
#define REGION_NAME(a, b, c) REGION(a, b, c)
#define REGION_DECLARE(a, b, c) extern uint32_t REGION_NAME(a, b, c)

/* The section names come from the scatter file */
REGION_DECLARE(Load$$LR$$, LR_NS_PARTITION, $$Base);
REGION_DECLARE(Load$$LR$$, LR_VENEER, $$Base);
REGION_DECLARE(Load$$LR$$, LR_VENEER, $$Limit);
#ifdef BL2
REGION_DECLARE(Load$$LR$$, LR_SECONDARY_PARTITION, $$Base);
#endif /* BL2 */

const struct memory_region_limits memory_regions = {
    .non_secure_code_start =
        (uint32_t)&REGION_NAME(Load$$LR$$, LR_NS_PARTITION, $$Base) +
        BL2_HEADER_SIZE,

    .non_secure_partition_base =
        (uint32_t)&REGION_NAME(Load$$LR$$, LR_NS_PARTITION, $$Base),

    .non_secure_partition_limit =
        (uint32_t)&REGION_NAME(Load$$LR$$, LR_NS_PARTITION, $$Base) +
        NS_PARTITION_SIZE - 1,

    .veneer_base = (uint32_t)&REGION_NAME(Load$$LR$$, LR_VENEER, $$Base),

    .veneer_limit = (uint32_t)&REGION_NAME(Load$$LR$$, LR_VENEER, $$Limit),

};

/* Non-secure peripheral end address for the platform */
#define NS_PERIPHERAL_REGION_ALIAS_END 0x77FFFFFF

/* To write into AIRCR register, 0x5FA value must be write to the VECTKEY field,
 * otherwise the processor ignores the write.
 */
#define SCB_AIRCR_WRITE_MASK ((0x5FAUL << SCB_AIRCR_VECTKEY_Pos))

/* Coprocessor access control register masks. Not defined in cmsis/core_cm33.h
 */
#define SCB_CPACR_CP10_MASK (0x3UL << 20U)
#define SCB_CPACR_CP11_MASK (0x3UL << 22U)

#if defined(__VTOR_PRESENT) && (__VTOR_PRESENT == 1U)
extern uint32_t __Vectors;
#endif

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
    uint32_t reg_value = SCB->AIRCR;

    /* Clear SCB_AIRCR_VECTKEY bits */
    reg_value &= ~(uint32_t)(SCB_AIRCR_VECTKEY_Msk);

    /* Enable system reset request only to the secure world */
    reg_value |= (uint32_t)(SCB_AIRCR_WRITE_MASK | SCB_AIRCR_SYSRESETREQS_Msk);

    /* !!!! agd 7/26/2019 This breaks the CCS debugger reset */
//    SCB->AIRCR = reg_value;

    /* Allow privileged and unprivileged access to VFP instructions */
    SCB->CPACR |= (SCB_CPACR_CP10_MASK | SCB_CPACR_CP11_MASK);

    /* Allow NS code to push/pop VFP regs */
    SCB->NSACR |= (SCB_NSACR_CP10_Msk | SCB_NSACR_CP11_Msk);

    /* Setup final trim of the device */
    SetupTrimDevice();

    /* Disable RTC timer */
    AONRTCDisable();

    /* Reset RTC timer */
    AONRTCReset();

    /* Sync AON interface */
    SysCtrlAonUpdate();

    /* Start RTC timer */
    AONRTCEnable();

    return TFM_PLAT_ERR_SUCCESS;
}

enum tfm_plat_err_t init_debug(void)
{
    return TFM_PLAT_ERR_SUCCESS;
}

/*----------------- NVIC interrupt target state to NS configuration ----------*/
enum tfm_plat_err_t nvic_interrupt_target_state_cfg(void)
{
    /* Target every interrupt to NS; unimplemented interrupts will be WI */
    for (uint8_t i = 0; i < (sizeof(NVIC->ITNS) / sizeof(NVIC->ITNS[0])); i++)
    {
        NVIC->ITNS[i] = 0xFFFFFFFF;
    }
    return TFM_PLAT_ERR_SUCCESS;
}

/*----------------- NVIC interrupt enabling for S peripherals ----------------*/
enum tfm_plat_err_t nvic_interrupt_enable(void)
{
    return TFM_PLAT_ERR_SUCCESS;
}

void verified_reg_write(const uint32_t addr, const uint32_t val, const uint32_t verify_mask)
{
    HWREG(addr) = val;

    /* Reset system if register read back does not match expected */
    if ((HWREG(addr) & verify_mask) != (val & verify_mask))
    {
        NVIC_SystemReset();
    }
}

#define PRCM_CPULOCK_ALL_LOCKS_M                                               \
    (PRCM_CPULOCK_LOCKNSVTOR | PRCM_CPULOCK_LOCKSVTAIRCR |                     \
     PRCM_CPULOCK_LOCKSAU | PRCM_CPULOCK_LOCKNSMPU | PRCM_CPULOCK_LOCKSMPU)

/*------------------- SAU/IDAU configuration functions -----------------------*/
__attribute__((noinline))
void sau_and_idau_cfg(void)
{
    /* Clear security config valid */
    verified_reg_write(PRCM_SEC_NONBUF_BASE + PRCM_O_BUSSECCFG,
                       PRCM_BUSSECCFG_BUS_CFG_M,
                       PRCM_BUSSECCFG_BUS_CFG_M | PRCM_BUSSECCFG_VALID);

    /* Clear all CPU lock bits and leave everything unlocked */
    verified_reg_write(PRCM_BASE + PRCM_O_CPULOCK, 0, PRCM_CPULOCK_ALL_LOCKS_M);

    /* Configure IDAU NSC Flash base address */
    verified_reg_write(PRCM_BASE + PRCM_O_NVMNSCADDR,
                       CMSE_VENEER_REGION_START & PRCM_NVMNSCADDR_BOUNDARY_M,
                       PRCM_NVMNSCADDR_BOUNDARY_M);

    /* Configure IDAU NS Flash base address */
    verified_reg_write(PRCM_BASE + PRCM_O_NVMNSADDR,
                       NS_ROM_ALIAS_BASE & PRCM_NVMNSADDR_BOUNDARY_M,
                       PRCM_NVMNSADDR_BOUNDARY_M);

    /* Configure IDAU NSC RAM base address.
     * NSC region in RAM is not utilized (region size is zero) so this address
     * matches the NS RAM base. */
    verified_reg_write(PRCM_BASE + PRCM_O_SRAMNSCADDR,
                       NS_RAM_ALIAS_BASE & PRCM_SRAMNSCADDR_BOUNDARY_M,
                       PRCM_SRAMNSCADDR_BOUNDARY_M);

    /* Configure IDAU NS RAM base address */
    verified_reg_write(PRCM_BASE + PRCM_O_SRAMNSADDR,
                       NS_RAM_ALIAS_BASE & PRCM_SRAMNSADDR_BOUNDARY_M,
                       PRCM_SRAMNSADDR_BOUNDARY_M);

    /* Set security config valid: Use non-buffered alias to ensure security
     * config has taken effect (response path is on ULL -> SVT clock) */
    verified_reg_write(PRCM_SEC_NONBUF_BASE + PRCM_O_BUSSECCFG,
                       PRCM_BUSSECCFG_BUS_CFG_M | PRCM_BUSSECCFG_VALID,
                       PRCM_BUSSECCFG_BUS_CFG_M | PRCM_BUSSECCFG_VALID);

    /* Define SAU region up to end of NS peripheral alias space as non-secure.
     * IDAU will define access rules */
    SAU->RNR = 0;
    SAU->RBAR = (0x00000000 & SAU_RBAR_BADDR_Msk);
    SAU->RLAR = (NS_PERIPHERAL_REGION_ALIAS_END & SAU_RLAR_LADDR_Msk) |
                SAU_RLAR_ENABLE_Msk;

    /* Enable SAU */
    TZ_SAU_Enable();

    /* Configure secure VTOR. VTOR is normally configured by SystemInit() but
     * since the PRCM CPULOCK.LOCKSVTAIRCR is still set when SystemInit() is
     * called, the register write has no effect so we must configure VTOR again
     * here after the lock bit has been cleared. */
#if defined(__VTOR_PRESENT) && (__VTOR_PRESENT == 1U)
    SCB->VTOR = (uint32_t)&__Vectors;
#endif
}

struct tfm_spm_partition_platform_data_t tfm_peripheral_std_uart = {0, 0};
