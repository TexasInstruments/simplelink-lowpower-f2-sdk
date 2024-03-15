/**************************************************************************************************
  Filename:       zcl_ss.h
  Revised:        $Date: 2014-04-29 23:23:15 -0700 (Tue, 29 Apr 2014) $
  Revision:       $Revision: 38309 $

  Description:    This file contains the ZCL Security and Safety definitions.


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

/**
 *  @defgroup ZCL_SS ZCL Safety and Security Clusters
 *  @brief This module implements the Safety and Security ZCL
 *  @{
 *
 *  @file  zcl_ss.h
 *  @brief      HA SS header
 */

#ifndef ZCL_SS_H
#define ZCL_SS_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "zcl.h"
#include "ti_zstack_config.h"

/*********************************************************************
 * CONSTANTS
 */
extern const uint8_t zclSS_UknownIeeeAddress[];


/**
 * @defgroup SS_IAS_ZONE_ATTRID SS IAS Zone Attributes
 * @{
 * @brief This group defines the Zone Information attributes set
 * defined in the ZCL v7 specification
 */
///State of the zone
#define ATTRID_IAS_ZONE_ZONE_STATE                                         0x0000 // M, R, ENUM8
/// The Zone Type dictates the meaning of Alarm1 and 10828 Alarm2 bits of
/// the ZoneStatus attribute.
#define ATTRID_IAS_ZONE_ZONE_TYPE                                          0x0001 // M, R, ENUM16
/// The ZoneStatus attribute is a bit map with status of each alarm.
#define ATTRID_IAS_ZONE_ZONE_STATUS                                        0x0002 // M, R, BITMAP16
/** @} End SS_IAS_ZONE_ATTRID */

/**
 * @defgroup STATE_SS_IAS_ZONE SS IAS Zone State
 * @{
 * @brief This group defines the Zone State Attribute values
 * defined in the ZCL v7 specification
 */
/// Not enrolled
#define SS_IAS_ZONE_STATE_NOT_ENROLLED                                   0x00
/// Enrolled (the client will react to Zone State Change
/// Notification commands from the server)
#define SS_IAS_ZONE_STATE_ENROLLED                                       0x01
/** @} End STATE_SS_IAS_ZONE */

/**
 * @defgroup TYPE_SS_IAS_ZONE SS IAS Zone Type
 * @{
 * @brief This group defines the Zone Type Attribute values
 * defined in the ZCL v7 specification
 * NOTE: if more Zone Type Attribute values are added,
 */
/// Zone Type      |       Alarm1       |       Alarm2       |
/// Standard CIE   |    System Alarm    |                    |
#define SS_IAS_ZONE_TYPE_STANDARD_CIE                                    0x0000
/// Zone Type      |          Alarm1          |         Alarm2          |
/// Motion sensor  |   Intrusion indication   |   Presence indication   |
#define SS_IAS_ZONE_TYPE_MOTION_SENSOR                                   0x000D
/// Zone Type      |          Alarm1           |         Alarm2          |
/// Contact switch |   1st portal Open-Close   | 2nd portal Open-Close   |
#define SS_IAS_ZONE_TYPE_CONTACT_SWITCH                                  0x0015
/// Zone Type      | Alarm1  | Alarm2  | Description                        |
/// D/W handle     |   0b0   |   0b0   | (Door/window) closed               |
///                |   0b1   |   0b0   | (Door/window) tilted (partly open) |
///                |   0b1   |   0b1   | (Door/window) open                 |
#define SS_IAS_ZONE_TYPE_DOOR_WINDOW_HANDLE                              0x0016
/// Zone Type      |      Alarm1       |      Alarm2      |
/// Fire sensor    |  Fire indication  |                  |
#define SS_IAS_ZONE_TYPE_FIRE_SENSOR                                     0x0028
/// Zone Type     |           Alarm1            |      Alarm2      |
/// Water sensor  |  Water overflow indication  |                  |
#define SS_IAS_ZONE_TYPE_WATER_SENSOR                                    0x002A
/// Zone Type                   |    Alarm1     |       Alarm2       |
/// Carbon Monoxide (CO) sensor | CO indication | Cooking indication |
#define SS_IAS_ZONE_TYPE_GAS_SENSOR                                      0x002B
/// Zone Type                 |     Alarm1      |      Alarm2      |
/// Personal emergency device | Fall/Concussion | Emergency button |
#define SS_IAS_ZONE_TYPE_PERSONAL_EMERGENCY_DEVICE                       0x002C
/// Zone Type                 |       Alarm1        |    Alarm2     |
/// Vibration/Movement sensor | Movement indication |   Vibration   |
#define SS_IAS_ZONE_TYPE_VIBRATION_MOVEMENT_SENSOR                       0x002D
/// Zone Type             |    Alarm1     |    Alarm2     |
/// Remote Control        |    Panic      |   Emergency   |
#define SS_IAS_ZONE_TYPE_REMOTE_CONTROL                                  0x010F
/// Zone Type             |    Alarm1     |    Alarm2     |
/// Key fob               |    Panic      |   Emergency   |
#define SS_IAS_ZONE_TYPE_KEY_FOB                                         0x0115
/// Zone Type             |   Alarm1   |    Alarm2     |
/// Keypad                |   Panic    |   Emergency   |
#define SS_IAS_ZONE_TYPE_KEYPAD                                          0x021D
/// Zone Type               |   Alarm1   |   Alarm2    |
/// Standard Warning Device |            |             |
#define SS_IAS_ZONE_TYPE_STANDARD_WARNING_DEVICE                         0x0225
/// Zone Type           |          Alarm1           |   Alarm2    |
/// Glass break sensor  |  Glass breakage detected  |             |
#define SS_IAS_ZONE_TYPE_GLASS_BREAK_SENSOR                              0x0226
/// Zone Type         |   Alarm1   |   Alarm2    |
/// Security repeater |            |             |
#define SS_IAS_ZONE_TYPE_SECURITY_REPEATER                               0x0229
/// Zone Type         |   Alarm1   |   Alarm2    |
/// Invalid Zone Type |            |             |
#define SS_IAS_ZONE_TYPE_INVALID_ZONE_TYPE                               0xFFFF
/** @} End TYPE_SS_IAS_ZONE */

/**
 * @defgroup STATUS_SS_IAS_ZONE SS IAS Zone Status
 * @{
 * @brief This group defines the Zone Status Attribute values
 * defined in the ZCL v7 specification
 */
 /// opened or alarmed
#define SS_IAS_ZONE_STATUS_ALARM1_ALARMED                                0x0001
/// opened or alarmed
#define SS_IAS_ZONE_STATUS_ALARM2_ALARMED                                0x0002
/// Tampered
#define SS_IAS_ZONE_STATUS_TAMPERED_YES                                  0x0004
/// Low battery
#define SS_IAS_ZONE_STATUS_BATTERY_LOW                                   0x0008
/// Notify
#define SS_IAS_ZONE_STATUS_SUPERVISION_REPORTS_YES                       0x0010
/// Notify restore
#define SS_IAS_ZONE_STATUS_RESTORE_REPORTS_YES                           0x0020
/// Trouble/Failure
#define SS_IAS_ZONE_STATUS_TROUBLE_YES                                   0x0040
/// AC/Mains fault
#define SS_IAS_ZONE_STATUS_AC_MAINS_FAULT                                0x0080
/// Sensor is in test mode
#define SS_IAS_ZONE_STATUS_TEST                                          0x0100
/// Sensor detects a defective battery
#define SS_IAS_ZONE_STATUS_BATTERY_DEFECT                                0x0200


/** @} End STATUS_SS_IAS_ZONE */

/**
 * @defgroup SS_IAS_ATTRID SS IAS Attribute
 * @{
 * @brief This group defines the Zone Settings attributes set
 * defined in the ZCL v7 specification
 */
/// The IAS_CIE_Address attribute specifies the address that commands
/// generated by the server SHALL be sent to.
#define ATTRID_SS_IAS_CIE_ADDRESS                                        0x0010 // M, R/W, IEEE ADDRESS
#define ATTRID_IAS_ZONE_IASCIE_ADDRESS                                         0x0010 // M, R/W, IEEE ADDRESS
/// A unique reference number allocated by the CIE at zone enrollment time.
#define ATTRID_IAS_ZONE_ZONE_ID                                                0x0011 // M, R, uint8_t
/// Provides the total number of sensitivity levels supported by the
/// IAS Zone server.
#define ATTRID_IAS_ZONE_NUMBER_OF_ZONE_SENSITIVITY_LEVELS_SUPPORTED                  0x0012 // O, R, uint8_t
/// Allows an IAS Zone client to query and configure the IAS Zone server's
/// sensitivity level.
#define ATTRID_IAS_ZONE_CURRENT_ZONE_SENSITIVITY_LEVEL                         0x0013 // O, R, uint8_t
/** @} End SS_IAS_ATTRID */

/**
 * @defgroup SS_IAS_ZONE_STATUS_COMMAND_GENERATED SS IAS Zone Status Command Generated
 * @{
 * @brief This group defines the server commands generated (Server-to-Client in ZCL Header)
 * defined in the ZCL v7 specification
 */
 /// The Zone Status Change Notification command is generated when a change
 /// takes place in one or more bits of the ZoneStatus attribute.
#define COMMAND_IAS_ZONE_ZONE_STATUS_CHANGE_NOTIFICATION                   0x00
/// The Zone Enroll Request command is generated when a device embodying
/// the Zone server cluster wishes to be enrolled as an active alarm device.
#define COMMAND_IAS_ZONE_ZONE_ENROLL_REQUEST                        0x01
/** @} End SS_IAS_ZONE_STATUS_COMMAND_GENERATED */

/**
 * @defgroup SS_IAS_ZONE_STATUS_COMMAND_RECEIVED SS IAS Zone Status Command Received
 * @{
 * @brief This group defines the server commands received (Client-to-Server in ZCL Header)
 * defined in the ZCL v7 specification
 */
 /// On receipt, the device embodying the Zone server is notified that it is
 /// now enrolled as an active alarm device
#define COMMAND_IAS_ZONE_ZONE_ENROLL_RESPONSE                       0x00
/// Used to tell the IAS Zone server to commence normal operation mode.
#define COMMAND_IAS_ZONE_INITIATE_NORMAL_OPERATION_MODE                   0x01
/// This command enables IAS Zone servers to be remotely placed into a
/// test mode so that the user or installer MAY configure their field of view,
/// sensitivity, and other operational parameters.
#define COMMAND_IAS_ZONE_INITIATE_TEST_MODE                        0x02
/** @} End SS_IAS_ZONE_STATUS_COMMAND_RECEIVED */

/**
 * @defgroup ENROLL_RESPONSE_SS_IAS_ZONE SS IAS Zone Enroll Response
 * @{
 * @brief This group defines the permitted values for Enroll Response Code field
 * defined in the ZCL v7 specification
 */
#define SS_IAS_ZONE_STATUS_ENROLL_RESPONSE_CODE_SUCCESS                  0x00
#define SS_IAS_ZONE_STATUS_ENROLL_RESPONSE_CODE_NOT_SUPPORTED            0x01
#define SS_IAS_ZONE_STATUS_ENROLL_RESPONSE_CODE_NO_ENROLL_PERMIT         0x02
#define SS_IAS_ZONE_STATUS_ENROLL_RESPONSE_CODE_TOO_MANY_ZONES           0x03
/** @} End ENROLL_RESPONSE_SS_IAS_ZONE */

/**
 * @defgroup SS_IAS_ZONE_PAYLOAD SS IAS Zone Payload
 * @{
 * @brief This group defines the payload lengths
 * defined in the ZCL v7 specification
 */
#define PAYLOAD_LEN_ZONE_STATUS_CHANGE_NOTIFICATION   6
#define PAYLOAD_LEN_ZONE_ENROLL_REQUEST               4
#define PAYLOAD_LEN_ZONE_STATUS_ENROLL_RSP            2
#define PAYLOAD_LEN_ZONE_STATUS_INIT_TEST_MODE        2
/** @} End SS_IAS_ZONE_PAYLOAD */
/** @} End SS_IAS_ZONE */

/**
 * @defgroup SS_IAS_ACE_COMMAND_RECEIVED SS IAS ACE Commands Received
 * @{
 * @brief This group defines the server commands received (Client-to-Server in ZCL Header)
 * defined in the ZCL v7 specification
 */
/// On receipt of this command, the receiving device sets its arm mode
/// according to the value of the Arm Mode field.
#define COMMAND_IASACE_ARM                                           0x00
/// Provides IAS ACE clients with a method to send zone bypass requests
/// to the IAS ACE server.
#define COMMAND_IASACE_BYPASS                                        0x01
/// Command to indicate emergency.
#define COMMAND_IASACE_EMERGENCY                                     0x02
/// Command to indicate fire emergency.
#define COMMAND_IASACE_FIRE                                          0x03
/// Command to indicate panic emergency.
#define COMMAND_IASACE_PANIC                                         0x04
/// On receipt of this command, the device SHALL generate a Get Zone ID
/// Map Response command
#define COMMAND_IASACE_GET_ZONE_ID_MAP                               0x05
/// On receipt of this command, the device SHALL generate a Get Zone
/// Information Response command
#define COMMAND_IASACE_GET_ZONE_INFORMATION                          0x06
/// This command is used by ACE clients to request an update to the status.
#define COMMAND_IASACE_GET_PANEL_STATUS                              0x07
/// Provides IAS ACE clients with a way to retrieve the list of
/// zones to be bypassed.
#define COMMAND_IASACE_GET_BYPASSED_ZONE_LIST                        0x08
/// This command is used by ACE clients to request an update of the
/// status of the IAS Zone devices managed by the ACE server.
#define COMMAND_IASACE_GET_ZONE_STATUS                               0x09
/** @} End SS_IAS_ACE_COMMAND_RECEIVED */

/**
 * @defgroup SS_IAS_ACE_COMMAND_GENERATED SS IAS ACE Commands Generated
 * @{
 * @brief This group defines the server commands generated (Server-to-Client in ZCL Header)
 * defined in the ZCL v7 specification
 */
#define COMMAND_IASACE_ARM_RESPONSE                                  0x00
#define COMMAND_IASACE_GET_ZONE_ID_MAP_RESPONSE                      0x01
#define COMMAND_IASACE_GET_ZONE_INFORMATION_RESPONSE                 0x02
#define COMMAND_IASACE_ZONE_STATUS_CHANGED                           0x03
#define COMMAND_IASACE_PANEL_STATUS_CHANGED                          0x04
#define COMMAND_IASACE_GET_PANEL_STATUS_RESPONSE                     0x05
#define COMMAND_IASACE_SET_BYPASSED_ZONE_LIST                        0x06
#define COMMAND_IASACE_BYPASS_RESPONSE                               0x07
#define COMMAND_IASACE_GET_ZONE_STATUS_RESPONSE                      0x08
/** @} End SS_IAS_ACE_COMMAND_GENERATED */

/**
 * @defgroup ARM_MODE_SS_IAS_ACE SS IAS ACE Arm Modes
 * @{
 * @brief This group defines the Arm Mode field permitted values
 * defined in the ZCL v7 specification
 */
#define SS_IAS_ACE_ARM_DISARM                                            0x00
#define SS_IAS_ACE_ARM_DAY_HOME_ZONES_ONLY                               0x01
#define SS_IAS_ACE_ARM_NIGHT_SLEEP_ZONES_ONLY                            0x02
#define SS_IAS_ACE_ARM_ALL_ZONES                                         0x03
/** @} End ARM_MODE_SS_IAS_ACE */

/**
 * @defgroup ARM_NOTIFICATION_SS_IAS_ACE SS IAS ACE Arm Notifications
 * @{
 * @brief This group defines the Arm Notification field permitted values
 * defined in the ZCL v7 specification
 */
#define SS_IAS_ACE_ARM_NOTIFICATION_ALL_ZONES_DISARMED                   0x00
#define SS_IAS_ACE_ARM_NOTIFICATION_DAY_HOME_ZONES_ONLY                  0x01
#define SS_IAS_ACE_ARM_NOTIFICATION_NIGHT_SLEEP_ZONES_ONLY               0x02
#define SS_IAS_ACE_ARM_NOTIFICATION_ALL_ZONES_ARMED                      0x03
#define SS_IAS_ACE_ARM_NOTIFICATION_INVALID_ARM_DISARM_CODE              0x04
#define SS_IAS_ACE_ARM_NOTIFICATION_NOT_READY_TO_ARM                     0x05
#define SS_IAS_ACE_ARM_NOTIFICATION_ALREADY_DISARMED                     0x06
/** @} End ARM_NOTIFICATION_SS_IAS_ACE */

/**
 * @defgroup PANEL_STATUS_SS_IAS_ACE SS IAS ACE Panel Status
 * @{
 * @brief This group defines the Panel Status field permitted values
 * defined in the ZCL v7 specification
 */
#define SS_IAS_ACE_PANEL_STATUS_ALL_ZONES_DISARMED                       0x00
#define SS_IAS_ACE_PANEL_STATUS_ARMED_STAY                               0x01
#define SS_IAS_ACE_PANEL_STATUS_ARMED_NIGHT                              0x02
#define SS_IAS_ACE_PANEL_STATUS_ARMED_AWAY                               0x03
#define SS_IAS_ACE_PANEL_STATUS_EXIT_DELAY                               0x04
#define SS_IAS_ACE_PANEL_STATUS_ENTRY_DELAY                              0x05
#define SS_IAS_ACE_PANEL_STATUS_NOT_READY_TO_ARM                         0x06
#define SS_IAS_ACE_PANEL_STATUS_IN_ALARM                                 0x07
#define SS_IAS_ACE_PANEL_STATUS_ARMING_STAY                              0x08
#define SS_IAS_ACE_PANEL_STATUS_ARMING_NIGHT                             0x09
#define SS_IAS_ACE_PANEL_STATUS_ARMING_AWAY                              0x0A
/** @} End PANEL_STATUS_SS_IAS_ACE */

/**
 * @defgroup AUDIBLE_NOTIFICATION_SS_IAS_ACE SS IAS ACE Audible Notification
 * @{
 * @brief This group defines the Audible Notification field permitted values
 * defined in the ZCL v7 specification
 */
#define SS_IAS_ACE_AUDIBLE_NOTIFICATION_MUTE                             0x00
#define SS_IAS_ACE_AUDIBLE_NOTIFICATION_DEFAULT_SOUND                    0x01
/** @} End AUDIBLE_NOTIFICATION_SS_IAS_ACE */

/**
 * @defgroup ALARM_STATUS_SS_IAS_ACE SS IAS ACE Alarm Status
 * @{
 * @brief This group defines the Alarm Status field permitted values
 * defined in the ZCL v7 specification
 */
#define SS_IAS_ACE_ALARM_STATUS_NO_ALARM                                 0x00
#define SS_IAS_ACE_ALARM_STATUS_BURGLAR                                  0x01
#define SS_IAS_ACE_ALARM_STATUS_FIRE                                     0x02
#define SS_IAS_ACE_ALARM_STATUS_EMERGENCY                                0x03
#define SS_IAS_ACE_ALARM_STATUS_POLICE_PANIC                             0x04
#define SS_IAS_ACE_ALARM_STATUS_FIRE_PANIC                               0x05
#define SS_IAS_ACE_ALARM_STATUS_EMERGENCY_PANIC                          0x06
/** @} End ALARM_STATUS_SS_IAS_ACE */

/**
 * @defgroup BYPASS_RESULT_SS_IAS_ACE SS IAS ACE Bypass Result
 * @{
 * @brief This group defines the Bypass Result field permitted values
 * defined in the ZCL v7 specification
 */
#define SS_IAS_ACE_BYPASS_RESULT_ZONE_BYPASSED                           0x00
#define SS_IAS_ACE_BYPASS_RESULT_ZONE_NOT_BYPASSED                       0x01
#define SS_IAS_ACE_BYPASS_RESULT_NOT_ALLOWED                             0x02
#define SS_IAS_ACE_BYPASS_RESULT_INVALID_ZONE_ID                         0x03
#define SS_IAS_ACE_BYPASS_RESULT_UNKNOWN_ZONE_ID                         0x04
#define SS_IAS_ACE_BYPASS_RESULT_INVALID_ARM_DISARM_CODE                 0x05
/** @} End BYPASS_RESULT_SS_IAS_ACE */

/**
 * @defgroup FIELD_SS_IAS_ACE SS IAS ACE Field Lengths
 * @{
 * @brief This group defines the field lengths
 */
#define ZONE_ID_MAP_ARRAY_SIZE  16
#define ARM_DISARM_CODE_LEN     8
#define ZONE_LABEL_LEN          24
/** @} End FIELD_SS_IAS_ACE */

/**
 * @defgroup SS_IAS_ACE_PAYLOAD_LEN SS IAS ACE Payload Length
 * @{
 * @brief This group defines the payload lengths
 */
#define PAYLOAD_LEN_GET_ZONE_STATUS                 5
#define PAYLOAD_LEN_PANEL_STATUS_CHANGED            4
#define PAYLOAD_LEN_GET_PANEL_STATUS_RESPONSE       4
/** @} End SS_IAS_ACE_PAYLOAD_LEN */
/** @} End SS_IAS_ACE */

/**
 * @defgroup SS_IAS_WD_ATTRID SS IAS WD Attribute ID
 * @{
 * @brief This group defines the Maximum Duration attribute
 * defined in the ZCL v7 specification
 */
/// The MaxDuration attribute specifies the maximum time in seconds that the
/// siren will sound continuously, regardless of start/stop commands.
#define ATTRID_IASWD_MAX_DURATION                                0x0000
/** @} End SS_IAS_WD_ATTRID */

/**
 * @defgroup SS_IAS_WD_COMMAND_RECIEVED SS IAS WD Commands Received
 * @{
 * @brief This group defines the server commands received (Client-to-Server in ZCL Header)
 * defined in the ZCL v7 specification
 */
/// This command starts the WD operation. The WD alerts the surrounding area
/// by audible (siren) and visual (strobe) signals.
#define COMMAND_IASWD_START_WARNING                                  0x00
/// This command uses the WD capabilities to emit a quick audible/visible
/// pulse called a "squawk". The squawk command has no effect if the WD
/// is currently active (warning in progress).
#define COMMAND_IASWD_SQUAWK                                         0x01
/** @} End SS_IAS_WD_COMMAND_RECIEVED */

/**
 * @defgroup START_WARNING_MODE_SS_IAS_WD SS IAS WD Warning Mode
 * @{
 * @brief This group defines the warning mode field values
 * defined in the ZCL v7 specification
 */
#define SS_IAS_START_WARNING_WARNING_MODE_STOP                           0
#define SS_IAS_START_WARNING_WARNING_MODE_BURGLAR                        1
#define SS_IAS_START_WARNING_WARNING_MODE_FIRE                           2
#define SS_IAS_START_WARNING_WARNING_MODE_EMERGENCY                      3
#define SS_IAS_START_WARNING_WARNING_MODE_POLICE_PANIC                   4
#define SS_IAS_START_WARNING_WARNING_MODE_FIRE_PANIC                     5
#define SS_IAS_START_WARNING_WARNING_MODE_EMERGENCY_PANIC                6
/** @} End START_WARNING_MODE_SS_IAS_WD */

/**
 * @defgroup START_WARNING_STROBE_SS_IAS_WD SS IAS WD Warning Strobe
 * @{
 * @brief This group defines the start warning: strobe field values
 * defined in the ZCL v7 specification
 */
#define SS_IAS_START_WARNING_STROBE_NO_STROBE_WARNING                    0
#define SS_IAS_START_WARNING_STROBE_USE_STPOBE_IN_PARALLEL_TO_WARNING    1
/** @} End START_WARNING_STROBE_SS_IAS_WD */

/**
 * @defgroup SIREN_LEVEL_SS_IAS_WD SS IAS WD Siren Level
 * @{
 * @brief This group defines the siren level field values
 * defined in the ZCL v7 specification
 */
#define SS_IAS_SIREN_LEVEL_LOW_LEVEL_SOUND                               0
#define SS_IAS_SIREN_LEVEL_MEDIUM_LEVEL_SOUND                            1
#define SS_IAS_SIREN_LEVEL_HIGH_LEVEL_SOUND                              2
#define SS_IAS_SIREN_LEVEL_VERY_HIGH_LEVEL_SOUND                         3
/** @} End SIREN_LEVEL_SS_IAS_WD */

/**
 * @defgroup STROBE_LEVEL_SS_IAS_WD SS IAS WD Strobe Level
 * @{
 * @brief This group defines the strobe level field values
 * defined in the ZCL v7 specification
 */
#define SS_IAS_STROBE_LEVEL_LOW_LEVEL_STROBE                             0
#define SS_IAS_STROBE_LEVEL_MEDIUM_LEVEL_STROBE                          1
#define SS_IAS_STROBE_LEVEL_HIGH_LEVEL_STROBE                            2
#define SS_IAS_STROBE_LEVEL_VERY_HIGH_LEVEL_STROBE                       3
/** @} End STROBE_LEVEL_SS_IAS_WD */

/**
 * @defgroup SQUAWK_MODE_SS_IAS_WD SS IAS WD Squawk Mode
 * @{
 * @brief This group defines the squawk mode field values
 * defined in the ZCL v7 specification
 */
#define SS_IAS_SQUAWK_SQUAWK_MODE_SYSTEM_ALARMED_NOTIFICATION_SOUND      0
#define SS_IAS_SQUAWK_SQUAWK_MODE_SYSTEM_DISARMED_NOTIFICATION_SOUND     1
/** @} End SQUAWK_MODE_SS_IAS_WD */

/**
 * @defgroup SQUAWK_STROBE_SS_IAS_WD SS IAS WD Squawk Strobe
 * @{
 * @brief This group defines the squawk strobe field values
 * defined in the ZCL v7 specification
 */
#define SS_IAS_SQUAWK_STROBE_NO_STROBE_SQUAWK                            0
#define SS_IAS_SQUAWK_STROBE_USE_STROBE_BLINK_IN_PARALLEL_TO_SQUAWK      1
/** @} End SQUAWK_STROBE_SS_IAS_WD */

/**
 * @defgroup SQUAWK_LEVEL_SS_IAS_WD SS IAS WD Squawk Level
 * @{
 * @brief This group defines the squawk level field values
 * defined in the ZCL v7 specification
 */
#define SS_IAS_SQUAWK_SQUAWK_LEVEL_LOW_LEVEL_SOUND                       0
#define SS_IAS_SQUAWK_SQUAWK_LEVEL_MEDIUM_LEVEL_SOUND                    1
#define SS_IAS_SQUAWK_SQUAWK_LEVEL_HIGH_LEVEL_SOUND                      2
#define SS_IAS_SQUAWK_SQUAWK_LEVEL_VERY_HIGH_LEVEL_SOUND                 3
/** @} End SQUAWK_LEVEL_SS_IAS_WD */

/**
 * @defgroup MAX_ENTRIES_SS_IAS_WD SS IAS WD Maximum Entries
 * @{
 * @brief This group defines the maximum number of entries in the Zone table
 */
#define ZCL_SS_MAX_ZONES                                                 256
#define ZCL_SS_MAX_ZONE_ID                                               254
/** @} End MAX_ENTRIES_SS_IAS_WD */
/** @} End SS_IAS_WD */

/*********************************************************************
 * TYPEDEFS
 */

/**
 * @defgroup ZCL_IAS_ZONE_CALLBACKS ZCL IAS Zone Structs and Callbacks
 * @{
 * @brief This group defines the structs and callbacks used for IAS Zone devices
 */
/*** Structures used for callback functions  ***/
typedef struct
{
  uint16_t zoneStatus;     //!< current zone status - bit map
  uint8_t  extendedStatus; //!< bit map, currently set to All zeroes ( reserved )
  uint8_t  zoneID;         //!< allocated zone ID
  uint16_t delay;          //!< delay from change in ZoneStatus attr to transmission of change notification cmd
} zclZoneChangeNotif_t;

typedef struct
{
  afAddrType_t *srcAddr;         //!< initiator's address
  uint8_t        zoneID;           //!< allocated zone ID
  uint16_t       zoneType;         //!< current value of Zone Type attribute
  uint16_t       manufacturerCode; //!< manufacturer Code from node descriptor for the device
} zclZoneEnrollReq_t;

typedef struct
{
  uint8_t  responseCode; //!< value of  response Code
  uint8_t  zoneID;       //!< index to the zone table of the CIE
  uint16_t srcAddr;      //!< device sending the cmd
} zclZoneEnrollRsp_t;

/// This callback is called to process a Change Notification command
typedef ZStatus_t (*zclSS_ChangeNotification_t)( zclZoneChangeNotif_t *pCmd, afAddrType_t *srcAddr );

/// This callback is called to process a Enroll Request command
typedef ZStatus_t (*zclSS_EnrollRequest_t)( zclZoneEnrollReq_t *pReq, uint8_t endpoint );

/// This callback is called to process a Enroll Response command
typedef ZStatus_t (*zclSS_EnrollResponse_t)( zclZoneEnrollRsp_t *pRsp );
/** @} End ZCL_IAS_ZONE_CALLBACKS */

/**
 * @defgroup ZCL_IAS_TEST_CALLBACKS ZCL IAS Test Mode Structs and Callbacks
 * @{
 * @brief This group defines the structs and callbacks used for IAS Test Mode
 */
typedef struct
{
  uint8_t testModeDuration;         //!< Specifies the duration, in seconds,
                                  //!<for which the IAS Zone server SHALL
                                  //!<operate in its test mode.
  uint8_t currZoneSensitivityLevel; //!< Specifies the sensitivity level the IAS
                                  //!<Zone server SHALL use for the duration of
                                  //!<the Test Mode and with which it must update
                                  //!<its CurrentZoneSensitivityLevel attribute.
} zclZoneInitTestMode_t;

/// This callback is called to process a Initiate Normal Operation Mode Response command
typedef ZStatus_t (*zclSS_InitNormalOpModeResponse_t)( zclIncoming_t *pInMsg );

/// This callback is called to process a Initiate Test Mode Response command
typedef ZStatus_t (*zclSS_InitTestModeResponse_t)( zclIncoming_t *pInMsg );

/// This callback is called to process a Initiate Normal Operation Mode command
typedef ZStatus_t (*zclSS_InitNormalOpMode_t)( zclIncoming_t *pInMsg );

/// This callback is called to process a Initiate Test Mode Response command
typedef ZStatus_t (*zclSS_InitTestMode_t)( zclZoneInitTestMode_t *pCmd, zclIncoming_t *pInMsg );
/** @} End ZCL_IAS_TEST_CALLBACKS */

/**
 * @defgroup ZCL_IAS_ACE_CALLBACKS ZCL IAS ACE Structs and Callbacks
 * @{
 * @brief This group defines the structs and callbacks used for IAS ACE devices
 */
typedef struct
{
  uint8_t         armMode;          //!< The Arm Mode field
  UTF8String_t  armDisarmCode;    //!< The Arm/Disarm Code SHALL be a code entered into the ACE client
  uint8_t         zoneID;           //!< Zone ID is the index of the Zone in the CIE's zone table
} zclACEArm_t;

typedef struct
{
  uint8_t numberOfZones;        //!< number of zones ( one byte)
  uint8_t *bypassBuf;           //!< zone IDs array of 256 entries one byte each
  UTF8String_t armDisarmCode; //!< The Arm/Disarm Code SHALL be a code entered into the ACE client
} zclACEBypass_t;

typedef struct
{
  uint8_t  startingZoneID;     //!< at which the client like to obtain information
  uint8_t  maxNumZoneIDs;      //!< number of Zone statuses returned by Server
  uint8_t  zoneStatusMaskFlag; //!< boolean field
  uint16_t zoneStatusMask;     //!< Coupled with the Zone Status Mask Flag field,
                             //!< functions as a mask to enable IAS ACE clients to
                             //!< get information about 11204 the Zone IDs whose
                             //!< ZoneStatus attribute
} zclACEGetZoneStatus_t;

typedef struct
{
  uint8_t zoneID;             //!< index to the zone table of the CIE
  uint16_t zoneType;          //!< value of Zone Type atribute
  uint8_t *ieeeAddr;          //!< pointer to 64 bit address
  UTF8String_t  zoneLabel;  //!< Provides the Zone Label stored in the IAS CIE.
} zclACEGetZoneInfoRsp_t;

typedef struct
{
  uint8_t   zoneID;               //!< index to the zone table of the CIE
  uint16_t  zoneStatus;           //!< The ZoneStatus attribute is a bit map.
  uint8_t   audibleNotification;  //!< Provide the ACE client with information on
                                //!< which type of audible notification it SHOULD
                                //!< make for the zone status change.
  UTF8String_t  zoneLabel;      //!< Provides the Zone Label stored in the IAS CIE.
} zclACEZoneStatusChanged_t;

typedef struct
{
  uint8_t panelStatus;            //!< Panel Status enumeration
  uint8_t secondsRemaining;       //!< Remaining time in seconds
  uint8_t audibleNotification;    //!< Provide the ACE client with information on
                                //!< which type of audible notification it SHOULD
                                //!< make for the zone status change.
  uint8_t alarmStatus;            //!< Provides the ACE client with information on the
                                //!< type of alarm the panel is in if its Panel Status
                                //!< field indicates it is "in alarm."
} zclACEPanelStatusChanged_t;

typedef struct
{
  uint8_t panelStatus;            //!< Panel Status enumeration
  uint8_t secondsRemaining;       //!< Remaining time in seconds
  uint8_t audibleNotification;    //!< Provide the ACE client with information on
                                //!< which type of audible notification it SHOULD
                                //!< make for the zone status change.
  uint8_t alarmStatus;            //!< Provides the ACE client with information on the
                                //!< type of alarm the panel is in if its Panel Status
                                //!< field indicates it is "in alarm."
} zclACEGetPanelStatusRsp_t;

typedef struct
{
  uint8_t numberOfZones;  //!< Number of zones in the payload
  uint8_t *zoneID;        //!< List of Zone IDs included in the payload
} zclACESetBypassedZoneList_t;

typedef struct
{
  uint8_t numberOfZones;  //!< Number of zones in the payload
  uint8_t *bypassResult;  //!< An array of Zone IDs for each zone requested to be bypassed via the Bypass
} zclACEBypassRsp_t;

typedef struct
{
  uint8_t zoneID;       //!< index to the zone table of the CIE
  uint16_t zoneStatus;  //!< Status of the zone
} zclACEZoneStatus_t;

typedef struct
{
  uint8_t zoneStatusComplete;     //!< Indicates whether there are additional Zone IDs
                                //!< managed by the IAS ACE Server with Zone Status
                                //!< information to be obtained.
  uint8_t numberOfZones;          //!< Number of zones in the payload
  zclACEZoneStatus_t *zoneInfo; //!< List of zones. The number of zones is defined by numberOfZones.
} zclACEGetZoneStatusRsp_t;

/***  typedef for IAS ACE Zone table  ***/
typedef struct
{
  uint8_t   zoneID;                     //!< index to the zone table of the CIE
  uint16_t  zoneType;                   //!< value of Zone Type atribute
  uint8_t   zoneAddress[Z_EXTADDR_LEN]; //!< Device Address
} IAS_ACE_ZoneTable_t;

/// This callback is called to process an Arm command
typedef uint8_t (*zclSS_ACE_Arm_t)( zclACEArm_t *pCmd );

/// This callback is called to process a Bypass command
typedef ZStatus_t (*zclSS_ACE_Bypass_t)( zclACEBypass_t *pCmd );

/// This callback is called to process an Emergency command
typedef ZStatus_t (*zclSS_ACE_Emergency_t)( void );

/// This callback is called to process a Fire command
typedef ZStatus_t (*zclSS_ACE_Fire_t)( void );

/// This callback is called to process a Panic command
typedef ZStatus_t (*zclSS_ACE_Panic_t)( void );

/// This callback is called to process a Get Zone ID Map command
typedef ZStatus_t (*zclSS_ACE_GetZoneIDMap_t)( void );

/// This callback is called to process a Get Zone Information command
typedef ZStatus_t (*zclSS_ACE_GetZoneInformation_t)( zclIncoming_t *pInMsg );

/// This callback is called to process a Get Panel Status command
typedef ZStatus_t (*zclSS_ACE_GetPanelStatus_t)( zclIncoming_t *pInMsg );

/// This callback is called to process a Get Bypassed Zone List command
typedef ZStatus_t (*zclSS_ACE_GetBypassedZoneList_t)( zclIncoming_t *pInMsg );

/// This callback is called to process a Get Zone Status command
typedef ZStatus_t (*zclSS_ACE_GetZoneStatus_t)( zclIncoming_t *pInMsg );

/// This callback is called to process an Arm Response command
typedef ZStatus_t (*zclSS_ACE_ArmResponse_t)( uint8_t armNotification );

/// This callback is called to process a Get Zone ID Map Response command
typedef ZStatus_t (*zclSS_ACE_GetZoneIDMapResponse_t)( uint16_t *zoneIDMap );

/// This callback is called to process a Get Zone Information Response command
typedef ZStatus_t (*zclSS_ACE_GetZoneInformationResponse_t)( zclACEGetZoneInfoRsp_t *pRsp );

/// This callback is called to process a Zone Status Changed command
typedef ZStatus_t (*zclSS_ACE_ZoneStatusChanged_t)( zclACEZoneStatusChanged_t *pCmd );

/// This callback is called to process a Panel Status Changed command
typedef ZStatus_t (*zclSS_ACE_PanelStatusChanged_t)( zclACEPanelStatusChanged_t *pCmd );

/// This callback is called to process a Get Panel Status Response command
typedef ZStatus_t (*zclSS_ACE_GetPanelStatusResponse_t)( zclACEGetPanelStatusRsp_t *pCmd );

/// This callback is called to process a Set Bypassed Zone List command
typedef ZStatus_t (*zclSS_ACE_SetBypassedZoneList_t)( zclACESetBypassedZoneList_t *pCmd );

/// This callback is called to process an Bypass Response command
typedef ZStatus_t (*zclSS_ACE_BypassResponse_t)( zclACEBypassRsp_t *pCmd );

/// This callback is called to process an Get Zone Status Response command
typedef ZStatus_t (*zclSS_ACE_GetZoneStatusResponse_t)( zclACEGetZoneStatusRsp_t *pCmd );
/** @} End ZCL_IAS_ACE_CALLBACKS */

/**
 * @defgroup ZCL_IAS_WD_CALLBACKS ZCL IAS WD Structs and Callbacks
 * @{
 * @brief This group defines the structs and callbacks used for IAS WD devices
 */
typedef struct
{
  unsigned int warnMode:4;        //!< Warning Mode
  unsigned int warnStrobe:2;      //!< Strobe
  unsigned int warnSirenLevel:2;  //!< Siren Level
} warningbits_t;

typedef union
{
  warningbits_t  warningbits; //!< Warning struct
  uint8_t          warningbyte; //!< warning byte value
} warning_t;

typedef struct
{
  warning_t   warningmessage;   //!< Warning message struct
  uint16_t      warningDuration;  //!< Requested duration of warning, in seconds.
  uint8_t       strobeDutyCycle;  //!< Indicates the length of the flash cycle.
  uint8_t       strobeLevel;      //!< Indicates the intensity of the strobe as
                                //!< shown in the table below.
} zclWDStartWarning_t;


/***  ZCL WD Cluster: COMMAND_WD_SQUAWK Cmd payload  ***/
typedef struct
{
  unsigned int squawkMode:4;  //!< System is disarmed if set
  unsigned int strobe:1;      //!< Use strobe blink in parallel to squawk if set
  unsigned int reserved:1;    //!< Reserved
  unsigned int squawkLevel:2; //!< Sound Level
} squawkbits_t;

typedef union
{
  squawkbits_t  squawkbits;   //!< squawk bits struct
  uint8_t         squawkbyte;   //!< squawk byte
} zclWDSquawk_t;

/// This callback is called to process a Start Warning command
typedef ZStatus_t (*zclSS_WD_StartWarning_t)( zclWDStartWarning_t *pCmd );

/// This callback is called to process a Squawk command
typedef ZStatus_t (*zclSS_WD_Squawk_t)( zclWDSquawk_t *pCmd );
/** @} End ZCL_IAS_WD_CALLBACKS */

/// Register Callbacks table entry - enter function pointers for callbacks that
/// the application would like to receive
/// Side-Comments are necessary for app-builder parsing
typedef struct
{
  zclSS_ChangeNotification_t               pfnChangeNotification;             //!< Change Notification command (COMMAND_IAS_ZONE_ZONE_STATUS_CHANGE_NOTIFICATION)
  zclSS_EnrollRequest_t                    pfnEnrollRequest;                  //!< Enroll Request command (COMMAND_IAS_ZONE_ZONE_ENROLL_REQUEST)
  zclSS_EnrollResponse_t                   pfnEnrollResponse;                 //!< Enroll Reponse command (COMMAND_IAS_ZONE_ZONE_ENROLL_RESPONSE)
  zclSS_InitNormalOpMode_t                 pfnInitNormalOpMode;               //!< Initiate Normal Operation Mode command (COMMAND_IAS_ZONE_INITIATE_NORMAL_OPERATION_MODE)
  zclSS_InitTestMode_t                     pfnInitTestMode;                   //!< Initiate Test Mode command (COMMAND_IAS_ZONE_INITIATE_TEST_MODE)
  zclSS_ACE_Arm_t                          pfnACE_Arm;                        //!< Arm command (COMMAND_IASACE_ARM)
  zclSS_ACE_Bypass_t                       pfnACE_Bypass;                     //!< Bypass command (COMMAND_IASACE_BYPASS)
  zclSS_ACE_Emergency_t                    pfnACE_Emergency;                  //!< Emergency command (COMMAND_IASACE_EMERGENCY)
  zclSS_ACE_Fire_t                         pfnACE_Fire;                       //!< Fire command (COMMAND_IASACE_FIRE)
  zclSS_ACE_Panic_t                        pfnACE_Panic;                      //!< Panic command (COMMAND_IASACE_PANIC)
  zclSS_ACE_GetZoneIDMap_t                 pfnACE_GetZoneIDMap;               //!< Get Zone ID Map command (COMMAND_IASACE_GET_ZONE_ID_MAP)
  zclSS_ACE_GetZoneInformation_t           pfnACE_GetZoneInformation;         //!< Get Zone Information Command (COMMAND_IASACE_GET_ZONE_INFORMATION)
  zclSS_ACE_GetPanelStatus_t               pfnACE_GetPanelStatus;             //!< Get Panel Status Command (COMMAND_IASACE_GET_PANEL_STATUS)
  zclSS_ACE_GetBypassedZoneList_t          pfnACE_GetBypassedZoneList;        //!< Get Bypassed Zone List Command (COMMAND_IASACE_GET_BYPASSED_ZONE_LIST)
  zclSS_ACE_GetZoneStatus_t                pfnACE_GetZoneStatus;              //!< Get Zone Status Command (COMMAND_IASACE_GET_ZONE_STATUS)
  zclSS_ACE_ArmResponse_t                  pfnACE_ArmResponse;                //!< ArmResponse command (COMMAND_IASACE_ARM_RESPONSE)
  zclSS_ACE_GetZoneIDMapResponse_t         pfnACE_GetZoneIDMapResponse;       //!< Get Zone ID Map Response command (COMMAND_IASACE_GET_ZONE_ID_MAP_RESPONSE)
  zclSS_ACE_GetZoneInformationResponse_t   pfnACE_GetZoneInformationResponse; //!< Get Zone Information Response command (COMMAND_IASACE_GET_ZONE_INFORMATION_RESPONSE)
  zclSS_ACE_ZoneStatusChanged_t            pfnACE_ZoneStatusChanged;          //!< Zone Status Changed command (COMMAND_IASACE_ZONE_STATUS_CHANGED)
  zclSS_ACE_PanelStatusChanged_t           pfnACE_PanelStatusChanged;         //!< Panel Status Changed command (COMMAND_IASACE_PANEL_STATUS_CHANGED)
  zclSS_ACE_GetPanelStatusResponse_t       pfnACE_GetPanelStatusResponse;     //!< Get Panel Status Response command (COMMAND_IASACE_GET_PANEL_STATUS_RESPONSE)
  zclSS_ACE_SetBypassedZoneList_t          pfnACE_SetBypassedZoneList;        //!< Set Bypassed Zone List command (COMMAND_IASACE_SET_BYPASSED_ZONE_LIST)
  zclSS_ACE_BypassResponse_t               pfnACE_BypassResponse;             //!< Bypass Response command (COMMAND_IASACE_BYPASS_RESPONSE)
  zclSS_ACE_GetZoneStatusResponse_t        pfnACE_GetZoneStatusResponse;      //!< Get Zone Status Response command (COMMAND_IASACE_GET_ZONE_STATUS_RESPONSE)
  zclSS_WD_StartWarning_t                  pfnWD_StartWarning;                //!< Start Warning command (COMMAND_IASWD_START_WARNING)
  zclSS_WD_Squawk_t                        pfnWD_Squawk;                      //!< Squawk command (COMMAND_IASWD_SQUAWK)

} zclSS_AppCallbacks_t;


/*********************************************************************
 * FUNCTION MACROS
 */

#ifdef ZCL_ZONE
/**
 * @addtogroup ZCL_ZONE_COMMANDS ZCL IAS Zone Commands
 * @{
 */
/**
 *  @brief Send a InitNormalOperationMode Cmd  ( IAS Zone Cluster )
 *  Use like:
 *      zclSS_IAS_Send_ZoneStatusInitNormalOperationModeCmd( uint16_t srcEP, afAddrType_t *dstAddr, uint8_t disableDefaultRsp, uint8_t seqNum );
 */
#define zclSS_IAS_Send_ZoneStatusInitNormalOperationModeCmd(a,b,c,d) zcl_SendCommand( (a), (b), ZCL_CLUSTER_ID_SS_IAS_ZONE,\
                                                                          COMMAND_IAS_ZONE_INITIATE_NORMAL_OPERATION_MODE, TRUE,\
                                                                          ZCL_FRAME_CLIENT_SERVER_DIR, (c), 0, (d),  0, NULL )
/** @} End ZCL_ZONE_COMMANDS */
#endif // ZCL_ZONE

/**
 * @defgroup ZCL_ACE_COMMAND_MACROS ZCL IAS ACE Command Macros
 * @{
 * @brief This group defines the function macros for IAS ACE commands
 */
#ifdef ZCL_ACE
/**
 *  @brief Send an Emergency Cmd  ( IAS ACE Cluster )
 *  Use like:
 *      ZStatus_t zclSS_Send_IAS_ACE_EmergencyCmd( uint16_t srcEP, afAddrType_t *dstAddr, uint8_t disableDefaultRsp, uint8_t seqNum );
 */
#define zclSS_Send_IAS_ACE_EmergencyCmd(a,b,c,d) zcl_SendCommand( (a), (b), ZCL_CLUSTER_ID_SS_IAS_ACE,\
                                                      COMMAND_IASACE_EMERGENCY, TRUE,\
                                                      ZCL_FRAME_CLIENT_SERVER_DIR, (c), 0, (d), 0, NULL )

/**
 *  @brief Send a Fire Cmd  ( IAS ACE Cluster )
 *  Use like:
 *      ZStatus_t zclSS_Send_IAS_ACE_FireCmd( uint16_t srcEP, afAddrType_t *dstAddr, uint8_t disableDefaultRsp, uint8_t seqNum );
 */
#define zclSS_Send_IAS_ACE_FireCmd(a,b,c,d) zcl_SendCommand( (a), (b), ZCL_CLUSTER_ID_SS_IAS_ACE,\
                                                      COMMAND_IASACE_FIRE, TRUE,\
                                                      ZCL_FRAME_CLIENT_SERVER_DIR, (c), 0, (d), 0, NULL )

/**
 *  @brief Send a Panic Cmd  ( IAS ACE Cluster )
 *  Use like:
 *      ZStatus_t zclSS_Send_IAS_ACE_PanicCmd( uint16_t srcEP, afAddrType_t *dstAddr, uint8_t disableDefaultRsp, uint8_t seqNum );
 */
#define zclSS_Send_IAS_ACE_PanicCmd(a,b,c,d) zcl_SendCommand( (a), (b), ZCL_CLUSTER_ID_SS_IAS_ACE,\
                                                      COMMAND_IASACE_PANIC, TRUE,\
                                                      ZCL_FRAME_CLIENT_SERVER_DIR, (c), 0, (d), 0, NULL )

/**
 *  @brief Send a GetZoneIDMap Cmd  ( IAS ACE Cluster )
 *  Use like:
 *      ZStatus_t zclSS_Send_IAS_ACE_GetZoneIDMapCmd( uint16_t srcEP, afAddrType_t *dstAddr, uint8_t disableDefaultRsp, uint8_t seqNum );
 */
#define zclSS_Send_IAS_ACE_GetZoneIDMapCmd(a,b,c,d) zcl_SendCommand( (a), (b), ZCL_CLUSTER_ID_SS_IAS_ACE,\
                                                      COMMAND_IASACE_GET_ZONE_ID_MAP, TRUE,\
                                                      ZCL_FRAME_CLIENT_SERVER_DIR, (c), 0, (d),  0, NULL )

/**
 *  @brief Send a GetPanelStatus Cmd  ( IAS ACE Cluster )
 *  Use like:
 *      ZStatus_t zclSS_Send_IAS_ACE_GetPanelStatusCmd( uint16_t srcEP, afAddrType_t *dstAddr, uint8_t disableDefaultRsp, uint8_t seqNum );
 */
#define zclSS_Send_IAS_ACE_GetPanelStatusCmd(a,b,c,d) zcl_SendCommand( (a), (b), ZCL_CLUSTER_ID_SS_IAS_ACE,\
                                                      COMMAND_IASACE_GET_PANEL_STATUS, TRUE,\
                                                      ZCL_FRAME_CLIENT_SERVER_DIR, (c), 0, (d),  0, NULL )

/**
 *  @brief Send a GetBypassedZoneList Cmd  ( IAS ACE Cluster )
 *  Use like:
 *      ZStatus_t zclSS_Send_IAS_ACE_GetBypassedZoneListCmd( uint16_t srcEP, afAddrType_t *dstAddr, uint8_t disableDefaultRsp, uint8_t seqNum );
 */
#define zclSS_Send_IAS_ACE_GetBypassedZoneListCmd(a,b,c,d) zcl_SendCommand( (a), (b), ZCL_CLUSTER_ID_SS_IAS_ACE,\
                                                      COMMAND_IASACE_GET_BYPASSED_ZONE_LIST, TRUE,\
                                                      ZCL_FRAME_CLIENT_SERVER_DIR, (c), 0, (d),  0, NULL )
#endif // ZCL_ACE
/** @} End ZCL_ACE_COMMAND_MACROS */

/*********************************************************************
 * VARIABLES
 */

/*********************************************************************
 * FUNCTIONS
 */


/*!
 * @param   endpoint - application's endpoint
 * @param   callbacks - pointer to the callback record.
 *
 * @return  ZMemError if not able to allocate
 */
extern ZStatus_t zclSS_RegisterCmdCallbacks( uint8_t endpoint, zclSS_AppCallbacks_t *callbacks );

/**
 * @defgroup ZCL_ZONE_COMMANDS ZCL IAS Zone Commands
 * @{
 * @brief This group defines the functions for IAS Zone commands
 */
#ifdef ZCL_ZONE
/*!
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   zoneStatus - current zone status - bit map
 * @param   extendedStatus - bit map, currently set to All zeros ( reserved)
 * @param   zoneID - allocated zone ID
 * @param   delay - delay from change in ZoneStatus attr to transmission of change notification cmd
 * @param   disableDefaultRsp - toggle for enabling/disabling default response
 * @param   seqNum - command sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclSS_IAS_Send_ZoneStatusChangeNotificationCmd( uint8_t srcEP, afAddrType_t *dstAddr,
                                                             uint16_t zoneStatus, uint8_t extendedStatus,
                                                             uint8_t zoneID, uint16_t delay,
                                                             uint8_t disableDefaultRsp, uint8_t seqNum );

/*!
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   zoneType -    current value of Zone Type attribute
 * @param   manufacturerCode - manuf. code from node descriptor for the device
 * @param   disableDefaultRsp - toggle for enabling/disabling default response
 * @param   seqNum - command sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclSS_IAS_Send_ZoneStatusEnrollRequestCmd( uint8_t srcEP, afAddrType_t *dstAddr,
                                                      uint16_t zoneType, uint16_t manufacturerCode,
                                                      uint8_t disableDefaultRsp, uint8_t seqNum );

/*!
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   responseCode -  value of  response Code
 * @param   zoneID  - index to the zone table of the CIE
 * @param   disableDefaultRsp - toggle for enabling/disabling default response
 * @param   seqNum - command sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclSS_IAS_Send_ZoneStatusEnrollResponseCmd( uint8_t srcEP, afAddrType_t *dstAddr,
                                                             uint8_t responseCode, uint8_t zoneID,
                                                             uint8_t disableDefaultRsp, uint8_t seqNum );

/*!
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd -  pointer to command structure
 * @param   disableDefaultRsp - toggle for enabling/disabling default response
 * @param   seqNum - command sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclSS_IAS_Send_ZoneStatusInitTestModeCmd( uint8_t srcEP, afAddrType_t *dstAddr,
                                                    zclZoneInitTestMode_t *pCmd,
                                                    uint8_t disableDefaultRsp, uint8_t seqNum );
#endif // ZCL_ZONE

#if defined(ZCL_ZONE) || defined(ZCL_ACE)
/*!
 * @param   endpoint - endpoint of zone
 * @param   zoneID - ID to look for zone
 * @param   ieeeAddr - Device IEEE Address
 *
 * @return  none
 */
extern void zclSS_UpdateZoneAddress( uint8_t endpoint, uint8_t zoneID, uint8_t *ieeeAddr );


/*!
 * @param   endpoint - endpoint of zone to be removed
 * @param   zoneID - ID to look for zone
 *
 * @return  TRUE if removed, FALSE if not found
 */
extern uint8_t zclSS_RemoveZone( uint8_t endpoint, uint8_t zoneID );


/*!
 * @param   endpoint - endpoint of zone to be found
 * @param   zoneID - ID to look for zone
 *
 * @return  a pointer to the zone information, NULL if not found
 */
extern IAS_ACE_ZoneTable_t *zclSS_FindZone( uint8_t endpoint, uint8_t zoneID );
#endif // ZCL_ZONE || ZCL_ACE
/** @} End ZCL_ZONE_COMMANDS */

/**
 * @defgroup ZCL_ACE_COMMANDS ZCL IAS ACE Parse and Send Commands
 * @{
 * @brief This group defines the functions for IAS ACE to parse and send commands
 */
#ifdef ZCL_ACE
/*!
 * @param   pCmd - pointer to the output data struct
 * @param   pInBuf - pointer to the input data buffer
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclSS_ParseInCmd_ACE_Arm( zclACEArm_t *pCmd, uint8_t *pInBuf );

/*!
 * @param   pCmd - pointer to the output data struct
 * @param   pInBuf - pointer to the input data buffer
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclSS_ParseInCmd_ACE_Bypass( zclACEBypass_t *pCmd, uint8_t *pInBuf );

/*!
 * @param   pCmd - pointer to the output data struct
 * @param   pInBuf - pointer to the input data buffer
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclSS_ParseInCmd_ACE_GetZoneInformationResponse( zclACEGetZoneInfoRsp_t *pCmd,
                                                                  uint8_t *pInBuf );

/*!
 * @param   pCmd - pointer to the output data struct
 * @param   pInBuf - pointer to the input data buffer
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclSS_ParseInCmd_ACE_ZoneStatusChanged( zclACEZoneStatusChanged_t *pCmd,
                                                         uint8_t *pInBuf );

/*!
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - pointer to command structure
 * @param   disableDefaultRsp - toggle for enabling/disabling default response
 * @param   seqNum - command sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclSS_Send_IAS_ACE_ArmCmd( uint8_t srcEP, afAddrType_t *dstAddr,
                                            zclACEArm_t *pCmd,
                                            uint8_t disableDefaultRsp, uint8_t seqNum );

/*!
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - pointer to command structure
 * @param   disableDefaultRsp - toggle for enabling/disabling default response
 * @param   seqNum - command sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclSS_Send_IAS_ACE_BypassCmd( uint8_t srcEP, afAddrType_t *dstAddr,
                                               zclACEBypass_t *pCmd,
                                               uint8_t disableDefaultRsp, uint8_t seqNum );

/*!
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   zoneID - 8 bit value from 0 to 255
 * @param   disableDefaultRsp - toggle for enabling/disabling default response
 * @param   seqNum - command sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclSS_Send_IAS_ACE_GetZoneInformationCmd( uint8_t srcEP, afAddrType_t *dstAddr,
                                          uint8_t zoneID, uint8_t disableDefaultRsp, uint8_t seqNum );

/*!
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - pointer to command structure
 * @param   disableDefaultRsp - toggle for enabling/disabling default response
 * @param   seqNum - command sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclSS_Send_IAS_ACE_GetZoneStatusCmd( uint8_t srcEP, afAddrType_t *dstAddr,
                                                      zclACEGetZoneStatus_t *pCmd,
                                                      uint8_t disableDefaultRsp, uint8_t seqNum );

/*!
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   armNotification - notification parameter
 * @param   disableDefaultRsp - toggle for enabling/disabling default response
 * @param   seqNum - command sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclSS_Send_IAS_ACE_ArmResponse( uint8_t srcEP, afAddrType_t *dstAddr,
                       uint8_t armNotification, uint8_t disableDefaultRsp, uint8_t seqNum );

/*!
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   zoneIDMap - pointer to an array of 16 uint16_t
 * @param   disableDefaultRsp - toggle for enabling/disabling default response
 * @param   seqNum - command sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclSS_Send_IAS_ACE_GetZoneIDMapResponseCmd( uint8_t srcEP, afAddrType_t *dstAddr,
                                       uint16_t *zoneIDMap, uint8_t disableDefaultRsp, uint8_t seqNum );

/*!
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - pointer to command structure
 * @param   disableDefaultRsp - toggle for enabling/disabling default response
 * @param   seqNum - command sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclSS_Send_IAS_ACE_GetZoneInformationResponseCmd( uint8_t srcEP, afAddrType_t *dstAddr,
                                                    zclACEGetZoneInfoRsp_t *pCmd,
                                                    uint8_t disableDefaultRsp, uint8_t seqNum );

/*!
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - pointer to command structure
 * @param   disableDefaultRsp - toggle for enabling/disabling default response
 * @param   seqNum - command sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclSS_Send_IAS_ACE_ZoneStatusChangedCmd( uint8_t srcEP, afAddrType_t *dstAddr,
                                            zclACEZoneStatusChanged_t *pCmd,
                                            uint8_t disableDefaultRsp, uint8_t seqNum );

/*!
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - pointer to command structure
 * @param   disableDefaultRsp - toggle for enabling/disabling default response
 * @param   seqNum - command sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclSS_Send_IAS_ACE_PanelStatusChangedCmd( uint8_t srcEP, afAddrType_t *dstAddr,
                                            zclACEPanelStatusChanged_t *pCmd,
                                            uint8_t disableDefaultRsp, uint8_t seqNum );

/*!
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - pointer to command structure
 * @param   disableDefaultRsp - toggle for enabling/disabling default response
 * @param   seqNum - command sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclSS_Send_IAS_ACE_GetPanelStatusResponseCmd( uint8_t srcEP, afAddrType_t *dstAddr,
                                            zclACEGetPanelStatusRsp_t *pCmd,
                                            uint8_t disableDefaultRsp, uint8_t seqNum );

/*!
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - pointer to command structure
 * @param   disableDefaultRsp - toggle for enabling/disabling default response
 * @param   seqNum - command sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclSS_Send_IAS_ACE_SetBypassedZoneListCmd( uint8_t srcEP, afAddrType_t *dstAddr,
                                            zclACESetBypassedZoneList_t *pCmd,
                                            uint8_t disableDefaultRsp, uint8_t seqNum );

/*!
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - pointer to command structure
 * @param   disableDefaultRsp - toggle for enabling/disabling default response
 * @param   seqNum - command sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclSS_Send_IAS_ACE_BypassResponseCmd( uint8_t srcEP, afAddrType_t *dstAddr,
                                            zclACEBypassRsp_t *pCmd,
                                            uint8_t disableDefaultRsp, uint8_t seqNum );

/*!
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - pointer to command structure
 * @param   disableDefaultRsp - toggle for enabling/disabling default response
 * @param   seqNum - command sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclSS_Send_IAS_ACE_GetZoneStatusResponseCmd( uint8_t srcEP, afAddrType_t *dstAddr,
                                            zclACEGetZoneStatusRsp_t *pCmd,
                                            uint8_t disableDefaultRsp, uint8_t seqNum );
#endif // ZCL_ACE
/** @} End ZCL_ACE_COMMANDS */

/**
 * @defgroup ZCL_WD_COMMANDS ZCL IAS WD Commands
 * @{
 * @brief This group defines the functions for IAS WD commands
 */
#ifdef ZCL_WD
/*!
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   warning - pointer to command structure
 * @param   disableDefaultRsp - toggle for enabling/disabling default response
 * @param   seqNum - command sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclSS_Send_IAS_WD_StartWarningCmd( uint8_t srcEP, afAddrType_t *dstAddr,
                                                    zclWDStartWarning_t *warning,
                                                    uint8_t disableDefaultRsp, uint8_t seqNum );

/*!
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   squawk - pointer to the command structure
 * @param   disableDefaultRsp - toggle for enabling/disabling default response
 * @param   seqNum - command sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclSS_Send_IAS_WD_SquawkCmd( uint8_t srcEP, afAddrType_t *dstAddr,
                                              zclWDSquawk_t *squawk,
                                              uint8_t disableDefaultRsp, uint8_t seqNum );
#endif // ZCL_WD
/** @} End ZCL_WD_COMMANDS */

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* ZCL_SS_H */

/** @} End ZCL_SS */
