/******************************************************************************

 @file fh_pib.h

 @brief TIMAC 2.0 FH PIB API

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

#ifndef _FHPIB_H_
#define _FHPIB_H_

/*!
 FH PIB API Interfaces
 ============================
 - FHPIB_reset()
 - FHPIB_set()
 - FHPIB_get()
 - FHPIB_getLen()

 <BR><BR>
 */

#include "hal_types.h"
#include "hal_mcu.h"
#include "mac_api.h"
#include "fh_api.h"

/******************************************************************************
 Defines
 *****************************************************************************/

/*! The parent EUI address */
#define FHPIB_TRACK_PARENT_EUI              0x2000
/*! Time between start of two broadcast slots in msec */
#define FHPIB_BC_INTERVAL                   0x2001
/*! Unicast excluded channels */
#define FHPIB_UC_EXCLUDED_CHANNELS          0x2002
/*! Broadcast excluded channels */
#define FHPIB_BC_EXCLUDED_CHANNELS          0x2003
/*! Duration of node's unicast slot */
#define FHPIB_UC_DWELL_INTERVAL             0x2004
/*! Duration of broadcast slot */
#define FHPIB_BC_DWELL_INTERVAL             0x2005
/*! Clock drift in PPM */
#define FHPIB_CLOCK_DRIFT                   0x2006
/*! Timing accuracy in 10usec resolution */
#define FHPIB_TIMING_ACCURACY               0x2007
/*! Unicast channel hopping function */
#define FHPIB_UC_CHANNEL_FUNCTION           0x2008
/*! Broadcast channel hopping function */
#define FHPIB_BC_CHANNEL_FUNCTION           0x2009
/*! Node is propagating parent's BS-IE */
#define FHPIB_USE_PARENT_BS_IE              0x200A
/*! Broadcast schedule ID for broadcast channel hopping sequence */
#define FHPIB_BROCAST_SCHED_ID              0x200B
/*! Unicast channel number when no hopping */
#define FHPIB_UC_FIXED_CHANNEL              0x200C
/*! Broadcast channel number when no hopping */
#define FHPIB_BC_FIXED_CHANNEL              0x200D
/*! Number of nodes in the PAN */
#define FHPIB_PAN_SIZE                      0x200E
/*! Estimate of routing path ETX to the PAN coordinator */
#define FHPIB_ROUTING_COST                  0x200F
/*! RPL(1), MHDS(0)*/
#define FHPIB_ROUTING_METHOD                0x2010
/*! Node can accept EAPOL message */
#define FHPIB_EAPOL_READY                   0x2011
/*! Wi-SUN FAN version */
#define FHPIB_FAN_TPS_VERSION               0x2012
/*! Network Name */
#define FHPIB_NET_NAME                      0x2013
/*! PAN version to notify PAN configuration changes */
#define FHPIB_PAN_VERSION                   0x2014
/*! Low order 64 bits of SHA256 hash of GTK */
#define FHPIB_GTK_0_HASH                    0x2015
/*! Low order 64 bits of SHA256 hash of GTK */
#define FHPIB_GTK_1_HASH                    0x2016
/*! Low order 64 bits of SHA256 hash of GTK */
#define FHPIB_GTK_2_HASH                    0x2017
/*! Low order 64 bits of SHA256 hash of GTK */
#define FHPIB_GTK_3_HASH                    0x2018
/*! Time in min during which the node info considered as valid */
#define FHPIB_NEIGHBOR_VALID_TIME           0x2019
/*! FH additional base wait time to sense target channel */
#define FHPIB_CSMA_BASE_BACKOFF             0x201A
/*! number of non-sleep nodes can be supported */
#define FHPIB_NUM_MAX_NON_SLEEP_NODES       0x201B
/*! number of sleep nodes can be supported */
#define FHPIB_NUM_MAX_SLEEP_NODES           0x201C
/*! number of temp nodes can be supported */
#define FHPIB_NUM_MAX_TEMP_NODES            0x201D

#ifndef FHPIB_MAX_BIT_MAP_SIZE
/*! maximum FH bitmap size */
#define FHPIB_MAX_BIT_MAP_SIZE              MAC_154G_CHANNEL_BITMAP_SIZ
#endif

/*! maximum size of network name */
#define FHPIB_NET_NAME_SIZE_MAX             (32)
/*! GTK Hash size */
#define FHPIB_GTK_HASH_SIZE                 (8)

#define FHPIB_DEFAULT_EUI                   \
        {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}

#define FHPIB_MAX_NUM_DEVICE                    (50)

#define FH_PIB_ID_START         FHPIB_TRACK_PARENT_EUI
#define FH_PIB_ID_END           FHPIB_NUM_MAX_TEMP_NODES
#define FH_PIB_ID_OFFSET        (0)
#define FH_PIB_ID_INVALID       (0xFFFF)

#define FH_UINT8_SIZE           (1)
#define FH_UINT16_SIZE          (2)
#define FH_UINT32_SIZE          (4)

/******************************************************************************
 Typedefs
 *****************************************************************************/

/* FH related PIB access and min/max table type */
typedef struct __attribute__((__packed__))
{
    uint8_t     offset;
    uint8_t     len;
    uint32_t    min;
    uint32_t    max;
} FH_PibTbl_t;


/*! FH-related PIB type */
typedef struct __attribute__((__packed__))
{
    /*! tracked parent EUI address */
    sAddrExt_t      macTrackParentEUI;
    /*! Broadcast interval */
    uint32_t        macBcInterval;
    /*! Unicast excluded channel list */
    uint8_t         macUcExcludedChannels[FHPIB_MAX_BIT_MAP_SIZE];
    /*! Broadcast excluded channel list */
    uint8_t         macBcExcludedChannels[FHPIB_MAX_BIT_MAP_SIZE];
    /*! Unicast dwell time in ms */
    uint8_t         macUcDwellInterval;
    /*! Broadcast dwell tim in ms */
    uint8_t         macBcDwellInterval;
    /*! Clock drift */
    uint8_t         macClockDrift;
    /*! Clock timing accuracy */
    uint8_t         macTimingAccuracy;
    /*! Unicast Channel function */
    uint8_t         macUcChannelFunction;
    /*! Broadcast Channel Function */
    uint8_t         macBcChannelFunction;
    /*! use parent BS IE flag */
    uint8_t         macUseParentBSIE;
    /*! Broadcast Schedule ID */
    uint16_t        macBroadcastSchedId;
    /*! Unicast Fixed channel only UC fixed channel mode */
    uint16_t        macUcFixedChannel;
    /*! Broadcast Fixed channel only BC fixed channel mode */
    uint16_t        macBcFixedChannel;
    /*! PAN ID size */
    uint16_t        macPanSize;
    /*! routing cost */
    uint8_t         macRoutingCost;
    /*! routing method */
    uint8_t         macRoutingMethod;
    /*! EAPOL ready flag */
    uint8_t         macEapolReady;
    /*! FAN TPS version */
    uint8_t         macFanTpsVersion;
    /*! network name */
    uint8_t         macNetName[FHPIB_NET_NAME_SIZE_MAX];
    /*! PAN version */
    uint16_t        macPanVersion;
    /*! GTK0 Hash */
    uint8_t         macGtk0Hash[FHPIB_GTK_HASH_SIZE];
    /*! GTK1 Hash */
    uint8_t         macGtk1Hash[FHPIB_GTK_HASH_SIZE];
    /*! GTK2 Hash */
    uint8_t         macGtk2Hash[FHPIB_GTK_HASH_SIZE];
    /*! GTK3 Hash */
    uint8_t         macGtk3Hash[FHPIB_GTK_HASH_SIZE];
    /*! Neighbor Valid Time */
    uint16_t        macNeighborValidTime;
    /*! base backoff Time */
    uint8_t         macBaseBackOff;
    /*! maximum non-sleep node */
    uint16_t         macMaxNonSleepNodes;
    /*! maximum sleep node */
    uint16_t         macMaxSleepNodes;
    /*! maximum temp table node */
    uint16_t         macMaxTempNodes;

} FHPIB_DB_t;

/******************************************************************************
 Global Externs
 *****************************************************************************/

/******************************************************************************
 Prototypes
 *****************************************************************************/

/*!
 * @brief       reset the FH PIBs to their default values. This API must be
 *             called by all other APIs
 */
extern MAC_INTERNAL_API void FHPIB_reset(void);


/*!
 * @brief       set the FH PIBs to a value
 * @param fhPibId - FH PIB ID value
 * @param pData - pointer to FH PIB data
 *
 * @return      The status of the operation, as follows:<BR>
 *              [FH_STATUS_SUCCESS](@ref FH_STATUS_SUCCESS)
 *               - Operation successful<BR>
 *              [other values]
 *               - failure<BR>
 */
extern MAC_INTERNAL_API FHAPI_status FHPIB_set(uint16_t fhPibId,void *pData);

/*!
 * @brief       get a FH PIB value
 * @param fhPibId - FH PIB ID value
 * @param pData - pointer to FH PIB data
 *
 * @return      The status of the operation, as follows:<BR>
 *              [FH_STATUS_SUCCESS](@ref FH_STATUS_SUCCESS)
 *               - Operation successful<BR>
 *              [other values]
 *               - failure<BR>
 */
extern MAC_INTERNAL_API FHAPI_status FHPIB_get(uint16_t fhPibId,void *pData);

/*!
 * @brief       gets the FH PIB attribute size
 * @param fhPibId - FH PIB ID value
 *
 * @return      The size of FH PIB in bytes:<BR>
 *
 */
extern MAC_INTERNAL_API uint8_t FHPIB_getLen(uint16_t fhPibId);
#endif // end of _FHPIB_H_ definition
