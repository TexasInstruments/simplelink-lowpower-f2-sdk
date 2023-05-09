/******************************************************************************

 @file  macwrapper.c

 @brief MAC Wrapper function interface defintion used by MLE.

 Group: WCS, LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2014-2023, Texas Instruments Incorporated

 All rights reserved not granted herein.
 Limited License.

 Texas Instruments Incorporated grants a world-wide, royalty-free,
 non-exclusive license under copyrights and patents it now or hereafter
 owns or controls to make, have made, use, import, offer to sell and sell
 ("Utilize") this software subject to the terms herein. With respect to the
 foregoing patent license, such license is granted solely to the extent that
 any such patent is necessary to Utilize the software alone. The patent
 license shall not apply to any combinations which include this software,
 other than combinations with devices manufactured by or for TI ("TI
 Devices"). No hardware patent is licensed hereunder.

 Redistributions must preserve existing copyright notices and reproduce
 this license (including the above copyright notice and the disclaimer and
 (if applicable) source code license limitations below) in the documentation
 and/or other materials provided with the distribution.

 Redistribution and use in binary form, without modification, are permitted
 provided that the following conditions are met:

   * No reverse engineering, decompilation, or disassembly of this software
     is permitted with respect to any software provided in binary form.
   * Any redistribution and use are licensed by TI for use only with TI Devices.
   * Nothing shall obligate TI to provide you with source code for the software
     licensed and provided to you in object code.

 If software source code is provided to you, modification and redistribution
 of the source code are permitted provided that the following conditions are
 met:

   * Any redistribution and use of the source code, including any resulting
     derivative works, are licensed by TI for use only with TI Devices.
   * Any redistribution and use of any object code compiled from the source
     code and any resulting derivative works, are licensed by TI for use
     only with TI Devices.

 Neither the name of Texas Instruments Incorporated nor the names of its
 suppliers may be used to endorse or promote products derived from this
 software without specific prior written permission.

 DISCLAIMER.

 THIS SOFTWARE IS PROVIDED BY TI AND TI'S LICENSORS "AS IS" AND ANY EXPRESS
 OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 IN NO EVENT SHALL TI AND TI'S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 ******************************************************************************
 
 
 *****************************************************************************/

/* ------------------------------------------------------------------------------------------------
 *                                           Includes
 * ------------------------------------------------------------------------------------------------
 */

#ifndef LIBRARY
#include "ti_wisunfan_features.h"
#endif

#include "macwrapper.h"

#include "hal_mcu.h"
#include "timac_api.h"
#include "mac_security_pib.h"
#include "mac_spec.h"
#include "mac_mgmt.h"
#include "mac_main.h"

/******************************************************************************
  ROM jump table will support
  1. TIMAC ROM image build
  2. TIMAC stack build without ROM image
  3. TIMAC stack with calling ROM image
 *****************************************************************************/
#include "rom_jt_154.h"

/**
 * Invalid key index to be used to mark that a specific key lookup entry is
 * not used.
 */
#define MACWRAPPER_INVALID_KEY_INDEX        0

#ifdef FEATURE_MAC_SECURITY
#if !defined(TIMAC_ROM_PATCH)
int8 macWrapper8ByteUnused(const uint8 *extaddr)
{
  int8 k;

  for (k = 0; k < 8; k++)
  {
    if (extaddr[k] != 0xff)
    {
      return 0;
    }
  }
  return 1;
}

/* See macwrapper.h for documentation */
uint8 macWrapperAddDevice(ApiMac_secAddDevice_t *param)
{
  uint16 i;
  uint8 kk;
  halIntState_t is;
  macSecurityPibDeviceEntry_t deviceEntry;
  deviceEntry.macDeviceEntry.frameCtr = MAP_osal_mem_alloc(macCfg.maxNodeKeyTableEntries * sizeof (frameCntr_t));

  HAL_ENTER_CRITICAL_SECTION(is);
  MAP_MAC_MlmeGetSecurityReq(MAC_DEVICE_TABLE_ENTRIES, &i);
  if (i == 0)
  {
    /* In case device descriptor table is not initialized,
     * initialize the device descriptor table. */
    for (i = 0; i < macCfg.maxDeviceTableEntries; i++)
    {
      deviceEntry.device_index = i;
      deviceEntry.macDeviceEntry.panID = 0xffffu;
      deviceEntry.macDeviceEntry.shortAddress = 0xffffu;
      MAP_osal_memset(deviceEntry.macDeviceEntry.extAddress, 0xff, 8);
      for (kk = 0; kk < macCfg.maxNodeKeyTableEntries; kk++)
      {
          deviceEntry.macDeviceEntry.frameCtr[kk].frameCounter = 0;
          deviceEntry.macDeviceEntry.frameCtr[kk].frameCntrIdx = 0xffff;
      }
      deviceEntry.macDeviceEntry.exempt = FALSE;
      MAP_MAC_MlmeSetSecurityReq(MAC_DEVICE_ENTRY, &deviceEntry);
    }
    MAP_MAC_MlmeSetSecurityReq(MAC_DEVICE_TABLE_ENTRIES, &i);
  }

  /* Search for the matching extended address */
  for (i = 0; i < macCfg.maxKeyTableEntries; i++)
  {
    uint8 matchingKey = FALSE, unusedKey = TRUE;
    uint16 j;

    for (j = 0; j < macCfg.maxKeyIdLookUpEntries; j++)
    {
      macSecurityPibKeyIdLookupEntry_t lookupEntry;
      uint8 lookupLen;

      lookupEntry.key_index = i;
      lookupEntry.key_id_lookup_index = j;
      if (MAP_MAC_MlmeGetSecurityReq(MAC_KEY_ID_LOOKUP_ENTRY, &lookupEntry) != MAC_SUCCESS)
      {
        continue;
      }
      if (lookupEntry.macKeyIdLookupEntry.lookupDataSize == 1 &&
          lookupEntry.macKeyIdLookupEntry.lookupData[8] == 0)
      {
        if (MAP_macWrapper8ByteUnused(lookupEntry.macKeyIdLookupEntry.lookupData))
        {
          /* This key lookup entry is invalid */
          unusedKey = TRUE;
          break;
        }
      }
      /* This key has at least one valid lookup entry */
      unusedKey = FALSE;
      if (param->keyIdLookupDataSize == 0)
      {
        /* Key Id Lookup Data length in bytes */
        lookupLen = 5;
      }
      else
      {
        lookupLen = 9;
      }
      if (lookupEntry.macKeyIdLookupEntry.lookupDataSize == param->keyIdLookupDataSize &&
          MAP_osal_memcmp(lookupEntry.macKeyIdLookupEntry.lookupData, param->keyIdLookupData,
                      lookupLen))
      {
        /* Key matches */
        matchingKey = TRUE;
        break;
      }
    }
    if (unusedKey)
    {
      continue;
    }
    if (!matchingKey && !param->duplicateDevFlag)
    {
      /* No need to create a device entry for this key */
      continue;
    }

    /* Now search for the key device table entries for the matching device */
    for (j = 0; j < macCfg.maxKeyDeviceTableEntries; j++)
    {
      macSecurityPibKeyDeviceEntry_t keyDeviceEntry;
      keyDeviceEntry.key_index = i;
      keyDeviceEntry.key_device_index = j;
      if (MAP_MAC_MlmeGetSecurityReq(MAC_KEY_DEVICE_ENTRY, &keyDeviceEntry) != MAC_SUCCESS ||
          keyDeviceEntry.macKeyDeviceEntry.deviceDescriptorHandle == 0xffff)
      {
        continue;
      }
      deviceEntry.device_index = keyDeviceEntry.macKeyDeviceEntry.deviceDescriptorHandle;
      if (MAP_MAC_MlmeGetSecurityReq(MAC_DEVICE_ENTRY, &deviceEntry) != MAC_SUCCESS)
      {
        /* Security PIB is corrupt */
        HAL_EXIT_CRITICAL_SECTION(is);
        MAP_osal_mem_free(deviceEntry.macDeviceEntry.frameCtr);
        return MAC_BAD_STATE;
      }
      if (MAP_osal_memcmp(deviceEntry.macDeviceEntry.extAddress, param->extAddr, 8))
      {
        /* Matching device */
        /* Update the device descriptor */
        deviceEntry.macDeviceEntry.panID = param->panID;
        deviceEntry.macDeviceEntry.shortAddress = param->shortAddr;
        if (matchingKey)
        {
          deviceEntry.macDeviceEntry.exempt = param->exempt;
          for(kk = 0; kk < macCfg.maxNodeKeyTableEntries; kk++)
          {
            if((deviceEntry.macDeviceEntry.frameCtr[kk].frameCntrIdx == 0xffff)
               || (deviceEntry.macDeviceEntry.frameCtr[kk].frameCntrIdx == i))
            {
              break;
            }
          }
          if(kk < macCfg.maxNodeKeyTableEntries)
          {
              deviceEntry.macDeviceEntry.frameCtr[kk].frameCounter = param->frameCounter;
              deviceEntry.macDeviceEntry.frameCtr[kk].frameCntrIdx = i;
          }
          else
          {
              /* PIB is corrupt */
              HAL_EXIT_CRITICAL_SECTION(is);
              MAP_osal_mem_free(deviceEntry.macDeviceEntry.frameCtr);
              return MAC_BAD_STATE;
          }
        }
        MAP_MAC_MlmeSetSecurityReq(MAC_DEVICE_ENTRY, &deviceEntry);
        /* Update the key device descriptor */
        if (matchingKey && (keyDeviceEntry.macKeyDeviceEntry.uniqueDevice != param->uniqueDevice))
        {
          keyDeviceEntry.macKeyDeviceEntry.uniqueDevice = param->uniqueDevice;
          MAP_MAC_MlmeSetSecurityReq(MAC_KEY_DEVICE_ENTRY, &keyDeviceEntry);
        }
        break;
      }
    }

    if (j == macCfg.maxKeyDeviceTableEntries)
    {
      /* Matching device is not found. Add a new device descriptor
       * and key device descriptor */
      for (j = 0; j < macCfg.maxDeviceTableEntries; j++)
      {
        uint16 k;

        deviceEntry.device_index = j;
        if (MAP_MAC_MlmeGetSecurityReq(MAC_DEVICE_ENTRY, &deviceEntry) != MAC_SUCCESS)
        {
          /* PIB is corrupt */
          HAL_EXIT_CRITICAL_SECTION(is);
          MAP_osal_mem_free(deviceEntry.macDeviceEntry.frameCtr);
          return MAC_BAD_STATE;
        }

        if (MAP_macWrapper8ByteUnused(deviceEntry.macDeviceEntry.extAddress) ||
            MAP_osal_memcmp(deviceEntry.macDeviceEntry.extAddress, param->extAddr, 8))
        {
          /* Empty slot found */
          deviceEntry.macDeviceEntry.panID = param->panID;
          deviceEntry.macDeviceEntry.shortAddress = param->shortAddr;
          deviceEntry.macDeviceEntry.exempt = param->exempt;
          for(kk = 0; kk < macCfg.maxNodeKeyTableEntries; kk++)
          {
              if((deviceEntry.macDeviceEntry.frameCtr[kk].frameCntrIdx == 0xffff)
                 || (deviceEntry.macDeviceEntry.frameCtr[kk].frameCntrIdx == i))
              {
                  break;
              }
          }
          if(kk < macCfg.maxNodeKeyTableEntries)
          {
              if (matchingKey)
              {
                deviceEntry.macDeviceEntry.frameCtr[kk].frameCounter = param->frameCounter;
              }
              else
              {
                  deviceEntry.macDeviceEntry.frameCtr[kk].frameCounter = 0;
              }
              deviceEntry.macDeviceEntry.frameCtr[kk].frameCntrIdx = i;
          }
          else
          {
            /* Empty slot was not found */
            HAL_EXIT_CRITICAL_SECTION(is);
            MAP_osal_mem_free(deviceEntry.macDeviceEntry.frameCtr);
            return MAC_NO_RESOURCES;
          }

          MAP_osal_memcpy(deviceEntry.macDeviceEntry.extAddress, param->extAddr, 8);

#ifdef FEATURE_FREQ_HOP_MODE
          /* call back for FH add device */
          if ( (MAC_FH_ENABLED) && (MAC_FH_ADD_DEVICE_FN) )
          {
            MAC_FH_ADD_DEVICE_FN((sAddrExt_t *)param->extAddr, deviceEntry.device_index);
          }
#endif /* FEATURE_FREQ_HOP_MODE */

          /* Look for an empty slot in key device descriptor table */
          for (k = 0; k < macCfg.maxKeyDeviceTableEntries; k++)
          {
            macSecurityPibKeyDeviceEntry_t keyDeviceEntry;

            keyDeviceEntry.key_index = i;
            keyDeviceEntry.key_device_index = k;

            if (MAP_MAC_MlmeGetSecurityReq(MAC_KEY_DEVICE_ENTRY, &keyDeviceEntry) ==
                MAC_INVALID_PARAMETER ||
                keyDeviceEntry.macKeyDeviceEntry.deviceDescriptorHandle == 0xffff)
            {
              /* Empty slot found */
              keyDeviceEntry.macKeyDeviceEntry.deviceDescriptorHandle = deviceEntry.device_index;
              keyDeviceEntry.macKeyDeviceEntry.uniqueDevice = param->uniqueDevice;
              keyDeviceEntry.macKeyDeviceEntry.blackListed = FALSE;

              MAP_MAC_MlmeSetSecurityReq(MAC_DEVICE_ENTRY, &deviceEntry);
              MAP_MAC_MlmeSetSecurityReq(MAC_KEY_DEVICE_ENTRY, &keyDeviceEntry);
              break;
            }
          }
          if (k < macCfg.maxDeviceTableEntries)
          {
            /* Empty slot was found */
            break;
          }
        }
      }
      if (j == macCfg.maxDeviceTableEntries)
      {
        /* Empty slot was not found */
        HAL_EXIT_CRITICAL_SECTION(is);
        MAP_osal_mem_free(deviceEntry.macDeviceEntry.frameCtr);
        return MAC_NO_RESOURCES;
      }
      else
      {
          /*break from key_index loop */
          break;
      }
    }
  }
  HAL_EXIT_CRITICAL_SECTION(is);
  MAP_osal_mem_free(deviceEntry.macDeviceEntry.frameCtr);
  return MAC_SUCCESS;
}

/* See macwrapper.h for documentation */
uint8 macWrapperDeleteDevice(ApiMac_sAddrExt_t *param)
{
  uint16 i;
  halIntState_t is;
  macSecurityPibDeviceEntry_t deviceEntry;
  deviceEntry.macDeviceEntry.frameCtr = MAP_osal_mem_alloc(macCfg.maxNodeKeyTableEntries * sizeof (frameCntr_t));

  HAL_ENTER_CRITICAL_SECTION(is);

  /* Search for the matching extended address */
  for (i = 0; i < macCfg.maxKeyTableEntries; i++)
  {
    uint8 j, unusedKey = TRUE;

    /* Check if the key is used */
    for (j = 0; j < macCfg.maxKeyIdLookUpEntries; j++)
    {
      macSecurityPibKeyIdLookupEntry_t lookupEntry;

      lookupEntry.key_index = i;
      lookupEntry.key_id_lookup_index = j;
      if (MAP_MAC_MlmeGetSecurityReq(MAC_KEY_ID_LOOKUP_ENTRY, &lookupEntry) != MAC_SUCCESS)
      {
        /* If none of the key ID look up entries associated with a key are initialized,
         * the entry is considered as unused.
         * Hence the default value of unusedKey was set to TRUE above.
         */
        continue;
      }
      if (lookupEntry.macKeyIdLookupEntry.lookupDataSize == 1 &&
          lookupEntry.macKeyIdLookupEntry.lookupData[8] == 0)
      {
        uint8 k;

        for (k = 0; k < 8; k++)
        {
          if (lookupEntry.macKeyIdLookupEntry.lookupData[k] != 0xff)
          {
            break;
          }
        }
        if (k == 8)
        {
          /* This key lookup entry is marked to indicate unused key.
           * If any single lookup entry per key has this mark,
           * the key is considered as unused regardless of the state of
           * other key lookup entries associated with the same key.
           */
          unusedKey = TRUE;
          break;
        }
      }
      /* This key has at least one valid lookup entry,
       * which tentatively indicates that the key may be in use,
       * unless there is another lookup entry associated with the key
       * which has the mark of unused key.
       */
      unusedKey = FALSE;
    }
    if (unusedKey)
    {
      continue;
    }

    /* Now search for the key device table entries for the matching device */
    for (j = 0; j < macCfg.maxDeviceTableEntries; j++)
    {
      macSecurityPibKeyDeviceEntry_t keyDeviceEntry;

      keyDeviceEntry.key_index = i;
      keyDeviceEntry.key_device_index = j;
      if (MAP_MAC_MlmeGetSecurityReq(MAC_KEY_DEVICE_ENTRY, &keyDeviceEntry) != MAC_SUCCESS ||
          keyDeviceEntry.macKeyDeviceEntry.deviceDescriptorHandle == 0xffff)
      {
        continue;
      }
      deviceEntry.device_index = keyDeviceEntry.macKeyDeviceEntry.deviceDescriptorHandle;
      if (MAP_MAC_MlmeGetSecurityReq(MAC_DEVICE_ENTRY, &deviceEntry) != MAC_SUCCESS)
      {
        /* Security PIB is corrupt */
        HAL_EXIT_CRITICAL_SECTION(is);
        MAP_osal_mem_free(deviceEntry.macDeviceEntry.frameCtr);
        return MAC_BAD_STATE;
      }
      if (MAP_osal_memcmp(deviceEntry.macDeviceEntry.extAddress, param, 8))
      {
        /* Matching device */

        /* Update the device descriptor */
        deviceEntry.macDeviceEntry.panID = 0xffffu;
        deviceEntry.macDeviceEntry.shortAddress = 0xffffu;
        MAP_osal_memset(deviceEntry.macDeviceEntry.extAddress, 0xff, 8);
        MAP_MAC_MlmeSetSecurityReq(MAC_DEVICE_ENTRY, &deviceEntry);

        /* Update the key device descriptor */
        keyDeviceEntry.macKeyDeviceEntry.deviceDescriptorHandle = 0xffff;
        MAP_MAC_MlmeSetSecurityReq(MAC_KEY_DEVICE_ENTRY, &keyDeviceEntry);
        break;
      }
      if (MAP_macWrapper8ByteUnused(deviceEntry.macDeviceEntry.extAddress))
      {
        /* Device entry is already cleared. It would happen for i > 0,
         * since the entry must have been cleared at i = 0, iteration. */
        keyDeviceEntry.macKeyDeviceEntry.deviceDescriptorHandle = 0xffff;
        MAP_MAC_MlmeSetSecurityReq(MAC_KEY_DEVICE_ENTRY, &keyDeviceEntry);
      }
    }
  }
  HAL_EXIT_CRITICAL_SECTION(is);
  MAP_osal_mem_free(deviceEntry.macDeviceEntry.frameCtr);
  return MAC_SUCCESS;
}

/* See macwrapper.h for documentation */
uint8 macWrapperDeleteKeyAndAssociatedDevices( uint16 keyIndex )
{
  uint8 j, kk;
  halIntState_t is;
  macSecurityPibDeviceEntry_t deviceEntry;
  deviceEntry.macDeviceEntry.frameCtr = MAP_osal_mem_alloc(macCfg.maxNodeKeyTableEntries * sizeof (frameCntr_t));

  HAL_ENTER_CRITICAL_SECTION(is);

  /* Set the key lookup list associated with this key to default value */
  for (j = 0; j < macCfg.maxKeyIdLookUpEntries; j++)
  {
    macSecurityPibKeyIdLookupEntry_t lookupEntry;

    lookupEntry.key_index = keyIndex;
    lookupEntry.key_id_lookup_index = j;
    if ( MAP_MAC_MlmeGetSecurityReq(MAC_KEY_ID_LOOKUP_ENTRY, &lookupEntry) != MAC_SUCCESS )
    {
      continue;
    }

    if ( (lookupEntry.macKeyIdLookupEntry.lookupDataSize == 1 &&
          lookupEntry.macKeyIdLookupEntry.lookupData[MAC_KEY_LOOKUP_LONG_LEN - 1] !=
            MACWRAPPER_INVALID_KEY_INDEX)
         || (lookupEntry.macKeyIdLookupEntry.lookupDataSize == 0 &&
             lookupEntry.macKeyIdLookupEntry.lookupData[MAC_KEY_LOOKUP_SHORT_LEN - 1] !=
               MACWRAPPER_INVALID_KEY_INDEX) )
    {
      uint8 k;
      uint8 size = 4;
      if (lookupEntry.macKeyIdLookupEntry.lookupDataSize)
      {
        size = 8;
      }
      for( k = 0; k < size; k++ )
      {
        lookupEntry.macKeyIdLookupEntry.lookupData[k] = 0xff;
      }
      lookupEntry.macKeyIdLookupEntry.lookupData[k] = 0x00;
      MAP_MAC_MlmeSetSecurityReq(MAC_KEY_ID_LOOKUP_ENTRY, &lookupEntry);
    }
  }

  /* Search for the device table enteries associated with this key and delete (re-initialize) the entry */
  for (j = 0; j < macCfg.maxKeyDeviceTableEntries; j++)
  {
    macSecurityPibKeyDeviceEntry_t keyDeviceEntry;

    keyDeviceEntry.key_index = keyIndex;
    keyDeviceEntry.key_device_index = j;
    if (MAP_MAC_MlmeGetSecurityReq(MAC_KEY_DEVICE_ENTRY, &keyDeviceEntry) != MAC_SUCCESS ||
        keyDeviceEntry.macKeyDeviceEntry.deviceDescriptorHandle == 0xffff)
    {
      continue;
    }
    deviceEntry.device_index = keyDeviceEntry.macKeyDeviceEntry.deviceDescriptorHandle;
    if (MAP_MAC_MlmeGetSecurityReq(MAC_DEVICE_ENTRY, &deviceEntry) != MAC_SUCCESS)
    {
      /* Security PIB is corrupt */
      HAL_EXIT_CRITICAL_SECTION(is);
      MAP_osal_mem_free(deviceEntry.macDeviceEntry.frameCtr);
      return MAC_BAD_STATE;
    }

    /* Valid device */
    /* Update the device descriptor only when the other key
     * is not using the device. */
    {
      uint16 k;
      for (k = 0; k < macCfg.maxKeyDeviceTableEntries; k++)
      {
        keyDeviceEntry.key_index = keyIndex; //^= 1;
        keyDeviceEntry.key_device_index = k;
        if (MAP_MAC_MlmeGetSecurityReq(MAC_KEY_DEVICE_ENTRY, &keyDeviceEntry)
            == MAC_SUCCESS &&
            keyDeviceEntry.macKeyDeviceEntry.deviceDescriptorHandle
            == deviceEntry.device_index)
        {
          break;
        }
      }
      if (k == macCfg.maxKeyDeviceTableEntries)
      {
        deviceEntry.macDeviceEntry.panID = 0xffffu;
        deviceEntry.macDeviceEntry.shortAddress = 0xffffu;
        MAP_osal_memset(deviceEntry.macDeviceEntry.extAddress, 0xff, 8);
        for(kk = 0; kk < macCfg.maxNodeKeyTableEntries; kk++)
        {
          if(deviceEntry.macDeviceEntry.frameCtr[kk].frameCntrIdx == keyIndex)
          {
            break;
          }
        }
        if(kk < macCfg.maxNodeKeyTableEntries )
        {
          deviceEntry.macDeviceEntry.frameCtr[kk].frameCounter = 0;
          deviceEntry.macDeviceEntry.frameCtr[kk].frameCntrIdx = 0xffff;
          //deviceEntry.macDeviceEntry.frameCounter[keyIndex] = 0;
          MAP_MAC_MlmeSetSecurityReq(MAC_DEVICE_ENTRY, &deviceEntry);
        }
        else
        {
            HAL_EXIT_CRITICAL_SECTION(is);
            MAP_osal_mem_free(deviceEntry.macDeviceEntry.frameCtr);
            return MAC_BAD_STATE;
        }
      }
    }

    /* No need to update Key Device Descriptor..it will be updated later */
  }
  HAL_EXIT_CRITICAL_SECTION(is);
  MAP_osal_mem_free(deviceEntry.macDeviceEntry.frameCtr);
  return MAC_SUCCESS;
} /* macWrapperDeleteKeyAndAssociatedDevices() */


/* See macwrapper.h for documentation */
uint8 macWrapperDeleteAllDevices(void)
{
  uint16 i;
  halIntState_t is;
  macSecurityPibDeviceEntry_t deviceEntry;
  deviceEntry.macDeviceEntry.frameCtr = MAP_osal_mem_alloc(macCfg.maxNodeKeyTableEntries * sizeof (frameCntr_t));

  HAL_ENTER_CRITICAL_SECTION(is);

  /* Search for the valid device descriptors */
  for (i = 0; i < macCfg.maxKeyTableEntries; i++)
  {
    uint8 j, unusedKey = TRUE;

    /* Check if the key is used */
    for (j = 0; j < macCfg.maxKeyIdLookUpEntries; j++)
    {
      macSecurityPibKeyIdLookupEntry_t lookupEntry;

      lookupEntry.key_index = i;
      lookupEntry.key_id_lookup_index = j;
      if (MAP_MAC_MlmeGetSecurityReq(MAC_KEY_ID_LOOKUP_ENTRY, &lookupEntry) != MAC_SUCCESS)
      {
        /* If none of the key ID look up entries associated with a key are initialized,
         * the entry is considered as unused.
         * Hence the default value of unusedKey was set to TRUE above.
         */
        continue;
      }
      if (lookupEntry.macKeyIdLookupEntry.lookupDataSize == 1 &&
          lookupEntry.macKeyIdLookupEntry.lookupData[MAC_KEY_LOOKUP_LONG_LEN - 1] ==
            MACWRAPPER_INVALID_KEY_INDEX)
      {
        uint8 k;

        for (k = 0; k < 8; k++)
        {
          if (lookupEntry.macKeyIdLookupEntry.lookupData[k] != 0xff)
          {
            break;
          }
        }
        if (k == 8)
        {
          /* This key lookup entry is marked to indicate unused key.
           * If any single lookup entry per key has this mark,
           * the key is considered as unused regardless of the state of
           * other key lookup entries associated with the same key.
           */
          unusedKey = TRUE;
          break;
        }
      }
      /* This key has at least one valid lookup entry,
       * which tentatively indicates that the key may be in use,
       * unless there is another lookup entry associated with the key
       * which has the mark of unused key.
       */
      unusedKey = FALSE;
    }
    if (unusedKey)
    {
      continue;
    }

    /* Now search for the key device table entries for the matching device */
    for (j = 0; j < macCfg.maxKeyDeviceTableEntries; j++)
    {
      macSecurityPibKeyDeviceEntry_t keyDeviceEntry;

      keyDeviceEntry.key_index = i;
      keyDeviceEntry.key_device_index = j;
      if (MAP_MAC_MlmeGetSecurityReq(MAC_KEY_DEVICE_ENTRY, &keyDeviceEntry) != MAC_SUCCESS ||
          keyDeviceEntry.macKeyDeviceEntry.deviceDescriptorHandle == 0xffff)
      {
        continue;
      }
      deviceEntry.device_index = keyDeviceEntry.macKeyDeviceEntry.deviceDescriptorHandle;
      if (MAP_MAC_MlmeGetSecurityReq(MAC_DEVICE_ENTRY, &deviceEntry) != MAC_SUCCESS)
      {
        /* Security PIB is corrupt */
        HAL_EXIT_CRITICAL_SECTION(is);
        MAP_osal_mem_free(deviceEntry.macDeviceEntry.frameCtr);
        return MAC_BAD_STATE;
      }
      /* Valid device */

      /* Update the device descriptor */
      deviceEntry.macDeviceEntry.panID = 0xffffu;
      deviceEntry.macDeviceEntry.shortAddress = 0xffffu;
      MAP_osal_memset(deviceEntry.macDeviceEntry.extAddress, 0xff, 8);
      MAP_MAC_MlmeSetSecurityReq(MAC_DEVICE_ENTRY, &deviceEntry);

      /* Update the key device descriptor */
      keyDeviceEntry.macKeyDeviceEntry.deviceDescriptorHandle = 0xffff;
      MAP_MAC_MlmeSetSecurityReq(MAC_KEY_DEVICE_ENTRY, &keyDeviceEntry);
    }
  }
  HAL_EXIT_CRITICAL_SECTION(is);
  MAP_osal_mem_free(deviceEntry.macDeviceEntry.frameCtr);
  return MAC_SUCCESS;
}
#endif //#if !defined(TIMAC_ROM_PATCH)

#if !defined(TIMAC_ROM_PATCH)

/* See macwrapper.h for documentation */
uint8 macWrapperAddKeyInitFCtr( ApiMac_secAddKeyInitFrameCounter_t *param )
{
  halIntState_t is;

  macSecurityPibKeyEntry_t mtKeyEntry;
  macSecurityPibKeyIdLookupEntry_t mtKeyIdLookupEntry;
  uint16 numKeys;
  uint16 i;
  uint8 kk;
  unsigned char result;
  uint8 lookupList[MAC_MAX_KEY_LOOKUP_LEN+2];
  macSecurityPibDeviceEntry_t deviceEntry;
  deviceEntry.macDeviceEntry.frameCtr = MAP_osal_mem_alloc(macCfg.maxNodeKeyTableEntries * sizeof (frameCntr_t));

  /* The byte that indicates number of lookup entries is skipped to
   * set the following pointer to the start of a lookup entry. */
  uint8* pLookUpData = &lookupList[1];

  lookupList[0] = 1;
  lookupList[1] = param->lookupDataSize;
  MAP_osal_memcpy( &lookupList[2], param->lookupData, MAC_MAX_KEY_LOOKUP_LEN);

  HAL_ENTER_CRITICAL_SECTION( is );

  MAP_MAC_MlmeGetSecurityReq(MAC_KEY_TABLE_ENTRIES, &numKeys);

  /* If the security table has not been updated previously, update it now.
   * It can have only two entries one per key */
  if ( param->newKeyFlag )
  {

    {
      macSecurityPibKeyUsageEntry_t mtKeyUsageEntry = {0, 0, {MAC_FRAME_TYPE_DATA, MAC_DATA_REQ_FRAME}};

      /* Set the key usage for this key */
      mtKeyUsageEntry.key_index = param->replaceKeyIndex;
      mtKeyUsageEntry.key_key_usage_index = 0;
      MAP_MAC_MlmeSetSecurityReq(MAC_KEY_USAGE_ENTRY, &mtKeyUsageEntry);

    }


    /* Need to duplicate the key Device Table Entries from the other key */
    for ( i = 0; i < macCfg.maxKeyDeviceTableEntries; i++)
    {
      macSecurityPibKeyDeviceEntry_t mtKeyDeviceEntry;

      mtKeyDeviceEntry.key_index = param->replaceKeyIndex; // ^ 1;
      mtKeyDeviceEntry.key_device_index = i;

      if (MAP_MAC_MlmeGetSecurityReq(MAC_KEY_DEVICE_ENTRY, &mtKeyDeviceEntry) != MAC_SUCCESS ||
          mtKeyDeviceEntry.macKeyDeviceEntry.deviceDescriptorHandle == 0xffff)
      {
        mtKeyDeviceEntry.macKeyDeviceEntry.deviceDescriptorHandle = 0xffff;
      }
      else
      {
        /* Update the frame counter for the device descriptor */
        deviceEntry.device_index =
            mtKeyDeviceEntry.macKeyDeviceEntry.deviceDescriptorHandle;

        if (MAP_MAC_MlmeGetSecurityReq(MAC_DEVICE_ENTRY, &deviceEntry)
            != MAC_SUCCESS)
        {
          /* Security PIB is corrupt */
          MAP_osal_mem_free(deviceEntry.macDeviceEntry.frameCtr);
          HAL_EXIT_CRITICAL_SECTION(is);
          return MAC_BAD_STATE;
        }

        /* Valid device */
        /* Frame counter for the device descriptor for the replaceKeyIndex is
         * set to 0x00 to indicate a new key */
        for(kk = 0; kk < macCfg.maxNodeKeyTableEntries; kk++)
        {
            if((deviceEntry.macDeviceEntry.frameCtr[kk].frameCntrIdx == 0xffff)
               || (deviceEntry.macDeviceEntry.frameCtr[kk].frameCntrIdx == i))
                break;
        }
        if(kk == macCfg.maxNodeKeyTableEntries )
        {
            /* Security PIB is corrupt */
            MAP_osal_mem_free(deviceEntry.macDeviceEntry.frameCtr);
            HAL_EXIT_CRITICAL_SECTION(is);
            return MAC_BAD_STATE;
        }
        deviceEntry.macDeviceEntry.frameCtr[kk].frameCounter = 0;
        deviceEntry.macDeviceEntry.frameCtr[kk].frameCntrIdx = param->replaceKeyIndex;

        /* Update the device descriptor */
        MAP_MAC_MlmeSetSecurityReq(MAC_DEVICE_ENTRY, &deviceEntry);
      }

      /* update the key device entry for this key and device */
      mtKeyDeviceEntry.key_index = param->replaceKeyIndex;
      mtKeyDeviceEntry.macKeyDeviceEntry.blackListed = FALSE;
      mtKeyDeviceEntry.macKeyDeviceEntry.uniqueDevice = FALSE;

      result =  MAP_MAC_MlmeSetSecurityReq(MAC_KEY_DEVICE_ENTRY, &mtKeyDeviceEntry);
    }
  }

  /* Compile flag to compile code piece that supports only one lookup
   * entry but minimize the code size in such a way. */
  /* Initialize the key Id look up data  */
  mtKeyIdLookupEntry.key_index = param->replaceKeyIndex;
  mtKeyIdLookupEntry.key_id_lookup_index = 0;

  MAP_osal_memcpy( mtKeyIdLookupEntry.macKeyIdLookupEntry.lookupData, &pLookUpData[1], pLookUpData[0] );
  mtKeyIdLookupEntry.macKeyIdLookupEntry.lookupDataSize = 0x01;

  result = MAP_MAC_MlmeSetSecurityReq(MAC_KEY_ID_LOOKUP_ENTRY, &mtKeyIdLookupEntry);
  if ( result != MAC_SUCCESS )
  {
    MAP_osal_mem_free(deviceEntry.macDeviceEntry.frameCtr);
    HAL_EXIT_CRITICAL_SECTION(is);
    return result;
  }
  /* Add the key entry */
  mtKeyEntry.key_index = param->replaceKeyIndex;
  MAP_osal_memcpy( mtKeyEntry.keyEntry, param->key, MAC_KEY_MAX_LEN );
  mtKeyEntry.frameCounter = param->frameCounter;

  result = MAP_MAC_MlmeSetSecurityReq(MAC_KEY_ENTRY, &mtKeyEntry);
  if ( result != MAC_SUCCESS )
  {
    MAP_osal_mem_free(deviceEntry.macDeviceEntry.frameCtr);
    HAL_EXIT_CRITICAL_SECTION(is);
    return result;
  }

  MAP_osal_mem_free(deviceEntry.macDeviceEntry.frameCtr);
  HAL_EXIT_CRITICAL_SECTION(is);
  return result;
}/* macWrapperAddKeyInitFCtr() */
#endif //#if !defined(TIMAC_ROM_PATCH)

#endif /* FEATURE_MAC_SECURITY */
