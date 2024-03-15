/*
 * Copyright (c) 2017-2022, ARM Limited. All rights reserved.
 * Copyright (c) 2020-2023, Texas Instruments Incorporated. All rights reserved.
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
#include "Driver_Common.h"
#include "fih.h"
#include "platform_retarget_dev.h"
#include "region_defs.h"
#include "region.h"
#include "tfm_plat_defs.h"
#include "ti_safe.h"

/* TI CC13x4/CC26x4 SDK include(s) */
#include "ti/devices/cc13x4_cc26x4/driverlib/aon_rtc.h"
#include "ti/devices/cc13x4_cc26x4/driverlib/flash.h"
#include "ti/devices/cc13x4_cc26x4/driverlib/setup.h"    /* SetupTrimDevice() */
#include "ti/devices/cc13x4_cc26x4/driverlib/sys_ctrl.h" /* SysCtrlSystemReset() */
#include "ti/devices/cc13x4_cc26x4/inc/hw_memmap.h"
#include "ti/devices/cc13x4_cc26x4/inc/hw_prcm.h"
#include "ti/devices/cc13x4_cc26x4/inc/hw_types.h" /* HWREG() */

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

    .veneer_base = (uint32_t)&REGION_NAME(Image$$, ER_VENEER, $$Base),

    .veneer_limit = (uint32_t)&REGION_NAME(Image$$, VENEER_ALIGN, $$Limit),
};

/* Non-secure peripheral end address for the platform */
#define NS_PERIPHERAL_REGION_ALIAS_END 0x77FFFFFF

/* To write into AIRCR register, 0x5FA value must be write to the VECTKEY field,
 * otherwise the processor ignores the write.
 */
#define SCB_AIRCR_WRITE_MASK ((0x5FAUL << SCB_AIRCR_VECTKEY_Pos))

/* Flash write/erase protect sector 0 */
#define FLASH_WEPROT_SECTOR_0 0x1UL

/* Dummy platform data as this is not yet supported */
struct platform_data_t tfm_peripheral_std_uart = {0, 0};

/* Exception / Interrupt Vector table */
extern const VECTOR_TABLE_Type __VECTOR_TABLE[];

/* Always inline to mitigate against single instruction glitch skipping a branch instruction */
__attribute__((always_inline)) static inline void we_protect_preprovisioned_data(void)
{
    /* Verify sticky enable for main flash is enabled */
    SAFE_IF((HWREG(FLASH_BASE + FLASH_O_CFG) & FLASH_CFG_MAIN_STICKY_EN) != FLASH_CFG_MAIN_STICKY_EN)
    {
        /* Cold reset of entire chip causing boot code to run again */
        SysCtrlSystemReset();
        /* Spin forever if reset did not get executed */
        while (1) {}
    }

    /* Pre-provisioned data is stored in main flash sector 0. Write sector 0
     * write & erase protection bit to ensure it is set. Writing zeros to the WE
     * protect register has no effect because the sticky enable is set for main
     * flash protection. */
    verified_reg_write(FLASH_BASE + FLASH_O_WEPROT_B0_31_0_BY1, FLASH_WEPROT_SECTOR_0, FLASH_WEPROT_SECTOR_0);
}

enum tfm_plat_err_t enable_fault_handlers(void)
{
    /* Explicitly set secure fault priority to the highest */
    NVIC_SetPriority(SecureFault_IRQn, 0);

    /* Enables Bus, Memory, Usage and Secure faults */
    SCB->SHCSR |= (SCB_SHCSR_USGFAULTENA_Msk | SCB_SHCSR_BUSFAULTENA_Msk | SCB_SHCSR_MEMFAULTENA_Msk |
                   SCB_SHCSR_SECUREFAULTENA_Msk);

    return TFM_PLAT_ERR_SUCCESS;
}

enum tfm_plat_err_t system_reset_cfg(void)
{
    uint32_t reg_value = SCB->AIRCR;

    /* Clear SCB_AIRCR_VECTKEY bits */
    reg_value &= ~(uint32_t)(SCB_AIRCR_VECTKEY_Msk);

    /* Enable system reset request only to the secure world */
    reg_value |= (uint32_t)(SCB_AIRCR_WRITE_MASK | SCB_AIRCR_SYSRESETREQS_Msk);
    SCB->AIRCR = reg_value;

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

    /* Ensure pre-provisioned data in flash cannot be written or erased */
    we_protect_preprovisioned_data();

    return TFM_PLAT_ERR_SUCCESS;
}

enum tfm_plat_err_t init_debug(void)
{
    /* Nothing to do since CC26x4 Boot ROM handles locking of debug as specified
     * in the CCFG */
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

#define PRCM_CPULOCK_ALL_LOCKS_M                                                                           \
    (PRCM_CPULOCK_LOCKNSVTOR | PRCM_CPULOCK_LOCKSVTAIRCR | PRCM_CPULOCK_LOCKSAU | PRCM_CPULOCK_LOCKNSMPU | \
     PRCM_CPULOCK_LOCKSMPU)

/*------------------- SAU/IDAU configuration functions -----------------------*/
FIH_RET_TYPE(int32_t) sau_and_idau_cfg(void)
{
    /* Clear security config valid */
    verified_reg_write(PRCM_SEC_NONBUF_BASE + PRCM_O_BUSSECCFG,
                       PRCM_BUSSECCFG_BUS_CFG_M,
                       PRCM_BUSSECCFG_BUS_CFG_M | PRCM_BUSSECCFG_VALID);

    /* Clear all CPU lock bits and leave everything unlocked */
    verified_reg_write(PRCM_BASE + PRCM_O_CPULOCK, 0, PRCM_CPULOCK_ALL_LOCKS_M);

    /* Configure IDAU NSC Flash base address */
    verified_reg_write(PRCM_BASE + PRCM_O_NVMNSCADDR,
                       memory_regions.veneer_base & PRCM_NVMNSCADDR_BOUNDARY_M,
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

    /*** Define SAU regions so that the S-image trailer is marked secure ***/
    /* Define SAU region 0 as NSC for veneers using their actual size */
    SAU->RNR  = 0U;
    SAU->RBAR = (memory_regions.veneer_base & SAU_RBAR_BADDR_Msk);
    SAU->RLAR = (memory_regions.veneer_limit & SAU_RLAR_LADDR_Msk) | SAU_RLAR_ENABLE_Msk | SAU_RLAR_NSC_Msk;

    /* Define SAU region 1 as NS from the start of NS flash to the end of the NS peripherals */
    SAU->RNR  = 1U;
    SAU->RBAR = (NS_ROM_ALIAS_BASE & SAU_RBAR_BADDR_Msk);
    SAU->RLAR = (NS_PERIPHERAL_REGION_ALIAS_END & SAU_RLAR_LADDR_Msk) | SAU_RLAR_ENABLE_Msk;

    /* Disable all unused SAU regions (enable bit for each region is undefined
     * after a reset) */
    SAU->RNR  = 2U;
    SAU->RLAR = 0U;
    SAU->RNR  = 3U;
    SAU->RLAR = 0U;

    /* Enable SAU */
    TZ_SAU_Enable();

    /* Configure secure VTOR. VTOR is normally configured by SystemInit() but
     * since the PRCM CPULOCK.LOCKSVTAIRCR is still set when SystemInit() is
     * called, the register write has no effect so we must configure VTOR again
     * here after the lock bit has been cleared. */
#if defined (__VTOR_PRESENT) && (__VTOR_PRESENT == 1U)
    SCB->VTOR = (uint32_t) &(__VECTOR_TABLE[0]);
#endif

    FIH_RET(fih_int_encode(ARM_DRIVER_OK));
}
