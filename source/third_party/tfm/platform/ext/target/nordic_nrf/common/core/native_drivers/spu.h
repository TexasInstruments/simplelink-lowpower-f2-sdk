/*
 * Copyright (c) 2020 Nordic Semiconductor ASA. All rights reserved.
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

#ifndef __SPU_H__
#define __SPU_H__

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include <hal/nrf_spu.h>

#define SPU_LOCK_CONF_LOCKED true
#define SPU_LOCK_CONF_UNLOCKED false
#define SPU_SECURE_ATTR_SECURE true
#define SPU_SECURE_ATTR_NONSECURE false

/**
 * \brief SPU interrupt enabling
 *
 * Enable security violations outside the Cortex-M33
 * to trigger SPU interrupts.
 */
void spu_enable_interrupts(void);

/**
 * \brief SPU event clearing
 *
 * Clear SPU event registers
 */
void spu_clear_events(void);

/**
 * \brief Reset TF-M memory regions to being Secure.
 *
 * Reset all (Flash or SRAM, but excluding the regions owned by the
 * bootloader(s)) memory region permissions to be Secure and have the
 * default (Read-Write-Execute allow) access policy.
 *
 * \note region lock is not applied to allow modifying the configuration.
 */
void spu_regions_reset_unlocked_secure(void);

/**
 * \brief Configure the SPU Flash memory region
 */
void spu_regions_flash_config(uint32_t start_addr, uint32_t limit_addr, bool secure_attr,
			      uint32_t permissions, bool lock_conf);

/**
 * \brief Configure SPU SRAM memory regions
 */
void spu_regions_sram_config(uint32_t start_addr, uint32_t limit_addr, bool secure_attr,
			     uint32_t permissions, bool lock_conf);

/**
 * \brief Configure Non-Secure Callable area
 *
 * Configure a single region in Secure Flash as Non-Secure Callable
 * (NSC) area.
 *
 * \note Any Secure Entry functions, exposing secure services to the
 * Non-Secure firmware, shall be located inside this NSC area.
 *
 * If the start address of the NSC area is hard-coded, it must follow
 * the HW restrictions: The size must be a power of 2 between 32 and
 * 4096, and the end address must fall on a SPU region boundary.
 *
 * \note region lock is applied to prevent further modification during
 *  the current reset cycle.
 */
void spu_regions_flash_config_non_secure_callable(uint32_t start_addr, uint32_t limit_addr);

/**
 * \brief Restrict access to peripheral to secure
 *
 *  Configure a device peripheral to be accessible from Secure domain only.
 *
 * \param periph_base_addr peripheral base address
 *                         (must correspond to a valid peripheral ID)
 * \param periph_lock Variable indicating whether to lock peripheral security
 *
 * \note
 * - peripheral shall not be a Non-Secure only peripheral
 * - DMA transactions are configured as Secure
 */
void spu_peripheral_config_secure(uint32_t periph_base_addr, bool periph_lock);

/**
 * Configure a device peripheral to be accessible from Non-Secure domain.
 *
 * \param periph_base_addr peripheral base address
 *                         (must correspond to a valid peripheral ID)
 * \param periph_lock Variable indicating whether to lock peripheral security
 *
 * \note
 * - peripheral shall not be a Secure-only peripheral
 * - DMA transactions are configured as Non-Secure
 */
void spu_peripheral_config_non_secure(uint32_t periph_base_addr, bool periph_lock);

/**
 * Configure DPPI channels to be accessible from Non-Secure domain.
 *
 * \param channels_mask Bitmask with channels configuration.
 * \param lock_conf Variable indicating whether to lock DPPI channel security
 *
 * \note all channels are configured as Non-Secure
 */
static inline void spu_dppi_config_non_secure(uint32_t channels_mask, bool lock_conf)
{
    nrf_spu_dppi_config_set(NRF_SPU, 0, channels_mask, lock_conf);
}

/**
 * Configure GPIO pins to be accessible from Non-Secure domain.
 *
 * \param port_number GPIO Port number
 * \param gpio_mask Bitmask with gpio configuration.
 * \param lock_conf Variable indicating whether to lock GPIO port security
 *
 * \note all pins are configured as Non-Secure
 */
static inline void spu_gpio_config_non_secure(uint8_t port_number, uint32_t gpio_mask,
    bool lock_conf)
{
    nrf_spu_gpio_config_set(NRF_SPU, port_number, gpio_mask, lock_conf);
}

/**
 * \brief Return base address of a Flash SPU regions
 *
 * Get the base (lowest) address of a particular Flash SPU region
 *
 * \param region_id Valid flash SPU region ID
 *
 * \return the base address of the given flash SPU region
 */
uint32_t spu_regions_flash_get_base_address_in_region(uint32_t region_id);

/**
 * \brief Return last address of a Flash SPU regions
 *
 * Get the last (highest) address of a particular Flash SPU region
 *
 * \param region_id Valid flash SPU region ID
 *
 * \return the last address of the given flash SPU region
 */
uint32_t spu_regions_flash_get_last_address_in_region(uint32_t region_id);

/**
 * \brief Return the ID of the first Flash SPU region
 *
 * \return the first Flash region ID
 */
uint32_t spu_regions_flash_get_start_id(void);

/**
 * \brief Return the ID of the last Flash SPU region
 *
 * \return the last Flash region ID
 */
uint32_t spu_regions_flash_get_last_id(void);

/**
 * \brief Return the size of Flash SPU regions
 *
 * \return the size of Flash SPU regions
 */
uint32_t spu_regions_flash_get_region_size(void);

/**
 * \brief Return base address of a SRAM SPU regions
 *
 * Get the base (lowest) address of a particular SRAM SPU region
 *
 * \param region_id Valid SRAM SPU region ID
 *
 * \return the base address of the given SRAM SPU region
 */
uint32_t spu_regions_sram_get_base_address_in_region(uint32_t region_id);

/**
 * \brief Return last address of a SRAM SPU regions
 *
 * Get the last (highest) address of a particular SRAM SPU region
 *
 * \param region_id Valid SRAM SPU region ID
 *
 * \return the last address of the given SRAM SPU region
 */
uint32_t spu_regions_sram_get_last_address_in_region(uint32_t region_id);

/**
 * \brief Return the ID of the first SRAM SPU region
 *
 * \return the first SRAM region ID
 */
uint32_t spu_regions_sram_get_start_id(void);

/**
 * \brief Return the ID of the last SRAM SPU region
 *
 * \return the last SRAM region ID
 */
uint32_t spu_regions_sram_get_last_id(void);

/**
 * \brief Return the size of SRAM SPU regions
 *
 * \return the size of SRAM SPU regions
 */
uint32_t spu_regions_sram_get_region_size(void);

#endif
