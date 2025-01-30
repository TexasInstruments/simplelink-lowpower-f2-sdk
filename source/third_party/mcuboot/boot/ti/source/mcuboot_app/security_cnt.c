/*
 * Copyright (c) 2020 Arm Limited.
 * Copyright (c) 2022 Texas Instruments
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

#include <stdint.h>
#include "bootutil_priv.h"
#include "bootutil/security_cnt.h"
#include "flash_map_backend.h"

#ifdef MCUBOOT_HW_ROLLBACK_PROT

#if (DeviceFamily_PARENT != DeviceFamily_PARENT_CC13X4_CC26X3_CC26X4) || \
    (DeviceFamily_PARENT != DeviceFamily_PARENT_CC23X0)
    #error "MCUBOOT_HW_ROLLBACK_PROT not allowed in this device family"
#endif

#if defined (DUAL_SLOT) && (DeviceFamily_PARENT == DeviceFamily_PARENT_CC13X4_CC26X3_CC26X4)
    #error "MCUBOOT_HW_ROLLBACK_PROT not supported for TZ_Enabled build config"
#endif

#define BIMINFO_SIZE                    0x800 // One sector (last sector in MCUBoot region)
#define BIMINFO_BASE_ADDRESS            BOOTLOADER_BASE_ADDRESS + BOOT_BOOTLOADER_SIZE - BIMINFO_SIZE
#define MAX_SECURITY_CNT                (BIMINFO_SIZE - BIM_INFO_MAGIC_SZ) / BIM_ITEM_SZ

static const struct flash_area bimInfo =
{
    .fa_id = FLASH_AREA_BOOTLOADER,
    .fa_device_id = FLASH_DEVICE_INTERNAL_FLASH,
    .fa_off = BIMINFO_BASE_ADDRESS,
    .fa_size = BIMINFO_SIZE,
};

/* Declaration of internal static functions */
static int readBimSector(uint32_t offset, uint8_t *pBuf, uint32_t len);
static int writeBimSector(uint32_t offset, uint8_t *pBuf, uint32_t len);
static int eraseBimSector(void);
static bool checkBimInfoMagic(uint16_t pageMagic);
static int writeBimInfoMagic(uint16_t pageMagic);
static bool IsWithinBimInfoPage(uint32_t offset);
static uint32_t findEmptyEntry(void);
static uint32_t findValidEntry(uint32_t image_id, sc_item_t *readitem);
static int writeOneEntry(uint32_t offset, uint32_t image_id, uint32_t sc);
static int invalidateItem(uint32_t offset, sc_item_t *writeItem);

/* Public functions */

/**
 * Initialises the security counters.
 *
 * @return  FIH_SUCCESS on success
 */
fih_int boot_nv_security_counter_init(void)
{
    fih_int rc= FIH_SUCCESS;

    if (checkBimInfoMagic(BIM_INFO_PAGE_MAGIC)) {

        return(FIH_SUCCESS);
    }
    else if (checkBimInfoMagic(BIM_INFO_PAGE_ERASED)) {
        rc = writeBimInfoMagic(BIM_INFO_PAGE_MAGIC);
        if (rc != BIMINFO_SUCCESS) {
            return (FIH_FAILURE);
        }
        // default image version =0, security counter = 0
        rc = writeOneEntry(BIM_INFO_MAGIC_SZ, 0, 0);
        if (rc != BIMINFO_SUCCESS) {
            return (FIH_FAILURE);
        }
    }
    else {
        eraseBimSector();       // in case the bim info sector is not fully erased or corrupted
        rc = writeBimInfoMagic(BIM_INFO_PAGE_MAGIC);
        if (rc != BIMINFO_SUCCESS) {
            return (FIH_FAILURE);
        }
        // default image version =0, security counter = 0
        rc = writeOneEntry(BIM_INFO_MAGIC_SZ, 0, 0);
        if (rc != BIMINFO_SUCCESS) {
            return (FIH_FAILURE);
        }
    }

    return(rc);
}

/**
 * Reads the stored value of a given image's security counter.
 *
 * @param image_id          Index of the image (from 0).
 * @param security_cnt      Pointer to store the security counter value.
 *
 * @return                  FIH_SUCCESS on success
 */
fih_int boot_nv_security_counter_get(uint32_t image_id, fih_int *security_cnt)
{
    uint32_t offset;
    sc_item_t readitem;

    offset = findValidEntry(image_id, &readitem);
    if(offset == 0) {
        /* cannot find a valid item */
        return(FIH_FAILURE);
    }
    *security_cnt = readitem.security_counter;

    return(FIH_SUCCESS);
}

/**
 * Updates the stored value of a given image's security counter with a new
 * security counter value if the new one is greater.
 *
 * @param image_id          Index of the image (from 0).
 * @param img_security_cnt  New security counter value. The new value must be
 *                          between 0 and UINT32_MAX and it must be greater than
 *                          or equal to the current security counter value.
 *
 * @return                  0 on success; nonzero on failure.
 */
int32_t boot_nv_security_counter_update(uint32_t image_id, uint32_t img_security_cnt)
{
    uint32_t offset;
    sc_item_t readitem;

    if(img_security_cnt > MAX_SECURITY_CNT)
    {
        /* out of range */
        return(BIMINFO_FAIL);
    }

    offset = findValidEntry(image_id, &readitem);
    if(offset == BIMINFO_FULL) {
        /* the page is full*/
        return(BIMINFO_FAIL);
    }
    else if(offset == 0) {
        /* can't find a valid item
         * and the page is not full, so add a new item to the bim info sector*/
        offset = findEmptyEntry();
        writeOneEntry(offset, image_id, img_security_cnt);
        return(BIMINFO_SUCCESS);
    }

    /* found a valid item */
    if(readitem.security_counter == img_security_cnt)
    {
        /* new security counter is the same. No need to update*/
        return(BIMINFO_SUCCESS);
    }
    else if(readitem.security_counter > img_security_cnt)
    {
        /* new security counter is smaller than existing one.
         * this should not happen. Report as an error */
        return(BIMINFO_FAIL);
    }

    /* new counter needs to be added to bim info sector */
    invalidateItem(offset, &readitem);

    offset = findEmptyEntry();
    writeOneEntry(offset, image_id, img_security_cnt);

    return(BIMINFO_SUCCESS);
}

/**
 * Lock the BIM INFO sector
 */
void lockBimSector(void)
{
    flash_area_lock(&bimInfo);
}

/**
 * Read bim info sector 
 * @param offset           offset address (delta address from base address)
 * @param pBuf             buffer pointer for data to be read
 * @param len              byte length of data to be read
 *
 * @return                  0 on success; nonzero on failure.
 */
static int readBimSector(uint32_t offset, uint8_t *pBuf, uint32_t len)
{
    int rc;

    rc = flash_area_read(&bimInfo, offset, pBuf, len);
    if (rc != BIMINFO_SUCCESS) {
        return BOOT_EFLASH;
    }
    return rc;
}

/**
 * write into bim info sector 
 * @param offset           offset address (delta address from base address)
 * @param pBuf             buffer pointer for data to be written
 * @param len              byte length of data to be written
 *
 * @return                  0 on success; nonzero on failure.
 */
static int writeBimSector(uint32_t offset, uint8_t *pBuf, uint32_t len)
{
    int rc;

    rc = flash_area_write(&bimInfo, offset, pBuf, len);
    if (rc != BIMINFO_SUCCESS) {
        return BOOT_EFLASH;
    }
    return rc;
}

/**
 * Erase the bim info sector 
 */
static int eraseBimSector(void)
{
    int rc;

    rc = flash_area_erase(&bimInfo, 0, 2);
    if (rc != BIMINFO_SUCCESS) {
        return BOOT_EFLASH;
    }
    return rc;
}

/**
 * Read the bim info magic number
 * @param pageMagic        magic number to be compared
 *
 * @return                 true if the magic number matches, false otherwise
 */
static bool checkBimInfoMagic(uint16_t pageMagic)
{
    int rc;
    sc_page_head_t buf;

    rc = readBimSector(0, (uint8_t*)&buf, BIM_INFO_MAGIC_SZ);
    if (rc != BIMINFO_SUCCESS) {
        return BOOT_EFLASH;
    }
    if(buf.sc_page_id == pageMagic) {
        return true;
    }
    else {
        return false;
    }
}

/**
 * write the magic number number into bim info sector at zero index.
 * @param pageMagic        magic number to be written 
 *
 * @return                 0 on success; nonzero on failure.
 */
static int writeBimInfoMagic(uint16_t pageMagic)
{
    int rc;
    sc_page_head_t buf;

    buf.sc_page_id = pageMagic;
    rc = writeBimSector(0,(uint8_t*)&buf, sizeof(sc_page_head_t));

    if (rc != BIMINFO_SUCCESS) {
        return BOOT_EFLASH;
    }
    return rc;
}

/**
 * Check if the offset addres is within the bim info sector.
 * @param offset        offset address 
 *
 * @return               true on success; false on failure.
 */
static bool IsWithinBimInfoPage(uint32_t offset)
{
    uint32_t startAddress = BIMINFO_BASE_ADDRESS + offset;

    if((startAddress >= BIMINFO_BASE_ADDRESS + BIM_INFO_MAGIC_SZ) && (startAddress <=  BIMINFO_BASE_ADDRESS + BIMINFO_SIZE - BIM_ITEM_SZ))
    {
        return true;
    }
    else
    {
        return false;
    }
}

/**
 * Finds and returns the offset of the first empty entry with at least one new item.
 *
 * @return      return the offset of the first empty entry, otherwise BIMINFO_FULL.
 */
static uint32_t findEmptyEntry(void)
{
    uint32_t readOffset = BIM_INFO_MAGIC_SZ;
    sc_item_t readItem;

    do
    {
        readBimSector(readOffset, (uint8_t*)&readItem, BIM_ITEM_SZ);
        if ((readItem.sc_status = 0xFF) & (readItem.item_id == 0xFFFF))
        {
            return readOffset;
        }
        else
        {
            readOffset += BIM_ITEM_SZ;
        }


    } while(IsWithinBimInfoPage(readOffset));

    /* can't find an empty entry */
    return(BIMINFO_FULL);
}

/**
 * Find he valid entry.
 * @param image_id       image id. it is alwasy zero 
 * @param image_id       buffer point to store the item found 
 *
 * @return               retun the offset address of the valid item
 * if no valid entry, then return zero
 * if no valid entry and the page is full, then return 0xFF
 */
static uint32_t findValidEntry(uint32_t image_id, sc_item_t *readitem)
{
    uint32_t readOffset = BIM_INFO_MAGIC_SZ;

    do
    {
        readBimSector(readOffset, (uint8_t*)readitem, BIM_ITEM_SZ);
        if((readitem->sc_status == SC_STATUS_VALID) & (readitem->item_id == SC_ITEM_ID) & (readitem->image_id == image_id))
        {
            return readOffset;
        }
        else if((readitem->sc_status = 0xFF) & (readitem->item_id == 0xFFFF))
        {
            /* empty entry, so no need to go further */
            return 0;
        }
        else
        {
            readOffset += BIM_ITEM_SZ;
        }


    } while(IsWithinBimInfoPage(readOffset));

    /* can't find a valid entry */
    return(BIMINFO_FULL);
}

/**
 * Write one entry to bim info sector.
 * @param offset       Target offset address 
 * @param image_id     image id  
 * @param image_id     security counter value
 * @return             return 0 on success, otherwise non zero value
 */
static int writeOneEntry(uint32_t offset, uint32_t image_id, uint32_t sc)
{
    sc_item_t writeItem;

    writeItem.image_id = image_id;
    writeItem.security_counter = sc;
    writeItem.item_id = SC_ITEM_ID;
    writeItem.sc_status = SC_STATUS_VALID;

    return (writeBimSector(offset, (uint8_t*)&writeItem, BIM_ITEM_SZ));

}

/**
 * Invalid the existing etnry.
 * @param offset       Target offset address 
 * @param writeItem    the entry to be updated  
 * @return             return 0 on success, otherwise non zero value
 */
static int invalidateItem(uint32_t offset, sc_item_t *writeItem)
{
    writeItem->sc_status = SC_STATUS_INVALID;

    return (writeBimSector(offset, (uint8_t*)writeItem, BIM_ITEM_SZ));

}

#endif
