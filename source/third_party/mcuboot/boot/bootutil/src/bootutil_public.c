/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Copyright (c) 2017-2019 Linaro LTD
 * Copyright (c) 2016-2019 JUUL Labs
 * Copyright (c) 2019-2023 Arm Limited
 * Copyright (c) 2020-2023 Nordic Semiconductor ASA
 *
 * Original license:
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

/**
 * @file
 * @brief Public MCUBoot interface API implementation
 *
 * This file contains API implementation which can be combined with
 * the application in order to interact with the MCUBoot bootloader.
 * This file contains shared code-base betwen MCUBoot and the application
 * which controls DFU process.
 */

#include <string.h>
#include <inttypes.h>
#include <stddef.h>

#include "sysflash/sysflash.h"
#include "flash_map_backend.h"

#include "bootutil/image.h"
#include "bootutil/bootutil_public.h"
#include "bootutil_priv.h"
#include "bootutil_misc.h"
#include "bootutil/bootutil_log.h"

#ifdef CONFIG_MCUBOOT
MCUBOOT_LOG_MODULE_DECLARE(mcuboot);
#else
MCUBOOT_LOG_MODULE_REGISTER(mcuboot_util);
#endif

#if BOOT_MAX_ALIGN == 8
const union boot_img_magic_t boot_img_magic = {
    .val = {
        0x77, 0xc2, 0x95, 0xf3,
        0x60, 0xd2, 0xef, 0x7f,
        0x35, 0x52, 0x50, 0x0f,
        0x2c, 0xb6, 0x79, 0x80
    }
};

#else
const union boot_img_magic_t boot_img_magic = {
    .align = BOOT_MAX_ALIGN,
    .magic = {
        0x2d, 0xe1,
        0x5d, 0x29, 0x41, 0x0b,
        0x8d, 0x77, 0x67, 0x9c,
        0x11, 0x0f, 0x1f, 0x8a
    }
};
#endif

struct boot_swap_table {
    uint8_t magic_primary_slot;
    uint8_t magic_secondary_slot;
    uint8_t image_ok_primary_slot;
    uint8_t image_ok_secondary_slot;
    uint8_t copy_done_primary_slot;

    uint8_t swap_type;
};

/**
 * This set of tables maps image trailer contents to swap operation type.
 * When searching for a match, these tables must be iterated sequentially.
 *
 * NOTE: the table order is very important. The settings in the secondary
 * slot always are priority to the primary slot and should be located
 * earlier in the table.
 *
 * The table lists only states where there is action needs to be taken by
 * the bootloader, as in starting/finishing a swap operation.
 */
static const struct boot_swap_table boot_swap_tables[] = {
    {
        .magic_primary_slot =       BOOT_MAGIC_ANY,
        .magic_secondary_slot =     BOOT_MAGIC_GOOD,
        .image_ok_primary_slot =    BOOT_FLAG_ANY,
        .image_ok_secondary_slot =  BOOT_FLAG_UNSET,
        .copy_done_primary_slot =   BOOT_FLAG_ANY,
        .swap_type =                BOOT_SWAP_TYPE_TEST,
    },
    {
        .magic_primary_slot =       BOOT_MAGIC_ANY,
        .magic_secondary_slot =     BOOT_MAGIC_GOOD,
        .image_ok_primary_slot =    BOOT_FLAG_ANY,
        .image_ok_secondary_slot =  BOOT_FLAG_SET,
        .copy_done_primary_slot =   BOOT_FLAG_ANY,
        .swap_type =                BOOT_SWAP_TYPE_PERM,
    },
    {
        .magic_primary_slot =       BOOT_MAGIC_GOOD,
        .magic_secondary_slot =     BOOT_MAGIC_UNSET,
        .image_ok_primary_slot =    BOOT_FLAG_UNSET,
        .image_ok_secondary_slot =  BOOT_FLAG_ANY,
        .copy_done_primary_slot =   BOOT_FLAG_SET,
        .swap_type =                BOOT_SWAP_TYPE_REVERT,
    },
};

#define BOOT_SWAP_TABLES_COUNT \
    (sizeof boot_swap_tables / sizeof boot_swap_tables[0])

static int
boot_flag_decode(uint8_t flag)
{
    if (flag != BOOT_FLAG_SET) {
        return BOOT_FLAG_BAD;
    }
    return BOOT_FLAG_SET;
}

uint32_t
boot_swap_info_off(const struct flash_area *fap)
{
    return boot_copy_done_off(fap) - BOOT_MAX_ALIGN;
}

/**
 * Determines if a status source table is satisfied by the specified magic
 * code.
 *
 * @param tbl_val               A magic field from a status source table.
 * @param val                   The magic value in a trailer, encoded as a
 *                                  BOOT_MAGIC_[...].
 *
 * @return                      1 if the two values are compatible;
 *                              0 otherwise.
 */
int
boot_magic_compatible_check(uint8_t tbl_val, uint8_t val)
{
    switch (tbl_val) {
    case BOOT_MAGIC_ANY:
        return 1;

    case BOOT_MAGIC_NOTGOOD:
        return val != BOOT_MAGIC_GOOD;

    default:
        return tbl_val == val;
    }
}

bool bootutil_buffer_is_erased(const struct flash_area *area,
                               const void *buffer, size_t len)
{
    size_t i;
    uint8_t *u8b;
    uint8_t erased_val;

    if (buffer == NULL || len == 0) {
        return false;
    }

    erased_val = flash_area_erased_val(area);
    for (i = 0, u8b = (uint8_t *)buffer; i < len; i++) {
        if (u8b[i] != erased_val) {
            return false;
        }
    }

    return true;
}

static int
boot_read_flag(const struct flash_area *fap, uint8_t *flag, uint32_t off)
{
    int rc;

    rc = flash_area_read(fap, off, flag, sizeof *flag);
    if (rc < 0) {
        return BOOT_EFLASH;
    }
    if (bootutil_buffer_is_erased(fap, flag, sizeof *flag)) {
        *flag = BOOT_FLAG_UNSET;
    } else {
        *flag = boot_flag_decode(*flag);
    }

    return 0;
}

static inline int
boot_read_copy_done(const struct flash_area *fap, uint8_t *copy_done)
{
    return boot_read_flag(fap, copy_done, boot_copy_done_off(fap));
}


int
boot_read_swap_state(const struct flash_area *fap,
                     struct boot_swap_state *state)
{
    uint8_t magic[BOOT_MAGIC_SZ];
    uint32_t off;
    uint8_t swap_info;
    int rc;

    off = boot_magic_off(fap);
    rc = flash_area_read(fap, off, magic, BOOT_MAGIC_SZ);
    if (rc < 0) {
        return BOOT_EFLASH;
    }
    if (bootutil_buffer_is_erased(fap, magic, BOOT_MAGIC_SZ)) {
        state->magic = BOOT_MAGIC_UNSET;
    } else {
        state->magic = boot_magic_decode(magic);
    }

    off = boot_swap_info_off(fap);
    rc = flash_area_read(fap, off, &swap_info, sizeof swap_info);
    if (rc < 0) {
        return BOOT_EFLASH;
    }

    /* Extract the swap type and image number */
    state->swap_type = BOOT_GET_SWAP_TYPE(swap_info);
    state->image_num = BOOT_GET_IMAGE_NUM(swap_info);

    if (bootutil_buffer_is_erased(fap, &swap_info, sizeof swap_info) ||
            state->swap_type > BOOT_SWAP_TYPE_REVERT) {
        state->swap_type = BOOT_SWAP_TYPE_NONE;
        state->image_num = 0;
    }

    rc = boot_read_copy_done(fap, &state->copy_done);
    if (rc) {
        return BOOT_EFLASH;
    }

    return boot_read_image_ok(fap, &state->image_ok);
}

int
boot_read_swap_state_by_id(int flash_area_id, struct boot_swap_state *state)
{
    const struct flash_area *fap;
    int rc;

    rc = flash_area_open(flash_area_id, &fap);
    if (rc != 0) {
        return BOOT_EFLASH;
    }

    rc = boot_read_swap_state(fap, state);
    flash_area_close(fap);
    return rc;
}

int
boot_write_magic(const struct flash_area *fap)
{
    uint32_t off;
    uint32_t pad_off;
    int rc;
    uint8_t magic[BOOT_MAGIC_ALIGN_SIZE];
    uint8_t erased_val;

    off = boot_magic_off(fap);

    /* image_trailer structure was modified with additional padding such that
     * the pad+magic ends up in a flash minimum write region. The address
     * returned by boot_magic_off() is the start of magic which is not the
     * start of the flash write boundary and thus writes to the magic will fail.
     * To account for this change, write to magic is first padded with 0xFF
     * before writing to the trailer.
     */
    pad_off = ALIGN_DOWN(off, BOOT_MAX_ALIGN);

    erased_val = flash_area_erased_val(fap);

    memset(&magic[0], erased_val, sizeof(magic));
    memcpy(&magic[BOOT_MAGIC_ALIGN_SIZE - BOOT_MAGIC_SZ], BOOT_IMG_MAGIC, BOOT_MAGIC_SZ);

    BOOT_LOG_DBG("writing magic; fa_id=%d off=0x%lx (0x%lx)",
                 fap->fa_id, (unsigned long)off,
                 (unsigned long)(fap->fa_off + off));
    rc = flash_area_write(fap, pad_off, &magic[0], BOOT_MAGIC_ALIGN_SIZE);
    if (rc != 0) {
        return BOOT_EFLASH;
    }

    return 0;
}

/**
 * Write trailer data; status bytes, swap_size, etc
 *
 * @returns 0 on success, != 0 on error.
 */
int
boot_write_trailer(const struct flash_area *fap, uint32_t off,
        const uint8_t *inbuf, uint8_t inlen)
{
    uint8_t buf[BOOT_MAX_ALIGN];
    uint32_t align;
    uint8_t erased_val;
    int rc;

    align = flash_area_align(fap);
    if (inlen > BOOT_MAX_ALIGN || align > BOOT_MAX_ALIGN) {
        return -1;
    }
    erased_val = flash_area_erased_val(fap);
    if (align < inlen) {
        align = inlen;
    }
    memcpy(buf, inbuf, inlen);
    memset(&buf[inlen], erased_val, align - inlen);

    rc = flash_area_write(fap, off, buf, align);
    if (rc != 0) {
        return BOOT_EFLASH;
    }

    return 0;
}

int
boot_write_trailer_flag(const struct flash_area *fap, uint32_t off,
        uint8_t flag_val)
{
    const uint8_t buf[1] = { flag_val };
    return boot_write_trailer(fap, off, buf, 1);
}

int
boot_write_image_ok(const struct flash_area *fap)
{
    uint32_t off;

    off = boot_image_ok_off(fap);
    BOOT_LOG_DBG("writing image_ok; fa_id=%d off=0x%lx (0x%lx)",
                 fap->fa_id, (unsigned long)off,
                 (unsigned long)(fap->fa_off + off));
    return boot_write_trailer_flag(fap, off, BOOT_FLAG_SET);
}

int
boot_read_image_ok(const struct flash_area *fap, uint8_t *image_ok)
{
    return boot_read_flag(fap, image_ok, boot_image_ok_off(fap));
}

/**
 * Writes the specified value to the `swap-type` field of an image trailer.
 * This value is persisted so that the boot loader knows what swap operation to
 * resume in case of an unexpected reset.
 */
int
boot_write_swap_info(const struct flash_area *fap, uint8_t swap_type,
                     uint8_t image_num)
{
    uint32_t off;
    uint8_t swap_info;

    BOOT_SET_SWAP_INFO(swap_info, image_num, swap_type);
    off = boot_swap_info_off(fap);
    BOOT_LOG_DBG("writing swap_info; fa_id=%d off=0x%lx (0x%lx), swap_type=0x%x"
                 " image_num=0x%x",
                 fap->fa_id, (unsigned long)off,
                 (unsigned long)(fap->fa_off + off), swap_type, image_num);
    return boot_write_trailer(fap, off, (const uint8_t *) &swap_info, 1);
}

int
boot_swap_type_multi(int image_index)
{
    const struct boot_swap_table *table;
    struct boot_swap_state primary_slot;
    struct boot_swap_state secondary_slot;
    int rc;
    size_t i;

    rc = boot_read_swap_state_by_id(FLASH_AREA_IMAGE_PRIMARY(image_index),
                                    &primary_slot);
    if (rc) {
        return BOOT_SWAP_TYPE_PANIC;
    }

    rc = boot_read_swap_state_by_id(FLASH_AREA_IMAGE_SECONDARY(image_index),
                                    &secondary_slot);
    if (rc) {
        return BOOT_SWAP_TYPE_PANIC;
    }

    for (i = 0; i < BOOT_SWAP_TABLES_COUNT; i++) {
        table = boot_swap_tables + i;

        if (boot_magic_compatible_check(table->magic_primary_slot,
                                        primary_slot.magic) &&
            boot_magic_compatible_check(table->magic_secondary_slot,
                                        secondary_slot.magic) &&
            (table->image_ok_primary_slot == BOOT_FLAG_ANY   ||
                table->image_ok_primary_slot == primary_slot.image_ok) &&
            (table->image_ok_secondary_slot == BOOT_FLAG_ANY ||
                table->image_ok_secondary_slot == secondary_slot.image_ok) &&
            (table->copy_done_primary_slot == BOOT_FLAG_ANY  ||
                table->copy_done_primary_slot == primary_slot.copy_done)) {
            BOOT_LOG_INF("Image index: %d, Swap type: %s", image_index,
                         table->swap_type == BOOT_SWAP_TYPE_TEST   ? "test"   :
                         table->swap_type == BOOT_SWAP_TYPE_PERM   ? "perm"   :
                         table->swap_type == BOOT_SWAP_TYPE_REVERT ? "revert" :
                         "BUG; can't happen");
            if (table->swap_type != BOOT_SWAP_TYPE_TEST &&
                    table->swap_type != BOOT_SWAP_TYPE_PERM &&
                    table->swap_type != BOOT_SWAP_TYPE_REVERT) {
                return BOOT_SWAP_TYPE_PANIC;
            }
            return table->swap_type;
        }
    }

    BOOT_LOG_INF("Image index: %d, Swap type: none", image_index);
    return BOOT_SWAP_TYPE_NONE;
}

/*
 * This function is not used by the bootloader itself, but its required API
 * by external tooling like mcumgr.
 */
int
boot_swap_type(void)
{
    return boot_swap_type_multi(0);
}

/**
 * Marks the image in the secondary slot as pending.  On the next reboot,
 * the system will perform a one-time boot of the the secondary slot image.
 *
 * @param permanent         Whether the image should be used permanently or
 *                              only tested once:
 *                                  0=run image once, then confirm or revert.
 *                                  1=run image forever.
 *
 * @return                  0 on success; nonzero on failure.
 */
int
boot_set_pending(int permanent)
{
    const struct flash_area *fap;
    struct boot_swap_state state_secondary_slot;
    uint8_t swap_type;
    int rc;

    rc = boot_read_swap_state_by_id(FLASH_AREA_IMAGE_SECONDARY(0),
                                    &state_secondary_slot);
    if (rc != 0) {
        return rc;
    }

    switch (state_secondary_slot.magic) {
    case BOOT_MAGIC_GOOD:
        /* Swap already scheduled. */
        return 0;

    case BOOT_MAGIC_UNSET:
        rc = flash_area_open(FLASH_AREA_IMAGE_SECONDARY(0), &fap);
        if (rc != 0) {
            rc = BOOT_EFLASH;
        } else {
            rc = boot_write_magic(fap);
        }

        if (rc == 0 && permanent) {
            rc = boot_write_image_ok(fap);
        }

        if (rc == 0) {
            if (permanent) {
                swap_type = BOOT_SWAP_TYPE_PERM;
            } else {
                swap_type = BOOT_SWAP_TYPE_TEST;
            }
            rc = boot_write_swap_info(fap, swap_type, 0);
        }

        flash_area_close(fap);
        return rc;

    case BOOT_MAGIC_BAD:
        /* The image slot is corrupt.  There is no way to recover, so erase the
         * slot to allow future upgrades.
         */
        rc = flash_area_open(FLASH_AREA_IMAGE_SECONDARY(0), &fap);
        if (rc != 0) {
            return BOOT_EFLASH;
        }

        flash_area_erase(fap, 0, fap->fa_size);
        flash_area_close(fap);
        return BOOT_EBADIMAGE;

    default:
        assert(0);
        return BOOT_EBADIMAGE;
    }
}

/**
 * Marks the image in the primary slot as confirmed.  The system will continue
 * booting into the image in the primary slot until told to boot from a
 * different slot.
 *
 * @return                  0 on success; nonzero on failure.
 */
int
boot_set_confirmed(void)
{
    const struct flash_area *fap;
    struct boot_swap_state state_primary_slot;
    int rc;

    rc = boot_read_swap_state_by_id(FLASH_AREA_IMAGE_PRIMARY(0),
                                    &state_primary_slot);
    if (rc != 0) {
        return rc;
    }

    switch (state_primary_slot.magic) {
    case BOOT_MAGIC_GOOD:
        /* Confirm needed; proceed. */
        break;

    case BOOT_MAGIC_UNSET:
        /* Already confirmed. */
        return 0;

    case BOOT_MAGIC_BAD:
        /* Unexpected state. */
        return BOOT_EBADVECT;
    }

    rc = flash_area_open(FLASH_AREA_IMAGE_PRIMARY(0), &fap);
    if (rc) {
        rc = BOOT_EFLASH;
        goto done;
    }

    if (state_primary_slot.copy_done == BOOT_FLAG_UNSET) {
        /* Swap never completed.  This is unexpected. */
        rc = BOOT_EBADVECT;
        goto done;
    }

    if (state_primary_slot.image_ok != BOOT_FLAG_UNSET) {
        /* Already confirmed. */
        goto done;
    }

    rc = boot_write_image_ok(fap);

done:
    flash_area_close(fap);
    return rc;
}

int
boot_image_load_header(const struct flash_area *fap,
                       struct image_header *hdr)
{
    uint32_t size;
    int rc = flash_area_read(fap, 0, hdr, sizeof *hdr);

    if (rc != 0) {
        rc = BOOT_EFLASH;
        BOOT_LOG_ERR("Failed reading image header");
	return BOOT_EFLASH;
    }

    if (hdr->ih_magic != IMAGE_MAGIC) {
        BOOT_LOG_ERR("Bad image magic 0x%lx", (unsigned long)hdr->ih_magic);

        return BOOT_EBADIMAGE;
    }

    if (hdr->ih_flags & IMAGE_F_NON_BOOTABLE) {
        BOOT_LOG_ERR("Image not bootable");

        return BOOT_EBADIMAGE;
    }

    if (!boot_u32_safe_add(&size, hdr->ih_img_size, hdr->ih_hdr_size) ||
        size >= fap->fa_size) {
        return BOOT_EBADIMAGE;
    }

    return 0;
}
