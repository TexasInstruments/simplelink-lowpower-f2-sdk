/*
 * Copyright (c) 2017-2018 ARM Limited
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

#ifndef __REGION_DEFS_H__
#define __REGION_DEFS_H__

#include "flash_layout.h"

#define TOTAL_ROM_SIZE FLASH_TOTAL_SIZE
#define TOTAL_RAM_SIZE (0x00040000)     /* 256 KB */

#define BL2_HEAP_SIZE           0x0001000
#define BL2_MSP_STACK_SIZE      0x0001000

#define S_HEAP_SIZE             0x0001000
#define S_MSP_STACK_SIZE_INIT   0x0000400
#define S_MSP_STACK_SIZE        0x0000800
#define S_PSP_STACK_SIZE        0x0000800

#define NS_HEAP_SIZE            0x0001000
#define NS_MSP_STACK_SIZE       0x0000400
#define NS_PSP_STACK_SIZE       0x0000C00

/* This size of buffer is big enough to store an attestation
 * token produced by initial attestation service
 */
#define PSA_INITIAL_ATTEST_TOKEN_MAX_SIZE   (0x250)
#ifdef BL2
#ifndef LINK_TO_SECONDARY_PARTITION
#define  S_IMAGE_PRIMARY_PARTITION_OFFSET   (FLASH_AREA_IMAGE_0_OFFSET)
#define  S_IMAGE_SECONDARY_PARTITION_OFFSET (FLASH_AREA_IMAGE_1_OFFSET)
#else
#define  S_IMAGE_PRIMARY_PARTITION_OFFSET   (FLASH_AREA_IMAGE_1_OFFSET)
#define  S_IMAGE_SECONDARY_PARTITION_OFFSET (FLASH_AREA_IMAGE_0_OFFSET)
#endif /* !LINK_TO_SECONDARY_PARTITION */
#else
#define  S_IMAGE_PRIMARY_PARTITION_OFFSET (0x0)
#endif /* BL2 */

#ifndef LINK_TO_SECONDARY_PARTITION
#define NS_IMAGE_PRIMARY_PARTITION_OFFSET (0x100000)
#else
#define NS_IMAGE_PRIMARY_PARTITION_OFFSET (0x200000)
#endif /* !LINK_TO_SECONDARY_PARTITION */

/*
 * Boot partition structure if MCUBoot is used:
 * 0x0_0000 Bootloader header
 * 0x0_0400 Image area
 * 0x7_0000 Trailer
 */
/* IMAGE_CODE_SIZE is the space available for the software binary image.
 * It is less than the FLASH_PARTITION_SIZE because we reserve space
 * for the image header and trailer introduced by the bootloader.
 */
#ifdef BL2
#define BL2_HEADER_SIZE      (0x400)
#define BL2_TRAILER_SIZE     (0x10000)
#else
/* No header if no bootloader, but keep IMAGE_CODE_SIZE the same */
#define BL2_HEADER_SIZE      (0x0)
#define BL2_TRAILER_SIZE     (0x10400)
#endif /* BL2 */

#define IMAGE_CODE_SIZE \
            (FLASH_PARTITION_SIZE - BL2_HEADER_SIZE - BL2_TRAILER_SIZE)

/* FIXME: veneer region size is increased temporarily while both legacy veneers
 * and their iovec-based equivalents co-exist for secure partitions. To be
 * adjusted as legacy veneers are eliminated
 */
#define CMSE_VENEER_REGION_SIZE     (0x00000380)

/* Alias definitions for secure and non-secure areas*/
#define S_ROM_ALIAS(x)  (S_ROM_ALIAS_BASE + x)
#define NS_ROM_ALIAS(x) (NS_ROM_ALIAS_BASE + x)

#define S_RAM_ALIAS(x)  (S_RAM_ALIAS_BASE + x)
#define NS_RAM_ALIAS(x) (NS_RAM_ALIAS_BASE + x)

/* Secure regions */
#define  S_IMAGE_PRIMARY_AREA_OFFSET \
             (S_IMAGE_PRIMARY_PARTITION_OFFSET + BL2_HEADER_SIZE)
#define S_CODE_START    (S_ROM_ALIAS(0))
#define S_CODE_SIZE     (S_ROM_ALIAS_LIMIT - S_ROM_ALIAS_BASE - CMSE_VENEER_REGION_SIZE)
#define S_CODE_LIMIT    (S_CODE_START + S_CODE_SIZE - 1)

#define S_DATA_START    (S_RAM_ALIAS(0))
#define S_DATA_SIZE     (S_RAM_ALIAS_LIMIT - S_RAM_ALIAS_BASE)
#define S_DATA_LIMIT    (S_DATA_START + S_DATA_SIZE - 1)

/* CMSE Veneers region */
#define CMSE_VENEER_REGION_START  (S_CODE_LIMIT + 1)
#define CMSE_VENEER_REGION_LIMIT  (CMSE_VENEER_REGION_START + \
                                   CMSE_VENEER_REGION_SIZE - 1)

/* Non-secure regions */
#define NS_IMAGE_PRIMARY_AREA_OFFSET \
                        (NS_IMAGE_PRIMARY_PARTITION_OFFSET + BL2_HEADER_SIZE)
#define NS_CODE_START   (NS_ROM_ALIAS(0))
#define NS_CODE_SIZE    (NS_ROM_ALIAS_LIMIT - NS_ROM_ALIAS_BASE)
#define NS_CODE_LIMIT   (NS_CODE_START + NS_CODE_SIZE - 1)

#define NS_DATA_START   (NS_RAM_ALIAS(0))
#define NS_DATA_SIZE    (NS_RAM_ALIAS_LIMIT - NS_RAM_ALIAS_BASE)
#define NS_DATA_LIMIT   (NS_DATA_START + NS_DATA_SIZE - 1)

/* NS partition information is used for MPC configuration */
#define NS_PARTITION_START \
            (NS_ROM_ALIAS(0))

#define NS_PARTITION_SIZE (NS_CODE_SIZE)

/* Secondary partition for new images in case of firmware upgrade */
#define SECONDARY_PARTITION_START \
            (NS_ROM_ALIAS(S_IMAGE_SECONDARY_PARTITION_OFFSET))

#define SECONDARY_PARTITION_SIZE (2 * FLASH_PARTITION_SIZE)

#define NS_PARTITION_LIMIT   (NS_CODE_LIMIT)

#ifdef BL2
/* Bootloader regions */
#define BL2_CODE_START    (S_ROM_ALIAS(FLASH_AREA_BL2_OFFSET))
#define BL2_CODE_SIZE     (FLASH_AREA_BL2_SIZE)
#define BL2_CODE_LIMIT    (BL2_CODE_START + BL2_CODE_SIZE - 1)

#define BL2_DATA_START    (S_RAM_ALIAS(0x0))
#define BL2_DATA_SIZE     (TOTAL_RAM_SIZE)
#define BL2_DATA_LIMIT    (BL2_DATA_START + BL2_DATA_SIZE - 1)
#endif /* BL2 */

#endif /* __REGION_DEFS_H__ */
