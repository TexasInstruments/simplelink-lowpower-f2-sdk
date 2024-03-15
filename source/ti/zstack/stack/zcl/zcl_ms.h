/**************************************************************************************************
  Filename:       zcl_ms.h
  Revised:        $Date: 2013-10-16 16:38:58 -0700 (Wed, 16 Oct 2013) $
  Revision:       $Revision: 35701 $

  Description:    This file contains the ZCL Measurement and Sensing Definitions


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

#ifndef ZCL_MS_H
#define ZCL_MS_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "zcl.h"
#include "ti_zstack_config.h"

#ifdef ZCL_MS
/*********************************************************************
 * CONSTANTS
 */


/*****************************************************************************/
/***    Electrical Measurement Cluster Attributes                         ***/
/*****************************************************************************/
#define ATTRID_ELECTRICAL_MEASUREMENT_MEASUREMENT_TYPE 			                            0x0000
#define ATTRID_ELECTRICAL_MEASUREMENT_DC_VOLTAGE 			                                  0x0100
#define ATTRID_ELECTRICAL_MEASUREMENT_DC_VOLTAGE_MIN 		                	              0x0101
#define ATTRID_ELECTRICAL_MEASUREMENT_DC_VOLTAGE_MAX 		                	              0x0102
#define ATTRID_ELECTRICAL_MEASUREMENT_DC_CURRENT 		                    	              0x0103
#define ATTRID_ELECTRICAL_MEASUREMENT_DC_CURRENT_MIN 	                		              0x0104
#define ATTRID_ELECTRICAL_MEASUREMENT_DC_CURRENT_MAX 	                		              0x0105
#define ATTRID_ELECTRICAL_MEASUREMENT_DC_POWER 			                                    0x0106
#define ATTRID_ELECTRICAL_MEASUREMENT_DC_POWER_MIN 		                    	            0x0107
#define ATTRID_ELECTRICAL_MEASUREMENT_DC_POWER_MAX 		                    	            0x0108
#define ATTRID_ELECTRICAL_MEASUREMENT_DC_VOLTAGE_MULTIPLIER         			              0x0200
#define ATTRID_ELECTRICAL_MEASUREMENT_DC_VOLTAGE_DIVISOR 	              		            0x0201
#define ATTRID_ELECTRICAL_MEASUREMENT_DC_CURRENT_MULTIPLIER 		        	              0x0202
#define ATTRID_ELECTRICAL_MEASUREMENT_DC_CURRENT_DIVISOR 		            	              0x0203
#define ATTRID_ELECTRICAL_MEASUREMENT_DC_POWER_MULTIPLIER 	            		            0x0204
#define ATTRID_ELECTRICAL_MEASUREMENT_DC_POWER_DIVISOR 		                	            0x0205
#define ATTRID_ELECTRICAL_MEASUREMENT_AC_FREQUENCY 		                    	            0x0300
#define ATTRID_ELECTRICAL_MEASUREMENT_AC_FREQUENCY_MIN 	                		            0x0301
#define ATTRID_ELECTRICAL_MEASUREMENT_AC_FREQUENCY_MAX 	                		            0x0302
#define ATTRID_ELECTRICAL_MEASUREMENT_NEUTRAL_CURRENT 		                	            0x0303
#define ATTRID_ELECTRICAL_MEASUREMENT_TOTAL_ACTIVE_POWER 		            	              0x0304
#define ATTRID_ELECTRICAL_MEASUREMENT_TOTAL_REACTIVE_POWER 		            	            0x0305
#define ATTRID_ELECTRICAL_MEASUREMENT_TOTAL_APPARENT_POWER 		            	            0x0306
#define ATTRID_ELECTRICAL_MEASUREMENT_MEASURED_1ST_HARMONIC_CURRENT 			              0x0307
#define ATTRID_ELECTRICAL_MEASUREMENT_MEASURED_3RD_HARMONIC_CURRENT 			              0x0308
#define ATTRID_ELECTRICAL_MEASUREMENT_MEASURED_5TH_HARMONIC_CURRENT 			              0x0309
#define ATTRID_ELECTRICAL_MEASUREMENT_MEASURED_7TH_HARMONIC_CURRENT 			              0x030A
#define ATTRID_ELECTRICAL_MEASUREMENT_MEASURED_9TH_HARMONIC_CURRENT 			              0x030B
#define ATTRID_ELECTRICAL_MEASUREMENT_MEASURED_1_1TH_HARMONIC_CURRENT 		  	          0x030C
#define ATTRID_ELECTRICAL_MEASUREMENT_MEASURED_PHASE_1ST_HARMONIC_CURRENT 		          0x030D
#define ATTRID_ELECTRICAL_MEASUREMENT_MEASURED_PHASE_3RD_HARMONIC_CURRENT 		          0x030E
#define ATTRID_ELECTRICAL_MEASUREMENT_MEASURED_PHASE_5TH_HARMONIC_CURRENT 		          0x030F
#define ATTRID_ELECTRICAL_MEASUREMENT_MEASURED_PHASE_7TH_HARMONIC_CURRENT 		          0x0310
#define ATTRID_ELECTRICAL_MEASUREMENT_MEASURED_PHASE_9TH_HARMONIC_CURRENT 		          0x0311
#define ATTRID_ELECTRICAL_MEASUREMENT_MEASURED_PHASE_1_1TH_HARMONIC_CURRENT 	          0x0312
#define ATTRID_ELECTRICAL_MEASUREMENT_AC_FREQUENCY_MULTIPLIER 		        	            0x0400
#define ATTRID_ELECTRICAL_MEASUREMENT_AC_FREQUENCY_DIVISOR 		            	            0x0401
#define ATTRID_ELECTRICAL_MEASUREMENT_POWER_MULTIPLIER 			                            0x0402
#define ATTRID_ELECTRICAL_MEASUREMENT_POWER_DIVISOR 		                	              0x0403
#define ATTRID_ELECTRICAL_MEASUREMENT_HARMONIC_CURRENT_MULTIPLIER 	    		            0x0404
#define ATTRID_ELECTRICAL_MEASUREMENT_PHASE_HARMONIC_CURRENT_MULTIPLIER 		            0x0405
#define ATTRID_ELECTRICAL_MEASUREMENT_LINE_CURRENT 		                    	            0x0501
#define ATTRID_ELECTRICAL_MEASUREMENT_ACTIVE_CURRENT 	                		              0x0502
#define ATTRID_ELECTRICAL_MEASUREMENT_REACTIVE_CURRENT 	                		            0x0503
#define ATTRID_ELECTRICAL_MEASUREMENT_RMS_VOLTAGE 		                    	            0x0505
#define ATTRID_ELECTRICAL_MEASUREMENT_RMS_VOLTAGE_MIN 	                		            0x0506
#define ATTRID_ELECTRICAL_MEASUREMENT_RMS_VOLTAGE_MAX 	                		            0x0507
#define ATTRID_ELECTRICAL_MEASUREMENT_RMS_CURRENT 		                    	            0x0508
#define ATTRID_ELECTRICAL_MEASUREMENT_RMS_CURRENT_MIN 	                		            0x0509
#define ATTRID_ELECTRICAL_MEASUREMENT_RMS_CURRENT_MAX 	                		            0x050A
#define ATTRID_ELECTRICAL_MEASUREMENT_ACTIVE_POWER 		                    	            0x050B
#define ATTRID_ELECTRICAL_MEASUREMENT_ACTIVE_POWER_MIN 	                		            0x050C
#define ATTRID_ELECTRICAL_MEASUREMENT_ACTIVE_POWER_MAX 	                		            0x050D
#define ATTRID_ELECTRICAL_MEASUREMENT_REACTIVE_POWER 	                		              0x050E
#define ATTRID_ELECTRICAL_MEASUREMENT_APPARENT_POWER 		                	              0x050F
#define ATTRID_ELECTRICAL_MEASUREMENT_POWER_FACTOR 			                                0x0510
#define ATTRID_ELECTRICAL_MEASUREMENT_AVERAGE_RMS_VOLTAGE_MEASUREMENT_PERIOD 	          0x0511
#define ATTRID_ELECTRICAL_MEASUREMENT_AVERAGE_RMS_OVER_VOLTAGE_COUNTER 			            0x0512
#define ATTRID_ELECTRICAL_MEASUREMENT_AVERAGE_RMS_UNDER_VOLTAGE_COUNTER 		            0x0513
#define ATTRID_ELECTRICAL_MEASUREMENT_RMS_EXTREME_OVER_VOLTAGE_PERIOD 			            0x0514
#define ATTRID_ELECTRICAL_MEASUREMENT_RMS_EXTREME_UNDER_VOLTAGE_PERIOD 			            0x0515
#define ATTRID_ELECTRICAL_MEASUREMENT_RMS_VOLTAGE_SAG_PERIOD 		        	              0x0516
#define ATTRID_ELECTRICAL_MEASUREMENT_RMS_VOLTAGE_SWELL_PERIOD 		        	            0x0517
#define ATTRID_ELECTRICAL_MEASUREMENT_AC_VOLTAGE_MULTIPLIER 	        		              0x0600
#define ATTRID_ELECTRICAL_MEASUREMENT_AC_VOLTAGE_DIVISOR 		            	              0x0601
#define ATTRID_ELECTRICAL_MEASUREMENT_AC_CURRENT_MULTIPLIER 	        		              0x0602
#define ATTRID_ELECTRICAL_MEASUREMENT_AC_CURRENT_DIVISOR 		            	              0x0603
#define ATTRID_ELECTRICAL_MEASUREMENT_AC_POWER_MULTIPLIER 		            	            0x0604
#define ATTRID_ELECTRICAL_MEASUREMENT_AC_POWER_DIVISOR 			                            0x0605
#define ATTRID_ELECTRICAL_MEASUREMENT_DC_OVERLOAD_ALARMS_MASK 	        		            0x0700
#define ATTRID_ELECTRICAL_MEASUREMENT_DC_VOLTAGE_OVERLOAD 		            	            0x0701
#define ATTRID_ELECTRICAL_MEASUREMENT_DC_CURRENT_OVERLOAD 		            	            0x0702
#define ATTRID_ELECTRICAL_MEASUREMENT_AC_ALARMS_MASK 			                              0x0800
#define ATTRID_ELECTRICAL_MEASUREMENT_AC_VOLTAGE_OVERLOAD 		            	            0x0801
#define ATTRID_ELECTRICAL_MEASUREMENT_AC_CURRENT_OVERLOAD 		            	            0x0802
#define ATTRID_ELECTRICAL_MEASUREMENT_AC_ACTIVE_POWER_OVERLOAD 	        		            0x0803
#define ATTRID_ELECTRICAL_MEASUREMENT_AC_REACTIVE_POWER_OVERLOAD 	    		              0x0804
#define ATTRID_ELECTRICAL_MEASUREMENT_AVERAGE_RMS_OVER_VOLTAGE 		        	            0x0805
#define ATTRID_ELECTRICAL_MEASUREMENT_AVERAGE_RMS_UNDER_VOLTAGE 	       		            0x0806
#define ATTRID_ELECTRICAL_MEASUREMENT_RMS_EXTREME_OVER_VOLTAGE 	        		            0x0807
#define ATTRID_ELECTRICAL_MEASUREMENT_RMS_EXTREME_UNDER_VOLTAGE        			            0x0808
#define ATTRID_ELECTRICAL_MEASUREMENT_RMS_VOLTAGE_SAG 		                	            0x0809
#define ATTRID_ELECTRICAL_MEASUREMENT_RMS_VOLTAGE_SWELL 	            		              0x080A
#define ATTRID_ELECTRICAL_MEASUREMENT_LINE_CURRENT_PH_B 	            		              0x0901
#define ATTRID_ELECTRICAL_MEASUREMENT_ACTIVE_CURRENT_PH_B 	            		            0x0902
#define ATTRID_ELECTRICAL_MEASUREMENT_REACTIVE_CURRENT_PH_B 	        		              0x0903
#define ATTRID_ELECTRICAL_MEASUREMENT_RMS_VOLTAGE_PH_B 		                	            0x0905
#define ATTRID_ELECTRICAL_MEASUREMENT_RMS_VOLTAGE_MIN_PH_B 	            		            0x0906
#define ATTRID_ELECTRICAL_MEASUREMENT_RMS_VOLTAGE_MAX_PH_B 	            		            0x0907
#define ATTRID_ELECTRICAL_MEASUREMENT_RMS_CURRENT_PH_B 		                	            0x0908
#define ATTRID_ELECTRICAL_MEASUREMENT_RMS_CURRENT_MIN_PH_B 	            		            0x0909
#define ATTRID_ELECTRICAL_MEASUREMENT_RMS_CURRENT_MAX_PH_B 		            	            0x090A
#define ATTRID_ELECTRICAL_MEASUREMENT_ACTIVE_POWER_PH_B 		            	              0x090B
#define ATTRID_ELECTRICAL_MEASUREMENT_ACTIVE_POWER_MIN_PH_B 	        		              0x090C
#define ATTRID_ELECTRICAL_MEASUREMENT_ACTIVE_POWER_MAX_PH_B 	        		              0x090D
#define ATTRID_ELECTRICAL_MEASUREMENT_REACTIVE_POWER_PH_B 		            	            0x090E
#define ATTRID_ELECTRICAL_MEASUREMENT_APPARENT_POWER_PH_B 		            	            0x090F
#define ATTRID_ELECTRICAL_MEASUREMENT_POWER_FACTOR_PH_B 		            	              0x0910
#define ATTRID_ELECTRICAL_MEASUREMENT_AVERAGE_RMS_VOLTAGE_MEASUREMENT_PERIOD_PH_B       0x0911
#define ATTRID_ELECTRICAL_MEASUREMENT_AVERAGE_RMS_OVER_VOLTAGE_COUNTER_PH_B 	          0x0912
#define ATTRID_ELECTRICAL_MEASUREMENT_AVERAGE_RMS_UNDER_VOLTAGE_COUNTER_PH_B 	          0x0913
#define ATTRID_ELECTRICAL_MEASUREMENT_RMS_EXTREME_OVER_VOLTAGE_PERIOD_PH_B 		          0x0914
#define ATTRID_ELECTRICAL_MEASUREMENT_RMS_EXTREME_UNDER_VOLTAGE_PERIOD_PH_B 	          0x0915
#define ATTRID_ELECTRICAL_MEASUREMENT_RMS_VOLTAGE_SAG_PERIOD_PH_B 			                0x0916
#define ATTRID_ELECTRICAL_MEASUREMENT_RMS_VOLTAGE_SWELL_PERIOD_PH_B 			              0x0917
#define ATTRID_ELECTRICAL_MEASUREMENT_LINE_CURRENT_PH_C 			                          0x0A01
#define ATTRID_ELECTRICAL_MEASUREMENT_ACTIVE_CURRENT_PH_C 			                        0x0A02
#define ATTRID_ELECTRICAL_MEASUREMENT_REACTIVE_CURRENT_PH_C 	        		              0x0A03
#define ATTRID_ELECTRICAL_MEASUREMENT_RMS_VOLTAGE_PH_C 		                   	          0x0A05
#define ATTRID_ELECTRICAL_MEASUREMENT_RMS_VOLTAGE_MIN_PH_C 		            	            0x0A06
#define ATTRID_ELECTRICAL_MEASUREMENT_RMS_VOLTAGE_MAX_PH_C 		            	            0x0A07
#define ATTRID_ELECTRICAL_MEASUREMENT_RMS_CURRENT_PH_C 			                            0x0A08
#define ATTRID_ELECTRICAL_MEASUREMENT_RMS_CURRENT_MIN_PH_C 	            		            0x0A09
#define ATTRID_ELECTRICAL_MEASUREMENT_RMS_CURRENT_MAX_PH_C 		            	            0x0A0A
#define ATTRID_ELECTRICAL_MEASUREMENT_ACTIVE_POWER_PH_C 		            	              0x0A0B
#define ATTRID_ELECTRICAL_MEASUREMENT_ACTIVE_POWER_MIN_PH_C 	        		              0x0A0C
#define ATTRID_ELECTRICAL_MEASUREMENT_ACTIVE_POWER_MAX_PH_C 	        		              0x0A0D
#define ATTRID_ELECTRICAL_MEASUREMENT_REACTIVE_POWER_PH_C 		              	          0x0A0E
#define ATTRID_ELECTRICAL_MEASUREMENT_APPARENT_POWER_PH_C 		            	            0x0A0F
#define ATTRID_ELECTRICAL_MEASUREMENT_POWER_FACTOR_PH_C 		            	              0x0A10
#define ATTRID_ELECTRICAL_MEASUREMENT_AVERAGE_RMS_VOLTAGE_MEASUREMENT_PERIOD_PH_C       0x0A11
#define ATTRID_ELECTRICAL_MEASUREMENT_AVERAGE_RMS_OVER_VOLTAGE_COUNTER_PH_C 	          0x0A12
#define ATTRID_ELECTRICAL_MEASUREMENT_AVERAGE_RMS_UNDER_VOLTAGE_COUNTER_PH_C 	          0x0A13
#define ATTRID_ELECTRICAL_MEASUREMENT_RMS_EXTREME_OVER_VOLTAGE_PERIOD_PH_C 		          0x0A14
#define ATTRID_ELECTRICAL_MEASUREMENT_RMS_EXTREME_UNDER_VOLTAGE_PERIOD_PH_C 	          0x0A15
#define ATTRID_ELECTRICAL_MEASUREMENT_RMS_VOLTAGE_SAG_PERIOD_PH_C 			                0x0A16
#define ATTRID_ELECTRICAL_MEASUREMENT_RMS_VOLTAGE_SWELL_PERIOD_PH_C 			              0x0A17

/*****************************************************************************/
/***    Electrical Measurement Cluster Commands                         ***/
/*****************************************************************************/
#define COMMAND_ELECTRICAL_MEASUREMENT_GET_PROFILE_INFO_RESPONSE 			      0x00
#define COMMAND_ELECTRICAL_MEASUREMENT_GET_MEASUREMENT_PROFILE_RESPONSE 		0x01
#define COMMAND_ELECTRICAL_MEASUREMENT_GET_PROFILE_INFO 			              0x00
#define COMMAND_ELECTRICAL_MEASUREMENT_GET_MEASUREMENT_PROFILE 			        0x01

/*****************************************************************************/
/***    Illuminance Measurement Cluster Attributes                         ***/
/*****************************************************************************/
    // Illuminance Measurement Information attribute set
#define ATTRID_ILLUMINANCE_MEASUREMENT_MEASURED_VALUE                             0x0000
#define ATTRID_ILLUMINANCE_MEASUREMENT_MIN_MEASURED_VALUE                         0x0001
#define ATTRID_ILLUMINANCE_MEASUREMENT_MAX_MEASURED_VALUE                         0x0002
#define ATTRID_ILLUMINANCE_MEASUREMENT_TOLERANCE                                  0x0003
#define ATTRID_ILLUMINANCE_MEASUREMENT_LIGHT_SENSOR_TYPE                          0x0004

    // Illuminance Measurement Settings attribute set
// #define ATTRID_MS_ILLUMINANCE_MIN_PERCENT_CHANGE                         0x0100
// #define ATTRID_MS_ILLUMINANCE_MIN_ABSOLUTE_CHANGE                        0x0101

  /*** Light Sensor Type attribute values ***/
#define MS_ILLUMINANCE_LIGHT_SENSOR_PHOTODIODE                           0x00
#define MS_ILLUMINANCE_LIGHT_SENSOR_CMOS                                 0x01
#define MS_ILLUMINANCE_LIGHT_SENSOR_UNKNOWN                              0xFF

/*****************************************************************************/
/***    Illuminance Level Sensing Configuration Cluster Attributes         ***/
/*****************************************************************************/
    // Illuminance Level Sensing Information attribute set
#define ATTRID_ILLUMINANCE_LEVEL_SENSING_LEVEL_STATUS                 0x0000
#define ATTRID_ILLUMINANCE_LEVEL_SENSING_LIGHT_SENSOR_TYPE            0x0001

/***  Level Status attribute values  ***/
#define MS_ILLUMINANCE_LEVEL_ON_TARGET                                   0x00
#define MS_ILLUMINANCE_LEVEL_BELOW_TARGET                                0x01
#define MS_ILLUMINANCE_LEVEL_ABOVE_TARGET                                0x02

/***  Light Sensor Type attribute values  ***/
#define MS_ILLUMINANCE_LEVEL_LIGHT_SENSOR_PHOTODIODE                     0x00
#define MS_ILLUMINANCE_LEVEL_LIGHT_SENSOR_CMOS                           0x01
#define MS_ILLUMINANCE_LEVEL_LIGHT_SENSOR_UNKNOWN                        0xFF

    // Illuminance Level Sensing Settings attribute set
#define ATTRID_ILLUMINANCE_LEVEL_SENSING_ILLUMINANCE_TARGET_LEVEL     0x0010

/*****************************************************************************/
/***    Temperature Measurement Cluster Attributes                         ***/
/*****************************************************************************/
  // Temperature Measurement Information attributes set
#define ATTRID_TEMPERATURE_MEASUREMENT_MEASURED_VALUE                             0x0000 // M, R, int16_t
#define ATTRID_TEMPERATURE_MEASUREMENT_MIN_MEASURED_VALUE                         0x0001 // M, R, int16_t
#define ATTRID_TEMPERATURE_MEASUREMENT_MAX_MEASURED_VALUE                         0x0002 // M, R, int16_t
#define ATTRID_TEMPERATURE_MEASUREMENT_TOLERANCE                                  0x0003 // O, R, uint16_t

  // Temperature Measurement Settings attributes set
#define ATTRID_TEMPERATURE_MEASUREMENT_MIN_PERCENT_CHANGE                         0x0010
#define ATTRID_TEMPERATURE_MEASUREMENT_MIN_ABSOLUTE_CHANGE                        0x0011

/*****************************************************************************/
/***    Pressure Measurement Cluster Attributes                            ***/
/*****************************************************************************/
  // Pressure Measurement Information attribute set
#define ATTRID_PRESSURE_MEASUREMENT_MEASURED_VALUE                    0x0000
#define ATTRID_PRESSURE_MEASUREMENT_MIN_MEASURED_VALUE                0x0001
#define ATTRID_PRESSURE_MEASUREMENT_MAX_MEASURED_VALUE                0x0002
#define ATTRID_PRESSURE_MEASUREMENT_TOLERANCE                         0x0003
#define ATTRID_PRESSURE_MEASUREMENT_SCALED_VALUE                      0x0010
#define ATTRID_PRESSURE_MEASUREMENT_MIN_SCALED_VALUE                  0x0011
#define ATTRID_PRESSURE_MEASUREMENT_MAX_SCALED_VALUE                  0x0012
#define ATTRID_PRESSURE_MEASUREMENT_SCALED_TOLERANCE                  0x0013
#define ATTRID_PRESSURE_MEASUREMENT_SCALE                             0x0014


  // Pressure Measurement Settings attribute set
// #define ATTRID_MS_PRESSURE_MEASUREMENT_MIN_PERCENT_CHANGE                0x0100
// #define ATTRID_MS_PRESSURE_MEASUREMENT_MIN_ABSOLUTE_CHANGE               0x0101

/*****************************************************************************/
/***        Flow Measurement Cluster Attributes                            ***/
/*****************************************************************************/
  // Flow Measurement Information attribute set
#define ATTRID_FLOW_MEASUREMENT_MEASURED_VALUE                        0x0000
#define ATTRID_FLOW_MEASUREMENT_MIN_MEASURED_VALUE                    0x0001
#define ATTRID_FLOW_MEASUREMENT_MAX_MEASURED_VALUE                    0x0002
#define ATTRID_FLOW_MEASUREMENT_TOLERANCE                             0x0003

  // Flow Measurement Settings attribute set
// #define ATTRID_MS_FLOW_MEASUREMENT_MIN_PERCENT_CHANGE                    0x0100
// #define ATTRID_MS_FLOW_MEASUREMENT_MIN_ABSOLUTE_CHANGE                   0x0101

/*****************************************************************************/
/***        Relativity Humidity Cluster Attributes                           ***/
/*****************************************************************************/
  // Relativity Humidity Information attribute set
#define ATTRID_RELATIVITY_HUMIDITY_MEASURED_VALUE                       0x0000
#define ATTRID_RELATIVITY_HUMIDITY_MIN_MEASURED_VALUE                   0x0001
#define ATTRID_RELATIVITY_HUMIDITY_MAX_MEASURED_VALUE                   0x0002
#define ATTRID_RELATIVITY_HUMIDITY_TOLERANCE                            0x0003

/*****************************************************************************/
/***         Occupancy Sensing Cluster Attributes                          ***/
/*****************************************************************************/
    // Occupancy Sensor Configuration attribute set
#define ATTRID_OCCUPANCY_SENSING_OCCUPANCY                              0x0000 // M, R, BITMAP8
#define ATTRID_OCCUPANCY_SENSING_OCCUPANCY_SENSOR_TYPE                  0x0001 // M, R, ENUM8
#define ATTRID_OCCUPANCY_SENSING_OCCUPANCY_SENSOR_TYPE_BITMAP           0x0002

/*** Occupancy Sensor Type Attribute values ***/
#define MS_OCCUPANCY_SENSOR_TYPE_PIR                                     0x00
#define MS_OCCUPANCY_SENSOR_TYPE_ULTRASONIC                              0x01
#define MS_OCCUPANCY_SENSOR_TYPE_PIR_AND_ULTRASONIC                      0x02

    // PIR Configuration attribute set
#define ATTRID_OCCUPANCY_SENSING_PIR_OCCUPIED_TO_UNOCCUPIED_DELAY                       0x0010 // O, R/W, uint16_t
#define ATTRID_OCCUPANCY_SENSING_PIR_UNOCCUPIED_TO_OCCUPIED_DELAY                       0x0011 // O, R/W, uint16_t
#define ATTRID_OCCUPANCY_SENSING_PIR_UNOCCUPIED_TO_OCCUPIED_THRESHOLD                   0x0012 // O, R/W, uint8_t

    // Ultrasonic Configuration attribute set
#define ATTRID_OCCUPANCY_SENSING_ULTRASONIC_OCCUPIED_TO_UNOCCUPIED_DELAY                0x0020 // O, R/W, uint16_t
#define ATTRID_OCCUPANCY_SENSING_ULTRASONIC_UNOCCUPIED_TO_OCCUPIED_DELAY                0x0021 // O, R/W, uint16_t
#define ATTRID_OCCUPANCY_SENSING_ULTRASONIC_UNOCCUPIED_TO_OCCUPIED_THRESHOLD            0x0022 // O, R/W, uint8_t

    // Physical Contact Configuration attribute set
#define ATTRID_OCCUPANCY_SENSING_PHYSICAL_CONTACT_OCCUPIED_TO_UNOCCUPIED_DELAY          0x0030 // O, R/W, uint16_t
#define ATTRID_OCCUPANCY_SENSING_PHYSICAL_CONTACT_UNOCCUPIED_TO_OCCUPIED_DELAY          0x0031 // O, R/W, uint16_t
#define ATTRID_OCCUPANCY_SENSING_PHYSICAL_CONTACT_UNOCCUPIED_TO_OCCUPIED_THRESHOLD      0x0032 // O, R/W, uint8_t

/************************************************************************************
 * MACROS
 */


/****************************************************************************
 * TYPEDEFS
 */

typedef void (*zclMS_PlaceHolder_t)( void );

// Register Callbacks table entry - enter function pointers for callbacks that
// the application would like to receive
typedef struct
{
  zclMS_PlaceHolder_t               pfnMSPlaceHolder; // Place Holder
//  NULL
} zclMS_AppCallbacks_t;


/****************************************************************************
 * VARIABLES
 */


/****************************************************************************
 * FUNCTIONS
 */

 /*
  * Register for callbacks from this cluster library
  */
extern ZStatus_t zclMS_RegisterCmdCallbacks( uint8_t endpoint, zclMS_AppCallbacks_t *callbacks );

#endif // ZCL_MS

#ifdef __cplusplus
}
#endif

#endif /* ZCL_MS_H */
