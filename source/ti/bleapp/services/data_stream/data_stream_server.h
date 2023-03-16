/******************************************************************************

 @file  data_stream_server.h

 @brief This file contains the Data Stream service definitions and prototypes.

 Group: WCS, BTS
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2010-2023, Texas Instruments Incorporated
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

#ifndef DATASTREAMSERVER_H
#define DATASTREAMSERVER_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */

/*********************************************************************
 * CONSTANTS
 */
// Service UUID
#define DATASTREAMSERVER_SERV_UUID 0xC0C0

// Characteristic defines
#define DATASTREAMSERVER_DATAIN_ID   0
#define DATASTREAMSERVER_DATAIN_UUID 0xC0C1

// Characteristic defines
#define DATASTREAMSERVER_DATAOUT_ID   1
#define DATASTREAMSERVER_DATAOUT_UUID 0xC0C2

/*********************************************************************
 * Profile Callbacks
 */
// Callback to indicate client characteristic configuration has been updated
typedef void (*DataStreamServer_cccUpdate_t)( uint16 connHandle, uint16 value);

// Callback when data is received
typedef void (*DataStreamServer_incomingData_t)( uint16 connHandle, uint8 *pValue, uint16 len );

typedef struct
{
  DataStreamServer_cccUpdate_t           pfnCccUpdateCb;     // Called when client characteristic configuration has been updated
  DataStreamServer_incomingData_t        pfnIncomingDataCB;  // Called when receiving data
} DataStreamServer_CB_t;

/*********************************************************************
 * API FUNCTIONS
 */

/*********************************************************************
 * @fn      DataStreamServer_addService
 *
 * @brief   This function initializes the Data Stream Server service
 *          by registering GATT attributes with the GATT server.
 *
 * @return  SUCCESS or stack call status
 */
bStatus_t DataStreamServer_addService( void );

/*
 * @fn      DataStreamServer_registerProfileCBs
 *
 * @brief   Registers the profile callback function. Only call
 *          this function once.
 *
 * @param   profileCallback - pointer to profile callback.
 *
 * @return  SUCCESS or INVALIDPARAMETER
 */
bStatus_t DataStreamServer_registerProfileCBs( DataStreamServer_CB_t *profileCallback );

/*
 * @fn      DataStreamServer_setParameter
 *
 * @brief   Set a Data Stream Service parameter.
 *
 * @param   param - Characteristic UUID
 * @param   pValue - pointer to data to write.  This is dependent on
 *          the parameter ID and WILL be cast to the appropriate
 *          data type (example: data type of uint16 will be cast to
 *          uint16 pointer).
 * @param   len - length of data to write
 *
 * @return  SUCCESS or stack call status
 */
bStatus_t DataStreamServer_setParameter( uint8 param, void *pValue, uint16 len);

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* DATASTREAMSERVER_H */
