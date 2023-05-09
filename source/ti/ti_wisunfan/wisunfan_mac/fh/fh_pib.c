/******************************************************************************

 @file fh_pib.c

 @brief TIMAC 2.0 FH PIB API

 Group: WCS LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2016-2023, Texas Instruments Incorporated

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

#ifndef LIBRARY
#include "ti_wisunfan_features.h"
#endif

#include <stddef.h>
#include "fh_api.h"
#include "fh_pib.h"
#include "fh_util.h"
#include "mac_mgmt.h"

#ifdef FEATURE_FREQ_HOP_MODE

/******************************************************************************
  ROM jump table will support
  1. TIMAC ROM image build
  2. TIMAC stack build without ROM image
  3. TIMAC stack with calling ROM image
 *****************************************************************************/
#include "rom_jt_154.h"

/******************************************************************************
 Constants and definitions
 *****************************************************************************/

/******************************************************************************
 Local variables
 *****************************************************************************/

/******************************************************************************
 Local Function Prototypes
 *****************************************************************************/

uint16_t FHPIB_getIndex(uint16_t fhPibId)
{
    if((fhPibId >= FH_PIB_ID_START) && (fhPibId <= FH_PIB_ID_END))
    {
        return(fhPibId - FH_PIB_ID_START + FH_PIB_ID_OFFSET);
    }
    else
    {
        return(FH_PIB_ID_INVALID);
    }
}

/******************************************************************************
 Public Functions
 *****************************************************************************/
#if !defined(TIMAC_ROM_PATCH)
/*!
 FHPIB_reset

 Public function defined in fh_pib.h
 */
MAC_INTERNAL_API void FHPIB_reset(void)
{
    /* copy FH-related PIB defaults */
    //FHPIB_db = FHPIB_defaults;
    MAP_osal_memcpy((void *)&FHPIB_db, (const void *)&FHPIB_defaults, sizeof(FHPIB_db));
}
#endif

/*!
 FHPIB_getLen

 Public function defined in fh_pib.h
 */
MAC_INTERNAL_API uint8_t FHPIB_getLen(uint16_t fhPibId)
{
    uint16_t i;

    if((i = MAP_FHPIB_getIndex(fhPibId)) == FH_PIB_ID_INVALID)
    {
        return(0);
    }

    return (FH_PibTbl[i].len);
}

/*!
 FHPIB_set

 Public function defined in fh_pib.h
 */
MAC_INTERNAL_API FHAPI_status FHPIB_set(uint16_t fhPibId, void *pData)
{
    uint16_t i;
    uint8_t pibLen;
    uint8_t numChannels, maxChannels, excludedChannels;
    halIntState_t intState;

    if((i = MAP_FHPIB_getIndex(fhPibId)) == FH_PIB_ID_INVALID)
    {
        return(FHAPI_STATUS_ERR_NOT_SUPPORTED_PIB);
    }

    /* do range check; no range check if min and max are zero */
    if ((FH_PibTbl[i].min != 0) || (FH_PibTbl[i].max != 0))
    {
        /* if min == max, this is a read-only attribute */
        if (FH_PibTbl[i].min == FH_PibTbl[i].max)
        {
            return(FHAPI_STATUS_ERR_READ_ONLY_PIB);
        }

        /* range check for general case */
        pibLen = FH_PibTbl[i].len;
        switch(pibLen)
        {
            case FH_UINT8_SIZE:
                if ((*((uint8_t *) pData) < FH_PibTbl[i].min)
                 || (*((uint8_t *) pData) > FH_PibTbl[i].max))
                {
                    return(FHAPI_STATUS_ERR_INVALID_PARAM_PIB);
                }
                break;
            case FH_UINT16_SIZE:
                if ((*((uint16_t *) pData) < FH_PibTbl[i].min)
                 || (*((uint16_t *) pData) > FH_PibTbl[i].max))
                {
                    return(FHAPI_STATUS_ERR_INVALID_PARAM_PIB);
                }
                break;
            case FH_UINT32_SIZE:
                if ((*((uint32_t *) pData) < FH_PibTbl[i].min)
                 || (*((uint32_t *) pData) > FH_PibTbl[i].max))
                {
                    return(FHAPI_STATUS_ERR_INVALID_PARAM_PIB);
                }
                break;
            default:
                break;
        }
    }

    HAL_ENTER_CRITICAL_SECTION(intState);
    switch(fhPibId)
    {
        case FHPIB_NET_NAME:
            MAP_osal_memset((uint8_t *)&FHPIB_db + FH_PibTbl[i].offset, 0,
                    FH_PibTbl[i].len);
            MAP_osal_memcpy((uint8_t *)&FHPIB_db + FH_PibTbl[i].offset, pData,
                    MAP_osal_strlen((char *)pData) < FH_PibTbl[i].len ?
                    MAP_osal_strlen((char *)pData) : FH_PibTbl[i].len);
            break;
        case FHPIB_BC_FIXED_CHANNEL:
        case FHPIB_UC_FIXED_CHANNEL:
            maxChannels = MAP_FHUTIL_getMaxChannels();
            if (*((uint16_t *) pData) < maxChannels)
            {
                MAP_osal_memcpy((uint8_t *)&FHPIB_db + FH_PibTbl[i].offset, pData,
                        FH_PibTbl[i].len);
            }
            else
            {
                HAL_EXIT_CRITICAL_SECTION(intState);
                return FHAPI_STATUS_ERR_INVALID_PARAM_PIB;
            }
            break;
        case FHPIB_BC_EXCLUDED_CHANNELS:
        case FHPIB_UC_EXCLUDED_CHANNELS:
            maxChannels = MAP_FHUTIL_getMaxChannels();
            excludedChannels = MAP_FHUTIL_getBitCounts((uint8_t *)pData, maxChannels);
            numChannels = maxChannels - excludedChannels;
            if(numChannels > 0)
            {
                MAP_osal_memcpy((uint8_t *)&FHPIB_db + FH_PibTbl[i].offset, pData,
                        FH_PibTbl[i].len);
            }
            else
            {
                HAL_EXIT_CRITICAL_SECTION(intState);
                return FHAPI_STATUS_ERR_INVALID_PARAM_PIB;
            }
            if(fhPibId == FHPIB_UC_EXCLUDED_CHANNELS)
            {
                FH_hnd.ucNumChannels = numChannels;
            }
            else if(fhPibId == FHPIB_BC_EXCLUDED_CHANNELS)
            {
                FH_hnd.bcNumChannels = numChannels;
            }
            break;
#ifdef FEATURE_WISUN_SUPPORT
        case FHPIB_ROUTING_COST:
            MAP_osal_memcpy((uint8_t *)&FHPIB_db_new.macRoutingCost , pData,
                                sizeof(uint16_t));
            break;
#endif
        default:
            MAP_osal_memcpy((uint8_t *)&FHPIB_db + FH_PibTbl[i].offset, pData,
                    FH_PibTbl[i].len);
            break;
    }
    /* sanity checking for number of non-sllep and leep node */
    if (fhPibId == FHPIB_NUM_MAX_NON_SLEEP_NODES)
    {
        /* reset the sleep node to zero */
        FHPIB_db.macMaxSleepNodes = 0;
    }

    if (fhPibId == FHPIB_NUM_MAX_SLEEP_NODES)
    {
        /* sanity checking to make sure total number is not greater than MAX device
         * table size
         */
        if (  ((FHPIB_db.macMaxNonSleepNodes + FHPIB_db.macMaxSleepNodes) > FHPIB_MAX_NUM_DEVICE ) ||
              ((FHPIB_db.macMaxNonSleepNodes == 0 ) && (FHPIB_db.macMaxSleepNodes == 0) ) )
        {
            /* back to default */
            FHPIB_db.macMaxNonSleepNodes = FHPIB_defaults.macMaxNonSleepNodes;
            FHPIB_db.macMaxSleepNodes = FHPIB_defaults.macMaxSleepNodes;
            HAL_EXIT_CRITICAL_SECTION(intState);
            return(FHAPI_STATUS_ERR_INVALID_PARAM_PIB);
        }
    }
    HAL_EXIT_CRITICAL_SECTION(intState);
    return(FHAPI_STATUS_SUCCESS);
}

/*!
 FHPIB_get

 Public function defined in fh_pib.h
 */
MAC_INTERNAL_API FHAPI_status FHPIB_get(uint16_t fhPibId, void *pData)
{
    uint16_t i;
    halIntState_t intState;

    if((i = MAP_FHPIB_getIndex(fhPibId)) == FH_PIB_ID_INVALID)
    {
        return(FHAPI_STATUS_ERR_NOT_SUPPORTED_PIB);
    }

    HAL_ENTER_CRITICAL_SECTION(intState);
#ifdef FEATURE_WISUN_SUPPORT
    if( fhPibId == FHPIB_ROUTING_COST )
    {
        MAP_osal_memcpy(pData, (uint8_t *)&FHPIB_db_new.macRoutingCost,
                        sizeof(uint16_t));
        HAL_EXIT_CRITICAL_SECTION(intState);
        return(FHAPI_STATUS_SUCCESS);
    }
#endif
    MAP_osal_memcpy(pData, (uint8_t *)&FHPIB_db + FH_PibTbl[i].offset,
            FH_PibTbl[i].len);

    HAL_EXIT_CRITICAL_SECTION(intState);
    return(FHAPI_STATUS_SUCCESS);
}
#else
/*!
 FHPIB_getLen

 Public function defined in fh_pib.h
 */
MAC_INTERNAL_API uint8_t FHPIB_getLen(uint16_t fhPibId)
{
    (void)fhPibId;
    return(0);
}

/*!
 FHPIB_get

 Public function defined in fh_pib.h
 */
MAC_INTERNAL_API FHAPI_status FHPIB_get(uint16_t fhPibId, void *pData)
{
    (void)fhPibId;
    (void)pData;
    return(FHAPI_STATUS_ERR);
}
#endif /* FEATURE_FREQ_HOP_MODE */
