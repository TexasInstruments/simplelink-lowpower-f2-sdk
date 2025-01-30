/******************************************************************************

 @file  macwrapper.h

 @brief MAC Wrapper function interface defintion used by MLE.

 Group: WCS, LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2014-2025, Texas Instruments Incorporated

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

#ifndef MACWRAPPER_H
#define MACWRAPPER_H

/* ------------------------------------------------------------------------------------------------
 *                                           Includes
 * ------------------------------------------------------------------------------------------------
 */

#ifdef __cplusplus
extern "C"
{
#endif

#include "hal_types.h"
#include "api_mac.h"


/**
 * Adds a new MAC device table entry in such a way as not to hinder the incoming or outgoing
 * packets.
 *
 * It is ideal that the whole operation is done exclusive of any other MAC operation,
 * via use of critical section on the processor where MAC is implemented, in which case
 * implementation of this function will differ whether the MLE resides in the same processor
 * or in another processor.
 *
 * @param panId       PAN ID of the new device
 * @param shortAddr   short address of the new device
 * @param extAddr     extended address of the new device in little-endian
 * @param exempt      device descriptor exempt field value (TRUE or FALSE)
 * @param keyIdLookupDataSize  key ID lookup data size as it is stored in PIB,
 *                             i.e., 0 for 5 bytes, 1 for 9 bytes.
 * @param keyIdLookupData      key ID lookup data, to look for the key table entry
 *                             and create proper key device descriptor for this device.
 * @param frameCounter frame counter
 * @param uniqueDevice key device descriptor uniqueDevice field value (TRUE or FALSE)
 * @param duplicateDevFlag     a flag (TRUE or FALSE) to indicate whether the device
 *                             entry should be duplicated even for the keys that do
 *                             not match the key ID lookup data.
 *                             The device descriptors that are pointed by the key
 *                             device descriptors that do not match the key ID
 *                             lookup data shall not update the frame counter
 *                             based on the frameCounter argument to this function
 *                             or shall set the frame counter to zero when the entry
 *                             is newly created.
 */

extern uint8 macWrapperAddDevice(ApiMac_secAddDevice_t *param);

  /**
   * Removes MAC device table entries in such a way as not to hinder the incoming or outgoing
   * packet for the device table entries other than the one being removed.
   *
   * It is ideal that the whole operation is done exclusive of any other MAC operation,
   * via use of critical section on the processor where MAC is implemented, in which case
   * implementation of this function will differ whether the MLE resides in the same processor
   * or in another processor.
   *
   * @param extAddr  extended address of the device table entries that shall be removed,
   *                 in little-endian.
   * @return MAC error code
   */
  extern uint8 macWrapperDeleteDevice(ApiMac_sAddrExt_t *param);

  /**
   * Removes the key at the specified key Index and removes all
   * MAC device table enteries associated with this key. Also
   * removes(initializes) the key lookup list associated with
   * this key.
   *
   * @param keyIndex      mac secuirty key table index of the key
   *                      to be removed.
   *
   * @return              zero when successful. Non-zero,
   *                      otherwise.
   */
  extern uint8 macWrapperDeleteKeyAndAssociatedDevices( uint16 keyIndex );

  /**
   * Removes all MAC device table entries in such a way as not to hinder the incoming or outgoing
   * packet for the device table entries other than the one being removed.
   *
   * It is ideal that the whole operation is done exclusive of any other MAC operation,
   * via use of critical section on the processor where MAC is implemented, in which case
   * implementation of this function will differ whether the MLE resides in the same processor
   * or in another processor.
   *
   * @return MAC error code
   */
  extern uint8 macWrapperDeleteAllDevices(void);

  /**
   * Reads the frame counter value associated with a MAC security key
   * indexed by the designated key identifier and the default key source.
   * @param keyid         key id
   * @param pFrameCounter pointer to a buffer to store the outgoing frame counter of the key
   * @return Zero when successful. Non-zero, otherwise.
   */
  extern uint8 macWrapperGetDefaultSourceKey(uint8 keyid,
                                                     uint32 *pFrameCounter);

  /**
   * Adds the MAC security key, adds the associated lookup list
   * for the key, initializes the frame counter to the value
   * provided. It also duplicates the device table enteries
   * (associated with the previous key if any) if available based
   * on the flag dupDevFlag value and associates the device
   * descriptor with this key.
   *
   * @param pKey            pointer to the buffer containing the
   *                        key data.
   * @param frameCounter    value of the frame counter.
   * @param replaceKeyIndex internal key index of the mac security
   *                        key table where the key needs to be
   *                        written.
   * @param newKeyFlag      If set to one, the function will
   *                        duplicate the device table enteries
   *                        associated with the previous key, and
   *                        associate it with the key.
   *                        If set to zero, the function will
   *                        not alter device table entries associated
   *                        with whatever key that was stored in
   *                        the key table location as designated
   *                        by replaceKeyIndex.
   *
   * @param lookupList      list of lookup data which needs to be
   *                        associated with this key. The lookup
   *                        data should be in the following form.
   *                        count (1byte): number of lookup data,
   *                        followed by the lookup data which
   *                        consists of,
   *                        datasize (1byte): size of the lookup
   *                        data, data (actual lookup data)
   *
   * @return                zero when successful. Non-zero
   *                        otherwise.
   */
  extern uint8 macWrapperAddKeyInitFCtr( ApiMac_secAddKeyInitFrameCounter_t *param );


#ifdef __cplusplus
}
#endif

#endif /* MACWRAPPER_H */
