/******************************************************************************

 @file fh_util.h

 @brief TIMAC 2.0 FH utility API

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

/******************************************************************************
 Includes
 *****************************************************************************/

#ifndef FH_UTIL_H
#define FH_UTIL_H

/* ------------------------------------------------------------------------------------------------
 *                                            Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "hal_types.h"
#include "fh_pib.h"
#include "fh_nt.h"
#include "mac_low_level.h"

 /* ------------------------------------------------------------------------------------------------
 *                                            Defines
 * ------------------------------------------------------------------------------------------------
 */


/* ------------------------------------------------------------------------------------------------
 *                                           Typedefs
 * ------------------------------------------------------------------------------------------------
 */



/* ------------------------------------------------------------------------------------------------
 *                                           Global Externs
 * ------------------------------------------------------------------------------------------------
 */



/* ------------------------------------------------------------------------------------------------
 *                                           Prototypes
 * ------------------------------------------------------------------------------------------------
 */

/*!
 * @brief       This function computes the backoff duration to be used for
 *              a broadcast transmission
 *
 * @param       pBackOffDur - pointer to current value of backoff duration
 * @param       pChIdx - The pointer to current chosen channel index
 *
 * @return      The status of the operation, as follows:<BR>
 *              [FHAPI_STATUS_SUCCESS](@ref FHAPI_STATUS_SUCCESS)
 *               - Operation successful<BR>
 *              [FHAPI_STATUS_ERR_OUT_SLOT] (@ref FHAPI_STATUS_ERR_OUT_SLOT)
 *               - the back off will be out of slot (unicast or broadcast)<BR>
 */
FHAPI_status FHUTIL_getBcTxParams(uint32_t *pBackOffDur, uint8_t *pChIdx);

/*!
 * @brief       Calcultaes the tx channel for unicast transmission
 *              at a given offset
 *
 * @param       pEntry - Pointer to neighbor table entry
 * @param       txUfsi - The current ufsi value for the neighbor
 * @param       offset - the desired offset after which the frame is to
 *              be transmitted
 *
 * @return      uint8_t - computed tx channel
 */
uint8_t FHUTIL_getTxChannel(NODE_ENTRY_s *pEntry, uint32_t txUfsi, uint32_t offset);

/*!
 * @brief       This function is called by low level to update backoffDuration
 *
 * @param       pEntry - Pointer to neighbor table entry
 * @param       pBackOffDur - The current value of chosen backoff duration.
 * @param       txUfsi - The current ufsi value for the neighbor
 */
FHAPI_status FHUTIL_adjBackOffDur(NODE_ENTRY_s *pEntry, uint32_t *pBackoffDur, uint32_t *txUfsi);

/*!
 * @brief       Asserts the exection based on error conditions
 *
 * @param       pData - pointer to data with error information
 */
void FHUTIL_assert(void *pData);

/*!
 * @brief       Performans no action. useful for state transisions with no special
 *              handling requirements
 *
 * @param       pData - pointer to data with relevant information
 */
void FHUTIL_noAction(void *pData);

/*!
 * @brief       This function gets the current value of broadcast fractional
 *              interval offset
 *
 * @param       bfio - pointer to bfio value
 * @param       slotIdx - pointer to value of brodcast slot id
 *
 * @return      The status of the operation, as follows:<BR>
 *              [FHAPI_STATUS_SUCCESS](@ref FHAPI_STATUS_SUCCESS)
 *               - Operation successful<BR>
 */
FHAPI_status FHUTIL_getCurBfio(uint32_t *bfio, uint16_t *slotIdx);

/*!
 * @brief       This function gets the current value of the ufsi
 *
 * @param       ufsi - pointer to unicast fractional offset index
 *
 * @return      The status of the operation, as follows:<BR>
 *              [FHAPI_STATUS_SUCCESS](@ref FHAPI_STATUS_SUCCESS)
 *               - Operation successful<BR>
 */
FHAPI_status FHUTIL_getCurUfsi(uint32_t *ufsi);

/*!
 * @brief       Calculates UFSI based on ufsi and dwell time
 *
 * @param       ufsi - unicast fractional slot interval
 * @param       ucDwellTime - unicast dwell time
 *
 * @return      uint32_t computed ufsi value
 */
uint32_t FHUTIL_calcUfsi(uint32_t ufsi, uint8_t ucDwellTime);

/*!
 * @brief       update btie based on bfio and slotIdx
 *
 * @param       bfio - new bfio value
 * @param       slotIdx - new broadcast slot index *
 *
 * @return      The status of the operation, as follows:<BR>
 *              [0]- Operation successful<BR>
 *              [1]- Operation failure<BR>
 */
uint8_t FHUTIl_updateBTIE(uint32_t bfio, uint16_t slotIdx);

/*!
 * @brief       Compute the physical channel cooresponding to 0 index based on
 *              PHY configurations
 *
 * @return      uint32_t - The physical channel value
 */
uint32_t FHUTIL_getCh0(void);

/*!
 * @brief       Compute the channel spacing to be used based on configuration
 *
 * @return      uint8_t - channel spacing value
 */
uint8_t FHUTIL_getChannelSpacing(void);

/*!
 * @brief       Compute the maximum number of channels based on configuration
 *
 * @return      uint8_t - total number of channels
 */
uint8_t FHUTIL_getMaxChannels(void);

/*!
 * @brief       Computes the number of ones in a exclude channel bit accounting
 *              for max possible channels
 *
 *@param        bitmap - the exclude channel bitmap
 *@param        maxChannels - maximum possible number of channels
 *
 * @return      uint8_t - total number of ones in bitmap
 */
uint8_t FHUTIL_getBitCounts(uint8_t *bitmap, uint8_t maxChannels);

/*!
 * @brief       Updates the total number of unicast channels based on
 *              exclude channel bitmap
 *
 *@param        bitmap - the exclude channel bitmap
 */
void FHUTIL_updateUcNumChannels(uint8_t *bitmap);

/*!
 * @brief       Updates the total number of broadcast channels based on
 *              exclude channel bitmap
 *
 *@param        bitmap - the exclude channel bitmap
 */
void FHUTIL_updateBcNumChannels(uint8_t *bitmap);

/*!
 * @brief       Computes the dynamic guardtime to be applied ffor a neighbor
 *
 * @param       pEntry - pointer to neighbor table entry
 *
 * @return      uint32_t the computed guard time to be applied
 */
uint32_t FHUTIL_getGuardTime(NODE_ENTRY_s *pEntry);

/*!
 * @brief       Computes the elapsed time since last known RTC time stamp
 *
 * @param       prevTime - time in tickts from which the elapsed time is to be
 *              computed
 * @param       slotIdx - pointer to value of brodcast slot id
 *
 * @return      uint32_t the elapsed time value in clock tick periods
 */
uint32_t FHUTIL_getElapsedTime(uint32_t prevTime);

/*!
 * @brief       Computes the elapsed time between two given rtc time stamps
 *
 * @param       curTime - the RTC time stamp to which elapsed time is to
 *              be computed
 * @param       oldTIme - the RTC time stamp from which elapsed time is to
 *              be computed
 *
 * @return      uint32_t the computed guard time to be applied
 */
uint32_t FHUTIL_elapsedTime(uint32_t curTime, uint32_t oldTime);
/*!
 * @brief       Computes the target node's ufsi
 *
 * @param       pEntry - neighbor table entry containing target's info.
 *
 * @return      uint32_t the computed txUfsi
 */
uint32_t FHUTIL_getTxUfsi(NODE_ENTRY_s *pEntry);

#if 1 //def FH_PRINT_DEBUG
/*!
 * @brief       Prints TX debug information.
 *
 */
void FHUTIL_printTxDebugInfo(void);

/*!
 * @brief       Prints CSMA debug information.
 *
 * @param       backOffDur - backoff duration
 * @param       maxTxCSMABackOddDelay - maximum allowed backoff duration
 */
void FHUTIL_printCsmaDebugInfo(uint32_t backOffDur, uint8_t macTxCsmaBackoffDelay);
/*!
 * @brief       Prints RX debug information.
 */
void FH_printDebugInfo(void);
#endif
#endif
