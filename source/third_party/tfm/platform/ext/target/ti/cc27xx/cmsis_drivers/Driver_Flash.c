/*
 * Copyright (c) 2013-2012 ARM Limited. All rights reserved.
 * Copyright (c) 2024, Texas Instruments Incorporated. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include "Driver_Flash.h"
#include "flash_layout.h"

#include <ti/drivers/dpl/HwiP.h>

#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(driverlib/flash.h) /* FAPI status codes */
#include DeviceFamily_constructPath(driverlib/hapi.h) /* HAPI functions */

#ifndef ARG_UNUSED
#define ARG_UNUSED(arg)  ((void)arg)
#endif

/* Driver version */
#define ARM_FLASH_DRV_VERSION ARM_DRIVER_VERSION_MAJOR_MINOR(1, 0)

#define ARM_FLASH_DRV_ERASE_BYTE 0xFF

/*
 * ARM FLASH device structure
 */
struct arm_flash_dev_t {
    const uint32_t memory_base;   /*!< FLASH memory base address */
    ARM_FLASH_INFO *data;         /*!< FLASH data */
};

/* Flash Status */
static ARM_FLASH_STATUS FlashStatus = {0, 0, 0};

/* Driver Version */
static const ARM_DRIVER_VERSION DriverVersion = {
    ARM_FLASH_API_VERSION,
    ARM_FLASH_DRV_VERSION
};

/**
  * \brief Flash driver capability macro definitions \ref ARM_FLASH_CAPABILITIES
  */
/* Flash Ready event generation capability values */
#define EVENT_READY_NOT_AVAILABLE   (0u)
#define EVENT_READY_AVAILABLE       (1u)
/* Chip erase capability values */
#define CHIP_ERASE_NOT_SUPPORTED    (0u)
#define CHIP_ERASE_SUPPORTED        (1u)

/**
 * Data width values for ARM_FLASH_CAPABILITIES::data_width
 * \ref ARM_FLASH_CAPABILITIES
 */
 enum {
    DATA_WIDTH_8BIT   = 0u,
    DATA_WIDTH_16BIT,
    DATA_WIDTH_32BIT,
    DATA_WIDTH_ENUM_SIZE
};

static const uint32_t data_width_byte[DATA_WIDTH_ENUM_SIZE] = {
    sizeof(uint8_t),
    sizeof(uint16_t),
    sizeof(uint32_t),
};

/* Driver Capabilities */
static const ARM_FLASH_CAPABILITIES DriverCapabilities =
{
    EVENT_READY_NOT_AVAILABLE,
    DATA_WIDTH_8BIT,
    CHIP_ERASE_SUPPORTED
};

static ARM_FLASH_INFO ARM_FLASH0_DEV_DATA =
{
    .sector_info  = NULL,  /* Uniform sector layout */
    .sector_count = FLASH0_SIZE / FLASH0_SECTOR_SIZE,
    .sector_size  = FLASH0_SECTOR_SIZE,
    .page_size    = FLASH0_PAGE_SIZE,
    .program_unit = FLASH0_PROGRAM_UNIT,
    .erased_value = ARM_FLASH_DRV_ERASE_BYTE
};

static struct arm_flash_dev_t ARM_FLASH0_DEV =
{
    .memory_base = FLASH0_BASE_S,
    .data        = &(ARM_FLASH0_DEV_DATA)
};

struct arm_flash_dev_t *FLASH0_DEV = &ARM_FLASH0_DEV;

/*
 * Functions
 */

/**
  * \brief      Checks if the Flash memory range is valid
  * \param[in]  flash_dev  Flash device structure \ref arm_flash_dev_t
  * \param[in]  offset     Highest Flash memory address which would be accessed
  * \return     True if Flash memory boundaries are not violated, false
  *             otherwise.
  */
static bool isFlashRangeValid(struct arm_flash_dev_t *flash_dev, uint32_t offset)
{
    bool isValid = true;
    uint32_t flash_limit;

    flash_limit = (flash_dev->data->sector_count * flash_dev->data->sector_size) - 1;

    if (offset > flash_limit) {
        isValid = false;
    }

    return isValid;
}

/**
  * \brief      Translates Flash API (FAPI) status into ARM driver status code
  * \param[in]  fapiStatus FAPI status code
  * \return     ARM driver status code.
  */
static int32_t translateFAPIStatus(uint32_t fapiStatus)
{
    int32_t status;

    switch (fapiStatus) {
        case FAPI_STATUS_SUCCESS:
            status = ARM_DRIVER_OK;
            break;

        case FAPI_STATUS_FSM_ERROR:
            status = ARM_DRIVER_ERROR;
            break;

        default:
            status = ARM_DRIVER_ERROR_PARAMETER;
            break;
    }

    return status;
}

static ARM_DRIVER_VERSION ARM_Flash_GetVersion(void)
{
    return DriverVersion;
}

static ARM_FLASH_CAPABILITIES ARM_Flash_GetCapabilities(void)
{
    return DriverCapabilities;
}

static int32_t ARM_Flash_Initialize(ARM_Flash_SignalEvent_t cb_event)
{
    ARG_UNUSED(cb_event);

    if (DriverCapabilities.data_width >= DATA_WIDTH_ENUM_SIZE) {
        return ARM_DRIVER_ERROR;
    }

    /* Nothing to be done */
    return ARM_DRIVER_OK;
}

static int32_t ARM_Flash_Uninitialize(void)
{
    /* Nothing to be done */
    return ARM_DRIVER_OK;
}

static int32_t ARM_Flash_PowerControl(ARM_POWER_STATE state)
{
    int32_t status;

    switch (state) {
        case ARM_POWER_FULL:
            /* Nothing to be done */
            status = ARM_DRIVER_OK;
            break;

        case ARM_POWER_OFF: /* Fall through */
        case ARM_POWER_LOW: /* Fall through */
        default:
            status = ARM_DRIVER_ERROR_UNSUPPORTED;
            break;
    }

    return status;
}

static int32_t ARM_Flash_ReadData(uint32_t addr, void *data, uint32_t cnt)
{
    /* Conversion between data items and bytes */
    cnt *= data_width_byte[DriverCapabilities.data_width];

    /* Check flash memory boundaries */
    if (!isFlashRangeValid(FLASH0_DEV, addr + cnt - 1)) {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    (void)memcpy(data, (void *)addr, cnt);

    /* Conversion between bytes and data items */
    cnt /= data_width_byte[DriverCapabilities.data_width];

    return cnt;
}

static int32_t ARM_Flash_ProgramData(uint32_t addr, const void *data, uint32_t cnt)
{
    uint32_t fapi_status;
    uint32_t status;
    uintptr_t key;

    /* Conversion between data items and bytes */
    cnt *= data_width_byte[DriverCapabilities.data_width];

    /* CMSIS -> FAPI -> HAPI (Secure ROM)
     * CC27xx FlashProgram handles VIMS cache and line buffer disable/restore
     * automatically. The underlying Flash API performs write verification so it
     * is not necessary to check that the flash area is erased prior to
     * programming.
     */
    key = HwiP_disable();
    fapi_status = FlashProgram((uint8_t *)data, addr, cnt);
    HwiP_restore(key);

    status = translateFAPIStatus(fapi_status);

    if (status != ARM_DRIVER_OK) {
        return status;
    }

    /* Conversion between bytes and data items */
    cnt /= data_width_byte[DriverCapabilities.data_width];

    return cnt;
}

static int32_t ARM_Flash_EraseSector(uint32_t addr)
{
    uint32_t status;
    uintptr_t key;

    /* CMSIS -> FAPI -> HAPI (Secure ROM)
     * CC27xx FlashEraseSector handles VIMS cache and line buffer disable/restore
     * automatically.
     */
    key = HwiP_disable();
    status = FlashEraseSector(addr);
    HwiP_restore(key);

    return translateFAPIStatus(status);
}

static int32_t ARM_Flash_EraseChip(void)
{
    /* Erasing the entire on-chip flash cannot be done while executing from it */
    return ARM_DRIVER_ERROR_UNSUPPORTED;
}

static ARM_FLASH_STATUS ARM_Flash_GetStatus(void)
{
    return FlashStatus;
}

static ARM_FLASH_INFO * ARM_Flash_GetInfo(void)
{
    return FLASH0_DEV->data;
}

ARM_DRIVER_FLASH Driver_FLASH0 = {
    ARM_Flash_GetVersion,
    ARM_Flash_GetCapabilities,
    ARM_Flash_Initialize,
    ARM_Flash_Uninitialize,
    ARM_Flash_PowerControl,
    ARM_Flash_ReadData,
    ARM_Flash_ProgramData,
    ARM_Flash_EraseSector,
    ARM_Flash_EraseChip,
    ARM_Flash_GetStatus,
    ARM_Flash_GetInfo
};
