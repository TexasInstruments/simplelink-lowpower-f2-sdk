/*
 * Copyright (c) 2017-2022, ARM Limited. All rights reserved.
 * Copyright (c) 2024, Texas Instruments Incorporated. All rights reserved.
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

/* TI CC27xx SDK include(s) */
//#include "ti/devices/cc27xx/driverlib/aon_rtc.h"
#include "ti/devices/cc27xx/driverlib/setup.h"   /* SetupTrimDevice() */
#include "ti/devices/cc27xx/driverlib/hapi.h"
#include "ti/devices/cc27xx/inc/hw_memmap.h"
#include "ti/devices/cc27xx/inc/hw_tcm.h"
#include "ti/devices/cc27xx/inc/hw_types.h"      /* HWREG() for verified_reg_write() */
#include "ti/devices/cc27xx/inc/hw_vims.h"

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

/* To write into AIRCR register, 0x5FA value must be write to the VECTKEY field,
 * otherwise the processor ignores the write.
 */
#define SCB_AIRCR_WRITE_MASK ((0x5FAUL << SCB_AIRCR_VECTKEY_Pos))

/* VIMS flash write/erase protect sector 0 */
#define VIMS_WEPRA_SECTOR_0 0xFFFFFFFEU

/* Dummy platform data as this is not yet supported */
struct platform_data_t tfm_peripheral_std_uart = {0, 0};

/* Exception / Interrupt Vector table */
extern const VECTOR_TABLE_Type __VECTOR_TABLE[];

/* Forward declarations */
static uint32_t calc_odd_parity(uint32_t val, uint_fast8_t startBit, uint_fast8_t endBit);
static uint32_t tcm_reg_parity(uint32_t val);

/* Always inline to mitigate against single instruction glitch skipping a branch instruction */
__attribute__((always_inline)) static inline void we_protect_preprovisioned_data(void)
{
    /* Pre-provisioned data is stored in main flash sector 0. Clear sector 0
     * write & erase protection bit to ensure it is protected. Writing ones to
     * the WE protect register has no effect because the bits are sticky 0.
     */
    verified_reg_write(VIMS_BASE + VIMS_O_WEPRA, VIMS_WEPRA_SECTOR_0, VIMS_WEPRA_SECTOR_0);
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

#if 0  // TODO: Enable these functions once driverlib support is added if not already handled by the system partition
    /* Disable RTC timer */
    AONRTCDisable();

    /* Reset RTC timer */
    AONRTCReset();

    /* Sync AON interface */
    SysCtrlAonUpdate();

    /* Start RTC timer */
    AONRTCEnable();
#endif

    /* Ensure pre-provisioned data in flash cannot be written or erased */
    we_protect_preprovisioned_data();

    return TFM_PLAT_ERR_SUCCESS;
}

enum tfm_plat_err_t init_debug(void)
{
    /* Nothing to do since CC27xx Boot ROM handles locking of debug as specified
     * in the CCFG.
     */
    return TFM_PLAT_ERR_SUCCESS;
}

/*----------------- NVIC interrupt target state to NS configuration ----------*/
enum tfm_plat_err_t nvic_interrupt_target_state_cfg(void)
{
    /* Target every interrupt to NS; unimplemented interrupts will be WI */
    for (uint8_t i = 0U; i < (sizeof(NVIC->ITNS) / sizeof(NVIC->ITNS[0])); i++)
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

/* Returns the odd parity value for the input value bit range specified */
static uint32_t calc_odd_parity(uint32_t val, uint_fast8_t startBit, uint_fast8_t endBit)
{
    uint_fast8_t i;
    uint32_t parity = 0U;

    for (i = startBit; i <= endBit; i++)
    {
        parity ^= (val >> i) & 0x1U;
    }

    return ~parity & 0x1U;
}

/* Returns the register value with parity bits populated */
static uint32_t tcm_reg_parity(uint32_t val)
{
    uint32_t regVal = val;

    /* Clear parity bits */
    regVal &= ~TCM_GSKEN0_PAR_M;

    /* Calculate parity bits.
     * - Bit 29 stores the odd parity for bits[7:0].
     * - Bit 30 stores the odd parity of bits[15:8].
     * - Bit 31 stores the odd parity of bits[23:16].
     */
    regVal |= calc_odd_parity(val, 0U, 7U) << 29U;
    regVal |= calc_odd_parity(val, 8U, 15U) << 30U;
    regVal |= calc_odd_parity(val, 16U, 23U) << 31U;

    return regVal;
}

/* Non-secure memory address space bit */
#define NS_ADDR_SPACE_BIT28 (0x1U << 28U)

#if ((NS_ROM_ALIAS_BASE & 0x1FFF) != 0)
    #error "NS ROM base must be 8KB aligned for TCM Flash watermark resolution"
#elif ((NS_ROM_ALIAS_BASE & NS_ADDR_SPACE_BIT28) == 0)
    #error "NS ROM base must have bit 28 set"
#elif ((NS_ROM_ALIAS_LIMIT & NS_ADDR_SPACE_BIT28) == 0)
    #error "NS ROM limit must have bit 28 set"
#endif

#if ((NS_RAM_ALIAS_BASE & 0x3FF) != 0)
    #error "NS RAM base must be 1KB aligned for TCM SRAM watermark resolution"
#elif ((NS_RAM_ALIAS_BASE & NS_ADDR_SPACE_BIT28) == 0)
    #error "NS RAM base must have bit 28 set"
#elif ((NS_RAM_ALIAS_LIMIT & NS_ADDR_SPACE_BIT28) == 0)
    #error "NS RAM limit must have bit 28 set"
#endif

#define GASKET_ENABLE_0  0x00E80084U /* CLKCTRL, MICPGA, MICADC, EVTSVT, AFA, HSM secure */
#define GASKET_ENABLE_1  0x0001FF7EU /* All secure except LGPT and IOC */

/*------------------- SAU/IDAU configuration functions -----------------------*/
FIH_RET_TYPE(int32_t) sau_and_idau_cfg(void)
{
    uint32_t wmcfg;

    /**************************************************************************
     * Configure IDAU: TrustZone security Control Module (TCM)
     **************************************************************************
     */

    /* Call ROM code to clear TCM Sync bit before modifying other TCM regs */
    if (!HapiSetTcmSync(false))
    {
        /* Reset if ROM code fails to clear Sync bit */
        HapiResetDevice();
    }

    /* Configure TrustZone security Control Module (TCM) SRAM and VIMS
     * watermark. Watermark resolution is 8KB for VIMS flash and 1KB for SRAM.
     * By design, the first 8KB in flash and 1KB in SRAM are always secure so
     * those values must be subtracted from the desired flash and SRAM watermark
     * addresses. Memory below the watermark address is designated as Non-Secure
     * Callable (NSC). Secure memory that is not intended to be NSC must be
     * designated using the SAU.
     */
    wmcfg = ((NS_ROM_ALIAS_BASE - 0x2000U) >> 13U) & TCM_WMCFG_VIMSWM_M;
    wmcfg |= (((NS_RAM_ALIAS_BASE - 0x400U) >> 10U) << TCM_WMCFG_SRAMWM_S) & TCM_WMCFG_SRAMWM_M;
    verified_reg_write(TCM_BASE + TCM_O_WMCFG,
                       tcm_reg_parity(wmcfg),
                       0xFFFFFFFFU);

    /* Configure TrustZone security Control Module (TCM) gaskets */
    verified_reg_write(TCM_BASE + TCM_O_GSKEN0,
                       tcm_reg_parity(GASKET_ENABLE_0),
                       0xFFFFFFFFU);

    verified_reg_write(TCM_BASE + TCM_O_GSKEN1,
                       tcm_reg_parity(GASKET_ENABLE_1),
                       0xFFFFFFFFU);

    /* Call ROM code to set TCM Sync bit for TCM config to take effect */
    if (!HapiSetTcmSync(true))
    {
        /* Reset if ROM code fails to set Sync bit */
        HapiResetDevice();
    }

    /* Disable TCM write access */
    verified_reg_write(TCM_BASE + TCM_O_REGWEN,
                       tcm_reg_parity(TCM_REGWEN_SYNC | TCM_REGWEN_WEN_DIS),
                       0x7FFFFFFFU); /* Ignore Bit 31 as it always reads 0 */

    /**************************************************************************
     * Configure SAU regions so that the S-image, S-image trailer, and Secure
     * SRAM are designated as Secure. When the SAU is enabled, memory that is
     * not covered by an enabled SAU region is designated as Secure.
     **************************************************************************
     */

    /* Define SAU region 0 as NSC for veneers using their actual size */
    SAU->RNR  = 0U;
    SAU->RBAR = memory_regions.veneer_base & SAU_RBAR_BADDR_Msk;
    SAU->RLAR = (memory_regions.veneer_limit & SAU_RLAR_LADDR_Msk) | SAU_RLAR_ENABLE_Msk | SAU_RLAR_NSC_Msk;

    /* Define SAU region 1 as NS from the start of NS flash to the end of the NS
     * flash.
     */
    SAU->RNR  = 1U;
    SAU->RBAR = NS_ROM_ALIAS_BASE & SAU_RBAR_BADDR_Msk;
    SAU->RLAR = (NS_ROM_ALIAS_LIMIT & SAU_RLAR_LADDR_Msk) | SAU_RLAR_ENABLE_Msk;

    /* Define SAU region 2 as NS from the start of NS RAM to the end of the
     * address space. Within the memory-mapped peripheral address range, secure
     * peripherals will be designated as NSC memory by the TCM gaskets.
     */
    SAU->RNR  = 2U;
    SAU->RBAR = NS_RAM_ALIAS_BASE & SAU_RBAR_BADDR_Msk;
    SAU->RLAR = (0xFFFFFFFFUL & SAU_RLAR_LADDR_Msk) | SAU_RLAR_ENABLE_Msk;

    /* Disable all unused SAU regions (enable bit for each region is undefined
     * after a reset).
     */
    SAU->RNR  = 3U;
    SAU->RLAR = 0U;
    SAU->RNR  = 4U;
    SAU->RLAR = 0U;
    SAU->RNR  = 5U;
    SAU->RLAR = 0U;
    SAU->RNR  = 6U;
    SAU->RLAR = 0U;
    SAU->RNR  = 7U;
    SAU->RLAR = 0U;

    /* Enable SAU */
    TZ_SAU_Enable();

    /* Configure secure VTOR. VTOR is normally configured by SystemInit() but
     * since the PRCM CPULOCK.LOCKSVTAIRCR is still set when SystemInit() is
     * called, the register write has no effect so we must configure VTOR again
     * here after the lock bit has been cleared.
     */
#if defined (__VTOR_PRESENT) && (__VTOR_PRESENT == 1U)
    SCB->VTOR = (uint32_t) &(__VECTOR_TABLE[0]);
#endif

    FIH_RET(fih_int_encode(ARM_DRIVER_OK));
}
