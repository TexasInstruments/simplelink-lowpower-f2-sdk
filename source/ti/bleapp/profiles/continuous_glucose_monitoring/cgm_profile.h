/******************************************************************************

 @file  cgm_profile.h

 @brief This file contains the CGM profile definitions and prototypes.

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

#ifndef CGMPROFILE_H
#define CGMPROFILE_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include <ti/bleapp/ble_app_util/inc/bleapputil_api.h>
#include <ti/bleapp/services/continuous_glucose_monitoring/cgm_server.h>

/*********************************************************************
 * DEFINES
 */
// CGMP default session run time
#define CGMP_DEFAUALT_SRT         12

// CGMP convert minutes to milliseconds
#define CGMP_MIM_TO_MSEC          60000

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */

/*********************************************************************
 * Profile Callback
 */

//Callback to indicate client characteristic configuration has been updated for Measurement characteristic
typedef void ( *CGMP_measOnCccUpdate_t )( uint16 connHandle, uint16 pValue );

//Callback to indicate client characteristic configuration has been updated for RACP characteristic
typedef void ( *CGMP_racpOnCccUpdate_t )( uint16 connHandle, uint16 pValue );

//Callback to indicate client characteristic configuration has been updated for CGMCP characteristic
typedef void ( *CGMP_cgmcpOnCccUpdate_t )( uint16 connHandle, uint16 pValue );

//Callback to indicate that session start time value has been updated
typedef void ( *CGMP_sstUpdate_t )( CGMS_sst_t *pValue );

typedef struct
{
  CGMP_measOnCccUpdate_t   pfnMeasOnCccUpdateCB;   // Called when client characteristic configuration has been updated on Measurement characteristic
  CGMP_racpOnCccUpdate_t   pfnRACPOnCccUpdateCB;   // Called when client characteristic configuration has been updated on RACP characteristic
  CGMP_cgmcpOnCccUpdate_t  pfnCGMCPOnCccUpdateCB;  // Called when client characteristic configuration has been updated on CGMCP characteristic
  CGMP_sstUpdate_t         pfnSSTUpdateCB;         // Called when session start time value has been updated
} CGMP_cb_t;

/*********************************************************************
 * API FUNCTIONS
 */

/*********************************************************************
 * @fn      CGMP_start
 *
 * @brief   This function adds the CGM Server service,
 *          registers the service callback function and initializes
 *          the service characteristics
 *
 * @param   appCallbacks - pointer to application callback
 * @param   feat - CGM Feature parameters
 * @param   timeOffset - CGM initial time offset parameter
 *
 * @return  SUCCESS or stack call status
 */
bStatus_t CGMP_start( CGMP_cb_t *appCallbacks, CGMS_feat_t feat, uint16 timeOffset );

/*
 * @fn      CGMP_addMeaserment
 *
 * @brief   Add CGM measurement to data base
 *
 * @param   glucoseConcen - a Glucose concentration in mg/dL units
 * @param   timeOffset - Minutes since the Session Start Time
 *
 * @return  SUCCESS or stack call status
 */
bStatus_t CGMP_addMeaserment( uint16  glucoseConcen, uint16  timeOffset );

/*
 * @fn      CGMP_updateStatus
 *
 * @brief   Update status characteristic on the server attributes table
 *
 * @param   status - updated status value
 * @param   len - length of data to write
 *
 * @return  SUCCESS or stack call status
 */
bStatus_t CGMP_updateStatus( CGMS_status_t stat );

/*
 * @fn      CGMP_updateSessionRunTime
 *
 * @brief   Update session run time characteristic on the server attributes table
 *
 * @param   timeToReduce - value in hours to reduce from session run time charicterstic
 *
 * @return  SUCCESS or stack call status
 */
bStatus_t CGMP_updateSessionRunTime( uint16 timeToReduce );

/*
 * @fn      CGMP_clockCB
 *
 * @brief   Callback function for clock module
 *
 * @param   arg - argument passed to callback function.
 *
 * @return  none
 */
#ifdef FREERTOS
void CGMP_clockCB( uint32_t arg );
#else
void CGMP_clockCB( uintptr_t arg );
#endif // FREERTOS

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* CGMROFILE_H */
