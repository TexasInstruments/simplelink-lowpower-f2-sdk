/**************************************************************************************************
  Filename:       zcl_appliance_identification.h
  Revised:        $Date: 2013-10-16 16:38:58 -0700 (Wed, 16 Oct 2013) $
  Revision:       $Revision: 35701 $

  Description:    This file contains the ZCL Appliance Identification definitions.


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

#ifndef ZCL_APPLIANCE_IDENTIFICATION_H
#define ZCL_APPLIANCE_IDENTIFICATION_H

#ifdef ZCL_APPLIANCE_IDENTIFICATION

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

/*************************************************************/
/***  EN50523 Appliance Identification Cluster Attributes ***/
/************************************************************/

// Server Attributes
#define ATTRID_APPLIANCE_IDENTIFICATION_BASIC_IDENTIFICATION            0x0000 // M, R, UINT56
#define ATTRID_APPLIANCE_IDENTIFICATION_COMPANY_NAME                    0x0010 // O, R, character string up to 16 octets
#define ATTRID_APPLIANCE_IDENTIFICATION_COMPANY_ID                      0x0011 // O, R, uint16_t
#define ATTRID_APPLIANCE_IDENTIFICATION_BRAND_NAME                      0x0012 // O, R, character string up to 16 octets
#define ATTRID_APPLIANCE_IDENTIFICATION_BRAND_ID                        0x0013 // O, R, uint16_t
#define ATTRID_APPLIANCE_IDENTIFICATION_MODEL                           0x0014 // O, R, octet string up to 16 octets
#define ATTRID_APPLIANCE_IDENTIFICATION_PART_NUMBER                     0x0015 // O, R, octet string up to 16 octets
#define ATTRID_APPLIANCE_IDENTIFICATION_PRODUCT_REVISION                0x0016 // O, R, octet string up to 6 octets
#define ATTRID_APPLIANCE_IDENTIFICATION_SOFTWARE_REVISION               0x0017 // O, R, octet string up to 6 octets
#define ATTRID_APPLIANCE_IDENTIFICATION_PRODUCT_TYPE_NAME               0x0018 // O, R, octet string up to 2 octets
#define ATTRID_APPLIANCE_IDENTIFICATION_PRODUCT_TYPE_ID                 0x0019 // O, R, uint16_t
#define ATTRID_APPLIANCE_IDENTIFICATION_CECED_SPECIFICATION_VERSION     0x001A // O, R, uint8_t

// Server Attribute Defaults
#define ATTR_DEFAULT_APPLIANCE_IDENTIFICATION_BASIC_IDENTIFICATION            {0,0,0,0,0,0,0}
#define ATTR_DEFAULT_APPLIANCE_IDENTIFICATION_COMPANY_NAME                    " "
#define ATTR_DEFAULT_APPLIANCE_IDENTIFICATION_COMPANY_ID                      0
#define ATTR_DEFAULT_APPLIANCE_IDENTIFICATION_BRAND_NAME                      " "
#define ATTR_DEFAULT_APPLIANCE_IDENTIFICATION_BRAND_ID                        0
#define ATTR_DEFAULT_APPLIANCE_IDENTIFICATION_MODEL                           {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
#define ATTR_DEFAULT_APPLIANCE_IDENTIFICATION_PART_NUMBER                     {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
#define ATTR_DEFAULT_APPLIANCE_IDENTIFICATION_PRODUCT_REVISION                {0,0,0,0,0,0}
#define ATTR_DEFAULT_APPLIANCE_IDENTIFICATION_SOFTWARE_REVISION               {0,0,0,0,0,0}
#define ATTR_DEFAULT_APPLIANCE_IDENTIFICATION_PRODUCT_TYPE_NAME               {0,0}
#define ATTR_DEFAULT_APPLIANCE_IDENTIFICATION_PRODUCT_TYPE_ID                 0
#define ATTR_DEFAULT_APPLIANCE_IDENTIFICATION_CECED_SPECIFICATION_VERSION     0

// Product Type IDs
#define PRODUCT_TYPE_ID_WHITE_GOODS           0x0000
#define PRODUCT_TYPE_ID_DISHWASHER            0x5601
#define PRODUCT_TYPE_ID_TUMBLE_DRYER          0x5602
#define PRODUCT_TYPE_ID_WASHER_DRYER          0x5603
#define PRODUCT_TYPE_ID_WASHING_MACHINE       0x5604
#define PRODUCT_TYPE_ID_HOBS                  0x5E03
#define PRODUCT_TYPE_ID_INDUCTION_HOBS        0x5E09
#define PRODUCT_TYPE_ID_OVEN                  0x5E01
#define PRODUCT_TYPE_ID_ELECTRICAL_OVEN       0x5E06
#define PRODUCT_TYPE_ID_REFRIGERATOR_FREEZER  0x6601

// Section 9.8.2.15 of HA 1.2 Spec 11-5474-47, Table 9.49 CECED Specification Version
#define CECED_VERSION_1_0_NC     0x10  // Compliant with v1.0, not certified
#define CECED_VERSION_1_0_C      0x1A  // Compliant with v1.0, certified

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

#endif // ZCL_APPLIANCE_IDENTIFICATION
#endif /* ZCL_APPLIANCE_IDENTIFICATION_H */
