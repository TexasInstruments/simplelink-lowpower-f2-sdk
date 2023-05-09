/******************************************************************************

 @file fh_nt.c

 @brief TIMAC 2.0 FH neighbor table API

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

#include "fh_api.h"
#include "fh_nt.h"
#include "fh_ie.h"
#include "fh_util.h"
#include "fh_mgr.h"
#include "mac_radio.h"
#include "mac_pib.h"
#include "mac_mgmt.h"

/******************************************************************************
 Constants and definitions
 *****************************************************************************/

#define CLOCK_LT(a,b)                   ((signed long)((a)-(b)) < 0)
#define NODE_TO_PURGE                   (0)
#define NODE_TO_KEEP                    (1)
#define CLOCK_DRIFT_UNKNOWN             (255)
#define CHANNEL_FUNCTION_UNKNOWN        (255)
#define ONE_HOUR_IN_MS                  (60*60*1000)
#define PURGE_TIMER_PERIOD              ONE_HOUR_IN_MS
#define PURGE_INTERVAL                  (10 * ONE_HOUR_IN_MS)

#ifdef FEATURE_FREQ_HOP_MODE

/******************************************************************************
  ROM jump table will support
  1. TIMAC ROM image build
  2. TIMAC stack build without ROM image
  3. TIMAC stack with calling ROM image
 *****************************************************************************/
#include "rom_jt_154.h"


/******************************************************************************
 Local variables
 *****************************************************************************/

/******************************************************************************
 Global variables
 *****************************************************************************/

/******************************************************************************
 Local Function Prototypes
 *****************************************************************************/

uint16_t FHNT_addTempEUI(sAddrExt_t *peui);
uint16_t FHNT_delTempIndex(uint16_t eui_index);
uint16_t FHNT_getFixedNTIndexFromTempIndex(uint16_t tempIndex);
uint16_t FHNT_getNTIndexFromTempIndex(uint16_t tempIndex, uint8_t *pTableType);
void FHNT_getRemoveEntryFromTemp(void);

#if (defined FEATURE_MAC_SECURITY) || (defined TIMAC_ROM_IMAGE_BUILD)
uint8_t FHNT_delTempEUI(sAddrExt_t *peui);
#endif


/******************************************************************************
 Local Functions
 *****************************************************************************/
void FHNT_initTempTable(void)
{
    uint16_t i;
    sAddrExt_t  FH_InValidEUI;

    MAP_osal_memcpy(&FH_InValidEUI,&(FHPIB_defaults.macTrackParentEUI),sizeof(sAddrExt_t));
    /*
     * reset the all value to 0xFF (invalid address)
     */
    for (i=0; i < FHNT_tempTable.maxNumNode;i++)
    {
        MAP_osal_memcpy(&FHNT_tempTable.pAddr[i], &FH_InValidEUI,sizeof(sAddrExt_t));
    }
    FHNT_tempTable.num_node = 0;

    /* set the eui_index to invalid value */
    for(i = 0; i < FHNT_hnd.maxNumNonSleepNodes; i++)
    {
        FHNT_hnd.pNonSleepNtTable[i].EUI_index = MAC_EUI_INVALID_INDEX;
        FHNT_hnd.pNonSleepNtTable[i].valid = FHNT_NODE_INVALID;
    }

#ifdef FHNT_SPLIT_TABLE
    /* set the eui_index to invalid value */

    for(i = 0; i < FHNT_hnd.maxNumSleepNodes; i++)
    {
        FHNT_hnd.pSleepNtTable[i].EUI_index = MAC_EUI_INVALID_INDEX;
        FHNT_hnd.pSleepNtTable[i].valid = FHNT_NODE_INVALID;
    }

#endif
    MAP_osal_memset(&FHNT_Dbg,0x0,sizeof(FHNT_Dbg));
}

#ifdef FHNT_SPLIT_TABLE
uint16_t FHNT_getFixedNTIndexFromTempIndex(uint16_t tempIndex)
{
    uint16_t eui_index,i;
    NODE_FIXED_ENTRY_s *pFixedNodeEntry;

    pFixedNodeEntry = FHNT_hnd.pSleepNtTable;
    /*
     * convert the temp table index to EUI index in FHNT
     */
#if (defined TIMAC_ROM_IMAGE_BUILD)
    if (FHNT_hnd.macSecurity)
    {
        eui_index = tempIndex + macCfg.maxDeviceTableEntries;
    }
    else
    {
        eui_index = tempIndex;
    }
#else
#ifdef FEATURE_MAC_SECURITY
    eui_index = tempIndex + macCfg.maxDeviceTableEntries;
#else
    eui_index = tempIndex;
#endif
#endif
    for(i = 0; i < FHNT_hnd.maxNumSleepNodes; i++, pFixedNodeEntry++)
    {
        if (pFixedNodeEntry->valid == 0)
            continue;

        if( pFixedNodeEntry->EUI_index == eui_index )
        {
            return i;
        }
    }

    FHNT_Dbg.num_temp_index_not_found_in_nt++;

    return MAC_EUI_INVALID_INDEX;
}

#endif

uint16_t FHNT_getNTIndexFromTempIndex(uint16_t tempIndex,uint8_t *pTableType)
{
    uint16_t eui_index,i;
    NODE_ENTRY_s *pNodeEntry;

#ifdef FHNT_SPLIT_TABLE
    eui_index = MAP_FHNT_getFixedNTIndexFromTempIndex(tempIndex);

    if (eui_index != MAC_EUI_INVALID_INDEX)
    {
        *pTableType = FHNT_TABLE_TYPE_FIXED;
        return eui_index;
    }
#endif

    *pTableType = FHNT_TABLE_TYPE_HOPPING;
    pNodeEntry = FHNT_hnd.pNonSleepNtTable;
    /*
     * convert the temp table index to EUI index in FHNT
     */
#if (defined TIMAC_ROM_IMAGE_BUILD)
    if (FHNT_hnd.macSecurity)
    {
        eui_index = tempIndex + macCfg.maxDeviceTableEntries;
    }
    else
    {
        eui_index = tempIndex;
    }
#else

#ifdef FEATURE_MAC_SECURITY
    eui_index = tempIndex + macCfg.maxDeviceTableEntries;
#else
    eui_index = tempIndex;
#endif
#endif

    for(i = 0; i < FHNT_hnd.maxNumNonSleepNodes; i++, pNodeEntry++)
    {
        if (pNodeEntry->valid == 0)
            continue;

        if( pNodeEntry->EUI_index == eui_index )
        {
            return i;
        }
    }

    FHNT_Dbg.num_temp_index_not_found_in_nt++;

    return MAC_EUI_INVALID_INDEX;
}

void FHNT_getRemoveEntryFromTemp(void)
{
    uint16_t i,nt_index,eui_index;
    uint32_t maxElapsedTime;
    uint32_t elapsedTime;
    uint32_t curTime,nodeTime;
    uint16_t maxIdx;
    halIntState_t intState;
    uint8_t tableType;
#ifdef FHNT_SPLIT_TABLE
    uint8_t maxTableType;
#endif
    sAddrExt_t  FH_InValidEUI;

    FHNT_Dbg.num_device_del_temp++;

    MAP_osal_memcpy(&FH_InValidEUI,&(FHPIB_defaults.macTrackParentEUI),sizeof(sAddrExt_t));
    curTime = MAP_ICall_getTicks();
    maxElapsedTime = 0;
    maxIdx = 0;

    for (i=0; i < FHNT_tempTable.maxNumNode;i++)
    {
        /*
         * get the eui_index in FHNT
         */
        nt_index = MAP_FHNT_getNTIndexFromTempIndex(i,&tableType);
        if (nt_index == MAC_EUI_INVALID_INDEX)
        {
            /*
             * error case, we can't find
             * temp table entry is not reference by NT
             * this EUI maybe is used by fixed hopping table
             */
            MAP_FHUTIL_assert(NULL);
            return ;
        }

#ifdef FHNT_SPLIT_TABLE
        if (tableType == FHNT_TABLE_TYPE_FIXED)
        {
            nodeTime = FHNT_hnd.pNonSleepNtTable[nt_index].ref_timeStamp << 16;
        }
        else
#endif
        {
            nodeTime = FHNT_hnd.pNonSleepNtTable[nt_index].ref_timeStamp;
        }
        elapsedTime = MAP_FHUTIL_elapsedTime(curTime,nodeTime);
        if(elapsedTime > maxElapsedTime)
        {
            maxElapsedTime = elapsedTime;
            maxIdx = nt_index;
#ifdef FHNT_SPLIT_TABLE
            maxTableType = tableType;
#endif
        }
    }

    HAL_ENTER_CRITICAL_SECTION(intState);
    /*
     * here temp table is full, need to remove the oldest entry
     */
#ifdef FHNT_SPLIT_TABLE
    if (maxTableType == FHNT_TABLE_TYPE_FIXED)
    {
        NODE_FIXED_ENTRY_s *pEntry;

        pEntry = &(FHNT_hnd.pSleepNtTable[maxIdx]);
        eui_index = pEntry->EUI_index;
        /* remove entry from fixed table */
        pEntry->valid = FHNT_NODE_INVALID;
        pEntry->EUI_index = MAC_EUI_INVALID_INDEX;

        FHNT_hnd.numSleepNode--;
    }
    else
#endif
    {
        NODE_ENTRY_s *pNode;

        pNode = &(FHNT_hnd.pNonSleepNtTable[maxIdx]);

        eui_index = pNode->EUI_index;
        /* remove entry from the hopping tabel */
        pNode->valid = FHNT_NODE_INVALID;
        pNode->EUI_index = MAC_EUI_INVALID_INDEX;
        FHNT_hnd.numNonSleepNode--;
    }
    /*
     * temp table index
     */
#if (defined TIMAC_ROM_IMAGE_BUILD)
    if (FHNT_hnd.macSecurity)
    {
        i = eui_index - macCfg.maxDeviceTableEntries;
    }
    else
    {
        i = eui_index;
    }
#else

#ifdef FEATURE_MAC_SECURITY
    i = eui_index - macCfg.maxDeviceTableEntries;
#else
    i = eui_index ;
#endif

#endif
    /* put the invalid entry in temp table*/

    MAP_osal_memcpy(&FHNT_tempTable.pAddr[i],&FH_InValidEUI,sizeof(sAddrExt_t));

    FHNT_tempTable.num_node --;

    HAL_EXIT_CRITICAL_SECTION(intState);

    return ;
}

uint16_t FHNT_addTempEUI(sAddrExt_t *peui)
{
    uint16_t i;
    halIntState_t intState;
    uint16_t index_base=0;
    sAddrExt_t  FH_InValidEUI;

    MAP_osal_memcpy(&FH_InValidEUI,&(FHPIB_defaults.macTrackParentEUI),sizeof(sAddrExt_t));
#if (defined TIMAC_ROM_IMAGE_BUILD)
    if (FHNT_hnd.macSecurity)
    {
        uint16_t eui_index = MAP_macGetEUIIndex(peui);
        if(eui_index != MAC_EUI_INVALID_INDEX)
        {
            return(eui_index);
        }
        index_base = macCfg.maxDeviceTableEntries;
    }

#else
#ifdef FEATURE_MAC_SECURITY
    /*
     * check to see if the EUI address is found in the mac device
     * if found, don't create the temp entry
     */
    uint16_t eui_index = MAP_macGetEUIIndex(peui);
    if(eui_index != MAC_EUI_INVALID_INDEX)
    {
        return(eui_index);
    }
    index_base = macCfg.maxDeviceTableEntries;
#endif

#endif

    for (i=0; i< FHNT_tempTable.maxNumNode;i++)
    {
        /*
         * find the first the empty slot
         */
        if (MAP_memcmp(&FHNT_tempTable.pAddr[i], &FH_InValidEUI,sizeof(sAddrExt_t)) == 0 )
        {
            MAP_osal_memcpy(&FHNT_tempTable.pAddr[i],peui,sizeof(sAddrExt_t));

            HAL_ENTER_CRITICAL_SECTION(intState);
            FHNT_tempTable.num_node ++;
            HAL_EXIT_CRITICAL_SECTION(intState);
            break;

        }
    }

    return i+index_base;
}

uint16_t FHNT_delTempIndex(uint16_t eui_index)
{
    uint16_t tempIndex;
    sAddrExt_t  FH_InValidEUI;

    MAP_osal_memcpy(&FH_InValidEUI,&(FHPIB_defaults.macTrackParentEUI),sizeof(sAddrExt_t));
#if (defined TIMAC_ROM_IMAGE_BUILD)
    if (FHNT_hnd.macSecurity)
    {
        if (eui_index < macCfg.maxDeviceTableEntries)
        {   /*
            * EUI is MAC device table, dont mess up the temp table
            */
            return eui_index;
        }
        tempIndex = eui_index - macCfg.maxDeviceTableEntries;
    }
    else
    {
        tempIndex = eui_index ;
    }
#else

#ifdef FEATURE_MAC_SECURITY
    if (eui_index < macCfg.maxDeviceTableEntries)
    {   /*
        * EUI is MAC device table, dont mess up the temp table
        */
        return eui_index;
    }
    tempIndex = eui_index - macCfg.maxDeviceTableEntries;
#else
    tempIndex = eui_index ;
#endif

#endif
    MAP_memcmp(&FHNT_tempTable.pAddr[tempIndex], &FH_InValidEUI,sizeof(FH_InValidEUI));
    FHNT_tempTable.num_node--;

    return tempIndex;
}

//inline static uint8_t FHNT_assessTime(uint32_t curTime,
uint8_t FHNT_assessTime(uint32_t curTime,
                                      uint32_t oldTime,
                                      uint32_t validTime)
{
    uint32_t elapsedTime = MAP_FHUTIL_elapsedTime(curTime, oldTime);

    if(elapsedTime >= validTime)
    {
        return NODE_TO_PURGE;
    }
    else
    {
        return NODE_TO_KEEP;
    }
}

void FHNT_purgeTimerIsrCb(uint8_t parameter)
{
    uint32_t curTime;

    curTime = MAP_ICall_getTicks();
    FHNT_hnd.purgeCount++;
    FHNT_hnd.purgeTime = curTime;
    MAP_FHNT_purgeEntry(curTime);
    MAP_FHMGR_macStartFHTimer(&FHNT_hnd.purgeTimer, TRUE);
}

NODE_ENTRY_s *FHNT_getRemoveEntry(void)
{
    uint16_t i;
    NODE_ENTRY_s *pNodeEntry;
    uint32_t maxElapsedTime;
    uint32_t elapsedTime;
    uint32_t curTime;
    uint16_t maxIdx;
    sAddrExt_t parentEUI;

    pNodeEntry = FHNT_hnd.pNonSleepNtTable;

    MAP_FHPIB_get(FHPIB_TRACK_PARENT_EUI, &parentEUI);

    curTime = MAP_ICall_getTicks();
    maxElapsedTime = 0;
    maxIdx = MAC_EUI_INVALID_INDEX;

    for(i = 0; i < FHNT_hnd.maxNumNonSleepNodes; i++, pNodeEntry++)
    {
        sAddrExt_t *pEUI;
        pEUI = MAP_FHNT_getEUI(pNodeEntry->EUI_index);

        if(MAP_memcmp(&parentEUI, pEUI, sizeof(sAddrExt_t)))
        {
            elapsedTime = MAP_FHUTIL_elapsedTime(curTime,
                                             pNodeEntry->ref_timeStamp);
            if(elapsedTime > maxElapsedTime)
            {
                maxElapsedTime = elapsedTime;
                maxIdx = i;
            }
        }
    }

    if ( maxIdx == MAC_EUI_INVALID_INDEX )
    {
        return NULL;
    }

    /* purge the temp table */
    MAP_FHNT_delTempIndex(FHNT_hnd.pNonSleepNtTable[maxIdx].EUI_index);

    return (&FHNT_hnd.pNonSleepNtTable[maxIdx]);
}

/******************************************************************************
 Public Functions
 *****************************************************************************/

/*!
 FHNT_reset

 Public function defined in fh_nt.h
 */
MAC_INTERNAL_API void FHNT_reset(void)
{
    uint16_t memSize;

    MAP_osal_memset(&FHNT_hnd, 0, sizeof(FHNT_HND_s));

    /*
     * the maxNumEntry is defined in fh_global.c
     * only reset the reset field
     */
    FH_txTiming.wrIndex = 0;
    memSize = FH_txTiming.maxNumEntry * sizeof(FH_TX_TIMING_s);
    FH_txTiming.pTxPkt = MAP_osal_mem_alloc(memSize);
    if (FH_txTiming.pTxPkt == NULL)
    {
        MAP_FHUTIL_assert(FH_txTiming.pTxPkt);
    }

    MAP_osal_memset(FH_txTiming.pTxPkt, 0,memSize);
}

/*!
 FHNT_init

 Public function defined in fh_nt.h
 */

MAC_INTERNAL_API void FHNT_init(void)
{
    MAP_FHNT_reset();

    /*initialize the purge timer */
    FHNT_hnd.purgeTimer.pFunc = MAP_FHNT_purgeTimerIsrCb;

    FHNT_hnd.purgeTimer.duration = PURGE_TIMER_PERIOD * (TICKPERIOD_MS_US);

    /*Start purge Timer */
    MAP_FHMGR_macStartFHTimer(&FHNT_hnd.purgeTimer, TRUE);
}

/*!
 FHNT_purgeEntry

 Public function defined in fh_nt.h
 */
MAC_INTERNAL_API void FHNT_purgeEntry(uint32_t ts)
{
    halIntState_t intState;
    uint16_t i;
    NODE_ENTRY_s *pNodeEntry;
    sAddrExt_t parentEUI;

    MAP_FHPIB_get(FHPIB_TRACK_PARENT_EUI, &parentEUI);

    /* purge the fixed table */
#ifdef FHNT_SPLIT_TABLE

    if (FHNT_hnd.numSleepNode)
    {
        NODE_FIXED_ENTRY_s *pFixedNodeEntry;

        pFixedNodeEntry = FHNT_hnd.pSleepNtTable;

        for(i = 0; i < FHNT_hnd.maxNumSleepNodes; i++, pNodeEntry++)
        {
            if(pFixedNodeEntry->valid & FHNT_NODE_W_UTIE)
            {
                sAddrExt_t *pEUI;

                pEUI = MAP_FHNT_getEUI(pFixedNodeEntry->EUI_index);

                if(MAP_memcmp(&parentEUI, pEUI, sizeof(sAddrExt_t)))
                {
                    if(MAP_FHNT_assessTime(ts, pFixedNodeEntry->ref_timeStamp<<16,
                                     ((uint32_t)PURGE_INTERVAL * TICKPERIOD_MS_US))
                                      == NODE_TO_PURGE)
                    {
                        pFixedNodeEntry->valid = FHNT_NODE_INVALID;

                        /* purge the temp table */
                        MAP_FHNT_delTempIndex(pFixedNodeEntry->EUI_index);
                        pFixedNodeEntry->EUI_index = MAC_EUI_INVALID_INDEX;

                        HAL_ENTER_CRITICAL_SECTION(intState);
                        if (FHNT_hnd.numSleepNode)
                        {
                            FHNT_hnd.numSleepNode--;
                        }
                        HAL_EXIT_CRITICAL_SECTION(intState);

                    }
                }
            }
        }
    }
#endif

    if (FHNT_hnd.numNonSleepNode == 0)
    {
        return;
    }

    pNodeEntry = FHNT_hnd.pNonSleepNtTable;

    for(i = 0; i < FHNT_hnd.maxNumNonSleepNodes; i++, pNodeEntry++)
    {
        if(pNodeEntry->valid & FHNT_NODE_W_UTIE)
        {
            sAddrExt_t *pEUI;

            pEUI = MAP_FHNT_getEUI(pNodeEntry->EUI_index);

            if(MAP_memcmp(&parentEUI, pEUI, sizeof(sAddrExt_t)))
            {
                if(MAP_FHNT_assessTime(ts, pNodeEntry->ref_timeStamp,
                                 ((uint32_t)PURGE_INTERVAL * TICKPERIOD_MS_US))
                                  == NODE_TO_PURGE)
                {
                    pNodeEntry->valid = FHNT_NODE_INVALID;

                    /* purge the temp table */
                    MAP_FHNT_delTempIndex(pNodeEntry->EUI_index);
                    pNodeEntry->EUI_index = MAC_EUI_INVALID_INDEX;

                    HAL_ENTER_CRITICAL_SECTION(intState);
                    if (FHNT_hnd.numNonSleepNode)
                    {
                        FHNT_hnd.numNonSleepNode--;
                    }
                    HAL_EXIT_CRITICAL_SECTION(intState);

                    /* need to purge the temp table */
                }
            }
        }
    }
}

/*!
 FHNT_createEntry

 Public function defined in fh_nt.h
 */
MAC_INTERNAL_API FHAPI_status FHNT_createEntry(sAddrExt_t *pAddr,NODE_ENTRY_s *pEntry)
{
    uint16_t i;
    uint16_t noOfNode;
    halIntState_t intState;
    NODE_ENTRY_s *pNodeEntry = NULL;

    if (FHNT_hnd.maxNumNonSleepNodes == 0 )
    {   /* there is no non-sleep NT */
        return FHAPI_STATUS_ERR_NO_ENTRY_IN_THE_NEIGHBOR;
    }

    noOfNode = FHNT_hnd.numNonSleepNode;

    if(noOfNode == FHNT_hnd.maxNumNonSleepNodes)
    {
        pNodeEntry = MAP_FHNT_getRemoveEntry();
    }
    else if (FHNT_tempTable.num_node == FHNT_tempTable.maxNumNode)
    {   /* the temp table is full
        *  get removed NT entry
        */
        MAP_FHNT_getRemoveEntryFromTemp();

    }

    if (pNodeEntry == NULL)
    {
        pNodeEntry = FHNT_hnd.pNonSleepNtTable;

        for(i = 0; i < FHNT_hnd.maxNumNonSleepNodes; i++, pNodeEntry++)
        {
            if(!pNodeEntry->valid)
            {
                HAL_ENTER_CRITICAL_SECTION(intState);
                FHNT_hnd.numNonSleepNode++;
                HAL_EXIT_CRITICAL_SECTION(intState);
                break;
            }
        }
        if(i == FHNT_hnd.maxNumNonSleepNodes )    //this means problem happens
        {
            return FHAPI_STATUS_ERR;
        }
    }

    MAP_osal_memset(pNodeEntry, 0, sizeof(NODE_ENTRY_s));

    /*
     * need to add the temp table first
     * and save the temp_table index
     */
    pNodeEntry->EUI_index = MAP_FHNT_addTempEUI(pAddr) ;
    pNodeEntry->UsieParams_s.clockDrift = CLOCK_DRIFT_UNKNOWN;
    pNodeEntry->UsieParams_s.channelFunc = CHANNEL_FUNCTION_UNKNOWN;
    pNodeEntry->valid = FHNT_NODE_CREATED;

    //*pEntry = *pNodeEntry;
    MAP_osal_memcpy(pEntry, pNodeEntry, sizeof(NODE_ENTRY_s));
    return FHAPI_STATUS_SUCCESS;
}

/*!
 FHNT_getEntry

 Public function defined in fh_nt.h
 */
MAC_INTERNAL_API FHAPI_status FHNT_getEntry(sAddr_t *pDstAddr,NODE_ENTRY_s *pEntry)
{
    uint16_t i;
    NODE_ENTRY_s *pNodeEntry;
    uint16_t neighborValidTime;
    uint32_t curTime;
    sAddrExt_t *pEUI;
    FHAPI_status status = FHAPI_STATUS_SUCCESS;
    sAddrExt_t *pAddr = (sAddrExt_t *)pDstAddr;
    sAddr_t localDstAddr;
    sAddr_t *pLocalDstAddr = &localDstAddr;

    //check for PHY_ID
    /* check for coord address and map to ext.. addr */
    if((pDstAddr->addrMode == SADDR_MODE_SHORT ) && !macPanCoordinator)
    {
        if(!MAP_memcmp(&pDstAddr->addr.shortAddr, &macPib.coordShortAddress, 2))
        {
            pLocalDstAddr->addrMode = SADDR_MODE_EXT;
            MAP_sAddrExtCpy(pLocalDstAddr->addr.extAddr, macPib.coordExtendedAddress.addr.extAddr);
        }
        pAddr = &pLocalDstAddr->addr.extAddr;
    }

    if(macPanCoordinator)
    {
        status = MAP_FHNT_getOptEntry(pDstAddr, pEntry);

        if(status == FHAPI_STATUS_SUCCESS)
        {
            return status;
        }
    }


#ifdef FHNT_SPLIT_TABLE
    /*
     * check the fixed table entry first
     */
    status = MAP_FHNT_getFixedEntry(pAddr,pEntry);
    if (status == FHAPI_STATUS_SUCCESS)
    {   /* find the entry in the fixed table */
        return status;
    }
#endif

    pEntry->valid = FHNT_NODE_INVALID;
    status = FHAPI_STATUS_ERR_NO_ENTRY_IN_THE_NEIGHBOR;

    pNodeEntry = FHNT_hnd.pNonSleepNtTable;

    MAP_FHPIB_get(FHPIB_NEIGHBOR_VALID_TIME, &neighborValidTime);
    curTime = MAP_ICall_getTicks();

    for(i = 0; i < FHNT_hnd.maxNumNonSleepNodes; i++, pNodeEntry++)
    {
        if (pNodeEntry->valid == 0 )
            continue;

        /*
         * entry in valid, check if the EUI is matched
         */
        pEUI = MAP_FHNT_getEUI(pNodeEntry->EUI_index);
        if (MAP_memcmp(pEUI, pAddr, sizeof(sAddrExt_t)) == 0 )
        {
             if(pNodeEntry->UsieParams_s.channelFunc == FHIE_CF_DH1CF)
            {
                if(MAP_FHNT_assessTime(curTime, pNodeEntry->ref_timeStamp,
                  (neighborValidTime * 60 * 1000 * TICKPERIOD_MS_US))
                   == NODE_TO_PURGE)
                {
                    pNodeEntry->valid |= FHNT_NODE_EXPIRED;
                    status = FHAPI_STATUS_ERR_EXPIRED_NODE;
                }
                else
                {
                    status = FHAPI_STATUS_SUCCESS;
                }
            }
            else if ( pNodeEntry->UsieParams_s.channelFunc ==
                            FHIE_CF_SINGLE_CHANNEL)
            {
                status = FHAPI_STATUS_SUCCESS;
            }
            else
            {
                status = FHAPI_STATUS_ERR;
            }

            /* copy node entry */
            //*pEntry = *pNodeEntry;
            MAP_osal_memcpy(pEntry, pNodeEntry, sizeof(NODE_ENTRY_s));
            break;
        }
    }

    return status;
}

MAC_INTERNAL_API FHAPI_status FHNT_putEntry(const NODE_ENTRY_s *pEntry)
{
    uint16_t i;
    NODE_ENTRY_s *pNodeEntry;

#ifdef FHNT_SPLIT_TABLE
    /*
     * check the fixed table first
     */
    if (pEntry->UsieParams_s.channelFunc == FHIE_CF_SINGLE_CHANNEL)
    {
        return MAP_FHNT_putFixedEntry(pEntry);
    }
#endif

    pNodeEntry = FHNT_hnd.pNonSleepNtTable;

    for(i = 0; i < FHNT_hnd.maxNumNonSleepNodes; i++, pNodeEntry++)
    {
        if (pNodeEntry->valid == 0 )
            continue;

        /*
         * entry in valid, check if the EUI_index is matched
         */
        if (pNodeEntry->EUI_index == pEntry->EUI_index)
        {
            //*pNodeEntry = *pEntry;
            MAP_osal_memcpy(pNodeEntry, pEntry, sizeof(NODE_ENTRY_s));

            return FHAPI_STATUS_SUCCESS;
        }
    }

    return FHAPI_STATUS_ERR_NO_ENTRY_IN_THE_NEIGHBOR;
}

MAC_INTERNAL_API sAddrExt_t *FHNT_getEUI(uint16_t index)
{
#if (defined TIMAC_ROM_IMAGE_BUILD)
    if (FHNT_hnd.macSecurity)
    {
        uint16_t temp_index;
        if (index < macCfg.maxDeviceTableEntries )
        {
            /* get EUI from the macDevice table
             *
             */
            return MAP_macGetEUI(index);
        }
        else
        {
            /* get the FH NT temp table
             *
             */
            temp_index = index - macCfg.maxDeviceTableEntries ;

            return &FHNT_tempTable.pAddr[temp_index];
        }

    }
    else
    {
        return &FHNT_tempTable.pAddr[index];
    }
#else

#ifdef FEATURE_MAC_SECURITY
    /*
     * secuirty is enabled build
     */
    uint16_t temp_index;
    if (index < macCfg.maxDeviceTableEntries )
    {
        /* get EUI from the macDevice table
         *
         */
        return MAP_macGetEUI(index);
    }
    else
    {
        /* get the FH NT temp table
         *
         */
        temp_index = index - macCfg.maxDeviceTableEntries ;

        return &FHNT_tempTable.pAddr[temp_index];
    }
#else
    /*
     * security is disabled build
     */
    return &FHNT_tempTable.pAddr[index];
#endif

#endif
}

MAC_INTERNAL_API uint16_t FHNT_AddDeviceCB(sAddrExt_t *pEUI, uint16_t devIndex)
{
#if (defined TIMAC_ROM_IMAGE_BUILD)

    uint16_t i, tempIndex, ntIndex;
    halIntState_t intState;
    uint8_t tableType;
    sAddrExt_t  FH_InValidEUI;

    MAP_osal_memcpy(&FH_InValidEUI,&(FHPIB_defaults.macTrackParentEUI),sizeof(sAddrExt_t));
    FHNT_Dbg.num_device_add++;

    if (FHNT_hnd.macSecurity)
    {
        /*
         * step 1:
         *  search the temp table to find the matched EUI address, save the
         *  temp index
         * step 2:
         *  from the temp table index to find the corresponding entry in the FH NT
         *  nt_index
         * step3
         *  update the NT entry EUi to device index
         */
        tempIndex = MAC_EUI_INVALID_INDEX;   /* invalid value */
        for (i=0; i<FHNT_tempTable.maxNumNode;i++)
        {
            if (MAP_memcmp(&FHNT_tempTable.pAddr[i], pEUI,sizeof(sAddrExt_t) ) == 0)
            {   /* find the matched address */
                tempIndex = i;
                break;
            }
        }

        if (tempIndex == MAC_EUI_INVALID_INDEX)
        {
            FHNT_Dbg.num_eui_not_found_in_temp++;
            return tempIndex;
        }

        ntIndex = MAP_FHNT_getNTIndexFromTempIndex(tempIndex,&tableType);

        if (ntIndex == MAC_EUI_INVALID_INDEX )
        {   /*
             * EUI is address in the temp table, but it is not reference by NT
            */
            return ntIndex;
        }

        if (tableType == FHNT_TABLE_TYPE_HOPPING)
        {
            /* update the EUI to deviceIndex */
            FHNT_hnd.pNonSleepNtTable[ntIndex].EUI_index = devIndex;
        }
    #ifdef FHNT_SPLIT_TABLE
        else
        {
            /* update the EUI to deviceIndex */
            FHNT_hnd.pSleepNtTable[ntIndex].EUI_index = devIndex;
        }
    #endif

        /* invalide the temp entry */
        MAP_osal_memcpy(&FHNT_tempTable.pAddr[tempIndex],&FH_InValidEUI,sizeof(sAddrExt_t));
        HAL_ENTER_CRITICAL_SECTION(intState);
        FHNT_tempTable.num_node--;
        HAL_EXIT_CRITICAL_SECTION(intState);

        return ntIndex;
    }
    else
    {
        return MAC_EUI_INVALID_INDEX;
    }
#else

#ifdef FEATURE_MAC_SECURITY
    uint16_t i, tempIndex, ntIndex;
    halIntState_t intState;
    uint8_t tableType;
    sAddrExt_t  FH_InValidEUI;

    FHNT_Dbg.num_device_add++;
    MAP_osal_memcpy(&FH_InValidEUI,&(FHPIB_defaults.macTrackParentEUI),sizeof(sAddrExt_t));
    /*
     * step 1:
     *  search the temp table to find the matched EUI address, save the
     *  temp index
     * step 2:
     *  from the temp table index to find the corresponding entry in the FH NT
     *  nt_index
     * step3
     *  update the NT entry EUi to device index
     */
    tempIndex = MAC_EUI_INVALID_INDEX;   /* invalid value */
    for (i=0; i<FHNT_tempTable.maxNumNode;i++)
    {
        if (MAP_memcmp(&FHNT_tempTable.pAddr[i], pEUI,sizeof(sAddrExt_t) ) == 0)
        {   /* find the matched address */
            tempIndex = i;
            break;
        }
    }

    if (tempIndex == MAC_EUI_INVALID_INDEX)
    {
        FHNT_Dbg.num_eui_not_found_in_temp++;
        return tempIndex;
    }

    ntIndex = MAP_FHNT_getNTIndexFromTempIndex(tempIndex,&tableType);

    if (ntIndex == MAC_EUI_INVALID_INDEX )
    {   /*
         * EUI is address in the temp table, but it is not reference by NT
        */
        return ntIndex;
    }

    if (tableType == FHNT_TABLE_TYPE_HOPPING)
    {
        /* update the EUI to deviceIndex */
        FHNT_hnd.pNonSleepNtTable[ntIndex].EUI_index = devIndex;
    }
#ifdef FHNT_SPLIT_TABLE
    else
    {
        /* update the EUI to deviceIndex */
        FHNT_hnd.pSleepNtTable[ntIndex].EUI_index = devIndex;
    }
#endif

    /* invalide the temp entry */
    MAP_osal_memcpy(&FHNT_tempTable.pAddr[tempIndex],&FH_InValidEUI,sizeof(sAddrExt_t));
    HAL_ENTER_CRITICAL_SECTION(intState);
    FHNT_tempTable.num_node--;
    HAL_EXIT_CRITICAL_SECTION(intState);

    return ntIndex;
#else
    (void)pEUI;
    (void)devIndex;
    return MAC_EUI_INVALID_INDEX;
#endif /* FEATURE_MAC_SECURITY */

#endif
}

MAC_INTERNAL_API uint16_t FHNT_DelDeviceCB(sAddrExt_t *pEUI, uint16_t devIndex)
{
#if (defined TIMAC_ROM_IMAGE_BUILD)
    uint16_t i;
    NODE_ENTRY_s *pNodeEntry;
    halIntState_t intState;

    if (FHNT_hnd.macSecurity)
    {
        HAL_ENTER_CRITICAL_SECTION(intState);

        pNodeEntry = FHNT_hnd.pNonSleepNtTable;

        /*
         * step 1:
         *  from devIndex to find the corresponding ntIndex
         * step 2:
         *  zero the entry and mark the corresponding NT entry-eui-index with invalid value
         */

        for (i = 0; i < FHNT_hnd.maxNumNonSleepNodes ;i++, pNodeEntry++)
        {
            if (pNodeEntry->valid == 0)
                continue;

            if ( pNodeEntry->EUI_index == devIndex)
            {
                /* find the entry */
                pNodeEntry->valid = 0;
                pNodeEntry->EUI_index = MAC_EUI_INVALID_INDEX;

                HAL_EXIT_CRITICAL_SECTION(intState);

                return i;
            }

        }

        HAL_EXIT_CRITICAL_SECTION(intState);
        return MAC_EUI_INVALID_INDEX;

    }
    else
    {
        return MAC_EUI_INVALID_INDEX;
    }
#else

#ifdef FEATURE_MAC_SECURITY
    uint16_t i;
    NODE_ENTRY_s *pNodeEntry;
    halIntState_t intState;

    HAL_ENTER_CRITICAL_SECTION(intState);

    pNodeEntry = FHNT_hnd.pNonSleepNtTable;

    /*
     * step 1:
     *  from devIndex to find the corresponding ntIndex
     * step 2:
     *  zero the entry and mark the corresponding NT entry-eui-index with invalid value
     */

    for (i = 0; i < FHNT_hnd.maxNumNonSleepNodes ;i++, pNodeEntry++)
    {
        if (pNodeEntry->valid == 0)
            continue;

        if ( pNodeEntry->EUI_index == devIndex)
        {
            /* find the entry */
            pNodeEntry->valid = 0;
            pNodeEntry->EUI_index = MAC_EUI_INVALID_INDEX;

            HAL_EXIT_CRITICAL_SECTION(intState);

            return i;
        }

    }

    HAL_EXIT_CRITICAL_SECTION(intState);
    return MAC_EUI_INVALID_INDEX;
#else
    (void)pEUI;
    (void)devIndex;
    return MAC_EUI_INVALID_INDEX;
#endif /* FEATURE_MAC_SECURITY */

#endif
}

#ifdef FHNT_SPLIT_TABLE
/*!
 FHNT_getFixedEntry

 Public function defined in fh_nt.h
 */

MAC_INTERNAL_API FHAPI_status FHNT_getFixedEntry(sAddrExt_t *pAddr,
                                            NODE_ENTRY_s *pEntry)
{
    uint16_t i;
    NODE_FIXED_ENTRY_s *pNodeEntry;
    sAddrExt_t *pEUI;
    FHAPI_status status;

    pEntry->valid = FHNT_NODE_INVALID;
    status = FHAPI_STATUS_ERR_NO_ENTRY_IN_THE_NEIGHBOR;

    pNodeEntry = FHNT_hnd.pSleepNtTable;
    for(i = 0; i < FHNT_hnd.maxNumSleepNodes; i++, pNodeEntry++)
    {
        if (pNodeEntry->valid == 0 )
            continue;

        /*
         * entry in valid, check if the EUI is matched
         */
        pEUI = MAP_FHNT_getEUI(pNodeEntry->EUI_index);
        if (MAP_memcmp(pEUI, pAddr, sizeof(sAddrExt_t)) == 0 )
        {
            /*
             * fixed channel, we don't check ref_timestamp
             */

            status = FHAPI_STATUS_SUCCESS;

            /* copy node entry */
            pEntry->valid = pNodeEntry->valid;
            pEntry->ref_timeStamp = pNodeEntry->ref_timeStamp <<16;
            pEntry->EUI_index = pNodeEntry->EUI_index;
            pEntry->UsieParams_s.channelFunc = FHIE_CF_SINGLE_CHANNEL;
            pEntry->UsieParams_s.chInfo.fixedChannel = pNodeEntry->fixedChannel;

            break;
        }
    }

    return status;
}

MAC_INTERNAL_API FHAPI_status FHNT_putFixedEntry(const NODE_ENTRY_s *pEntry)
{
    uint16_t i;
    NODE_FIXED_ENTRY_s *pNodeEntry;

    pNodeEntry = FHNT_hnd.pSleepNtTable;
    for(i = 0; i < FHNT_hnd.maxNumSleepNodes; i++, pNodeEntry++)
    {
        if (pNodeEntry->valid == 0 )
            continue;

        /*
         * entry in valid, check if the EUI_index is matched
         */
        if (pNodeEntry->EUI_index == pEntry->EUI_index)
        {
            pNodeEntry->valid = pEntry->valid ;
            /* only high 16 bits */
            pNodeEntry->ref_timeStamp =  pEntry->ref_timeStamp >>16;
            pNodeEntry->EUI_index =pEntry->EUI_index ;

            pNodeEntry->fixedChannel = pEntry->UsieParams_s.chInfo.fixedChannel;

            return FHAPI_STATUS_SUCCESS;
        }
    }

    return FHAPI_STATUS_ERR_NO_ENTRY_IN_THE_NEIGHBOR;
}

NODE_FIXED_ENTRY_s *FHNT_getRemoveFixedEntry(void)
{
    uint16_t i;
    NODE_FIXED_ENTRY_s *pNodeEntry;
    uint32_t maxElapsedTime;
    uint32_t elapsedTime;
    uint32_t curTime;
    uint16_t maxIdx;
    sAddrExt_t parentEUI;

    pNodeEntry = FHNT_hnd.pSleepNtTable;

    MAP_FHPIB_get(FHPIB_TRACK_PARENT_EUI, &parentEUI);

    curTime = MAP_ICall_getTicks();
    maxElapsedTime = 0;
    maxIdx = MAC_EUI_INVALID_INDEX;

    for(i = 0; i < FHNT_hnd.maxNumSleepNodes; i++, pNodeEntry++)
    {
        sAddrExt_t *pEUI;
        pEUI = MAP_FHNT_getEUI(pNodeEntry->EUI_index);
        if(MAP_memcmp(&parentEUI, pEUI, sizeof(sAddrExt_t)))
        {
            elapsedTime = MAP_FHUTIL_elapsedTime(curTime,
                                             (pNodeEntry->ref_timeStamp)<<16);
            if(elapsedTime > maxElapsedTime)
            {
                maxElapsedTime = elapsedTime;
                maxIdx = i;
            }
        }
    }

    if ( maxIdx == MAC_EUI_INVALID_INDEX )
    {
        return NULL;
    }
    /* pureg the temp table */
    MAP_FHNT_delTempIndex(FHNT_hnd.pSleepNtTable[maxIdx].EUI_index);

    return(&FHNT_hnd.pSleepNtTable[maxIdx]);
}


MAC_INTERNAL_API FHAPI_status FHNT_createFixedEntry(sAddrExt_t *pAddr,NODE_ENTRY_s *pEntry)
{
    uint16_t i;
    uint16_t noOfNode;
    halIntState_t intState;
    NODE_FIXED_ENTRY_s *pFixedNodeEntry = NULL;

    if (FHNT_hnd.maxNumSleepNodes == 0 )
    {   /* there is no sleep NT */
        return FHAPI_STATUS_ERR_NO_ENTRY_IN_THE_NEIGHBOR;
    }

    noOfNode = FHNT_hnd.numSleepNode;

    if(noOfNode == FHNT_hnd.maxNumSleepNodes)
    {
        pFixedNodeEntry = MAP_FHNT_getRemoveFixedEntry();
    }
    else if (FHNT_tempTable.num_node == FHNT_tempTable.maxNumNode)
    {   /* the temp table is full
        *  get removed NT entry
        */
        MAP_FHNT_getRemoveEntryFromTemp();
    }

    if (pFixedNodeEntry == NULL)
    {

        pFixedNodeEntry = FHNT_hnd.pSleepNtTable;
        for(i = 0; i < FHNT_hnd.maxNumSleepNodes; i++, pFixedNodeEntry++)
        {
            if(!pFixedNodeEntry->valid)
            {
                HAL_ENTER_CRITICAL_SECTION(intState);
                FHNT_hnd.numSleepNode++;
                HAL_EXIT_CRITICAL_SECTION(intState);
                break;
            }
        }
        if(i == FHNT_hnd.maxNumSleepNodes)
        {
            return FHAPI_STATUS_ERR;
        }
    }

    //MAP_osal_memset(pFixedNodeEntry, 0, sizeof(NODE_FIXED_ENTRY_s));

    /*
     * need to add the temp table first
     * and save the temp_table index
     */
    pFixedNodeEntry->EUI_index = MAP_FHNT_addTempEUI(pAddr) ;

    pFixedNodeEntry->valid = FHNT_NODE_CREATED;

    pEntry->EUI_index = pFixedNodeEntry->EUI_index;
    pEntry->valid = pFixedNodeEntry->valid;
    /* fixed entry */
    pEntry->UsieParams_s.channelFunc = FHIE_CF_SINGLE_CHANNEL;
    return FHAPI_STATUS_SUCCESS;
}

#endif /* FHNT_SPLIT_TABLE */

MAC_INTERNAL_API FHAPI_status FHNT_getOptEntry(sAddr_t *pSrcAddr, NODE_ENTRY_s *pEntry)
{
    uint8 i;
    NODE_OPT_ENTRY_s *pNodeEntry;
    uint8 addrMode = pSrcAddr->addrMode;
    uint8 addrSize = macAddrSize[addrMode];
    FHAPI_status status = FHAPI_STATUS_ERR_NO_ENTRY_IN_THE_NEIGHBOR;
    halIntState_t s;

    pNodeEntry = FHNT_tableOpt.node;
    HAL_ENTER_CRITICAL_SECTION(s);
    for(i = 0; i < FH_OPT_LRM_NODES; i++)
    {
        if (addrMode == pNodeEntry->nodeAddr.addrMode)
        {
           if(MAP_memcmp(&pNodeEntry->nodeAddr, pSrcAddr, addrSize) == 0)
           {
               status = FHAPI_STATUS_SUCCESS;
               pEntry->UsieParams_s.channelFunc = FHIE_CF_SINGLE_CHANNEL;
               pEntry->UsieParams_s.chInfo.fixedChannel = pNodeEntry->channel;
               break;
           }
        }
        pNodeEntry++;
    }
    HAL_EXIT_CRITICAL_SECTION(s);
    return status;
}

MAC_INTERNAL_API NODE_OPT_ENTRY_s *FHNT_getAvailEntry(sAddr_t *pSrcAddr)
{
    uint8 i;
    NODE_OPT_ENTRY_s *pNodeEntry = NULL;
    NODE_OPT_ENTRY_s *pAvailNodeEntry = NULL;
    uint8 addrMode = pSrcAddr->addrMode;
    uint8 addrSize = macAddrSize[addrMode];

    pNodeEntry = FHNT_tableOpt.node;

    for(i = 0; i < FH_OPT_LRM_NODES; i++)
    {
        if (addrMode == pNodeEntry->nodeAddr.addrMode)
        {
           if (MAP_memcmp(&pNodeEntry->nodeAddr, pSrcAddr, addrSize) == 0)
           {
               pAvailNodeEntry = pNodeEntry;
               break;
           }
        }
        pNodeEntry++;
    }

    return pAvailNodeEntry;
}

void FHNT_addOptEntry(sAddr_t *pSrcAddr, uint8_t channel)
{
    NODE_OPT_ENTRY_s *pNodeEntry = NULL;
    halIntState_t s;

    HAL_ENTER_CRITICAL_SECTION(s);

    /*if node in NT, it will be returned in pNodeEntry */
    pNodeEntry = MAP_FHNT_getAvailEntry(pSrcAddr);

    /* no existing entry in NT, get next entry in the NT and
       update index */
    if (pNodeEntry == NULL)
    {
         pNodeEntry = &FHNT_tableOpt.node[FHNT_tableOpt.nodeIdx];
         /*update index in a circular way*/
         FHNT_tableOpt.nodeIdx++;
         if(FHNT_tableOpt.nodeIdx == FH_OPT_LRM_NODES)
         {
             FHNT_tableOpt.nodeIdx = 0;
         }
    }
    MAP_osal_memcpy(&pNodeEntry->nodeAddr, pSrcAddr, sizeof(sAddr_t));

    pNodeEntry->channel = channel;

    HAL_EXIT_CRITICAL_SECTION(s);
}
#endif
