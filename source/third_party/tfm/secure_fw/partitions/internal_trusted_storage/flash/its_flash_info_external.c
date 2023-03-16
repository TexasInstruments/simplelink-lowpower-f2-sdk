/*
 * Copyright (c) 2019-2020, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "its_flash.h"

#include "Driver_Flash.h"
#include "flash_layout.h"
#include "its_utils.h"
#include "ps_object_defs.h"

#ifndef PS_FLASH_DEV_NAME
#error "PS_FLASH_DEV_NAME must be defined by the target in flash_layout.h"
#endif

#ifndef PS_FLASH_AREA_ADDR
#error "PS_FLASH_AREA_ADDR must be defined by the target in flash_layout.h"
#endif

/* Adjust to a size that will allow all assets to fit */
#ifndef PS_FLASH_AREA_SIZE
#error "PS_FLASH_AREA_SIZE must be defined by the target in flash_layout.h"
#endif

/* Adjust to match the size of the flash device's physical erase unit */
#ifndef PS_SECTOR_SIZE
#error "PS_SECTOR_SIZE must be defined by the target in flash_layout.h"
#endif

/* Adjust so that the maximum required asset size will fit in one block */
#ifndef PS_SECTORS_PER_BLOCK
#error "PS_SECTORS_PER_BLOCK must be defined by the target in flash_layout.h"
#endif

/* Adjust to match the size of the flash device's physical program unit */
#ifndef PS_FLASH_PROGRAM_UNIT
#error "PS_FLASH_PROGRAM_UNIT must be defined by the target in flash_layout.h"
#elif (PS_FLASH_PROGRAM_UNIT < 1 || PS_FLASH_PROGRAM_UNIT > PS_SECTOR_SIZE)
#error "PS_FLASH_PROGRAM_UNIT must be between 1 and PS_SECTOR_SIZE inclusive"
#elif (PS_FLASH_PROGRAM_UNIT & (PS_FLASH_PROGRAM_UNIT - 1) != 0)
#error "PS_FLASH_PROGRAM_UNIT must be a power of two"
#endif

/* Include the correct flash interface implementation */
#ifdef PS_RAM_FS
#include "its_flash_ram.h"
#define FLASH_INFO_INIT its_flash_ram_init
#define FLASH_INFO_READ its_flash_ram_read
#define FLASH_INFO_WRITE its_flash_ram_write
#define FLASH_INFO_FLUSH its_flash_ram_flush
#define FLASH_INFO_ERASE its_flash_ram_erase

#elif (PS_FLASH_PROGRAM_UNIT <= 16)
#include "its_flash_nor.h"
#define FLASH_INFO_INIT its_flash_nor_init
#define FLASH_INFO_READ its_flash_nor_read
#define FLASH_INFO_WRITE its_flash_nor_write
#define FLASH_INFO_FLUSH its_flash_nor_flush
#define FLASH_INFO_ERASE its_flash_nor_erase

/* Require each file in the filesystem to be aligned to the program unit */
#define PS_FLASH_ALIGNMENT PS_FLASH_PROGRAM_UNIT

#else
#include "its_flash_nand.h"
#define FLASH_INFO_INIT its_flash_nand_init
#define FLASH_INFO_READ its_flash_nand_read
#define FLASH_INFO_WRITE its_flash_nand_write
#define FLASH_INFO_FLUSH its_flash_nand_flush
#define FLASH_INFO_ERASE its_flash_nand_erase

/* The flash block is programmed in one shot, so no filesystem alignment is
 * required.
 */
#define PS_FLASH_ALIGNMENT 1
#endif

/* Calculate the block layout */
#define FLASH_INFO_BLOCK_SIZE (PS_SECTOR_SIZE * PS_SECTORS_PER_BLOCK)
#define FLASH_INFO_NUM_BLOCKS (PS_FLASH_AREA_SIZE / FLASH_INFO_BLOCK_SIZE)

/* Maximum file size */
#define FLASH_INFO_MAX_FILE_SIZE ITS_UTILS_ALIGN(PS_MAX_OBJECT_SIZE, \
                                                 PS_FLASH_ALIGNMENT)

/* Maximum number of files */
#define FLASH_INFO_MAX_NUM_FILES PS_MAX_NUM_OBJECTS

/* Default value of each byte in the flash when erased */
#define FLASH_INFO_ERASE_VAL 0xFFU

#ifdef PS_RAM_FS
/* Allocate a static buffer to emulate storage in RAM */
static uint8_t ps_block_data[FLASH_INFO_BLOCK_SIZE * FLASH_INFO_NUM_BLOCKS];
#define FLASH_INFO_DEV ps_block_data
#else
/* Import the CMSIS flash device driver */
extern ARM_DRIVER_FLASH PS_FLASH_DEV_NAME;
#define FLASH_INFO_DEV &PS_FLASH_DEV_NAME
#endif

const struct its_flash_info_t its_flash_info_external = {
    .init = FLASH_INFO_INIT,
    .read = FLASH_INFO_READ,
    .write = FLASH_INFO_WRITE,
    .flush = FLASH_INFO_FLUSH,
    .erase = FLASH_INFO_ERASE,
    .flash_dev = (void *)FLASH_INFO_DEV,
    .flash_area_addr = PS_FLASH_AREA_ADDR,
    .sector_size = PS_SECTOR_SIZE,
    .block_size = FLASH_INFO_BLOCK_SIZE,
    .num_blocks = FLASH_INFO_NUM_BLOCKS,
    .program_unit = PS_FLASH_ALIGNMENT,
    .max_file_size = FLASH_INFO_MAX_FILE_SIZE,
    .max_num_files = FLASH_INFO_MAX_NUM_FILES,
    .erase_val = FLASH_INFO_ERASE_VAL,
};

/* Checks at compile time that the flash device configuration is valid */
#include \
"secure_fw/partitions/internal_trusted_storage/flash_fs/its_flash_fs_check_info.h"
