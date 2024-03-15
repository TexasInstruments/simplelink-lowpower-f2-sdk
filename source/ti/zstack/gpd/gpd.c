/**************************************************************************************************
  Filename:       gpd.c
  Revised:        $Date: 2014-10-06 15:42:43 -0700 (Mon, 06 Oct 2014) $
  Revision:       $Revision: 40449 $

  Description:    Green Power device functions.


  Copyright 2005-2014 Texas Instruments Incorporated.

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
**************************************************************************************************/

#ifdef __cplusplus
extern "C"
{
#endif

/******************************************************************************
 * INCLUDES
 */
#include "ti_drivers_config.h"
#include "gpd.h"
#include "api_mac.h"
#include "mac_api.h"
#include "nvintf.h"
#include "stdlib.h"
#include "string.h"
#include "rom_jt_154.h"
#include "gpd_memory.h"
#include "gpd_common.h"

/******************************************************************************
 * CONSTANTS
 */
#define NV_ITEM_UNINIT            0x09
#define NV_OPER_FAILED            0x0A
#define GP_NV_EX_LEGACY           0x0000

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */
static ApiMac_mcpsDataReq_t *pDuplicateFrame = NULL;

#if (GP_SECURITY_LEVEL == GP_SECURITY_LVL_4FC_4MIC) || (GP_SECURITY_LEVEL == GP_SECURITY_LVL_4FC_4MIC_ENCRYPT)
static uint32_t gpdSecurityFrameCounter = 0;
#endif

/*********************************************************************
 * GLOBAL VARIABLES
 */

/* handle returned from AESCCM_open() call */
AESCCM_Handle ZAESCCM_handle;

/*********************************************************************
* LOCAL FUNCTIONS
*/
static uint8_t GreenPowerSecureDataFrameSend( gpdfReq_t *pReq, uint8_t channel );
static uint8_t GreenPowerUnsecureDataFrameSend( gpdfReq_t *pReq, uint8_t channel );
static ApiMac_status_t gpdSendFrame(uint8_t *pGPDF, uint8_t frameLen, uint8_t channel);
static void *gp_BuildMsg(gpdfReq_t *pReq, uint8_t *pMsgLen);
static void *gp_BuildUnsecureMsg(gpdfReq_t *pReq, uint8_t *pMsgLen);
#if (GP_SECURITY_LEVEL == GP_SECURITY_LVL_4FC_4MIC) || (GP_SECURITY_LEVEL == GP_SECURITY_LVL_4FC_4MIC_ENCRYPT)
#if defined ( GPD_COMMISSIONING_CMD_SUPPORT )
static uint8_t gpd_encryptKey(gpdfReq_t *pReq, uint8_t *key, uint32_t frameCounter);
static void gp_BuildKeyNonce( gpd_ID_t *pGpdId, uint32_t securityCounter, uint8_t *nonce );
static uint8_t getCommisioningCmdLen(void);
#endif
static void gp_BuildNonce( gpd_ID_t *pGpdId, uint32_t securityCounter, uint8_t *nonce );
static void* gpd_ccmStar( uint8_t ed_flag, gpdfReq_t *pReq, uint8_t *msgLen);
#endif
static uint8_t  gpGetDataTypeLength( uint8_t dataType );
static uint16_t gpGetAttrDataLength( uint8_t dataType, uint8_t *pData );
static uint8_t *gpSerializeData( uint8_t dataType, void *attrData, uint8_t *buf );
static uint8_t *gp_buffer_uint32(uint8_t *buf, uint32_t val);

/******************************************************************************
 Public Functions
 *****************************************************************************/

#if (GP_SECURITY_LEVEL == GP_SECURITY_LVL_4FC_4MIC) || (GP_SECURITY_LEVEL == GP_SECURITY_LVL_4FC_4MIC_ENCRYPT)
/**************************************************
* @fn      gpdAESCCMInit
*
* @brief   Initialize AESCCM for green power device
*
* @return  status
*/
uint8_t gpdAESCCMInit(void)
{
    AESCCM_Params AESCCMParams;

    // Initialize AESCCM Driver
     AESCCM_init();
     AESCCM_Params_init(&AESCCMParams);
     AESCCMParams.returnBehavior = AESCCM_RETURN_BEHAVIOR_POLLING;
     ZAESCCM_handle = AESCCM_open(CONFIG_AESCCM_0, &AESCCMParams);
     if (!ZAESCCM_handle)
     {
      /* error */
       return FAILURE;
     }
     return SUCCESS;
}
#endif

/**************************************************
* @fn      GreenPowerDataFrameSend
*
* @brief   Send GPDF on requested channel
*
* @param   pReq    - pointer to structure containing data and where to send it.
* @param   channel - channel to send the frame
* @param   secure  - apply security
* @return  status
*/
uint8_t GreenPowerDataFrameSend( gpdfReq_t *pReq, uint8_t channel, bool secure)
{
  if(secure == TRUE)
  {
      return GreenPowerSecureDataFrameSend( pReq, channel );
  }
  else
  {
      return GreenPowerUnsecureDataFrameSend( pReq, channel );
  }
}

#if defined ( GPD_COMMISSIONING_CMD_SUPPORT )
/**************************************************
 * @fn      GreenPowerCommissioningSend
 *
 * @brief   Send a Commisioning Data Frame packet.
 *
 * @param   pCommissioningGpdf - pointer to structure containing commissioning data to send it
 * @param   channel            - channel to send the frame
 *
 * @return  status
 */
uint8_t GreenPowerCommissioningSend( gpdfReq_t *pCommissioningGpdf, uint8_t channel)
{
    gpdfReq_t commissioningGpdf;
    uint8_t *pPayload = NULL;
    uint8_t payloadSize;
    uint8_t stat = MAC_NO_RESOURCES;
    payloadSize =  getCommisioningCmdLen();
    commissioningGpdf.payload = OsalPort_malloc(payloadSize);
    if(commissioningGpdf.payload != NULL)
    {
        pPayload = commissioningGpdf.payload;
        OsalPort_memcpy(&commissioningGpdf, pCommissioningGpdf, (sizeof(gpdfReq_t) - sizeof(uint8_t*)));
        OsalPort_memcpy(pPayload, pCommissioningGpdf->payload, payloadSize);
        commissioningGpdf.payloadSize = payloadSize;
        pPayload += 3; // gpdDeviceID + options + extended options
#if (GP_SECURITY_LEVEL == GP_SECURITY_LVL_4FC_4MIC) || (GP_SECURITY_LEVEL == GP_SECURITY_LVL_4FC_4MIC_ENCRYPT)
        pCommissioningGpdf->securityFrameCounter = gpdSecurityFrameCounter;
        // set the MAC sequence number to the lower byte of the gpd security frame counter
        ApiMac_mlmeSetReqUint8(ApiMac_attribute_dsn, (uint8_t)(gpdSecurityFrameCounter & 0xFF) );
        // update the gpd security frame counter in NV
        // store the incremented value in NV, so it will be valid for the first frame
        // sent after restoring from NV
        gpdSecurityFrameCounter++;
        gp_nv_write(GP_NV_SEC_FRAME_COUNTER, sizeof(gpdSecurityFrameCounter), &gpdSecurityFrameCounter);
#endif

        // The Commissioning command payload
#if (EXOPT_GPD_KEY_PRESENT == TRUE) && (EXOPT_KEY_ENCRYPTION == TRUE) && ((EXOPT_SECURITY_LEVEL == GP_SECURITY_LVL_4FC_4MIC) || (EXOPT_SECURITY_LEVEL == GP_SECURITY_LVL_4FC_4MIC_ENCRYPT))
        stat = gpd_encryptKey(pCommissioningGpdf, pPayload, pCommissioningGpdf->securityFrameCounter);
        if(stat != MAC_SUCCESS)
        {
          return stat;
        }
        pPayload += GP_KEY_LENGTH;
        OsalPort_memcpy(pPayload, (uint8_t*)&(((gpdCommissioningCommand_t*)pCommissioningGpdf->payload)->GPDKeyMIC), GP_SECURITY_MIC_SIZE);
        pPayload += GP_SECURITY_MIC_SIZE;

#endif
#if (GP_SECURITY_LEVEL == GP_SECURITY_LVL_4FC_4MIC) || (GP_SECURITY_LEVEL == GP_SECURITY_LVL_4FC_4MIC_ENCRYPT)
        OsalPort_memcpy(pPayload, (uint8_t*)&pCommissioningGpdf->securityFrameCounter, sizeof(uint32_t));
        pPayload += 4; // outgoing counter
#endif
        *pPayload = ((gpdCommissioningCommand_t*)pCommissioningGpdf->payload)->applicationInformation;
        stat = GreenPowerDataFrameSend(&commissioningGpdf, channel, FALSE);
        OsalPort_free(commissioningGpdf.payload);
    }
    return ( stat );
}
#endif


/**************************************************
 * @fn      GreenPowerAttributeReportingSend
 *
 * @brief   Send a Attribute Reporting Data Frame packet.
 *
 * @param   clusterID      - Cluster Id of the attributes to be reported
 * @param   numAttr        - Number of attributes to be reported in the frame
 * @param   attrReportGpdf - pointer to structure containing Attribute Reporting data to send it
 * @param   channel        - channel to send the frame
 *
 * @return  status
 */
uint8_t GreenPowerAttributeReportingSend(uint16_t clusterID, uint8_t numAttr, gpdfReq_t *attrReportGpdf, uint8_t channel )
{
  gpdfReq_t   gpdfAttributeReporting;
  gpReport_t* pAttrReport;
  uint8_t       *pPayload = NULL;
  uint8_t       stat = MAC_NO_RESOURCES;
  uint8_t       i;

  attrReportGpdf->payloadSize = sizeof(uint16_t);
  //Consider Cluster ID, Attribute reported will be calculated below
  pAttrReport = (gpReport_t*)attrReportGpdf->payload;
  for(i = 0; i < numAttr; i++)
  {
    attrReportGpdf->payloadSize += gpGetAttrDataLength(pAttrReport->dataType, pAttrReport->attrData );
    //Consider the attribute ID and the data type field for each attribute
    attrReportGpdf->payloadSize += sizeof(uint16_t) + sizeof(uint8_t);
    pAttrReport++;
  }

  pPayload = OsalPort_malloc( attrReportGpdf->payloadSize);
  if(pPayload != NULL)
  {
    OsalPort_memcpy(&gpdfAttributeReporting, attrReportGpdf, sizeof(gpdfReq_t));
    gpdfAttributeReporting.payload = pPayload;

    OsalPort_memcpy(pPayload, &clusterID, sizeof(uint16_t));
    pPayload += sizeof(uint16_t);

    pAttrReport = (gpReport_t*)attrReportGpdf->payload;
    //Serialize the attributes
    for ( i = 0; i < numAttr; i++ )
    {
      *pPayload++ = LO_UINT16( pAttrReport->attrID );
      *pPayload++ = HI_UINT16( pAttrReport->attrID );
      *pPayload++ = pAttrReport->dataType;
      pPayload = gpSerializeData( pAttrReport->dataType, pAttrReport->attrData, pPayload );
      pAttrReport++;
    }

    OsalPort_memcpy(pPayload,attrReportGpdf->payload, attrReportGpdf->payloadSize - sizeof(uint16_t));
    stat = GreenPowerDataFrameSend(&gpdfAttributeReporting, channel, TRUE);
    OsalPort_free(gpdfAttributeReporting.payload);
  }
  return ( stat );
}

/******************************************************************************
 Local Functions
 *****************************************************************************/

/**************************************************
* @fn      GreenPowerDataFrameSend
*
* @brief   Send a MAC Data Frame packet, calls the passed in function to apply non-MAC security
*          on the MAC data field after the MAC buffer allocation.
*
* @param   pReq    - pointer to structure containing data and where to send it.
* @param   channel - channel to send the frame
*
* @return  status
*/
static uint8_t GreenPowerSecureDataFrameSend( gpdfReq_t *pReq, uint8_t channel )
{
  uint8_t *gpdFrame = NULL;
  uint8_t frameLen = 0;
  uint8_t stat = MAC_NO_RESOURCES;

#if (GP_SECURITY_LEVEL == GP_SECURITY_LVL_NO_SEC)
  gpdFrame = gp_BuildMsg( pReq, &frameLen );
#elif (GP_SECURITY_LEVEL == GP_SECURITY_LVL_4FC_4MIC) || (GP_SECURITY_LEVEL == GP_SECURITY_LVL_4FC_4MIC_ENCRYPT)
  pReq->securityFrameCounter = gpdSecurityFrameCounter;
  // set the MAC sequence number to the lower byte of the gpd security frame counter
  ApiMac_mlmeSetReqUint8(ApiMac_attribute_dsn, (uint8_t)(gpdSecurityFrameCounter & 0xFF) );
  // update the gpd security frame counter in NV
  // store the incremented value in NV, so it will be valid for the first frame
  // sent after restoring from NV
  gpdSecurityFrameCounter++;
  gp_nv_write(GP_NV_SEC_FRAME_COUNTER, sizeof(gpdSecurityFrameCounter), &gpdSecurityFrameCounter);
  gpdFrame = gpd_ccmStar(true, pReq, &frameLen);
#endif

  stat = gpdSendFrame(gpdFrame, frameLen, channel);
  return ( stat );
}

/**************************************************
* @fn      GreenPowerUnsecureDataFrameSend
*
* @brief   Send a MAC Unsecure Data Frame packet, calls the passed in function to apply non-MAC security
*          on the MAC data field after the MAC buffer allocation.
*
* @param   pReq    - pointer to structure containing data and where to send it.
* @param   channel - channel to send the frame
*
* @return  status
*/
static uint8_t GreenPowerUnsecureDataFrameSend( gpdfReq_t *pReq, uint8_t channel )
{
  uint8_t *gpdFrame = NULL;
  uint8_t frameLen = 0;
  uint8_t stat = MAC_NO_RESOURCES;

  gpdFrame = gp_BuildUnsecureMsg( pReq, &frameLen );
  stat = gpdSendFrame(gpdFrame, frameLen, channel);
  return ( stat );
}

/*********************************************************************
 * @fn      zclGetAttrDataLength
 *
 * @brief   Return the length of the attribute.
 *
 * @param   dataType - data type
 * @param   pData - pointer to data
 *
 * @return  returns atrribute length
 */
static uint16_t gpGetAttrDataLength( uint8_t dataType, uint8_t *pData )
{
  uint16_t dataLen = 0;

  if ( dataType == ZCL_DATATYPE_LONG_CHAR_STR || dataType == ZCL_DATATYPE_LONG_OCTET_STR )
  {
    dataLen = BUILD_UINT16( pData[0], pData[1] ) + 2; // long string length + 2 for length field
  }
  else if ( dataType == ZCL_DATATYPE_CHAR_STR || dataType == ZCL_DATATYPE_OCTET_STR )
  {
    dataLen = *pData + 1; // string length + 1 for length field
  }
  else
  {
    dataLen = gpGetDataTypeLength( dataType );
  }

  return ( dataLen );
}

/*********************************************************************
 * @fn      zclGetDataTypeLength
 *
 * @brief   Return the length of the datatype in octet.
 *
 *          NOTE: Should not be called for ZCL_DATATYPE_OCTECT_STR or
 *                ZCL_DATATYPE_CHAR_STR data types.
 *
 * @param   dataType - data type
 *
 * @return  length of data
 */
static uint8_t gpGetDataTypeLength( uint8_t dataType )
{
  uint8_t len;

  switch ( dataType )
  {
    case ZCL_DATATYPE_DATA8:
    case ZCL_DATATYPE_BOOLEAN:
    case ZCL_DATATYPE_BITMAP8:
    case ZCL_DATATYPE_INT8:
    case ZCL_DATATYPE_UINT8:
    case ZCL_DATATYPE_ENUM8:
      len = 1;
      break;

    case ZCL_DATATYPE_DATA16:
    case ZCL_DATATYPE_BITMAP16:
    case ZCL_DATATYPE_UINT16:
    case ZCL_DATATYPE_INT16:
    case ZCL_DATATYPE_ENUM16:
    case ZCL_DATATYPE_SEMI_PREC:
    case ZCL_DATATYPE_CLUSTER_ID:
    case ZCL_DATATYPE_ATTR_ID:
      len = 2;
      break;

    case ZCL_DATATYPE_DATA24:
    case ZCL_DATATYPE_BITMAP24:
    case ZCL_DATATYPE_UINT24:
    case ZCL_DATATYPE_INT24:
      len = 3;
      break;

    case ZCL_DATATYPE_DATA32:
    case ZCL_DATATYPE_BITMAP32:
    case ZCL_DATATYPE_UINT32:
    case ZCL_DATATYPE_INT32:
    case ZCL_DATATYPE_SINGLE_PREC:
    case ZCL_DATATYPE_TOD:
    case ZCL_DATATYPE_DATE:
    case ZCL_DATATYPE_UTC:
    case ZCL_DATATYPE_BAC_OID:
      len = 4;
      break;

   case ZCL_DATATYPE_UINT40:
   case ZCL_DATATYPE_INT40:
       len = 5;
       break;

   case ZCL_DATATYPE_UINT48:
   case ZCL_DATATYPE_INT48:
       len = 6;
       break;

   case ZCL_DATATYPE_UINT56:
   case ZCL_DATATYPE_INT56:
       len = 7;
       break;

   case ZCL_DATATYPE_DOUBLE_PREC:
   case ZCL_DATATYPE_IEEE_ADDR:
   case ZCL_DATATYPE_UINT64:
   case ZCL_DATATYPE_INT64:
     len = 8;
     break;

    case ZCL_DATATYPE_128_BIT_SEC_KEY:
     len = SEC_KEY_LEN;
     break;

    case ZCL_DATATYPE_NO_DATA:
    case ZCL_DATATYPE_UNKNOWN:
      // Fall through

    default:
      len = 0;
      break;
  }

  return ( len );
}

/*********************************************************************
 * @fn      gpSerializeData
 *
 * @brief   Builds a buffer from the attribute data to sent out over
 *          the air.
 *          NOTE - Not compatible with application's attributes callbacks.
 *
 * @param   dataType - data types defined in zcl.h
 * @param   attrData - pointer to the attribute data
 * @param   buf - where to put the serialized data
 *
 * @return  pointer to end of destination buffer
 */
static uint8_t *gpSerializeData( uint8_t dataType, void *attrData, uint8_t *buf )
{
  uint8_t *pStr;
  uint16_t len;

  if ( attrData == NULL )
  {
    return ( buf );
  }

  switch ( dataType )
  {
    case ZCL_DATATYPE_DATA8:
    case ZCL_DATATYPE_BOOLEAN:
    case ZCL_DATATYPE_BITMAP8:
    case ZCL_DATATYPE_INT8:
    case ZCL_DATATYPE_UINT8:
    case ZCL_DATATYPE_ENUM8:
      *buf++ = *((uint8_t *)attrData);
       break;

    case ZCL_DATATYPE_DATA16:
    case ZCL_DATATYPE_BITMAP16:
    case ZCL_DATATYPE_UINT16:
    case ZCL_DATATYPE_INT16:
    case ZCL_DATATYPE_ENUM16:
    case ZCL_DATATYPE_SEMI_PREC:
    case ZCL_DATATYPE_CLUSTER_ID:
    case ZCL_DATATYPE_ATTR_ID:
      *buf++ = LO_UINT16( *((uint16_t*)attrData) );
      *buf++ = HI_UINT16( *((uint16_t*)attrData) );
      break;

    case ZCL_DATATYPE_DATA24:
    case ZCL_DATATYPE_BITMAP24:
    case ZCL_DATATYPE_UINT24:
    case ZCL_DATATYPE_INT24:
      *buf++ = BREAK_UINT32( *((uint32_t*)attrData), 0 );
      *buf++ = BREAK_UINT32( *((uint32_t*)attrData), 1 );
      *buf++ = BREAK_UINT32( *((uint32_t*)attrData), 2 );
      break;

    case ZCL_DATATYPE_DATA32:
    case ZCL_DATATYPE_BITMAP32:
    case ZCL_DATATYPE_UINT32:
    case ZCL_DATATYPE_INT32:
    case ZCL_DATATYPE_SINGLE_PREC:
    case ZCL_DATATYPE_TOD:
    case ZCL_DATATYPE_DATE:
    case ZCL_DATATYPE_UTC:
    case ZCL_DATATYPE_BAC_OID:
      buf = gp_buffer_uint32( buf, *((uint32_t*)attrData) );
      break;

    case ZCL_DATATYPE_UINT40:
    case ZCL_DATATYPE_INT40:
      pStr = (uint8_t*)attrData;
      buf = gp_memcpy( buf, pStr, 5 );
      break;

    case ZCL_DATATYPE_UINT48:
    case ZCL_DATATYPE_INT48:
      pStr = (uint8_t*)attrData;
      buf = gp_memcpy( buf, pStr, 6 );
      break;

    case ZCL_DATATYPE_UINT56:
    case ZCL_DATATYPE_INT56:
      pStr = (uint8_t*)attrData;
      buf = gp_memcpy( buf, pStr, 7 );
      break;

    case ZCL_DATATYPE_DOUBLE_PREC:
    case ZCL_DATATYPE_IEEE_ADDR:
    case ZCL_DATATYPE_UINT64:
    case ZCL_DATATYPE_INT64:
      pStr = (uint8_t*)attrData;
      buf = gp_memcpy( buf, pStr, 8 );
      break;

    case ZCL_DATATYPE_CHAR_STR:
    case ZCL_DATATYPE_OCTET_STR:
      pStr = (uint8_t*)attrData;
      len = *pStr;
      buf = gp_memcpy( buf, pStr, len+1 ); // Including length field
      break;

    case ZCL_DATATYPE_LONG_CHAR_STR:
    case ZCL_DATATYPE_LONG_OCTET_STR:
      pStr = (uint8_t*)attrData;
      len = BUILD_UINT16( pStr[0], pStr[1] );
      buf = gp_memcpy( buf, pStr, len+2 ); // Including length field
      break;

    case ZCL_DATATYPE_128_BIT_SEC_KEY:
      pStr = (uint8_t*)attrData;
      buf = gp_memcpy( buf, pStr, SEC_KEY_LEN );
      break;

    case ZCL_DATATYPE_NO_DATA:
    case ZCL_DATATYPE_UNKNOWN:
      // Fall through

    default:
      break;
  }

  return ( buf );
}



/*********************************************************************
 * @fn      gp_buffer_uint32
 *
 * @brief   Abstract function to break a uin32 into a buffer
 *
 * @param   buf - pointer to destination memory
 * @param   val - value to break
 *
 * @return  pointer to buffer after set
 */
static uint8_t *gp_buffer_uint32(uint8_t *buf, uint32_t val)
{
    *buf++ = BREAK_UINT32(val, 0);
    *buf++ = BREAK_UINT32(val, 1);
    *buf++ = BREAK_UINT32(val, 2);
    *buf++ = BREAK_UINT32(val, 3);

    return buf;
}

/**************************************************
 * @fn      gpdDuplicateFrameInt
 *
 * @brief   assign point to the received structure.
 *
 * @param   pDataReq - pointer to structure data request
 *
 * @return  status
 */
void gpdDuplicateFrameInit(ApiMac_mcpsDataReq_t *pDataReq)
{
    pDuplicateFrame = pDataReq;
}

/******************************************************************************
 Static Functions
 *****************************************************************************/

/**************************************************
 * @fn      gpdSendFrame
 *
 * @brief   Calls ApiMac_mcpsDataReq() to send the GPDF
 *          frame.
 *
 * @param pGPDF    - pointer to GPDF structure data
 * @param frameLen - frame length
 * @param channel  - channel to send the frame
 * @return  ApiMac_status_t
 */
static ApiMac_status_t gpdSendFrame(uint8_t *pGPDF, uint8_t frameLen, uint8_t channel)
{
    ApiMac_mcpsDataReq_t dataReq;
    ApiMac_status_t stat = ApiMac_status_noResources;

    if(pGPDF != NULL)
    {
        memset(&dataReq, 0, sizeof(ApiMac_mcpsDataReq_t));
        dataReq.txOptions.useGreenPower = FALSE;
        dataReq.channel = channel;
        dataReq.power = 0x50;
        dataReq.gpOffset = 0x05;
        dataReq.gpDuration = 0x00;
        dataReq.msduHandle = 0;

        ApiMac_mlmeSetReqUint8(ApiMac_attribute_logicalChannel, channel);

        dataReq.dstAddr.addr.shortAddr = 0xFFFF;
        dataReq.dstAddr.addrMode = ApiMac_addrType_short;
#if (GPD_APP_ID == GPD_APP_TYPE_IEEE_ID)
        dataReq.srcAddrMode = ApiMac_addrType_extended;
#endif
        dataReq.dstPanId = 0xFFFF;
        dataReq.msdu.len = frameLen;
        dataReq.msdu.p = OsalPort_malloc(dataReq.msdu.len);

        if (dataReq.msdu.p != NULL)
        {
            /* Copy data */
            OsalPort_memcpy( dataReq.msdu.p, pGPDF, frameLen );
            /* Call Mac Data Request */
#if (SEQUENCE_NUMBER_CAP == TRUE)
            uint8_t secNum;
            ApiMac_mlmeGetReqUint8(ApiMac_attribute_dsn, (uint8_t*)&secNum);
            stat = ApiMac_mcpsDataReq(&dataReq);
            secNum++;
            gp_nv_write(GP_NV_SEQUENCE_NUMBER, 1, &secNum);
#elif (SEQUENCE_NUMBER_CAP == FALSE)
            stat = ApiMac_mcpsDataReq(&dataReq);
#endif

            if(stat == ApiMac_status_success)
            {
                OsalPort_memcpy(pDuplicateFrame, &dataReq, sizeof(ApiMac_mcpsDataReq_t));
            }
            else
            {
                OsalPort_free(dataReq.msdu.p);
            }
        }
        OsalPort_free(pGPDF);
    }
    return(stat);
}

/******************************************************************************
 * @fn      gp_BuildMsg
 *
 * @brief   Build message for gpdf
 *
 * @param   pReq       - Pointer to GPDF request
 * @param   pMsgLen    - Pointer to store the message length
 *
 * @return  Item length, if found; zero otherwise.
 */
static void *gp_BuildMsg(gpdfReq_t *pReq, uint8_t *pMsgLen)
{
    uint8_t *pMsg = NULL;
    uint8_t *pM = NULL;
    uint8_t authLen;
    uint8_t encLen;

    // Build Message
    // NWK Frame Control || Ext NWK Frame Control || SrcID || Frame counter || Command ID || Payload;
    authLen = sizeof(pReq->NWKFrameControl);
    if( FRAME_CTRL_EXT == TRUE )
    {
      authLen += sizeof(pReq->NWKExtFC);
    }

#if (GPD_APP_ID == GPD_APP_TYPE_SRC_ID)
    //Check the frame to not be GP_MAINTENANCE_FRAME
    if((pReq->NWKFrameControl & 0x03) != GP_MAINTENANCE_FRAME)
    {
        authLen += GP_SRC_ID_LEN;
    }
#elif (GPD_APP_ID == GPD_APP_TYPE_IEEE_ID)
    authLen += sizeof(pReq->endPoint);
#endif

#if (GP_SECURITY_LEVEL == GP_SECURITY_LVL_4FC_4MIC) || (GP_SECURITY_LEVEL == GP_SECURITY_LVL_4FC_4MIC_ENCRYPT)
    authLen += sizeof(pReq->securityFrameCounter);
#endif

#if ( (GP_SECURITY_LEVEL == GP_SECURITY_LVL_4FC_4MIC) || (GP_SECURITY_LEVEL == GP_SECURITY_LVL_NO_SEC) ) // check for no security or Auth Only
    authLen += sizeof(pReq->gpdCmdID);
    authLen +=  pReq->payloadSize;
    encLen = 0; // m = 0
#elif ( GP_SECURITY_LEVEL == GP_SECURITY_LVL_4FC_4MIC_ENCRYPT ) // auth and encrypt
    // a = Hdr+Aux
    encLen = sizeof(pReq->gpdCmdID);
    encLen += pReq->payloadSize;
#endif

    *pMsgLen = authLen + encLen;
#if (GP_SECURITY_LEVEL == GP_SECURITY_LVL_4FC_4MIC) || (GP_SECURITY_LEVEL == GP_SECURITY_LVL_4FC_4MIC_ENCRYPT)
    *pMsgLen += GP_SECURITY_MIC_SIZE;
#endif

    pMsg = (uint8_t *)OsalPort_malloc(*pMsgLen);
    if( pMsg != NULL )
    {
        pM = pMsg;
        *pM++ = pReq->NWKFrameControl;
        if( FRAME_CTRL_EXT == TRUE )
        {
            *pM++ = pReq->NWKExtFC;
        }

#if (GPD_APP_ID == GPD_APP_TYPE_SRC_ID)
        //Check the frame to not be GP_MAINTENANCE_FRAME
        if((pReq->NWKFrameControl & 0x03) != GP_MAINTENANCE_FRAME)
        {
            OsalPort_memcpy(pM, (uint8_t *)&pReq->gpdId.GPDId.SrcID, GP_SRC_ID_LEN);
            pM += GP_SRC_ID_LEN;
        }
#elif (GPD_APP_ID == GPD_APP_TYPE_IEEE_ID)
        *pM++ = pReq->endPoint;
#endif

#if (GP_SECURITY_LEVEL == GP_SECURITY_LVL_4FC_4MIC) || (GP_SECURITY_LEVEL == GP_SECURITY_LVL_4FC_4MIC_ENCRYPT)
        OsalPort_memcpy(pM, &pReq->securityFrameCounter, sizeof(pReq->securityFrameCounter));
        pM += sizeof(pReq->securityFrameCounter);
#endif
        *pM++ = pReq->gpdCmdID;
        if(pReq->payloadSize > 0)
        {
            OsalPort_memcpy(pM,pReq->payload,pReq->payloadSize);
        }
    }
    return pMsg;
}


/******************************************************************************
 * @fn      gp_BuildUnsecureMsg
 *
 * @brief   Build unsecured message for the gpdf
 *
 * @param   pReq       - Pointer to GPDF request
 * @param   pMsgLen    - Pointer to store the message length
 *
 * @return  Item length, if found; zero otherwise.
 */
static void *gp_BuildUnsecureMsg(gpdfReq_t *pReq, uint8_t *pMsgLen)
{
    uint8_t *pMsg = NULL;
    uint8_t *pM = NULL;
    uint8_t msgLen;

    // Build Message
    // NWK Frame Control || Ext NWK Frame Control || SrcID || Frame counter || Command ID || Payload;
    msgLen = sizeof(pReq->NWKFrameControl);
    if( FRAME_CTRL_EXT == TRUE )
    {
        msgLen += sizeof(pReq->NWKExtFC);
    }

#if (GPD_APP_ID == GPD_APP_TYPE_SRC_ID)
    msgLen += GP_SRC_ID_LEN;
#elif (GPD_APP_ID == GPD_APP_TYPE_IEEE_ID)
    msgLen += sizeof(pReq->endPoint);
#endif

    msgLen += sizeof(pReq->gpdCmdID);
    msgLen +=  pReq->payloadSize;
    *pMsgLen = msgLen;

    pMsg = (uint8_t *)OsalPort_malloc(*pMsgLen);
    if( pMsg != NULL )
    {
        pM = pMsg;
        *pM++ = pReq->NWKFrameControl;
        if( FRAME_CTRL_EXT == TRUE )
        {
            *pM++ = pReq->NWKExtFC;
        }

#if (GPD_APP_ID == GPD_APP_TYPE_SRC_ID)
        OsalPort_memcpy(pM, (uint8_t *)&pReq->gpdId.GPDId.SrcID, GP_SRC_ID_LEN);
        pM += GP_SRC_ID_LEN;
#elif (GPD_APP_ID == GPD_APP_TYPE_IEEE_ID)
        *pM++ = pReq->endPoint;
#endif

        *pM++ = pReq->gpdCmdID;
        if(pReq->payloadSize > 0)
        {
            OsalPort_memcpy(pM,pReq->payload,pReq->payloadSize);
        }
    }
    return pMsg;
}
#if (GP_SECURITY_LEVEL == GP_SECURITY_LVL_4FC_4MIC) || (GP_SECURITY_LEVEL == GP_SECURITY_LVL_4FC_4MIC_ENCRYPT)

#if defined ( GPD_COMMISSIONING_CMD_SUPPORT )
/*********************************************************************
 * @fn        gpd_encryptKey
 *
 * @brief     Create an encrypted key
 *
 * @param     *pReq  - Pointer to GPDF request
 * @param     *key   - Pointer to key values
 * @
 */
static uint8_t gpd_encryptKey(gpdfReq_t *pReq, uint8_t *key, uint32_t frameCounter)
{
  uint8_t srcID[GP_SRC_ID_LEN];
  uint32_t id;
  uint8_t tclk[GP_KEY_LENGTH] = TC_LINK_KEY;
  uint8_t nonce[GP_NONCE_LENGTH]={0x00};
  uint8_t status = MAC_SECURITY_ERROR;
  int_fast16_t statusCrypto;
  CryptoKey cryptoKey;

  /* Initialize AES key */
  statusCrypto = CryptoKeyPlaintext_initKey(&cryptoKey, (uint8_t*)tclk, GP_KEY_LENGTH);
  if (statusCrypto == AESCCM_STATUS_SUCCESS)
  {
    AESCCM_Operation operationOneStepEncrypt;

#if(GPD_APP_ID == GPD_APP_TYPE_SRC_ID)
    *(uint32_t *)srcID = pReq->gpdId.GPDId.SrcID;
#if(EXT_OPT_SECURITY_KEY == 0)
    id = frameCounter;
#elif(EXT_OPT_SECURITY_KEY == 1)
    id = pReq->gpdId.GPDId.SrcID;
#endif
#elif (GPD_APP_ID == GPD_APP_TYPE_IEEE_ID)
    OsalPort_memcpy(srcID, pReq->gpdId.GPDId.GPDExtAddr, GP_SRC_ID_LEN);
#if(EXT_OPT_SECURITY_KEY == 0)
    id = frameCounter;
#elif(EXT_OPT_SECURITY_KEY == 1)
    id = BUILD_UINT32(pReq->gpdId.GPDId.GPDExtAddr[0],
                      pReq->gpdId.GPDId.GPDExtAddr[1],
                      pReq->gpdId.GPDId.GPDExtAddr[2],
                      pReq->gpdId.GPDId.GPDExtAddr[3]);
#endif
#endif
    //build nonce for key encryption
    gp_BuildKeyNonce(&pReq->gpdId, id, nonce);

    AESCCM_Operation_init(&operationOneStepEncrypt);
    operationOneStepEncrypt.key            = &cryptoKey;
    operationOneStepEncrypt.aad            = (uint8_t*)&srcID;
    operationOneStepEncrypt.aadLength      = GP_SRC_ID_LEN;
    operationOneStepEncrypt.input          = key;
    operationOneStepEncrypt.output         = key;
    operationOneStepEncrypt.inputLength    = GP_KEY_LENGTH;
    operationOneStepEncrypt.nonce          = nonce;
    operationOneStepEncrypt.nonceLength    = GP_NONCE_LENGTH;
    operationOneStepEncrypt.mac            = (uint8_t*)&(((gpdCommissioningCommand_t*)pReq->payload)->GPDKeyMIC);
    operationOneStepEncrypt.macLength      = GP_SECURITY_MIC_SIZE;

    statusCrypto = AESCCM_oneStepEncrypt(ZAESCCM_handle, &operationOneStepEncrypt);
    if(statusCrypto == AESCCM_STATUS_SUCCESS)
    {
      status = MAC_SUCCESS;
    }
  }
  return status;
}

/******************************************************************************
 * @fn      gp_BuildKeyNonce
 *
 * @brief   Build Nonce array for incoming or outgoing
 *
 * @param   pGpdId            - GPD id
 * @param   securityCounter   - Security Frame Control value
 * @param   *nonce            - Pointer to item nonce data
 *
 * @return  Item length, if found; zero otherwise.
 */
static void gp_BuildKeyNonce( gpd_ID_t *pGpdId, uint32_t securityCounter, uint8_t *nonce )
{
#if(GPD_APP_ID == GPD_APP_TYPE_SRC_ID)
#if(EXT_OPT_SECURITY_KEY == 1)
    nonce = OsalPort_memcpy(nonce, (uint8_t *)&pGpdId->GPDId.SrcID, GP_SRC_ID_LEN);
    nonce = OsalPort_memcpy(nonce, (uint8_t *)&pGpdId->GPDId.SrcID, GP_SRC_ID_LEN);
#else
    memset(nonce, 0, GP_SRC_ID_LEN);
    nonce += GP_SRC_ID_LEN;
    nonce = OsalPort_memcpy(nonce, (uint8_t *)&pGpdId->GPDId.SrcID, GP_SRC_ID_LEN);
#endif
#elif (GPD_APP_ID == GPD_APP_TYPE_IEEE_ID)
    nonce = OsalPort_memcpy(nonce, (uint8_t *)pGpdId->GPDId.GPDExtAddr, Z_EXATDDR_LEN);
#endif
    nonce = OsalPort_memcpy(nonce, &securityCounter, sizeof(securityCounter));

    *nonce = GP_NONCE_SEC_CONTROL;
#if ( (GPD_APP_ID == GPD_APP_TYPE_IEEE_ID) && (EXT_OPT_SECURITY_KEY == 0) )
    *nonce |= 0xC0;
#endif
}
#endif

/******************************************************************************
 * @fn      gp_BuildNonce
 *
 * @brief   Build Nonce array for incoming or outgoing
 *
 * @param   pGpdId            - GPD id
 * @param   securityCounter   - Security Frame Control value
 * @param   *nonce            - Pointer to item nonce data
 *
 * @return  Item length, if found; zero otherwise.
 */
static void gp_BuildNonce( gpd_ID_t *pGpdId, uint32_t securityCounter, uint8_t *nonce )
{
#if(GPD_APP_ID == GPD_APP_TYPE_SRC_ID)
    OsalPort_memcpy(nonce, (uint8_t *)&pGpdId->GPDId.SrcID, GP_SRC_ID_LEN);
    nonce += GP_SRC_ID_LEN;
    OsalPort_memcpy(nonce, (uint8_t *)&pGpdId->GPDId.SrcID, GP_SRC_ID_LEN);
    nonce += GP_SRC_ID_LEN;
#elif (GPD_APP_ID == GPD_APP_TYPE_IEEE_ID)
    OsalPort_memcpy(nonce, (uint8_t *)pGpdId->GPDId.GPDExtAddr, Z_EXATDDR_LEN);
    nonce += Z_EXATDDR_LEN;
#endif
    OsalPort_memcpy(nonce, &securityCounter, sizeof(securityCounter));
    nonce += sizeof(securityCounter);
    *nonce = GP_NONCE_SEC_CONTROL;
}

/*********************************************************************
 * @fn      gpd_ccmStar
 *
 * @brief   processing for the ccm* protocol for Green Power device
 *
 * @param   ed_flag - Apply encryption flag
 *          pReq    - Pointer to GPDF request
 *          msgLen  - Pointer to store the message length
 *
 * @return  void*   - Pointer to message
 */
void* gpd_ccmStar( uint8_t ed_flag, gpdfReq_t *pReq, uint8_t *msgLen)
{
  uint8_t authLen;
  uint8_t encLen;
  uint8_t *pMic = NULL;
  uint8_t *secMsg = NULL;
  uint8_t key[] = GP_SECURITY_KEY;
  uint8_t nonce[GP_NONCE_LENGTH]={0x00};
  int_fast16_t statusCrypto;
  CryptoKey cryptoKey;
  AESCCM_Operation operationOneStepAESCCM;

  /* Initialize AES key */
  statusCrypto = CryptoKeyPlaintext_initKey(&cryptoKey, (uint8_t*)key, SEC_KEY_LEN);
  gp_BuildNonce( &pReq->gpdId, pReq->securityFrameCounter, nonce );

  secMsg = gp_BuildMsg( pReq, msgLen );
  if(secMsg == NULL)
  {
      return NULL;
  }
  pMic = secMsg + (*msgLen - GP_SECURITY_MIC_SIZE);

  AESCCM_Operation_init(&operationOneStepAESCCM);

  // Assume secMsg[] is sized large enough to insert MIC-T or encrypted MIC-U
  // after payload, but cannot assume that we have enough space (16 bytes)
  // to use it as Cstate buffer. We pass ccmLVal = 2
  authLen = *msgLen - GP_SECURITY_MIC_SIZE;
#if (GP_SECURITY_LEVEL == GP_SECURITY_LVL_4FC_4MIC)
  operationOneStepAESCCM.input          = NULL;
  operationOneStepAESCCM.output         = NULL;
  operationOneStepAESCCM.inputLength    = 0;
#elif (GP_SECURITY_LEVEL == GP_SECURITY_LVL_4FC_4MIC_ENCRYPT)
  encLen = sizeof(pReq->gpdCmdID) + pReq->payloadSize;
  authLen -= encLen;
  operationOneStepAESCCM.input          = secMsg+authLen;
  operationOneStepAESCCM.output         = secMsg+authLen;
  operationOneStepAESCCM.inputLength    = encLen;
#endif

  operationOneStepAESCCM.key            = &cryptoKey;
  operationOneStepAESCCM.aad            = secMsg;
  operationOneStepAESCCM.aadLength      = authLen;
  operationOneStepAESCCM.nonce          = nonce;
  operationOneStepAESCCM.nonceLength    = GP_NONCE_LENGTH;
  operationOneStepAESCCM.mac            = pMic;
  operationOneStepAESCCM.macLength      = GP_SECURITY_MIC_SIZE;

  if (ed_flag == true) // Apply Auth & Encryption
  {
    statusCrypto = AESCCM_oneStepEncrypt(ZAESCCM_handle, &operationOneStepAESCCM);
  }
  else // Apply Decryption and Check Auth
  {
    statusCrypto = AESCCM_oneStepDecrypt(ZAESCCM_handle, &operationOneStepAESCCM);
  }
  if(statusCrypto != AESCCM_STATUS_SUCCESS)
  {
    OsalPort_free(secMsg);
  }
  return secMsg;
}
#endif

#if defined ( GPD_COMMISSIONING_CMD_SUPPORT )
/**************************************************
 * @fn      getCommissioningCmdLen
 *
 * @brief   Calculate the length of commissioning cmd.
 *
 * @return  payloadLen
 */

static uint8_t getCommisioningCmdLen(void)
{
    uint8_t payloadLen;

    // gpdDeviceID + options
    payloadLen = 2;
#if ( OPT_EXTENDED_OPTIONS == TRUE )
    // + extendedOptions
    payloadLen += 1;
#endif
#if (EXOPT_GPD_KEY_PRESENT == TRUE)
    payloadLen += GP_KEY_LENGTH;
#if (EXOPT_KEY_ENCRYPTION == TRUE)
    payloadLen += GP_SECURITY_MIC_SIZE;
#endif
    payloadLen += 4;
#endif

#if (OPT_APPLICATION_INFORMATION == TRUE)
    payloadLen += 1;
#endif
  //TODO: calculate App info payloads
    return payloadLen;
}
#endif

#if (GP_SECURITY_LEVEL == GP_SECURITY_LVL_4FC_4MIC) || (GP_SECURITY_LEVEL == GP_SECURITY_LVL_4FC_4MIC_ENCRYPT)
/**************************************************
 * @fn      gpd_setSecurityFrameCounter
 *
 * @brief   Set the value of the security frame counter
 *
 * @param   frameCounter - value to set GPD sec frame counter
 *
 */
void gpd_setSecurityFrameCounter(uint32_t frameCounter)
{
  gpdSecurityFrameCounter = frameCounter;
}
#endif
