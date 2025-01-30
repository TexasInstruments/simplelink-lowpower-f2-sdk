/******************************************************************************

 @file  oad_reset_service.c

 @brief This is the TI proprietary OAD Reset Service.

 Group: WCS, BTS
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2022-2025, Texas Instruments Incorporated
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

 ******************************************************************************
 
 
 *****************************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include <string.h>

#include "comdef.h"
#include "onboard.h"
#include "ti/bleapp/ble_app_util/inc/bleapputil_api.h"
#include "ti/bleapp/services/oad/oad_reset_service.h"

/*********************************************************************
 * MACROS
 */
/*
 * This define controls whether the OAD profile requires BLE security
 * By default, this is off.
 */

#ifdef OAD_BLE_SECURITY
    #define OAD_WRITE_PERMIT     GATT_PERMIT_AUTHEN_WRITE
#else
    #define OAD_WRITE_PERMIT     GATT_PERMIT_WRITE
#endif //OAD_SECURITY
/*********************************************************************
 * Service Attributes - Variables
 */

// OAD Reset Service UUID
CONST uint8 oadResetServUUID[ATT_UUID_SIZE] =
{
    TI_BASE_UUID_128(OAD_RESET_SERVICE_UUID)
};

CONST uint8 oadResetCharUUID[ATT_UUID_SIZE] =
{
    TI_BASE_UUID_128(OAD_RESET_CHAR_UUID)
};

// OAD Reset Service attribute
CONST gattAttrType_t oadResetService = { ATT_UUID_SIZE, oadResetServUUID };

// OAD Reset Service Characteristic Properties
static uint8 oadResetCharProps = GATT_PROP_WRITE_NO_RSP | GATT_PROP_WRITE;

// OAD Reset Service Characteristic Value
static uint8 oadResetCharVal = 0;

// OAD Characteristic User Description
static uint8 oadResetCharUserDesc[] = "OAD Start/Reset";

/*********************************************************************
 * Service Attributes - Table
 */

static gattAttribute_t oadResetServiceAttrTbl[] =
{
/*--define--*//*--------------type-------------*/ /*-----------permissions-----------*/ /*----------pValue----------*/
  // OAD reset Service
  GATT_BT_ATT( primaryServiceUUID,                  GATT_PERMIT_READ,                     (uint8 *) &oadResetService     ),

  // OAD Reset Characteristic Declaration
  GATT_BT_ATT( characterUUID,                       GATT_PERMIT_READ,                     &oadResetCharProps             ),
  // OAD reset Characteristic Value
  GATT_ATT   ( oadResetCharUUID,                    OAD_WRITE_PERMIT,                     &oadResetCharVal               ),
  // OAD reset User Description
  GATT_BT_ATT( charUserDescUUID,                    GATT_PERMIT_READ,                     oadResetCharUserDesc           ),
};

/*********************************************************************
 * LOCAL VARIABLES
 */

static oadServiceCB_t oadResetWriteCB = NULL;


/*********************************************************************
 * LOCAL FUNCTIONS - Prototypes
 */
static bStatus_t OadResetReadAttrCB(uint16 connHandle, gattAttribute_t *pAttr,
                                    uint8 *pValue, uint16 *pLen, uint16 offset,
                                    uint16 maxLen, uint8 method);

static bStatus_t OadResetWriteAttrCB(uint16 connHandle, gattAttribute_t *pAttr,
                                     uint8 *pValue, uint16 len, uint16 offset,
                                     uint8 method);

/*********************************************************************
 * PROFILE CALLBACKS
 */

// OAD Reset Service Callbacks
// Note: When an operation on a characteristic requires authorization and
// pfnAuthorizeAttrCB is not defined for that characteristic's service, the
// Stack will report a status of ATT_ERR_UNLIKELY to the client.  When an
// operation on a characteristic requires authorization the Stack will call
// pfnAuthorizeAttrCB to check a client's authorization prior to calling
// pfnReadAttrCB or pfnWriteAttrCB, so no checks for authorization need to be
// made within these functions.
CONST gattServiceCBs_t oadResetServiceCBs =
{
    OadResetReadAttrCB,  // Read callback function pointer
    OadResetWriteAttrCB, // Write callback function pointer
    NULL                 // Authorization callback function pointer
};

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      OadReset_AddService
 *
 * @brief   Initializes the OAD reset service by registering
 *          GATT attributes with the GATT server.
 *
 * @param   pfnOadServiceCB - Callback function from service
 *                            to profile after context switch.
 *
 * @return  SUCCESS or INVALIDPARAMETER
 */
bStatus_t OadReset_AddService(oadServiceCB_t pfnOadServiceCB)
{
    bStatus_t status = SUCCESS;

    if(pfnOadServiceCB != NULL)
    {
        // Register a write callback function.
        oadResetWriteCB = pfnOadServiceCB;

        // Register GATT attribute list and CBs with GATT Server App
        status = GATTServApp_RegisterService(oadResetServiceAttrTbl,
                                             GATT_NUM_ATTRS(oadResetServiceAttrTbl),
                                             GATT_MAX_ENCRYPT_KEY_SIZE,
                                             &oadResetServiceCBs);
    }
    else
    {
        status =  INVALIDPARAMETER;
    }

    return (status);
}

/*********************************************************************
 * Callback Functions - These run in the BLE-Stack context!
 *********************************************************************/

/*********************************************************************
 * @fn      OadResetReadAttrCB
 *
 * @brief   Read an attribute.
 *
 * @param   connHandle - connection message was received on
 * @param   pAttr      - pointer to attribute
 * @param   pValue     - pointer to data to be read
 * @param   pLen       - length of data to be read
 * @param   offset     - offset of the first octet to be read
 * @param   maxLen     - maximum length of data to be read
 * @param   method     - type of read message
 *
* @return      ATT_ERR_INVALID_HANDLE, reads are not allowed.
 */
static bStatus_t OadResetReadAttrCB(uint16 connHandle, gattAttribute_t *pAttr,
                                    uint8 *pValue, uint16 *pLen, uint16 offset,
                                    uint16 maxLen, uint8 method)
{
    return (ATT_ERR_INVALID_HANDLE);
}

/*********************************************************************
 * @fn      OadResetWriteAttrCB
 *
 * @brief   Validate attribute data prior to a write operation
 *
 * @param   connHandle - connection message was received on
 * @param   pAttr - pointer to attribute
 * @param   pValue - pointer to data to be written
 * @param   len - length of data
 * @param   offset - offset of the first octet to be written
 * @param   method - type of write message
 *
 * @return  SUCCESS, blePending or Failure
 */
static bStatus_t OadResetWriteAttrCB(uint16 connHandle, gattAttribute_t *pAttr,
                                uint8 *pValue, uint16 len, uint16 offset,
                                uint8 method)
{
    bStatus_t status = SUCCESS;

    if ( pAttr->type.len == ATT_UUID_SIZE )
    {
        // 128-bit UUID
        if (!memcmp(pAttr->type.uuid, oadResetCharUUID, ATT_UUID_SIZE))
        {
            // Notify Profile
            if (oadResetWriteCB != NULL)
            {
                oadResetWrite_t* pOadResetWriteEvt = ICall_malloc(sizeof(oadResetWrite_t));

                if(pOadResetWriteEvt == NULL)
                {
                    return (ATT_ERR_INSUFFICIENT_RESOURCES);
                }
                else
                {
                    pOadResetWriteEvt->connHandle = connHandle;
                    pOadResetWriteEvt->cmd = pValue[0];
                    status = BLEAppUtil_invokeFunction(oadResetWriteCB, (char*)pOadResetWriteEvt);
                }
            }
        }
        else
        {
            status = ATT_ERR_INVALID_HANDLE;
        }
    }
    return (status);
}

/*********************************************************************
*********************************************************************/
