/**************************************************************************************************
  Filename:       gpd_temperaturesensor_data.c
  Revised:        $Date: 2018-01-03 15:00:02 -0700 (Wednesday, 03 Jan 2014) $
  Revision:       $Revision: 38502 $


  Description:    Green Power Device Temperature Sensor data for application


  Copyright 2006-2014 Texas Instruments Incorporated. All rights reserved.

  IMPORTANT: Your use of this Software is limited to those specific rights
  granted under the terms of a software license agreement between the user
  who downloaded the software, his/her employer (which must be your employer)
  and Texas Instruments Incorporated (the "License").  You may not use this
  Software unless you agree to abide by the terms of the License. The License
  limits your use, and you acknowledge, that the Software may not be modified,
  copied or distributed unless embedded on a Texas Instruments microcontroller
  or used solely and exclusively in conjunction with a Texas Instruments radio
  frequency transceiver, which is integrated into your product.  Other than for
  the foregoing purpose, you may not use, reproduce, copy, prepare derivative
  works of, modify, distribute, perform, display or sell this Software and/or
  its documentation for any purpose.

  YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE
  PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
  INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, TITLE,
  NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL
  TEXAS INSTRUMENTS OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER CONTRACT,
  NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER
  LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
  INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE
  OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT
  OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
  (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.

  Should you have any questions regarding your right to use this Software,
  contact Texas Instruments Incorporated at www.TI.com.
**************************************************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include "gpd.h"
#include "gpd_common.h"

/*********************************************************************
 * CONSTANTS
 */

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */

uint16_t gpd_attr_CurrentTemperature = 17;    //Actual attribute to be reported by GPD



/*              */
gpdCommissioningCommand_t commissiongCmd =
{
    DEVICE_ID,                                //gpdDeviceID
    GPD_COMMISIONING_OPTIONS,                 //options
#if ( OPT_EXTENDED_OPTIONS == TRUE )
    GPD_COMMISIONING_EXOPTIONS,               //extendedOptions
#endif
#if (EXOPT_GPD_KEY_PRESENT == TRUE)
    GP_SECURITY_KEY,                          //GPDKey
#if (EXOPT_KEY_ENCRYPTION == TRUE)
    0x00000000,                               //GPDKeyMIC
#endif                                        // EXOPT_KEY_ENCRYPTION
    0x00000000,                               //GPDOutgoingCounter
#endif                                        // EXOPT_GPD_KEY_PRESENT

#if (OPT_APPLICATION_INFORMATION == TRUE)
    0x20,                                     //applicationInformation
#endif
                                              //manufacturerID
                                              //modelID
                                              //numberGPDCommands
                                              //GPDCommandIDList
                                              //clusterList
                                              //switchInformation
};

/*              */
gpdfReq_t commissioningReq =
{
    GPD_FRAME_CONTROL,                        //NWKFrameControl
    GPD_UNSECURED_EXT_FRAME_CONTROL,          //NWKExtFC
    {
      (uint8_t)GPD_APP_ID,                      //gpd_ID_t.AppID
      {
#if (GPD_APP_ID == GPD_APP_TYPE_SRC_ID)
        .SrcID = GPD_ID,                      //union GPDId.SrcID
#elif (GPD_APP_ID == GPD_APP_TYPE_IEEE_ID)
        .GPDExtAddr = GPD_ID,                 //union GPDId.GPDExtAddr
#endif
      },
    },
#if (GPD_APP_ID == GPD_APP_TYPE_IEEE_ID)
    0x0A,                                     //endPoint
#endif
#if (GP_SECURITY_LEVEL == GP_SECURITY_LVL_4FC_4MIC) || (GP_SECURITY_LEVEL == GP_SECURITY_LVL_4FC_4MIC_ENCRYPT)
    0x000000,                                 //securityFrameCounter
#endif
    0xE0,                                     //gpdCmdID
    0,                                        //payloadSize, calculated during command send
    (uint8_t*)&commissiongCmd                   //*payload
};

/*              */
reportDescriptor_t reportDescriptor =
{
  0,                                          // report identifier
  APPDESC_REPORT_OPTIONS,
  0,                                          // timeout period
  8,                                          // remainig length of report description
  APPDESC_DATA_POINT_OPTIONS,                 // Data Point Options
  ZCL_CLUSTER_ID_MS_TEMPERATURE_MEASUREMENT,  // Cluster ID
  0x0000,                                     // Manufacturer ID
  0x0000,                                     // Attribute ID
  ZCL_DATATYPE_INT16,                         // Attribute Data Type
  APPDESC_ATTRIBUTE_OPTIONS,
  0x00,                                       // offset
  0x00,                                       // attribute Value
  NULL                                        // pointer to next report descriptor
};


//Array of attributes to be reported. The number of attributes defined in this array must be the same as the number used in GreenPowerAttributeReportingSend
gpReport_t  attributeReport[1] =
{
 {
  0x0000,    //Attribute ID for measured value
  0x29,      //Data type Signed 16-bit Integer
  (uint8_t*)&gpd_attr_CurrentTemperature,
 },
//  {
//    AttributeID in the same cluster
//    datatype
//    &anotherAttribute
//  }
};


gpdfReq_t attributeReportCmd =
{
    GPD_FRAME_CONTROL,                      //NWKFrameControl
    GPD_EXT_FRAME_CONTROL,                  //NWKExtFC
    {
      (uint8_t)GPD_APP_ID,                           //gpd_ID_t.AppID
      {
#if (GPD_APP_ID == GPD_APP_TYPE_SRC_ID)
        .SrcID = GPD_ID,                    //union GPDId.SrcID
#elif (GPD_APP_ID == GPD_APP_TYPE_IEEE_ID)
        .GPDExtAddr = GPD_ID,               //union GPDId.GPDExtAddr
#endif
      },
    },
#if (GPD_APP_ID == GPD_APP_TYPE_IEEE_ID)
    0x0A,                                   //endPoint
#endif
#if (GP_SECURITY_LEVEL == GP_SECURITY_LVL_4FC_4MIC) || (GP_SECURITY_LEVEL == GP_SECURITY_LVL_4FC_4MIC_ENCRYPT)
    0x000000,                               //securityFrameCounter
#endif
    GP_COMMAND_ATTR_REPORTING,              //gpdCmdID
    0,                                      //payloadSize, calculated during command send
    (uint8_t*)&attributeReport,               //*payload
};



const uint8_t gpdDeviceId = DEVICE_ID;
const uint8_t frameDuplicates = GPDF_FRAME_DUPLICATES;
const uint8_t gpdChannel = GPD_CHANNEL;

/*********************************************************************
 * GLOBAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL FUNCTIONS
 */
#if (GP_SECURITY_LEVEL==GP_SECURITY_LVL_NO_SEC)
#warning: GPD Temperature Sensor is meant to have security level 2 or higher. Disabling security will require several changes into the app and might not be compliant with GP Specification
#endif
/****************************************************************************
****************************************************************************/
