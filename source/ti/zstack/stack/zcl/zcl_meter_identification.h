/**************************************************************************************************
  Filename:       zcl_meter_identification.h
  Revised:        $Date: 2013-10-16 16:38:58 -0700 (Wed, 16 Oct 2013) $
  Revision:       $Revision: 35701 $

  Description:    This file contains the ZCL Meter Identification definitions.


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

#ifndef ZCL_METER_IDENTIFICATION_H
#define ZCL_METER_IDENTIFICATION_H

#ifdef ZCL_METER_IDENTIFICATION

#ifdef __cplusplus
extern "C"
{
#endif

/******************************************************************************
 * INCLUDES
 */
#include "zcl.h"
#include "ti_zstack_config.h"


/******************************************************************************
 * CONSTANTS
 */

/************************************************/
/***  Meter Identification Cluster Attributes ***/
/************************************************/

// Server Attributes
#define ATTRID_METER_IDENTIFICATION_COMPANY_NAME                0x0000  // M, R, character string up to 16 octets
#define ATTRID_METER_IDENTIFICATION_METER_TYPE_ID               0x0001  // M, R, uint16_t
#define ATTRID_METER_IDENTIFICATION_DATA_QUALITY_ID             0x0004  // M, R, uint16_t
#define ATTRID_METER_IDENTIFICATION_CUSTOMER_NAME               0x0005  // O, R, character string up to 16 octets
#define ATTRID_METER_IDENTIFICATION_MODEL                       0x0006  // O, R, character string up to 16 octets
#define ATTRID_METER_IDENTIFICATION_PART_NUMBER                 0x0007  // O, R, character string up to 16 octets
#define ATTRID_METER_IDENTIFICATION_PRODUCT_REVISION            0x0008  // O, R, character string up to 6 octets
#define ATTRID_METER_IDENTIFICATION_SOFTWARE_REVISION           0x000A  // O, R, character string up to 6 octets
#define ATTRID_METER_IDENTIFICATION_UTILITY_NAME                0x000B  // O, R, character string up to 16 octets
#define ATTRID_METER_IDENTIFICATION_POD                         0x000C  // M, R, character string up to 16 octets
#define ATTRID_METER_IDENTIFICATION_AVAILABLE_POWER             0x000D  // M, R, INT24
#define ATTRID_METER_IDENTIFICATION_POWER_THRESHOLD             0x000E  // M, R, INT24

// Server Attribute Defaults
#define ATTR_DEFAULT_METER_IDENTIFICATION_COMPANY_NAME                ""
#define ATTR_DEFAULT_METER_IDENTIFICATION_METER_TYPE_ID               0
#define ATTR_DEFAULT_METER_IDENTIFICATION_DATA_QUALITY_ID             0
#define ATTR_DEFAULT_METER_IDENTIFICATION_CUSTOMER_NAME               ""
#define ATTR_DEFAULT_METER_IDENTIFICATION_MODEL                       {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
#define ATTR_DEFAULT_METER_IDENTIFICATION_PART_NUMBER                 {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
#define ATTR_DEFAULT_METER_IDENTIFICATION_PRODUCT_REVISION            {0,0,0,0,0,0}
#define ATTR_DEFAULT_METER_IDENTIFICATION_SOFTWARE_REVISION           {0,0,0,0,0,0}
#define ATTR_DEFAULT_METER_IDENTIFICATION_UTILITY_NAME                ""
#define ATTR_DEFAULT_METER_IDENTIFICATION_POD                         ""
#define ATTR_DEFAULT_METER_IDENTIFICATION_AVAILABLE_POWER             0
#define ATTR_DEFAULT_METER_IDENTIFICATION_POWER_THRESHOLD             0

/*******************************************************************************
 * TYPEDEFS
 */

/******************************************************************************
 * FUNCTION MACROS
 */

/******************************************************************************
 * VARIABLES
 */

/******************************************************************************
 * FUNCTIONS
 */

/*
 * Register for callbacks from this cluster library
 */

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif // ZCL_METER_IDENTIFICATION
#endif /* ZCL_METER_IDENTIFICATION_H */
