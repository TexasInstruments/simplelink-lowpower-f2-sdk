/******************************************************************************
 Filename:       zcl_port.h
 Revised:        $Date: 2015-02-12 12:55:11 -0800 (Thu, 12 Feb 2015) $
 Revision:       $Revision: 42532 $

 Description:    This file contains the the protypes and definitions
                 needed for the ZCL port.

  Copyright (c) 2019, Texas Instruments Incorporated
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
 *****************************************************************************/

#ifndef ZCL_PORT_H
#define ZCL_PORT_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "af.h"
#include "rom_jt_154.h"
#include "nvintf.h"
#include "zstack.h"
#include "zcl.h"
#include "zcomdef.h"
#include "ti_zstack_config.h"

/**
 \defgroup ZclPort ZCL Porting Layer
 <BR>
 This module is a collection of functions used to make the ZStack ZCL module
 portable between OSAL, Linux and TI-RTOS.
 <BR>
 The ZCL parsing/building functions, along with profile code, will call these
 functions and this module is responsible for performing the translation
 or function requested.
 <BR>
 The following APIs are functions that the Application needs to call to pass
 information that this porting layer needs.  The other functions in this module
 are called by ZCL, not by the application.
 <BR>
 */

/**
 * \ingroup ZclPort
 * @{
 */


/**
 * @brief   Function pointer definition to callback ZCL handler external function.
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  TRUE if you've handle it, FALSE if not
 */
typedef uint8_t (*zclport_pFnZclHandleExternal)(zclIncoming_t *pInMsg);

/** @} end group ZclPort */

/**
 * \ingroup ZclPort
 * @{
 */

/**
 * @brief   Register an AF Endpoint.  This is needed by the ZCL code
 *          to find an AF endpoint descriptor.
 *
 * @param   entity - The task entity ID.
 * @param   *pEpDesc - Pointer to an endpoint Descriptor.
 *
 * @return  true if added to table, false if table full
 */
extern bool zclport_registerEndpoint(uint8_t  entity,
                                     endPointDesc_t *pEpDesc);

/**
 * @brief   Call to register the function pointers for the NV driver.
 *
 * @param   pfnNV - pointer to the NV driver functions
 * @param   sceneNVID - Scene Table NV ID
 */
extern void zclport_registerNV(NVINTF_nvFuncts_t *pfnNV, uint16_t sceneNVID);

/**
 * @brief   Call to register a function pointer to handle
 *          zcl_HandleExternal() messages.
 *
 * @param   endpoint - endpoint ID
 * @param   pfn - pointer to the function that handles ZCL External messages.
 *
 * @return  true if able to regiester, otherwise false
 */
extern bool zclport_registerZclHandleExternal( uint8_t endpoint, zclport_pFnZclHandleExternal pfn);

/**
 * @brief   Call to get Device Information.
 *
 * @param  entity - The task entity ID.
 *
 * @return  pointer to structure containing info
 */
extern zstack_sysNwkInfoReadRsp_t *zclport_getDeviceInfo(uint8_t entity);

/**
 * @brief   Determines if the device is already part of a network by
 *          asking the stack thread.
 *          <BR>
 *          "Part of the Network" means that the device's network state is
 *           that of an end device or router/coordinator in a network, and
 *           not initialized or in an intermediate state (like scannning,
 *           joining or rejoining).  A good use of this function would be
 *           to call this function before starting the device, to
 *           "pre"-determine if the device needs to join a network or just
 *           start from the parameters already in NV.
 *
 * @param  entity - The task entity ID.
 *
 * @return  true if already part of a network, false if not
 */
extern bool zclport_isAlreadyPartOfNetwork(uint8_t entity);

/**
 * @brief   If the NV item does not already exist, it is created and
 *          initialized with the data passed to the function, if any.
 *
 * @param   id - NV ID
 * @param   subId - NV ID
 * @param   len - length of NV item
 * @param   buf - pointer to NV item
 *
 * @return  status
 */
extern uint8_t zclport_initializeNVItem(uint16_t id, uint16_t subId,
                                        uint16_t len,
                                        void *buf);

/**
 * @brief   Write a data item to NV. Function can write an entire item to NV or
 *          an element of an item by indexing into the item with an offset.
 *
 * @param   id - NV ID
 * @param   subId - NV ID
 * @param   len - length of NV item to write
 * @param   buf - pointer to NV item
 *
 * @return  status
 */
extern uint8_t zclport_writeNV(uint16_t id, uint16_t subId,
                               uint16_t len,
                               void *buf);

/**
 * @brief   Read data from NV. This function can be used to read an entire item
 *          from NV or an element of an item by indexing into the item with an
 *          offset. Read data is copied into buf.
 *
 * @param   id - NV ID
 * @param   subId - NV ID
 * @param   ndx - offset into NV item
 * @param   len - length of NV item to read
 * @param   buf - pointer to NV item
 *
 * @return  status
 */
extern uint8_t zclport_readNV(uint16_t id, uint16_t subId, uint16_t ndx,
                              uint16_t len,
                              void *buf);

/********************************************************************
 * @fn      zclport_deleteNV
 *
 * @brief   Delete item from NV. This function will fail if the length
 *          parameter does not match the length of the item in NV.
 *
 * @param   id  - Valid NV item Id.
 * @param   subId - Valid NV item sub Id.
 * @param   len - Length of item to delete.
 *
 * @return  status
 */
extern uint8_t zclport_deleteNV(uint16_t id, uint16_t subId, uint16_t len);

extern afStatus_t zcl_AF_DataRequest(afAddrType_t *dstAddr, endPointDesc_t *srcEP,
                          uint16_t cID, uint16_t bufLen, uint8_t *buf,
                          uint8_t *transID, uint8_t options,
                          uint8_t radius);

/**
 *
 * @brief   Add a group for an endpoint
 *
 * @param   endpoint -
 * @param   group - new group
 *
 * @return  ZStatus_t
 */
ZStatus_t zclport_aps_AddGroup(uint8_t endpoint, aps_Group_t *group);

/** @} end group ZclPort */

/*********************************************************************
 *********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* ZCL_PORT_H */
