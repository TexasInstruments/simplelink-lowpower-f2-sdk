/******************************************************************************

 @file  mac_security_pib.h

 @brief Internal interface file for the Security-related MAC PIB module.

 Group: WCS, LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2010-2025, Texas Instruments Incorporated

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

#ifndef MAC_SECURITY_PIB_H
#define MAC_SECURITY_PIB_H

/* ------------------------------------------------------------------------------------------------
 *                                          Includes
 * ------------------------------------------------------------------------------------------------
 */

#include "timac_api.h"
#include "mac_high_level.h"

/* ------------------------------------------------------------------------------------------------
 *                                           Constants
 * ------------------------------------------------------------------------------------------------
 */

/* Invalid security PIB table index used for error code */
//#define MAC_SECURITY_PIB_INVALID     ((uint8) (sizeof(macSecurityPibTbl) / sizeof(macSecurityPibTbl[0])))
#define MAC_SECURITY_PIB_INVALID     (0xFF)
/* ------------------------------------------------------------------------------------------------
 *                                           Typedefs
 * ------------------------------------------------------------------------------------------------
 */

/* Security related PIB access and min/max table type */
typedef struct
{
  uint8     offset;
  uint16     len;
  uint8     min;
  uint16     max;
} macSecurityPibTbl_t;


/* Security-related MAC PIB type */
typedef struct
{
  uint16             keyTableEntries;
  uint16             deviceTableEntries;
  uint8              securityLevelTableEntries;

  uint8              autoRequestSecurityLevel;
  uint8              autoRequestKeyIdMode;
  uint8              autoRequestKeySource[MAC_KEY_SOURCE_MAX_LEN];
  uint8              autoRequestKeyIndex;

  uint8              defaultKeySource[MAC_KEY_SOURCE_MAX_LEN];
  sAddr_t            panCoordExtendedAddress;
  uint16             panCoordShortAddress;

  /* Propriority Security PIBs */
  keyDescriptor_t            macKeyTable[MAX_KEY_TABLE_ENTRIES];
  deviceDescriptor_t         *macDeviceTable;
  // deviceDescriptor_t         macDeviceTable[MAX_DEVICE_TABLE_ENTRIES];
  keyDeviceDescriptor_t      *macKeyDeviceList;
  // keyDeviceDescriptor_t      macKeyDeviceList[MAX_KEY_TABLE_ENTRIES][MAX_DEVICE_TABLE_ENTRIES];
  keyIdLookupDescriptor_t    macKeyIdLookupList[MAX_KEY_TABLE_ENTRIES];
  keyUsageDescriptor_t       macKeyUsageList[MAX_KEY_USAGE_TABLE_ENTRIES];
  securityLevelDescriptor_t  macSecurityLevelTable[MAX_SECURITY_LEVEL_TABLE_ENTRIES];
} macSecurityPib_t;

/* ------------------------------------------------------------------------------------------------
 *                                           Function Prototypes
 * ------------------------------------------------------------------------------------------------
 */

MAC_INTERNAL_API void macSecurityPibReset(void);
MAC_INTERNAL_API uint8 MAC_MlmeGetPointerSecurityReq(uint8 pibAttribute, void **pValue);
MAC_INTERNAL_API uint8 macSecurityPibIndex(uint8 pibAttribute);

/**************************************************************************************************
*/

#endif /* MAC_SECURITY_PIB_H */

