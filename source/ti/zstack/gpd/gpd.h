/**************************************************************************************************
  Filename:       gpd.h
  Revised:        $Date: 2016-05-23 11:51:49 -0700 (Mon, 23 May 2016) $
  Revision:       $Revision: - $

  Description:    This file contains the Green Power Device functions.


  Copyright 2006-2014 Texas Instruments Incorporated.

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

#ifndef GPD_H_
#define GPD_H_

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "gpd_common.h"
#include "api_mac.h"
#include "zcomdef.h"
#include "nvintf.h"
#include "ti_zstack_config.h"

#include <ti/drivers/AESCCM.h>
#include <ti/drivers/cryptoutils/cryptokey/CryptoKeyPlaintext.h>

/******************************************************************************
 * DEFINES
 */


/*********************************************************************
 * GLOBAL VARIABLES
 */
extern ApiMac_sAddrExt_t ApiMac_extAddr;
extern AESCCM_Handle ZAESCCM_handle;

/******************************************************************************
 * TYPEDEFS
 */
typedef struct reportDescriptor
{
    uint8_t  reportIdentifier;
    uint8_t  reportOptions;
    uint16_t timeoutPeriod;
    uint8_t  remainingLengthReportDescriptor;
    uint8_t  dataPointOptions;
    uint16_t ClusterID;
    uint16_t manufacturerID;
    uint16_t attributeID;
    uint8_t  attributeDataType;
    uint8_t  attributeOptions;
    uint8_t  attributeOffsetWithinReport;
    uint8_t  *attributeValue;
    struct reportDescriptor *pNext;
}reportDescriptor_t;

/*              */
typedef struct
{
    uint8_t totalNumbersReports;
    uint8_t numberReport;
    uint8_t *reportDescriptor;
}applicationDescription_t;

/*              */
typedef struct
{
    uint8_t        AppID;
    union
    {
        uint32_t   SrcID;
        uint8_t   *GPDExtAddr;
    }GPDId;
}gpd_ID_t;

/*              */
typedef struct
{
    uint8_t  gpdDeviceID;
    uint8_t  options;
#if ( OPT_EXTENDED_OPTIONS == TRUE )
    uint8_t  extendedOptions;
#endif
#if (EXOPT_GPD_KEY_PRESENT == TRUE)
    uint8_t  GPDKey[ GP_KEY_LENGTH ];
#if (EXOPT_KEY_ENCRYPTION == TRUE)
    uint32_t GPDKeyMIC;
#endif  // EXOPT_KEY_ENCRYPTION
    uint32_t GPDOutgoingCounter;
#endif  // EXOPT_GPD_KEY_PRESENT
#if (OPT_APPLICATION_INFORMATION == TRUE)
    uint8_t  applicationInformation;
#endif
    uint16_t manufacturerID;
    uint16_t modelID;
    uint8_t  numberGPDCommands;
    uint8_t  GPDCommandIDList;
    uint8_t  clusterList;
    uint8_t  switchInformation;
}gpdCommissioningCommand_t;

/*              */
typedef struct
{
    uint8_t     NWKFrameControl;
    uint8_t     NWKExtFC;
    gpd_ID_t  gpdId;
#if (GPD_APP_ID == GPD_APP_TYPE_IEEE_ID)
    uint8_t     endPoint;
#endif
#if (GP_SECURITY_LEVEL == GP_SECURITY_LVL_4FC_4MIC) || (GP_SECURITY_LEVEL == GP_SECURITY_LVL_4FC_4MIC_ENCRYPT)
    uint32_t    securityFrameCounter;
#endif
    uint8_t     gpdCmdID;
    uint8_t     payloadSize;
    uint8_t     *payload;
}gpdfReq_t;


// Attribute Report
typedef struct
{
  uint16_t attrID;             // atrribute ID
  uint8_t  dataType;           // attribute data type
  uint8_t  *attrData;          // this structure is allocated, so the data is HERE
                             // - the size depends on the data type of attrID
} gpReport_t;



/******************************************************************************
 * PUBLIC FUNCTIONS
 */

/*
* @brief   Initialize AESCCM for green power device
*/
uint8_t gpdAESCCMInit(void);

/*
 * @brief   Send GPDF on requested channel
 */
extern uint8_t GreenPowerDataFrameSend( gpdfReq_t *pReq, uint8_t channel, bool secure);
#if defined ( GPD_COMMISSIONING_CMD_SUPPORT )
extern uint8_t GreenPowerCommissioningSend( gpdfReq_t *pCommissioningGpdf, uint8_t channel);
#endif
extern uint8_t GreenPowerAttributeReportingSend(uint16_t clusterID, uint8_t numAttr, gpdfReq_t *attrReportGpdf, uint8_t channel );
extern void gpdDuplicateFrameInit(ApiMac_mcpsDataReq_t *pDataReq);
#if (GP_SECURITY_LEVEL == GP_SECURITY_LVL_4FC_4MIC) || (GP_SECURITY_LEVEL == GP_SECURITY_LVL_4FC_4MIC_ENCRYPT)
extern void gpd_setSecurityFrameCounter(uint32_t frameCounter);
#endif
#ifdef __cplusplus
}
#endif

#endif /* GPD_H_ */
