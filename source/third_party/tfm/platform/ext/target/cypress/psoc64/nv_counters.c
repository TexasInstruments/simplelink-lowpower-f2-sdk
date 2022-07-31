/*
 * Copyright (c) 2018-2020, Arm Limited. All rights reserved.
 * Copyright (c) 2020, Cypress Semiconductor Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "tfm_plat_nv_counters.h"

#include <limits.h>
#include <stddef.h>
#include "Driver_Flash.h"
#include "flash_layout.h"

/* Compilation time checks to be sure the defines are well defined */
#ifndef TFM_NV_COUNTERS_AREA_SIZE
#error "TFM_NV_COUNTERS_AREA_SIZE must be defined in flash_layout.h"
#endif

#ifndef TFM_NV_COUNTERS_SECTOR_ADDR
#error "TFM_NV_COUNTERS_SECTOR_ADDR must be defined in flash_layout.h"
#endif

#ifndef TFM_NV_COUNTERS_SECTOR_SIZE
#error "TFM_NV_COUNTERS_SECTOR_SIZE must be defined in flash_layout.h"
#endif

#ifndef TFM_NV_COUNTERS_FLASH_DEV
#error "TFM_NV_COUNTERS_FLASH_DEV must be defined in flash_layout.h"
#endif

#if TFM_NV_COUNTERS_AREA_SIZE != (TFM_NV_COUNTERS_SECTOR_SIZE * 2)
/* Code currently assumes exactly two sectors */
#error "TFM_NV_COUNTERS_AREA_SIZE must be exactly twice TFM_NV_COUNTERS_SECTOR_SIZE"
#endif
/* End of compilation time checks to be sure the defines are well defined */

#define SECTOR_OFFSET    0
#define NV_COUNTER_SIZE  sizeof(uint32_t)
#define INIT_VALUE_SIZE  sizeof(uint32_t)
#define CHECKSUM_SIZE    sizeof(uint32_t)
#define NUM_NV_COUNTERS  ((TFM_NV_COUNTERS_SECTOR_SIZE - INIT_VALUE_SIZE \
                            - CHECKSUM_SIZE) / NV_COUNTER_SIZE)
#define BACKUP_ADDRESS (TFM_NV_COUNTERS_SECTOR_ADDR + \
                        TFM_NV_COUNTERS_SECTOR_SIZE)

#define NV_COUNTERS_INITIALIZED 0xC0DE0042

/**
 * \brief Struct representing the NV counter data in flash.
 */
struct nv_counters_t {
    uint32_t checksum;
    uint32_t init_value; /**< Watermark to indicate if the NV counters have been
                          *   initialised
                          */
    uint32_t counters[NUM_NV_COUNTERS]; /**< Array of NV counters */
};

/* Import the CMSIS flash device driver */
extern ARM_DRIVER_FLASH TFM_NV_COUNTERS_FLASH_DEV;

static uint32_t calc_checksum(const uint32_t *data, size_t len)
{
    uint32_t sum = 0;

    for (uint32_t i = 0; i < len/sizeof(uint32_t); i++) {
        sum ^= data[i];
    }
    return sum;
}

static bool is_valid(const struct nv_counters_t *nv_counters)
{
    return ((nv_counters->init_value == NV_COUNTERS_INITIALIZED) &&
            (!calc_checksum(&nv_counters->checksum, sizeof(*nv_counters))));
}

static void set_checksum(struct nv_counters_t *nv_counters)
{
    uint32_t sum = calc_checksum(&nv_counters->init_value,
                                 sizeof(*nv_counters)
                                  - sizeof(nv_counters->checksum));

    nv_counters->checksum = sum;
}

enum tfm_plat_err_t tfm_plat_init_nv_counter(void)
{
    int32_t  err;
    uint32_t i;
    struct nv_counters_t nv_counters;

    err = TFM_NV_COUNTERS_FLASH_DEV.Initialize(NULL);
    if (err != ARM_DRIVER_OK) {
        return TFM_PLAT_ERR_SYSTEM_ERR;
    }

    /* Read the whole sector so we can write it back to flash later */
    err = TFM_NV_COUNTERS_FLASH_DEV.ReadData(TFM_NV_COUNTERS_SECTOR_ADDR,
                                             &nv_counters,
                                             TFM_NV_COUNTERS_SECTOR_SIZE);
    if (err != ARM_DRIVER_OK) {
        return TFM_PLAT_ERR_SYSTEM_ERR;
    }

    if (is_valid(&nv_counters)) {
        return TFM_PLAT_ERR_SUCCESS;
    }

    /* Check the backup watermark */
    err = TFM_NV_COUNTERS_FLASH_DEV.ReadData(BACKUP_ADDRESS,
                                             &nv_counters,
                                             TFM_NV_COUNTERS_SECTOR_SIZE);
    if (err != ARM_DRIVER_OK) {
        return TFM_PLAT_ERR_SYSTEM_ERR;
    }

    /* Erase sector before writing to it */
    err = TFM_NV_COUNTERS_FLASH_DEV.EraseSector(TFM_NV_COUNTERS_SECTOR_ADDR);
    if (err != ARM_DRIVER_OK) {
        return TFM_PLAT_ERR_SYSTEM_ERR;
    }

    if (is_valid(&nv_counters)) {
        /* Copy from the backup to the main */
        err = TFM_NV_COUNTERS_FLASH_DEV.ProgramData(TFM_NV_COUNTERS_SECTOR_ADDR,
                                                    &nv_counters,
                                                    TFM_NV_COUNTERS_SECTOR_SIZE);
        if (err != ARM_DRIVER_OK) {
            return TFM_PLAT_ERR_SYSTEM_ERR;
        }

        return TFM_PLAT_ERR_SUCCESS;
    }

    /* Add watermark to indicate that NV counters have been initialized */
    nv_counters.init_value = NV_COUNTERS_INITIALIZED;

    /* Initialize all counters to 0 */
    for (i = 0; i < NUM_NV_COUNTERS; i++) {
        nv_counters.counters[i] = 0;
    }

    set_checksum(&nv_counters);

    /* Write the in-memory block content after modification to flash */
    err = TFM_NV_COUNTERS_FLASH_DEV.ProgramData(TFM_NV_COUNTERS_SECTOR_ADDR,
                                                &nv_counters,
                                                TFM_NV_COUNTERS_SECTOR_SIZE);
    if (err != ARM_DRIVER_OK) {
        return TFM_PLAT_ERR_SYSTEM_ERR;
    }

    return TFM_PLAT_ERR_SUCCESS;
}

enum tfm_plat_err_t tfm_plat_read_nv_counter(enum tfm_nv_counter_t counter_id,
                                             uint32_t size, uint8_t *val)
{
    int32_t  err;
    uint32_t flash_addr = TFM_NV_COUNTERS_SECTOR_ADDR
                           + offsetof(struct nv_counters_t, counters)
                           + (counter_id * NV_COUNTER_SIZE);

    if (size != NV_COUNTER_SIZE) {
        return TFM_PLAT_ERR_SYSTEM_ERR;
    }

    err = TFM_NV_COUNTERS_FLASH_DEV.ReadData(flash_addr, val, NV_COUNTER_SIZE);
    if (err != ARM_DRIVER_OK) {
        return TFM_PLAT_ERR_SYSTEM_ERR;
    }

    return TFM_PLAT_ERR_SUCCESS;
}

enum tfm_plat_err_t tfm_plat_set_nv_counter(enum tfm_nv_counter_t counter_id,
                                            uint32_t value)
{
    int32_t  err;
    struct nv_counters_t nv_counters;

    /* Read the whole sector so we can write it back to flash later */
    err = TFM_NV_COUNTERS_FLASH_DEV.ReadData(TFM_NV_COUNTERS_SECTOR_ADDR,
                                             &nv_counters,
                                             TFM_NV_COUNTERS_SECTOR_SIZE);
    if (err != ARM_DRIVER_OK) {
        return TFM_PLAT_ERR_SYSTEM_ERR;
    }

    if (value != nv_counters.counters[counter_id]) {

        if (value < nv_counters.counters[counter_id]) {
            return TFM_PLAT_ERR_INVALID_INPUT;
        }

        /* Erase backup sector */
        err = TFM_NV_COUNTERS_FLASH_DEV.EraseSector(BACKUP_ADDRESS);
        if (err != ARM_DRIVER_OK) {
            return TFM_PLAT_ERR_SYSTEM_ERR;
        }

        nv_counters.counters[counter_id] = value;

        set_checksum(&nv_counters);

        /* write sector data to backup sector */
        err = TFM_NV_COUNTERS_FLASH_DEV.ProgramData(BACKUP_ADDRESS,
                                                    &nv_counters,
                                                    TFM_NV_COUNTERS_SECTOR_SIZE);
        if (err != ARM_DRIVER_OK) {
            return TFM_PLAT_ERR_SYSTEM_ERR;
        }

        /* Erase sector before writing to it */
        err = TFM_NV_COUNTERS_FLASH_DEV.EraseSector(TFM_NV_COUNTERS_SECTOR_ADDR);
        if (err != ARM_DRIVER_OK) {
            return TFM_PLAT_ERR_SYSTEM_ERR;
        }

        /* Write the in-memory block content after modification to flash */
        err = TFM_NV_COUNTERS_FLASH_DEV.ProgramData(TFM_NV_COUNTERS_SECTOR_ADDR,
                                                    &nv_counters,
                                                    TFM_NV_COUNTERS_SECTOR_SIZE);
        if (err != ARM_DRIVER_OK) {
            return TFM_PLAT_ERR_SYSTEM_ERR;
        }
    }

    return TFM_PLAT_ERR_SUCCESS;
}

enum tfm_plat_err_t tfm_plat_increment_nv_counter(
                                           enum tfm_nv_counter_t counter_id)
{
    uint32_t security_cnt;
    enum tfm_plat_err_t err;

    err = tfm_plat_read_nv_counter(counter_id,
                                   sizeof(security_cnt),
                                   (uint8_t *)&security_cnt);
    if (err != TFM_PLAT_ERR_SUCCESS) {
        return err;
    }

    if (security_cnt == UINT32_MAX) {
        return TFM_PLAT_ERR_MAX_VALUE;
    }

    return tfm_plat_set_nv_counter(counter_id, security_cnt + 1u);
}
