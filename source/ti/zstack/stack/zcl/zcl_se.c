/**************************************************************************************************
  Filename:       zcl_se.c
  Revised:        $Date: 2015-01-21 15:48:10 -0800 (Wed, 21 Jan 2015) $
  Revision:       $Revision: 41948 $

  Description:    Zigbee Cluster Library - SE (Smart Energy).


  Copyright (c) 2019, Texas Instruments Incorporated
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
**************************************************************************************************/


/**************************************************************************************************
 * INCLUDES
 */

#include "zcomdef.h"
#include "ti_zstack_config.h"
#include "rom_jt_154.h"
#include "stub_aps.h"
#include "zcl.h"
#include "zcl_general.h"
//#include "zcl_key_establish.h"
#include "zcl_se.h"


/**************************************************************************************************
 * CONSTANTS
 */

// ZCL_CLUSTER_ID_SE_DRLC:
#define ZCL_SE_DRLC_LOAD_CTRL_EVT_LEN             23
#define ZCL_SE_DRLC_CANCEL_LOAD_CTRL_EVT          12
#define ZCL_SE_DRLC_CANCEL_ALL_LOAD_CTRL_EVTS_LEN 1
#define ZCL_SE_DRLC_REPORT_EVT_STATUS_LEN         18
#define ZCL_SE_DRLC_GET_SCHEDULED_EVTS_LEN        5

// ZCL_CLUSTER_ID_SE_METERING:
#define ZCL_SE_METERING_SP_TOU_SET_LEN                 24
#define ZCL_SE_METERING_SP_TOU_SET_NO_BILL_LEN         7
#define ZCL_SE_METERING_SP_BLOCK_TIER_SET_LEN          25
#define ZCL_SE_METERING_SP_BLOCK_TIER_SET_NO_BILL_LEN  8
#define ZCL_SE_METERING_GET_PROFILE_RSP_LEN            7
#define ZCL_SE_METERING_REQ_FAST_POLL_MODE_RSP_LEN     5
#define ZCL_SE_METERING_SCHEDULE_SNAPSHOT_RSP_LEN      4
#define ZCL_SE_METERING_TAKE_SNAPSHOT_RSP_LEN          5
#define ZCL_SE_METERING_PUBLISH_SNAPSHOT_LEN           16
#define ZCL_SE_METERING_GET_SAMPLED_DATA_RSP_LEN       11
#define ZCL_SE_METERING_CFG_MIRROR_LEN                 9
#define ZCL_SE_METERING_CFG_NOTIF_SCHEME_LEN           9
#define ZCL_SE_METERING_CFG_NOTIF_FLAGS_LEN            12
#define ZCL_SE_METERING_GET_NOTIF_MSG_LEN              7
#define ZCL_SE_METERING_SUPPLY_STATUS_RSP_LEN          13
#define ZCL_SE_METERING_START_SAMPLING_RSP_LEN         2
#define ZCL_SE_METERING_GET_PROFILE_LEN                6
#define ZCL_SE_METERING_REQ_MIRROR_RSP_LEN             2
#define ZCL_SE_METERING_MIRROR_REMOVED_LEN             2
#define ZCL_SE_METERING_REQ_FAST_POLL_MODE_LEN         2
#define ZCL_SE_METERING_SCHEDULE_SNAPSHOT_LEN          6
#define ZCL_SE_METERING_SNAPSHOT_SCHEDULE_LEN          13
#define ZCL_SE_METERING_TAKE_SNAPSHOT_LEN              4
#define ZCL_SE_METERING_GET_SNAPSHOT_LEN               13
#define ZCL_SE_METERING_START_SAMPLING_LEN             13
#define ZCL_SE_METERING_GET_SAMPLED_DATA_LEN           9
#define ZCL_SE_METERING_MIRROR_REPORT_ATTR_RSP_LEN     1
#define ZCL_SE_METERING_RESET_LOAD_LIMIT_CNTR_LEN      8
#define ZCL_SE_METERING_CHANGE_SUPPLY_LEN              18
#define ZCL_SE_METERING_LOCAL_CHANGE_SUPPLY_LEN        1
#define ZCL_SE_METERING_SET_SUPPLY_STATUS_LEN          8
#define ZCL_SE_METERING_SET_UNCTRLD_FLOW_THRESHOLD_LEN 18

// ZCL_CLUSTER_ID_SE_PRICE:
#define ZCL_SE_PRICE_PUBLISH_PRICE_LEN               47
#define ZCL_SE_PRICE_PUBLISH_PRICE_OLD_LEN           42
#define ZCL_SE_PRICE_PUBLISH_PRICE_SEG1_LEN          5
#define ZCL_SE_PRICE_PUBLISH_PRICE_SEG2_LEN          42
#define ZCL_SE_PRICE_PUBLISH_BLOCK_PERIOD_LEN        19
#define ZCL_SE_PRICE_PUBLISH_BLOCK_PERIOD_OLD_LEN    17
#define ZCL_SE_PRICE_PUBLISH_CONVERSION_FACTOR_LEN   13
#define ZCL_SE_PRICE_PUBLISH_CALORIFIC_VALUE_LEN     14
#define ZCL_SE_PRICE_PUBLISH_TARIFF_INFO_LEN         35
#define ZCL_SE_PRICE_PUBLISH_TARIFF_INFO_SEG1_LEN    18
#define ZCL_SE_PRICE_PUBLISH_TARIFF_INFO_SEG2_LEN    17
#define ZCL_SE_PRICE_MATRIX_SUB_FLD_LEN              5
#define ZCL_SE_PRICE_PUBLISH_PRICE_MATRIX_LEN        19
#define ZCL_SE_PRICE_BLOCK_THRESHOLD_SUB_FLD_LEN     1
#define ZCL_SE_PRICE_PUBLISH_BLOCK_THRESHOLD_LEN     19
#define ZCL_SE_PRICE_PUBLISH_CO2_VALUE_LEN           19
#define ZCL_SE_PRICE_TIER_LABEL_LEN                  2
#define ZCL_SE_PRICE_PUBLISH_TIER_LABELS_LEN         15
#define ZCL_SE_PRICE_PUBLISH_BILL_PERIOD_LEN         17
#define ZCL_SE_PRICE_PUBLISH_CONSOLIDATED_BILL_LEN   24
#define ZCL_SE_PRICE_PUBLISH_CPP_EVT_LEN             17
#define ZCL_SE_PRICE_PUBLISH_CREDIT_PAYMENT_LEN      26
#define ZCL_SE_PRICE_PUBLISH_CURRENCY_CONVERSION_LEN 25
#define ZCL_SE_PRICE_CANCEL_TARIFF_LEN               9
#define ZCL_SE_PRICE_GET_CURR_PRICE_LEN              1
#define ZCL_SE_PRICE_GET_SCHEDULED_PRICE_LEN         5
#define ZCL_SE_PRICE_PRICE_ACK_LEN                   13
#define ZCL_SE_PRICE_GET_BLOCK_PERIOD_LEN            6
#define ZCL_SE_PRICE_GET_BLOCK_PERIOD_OLD_LEN        5
#define ZCL_SE_PRICE_GET_CONVERSION_FACTOR_LEN       9
#define ZCL_SE_PRICE_GET_CONVERSION_FACTOR_OLD_LEN   8
#define ZCL_SE_PRICE_GET_CALORIFIC_VALUE_LEN         9
#define ZCL_SE_PRICE_GET_CALORIFIC_VALUE_OLD_LEN     5
#define ZCL_SE_PRICE_GET_TARIFF_INFO_LEN             10
#define ZCL_SE_PRICE_GET_PRICE_MATRIX_LEN            4
#define ZCL_SE_PRICE_GET_BLOCK_THRESHOLDS_LEN        4
#define ZCL_SE_PRICE_GET_CO2_VALUE_LEN               10
#define ZCL_SE_PRICE_GET_TIER_LABELS_LEN             4
#define ZCL_SE_PRICE_GET_BILL_PERIOD_LEN             10
#define ZCL_SE_PRICE_GET_CONSOLIDATED_BILL_LEN       10
#define ZCL_SE_PRICE_CPP_EVT_RSP_LEN                 5
#define ZCL_SE_PRICE_GET_CREDIT_PAYMENT_LEN          5

// ZCL_CLUSTER_ID_SE_MESSAGING:
#define ZCL_SE_MESSAGING_DISPLAY_MSG_LEN          13
#define ZCL_SE_MESSAGING_DISPLAY_MSG_OLD_LEN      12
#define ZCL_SE_MESSAGING_CANCEL_MSG_LEN           5
#define ZCL_SE_MESSAGING_CANCEL_ALL_MSGS_LEN      4
#define ZCL_SE_MESSAGING_MSG_CFM_LEN              10
#define ZCL_SE_MESSAGING_MSG_CFM_OLD_LEN          8
#define ZCL_SE_MESSAGING_GET_MSG_CANCELLATION_LEN 4

// ZCL_CLUSTER_ID_SE_TUNNELING:
#define ZCL_SE_TUNNELING_REQ_TUNNEL_RSP_LEN            5
#define ZCL_SE_TUNNELING_TRANSFER_DATA_LEN             2
#define ZCL_SE_TUNNELING_TRANSFER_DATA_ERR_LEN         3
#define ZCL_SE_TUNNELING_ACK_TRANSFER_DATA_LEN         4
#define ZCL_SE_TUNNELING_READY_DATA_LEN                4
#define ZCL_SE_TUNNELING_SUPP_TUNNEL_PROTOCOLS_RSP_LEN 2
#define ZCL_SE_TUNNELING_TUNNEL_CLOSURE_NOTIF_LEN      2
#define ZCL_SE_TUNNELING_REQ_TUNNEL_LEN                6
#define ZCL_SE_TUNNELING_CLOSE_TUNNEL_LEN              2
#define ZCL_SE_TUNNELING_GET_SUPP_TUNNEL_PROTOCOLS_LEN 1
#define ZCL_SE_TUNNELING_PROTOCOL_PAYLOAD_LEN          3

// ZCL_CLUSTER_ID_SE_PREPAYMENT:
#define ZCL_SE_PREPAYMENT_DEBT_CREDIT_STATUS_LEN           24
#define ZCL_SE_PREPAYMENT_PUBLISH_PREPAY_SNAPSHOT_LEN      16
#define ZCL_SE_PREPAYMENT_CHANGE_PAYMENT_MODE_RSP_LEN      13
#define ZCL_SE_PREPAYMENT_CONSUMER_TOP_UP_RSP_LEN          10
#define ZCL_SE_PREPAYMENT_TOP_UP_LEN                       9
#define ZCL_SE_PREPAYMENT_PUBLISH_TOP_UP_LOG_LEN           2
#define ZCL_SE_PREPAYMENT_DEBT_REC_LEN                     13
#define ZCL_SE_PREPAYMENT_PUBLISH_DEBT_LOG_LEN             2
#define ZCL_SE_PREPAYMENT_SEL_AVAIL_EMRGNCY_CREDIT_LEN     5
#define ZCL_SE_PREPAYMENT_CHANGE_DEBT_LEN                  24
#define ZCL_SE_PREPAYMENT_CHANGE_DEBT_SEG1_LEN             5
#define ZCL_SE_PREPAYMENT_CHANGE_DEBT_SEG2_LEN             19
#define ZCL_SE_PREPAYMENT_EMRGNCY_CREDIT_SETUP_LEN         16
#define ZCL_SE_PREPAYMENT_CONSUMER_TOP_UP_LEN              2
#define ZCL_SE_PREPAYMENT_CREDIT_ADJ_LEN                   13
#define ZCL_SE_PREPAYMENT_CHANGE_PAYMENT_MODE_LEN          18
#define ZCL_SE_PREPAYMENT_GET_PREPAY_SNAPSHOT_LEN          13
#define ZCL_SE_PREPAYMENT_GET_TOP_UP_LOG_LEN               5
#define ZCL_SE_PREPAYMENT_SET_LOW_CREDIT_WARN_LEVEL_LEN    4
#define ZCL_SE_PREPAYMENT_GET_DEBT_REPAY_LOG_LEN           6
#define ZCL_SE_PREPAYMENT_SET_MAX_CREDIT_LIMIT_LEN         20
#define ZCL_SE_PREPAYMENT_SET_OVERALL_DEBT_CAP_LEN         16

// ZCL_CLUSTER_ID_SE_CALENDAR:
#define ZCL_SE_CALENDAR_PUBLISH_CALENDAR_LEN     22
#define ZCL_SE_CALENDAR_DAY_ENTRY_LEN            3
#define ZCL_SE_CALENDAR_PUBLISH_DAYPROFILE_LEN   17
#define ZCL_SE_CALENDAR_PUBLISH_WEEK_PROFILE_LEN 20
#define ZCL_SE_CALENDAR_SEASON_ENTRY_LEN         5
#define ZCL_SE_CALENDAR_PUBLISH_SEASONS_LEN      14
#define ZCL_SE_CALENDAR_SPECIAL_DAY_ENTRY_LEN    5
#define ZCL_SE_CALENDAR_PUBLISH_SPECIAL_DAYS_LEN 20
#define ZCL_SE_CALENDAR_CANCEL_CALENDAR_LEN      9
#define ZCL_SE_CALENDAR_GET_CALENDAR_LEN         14
#define ZCL_SE_CALENDAR_GET_DAY_PROFILES_LEN     10
#define ZCL_SE_CALENDAR_GET_WEEK_PROFILES_LEN    10
#define ZCL_SE_CALENDAR_GET_SEASONS_LEN          8
#define ZCL_SE_CALENDAR_GET_SPECIAL_DAYS_LEN     14

// ZCL_CLUSTER_ID_SE_DEVICE_MGMT:
#define ZCL_SE_DEVICE_MGMT_PUBLISH_CHANGE_OF_TENANCY_LEN  17
#define ZCL_SE_DEVICE_MGMT_PUBLISH_CHANGE_OF_SUPPLIER_LEN 23
#define ZCL_SE_DEVICE_MGMT_REQ_NEW_PW_RSP_LEN             12
#define ZCL_SE_DEVICE_MGMT_UPDATE_SITE_ID_LEN             13
#define ZCL_SE_DEVICE_MGMT_EVT_CFG_LIST_LEN               1
#define ZCL_SE_DEVICE_MGMT_EVT_ID_LEN                     2
#define ZCL_SE_DEVICE_MGMT_SET_EVT_CFG_LEN                8
#define ZCL_SE_DEVICE_MGMT_GET_EVT_CFG_LEN                2
#define ZCL_SE_DEVICE_MGMT_UPDATE_CIN_LEN                 13
#define ZCL_SE_DEVICE_MGMT_REQ_NEW_PW_LEN                 1
#define ZCL_SE_DEVICE_MGMT_EVT_LEN                        3
#define ZCL_SE_DEVICE_MGMT_REPORT_EVT_CFG_LEN             2

// ZCL_CLUSTER_ID_SE_EVENTS:
#define ZCL_SE_EVENTS_PUBLISH_EVT_LEN       9
#define ZCL_SE_EVENTS_PUBLISH_EVT_LOG_LEN   4
#define ZCL_SE_EVENTS_EVT_LOG_HDR_LEN       1
#define ZCL_SE_EVENTS_LOGGED_EVT_LEN        8
#define ZCL_SE_EVENTS_CLEAR_EVT_LOG_RSP_LEN 1
#define ZCL_SE_EVENTS_GET_EVT_LOG_LEN       14
#define ZCL_SE_EVENTS_CLEAR_EVT_LOG_LEN     1
#define ZCL_SE_EVENTS_LOG_ID_MASK           0x0F
#define ZCL_SE_EVENTS_GET_EVT_CTRL_MASK     0xF0
#define ZCL_SE_EVENTS_GET_EVT_CTRL_SHIFT    4

// ZCL_CLUSTER_ID_SE_ENERGY_MGMT:
#define ZCL_SE_ENERGY_MGMT_REPORT_EVT_STATUS_LEN 17
#define ZCL_SE_ENERGY_MGMT_MANAGE_EVT_LEN        8

// ZCL_CLUSTER_ID_SE_MDU_PAIRING:
#define ZCL_SE_MDU_PAIRING_PAIRING_RSP_LEN 7
#define ZCL_SE_MDU_PAIRING_PAIRING_REQ_LEN 12


/**************************************************************************************************
 * TYPEDEFS
 */

typedef struct zclSE_CBRecType
{
  struct zclSE_CBRecType  *pNext;
  uint8_t                   appEP;
  zclSE_AppCallbacks_t    *pCBs;
} zclSE_CBRec_t;


/**************************************************************************************************
 * FUNCTION PROTOTYPES
 */


/**************************************************************************************************
 * LOCAL VARIABLES
 */

static zclSE_CBRec_t *zcl_SECBs = (zclSE_CBRec_t *)NULL;
static uint8_t zclSE_PluginRegisted = FALSE;
static uint8_t* se_buffer_uint24( uint8_t *buf, uint24 val );

/**************************************************************************************************
 * LOCAL FUNCTIONS
 */

/**************************************************************************************************
 * @fn      zclSE_UTF8StringLen
 *
 * @brief   Called to get the length of UTF8String.
 *
 * @param   pUTF8 - pointer to the UTF8String_t
 *
 * @return  uint8_t - string length
 */
static uint8_t zclSE_UTF8StringLen( UTF8String_t *pUTF8 )
{
  uint8_t len = 0;

  // Check for string length
  if ( ( pUTF8->strLen != 0 ) && ( pUTF8->strLen != 0xFF ) )
  {
    // Check for string data
    if ( pUTF8->pStr != NULL )
    {
      len = pUTF8->strLen;
    }
  }

  return len;
}

/**************************************************************************************************
 * @fn      zclSE_UTF8StringBuild
 *
 * @brief   Called to build a UTF8String.
 *
 * @param   pBuf - output buffer
 * @param   pUTF8 - pointer to the UTF8String_t
 *
 * @return  uint8_t * - advanced output buffer
 */
static uint8_t* zclSE_UTF8StringBuild( uint8_t *pBuf, UTF8String_t *pUTF8 )
{
  // Write the string length
  *pBuf++ = pUTF8->strLen;

  // Check for string length and valid string data
  if ( ( pUTF8->strLen != 0 ) && ( pUTF8->strLen != 0xFF ) && pUTF8->pStr )
  {
    // Write the string data
    pBuf = OsalPort_memcpy( pBuf, pUTF8->pStr, pUTF8->strLen );
  }

  return pBuf;
}

/**************************************************************************************************
 * @fn      zclSE_UTF8StringParse
 *
 * @brief   Called to parse a UTF8String from a message.
 *
 * @param   pBuf - input buffer
 * @param   pBufEnd - input buffer end
 * @param   pUTF8 - pointer to the UTF8String_t
 * @param   pExceeded - flag indicating string length exceeds input buffer end
 *
 * @return  uint8_t * - advanced input buffer
 */
static uint8_t* zclSE_UTF8StringParse( uint8_t *pBuf, uint8_t* pBufEnd, UTF8String_t *pUTF8,
                                     uint8_t *pExceeded )
{
  pUTF8->strLen = *pBuf++;

  if ( pUTF8->strLen == 0xFF )
  {
    pUTF8->strLen = 0;
  }

  if ( pUTF8->strLen != 0 )
  {
    pUTF8->pStr = pBuf;
  }
  else
  {
    pUTF8->pStr = NULL;
  }

  // Check for valid string length
  if ( ( pBufEnd - pBuf ) < pUTF8->strLen )
  {
    pBuf = pBufEnd;

    *pExceeded = TRUE;
  }
  else
  {
    pBuf = pBuf + pUTF8->strLen;

    *pExceeded = FALSE;
  }

  return pBuf;
}

/**************************************************************************************************
 * @fn      zclSE_FindCallbacks
 *
 * @brief   Find the callbacks for an application endpoint.
 *
 * @param   appEP - application endpoint
 *
 * @return  pointer to zclSE_AppCallbacks_t or NULL
 */
static zclSE_AppCallbacks_t *zclSE_FindCallbacks( uint8_t appEP )
{
  zclSE_CBRec_t *pRec;

  pRec = zcl_SECBs;
  while ( pRec )
  {
    if ( pRec->appEP == appEP )
    {
      return ( pRec->pCBs );
    }

    pRec = pRec->pNext;
  }

  return ( (zclSE_AppCallbacks_t *)NULL );
}

/**************************************************************************************************
 * @fn      zclSE_HdlIncoming
 *
 * @brief   Callback from ZCL to process incoming Commands specific
 *          to this cluster library or Profile commands for attributes
 *          that aren't in the attribute list
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclSE_HdlIncoming( zclIncoming_t *pInMsg )
{
  ZStatus_t status = ZCL_STATUS_SUCCESS;

#if defined ( INTER_PAN ) || defined ( BDB_TL_INITIATOR ) || defined ( BDB_TL_TARGET )
  if ( StubAPS_InterPan( pInMsg->msg->srcAddr.panId, pInMsg->msg->srcAddr.endPoint ) &&
       !( pInMsg->msg->clusterId == ZCL_CLUSTER_ID_SE_PRICE ||
          pInMsg->msg->clusterId == ZCL_CLUSTER_ID_SE_MESSAGING )                       )
  {
    return status; // Cluster not supported thru Inter-PAN
  }
#endif
  if ( zcl_ClusterCmd( pInMsg->hdr.fc.type ) )
  {
    // Check for manufacturer specific command
    if ( pInMsg->hdr.fc.manuSpecific == 0 )
    {
      status = zclSE_HdlSpecificCmdHook( pInMsg );
    }
    else
    {
      // Manufacturer specific command not handled
      status = ZCL_STATUS_FAILURE;
    }
  }
  else
  {
    // Should never get here
    status = ZCL_STATUS_FAILURE;
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_HdlSpecificServerCmd
 *
 * @brief   Process Client-to-Server Commands.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCBs - callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclSE_HdlSpecificServerCmd( zclIncoming_t *pInMsg,
                                             zclSE_AppCallbacks_t *pCBs )
{
  ZStatus_t status;

  // Process Client-to-Server Commands
  switch ( pInMsg->msg->clusterId )
  {
#ifdef ZCL_SE_PRICE_SERVER
    case ZCL_CLUSTER_ID_SE_PRICE:
      status = zclSE_PriceHdlServerCmd( pInMsg, pCBs->pPriceServerCBs );
      break;
#endif

#ifdef ZCL_SE_DRLC_SERVER
    case ZCL_CLUSTER_ID_SE_DRLC:
      status = zclSE_DRLC_HdlServerCmd( pInMsg, pCBs->pDRLC_ServerCBs );
      break;
#endif

#ifdef ZCL_SE_METERING_SERVER
    case ZCL_CLUSTER_ID_SE_METERING:
      status = zclSE_MeteringHdlServerCmd( pInMsg, pCBs->pMeteringServerCBs );
      break;
#endif

#ifdef ZCL_SE_MESSAGING_SERVER
    case ZCL_CLUSTER_ID_SE_MESSAGING:
      status = zclSE_MessagingHdlServerCmd( pInMsg, pCBs->pMessagingServerCBs );
      break;
#endif

#ifdef ZCL_SE_TUNNELING_SERVER
    case ZCL_CLUSTER_ID_SE_TUNNELING:
      status = zclSE_TunnelingHdlServerCmd( pInMsg, pCBs->pTunnelingServerCBs );
      break;
#endif

#ifdef ZCL_SE_PREPAYMENT_SERVER
    case ZCL_CLUSTER_ID_SE_PREPAYMENT:
      status = zclSE_PrepaymentHdlServerCmd( pInMsg, pCBs->pPrepaymentServerCBs );
      break;
#endif

#ifdef ZCL_SE_ENERGY_MGMT_SERVER
    case ZCL_CLUSTER_ID_SE_ENERGY_MGMT:
      status = zclSE_EnergyMgmtHdlServerCmd( pInMsg, pCBs->pEnergyMgmtServerCBs );
      break;
#endif

#ifdef ZCL_SE_CALENDAR_SERVER
    case ZCL_CLUSTER_ID_SE_CALENDAR:
      status = zclSE_CalendarHdlServerCmd( pInMsg, pCBs->pCalendarServerCBs );
      break;
#endif

#ifdef ZCL_SE_DEVICE_MGMT_SERVER
    case ZCL_CLUSTER_ID_SE_DEVICE_MGMT:
      status = zclSE_DeviceMgmtHdlServerCmd( pInMsg, pCBs->pDeviceMgmtServerCBs );
      break;
#endif

#ifdef ZCL_SE_EVENTS_SERVER
    case ZCL_CLUSTER_ID_SE_EVENTS:
      status = zclSE_EventsHdlServerCmd( pInMsg, pCBs->pEventsServerCBs );
      break;
#endif

#ifdef ZCL_SE_MDU_PAIRING_SERVER
    case ZCL_CLUSTER_ID_SE_MDU_PAIRING:
      status = zclSE_MDUPairingHdlServerCmd( pInMsg, pCBs->pMDUPairingServerCBs );
      break;
#endif

    default:
      (void) pCBs;
      status = ZCL_STATUS_FAILURE;
      break;
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_HdlSpecificClientCmd
 *
 * @brief   Process Server-to-Client Commands.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCBs - callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclSE_HdlSpecificClientCmd( zclIncoming_t *pInMsg,
                                             zclSE_AppCallbacks_t *pCBs )
{
  ZStatus_t status;

  // Process Server-to-Client Commands
  switch ( pInMsg->msg->clusterId )
  {
#ifdef ZCL_SE_PRICE_CLIENT
    case ZCL_CLUSTER_ID_SE_PRICE:
      status = zclSE_PriceHdlClientCmd( pInMsg, pCBs->pPriceClientCBs );
      break;
#endif

#ifdef ZCL_SE_DRLC_CLIENT
    case ZCL_CLUSTER_ID_SE_DRLC:
      status = zclSE_DRLC_HdlClientCmd( pInMsg, pCBs->pDRLC_ClientCBs );
      break;
#endif

#ifdef ZCL_SE_METERING_CLIENT
    case ZCL_CLUSTER_ID_SE_METERING:
      status = zclSE_MeteringHdlClientCmd( pInMsg, pCBs->pMeteringClientCBs );
      break;
#endif

#ifdef ZCL_SE_MESSAGING_CLIENT
    case ZCL_CLUSTER_ID_SE_MESSAGING:
      status = zclSE_MessagingHdlClientCmd( pInMsg, pCBs->pMessagingClientCBs );
      break;
#endif

#ifdef ZCL_SE_TUNNELING_CLIENT
    case ZCL_CLUSTER_ID_SE_TUNNELING:
      status = zclSE_TunnelingHdlClientCmd( pInMsg, pCBs->pTunnelingClientCBs );
      break;
#endif

#ifdef ZCL_SE_PREPAYMENT_CLIENT
    case ZCL_CLUSTER_ID_SE_PREPAYMENT:
      status = zclSE_PrepaymentHdlClientCmd( pInMsg, pCBs->pPrepaymentClientCBs );
      break;
#endif

#ifdef ZCL_SE_ENERGY_MGMT_CLIENT
    case ZCL_CLUSTER_ID_SE_ENERGY_MGMT:
      status = zclSE_EnergyMgmtHdlClientCmd( pInMsg, pCBs->pEnergyMgmtClientCBs );
      break;
#endif

#ifdef ZCL_SE_CALENDAR_CLIENT
    case ZCL_CLUSTER_ID_SE_CALENDAR:
      status = zclSE_CalendarHdlClientCmd( pInMsg, pCBs->pCalendarClientCBs );
      break;
#endif

#ifdef ZCL_SE_DEVICE_MGMT_CLIENT
    case ZCL_CLUSTER_ID_SE_DEVICE_MGMT:
      status = zclSE_DeviceMgmtHdlClientCmd( pInMsg, pCBs->pDeviceMgmtClientCBs );
      break;
#endif

#ifdef ZCL_SE_EVENTS_CLIENT
    case ZCL_CLUSTER_ID_SE_EVENTS:
      status = zclSE_EventsHdlClientCmd( pInMsg, pCBs->pEventsClientCBs );
      break;
#endif

#ifdef ZCL_SE_MDU_PAIRING_CLIENT
    case ZCL_CLUSTER_ID_SE_MDU_PAIRING:
      status = zclSE_MDUPairingHdlClientCmd( pInMsg, pCBs->pMDUPairingClientCBs );
      break;
#endif

    default:
      (void) pCBs;
      status = ZCL_STATUS_FAILURE;
      break;
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_MeteringSP_ParseTOU
 *
 * @brief   Called to parse a publish snapshot payload.
 *
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclSE_MeteringSP_ParseTOU( zclSE_MeteringPublishSnapshot_t *pCmd )
{
  uint8_t *pBuf = pCmd->pRawPayload;
  zclSE_MeteringTOU_Set_t *pTOU;
  uint16_t usedLen;

  // Check for minimum packet length
  if ( pCmd->rawPayloadLen < ZCL_SE_METERING_SP_TOU_SET_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  // Allocate memory for the snapshot data
  pTOU = OsalPort_malloc( sizeof( zclSE_MeteringTOU_Set_t ) );
  if ( pTOU == NULL )
  {
    return ZCL_STATUS_SOFTWARE_FAILURE;
  }

  OsalPort_memcpy( pTOU->currSumm, pBuf, 6 );
  pBuf += 6;
  pTOU->billToDate = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pTOU->billToDateTimeStamp = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pTOU->projBill = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pTOU->projBillTimeStamp = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pTOU->billTrailingDigit = *pBuf++ >> 4;
  pTOU->numOfTiersInUse = *pBuf++;
  pTOU->pTierSumm = pBuf;

  usedLen = ZCL_SE_METERING_SP_TOU_SET_LEN + ( pTOU->numOfTiersInUse * 6 );

  // Check for valid summation length
  if ( pCmd->rawPayloadLen < usedLen )
  {
    OsalPort_free( pTOU );

    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->payload.pTOU = pTOU;

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_MeteringSP_ParseTOU_NoBill
 *
 * @brief   Called to parse a publish snapshot payload.
 *
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclSE_MeteringSP_ParseTOU_NoBill(
                   zclSE_MeteringPublishSnapshot_t *pCmd )
{
  uint8_t *pBuf = pCmd->pRawPayload;
  zclSE_MeteringTOU_NoBillInfo_t *pTOU;
  uint16_t usedLen;

  // Check for minimum packet length
  if ( pCmd->rawPayloadLen < ZCL_SE_METERING_SP_TOU_SET_NO_BILL_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  // Allocate memory for the snapshot data
  pTOU = OsalPort_malloc( sizeof( zclSE_MeteringTOU_NoBillInfo_t ) );
  if ( pTOU == NULL )
  {
      return ZCL_STATUS_SOFTWARE_FAILURE;
  }

  OsalPort_memcpy( pTOU->currSumm, pBuf, 6 );
  pBuf += 6;
  pTOU->numOfTiersInUse = *pBuf++;
  pTOU->pTierSumm = pBuf;

  usedLen = ZCL_SE_METERING_SP_TOU_SET_NO_BILL_LEN + ( pTOU->numOfTiersInUse * 6 );

  // Check for valid summation length
  if ( pCmd->rawPayloadLen < usedLen )
  {
    OsalPort_free( pTOU );

    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->payload.pTOU_NoBill = pTOU;

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_MeteringSP_ParseBlockTier
 *
 * @brief   Called to parse a publish snapshot payload.
 *
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclSE_MeteringSP_ParseBlockTier( zclSE_MeteringPublishSnapshot_t *pCmd )
{
  uint8_t *pBuf = pCmd->pRawPayload;
  zclSE_MeteringBlockTierSet_t *pBlockTier;
  uint16_t usedLen;

  // Check for minimum packet length
  if ( pCmd->rawPayloadLen < ZCL_SE_METERING_SP_BLOCK_TIER_SET_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  // Allocate memory for the snapshot data
  pBlockTier = OsalPort_malloc( sizeof( zclSE_MeteringBlockTierSet_t ) );
  if ( pBlockTier == NULL )
  {
      return ZCL_STATUS_SOFTWARE_FAILURE;
  }

  OsalPort_memcpy( pBlockTier->currSumm, pBuf, 6 );
  pBuf += 6;
  pBlockTier->billToDate = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pBlockTier->billToDateTimeStamp = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pBlockTier->projBill = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pBlockTier->projBillTimeStamp = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pBlockTier->billTrailingDigit = *pBuf++ >> 4;
  pBlockTier->numOfTiersInUse =  *pBuf++;
  pBlockTier->pTierSumm = pBuf;

  usedLen = ZCL_SE_METERING_SP_BLOCK_TIER_SET_LEN +
            ( pBlockTier->numOfTiersInUse * 6 );

  // Check for valid summation length
  if ( pCmd->rawPayloadLen < usedLen )
  {
    OsalPort_free( pBlockTier );

    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pBuf = pBuf + usedLen;

  pBlockTier->tierBlockNumOfTiers = *pBuf >> 4;
  pBlockTier->tierBlockNumOfBlocks =  *pBuf++ & 0x0F;
  pBlockTier->pTierBlockSumm = pBuf;

  usedLen += ( pBlockTier->tierBlockNumOfTiers *
               pBlockTier->tierBlockNumOfBlocks * 6 );

  // Check for valid summation length
  if ( pCmd->rawPayloadLen < usedLen )
  {
    OsalPort_free( pBlockTier );

    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->payload.pBlockTier = pBlockTier;

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_MeteringSP_ParseBlockTierNoBill
 *
 * @brief   Called to parse a publish snapshot payload.
 *
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclSE_MeteringSP_ParseBlockTierNoBill(
                   zclSE_MeteringPublishSnapshot_t *pCmd )
{
  uint8_t *pBuf = pCmd->pRawPayload;
  zclSE_MeteringBlockTierSetNoBillInfo_t *pBlockTier;
  uint16_t usedLen;

  // Check for minimum packet length
  if ( pCmd->rawPayloadLen < ZCL_SE_METERING_SP_BLOCK_TIER_SET_NO_BILL_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  // Allocate memory for the snapshot data
  pBlockTier = OsalPort_malloc( sizeof( zclSE_MeteringBlockTierSetNoBillInfo_t ) );
  if ( pBlockTier == NULL )
  {
      return ZCL_STATUS_SOFTWARE_FAILURE;
  }

  OsalPort_memcpy( pBlockTier->currSumm, pBuf, 6 );
  pBuf += 6;
  pBlockTier->numOfTiersInUse = *pBuf++;
  pBlockTier->pTierSumm = pBuf;

  usedLen = ZCL_SE_METERING_SP_BLOCK_TIER_SET_NO_BILL_LEN +
            ( pBlockTier->numOfTiersInUse * 6 );

  // Check for valid summation length
  if ( pCmd->rawPayloadLen < usedLen )
  {
    OsalPort_free( pBlockTier );

    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pBuf = pBuf + usedLen;

  pBlockTier->tierBlockNumOfTiers = *pBuf >> 4;
  pBlockTier->tierBlockNumOfBlocks = *pBuf++ & 0x0F;
  pBlockTier->pTierBlockSumm = pBuf;

  usedLen += ( pBlockTier->tierBlockNumOfTiers *
               pBlockTier->tierBlockNumOfBlocks * 6 );

  // Check for valid summation length
  if ( pCmd->rawPayloadLen < usedLen )
  {
    OsalPort_free( pBlockTier );

    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->payload.pBlockTierNoBill = pBlockTier;

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_MeteringSP_Len
 *
 * @brief   Called to get the length of snapshot payload.
 *
 * @param   pCmd - command payload
 *
 * @return  uint16_t - length
 */
static uint16_t zclSE_MeteringSP_Len( zclSE_MeteringPublishSnapshot_t *pCmd )
{
  uint16_t len = 0;

  // Check for a non fragmented, valid "payload", then calculate length
  if ( ( pCmd->cmdTotal <= 1 ) && ( pCmd->payload.pTOU != NULL ) )
  {
    switch( pCmd->payloadType )
    {
      case ZCL_SE_METERING_SP_TOU_SET_DLVD:
      case ZCL_SE_METERING_SP_TOU_SET_RCVD:
        len = ZCL_SE_METERING_SP_TOU_SET_LEN +
              ( pCmd->payload.pTOU->numOfTiersInUse * 6 );
        break;

      case ZCL_SE_METERING_SP_BLOCK_TIER_SET_DLVD:
      case ZCL_SE_METERING_SP_BLOCK_TIER_SET_RCVD:
        len = ZCL_SE_METERING_SP_BLOCK_TIER_SET_LEN +
              ( pCmd->payload.pBlockTier->numOfTiersInUse * 6 ) +
              ( ( pCmd->payload.pBlockTier->tierBlockNumOfTiers *
                  pCmd->payload.pBlockTier->tierBlockNumOfBlocks  ) * 6 );
        break;

      case ZCL_SE_METERING_SP_TOU_SET_DLVD_NO_BILL:
      case ZCL_SE_METERING_SP_TOU_SET_RCVD_NO_BILL:
        len = ZCL_SE_METERING_SP_TOU_SET_NO_BILL_LEN +
              ( pCmd->payload.pTOU_NoBill->numOfTiersInUse * 6 );
        break;

      case ZCL_SE_METERING_SP_BLOCK_TIER_SET_DLVD_NO_BILL:
      case ZCL_SE_METERING_SP_BLOCK_TIER_SET_RCVD_NO_BILL:
        len = ZCL_SE_METERING_SP_BLOCK_TIER_SET_NO_BILL_LEN +
              ( pCmd->payload.pBlockTier->numOfTiersInUse * 6 ) +
              ( ( pCmd->payload.pBlockTier->tierBlockNumOfTiers *
                  pCmd->payload.pBlockTier->tierBlockNumOfBlocks  ) * 6 );

      case ZCL_SE_METERING_SP_DATA_UNAVAIL:
      default:
        // Unknown type - no payload
        break;
    }
  }
  else if ( pCmd->pRawPayload != NULL )
  {
    // Fragmented -- use raw payload fields
    len = pCmd->rawPayloadLen;
  }

  return len;
}

/**************************************************************************************************
 * @fn      zclSE_MeteringSP_Serialize
 *
 * @brief   Serialize snapshot payload.
 *
 * @param   pCmd - command payload
 * @param   pBuf - output buffer
 *
 * @return  uint8_t * - advanced output buffer
 */
static uint8_t *zclSE_MeteringSP_Serialize( zclSE_MeteringPublishSnapshot_t *pCmd,
                                          uint8_t *pBuf )
{
  uint8_t tier;
  uint8_t tierBlk;
  uint8_t tierBlkTotal;
  uint16_t offset;

  // Check for a non fragmented, valid "payload", then serialize
  if ( ( pCmd->cmdTotal <= 1 ) && ( pCmd->payload.pTOU != NULL ) )
  {
    switch( pCmd->payloadType )
    {
      case ZCL_SE_METERING_SP_TOU_SET_DLVD:
      case ZCL_SE_METERING_SP_TOU_SET_RCVD:
        {
          zclSE_MeteringTOU_Set_t *pTOU = pCmd->payload.pTOU;
          pBuf = OsalPort_memcpy( pBuf, pTOU->currSumm, 6 );
          pBuf = OsalPort_bufferUint32( pBuf, pTOU->billToDate );
          pBuf = OsalPort_bufferUint32( pBuf, pTOU->billToDateTimeStamp );
          pBuf = OsalPort_bufferUint32( pBuf, pTOU->projBill );
          pBuf = OsalPort_bufferUint32( pBuf, pTOU->projBillTimeStamp );
          *pBuf++ = pTOU->billTrailingDigit << 4;
          *pBuf++ = pTOU->numOfTiersInUse;

          offset = 0;
          for ( tier = 0; tier < pTOU->numOfTiersInUse; tier++ )
          {
            pBuf = OsalPort_memcpy( pBuf, &pTOU->pTierSumm[offset], 6 );
            offset += 6;
          }
        }
        break;

      case ZCL_SE_METERING_SP_BLOCK_TIER_SET_DLVD:
      case ZCL_SE_METERING_SP_BLOCK_TIER_SET_RCVD:
        {
          zclSE_MeteringBlockTierSet_t *pBlockTier = pCmd->payload.pBlockTier;

          pBuf = OsalPort_memcpy( pBuf, pBlockTier->currSumm, 6 );
          pBuf = OsalPort_bufferUint32( pBuf, pBlockTier->billToDate );
          pBuf = OsalPort_bufferUint32( pBuf, pBlockTier->billToDateTimeStamp );
          pBuf = OsalPort_bufferUint32( pBuf, pBlockTier->projBill );
          pBuf = OsalPort_bufferUint32( pBuf, pBlockTier->projBillTimeStamp );
          *pBuf++ = pBlockTier->billTrailingDigit << 4;
          *pBuf++ = pBlockTier->numOfTiersInUse;

          offset = 0;
          for ( tier = 0; tier < pBlockTier->numOfTiersInUse; tier++ )
          {
            pBuf = OsalPort_memcpy( pBuf, &pBlockTier->pTierSumm[offset], 6 );
            offset += 6;
          }

          *pBuf++ = ( ( pBlockTier->tierBlockNumOfTiers << 4    ) |
                      ( pBlockTier->tierBlockNumOfBlocks & 0x0F )   );

          tierBlkTotal = pBlockTier->tierBlockNumOfTiers *
                         pBlockTier->tierBlockNumOfBlocks;

          offset = 0;
          for ( tierBlk = 0; tierBlk < tierBlkTotal; tierBlk++ )
          {
            pBuf = OsalPort_memcpy( pBuf, &pBlockTier->pTierBlockSumm[offset], 6 );
            offset += 6;
          }
        }
        break;

      case ZCL_SE_METERING_SP_TOU_SET_DLVD_NO_BILL:
      case ZCL_SE_METERING_SP_TOU_SET_RCVD_NO_BILL:
        {
          zclSE_MeteringTOU_NoBillInfo_t *pTOU = pCmd->payload.pTOU_NoBill;
          pBuf = OsalPort_memcpy( pBuf, pTOU->currSumm, 6 );
          *pBuf++ = pTOU->numOfTiersInUse;

          offset = 0;
          for ( tier = 0; tier < pTOU->numOfTiersInUse; tier++ )
          {
            pBuf = OsalPort_memcpy( pBuf, &pTOU->pTierSumm[offset], 6 );
            offset += 6;
          }
        }
        break;

      case ZCL_SE_METERING_SP_BLOCK_TIER_SET_DLVD_NO_BILL:
      case ZCL_SE_METERING_SP_BLOCK_TIER_SET_RCVD_NO_BILL:
        {
          zclSE_MeteringBlockTierSetNoBillInfo_t *pBlockTier =
            pCmd->payload.pBlockTierNoBill;

          pBuf = OsalPort_memcpy( pBuf, pBlockTier->currSumm, 6 );
          *pBuf++ = pBlockTier->numOfTiersInUse;

          offset = 0;
          for ( tier = 0; tier < pBlockTier->numOfTiersInUse; tier++ )
          {
            pBuf = OsalPort_memcpy( pBuf, &pBlockTier->pTierSumm[offset], 6 );
            offset += 6;
          }

          *pBuf++ = ( ( pBlockTier->tierBlockNumOfTiers << 4    ) |
                      ( pBlockTier->tierBlockNumOfBlocks & 0x0F )   );

          tierBlkTotal = pBlockTier->tierBlockNumOfTiers *
                         pBlockTier->tierBlockNumOfBlocks;

          offset = 0;
          for ( tierBlk = 0; tierBlk < tierBlkTotal; tierBlk++ )
          {
            pBuf = OsalPort_memcpy( pBuf, &pBlockTier->pTierBlockSumm[offset], 6 );
            offset += 6;
          }
        }
        break;

      case ZCL_SE_METERING_SP_DATA_UNAVAIL:
      default:
        // Unknown type - no payload
        break;
    }
  }
  else if ( pCmd->pRawPayload )
  {
    // Fragmented -- use raw payload fields
    pBuf = OsalPort_memcpy( pBuf, pCmd->pRawPayload, pCmd->rawPayloadLen );
  }

  return pBuf;
}

/**************************************************************************************************
 * @fn      zclSE_MessagingBuildDisplayMsg
 *
 * @brief   Build a display message payload.
 *
 * @param   pCmd - command payload
 * @param   ppCmdBuf - allocated command buffer
 * @param   pCmdBufLen - size of allocated command buffer
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclSE_MessagingBuildDisplayMsg
                   ( zclSE_MessagingDisplayMsg_t *pCmd,
                     uint8_t **ppCmdBuf, uint16_t *pCmdBufLen )
{
  ZStatus_t status = ZSuccess;
  uint8_t *pBuf;

  // Allocate command buffer
  *pCmdBufLen = ZCL_SE_MESSAGING_DISPLAY_MSG_LEN + zclSE_UTF8StringLen( &pCmd->msg );

  *ppCmdBuf = OsalPort_malloc( *pCmdBufLen );
  if ( ppCmdBuf == NULL )
  {
    return ZMemError;
  }

  pBuf = *ppCmdBuf;
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->msgID );
  *pBuf = pCmd->msgCtrl.transmission & 0x03;
  *pBuf |= ( pCmd->msgCtrl.importanceLevel << 2 ) & 0x0C;
  *pBuf |= ( pCmd->msgCtrl.enhancedCfmRequired << 5 ) & 0x60;
  *pBuf++ |= pCmd->msgCtrl.msgCfmRequired << 7;
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->startTime );
  *pBuf++ = LO_UINT16( pCmd->duration );
  *pBuf++ = HI_UINT16( pCmd->duration );
  pBuf = zclSE_UTF8StringBuild( pBuf, &pCmd->msg );
  *pBuf = pCmd->extMsgCtrl.msgConfirmed;

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_TunnelingSendTransferData
 *
 * @brief   Send transfer data.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   cmdID - command ID
 * @param   pCmd - command payload
 * @param   direction - command direction
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclSE_TunnelingSendTransferData(
                   uint8_t srcEP, afAddrType_t *dstAddr, uint8_t cmdID,
                   zclSE_TunnelingTransferData_t *pCmd, uint8_t direction,
                   uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t *pCmdBuf;
  uint16_t cmdBufLen;
  uint8_t *pBuf;

  // Allocate command buffer
  cmdBufLen = ZCL_SE_TUNNELING_TRANSFER_DATA_LEN + pCmd->dataLen;

  pCmdBuf = OsalPort_malloc( cmdBufLen );
  if ( pCmdBuf == NULL )
  {
    return ZMemError;
  }

  pBuf = pCmdBuf;
  *pBuf++ = LO_UINT16( pCmd->tunnelID );
  *pBuf++ = HI_UINT16( pCmd->tunnelID );
  OsalPort_memcpy( pBuf, pCmd->pData, pCmd->dataLen );

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_TUNNELING,
                            cmdID, TRUE, direction, disableDefaultRsp,
                            0, seqNum, cmdBufLen, pCmdBuf );

  OsalPort_free( pCmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_TunnelingSendTransferDataErr
 *
 * @brief   Send transfer data error.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   cmdID - command ID
 * @param   pCmd - command payload
 * @param   direction - command direction
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclSE_TunnelingSendTransferDataErr(
                   uint8_t srcEP, afAddrType_t *dstAddr, uint8_t cmdID,
                   zclSE_TunnelingTransferDataErr_t *pCmd, uint8_t direction,
                   uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t cmdBuf[ZCL_SE_TUNNELING_TRANSFER_DATA_ERR_LEN];
  uint8_t *pBuf = cmdBuf;

  *pBuf++ = LO_UINT16( pCmd->tunnelID );
  *pBuf++ = HI_UINT16( pCmd->tunnelID );
  *pBuf   = pCmd->status;

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_TUNNELING,
                            cmdID, TRUE, direction, disableDefaultRsp,
                            0, seqNum, ZCL_SE_TUNNELING_TRANSFER_DATA_ERR_LEN,
                            cmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_TunnelingSendAckTransferData
 *
 * @brief   Send ACK transfer data.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   cmdID - command ID
 * @param   pCmd - command payload
 * @param   direction - command direction
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclSE_TunnelingSendAckTransferData(
                   uint8_t srcEP, afAddrType_t *dstAddr, uint8_t cmdID,
                   zclSE_TunnelingAckTransferData_t *pCmd, uint8_t direction,
                   uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t cmdBuf[ZCL_SE_TUNNELING_ACK_TRANSFER_DATA_LEN];
  uint8_t *pBuf = cmdBuf;

  *pBuf++ = LO_UINT16( pCmd->tunnelID );
  *pBuf++ = HI_UINT16( pCmd->tunnelID );
  *pBuf++ = LO_UINT16( pCmd->numOfBytesLeft );
  *pBuf   = HI_UINT16( pCmd->numOfBytesLeft );

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_TUNNELING,
                            cmdID, TRUE, direction, disableDefaultRsp,
                            0, seqNum, ZCL_SE_TUNNELING_ACK_TRANSFER_DATA_LEN,
                            cmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_TunnelingSendReadyData
 *
 * @brief   Send ready data.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   cmdID - command ID
 * @param   pCmd - command payload
 * @param   direction - command direction
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclSE_TunnelingSendReadyData(
                   uint8_t srcEP, afAddrType_t *dstAddr, uint8_t cmdID,
                   zclSE_TunnelingReadyData_t *pCmd, uint8_t direction,
                   uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t cmdBuf[ZCL_SE_TUNNELING_READY_DATA_LEN];
  uint8_t *pBuf = cmdBuf;

  *pBuf++ = LO_UINT16( pCmd->tunnelID );
  *pBuf++ = HI_UINT16( pCmd->tunnelID );
  *pBuf++ = LO_UINT16( pCmd->numOfOctetsLeft );
  *pBuf   = HI_UINT16( pCmd->numOfOctetsLeft );

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_TUNNELING,
                            cmdID, TRUE, direction, disableDefaultRsp,
                            0, seqNum, ZCL_SE_TUNNELING_READY_DATA_LEN,
                            cmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_EventsEvtLogLen
 *
 * @brief   Called to get the length of an event log.
 *
 * @param   pCmd - command payload
 *
 * @return  uint16_t - length
 */
static uint16_t zclSE_EventsEvtLogLen( zclSE_EventsPublishEvtLog_t *pCmd )
{
  uint16_t len = 0;

  // Check for a non fragmented, valid "pEvts", then calculate length
  if ( pCmd->cmdTotal <= 1 && pCmd->log.pEvts != NULL )
  {
    uint8_t evtIdx;
    zclSE_EventsLoggedEvt_t *pEvt;

    len = ZCL_SE_EVENTS_EVT_LOG_HDR_LEN;

    for ( evtIdx = 0; evtIdx < pCmd->log.numOfEvts; evtIdx++ )
    {
      pEvt = &pCmd->log.pEvts[evtIdx];

      len += ZCL_SE_EVENTS_LOGGED_EVT_LEN + zclSE_UTF8StringLen( &pEvt->evtData );
    }
  }
  else if ( pCmd->pRawPayload != NULL )
  {
    // Fragmented -- use raw payload fields
    len = pCmd->rawPayloadLen;
  }

  return len;
}

/**************************************************************************************************
 * @fn      zclSE_EventsEvtLogSerialize
 *
 * @brief   Serialize an event log.
 *
 * @param   pCmd - command payload
 * @param   pBuf - output buffer
 *
 * @return  uint8_t * - advanced output buffer
 */
static uint8_t *zclSE_EventsEvtLogSerialize( zclSE_EventsPublishEvtLog_t *pCmd,
                                           uint8_t *pBuf )
{
  // Check for a non fragmented, valid "pEvts", then serialize
  if ( pCmd->cmdTotal <= 1 && pCmd->log.pEvts != NULL )
  {
    uint8_t evtIdx;
    zclSE_EventsLoggedEvt_t *pEvt;

    *pBuf = pCmd->log.numOfEvts << 4;
    *pBuf++ |= pCmd->log.ctrl & 0x0F;

    for ( evtIdx = 0; evtIdx < pCmd->log.numOfEvts; evtIdx++ )
    {
      pEvt = &pCmd->log.pEvts[evtIdx];

      *pBuf++ = pEvt->logID;
      *pBuf++ = LO_UINT16( pEvt->evtID );
      *pBuf++ = HI_UINT16( pEvt->evtID );
      pBuf = OsalPort_bufferUint32( pBuf, pEvt->evtTime );
      pBuf = zclSE_UTF8StringBuild( pBuf, &pEvt->evtData );
    }
  }
  else if ( pCmd->pRawPayload )
  {
    // Fragmented -- use raw payload fields
    pBuf = OsalPort_memcpy( pBuf, pCmd->pRawPayload, pCmd->rawPayloadLen );
  }

  return pBuf;
}


/**************************************************************************************************
 * PUBLIC FUNCTIONS
 */

/**************************************************************************************************
 * @fn      zclSE_DRLC_SendLoadCtrlEvt
 *
 * @brief   Send COMMAND_SE_DRLC_LOAD_CTRL_EVT.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_DRLC_SendLoadCtrlEvt( uint8_t srcEP, afAddrType_t *dstAddr,
                                      zclSE_DRLC_LoadCtrlEvt_t *pCmd,
                                      uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t *pCmdBuf;
  uint16_t cmdBufLen;
  uint8_t *pBuf;

  // Allocate command buffer
  cmdBufLen = ZCL_SE_DRLC_LOAD_CTRL_EVT_LEN;

  pCmdBuf = OsalPort_malloc( cmdBufLen );
  if ( pCmdBuf == NULL )
  {
    return ZMemError;
  }

  pBuf = pCmdBuf;
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->issuerEvtID );
  *pBuf++ = LO_UINT16( pCmd->deviceClass );
  *pBuf++ = HI_UINT16( pCmd->deviceClass );
  *pBuf++ = pCmd->utilityEnrollmentGroup;
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->startTime );
  *pBuf++ = LO_UINT16( pCmd->duration );
  *pBuf++ = HI_UINT16( pCmd->duration );
  *pBuf++ = pCmd->criticalityLevel;
  *pBuf++ = pCmd->coolingTempOffset;
  *pBuf++ = pCmd->heatingTempOffset;
  *pBuf++ = LO_UINT16( pCmd->coolingTempSetPt );
  *pBuf++ = HI_UINT16( pCmd->coolingTempSetPt );
  *pBuf++ = LO_UINT16( pCmd->heatingTempSetPt );
  *pBuf++ = HI_UINT16( pCmd->heatingTempSetPt );
  *pBuf++ = pCmd->avgLoadAdjPct;
  *pBuf++ = pCmd->dutyCycle;
  *pBuf = pCmd->evtCtrl;

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_DRLC,
                            COMMAND_SE_DRLC_LOAD_CTRL_EVT, TRUE,
                            ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0, seqNum,
                            cmdBufLen, pCmdBuf );

  OsalPort_free( pCmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_DRLC_SendCancelLoadCtrlEvt
 *
 * @brief   Send COMMAND_SE_DRLC_CANCEL_LOAD_CTRL_EVT.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_DRLC_SendCancelLoadCtrlEvt( uint8_t srcEP, afAddrType_t *dstAddr,
                                            zclSE_DRLC_CancelLoadCtrlEvt_t *pCmd,
                                            uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t cmdBuf[ZCL_SE_DRLC_CANCEL_LOAD_CTRL_EVT];
  uint8_t *pBuf = cmdBuf;

  pBuf = OsalPort_bufferUint32( pBuf, pCmd->issuerEvtID );
  *pBuf++ = LO_UINT16( pCmd->deviceClass );
  *pBuf++ = HI_UINT16( pCmd->deviceClass );
  *pBuf++ = pCmd->utilityEnrollmentGroup;
  *pBuf++ = pCmd->cancelCtrl;
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->effectiveTime );

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_DRLC,
                            COMMAND_SE_DRLC_CANCEL_LOAD_CTRL_EVT, TRUE,
                            ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0, seqNum,
                            ZCL_SE_DRLC_CANCEL_LOAD_CTRL_EVT, cmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_DRLC_SendCancelAllLoadCtrlEvts
 *
 * @brief   Send COMMAND_SE_DRLC_CANCEL_ALL_LOAD_CTRL_EVTS.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_DRLC_SendCancelAllLoadCtrlEvts(
            uint8_t srcEP, afAddrType_t *dstAddr,
            zclSE_DRLC_CancelAllLoadCtrlEvts_t *pCmd,
            uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t cmdBuf[ZCL_SE_DRLC_CANCEL_ALL_LOAD_CTRL_EVTS_LEN];
  uint8_t *pBuf = cmdBuf;

  *pBuf++ = pCmd->cancelCtrl;

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_DRLC,
                            COMMAND_SE_DRLC_CANCEL_ALL_LOAD_CTRL_EVTS, TRUE,
                            ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0, seqNum,
                            ZCL_SE_DRLC_CANCEL_ALL_LOAD_CTRL_EVTS_LEN, cmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_DRLC_SendReportEvtStatus
 *
 * @brief   Send COMMAND_SE_DRLC_REPORT_EVT_STATUS.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_DRLC_SendReportEvtStatus( uint8_t srcEP, afAddrType_t *dstAddr,
                                          zclSE_DRLC_ReportEvtStatus_t *pCmd,
                                          uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t *pCmdBuf;
  uint16_t cmdBufLen;
  uint8_t *pBuf;

  // Allocate command buffer
  cmdBufLen = ZCL_SE_DRLC_REPORT_EVT_STATUS_LEN + ZCL_SE_DRLC_REPORT_EVT_STATUS_SIG_LEN;

  pCmdBuf = OsalPort_malloc( cmdBufLen );
  if ( pCmdBuf == NULL )
  {
    return ZMemError;
  }

  pBuf = pCmdBuf;
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->issuerEvtID );
  *pBuf++ = pCmd->evtStatus;
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->evtStatusTime );
  *pBuf++ = pCmd->critLevelApplied;
  *pBuf++ = LO_UINT16( pCmd->coolingTempSetPtApplied );
  *pBuf++ = HI_UINT16( pCmd->coolingTempSetPtApplied );
  *pBuf++ = LO_UINT16( pCmd->heatingTempSetPtApplied );
  *pBuf++ = HI_UINT16( pCmd->heatingTempSetPtApplied );
  *pBuf++ = pCmd->avgLoadAdjPctApplied;
  *pBuf++ = pCmd->dutyCycleApplied;
  *pBuf++ = pCmd->evtCtrl;
  *pBuf++ = pCmd->signatureType;

  memset( pBuf, 0xFF, ZCL_SE_DRLC_REPORT_EVT_STATUS_SIG_LEN );

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_DRLC,
                            COMMAND_SE_DRLC_REPORT_EVT_STATUS, TRUE,
                            ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0, seqNum,
                            cmdBufLen, pCmdBuf );

  OsalPort_free( pCmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_DRLC_SendGetScheduledEvts
 *
 * @brief   Send COMMAND_SE_DRLC_GET_SCHEDULED_EVTS.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_DRLC_SendGetScheduledEvts( uint8_t srcEP, afAddrType_t *dstAddr,
                                           zclSE_DRLC_GetScheduledEvts_t *pCmd,
                                           uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t cmdBuf[ZCL_SE_DRLC_GET_SCHEDULED_EVTS_LEN];
  uint8_t *pBuf = cmdBuf;

  pBuf = OsalPort_bufferUint32( pBuf, pCmd->startTime );
  *pBuf++ = pCmd->numOfEvents;

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_DRLC,
                            COMMAND_SE_DRLC_GET_SCHEDULED_EVTS, TRUE,
                            ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp,
                            0, seqNum, ZCL_SE_DRLC_GET_SCHEDULED_EVTS_LEN, cmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_DRLC_ParseLoadCtrlEvt
 *
 * @brief   Parse COMMAND_SE_DRLC_LOAD_CTRL_EVT.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_DRLC_ParseLoadCtrlEvt( zclIncoming_t *pInMsg,
                                       zclSE_DRLC_LoadCtrlEvt_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_DRLC_LOAD_CTRL_EVT_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->issuerEvtID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->deviceClass = BUILD_UINT16( pBuf[0], pBuf[1] );
  pBuf += 2;
  pCmd->utilityEnrollmentGroup = *pBuf++;
  pCmd->startTime = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->duration = BUILD_UINT16( pBuf[0], pBuf[1] );
  pBuf += 2;
  pCmd->criticalityLevel = *pBuf++;
  pCmd->coolingTempOffset = *pBuf++;
  pCmd->heatingTempOffset = *pBuf++;
  pCmd->coolingTempSetPt = BUILD_UINT16( pBuf[0], pBuf[1] );
  pBuf += 2;
  pCmd->heatingTempSetPt = BUILD_UINT16( pBuf[0], pBuf[1] );
  pBuf += 2;
  pCmd->avgLoadAdjPct = *pBuf++;
  pCmd->dutyCycle = *pBuf++;
  pCmd->evtCtrl = *pBuf++;

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_DRLC_ParseCancelLoadCtrlEvt
 *
 * @brief   Parse COMMAND_SE_DRLC_CANCEL_LOAD_CTRL_EVT.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_DRLC_ParseCancelLoadCtrlEvt(
            zclIncoming_t *pInMsg,
            zclSE_DRLC_CancelLoadCtrlEvt_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_DRLC_CANCEL_LOAD_CTRL_EVT )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->issuerEvtID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->deviceClass = BUILD_UINT16( pBuf[0], pBuf[1] );
  pBuf += 2;
  pCmd->utilityEnrollmentGroup = *pBuf++;
  pCmd->cancelCtrl = *pBuf++;
  pCmd->effectiveTime = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;

  return ZCL_STATUS_SUCCESS;
}


/**************************************************************************************************
 * @fn      zclSE_DRLC_ParseCancelAllLoadCtrlEvts
 *
 * @brief   Parse COMMAND_SE_DRLC_CANCEL_ALL_LOAD_CTRL_EVTS.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_DRLC_ParseCancelAllLoadCtrlEvts(
            zclIncoming_t *pInMsg,
            zclSE_DRLC_CancelAllLoadCtrlEvts_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_DRLC_CANCEL_ALL_LOAD_CTRL_EVTS_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->cancelCtrl = *pBuf++;

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_DRLC_ParseReportEvtStatus
 *
 * @brief   Parse COMMAND_SE_DRLC_REPORT_EVT_STATUS.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_DRLC_ParseReportEvtStatus( zclIncoming_t *pInMsg,
                                           zclSE_DRLC_ReportEvtStatus_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ( ZCL_SE_DRLC_REPORT_EVT_STATUS_LEN +
                            ZCL_SE_DRLC_REPORT_EVT_STATUS_SIG_LEN ) )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->issuerEvtID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->evtStatus = *pBuf++;
  pCmd->evtStatusTime = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->critLevelApplied = *pBuf++;
  pCmd->coolingTempSetPtApplied = BUILD_UINT16( pBuf[0], pBuf[1] );
  pBuf += 2;
  pCmd->heatingTempSetPtApplied = BUILD_UINT16( pBuf[0], pBuf[1] );
  pBuf += 2;
  pCmd->avgLoadAdjPctApplied = *pBuf++;
  pCmd->dutyCycleApplied = *pBuf++;
  pCmd->evtCtrl = *pBuf++;
  pCmd->signatureType = *pBuf++;
  pCmd->signature.strLen = ZCL_SE_DRLC_REPORT_EVT_STATUS_SIG_LEN;
  pCmd->signature.pStr = pBuf;

  return ZCL_STATUS_SUCCESS;
}


/**************************************************************************************************
 * @fn      zclSE_DRLC_ParseGetScheduledEvts
 *
 * @brief   Parse COMMAND_SE_DRLC_GET_SCHEDULED_EVTS.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_DRLC_ParseGetScheduledEvts( zclIncoming_t *pInMsg,
                                            zclSE_DRLC_GetScheduledEvts_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_DRLC_GET_SCHEDULED_EVTS_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->startTime = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->numOfEvents = *pBuf++;

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_DRLC_HdlLoadCtrlEvt
 *
 * @brief   Handle COMMAND_SE_DRLC_LOAD_CTRL_EVT.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_DRLC_HdlLoadCtrlEvt( zclIncoming_t *pInMsg,
                                     zclSE_DRLC_LoadCtrlEvtCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_DRLC_LoadCtrlEvt_t *pCmd;

    pCmd = OsalPort_malloc( sizeof( zclSE_DRLC_LoadCtrlEvt_t ) );
    if ( pCmd == NULL )
    {
       return ZCL_STATUS_SOFTWARE_FAILURE;
    }

    status = zclSE_DRLC_ParseLoadCtrlEvt( pInMsg, pCmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      pfnCB( pInMsg, pCmd );

      status = ZCL_STATUS_CMD_HAS_RSP;
    }

    OsalPort_free( pCmd );
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_DRLC_HdlCancelLoadCtrlEvt
 *
 * @brief   Handle COMMAND_SE_DRLC_CANCEL_LOAD_CTRL_EVT.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_DRLC_HdlCancelLoadCtrlEvt( zclIncoming_t *pInMsg,
                                           zclSE_DRLC_CancelLoadCtrlEvtCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_DRLC_CancelLoadCtrlEvt_t cmd;

    status = zclSE_DRLC_ParseCancelLoadCtrlEvt( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      pfnCB( pInMsg, &cmd );

      status = ZCL_STATUS_CMD_HAS_RSP;
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_DRLC_HdlCancelAllLoadCtrlEvts
 *
 * @brief   Handle COMMAND_SE_DRLC_CANCEL_ALL_LOAD_CTRL_EVTS.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_DRLC_HdlCancelAllLoadCtrlEvts(
            zclIncoming_t *pInMsg,
            zclSE_DRLC_CancelAllLoadCtrlEvtsCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_DRLC_CancelAllLoadCtrlEvts_t cmd;

    status = zclSE_DRLC_ParseCancelAllLoadCtrlEvts( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      pfnCB( pInMsg, &cmd );

      status = ZCL_STATUS_CMD_HAS_RSP;
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_DRLC_HdlReportEvtStatus
 *
 * @brief   Handle COMMAND_SE_DRLC_REPORT_EVT_STATUS.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_DRLC_HdlReportEvtStatus( zclIncoming_t *pInMsg,
                                         zclSE_DRLC_ReportEvtStatusCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_DRLC_ReportEvtStatus_t *pCmd;

    pCmd = OsalPort_malloc( sizeof( zclSE_DRLC_ReportEvtStatus_t ) );
    if ( pCmd == NULL )
    {
       return ZCL_STATUS_SOFTWARE_FAILURE;
    }

    status = zclSE_DRLC_ParseReportEvtStatus( pInMsg, pCmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      pfnCB( pInMsg, pCmd );
    }

    OsalPort_free( pCmd );
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_DRLC_HdlGetScheduledEvts
 *
 * @brief   Handle COMMAND_SE_DRLC_GET_SCHEDULED_EVTS.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_DRLC_HdlGetScheduledEvts( zclIncoming_t *pInMsg,
                                          zclSE_DRLC_GetScheduledEvtsCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_DRLC_GetScheduledEvts_t cmd;

    status = zclSE_DRLC_ParseGetScheduledEvts( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      status = pfnCB( pInMsg, &cmd );
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_DRLC_HdlClientCmd
 *
 * @brief   Client command handler for ZCL_CLUSTER_ID_SE_DRLC.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCBs - client callbacks
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_DRLC_HdlClientCmd
            ( zclIncoming_t *pInMsg,
              const zclSE_DRLC_ClientCBs_t *pCBs )
{
  ZStatus_t status;

  // Guard against NULL pointer
  if ( pCBs == NULL )
  {
    return ZCL_STATUS_FAILURE;
  }

  // Process Server-to-Client Commands
  switch ( pInMsg->hdr.commandID )
  {
    case COMMAND_SE_DRLC_LOAD_CTRL_EVT:
      status = zclSE_DRLC_HdlLoadCtrlEvt( pInMsg, pCBs->pfnLoadCtrlEvt );
      break;

    case COMMAND_SE_DRLC_CANCEL_LOAD_CTRL_EVT:
      status = zclSE_DRLC_HdlCancelLoadCtrlEvt( pInMsg, pCBs->pfnCancelLoadCtrlEvt );
      break;

    case COMMAND_SE_DRLC_CANCEL_ALL_LOAD_CTRL_EVTS:
      status = zclSE_DRLC_HdlCancelAllLoadCtrlEvts( pInMsg, pCBs->pfnCancelAllLoadCtrlEvts );
      break;

    default:
      status = ZCL_STATUS_FAILURE;
      break;
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_DRLC_HdlServerCmd
 *
 * @brief   Server command handler for ZCL_CLUSTER_ID_SE_DRLC.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCBs - server callbacks
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_DRLC_HdlServerCmd
            ( zclIncoming_t *pInMsg,
              const zclSE_DRLC_ServerCBs_t *pCBs )
{
  ZStatus_t status;

  // Guard against NULL pointer
  if ( pCBs == NULL )
  {
    return ZCL_STATUS_FAILURE;
  }

  // Process Client-to-Server Commands
  switch ( pInMsg->hdr.commandID )
  {
    case COMMAND_SE_DRLC_REPORT_EVT_STATUS:
      status = zclSE_DRLC_HdlReportEvtStatus( pInMsg, pCBs->pfnReportEvtStatus );
      break;

    case COMMAND_SE_DRLC_GET_SCHEDULED_EVTS:
      status = zclSE_DRLC_HdlGetScheduledEvts( pInMsg, pCBs->pfnGetScheduledEvts );
      break;

    default:
      status = ZCL_STATUS_FAILURE;
      break;
  }

  return status;

}

/**************************************************************************************************
 * @fn      zclSE_MeteringSendGetProfileRsp
 *
 * @brief   Send COMMAND_SE_METERING_GET_PROFILE_RSP.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MeteringSendGetProfileRsp( uint8_t srcEP, afAddrType_t *dstAddr,
                                           zclSE_MeteringGetProfileRsp_t *pCmd,
                                           uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t *pCmdBuf;
  uint16_t cmdBufLen;
  uint8_t *pBuf;
  uint8_t interval;

  // Allocate command buffer
  cmdBufLen = ZCL_SE_METERING_GET_PROFILE_RSP_LEN + ( 3 * pCmd->numOfPeriodDlvd );

  pCmdBuf = OsalPort_malloc( cmdBufLen );
  if ( pCmdBuf == NULL )
  {
    return ZMemError;
  }

  pBuf = pCmdBuf;
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->endTime );
  *pBuf++ = pCmd->status;
  *pBuf++ = pCmd->profileIntervalPeriod;
  *pBuf++ = pCmd->numOfPeriodDlvd;

  for ( interval = 0; interval < pCmd->numOfPeriodDlvd; interval++ )
  {
    pBuf = se_buffer_uint24( pBuf, (uint24) &pCmd->pIntervals[interval] );
  }

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_METERING,
                            COMMAND_SE_METERING_GET_PROFILE_RSP, TRUE,
                            ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0,
                            seqNum, cmdBufLen, pCmdBuf );

  OsalPort_free( pCmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_MeteringSendReqFastPollModeRsp
 *
 * @brief   Send COMMAND_SE_METERING_REQ_FAST_POLL_MODE_RSP.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MeteringSendReqFastPollModeRsp(
            uint8_t srcEP, afAddrType_t *dstAddr,
            zclSE_MeteringReqFastPollModeRsp_t *pCmd,
            uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t cmdBuf[ZCL_SE_METERING_REQ_FAST_POLL_MODE_RSP_LEN];
  uint8_t *pBuf = cmdBuf;

  *pBuf++ = pCmd->appliedUpdatePeriod;
  OsalPort_bufferUint32( pBuf, pCmd->endTime );

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_METERING,
                            COMMAND_SE_METERING_REQ_FAST_POLL_MODE_RSP, TRUE,
                            ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0,
                            seqNum, ZCL_SE_METERING_REQ_FAST_POLL_MODE_RSP_LEN,
                            cmdBuf );
  return status;
}

/**************************************************************************************************
 * @fn      zclSE_MeteringSendScheduleSnapshotRsp
 *
 * @brief   Send COMMAND_SE_METERING_SCHEDULE_SNAPSHOT_RSP.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MeteringSendScheduleSnapshotRsp(
            uint8_t srcEP, afAddrType_t *dstAddr,
            zclSE_MeteringScheduleSnapshotRsp_t *pCmd,
            uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t *pCmdBuf;
  uint16_t cmdBufLen;
  uint8_t *pBuf;
  uint8_t snapshot;

  // Allocate command buffer
  cmdBufLen = ZCL_SE_METERING_SCHEDULE_SNAPSHOT_RSP_LEN + ( pCmd->numOfEntries * 2 );

  pCmdBuf = OsalPort_malloc( cmdBufLen );
  if ( pCmdBuf == NULL )
  {
    return ZMemError;
  }

  pBuf = pCmdBuf;
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->issuerEvtID );

  for ( snapshot = 0; snapshot < pCmd->numOfEntries; snapshot++ )
  {
    *pBuf++ = pCmd->pEntries[snapshot].scheduleID;
    *pBuf++ = pCmd->pEntries[snapshot].scheduleCfm;
  }

  status =  zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_METERING,
                             COMMAND_SE_METERING_SCHEDULE_SNAPSHOT_RSP, TRUE,
                             ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0,
                             seqNum, cmdBufLen, pCmdBuf );

  OsalPort_free( pCmdBuf );

  return ( status );
}

/**************************************************************************************************
 * @fn      zclSE_MeteringSendTakeSnapshotRsp
 *
 * @brief   Send COMMAND_SE_METERING_TAKE_SNAPSHOT_RSP.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MeteringSendTakeSnapshotRsp(
            uint8_t srcEP, afAddrType_t *dstAddr,
            zclSE_MeteringTakeSnapshotRsp_t *pCmd,
            uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t cmdBuf[ZCL_SE_METERING_TAKE_SNAPSHOT_RSP_LEN];
  uint8_t *pBuf = cmdBuf;

  pBuf = OsalPort_bufferUint32( pBuf, pCmd->snapshotID );
  *pBuf++ = pCmd->snapshotCfm;

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_METERING,
                            COMMAND_SE_METERING_TAKE_SNAPSHOT_RSP, TRUE,
                            ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0,
                            seqNum, ZCL_SE_METERING_TAKE_SNAPSHOT_RSP_LEN,
                            cmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_MeteringSendPublishSnapshot
 *
 * @brief   Send COMMAND_SE_METERING_PUBLISH_SNAPSHOT.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MeteringSendPublishSnapshot(
            uint8_t srcEP, afAddrType_t *dstAddr,
            zclSE_MeteringPublishSnapshot_t *pCmd,
            uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t *pCmdBuf;
  uint16_t cmdBufLen;
  uint8_t *pBuf;

  // Allocate command buffer
  cmdBufLen = ZCL_SE_METERING_PUBLISH_SNAPSHOT_LEN +
              zclSE_MeteringSP_Len( pCmd );

  pCmdBuf = OsalPort_malloc( cmdBufLen );
  if ( pCmdBuf == NULL )
  {
    return ZMemError;
  }

  pBuf = pCmdBuf;
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->snapshotID );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->time );
  *pBuf++ = pCmd->totalFound;
  *pBuf++ = pCmd->cmdIdx;
  *pBuf++ = pCmd->cmdTotal;
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->cause );
  *pBuf++ = pCmd->payloadType;
  pBuf = zclSE_MeteringSP_Serialize( pCmd, pBuf );

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_METERING,
                            COMMAND_SE_METERING_PUBLISH_SNAPSHOT, TRUE,
                            ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp,
                            0, seqNum, cmdBufLen, pCmdBuf );

  OsalPort_free( pCmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_MeteringSendGetSampledDataRs
 *
 * @brief   Send COMMAND_SE_METERING_GET_SAMPLED_DATA_RSP.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MeteringSendGetSampledDataRsp(
            uint8_t srcEP, afAddrType_t *dstAddr,
            zclSE_MeteringGetSampledDataRsp_t *pCmd,
            uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t *pCmdBuf;
  uint16_t cmdBufLen;
  uint8_t *pBuf;
  uint8_t sample;

  // Allocate command buffer
  cmdBufLen = ZCL_SE_METERING_GET_SAMPLED_DATA_RSP_LEN + ( 3 * pCmd->numOfSamples );

  pCmdBuf = OsalPort_malloc( cmdBufLen );
  if ( pCmdBuf == NULL )
  {
    return ZMemError;
  }

  pBuf = pCmdBuf;
  *pBuf++ = LO_UINT16( pCmd->sampleID );
  *pBuf++ = HI_UINT16( pCmd->sampleID );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->startTime );
  *pBuf++ = pCmd->type;
  *pBuf++ = LO_UINT16( pCmd->reqInterval );
  *pBuf++ = HI_UINT16( pCmd->reqInterval );
  *pBuf++ = LO_UINT16( pCmd->numOfSamples );
  *pBuf++ = HI_UINT16( pCmd->numOfSamples );

  for ( sample = 0; sample < pCmd->numOfSamples; sample++ )
  {
    pBuf = se_buffer_uint24( pBuf, pCmd->pSamples[sample] );
  }

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_METERING,
                            COMMAND_SE_METERING_GET_SAMPLED_DATA_RSP, TRUE,
                            ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0,
                            seqNum, cmdBufLen, pCmdBuf );

  OsalPort_free( pCmdBuf );

  return ( status );
}

/**************************************************************************************************
 * @fn      zclSE_MeteringSendCfgMirror
 *
 * @brief   Send COMMAND_SE_METERING_CFG_MIRROR.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MeteringSendCfgMirror( uint8_t srcEP, afAddrType_t *dstAddr,
                                       zclSE_MeteringCfgMirror_t *pCmd,
                                       uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t cmdBuf[ZCL_SE_METERING_CFG_MIRROR_LEN];
  uint8_t *pBuf = cmdBuf;

  pBuf = OsalPort_bufferUint32( pBuf, pCmd->issuerEvtID );
  pBuf = se_buffer_uint24( pBuf, pCmd->reportingInterval );
  *pBuf++ = pCmd->mirrorNotifReporting;
  *pBuf = pCmd->notifScheme;

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_METERING,
                            COMMAND_SE_METERING_CFG_MIRROR, TRUE,
                            ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0,
                            seqNum, ZCL_SE_METERING_CFG_MIRROR_LEN, cmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_MeteringSendCfgNotifScheme
 *
 * @brief   Send COMMAND_SE_METERING_CFG_NOTIF_SCHEME.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MeteringSendCfgNotifScheme( uint8_t srcEP, afAddrType_t *dstAddr,
                                            zclSE_MeteringCfgNotifScheme_t *pCmd,
                                            uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t cmdBuf[ZCL_SE_METERING_CFG_NOTIF_SCHEME_LEN];
  uint8_t *pBuf = cmdBuf;

  pBuf = OsalPort_bufferUint32( pBuf, pCmd->issuerEvtID );
  *pBuf++ = pCmd->notifScheme;
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->notifFlagOrder );

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_METERING,
                            COMMAND_SE_METERING_CFG_NOTIF_SCHEME, TRUE,
                            ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0,
                            seqNum, ZCL_SE_METERING_CFG_NOTIF_SCHEME_LEN, cmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_MeteringSendCfgNotifFlag
 *
 * @brief   Send COMMAND_SE_METERING_CFG_NOTIF_FLAG.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MeteringSendCfgNotifFlag( uint8_t srcEP, afAddrType_t *dstAddr,
                                          zclSE_MeteringCfgNotifFlag_t *pCmd,
                                          uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t *pCmdBuf;
  uint16_t cmdBufLen;
  uint8_t *pBuf;
  uint8_t cmd;

  // Allocate command buffer
  cmdBufLen = ZCL_SE_METERING_CFG_NOTIF_FLAGS_LEN + pCmd->numOfCmds;

  pCmdBuf = OsalPort_malloc( cmdBufLen );
  if ( pCmdBuf == NULL )
  {
    return ZMemError;
  }

  pBuf = pCmdBuf;
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->issuerEvtID );
  *pBuf++ = pCmd->notifScheme;
  *pBuf++ = LO_UINT16( pCmd->notifFlagAttrID );
  *pBuf++ = HI_UINT16( pCmd->notifFlagAttrID);
  *pBuf++ = LO_UINT16( pCmd->clusterID );
  *pBuf++ = HI_UINT16( pCmd->clusterID );
  *pBuf++ = LO_UINT16( pCmd->manuCode );
  *pBuf++ = HI_UINT16( pCmd->manuCode );
  *pBuf++ = pCmd->numOfCmds;

  for ( cmd = 0; cmd < pCmd->numOfCmds; cmd++ )
  {
    *pBuf++ = pCmd->pCmdIDs[cmd];
  }

  status =  zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_METERING,
                             COMMAND_SE_METERING_CFG_NOTIF_FLAG, TRUE,
                             ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0,
                             seqNum, cmdBufLen, pCmdBuf );

  OsalPort_free( pCmdBuf );

  return ( status );
}

/**************************************************************************************************
 * @fn      zclSE_MeteringSendGetNotifMsg
 *
 * @brief   Send COMMAND_SE_METERING_GET_NOTIF_MSG.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MeteringSendGetNotifMsg( uint8_t srcEP, afAddrType_t *dstAddr,
                                         zclSE_MeteringGetNotifMsg_t *pCmd,
                                         uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t cmdBuf[ZCL_SE_METERING_GET_NOTIF_MSG_LEN];
  uint8_t *pBuf = cmdBuf;

  *pBuf++ = pCmd->notifScheme;
  *pBuf++ = LO_UINT16( pCmd->notifFlagAttrID );
  *pBuf++ = HI_UINT16( pCmd->notifFlagAttrID );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->notifFlags );

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_METERING,
                            COMMAND_SE_METERING_GET_NOTIF_MSG, TRUE,
                            ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0,
                            seqNum, ZCL_SE_METERING_GET_NOTIF_MSG_LEN, cmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_MeteringSendSupplyStatusRsp
 *
 * @brief   Send COMMAND_SE_METERING_SUPPLY_STATUS_RSP.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MeteringSendSupplyStatusRsp(
            uint8_t srcEP, afAddrType_t *dstAddr,
            zclSE_MeteringSupplyStatusRsp_t *pCmd,
            uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t cmdBuf[ZCL_SE_METERING_SUPPLY_STATUS_RSP_LEN];
  uint8_t *pBuf = cmdBuf;

  pBuf = OsalPort_bufferUint32( pBuf, pCmd->providerID );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->issuerEvtID );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->implTime );
  *pBuf++ = pCmd->supplyStatus;

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_METERING,
                            COMMAND_SE_METERING_SUPPLY_STATUS_RSP, TRUE,
                            ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0,
                            seqNum, ZCL_SE_METERING_SUPPLY_STATUS_RSP_LEN, cmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_MeteringSendStartSamplingRsp
 *
 * @brief   Send COMMAND_SE_METERING_START_SAMPLING_RSP.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MeteringSendStartSamplingRsp(
            uint8_t srcEP, afAddrType_t *dstAddr,
            zclSE_MeteringStartSamplingRsp_t *pCmd,
            uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t cmdBuf[ZCL_SE_METERING_START_SAMPLING_RSP_LEN];
  uint8_t *pBuf = cmdBuf;

  *pBuf++ = LO_UINT16( pCmd->sampleID );
  *pBuf++ = HI_UINT16( pCmd->sampleID );

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_METERING,
                            COMMAND_SE_METERING_START_SAMPLING_RSP, TRUE,
                            ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0,
                            seqNum, ZCL_SE_METERING_START_SAMPLING_RSP_LEN, cmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_MeteringSendGetProfile
 *
 * @brief   Send COMMAND_SE_METERING_GET_PROFILE.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MeteringSendGetProfile( uint8_t srcEP, afAddrType_t *dstAddr,
                                        zclSE_MeteringGetProfile_t *pCmd,
                                        uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t cmdBuf[ZCL_SE_METERING_GET_PROFILE_LEN];
  uint8_t *pBuf = cmdBuf;

  *pBuf++ = pCmd->intervalChan;
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->endTime );
  *pBuf++ = pCmd->numOfPeriods;

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_METERING,
                            COMMAND_SE_METERING_GET_PROFILE, TRUE,
                            ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0,
                            seqNum, ZCL_SE_METERING_GET_PROFILE_LEN, cmdBuf );

  return status;

}

/**************************************************************************************************
 * @fn      zclSE_MeteringSendReqMirrorRsp
 *
 * @brief   Send COMMAND_SE_METERING_REQ_MIRROR_RSP.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MeteringSendReqMirrorRsp( uint8_t srcEP, afAddrType_t *dstAddr,
                                          zclSE_MeteringReqMirrorRsp_t *pCmd,
                                          uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t cmdBuf[ZCL_SE_METERING_REQ_MIRROR_RSP_LEN];
  uint8_t *pBuf = cmdBuf;

  *pBuf++ = LO_UINT16( pCmd->epID );
  *pBuf++ = HI_UINT16( pCmd->epID );

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_METERING,
                            COMMAND_SE_METERING_REQ_MIRROR_RSP, TRUE,
                            ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0,
                            seqNum, ZCL_SE_METERING_REQ_MIRROR_RSP_LEN, cmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_MeteringSendMirrorRemoved
 *
 * @brief   Send COMMAND_SE_METERING_MIRROR_REMOVED.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MeteringSendMirrorRemoved( uint8_t srcEP, afAddrType_t *dstAddr,
                                           zclSE_MeteringMirrorRemoved_t *pCmd,
                                           uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t cmdBuf[ZCL_SE_METERING_MIRROR_REMOVED_LEN];
  uint8_t *pBuf = cmdBuf;

  *pBuf++ = LO_UINT16( pCmd->epID );
  *pBuf++ = HI_UINT16( pCmd->epID );

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_METERING,
                            COMMAND_SE_METERING_MIRROR_REMOVED, TRUE,
                            ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0,
                            seqNum, ZCL_SE_METERING_MIRROR_REMOVED_LEN, cmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_MeteringSendReqFastPollMode
 *
 * @brief   Send COMMAND_SE_METERING_REQ_FAST_POLL_MODE.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MeteringSendReqFastPollMode(
            uint8_t srcEP, afAddrType_t *dstAddr,
            zclSE_MeteringReqFastPollMode_t *pCmd,
            uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t cmdBuf[ZCL_SE_METERING_REQ_FAST_POLL_MODE_LEN];
  uint8_t *pBuf = cmdBuf;

  *pBuf++ = pCmd->updatePeriod;
  *pBuf++ = pCmd->duration;

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_METERING,
                            COMMAND_SE_METERING_REQ_FAST_POLL_MODE, TRUE,
                            ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0,
                            seqNum, ZCL_SE_METERING_REQ_FAST_POLL_MODE_LEN, cmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_MeteringSendScheduleSnapshot
 *
 * @brief   Send COMMAND_SE_METERING_SCHEDULE_SNAPSHOT.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MeteringSendScheduleSnapshot(
            uint8_t srcEP, afAddrType_t *dstAddr,
            zclSE_MeteringScheduleSnapshot_t *pCmd,
            uint8_t disableDefaultRsp, uint8_t seqNum )

{
  ZStatus_t status;
  uint8_t *pCmdBuf;
  uint16_t cmdBufLen = 0;
  uint8_t *pBuf;

  // Check for a non fragmented, valid "pSchedules" field, then calculate length
  if ( ( pCmd->cmdTotal <= 1 ) && ( pCmd->pSchedules != NULL ) )
  {
    cmdBufLen = pCmd->numOfSchedules * ZCL_SE_METERING_SNAPSHOT_SCHEDULE_LEN;
  }
  else if ( pCmd->pRawPayload != NULL )
  {
    // Fragmented -- use raw payload fields
    cmdBufLen = pCmd->rawPayloadLen;
  }

  // Allocate command buffer
  cmdBufLen = ZCL_SE_METERING_SCHEDULE_SNAPSHOT_LEN + cmdBufLen;

  pCmdBuf = OsalPort_malloc( cmdBufLen );
  if ( pCmdBuf == NULL )
  {
    return ZMemError;
  }

  pBuf = pCmdBuf;
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->issuerEvtID );
  *pBuf++ = pCmd->cmdIdx;
  *pBuf++ = pCmd->cmdTotal;

  // Check for a non fragmented, valid "pSchedules" field, then serialize
  if ( ( pCmd->cmdTotal <= 1 ) && ( pCmd->pSchedules != NULL ) )
  {
    uint8_t schedule;

    for ( schedule = 0; schedule < pCmd->numOfSchedules; schedule++ )
    {
      *pBuf++ = pCmd->pSchedules[schedule].scheduleID;
      pBuf = OsalPort_bufferUint32( pBuf, pCmd->pSchedules[schedule].startTime );
      pBuf = se_buffer_uint24( pBuf, pCmd->pSchedules[schedule].schedule );
      *pBuf++ = pCmd->pSchedules[schedule].payloadType;
      pBuf = OsalPort_bufferUint32( pBuf, pCmd->pSchedules[schedule].cause );
    }
  }
  else if ( pCmd->pRawPayload )
  {
    // Fragmented -- use raw payload fields
    pBuf = OsalPort_memcpy( pBuf, pCmd->pRawPayload, pCmd->rawPayloadLen );
  }

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_METERING,
                            COMMAND_SE_METERING_SCHEDULE_SNAPSHOT, TRUE,
                            ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0,
                            seqNum, cmdBufLen, pCmdBuf );

  OsalPort_free( pCmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_MeteringSendTakeSnapshot
 *
 * @brief   Send COMMAND_SE_METERING_TAKE_SNAPSHOT.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MeteringSendTakeSnapshot( uint8_t srcEP, afAddrType_t *dstAddr,
                                          zclSE_MeteringTakeSnapshot_t *pCmd,
                                          uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t cmdBuf[ZCL_SE_METERING_TAKE_SNAPSHOT_LEN];
  uint8_t *pBuf = cmdBuf;

  pBuf = OsalPort_bufferUint32( pBuf, pCmd->cause );

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_METERING,
                            COMMAND_SE_METERING_SCHEDULE_SNAPSHOT, TRUE,
                            ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0,
                            seqNum, ZCL_SE_METERING_TAKE_SNAPSHOT_LEN, cmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_MeteringSendGetSnapshot
 *
 * @brief   Send COMMAND_SE_METERING_GET_SNAPSHOT.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MeteringSendGetSnapshot( uint8_t srcEP, afAddrType_t *dstAddr,
                                         zclSE_MeteringGetSnapshot_t *pCmd,
                                         uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t cmdBuf[ZCL_SE_METERING_GET_SNAPSHOT_LEN];
  uint8_t *pBuf = cmdBuf;

  pBuf = OsalPort_bufferUint32( pBuf, pCmd->earliestStartTime );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->latestEndTime );
  *pBuf++ = pCmd->offset;
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->cause );

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_METERING,
                            COMMAND_SE_METERING_GET_SNAPSHOT, TRUE,
                            ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0,
                            seqNum, ZCL_SE_METERING_GET_SNAPSHOT_LEN, cmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_MeteringSendStartSampling
 *
 * @brief   Send COMMAND_SE_METERING_START_SAMPLING.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MeteringSendStartSampling( uint8_t srcEP, afAddrType_t *dstAddr,
                                           zclSE_MeteringStartSampling_t *pCmd,
                                           uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t cmdBuf[ZCL_SE_METERING_START_SAMPLING_LEN];
  uint8_t *pBuf = cmdBuf;

  pBuf = OsalPort_bufferUint32( pBuf, pCmd->issuerEvtID );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->startTime );
  *pBuf++ = pCmd->type;
  *pBuf++ = LO_UINT16( pCmd->reqInterval );
  *pBuf++ = HI_UINT16( pCmd->reqInterval );
  *pBuf++ = LO_UINT16( pCmd->maxNumOfSamples );
  *pBuf++ = HI_UINT16( pCmd->maxNumOfSamples );

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_METERING,
                            COMMAND_SE_METERING_START_SAMPLING, TRUE,
                            ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0,
                            seqNum, ZCL_SE_METERING_START_SAMPLING_LEN, cmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_MeteringSendGetSampledData
 *
 * @brief   Send COMMAND_SE_METERING_GET_SAMPLED_DATA.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MeteringSendGetSampledData( uint8_t srcEP, afAddrType_t *dstAddr,
                                            zclSE_MeteringGetSampledData_t *pCmd,
                                            uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t cmdBuf[ZCL_SE_METERING_GET_SAMPLED_DATA_LEN];
  uint8_t *pBuf = cmdBuf;

  *pBuf++ = LO_UINT16( pCmd->sampleID );
  *pBuf++ = HI_UINT16( pCmd->sampleID );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->earliestTime );
  *pBuf++ = pCmd->type;
  *pBuf++ = LO_UINT16( pCmd->numOfSamples );
  *pBuf++ = HI_UINT16( pCmd->numOfSamples );

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_METERING,
                            COMMAND_SE_METERING_GET_SAMPLED_DATA, TRUE,
                            ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0,
                            seqNum, ZCL_SE_METERING_GET_SAMPLED_DATA_LEN, cmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_MeteringSendMirrorReportAttrRsp
 *
 * @brief   Send COMMAND_SE_METERING_MIRROR_REPORT_ATTR_RSP.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MeteringSendMirrorReportAttrRsp(
            uint8_t srcEP, afAddrType_t *dstAddr,
            zclSE_MeteringMirrorReportAttrRsp_t *pCmd,
            uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t *pCmdBuf;
  uint16_t cmdBufLen;
  uint8_t *pBuf;
  uint8_t flag;

  // Allocate command buffer
  cmdBufLen = ZCL_SE_METERING_MIRROR_REPORT_ATTR_RSP_LEN +
              ( 4 * pCmd->numOfNotifFlags );

  pCmdBuf = OsalPort_malloc( cmdBufLen );
  if ( pCmdBuf == NULL )
  {
    return ZMemError;
  }

  pBuf = pCmdBuf;
  *pBuf++ = pCmd->notifScheme;

  for ( flag = 0; flag < pCmd->numOfNotifFlags; flag++ )
  {
    pBuf = OsalPort_bufferUint32( pBuf, pCmd->pNotifFlags[flag] );
  }


  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_METERING,
                            COMMAND_SE_METERING_MIRROR_REPORT_ATTR_RSP, TRUE,
                            ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0,
                            seqNum, cmdBufLen, pCmdBuf );

  OsalPort_free( pCmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_MeteringSendResetLoadLimitCntr
 *
 * @brief   Send COMMAND_SE_METERING_RESET_LOAD_LIMIT_CNTR.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MeteringSendResetLoadLimitCntr(
            uint8_t srcEP, afAddrType_t *dstAddr,
            zclSE_MeteringResetLoadLimitCntr_t *pCmd,
            uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t cmdBuf[ZCL_SE_METERING_RESET_LOAD_LIMIT_CNTR_LEN];
  uint8_t *pBuf = cmdBuf;

  pBuf = OsalPort_bufferUint32( pBuf, pCmd->providerID );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->issuerEvtID );

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_METERING,
                            COMMAND_SE_METERING_RESET_LOAD_LIMIT_CNTR, TRUE,
                            ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0,
                            seqNum, ZCL_SE_METERING_RESET_LOAD_LIMIT_CNTR_LEN, cmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_MeteringSendChangeSupply
 *
 * @brief   Send COMMAND_SE_METERING_CHANGE_SUPPLY.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MeteringSendChangeSupply( uint8_t srcEP, afAddrType_t *dstAddr,
                                          zclSE_MeteringChangeSupply_t *pCmd,
                                          uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t cmdBuf[ZCL_SE_METERING_CHANGE_SUPPLY_LEN];
  uint8_t *pBuf = cmdBuf;

  pBuf = OsalPort_bufferUint32( pBuf, pCmd->providerID );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->issuerEvtID );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->reqTime );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->implTime );
  *pBuf++ = pCmd->propSupplyStatus;
  *pBuf++ = pCmd->supplyCtrlBits;

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_METERING,
                            COMMAND_SE_METERING_CHANGE_SUPPLY, TRUE,
                            ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0,
                            seqNum, ZCL_SE_METERING_CHANGE_SUPPLY_LEN, cmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_MeteringSendLocalChangeSupply
 *
 * @brief   Send COMMAND_SE_METERING_LOCAL_CHANGE_SUPPLY.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MeteringSendLocalChangeSupply(
            uint8_t srcEP, afAddrType_t *dstAddr,
            zclSE_MeteringLocalChangeSupply_t *pCmd,
            uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t cmdBuf[ZCL_SE_METERING_LOCAL_CHANGE_SUPPLY_LEN];
  uint8_t *pBuf = cmdBuf;

  *pBuf++ = pCmd->propSupplyStatus;

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_METERING,
                            COMMAND_SE_METERING_LOCAL_CHANGE_SUPPLY, TRUE,
                            ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0,
                            seqNum, ZCL_SE_METERING_LOCAL_CHANGE_SUPPLY_LEN, cmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_MeteringSendSetSupplyStatus
 *
 * @brief   Send COMMAND_SE_METERING_SET_SUPPLY_STATUS.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MeteringSendSetSupplyStatus(
            uint8_t srcEP, afAddrType_t *dstAddr,
            zclSE_MeteringSetSupplyStatus_t *pCmd,
            uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t cmdBuf[ZCL_SE_METERING_SET_SUPPLY_STATUS_LEN];
  uint8_t *pBuf = cmdBuf;

  pBuf = OsalPort_bufferUint32( pBuf, pCmd->issuerEvtID );
  *pBuf++ = pCmd->tamperState;
  *pBuf++ = pCmd->depletionState;
  *pBuf++ = pCmd->unctrldFlowState;
  *pBuf++ = pCmd->loadLimitState;

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_METERING,
                            COMMAND_SE_METERING_SET_SUPPLY_STATUS, TRUE,
                            ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0,
                            seqNum, ZCL_SE_METERING_SET_SUPPLY_STATUS_LEN, cmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_MeteringSendSetUnctrldFlowThreshold
 *
 * @brief   Send COMMAND_SE_METERING_SET_UNCTRLD_FLOW_THRESHOLD.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MeteringSendSetUnctrldFlowThreshold(
            uint8_t srcEP, afAddrType_t *dstAddr,
            zclSE_MeteringSetUnctrldFlowThreshold_t *pCmd,
            uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t cmdBuf[ZCL_SE_METERING_SET_UNCTRLD_FLOW_THRESHOLD_LEN];
  uint8_t *pBuf = cmdBuf;

  pBuf = OsalPort_bufferUint32( pBuf, pCmd->providerID );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->issuerEvtID );
  *pBuf++ = LO_UINT16( pCmd->unctrldFlowState );
  *pBuf++ = HI_UINT16( pCmd->unctrldFlowState );
  *pBuf++ = pCmd->uom;
  *pBuf++ = LO_UINT16( pCmd->multi );
  *pBuf++ = HI_UINT16( pCmd->multi );
  *pBuf++ = LO_UINT16( pCmd->div );
  *pBuf++ = HI_UINT16( pCmd->div );
  *pBuf++ = pCmd->stabilisationPeriod;
  *pBuf++ = LO_UINT16( pCmd->measurementPeriod );
  *pBuf++ = HI_UINT16( pCmd->measurementPeriod );

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_METERING,
                            COMMAND_SE_METERING_SET_UNCTRLD_FLOW_THRESHOLD, TRUE,
                            ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0,
                            seqNum, ZCL_SE_METERING_SET_UNCTRLD_FLOW_THRESHOLD_LEN, cmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_MeteringParseGetProfileRsp
 *
 * @brief   Parse COMMAND_SE_METERING_GET_PROFILE_RSP.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MeteringParseGetProfileRsp( zclIncoming_t *pInMsg,
                                            zclSE_MeteringGetProfileRsp_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;
  uint8_t interval;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_METERING_GET_PROFILE_RSP_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->endTime = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->status = *pBuf++;
  pCmd->profileIntervalPeriod = *pBuf++;
  pCmd->numOfPeriodDlvd = *pBuf++;

  if ( pCmd->numOfPeriodDlvd )
  {
    // Allocate memory
    pCmd->pIntervals = OsalPort_malloc( pCmd->numOfPeriodDlvd * sizeof( uint32_t ) );
    if ( pCmd->pIntervals == NULL )
    {
      return ZCL_STATUS_SOFTWARE_FAILURE;
    }

    for ( interval = 0; interval < pCmd->numOfPeriodDlvd; interval++ )
    {
      pCmd->pIntervals[interval] = OsalPort_buildUint32( pBuf, 3 );
      pBuf += 3;
    }
  }
  else
  {
    pCmd->pIntervals = NULL;
  }

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_MeteringParseReqFastPollModeRsp
 *
 * @brief   Parse COMMAND_SE_METERING_REQ_FAST_POLL_MODE_RSP.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MeteringParseReqFastPollModeRsp(
            zclIncoming_t *pInMsg,
            zclSE_MeteringReqFastPollModeRsp_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_METERING_REQ_FAST_POLL_MODE_RSP_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->appliedUpdatePeriod = *pBuf++;
  pCmd->endTime = OsalPort_buildUint32( pBuf, 4 );

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_MeteringParseScheduleSnapshotRsp
 *
 * @brief   Parse COMMAND_SE_METERING_SCHEDULE_SNAPSHOT_RSP.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MeteringParseScheduleSnapshotRsp(
            zclIncoming_t *pInMsg,
            zclSE_MeteringScheduleSnapshotRsp_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;
  uint8_t snapshot;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_METERING_SCHEDULE_SNAPSHOT_RSP_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->issuerEvtID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->numOfEntries = ( pInMsg->pDataLen -
                         ZCL_SE_METERING_SCHEDULE_SNAPSHOT_RSP_LEN ) / 2;

  if ( pCmd->numOfEntries )
  {
    // Allocate memory
    pCmd->pEntries = OsalPort_malloc( pCmd->numOfEntries *
                                     sizeof( zclSE_MeteringScheduleRspEntry_t ) );
    if ( pCmd->pEntries == NULL )
    {
      return ZCL_STATUS_SOFTWARE_FAILURE;
    }

    for ( snapshot = 0; snapshot < pCmd->numOfEntries; snapshot++ )
    {
      pCmd->pEntries[snapshot].scheduleID = *pBuf++;
      pCmd->pEntries[snapshot].scheduleCfm = *pBuf++;
    }
  }
  else
  {
    pCmd->pEntries = NULL;
  }

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_MeteringParseTakeSnapshotRsp
 *
 * @brief   Parse COMMAND_SE_METERING_TAKE_SNAPSHOT_RSP.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MeteringParseTakeSnapshotRsp(
            zclIncoming_t *pInMsg,
            zclSE_MeteringTakeSnapshotRsp_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_METERING_TAKE_SNAPSHOT_RSP_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->snapshotID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->snapshotCfm = *pBuf;

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_MeteringParsePublishSnapshot
 *
 * @brief   Parse COMMAND_SE_METERING_PUBLISH_SNAPSHOT.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MeteringParsePublishSnapshot(
            zclIncoming_t *pInMsg,
            zclSE_MeteringPublishSnapshot_t *pCmd )
{
  ZStatus_t status = ZCL_STATUS_SUCCESS;
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_METERING_PUBLISH_SNAPSHOT_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->snapshotID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->time = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->totalFound = *pBuf++;
  pCmd->cmdIdx = *pBuf++;
  pCmd->cmdTotal = *pBuf++;
  pCmd->cause = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->payloadType = *pBuf++;
  pCmd->rawPayloadLen = pInMsg->pDataLen - ZCL_SE_METERING_PUBLISH_SNAPSHOT_LEN;

  if ( pCmd->rawPayloadLen )
  {
    pCmd->pRawPayload = pBuf;
  }
  else
  {
    pCmd->pRawPayload = NULL;
  }

  // Check for fragmentation
  if ( pCmd->cmdTotal <= 1 )
  {
    status = zclSE_MeteringSP_Parse( pCmd );
  }
  else
  {
    pCmd->payload.pTOU = NULL;
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_MeteringParseGetSampledDataRsp
 *
 * @brief   Parse COMMAND_SE_METERING_GET_SAMPLED_DATA_RSP.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MeteringParseGetSampledDataRsp(
            zclIncoming_t *pInMsg,
            zclSE_MeteringGetSampledDataRsp_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;
  uint8_t sample;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_METERING_GET_SAMPLED_DATA_RSP_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->sampleID = BUILD_UINT16( pBuf[0], pBuf[1] );
  pBuf += 2;
  pCmd->startTime = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->type = *pBuf++;
  pCmd->reqInterval = BUILD_UINT16( pBuf[0], pBuf[1] );
  pBuf += 2;
  pCmd->numOfSamples = BUILD_UINT16( pBuf[0], pBuf[1] );
  pBuf += 2;

  if ( pCmd->numOfSamples )
  {
    // Check for valid samples length
    if ( ( pInMsg->pDataLen - ZCL_SE_METERING_GET_SAMPLED_DATA_RSP_LEN ) <
         ( pCmd->numOfSamples * 3                                      )   )
    {
      return ZCL_STATUS_MALFORMED_COMMAND;
    }

    pCmd->pSamples = OsalPort_malloc( pCmd->numOfSamples * sizeof( uint32_t ) );
    if ( pCmd->pSamples == NULL )
    {
      return ZCL_STATUS_SOFTWARE_FAILURE;
    }

    for ( sample = 0; sample < pCmd->numOfSamples; sample++ )
    {
      pCmd->pSamples[sample] = OsalPort_buildUint32( pBuf, 3 );
      pBuf += 3;
    }
  }
  else
  {
    pCmd->pSamples = NULL;
  }

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_MeteringParseCfgMirror
 *
 * @brief   Parse COMMAND_SE_METERING_CFG_MIRROR.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MeteringParseCfgMirror( zclIncoming_t *pInMsg,
                                        zclSE_MeteringCfgMirror_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_METERING_CFG_MIRROR_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->issuerEvtID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->reportingInterval = OsalPort_buildUint32( pBuf, 3 );
  pBuf += 3;
  pCmd->mirrorNotifReporting = *pBuf++;
  pCmd->notifScheme = *pBuf;

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_MeteringParseCfgNotifScheme
 *
 * @brief   Parse COMMAND_SE_METERING_CFG_NOTIF_SCHEME.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MeteringParseCfgNotifScheme(
            zclIncoming_t *pInMsg,
            zclSE_MeteringCfgNotifScheme_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_METERING_CFG_NOTIF_SCHEME_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->issuerEvtID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->notifScheme = *pBuf++;
  pCmd->notifFlagOrder = OsalPort_buildUint32( pBuf, 4 );

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_MeteringParseCfgNotifFlag
 *
 * @brief   Parse COMMAND_SE_METERING_CFG_NOTIF_FLAG.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MeteringParseCfgNotifFlag( zclIncoming_t *pInMsg,
                                           zclSE_MeteringCfgNotifFlag_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;
  uint8_t cmd;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_METERING_CFG_NOTIF_FLAGS_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->issuerEvtID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->notifScheme = *pBuf++;
  pCmd->notifFlagAttrID = BUILD_UINT16( pBuf[0], pBuf[1] );
  pBuf += 2;
  pCmd->clusterID = BUILD_UINT16( pBuf[0], pBuf[1] );
  pBuf += 2;
  pCmd->manuCode = BUILD_UINT16( pBuf[0], pBuf[1] );
  pBuf += 2;
  pCmd->numOfCmds = *pBuf++;

  if ( pCmd->numOfCmds )
  {
    if ( ( pInMsg->pDataLen - ZCL_SE_METERING_CFG_NOTIF_FLAGS_LEN ) < pCmd->numOfCmds )
    {
      return ZCL_STATUS_MALFORMED_COMMAND;
    }

    pCmd->pCmdIDs = OsalPort_malloc( pCmd->numOfCmds );
    if ( pCmd->pCmdIDs == NULL )
    {
      return ZCL_STATUS_SOFTWARE_FAILURE;
    }

    for ( cmd = 0; cmd < pCmd->numOfCmds; cmd++ )
    {
      pCmd->pCmdIDs[cmd] = *pBuf++;
    }
  }
  else
  {
    pCmd->pCmdIDs = NULL;
  }

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_MeteringParseGetNotifMsg
 *
 * @brief   Parse COMMAND_SE_METERING_GET_NOTIF_MSG.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MeteringParseGetNotifMsg( zclIncoming_t *pInMsg,
                                          zclSE_MeteringGetNotifMsg_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_METERING_GET_NOTIF_MSG_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->notifScheme = *pBuf++;
  pCmd->notifFlagAttrID = BUILD_UINT16( pBuf[0], pBuf[1] );
  pBuf += 2;
  pCmd->notifFlags = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_MeteringParseSupplyStatusRsp
 *
 * @brief   Parse COMMAND_SE_METERING_SUPPLY_STATUS_RSP
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MeteringParseSupplyStatusRsp(
            zclIncoming_t *pInMsg,
            zclSE_MeteringSupplyStatusRsp_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_METERING_SUPPLY_STATUS_RSP_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->providerID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->issuerEvtID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->implTime = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->supplyStatus = *pBuf++;

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_MeteringParseStartSamplingRsp
 *
 * @brief   Parse COMMAND_SE_METERING_START_SAMPLING_RSP.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MeteringParseStartSamplingRsp(
            zclIncoming_t *pInMsg,
            zclSE_MeteringStartSamplingRsp_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_METERING_START_SAMPLING_RSP_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->sampleID = BUILD_UINT16( pBuf[0], pBuf[1] );
  pBuf += 2;

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_MeteringParseGetProfile
 *
 * @brief   Parse COMMAND_SE_METERING_GET_PROFILE.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MeteringParseGetProfile( zclIncoming_t *pInMsg,
                                         zclSE_MeteringGetProfile_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_METERING_GET_PROFILE_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->intervalChan = *pBuf++;
  pCmd->endTime = OsalPort_buildUint32( pBuf, 4 );
  pCmd->numOfPeriods = *pBuf++;

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_MeteringParseReqMirrorRsp
 *
 * @brief   Parse COMMAND_SE_METERING_REQ_MIRROR_RSP.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MeteringParseReqMirrorRsp( zclIncoming_t *pInMsg,
                                           zclSE_MeteringReqMirrorRsp_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_METERING_REQ_MIRROR_RSP_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->epID = BUILD_UINT16( pBuf[0], pBuf[1] );
  pBuf += 2;

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_MeteringParseMirrorRemoved
 *
 * @brief   Parse COMMAND_SE_METERING_MIRROR_REMOVED.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MeteringParseMirrorRemoved( zclIncoming_t *pInMsg,
                                            zclSE_MeteringMirrorRemoved_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_METERING_MIRROR_REMOVED_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->epID = BUILD_UINT16( pBuf[0], pBuf[1] );
  pBuf += 2;

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_MeteringParseReqFastPollMode
 *
 * @brief   Parse COMMAND_SE_METERING_REQ_FAST_POLL_MODE.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MeteringParseReqFastPollMode(
            zclIncoming_t *pInMsg,
            zclSE_MeteringReqFastPollMode_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_METERING_REQ_FAST_POLL_MODE_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->updatePeriod = *pBuf++;
  pCmd->duration = *pBuf++;

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_MeteringParseScheduleSnapshot
 *
 * @brief   Parse COMMAND_SE_METERING_SCHEDULE_SNAPSHOT.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MeteringParseScheduleSnapshot(
            zclIncoming_t *pInMsg,
            zclSE_MeteringScheduleSnapshot_t *pCmd )
{
  ZStatus_t status = ZCL_STATUS_SUCCESS;
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_METERING_SCHEDULE_SNAPSHOT_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->issuerEvtID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->cmdIdx = *pBuf++;
  pCmd->cmdTotal = *pBuf++;
  pCmd->rawPayloadLen = pInMsg->pDataLen - ZCL_SE_METERING_SCHEDULE_SNAPSHOT_LEN;

  if ( pCmd->rawPayloadLen )
  {
    pCmd->pRawPayload = pBuf;
  }
  else
  {
    pCmd->pRawPayload = NULL;
  }

  // Check for fragmentation
  if ( pCmd->cmdTotal <= 1 )
  {
    status = zclSE_MeteringSnapshotScheduleParse( pCmd );
  }
  else
  {
    pCmd->pSchedules = NULL;
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_MeteringParseTakeSnapshot
 *
 * @brief   Parse COMMAND_SE_METERING_TAKE_SNAPSHOT.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MeteringParseTakeSnapshot( zclIncoming_t *pInMsg,
                                           zclSE_MeteringTakeSnapshot_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_METERING_TAKE_SNAPSHOT_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->cause = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_MeteringParseGetSnapshot
 *
 * @brief   Parse COMMAND_SE_METERING_GET_SNAPSHOT.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MeteringParseGetSnapshot( zclIncoming_t *pInMsg,
                                          zclSE_MeteringGetSnapshot_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_METERING_GET_SNAPSHOT_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->earliestStartTime = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->latestEndTime = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->offset = *pBuf++;
  pCmd->cause = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_MeteringParseStartSampling
 *
 * @brief   Parse COMMAND_SE_METERING_START_SAMPLING.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MeteringParseStartSampling( zclIncoming_t *pInMsg,
                                            zclSE_MeteringStartSampling_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_METERING_START_SAMPLING_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->issuerEvtID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->startTime = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->type = *pBuf++;
  pCmd->reqInterval = BUILD_UINT16( pBuf[0], pBuf[1] );
  pBuf += 2;
  pCmd->maxNumOfSamples = BUILD_UINT16( pBuf[0], pBuf[1] );
  pBuf += 2;

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_MeteringParseGetSampledData
 *
 * @brief   Parse COMMAND_SE_METERING_GET_SAMPLED_DATA.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MeteringParseGetSampledData(
            zclIncoming_t *pInMsg,
            zclSE_MeteringGetSampledData_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_METERING_GET_SAMPLED_DATA_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->sampleID = BUILD_UINT16( pBuf[0], pBuf[1] );
  pBuf += 2;
  pCmd->earliestTime = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->type = *pBuf++;
  pCmd->numOfSamples = BUILD_UINT16( pBuf[0], pBuf[1] );
  pBuf += 2;

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_MeteringParseMirrorReportAttrRsp
 *
 * @brief   Parse COMMAND_SE_METERING_MIRROR_REPORT_ATTR_RSP.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MeteringParseMirrorReportAttrRsp(
            zclIncoming_t *pInMsg,
            zclSE_MeteringMirrorReportAttrRsp_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;
  uint8_t flag;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_METERING_MIRROR_REPORT_ATTR_RSP_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->notifScheme = *pBuf++;
  pCmd->numOfNotifFlags = ( pInMsg->pDataLen -
                            ZCL_SE_METERING_MIRROR_REPORT_ATTR_RSP_LEN ) / 4;

  if ( pCmd->numOfNotifFlags )
  {
    pCmd->pNotifFlags = OsalPort_malloc( 4 * pCmd->numOfNotifFlags );
    if ( pCmd->pNotifFlags == NULL )
    {
      return ZCL_STATUS_SOFTWARE_FAILURE;
    }

    for ( flag = 0; flag < pCmd->numOfNotifFlags; flag++ )
    {
      pCmd->pNotifFlags[flag] = OsalPort_buildUint32( pBuf, 4 );
      pBuf += 4;
    }
  }
  else
  {
    pCmd->pNotifFlags = NULL;
  }

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_MeteringParseResetLoadLimitCntr
 *
 * @brief   Parse COMMAND_SE_METERING_RESET_LOAD_LIMIT_CNTR.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MeteringParseResetLoadLimitCntr(
            zclIncoming_t *pInMsg,
            zclSE_MeteringResetLoadLimitCntr_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_METERING_RESET_LOAD_LIMIT_CNTR_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }
  pCmd->providerID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->issuerEvtID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_MeteringParseChangeSupply
 *
 * @brief   Parse COMMAND_SE_METERING_CHANGE_SUPPLY.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MeteringParseChangeSupply( zclIncoming_t *pInMsg,
                                           zclSE_MeteringChangeSupply_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_METERING_CHANGE_SUPPLY_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->providerID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->issuerEvtID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->reqTime = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->implTime = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->propSupplyStatus = *pBuf++;
  pCmd->supplyCtrlBits = *pBuf++;

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_MeteringParseLocalChangeSupply
 *
 * @brief   Parse COMMAND_SE_METERING_LOCAL_CHANGE_SUPPLY.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MeteringParseLocalChangeSupply(
            zclIncoming_t *pInMsg,
            zclSE_MeteringLocalChangeSupply_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_METERING_LOCAL_CHANGE_SUPPLY_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->propSupplyStatus = *pBuf++;

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_MeteringParseSetSupplyStatus
 *
 * @brief   Parse COMMAND_SE_METERING_SET_SUPPLY_STATUS.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MeteringParseSetSupplyStatus(
            zclIncoming_t *pInMsg,
            zclSE_MeteringSetSupplyStatus_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_METERING_SET_SUPPLY_STATUS_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->issuerEvtID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->tamperState = *pBuf++;
  pCmd->depletionState = *pBuf++;
  pCmd->unctrldFlowState = *pBuf++;
  pCmd->loadLimitState = *pBuf++;

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_MeteringParseSetUnctrldFlowThreshold
 *
 * @brief   Parse COMMAND_SE_METERING_SET_UNCTRLD_FLOW_THRESHOLD.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MeteringParseSetUnctrldFlowThreshold(
            zclIncoming_t *pInMsg,
            zclSE_MeteringSetUnctrldFlowThreshold_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_METERING_SET_UNCTRLD_FLOW_THRESHOLD_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->providerID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->issuerEvtID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->unctrldFlowState = BUILD_UINT16( pBuf[0], pBuf[1] );
  pBuf += 2;
  pCmd->uom = *pBuf++;
  pCmd->multi = BUILD_UINT16( pBuf[0], pBuf[1] );
  pBuf += 2;
  pCmd->div = BUILD_UINT16( pBuf[0], pBuf[1] );
  pBuf += 2;
  pCmd->stabilisationPeriod = *pBuf++;
  pCmd->measurementPeriod = BUILD_UINT16( pBuf[0], pBuf[1] );
  pBuf += 2;

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_MeteringHdlGetProfileRsp
 *
 * @brief   Handle COMMAND_SE_METERING_GET_PROFILE_RSP.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MeteringHdlGetProfileRsp( zclIncoming_t *pInMsg,
                                          zclSE_MeteringGetProfileRspCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_MeteringGetProfileRsp_t cmd;

    status = zclSE_MeteringParseGetProfileRsp( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      pfnCB( pInMsg, &cmd );

      if ( cmd.pIntervals != NULL )
      {
        OsalPort_free( cmd.pIntervals );
      }
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_MeteringHdlReqMirror
 *
 * @brief   Handle COMMAND_SE_METERING_REQ_MIRROR (handle with no payload).
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MeteringHdlReqMirror( zclIncoming_t *pInMsg,
                                      zclSE_MeteringReqMirrorCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    pfnCB( pInMsg );

    status = ZCL_STATUS_CMD_HAS_RSP;
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_MeteringHdlRemoveMirror
 *
 * @brief   Handle COMMAND_SE_METERING_REMOVE_MIRROR (handle with no payload).
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MeteringHdlRemoveMirror( zclIncoming_t *pInMsg,
                                         zclSE_MeteringRemoveMirrorCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    pfnCB( pInMsg );

    status = ZCL_STATUS_CMD_HAS_RSP;
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_MeteringHdlReqFastPollModeRsp
 *
 * @brief   Handle COMMAND_SE_METERING_REQ_FAST_POLL_MODE_RSP.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MeteringHdlReqFastPollModeRsp
            ( zclIncoming_t *pInMsg,
              zclSE_MeteringReqFastPollModeRspCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_MeteringReqFastPollModeRsp_t cmd;

    status = zclSE_MeteringParseReqFastPollModeRsp( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      pfnCB( pInMsg, &cmd );
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_MeteringHdlScheduleSnapshotRsp
 *
 * @brief   Handle COMMAND_SE_METERING_SCHEDULE_SNAPSHOT_RSP.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MeteringHdlScheduleSnapshotRsp
            ( zclIncoming_t *pInMsg,
              zclSE_MeteringScheduleSnapshotRspCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_MeteringScheduleSnapshotRsp_t cmd;

    status = zclSE_MeteringParseScheduleSnapshotRsp( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      pfnCB( pInMsg, &cmd );

      if ( cmd.pEntries != NULL )
      {
        OsalPort_free( cmd.pEntries );
      }
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_MeteringHdlTakeSnapshotRsp
 *
 * @brief   Handle COMMAND_SE_METERING_TAKE_SNAPSHOT_RSP.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MeteringHdlTakeSnapshotRsp( zclIncoming_t *pInMsg,
                                            zclSE_MeteringTakeSnapshotRspCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_MeteringTakeSnapshotRsp_t cmd;

    status = zclSE_MeteringParseTakeSnapshotRsp( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      pfnCB( pInMsg, &cmd );
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_MeteringHdlPublishSnapshot
 *
 * @brief   Handle COMMAND_SE_METERING_PUBLISH_SNAPSHOT.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MeteringHdlPublishSnapshot( zclIncoming_t *pInMsg,
                                            zclSE_MeteringPublishSnapshotCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_MeteringPublishSnapshot_t *pCmd;

    pCmd = OsalPort_malloc( sizeof( zclSE_MeteringPublishSnapshot_t ) );
    if ( pCmd == NULL )
    {
       return ZCL_STATUS_SOFTWARE_FAILURE;
    }

    status = zclSE_MeteringParsePublishSnapshot( pInMsg, pCmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      status = pfnCB( pInMsg, pCmd );

      if ( pCmd->payload.pTOU != NULL )
      {
        OsalPort_free( pCmd->payload.pTOU );
      }
    }

    OsalPort_free( pCmd );
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_MeteringHdlGetSampledDataRsp
 *
 * @brief   Handle COMMAND_SE_METERING_GET_SAMPLED_DATA_RSP.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MeteringHdlGetSampledDataRsp
            ( zclIncoming_t *pInMsg,
              zclSE_MeteringGetSampledDataRspCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_MeteringGetSampledDataRsp_t cmd;

    status = zclSE_MeteringParseGetSampledDataRsp( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      pfnCB( pInMsg, &cmd );

      if ( cmd.pSamples != NULL )
      {
        OsalPort_free( cmd.pSamples );
      }
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_MeteringHdlCfgMirror
 *
 * @brief   Handle COMMAND_SE_METERING_CFG_MIRROR.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MeteringHdlCfgMirror( zclIncoming_t *pInMsg,
                                      zclSE_MeteringCfgMirrorCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_MeteringCfgMirror_t cmd;

    status = zclSE_MeteringParseCfgMirror( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      status = pfnCB( pInMsg, &cmd );
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_MeteringHdlCfgNotifScheme
 *
 * @brief   Handle COMMAND_SE_METERING_CFG_NOTIF_SCHEME.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MeteringHdlCfgNotifScheme( zclIncoming_t *pInMsg,
                                           zclSE_MeteringCfgNotifSchemeCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_MeteringCfgNotifScheme_t cmd;

    status = zclSE_MeteringParseCfgNotifScheme( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      pfnCB( pInMsg, &cmd );
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_MeteringHdlCfgNotifFlag
 *
 * @brief   Handle COMMAND_SE_METERING_CFG_NOTIF_FLAG.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MeteringHdlCfgNotifFlag( zclIncoming_t *pInMsg,
                                         zclSE_MeteringCfgNotifFlagCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_MeteringCfgNotifFlag_t cmd;

    status = zclSE_MeteringParseCfgNotifFlag( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      pfnCB( pInMsg, &cmd );

      if ( cmd.pCmdIDs != NULL )
      {
        OsalPort_free( cmd.pCmdIDs );
      }
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_MeteringHdlGetNotifMsg
 *
 * @brief   Handle COMMAND_SE_METERING_GET_NOTIF_MSG.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MeteringHdlGetNotifMsg( zclIncoming_t *pInMsg,
                                        zclSE_MeteringGetNotifMsgCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_MeteringGetNotifMsg_t cmd;

    status = zclSE_MeteringParseGetNotifMsg( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      pfnCB( pInMsg, &cmd );

      status = ZCL_STATUS_CMD_HAS_RSP;
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_MeteringHdlSupplyStatusRsp
 *
 * @brief   Handle COMMAND_SE_METERING_SUPPLY_STATUS_RSP.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MeteringHdlSupplyStatusRsp( zclIncoming_t *pInMsg,
                                            zclSE_MeteringSupplyStatusRspCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_MeteringSupplyStatusRsp_t cmd;

    status = zclSE_MeteringParseSupplyStatusRsp( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      pfnCB( pInMsg, &cmd );
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_MeteringHdlStartSamplingRsp
 *
 * @brief   Handle COMMAND_SE_METERING_START_SAMPLING_RSP.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MeteringHdlStartSamplingRsp(
            zclIncoming_t *pInMsg,
            zclSE_MeteringStartSamplingRspCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_MeteringStartSamplingRsp_t cmd;

    status = zclSE_MeteringParseStartSamplingRsp( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      pfnCB( pInMsg, &cmd );
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_MeteringHdlGetProfile
 *
 * @brief   Handle COMMAND_SE_METERING_GET_PROFILE.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MeteringHdlGetProfile( zclIncoming_t *pInMsg,
                                       zclSE_MeteringGetProfileCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_MeteringGetProfile_t cmd;

    status = zclSE_MeteringParseGetProfile( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      pfnCB( pInMsg, &cmd );

      status = ZCL_STATUS_CMD_HAS_RSP;
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_MeteringHdlReqMirrorRsp
 *
 * @brief   Handle COMMAND_SE_METERING_REQ_MIRROR_RSP.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MeteringHdlReqMirrorRsp( zclIncoming_t *pInMsg,
                                         zclSE_MeteringReqMirrorRspCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_MeteringReqMirrorRsp_t cmd;

    status = zclSE_MeteringParseReqMirrorRsp( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      pfnCB( pInMsg, &cmd );
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_MeteringHdlMirrorRemoved
 *
 * @brief   Handle COMMAND_SE_METERING_MIRROR_REMOVED.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MeteringHdlMirrorRemoved( zclIncoming_t *pInMsg,
                                          zclSE_MeteringMirrorRemovedCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_MeteringMirrorRemoved_t cmd;

    status = zclSE_MeteringParseMirrorRemoved( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      pfnCB( pInMsg, &cmd );
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_MeteringHdlReqFastPollMode
 *
 * @brief   Handle COMMAND_SE_METERING_REQ_FAST_POLL_MODE.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MeteringHdlReqFastPollMode( zclIncoming_t *pInMsg,
                                            zclSE_MeteringReqFastPollModeCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_MeteringReqFastPollMode_t cmd;

    status = zclSE_MeteringParseReqFastPollMode( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      pfnCB( pInMsg, &cmd );

      status = ZCL_STATUS_CMD_HAS_RSP;
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_MeteringHdlScheduleSnapshot
 *
 * @brief   Handle COMMAND_SE_METERING_SCHEDULE_SNAPSHOT.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MeteringHdlScheduleSnapshot
            ( zclIncoming_t *pInMsg,
              zclSE_MeteringScheduleSnapshotCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_MeteringScheduleSnapshot_t cmd;

    status = zclSE_MeteringParseScheduleSnapshot( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      status = pfnCB( pInMsg, &cmd );

      if ( cmd.pSchedules != NULL )
      {
        OsalPort_free( cmd.pSchedules );
      }
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_MeteringHdlTakeSnapshot
 *
 * @brief   Handle COMMAND_SE_METERING_TAKE_SNAPSHOT.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MeteringHdlTakeSnapshot( zclIncoming_t *pInMsg,
                                         zclSE_MeteringTakeSnapshotCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_MeteringTakeSnapshot_t cmd;

    status = zclSE_MeteringParseTakeSnapshot( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      pfnCB( pInMsg, &cmd );

      status = ZCL_STATUS_CMD_HAS_RSP;
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_MeteringHdlGetSnapshot
 *
 * @brief   Handle COMMAND_SE_METERING_GET_SNAPSHOT.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MeteringHdlGetSnapshot( zclIncoming_t *pInMsg,
                                        zclSE_MeteringGetSnapshotCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_MeteringGetSnapshot_t cmd;

    status = zclSE_MeteringParseGetSnapshot( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      status = pfnCB( pInMsg, &cmd );
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_MeteringHdlStartSampling
 *
 * @brief   Handle COMMAND_SE_METERING_START_SAMPLING.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MeteringHdlStartSampling( zclIncoming_t *pInMsg,
                                          zclSE_MeteringStartSamplingCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_MeteringStartSampling_t cmd;

    status = zclSE_MeteringParseStartSampling( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      pfnCB( pInMsg, &cmd );

      status = ZCL_STATUS_CMD_HAS_RSP;
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_MeteringHdlGetSampledData
 *
 * @brief   Handle COMMAND_SE_METERING_GET_SAMPLED_DATA.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MeteringHdlGetSampledData( zclIncoming_t *pInMsg,
                                           zclSE_MeteringGetSampledDataCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_MeteringGetSampledData_t cmd;

    status = zclSE_MeteringParseGetSampledData( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      status = pfnCB( pInMsg, &cmd );
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_MeteringHdlMirrorReportAttrRsp
 *
 * @brief   Handle COMMAND_SE_METERING_MIRROR_REPORT_ATTR_RSP.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MeteringHdlMirrorReportAttrRsp
            ( zclIncoming_t *pInMsg,
              zclSE_MeteringMirrorReportAttrRspCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_MeteringMirrorReportAttrRsp_t cmd;

    status = zclSE_MeteringParseMirrorReportAttrRsp( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      pfnCB( pInMsg, &cmd );

      if ( cmd.pNotifFlags )
      {
        OsalPort_free( cmd.pNotifFlags );
      }
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_MeteringHdlResetLoadLimitCntr
 *
 * @brief   Handle COMMAND_SE_METERING_RESET_LOAD_LIMIT_CNTR.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MeteringHdlResetLoadLimitCntr
            ( zclIncoming_t *pInMsg,
              zclSE_MeteringResetLoadLimitCntrCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_MeteringResetLoadLimitCntr_t cmd;

    status = zclSE_MeteringParseResetLoadLimitCntr( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      pfnCB( pInMsg, &cmd );
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_MeteringHdlChangeSupply
 *
 * @brief   Handle COMMAND_SE_METERING_CHANGE_SUPPLY.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MeteringHdlChangeSupply( zclIncoming_t *pInMsg,
                                         zclSE_MeteringChangeSupplyCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_MeteringChangeSupply_t cmd;

    status = zclSE_MeteringParseChangeSupply( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      status = pfnCB( pInMsg, &cmd );
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_MeteringHdlLocalChangeSupply
 *
 * @brief   Handle COMMAND_SE_METERING_LOCAL_CHANGE_SUPPLY.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MeteringHdlLocalChangeSupply(
            zclIncoming_t *pInMsg,
            zclSE_MeteringLocalChangeSupplyCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_MeteringLocalChangeSupply_t cmd;

    status = zclSE_MeteringParseLocalChangeSupply( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      status = pfnCB( pInMsg, &cmd );
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_MeteringHdlSetSupplyStatus
 *
 * @brief   Handle COMMAND_SE_METERING_SET_SUPPLY_STATUS.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MeteringHdlSetSupplyStatus( zclIncoming_t *pInMsg,
                                            zclSE_MeteringSetSupplyStatusCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_MeteringSetSupplyStatus_t cmd;

    status = zclSE_MeteringParseSetSupplyStatus( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      pfnCB( pInMsg, &cmd );
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_MeteringHdlSetUnctrldFlowThreshold
 *
 * @brief   Handle COMMAND_SE_METERING_SET_UNCTRLD_FLOW_THRESHOLD.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MeteringHdlSetUnctrldFlowThreshold(
            zclIncoming_t *pInMsg,
            zclSE_MeteringSetUnctrldFlowThresholdCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_MeteringSetUnctrldFlowThreshold_t cmd;

    status = zclSE_MeteringParseSetUnctrldFlowThreshold( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      pfnCB( pInMsg, &cmd );
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_MeteringHdlClientCmd
 *
 * @brief   Client command handler for ZCL_CLUSTER_ID_SE_METERING.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCBs - client callbacks
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MeteringHdlClientCmd( zclIncoming_t *pInMsg,
                                      const zclSE_MeteringClientCBs_t *pCBs )
{
  ZStatus_t status;

  // Guard against NULL pointer
  if ( pCBs == NULL )
  {
    return ZCL_STATUS_FAILURE;
  }

  // Process Server-to-Client Commands
  switch ( pInMsg->hdr.commandID )
  {
    case COMMAND_SE_METERING_GET_PROFILE_RSP:
      status = zclSE_MeteringHdlGetProfileRsp( pInMsg, pCBs->pfnGetProfileRsp );
      break;

    case COMMAND_SE_METERING_REQ_MIRROR:
      status = zclSE_MeteringHdlReqMirror( pInMsg, pCBs->pfnReqMirror );
      break;

    case COMMAND_SE_METERING_REMOVE_MIRROR:
      status = zclSE_MeteringHdlRemoveMirror( pInMsg, pCBs->pfnRemoveMirror );
      break;

    case COMMAND_SE_METERING_REQ_FAST_POLL_MODE_RSP:
      status = zclSE_MeteringHdlReqFastPollModeRsp( pInMsg, pCBs->pfnReqFastPollModeRsp );
      break;

    case COMMAND_SE_METERING_SCHEDULE_SNAPSHOT_RSP:
      status = zclSE_MeteringHdlScheduleSnapshotRsp( pInMsg, pCBs->pfnScheduleSnapshotRsp );
      break;

    case COMMAND_SE_METERING_TAKE_SNAPSHOT_RSP:
      status = zclSE_MeteringHdlTakeSnapshotRsp( pInMsg, pCBs->pfnTakeSnapshotRsp );
      break;

    case COMMAND_SE_METERING_PUBLISH_SNAPSHOT:
      status = zclSE_MeteringHdlPublishSnapshot( pInMsg, pCBs->pfnPublishSnapshot );
      break;

    case COMMAND_SE_METERING_GET_SAMPLED_DATA_RSP:
      status = zclSE_MeteringHdlGetSampledDataRsp( pInMsg, pCBs->pfnGetSampledDataRsp );
      break;

    case COMMAND_SE_METERING_CFG_MIRROR:
      status = zclSE_MeteringHdlCfgMirror( pInMsg, pCBs->pfnCfgMirror );
      break;

    case COMMAND_SE_METERING_CFG_NOTIF_SCHEME:
      status = zclSE_MeteringHdlCfgNotifScheme( pInMsg, pCBs->pfnCfgNotifScheme );
      break;

    case COMMAND_SE_METERING_CFG_NOTIF_FLAG:
      status = zclSE_MeteringHdlCfgNotifFlag( pInMsg, pCBs->pfnCfgNotifFlag );
      break;

    case COMMAND_SE_METERING_GET_NOTIF_MSG:
      status = zclSE_MeteringHdlGetNotifMsg( pInMsg, pCBs->pfnGetNotifMsg );
      break;

    case COMMAND_SE_METERING_SUPPLY_STATUS_RSP:
      status = zclSE_MeteringHdlSupplyStatusRsp( pInMsg, pCBs->pfnSupplyStatusRsp );
      break;

    case COMMAND_SE_METERING_START_SAMPLING_RSP:
      status = zclSE_MeteringHdlStartSamplingRsp( pInMsg, pCBs->pfnStartSamplingRsp );
      break;

    default:
      status = ZCL_STATUS_FAILURE;
      break;
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_MeteringHdlServerCmd
 *
 * @brief   Server command handler for ZCL_CLUSTER_ID_SE_METERING.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCBs - server callbacks
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MeteringHdlServerCmd( zclIncoming_t *pInMsg,
                                      const zclSE_MeteringServerCBs_t *pCBs )
{
  ZStatus_t status;

  // Guard against NULL pointer
  if ( pCBs == NULL )
  {
    return ZCL_STATUS_FAILURE;
  }

  // Process Client-to-Server Commands
  switch ( pInMsg->hdr.commandID )
  {
    case COMMAND_SE_METERING_GET_PROFILE:
      status = zclSE_MeteringHdlGetProfile( pInMsg, pCBs->pfnGetProfile );
      break;

    case COMMAND_SE_METERING_REQ_MIRROR_RSP:
      status = zclSE_MeteringHdlReqMirrorRsp( pInMsg, pCBs->pfnReqMirrorRsp );
      break;

    case COMMAND_SE_METERING_MIRROR_REMOVED:
      status = zclSE_MeteringHdlMirrorRemoved( pInMsg, pCBs->pfnMirrorRemoved );
      break;

    case COMMAND_SE_METERING_REQ_FAST_POLL_MODE:
      status = zclSE_MeteringHdlReqFastPollMode( pInMsg, pCBs->pfnReqFastPollMode );
      break;

    case COMMAND_SE_METERING_SCHEDULE_SNAPSHOT:
      status = zclSE_MeteringHdlScheduleSnapshot( pInMsg, pCBs->pfnScheduleSnapshot );
      break;

    case COMMAND_SE_METERING_TAKE_SNAPSHOT:
      status = zclSE_MeteringHdlTakeSnapshot( pInMsg, pCBs->pfnTakeSnapshot );
      break;

    case COMMAND_SE_METERING_GET_SNAPSHOT:
      status = zclSE_MeteringHdlGetSnapshot( pInMsg, pCBs->pfnGetSnapshot );
      break;

    case COMMAND_SE_METERING_START_SAMPLING:
      status = zclSE_MeteringHdlStartSampling( pInMsg, pCBs->pfnStartSampling );
      break;

    case COMMAND_SE_METERING_GET_SAMPLED_DATA:
      status = zclSE_MeteringHdlGetSampledData( pInMsg, pCBs->pfnGetSampledData );
      break;

    case COMMAND_SE_METERING_MIRROR_REPORT_ATTR_RSP:
      status = zclSE_MeteringHdlMirrorReportAttrRsp( pInMsg, pCBs->pfnMirrorReportAttrRsp );
      break;

    case COMMAND_SE_METERING_RESET_LOAD_LIMIT_CNTR:
      status = zclSE_MeteringHdlResetLoadLimitCntr( pInMsg, pCBs->pfnResetLoadLimitCntr );
      break;

    case COMMAND_SE_METERING_CHANGE_SUPPLY:
      status = zclSE_MeteringHdlChangeSupply( pInMsg, pCBs->pfnChangeSupply );
      break;

    case COMMAND_SE_METERING_LOCAL_CHANGE_SUPPLY:
      status = zclSE_MeteringHdlLocalChangeSupply( pInMsg, pCBs->pfnLocalChangeSupply );
      break;

    case COMMAND_SE_METERING_SET_SUPPLY_STATUS:
      status = zclSE_MeteringHdlSetSupplyStatus( pInMsg, pCBs->pfnSetSupplyStatus );
      break;

    case COMMAND_SE_METERING_SET_UNCTRLD_FLOW_THRESHOLD:
      status = zclSE_MeteringHdlSetUnctrldFlowThreshold( pInMsg,
                                                         pCBs->pfnSetUnctrldFlowThreshold );
      break;

    default:
      status = ZCL_STATUS_FAILURE;
      break;
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_MeteringSP_Parse
 *
 * @brief   Utility function for parsing a snapshot payload (SP - SNAPSHOT PAYLOAD) -- see
 *          "zclSE_MeteringPublishSnapshot_t".
 *
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MeteringSP_Parse( zclSE_MeteringPublishSnapshot_t *pCmd )
{
  ZStatus_t status = ZCL_STATUS_SUCCESS;

  // Initialize in case of failure
  pCmd->payload.pTOU = NULL;

  // Check for valid raw fields
  if ( pCmd->rawPayloadLen && ( pCmd->pRawPayload == NULL ) )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  switch( pCmd->payloadType )
  {
    case ZCL_SE_METERING_SP_TOU_SET_DLVD:
    case ZCL_SE_METERING_SP_TOU_SET_RCVD:
      status = zclSE_MeteringSP_ParseTOU( pCmd );
      break;

    case ZCL_SE_METERING_SP_BLOCK_TIER_SET_DLVD:
    case ZCL_SE_METERING_SP_BLOCK_TIER_SET_RCVD:
      status = zclSE_MeteringSP_ParseBlockTier( pCmd );
      break;

    case ZCL_SE_METERING_SP_TOU_SET_DLVD_NO_BILL:
    case ZCL_SE_METERING_SP_TOU_SET_RCVD_NO_BILL:
      status = zclSE_MeteringSP_ParseTOU_NoBill( pCmd );
      break;

    case ZCL_SE_METERING_SP_BLOCK_TIER_SET_DLVD_NO_BILL:
    case ZCL_SE_METERING_SP_BLOCK_TIER_SET_RCVD_NO_BILL:
      status = zclSE_MeteringSP_ParseBlockTierNoBill( pCmd );
      break;

    case ZCL_SE_METERING_SP_DATA_UNAVAIL:
    default:
      // Unknown type - no payload
      break;
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_MeteringSnapshotScheduleParse
 *
 * @brief   Utility function for parsing a raw snapshot schedule command -- see
 *          "zclSE_MeteringScheduleSnapshot_t".
 *
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MeteringSnapshotScheduleParse( zclSE_MeteringScheduleSnapshot_t *pCmd )
{
  // Initialize in case of failure
  pCmd->pSchedules = NULL;

  // Check for valid raw fields
  if ( pCmd->rawPayloadLen && ( pCmd->pRawPayload == NULL ) )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  // Get the number of schedules
  pCmd->numOfSchedules = pCmd->rawPayloadLen / ZCL_SE_METERING_SNAPSHOT_SCHEDULE_LEN;

  if ( pCmd->numOfSchedules )
  {
    uint8_t *pBuf = pCmd->pRawPayload;
    uint8_t schedule;

    // Allocate memory for the schedule data
    pCmd->pSchedules = OsalPort_malloc( pCmd->numOfSchedules *
                                       sizeof( zclSE_MeteringSnapshotSchedule_t ) );
    if ( pCmd->pSchedules == NULL )
    {
      return ZCL_STATUS_SOFTWARE_FAILURE;
    }

    for ( schedule = 0; schedule < pCmd->numOfSchedules; schedule++ )
    {
      pCmd->pSchedules[schedule].scheduleID = *pBuf++;
      pCmd->pSchedules[schedule].startTime = OsalPort_buildUint32( pBuf, 4 );
      pBuf += 4;
      pCmd->pSchedules[schedule].schedule = OsalPort_buildUint32( pBuf, 3 );;
      pBuf += 3;
      pCmd->pSchedules[schedule].payloadType = *pBuf++;
      pCmd->pSchedules[schedule].cause = OsalPort_buildUint32( pBuf, 4 );
      pBuf += 4;
    }
  }

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_PriceSendPublishPrice
 *
 * @brief   Send COMMAND_SE_PRICE_PUBLISH_PRICE.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PriceSendPublishPrice( uint8_t srcEP, afAddrType_t *dstAddr,
                                       zclSE_PricePublishPrice_t *pCmd,
                                       uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t *pCmdBuf;
  uint16_t cmdBufLen;
  uint8_t *pBuf;

  // Allocate command buffer
  cmdBufLen = ZCL_SE_PRICE_PUBLISH_PRICE_LEN + zclSE_UTF8StringLen( &pCmd->rateLabel );

  pCmdBuf = OsalPort_malloc( cmdBufLen );
  if ( pCmdBuf == NULL )
  {
    return ZMemError;
  }

  pBuf = pCmdBuf;
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->providerID );
  pBuf = zclSE_UTF8StringBuild( pBuf, &pCmd->rateLabel );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->issuerEvtID );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->currTime );
  *pBuf++ = pCmd->uom;
  *pBuf++ = LO_UINT16( pCmd->currency );
  *pBuf++ = HI_UINT16( pCmd->currency );
  *pBuf = pCmd->priceTrailingDigit << 4;
  *pBuf++ |= pCmd->priceTier & 0x0F;
  *pBuf = pCmd->numOfPriceTiers << 4;
  *pBuf++ |= pCmd->regTier & 0x0F;
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->startTime );
  *pBuf++ = LO_UINT16( pCmd->duration );
  *pBuf++ = HI_UINT16( pCmd->duration );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->price );
  *pBuf++ = pCmd->priceRatio;
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->generationPrice );
  *pBuf++ = pCmd->generationPriceRatio;
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->altCostDlvd );
  *pBuf++ = pCmd->altCostUnit;
  *pBuf++ = pCmd->altCostTrailingDigit;
  *pBuf++ = pCmd->numOfBlockThresholds;
  *pBuf++ = pCmd->priceCtrl;
  *pBuf++ = pCmd->numOfGenerationTiers;
  *pBuf++ = pCmd->generationTier;
  *pBuf++ = pCmd->extNumOfPriceTiers;
  *pBuf++ = pCmd->extPriceTier;
  *pBuf++ = pCmd->extRegTier;

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_PRICE,
                            COMMAND_SE_PRICE_PUBLISH_PRICE, TRUE,
                            ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0,
                            seqNum, cmdBufLen, pCmdBuf );

  OsalPort_free( pCmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PriceSendPublishBlockPeriod
 *
 * @brief   Send COMMAND_SE_PRICE_PUBLISH_BLOCK_PERIOD.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PriceSendPublishBlockPeriod(
            uint8_t srcEP, afAddrType_t *dstAddr,
            zclSE_PricePublishBlockPeriod_t *pCmd,
            uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t cmdBuf[ZCL_SE_PRICE_PUBLISH_BLOCK_PERIOD_LEN];
  uint8_t *pBuf = cmdBuf;

  pBuf = OsalPort_bufferUint32( pBuf, pCmd->providerID );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->issuerEvtID );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->blockPeriodStartTime );
  pBuf = se_buffer_uint24( pBuf, pCmd->blockPeriodDuration );
  *pBuf++ = pCmd->blockPeriodCtrl;
  *pBuf = pCmd->blockPeriodDurationCtrl << 4;
  *pBuf++ |= pCmd->blockPeriodDurationTimeBase & 0x0F;
  *pBuf++ = pCmd->tariffType;
  *pBuf++ = pCmd->tariffResolutionPeriod;

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_PRICE,
                            COMMAND_SE_PRICE_PUBLISH_BLOCK_PERIOD, TRUE,
                            ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0,
                            seqNum, ZCL_SE_PRICE_PUBLISH_BLOCK_PERIOD_LEN, cmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PriceSendPublishConversionFactor
 *
 * @brief   Send COMMAND_SE_PRICE_PUBLISH_CONVERSION_FACTOR.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PriceSendPublishConversionFactor(
            uint8_t srcEP, afAddrType_t *dstAddr,
            zclSE_PricePublishConversionFactor_t *pCmd,
            uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t cmdBuf[ZCL_SE_PRICE_PUBLISH_CONVERSION_FACTOR_LEN];
  uint8_t *pBuf = cmdBuf;

  pBuf = OsalPort_bufferUint32( pBuf, pCmd->issuerEvtID );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->startTime );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->conversionFactor );
  *pBuf++ = pCmd->conversionFactorTrailingDigit;

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_PRICE,
                            COMMAND_SE_PRICE_PUBLISH_CONVERSION_FACTOR, TRUE,
                            ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0,
                            seqNum, ZCL_SE_PRICE_PUBLISH_CONVERSION_FACTOR_LEN, cmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PriceSendPublishCalorificValue
 *
 * @brief   Send COMMAND_SE_PRICE_PUBLISH_CALORIFIC_VALUE.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PriceSendPublishCalorificValue(
            uint8_t srcEP, afAddrType_t *dstAddr,
            zclSE_PricePublishCalorificValue_t *pCmd,
            uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t cmdBuf[ZCL_SE_PRICE_PUBLISH_CALORIFIC_VALUE_LEN];
  uint8_t *pBuf = cmdBuf;

  pBuf = OsalPort_bufferUint32( pBuf, pCmd->issuerEvtID );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->startTime );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->calorificValue );
  *pBuf++ = pCmd->calorificValueUnit;
  *pBuf++ = pCmd->calorificValueTrailingDigit;

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_PRICE,
                            COMMAND_SE_PRICE_PUBLISH_CALORIFIC_VALUE, TRUE,
                            ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0,
                            seqNum, ZCL_SE_PRICE_PUBLISH_CALORIFIC_VALUE_LEN, cmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PriceSendPublishTariffInfo
 *
 * @brief   Send COMMAND_SE_PRICE_PUBLISH_TARIFF_INFO.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PriceSendPublishTariffInfo( uint8_t srcEP, afAddrType_t *dstAddr,
                                            zclSE_PricePublishTariffInfo_t *pCmd,
                                            uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t *pCmdBuf;
  uint16_t cmdBufLen;
  uint8_t *pBuf;

  // Allocate command buffer
  cmdBufLen = ZCL_SE_PRICE_PUBLISH_TARIFF_INFO_LEN +
              zclSE_UTF8StringLen( &pCmd->tariffLabel );

  pCmdBuf = OsalPort_malloc( cmdBufLen );
  if ( pCmdBuf == NULL )
  {
    return ZMemError;
  }

  pBuf = pCmdBuf;
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->providerID );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->issuerEvtID );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->issuerTariffID );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->startTime );
  *pBuf = pCmd->chargingScheme << 4;
  *pBuf++ |= pCmd->tariffType & 0x0F;
  pBuf = zclSE_UTF8StringBuild( pBuf, &pCmd->tariffLabel );
  *pBuf++ = pCmd->priceTiersInUse;
  *pBuf++ = pCmd->blockThresholdsInUse;
  *pBuf++ = pCmd->uom;
  *pBuf++ = LO_UINT16( pCmd->currency );
  *pBuf++ = HI_UINT16( pCmd->currency );
  *pBuf++ = pCmd->priceTrailingDigit;
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->standingCharge );
  *pBuf++ = pCmd->tierBlockMode;
  pBuf = se_buffer_uint24( pBuf, pCmd->blockThresholdMult );
  pBuf = se_buffer_uint24( pBuf, pCmd->blockThresholdDiv );

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_PRICE,
                            COMMAND_SE_PRICE_PUBLISH_TARIFF_INFO, TRUE,
                            ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0,
                            seqNum, cmdBufLen, pCmdBuf );

  OsalPort_free( pCmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PriceSendPublishPriceMatrix
 *
 * @brief   Send COMMAND_SE_PRICE_PUBLISH_PRICE_MATRIX.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PriceSendPublishPriceMatrix(
            uint8_t srcEP, afAddrType_t *dstAddr,
            zclSE_PricePublishPriceMatrix_t *pCmd,
            uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t *pCmdBuf;
  uint16_t cmdBufLen = 0;
  uint8_t *pBuf;

  // Check for a non fragmented, valid "pSubFlds" field, then calculate length
  if ( ( pCmd->cmdTotal <= 1 ) && ( pCmd->pSubFlds != NULL ) )
  {
    cmdBufLen = pCmd->numOfSubFlds * ZCL_SE_PRICE_MATRIX_SUB_FLD_LEN;
  }
  else if ( pCmd->pRawPayload != NULL )
  {
    // Fragmented -- use raw payload fields
    cmdBufLen = pCmd->rawPayloadLen;
  }

  // Allocate command buffer
  cmdBufLen = ZCL_SE_PRICE_PUBLISH_PRICE_MATRIX_LEN + cmdBufLen;

  pCmdBuf = OsalPort_malloc( cmdBufLen );
  if ( pCmdBuf == NULL )
  {
    return ZMemError;
  }

  pBuf = pCmdBuf;
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->providerID );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->issuerEvtID );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->startTime );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->issuerTariffID );
  *pBuf++ = pCmd->cmdIdx;
  *pBuf++ = pCmd->cmdTotal;
  *pBuf++ = pCmd->subFldCtrl;

  // Check for a non fragmented, valid "pSubFlds" field, then serialize
  if ( ( pCmd->cmdTotal <= 1 ) && ( pCmd->pSubFlds != NULL ) )
  {
    uint8_t payload;

    for ( payload = 0; payload < pCmd->numOfSubFlds; payload++ )
    {
      if ( pCmd->subFldCtrl & 0x01 )
      {
        *pBuf++ = pCmd->pSubFlds[payload].tier;
      }
      else
      {
        *pBuf++ = ( ( pCmd->pSubFlds[payload].tier << 4      ) |
                    ( pCmd->pSubFlds[payload].blockID & 0x0F )  );
      }

      pBuf = OsalPort_bufferUint32( pBuf, pCmd->pSubFlds[payload].price );
    }
  }
  else if ( pCmd->pRawPayload )
  {
    // Fragmented -- use raw payload fields
    pBuf = OsalPort_memcpy( pBuf, pCmd->pRawPayload, pCmd->rawPayloadLen );
  }

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_PRICE,
                            COMMAND_SE_PRICE_PUBLISH_PRICE_MATRIX, TRUE,
                            ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0,
                            seqNum, cmdBufLen, pCmdBuf );

  OsalPort_free( pCmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PriceSendPublishBlockThreshold
 *
 * @brief   Send COMMAND_SE_PRICE_PUBLISH_BLOCK_THRESHOLD.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PriceSendPublishBlockThreshold(
            uint8_t srcEP, afAddrType_t *dstAddr,
            zclSE_PricePublishBlockThreshold_t *pCmd,
            uint8_t disableDefaultRsp, uint8_t seqNum )
{

  ZStatus_t status;
  uint8_t *pCmdBuf;
  uint16_t cmdBufLen = 0;
  uint8_t *pBuf;
  uint8_t fld;
  uint8_t thrsh;

  // Check for a non fragmented, valid "pSubFlds" field, then calculate length
  if ( ( pCmd->cmdTotal <= 1 ) && ( pCmd->pSubFlds != NULL ) )
  {
    for ( fld = 0; fld < pCmd->numOfSubFlds; fld++ )
    {
      cmdBufLen += ZCL_SE_PRICE_BLOCK_THRESHOLD_SUB_FLD_LEN +
                    ( pCmd->pSubFlds[fld].numOfBlockThresholds *
                      ZCL_SE_PRICE_BLOCK_THRESHOLD_LEN );
    }
  }
  else if ( pCmd->pRawPayload != NULL )
  {
    // Fragmented -- use raw payload fields
    cmdBufLen = pCmd->rawPayloadLen;
  }

  // Allocate command buffer
  cmdBufLen = ZCL_SE_PRICE_PUBLISH_BLOCK_THRESHOLD_LEN + cmdBufLen;

  pCmdBuf = OsalPort_malloc( cmdBufLen );
  if ( pCmdBuf == NULL )
  {
    return ZMemError;
  }

  pBuf = pCmdBuf;
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->providerID );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->issuerEvtID );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->startTime );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->issuerTariffID );
  *pBuf++ = pCmd->cmdIdx;
  *pBuf++ = pCmd->cmdTotal;
  *pBuf++ = pCmd->subFldCtrl;

  // Check for a non fragmented, valid "pSubFlds" field, then serialize
  if ( ( pCmd->cmdTotal <= 1 ) && ( pCmd->pSubFlds != NULL ) )
  {
    for ( fld = 0; fld < pCmd->numOfSubFlds; fld++ )
    {
      *pBuf++ = ( ( pCmd->pSubFlds[fld].tier << 4                   ) |
                  ( pCmd->pSubFlds[fld].numOfBlockThresholds & 0x0F )   );

      for ( thrsh = 0; thrsh < pCmd->pSubFlds[fld].numOfBlockThresholds; thrsh++ )
      {
        pBuf = OsalPort_memcpy( pBuf, pCmd->pSubFlds[fld].pBlockThresholds[thrsh],
                            ZCL_SE_PRICE_BLOCK_THRESHOLD_LEN );
      }
    }
  }
  else if ( pCmd->pRawPayload )
  {
    // Fragmented -- use raw payload fields
    pBuf = OsalPort_memcpy( pBuf, pCmd->pRawPayload, pCmd->rawPayloadLen );
  }

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_PRICE,
                            COMMAND_SE_PRICE_PUBLISH_BLOCK_THRESHOLD, TRUE,
                            ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0,
                            seqNum, cmdBufLen, pCmdBuf );

  OsalPort_free( pCmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PriceSendPublishCO2Value
 *
 * @brief   Send COMMAND_SE_PRICE_PUBLISH_CO2_VALUE.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PriceSendPublishCO2Value( uint8_t srcEP, afAddrType_t *dstAddr,
                                          zclSE_PricePublishCO2Value_t *pCmd,
                                          uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t cmdBuf[ZCL_SE_PRICE_PUBLISH_CO2_VALUE_LEN];
  uint8_t *pBuf = cmdBuf;

  pBuf = OsalPort_bufferUint32( pBuf, pCmd->providerID );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->issuerEvtID );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->startTime );
  *pBuf++ = pCmd->tariffType;
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->co2Value );
  *pBuf++ = pCmd->co2ValueUnit;
  *pBuf = pCmd->co2ValueTrailingDigit;

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_PRICE,
                            COMMAND_SE_PRICE_PUBLISH_CO2_VALUE, TRUE,
                            ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0,
                            seqNum, ZCL_SE_PRICE_PUBLISH_CO2_VALUE_LEN, cmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PriceSendPublishTierLabels
 *
 * @brief   Send COMMAND_SE_PRICE_PUBLISH_TIER_LABELS.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PriceSendPublishTierLabels( uint8_t srcEP, afAddrType_t *dstAddr,
                                            zclSE_PricePublishTierLabels_t *pCmd,
                                            uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t *pCmdBuf;
  uint16_t cmdBufLen = 0;
  uint8_t *pBuf;
  uint8_t label;

  // Check for a non fragmented, valid "pTierLabels" field, then calculate length
  if ( ( pCmd->cmdTotal <= 1 ) && ( pCmd->pTierLabels != NULL ) )
  {
    for ( label = 0; label < pCmd->numOfTierLabels; label++ )
    {
      cmdBufLen += ( ZCL_SE_PRICE_TIER_LABEL_LEN +
                     zclSE_UTF8StringLen( &pCmd->pTierLabels[label].tierLabel ) );
    }
  }
  else if ( pCmd->pRawPayload != NULL )
  {
    // Fragmented -- use raw payload fields
    cmdBufLen = pCmd->rawPayloadLen;
  }

  // Allocate command buffer
  cmdBufLen = ZCL_SE_PRICE_PUBLISH_TIER_LABELS_LEN + cmdBufLen;

  pCmdBuf = OsalPort_malloc( cmdBufLen );
  if ( pCmdBuf == NULL )
  {
    return ZMemError;
  }

  pBuf = pCmdBuf;
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->providerID );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->issuerEvtID );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->issuerTariffID );
  *pBuf++ = pCmd->cmdIdx;
  *pBuf++ = pCmd->cmdTotal;
  *pBuf++ = pCmd->numOfTierLabels;

  // Check for a non fragmented, valid "pTierLabels" field, then serialize
  if ( ( pCmd->cmdTotal <= 1 ) && ( pCmd->pTierLabels != NULL ) )
  {
    for ( label = 0; label < pCmd->numOfTierLabels; label++ )
    {
      *pBuf++ = pCmd->pTierLabels[label].tierID;
      pBuf = zclSE_UTF8StringBuild( pBuf, &pCmd->pTierLabels[label].tierLabel );
    }
  }
  else if ( pCmd->pRawPayload )
  {
    // Fragmented -- use raw payload fields
    pBuf = OsalPort_memcpy( pBuf, pCmd->pRawPayload, pCmd->rawPayloadLen );
  }

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_PRICE,
                            COMMAND_SE_PRICE_PUBLISH_TIER_LABELS, TRUE,
                            ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0,
                            seqNum, cmdBufLen, pCmdBuf );

  OsalPort_free( pCmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PriceSendPublishBillPeriod
 *
 * @brief   Send COMMAND_SE_PRICE_PUBLISH_BILL_PERIOD.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PriceSendPublishBillPeriod( uint8_t srcEP, afAddrType_t *dstAddr,
                                            zclSE_PricePublishBillPeriod_t *pCmd,
                                            uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t cmdBuf[ZCL_SE_PRICE_PUBLISH_BILL_PERIOD_LEN ];
  uint8_t *pBuf = cmdBuf;

  pBuf = OsalPort_bufferUint32( pBuf, pCmd->providerID );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->issuerEvtID );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->billPeriodStartTime );
  pBuf = se_buffer_uint24( pBuf, pCmd->billPeriodDuration );
  *pBuf = pCmd->billPeriodDurationTimeCtrl << 4;
  *pBuf++ |= pCmd->billPeriodDurationTimeBase & 0x0F;
  *pBuf = pCmd->tariffType;

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_PRICE,
                            COMMAND_SE_PRICE_PUBLISH_BILL_PERIOD, TRUE,
                            ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0,
                            seqNum, ZCL_SE_PRICE_PUBLISH_BILL_PERIOD_LEN, cmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PriceSendPublishConsolidatedBill
 *
 * @brief   Send COMMAND_SE_PRICE_PUBLISH_CONSOLIDATED_BILL.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
Status_t zclSE_PriceSendPublishConsolidatedBill(
           uint8_t srcEP, afAddrType_t *dstAddr,
           zclSE_PricePublishConsolidatedBill_t *pCmd,
           uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t *pCmdBuf;
  uint16_t cmdBufLen;
  uint8_t *pBuf;

  // Allocate command buffer
  cmdBufLen = ZCL_SE_PRICE_PUBLISH_CONSOLIDATED_BILL_LEN;

  pCmdBuf = OsalPort_malloc( cmdBufLen );
  if ( pCmdBuf == NULL )
  {
    return ZMemError;
  }

  pBuf = pCmdBuf;
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->providerID );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->issuerEvtID );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->billPeriodStartTime );
  pBuf = se_buffer_uint24( pBuf, pCmd->billPeriodDuration );
  *pBuf = pCmd->billPeriodDurationTimeCtrl << 4;
  *pBuf++ |= pCmd->billPeriodDurationTimeBase & 0x0F;
  *pBuf++ = pCmd->tariffType;
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->consolidatedBill );
  *pBuf++ = LO_UINT16( pCmd->currency );
  *pBuf++ = HI_UINT16( pCmd->currency );
  *pBuf = pCmd->billTrailingDigit;

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_PRICE,
                            COMMAND_SE_PRICE_PUBLISH_CONSOLIDATED_BILL, TRUE,
                            ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0,
                            seqNum, cmdBufLen, pCmdBuf );

  OsalPort_free( pCmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PriceSendPublishCPPEvt
 *
 * @brief   Send COMMAND_SE_PRICE_PUBLISH_CPP_EVT.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PriceSendPublishCPPEvt( uint8_t srcEP, afAddrType_t *dstAddr,
                                        zclSE_PricePublishCPPEvt_t *pCmd,
                                        uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t cmdBuf[ZCL_SE_PRICE_PUBLISH_CPP_EVT_LEN];
  uint8_t *pBuf = cmdBuf;

  pBuf = OsalPort_bufferUint32( pBuf, pCmd->providerID );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->issuerEvtID );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->startTime );
  *pBuf++ = LO_UINT16( pCmd->duration );
  *pBuf++ = HI_UINT16( pCmd->duration );
  *pBuf++ = pCmd->tariffType;
  *pBuf++ = pCmd->cppPriceTier;
  *pBuf = pCmd->cppAuth;

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_PRICE,
                            COMMAND_SE_PRICE_PUBLISH_CPP_EVT, TRUE,
                            ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0,
                            seqNum, ZCL_SE_PRICE_PUBLISH_CPP_EVT_LEN, cmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PriceSendPublishCreditPayment
 *
 * @brief   Send COMMAND_SE_PRICE_PUBLISH_CREDIT_PAYMENT.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PriceSendPublishCreditPayment(
            uint8_t srcEP, afAddrType_t *dstAddr,
            zclSE_PricePublishCreditPayment_t *pCmd,
            uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t *pBuf;
  uint8_t *pCmdBuf;
  uint16_t cmdBufLen;

  // Allocate command buffer
  cmdBufLen = ZCL_SE_PRICE_PUBLISH_CREDIT_PAYMENT_LEN +
              zclSE_UTF8StringLen( &pCmd->creditPaymentRef );

  pCmdBuf = OsalPort_malloc( cmdBufLen );
  if ( pCmdBuf == NULL )
  {
    return ZMemError;
  }

  pBuf = pCmdBuf;
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->providerID );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->issuerEvtID );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->creditPaymentDueDate );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->creditPaymentOverdueAmnt );
  *pBuf++ = pCmd->creditPaymentStatus;
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->creditPayment );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->creditPaymentDate );
  pBuf = zclSE_UTF8StringBuild( pBuf, &pCmd->creditPaymentRef );

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_PRICE,
                            COMMAND_SE_PRICE_PUBLISH_CREDIT_PAYMENT, TRUE,
                            ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0,
                            seqNum, cmdBufLen, pCmdBuf );

  OsalPort_free( pCmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PriceSendPublishCurrencyConversion
 *
 * @brief   Send COMMAND_SE_PRICE_PUBLISH_CURRENCY_CONVERSION.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PriceSendPublishCurrencyConversion(
            uint8_t srcEP, afAddrType_t *dstAddr,
            zclSE_PricePublishCurrencyConversion_t *pCmd,
            uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t *pCmdBuf;
  uint16_t cmdBufLen;
  uint8_t *pBuf;

  // Allocate command buffer
  cmdBufLen = ZCL_SE_PRICE_PUBLISH_CURRENCY_CONVERSION_LEN;

  pCmdBuf = OsalPort_malloc( cmdBufLen );
  if ( pCmdBuf == NULL )
  {
    return ZMemError;
  }

  pBuf = pCmdBuf;
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->providerID );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->issuerEvtID );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->startTime );
  *pBuf++ = LO_UINT16( pCmd->oldCurrency );
  *pBuf++ = HI_UINT16( pCmd->oldCurrency );
  *pBuf++ = LO_UINT16( pCmd->newCurrency );
  *pBuf++ = HI_UINT16( pCmd->newCurrency );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->conversionFactor );
  *pBuf++ = pCmd->conversionFactorTrailingDigit;
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->currencyChangeCtrlFlag );

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_PRICE,
                            COMMAND_SE_PRICE_PUBLISH_CURRENCY_CONVERSION, TRUE,
                            ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0,
                            seqNum, cmdBufLen, pCmdBuf );

  OsalPort_free( pCmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PriceSendCancelTariff
 *
 * @brief   Send COMMAND_SE_PRICE_CANCEL_TARIFF.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PriceSendCancelTariff( uint8_t srcEP, afAddrType_t *dstAddr,
                                       zclSE_PriceCancelTariff_t *pCmd,
                                       uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t cmdBuf[ZCL_SE_PRICE_CANCEL_TARIFF_LEN];
  uint8_t *pBuf = cmdBuf;

  pBuf = OsalPort_bufferUint32( pBuf, pCmd->providerID );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->issuerTariffID );
  *pBuf++ = pCmd->tariffType;

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_PRICE,
                            COMMAND_SE_PRICE_CANCEL_TARIFF, TRUE,
                            ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0,
                            seqNum, ZCL_SE_PRICE_CANCEL_TARIFF_LEN, cmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PriceSendGetCurrPrice
 *
 * @brief   Send COMMAND_SE_PRICE_GET_CURR_PRICE.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PriceSendGetCurrPrice( uint8_t srcEP, afAddrType_t *dstAddr,
                                       zclSE_PriceGetCurrPrice_t *pCmd,
                                       uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t cmdBuf[ZCL_SE_PRICE_GET_CURR_PRICE_LEN];
  uint8_t *pBuf = cmdBuf;

  *pBuf++ = pCmd->options;

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_PRICE,
                            COMMAND_SE_PRICE_GET_CURR_PRICE, TRUE,
                            ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0,
                            seqNum, ZCL_SE_PRICE_GET_CURR_PRICE_LEN, cmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PriceSendGetScheduledPrice
 *
 * @brief   Send COMMAND_SE_PRICE_GET_SCHEDULED_PRICE.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PriceSendGetScheduledPrice( uint8_t srcEP, afAddrType_t *dstAddr,
                                            zclSE_PriceGetScheduledPrice_t *pCmd,
                                            uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t cmdBuf[ZCL_SE_PRICE_GET_SCHEDULED_PRICE_LEN];
  uint8_t *pBuf = cmdBuf;

  pBuf = OsalPort_bufferUint32( pBuf, pCmd->startTime );
  *pBuf++ = pCmd->numOfEvts;

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_PRICE,
                            COMMAND_SE_PRICE_GET_SCHEDULED_PRICE, TRUE,
                            ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0,
                            seqNum, ZCL_SE_PRICE_GET_SCHEDULED_PRICE_LEN, cmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PriceSendPriceAck
 *
 * @brief   Send COMMAND_SE_PRICE_PRICE_ACK.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PriceSendPriceAck( uint8_t srcEP, afAddrType_t *dstAddr,
                                   zclSE_PricePriceAck_t *pCmd,
                                   uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t cmdBuf[ZCL_SE_PRICE_PRICE_ACK_LEN];
  uint8_t *pBuf = cmdBuf;

  pBuf = OsalPort_bufferUint32( pBuf, pCmd->providerID );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->issuerEvtID );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->priceAckTime );
  *pBuf++ = pCmd->ctrl;

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_PRICE,
                            COMMAND_SE_PRICE_PRICE_ACK, TRUE,
                            ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0,
                            seqNum, ZCL_SE_PRICE_PRICE_ACK_LEN, cmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PriceSendGetBlockPeriod
 *
 * @brief   Send COMMAND_SE_PRICE_GET_BLOCK_PERIOD.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PriceSendGetBlockPeriod( uint8_t srcEP, afAddrType_t *dstAddr,
                                         zclSE_PriceGetBlockPeriod_t *pCmd,
                                         uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t cmdBuf[ZCL_SE_PRICE_GET_BLOCK_PERIOD_LEN];
  uint8_t *pBuf = cmdBuf;

  pBuf = OsalPort_bufferUint32( pBuf, pCmd->startTime );
  *pBuf++ = pCmd->numOfEvts;
  *pBuf = pCmd->tariffType;

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_PRICE,
                            COMMAND_SE_PRICE_GET_BLOCK_PERIOD, TRUE,
                            ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0,
                            seqNum, ZCL_SE_PRICE_GET_BLOCK_PERIOD_LEN, cmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PriceSendGetConversionFactor
 *
 * @brief   Send COMMAND_SE_PRICE_GET_CONVERSION_FACTOR.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PriceSendGetConversionFactor(
            uint8_t srcEP, afAddrType_t *dstAddr,
            zclSE_PriceGetConversionFactor_t *pCmd,
            uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t cmdBuf[ZCL_SE_PRICE_GET_CONVERSION_FACTOR_LEN];
  uint8_t *pBuf = cmdBuf;

  pBuf = OsalPort_bufferUint32( pBuf, pCmd->earliestStartTime );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->minIssuerEvtID );
  *pBuf++ = pCmd->numOfCmds;

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_PRICE,
                            COMMAND_SE_PRICE_GET_CONVERSION_FACTOR, TRUE,
                            ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0,
                            seqNum, ZCL_SE_PRICE_GET_CONVERSION_FACTOR_LEN, cmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PriceSendGetCalorificValue
 *
 * @brief   Send COMMAND_SE_PRICE_GET_CALORIFIC_VALUE.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PriceSendGetCalorificValue( uint8_t srcEP, afAddrType_t *dstAddr,
                                            zclSE_PriceGetCalorificValue_t *pCmd,
                                            uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t cmdBuf[ZCL_SE_PRICE_GET_CALORIFIC_VALUE_LEN];
  uint8_t *pBuf = cmdBuf;

  pBuf = OsalPort_bufferUint32( pBuf, pCmd->earliestStartTime );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->minIssuerEvtID );
  *pBuf++ = pCmd->numOfCmds;

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_PRICE,
                            COMMAND_SE_PRICE_GET_CALORIFIC_VALUE, TRUE,
                            ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0,
                            seqNum, ZCL_SE_PRICE_GET_CALORIFIC_VALUE_LEN, cmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PriceSendGetTariffInfo
 *
 * @brief   Send COMMAND_SE_PRICE_GET_TARIFF_INFO.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PriceSendGetTariffInfo( uint8_t srcEP, afAddrType_t *dstAddr,
                                        zclSE_PriceGetTariffInfo_t *pCmd,
                                        uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t cmdBuf[ZCL_SE_PRICE_GET_TARIFF_INFO_LEN];
  uint8_t *pBuf = cmdBuf;

  pBuf = OsalPort_bufferUint32( pBuf, pCmd->earliestStartTime );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->minIssuerEvtID );
  *pBuf++ = pCmd->numOfCmds;
  *pBuf++ = pCmd->tariffType;

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_PRICE,
                            COMMAND_SE_PRICE_GET_TARIFF_INFO, TRUE,
                            ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0,
                            seqNum, ZCL_SE_PRICE_GET_TARIFF_INFO_LEN, cmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PriceSendGetPriceMatrix
 *
 * @brief   Send COMMAND_SE_PRICE_GET_PRICE_MATRIX.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PriceSendGetPriceMatrix( uint8_t srcEP, afAddrType_t *dstAddr,
                                         zclSE_PriceGetPriceMatrix_t *pCmd,
                                         uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t cmdBuf[ZCL_SE_PRICE_GET_PRICE_MATRIX_LEN];
  uint8_t *pBuf = cmdBuf;

  pBuf = OsalPort_bufferUint32( pBuf, pCmd->issuerTariffID );

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_PRICE,
                            COMMAND_SE_PRICE_GET_PRICE_MATRIX, TRUE,
                            ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0,
                            seqNum, ZCL_SE_PRICE_GET_PRICE_MATRIX_LEN, cmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PriceSendGetBlockThresholds
 *
 * @brief   Send COMMAND_SE_PRICE_GET_BLOCK_THRESHOLDS.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PriceSendGetBlockThresholds(
            uint8_t srcEP, afAddrType_t *dstAddr,
            zclSE_PriceGetBlockThresholds_t *pCmd,
            uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t cmdBuf[ZCL_SE_PRICE_GET_BLOCK_THRESHOLDS_LEN];
  uint8_t *pBuf = cmdBuf;

  pBuf = OsalPort_bufferUint32( pBuf, pCmd->issuerTariffID );

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_PRICE,
                            COMMAND_SE_PRICE_GET_BLOCK_THRESHOLDS, TRUE,
                            ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0,
                            seqNum, ZCL_SE_PRICE_GET_BLOCK_THRESHOLDS_LEN, cmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PriceSendGetCO2Value
 *
 * @brief   Send COMMAND_SE_PRICE_GET_CO2_VALUE.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PriceSendGetCO2Value( uint8_t srcEP, afAddrType_t *dstAddr,
                                      zclSE_PriceGetCO2Value_t *pCmd,
                                      uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t cmdBuf[ZCL_SE_PRICE_GET_CO2_VALUE_LEN];
  uint8_t *pBuf = cmdBuf;

  pBuf = OsalPort_bufferUint32( pBuf, pCmd->earliestStartTime );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->minIssuerEvtID );
  *pBuf++ = pCmd->numOfCmds;
  *pBuf++ = pCmd->tariffType;

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_PRICE,
                            COMMAND_SE_PRICE_GET_CO2_VALUE, TRUE,
                            ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0,
                            seqNum, ZCL_SE_PRICE_GET_CO2_VALUE_LEN, cmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PriceSendGetTierLabels
 *
 * @brief   Send COMMAND_SE_PRICE_GET_TIER_LABELS.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PriceSendGetTierLabels( uint8_t srcEP, afAddrType_t *dstAddr,
                                        zclSE_PriceGetTierLabels_t *pCmd,
                                        uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t cmdBuf[ZCL_SE_PRICE_GET_TIER_LABELS_LEN];
  uint8_t *pBuf = cmdBuf;

  pBuf = OsalPort_bufferUint32( pBuf, pCmd->issuerTariffID );

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_PRICE,
                            COMMAND_SE_PRICE_GET_TIER_LABELS, TRUE,
                            ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0,
                            seqNum, ZCL_SE_PRICE_GET_TIER_LABELS_LEN, cmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PriceSendGetBillPeriod
 *
 * @brief   Send COMMAND_SE_PRICE_GET_BILL_PERIOD.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PriceSendGetBillPeriod( uint8_t srcEP, afAddrType_t *dstAddr,
                                        zclSE_PriceGetBillPeriod_t *pCmd,
                                        uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t cmdBuf[ZCL_SE_PRICE_GET_BILL_PERIOD_LEN];
  uint8_t *pBuf = cmdBuf;

  pBuf = OsalPort_bufferUint32( pBuf, pCmd->earliestStartTime );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->minIssuerEvtID );
  *pBuf++ = pCmd->numOfCmds;
  *pBuf++ = pCmd->tariffType;

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_PRICE,
                            COMMAND_SE_PRICE_GET_BILL_PERIOD, TRUE,
                            ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0,
                            seqNum, ZCL_SE_PRICE_GET_BILL_PERIOD_LEN, cmdBuf);

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PriceSendGetConsolidatedBill
 *
 * @brief   Send COMMAND_SE_PRICE_GET_CONSOLIDATED_BILL.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PriceSendGetConsolidatedBill(
            uint8_t srcEP, afAddrType_t *dstAddr,
            zclSE_PriceGetConsolidatedBill_t *pCmd,
            uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t cmdBuf[ZCL_SE_PRICE_GET_CONSOLIDATED_BILL_LEN];
  uint8_t *pBuf = cmdBuf;

  pBuf = OsalPort_bufferUint32( pBuf, pCmd->earliestStartTime );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->minIssuerEvtID );
  *pBuf++ = pCmd->numOfCmds;
  *pBuf++ = pCmd->tariffType;

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_PRICE,
                            COMMAND_SE_PRICE_GET_CONSOLIDATED_BILL, TRUE,
                            ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0,
                            seqNum, ZCL_SE_PRICE_GET_CONSOLIDATED_BILL_LEN, cmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PriceSendCPPEvtRsp
 *
 * @brief   Send COMMAND_SE_PRICE_CPP_EVT_RSP.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PriceSendCPPEvtRsp( uint8_t srcEP, afAddrType_t *dstAddr,
                                    zclSE_PriceCPPEvtRsp_t *pCmd,
                                    uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t cmdBuf[ZCL_SE_PRICE_CPP_EVT_RSP_LEN];
  uint8_t *pBuf = cmdBuf;

  pBuf = OsalPort_bufferUint32( pBuf, pCmd->issuerEvtID );
  *pBuf++ = pCmd->cppAuth;

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_PRICE,
                            COMMAND_SE_PRICE_CPP_EVT_RSP, TRUE,
                            ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0,
                            seqNum, ZCL_SE_PRICE_CPP_EVT_RSP_LEN, cmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PriceSendGetCreditPayment
 *
 * @brief   Send COMMAND_SE_PRICE_GET_CREDIT_PAYMENT.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PriceSendGetCreditPayment( uint8_t srcEP, afAddrType_t *dstAddr,
                                           zclSE_PriceGetCreditPayment_t *pCmd,
                                           uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t cmdBuf[ZCL_SE_PRICE_GET_CREDIT_PAYMENT_LEN];
  uint8_t *pBuf = cmdBuf;

  pBuf = OsalPort_bufferUint32( pBuf, pCmd->latestEndTime );
  *pBuf++ = pCmd->numOfRecords;

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_PRICE,
                            COMMAND_SE_PRICE_GET_CREDIT_PAYMENT, TRUE,
                            ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0,
                            seqNum, ZCL_SE_PRICE_GET_CREDIT_PAYMENT_LEN, cmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PriceParsePublishPrice
 *
 * @brief   Parse COMMAND_SE_PRICE_PUBLISH_PRICE.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PriceParsePublishPrice( zclIncoming_t *pInMsg,
                                        zclSE_PricePublishPrice_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;
  uint8_t *pBufEnd = pInMsg->pData + pInMsg->pDataLen;
  uint8_t exceeded;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_PRICE_PUBLISH_PRICE_SEG1_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->providerID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pBuf = zclSE_UTF8StringParse( pBuf, pBufEnd, &pCmd->rateLabel, &exceeded );

  // Check for string length exceeded flag and old command length
  if ( exceeded ||
       ( pInMsg->pDataLen < ( pCmd->rateLabel.strLen +
                              ZCL_SE_PRICE_PUBLISH_PRICE_OLD_LEN ) ) )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->issuerEvtID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->currTime = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->uom = *pBuf++;
  pCmd->currency = BUILD_UINT16( pBuf[0], pBuf[1] );
  pBuf += 2;
  pCmd->priceTrailingDigit = *pBuf >> 4;
  pCmd->priceTier = *pBuf++ & 0x0F;
  pCmd->numOfPriceTiers = *pBuf >> 4;
  pCmd->regTier = *pBuf++ & 0x0F;
  pCmd->startTime = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->duration = BUILD_UINT16( pBuf[0], pBuf[1] );
  pBuf += 2;
  pCmd->price = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->priceRatio = *pBuf++;
  pCmd->generationPrice = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->generationPriceRatio = *pBuf++;
  pCmd->altCostDlvd = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->altCostUnit = *pBuf++;
  pCmd->altCostTrailingDigit = *pBuf++;
  pCmd->numOfBlockThresholds = *pBuf++;
  pCmd->priceCtrl = *pBuf++;

  // Check for older command length
  if ( pInMsg->pDataLen < ( pCmd->rateLabel.strLen +
                            ZCL_SE_PRICE_PUBLISH_PRICE_LEN ) )
  {
    // Set default values
    pCmd->numOfGenerationTiers = 0;
    pCmd->generationTier = 0;
    pCmd->extNumOfPriceTiers = 0;
    pCmd->extPriceTier = 0;
    pCmd->extRegTier = 0;
  }
  else
  {
    // Parse rest of command
    pCmd->numOfGenerationTiers = *pBuf++;
    pCmd->generationTier = *pBuf++;
    pCmd->extNumOfPriceTiers = *pBuf++;
    pCmd->extPriceTier = *pBuf++;
    pCmd->extRegTier = *pBuf;
  }

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_PriceParsePublishBlockPeriod
 *
 * @brief   Parse COMMAND_SE_PRICE_PUBLISH_BLOCK_PERIOD.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PriceParsePublishBlockPeriod(
            zclIncoming_t *pInMsg,
            zclSE_PricePublishBlockPeriod_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length using older command length
  if ( pInMsg->pDataLen < ZCL_SE_PRICE_GET_BLOCK_PERIOD_OLD_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->providerID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->issuerEvtID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->blockPeriodStartTime = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->blockPeriodDuration = OsalPort_buildUint32( pBuf, 3 );
  pBuf += 3;

  // Check for older command length
  if ( pInMsg->pDataLen < ZCL_SE_PRICE_PUBLISH_BLOCK_PERIOD_LEN )
  {
    pBuf++; // Skip past obsolete field
    pCmd->blockPeriodCtrl = *pBuf++;

    // Set default values
    pCmd->blockPeriodDurationCtrl = 0;
    pCmd->blockPeriodDurationTimeBase = 2;
    pCmd->tariffType = 1;
    pCmd->tariffResolutionPeriod = 0;
  }
  else
  {
    pCmd->blockPeriodCtrl = *pBuf++;
    pCmd->blockPeriodDurationCtrl = *pBuf >> 4;
    pCmd->blockPeriodDurationTimeBase = *pBuf++ & 0x0F;
    pCmd->tariffType = *pBuf++;
    pCmd->tariffResolutionPeriod = *pBuf++;
  }

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_PriceParsePublishConversionFactor
 *
 * @brief   Parse COMMAND_SE_PRICE_PUBLISH_CONVERSION_FACTOR.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PriceParsePublishConversionFactor(
            zclIncoming_t *pInMsg,
            zclSE_PricePublishConversionFactor_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_PRICE_PUBLISH_CONVERSION_FACTOR_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->issuerEvtID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->startTime = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->conversionFactor = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->conversionFactorTrailingDigit = *pBuf++;

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_PriceParsePublishCalorificValue
 *
 * @brief   Parse COMMAND_SE_PRICE_PUBLISH_CALORIFIC_VALUE.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PriceParsePublishCalorificValue(
            zclIncoming_t *pInMsg,
            zclSE_PricePublishCalorificValue_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_PRICE_PUBLISH_CALORIFIC_VALUE_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->issuerEvtID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->startTime = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->calorificValue = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->calorificValueUnit = *pBuf++;
  pCmd->calorificValueTrailingDigit = *pBuf++;

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_PriceParsePublishTariffInfo
 *
 * @brief   Parse COMMAND_SE_PRICE_PUBLISH_TARIFF_INFO.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PriceParsePublishTariffInfo(
            zclIncoming_t *pInMsg,
            zclSE_PricePublishTariffInfo_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;
  uint8_t *pBufEnd = pInMsg->pData + pInMsg->pDataLen;
  uint8_t exceeded;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_PRICE_PUBLISH_TARIFF_INFO_SEG1_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  // Parse the command buffer
  pCmd->providerID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->issuerEvtID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->issuerTariffID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->startTime = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->chargingScheme = *pBuf >> 4;
  pCmd->tariffType  = *pBuf++ & 0x0F;
  pBuf = zclSE_UTF8StringParse( pBuf, pBufEnd, &pCmd->tariffLabel, &exceeded );

  // Check for string length exceeded flag and minimum packet length
  if ( exceeded || ( ( pBufEnd - pBuf                            ) <
                     ( ZCL_SE_PRICE_PUBLISH_TARIFF_INFO_SEG2_LEN )   ) )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->priceTiersInUse = *pBuf++;
  pCmd->blockThresholdsInUse = *pBuf++;
  pCmd->uom = *pBuf++;
  pCmd->currency = BUILD_UINT16( pBuf[0], pBuf[1] );
  pBuf += 2;
  pCmd->priceTrailingDigit = *pBuf++;
  pCmd->standingCharge = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->tierBlockMode = *pBuf++;
  pCmd->blockThresholdMult = OsalPort_buildUint32( pBuf, 3 );
  pBuf += 3;
  pCmd->blockThresholdDiv = OsalPort_buildUint32( pBuf, 3 );
  pBuf += 3;

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_PriceParsePublishPriceMatrix
 *
 * @brief   Parse COMMAND_SE_PRICE_PUBLISH_PRICE_MATRIX.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PriceParsePublishPriceMatrix(
            zclIncoming_t *pInMsg,
            zclSE_PricePublishPriceMatrix_t *pCmd )
{
  ZStatus_t status = ZCL_STATUS_SUCCESS;
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_PRICE_PUBLISH_PRICE_MATRIX_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->providerID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->issuerEvtID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->startTime = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->issuerTariffID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->cmdIdx = *pBuf++;
  pCmd->cmdTotal = *pBuf++;
  pCmd->subFldCtrl = *pBuf++;
  pCmd->rawPayloadLen = pInMsg->pDataLen - ZCL_SE_PRICE_PUBLISH_PRICE_MATRIX_LEN;

  if ( pCmd->rawPayloadLen )
  {
    pCmd->pRawPayload = pBuf;
  }
  else
  {
    pCmd->pRawPayload = NULL;
  }

  // Check for fragmentation
  if ( pCmd->cmdTotal <= 1 )
  {
    status = zclSE_PriceMatrixSubFldParse( pCmd );
  }
  else
  {
    pCmd->pSubFlds = NULL;
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PriceParsePublishBlockThreshold
 *
 * @brief   Parse COMMAND_SE_PRICE_PUBLISH_BLOCK_THRESHOLD.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PriceParsePublishBlockThreshold(
            zclIncoming_t *pInMsg,
            zclSE_PricePublishBlockThreshold_t *pCmd )
{
  ZStatus_t status = ZCL_STATUS_SUCCESS;
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_PRICE_PUBLISH_BLOCK_THRESHOLD_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  // Parse the command buffer
  pCmd->providerID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->issuerEvtID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->startTime = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->issuerTariffID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->cmdIdx = *pBuf++;
  pCmd->cmdTotal = *pBuf++;
  pCmd->subFldCtrl = *pBuf++;
  pCmd->rawPayloadLen = pInMsg->pDataLen - ZCL_SE_PRICE_PUBLISH_BLOCK_THRESHOLD_LEN;

  if ( pCmd->rawPayloadLen )
  {
    pCmd->pRawPayload = pBuf;
  }
  else
  {
    pCmd->pRawPayload = NULL;
  }

  // Check for fragmentation
  if ( pCmd->cmdTotal <= 1 )
  {
    status = zclSE_PriceBlockThresholdSubFldParse( pCmd );
  }
  else
  {
    pCmd->pSubFlds = NULL;
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PriceParsePublishCO2Value
 *
 * @brief   Parse COMMAND_SE_PRICE_PUBLISH_CO2_VALUE
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PriceParsePublishCO2Value( zclIncoming_t *pInMsg,
                                           zclSE_PricePublishCO2Value_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_PRICE_PUBLISH_CO2_VALUE_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->providerID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->issuerEvtID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->startTime = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->tariffType = *pBuf++;
  pCmd->co2Value = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->co2ValueUnit = *pBuf++;
  pCmd->co2ValueTrailingDigit = *pBuf++;

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_PriceParsePublishTierLabels
 *
 * @brief   Parse COMMAND_SE_PRICE_PUBLISH_TIER_LABELS.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PriceParsePublishTierLabels(
            zclIncoming_t *pInMsg,
            zclSE_PricePublishTierLabels_t *pCmd )
{
  ZStatus_t status = ZCL_STATUS_SUCCESS;
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_PRICE_PUBLISH_TIER_LABELS_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  // Parse the command buffer
  pCmd->providerID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->issuerEvtID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->issuerTariffID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->cmdIdx = *pBuf++;
  pCmd->cmdTotal = *pBuf++;
  pCmd->numOfTierLabels = *pBuf++;
  pCmd->rawPayloadLen = pInMsg->pDataLen - ZCL_SE_PRICE_PUBLISH_TIER_LABELS_LEN;

  if ( pCmd->rawPayloadLen )
  {
    pCmd->pRawPayload = pBuf;
  }
  else
  {
    pCmd->pRawPayload = NULL;
  }

  // Check for fragmentation
  if ( pCmd->cmdTotal <= 1 )
  {
    status = zclSE_PriceTierLabelParse( pCmd );
  }
  else
  {
    pCmd->pTierLabels = NULL;
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PriceParsePublishBillPeriod
 *
 * @brief   Parse COMMAND_SE_PRICE_PUBLISH_BILL_PERIOD.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PriceParsePublishBillPeriod(
            zclIncoming_t *pInMsg,
            zclSE_PricePublishBillPeriod_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_PRICE_PUBLISH_BILL_PERIOD_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->providerID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->issuerEvtID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->billPeriodStartTime = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->billPeriodDuration = OsalPort_buildUint32( pBuf, 3 );
  pBuf += 3;
  pCmd->billPeriodDurationTimeCtrl = *pBuf >> 4;
  pCmd->billPeriodDurationTimeBase = *pBuf++ & 0x0F;
  pCmd->tariffType = *pBuf++;

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_PriceParsePublishConsolidatedBill
 *
 * @brief   Parse COMMAND_SE_PRICE_PUBLISH_CONSOLIDATED_BILL.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PriceParsePublishConsolidatedBill(
            zclIncoming_t *pInMsg,
            zclSE_PricePublishConsolidatedBill_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_PRICE_PUBLISH_CONSOLIDATED_BILL_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  // Parse the command buffer
  pCmd->providerID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->issuerEvtID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->billPeriodStartTime = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->billPeriodDuration = OsalPort_buildUint32( pBuf, 3 );
  pBuf += 3;
  pCmd->billPeriodDurationTimeCtrl = *pBuf >> 4;
  pCmd->billPeriodDurationTimeBase = *pBuf++ & 0x0F;
  pCmd->tariffType = *pBuf++;
  pCmd->consolidatedBill = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->currency = BUILD_UINT16( pBuf[0], pBuf[1] );
  pBuf += 2;
  pCmd->billTrailingDigit = *pBuf++;

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_PriceParsePublishCPPEvt
 *
 * @brief   Parse COMMAND_SE_PRICE_PUBLISH_CPP_EVT.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PriceParsePublishCPPEvt( zclIncoming_t *pInMsg,
                                         zclSE_PricePublishCPPEvt_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_PRICE_PUBLISH_CPP_EVT_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->providerID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->issuerEvtID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->startTime = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->duration = BUILD_UINT16( pBuf[0], pBuf[1] );
  pBuf += 2;
  pCmd->tariffType = *pBuf++;
  pCmd->cppPriceTier = *pBuf++;
  pCmd->cppAuth = *pBuf++;

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_PriceParsePublishCreditPayment
 *
 * @brief   Parse COMMAND_SE_PRICE_PUBLISH_CREDIT_PAYMENT.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PriceParsePublishCreditPayment(
            zclIncoming_t *pInMsg,
            zclSE_PricePublishCreditPayment_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;
  uint8_t *pBufEnd = pInMsg->pData + pInMsg->pDataLen;
  uint8_t exceeded;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_PRICE_PUBLISH_CREDIT_PAYMENT_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->providerID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->issuerEvtID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->creditPaymentDueDate = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->creditPaymentOverdueAmnt = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->creditPaymentStatus = *pBuf++;
  pCmd->creditPayment = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->creditPaymentDate = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pBuf = zclSE_UTF8StringParse( pBuf, pBufEnd, &pCmd->creditPaymentRef, &exceeded );

  // Check for string length exceeded flag
  if ( exceeded )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_PriceParsePublishCurrencyConversion
 *
 * @brief   Parse COMMAND_SE_PRICE_PUBLISH_CURRENCY_CONVERSION.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PriceParsePublishCurrencyConversion(
            zclIncoming_t *pInMsg,
            zclSE_PricePublishCurrencyConversion_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_PRICE_PUBLISH_CURRENCY_CONVERSION_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  // Parse the command buffer
  pCmd->providerID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->issuerEvtID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->startTime = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->oldCurrency = BUILD_UINT16( pBuf[0], pBuf[1] );
  pBuf += 2;
  pCmd->newCurrency = BUILD_UINT16( pBuf[0], pBuf[1] );
  pBuf += 2;
  pCmd->conversionFactor = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->conversionFactorTrailingDigit = *pBuf++;
  pCmd->currencyChangeCtrlFlag = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_PriceParseCancelTariff
 *
 * @brief   Parse COMMAND_SE_PRICE_CANCEL_TARIFF.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PriceParseCancelTariff( zclIncoming_t *pInMsg,
                                        zclSE_PriceCancelTariff_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_PRICE_CANCEL_TARIFF_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->providerID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->issuerTariffID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->tariffType = *pBuf++;

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_PriceParseGetCurrPrice
 *
 * @brief   Parse COMMAND_SE_PRICE_GET_CURR_PRICE.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PriceParseGetCurrPrice( zclIncoming_t *pInMsg,
                                        zclSE_PriceGetCurrPrice_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_PRICE_GET_CURR_PRICE_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->options = *pBuf++;

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_PriceParseGetScheduledPrice
 *
 * @brief   Parse COMMAND_SE_PRICE_GET_SCHEDULED_PRICE.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PriceParseGetScheduledPrice(
            zclIncoming_t *pInMsg,
            zclSE_PriceGetScheduledPrice_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_PRICE_GET_SCHEDULED_PRICE_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->startTime = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->numOfEvts = *pBuf++;

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_PriceParsePriceAck
 *
 * @brief   Parse COMMAND_SE_PRICE_PRICE_ACK.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PriceParsePriceAck( zclIncoming_t *pInMsg,
                                    zclSE_PricePriceAck_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_PRICE_PRICE_ACK_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->providerID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->issuerEvtID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->priceAckTime = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->ctrl = *pBuf++;

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_PriceParseGetBlockPeriod
 *
 * @brief   Parse COMMAND_SE_PRICE_GET_BLOCK_PERIOD.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PriceParseGetBlockPeriod( zclIncoming_t *pInMsg,
                                          zclSE_PriceGetBlockPeriod_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length using older command length
  if ( pInMsg->pDataLen < ZCL_SE_PRICE_GET_BLOCK_PERIOD_OLD_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->startTime = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->numOfEvts = *pBuf++;

  // Check for older command length
  if ( pInMsg->pDataLen < ZCL_SE_PRICE_GET_BLOCK_PERIOD_LEN )
  {
    pCmd->tariffType = 0;
  }
  else
  {
    pCmd->tariffType = *pBuf++;
  }

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_PriceParseGetConversionFactor
 *
 * @brief   Parse COMMAND_SE_PRICE_GET_CONVERSION_FACTOR.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PriceParseGetConversionFactor(
            zclIncoming_t *pInMsg,
            zclSE_PriceGetConversionFactor_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length using older command length
  if ( pInMsg->pDataLen < ZCL_SE_PRICE_GET_CONVERSION_FACTOR_OLD_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->earliestStartTime = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->minIssuerEvtID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;

  // Check for older command length
  if ( pInMsg->pDataLen < ZCL_SE_PRICE_GET_CONVERSION_FACTOR_LEN )
  {
    pCmd->numOfCmds = (uint8_t)pCmd->minIssuerEvtID;
    pCmd->minIssuerEvtID = 0;
  }
  else
  {
    pCmd->numOfCmds = *pBuf++;
  }

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_PriceParseGetCalorificValue
 *
 * @brief   Parse COMMAND_SE_PRICE_GET_CALORIFIC_VALUE.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PriceParseGetCalorificValue(
            zclIncoming_t *pInMsg,
            zclSE_PriceGetCalorificValue_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length using older command length
  if ( pInMsg->pDataLen < ZCL_SE_PRICE_GET_CALORIFIC_VALUE_OLD_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->earliestStartTime = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;

  // Check for older command length
  if ( pInMsg->pDataLen < ZCL_SE_PRICE_GET_CALORIFIC_VALUE_LEN )
  {
    pCmd->minIssuerEvtID = 0;
    pCmd->numOfCmds = *pBuf++;
  }
  else
  {
    pCmd->minIssuerEvtID = OsalPort_buildUint32( pBuf, 4 );
    pBuf += 4;
    pCmd->numOfCmds = *pBuf++;
  }

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_PriceParseGetTariffInfo
 *
 * @brief   Parse COMMAND_SE_PRICE_GET_TARIFF_INFO.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PriceParseGetTariffInfo( zclIncoming_t *pInMsg,
                                         zclSE_PriceGetTariffInfo_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_PRICE_GET_TARIFF_INFO_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->earliestStartTime = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->minIssuerEvtID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->numOfCmds = *pBuf++;
  pCmd->tariffType = *pBuf++;

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_PriceParseGetPriceMatrix
 *
 * @brief   Parse COMMAND_SE_PRICE_GET_PRICE_MATRIX.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PriceParseGetPriceMatrix( zclIncoming_t *pInMsg,
                                          zclSE_PriceGetPriceMatrix_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_PRICE_GET_PRICE_MATRIX_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->issuerTariffID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_PriceParseGetBlockThresholds
 *
 * @brief   Parse COMMAND_SE_PRICE_GET_BLOCK_THRESHOLDS.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PriceParseGetBlockThresholds(
            zclIncoming_t *pInMsg,
            zclSE_PriceGetBlockThresholds_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_PRICE_GET_BLOCK_THRESHOLDS_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->issuerTariffID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_PriceParseGetCO2Value
 *
 * @brief   Parse COMMAND_SE_PRICE_GET_CO2_VALUE.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PriceParseGetCO2Value( zclIncoming_t *pInMsg,
                                       zclSE_PriceGetCO2Value_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_PRICE_GET_CO2_VALUE_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->earliestStartTime = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->minIssuerEvtID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->numOfCmds = *pBuf++;
  pCmd->tariffType = *pBuf++;

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_PriceParseGetTierLabels
 *
 * @brief   Parse COMMAND_SE_PRICE_GET_TIER_LABELS.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PriceParseGetTierLabels( zclIncoming_t *pInMsg,
                                         zclSE_PriceGetTierLabels_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_PRICE_GET_TIER_LABELS_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->issuerTariffID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_PriceParseGetBillPeriod
 *
 * @brief   Parse COMMAND_SE_PRICE_GET_BILL_PERIOD.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PriceParseGetBillPeriod( zclIncoming_t *pInMsg,
                                         zclSE_PriceGetBillPeriod_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_PRICE_GET_BILL_PERIOD_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->earliestStartTime = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->minIssuerEvtID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->numOfCmds = *pBuf++;
  pCmd->tariffType = *pBuf++;

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_PriceParseGetConsolidatedBill
 *
 * @brief   Parse COMMAND_SE_PRICE_GET_CONSOLIDATED_BILL.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PriceParseGetConsolidatedBill(
            zclIncoming_t *pInMsg,
            zclSE_PriceGetConsolidatedBill_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_PRICE_GET_CONSOLIDATED_BILL_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->earliestStartTime = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->minIssuerEvtID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->numOfCmds = *pBuf++;
  pCmd->tariffType = *pBuf++;

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_PriceParseCPPEvtRsp
 *
 * @brief   Parse COMMAND_SE_PRICE_CPP_EVT_RSP.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PriceParseCPPEvtRsp( zclIncoming_t *pInMsg,
                                     zclSE_PriceCPPEvtRsp_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_PRICE_CPP_EVT_RSP_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->issuerEvtID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->cppAuth = *pBuf++;

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_PriceParseGetCreditPayment
 *
 * @brief   Parse COMMAND_SE_PRICE_GET_CREDIT_PAYMENT.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PriceParseGetCreditPayment( zclIncoming_t *pInMsg,
                                            zclSE_PriceGetCreditPayment_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_PRICE_GET_CREDIT_PAYMENT_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->latestEndTime = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->numOfRecords = *pBuf++;

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_PriceHdlPublishPrice
 *
 * @brief   Handle COMMAND_SE_PRICE_PUBLISH_PRICE.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PriceHdlPublishPrice( zclIncoming_t *pInMsg,
                                      zclSE_PricePublishPriceCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_PricePublishPrice_t *pCmd;

    pCmd = OsalPort_malloc( sizeof( zclSE_PricePublishPrice_t ) );
    if ( pCmd == NULL )
    {
       return ZCL_STATUS_SOFTWARE_FAILURE;
    }

    status = zclSE_PriceParsePublishPrice( pInMsg, pCmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      pfnCB( pInMsg, pCmd );
    }

    OsalPort_free( pCmd );
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PriceHdlPublishBlockPeriod
 *
 * @brief   Handle COMMAND_SE_PRICE_PUBLISH_BLOCK_PERIOD.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PriceHdlPublishBlockPeriod( zclIncoming_t *pInMsg,
                                            zclSE_PricePublishBlockPeriodCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_PricePublishBlockPeriod_t *pCmd;

    pCmd = OsalPort_malloc( sizeof( zclSE_PricePublishBlockPeriod_t ) );
    if ( pCmd == NULL )
    {
       return ZCL_STATUS_SOFTWARE_FAILURE;
    }

    status = zclSE_PriceParsePublishBlockPeriod( pInMsg, pCmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      pfnCB( pInMsg, pCmd );
    }

    OsalPort_free( pCmd );
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PriceHdlPublishConversionFactor
 *
 * @brief   Handle COMMAND_SE_PRICE_PUBLISH_CONVERSION_FACTOR.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PriceHdlPublishConversionFactor(
            zclIncoming_t *pInMsg,
            zclSE_PricePublishConversionFactorCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_PricePublishConversionFactor_t cmd;

    status = zclSE_PriceParsePublishConversionFactor( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      pfnCB( pInMsg, &cmd );
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PriceHdlPublishCalorificValue
 *
 * @brief   Handle COMMAND_SE_PRICE_PUBLISH_CALORIFIC_VALUE.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PriceHdlPublishCalorificValue(
            zclIncoming_t *pInMsg,
            zclSE_PricePublishCalorificValueCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_PricePublishCalorificValue_t cmd;

    status = zclSE_PriceParsePublishCalorificValue( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      pfnCB( pInMsg, &cmd );
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PriceHdlPublishTariffInfo
 *
 * @brief   Handle COMMAND_SE_PRICE_PUBLISH_TARIFF_INFO.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PriceHdlPublishTariffInfo( zclIncoming_t *pInMsg,
                                           zclSE_PricePublishTariffInfoCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_PricePublishTariffInfo_t *pCmd;

    pCmd = OsalPort_malloc( sizeof( zclSE_PricePublishTariffInfo_t ) );
    if ( pCmd == NULL )
    {
       return ZCL_STATUS_SOFTWARE_FAILURE;
    }

    status = zclSE_PriceParsePublishTariffInfo( pInMsg, pCmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      pfnCB( pInMsg, pCmd );
    }

    OsalPort_free( pCmd );
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PriceHdlPublishPriceMatrix
 *
 * @brief   Handle COMMAND_SE_PRICE_PUBLISH_PRICE_MATRIX.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PriceHdlPublishPriceMatrix( zclIncoming_t *pInMsg,
                                            zclSE_PricePublishPriceMatrixCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_PricePublishPriceMatrix_t cmd;

    status = zclSE_PriceParsePublishPriceMatrix( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      status = pfnCB( pInMsg, &cmd );

      if ( cmd.pSubFlds != NULL )
      {
        OsalPort_free( cmd.pSubFlds );
      }
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PriceHdlPublishBlockThreshold
 *
 * @brief   Handle COMMAND_SE_PRICE_PUBLISH_BLOCK_THRESHOLD.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PriceHdlPublishBlockThreshold(
            zclIncoming_t *pInMsg,
            zclSE_PricePublishBlockThresholdCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_PricePublishBlockThreshold_t cmd;

    status = zclSE_PriceParsePublishBlockThreshold( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      pfnCB( pInMsg, &cmd );

      if ( cmd.pSubFlds != NULL )
      {
        OsalPort_free( cmd.pSubFlds );
      }
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PriceHdlPublishCO2Value
 *
 * @brief   Handle COMMAND_SE_PRICE_PUBLISH_CO2_VALUE.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PriceHdlPublishCO2Value( zclIncoming_t *pInMsg,
                                         zclSE_PricePublishCO2ValueCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_PricePublishCO2Value_t *pCmd;

    pCmd = OsalPort_malloc( sizeof( zclSE_PricePublishCO2Value_t ) );
    if ( pCmd == NULL )
    {
       return ZCL_STATUS_SOFTWARE_FAILURE;
    }

    status = zclSE_PriceParsePublishCO2Value( pInMsg, pCmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      pfnCB( pInMsg, pCmd );
    }

    OsalPort_free( pCmd );
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PriceHdlPublishTierLabels
 *
 * @brief   Handle COMMAND_SE_PRICE_PUBLISH_TIER_LABELS.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PriceHdlPublishTierLabels( zclIncoming_t *pInMsg,
                                           zclSE_PricePublishTierLabelsCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_PricePublishTierLabels_t cmd;

    status = zclSE_PriceParsePublishTierLabels( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      pfnCB( pInMsg, &cmd );

      if ( cmd.pTierLabels != NULL )
      {
        OsalPort_free( cmd.pTierLabels );
      }
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PriceHdlPublishBillPeriod
 *
 * @brief   Handle COMMAND_SE_PRICE_PUBLISH_BILL_PERIOD.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PriceHdlPublishBillPeriod(
            zclIncoming_t *pInMsg,
            zclSE_PricePublishBillPeriodCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_PricePublishBillPeriod_t cmd;

    status = zclSE_PriceParsePublishBillPeriod( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      pfnCB( pInMsg, &cmd );
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PriceHdlPublishConsolidatedBill
 *
 * @brief   Handle COMMAND_SE_PRICE_PUBLISH_CONSOLIDATED_BILL.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PriceHdlPublishConsolidatedBill(
            zclIncoming_t *pInMsg,
            zclSE_PricePublishConsolidatedBillCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_PricePublishConsolidatedBill_t *pCmd;

    pCmd = OsalPort_malloc( sizeof( zclSE_PricePublishConsolidatedBill_t ) );
    if ( pCmd == NULL )
    {
       return ZCL_STATUS_SOFTWARE_FAILURE;
    }

    status = zclSE_PriceParsePublishConsolidatedBill( pInMsg, pCmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      pfnCB( pInMsg, pCmd );
    }

    OsalPort_free( pCmd );
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PriceHdlPublishCPPEvt
 *
 * @brief   Handle COMMAND_SE_PRICE_PUBLISH_CPP_EVT.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PriceHdlPublishCPPEvt( zclIncoming_t *pInMsg,
                                       zclSE_PricePublishCPPEvtCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_PricePublishCPPEvt_t cmd;

    status = zclSE_PriceParsePublishCPPEvt( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      pfnCB( pInMsg, &cmd );
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PriceHdlPublishCreditPayment
 *
 * @brief   Handle COMMAND_SE_PRICE_PUBLISH_CREDIT_PAYMENT.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PriceHdlPublishCreditPayment(
            zclIncoming_t *pInMsg,
            zclSE_PricePublishCreditPaymentCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_PricePublishCreditPayment_t *pCmd;

    pCmd = OsalPort_malloc( sizeof( zclSE_PricePublishCreditPayment_t ) );
    if ( pCmd == NULL )
    {
       return ZCL_STATUS_SOFTWARE_FAILURE;
    }

    status = zclSE_PriceParsePublishCreditPayment( pInMsg, pCmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      pfnCB( pInMsg, pCmd );
    }

    OsalPort_free( pCmd );
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PriceHdlPublishCurrencyConversion
 *
 * @brief   Handle COMMAND_SE_PRICE_PUBLISH_CURRENCY_CONVERSION.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PriceHdlPublishCurrencyConversion(
            zclIncoming_t *pInMsg,
            zclSE_PricePublishCurrencyConversionCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_PricePublishCurrencyConversion_t *pCmd;

    pCmd = OsalPort_malloc( sizeof( zclSE_PricePublishCurrencyConversion_t ) );
    if ( pCmd == NULL )
    {
       return ZCL_STATUS_SOFTWARE_FAILURE;
    }

    status = zclSE_PriceParsePublishCurrencyConversion( pInMsg, pCmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      pfnCB( pInMsg, pCmd );
    }

    OsalPort_free( pCmd );
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PriceHdlCancelTariff
 *
 * @brief   Handle COMMAND_SE_PRICE_CANCEL_TARIFF.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PriceHdlCancelTariff( zclIncoming_t *pInMsg,
                                      zclSE_PriceCancelTariffCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_PriceCancelTariff_t cmd;

    status = zclSE_PriceParseCancelTariff( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      pfnCB( pInMsg, &cmd );
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PriceHdlGetCurrPrice
 *
 * @brief   Handle COMMAND_SE_PRICE_GET_CURR_PRICE.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PriceHdlGetCurrPrice( zclIncoming_t *pInMsg,
                                      zclSE_PriceGetCurrPriceCB_t pfnCB )
{

  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_PriceGetCurrPrice_t cmd;

    status = zclSE_PriceParseGetCurrPrice( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      status = pfnCB( pInMsg, &cmd );
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PriceHdlGetScheduledPrice
 *
 * @brief   Handle COMMAND_SE_PRICE_GET_SCHEDULED_PRICE.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PriceHdlGetScheduledPrice( zclIncoming_t *pInMsg,
                                           zclSE_PriceGetScheduledPriceCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_PriceGetScheduledPrice_t cmd;

    status = zclSE_PriceParseGetScheduledPrice( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      status = pfnCB( pInMsg, &cmd );
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PriceHdlPriceAck
 *
 * @brief   Handle COMMAND_SE_PRICE_PRICE_ACK.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PriceHdlPriceAck( zclIncoming_t *pInMsg,
                                  zclSE_PricePriceAckCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_PricePriceAck_t cmd;

    status = zclSE_PriceParsePriceAck( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      pfnCB( pInMsg, &cmd );
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PriceHdlGetBlockPeriod
 *
 * @brief   Handle COMMAND_SE_PRICE_GET_BLOCK_PERIOD.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PriceHdlGetBlockPeriod( zclIncoming_t *pInMsg,
                                        zclSE_PriceGetBlockPeriodCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_PriceGetBlockPeriod_t cmd;

    status = zclSE_PriceParseGetBlockPeriod( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      status = pfnCB( pInMsg, &cmd );
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PriceHdlGetConversionFactor
 *
 * @brief   Handle COMMAND_SE_PRICE_GET_CONVERSION_FACTOR.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PriceHdlGetConversionFactor(
            zclIncoming_t *pInMsg,
            zclSE_PriceGetConversionFactorCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_PriceGetConversionFactor_t cmd;

    status = zclSE_PriceParseGetConversionFactor( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      status = pfnCB( pInMsg, &cmd );
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PriceHdlGetCalorificValue
 *
 * @brief   Handle COMMAND_SE_PRICE_GET_CALORIFIC_VALUE.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PriceHdlGetCalorificValue( zclIncoming_t *pInMsg,
                                           zclSE_PriceGetCalorificValueCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_PriceGetCalorificValue_t cmd;

    status = zclSE_PriceParseGetCalorificValue( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      status = pfnCB( pInMsg, &cmd );
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PriceHdlGetTariffInfo
 *
 * @brief   Handle COMMAND_SE_PRICE_GET_TARIFF_INFO.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PriceHdlGetTariffInfo( zclIncoming_t *pInMsg,
                                       zclSE_PriceGetTariffInfoCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_PriceGetTariffInfo_t cmd;

    status = zclSE_PriceParseGetTariffInfo( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      status = pfnCB( pInMsg, &cmd );
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PriceHdlGetPriceMatrix
 *
 * @brief   Handle COMMAND_SE_PRICE_GET_PRICE_MATRIX.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PriceHdlGetPriceMatrix( zclIncoming_t *pInMsg,
                                        zclSE_PriceGetPriceMatrixCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_PriceGetPriceMatrix_t cmd;

    status = zclSE_PriceParseGetPriceMatrix( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      status = pfnCB( pInMsg, &cmd );
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PriceHdlGetBlockThresholds
 *
 * @brief   Handle COMMAND_SE_PRICE_GET_BLOCK_THRESHOLDS.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PriceHdlGetBlockThresholds( zclIncoming_t *pInMsg,
                                            zclSE_PriceGetBlockThresholdsCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_PriceGetBlockThresholds_t cmd;

    status = zclSE_PriceParseGetBlockThresholds( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      status = pfnCB( pInMsg, &cmd );
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PriceHdlGetCO2Value
 *
 * @brief   Handle COMMAND_SE_PRICE_GET_CO2_VALUE.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PriceHdlGetCO2Value( zclIncoming_t *pInMsg,
                                     zclSE_PriceGetCO2ValueCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_PriceGetCO2Value_t cmd;

    status = zclSE_PriceParseGetCO2Value( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      status = pfnCB( pInMsg, &cmd );
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PriceHdlGetTierLabels
 *
 * @brief   Handle COMMAND_SE_PRICE_GET_TIER_LABELS.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PriceHdlGetTierLabels( zclIncoming_t *pInMsg,
                                       zclSE_PriceGetTierLabelsCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_PriceGetTierLabels_t cmd;

    status = zclSE_PriceParseGetTierLabels( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      status = pfnCB( pInMsg, &cmd );
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PriceHdlGetBillPeriod
 *
 * @brief   Handle COMMAND_SE_PRICE_GET_BILL_PERIOD.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PriceHdlGetBillPeriod( zclIncoming_t *pInMsg,
                                       zclSE_PriceGetBillPeriodCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_PriceGetBillPeriod_t cmd;

    status = zclSE_PriceParseGetBillPeriod( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      status = pfnCB( pInMsg, &cmd );
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PriceHdlGetConsolidatedBill
 *
 * @brief   Handle COMMAND_SE_PRICE_GET_CONSOLIDATED_BILL.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PriceHdlGetConsolidatedBill(
            zclIncoming_t *pInMsg,
            zclSE_PriceGetConsolidatedBillCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_PriceGetConsolidatedBill_t cmd;

    status = zclSE_PriceParseGetConsolidatedBill( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      status = pfnCB( pInMsg, &cmd );
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PriceHdlCPPEvtRsp
 *
 * @brief   Handle COMMAND_SE_PRICE_CPP_EVT_RSP.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PriceHdlCPPEvtRsp( zclIncoming_t *pInMsg,
                                   zclSE_PriceCPPEvtRspCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_PriceCPPEvtRsp_t cmd;

    status = zclSE_PriceParseCPPEvtRsp( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      pfnCB( pInMsg, &cmd );

      status = ZCL_STATUS_CMD_HAS_RSP;
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PriceHdlGetCreditPayment
 *
 * @brief   Handle COMMAND_SE_PRICE_GET_CREDIT_PAYMENT.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PriceHdlGetCreditPayment( zclIncoming_t *pInMsg,
                                          zclSE_PriceGetCreditPaymentCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_PriceGetCreditPayment_t cmd;

    status = zclSE_PriceParseGetCreditPayment( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      status = pfnCB( pInMsg, &cmd );
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PriceHdlGetCurrencyConversion
 *
 * @brief   Handle COMMAND_SE_PRICE_GET_CURRENCY_CONVERSION (handle with no payload).
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PriceHdlGetCurrencyConversion(
            zclIncoming_t *pInMsg,
            zclSE_PriceGetCurrencyConversionCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
      status = pfnCB( pInMsg );
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PriceHdlGetTariffCancellation
 *
 * @brief   Handle COMMAND_SE_PRICE_GET_TARIFF_CANCELLATION (handle with no payload).
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PriceHdlGetTariffCancellation(
            zclIncoming_t *pInMsg,
            zclSE_PriceGetTariffCancellationCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
      status = pfnCB( pInMsg );
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PriceHdlClientCmd
 *
 * @brief   Client command handler for ZCL_CLUSTER_ID_SE_PRICE.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCBs - client callbacks
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PriceHdlClientCmd( zclIncoming_t *pInMsg,
                                   const zclSE_PriceClientCBs_t *pCBs )
{
  ZStatus_t status;

  // Guard against NULL pointer
  if ( pCBs == NULL )
  {
    return ZCL_STATUS_FAILURE;
  }

  // Process Server-to-Client Commands
  switch ( pInMsg->hdr.commandID )
  {
    case COMMAND_SE_PRICE_PUBLISH_PRICE:
      status = zclSE_PriceHdlPublishPrice( pInMsg, pCBs->pfnPublishPrice );
      break;

    case COMMAND_SE_PRICE_PUBLISH_BLOCK_PERIOD:
      status = zclSE_PriceHdlPublishBlockPeriod( pInMsg, pCBs->pfnPublishBlockPeriod );
      break;

    case COMMAND_SE_PRICE_PUBLISH_CONVERSION_FACTOR:
      status = zclSE_PriceHdlPublishConversionFactor( pInMsg, pCBs->pfnPublishConversionFactor );
      break;

    case COMMAND_SE_PRICE_PUBLISH_CALORIFIC_VALUE:
      status = zclSE_PriceHdlPublishCalorificValue( pInMsg, pCBs->pfnPublishCalorificValue );
      break;

    case COMMAND_SE_PRICE_PUBLISH_TARIFF_INFO:
      status = zclSE_PriceHdlPublishTariffInfo( pInMsg, pCBs->pfnPublishTariffInfo );
      break;

    case COMMAND_SE_PRICE_PUBLISH_PRICE_MATRIX:
      status = zclSE_PriceHdlPublishPriceMatrix( pInMsg, pCBs->pfnPublishPriceMatrix );
      break;

    case COMMAND_SE_PRICE_PUBLISH_BLOCK_THRESHOLD:
      status = zclSE_PriceHdlPublishBlockThreshold( pInMsg, pCBs->pfnPublishBlockThreshold );
      break;

    case COMMAND_SE_PRICE_PUBLISH_CO2_VALUE:
      status = zclSE_PriceHdlPublishCO2Value( pInMsg, pCBs->pfnPublishCO2Value );
      break;

    case COMMAND_SE_PRICE_PUBLISH_TIER_LABELS:
      status = zclSE_PriceHdlPublishTierLabels( pInMsg, pCBs->pfnPublishTierLabels );
      break;

    case COMMAND_SE_PRICE_PUBLISH_BILL_PERIOD:
      status = zclSE_PriceHdlPublishBillPeriod( pInMsg, pCBs->pfnPublishBillPeriod );
      break;

    case COMMAND_SE_PRICE_PUBLISH_CONSOLIDATED_BILL:
      status = zclSE_PriceHdlPublishConsolidatedBill( pInMsg, pCBs->pfnPublishConsolidatedBill );
      break;

    case COMMAND_SE_PRICE_PUBLISH_CPP_EVT:
      status = zclSE_PriceHdlPublishCPPEvt( pInMsg, pCBs->pfnPublishCPPEvt );
      break;

    case COMMAND_SE_PRICE_PUBLISH_CREDIT_PAYMENT:
      status = zclSE_PriceHdlPublishCreditPayment( pInMsg, pCBs->pfnPublishCreditPayment );
      break;

    case COMMAND_SE_PRICE_PUBLISH_CURRENCY_CONVERSION:
      status = zclSE_PriceHdlPublishCurrencyConversion( pInMsg,
                                                        pCBs->pfnPublishCurrencyConversion );
      break;

    case COMMAND_SE_PRICE_CANCEL_TARIFF:
      status = zclSE_PriceHdlCancelTariff( pInMsg, pCBs->pfnCancelTariff );
      break;

    default:
      status = ZCL_STATUS_FAILURE;
      break;
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PriceHdlServerCmd
 *
 * @brief   Server command handler for ZCL_CLUSTER_ID_SE_PRICE.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCBs - server callbacks
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PriceHdlServerCmd( zclIncoming_t *pInMsg,
                                   const zclSE_PriceServerCBs_t *pCBs )
{
  ZStatus_t status;

  // Guard against NULL pointer
  if ( pCBs == NULL )
  {
    return ZCL_STATUS_FAILURE;
  }

  // Process Client-to-Server Commands
  switch ( pInMsg->hdr.commandID )
  {
    case COMMAND_SE_PRICE_GET_CURR_PRICE:
      status = zclSE_PriceHdlGetCurrPrice( pInMsg, pCBs->pfnGetCurrPrice );
      break;

    case COMMAND_SE_PRICE_GET_SCHEDULED_PRICE:
      status = zclSE_PriceHdlGetScheduledPrice( pInMsg, pCBs->pfnGetScheduledPrice );
      break;

    case COMMAND_SE_PRICE_PRICE_ACK:
      status = zclSE_PriceHdlPriceAck( pInMsg, pCBs->pfnPriceAck );
      break;

    case COMMAND_SE_PRICE_GET_BLOCK_PERIOD:
      status = zclSE_PriceHdlGetBlockPeriod( pInMsg, pCBs->pfnGetBlockPeriod );
      break;

    case COMMAND_SE_PRICE_GET_CONVERSION_FACTOR:
      status = zclSE_PriceHdlGetConversionFactor( pInMsg, pCBs->pfnGetConversionFactor );
      break;

    case COMMAND_SE_PRICE_GET_CALORIFIC_VALUE:
      status = zclSE_PriceHdlGetCalorificValue( pInMsg, pCBs->pfnGetCalorificValue );
      break;

    case COMMAND_SE_PRICE_GET_TARIFF_INFO:
      status = zclSE_PriceHdlGetTariffInfo( pInMsg, pCBs->pfnGetTariffInfo );
      break;

    case COMMAND_SE_PRICE_GET_PRICE_MATRIX:
      status = zclSE_PriceHdlGetPriceMatrix( pInMsg, pCBs->pfnGetPriceMatrix );
      break;

    case COMMAND_SE_PRICE_GET_BLOCK_THRESHOLDS:
      status = zclSE_PriceHdlGetBlockThresholds( pInMsg, pCBs->pfnGetBlockThresholds );
      break;

    case COMMAND_SE_PRICE_GET_CO2_VALUE:
      status = zclSE_PriceHdlGetCO2Value( pInMsg, pCBs->pfnGetCO2Value );
      break;

    case COMMAND_SE_PRICE_GET_TIER_LABELS:
      status = zclSE_PriceHdlGetTierLabels( pInMsg, pCBs->pfnGetTierLabels );
      break;

    case COMMAND_SE_PRICE_GET_BILL_PERIOD:
      status = zclSE_PriceHdlGetBillPeriod( pInMsg, pCBs->pfnGetBillPeriod );
      break;

    case COMMAND_SE_PRICE_GET_CONSOLIDATED_BILL:
      status = zclSE_PriceHdlGetConsolidatedBill( pInMsg, pCBs->pfnGetConsolidatedBill );
      break;

    case COMMAND_SE_PRICE_CPP_EVT_RSP:
      status = zclSE_PriceHdlCPPEvtRsp( pInMsg, pCBs->pfnCPPEvtRsp );
      break;

    case COMMAND_SE_PRICE_GET_CREDIT_PAYMENT:
      status = zclSE_PriceHdlGetCreditPayment( pInMsg, pCBs->pfnGetCreditPayment );
      break;

    case COMMAND_SE_PRICE_GET_CURRENCY_CONVERSION:
      status = zclSE_PriceHdlGetCurrencyConversion( pInMsg, pCBs->pfnGetCurrencyConversion );
      break;

    case COMMAND_SE_PRICE_GET_TARIFF_CANCELLATION:
      status = zclSE_PriceHdlGetTariffCancellation( pInMsg, pCBs->pfnGetTariffCancellation );
      break;

    default:
      status = ZCL_STATUS_FAILURE;
      break;
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PriceMatrixSubFldParse
 *
 * @brief   Utility function for parsing "pSubFlds" payload -- see
 *          "zclSE_PricePublishPriceMatrix_t"
 *
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PriceMatrixSubFldParse( zclSE_PricePublishPriceMatrix_t *pCmd )
{
  // Initialize in case of failure
  pCmd->pSubFlds = NULL;

  // Check for valid raw fields
  if ( pCmd->rawPayloadLen && ( pCmd->pRawPayload == NULL ) )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  // Get the number of sub fields
  pCmd->numOfSubFlds = pCmd->rawPayloadLen / ZCL_SE_PRICE_MATRIX_SUB_FLD_LEN;

  if ( pCmd->numOfSubFlds )
  {
    uint8_t *pBuf = pCmd->pRawPayload;
    uint8_t fld;

    // Allocate memory for the sub field data
    pCmd->pSubFlds = OsalPort_malloc( pCmd->numOfSubFlds *
                                     sizeof( zclSE_PriceMatrixSubFld_t ) );

    if ( pCmd->pSubFlds == NULL )
    {
      return ZCL_STATUS_SOFTWARE_FAILURE;
    }

    for ( fld = 0; fld < pCmd->numOfSubFlds; fld++ )
    {
      if ( pCmd->subFldCtrl & 0x01 )
      {
        pCmd->pSubFlds[fld].tier = *pBuf++;
        pCmd->pSubFlds[fld].blockID = 0;
      }
      else
      {
        pCmd->pSubFlds[fld].tier = *pBuf >> 4;
        pCmd->pSubFlds[fld].blockID = *pBuf++ & 0x0F;
      }

      pCmd->pSubFlds[fld].price = OsalPort_buildUint32( pBuf, 4 );
      pBuf += 4;
    }
  }

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_PriceBlockThresholdSubFldParse
 *
 * @brief   Utility function for parsing "pSubFlds" payload -- see
 *          "zclSE_PricePublishBlockThreshold_t"
 *
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PriceBlockThresholdSubFldParse(
            zclSE_PricePublishBlockThreshold_t *pCmd )
{
  uint8_t *pBuf;
  uint8_t *pBufEnd;
  uint8_t subFldTotal;
  uint8_t fld;
  uint8_t thresholdCnt;

  // Initialize in case of failure
  pCmd->pSubFlds = NULL;

  // Check for valid raw fields
  if ( pCmd->rawPayloadLen && ( pCmd->pRawPayload == NULL ) )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  // Initialize buffer pointers
  pBuf = pCmd->pRawPayload;
  pBufEnd = pCmd->pRawPayload + pCmd->rawPayloadLen;

  // Calculate number of sub fields
  subFldTotal = 0;

  while ( ( pBufEnd - pBuf ) >= ZCL_SE_PRICE_BLOCK_THRESHOLD_SUB_FLD_LEN )
  {
    // Add to sub field total
    subFldTotal++;

    // How many block thresholds in this sub field
    thresholdCnt = *pBuf++ & 0x0F;

    // Check for valid block threshold length
    if ( ( pBufEnd - pBuf                                  ) <
         ( thresholdCnt * ZCL_SE_PRICE_BLOCK_THRESHOLD_LEN )   )
    {
      return ZCL_STATUS_MALFORMED_COMMAND;
    }

    // Advance to next sub field
    pBuf += ( thresholdCnt * ZCL_SE_PRICE_BLOCK_THRESHOLD_LEN );
  }


  if ( subFldTotal )
  {
    // Allocate memory for the sub field data
    pCmd->pSubFlds = OsalPort_malloc( subFldTotal *
                                     sizeof( zclSE_PriceBlockThresholdSubFld_t ) );

    if ( pCmd->pSubFlds == NULL )
    {
      return ZCL_STATUS_SOFTWARE_FAILURE;
    }

    // Save total number
    pCmd->numOfSubFlds = subFldTotal;

    // Reset buffer pointer
    pBuf = pCmd->pRawPayload;

    for ( fld = 0; fld < subFldTotal; fld++ )
    {
      pCmd->pSubFlds[fld].tier = *pBuf >> 4;
      thresholdCnt = *pBuf++ & 0x0F;
      pCmd->pSubFlds[fld].numOfBlockThresholds = thresholdCnt;

      if ( thresholdCnt )
      {
        // Point to current set of block thresholds
        pCmd->pSubFlds[fld].pBlockThresholds = (void *)pBuf;
      }
      else
      {
        // No block thresholds
        pCmd->pSubFlds[fld].pBlockThresholds = NULL;
      }

      // Advance to next sub field
      pBuf += ( thresholdCnt * ZCL_SE_PRICE_BLOCK_THRESHOLD_LEN );
    }
  }

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_PriceTierLabelParse
 *
 * @brief   Utility function for parsing "pTierLabels" payload -- see
 *          "zclSE_PricePublishTierLabels_t"
 *
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PriceTierLabelParse( zclSE_PricePublishTierLabels_t *pCmd )
{
  zclSE_PriceTierLabel_t *pTierLabels = NULL;

  // Initialize in case of failure
  pCmd->pTierLabels = NULL;

  // Check for valid raw fields
  if ( pCmd->rawPayloadLen && ( pCmd->pRawPayload == NULL ) )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  if ( pCmd->numOfTierLabels )
  {
    uint8_t *pBuf = pCmd->pRawPayload;
    uint8_t *pBufEnd = pCmd->pRawPayload + pCmd->rawPayloadLen;
    uint8_t exceeded;
    uint8_t label;

    // Allocate memory for the label data
    pTierLabels = OsalPort_malloc( pCmd->numOfTierLabels *
                                  sizeof( zclSE_PriceTierLabel_t ) );

    if ( pTierLabels == NULL )
    {
      return ZCL_STATUS_SOFTWARE_FAILURE;
    }

    for ( label = 0; label < pCmd->numOfTierLabels; label++ )
    {
      // Check for valid event payload length
      if ( ( pBufEnd - pBuf ) < ZCL_SE_PRICE_TIER_LABEL_LEN )
      {
        OsalPort_free( pTierLabels );
        return ZCL_STATUS_MALFORMED_COMMAND;
      }

      pTierLabels[label].tierID = *pBuf++;
      pBuf = zclSE_UTF8StringParse( pBuf, pBufEnd,
                                    &pTierLabels[label].tierLabel, &exceeded );

      // Check for string length exceeded flag
      if ( exceeded )
      {
        OsalPort_free( pTierLabels );
        return ZCL_STATUS_MALFORMED_COMMAND;
      }
    }
  }

  pCmd->pTierLabels = pTierLabels;

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_MessagingSendDisplayMsg
 *
 * @brief   Send COMMAND_SE_MESSAGING_DISPLAY_MSG.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MessagingSendDisplayMsg( uint8_t srcEP, afAddrType_t *dstAddr,
                                         zclSE_MessagingDisplayMsg_t *pCmd,
                                         uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t *pCmdBuf;
  uint16_t cmdBufLen;

  status = zclSE_MessagingBuildDisplayMsg( pCmd, &pCmdBuf, &cmdBufLen );

  if ( status == ZSuccess )
  {
    status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_MESSAGING,
                              COMMAND_SE_MESSAGING_DISPLAY_MSG, TRUE,
                              ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp,
                              0, seqNum, cmdBufLen, pCmdBuf );

    OsalPort_free( pCmdBuf );
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_MessagingSendCancelMsg
 *
 * @brief   Send COMMAND_SE_MESSAGING_CANCEL_MSG.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MessagingSendCancelMsg( uint8_t srcEP, afAddrType_t *dstAddr,
                                        zclSE_MessagingCancelMsg_t *pCmd,
                                        uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t cmdBuf[ZCL_SE_MESSAGING_CANCEL_MSG_LEN];
  uint8_t *pBuf = cmdBuf;

  pBuf = OsalPort_bufferUint32( pBuf, pCmd->msgID );
  *pBuf = pCmd->msgCtrl;

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_MESSAGING,
                            COMMAND_SE_MESSAGING_CANCEL_MSG, TRUE,
                            ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp,
                            0, seqNum, ZCL_SE_MESSAGING_CANCEL_MSG_LEN, cmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_MessagingSendDisplayProtectedMsg
 *
 * @brief   Send COMMAND_SE_MESSAGING_DISPLAY_PROTECTED_MSG.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MessagingSendDisplayProtectedMsg(
            uint8_t srcEP, afAddrType_t *dstAddr,
            zclSE_MessagingDisplayMsg_t *pCmd,
            uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t *pCmdBuf;
  uint16_t cmdBufLen;

  status = zclSE_MessagingBuildDisplayMsg( pCmd, &pCmdBuf, &cmdBufLen );

  if ( status == ZSuccess )
  {
    status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_MESSAGING,
                              COMMAND_SE_MESSAGING_DISPLAY_PROTECTED_MSG, TRUE,
                              ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp,
                              0, seqNum, cmdBufLen, pCmdBuf );

    OsalPort_free( pCmdBuf );
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_MessagingSendCancelAllMsgs
 *
 * @brief   Send COMMAND_SE_MESSAGING_CANCEL_ALL_MSGS.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MessagingSendCancelAllMsgs( uint8_t srcEP, afAddrType_t *dstAddr,
                                            zclSE_MessagingCancelAllMsgs_t *pCmd,
                                            uint8_t disableDefaultRsp, uint8_t seqNum )
{

  ZStatus_t status;
  uint8_t cmdBuf[ZCL_SE_MESSAGING_CANCEL_ALL_MSGS_LEN];
  uint8_t *pBuf = cmdBuf;

  OsalPort_bufferUint32( pBuf, pCmd->implTime );

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_MESSAGING,
                            COMMAND_SE_MESSAGING_CANCEL_ALL_MSGS, TRUE,
                            ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp,
                            0, seqNum, ZCL_SE_MESSAGING_CANCEL_ALL_MSGS_LEN, cmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_MessagingSendMsgCfm
 *
 * @brief   Send COMMAND_SE_MESSAGING_MSG_CFM.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MessagingSendMsgCfm( uint8_t srcEP, afAddrType_t *dstAddr,
                                     zclSE_MessagingMsgCfm_t *pCmd,
                                     uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t *pCmdBuf;
  uint16_t cmdBufLen;
  uint8_t *pBuf;

  // Allocate command buffer
  cmdBufLen = ZCL_SE_MESSAGING_MSG_CFM_LEN + zclSE_UTF8StringLen( &pCmd->msgCfmRsp );

  pCmdBuf = OsalPort_malloc( cmdBufLen );
  if ( pCmdBuf == NULL )
  {
    return ZMemError;
  }

  pBuf = pCmdBuf;
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->msgID );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->cfmTime );
  *pBuf++ = pCmd->msgCfmCtrl;
  pBuf = zclSE_UTF8StringBuild( pBuf, &pCmd->msgCfmRsp );

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_MESSAGING,
                            COMMAND_SE_MESSAGING_MSG_CFM, TRUE,
                            ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp,
                            0, seqNum, cmdBufLen, pCmdBuf );

  OsalPort_free( pCmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_MessagingSendGetMsgCancellation
 *
 * @brief   Send COMMAND_SE_MESSAGING_GET_MSG_CANCELLATION.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MessagingSendGetMsgCancellation
            ( uint8_t srcEP, afAddrType_t *dstAddr,
              zclSE_MessagingGetMsgCancellation_t *pCmd,
              uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t cmdBuf[ZCL_SE_MESSAGING_GET_MSG_CANCELLATION_LEN];
  uint8_t *pBuf = cmdBuf;

  OsalPort_bufferUint32( pBuf, pCmd->earliestImplTime );

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_MESSAGING,
                            COMMAND_SE_MESSAGING_GET_MSG_CANCELLATION, TRUE,
                            ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp,
                            0, seqNum, ZCL_SE_MESSAGING_GET_MSG_CANCELLATION_LEN,
                            cmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_MessagingParseDisplayMsg
 *
 * @brief   Parse COMMAND_SE_MESSAGING_DISPLAY_MSG.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MessagingParseDisplayMsg( zclIncoming_t *pInMsg,
                                          zclSE_MessagingDisplayMsg_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;
  uint8_t *pBufEnd = pInMsg->pData + pInMsg->pDataLen;
  uint8_t exceeded;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_MESSAGING_DISPLAY_MSG_OLD_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->msgID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->msgCtrl.transmission = *pBuf & 0x03;
  pCmd->msgCtrl.importanceLevel = *pBuf & 0x0C >> 2;
  pCmd->msgCtrl.enhancedCfmRequired = *pBuf & 0x60 >> 5;
  pCmd->msgCtrl.msgCfmRequired = *pBuf++ >> 7;
  pCmd->startTime = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->duration = BUILD_UINT16( pBuf[0], pBuf[1] );
  pBuf += 2;
  pBuf = zclSE_UTF8StringParse( pBuf, pBufEnd, &pCmd->msg, &exceeded );

  // Check for string length exceeded flag
  if ( exceeded )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  // Check for SE 1.2 optional fields
  if ( pBuf < pBufEnd )
  {
    pCmd->extMsgCtrl.msgConfirmed = *pBuf;
  }

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_MessagingParseCancelMsg
 *
 * @brief   Parse COMMAND_SE_MESSAGING_CANCEL_MSG.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MessagingParseCancelMsg( zclIncoming_t *pInMsg,
                                         zclSE_MessagingCancelMsg_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_MESSAGING_CANCEL_MSG_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->msgID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->msgCtrl = *pBuf;

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_MessagingParseCancelAllMsgs
 *
 * @brief   Parse COMMAND_SE_MESSAGING_CANCEL_ALL_MSGS.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MessagingParseCancelAllMsgs(
            zclIncoming_t *pInMsg,
            zclSE_MessagingCancelAllMsgs_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_MESSAGING_CANCEL_ALL_MSGS_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->implTime = OsalPort_buildUint32( pBuf, 4 );

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_MessagingParseMsgCfm
 *
 * @brief   Parse COMMAND_SE_MESSAGING_MSG_CFM.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MessagingParseMsgCfm( zclIncoming_t *pInMsg,
                                      zclSE_MessagingMsgCfm_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;
  uint8_t *pBufEnd = pInMsg->pData + pInMsg->pDataLen;
  uint8_t exceeded;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_MESSAGING_MSG_CFM_OLD_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  // Initialize all fields
  memset( pCmd, 0, sizeof( zclSE_MessagingMsgCfm_t ) );

  pCmd->msgID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->cfmTime = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;

  // Check for SE 1.2 optional fields
  if ( pInMsg->pDataLen >= ZCL_SE_MESSAGING_MSG_CFM_LEN )
  {
    pCmd->msgCfmCtrl = *pBuf++;
    pBuf = zclSE_UTF8StringParse( pBuf, pBufEnd, &pCmd->msgCfmRsp, &exceeded );

    // Check for string length exceeded flag
    if ( exceeded )
    {
      return ZCL_STATUS_MALFORMED_COMMAND;
    }
  }

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_MessagingParseGetMsgCancellation
 *
 * @brief   Parse COMMAND_SE_MESSAGING_GET_MSG_CANCELLATION.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MessagingParseGetMsgCancellation
            ( zclIncoming_t *pInMsg,
              zclSE_MessagingGetMsgCancellation_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_MESSAGING_GET_MSG_CANCELLATION_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->earliestImplTime = OsalPort_buildUint32( pBuf, 4 );

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_MessagingHdlDisplayMsg
 *
 * @brief   Handle COMMAND_SE_MESSAGING_DISPLAY_MSG.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MessagingHdlDisplayMsg( zclIncoming_t *pInMsg,
                                        zclSE_MessagingDisplayMsgCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_MessagingDisplayMsg_t *pCmd;

    pCmd = OsalPort_malloc( sizeof( zclSE_MessagingDisplayMsg_t ) );
    if ( pCmd == NULL )
    {
       return ZCL_STATUS_SOFTWARE_FAILURE;
    }

    status = zclSE_MessagingParseDisplayMsg( pInMsg, pCmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      pfnCB( pInMsg, pCmd );
    }

    OsalPort_free( pCmd );
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_MessagingHdlCancelMsg
 *
 * @brief   Handle COMMAND_SE_MESSAGING_CANCEL_MSG.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MessagingHdlCancelMsg( zclIncoming_t *pInMsg,
                                       zclSE_MessagingCancelMsgCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_MessagingCancelMsg_t cmd;

    status = zclSE_MessagingParseCancelMsg( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      pfnCB( pInMsg, &cmd );
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_MessagingHdlDisplayProtectedMsg
 *
 * @brief   Handle COMMAND_SE_MESSAGING_DISPLAY_PROTECTED_MSG.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MessagingHdlDisplayProtectedMsg(
            zclIncoming_t *pInMsg,
            zclSE_MessagingDisplayProtectedMsgCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_MessagingDisplayMsg_t *pCmd;

    pCmd = OsalPort_malloc( sizeof( zclSE_MessagingDisplayMsg_t ) );
    if ( pCmd == NULL )
    {
       return ZCL_STATUS_SOFTWARE_FAILURE;
    }

    status = zclSE_MessagingParseDisplayMsg( pInMsg, pCmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      pfnCB( pInMsg, pCmd );
    }

    OsalPort_free( pCmd );
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_MessagingHdlCancelAllMsgs
 *
 * @brief   Handle COMMAND_SE_MESSAGING_CANCEL_ALL_MSGS.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MessagingHdlCancelAllMsgs( zclIncoming_t *pInMsg,
                                           zclSE_MessagingCancelAllMsgsCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_MessagingCancelAllMsgs_t cmd;

    status = zclSE_MessagingParseCancelAllMsgs( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      pfnCB( pInMsg, &cmd );
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_MessagingHdlGetLastMsg
 *
 * @brief   Handle COMMAND_SE_MESSAGING_GET_LAST_MSG (handle with no payload).
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MessagingHdlGetLastMsg( zclIncoming_t *pInMsg,
                                        zclSE_MessagingGetLastMsgCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    status = pfnCB( pInMsg );
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_MessagingHdlMsgCfm
 *
 * @brief   Handle COMMAND_SE_MESSAGING_MSG_CFM.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MessagingHdlMsgCfm( zclIncoming_t *pInMsg,
                                    zclSE_MessagingMsgCfmCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_MessagingMsgCfm_t cmd;

    status = zclSE_MessagingParseMsgCfm( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      pfnCB( pInMsg, &cmd );
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_MessagingHdlGetMsgCancellation
 *
 * @brief   Handle COMMAND_SE_MESSAGING_GET_MSG_CANCELLATION.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MessagingHdlGetMsgCancellation(
            zclIncoming_t *pInMsg,
            zclSE_MessagingGetMsgCancellationCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_MessagingGetMsgCancellation_t cmd;

    status = zclSE_MessagingParseGetMsgCancellation( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      status = pfnCB( pInMsg, &cmd );
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_MessagingHdlClientCmd
 *
 * @brief   Client command handler for ZCL_CLUSTER_ID_SE_MESSAGING.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCBs - client callbacks
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MessagingHdlClientCmd( zclIncoming_t *pInMsg,
                                       const zclSE_MessagingClientCBs_t *pCBs )
{
  ZStatus_t status;

  // Guard against NULL pointer
  if ( pCBs == NULL )
  {
    return ZCL_STATUS_FAILURE;
  }

  // Process Server-to-Client Commands
  switch ( pInMsg->hdr.commandID )
  {
    case COMMAND_SE_MESSAGING_DISPLAY_MSG:
      status = zclSE_MessagingHdlDisplayMsg( pInMsg, pCBs->pfnDisplayMsg );
      break;

    case COMMAND_SE_MESSAGING_CANCEL_MSG:
      status = zclSE_MessagingHdlCancelMsg( pInMsg, pCBs->pfnCancelMsg );
      break;

    case COMMAND_SE_MESSAGING_DISPLAY_PROTECTED_MSG:
      status = zclSE_MessagingHdlDisplayProtectedMsg( pInMsg, pCBs->pfnDisplayProtectedMsg );
      break;

    case COMMAND_SE_MESSAGING_CANCEL_ALL_MSGS:
      status = zclSE_MessagingHdlCancelAllMsgs( pInMsg, pCBs->pfnCancelAllMsgs );
      break;

    default:
      status = ZCL_STATUS_FAILURE;
      break;
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_MessagingHdlServerCmd
 *
 * @brief   Server command handler for ZCL_CLUSTER_ID_SE_MESSAGING.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCBs - server callbacks
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MessagingHdlServerCmd( zclIncoming_t *pInMsg,
                                       const zclSE_MessagingServerCBs_t *pCBs )
{
  ZStatus_t status;

  // Guard against NULL pointer
  if ( pCBs == NULL )
  {
    return ZCL_STATUS_FAILURE;
  }

  // Process Client-to-Server Commands
  switch ( pInMsg->hdr.commandID )
  {
    case COMMAND_SE_MESSAGING_GET_LAST_MSG:
      status = zclSE_MessagingHdlGetLastMsg( pInMsg, pCBs->pfnGetLastMsg );
      break;

    case COMMAND_SE_MESSAGING_MSG_CFM:
      status = zclSE_MessagingHdlMsgCfm( pInMsg, pCBs->pfnMsgCfm );
      break;

    case COMMAND_SE_MESSAGING_GET_MSG_CANCELLATION:
      status = zclSE_MessagingHdlGetMsgCancellation( pInMsg, pCBs->pfnGetMsgCancellation );
      break;

    default:
      status = ZCL_STATUS_FAILURE;
      break;
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_TunnelingSendReqTunnelRsp
 *
 * @brief   Send COMMAND_SE_TUNNELING_REQ_TUNNEL_RSP.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_TunnelingSendReqTunnelRsp( uint8_t srcEP, afAddrType_t *dstAddr,
                                           zclSE_TunnelingReqTunnelRsp_t *pCmd,
                                           uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t cmdBuf[ZCL_SE_TUNNELING_REQ_TUNNEL_RSP_LEN];
  uint8_t *pBuf = cmdBuf;

  *pBuf++ = LO_UINT16( pCmd->tunnelID ) ;
  *pBuf++ = HI_UINT16( pCmd->tunnelID );
  *pBuf++ = pCmd->status;
  *pBuf++ = LO_UINT16( pCmd->maxTransferSize ) ;
  *pBuf   = HI_UINT16( pCmd->maxTransferSize );

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_TUNNELING,
                            COMMAND_SE_TUNNELING_REQ_TUNNEL_RSP, TRUE,
                            ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0,
                            seqNum, ZCL_SE_TUNNELING_REQ_TUNNEL_RSP_LEN, cmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_TunnelingSendServerTransferData
 *
 * @brief   Send COMMAND_SE_TUNNELING_SERVER_TRANSFER_DATA.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_TunnelingSendServerTransferData(
            uint8_t srcEP, afAddrType_t *dstAddr,
            zclSE_TunnelingTransferData_t *pCmd,
            uint8_t disableDefaultRsp, uint8_t seqNum )
{
  return zclSE_TunnelingSendTransferData
           ( srcEP, dstAddr, COMMAND_SE_TUNNELING_SERVER_TRANSFER_DATA,
             pCmd, ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, seqNum );
}

/**************************************************************************************************
 * @fn      zclSE_TunnelingSendServerTransferDataErr
 *
 * @brief   Send COMMAND_SE_TUNNELING_SERVER_TRANSFER_DATA_ERR.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_TunnelingSendServerTransferDataErr(
            uint8_t srcEP, afAddrType_t *dstAddr,
            zclSE_TunnelingTransferDataErr_t *pCmd,
            uint8_t disableDefaultRsp, uint8_t seqNum )
{
  return zclSE_TunnelingSendTransferDataErr
           ( srcEP, dstAddr, COMMAND_SE_TUNNELING_SERVER_TRANSFER_DATA_ERR,
             pCmd, ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, seqNum );
}

/**************************************************************************************************
 * @fn      zclSE_TunnelingSendServerAckTransferData
 *
 * @brief   Send COMMAND_SE_TUNNELING_SERVER_ACK_TRANSFER_DATA.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_TunnelingSendServerAckTransferData(
            uint8_t srcEP, afAddrType_t *dstAddr,
            zclSE_TunnelingAckTransferData_t *pCmd,
            uint8_t disableDefaultRsp, uint8_t seqNum )
{
  return zclSE_TunnelingSendAckTransferData
           ( srcEP, dstAddr, COMMAND_SE_TUNNELING_SERVER_ACK_TRANSFER_DATA,
             pCmd, ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, seqNum );
}

/**************************************************************************************************
 * @fn      zclSE_TunnelingSendServerReadyData
 *
 * @brief   Send COMMAND_SE_TUNNELING_SERVER_READY_DATA.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_TunnelingSendServerReadyData(
            uint8_t srcEP, afAddrType_t *dstAddr,
            zclSE_TunnelingReadyData_t *pCmd,
            uint8_t disableDefaultRsp, uint8_t seqNum )
{
  return zclSE_TunnelingSendReadyData
           ( srcEP, dstAddr, COMMAND_SE_TUNNELING_SERVER_READY_DATA,
             pCmd, ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, seqNum );
}

/**************************************************************************************************
 * @fn      zclSE_TunnelingSendSuppTunnelProtocolsRsp
 *
 * @brief   Send COMMAND_SE_TUNNELING_SUPP_TUNNEL_PROTOCOLS_RSP.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_TunnelingSendSuppTunnelProtocolsRsp(
            uint8_t srcEP, afAddrType_t *dstAddr,
            zclSE_TunnelingSuppTunnelProtocolsRsp_t *pCmd,
            uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t *pCmdBuf;
  uint16_t cmdBufLen;
  uint8_t *pBuf;
  uint8_t proto;

  // Allocate command buffer
  cmdBufLen = ZCL_SE_TUNNELING_SUPP_TUNNEL_PROTOCOLS_RSP_LEN +
              ( pCmd->protoListCount * ZCL_SE_TUNNELING_PROTOCOL_PAYLOAD_LEN );

  pCmdBuf = OsalPort_malloc( cmdBufLen );
  if ( pCmdBuf == NULL )
  {
    return ZMemError;
  }

  pBuf = pCmdBuf;
  *pBuf++ = pCmd->protoListComplete;
  *pBuf++ = pCmd->protoListCount;

  for ( proto = 0; proto < pCmd->protoListCount; proto++ )
  {
    *pBuf++ = LO_UINT16( pCmd->protoList[proto].manuCode );
    *pBuf++ = HI_UINT16( pCmd->protoList[proto].manuCode );
    *pBuf++ = pCmd->protoList[proto].protoID;
  }

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_TUNNELING,
                            COMMAND_SE_TUNNELING_SUPP_TUNNEL_PROTOCOLS_RSP, TRUE,
                            ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0,
                            seqNum, cmdBufLen, pCmdBuf );

  OsalPort_free( pCmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_TunnelingSendTunnelClosureNotif
 *
 * @brief   Send COMMAND_SE_TUNNELING_TUNNEL_CLOSURE_NOTIF.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_TunnelingSendTunnelClosureNotif(
            uint8_t srcEP, afAddrType_t *dstAddr,
            zclSE_TunnelingTunnelClosureNotif_t *pCmd,
            uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t cmdBuf[ZCL_SE_TUNNELING_TUNNEL_CLOSURE_NOTIF_LEN];
  uint8_t *pBuf = cmdBuf;

  *pBuf++ = LO_UINT16( pCmd->tunnelID );
  *pBuf++ = HI_UINT16( pCmd->tunnelID );

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_TUNNELING,
                            COMMAND_SE_TUNNELING_TUNNEL_CLOSURE_NOTIF, TRUE,
                            ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0,
                            seqNum, ZCL_SE_TUNNELING_TUNNEL_CLOSURE_NOTIF_LEN, cmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_TunnelingSendReqTunnel
 *
 * @brief   Send COMMAND_SE_TUNNELING_REQ_TUNNEL.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_TunnelingSendReqTunnel( uint8_t srcEP, afAddrType_t *dstAddr,
                                        zclSE_TunnelingReqTunnel_t *pCmd,
                                        uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t cmdBuf[ZCL_SE_TUNNELING_REQ_TUNNEL_LEN];
  uint8_t *pBuf = cmdBuf;

  *pBuf++ = pCmd->protoID;
  *pBuf++ = LO_UINT16( pCmd->manuCode );
  *pBuf++ = HI_UINT16( pCmd->manuCode );
  *pBuf++ = pCmd->flowCtrlSupp;
  *pBuf++ = LO_UINT16( pCmd->maxTransferSize );
  *pBuf   = HI_UINT16( pCmd->maxTransferSize );

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_TUNNELING,
                            COMMAND_SE_TUNNELING_REQ_TUNNEL, TRUE,
                            ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0,
                            seqNum, ZCL_SE_TUNNELING_REQ_TUNNEL_LEN, cmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_TunnelingSendCloseTunnel
 *
 * @brief   Send COMMAND_SE_TUNNELING_CLOSE_TUNNEL.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_TunnelingSendCloseTunnel( uint8_t srcEP, afAddrType_t *dstAddr,
                                          zclSE_TunnelingCloseTunnel_t *pCmd,
                                          uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t cmdBuf[ZCL_SE_TUNNELING_CLOSE_TUNNEL_LEN];
  uint8_t *pBuf = cmdBuf;

  *pBuf++ = LO_UINT16( pCmd->tunnelID );
  *pBuf   = HI_UINT16( pCmd->tunnelID );

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_TUNNELING,
                            COMMAND_SE_TUNNELING_CLOSE_TUNNEL, TRUE,
                            ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0,
                            seqNum, ZCL_SE_TUNNELING_CLOSE_TUNNEL_LEN, cmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_TunnelingSendClientTransferData
 *
 * @brief   Send COMMAND_SE_TUNNELING_CLIENT_TRANSFER_DATA.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_TunnelingSendClientTransferData(
            uint8_t srcEP, afAddrType_t *dstAddr,
            zclSE_TunnelingTransferData_t *pCmd,
            uint8_t disableDefaultRsp, uint8_t seqNum )
{
  return zclSE_TunnelingSendTransferData
           ( srcEP, dstAddr, COMMAND_SE_TUNNELING_CLIENT_TRANSFER_DATA,
             pCmd, ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, seqNum );
}

/**************************************************************************************************
 * @fn      zclSE_TunnelingSendClientTransferDataErr
 *
 * @brief   Send COMMAND_SE_TUNNELING_CLIENT_TRANSFER_DATA_ERR.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_TunnelingSendClientTransferDataErr(
            uint8_t srcEP, afAddrType_t *dstAddr,
            zclSE_TunnelingTransferDataErr_t *pCmd,
            uint8_t disableDefaultRsp, uint8_t seqNum )
{
  return zclSE_TunnelingSendTransferDataErr
           ( srcEP, dstAddr, COMMAND_SE_TUNNELING_CLIENT_TRANSFER_DATA_ERR,
             pCmd, ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, seqNum );
}

/**************************************************************************************************
 * @fn      zclSE_TunnelingSendClientAckTransferData
 *
 * @brief   Send COMMAND_SE_TUNNELING_CLIENT_ACK_TRANSFER_DATA.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_TunnelingSendClientAckTransferData(
            uint8_t srcEP, afAddrType_t *dstAddr,
            zclSE_TunnelingAckTransferData_t *pCmd,
            uint8_t disableDefaultRsp, uint8_t seqNum )
{
  return zclSE_TunnelingSendAckTransferData
           ( srcEP, dstAddr, COMMAND_SE_TUNNELING_CLIENT_ACK_TRANSFER_DATA,
             pCmd, ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, seqNum );
}

/**************************************************************************************************
 * @fn      zclSE_TunnelingSendClientReadyData
 *
 * @brief   Send COMMAND_SE_TUNNELING_CLIENT_READY_DATA.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_TunnelingSendClientReadyData(
            uint8_t srcEP, afAddrType_t *dstAddr,
            zclSE_TunnelingReadyData_t *pCmd,
            uint8_t disableDefaultRsp, uint8_t seqNum )
{
  return zclSE_TunnelingSendReadyData
           ( srcEP, dstAddr, COMMAND_SE_TUNNELING_CLIENT_READY_DATA,
             pCmd, ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, seqNum );
}

/**************************************************************************************************
 * @fn      zclSE_TunnelingSendGetSuppTunnelProtocols
 *
 * @brief   Send COMMAND_SE_TUNNELING_GET_SUPP_TUNNEL_PROTOCOLS.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_TunnelingSendGetSuppTunnelProtocols(
            uint8_t srcEP, afAddrType_t *dstAddr,
            zclSE_TunnelingGetSuppTunnelProtocols_t *pCmd,
            uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t cmdBuf[ZCL_SE_TUNNELING_GET_SUPP_TUNNEL_PROTOCOLS_LEN];
  uint8_t *pBuf = cmdBuf;

  *pBuf = pCmd->offset;

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_TUNNELING,
                            COMMAND_SE_TUNNELING_GET_SUPP_TUNNEL_PROTOCOLS, TRUE,
                            ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0,
                            seqNum, ZCL_SE_TUNNELING_GET_SUPP_TUNNEL_PROTOCOLS_LEN, cmdBuf );
  return status;
}

/**************************************************************************************************
 * @fn      zclSE_TunnelingParseReqTunnelRsp
 *
 * @brief   Parse COMMAND_SE_TUNNELING_REQ_TUNNEL_RSP.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_TunnelingParseReqTunnelRsp( zclIncoming_t *pInMsg,
                                            zclSE_TunnelingReqTunnelRsp_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_TUNNELING_REQ_TUNNEL_RSP_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->tunnelID = BUILD_UINT16( pBuf[0], pBuf[1] );
  pBuf += 2;
  pCmd->status = *pBuf++;
  pCmd->maxTransferSize = BUILD_UINT16( pBuf[0], pBuf[1] );

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_TunnelingParseTransferData
 *
 * @brief   Parse COMMAND_SE_TUNNELING_SERVER_TRANSFER_DATA and
 *                COMMAND_SE_TUNNELING_CLIENT_TRANSFER_DATA.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_TunnelingParseTransferData(
            zclIncoming_t *pInMsg,
            zclSE_TunnelingTransferData_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_TUNNELING_TRANSFER_DATA_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->tunnelID = BUILD_UINT16( pBuf[0], pBuf[1] );
  pBuf += 2;
  pCmd->dataLen = pInMsg->pDataLen - ZCL_SE_TUNNELING_TRANSFER_DATA_LEN;
  if ( pCmd->dataLen )
  {
    pCmd->pData = pBuf;
  }
  else
  {
    pCmd->pData = NULL;
  }

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_TunnelingParseTransferDataErr
 *
 * @brief   Parse COMMAND_SE_TUNNELING_SERVER_TRANSFER_DATA_ERR and
 *                COMMAND_SE_TUNNELING_CLIENT_TRANSFER_DATA_ERR.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_TunnelingParseTransferDataErr(
            zclIncoming_t *pInMsg,
            zclSE_TunnelingTransferDataErr_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_TUNNELING_TRANSFER_DATA_ERR_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->tunnelID = BUILD_UINT16( pBuf[0], pBuf[1] );
  pBuf += 2;
  pCmd->status = *pBuf;

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_TunnelingParseAckTransferData
 *
 * @brief   Parse COMMAND_SE_TUNNELING_SERVER_ACK_TRANSFER_DATA and
 *                COMMAND_SE_TUNNELING_CLIENT_ACK_TRANSFER_DATA.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_TunnelingParseAckTransferData(
            zclIncoming_t *pInMsg,
            zclSE_TunnelingAckTransferData_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_TUNNELING_ACK_TRANSFER_DATA_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->tunnelID = BUILD_UINT16( pBuf[0], pBuf[1] );
  pBuf += 2;
  pCmd->numOfBytesLeft = BUILD_UINT16( pBuf[0], pBuf[1] );

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_TunnelingParseReadyData
 *
 * @brief   Parse COMMAND_SE_TUNNELING_SERVER_READY_DATA and
 *                COMMAND_SE_TUNNELING_CLIENT_READY_DATA.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_TunnelingParseReadyData(
            zclIncoming_t *pInMsg,
            zclSE_TunnelingReadyData_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_TUNNELING_READY_DATA_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->tunnelID = BUILD_UINT16( pBuf[0], pBuf[1] );
  pBuf += 2;
  pCmd->numOfOctetsLeft = BUILD_UINT16( pBuf[0], pBuf[1] );

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_TunnelingParseSuppTunnelProtocolsRsp
 *
 * @brief   Parse COMMAND_SE_TUNNELING_SUPP_TUNNEL_PROTOCOLS_RSP.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_TunnelingParseSuppTunnelProtocolsRsp(
            zclIncoming_t *pInMsg,
            zclSE_TunnelingSuppTunnelProtocolsRsp_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;
  uint8_t proto;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_TUNNELING_SUPP_TUNNEL_PROTOCOLS_RSP_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->protoListComplete = *pBuf++;
  pCmd->protoListCount = *pBuf++;

  // Check for protocol list
  if ( pCmd->protoListCount )
  {
    // Check for valid protocol list length
    if ( ( pInMsg->pDataLen - ZCL_SE_TUNNELING_SUPP_TUNNEL_PROTOCOLS_RSP_LEN ) <
         ( pCmd->protoListCount * ZCL_SE_TUNNELING_PROTOCOL_PAYLOAD_LEN )        )
    {
      return ZCL_STATUS_MALFORMED_COMMAND;
    }

    // Allocate buffer
    pCmd->protoList =
      OsalPort_malloc( pCmd->protoListCount * sizeof( zclSE_TunnelingProtocol_t ) );
    if ( pCmd->protoList == NULL )
    {
      return ZCL_STATUS_SOFTWARE_FAILURE;
    }

    // Copy the data
    for ( proto = 0; proto < pCmd->protoListCount; proto++ )
    {
      pCmd->protoList[proto].manuCode = BUILD_UINT16( pBuf[0], pBuf[1] );
      pBuf += 2;
      pCmd->protoList[proto].protoID = *pBuf++;
    }
  }
  else
  {
    pCmd->protoList = NULL;
  }

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_TunnelingParseTunnelClosureNotif
 *
 * @brief   Parse COMMAND_SE_TUNNELING_TUNNEL_CLOSURE_NOTIF.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_TunnelingParseTunnelClosureNotif(
            zclIncoming_t *pInMsg,
            zclSE_TunnelingTunnelClosureNotif_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_TUNNELING_TUNNEL_CLOSURE_NOTIF_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->tunnelID = BUILD_UINT16( pBuf[0], pBuf[1] );

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_TunnelingParseReqTunnel
 *
 * @brief   Parse COMMAND_SE_TUNNELING_REQ_TUNNEL.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_TunnelingParseReqTunnel( zclIncoming_t *pInMsg,
                                         zclSE_TunnelingReqTunnel_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_TUNNELING_REQ_TUNNEL_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->protoID = *pBuf++;
  pCmd->manuCode = BUILD_UINT16( pBuf[0], pBuf[1] );
  pBuf += 2;
  pCmd->flowCtrlSupp = *pBuf++;
  pCmd->maxTransferSize = BUILD_UINT16( pBuf[0], pBuf[1] );

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_TunnelingParseCloseTunnel
 *
 * @brief   Parse COMMAND_SE_TUNNELING_CLOSE_TUNNEL.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_TunnelingParseCloseTunnel( zclIncoming_t *pInMsg,
                                           zclSE_TunnelingCloseTunnel_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;

  if ( pInMsg->pDataLen < ZCL_SE_TUNNELING_CLOSE_TUNNEL_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->tunnelID = BUILD_UINT16( pBuf[0], pBuf[1] );

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_TunnelingParseGetSuppTunnelProtocols
 *
 * @brief   Parse COMMAND_SE_TUNNELING_GET_SUPP_TUNNEL_PROTOCOLS.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_TunnelingParseGetSuppTunnelProtocols(
            zclIncoming_t *pInMsg,
            zclSE_TunnelingGetSuppTunnelProtocols_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;

  if ( pInMsg->pDataLen < ZCL_SE_TUNNELING_GET_SUPP_TUNNEL_PROTOCOLS_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->offset = *pBuf;

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_TunnelingHdlReqTunnelRsp
 *
 * @brief   Handle COMMAND_SE_TUNNELING_REQ_TUNNEL_RSP.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_TunnelingHdlReqTunnelRsp( zclIncoming_t *pInMsg,
                                          zclSE_TunnelingReqTunnelRspCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_TunnelingReqTunnelRsp_t cmd;

    status = zclSE_TunnelingParseReqTunnelRsp( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      pfnCB( pInMsg, &cmd );
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_TunnelingHdlTransferData
 *
 * @brief   Handle COMMAND_SE_TUNNELING_SERVER_TRANSFER_DATA and
 *                 COMMAND_SE_TUNNELING_CLIENT_TRANSFER_DATA.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_TunnelingHdlTransferData(
            zclIncoming_t *pInMsg,
            zclSE_TunnelingTransferDataCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_TunnelingTransferData_t cmd;

    status = zclSE_TunnelingParseTransferData( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      pfnCB( pInMsg, &cmd );

      return ZCL_STATUS_CMD_HAS_RSP;
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_TunnelingHdlTransferDataErr
 *
 * @brief   Handle COMMAND_SE_TUNNELING_SERVER_TRANSFER_DATA_ERR and
 *                 COMMAND_SE_TUNNELING_CLIENT_TRANSFER_DATA_ERR.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_TunnelingHdlTransferDataErr(
            zclIncoming_t *pInMsg,
            zclSE_TunnelingTransferDataErrCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_TunnelingTransferDataErr_t cmd;

    status = zclSE_TunnelingParseTransferDataErr( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      pfnCB( pInMsg, &cmd );
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_TunnelingHdlAckTransferData
 *
 * @brief   Handle COMMAND_SE_TUNNELING_SERVER_ACK_TRANSFER_DATA and
 *                 COMMAND_SE_TUNNELING_CLIENT_ACK_TRANSFER_DATA.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_TunnelingHdlAckTransferData(
            zclIncoming_t *pInMsg,
            zclSE_TunnelingAckTransferDataCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_TunnelingAckTransferData_t cmd;

    status = zclSE_TunnelingParseAckTransferData( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      pfnCB( pInMsg, &cmd );
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_TunnelingHdlReadyData
 *
 * @brief   Handle COMMAND_SE_TUNNELING_SERVER_READY_DATA and
 *                 COMMAND_SE_TUNNELING_CLIENT_READY_DATA.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_TunnelingHdlReadyData(
            zclIncoming_t *pInMsg,
            zclSE_TunnelingReadyDataCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_TunnelingReadyData_t cmd;

    status = zclSE_TunnelingParseReadyData( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      pfnCB( pInMsg, &cmd );
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_TunnelingHdlSuppTunnelProtocolsRsp
 *
 * @brief   Handle COMMAND_SE_TUNNELING_SUPP_TUNNEL_PROTOCOLS_RSP.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_TunnelingHdlSuppTunnelProtocolsRsp(
            zclIncoming_t *pInMsg,
            zclSE_TunnelingSuppTunnelProtocolsRspCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_TunnelingSuppTunnelProtocolsRsp_t cmd;

    status = zclSE_TunnelingParseSuppTunnelProtocolsRsp( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      pfnCB( pInMsg, &cmd );

      // Free allocated memory
      if ( cmd.protoList )
      {
        OsalPort_free( cmd.protoList );
      }
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_TunnelingHdlTunnelClosureNotif
 *
 * @brief   Handle COMMAND_SE_TUNNELING_TUNNEL_CLOSURE_NOTIF.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_TunnelingHdlTunnelClosureNotif(
            zclIncoming_t *pInMsg,
            zclSE_TunnelingTunnelClosureNotifCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_TunnelingTunnelClosureNotif_t cmd;

    status = zclSE_TunnelingParseTunnelClosureNotif( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      pfnCB( pInMsg, &cmd );
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_TunnelingHdlReqTunnel
 *
 * @brief   Handle COMMAND_SE_TUNNELING_REQ_TUNNEL.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_TunnelingHdlReqTunnel( zclIncoming_t *pInMsg,
                                       zclSE_TunnelingReqTunnelCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_TunnelingReqTunnel_t cmd;

    status = zclSE_TunnelingParseReqTunnel( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      pfnCB( pInMsg, &cmd );

      status = ZCL_STATUS_CMD_HAS_RSP;
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_TunnelingHdlCloseTunnel
 *
 * @brief   Handle COMMAND_SE_TUNNELING_CLOSE_TUNNEL.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_TunnelingHdlCloseTunnel( zclIncoming_t *pInMsg,
                                         zclSE_TunnelingCloseTunnelCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_TunnelingCloseTunnel_t cmd;

    status = zclSE_TunnelingParseCloseTunnel( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      status = pfnCB( pInMsg, &cmd );
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_TunnelingHdlGetSuppTunnelProtocols
 *
 * @brief   Handle COMMAND_SE_TUNNELING_GET_SUPP_TUNNEL_PROTOCOLS.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_TunnelingHdlGetSuppTunnelProtocols(
            zclIncoming_t *pInMsg,
            zclSE_TunnelingGetSuppTunnelProtocolsCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_TunnelingGetSuppTunnelProtocols_t cmd;

    status = zclSE_TunnelingParseGetSuppTunnelProtocols( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      pfnCB( pInMsg, &cmd );

      status = ZCL_STATUS_CMD_HAS_RSP;
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_TunnelingHdlClientCmd
 *
 * @brief   Client command handler for ZCL_CLUSTER_ID_SE_TUNNELING.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCBs - client callbacks
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_TunnelingHdlClientCmd( zclIncoming_t *pInMsg,
                                       const zclSE_TunnelingClientCBs_t *pCBs )
{
  ZStatus_t status;

  // Guard against NULL pointer
  if ( pCBs == NULL )
  {
    return ZCL_STATUS_FAILURE;
  }

  // Process Server-to-Client Commands
  switch ( pInMsg->hdr.commandID )
  {
    case COMMAND_SE_TUNNELING_REQ_TUNNEL_RSP:
      status = zclSE_TunnelingHdlReqTunnelRsp( pInMsg, pCBs->pfnReqTunnelRsp );
      break;

    case COMMAND_SE_TUNNELING_SERVER_TRANSFER_DATA:
      status = zclSE_TunnelingHdlTransferData( pInMsg, pCBs->pfnServerTransferData );
      break;

    case COMMAND_SE_TUNNELING_SERVER_TRANSFER_DATA_ERR:
      status = zclSE_TunnelingHdlTransferDataErr( pInMsg, pCBs->pfnServerTransferDataErr );
      break;

    case COMMAND_SE_TUNNELING_SERVER_ACK_TRANSFER_DATA:
      status = zclSE_TunnelingHdlAckTransferData( pInMsg, pCBs->pfnServerAckTransferData );
      break;

    case COMMAND_SE_TUNNELING_SERVER_READY_DATA:
      status = zclSE_TunnelingHdlReadyData( pInMsg, pCBs->pfnServerReadyData );
      break;

    case COMMAND_SE_TUNNELING_SUPP_TUNNEL_PROTOCOLS_RSP:
      status = zclSE_TunnelingHdlSuppTunnelProtocolsRsp( pInMsg, pCBs->pfnSuppTunnelProtocolsRsp );
      break;

    case COMMAND_SE_TUNNELING_TUNNEL_CLOSURE_NOTIF:
      status = zclSE_TunnelingHdlTunnelClosureNotif( pInMsg, pCBs->pfnTunnelClosureNotif );
      break;

    default:
      status = ZCL_STATUS_FAILURE;
      break;
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_TunnelingHdlServerCmd
 *
 * @brief   Server command handler for ZCL_CLUSTER_ID_SE_TUNNELING.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCBs - server callbacks
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_TunnelingHdlServerCmd( zclIncoming_t *pInMsg,
                                       const zclSE_TunnelingServerCBs_t *pCBs )
{
  ZStatus_t status;

  // Guard against NULL pointer
  if ( pCBs == NULL )
  {
    return ZCL_STATUS_FAILURE;
  }

  // Process Client-to-Server Commands
  switch ( pInMsg->hdr.commandID )
  {
    case COMMAND_SE_TUNNELING_REQ_TUNNEL:
      status = zclSE_TunnelingHdlReqTunnel( pInMsg, pCBs->pfnReqTunnel );
      break;

    case COMMAND_SE_TUNNELING_CLOSE_TUNNEL:
      status = zclSE_TunnelingHdlCloseTunnel( pInMsg, pCBs->pfnCloseTunnel );
      break;

    case COMMAND_SE_TUNNELING_CLIENT_TRANSFER_DATA:
      status = zclSE_TunnelingHdlTransferData( pInMsg, pCBs->pfnClientTransferData );
      break;

    case COMMAND_SE_TUNNELING_CLIENT_TRANSFER_DATA_ERR:
      status = zclSE_TunnelingHdlTransferDataErr( pInMsg, pCBs->pfnClientTransferDataErr );
      break;

    case COMMAND_SE_TUNNELING_CLIENT_ACK_TRANSFER_DATA:
      status = zclSE_TunnelingHdlAckTransferData( pInMsg, pCBs->pfnClientAckTransferData );
      break;

    case COMMAND_SE_TUNNELING_CLIENT_READY_DATA:
      status = zclSE_TunnelingHdlReadyData( pInMsg, pCBs->pfnClientReadyData );
      break;

    case COMMAND_SE_TUNNELING_GET_SUPP_TUNNEL_PROTOCOLS:
      status = zclSE_TunnelingHdlGetSuppTunnelProtocols( pInMsg, pCBs->pfnGetSuppTunnelProtocols );
      break;

    default:
      status = ZCL_STATUS_FAILURE;
      break;
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PrepaymentSendPublishPrepaySnapshot
 *
 * @brief   Send COMMAND_SE_PREPAYMENT_PUBLISH_PREPAY_SNAPSHOT.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PrepaymentSendPublishPrepaySnapshot(
            uint8_t srcEP, afAddrType_t *dstAddr,
            zclSE_PrepaymentPublishPrepaySnapshot_t *pCmd,
            uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t *pCmdBuf;
  uint16_t cmdBufLen;
  uint8_t *pBuf;

  // Check for a non fragmented, valid "payload" field, then calculate length
  if ( ( pCmd->cmdTotal <= 1 ) && ( pCmd->payload.pStatus != NULL ) )
  {
    if ( pCmd->type == ZCL_SE_PREPAYMENT_SNAPSHOT_DEBT_CREDIT_STATUS )
    {
      cmdBufLen = ZCL_SE_PREPAYMENT_DEBT_CREDIT_STATUS_LEN;
    }
    else
    {
      cmdBufLen = 0;
    }
  }
  else if ( pCmd->pRawPayload != NULL )
  {
    // Fragmented -- use raw payload fields
    cmdBufLen = pCmd->rawPayloadLen;
  }

  // Allocate command buffer
  cmdBufLen = ZCL_SE_PREPAYMENT_PUBLISH_PREPAY_SNAPSHOT_LEN + cmdBufLen;

  pCmdBuf = OsalPort_malloc( cmdBufLen );
  if ( pCmdBuf == NULL )
  {
    return ZMemError;
  }

  pBuf = pCmdBuf;
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->snapshotID );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->time );
  *pBuf++ = pCmd->totalFound;
  *pBuf++ = pCmd->cmdIdx;
  *pBuf++ = pCmd->cmdTotal;
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->cause );
  *pBuf++ = pCmd->type;

  // Check for a non fragmented, valid "payload" field, then serialize
  if ( ( pCmd->cmdTotal <= 1 ) && ( pCmd->payload.pStatus != NULL ) )
  {
    if ( pCmd->type == ZCL_SE_PREPAYMENT_SNAPSHOT_DEBT_CREDIT_STATUS )
    {
      pBuf = OsalPort_bufferUint32( pBuf, pCmd->payload.pStatus->accumDebt );
      pBuf = OsalPort_bufferUint32( pBuf, pCmd->payload.pStatus->type1DebtRemaining );
      pBuf = OsalPort_bufferUint32( pBuf, pCmd->payload.pStatus->type2DebtRemaining );
      pBuf = OsalPort_bufferUint32( pBuf, pCmd->payload.pStatus->type3DebtRemaining );
      pBuf = OsalPort_bufferUint32( pBuf, pCmd->payload.pStatus->emrgncyCreditRemaining );
      pBuf = OsalPort_bufferUint32( pBuf, pCmd->payload.pStatus->creditRemaining );
    }
  }
  else if ( pCmd->pRawPayload )
  {
    // Fragmented -- use raw payload fields
    pBuf = OsalPort_memcpy( pBuf, pCmd->pRawPayload, pCmd->rawPayloadLen );
  }

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_PREPAYMENT,
                            COMMAND_SE_PREPAYMENT_PUBLISH_PREPAY_SNAPSHOT, TRUE,
                            ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0,
                            seqNum, cmdBufLen, pCmdBuf );

  OsalPort_free( pCmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PrepaymentSendChangePaymentModeRsp
 *
 * @brief   Send COMMAND_SE_PREPAYMENT_CHANGE_PAYMENT_MODE_RSP.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PrepaymentSendChangePaymentModeRsp(
            uint8_t srcEP, afAddrType_t *dstAddr,
            zclSE_PrepaymentChangePaymentModeRsp_t *pCmd,
            uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t cmdBuf[ZCL_SE_PREPAYMENT_CHANGE_PAYMENT_MODE_RSP_LEN];
  uint8_t *pBuf = cmdBuf;

  *pBuf++ = pCmd->frndlyCredit;
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->frndlyCreditCalendarID );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->emrgncyCreditLimit );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->emrgncyCreditThreshold );

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_PREPAYMENT,
                            COMMAND_SE_PREPAYMENT_CHANGE_PAYMENT_MODE_RSP, TRUE,
                            ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0,
                            seqNum, ZCL_SE_PREPAYMENT_CHANGE_PAYMENT_MODE_RSP_LEN, cmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PrepaymentSendConsumerTopUpRsp
 *
 * @brief   Send COMMAND_SE_PREPAYMENT_CONSUMER_TOP_UP_RSP.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PrepaymentSendConsumerTopUpRsp(
            uint8_t srcEP, afAddrType_t *dstAddr,
            zclSE_PrepaymentConsumerTopUpRsp_t *pCmd,
            uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t cmdBuf[ZCL_SE_PREPAYMENT_CONSUMER_TOP_UP_RSP_LEN];
  uint8_t *pBuf = cmdBuf;

  *pBuf++ = pCmd->resultType;
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->topupValue );
  *pBuf++ = pCmd->srcOfTopUp;
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->creditRemaining );

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_PREPAYMENT,
                            COMMAND_SE_PREPAYMENT_CONSUMER_TOP_UP_RSP, TRUE,
                            ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0,
                            seqNum, ZCL_SE_PREPAYMENT_CONSUMER_TOP_UP_RSP_LEN, cmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PrepaymentSendPublishTopUpLog
 *
 * @brief   Send COMMAND_SE_PREPAYMENT_PUBLISH_TOP_UP_LOG.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PrepaymentSendPublishTopUpLog(
            uint8_t srcEP, afAddrType_t *dstAddr,
            zclSE_PrepaymentPublishTopUpLog_t *pCmd,
            uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t *pCmdBuf;
  uint16_t cmdBufLen = 0;
  uint8_t *pBuf;
  uint8_t topUp;

  // Check for a non fragmented, valid "pTopUps" field, then calculate length
  if ( ( pCmd->cmdTotal <= 1 ) && ( pCmd->pTopUps != NULL ) )
  {
    for ( topUp = 0; topUp < pCmd->numOfTopUps; topUp++ )
    {
      cmdBufLen += ( ZCL_SE_PREPAYMENT_TOP_UP_LEN +
                     zclSE_UTF8StringLen( &pCmd->pTopUps[topUp].code ) );
    }
  }
  else if ( pCmd->pRawPayload != NULL )
  {
    // Fragmented -- use raw payload fields
    cmdBufLen = pCmd->rawPayloadLen;
  }

  // Allocate command buffer
  cmdBufLen = ZCL_SE_PREPAYMENT_PUBLISH_TOP_UP_LOG_LEN + cmdBufLen;

  pCmdBuf = OsalPort_malloc( cmdBufLen );
  if ( pCmdBuf == NULL )
  {
    return ZMemError;
  }

  pBuf = pCmdBuf;
  *pBuf++ = pCmd->cmdIdx;
  *pBuf++ = pCmd->cmdTotal;

  // Check for a non fragmented, valid "pTopUps" field, then serialize
  if ( ( pCmd->cmdTotal <= 1 ) && ( pCmd->pTopUps != NULL ) )
  {
    for ( topUp = 0; topUp < pCmd->numOfTopUps; topUp++ )
    {
      pBuf = zclSE_UTF8StringBuild( pBuf, &pCmd->pTopUps[topUp].code );
      pBuf = OsalPort_bufferUint32( pBuf, pCmd->pTopUps[topUp].amnt );
      pBuf = OsalPort_bufferUint32( pBuf, pCmd->pTopUps[topUp].time );
    }
  }
  else if ( pCmd->pRawPayload )
  {
    // Fragmented -- use raw payload fields
    pBuf = OsalPort_memcpy( pBuf, pCmd->pRawPayload, pCmd->rawPayloadLen );
  }

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_PREPAYMENT,
                            COMMAND_SE_PREPAYMENT_PUBLISH_TOP_UP_LOG, TRUE,
                            ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0,
                            seqNum, cmdBufLen, pCmdBuf );

  OsalPort_free( pCmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PrepaymentSendPublishDebtLog
 *
 * @brief   Send COMMAND_SE_PREPAYMENT_PUBLISH_DEBT_LOG.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PrepaymentSendPublishDebtLog(
            uint8_t srcEP, afAddrType_t *dstAddr,
            zclSE_PrepaymentPublishDebtLog_t *pCmd,
            uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t *pCmdBuf;
  uint16_t cmdBufLen = 0;
  uint8_t *pBuf;

  // Check for a non fragmented, valid "pRecs" field, then calculate length
  if ( ( pCmd->cmdTotal <= 1 ) && ( pCmd->pRecs != NULL ) )
  {
    cmdBufLen = pCmd->numOfRecs * ZCL_SE_PREPAYMENT_DEBT_REC_LEN;
  }
  else if ( pCmd->pRawPayload != NULL )
  {
    // Fragmented -- use raw payload fields
    cmdBufLen = pCmd->rawPayloadLen;
  }

  // Allocate command buffer
  cmdBufLen = ZCL_SE_PREPAYMENT_PUBLISH_DEBT_LOG_LEN + cmdBufLen;

  pCmdBuf = OsalPort_malloc( cmdBufLen );
  if ( pCmdBuf == NULL )
  {
    return ZMemError;
  }

  pBuf = pCmdBuf;
  *pBuf++ = pCmd->cmdIdx;
  *pBuf++ = pCmd->cmdTotal;

  // Check for a non fragmented, valid "pRecs" field, then serialize
  if ( ( pCmd->cmdTotal <= 1 ) && ( pCmd->pRecs != NULL ) )
  {
    uint8_t rec;

    for ( rec = 0; rec < pCmd->numOfRecs; rec++ )
    {
      pBuf = OsalPort_bufferUint32( pBuf, pCmd->pRecs[rec].collTime );
      pBuf = OsalPort_bufferUint32( pBuf, pCmd->pRecs[rec].amntColl );
      *pBuf++ = pCmd->pRecs[rec].debtType;
      pBuf = OsalPort_bufferUint32( pBuf, pCmd->pRecs[rec].outstandingDebt );
    }
  }
  else if ( pCmd->pRawPayload )
  {
    // Fragmented -- use raw payload fields
    pBuf = OsalPort_memcpy( pBuf, pCmd->pRawPayload, pCmd->rawPayloadLen );
  }

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_PREPAYMENT,
                            COMMAND_SE_PREPAYMENT_PUBLISH_DEBT_LOG, TRUE,
                            ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0,
                            seqNum, cmdBufLen, pCmdBuf );

  OsalPort_free( pCmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PrepaymentSendSelAvailEmrgncyCredit
 *
 * @brief   Send COMMAND_SE_PREPAYMENT_SEL_AVAIL_EMRGNCY_CREDIT.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PrepaymentSendSelAvailEmrgncyCredit(
            uint8_t srcEP, afAddrType_t *dstAddr,
            zclSE_PrepaymentSelAvailEmrgncyCredit_t *pCmd,
            uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t cmdBuf[ZCL_SE_PREPAYMENT_SEL_AVAIL_EMRGNCY_CREDIT_LEN];
  uint8_t *pBuf = cmdBuf;

  pBuf = OsalPort_bufferUint32( pBuf, pCmd->cmdIssueDateTime );
  *pBuf++ = pCmd->origDevice;

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_PREPAYMENT,
                            COMMAND_SE_PREPAYMENT_SEL_AVAIL_EMRGNCY_CREDIT, TRUE,
                            ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0,
                            seqNum, ZCL_SE_PREPAYMENT_SEL_AVAIL_EMRGNCY_CREDIT_LEN, cmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PrepaymentSendChangeDebt
 *
 * @brief   Send COMMAND_SE_PREPAYMENT_CHANGE_DEBT.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PrepaymentSendChangeDebt( uint8_t srcEP, afAddrType_t *dstAddr,
                                          zclSE_PrepaymentChangeDebt_t *pCmd,
                                          uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t *pCmdBuf;
  uint16_t cmdBufLen;
  uint8_t *pBuf;

  // Allocate command buffer
  cmdBufLen = ZCL_SE_PREPAYMENT_CHANGE_DEBT_LEN +
              zclSE_UTF8StringLen( &pCmd->debtLabel );

  pCmdBuf = OsalPort_malloc( cmdBufLen );
  if ( pCmdBuf == NULL )
  {
    return ZMemError;
  }

  pBuf = pCmdBuf;
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->issuerEvtID );
  pBuf = zclSE_UTF8StringBuild( pBuf, &pCmd->debtLabel );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->debtAmnt );
  *pBuf++ = pCmd->debtRecoveryMethod;
  *pBuf++ = pCmd->debtAmntType;
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->debtRecoveryStartTime );
  *pBuf++ = LO_UINT16( pCmd->debtRecoveryCollTime );
  *pBuf++ = HI_UINT16( pCmd->debtRecoveryCollTime );
  *pBuf++ = pCmd->debtRecoveryFreq;
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->debtRecoveryAmnt );
  *pBuf++ = LO_UINT16( pCmd->debtRecoveryBalPct );
  *pBuf++ = HI_UINT16( pCmd->debtRecoveryBalPct );

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_PREPAYMENT,
                            COMMAND_SE_PREPAYMENT_CHANGE_DEBT, TRUE,
                            ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0,
                            seqNum, cmdBufLen, pCmdBuf );

  OsalPort_free( pCmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PrepaymentSendEmrgncyCreditSetup
 *
 * @brief   Send COMMAND_SE_PREPAYMENT_EMRGNCY_CREDIT_SETUP.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PrepaymentSendEmrgncyCreditSetup(
            uint8_t srcEP, afAddrType_t *dstAddr,
            zclSE_PrepaymentEmrgncyCreditSetup_t *pCmd,
            uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t cmdBuf[ZCL_SE_PREPAYMENT_EMRGNCY_CREDIT_SETUP_LEN];
  uint8_t *pBuf = cmdBuf;

  pBuf = OsalPort_bufferUint32( pBuf, pCmd->issuerEvtID );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->startTime );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->emrgncyCreditLimit );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->emrgncyCreditThreshold );

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_PREPAYMENT,
                            COMMAND_SE_PREPAYMENT_EMRGNCY_CREDIT_SETUP, TRUE,
                            ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0,
                            seqNum, ZCL_SE_PREPAYMENT_EMRGNCY_CREDIT_SETUP_LEN, cmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PrepaymentSendConsumerTopUp
 *
 * @brief   Send COMMAND_SE_PREPAYMENT_CONSUMER_TOP_UP.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PrepaymentSendConsumerTopUp(
            uint8_t srcEP, afAddrType_t *dstAddr,
            zclSE_PrepaymentConsumerTopUp_t *pCmd,
            uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t *pCmdBuf;
  uint16_t cmdBufLen;
  uint8_t *pBuf;

  // Allocate command buffer
  cmdBufLen = ZCL_SE_PREPAYMENT_CONSUMER_TOP_UP_LEN +
              zclSE_UTF8StringLen( &pCmd->topUpCode );

  pCmdBuf = OsalPort_malloc( cmdBufLen );
  if ( pCmdBuf == NULL )
  {
    return ZMemError;
  }

  pBuf = pCmdBuf;
  *pBuf++ = pCmd->origDevice;
  pBuf = zclSE_UTF8StringBuild( pBuf, &pCmd->topUpCode );

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_PREPAYMENT,
                            COMMAND_SE_PREPAYMENT_CONSUMER_TOP_UP, TRUE,
                            ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0,
                            seqNum, cmdBufLen, pCmdBuf );

  OsalPort_free( pCmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PrepaymentSendCreditAdj
 *
 * @brief   Send COMMAND_SE_PREPAYMENT_CREDIT_ADJ.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PrepaymentSendCreditAdj(
            uint8_t srcEP, afAddrType_t *dstAddr,
            zclSE_PrepaymentCreditAdj_t *pCmd,
            uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t cmdBuf[ZCL_SE_PREPAYMENT_CREDIT_ADJ_LEN];
  uint8_t *pBuf = cmdBuf;

  pBuf = OsalPort_bufferUint32( pBuf, pCmd->issuerEvtID );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->startTime );
  *pBuf++ = pCmd->creditAdjType;
  OsalPort_bufferUint32( pBuf, pCmd->creditAdjValue );

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_PREPAYMENT,
                            COMMAND_SE_PREPAYMENT_CREDIT_ADJ, TRUE,
                            ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0,
                            seqNum, ZCL_SE_PREPAYMENT_CREDIT_ADJ_LEN, cmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PrepaymentSendChangePaymentMode
 *
 * @brief   Send COMMAND_SE_PREPAYMENT_CHANGE_PAYMENT_MODE.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PrepaymentSendChangePaymentMode(
            uint8_t srcEP, afAddrType_t *dstAddr,
            zclSE_PrepaymentChangePaymentMode_t *pCmd,
            uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t cmdBuf[ZCL_SE_PREPAYMENT_CHANGE_PAYMENT_MODE_LEN];
  uint8_t *pBuf = cmdBuf;

  pBuf = OsalPort_bufferUint32( pBuf, pCmd->providerID );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->issuerEvtID );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->implDateTime );
  *pBuf++ = LO_UINT16( pCmd->propPaymentCtrlCfg );
  *pBuf++ = HI_UINT16( pCmd->propPaymentCtrlCfg );
  OsalPort_bufferUint32( pBuf, pCmd->cutOffValue );

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_PREPAYMENT,
                            COMMAND_SE_PREPAYMENT_CHANGE_PAYMENT_MODE, TRUE,
                            ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0,
                            seqNum, ZCL_SE_PREPAYMENT_CHANGE_PAYMENT_MODE_LEN, cmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PrepaymentSendGetPrepaySnapshot
 *
 * @brief   Send COMMAND_SE_PREPAYMENT_GET_PREPAY_SNAPSHOT.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PrepaymentSendGetPrepaySnapshot(
            uint8_t srcEP, afAddrType_t *dstAddr,
            zclSE_PrepaymentGetPrepaySnapshot_t *pCmd,
            uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t cmdBuf[ZCL_SE_PREPAYMENT_GET_PREPAY_SNAPSHOT_LEN];
  uint8_t *pBuf = cmdBuf;

  pBuf = OsalPort_bufferUint32( pBuf, pCmd->earliestStartTime );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->latestEndTime );
  *pBuf++ = pCmd->offset;
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->cause );

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_PREPAYMENT,
                            COMMAND_SE_PREPAYMENT_GET_PREPAY_SNAPSHOT, TRUE,
                            ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0,
                            seqNum, ZCL_SE_PREPAYMENT_GET_PREPAY_SNAPSHOT_LEN, cmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PrepaymentSendGetTopUpLog
 *
 * @brief   Send COMMAND_SE_PREPAYMENT_GET_TOP_UP_LOG.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PrepaymentSendGetTopUpLog( uint8_t srcEP, afAddrType_t *dstAddr,
                                           zclSE_PrepaymentGetTopUpLog_t *pCmd,
                                           uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t cmdBuf[ZCL_SE_PREPAYMENT_GET_TOP_UP_LOG_LEN];
  uint8_t *pBuf = cmdBuf;

  pBuf = OsalPort_bufferUint32( pBuf, pCmd->latestEndTime );
  *pBuf++ = pCmd->numOfRecords;

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_PREPAYMENT,
                            COMMAND_SE_PREPAYMENT_GET_TOP_UP_LOG, TRUE,
                            ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0,
                            seqNum, ZCL_SE_PREPAYMENT_GET_TOP_UP_LOG_LEN, cmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PrepaymentSendSetLowCreditWarnLevel
 *
 * @brief   Send COMMAND_SE_PREPAYMENT_SET_LOW_CREDIT_WARN_LEVEL.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PrepaymentSendSetLowCreditWarnLevel(
            uint8_t srcEP, afAddrType_t *dstAddr,
            zclSE_PrepaymentSetLowCreditWarnLevel_t *pCmd,
            uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t cmdBuf[ZCL_SE_PREPAYMENT_SET_LOW_CREDIT_WARN_LEVEL_LEN];
  uint8_t *pBuf = cmdBuf;

  pBuf = OsalPort_bufferUint32( pBuf, pCmd->level );

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_PREPAYMENT,
                            COMMAND_SE_PREPAYMENT_SET_LOW_CREDIT_WARN_LEVEL, TRUE,
                            ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0,
                            seqNum, ZCL_SE_PREPAYMENT_SET_LOW_CREDIT_WARN_LEVEL_LEN, cmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PrepaymentSendGetDebtRepayLog
 *
 * @brief   Send COMMAND_SE_PREPAYMENT_GET_DEBT_REPAY_LOG.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PrepaymentSendGetDebtRepayLog(
            uint8_t srcEP, afAddrType_t *dstAddr,
            zclSE_PrepaymentGetDebtRepayLog_t *pCmd,
            uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t cmdBuf[ZCL_SE_PREPAYMENT_GET_DEBT_REPAY_LOG_LEN];
  uint8_t *pBuf = cmdBuf;

  pBuf = OsalPort_bufferUint32( pBuf, pCmd->latestEndTime );
  *pBuf++ = pCmd->numOfDebts;
  *pBuf++ = pCmd->debtType;


  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_PREPAYMENT,
                            COMMAND_SE_PREPAYMENT_GET_DEBT_REPAY_LOG, TRUE,
                            ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0,
                            seqNum, ZCL_SE_PREPAYMENT_GET_DEBT_REPAY_LOG_LEN, cmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PrepaymentSendSetMaxCreditLimit
 *
 * @brief   Send COMMAND_SE_PREPAYMENT_SET_MAX_CREDIT_LIMIT.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PrepaymentSendSetMaxCreditLimit(
            uint8_t srcEP, afAddrType_t *dstAddr,
            zclSE_PrepaymentSetMaxCreditLimit_t *pCmd,
            uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t cmdBuf[ZCL_SE_PREPAYMENT_SET_MAX_CREDIT_LIMIT_LEN];
  uint8_t *pBuf = cmdBuf;

  pBuf = OsalPort_bufferUint32( pBuf, pCmd->providerID );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->issuerEvtID );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->implDateTime );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->maxCreditLevel );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->maxCreditPerTopUp );

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_PREPAYMENT,
                            COMMAND_SE_PREPAYMENT_SET_MAX_CREDIT_LIMIT, TRUE,
                            ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0,
                            seqNum, ZCL_SE_PREPAYMENT_SET_MAX_CREDIT_LIMIT_LEN, cmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PrepaymentSendSetOverallDebtCap
 *
 * @brief   Send COMMAND_SE_PREPAYMENT_SET_OVERALL_DEBT_CAP.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PrepaymentSendSetOverallDebtCap(
            uint8_t srcEP, afAddrType_t *dstAddr,
            zclSE_PrepaymentSetOverallDebtCap_t *pCmd,
            uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t cmdBuf[ZCL_SE_PREPAYMENT_SET_OVERALL_DEBT_CAP_LEN];
  uint8_t *pBuf = cmdBuf;

  pBuf = OsalPort_bufferUint32( pBuf, pCmd->providerID );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->issuerEvtID );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->implDateTime );
  OsalPort_bufferUint32( pBuf, pCmd->overallDebtCap );

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_PREPAYMENT,
                            COMMAND_SE_PREPAYMENT_SET_OVERALL_DEBT_CAP, TRUE,
                            ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0,
                            seqNum, ZCL_SE_PREPAYMENT_SET_OVERALL_DEBT_CAP_LEN, cmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PrepaymentParsePublishPrepaySnapshot
 *
 * @brief   Parse COMMAND_SE_PREPAYMENT_PUBLISH_PREPAY_SNAPSHOT.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PrepaymentParsePublishPrepaySnapshot(
            zclIncoming_t *pInMsg,
            zclSE_PrepaymentPublishPrepaySnapshot_t *pCmd )
{
  ZStatus_t status = ZCL_STATUS_SUCCESS;
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_PREPAYMENT_PUBLISH_PREPAY_SNAPSHOT_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->snapshotID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->time = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->totalFound = *pBuf++;
  pCmd->cmdIdx = *pBuf++;
  pCmd->cmdTotal = *pBuf++;
  pCmd->cause = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->type = *pBuf++;
  pCmd->rawPayloadLen = pInMsg->pDataLen - ZCL_SE_PREPAYMENT_PUBLISH_PREPAY_SNAPSHOT_LEN;

  if ( pCmd->rawPayloadLen )
  {
    pCmd->pRawPayload = pBuf;
  }
  else
  {
    pCmd->pRawPayload = NULL;
  }

  // Check for fragmentation
  if ( pCmd->cmdTotal <= 1 )
  {
    status = zclSE_PrepaymentSnapshotPayloadParse( pCmd );
  }
  else
  {
    pCmd->payload.pStatus = NULL;
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PrepaymentParseChangePaymentModeRsp
 *
 * @brief   Parse COMMAND_SE_PREPAYMENT_CHANGE_PAYMENT_MODE_RSP.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PrepaymentParseChangePaymentModeRsp(
            zclIncoming_t *pInMsg,
            zclSE_PrepaymentChangePaymentModeRsp_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_PREPAYMENT_CHANGE_PAYMENT_MODE_RSP_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->frndlyCredit = *pBuf++;
  pCmd->frndlyCreditCalendarID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->emrgncyCreditLimit = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->emrgncyCreditThreshold = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_PrepaymentParseConsumerTopUpRsp
 *
 * @brief   Parse COMMAND_SE_PREPAYMENT_CONSUMER_TOP_UP_RSP.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PrepaymentParseConsumerTopUpRsp(
            zclIncoming_t *pInMsg,
            zclSE_PrepaymentConsumerTopUpRsp_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_PREPAYMENT_CONSUMER_TOP_UP_RSP_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->resultType = *pBuf++;
  pCmd->topupValue = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->srcOfTopUp = *pBuf++;
  pCmd->creditRemaining = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_PrepaymentParsePublishTopUpLog
 *
 * @brief   Parse COMMAND_SE_PREPAYMENT_PUBLISH_TOP_UP_LOG.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PrepaymentParsePublishTopUpLog(
            zclIncoming_t *pInMsg,
            zclSE_PrepaymentPublishTopUpLog_t *pCmd )
{
  ZStatus_t status = ZCL_STATUS_SUCCESS;
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_PREPAYMENT_PUBLISH_TOP_UP_LOG_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  // Parse the command buffer
  pCmd->cmdIdx = *pBuf++;
  pCmd->cmdTotal = *pBuf++;
  pCmd->numOfTopUps = 0;
  pCmd->rawPayloadLen = pInMsg->pDataLen - ZCL_SE_PREPAYMENT_PUBLISH_TOP_UP_LOG_LEN;

  if ( pCmd->rawPayloadLen )
  {
    pCmd->pRawPayload = pBuf;
  }
  else
  {
    pCmd->pRawPayload = NULL;
  }

  // Check for fragmentation
  if ( pCmd->cmdTotal <= 1 )
  {
    status = zclSE_PrepaymentTopUpParse( pCmd );
  }
  else
  {
    pCmd->pTopUps = NULL;
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PrepaymentParsePublishDebtLog
 *
 * @brief   Parse COMMAND_SE_PREPAYMENT_PUBLISH_DEBT_LOG.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PrepaymentParsePublishDebtLog(
            zclIncoming_t *pInMsg,
            zclSE_PrepaymentPublishDebtLog_t *pCmd )
{
  ZStatus_t status = ZCL_STATUS_SUCCESS;
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_PREPAYMENT_PUBLISH_DEBT_LOG_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->cmdIdx = *pBuf++;
  pCmd->cmdTotal = *pBuf++;
  pCmd->numOfRecs = 0;
  pCmd->rawPayloadLen = pInMsg->pDataLen - ZCL_SE_PREPAYMENT_PUBLISH_DEBT_LOG_LEN;

  if ( pCmd->rawPayloadLen )
  {
    pCmd->pRawPayload = pBuf;
  }
  else
  {
    pCmd->pRawPayload = NULL;
  }

  // Check for fragmentation
  if ( pCmd->cmdTotal <= 1 )
  {
    status = zclSE_PrepaymentDebtRecParse( pCmd );
  }
  else
  {
    pCmd->pRecs = NULL;
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PrepaymentParseSelAvailEmrgncyCredit
 *
 * @brief   Parse COMMAND_SE_PREPAYMENT_SEL_AVAIL_EMRGNCY_CREDIT.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PrepaymentParseSelAvailEmrgncyCredit(
            zclIncoming_t *pInMsg,
            zclSE_PrepaymentSelAvailEmrgncyCredit_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_PREPAYMENT_SEL_AVAIL_EMRGNCY_CREDIT_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->cmdIssueDateTime = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->origDevice = *pBuf++;

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_PrepaymentParseChangeDebt
 *
 * @brief   Parse COMMAND_SE_PREPAYMENT_CHANGE_DEBT.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PrepaymentParseChangeDebt( zclIncoming_t *pInMsg,
                                           zclSE_PrepaymentChangeDebt_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;
  uint8_t *pBufEnd = pInMsg->pData + pInMsg->pDataLen;
  uint8_t exceeded;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_PREPAYMENT_CHANGE_DEBT_SEG1_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->issuerEvtID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pBuf = zclSE_UTF8StringParse( pBuf, pBufEnd, &pCmd->debtLabel, &exceeded );

  // Check for string length exceeded flag and rest of packet
  if ( exceeded ||
       ( pInMsg->pDataLen < ( pCmd->debtLabel.strLen +
                              ZCL_SE_PREPAYMENT_CHANGE_DEBT_LEN ) ) )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->debtAmnt = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->debtRecoveryMethod = *pBuf++;
  pCmd->debtAmntType = *pBuf++;
  pCmd->debtRecoveryStartTime = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->debtRecoveryCollTime = BUILD_UINT16( pBuf[0], pBuf[1] );
  pBuf += 2;
  pCmd->debtRecoveryFreq = *pBuf++;
  pCmd->debtRecoveryAmnt = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->debtRecoveryBalPct = BUILD_UINT16( pBuf[0], pBuf[1] );
  pBuf += 2;

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_PrepaymentParseEmrgncyCreditSetup
 *
 * @brief   Parse COMMAND_SE_PREPAYMENT_EMRGNCY_CREDIT_SETUP.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PrepaymentParseEmrgncyCreditSetup(
            zclIncoming_t *pInMsg,
            zclSE_PrepaymentEmrgncyCreditSetup_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_PREPAYMENT_EMRGNCY_CREDIT_SETUP_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->issuerEvtID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->startTime = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->emrgncyCreditLimit = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->emrgncyCreditThreshold = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_PrepaymentParseConsumerTopUp
 *
 * @brief   Parse COMMAND_SE_PREPAYMENT_CONSUMER_TOP_UP.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PrepaymentParseConsumerTopUp(
            zclIncoming_t *pInMsg,
            zclSE_PrepaymentConsumerTopUp_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;
  uint8_t *pBufEnd = pInMsg->pData + pInMsg->pDataLen;
  uint8_t exceeded;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_PREPAYMENT_CONSUMER_TOP_UP_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->origDevice = *pBuf++;
  pBuf = zclSE_UTF8StringParse( pBuf, pBufEnd, &pCmd->topUpCode, &exceeded );

  // Check for string length exceeded flag
  if ( exceeded )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_PrepaymentParseCreditAdj
 *
 * @brief   Parse COMMAND_SE_PREPAYMENT_CREDIT_ADJ.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PrepaymentParseCreditAdj(
            zclIncoming_t *pInMsg,
            zclSE_PrepaymentCreditAdj_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_PREPAYMENT_CREDIT_ADJ_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->issuerEvtID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->startTime = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->creditAdjType = *pBuf++;
  pCmd->creditAdjValue = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_PrepaymentParseChangePaymentMode
 *
 * @brief   Parse COMMAND_SE_PREPAYMENT_CHANGE_PAYMENT_MODE.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PrepaymentParseChangePaymentMode(
            zclIncoming_t *pInMsg,
            zclSE_PrepaymentChangePaymentMode_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_PREPAYMENT_CHANGE_PAYMENT_MODE_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->providerID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->issuerEvtID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->implDateTime = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->propPaymentCtrlCfg = BUILD_UINT16( pBuf[0], pBuf[1] );
  pBuf += 2;
  pCmd->cutOffValue = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_PrepaymentParseGetPrepaySnapshot
 *
 * @brief   Parse COMMAND_SE_PREPAYMENT_GET_PREPAY_SNAPSHOT.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PrepaymentParseGetPrepaySnapshot(
            zclIncoming_t *pInMsg,
            zclSE_PrepaymentGetPrepaySnapshot_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_PREPAYMENT_GET_PREPAY_SNAPSHOT_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->earliestStartTime = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->latestEndTime = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->offset = *pBuf++;
  pCmd->cause = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_PrepaymentParseGetTopUpLog
 *
 * @brief   Parse COMMAND_SE_PREPAYMENT_GET_TOP_UP_LOG.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PrepaymentParseGetTopUpLog( zclIncoming_t *pInMsg,
                                            zclSE_PrepaymentGetTopUpLog_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_PREPAYMENT_GET_TOP_UP_LOG_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->latestEndTime = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->numOfRecords = *pBuf++;

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_PrepaymentParseSetLowCreditWarnLevel
 *
 * @brief   Parse COMMAND_SE_PREPAYMENT_SET_LOW_CREDIT_WARN_LEVEL.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PrepaymentParseSetLowCreditWarnLevel(
            zclIncoming_t *pInMsg,
            zclSE_PrepaymentSetLowCreditWarnLevel_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_PREPAYMENT_SET_LOW_CREDIT_WARN_LEVEL_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->level = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_PrepaymentParseGetDebtRepayLog
 *
 * @brief   Parse COMMAND_SE_PREPAYMENT_GET_DEBT_REPAY_LOG.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PrepaymentParseGetDebtRepayLog(
            zclIncoming_t *pInMsg,
            zclSE_PrepaymentGetDebtRepayLog_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_PREPAYMENT_GET_DEBT_REPAY_LOG_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->latestEndTime = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->numOfDebts = *pBuf++;
  pCmd->debtType = *pBuf++;

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_PrepaymentParseSetMaxCreditLimit
 *
 * @brief   Parse COMMAND_SE_PREPAYMENT_SET_MAX_CREDIT_LIMIT.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PrepaymentParseSetMaxCreditLimit(
            zclIncoming_t *pInMsg,
            zclSE_PrepaymentSetMaxCreditLimit_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_PREPAYMENT_SET_MAX_CREDIT_LIMIT_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->providerID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->issuerEvtID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->implDateTime = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->maxCreditLevel = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->maxCreditPerTopUp = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_PrepaymentParseSetOverallDebtCap
 *
 * @brief   Parse COMMAND_SE_PREPAYMENT_SET_OVERALL_DEBT_CAP.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PrepaymentParseSetOverallDebtCap(
            zclIncoming_t *pInMsg,
            zclSE_PrepaymentSetOverallDebtCap_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_PREPAYMENT_SET_OVERALL_DEBT_CAP_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->providerID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->issuerEvtID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->implDateTime = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->overallDebtCap = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_PrepaymentHdlPublishPrepaySnapshot
 *
 * @brief   Handle COMMAND_SE_PREPAYMENT_PUBLISH_PREPAY_SNAPSHOT.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PrepaymentHdlPublishPrepaySnapshot(
            zclIncoming_t *pInMsg,
            zclSE_PrepaymentPublishPrepaySnapshotCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_PrepaymentPublishPrepaySnapshot_t *pCmd;

    pCmd = OsalPort_malloc( sizeof( zclSE_PrepaymentPublishPrepaySnapshot_t ) );
    if ( pCmd == NULL )
    {
       return ZCL_STATUS_SOFTWARE_FAILURE;
    }

    status = zclSE_PrepaymentParsePublishPrepaySnapshot( pInMsg, pCmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      status = pfnCB( pInMsg, pCmd );

      if ( pCmd->payload.pStatus != NULL )
      {
        OsalPort_free( pCmd->payload.pStatus );
      }
    }

    OsalPort_free( pCmd );
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PrepaymentHdlChangePaymentModeRsp
 *
 * @brief   Handle COMMAND_SE_PREPAYMENT_CHANGE_PAYMENT_MODE_RSP.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PrepaymentHdlChangePaymentModeRsp(
            zclIncoming_t *pInMsg,
            zclSE_PrepaymentChangePaymentModeRspCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_PrepaymentChangePaymentModeRsp_t cmd;

    status = zclSE_PrepaymentParseChangePaymentModeRsp( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      pfnCB( pInMsg, &cmd );
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PrepaymentHdlConsumerTopUpRsp
 *
 * @brief   Handle COMMAND_SE_PREPAYMENT_CONSUMER_TOP_UP_RSP.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PrepaymentHdlConsumerTopUpRsp(
            zclIncoming_t *pInMsg,
            zclSE_PrepaymentConsumerTopUpRspCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_PrepaymentConsumerTopUpRsp_t cmd;

    status = zclSE_PrepaymentParseConsumerTopUpRsp( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      pfnCB( pInMsg, &cmd );
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PrepaymentHdlPublishTopUpLog
 *
 * @brief   Handle COMMAND_SE_PREPAYMENT_PUBLISH_TOP_UP_LOG.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PrepaymentHdlPublishTopUpLog(
            zclIncoming_t *pInMsg,
            zclSE_PrepaymentPublishTopUpLogCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_PrepaymentPublishTopUpLog_t cmd;

    status = zclSE_PrepaymentParsePublishTopUpLog( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      status = pfnCB( pInMsg, &cmd );

      if ( cmd.pTopUps != NULL )
      {
        OsalPort_free( cmd.pTopUps );
      }
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PrepaymentHdlPublishDebtLog
 *
 * @brief   Handle COMMAND_SE_PREPAYMENT_PUBLISH_DEBT_LOG.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PrepaymentHdlPublishDebtLog(
            zclIncoming_t *pInMsg,
            zclSE_PrepaymentPublishDebtLogCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_PrepaymentPublishDebtLog_t cmd;

    status = zclSE_PrepaymentParsePublishDebtLog( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      status = pfnCB( pInMsg, &cmd );

      if ( cmd.pRecs != NULL )
      {
        OsalPort_free( cmd.pRecs );
      }
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PrepaymentHdlSelAvailEmrgncyCredit
 *
 * @brief   Handle COMMAND_SE_PREPAYMENT_SEL_AVAIL_EMRGNCY_CREDIT.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PrepaymentHdlSelAvailEmrgncyCredit(
            zclIncoming_t *pInMsg,
            zclSE_PrepaymentSelAvailEmrgncyCreditCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_PrepaymentSelAvailEmrgncyCredit_t cmd;

    status = zclSE_PrepaymentParseSelAvailEmrgncyCredit( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      status = pfnCB( pInMsg, &cmd );
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PrepaymentHdlChangeDebt
 *
 * @brief   Handle COMMAND_SE_PREPAYMENT_CHANGE_DEBT.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PrepaymentHdlChangeDebt( zclIncoming_t *pInMsg,
                                         zclSE_PrepaymentChangeDebtCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_PrepaymentChangeDebt_t *pCmd;

    pCmd = OsalPort_malloc( sizeof( zclSE_PrepaymentChangeDebt_t ) );
    if ( pCmd == NULL )
    {
       return ZCL_STATUS_SOFTWARE_FAILURE;
    }

    status = zclSE_PrepaymentParseChangeDebt( pInMsg, pCmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      pfnCB( pInMsg, pCmd );
    }

    OsalPort_free( pCmd );
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PrepaymentHdlEmrgncyCreditSetup
 *
 * @brief   Handle COMMAND_SE_PREPAYMENT_EMRGNCY_CREDIT_SETUP.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PrepaymentHdlEmrgncyCreditSetup(
            zclIncoming_t *pInMsg,
            zclSE_PrepaymentEmrgncyCreditSetupCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_PrepaymentEmrgncyCreditSetup_t cmd;

    status = zclSE_PrepaymentParseEmrgncyCreditSetup( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      pfnCB( pInMsg, &cmd );
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PrepaymentHdlConsumerTopUp
 *
 * @brief   Handle COMMAND_SE_PREPAYMENT_CONSUMER_TOP_UP.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PrepaymentHdlConsumerTopUp( zclIncoming_t *pInMsg,
                                            zclSE_PrepaymentConsumerTopUpCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_PrepaymentConsumerTopUp_t cmd;

    status = zclSE_PrepaymentParseConsumerTopUp( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      status = pfnCB( pInMsg, &cmd );
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PrepaymentHdlCreditAdj
 *
 * @brief   Handle COMMAND_SE_PREPAYMENT_CREDIT_ADJ.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PrepaymentHdlCreditAdj(
            zclIncoming_t *pInMsg,
            zclSE_PrepaymentCreditAdjCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_PrepaymentCreditAdj_t cmd;

    status = zclSE_PrepaymentParseCreditAdj( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      pfnCB( pInMsg, &cmd );
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PrepaymentHdlChangePaymentMode
 *
 * @brief   Handle COMMAND_SE_PREPAYMENT_CHANGE_PAYMENT_MODE.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PrepaymentHdlChangePaymentMode(
            zclIncoming_t *pInMsg,
            zclSE_PrepaymentChangePaymentModeCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_PrepaymentChangePaymentMode_t cmd;

    status = zclSE_PrepaymentParseChangePaymentMode( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      pfnCB( pInMsg, &cmd );

      status = ZCL_STATUS_CMD_HAS_RSP;
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PrepaymentHdlGetPrepaySnapshot
 *
 * @brief   Handle COMMAND_SE_PREPAYMENT_GET_PREPAY_SNAPSHOT.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PrepaymentHdlGetPrepaySnapshot(
            zclIncoming_t *pInMsg,
            zclSE_PrepaymentGetPrepaySnapshotCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_PrepaymentGetPrepaySnapshot_t cmd;

    status = zclSE_PrepaymentParseGetPrepaySnapshot( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      status = pfnCB( pInMsg, &cmd );
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PrepaymentHdlGetTopUpLog
 *
 * @brief   Handle COMMAND_SE_PREPAYMENT_GET_TOP_UP_LOG.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PrepaymentHdlGetTopUpLog( zclIncoming_t *pInMsg,
                                          zclSE_PrepaymentGetTopUpLogCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_PrepaymentGetTopUpLog_t cmd;

    status = zclSE_PrepaymentParseGetTopUpLog( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      status = pfnCB( pInMsg, &cmd );
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PrepaymentHdlSetLowCreditWarnLevel
 *
 * @brief   Handle COMMAND_SE_PREPAYMENT_SET_LOW_CREDIT_WARN_LEVEL.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PrepaymentHdlSetLowCreditWarnLevel(
            zclIncoming_t *pInMsg,
            zclSE_PrepaymentSetLowCreditWarnLevelCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_PrepaymentSetLowCreditWarnLevel_t cmd;

    status = zclSE_PrepaymentParseSetLowCreditWarnLevel( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      pfnCB( pInMsg, &cmd );
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PrepaymentHdlGetDebtRepayLog
 *
 *  @brief   Handle COMMAND_SE_PREPAYMENT_GET_DEBT_REPAY_LOG.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PrepaymentHdlGetDebtRepayLog(
            zclIncoming_t *pInMsg,
            zclSE_PrepaymentGetDebtRepayLogCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_PrepaymentGetDebtRepayLog_t cmd;

    status = zclSE_PrepaymentParseGetDebtRepayLog( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      status = pfnCB( pInMsg, &cmd );
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PrepaymentHdlSetMaxCreditLimit
 *
 * @brief   Handle COMMAND_SE_PREPAYMENT_SET_MAX_CREDIT_LIMIT.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PrepaymentHdlSetMaxCreditLimit(
            zclIncoming_t *pInMsg,
            zclSE_PrepaymentSetMaxCreditLimitCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_PrepaymentSetMaxCreditLimit_t cmd;

    status = zclSE_PrepaymentParseSetMaxCreditLimit( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      pfnCB( pInMsg, &cmd );
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PrepaymentHdlSetOverallDebtCap
 *
 * @brief   Handle COMMAND_SE_PREPAYMENT_SET_OVERALL_DEBT_CAP.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PrepaymentHdlSetOverallDebtCap(
            zclIncoming_t *pInMsg,
            zclSE_PrepaymentSetOverallDebtCapCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_PrepaymentSetOverallDebtCap_t cmd;

    status = zclSE_PrepaymentParseSetOverallDebtCap( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      pfnCB( pInMsg, &cmd );
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PrepaymentHdlClientCmd
 *
 * @brief   Client command handler for ZCL_CLUSTER_ID_SE_PREPAYMENT.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCBs - client callbacks
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PrepaymentHdlClientCmd( zclIncoming_t *pInMsg,
                                        const zclSE_PrepaymentClientCBs_t *pCBs )
{
  ZStatus_t status;

  // Guard against NULL pointer
  if ( pCBs == NULL )
  {
    return ZCL_STATUS_FAILURE;
  }

  // Process Server-to-Client Commands
  switch ( pInMsg->hdr.commandID )
  {
    case COMMAND_SE_PREPAYMENT_PUBLISH_PREPAY_SNAPSHOT:
      status = zclSE_PrepaymentHdlPublishPrepaySnapshot( pInMsg, pCBs->pfnPublishPrepaySnapshot );
      break;

    case COMMAND_SE_PREPAYMENT_CHANGE_PAYMENT_MODE_RSP:
      status = zclSE_PrepaymentHdlChangePaymentModeRsp( pInMsg, pCBs->pfnChangePaymentModeRsp );
      break;

    case COMMAND_SE_PREPAYMENT_CONSUMER_TOP_UP_RSP:
      status = zclSE_PrepaymentHdlConsumerTopUpRsp( pInMsg, pCBs->pfnConsumerTopUpRsp );
      break;

    case COMMAND_SE_PREPAYMENT_PUBLISH_TOP_UP_LOG:
      status = zclSE_PrepaymentHdlPublishTopUpLog( pInMsg, pCBs->pfnPublishTopUpLog );
      break;

    case COMMAND_SE_PREPAYMENT_PUBLISH_DEBT_LOG:
      status = zclSE_PrepaymentHdlPublishDebtLog( pInMsg, pCBs->pfnPublishDebtLog );
      break;

    default:
      status = ZCL_STATUS_FAILURE;
      break;
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PrepaymentHdlServerCmd
 *
 * @brief   Server command handler for ZCL_CLUSTER_ID_SE_PREPAYMENT.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCBs - server callbacks
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PrepaymentHdlServerCmd( zclIncoming_t *pInMsg,
                                        const zclSE_PrepaymentServerCBs_t *pCBs )
{
  ZStatus_t status;

  // Guard against NULL pointer
  if ( pCBs == NULL )
  {
    return ZCL_STATUS_FAILURE;
  }

  // Process Client-to-Server Commands
  switch ( pInMsg->hdr.commandID )
  {
    case COMMAND_SE_PREPAYMENT_SEL_AVAIL_EMRGNCY_CREDIT:
      status = zclSE_PrepaymentHdlSelAvailEmrgncyCredit( pInMsg, pCBs->pfnSelAvailEmrgncyCredit );
      break;

    case COMMAND_SE_PREPAYMENT_CHANGE_DEBT:
      status = zclSE_PrepaymentHdlChangeDebt( pInMsg, pCBs->pfnChangeDebt );
      break;

    case COMMAND_SE_PREPAYMENT_EMRGNCY_CREDIT_SETUP:
      status = zclSE_PrepaymentHdlEmrgncyCreditSetup( pInMsg, pCBs->pfnEmrgncyCreditSetup );
      break;

    case COMMAND_SE_PREPAYMENT_CONSUMER_TOP_UP:
      status = zclSE_PrepaymentHdlConsumerTopUp( pInMsg, pCBs->pfnConsumerTopUp );
      break;

    case COMMAND_SE_PREPAYMENT_CREDIT_ADJ:
      status = zclSE_PrepaymentHdlCreditAdj( pInMsg, pCBs->pfnCreditAdj );
      break;

    case COMMAND_SE_PREPAYMENT_CHANGE_PAYMENT_MODE:
      status = zclSE_PrepaymentHdlChangePaymentMode( pInMsg, pCBs->pfnChangePaymentMode );
      break;

    case COMMAND_SE_PREPAYMENT_GET_PREPAY_SNAPSHOT:
      status = zclSE_PrepaymentHdlGetPrepaySnapshot( pInMsg, pCBs->pfnGetPrepaySnapshot );
      break;

    case COMMAND_SE_PREPAYMENT_GET_TOP_UP_LOG:
      status = zclSE_PrepaymentHdlGetTopUpLog( pInMsg, pCBs->pfnGetTopUpLog );
      break;

    case COMMAND_SE_PREPAYMENT_SET_LOW_CREDIT_WARN_LEVEL:
      status = zclSE_PrepaymentHdlSetLowCreditWarnLevel( pInMsg, pCBs->pfnSetLowCreditWarnLevel );
      break;

    case COMMAND_SE_PREPAYMENT_GET_DEBT_REPAY_LOG:
      status = zclSE_PrepaymentHdlGetDebtRepayLog( pInMsg, pCBs->pfnGetDebtRepayLog );
      break;

    case COMMAND_SE_PREPAYMENT_SET_MAX_CREDIT_LIMIT:
      status = zclSE_PrepaymentHdlSetMaxCreditLimit( pInMsg, pCBs->pfnSetMaxCreditLimit );
      break;

    case COMMAND_SE_PREPAYMENT_SET_OVERALL_DEBT_CAP:
      status = zclSE_PrepaymentHdlSetOverallDebtCap( pInMsg, pCBs->pfnSetOverallDebtCap );
      break;

    default:
      status = ZCL_STATUS_FAILURE;
      break;
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_PrepaymentSnapshotPayloadParse
 *
 * @brief   Utility function for parsing "payload" payload -- see
 *          "zclSE_PrepaymentPublishPrepaySnapshot_t"
 *
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PrepaymentSnapshotPayloadParse(
            zclSE_PrepaymentPublishPrepaySnapshot_t *pCmd )
{
  // Initialize in case of failure
  pCmd->payload.pStatus = NULL;

  // Check for valid raw fields
  if ( pCmd->rawPayloadLen && ( pCmd->pRawPayload == NULL ) )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  if ( pCmd->type == ZCL_SE_PREPAYMENT_SNAPSHOT_DEBT_CREDIT_STATUS )
  {
    uint8_t *pBuf = pCmd->pRawPayload;

    // Check for minimum payload length
    if ( pCmd->rawPayloadLen < ZCL_SE_PREPAYMENT_DEBT_CREDIT_STATUS_LEN )
    {
      return ZCL_STATUS_MALFORMED_COMMAND;
    }

    // Allocate memory for the status data
    pCmd->payload.pStatus = OsalPort_malloc( sizeof( zclSE_PrepaymentDebtCreditStatus_t ) );
    if ( pCmd->payload.pStatus == NULL )
    {
      return ZCL_STATUS_SOFTWARE_FAILURE;
    }

    pCmd->payload.pStatus->accumDebt = OsalPort_buildUint32( pBuf, 4 );
    pBuf += 4;
    pCmd->payload.pStatus->type1DebtRemaining = OsalPort_buildUint32( pBuf, 4 );
    pBuf += 4;
    pCmd->payload.pStatus->type2DebtRemaining = OsalPort_buildUint32( pBuf, 4 );
    pBuf += 4;
    pCmd->payload.pStatus->type3DebtRemaining = OsalPort_buildUint32( pBuf, 4 );
    pBuf += 4;
    pCmd->payload.pStatus->emrgncyCreditRemaining = OsalPort_buildUint32( pBuf, 4 );
    pBuf += 4;
    pCmd->payload.pStatus->creditRemaining = OsalPort_buildUint32( pBuf, 4 );
    pBuf += 4;
  }

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_PrepaymentTopUpParse
 *
 * @brief   Utility function for parsing "pTopUps" payload -- see
 *          "zclSE_PrepaymentPublishTopUpLog_t"
 *
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PrepaymentTopUpParse( zclSE_PrepaymentPublishTopUpLog_t *pCmd )
{
  uint8_t *pBuf;
  uint8_t *pBufEnd;
  uint8_t topUpTotal;
  UTF8String_t utf8;
  uint8_t exceeded;
  uint8_t topUp;

  // Initialize in case of failure
  pCmd->pTopUps = NULL;

  // Check for valid raw fields
  if ( pCmd->rawPayloadLen && ( pCmd->pRawPayload == NULL ) )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  // Initialize buffer pointers
  pBuf = pCmd->pRawPayload;
  pBufEnd = pCmd->pRawPayload + pCmd->rawPayloadLen;

  // Calculate number of top ups
  topUpTotal = 0;

  while ( ( pBufEnd - pBuf ) >= ZCL_SE_PREPAYMENT_TOP_UP_LEN )
  {
    // Add to top up total
    topUpTotal++;

    pBuf = zclSE_UTF8StringParse( pBuf, pBufEnd, &utf8, &exceeded );

    // Check for string length exceeded flag AND check for the rest of the top up fields. Subtract
    // one from top up length since string length has already been processed.
    if ( exceeded || ( ( pBufEnd - pBuf ) < ( ZCL_SE_PREPAYMENT_TOP_UP_LEN - 1 ) ) )
    {
      return ZCL_STATUS_MALFORMED_COMMAND;
    }

    // Advance to next top up field
    pBuf += ( ZCL_SE_PREPAYMENT_TOP_UP_LEN - 1 );
  }

  if ( topUpTotal )
  {
    // Allocate memory for the top up data
    pCmd->pTopUps = OsalPort_malloc( topUpTotal * sizeof( zclSE_PrepaymentTopUp_t ) );

    if ( pCmd->pTopUps == NULL )
    {
      return ZCL_STATUS_SOFTWARE_FAILURE;
    }

    // Save total number
    pCmd->numOfTopUps = topUpTotal;

    // Reset buffer pointer
    pBuf = pCmd->pRawPayload;

    for ( topUp = 0; topUp < topUpTotal; topUp++ )
    {
      pBuf = zclSE_UTF8StringParse( pBuf, pBufEnd, &pCmd->pTopUps[topUp].code, &exceeded );
      pCmd->pTopUps[topUp].amnt = OsalPort_buildUint32( pBuf, 4 );
      pBuf += 4;
      pCmd->pTopUps[topUp].time = OsalPort_buildUint32( pBuf, 4 );
      pBuf += 4;
    }
  }

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_PrepaymentDebtRecParse
 *
 * @brief   Utility function for parsing "pRecs" payload -- see "zclSE_PrepaymentPublishDebtLog_t"
 *
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_PrepaymentDebtRecParse( zclSE_PrepaymentPublishDebtLog_t *pCmd )
{
  // Initialize in case of failure
  pCmd->pRecs = NULL;

  // Check for valid raw fields
  if ( pCmd->rawPayloadLen && ( pCmd->pRawPayload == NULL ) )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  // Get the number of records
  pCmd->numOfRecs = pCmd->rawPayloadLen / ZCL_SE_PREPAYMENT_DEBT_REC_LEN;

  if ( pCmd->numOfRecs )
  {
    uint8_t *pBuf = pCmd->pRawPayload;
    uint8_t rec;

    // Allocate memory for the record data
    pCmd->pRecs = OsalPort_malloc( pCmd->numOfRecs *
                                  sizeof( zclSE_PrepaymentDebtRec_t ) );
    if ( pCmd->pRecs == NULL )
    {
      return ZCL_STATUS_SOFTWARE_FAILURE;
    }

    for ( rec = 0; rec < pCmd->numOfRecs; rec++ )
    {
      pCmd->pRecs[rec].collTime = OsalPort_buildUint32( pBuf, 4 );
      pBuf += 4;
      pCmd->pRecs[rec].amntColl = OsalPort_buildUint32( pBuf, 4 );
      pBuf += 4;
      pCmd->pRecs[rec].debtType = *pBuf++;
      pCmd->pRecs[rec].outstandingDebt = OsalPort_buildUint32( pBuf, 4 );
      pBuf += 4;
    }
  }

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_CalendarSendPublishCalendar
 *
 * @brief   Send COMMAND_SE_CALENDAR_PUBLISH_CALENDAR.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_CalendarSendPublishCalendar(
            uint8_t srcEP, afAddrType_t *dstAddr,
            zclSE_CalendarPublishCalendar_t *pCmd,
            uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t *pCmdBuf;
  uint16_t cmdBufLen;
  uint8_t *pBuf;

  // Allocate command buffer
  cmdBufLen = ZCL_SE_CALENDAR_PUBLISH_CALENDAR_LEN +
              zclSE_UTF8StringLen( &pCmd->calendarName );

  pCmdBuf = OsalPort_malloc( cmdBufLen );
  if ( pCmdBuf == NULL )
  {
    return ZMemError;
  }

  pBuf = pCmdBuf;
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->providerID );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->issuerEvtID );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->issuerCalendarID );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->startTime );
  *pBuf++ = pCmd->calendarType;
  *pBuf++ = pCmd->calendarTimeRef;
  pBuf = zclSE_UTF8StringBuild( pBuf, &pCmd->calendarName );
  *pBuf++ = pCmd->numOfSeasons;
  *pBuf++ = pCmd->numOfWeekProfiles;
  *pBuf = pCmd->numOfDayProfiles;

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_CALENDAR,
                            COMMAND_SE_CALENDAR_PUBLISH_CALENDAR, TRUE,
                            ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0,
                            seqNum, cmdBufLen, pCmdBuf );

  OsalPort_free( pCmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_CalendarSendPublishDayProfile
 *
 * @brief   Send COMMAND_SE_CALENDAR_PUBLISH_DAY_PROFILE.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_CalendarSendPublishDayProfile(
            uint8_t srcEP, afAddrType_t *dstAddr,
            zclSE_CalendarPublishDayProfile_t *pCmd,
            uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t *pCmdBuf;
  uint16_t cmdBufLen = 0;
  uint8_t *pBuf;

  // Check for a non fragmented, valid "entries" field, then calculate length
  if ( ( pCmd->cmdTotal <= 1 ) && ( pCmd->entries.pRate != NULL ) )
  {
    // All the entry types have same length;
    cmdBufLen = pCmd->numOfEntries * ZCL_SE_CALENDAR_DAY_ENTRY_LEN;
  }
  else if ( pCmd->pRawPayload != NULL )
  {
    // Fragmented -- use raw payload fields
    cmdBufLen = pCmd->rawPayloadLen;
  }

  // Allocate command buffer
  cmdBufLen = ZCL_SE_CALENDAR_PUBLISH_DAYPROFILE_LEN + cmdBufLen;

  pCmdBuf = OsalPort_malloc( cmdBufLen );
  if ( pCmdBuf == NULL )
  {
    return ZMemError;
  }

  pBuf = pCmdBuf;
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->profileID );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->issuerEvtID );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->issuerCalendarID );
  *pBuf++ = pCmd->dayID;
  *pBuf++ = pCmd->numOfEntries;
  *pBuf++ = pCmd->cmdIdx;
  *pBuf++ = pCmd->cmdTotal;
  *pBuf++ = pCmd->calendarType;

  // Check for a non fragmented, valid "entries" field, then serialize
  if ( ( pCmd->cmdTotal <= 1 ) && ( pCmd->entries.pRate != NULL ) )
  {
    uint8_t entry;

    // Since each entry type has the same memory layout, just assume rate type
    for ( entry = 0; entry < pCmd->numOfEntries; entry++ )
    {
      *pBuf++ = LO_UINT16( pCmd->entries.pRate[entry].startTime );
      *pBuf++ = HI_UINT16( pCmd->entries.pRate[entry].startTime );
      *pBuf++ = pCmd->entries.pRate[entry].priceTier;
    }
  }
  else if ( pCmd->pRawPayload )
  {
    // Fragmented -- use raw payload fields
    pBuf = OsalPort_memcpy( pBuf, pCmd->pRawPayload, pCmd->rawPayloadLen );
  }

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_CALENDAR,
                            COMMAND_SE_CALENDAR_PUBLISH_DAY_PROFILE, TRUE,
                            ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0,
                            seqNum, cmdBufLen, pCmdBuf );

  OsalPort_free( pCmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_CalendarSendPublishWeekProfile
 *
 * @brief   Send COMMAND_SE_CALENDAR_PUBLISH_WEEK_PROFILE.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_CalendarSendPublishWeekProfile(
            uint8_t srcEP, afAddrType_t *dstAddr,
            zclSE_CalendarPublishWeekProfile_t *pCmd,
            uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t cmdBuf[ZCL_SE_CALENDAR_PUBLISH_WEEK_PROFILE_LEN];
  uint8_t *pBuf = cmdBuf;

  pBuf = OsalPort_bufferUint32( pBuf, pCmd->providerID );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->issuerEvtID );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->issuerCalendarID );
  *pBuf++ = pCmd->weekID;
  *pBuf++ = pCmd->dayIDRefMonday;
  *pBuf++ = pCmd->dayIDRefTuestday;
  *pBuf++ = pCmd->dayIDRefWednesday;
  *pBuf++ = pCmd->dayIDRefThursday;
  *pBuf++ = pCmd->dayIDRefFriday;
  *pBuf++ = pCmd->dayIDRefSaturday;
  *pBuf   = pCmd->dayIDRefSunday;

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_CALENDAR,
                            COMMAND_SE_CALENDAR_PUBLISH_WEEK_PROFILE, TRUE,
                            ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0,
                            seqNum, ZCL_SE_CALENDAR_PUBLISH_WEEK_PROFILE_LEN, cmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_CalendarSendPublishSeasons
 *
 * @brief   Send COMMAND_SE_CALENDAR_PUBLISH_SEASONS.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_CalendarSendPublishSeasons( uint8_t srcEP, afAddrType_t *dstAddr,
                                            zclSE_CalendarPublishSeasons_t *pCmd,
                                            uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t *pCmdBuf;
  uint16_t cmdBufLen = 0;
  uint8_t *pBuf;

  // Check for a non fragmented, valid "pSeasonEntries" field, then calculate length
  if ( ( pCmd->cmdTotal <= 1 ) && ( pCmd->pSeasonEntries != NULL ) )
  {
    cmdBufLen = pCmd->numSeasonEntries * ZCL_SE_CALENDAR_SEASON_ENTRY_LEN;
  }
  else if ( pCmd->pRawPayload != NULL )
  {
    // Fragmented -- use raw payload fields
    cmdBufLen = pCmd->rawPayloadLen;
  }

  // Allocate command buffer
  cmdBufLen = ZCL_SE_CALENDAR_PUBLISH_SEASONS_LEN + cmdBufLen;

  pCmdBuf = OsalPort_malloc( cmdBufLen );
  if ( pCmdBuf == NULL )
  {
    return ZMemError;
  }

  pBuf = pCmdBuf;
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->providerID );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->issuerEvtID );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->issuerCalendarID );
  *pBuf++ = pCmd->cmdIdx;
  *pBuf++ = pCmd->cmdTotal;

  // Check for a non fragmented, valid "pSeasonEntries" field, then serialize
  if ( ( pCmd->cmdTotal <= 1 ) && ( pCmd->pSeasonEntries != NULL ) )
  {
    uint8_t entry;

    for ( entry = 0; entry < pCmd->numSeasonEntries; entry++ )
    {
      pBuf = OsalPort_bufferUint32( pBuf, pCmd->pSeasonEntries[entry].seasonStartDate );
      *pBuf++ = pCmd->pSeasonEntries[entry].weekIDRef;
    }
  }
  else if ( pCmd->pRawPayload )
  {
    // Fragmented -- use raw payload fields
    pBuf = OsalPort_memcpy( pBuf, pCmd->pRawPayload, pCmd->rawPayloadLen );
  }

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_CALENDAR,
                            COMMAND_SE_CALENDAR_PUBLISH_SEASONS, TRUE,
                            ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0,
                            seqNum, cmdBufLen, pCmdBuf );

  OsalPort_free( pCmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_CalendarSendPublishSpecialDays
 *
 * @brief   Send COMMAND_SE_CALENDAR_PUBLISH_SPECIAL_DAYS.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_CalendarSendPublishSpecialDays(
            uint8_t srcEP, afAddrType_t *dstAddr,
            zclSE_CalendarPublishSpecialDays_t *pCmd,
            uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t *pCmdBuf;
  uint16_t cmdBufLen = 0;
  uint8_t *pBuf;

  // Check for a non fragmented, valid "pEntries" field, then calculate length
  if ( ( pCmd->cmdTotal <= 1 ) && ( pCmd->pEntries != NULL ) )
  {
    cmdBufLen = pCmd->numOfEntries * ZCL_SE_CALENDAR_SPECIAL_DAY_ENTRY_LEN;
  }
  else if ( pCmd->pRawPayload != NULL )
  {
    // Fragmented -- use raw payload fields
    cmdBufLen = pCmd->rawPayloadLen;
  }

  // Allocate command buffer
  cmdBufLen = ZCL_SE_CALENDAR_PUBLISH_SPECIAL_DAYS_LEN + cmdBufLen;

  pCmdBuf = OsalPort_malloc( cmdBufLen );
  if ( pCmdBuf == NULL )
  {
    return ZMemError;
  }

  pBuf = pCmdBuf;
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->providerID );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->issuerEvtID );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->issuerCalendarID );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->startTime );
  *pBuf++ = pCmd->calendarType;
  *pBuf++ = pCmd->numOfEntries;
  *pBuf++ = pCmd->cmdIdx;
  *pBuf++ = pCmd->cmdTotal;

  // Check for a non fragmented, valid "pEntries" field, then serialize
  if ( ( pCmd->cmdTotal <= 1 ) && ( pCmd->pEntries != NULL ) )
  {
    uint8_t entry;

    for ( entry = 0; entry < pCmd->numOfEntries; entry++ )
    {
      pBuf = OsalPort_bufferUint32( pBuf, pCmd->pEntries[entry].specialDayDate );
      *pBuf++ = pCmd->pEntries[entry].dayIDRef;
    }
  }
  else if ( pCmd->pRawPayload )
  {
    // Fragmented -- use raw payload fields
    pBuf = OsalPort_memcpy( pBuf, pCmd->pRawPayload, pCmd->rawPayloadLen );
  }

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_CALENDAR,
                            COMMAND_SE_CALENDAR_PUBLISH_SPECIAL_DAYS, TRUE,
                            ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0,
                            seqNum, cmdBufLen, pCmdBuf );

  OsalPort_free( pCmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_CalendarSendCancelCalendar
 *
 * @brief   Send COMMAND_SE_CALENDAR_CANCEL_CALENDAR.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_CalendarSendCancelCalendar( uint8_t srcEP, afAddrType_t *dstAddr,
                                            zclSE_CalendarCancelCalendar_t *pCmd,
                                            uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t cmdBuf[ZCL_SE_CALENDAR_CANCEL_CALENDAR_LEN];
  uint8_t *pBuf = cmdBuf;

  pBuf = OsalPort_bufferUint32( pBuf, pCmd->providerID );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->issuerCalendarID );
  *pBuf = pCmd->calendarType;

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_CALENDAR,
                            COMMAND_SE_CALENDAR_CANCEL_CALENDAR, TRUE,
                            ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0,
                            seqNum, ZCL_SE_CALENDAR_CANCEL_CALENDAR_LEN, cmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_CalendarSendGetCalendar
 *
 * @brief   Send COMMAND_SE_CALENDAR_GET_CALENDAR.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_CalendarSendGetCalendar( uint8_t srcEP, afAddrType_t *dstAddr,
                                         zclSE_CalendarGetCalendar_t *pCmd,
                                         uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t cmdBuf[ZCL_SE_CALENDAR_GET_CALENDAR_LEN];
  uint8_t *pBuf = cmdBuf;

  pBuf = OsalPort_bufferUint32( pBuf, pCmd->earliestStartTime );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->minIssuerEvtID );
  *pBuf++ = pCmd->numOfCalendars;
  *pBuf++ = pCmd->calendarType;
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->providerID );

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_CALENDAR,
                            COMMAND_SE_CALENDAR_GET_CALENDAR, TRUE,
                            ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0,
                            seqNum, ZCL_SE_CALENDAR_GET_CALENDAR_LEN, cmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_CalendarSendGetDayProfiles
 *
 * @brief   Send COMMAND_SE_CALENDAR_GET_DAY_PROFILES.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_CalendarSendGetDayProfiles( uint8_t srcEP, afAddrType_t *dstAddr,
                                            zclSE_CalendarGetDayProfiles_t *pCmd,
                                            uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t cmdBuf[ZCL_SE_CALENDAR_GET_DAY_PROFILES_LEN];
  uint8_t *pBuf = cmdBuf;

  pBuf = OsalPort_bufferUint32( pBuf, pCmd->providerID );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->issuerCalendarID );
  *pBuf++ = pCmd->startDayID;
  *pBuf++ = pCmd->numOfDays;

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_CALENDAR,
                            COMMAND_SE_CALENDAR_GET_DAY_PROFILES, TRUE,
                            ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0,
                            seqNum, ZCL_SE_CALENDAR_GET_DAY_PROFILES_LEN, cmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_CalendarSendGetWeekProfiles
 *
 * @brief   Send COMMAND_SE_CALENDAR_GET_WEEK_PROFILES.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_CalendarSendGetWeekProfiles(
            uint8_t srcEP, afAddrType_t *dstAddr,
            zclSE_CalendarGetWeekProfiles_t *pCmd,
            uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t cmdBuf[ZCL_SE_CALENDAR_GET_WEEK_PROFILES_LEN];
  uint8_t *pBuf = cmdBuf;

  pBuf = OsalPort_bufferUint32( pBuf, pCmd->providerID );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->issuerCalendarID );
  *pBuf++ = pCmd->startWeekID;
  *pBuf++ = pCmd->numOfWeeks;

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_CALENDAR,
                            COMMAND_SE_CALENDAR_GET_WEEK_PROFILES, TRUE,
                            ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0,
                            seqNum, ZCL_SE_CALENDAR_GET_WEEK_PROFILES_LEN, cmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_CalendarSendGetSeasons
 *
 * @brief   Send COMMAND_SE_CALENDAR_GET_SEASONS.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_CalendarSendGetSeasons( uint8_t srcEP, afAddrType_t *dstAddr,
                                        zclSE_CalendarGetSeasons_t *pCmd,
                                        uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t cmdBuf[ZCL_SE_CALENDAR_GET_SEASONS_LEN];
  uint8_t *pBuf = cmdBuf;

  pBuf = OsalPort_bufferUint32( pBuf, pCmd->providerID );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->issuerCalendarID );

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_CALENDAR,
                            COMMAND_SE_CALENDAR_GET_SEASONS, TRUE,
                            ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0,
                            seqNum, ZCL_SE_CALENDAR_GET_SEASONS_LEN, cmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_CalendarSendGetSpecialDays
 *
 * @brief   Send COMMAND_SE_CALENDAR_GET_SPECIAL_DAYS.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_CalendarSendGetSpecialDays( uint8_t srcEP, afAddrType_t *dstAddr,
                                            zclSE_CalendarGetSpecialDays_t *pCmd,
                                            uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t cmdBuf[ZCL_SE_CALENDAR_GET_SPECIAL_DAYS_LEN];
  uint8_t *pBuf = cmdBuf;

  pBuf = OsalPort_bufferUint32( pBuf, pCmd->startTime );
  *pBuf++ = pCmd->numOfEvents;
  *pBuf++ = pCmd->calendarType;
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->providerID );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->issuerCalendarID );

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_CALENDAR,
                            COMMAND_SE_CALENDAR_GET_SPECIAL_DAYS, TRUE,
                            ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0,
                            seqNum, ZCL_SE_CALENDAR_GET_SPECIAL_DAYS_LEN, cmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_CalendarParsePublishCalendar
 *
 * @brief   Parse COMMAND_SE_CALENDAR_PUBLISH_CALENDAR.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_CalendarParsePublishCalendar(
            zclIncoming_t *pInMsg,
            zclSE_CalendarPublishCalendar_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;
  uint8_t *pBufEnd = pInMsg->pData + pInMsg->pDataLen;
  uint8_t exceeded;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_CALENDAR_PUBLISH_CALENDAR_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  // Parse the command buffer
  pCmd->providerID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->issuerEvtID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->issuerCalendarID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->startTime = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->calendarType = *pBuf++;
  pCmd->calendarTimeRef = *pBuf++;
  pBuf = zclSE_UTF8StringParse( pBuf, pBufEnd, &pCmd->calendarName, &exceeded );

  // Check for string length exceeded flag
  if ( exceeded )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->numOfSeasons = *pBuf++;
  pCmd->numOfWeekProfiles = *pBuf++;
  pCmd->numOfDayProfiles = *pBuf;

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_CalendarParsePublishDayProfile
 *
 * @brief   Parse COMMAND_SE_CALENDAR_PUBLISH_DAY_PROFILE.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_CalendarParsePublishDayProfile(
            zclIncoming_t *pInMsg,
            zclSE_CalendarPublishDayProfile_t *pCmd )
{
  ZStatus_t status = ZCL_STATUS_SUCCESS;
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_CALENDAR_PUBLISH_DAYPROFILE_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->profileID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->issuerEvtID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->issuerCalendarID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->dayID = *pBuf++;
  pCmd->numOfEntries = *pBuf++;
  pCmd->cmdIdx = *pBuf++;
  pCmd->cmdTotal = *pBuf++;
  pCmd->calendarType = *pBuf++;
  pCmd->rawPayloadLen = pInMsg->pDataLen - ZCL_SE_CALENDAR_PUBLISH_DAYPROFILE_LEN;

  if ( pCmd->rawPayloadLen )
  {
    pCmd->pRawPayload = pBuf;
  }
  else
  {
    pCmd->pRawPayload = NULL;
  }

  // Check for fragmentation
  if ( pCmd->cmdTotal <= 1 )
  {
    status = zclSE_CalendarDayEntryParse( pCmd );
  }
  else
  {
    pCmd->entries.pRate = NULL;
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_CalendarParsePublishWeekProfile
 *
 * @brief   Parse COMMAND_SE_CALENDAR_PUBLISH_WEEK_PROFILE.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_CalendarParsePublishWeekProfile(
            zclIncoming_t *pInMsg,
            zclSE_CalendarPublishWeekProfile_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_CALENDAR_PUBLISH_WEEK_PROFILE_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  // Parse the command buffer
  pCmd->providerID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->issuerEvtID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->issuerCalendarID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->weekID = *pBuf++;
  pCmd->dayIDRefMonday    = *pBuf++;
  pCmd->dayIDRefTuestday  = *pBuf++;
  pCmd->dayIDRefWednesday = *pBuf++;
  pCmd->dayIDRefThursday  = *pBuf++;
  pCmd->dayIDRefFriday    = *pBuf++;
  pCmd->dayIDRefSaturday  = *pBuf++;
  pCmd->dayIDRefSunday    = *pBuf++;

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_CalendarParsePublishSeasons
 *
 * @brief   Parse COMMAND_SE_CALENDAR_PUBLISH_SEASONS.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_CalendarParsePublishSeasons(
            zclIncoming_t *pInMsg,
            zclSE_CalendarPublishSeasons_t *pCmd )
{
  ZStatus_t status = ZCL_STATUS_SUCCESS;
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_CALENDAR_PUBLISH_SEASONS_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->providerID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->issuerEvtID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->issuerCalendarID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->cmdIdx = *pBuf++;
  pCmd->cmdTotal = *pBuf++;
  pCmd->rawPayloadLen = pInMsg->pDataLen - ZCL_SE_CALENDAR_PUBLISH_SEASONS_LEN;

  if ( pCmd->rawPayloadLen )
  {
    pCmd->pRawPayload = pBuf;
  }
  else
  {
    pCmd->pRawPayload = NULL;
  }

  // Check for fragmentation
  if ( pCmd->cmdTotal <= 1 )
  {
    status = zclSE_CalendarSeasonEntryParse( pCmd );
  }
  else
  {
    pCmd->pSeasonEntries = NULL;
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_CalendarParsePublishSpecialDays
 *
 * @brief   Parse COMMAND_SE_CALENDAR_PUBLISH_SPECIAL_DAYS.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_CalendarParsePublishSpecialDays(
            zclIncoming_t *pInMsg,
            zclSE_CalendarPublishSpecialDays_t *pCmd )
{
  ZStatus_t status = ZCL_STATUS_SUCCESS;
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_CALENDAR_PUBLISH_SPECIAL_DAYS_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->providerID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->issuerEvtID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->issuerCalendarID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->startTime = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->calendarType = *pBuf++;
  pCmd->numOfEntries = *pBuf++;
  pCmd->cmdIdx = *pBuf++;
  pCmd->cmdTotal = *pBuf++;
  pCmd->rawPayloadLen = pInMsg->pDataLen - ZCL_SE_CALENDAR_PUBLISH_SPECIAL_DAYS_LEN;

  if ( pCmd->rawPayloadLen )
  {
    pCmd->pRawPayload = pBuf;
  }
  else
  {
    pCmd->pRawPayload = NULL;
  }

  // Check for fragmentation
  if ( pCmd->cmdTotal <= 1 )
  {
    status = zclSE_CalendarSpecialDayEntryParse( pCmd );
  }
  else
  {
    pCmd->pEntries = NULL;
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_CalendarParseCancelCalendar
 *
 * @brief   Parse COMMAND_SE_CALENDAR_CANCEL_CALENDAR.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_CalendarParseCancelCalendar(
            zclIncoming_t *pInMsg,
            zclSE_CalendarCancelCalendar_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_CALENDAR_CANCEL_CALENDAR_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->providerID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->issuerCalendarID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->calendarType = *pBuf++;

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_CalendarParseGetCalendar
 *
 * @brief   Parse COMMAND_SE_CALENDAR_GET_CALENDAR.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_CalendarParseGetCalendar( zclIncoming_t *pInMsg,
                                          zclSE_CalendarGetCalendar_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_CALENDAR_GET_CALENDAR_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->earliestStartTime = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->minIssuerEvtID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->numOfCalendars = *pBuf++;
  pCmd->calendarType   = *pBuf++;
  pCmd->providerID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_CalendarParseGetDayProfiles
 *
 * @brief   Parse COMMAND_SE_CALENDAR_GET_DAY_PROFILES.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_CalendarParseGetDayProfiles(
            zclIncoming_t *pInMsg,
            zclSE_CalendarGetDayProfiles_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_CALENDAR_GET_DAY_PROFILES_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->providerID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->issuerCalendarID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->startDayID = *pBuf++;
  pCmd->numOfDays = *pBuf++;

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_CalendarParseGetWeekProfiles
 *
 * @brief   Parse COMMAND_SE_CALENDAR_GET_WEEK_PROFILES.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_CalendarParseGetWeekProfiles(
            zclIncoming_t *pInMsg,
            zclSE_CalendarGetWeekProfiles_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_CALENDAR_GET_WEEK_PROFILES_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->providerID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->issuerCalendarID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->startWeekID = *pBuf++;
  pCmd->numOfWeeks = *pBuf++;

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_CalendarParseGetSeasons
 *
 * @brief   Parse COMMAND_SE_CALENDAR_GET_SEASONS.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_CalendarParseGetSeasons( zclIncoming_t *pInMsg,
                                         zclSE_CalendarGetSeasons_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_CALENDAR_GET_SEASONS_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->providerID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->issuerCalendarID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_CalendarParseGetSpecialDays
 *
 * @brief   Parse COMMAND_SE_CALENDAR_GET_SPECIAL_DAYS.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_CalendarParseGetSpecialDays(
            zclIncoming_t *pInMsg,
            zclSE_CalendarGetSpecialDays_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_CALENDAR_GET_SPECIAL_DAYS_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->startTime = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->numOfEvents  = *pBuf++;
  pCmd->calendarType = *pBuf++;
  pCmd->providerID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->issuerCalendarID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_CalendarHdlPublishCalendar
 *
 * @brief   Handle COMMAND_SE_CALENDAR_PUBLISH_CALENDAR.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_CalendarHdlPublishCalendar( zclIncoming_t *pInMsg,
                                            zclSE_CalendarPublishCalendarCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_CalendarPublishCalendar_t *pCmd;

    pCmd = OsalPort_malloc( sizeof( zclSE_CalendarPublishCalendar_t ) );
    if ( pCmd == NULL )
    {
       return ZCL_STATUS_SOFTWARE_FAILURE;
    }

    status = zclSE_CalendarParsePublishCalendar( pInMsg, pCmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      pfnCB( pInMsg, pCmd );
    }

    OsalPort_free( pCmd );
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_CalendarHdlPublishDayProfile
 *
 * @brief   Handle COMMAND_SE_CALENDAR_PUBLISH_DAY_PROFILE.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_CalendarHdlPublishDayProfile(
            zclIncoming_t *pInMsg,
            zclSE_CalendarPublishDayProfileCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_CalendarPublishDayProfile_t *pCmd;

    pCmd = OsalPort_malloc( sizeof( zclSE_CalendarPublishDayProfile_t ) );
    if ( pCmd == NULL )
    {
       return ZCL_STATUS_SOFTWARE_FAILURE;
    }

    status = zclSE_CalendarParsePublishDayProfile( pInMsg, pCmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      status = pfnCB( pInMsg, pCmd );

      if ( pCmd->entries.pRate != NULL )
      {
        OsalPort_free( pCmd->entries.pRate );
      }
    }

    OsalPort_free( pCmd );
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_CalendarHdlPublishWeekProfile
 *
 * @brief   Handle COMMAND_SE_CALENDAR_PUBLISH_WEEK_PROFILE.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_CalendarHdlPublishWeekProfile(
            zclIncoming_t *pInMsg,
            zclSE_CalendarPublishWeekProfileCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_CalendarPublishWeekProfile_t cmd;

    status = zclSE_CalendarParsePublishWeekProfile( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      status = pfnCB( pInMsg, &cmd );
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_CalendarHdlPublishSeasons
 *
 * @brief   Handle COMMAND_SE_CALENDAR_PUBLISH_SEASONS.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_CalendarHdlPublishSeasons( zclIncoming_t *pInMsg,
                                           zclSE_CalendarPublishSeasonsCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_CalendarPublishSeasons_t *pCmd;

    pCmd = OsalPort_malloc( sizeof( zclSE_CalendarPublishSeasons_t ) );
    if ( pCmd == NULL )
    {
       return ZCL_STATUS_SOFTWARE_FAILURE;
    }

    status = zclSE_CalendarParsePublishSeasons( pInMsg, pCmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      status = pfnCB( pInMsg, pCmd );

      if ( pCmd->pSeasonEntries != NULL )
      {
        OsalPort_free( pCmd->pSeasonEntries );
      }
    }

    OsalPort_free( pCmd );
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_CalendarHdlPublishSpecialDays
 *
 * @brief   Handle COMMAND_SE_CALENDAR_PUBLISH_SPECIAL_DAYS.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_CalendarHdlPublishSpecialDays(
            zclIncoming_t *pInMsg,
            zclSE_CalendarPublishSpecialDaysCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_CalendarPublishSpecialDays_t *pCmd;

    pCmd = OsalPort_malloc( sizeof( zclSE_CalendarPublishSpecialDays_t ) );
    if ( pCmd == NULL )
    {
       return ZCL_STATUS_SOFTWARE_FAILURE;
    }

    status = zclSE_CalendarParsePublishSpecialDays( pInMsg, pCmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      status = pfnCB( pInMsg, pCmd );

      if ( pCmd->pEntries != NULL )
      {
        OsalPort_free( pCmd->pEntries );
      }
    }

    OsalPort_free( pCmd );
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_CalendarHdlCancelCalendar
 *
 * @brief   Handle COMMAND_SE_CALENDAR_CANCEL_CALENDAR.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_CalendarHdlCancelCalendar( zclIncoming_t *pInMsg,
                                           zclSE_CalendarCancelCalendarCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_CalendarCancelCalendar_t cmd;

    status = zclSE_CalendarParseCancelCalendar( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      pfnCB( pInMsg, &cmd );
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_CalendarHdlGetCalendar
 *
 * @brief   Handle COMMAND_SE_CALENDAR_GET_CALENDAR.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_CalendarHdlGetCalendar( zclIncoming_t *pInMsg,
                                        zclSE_CalendarGetCalendarCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_CalendarGetCalendar_t cmd;

    status = zclSE_CalendarParseGetCalendar( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      pfnCB( pInMsg, &cmd );

      status = ZCL_STATUS_CMD_HAS_RSP;
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_CalendarHdlGetDayProfiles
 *
 * @brief   Handle COMMAND_SE_CALENDAR_GET_DAY_PROFILES.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_CalendarHdlGetDayProfiles( zclIncoming_t *pInMsg,
                                           zclSE_CalendarGetDayProfilesCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_CalendarGetDayProfiles_t cmd;

    status = zclSE_CalendarParseGetDayProfiles( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      status = pfnCB( pInMsg, &cmd );
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_CalendarHdlGetWeekProfiles
 *
 * @brief   Handle COMMAND_SE_CALENDAR_GET_WEEK_PROFILES.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_CalendarHdlGetWeekProfiles( zclIncoming_t *pInMsg,
                                            zclSE_CalendarGetWeekProfilesCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_CalendarGetWeekProfiles_t cmd;

    status = zclSE_CalendarParseGetWeekProfiles( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      status = pfnCB( pInMsg, &cmd );
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_CalendarHdlGetSeasons
 *
 * @brief   Handle COMMAND_SE_CALENDAR_GET_SEASONS.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_CalendarHdlGetSeasons( zclIncoming_t *pInMsg,
                                       zclSE_CalendarGetSeasonsCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_CalendarGetSeasons_t cmd;

    status = zclSE_CalendarParseGetSeasons( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      pfnCB( pInMsg, &cmd );

      status = ZCL_STATUS_CMD_HAS_RSP;
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_CalendarHdlGetSpecialDays
 *
 * @brief   Handle COMMAND_SE_CALENDAR_GET_SPECIAL_DAYS.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_CalendarHdlGetSpecialDays( zclIncoming_t *pInMsg,
                                           zclSE_CalendarGetSpecialDaysCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_CalendarGetSpecialDays_t cmd;

    status = zclSE_CalendarParseGetSpecialDays( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      pfnCB( pInMsg, &cmd );

      status = ZCL_STATUS_CMD_HAS_RSP;
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_CalendarHdlGetCalendarCancellation
 *
 * @brief   Handle COMMAND_SE_CALENDAR_GET_CALENDAR_CANCELLATION.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_CalendarHdlGetCalendarCancellation(
            zclIncoming_t *pInMsg,
            zclSE_CalendarGetCalendarCancellationCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    status = pfnCB( pInMsg );
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_CalendarHdlClientCmd
 *
 * @brief   Client command handler for ZCL_CLUSTER_ID_SE_CALENDAR.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCBs - client callbacks
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_CalendarHdlClientCmd( zclIncoming_t *pInMsg,
                                      const zclSE_CalendarClientCBs_t *pCBs )
{
  ZStatus_t status;

  // Guard against NULL pointer
  if ( pCBs == NULL )
  {
    return ZCL_STATUS_FAILURE;
  }

  // Process Server-to-Client Commands
  switch ( pInMsg->hdr.commandID )
  {
    case COMMAND_SE_CALENDAR_PUBLISH_CALENDAR:
      status = zclSE_CalendarHdlPublishCalendar( pInMsg, pCBs->pfnPublishCalendar );
      break;

    case COMMAND_SE_CALENDAR_PUBLISH_DAY_PROFILE:
      status = zclSE_CalendarHdlPublishDayProfile( pInMsg, pCBs->pfnPublishDayProfile );
      break;

    case COMMAND_SE_CALENDAR_PUBLISH_WEEK_PROFILE:
      status = zclSE_CalendarHdlPublishWeekProfile( pInMsg, pCBs->pfnPublishWeekProfile );
      break;

    case COMMAND_SE_CALENDAR_PUBLISH_SEASONS:
      status = zclSE_CalendarHdlPublishSeasons( pInMsg, pCBs->pfnPublishSeasons );
      break;

    case COMMAND_SE_CALENDAR_PUBLISH_SPECIAL_DAYS:
      status = zclSE_CalendarHdlPublishSpecialDays( pInMsg, pCBs->pfnPublishSpecialDays );
      break;

    case COMMAND_SE_CALENDAR_CANCEL_CALENDAR:
      status = zclSE_CalendarHdlCancelCalendar( pInMsg, pCBs->pfnCancelCalendar );
      break;

    default:
      status = ZCL_STATUS_FAILURE;
      break;
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_CalendarHdlServerCmd
 *
 * @brief   Server command handler for ZCL_CLUSTER_ID_SE_CALENDAR.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCBs - server callbacks
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_CalendarHdlServerCmd( zclIncoming_t *pInMsg,
                                      const zclSE_CalendarServerCBs_t *pCBs )
{
  ZStatus_t status;

  // Guard against NULL pointer
  if ( pCBs == NULL )
  {
    return ZCL_STATUS_FAILURE;
  }

  // Process Client-to-Server Commands
  switch ( pInMsg->hdr.commandID )
  {
    case COMMAND_SE_CALENDAR_GET_CALENDAR:
      status = zclSE_CalendarHdlGetCalendar( pInMsg, pCBs->pfnGetCalendar );
      break;

    case COMMAND_SE_CALENDAR_GET_DAY_PROFILES:
      status = zclSE_CalendarHdlGetDayProfiles( pInMsg, pCBs->pfnGetDayProfiles );
      break;

    case COMMAND_SE_CALENDAR_GET_WEEK_PROFILES:
      status = zclSE_CalendarHdlGetWeekProfiles( pInMsg, pCBs->pfnGetWeekProfiles );
      break;

    case COMMAND_SE_CALENDAR_GET_SEASONS:
      status = zclSE_CalendarHdlGetSeasons( pInMsg, pCBs->pfnGetSeasons );
      break;

    case COMMAND_SE_CALENDAR_GET_SPECIAL_DAYS:
      status = zclSE_CalendarHdlGetSpecialDays( pInMsg, pCBs->pfnGetSpecialDays );
      break;

    case COMMAND_SE_CALENDAR_GET_CALENDAR_CANCELLATION:
      status = zclSE_CalendarHdlGetCalendarCancellation( pInMsg,
                                                            pCBs->pfnGetCalendarCancellation );
      break;

    default:
      status = ZCL_STATUS_FAILURE;
      break;
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_CalendarDayEntryParse
 *
 * @brief   Utility function for parsing "entries" payload -- see
 *          "zclSE_CalendarPublishDayProfile_t"
 *
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_CalendarDayEntryParse( zclSE_CalendarPublishDayProfile_t *pCmd )
{
  uint8_t len;

  // Initialize in case of failure
  pCmd->entries.pRate = NULL;

  // Get expected length for entries
  len = ZCL_SE_CALENDAR_DAY_ENTRY_LEN * pCmd->numOfEntries;

  // Check for valid raw fields and minimum packet length
  if ( ( pCmd->rawPayloadLen && ( pCmd->pRawPayload == NULL ) ) ||
       ( pCmd->rawPayloadLen < len                            )    )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  if ( pCmd->numOfEntries )
  {
    uint8_t *pBuf = pCmd->pRawPayload;
    uint8_t entry;

    // Allocate memory for the entry data
    pCmd->entries.pRate = OsalPort_malloc( pCmd->numOfEntries *
                                          sizeof( zclSE_CalendarDayEntry_t ) );

    if ( pCmd->entries.pRate == NULL )
    {
      return ZCL_STATUS_SOFTWARE_FAILURE;
    }

    // Since each entry type has the same memory layout, just assume rate type
    for ( entry = 0; entry < pCmd->numOfEntries; entry++ )
    {
      pCmd->entries.pRate[entry].startTime = BUILD_UINT16( pBuf[0], pBuf[1] );
      pBuf += 2;
      pCmd->entries.pRate[entry].priceTier = *pBuf++;
    }
  }

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_CalendarSeasonEntryParse
 *
 * @brief   Utility function for parsing "pSeasonEntries" payload -- see
 *          "zclSE_CalendarPublishSeasons_t"
 *
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_CalendarSeasonEntryParse( zclSE_CalendarPublishSeasons_t *pCmd )
{
  // Initialize in case of failure
  pCmd->pSeasonEntries = NULL;

  // Check for valid raw fields
  if ( pCmd->rawPayloadLen && ( pCmd->pRawPayload == NULL ) )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  // Get the number of entries
  pCmd->numSeasonEntries = pCmd->rawPayloadLen / ZCL_SE_CALENDAR_SEASON_ENTRY_LEN;

  if ( pCmd->numSeasonEntries )
  {
    uint8_t *pBuf = pCmd->pRawPayload;
    uint8_t entry;

    // Allocate memory for the entry data
    pCmd->pSeasonEntries = OsalPort_malloc( pCmd->numSeasonEntries *
                                           sizeof( zclSE_CalendarSeasonEntry_t ) );

    if ( pCmd->pSeasonEntries == NULL )
    {
      return ZCL_STATUS_SOFTWARE_FAILURE;
    }

    for ( entry = 0; entry < pCmd->numSeasonEntries; entry++ )
    {
      pCmd->pSeasonEntries[entry].seasonStartDate = OsalPort_buildUint32( pBuf, 4 );
      pBuf += 4;
      pCmd->pSeasonEntries[entry].weekIDRef = *pBuf++;
    }
  }

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_CalendarSpecialDayEntryParse
 *
 * @brief   Utility function for parsing "pEntries" payload -- see
 *          "zclSE_CalendarPublishSpecialDays_t"
 *
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_CalendarSpecialDayEntryParse(
            zclSE_CalendarPublishSpecialDays_t *pCmd )
{
  uint8_t len;

  // Initialize in case of failure
  pCmd->pEntries = NULL;

  // Get expected length for entries
  len = ZCL_SE_CALENDAR_SPECIAL_DAY_ENTRY_LEN * pCmd->numOfEntries;

  // Check for valid raw fields and minimum packet length
  if ( ( pCmd->rawPayloadLen && ( pCmd->pRawPayload == NULL ) ) ||
       ( pCmd->rawPayloadLen < len                            )    )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  if ( pCmd->numOfEntries )
  {
    uint8_t *pBuf = pCmd->pRawPayload;
    uint8_t entry;

    // Allocate memory for the entry data
    pCmd->pEntries = OsalPort_malloc( pCmd->numOfEntries *
                                     sizeof( zclSE_CalendarSpecialDayEntry_t ) );

    if ( pCmd->pEntries == NULL )
    {
      return ZCL_STATUS_SOFTWARE_FAILURE;
    }

    for ( entry = 0; entry < pCmd->numOfEntries; entry++ )
    {
      pCmd->pEntries[entry].specialDayDate = OsalPort_buildUint32( pBuf, 4 );
      pBuf += 4;
      pCmd->pEntries[entry].dayIDRef = *pBuf++;
    }
  }

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_DeviceMgmtSendPublishChangeOfTenancy
 *
 * @brief   Send COMMAND_SE_DEVICE_MGMT_PUBLISH_CHANGE_OF_TENANCY.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_DeviceMgmtSendPublishChangeOfTenancy(
            uint8_t srcEP, afAddrType_t *dstAddr,
            zclSE_DeviceMgmtPublishChangeOfTenancy_t *pCmd,
            uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t cmdBuf[ZCL_SE_DEVICE_MGMT_PUBLISH_CHANGE_OF_TENANCY_LEN];
  uint8_t *pBuf = cmdBuf;

  pBuf = OsalPort_bufferUint32( pBuf, pCmd->providerID );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->issuerEvtID );
  *pBuf++ = pCmd->tariffType;
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->implDateTime );
  OsalPort_bufferUint32( pBuf, pCmd->propTenancyChangeCtrl );

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_DEVICE_MGMT,
                            COMMAND_SE_DEVICE_MGMT_PUBLISH_CHANGE_OF_TENANCY, TRUE,
                            ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0,
                            seqNum, ZCL_SE_DEVICE_MGMT_PUBLISH_CHANGE_OF_TENANCY_LEN, cmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_DeviceMgmtSendPublishChangeOfSupplier
 *
 * @brief   Send COMMAND_SE_DEVICE_MGMT_PUBLISH_CHANGE_OF_SUPPLIER.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_DeviceMgmtSendPublishChangeOfSupplier(
            uint8_t srcEP, afAddrType_t *dstAddr,
            zclSE_DeviceMgmtPublishChangeOfSupplier_t *pCmd,
            uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t *pCmdBuf;
  uint16_t cmdBufLen;
  uint8_t *pBuf;

  // Allocate command buffer
  cmdBufLen = ZCL_SE_DEVICE_MGMT_PUBLISH_CHANGE_OF_SUPPLIER_LEN +
              zclSE_UTF8StringLen( &pCmd->propProviderName ) +
              zclSE_UTF8StringLen( &pCmd->propProviderContactDetails );

  pCmdBuf = OsalPort_malloc( cmdBufLen );
  if ( pCmdBuf == NULL )
  {
    return ZMemError;
  }

  pBuf = pCmdBuf;
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->currProviderID );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->issuerEvtID );
  *pBuf++ = pCmd->tariffType;
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->propProviderID );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->providerChangeImplTime );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->providerChangeCtrl );
  pBuf = zclSE_UTF8StringBuild( pBuf, &pCmd->propProviderName );
  pBuf = zclSE_UTF8StringBuild( pBuf, &pCmd->propProviderContactDetails );

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_DEVICE_MGMT,
                            COMMAND_SE_DEVICE_MGMT_PUBLISH_CHANGE_OF_SUPPLIER, TRUE,
                            ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0,
                            seqNum, cmdBufLen, pCmdBuf );

  OsalPort_free( pCmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_DeviceMgmtSendReqNewPWRsp
 *
 * @brief   Send COMMAND_SE_DEVICE_MGMT_REQ_NEW_PW_RSP.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_DeviceMgmtSendReqNewPWRsp( uint8_t srcEP, afAddrType_t *dstAddr,
                                           zclSE_DeviceMgmtReqNewPWRsp_t *pCmd,
                                           uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t *pCmdBuf;
  uint16_t cmdBufLen;
  uint8_t *pBuf;

  // Allocate command buffer
  cmdBufLen = ZCL_SE_DEVICE_MGMT_REQ_NEW_PW_RSP_LEN + zclSE_UTF8StringLen( &pCmd->pw );

  pCmdBuf = OsalPort_malloc( cmdBufLen );
  if ( pCmdBuf == NULL )
  {
    return ZMemError;
  }

  pBuf = pCmdBuf;
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->issuerEvtID );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->implDateTime );
  *pBuf++ = LO_UINT16( pCmd->duration );
  *pBuf++ = HI_UINT16( pCmd->duration );
  *pBuf++ = pCmd->type;
  pBuf = zclSE_UTF8StringBuild( pBuf, &pCmd->pw );

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_DEVICE_MGMT,
                            COMMAND_SE_DEVICE_MGMT_REQ_NEW_PW_RSP, TRUE,
                            ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0,
                            seqNum, cmdBufLen, pCmdBuf );

  OsalPort_free( pCmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_DeviceMgmtSendUpdateSiteID
 *
 * @brief   Send COMMAND_SE_DEVICE_MGMT_UPDATE_SITE_ID.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_DeviceMgmtSendUpdateSiteID( uint8_t srcEP, afAddrType_t *dstAddr,
                                            zclSE_DeviceMgmtUpdateSiteID_t *pCmd,
                                            uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t *pCmdBuf;
  uint16_t cmdBufLen;
  uint8_t *pBuf;

  // Allocate command buffer
  cmdBufLen = ZCL_SE_DEVICE_MGMT_UPDATE_SITE_ID_LEN +
              zclSE_UTF8StringLen( &pCmd->siteID );

  pCmdBuf = OsalPort_malloc( cmdBufLen );
  if ( pCmdBuf == NULL )
  {
    return ZMemError;
  }

  pBuf = pCmdBuf;
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->issuerEvtID );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->siteIDTime );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->providerID );
  pBuf = zclSE_UTF8StringBuild( pBuf, &pCmd->siteID );

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_DEVICE_MGMT,
                            COMMAND_SE_DEVICE_MGMT_UPDATE_SITE_ID, TRUE,
                            ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0,
                            seqNum, cmdBufLen, pCmdBuf );

  OsalPort_free( pCmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_DeviceMgmtSendSetEvtCfg
 *
 * @brief   Send COMMAND_SE_DEVICE_MGMT_SET_EVT_CFG.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_DeviceMgmtSendSetEvtCfg( uint8_t srcEP, afAddrType_t *dstAddr,
                                         zclSE_DeviceMgmtSetEvtCfg_t *pCmd,
                                         uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t *pCmdBuf;
  uint16_t cmdBufLen;
  uint8_t *pBuf;

  // Calculate variable payload size
  if ( pCmd->cfgCtrl == ZCL_SE_DEVICE_MGMT_EVT_CFG_BY_LIST )
  {
    cmdBufLen = ZCL_SE_DEVICE_MGMT_EVT_CFG_LIST_LEN +
                pCmd->cfgCtrlFld.cfgList.numOfEvtIDs * ZCL_SE_DEVICE_MGMT_EVT_ID_LEN;
  }
  else if ( pCmd->cfgCtrl == ZCL_SE_DEVICE_MGMT_EVT_CFG_BY_EVT_GROUP )
  {
    cmdBufLen = sizeof( uint16_t );
  }
  else if ( ( pCmd->cfgCtrl == ZCL_SE_DEVICE_MGMT_EVT_CFG_BY_LOG_TYPE ) ||
            ( pCmd->cfgCtrl == ZCL_SE_DEVICE_MGMT_EVT_CFG_BY_MATCH )     )
  {
    cmdBufLen = sizeof( uint8_t );
  }
  else
  {
    cmdBufLen = 0;
  }

  // Allocate command buffer
  cmdBufLen = ZCL_SE_DEVICE_MGMT_SET_EVT_CFG_LEN + cmdBufLen;

  pCmdBuf = OsalPort_malloc( cmdBufLen );
  if ( pCmdBuf == NULL )
  {
    return ZMemError;
  }

  pBuf = pCmdBuf;
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->issuerEvtID );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->startDateTime );
  *pBuf++ = pCmd->evtCfg;
  *pBuf++ = pCmd->cfgCtrl;

  if ( pCmd->cfgCtrl == ZCL_SE_DEVICE_MGMT_EVT_CFG_BY_LIST )
  {
    uint8_t id;

    *pBuf++ = pCmd->cfgCtrlFld.cfgList.numOfEvtIDs;

    for ( id = 0; id < pCmd->cfgCtrlFld.cfgList.numOfEvtIDs; id++ )
    {
      *pBuf++ = LO_UINT16( pCmd->cfgCtrlFld.cfgList.pEvtIDs[id] );
      *pBuf++ = HI_UINT16( pCmd->cfgCtrlFld.cfgList.pEvtIDs[id] );
    }
  }
  else if ( pCmd->cfgCtrl == ZCL_SE_DEVICE_MGMT_EVT_CFG_BY_EVT_GROUP )
  {
    *pBuf++ = LO_UINT16( pCmd->cfgCtrlFld.groupID );
    *pBuf++ = HI_UINT16( pCmd->cfgCtrlFld.groupID );
  }
  else if ( ( pCmd->cfgCtrl == ZCL_SE_DEVICE_MGMT_EVT_CFG_BY_LOG_TYPE ) ||
            ( pCmd->cfgCtrl == ZCL_SE_DEVICE_MGMT_EVT_CFG_BY_MATCH )     )
  {
    *pBuf++ = pCmd->cfgCtrlFld.logID; // Reuse for match since same layout
  }

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_DEVICE_MGMT,
                            COMMAND_SE_DEVICE_MGMT_SET_EVT_CFG, TRUE,
                            ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0,
                            seqNum, cmdBufLen, pCmdBuf );

  OsalPort_free( pCmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_DeviceMgmtSendGetEvtCfg
 *
 * @brief   Send COMMAND_SE_DEVICE_MGMT_GET_EVT_CFG.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_DeviceMgmtSendGetEvtCfg( uint8_t srcEP, afAddrType_t *dstAddr,
                                         zclSE_DeviceMgmtGetEvtCfg_t *pCmd,
                                         uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t cmdBuf[ZCL_SE_DEVICE_MGMT_GET_EVT_CFG_LEN];
  uint8_t *pBuf = cmdBuf;

  *pBuf++ = LO_UINT16( pCmd->evtID );
  *pBuf++ = HI_UINT16( pCmd->evtID );

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_DEVICE_MGMT,
                            COMMAND_SE_DEVICE_MGMT_GET_EVT_CFG, TRUE,
                            ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0,
                            seqNum, ZCL_SE_DEVICE_MGMT_GET_EVT_CFG_LEN, cmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_DeviceMgmtSendUpdateCIN
 *
 * @brief   Send COMMAND_SE_DEVICE_MGMT_UPDATE_CIN.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_DeviceMgmtSendUpdateCIN( uint8_t srcEP, afAddrType_t *dstAddr,
                                         zclSE_DeviceMgmtUpdateCIN_t *pCmd,
                                         uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t *pCmdBuf;
  uint16_t cmdBufLen;
  uint8_t *pBuf;

  // Allocate command buffer
  cmdBufLen = ZCL_SE_DEVICE_MGMT_UPDATE_CIN_LEN +
              zclSE_UTF8StringLen( &pCmd->customerIDNum );

  pCmdBuf = OsalPort_malloc( cmdBufLen );
  if ( pCmdBuf == NULL )
  {
    return ZMemError;
  }

  pBuf = pCmdBuf;
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->issuerEvtID );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->time );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->providerID );
  pBuf = zclSE_UTF8StringBuild( pBuf, &pCmd->customerIDNum );

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_DEVICE_MGMT,
                            COMMAND_SE_DEVICE_MGMT_UPDATE_CIN, TRUE,
                            ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0,
                            seqNum, cmdBufLen, pCmdBuf );

  OsalPort_free( pCmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_DeviceMgmtSendReqNewPW
 *
 * @brief   Send COMMAND_SE_DEVICE_MGMT_REQ_NEW_PW.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_DeviceMgmtSendReqNewPW( uint8_t srcEP, afAddrType_t *dstAddr,
                                        zclSE_DeviceMgmtReqNewPW_t *pCmd,
                                        uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t cmdBuf[ZCL_SE_DEVICE_MGMT_REQ_NEW_PW_LEN];
  uint8_t *pBuf = cmdBuf;

  *pBuf++ = pCmd->type;

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_DEVICE_MGMT,
                            COMMAND_SE_DEVICE_MGMT_REQ_NEW_PW, TRUE,
                            ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0,
                            seqNum, ZCL_SE_DEVICE_MGMT_REQ_NEW_PW_LEN, cmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_DeviceMgmtSendReportEvtCfg
 *
 * @brief   Send COMMAND_SE_DEVICE_MGMT_REPORT_EVT_CFG.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_DeviceMgmtSendReportEvtCfg( uint8_t srcEP, afAddrType_t *dstAddr,
                                            zclSE_DeviceMgmtReportEvtCfg_t *pCmd,
                                            uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t *pCmdBuf;
  uint16_t cmdBufLen = 0;
  uint8_t *pBuf;

  // Check for a non fragmented, valid "pEvts" field, then calculate length
  if ( ( pCmd->cmdTotal <= 1 ) && ( pCmd->pEvts != NULL ) )
  {
    cmdBufLen = pCmd->numOfEvts * ZCL_SE_DEVICE_MGMT_EVT_LEN;
  }
  else if ( pCmd->pRawPayload != NULL )
  {
    // Fragmented -- use raw payload fields
    cmdBufLen = pCmd->rawPayloadLen;
  }

  // Allocate command buffer
  cmdBufLen = ZCL_SE_DEVICE_MGMT_REPORT_EVT_CFG_LEN + cmdBufLen;

  pCmdBuf = OsalPort_malloc( cmdBufLen );
  if ( pCmdBuf == NULL )
  {
    return ZMemError;
  }

  pBuf = pCmdBuf;
  *pBuf++ = pCmd->cmdIdx;
  *pBuf++ = pCmd->cmdTotal;

  // Check for a non fragmented, valid "pEvts" field, then serialize
  if ( ( pCmd->cmdTotal <= 1 ) && ( pCmd->pEvts != NULL ) )
  {
    uint8_t evt;

    for ( evt = 0; evt < pCmd->numOfEvts; evt++ )
    {
      *pBuf++ = LO_UINT16( pCmd->pEvts[evt].evtID );
      *pBuf++ = HI_UINT16( pCmd->pEvts[evt].evtID );
      *pBuf++ = pCmd->pEvts[evt].evtCfg;
    }
  }
  else if ( pCmd->pRawPayload )
  {
    // Fragmented -- use raw payload fields
    pBuf = OsalPort_memcpy( pBuf, pCmd->pRawPayload, pCmd->rawPayloadLen );
  }

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_DEVICE_MGMT,
                            COMMAND_SE_DEVICE_MGMT_REPORT_EVT_CFG, TRUE,
                            ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0,
                            seqNum, cmdBufLen, pCmdBuf );

  OsalPort_free( pCmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_DeviceMgmtParsePublishChangeOfTenancy
 *
 * @brief   Parse COMMAND_SE_DEVICE_MGMT_PUBLISH_CHANGE_OF_TENANCY.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_DeviceMgmtParsePublishChangeOfTenancy(
            zclIncoming_t *pInMsg,
            zclSE_DeviceMgmtPublishChangeOfTenancy_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_DEVICE_MGMT_PUBLISH_CHANGE_OF_TENANCY_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  // Parse the command buffer
  pCmd->providerID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->issuerEvtID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->tariffType = *pBuf++;
  pCmd->implDateTime = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->propTenancyChangeCtrl = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_DeviceMgmtParsePublishChangeOfSupplier
 *
 * @brief   Parse COMMAND_SE_DEVICE_MGMT_PUBLISH_CHANGE_OF_SUPPLIER.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_DeviceMgmtParsePublishChangeOfSupplier(
            zclIncoming_t *pInMsg,
            zclSE_DeviceMgmtPublishChangeOfSupplier_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;
  uint8_t *pBufEnd = pInMsg->pData + pInMsg->pDataLen;
  uint8_t exceeded;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_DEVICE_MGMT_PUBLISH_CHANGE_OF_SUPPLIER_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->currProviderID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->issuerEvtID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->tariffType = *pBuf++;
  pCmd->propProviderID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->providerChangeImplTime = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->providerChangeCtrl = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pBuf = zclSE_UTF8StringParse( pBuf, pBufEnd, &pCmd->propProviderName, &exceeded );

  // Check for string length exceeded flag
  if ( exceeded )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pBuf = zclSE_UTF8StringParse( pBuf, pBufEnd, &pCmd->propProviderContactDetails, &exceeded );

  // Check for string length exceeded flag
  if ( exceeded )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_DeviceMgmtParseReqNewPWRsp
 *
 * @brief   Parse COMMAND_SE_DEVICE_MGMT_REQ_NEW_PW_RSP.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_DeviceMgmtParseReqNewPWRsp( zclIncoming_t *pInMsg,
                                            zclSE_DeviceMgmtReqNewPWRsp_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;
  uint8_t *pBufEnd = pInMsg->pData + pInMsg->pDataLen;
  uint8_t exceeded;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_DEVICE_MGMT_REQ_NEW_PW_RSP_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->issuerEvtID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->implDateTime = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->duration = BUILD_UINT16( pBuf[0], pBuf[1] );
  pBuf += 2;
  pCmd->type = *pBuf++;
  pBuf = zclSE_UTF8StringParse( pBuf, pBufEnd, &pCmd->pw, &exceeded );

  // Check for string length exceeded flag
  if ( exceeded )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_DeviceMgmtParseUpdateSiteID
 *
 * @brief   Parse COMMAND_SE_DEVICE_MGMT_UPDATE_SITE_ID.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_DeviceMgmtParseUpdateSiteID(
            zclIncoming_t *pInMsg,
            zclSE_DeviceMgmtUpdateSiteID_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;
  uint8_t *pBufEnd = pInMsg->pData + pInMsg->pDataLen;
  uint8_t exceeded;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_DEVICE_MGMT_UPDATE_SITE_ID_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  // Parse the command buffer
  pCmd->issuerEvtID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->siteIDTime = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->providerID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pBuf = zclSE_UTF8StringParse( pBuf, pBufEnd, &pCmd->siteID, &exceeded );

  // Check for string length exceeded flag
  if ( exceeded )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_DeviceMgmtParseSetEvtCfg
 *
 * @brief   Parse COMMAND_SE_DEVICE_MGMT_SET_EVT_CFG.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_DeviceMgmtParseSetEvtCfg( zclIncoming_t *pInMsg,
                                          zclSE_DeviceMgmtSetEvtCfg_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;
  uint8_t *pBufEnd = pInMsg->pData + pInMsg->pDataLen;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_DEVICE_MGMT_SET_EVT_CFG_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->issuerEvtID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->startDateTime = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->evtCfg = *pBuf++;
  pCmd->cfgCtrl = *pBuf++;
  memset( &pCmd->cfgCtrlFld, 0, sizeof( zclSE_DeviceMgmtEvtCfgCtrl_t ) );

  // Calculate variable payload size
  if ( pCmd->cfgCtrl == ZCL_SE_DEVICE_MGMT_EVT_CFG_BY_LIST )
  {
    if ( pInMsg->pDataLen < ZCL_SE_DEVICE_MGMT_EVT_CFG_LIST_LEN )
    {
      return ZCL_STATUS_MALFORMED_COMMAND;
    }

    pCmd->cfgCtrlFld.cfgList.numOfEvtIDs = *pBuf++;

    if ( pCmd->cfgCtrlFld.cfgList.numOfEvtIDs )
    {
      uint8_t id;

      // Make sure entire list is there
      if ( ( pBufEnd - pBuf  ) <
           ( pCmd->cfgCtrlFld.cfgList.numOfEvtIDs * ZCL_SE_DEVICE_MGMT_EVT_ID_LEN ) )
      {
        return ZCL_STATUS_MALFORMED_COMMAND;
      }

      pCmd->cfgCtrlFld.cfgList.pEvtIDs = OsalPort_malloc( pCmd->cfgCtrlFld.cfgList.numOfEvtIDs *
                                                         ZCL_SE_DEVICE_MGMT_EVT_ID_LEN );

      if ( pCmd->cfgCtrlFld.cfgList.pEvtIDs == NULL )
      {
        return ZCL_STATUS_SOFTWARE_FAILURE;
      }

      for ( id = 0; id < pCmd->cfgCtrlFld.cfgList.numOfEvtIDs; id++ )
      {
        pCmd->cfgCtrlFld.cfgList.pEvtIDs[id] = BUILD_UINT16( pBuf[0], pBuf[1] );
        pBuf += 2;
      }
    }
  }
  else if ( pCmd->cfgCtrl == ZCL_SE_DEVICE_MGMT_EVT_CFG_BY_EVT_GROUP )
  {
    if ( pInMsg->pDataLen < sizeof( uint16_t ) )
    {
      return ZCL_STATUS_MALFORMED_COMMAND;
    }

    pCmd->cfgCtrlFld.groupID = BUILD_UINT16( pBuf[0], pBuf[1] );
    pBuf += 2;
  }
  else if ( ( pCmd->cfgCtrl == ZCL_SE_DEVICE_MGMT_EVT_CFG_BY_LOG_TYPE ) ||
            ( pCmd->cfgCtrl == ZCL_SE_DEVICE_MGMT_EVT_CFG_BY_MATCH )     )
  {
    if ( pInMsg->pDataLen < sizeof( uint8_t ) )
    {
      return ZCL_STATUS_MALFORMED_COMMAND;
    }

    pCmd->cfgCtrlFld.logID = *pBuf++; // Reuse for match since same layout
  }

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_DeviceMgmtParseGetEvtCfg
 *
 * @brief   Parse COMMAND_SE_DEVICE_MGMT_GET_EVT_CFG.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_DeviceMgmtParseGetEvtCfg( zclIncoming_t *pInMsg,
                                          zclSE_DeviceMgmtGetEvtCfg_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_DEVICE_MGMT_GET_EVT_CFG_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->evtID = BUILD_UINT16( pBuf[0], pBuf[1] );
  pBuf += 2;

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_DeviceMgmtParseUpdateCIN
 *
 * @brief   Parse COMMAND_SE_DEVICE_MGMT_UPDATE_CIN.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_DeviceMgmtParseUpdateCIN( zclIncoming_t *pInMsg,
                                          zclSE_DeviceMgmtUpdateCIN_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;
  uint8_t *pBufEnd = pInMsg->pData + pInMsg->pDataLen;
  uint8_t exceeded;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_DEVICE_MGMT_UPDATE_CIN_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->issuerEvtID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->time = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->providerID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pBuf = zclSE_UTF8StringParse( pBuf, pBufEnd, &pCmd->customerIDNum, &exceeded );

  // Check for string length exceeded flag
  if ( exceeded )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_DeviceMgmtParseReqNewPW
 *
 * @brief   Parse COMMAND_SE_DEVICE_MGMT_REQ_NEW_PW.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_DeviceMgmtParseReqNewPW( zclIncoming_t *pInMsg,
                                         zclSE_DeviceMgmtReqNewPW_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_DEVICE_MGMT_REQ_NEW_PW_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->type = *pBuf++;

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_DeviceMgmtParseReportEvtCfg
 *
 * @brief   Parse COMMAND_SE_DEVICE_MGMT_REPORT_EVT_CFG.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_DeviceMgmtParseReportEvtCfg(
            zclIncoming_t *pInMsg,
            zclSE_DeviceMgmtReportEvtCfg_t *pCmd )
{
  ZStatus_t status = ZCL_STATUS_SUCCESS;
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_DEVICE_MGMT_REPORT_EVT_CFG_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->cmdIdx = *pBuf++;
  pCmd->cmdTotal = *pBuf++;
  pCmd->rawPayloadLen = pInMsg->pDataLen - ZCL_SE_DEVICE_MGMT_REPORT_EVT_CFG_LEN;

  if ( pCmd->rawPayloadLen )
  {
    pCmd->pRawPayload = pBuf;
  }
  else
  {
    pCmd->pRawPayload = NULL;
  }

  // Check for fragmentation
  if ( pCmd->cmdTotal <= 1 )
  {
    status = zclSE_DeviceMgmtEvtParse( pCmd );
  }
  else
  {
    pCmd->pEvts = NULL;
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_DeviceMgmtHdlPublishChangeOfTenancy
 *
 * @brief   Handle COMMAND_SE_DEVICE_MGMT_PUBLISH_CHANGE_OF_TENANCY.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_DeviceMgmtHdlPublishChangeOfTenancy(
            zclIncoming_t *pInMsg,
            zclSE_DeviceMgmtPublishChangeOfTenancyCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_DeviceMgmtPublishChangeOfTenancy_t cmd;

    status = zclSE_DeviceMgmtParsePublishChangeOfTenancy( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      pfnCB( pInMsg, &cmd );
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_DeviceMgmtHdlPublishChangeOfSupplier
 *
 * @brief   Handle COMMAND_SE_DEVICE_MGMT_PUBLISH_CHANGE_OF_SUPPLIER.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_DeviceMgmtHdlPublishChangeOfSupplier(
            zclIncoming_t *pInMsg,
            zclSE_DeviceMgmtPublishChangeOfSupplierCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_DeviceMgmtPublishChangeOfSupplier_t *pCmd;

    pCmd = OsalPort_malloc( sizeof( zclSE_DeviceMgmtPublishChangeOfSupplier_t ) );
    if ( pCmd == NULL )
    {
       return ZCL_STATUS_SOFTWARE_FAILURE;
    }

    status = zclSE_DeviceMgmtParsePublishChangeOfSupplier( pInMsg, pCmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      pfnCB( pInMsg, pCmd );
    }

    OsalPort_free( pCmd );
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_DeviceMgmtHdlReqNewPWRsp
 *
 * @brief   Handle COMMAND_SE_DEVICE_MGMT_REQ_NEW_PW_RSP.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_DeviceMgmtHdlReqNewPWRsp( zclIncoming_t *pInMsg,
                                          zclSE_DeviceMgmtReqNewPWRspCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_DeviceMgmtReqNewPWRsp_t cmd;

    status = zclSE_DeviceMgmtParseReqNewPWRsp( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      pfnCB( pInMsg, &cmd );
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_DeviceMgmtHdlUpdateSiteID
 *
 * @brief   Handle COMMAND_SE_DEVICE_MGMT_UPDATE_SITE_ID.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_DeviceMgmtHdlUpdateSiteID( zclIncoming_t *pInMsg,
                                           zclSE_DeviceMgmtUpdateSiteID_CB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_DeviceMgmtUpdateSiteID_t cmd;

    status = zclSE_DeviceMgmtParseUpdateSiteID( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      pfnCB( pInMsg, &cmd );
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_DeviceMgmtHdlSetEvtCfg
 *
 * @brief   Handle COMMAND_SE_DEVICE_MGMT_SET_EVT_CFG.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_DeviceMgmtHdlSetEvtCfg( zclIncoming_t *pInMsg,
                                        zclSE_DeviceMgmtSetEvtCfgCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_DeviceMgmtSetEvtCfg_t cmd;

    status = zclSE_DeviceMgmtParseSetEvtCfg( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      pfnCB( pInMsg, &cmd );

      if ( cmd.cfgCtrlFld.cfgList.pEvtIDs != NULL )
      {
        OsalPort_free( cmd.cfgCtrlFld.cfgList.pEvtIDs );
      }
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_DeviceMgmtHdlGetEvtCfg
 *
 * @brief   Handle COMMAND_SE_DEVICE_MGMT_GET_EVT_CFG.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_DeviceMgmtHdlGetEvtCfg( zclIncoming_t *pInMsg,
                                        zclSE_DeviceMgmtGetEvtCfgCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_DeviceMgmtGetEvtCfg_t cmd;

    status = zclSE_DeviceMgmtParseGetEvtCfg( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      status = pfnCB( pInMsg, &cmd );
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_DeviceMgmtHdlUpdateCIN
 *
 * @brief   Handle COMMAND_SE_DEVICE_MGMT_UPDATE_CIN.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_DeviceMgmtHdlUpdateCIN( zclIncoming_t *pInMsg,
                                        zclSE_DeviceMgmtUpdateCIN_CB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_DeviceMgmtUpdateCIN_t cmd;

    status = zclSE_DeviceMgmtParseUpdateCIN( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      status = pfnCB( pInMsg, &cmd );
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_DeviceMgmtHdlGetChangeOfTenancy
 *
 * @brief   Handle COMMAND_SE_DEVICE_MGMT_GET_CHANGE_OF_TENANCY (handle with no payload).
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_DeviceMgmtHdlGetChangeOfTenancy(
            zclIncoming_t *pInMsg,
            zclSE_DeviceMgmtGetChangeOfTenancyCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    status = pfnCB( pInMsg );
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_DeviceMgmtHdlGetChangeOfSupplier
 *
 * @brief   Handle COMMAND_SE_DEVICE_MGMT_GET_CHANGE_OF_SUPPLIER (handle with no payload).
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_DeviceMgmtHdlGetChangeOfSupplier(
            zclIncoming_t *pInMsg,
            zclSE_DeviceMgmtGetChangeOfSupplierCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    status = pfnCB( pInMsg );
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_DeviceMgmtHdlReqNewPW
 *
 * @brief   Handle COMMAND_SE_DEVICE_MGMT_REQ_NEW_PW.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_DeviceMgmtHdlReqNewPW( zclIncoming_t *pInMsg,
                                       zclSE_DeviceMgmtReqNewPW_CB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_DeviceMgmtReqNewPW_t cmd;

    status = zclSE_DeviceMgmtParseReqNewPW( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      status = pfnCB( pInMsg, &cmd );
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_DeviceMgmtHdlGetSiteID
 *
 * @brief   Handle COMMAND_SE_DEVICE_MGMT_GET_SITE_ID (handle with no payload).
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_DeviceMgmtHdlGetSiteID( zclIncoming_t *pInMsg,
                                        zclSE_DeviceMgmtGetSiteID_CB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    status = pfnCB( pInMsg );
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_DeviceMgmtHdlReportEvtCfg
 *
 * @brief   Handle COMMAND_SE_DEVICE_MGMT_REPORT_EVT_CFG.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_DeviceMgmtHdlReportEvtCfg( zclIncoming_t *pInMsg,
                                           zclSE_DeviceMgmtReportEvtCfgCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_DeviceMgmtReportEvtCfg_t cmd;

    status = zclSE_DeviceMgmtParseReportEvtCfg( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      status = pfnCB( pInMsg, &cmd );

      if ( cmd.pEvts != NULL )
      {
        OsalPort_free( cmd.pEvts );
      }
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_DeviceMgmtHdlGetCIN
 *
 * @brief   Handle COMMAND_SE_DEVICE_MGMT_GET_CIN (handle with no payload).
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_DeviceMgmtHdlGetCIN( zclIncoming_t *pInMsg,
                                     zclSE_DeviceMgmtGetCIN_CB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    status = pfnCB( pInMsg );
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_DeviceMgmtHdlClientCmd
 *
 * @brief   Client command handler for ZCL_CLUSTER_ID_SE_DEVICE_MGMT.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCBs - client callbacks
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_DeviceMgmtHdlClientCmd( zclIncoming_t *pInMsg,
                                        const zclSE_DeviceMgmtClientCBs_t *pCBs )
{
  ZStatus_t status;

  // Guard against NULL pointer
  if ( pCBs == NULL )
  {
    return ZCL_STATUS_FAILURE;
  }

  // Process Server-to-Client Commands
  switch ( pInMsg->hdr.commandID )
  {
    case COMMAND_SE_DEVICE_MGMT_PUBLISH_CHANGE_OF_TENANCY:
      status = zclSE_DeviceMgmtHdlPublishChangeOfTenancy( pInMsg,
                                                          pCBs->pfnPublishChangeOfTenancy );
      break;

    case COMMAND_SE_DEVICE_MGMT_PUBLISH_CHANGE_OF_SUPPLIER:
      status = zclSE_DeviceMgmtHdlPublishChangeOfSupplier( pInMsg,
                                                           pCBs->pfnPublishChangeOfSupplier );
      break;

    case COMMAND_SE_DEVICE_MGMT_REQ_NEW_PW_RSP:
      status = zclSE_DeviceMgmtHdlReqNewPWRsp( pInMsg, pCBs->pfnReqNewPWRsp );
      break;

    case COMMAND_SE_DEVICE_MGMT_UPDATE_SITE_ID:
      status = zclSE_DeviceMgmtHdlUpdateSiteID( pInMsg, pCBs->pfnUpdateSiteID_ );
      break;

    case COMMAND_SE_DEVICE_MGMT_SET_EVT_CFG:
      status = zclSE_DeviceMgmtHdlSetEvtCfg( pInMsg, pCBs->pfnSetEvtCfg );
      break;

    case COMMAND_SE_DEVICE_MGMT_GET_EVT_CFG:
      status = zclSE_DeviceMgmtHdlGetEvtCfg( pInMsg, pCBs->pfnGetEvtCfg );
      break;

    case COMMAND_SE_DEVICE_MGMT_UPDATE_CIN:
      status = zclSE_DeviceMgmtHdlUpdateCIN( pInMsg, pCBs->pfnUpdateCIN_ );
      break;

    default:
      status = ZCL_STATUS_FAILURE;
      break;
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_DeviceMgmtHdlServerCmd
 *
 * @brief   Server command handler for ZCL_CLUSTER_ID_SE_DEVICE_MGMT.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCBs - server callbacks
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_DeviceMgmtHdlServerCmd( zclIncoming_t *pInMsg,
                                        const zclSE_DeviceMgmtServerCBs_t *pCBs )
{
  ZStatus_t status;

  // Guard against NULL pointer
  if ( pCBs == NULL )
  {
    return ZCL_STATUS_FAILURE;
  }

  // Process Client-to-Server Commands
  switch ( pInMsg->hdr.commandID )
  {
    case COMMAND_SE_DEVICE_MGMT_GET_CHANGE_OF_TENANCY:
      status = zclSE_DeviceMgmtHdlGetChangeOfTenancy( pInMsg, pCBs->pfnGetChangeOfTenancy );
      break;

    case COMMAND_SE_DEVICE_MGMT_GET_CHANGE_OF_SUPPLIER:
      status = zclSE_DeviceMgmtHdlGetChangeOfSupplier( pInMsg, pCBs->pfnGetChangeOfSupplier );
      break;

    case COMMAND_SE_DEVICE_MGMT_REQ_NEW_PW:
      status = zclSE_DeviceMgmtHdlReqNewPW( pInMsg, pCBs->pfnReqNewPW_ );
      break;

    case COMMAND_SE_DEVICE_MGMT_GET_SITE_ID:
      status = zclSE_DeviceMgmtHdlGetSiteID( pInMsg, pCBs->pfnGetSiteID_ );
      break;

    case COMMAND_SE_DEVICE_MGMT_REPORT_EVT_CFG:
      status = zclSE_DeviceMgmtHdlReportEvtCfg( pInMsg,
                                                pCBs->pfnReportEvtCfg );
      break;

    case COMMAND_SE_DEVICE_MGMT_GET_CIN:
      status = zclSE_DeviceMgmtHdlGetCIN( pInMsg, pCBs->pfnGetCIN_ );
      break;

    default:
      status = ZCL_STATUS_FAILURE;
      break;
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_DeviceMgmtEvtParse
 *
 * @brief   Utility function for parsing "pEvts" payload -- see "zclSE_DeviceMgmtReportEvtCfg_t"
 *
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_DeviceMgmtEvtParse( zclSE_DeviceMgmtReportEvtCfg_t *pCmd )
{
  // Initialize in case of failure
  pCmd->pEvts = NULL;

  // Check for valid raw fields
  if ( pCmd->rawPayloadLen && ( pCmd->pRawPayload == NULL ) )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  // Get the number of events
  pCmd->numOfEvts = pCmd->rawPayloadLen / ZCL_SE_DEVICE_MGMT_EVT_LEN;

  if ( pCmd->numOfEvts )
  {
    uint8_t *pBuf = pCmd->pRawPayload;
    uint8_t evt;

    // Allocate memory for the event data
    pCmd->pEvts = OsalPort_malloc( pCmd->numOfEvts * sizeof( zclSE_DeviceMgmtEvt_t ) );

    if ( pCmd->pEvts == NULL )
    {
      return ZCL_STATUS_SOFTWARE_FAILURE;
    }

    for ( evt = 0; evt < pCmd->numOfEvts; evt++ )
    {
      pCmd->pEvts[evt].evtID = BUILD_UINT16( pBuf[0], pBuf[1] );
      pBuf += 2;
      pCmd->pEvts[evt].evtCfg = *pBuf++;
    }
  }

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_EventsSendPublishEvt
 *
 * @brief   Send COMMAND_SE_EVENTS_PUBLISH_EVT.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_EventsSendPublishEvt( uint8_t srcEP, afAddrType_t *dstAddr,
                                      zclSE_EventsPublishEvt_t *pCmd,
                                      uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t *pCmdBuf;
  uint16_t cmdBufLen;
  uint8_t *pBuf;

  // Allocate command buffer
  cmdBufLen = ZCL_SE_EVENTS_PUBLISH_EVT_LEN + zclSE_UTF8StringLen( &pCmd->evtData );

  pCmdBuf = OsalPort_malloc( cmdBufLen );
  if ( pCmdBuf == NULL )
  {
    return ZMemError;
  }

  pBuf = pCmdBuf;
  *pBuf++ = pCmd->logID;
  *pBuf++ = LO_UINT16( pCmd->evtID );
  *pBuf++ = HI_UINT16( pCmd->evtID );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->evtTime );
  *pBuf++ = pCmd->evtCtrl;
  pBuf = zclSE_UTF8StringBuild( pBuf, &pCmd->evtData );

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_EVENTS,
                            COMMAND_SE_EVENTS_PUBLISH_EVT, TRUE,
                            ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp,
                            0, seqNum, cmdBufLen, pCmdBuf );

  OsalPort_free( pCmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_EventsSendPublishEvtLog
 *
 * @brief   Send COMMAND_SE_EVENTS_PUBLISH_EVT_LOG.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_EventsSendPublishEvtLog( uint8_t srcEP, afAddrType_t *dstAddr,
                                         zclSE_EventsPublishEvtLog_t *pCmd,
                                         uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t *pCmdBuf;
  uint16_t cmdBufLen;
  uint8_t *pBuf;

  // Allocate command buffer
  cmdBufLen = ZCL_SE_EVENTS_PUBLISH_EVT_LOG_LEN +
              zclSE_EventsEvtLogLen( pCmd );

  pCmdBuf = OsalPort_malloc( cmdBufLen );
  if ( pCmdBuf == NULL )
  {
    return ZMemError;
  }

  pBuf = pCmdBuf;
  *pBuf++ = LO_UINT16( pCmd->numOfMatchingEvts );
  *pBuf++ = HI_UINT16( pCmd->numOfMatchingEvts );
  *pBuf++ = pCmd->cmdIdx;
  *pBuf++ = pCmd->cmdTotal;
  pBuf = zclSE_EventsEvtLogSerialize( pCmd, pBuf );

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_EVENTS,
                            COMMAND_SE_EVENTS_PUBLISH_EVT_LOG, TRUE,
                            ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp,
                            0, seqNum, cmdBufLen, pCmdBuf );

  OsalPort_free( pCmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_EventsSendClearEvtLogRsp
 *
 * @brief   Send COMMAND_SE_EVENTS_CLEAR_EVT_LOG_RSP.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_EventsSendClearEvtLogRsp( uint8_t srcEP, afAddrType_t *dstAddr,
                                          zclSE_EventsClearEvtLogRsp_t *pCmd,
                                          uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t cmdBuf[ZCL_SE_EVENTS_CLEAR_EVT_LOG_RSP_LEN];
  uint8_t *pBuf = cmdBuf;

  *pBuf = pCmd->clearStatus;

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_EVENTS,
                            COMMAND_SE_EVENTS_CLEAR_EVT_LOG_RSP, TRUE,
                            ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp,
                            0, seqNum, ZCL_SE_EVENTS_CLEAR_EVT_LOG_RSP_LEN, cmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_EventsSendGetEvtLog
 *
 * @brief   Send COMMAND_SE_EVENTS_GET_EVT_LOG.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_EventsSendGetEvtLog( uint8_t srcEP, afAddrType_t *dstAddr,
                                     zclSE_EventsGetEvtLog_t *pCmd,
                                     uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t cmdBuf[ZCL_SE_EVENTS_GET_EVT_LOG_LEN];
  uint8_t *pBuf = cmdBuf;

  *pBuf = pCmd->evtCtrl << 4;
  *pBuf++ |= pCmd->logID & 0x0F;
  *pBuf++ = LO_UINT16( pCmd->evtID );
  *pBuf++ = HI_UINT16( pCmd->evtID );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->startTime );
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->endTime );
  *pBuf++ = pCmd->numOfEvts;
  *pBuf++ = LO_UINT16( pCmd->evtOffset );
  *pBuf = HI_UINT16( pCmd->evtOffset );

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_EVENTS,
                            COMMAND_SE_EVENTS_GET_EVT_LOG, TRUE,
                            ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp,
                            0, seqNum, ZCL_SE_EVENTS_GET_EVT_LOG_LEN, cmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_EventsSendClearEvtLog
 *
 * @brief   Send COMMAND_SE_EVENTS_CLEAR_EVT_LOG.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_EventsSendClearEvtLog( uint8_t srcEP, afAddrType_t *dstAddr,
                                       zclSE_EventsClearEvtLog_t *pCmd,
                                       uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t cmdBuf[ZCL_SE_EVENTS_CLEAR_EVT_LOG_LEN];
  uint8_t *pBuf = cmdBuf;

  *pBuf = pCmd->logID & ZCL_SE_EVENTS_LOG_ID_MASK;

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_EVENTS,
                          COMMAND_SE_EVENTS_CLEAR_EVT_LOG, TRUE,
                          ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp,
                          0, seqNum, ZCL_SE_EVENTS_CLEAR_EVT_LOG_LEN, cmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_EventsParsePublishEvt
 *
 * @brief   Parse COMMAND_SE_EVENTS_PUBLISH_EVT.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_EventsParsePublishEvt( zclIncoming_t *pInMsg,
                                       zclSE_EventsPublishEvt_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;
  uint8_t *pBufEnd = pInMsg->pData + pInMsg->pDataLen;
  uint8_t exceeded;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_EVENTS_PUBLISH_EVT_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->logID = *pBuf++;
  pCmd->evtID = BUILD_UINT16( pBuf[0], pBuf[1] );
  pBuf += 2;
  pCmd->evtTime = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->evtCtrl = *pBuf++;
  pBuf = zclSE_UTF8StringParse( pBuf, pBufEnd, &pCmd->evtData, &exceeded );

  // Check for string length exceeded flag
  if ( exceeded )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_EventsParsePublishEvtLog
 *
 * @brief   Parse COMMAND_SE_EVENTS_PUBLISH_EVT_LOG.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_EventsParsePublishEvtLog( zclIncoming_t *pInMsg,
                                          zclSE_EventsPublishEvtLog_t *pCmd )
{
  ZStatus_t status = ZCL_STATUS_SUCCESS;
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_EVENTS_PUBLISH_EVT_LOG_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->numOfMatchingEvts = BUILD_UINT16( pBuf[0], pBuf[1] );
  pBuf += 2;
  pCmd->cmdIdx = *pBuf++;
  pCmd->cmdTotal = *pBuf++;
  pCmd->rawPayloadLen = pInMsg->pDataLen - ZCL_SE_EVENTS_PUBLISH_EVT_LOG_LEN;

  if ( pCmd->rawPayloadLen )
  {
    pCmd->pRawPayload = pBuf;
  }
  else
  {
    pCmd->pRawPayload = NULL;
  }

  // Check for fragmentation
  if ( pCmd->cmdTotal <= 1 )
  {
    status = zclSE_EventsEvtLogParse( pCmd );
  }
  else
  {
    memset( &pCmd->log, 0, sizeof( zclSE_EventsEvtLog_t ));
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_EventsParseClearEvtLogRsp
 *
 * @brief   Parse COMMAND_SE_EVENTS_CLEAR_EVT_LOG_RSP.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_EventsParseClearEvtLogRsp( zclIncoming_t *pInMsg,
                                           zclSE_EventsClearEvtLogRsp_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_EVENTS_CLEAR_EVT_LOG_RSP_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->clearStatus = *pBuf;

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_EventsParseGetEvtLog
 *
 * @brief   Parse COMMAND_SE_EVENTS_GET_EVT_LOG.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_EventsParseGetEvtLog( zclIncoming_t *pInMsg,
                                      zclSE_EventsGetEvtLog_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_EVENTS_GET_EVT_LOG_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->evtCtrl = *pBuf >> 4;
  pCmd->logID = *pBuf++ & 0x0F;
  pCmd->evtID = BUILD_UINT16( pBuf[0], pBuf[1] );
  pBuf += 2;
  pCmd->startTime = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->endTime = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->numOfEvts = *pBuf++;
  pCmd->evtOffset = BUILD_UINT16( pBuf[0], pBuf[1] );
  pBuf += 2;

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_EventsParseClearEvtLog
 *
 * @brief   Parse COMMAND_SE_EVENTS_CLEAR_EVT_LOG.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_EventsParseClearEvtLog( zclIncoming_t *pInMsg,
                                        zclSE_EventsClearEvtLog_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_EVENTS_CLEAR_EVT_LOG_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->logID = *pBuf & ZCL_SE_EVENTS_LOG_ID_MASK;

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_EventsHdlPublishEvt
 *
 * @brief   Handle COMMAND_SE_EVENTS_PUBLISH_EVT.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_EventsHdlPublishEvt( zclIncoming_t *pInMsg,
                                     zclSE_EventsPublishEvtCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_EventsPublishEvt_t cmd;

    status = zclSE_EventsParsePublishEvt( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      pfnCB( pInMsg, &cmd );
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_EventsHdlPublishEvtLog
 *
 * @brief   Handle COMMAND_SE_EVENTS_PUBLISH_EVT_LOG.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_EventsHdlPublishEvtLog( zclIncoming_t *pInMsg,
                                        zclSE_EventsPublishEvtLogCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_EventsPublishEvtLog_t cmd;

    status = zclSE_EventsParsePublishEvtLog( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      status = pfnCB( pInMsg, &cmd );

      if ( cmd.log.pEvts )
      {
        OsalPort_free( cmd.log.pEvts );
      }
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_EventsHdlClearEvtLogRsp
 *
 * @brief   Handle COMMAND_SE_EVENTS_CLEAR_EVT_LOG_RSP.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_EventsHdlClearEvtLogRsp( zclIncoming_t *pInMsg,
                                         zclSE_EventsClearEvtLogRspCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_EventsClearEvtLogRsp_t cmd;

    status = zclSE_EventsParseClearEvtLogRsp( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      pfnCB( pInMsg, &cmd );
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_EventsHdlGetEvtLog
 *
 * @brief   Handle COMMAND_SE_EVENTS_GET_EVT_LOG.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_EventsHdlGetEvtLog( zclIncoming_t *pInMsg,
                                    zclSE_EventsGetEvtLogCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_EventsGetEvtLog_t cmd;

    status = zclSE_EventsParseGetEvtLog( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      status = pfnCB( pInMsg, &cmd );
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_EventsHdlClearEvtLog
 *
 * @brief   Handle COMMAND_SE_EVENTS_CLEAR_EVT_LOG.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_EventsHdlClearEvtLog( zclIncoming_t *pInMsg,
                                      zclSE_EventsClearEvtLogCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_EventsClearEvtLog_t cmd;

    status = zclSE_EventsParseClearEvtLog( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      pfnCB( pInMsg, &cmd );

      status = ZCL_STATUS_CMD_HAS_RSP;
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_EventsHdlClientCmd
 *
 * @brief   Client command handler for ZCL_CLUSTER_ID_SE_EVENTS.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCBs - client callbacks
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_EventsHdlClientCmd( zclIncoming_t *pInMsg,
                                    const zclSE_EventsClientCBs_t *pCBs )
{
  ZStatus_t status;

  // Guard against NULL pointer
  if ( pCBs == NULL )
  {
    return ZCL_STATUS_FAILURE;
  }

  // Process Server-to-Client Commands
  switch ( pInMsg->hdr.commandID )
  {
    case COMMAND_SE_EVENTS_PUBLISH_EVT:
      status = zclSE_EventsHdlPublishEvt( pInMsg, pCBs->pfnPublishEvt );
      break;

    case COMMAND_SE_EVENTS_PUBLISH_EVT_LOG:
      status = zclSE_EventsHdlPublishEvtLog( pInMsg, pCBs->pfnPublishEvtLog );
      break;

    default:
      status = ZCL_STATUS_FAILURE;
      break;
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_EventsHdlServerCmd
 *
 * @brief   Server command handler for ZCL_CLUSTER_ID_SE_EVENTS.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCBs - server callbacks
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_EventsHdlServerCmd( zclIncoming_t *pInMsg,
                                    const zclSE_EventsServerCBs_t *pCBs )
{
  ZStatus_t status;

  // Guard against NULL pointer
  if ( pCBs == NULL )
  {
    return ZCL_STATUS_FAILURE;
  }

  // Process Client-to-Server Commands
  switch ( pInMsg->hdr.commandID )
  {
    case COMMAND_SE_EVENTS_GET_EVT_LOG:
      status = zclSE_EventsHdlGetEvtLog( pInMsg, pCBs->pfnGetEvtLog );
      break;

    case COMMAND_SE_EVENTS_CLEAR_EVT_LOG:
      status = zclSE_EventsHdlClearEvtLog( pInMsg, pCBs->pfnClearEvtLog );
      break;

    default:
      status = ZCL_STATUS_FAILURE;
      break;
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_EventsEvtLogParse
 *
 * @brief   Utility function for parsing an event log in the COMMAND_SE_EVENTS_PUBLISH_EVT_LOG
 *          payload.
 *
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_EventsEvtLogParse( zclSE_EventsPublishEvtLog_t *pCmd )
{
  uint8_t *pBuf;
  uint8_t *pBufEnd;
  zclSE_EventsEvtLog_t* pLog;

  // Get the log field
  pLog = &pCmd->log;

  // Initialize in case of failure
  memset( pLog, 0, sizeof( zclSE_EventsEvtLog_t ));

  // Check for valid raw fields and minimum packet length
  if ( ( pCmd->rawPayloadLen && ( pCmd->pRawPayload == NULL ) ) ||
       ( pCmd->rawPayloadLen < ZCL_SE_EVENTS_EVT_LOG_HDR_LEN  )     )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  // Initialize buffer pointers
  pBuf = pCmd->pRawPayload;
  pBufEnd = pCmd->pRawPayload + pCmd->rawPayloadLen;

  // Parse the event log header
  pLog->numOfEvts = *pBuf >> 4;
  pLog->ctrl = *pBuf++ & 0x0F;

  if ( pLog->numOfEvts )
  {
    uint8_t exceeded;
    uint8_t evtIdx;
    zclSE_EventsLoggedEvt_t *pEvts;
    zclSE_EventsLoggedEvt_t *pEvt;

    // Allocate memory for the event data
    pEvts = OsalPort_malloc( sizeof( zclSE_EventsLoggedEvt_t ) * pLog->numOfEvts );
    if ( pEvts == NULL )
    {
      return ZCL_STATUS_SOFTWARE_FAILURE;
    }

    // Parse each event in the raw payload
    for ( evtIdx = 0; evtIdx < pLog->numOfEvts; evtIdx++ )
    {
      // Check for valid event payload length
      if ( ( pBufEnd - pBuf  ) < ZCL_SE_EVENTS_LOGGED_EVT_LEN )
      {
        OsalPort_free( pEvts );
        return ZCL_STATUS_MALFORMED_COMMAND;
      }

      pEvt = &pEvts[evtIdx];

      pEvt->logID = *pBuf++;
      pEvt->evtID = BUILD_UINT16( pBuf[0], pBuf[1] );
      pBuf += 2;
      pEvt->evtTime = OsalPort_buildUint32( pBuf, 4 );
      pBuf += 4;
      pBuf = zclSE_UTF8StringParse( pBuf, pBufEnd, &pEvt->evtData, &exceeded );

      // Check for string length exceeded flag
      if ( exceeded )
      {
        OsalPort_free( pEvts );
        return ZCL_STATUS_MALFORMED_COMMAND;
      }
    }

    pLog->pEvts = pEvts;
  }

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_EnergyMgmtSendReportEvtStatus
 *
 * @brief   Send COMMAND_SE_ENERGY_MGMT_REPORT_EVT_STATUS.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_EnergyMgmtSendReportEvtStatus(
            uint8_t srcEP, afAddrType_t *dstAddr,
            zclSE_EnergyMgmtReportEvtStatus_t *pCmd,
            uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t cmdBuf[ZCL_SE_ENERGY_MGMT_REPORT_EVT_STATUS_LEN];
  uint8_t *pBuf = cmdBuf;

  pBuf = OsalPort_bufferUint32( pBuf, pCmd->issuerEvtID );
  *pBuf++ = pCmd->evtStatus;
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->evtStatusTime );
  *pBuf++ = pCmd->critLevelApplied;
  *pBuf++ = LO_UINT16( pCmd->coolingTempSetPtApplied );
  *pBuf++ = HI_UINT16( pCmd->coolingTempSetPtApplied );
  *pBuf++ = LO_UINT16( pCmd->heatingTempSetPtApplied );
  *pBuf++ = HI_UINT16( pCmd->heatingTempSetPtApplied );
  *pBuf++ = pCmd->avgLoadAdjPctApplied;
  *pBuf++ = pCmd->dutyCycleApplied;
  *pBuf = pCmd->evtCtrl;

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_ENERGY_MGMT,
                            COMMAND_SE_ENERGY_MGMT_REPORT_EVT_STATUS, TRUE,
                            ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0,
                            seqNum, ZCL_SE_ENERGY_MGMT_REPORT_EVT_STATUS_LEN, cmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_EnergyMgmtSendManageEvt
 *
 * @brief   Send COMMAND_SE_ENERGY_MGMT_MANAGE_EVT.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_EnergyMgmtSendManageEvt( uint8_t srcEP, afAddrType_t *dstAddr,
                                         zclSE_EnergyMgmtManageEvt_t *pCmd,
                                         uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t cmdBuf[ZCL_SE_ENERGY_MGMT_MANAGE_EVT_LEN];
  uint8_t *pBuf = cmdBuf;

  pBuf = OsalPort_bufferUint32( pBuf, pCmd->issuerEvtID );
  *pBuf++ = LO_UINT16( pCmd->deviceClass );
  *pBuf++ = HI_UINT16( pCmd->deviceClass );
  *pBuf++ = pCmd->utilityEnrollmentGroup;
  *pBuf = pCmd->actionsRequired;

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_ENERGY_MGMT,
                            COMMAND_SE_ENERGY_MGMT_MANAGE_EVT, TRUE,
                            ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0,
                            seqNum, ZCL_SE_ENERGY_MGMT_MANAGE_EVT_LEN, cmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_EnergyMgmtParseReportEvtStatus
 *
 * @brief   Parse COMMAND_SE_ENERGY_MGMT_REPORT_EVT_STATUS
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_EnergyMgmtParseReportEvtStatus(
            zclIncoming_t *pInMsg,
            zclSE_EnergyMgmtReportEvtStatus_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_ENERGY_MGMT_REPORT_EVT_STATUS_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->issuerEvtID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->evtStatus = *pBuf++;
  pCmd->evtStatusTime = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->critLevelApplied = *pBuf++;
  pCmd->coolingTempSetPtApplied = BUILD_UINT16( pBuf[0], pBuf[1] );
  pBuf += 2;
  pCmd->heatingTempSetPtApplied = BUILD_UINT16( pBuf[0], pBuf[1] );
  pBuf += 2;
  pCmd->avgLoadAdjPctApplied = *pBuf++;
  pCmd->dutyCycleApplied = *pBuf++;
  pCmd->evtCtrl = *pBuf;

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_EnergyMgmtParseManageEvt
 *
 * @brief   Parse COMMAND_SE_ENERGY_MGMT_MANAGE_EVT
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_EnergyMgmtParseManageEvt( zclIncoming_t *pInMsg,
                                          zclSE_EnergyMgmtManageEvt_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_ENERGY_MGMT_MANAGE_EVT_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->issuerEvtID = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->deviceClass = BUILD_UINT16( pBuf[0], pBuf[1] );
  pBuf += 2;
  pCmd->utilityEnrollmentGroup = *pBuf++;
  pCmd->actionsRequired =  *pBuf;

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_EnergyMgmtHdlReportEvtStatus
 *
 * @brief   Handle COMMAND_SE_ENERGY_MGMT_REPORT_EVT_STATUS.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_EnergyMgmtHdlReportEvtStatus(
            zclIncoming_t *pInMsg,
            zclSE_EnergyMgmtReportEvtStatusCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_EnergyMgmtReportEvtStatus_t *pCmd;

    pCmd = OsalPort_malloc( sizeof( zclSE_EnergyMgmtReportEvtStatus_t ) );
    if ( pCmd == NULL )
    {
       return ZCL_STATUS_SOFTWARE_FAILURE;
    }

    status = zclSE_EnergyMgmtParseReportEvtStatus( pInMsg, pCmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      pfnCB( pInMsg, pCmd );
    }

    OsalPort_free( pCmd );
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_EnergyMgmtHdlManageEvt
 *
 * @brief   Handle COMMAND_SE_ENERGY_MGMT_MANAGE_EVT.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_EnergyMgmtHdlManageEvt( zclIncoming_t *pInMsg,
                                        zclSE_EnergyMgmtManageEvtCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_EnergyMgmtManageEvt_t cmd;

    status = zclSE_EnergyMgmtParseManageEvt( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      pfnCB( pInMsg, &cmd );

      status = ZCL_STATUS_CMD_HAS_RSP;
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_EnergyMgmtHdlClientCmd
 *
 * @brief   Client command handler for ZCL_CLUSTER_ID_SE_ENERGY_MGMT.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCBs - client callbacks
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_EnergyMgmtHdlClientCmd( zclIncoming_t *pInMsg,
                                        const zclSE_EnergyMgmtClientCBs_t *pCBs )
{
  ZStatus_t status;

  // Guard against NULL pointer
  if ( pCBs == NULL )
  {
    return ZCL_STATUS_FAILURE;
  }

  // Process Server-to-Client Commands
  switch ( pInMsg->hdr.commandID )
  {
    case COMMAND_SE_ENERGY_MGMT_REPORT_EVT_STATUS:
      status = zclSE_EnergyMgmtHdlReportEvtStatus( pInMsg, pCBs->pfnReportEvtStatus );
      break;

    default:
      status = ZCL_STATUS_FAILURE;
      break;
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_EnergyMgmtHdlServerCmd
 *
 * @brief   Server command handler for ZCL_CLUSTER_ID_SE_ENERGY_MGMT.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCBs - server callbacks
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_EnergyMgmtHdlServerCmd( zclIncoming_t *pInMsg,
                                        const zclSE_EnergyMgmtServerCBs_t *pCBs )
{
  ZStatus_t status;

  // Guard against NULL pointer
  if ( pCBs == NULL )
  {
    return ZCL_STATUS_FAILURE;
  }

  // Process Client-to-Server Commands
  switch ( pInMsg->hdr.commandID )
  {
    case COMMAND_SE_ENERGY_MGMT_MANAGE_EVT:
      status = zclSE_EnergyMgmtHdlManageEvt( pInMsg, pCBs->pfnManageEvt );
      break;

    default:
      status = ZCL_STATUS_FAILURE;
      break;
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_MDUPairingSendPairingRsp
 *
 * @brief   Send COMMAND_SE_MDU_PAIRING_PAIRING_RSP.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MDUPairingSendPairingRsp( uint8_t srcEP, afAddrType_t *dstAddr,
                                          zclSE_MDUPairingPairingRsp_t *pCmd,
                                          uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status = ZSuccess;
  uint8_t *pCmdBuf;
  uint16_t cmdBufLen = 0;
  uint8_t *pBuf;

  // Check for a non fragmented, valid "pDevices" field, then calculate length
  if ( ( pCmd->cmdTotal <= 1 ) && ( pCmd->pDevices != NULL ) )
  {
    cmdBufLen = pCmd->numOfDevices * Z_EXTADDR_LEN;
  }
  else if ( pCmd->pRawPayload != NULL )
  {
    // Fragmented -- use raw payload fields
    cmdBufLen = pCmd->rawPayloadLen;
  }

  // Allocate command buffer
  cmdBufLen = ZCL_SE_MDU_PAIRING_PAIRING_RSP_LEN + cmdBufLen;

  pCmdBuf = OsalPort_malloc( cmdBufLen );
  if ( pCmdBuf == NULL )
  {
    return ZMemError;
  }

  pBuf = pCmdBuf;
  pBuf = OsalPort_bufferUint32( pBuf, pCmd->version );
  *pBuf++ = pCmd->numOfDevices;
  *pBuf++ = pCmd->cmdIdx;
  *pBuf++ = pCmd->cmdTotal;

   // Check for a non fragmented, valid "pDevices" field, then serialize
  if ( ( pCmd->cmdTotal <= 1 ) && ( pCmd->pDevices != NULL ) )
  {
    uint8_t device;

    for ( device = 0; device < pCmd->numOfDevices; device++ )
    {
      pBuf = OsalPort_memcpy( pBuf, pCmd->pDevices[device], Z_EXTADDR_LEN );
    }
  }
  else if ( pCmd->pRawPayload )
  {
    // Fragmented -- use raw payload fields
    pBuf = OsalPort_memcpy( pBuf, pCmd->pRawPayload, pCmd->rawPayloadLen );
  }

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_MDU_PAIRING,
                            COMMAND_SE_MDU_PAIRING_PAIRING_RSP, TRUE,
                            ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0,
                            seqNum, cmdBufLen, pCmdBuf );

      OsalPort_free(pCmdBuf);

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_MDUPairingSendPairingReq
 *
 * @brief   Send COMMAND_SE_MDU_PAIRING_PAIRING_REQ.
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - destination address
 * @param   pCmd - command payload
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MDUPairingSendPairingReq( uint8_t srcEP, afAddrType_t *dstAddr,
                                          zclSE_MDUPairingPairingReq_t *pCmd,
                                          uint8_t disableDefaultRsp, uint8_t seqNum )
{
  ZStatus_t status;
  uint8_t cmdBuf[ZCL_SE_MDU_PAIRING_PAIRING_REQ_LEN];
  uint8_t *pBuf = cmdBuf;

  pBuf = OsalPort_bufferUint32( pBuf, pCmd->version );
  OsalPort_memcpy( pBuf, pCmd->extAddr, Z_EXTADDR_LEN );

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SE_MDU_PAIRING,
                            COMMAND_SE_MDU_PAIRING_PAIRING_REQ, TRUE,
                            ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0,
                            seqNum, ZCL_SE_MDU_PAIRING_PAIRING_REQ_LEN, cmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_MDUPairingParsePairingRsp
 *
 * @brief   Parse COMMAND_SE_MDU_PAIRING_PAIRING_RSP.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MDUPairingParsePairingRsp( zclIncoming_t *pInMsg,
                                           zclSE_MDUPairingPairingRsp_t *pCmd )
{
  ZStatus_t status = ZCL_STATUS_SUCCESS;
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_MDU_PAIRING_PAIRING_RSP_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->version = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  pCmd->numOfDevices = *pBuf++;
  pCmd->cmdIdx = *pBuf++;
  pCmd->cmdTotal = *pBuf++;
  pCmd->rawPayloadLen = pInMsg->pDataLen - ZCL_SE_MDU_PAIRING_PAIRING_RSP_LEN;

  if ( pCmd->rawPayloadLen )
  {
    pCmd->pRawPayload = pBuf;
  }
  else
  {
    pCmd->pRawPayload = NULL;
  }

  // Check for fragmentation
  if ( pCmd->cmdTotal <= 1 )
  {
    status = zclSE_MDUPairingDevicesParse( pCmd );
  }
  else
  {
    pCmd->pDevices = NULL;
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_MDUPairingParsePairingReq
 *
 * @brief   Parse COMMAND_SE_MDU_PAIRING_PAIRING_REQ.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MDUPairingParsePairingReq( zclIncoming_t *pInMsg,
                                           zclSE_MDUPairingPairingReq_t *pCmd )
{
  uint8_t *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_SE_MDU_PAIRING_PAIRING_REQ_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->version = OsalPort_buildUint32( pBuf, 4 );
  pBuf += 4;
  OsalPort_memcpy( pCmd->extAddr, pBuf, Z_EXTADDR_LEN );

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_MDUPairingHdlPairingRsp
 *
 * @brief   Handle COMMAND_SE_MDU_PAIRING_PAIRING_RSP.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MDUPairingHdlPairingRsp( zclIncoming_t *pInMsg,
                                         zclSE_MDUPairingPairingRspCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_MDUPairingPairingRsp_t cmd;

    status = zclSE_MDUPairingParsePairingRsp( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      status = pfnCB( pInMsg, &cmd );
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_MDUPairingHdlPairingReq
 *
 * @brief   Handle COMMAND_SE_MDU_PAIRING_PAIRING_REQ.
 *
 * @param   pInMsg - incoming message to process
 * @param   pfnCB - command callback
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MDUPairingHdlPairingReq( zclIncoming_t *pInMsg,
                                         zclSE_MDUPairingPairingReqCB_t pfnCB )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;

  if ( pfnCB )
  {
    zclSE_MDUPairingPairingReq_t cmd;

    status = zclSE_MDUPairingParsePairingReq( pInMsg, &cmd );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      pfnCB( pInMsg, &cmd );

      status = ZCL_STATUS_CMD_HAS_RSP;
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_MDUPairingHdlClientCmd
 *
 * @brief   Client command handler for ZCL_CLUSTER_ID_SE_MDU_PAIRING.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCBs - client callbacks
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MDUPairingHdlClientCmd( zclIncoming_t *pInMsg,
                                        const zclSE_MDUPairingClientCBs_t *pCBs )
{
  ZStatus_t status;

  // Guard against NULL pointer
  if ( pCBs == NULL )
  {
    return ZCL_STATUS_FAILURE;
  }

  // Process Server-to-Client Commands
  switch ( pInMsg->hdr.commandID )
  {
    case COMMAND_SE_MDU_PAIRING_PAIRING_RSP:
      status = zclSE_MDUPairingHdlPairingRsp( pInMsg, pCBs->pfnPairingReq );
      break;

    default:
      status = ZCL_STATUS_FAILURE;
      break;
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_MDUPairingHdlServerCmd
 *
 * @brief   Server command handler for ZCL_CLUSTER_ID_SE_MDU_PAIRING.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCBs - server callbacks
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MDUPairingHdlServerCmd( zclIncoming_t *pInMsg,
                                        const zclSE_MDUPairingServerCBs_t *pCBs )
{
  ZStatus_t status;

  // Guard against NULL pointer
  if ( pCBs == NULL )
  {
    return ZCL_STATUS_FAILURE;
  }

  // Process Client-to-Server Commands
  switch ( pInMsg->hdr.commandID )
  {
    case COMMAND_SE_MDU_PAIRING_PAIRING_REQ:
      status = zclSE_MDUPairingHdlPairingReq( pInMsg, pCBs->pfnPairingReq );
      break;

    default:
      status = ZCL_STATUS_FAILURE;
      break;
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_MDUPairingDevicesParse
 *
 * @brief   Utility function for parsing "pDevices" payload -- see "zclSE_MDUPairingPairingRsp_t"
 *
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_MDUPairingDevicesParse( zclSE_MDUPairingPairingRsp_t *pCmd )
{
  // Initialize in case of failure
  pCmd->pDevices = NULL;

  // Check for valid raw fields
  if ( pCmd->rawPayloadLen && ( pCmd->pRawPayload == NULL ) )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  // Get the number of devices
  pCmd->numOfDevices = pCmd->rawPayloadLen / Z_EXTADDR_LEN;

  // Check for devices
  if ( pCmd->numOfDevices )
  {
    pCmd->pDevices = (void *)pCmd->pRawPayload;
  }

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclSE_HdlSpecificCmd
 *
 * @brief   Standard ZCL SE specific command handler.  This function is safe to call within the
 *          context of zclSE_HdlSpecificCmdHook.
 *
 * @param   pInMsg - incoming message to process
 * @param   pCBs - callbacks
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_HdlSpecificCmd( zclIncoming_t *pInMsg, zclSE_AppCallbacks_t *pCBs )
{
  ZStatus_t status;

  if ( zcl_ServerCmd( pInMsg->hdr.fc.direction ) )
  {
    // Process Client-to-Server Commands
    status = zclSE_HdlSpecificServerCmd( pInMsg, pCBs );
  }
  else
  {
    // Process Server-to-Client Commands
    status = zclSE_HdlSpecificClientCmd( pInMsg, pCBs );
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclSE_HdlAppSpecificCmd
 *
 * @brief   Handles ZCL SE specific commands, first looking up the endpoint callbacks and then
 *          calling zclSE_HdlSpecificCmd.  Application callbacks are registered via
 *          zclSE_RegisterCmdCallbacks.  This function is safe to call within the context of
 *          zclSE_HdlSpecificCmdHook.
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_HdlAppSpecificCmd( zclIncoming_t *pInMsg )
{
  ZStatus_t status = ZCL_STATUS_FAILURE;
  zclSE_AppCallbacks_t *pCBs = NULL;

  // Look for endpoint callbacks
  pCBs = zclSE_FindCallbacks( pInMsg->msg->endPoint );
  if ( pCBs != NULL )
  {
    status = zclSE_HdlSpecificCmd( pInMsg, pCBs );
  }

  return status;
}

#if !defined ( ZCL_SE_HDL_SPECIFIC_CMD_HOOK )
/**************************************************************************************************
 * @fn      zclSE_HdlSpecificCmdHook
 *
 * @brief   Hook to override standard ZCL SE specific command handling, see zclSE_RegisterPlugin.
 *          Implement zclSE_HdlSpecificCmdHook and define ZCL_SE_HDL_SPECIFIC_CMD_HOOK.  Within the
 *          context of zclSE_HdlSpecificCmdHook, any of the zclSE_*Hdl* functions can be called
 *          safely.
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSE_HdlSpecificCmdHook( zclIncoming_t *pInMsg )
{
  return zclSE_HdlAppSpecificCmd( pInMsg );
}
#endif // ZCL_SE_HDL_SPECIFIC_CMD_HOOK

/**************************************************************************************************
 * @fn      zclSE_RegisterPlugin
 *
 * @brief   Register the SE plugin with ZCL.  If the application calls zclSE_RegisterCmdCallbacks,
 *          zclSE_RegisterPlugin will be called internally.  But if the application uses
 *          the ZCL_SE_HDL_SPECIFIC_CMD_HOOK, the application must call zclSE_RegisterPlugin during
 *          application initialization.
 *
 * @param   void
 *
 * @return  void
 */
void zclSE_RegisterPlugin( void )
{
  // Register as a ZCL Plugin
  if ( !zclSE_PluginRegisted )
  {
    zcl_registerPlugin( ZCL_CLUSTER_ID_SE_PRICE,
                        ZCL_CLUSTER_ID_SE_MDU_PAIRING,
                        zclSE_HdlIncoming );

    zclSE_PluginRegisted = TRUE;
  }
}

/**************************************************************************************************
 * @fn      zclSE_RegisterCmdCallbacks
 *
 * @brief   Register application command callbacks.
 *
 * @param   appEP - application endpoint
 * @param   pCBs - callbacks
 *
 * @return  ZStatus_t (ZMemError if not able to allocate)
 */
ZStatus_t zclSE_RegisterCmdCallbacks( uint8_t appEP, zclSE_AppCallbacks_t *pCBs )
{
  zclSE_CBRec_t *pNewRec;
  zclSE_CBRec_t *pSelect;

  // Register as a ZCL Plugin
  zclSE_RegisterPlugin();

  // Fill in the new profile list
  pNewRec = OsalPort_malloc( sizeof( zclSE_CBRec_t ) );
  if ( pNewRec == NULL )
  {
    return ZMemError;
  }

  pNewRec->pNext = (zclSE_CBRec_t *)NULL;
  pNewRec->appEP = appEP;
  pNewRec->pCBs = pCBs;

  // Find spot in list
  if ( zcl_SECBs == NULL )
  {
    zcl_SECBs = pNewRec;
  }
  else
  {
    // Look for end of list
    pSelect = zcl_SECBs;
    while ( pSelect->pNext != NULL )
    {
      pSelect = pSelect->pNext;
    }

    // Put new item at end of list
    pSelect->pNext = pNewRec;
  }

  return ZSuccess;
}


/*********************************************************************
 * @fn      se_buffer_uint24
 *
 * @brief
 *
 *   Buffer an uint24 value - LSB first. Note that type uint24 is
 *   typedef to uint32_t in comdef.h
 *
 * @param   buf - buffer
 * @param   val - uint24 value
 *
 * @return  pointer to end of destination buffer
 */
static uint8_t* se_buffer_uint24( uint8_t *buf, uint24 val )
{
  *buf++ = BREAK_UINT32( val, 0 );
  *buf++ = BREAK_UINT32( val, 1 );
  *buf++ = BREAK_UINT32( val, 2 );

  return buf;
}


/********************************************************************************************
*********************************************************************************************/
