/*
 * Copyright (c) 2019-2022, Texas Instruments Incorporated
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

/* This header file is included from linker scatter file as well, where only
 * limited C constructs are allowed. Therefore it is not possible to include
 * here the platform_retarget.h to access flash related defines. To resolve this
 * some of the values are redefined here with different names, these are marked
 * with comment.
 */
#define FLASH0_BASE_S        (0x0)
#define FLASH0_BASE_NS       NS_ROM_ALIAS_BASE
#define FLASH0_SIZE          (0x00100000)  /* 1 MB */
#define FLASH0_SECTOR_SIZE   (0x00000800)  /* 2 KB */
#define FLASH0_PAGE_SIZE     (0x00000800)  /* 2 KB */
#define FLASH0_PROGRAM_UNIT  (0x1)         /* Minimum write size */

/* Size of a Secure and of a Non-secure image */
#define FLASH_S_PARTITION_SIZE          (S_ROM_ALIAS_LIMIT - FLASH_AREA_BL2_SIZE - \
                                         FLASH_ITS_AREA_SIZE - FLASH_PS_AREA_SIZE - FLASH_NV_COUNTERS_AREA_SIZE)
#define FLASH_NS_PARTITION_SIZE         (NS_ROM_ALIAS_LIMIT - NS_ROM_ALIAS_BASE)

/* Sector size of the flash hardware */
#define FLASH_AREA_IMAGE_SECTOR_SIZE    FLASH0_SECTOR_SIZE
#define FLASH_TOTAL_SIZE                FLASH0_SIZE

/* Flash layout info for BL2 bootloader */
#define FLASH_BASE_ADDRESS              FLASH0_BASE_S

/* Offset and size definitions of the flash partitions that are handled by the
 * bootloader. The image swapping is done between IMAGE_0 and IMAGE_1, SCRATCH
 * is used as a temporary storage during image swapping.
 */
#define FLASH_AREA_BL2_OFFSET           (0x0)
#define FLASH_AREA_BL2_SIZE             (0x6800) /* 26 KB - Pre-provisioned keys + BL2 + BIM info area */

#define FLASH_AREA_0_OFFSET             (FLASH_AREA_BL2_OFFSET + FLASH_AREA_BL2_SIZE + \
                                               FLASH_ITS_AREA_SIZE + FLASH_PS_AREA_SIZE + FLASH_NV_COUNTERS_AREA_SIZE)
#define FLASH_AREA_0_SIZE               (FLASH_S_PARTITION_SIZE + \
                                               FLASH_NS_PARTITION_SIZE)

#define FLASH_AREA_1_OFFSET             (FLASH_AREA_0_OFFSET + FLASH_AREA_0_SIZE))
#define FLASH_AREA_1_SIZE               (FLASH_S_PARTITION_SIZE + \
                                               FLASH_NS_PARTITION_SIZE)

#define FLASH_AREA_IMAGE_SCRATCH_OFFSET (FLASH_AREA_1_OFFSET + FLASH_AREA_1_SIZE))
#define FLASH_AREA_IMAGE_SCRATCH_SIZE   (FLASH_S_PARTITION_SIZE + \
                                         FLASH_NS_PARTITION_SIZE)

/* Maximum number of status entries supported by the bootloader. */
#define BOOT_STATUS_MAX_ENTRIES         ((FLASH_S_PARTITION_SIZE + \
                                          FLASH_NS_PARTITION_SIZE) / \
                                         FLASH_AREA_IMAGE_SCRATCH_SIZE)

/** Maximum number of image sectors supported by the bootloader. */
#define BOOT_MAX_IMG_SECTORS            ((FLASH_S_PARTITION_SIZE + \
                                          FLASH_NS_PARTITION_SIZE) / \
                                         FLASH_AREA_IMAGE_SECTOR_SIZE)

/* Internal Trusted Storage (ITS) Service definitions */
#define FLASH_ITS_AREA_OFFSET           (FLASH_AREA_BL2_OFFSET + FLASH_AREA_BL2_SIZE)
#define FLASH_ITS_AREA_SIZE             (0x5000)  /* 20 KB */

/* Protected Storage (PS) Service definitions */
#define FLASH_PS_AREA_OFFSET            (FLASH_ITS_AREA_OFFSET + \
                                         FLASH_ITS_AREA_SIZE)
#define FLASH_PS_AREA_SIZE              (0x1000)  /* 4 KB */

/* NV Counters definitions */
#define FLASH_NV_COUNTERS_AREA_OFFSET   (FLASH_PS_AREA_OFFSET + \
                                         FLASH_PS_AREA_SIZE)
#define FLASH_NV_COUNTERS_AREA_SIZE     (FLASH_AREA_IMAGE_SECTOR_SIZE)

/* Offset and size definition in flash area, used by assemble.py */
#define SECURE_IMAGE_OFFSET             (0x0)
#define SECURE_IMAGE_MAX_SIZE           FLASH_S_PARTITION_SIZE

#define NON_SECURE_IMAGE_OFFSET         (SECURE_IMAGE_OFFSET + \
                                         SECURE_IMAGE_MAX_SIZE)
#define NON_SECURE_IMAGE_MAX_SIZE       FLASH_NS_PARTITION_SIZE

/* Flash device name used by BL2
 * Name is defined in flash driver file: Driver_Flash.c
 */
#define FLASH_DEV_NAME Driver_FLASH0

/* Secure Storage (PS) Service definitions
 * Note: Further documentation of these definitions can be found in the
 * TF-M PS Integration Guide.
 */
#define PS_FLASH_DEV_NAME Driver_FLASH0

/* In this target the CMSIS driver requires only the offset from the base
 * address instead of the full memory address.
 */
#define PS_FLASH_AREA_ADDR     FLASH_PS_AREA_OFFSET
/* Dedicated flash area for PS */
#define PS_FLASH_AREA_SIZE     FLASH_PS_AREA_SIZE
#define PS_SECTOR_SIZE         FLASH_AREA_IMAGE_SECTOR_SIZE
/* Number of PS_SECTOR_SIZE per block */
#define PS_SECTORS_PER_BLOCK   (0x1)
/* Specifies the smallest flash programmable unit in bytes */
#define PS_FLASH_PROGRAM_UNIT  0x1
/* The maximum asset size to be stored in the PS area */
#define PS_MAX_ASSET_SIZE      512
/* The maximum number of assets to be stored in the PS area */
#define PS_NUM_ASSETS          4

/* Internal Trusted Storage (ITS) Service definitions
 * Note: Further documentation of these definitions can be found in the
 * TF-M ITS Integration Guide. The ITS should be in the internal flash, but is
 * allocated in the external flash just for development platforms that don't
 * have internal flash available.
 */
#define ITS_FLASH_DEV_NAME Driver_FLASH0

/* In this target the CMSIS driver requires only the offset from the base
 * address instead of the full memory address.
 */
#define ITS_FLASH_AREA_ADDR     FLASH_ITS_AREA_OFFSET
/* Dedicated flash area for ITS */
#define ITS_FLASH_AREA_SIZE     FLASH_ITS_AREA_SIZE
#define ITS_SECTOR_SIZE         FLASH_AREA_IMAGE_SECTOR_SIZE
/* Number of ITS_SECTOR_SIZE per block */
#define ITS_SECTORS_PER_BLOCK   (0x1)
/* Specifies the smallest flash programmable unit in bytes */
#define ITS_FLASH_PROGRAM_UNIT  (0x1)
/* The maximum asset size to be stored in the ITS area */
#define ITS_MAX_ASSET_SIZE      (1100)  /* Worst case for SECP521 key */
/* The maximum number of assets to be stored in the ITS area */
#define ITS_NUM_ASSETS          (32)

/* NV Counters definitions */
#define TFM_NV_COUNTERS_AREA_ADDR    FLASH_NV_COUNTERS_AREA_OFFSET
#define TFM_NV_COUNTERS_AREA_SIZE    (0x18) /* 24 Bytes */
#define TFM_NV_COUNTERS_SECTOR_ADDR  FLASH_NV_COUNTERS_AREA_OFFSET
#define TFM_NV_COUNTERS_SECTOR_SIZE  FLASH_AREA_IMAGE_SECTOR_SIZE

#define S_ROM_ALIAS_BASE   (0x0000D000)
#define S_ROM_ALIAS_LIMIT  (0x00038000)
#define S_RAM_ALIAS_BASE   (0x20000000)
#define S_RAM_ALIAS_LIMIT  (0x2000C000)

#define NS_ROM_ALIAS_BASE  (0x00038000)
#define NS_ROM_ALIAS_LIMIT (0x00100000)
#define NS_RAM_ALIAS_BASE  (0x2000C000)
#define NS_RAM_ALIAS_LIMIT (0x20040000)

#define TOTAL_ROM_SIZE FLASH_TOTAL_SIZE
#define TOTAL_RAM_SIZE (0x00040000)     /* 256 KB */

#endif /* __FLASH_LAYOUT_H__ */