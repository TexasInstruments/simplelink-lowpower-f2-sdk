/******************************************************************************

 @file  mt_rcp_lmac.h

 @brief Monitor/Test command/response definitions for RCP MAC subsystem

 Group: Connectivity
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2025-2026, Texas Instruments Incorporated
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
#ifndef MTRCPLMAC_H
#define MTRCPLMAC_H

/******************************************************************************
 Includes
 *****************************************************************************/
#include "api_mac.h"

#include "mt.h"

#include "rcp_types.h"
#include "rcp_lmac.h"

#ifdef __cplusplus
extern "C"
{
#endif

/******************************************************************************
 Global Variables
 *****************************************************************************/

extern MT_rcp_LMAC_dbg_t MT_rcp_LMAC_dbg;


/******************************************************************************
 Public Function Prototypes
 *****************************************************************************/

extern uint8_t MtRcpLMac_commandProcessing(Mt_mpb_t *pMpb);

extern void MtRcpLMac_RcpInitCnf(rcp_init_cnf_t *pCnfRcpInit);

extern void MtRcpLMac_MacCfgGetCnf(rcp_mac_config_get_cnf_t *pCnfMacCfgGet);

extern void MtRcpLMac_MacCfgSetCnf(rcp_mac_config_set_cnf_t *pCnfMacCfgSet, uint8_t mt_cmd1);

extern void MtRcpLMac_DataInd(rcp_data_ind_t *pInd);

extern void MtRcpLMac_DataCnf(rcp_data_cnf_t *pCnf);


#ifdef __cplusplus
}
#endif

#endif /* MTRCPLMAC_H */
