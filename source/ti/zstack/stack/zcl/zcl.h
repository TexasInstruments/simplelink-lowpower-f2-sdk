/**************************************************************************************************
  Filename:       zcl.h
  Revised:        $Date: 2014-11-25 09:19:55 -0800 (Tue, 25 Nov 2014) $
  Revision:       $Revision: 41240 $

  Description:    This file contains the Zigbee Cluster Library Foundation definitions.


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
 *  @defgroup ZCL ZCL
 *  @brief This module implements the Zigbee Cluster Library
 *  @{
 *
 *  @file  zcl.h
 *  @brief      Top level ZCL header
 */


#ifndef ZCL_H
#define ZCL_H

#if defined ( BDB_REPORTING )
#ifndef ZCL_REPORTING_DEVICE
  #define ZCL_REPORTING_DEVICE
#endif
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#if !defined ( ZCL_STANDALONE ) || defined ( ZCL_STANDALONE_OSAL )
  #include "osal.h"
  #include "osal_nv.h"
  #include "osal_tasks.h"
#endif

#include "af.h"
#include "aps_groups.h"
#include "ti_zstack_config.h"

/*********************************************************************
 * CONSTANTS
 */

/**
 * @defgroup ZCL_CLUSTER_IDS ZCL Cluster IDs
 * @{
 */

/**
 * @defgroup ZCL_GENERAL_CLUSTER_IDS ZCL General Clusters
 * @{
 * @brief This group defines the IDs for the General Clusters
 * defined in the ZCL v7 specification
 */
/// Basic Cluster. Attributes for determining basic information about a device,
/// setting user device information such as description of location, and
/// enabling a device.
#define ZCL_CLUSTER_ID_GENERAL_BASIC                             0x0000
/// Power Configuration Cluster. Attributes for determining more detailed
/// information about a device's power source(s), and for configuring
/// under/over voltage alarms.
#define ZCL_CLUSTER_ID_GENERAL_POWER_CFG                         0x0001
/// Device Temperature Configuration Cluster. Attributes for determining
/// information about a device's internal temperature, and for configuring
/// under/over temperature alarms.
#define ZCL_CLUSTER_ID_GENERAL_DEVICE_TEMP_CONFIG                0x0002
/// Identify Cluster. Attributes and commands for putting a device into
/// Identification mode (e.g., flashing a light).
#define ZCL_CLUSTER_ID_GENERAL_IDENTIFY                          0x0003
/// Groups Cluster. Attributes and commands for allocating a device to one or
/// more of a number of groups of devices, where each group is addressable by a group address
#define ZCL_CLUSTER_ID_GENERAL_GROUPS                            0x0004
/// Scenes Cluster. Attributes and commands for setting up and recalling a number
/// of scenes for a device. Each scene corresponds to a set of stored values of
/// specified device attributes.
#define ZCL_CLUSTER_ID_GENERAL_SCENES                            0x0005
/// On/Off Cluster. Attributes and commands for switching devices between 'On'
/// and 'Off' states.
#define ZCL_CLUSTER_ID_GENERAL_ON_OFF                            0x0006
/// On/Off Switch Configuration Cluster. Attributes and commands for configuring
/// on/off switching devices.
#define ZCL_CLUSTER_ID_GENERAL_ON_OFF_SWITCH_CONFIGURATION       0x0007
/// Level Control Cluster. Attributes and commands for controlling a characteristic
/// of devices that can be set to a level between fully 'On' and fully 'Off'.
#define ZCL_CLUSTER_ID_GENERAL_LEVEL_CONTROL                     0x0008
/// Alarms Cluster. Attributes and commands for sending alarm notifications and
/// configuring alarm functionality.
#define ZCL_CLUSTER_ID_GENERAL_ALARMS                            0x0009
/// Time Cluster. Attributes and commands that provide an interface to a
/// real-time clock.
#define ZCL_CLUSTER_ID_GENERAL_TIME                              0x000A
/// RSSI Location Cluster. Attributes and commands for exchanging location information
/// and channel parameters among devices, and (optionally) reporting data to a
/// centralized device that collects data from devices in the network and calculates
/// their positions from the set of collected data.
#define ZCL_CLUSTER_ID_GENERAL_LOCATION                          0x000B
/// Diagnostics Cluster. Attributes and commands that provide an interface to
/// diagnostics of the stack.
#define ZCL_CLUSTER_ID_HA_DIAGNOSTIC                             0x0B05
/// Poll Control Cluster. Attributes and commands that provide an interface to
/// control the polling of sleeping end device.
#define ZCL_CLUSTER_ID_GENERAL_POLL_CONTROL                      0x0020
/// Power Profile Cluster. Attributes and commands that provide an interface to
/// the power profile of a device.
#define ZCL_CLUSTER_ID_GENERAL_POWER_PROFILE                     0x001A
/// Meter Identification Cluster. Attributes and commands that provide an interface
/// to meter identification.
#define ZCL_CLUSTER_ID_HA_METER_IDENTIFICATION                   0x0B01
/// Analog Input (basic) Cluster. An interface for reading the value of an analog
/// measurement and accessing various characteristics of that measurement.
#define ZCL_CLUSTER_ID_GENERAL_ANALOG_INPUT_BASIC                0x000C
/// Analog Output (basic) Cluster. An interface for setting the value of an analog
/// output (typically to the environment) and accessing various characteristics
/// of that value.
#define ZCL_CLUSTER_ID_GENERAL_ANALOG_OUTPUT_BASIC               0x000D
/// Analog Value (basic) Cluster. An interface for setting an analog value,
/// typically used as a control system parameter, and accessing various
/// characteristics of that value.
#define ZCL_CLUSTER_ID_GENERAL_ANALOG_VALUE_BASIC                0x000E
/// Binary Input (basic) Cluster. An interface for reading the value of a binary
/// measurement and accessing various characteristics of that measurement.
#define ZCL_CLUSTER_ID_GENERAL_BINARY_INPUT_BASIC                0x000F
/// Binary Output (basic) Cluster. An interface for setting the value of a binary
/// output (typically to the environment) and accessing various characteristics
/// of that value.
#define ZCL_CLUSTER_ID_GENERAL_BINARY_OUTPUT_BASIC               0x0010
/// Binary Value (basic) Cluster. An interface for setting a binary value,
/// typically used as a control system parameter, and accessing various
/// characteristics of that value.
#define ZCL_CLUSTER_ID_GENERAL_BINARY_VALUE_BASIC                0x0011
/// Multistate Input (basic) Cluster. An interface for reading the value of a
/// multistate measurement and accessing various characteristics of that measurement.
#define ZCL_CLUSTER_ID_GENERAL_MULTISTATE_INPUT_BASIC            0x0012
/// Multistate Output (basic) Cluster. An interface for setting the value of a
/// multistate output (typically to the environment) and accessing various
/// characteristics of that value.
#define ZCL_CLUSTER_ID_GENERAL_MULTISTATE_OUTPUT_BASIC           0x0013
/// Multistate Value (basic) Cluster. An interface for setting a multistate value,
/// typically used as a control system parameter, and accessing various characteristics
/// of that value.
#define ZCL_CLUSTER_ID_GENERAL_MULTISTATE_VALUE_BASIC            0x0014
/** @} End ZCL_GENERAL_CLUSTER_IDS */

/**
 * @defgroup ZCL_MS_CLUSTER_IDS ZCL Measurement & Sensing Clusters
 * @{
 * @brief This group defines the IDs for the Measurement & Sensing Clusters
 * defined in the ZCL v7 specification
 */
/// Illuminance Measurement Cluster. Attributes and commands for configuring the
/// measurement of illuminance, and reporting illuminance measurements.
#define ZCL_CLUSTER_ID_MS_ILLUMINANCE_MEASUREMENT            0x0400
/// Illuminance Level Sensing Cluster. Attributes and commands for configuring
/// the sensing of illuminance levels, and reporting whether illuminance is
/// above, below, or on target.
#define ZCL_CLUSTER_ID_MS_ILLUMINANCE_LEVEL_SENSING_CONFIG   0x0401
/// Temperature Measurement Cluster. Attributes and commands for configuring the
/// measurement of temperature, and reporting temperature measurements.
#define ZCL_CLUSTER_ID_MS_TEMPERATURE_MEASUREMENT            0x0402
/// Pressure Measurement Cluster. Attributes and commands for configuring the
/// measurement of pressure, and reporting pressure measurements.
#define ZCL_CLUSTER_ID_MS_PRESSURE_MEASUREMENT               0x0403
/// Flow Measurement Cluster. Attributes and commands for configuring the measurement
/// of flow, and reporting flow rates.
#define ZCL_CLUSTER_ID_MS_FLOW_MEASUREMENT                   0x0404
/// Relative Humidity Measurement Cluster. Attributes and commands for configuring
/// the measurement of relative humidity, and reporting relative humidity measurements.
#define ZCL_CLUSTER_ID_MS_RELATIVE_HUMIDITY                  0x0405
/// Occupancy Sensing Cluster. Attributes and commands for configuring occupancy
/// sensing, and reporting occupancy status.
#define ZCL_CLUSTER_ID_MS_OCCUPANCY_SENSING                  0x0406
/// Electrical Measurement Cluster. Attributes and commands for measuring electrical usage.
#define ZCL_CLUSTER_ID_MS_ELECTRICAL_MEASUREMENT             0x0B04
/** @} End ZCL_MS_CLUSTER_IDS */

/**
 * @defgroup ZCL_LIGHTING_CLUSTER_IDS ZCL Lighting Clusters
 * @{
 * @brief This group defines the IDs for the Lighting Clusters
 * defined in the ZCL v7 specification
 */
/// Color Control Cluster. Attributes and commands for changing the color of a light.
#define ZCL_CLUSTER_ID_LIGHTING_COLOR_CONTROL                0x0300
/// Ballast Configuration Cluster. Attributes and commands for configuring a lighting ballast.
#define ZCL_CLUSTER_ID_LIGHTING_BALLAST_CONFIG               0x0301
/** @} End ZCL_LIGHTING_CLUSTER_IDS */

/**
 * @defgroup ZCL_HVAC_CLUSTER_IDS ZCL HVAC Clusters
 * @{
 * @brief This group defines the IDs for the HVAC Clusters
 * defined in the ZCL v7 specification
 */
/// Pump Configuration and Control Cluster. Attributes and commands for the
/// setup and control of pump devices.
#define ZCL_CLUSTER_ID_HVAC_PUMP_CONFIG_CONTROL              0x0200
/// Thermostat Cluster. Attributes and commands for functionality of a thermostat.
#define ZCL_CLUSTER_ID_HVAC_THERMOSTAT                       0x0201
/// Fan Control Cluster. Attributes and commands to control the speed of a fan
/// as part of a heating / cooling system.
#define ZCL_CLUSTER_ID_HVAC_FAN_CONTROL                      0x0202
/// Dehumidification Control Cluster. Attributes and commands for dehumidification
/// functionality devices.
#define ZCL_CLUSTER_ID_HVAC_DIHUMIDIFICATION_CONTROL         0x0203
/// Thermostat User Interface Configuration Cluster. Attributes and commands for
/// configuration of the user interface for a thermostat, or a thermostat controller
/// device, that supports a keypad and LCD screen.
#define ZCL_CLUSTER_ID_HVAC_USER_INTERFACE_CONFIG            0x0204
/** @} End ZCL_HVAC_CLUSTER_IDS */

/**
 * @defgroup ZCL_CLOSURES_CLUSTER_IDS ZCL Closures Clusters
 * @{
 * @brief This group defines the IDs for the HVAC Clusters
 * defined in the ZCL v7 specification
 */
/// Shade Configuration Cluster. Attributes and commands for reading information
/// about a shade, and configuring its open and closed limits.
#define ZCL_CLUSTER_ID_CLOSURES_SHADE_CONFIG                 0x0100
/// Door Lock Cluster. Attributes and commands for generic way to secure a door.
#define ZCL_CLUSTER_ID_CLOSURES_DOOR_LOCK                    0x0101
/// Window Covering Cluster. Attributes and commands for controlling and adjusting
/// automatic window coverings such as drapery motors, automatic shades, and blinds.
#define ZCL_CLUSTER_ID_CLOSURES_WINDOW_COVERING              0x0102
/** @} End ZCL_CLOSURES_CLUSTER_IDS */

/**
 * @defgroup ZCL_SS_CLUSTER_IDS ZCL Security & Safety Clusters
 * @{
 * @brief This group defines the IDs for the Security & Safety Clusters
 * defined in the ZCL v7 specification
 */
/// IAS Zone Cluster. Attributes and commands for functionality of an IAS
/// security zone device.
#define ZCL_CLUSTER_ID_SS_IAS_ZONE                           0x0500
/// IAS ACE Cluster. Attributes and commands for any Ancillary Control Equipment
/// of the IAS system.
#define ZCL_CLUSTER_ID_SS_IAS_ACE                            0x0501
/// IAS WD Cluster. Attributes and commands for functionality of any Warning Device
/// equipment of the IAS system.
#define ZCL_CLUSTER_ID_SS_IAS_WD                             0x0502
/** @} End ZCL_SS_CLUSTER_IDS */

/**
 * @defgroup ZCL_PI_CLUSTER_IDS ZCL Protocol Interfaces Clusters
 * @{
 * @brief This group defines the IDs for the Protocol Interfaces Clusters
 * defined in the ZCL v7 specification
 */
/// Partition Cluster. The commands and attributes for enabling partitioning of
/// a large frame between devices.
#define ZCL_CLUSTER_ID_GENERAL_PARTITION                         0x0016
/// Generic tunnel Cluster. The minimum common commands and attributes required
/// to tunnel any protocol.
#define ZCL_CLUSTER_ID_PI_GENERIC_TUNNEL                     0x0600
/// BACnet protocol tunnel Cluster. Commands and attributes required to tunnel
/// the BACnet protocol.
#define ZCL_CLUSTER_ID_PI_BACNET_PROTOCOL_TUNNEL             0x0601
/// Analog input (BACnet regular) Cluster. An interface for accessing a number
/// of commonly used BACnet based attributes of an analog measurement.
#define ZCL_CLUSTER_ID_PI_ANALOG_INPUT_BACNET_REG            0x0602
/// Analog input (BACnet extended). An interface for accessing a number of
/// BACnet based attributes of an analog measurement.
#define ZCL_CLUSTER_ID_PI_ANALOG_INPUT_BACNET_EXT            0x0603
/// Analog output (BACnet regular). An interface for accessing a number of
/// commonly used BACnet based attributes of an analog output.
#define ZCL_CLUSTER_ID_PI_ANALOG_OUTPUT_BACNET_REG           0x0604
/// Analog output (BACnet extended) Cluster. An interface for accessing a number
/// of BACnet based attributes of an analog output.
#define ZCL_CLUSTER_ID_PI_ANALOG_OUTPUT_BACNET_EXT           0x0605
/// Analog value(BACnet regular) Cluster. An interface for accessing a number of
/// commonly used BACnet based attributes of an analog value, typically used as
/// a control system parameter.
#define ZCL_CLUSTER_ID_PI_ANALOG_VALUE_BACNET_REG            0x0606
/// Analog value(BACnet extended) Cluster. An interface for accessing a number
/// of BACnet based attributes of an analog value, typically used as a control
/// system parameter.
#define ZCL_CLUSTER_ID_PI_ANALOG_VALUE_BACNET_EXT            0x0607
/// Binary input (BACnet regular) Cluster. An interface for accessing a number
/// of commonly used BACnet based attributes of a binary measurement.
#define ZCL_CLUSTER_ID_PI_BINARY_INPUT_BACNET_REG            0x0608
/// Binary input (BACnet extended) Cluster. An interface for accessing a number
/// of BACnet based attributes of a binary measurement.
#define ZCL_CLUSTER_ID_PI_BINARY_INPUT_BACNET_EXT            0x0609
/// Binary output (BACnet regular) Cluster. An interface for accessing a number
/// of commonly used BACnet based attributes of a binary output.
#define ZCL_CLUSTER_ID_PI_BINARY_OUTPUT_BACNET_REG           0x060A
/// Binary output (BACnet extended) Cluster. An interface for accessing a number
/// of BACnet based attributes of a binary output.
#define ZCL_CLUSTER_ID_PI_BINARY_OUTPUT_BACNET_EXT           0x060B
/// Binary value (BACnet regular) Cluster. An interface for accessing a number
/// of commonly used BACnet based attributes of a binary value, typically used
/// as a control system parameter.
#define ZCL_CLUSTER_ID_PI_BINARY_VALUE_BACNET_REG            0x060C
/// Binary value (BACnet extended) Cluster. An interface for accessing a number
/// of BACnet based attributes of a binary value, typically used as a control
/// system parameter.
#define ZCL_CLUSTER_ID_PI_BINARY_VALUE_BACNET_EXT            0x060D
/// Multistate input (BACnet regular) Cluster. An interface for accessing a
/// number of commonly used BACnet based attributes of a multistate measurement.
#define ZCL_CLUSTER_ID_PI_MULTISTATE_INPUT_BACNET_REG        0x060E
/// Multistate input (BACnet extended) Cluster. An interface for accessing a
/// number of BACnet based attributes of a multistate measurement.
#define ZCL_CLUSTER_ID_PI_MULTISTATE_INPUT_BACNET_EXT        0x060F
/// Multistate output (BACnet regular) Cluster. An interface for accessing a number
/// of commonly used BACnet based attributes of a multistate output.
#define ZCL_CLUSTER_ID_PI_MULTISTATE_OUTPUT_BACNET_REG       0x0610
/// Multistate output (BACnet extended) Cluster. An interface for accessing a
/// number of BACnet based attributes of a multistate output.
#define ZCL_CLUSTER_ID_PI_MULTISTATE_OUTPUT_BACNET_EXT       0x0611
/// Multistate value (BACnet regular) Cluster. An interface for accessing a number
/// of commonly used BACnet based attributes of a multistate value, typically used
/// as a control system parameter.
#define ZCL_CLUSTER_ID_PI_MULTISTATE_VALUE_BACNET_REG        0x0612
/// Multistate value (BACnet extended) Cluster. An interface for accessing a number
/// of BACnet based attributes of a multistate value, typically used as a control
/// system parameter.
#define ZCL_CLUSTER_ID_PI_MULTISTATE_VALUE_BACNET_EXT        0x0613
/// 11073 Protocol Tunnel Cluster. Interface for 11073 Protocol Tunnel used in
/// health care applications
#define ZCL_CLUSTER_ID_PI_11073_PROTOCOL_TUNNEL              0x0614
/// ISO7816 Tunnel Cluster. Commands and attributes for mobile office solutions
/// using devices.
#define ZCL_CLUSTER_ID_PI_ISO7818_PROTOCOL_TUNNEL            0x0615
/** @} End ZCL_PI_CLUSTER_IDS */

/**
 * @defgroup ZCL_SE_CLUSTER_IDS ZCL Smart Energy Clusters
 * @{
 * @brief This group defines the IDs for the Smart Energy Clusters
 * defined in the ZCL v7 specification
 */
/// Price Cluster. Commands and attributes for reporting price.
#define ZCL_CLUSTER_ID_SE_PRICE                              0x0700
/// Demand Response and Load Control Cluster. Commands and attributes for
/// providing demand response and load control of devices.
#define ZCL_CLUSTER_ID_SE_DRLC                               0x0701
/// Metering Cluster. Commands and attributes for reporting metering data.
#define ZCL_CLUSTER_ID_SE_METERING                           0x0702
/// Messaging Cluster. Commands and attributes for sending messages to devices.
#define ZCL_CLUSTER_ID_SE_MESSAGING                          0x0703
/// Tunneling Cluster. Commands and attributes for establishing and using a
/// tunnel between two devices.
#define ZCL_CLUSTER_ID_SE_TUNNELING                          0x0704
/// Tunnel Cluster. Commands and attributes for mobile office solutions using devices.
#define ZCL_CLUSTER_ID_SE_PREPAYMENT                         0x0705
/// ISO7816 Tunnel Cluster. Commands and attributes for mobile office solutions using devices.
#define ZCL_CLUSTER_ID_SE_ENERGY_MGMT                        0x0706
/// ISO7816 Tunnel Cluster. Commands and attributes for mobile office solutions using devices.
#define ZCL_CLUSTER_ID_SE_CALENDAR                           0x0707
/// ISO7816 Tunnel Cluster. Commands and attributes for mobile office solutions using devices.
#define ZCL_CLUSTER_ID_SE_DEVICE_MGMT                        0x0708
/// ISO7816 Tunnel Cluster. Commands and attributes for mobile office solutions using devices.
#define ZCL_CLUSTER_ID_SE_EVENTS                             0x0709
/// ISO7816 Tunnel Cluster. Commands and attributes for mobile office solutions using devices.
#define ZCL_CLUSTER_ID_SE_MDU_PAIRING                        0x070A
/// Key Establishment Cluster. Commands and attributes for application level
/// security establishment.
#define ZCL_CLUSTER_ID_SE_KEY_ESTABLISHMENT                  0x0800
/** @} End ZCL_SE_CLUSTER_IDS */

/**
 * @defgroup ZCL_OTA_CLUSTER_IDS ZCL Over-The-Air Upgrade Clusters
 * @{
 * @brief This group defines the IDs for the Over-The-Air Upgrade Clusters
 * defined in the ZCL v7 specification
 */
/// OTA Upgrade Cluster. Parameters and commands for upgrading image on
/// devices Over-The-Air.
#define ZCL_CLUSTER_ID_OTA                                   0x0019
/** @} End ZCL_OTA_CLUSTER_IDS */

/**
 * @defgroup ZCL_TELECOMMUNICATIONS_CLUSTER_IDS ZCL Telecommunications Clusters
 * @{
 * @brief This group defines the IDs for the Telecommunications Clusters
 * defined in the ZCL v7 specification
 */
/// Information Cluster. Commands and attributes for information delivery.
#define ZCL_CLUSTER_ID_TELECOMMUNICATIONS_INFORMATION        0x0900
/// Voice Over ZigBee Cluster. Commands and attributes for voice receiving
/// and transmitting.
#define ZCL_CLUSTER_ID_TELECOMMUNICATIONS_VOICE_OVER_ZIGBEE  0x0904
/// Chatting Cluster. Commands and attributes for sending chat messages.
#define ZCL_CLUSTER_ID_TELECOMMUNICATIONS_CHATTING           0x0905
/** @} End ZCL_TELECOMMUNICATIONS_CLUSTER_IDS */

/**
 * @defgroup ZCL_COMMISSIONING_CLUSTER_IDS ZCL Commissioning Clusters
 * @{
 * @brief This group defines the IDs for the Commissioning Clusters
 * defined in the ZCL v7 specification
 */
/// Commissioning Cluster. The commands and attributes for commissioning a
/// device onto the network.
#define ZCL_CLUSTER_ID_GENERAL_COMMISSIONING                     0x0015
/// Touchlink Cluster. The commands and attributes for Touchlink commissioning
/// a device.
#define ZCL_CLUSTER_ID_TOUCHLINK                             0x1000
/** @} End ZCL_COMMISSIONING_CLUSTER_IDS */

/**
 * @defgroup ZCL_RETAIL_CLUSTER_IDS ZCL Retail Clusters
 * @{
 * @brief This group defines the IDs for the Retail Clusters
 * defined in the ZCL v7 specification
 */
/// Retail Tunnel Cluster. Interface for manufacturer specific information
/// to be exchanged.
#define ZCL_CLUSTER_ID_PI_RETAIL_TUNNEL                      0x0617
/// Mobile Device Configuration Cluster. Interface to manage mobile
/// devices in a network.
#define ZCL_CLUSTER_ID_MOBILE_DEVICE_CONFIGURATION           0x0022
/// Neighbor Cleaning Cluster.Interface to manage mobile devices in a network.
#define ZCL_CLUSTER_ID_NEIGHBOR_CLEANING                     0x0023
/// Nearest Gateway Cluster.Interface to enable communication of nearest
/// gateway to devices.
#define ZCL_CLUSTER_ID_NEAREST_GATEWAY                       0x0024
/** @} End ZCL_RETAIL_CLUSTER_IDS */

/**
 * @defgroup ZCL_APPLICANCE_CLUSTER_IDS ZCL Appliance Clusters
 * @{
 * @brief This group defines the IDs for the Appliance Clusters
 * defined in the ZCL v7 specification
 */
/// EN50523 Appliance Control Cluster. Commands and attributes for controlling
/// household appliances.
#define ZCL_CLUSTER_ID_GENERAL_APPLIANCE_CONTROL                 0x001B
/// EN50523 Appliance Identification Cluster. Commands and attributes for
/// appliance information and device settings.
#define ZCL_CLUSTER_ID_HA_APPLIANCE_IDENTIFICATION           0x0B00
/// EN50523 Appliance Events and Alerts Cluster. Commands and attributes for
/// appliance events and alerts.
#define ZCL_CLUSTER_ID_HA_APPLIANCE_EVENTS_ALERTS            0x0B02
/// EN50523 Appliance Statistics Cluster. Commands and attributes for
/// appliance statistics.
#define ZCL_CLUSTER_ID_HA_APPLIANCE_STATISTICS               0x0B03
/** @} End ZCL_APPLICANCE_CLUSTER_IDS */

/**
 * @defgroup ZCL_GP_CLUSTER_IDS ZCL Green Power Clusters
 * @{
 * @brief This group defines the IDs for the Green Power Clusters
 * defined in the ZCL v7 specification
 */
/// Green Power Cluster. The commands and attributes for Green Power functionality.
#define ZCL_CLUSTER_ID_GREEN_POWER                           0x0021
/** @} End ZCL_GP_CLUSTER_IDS */

/*** Frame Control bit mask ***/
#define ZCL_FRAME_CONTROL_TYPE                          0x03
#define ZCL_FRAME_CONTROL_MANU_SPECIFIC                 0x04
#define ZCL_FRAME_CONTROL_DIRECTION                     0x08
#define ZCL_FRAME_CONTROL_DISABLE_DEFAULT_RSP           0x10

/*** Frame Types ***/
#define ZCL_FRAME_TYPE_PROFILE_CMD                      0x00
#define ZCL_FRAME_TYPE_SPECIFIC_CMD                     0x01

/*** Frame Client/Server Directions ***/
#define ZCL_FRAME_CLIENT_SERVER_DIR                     0x00
#define ZCL_FRAME_SERVER_CLIENT_DIR                     0x01

/*** Chipcon Manufacturer Code ***/
#define CC_MANUFACTURER_CODE                            0x1001

/**
 * @defgroup ZCL_FOUNDATION_COMMAND_IDS ZCL Foundation Command IDs
 * @{
 */
/*** Foundation Command IDs ***/
/// The Read Attributes command is generated when a device wishes to determine
/// the values of one or more attributes located on another device
#define ZCL_CMD_READ                                    0x00
/// The Read Attributes Response command is generated in response to a Read
/// Attributes or Read Attributes Structured command.
#define ZCL_CMD_READ_RSP                                0x01
/// The Write Attributes command is generated when a device wishes to change
/// the values of one or more attributes located on another device.
#define ZCL_CMD_WRITE                                   0x02
/// The Write Attributes Undivided command is generated when a device wishes
/// to change the values of one or more attributes located on another device.
#define ZCL_CMD_WRITE_UNDIVIDED                         0x03
/// The Write Attributes Response command is generated in response to a Write
/// Attributes command.
#define ZCL_CMD_WRITE_RSP                               0x04
/// The Write Attributes command is generated when a device wishes to change
/// the values of one or more attributes located on another device but response is not required.
#define ZCL_CMD_WRITE_NO_RSP                            0x05
/// The report configuration command is generated when a device wishes to
/// configure a device to automatically report the values of one or more of its
/// attributes, or to receive such reports.
#define ZCL_CMD_CONFIG_REPORT                           0x06
/// The Configure Reporting Response command is generated in response to a
/// Configure Reporting command.
#define ZCL_CMD_CONFIG_REPORT_RSP                       0x07
/// The Read Reporting Configuration command is used to read the configuration
/// details of the reporting mechanism for one or more of the attributes of a cluster.
#define ZCL_CMD_READ_REPORT_CFG                         0x08
/// The Read Reporting Configuration Response command is used to respond to a
/// Read Reporting Configuration command.
#define ZCL_CMD_READ_REPORT_CFG_RSP                     0x09
/// The Report Attributes command is used by a device to report the values of
/// one or more of its attributes to another device
#define ZCL_CMD_REPORT                                  0x0a
/// The Default Response command SHALL be generated when all 4 of these criteria are met:
/// 1. A device receives a unicast command that is not a Default Response command.
/// 2. No other command is sent in response to the received command, using the
/// same Transaction sequence number as the received command.
/// 3. The Disable Default Response bit of its Frame control field is set to 0
/// or when an error results.
#define ZCL_CMD_DEFAULT_RSP                             0x0b
/// The Discover Attributes command is generated when a remote device wishes to
/// discover the identifiers and types of the attributes on a device which are
/// supported within the cluster to which this command is directed.
#define ZCL_CMD_DISCOVER_ATTRS                          0x0c
/// The Discover Attributes Response command is generated in response to a
/// Discover Attributes command.
#define ZCL_CMD_DISCOVER_ATTRS_RSP                      0x0d
/// This command may be used to discover all commands processed (received)
/// by this cluster, including optional or manufacturer-specific commands.
#define ZCL_CMD_DISCOVER_CMDS_RECEIVED                  0x11
/// The Discover Commands Received Response command is sent in response to a
/// Discover Commands Received command, and is used to discover which commands
/// a cluster can process.
#define ZCL_CMD_DISCOVER_CMDS_RECEIVED_RSP              0x12
/// This command may be used to discover all commands which may be generated
/// (sent) by the cluster, including optional or manufacturer-specific commands.
#define ZCL_CMD_DISCOVER_CMDS_GEN                 0x13
/// The Discover Commands Generated Response command is sent in response to a
/// Discover Commands Generated command, and is used to discover which commands
/// a cluster supports.
#define ZCL_CMD_DISCOVER_CMDS_GEN_RSP             0x14
/// This command is similar to the discover attributes command, but also includes
/// a field to indicate whether the attribute is readable, writeable or reportable.
#define ZCL_CMD_DISCOVER_ATTRS_EXT                      0x15
/// This command is sent in response to a Discover Attributes Extended command,
/// and is used to determine if attributes are readable, writable or reportable.
#define ZCL_CMD_DISCOVER_ATTRS_EXT_RSP                  0x16
/** @} End ZCL_FOUNDATION_COMMAND_IDS */

#define ZCL_CMD_MAX                     ZCL_CMD_DISCOVER_ATTRS_EXT_RSP
#define ZCL_DATABUF_SEND                0x0020

// define reporting constant
#define ZCL_REPORTING_OFF     0xFFFF  // turn off reporting (maxReportInt)

// define command direction flag masks
#define CMD_DIR_SERVER_GENERATED          0x01
#define CMD_DIR_CLIENT_GENERATED          0x02
#define CMD_DIR_SERVER_RECEIVED           0x04
#define CMD_DIR_CLIENT_RECEIVED           0x08

/**
 * @defgroup ZCL_FOUNDATION_COMMAND_IDS ZCL Foundation Command IDs
 * @{
 */
/*** Data Types ***/
#define ZCL_DATATYPE_NO_DATA                            0x00
#define ZCL_DATATYPE_DATA8                              0x08
#define ZCL_DATATYPE_DATA16                             0x09
#define ZCL_DATATYPE_DATA24                             0x0a
#define ZCL_DATATYPE_DATA32                             0x0b
#define ZCL_DATATYPE_DATA40                             0x0c
#define ZCL_DATATYPE_DATA48                             0x0d
#define ZCL_DATATYPE_DATA56                             0x0e
#define ZCL_DATATYPE_DATA64                             0x0f
#define ZCL_DATATYPE_BOOLEAN                            0x10
#define ZCL_DATATYPE_BITMAP8                            0x18
#define ZCL_DATATYPE_BITMAP16                           0x19
#define ZCL_DATATYPE_BITMAP24                           0x1a
#define ZCL_DATATYPE_BITMAP32                           0x1b
#define ZCL_DATATYPE_BITMAP40                           0x1c
#define ZCL_DATATYPE_BITMAP48                           0x1d
#define ZCL_DATATYPE_BITMAP56                           0x1e
#define ZCL_DATATYPE_BITMAP64                           0x1f
#define ZCL_DATATYPE_UINT8                              0x20
#define ZCL_DATATYPE_UINT16                             0x21
#define ZCL_DATATYPE_UINT24                             0x22
#define ZCL_DATATYPE_UINT32                             0x23
#define ZCL_DATATYPE_UINT40                             0x24
#define ZCL_DATATYPE_UINT48                             0x25
#define ZCL_DATATYPE_UINT56                             0x26
#define ZCL_DATATYPE_UINT64                             0x27
#define ZCL_DATATYPE_INT8                               0x28
#define ZCL_DATATYPE_INT16                              0x29
#define ZCL_DATATYPE_INT24                              0x2a
#define ZCL_DATATYPE_INT32                              0x2b
#define ZCL_DATATYPE_INT40                              0x2c
#define ZCL_DATATYPE_INT48                              0x2d
#define ZCL_DATATYPE_INT56                              0x2e
#define ZCL_DATATYPE_INT64                              0x2f
#define ZCL_DATATYPE_ENUM8                              0x30
#define ZCL_DATATYPE_ENUM16                             0x31
#define ZCL_DATATYPE_SEMI_PREC                          0x38
#define ZCL_DATATYPE_SINGLE_PREC                        0x39
#define ZCL_DATATYPE_DOUBLE_PREC                        0x3a
#define ZCL_DATATYPE_OCTET_STR                          0x41
#define ZCL_DATATYPE_CHAR_STR                           0x42
#define ZCL_DATATYPE_LONG_OCTET_STR                     0x43
#define ZCL_DATATYPE_LONG_CHAR_STR                      0x44
#define ZCL_DATATYPE_ARRAY                              0x48
#define ZCL_DATATYPE_STRUCT                             0x4c
#define ZCL_DATATYPE_SET                                0x50
#define ZCL_DATATYPE_BAG                                0x51
#define ZCL_DATATYPE_TOD                                0xe0
#define ZCL_DATATYPE_DATE                               0xe1
#define ZCL_DATATYPE_UTC                                0xe2
#define ZCL_DATATYPE_CLUSTER_ID                         0xe8
#define ZCL_DATATYPE_ATTR_ID                            0xe9
#define ZCL_DATATYPE_BAC_OID                            0xea
#define ZCL_DATATYPE_IEEE_ADDR                          0xf0
#define ZCL_DATATYPE_128_BIT_SEC_KEY                    0xf1
#define ZCL_DATATYPE_UNKNOWN                            0xff
/** @} End ZCL_FOUNDATION_COMMAND_IDS */

/**
 * @defgroup ZCL_ERROR_CODES ZCL Error Codes
 * @{
 */
#define ZCL_STATUS_SUCCESS                              0x00
#define ZCL_STATUS_FAILURE                              0x01
// 0x02-0x7D are reserved.
#define ZCL_STATUS_NOT_AUTHORIZED                       0x7E
#define ZCL_STATUS_MALFORMED_COMMAND                    0x80
#define ZCL_STATUS_UNSUP_CLUSTER_COMMAND                0x81
#define ZCL_STATUS_UNSUP_GENERAL_COMMAND                0x82
#define ZCL_STATUS_UNSUP_MANU_CLUSTER_COMMAND           0x83
#define ZCL_STATUS_UNSUP_MANU_GENERAL_COMMAND           0x84
#define ZCL_STATUS_INVALID_FIELD                        0x85
#define ZCL_STATUS_UNSUPPORTED_ATTRIBUTE                0x86
#define ZCL_STATUS_INVALID_VALUE                        0x87
#define ZCL_STATUS_READ_ONLY                            0x88
#define ZCL_STATUS_INSUFFICIENT_SPACE                   0x89
#define ZCL_STATUS_DUPLICATE_EXISTS                     0x8a
#define ZCL_STATUS_NOT_FOUND                            0x8b
#define ZCL_STATUS_UNREPORTABLE_ATTRIBUTE               0x8c
#define ZCL_STATUS_INVALID_DATA_TYPE                    0x8d
#define ZCL_STATUS_INVALID_SELECTOR                     0x8e
#define ZCL_STATUS_WRITE_ONLY                           0x8f
#define ZCL_STATUS_INCONSISTENT_STARTUP_STATE           0x90
#define ZCL_STATUS_DEFINED_OUT_OF_BAND                  0x91
#define ZCL_STATUS_INCONSISTENT                         0x92
#define ZCL_STATUS_ACTION_DENIED                        0x93
#define ZCL_STATUS_TIMEOUT                              0x94
#define ZCL_STATUS_ABORT                                0x95
#define ZCL_STATUS_INVALID_IMAGE                        0x96
#define ZCL_STATUS_WAIT_FOR_DATA                        0x97
#define ZCL_STATUS_NO_IMAGE_AVAILABLE                   0x98
#define ZCL_STATUS_REQUIRE_MORE_IMAGE                   0x99
// 0xbd-bf are reserved.
#define ZCL_STATUS_HARDWARE_FAILURE                     0xc0
#define ZCL_STATUS_SOFTWARE_FAILURE                     0xc1
#define ZCL_STATUS_CALIBRATION_ERROR                    0xc2
// 0xc3-0xff are reserved.
#define ZCL_STATUS_CMD_HAS_RSP                          0xFF // Non-standard status (used for Default Rsp)
/** @} End ZCL_ERROR_CODES */

/**
 * @defgroup ZCL_ACCESS_CONTROL_BITS ZCL Access Control Bits
 * @{
*  @brief Access control (bitmask values)
 */
#define ACCESS_CONTROL_NONE								              0x00
#define ACCESS_CONTROL_READ                             0x01  // attribute can be read
#define ACCESS_CONTROL_WRITE                            0x02  // attribute can be written
#define ACCESS_REPORTABLE                               0x04  // indicate attribute is reportable
#define ACCESS_CONTROL_COMMAND                          0x08
#define ACCESS_CONTROL_AUTH_READ                        0x10
#define ACCESS_CONTROL_AUTH_WRITE                       0x20
#define ACCESS_GLOBAL                                   0x40  // TI unique to indicate attributes that are in both, client and server side of the cluster in the endpoint
#define ACCESS_CLIENT                                   0x80  // TI unique, indicate client side attribute
//NOTE: If no global or client access is define, then server side of the attribute is assumed
// Access Control for client
#define ACCESS_CONTROL_MASK                             0x07
// Access Control as reported OTA via DiscoveryAttributesExtended
#define ACCESS_CONTROLEXT_MASK                          0x07  // read/write/reportable bits same as above
/** @} End ZCL_ACCESS_CONTROL_BITS */

#define ZCL_ATTR_ID_MAX                                 0xFFFF

// Used by Configure Reporting Command
#define ZCL_SEND_ATTR_REPORTS                           0x00
#define ZCL_EXPECT_ATTR_REPORTS                         0x01

// Predefined Maximum String Length
#define MAX_UTF8_STRING_LEN                             50

// Used by zclReadWriteCB_t callback function
#define ZCL_OPER_LEN                                    0x00 // Get length of attribute value to be read
#define ZCL_OPER_READ                                   0x01 // Read attribute value
#define ZCL_OPER_WRITE                                  0x02 // Write new attribute value

#define ATTRID_CLUSTER_REVISION                         0xFFFD // The ClusterRevision global attribute is mandatory for all cluster instances, client and server, conforming to ZCL revision 6 (ZCL6) and later ZCL revisions.
#define ATTRID_ATTRIBUTE_REPORTING_STATUS               0xFFFE // The ClusterRevision global attribute is mandatory for all cluster instances, client and server, conforming to ZCL revision 6 (ZCL6) and later ZCL revisions.

/** @} End ZCL_CONSTANTS */

/*********************************************************************
 * MACROS
 */
#define zcl_ProfileCmd( a )         ( (a) == ZCL_FRAME_TYPE_PROFILE_CMD )
#define zcl_ClusterCmd( a )         ( (a) == ZCL_FRAME_TYPE_SPECIFIC_CMD )

#define zcl_ServerCmd( a )          ( (a) == ZCL_FRAME_CLIENT_SERVER_DIR )
#define zcl_ClientCmd( a )          ( (a) == ZCL_FRAME_SERVER_CLIENT_DIR )

#define UNICAST_MSG( msg )          ( (msg)->wasBroadcast == false && (msg)->groupId == 0 )

// Check for Cluster IDs
#define ZCL_CLUSTER_ID_GEN( id )      ( /* (id) >= ZCL_CLUSTER_ID_GENERAL_BASIC &&*/ \
                                        (id) <= ZCL_CLUSTER_ID_GENERAL_COMMISSIONING )
#define ZCL_CLUSTER_ID_CLOSURES( id ) ( (id) >= ZCL_CLUSTER_ID_CLOSURES_SHADE_CONFIG && \
                                        (id) <= ZCL_CLUSTER_ID_CLOSURES_WINDOW_COVERING )
#define ZCL_CLUSTER_ID_HVAC( id )     ( (id) >= ZCL_CLUSTER_ID_HVAC_PUMP_CONFIG_CONTROL && \
                                        (id) <= ZCL_CLUSTER_ID_HVAC_USER_INTERFACE_CONFIG )
#define ZCL_CLUSTER_ID_LIGHTING( id ) ( (id) >= ZCL_CLUSTER_ID_LIGHTING_COLOR_CONTROL && \
                                        (id) <= ZCL_CLUSTER_ID_LIGHTING_BALLAST_CONFIG )
#define ZCL_CLUSTER_ID_MS( id )       ( (id) >= ZCL_CLUSTER_ID_MS_ILLUMINANCE_MEASUREMENT && \
                                        (id) <= ZCL_CLUSTER_ID_MS_OCCUPANCY_SENSING )
#define ZCL_CLUSTER_ID_SS( id )       ( (id) >= ZCL_CLUSTER_ID_SS_IAS_ZONE && \
                                        (id) <= ZCL_CLUSTER_ID_SS_IAS_WD )
#define ZCL_CLUSTER_ID_KEY( id )      ( (id) == ZCL_CLUSTER_ID_GENERAL_KEY_ESTABLISHMENT )
#define ZCL_CLUSTER_ID_SE( id )       ( (id) >= ZCL_CLUSTER_ID_SE_PRICE && \
                                        (id) <= ZCL_CLUSTER_ID_SE_MDU_PAIRING )
#define ZCL_CLUSTER_ID_PI( id )       ( (id) >= ZCL_CLUSTER_ID_PI_GENERIC_TUNNEL && \
                                        (id) <= ZCL_CLUSTER_ID_PI_ISO7818_PROTOCOL_TUNNEL )
#define ZCL_CLUSTER_ID_PI_RETAIL_T( id )( (id) == ZCL_CLUSTER_ID_PI_RETAIL_TUNNEL )
#define ZCL_CLUSTER_ID_PI_RETAIL( id )       ( (id) >= ZCL_CLUSTER_ID_MOBILE_DEVICE_CONFIGURATION && \
                                        (id) <= ZCL_CLUSTER_ID_NEIGHBOR_CLEANING )
#define ZCL_CLUSTER_ID_LL( id )       ( (id) == ZCL_CLUSTER_ID_TOUCHLINK )
#define ZCL_CLUSTER_ID_PART( id )     ( (id) == ZCL_CLUSTER_ID_GENERAL_PARTITION )

#define ZCL_CLUSTER_ID_PC( id )       ( (id) == ZCL_CLUSTER_ID_GENERAL_POLL_CONTROL )

#define ZCL_CLUSTER_ID_EM( id )	      ( (id) == ZCL_CLUSTER_ID_MS_ELECTRICAL_MEASUREMENT )

#define ZCL_CLUSTER_ID_DIAG( id )     ( (id) == ZCL_CLUSTER_ID_HA_DIAGNOSTIC )

#define ZCL_CLUSTER_ID_MI( id )       ( (id) == ZCL_CLUSTER_ID_HA_METER_IDENTIFICATION )

#define ZCL_CLUSTER_ID_APPL_C( id )   ( (id) == ZCL_CLUSTER_ID_GENERAL_APPLIANCE_CONTROL )

#define ZCL_CLUSTER_ID_APPL_I( id )   ( (id) == ZCL_CLUSTER_ID_HA_APPLIANCE_IDENTIFICATION )

#define ZCL_CLUSTER_ID_APPL_STAT( id ) ( (id) == ZCL_CLUSTER_ID_HA_APPLIANCE_STATISTICS )

#define ZCL_CLUSTER_ID_APPL_E_A( id ) ( (id) == ZCL_CLUSTER_ID_HA_APPLIANCE_EVENTS_ALERTS )

#define ZCL_CLUSTER_ID_APPL_S( id )   ( (id) == ZCL_CLUSTER_ID_HA_APPLIANCE_STATISTICS )

#define ZCL_CLUSTER_ID_PP( id )       ( (id) == ZCL_CLUSTER_ID_GENERAL_POWER_PROFILE )

#define ZCL_CLUSTER_ID_DL( id )       ( (id) == ZCL_CLUSTER_ID_CLOSURES_DOOR_LOCK )

#define ZCL_CLUSTER_ID_TEL( id )       ( (id) >= ZCL_CLUSTER_ID_TELECOMMUNICATIONS_INFORMATION && \
                                        (id) <= ZCL_CLUSTER_ID_TELECOMMUNICATIONS_CHATTING )

#define ZCL_CLUSTER_ID_TL( id )       ( (id) == ZCL_CLUSTER_ID_TOUCHLINK )

/*********************************************************************
 * TYPEDEFS
 */
/**
 * @defgroup ZCL_TYPEDEFS ZCL Typedefs
 * @{
 */

/// @brief zcl_ProcessMessageMSG() return codes
typedef enum
{
  ZCL_PROC_SUCCESS = 0,                 //!< Message was processed
  ZCL_PROC_INVALID ,                    //!< Format or parameter was wrong
  ZCL_PROC_EP_NOT_FOUND,                //!< Endpoint descriptor not found
  ZCL_PROC_NOT_OPERATIONAL,             //!< Can't respond to this command
  ZCL_PROC_INTERPAN_FOUNDATION_CMD,     //!< INTER-PAN and Foundation Command (not allowed)
  ZCL_PROC_NOT_SECURE,                  //!< Security was required but the message is not secure
  ZCL_PROC_MANUFACTURER_SPECIFIC,       //!< Manufacturer Specific command - not handled
  ZCL_PROC_MANUFACTURER_SPECIFIC_DR,    //!< Manufacturer Specific command - not handled, but default response sent
  ZCL_PROC_NOT_HANDLED,                 //!< No default response was sent and the message was not handled
  ZCL_PROC_NOT_HANDLED_DR,              //!< default response was sent and the message was not handled
} zclProcMsgStatus_t;

// ZCL header - frame control field
/// The frame control field is 8 bits in length and contains information defining
/// the command type and other control flags.
typedef struct
{
  unsigned int type:2; //!< Frame type
  unsigned int manuSpecific:1; //!< Manufacturer specific
  unsigned int direction:1; //!< Direction
  unsigned int disableDefaultRsp:1; //!< Disable Default Response
  unsigned int reserved:3; //!< Reserved
} zclFrameControl_t;

// ZCL header
/// The ZCL frame format is composed of a ZCL header and a ZCL payload
typedef struct
{
  zclFrameControl_t fc; //<! The frame control field is 8 bits in length and contains information defining the command type and other control flags.
  uint16_t            manuCode; //<! The manufacturer code field is 16 bits in length and specifies the assigned manufacturer code for proprietary extensions.
  uint8_t             transSeqNum; //<! The Transaction Sequence Number field is 8 bits in length and specifies an identification number for a single transaction that includes one or more frames in both directions.
  uint8_t             commandID; //<! The Command Identifier field is 8 bits in length and specifies the cluster command being used.
} zclFrameHdr_t;

#ifdef ZCL_READ
/// Read Attribute Command format
PACKED_TYPEDEF_STRUCT
{
  uint8_t  numAttr;            //!< number of attributes in the list
  uint16_t attrID[];           //!< supported attributes list - this structure should
                               //!< be allocated with the appropriate number of attributes.
} zclReadCmd_t;

/// Read Attribute Response Status record
PACKED_TYPEDEF_STRUCT
{
  uint16_t attrID;            //!< attribute ID
  uint8_t  status;            //!< should be ZCL_STATUS_SUCCESS or error
  uint8_t  dataType;          //!< attribute data type
  uint8_t  *data;             //!< this structure is allocated, so the data is HERE
                              //!< - the size depends on the attribute data type
} zclReadRspStatus_t;

/// Read Attribute Response Command format
PACKED_TYPEDEF_STRUCT
{
  uint8_t              numAttr;     //!< number of attributes in the list
  zclReadRspStatus_t attrList[];  //!< attribute status list
} zclReadRspCmd_t;
#endif // ZCL_READ

/// Write Attribute record
PACKED_TYPEDEF_STRUCT
{
  uint16_t attrID;             //!< attribute ID
  uint8_t  dataType;           //!< attribute data type
  uint8_t  *attrData;          //!< this structure is allocated, so the data is HERE
                             //!<  - the size depends on the attribute data type
} zclWriteRec_t;

/// Write Attribute Command format
PACKED_TYPEDEF_STRUCT
{
  uint8_t         numAttr;     //!< number of attribute records in the list
  zclWriteRec_t attrList[];  //!< attribute records
} zclWriteCmd_t;

/// Write Attribute Status record
PACKED_TYPEDEF_STRUCT
{
  uint8_t  status;             //!< should be ZCL_STATUS_SUCCESS or error
  uint16_t attrID;             //!< attribute ID
} zclWriteRspStatus_t;

/// Write Attribute Response Command format
PACKED_TYPEDEF_STRUCT
{
  uint8_t               numAttr;     //!< number of attribute status in the list
  zclWriteRspStatus_t attrList[];  //!< attribute status records
} zclWriteRspCmd_t;

/// Configure Reporting Command format
PACKED_TYPEDEF_STRUCT
{
  uint8_t  direction;          //!< to send or receive reports of the attribute
  uint16_t attrID;             //!< attribute ID
  uint8_t  dataType;           //!< attribute data type
  uint16_t minReportInt;       //!< minimum reporting interval
  uint16_t maxReportInt;       //!< maximum reporting interval, 0xFFFF=off
  uint16_t timeoutPeriod;      //!< timeout period
  uint8_t  *reportableChange;  //!< reportable change (only applicable to analog data type)
                             //!< - the size depends on the attribute data type
} zclCfgReportRec_t;

/// Configure report attribute list
PACKED_TYPEDEF_STRUCT
{
  uint8_t             numAttr;    //!< number of attribute IDs in the list
  zclCfgReportRec_t attrList[]; //!< attribute ID list
} zclCfgReportCmd_t;

/// Attribute Status record
PACKED_TYPEDEF_STRUCT
{
  uint8_t  status;             //!< should be ZCL_STATUS_SUCCESS or error
  uint8_t  direction;          //!< whether attributes are reported or reports of attributes are received
  uint16_t attrID;             //!< attribute ID
} zclCfgReportStatus_t;

/// Configure Reporting Response Command format
PACKED_TYPEDEF_STRUCT
{
  uint8_t                numAttr;    //!< number of attribute status in the list
  zclCfgReportStatus_t attrList[]; //!< attribute status records
} zclCfgReportRspCmd_t;

/// Read Reporting Configuration Command format
PACKED_TYPEDEF_STRUCT
{
  uint8_t  direction; //!< to send or receive reports of the attribute
  uint16_t attrID;    //!< attribute ID
} zclReadReportCfgRec_t;

/// Read Report Configuration Command format
PACKED_TYPEDEF_STRUCT
{
  uint8_t                 numAttr;    //!< number of attributes in the list
  zclReadReportCfgRec_t attrList[]; //!< attribute ID list
} zclReadReportCfgCmd_t;

/// Attribute Reporting Configuration record
PACKED_TYPEDEF_STRUCT
{
  uint8_t  status;             //!< status field
  uint8_t  direction;          //!< to send or receive reports of the attribute
  uint16_t attrID;             //!< attribute ID
  uint8_t  dataType;           //!< attribute data type
  uint16_t minReportInt;       //!< minimum reporting interval
  uint16_t maxReportInt;       //!< maximum reporting interval
  uint16_t timeoutPeriod;      //!< timeout period
  uint8_t  *reportableChange;  //!< reportable change (only applicable to analog data type)
                             //!< - the size depends on the attribute data type
} zclReportCfgRspRec_t;

/// Read Reporting Configuration Response Command format
PACKED_TYPEDEF_STRUCT
{
  uint8_t                numAttr;    //!< number of records in the list
  zclReportCfgRspRec_t attrList[]; //!< attribute reporting configuration list
} zclReadReportCfgRspCmd_t;

/// Attribute Report
PACKED_TYPEDEF_STRUCT
{
  uint16_t attrID;             //!< atrribute ID
  uint8_t  dataType;           //!< attribute data type
  uint8_t  *attrData;          //!< this structure is allocated, so the data is HERE
                             //!< - the size depends on the data type of attrID
} zclReport_t;

/// Report Attributes Command format
PACKED_TYPEDEF_STRUCT
{
  uint8_t       numAttr;       //!< number of reports in the list
  zclReport_t attrList[];    //!< attribute report list
} zclReportCmd_t;

/// Default Response Command format
PACKED_TYPEDEF_STRUCT
{
  uint8_t  commandID;
  uint8_t  statusCode;
} zclDefaultRspCmd_t;

/// Discover Attributes and Attributes Extended Command format
PACKED_TYPEDEF_STRUCT
{
  uint16_t startAttr;          //!< specifies the minimum value of the identifier
                             //!< to begin attribute discovery.
  uint8_t  maxAttrIDs;         //!< maximum number of attribute IDs that are to be
                             //!< returned in the resulting response command.
} zclDiscoverAttrsCmd_t;

/// Attribute Report info
PACKED_TYPEDEF_STRUCT
{
  uint16_t attrID;             //!< attribute ID
  uint8_t  dataType;           //!< attribute data type (see Table 17 in Spec)
} zclDiscoverAttrInfo_t;

/// Discover Attributes Response Command format
PACKED_TYPEDEF_STRUCT
{
  uint8_t             discComplete; //!< whether or not there're more attributes to be discovered
  uint8_t             numAttr;      //!< number of attributes in the list
  zclDiscoverAttrInfo_t attrList[];   //!< supported attributes list
} zclDiscoverAttrsRspCmd_t;

/// String Data Type
PACKED_TYPEDEF_STRUCT
{
  uint8_t strLen; //!< String length
  uint8_t *pStr; //!< Pointer to string buffer
} UTF8String_t;

/// Discover Commands Received, Discover Commands Generated
PACKED_TYPEDEF_STRUCT
{
  uint8_t startCmdID; //!< Start command ID to discover
  uint8_t maxCmdID; //!< Maximum command ID to discover
} zclDiscoverCmdsCmd_t;

/// Discover Commands Received Response Command, Discover Commands Generated Response
PACKED_TYPEDEF_STRUCT
{
  uint8_t discComplete;
  uint8_t cmdType;    //!< either ZCL_CMD_DISCOVER_CMDS_GEN or ZCL_CMD_DISCOVER_CMDS_RECEIVED
  uint8_t numCmd;     //!< number of provided commands
  uint8_t pCmdID[];    //!< variable length array
} zclDiscoverCmdsCmdRsp_t;

/// Attribute info
PACKED_TYPEDEF_STRUCT
{
  uint16_t attrID; //!< Attribute ID
  uint8_t attrDataType; //!< Data Type
  uint8_t attrAccessControl; //!< Access control for attribute
} zclExtAttrInfo_t;

/// Discover Attributes Extended Response Command format
PACKED_TYPEDEF_STRUCT
{
  uint8_t discComplete;
  uint8_t numAttr;                  //!< number of attributes provided
  zclExtAttrInfo_t  aExtAttrInfo[];    //!< variable length array
} zclDiscoverAttrsExtRsp_t;

/*********************************************************************
 * Plugins
 */

/// Incoming ZCL message, this buffer will be allocated, cmd will point to the
/// the command record.
typedef struct
{
  afIncomingMSGPacket_t *msg;        //!< incoming message
  zclFrameHdr_t         hdr;         //!< ZCL header parsed
  uint8_t                 *pData;      //!< pointer to data after header
  uint16_t                pDataLen;    //!< length of remaining data
  void                  *attrCmd;    //!< pointer to the parsed attribute or command
} zclIncoming_t;

// Incoming ZCL message passed to the Application. This buffer will be
// allocated and attrCmd will point to the command record.
//
// NOTE - the Application must deallocate the message plus attrCmd buffer.
//
/// Incoming ZCL message passed to the Application.
typedef struct
{
  OsalPort_EventHdr hdr;         //!< OSAL header
  zclFrameHdr_t    zclHdr;      //!< ZCL header parsed
  uint16_t           clusterId;   //!< Cluster ID
  afAddrType_t     srcAddr;     //!< Sender's address
  uint8_t            endPoint;    //!< destination endpoint
  void             *attrCmd;    //!< pointer to the parsed attribute or command; must be freed by Application
} zclIncomingMsg_t;

/*!
 *
 * @brief   Function pointer type to handle incoming messages
 *
 * @param  pInHdlrMsg - incomming message buffer
 *
 * @return  ZStatus_t
 */
typedef ZStatus_t (*zclInHdlr_t)( zclIncoming_t *pInHdlrMsg );

/*!
 *
 * @brief Function pointer type to handle incoming write commands.
 *
 * @param   msg - incoming message
 * @param   logicalClusterID - logical cluster ID
 * @param   writeRec - received data to be written
 *
 * @return  ZStatus_t
 */
typedef ZStatus_t (*zclInWrtHdlr_t)( zclIncoming_t *msg, uint16_t logicalClusterID, zclWriteRec_t *writeRec );

/// Command record
typedef struct
{
  uint16_t   clusterID; //!< Cluster ID
  uint8_t    cmdID; //!< Command ID
  uint8_t    flag;  //!< one of CMD_DIR_CLIENT_GENERATED, CMD_DIR_CLIENT_RECEIVED, CMD_DIR_SERVER_GENERATED, CMD_DIR_SERVER_RECEIVED
} zclCommandRec_t;

/// ZCL Attribute definition
typedef struct
{
  uint16_t  attrId;         //!< Attribute ID
  uint8_t   dataType;       //!< Data Type - defined in af.h
  uint8_t   accessControl;  //!< Read/write - bit field
  void    *dataPtr;       //!< Pointer to data field
} zclAttribute_t;

/// Attribute record
typedef struct
{
  uint16_t          clusterID;    //!< Real cluster ID
  zclAttribute_t  attr;
} zclAttrRec_t;

/*!
 *
 * @brief Function pointer type to validate attribute data.
 *
 * @param   pAttr - where data to be written
 * @param   pAttrInfo - pointer to attribute info
 *
 * @return  uint8_t
 */
typedef uint8_t (*zclValidateAttrData_t)( zclAttrRec_t *pAttr, zclWriteRec_t *pAttrInfo );

/*!
 *
 * @brief Function pointer type to read/write attribute data.
 *
 * @param   clusterId - cluster that attribute belongs to
 * @param   attribute to be read or written
 * @param   oper - ZCL_OPER_LEN, ZCL_OPER_READ, or ZCL_OPER_WRITE
 * @param   pValue - pointer to attribute (length) value
 * @param   pLen - length of attribute value read
 *
 * @return  ZCL_STATUS_SUCCESS: Operation successful, ZCL Error Status: Operation not successful
 */
typedef ZStatus_t (*zclReadWriteCB_t)( uint16_t clusterId, uint16_t attrId, uint8_t oper,
                                       uint8_t *pValue, uint16_t *pLen );


/*!
 *
 * @brief Callback function prototype to authorize a Read or Write operation on a given attribute.
 *
 * @param   srcAddr - source Address
 * @param   pAttr - pointer to attribute
 * @param   oper - ZCL_OPER_LEN, ZCL_OPER_READ, or ZCL_OPER_WRITE
 *
 * @return  ZCL_STATUS_SUCCESS: Operation successful, ZCL_STATUS_NOT_AUTHORIZED: Operation not authorized
 */
typedef ZStatus_t (*zclAuthorizeCB_t)( afAddrType_t *srcAddr, zclAttrRec_t *pAttr, uint8_t oper );

/// ZCL cluster options
typedef struct
{
  uint16_t  clusterID;      //!< Real cluster ID
  uint8_t   option;         //!< AF_WILDCARD_PROFILEID, AF_PREPROCESS, AF_LIMIT_CONCENTRATOR, AF_ACK_REQUEST, AF_SUPRESS_ROUTE_DISC_NETWORK, AF_EN_SECURITY, AF_SKIP_ROUTING
} zclOptionRec_t;

/// Parse received command
typedef struct
{
  uint8_t  endpoint; //!< End Point
  uint16_t dataLen; //!< Buffer Length
  uint8_t  *pData; //!< Pointer to the buffer
} zclParseCmd_t;

/// Attribute record list item
typedef struct zclAttrRecsList
{
  struct zclAttrRecsList *next;
  uint8_t                  endpoint;      //!< Used to link it into the endpoint descriptor
  zclReadWriteCB_t       pfnReadWriteCB;//!< Read or Write attribute value callback function
  zclAuthorizeCB_t       pfnAuthorizeCB;//!< Authorize Read or Write operation
  uint8_t                  numAttributes; //!< Number of the following records
  CONST zclAttrRec_t     *attrs;        //!< attribute records
} zclAttrRecsList;
/** @} End ZCL_TYPEDEFS */

/*********************************************************************
 * GLOBAL VARIABLES
 */
//extern uint8_t zcl_TaskID;
extern uint8_t zcl_InSeqNum;
extern uint8_t zcl_radius;

/*********************************************************************
 * FUNCTION MACROS
 */
#ifdef ZCL_WRITE
/*
 *  @brief Send a Write Command - ZCL_CMD_WRITE
 *  Use like:
 *      ZStatus_t zcl_SendWrite( uint8_t srcEP, afAddrType_t *dstAddr, uint16_t realClusterID, zclWriteCmd_t *writeCmd, uint8_t direction, uint8_t disableDefaultRsp, uint8_t seqNum );
 */
#define zcl_SendWrite(a,b,c,d,e,f,g) (zcl_SendWriteRequest( (a), (b), (c), (d), ZCL_CMD_WRITE, (e), (f), (g) ))

/*
 *  @brief Send a Write Undivided Command - ZCL_CMD_WRITE_UNDIVIDED
 *  Use like:
 *      ZStatus_t zcl_SendWriteUndivided( uint8_t srcEP, afAddrType_t *dstAddr, uint16_t realClusterID, zclWriteCmd_t *writeCmd, uint8_t direction, uint8_t disableDefaultRsp, uint8_t seqNum );
 */
#define zcl_SendWriteUndivided(a,b,c,d,e,f,g) (zcl_SendWriteRequest( (a), (b), (c), (d), ZCL_CMD_WRITE_UNDIVIDED, (e), (f), (g) ))

/*
 *  @brief Send a Write No Response Command - ZCL_CMD_WRITE_NO_RSP
 *  Use like:
 *      ZStatus_t zcl_SendWriteNoRsp( uint8_t srcEP, afAddrType_t *dstAddr, uint16_t realClusterID, zclWriteCmd_t *writeCmd, uint8_t direction, uint8_t disableDefaultRsp, uint8_t seqNum );
 */
#define zcl_SendWriteNoRsp(a,b,c,d,e,f,g) (zcl_SendWriteRequest( (a), (b), (c), (d), ZCL_CMD_WRITE_NO_RSP, (e), (f), (g) ))
#endif // ZCL_WRITE

/*
 *  @brief Send a ZCL Command from application thread
 *  Use like:
 *      ZStatus_t zcl_SendCommand( uint8_t srcEP, afAddrType_t *dstAddr,
 *                                uint16_t clusterID, uint8_t cmd, uint8_t specific, uint8_t direction,
 *                                uint8_t disableDefaultRsp, uint16_t manuCode, uint8_t seqNum,
 *                                uint16_t cmdFormatLen, uint8_t *cmdFormat );
 */
#define zcl_SendCommand(a,b,c,d,e,f,g,h,i,j,k)        (zcl_SendCommandEx(a,b,c,d,e,f,g,h,i,j,k,TRUE))

/*
 *  @brief Send a ZCL Command from Stack thread
 *  Use like:
 *      ZStatus_t zcl_StackSendCommand( uint8_t srcEP, afAddrType_t *dstAddr,
 *                                uint16_t clusterID, uint8_t cmd, uint8_t specific, uint8_t direction,
 *                                uint8_t disableDefaultRsp, uint16_t manuCode, uint8_t seqNum,
 *                                uint16_t cmdFormatLen, uint8_t *cmdFormat );
 */
#define zcl_StackSendCommand(a,b,c,d,e,f,g,h,i,j,k)   (zcl_SendCommandEx(a,b,c,d,e,f,g,h,i,j,k,FALSE))

#ifdef ZCL_REPORTING_DEVICE

/*
 *  @brief Send a ZCL Configure Report Response from Application thread
 *  Use like:
 *      ZStatus_t zcl_StackSendCommand( uint8_t srcEP, afAddrType_t *dstAddr,
 *                                uint16_t clusterID, uint8_t cmd, uint8_t specific, uint8_t direction,
 *                                uint8_t disableDefaultRsp, uint16_t manuCode, uint8_t seqNum,
 *                                uint16_t cmdFormatLen, uint8_t *cmdFormat );
 */
#define zcl_SendConfigReportRspCmd(a,b,c,d,e,f,g)      (zcl_SendConfigReportRspCmdEx(a,b,c,d,e,f,g,TRUE))

/*
 *  @brief Send a ZCL Configure Report Response from Stack thread
 *  Use like:
 *      ZStatus_t zcl_StackSendCommand( uint8_t srcEP, afAddrType_t *dstAddr,
 *                                uint16_t clusterID, uint8_t cmd, uint8_t specific, uint8_t direction,
 *                                uint8_t disableDefaultRsp, uint16_t manuCode, uint8_t seqNum,
 *                                uint16_t cmdFormatLen, uint8_t *cmdFormat );
 */
#define zcl_StackSendConfigReportRspCmd(a,b,c,d,e,f,g) (zcl_SendConfigReportRspCmdEx(a,b,c,d,e,f,g,FALSE))

/*
 *  @brief Send a ZCL Read Report Configuration Response from Application thread
 *  Use like:
 *      ZStatus_t zcl_StackSendCommand( uint8_t srcEP, afAddrType_t *dstAddr,
 *                                uint16_t clusterID, uint8_t cmd, uint8_t specific, uint8_t direction,
 *                                uint8_t disableDefaultRsp, uint16_t manuCode, uint8_t seqNum,
 *                                uint16_t cmdFormatLen, uint8_t *cmdFormat );
 */
#define zcl_SendReadReportCfgRspCmd(a,b,c,d,e,f,g)      (zcl_SendReadReportCfgRspCmdEx(a,b,c,d,e,f,g,TRUE))

/*
 *  @brief Send a ZCL Read Report Configuration Response from Stack thread
 *  Use like:
 *      ZStatus_t zcl_StackSendCommand( uint8_t srcEP, afAddrType_t *dstAddr,
 *                                uint16_t clusterID, uint8_t cmd, uint8_t specific, uint8_t direction,
 *                                uint8_t disableDefaultRsp, uint16_t manuCode, uint8_t seqNum,
 *                                uint16_t cmdFormatLen, uint8_t *cmdFormat );
 */
#define zcl_StackSendReadReportCfgRspCmd(a,b,c,d,e,f,g) (zcl_SendReadReportCfgRspCmdEx(a,b,c,d,e,f,g,FALSE))

/*
 *  @brief Send a ZCL Report from Application thread
 *  Use like:
 *      ZStatus_t zcl_StackSendCommand( uint8_t srcEP, afAddrType_t *dstAddr,
 *                                uint16_t clusterID, uint8_t cmd, uint8_t specific, uint8_t direction,
 *                                uint8_t disableDefaultRsp, uint16_t manuCode, uint8_t seqNum,
 *                                uint16_t cmdFormatLen, uint8_t *cmdFormat );
 */
#define zcl_SendReportCmd(a,b,c,d,e,f,g)       (zcl_SendReportCmdEx(a,b,c,d,e,f,g,TRUE))

/*
 *  @brief Send a ZCL Report from Stack thread
 *  Use like:
 *      ZStatus_t zcl_StackSendCommand( uint8_t srcEP, afAddrType_t *dstAddr,
 *                                uint16_t clusterID, uint8_t cmd, uint8_t specific, uint8_t direction,
 *                                uint8_t disableDefaultRsp, uint16_t manuCode, uint8_t seqNum,
 *                                uint16_t cmdFormatLen, uint8_t *cmdFormat );
 */
#define zcl_StackSendReportCmd(a,b,c,d,e,f,g)  (zcl_SendReportCmdEx(a,b,c,d,e,f,g,FALSE))

#endif


#if !defined ( ZCL_STANDALONE ) || defined ( ZCL_STANDALONE_OSAL )
  #define zcl_mem_alloc      OsalPort_malloc
  #define zcl_memset         memset
  #define zcl_memcpy         OsalPort_memcpy
  #define zcl_mem_free       OsalPort_free
  #define zcl_buffer_uint32  OsalPort_bufferUint32
  //#define zcl_nv_item_init   osal_nv_item_init
  #define zcl_nv_write       osal_nv_write
  #define zcl_nv_read        osal_nv_read
  #define zcl_build_uint32   OsalPort_buildUint32
  #define zcl_cpyExtAddr     osal_cpyExtAddr
#else
  extern void *zcl_mem_alloc( uint16_t size );
  extern void *zcl_memset( void *dest, uint8_t value, int len );
  extern void *zcl_memcpy( void *dst, void *src, unsigned int len );
  extern uint8_t zcl_memcmp(const void *src1, const void *src2, unsigned int len);
  extern void zcl_mem_free(void *ptr);
  extern uint8_t* zcl_buffer_uint32( uint8_t *buf, uint32_t val );
  extern uint8_t zcl_nv_item_init( uint16_t id, uint16_t len, void *buf );
  extern uint8_t zcl_nv_write( uint16_t id, uint16_t ndx, uint16_t len, void *buf );
  extern uint8_t zcl_nv_read( uint16_t id, uint16_t ndx, uint16_t len, void *buf );
  extern uint32_t zcl_build_uint32( uint8_t *swapped, uint8_t len );
  extern void *zcl_cpyExtAddr(uint8_t * pDest, const uint8_t * pSrc);
#endif

/*********************************************************************
 * FUNCTIONS
 */

/**
 * @defgroup ZCL_FUNCTIONS ZCL Functions
 * @{
 */

/*!
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  TRUE
 */
extern uint8_t zcl_HandleExternal( zclIncoming_t *pInMsg );


#if !defined ( ZCL_STANDALONE )
 /*
  * Initialization for the task
  */
extern void zcl_Init( byte task_id );
#endif

#if !defined ( ZCL_STANDALONE )
/*!
 *
 * @param       task_id - task id
 * @param       events - event bitmap
 *
 * @return      unprocessed events
 */
extern uint16_t zcl_event_loop( byte task_id, uint16_t events );
#endif

#if !defined ( ZCL_STANDALONE )
/*!
 *
 * @param   taskId - task Id of the Application where commands will be sent to
 *
 * @return  TRUE if task registeration successful, FALSE otherwise
 *********************************************************************/
extern uint8_t zcl_registerForMsg( uint8_t taskId );

/*!
 *
 *          NOTE: Any Task registered for a unique end point will take
 *          priority over any Task registered with the AF_BROADCAST_ENDPOINT
 *          value.  ie. If task A registers for End Point 1, task B registers
 *          for AF_BROADCAST_ENDPOINT,  commands addressed to End Point 1 will be
 *          sent to Task A and NOT Task B.
 *
 * @param   taskId - task Id of the Application where commands will be sent to
 * @param   endPointId - end point Id of interest
 *
 * @return  TRUE if task registeration successful, FALSE otherwise
 *********************************************************************/
extern uint8_t zcl_registerForMsgExt( uint8_t taskId, uint8_t endPointId  );

#endif

/*!
 *
 * @param       startLogCluster - starting cluster ID
 * @param       endLogCluster - ending cluster ID
 * @param       pfnIncomingHdlr - function pointer to incoming message handler
 *
 * @return      ZSuccess if OK
 */
extern ZStatus_t zcl_registerPlugin( uint16_t startLogCluster, uint16_t endLogCluster,
                                     zclInHdlr_t pfnIncomingHdlr );

/*!
 *
 * @param       endpoint - endpoint the attribute list belongs to
 * @param       cmdListSize - size of the command list
 * @param       newCmdList - array of command records
 *
 * @return      ZSuccess if OK
 */
extern ZStatus_t zcl_registerCmdList( uint8_t endpoint, CONST uint8_t cmdListSize, CONST zclCommandRec_t newCmdList[] );

/*!
 *
 * @param       endpoint - endpoint the attribute list belongs to
 * @param       numAttr - number of attributes in list
 * @param       newAttrList - array of Attribute records.
 *                            NOTE: THE ATTRIBUTE IDs (FOR A CLUSTER) MUST BE IN
 *                            ASCENDING ORDER. OTHERWISE, THE DISCOVERY RESPONSE
 *                            COMMAND WILL NOT HAVE THE RIGHT ATTRIBUTE INFO
 *
 * @return      ZSuccess if OK
 */
extern ZStatus_t zcl_registerAttrList( uint8_t endpoint, uint8_t numAttr, CONST zclAttrRec_t newAttrList[] );

/*!
 *
 * @param       endpoint - endpoint the option list belongs to
 * @param       numOption - number of options in list
 * @param       optionList - array of cluster option records.
 *
 *              NOTE: This API should be called to enable 'Application
 *                    Link Key' security and/or 'APS ACK' for a specific
 *                    Cluster. The 'Application Link Key' is discarded
 *                    if security isn't enabled on the device.
 *                    The default behavior is 'Network Key' when security
 *                    is enabled and no 'APS ACK' for the ZCL messages.
 *
 * @return      ZSuccess if OK
 */
extern ZStatus_t zcl_registerClusterOptionList( uint8_t endpoint, uint8_t numOption, zclOptionRec_t optionList[] );

/*!
 *
 * @param       pfnValidateAttrData - function pointer to validate routine
 *
 * @return      ZSuccess if OK
 */
extern ZStatus_t zcl_registerValidateAttrData( zclValidateAttrData_t pfnValidateAttrData );

/*!
 *
 *              Note: The pfnReadWriteCB callback function is only required
 *                    when the attribute data format is unknown to ZCL. The
 *                    callback function gets called when the pointer 'dataPtr'
 *                    to the attribute value is NULL in the attribute database
 *                    registered with the ZCL.
 *
 *              Note: The pfnAuthorizeCB callback function is only required
 *                    when the Read/Write operation on an attribute requires
 *                    authorization (i.e., attributes with ACCESS_CONTROL_AUTH_READ
 *                    or ACCESS_CONTROL_AUTH_WRITE access permissions).
 *
 * @param       endpoint - application's endpoint
 * @param       pfnReadWriteCB - function pointer to read/write routine
 * @param       pfnAuthorizeCB - function pointer to authorize read/write operation
 *
 * @return      ZSuccess if successful. ZFailure, otherwise.
 */
extern ZStatus_t zcl_registerReadWriteCB( uint8_t endpoint, zclReadWriteCB_t pfnReadWriteCB,
                                          zclAuthorizeCB_t pfnAuthorizeCB );

/*!
 *
 * @param   pkt - incoming message
 *
 * @return  zclProcMsgStatus_t
 */
extern zclProcMsgStatus_t zcl_ProcessMessageMSG( afIncomingMSGPacket_t *pkt );

/*!
 *
 *          NOTE: The calling application is responsible for incrementing
 *                the Sequence Number.
 *
 * @param   srcEP - source endpoint
 * @param   dstAddr - destination address
 * @param   clusterID - cluster ID
 * @param   cmd - command ID
 * @param   specific - whether the command is Cluster Specific
 * @param   direction - client/server direction of the command
 * @param   disableDefaultRsp - disable Default Response command
 * @param   manuCode - manufacturer code for proprietary extensions to a profile
 * @param   seqNum - identification number for the transaction
 * @param   cmdFormatLen - length of the command to be sent
 * @param   cmdFormat - command to be sent
 * @param   isReqFromApp - Indicates where it comes from application thread or stack thread
 *
 * @return  ZSuccess if OK
 */
extern ZStatus_t zcl_SendCommandEx( uint8_t srcEP, afAddrType_t *dstAddr,
                                  uint16_t clusterID, uint8_t cmd, uint8_t specific, uint8_t direction,
                                  uint8_t disableDefaultRsp, uint16_t manuCode, uint8_t seqNum,
                                  uint16_t cmdFormatLen, uint8_t *cmdFormat, uint8_t isReqFromApp  );

#ifdef ZCL_READ
/*!
 *
 * @param   srcEP - Application's endpoint
 * @param   dstAddr - destination address
 * @param   realClusterID - cluster ID
 * @param   readCmd - read command to be sent
 * @param   direction - direction of the command
 * @param   disableDefaultRsp - disable Default Response command
 * @param   seqNum - transaction sequence number
 *
 * @return  ZSuccess if OK
 */
extern ZStatus_t zcl_SendRead( uint8_t srcEP, afAddrType_t *dstAddr,
                               uint16_t realClusterID, zclReadCmd_t *readCmd,
                               uint8_t direction, uint8_t disableDefaultRsp, uint8_t seqNum );

/*!
 *
 * @param   srcEP - Application's endpoint
 * @param   dstAddr - destination address
 * @param   realClusterID - cluster ID
 * @param   readRspCmd - read response command to be sent
 * @param   direction - direction of the command
 * @param   disableDefaultRsp - disable Default Response command
 * @param   seqNum - transaction sequence number
 *
 * @return  ZSuccess if OK
 */
extern ZStatus_t zcl_SendReadRsp( uint8_t srcEP, afAddrType_t *dstAddr,
                                  uint16_t realClusterID, zclReadRspCmd_t *readRspCmd,
                                  uint8_t direction, uint8_t disableDefaultRsp, uint8_t seqNum );

/*!
 *
 * @param   endpoint - application's endpoint
 * @param   clusterId - cluster that attribute belongs to
 * @param   attrId - attribute id
 * @param   pAttrData - where to put attribute data
 * @param   pDataLen - where to put attribute data length
 *
 * @return  Successful if data was read
 */
extern ZStatus_t zcl_ReadAttrData( uint8_t endpoint, uint16_t clusterId, uint16_t attrId,
                                   uint8_t *pAttrData, uint16_t *pDataLen );

#endif // ZCL_READ

#ifdef ZCL_WRITE
/*
 *  Function for Writing an Attribute
 */
extern ZStatus_t zcl_SendWriteRequest( uint8_t srcEP, afAddrType_t *dstAddr,
                                       uint16_t realClusterID, zclWriteCmd_t *writeCmd,
                                       uint8_t cmd, uint8_t direction, uint8_t disableDefaultRsp, uint8_t seqNum );

/*!
 *
 * @param   srcEP - source endpoint
 * @param   dstAddr - destination address
 * @param   realClusterID - cluster ID
 * @param   writeRspCmd - ZCL_CMD_WRITE, ZCL_CMD_WRITE_UNDIVIDED or ZCL_CMD_WRITE_NO_RSP
 * @param   direction - direction of the command
 * @param   disableDefaultRsp - disable Default Response command
 * @param   seqNum - transaction sequence number
 *
 * @return  ZSuccess if OK
 */
extern ZStatus_t zcl_SendWriteRsp( uint8_t srcEP, afAddrType_t *dstAddr,
                                   uint16_t realClusterID, zclWriteRspCmd_t *writeRspCmd,
                                   uint8_t direction, uint8_t disableDefaultRsp, uint8_t seqNum );
#endif // ZCL_WRITE

#ifdef ZCL_REPORT_CONFIGURING_DEVICE
/*!
 *
 * @param   srcEP - source endpoint
 * @param   dstAddr - destination address
 * @param   realClusterID - cluster ID
 * @param   cfgReportCmd - configure reporting command to be sent
 * @param   direction - direction of the command
 * @param   disableDefaultRsp - disable Default Response command
 * @param   seqNum - transaction sequence number
 *
 * @return  ZSuccess if OK
 */
extern ZStatus_t zcl_SendConfigReportCmd( uint8_t srcEP, afAddrType_t *dstAddr,
                          uint16_t realClusterID, zclCfgReportCmd_t *cfgReportCmd,
                          uint8_t direction, uint8_t disableDefaultRsp, uint8_t seqNum );

#endif

#ifdef ZCL_REPORTING_DEVICE

/*!
 *
 * @param   srcEP - source endpoint
 * @param   dstAddr - destination address
 * @param   realClusterID - cluster ID
 * @param   cfgReportRspCmd - configure reporting response command to be sent
 * @param   direction - direction of the command
 * @param   disableDefaultRsp - disable Default Response command
 * @param   seqNum - transaction sequence number
 * @param   isReqFromApp - Indicates where it comes from application thread or stack thread
 *
 * @return  ZSuccess if OK
 */
extern ZStatus_t zcl_SendConfigReportRspCmdEx( uint8_t srcEP, afAddrType_t *dstAddr,
                    uint16_t realClusterID, zclCfgReportRspCmd_t *cfgReportRspCmd,
                    uint8_t direction, uint8_t disableDefaultRsp, uint8_t seqNum, uint8_t isReqFromApp );
#endif

#ifdef ZCL_REPORT_CONFIGURING_DEVICE
/*!
 *
 * @param   srcEP - source endpoint
 * @param   dstAddr - destination address
 * @param   realClusterID - cluster ID
 * @param   readReportCfgCmd - read reporting configuration command to be sent
 * @param   direction - direction of the command
 * @param   disableDefaultRsp - disable Default Response command
 * @param   seqNum - transaction sequence number
 *
 * @return  ZSuccess if OK
 */
extern ZStatus_t zcl_SendReadReportCfgCmd( uint8_t srcEP, afAddrType_t *dstAddr,
                              uint16_t realClusterID, zclReadReportCfgCmd_t *readReportCfgCmd,
                              uint8_t direction, uint8_t disableDefaultRsp, uint8_t seqNum );
#endif

#ifdef ZCL_REPORTING_DEVICE

/*!
 *
 * @param   srcEP - source endpoint
 * @param   dstAddr - destination address
 * @param   realClusterID - cluster ID
 * @param   readReportCfgRspCmd - read reporting configuration response command to be sent
 * @param   direction - direction of the command
 * @param   disableDefaultRsp - disable Default Response command
 * @param   seqNum - transaction sequence number
 * @param   isReqFromApp - Indicates where it comes from application thread or stack thread
 *
 * @return  ZSuccess if OK
 */
extern ZStatus_t zcl_SendReadReportCfgRspCmdEx( uint8_t srcEP, afAddrType_t *dstAddr,
                        uint16_t realClusterID, zclReadReportCfgRspCmd_t *readReportCfgRspCmd,
                        uint8_t direction, uint8_t disableDefaultRsp, uint8_t seqNum, uint8_t isReqFromApp );

/*!
 *
 * @param   srcEP - source endpoint
 * @param   dstAddr - destination address
 * @param   realClusterID - cluster ID
 * @param   reportCmd - report command to be sent
 * @param   direction - direction of the command
 * @param   disableDefaultRsp - disable Default Response command
 * @param   seqNum - transaction sequence number
 * @param   isReqFromApp - Indicates where it comes from application thread or stack thread
 *
 * @return  ZSuccess if OK
 */
extern ZStatus_t zcl_SendReportCmdEx( uint8_t srcEP, afAddrType_t *dstAddr,
                              uint16_t realClusterID, zclReportCmd_t *reportCmd,
                              uint8_t direction, uint8_t disableDefaultRsp, uint8_t seqNum, uint8_t isReqFromApp );
#endif

/*!
 *
 *          Note: The manufacturer code field should be set if this
 *          command is being sent in response to a manufacturer specific
 *          command.
 *
 * @param   srcEP - source endpoint
 * @param   dstAddr - destination address
 * @param   realClusterID - cluster ID
 * @param   defaultRspCmd - default response command to be sent
 * @param   direction - direction of the command
 * @param   disableDefaultRsp - disable Default Response command
 * @param   manuCode - manufacturer code for proprietary extensions to a profile
 * @param   seqNum - transaction sequence number
 *
 * @return  ZSuccess if OK
 */
extern ZStatus_t zcl_SendDefaultRspCmd( uint8_t srcEP, afAddrType_t *dstAddr, uint16_t realClusterID,
                                        zclDefaultRspCmd_t *defaultRspCmd, uint8_t direction,
                                        uint8_t disableDefaultRsp, uint16_t manuCode, uint8_t seqNum );

#ifdef ZCL_DISCOVER
/*!
 *
 * @param   srcEP - source endpoint
 * @param   dstAddr - destination address
 * @param   clusterID - cluster ID
 * @param   cmdType - requesting command ID
 * @param   pDiscoverCmd - discover command to be sent
 * @param   direction - direction of the command
 * @param   disableDefaultRsp - disable Default Response command
 * @param   seqNum - transaction sequence number
 *
 * @return  ZSuccess if OK
 */
extern ZStatus_t zcl_SendDiscoverCmdsCmd( uint8_t srcEP, afAddrType_t *dstAddr,
                            uint16_t clusterID, uint8_t cmdType, zclDiscoverCmdsCmd_t *pDiscoverCmd,
                            uint8_t direction, uint8_t disableDefaultRsp, uint8_t seqNum );

/*!
 *
 * @param   srcEP - source endpoint
 * @param   dstAddr - destination address
 * @param   clusterID - cluster ID
 * @param   pDiscoverRspCmd - response command to be sent
 * @param   direction - direction of the command
 * @param   disableDefaultRsp - disable Default Response command
 * @param   seqNum - transaction sequence number
 *
 * @return  ZSuccess if OK
 */
extern ZStatus_t zcl_SendDiscoverCmdsRspCmd( uint8_t srcEP, afAddrType_t *dstAddr,
                          uint16_t clusterID, zclDiscoverCmdsCmdRsp_t *pDiscoverRspCmd,
                          uint8_t direction, uint8_t disableDefaultRsp, uint8_t seqNum );

/*!
 *
 * @param   srcEP - source endpoint
 * @param   dstAddr - destination address
 * @param   realClusterID - cluster ID
 * @param   pDiscoverCmd - discover command to be sent
 * @param   direction - direction of the command
 * @param   disableDefaultRsp - disable Default Response command
 * @param   seqNum - transaction sequence number
 *
 * @return  ZSuccess if OK
 */
extern ZStatus_t zcl_SendDiscoverAttrsCmd( uint8_t srcEP, afAddrType_t *dstAddr,
                            uint16_t realClusterID, zclDiscoverAttrsCmd_t *pDiscoverCmd,
                            uint8_t direction, uint8_t disableDefaultRsp, uint8_t seqNum );

/*!
 *
 * @param   srcEP - source endpoint
 * @param   dstAddr - destination address
 * @param   realClusterID - cluster ID
 * @param   pDiscoverRspCmd - report response command to be sent
 * @param   direction - direction of the command
 * @param   disableDefaultRsp - disable Default Response command
 * @param   seqNum - transaction sequence number
 *
 * @return  ZSuccess if OK
 */
extern ZStatus_t zcl_SendDiscoverAttrsRspCmd( uint8_t srcEP, afAddrType_t *dstAddr,
                      uint16_t realClusterID, zclDiscoverAttrsRspCmd_t *pDiscoverRspCmd,
                      uint8_t direction, uint8_t disableDefaultRsp, uint8_t seqNum );

/*!
 *
 * @param   srcEP - source endpoint
 * @param   dstAddr - destination address
 * @param   realClusterID - cluster ID
 * @param   pDiscoverAttrsExt:
 *            - startAttrID: the first attribute to be selected
 *            - maxAttrIDs: maximum number of returned attributes
 * @param   direction - direction of the command
 * @param   disableDefaultRsp - disable Default Response command
 * @param   seqNum - transaction sequence number
 *
 * @return  ZSuccess if OK
 */
extern ZStatus_t zcl_SendDiscoverAttrsExt( uint8_t srcEP, afAddrType_t *dstAddr,
                                     uint16_t realClusterID, zclDiscoverAttrsCmd_t *pDiscoverAttrsExt,
                                     uint8_t direction, uint8_t disableDefaultRsp, uint8_t seqNum );

/*!
 *
 * @param   srcEP - source endpoint
 * @param   dstAddr - destination address
 * @param   realClusterID - cluster ID
 * @param   pDiscoverAttrsExtRsp:
 *            - discComplete: indicates whether all requested attributes returned
 *            - attrID: attribute ID
 *            - attrDataType: data type of the given attribute
 *            - attrAccessControl: access control of the given attribute
 * @param   direction - direction of the command
 * @param   disableDefaultRsp - disable Default Response command
 * @param   seqNum - transaction sequence number
 *
 * @return  ZSuccess if OK
 */
extern ZStatus_t zcl_SendDiscoverAttrsExtRsp( uint8_t srcEP, afAddrType_t *dstAddr,
                                         uint16_t realClusterID, zclDiscoverAttrsExtRsp_t *pDiscoverAttrsExtRsp,
                                         uint8_t direction, uint8_t disableDefaultRsp, uint8_t seqNum );
#endif // ZCL_DISCOVER

#ifdef ZCL_READ
/*!
 *
 *      NOTE: THIS FUNCTION ALLOCATES THE RETURN BUFFER, SO THE CALLING
 *            FUNCTION IS RESPONSIBLE TO FREE THE MEMORY.
 *
 * @param   pCmd - pointer to incoming data to parse
 *
 * @return  pointer to the parsed command structure
 */
extern void *zclParseInReadCmd( zclParseCmd_t *pCmd );
#endif // ZCL_READ

#ifdef ZCL_WRITE
/*!
 *
 *      NOTE: THIS FUNCTION ALLOCATES THE RETURN BUFFER, SO THE CALLING
 *            FUNCTION IS RESPONSIBLE TO FREE THE MEMORY.
 *
 * @param   pCmd - pointer to incoming data to parse
 *
 * @return  pointer to the parsed command structure
 */
extern void *zclParseInWriteCmd( zclParseCmd_t *pCmd );
#endif // ZCL_WRITE

#ifdef ZCL_REPORTING_DEVICE
/*!
 *
 *      NOTE: THIS FUNCTION ALLOCATES THE RETURN BUFFER, SO THE CALLING
 *            FUNCTION IS RESPONSIBLE TO FREE THE MEMORY.
 *
 * @param   pCmd - pointer to incoming data to parse
 *
 * @return  pointer to the parsed command structure
 */
extern void *zclParseInConfigReportCmd( zclParseCmd_t *pCmd );
/*!
 *
 *      NOTE: THIS FUNCTION ALLOCATES THE RETURN BUFFER, SO THE CALLING
 *            FUNCTION IS RESPONSIBLE TO FREE THE MEMORY.
 *
 * @param   pCmd - pointer to incoming data to parse
 *
 * @return  pointer to the parsed command structure
 */
extern void *zclParseInReadReportCfgCmd( zclParseCmd_t *pCmd );
#endif

#if defined ZCL_REPORTING_DEVICE || defined ZCL_REPORT_CONFIGURING_DEVICE
/*!
 *
 * @param   dataType - data type
 *
 * @return  TRUE if data type is analog
 */
extern uint8_t zclAnalogDataType( uint8_t dataType );
#endif


#ifdef ZCL_REPORT_DESTINATION_DEVICE
/*!
 *
 *      NOTE: THIS FUNCTION ALLOCATES THE RETURN BUFFER, SO THE CALLING
 *            FUNCTION IS RESPONSIBLE TO FREE THE MEMORY.
 *
 * @param   pCmd - pointer to incoming data to parse
 *
 * @return  pointer to the parsed command structure
 */
extern void *zclParseInReportCmd( zclParseCmd_t *pCmd );
#endif

#ifdef ZCL_DISCOVER
/*!
 *
 *      NOTE: THIS FUNCTION ALLOCATES THE RETURN BUFFER, SO THE CALLING
 *            FUNCTION IS RESPONSIBLE TO FREE THE MEMORY.
 *
 * @param   pCmd - pointer to incoming data to parse
 *
 * @return  pointer to the parsed command structure
 */
extern void *zclParseInDiscCmdsCmd( zclParseCmd_t *pCmd );

/*!
 *
 *      NOTE: THIS FUNCTION ALLOCATES THE RETURN BUFFER, SO THE CALLING
 *            FUNCTION IS RESPONSIBLE TO FREE THE MEMORY.
 *
 * @param   pCmd - pointer to incoming data to parse
 *
 * @return  pointer to the parsed command structure
 */
extern void *zclParseInDiscAttrsCmd( zclParseCmd_t *pCmd );

/*!
 *
 * @param   endpoint - Application's endpoint
 * @param   clusterID - cluster ID
 * @param   cmdID - command ID
 * @param   pCmd - attribute record to be returned
 *
 * @return  TRUE if record found. FALSE, otherwise.
 */
extern uint8_t zclFindCmdRec( uint8_t endpoint, uint16_t clusterID, uint8_t cmdID, zclCommandRec_t *pCmd );
#endif // ZCL_DISCOVER

/*!
 *
 * @param   hdr - place to put the frame control information
 * @param   pData - incoming buffer to parse
 *
 * @return  pointer past the header
 */
extern uint8_t *zclParseHdr( zclFrameHdr_t *hdr, uint8_t *pData );

/*!
 *
 * @param   endpoint - Application's endpoint
 * @param   realClusterID - cluster ID
 * @param   attrId - attribute looking for
 * @param   pAttr - attribute record to be returned
 *
 * @return  TRUE if record found. FALSE, otherwise.
 */
extern uint8_t zclFindAttrRec( uint8_t endpoint, uint16_t realClusterID, uint16_t attrId, zclAttrRec_t *pAttr );

#if defined ( ZCL_STANDALONE )
/*!
 *
 * @param   endpoint - endpoint the attribute list belongs to
 * @param   numAttr - number of attributes in list
 * @param   attrList - array of attribute records.
 *                     NOTE: THE ATTRIBUTE IDs (FOR A CLUSTER) MUST BE
 *                     IN ASCENDING ORDER. OTHERWISE, THE DISCOVERY
 *                     RESPONSE COMMAND WILL NOT HAVE THE RIGHT
 *                     ATTRIBUTE INFO
 *
 * @return  TRUE if successful, FALSE otherwise.
 */
extern uint8_t zclSetAttrRecList( uint8_t endpoint, uint8_t numAttr, CONST zclAttrRec_t attrList[] );
#endif

/*!
 *
 * @param   pAttrData - where to put attribute data
 * @param   pAttr - pointer to attribute
 * @param   pDataLen - where to put attribute data length
 *
 * @return Success
 */
extern ZStatus_t zclReadAttrData( uint8_t *pAttrData, zclAttrRec_t *pAttr, uint16_t *pDataLen );

/*!
 *
 *          NOTE: Should not be called for ZCL_DATATYPE_OCTECT_STR or
 *                ZCL_DATATYPE_CHAR_STR data types.
 *
 * @param   dataType - data type
 *
 * @return  length of data
 */
extern uint8_t zclGetDataTypeLength( uint8_t dataType );

/*!
 *
 * @param   dataType - data types defined in zcl.h
 * @param   attrData - pointer to the attribute data
 * @param   buf - where to put the serialized data
 *
 * @return  pointer to end of destination buffer
 */
extern uint8_t *zclSerializeData( uint8_t dataType, void *attrData, uint8_t *buf );

/*!
 *
 * @param   dataType - data type
 * @param   pData - pointer to data
 *
 * @return  returns atrribute length
 */
extern uint16_t zclGetAttrDataLength( uint8_t dataType, uint8_t *pData);

/*!
 *
 *
 * @return      pointer to original AF message, NULL if not processing
 *              AF message.
 */
extern afIncomingMSGPacket_t *zcl_getRawAFMsg( void );

/*!
 *
 *
 * @return  next ZCL frame counter
 */
extern uint8_t zcl_getFrameCounter( void );

/** @} End ZCL_FUNCTIONS */

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* ZCL_H */

/** @} End ZCL */
