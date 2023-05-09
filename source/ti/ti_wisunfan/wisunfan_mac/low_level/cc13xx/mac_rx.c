/******************************************************************************

 @file  mac_rx.c

 @brief Describe the purpose and contents of the file.

 Group: WCS, LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2006-2023, Texas Instruments Incorporated

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

/* ------------------------------------------------------------------------------------------------
 *                                          Includes
 * ------------------------------------------------------------------------------------------------
 */

#ifndef LIBRARY
#include "ti_wisunfan_features.h"
#endif

/* hal */
#include "hal_defs.h"
#include "hal_types.h"

/* high-level */
#include "mac_high_level.h"
#include "mac_spec.h"
#include "mac_pib.h"
#include "mac_hl_patch.h"

/* MAC security */
#include "mac_security.h"

/* exported low-level */
#include "mac_low_level.h"

/* low-level specific */
#include "mac.h"
#include "mac_rx.h"
#include "mac_tx.h"
#include "mac_rx_onoff.h"
#include "mac_radio.h"
#include "mac_radio_tx.h"

/* target specific */
#include "mac_radio_defs.h"
#include "mac_settings.h"

/* Activity Tracking */
#include "mac_activity.h"

/* debug */
#include "mac_assert.h"

// SW Tracer
#include "dbg.h"

#ifdef DEBUG_SW_TRACE
#define DBG_ENABLE
#ifndef _DBGFILE
#define _DBGFILE mac_rx_c
#endif
#include "dbgid_sys_mst.h"
#endif // DEBUG_SW_TRACE

#include "mac_mgmt.h"
#include "mac_data.h"

/******************************************************************************
  ROM jump table will support
  1. TIMAC ROM image build
  2. TIMAC stack build without ROM image
  3. TIMAC stack with calling ROM image
 *****************************************************************************/
#include "rom_jt_154.h"

#ifdef FEATURE_WISUN_SUPPORT
#include "fh_data.h"
#endif
/* ------------------------------------------------------------------------------------------------
 *                                            Defines
 * ------------------------------------------------------------------------------------------------
 */
#define MAX_PAYLOAD_BYTES_READ_PER_INTERRUPT   16   /* adjustable to tune performance */

/* receive FIFO bytes needed to start a valid receive (see function rxStartIsr for details) */
#define RX_THRESHOLD_START_LEN    (MAC_PHY_PHR_LEN        +  \
                                   MAC_FCF_FIELD_LEN      +  \
                                   MAC_SEQ_NUM_FIELD_LEN  +  \
                                   MAC_FCS_FIELD_LEN)

/* maximum size of addressing fields (note: command frame identifier processed as part of address) */
#define MAX_ADDR_FIELDS_LEN  ((MAC_EXT_ADDR_FIELD_LEN + MAC_PAN_ID_FIELD_LEN) * 2)

/* addressing mode reserved value */
#define ADDR_MODE_RESERVERED  1

/* length of command frame identifier */
#define CMD_FRAME_ID_LEN      1

/* PHR mode switch mask */
#define PHY_PACKET_MODE_SWITCH 0x80

/* value for promiscuous off, must not conflict with other mode variants from separate include files */
#define PROMISCUOUS_MODE_OFF  0x00

/* bit of proprietary FCS format that indicates if the CRC is OK */
#define PROPRIETARY_FCS_CRC_OK_BIT  0x80

/* bit of proprietary FCS format that indicates the packet should be rejected by frame filtering */
#define PROPRIETARY_FCS_FRAME_IGNORE  0x40

/* bit of proprietary FCS format that indicates if the CRC is OK and Ignore bit*/
#define PROPRIETARY_FCS_CRC_OK_IGNORE_BIT  0xC0

/* dummy length value for unused entry in lookup table */
#define DUMMY_LEN   0xBE

/* value for rxThresholdIntState */
#define RX_THRESHOLD_INT_STATE_INACTIVE   0
#define RX_THRESHOLD_INT_STATE_ACTIVE     1
#define RX_THRESHOLD_INT_STATE_RESET      2

/* ------------------------------------------------------------------------------------------------
 *                                             Macros
 * ------------------------------------------------------------------------------------------------
 */
#ifdef MEM_ALLOC
#undef MEM_ALLOC
#endif
#ifdef MEM_FREE
#undef MEM_FREE
#endif
#define MEM_ALLOC(x)   MAP_macDataRxMemAlloc(x)
#define MEM_FREE(x)    MAP_macDataRxMemFree((uint8 **)x)

/*
 *  Macro for encoding frame control information into internal flags format.
 *  Parameter is pointer to the frame.  NOTE!  If either the internal frame
 *  format *or* the specification changes, this macro will need to be modified.
 */
#define INTERNAL_FCF_FLAGS(p)  ((((p)[1] >> 4) & 0x03) | ((p)[0] & 0x78))

/*
 *  The radio replaces the actual FCS with different information.  This proprietary FCS is
 *  the same length as the original and includes:
 *    1) the RSSI value
 *    2) the average correlation value (used for LQI)
 *    3) a CRC passed bit
 *
 *  These macros decode the proprietary FCS.  The macro parameter is a pointer to the two byte FCS.
 */
#define PROPRIETARY_FCS_RSSI(p)                 ((int8)((p)[0]))
#define PROPRIETARY_FCS_CRC_OK(p)               ((p)[1] & PROPRIETARY_FCS_CRC_OK_BIT)
#define PROPRIETARY_FCS_CORRELATION_VALUE(p)    ((p)[1] & ~PROPRIETARY_FCS_CRC_OK_IGNORE_BIT)

/*
 *  Macros for security control field.
 */
#define SECURITY_LEVEL(s)                       (s & 0x07)
#define KEY_IDENTIFIER_MODE(s)                  ((s & 0x18) >> 3)
#define SECURITY_CONTROL_RESERVED(s)            ((s & 0xE0) >> 5)

/* ------------------------------------------------------------------------------------------------
 *                                       Global Variables
 * ------------------------------------------------------------------------------------------------
 */
volatile uint8 macRxActive;
uint8 macRxFilter;
volatile uint8 macRxOutgoingAckFlag;
uint8 rxPromiscuousMode;


#ifdef FH_BENCHMARK_BUILD_TIMES
uint16_t macDataRxParseTime[NUM_MAC_BENCHMARK_CNT];
uint8_t macDataRxParseIdx = 0;
#endif

#ifdef FH_HOP_DEBUG
MAC_SFD_DBG macSfdDbg;
#endif

/* ------------------------------------------------------------------------------------------------
 *                                       Local Constants
 * ------------------------------------------------------------------------------------------------
 */
static const uint8 CODE macRxAddrVer2Len[] =
{
  0,                         /* no address */
  DUMMY_LEN,                 /* reserved */
  MAC_SHORT_ADDR_FIELD_LEN,  /* short address */
  MAC_EXT_ADDR_FIELD_LEN     /* extended address */
};

/* ------------------------------------------------------------------------------------------------
 *                                         Local Variables
 * ------------------------------------------------------------------------------------------------
 */
macRx_t  *pRxBuf;

static uint8   rxBuf[MAC_PHY_PHR_LEN + MAC_FCF_FIELD_LEN + MAC_SEQ_NUM_FIELD_LEN];
static uint16  rxNextLen;
static uint16  rxPayloadLen;
static uint8   rxFilter;
static uint8   rxAckWithPending;
uint8  rxSeqNum;
static uint8  seqNumSuppressed;
static uint16 fcf;

#ifdef TI_WISUN_FAN_DEBUG
/* if filterMode is 0, both are disabled
 * if filterMode is 1, allowlist is enabled, if filterMode is 2, denylist is enabled. */
uint8_t filterMode = 2;

sAddrExt_t mac_eui_filter_list[SIZE_OF_EUI_LIST] = {0};
#endif
#if defined(__TI_COMPILER_VERSION__)
#pragma DATA_ALIGN(macRxDataEntryQueue, 4)
#elif defined(__IAR_SYSTEMS_ICC__)
#pragma data_alignment=4
#elif defined(__GNUC__)
__attribute__ ((aligned (4)))
#endif

/* RX data queue and data entry */
dataQueue_t   macRxDataEntryQueue;
uint16        macRxDataEntryOffset;

/* RX radio buffer pointer */
uint8        *rxRadioBufferPtr;

extern uint32 macRxSymbol;

/* ------------------------------------------------------------------------------------------------
 *                                       Local Macros and Prototypes
 * ------------------------------------------------------------------------------------------------
 */

#define MAC_RADIO_READ_RX_FIFO( buf, x )  st( \
  MAP_osal_memcpy( buf, macRxDataEntryQueue.pCurrEntry + sizeof(rfc_dataEntry_t) + macRxDataEntryOffset, x ); \
  macRxDataEntryOffset += x; \
)

#define MAC_RADIO_FLUSH_RX_FIFO()  st( \
  halIntState_t  cs; \
  /* Prepare the pNextDataEntry for the next RX frame interrupt */ \
  HAL_ENTER_CRITICAL_SECTION(cs); \
  /* mark the next System data entry as Pending */ \
  ((rfc_dataEntry_t*) (((rfc_dataEntry_t*) (macRxDataEntryQueue.pCurrEntry))->pNextEntry))->status = DATA_ENTRY_PENDING; \
  /* advance to the next data entry in the data entry queue */ \
  macRxDataEntryQueue.pCurrEntry = (uint8_t*) (((rfc_dataEntry_t*) macRxDataEntryQueue.pCurrEntry)->pNextEntry); \
  HAL_EXIT_CRITICAL_SECTION(cs); \
 )

inline static void rxStartIsr(void);
inline static void rxAddrIsr(void);

#ifdef FEATURE_MAC_SECURITY
  inline static void rxSecurityHdrIsr(void);
#endif

static void rxPayloadIsr(void);
inline static void rxFcsIsr(void);

static void rxDiscardFrame(void);
static void rxDone(void);
static void rxPostRxUpdates(void);

/**************************************************************************************************
 * @fn          macRxInit
 *
 * @brief       Initialize receive variable states. OSAL must be initialized.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macRxInit(void)
{
  rfc_dataEntryPointer_t  rxDataEntry;

  macRxFilter          = RX_FILTER_OFF;
  rxPromiscuousMode    = PROMISCUOUS_MODE_OFF;
  pRxBuf               = NULL; /* required for macRxReset() to function correctly */
  macRxActive          = MAC_RX_ACTIVE_NO_ACTIVITY;
  macRxOutgoingAckFlag = 0;
#ifdef FEATURE_WISUN_EDFE_SUPPORT
  macEdfeInfo.rxFrameType = 0;
#endif

  /* Allocate rxRadioBuffer. This is allocated once and never freed */
  rxRadioBufferPtr = MEM_ALLOC(macCfg.macMaxFrameSize + MAC_PHY_PHR_LEN +
                               MAC_FCS4_FIELD_LEN + sizeof(rfc_dataEntry_t));
  /* Abort if memory alloc fails */
  if (rxRadioBufferPtr == NULL)
  {
    MAC_ASSERT_FORCED();
  }

  /* initialize data entry header and point to self */
  rxDataEntry.pNextEntry   = rxRadioBufferPtr;
  rxDataEntry.status       = DATA_ENTRY_PENDING;
  rxDataEntry.config.type  = DATA_ENTRY_TYPE_GEN;
  rxDataEntry.config.lenSz = 0;
  rxDataEntry.config.irqIntv = 0;
  rxDataEntry.length       = macCfg.macMaxFrameSize;
  MAP_osal_memcpy(rxRadioBufferPtr, &rxDataEntry, sizeof(rfc_dataEntry_t));

  /* Setup RX data queue and RX ring buffer */
  macRxDataEntryQueue.pCurrEntry  = rxRadioBufferPtr;
  macRxDataEntryQueue.pLastEntry = NULL;

  /* Setup MAC RX command */
  macSetupReceiveCmd();

  /* Power management state may change. Hence, vote. */
  MAC_PWR_VOTE_NO_SWI(FALSE);
}


/**************************************************************************************************
 * @fn          macRxRadioPowerUpInit
 *
 * @brief       Initialization for after radio first powers up.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macRxRadioPowerUpInit(void)
{
  /* clear any accidental threshold interrupt that happened as part of power up sequence */
  MAC_RADIO_CLEAR_RX_THRESHOLD_INTERRUPT_FLAG();

  /* enable threshold interrupts */
  MAC_RADIO_ENABLE_RX_THRESHOLD_INTERRUPT();
}


/**************************************************************************************************
 * @fn          macRxHaltCleanup
 *
 * @brief       Cleanup up the receive logic after receiver is forced off.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macRxHaltCleanup(void)
{
  DBG_PRINT0(DBGSYS, "macRxHaltCleanup()");

  /* if data buffer has been allocated, free it */
  if (pRxBuf != NULL)
  {
    MEM_FREE((uint8 **)&pRxBuf);
  }

  /* flush radio queue */
  MAC_RADIO_FLUSH_RX_FIFO();

  /* if receive was active, perform the post receive updates */
  if (macRxActive || macRxOutgoingAckFlag)
  {
    macRxActive = MAC_RX_ACTIVE_NO_ACTIVITY;
    macRxOutgoingAckFlag = 0;

    rxPostRxUpdates();

    /* Power management state may change. Hence, vote. */
    MAC_PWR_VOTE_NO_SWI(FALSE);
  }
}


/**************************************************************************************************
 * @fn          macRxFrameIsr
 *
 * @brief       Interrupt service routine called when RX frame is received.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
extern void macRxFrameIsr( void )
{
  /*
   *  Call the function that handles the current receive state.
   *  The rxStartIsr is used as the exit condition.
   */
  rxStartIsr();

  /* flush radio queue */
  MAC_RADIO_FLUSH_RX_FIFO();
}

/*=================================================================================================
 * @fn          rxStartIsr
 *
 * @brief       First ISR state for receiving a packet - compute packet length, allocate
 *              buffer, initialize buffer. Acknowledgments are handled immediately without
 *              allocating a buffer.
 *
 * @param       none
 *
 * @return      none
 *=================================================================================================
 */
inline static void rxStartIsr(void)
{
  uint8  addrLen;

  uint8  dstAddrMode;
  uint8  srcAddrMode;
  uint8  mhrLen = 0;
  uint16 rxUnreadLen;
  uint8  frameVersion;
#if defined(COMBO_MAC) || !defined(FREQ_2_4G)
  bool fcsType;
#endif

  DBG_PRINT0(DBGSYS, "rxStartIsr()");

  macRxSymbol = macSymbolTimerCount();

  /* indicate rx is active */
  macRxActive = MAC_RX_ACTIVE_STARTED;

  /* clear RX data entry offset */
  macRxDataEntryOffset = 0;

  MAC_RADIO_CANCEL_ACK_TX_DONE_CALLBACK();
  macRxOutgoingAckFlag = 0;

#if defined(FEATURE_WISUN_EDFE_SUPPORT) || defined (FEATURE_WISUN_MIN_EDFE)
  macEdfeInfo.rxFrameType = 0;
#endif

  /*
   *  Make a module-local copy of macRxFilter.  This prevents the selected
   *  filter from changing in the middle of a receive.
   */
  rxFilter = macRxFilter;

  /* Reverse bit order in each byte after the header and before the FCS */
  rxUnreadLen = BUILD_UINT16(*(rxRadioBufferPtr + sizeof(rfc_dataEntry_t)),
                             *(rxRadioBufferPtr + sizeof(rfc_dataEntry_t) + 1)
                               & MAC_PHY_PHR_LEN_MSB_MASK);

  fcsType = *(rxRadioBufferPtr + sizeof(rfc_dataEntry_t) + 1) & MAC_PHY_PHR_FCS_TYPE_MASK;

  if(!fcsType)
  {
    rxUnreadLen -=  MAC_FCS4_FIELD_LEN;
  }
  else
  {
    rxUnreadLen -=  MAC_FCS2_FIELD_LEN;
  }
  /* PHR filtering */
  if ((*(rxRadioBufferPtr + sizeof(rfc_dataEntry_t) + 1) & PHY_PACKET_MODE_SWITCH) ||
      (rxUnreadLen > macCfg.macMaxFrameSize))
  {
    DBG_PRINT0(DBGSYS, "Incoming frame filtered! Mode switch set or frame size too large.");

    /* Mode switch bit is set or length too long - discard the packet and return */
    rxDiscardFrame();
    return;
  }
  
  macMcuBitReverse(rxRadioBufferPtr + sizeof(rfc_dataEntry_t) + 2, rxUnreadLen);
  
  /*-------------------------------------------------------------------------------
   *  Read initial frame information from FIFO.
   *
   *   This code is not triggered until the following are in the RX FIFO:
   *     frame length          - two bytes containing length of MAC frame (excludes this field)
   *     frame control field   - two bytes defining frame type, addressing fields, control flags
   *     sequence number       - one byte unique sequence identifier
   *     additional two bytes  - these bytes are available in case the received frame is an ACK,
   *                             if so, the frame can be verified and responded to immediately,
   *                             if not an ACK, these bytes will be processed normally
   */

  /* read frame length, frame control field, and sequence number from FIFO */
  MAC_RADIO_READ_RX_FIFO(rxBuf, MAC_PHY_PHR_LEN + MAC_FCF_FIELD_LEN + MAC_SEQ_NUM_FIELD_LEN);

  /* bytes to read from FIFO equals frame length minus length of FCF and SeqNum fields */
  /* We assume SeqNum is not suppressed here and adjust later. */
  rxUnreadLen -= MAC_FCF_FIELD_LEN + MAC_SEQ_NUM_FIELD_LEN;

  /* make local copys of FCF and seqence number */
  fcf = BUILD_UINT16(rxBuf[2], rxBuf[3]);
  rxSeqNum = MAC_SEQ_NUMBER(&rxBuf[2]);

   /* Obtain Frame Version */
  frameVersion = MAC_FRAME_VERSION(&rxBuf[2]);

  /* frame version filtering */
  if(frameVersion != MAC_FRAME_VERSION_2)
  {
    DBG_PRINT0(DBGSYS, "Incoming frame filtered! Invalid Frame Version.");
    /* discard rest of frame */
    rxDiscardFrame();
    return;
  }

  /* Discard Beacons in FH Mode */
  if (MAC_FH_ENABLED && (fcf & MAC_FCF_FRAME_TYPE_MASK) == MAC_FRAME_TYPE_BEACON)
  {
      /* discard rest of frame */
      rxDiscardFrame();
      return;
  }

  /* Discrad if a DATA packet is received in LISTEN_FOR_ACK mode */
  if ((MAC_FH_ENABLED) && (macTxActive == MAC_TX_ACTIVE_LISTEN_FOR_ACK) &&
     ((fcf & MAC_FCF_FRAME_TYPE_MASK) != MAC_FRAME_TYPE_ACK))
	 {
       /* discard rest of frame */
       rxDiscardFrame();
       return;
	 }

#ifdef FEATURE_WISUN_EDFE_SUPPORT
  /* Discard if DFE data is received in EDFE mode */
  if ((MAC_FH_ENABLED) && (macEdfeInfo.txFrameType) &&
     (fcf & MAC_FCF_ACK_REQUEST_MASK))
     {
       /* handle failure cases */
       MAP_FHDATA_compEDFEReq(FHAPI_STATUS_ERR_EDFE_DFE_RCV);
       return;
     }
#endif
  /*-------------------------------------------------------------------------------
   *  Apply filtering.
   *
   *  For efficiency, see if filtering is even 'on' before processing.  Also test
   *  to make sure promiscuous mode is disabled.  If promiscuous mode is enabled,
   *  do not apply filtering.
   */
  if ((rxFilter != RX_FILTER_OFF) && !rxPromiscuousMode)
  {
    if (/* filter all frames */
        (rxFilter == RX_FILTER_ALL) ||

        /* filter non-beacon frames */
        ((rxFilter == RX_FILTER_NON_BEACON_FRAMES) &&
         ((fcf & MAC_FCF_FRAME_TYPE_MASK) != MAC_FRAME_TYPE_BEACON)) ||

        /* filter non-command frames */
        ((rxFilter == RX_FILTER_NON_COMMAND_FRAMES) &&
         ((fcf & MAC_FCF_FRAME_TYPE_MASK) != MAC_FRAME_TYPE_COMMAND)))
    {
      /* discard rest of frame */
      rxDiscardFrame();
      return;
    }
  }

  /* Check if security properties match */
#ifdef FEATURE_MAC_SECURITY
  if ((fcf & MAC_FCF_SEC_ENABLED_MASK) &&
      (!(fcf & MAC_FCF_FRAME_VERSION_MASK) || !(pMacPib->securityEnabled)))
#else /* FEATURE_MAC_SECURITY */
  if (fcf & MAC_FCF_SEC_ENABLED_MASK)
#endif /* FEATURE_MAC_SECURITY */
  {
    /* discard rest of frame */
    rxDiscardFrame();
    return;
  }

  /* Check if frame version is proper */
  if ((((fcf & MAC_FCF_FRAME_VERSION_MASK) >> MAC_FCF_FRAME_VERSION_POS) < 2)
      && (fcf & MAC_FCF_IE_PRESENT_MASK))
  {
    /* discard rest of frame */
    rxDiscardFrame();
    return;
  }

  /*-------------------------------------------------------------------------------
   *  Compute length of addressing fields.  Compute payload length.
   */

  /* decode addressing modes */
  dstAddrMode = MAC_DEST_ADDR_MODE(&rxBuf[2]);
  srcAddrMode = MAC_SRC_ADDR_MODE(&rxBuf[2]);
  if((dstAddrMode == SADDR_MODE_RESERVED) ||(srcAddrMode == SADDR_MODE_RESERVED))
  {
    DBG_PRINT0(DBGSYS, "Incoming frame filtered! Invalid Addr Mode.");

    /* Discard the packet and return */
    rxDiscardFrame();
    return;
  }
  /*
   *  Compute the addressing field length.  A lookup table based on addressing
   *  mode is used for efficiency.  If the source address is present and the
   *  frame is intra-PAN, the PAN Id is not repeated.  In this case, the address
   *  length is adjusted to match the smaller length.
   */
  if(frameVersion == MAC_FRAME_VERSION_2)
  {
	addrLen = macRxAddrVer2Len[dstAddrMode] + macRxAddrVer2Len[srcAddrMode];

    if(fcf & MAC_FCF_INTRA_PAN_MASK)
    {
      if(srcAddrMode == SADDR_MODE_NONE && dstAddrMode == SADDR_MODE_NONE)
      {
        /* Only Destination PAN Id field is present */
        addrLen = MAC_PAN_ID_FIELD_LEN;
      }
    }
    else
    {
      if(!(srcAddrMode == SADDR_MODE_NONE && dstAddrMode == SADDR_MODE_NONE))
      {
        /* Only One PAN Id field is present */
        addrLen += MAC_PAN_ID_FIELD_LEN;
      }
    }
  }
  else
  {
      /* discard frame and exit */
      rxDiscardFrame();
      return;
  }


  /* If rxSeqNum is suppressed, seqNumSuppressed will be MAC_SEQ_NUM_FIELD_LEN,
   * otherwise 0
   */
  seqNumSuppressed = (fcf & MAC_FCF_SEQ_NO_SUPPRESS_MASK) ?
                     MAC_SEQ_NUM_FIELD_LEN : 0;

  /*
   *  For 15.4g, if there are not enough unread bytes to include the computed address
   *  plus FCS field, the frame is corrupted and must be discarded.
   */
  if ((addrLen - seqNumSuppressed) > rxUnreadLen)

  {
    /* discard frame and exit */
    rxDiscardFrame();
    return;
  }

  /* 15.4g aux security header plus payload length is equal to unread bytes minus
   * address length, minus the FCS.
   * rxUnreadLen was calculated assuming seqnum presented. If seq number is
   * suppressed, it should be compensated when calculating rxPayloadLen.
   */
  rxPayloadLen = rxUnreadLen - addrLen + seqNumSuppressed;

  /*-------------------------------------------------------------------------------
   *  Allocate memory for the incoming frame.
   */
#ifdef FEATURE_MAC_SECURITY
  if (fcf & MAC_FCF_SEC_ENABLED_MASK)
  {
    /* increase the allocation size of MAC header for security */
    mhrLen = MAC_MHR_LEN;
  }
#endif /* FEATURE_MAC_SECURITY */

  pRxBuf = (macRx_t *) MEM_ALLOC(sizeof(macRx_t) + mhrLen + rxPayloadLen);
  if (pRxBuf == NULL)
  {
    /* Do not cancel the outgoing ACK for CC13xx. The received packet
     * will be processed upon next buffer full receive.
     */
    DBG_PRINT0(DBGSYS, "Heap memory full!");

    /* buffer allocation failed, discard the frame and exit*/
    rxDiscardFrame();
    return;
  }

  memset( pRxBuf, 0x0, sizeof(macRx_t));

  /*-------------------------------------------------------------------------------
   *  Set up to process ACK request.  Do not ACK if in promiscuous mode.
   */
  rxAckWithPending = 0;
  if (!rxPromiscuousMode)
  {
    macRxOutgoingAckFlag = fcf & MAC_FCF_ACK_REQUEST_MASK;
  }
 /*-------------------------------------------------------------------------------
  *  Populate the receive buffer going up to high-level.
  */

  /* configure the payload buffer
   * save MAC header pointer regardless of security status.
   */
  pRxBuf->mhr.p = pRxBuf->msdu.p = (uint8 *) (pRxBuf + 1);
  pRxBuf->mhr.len = pRxBuf->msdu.len = rxPayloadLen;

#ifdef FEATURE_MAC_SECURITY
  if (fcf & MAC_FCF_SEC_ENABLED_MASK)
  {
    /* Copy FCF and sequence number(if exists) to RX buffer */
    pRxBuf->mhr.len = MAC_FCF_FIELD_LEN + MAC_SEQ_NUM_FIELD_LEN - seqNumSuppressed;
    MAP_osal_memcpy(pRxBuf->mhr.p, &rxBuf[2], pRxBuf->mhr.len);
    pRxBuf->mhr.p += pRxBuf->mhr.len;
  }
#endif /* FEATURE_MAC_SECURITY */

  /* copy fcf to the internal struct */
  pRxBuf->internal.fcf = fcf;

  /* compress fcf into flags together with rxAckWithPending */
  pRxBuf->internal.flags = INTERNAL_FCF_FLAGS(&rxBuf[2]) | rxAckWithPending;

  /* initialize the fh frame type to invalid */
  pRxBuf->internal.fhFrameType = MAC_FH_FRAME_INVALID;

  /* take the sequence number if not suppressed */
  pRxBuf->mac.dsn = seqNumSuppressed ? 0 : rxSeqNum;

  /* address modes */
  pRxBuf->mac.srcAddr.addrMode  = srcAddrMode;
  pRxBuf->mac.dstAddr.addrMode  = dstAddrMode;

  /* rewind the RX data queue offset by MAC_SEQ_NUM_FIELD_LEN
   * if seqnum is suppressed
   */
  macRxDataEntryOffset -= seqNumSuppressed;

  /*-------------------------------------------------------------------------------
   *  If the processing the addressing fields does not require more bytes from
   *  the FIFO go directly address processing function.  Otherwise, configure
   *  interrupt to jump there once bytes are received.
   */
  if (addrLen == 0)
  {
    /* no addressing fields to read, prepare for payload interrupts */
    rxPayloadIsr();
  }
  else
  {
    /* need to read and process addressing fields, prepare for address interrupt */
    rxNextLen = addrLen;
    rxAddrIsr();
  }
}


/*=================================================================================================
 * @fn          rxAddrIsr
 *
 * @brief       Receive ISR state for decoding address.  Reads and stores the address information
 *              from the incoming packet.
 *
 * @param       none
 *
 * @return      none
 *=================================================================================================
 */
inline static void rxAddrIsr(void)
{
  uint8 buf[MAX_ADDR_FIELDS_LEN];
  uint8 dstAddrMode;
  uint8 srcAddrMode;
  uint8 frameVersion;
  uint8 panIdCompression;
#ifdef FEATURE_MAC_SECURITY
  uint8 securityControl;
#endif /* FEATURE_MAC_SECURITY */
  uint8 *p;

  MAC_ASSERT(rxNextLen != 0); /* logic assumes at least one address byte in buffer */

  /*  read out address fields into local buffer in one shot */
  MAC_RADIO_READ_RX_FIFO(buf, rxNextLen);

  /* set pointer to buffer with addressing fields */
  p = buf;

  /* Obtain Frame Version and Pan Id Compression */
  frameVersion = MAC_FRAME_VERSION(&rxBuf[2]);
  panIdCompression = pRxBuf->internal.flags & MAC_RX_FLAG_INTRA_PAN;

  /* destination address */
  dstAddrMode = MAC_DEST_ADDR_MODE(&rxBuf[2]);
  srcAddrMode = MAC_SRC_ADDR_MODE(&rxBuf[2]);

  bool match = TRUE;
  if(frameVersion != MAC_FRAME_VERSION_2)
  {
      match = FALSE;
  }
  else /* MAC_FRAME_VERSION_2 */
  {
      if(panIdCompression)
      {
          if(dstAddrMode)
          {
              pRxBuf->mac.dstPanId = macFrmFilter.localPanID;
          }
          else
          {
              pRxBuf->mac.dstPanId = MAC_PAN_ID_BROADCAST;
          }
      }
      if(dstAddrMode)
      {
          if(!panIdCompression)
          {
              pRxBuf->mac.dstPanId = BUILD_UINT16(p[0], p[1]);
              p += MAC_PAN_ID_FIELD_LEN;
          }
          /* PANID filtering */
          match = FALSE;
          if(pRxBuf->mac.dstPanId == macFrmFilter.localPanID || pRxBuf->mac.dstPanId == MAC_PAN_ID_BROADCAST)
          {
              if(dstAddrMode == SADDR_MODE_EXT)
              {
                  sAddrExtRevCpy(pRxBuf->mac.dstAddr.addr.extAddr, p);
                  p += MAC_EXT_ADDR_FIELD_LEN;
                  /* Extended address filtering */
                  match = sAddrExtCmp(pRxBuf->mac.dstAddr.addr.extAddr, macFrmFilter.localExtAddr);
              }
              else
              {
                  pRxBuf->mac.dstAddr.addr.shortAddr = BUILD_UINT16(p[0], p[1]);
                  p += MAC_SHORT_ADDR_FIELD_LEN;
                  /* Short address filtering */
                  match = (bool)(pRxBuf->mac.dstAddr.addr.shortAddr == macFrmFilter.localShortAddr ||
                                 pRxBuf->mac.dstAddr.addr.shortAddr == MAC_SHORT_ADDR_BROADCAST);
              }
          }
      }
      pRxBuf->mac.srcPanId = pRxBuf->mac.dstPanId;
      if(srcAddrMode)
      {
          if(!panIdCompression && !dstAddrMode)
          {
              pRxBuf->mac.srcPanId = BUILD_UINT16(p[0], p[1]);
              p += MAC_PAN_ID_FIELD_LEN;
          }
          if(srcAddrMode == SADDR_MODE_EXT)
          {
              sAddrExtRevCpy(pRxBuf->mac.srcAddr.addr.extAddr, p);
              p += MAC_EXT_ADDR_FIELD_LEN;
          }
          else
          {
              pRxBuf->mac.srcAddr.addr.shortAddr = BUILD_UINT16(p[0], p[1]);
              p += MAC_SHORT_ADDR_FIELD_LEN;
          }
      }
      if(!srcAddrMode && !dstAddrMode && panIdCompression)
      {
          pRxBuf->mac.dstPanId = pRxBuf->mac.srcPanId = BUILD_UINT16(p[0], p[1]);
          p += MAC_PAN_ID_FIELD_LEN;
          match = (bool)(pRxBuf->mac.dstPanId == macFrmFilter.localPanID || pRxBuf->mac.dstPanId == MAC_PAN_ID_BROADCAST);
      }
  }

/* MAC EUI Filtering if Enabled */
#ifdef TI_WISUN_FAN_DEBUG
  if(srcAddrMode == SADDR_MODE_EXT)
  {
      if(filterMode == ALLOWLIST_ENABLED)
      {
        /* address filtering enabled, using allowlist
        * if match is not found, do not allow packet */
        if(!macRxIsMatchFound(pRxBuf->mac.srcAddr.addr.extAddr))
        {
            match = false;
        }
      }
      else if(filterMode == DENYLIST_ENABLED)
      {
        /* address filtering enabled, using denylist
        * if match is found, do not allow packet */
        if(macRxIsMatchFound(pRxBuf->mac.srcAddr.addr.extAddr))
        {
            match = false;
        }
      }
  }
#endif
  /* if the incoming frame does not pass address filtering, cleanup and return */
  if (!match)
  {
      DBG_PRINT0(DBGSYS, "Incoming frame filtered!");

      /* Cancel outgoing ACK */
      MAC_RADIO_CANCEL_TX_ACK();

      /* Clean up and return */
      macRxHaltCleanup();
      return;
  }

#ifdef FEATURE_MAC_SECURITY
  // Check fcf to see if security is enabled
  if (pRxBuf->internal.fcf & MAC_FCF_SEC_ENABLED_MASK)
  {
    uint8 keyIdMode;

    /* Copy addressing fields to RX buffer */
    MAP_osal_memcpy(pRxBuf->mhr.p, buf, rxNextLen);
    pRxBuf->mhr.p   += rxNextLen;
    pRxBuf->mhr.len += rxNextLen;

    /*-------------------------------------------------------------------------------
     *  Prepare for auxiliary security header interrupts.
     */

    /* read out security control field from FIFO (threshold set so bytes are guaranteed to be there) */
    MAC_RADIO_READ_RX_FIFO(&securityControl, MAC_SEC_CONTROL_FIELD_LEN);

    /* Copy security fields to MHR buffer */
    *pRxBuf->mhr.p   = securityControl;
    pRxBuf->mhr.p   += MAC_SEC_CONTROL_FIELD_LEN;
    pRxBuf->mhr.len += MAC_SEC_CONTROL_FIELD_LEN;

    /* store security level and key ID mode */
    pRxBuf->sec.securityLevel = SECURITY_LEVEL(securityControl);
    pRxBuf->sec.keyIdMode = keyIdMode = KEY_IDENTIFIER_MODE(securityControl);

    /* corrupted RX frame, should never occur */
    MAC_ASSERT(keyIdMode <= MAC_KEY_ID_MODE_8);

    if (pRxBuf->sec.securityLevel == MAC_SEC_LEVEL_NONE)
    {
      /* Security Enabled subfield is one but the Security Level in the header in zero
       * MAC_UNSUPPORTED_SECURITY - Cancel the outgoing TX ACK.
       * It may be too late but we have to try.
       */
      MAC_RADIO_CANCEL_TX_ACK();

      /* clean up after unsupported security */
      macRxHaltCleanup();
      return;
    }

    /* get the next RX length according to AuxLen table minus security control field.
     * The security control length is counted already.
     */
    rxNextLen = macKeySourceLen[keyIdMode] + MAC_FRAME_COUNTER_LEN;
    if (rxPayloadLen < rxNextLen)
    {
      /* Something is wrong! Halt RX and cleanup. This could happen when a
       * corrupted frame happens to have a good CRC.
       */
      MAC_RADIO_CANCEL_TX_ACK();

      /* Clean up after wrong payload length  */
      macRxHaltCleanup();
      return;
    }
    rxSecurityHdrIsr();
  }
#endif /* FEATURE_MAC_SECURITY */

#ifdef FEATURE_ENHANCED_ACK
    /* Do not check the FH enabled flag unless
     * the enhanced ACK feature is enabled.
     */
    if ((!MAC_FH_ENABLED)
    || ((MAC_FH_ENABLED) && (frameVersion <= MAC_FRAME_VERSION_1))
    )
#endif /* FEATURE_ENHANCED_ACK */
    {
      if (macRxOutgoingAckFlag && !seqNumSuppressed)
      {
          /* request a callback to macRxAckTxDoneCallback() when the ACK transmit has finished */
          MAC_RADIO_REQUEST_ACK_TX_DONE_CALLBACK();
          /* prepare and start ACK timer callback */
          macRadioSendImmAck(rxSeqNum, rxAckWithPending);
      }
    }
  /*-------------------------------------------------------------------------------
   *  Prepare for payload interrupts.
   */
  rxPayloadIsr();
}


#ifdef FEATURE_MAC_SECURITY
/*=================================================================================================
 * @fn          rxSecurityHdrIsr
 *
 * @brief       Receive ISR state for reading out and storing the auxiliary security header.
 *
 * @param       none
 *
 * @return      none
 *=================================================================================================
 */
inline static void rxSecurityHdrIsr(void)
{
  uint8 buf[MAC_FRAME_COUNTER_LEN + MAC_KEY_ID_8_LEN];

  /* read out frame counter and key ID */
  MAC_RADIO_READ_RX_FIFO(buf, rxNextLen);

  /* Incoming frame counter */
  pRxBuf->internal.frameCntr = BUILD_UINT32(buf[0], buf[1], buf[2], buf[3]);
  if (rxNextLen - MAC_FRAME_COUNTER_LEN > 0)
  {
    /* Explicit mode */
    MAP_osal_memcpy(pRxBuf->sec.keySource, &buf[MAC_FRAME_COUNTER_LEN], rxNextLen - MAC_FRAME_COUNTER_LEN - 1);
    pRxBuf->sec.keyIndex = buf[rxNextLen - MAC_KEY_INDEX_LEN];
  }

  /* Copy security fields to RX buffer */
  MAP_osal_memcpy(pRxBuf->mhr.p, buf, rxNextLen);
  pRxBuf->mhr.p   += rxNextLen;
  pRxBuf->mhr.len += rxNextLen;

  /* Update payload pointer and payload length. The rxPayloadLen includes security header length
   * and SCF byte. The security header and SCF length must be deducted from the rxPayloadLen.
   */
  rxPayloadLen    -= (rxNextLen + MAC_SEC_CONTROL_FIELD_LEN);
  pRxBuf->msdu.len = rxPayloadLen;
  pRxBuf->mhr.len += rxPayloadLen;
}
#endif /* FEATURE_MAC_SECURITY */


/*=================================================================================================
 * @fn          rxPayloadIsr
 *
 * @brief       Receive ISR state for reading out and storing the packet payload.
 *
 * @param       none
 *
 * @return      none
 *=================================================================================================
 */
static void rxPayloadIsr(void)
{
  MAC_RADIO_READ_RX_FIFO(pRxBuf->mhr.p, rxPayloadLen);
  pRxBuf->mhr.p += rxPayloadLen;
  rxFcsIsr();
}

/*=================================================================================================
 * @fn          rxFcsIsr
 *
 * @brief       Receive ISR state for handling the FCS.
 *
 * @param       none
 *
 * @return      none
 *=================================================================================================
 */

inline static void rxFcsIsr(void)
{
  uint32 timeStamp;

  int8   rssiDbm;
  uint8  corr;

#ifdef FH_BENCHMARK_BUILD_TIMES
  uint32_t startDataParseTs;
#endif

  DBG_PRINT0(DBGSYS, "rxFcsIsr()");

  /* decode RSSI  values */
  MAC_RADIO_READ_RX_FIFO(&rssiDbm, 1);
  corr = 0;
  DBG_PRINT2(DBGSYS, "RSSI = %i, Corr = %i", rssiDbm, corr);

#ifdef FH_BENCHMARK_BUILD_TIMES
  startDataParseTs = ICall_getTicks();
#endif

  /* read timestamp */
  MAC_RADIO_READ_RX_FIFO((uint8 *)&timeStamp, 4);

  /* reverse it */
  timeStamp = MAP_osal_build_uint32((uint8 *)&timeStamp, 4);

  DBG_PRINTL1(DBGSYS, "RX Time Stamp=0x%X", timeStamp);

  /* The time, in aUnitBackoffPeriod units, at which the frame was received */
#ifndef FEATURE_WISUN_SUPPORT
  pRxBuf->mac.timestamp  = MAC_RADIO_SYMBOL_CAPTURE( timeStamp );
#else
  /* This time is represented in us as expected by nano stack */
  pRxBuf->mac.timestamp  = MAP_ICall_getTicks() * ClockP_getSystemTickPeriod();
#endif

#ifndef FEATURE_WISUN_SUPPORT
  /* The time, in RAT ticks, at which the frame was received */
  pRxBuf->mac.timestamp2 = MAC_RADIO_TIMER_CAPTURE( timeStamp );
#else
  /* This time is represented in us as expected by nano stack */
  pRxBuf->mac.timestamp2 = MAP_ICall_getTicks() * ClockP_getSystemTickPeriod();
#endif

  /*
   *  The footer contains:
   *    1) the 1-byt RSSI value if rxConfig.bAppendRssi is 1
   *    2) the 4-byte timestamp indicating the start of the packet if rxConfig.bAppendTimeStamp is 1
   */

  /*
   *  See if the frame should be passed up to high-level MAC.  If the CRC is OK, the
   *  the frame is always passed up.  TODO: Frames with a bad CRC are also passed up
   *  *if* a special variant of promiscuous mode is active. Implement
   *  MAC_PROMISCUOUS_MODE_WITH_BAD_CRC by calling macRxFrameIsr even for status of
   *  MB_RX_NOK_INT
   */

#ifdef FEATURE_SYSTEM_STATS
  /* Increment diagnostic CRC success counter */
  macLowLevelDiags( MAC_DIAGS_RX_CRC_PASS );
#endif /* FEATURE_SYSTEM_STATS */


  macRxActive = MAC_RX_ACTIVE_DONE;

  /* record parameters that get passed up to high-level */
  pRxBuf->internal.flags |= ( rxAckWithPending );
  pRxBuf->mac.mpduLinkQuality = macRadioComputeLQI(rssiDbm, corr);
  pRxBuf->mac.rssi = rssiDbm;
  pRxBuf->mac.correlation = corr;

  /* set the MSDU pointer to point at start of data */
  pRxBuf->mhr.p   = (uint8 *) (pRxBuf + 1);                 //pointing to header
  pRxBuf->msdu.p += (pRxBuf->mhr.len - pRxBuf->msdu.len);   //pointing to IE

  if ( pRxBuf->internal.fcf & MAC_FCF_IE_PRESENT_MASK )
  {
    MAP_macUpdateHeaderIEPtr(pRxBuf);

#ifdef FEATURE_WISUN_EDFE_SUPPORT
    if(macEdfeInfo.txFrameType && !macEdfeInfo.rxFrameType)
    {
        MEM_FREE((uint8 **)&pRxBuf);
        pRxBuf = NULL;

        /* receive is done, exit from here */
        macRxHaltCleanup();

        MAP_FHDATA_compEDFEReq(FHAPI_STATUS_ERR_EDFE_DFE_RCV);
        return;

    }
#endif

    /* if HIE length is zero, we need to drop this packet */
    if ( (pRxBuf->pHdrIes) && (pRxBuf->headerIeLen == 0) )
    {
      MEM_FREE((uint8 **)&pRxBuf);
      pRxBuf = NULL;

      /* receive is done, exit from here */
      macRxHaltCleanup();
      return;
    }
#ifdef FEATURE_MAC_SECURITY
    pRxBuf->internal.ptrCData = pRxBuf->msdu.p;
#endif /* FEATURE_MAC_SECURITY */
  }
#ifdef FEATURE_MAC_SECURITY
  else if (pRxBuf->internal.fcf & MAC_FCF_SEC_ENABLED_MASK)
  {
    if ((pRxBuf->internal.fcf & MAC_FCF_FRAME_TYPE_MASK)
        == MAC_FRAME_TYPE_BEACON)
    {
      // Decrypt only Beacon Payload
      pRxBuf->internal.ptrCData = pRxBuf->msdu.p + MAC_PENDING_ADDR_OFFSET +
        MAC_PEND_FIELDS_LEN(pRxBuf->msdu.p[MAC_PENDING_ADDR_OFFSET]) + 1;
    }
    else if ((pRxBuf->internal.fcf & MAC_FCF_FRAME_TYPE_MASK)
             == MAC_FRAME_TYPE_COMMAND)
    {
      // Decrypt only Command Contents
      pRxBuf->internal.ptrCData = pRxBuf->msdu.p + 1;
    }
    else
    {
      // Decrypt the payload as a whole
      pRxBuf->internal.ptrCData = pRxBuf->msdu.p;
    }
  }

  if (pRxBuf->internal.fcf & MAC_FCF_SEC_ENABLED_MASK)
  {
#ifdef FEATURE_WISUN_EDFE_SUPPORT
    if((pRxBuf->mac.srcAddr.addrMode == SADDR_MODE_NONE)
    &&((macEdfeInfo.rxFrameType == MAC_FRAME_TYPE_EDFE_RFRM) || (macEdfeInfo.rxFrameType == MAC_FRAME_TYPE_EDFE_FFRM)))
    {
      MAP_osal_memcpy(&pRxBuf->mac.srcAddr, &macEdfeInfo.dstAddr, sizeof(sAddr_t));
    }
#endif

    /* Incoming frame security processing */
    pRxBuf->hdr.status = MAP_macIncomingFrameSecurity(pRxBuf);
    pRxBuf->mac.frameCntr = pRxBuf->internal.frameCntr;
#if defined ( FEATURE_SYSTEM_STATS )
    if (pRxBuf->hdr.status != MAC_SUCCESS)
    {
      macLowLevelDiags( MAC_DIAGS_RX_SEC_FAIL );
    }
#endif /* FEATURE_SYSTEM_STATS */
  }
  else
#endif /* FEATURE_MAC_SECURITY */
  {
    if (pRxBuf->payloadIePresent)
    {
      MAP_macUpdatePayloadIEPtr(pRxBuf);

      if ( (pRxBuf->mac.pPayloadIE) && (pRxBuf->mac.payloadIeLen == 0))
      {
        MEM_FREE((uint8 **)&pRxBuf);
        pRxBuf = NULL;

        /* receive is done, exit from here */
        macRxHaltCleanup();
        return;
      }
    }
  }

#ifdef FEATURE_ENHANCED_ACK

  if (MAC_FH_ENABLED)
  {
    /* Process received Enhanced ACK */
    if (((pRxBuf->internal.fcf & MAC_FCF_FRAME_TYPE_MASK) == MAC_FRAME_TYPE_ACK) &&
        (rxPromiscuousMode == PROMISCUOUS_MODE_OFF) && (pRxBuf->hdr.status == MAC_SUCCESS))
    {
      uint8_t  rslIE = 0;
      sAddrExt_t srcExtAddr;
      sAddr_t  srcAddr;

      sAddrExtCpy(srcExtAddr, pRxBuf->mac.srcAddr.addr.extAddr);
      MAP_osal_memcpy(&srcAddr, &pRxBuf->mac.srcAddr, sizeof(sAddr_t));

      macAckDbg.rxAckPkt++;
      if (FHAPI_STATUS_SUCCESS == MAC_FH_EXTRACT_HIE_FN(pRxBuf->pHdrIes, FH_IE_TYPE_RSL_IE, (uint8_t *)&rslIE))
      {
        if (pMacDataTx != NULL)
        {
          pMacDataTx->internal.rsl = rslIE;
        }
      }

      if(pRxBuf->mac.srcAddr.addrMode == SADDR_MODE_SHORT)
      {
          if(!memcmp(&pRxBuf->mac.srcAddr.addr.shortAddr, &macPib.coordShortAddress, 2))
          {
            sAddrExtCpy(srcExtAddr, macPib.coordExtendedAddress.addr.extAddr);
          }
      }

      /* see if transmit is listening for an ACK */
      if (macTxActive == MAC_TX_ACTIVE_LISTEN_FOR_ACK)
      {
        DBG_PRINT1(DBGSYS, "Enhanced ACK: Enhanced ACK received. rsl=0x%X", pMacDataTx->internal.rsl);

        macAckDbg.rxAckPktCb++;
        /* call transmit logic to indicate ACK was received */
        macTxAckReceivedCallback(rxSeqNum, pRxBuf->internal.fcf & MAC_FCF_FRAME_PENDING_MASK);
      }

      /* update Broadcast timing, if BT-IE present and packet from PARENT_EUI */
      if ((MAC_FH_ENABLED) && (MAC_FH_COMPLETE_RX_CB_FN ))
      {
        MAC_FH_COMPLETE_RX_CB_FN(pRxBuf);
      }


      // For wisun stack, strictly do not deallocate the memory pointed to by pRXBuf here.
      // This is because we pass the enhanced ACK content to Nanostack for parsing IEs present in the ack frame.
      //This memory will be freed later in api_mac.c processIncommingIcallMsg after nanostack is done parsing
      //ack frame
#ifndef FEATURE_WISUN_SUPPORT
      MEM_FREE((uint8 **)&pRxBuf);
#endif //FEATURE_WISUN_SUPPORT

      pRxBuf = NULL;

      /* receive is done, exit from here */
      rxDone();
      return;
    }
    /* Send Enhanced ACK */
    else if((macRxOutgoingAckFlag) &&
            (((pRxBuf->internal.fcf & MAC_FCF_FRAME_VERSION_MASK) >> MAC_FCF_FRAME_VERSION_POS) == MAC_FRAME_VERSION_2))
    {

        if((pRxBuf->hdr.status == MAC_SUCCESS))
        {
            DBG_PRINT0(DBGSYS, "Enhanced ACK: Sending enhanced ACK");
            MAC_RADIO_REQUEST_ACK_TX_DONE_CALLBACK();
            /* save the RSSi for RSL IE for later use */
            rxFhRsl = rssiDbm;

            macAckDbg.rxDataPktARQ++;
            #ifdef FH_BENCHMARK_BUILD_TIMES
            macDataRxParseTime[(macDataRxParseIdx++)%NUM_MAC_BENCHMARK_CNT] = ICall_getTicks() - startDataParseTs;
            #endif
            macRadioSendEnhAck(rxAckWithPending, &pRxBuf->mac.srcAddr, &pRxBuf->sec);
        }
        else
        {
            macRxHaltCleanup();
            return;
        }

    }//send enh ack
  } //if MAC_FH_ENABLED
#endif /* FEATURE_ENHANCED_ACK */

#ifdef FEATURE_WISUN_MIN_EDFE
  if (MAC_FH_ENABLED) //redundant check as this code is enabled only in FH mode
  {
        if (((pRxBuf->internal.fcf & MAC_FCF_FRAME_TYPE_MASK) == MAC_FRAME_TYPE_DATA) &&
         (rxPromiscuousMode == PROMISCUOUS_MODE_OFF) && (!macRxOutgoingAckFlag) && (macEdfeInfo.rxFrameType == MAC_FRAME_TYPE_EDFE)/* && version check ? */)
         // no need to check fcie presence here as macEDFEInfo rxFrameType is updated to non-zero only if fcie is present
        {
            if(pRxBuf->hdr.status == MAC_SUCCESS)
            {

                macRadioSendEDFEEfrm(&pRxBuf->mac.srcAddr, &pRxBuf->sec, MAC_FRAME_TYPE_EDFE_FFRM);

                #ifdef FH_HOP_DEBUG
                    FH_Dbg.numEDFEFinFrSent++;
                #endif
            }
            else
            {
                macRxHaltCleanup();
                return;
            }

        }
  }
#endif //FEATURE_WISUN_MIN_EDFE

#ifdef FEATURE_WISUN_EDFE_SUPPORT
  if (MAC_FH_ENABLED && macEdfeInfo.rxFrameType && (rxPromiscuousMode == PROMISCUOUS_MODE_OFF)
   && (((pRxBuf->internal.fcf & MAC_FCF_FRAME_VERSION_MASK) >> MAC_FCF_FRAME_VERSION_POS) == MAC_FRAME_VERSION_2))
  {
    /* Process received Enhanced ACK */
    if (pRxBuf->hdr.status == MAC_SUCCESS)
    {
          uint8_t  rslIE = 0;
          sAddrExt_t srcExtAddr;
          sAddr_t  srcAddr;

          sAddrExtCpy(srcExtAddr, pRxBuf->mac.srcAddr.addr.extAddr);
          MAP_osal_memcpy(&srcAddr, &pRxBuf->mac.srcAddr, sizeof(sAddr_t));

          if (FHAPI_STATUS_SUCCESS == MAC_FH_EXTRACT_HIE_FN(pRxBuf->pHdrIes, FH_IE_TYPE_RSL_IE, (uint8_t *)&rslIE))
          {
            if (pMacDataTx != NULL)
            {
              pMacDataTx->internal.rsl = rslIE;
            }
          }

          DBG_PRINT0(DBGSYS, "Received EDFE");
          /* save the RSSi for RSL IE for later use */
          rxFhRsl = rssiDbm;
          macRxActive = MAC_RX_ACTIVE_NO_ACTIVITY;
          macTxEDFEReplyCallback(macEdfeInfo.txFrameType);
          MAP_FHSM_event(&FH_FSM, FH_EVT_EDFE_RCV, pRxBuf);
          /* If not data frame, discard here */
          if (pRxBuf->msdu.len == 0)
          {
              /* update Broadcast timing, if BT-IE present and packet from PARENT_EUI */
              if ((MAC_FH_ENABLED) && (MAC_FH_COMPLETE_RX_CB_FN ))
              {
                MAC_FH_COMPLETE_RX_CB_FN(pRxBuf);
              }

              MEM_FREE((uint8 **)&pRxBuf);
              pRxBuf = NULL;
              return;
          }
    }
    else
    {
      macRxHaltCleanup();
      return;
    }
  }
#endif


  /* finally... execute callback function */
  if ((MAC_FH_ENABLED) && (MAC_FH_COMPLETE_RX_CB_FN))
  {
    MAC_FH_COMPLETE_RX_CB_FN(pRxBuf);
  }

  MAP_macRxCompleteCallback(pRxBuf);
  pRxBuf = NULL; /* needed to indicate buffer is no longer allocated */

  /* reset receive state, and complete receive logic */
  rxDone();
}


/*=================================================================================================
 * @fn          macRxNokIsr
 *
 * @brief       Frame received with bad CRC.
 *
 * @param       none
 *
 * @return      none
 *=================================================================================================
 */
void macRxNokIsr(void)
{
  DBG_PRINT0(DBGSYS, "macRxNokIsr()");

#ifdef FEATURE_SYSTEM_STATS
  /* Increment diagnostic CRC failure counter */
  macLowLevelDiags( MAC_DIAGS_RX_CRC_FAIL );
#endif /* FEATURE_SYSTEM_STATS */

  /*
   *  The CRC is bad so no ACK was sent.  Cancel any callback and clear the flag.
   *  (It's OK to cancel the outgoing ACK even if an ACK was not requested.  It's
   *  slightly more efficient to do so.) Note that the RX buffer is automatically
   *  flushed.
   */
  MAC_RADIO_CANCEL_ACK_TX_DONE_CALLBACK();
  macRxOutgoingAckFlag = 0;

  if (macTxActive == MAC_TX_ACTIVE_LISTEN_FOR_ACK)
  {
    /* CRC error - ACK is not received */
    macTxAckNotReceivedCallback(0);
  }
  /* flush radio queue */
  MAC_RADIO_FLUSH_RX_FIFO();

  /* complete receive logic */
  rxDone();
}

/*=================================================================================================
 * @fn          rxDone
 *
 * @brief       Common exit point for receive.
 *
 * @param       none
 *
 * @return      none
 *=================================================================================================
 */
static void rxDone(void)
{
  DBG_PRINT0(DBGSYS, "rxDone()");

  /* mark receive as inactive */
  macRxActive = MAC_RX_ACTIVE_NO_ACTIVITY;

  /* if there is no outgoing ACK, run the post receive updates */
  if (!macRxOutgoingAckFlag)
  {
    rxPostRxUpdates();
  }
}


/**************************************************************************************************
 * @fn          macRxAckTxDoneCallback
 *
 * @brief       Function called when the outoing ACK has completed transmitting.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void macRxAckTxDoneCallback(void)
{
  macRxOutgoingAckFlag = 0;

  DBG_PRINT0(DBGSYS, "macRxAckTxDoneCallback()");

  /*
   *  With certain interrupt priorities and timing conditions, it is possible this callback
   *  could be executed before the primary receive logic completes.  To prevent this, the
   *  post updates are only executed if receive logic is no longer active.  In the case the
   *  post updates are not executed here, they will execute when the main receive logic
   *  completes.
   */
  if (!macRxActive)
  {
    rxPostRxUpdates();
  }
}

#if defined(FEATURE_WISUN_EDFE_SUPPORT) || defined(FEATURE_WISUN_MIN_EDFE)
/**************************************************************************************************
 * @fn          macRxEDFETxDoneCallback
 *
 * @brief       Function called when EDFE response has completed transmitting.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void macRxEDFETxDoneCallback(void)
{
  macEdfeInfo.rxFrameType = 0;

  DBG_PRINT0(DBGSYS, "macRxEDFETxDoneCallback()");

  /*
   *  With certain interrupt priorities and timing conditions, it is possible this callback
   *  could be executed before the primary receive logic completes.  To prevent this, the
   *  post updates are only executed if receive logic is no longer active.  In the case the
   *  post updates are not executed here, they will execute when the main receive logic
   *  completes.
   */
  if (!macRxActive)
  {
    rxPostRxUpdates();
  }
}
#endif

/*=================================================================================================
 * @fn          rxPostRxUpdates
 *
 * @brief       Updates that need to be performed once receive is complete.
 *
 *              It is not fatal to execute this function if somehow receive is active.  Under
 *              certain timing/interrupt conditions a new receive may have started before this
 *              function executes.  This should happen very rarely (if it happens at all) and
 *              would cause no problems.
 *
 * @param       none
 *
 * @return      none
 *=================================================================================================
 */
static void rxPostRxUpdates(void)
{
  /* turn off receiver if permitted */
  macRxOffRequest();
  /* update the transmit power, update may have been blocked by transmit of outgoing ACK */
  macRadioUpdateTxPower();

  if (MAC_FH_ENABLED)
  {
    macSfdDetectCback(MAC_SFD_FRAME_RECEIVED);
  }

  /* initiate and transmit packet that was queued during receive */
  macTxStartQueuedFrame();

  if ((MAC_FH_ENABLED) && (MAC_FH_SEND_DATA_FN))
  {
    /* send the TX queue data */
    MAP_macSetEvent(MAC_TX_COMPLETE_TASK_EVT);
  }
}


/*=================================================================================================
 * @fn          rxDiscardFrame
 *
 * @brief       Initializes for discarding a packet.  Must be called before ACK is strobed.
 *
 * @param       none
 *
 * @return      none
 *=================================================================================================
 */
static void rxDiscardFrame(void)
{
  DBG_PRINT0(DBGSYS, "rxDiscardFrame()");

  if(MAC_FH_ENABLED)
  {
    macRxHaltCleanup();
  }
}


/*=================================================================================================
 * @fn          macRxSequenceNum
 *
 * @brief       Give access to rxSeqNum.
 *
 * @param       none
 *
 * @return      mac RX sequence number
 *=================================================================================================
 */
MAC_INTERNAL_API uint8 macRxSequenceNum(void)
{
  return (rxSeqNum);
}


#ifdef TI_WISUN_FAN_DEBUG
/*=================================================================================================
 * @fn          macRx_insertAddrIntoList
 *
 * @brief       Add eui address to either allow or deny list if possible
 *
 * @param       eui address to be added to either allow or deny list
 *
 * @return      true/false based on whether the address was able to be inserted or not
 *=================================================================================================
 */
sAddrExt_t allZeroExtAddr = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

bool macRx_insertAddrIntoList(uint8_t* euiAddress)
{
    uint8_t index = 0;
    bool retVal = false;

    for(index = 0; index < SIZE_OF_EUI_LIST; index++)
    {
        if(sAddrExtCmp(mac_eui_filter_list[index], allZeroExtAddr))
        {
            /* copy the string over if there is nothing stored there */
            sAddrExtCpy(mac_eui_filter_list[index], euiAddress);
            retVal = true;
            break;
        }
    }

    return(retVal);
}

/*=================================================================================================
 * @fn          macRx_removeAddrFromList
 *
 * @brief       Remove eui address from either allow or deny list if present
 *
 * @param       eui address to be removed from either allow or deny list
 *
 * @return      true/false based on whether the address was able to be removed or not
 *=================================================================================================
 */
bool macRx_removeAddrFromList(uint8_t* euiAddress)
{
    uint8_t index = 0;
    bool retVal = false;

    for(index = 0; index < SIZE_OF_EUI_LIST; index++)
    {
        if(sAddrExtCmp(mac_eui_filter_list[index], euiAddress))
        {
            /* copy the string over if there is nothing stored there */
            sAddrExtCpy(mac_eui_filter_list[index], allZeroExtAddr);
            retVal = true;
            break;
        }
    }

    return(retVal);
}

/*=================================================================================================
 * @fn          macRxIsMatchFound
 *
 * @brief       Pass in EUI address and traverse a list to see if the address is in the list
 *
 * @param       none
 *
 * @return      true if EUI address is in list, false if not
 *=================================================================================================
 */
bool macRxIsMatchFound(sAddrExt_t euiAddress)
{
    uint16_t index = 0;

    for(index = 0; index < SIZE_OF_EUI_LIST; index++)
    {
      if(sAddrExtCmp(euiAddress, mac_eui_filter_list[index]))
      {
          /* specified dest address found in list */
          return true;
      }
    }
    return false;
}
#endif

/**************************************************************************************************
 *                                  Compile Time Integrity Checks
 **************************************************************************************************
 */

/* check for changes to the spec that would affect the source code */
#if ((MAC_FCF_FIELD_LEN           !=  2    )   ||  \
     (MAC_FCF_FRAME_TYPE_POS      !=  0    )   ||  \
     (MAC_FCF_FRAME_PENDING_POS   !=  4    )   ||  \
     (MAC_FCF_ACK_REQUEST_POS     !=  5    )   ||  \
     (MAC_FCF_INTRA_PAN_POS       !=  6    )   ||  \
     (MAC_FCF_IE_LIST_PRESENT_POS !=  9    )   ||  \
     (MAC_FCF_DST_ADDR_MODE_POS   !=  10   )   ||  \
     (MAC_FCF_FRAME_VERSION_POS   !=  12   )   ||  \
     (MAC_FCF_SRC_ADDR_MODE_POS   !=  14   ))
#error "ERROR!  Change to the spec that requires modification of source code."
#endif

/* check for changes to the internal flags format */
#if ((MAC_RX_FLAG_VERSION      !=  0x03)  ||  \
     (MAC_RX_FLAG_ACK_PENDING  !=  0x04)  ||  \
     (MAC_RX_FLAG_SECURITY     !=  0x08)  ||  \
     (MAC_RX_FLAG_PENDING      !=  0x10)  ||  \
     (MAC_RX_FLAG_ACK_REQUEST  !=  0x20)  ||  \
     (MAC_RX_FLAG_INTRA_PAN    !=  0x40))
#error "ERROR!  Change to the internal RX flags format.  Requires modification of source code."
#endif

/* validate CRC OK bit optimization */
#if (MAC_RX_FLAG_CRC_OK != PROPRIETARY_FCS_CRC_OK_BIT)
#error "ERROR!  Optimization relies on these bits having the same position."
#endif

#if (MAC_RX_ACTIVE_NO_ACTIVITY != 0x00)
#error "ERROR! Zero is reserved value of macRxActive. Allows boolean operations, e.g !macRxActive."
#endif

#if (MAC_PROMISCUOUS_MODE_OFF != 0x00)
#error "ERROR! Zero is reserved value of rxPromiscuousMode. Allows boolean operations, e.g !rxPromiscuousMode."
#endif


/**************************************************************************************************
*/
