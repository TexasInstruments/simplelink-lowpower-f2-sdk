/******************************************************************************

 @file timac_ns_interface.h

 @brief TIMAC 2.0 Middleware Example Application Header

 Group: WCS LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2016-2025, Texas Instruments Incorporated
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions
 are met:

 *  Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.

 *  Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.

 *  Neither the name of Texas Instruments Incorporated nor the names of
    its contributors may be used to endorse or promote products derived
    from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 ******************************************************************************
 
 
 *****************************************************************************/
#ifndef TIMAC_NS_INTERFACE_H
#define TIMAC_NS_INTERFACE_H

/******************************************************************************
 Includes
 *****************************************************************************/

//#include "smsgs.h"
#include "osal_port.h"
#include "stdbool.h"

#include "ns_types.h"
#include "mac_mcps.h"
#include "api_mac.h"
#include "timac_api.h"
#include "timac_api.h"

#ifdef __cplusplus
extern "C"
{
#endif

/******************************************************************************
 Constants and definitions
 *****************************************************************************/

/******************************************************************************
 Structures
 *****************************************************************************/

/******************************************************************************
 Global Variables
 *****************************************************************************/


/******************************************************************************
 Function Prototypes
 *****************************************************************************/
extern void timacConvertDataReq(mcps_data_req_t *mdataReq, ApiMac_mcpsDataReq_t *tdataReq);
extern void timacConvertDataCnf(mcps_data_conf_t *mdataCnf, macMcpsDataCnf_t *tdataCnf);
extern void timacConvertDataInd(mcps_data_ind_t *mdataInd, macMcpsDataInd_t *tdataInd);
extern void timacConvertIncludeIes(wh_ie_sub_list_t *ie_header_mask, wp_nested_ie_sub_list_t *nested_wp_id, ApiMac_mcpsDataReq_t *tdataReq, bool mpxIe);
/*!
 * @brief Initialize this application.
 */
void timac_initialize(struct mac_api_s *api);

#ifdef __cplusplus
}
#endif

#endif /* TIMAC_NS_INTERFACE_H */
