/**************************************************************************************************
  Filename:       mt_znp.h
  Revised:        $Date: 2011-08-22 16:18:12 -0700 (Mon, 22 Aug 2011) $
  Revision:       $Revision: 27245 $

  Description:    Declarations for the ZNP sub-module of the MT API.


  Copyright 2011 Texas Instruments Incorporated.

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
#ifndef MT_ZNP_H
#define MT_ZNP_H

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------------------------------------
 *                                          Includes
 * ------------------------------------------------------------------------------------------------
 */

#include "comdef.h"
#include "nwk.h"
#include "zd_app.h"

/* ------------------------------------------------------------------------------------------------
 *                                          Constants
 * ------------------------------------------------------------------------------------------------
 */

#define MT_ZNP_EP_ID_LIST_MAX               4
#define MT_ZNP_ZDO_MSG_CB_LIST_MAX          16
//efine MAX_ZDO_CB_FUNC                     6 - currently in zd_app.h

// mt_znp_basic_cfg_t.cmdDisc bit masks:
#define MT_ZNP_CMD_DISC_RESET_NWK           0x80
#define MT_ZNP_CMD_DISC_ZDO_START           0x40

/* ------------------------------------------------------------------------------------------------
 *                                           Typedefs
 * ------------------------------------------------------------------------------------------------
 */

typedef struct {
  uint16_t id;  // Application Profile Id.
  uint8_t ep;   // Application EndPoint.
} ep_id_t;

typedef ep_id_t ep_id_list_t[MT_ZNP_EP_ID_LIST_MAX];

// An array of the ZDO function callbacks registered to MT_ZDO.
typedef uint8_t zdo_func_cb_list_t[MAX_ZDO_CB_FUNC];

// An array of the uint16_t Cluster Id's registered to receive ZDO message callbacks.
typedef uint16_t zdo_msg_cb_list_t[MT_ZNP_ZDO_MSG_CB_LIST_MAX];

typedef struct {
  uint32_t        basicRspRate;                       // Rate at which to generate this AREQ response.

  uint32_t        zgChannelList;                      // ZCD_NV_CHANLIST.
  uint16_t        zgConfigPANID;                      // ZCD_NV_PANID.
  uint8_t         zgStartupOptions;                   // ZCD_NV_STARTUP_OPTION.
  uint8_t         zgDeviceLogicalType;                // ZCD_NV_LOGICAL_TYPE.

  // A high-use subset of the nwkIB_t _NIB.
  uint16_t        nwkDevAddress;                      // Device's short address.
  uint16_t        nwkCoordAddress;                    // Parent's short address.
  uint16_t        nwkPanId;                           // Device's PanId.
  uint8_t         nwkLogicalChannel;                  // Current logical channel in use.
  nwk_states_t  nwkState;                           // Device's network state.
  uint8_t         nwkCoordExtAddress[Z_EXTADDR_LEN];  // Parent's IEEE address.

  uint8_t         aExtendedAddress[Z_EXTADDR_LEN];    // 64-bit Extended Address of this device.

  devStates_t   devState;                           // ZDO device state.
#if defined ( INTER_PAN ) || defined ( BDB_TL_INITIATOR ) || defined ( BDB_TL_TARGET )
  uint8_t appEndPoint;
  uint8_t         spare1[2];
#else
  uint8_t         spare1[3];
#endif

  ep_id_list_t  epIdList;
  zdo_msg_cb_list_t zdoMsgCBList;
  zdo_func_cb_list_t zdoFuncCBList;
} mt_znp_basic_rsp_t;

typedef struct {
  uint32_t        basicRspRate;                       // Rate at which to generate the basic response.

  uint32_t        zgChannelList;                      // ZCD_NV_CHANLIST.
  uint16_t        zgConfigPANID;                      // ZCD_NV_PANID.
  uint8_t         zgDeviceLogicalType;                // ZCD_NV_LOGICAL_TYPE.

  uint8_t         cmdDisc;                            // Discrete command bits.
} mt_znp_basic_cfg_t;

/* ------------------------------------------------------------------------------------------------
 *                                          Global Variables
 * ------------------------------------------------------------------------------------------------
 */

extern uint32_t MT_PeriodicMsgRate;

/* ------------------------------------------------------------------------------------------------
 *                                          Functions
 * ------------------------------------------------------------------------------------------------
 */

#if defined (MT_ZNP_FUNC)
/**************************************************************************************************
 * @fn          MT_ZnpCommandProcessing
 *
 * @brief       Process all MT ZNP commands.
 *
 * input parameters
 *
 * @param       pBuf - Pointer to the MT buffer containing a ZNP command.
 *
 * output parameters
 *
 * None.
 *
 * @return      A 1-byte SRSP value or MT_RPC_SRSP_SENT or MT_RPC_SRSP_SENT;
 */
uint8_t MT_ZnpCommandProcessing(uint8_t *pBuf);

/**************************************************************************************************
 * @fn          MT_ZnpBasicResponse
 *
 * @brief       Build and send the ZNP Basic Response to the ZAP.
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      true if message built and sent; false otherwise.
 */
bool MT_ZnpBasicRsp(void);
#endif

#ifdef __cplusplus
};
#endif

#endif
/**************************************************************************************************
*/
