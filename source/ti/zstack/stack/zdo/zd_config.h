/**************************************************************************************************
  Filename:       zd_config.h
  Revised:        $Date: 2015-05-18 16:50:53 -0700 (Mon, 18 May 2015) $
  Revision:       $Revision: 43840 $

  Description:    This file contains the configuration attributes for the Zigbee Device Object.
                  These are references to Configuration items that MUST be defined in ZDApp.c.
                  The names mustn't change.


  Copyright 2004-2014 Texas Instruments Incorporated.

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

#ifndef ZDCONFIG_H
#define ZDCONFIG_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "af.h"
#include "nwk_util.h"

/*********************************************************************
 * Enable Features
 */

//Refer to zd_profile.h
//BDB defines the minimum requirements on ZDO services (section 6.6 BDB spec 13-0302-13-00)
#ifndef ZDO_ACTIVEEP_REQUEST
  #define ZDO_ACTIVEEP_REQUEST
#endif
#ifndef ZDO_NODEDESC_REQUEST
  #define ZDO_NODEDESC_REQUEST
#endif
#ifndef ZDO_SIMPLEDESC_REQUEST
  #define ZDO_SIMPLEDESC_REQUEST
#endif
#ifndef ZDO_IEEEADDR_REQUEST
  #define ZDO_IEEEADDR_REQUEST
#endif
#ifndef ZDO_NWKADDR_REQUEST
  #define ZDO_NWKADDR_REQUEST
#endif
#ifndef ZDO_MATCH_REQUEST
  #define ZDO_MATCH_REQUEST
#endif
#ifndef ZDO_MGMT_BIND_REQUEST
//  #define ZDO_MGMT_BIND_REQUEST
#endif
#ifndef ZDO_MGMT_BIND_RESPONSE
  #define ZDO_MGMT_BIND_RESPONSE
#endif
#ifndef ZDO_MGMT_LQI_REQUEST
//  #define ZDO_MGMT_LQI_REQUEST
#endif
#ifndef ZDO_MGMT_LQI_RESPONSE
  #define ZDO_MGMT_LQI_RESPONSE
#endif
#ifndef ZDO_MGMT_LEAVE_RESPONSE
  #define ZDO_MGMT_LEAVE_RESPONSE
#endif
  //Enable identify
#ifndef ZCL_IDENTIFY
  // #warning Z3.0 devices require Identify cluster for "Finding and Binding" discovery process which is mandatory, unless the implementation is a dynamic device (endpoints and clusters defined at run time). If you are implementing a dynamic device or a non-Z3.0 compliant device, please disregard this warning message.
#endif
#ifndef ZDO_MGMT_PERMIT_JOIN_RESPONSE
  #define ZDO_MGMT_PERMIT_JOIN_RESPONSE
#endif



#if defined ( MT_ZDO_FUNC )
  // All of the ZDO functions are enabled for ZTool use.
  #define ZDO_NWKADDR_REQUEST
  #define ZDO_IEEEADDR_REQUEST
  #define ZDO_MATCH_REQUEST
  #define ZDO_NODEDESC_REQUEST
  #define ZDO_POWERDESC_REQUEST
  #define ZDO_SIMPLEDESC_REQUEST
  #define ZDO_ACTIVEEP_REQUEST

  #define ZDO_COMPLEXDESC_REQUEST
  #define ZDO_USERDESC_REQUEST
  #define ZDO_USERDESCSET_REQUEST
  #define ZDO_ENDDEVICEBIND_REQUEST
  #define ZDO_BIND_UNBIND_REQUEST
  #define ZDO_SERVERDISC_REQUEST
  #define ZDO_NETWORKSTART_REQUEST
  #define ZDO_MANUAL_JOIN

  #define ZDO_COMPLEXDESC_RESPONSE
  #define ZDO_USERDESC_RESPONSE
  #define ZDO_USERDESCSET_RESPONSE
  #define ZDO_SERVERDISC_RESPONSE
  #define ZDO_ENDDEVICE_ANNCE
  #define ZDO_ENDDEVICEBIND_RESPONSE

  #if defined ( MT_ZDO_MGMT )
    #define ZDO_MGMT_NWKDISC_REQUEST
    #define ZDO_MGMT_LQI_REQUEST
    #define ZDO_MGMT_RTG_REQUEST
    #define ZDO_MGMT_BIND_REQUEST
    #define ZDO_MGMT_LEAVE_REQUEST
    #define ZDO_MGMT_JOINDIRECT_REQUEST
    #define ZDO_MGMT_PERMIT_JOIN_REQUEST
    #define ZDO_MGMT_NWKUPDATE_REQUEST
    #define ZDO_MGMT_NWKDISC_RESPONSE
    #define ZDO_MGMT_LQI_RESPONSE
    #define ZDO_MGMT_RTG_RESPONSE
    #define ZDO_MGMT_BIND_RESPONSE
    #define ZDO_MGMT_LEAVE_RESPONSE
    #define ZDO_MGMT_JOINDIRECT_RESPONSE
    #define ZDO_MGMT_PERMIT_JOIN_RESPONSE
    #define ZDO_MGMT_NWKUPDATE_NOTIFY
  #endif

#elif defined ( ZDO_API_BASIC )

  // Normal operation and sample apps only use End Device Bind
  // and Match Request.

  #define ZDO_MATCH_REQUEST
  #define ZDO_ENDDEVICEBIND_REQUEST
  #define ZDO_BIND_UNBIND_REQUEST
  #define ZDO_ENDDEVICE_ANNCE
  #define ZDO_NWKADDR_REQUEST
  #define ZDO_IEEEADDR_REQUEST
  #define ZDO_BIND_UNBIND_RESPONSE
  #define ZDO_MGMT_PERMIT_JOIN_REQUEST
  #define ZDO_MGMT_PERMIT_JOIN_RESPONSE
  #define ZDO_MGMT_LEAVE_REQUEST
  #define ZDO_MGMT_LEAVE_RESPONSE
  #define ZDO_ENDDEVICEBIND_RESPONSE

#elif defined ( ZDO_API_ADVANCED )

  #define ZDO_NWKADDR_REQUEST
  #define ZDO_IEEEADDR_REQUEST
  #define ZDO_MATCH_REQUEST
  #define ZDO_NODEDESC_REQUEST
  #define ZDO_POWERDESC_REQUEST
  #define ZDO_SIMPLEDESC_REQUEST
  #define ZDO_ACTIVEEP_REQUEST

  #define ZDO_COMPLEXDESC_REQUEST
  #define ZDO_USERDESC_REQUEST
  #define ZDO_USERDESCSET_REQUEST
  #define ZDO_ENDDEVICEBIND_REQUEST
  #define ZDO_BIND_UNBIND_REQUEST
  #define ZDO_SERVERDISC_REQUEST
  #define ZDO_NETWORKSTART_REQUEST
  #define ZDO_MANUAL_JOIN

  #define ZDO_COMPLEXDESC_RESPONSE
  #define ZDO_USERDESC_RESPONSE
  #define ZDO_USERDESCSET_RESPONSE
  #define ZDO_SERVERDISC_RESPONSE
  #define ZDO_ENDDEVICE_ANNCE

  #define ZDO_MGMT_NWKDISC_REQUEST
  #define ZDO_MGMT_LQI_REQUEST
  #define ZDO_MGMT_RTG_REQUEST
  #define ZDO_MGMT_BIND_REQUEST
  #define ZDO_MGMT_LEAVE_REQUEST
  #define ZDO_MGMT_JOINDIRECT_REQUEST
  #define ZDO_MGMT_PERMIT_JOIN_REQUEST
  #define ZDO_MGMT_NWKUPDATE_REQUEST
  #define ZDO_MGMT_NWKDISC_RESPONSE
  #define ZDO_MGMT_LQI_RESPONSE
  #define ZDO_MGMT_RTG_RESPONSE
  #define ZDO_MGMT_BIND_RESPONSE
  #define ZDO_MGMT_LEAVE_RESPONSE
  #define ZDO_MGMT_JOINDIRECT_RESPONSE
  #define ZDO_MGMT_PERMIT_JOIN_RESPONSE
  #define ZDO_MGMT_NWKUPDATE_NOTIFY

#else // !MT_ZDO_FUNC && && !ZDO_API_BASIC && !ZDO_API_ADVANCED

  // Normal operation and sample apps only use End Device Bind
  // and Match Request.

  //#define ZDO_NWKADDR_REQUEST
  //#define ZDO_IEEEADDR_REQUEST
  #define ZDO_MATCH_REQUEST
  //#define ZDO_NODEDESC_REQUEST
  //#define ZDO_POWERDESC_REQUEST
  //#define ZDO_SIMPLEDESC_REQUEST
  //#define ZDO_ACTIVEEP_REQUEST
  //#define ZDO_COMPLEXDESC_REQUEST
  //#define ZDO_USERDESC_REQUEST
  //#define ZDO_USERDESCSET_REQUEST
  #define ZDO_ENDDEVICEBIND_REQUEST
  #define ZDO_BIND_UNBIND_REQUEST
  //#define ZDO_SERVERDISC_REQUEST
  //#define ZDO_NETWORKSTART_REQUEST
  //#define ZDO_MANUAL_JOIN

  //#define ZDO_BIND_UNBIND_RESPONSE
  //#define ZDO_COMPLEXDESC_RESPONSE
  //#define ZDO_USERDESC_RESPONSE
  //#define ZDO_USERDESCSET_RESPONSE
  //#define ZDO_SERVERDISC_RESPONSE
  #define ZDO_ENDDEVICE_ANNCE

  //#define ZDO_MGMT_NWKDISC_REQUEST
  //#define ZDO_MGMT_LQI_REQUEST
  //#define ZDO_MGMT_RTG_REQUEST
  //#define ZDO_MGMT_BIND_REQUEST
  //#define ZDO_MGMT_LEAVE_REQUEST
  //#define ZDO_MGMT_JOINDIRECT_REQUEST
  #define ZDO_MGMT_PERMIT_JOIN_REQUEST
  //#define ZDO_MGMT_NWKDISC_RESPONSE
  //#define ZDO_MGMT_LQI_RESPONSE
  //#define ZDO_MGMT_RTG_RESPONSE
  #define ZDO_MGMT_BIND_RESPONSE
  //#define ZDO_MGMT_LEAVE_RESPONSE
  //#define ZDO_MGMT_JOINDIRECT_RESPONSE
  #define ZDO_MGMT_PERMIT_JOIN_RESPONSE
  #define ZDO_ENDDEVICEBIND_RESPONSE

  // Binding needs this request to do a 64 to 16 bit conversion
#if !defined(ZDO_NWKADDR_REQUEST)
  #define ZDO_NWKADDR_REQUEST
#endif
#if !defined(ZDO_IEEEADDR_REQUEST)
  #define ZDO_IEEEADDR_REQUEST
#endif
  #define ZDO_BIND_UNBIND_RESPONSE

#endif  // !MT_ZDO_FUNC



/*********************************************************************
 * Constants
 */

#define MAX_BUFFER_SIZE		        	80

#if defined ( ZIGBEE_FRAGMENTATION )
  // The application/profile must fill this field out.
  #define MAX_TRANSFER_SIZE	        	160
#else
  #define MAX_TRANSFER_SIZE	        	80
#endif

#define MAX_ENDPOINTS	            	254

#define MAX_PARENT_ANNCE_CHILD		  10

// Node Description Bitfields
#define ZDOLOGICALTYPE_MASK		    	0x07
#define ZDOAPSFLAGS_MASK		      	0x07
#define ZDOFREQUENCYBANDS_MASK    	0x1F
#define ZDOAPSFLAGS_BITLEN	    		3

#define SIMPLE_DESC_DATA_SIZE				7
#define NODE_DESC_DATA_SIZE					10

// Simple Description Bitfields
#define ZDOENDPOINT_BITLEN		      5
#define ZDOENDPOINT_MASK		        0x1F
#define ZDOINTERFACE_MASK	      		0x07
#define ZDOAPPFLAGS_MASK	      		0x0F
#define ZDOAPPDEVVER_MASK		      	0x0F
#define ZDOAPPDEVVER_BITLEN		    	4

/*********************************************************************
 * Attributes
 */

extern NodeDescriptorFormat_t ZDO_Config_Node_Descriptor;
extern NodePowerDescriptorFormat_t ZDO_Config_Power_Descriptor;

/*********************************************************************
 * Function Prototypes
 */
extern void ZDConfig_InitDescriptors( void );
extern void ZDConfig_UpdateNodeDescriptor( void );
extern void ZDConfig_UpdatePowerDescriptor( void );



/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* ZDCONFIG_H */
