/**************************************************************************************************
  Filename:       gpd_config.h
  Revised:        $Date: 2011-05-05 13:52:19 -0700 (Thu, 05 May 2011) $
  Revision:       $Revision: 25878 $

  Description:    This file contains the configuration parameters for a Green Power Device.


  Copyright 2005-2011 Texas Instruments Incorporated.

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

#ifndef GPD_CONFIG_H
#define GPD_CONFIG_H

#ifdef __cplusplus
extern "C"
{
#endif

/******************************************************************************
 * INCLUDES
 */
#include "gpd.h"
#include "ti_zstack_config.h"

/*
 * GPD CONFIGURATION
 */
#define TC_LINK_KEY          {0x5A, 0x69, 0x67, 0x42, 0x65, 0x65, 0x41, 0x6C, 0x6C, 0x69, 0x61, 0x6E, 0x63, 0x65, 0x30, 0x39}

/*
 * GPDF SETTINGS
 */
#if ( (GP_SECURITY_LEVEL == GP_SECURITY_LVL_4FC_4MIC) || (GP_SECURITY_LEVEL == GP_SECURITY_LVL_4FC_4MIC_ENCRYPT) || (GPD_APP_ID == GPD_APP_TYPE_IEEE_ID) )
#define FRAME_CTRL_EXT             TRUE //Default = TRUE
#elif (GP_SECURITY_LEVEL == GP_SECURITY_LVL_NO_SEC)
#define FRAME_CTRL_EXT             FALSE
#endif

/* GPDF OPTIONS */
#define GPD_FRAME_CONTROL        (uint8_t)(((GP_DATA_FRAME)&0x03) | (((GP_NWK_PROTOCOL_VERSION)&0x0F)<<2) | (((AUTO_COMMISSIONING)&0x01)<<6)   | (((FRAME_CTRL_EXT)&0x01)<<7))

/* GPDF EXTENDED OPTIONS */
#if ( (EXT_OPT_KEY_TYPE == KEY_TYPE_NO_KEY ) || (EXT_OPT_KEY_TYPE == KEY_TYPE_ZIGBEE_NWK_KEY ) || (EXT_OPT_KEY_TYPE == KEY_TYPE_GPD_GROUP_KEY ) || (EXT_OPT_KEY_TYPE == KEY_TYPE_NWK_KEY_DERIVED_GPD_GROUP_KEY ) )
  #define EXT_OPT_SECURITY_KEY  0
#elif( (EXT_OPT_KEY_TYPE == KEY_TYPE_OUT_BOX_GPD_KEY ) || (EXT_OPT_KEY_TYPE == KEY_TYPE_DERIVED_INDIVIDUAL_GPD_KEY ) )
  #define EXT_OPT_SECURITY_KEY  1
#endif

#define GPD_EXT_FRAME_CONTROL                 (uint8_t)(((GPD_APP_ID)&0x07) | (((GP_SECURITY_LEVEL)&0x03)<<3) | (((EXT_OPT_SECURITY_KEY)&0x01)<<5) | (((RX_AFTER_TX)&0x01)<<6))
#define GPD_UNSECURED_EXT_FRAME_CONTROL       (uint8_t)(((GPD_APP_ID)&0x07) | (((GP_SECURITY_LVL_NO_SEC)&0x03)<<3) | (((EXT_OPT_SECURITY_KEY)&0x01)<<5) | (((RX_AFTER_TX)&0x01)<<6))

#ifdef __cplusplus
}
#endif

#endif /* GPD_CONFIG_H */
