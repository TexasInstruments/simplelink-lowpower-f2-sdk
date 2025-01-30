/******************************************************************************

 @file  dev_info_service.c

 @brief This file contains the Device Information service.

 Group: WCS, BTS
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2012-2025, Texas Instruments Incorporated
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

#include <icall.h>
#include "util.h"
/* This Header file contains all BLE API and icall structure definition */
#include "icall_ble_api.h"

#include <ti/bleapp/services/dev_info/dev_info_service.h>
/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */
// Device information service
GATT_BT_UUID(devInfoServUUID, DEVINFO_SERV_UUID);

// System ID
GATT_BT_UUID(devInfoSystemIdUUID, SYSTEM_ID_UUID);

// Model Number String
GATT_BT_UUID(devInfoModelNumberUUID, MODEL_NUMBER_UUID);

// Serial Number String
GATT_BT_UUID(devInfoSerialNumberUUID, SERIAL_NUMBER_UUID);

// Firmware Revision String
GATT_BT_UUID(devInfoFirmwareRevUUID, FIRMWARE_REV_UUID);

// Hardware Revision String
GATT_BT_UUID(devInfoHardwareRevUUID, HARDWARE_REV_UUID);

// Software Revision String
GATT_BT_UUID(devInfoSoftwareRevUUID, SOFTWARE_REV_UUID);

// Manufacturer Name String
GATT_BT_UUID(devInfoMfrNameUUID, MANUFACTURER_NAME_UUID);

// IEEE 11073-20601 Regulatory Certification Data List
GATT_BT_UUID(devInfo11073CertUUID, IEEE_11073_CERT_DATA_UUID);

// PnP ID
GATT_BT_UUID(devInfoPnpIdUUID, PNP_ID_UUID);

/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */

/*********************************************************************
 * Profile Attributes - variables
 */

// Device Information Service attribute
static CONST gattAttrType_t devInfoService = GATT_ATT_BT_UUID_TYPE(devInfoServUUID);

// System ID characteristic
static uint8 devInfoSystemIdProps = GATT_PROP_READ;
static uint8 devInfoSystemId[DEVINFO_SYSTEM_ID_LEN] = {0, 0, 0, 0, 0, 0, 0, 0};

// Model Number String characteristic
static uint8 devInfoModelNumberProps = GATT_PROP_READ;
static uint8 devInfoModelNumber[DEVINFO_STR_ATTR_LEN+1] = "Model Number";

// Serial Number String characteristic
static uint8 devInfoSerialNumberProps = GATT_PROP_READ;
static uint8 devInfoSerialNumber[DEVINFO_STR_ATTR_LEN+1] = "Serial Number";

// Firmware Revision String characteristic
static uint8 devInfoFirmwareRevProps = GATT_PROP_READ;
static uint8 devInfoFirmwareRev[DEVINFO_STR_ATTR_LEN+1] = "Firmware Revision";

// Hardware Revision String characteristic
static uint8 devInfoHardwareRevProps = GATT_PROP_READ;
static uint8 devInfoHardwareRev[DEVINFO_STR_ATTR_LEN+1] = "Hardware Revision";

// Software Revision String characteristic
static uint8 devInfoSoftwareRevProps = GATT_PROP_READ;
static uint8 devInfoSoftwareRev[DEVINFO_STR_ATTR_LEN+1] = "Software Revision";

// Manufacturer Name String characteristic
static uint8 devInfoMfrNameProps = GATT_PROP_READ;
static uint8 devInfoMfrName[DEVINFO_STR_ATTR_LEN+1] = "Manufacturer Name";

// IEEE 11073-20601 Regulatory Certification Data List characteristic
static uint8 devInfo11073CertProps = GATT_PROP_READ;
static uint8 defaultDevInfo11073Cert[] =
{
  DEVINFO_11073_BODY_EXP,     // authoritative body type
  0x00,                       // authoritative body structure type
                              // authoritative body data follows below:
  'e', 'x', 'p', 'e', 'r', 'i', 'm', 'e', 'n', 't', 'a', 'l'
};

// The length of this characteristic is not fixed
static uint8 *devInfo11073Cert = defaultDevInfo11073Cert;
static uint8 devInfo11073CertLen = sizeof(defaultDevInfo11073Cert);

// PnP ID characteristic
static uint8 devInfoPnpIdProps = GATT_PROP_READ;
static uint8 devInfoPnpId[DEVINFO_PNP_ID_LEN] =
{
  1,                                      // Vendor ID source (1=Bluetooth SIG)
  LO_UINT16(0x000D), HI_UINT16(0x000D),   // Vendor ID (Texas Instruments)
  LO_UINT16(0x0000), HI_UINT16(0x0000),   // Product ID (vendor-specific)
  LO_UINT16(0x0110), HI_UINT16(0x0110)    // Product version (JJ.M.N)
};

/*********************************************************************
 * Profile Attributes - Table
 */

static gattAttribute_t devInfoAttrTbl[] =
{
  /*---------------type----------------*/  /*--permissions--*/  /*-----------pValue-----------*/

  // Device Information Service
  GATT_BT_ATT( primaryServiceUUID,          GATT_PERMIT_READ,       (uint8 *)&devInfoService ),

  // System ID Declaration
  GATT_BT_ATT( characterUUID,               GATT_PERMIT_READ,       &devInfoSystemIdProps ),
  // System ID Value
  GATT_BT_ATT( devInfoSystemIdUUID,         GATT_PERMIT_READ,       (uint8 *) devInfoSystemId ),

  // Model Number String Declaration
  GATT_BT_ATT( characterUUID,               GATT_PERMIT_READ,       &devInfoModelNumberProps ),
  // Model Number Value
  GATT_BT_ATT( devInfoModelNumberUUID,      GATT_PERMIT_READ,       (uint8 *) devInfoModelNumber ),

  // Serial Number String Declaration
  GATT_BT_ATT( characterUUID,               GATT_PERMIT_READ,       &devInfoSerialNumberProps ),
  // Serial Number Value
  GATT_BT_ATT( devInfoSerialNumberUUID,     GATT_PERMIT_READ,       (uint8 *) devInfoSerialNumber ),

  // Firmware Revision String Declaration
  GATT_BT_ATT( characterUUID,               GATT_PERMIT_READ,       &devInfoFirmwareRevProps ),
  // Firmware Revision Value
  GATT_BT_ATT( devInfoFirmwareRevUUID,      GATT_PERMIT_READ,       (uint8 *) devInfoFirmwareRev ),

  // Hardware Revision String Declaration
  GATT_BT_ATT( characterUUID,               GATT_PERMIT_READ,       &devInfoHardwareRevProps ),
  // Hardware Revision Value
  GATT_BT_ATT( devInfoHardwareRevUUID,      GATT_PERMIT_READ,       (uint8 *) devInfoHardwareRev ),

  // Software Revision String Declaration
  GATT_BT_ATT( characterUUID,               GATT_PERMIT_READ,       &devInfoSoftwareRevProps ),
  // Software Revision Value
  GATT_BT_ATT( devInfoSoftwareRevUUID,      GATT_PERMIT_READ,       (uint8 *) devInfoSoftwareRev ),

  // Manufacturer Name String Declaration
  GATT_BT_ATT( characterUUID,               GATT_PERMIT_READ,       &devInfoMfrNameProps ),
  // Manufacturer Name Value
  GATT_BT_ATT( devInfoMfrNameUUID,          GATT_PERMIT_READ,       (uint8 *) devInfoMfrName ),

  // IEEE 11073-20601 Regulatory Certification Data List Declaration
  GATT_BT_ATT( characterUUID,               GATT_PERMIT_READ,       &devInfo11073CertProps ),
  // IEEE 11073-20601 Regulatory Certification Data List Value
  GATT_BT_ATT( devInfo11073CertUUID,        GATT_PERMIT_READ,       defaultDevInfo11073Cert ),

  // PnP ID Declaration
  GATT_BT_ATT( characterUUID,               GATT_PERMIT_READ,       &devInfoPnpIdProps ),
  // PnP ID Value
  GATT_BT_ATT( devInfoPnpIdUUID,            GATT_PERMIT_READ,       (uint8 *) devInfoPnpId )
};


/*********************************************************************
 * LOCAL FUNCTIONS
 */
static bStatus_t DevInfo_readAttrCB( uint16 connHandle, gattAttribute_t *pAttr,
                                     uint8 *pValue, uint16 *pLen, uint16 offset,
                                     uint16 maxLen, uint8 method );

/*********************************************************************
 * PROFILE CALLBACKS
 */
// Device Info Service Callbacks
// Note: When an operation on a characteristic requires authorization and
// pfnAuthorizeAttrCB is not defined for that characteristic's service, the
// Stack will report a status of ATT_ERR_UNLIKELY to the client.  When an
// operation on a characteristic requires authorization the Stack will call
// pfnAuthorizeAttrCB to check a client's authorization prior to calling
// pfnReadAttrCB or pfnWriteAttrCB, so no checks for authorization need to be
// made within these functions.
CONST gattServiceCBs_t devInfoCBs =
{
  DevInfo_readAttrCB, // Read callback function pointer
  NULL,               // Write callback function pointer
  NULL                // Authorization callback function pointer
};

/*********************************************************************
 * NETWORK LAYER CALLBACKS
 */

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      DevInfo_AddService
 *
 * @brief   Initializes the Device Information service by registering
 *          GATT attributes with the GATT server.
 *
 * @return  Success or Failure
 */
bStatus_t DevInfo_addService( void )
{
  // Register GATT attribute list and CBs with GATT Server App
  return GATTServApp_RegisterService( devInfoAttrTbl,
                                      GATT_NUM_ATTRS( devInfoAttrTbl ),
                                      GATT_MAX_ENCRYPT_KEY_SIZE,
                                      &devInfoCBs );
}

/*********************************************************************
 * @fn      DevInfo_setParameter
 *
 * @brief   Set a Device Information parameter.
 *
 * @param   param - Profile parameter ID
 * @param   len - length of data to write
 * @param   value - pointer to data to write.  This is dependent on
 *          the parameter ID and WILL be cast to the appropriate
 *          data type (example: data type of uint16 will be cast to
 *          uint16 pointer).
 *
 * @return  bStatus_t
 */
bStatus_t DevInfo_setParameter( uint8 param, uint8 len, void *value )
{
  bStatus_t ret = SUCCESS;

  switch ( param )
  {
     case DEVINFO_SYSTEM_ID:
      // verify length
      if (len == sizeof(devInfoSystemId))
      {
        memcpy(devInfoSystemId, value, len);
      }
      else
      {
        ret = bleInvalidRange;
      }
      break;

    case DEVINFO_MODEL_NUMBER:
      // verify length, leave room for null-terminate char
      if (len <= DEVINFO_STR_ATTR_LEN)
      {
        memset(devInfoModelNumber, 0, DEVINFO_STR_ATTR_LEN+1);
        memcpy(devInfoModelNumber, value, len);
      }
      else
      {
        ret = bleInvalidRange;
      }
      break;
    case DEVINFO_SERIAL_NUMBER:
      // verify length, leave room for null-terminate char
      if (len <= DEVINFO_STR_ATTR_LEN)
      {
        memset(devInfoSerialNumber, 0, DEVINFO_STR_ATTR_LEN+1);
        memcpy(devInfoSerialNumber, value, len);
      }
      else
      {
        ret = bleInvalidRange;
      }
      break;

    case DEVINFO_FIRMWARE_REV:
      // verify length, leave room for null-terminate char
      if (len <= DEVINFO_STR_ATTR_LEN)
      {
        memset(devInfoFirmwareRev, 0, DEVINFO_STR_ATTR_LEN+1);
        memcpy(devInfoFirmwareRev, value, len);
      }
      else
      {
        ret = bleInvalidRange;
      }
      break;

    case DEVINFO_HARDWARE_REV:
      // verify length, leave room for null-terminate char
      if (len <= DEVINFO_STR_ATTR_LEN)
      {
        memset(devInfoHardwareRev, 0, DEVINFO_STR_ATTR_LEN+1);
        memcpy(devInfoHardwareRev, value, len);
      }
      else
      {
        ret = bleInvalidRange;
      }
      break;

    case DEVINFO_SOFTWARE_REV:
      // verify length, leave room for null-terminate char
      if (len <= DEVINFO_STR_ATTR_LEN)
      {
        memset(devInfoSoftwareRev, 0, DEVINFO_STR_ATTR_LEN+1);
        memcpy(devInfoSoftwareRev, value, len);
      }
      else
      {
        ret = bleInvalidRange;
      }
      break;

    case DEVINFO_MANUFACTURER_NAME:
      // verify length, leave room for null-terminate char
      if (len <= DEVINFO_STR_ATTR_LEN)
      {
        memset(devInfoMfrName, 0, DEVINFO_STR_ATTR_LEN+1);
        memcpy(devInfoMfrName, value, len);
      }
      else
      {
        ret = bleInvalidRange;
      }
      break;

    case DEVINFO_11073_CERT_DATA:
      {
        // Allocate buffer for new certification
        uint8 *pCert = ICall_malloc(len);
        if (pCert != NULL)
        {
          if (devInfo11073Cert != defaultDevInfo11073Cert)
          {
            // Free existing certification buffer
            ICall_free(devInfo11073Cert);
          }

          // Copy over new certification
          memcpy(pCert, value, len);

          // Update our globals
          devInfo11073Cert = pCert;
          devInfo11073CertLen = len;
        }
        else
        {
          ret = bleMemAllocError;
        }
      }
      break;

    case DEVINFO_PNP_ID:
      // verify length
      if (len == sizeof(devInfoPnpId))
      {
        memcpy(devInfoPnpId, value, len);
      }
      else
      {
        ret = bleInvalidRange;
      }
      break;

    default:
      ret = INVALIDPARAMETER;
      break;
  }

  return ( ret );
}

/*********************************************************************
 * @fn      DevInfo_getParameter
 *
 * @brief   Get a Device Information parameter.
 *
 * @param   param - Profile parameter ID
 * @param   value - pointer to data to get.  This is dependent on
 *          the parameter ID and WILL be cast to the appropriate
 *          data type (example: data type of uint16 will be cast to
 *          uint16 pointer).
 *
 * @return  bStatus_t
 */
bStatus_t DevInfo_getParameter( uint8 param, void *value )
{
  bStatus_t ret = SUCCESS;

  switch ( param )
  {
    case DEVINFO_SYSTEM_ID:
      memcpy(value, devInfoSystemId, sizeof(devInfoSystemId));
      break;

    case DEVINFO_MODEL_NUMBER:
      memcpy(value, devInfoModelNumber, DEVINFO_STR_ATTR_LEN);
      break;
    case DEVINFO_SERIAL_NUMBER:
      memcpy(value, devInfoSerialNumber, DEVINFO_STR_ATTR_LEN);
      break;

    case DEVINFO_FIRMWARE_REV:
      memcpy(value, devInfoFirmwareRev, DEVINFO_STR_ATTR_LEN);
      break;

    case DEVINFO_HARDWARE_REV:
      memcpy(value, devInfoHardwareRev, DEVINFO_STR_ATTR_LEN);
      break;

    case DEVINFO_SOFTWARE_REV:
      memcpy(value, devInfoSoftwareRev, DEVINFO_STR_ATTR_LEN);
      break;

    case DEVINFO_MANUFACTURER_NAME:
      memcpy(value, devInfoMfrName, DEVINFO_STR_ATTR_LEN);
      break;

    case DEVINFO_11073_CERT_DATA:
      memcpy(value, devInfo11073Cert, devInfo11073CertLen);
      break;

    case DEVINFO_PNP_ID:
      memcpy(value, devInfoPnpId, sizeof(devInfoPnpId));
      break;

    default:
      ret = INVALIDPARAMETER;
      break;
  }

  return ( ret );
}

/*********************************************************************
 * @fn          DevInfo_ReadAttrCB
 *
 * @brief       Read an attribute.
 *
 * @param       connHandle - connection message was received on
 * @param       pAttr - pointer to attribute
 * @param       pValue - pointer to data to be read
 * @param       pLen - length of data to be read
 * @param       offset - offset of the first octet to be read
 * @param       maxLen - maximum length of data to be read
 * @param       method - type of read message
 *
 * @return      SUCCESS, blePending or Failure
 */
static bStatus_t DevInfo_readAttrCB( uint16 connHandle, gattAttribute_t *pAttr,
                                     uint8 *pValue, uint16 *pLen, uint16 offset,
                                     uint16 maxLen, uint8 method )
{
  bStatus_t status = SUCCESS;
  uint16 uuid = BUILD_UINT16( pAttr->type.uuid[0], pAttr->type.uuid[1]);

  // If the value offset of the Read Blob Request is greater than the
  // length of the attribute value, an Error Response shall be sent with
  // the error code Invalid Offset.
  switch (uuid)
  {
    case SYSTEM_ID_UUID:
      // verify offset
      if (offset > sizeof(devInfoSystemId))
      {
        status = ATT_ERR_INVALID_OFFSET;
      }
      else
      {
        // determine read length
        *pLen = MIN(maxLen, (sizeof(devInfoSystemId) - offset));

        // copy data
        memcpy(pValue, &devInfoSystemId[offset], *pLen);
      }
      break;

    case MODEL_NUMBER_UUID:
    case SERIAL_NUMBER_UUID:
    case FIRMWARE_REV_UUID:
    case HARDWARE_REV_UUID:
    case SOFTWARE_REV_UUID:
    case MANUFACTURER_NAME_UUID:
      {
        uint16 len = strlen((char *)(pAttr->pValue));

        // verify offset
        if (offset > len)
        {
          status = ATT_ERR_INVALID_OFFSET;
        }
        else
        {
          // determine read length (exclude null terminating character)
          *pLen = MIN(maxLen, (len - offset));

          // copy data
          memcpy(pValue, &(pAttr->pValue[offset]), *pLen);
        }
      }
      break;

    case IEEE_11073_CERT_DATA_UUID:
      // verify offset
      if (offset > devInfo11073CertLen)
      {
        status = ATT_ERR_INVALID_OFFSET;
      }
      else
      {
        // determine read length
        *pLen = MIN(maxLen, (devInfo11073CertLen - offset));

        // copy data
        memcpy(pValue, &devInfo11073Cert[offset], *pLen);
      }
      break;

    case PNP_ID_UUID:
      // verify offset
      if (offset > sizeof(devInfoPnpId))
      {
        status = ATT_ERR_INVALID_OFFSET;
      }
      else
      {
        // determine read length
        *pLen = MIN(maxLen, (sizeof(devInfoPnpId) - offset));

        // copy data
        memcpy(pValue, &devInfoPnpId[offset], *pLen);
      }
      break;

    default:
      *pLen = 0;
      status = ATT_ERR_ATTR_NOT_FOUND;
      break;
  }

  return ( status );
}

/*********************************************************************
*********************************************************************/
