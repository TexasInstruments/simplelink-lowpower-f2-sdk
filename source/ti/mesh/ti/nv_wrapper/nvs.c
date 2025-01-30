/******************************************************************************

 @file  nvs_wrapper.c

 @brief This module defines the Zephyr simple non-volatile memory functions as a
        wrapper to NVOCMP(NV On Chip multi Page)

 Group: WCS, LPC, BTS
 Target Device: cc13x2_26x2

 ******************************************************************************

 Copyright (c) 2018-20120, Texas Instruments Incorporated
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions
 are met:

 *  Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.

 *  Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.

 *  Neither the name of Texas Instruments Incorporated nor the names of
    its contributors may be used to endorse or promote products derived
    from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************

 *****************************************************************************/

#ifdef CONFIG_BT_SETTINGS
//*****************************************************************************
// Includes
//*****************************************************************************
#include <string.h>
#include <errno.h>
#include <inttypes.h>
#include <kernel.h>
#include <zephyr.h>
#include "nvintf.h"
#include "nvocmp.h"
#include <fs/nvs.h>
#include <osal_snv.h>

//*****************************************************************************
// Global variables
//*****************************************************************************
extern struct k_mutex settings_lock;

//*****************************************************************************
// Local variables
//*****************************************************************************
 NVINTF_nvFuncts_t nvFps;
static uint8_t nvInitialized = 0;

/*********************************************************************
 * @fn      nvs_init
 *
 * @brief   initialize NV service.
 *
 * @param   fs - file system structure. not used.
 *               set fs to NULL before call
 * @param   dev_name - flash device name. not used.
 *               set dev_name to NULL before call
 *
 * @return  0 when success or -EFAULT when fail
 */
int nvs_init(struct nvs_fs *fs, const char *dev_name)
{
  uint8_t rc;
  (void)fs;
  (void)dev_name;

  if(nvInitialized == 0)
  {
    NVOCMP_loadApiPtrs(&nvFps);

    rc = nvFps.initNV(NULL);
    if(rc)
    {
      return -EFAULT;
    }
    else
    {
      nvInitialized = 1;
      return 0;
    }
  }
  else
  {
    return 0;
  }
}

/*********************************************************************
 * @fn      nvs_clear
 *
 * @brief   erase whole NV.
 *
 * @param   fs - file system structure. not used.
 *               set fs to NULL before call
 *
 * @return  0 when success or -EFAULT when fail.
 */
int nvs_clear(struct nvs_fs *fs)
{
  uint8_t rc;

  rc = nvFps.eraseNV();

  if(rc)
  {
    return -EFAULT;
  }
  else
  {
    return 0;
  }
}

/*********************************************************************
 * @fn      nvs_write
 *
 * @brief   write data into NV.
 *
 * @param   fs - file system structure. not used.
 *               set fs to NULL before call
 * @param   id - item id.
 * @param   data - buffer to write.
 * @param   len - size of data to write
 *
 * @return  len when success or 0 when fail.
 */
ssize_t nvs_write(struct nvs_fs *fs, uint16_t id, const void *data, size_t len)
{
  uint8_t rc;

  NVINTF_itemID_t itemId;
  itemId.systemID = NVINTF_SYSID_BMESH;
  itemId.itemID = id;
  itemId.subID = 0;

  rc = nvFps.writeItem(itemId, len, (void *)data);

  if(rc)
  {
    /* if write fails, return 0 */
    return 0;
  }
  else
  {
    /* if write succeed, return len */
    return len;
  }
}

/*********************************************************************
 * @fn      nvs_delete
 *
 * @brief   delete data from NV.
 *
 * @param   fs - file system structure. not used.
 *               set fs to NULL before call
 * @param   id - item id.
 *
 * @return  len when success or -EFAULT when fail.
 */
int nvs_delete(struct nvs_fs *fs, uint16_t id)
{
  uint8_t rc;
  NVINTF_itemID_t itemId;

  itemId.systemID = NVINTF_SYSID_BMESH;
  itemId.itemID = id;
  itemId.subID = 0;

  rc = nvFps.deleteItem(itemId);
  if(rc == NVINTF_LOWPOWER)
  {
    return -EFAULT;
  }
  else
  {
    return 0;
  }
}

/*********************************************************************
 * @fn      nvs_read_hist
 *
 * @brief   read data history from NV.
 *
 * @param   fs - file system structure. not used.
 *               set fs to NULL before call
 * @param   id - item id.
 * @param   data - buffer to read data into.
 * @param   len - length to read
 * @param   cnt - history number. 0 means latest. all other numbers are rejected.
 *
 * @return  len when success or other error code in errno.h when fail.
 */
ssize_t nvs_read_hist(struct nvs_fs *fs, uint16_t id, void *data, size_t len,
		      uint16_t cnt)
{
  uint8_t rc;
  NVINTF_itemID_t itemId;

  itemId.systemID = NVINTF_SYSID_BMESH;
  itemId.itemID = id;
  itemId.subID = 0;

  uint32_t itemLen = nvFps.getItemLen(itemId);
  if(cnt)
  {
    return -ENOENT;
  }

  if (itemLen <= len)
  {
    rc = nvFps.readItem(itemId, 0, itemLen, data);
    if(rc == 0)
    {
      return itemLen;
    }
    else if(rc == NVINTF_NOTFOUND)
    {
      return -ENOENT;
    }
    else
    {
      return -EINVAL;
    }
  }
  else
  {
    return -EINVAL;
  }
}

/*********************************************************************
 * @fn      nvs_read
 *
 * @brief   read data from NV.
 *
 * @param   fs - file system structure. not used.
 *               set fs to NULL before call
 * @param   id - item id.
 * @param   data - buffer to read data into.
 * @param   len - length of buffer
 *
 * @return  len when success or other error code in errno.h when fail.
 */
ssize_t nvs_read(struct nvs_fs *fs, uint16_t id, void *data, size_t len)
{
	return nvs_read_hist(fs, id, data, len, 0);;
}

/*********************************************************************
 * @fn      nvs_calc_free_space
 *
 * @brief   read available space from NV.
 *
 * @param   fs - file system structure. not used.
 *               set fs to NULL before call
 *
 * @return  size of available memory.
 */
ssize_t nvs_calc_free_space(struct nvs_fs *fs)
{
  uint16_t freespace;

  freespace = nvFps.getFreeNV();

  return (ssize_t)freespace;
}
#endif //CONFIG_BT_SETTINGS
