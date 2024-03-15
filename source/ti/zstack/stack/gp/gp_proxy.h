/**************************************************************************************************
  Filename:       gp_proxy.h
  Revised:        $Date: 2016-05-23 11:51:49 -0700 (Mon, 23 May 2016) $
  Revision:       $Revision: - $

  Description:    This file contains the common Green Power stub definitions.


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

#if !defined (DISABLE_GREENPOWER_BASIC_PROXY) && (ZG_BUILD_RTR_TYPE)

#ifndef PROXYBASIC_H
#define PROXYBASIC_H


#ifdef __cplusplus
extern "C"
{
#endif



/*********************************************************************
 * INCLUDES
 */

#include "zcomdef.h"
#include "zmac.h"
#include "af.h"
#include "zcl_green_power.h"
#include "cgp_stub.h"
#include "gp_interface.h"

/*********************************************************************
 * MACROS
 */

 /*********************************************************************
 * ENUM
 */

 /*********************************************************************
 * CONSTANTS
 */

 /*********************************************************************
 * TYPEDEFS
 */
typedef struct
{
  gpdID_t       gpdId;                //Address identifier for Green Power Device that is in commissioning.
  uint8_t         keyPresent;           //Flag that tells if key is present on commissioning frame.
  uint8_t         pkey[SEC_KEY_LEN];    //Pointer to security key.
} gpdCommissioningDevice_t;

/*********************************************************************
 * GLOBAL VARIABLES
 */
extern uint8_t  zgGP_ProxyCommissioningMode;     //Global flag that states if in commissioning mode or in operational mode.
extern uint16_t gp_commissionerAddress;          //Address to which send the notifications during commissioning mode
extern uint8_t  gp_unicastCommunication;         //Flag to indicate if the commissioner requested unicast communication or not.
#if !defined (USE_ICALL)
extern ZDO_DeviceAnnce_t*                  GP_aliasConflictAnnce;
#endif

/*********************************************************************
 * FUNCTION MACROS
 */


/*********************************************************************
 * FUNCTIONS
 */

/*
 * @brief   Callback from the ZCL GreenPower Cluster Library when
 *          it received an Gp Pairing Command for this application.
 */
extern void zclGp_GpPairingCommandCB( zclGpPairing_t *pCmd );

/*
 * @brief   Callback from the ZCL GreenPower Cluster Library when
 *          it received a Gp proxy table req.
 */
extern void zclGp_GpProxyTableReqCB( zclGpTableRequest_t *pCmd );

/*
 * @brief   Callback from the ZCL GreenPower Cluster Library when
 *          it received a Gp Response command. (Ref A.3.5.2.1)
 */
extern void zclGp_GpResponseCommandCB(zclGpResponse_t *pCmd);

/*
 * @brief   Callback from the ZCL GreenPower Cluster Library when
 *          it received a Gp Commissioning Mode command.
 */
extern void zclGp_GpProxyCommissioningModeCB(zclGpProxyCommissioningMode_t* pCmd);

/*
 * @brief       General function to init the NV items for proxy table
 */
extern uint8_t gp_ProxyTblInit( uint8_t resetTable );

/*
 * @brief   Stop Commissioning mode, either timeout or pairing success
 */
extern void gp_stopCommissioningMode(void);

/*
 * @brief   General function to get proxy table entry by gpdID (GP Src ID or Extended Adddress)
 */
extern uint8_t gp_getProxyTableByGpId(gpdID_t *gpdID, uint8_t *pEntry, uint16_t* NvProxyTableIndex);

/*
 * @brief   General function to get proxy table entry by NV index
 */
extern uint8_t gp_getProxyTableByIndex( uint16_t nvIndex, uint8_t *pEntry );

/*
 * @brief   Handle Gp attributes.
 */
extern ZStatus_t zclGpp_ReadWriteAttrCB( uint16_t clusterId, uint16_t attrId, uint8_t oper,
                                         uint8_t *pValue, uint16_t *pLen );

/*
 * @brief       General function fill the proxy table vector
 */
extern uint8_t gp_PairingUpdateProxyTbl( gpPairingCmd_t* payload );

/*
 * @brief This passes the MCPS data indications received in MAC to the application
 */
extern void gp_dataIndProxy(gp_DataInd_t *gp_DataInd);

/*
 * @brief       Performs Security Operations according to Proxy
 */
extern uint8_t gp_SecurityOperationProxy( gp_SecReq_t* pInd, uint8_t* pKeyType, void* pKey);

/*
 * @brief Primitive to notify GP EndPoint the status of a previews DataReq
 */
extern void GP_DataCnf(gp_DataCnf_t *gp_DataCnf);

/*
 * @brief   Register a callback in which the application will be notified about
 *          commissioning mode indication
 */
extern void gp_RegisterCommissioningModeCB(gpCommissioningMode_t gpCommissioningModeCB);

/*
 * @brief   Return to operational channel after commissioning a GPD
 */
extern void gp_returnOperationalChannel(void);

/*
* @brief       Get Green Power Proxy commissioning mode
*/
extern bool gp_GetProxyCommissioningMode(void);

/*
 * @brief       Enable or disable Green Power Proxy commissioning mode
 */
extern void gp_SetProxyCommissioningMode(bool enabled);

/*
 * @brief       General function to check if it has the announced device
 *              listed in the SinkAddressList and look for address conflict
 *              resolution.
 */
extern void gp_CheckAnnouncedDevice ( uint8_t *sinkIEEE, uint16_t sinkNwkAddr );

/*
 * @brief       Populate the given item data
 */
extern uint8_t pt_ZclReadGetProxyEntry( uint16_t nvId, uint8_t* pData, uint8_t* len );

/*
 * @brief       To update the proxy table NV vectors
 */
extern uint8_t gp_UpdateProxyTbl( uint8_t* pEntry, uint32_t options, uint8_t conflictResolution );

#ifdef __cplusplus
}
#endif


#endif /* PROXYBASIC_H */

#endif // #if !defined (DISABLE_GREENPOWER_BASIC_PROXY) && (ZG_BUILD_RTR_TYPE)
