/**************************************************************************************************
  Filename:       zcl_closures.h
  Revised:        $Date: 2014-02-04 16:43:21 -0800 (Tue, 04 Feb 2014) $
  Revision:       $Revision: 37119 $

  Description:    This file contains the ZCL Closures definitions.


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
 *  @defgroup ZCL_CLOSURES ZCL Closures
 *  @brief This module implements the ZCL Closures definitions
 *  @{
 *
 *  @file  zcl_closures.h
 *  @brief      ZCL Closures definitions
 */

#ifndef ZCL_CLOSURES_H
#define ZCL_CLOSURES_H

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

/**
* @defgroup CLOSURE_SHADE_MACROS ZCL Closure Shade Config Cluster Macros
* @brief This group provides ZCL Closure Shade Config Cluster macros
* defined in the ZCL v7 specification
* @{
*/

/**********************************************/
/*** Shade Configuration Cluster Attributes ***/
/**********************************************/
  // Shade information attributes set
/// The PhysicalClosedLimit attribute indicates the most closed (numerically lowest)
/// position that the shade can physically move to.
#define ATTRID_SHADE_CONFIGURATION_PHYSICAL_CLOSED_LIMIT                          0x0000 // O, R, uint16_t
/// The MotorStepSize attribute indicates the angle the shade motor moves for
/// one step, measured in 1/10ths of a degree.
#define ATTRID_SHADE_CONFIGURATION_MOTOR_STEP_SIZE                                0x0001 // O, R, uint8_t
/// The Status attribute indicates the status of a number of shade functions.
#define ATTRID_SHADE_CONFIGURATION_STATUS                                         0x0002 // M, R/W, BITMAP8

/*** Status attribute bit values ***/
/// Shade operational if set
#define CLOSURES_STATUS_SHADE_IS_OPERATIONAL                           0x01
/// Shade adjusting if set
#define CLOSURES_STATUS_SHADE_IS_ADJUSTING                             0x02
/// Shade direction opening if set
#define CLOSURES_STATUS_SHADE_DIRECTION                                0x04
/// Direction corresponding to forward direction of motor, opening if set
#define CLOSURES_STATUS_SHADE_MOTOR_FORWARD_DIRECTION                  0x08

// Shade settings attributes set
/// The ClosedLimit attribute indicates the most closed position that the
/// shade can move to.
#define ATTRID_SHADE_CONFIGURATION_CLOSED_LIMIT                                   0x0010
/// The Mode attribute indicates the current operating mode of the shade.
#define ATTRID_SHADE_CONFIGURATION_MODE                                           0x0011

/*** Mode attribute values ***/
#define CLOSURES_MODE_NORMAL_MODE                                      0x00
#define CLOSURES_MODE_CONFIGURE_MODE                                   0x01

// cluster has no specific commands

/**********************************************/
/*** Logical Cluster ID - for mapping only  ***/
/***  These are not to be used over-the-air ***/
/**********************************************/
/// The ClosedLimit attribute indicates the most closed position that the
/// shade can move to.
#define ZCL_CLOSURES_LOGICAL_CLUSTER_ID_SHADE_CONFIG                   0x0010
/** @} End CLOSURE_SHADE_MACROS */

/**
* @defgroup CLOSURE_DOORLOCK_MACROS ZCL Closure Doorlock Cluster Macros
* @brief This group provides ZCL Closure Doorlock Cluster macros
* defined in the ZCL v7 specification
* @{
*/

/************************************/
/*** Door Lock Cluster Attributes ***/
/************************************/
#ifdef ZCL_DOORLOCK

/// Enum for lock state
#define ATTRID_DOOR_LOCK_LOCK_STATE                                          0x0000
/// Enum for lock type
#define ATTRID_DOOR_LOCK_LOCK_TYPE                                           0x0001
/// Actuator enabled of set
#define ATTRID_DOOR_LOCK_ACTUATOR_ENABLED                                    0x0002
/// Enum for door state
#define ATTRID_DOOR_LOCK_DOOR_STATE                                          0x0003
/// This attribute holds the number of door open events that have occurred
/// since it was last zeroed.
#define ATTRID_DOOR_LOCK_DOOR_OPEN_EVENTS                             0x0004
/// This attribute holds the number of door closed events that have occurred
/// since it was last zeroed.
#define ATTRID_DOOR_LOCK_DOOR_CLOSED_EVENTS                           0x0005
/// This attribute holds the number of minutes the door has been open since
/// the last time it transitioned from closed to open.
#define ATTRID_DOOR_LOCK_OPEN_PERIOD                                         0x0006

// User, PIN, Schedule, & Log Information Attributes
/// The number of available log records.
#define ATTRID_DOOR_LOCK_NUMBER_OF_LOG_RECORDS_SUPPORTED                       0x0010  // O, R, uint16_t
/// Number of total users supported by the lock.
#define ATTRID_DOOR_LOCK_NUMBER_OF_TOTAL_USERS_SUPPORTED                        0x0011  // O, R, uint16_t
/// The number of PIN users supported.
#define ATTRID_DOOR_LOCK_NUMBER_OF_PIN_USERS_SUPPORTED                          0x0012  // O, R, uint16_t
/// The number of RFID users supported.
#define ATTRID_DOOR_LOCK_NUMBER_OF_RFID_USERS_SUPPORTED                         0x0013  // O, R, uint16_t
/// number of configurable week day schedule supported per user.
#define ATTRID_DOOR_LOCK_NUMBER_OF_WEEK_DAY_SCHEDULES_SUPPORTED_PER_USER        0x0014  // O, R, uint8_t
/// The number of configurable year day schedule supported per user.
#define ATTRID_DOOR_LOCK_NUMBER_OF_YEAR_DAY_SCHEDULES_SUPPORTED_PER_USER        0x0015  // O, R, uint8_t
/// The number of holiday schedules supported for the entire door lock device.
#define ATTRID_DOOR_LOCK_NUMBER_OF_HOLIDAY_SCHEDULES_SUPPORTED                 0x0016  // O, R, uint8_t
/// An 8 bit value indicates the maximum length in bytes of a PIN Code on this device.
#define ATTRID_DOOR_LOCK_MAX_PIN_CODE_LENGTH                                         0x0017  // O, R, uint8_t
/// An 8 bit value indicates the minimum length in bytes of a PIN Code on this device.
#define ATTRID_DOOR_LOCK_MIN_PIN_CODE_LENGTH                                         0x0018  // O, R, uint8_t
/// An 8 bit value indicates the maximum length in bytes of a RFID Code on this device.
#define ATTRID_DOOR_LOCK_MAX_RFID_CODE_LENGTH                                        0x0019  // O, R, uint8_t
/// An 8 bit value indicates the minimum length in bytes of a RFID Code on this device.
#define ATTRID_DOOR_LOCK_MIN_RFID_CODE_LENGTH                                        0x001A  // O, R, uint8_t

// Operational Settings Attributes
/// Enable/disable event logging.
#define ATTRID_DOOR_LOCK_ENABLE_LOGGING                                      0x0020  // O, R/W, BOOLEAN
/// Modifies the language for the on-screen or audible user interface using three bytes from ISO-639-1.
#define ATTRID_DOOR_LOCK_LANGUAGE                                            0x0021  // O, R/W, CHAR STRING
/// Configuration for LED events
#define ATTRID_DOOR_LOCK_LED_SETTINGS                                        0x0022  // O, R/W, uint8_t
/// The number of seconds to wait after unlocking a lock before it automatically
/// locks again.
#define ATTRID_DOOR_LOCK_AUTO_RELOCK_TIME                                    0x0023  // O, R/W, uint32_t
/// The sound volume on a door lock has three possible settings: silent, low
/// and high volumes.
#define ATTRID_DOOR_LOCK_SOUND_VOLUME                                        0x0024  // O, R/W, uint8_t
/// shows the current operating mode and which interfaces are enabled during
/// each of the operating mode.
#define ATTRID_DOOR_LOCK_OPERATING_MODE                                      0x0025  // O, R/W, ENUM8
/// This bitmap contains all operating bits of the Operating Mode Attribute
/// supported by the lock.
#define ATTRID_DOOR_LOCK_SUPPORTED_OPERATING_MODES                           0x0026  // O, R, BITMAP16
/// This attribute represents the default configurations as they are physically
/// set on the device.
#define ATTRID_DOOR_LOCK_DEFAULT_CONFIGURATION_REGISTER                      0x0027  // O, R, BITMAP16
/// Enable/disable local programming on the door lock.
#define ATTRID_DOOR_LOCK_ENABLE_LOCAL_PROGRAMMING                            0x0028  // O, R/W, BOOLEAN
/// Enable/disable the ability to lock the door lock with a single touch on the door lock.
#define ATTRID_DOOR_LOCK_ENABLE_ONE_TOUCH_LOCKING                            0x0029  // O, R/W, BOOLEAN
/// Enable/disable an inside LED that allows the user to see at a glance if the door is locked.
#define ATTRID_DOOR_LOCK_ENABLE_INSIDE_STATUS_LED                            0x002A  // O, R/W, BOOLEAN
/// Enable/disable a button inside the door that is used to put the lock into
/// privacy mode. When the lock is in privacy mode it cannot be manipulated from the outside.
#define ATTRID_DOOR_LOCK_ENABLE_PRIVACY_MODE_BUTTON                          0x002B  // O, R/W, BOOLEAN

// Security Settings Attributes
/// The number of incorrect codes or RFID presentment attempts a user is allowed
/// to enter before the door will enter a lockout state.
#define ATTRID_DOOR_LOCK_WRONG_CODE_ENTRY_LIMIT                              0x0030  // O, R/W, uint8_t
/// The number of seconds that the lock shuts down following wrong code entry.
#define ATTRID_DOOR_LOCK_USER_CODE_TEMPORARY_DISABLE_TIME                    0x0031  // O, R/W, uint8_t
/// Boolean set to True if it is ok for the door lock server to send PINs over the air.
#define ATTRID_DOOR_LOCK_SEND_PIN_OVER_THE_AIR                                        0x0032  // O, R/W, BOOLEAN
/// Boolean set to True if the door lock server requires that an optional PINs
/// be included in the payload of RF lock operation events like Lock, Unlock
/// and Toggle in order to function.
#define ATTRID_DOORLOCK_REQUIRE_PIN_FOR_RF_OPERATION                        0x0033  // O, R/W, BOOLEAN
#define ATTRID_DOOR_LOCK_REQUIRE_PI_NFOR_RF_OPERATION                        0x0033  // O, R/W, BOOLEAN
/// Door locks MAY sometimes wish to implement a higher level of security
/// within the application protocol in addition to the default network security.
#define ATTRID_DOOR_LOCK_SECURITY_LEVEL                               0x0034  // O, R, uint8_t

// Alarm and Event Masks Attributes
/// The alarm mask is used to turn on/off alarms for particular functions.
#define ATTRID_DOOR_LOCK_ALARM_MASK                                          0x0040  // O, R/W, BITMAP16
/// Event mask used to turn on and off the transmission of keypad operation events.
#define ATTRID_DOOR_LOCK_KEYPAD_OPERATION_EVENT_MASK                         0x0041  // O, R/W, BITMAP16
/// Event mask used to turn on and off the transmission of RF operation events.
#define ATTRID_DOOR_LOCK_RF_OPERATION_EVENT_MASK                             0x0042  // O, R/W, BITMAP16
/// Event mask used to turn on and off manual operation events.
#define ATTRID_DOOR_LOCK_MANUAL_OPERATION_EVENT_MASK                         0x0043  // O, R/W, BITMAP16
/// Event mask used to turn on and off RFID operation events.
#define ATTRID_DOOR_LOCK_RFID_OPERATION_EVENT_MASK                           0x0044  // O, R/W, BITMAP16
/// Event mask used to turn on and off keypad programming events.
#define ATTRID_DOOR_LOCK_KEYPAD_PROGRAMMING_EVENT_MASK                       0x0045  // O, R/W, BITMAP16
/// Event mask used to turn on and off RF programming events.
#define ATTRID_DOOR_LOCK_RF_PROGRAMMING_EVENT_MASK                           0x0046  // O, R/W, BITMAP16
/// Event mask used to turn on and off RFID programming events.
#define ATTRID_DOOR_LOCK_RFID_PROGRAMMING_EVENT_MASK                         0x0047  // O, R/W, BITMAP16

// User, PIN, Schedule, & Log Information Attribute Defaults
/// User, PIN, Schedule, Log Information Attribute Set default values
#define ATTR_DEFAULT_DOORLOCK_NUM_OF_LOCK_RECORDS_SUPPORTED                       0
#define ATTR_DEFAULT_DOORLOCK_NUM_OF_TOTAL_USERS_SUPPORTED                        0
#define ATTR_DEFAULT_DOORLOCK_NUM_OF_PIN_USERS_SUPPORTED                          0
#define ATTR_DEFAULT_DOORLOCK_NUM_OF_RFID_USERS_SUPPORTED                         0
#define ATTR_DEFAULT_DOORLOCK_NUM_OF_WEEK_DAY_SCHEDULES_SUPPORTED_PER_USER        0
#define ATTR_DEFAULT_DOORLOCK_NUM_OF_YEAR_DAY_SCHEDULES_SUPPORTED_PER_USER        0
#define ATTR_DEFAULT_DOORLOCK_NUM_OF_HOLIDAY_SCHEDULEDS_SUPPORTED                 0
#define ATTR_DEFAULT_DOORLOCK_MAX_PIN_LENGTH                                      0x08
#define ATTR_DEFAULT_DOORLOCK_MIN_PIN_LENGTH                                      0x04
#define ATTR_DEFAULT_DOORLOCK_MAX_RFID_LENGTH                                     0x14
#define ATTR_DEFAULT_DOORLOCK_MIN_RFID_LENGTH                                     0x08

// Operational Settings Attribute Defaults
/// Operational Settings Attribute Set default values
#define ATTR_DEFAULT_DOORLOCK_ENABLE_LOGGING                                      0
#define ATTR_DEFAULT_DOORLOCK_LANGUAGE                                            {0,0,0}
#define ATTR_DEFAULT_DOORLOCK_LED_SETTINGS                                        0
#define ATTR_DEFAULT_DOORLOCK_AUTO_RELOCK_TIME                                    0
#define ATTR_DEFAULT_DOORLOCK_SOUND_VOLUME                                        0
#define ATTR_DEFAULT_DOORLOCK_OPERATING_MODE                                      0
#define ATTR_DEFAULT_DOORLOCK_SUPPORTED_OPERATING_MODES                           0x0001
#define ATTR_DEFAULT_DOORLOCK_DEFAULT_CONFIGURATION_REGISTER                      0
#define ATTR_DEFAULT_DOORLOCK_ENABLE_LOCAL_PROGRAMMING                            0
#define ATTR_DEFAULT_DOORLOCK_ENABLE_ONE_TOUCH_LOCKING                            0
#define ATTR_DEFAULT_DOORLOCK_ENABLE_INSIDE_STATUS_LED                            0
#define ATTR_DEFAULT_DOORLOCK_ENABLE_PRIVACY_MODE_BUTTON                          0

// Security Settings Attribute Defaults
/// Security Settings Attribute Set default values
#define ATTR_DEFAULT_DOORLOCK_WRONG_CODE_ENTRY_LIMIT                              0
#define ATTR_DEFAULT_DOORLOCK_USER_CODE_TEMPORARY_DISABLE_TIME                    0
#define ATTR_DEFAULT_DOORLOCK_SEND_PIN_OTA                                        0
#define ATTR_DEFAULT_DOORLOCK_REQUIRE_PIN_FOR_RF_OPERATION                        0
#define ATTR_DEFAULT_DOORLOCK_ZIGBEE_SECURITY_LEVEL                               0

// Alarm and Event Masks Attribute Defaults
/// The alarm mask is used to turn on/off alarms for particular functions.
#define ATTR_DEFAULT_DOORLOCK_ALARM_MASK                                          0x0000
/// Event mask used to turn on and off the transmission of keypad operation events.
#define ATTR_DEFAULT_DOORLOCK_KEYPAD_OPERATION_EVENT_MASK                         0x0000
/// Event mask used to turn on and off the transmission of RF operation events.
#define ATTR_DEFAULT_DOORLOCK_RF_OPERATION_EVENT_MASK                             0x0000
/// Event mask used to turn on and off manual operation events.
#define ATTR_DEFAULT_DOORLOCK_MANUAL_OPERATION_EVENT_MASK                         0x0000
/// Event mask used to turn on and off RFID operation events.
#define ATTR_DEFAULT_DOORLOCK_RFID_OPERATION_EVENT_MASK                           0x0000
/// Event mask used to turn on and off keypad programming events.
#define ATTR_DEFAULT_DOORLOCK_KEYPAD_PROGRAMMING_EVENT_MASK                       0x0000
/// Event mask used to turn on and off RF programming events.
#define ATTR_DEFAULT_DOORLOCK_RF_PROGRAMMING_EVENT_MASK                           0x0000
/// Event mask used to turn on and off RFID programming events.
#define ATTR_DEFAULT_DOORLOCK_RFID_PROGRAMMING_EVENT_MASK                         0x0000

 /******************************************************************************************
  * Operating Mode enumerations
  * Interface: (E = Enable; D = Disable)
  * Devices:  (K = Keypad; RF; RFID)
  */
/// Normal Mode: The lock operates normally. All interfaces are enabled.
#define DOORLOCK_OP_MODE_NORMAL                   0x00  // K = E;   RF = E;   RFID = E
/// Vacation Mode: Only RF interaction is enabled. The keypad cannot be operated.
#define DOORLOCK_OP_MODE_VACATION                 0x01  // K = D;   RF = E;   RFID = E
/// Privacy Mode: All external interaction with the door lock is disabled.
#define DOORLOCK_OP_MODE_PRIVACY                  0x02  // K = D;   RF = D;   RFID = D
/// No RF Lock or Unlock: This mode only disables RF interaction with the lock.
#define DOORLOCK_OP_MODE_NO_RF_LOCK_UNLOCK        0x03  // K = E;   RF = D;   RFID = E
/// Passage Mode: The lock is open or can be open or closed at will without
/// the use of a Keypad or other means of user validation.
#define DOORLOCK_OP_MODE_PASSAGE                  0x04  // K = N/A; RF = N/A; RFID = N/A

/*** Lock State Attribute types ***/
/// Lock state values.
#define CLOSURES_LOCK_STATE_NOT_FULLY_LOCKED               0x00
#define CLOSURES_LOCK_STATE_LOCKED                         0x01
#define CLOSURES_LOCK_STATE_UNLOCKED                       0x02

/*** Lock Type Attribute types ***/
/// Lock type values
#define CLOSURES_LOCK_TYPE_DEADBOLT                        0x00
#define CLOSURES_LOCK_TYPE_MAGNETIC                        0x01
#define CLOSURES_LOCK_TYPE_OTHER                           0x02
#define CLOSURES_LOCK_TYPE_MORTISE                         0x03
#define CLOSURES_LOCK_TYPE_RIM                             0x04
#define CLOSURES_LOCK_TYPE_LATCH_BOLT                      0x05
#define CLOSURES_LOCK_TYPE_CYLINDRICAL_LOCK                0x06
#define CLOSURES_LOCK_TYPE_TUBULAR_LOCK                    0x07
#define CLOSURES_LOCK_TYPE_INTERCONNECTED_LOCK             0x08
#define CLOSURES_LOCK_TYPE_DEAD_LATCH                      0x09
#define CLOSURES_LOCK_TYPE_DOOR_FURNITURE                  0x0A

/*** Door State Attribute types ***/
/// Door State values
#define CLOSURES_DOOR_STATE_OPEN                           0x00
#define CLOSURES_DOOR_STATE_CLOSED                         0x01
#define CLOSURES_DOOR_STATE_ERROR_JAMMED                   0x02
#define CLOSURES_DOOR_STATE_ERROR_FORCED_OPEN              0x03
#define CLOSURES_DOOR_STATE_ERROR_UNSPECIFIED              0x04

/**********************************/
/*** Door Lock Cluster Commands ***/
/**********************************/
  // Server Commands Received
/// This command causes the lock device to lock the door.
#define COMMAND_DOOR_LOCK_LOCK_DOOR                         0x00 // M  zclDoorLock_t
/// This command causes the lock device to unlock the door.
#define COMMAND_DOOR_LOCK_UNLOCK_DOOR                       0x01 // M  zclDoorLock_t
/// Request the status of the lock.
#define COMMAND_DOOR_LOCK_TOGGLE                       0x02 // O  zclDoorLock_t
/// This command causes the lock device to unlock the door with a timeout parameter.
#define COMMAND_DOOR_LOCK_UNLOCK_WITH_TIMEOUT               0x03 // O  zclDoorLockUnlockTimeout_t
/// Request a log record.
#define COMMAND_DOOR_LOCK_GET_LOG_RECORD                    0x04 // O  zclDoorLockGetLogRecord_t
/// Set a PIN into the lock.
#define COMMAND_DOOR_LOCK_SET_PIN_CODE                      0x05 // O  zclDoorLockSetPINCode_t
/// Retrieve a PIN Code.
#define COMMAND_DOOR_LOCK_GET_PIN_CODE                      0x06 // O  zclDoorLockUserID_t
/// Delete a PIN.
#define COMMAND_DOOR_LOCK_CLEAR_PIN_CODE                    0x07 // O  zclDoorLockUserID_t
/// Clear out all PINs on the lock.
#define COMMAND_DOOR_LOCK_CLEAR_ALL_PIN_CODES               0x08 // O  no payload
/// Set the status of a user ID.
#define COMMAND_DOOR_LOCK_SET_USER_STATUS                   0x09 // O  zclDoorLockSetUserStatus_t
/// Get the status of a user.
#define COMMAND_DOOR_LOCK_GET_USER_STATUS                   0x0A // O  zclDoorLockUserID_t
/// Set a weekly repeating schedule for a specified user.
#define COMMAND_DOOR_LOCK_SET_WEEKDAY_SCHEDULE             0x0B // O  zclDoorLockSetWeekDaySchedule_t
/// Retrieve the specific weekly schedule for the specific user.
#define COMMAND_DOOR_LOCK_GET_WEEKDAY_SCHEDULE             0x0C // O  zclDoorLockSchedule_t
/// Clear the specific weekly schedule for the specific user.
#define COMMAND_DOOR_LOCK_CLEAR_WEEKDAY_SCHEDULE           0x0D // O  zclDoorLockSchedule_t
/// Set a time-specific schedule ID for a specified user.
#define COMMAND_DOOR_LOCK_SET_YEAR_DAY_SCHEDULE             0x0E // O  zclDoorLockSetYearDaySchedule_t
/// Retrieve the specific year day schedule for the specific user.
#define COMMAND_DOOR_LOCK_GET_YEAR_DAY_SCHEDULE             0x0F // O  zclDoorLockSchedule_t
/// Clears the specific year day schedule for the specific user.
#define COMMAND_DOOR_LOCK_CLEAR_YEAR_DAY_SCHEDULE           0x10 // O  zclDoorLockSchedule_t
/// Set the holiday Schedule by specifying local start time and local end time
/// with respect to any Lock Operating Mode.
#define COMMAND_DOOR_LOCK_SET_HOLIDAY_SCHEDULE              0x11 // O  zclDoorLockSetHolidaySchedule_t
/// Get the holiday Schedule by specifying Holiday ID.
#define COMMAND_DOOR_LOCK_GET_HOLIDAY_SCHEDULE              0x12 // O  zclDoorLockHolidayScheduleID_t
/// Clear the holiday Schedule by specifying Holiday ID.
#define COMMAND_DOOR_LOCK_CLEAR_HOLIDAY_SCHEDULE            0x13 // O  zclDoorLockHolidayScheduleID_t
/// Set the type byte for a specified user.
#define COMMAND_DOOR_LOCK_SET_USER_TYPE                     0x14 // O  zclDoorLockSetUserType_t
/// Retrieve the type byte for a specific user.
#define COMMAND_DOOR_LOCK_GET_USER_TYPE                     0x15 // O  zclDoorLockUserID_t
/// Set an ID for RFID access into the lock.
#define COMMAND_DOOR_LOCK_SET_RFID_CODE                     0x16 // O  zclDoorLockSetRFIDCode_t
/// Retrieve an ID.
#define COMMAND_DOOR_LOCK_GET_RFID_CODE                     0x17 // O  zclDoorLockUserID_t
/// Delete an ID.
#define COMMAND_DOOR_LOCK_CLEAR_RFID_CODE                   0x18 // O  zclDoorLockUserID_t
/// Clear out all RFIDs on the lock.
#define COMMAND_DOOR_LOCK_CLEAR_ALL_RFID_CODES              0x19 // O  no payload

// Server Commands Generated
/// This command is sent in response to a Lock command with one status byte payload.
#define COMMAND_DOOR_LOCK_LOCK_DOOR_RESPONSE                     0x00 // M  status field
/// This command is sent in response to a Toggle command with one status byte payload.
#define COMMAND_DOOR_LOCK_UNLOCK_DOOR_RESPONSE                   0x01 // M  status field
/// This command is sent in response to a Toggle command with one status byte payload.
#define COMMAND_DOOR_LOCK_TOGGLE_RESPONSE                   0x02 // O  status field
/// This command is sent in response to an Unlock with Timeout command with
/// one status byte payload.
#define COMMAND_DOOR_LOCK_UNLOCK_WITH_TIMEOUT_RESPONSE           0x03 // O  status field
/// Returns the specified log record.
#define COMMAND_DOOR_LOCK_GET_LOG_RECORD_RESPONSE                0x04 // O  zclDoorLockGetLogRecordRsp_t
/// Returns status of the PIN set command.
#define COMMAND_DOOR_LOCK_SET_PIN_CODE_RESPONSE                  0x05 // O  status field
/// Returns the PIN for the specified user ID.
#define COMMAND_DOOR_LOCK_GET_PIN_CODE_RESPONSE                  0x06 // O  zclDoorLockGetPINCodeRsp_t
/// Returns pass/fail of the command.
#define COMMAND_DOOR_LOCK_CLEAR_PIN_CODE_RESPONSE                0x07 // O  status field
/// Returns pass/fail of the command.
#define COMMAND_DOOR_LOCK_CLEAR_ALL_PIN_CODES_RESPONSE           0x08 // O  status field
/// Returns the pass or fail value for the setting of the user status.
#define COMMAND_DOOR_LOCK_SET_USER_STATUS_RESPONSE               0x09 // O  status field
/// Returns the user status for the specified user ID.
#define COMMAND_DOOR_LOCK_GET_USER_STATUS_RESPONSE               0x0A // O  zclDoorLockGetUserStateRsp_t
/// Returns pass/fail of the command.
#define COMMAND_DOOR_LOCK_SET_WEEKDAY_SCHEDULE_RESPONSE         0x0B // O  status field
/// Returns the weekly repeating schedule data for the specified schedule ID.
#define COMMAND_DOOR_LOCK_GET_WEEKDAY_SCHEDULE_RESPONSE         0x0C // O  zclDoorLockGetWeekDayScheduleRsp_t
/// Returns pass/fail of the command.
#define COMMAND_DOOR_LOCK_CLEAR_WEEKDAY_SCHEDULE_RESPONSE       0x0D // O  status field
/// Returns pass/fail of the command.
#define COMMAND_DOOR_LOCK_SET_YEAR_DAY_SCHEDULE_RESPONSE         0x0E // O  status field
/// Returns the weekly repeating schedule data for the specified schedule ID.
#define COMMAND_DOOR_LOCK_GET_YEAR_DAY_SCHEDULE_RESPONSE         0x0F // O  zclDoorLockGetYearDayScheduleRsp_t
/// Returns pass/fail of the command.
#define COMMAND_DOOR_LOCK_CLEAR_YEAR_DAY_SCHEDULE_RESPONSE       0x10 // O  status field
/// Returns pass/fail of the command.
#define COMMAND_DOOR_LOCK_SET_HOLIDAY_SCHEDULE_RESPONSE          0x11 // O  status field
/// Returns the Holiday Schedule Entry for the specified Holiday ID.
#define COMMAND_DOOR_LOCK_GET_HOLIDAY_SCHEDULE_RESPONSE          0x12 // O  zclDoorLockGetHolidayScheduleRsp_t
/// Returns pass/fail of the command.
#define COMMAND_DOOR_LOCK_CLEAR_HOLIDAY_SCHEDULE_RESPONSE        0x13 // O  status field
/// Returns the pass or fail value for the setting of the user type.
#define COMMAND_DOOR_LOCK_SET_USER_TYPE_RESPONSE                 0x14 // O  status field
/// Returns the user type for the specified user ID.
#define COMMAND_DOOR_LOCK_GET_USER_TYPE_RESPONSE                 0x15 // O  zclDoorLockGetUserTypeRsp_t
/// Returns status of the Set RFID Code command.
#define COMMAND_DOOR_LOCK_SET_RFID_CODE_RESPONSE                 0x16 // O  status field
/// Returns the RFID code for the specified user ID.
#define COMMAND_DOOR_LOCK_GET_RFID_CODE_RESPONSE                 0x17 // O  zclDoorLockGetRFIDCodeRsp_t
/// Returns pass/fail of the command.
#define COMMAND_DOOR_LOCK_CLEAR_RFID_CODE_RESPONSE               0x18 // O  status field
/// Returns pass/fail of the command.
#define COMMAND_DOOR_LOCK_CLEAR_ALL_RFID_CODES_RESPONSE          0x19 // O  status field
/// The door lock server sends out operation event notification when the event
/// is triggered by the various event sources.
#define COMMAND_DOOR_LOCK_OPERATING_EVENT_NOTIFICATION      0x20 // O  zclDoorLockOperationalEventNotification_t
/// The door lock optionally sends out notifications (if they are enabled)
/// whenever there is a significant operational event on the lock.
#define COMMAND_DOOR_LOCK_PROGRAMMING_EVENT_NOTIFICATION    0x21 // O  zclDoorLockProgrammingEventNotification_t


/*** User Status Value enums ***/
/// The following User Status and User Type values are used in the payload
/// of multiple commands.
#define USER_STATUS_AVAILABLE                                   0x00
#define USER_STATUS_OCCUPIED_ENABLED                            0x01
#define USER_STATUS_RESERVED                                    0x02
#define USER_STATUS_OCCUPIED_DISABLED                           0x03

/*** User Type Value enums ***/
/// Used to indicate what the type is for a specific user ID.
#define USER_TYPE_UNRESTRICTED_USER                             0x00 // default
#define USER_TYPE_YEAR_DAY_SCHEDULE_USER                        0x01
#define USER_TYPE_WEEK_DAY_SCHEDULE_USER                        0x02
#define USER_TYPE_MASTER_USER                                   0x03

/*** Operation (Programming) Event Source Value enums ***/
/// A source value where available sources are
#define OPERATION_EVENT_SOURCE_KEYPAD                           0x00
#define OPERATION_EVENT_SOURCE_RF                               0x01
#define OPERATION_EVENT_SOURCE_MANUAL                           0x02   // "Reserved" for Programming Event
#define OPERATION_EVENT_SOURCE_RFID                             0x03
#define OPERATION_EVENT_SOURCE_INDETERMINATE                    0xFF

/*** Operation Event Code Value enums ***/
/// Operation Event Code Value enum
#define OPERATION_EVENT_CODE_UNKNOWN_OR_MFG_SPECIFIC            0x00 // Applicable: Keypad, RF, Manual, RFID
#define OPERATION_EVENT_CODE_LOCK                               0x01 // Applicable: Keypad, RF, Manual, RFID
#define OPERATION_EVENT_CODE_UNLOCK                             0x02 // Applicable: Keypad, RF, Manual, RFID
#define OPERATION_EVENT_CODE_LOCK_FAILURE_INVALID_PIN_OR_ID     0x03 // Applicable: Keypad, RF, RFID
#define OPERATION_EVENT_CODE_LOCK_FAILURE_INVALID_SCHEDULE      0x04 // Applicable: Keypad, RF, RFID
#define OPERATION_EVENT_CODE_UNLOCK_FAILURE_INVALID_PIN_OR_ID   0x05 // Applicable: Keypad, RF, RFID
#define OPERATION_EVENT_CODE_UNLOCK_FAILURE_INVALID_SCHEDULE    0x06 // Applicable: Keypad, RF, RFID
#define OPERATION_EVENT_CODE_ONE_TOUCH_LOCK                     0x07 // Applicable: Manual
#define OPERATION_EVENT_CODE_KEY_LOCK                           0x08 // Applicable: Manual
#define OPERATION_EVENT_CODE_KEY_UNLOCK                         0x09 // Applicable: Manual
#define OPERATION_EVENT_CODE_AUTO_LOCK                          0x0A // Applicable: Manual
#define OPERATION_EVENT_CODE_SCHEDULE_LOCK                      0x0B // Applicable: Manual
#define OPERATION_EVENT_CODE_SCHEDULE_UNLOCK                    0x0C // Applicable: Manual
#define OPERATION_EVENT_CODE_MANUAL_LOCK                        0x0D // Applicable: Manual
#define OPERATION_EVENT_CODE_MANUAL_UNLOCK                      0x0E // Applicable: Manual

/*** Programming Event Code enums ***/
/// Programming Event Code enums
#define PROGRAMMING_EVENT_CODE_UNKNOWN_OR_MFG_SPECIFIC          0x00 // Applicable: Keypad, RF, RFID
#define PROGRAMMING_EVENT_CODE_MASTER_CODE_CHANGED              0x01 // Applicable: Keypad
#define PROGRAMMING_EVENT_CODE_PIN_CODE_ADDED                   0x02 // Applicable: Keypad, RF
#define PROGRAMMING_EVENT_CODE_PIN_CODE_DELETED                 0x03 // Applicable: Keypad, RF
#define PROGRAMMING_EVENT_CODE_PIN_CODE_CHANGED                 0x04 // Applicable: Keypad, RF
#define PROGRAMMING_EVENT_CODE_RFID_CODE_ADDED                  0x05 // Applicable: RFID
#define PROGRAMMING_EVENT_CODE_RFID_CODE_DELETED                0x06 // Applicable: RFID

/// Door Lock cluster commands payload lengths
#define DOORLOCK_RES_PAYLOAD_LEN                                0x01
#define PAYLOAD_LEN_UNLOCK_TIMEOUT   2
#define PAYLOAD_LEN_GET_LOG_RECORD    2
#define PAYLOAD_LEN_SET_PIN_CODE    4 // not including pPIN
#define PAYLOAD_LEN_USER_ID   2
#define PAYLOAD_LEN_SET_USER_STATUS   3
#define PAYLOAD_LEN_SET_WEEK_DAY_SCHEDULE   8
#define PAYLOAD_LEN_SCHEDULE    3
#define PAYLOAD_LEN_SET_YEAR_DAY_SCHEDULE   11
#define PAYLOAD_LEN_SET_HOLIDAY_SCHEDULE    10
#define PAYLOAD_LEN_HOLIDAY_SCHEDULE    1
#define PAYLOAD_LEN_SET_USER_TYPE   3
#define PAYLOAD_LEN_SET_RFID_CODE   4 // not including pRfidCode
#define PAYLOAD_LEN_GET_LOG_RECORD_RSP    11  // not including pPIN
#define PAYLOAD_LEN_GET_PIN_CODE_RSP    4 // not including pCode
#define PAYLOAD_LEN_GET_USER_STATUS_RSP   3
#define PAYLOAD_LEN_GET_USER_TYPE_RSP   3
#define PAYLOAD_LEN_GET_WEEK_DAY_SCHEDULE_RSP   9
#define PAYLOAD_LEN_GET_YEAR_DAY_SCHEDULE_RSP   12
#define PAYLOAD_LEN_GET_HOLIDAY_SCHEDULE_RSP    11
#define PAYLOAD_LEN_GET_RFID_CODE_RSP   4 // not including pRfidCode
#define PAYLOAD_LEN_OPERATION_EVENT_NOTIFICATION    9 // not including pData
#define PAYLOAD_LEN_PROGRAMMING_EVENT_NOTIFICATION    11 // not including pData
#endif //ZCL_DOORLOCK
/** @} End CLOSURE_DOORLOCK_MACROS */

/**
* @defgroup CLOSURE_WINDOW_MACROS ZCL Closure Window Covering Cluster Macros
* @brief This group provides ZCL Closure Windo Covering Cluster macros
* defined in the ZCL v7 specification
* @{
*/
#ifdef ZCL_WINDOWCOVERING
/**********************************************/
/*** Window Covering Cluster Attribute Sets ***/
/**********************************************/
#define ATTRSET_WINDOW_COVERING_INFO                        0x0000
#define ATTRSET_WINDOW_COVERING_SETTINGS                    0x0010

/******************************************/
/*** Window Covering Cluster Attributes ***/
/******************************************/
//Window Covering Information
/// The WindowCoveringType attribute identifies the type of window covering
/// being controlled by this endpoint
#define ATTRID_WINDOW_COVERING_WINDOW_COVERING_TYPE                                                0x0000
/// The PhysicalClosedLimitLift attribute identifies the maximum possible encoder
/// position possible (in centimeters) to position the height of the window
/// covering - this is ignored if the device is running in Open Loop Control.
#define ATTRID_WINDOW_COVERING_PHYSICAL_CLOSED_LIMIT_LIFT                                          0x0001
/// The PhysicalClosedLimitTilt attribute identifies the maximum possible
/// encoder position possible (tenth of a degrees) to position the angle of the
/// window covering - this is ignored if the device is running in Open Loop Control.
#define ATTRID_WINDOW_COVERING_PHYSICAL_CLOSED_LIMIT_TILT                                          0x0002
/// The CurrentPositionLift attribute identifies the actual position (in centimeters)
/// of the window covering from the top of the shade if Closed Loop Control is
/// enabled. This attribute is ignored if the device is running in Open Loop Control.
#define ATTRID_WINDOW_COVERING_CURRENT_POSITION_LIFT                                               0x0003
/// The NumberOfActuationsTilt attribute identifies the total number of tilt
/// actuations applied to the Window Covering since the device was installed.
#define ATTRID_WINDOW_COVERING_CURRENT_POSITION_TILT                                               0x0004
/// The NumberOfActuationsLift attribute identifies the total number of lift
/// actuations applied to the Window Covering since the device was installed.
#define ATTRID_WINDOW_COVERING_NUMBER_OF_ACTUATIONS_LIFT                                           0x0005
/// The NumberOfActuationsTilt attribute identifies the total number of tilt
/// actuations applied to the Window Covering since the device was installed.
#define ATTRID_WINDOW_COVERING_NUMBER_OF_ACTUATIONS_TILT                                           0x0006
/// The ConfigStatus attribute makes configuration and status information available.
#define ATTRID_WINDOW_COVERING_CONFIG_OR_STATUS                                                    0x0007
/// The CurrentPositionLiftPercentage attribute identifies the actual position as
/// a percentage between the InstalledOpenLimitLift attribute and the
/// InstalledClosedLimitLift attribute of the window covering from the up/open
/// position if Closed Loop Control is enabled.
#define ATTRID_WINDOW_COVERING_CURRENT_POSITION_LIFT_PERCENTAGE                                    0x0008
/// The CurrentPositionTiltPercentage attribute identifies the actual position as
/// a percentage between the  InstalledOpenLimitTilt attribute and the
/// InstalledClosedLimitTilt attribute of the window covering from the up/open
/// position if Closed Loop Control is enabled.
#define ATTRID_WINDOW_COVERING_CURRENT_POSITION_TILT_PERCENTAGE                                    0x0009

//Window Covering Setting
/// The InstalledOpenLimitLift attribute identifies the Open Limit for Lifting
/// the Window Covering whether position (in centimeters) is encoded or timed.
#define ATTRID_WINDOW_COVERING_INSTALLED_OPEN_LIMIT_LIFT                                               0x0010
/// The InstalledClosedLimitLift attribute identifies the Closed Limit for
/// Lifting the Window Covering whether position (in centimeters) is encoded or timed.
#define ATTRID_WINDOW_COVERING_INSTALLED_CLOSED_LIMIT_LIFT                                             0x0011
/// The InstalledOpenLimitTilt attribute identifies the Open Limit for Tilting
/// the Window Covering whether position (in  tenth of a degree) is encoded or timed.
#define ATTRID_WINDOW_COVERING_INSTALLED_OPEN_LIMIT_TILT                                               0x0012
/// The InstalledClosedLimitTilt attribute identifies the Closed Limit for Tilting
/// the Window Covering whether position (in tenth of a degree) is encoded or timed.
#define ATTRID_WINDOW_COVERING_INSTALLED_CLOSED_LIMIT_TILT                                             0x0013
/// The VelocityLift attribute identifies the velocity (in centimeters per second)
/// associated with Lifting the Window Covering.
#define ATTRID_WINDOW_COVERING_VELOCITY_LIFT                                                           0x0014
/// The AccelerationTimeLift attribute identifies any ramp up times to reaching
/// the velocity setting (in tenth of a second) for positioning the Window Covering.
#define ATTRID_WINDOW_COVERING_ACCELERATION_TIME_LIFT                                                  0x0015
/// The DecelerationTimeLift attribute identifies any ramp down times associated
/// with stopping the positioning (in tenth of a second) of the Window Covering.
#define ATTRID_WINDOW_COVERING_DECELERATION_TIME_LIFT                                                  0x0016
/// The Mode attribute allows configuration of the Window Covering.
#define ATTRID_WINDOW_COVERING_MODE                                                                    0x0017
/// Identifies the number of Intermediate Setpoints supported by the Window Covering
/// for Lift and then identifies the position settings for those Intermediate
/// Setpoints if Closed Loop Control is supported.
#define ATTRID_WINDOW_COVERING_INTERMEDIATE_SETPOINTS_LIFT                                             0x0018
/// Identifies the number of Intermediate Setpoints supported by the Window
/// Covering for Tilt and then identifies the position settings for those
/// Intermediate Setpoints if Closed Loop Control is supported.
#define ATTRID_WINDOW_COVERING_INTERMEDIATE_SETPOINTS_TILT                                             0x0019

/*** Window Covering Type Attribute types ***/
/// Window Covering Type enum
#define CLOSURES_WINDOW_COVERING_TYPE_ROLLERSHADE                       0x00
#define CLOSURES_WINDOW_COVERING_TYPE_ROLLERSHADE_2_MOTOR               0x01
#define CLOSURES_WINDOW_COVERING_TYPE_ROLLERSHADE_EXTERIOR              0x02
#define CLOSURES_WINDOW_COVERING_TYPE_ROLLERSHADE_EXTERIOR_2_MOTOR      0x03
#define CLOSURES_WINDOW_COVERING_TYPE_DRAPERY                           0x04
#define CLOSURES_WINDOW_COVERING_TYPE_AWNING                            0x05
#define CLOSURES_WINDOW_COVERING_TYPE_SHUTTER                           0x06
#define CLOSURES_WINDOW_COVERING_TYPE_TILT_BLIND_TILT_ONLY              0x07
#define CLOSURES_WINDOW_COVERING_TYPE_TILT_BLIND_LIFT_AND_TILT          0x08
#define CLOSURES_WINDOW_COVERING_TYPE_PROJECTOR_SCREEN                  0x09


/****************************************/
/*** Window Covering Cluster Commands ***/
/****************************************/
/// Upon receipt of this command, the Window Covering will adjust the window so
/// the physical lift is at the  InstalledOpenLimit - Lift and the tilt is at the
/// InstalledOpenLimit - Tilt. This will happen as fast as possible.
#define COMMAND_WINDOW_COVERING_UP_OR_OPEN                            ( 0x00 )
/// Upon receipt of this command, the Window Covering will adjust the window so
/// the physical lift is at the InstalledClosedLimit - Lift and the tilt is at the
/// InstalledClosedLimit - Tilt. This will happen as fast as possible.
#define COMMAND_WINDOW_COVERING_DOWN_OR_CLOSE                         ( 0x01 )
/// Upon receipt of this command, the Window Covering will stop any adjusting to
/// the physical tilt and lift that is currently  occurring.
#define COMMAND_WINDOW_COVERING_STOP                               ( 0x02 )
/// Upon receipt of this command, the Window Covering will adjust the window so
/// the physical lift is at the lift value specified in the payload of this command
/// as long as that value is not larger than InstalledOpenLimit - Lift and not
/// smaller than InstalledClosedLimit - Lift.
#define COMMAND_WINDOW_COVERING_GO_TO_LIFT_VALUE                   ( 0x04 )
/// Upon receipt of this command, the Window Covering will adjust the window so
/// the physical lift is at the lift percentage specified in the payload of this command.
#define COMMAND_WINDOW_COVERING_GO_TO_LIFT_PERCENTAGE              ( 0x05 )
/// Upon receipt of this command, the Window Covering will adjust the window so
/// the physical tilt is at the tilt value specified in the payload of this command
/// as long as that value is not larger than InstalledOpenLimit - Tilt and not
/// smaller than InstalledClosedLimit - Tilt.
#define COMMAND_WINDOW_COVERING_GO_TO_TILT_VALUE                   ( 0x07 )
/// Upon receipt of this command, the Window Covering will adjust the window so
/// the physical tilt is at the tilt percentage specified in the payload of this command.
#define COMMAND_WINDOW_COVERING_GO_TO_TILT_PERCENTAGE              ( 0x08 )

#define ZCL_WC_GOTOVALUEREQ_PAYLOADLEN                      ( 2 )
#define ZCL_WC_GOTOPERCENTAGEREQ_PAYLOADLEN                 ( 1 )

#endif // ZCL_WINDOWCOVERING
  /** @} End CLOSURE_WINDOW_MACROS */

/*********************************************************************
 * TYPEDEFS
 */

  /**
 * @defgroup ZCL_CLOSURE_TYPEDEFS ZCL Closure Typedefs
 * @{
 * @brief This group defines the typedefs used for closure devices
 */

/**
 * @brief Window Covering Cluster - Bits in Config/Status Attribute
 */
typedef struct
{
  uint8_t Operational : 1;              //!< Window Covering is operational or not
  uint8_t Online : 1;                   //!< Window Covering is enabled for transmitting over the Zigbee network or not
  uint8_t CommandsReversed : 1;         //!< Identifies the direction of rotation for the Window Covering
  uint8_t LiftControl : 1;              //!< Identifies, lift control supports open loop or closed loop
  uint8_t TiltControl : 1;              //!< Identifies, tilt control supports open loop or closed loop
  uint8_t LiftEncoderControlled : 1;    //!< Identifies, lift control uses Timer or Encoder
  uint8_t TiltEncoderControlled : 1;    //!< Identifies, tilt control uses Timer or Encoder
  uint8_t Reserved : 1;
}zclClosuresWcInfoConfigStatus_t;

/**
 * @brief Window Covering Cluster - Bits in Mode Attribute
 */
typedef struct
{
  uint8_t MotorReverseDirection : 1;    //!< Defines the direction of the motor rotation
  uint8_t RunInCalibrationMode : 1;     //!< Defines Window Covering is in calibration mode or in normal mode
  uint8_t RunInMaintenanceMode : 1;     //!< Defines motor is running in maintenance mode or in normal mode
  uint8_t LEDFeedback : 1;              //!< Enables or Disables feedback LED
  uint8_t Reserved : 4;
}zclClosuresWcSetMode_t;

/**
 * @brief Window Covering Cluster - Setpoint type
 */
typedef enum
{
  lift = 0,                         //!< Lift
  tilt = 1,                         //!< Tilt
}setpointType_t;

#ifdef ZCL_DOORLOCK
  /*** Doorlock Cluster Server Commands Received Structs ***/

/**
 * @brief Server Commands Received: Lock Door, Unlock Door, Toggle Door
 */
typedef struct
{
  uint8_t *pPinRfidCode;   //!< The PIN/RFID codes defined in this specification
                         //!< are all in ZCL OCTET STRING format.
} zclDoorLock_t;

/**
 * @brief Server Commands Received: Unlock with Timeout
 */
typedef struct
{
  uint16_t timeout;   	   //!< Timeout in seconds
  uint8_t *pPinRfidCode;   //!< The PIN/RFID codes defined in this specification
                         //!< are all in ZCL OCTET STRING format.
} zclDoorLockUnlockTimeout_t;

/**
 * @brief Server Commands Received: Get Log Record
 */
typedef struct
{
  uint16_t logIndex;            //!< Log record index
} zclDoorLockGetLogRecord_t;

/**
 * @brief Server Commands Received: Set PIN Code
 */
typedef struct
{
  uint16_t userID;		  //!< User ID is between 0 - [# of PIN Users Supported attribute].
  uint8_t userStatus;  	//!< Only the values 1 (Occupied/Enabled) and 3
                      //!< (Occupied/Disabled) are allowed for User Status.
  uint8_t userType;   	//!< e.g. USER_TYPE_UNRESTRICTED_USER.
  uint8_t *pPIN;    		//!< variable length string.
} zclDoorLockSetPINCode_t;

/**
 * @brief Server Commands Received: Get PIN Code, Clear PIN Code, Get User Status,
 *                           Get User Type, Get RFID Code, Clear RFID Code
 */
typedef struct
{
  uint16_t userID;        //!< Requested user id.
} zclDoorLockUserID_t;

/**
 * @brief Server Commands Received: Set User Status
 */
typedef struct
{
  uint16_t userID;      //!< Requested user id.
  uint8_t userStatus;   //!< e.g. USER_STATUS_AVAILABLE
} zclDoorLockSetUserStatus_t;

/**
 * @brief Server Commands Received: Set Week Day Schedule
 */
typedef struct
{
  uint8_t scheduleID;      //!< number is between 0 - [# of Week Day Schedules Per User attribute].
  uint16_t userID;         //!< is between 0 - [# of Total Users Supported attribute].
  uint8_t daysMask;        //!< bitmask of the effective days in the order XSFTWTMS.
  uint8_t startHour;       //!< in decimal format represented by 0x00 - 0x17 (00 to 23 hours).
  uint8_t startMinute;	   //!< in decimal format represented by 0x00 - 0x3B (00 to 59 mins).
  uint8_t endHour;         //!< in decimal format represented by 0x00 - 0x17 (00 to 23 hours).
                         //!, End Hour SHALL be equal or greater 10212 than Start Hour.
  uint8_t endMinute;       //!< in decimal format represented by 0x00 - 0x3B (00 to 59 mins).
} zclDoorLockSetWeekDaySchedule_t;

/**
 * @brief Server Commands Received: Get Week Day Schedule, Clear Week Day Schedule,
 *                           Get Year Day Schedule, Clear Year Day Schedule
 */
typedef struct
{
  uint8_t scheduleID;	  	//!< number is between 0 - [# of Week Day Schedules Per User attribute].
  uint16_t userID;        //!< is between 0 - [# of Total Users Supported attribute].
} zclDoorLockSchedule_t;

/**
 * @brief Server Commands Received: Set Year Day Schedule
 */
typedef struct
{
  uint8_t scheduleID;           	//!< number is between 0 - [# of Week Day Schedules Per User attribute].
  uint16_t userID;              	//!< is between 0 - [# of Total Users Supported attribute].
  uint32_t zigBeeLocalStartTime;	//!< Start time and end time are given in LocalTime.
  uint32_t zigBeeLocalEndTime;  	//!< When the Server Device receives the command,
                                //!< the Server Device MAY change the user type to
                                //!< the specific schedule user type.
} zclDoorLockSetYearDaySchedule_t;

/**
 * @brief Server Commands Received: Set Holiday Schedule
 */
typedef struct
{
 uint8_t holidayScheduleID;           //!< Holiday Schedule ID number is between 0 - [# of Holiday Schedules Supported attribute].
 uint32_t zigBeeLocalStartTime;       //!< Start time and end time are given in LocalTime.
 uint32_t zigBeeLocalEndTime;         //!< End of Holiday time.
 uint8_t operatingModeDuringHoliday;	//!< Operating Mode is valid enumeration value
                                    //!< as listed in operating mode attribute.
} zclDoorLockSetHolidaySchedule_t;

/**
 * @brief Server Commands Received: Get Holiday Schedule, Clear Holiday Schedule
 */
typedef struct
{
  uint8_t holidayScheduleID;	//!< Id for the scheduled holiday.
} zclDoorLockHolidayScheduleID_t;

/**
 * @brief Server Commands Received: Set User Type
 */
typedef struct
{
  uint16_t userID;  	//!< User ID is between 0 - [# of PIN Users Supported attribute].
  uint8_t userType;   //!< e.g. USER_TYPE_UNRESTRICTED_USER
} zclDoorLockSetUserType_t;

/**
 * @brief Server Commands Received: Set RFID Code
 */
typedef struct
{
  uint16_t userID;	 	//!< User ID is between 0 - [# of PIN Users Supported attribute].
  uint8_t userStatus; //!< Only the values 1 (Occupied/Enabled) and 3
                    //!< (Occupied/Disabled) are allowed for User Status.
  uint8_t userType; 	//!< e.g. USER_TYPE_UNRESTRICTED_USER
  uint8_t *pRfidCode;	//!< The PIN/RFID codes defined in this specification
                    //!< are all in ZCL OCTET STRING format.
} zclDoorLockSetRFIDCode_t;

  /*** Doorlock Cluster Server Commands Generated Structs ***/

/**
 * @brief Client Commands Received: Get Log Record Response
 */
typedef struct
{
  uint16_t logEntryID;        //!< the index into the log table where this log entry is stored.
  uint32_t timestamp;         //!< A LocalTime used to timestamp all events and alarms on the door lock.
  uint8_t eventType;    		  //!< Indicates the type of event that took place on the door lock.
  uint8_t source;             //!< Source of log entry
  uint8_t eventIDAlarmCode;	  //!< A one byte value indicating the type of event that
                            //!< took place on the door lock depending on the event code
                            //!<  table provided for a given event type and source.
  uint16_t userID;			      //!< A two byte value indicating the ID of the user who
                            //!< generated the event on the door lock if one is available.
  uint8_t *pPIN;    		    	//!< A string indicating the PIN code or RFID code that was
                            //!< used to create the event on the door lock if one is available.
} zclDoorLockGetLogRecordRsp_t;

/**
 * @brief Client Commands Received: Get PIN Code Response
 */
typedef struct
{
  uint16_t userID;	 	//!< User ID is between 0 - [# of PIN Users Supported attribute].
  uint8_t userStatus; //!< Only the values 1 (Occupied/Enabled) and 3
                    //!< (Occupied/Disabled) are allowed for User Status.
  uint8_t userType; 	//!< e.g. USER_TYPE_UNRESTRICTED_USER
  uint8_t *pCode;   	//!< variable length string
} zclDoorLockGetPINCodeRsp_t;

/**
 * @brief Client Commands Received: Get User Status Response
 */
typedef struct
{
  uint16_t userID;	 	//!< User ID is between 0 - [# of PIN Users Supported attribute].
  uint8_t userStatus; //!< Only the values 1 (Occupied/Enabled) and 3
                    //!< (Occupied/Disabled) are allowed for User Status.
} zclDoorLockGetUserStatusRsp_t;

/**
 * @brief Client Commands Received: Get User Type Response
 */
typedef struct
{
  uint16_t userID;	 	//!< User ID is between 0 - [# of PIN Users Supported attribute].
  uint8_t userType; 	//!< e.g. USER_TYPE_UNRESTRICTED_USER
} zclDoorLockGetUserTypeRsp_t;

/**
 * @brief Client Commands Received: Get Week Day Schedule Response
 */
typedef struct
{
  uint8_t scheduleID;      //!< number is between 0 - [# of Week Day Schedules Per User attribute].
  uint16_t userID;         //!< is between 0 - [# of Total Users Supported attribute].
  uint8_t status;          //!< Request status
  uint8_t daysMask;        //!< bitmask of the effective days in the order XSFTWTMS.
  uint8_t startHour;       //!< in decimal format represented by 0x00 - 0x17 (00 to 23 hours).
  uint8_t startMinute;	   //!< in decimal format represented by 0x00 - 0x3B (00 to 59 mins).
  uint8_t endHour;         //!< in decimal format represented by 0x00 - 0x17 (00 to 23 hours).
                         //!, End Hour SHALL be equal or greater 10212 than Start Hour.
  uint8_t endMinute;       //!< in decimal format represented by 0x00 - 0x3B (00 to 59 mins).
} zclDoorLockGetWeekDayScheduleRsp_t;

/**
 * @brief Client Commands Received: Get Year Day Schedule Response
 */
typedef struct
{
  uint8_t scheduleID;      //!< number is between 0 - [# of Week Day Schedules Per User attribute].
  uint16_t userID;         //!< is between 0 - [# of Total Users Supported attribute].
  uint8_t status;          //!< Request status
  uint32_t zigBeeLocalStartTime;     //!< Start time and end time are given in LocalTime.
  uint32_t zigBeeLocalEndTime;       //!< End of Holiday time.
} zclDoorLockGetYearDayScheduleRsp_t;

/**
 * @brief Client Commands Received: Get Holiday Schedule Response
 */
typedef struct
{
  uint8_t holidayScheduleID;           //!< //!< Holiday Schedule ID number is between 0 - [# of Holiday Schedules Supported attribute].
  uint8_t status;                      //!< //!< Request status
  uint32_t zigBeeLocalStartTime;       //!< Start time and end time are given in LocalTime.
  uint32_t zigBeeLocalEndTime;         //!< End of Holiday time.
  uint8_t operatingModeDuringHoliday;	 //!< Operating Mode is valid enumeration value
                                     //!< as listed in operating mode attribute.
} zclDoorLockGetHolidayScheduleRsp_t;

/**
 * @brief Client Commands Received: Get RFID Code Response
 */
typedef struct
{
  uint16_t userID;	   //!< is between 0 - [# of Total Users Supported attribute].
  uint8_t userStatus;  //!< Only the values 1 (Occupied/Enabled) and 3
                     //!< (Occupied/Disabled) are allowed for User Status.
  uint8_t userType;  	 //!< e.g. USER_TYPE_UNRESTRICTED_USER
  uint8_t *pRfidCode;  //!< The PIN/RFID codes defined in this specification
                     //!< are all in ZCL OCTET STRING format.
} zclDoorLockGetRFIDCodeRsp_t;

/**
 * @brief Client Commands Received: Operation Event Notification
 */
typedef struct
{
  uint8_t operationEventSource;   //!< This field indicates where the event was triggered from.
  uint8_t operationEventCode;   	//!< significant operational event on the lock.
  uint16_t userID;                //!< The User ID who performed the event.
  uint8_t pin;                    //!< The PIN that is associated with the User .
                                //!< ID who performed the event.
  uint32_t zigBeeLocalTime;       //!< The LocalTime that indicates when the event is triggered.
  uint8_t *pData;   			      	//!< The operation event notification command contains a variable string.
} zclDoorLockOperationEventNotification_t;

/**
 * @brief Client Commands Received: Programming Event Notification
 */
typedef struct
{
  uint8_t programEventSource;  	//!< This field indicates where the event was triggered from.
  uint8_t programEventCode;    	//!< significant programming  event on the lock.
  uint16_t userID;              //!< The User ID who performed the event.
  uint8_t pin;                  //!< The PIN that is associated with the User.
  uint8_t userType;   		     	//!< The User Type that is associated with the User ID who performed the event.
  uint8_t userStatus;   	  		//!< The User Status that is associated with the User ID who performed the event.
  uint32_t zigBeeLocalTime;     //!< The LocalTime that indicates when the event is triggered.
  uint8_t *pData;   				    //!< The programming event notification command contains a variable string
} zclDoorLockProgrammingEventNotification_t;

/// This callback is called to process an incoming Door Lock command
typedef ZStatus_t (*zclClosures_DoorLock_t) ( zclIncoming_t *pInMsg, zclDoorLock_t *pInCmd );

/// This callback is called to process an incoming Door Lock Response command
typedef ZStatus_t (*zclClosures_DoorLockRsp_t) ( zclIncoming_t *pInMsg, uint8_t status );

/// This callback is called to process an incoming Unlock With Timeout command
typedef ZStatus_t (*zclClosures_DoorLockUnlockWithTimeout_t) ( zclIncoming_t *pInMsg, zclDoorLockUnlockTimeout_t *pCmd );

/// This callback is called to process an incoming Get Log Record command
typedef ZStatus_t (*zclClosures_DoorLockGetLogRecord_t) ( zclIncoming_t *pInMsg, zclDoorLockGetLogRecord_t *pCmd );

/// This callback is called to process an incoming Set PIN Code command
typedef ZStatus_t (*zclClosures_DoorLockSetPINCode_t) ( zclIncoming_t *pInMsg, zclDoorLockSetPINCode_t *pCmd );

/// This callback is called to process an incoming Get PIN Code command
typedef ZStatus_t (*zclClosures_DoorLockGetPINCode_t) ( zclIncoming_t *pInMsg, zclDoorLockUserID_t *pCmd );

/// This callback is called to process an incoming Clear PIN Code command
typedef ZStatus_t (*zclClosures_DoorLockClearPINCode_t) ( zclIncoming_t *pInMsg, zclDoorLockUserID_t *pCmd );

/// This callback is called to process an incoming Clear All PIN Codes command
typedef ZStatus_t (*zclClosures_DoorLockClearAllPINCodes_t) ( zclIncoming_t *pInMsg );

/// This callback is called to process an incoming Set User Status command
typedef ZStatus_t (*zclClosures_DoorLockSetUserStatus_t) ( zclIncoming_t *pInMsg, zclDoorLockSetUserStatus_t *pCmd );

/// This callback is called to process an incoming Get User Status command
typedef ZStatus_t (*zclClosures_DoorLockGetUserStatus_t) ( zclIncoming_t *pInMsg, zclDoorLockUserID_t *pCmd );

/// This callback is called to process an incoming Set Week Day Schedule command
typedef ZStatus_t (*zclClosures_DoorLockSetWeekDaySchedule_t) ( zclIncoming_t *pInMsg, zclDoorLockSetWeekDaySchedule_t *pCmd );

/// This callback is called to process an incoming Get Week Day Schedule command
typedef ZStatus_t (*zclClosures_DoorLockGetWeekDaySchedule_t) ( zclIncoming_t *pInMsg, zclDoorLockSchedule_t *pCmd );

/// This callback is called to process an incoming Clear Week Day Schedule command
typedef ZStatus_t (*zclClosures_DoorLockClearWeekDaySchedule_t) ( zclIncoming_t *pInMsg, zclDoorLockSchedule_t *pCmd );

/// This callback is called to process an incoming Set Year Day Schedule command
typedef ZStatus_t (*zclClosures_DoorLockSetYearDaySchedule_t) ( zclIncoming_t *pInMsg, zclDoorLockSetYearDaySchedule_t *pCmd );

/// This callback is called to process an incoming Get Year Day Schedule command
typedef ZStatus_t (*zclClosures_DoorLockGetYearDaySchedule_t) ( zclIncoming_t *pInMsg, zclDoorLockSchedule_t *pCmd );

/// This callback is called to process an incoming Clear Year Day Schedule command
typedef ZStatus_t (*zclClosures_DoorLockClearYearDaySchedule_t) ( zclIncoming_t *pInMsg, zclDoorLockSchedule_t *pCmd );

/// This callback is called to process an incoming Set Holiday Schedule command
typedef ZStatus_t (*zclClosures_DoorLockSetHolidaySchedule_t) ( zclIncoming_t *pInMsg, zclDoorLockSetHolidaySchedule_t *pCmd );

/// This callback is called to process an incoming Get Holiday Schedule command
typedef ZStatus_t (*zclClosures_DoorLockGetHolidaySchedule_t) ( zclIncoming_t *pInMsg, zclDoorLockHolidayScheduleID_t *pCmd );

/// This callback is called to process an incoming Clear Holiday Schedule command
typedef ZStatus_t (*zclClosures_DoorLockClearHolidaySchedule_t) ( zclIncoming_t *pInMsg, zclDoorLockHolidayScheduleID_t *pCmd );

/// This callback is called to process an incoming Set User Type command
typedef ZStatus_t (*zclClosures_DoorLockSetUserType_t) ( zclIncoming_t *pInMsg, zclDoorLockSetUserType_t *pCmd );

/// This callback is called to process an incoming Get User Type command
typedef ZStatus_t (*zclClosures_DoorLockGetUserType_t) ( zclIncoming_t *pInMsg, zclDoorLockUserID_t *pCmd );

/// This callback is called to process an incoming Set RFID Code command
typedef ZStatus_t (*zclClosures_DoorLockSetRFIDCode_t) ( zclIncoming_t *pInMsg, zclDoorLockSetRFIDCode_t *pCmd );

/// This callback is called to process an incoming Get RFID Code command
typedef ZStatus_t (*zclClosures_DoorLockGetRFIDCode_t) ( zclIncoming_t *pInMsg, zclDoorLockUserID_t *pCmd );

/// This callback is called to process an incoming Clear RFID Code command
typedef ZStatus_t (*zclClosures_DoorLockClearRFIDCode_t) ( zclIncoming_t *pInMsg, zclDoorLockUserID_t *pCmd );

/// This callback is called to process an incoming Clear All RFID Codes command
typedef ZStatus_t (*zclClosures_DoorLockClearAllRFIDCodes_t) ( zclIncoming_t *pInMsg );

/// This callback is called to process an incoming Lock Door Response command
typedef ZStatus_t (*zclClosures_DoorLockLockDoorRsp_t) ( zclIncoming_t *pInMsg, uint8_t status );

/// This callback is called to process an incoming Unlock Door Response command
typedef ZStatus_t (*zclClosures_DoorLockUnlockDoorRsp_t) ( zclIncoming_t *pInMsg, uint8_t status );

/// This callback is called to process an incoming Toggle Door Response command
typedef ZStatus_t (*zclClosures_DoorLockToggleDoorRsp_t) ( zclIncoming_t *pInMsg, uint8_t status );

/// This callback is called to process an incoming Unlock With Timeout Response command
typedef ZStatus_t (*zclClosures_DoorLockUnlockWithTimeoutRsp_t) ( zclIncoming_t *pInMsg, uint8_t status );

/// This callback is called to process an incoming Get Log Record Response command
typedef ZStatus_t (*zclClosures_DoorLockGetLogRecordRsp_t) ( zclIncoming_t *pInMsg, zclDoorLockGetLogRecordRsp_t *pCmd );

/// This callback is called to process an incoming Set PIN Code Response command
typedef ZStatus_t (*zclClosures_DoorLockSetPINCodeRsp_t) ( zclIncoming_t *pInMsg, uint8_t status );

/// This callback is called to process an incoming Get PIN Code Response command
typedef ZStatus_t (*zclClosures_DoorLockGetPINCodeRsp_t) ( zclIncoming_t *pInMsg, zclDoorLockGetPINCodeRsp_t *pCmd );

/// This callback is called to process an incoming Clear PIN Code Response command
typedef ZStatus_t (*zclClosures_DoorLockClearPINCodeRsp_t) ( zclIncoming_t *pInMsg, uint8_t status );

/// This callback is called to process an incoming Clear All PIN Codes Response command
typedef ZStatus_t (*zclClosures_DoorLockClearAllPINCodesRsp_t) ( zclIncoming_t *pInMsg, uint8_t status );

/// This callback is called to process an incoming Set User Status Response command
typedef ZStatus_t (*zclClosures_DoorLockSetUserStatusRsp_t) ( zclIncoming_t *pInMsg, uint8_t status );

/// This callback is called to process an incoming Get User Status Response command
typedef ZStatus_t (*zclClosures_DoorLockGetUserStatusRsp_t) ( zclIncoming_t *pInMsg, zclDoorLockGetUserStatusRsp_t *pCmd );

/// This callback is called to process an incoming Set Week Day Schedule Response command
typedef ZStatus_t (*zclClosures_DoorLockSetWeekDayScheduleRsp_t) ( zclIncoming_t *pInMsg, uint8_t status );

/// This callback is called to process an incoming Get Week Day Schedule Response command
typedef ZStatus_t (*zclClosures_DoorLockGetWeekDayScheduleRsp_t) ( zclIncoming_t *pInMsg, zclDoorLockGetWeekDayScheduleRsp_t *pCmd );

/// This callback is called to process an incoming Clear Week Day Schedule Response command
typedef ZStatus_t (*zclClosures_DoorLockClearWeekDayScheduleRsp_t) ( zclIncoming_t *pInMsg, uint8_t status );

/// This callback is called to process an incoming Set Year Day Schedule Response command
typedef ZStatus_t (*zclClosures_DoorLockSetYearDayScheduleRsp_t) ( zclIncoming_t *pInMsg, uint8_t status );

/// This callback is called to process an incoming Get Year Day Schedule Response command
typedef ZStatus_t (*zclClosures_DoorLockGetYearDayScheduleRsp_t) ( zclIncoming_t *pInMsg, zclDoorLockGetYearDayScheduleRsp_t *pCmd );

/// This callback is called to process an incoming Clear Year Day Schedule Response command
typedef ZStatus_t (*zclClosures_DoorLockClearYearDayScheduleRsp_t) ( zclIncoming_t *pInMsg, uint8_t status );

/// This callback is called to process an incoming Set Holiday Schedule Response command
typedef ZStatus_t (*zclClosures_DoorLockSetHolidayScheduleRsp_t) ( zclIncoming_t *pInMsg, uint8_t status );

/// This callback is called to process an incoming Get Holiday Schedule Response command
typedef ZStatus_t (*zclClosures_DoorLockGetHolidayScheduleRsp_t) ( zclIncoming_t *pInMsg, zclDoorLockGetHolidayScheduleRsp_t *pCmd );

/// This callback is called to process an incoming Clear Holiday Schedule Response command
typedef ZStatus_t (*zclClosures_DoorLockClearHolidayScheduleRsp_t) ( zclIncoming_t *pInMsg, uint8_t status );

/// This callback is called to process an incoming Set User Type Response command
typedef ZStatus_t (*zclClosures_DoorLockSetUserTypeRsp_t) ( zclIncoming_t *pInMsg, uint8_t status );

/// This callback is called to process an incoming Get User Type Response command
typedef ZStatus_t (*zclClosures_DoorLockGetUserTypeRsp_t) ( zclIncoming_t *pInMsg, zclDoorLockGetUserTypeRsp_t *pCmd );

/// This callback is called to process an incoming Set RFID Code Response command
typedef ZStatus_t (*zclClosures_DoorLockSetRFIDCodeRsp_t) ( zclIncoming_t *pInMsg, uint8_t status );

/// This callback is called to process an incoming Get RFID Code Response command
typedef ZStatus_t (*zclClosures_DoorLockGetRFIDCodeRsp_t) ( zclIncoming_t *pInMsg, zclDoorLockGetRFIDCodeRsp_t *pCmd );

/// This callback is called to process an incoming Clear RFID Code Response command
typedef ZStatus_t (*zclClosures_DoorLockClearRFIDCodeRsp_t) ( zclIncoming_t *pInMsg, uint8_t status );

/// This callback is called to process an incoming Clear All RFID Codes Response command
typedef ZStatus_t (*zclClosures_DoorLockClearAllRFIDCodesRsp_t) ( zclIncoming_t *pInMsg, uint8_t status );

/// This callback is called to process an incoming Operation Event Notification command
typedef ZStatus_t (*zclClosures_DoorLockOperationEventNotification_t) ( zclIncoming_t *pInMsg, zclDoorLockOperationEventNotification_t *pCmd );

/// This callback is called to process an incoming Programming Event Notification command
typedef ZStatus_t (*zclClosures_DoorLockProgrammingEventNotification_t) ( zclIncoming_t *pInMsg, zclDoorLockProgrammingEventNotification_t *pCmd );
#endif // ZCL_DOORLOCK

/// This callback is called to process an incoming Window Covering cluster basic commands
typedef void (*zclClosures_WindowCoveringSimple_t) ( void );

/// This callback is called to process an incoming Window Covering cluster goto percentage commands
typedef bool (*zclClosures_WindowCoveringGotoPercentage_t) ( uint8_t percentage );

/// This callback is called to process an incoming Window Covering cluster goto value commands
typedef bool (*zclClosures_WindowCoveringGotoValue_t) ( uint16_t value );

/// This callback is called to process an incoming Window Covering cluster goto setpoint commands
typedef uint8_t (*zclClosures_WindowCoveringGotoSetpoint_t) ( uint8_t index );

#ifdef ZCL_DOORLOCK
/**
 * @brief Register Callbacks DoorLock Cluster table entry - enter function pointers for callbacks that
 * the application would like to receive
 */
typedef struct
{
  //!< #zclClosures_DoorLock_t
  zclClosures_DoorLock_t                             pfnDoorLock;                               //!< (COMMAND_DOOR_LOCK_LOCK_DOOR)
  //!< #zclClosures_DoorLockRsp_t
  zclClosures_DoorLockRsp_t                          pfnDoorLockRsp;                            //!< (COMMAND_DOOR_LOCK_LOCK_DOOR_RESPONSE)
  //!< #zclClosures_DoorLockUnlockWithTimeout_t
  zclClosures_DoorLockUnlockWithTimeout_t            pfnDoorLockUnlockWithTimeout;              //!< (COMMAND_DOOR_LOCK_UNLOCK_WITH_TIMEOUT)
  //!< #zclClosures_DoorLockGetLogRecord_t
  zclClosures_DoorLockGetLogRecord_t                 pfnDoorLockGetLogRecord;                   //!< (COMMAND_DOOR_LOCK_GET_LOG_RECORD)
  //!< #zclClosures_DoorLockSetPINCode_t
  zclClosures_DoorLockSetPINCode_t                   pfnDoorLockSetPINCode;                     //!< (COMMAND_DOOR_LOCK_SET_PIN_CODE)
  //!< #zclClosures_DoorLockGetPINCode_t
  zclClosures_DoorLockGetPINCode_t                   pfnDoorLockGetPINCode;                     //!< (COMMAND_DOOR_LOCK_GET_PIN_CODE)
  //!< #zclClosures_DoorLockClearPINCode_t
  zclClosures_DoorLockClearPINCode_t                 pfnDoorLockClearPINCode;                   //!< (COMMAND_DOOR_LOCK_CLEAR_PIN_CODE)
  //!< #zclClosures_DoorLockClearAllPINCodes_t
  zclClosures_DoorLockClearAllPINCodes_t             pfnDoorLockClearAllPINCodes;               //!< (COMMAND_DOOR_LOCK_CLEAR_ALL_PIN_CODES)
  //!< #zclClosures_DoorLockSetUserStatus_t
  zclClosures_DoorLockSetUserStatus_t                pfnDoorLockSetUserStatus;                  //!< (COMMAND_DOOR_LOCK_SET_USER_STATUS)
  //!< #zclClosures_DoorLockGetUserStatus_t
  zclClosures_DoorLockGetUserStatus_t                pfnDoorLockGetUserStatus;                  //!< (COMMAND_DOOR_LOCK_GET_USER_STATUS)
  //!< #zclClosures_DoorLockSetWeekDaySchedule_t
  zclClosures_DoorLockSetWeekDaySchedule_t           pfnDoorLockSetWeekDaySchedule;             //!< (COMMAND_DOOR_LOCK_SET_WEEKDAY_SCHEDULE)
  //!< #zclClosures_DoorLockGetWeekDaySchedule_t
  zclClosures_DoorLockGetWeekDaySchedule_t           pfnDoorLockGetWeekDaySchedule;             //!< (COMMAND_DOOR_LOCK_GET_WEEKDAY_SCHEDULE)
  //!< #zclClosures_DoorLockClearWeekDaySchedule_t
  zclClosures_DoorLockClearWeekDaySchedule_t         pfnDoorLockClearWeekDaySchedule;           //!< (COMMAND_DOOR_LOCK_CLEAR_WEEKDAY_SCHEDULE)
  //!< #zclClosures_DoorLockSetYearDaySchedule_t
  zclClosures_DoorLockSetYearDaySchedule_t           pfnDoorLockSetYearDaySchedule;             //!< (COMMAND_DOOR_LOCK_SET_YEAR_DAY_SCHEDULE)
  //!< #zclClosures_DoorLockGetYearDaySchedule_t
  zclClosures_DoorLockGetYearDaySchedule_t           pfnDoorLockGetYearDaySchedule;             //!< (COMMAND_DOOR_LOCK_GET_YEAR_DAY_SCHEDULE)
  //!< #zclClosures_DoorLockClearYearDaySchedule_t
  zclClosures_DoorLockClearYearDaySchedule_t         pfnDoorLockClearYearDaySchedule;           //!< (COMMAND_DOOR_LOCK_CLEAR_YEAR_DAY_SCHEDULE)
  //!< #zclClosures_DoorLockSetHolidaySchedule_t
  zclClosures_DoorLockSetHolidaySchedule_t           pfnDoorLockSetHolidaySchedule;             //!< (COMMAND_DOOR_LOCK_SET_HOLIDAY_SCHEDULE)
  //!< #zclClosures_DoorLockGetHolidaySchedule_t
  zclClosures_DoorLockGetHolidaySchedule_t           pfnDoorLockGetHolidaySchedule;             //!< (COMMAND_DOOR_LOCK_GET_HOLIDAY_SCHEDULE)
  //!< #zclClosures_DoorLockClearHolidaySchedule_t
  zclClosures_DoorLockClearHolidaySchedule_t         pfnDoorLockClearHolidaySchedule;           //!< (COMMAND_DOOR_LOCK_CLEAR_HOLIDAY_SCHEDULE)
  //!< #zclClosures_DoorLockSetUserType_t
  zclClosures_DoorLockSetUserType_t                  pfnDoorLockSetUserType;                    //!< (COMMAND_DOOR_LOCK_SET_USER_TYPE)
  //!< #zclClosures_DoorLockGetUserType_t
  zclClosures_DoorLockGetUserType_t                  pfnDoorLockGetUserType;                    //!< (COMMAND_DOOR_LOCK_GET_USER_TYPE)
  //!< #zclClosures_DoorLockSetRFIDCode_t
  zclClosures_DoorLockSetRFIDCode_t                  pfnDoorLockSetRFIDCode;                    //!< (COMMAND_DOOR_LOCK_SET_RFID_CODE)
  //!< #zclClosures_DoorLockGetRFIDCode_t
  zclClosures_DoorLockGetRFIDCode_t                  pfnDoorLockGetRFIDCode;                    //!< (COMMAND_DOOR_LOCK_GET_RFID_CODE)
  //!< #zclClosures_DoorLockClearRFIDCode_t
  zclClosures_DoorLockClearRFIDCode_t                pfnDoorLockClearRFIDCode;                  //!< (COMMAND_DOOR_LOCK_CLEAR_RFID_CODE)
  //!< #zclClosures_DoorLockClearAllRFIDCodes_t
  zclClosures_DoorLockClearAllRFIDCodes_t            pfnDoorLockClearAllRFIDCodes;              //!< (COMMAND_DOOR_LOCK_CLEAR_ALL_RFID_CODES)
  //!< #zclClosures_DoorLockUnlockWithTimeoutRsp_t
  zclClosures_DoorLockUnlockWithTimeoutRsp_t         pfnDoorLockUnlockWithTimeoutRsp;           //!< (COMMAND_DOOR_LOCK_UNLOCK_WITH_TIMEOUT_RESPONSE)
  //!< #zclClosures_DoorLockGetLogRecordRsp_t
  zclClosures_DoorLockGetLogRecordRsp_t              pfnDoorLockGetLogRecordRsp;                //!< (COMMAND_DOOR_LOCK_GET_LOG_RECORD_RESPONSE)
  //!< #zclClosures_DoorLockSetPINCodeRsp_t
  zclClosures_DoorLockSetPINCodeRsp_t                pfnDoorLockSetPINCodeRsp;                  //!< (COMMAND_DOOR_LOCK_SET_PIN_CODE_RESPONSE)
  //!< #zclClosures_DoorLockGetPINCodeRsp_t
  zclClosures_DoorLockGetPINCodeRsp_t                pfnDoorLockGetPINCodeRsp;                  //!< (COMMAND_DOOR_LOCK_GET_PIN_CODE_RESPONSE)
  //!< #zclClosures_DoorLockClearPINCodeRsp_t
  zclClosures_DoorLockClearPINCodeRsp_t              pfnDoorLockClearPINCodeRsp;                //!< (COMMAND_DOOR_LOCK_CLEAR_PIN_CODE_RESPONSE)
  //!< #zclClosures_DoorLockClearAllPINCodesRsp_t
  zclClosures_DoorLockClearAllPINCodesRsp_t          pfnDoorLockClearAllPINCodesRsp;            //!< (COMMAND_DOOR_LOCK_CLEAR_ALL_PIN_CODES_RESPONSE)
  //!< #zclClosures_DoorLockSetUserStatusRsp_t
  zclClosures_DoorLockSetUserStatusRsp_t             pfnDoorLockSetUserStatusRsp;               //!< (COMMAND_DOOR_LOCK_SET_USER_STATUS_RESPONSE)
  //!< #zclClosures_DoorLockGetUserStatusRsp_t
  zclClosures_DoorLockGetUserStatusRsp_t             pfnDoorLockGetUserStatusRsp;               //!< (COMMAND_DOOR_LOCK_GET_USER_STATUS_RESPONSE)
  //!< #zclClosures_DoorLockSetWeekDayScheduleRsp_t
  zclClosures_DoorLockSetWeekDayScheduleRsp_t        pfnDoorLockSetWeekDayScheduleRsp;          //!< (COMMAND_DOOR_LOCK_SET_WEEKDAY_SCHEDULE_RESPONSE)
  //!< #zclClosures_DoorLockGetWeekDayScheduleRsp_t
  zclClosures_DoorLockGetWeekDayScheduleRsp_t        pfnDoorLockGetWeekDayScheduleRsp;          //!< (COMMAND_DOOR_LOCK_GET_WEEKDAY_SCHEDULE_RESPONSE)
  //!< #zclClosures_DoorLockClearWeekDayScheduleRsp_t
  zclClosures_DoorLockClearWeekDayScheduleRsp_t      pfnDoorLockClearWeekDayScheduleRsp;        //!< (COMMAND_DOOR_LOCK_CLEAR_WEEKDAY_SCHEDULE_RESPONSE)
  //!< #zclClosures_DoorLockSetYearDayScheduleRsp_t
  zclClosures_DoorLockSetYearDayScheduleRsp_t        pfnDoorLockSetYearDayScheduleRsp;          //!< (COMMAND_DOOR_LOCK_SET_YEAR_DAY_SCHEDULE_RESPONSE)
  //!< #zclClosures_DoorLockGetYearDayScheduleRsp_t
  zclClosures_DoorLockGetYearDayScheduleRsp_t        pfnDoorLockGetYearDayScheduleRsp;          //!< (COMMAND_DOOR_LOCK_GET_YEAR_DAY_SCHEDULE_RESPONSE)
  //!< #zclClosures_DoorLockClearYearDayScheduleRsp_t
  zclClosures_DoorLockClearYearDayScheduleRsp_t      pfnDoorLockClearYearDayScheduleRsp;        //!< (COMMAND_DOOR_LOCK_CLEAR_YEAR_DAY_SCHEDULE_RESPONSE)
  //!< #zclClosures_DoorLockSetHolidayScheduleRsp_t
  zclClosures_DoorLockSetHolidayScheduleRsp_t        pfnDoorLockSetHolidayScheduleRsp;          //!< (COMMAND_DOOR_LOCK_SET_HOLIDAY_SCHEDULE_RESPONSE)
  //!< #zclClosures_DoorLockGetHolidayScheduleRsp_t
  zclClosures_DoorLockGetHolidayScheduleRsp_t        pfnDoorLockGetHolidayScheduleRsp;          //!< (COMMAND_DOOR_LOCK_GET_HOLIDAY_SCHEDULE_RESPONSE)
  //!< #zclClosures_DoorLockClearHolidayScheduleRsp_t
  zclClosures_DoorLockClearHolidayScheduleRsp_t      pfnDoorLockClearHolidayScheduleRsp;        //!< (COMMAND_DOOR_LOCK_CLEAR_HOLIDAY_SCHEDULE_RESPONSE)
  //!< #zclClosures_DoorLockSetUserTypeRsp_t
  zclClosures_DoorLockSetUserTypeRsp_t               pfnDoorLockSetUserTypeRsp;                 //!< (COMMAND_DOOR_LOCK_SET_USER_TYPE_RESPONSE)
  //!< #zclClosures_DoorLockGetUserTypeRsp_t
  zclClosures_DoorLockGetUserTypeRsp_t               pfnDoorLockGetUserTypeRsp;                 //!< (COMMAND_DOOR_LOCK_GET_USER_TYPE_RESPONSE)
  //!< #zclClosures_DoorLockSetRFIDCodeRsp_t
  zclClosures_DoorLockSetRFIDCodeRsp_t               pfnDoorLockSetRFIDCodeRsp;                 //!< (COMMAND_DOOR_LOCK_SET_RFID_CODE_RESPONSE)
  //!< #zclClosures_DoorLockGetRFIDCodeRsp_t
  zclClosures_DoorLockGetRFIDCodeRsp_t               pfnDoorLockGetRFIDCodeRsp;                 //!< (COMMAND_DOOR_LOCK_GET_RFID_CODE_RESPONSE)
  //!< #zclClosures_DoorLockClearRFIDCodeRsp_t
  zclClosures_DoorLockClearRFIDCodeRsp_t             pfnDoorLockClearRFIDCodeRsp;               //!< (COMMAND_DOOR_LOCK_CLEAR_RFID_CODE_RESPONSE)
  //!< #zclClosures_DoorLockClearAllRFIDCodesRsp_t
  zclClosures_DoorLockClearAllRFIDCodesRsp_t         pfnDoorLockClearAllRFIDCodesRsp;           //!< (COMMAND_DOOR_LOCK_CLEAR_ALL_RFID_CODES_RESPONSE)
  //!< #zclClosures_DoorLockOperationEventNotification_t
  zclClosures_DoorLockOperationEventNotification_t   pfnDoorLockOperationEventNotification;     //!< (COMMAND_DOOR_LOCK_OPERATING_EVENT_NOTIFICATION)
  //!< #zclClosures_DoorLockProgrammingEventNotification_t
  zclClosures_DoorLockProgrammingEventNotification_t pfnDoorLockProgrammingEventNotification;   //!< (COMMAND_DOOR_LOCK_PROGRAMMING_EVENT_NOTIFICATION)
} zclClosures_DoorLockAppCallbacks_t;
#endif  // ZCL_DOORLOCK

#ifdef ZCL_WINDOWCOVERING
/**
 * @brief Register Callbacks Window Covering Cluster table entry - enter function pointers for callbacks that
 * the application would like to receive
 */
typedef struct
{
  //!< #zclClosures_WindowCoveringSimple_t
  zclClosures_WindowCoveringSimple_t          pfnWindowCoveringUpOpen;                //!< (COMMAND_WINDOW_COVERING_UP_OR_OPEN)
  //!< #zclClosures_WindowCoveringSimple_t
  zclClosures_WindowCoveringSimple_t          pfnWindowCoveringDownClose;             //!< (COMMAND_WINDOW_COVERING_DOWN_OR_CLOSE)
  //!< #zclClosures_WindowCoveringSimple_t
  zclClosures_WindowCoveringSimple_t          pfnWindowCoveringStop;                  //!< (COMMAND_WINDOW_COVERING_STOP)
  //!< #zclClosures_WindowCoveringGotoValue_t
  zclClosures_WindowCoveringGotoValue_t       pfnWindowCoveringGotoLiftValue;         //!< (COMMAND_WINDOW_COVERING_GO_TO_LIFT_VALUE)
  //!< #zclClosures_WindowCoveringGotoPercentage_t
  zclClosures_WindowCoveringGotoPercentage_t  pfnWindowCoveringGotoLiftPercentage;    //!< (COMMAND_WINDOW_COVERING_GO_TO_LIFT_PERCENTAGE)
  //!< #zclClosures_WindowCoveringGotoValue_t
  zclClosures_WindowCoveringGotoValue_t       pfnWindowCoveringGotoTiltValue;         //!< (COMMAND_WINDOW_COVERING_GO_TO_TILT_VALUE)
  //!< #zclClosures_WindowCoveringGotoPercentage_t
  zclClosures_WindowCoveringGotoPercentage_t  pfnWindowCoveringGotoTiltPercentage;    //!< (COMMAND_WINDOW_COVERING_GO_TO_TILT_PERCENTAGE)
} zclClosures_WindowCoveringAppCallbacks_t;
#endif // ZCL_WINDOWCOVERING
/** @} End ZCL_CLOSURE_TYPEDEFS */

/*********************************************************************
 * VARIABLES
 */


/*********************************************************************
 * FUNCTIONS
 */
/**
 * @defgroup ZCL_CLOSURE_FUNCTIONS ZCL Closure Functions
 * @{
 * @brief This group defines the functions for Closure devices
 */
#ifdef ZCL_DOORLOCK
/*!
 * @param   endpoint - application's endpoint
 * @param   callbacks - pointer to the callback record.
 *
 * @return  ZMemError if not able to allocate
 */
extern ZStatus_t zclClosures_RegisterDoorLockCmdCallbacks( uint8_t endpoint, zclClosures_DoorLockAppCallbacks_t *callbacks );

/*
 * The following functions are used in low-level routines.
 * See Function Macros for app-level send functions
 */

/*!
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   cmd - COMMAND_DOOR_LOCK_LOCK_DOOR, COMMAND_DOOR_LOCK_UNLOCK_DOOR, COMMAND_DOOR_LOCK_TOGGLE
 * @param   pPayload:
 *           aPinRfidCode - PIN/RFID code in ZCL Octet String Format
 * @param   disableDefaultRsp - decides default response is necessary or not
 * @param   seqNum - sequence number of the command packet
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclClosures_SendDoorLockRequest( uint8_t srcEP, afAddrType_t *dstAddr, uint8_t cmd,
                                                  zclDoorLock_t *pPayload,
                                                  uint8_t disableDefaultRsp, uint8_t seqNum );

/*!
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pPayload:
 *           timeout - The timeout in seconds
 *           aPinRfidCode - PIN/RFID code in ZCL Octet String Format
 * @param   disableDefaultRsp - decides default response is necessary or not
 * @param   seqNum - sequence number of the command packet
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclClosures_SendDoorLockUnlockTimeoutRequest( uint8_t srcEP, afAddrType_t *dstAddr,
                                                               zclDoorLockUnlockTimeout_t *pPayload,
                                                               uint8_t disableDefaultRsp, uint8_t seqNum );

/*!
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   logIndex - Log number between 1 - [max log attribute]
 * @param   disableDefaultRsp - decides default response is necessary or not
 * @param   seqNum - sequence number of the command packet
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclClosures_SendDoorLockGetLogRecordRequest( uint8_t srcEP, afAddrType_t *dstAddr,
                                                              uint16_t logIndex, uint8_t disableDefaultRsp, uint8_t seqNum );

/*!
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pPayload:
 *           userID - User ID is between 0 - [# PINs User supported attribute]
 *           userStatus - Used to indicate what the status is for a specific User ID
 *           userType - Used to indicate what the type is for a specific User ID
 *           pPIN - A ZigBee string indicating the PIN code used to create the event on the door lock
 * @param   disableDefaultRsp - decides default response is necessary or not
 * @param   seqNum - sequence number of the command packet
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclClosures_SendDoorLockSetPINCodeRequest( uint8_t srcEP, afAddrType_t *dstAddr,
                                                            zclDoorLockSetPINCode_t *pPayload,
                                                            uint8_t disableDefaultRsp, uint8_t seqNum );

/*!
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   cmd - COMMAND_DOOR_LOCK_GET_PIN_CODE, COMMAND_DOOR_LOCK_CLEAR_PIN_CODE,
 *                COMMAND_DOOR_LOCK_GET_USER_STATUS, COMMAND_DOOR_LOCK_GET_USER_TYPE,
 *                COMMAND_DOOR_LOCK_GET_RFID_CODE, COMMAND_DOOR_LOCK_CLEAR_RFID_CODE
 * @param   userID - User ID is between 0 - [# PINs User supported attribute]
 * @param   disableDefaultRsp - decides default response is necessary or not
 * @param   seqNum - sequence number of the command packet
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclClosures_SendDoorLockUserIDRequest( uint8_t srcEP, afAddrType_t *dstAddr, uint8_t cmd,
                                                        uint16_t userID, uint8_t disableDefaultRsp, uint8_t seqNum );

/*!
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   cmd - COMMAND_DOOR_LOCK_CLEAR_ALL_PIN_CODES, COMMAND_DOOR_LOCK_CLEAR_ALL_RFID_CODES
 * @param   disableDefaultRsp - decides default response is necessary or not
 * @param   seqNum - sequence number of the command packet
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclClosures_SendDoorLockClearAllCodesRequest( uint8_t srcEP, afAddrType_t *dstAddr, uint8_t cmd,
                                                               uint8_t disableDefaultRsp, uint8_t seqNum );

/*!
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   userID - User ID is between 0 - [# PINs User supported attribute]
 * @param   userStatus - Used to indicate what the status is for a specific User ID
 * @param   disableDefaultRsp - decides default response is necessary or not
 * @param   seqNum - sequence number of the command packet
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclClosures_SendDoorLockSetUserStatusRequest( uint8_t srcEP, afAddrType_t *dstAddr,
                                                               uint16_t userID, uint8_t userStatus,
                                                               uint8_t disableDefaultRsp, uint8_t seqNum );

/*!
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   scheduleID - The Schedule ID # is between 0 - [# Schedule IDs per user attribute]
 * @param   userID - User ID is between 0 - [# PINs User supported attribute]
 * @param   daysMask - Bitmask of the effective days in the order XSFTWTMS
 * @param   startHour - The start hour of the Week Day Schedule: 0-23
 * @param   startMinute - The start minute of the Week Day Schedule: 0-59
 * @param   endHour - The end hour of the Week Day Schedule: 0-23
 * @param   endMinute - The end minute of the Week Day Schedule: 0-59
 * @param   disableDefaultRsp - decides default response is necessary or not
 * @param   seqNum - sequence number of the command packet
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclClosures_SendDoorLockSetWeekDayScheduleRequest( uint8_t srcEP, afAddrType_t *dstAddr,
                                                                    uint8_t scheduleID, uint16_t userID,
                                                                    uint8_t daysMask, uint8_t startHour,
                                                                    uint8_t startMinute, uint8_t endHour, uint8_t endMinute,
                                                                    uint8_t disableDefaultRsp, uint8_t seqNum );

/*!
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   cmd - COMMAND_DOOR_LOCK_GET_WEEKDAY_SCHEDULE, COMMAND_DOOR_LOCK_CLEAR_WEEKDAY_SCHEDULE,
 *                COMMAND_DOOR_LOCK_GET_YEAR_DAY_SCHEDULE, COMMAND_DOOR_LOCK_CLEAR_YEAR_DAY_SCHEDULE
 * @param   scheduleID - The Schedule ID # is between 0 - [# Schedule IDs per user attribute]
 * @param   userID - User ID is between 0 - [# PINs User supported attribute]
 * @param   disableDefaultRsp - decides default response is necessary or not
 * @param   seqNum - sequence number of the command packet
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclClosures_SendDoorLockScheduleRequest( uint8_t srcEP, afAddrType_t *dstAddr, uint8_t cmd,
                                                          uint8_t scheduleID, uint16_t userID,
                                                          uint8_t disableDefaultRsp, uint8_t seqNum );

/*!
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   scheduleID - The Schedule ID # is between 0 - [# Schedule IDs per user attribute]
 * @param   userID - User ID is between 0 - [# PINs User supported attribute]
 * @param   zigBeeLocalStartTime - Start time of the Year Day Schedule representing by ZigBeeLocalTime
 * @param   zigBeeLocalEndTime - End time of the Year Day Schedule representing by ZigBeeLocalTime
 * @param   disableDefaultRsp - decides default response is necessary or not
 * @param   seqNum - sequence number of the command packet
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclClosures_SendDoorLockSetYearDayScheduleRequest( uint8_t srcEP, afAddrType_t *dstAddr,
                                                                    uint8_t scheduleID, uint16_t userID,
                                                                    uint32_t zigBeeLocalStartTime, uint32_t zigBeeLocalEndTime,
                                                                    uint8_t disableDefaultRsp, uint8_t seqNum );

/*!
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   holidayScheduleID - A unique ID for given Holiday Schedule (0 to 254)
 * @param   zigBeeLocalStartTime - Start time of the Year Day Schedule representing by ZigBeeLocalTime
 * @param   zigBeeLocalEndTime - End time of the Year Day Schedule representing by ZigBeeLocalTime
 * @param   operatingModeDuringHoliday - A valid enumeration value as listed in operating mode attribute
 * @param   disableDefaultRsp - decides default response is necessary or not
 * @param   seqNum - sequence number of the command packet
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclClosures_SendDoorLockSetHolidayScheduleRequest( uint8_t srcEP, afAddrType_t *dstAddr,
                                                                    uint8_t holidayScheduleID, uint32_t zigBeeLocalStartTime,
                                                                    uint32_t zigBeeLocalEndTime, uint8_t operatingModeDuringHoliday,
                                                                    uint8_t disableDefaultRsp, uint8_t seqNum );

/*!
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   cmd - COMMAND_DOOR_LOCK_GET_HOLIDAY_SCHEDULE, COMMAND_DOOR_LOCK_CLEAR_HOLIDAY_SCHEDULE
 * @param   holidayScheduleID - A unique ID for given Holiday Schedule (0 to 254)
 * @param   disableDefaultRsp - decides default response is necessary or not
 * @param   seqNum - sequence number of the command packet
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclClosures_SendDoorLockHolidayScheduleRequest( uint8_t srcEP, afAddrType_t *dstAddr, uint8_t cmd,
                                                                 uint8_t holidayScheduleID,
                                                                 uint8_t disableDefaultRsp, uint8_t seqNum );

/*!
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   userID - User ID is between 0 - [# PINs User supported attribute]
 * @param   userType - Used to indicate what the type is for a specific User ID
 * @param   disableDefaultRsp - decides default response is necessary or not
 * @param   seqNum - sequence number of the command packet
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclClosures_SendDoorLockSetUserTypeRequest( uint8_t srcEP, afAddrType_t *dstAddr,
                                                             uint16_t userID, uint8_t userType,
                                                             uint8_t disableDefaultRsp, uint8_t seqNum );

/*!
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pPayload:
 *           userID - User ID is between 0 - [# PINs User supported attribute]
 *           userStatus - Used to indicate what the status is for a specific User ID
 *           userType - Used to indicate what the type is for a specific User ID
 *           aRfidCode - A ZigBee string indicating the RFID code used to create the event
 * @param   disableDefaultRsp - decides default response is necessary or not
 * @param   seqNum - sequence number of the command packet
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclClosures_SendDoorLockSetRFIDCodeRequest( uint8_t srcEP, afAddrType_t *dstAddr,
                                                             zclDoorLockSetRFIDCode_t *pPayload,
                                                             uint8_t disableDefaultRsp, uint8_t seqNum );

/*!
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   cmd - COMMAND_DOOR_LOCK_LOCK_DOOR_RESPONSE, COMMAND_DOOR_LOCK_UNLOCK_DOOR_RESPONSE
 *                COMMAND_DOOR_LOCK_TOGGLE_RSP, COMMAND_DOOR_LOCK_UNLOCK_WITH_TIMEOUT_RESPONSE,
 *                COMMAND_DOOR_LOCK_SET_PIN_CODE_RESPONSE, COMMAND_DOOR_LOCK_CLEAR_PIN_CODE_RESPONSE,
 *                COMMAND_DOOR_LOCK_CLEAR_ALL_PIN_CODES_RESPONSE, COMMAND_DOOR_LOCK_SET_USER_STATUS_RESPONSE,
 *                COMMAND_DOOR_LOCK_SET_WEEKDAY_SCHEDULE_RESPONSE, COMMAND_DOOR_LOCK_CLEAR_WEEKDAY_SCHEDULE_RESPONSE,
 *                COMMAND_DOOR_LOCK_SET_YEAR_DAY_SCHEDULE_RESPONSE, COMMAND_DOOR_LOCK_CLEAR_YEAR_DAY_SCHEDULE_RESPONSE,
 *                COMMAND_DOOR_LOCK_SET_HOLIDAY_SCHEDULE_RESPONSE, COMMAND_DOOR_LOCK_CLEAR_HOLIDAY_SCHEDULE_RESPONSE,
 *                COMMAND_DOOR_LOCK_SET_USER_TYPE_RESPONSE, COMMAND_DOOR_LOCK_SET_RFID_CODE_RESPONSE,
 *                COMMAND_DOOR_LOCK_CLEAR_RFID_CODE_RESPONSE, COMMAND_DOOR_LOCK_CLEAR_ALL_RFID_CODES_RESPONSE
 * @param   status - Returns the state due to the requesting command
 * @param   disableDefaultRsp - decides default response is necessary or not
 * @param   seqNum - sequence number of the command packet
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclClosures_SendDoorLockStatusResponse( uint8_t srcEP, afAddrType_t *dstAddr, uint8_t cmd,
                                                         uint8_t status, uint8_t disableDefaultRsp, uint8_t seqNum );

/*!
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pPayload:
 *           logEntryID - The index into the log table where this log entry is stored
 *           timestamp - A ZigBeeLocalTime used to timestamp all events and alarms on the door lock
 *           eventType - Indicates the type of event that took place on the door lock
 *           source - A source value of available sources
 *           eventIDAlarmCode - A one byte value indicating the type of event that took place on the door lock
 *           userID - User ID is between 0 - [# PINs User supported attribute]
 *           aPIN - A ZigBee string indicating the PIN code used to create the event on the door lock
 * @param   disableDefaultRsp - decides default response is necessary or not
 * @param   seqNum - sequence number of the command packet
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclClosures_SendDoorLockGetLogRecordResponse( uint8_t srcEP, afAddrType_t *dstAddr,
                                                               zclDoorLockGetLogRecordRsp_t *pPayload,
                                                               uint8_t disableDefaultRsp, uint8_t seqNum );

/*!
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pPayload:
 *           userID - User ID is between 0 - [# PINs User supported attribute]
 *           userStatus - Used to indicate what the status is for a specific User ID
 *           userType - Used to indicate what the type is for a specific User ID
 *           aCode - Returned PIN number
 * @param   disableDefaultRsp - decides default response is necessary or not
 * @param   seqNum - sequence number of the command packet
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclClosures_SendDoorLockGetPINCodeResponse( uint8_t srcEP, afAddrType_t *dstAddr,
                                                             zclDoorLockGetPINCodeRsp_t *pPayload,
                                                             uint8_t disableDefaultRsp, uint8_t seqNum );

/*!
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   userID - User ID is between 0 - [# PINs User supported attribute]
 * @param   userStatus - Used to indicate what the status is for a specific User ID
 * @param   disableDefaultRsp - decides default response is necessary or not
 * @param   seqNum - sequence number of the command packet
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclClosures_SendDoorLockGetUserStatusResponse( uint8_t srcEP, afAddrType_t *dstAddr,
                                                                uint16_t userID, uint8_t userStatus,
                                                                uint8_t disableDefaultRsp, uint8_t seqNum );

/*!
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd:
 *               scheduleID - The Schedule ID # is between 0 - [# Schedule IDs per user attribute]
 *               userID - User ID is between 0 - [# PINs User supported attribute]
 *               status - Returns the state due to the requesting command
 *               daysMask - Bitmask of the effective days in the order XSFTWTMS
 *               startHour - The start hour of the Week Day Schedule: 0-23
 *               startMinute - The start minute of the Week Day Schedule: 0-59
 *               endHour - The end hour of the Week Day Schedule: 0-23
 *               endMinute - The end minute of the Week Day Schedule: 0-59
 * @param   disableDefaultRsp - decides default response is necessary or not
 * @param   seqNum - sequence number of the command packet
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclClosures_SendDoorLockGetWeekDayScheduleResponse( uint8_t srcEP, afAddrType_t *dstAddr,
                                                                     zclDoorLockGetWeekDayScheduleRsp_t *pCmd,
                                                                     uint8_t disableDefaultRsp, uint8_t seqNum );

/*!
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd:
 *             scheduleID - The Schedule ID # is between 0 - [# Schedule IDs per user attribute]
 *             userID - User ID is between 0 - [# PINs User supported attribute]
 *             status - Returns the state due to the requesting command
 *             zigBeeLocalStartTime - Start time of the Year Day Schedule representing by ZigBeeLocalTime
 *             zigBeeLocalEndTime - End time of the Year Day Schedule representing by ZigBeeLocalTime
 * @param   disableDefaultRsp - decides default response is necessary or not
 * @param   seqNum - sequence number of the command packet
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclClosures_SendDoorLockGetYearDayScheduleResponse( uint8_t srcEP, afAddrType_t *dstAddr,
                                                                     zclDoorLockGetYearDayScheduleRsp_t *pCmd,
                                                                     uint8_t disableDefaultRsp, uint8_t seqNum );

/*!
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd:
 *             holidayScheduleID - A unique ID for given Holiday Schedule (0 to 254)
 *             status - Returns the state due to the requesting command
 *             zigBeeLocalStartTime - Start time of the Year Day Schedule representing by ZigBeeLocalTime
 *             zigBeeLocalEndTime - End time of the Year Day Schedule representing by ZigBeeLocalTime
 *             operatingModeDuringHoliday - A valid enumeration value as listed in operating mode attribute
 * @param   disableDefaultRsp - decides default response is necessary or not
 * @param   seqNum - sequence number of the command packet
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclClosures_SendDoorLockGetHolidayScheduleResponse( uint8_t srcEP, afAddrType_t *dstAddr,
                                                                     zclDoorLockGetHolidayScheduleRsp_t *pCmd,
                                                                     uint8_t disableDefaultRsp, uint8_t seqNum );

/*!
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   userID - User ID is between 0 - [# PINs User supported attribute]
 * @param   userType - Used to indicate what the type is for a specific User ID
 * @param   disableDefaultRsp - decides default response is necessary or not
 * @param   seqNum - sequence number of the command packet
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclClosures_SendDoorLockGetUserTypeResponse( uint8_t srcEP, afAddrType_t *dstAddr,
                                                              uint16_t userID, uint8_t userType,
                                                              uint8_t disableDefaultRsp, uint8_t seqNum );

/*!
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pPayload:
 *           userID - User ID is between 0 - [# PINs User supported attribute]
 *           userStatus - Used to indicate what the status is for a specific User ID
 *           userType - Used to indicate what the type is for a specific User ID
 *           aRfidCode - A ZigBee string indicating the RFID code used to create the event
 * @param   disableDefaultRsp - decides default response is necessary or not
 * @param   seqNum - sequence number of the command packet
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclClosures_SendDoorLockGetRFIDCodeResponse( uint8_t srcEP, afAddrType_t *dstAddr,
                                                              zclDoorLockGetRFIDCodeRsp_t *pPayload,
                                                              uint8_t disableDefaultRsp, uint8_t seqNum );

/*!
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pPayload:
 *           operationEventSource - Indicates where the event was triggered from
 *           operationEventCode - (Optional) a notification whenever there is a significant operation event on the lock
 *           userID - User ID is between 0 - [# PINs User supported attribute]
 *           pin - The PIN that is associated with the User ID who performed the event
 *           zigBeeLocalTime - Indicates when the event is triggered
 *           aData - Used to pass data associated with a particular event
 * @param   disableDefaultRsp - decides default response is necessary or not
 * @param   seqNum - sequence number of the command packet
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclClosures_SendDoorLockOperationEventNotification( uint8_t srcEP, afAddrType_t *dstAddr,
                                                                     zclDoorLockOperationEventNotification_t *pPayload,
                                                                     uint8_t disableDefaultRsp, uint8_t seqNum );

/*!
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pPayload:
 *           programEventSource - Indicates where the event was triggered from
 *           programEventCode - (Optional) a notification whenever there is a significant programming event on the lock
 *           userID - User ID is between 0 - [# PINs User supported attribute]
 *           pin - The PIN that is associated with the User ID who performed the event
 *           userType - Used to indicate what the type is for a specific User ID
 *           userStatus - Used to indicate what the status is for a specific User ID
 *           zigBeeLocalTime - Indicates when the event is triggered
 *           dataLen - Manufacture specific, describes length of aData
 *           aData - Used to pass data associated with a particular event
 * @param   disableDefaultRsp - decides default response is necessary or not
 * @param   seqNum - sequence number of the command packet
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclClosures_SendDoorLockProgrammingEventNotification( uint8_t srcEP, afAddrType_t *dstAddr,
                                                                       zclDoorLockProgrammingEventNotification_t *pPayload,
                                                                       uint8_t disableDefaultRsp, uint8_t seqNum );
#endif // ZCL_DOORLOCK

#ifdef ZCL_WINDOWCOVERING
/*!
 * @param   endpoint - application's endpoint
 * @param   callbacks - pointer to the callback record.
 *
 * @return  ZMemError if not able to allocate
 */
extern ZStatus_t zclClosures_RegisterWindowCoveringCmdCallbacks( uint8_t endpoint, zclClosures_WindowCoveringAppCallbacks_t *callbacks );

/*
 * The following functions are used in low-level routines.
 * See Function Macros for app-level send functions
 */

/*!
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   cmd - Command ID
 * @param   disableDefaultRsp - decides default response is necessary or not
 * @param   seqNum - sequence number of the command packet
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclClosures_WindowCoveringSimpleReq( uint8_t srcEP, afAddrType_t *dstAddr,
                                                      uint8_t cmd, uint8_t disableDefaultRsp, uint8_t seqNum );

/*!
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   cmd - Command ID for COMMAND_WINDOW_COVERING_GO_TO_LIFT_VALUE
 * @param   value - where you want the window covering to go
 * @param   disableDefaultRsp - decides default response is necessary or not
 * @param   seqNum - sequence number of the command packet
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclClosures_WindowCoveringSendGoToValueReq( uint8_t srcEP, afAddrType_t *dstAddr,
                                                             uint8_t cmd, uint16_t value,
                                                             uint8_t disableDefaultRsp, uint8_t seqNum );

/*!
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   cmd - Command ID e.g. COMMAND_WINDOW_COVERING_GO_TO_LIFT_PERCENTAGE
 * @param   percentageValue - what percentage you want the window to cover
 * @param   disableDefaultRsp - decides default response is necessary or not
 * @param   seqNum - sequence number of the command packet
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclClosures_WindowCoveringSendGoToPercentageReq( uint8_t srcEP, afAddrType_t *dstAddr,
                                                                  uint8_t cmd, uint8_t percentageValue,
                                                                  uint8_t disableDefaultRsp, uint8_t seqNum );
#endif // ZCL_WINDOWCOVERING
/*********************************************************************
 * FUNCTION MACROS
 */

/*!
 *  @brief Send a Door Lock Lock Command
 *
 *  Use like:
 *      ZStatus_t zclClosures_SendDoorLockLockDoor( uint8_t srcEP, afAddrType_t *dstAddr, zclDoorLock_t *pPayload, uint8_t disableDefaultRsp, uint8_t seqNum );
 */
#define zclClosures_SendDoorLockLockDoor(a, b, c, d, e) zclClosures_SendDoorLockRequest( (a), (b), COMMAND_DOOR_LOCK_LOCK_DOOR, (c), (d), (e) )

/*!
 *  @brief Send a Door Lock Unlock Command
 *
 *  Use like:
 *      ZStatus_t zclClosures_SendDoorLockUnlockDoor( uint8_t srcEP, afAddrType_t *dstAddr, zclDoorLock_t *pPayload, uint8_t disableDefaultRsp, uint8_t seqNum );
 */
#define zclClosures_SendDoorLockUnlockDoor(a, b, c, d, e) zclClosures_SendDoorLockRequest( (a), (b), COMMAND_DOOR_LOCK_UNLOCK_DOOR, (c), (d), (e) )

/*!
 *  @brief Send a Door Lock Toggle Command
 *
 *  Use like:
 *      ZStatus_t zclClosures_SendDoorLockToggleDoor( uint8_t srcEP, afAddrType_t *dstAddr, zclDoorLock_t *pPayload, uint8_t disableDefaultRsp, uint8_t seqNum );
 */
#define zclClosures_SendDoorLockToggleDoor(a, b, c, d, e) zclClosures_SendDoorLockRequest( (a), (b), COMMAND_DOOR_LOCK_TOGGLE, (c), (d), (e) )

/*!
 *  @brief Send a Get PIN Code Command
 *
 *  Use like:
 *      ZStatus_t zclClosures_SendDoorLockGetPINCode( uint8_t srcEP, afAddrType_t *dstAddr, uint16_t userID, uint8_t disableDefaultRsp, uint8_t seqNum );
 */
#define zclClosures_SendDoorLockGetPINCode(a, b, c, d, e) zclClosures_SendDoorLockUserIDRequest( (a), (b), COMMAND_DOOR_LOCK_GET_PIN_CODE, (c), (d), (e) )

/*!
 *  @brief Send a Clear PIN Code Command
 *
 *  Use like:
 *      ZStatus_t zclClosures_SendDoorLockClearPINCode( uint8_t srcEP, afAddrType_t *dstAddr, uint16_t userID, uint8_t disableDefaultRsp, uint8_t seqNum );
 */
#define zclClosures_SendDoorLockClearPINCode(a, b, c, d, e) zclClosures_SendDoorLockUserIDRequest( (a), (b), COMMAND_DOOR_LOCK_CLEAR_PIN_CODE, (c), (d), (e) )

/*!
 *  @brief Send a Clear All PIN Codes Command
 *
 *  Use like:
 *      ZStatus_t zclClosures_SendDoorLockClearAllPINCodes( uint8_t srcEP, afAddrType_t *dstAddr, uint8_t disableDefaultRsp, uint8_t seqNum );
 */
#define zclClosures_SendDoorLockClearAllPINCodes(a, b, c, d) zclClosures_SendDoorLockClearAllCodesRequest( (a), (b), COMMAND_DOOR_LOCK_CLEAR_ALL_PIN_CODES, (c), (d) )

/*!
 *  @brief Send a Get User Status Command
 *
 *  Use like:
 *      ZStatus_t zclClosures_SendDoorLockGetUserStatus( uint8_t srcEP, afAddrType_t *dstAddr, uint16_t userID, uint8_t disableDefaultRsp, uint8_t seqNum )
 */
#define zclClosures_SendDoorLockGetUserStatus(a, b, c, d, e) zclClosures_SendDoorLockUserIDRequest( (a), (b), COMMAND_DOOR_LOCK_GET_USER_STATUS, (c), (d), (e) )

/*!
 *  @brief Send a Get Week Day Schedule Command
 *
 *  Use like:
 *      ZStatus_t zclClosures_SendDoorLockGetWeekDaySchedule( uint8_t srcEP, afAddrType_t *dstAddr, uint8_t scheduleID, uint16_t userID, uint8_t disableDefaultRsp, uint8_t seqNum);
 */
#define zclClosures_SendDoorLockGetWeekDaySchedule(a, b, c, d, e, f) zclClosures_SendDoorLockScheduleRequest( (a), (b), COMMAND_DOOR_LOCK_GET_WEEKDAY_SCHEDULE, (c), (d), (e), (f) )

/*!
 *  @brief Send a Clear Week Day Schedule Command
 *
 *  Use like:
 *      ZStatus_t zclClosures_SendDoorLockClearWeekDaySchedule( uint8_t srcEP, afAddrType_t *dstAddr, uint8_t scheduleID, uint16_t userID, uint8_t disableDefaultRsp, uint8_t seqNum );
 */
/// This is a redefinition of zclClosures_SendDoorLockScheduleRequest with the parameter cmd set to @ref COMMAND_DOOR_LOCK_CLEAR_WEEKDAY_SCHEDULE
#define zclClosures_SendDoorLockClearWeekDaySchedule(a, b, c, d, e, f) zclClosures_SendDoorLockScheduleRequest( (a), (b), COMMAND_DOOR_LOCK_CLEAR_WEEKDAY_SCHEDULE, (c), (d), (e), (f) )

/*!
 *  @brief Send a Get Year Day Schedule Command
 *
 *  Use like:
 *      ZStatus_t zclClosures_SendDoorLockGetYearDaySchedule( uint8_t srcEP, afAddrType_t *dstAddr, uint8_t scheduleID, uint16_t userID, uint8_t disableDefaultRsp, uint8_t seqNum );
 */
#define zclClosures_SendDoorLockGetYearDaySchedule(a, b, c, d, e, f) zclClosures_SendDoorLockScheduleRequest( (a), (b), COMMAND_DOOR_LOCK_GET_YEAR_DAY_SCHEDULE, (c), (d), (e), (f) )

/*!
 *  @brief Send a Clear Year Day Schedule Command
 *
 *  Use like:
 *      ZStatus_t zclClosures_SendDoorLockClearYearDaySchedule( uint8_t srcEP, afAddrType_t *dstAddr, uint8_t scheduleID, uint16_t userID, uint8_t disableDefaultRsp, uint8_t seqNum );
 */
#define zclClosures_SendDoorLockClearYearDaySchedule(a, b, c, d, e, f) zclClosures_SendDoorLockScheduleRequest( (a), (b), COMMAND_DOOR_LOCK_CLEAR_YEAR_DAY_SCHEDULE, (c), (d), (e), (f) )

/*!
 *  @brief Send a Get Holiday Schedule Command
 *
 *  Use like:
 *      ZStatus_t zclClosures_SendDoorLockGetHolidaySchedule( uint8_t srcEP, afAddrType_t *dstAddr, uint8_t holidayScheduleID, uint8_t disableDefaultRsp, uint8_t seqNum );
 */
#define zclClosures_SendDoorLockGetHolidaySchedule(a, b, c, d, e) zclClosures_SendDoorLockHolidayScheduleRequest( (a), (b), COMMAND_DOOR_LOCK_GET_HOLIDAY_SCHEDULE, (c), (d), (e) )

/*!
 *  @brief Send a Clear Holiday Schedule Command
 *
 *  Use like:
 *      ZStatus_t zclClosures_SendDoorLockClearHolidaySchedule( uint8_t srcEP, afAddrType_t *dstAddr, uint8_t holidayScheduleID, uint8_t disableDefaultRsp, uint8_t seqNum );
 */
#define zclClosures_SendDoorLockClearHolidaySchedule(a, b, c, d, e) zclClosures_SendDoorLockHolidayScheduleRequest( (a), (b), COMMAND_DOOR_LOCK_CLEAR_HOLIDAY_SCHEDULE, (c), (d), (e) )

/*!
 *  @brief Send a Get User Type Command
 *
 *  Use like:
 *      ZStatus_t zclClosures_SendDoorLockGetUserType( uint8_t srcEP, afAddrType_t *dstAddr, uint16_t userID, uint8_t disableDefaultRsp, uint8_t seqNum );
 */
#define zclClosures_SendDoorLockGetUserType(a, b, c, d, e) zclClosures_SendDoorLockUserIDRequest( (a), (b), COMMAND_DOOR_LOCK_GET_USER_TYPE, (c), (d), (e) )

/*!
 *  @brief Send a Get RFID Code Command
 *
 *  Use like:
 *      ZStatus_t zclClosures_SendDoorLockGetRFIDCode( uint8_t srcEP, afAddrType_t *dstAddr, uint16_t userID, uint8_t disableDefaultRsp, uint8_t seqNum );
 */
#define zclClosures_SendDoorLockGetRFIDCode(a, b, c, d, e) zclClosures_SendDoorLockUserIDRequest( (a), (b), COMMAND_DOOR_LOCK_GET_RFID_CODE, (c), (d), (e) )

/*!
 *  @brief Send a Clear RFID Code Command
 *
 *  Use like:
 *      ZStatus_t zclClosures_SendDoorLockClearRFIDCode( uint8_t srcEP, afAddrType_t *dstAddr, uint16_t userID, uint8_t disableDefaultRsp, uint8_t seqNum );
 */
#define zclClosures_SendDoorLockClearRFIDCode(a, b, c, d, e) zclClosures_SendDoorLockUserIDRequest( (a), (b), COMMAND_DOOR_LOCK_CLEAR_RFID_CODE, (c), (d), (e) )

/*!
 *  @brief Send a Clear All RFID Codes Command
 *
 *  Use like:
 *      ZStatus_t zclClosures_SendDoorLockClearAllRFIDCodes( uint8_t srcEP, afAddrType_t *dstAddr, uint8_t disableDefaultRsp, uint8_t seqNum )
 */
/// This is a redefinition of zclClosures_SendDoorLockClearAllCodesRequest with the parameter cmd set to @ref COMMAND_DOOR_LOCK_CLEAR_ALL_RFID_CODES
#define zclClosures_SendDoorLockClearAllRFIDCodes(a, b, c, d) zclClosures_SendDoorLockClearAllCodesRequest( (a), (b), COMMAND_DOOR_LOCK_CLEAR_ALL_RFID_CODES, (c), (d) )

/*!
 *  @brief Send a Door Lock Lock Response
 *
 *  Use like:
 *      ZStatus_t zclClosures_SendDoorLockLockDoorRsp( uint8_t srcEP, afAddrType_t *dstAddr, uint8_t status, uint8_t disableDefaultRsp, uint8_t seqNum )
 */
#define zclClosures_SendDoorLockLockDoorRsp(a, b, c, d, e) zclClosures_SendDoorLockStatusResponse( (a), (b), COMMAND_DOOR_LOCK_LOCK_DOOR_RESPONSE, (c), (d), (e) )

/*!
 *  @brief Send a Door Lock Unlock Response
 *
 *  Use like:
 *      ZStatus_t zclClosures_SendDoorLockUnlockDoorRsp( uint8_t srcEP, afAddrType_t *dstAddr, uint8_t status, uint8_t disableDefaultRsp, uint8_t seqNum )
 */
#define zclClosures_SendDoorLockUnlockDoorRsp(a, b, c, d, e) zclClosures_SendDoorLockStatusResponse( (a), (b), COMMAND_DOOR_LOCK_UNLOCK_DOOR_RESPONSE, (c), (d), (e) )

/*!
 *  @brief Send a Toggle Response
 *
 *  Use like:
 *      ZStatus_t zclClosures_SendDoorLockToggleDoorRsp( uint8_t srcEP, afAddrType_t *dstAddr, uint8_t status, uint8_t disableDefaultRsp, uint8_t seqNum );
 */
#define zclClosures_SendDoorLockToggleDoorRsp(a, b, c, d, e) zclClosures_SendDoorLockStatusResponse( (a), (b), COMMAND_DOOR_LOCK_TOGGLE_RESPONSE, (c), (d), (e) )

/*!
 *  @brief Send a Unlock With Timeout Response
 *
 *  Use like:
 *      ZStatus_t zclClosures_SendDoorLockUnlockWithTimeoutRsp( uint8_t srcEP, afAddrType_t *dstAddr, uint8_t status, uint8_t disableDefaultRsp, uint8_t seqNum );
 */
#define zclClosures_SendDoorLockUnlockWithTimeoutRsp(a, b, c, d, e) zclClosures_SendDoorLockStatusResponse( (a), (b), COMMAND_DOOR_LOCK_UNLOCK_WITH_TIMEOUT_RESPONSE, (c), (d), (e) )

/*!
 *  @brief Send a Set PIN Code Response
 *
 *  Use like:
 *      ZStatus_t zclClosures_SendDoorLockSetPINCodeRsp( uint8_t srcEP, afAddrType_t *dstAddr, uint8_t status, uint8_t disableDefaultRsp, uint8_t seqNum );
 */
#define zclClosures_SendDoorLockSetPINCodeRsp(a, b, c, d, e) zclClosures_SendDoorLockStatusResponse( (a), (b), COMMAND_DOOR_LOCK_SET_PIN_CODE_RESPONSE, (c), (d), (e) )

/*!
 *  @brief Send a Clear PIN Code Response
 *
 *  Use like:
 *      ZStatus_t zclClosures_SendDoorLockClearPINCodeRsp( uint8_t srcEP, afAddrType_t *dstAddr, uint8_t status, uint8_t disableDefaultRsp, uint8_t seqNum );
 */
#define zclClosures_SendDoorLockClearPINCodeRsp(a, b, c, d, e) zclClosures_SendDoorLockStatusResponse( (a), (b), COMMAND_DOOR_LOCK_CLEAR_PIN_CODE_RESPONSE, (c), (d), (e) )

/*!
 *  @brief Send a Clear All PIN Codes Response
 *
 *  Use like:
 *      ZStatus_t zclClosures_SendDoorLockClearAllPINCodesRsp( uint8_t srcEP, afAddrType_t *dstAddr, uint8_t status, uint8_t disableDefaultRsp, uint8_t seqNum );
 */
#define zclClosures_SendDoorLockClearAllPINCodesRsp(a, b, c, d, e) zclClosures_SendDoorLockStatusResponse( (a), (b), COMMAND_DOOR_LOCK_CLEAR_ALL_PIN_CODES_RESPONSE, (c), (d), (e) )

/*!
 *  @brief Send a Set User Status Response
 *
 *  Use like:
 *      ZStatus_t zclClosures_SendDoorLockSetUserStatusRsp( uint8_t srcEP, afAddrType_t *dstAddr, uint8_t status, uint8_t disableDefaultRsp, uint8_t seqNum );
 */
#define zclClosures_SendDoorLockSetUserStatusRsp(a, b, c, d, e) zclClosures_SendDoorLockStatusResponse( (a), (b), COMMAND_DOOR_LOCK_SET_USER_STATUS_RESPONSE, (c), (d), (e) )

/*!
 *  @brief Send a Set Week Day Schedule Response
 *
 *  Use like:
 *      ZStatus_t zclClosures_SendDoorLockSetWeekDayScheduleRsp( uint8_t srcEP, afAddrType_t *dstAddr, uint8_t status, uint8_t disableDefaultRsp, uint8_t seqNum );
 */
#define zclClosures_SendDoorLockSetWeekDayScheduleRsp(a, b, c, d, e) zclClosures_SendDoorLockStatusResponse( (a), (b), COMMAND_DOOR_LOCK_SET_WEEKDAY_SCHEDULE_RESPONSE, (c), (d), (e) )

/*!
 *  @brief Send a Clear Week Day Schedule Response
 *
 *  Use like:
 *      ZStatus_t zclClosures_SendDoorLockClearWeekDayScheduleRsp( uint8_t srcEP, afAddrType_t *dstAddr, uint8_t status, uint8_t disableDefaultRsp, uint8_t seqNum );
 */
#define zclClosures_SendDoorLockClearWeekDayScheduleRsp(a, b, c, d, e) zclClosures_SendDoorLockStatusResponse( (a), (b), COMMAND_DOOR_LOCK_CLEAR_WEEKDAY_SCHEDULE_RESPONSE, (c), (d), (e) )

/*!
 *  @brief Send a Set Year Day Schedule Response
 *
 *  Use like:
 *      ZStatus_t zclClosures_SendDoorLockSetYearDayScheduleRsp( uint8_t srcEP, afAddrType_t *dstAddr, uint8_t status, uint8_t disableDefaultRsp, uint8_t seqNum );
 */
#define zclClosures_SendDoorLockSetYearDayScheduleRsp(a, b, c, d, e) zclClosures_SendDoorLockStatusResponse( (a), (b), COMMAND_DOOR_LOCK_SET_YEAR_DAY_SCHEDULE_RESPONSE, (c), (d), (e) )

/*!
 *  @brief Send a Clear Year Day Schedule Response
 *
 *  Use like:
 *      ZStatus_t zclClosures_SendDoorLockClearYearDayScheduleRsp( uint8_t srcEP, afAddrType_t *dstAddr, uint8_t status, uint8_t disableDefaultRsp, uint8_t seqNum );
 */
#define zclClosures_SendDoorLockClearYearDayScheduleRsp(a, b, c, d, e) zclClosures_SendDoorLockStatusResponse( (a), (b), COMMAND_DOOR_LOCK_CLEAR_YEAR_DAY_SCHEDULE_RESPONSE, (c), (d), (e) )

/*!
 *  @brief Send a Set Holiday Schedule Response
 *
 *  Use like:
 *      ZStatus_t zclClosures_SendDoorLockSetHolidayScheduleRsp( uint8_t srcEP, afAddrType_t *dstAddr, uint8_t status, uint8_t disableDefaultRsp, uint8_t seqNum );
 */
#define zclClosures_SendDoorLockSetHolidayScheduleRsp(a, b, c, d, e) zclClosures_SendDoorLockStatusResponse( (a), (b), COMMAND_DOOR_LOCK_SET_HOLIDAY_SCHEDULE_RESPONSE, (c), (d), (e) )

/*!
 *  @brief Send a Clear Holiday Schedule Response
 *
 *  Use like:
 *      ZStatus_t zclClosures_SendDoorLockClearHolidayScheduleRsp( uint8_t srcEP, afAddrType_t *dstAddr, uint8_t status, uint8_t disableDefaultRsp, uint8_t seqNum );
 */
#define zclClosures_SendDoorLockClearHolidayScheduleRsp(a, b, c, d, e) zclClosures_SendDoorLockStatusResponse( (a), (b), COMMAND_DOOR_LOCK_CLEAR_HOLIDAY_SCHEDULE_RESPONSE, (c), (d), (e) )

/*!
 *  @brief Send a Set User Type Response
 *
 *  Use like:
 *      ZStatus_t zclClosures_SendDoorLockSetUserTypeRsp( uint8_t srcEP, afAddrType_t *dstAddr, uint8_t status, uint8_t disableDefaultRsp, uint8_t seqNum );
 */
#define zclClosures_SendDoorLockSetUserTypeRsp(a, b, c, d, e) zclClosures_SendDoorLockStatusResponse( (a), (b), COMMAND_DOOR_LOCK_SET_USER_TYPE_RESPONSE, (c), (d), (e) )

/*!
 *  @brief Send a Set RFID Code Response
 *
 *  Use like:
 *      ZStatus_t zclClosures_SendDoorLockSetRFIDCodeRsp( uint8_t srcEP, afAddrType_t *dstAddr, uint8_t status, uint8_t disableDefaultRsp, uint8_t seqNum );
 */
#define zclClosures_SendDoorLockSetRFIDCodeRsp(a, b, c, d, e) zclClosures_SendDoorLockStatusResponse( (a), (b), COMMAND_DOOR_LOCK_SET_RFID_CODE_RESPONSE, (c), (d), (e) )

/*!
 *  @brief Send a Clear RFID Code Response
 *
 *  Use like:
 *      ZStatus_t zclClosures_SendDoorLockClearRFIDCodeRsp( uint8_t srcEP, afAddrType_t *dstAddr, uint8_t status, uint8_t disableDefaultRsp, uint8_t seqNum );
 */
#define zclClosures_SendDoorLockClearRFIDCodeRsp(a, b, c, d, e) zclClosures_SendDoorLockStatusResponse( (a), (b), COMMAND_DOOR_LOCK_CLEAR_RFID_CODE_RESPONSE, (c), (d), (e) )

/*!
 *  @brief Send a Clear All RFID Codes Response
 *
 *  Use like:
 *      ZStatus_t zclClosures_SendDoorLockClearAllRFIDCodesRsp( uint8_t srcEP, afAddrType_t *dstAddr, uint8_t status, uint8_t disableDefaultRsp, uint8_t seqNum );
 */
#define zclClosures_SendDoorLockClearAllRFIDCodesRsp(a, b, c, d, e) zclClosures_SendDoorLockStatusResponse( (a), (b), COMMAND_DOOR_LOCK_CLEAR_ALL_RFID_CODES_RESPONSE, (c), (d), (e) )

/*!
 *  @brief Send a Up/Open Request Command
 *
 *  Use like:
 *      ZStatus_t zclClosures_SendUpOpen( uint8_t srcEP, afAddrType_t *dstAddr, uint8_t disableDefaultRsp, uint8_t seqNum )
 */
#define zclClosures_SendUpOpen(a, b, c, d) zclClosures_WindowCoveringSimpleReq( (a), (b), COMMAND_WINDOW_COVERING_UP_OR_OPEN, (c), (d) )

/*!
 *  @brief Send a Down/Close Request Command
 *
 *  Use like:
 *      ZStatus_t zclClosures_SendDownClose( uint8_t srcEP, afAddrType_t *dstAddr, uint8_t disableDefaultRsp, uint8_t seqNum )
 */
#define zclClosures_SendDownClose(a, b, c, d) zclClosures_WindowCoveringSimpleReq( (a), (b), COMMAND_WINDOW_COVERING_DOWN_OR_CLOSE, (c), (d) )

/*!
 *  @brief Send a Stop Request Command
 *
 *  Use like:
 *      ZStatus_t zclClosures_SendStop( uint8_t srcEP, afAddrType_t *dstAddr, uint8_t disableDefaultRsp, uint8_t seqNum )
 */
#define zclClosures_SendStop(a, b, c, d) zclClosures_WindowCoveringSimpleReq( (a), (b), COMMAND_WINDOW_COVERING_STOP, (c), (d) )

/*!
 *  @brief Send a GoToLiftValue Request Command
 *
 *  Use like:
 *      ZStatus_t zclClosures_SendGoToLiftValue( uint8_t srcEP, afAddrType_t *dstAddr, uint16_t liftValue, uint8_t disableDefaultRsp, uint8_t seqNum )
 */
#define zclClosures_SendGoToLiftValue(a, b, c, d, e) zclClosures_WindowCoveringSendGoToValueReq( (a), (b), COMMAND_WINDOW_COVERING_GO_TO_LIFT_VALUE, (c), (d), (e))

/*!
 *  @brief Send a GoToLiftPercentage Request Command
 *
 *  Use like:
 *      ZStatus_t zclClosures_SendGoToLiftPercentage( uint8_t srcEP, afAddrType_t *dstAddr, uint8_t percentageLiftValue, uint8_t disableDefaultRsp, uint8_t seqNum )
 */
#define zclClosures_SendGoToLiftPercentage(a, b, c, d, e) zclClosures_WindowCoveringSendGoToPercentageReq( (a), (b), COMMAND_WINDOW_COVERING_GO_TO_LIFT_PERCENTAGE, (c), (d), (e))

/*!
 *  @brief Send a GoToTiltValue Request Command
 *
 *  Use like:
 *      ZStatus_t zclClosures_SendGoToTiltValue( uint8_t srcEP, afAddrType_t *dstAddr, uint16_t tiltValue, uint8_t disableDefaultRsp, uint8_t seqNum )
 */
#define zclClosures_SendGoToTiltValue(a, b, c, d, e) zclClosures_WindowCoveringSendGoToValueReq( (a), (b), COMMAND_WINDOW_COVERING_GO_TO_TILT_VALUE, (c), (d), (e))

/*!
 *  @brief Send a GoToTiltPercentage Request Command
 *
 *  Use like:
 *      ZStatus_t zclClosures_SendGoToTiltPercentage( uint8_t srcEP, afAddrType_t *dstAddr, uint8_t percentageTiltValue, uint8_t disableDefaultRsp, uint8_t seqNum )
 */
#define zclClosures_SendGoToTiltPercentage(a, b, c, d, e) zclClosures_WindowCoveringSendGoToPercentageReq( (a), (b), COMMAND_WINDOW_COVERING_GO_TO_TILT_PERCENTAGE, (c), (d), (e))
/** @} End ZCL_CLOSURE_FUNCTIONS */

#ifdef __cplusplus
}
#endif

#endif /* ZCL_CLOSURES_H */

/** @} End ZCL_CLOSURES */

