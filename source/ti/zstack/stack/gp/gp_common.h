/**************************************************************************************************
  Filename:       gp_common.h
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

#ifndef GP_COMMON_H
#define GP_COMMON_H



#ifdef __cplusplus
extern "C"
{
#endif



/*********************************************************************
 * INCLUDES
 */
#include "cgp_stub.h"
#include "nwk_globals.h"

#if (!defined (DISABLE_GREENPOWER_BASIC_PROXY) && (ZG_BUILD_RTR_TYPE))
#include <ti/drivers/dpl/ClockP.h>
#include <ti/sysbios/knl/Semaphore.h>
#include "zcomdef.h"
#include "zmac.h"
#include "af.h"
#include "gp_proxy.h"
#include "gp_sink.h"
#include "zstack.h"
#include "gp_interface.h"
#endif
/*********************************************************************
 * MACROS
 */

 /*********************************************************************
 * ENUM
 */
enum
{
  DGP_HANDLE_TYPE,
  GPEP_HANDLE_TYPE,
};

 /*********************************************************************
 * CONSTANTS
 */

// GP Shared key
#define GP_SHARED_KEY             { 0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,\
                                    0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF }

#define GP_DATA_IND_QUEUE_MAX_ENTRY    5



#define GP_COMM_OPT_ACTION_MASK                     0x01
#define GP_COMM_OPT_EXIT_ON_WINDOW_EXPIRATION_MASK  0x02
#define GP_COMM_OPT_EXIT_ON_PAIRING_SUCCESS_MASK    0x04
#define GP_COMM_OPT_EXIT_ON_GP_COMM_MODE_EXIT_MASK  0x08
#define GP_COMM_OPT_CHANNEL_PRES_MASK               0x10
#define GP_COMM_OPT_UNICAST_COMM_MASK               0x20

#define GP_SHARED_KEY_TYPE                              0x00
#define GP_INDIVIDUAL_KEY_TYPE                          0x01

#define GP_SECURITY_KEY_TYPE_NO_KEY                     0x00
#define GP_SECURITY_KEY_TYPE_ZIGBEE_NWK_KEY             0x01
#define GP_SECURITY_KEY_TYPE_GPD_GROUP_KEY              0x02
#define GP_SECURITY_KEY_TYPE_NWK_KEY_DERIVED_GPD_GROUP  0x03
#define GP_SECURITY_KEY_TYPE_OUT_OF_BOX_GPD_KEY         0x04
#define GP_SECURITY_KEY_TYPE_DERIVED_IND_GPD_KEY        0x07

#define GP_RSP_CMD_OPT_APP_ID_MASK                       0x07
#define GP_RSP_CMD_OPT_TRANSMIT_ON_ENDPOINT_MATCH_MASK   0x08

#define GP_COMMISSIONING_COMMAND_ID                      0xE0  //Data frame
#define GP_DECOMMISSIONING_COMMAND_ID                    0xE1  //Data frame
#define GP_SUCCESS_COMMAND_ID                            0xE2  //Data frame
#define GP_CHANNEL_REQ_COMMAND_ID                        0xE3  //Maintenance frame
#define GP_APP_DESC_COMMAND_ID                           0xE4  //Data frame
#define GP_COMMISSIONING_REPLY_COMMAND_ID                0xF0  //Data frame
#define GP_CHANNEL_CCONFIG_COMMAND_ID                    0xF3  //Maintenance frame


extern CONFIG_ITEM uint8_t zgpSharedKey[SEC_KEY_LEN];

#define GP_QUEUE_DATA_SEND_INTERVAL 50

 /*********************************************************************
 * TYPEDEFS
 */

typedef uint8_t* (*getHandleFunction_t) (uint8_t handle);


#if (!defined (DISABLE_GREENPOWER_BASIC_PROXY) && (ZG_BUILD_RTR_TYPE))
typedef struct
{
  gpdID_t       gpdId;                //Address identifier for Green Power device that generates the notification
  uint8_t         GPDEndpoint;          //Endpoint of the GP if application ID is 0b010
  uint32_t        securityFrameCounter; //Security frame counter from Green Power Device frame
  uint8_t         gpCmdId;              //GreenPower Command Id
  uint8_t         DataLen;              //command payload length
  uint8_t        *pData;                //command payload
  uint16_t        GPPAddress;           //Address of the GPP that generates the Notification
} gpdIndication_t;

/*********************************************************************
 * GLOBAL VARIABLES
 */
extern byte gp_TaskID;

extern uint8_t const ppgCommissioningWindow;
extern uint8_t gpApplicationAllowChannelChange;  //Flag to indicate if application allows or not change channel during GP commissioning
extern gp_DataInd_t        *gp_DataIndList;
#endif

extern gp_DataInd_t        *dgp_DataIndList;

#if (!defined (DISABLE_GREENPOWER_BASIC_PROXY) && (ZG_BUILD_RTR_TYPE))
extern  gp_DataReqPending_t gpTxQueueList[GP_TX_QUEUE_MAX_ENTRY];
extern const uint8_t gGP_TX_QUEUE_MAX_ENTRY;
extern gpCommissioningNotificationMsg_t commissioningNotificationMsg;
extern uint8_t gpAppEntity;
extern ClockP_Struct gpAppExpireDuplicateClk;
extern ClockP_Handle gpAppExpireDuplicateClkHandle;
extern ClockP_Struct gpAppTempMasterTimeoutClk;
extern ClockP_Handle gpAppTempMasterTimeoutClkHandle;
#endif
/*********************************************************************
 * FUNCTION MACROS
 */


/*********************************************************************
 * FUNCTIONS
 */

#if (!defined (DISABLE_GREENPOWER_BASIC_PROXY) && (ZG_BUILD_RTR_TYPE))
#if (defined (USE_ICALL) || defined (OSAL_PORT2TIRTOS))
/*
 * @brief       Initialize the green power module.
 *              This function must be called by the application during its initialization.
 */
extern void app_Green_Power_Init(uint8_t  zclSampleApp_Entity, uint32_t *zclSampleAppEvents,
                          Semaphore_Handle zclSampleAppSem, uint16_t gpDataSendEventValue,
                          uint16_t gpExpireDuplicateEventValue, uint16_t gpTempMasterEventValue);
#endif
/*
 * @brief       This function registers the Green Power endpoint and initializes
 *              the proxy and sink table.
 */
extern void gp_endpointInit( uint8_t entity );

/*
 * @brief   Find if frame is duplicates
 */
extern gp_DataInd_t* gp_DataIndFindDuplicate(uint8_t handle, uint8_t secLvl);

/*
 * @brief       General function fill the proxy table vector item
 */
extern void gp_PopulateField( uint8_t* pField, uint8_t** pData, uint8_t size );

/*
 * @param       currEntryId - NV ID of the proxy table
 *              pNew - New table entry array to be added
 */
extern uint8_t gpLookForGpd( uint16_t currEntryId, uint8_t* pNew );

/*
 * @brief       Primitive from dGP stub to GP EndPoint asking how to process a GPDF.
 */
extern void GP_SecReq(gp_SecReq_t *gp_SecReq);

/*
 * @brief       Copy proxy table memory.
 */
extern void proxyTableCpy(void *dst, void *src);

/*
 * @brief      Copy sink table memory.
 */
extern void sinkTableCpy(void *dst, void *src);

/*
 * @brief       Process the expiration of the packets in the duplicate filtering
 *              list. Assumption is the first in the queue is the first into expire.
 */
extern void gp_expireDuplicateFiltering(void);

#endif

/*
 * @brief       Append a DataInd to a list of DataInd (waiting for GP Sec Rsp List,
 *              or list to filter duplicate packets)
 */
extern void gp_DataIndAppendToList(gp_DataInd_t *gp_DataInd, gp_DataInd_t **DataIndList);

/*
 * @brief  Returns a new handle for the type of msg.
 */
extern uint8_t gp_GetHandle(uint8_t handleType);

/*
* @brief       General function fill uint16_t from pointer.
*/
extern void gp_u16CastPointer( uint8_t *data, uint8_t *p );

/*
* @brief       General function fill uint16_t reversed from pointer.
*/
extern void gp_u16CastPointerReverse( uint8_t *data, uint8_t *p );

/*
* @brief       General function fill uint16_t from pointer.
*/
extern uint16_t gp_aliasDerivation( gpdID_t *pGpdId );

#if (!defined (DISABLE_GREENPOWER_BASIC_PROXY) && (ZG_BUILD_RTR_TYPE))
/*
 * @brief       Search for a DataInd entry with matching handle
 */
extern gp_DataInd_t* gp_DataIndGet(uint8_t handle);

#if (defined (USE_ICALL) || defined (OSAL_PORT2TIRTOS))
/*
 * @brief       Process green power data indication message from gpStub
 *
 */
extern void gp_processDataIndMsg(zstack_gpDataInd_t *pInMsg);

/*
 * @brief       Process green power security request message from gpStub
 */
extern void gp_processSecRecMsg(zstack_gpSecReq_t *pInMsg);

/*
 * @brief       Process device announce with proxy table for address conflict resolution
 */
extern void gp_processCheckAnnceMsg(zstack_gpCheckAnnounce_t *pInMsg);

/*
 * @brief       Process Green Power Success command
 */
extern void gp_processGpCommissioningSuccesss(zstack_gpCommissioningSuccess_t *pInMsg);

/*
 * @brief       General function fill the proxy table vector
 */
extern void gp_sinkAddProxyEntry( uint8_t* sinkEntry );

/*
 * @brief       Process a Green Power Commissioning command security key
 *              either for key encryption or decryption
 */
extern ZStatus_t gp_processCommissioningKey(gpdID_t *pGPDId, gpdCommissioningCmd_t *pCommissioningCmd, zstack_gpEncryptDecryptCommissioningKeyRsp_t *pKeyRsp);


/*
 * @brief       Notify the application to send a Green Power Notification command
 */
extern void zcl_gpSendNotification(void);

/*
 * @brief       Notify the application to send a Green Power Commissioning Notification command
 */
extern void zcl_gpSendCommissioningNotification(void);

#endif  //(defined (USE_ICALL) || defined (OSAL_PORT2TIRTOS))

#endif  //(!defined (DISABLE_GREENPOWER_BASIC_PROXY) && (ZG_BUILD_RTR_TYPE))


#ifdef __cplusplus
}
#endif

#endif  /* GP_COMMON_H */
