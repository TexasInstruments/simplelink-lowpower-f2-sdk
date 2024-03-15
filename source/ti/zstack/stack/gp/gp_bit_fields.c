/**************************************************************************************************
  Filename:       gp_bitFields.c
  Revised:        $Date: 2018-09-13 11:51:49 -0700 (Thu, 13 Feb 2018) $
  Revision:       $Revision: - $

  Description:    This file contains the Green Power bit fields deffinitions.


  Copyright 2006-2015 Texas Instruments Incorporated.

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
#include "string.h"
#include "zglobals.h"
#include "gp_bit_fields.h"

 #if !defined (DISABLE_GREENPOWER_BASIC_PROXY) && (ZG_BUILD_RTR_TYPE)

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

 /*********************************************************************
 * LOCAL VARIABLES
 */

 /*********************************************************************
 * LOCAL FUNCTIONS
 */


/*********************************************************************
 * @fn      gp_bfParse_commissioningNotificationOptions
 *
 * @brief   Converts Commissioning Notification options field from
 *          uint16_t to gpCommissioningNotificationOptions_t struct
 *
 * @param   options - pointer to commissioning notification options struct.
 * @param   bitField - Green Power commissioning notification bitfield.
 *
 * @return  ZMemError if not able to allocate
 */
void gp_bfParse_commissioningNotificationOptions( gpCommissioningNotificationOptions_t* pOptions, uint16_t bitField )
{
    pOptions->appId = GP_GET_APPLICATION_ID(LO_UINT16(bitField));
    pOptions->rxAfterTx = GP_GET_RX_AFTER_TX(LO_UINT16(bitField));
    pOptions->securityLevel = GP_CNTF_GET_SEC_LEVEL(LO_UINT16(bitField));
    pOptions->securityKeyType = GP_CNTF_GET_SEC_KEY_TYPE(bitField);
    pOptions->securityProcessingFailed = GP_CNTF_GET_SEC_FAIL(HI_UINT16(bitField));
    pOptions->bidirectionalCapability = GP_CNTF_GET_BIDIRECTIONAL_CAP(HI_UINT16(bitField));
    pOptions->proxyInfoPresent = GP_CNTF_GET_PROXY_INFO(HI_UINT16(bitField));
}

#endif  // (DISABLE_GREENPOWER_BASIC_PROXY) && (ZG_BUILD_RTR_TYPE)
