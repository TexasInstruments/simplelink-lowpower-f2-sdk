/******************************************************************************

 @file fh_nt.h

 @brief TIMAC 2.0 FH neighbor table API

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

#ifndef _FHNT_H_
#define _FHNT_H_

/******************************************************************************
 Includes
 *****************************************************************************/

#include "hal_types.h"
#include "hal_mcu.h"
#include "fh_pib.h"
#include "fh_data.h"

#ifdef FEATURE_MAC_SECURITY
#include "mac_security_pib.h"
#include "mac_security.h"
#endif

#ifndef FEATURE_WISUN_SUPPORT
#define FHNT_SPLIT_TABLE
#endif



//#define FHNT_MAX_DEVICE_TABLE_ENTRIES           (100)
#define FHNT_WISUN_JOIN_TABLE_SIZE                 30
#define FHNT_TABLE_TYPE_FIXED                   (0)
#define FHNT_TABLE_TYPE_HOPPING                 (1)
#define FHNT_TABLE_TYPE_JOIN                    (2)

/******************************************************************************
 Constants and definitions
 *****************************************************************************/

/*! Node Entry is invalid */
#define FHNT_NODE_INVALID                   (0x0000)
/*! Node Entry is just created */
#define FHNT_NODE_CREATED                   (0x0001)
/*! Node Entry has UTIE information */
#define FHNT_NODE_W_UTIE                    (0x0002)
/*! Node Entry has USIE information */
#define FHNT_NODE_W_USIE                    (0x0004)
/*! Node Entry infomation is expired */
#define FHNT_NODE_EXPIRED                   (0x0008)

/******************************************************************************
 Typedefs
 *****************************************************************************/

/*! Node Entry channel information */
//typedef struct chInfo
typedef union chInfo
{
  uint8_t fixedChannel;
  uint8_t bitMap[FHPIB_MAX_BIT_MAP_SIZE];
}chInfo_t;

/*! Node Entry USIE parameter */
typedef struct structUsieParams
{
   uint8_t dwellInterval;
   uint8_t clockDrift;
   uint8_t timingAccuracy;
   /*fixed or list */
   uint8_t channelFunc;
   chInfo_t chInfo;
   /* numChannels = max. chans - # of excl chans */
   uint8_t numChannels;
} usieParams_t;

/*! Node Entry BSIE parameter */
struct structBsieParams
{
   uint32_t bcInterval;
   uint16_t bcSchedId;
   usieParams_t UsieParams_s;
};

/*! Node Fixed Table Entry structure */
typedef struct __attribute__((__packed__)) node_fixed_entry
{
   uint16_t       ref_timeStamp;

   uint8_t        fixedChannel;

   uint8_t        valid;
   uint16_t        EUI_index;
} NODE_FIXED_ENTRY_s;

/*! Node Entry structure */
typedef struct __attribute__((__packed__)) node_entry
{
   uint32_t       ufsi;

   uint32_t       ref_timeStamp;

   //sAddrExt_t     dstAddr;

   usieParams_t   UsieParams_s;

   uint8_t        valid;
   //uint16_t        EUI_index;
   sAddrExt_t     extAddr;
} NODE_ENTRY_s;

/*! Node Entry purge timer */
typedef struct nt_hnd_s
{
  uint32_t                  purgeCount;
  uint32_t                  purgeTime;
  FH_macFHTimer_t           purgeTimer;
  uint16_t                  maxNumNonSleepNodes;   /*<! Maximum number of nonSleep Node */
  uint16_t                  maxNumSleepNodes;      /*<! Maximum number of Sleep Node */
  uint16_t                  numNonSleepNode;       /*<! number of active non-sleepy node */
  uint16_t                  numSleepNode;          /*<! number of active sleepy node */
  uint16_t                  numJoinNode;           /*<! number of joining nodes */
  NODE_ENTRY_s             *pNonSleepNtTable;      /*<! pointer to network node NT */
  NODE_ENTRY_s             *pJoinTable;            /*<! pointer to joining node NT */
  NODE_FIXED_ENTRY_s       *pSleepNtTable;         /*<! pointer to sleep node NT table  */
  uint8_t                  macSecurity;            /*<! FH module security setting  */
} FHNT_HND_s;

typedef struct FHNT_TEMP_table
{
    sAddrExt_t  *pAddr;
    uint16_t     maxNumNode;
    uint16_t     num_node;
} FHNT_TEMP_TABLE_s;

typedef struct nt_eui_debug
{
    uint16_t num_device_add;
    uint16_t num_device_del_temp;
    uint8_t  num_eui_not_found_in_temp;
    uint8_t  num_temp_index_not_found_in_nt;
}FHNT_DEBUG_s;

#define FH_OPT_LRM_NODES 10

typedef struct {
  sAddr_t nodeAddr;
  uint8   channel;
} NODE_OPT_ENTRY_s;

typedef struct {
    NODE_OPT_ENTRY_s node[FH_OPT_LRM_NODES];
    uint8    nodeIdx;
} FHNT_OPT_Table_s;

/******************************************************************************
 Global Externs
 *****************************************************************************/

/******************************************************************************
 Prototype
 *****************************************************************************/

/*!
 * @brief       This function is used to initialize node variables
 */
MAC_INTERNAL_API void FHNT_reset(void);

/*!
 * @brief       This function is used to initialize node variables and timer
 */
MAC_INTERNAL_API void FHNT_init(void);

/*!
 * @brief       This function is used to create the new entry in neighbor table
 *
 * @param pAddr - pointer of node address
 *
 * @return      pointer of node entry
 */
MAC_INTERNAL_API FHAPI_status FHNT_createEntry(sAddrExt_t *pAddr,NODE_ENTRY_s *pEntry);

/*!
 * @brief Create a table entry with address pAddr for a specific table type.
 *        Copies network info from existing tables if entry exists.
 *
 * @param tableType - Type of the table to create an entry for. Supported types are
 *                    FHNT_TABLE_TYPE_JOIN only
 * @param pAddr     - Extended address of entry to create in the table.
 */
MAC_INTERNAL_API FHAPI_status FHNT_createTableEntry(uint8_t tableType, uint8_t *pAddr);


MAC_INTERNAL_API FHAPI_status FHNT_deleteEntry(sAddrExt_t *pAddr);

/*!
 * @brief Delete a table entry with address pAddr for a specific table type.
 *
 * @param tableType - Type of the table to delete an entry for. Supported types are
 *                    FHNT_TABLE_TYPE_JOIN only
 * @param pAddr     - Extended address of entry to delete in the table.
 */
MAC_INTERNAL_API FHAPI_status FHNT_deleteTableEntry(uint8_t tableType, uint8_t *pAddr);

/*!
 * @brief Restore a table entry with address pAddr for a specific table type.
 *        This restores the specified entry to the primary (nonSleepy) table.
 *
 * @param tableType - Type of the table to restore an entry from. Supported types are
 *                    FHNT_TABLE_TYPE_JOIN only
 * @param pAddr     - Extended address of entry to restore in the table.
 */
MAC_INTERNAL_API FHAPI_status FHNT_restoreTableEntry(uint8_t tableType, uint8_t *pAddr);

/*!
 * @brief       This function is used to retrieve the node entry information
 *              from neighbor table
 *
 * @param pAddr - pointer of node address
 * @param pEntry - pointer of pointer of node entry
 *
 * @return      The status of the operation, as follows:<BR>
 *              [FHAPI_STATUS_SUCCESS]
 *              (@ref FHAPI_STATUS_SUCCESS)
 *               - Operation successful<BR>
 *              [FHAPI_STATUS_ERR_EXPIRED_NODE]
 *              (@ref FHAPI_STATUS_ERR_EXPIRED_NODE)
 *               - UTIE info is expired<BR>
 *              [FHAPI_STATUS_ERR_NO_ENTRY_IN_THE_NEIGHBOR]
 *              (@ref FHAPI_STATUS_ERR_NO_ENTRY_IN_THE_NEIGHBOR)
 *               - the destination address is not in the FH neighbor table<BR>
 *              [FHAPI_STATUS_ERR]
 *              (@ref FHAPI_STATUS_ERR)
 *               - parameter is wrong<BR>
 */
MAC_INTERNAL_API FHAPI_status FHNT_getEntry(sAddr_t *pDstAddr,
                                            NODE_ENTRY_s *pEntry);

MAC_INTERNAL_API FHAPI_status FHNT_putEntry(const NODE_ENTRY_s *pEntry);

/*!
 * @brief       This function is used to purge the node entry information
 *              from neighbor table
 *
 * @param ts - time stamp
 */
MAC_INTERNAL_API void FHNT_purgeEntry(uint32_t ts);

MAC_INTERNAL_API sAddrExt_t *FHNT_getEUI(uint16_t index);

MAC_INTERNAL_API uint16_t FHNT_AddDeviceCB(sAddrExt_t *pEUI, uint16_t devIndex);
MAC_INTERNAL_API uint16_t FHNT_DelDeviceCB(sAddrExt_t *pEUI, uint16_t devIndex);

#ifdef FHNT_SPLIT_TABLE
MAC_INTERNAL_API FHAPI_status FHNT_getFixedEntry(sAddrExt_t *pAddr,
                                            NODE_ENTRY_s *pEntry);
MAC_INTERNAL_API FHAPI_status FHNT_putFixedEntry(const NODE_ENTRY_s *pEntry);
MAC_INTERNAL_API FHAPI_status FHNT_createFixedEntry(sAddrExt_t *pAddr,NODE_ENTRY_s *pEntry);

#endif

void FHNT_initTempTable(void);

void FHNT_addOptEntry(sAddr_t *pSrcAddr, uint8_t channel);
MAC_INTERNAL_API FHAPI_status FHNT_getOptEntry(sAddr_t *pSrcAddr, NODE_ENTRY_s *pEntry);
#endif
