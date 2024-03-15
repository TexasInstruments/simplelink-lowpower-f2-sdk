/**************************************************************************************************
  Filename:       zcl_lighting.h
  Revised:        $Date: 2013-10-16 16:38:58 -0700 (Wed, 16 Oct 2013) $
  Revision:       $Revision: 35701 $

  Description:    This file contains the ZCL Lighting library definitions.


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

#ifndef ZCL_LIGHTING_H
#define ZCL_LIGHTING_H

#ifdef __cplusplus
extern "C"
{
#endif

/******************************************************************************
 * INCLUDES
 */
#include "zcl.h"
#include "ti_zstack_config.h"

#ifdef ZCL_LIGHTING
/******************************************************************************
 * CONSTANTS
 */

/*****************************************/
/***  Color Control Cluster Attributes ***/
/*****************************************/
  // Color Information attributes set
#define ATTRID_COLOR_CONTROL_CURRENT_HUE                        0x0000
#define ATTRID_COLOR_CONTROL_CURRENT_SATURATION                 0x0001
#define ATTRID_COLOR_CONTROL_REMAINING_TIME                     0x0002
#define ATTRID_COLOR_CONTROL_CURRENT_X                          0x0003
#define ATTRID_COLOR_CONTROL_CURRENT_Y                          0x0004
#define ATTRID_COLOR_CONTROL_DRIFT_COMPENSATION                 0x0005
#define ATTRID_COLOR_CONTROL_COMPENSATION_TEXT                  0x0006
#define ATTRID_COLOR_CONTROL_COLOR_TEMPERATURE_MIREDS           0x0007
#define ATTRID_COLOR_CONTROL_COLOR_MODE                         0x0008
#define ATTRID_COLOR_CONTROL_OPTIONS                            0x000F

  // Defined Primaries Inofrmation attribute Set
#define ATTRID_COLOR_CONTROL_NUMBER_OF_PRIMARIES                0x0010
#define ATTRID_COLOR_CONTROL_PRIMARY_1_X                        0x0011
#define ATTRID_COLOR_CONTROL_PRIMARY_1_Y                        0x0012
#define ATTRID_COLOR_CONTROL_PRIMARY_1_INTENSITY                0x0013
  // 0x0014 is reserved
#define ATTRID_COLOR_CONTROL_PRIMARY_2_X                        0x0015
#define ATTRID_COLOR_CONTROL_PRIMARY_2_Y                        0x0016
#define ATTRID_COLOR_CONTROL_PRIMARY_2_INTENSITY                0x0017
  // 0x0018 is reserved
#define ATTRID_COLOR_CONTROL_PRIMARY_3_X                        0x0019
#define ATTRID_COLOR_CONTROL_PRIMARY_3_Y                        0x001a
#define ATTRID_COLOR_CONTROL_PRIMARY_3_INTENSITY                0x001b

  // Additional Defined Primaries Information attribute set
#define ATTRID_COLOR_CONTROL_PRIMARY_4_X                        0x0020
#define ATTRID_COLOR_CONTROL_PRIMARY_4_Y                        0x0021
#define ATTRID_COLOR_CONTROL_PRIMARY_4_INTENSITY                0x0022
  // 0x0023 is reserved
#define ATTRID_COLOR_CONTROL_PRIMARY_5_X                        0x0024
#define ATTRID_COLOR_CONTROL_PRIMARY_5_Y                        0x0025
#define ATTRID_COLOR_CONTROL_PRIMARY_5_INTENSITY                0x0026
  // 0x0027 is reserved
#define ATTRID_COLOR_CONTROL_PRIMARY_6_X                        0x0028
#define ATTRID_COLOR_CONTROL_PRIMARY_6_Y                        0x0029
#define ATTRID_COLOR_CONTROL_PRIMARY_6_INTENSITY                0x002a

  // Defined Color Points Settings attribute set
#define ATTRID_COLOR_CONTROL_WHITE_POINT_X                      0x0030
#define ATTRID_COLOR_CONTROL_WHITE_POINT_Y                      0x0031
#define ATTRID_COLOR_CONTROL_COLOR_POINT_RX                     0x0032
#define ATTRID_COLOR_CONTROL_COLOR_POINT_RY                     0x0033
#define ATTRID_COLOR_CONTROL_COLOR_POINT_R_INTENSITY            0x0034
  // 0x0035 is reserved
#define ATTRID_COLOR_CONTROL_COLOR_POINT_GX                     0x0036
#define ATTRID_COLOR_CONTROL_COLOR_POINT_GY                     0x0037
#define ATTRID_COLOR_CONTROL_COLOR_POINT_G_INTENSITY            0x0038
  // 0x0039 is reserved
#define ATTRID_COLOR_CONTROL_COLOR_POINT_BX                     0x003a
#define ATTRID_COLOR_CONTROL_COLOR_POINT_BY                     0x003b
#define ATTRID_COLOR_CONTROL_COLOR_POINT_B_INTENSITY            0x003c
  // 0x003d is reserved
#define ATTRID_COLOR_CONTROL_ENHANCED_CURRENT_HUE               0x4000
#define ATTRID_COLOR_CONTROL_ENHANCED_COLOR_MODE                0x4001
#define ATTRID_COLOR_CONTROL_COLOR_LOOP_ACTIVE                  0x4002
#define ATTRID_COLOR_CONTROL_COLOR_LOOP_DIRECTION               0x4003
#define ATTRID_COLOR_CONTROL_COLOR_LOOP_TIME                    0x4004
#define ATTRID_COLOR_CONTROL_COLOR_LOOP_START_ENHANCED_HUE      0x4005
#define ATTRID_COLOR_CONTROL_COLOR_LOOP_STORED_ENHANCED_HUE     0x4006
#define ATTRID_COLOR_CONTROL_COLOR_CAPABILITIES                 0x400a
#define ATTRID_COLOR_CONTROL_COLOR_TEMP_PHYSICAL_MIN_MIREDS            0x400b
#define ATTRID_COLOR_CONTROL_COLOR_TEMP_PHYSICAL_MAX_MIREDS            0x400c
#define ATTRID_COLOR_CONTROL_COUPLE_COLOR_TEMP_TO_LEVEL_MIN_MIREDS     0x400d
#define ATTRID_COLOR_CONTROL_START_UP_COLOR_TEMPERATURE_MIREDS         0x4010

 /***  Color Information attributes range limits   ***/
#define LIGHTING_COLOR_HUE_MAX                                           0xfe
#define LIGHTING_COLOR_SAT_MAX                                           0xfe
#define LIGHTING_COLOR_REMAINING_TIME_MAX                                0xfffe
#define LIGHTING_COLOR_CURRENT_X_MAX                                     0xfeff
#define LIGHTING_COLOR_CURRENT_Y_MAX                                     0xfeff
#define LIGHTING_COLOR_TEMPERATURE_MAX                                   0xfeff

  /*** Drift Compensation Attribute values ***/
#define DRIFT_COMP_NONE                                                  0x00
#define DRIFT_COMP_OTHER_UNKNOWN                                         0x01
#define DRIFT_COMP_TEMPERATURE_MONITOR                                   0x02
#define DRIFT_COMP_OPTICAL_LUMINANCE_MONITOR_FEEDBACK                    0x03
#define DRIFT_COMP_OPTICAL_COLOR_MONITOR_FEEDBACK                        0x04

  /*** Color Mode Attribute values ***/
#define COLOR_MODE_CURRENT_HUE_SATURATION                                0x00
#define COLOR_MODE_CURRENT_X_Y                                           0x01
#define COLOR_MODE_COLOR_TEMPERATURE                                     0x02

  /*** Enhanced Color Mode Attribute values ***/
#define ENHANCED_COLOR_MODE_CURRENT_HUE_SATURATION                       0x00
#define ENHANCED_COLOR_MODE_CURRENT_X_Y                                  0x01
#define ENHANCED_COLOR_MODE_COLOR_TEMPERATURE                            0x02
#define ENHANCED_COLOR_MODE_ENHANCED_CURRENT_HUE_SATURATION              0x03

  /*** Color Capabilities Attribute bit masks ***/
#define COLOR_CAPABILITIES_ATTR_BIT_HUE_SATURATION                       0x01
#define COLOR_CAPABILITIES_ATTR_BIT_ENHANCED_HUE                         0x02
#define COLOR_CAPABILITIES_ATTR_BIT_COLOR_LOOP                           0x04
#define COLOR_CAPABILITIES_ATTR_BIT_X_Y_ATTRIBUTES                       0x08
#define COLOR_CAPABILITIES_ATTR_BIT_COLOR_TEMPERATURE                    0x10

/*****************************************/
/***  Color Control Cluster Commands   ***/
/*****************************************/
#define COMMAND_COLOR_CONTROL_MOVE_TO_HUE                                     0x00
#define COMMAND_COLOR_CONTROL_MOVE_HUE                                        0x01
#define COMMAND_COLOR_CONTROL_STEP_HUE                                        0x02
#define COMMAND_COLOR_CONTROL_MOVE_TO_SATURATION                              0x03
#define COMMAND_COLOR_CONTROL_MOVE_SATURATION                                 0x04
#define COMMAND_COLOR_CONTROL_STEP_SATURATION                                 0x05
#define COMMAND_COLOR_CONTROL_MOVE_TO_HUE_AND_SATURATION                      0x06
#define COMMAND_COLOR_CONTROL_MOVE_TO_COLOR                                   0x07
#define COMMAND_COLOR_CONTROL_MOVE_COLOR                                      0x08
#define COMMAND_COLOR_CONTROL_STEP_COLOR                                      0x09
#define COMMAND_COLOR_CONTROL_MOVE_TO_COLOR_TEMPERATURE                       0x0a
#define COMMAND_COLOR_CONTROL_ENHANCED_MOVE_TO_HUE                            0x40
#define COMMAND_COLOR_CONTROL_ENHANCED_MOVE_HUE                               0x41
#define COMMAND_COLOR_CONTROL_ENHANCED_STEP_HUE                               0x42
#define COMMAND_COLOR_CONTROL_ENHANCED_MOVE_TO_HUE_AND_SATURATION             0x43
#define COMMAND_COLOR_CONTROL_COLOR_LOOP_SET                                  0x44
#define COMMAND_COLOR_CONTROL_STOP_MOVE_STEP                                  0x47
#define COMMAND_COLOR_CONTROL_MOVE_COLOR_TEMPERATURE                          0x4b
#define COMMAND_COLOR_CONTROL_STEP_COLOR_TEMPERATURE                          0x4c

  /***  Move To Hue Cmd payload: direction field values  ***/
#define LIGHTING_MOVE_TO_HUE_DIRECTION_SHORTEST_DISTANCE                 0x00
#define LIGHTING_MOVE_TO_HUE_DIRECTION_LONGEST_DISTANCE                  0x01
#define LIGHTING_MOVE_TO_HUE_DIRECTION_UP                                0x02
#define LIGHTING_MOVE_TO_HUE_DIRECTION_DOWN                              0x03
  /***  Move Hue Cmd payload: moveMode field values   ***/
#define LIGHTING_MOVE_HUE_STOP                                           0x00
#define LIGHTING_MOVE_HUE_UP                                             0x01
#define LIGHTING_MOVE_HUE_DOWN                                           0x03
  /***  Step Hue Cmd payload: stepMode field values ***/
#define LIGHTING_STEP_HUE_UP                                             0x01
#define LIGHTING_STEP_HUE_DOWN                                           0x03
  /***  Move Saturation Cmd payload: moveMode field values ***/
#define LIGHTING_MOVE_SATURATION_STOP                                    0x00
#define LIGHTING_MOVE_SATURATION_UP                                      0x01
#define LIGHTING_MOVE_SATURATION_DOWN                                    0x03
  /***  Step Saturation Cmd payload: stepMode field values ***/
#define LIGHTING_STEP_SATURATION_UP                                      0x01
#define LIGHTING_STEP_SATURATION_DOWN                                    0x03
  /***  Color Loop Set Cmd payload: action field values  ***/
#define LIGHTING_COLOR_LOOP_ACTION_DEACTIVATE                            0x00
#define LIGHTING_COLOR_LOOP_ACTION_ACTIVATE_FROM_START_HUE               0x01
#define LIGHTING_COLOR_LOOP_ACTION_ACTIVATE_FROM_ENH_CURR_HUE            0x02
  /***  Color Loop Set Cmd payload: direction field values   ***/
#define LIGHTING_COLOR_LOOP_DIRECTION_DECREMENT                          0x00
#define LIGHTING_COLOR_LOOP_DIRECTION_INCREMENT                          0x01

/*****************************************************************************/
/***          Ballast Configuration Cluster Attributes                     ***/
/*****************************************************************************/
  // Ballast Information attribute set
#define ATTRID_BALLAST_CONFIGURATION_PHYSICAL_MIN_LEVEL                0x0000
#define ATTRID_BALLAST_CONFIGURATION_PHYSICAL_MAX_LEVEL                0x0001
#define ATTRID_BALLAST_CONFIGURATION_BALLAST_STATUS                           0x0002
/*** Ballast Status Attribute values (by bit number) ***/
#define LIGHTING_BALLAST_STATUS_NON_OPERATIONAL                          1 // bit 0 is set
#define LIGHTING_BALLAST_STATUS_LAMP_IS_NOT_IN_SOCKET                    2 // bit 1 is set
  // Ballast Settings attributes set
#define ATTRID_BALLAST_CONFIGURATION_MIN_LEVEL                                0x0010
#define ATTRID_BALLAST_CONFIGURATION_MAX_LEVEL                                0x0011
#define ATTRID_BALLAST_CONFIGURATION_POWER_ON_LEVEL                           0x0012
#define ATTRID_BALLAST_CONFIGURATION_POWER_ON_FADE_TIME                       0x0013
#define ATTRID_BALLAST_CONFIGURATION_INTRINSIC_BALLAST_FACTOR                  0x0014
#define ATTRID_BALLAST_CONFIGURATION_BALLAST_FACTOR_ADJUSTMENT                0x0015
  // Lamp Information attributes set
#define ATTRID_BALLAST_CONFIGURATION_LAMP_QUANTITY                            0x0020
  // Lamp Settings attributes set
#define ATTRID_BALLAST_CONFIGURATION_LAMP_TYPE                                0x0030
#define ATTRID_BALLAST_CONFIGURATION_LAMP_MANUFACTURER                        0x0031
#define ATTRID_BALLAST_CONFIGURATION_LAMP_RATED_HOURS                         0x0032
#define ATTRID_BALLAST_CONFIGURATION_LAMP_BURN_HOURS                          0x0033
#define ATTRID_BALLAST_CONFIGURATION_LAMP_ALARM_MODE                          0x0034
#define ATTRID_BALLAST_CONFIGURATION_LAMP_BURN_HOURS_TRIP_POINT               0x0035
/*** Lamp Alarm Mode attribute values  ***/
#define LIGHTING_BALLAST_LAMP_ALARM_MODE_BIT_0_NO_ALARM                  0
#define LIGHTING_BALLAST_LAMP_ALARM_MODE_BIT_0_ALARM                     1

/*******************************************************************************
 * TYPEDEFS
 */

/*** ZCL Color Control Cluster: Move To Hue Cmd payload ***/
typedef struct
{
  uint8_t  hue;
  uint8_t  direction;
  uint16_t transitionTime;
} zclCmdLightingMoveToHuePayload_t;

/*** ZCL Color Control Cluster: Move Hue Cmd payload ***/
typedef struct
{
  uint8_t moveMode;
  uint8_t rate;
} zclCmdLightingMoveHuePayload_t;

/*** ZCL Color Control Cluster: Step Hue Cmd payload ***/
typedef struct
{
  uint8_t stepMode;
  uint8_t transitionTime;
} zclCmdLightingStepHuePayload_t;

/*** ZCL Color Control Cluster: Move to Saturation Cmd payload ***/
typedef struct
{
  uint8_t  saturation;
  uint16_t transitionTime;
} zclCmdLightingMoveToSaturationPayload_t;

/*** ZCL Color Control Cluster: Move Saturation Cmd payload ***/
typedef struct
{
  uint8_t moveMode;
  uint8_t rate;
} zclCmdLightingMoveSaturationPayload_t;

/*** ZCL Color Control Cluster: Step Saturation Cmd payload ***/
typedef struct
{
  uint8_t stepMode;
  uint8_t transitionTime;
} zclCmdLightingStepSaturationPayload_t;

/*** ZCL Color Control Cluster: Move To Hue and Saturation Cmd payload ***/
typedef struct
{
  uint8_t  hue;
  uint8_t  saturation;
  uint16_t transitionTime;
} zclCmdLightingMoveToHueAndSaturationPayload_t;

/*** Structures used for callback functions ***/

typedef struct
{
  uint8_t  hue;            // target hue value
  uint8_t  direction;      // direction of change
  uint16_t transitionTime; // tame taken to move to the target hue in 1/10 sec increments
} zclCCMoveToHue_t;

typedef struct
{
  uint8_t moveMode; // LIGHTING_MOVE_HUE_STOP, LIGHTING_MOVE_HUE_UP, LIGHTING_MOVE_HUE_DOWN
  uint8_t rate;     // the movement in steps per second, where step is a change in the device's hue of one unit
} zclCCMoveHue_t;

typedef struct
{
  uint8_t stepMode;       // LIGHTING_STEP_HUE_UP, LIGHTING_STEP_HUE_DOWN
  uint8_t stepSize;       // change to the current value of the device's hue
  uint8_t transitionTime; // the movement in steps per 1/10 second
} zclCCStepHue_t;

typedef struct
{
  uint8_t saturation;      // target saturation value
  uint16_t transitionTime; // time taken move to the target saturation, in 1/10 second units
} zclCCMoveToSaturation_t;

typedef struct
{
  uint8_t moveMode; // LIGHTING_MOVE_SATURATION_STOP, LIGHTING_MOVE_SATURATION_UP,
                  // LIGHTING_MOVE_SATURATION_DOWN
  uint8_t rate;     // rate of movement in step/sec; step is the device's saturation of one unit
} zclCCMoveSaturation_t;

typedef struct
{
  uint8_t stepMode;       // LIGHTING_STEP_SATURATION_UP, LIGHTING_STEP_SATURATION_DOWN
  uint8_t stepSize;       // change to the current value of the device's hue
  uint8_t transitionTime; // time to perform a single step in 1/10 of second
} zclCCStepSaturation_t;

typedef struct
{
  uint8_t hue;             // a target hue
  uint8_t saturation;      // a target saturation
  uint16_t transitionTime; // time to move, equal of the value of the field in 1/10 seconds
} zclCCMoveToHueAndSaturation_t;

typedef struct
{
  uint16_t colorX;         // a target color X
  uint16_t colorY;         // a target color Y
  uint16_t transitionTime; // time to move, equal of the value of the field in 1/10 seconds
} zclCCMoveToColor_t;

typedef struct
{
  int16_t rateX;          // rate of movement in steps per second. A step is a change
                        // in the device's CurrentX attribute of one unit.
  int16_t rateY;          // rate of movement in steps per second. A step is a change
                        // in the device's CurrentYattribute of one unit.
} zclCCMoveColor_t;

typedef struct
{
  int16_t stepX;           // change to be added to the device's CurrentX attribute
  int16_t stepY;           // change to be added to the device's CurrentY attribute
  uint16_t transitionTime; // time to move, equal of the value of the field in 1/10 seconds
} zclCCStepColor_t;

typedef struct
{
  uint16_t colorTemperature; // a target color temperature
  uint16_t transitionTime;   // time to move, equal of the value of the field in 1/10 seconds
} zclCCMoveToColorTemperature_t;

typedef struct
{
  uint16_t enhancedHue;    // target enhanced hue value
  uint8_t direction;       // direction of change
  uint16_t transitionTime; // tame taken to move to the target hue in 1/10 sec increments
} zclCCEnhancedMoveToHue_t;

typedef struct
{
  uint8_t moveMode; // LIGHTING_MOVE_HUE_STOP, LIGHTING_MOVE_HUE_UP, LIGHTING_MOVE_HUE_DOWN
  uint16_t rate;    // the movement in steps per second, where step is a change in the device's hue of one unit
} zclCCEnhancedMoveHue_t;

typedef struct
{
  uint8_t stepMode;        // LIGHTING_STEP_HUE_UP, LIGHTING_STEP_HUE_DOWN
  uint16_t stepSize;       // change to the current value of the device's hue
  uint16_t transitionTime; // the movement in steps per 1/10 second
} zclCCEnhancedStepHue_t;

typedef struct
{
  uint16_t enhancedHue;    // a target hue
  uint8_t saturation;      // a target saturation
  uint16_t transitionTime; // time to move, equal of the value of the field in 1/10 seconds
} zclCCEnhancedMoveToHueAndSaturation_t;

typedef struct
{
  unsigned int action:1;
  unsigned int direction:1;
  unsigned int time:1;
  unsigned int startHue:1;
  unsigned int reserved:4;
} zclCCColorLoopSet_updateFlagsBits_t;

typedef union
{
  zclCCColorLoopSet_updateFlagsBits_t bits;
  uint8_t byte;
} zclCCColorLoopSet_updateFlags_t;

typedef struct
{
  zclCCColorLoopSet_updateFlags_t updateFlags;
  uint8_t action;     // LIGHTING_COLOR_LOOP_ACTION_DEACTIVATE, LIGHTING_COLOR_LOOP_ACTION_ACTIVATE_FROM_START_HUE, LIGHTING_COLOR_LOOP_ACTION_ACTIVATE_FROM_ENH_CURR_HUE
  uint8_t direction;  // LIGHTING_COLOR_LOOP_DIRECTION_DECREMENT, LIGHTING_COLOR_LOOP_DIRECTION_INCREMENT
  uint16_t time;      // number of seconds on which to perform a full color loop
  uint16_t startHue;  // the starting hue to use
} zclCCColorLoopSet_t;

typedef struct
{
  uint8_t moveMode;
  uint16_t rate;
  uint16_t mininunMireds;
  uint16_t maximumMireds;

}zclCCMoveColorTemperature_t;

typedef struct
{
    uint8_t stepMode;
    uint16_t stepSize;
    uint16_t transmitionTime;
    uint16_t mininunMireds;
    uint16_t maximumMireds;

}zclCCStepColorTemperature_t;

// This callback is called to process a Move To Hue command
typedef ZStatus_t (*zclLighting_ColorControl_MoveToHue_t)( zclCCMoveToHue_t *pCmd );

// This callback is called to process a Move Hue command
typedef ZStatus_t (*zclLighting_ColorControl_MoveHue_t)( zclCCMoveHue_t *pCmd );

// This callback is called to process a Step Hue command
typedef ZStatus_t (*zclLighting_ColorControl_StepHue_t)( zclCCStepHue_t *pCmd );

// This callback is called to process a Move To Saturation command
typedef ZStatus_t (*zclLighting_ColorControl_MoveToSaturation_t)( zclCCMoveToSaturation_t *pCmd );

// This callback is called to process a Move Saturation command
typedef ZStatus_t (*zclLighting_ColorControl_MoveSaturation_t)( zclCCMoveSaturation_t *pCmd );

// This callback is called to process a Step Saturation command
typedef ZStatus_t (*zclLighting_ColorControl_StepSaturation_t)( zclCCStepSaturation_t *pCmd );

// This callback is called to process a Move to Hue and Saturation command
typedef ZStatus_t (*zclLighting_ColorControl_MoveToHueAndSaturation_t)( zclCCMoveToHueAndSaturation_t *pCmd );

// This callback is called to process a Move to Color command
typedef ZStatus_t (*zclLighting_ColorControl_MoveToColor_t)( zclCCMoveToColor_t *pCmd );

// This callback is called to process a Move Color command
typedef void (*zclLighting_ColorControl_MoveColor_t)( zclCCMoveColor_t *pCmd );

// This callback is called to process a Step Color command
typedef ZStatus_t (*zclLighting_ColorControl_StepColor_t)( zclCCStepColor_t *pCmd );

// This callback is called to process a Move to Color Temperature command
typedef ZStatus_t (*zclLighting_ColorControl_MoveToColorTemperature_t)( zclCCMoveToColorTemperature_t *pCmd );

// This callback is called to process an Enhanced Move To Hue command
typedef ZStatus_t (*zclLighting_ColorControl_EnhancedMoveToHue_t)( zclCCEnhancedMoveToHue_t *pCmd );

// This callback is called to process an Enhanced Move Hue command
typedef ZStatus_t (*zclLighting_ColorControl_EnhancedMoveHue_t)( zclCCEnhancedMoveHue_t *pCmd );

// This callback is called to process an Enhanced Step Hue command
typedef ZStatus_t (*zclLighting_ColorControl_EnhancedStepHue_t)( zclCCEnhancedStepHue_t *pCmd );

// This callback is called to process an Enhanced Move to Hue and Saturation command
typedef ZStatus_t (*zclLighting_ColorControl_EnhancedMoveToHueAndSaturation_t)( zclCCEnhancedMoveToHueAndSaturation_t *pCmd );

// This callback is called to process an Color Loop Set command
typedef ZStatus_t (*zclLighting_ColorControl_ColorLoopSet_t)( zclCCColorLoopSet_t *pCmd );

// This callback is called to process a Stop Step Move command
typedef ZStatus_t (*zclLighting_ColorControl_StopMoveStep_t)( void );

typedef ZStatus_t (*zclLighting_ColorControl_MoveColorTemperature_t)( zclCCMoveColorTemperature_t *pCmd );
typedef ZStatus_t (*zclLighting_ColorControl_StepColorTemperature_t)( zclCCStepColorTemperature_t *pCmd );
// Register Callbacks table entry - enter function pointers for callbacks that
// the application would like to receive
typedef struct
{
  zclLighting_ColorControl_MoveToHue_t                        pfnColorControl_MoveToHue;                        //!< (COMMAND_COLOR_CONTROL_MOVE_TO_HUE)
  zclLighting_ColorControl_MoveHue_t                          pfnColorControl_MoveHue;                          //!< (COMMAND_COLOR_CONTROL_MOVE_HUE)
  zclLighting_ColorControl_StepHue_t                          pfnColorControl_StepHue;                          //!< (COMMAND_COLOR_CONTROL_STEP_HUE)
  zclLighting_ColorControl_MoveToSaturation_t                 pfnColorControl_MoveToSaturation;                 //!< (COMMAND_COLOR_CONTROL_MOVE_TO_SATURATION)
  zclLighting_ColorControl_MoveSaturation_t                   pfnColorControl_MoveSaturation;                   //!< (COMMAND_COLOR_CONTROL_MOVE_SATURATION)
  zclLighting_ColorControl_StepSaturation_t                   pfnColorControl_StepSaturation;                   //!< (COMMAND_COLOR_CONTROL_STEP_SATURATION)
  zclLighting_ColorControl_MoveToHueAndSaturation_t           pfnColorControl_MoveToHueAndSaturation;           //!< (COMMAND_COLOR_CONTROL_MOVE_TO_HUE_AND_SATURATION)
  zclLighting_ColorControl_MoveToColor_t                      pfnColorControl_MoveToColor;                      //!< (COMMAND_COLOR_CONTROL_MOVE_TO_COLOR)
  zclLighting_ColorControl_MoveColor_t                        pfnColorControl_MoveColor;                        //!< (COMMAND_COLOR_CONTROL_MOVE_COLOR)
  zclLighting_ColorControl_StepColor_t                        pfnColorControl_StepColor;                        //!< (COMMAND_COLOR_CONTROL_STEP_COLOR)
  zclLighting_ColorControl_MoveToColorTemperature_t           pfnColorControl_MoveToColorTemperature;           //!< (COMMAND_COLOR_CONTROL_MOVE_TO_COLOR_TEMPERATURE)
  zclLighting_ColorControl_EnhancedMoveToHue_t                pfnColorControl_EnhancedMoveToHue;                //!< (COMMAND_COLOR_CONTROL_ENHANCED_MOVE_TO_HUE)
  zclLighting_ColorControl_EnhancedMoveHue_t                  pfnColorControl_EnhancedMoveHue;                  //!< (COMMAND_COLOR_CONTROL_ENHANCED_MOVE_HUE)
  zclLighting_ColorControl_EnhancedStepHue_t                  pfnColorControl_EnhancedStepHue;                  //!< (COMMAND_COLOR_CONTROL_ENHANCED_STEP_HUE)
  zclLighting_ColorControl_EnhancedMoveToHueAndSaturation_t   pfnColorControl_EnhancedMoveToHueAndSaturation;   //!< (COMMAND_COLOR_CONTROL_ENHANCED_MOVE_TO_HUE_AND_SATURATION)
  zclLighting_ColorControl_ColorLoopSet_t                     pfnColorControl_ColorLoopSet;                     //!< (COMMAND_COLOR_CONTROL_COLOR_LOOP_SET)
  zclLighting_ColorControl_StopMoveStep_t                     pfnColorControl_StopMoveStep;                     //!< (COMMAND_COLOR_CONTROL_STOP_MOVE_STEP)
  zclLighting_ColorControl_MoveColorTemperature_t             pfnColorControl_MoveColorTemperature;             //!< (COMMAND_COLOR_CONTROL_MOVE_COLOR_TEMPERATURE)
  zclLighting_ColorControl_StepColorTemperature_t             pfnColorControl_StepColorTemperature;             //!< (COMMAND_COLOR_CONTROL_STEP_COLOR_TEMPERATURE)
} zclLighting_AppCallbacks_t;


/******************************************************************************
 * FUNCTION MACROS
 */
#ifdef ZCL_LIGHT_LINK_ENHANCE
/*
 *  Send out anStop Move Step Command - COMMAND_COLOR_CONTROL_STOP_MOVE_STEP
 *  Use like:
 *      ZStatus_t zclLighting_ColorControl_Send_StopMoveStepCmd( uint16_t srcEP, afAddrType_t *dstAddr, uint8_t disableDefaultRsp, uint8_t seqNum );
 */
#define zclLighting_ColorControl_Send_StopMoveStepCmd(a,b,c,d) zcl_SendCommand( (a), (b), ZCL_CLUSTER_ID_LIGHTING_COLOR_CONTROL, COMMAND_COLOR_CONTROL_STOP_MOVE_STEP, TRUE, ZCL_FRAME_CLIENT_SERVER_DIR, (c), 0, (d), 0, NULL )
#endif //ZCL_LIGHT_LINK_ENHANCE

/******************************************************************************
 * VARIABLES
 */

/******************************************************************************
 * FUNCTIONS
 */

/*
 * Register for callbacks from this cluster library
 */
extern ZStatus_t zclLighting_RegisterCmdCallbacks( uint8_t endpoint, zclLighting_AppCallbacks_t *callbacks );


/*
 * Call to send out a Move To Hue Command
 *      hue - target hue value
 *      direction - direction of hue change
 *      transitionTime - tame taken to move to the target hue in 1/10 sec increments
 */
extern ZStatus_t zclLighting_ColorControl_Send_MoveToHueCmd( uint8_t srcEP, afAddrType_t *dstAddr,
                                              uint8_t hue, uint8_t direction, uint16_t transitionTime,
                                              uint8_t disableDefaultRsp, uint8_t seqNum );

/*
 * Call to send out a Move Hue Command
 *      moveMode - LIGHTING_MOVE_HUE_STOP, LIGHTING_MOVE_HUE_UP, LIGHTING_MOVE_HUE_DOWN
 *      rate - the movement in steps per second (step is a change in the device's hue
 *             of one unit)
 */
extern ZStatus_t zclLighting_ColorControl_Send_MoveHueCmd( uint8_t srcEP, afAddrType_t *dstAddr,
                                                           uint8_t moveMode, uint8_t rate,
                                                           uint8_t disableDefaultRsp, uint8_t seqNum );

/*
 * Call to send out a Step Hue Command
 *      stepMode - LIGHTING_STEP_HUE_UP, LIGHTING_STEP_HUE_DOWN
 *      amount - number of hue units to step
 *      transitionTime - the movement in steps per 1/10 second
 */
extern ZStatus_t zclLighting_ColorControl_Send_StepHueCmd( uint8_t srcEP, afAddrType_t *dstAddr,
                                          uint8_t stepMode, uint8_t stepSize, uint8_t transitionTime,
                                          uint8_t disableDefaultRsp, uint8_t seqNum );

/*
 * Call to send out a Move To Saturation Command
 *      saturation - target saturation value
 *      transitionTime - time taken move to the target saturation, in 1/10 second units
 */
extern ZStatus_t zclLighting_ColorControl_Send_MoveToSaturationCmd( uint8_t srcEP, afAddrType_t *dstAddr,
                                                uint8_t saturation, uint16_t transitionTime,
                                                uint8_t disableDefaultRsp, uint8_t seqNum );

/*
 * Call to send out a Move Saturation Command
 *      moveMode - LIGHTING_MOVE_SATURATION_STOP, LIGHTING_MOVE_SATURATION_UP,
 *                 LIGHTING_MOVE_SATURATION_DOWN
 *      rate -  rate of movement in step per second; step is the device's
 *              saturation of one unit
 */
extern ZStatus_t zclLighting_ColorControl_Send_MoveSaturationCmd( uint8_t srcEP, afAddrType_t *dstAddr,
                                                                  uint8_t moveMode, uint8_t rate,
                                                                  uint8_t disableDefaultRsp, uint8_t seqNum );

/*
 * Call to send out a Step Saturation Command
 *      stepMode -  LIGHTING_STEP_SATURATION_UP, LIGHTING_STEP_SATURATION_DOWN
 *      amount -  number of units to change the saturation level by
 *      transitionTime - time to perform a single step in 1/10 of second
 */
extern ZStatus_t zclLighting_ColorControl_Send_StepSaturationCmd( uint8_t srcEP, afAddrType_t *dstAddr,
                                                uint8_t stepMode, uint8_t stepSize, uint8_t transitionTime,
                                                uint8_t disableDefaultRsp, uint8_t seqNum );

/*
 * Call to send out a Move To Hue And Saturation  Command
 *      hue - 	target hue
 *      saturation -  target saturation
 *      transitionTime -  time to move, equal of the value of the field in 1/10 seconds
 */
extern ZStatus_t zclLighting_ColorControl_Send_MoveToHueAndSaturationCmd( uint8_t srcEP, afAddrType_t *dstAddr,
                                                          uint8_t hue, uint8_t saturation, uint16_t transitionTime,
                                                          uint8_t disableDefaultRsp, uint8_t seqNum );

/*
 * Call to send out a Move To Color Command
 *      colorX -  target color X
 *      colorY -  target color Y
 *      transitionTime -  time to move, equal of the value of the field in 1/10 seconds
 */
extern ZStatus_t zclLighting_ColorControl_Send_MoveToColorCmd( uint8_t srcEP, afAddrType_t *dstAddr,
                                              uint16_t colorX, uint16_t colorY, uint16_t transitionTime,
                                              uint8_t disableDefaultRsp, uint8_t seqNum );

/*
 * Call to send out a Move Color Command
 *      rateX -  rate of movement in steps per second. A step is a change
 *               in the device's CurrentX attribute of one unit.
 *      rateY -  rate of movement in steps per second. A step is a change
 *               in the device's CurrentY attribute of one unit.
 */
extern ZStatus_t zclLighting_ColorControl_Send_MoveColorCmd( uint8_t srcEP, afAddrType_t *dstAddr,
                                                             int16_t rateX, int16_t rateY,
                                                             uint8_t disableDefaultRsp, uint8_t seqNum );

/*
 * Call to send out a Step Color Command
 *      stepX -  change to be added to the device's CurrentX attribute
 *      stepY -  change to be added to the device's CurrentY attribute
 *      transitionTime -  time to perform the color change, equal of
 *                        the value of the field in 1/10 seconds
 */
extern ZStatus_t zclLighting_ColorControl_Send_StepColorCmd( uint8_t srcEP, afAddrType_t *dstAddr,
                                                int16_t stepX, int16_t stepY, uint16_t transitionTime,
                                                uint8_t disableDefaultRsp, uint8_t seqNum );

/*
 * Call to send out a Move To Color Temperature Command
 *      colorTemperature - a target color temperature
 *      transitionTime -  time to perform the color change, equal of
 *                        the value of the field in 1/10 seconds
 */
extern ZStatus_t zclLighting_ColorControl_Send_MoveToColorTemperatureCmd( uint8_t srcEP, afAddrType_t *dstAddr,
                                                              uint16_t colorTemperature, uint16_t transitionTime,
                                                              uint8_t disableDefaultRsp, uint8_t seqNum );
#ifdef ZCL_LIGHT_LINK_ENHANCE
/*
 * Call to send out an Enhanced Move To Hue Command
 *      enhancedHue - a target extended hue for lamp
 *      direction - direction of hue change
 *      transitionTime -  time to perform the color change, equal of
 *                        the value of the field in 1/10 seconds
 */
ZStatus_t zclLighting_ColorControl_Send_EnhancedMoveToHueCmd( uint8_t srcEP, afAddrType_t *dstAddr,
                                                              uint16_t enhancedHue, uint8_t direction,
                                                              uint16_t transitionTime, uint8_t disableDefaultRsp,
                                                              uint8_t seqNum );
/*
 * Call to send out an Enhanced Move Hue Command
 *      moveMode - LIGHTING_MOVE_HUE_STOP, LIGHTING_MOVE_HUE_UP,
 *                 LIGHTING_MOVE_HUE_DOWN
 *      rate - the movement in steps per second, where step is
 *             a change in the device's hue of one unit
 */
ZStatus_t zclLighting_ColorControl_Send_EnhancedMoveHueCmd( uint8_t srcEP, afAddrType_t *dstAddr,
                                                            uint8_t moveMode, uint16_t rate,
                                                            uint8_t disableDefaultRsp, uint8_t seqNum );
/*
 * Call to send out an Enhanced Step Hue Command
 *      stepMode -	LIGHTING_STEP_HUE_UP, LIGHTING_STEP_HUE_DOWN
 *      stepSize -  change to the current value of the device's hue
 *      transitionTime - the movement in steps per 1/10 second
 */
ZStatus_t zclLighting_ColorControl_Send_EnhancedStepHueCmd( uint8_t srcEP, afAddrType_t *dstAddr,
                                  uint8_t stepMode, uint16_t stepSize, uint16_t transitionTime,
                                  uint8_t disableDefaultRsp, uint8_t seqNum );
/*
 * Call to send out an Enhanced Move To Hue And Saturation Command
 *      Enhanced hue - a target Enhanced hue for lamp
 *      saturation - a target saturation
 *      transitionTime -  time to move, equal of the value of the field in 1/10 seconds
 */
ZStatus_t zclLighting_ColorControl_Send_EnhancedMoveToHueAndSaturationCmd( uint8_t srcEP, afAddrType_t *dstAddr,
                                                  uint16_t enhancedHue, uint8_t saturation, uint16_t transitionTime,
                                                  uint8_t disableDefaultRsp, uint8_t seqNum );

/*
 * Call to send out a Color Loop Set Command
 *      updateFlag - which color loop attributes to update before the color loop is started
 *      action - action to take for the color loop
 *      direction - direction for the color loop (decrement or increment)
 *      time - number of seconds over which to perform a full color loop
 *      startHue - starting hue to use for the color loop
 */
ZStatus_t zclLighting_ColorControl_Send_ColorLoopSetCmd(uint8_t srcEP, afAddrType_t *dstAddr,
                                                  zclCCColorLoopSet_updateFlags_t updateFlags, uint8_t action,
                                                  uint8_t direction, uint16_t time, uint16_t startHue,
                                                  uint8_t disableDefaultRsp, uint8_t seqNum);
#endif //ZCL_LIGHT_LINK_ENHANCE


/*********************************************************************
*********************************************************************/
#endif // ZCL_LIGHTING

#ifdef __cplusplus
}
#endif

#endif /* ZCL_LIGHTING_H */
