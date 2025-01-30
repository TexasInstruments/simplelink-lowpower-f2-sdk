/******************************************************************************

 @file  oad_service.c

 @brief This is the TI proprietary OAD Services.

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

#include "ti/bleapp/ble_app_util/inc/bleapputil_api.h"
#include "ti/bleapp/services/oad/oad_service.h"

/* This Header file contains all BLE API and icall structure definition */
#include "icall_ble_api.h"

/*********************************************************************
 * MACROS
 */
 /*!
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

// OAD Service UUID
CONST uint8 oadServUUID[ATT_UUID_SIZE] =
{
    TI_BASE_UUID_128(OAD_SERVICE_UUID)
};

CONST uint8 oadCharUUID[OAD_CHAR_CNT][ATT_UUID_SIZE] =
{
    // OAD Image Identify UUID
    TI_BASE_UUID_128(OAD_IMG_IDENTIFY_UUID),

    // OAD Image Block Request/Response UUID
    TI_BASE_UUID_128(OAD_IMG_BLOCK_UUID),

    // OAD Extended Control UUID
    TI_BASE_UUID_128(OAD_EXT_CTRL_UUID)
};

// OAD Service attribute
CONST gattAttrType_t oadService = { ATT_UUID_SIZE, oadServUUID };

// OAD Characteristic Properties
static uint8 oadCharProps = GATT_PROP_WRITE_NO_RSP | GATT_PROP_WRITE | GATT_PROP_NOTIFY;

static uint8 oadCharExtCtrlProps = GATT_PROP_WRITE_NO_RSP | GATT_PROP_NOTIFY;

// Place holders for the GATT Server App to be able to lookup handles.
static uint8 oadCharVals[OAD_CHAR_CNT] = {0, 0 , SUCCESS};

// OAD Client Characteristic Configs
static gattCharCfg_t *oadImgIdentifyConfig;
static gattCharCfg_t *oadImgBlockConfig;
static gattCharCfg_t *oadExtCtrlConfig;

static gattCharCfg_t *oadCCCDTable;

// OAD Characteristic user descriptions
static uint8 oadImgIdentifyDesc[] = "Img Identify";
static uint8 oadImgBlockDesc[] = "Img Block";
static uint8 oadExtCtrlDesc[] = "OAD Extended Control";

/*********************************************************************
 * Service Attributes - Table
 */

static gattAttribute_t oadAttrTbl[] =
{
/*--define--*//*--------------type-------------*/ /*-----------permissions-----------*/ /*----------pValue----------*/
  // OAD Service
  GATT_BT_ATT( primaryServiceUUID,                  GATT_PERMIT_READ,                     (uint8 *) &oadService          ),

  // OAD Image Identify Characteristic Declaration
  GATT_BT_ATT( characterUUID,                       GATT_PERMIT_READ,                     &oadCharProps                  ),
  // OAD Image Identify Characteristic Value
  GATT_ATT   ( oadCharUUID[OAD_SRV_IDENTIFY_REQ],   OAD_WRITE_PERMIT,                     oadCharVals                    ),
  // Characteristic configuration
  GATT_BT_ATT( clientCharCfgUUID,                   GATT_PERMIT_READ | OAD_WRITE_PERMIT,  (uint8 *)&oadImgIdentifyConfig ),
  // OAD Image Identify User Description
  GATT_BT_ATT( charUserDescUUID,                    GATT_PERMIT_READ,                     (uint8 *)oadImgIdentifyDesc    ),

  // OAD Image Block Request/Response Characteristic Declaration
  GATT_BT_ATT( characterUUID,                       GATT_PERMIT_READ,                     &oadCharProps                  ),
  // OAD Image Block Request/Response Characteristic Value
  GATT_ATT   ( oadCharUUID[OAD_SRV_BLOCK_REQ],      OAD_WRITE_PERMIT,                     oadCharVals + OAD_SRV_BLOCK_REQ),
  // Characteristic configuration
  GATT_BT_ATT( clientCharCfgUUID,                   GATT_PERMIT_READ | OAD_WRITE_PERMIT,  (uint8 *)&oadImgBlockConfig    ),
  // OAD Image Block Request/Response User Description
  GATT_BT_ATT( charUserDescUUID,                    GATT_PERMIT_READ,                     (uint8 *)oadImgBlockDesc       ),

  // OAD Extended Control Characteristic Declaration
  GATT_BT_ATT( characterUUID,                       GATT_PERMIT_READ,                     &oadCharExtCtrlProps           ),
  // OAD Extended Control Characteristic Value
  GATT_ATT   ( oadCharUUID[OAD_SRV_CTRL_CMD],       OAD_WRITE_PERMIT,                     oadCharVals + OAD_SRV_CTRL_CMD ),
  // Characteristic configuration
  GATT_BT_ATT( clientCharCfgUUID,                   GATT_PERMIT_READ | OAD_WRITE_PERMIT,  (uint8 *)&oadExtCtrlConfig     ),
  // OAD Extended Control User Description
  GATT_BT_ATT( charUserDescUUID,                    GATT_PERMIT_READ,                     (uint8 *)oadExtCtrlDesc        ),
};

/*********************************************************************
 * LOCAL VARIABLES
 */

static oadServiceCB_t oadWriteCB = NULL;

// OAD is only allowed via a single connection, store it here
static uint16 activeOadCxnHandle = LINKDB_CONNHANDLE_INVALID;

/*********************************************************************
 * LOCAL FUNCTIONS - Prototypes
 */
static bStatus_t OadReadAttrCB(uint16 connHandle, gattAttribute_t *pAttr,
                               uint8 *pValue, uint16 *pLen, uint16 offset,
                               uint16 maxLen, uint8 method);

static bStatus_t OadWriteAttrCB(uint16 connHandle, gattAttribute_t *pAttr,
                                uint8 *pValue, uint16 len, uint16 offset,
                                uint8 method);

/*********************************************************************
 * PROFILE CALLBACKS
 */

// OAD Service Callbacks
// Note: When an operation on a characteristic requires authorization and
// pfnAuthorizeAttrCB is not defined for that characteristic's service, the
// Stack will report a status of ATT_ERR_UNLIKELY to the client.  When an
// operation on a characteristic requires authorization the Stack will call
// pfnAuthorizeAttrCB to check a client's authorization prior to calling
// pfnReadAttrCB or pfnWriteAttrCB, so no checks for authorization need to be
// made within these functions.
CONST gattServiceCBs_t oadServiceCBs =
{
    OadReadAttrCB,  // Read callback function pointer.
    OadWriteAttrCB, // Write callback function pointer.
    NULL            // Authorization callback function pointer.
};

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      OAD_AddService
 *
 * @brief   Initializes the OAD service by registering
 *          GATT attributes with the GATT server.
 *
 * @param   pfnOadServiceCB - Callback function from service
 *                            to profile after context switch.
 *
 * @return  SUCCESS, INVALIDPARAMETER or bleMemAllocError
 */
bStatus_t OAD_AddService(oadServiceCB_t pfnOadServiceCB)
{
    bStatus_t status = SUCCESS;

    if(pfnOadServiceCB != NULL)
    {
        /*
        * Allocate all memory for OAD CCCDs in a single malloc. This is
        * more efficient in terms of heap. The individual pointers will
        * be set as offsets into the table. Each CCCD entry is of size
        * sizeof(gattCharCfg_t)*linkDBNumConns)
        */
        uint8 oadCCCDTableSize = (sizeof(gattCharCfg_t) * linkDBNumConns) \
                                                        * OAD_CHAR_CNT;
        oadCCCDTable = (gattCharCfg_t *)ICall_malloc(oadCCCDTableSize);

        // Ensure there is heap memory available for the CCCD table
        if (oadCCCDTable == NULL)
        {
            return (bleMemAllocError);
        }

        // ImgID has no offset into the table
        oadImgIdentifyConfig = oadCCCDTable;
        // ImgBlock is the second entry
        oadImgBlockConfig = oadCCCDTable + linkDBNumConns;
        // ExtCtrl is the third entry
        oadExtCtrlConfig = oadCCCDTable + 2*linkDBNumConns;

        // Initialize Client Characteristic Configuration attributes.
        for(uint8 conn_index = 0; conn_index<linkDBNumConns;conn_index++)
        {
            GATTServApp_InitCharCfg(LINKDB_CONNHANDLE_INVALID, oadImgIdentifyConfig + conn_index);
            GATTServApp_InitCharCfg(LINKDB_CONNHANDLE_INVALID, oadImgBlockConfig + conn_index);
            GATTServApp_InitCharCfg(LINKDB_CONNHANDLE_INVALID, oadExtCtrlConfig + conn_index);
        }

        // Register a write callback function.
        oadWriteCB = pfnOadServiceCB;

        status = GATTServApp_RegisterService(oadAttrTbl,
                                            GATT_NUM_ATTRS(oadAttrTbl),
                                            GATT_MAX_ENCRYPT_KEY_SIZE,
                                            &oadServiceCBs);
    }
    else
    {
        status =  INVALIDPARAMETER;
    }

    return (status);
}

/*********************************************************************
 * @fn      OADService_setParameter
 *
 * @brief   Set a OAD Characteristic parameter.
 *
 * @param   param - Service parameter ID
 * @param   len - length of data to write
 * @param   value - pointer to data to write.  This is dependent on
 *          the parameter ID and WILL be cast to the appropriate
 *          data type (example: data type of uint16 will be cast to
 *          uint16 pointer).
 *
 * @return  bStatus_t
 */
bStatus_t OADService_setParameter(oadServiceChar_e srvChar, uint8 len, void *value)
{
    bStatus_t status = SUCCESS;
    uint16 notify;

    if(srvChar == OAD_SRV_IDENTIFY_REQ)
    {
        notify = GATTServApp_ReadCharCfg(activeOadCxnHandle, oadImgIdentifyConfig);
    }
    else if(srvChar == OAD_SRV_CTRL_CMD)
    {
        notify = GATTServApp_ReadCharCfg(activeOadCxnHandle, oadExtCtrlConfig);
    }
    else
    {
        return (ATT_ERR_INVALID_VALUE);
    }

    // If notifications enabled
    if (notify == GATT_CLIENT_CFG_NOTIFY)
    {
        attHandleValueNoti_t notification;

        notification.pValue = GATT_bm_alloc(activeOadCxnHandle, ATT_HANDLE_VALUE_NOTI,
                                            len, NULL);
        if (NULL == notification.pValue)
        {
            return (ATT_ERR_INSUFFICIENT_RESOURCES);
        }
        else
        {
            gattAttribute_t *pAttr;

            pAttr = GATTServApp_FindAttr(oadAttrTbl, GATT_NUM_ATTRS(oadAttrTbl),
                                         oadCharVals+srvChar);

            if(pAttr == NULL)
            {
                // If we cannot find the attribute, report an error
                return (ATT_ERR_ATTR_NOT_FOUND);
            }

            notification.handle = pAttr->handle;
            notification.len = len;

            memcpy(notification.pValue, value, len);

            status = GATT_Notification(activeOadCxnHandle, &notification, FALSE);

            if (status != SUCCESS)
            {
                // The stack will free the memory for us if the
                // the notification is successful, otherwise we have
                // to free the memory manually
                GATT_bm_free((gattMsg_t *)&notification, ATT_HANDLE_VALUE_NOTI);
            }
        }
    }
    else
    {
        // Notifications are required for OAD, if they are not enabled
        // the process cannot continue
        status = bleIncorrectMode;
    }

    return (status);
}

/*********************************************************************
 * @fn      OADService_setActiveConnHandle
 *
 * @brief   Set an active connection handle
 *
 * @param   connHandle - Active connection handle
 *
 */
void OADService_setActiveConnHandle(uint16 connHandle)
{
    activeOadCxnHandle = connHandle;
}

/*********************************************************************
 * Callback Functions - These run in the BLE-Stack context!
 *********************************************************************/

/*********************************************************************
 * @fn      OadReadAttrCB
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
static bStatus_t OadReadAttrCB(uint16 connHandle, gattAttribute_t *pAttr,
                               uint8 *pValue, uint16 *pLen, uint16 offset,
                               uint16 maxLen, uint8 method)
{
    bStatus_t status;

    // Make sure it's not a blob operation (no attributes in the profile are long)
    if (offset > 0)
    {
        return (ATT_ERR_ATTR_NOT_LONG);
    }
    else
    {
        *pLen = 0;
        status = ATT_ERR_INVALID_HANDLE;
    }

    return (status);
}

/*********************************************************************
 * @fn      OadWriteAttrCB
 *
 * @brief   Validate and Write attribute data
 *
 * @param   connHandle - connection message was received on
 * @param   pAttr      - pointer to attribute
 * @param   pValue     - pointer to data to be written
 * @param   len        - length of data
 * @param   offset     - offset of the first octet to be written
 * @param   method     - type of write message
 *
 * @return  SUCCESS, blePending or Failure
 */
static bStatus_t OadWriteAttrCB(uint16 connHandle, gattAttribute_t *pAttr,
                                uint8 *pValue, uint16 len, uint16 offset,
                                uint8 method)
{
    bStatus_t status = SUCCESS;

    if (pAttr->type.len == ATT_BT_UUID_SIZE)
    {
        // 16-bit UUID
        uint16 uuid = BUILD_UINT16(pAttr->type.uuid[0], pAttr->type.uuid[1]);
        if (uuid == GATT_CLIENT_CHAR_CFG_UUID)
        {
            // Process a CCC write request.
            status = GATTServApp_ProcessCCCWriteReq(connHandle, pAttr, pValue,
                                                    len, offset,
                                                    GATT_CLIENT_CFG_NOTIFY);
        }
        else
        {
            status = ATT_ERR_ATTR_NOT_FOUND; // Should never get here!
        }
    }
    else
    // 128-bit UUID
    {
        uint8 char_index;
        oadServiceChar_e serviceChar;

        for (char_index = 0; char_index<OAD_CHAR_CNT + 1; char_index++)
        {
            if (!memcmp(pAttr->type.uuid, oadCharUUID[char_index],ATT_UUID_SIZE))
            {
                break; //found match
            }
        }
        switch (char_index)
        {

            case OAD_SRV_IDENTIFY_REQ:
            case OAD_SRV_BLOCK_REQ:
            case OAD_SRV_CTRL_CMD:
                serviceChar = (oadServiceChar_e)char_index;
                break;

            default:
                return (ATT_ERR_ATTR_NOT_FOUND); // Should never get here!;
        }

        if(oadWriteCB != NULL)
        {
            oadSrvWriteReq_t* pOADSrvWriteReq= ICall_malloc(sizeof(oadSrvWriteReq_t));

            if(pOADSrvWriteReq == NULL)
            {
                return (ATT_ERR_INSUFFICIENT_RESOURCES);
            }

            pOADSrvWriteReq->pData = ICall_malloc(sizeof(uint8) * len);
            if(pOADSrvWriteReq->pData == NULL)
            {
                return (ATT_ERR_INSUFFICIENT_RESOURCES);
            }
            //Build callback msg
            pOADSrvWriteReq->serviceChar = serviceChar;
            pOADSrvWriteReq->connHandle = connHandle;
            pOADSrvWriteReq->len = len;
            memcpy(pOADSrvWriteReq->pData, pValue, len);

            status = BLEAppUtil_invokeFunction(oadWriteCB, (char*)pOADSrvWriteReq);
        }

    }

    return (status);
}

/*********************************************************************
*********************************************************************/
