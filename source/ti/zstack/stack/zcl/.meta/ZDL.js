/*
 * Copyright (c) 2020 Texas Instruments Incorporated - http://www.ti.com
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS AS IS
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
"use strict";

const ZDL =
{
	"globals": [
		{
			"server": {
				"cluster": [
					{
						"_id": "0x0000",
						"_mandatory": "true"
					},
					{
						"_id": "0x0003",
						"_mandatory": "true"
					},
          {
            "_id": "0x0004",
            "_mandatory": "false"
          }
				]
			},
			"_name": "all"
		},
		{
			"client": {
				"cluster": [
					{
						"_id": "0x0003",
						"_mandatory": "true"
					},
					{
						"_id": "0x0004",
						"_mandatory": "false"
					},
					{
						"_id": "0x0005",
						"_mandatory": "false"
					}
				]
			},
			"_name": "initiator"
		},
		{
			"server": {
				"cluster": [
					{
						"_id": "0x0005",
						"_mandatory": "false"
					}
				]
			},
			"_name": "target"
		}],
	"devices": [
		{
			"_name": "Generic",
			"_id": "0xFFFE",
			"_definition": "ZCL_DEVICEID_GENERIC",
			"_location": "zcl_ha.h"
		},
		{
			"server": {
				"cluster": [
					{
						"_id": "0x0007",
						"_mandatory": "false"
					}
				]
			},
			"client": {
				"cluster": [
					{
						"_id": "0x0006",
						"_mandatory": "true"
					},
					{
						"_id": "0x0003",
						"_mandatory": "true"
					},
					{
						"_id": "0x0005",
						"_mandatory": "false"
					},
					{
						"_id": "0x0004",
						"_mandatory": "false"
					}
				]
			},
			"_name": "On/Off Switch",
			"_id": "0x0000",
			"_definition": "ZCL_DEVICEID_ON_OFF_SWITCH",
			"_location": "zcl_ha.h"
		},
		{
			"server": {
				"cluster": [
					{
						"_id": "0x0007",
						"_mandatory": "false"
					}
				]
			},
			"client": {
				"cluster": [
					{
						"_id": "0x0006",
						"_mandatory": "true"
					},
					{
						"_id": "0x0003",
						"_mandatory": "true"
					},
					{
						"_id": "0x0008",
						"_mandatory": "true"
					},
					{
						"_id": "0x0005",
						"_mandatory": "false"
					},
					{
						"_id": "0x0004",
						"_mandatory": "false"
					}
				]
			},
			"_name": "Level Control Switch",
			"_id": "0x0001",
			"_definition": "ZCL_DEVICEID_LEVEL_CONTROL_SWITCH",
			"_location": "zcl_ha.h"
		},
		{
			"server": {
				"cluster": [
					{
						"_id": "0x0005",
						"_mandatory": "true"
					},
					{
						"_id": "0x0004",
						"_mandatory": "true"
					},
					{
						"_id": "0x0006",
						"_mandatory": "true"
					}
				]
			},
			"_name": "On/Off Output",
			"_id": "0x0002",
			"_definition": "ZCL_DEVICEID_ON_OFF_OUTPUT",
			"_location": "zcl_ha.h"
		},
		{
			"server": {
				"cluster": [
					{
						"_id": "0x0005",
						"_mandatory": "true"
					},
					{
						"_id": "0x0004",
						"_mandatory": "true"
					},
					{
						"_id": "0x0006",
						"_mandatory": "true"
					},
					{
						"_id": "0x0008",
						"_mandatory": "true"
					}
				]
			},
			"_name": "Level Controllable Output",
			"_id": "0x0003",
			"_definition": "ZCL_DEVICEID_LEVEL_CONTROLLABLE_OUTPUT",
			"_location": "zcl_ha.h"
		},
		{
			"client": {
				"cluster": [
					{
						"_id": "0x0005",
						"_mandatory": "true"
					},
					{
						"_id": "0x0004",
						"_mandatory": "true"
					},
					{
						"_id": "0x0003",
						"_mandatory": "false"
					}
				]
			},
			"_name": "Scene Selector",
			"_id": "0x0004",
			"_definition": "ZCL_DEVICEID_SCENE_SELECTOR",
			"_location": "zcl_ha.h"
		},
		{
			"client": {
				"cluster": [
					{
						"_id": "0x0003",
						"_mandatory": "false"
					},
					{
						"_id": "0x0005",
						"_mandatory": "false"
					},
					{
						"_id": "0x0004",
						"_mandatory": "false"
					},
					{
						"_id": "0x0000",
						"_mandatory": "false"
					},
					{
						"_id": "0x0401",
						"_mandatory": "false"
					},
					{
						"_id": "0x0402",
						"_mandatory": "false"
					},
					{
						"_id": "0x0403",
						"_mandatory": "false"
					},
					{
						"_id": "0x0404",
						"_mandatory": "false"
					},
					{
						"_id": "0x0406",
						"_mandatory": "false"
					},
					{
						"_id": "0x0200",
						"_mandatory": "false"
					},
					{
						"_id": "0x0100",
						"_mandatory": "false"
					},
					{
						"_id": "0x0204",
						"_mandatory": "false"
					}
				]
			},
			"_name": "Configuration Tool",
			"_id": "0x0005",
			"_definition": "ZCL_DEVICEID_CONFIGURATION_TOOL",
			"_location": "zcl_ha.h",
			"_note": "Minimum one optional cluster"
		},
		{
			"client": {
				"cluster": [
					{
						"_id": "0x0003",
						"_mandatory": "false"
					},
					{
						"_id": "0x0005",
						"_mandatory": "false"
					},
					{
						"_id": "0x0004",
						"_mandatory": "false"
					},
					{
						"_id": "0x0007",
						"_mandatory": "false"
					},
					{
						"_id": "0x0006",
						"_mandatory": "false"
					},
					{
						"_id": "0x0008",
						"_mandatory": "false"
					},
					{
						"_id": "0x0000",
						"_mandatory": "false"
					},
					{
						"_id": "0x0401",
						"_mandatory": "false"
					},
					{
						"_id": "0x0402",
						"_mandatory": "false"
					},
					{
						"_id": "0x0200",
						"_mandatory": "false"
					},
					{
						"_id": "0x0100",
						"_mandatory": "false"
					},
					{
						"_id": "0x0300",
						"_mandatory": "false"
					},
					{
						"_id": "0x0400",
						"_mandatory": "false"
					},
					{
						"_id": "0x0102",
						"_mandatory": "false"
					},
					{
						"_id": "0x0101",
						"_mandatory": "false"
					},
					{
						"_id": "0x0201",
						"_mandatory": "false"
					}
				]
			},
			"_name": "Remote Control",
			"_id": "0x0006",
			"_definition": "ZCL_DEVICEID_REMOTE_CONTROL",
			"_location": "zcl_ha.h",
			"_note": "Minimum one optional cluster"
		},
		{
			"client": {
				"cluster": [
					{
						"_id": "0x0003",
						"_mandatory": "false"
					},
					{
						"_id": "0x0005",
						"_mandatory": "false"
					},
					{
						"_id": "0x0004",
						"_mandatory": "false"
					},
					{
						"_id": "0x0007",
						"_mandatory": "false"
					},
					{
						"_id": "0x0006",
						"_mandatory": "false"
					},
					{
						"_id": "0x0008",
						"_mandatory": "false"
					},
					{
						"_id": "0x0000",
						"_mandatory": "false"
					},
					{
						"_id": "0x0401",
						"_mandatory": "false"
					},
					{
						"_id": "0x0402",
						"_mandatory": "false"
					},
					{
						"_id": "0x0200",
						"_mandatory": "false"
					},
					{
						"_id": "0x0100",
						"_mandatory": "false"
					},
					{
						"_id": "0x0204",
						"_mandatory": "false"
					},
					{
						"_id": "0x0300",
						"_mandatory": "false"
					},
					{
						"_id": "0x0400",
						"_mandatory": "false"
					},
					{
						"_id": "0x0102",
						"_mandatory": "false"
					},
					{
						"_id": "0x0101",
						"_mandatory": "false"
					},
					{
						"_id": "0x0201",
						"_mandatory": "false"
					}
				]
			},
			"_name": "Combined Interface",
			"_id": "0x0007",
			"_definition": "ZCL_DEVICEID_COMBINED_INTERFACE",
			"_location": "zcl_ha.h",
			"_note": "Minimum one optional cluster"
		},
		{
			"server": {
				"cluster": [
					{
						"_id": "0x0003",
						"_mandatory": "true"
					}
				]
			},
			"_name": "Range Extender",
			"_id": "0x0008",
			"_definition": "ZCL_DEVICEID_RANGE_EXTENDER",
			"_location": "zcl_ha.h"
		},
		{
			"server": {
				"cluster": [
					{
						"_id": "0x0005",
						"_mandatory": "true"
					},
					{
						"_id": "0x0004",
						"_mandatory": "true"
					},
					{
						"_id": "0x0006",
						"_mandatory": "true"
					}
				]
			},
			"_name": "Mains Power Outlet",
			"_id": "0x0009",
			"_definition": "ZCL_DEVICEID_MAINS_POWER_OUTLET",
			"_location": "zcl_ha.h"
		},
		{
			"server": {
				"cluster": [
					{
						"_id": "0x0005",
						"_mandatory": "true"
					},
					{
						"_id": "0x0004",
						"_mandatory": "true"
					},
					{
						"_id": "0x0101",
						"_mandatory": "true"
					},
					{
						"_id": "0x0009",
						"_mandatory": "false"
					},
					{
						"_id": "0x0001",
						"_mandatory": "false"
					},
					{
						"_id": "0x0020",
						"_mandatory": "false"
					}
				]
			},
			"client": {
				"cluster": [
					{
						"_id": "0x000a",
						"_mandatory": "false"
					}
				]
			},
			"_name": "Door Lock",
			"_id": "0x000a",
			"_definition": "ZCL_DEVICEID_DOOR_LOCK",
			"_location": "zcl_ha.h"
		},
		{
			"client": {
				"cluster": [
					{
						"_id": "0x0005",
						"_mandatory": "true"
					},
					{
						"_id": "0x0004",
						"_mandatory": "true"
					},
					{
						"_id": "0x0003",
						"_mandatory": "true"
					},
					{
						"_id": "0x0101",
						"_mandatory": "true"
					}
				]
			},
			"_name": "Door Lock Controller",
			"_id": "0x000b",
			"_definition": "ZCL_DEVICEID_DOOR_LOCK_CONTROLLER",
			"_location": "zcl_ha.h"
		},
		{
			"server": {
				"cluster": [
					{
						"_id": "0x0005",
						"_mandatory": "true"
					},
					{
						"_id": "0x0004",
						"_mandatory": "true"
					},
					{
						"_id": "0x0006",
						"_mandatory": "true"
					},
					{
						"_id": "0x0008",
						"_mandatory": "true"
					},
					{
						"_id": "0x0100",
						"_mandatory": "true"
					}
				]
			},
			"_name": "Shade",
			"_id": "0x0200",
			"_definition": "ZCL_DEVICEID_SHADE",
			"_location": "zcl_ha.h"
		},
		{
			"client": {
				"cluster": [
					{
						"_id": "0x0003",
						"_mandatory": "true"
					},
					{
						"_id": "0x0006",
						"_mandatory": "true"
					},
					{
						"_id": "0x0008",
						"_mandatory": "true"
					},
					{
						"_id": "0x0005",
						"_mandatory": "false"
					},
					{
						"_id": "0x0004",
						"_mandatory": "false"
					},
					{
						"_id": "0x0100",
						"_mandatory": "false"
					}
				]
			},
			"_name": "Shade Controller",
			"_id": "0x0201",
			"_definition": "ZCL_DEVICEID_SHADE_CONTROLLER",
			"_location": "zcl_ha.h"
		},
		{
			"server": {
				"cluster": [
					{
						"_id": "0x0005",
						"_mandatory": "true"
					},
					{
						"_id": "0x0004",
						"_mandatory": "true"
					},
					{
						"_id": "0x0102",
						"_mandatory": "true"
					}
				]
			},
			"_name": "Window Covering",
			"_id": "0x0202",
			"_definition": "ZCL_DEVICEID_WINDOW_COVERING",
			"_location": "zcl_ha.h"
		},
		{
			"client": {
				"cluster": [
					{
						"_id": "0x0102",
						"_mandatory": "true"
					},
					{
						"_id": "0x0003",
						"_mandatory": "false"
					},
					{
						"_id": "0x0005",
						"_mandatory": "false"
					},
					{
						"_id": "0x0004",
						"_mandatory": "false"
					}
				]
			},
			"_name": "Window Covering Controller",
			"_id": "0x0203",
			"_definition": "ZCL_DEVICEID_WINDOW_COVERING_CONTROLLER",
			"_location": "zcl_ha.h"
		},
		{
			"server": {
				"cluster": [
					{
						"_id": "0x0004",
						"_mandatory": "false"
					},
					{
						"_id": "0x0008",
						"_mandatory": "false"
					},
					{
						"_id": "0x0202",
						"_mandatory": "false"
					}
				]
			},
			"_name": "Heating/Cooling Unit",
			"_id": "0x0300",
			"_definition": "ZCL_DEVICEID_HEATING_COOLING_UNIT",
			"_location": "zcl_ha.h"
		},
		{
			"server": {
				"cluster": [
					{
						"_id": "0x0201",
						"_mandatory": "true"
					},
					{
						"_id": "0x0005",
						"_mandatory": "false"
					},
					{
						"_id": "0x0004",
						"_mandatory": "false"
					},
					{
						"_id": "0x0204",
						"_mandatory": "false"
					},
					{
						"_id": "0x0009",
						"_mandatory": "false"
					},
					{
						"_id": "0x0001",
						"_mandatory": "false"
					},
					{
						"_id": "0x0020",
						"_mandatory": "false"
					}
				]
			},
			"client": {
				"cluster": [
					{
						"_id": "0x0402",
						"_mandatory": "false"
					},
					{
						"_id": "0x0406",
						"_mandatory": "false"
					},
					{
						"_id": "0x000a",
						"_mandatory": "false"
					},

					{
						"_id": "0x0202",
						"_mandatory": "false"
					},
					{
						"_id": "0x0405",
						"_mandatory": "false"
					}
				]
			},
			"_name": "Thermostat",
			"_id": "0x0301",
			"_definition": "ZCL_DEVICEID_THERMOSTAT",
			"_location": "zcl_ha.h"
		},
		{
			"server": {
				"cluster": [
					{
						"_id": "0x0402",
						"_mandatory": "true"
					}
				]
			},
			"client": {
				"cluster": [
					{
						"_id": "0x0003",
						"_mandatory": "true"
					},
					{
						"_id": "0x0004",
						"_mandatory": "false"
					}
				]
			},
			"_name": "Temperature Sensor",
			"_id": "0x0302",
			"_definition": "ZCL_DEVICEID_TEMPERATURE_SENSOR",
			"_location": "zcl_ha.h"
		},
		{
			"server": {
				"cluster": [
					{
						"_id": "0x0005",
						"_mandatory": "true"
					},
					{
						"_id": "0x0004",
						"_mandatory": "true"
					},
					{
						"_id": "0x0006",
						"_mandatory": "true"
					},
					{
						"_id": "0x0200",
						"_mandatory": "true"
					},
					{
						"_id": "0x0008",
						"_mandatory": "false"
					},
					{
						"_id": "0x0009",
						"_mandatory": "false"
					}
				]
			},
			"client": {
				"cluster": [
					{
						"_id": "0x0003",
						"_mandatory": "true"
					}
				]
			},
			"_name": "Pump",
			"_id": "0x0303",
			"_definition": "ZCL_DEVICEID_PUMP",
			"_location": "zcl_ha.h"
		},
		{
			"client": {
				"cluster": [
					{
						"_id": "0x0003",
						"_mandatory": "true"
					},
					{
						"_id": "0x0006",
						"_mandatory": "true"
					},
					{
						"_id": "0x0200",
						"_mandatory": "true"
					},
					{
						"_id": "0x0005",
						"_mandatory": "false"
					},
					{
						"_id": "0x0004",
						"_mandatory": "false"
					},
					{
						"_id": "0x0008",
						"_mandatory": "false"
					},
					{
						"_id": "0x0402",
						"_mandatory": "false"
					},
					{
						"_id": "0x0403",
						"_mandatory": "false"
					},
					{
						"_id": "0x0404",
						"_mandatory": "false"
					}
				]
			},
			"_name": "Pump Controller",
			"_id": "0x0304",
			"_definition": "ZCL_DEVICEID_PUMP_CONTROLLER",
			"_location": "zcl_ha.h"
		},
		{
			"server": {
				"cluster": [
					{
						"_id": "0x0403",
						"_mandatory": "true"
					}
				]
			},
			"client": {
				"cluster": [
					{
						"_id": "0x0003",
						"_mandatory": "true"
					},
					{
						"_id": "0x0004",
						"_mandatory": "false"
					}
				]
			},
			"_name": "Pressure Sensor",
			"_id": "0x0305",
			"_definition": "ZCL_DEVICEID_PRESSURE_SENSOR",
			"_location": "zcl_ha.h"
		},
		{
			"server": {
				"cluster": [
					{
						"_id": "0x0404",
						"_mandatory": "true"
					}
				]
			},
			"client": {
				"cluster": [
					{
						"_id": "0x0003",
						"_mandatory": "true"
					},
					{
						"_id": "0x0004",
						"_mandatory": "false"
					}
				]
			},
			"_name": "Flow Sensor",
			"_id": "0x0306",
			"_definition": "ZCL_DEVICEID_FLOW_SENSOR",
			"_location": "zcl_ha.h"
		},
		{
			"server": {
				"cluster": [
					{
						"_id": "0x0501",
						"_mandatory": "true"
					}
				]
			},
			"client": {
				"cluster": [
					{
						"_id": "0x0003",
						"_mandatory": "true"
					},
					{
						"_id": "0x0502",
						"_mandatory": "true"
					},
					{
						"_id": "0x0500",
						"_mandatory": "true"
					},
					{
						"_id": "0x0005",
						"_mandatory": "false"
					},
					{
						"_id": "0x0004",
						"_mandatory": "false"
					}
				]
			},
			"_name": "IAS CIE",
			"_id": "0x0400",
			"_definition": "ZCL_DEVICEID_IAS_CIE",
			"_location": "zcl_ha.h"
		},
		{
			"server": {
				"cluster": [
					{
						"_id": "0x0500",
						"_mandatory": "true"
					}
				]
			},
			"client": {
				"cluster": [
					{
						"_id": "0x0003",
						"_mandatory": "true"
					},
					{
						"_id": "0x0501",
						"_mandatory": "true"
					}
				]
			},
			"_name": "IAS ACE",
			"_id": "0x0401",
			"_definition": "ZCL_DEVICEID_IAS_ACE",
			"_location": "zcl_ha.h"
		},
		{
			"server": {
				"cluster": [
					{
						"_id": "0x0500",
						"_mandatory": "true"
					}
				]
			},
			"_name": "IAS Zone",
			"_id": "0x0402",
			"_definition": "ZCL_DEVICEID_IAS_ZONE",
			"_location": "zcl_ha.h"
		},
		{
			"server": {
				"cluster": [
					{
						"_id": "0x0502",
						"_mandatory": "true"
					},
					{
						"_id": "0x0500",
						"_mandatory": "true"
					},
					{
						"_id": "0x0005",
						"_mandatory": "false"
					},
					{
						"_id": "0x0004",
						"_mandatory": "false"
					}
				]
			},
			"_name": "IAS Warning",
			"_id": "0x0403",
			"_definition": "ZCL_DEVICEID_IAS_WARNING",
			"_location": "zcl_ha.h"
		},
		{
			"server": {
				"cluster": [
					{
						"_id": "0x0000",
						"_mandatory": "true"
					},
					{
						"_id": "0x0003",
						"_mandatory": "true"
					},
					{
						"_id": "0x0004",
						"_mandatory": "true"
					},
					{
						"_id": "0x0005",
						"_mandatory": "true"
					},
					{
						"_id": "0x0006",
						"_mandatory": "true"
					},
					{
						"_id": "0x0008",
						"_mandatory": "false"
					},
					{
						"_id": "0x1000",
						"_mandatory": "false"
					}
				]
			},
			"client": {
				"cluster": [

					{
						"_id": "0x0406",
						"_mandatory": "false"
					}
				]
			},
			"_name": "On/Off Light",
			"_id": "0x0100",
			"_definition": "ZCL_DEVICEID_ON_OFF_LIGHT",
			"_location": "zcl_ha.h"
		},
		{
			"server": {
				"cluster": [
					{
						"_id": "0x0000",
						"_mandatory": "true"
					},
					{
						"_id": "0x0003",
						"_mandatory": "true"
					},
					{
						"_id": "0x0004",
						"_mandatory": "true"
					},
					{
						"_id": "0x0005",
						"_mandatory": "true"
					},
					{
						"_id": "0x0006",
						"_mandatory": "true"
					},
					{
						"_id": "0x0008",
						"_mandatory": "true"
					},
					{
						"_id": "0x1000",
						"_mandatory": "false"
					}
				]
			},
			"client": {
				"cluster": [

					{
						"_id": "0x0406",
						"_mandatory": "false"
					}
				]
			},
			"_name": "Dimmable Light",
			"_id": "0x0101",
			"_definition": "ZCL_DEVICEID_DIMMABLE_LIGHT",
			"_location": "zcl_ha.h"
		},
		{
			"server": {
				"cluster": [
					{
						"_id": "0x0000",
						"_mandatory": "true"
					},
					{
						"_id": "0x0003",
						"_mandatory": "true"
					},
					{
						"_id": "0x0004",
						"_mandatory": "true"
					},
					{
						"_id": "0x0005",
						"_mandatory": "true"
					},
					{
						"_id": "0x0006",
						"_mandatory": "true"
					},
					{
						"_id": "0x0008",
						"_mandatory": "true"
					},
					{
						"_id": "0x0300",
						"_mandatory": "true"
					},
					{
						"_id": "0x1000",
						"_mandatory": "false"
					}
				]
			},
			"client": {
				"cluster": [
					{
						"_id": "0x0406",
						"_mandatory": "false"
					}
				]
			},
			"_name": "Color Dimmable",
			"_id": "0x0102",
			"_definition": "ZCL_DEVICEID_COLOR_DIMMABLE",
			"_location": "zcl_ha.h"
		},
		{
			"server": {
				"cluster": [
					{
						"_id": "0x0000",
						"_mandatory": "true"
					},
					{
						"_id": "0x0003",
						"_mandatory": "true"
					},
					{
						"_id": "0x0007",
						"_mandatory": "false"
					}
				]
			},
			"client": {
				"cluster": [
					{
						"_id": "0x0003",
						"_mandatory": "true"
					},
					{
						"_id": "0x0006",
						"_mandatory": "true"
					},
					{
						"_id": "0x0004",
						"_mandatory": "false"
					},
					{
						"_id": "0x0005",
						"_mandatory": "false"
					}
				]
			},
			"_name": "On/Off Light Switch",
			"_id": "0x0103",
			"_definition": "ZCL_DEVICEID_ON_OFF_LIGHT_SWITCH",
			"_location": "zcl_ha.h"
		},
		{
			"server": {
				"cluster": [
					{
						"_id": "0x0000",
						"_mandatory": "true"
					},
					{
						"_id": "0x0003",
						"_mandatory": "true"
					},
					{
						"_id": "0x0007",
						"_mandatory": "false"
					}
				]
			},
			"client": {
				"cluster": [
					{
						"_id": "0x0003",
						"_mandatory": "true"
					},
					{
						"_id": "0x0006",
						"_mandatory": "true"
					},
					{
						"_id": "0x0008",
						"_mandatory": "true"
					},
					{
						"_id": "0x0004",
						"_mandatory": "false"
					},
					{
						"_id": "0x0005",
						"_mandatory": "false"
					}
				]
			},
			"_name": "Dimmer Switch",
			"_id": "0x0104",
			"_definition": "ZCL_DEVICEID_DIMMER_SWITCH",
			"_location": "zcl_ha.h"
		},
		{
			"server": {
				"cluster": [
					{
						"_id": "0x0000",
						"_mandatory": "true"
					},
					{
						"_id": "0x0003",
						"_mandatory": "true"
					},
					{
						"_id": "0x0007",
						"_mandatory": "false"
					}
				]
			},
			"client": {
				"cluster": [
					{
						"_id": "0x0003",
						"_mandatory": "true"
					},
					{
						"_id": "0x0006",
						"_mandatory": "true"
					},
					{
						"_id": "0x0008",
						"_mandatory": "true"
					},
					{
						"_id": "0x0300",
						"_mandatory": "true"
					},
					{
						"_id": "0x0004",
						"_mandatory": "false"
					},
					{
						"_id": "0x0005",
						"_mandatory": "false"
					}
				]
			},
			"_name": "Color Dimmer Switch",
			"_id": "0x0105",
			"_definition": "ZCL_DEVICEID_COLOR_DIMMER_SWITCH",
			"_location": "zcl_ha.h"
		},
		{
			"server": {
				"cluster": [
					{
						"_id": "0x0000",
						"_mandatory": "true"
					},
					{
						"_id": "0x0003",
						"_mandatory": "true"
					},
					{
						"_id": "0x0400",
						"_mandatory": "true"
					}
				]
			},
			"client": {
				"cluster": [
					{
						"_id": "0x0003",
						"_mandatory": "true"
					},
					{
						"_id": "0x0004",
						"_mandatory": "false"
					}
				]
			},
			"_name": "Light Sensor",
			"_id": "0x0106",
			"_definition": "ZCL_DEVICEID_LIGHT_SENSOR",
			"_location": "zcl_ha.h"
		},
		{
			"server": {
				"cluster": [
					{
						"_id": "0x0000",
						"_mandatory": "true"
					},
					{
						"_id": "0x0003",
						"_mandatory": "true"
					},
					{
						"_id": "0x0406",
						"_mandatory": "true"
					}
				]
			},
			"client": {
				"cluster": [
					{
						"_id": "0x0003",
						"_mandatory": "true"
					},
					{
						"_id": "0x0004",
						"_mandatory": "false"
					}
				]
			},
			"_name": "Occupancy Sensor",
			"_id": "0x0107",
			"_definition": "ZCL_DEVICEID_OCCUPANCY_SENSOR",
			"_location": "zcl_ha.h"
		},
		{
			"server": {
				"cluster": [
					{
						"_id": "0x0000",
						"_mandatory": "true"
					},
					{
						"_id": "0x0001",
						"_mandatory": "true"
					},
					{
						"_id": "0x0002",
						"_mandatory": "true"
					},
					{
						"_id": "0x0003",
						"_mandatory": "true"
					},
					{
						"_id": "0x0004",
						"_mandatory": "true"
					},
					{
						"_id": "0x0005",
						"_mandatory": "true"
					},
					{
						"_id": "0x0006",
						"_mandatory": "true"
					},
					{
						"_id": "0x0301",
						"_mandatory": "true"
					},
					{
						"_id": "0x0008",
						"_mandatory": "false"
					},
					{
						"_id": "0x0401",
						"_mandatory": "false"
					},
					{
						"_id": "0x1000",
						"_mandatory": "false"
					}
				]
			},
			"client": {
				"cluster": [
					{
						"_id": "0x0400",
						"_mandatory": "false"
					},
					{
						"_id": "0x0401",
						"_mandatory": "false"
					},
					{
						"_id": "0x0406",
						"_mandatory": "false"
					}
				]
			},
			"_name": "On/Off Ballast",
			"_id": "0x0108",
			"_definition": "ZCL_DEVICEID_ON_OFF_BALLAST",
			"_location": "zcl_ll.h"
		},
		{
			"server": {
				"cluster": [
					{
						"_id": "0x0000",
						"_mandatory": "true"
					},
					{
						"_id": "0x0001",
						"_mandatory": "true"
					},
					{
						"_id": "0x0002",
						"_mandatory": "true"
					},
					{
						"_id": "0x0003",
						"_mandatory": "true"
					},
					{
						"_id": "0x0004",
						"_mandatory": "true"
					},
					{
						"_id": "0x0005",
						"_mandatory": "true"
					},
					{
						"_id": "0x0006",
						"_mandatory": "true"
					},
					{
						"_id": "0x0008",
						"_mandatory": "true"
					},
					{
						"_id": "0x0301",
						"_mandatory": "true"
					},
					{
						"_id": "0x0401",
						"_mandatory": "false"
					},
					{
						"_id": "0x1000",
						"_mandatory": "false"
					}
				]
			},
			"client": {
				"cluster": [
					{
						"_id": "0x0400",
						"_mandatory": "false"
					},
					{
						"_id": "0x0401",
						"_mandatory": "false"
					},
					{
						"_id": "0x0406",
						"_mandatory": "false"
					}
				]
			},
			"_name": "Dimmable Ballast",
			"_id": "0x0109",
			"_definition": "ZCL_DEVICEID_DIMMABLE_BALLAST",
			"_location": "zcl_ll.h"
		},
		{
			"server": {
				"cluster": [
					{
						"_id": "0x0000",
						"_mandatory": "true"
					},
					{
						"_id": "0x0003",
						"_mandatory": "true"
					},
					{
						"_id": "0x0004",
						"_mandatory": "true"
					},
					{
						"_id": "0x0005",
						"_mandatory": "true"
					},
					{
						"_id": "0x0006",
						"_mandatory": "true"
					},
					{
						"_id": "0x0008",
						"_mandatory": "false"
					}
				]
			},
			"client": {
				"cluster": [
				]
			},
			"_name": "On/Off Plug-in Unit",
			"_id": "0x010a",
			"_definition": "ZCL_DEVICEID_ON_OFF_PLUG_IN_UNIT",
			"_location": "zcl_ll.h"
		},
		{
			"server": {
				"cluster": [
					{
						"_id": "0x0000",
						"_mandatory": "true"
					},
					{
						"_id": "0x0003",
						"_mandatory": "true"
					},
					{
						"_id": "0x0004",
						"_mandatory": "true"
					},
					{
						"_id": "0x0005",
						"_mandatory": "true"
					},
					{
						"_id": "0x0006",
						"_mandatory": "true"
					},
					{
						"_id": "0x0008",
						"_mandatory": "true"
					}
				]
			},
			"client": {
				"cluster": [
				]
			},
			"_name": "Dimmable Plug-in Unit",
			"_id": "0x010b",
			"_definition": "ZCL_DEVICEID_DIMMABLE_PLUG_IN_UNIT",
			"_location": "zcl_ll.h"
		},
		{
			"server": {
				"cluster": [
					{
						"_id": "0x0000",
						"_mandatory": "true"
					},
					{
						"_id": "0x0003",
						"_mandatory": "true"
					},
					{
						"_id": "0x0004",
						"_mandatory": "true"
					},
					{
						"_id": "0x0005",
						"_mandatory": "true"
					},
					{
						"_id": "0x0006",
						"_mandatory": "true"
					},
					{
						"_id": "0x0008",
						"_mandatory": "true"
					},
					{
						"_id": "0x0300",
						"_mandatory": "true"
					},
					{
						"_id": "0x1000",
						"_mandatory": "false"
					}
				]
			},
			"client": {
				"cluster": [
				]
			},
			"_name": "Color Temperature Light",
			"_id": "0x010c",
			"_definition": "ZCL_DEVICEID_COLOR_TEMPERATURE_LIGHT",
			"_location": "zcl_ll.h"
		},
		{
			"server": {
				"cluster": [
					{
						"_id": "0x0000",
						"_mandatory": "true"
					},
					{
						"_id": "0x0003",
						"_mandatory": "true"
					},
					{
						"_id": "0x0004",
						"_mandatory": "true"
					},
					{
						"_id": "0x0005",
						"_mandatory": "true"
					},
					{
						"_id": "0x0006",
						"_mandatory": "true"
					},
					{
						"_id": "0x0008",
						"_mandatory": "true"
					},
					{
						"_id": "0x0300",
						"_mandatory": "true"
					},
					{
						"_id": "0x1000",
						"_mandatory": "false"
					}
				]
			},
			"client": {
				"cluster": [
				]
			},
			"_name": "Extended Color Light",
			"_id": "0x010d",
			"_definition": "ZCL_DEVICEID_EXTENDED_COLOR_LIGHT",
			"_location": "zcl_ll.h"
		},
		{
			"server": {
				"cluster": [
					{
						"_id": "0x0000",
						"_mandatory": "true"
					},
					{
						"_id": "0x0003",
						"_mandatory": "true"
					},
					{
						"_id": "0x0401",
						"_mandatory": "true"
					}
				]
			},
			"client": {
				"cluster": [
					{
						"_id": "0x0003",
						"_mandatory": "true"
					},
					{
						"_id": "0x0004",
						"_mandatory": "false"
					}
				]
			},
			"_name": "Light Level Sensor",
			"_id": "0x010e",
			"_definition": "ZCL_DEVICEID_LIGHT_LEVEL_SENSOR",
			"_location": "zcl_ll.h"
		},
		{
			"server": {
				"cluster": [
					{
						"_id": "0x0000",
						"_mandatory": "true"
					},
					{
						"_id": "0x0003",
						"_mandatory": "true"
					},
					{
						"_id": "0x1000",
						"_mandatory": "false"
					}
				]
			},
			"client": {
				"cluster": [
					{
						"_id": "0x0003",
						"_mandatory": "true"
					},
					{
						"_id": "0x0006",
						"_mandatory": "true"
					},
					{
						"_id": "0x0008",
						"_mandatory": "true"
					},
					{
						"_id": "0x0300",
						"_mandatory": "true"
					},
					{
						"_id": "0x0004",
						"_mandatory": "false"
					},
					{
						"_id": "0x1000",
						"_mandatory": "false"
					}
				]
			},
			"_name": "Color Controller",
			"_id": "0x0800",
			"_definition": "ZCL_DEVICEID_COLOR_CONTROLLER",
			"_location": "zcl_ll.h"
		},
		{
			"server": {
				"cluster": [
					{
						"_id": "0x0000",
						"_mandatory": "true"
					},
					{
						"_id": "0x0003",
						"_mandatory": "true"
					},
					{
						"_id": "0x1000",
						"_mandatory": "false"
					}
				]
			},
			"client": {
				"cluster": [
					{
						"_id": "0x0003",
						"_mandatory": "true"
					},
					{
						"_id": "0x0005",
						"_mandatory": "true"
					},
					{
						"_id": "0x0006",
						"_mandatory": "true"
					},
					{
						"_id": "0x0008",
						"_mandatory": "true"
					},
					{
						"_id": "0x0300",
						"_mandatory": "true"
					},
					{
						"_id": "0x0004",
						"_mandatory": "false"
					},
					{
						"_id": "0x1000",
						"_mandatory": "false"
					}
				]
			},
			"_name": "Color Scene Controller",
			"_id": "0x0810",
			"_definition": "ZCL_DEVICEID_COLOR_SCENE_CONTROLLER",
			"_location": "zcl_ll.h"
		},
		{
			"server": {
				"cluster": [
					{
						"_id": "0x0000",
						"_mandatory": "true"
					},
					{
						"_id": "0x0003",
						"_mandatory": "true"
					},
					{
						"_id": "0x1000",
						"_mandatory": "false"
					}
				]
			},
			"client": {
				"cluster": [
					{
						"_id": "0x0003",
						"_mandatory": "true"
					},
					{
						"_id": "0x0006",
						"_mandatory": "true"
					},
					{
						"_id": "0x0008",
						"_mandatory": "true"
					},
					{
						"_id": "0x0004",
						"_mandatory": "false"
					},
					{
						"_id": "0x1000",
						"_mandatory": "false"
					}
				]
			},
			"_name": "Non-Color Controller",
			"_id": "0x0820",
			"_definition": "ZCL_DEVICEID_NON_COLOR_CONTROLLER",
			"_location": "zcl_ll.h"
		},
		{
			"server": {
				"cluster": [
					{
						"_id": "0x0000",
						"_mandatory": "true"
					},
					{
						"_id": "0x0003",
						"_mandatory": "true"
					},
					{
						"_id": "0x1000",
						"_mandatory": "false"
					}
				]
			},
			"client": {
				"cluster": [
					{
						"_id": "0x0003",
						"_mandatory": "true"
					},
					{
						"_id": "0x0005",
						"_mandatory": "true"
					},
					{
						"_id": "0x0006",
						"_mandatory": "true"
					},
					{
						"_id": "0x0008",
						"_mandatory": "true"
					},
					{
						"_id": "0x0004",
						"_mandatory": "false"
					},
					{
						"_id": "0x1000",
						"_mandatory": "false"
					}
				]
			},
			"_name": "Non-Color Scene Controller",
			"_id": "0x0830",
			"_definition": "ZCL_DEVICEID_NON_COLOR_SCENE_CONTROLLER",
			"_location": "zcl_ll.h"
		},
		{
			"server": {
				"cluster": [
					{
						"_id": "0x0000",
						"_mandatory": "true"
					},
					{
						"_id": "0x0003",
						"_mandatory": "true"
					},
					{
						"_id": "0x1000",
						"_mandatory": "false"
					}
				]
			},
			"client": {
				"cluster": [
					{
						"_id": "0x0003",
						"_mandatory": "true"
					},
					{
						"_id": "0x0004",
						"_mandatory": "true"
					},
					{
						"_id": "0x0005",
						"_mandatory": "true"
					},
					{
						"_id": "0x0006",
						"_mandatory": "true"
					},
					{
						"_id": "0x0008",
						"_mandatory": "true"
					},
					{
						"_id": "0x0300",
						"_mandatory": "true"
					},
					{
						"_id": "0x0400",
						"_mandatory": "false"
					},
					{
						"_id": "0x0401",
						"_mandatory": "false"
					},
					{
						"_id": "0x0406",
						"_mandatory": "false"
					},
					{
						"_id": "0x1000",
						"_mandatory": "false"
					}
				]
			},
			"_name": "Control Bridge",
			"_id": "0x0840",
			"_definition": "ZCL_DEVICEID_CONTROL_BRIDGE",
			"_location": "zcl_ll.h"
		},
		{
			"server": {
				"cluster": [
					{
						"_id": "0x0000",
						"_mandatory": "true"
					},
					{
						"_id": "0x0003",
						"_mandatory": "true"
					},
					{
						"_id": "0x1000",
						"_mandatory": "false"
					}
				]
			},
			"client": {
				"cluster": [
					{
						"_id": "0x0003",
						"_mandatory": "true"
					},
					{
						"_id": "0x0006",
						"_mandatory": "true"
					},
					{
						"_id": "0x0004",
						"_mandatory": "false"
					},
					{
						"_id": "0x0005",
						"_mandatory": "false"
					},
					{
						"_id": "0x0008",
						"_mandatory": "false"
					},
					{
						"_id": "0x0300",
						"_mandatory": "false"
					},
					{
						"_id": "0x1000",
						"_mandatory": "false"
					}
				]
			},
			"_name": "On/Off Sensor",
			"_id": "0x0850",
			"_definition": "ZCL_DEVICEID_ON_OFF_SENSOR",
			"_location": "zcl_ll.h"
		}	]
};

exports = {
	ZDL: ZDL,
};
