/******************************************************************************

 @file fh_api.h

 @brief TIMAC 2.0 API

 Group: WCS LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2016-2025, Texas Instruments Incorporated

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

 ******************************************************************************
 
 
 *****************************************************************************/

#ifndef FH_API_H
#define FH_API_H

/******************************************************************************
 Includes
 *****************************************************************************/

#include "mac_high_level.h"
#include "hal_types.h"

/*!
 @mainpage TIMAC 2.0 FH API

 Overview
 ============================
 This document describes the application programming interface for the FH module
 The API provides an interface to the management and data services for HMAC and LMAC

 Callback Functions
 ============================
 These functions must be implemented by the application and are used to pass
 events and data from the MAC to the application.


 API Interfaces
 ============================
 - FHAPI_reset()
 - FHAPI_start()
 - FHAPI_startBS()
 - FHAPI_sendData()
 - FHAPI_stopAsync()
 - FHAPI_getTxParams()
 - FHAPI_completeTxCb()
 - FHAPI_completeRxCb()
 - FHAPI_setStateCb()
 - FHAPI_SFDRxCb()

 <BR><BR>
 */

/******************************************************************************
 Constants and definitions
 *****************************************************************************/

/*! FH Flow Control Header IE bitmap  */
#define FH_WISUN_HIE_FC_IE_BITMAP       0x00000001
/*! FH Unicast Time Header IE bitmap  */
#define FH_WISUN_HIE_UT_IE_BITMAP       0x00000002
/*! FH RSL Header IE bitmap  */
#define FH_WISUN_HIE_RSL_IE_BITMAP      0x00000004
/*! FH Broadcast time Header IE bitmap  */
#define FH_WISUN_HIE_BT_IE_BITMAP       0x00000008
/*! FH EAPOL Authentication Header IE bitmap  */
#define FH_WISUN_HIE_EA_IE_BITMAP       0x00000010
/*! FH Unicast Schedule Payload IE bitmap  */
#define FH_WISUN_PIE_US_IE_BITMAP       0x00010000
/*! FH broadcast Schedule Payload IE bitmap  */
#define FH_WISUN_PIE_BS_IE_BITMAP       0x00020000
/*! FH Vendor Payload IE bitmap  */
#define FH_WISUN_PIE_VP_IE_BITMAP       0x00400000
/*! FH PAN Payload IE bitmap  */
#define FH_WISUN_PIE_PAN_IE_BITMAP      0x00040000
/*! FH Network name Payload IE bitmap  */
#define FH_WISUN_PIE_NETNAME_IE_BITMAP  0x00080000
/*! FH PAN Version Payload IE bitmap  */
#define FH_WISUN_PIE_PANVER_IE_BITMAP   0x00100000
/*! FH GTK Hash Payload IE bitmap  */
#define FH_WISUN_PIE_GTKHASH_IE_BITMAP  0x00200000

/*Num times to check if a packet is detected or not in a certain time */
#define FH_MAXPDB                                1 //2

/******************************************************************************
 Typedefs
 *****************************************************************************/

/*! FH module status types */
typedef enum __fhapi_status_
{
    /*! Success */
    FHAPI_STATUS_SUCCESS                       = 0x00,
    /*! FH general error */
    FHAPI_STATUS_ERR                           = 0x61,
    /*! IE is not supported in FH */
    FHAPI_STATUS_ERR_NOT_SUPPORTED_IE          = 0x62,
    /*! There is no ASYNC message in MAC TX queue */
    FHAPI_STATUS_ERR_NOT_IN_ASYNC              = 0x63,
    /*! destination address is not in FH neighbor table */
    FHAPI_STATUS_ERR_NO_ENTRY_IN_THE_NEIGHBOR  = 0x64,
    /*! fh is not in UC or BC dwell time slot */
    FHAPI_STATUS_ERR_OUT_SLOT                  = 0x65,
    /*! address is invalid */
    FHAPI_STATUS_ERR_INVALID_ADDRESS           = 0x66,
    /*! IE format is wrong */
    FHAPI_STATUS_ERR_INVALID_FORMAT            = 0x67,
    /*! PIB is not supported in FH module */
    FHAPI_STATUS_ERR_NOT_SUPPORTED_PIB         = 0x68,
    /* PIB is read only in FH module */
    FHAPI_STATUS_ERR_READ_ONLY_PIB             = 0x69,
    /*! parameter is invalid in FH PIB API */
    FHAPI_STATUS_ERR_INVALID_PARAM_PIB         = 0x6A,
    /*! invalid FH frame type */
    FHAPI_STATUS_ERR_INVALID_FRAME_TYPE        = 0x6B,
    /*! expired FH node */
    FHAPI_STATUS_ERR_EXPIRED_NODE              = 0x6C,
    /*! patch is not needed */
    FHAPI_STATUS_ERR_NO_PATCH                  = 0x6D,
    /*! No Response Frame */
    FHAPI_STATUS_ERR_EDFE_NO_RFRM              = 0x70,
    /*! DFE Received */
    FHAPI_STATUS_ERR_EDFE_DFE_RCV              = 0x71,
    /*! CCA Failure */
    FHAPI_STATUS_ERR_EDFE_CCA_FAIL             = 0x72,
    /*! last status value of FH module */
    FHAPI_STATUS_ERR_NT_FULL                   = 0x73,

    FHAPI_STATUS_MAX
} FHAPI_status;

/*! FH module IE type */
typedef enum __ie_type
{
    /*! Flow Control IE */
    FH_IE_TYPE_FC_IE        = 0x3,
    /*! Unicast Time IE */
    FH_IE_TYPE_UT_IE        = 0x1,
    /*! RSL IE */
    FH_IE_TYPE_RSL_IE       = 0x4,
    /*! Broadcast Time IE */
    FH_IE_TYPE_BT_IE        = 0x2,
    /*! Unicast Schedule IE */
    FH_IE_TYPE_US_IE        = 0x1,
    /*! Broadcast Schedule IE */
    FH_IE_TYPE_BS_IE        = 0x2,
    /*! PAN IE */
    FH_IE_TYPE_PAN_IE       = 0x4,
    /*! Network Name IE */
    FH_IE_TYPE_NETNAME_IE   = 0x5,
    /*! PAN Version IE */
    FH_IE_TYPE_PANVER_IE    = 0x6,
    /*! GTK Hash IE */
    FH_IE_TYPE_GTKHASH_IE   = 0x7
} IE_TYPE_t;

/*! LMAC callback status */
typedef enum __lmac_status
{
    /*! LMAC TX Start */
    LMAC_TX_START               = 0,
    /*! LMAC received SFD */
    LMAC_RX_SFD                 = 1,
    /*! LMAC TX CCA Busy */
    LMAC_CCA_BUSY               = 2,
    /*! LMAC TX RSSI Busy */
    LMAC_RSSI_BUSY              = 3,
    /*! LMAC TX Push to Queue because LMAC in RX */
    LMAC_TX_PUSH_TO_QUEUE       = 4


} FH_LMAC_STATUS_t;

/******************************************************************************
 Global Externs
 *****************************************************************************/

/******************************************************************************
 Prototype
 *****************************************************************************/


/*!
 * @brief       resets timers and data structure of the FH module. This API must be
 *              called by all other APIs
 */
extern MAC_INTERNAL_API void FHAPI_reset(void);


/*!
 * @brief       starts the unicast timer and if coordinator broadcast timer as well
 */
extern MAC_INTERNAL_API void FHAPI_start(void);

/*!
 * @brief       starts the broadcast timer once a node joins to the network
 */
extern MAC_INTERNAL_API void FHAPI_startBS(void);


/*!
 * @brief       This function is called when a packet is to be transmitted using frequency
 *              hopping module. It will extract TX packets from MAC TX queue based on FH type
 *              (ASYNC, Broadcast or unicast). For ASYNC packet, it will call macTxFrame without
 *              CSMA/CA. All other type packets will be sent in CSMA/CA fashion
 */

extern MAC_INTERNAL_API void FHAPI_sendData(void);


/*!
 * @brief       stop an Async process already in place. If the FH module is not in the ASYNC
 *              mode, This function will check the MAC TX queue to see if there are any ASYNC
 *              request start messages. if there is any ASYNC start request message, it will be
 *              purged from MAC TX queue. if the FH is in the middle of ASYNC operation, it will
 *              wait to current PHY TX complete and stop ASYNC operation.
 *
 * @return      The status of the operation, as follows:<BR>
 *              [FHAPI_STATUS_SUCCESS](@ref FHAPI_STATUS_SUCCESS)
 *               - Operation successful<BR>
 *              [FHAPI_STATUS_ERR_NOT_IN_ASYNC] (@ref FHAPI_STATUS_ERR_NOT_IN_ASYNC)
 *               - there is no ASYNC request message in the MAC TX queue<BR>
 */
extern MAC_INTERNAL_API FHAPI_status FHAPI_stopAsync(void);

/*!
 * @brief       get TX channel and adjusted backOffDur. After this function call, pbackOffDur will
 *              be adjusted and fall within a slot
 *
 * @param pBackOffDur - pointer to the back off duration
 * @param chIdx - pointer to the computed TX channel.
 *
 * @return      The status of the operation, as follows:<BR>
 *              [FHAPI_STATUS_SUCCESS](@ref FHAPI_STATUS_SUCCESS)
 *               - Operation successful<BR>
 *              [FHAPI_STATUS_ERR_OUT_SLOT] (@ref FHAPI_STATUS_ERR_OUT_SLOT)
 *               - the back off will be out of slot (unicast or broadcast)<BR>
 *              [FHAPI_STATUS_ERR_NO_ENTRY_IN_THE_NEIGHBOR] (@ref FHAPI_STATUS_ERR_NO_ENTRY_IN_THE_NEIGHBOR)
 *               - the destination address is not in the FH neighbor table<BR>
 *              [FHAPI_STATUS_ERR_INVALID_FRAME_TYPE] (@ref FHAPI_STATUS_ERR_INVALID_FRAME_TYPE)
 *               - frame type in TX packet is invalid<BR>
 */
extern MAC_INTERNAL_API FHAPI_status FHAPI_getTxParams_old(uint32_t *pBackOffDur, uint8_t *chIdx);
extern MAC_INTERNAL_API FHAPI_status FHAPI_getTxParams(uint32_t *pBackOffDur, uint8_t *chIdx);
/*!
 * @brief       callback from LMAC to indicate transmission status
 *
 * @param status - status of PHY TX transmission status<BR>
 *                  [MAC_NO_TIME]
    *                - there is no time to transmit this packet<BR>
    *               [ other value]
    *                - LMAC TX is success<BR>
 */
extern MAC_INTERNAL_API void FHAPI_completeTxCb(uint8_t status);

/*!
 * @brief       callback from LMAC to indicate packet is received sucessfully
 *
 * @param pMsg - pointer of received packet<BR>
 *
 */
extern MAC_INTERNAL_API void FHAPI_completeRxCb(macRx_t *pMsg);

/*!
 * @brief       callback from LMAC to indicate radio state
 *
 * @param state - state of LMAC radio state <BR>
 *               [LMAC_TX_START]
 *               - LMAC TX is started <BR>
 *               [LMAC_RX_SFD]
 *               - LMAC detected SFD<BR>
 *               [LMAC_CCA_BUSY]
 *               - LMAC CCA busy<BR>
 *
 */
extern MAC_INTERNAL_API void FHAPI_setStateCb(FH_LMAC_STATUS_t state);


/*!
 * @brief       callback from LMAC to indicate SFD is detected
 *
 * @param status - the status of SDF trigger. The FH module will start the RX logic when
 *          	MAC_SFD_DETECTED status is received and terminate the RX logic
 *          	when a non-zero
 *               [MAC_SFD_DETECTED]
 *               - SFD is detected <BR>
 *               [MAC_SFD_FRAME_RECEIVED]
 *               - MAC RX packet is received successfully<BR>
 *               [MAC_SFD_FRAME_DISCARDED]
 *               - MAC frame is discarded<BR>
 *               [MAC_SFD_HALT_CLEANED_UP]
 *               - MAC frame is cleanup by HAL<BR>
 *               [MAC_SFD_CRC_ERROR]
 *               - received MAC framehas CRC error<BR>
 *
 */
extern MAC_INTERNAL_API void FHAPI_SFDRxCb(uint8 status);


/*!
 * @brief       get channel hopping function of the node transmitting to
 *
 * @return      TX channel is hopping or fixed
 */
extern MAC_INTERNAL_API uint8_t FHAPI_getTxChHoppingFunc(void);


/*!
 * @brief       get remaining time the node transmitting to will be on current slot
 *
 * @return      reamining dwell time
 */
extern MAC_INTERNAL_API uint32_t FHAPI_getRemDT(void);


/*!
 * Computes the number of ones in a exclude channel bit accounting
 *              for max possible channels
 *
 */
extern MAC_INTERNAL_API uint8_t FHAPI_getBitCount(void);
/**************************************************************************************************
 */

extern uint16_t FH_GetVpieLenInTxEvent(void);

extern uint8_t * FH_GetVpieDataInTxEvent(void);

extern uint8_t FHAPI_IsAsyncState(void);
#endif
