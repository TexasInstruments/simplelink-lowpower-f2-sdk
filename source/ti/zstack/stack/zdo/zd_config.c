/**************************************************************************************************
  Filename:       ZDConfig.c
  Revised:        $Date: 2013-05-07 13:02:49 -0700 (Tue, 07 May 2013) $
  Revision:       $Revision: 34167 $


  Description:    This file contains the configuration attributes for the Zigbee Device Object.
                  These are references to Configuration items that MUST be defined in ZDApp.c.
                  The names mustn't change.


  Copyright 2004-2013 Texas Instruments Incorporated.

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

/*********************************************************************
 * INCLUDES
 */
#include "zcomdef.h"
#include "af.h"
#include "zd_object.h"
#include "zd_config.h"

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
NodeDescriptorFormat_t ZDO_Config_Node_Descriptor;
NodePowerDescriptorFormat_t ZDO_Config_Power_Descriptor;

/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */

/*********************************************************************
 * @fn      ZDConfig_InitDescriptors()
 *
 * @brief   Setup the ZDO descriptors
 *             Node, Power
 *
 * @param   none
 *
 * @return  none
 */
void ZDConfig_InitDescriptors( void )
{
  ZDConfig_UpdateNodeDescriptor();
  ZDConfig_UpdatePowerDescriptor();
}

/*********************************************************************
 * @fn      ZDConfig_UpdateNodeDescriptor()
 *
 * @brief   Update the ZDO Node Descriptor
 *
 * @param   none
 *
 * @return  none
 */
void ZDConfig_UpdateNodeDescriptor( void )
{
  // Build the Node Descriptor
  if ( ZG_BUILD_COORDINATOR_TYPE && ZG_DEVICE_COORDINATOR_TYPE )
    ZDO_Config_Node_Descriptor.LogicalType = NODETYPE_COORDINATOR;
  else if ( ZSTACK_ROUTER_BUILD )
    ZDO_Config_Node_Descriptor.LogicalType = NODETYPE_ROUTER;
  else if ( ZSTACK_END_DEVICE_BUILD )
    ZDO_Config_Node_Descriptor.LogicalType = NODETYPE_DEVICE;

  ZDO_Config_Node_Descriptor.ComplexDescAvail = FALSE;      // set elsewhere
  ZDO_Config_Node_Descriptor.UserDescAvail = FALSE;         // set elsewhere
  ZDO_Config_Node_Descriptor.Reserved = 0;                  // Reserved
  ZDO_Config_Node_Descriptor.APSFlags = 0;                  // NO APS flags
  ZDO_Config_Node_Descriptor.FrequencyBand = NODEFREQ_2400; // Frequency Band

  // MAC Capabilities
  if ( ZSTACK_ROUTER_BUILD )
  {
    ZDO_Config_Node_Descriptor.CapabilityFlags
              = (CAPINFO_DEVICETYPE_FFD | CAPINFO_POWER_AC |
                 CAPINFO_RCVR_ON_IDLE | CAPINFO_ALLOC_ADDR);

    if ( ZG_BUILD_COORDINATOR_TYPE && ZG_DEVICE_COORDINATOR_TYPE )
      ZDO_Config_Node_Descriptor.CapabilityFlags |= CAPINFO_ALTPANCOORD;
  }
  else if ( ZSTACK_END_DEVICE_BUILD )
  {
    ZDO_Config_Node_Descriptor.CapabilityFlags = (CAPINFO_DEVICETYPE_RFD | CAPINFO_ALLOC_ADDR);

#if ( RFD_RX_ALWAYS_ON_CAPABLE == TRUE )
  if ( ZG_DEVICE_ENDDEVICE_TYPE && zgRxAlwaysOn == TRUE )
  {
    ZDO_Config_Node_Descriptor.CapabilityFlags |= CAPINFO_RCVR_ON_IDLE;
  }
#endif
  }

  // Manufacturer Code - *YOU FILL IN*
  ZDO_Config_Node_Descriptor.ManufacturerCode[0] = 0;
  ZDO_Config_Node_Descriptor.ManufacturerCode[1] = 0;

  // Maximum Buffer Size
  ZDO_Config_Node_Descriptor.MaxBufferSize = MAX_BUFFER_SIZE;

  // Maximum Incoming Transfer Size Field
  ZDO_Config_Node_Descriptor.MaxInTransferSize[0] = LO_UINT16( MAX_TRANSFER_SIZE );
  ZDO_Config_Node_Descriptor.MaxInTransferSize[1] = HI_UINT16( MAX_TRANSFER_SIZE );

  // Maximum Outgoing Transfer Size Field
  ZDO_Config_Node_Descriptor.MaxOutTransferSize[0] = LO_UINT16( MAX_TRANSFER_SIZE );
  ZDO_Config_Node_Descriptor.MaxOutTransferSize[1] = HI_UINT16( MAX_TRANSFER_SIZE );

  // Set the current stack revision
  ZDO_Config_Node_Descriptor.ServerMask |= (STACK_COMPLIANCE_CURRENT_REV << STACK_COMPLIANCE_CURRENT_REV_POS);

  // Descriptor Capability Field - extended active endpoint list and
  // extended simple descriptor are not supported.
  ZDO_Config_Node_Descriptor.DescriptorCapability = 0;
}

/*********************************************************************
 * @fn      ZDConfig_UpdatePowerDescriptor()
 *
 * @brief   Update the ZDO Power Descriptor
 *
 * @param   none
 *
 * @return  none
 */
void ZDConfig_UpdatePowerDescriptor( void )
{
  // Build the Power Descriptor
  if ( ZSTACK_ROUTER_BUILD )
  {
    ZDO_Config_Power_Descriptor.PowerMode = NODECURPWR_RCVR_ALWAYS_ON;
    ZDO_Config_Power_Descriptor.AvailablePowerSources = NODEAVAILPWR_MAINS;
    ZDO_Config_Power_Descriptor.CurrentPowerSource = NODEAVAILPWR_MAINS;
    ZDO_Config_Power_Descriptor.CurrentPowerSourceLevel = NODEPOWER_LEVEL_100;
  }
  else if ( ZSTACK_END_DEVICE_BUILD )
  {
    if ( nwk_GetConfigPollRate(POLL_RATE_TYPE_DEFAULT) )
    {
      ZDO_Config_Power_Descriptor.PowerMode = NODECURPWR_RCVR_AUTO;
    }
    else
    {
      ZDO_Config_Power_Descriptor.PowerMode = NODECURPWR_RCVR_STIM;
    }

    ZDO_Config_Power_Descriptor.AvailablePowerSources = NODEAVAILPWR_RECHARGE;
    ZDO_Config_Power_Descriptor.CurrentPowerSource = NODEAVAILPWR_RECHARGE;
    ZDO_Config_Power_Descriptor.CurrentPowerSourceLevel = NODEPOWER_LEVEL_66;
  }
}

/*********************************************************************
*********************************************************************/


