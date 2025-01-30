/******************************************************************************

 @file fh_ie.h

 @brief TIMAC 2.0 FH IE API

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

#ifndef _FHIE_H_
#define _FHIE_H_

/******************************************************************************
 Includes
 *****************************************************************************/

#include "hal_types.h"
#include "hal_mcu.h"


/******************************************************************************
 Constants and definitions
 *****************************************************************************/

/*! Network Name Size */
#define FHIE_NETNAME_SIZE                   (32)
/*! default channel 0 in FCC */
#define FHIE_DEFAULT_CH0                    (902200)
/*! default channel spacing */
#define FHIE_DEFAULT_CHANNEL_SPACING        (0)
/*! default number of channels */
#define FHIE_DEFAULT_NO_OF_CHANNELS         (129)

/*! Channel Plan Domain tag */
#define FHIE_CP_REGULATORY_DOMAIN           (0)
/*! Channel Plan vendor tag */
#define FHIE_CP_VENDOR_SPECIFIED            (1)
/*! Channel function: single channel function (fixed channel) */
#define FHIE_CF_SINGLE_CHANNEL              (0)
/*! Channel function: TR51CF */
#define FHIE_CF_TR51CF                      (1)
/*! Channel function: DH1CF */
#define FHIE_CF_DH1CF                       (2)
/*! Exclude channel control: NO EC */
#define FHIE_ECC_NO_EC                      (0)
/*! Exclude channel control: RANGE */
#define FHIE_ECC_RANGE                      (1)
/*! Exclude channel control: MASK */
#define FHIE_ECC_BIT_MASK                   (2)

/* long type */
/*! Sub ID: US IE */
#define FHIE_PIE_SUB_IE_SUB_ID_US_IE        0x01
/*! Sub ID: BS IE */
#define FHIE_PIE_SUB_IE_SUB_ID_BS_IE        0x02
/*! Sub ID: VP IE */
#define FHIE_PIE_SUB_IE_SUB_ID_VP_IE        0x03
/* short type */
/*! Sub ID: PAN IE */
#define FHIE_PIE_SUB_IE_SUB_ID_PAN_IE       0x04
/*! Sub ID: NETNAME IE */
#define FHIE_PIE_SUB_IE_SUB_ID_NETNAME_IE   0x05
/*! Sub ID: PANVER IE */
#define FHIE_PIE_SUB_IE_SUB_ID_PANVER_IE    0x06
/*! Sub ID: GTKHASH IE */
#define FHIE_PIE_SUB_IE_SUB_ID_GTKHASH_IE   0x07

/*! Sub ID: UT IE */
#define FHIE_HIE_SUB_ID_UT_IE               0x01
/*! Sub ID: BT IE */
#define FHIE_HIE_SUB_ID_BT_IE               0x02
/*! Sub ID: FC IE */
#define FHIE_HIE_SUB_ID_FC_IE               0x03
/*! Sub ID: RSL IE */
#define FHIE_HIE_SUB_ID_RSL_IE              0x04
/*! Sub ID: EA IE */
#define FHIE_HIE_SUB_ID_EA_IE               0x09


#define NO_OF_CHANNEL_PLAN                (1)   //  (7)


#define IE_DESCRIPTOR_LEN               (2)
#define SUB_IE_DESCRIPTOR_LEN           (2)
#define IE_TYPE_HEADER                  (0)
#define IE_TYPE_PAYLOAD                 (1)
#define IE_TYPE_S                       (1)
#define IE_TYPE_P                       (15)

/* payload ie */
#define PIE_GROUP_ID_WISUN              (0x4)
#define PIE_GROUP_ID_S                  (4)
#define PIE_GROUP_ID_P                  (11)
#define PIE_LEN_S                       (11)
#define PIE_LEN_P                       (0)
#define PIE_SUB_IE_TYPE_SHORT           (0x0)
#define PIE_SUB_IE_TYPE_LONG            (0x1)
#define PIE_SUB_IE_TYPE_S               (1)
#define PIE_SUB_IE_TYPE_P               (15)
#define PIE_SUB_IE_SUB_ID_SHORT_S       (7)
#define PIE_SUB_IE_SUB_ID_SHORT_P       (8)
#define PIE_SUB_IE_LEN_SHORT_S          (8)
#define PIE_SUB_IE_LEN_SHORT_P          (0)
#define PIE_SUB_IE_SUB_ID_LONG_S        (4)
#define PIE_SUB_IE_SUB_ID_LONG_P        (11)
#define PIE_SUB_IE_LEN_LONG_S           (11)
#define PIE_SUB_IE_LEN_LONG_P           (0)

/* header ie */
#define HIE_ELEM_ID_WISUN               (0x2a)
#define HIE_ELEM_ID_S                   (8)
#define HIE_ELEM_ID_P                   (7)
#define HIE_LEN_S                       (7)
#define HIE_LEN_P                       (0)

#define HIE_LEN_SUB_ID                  (1)
#define HIE_LEN_UT_IE                   (4)
#ifdef FEATURE_WISUN_SUPPORT
#define HIE_LEN_BT_IE                   (5)
#define HIE_LEN_EA_IE                   (8)
#else
#define HIE_LEN_BT_IE                   (6)
#endif
#define HIE_LEN_FC_IE                   (2)
#define HIE_LEN_RSL_IE                  (1)

#define FH_WISUN_PIE_BITMAP             0x00FF0000
#define FH_WISUN_HIE_BITMAP             0x0000000f
#define FH_WISUN_PIE_BITMAP_START       0x00010000
#define FH_WISUN_HIE_BITMAP_START       0x00000001

#define IE_PACKING(var,size,position)   \
    (((uint16_t)(var)&(((uint16_t)1<<(size))-1))<<(position))
#define IE_UNPACKING(var,size,position) \
    (((uint16_t)(var)>>(position))&(((uint16_t)1<<(size))-1))
#define GET_BYTE(var,bytenum)           \
    (((var)>>((bytenum)*8))&0xFF)
#define MAKE_UINT16(low,high)           \
    (((low)&0x00FF)|(((high)&0x00FF)<<8))

#define MAKE_PIE(id, len)               \
    (IE_PACKING(IE_TYPE_PAYLOAD, IE_TYPE_S, IE_TYPE_P)  \
    |IE_PACKING((id), PIE_GROUP_ID_S, PIE_GROUP_ID_P)  \
    |IE_PACKING((len), PIE_LEN_S, PIE_LEN_P))
#define MAKE_PIE_SUB_IE_LONG(id, len)   \
    (IE_PACKING(PIE_SUB_IE_TYPE_LONG, PIE_SUB_IE_TYPE_S, PIE_SUB_IE_TYPE_P) \
    |IE_PACKING((id), PIE_SUB_IE_SUB_ID_LONG_S, PIE_SUB_IE_SUB_ID_LONG_P)  \
    |IE_PACKING((len), PIE_SUB_IE_LEN_LONG_S, PIE_SUB_IE_LEN_LONG_P))
#define MAKE_PIE_SUB_IE_SHORT(id, len)  \
    (IE_PACKING(PIE_SUB_IE_TYPE_SHORT, PIE_SUB_IE_TYPE_S, PIE_SUB_IE_TYPE_P) \
    |IE_PACKING((id), PIE_SUB_IE_SUB_ID_SHORT_S, PIE_SUB_IE_SUB_ID_SHORT_P) \
    |IE_PACKING((len), PIE_SUB_IE_LEN_SHORT_S, PIE_SUB_IE_LEN_SHORT_P))
#define MAKE_HIE(id, len)               \
    (IE_PACKING(IE_TYPE_HEADER, IE_TYPE_S, IE_TYPE_P)   \
    |IE_PACKING((id), HIE_ELEM_ID_S, HIE_ELEM_ID_P)    \
    |IE_PACKING((len), HIE_LEN_S, HIE_LEN_P))
#define MAKE_CP(cp, cf, ecc)            (((cp)&7)|(((cf)&7)<<3)|(((ecc)&3)<<6))
#define GET_CHAN_PLAN(chctrl)           ((chctrl)&7)
#define GET_CHAN_FUNC(chctrl)           (((chctrl)>>3)&7)
#define GET_CHAN_ECC(chctrl)            (((chctrl)>>6)&3)
#ifndef FEATURE_WISUN_SUPPORT
#define MAKE_PANCTL(use, rm, er, fv)    \
    (((use)&1)|(((rm)&1)<<1)|(((er)&1)<<2)|(((fv)&7)<<5))
#else
#define MAKE_PANCTL(use, rm, fv)    \
    (((use)&1)|(((rm)&1)<<1)|(((fv)&7)<<5))
#endif
#define DEFAULT_CHANNEL_PLAN            FHIE_CP_REGULATORY_DOMAIN

#define FH_US_IE_FIXED_PART_LEN         (4)
#define FH_BS_IE_FIXED_PART_LEN         (10)
#ifndef FEATURE_WISUN_SUPPORT
#define FH_PAN_IE_LEN                   (4)
#else
#define FH_PAN_IE_LEN                   (5)
#endif

/* VPID(1) + MSGID (1) HopLimit (1) +Seq(1)+Originator(8) == 12 bytes
 * +user payload (15)
 */
#define FH_VP_IE_LEN                   (27)

#define FH_NETNAME_IE_LEN_MAX           FHIE_NETNAME_SIZE
#define FH_PANVER_IE_LEN                (2)
#define FH_GTK0HASH_LEN                 (8)
#define FH_GTKHASH_IE_LEN               (32)

#define FH_CP_RD_LEN                    (2)
#define FH_CP_VS_LEN                    (6)
#define FH_UC_FIXED_CHANNEL_LEN         (2)

#define FH_UC                           (0)
#define FH_BC                           (1)

#define MAX_NO_OF_CHANNELS              (17)

#if 0
#define NO_OF_CHANNEL_PLAN              \
    (sizeof(FHIE_channelPlan)/sizeof(FHIE_channelPlan_t))
#endif

#define FH_CHANNEL_OFFSET               (0)

#define FH_PIE_MAX_LEN                  (127)
#define FH_PIE_MIN_LEN                  SUB_IE_DESCRIPTOR_LEN
#define FH_HIE_MAX_LEN                  (127)
#define FH_HIE_MIN_LEN                  (1)

#define FH_VALID                        (1)
#define FH_INVALID                      (0)

#define FH_UT_IE_FT_PAN_ADVERT          (0)
#define FH_UT_IE_FT_PAN_ADVERT_SOLICIT  (1)
#define FH_UT_IE_FT_PAN_CONFIG          (2)
#define FH_UT_IE_FT_PAN_CONFIG_SOLICIT  (3)
#define FH_UT_IE_FT_DATA                (4)
#define FH_UT_IE_FT_ACK                 (5)
#define FH_UT_IE_FT_EAPOL               (6)
/* #define FHIE_DBG */
/******************************************************************************
 Typedefs
 *****************************************************************************/

/*! HIE UTIE definiton */
typedef struct {
    uint8_t frameTypeId;            /*!< FH frame type */
    uint32_t ufsi;                  /*!< ufsi */
} FHIE_utIE_t;

/*! HIE BTIE definiton */
typedef struct {
    uint16_t bsn;                   /*!< BSN */
    uint32_t bfio;                  /*!< bfio*/
} FHIE_btIE_t;

/*! HIE FCIE definiton */
typedef struct {
    uint8_t txFlowControl;          /*!< TX flow control */
    uint8_t rxFlowControl;          /*!< RX flow control */
} FHIE_fcIE_t;

/*! HIE RSLIE definiton */
typedef struct {
    uint8_t rsl;                    /*!< RSL value */
} FHIE_rslIE_t;

/*! PIE schedule IE definiton */
typedef struct {
    uint8_t dwellInterval;          /*!< dwell interval */
    uint8_t clockDrift;             /*!< clock drift */
    uint8_t timingAccuracy;         /*!< timing accuracy */
    uint8_t channelPlan;            /*!< channle plan */
    uint8_t channelFunction;        /*!< channel function */
    uint8_t excludedChannelControl; /*!< excluded channel control */
} schedInfo_t;

/*! channle info definiton */
typedef struct {
    uint16_t noOfChannels;              /*!< number of channels */
    uint16_t fixedChannel;              /*!< fixed channel */
    uint8_t excludedChannelMask[32];    /*!< excluded channles */
} chanInfo_t;

/*! USIE definiton */
typedef struct {
    schedInfo_t schedInfo;              /*!< unicast schedule info */
    chanInfo_t chanInfo;                /*!< unicast channel info */
} FHIE_usIe_t;

/*! BSIE definiton */
typedef struct {
    uint32_t broadcastInterval;         /*!< broadcast interval */
    uint16_t broadcastSchedId;          /*!< broadcast schedule ID */
    schedInfo_t schedInfo;              /*!< broadcast schedule info */
    chanInfo_t chanInfo;                /*!< broadcast channel info */
} FHIE_bsIE_t;

#define WISUN_VENDOR_ID_TI              (36)
#define VPIE_MSG_ID_PANID               (0x01)
#define VPIE_MSG_ID_MPL_PAYLOAD         (0x02)
#define VPIE_MSG_ID_TERMINATOR          (0xFF)

/*! VPIE definition */
typedef struct FHIE_vpIE_{
    //uint8_t  vendorID;                /*!< VENDOR ID, TI's ID = 36 (decimal) */
    uint16_t panID;                     /*!< network PAN in VP IE */
    uint8_t payload[FH_VP_IE_LEN];      /*!< VPIE payload */
} FHIE_vpIE_t;

/*! PANIE definiton */
typedef struct {
    uint16_t panSize;                   /*!< PAN size */
    uint8_t routingCost;                /*!< routing cost */
    uint8_t useParentBSIE;              /*!< use parent BSIE flag */
    uint8_t routingMethod;              /*!< routing method */
    uint8_t eapolReady;                 /*!< EAPOL ready flag */
    uint8_t fanTpsVersion;              /*!< FAN TPS version */
} FHIE_panIE_t;
#ifdef FEATURE_WISUN_SUPPORT
typedef struct {
    uint16_t panSize;                   /*!< PAN size */
    uint16_t routingCost;               /*!< routing cost */
    uint8_t useParentBSIE;              /*!< use parent BSIE flag */
    uint8_t routingMethod;              /*!< routing method */
    uint8_t fanTpsVersion;              /*!< FAN TPS version */
} FHIE_panIE_new_t;
#endif
/*! NETNAMEIE definiton */
typedef struct {
    uint8_t netName[32];                /*!< network name */
} FHIE_netNameIE_t;

/*! PANVERIE definiton */
typedef struct {
    uint16_t panVersion;                /*!< PAN version*/
} FHIE_panVerIE_t;

/*! GTK HASH definiton */
typedef struct {
    uint8_t gtk0Hash[8];                /*!< get hash-0*/
    uint8_t gtk1Hash[8];                /*!< get hash-1*/
    uint8_t gtk2Hash[8];                /*!< get hash-2*/
    uint8_t gtk3Hash[8];                /*!< get hash-3*/
} FHIE_gtkHashIE_t;

/*! IE Info definition */
typedef union {
    FHIE_fcIE_t fcIe;                   /*!< FC IE */
    FHIE_rslIE_t rslIe;                 /*!< RSL IE */
} FHIE_ieInfo_t;

#if defined(FEATURE_WISUN_EDFE_SUPPORT) || defined(FEATURE_WISUN_MIN_EDFE)
typedef struct {
    FHIE_fcIE_t   fcIe;                       /*!< FC IE */
    FHIE_rslIE_t  rslIe;                      /*!< RSL IE */
    uint8*        ptrFCIE;                    /*!< pointer to the FC IE Content */
    uint8*        ptrRSLIE;                   /*!< pointer to the RSL IE Content */
    uint8         txfc;                       /*!< update with the information from FC-IE */
    uint8         rxfc;                       /*!< update with the inforamtion from FC-IE */
    uint32        txOffset;                   /*!< tx offset time */
} FHIE_ieInfo_new_t;
#endif

typedef struct {
    uint8_t regulatoryDomain;
    uint8_t operatingClass;
    uint32_t ch0;
    uint8_t channelSpacing; //0:200kHz, 1:400kHz, 2:600kHz
    uint16_t noOfChannels;
    /* default exclusion channel list based on regulatory domain and operating class */
    uint8_t regChannelList[MAX_NO_OF_CHANNELS];
} FHIE_channelPlan_t;

typedef struct {
    uint8_t type;
    uint8_t id;
    uint16_t len;
    uint8_t *pVal;
} FHIE_ie_t;

#ifdef FHIE_DBG
typedef struct {
    uint16_t pie_getlen_try;
    uint16_t pie_getlen_fail;
    uint16_t hie_getlen_try;
    uint16_t hie_getlen_fail;
    uint16_t pie_gen_try;
    uint16_t pie_gen_fail;
    uint16_t pie_gen_not_supported;
    uint16_t hie_gen_try;
    uint16_t hie_gen_fail;
    uint16_t pie_parse_try;
    uint16_t pie_parse_invalid;
    uint16_t pie_parse_not_supported;
    uint16_t hie_parse_try;
    uint16_t hie_parse_invalid;
    uint16_t hie_parse_not_supported;
    uint16_t pie_extract_try;
    uint16_t pie_extract_invalid;
    uint16_t pie_extract_not_supported;
    uint16_t hie_extract_try;
    uint16_t hie_extract_invalid;
    uint16_t hie_extract_not_supported;
} FHIE_dbg_t;
#endif

/******************************************************************************
 Global Externs
 *****************************************************************************/

/******************************************************************************
 Prototypes
 *****************************************************************************/

/*!
 * @brief       get the cumulative length of the IEs indicated by the bitmap
 *
 * @param bitmap - bitmap of IEs (each bit corresponds to type of IE) <BR>
 *
 * @return      length of the IEs<BR>

 */
extern MAC_INTERNAL_API uint16_t FHIE_getLen(uint32_t bitmap);

/*!
 * @brief       generate the IE bit stream
 *
 * @param bitmap - bitmap of IEs (each bit corresponds to type of IE)
 * @param pdata - pointer to the buffer where IE bitstream is stored
 * @param pMacTx - pointer to MAC TX packet
 * @param pIeInfo - pointer to IE info data structure
 *
 * @return      length of the IEs<BR>

 */
#if defined(FEATURE_WISUN_EDFE_SUPPORT) || defined(FEATURE_WISUN_MIN_EDFE)
extern MAC_INTERNAL_API uint16_t FHIE_gen(uint8_t *pData,
                                          uint32_t bitmap,
                                          macTx_t *pMacTx,
                                          FHIE_ieInfo_new_t *pIeInfo);
#else
extern MAC_INTERNAL_API uint16_t FHIE_gen(uint8_t *pData,
                                          uint32_t bitmap,
                                          macTx_t *pMacTx,
                                          FHIE_ieInfo_t *pIeInfo);
#endif
/*!
 * @brief       parse the PIEs, USIE, BSIE, PANVER IE etc
 *
 * @param pData - pointer to input payload IE byte stream
 * @param pSrcAddr - pointer to source address of the received packet
 * @param pLen1 - pointer of number of bytes of parsed WiSUN PIEs (output)
 * @param pLen2 - pointer of number of bytes of unsupported WiSUN PIEs (output)
 *
 *
 * @return      status of operation<BR>

 */

extern MAC_INTERNAL_API FHAPI_status FHIE_parsePie(uint8_t *pData,
                                                   sAddrExt_t *pSrcAddr,
                                                   uint16_t *pLen1,
                                                   uint16_t *pLen2);


/*!
 * @brief       This function is used by LMAC to parse for header IEs.
 *              It parses UTIE, BTIE, FCIE, RSLIE
 *
 * @param pData - pointer to input header IE byte stream
 * @param pSrcAddr - pointer to source address of the received packet
 * @param ts  - SFD time stamp of the received packet in miliseconds
 * @param pLen1  - pointer of number of bytes of parsed WiSUN HIEs
 * @param pLen2  - pointer of number of bytes of unsupported WiSUN HIEs
 *
  * @return      The status of the operation, as follows:<BR>
 *              [FH_STATUS_SUCCESS](@ref FH_STATUS_SUCCESS)
 *               - Operation successful<BR>
 *              [FH_STATUS_ERR_INVALID_FORMAT]
 *              (@ref FH_STATUS_ERR_INVALID_FORMAT)
 *               - invalid IE format
 *              [FH_STATUS_ERR_NOT_SUPPORTED_IE]
 *              (@ref FH_STATUS_ERR_NOT_SUPPORTED_IE)
 *               - IE is not supported
 *
 */
extern MAC_INTERNAL_API FHAPI_status FHIE_parseHie(uint8_t *pData,
                                                   sAddrExt_t *pSrcAddr,
                                                   uint32_t ts,
                                                   uint16_t *pLen1,
                                                   uint16_t *pLen2);

/*!
 * @brief       extract the header IEs
 *
 * @param pdata - pointer to the received packet buffer
 * @param ieType - type of HIE - UTIE, BTIE
 * @param pOut  - pointer to extracted values (output)
 *
  * @return      The status of the operation, as follows:<BR>
 *              [FH_STATUS_SUCCESS](@ref FH_STATUS_SUCCESS)
 *               - Operation successful<BR>
 *              [FH_STATUS_ERR_INVALID_FORMAT]
 *              (@ref FH_STATUS_ERR_INVALID_FORMAT)
 *               - invalid IE format
 *              [FH_STATUS_ERR_NOT_SUPPORTED_IE]
 *              (@ref FH_STATUS_ERR_NOT_SUPPORTED_IE)
 *               - IE is not supported
 *
 */

extern MAC_INTERNAL_API FHAPI_status FHIE_extractHie(uint8_t *pData,
                                                     IE_TYPE_t ieType,
                                                     uint8_t *pOut);


/*!
 * @brief       extract the payload IE of type ieType
 *
 * @param pdata - pointer to the received packet buffer
 * @param ieType - type of HIE - UTIE, BTIE
 * @param pOut  - pointer to extracted values (output)
 *
  * @return      The status of the operation, as follows:<BR>
 *              [FH_STATUS_SUCCESS](@ref FH_STATUS_SUCCESS)
 *               - Operation successful<BR>
 *              [FH_STATUS_ERR_INVALID_FORMAT]
 *              (@ref FH_STATUS_ERR_INVALID_FORMAT)
 *               - invalid IE format
 *              [FH_STATUS_ERR_NOT_SUPPORTED_IE]
 *              (@ref FH_STATUS_ERR_NOT_SUPPORTED_IE)
 *               - IE is not supported
 *
 */

extern MAC_INTERNAL_API FHAPI_status FHIE_extractPie(uint8_t *pData,
                                                     IE_TYPE_t ieType,
                                                     uint8_t *pOut);
#endif
