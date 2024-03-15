
/******************************************************************************
  Filename:       ota_client.h
  Revised:        $Date: 2015-04-14 21:59:34 -0700 (Tue, 14 Apr 2015) $
  Revision:       $Revision: 43420 $

  Description:    Over-the-Air Upgrade Cluster client App definitions.


  Copyright 2015 Texas Instruments Incorporated.

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
******************************************************************************/

#ifndef OTA_CLIENT_APP_H
#define OTA_CLIENT_APP_H

#ifdef __cplusplus
extern "C"
{
#endif

/******************************************************************************
 * INCLUDES
 */
#include "zcl.h"
#include "ota_common.h"
#include "zcl_ota.h"
#include "zd_object.h"
#ifndef CUI_DISABLE
#include "cui.h"
#endif
#include <ti/sysbios/knl/Semaphore.h>

/******************************************************************************
 * CONSTANTS
 */

// Callback events to application from OTA
#define ZCL_OTA_START_CALLBACK                        0
#define ZCL_OTA_DL_COMPLETE_CALLBACK                  1

#define SAMPLEAPP_BL_OFFSET                           0x1F001

#define ST_FULL_IMAGE                                 0x01
#define ST_APP_ONLY_IMAGE                             0x02
#define ST_STACK_ONLY_IMAGE                           0x03
#define ST_FULL_FACTORY_IMAGE                         0x04

// OTA_Storage_Status_t status codes
typedef enum {
    OTA_Storage_Status_Success, ///< Success
    OTA_Storage_Failed,         ///< Fail
    OTA_Storage_CrcError,       ///< Acknowledgment or Response Timed out
    OTA_Storage_FlashError,     ///< flash access error
    OTA_Storage_Aborted,        ///< Canceled by application
    OTA_Storage_Rejected,       ///< OAD request rejected by application
} OTA_Storage_Status_t;

/******************************************************************************
 * TYPEDEFS
 */

/******************************************************************************
 * GLOBAL VARIABLES
 */

extern uint8_t zclOTA_ClientPdState;
extern uint32_t zclOTA_DownloadedImageSize;  // Downloaded image size


/******************************************************************************
 * FUNCTIONS
 */

/*********************************************************************
 * @fn      otaClient_setAttributes
 *
 * @brief   Sets pointers to attributes used by OTA Client module
 *
 * @param   *attrs - set of attributes from the application
 * @param   numAttrs - number of attributes in the list
 *
 * @return  void
 */
extern void otaClient_setAttributes( const zclAttrRec_t *attrs, uint8_t numAttrs );

/******************************************************************************
 *
 * @fn      otaClient_CreateTask
 *
 * @brief   Call to pass generated endpoint information and initialize task
 *
 * @param   endpoint - Endpoint to register
 *          epDesc  - Endpoint Descriptor
 *          attrArraySize - Size of attribute array
 *          attrs - Array of endpoint attributes
 *          cmdsArraySize - Size of command array
 *          cmds  - Array of commands for endpoint
 *
 * @return  none
 */
extern void otaClient_CreateTask(uint8_t endpoint, endPointDesc_t epDesc, uint8_t attrArraySize,
                           const zclAttrRec_t attrs[], uint8_t cmdsArraySize, const zclCommandRec_t cmds[]);

/******************************************************************************
 *
 * @fn      otaClient_Init
 *
 * @brief   Call to initialize the OTA Client Task
 *
 * @param   task_id
 *
 * @return  none
 */
extern void otaClient_Init ( Semaphore_Handle appSem, uint8_t stEnt, uint32_t cuiHandle);

/******************************************************************************
 * @fn          otaClient_event_loop
 *
 * @brief       Event Loop Processor for OTA Client task.
 *
 * @param       task_id - TaskId
 *              events - events
 *
 * @return      Unprocessed event bits
 */
extern uint16_t otaClient_event_loop( void );

extern void otaClient_ProcessIEEEAddrRsp( ZDO_NwkIEEEAddrResp_t *pMsg );

extern bool otaClient_ProcessMatchDescRsp ( ZDO_MatchDescRsp_t *pMsg );

/******************************************************************************
 * @fn      otaClient_RequestNextUpdate
 *
 * @brief   Called by an application after discovery of the OTA server
 *          to initiate the query next image of the OTA server.
 *
 * @param   srvAddr - Short address of the server
 * @param   srvEndPoint - Endpoint on the server
 *
 * @return  ZStatus_t
 */
extern void otaClient_RequestNextUpdate(uint16_t srvAddr, uint8_t srvEndPoint);

/******************************************************************************
 * @fn      otaClient_DiscoverServer
 *
 * @brief   Call to discover the OTA Client
 *
 * @param   task_id
 *
 * @return  none
 */
extern void otaClient_DiscoverServer( void );//uint8_t task_id );

/******************************************************************************
 * @fn      otaClient_InitializeSettings
 *
 * @brief   Call to initialize the OTA Client
 *
 * @param   task_id
 *
 * @return  none
 */

extern void otaClient_InitializeSettings( void );//uint8_t task_id);

/******************************************************************************
 * @fn      otaClient_SetEndpoint
 *
 * @brief   Set OTA endpoint.
 *
 * @param   endpoint - endpoint ID from which OTA functions can be accessed
 *
 * @return  true if endpoint set, else false
 */
extern bool otaClient_SetEndpoint( uint8_t endpoint);


/******************************************************************************
 * @fn          otaClient_loadExtImage
 *
 * @brief       Load the image from external flash
 *              and reboot.
 *
 * @param       none
 *
 * @return      none
 */
extern void otaClient_loadExtImage(uint8_t imageSelect);


/*********************************************************************
 * @fn          otaClient_UpdateStatusLine
 *
 * @brief       Generate part of the OTA Info string
 *
 * @param       none
 *
 * @return      none
 */
extern void otaClient_UpdateStatusLine(void);



#ifdef FACTORY_IMAGE
/******************************************************************************
 * @fn          otaClient_hasFactoryImage
 *
 * @brief   This function check if the valid factory image exists on external
 *          flash
 *
 * @param   None
 *
 * @return  TRUE If factory image exists on external flash, else FALSE
 *
 */
extern bool otaClient_hasFactoryImage(void);


/*******************************************************************************
 * @fn      otaClient_saveFactoryImage
 *
 * @brief   This function creates factory image backup of current running image
 *
 * @return  rtn  OTA_Storage_Status_Success/OTA_Storage_FlashError
 */
extern uint8_t otaClient_saveFactoryImage(void);
#endif

#endif /*OTA_CLIENT_APP_H*/
