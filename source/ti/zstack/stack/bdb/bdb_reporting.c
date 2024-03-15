/**************************************************************************************************
  Filename:       bdb_Reporting.c
  Revised:        $Date: 2016-02-25 11:51:49 -0700 (Thu, 25 Feb 2016) $
  Revision:       $Revision: - $

  Description:    This file contains the Reporting Attributes functions.


  Copyright 2006-2015 Texas Instruments Incorporated.

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
**************************************************************************************************/
#include "ti_zstack_config.h"

#ifdef BDB_REPORTING
/*********************************************************************
 * INCLUDES
 */

#include "bdb.h"
#include "zcl.h"
#include "zd_object.h"
#include "bdb_reporting.h"
#include "rom_jt_154.h"
#include "zcl_ms.h"
#include "osal_nv.h"
#include "bdb_interface.h"

/*********************************************************************
 * MACROS
 */
#define EQUAL_LLISTITEMDATA( a, b ) ( a.attrID == b.attrID )
#define EQUAL_LLISTCFGATTRITEMDATA( a, b ) ( a.endpoint == b.endpoint &&  a.attrID == b.attrID && a.cluster == b.cluster )
#define FLAGS_TURNOFFALLFLAGS( flags ) ( flags = 0x00 )
#define FLAGS_TURNOFFFLAG( flags, flagMask ) ( flags &= ~flagMask )
#define FLAGS_TURNONFLAG( flags, flagMask ) ( flags |= flagMask )
#define FLAGS_CHECKFLAG( flags, flagMask ) ( (flags & flagMask) > 0? BDBREPORTING_TRUE: BDBREPORTING_FALSE )

 /*********************************************************************
 * CONSTANTS
 */
#define BDBREPORTING_HASBINDING_FLAG_MASK      0x01
#define BDBREPORTING_NONEXTINCREMENT_FLAG_MASK 0x02


#if BDBREPORTING_MAX_ANALOG_ATTR_SIZE == 8
#define BDBREPORTING_DEFAULTCHANGEVALUE {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
#endif
#if BDBREPORTING_MAX_ANALOG_ATTR_SIZE == 4
#define BDBREPORTING_DEFAULTCHANGEVALUE {0x00, 0x00, 0x00, 0x00}
#endif
#if BDBREPORTING_MAX_ANALOG_ATTR_SIZE == 2
#define BDBREPORTING_DEFAULTCHANGEVALUE {0x00, 0x00}
#endif

#define BDBREPORTING_MAXINTERVAL_DEFAULT 0x0000
#define BDBREPORTING_MININTERVAL_DEFAULT 0xFFFF

/*********************************************************************
 * TYPEDEFS
 */
//Data to hold informaation about an attribute in a linked list thats inside
//the cluster-endpoint entry
typedef struct
{
  uint16_t attrID;
  uint8_t  lastValueReported[BDBREPORTING_MAX_ANALOG_ATTR_SIZE];
  uint8_t  reportableChange[BDBREPORTING_MAX_ANALOG_ATTR_SIZE];
} bdbReportAttrLive_t;


//This structrue holds the data of a attribute reporting configiration that
//is used at runtime and it's saved in the NV
typedef struct
{
  uint8_t endpoint;
  uint16_t cluster;
  uint16_t attrID;
  uint16_t minReportInt;
  uint16_t maxReportInt;
  uint16_t defaultMinReportInt;
  uint16_t defaultMaxReportInt;
  uint8_t  reportableChange[BDBREPORTING_MAX_ANALOG_ATTR_SIZE];
  uint8_t  defaultReportableChange[BDBREPORTING_MAX_ANALOG_ATTR_SIZE];
} bdbReportAttrCfgData_t;

//This structure represents a node in the linked list of the attributes
//data in the cluster-endpoint entry
typedef struct bdbLinkedListAttrItem
{
  bdbReportAttrLive_t* data;
  struct bdbLinkedListAttrItem *next;
} bdbLinkedListAttrItem_t;


//This structure represents a linked list of the attributes
//data in the cluster-endpoint entry
typedef struct bdbAttrLinkedListAttr
{
  uint8_t numItems;
  bdbLinkedListAttrItem_t *head;
} bdbAttrLinkedListAttr_t;

// This structure is an entry of a cluster-endpoint table used by the reporting
//code (the consolidated values) to actually report periodically
typedef struct
{
  uint8_t flags;
  uint8_t  endpoint;             // status field
  uint16_t  cluster;          // to send or receive reports of the attribute
  uint16_t  consolidatedMinReportInt;             // attribute ID
  uint16_t  consolidatedMaxReportInt;           // attribute data type
  uint16_t  timeSinceLastReport;
  bdbAttrLinkedListAttr_t attrLinkedList;
} bdbReportAttrClusterEndpoint_t;


//This structure serves to hold the flags data of a bdbReportAttrClusterEndpoint_t
//with key =(endpoint,cluster) in instance of the bdb reporting where the table is regenerated
typedef struct
{
  uint8_t flags;
  uint8_t  endpoint;
  uint16_t  cluster;
} bdbReportFlagsHolder_t;

//This structure holds the data of a default attribute reporting configuration
//entered by the application
typedef struct
{
  uint8_t endpoint;
  uint16_t cluster;
  uint16_t attrID;
  uint16_t minReportInt;
  uint16_t maxReportInt;
  uint8_t  reportableChange[BDBREPORTING_MAX_ANALOG_ATTR_SIZE];
} bdbReportAttrDefaultCfgData_t;

//This structure represents a node in the linked list of the default attributes
//configurations entered by the application
typedef struct bdbRepAttrDefaultCfgRecordLinkedListItem
{
  bdbReportAttrDefaultCfgData_t* data;
  struct bdbRepAttrDefaultCfgRecordLinkedListItem *next;
} bdbRepAttrDefaultCfgRecordLinkedListItem_t;

//This structure represents the linked list of the default attributes
//configurations entered by the application
typedef struct bdbRepAttrDefaultCfgRecordLinkedList
{
  uint8_t numItems;
  bdbRepAttrDefaultCfgRecordLinkedListItem_t *head;
} bdbRepAttrDefaultCfgRecordLinkedList_t;



/*********************************************************************
 * GLOBAL VARIABLES
 */
uint8_t gAttrDataValue[BDBREPORTING_MAX_ANALOG_ATTR_SIZE];

/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */

//Table of cluster-endpoint entries used to report periodically
bdbReportAttrClusterEndpoint_t bdb_reportingClusterEndpointArray[BDB_MAX_CLUSTERENDPOINTS_REPORTING];
//Current size of the cluster-endpoint table
uint8_t bdb_reportingClusterEndpointArrayCount;
//This variable has the timeout value of the currrent timer use to report peridically
uint16_t bdb_reportingNextEventTimeout;
//This variable hasthe index of the cluster-endpoint entry that trigger the current
//timer use to report periodically
uint8_t bdb_reportingNextClusterEndpointIndex;
//This is the table that holds in the memory the attribute reporting configurations (dynamic table)
bdbReportAttrCfgData_t* bdb_reportingAttrCfgRecordsArray;
//Current size of the attribute reporting configurations table
uint8_t bdb_reportingAttrCfgRecordsArrayCount;
//Max size of the attribute reporting configurations table
uint8_t bdb_reportingAttrCfgRecordsArrayMaxSize;
//Linked list for holding the default attribute reporting configurations
//enteres by the application
bdbRepAttrDefaultCfgRecordLinkedList_t attrDefaultCfgRecordLinkedList;
//Flag used to signal when not to accept more default attribute reporting configurations
uint8_t bdb_reportingAcceptDefaultConfs;

/*********************************************************************
 * PUBLIC FUNCTIONS PROTOYPES
 */

/*********************************************************************
 * LOCAL FUNCTIONS PROTOYPES
 */

//Begin: Single linked list for attributes in a cluster-endpoint live entry methods
static void bdb_InitReportAttrLiveValues( bdbReportAttrLive_t* item );
static void bdb_linkedListAttrInit( bdbAttrLinkedListAttr_t *list );
static uint8_t bdb_linkedListAttrAdd( bdbAttrLinkedListAttr_t *list, bdbReportAttrLive_t* data );
static bdbLinkedListAttrItem_t* bdb_linkedListAttrSearch( bdbAttrLinkedListAttr_t *list, bdbReportAttrLive_t* searchdata );
static bdbReportAttrLive_t* bdb_linkedListAttrRemove( bdbAttrLinkedListAttr_t *list );
static uint8_t bdb_linkedListAttrFreeAll( bdbAttrLinkedListAttr_t *list );
static void bdb_linkedListAttrClearList( bdbAttrLinkedListAttr_t *list );
static bdbLinkedListAttrItem_t* bdb_linkedListAttrGetAtIndex( bdbAttrLinkedListAttr_t *list, uint8_t index );
//End: Single Linked List methods

//Begin: Cluster-endpoint array live methods
static void bdb_clusterEndpointArrayInit( void );
static uint8_t bdb_clusterEndpointArrayAdd( uint8_t endpoint, uint16_t cluster, uint16_t consolidatedMinReportInt, uint16_t consolidatedMaxReportInt, uint16_t timeSinceLastReport );
static uint8_t bdb_clusterEndpointArrayGetMin( void );
static void bdb_clusterEndpointArrayMoveTo( uint8_t indexSrc, uint8_t indexDest );
static uint8_t bdb_clusterEndpointArrayUpdateAt( uint8_t index, uint16_t newTimeSinceLastReport, uint8_t markHasBinding, uint8_t noNextIncrement );
static void bdb_clusterEndpointArrayFreeAll( void );
static uint8_t bdb_clusterEndpointArraySearch( uint8_t endpoint, uint16_t cluster );
static uint8_t bdb_clusterEndpointArrayRemoveAt( uint8_t index );
static void bdb_clusterEndpointArrayIncrementAll( uint16_t timeSinceLastReportIncrement, uint8_t CheckNoIncrementFlag );
//End: Cluster-endpoint array live methods

//Begin: Single linked list default attr cfg records methods
static void bdb_repAttrDefaultCfgRecordInitValues( bdbReportAttrDefaultCfgData_t* item );
static void bdb_repAttrDefaultCfgRecordsLinkedListInit( bdbRepAttrDefaultCfgRecordLinkedList_t *list );
static uint8_t bdb_repAttrDefaultCfgRecordsLinkedListAdd( bdbRepAttrDefaultCfgRecordLinkedList_t *list, bdbReportAttrDefaultCfgData_t* data );
static bdbRepAttrDefaultCfgRecordLinkedListItem_t* bdb_repAttrDefaultCfgRecordsLinkedListSearch( bdbRepAttrDefaultCfgRecordLinkedList_t *list,
                                                                                                bdbReportAttrDefaultCfgData_t searchdata );
static bdbReportAttrDefaultCfgData_t* bdb_repAttrDefaultCfgRecordsLinkedListRemove( bdbRepAttrDefaultCfgRecordLinkedList_t *list );
static uint8_t bdb_repAttrDefaultCfgRecordsLinkedListFreeAll( bdbRepAttrDefaultCfgRecordLinkedList_t *list );
//End: Single linked list default attr cfg records methods

//Begin: Reporting attr configuration array methods
static void bdb_repAttrCfgRecordsArrayInit( void );
static uint8_t bdb_repAttrCfgRecordsArrayCreate( uint8_t maxNumRepAttrConfRecords );
static uint8_t bdb_repAttrCfgRecordsArrayAdd( uint8_t endpoint, uint16_t cluster, uint16_t attrID, uint16_t minReportInt,
                                           uint16_t maxReportInt, uint8_t  reportableChange[], uint16_t defMinReportInt, uint16_t defMaxReportInt, uint8_t defReportChange[] );
static void bdb_repAttrCfgRecordsArrayFreeAll( void );
static uint8_t bdb_repAttrCfgRecordsArraySearch( uint8_t endpoint, uint16_t cluster, uint16_t attrID );
static uint8_t bdb_repAttrCfgRecordsArrayConsolidateValues( uint8_t endpoint, uint16_t cluster,  uint16_t* consolidatedMinReportInt, uint16_t* consolidatedMaxReportInt );
//End: Reporting attr configuration array methods


static uint8_t bdb_repAttrBuildClusterEndPointArrayBasedOnConfRecordsArray( void );
static uint8_t bdb_RepConstructAttrCfgArray( void );
static void bdb_RepInitAttrCfgRecords( void );

static endPointDesc_t* bdb_FindEpDesc( uint8_t endPoint );
static uint8_t bdb_RepFindAttrEntry( uint8_t endpoint, uint16_t cluster, uint16_t attrID, zclAttribute_t* attrRes );
static uint8_t bdb_RepLoadCfgRecords( void );
static uint8_t bdb_isAttrValueChangedSurpassDelta( uint8_t datatype, uint8_t* delta, uint8_t* curValue, uint8_t* lastValue );
static uint16_t bdb_RepCalculateEventElapsedTime( uint32_t remainingTimeoutTimer, uint16_t nextEventTimeout );
static void bdb_RepRestartNextEventTimer( void );

static void bdb_RepStartReporting( void );
static void bdb_RepStopEventTimer( void );
static void bdb_RepSetupReporting( void );
static void bdb_RepReport( uint8_t indexClusterEndpoint );

extern zclAttrRecsList *zclFindAttrRecsList( uint8_t endpoint ); //Definition is located in zcl.h

/*********************************************************************
 * PUBLIC FUNCTIONS DEFINITIONS
 */

 /*********************************************************************
 * @fn          bdb_RepInit
 *
 * @brief       Initiates the tables and linked list used in the reporting code.
 *
 * @param       none
 *
 * @return      none
 */
void bdb_RepInit( void )
{
  bdb_reportingNextEventTimeout = 0;
  bdb_reportingAcceptDefaultConfs = BDBREPORTING_TRUE;
  bdb_repAttrCfgRecordsArrayInit( );
  bdb_repAttrDefaultCfgRecordsLinkedListInit( &attrDefaultCfgRecordLinkedList );
  bdb_clusterEndpointArrayInit( );
}

 /*********************************************************************
 * @fn          bdb_RepConstructReportingData
 *
 * @brief       Creates the attr reporting configurations by looking at
 *              the app endpoints, cluster and attr definitions or loads
 *              from NV the previous configurations.
 *
 * @param       none
 *
 * @return      none
 */
void bdb_RepConstructReportingData( void )
{
  //Don't accept anymore default attribute configurations entries
  bdb_reportingAcceptDefaultConfs = BDBREPORTING_FALSE;
  //Construct the attr cfg records
  bdb_RepInitAttrCfgRecords( );
  //Construct the endpoint-cluster array
  bdb_repAttrBuildClusterEndPointArrayBasedOnConfRecordsArray( );
  //Delete reporting configuration array, it's saved in NV
  bdb_repAttrCfgRecordsArrayFreeAll( );
}

 /*********************************************************************
 * @fn          bdb_RepMarkHasBindingInEndpointClusterArray
 *
 * @brief       Marks the binding flag as ON at the entry containig the
 *              cluster-endpoint pair.
 *
 * @param       endpoint - endpoint id of the entry to locate
 * @param       cluster - cluster id of the entry to locate
 *
 * @return      none
 */
void bdb_RepMarkHasBindingInEndpointClusterArray( uint8_t endpoint, uint16_t cluster, uint8_t unMark, uint8_t setNoNextIncrementFlag )
{
  uint8_t foundIndex;
  if( bdb_reportingClusterEndpointArrayCount > 0 )
  {
    foundIndex = bdb_clusterEndpointArraySearch( endpoint, cluster );
    if( foundIndex != BDBREPORTING_INVALIDINDEX )
    {
      if( unMark == BDBREPORTING_TRUE )
      {
        bdb_clusterEndpointArrayUpdateAt( foundIndex, 0, BDBREPORTING_FALSE, setNoNextIncrementFlag );
      }
      else
      {
        bdb_clusterEndpointArrayUpdateAt( foundIndex, 0, BDBREPORTING_TRUE, setNoNextIncrementFlag );
      }
    }
  }
}

 /*********************************************************************
 * @fn          bdb_RepStartReporting
 *
 * @brief       Restarts the periodic reporting timer, if the timer was already
 *              running it stops it and to before starting timer sets some state
 *              variables.
 *
 * @return      none
 */
static void bdb_RepStartReporting( void )
{
  //Stop if reporting timer is active
  if( !OsalPortTimers_getTimerTimeout( bdb_TaskID, BDB_REPORT_TIMEOUT ) )
  {
    //timerElapsedTime is zero
    OsalPortTimers_stopTimer( bdb_TaskID, BDB_REPORT_TIMEOUT );
    bdb_reportingNextEventTimeout = 0;
    bdb_reportingNextClusterEndpointIndex = BDBREPORTING_INVALIDINDEX;
    //Start Timer
    bdb_RepRestartNextEventTimer( );
  }
}

 /*********************************************************************
 * @fn          bdb_RepStartOrContinueReporting
 *
 * @brief       Restarts the periodic reporting timer, if the timer was already
 *              running it calculates the remaining time of timer before stopping it,
 *              then sustracts this elapsed time from the next event endpoint-cluster
 *              table.
 *
 * @return      none
 */
void bdb_RepStartOrContinueReporting( void )
{
  //Stop if reporting timer is active
  uint32_t remainingTimeOfEvent = OsalPortTimers_getTimerTimeout( bdb_TaskID, BDB_REPORT_TIMEOUT );
  if( remainingTimeOfEvent == 0 )
  {
    //Timer was not running
    bdb_RepStartReporting( );
  }
  else
  {
    uint16_t elapsedTime = bdb_RepCalculateEventElapsedTime( remainingTimeOfEvent, bdb_reportingNextEventTimeout );
    bdb_RepStopEventTimer( );

    bdb_clusterEndpointArrayIncrementAll( elapsedTime, BDBREPORTING_TRUE );
    bdb_RepStartReporting( );
  }

}

 /*********************************************************************
 * @fn          bdb_RepCalculateEventElapsedTime
 *
 * @brief       Calculate the elapsed time of the currently running timer,
 *              the remaining time is roundup.
 *
 * @param       remainingTimeoutTimer - timeout value from the OsalPortTimers_getTimerTimeout method,
 *              its in milliseconds units
 * @param       nextEventTimeout - the timeout given to the timer when it started
 *
 * @return      the elapsed time in seconds
 */
static uint16_t bdb_RepCalculateEventElapsedTime( uint32_t remainingTimeoutTimer, uint16_t nextEventTimeout )
{
  uint32_t passTimeOfEvent = 0;
  passTimeOfEvent = nextEventTimeout*1000 >= remainingTimeoutTimer? nextEventTimeout*1000 - remainingTimeoutTimer: 0;
  uint16_t elapsedTime = passTimeOfEvent / 1000;
  elapsedTime = elapsedTime + ((passTimeOfEvent % 1000) >0 ? 1:0); //roundup
  return elapsedTime;
}

 /*********************************************************************
 * @fn          bdb_RepProcessEvent
 *
 * @brief       Method that process the timer expired event in the reporting
 *              code, it calculate the next cluster-endpoint entry based
 *              on the minimum with consolidatedMaxReportInt - timeSinceLastReport,
 *              updates timeSinceLastReport of all entries. If the minimum is zero,
 *              report the cluster-endpoint attrs.
 *
 * @return      none
 */
void bdb_RepProcessEvent( void )
{
  bdb_clusterEndpointArrayIncrementAll( bdb_reportingNextEventTimeout, BDBREPORTING_FALSE );
  uint8_t minIndex =  bdb_clusterEndpointArrayGetMin( );
  if( minIndex == BDBREPORTING_INVALIDINDEX )
  {
    return;
  }
   uint16_t minVal = bdb_reportingClusterEndpointArray[minIndex].consolidatedMaxReportInt - bdb_reportingClusterEndpointArray[minIndex].timeSinceLastReport;
   if( minVal>0 )
   {
     bdb_reportingNextEventTimeout = minVal;
   }
   else
   {
     //Something was triggered, report clusterEndpoint with minIndex
     bdb_reportingNextClusterEndpointIndex = minIndex;
     bdb_RepReport( BDBREPORTING_INVALIDINDEX );
     bdb_clusterEndpointArrayUpdateAt( minIndex, 0, BDBREPORTING_IGNORE, BDBREPORTING_IGNORE );
     bdb_reportingNextEventTimeout = 0;
   }
   bdb_RepRestartNextEventTimer( );
}

/*********************************************************************
 * @fn      bdb_ProcessInConfigReportCmd
 *
 * @brief   Process the "Profile" Configure Reporting Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  ZStatus_t
 */
ZStatus_t bdb_ProcessInConfigReportCmd( zclIncomingMsg_t *pInMsg )
{
  zclCfgReportCmd_t *cfgReportCmd;
  zclCfgReportRec_t *reportRec;
  zclCfgReportRspCmd_t *cfgReportRspCmd;
  zclAttrRec_t attrRec;
  uint8_t status = ZCL_STATUS_SUCCESS;
  uint8_t i;
  uint8_t iNumRspRecords;

  // Find Ep Descriptor
  endPointDesc_t* epDescriptor = bdb_FindEpDesc( pInMsg->endPoint );
  if( epDescriptor == NULL )
  {
    return ( ZInvalidParameter );
  }

  // get a pointer to the report configuration record
  cfgReportCmd = (zclCfgReportCmd_t *)pInMsg->attrCmd;

  if( cfgReportCmd->numAttr == 0 )
  {
    return ( ZInvalidParameter );
  }

  // Allocate space for the response command
  cfgReportRspCmd = (zclCfgReportRspCmd_t *)OsalPort_malloc( sizeof ( zclCfgReportRspCmd_t ) +
                                                            ( cfgReportCmd->numAttr * sizeof ( zclCfgReportStatus_t) ) );
  if ( cfgReportRspCmd == NULL )
  {
    return ( ZMemError );
  }

  //stop any attribute reporting
  bdb_RepStopEventTimer( );

  //Load cfg records from NV
  status = bdb_RepLoadCfgRecords( );
  if( status != BDBREPORTING_SUCCESS )
  {
    OsalPort_free( cfgReportRspCmd );
    return ( ZMemError );
  }

  // Process each Attribute Reporting Configuration record
  uint8_t confchanged = BDBREPORTING_FALSE;
  iNumRspRecords = 0;
  for ( i = 0; i < cfgReportCmd->numAttr; i++ )
  {
    reportRec = &(cfgReportCmd->attrList[i]);

    status = ZCL_STATUS_SUCCESS;  // assume success for this rsp record

    uint8_t atrrCfgRecordIndex =  bdb_repAttrCfgRecordsArraySearch( pInMsg->endPoint, pInMsg->clusterId, reportRec->attrID );
    uint8_t status2 = zclFindAttrRec( pInMsg->endPoint, pInMsg->clusterId, reportRec->attrID, &attrRec );
    if( atrrCfgRecordIndex == BDBREPORTING_INVALIDINDEX || status2 == 0 )
    {
      //No cfg record found,
      status = ZCL_STATUS_INVALID_VALUE;
    }
    else
    {
      if ( reportRec->direction == ZCL_SEND_ATTR_REPORTS )
      {
        if ( reportRec->dataType == attrRec.attr.dataType )
        {
          // This the attribute that is to be reported, for now pass all attrs
          if ( attrRec.attr.accessControl & ACCESS_REPORTABLE )
          {
            if ( reportRec->minReportInt == BDBREPORTING_MININTERVAL_DEFAULT && reportRec->maxReportInt == BDBREPORTING_MAXINTERVAL_DEFAULT )
            {
              //Set the saved default configuration
              confchanged = BDBREPORTING_TRUE;
              bdb_reportingAttrCfgRecordsArray[atrrCfgRecordIndex].minReportInt = bdb_reportingAttrCfgRecordsArray[atrrCfgRecordIndex].defaultMinReportInt;
              bdb_reportingAttrCfgRecordsArray[atrrCfgRecordIndex].maxReportInt = bdb_reportingAttrCfgRecordsArray[atrrCfgRecordIndex].defaultMaxReportInt;
              memset( bdb_reportingAttrCfgRecordsArray[atrrCfgRecordIndex].reportableChange, 0x00, BDBREPORTING_MAX_ANALOG_ATTR_SIZE );
              OsalPort_memcpy( bdb_reportingAttrCfgRecordsArray[atrrCfgRecordIndex].reportableChange, bdb_reportingAttrCfgRecordsArray[atrrCfgRecordIndex].defaultReportableChange, BDBREPORTING_MAX_ANALOG_ATTR_SIZE );
              status = ZCL_STATUS_SUCCESS;
            }
            else
            {
              // valid configuration, change values
              confchanged = BDBREPORTING_TRUE;
              bdb_reportingAttrCfgRecordsArray[atrrCfgRecordIndex].minReportInt = reportRec->minReportInt;
              bdb_reportingAttrCfgRecordsArray[atrrCfgRecordIndex].maxReportInt = reportRec->maxReportInt;
              // For attributes of 'discrete' data types this field is omitted
              if ( zclAnalogDataType( reportRec->dataType ) )
              {
                memset( bdb_reportingAttrCfgRecordsArray[atrrCfgRecordIndex].reportableChange, 0x00, BDBREPORTING_MAX_ANALOG_ATTR_SIZE );
                OsalPort_memcpy( bdb_reportingAttrCfgRecordsArray[atrrCfgRecordIndex].reportableChange, reportRec->reportableChange, zclGetDataTypeLength( reportRec->dataType ) );
              }
              status = ZCL_STATUS_SUCCESS;
            }
          }
          else
          {
            // Attribute cannot be reported
            status = ZCL_STATUS_UNREPORTABLE_ATTRIBUTE;
          }
        }
        else
        {
          // Attribute data type is incorrect
          status = ZCL_STATUS_INVALID_DATA_TYPE;
        }
      }
      // receiving reports
      else
      {
        status = ZCL_STATUS_SUCCESS;
      }

    }

    // If not successful then record the status
    if ( status != ZCL_STATUS_SUCCESS )
    {
      cfgReportRspCmd->attrList[iNumRspRecords].status = status;
      cfgReportRspCmd->attrList[iNumRspRecords].direction = reportRec->direction;
      cfgReportRspCmd->attrList[iNumRspRecords].attrID = reportRec->attrID;
      ++iNumRspRecords;
    }

  } // going through each attribute

  if( confchanged == BDBREPORTING_TRUE )
  {
   //Write new configs into NV
   status = osal_nv_item_init( ZCD_NV_BDBREPORTINGCONFIG, sizeof(bdbReportAttrCfgData_t)*bdb_reportingAttrCfgRecordsArrayCount, bdb_reportingAttrCfgRecordsArray );
   if( status == SUCCESS )
   {
     //Overwrite values
     osal_nv_write( ZCD_NV_BDBREPORTINGCONFIG, sizeof(bdbReportAttrCfgData_t)*bdb_reportingAttrCfgRecordsArrayCount, bdb_reportingAttrCfgRecordsArray );
   }

    bdb_RepSetupReporting( );
  }

  // if no response records, then just say 1 with status of success
  cfgReportRspCmd->numAttr = iNumRspRecords;
  if ( cfgReportRspCmd->numAttr == 0 )
  {
    // Since all attributes were configured successfully, include a single
    // attribute status record in the response command with the status field
    // set to SUCCESS and the attribute ID field and direction omitted.
    cfgReportRspCmd->attrList[0].status = ZCL_STATUS_SUCCESS;
    cfgReportRspCmd->numAttr = 1;
  }

  // Send the response back
  zcl_StackSendConfigReportRspCmd( pInMsg->endPoint, &(pInMsg->srcAddr),
                              pInMsg->clusterId, cfgReportRspCmd, ZCL_FRAME_SERVER_CLIENT_DIR,
                              true, pInMsg->zclHdr.transSeqNum );
  OsalPort_free( cfgReportRspCmd );

  bdb_repAttrCfgRecordsArrayFreeAll( ); //Free reporting conf array from memory, its saved in NV

  bdb_RepStartReporting( );

  return ( ZSuccess ) ;
}


/*********************************************************************
 * @fn      bdb_ProcessInReadReportCfgCmd
 *
 * @brief   Process the "Profile" Read Reporting Configuration Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  ZStatus_t
 */
ZStatus_t bdb_ProcessInReadReportCfgCmd( zclIncomingMsg_t *pInMsg )
{
  zclReadReportCfgCmd_t *readReportCfgCmd;
  zclReadReportCfgRspCmd_t *readReportCfgRspCmd;
  zclReportCfgRspRec_t *reportRspRec;
  uint8_t hdrLen;
  uint8_t dataLen = 0;
  zclAttrRec_t attrRec;
  uint8_t i;
  uint8_t reportChangeLen;
  uint8_t status;

  // Find Ep Descriptor
  endPointDesc_t* epDescriptor = bdb_FindEpDesc( pInMsg->endPoint );
  if( epDescriptor==NULL )
  {
    return ( ZInvalidParameter );
  }

  readReportCfgCmd = (zclReadReportCfgCmd_t *)pInMsg->attrCmd;

  // Find out the response length (Reportable Change field is of variable length)
  for ( i = 0; i < readReportCfgCmd->numAttr; i++ )
  {
    // For supported attributes with 'analog' data type, find out the length of
    // the Reportable Change field
    if ( zclFindAttrRec( epDescriptor->endPoint, pInMsg->clusterId,
                         readReportCfgCmd->attrList[i].attrID, &attrRec ) )
    {
      if ( zclAnalogDataType( attrRec.attr.dataType ) )
      {
         reportChangeLen = zclGetDataTypeLength( attrRec.attr.dataType );

         dataLen += reportChangeLen;
      }
    }
  }

  hdrLen = sizeof( zclReadReportCfgRspCmd_t ) + ( readReportCfgCmd->numAttr * sizeof( zclReportCfgRspRec_t ) );

  // Allocate space for the response command
  readReportCfgRspCmd = (zclReadReportCfgRspCmd_t *)OsalPort_malloc( hdrLen + dataLen );
  if ( readReportCfgRspCmd == NULL )
  {
    return ( ZMemError );
  }

  //Load cfg records from NV
  status = bdb_RepLoadCfgRecords( );
  if( status != BDBREPORTING_SUCCESS )
  {
    OsalPort_free(readReportCfgRspCmd);
    return ( ZMemError );
  }

  readReportCfgRspCmd->numAttr=0;
  for ( i = 0; i < readReportCfgCmd->numAttr; i++ )
  {
    reportRspRec = &(readReportCfgRspCmd->attrList[i]);
    status = ZCL_STATUS_SUCCESS;  // assume success for this rsp record

    uint8_t atrrCfgRecordIndex =  bdb_repAttrCfgRecordsArraySearch( pInMsg->endPoint, pInMsg->clusterId, readReportCfgCmd->attrList[i].attrID );
    uint8_t status2 = zclFindAttrRec( pInMsg->endPoint, pInMsg->clusterId, readReportCfgCmd->attrList[i].attrID, &attrRec );
    if( atrrCfgRecordIndex != BDBREPORTING_INVALIDINDEX && status2 )
    {
      if ( attrRec.attr.accessControl & ACCESS_REPORTABLE )
      {
        // Get the Reporting Configuration
        reportRspRec->dataType = attrRec.attr.dataType;
        reportRspRec->minReportInt = bdb_reportingAttrCfgRecordsArray[atrrCfgRecordIndex].minReportInt;
        reportRspRec->maxReportInt = bdb_reportingAttrCfgRecordsArray[atrrCfgRecordIndex].maxReportInt;
        reportRspRec->reportableChange = bdb_reportingAttrCfgRecordsArray[atrrCfgRecordIndex].reportableChange;
      }
      else
      {
        // Attribute not in the Mandatory Reportable Attribute list
        status = ZCL_STATUS_UNREPORTABLE_ATTRIBUTE;
      }
    }
    else
    {
      // Attribute not found
      status = ZCL_STATUS_UNSUPPORTED_ATTRIBUTE;
    }
    reportRspRec->status = status;
    reportRspRec->direction = readReportCfgCmd->attrList[i].direction;
    reportRspRec->attrID = readReportCfgCmd->attrList[i].attrID;
    readReportCfgRspCmd->numAttr++;
  }

  // Send the response back
  zcl_StackSendReadReportCfgRspCmd( pInMsg->endPoint, &(pInMsg->srcAddr),
                               pInMsg->clusterId, readReportCfgRspCmd, ZCL_FRAME_SERVER_CLIENT_DIR,
                               true, pInMsg->zclHdr.transSeqNum );
  OsalPort_free( readReportCfgRspCmd );

  bdb_repAttrCfgRecordsArrayFreeAll( );//Free reporting cfg array from memory, its saved in NV

  return ( ZSuccess );
}


void bdb_RepUpdateMarkBindings( void )
{
  uint8_t numMarkedEntries = 0;
  uint8_t i;
  for(i=0; i<bdb_reportingClusterEndpointArrayCount; i++)
  {
    BindingEntry_t* bEntry = bindFind( bdb_reportingClusterEndpointArray[i].endpoint,bdb_reportingClusterEndpointArray[i].cluster,0 );
    if(bEntry !=  NULL)
    { //Found a binding with the given cluster and endpoint, mark the Endpoint-cluster entry (this activates reporting)
      if( FLAGS_CHECKFLAG( bdb_reportingClusterEndpointArray[i].flags, BDBREPORTING_HASBINDING_FLAG_MASK ) == BDBREPORTING_FALSE )
      {
        bdb_RepMarkHasBindingInEndpointClusterArray( bdb_reportingClusterEndpointArray[i].endpoint, bdb_reportingClusterEndpointArray[i].cluster, BDBREPORTING_FALSE, BDBREPORTING_IGNORE );
      }
      numMarkedEntries++;
    }
    else
    {
      if( FLAGS_CHECKFLAG( bdb_reportingClusterEndpointArray[i].flags, BDBREPORTING_HASBINDING_FLAG_MASK) == BDBREPORTING_TRUE )
      {
        bdb_RepMarkHasBindingInEndpointClusterArray( bdb_reportingClusterEndpointArray[i].endpoint, bdb_reportingClusterEndpointArray[i].cluster, BDBREPORTING_TRUE, BDBREPORTING_IGNORE );
      }
    }
  }

  //Checking is bdb_reporting timer is active
  if( OsalPortTimers_getTimerTimeout( bdb_TaskID, BDB_REPORT_TIMEOUT) > 0 )
  {
    //If timer is active
    if( numMarkedEntries == 0 ) //No entries
    {
      //Stop Timer
      OsalPortTimers_stopTimer( bdb_TaskID, BDB_REPORT_TIMEOUT );
    }
  }
  else
  {
    if( numMarkedEntries > 0 )
    {
      //Start timer
      bdb_RepStartReporting( );
    }
  }
}

/*********************************************************************
 * LOCAL FUNCTIONS DEFINITIONS
 */

/*
* Begin: Single linked list for attributes in a cluster-endpoint live entry methods
*/

/*********************************************************************
 * @fn      bdb_InitReportAttrLiveValues
 *
 * @brief   Set the bdbReportAttrLive_t fields to initiation values
 *
 * @param   item - Data to initiate
 *
 * @return
 */
static void bdb_InitReportAttrLiveValues( bdbReportAttrLive_t* item )
{
  uint8_t i;
  for( i=0; i<BDBREPORTING_MAX_ANALOG_ATTR_SIZE; i++ )
  {
    item->lastValueReported[i] = 0x00;
    item->reportableChange[i] = 0x00;
  }
  item->attrID = 0x0000;

}

/*********************************************************************
 * @fn      bdb_linkedListAttrInit
 *
 * @brief   Initates a linked list for the attrs in the cluster-endpoint entry
 *
 * @param   list - Pointer to linked list
 *
 * @return
 */
static void bdb_linkedListAttrInit( bdbAttrLinkedListAttr_t *list )
{
  list->head = NULL;
  list->numItems = 0;
}

/*********************************************************************
 * @fn      bdb_linkedListAttrAdd
 *
 * @brief   Initates a linked list for the attrs in the cluster-endpoint entry
 *
 * @param   list - Pointer to linked list
 *
 * @return  Status code (BDBREPORTING_SUCCESS or BDBREPORTING_ERROR)
 */
static uint8_t bdb_linkedListAttrAdd( bdbAttrLinkedListAttr_t *list, bdbReportAttrLive_t* data )
{
  uint8_t i;
  bdbLinkedListAttrItem_t* tmpItem = list->head;
  bdbLinkedListAttrItem_t* newItem;

  if (tmpItem != NULL )
  {
    for (i=0; i<list->numItems; i++)
    {
      // Don't add duplicate attribute
      if (tmpItem->data->attrID == data->attrID)
      {
        return BDBREPORTING_ERROR;
      }
      tmpItem = tmpItem->next;
    }
  }

  newItem = (bdbLinkedListAttrItem_t *)OsalPort_malloc( sizeof(bdbLinkedListAttrItem_t) );
  if( newItem == NULL )
  {
    return BDBREPORTING_ERROR;
  }
  newItem->data = data;
  newItem->next = list->head;
  list->head = newItem;
  list->numItems++;
  return BDBREPORTING_SUCCESS;
}

/*********************************************************************
 * @fn      bdb_linkedListAttrSearch
 *
 * @brief   Travers the linked list and search for a node (bdbReportAttrLive_t
 *          data) with a specific attrID
 *
 * @param   list - Pointer to linked list
 * @param   searchdata - data to search the list (has a specific attrID)
 *
 * @return  A pointer to the node in the list has the searched data, NULL if
 *          not found
 */
static bdbLinkedListAttrItem_t* bdb_linkedListAttrSearch( bdbAttrLinkedListAttr_t *list, bdbReportAttrLive_t* searchdata )
{
  bdbLinkedListAttrItem_t* cur = list->head;
  while( cur != NULL )
  {
    if( EQUAL_LLISTITEMDATA( (*(cur->data)), (*searchdata) ) )
    {
      return cur;
    }
    cur = cur->next;
  }
  return NULL;
}

/*********************************************************************
 * @fn      bdb_linkedListAttrRemove
 *
 * @brief   Remove the head node from the list
 *
 * @param   list - Pointer to linked list
 *
 * @return  A pointer to the data part of the deleted node, NULL if no node was deleted
 */
static bdbReportAttrLive_t* bdb_linkedListAttrRemove( bdbAttrLinkedListAttr_t *list )
{
  bdbReportAttrLive_t* resdata = NULL;
  bdbLinkedListAttrItem_t* cur = list->head;
  if( list->head == NULL )
  {
    return NULL;
  }
  list->head = cur->next;
  resdata =cur->data;
  OsalPort_free( cur );
  list->numItems--;
  return resdata;
}

/*********************************************************************
 * @fn      bdb_linkedListAttrFreeAll
 *
 * @brief   Deletes and deallocates all the memory from the linked list
 *
 * @param   list - Pointer to linked list
 *
 * @return  BDBREPORTING_SUCCESS if operation was successful
 */
static uint8_t bdb_linkedListAttrFreeAll( bdbAttrLinkedListAttr_t *list )
{
  bdbReportAttrLive_t* toremovedata;
  while( list->head != NULL  )
  {
    toremovedata = bdb_linkedListAttrRemove( list );
    OsalPort_free( toremovedata );
  }
  return BDBREPORTING_SUCCESS;
}

/*********************************************************************
 * @fn      bdb_linkedListAttrClearList
 *
 * @brief   Clears the list without freeing the nodes memory
 *
 * @param   list - Pointer to linked list
 *
 * @return
 */
static void bdb_linkedListAttrClearList( bdbAttrLinkedListAttr_t *list )
{
  list->head = NULL;
  list->numItems = 0;
}

/*********************************************************************
 * @fn      bdb_linkedListAttrGetAtIndex
 *
 * @brief   Returns the ith element of the list starting from the head
 *
 * @param   list - Pointer to linked list
 *
 * @return  A pointer to the ith node element
 */
static bdbLinkedListAttrItem_t* bdb_linkedListAttrGetAtIndex( bdbAttrLinkedListAttr_t *list, uint8_t index )
{
  if( index > list->numItems-1 )
  {
    return NULL;
  }
  bdbLinkedListAttrItem_t* cur = list->head;
  uint8_t i;
  for( i=0; i<=index; i++ )
  {
    if( cur == NULL )
    {
      return NULL;
    }
    if( i < index )
    {
      cur = cur->next;
    }
  }
  return cur;
}

/*
* End: Single linked list for attributes in a cluster-endpoint entry methods
*/


/*
* Begin: Cluster-endpoint array live methods
*/

/*********************************************************************
 * @fn      bdb_clusterEndpointArrayInit
 *
 * @brief   Initiates the clusterEndpoint array variables
 *
 * @return
 */
static void bdb_clusterEndpointArrayInit( void )
{
  bdb_reportingClusterEndpointArrayCount = 0;
}

/*********************************************************************
 * @fn      bdb_clusterEndpointArrayAdd
 *
 * @brief   Adds a new entry to the clusterEndpoint array
 *
 * @param   endpoint - Endpoint ID of the entry
 * @param   cluster - Cluster ID of the entry
 * @param   consolidatedMinReportInterval - Cluster ID of the entry

 *
 * @return  A pointer to the ith node element
 */
static uint8_t bdb_clusterEndpointArrayAdd( uint8_t endpoint, uint16_t cluster, uint16_t consolidatedMinReportInt, uint16_t consolidatedMaxReportInt, uint16_t timeSinceLastReport )
{
  if( bdb_reportingClusterEndpointArrayCount>=BDB_MAX_CLUSTERENDPOINTS_REPORTING )
  {
    return BDBREPORTING_ERROR;
  }
  bdb_reportingClusterEndpointArray[bdb_reportingClusterEndpointArrayCount].endpoint = endpoint;
  bdb_reportingClusterEndpointArray[bdb_reportingClusterEndpointArrayCount].cluster = cluster;

  bdb_reportingClusterEndpointArray[bdb_reportingClusterEndpointArrayCount].consolidatedMinReportInt = consolidatedMinReportInt;
  bdb_reportingClusterEndpointArray[bdb_reportingClusterEndpointArrayCount].consolidatedMaxReportInt = consolidatedMaxReportInt;
  bdb_reportingClusterEndpointArray[bdb_reportingClusterEndpointArrayCount].timeSinceLastReport = timeSinceLastReport;
  bdb_linkedListAttrInit( &bdb_reportingClusterEndpointArray[bdb_reportingClusterEndpointArrayCount].attrLinkedList );
  FLAGS_TURNOFFALLFLAGS( bdb_reportingClusterEndpointArray[bdb_reportingClusterEndpointArrayCount].flags );

  bdb_reportingClusterEndpointArrayCount++;
  return BDBREPORTING_SUCCESS;
}

static uint8_t bdb_clusterEndpointArrayGetMin( void )
{
  uint8_t indexMin = 0xFF;
  uint16_t ValueMin = 0xFFFF;
  uint16_t possibleMin;
  uint8_t i;
  for( i=0; i<bdb_reportingClusterEndpointArrayCount; i++ )
  {
    if( FLAGS_CHECKFLAG( bdb_reportingClusterEndpointArray[i].flags, BDBREPORTING_HASBINDING_FLAG_MASK ) == BDBREPORTING_TRUE )
    { //Only do with valid entries (HasBinding==true)
      if( bdb_reportingClusterEndpointArray[i].consolidatedMaxReportInt != BDBREPORTING_NOPERIODIC &&
         bdb_reportingClusterEndpointArray[i].consolidatedMaxReportInt != BDBREPORTING_REPORTOFF )
      {
        //If maxInterval is BDBREPORTING_NOPERIODIC=0x0000 or BDBREPORTING_REPORTOFF=0xFFFF, ignore to calculate min
        if( ValueMin == 0 )
        {
          //stop if we find a minValue of zero because there no other Min less than that
          break;
        }
        possibleMin = bdb_reportingClusterEndpointArray[i].consolidatedMaxReportInt - bdb_reportingClusterEndpointArray[i].timeSinceLastReport;
        if( possibleMin<ValueMin )
        {
          indexMin = i;
          ValueMin = possibleMin;
        }
      }
    }
  }
  return indexMin;

}

static uint8_t bdb_clusterEndpointArrayRemoveAt( uint8_t index )
{
  if( index>=bdb_reportingClusterEndpointArrayCount )
  {
    return BDBREPORTING_ERROR;
  }
  //Freeing list, all the other fields are not dynamic
  bdb_linkedListAttrFreeAll( &bdb_reportingClusterEndpointArray[index].attrLinkedList );
  //moving last element to free slot
  bdb_clusterEndpointArrayMoveTo( index, bdb_reportingClusterEndpointArrayCount-1 );
  bdb_reportingClusterEndpointArrayCount--;
  return BDBREPORTING_SUCCESS;
}

static void bdb_clusterEndpointArrayMoveTo( uint8_t indexSrc, uint8_t indexDest )
{
  bdb_reportingClusterEndpointArray[indexSrc].cluster = bdb_reportingClusterEndpointArray[indexDest].cluster;
  bdb_reportingClusterEndpointArray[indexSrc].endpoint = bdb_reportingClusterEndpointArray[indexDest].endpoint;
  bdb_reportingClusterEndpointArray[indexSrc].consolidatedMaxReportInt = bdb_reportingClusterEndpointArray[indexDest].consolidatedMaxReportInt;
  bdb_reportingClusterEndpointArray[indexSrc].consolidatedMinReportInt = bdb_reportingClusterEndpointArray[indexDest].consolidatedMinReportInt;
  bdb_reportingClusterEndpointArray[indexSrc].timeSinceLastReport = bdb_reportingClusterEndpointArray[indexDest].timeSinceLastReport;
  bdb_reportingClusterEndpointArray[indexSrc].attrLinkedList = bdb_reportingClusterEndpointArray[indexDest].attrLinkedList;
  bdb_reportingClusterEndpointArray[indexSrc].flags = bdb_reportingClusterEndpointArray[indexDest].flags;
  bdb_linkedListAttrClearList( &bdb_reportingClusterEndpointArray[indexDest].attrLinkedList );
}

static uint8_t bdb_clusterEndpointArrayUpdateAt( uint8_t index, uint16_t newTimeSinceLastReport, uint8_t markHasBinding, uint8_t markNoNextIncrement )
{
  if( index >= bdb_reportingClusterEndpointArrayCount )
  {
    return BDBREPORTING_ERROR;
  }
  bdb_reportingClusterEndpointArray[index].timeSinceLastReport = newTimeSinceLastReport;
  if( markHasBinding != BDBREPORTING_IGNORE )
  {
    if( markHasBinding == BDBREPORTING_TRUE )
    {
      FLAGS_TURNONFLAG( bdb_reportingClusterEndpointArray[index].flags, BDBREPORTING_HASBINDING_FLAG_MASK );
    }
    else
    {
      FLAGS_TURNOFFFLAG( bdb_reportingClusterEndpointArray[index].flags, BDBREPORTING_HASBINDING_FLAG_MASK );
    }
  }
  if( markNoNextIncrement != BDBREPORTING_IGNORE )
  {
    if( markNoNextIncrement == BDBREPORTING_TRUE )
    {
      FLAGS_TURNONFLAG( bdb_reportingClusterEndpointArray[index].flags, BDBREPORTING_NONEXTINCREMENT_FLAG_MASK );
    }
    else
    {
      FLAGS_TURNOFFFLAG( bdb_reportingClusterEndpointArray[index].flags, BDBREPORTING_NONEXTINCREMENT_FLAG_MASK );
    }
  }
  return BDBREPORTING_SUCCESS;
}

static void bdb_clusterEndpointArrayFreeAll( )
{
  uint8_t i;
  uint8_t numElements = bdb_reportingClusterEndpointArrayCount;
  for( i=0; i<numElements; i++ )
  {
    bdb_clusterEndpointArrayRemoveAt( 0 );
  }
}

static uint8_t bdb_clusterEndpointArraySearch( uint8_t endpoint, uint16_t cluster )
{
  uint8_t i;
  uint8_t foundIndex = BDBREPORTING_INVALIDINDEX;
  for( i=0; i<bdb_reportingClusterEndpointArrayCount; i++ )
  {
    if( bdb_reportingClusterEndpointArray[i].endpoint == endpoint && bdb_reportingClusterEndpointArray[i].cluster == cluster )
    {
      foundIndex = i;
      break;
    }
  }
  return foundIndex;
}

static void bdb_clusterEndpointArrayIncrementAll( uint16_t timeSinceLastReportIncrement, uint8_t CheckNoIncrementFlag )
{
  uint8_t i;
  uint8_t doIncrement;
  for( i=0; i<bdb_reportingClusterEndpointArrayCount; i++ )
  {
    doIncrement = BDBREPORTING_FALSE;
    if( FLAGS_CHECKFLAG( bdb_reportingClusterEndpointArray[i].flags, BDBREPORTING_HASBINDING_FLAG_MASK ) == BDBREPORTING_TRUE )
    {
      //Only do with valid entries (HasBinding==true)
      if( CheckNoIncrementFlag == BDBREPORTING_FALSE )
      {
        doIncrement = BDBREPORTING_TRUE;
      }
      else
      {
        if( FLAGS_CHECKFLAG( bdb_reportingClusterEndpointArray[i].flags, BDBREPORTING_NONEXTINCREMENT_FLAG_MASK ) == BDBREPORTING_FALSE )
        {
          doIncrement = BDBREPORTING_TRUE;
        }
      }
      if( doIncrement == BDBREPORTING_TRUE )
      {
        if( bdb_reportingClusterEndpointArray[i].consolidatedMaxReportInt != BDBREPORTING_NOPERIODIC &&  bdb_reportingClusterEndpointArray[i].consolidatedMaxReportInt != BDBREPORTING_REPORTOFF )
        {
          bdb_reportingClusterEndpointArray[i].timeSinceLastReport = (bdb_reportingClusterEndpointArray[i].timeSinceLastReport+timeSinceLastReportIncrement
                                                                      > bdb_reportingClusterEndpointArray[i].consolidatedMaxReportInt)?
                                                                      bdb_reportingClusterEndpointArray[i].consolidatedMaxReportInt:
                                                                      bdb_reportingClusterEndpointArray[i].timeSinceLastReport+timeSinceLastReportIncrement;
        }
      }
      FLAGS_TURNOFFFLAG( bdb_reportingClusterEndpointArray[i].flags, BDBREPORTING_NONEXTINCREMENT_FLAG_MASK ); //Always turn off, one shot functionality

    }
  }
}

/*
* End: Cluster-endpoint array live data methods
*/


/*
* Begin: Single linked list default attr cfg records methods
*/

static void bdb_repAttrDefaultCfgRecordInitValues( bdbReportAttrDefaultCfgData_t* item )
{
  uint8_t i;
  for( i=0; i<BDBREPORTING_MAX_ANALOG_ATTR_SIZE; i++ )
  {
    item->reportableChange[i] = 0x00;
  }
  item->attrID = 0x0000;
  item->endpoint = 0xFF;
  item->cluster = 0xFFFF;
  item->maxReportInt = 0x0000;
  item->minReportInt = 0x0000;
}

static void bdb_repAttrDefaultCfgRecordsLinkedListInit( bdbRepAttrDefaultCfgRecordLinkedList_t *list )
{
  list->head = NULL;
  list->numItems = 0;
}

static uint8_t bdb_repAttrDefaultCfgRecordsLinkedListAdd( bdbRepAttrDefaultCfgRecordLinkedList_t *list, bdbReportAttrDefaultCfgData_t* data )
{
  bdbRepAttrDefaultCfgRecordLinkedListItem_t* newItem = (bdbRepAttrDefaultCfgRecordLinkedListItem_t *)OsalPort_malloc( sizeof(bdbRepAttrDefaultCfgRecordLinkedListItem_t ) );
  if( newItem == NULL )
  {
    return BDBREPORTING_ERROR;
  }
  newItem->data = data;
  newItem->next = list->head;
  list->head = newItem;
  list->numItems++;
  return BDBREPORTING_SUCCESS;
}

static bdbRepAttrDefaultCfgRecordLinkedListItem_t* bdb_repAttrDefaultCfgRecordsLinkedListSearch( bdbRepAttrDefaultCfgRecordLinkedList_t *list, bdbReportAttrDefaultCfgData_t searchdata )
{
  bdbRepAttrDefaultCfgRecordLinkedListItem_t* cur = list->head;
  while( cur != NULL )
  {
    if( EQUAL_LLISTCFGATTRITEMDATA( (*(cur->data)), searchdata) )
    {
      return cur;
    }
    cur = cur->next;
  }
  return NULL;
}

static bdbReportAttrDefaultCfgData_t* bdb_repAttrDefaultCfgRecordsLinkedListRemove( bdbRepAttrDefaultCfgRecordLinkedList_t *list )
{
  bdbReportAttrDefaultCfgData_t* resdata = NULL;
  bdbRepAttrDefaultCfgRecordLinkedListItem_t* cur = list->head;
  if( list->head == NULL )
  {
    return NULL;
  }
  list->head = cur->next;
  resdata =cur->data;
  OsalPort_free( cur );
  list->numItems--;
  return resdata;
}

static uint8_t bdb_repAttrDefaultCfgRecordsLinkedListFreeAll( bdbRepAttrDefaultCfgRecordLinkedList_t *list )
{
  bdbReportAttrDefaultCfgData_t* toremovedata;
  while( list->head != NULL )
  {
    toremovedata = bdb_repAttrDefaultCfgRecordsLinkedListRemove( list );
    OsalPort_free( toremovedata );
  }
  return BDBREPORTING_SUCCESS;
}

/*
* End: Single linked list default attr cfg records methods
*/


/*
* Begin: Reporting attr configuration array methods
*/

static void bdb_repAttrCfgRecordsArrayInit( void )
{
  bdb_reportingAttrCfgRecordsArray = NULL;
  bdb_reportingAttrCfgRecordsArrayCount = 0;
}

static uint8_t bdb_repAttrCfgRecordsArrayCreate( uint8_t maxNumRepAttrConfRecords )
{
  if( maxNumRepAttrConfRecords==0 )
  {
    return BDBREPORTING_SUCCESS;
  }

  bdb_reportingAttrCfgRecordsArrayMaxSize = maxNumRepAttrConfRecords;
  bdb_reportingAttrCfgRecordsArray= (bdbReportAttrCfgData_t *)OsalPort_malloc( sizeof( bdbReportAttrCfgData_t )*bdb_reportingAttrCfgRecordsArrayMaxSize );
  bdb_reportingAttrCfgRecordsArrayCount = 0;
  if( bdb_reportingAttrCfgRecordsArray==NULL )
  {
    return BDBREPORTING_ERROR;
  }
  return BDBREPORTING_SUCCESS;
}

static uint8_t bdb_repAttrCfgRecordsArrayAdd( uint8_t endpoint, uint16_t cluster, uint16_t attrID, uint16_t minReportInt, uint16_t maxReportInt, uint8_t  reportableChange[],
                                           uint16_t defMinReportInt, uint16_t defMaxReportInt, uint8_t defReportChange[] )
{
  if( bdb_reportingAttrCfgRecordsArray==NULL )
  {
    return BDBREPORTING_ERROR;
  }
  if( bdb_reportingAttrCfgRecordsArrayCount>=bdb_reportingAttrCfgRecordsArrayMaxSize )
  {
    return BDBREPORTING_ERROR;
  }

  bdb_reportingAttrCfgRecordsArray[bdb_reportingAttrCfgRecordsArrayCount].endpoint = endpoint;
  bdb_reportingAttrCfgRecordsArray[bdb_reportingAttrCfgRecordsArrayCount].cluster = cluster;
  bdb_reportingAttrCfgRecordsArray[bdb_reportingAttrCfgRecordsArrayCount].attrID = attrID;
  bdb_reportingAttrCfgRecordsArray[bdb_reportingAttrCfgRecordsArrayCount].minReportInt = minReportInt;
  bdb_reportingAttrCfgRecordsArray[bdb_reportingAttrCfgRecordsArrayCount].maxReportInt = maxReportInt;
  if( reportableChange!=NULL )
  {
    OsalPort_memcpy( bdb_reportingAttrCfgRecordsArray[bdb_reportingAttrCfgRecordsArrayCount].reportableChange, reportableChange, BDBREPORTING_MAX_ANALOG_ATTR_SIZE );
  }
  bdb_reportingAttrCfgRecordsArray[bdb_reportingAttrCfgRecordsArrayCount].defaultMinReportInt = defMinReportInt;
  bdb_reportingAttrCfgRecordsArray[bdb_reportingAttrCfgRecordsArrayCount].defaultMaxReportInt = defMaxReportInt;
  if( defReportChange != NULL )
  {
    OsalPort_memcpy( bdb_reportingAttrCfgRecordsArray[bdb_reportingAttrCfgRecordsArrayCount].defaultReportableChange, defReportChange, BDBREPORTING_MAX_ANALOG_ATTR_SIZE );
  }
  bdb_reportingAttrCfgRecordsArrayCount++;
  return BDBREPORTING_SUCCESS;
}

static void bdb_repAttrCfgRecordsArrayFreeAll( void )
{
  if( bdb_reportingAttrCfgRecordsArray==NULL )
  {
    return;
  }
  OsalPort_free( bdb_reportingAttrCfgRecordsArray );
  bdb_reportingAttrCfgRecordsArrayCount = 0;
  bdb_reportingAttrCfgRecordsArray=NULL;
}

static uint8_t bdb_repAttrCfgRecordsArraySearch( uint8_t endpoint, uint16_t cluster, uint16_t attrID )
{
  uint8_t i;
  if( bdb_reportingAttrCfgRecordsArray == NULL )
  {
    return BDBREPORTING_INVALIDINDEX;
  }
  for( i=0; i<bdb_reportingAttrCfgRecordsArrayCount; i++ )
  {
    if( bdb_reportingAttrCfgRecordsArray[i].endpoint == endpoint && bdb_reportingAttrCfgRecordsArray[i].cluster == cluster && bdb_reportingAttrCfgRecordsArray[i].attrID == attrID )
    {
      return i;
    }
  }
  return BDBREPORTING_INVALIDINDEX;
}

static uint8_t bdb_repAttrCfgRecordsArrayConsolidateValues( uint8_t endpoint, uint16_t cluster,  uint16_t* consolidatedMinReportInt, uint16_t* consolidatedMaxReportInt )
{
  uint8_t i;
  *consolidatedMinReportInt =0xFFFF;
  *consolidatedMaxReportInt = 0xFFFF;
  uint8_t foundAttr = 0;
  if( bdb_reportingAttrCfgRecordsArray == NULL )
  {
    return BDBREPORTING_ERROR;
  }
  for( i=0; i<bdb_reportingAttrCfgRecordsArrayCount; i++ )
  {
    if( bdb_reportingAttrCfgRecordsArray[i].endpoint == endpoint && bdb_reportingAttrCfgRecordsArray[i].cluster == cluster )
    {
      foundAttr++;
      //Consolidate min value
      if( bdb_reportingAttrCfgRecordsArray[i].minReportInt < *consolidatedMinReportInt )
      {
        *consolidatedMinReportInt = bdb_reportingAttrCfgRecordsArray[i].minReportInt;
      }

      //Consolidate max value
      if( bdb_reportingAttrCfgRecordsArray[i].maxReportInt < *consolidatedMaxReportInt )
      {
        *consolidatedMaxReportInt = bdb_reportingAttrCfgRecordsArray[i].maxReportInt;
      }
    }
  }
  if( foundAttr==0 )
  {
    return BDBREPORTING_ERROR;
  }
  return BDBREPORTING_SUCCESS;
}

/*
* End: Reporting attr configuration array methods
*/


/*
* Begin: Helper methods
*/

static uint8_t bdb_repAttrBuildClusterEndPointArrayBasedOnConfRecordsArray( void )
{
  uint8_t i;
  uint16_t consolidatedMinReportInt =0xFFFF;
  uint16_t consolidatedMaxReportInt = 0xFFFF;
  uint8_t status;
  uint8_t returnStatus = BDBREPORTING_SUCCESS;
  if( bdb_reportingAttrCfgRecordsArray == NULL )
  {
    return BDBREPORTING_ERROR;
  }
  for( i=0; i<bdb_reportingAttrCfgRecordsArrayCount; i++ )
  {
    uint16_t curEndpoint = bdb_reportingAttrCfgRecordsArray[i].endpoint;
    uint16_t curCluster = bdb_reportingAttrCfgRecordsArray[i].cluster;
    //See if there is already a cluster endpoint item
    uint8_t searchedIndex = bdb_clusterEndpointArraySearch( curEndpoint, curCluster );
    if(searchedIndex == BDBREPORTING_INVALIDINDEX)
    {
      //Not found, add entry
      status = bdb_repAttrCfgRecordsArrayConsolidateValues( curEndpoint, curCluster, &consolidatedMinReportInt, &consolidatedMaxReportInt );
      if( status == BDBREPORTING_SUCCESS )
      {
        status = bdb_clusterEndpointArrayAdd( curEndpoint, curCluster, consolidatedMinReportInt, consolidatedMaxReportInt, 0 );
        if( status == BDBREPORTING_SUCCESS )
        {
          zclAttribute_t zclAttribute;
          uint8_t  status;
          //Add attr value
          bdbReportAttrLive_t* newItemData;
          newItemData = (bdbReportAttrLive_t *)OsalPort_malloc( sizeof(bdbReportAttrLive_t) );
          if( newItemData == NULL )
          {
            //Out of memory
            returnStatus = BDBREPORTING_OUTOFMEMORYERROR;
            break;
          }
          bdb_InitReportAttrLiveValues( newItemData );
          newItemData->attrID = bdb_reportingAttrCfgRecordsArray[i].attrID;
          OsalPort_memcpy( newItemData->reportableChange, bdb_reportingAttrCfgRecordsArray[i].reportableChange, BDBREPORTING_MAX_ANALOG_ATTR_SIZE );

          //Read the attribute to keep the table updated
          if(BDBREPORTING_TRUE == bdb_RepFindAttrEntry(curEndpoint,curCluster,newItemData->attrID,&zclAttribute))
          {
            OsalPort_memcpy(newItemData->lastValueReported, zclAttribute.dataPtr,BDBREPORTING_MAX_ANALOG_ATTR_SIZE);
          }

          status = bdb_linkedListAttrAdd( &(bdb_reportingClusterEndpointArray[bdb_reportingClusterEndpointArrayCount-1].attrLinkedList), newItemData );
          if( status == BDBREPORTING_ERROR )
          {
            returnStatus = BDBREPORTING_OUTOFMEMORYERROR;
            break;
          }
        }
        else
        {
          //Out of memory,
          returnStatus = BDBREPORTING_OUTOFMEMORYERROR;
          break;
        }
      }
    }
    else
    {
      zclAttribute_t zclAttribute;
      uint8_t  status;
      //Entry found, just add attr data to linked list
      bdbReportAttrLive_t* newItemData;
      newItemData = (bdbReportAttrLive_t *)OsalPort_malloc( sizeof( bdbReportAttrLive_t ) );
      if( newItemData == NULL )
      {
        returnStatus = BDBREPORTING_OUTOFMEMORYERROR;
        break;
      }
      bdb_InitReportAttrLiveValues( newItemData );
      newItemData->attrID = bdb_reportingAttrCfgRecordsArray[i].attrID;
      OsalPort_memcpy( newItemData->reportableChange, bdb_reportingAttrCfgRecordsArray[i].reportableChange, BDBREPORTING_MAX_ANALOG_ATTR_SIZE );

      //Read the attribute to keep the table updated
      if(BDBREPORTING_TRUE == bdb_RepFindAttrEntry(curEndpoint,curCluster,newItemData->attrID,&zclAttribute))
      {
        OsalPort_memcpy(newItemData->lastValueReported, zclAttribute.dataPtr,BDBREPORTING_MAX_ANALOG_ATTR_SIZE);
      }

      status = bdb_linkedListAttrAdd( &(bdb_reportingClusterEndpointArray[searchedIndex].attrLinkedList), newItemData );
      if( status == BDBREPORTING_ERROR )
      {
        returnStatus = BDBREPORTING_OUTOFMEMORYERROR;
        break;
      }

    }
  }
  return returnStatus;
}

static void bdb_RepInitAttrCfgRecords( void )
{
  bdb_RepConstructAttrCfgArray( ); //Here bdb_reportingAttrCfgRecordsArray is filled

  uint8_t status = osal_nv_item_init( ZCD_NV_BDBREPORTINGCONFIG, sizeof( bdbReportAttrCfgData_t )*bdb_reportingAttrCfgRecordsArrayCount, bdb_reportingAttrCfgRecordsArray );
  if( status == NV_OPER_FAILED )
  {
    return;
  }
  else
  {
      if( status == NV_ITEM_UNINIT )
      {
        //Do nothing because the reporting cf array data was written in the osal_nv_item method
      }
      else
      {
        //SUCCESS, There is NV data, read the data
        bdb_repAttrCfgRecordsArrayFreeAll(); //Clear previous cfg data
        uint16_t sizeNVRecord = osal_nv_item_len(ZCD_NV_BDBREPORTINGCONFIG);
        uint8_t attrCfgRecordsArrayCount = sizeNVRecord / sizeof(bdbReportAttrCfgData_t);

        status =  bdb_repAttrCfgRecordsArrayCreate(attrCfgRecordsArrayCount);
        if( status == BDBREPORTING_ERROR )
        {
          return; // No memory
        }
        osal_nv_read( ZCD_NV_BDBREPORTINGCONFIG,0, sizeof( bdbReportAttrCfgData_t )*attrCfgRecordsArrayCount ,bdb_reportingAttrCfgRecordsArray );
        bdb_reportingAttrCfgRecordsArrayCount = attrCfgRecordsArrayCount;
      }
  }

  bdb_repAttrDefaultCfgRecordsLinkedListFreeAll( &attrDefaultCfgRecordLinkedList ); //Free the attr default cfg list
}

static uint8_t bdb_RepConstructAttrCfgArray( void )
{
   epList_t *epCur =  epList;
   uint8_t status;
   uint8_t i;

   if( bdb_reportingAttrCfgRecordsArray != NULL )
   {
     bdb_repAttrCfgRecordsArrayFreeAll( );
   }

   uint8_t numRepAttr = 0;
   //First count the number of reportable attributes accross all endpoints
   for ( epCur = epList; epCur != NULL; epCur = epCur->nextDesc )
   {
     zclAttrRecsList* attrItem = zclFindAttrRecsList( epCur->epDesc->endPoint );
     if( attrItem== NULL )
     {
       continue;
     }
     if( attrItem->numAttributes > 0 )
     {
         for ( i = 0; i < attrItem->numAttributes; i++ )
         {
           if( attrItem->attrs[i].attr.accessControl & ACCESS_REPORTABLE )
           {
             numRepAttr++;
           }
         }
     }
   }
   status =  bdb_repAttrCfgRecordsArrayCreate( numRepAttr );
   if( status != BDBREPORTING_SUCCESS )
   {
     return status;
   }


   for ( epCur = epList; epCur != NULL; epCur = epCur->nextDesc )
   {
     zclAttrRecsList* attrItem = zclFindAttrRecsList( epCur->epDesc->endPoint );
     if( attrItem== NULL )
     {
       continue;
     }
     if( attrItem->numAttributes > 0 )
     {
       for ( i = 0; i < attrItem->numAttributes; i++ )
       {
         if( attrItem->attrs[i].attr.accessControl & ACCESS_REPORTABLE )
         {
           bdbReportAttrDefaultCfgData_t toSearch;
           toSearch.endpoint = epCur->epDesc->endPoint;
           toSearch.cluster = attrItem->attrs[i].clusterID;
           toSearch.attrID = attrItem->attrs[i].attr.attrId;
           bdbRepAttrDefaultCfgRecordLinkedListItem_t* lLItemFound = bdb_repAttrDefaultCfgRecordsLinkedListSearch( &attrDefaultCfgRecordLinkedList, toSearch );
           if( lLItemFound == NULL )
           {
             //Add with default static values
             uint8_t changeValue[] = BDBREPORTING_DEFAULTCHANGEVALUE;
             status = bdb_repAttrCfgRecordsArrayAdd( epCur->epDesc->endPoint, attrItem->attrs[i].clusterID,
                                                    attrItem->attrs[i].attr.attrId, BDBREPORTING_DEFAULTMININTERVAL, BDBREPORTING_DEFAULTMAXINTERVAL,
                                                    changeValue, BDBREPORTING_DEFAULTMININTERVAL, BDBREPORTING_DEFAULTMAXINTERVAL, changeValue );
           }
           else
           {
             //Add with user defined default values
             status = bdb_repAttrCfgRecordsArrayAdd( epCur->epDesc->endPoint, attrItem->attrs[i].clusterID,
                                                    attrItem->attrs[i].attr.attrId, lLItemFound->data->minReportInt, lLItemFound->data->maxReportInt,
                                                    lLItemFound->data->reportableChange, lLItemFound->data->minReportInt, lLItemFound->data->maxReportInt,
                                                    lLItemFound->data->reportableChange );
           }
         }
       }
     }

   }
   return BDBREPORTING_SUCCESS;

}

static uint8_t bdb_RepLoadCfgRecords( void )
{
  uint8_t status;

  uint16_t sizeNVRecord = osal_nv_item_len( ZCD_NV_BDBREPORTINGCONFIG );
  uint8_t attrCfgRecordsArrayCount = sizeNVRecord / sizeof( bdbReportAttrCfgData_t );

  if(sizeNVRecord == 0)
  {
      //Nv should be already created at this point.
      return BDBREPORTING_ERROR;
  }

  status =  bdb_repAttrCfgRecordsArrayCreate( attrCfgRecordsArrayCount );
  if( status == BDBREPORTING_ERROR )
  {
    return BDBREPORTING_OUTOFMEMORYERROR;
  }
  osal_nv_read( ZCD_NV_BDBREPORTINGCONFIG,0,sizeof( bdbReportAttrCfgData_t )*attrCfgRecordsArrayCount ,bdb_reportingAttrCfgRecordsArray );
  bdb_reportingAttrCfgRecordsArrayCount = attrCfgRecordsArrayCount;
  return BDBREPORTING_SUCCESS;

}

static void bdb_RepReport( uint8_t specificCLusterEndpointIndex )
{
  afAddrType_t dstAddr;
  zclReportCmd_t *pReportCmd = NULL;
  uint8_t i;

  bdbReportAttrClusterEndpoint_t* clusterEndpointItem = NULL;
  if( specificCLusterEndpointIndex == BDBREPORTING_INVALIDINDEX )
  {
    if( bdb_reportingNextClusterEndpointIndex < bdb_reportingClusterEndpointArrayCount )
    {
      clusterEndpointItem = &(bdb_reportingClusterEndpointArray[bdb_reportingNextClusterEndpointIndex]);
    }
  }
  else
  {
    clusterEndpointItem = &(bdb_reportingClusterEndpointArray[specificCLusterEndpointIndex]);
  }

  // actually send the report
  if( (clusterEndpointItem != NULL) &&
      (clusterEndpointItem->consolidatedMaxReportInt != ZCL_REPORTING_OFF) &&
      (clusterEndpointItem->attrLinkedList.numItems)
    )
  {
    uint8_t *pAttrData = NULL;
    uint8_t *pAttrDataTemp = NULL;
    dstAddr.addrMode = (afAddrMode_t)AddrNotPresent;
    dstAddr.addr.shortAddr = 0;
    dstAddr.endPoint = clusterEndpointItem->endpoint;
    dstAddr.panId = _NIB.nwkPanId;

    pReportCmd = OsalPort_malloc( sizeof( zclReportCmd_t ) + (clusterEndpointItem->attrLinkedList.numItems * sizeof( zclReport_t )) );
    pAttrData = OsalPort_malloc(clusterEndpointItem->attrLinkedList.numItems * BDBREPORTING_MAX_ANALOG_ATTR_SIZE);
    if ( (pReportCmd != NULL) && (pAttrData != NULL) )
    {
      pAttrDataTemp = pAttrData;
      pReportCmd->numAttr = clusterEndpointItem->attrLinkedList.numItems;
      for ( i = 0; i < clusterEndpointItem->attrLinkedList.numItems; ++ i )
      {
        pReportCmd->attrList[i].attrID   = 0xFFFF;
        pReportCmd->attrList[i].dataType = 0xFF;
        pReportCmd->attrList[i].attrData = NULL;

        bdbLinkedListAttrItem_t* attrListItem = bdb_linkedListAttrGetAtIndex( &clusterEndpointItem->attrLinkedList, i );
        if(attrListItem!=NULL)
        {
          zclAttribute_t attrRec;
          pReportCmd->attrList[i].attrID = attrListItem->data->attrID;
          uint8_t attrRes = bdb_RepFindAttrEntry( clusterEndpointItem->endpoint, clusterEndpointItem->cluster, attrListItem->data->attrID, &attrRec );
          if( attrRes == BDBREPORTING_TRUE )
          {
            pReportCmd->attrList[i].dataType = attrRec.dataType;
            pReportCmd->attrList[i].attrData = pAttrDataTemp;
            pAttrDataTemp = OsalPort_memcpy(pAttrDataTemp, attrRec.dataPtr, BDBREPORTING_MAX_ANALOG_ATTR_SIZE);
            //Update last value reported
            if( zclAnalogDataType( attrRec.dataType ) )
            {
              //Only if the datatype is analog
              memset( attrListItem->data->lastValueReported,0x00, BDBREPORTING_MAX_ANALOG_ATTR_SIZE );
              OsalPort_memcpy( attrListItem->data->lastValueReported, attrRec.dataPtr, zclGetDataTypeLength( attrRec.dataType ) );
            }
          }
        }
      }

      zcl_StackSendReportCmd( clusterEndpointItem->endpoint, &dstAddr,
                         clusterEndpointItem->cluster, pReportCmd,
                         ZCL_FRAME_SERVER_CLIENT_DIR, BDB_REPORTING_DISABLE_DEFAULT_RSP, zcl_getFrameCounter() );
    }
    if( (pReportCmd != NULL ) )
    {
      OsalPort_free( pReportCmd );
    }
    if ( (pAttrData != NULL) )
    {
      OsalPort_free( pAttrData );
    }
  }
}

static uint8_t bdb_isAttrValueChangedSurpassDelta( uint8_t datatype, uint8_t* delta, uint8_t* curValue, uint8_t* lastValue )
{
  uint8_t res = BDBREPORTING_FALSE;
  switch ( datatype )
  {
    case ZCL_DATATYPE_UINT8:
    {
      uint8_t L = *((uint8_t*)lastValue);
      uint8_t D = *((uint8_t*)delta);
      uint8_t C = *((uint8_t*)curValue);
      if( L >= C )
      {
        res = ( L-C >= D) ? BDBREPORTING_TRUE:BDBREPORTING_FALSE;
      }
      else
      {
        res = ( C-L >= D) ? BDBREPORTING_TRUE:BDBREPORTING_FALSE;
      }
      break;
    }
    case ZCL_DATATYPE_UINT16:
    {
      uint16_t L = *((uint16_t*)lastValue);
      uint16_t D = *((uint16_t*)delta);
      uint16_t C = *((uint16_t*)curValue);
      if(L>=C)
      {
        res = ( L-C >= D) ? BDBREPORTING_TRUE:BDBREPORTING_FALSE;
      }
      else
      {
        res = ( C-L >= D) ? BDBREPORTING_TRUE:BDBREPORTING_FALSE;
      }
      break;
    }
    case ZCL_DATATYPE_UINT24:
    {
      uint24 L = *((uint24*)lastValue);
      uint24 D = *((uint24*)delta);
      uint24 C = *((uint24*)curValue);
      if(L>=C)
      {
        res = ( L-C >= D) ? BDBREPORTING_TRUE:BDBREPORTING_FALSE;
      }
      else
      {
        res = ( C-L >= D) ? BDBREPORTING_TRUE:BDBREPORTING_FALSE;
      }
      break;
    }
    case ZCL_DATATYPE_UINT32:
    {
      uint32_t L = *((uint32_t*)lastValue);
      uint32_t D = *((uint32_t*)delta);
      uint32_t C = *((uint32_t*)curValue);
      if(L>=C)
      {
        res = ( L-C >= D) ? BDBREPORTING_TRUE:BDBREPORTING_FALSE;
      }
      else
      {
        res = ( C-L >= D) ? BDBREPORTING_TRUE:BDBREPORTING_FALSE;
      }
      break;
    }
    case ZCL_DATATYPE_INT8:
    {
      int8_t L = *((int8_t*)lastValue);
      int8_t D = *((int8_t*)delta);
      int8_t C = *((int8_t*)curValue);
      if(L>=C)
      {
        res = ( L-C >= D) ? BDBREPORTING_TRUE:BDBREPORTING_FALSE;
      }
      else
      {
        res = ( C-L >= D) ? BDBREPORTING_TRUE:BDBREPORTING_FALSE;
      }
      break;
    }
    case ZCL_DATATYPE_INT16:
    {
      int16_t L = *((int16_t*)lastValue);
      int16_t D = *((int16_t*)delta);
      int16_t C = *((int16_t*)curValue);
      if(L>=C)
      {
        res = ( L-C >= D) ? BDBREPORTING_TRUE:BDBREPORTING_FALSE;
      }
      else
      {
        res = ( C-L >= D) ? BDBREPORTING_TRUE:BDBREPORTING_FALSE;
      }
      break;
    }
    case ZCL_DATATYPE_INT24:
    {
      int24 L = *((int24*)lastValue);
      int24 D = *((int24*)delta);
      int24 C = *((int24*)curValue);
      if(L>=C)
      {
        res = ( L-C >= D) ? BDBREPORTING_TRUE:BDBREPORTING_FALSE;
      }
      else
      {
        res = ( C-L >= D) ? BDBREPORTING_TRUE:BDBREPORTING_FALSE;
      }
      break;
    }
    case ZCL_DATATYPE_INT32:
    {
      int32_t L = *((int32_t*)lastValue);
      int32_t D = *((int32_t*)delta);
      int32_t C = *((int32_t*)curValue);
      if(L>=C)
      {
        res = ( L-C >= D) ? BDBREPORTING_TRUE:BDBREPORTING_FALSE;
      }
      else
      {
        res = ( C-L >= D) ? BDBREPORTING_TRUE:BDBREPORTING_FALSE;
      }
      break;
    }
    case ZCL_DATATYPE_SINGLE_PREC:
    {
      float L = *((float*)lastValue);
      float D = *((float*)delta);
      float C = *((float*)curValue);
      if(L>=C)
      {
        res = ( L-C >= D) ? BDBREPORTING_TRUE:BDBREPORTING_FALSE;
      }
      else
      {
        res = ( C-L >= D) ? BDBREPORTING_TRUE:BDBREPORTING_FALSE;
      }
      break;
    }
    case ZCL_DATATYPE_DOUBLE_PREC:
    {
      double L = *((double*)lastValue);
      double D = *((double*)delta);
      double C = *((double*)curValue);
      if(L>=C)
      {
        res = ( L-C >= D) ? BDBREPORTING_TRUE:BDBREPORTING_FALSE;
      }
      else
      {
        res = ( C-L >= D) ? BDBREPORTING_TRUE:BDBREPORTING_FALSE;
      }
      break;
    }
    case ZCL_DATATYPE_INT40:
    case ZCL_DATATYPE_INT48:
    case ZCL_DATATYPE_INT56:
    case ZCL_DATATYPE_UINT64:
    case ZCL_DATATYPE_INT64:
    case ZCL_DATATYPE_SEMI_PREC:
    case ZCL_DATATYPE_UINT40:
    case ZCL_DATATYPE_UINT48:
    case ZCL_DATATYPE_UINT56:
    case ZCL_DATATYPE_TOD:
    case ZCL_DATATYPE_DATE:
    case ZCL_DATATYPE_UTC:
    {
      // Not implemented
      res = BDBREPORTING_FALSE;
      break;
    }
    default:
    {
      res = BDBREPORTING_FALSE;
      break;
    }
  }
  return res;
}

/*
* End: Helper methods
*/


/*
* Begin: Reporting timer related methods
*/

static void bdb_RepRestartNextEventTimer( void )
{
  uint32_t timeMs;
  // convert from seconds to milliseconds
  timeMs = 1000L * (bdb_reportingNextEventTimeout);
  OsalPortTimers_startTimer( bdb_TaskID, BDB_REPORT_TIMEOUT, timeMs );
}

static void bdb_RepSetupReporting( void )
{
  uint8_t numArrayFlags, i;
  //Stop if reporting timer is active
  OsalPortTimers_stopTimer( bdb_TaskID, BDB_REPORT_TIMEOUT );

  numArrayFlags = bdb_reportingClusterEndpointArrayCount;
  bdbReportFlagsHolder_t* arrayFlags = (bdbReportFlagsHolder_t *)OsalPort_malloc( sizeof( bdbReportFlagsHolder_t )*numArrayFlags );
  if( arrayFlags==NULL )
  {
    return;
  }
  for( i=0; i<numArrayFlags; i++ )
  {
    arrayFlags[i].endpoint =  bdb_reportingClusterEndpointArray[i].endpoint;
    arrayFlags[i].cluster =  bdb_reportingClusterEndpointArray[i].cluster;
    arrayFlags[i].flags =  bdb_reportingClusterEndpointArray[i].flags;
  }

  if( bdb_reportingClusterEndpointArrayCount > 0 )
  {
    bdb_clusterEndpointArrayFreeAll( );
  }

  //Built or rebuilt the clusterEndpoint array
  bdb_repAttrBuildClusterEndPointArrayBasedOnConfRecordsArray( );

  for( i=0; i<numArrayFlags;i++ )
  {
     uint8_t clusterEndpointIndex = bdb_clusterEndpointArraySearch( arrayFlags[i].endpoint, arrayFlags[i].cluster );
     if( clusterEndpointIndex != BDBREPORTING_INVALIDINDEX )
     {
       bdb_reportingClusterEndpointArray[clusterEndpointIndex].flags = arrayFlags[i].flags;
     }
  }
  OsalPort_free( arrayFlags );
}


static void bdb_RepStopEventTimer( void )
{
  OsalPortTimers_stopTimer( bdb_TaskID, BDB_REPORT_TIMEOUT );
}

/*
* End: Reporting timer related methods
*/

/*
* Begin: Ztack zcl helper methods
*/

/*********************************************************************
 * @fn      bdb_FindEpDesc
 *
 * @brief   Find the EndPoint descriptor pointer
 *
 * @param   endPoint - EndPoint Id
 *
 * @return  CurrEpDescriptor - Pointer to found Simple Descriptor, NULL otherwise
 */
static endPointDesc_t* bdb_FindEpDesc( uint8_t endPoint )
{
  endPointDesc_t *CurrEpDescriptor = NULL;

  epList_t *bdb_CurrEpDescriptorNextInList;

  bdb_CurrEpDescriptorNextInList = bdb_HeadEpDescriptorList;
  CurrEpDescriptor = bdb_CurrEpDescriptorNextInList->epDesc;

  while ( CurrEpDescriptor->endPoint != endPoint )
  {
    if ( bdb_CurrEpDescriptorNextInList->nextDesc->nextDesc != NULL )
    {
      bdb_CurrEpDescriptorNextInList = bdb_CurrEpDescriptorNextInList->nextDesc;
      CurrEpDescriptor = bdb_CurrEpDescriptorNextInList->epDesc;
    }
    else
    {
      return ( NULL );
    }
  }
  return CurrEpDescriptor;
}

static uint8_t bdb_RepFindAttrEntry( uint8_t endpoint, uint16_t cluster, uint16_t attrID, zclAttribute_t* attrRes )
{
  epList_t *epCur = epList;
  uint8_t i;

  zcl_memset(gAttrDataValue, 0, BDBREPORTING_MAX_ANALOG_ATTR_SIZE);
  for ( epCur = epList; epCur != NULL; epCur = epCur->nextDesc )
  {
    if( epCur->epDesc->endPoint == endpoint )
    {
      zclAttrRecsList* attrItem = zclFindAttrRecsList( epCur->epDesc->endPoint );

      if( (attrItem != NULL) && ( (attrItem->numAttributes > 0) && (attrItem->attrs != NULL) ) )
      {
        for ( i = 0; i < attrItem->numAttributes; i++ )
        {
          if ( ( attrItem->attrs[i].clusterID == cluster ) && ( attrItem->attrs[i].attr.attrId ==  attrID ) )
          {
            uint16_t dataLen;

            attrRes->attrId = attrItem->attrs[i].attr.attrId;
            attrRes->dataType = attrItem->attrs[i].attr.dataType;
            attrRes->accessControl = attrItem->attrs[i].attr.accessControl;

            dataLen = zclGetDataTypeLength(attrRes->dataType);
            zcl_ReadAttrData( endpoint, cluster, attrRes->attrId, gAttrDataValue, &dataLen );
            attrRes->dataPtr = gAttrDataValue;
            return BDBREPORTING_TRUE;
          }
        }
      }
    }
  }
  return BDBREPORTING_FALSE;
 }

/*
* End: Ztack zcl helper methods
*/




/*********************************************************************
*********************************************************************/

/*
* Begin: Reporting attr app API methods
*/



 /*********************************************************************
 * @fn          bdb_RepAddAttrCfgRecordDefaultToList
 *
 * @brief       Adds default configuration values for a Reportable Attribute Record
 *
 * @param       endpoint
 * @param       cluster
 * @param       attrID - Reporable attribute ID
 * @param       minReportInt - Default value for minimum reportable interval
 * @param       maxReportInt - Default value for maximum reportable interval
 * @param       reportableChange - buffer containing attribute value that is the
 *              delta change to trigger a report
 *
 * @return      ZInvalidParameter - No endpoint, cluster, attribute ID found in simple desc
 *              ZFailure - No memory to allocate entry
 *              ZSuccess
 *
 */
ZStatus_t bdb_RepAddAttrCfgRecordDefaultToList( uint8_t endpoint, uint16_t cluster, uint16_t attrID, uint16_t minReportInt, uint16_t maxReportInt, uint8_t* reportableChange )
{
  uint8_t status;
  epList_t *epCur;
  uint8_t i;

  if( bdb_reportingAcceptDefaultConfs == BDBREPORTING_FALSE )
  {
    //Don't accept anymore default attribute configurations
    return ZFailure;
  }

  //Find if endpoint and cluster values are valid
  uint8_t foundEndpCluster = BDBREPORTING_FALSE;
  for ( epCur = epList; epCur != NULL; epCur = epCur->nextDesc )
  {
    if( epCur->epDesc->endPoint != endpoint )
    {
      continue;
    }
    zclAttrRecsList* attrItem = zclFindAttrRecsList( epCur->epDesc->endPoint );
    if( attrItem== NULL )
    {
      continue;
    }
    if( attrItem->numAttributes == 0 || attrItem->attrs == NULL )
    {
      continue;
    }
    for ( i = 0; i < attrItem->numAttributes; i++ )
    {
      if( attrItem->attrs[i].clusterID != cluster )
      {
        continue;
      }
      foundEndpCluster = BDBREPORTING_TRUE;
      break;
    }
    break;
  }
  if( foundEndpCluster==BDBREPORTING_FALSE )
  {
    return ZInvalidParameter;
  }

  //Add default cfg values to list
  bdbReportAttrDefaultCfgData_t* record = (bdbReportAttrDefaultCfgData_t *)OsalPort_malloc( sizeof( bdbReportAttrDefaultCfgData_t ) );
  if( record == NULL)
  {
    return ZFailure; //Out of memory
  }
  bdb_repAttrDefaultCfgRecordInitValues( record );

  record->endpoint = endpoint;
  record->cluster = cluster;
  record->attrID = attrID;
  record->minReportInt = minReportInt;
  record->maxReportInt = maxReportInt;
  OsalPort_memcpy( record->reportableChange, reportableChange, BDBREPORTING_MAX_ANALOG_ATTR_SIZE );

  status = bdb_repAttrDefaultCfgRecordsLinkedListAdd( &attrDefaultCfgRecordLinkedList, record );
  if( status != BDBREPORTING_SUCCESS )
  {
    OsalPort_free( record );
    return ZFailure; //Out of memory
  }

  return ZSuccess;
}



 /*********************************************************************
 * @fn          bdb_RepChangedAttrValue
 *
 * @brief       Notify BDB reporting attribute module about the change of an
 *              attribute value to validate the triggering of a reporting attribute message.
 *
 * @param       endpoint
 * @param       cluster
 * @param       attrID - Reporable attribute ID
 *
 * @return      ZInvalidParameter - No endpoint, cluster, attribute ID found in simple desc
 *              ZSuccess
 */
ZStatus_t bdb_RepChangedAttrValue( uint8_t endpoint, uint16_t cluster, uint16_t attrID )
{
  uint8_t indexClusterEndpoint = bdb_clusterEndpointArraySearch( endpoint, cluster );
  if( indexClusterEndpoint == BDBREPORTING_INVALIDINDEX )
  {
    //cluter-endpoint not found
    return ZInvalidParameter;
  }
  if( FLAGS_CHECKFLAG( bdb_reportingClusterEndpointArray[indexClusterEndpoint].flags, BDBREPORTING_HASBINDING_FLAG_MASK ) == BDBREPORTING_FALSE )
  {
    //record has no binding
    return ZSuccess;
  }
  if( bdb_reportingClusterEndpointArray[indexClusterEndpoint].consolidatedMaxReportInt == BDBREPORTING_REPORTOFF )
  {
    //reporting if off for this cluster
    return ZSuccess;
  }

  bdbReportAttrLive_t searchdata;
  searchdata.attrID = attrID;
  bdbLinkedListAttrItem_t* attrNodeFound = bdb_linkedListAttrSearch( &(bdb_reportingClusterEndpointArray[indexClusterEndpoint].attrLinkedList), &searchdata );
  if( attrNodeFound == NULL || attrNodeFound->data == NULL )
  {
    return ZInvalidParameter; //Attr not found in cluster-endpoint array
  }

  zclAttribute_t attrRec;
  uint8_t attrRes = bdb_RepFindAttrEntry( endpoint, cluster, attrID, &attrRec );
  if( attrRes != BDBREPORTING_TRUE )
  {
    return ZInvalidParameter; //Attr not found in attributes app data
  }

  //Get time of timer if active
  uint32_t remainingTimeOfEvent = OsalPortTimers_getTimerTimeout( bdb_TaskID, BDB_REPORT_TIMEOUT );
  uint16_t elapsedTime = 0;
  uint8_t isTimeRemaining = BDBREPORTING_FALSE;
  if( remainingTimeOfEvent > 0 )
  {
    elapsedTime = bdb_RepCalculateEventElapsedTime( remainingTimeOfEvent, bdb_reportingNextEventTimeout );
    isTimeRemaining =  BDBREPORTING_TRUE;
  }

  if( bdb_reportingClusterEndpointArray[indexClusterEndpoint].consolidatedMinReportInt != BDBREPORTING_NOLIMIT &&
     (bdb_reportingClusterEndpointArray[indexClusterEndpoint].timeSinceLastReport + elapsedTime) <= bdb_reportingClusterEndpointArray[indexClusterEndpoint].consolidatedMinReportInt)
  {
      //Attr value has changed before minInterval, ommit reporting
      return ZSuccess;
  }


  if( zclAnalogDataType(attrRec.dataType) )
  {
    //Checking if   | lastvaluereported - currentvalue | >=  | changevalue |
    if( bdb_isAttrValueChangedSurpassDelta(attrRec.dataType, attrNodeFound->data->reportableChange, attrRec.dataPtr, attrNodeFound->data->lastValueReported ) == BDBREPORTING_FALSE )
    {
      //current value does not excced the delta, dont report
      return ZSuccess;
    }
  }
  else
  {
    //Attr is discrete, just report without checking the changeValue
  }

  //Stop reporting
  bdb_RepStopEventTimer( );
  bdb_RepReport( indexClusterEndpoint );
  if( isTimeRemaining == BDBREPORTING_TRUE )
  {
    bdb_clusterEndpointArrayIncrementAll( elapsedTime, BDBREPORTING_FALSE );
  }
  bdb_clusterEndpointArrayUpdateAt( indexClusterEndpoint, 0, BDBREPORTING_IGNORE, BDBREPORTING_IGNORE ); //return time since last report to zero
  //Restart reporting
  bdb_RepStartReporting( );

  return ZSuccess;
}

#endif //BDB_REPORTING

/*
* End: Reporting attr app API methods
*/

