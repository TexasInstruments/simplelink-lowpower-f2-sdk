/******************************************************************************

 @file  cgm_db.h

 @brief This file contains the Data Stream profile definitions and prototypes.

 Group: WCS, BTS
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2010-2025, Texas Instruments Incorporated
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

 ******************************************************************************
 
 
 *****************************************************************************/

#ifndef CGMDB_H
#define CGMDB_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */

#include <ti/bleapp/services/continuous_glucose_monitoring/cgm_server.h>

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * TYPEDEFS
 */

// Max Number of Records in the DB
#define CGM_DB_MAX_NUM_OF_RECORDS   100

// CGM Measurements Data Base Records Info
// (This is due to the cyclic manner of the data base)
typedef struct
{
  uint16               measDBMaxNumRecords;    // The current max number of records in the data base
  uint16               measDBHead;             // The index of the minimum record - Head
  uint16               measDBTail;             // The index of the maximum record - Tail
} CGMS_DB_recordsInfo_entry_t;

/*********************************************************************
 * Profile Callback
 */

/*********************************************************************
 * API FUNCTIONS
 */

/*********************************************************************
 * @fn      CGM_DB_initDataBase
 *
 * @brief   This function initializes the CGM's measurements records
 *          data base variables and array.
 *
 * @param   samplingInterval - Current Measurements Interval.
 *
 * @return  SUCCESS or stack call status
 */
bStatus_t CGM_DB_initDataBase( uint16 samplingInterval );

/*********************************************************************
 * @fn      CGM_DB_AddMeasRecord
 *
 * @brief   This function add a measurement record to the data base.
 *
 *
 * @param   pMeasRecord - pointer to the record's structure info.
 *
 * @return  SUCCESS or stack call status
 */
bStatus_t CGM_DB_addMeasRecord( CGMS_measRecord_t *pMeasRecord );

/*********************************************************************
 * @fn      CGM_DB_GetMeasRecord
 *
 * @brief   This function gets a measurement record (single) from the data base.
 *          Notice that the records number needs to be in the current records range.
 *
 * @param   timeOffset    - the time offset of the wanted record to extract.
 * @param   pMeasRecord   - pointer to the record's structure info that
 *                          would be filled.
 *
 * @return  SUCCESS or stack call status
 */
bStatus_t CGM_DB_getMeasRecord(uint16 timeOffset, CGMS_measRecord_t **pMeasRecord);

/*********************************************************************
 * @fn      CGM_DB_GetCurrentNumOfRecords
 *
 * @brief   This function gets the current number of measurements records
 *          in the data base.
 *
 * @param   None.
 *
 * @return  cgm_DB_NumOfRecords Value
 */
uint16 CGM_DB_getCurrentNumOfRecords(void);

/*********************************************************************
 * @fn      CGM_DB_getCurrentNumOfRecords
 *
 * @brief   This function gets the current number of measurements records
 *          in the data base.
 *
 * @param   None.
 *
 * @return  cgm_DB_NumOfRecords ValueGREATER
 */
uint16 CGM_DB_getNumOfRecordsGreaterOrEqual( uint16 timeOffset);

/*********************************************************************
 * @fn      CGM_DB_getMinTimeOffset
 *
 * @brief   This function gets the current time offset of the minimum record - Head
 *
 *
 * @param   None.
 *
 * @return  measDBHead Value
 */
uint16 CGM_DB_getMinTimeOffset(void);

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* CGMDB_H */
