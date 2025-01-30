/*
 * Copyright (c) 2017-2022 Arm Limited. All rights reserved.
 * Copyright (c) 2020 Nuvoton Technology Corp. All rights reserved.
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

#ifndef __FLASH_LAYOUT_H__
#define __FLASH_LAYOUT_H__

/* Flash layout on M2354 with BL2 (single image boot):
 *
 * 0x0000_0000 BL2 - MCUBoot (64KB)
 * 0x0001_0000 Protected Storage Area (28 KB)
 * 0x0000_7000 Internal Trusted Storage Area (28 KB)
 * 0x0000_E000 OTP / NV counters area (8 KB)
 * 0x0002_0000 Primary image area (320KB):
 *    0x0002_0000 Secure     image primary
 *    0x0007_0000 Non-secure image primary
 * 0x0010_0000 Scratch area (4KB)
 *
 */

/* This header file is included from linker scatter file as well, where only a
 * limited C constructs are allowed. Therefore it is not possible to include
 * here the platform_retarget.h to access flash related defines. To resolve this
 * some of the values are redefined here with different names, these are marked
 * with comment.
 */

/* Sector size of the flash hardware; same as FLASH0_SECTOR_SIZE */
#define FLASH_AREA_IMAGE_SECTOR_SIZE    (0x800)   /* 2 KB */
/* Same as FLASH0_SIZE */
#define FLASH_TOTAL_SIZE                (0x00100000)

/* Flash layout info for BL2 bootloader */
/* Same as FLASH0_BASE_S */
#define FLASH_BASE_ADDRESS              (0x00000000)

/* Offset and size definitions of the flash partitions that are handled by the
 * bootloader. The image swapping is done between IMAGE_PRIMARY and
 * IMAGE_SECONDARY, SCRATCH is used as a temporary storage during image
 * swapping.
 */
#define FLASH_AREA_BL2_OFFSET      (0x0)
#define FLASH_AREA_BL2_SIZE        (0x10000) /* 64 KB */

#if !defined(MCUBOOT_IMAGE_NUMBER) || (MCUBOOT_IMAGE_NUMBER == 1)
/* Size of a Secure and of a Non-secure image */
#define FLASH_S_PARTITION_SIZE          (0x60000)
#define FLASH_NS_PARTITION_SIZE         (0x40000)

#if (FLASH_S_PARTITION_SIZE > FLASH_NS_PARTITION_SIZE)
#define FLASH_MAX_PARTITION_SIZE FLASH_S_PARTITION_SIZE
#else
#define FLASH_MAX_PARTITION_SIZE FLASH_NS_PARTITION_SIZE
#endif


/* Secure + Non-secure image primary slot */
#define FLASH_AREA_0_ID            (1)
#define FLASH_AREA_0_OFFSET        (0x20000)
#define FLASH_AREA_0_SIZE          (FLASH_S_PARTITION_SIZE + FLASH_NS_PARTITION_SIZE)
/* Secure + Non-secure secondary slot */
#define FLASH_AREA_2_ID            (FLASH_AREA_0_ID + 1)
#define FLASH_AREA_2_OFFSET        (0x80000)
#define FLASH_AREA_2_SIZE          (0x80000)

/* Scratch area */
#define FLASH_AREA_SCRATCH_ID      (FLASH_AREA_2_ID + 1)
#define FLASH_AREA_SCRATCH_OFFSET  (0x100800)
#define FLASH_AREA_SCRATCH_SIZE    (0x800)

/* The maximum number of status entries supported by the bootloader. */
#define MCUBOOT_STATUS_MAX_ENTRIES ((FLASH_S_PARTITION_SIZE + \
                                     FLASH_NS_PARTITION_SIZE) / \
                                    FLASH_AREA_SCRATCH_SIZE)

/* Maximum number of image sectors supported by the bootloader. */
#define MCUBOOT_MAX_IMG_SECTORS    ((FLASH_S_PARTITION_SIZE + \
                                     FLASH_NS_PARTITION_SIZE) / \
                                    FLASH_AREA_IMAGE_SECTOR_SIZE)

#elif (MCUBOOT_IMAGE_NUMBER == 2)

/* Size of a Secure and of a Non-secure image */
#define FLASH_S_PARTITION_SIZE          (0x40000)
#define FLASH_NS_PARTITION_SIZE         (0x40000)

#if (FLASH_S_PARTITION_SIZE > FLASH_NS_PARTITION_SIZE)
#define FLASH_MAX_PARTITION_SIZE FLASH_S_PARTITION_SIZE
#else
#define FLASH_MAX_PARTITION_SIZE FLASH_NS_PARTITION_SIZE
#endif


 /* Secure image primary slot */
#define FLASH_AREA_0_ID            (1)
#define FLASH_AREA_0_OFFSET        (0x20000)
#define FLASH_AREA_0_SIZE          (FLASH_S_PARTITION_SIZE)
/* Non-secure image primary slot */
#define FLASH_AREA_1_ID            (FLASH_AREA_0_ID + 1)
#define FLASH_AREA_1_OFFSET        (0x80000)
#define FLASH_AREA_1_SIZE          (FLASH_NS_PARTITION_SIZE)
/* Secure image secondary slot */
#define FLASH_AREA_2_ID            (FLASH_AREA_1_ID + 1)
#define FLASH_AREA_2_OFFSET        (FLASH_AREA_0_OFFSET+FLASH_AREA_0_SIZE)
#define FLASH_AREA_2_SIZE          (0x20000)
/* Non-secure image secondary slot */
#define FLASH_AREA_3_ID            (FLASH_AREA_2_ID + 1)
#define FLASH_AREA_3_OFFSET        (FLASH_AREA_1_OFFSET+FLASH_AREA_1_SIZE)
#define FLASH_AREA_3_SIZE          (0x20000)
/* Scratch area */
#define FLASH_AREA_SCRATCH_ID      (FLASH_AREA_3_ID + 1)
#define FLASH_AREA_SCRATCH_OFFSET  (0x100800)
#define FLASH_AREA_SCRATCH_SIZE    (0x800)
/* The maximum number of status entries supported by the bootloader. */
#define MCUBOOT_STATUS_MAX_ENTRIES ((FLASH_S_PARTITION_SIZE + \
                                     FLASH_NS_PARTITION_SIZE) / \
                                    FLASH_AREA_SCRATCH_SIZE)
/* Maximum number of image sectors supported by the bootloader. */
#define MCUBOOT_MAX_IMG_SECTORS    ((FLASH_S_PARTITION_SIZE + \
                                     FLASH_NS_PARTITION_SIZE) / \
                                    FLASH_AREA_IMAGE_SECTOR_SIZE)
#else /* MCUBOOT_IMAGE_NUMBER > 2 */
#error "Only MCUBOOT_IMAGE_NUMBER 1 and 2 are supported!"
#endif /* MCUBOOT_IMAGE_NUMBER */

/* Protected Storage (PS) Service definitions */
#define FLASH_PS_AREA_OFFSET            (0x10000)
#define FLASH_PS_AREA_SIZE              (0x5000)

/* Internal Trusted Storage (ITS) Service definitions */
#define FLASH_ITS_AREA_OFFSET           (FLASH_PS_AREA_OFFSET + FLASH_PS_AREA_SIZE)
#define FLASH_ITS_AREA_SIZE             (0x5000)

/* OTP_definitions */
#define FLASH_OTP_NV_COUNTERS_AREA_OFFSET (FLASH_ITS_AREA_OFFSET + FLASH_ITS_AREA_SIZE)
#define FLASH_OTP_NV_COUNTERS_AREA_SIZE   (FLASH_AREA_IMAGE_SECTOR_SIZE * 2)
#define FLASH_OTP_NV_COUNTERS_SECTOR_SIZE FLASH_AREA_IMAGE_SECTOR_SIZE

/* Offset and size definition in flash area used by assemble.py */
#define SECURE_IMAGE_OFFSET             (0x0)
#define SECURE_IMAGE_MAX_SIZE           FLASH_S_PARTITION_SIZE

#define NON_SECURE_IMAGE_OFFSET         (SECURE_IMAGE_OFFSET + SECURE_IMAGE_MAX_SIZE)
#define NON_SECURE_IMAGE_MAX_SIZE       FLASH_NS_PARTITION_SIZE

/* Flash device name used by BL2
 * Name is defined in flash driver file: Driver_Flash.c
 */
#define FLASH_DEV_NAME Driver_FLASH0
/* Smallest flash programmable unit in bytes */
#define TFM_HAL_FLASH_PROGRAM_UNIT       (0x4)

/* Protected Storage (PS) Service definitions
 * Note: Further documentation of these definitions can be found in the
 * TF-M PS Integration Guide.
 */
#define TFM_HAL_PS_FLASH_DRIVER Driver_FLASH0

/* In this target the CMSIS driver requires only the offset from the base
 * address instead of the full memory address.
 */
#define TFM_HAL_PS_FLASH_AREA_ADDR     FLASH_PS_AREA_OFFSET
/* Dedicated flash area for PS */
#define TFM_HAL_PS_FLASH_AREA_SIZE     FLASH_PS_AREA_SIZE
#define PS_SECTOR_SIZE         FLASH_AREA_IMAGE_SECTOR_SIZE
/* Number of PS_SECTOR_SIZE per block */
#define TFM_HAL_PS_SECTORS_PER_BLOCK   (0x1)
/* Specifies the smallest flash programmable unit in bytes */
#define TFM_HAL_PS_PROGRAM_UNIT  (0x4)

/* Internal Trusted Storage (ITS) Service definitions
 * Note: Further documentation of these definitions can be found in the
 * TF-M ITS Integration Guide. The ITS should be in the internal flash, but is
 * allocated in the external flash just for development platforms that don't
 * have internal flash available.
 */
#define TFM_HAL_ITS_FLASH_DRIVER Driver_FLASH0

/* In this target the CMSIS driver requires only the offset from the base
 * address instead of the full memory address.
 */
#define TFM_HAL_ITS_FLASH_AREA_ADDR     FLASH_ITS_AREA_OFFSET
/* Dedicated flash area for ITS */
#define TFM_HAL_ITS_FLASH_AREA_SIZE     FLASH_ITS_AREA_SIZE
#define ITS_SECTOR_SIZE         FLASH_AREA_IMAGE_SECTOR_SIZE
/* Number of ITS_SECTOR_SIZE per block */
#define TFM_HAL_ITS_SECTORS_PER_BLOCK   (0x1)
/* Specifies the smallest flash programmable unit in bytes */
#define TFM_HAL_ITS_PROGRAM_UNIT  (0x4)

/* OTP / NV counter definitions */
#define TFM_OTP_NV_COUNTERS_AREA_SIZE   (FLASH_OTP_NV_COUNTERS_AREA_SIZE / 2)
#define TFM_OTP_NV_COUNTERS_AREA_ADDR   FLASH_OTP_NV_COUNTERS_AREA_OFFSET
#define TFM_OTP_NV_COUNTERS_SECTOR_SIZE FLASH_OTP_NV_COUNTERS_SECTOR_SIZE
#define TFM_OTP_NV_COUNTERS_BACKUP_AREA_ADDR (TFM_OTP_NV_COUNTERS_AREA_ADDR + \
                                              TFM_OTP_NV_COUNTERS_AREA_SIZE)

/* Use SRAM1 memory to store Code data */
#define S_ROM_ALIAS_BASE  (0x00000000)
#define NS_ROM_ALIAS_BASE (0x10000000)

/* FIXME: Use SRAM2 memory to store RW data */
#define S_RAM_ALIAS_BASE  (0x20000000)
#define NS_RAM_ALIAS_BASE (0x30000000)

#define TOTAL_ROM_SIZE FLASH_TOTAL_SIZE
#define TOTAL_RAM_SIZE (0x00040000)     /* 256 KB */

#endif /* __FLASH_LAYOUT_H__ */
