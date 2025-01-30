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

const ZCL =
{
	"clusters": [
		{
			"server": {
				"attribute": [
					{
						"_id": "0xfffd",
						"_name": "ClusterRevision",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_CLUSTER_REVISION",
						"_location": "zcl.h",
						"_clusterID": "0x0009",
						"_default": "1",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0xfffe",
						"_name": "AttributeReportingStatus",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_ATTRIBUTE_REPORTING_STATUS",
						"_location": "zcl.h",
						"_clusterID": "0x0009",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0000",
						"_name": "AlarmCount",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_ALARMS_ALARM_COUNT ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0009",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					}
				],
				"command": [
					{
						"_id": "0x00",
						"_name": "ResetAlarm",
						"_definition": "COMMAND_ALARMS_RESET_ALARM",
						"_location": "zcl_general.h",
						"_clusterID": "0x0009",
						"_required": "true"
					},
					{
						"_id": "0x01",
						"_name": "ResetAllAlarms",
						"_definition": "COMMAND_ALARMS_RESET_ALL_ALARMS",
						"_location": "zcl_general.h",
						"_clusterID": "0x0009",
						"_required": "true"
					},
					{
						"_id": "0x02",
						"_name": "GetAlarm",
						"_definition": "COMMAND_ALARMS_GET_ALARM",
						"_location": "zcl_general.h",
						"_clusterID": "0x0009",
						"_required": "false"
					},
					{
						"_id": "0x03",
						"_name": "ResetAlarmLog",
						"_definition": "COMMAND_ALARMS_RESET_ALARM_LOG",
						"_location": "zcl_general.h",
						"_clusterID": "0x0009",
						"_required": "false"
					}
				]
			},
			"client": {
				"attribute": [
					{
						"_id": "0xfffd",
						"_name": "ClusterRevision",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_CLUSTER_REVISION",
						"_location": "zcl.h",
						"_clusterID": "0x0009",
						"_default": "1",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0xfffe",
						"_name": "AttributeReportingStatus",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_ATTRIBUTE_REPORTING_STATUS",
						"_location": "zcl.h",
						"_clusterID": "0x0009",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					}
				],
				"command": [
					{
						"_id": "0x00",
						"_name": "Alarm",
						"_definition": "COMMAND_ALARMS_ALARM",
						"_location": "zcl_general.h",
						"_clusterID": "0x0009",
						"_required": "true"
					},
					{
						"_id": "0x01",
						"_name": "GetAlarmResponse",
						"_definition": "COMMAND_ALARMS_GET_ALARM_RESPONSE",
						"_location": "zcl_general.h",
						"_clusterID": "0x0009",
						"_required": "false"
					}
				]
			},
			"appcallback": {
				"callbacks": [
					{
						"_name": "BasicReset",
						"_returnType": "void",
						"_arguments": "( void )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Reset to Factory Defaults"
							},
							{
								"_comment": "/// command. On receipt of this command, the device resets all the attributes"
							},
							{
								"_comment": "/// of all its clusters to their factory defaults."
							}
						]
					},
					{
						"_name": "Identify",
						"_returnType": "void",
						"_arguments": "( zclIdentify_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Identify command."
							}
						]
					},
					{
						"_name": "IdentifyQuery",
						"_returnType": "void",
						"_arguments": "( zclIdentifyQuery_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Identify Query command."
							}
						]
					},
					{
						"_name": "IdentifyQueryRsp",
						"_returnType": "void",
						"_arguments": "( zclIdentifyQueryRsp_t *pRsp )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Identify Query Response command."
							}
						]
					},
					{
						"_name": "IdentifyTriggerEffect",
						"_returnType": "void",
						"_arguments": "( zclIdentifyTriggerEffect_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Identify Trigger Effect command."
							}
						]
					},
					{
						"_name": "#ifdef ZCL_ON_OFF",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "OnOff",
						"_returnType": "void",
						"_arguments": "( uint8_t cmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming On, Off or Toggle command."
							}
						]
					},
					{
						"_name": "OnOff_OffWithEffect",
						"_returnType": "void",
						"_arguments": "( zclOffWithEffect_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Off with Effect"
							}
						]
					},
					{
						"_name": "OnOff_OnWithRecallGlobalScene",
						"_returnType": "void",
						"_arguments": "( void )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming On with Recall Global Scene command."
							}
						]
					},
					{
						"_name": "OnOff_OnWithTimedOff",
						"_returnType": "void",
						"_arguments": "( zclOnWithTimedOff_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming On with Timed Off."
							}
						]
					},
					{
						"_name": "#endif",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "#ifdef ZCL_LEVEL_CTRL",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "LevelControlMoveToLevel",
						"_returnType": "void",
						"_arguments": "( zclLCMoveToLevel_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Level Control - Move to Level command"
							}
						]
					},
					{
						"_name": "LevelControlMove",
						"_returnType": "void",
						"_arguments": "( zclLCMove_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Level Control - Move command"
							}
						]
					},
					{
						"_name": "LevelControlStep",
						"_returnType": "void",
						"_arguments": "( zclLCStep_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Level Control - Step command"
							}
						]
					},
					{
						"_name": "LevelControlStop",
						"_returnType": "void",
						"_arguments": "( zclLCStop_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Level Control - Stop command"
							}
						]
					},
					{
						"_name": "LevelControlMoveFreq",
						"_returnType": "void",
						"_arguments": "( zclLCMoveFreq_t *pCmd)",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Level Control - Move to Closest Frequency command"
							}
						]
					},
					{
						"_name": "#endif",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "#ifdef ZCL_GROUPS",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "GroupRsp",
						"_returnType": "void",
						"_arguments": "( zclGroupRsp_t *pRsp )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an received Group Response message."
							},
							{
								"_comment": "/// This means that this app sent the request message."
							}
						]
					},
					{
						"_name": "#endif",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "#ifdef ZCL_SCENES",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "SceneStoreReq",
						"_returnType": "uint8_t",
						"_arguments": "( zclSceneReq_t *pReq )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Scene Store request."
							},
							{
								"_comment": "/// The app will fill in the 'extField' with what is needed to restore its"
							},
							{
								"_comment": "/// current settings. Before overwriting the content of the scene,"
							},
							{
								"_comment": "/// App needs to verify if there was a change with the previous configuration."
							},
							{
								"_comment": "/// App function returns TRUE if there was a change, otherwise returns FALSE."
							}
						]
					},
					{
						"_name": "SceneRecallReq",
						"_returnType": "void",
						"_arguments": "( zclSceneReq_t *pReq )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Scene Recall request"
							},
							{
								"_comment": "/// The app will use what's in the 'extField' to restore to these settings."
							}
						]
					},
					{
						"_name": "SceneRsp",
						"_returnType": "void",
						"_arguments": "( zclSceneRsp_t *pRsp )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Scene responses. This means"
							},
							{
								"_comment": "/// that this app sent the request for this response."
							}
						]
					},
					{
						"_name": "#endif",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "#ifdef ZCL_ALARMS",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "Alarm",
						"_returnType": "void",
						"_arguments": "( uint8_t direction, zclAlarm_t *pAlarm )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Alarm request or response command."
							}
						]
					},
					{
						"_name": "#endif",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "#ifdef SE_UK_EXT",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "GetEventLog",
						"_returnType": "void",
						"_arguments": "( uint8_t srcEP, afAddrType_t *srcAddr, zclGetEventLog_t *pEventLog, uint8_t seqNum )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Alarm Get Event Log command."
							}
						]
					},
					{
						"_name": "PublishEventLog",
						"_returnType": "void",
						"_arguments": "( afAddrType_t *srcAddr, zclPublishEventLog_t *pEventLog )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Alarm Publish Event Log command."
							}
						]
					},
					{
						"_name": "#endif",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "Location",
						"_returnType": "void",
						"_arguments": "( zclLocation_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to to process an incoming RSSI Location command."
							}
						]
					},
					{
						"_name": "LocationRsp",
						"_returnType": "void",
						"_arguments": "( zclLocationRsp_t *pRsp )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming RSSI Location response command."
							},
							{
								"_comment": "/// This means  that this app sent the request for this response."
							}
						]
					}
				],
				"_name": "zclGeneral_AppCallbacks_t",
				"_register": "zclGeneral_RegisterCmdCallbacks"
			},
			"_id": "0x0009",
			"_name": "Alarms",
			"_definition": "ZCL_CLUSTER_ID_GENERAL_ALARMS",
			"_primary_transaction": "2",
			"_class": "general",
			"_flag": "ZCL_ALARMS",
			"_location": "zcl_general.h"
		},
		{
			"server": {
				"attribute": [
					{
						"_id": "0xfffd",
						"_name": "ClusterRevision",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_CLUSTER_REVISION",
						"_location": "zcl.h",
						"_clusterID": "0x0301",
						"_default": "1",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0xfffe",
						"_name": "AttributeReportingStatus",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_ATTRIBUTE_REPORTING_STATUS",
						"_location": "zcl.h",
						"_clusterID": "0x0301",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0000",
						"_name": "PhysicalMinLevel",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_BALLAST_CONFIGURATION_PHYSICAL_MIN_LEVEL ",
						"_location": "zcl_lighting.h",
						"_clusterID": "0x0301",
						"_default": "1",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0x0001",
						"_name": "PhysicalMaxLevel",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_BALLAST_CONFIGURATION_PHYSICAL_MAX_LEVEL ",
						"_location": "zcl_lighting.h",
						"_clusterID": "0x0301",
						"_default": "254",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0x0002",
						"_name": "BallastStatus",
						"_type": "map8",
						"_zigbeetype": "ZCL_DATATYPE_BITMAP8",
						"_definition": "ATTRID_BALLAST_CONFIGURATION_BALLAST_STATUS ",
						"_location": "zcl_lighting.h",
						"_clusterID": "0x0301",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0010",
						"_name": "MinLevel",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_BALLAST_CONFIGURATION_MIN_LEVEL ",
						"_location": "zcl_lighting.h",
						"_clusterID": "0x0301",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0x0011",
						"_name": "MaxLevel",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_BALLAST_CONFIGURATION_MAX_LEVEL ",
						"_location": "zcl_lighting.h",
						"_clusterID": "0x0301",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0x0012",
						"_name": "PowerOnLevel",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_BALLAST_CONFIGURATION_POWER_ON_LEVEL ",
						"_location": "zcl_lighting.h",
						"_clusterID": "0x0301",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0013",
						"_name": "PowerOnFadeTime",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_BALLAST_CONFIGURATION_POWER_ON_FADE_TIME ",
						"_location": "zcl_lighting.h",
						"_clusterID": "0x0301",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0014",
						"_name": "IntrinsicBallastFactor",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_BALLAST_CONFIGURATION_INTRINSIC_BALLAST_FACTOR ",
						"_location": "zcl_lighting.h",
						"_clusterID": "0x0301",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0015",
						"_name": "BallastFactorAdjustment",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_BALLAST_CONFIGURATION_BALLAST_FACTOR_ADJUSTMENT ",
						"_location": "zcl_lighting.h",
						"_clusterID": "0x0301",
						"_default": "255",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0020",
						"_name": "LampQuantity",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_BALLAST_CONFIGURATION_LAMP_QUANTITY ",
						"_location": "zcl_lighting.h",
						"_clusterID": "0x0301",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0030",
						"_name": "LampType",
						"_type": "uint8[]",
						"_zigbeetype": "ZCL_DATATYPE_CHAR_STR",
						"_definition": "ATTRID_BALLAST_CONFIGURATION_LAMP_TYPE ",
						"_location": "zcl_lighting.h",
						"_clusterID": "0x0301",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0031",
						"_name": "LampManufacturer",
						"_type": "uint8[]",
						"_zigbeetype": "ZCL_DATATYPE_CHAR_STR",
						"_definition": "ATTRID_BALLAST_CONFIGURATION_LAMP_MANUFACTURER ",
						"_location": "zcl_lighting.h",
						"_clusterID": "0x0301",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0032",
						"_name": "LampRatedHours",
						"_type": "uint24",
						"_zigbeetype": "ZCL_DATATYPE_UINT24",
						"_definition": "ATTRID_BALLAST_CONFIGURATION_LAMP_RATED_HOURS ",
						"_location": "zcl_lighting.h",
						"_clusterID": "0x0301",
						"_default": "16777215",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0033",
						"_name": "LampBurnHours",
						"_type": "uint24",
						"_zigbeetype": "ZCL_DATATYPE_UINT24",
						"_definition": "ATTRID_BALLAST_CONFIGURATION_LAMP_BURN_HOURS ",
						"_location": "zcl_lighting.h",
						"_clusterID": "0x0301",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0034",
						"_name": "LampAlarmMode",
						"_type": "map8",
						"_zigbeetype": "ZCL_DATATYPE_BITMAP8",
						"_definition": "ATTRID_BALLAST_CONFIGURATION_LAMP_ALARM_MODE ",
						"_location": "zcl_lighting.h",
						"_clusterID": "0x0301",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0035",
						"_name": "LampBurnHoursTripPoint",
						"_type": "uint24",
						"_zigbeetype": "ZCL_DATATYPE_UINT24",
						"_definition": "ATTRID_BALLAST_CONFIGURATION_LAMP_BURN_HOURS_TRIP_POINT ",
						"_location": "zcl_lighting.h",
						"_clusterID": "0x0301",
						"_default": "16777215",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					}
				]
			},
			"client": {
				"attribute": [
					{
						"_id": "0xfffd",
						"_name": "ClusterRevision",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_CLUSTER_REVISION",
						"_location": "zcl.h",
						"_clusterID": "0x0301",
						"_default": "1",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0xfffe",
						"_name": "AttributeReportingStatus",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_ATTRIBUTE_REPORTING_STATUS",
						"_location": "zcl.h",
						"_clusterID": "0x0301",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					}
				],
			},
			"appcallback": {
				"callbacks": [
					{
						"_name": "ColorControl_MoveToHue",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclCCMoveToHue_t *pCmd )",
						"comments": [
							{
								"_comment": "// This callback is called to process a Move To Hue command"
							}
						]
					},
					{
						"_name": "ColorControl_MoveHue",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclCCMoveHue_t *pCmd )",
						"comments": [
							{
								"_comment": "// This callback is called to process a Move Hue command"
							}
						]
					},
					{
						"_name": "ColorControl_StepHue",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclCCStepHue_t *pCmd )",
						"comments": [
							{
								"_comment": "// This callback is called to process a Step Hue command"
							}
						]
					},
					{
						"_name": "ColorControl_MoveToSaturation",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclCCMoveToSaturation_t *pCmd )",
						"comments": [
							{
								"_comment": "// This callback is called to process a Move To Saturation command"
							}
						]
					},
					{
						"_name": "ColorControl_MoveSaturation",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclCCMoveSaturation_t *pCmd )",
						"comments": [
							{
								"_comment": "// This callback is called to process a Move Saturation command"
							}
						]
					},
					{
						"_name": "ColorControl_StepSaturation",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclCCStepSaturation_t *pCmd )",
						"comments": [
							{
								"_comment": "// This callback is called to process a Step Saturation command"
							}
						]
					},
					{
						"_name": "ColorControl_MoveToHueAndSaturation",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclCCMoveToHueAndSaturation_t *pCmd )",
						"comments": [
							{
								"_comment": "// This callback is called to process a Move to Hue and Saturation command"
							}
						]
					},
					{
						"_name": "ColorControl_MoveToColor",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclCCMoveToColor_t *pCmd )",
						"comments": [
							{
								"_comment": "// This callback is called to process a Move to Color command"
							}
						]
					},
					{
						"_name": "ColorControl_MoveColor",
						"_returnType": "void",
						"_arguments": "( zclCCMoveColor_t *pCmd )",
						"comments": [
							{
								"_comment": "// This callback is called to process a Move Color command"
							}
						]
					},
					{
						"_name": "ColorControl_StepColor",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclCCStepColor_t *pCmd )",
						"comments": [
							{
								"_comment": "// This callback is called to process a Step Color command"
							}
						]
					},
					{
						"_name": "ColorControl_MoveToColorTemperature",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclCCMoveToColorTemperature_t *pCmd )",
						"comments": [
							{
								"_comment": "// This callback is called to process a Move to Color Temperature command"
							}
						]
					},
					{
						"_name": "ColorControl_EnhancedMoveToHue",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclCCEnhancedMoveToHue_t *pCmd )",
						"comments": [
							{
								"_comment": "// This callback is called to process an Enhanced Move To Hue command"
							}
						]
					},
					{
						"_name": "ColorControl_EnhancedMoveHue",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclCCEnhancedMoveHue_t *pCmd )",
						"comments": [
							{
								"_comment": "// This callback is called to process an Enhanced Move Hue command"
							}
						]
					},
					{
						"_name": "ColorControl_EnhancedStepHue",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclCCEnhancedStepHue_t *pCmd )",
						"comments": [
							{
								"_comment": "// This callback is called to process an Enhanced Step Hue command"
							}
						]
					},
					{
						"_name": "ColorControl_EnhancedMoveToHueAndSaturation",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclCCEnhancedMoveToHueAndSaturation_t *pCmd )",
						"comments": [
							{
								"_comment": "// This callback is called to process an Enhanced Move to Hue and Saturation command"
							}
						]
					},
					{
						"_name": "ColorControl_ColorLoopSet",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclCCColorLoopSet_t *pCmd )",
						"comments": [
							{
								"_comment": "// This callback is called to process an Color Loop Set command"
							}
						]
					},
					{
						"_name": "ColorControl_StopMoveStep",
						"_returnType": "ZStatus_t",
						"_arguments": "( void )",
						"comments": [
							{
								"_comment": "// This callback is called to process a Stop Step Move command"
							}
						]
					},
					{
						"_name": "ColorControl_MoveColorTemperature",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclCCMoveColorTemperature_t *pCmd )"
					},
					{
						"_name": "ColorControl_StepColorTemperature",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclCCStepColorTemperature_t *pCmd )"
					}
				],
				"_name": "zclLighting_AppCallbacks_t",
				"_register": "zclLighting_RegisterCmdCallbacks"
			},
			"_id": "0x0301",
			"_name": "BallastConfiguration",
			"_definition": "ZCL_CLUSTER_ID_LIGHTING_BALLAST_CONFIG",
			"_primary_transaction": "1",
			"_class": "lighting",
			"_flag": "ZCL_LIGHTING",
			"_location": "zcl_lighting.h"
		},
		{
			"server": {
				"attribute": [
					{
						"_id": "0xfffd",
						"_name": "ClusterRevision",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_CLUSTER_REVISION",
						"_location": "zcl.h",
						"_clusterID": "0x0000",
						"_default": "1",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0xfffe",
						"_name": "AttributeReportingStatus",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_ATTRIBUTE_REPORTING_STATUS",
						"_location": "zcl.h",
						"_clusterID": "0x0000",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0000",
						"_name": "ZCLVersion",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_BASIC_ZCL_VERSION ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0000",
						"_default": "3",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0x0001",
						"_name": "ApplicationVersion",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_BASIC_APPLICATION_VERSION ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0000",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0002",
						"_name": "StackVersion",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_BASIC_STACK_VERSION ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0000",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0003",
						"_name": "HWVersion",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_BASIC_HW_VERSION ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0000",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0004",
						"_name": "ManufacturerName",
						"_type": "uint8[]",
						"_zigbeetype": "ZCL_DATATYPE_CHAR_STR",
						"_definition": "ATTRID_BASIC_MANUFACTURER_NAME ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0000",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0005",
						"_name": "ModelIdentifier",
						"_type": "uint8[]",
						"_zigbeetype": "ZCL_DATATYPE_CHAR_STR",
						"_definition": "ATTRID_BASIC_MODEL_IDENTIFIER ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0000",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0006",
						"_name": "DateCode",
						"_type": "uint8[]",
						"_zigbeetype": "ZCL_DATATYPE_CHAR_STR",
						"_definition": "ATTRID_BASIC_DATE_CODE ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0000",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0007",
						"_name": "PowerSource",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_BASIC_POWER_SOURCE ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0000",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0x0008",
						"_name": "GenericDevice-Class",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_BASIC_GENERIC_DEVICE_CLASS ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0000",
						"_default": "255",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0009",
						"_name": "GenericDevice-Type",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_BASIC_GENERIC_DEVICE_TYPE ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0000",
						"_default": "255",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x000a",
						"_name": "ProductCode",
						"_type": "uint8[]",
						"_zigbeetype": "ZCL_DATATYPE_OCTET_STR",
						"_definition": "ATTRID_BASIC_PRODUCT_CODE ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0000",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x000b",
						"_name": "ProductURL",
						"_type": "uint8[]",
						"_zigbeetype": "ZCL_DATATYPE_CHAR_STR",
						"_definition": "ATTRID_BASIC_PRODUCT_URL ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0000",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x000c",
						"_name": "ManufacturerVersionDetails",
						"_type": "uint8[]",
						"_zigbeetype": "ZCL_DATATYPE_CHAR_STR",
						"_definition": "ATTRID_BASIC_MANUFACTURER_VERSION_DETAILS ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0000",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x000d",
						"_name": "SerialNumber",
						"_type": "uint8[]",
						"_zigbeetype": "ZCL_DATATYPE_CHAR_STR",
						"_definition": "ATTRID_BASIC_SERIAL_NUMBER ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0000",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x000e",
						"_name": "ProductLabel",
						"_type": "uint8[]",
						"_zigbeetype": "ZCL_DATATYPE_CHAR_STR",
						"_definition": "ATTRID_BASIC_PRODUCT_LABEL ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0000",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0010",
						"_name": "LocationDescription",
						"_type": "uint8[]",
						"_zigbeetype": "ZCL_DATATYPE_CHAR_STR",
						"_definition": "ATTRID_BASIC_LOCATION_DESCRIPTION ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0000",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0011",
						"_name": "PhysicalEnvironment",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_BASIC_PHYSICAL_ENVIRONMENT ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0000",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0012",
						"_name": "DeviceEnabled",
						"_type": "bool",
						"_zigbeetype": "ZCL_DATATYPE_BOOLEAN",
						"_definition": "ATTRID_BASIC_DEVICE_ENABLED ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0000",
						"_default": "1",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0013",
						"_name": "AlarmMask",
						"_type": "map8",
						"_zigbeetype": "ZCL_DATATYPE_BITMAP8",
						"_definition": "ATTRID_BASIC_ALARM_MASK ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0000",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0014",
						"_name": "DisableLocalConfig",
						"_type": "map8",
						"_zigbeetype": "ZCL_DATATYPE_BITMAP8",
						"_definition": "ATTRID_BASIC_DISABLE_LOCAL_CONFIG ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0000",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x4000",
						"_name": "SWBuildID",
						"_type": "uint8[]",
						"_zigbeetype": "ZCL_DATATYPE_CHAR_STR",
						"_definition": "ATTRID_BASIC_SW_BUILD_ID ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0000",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					}
				],
				"command": [
					{
						"_id": "0x00",
						"_name": "ResetToFactoryDefaults",
						"_definition": "COMMAND_BASIC_RESET_TO_FACTORY_DEFAULTS",
						"_location": "zcl_general.h",
						"_callback": "BasicReset",
						"_clusterID": "0x0000",
						"_required": "false"
					}
				]
			},
			"client": {
				"attribute": [
					{
						"_id": "0xfffd",
						"_name": "ClusterRevision",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_CLUSTER_REVISION",
						"_location": "zcl.h",
						"_clusterID": "0x0000",
						"_default": "1",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0xfffe",
						"_name": "AttributeReportingStatus",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_ATTRIBUTE_REPORTING_STATUS",
						"_location": "zcl.h",
						"_clusterID": "0x0000",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					}
				],
			},
			"appcallback": {
				"callbacks": [
					{
						"_name": "BasicReset",
						"_returnType": "void",
						"_arguments": "( void )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Reset to Factory Defaults"
							},
							{
								"_comment": "/// command. On receipt of this command, the device resets all the attributes"
							},
							{
								"_comment": "/// of all its clusters to their factory defaults."
							}
						]
					},
					{
						"_name": "Identify",
						"_returnType": "void",
						"_arguments": "( zclIdentify_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Identify command."
							}
						]
					},
					{
						"_name": "IdentifyQuery",
						"_returnType": "void",
						"_arguments": "( zclIdentifyQuery_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Identify Query command."
							}
						]
					},
					{
						"_name": "IdentifyQueryRsp",
						"_returnType": "void",
						"_arguments": "( zclIdentifyQueryRsp_t *pRsp )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Identify Query Response command."
							}
						]
					},
					{
						"_name": "IdentifyTriggerEffect",
						"_returnType": "void",
						"_arguments": "( zclIdentifyTriggerEffect_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Identify Trigger Effect command."
							}
						]
					},
					{
						"_name": "#ifdef ZCL_ON_OFF",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "OnOff",
						"_returnType": "void",
						"_arguments": "( uint8_t cmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming On, Off or Toggle command."
							}
						]
					},
					{
						"_name": "OnOff_OffWithEffect",
						"_returnType": "void",
						"_arguments": "( zclOffWithEffect_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Off with Effect"
							}
						]
					},
					{
						"_name": "OnOff_OnWithRecallGlobalScene",
						"_returnType": "void",
						"_arguments": "( void )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming On with Recall Global Scene command."
							}
						]
					},
					{
						"_name": "OnOff_OnWithTimedOff",
						"_returnType": "void",
						"_arguments": "( zclOnWithTimedOff_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming On with Timed Off."
							}
						]
					},
					{
						"_name": "#endif",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "#ifdef ZCL_LEVEL_CTRL",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "LevelControlMoveToLevel",
						"_returnType": "void",
						"_arguments": "( zclLCMoveToLevel_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Level Control - Move to Level command"
							}
						]
					},
					{
						"_name": "LevelControlMove",
						"_returnType": "void",
						"_arguments": "( zclLCMove_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Level Control - Move command"
							}
						]
					},
					{
						"_name": "LevelControlStep",
						"_returnType": "void",
						"_arguments": "( zclLCStep_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Level Control - Step command"
							}
						]
					},
					{
						"_name": "LevelControlStop",
						"_returnType": "void",
						"_arguments": "( zclLCStop_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Level Control - Stop command"
							}
						]
					},
					{
						"_name": "LevelControlMoveFreq",
						"_returnType": "void",
						"_arguments": "( zclLCMoveFreq_t *pCmd)",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Level Control - Move to Closest Frequency command"
							}
						]
					},
					{
						"_name": "#endif",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "#ifdef ZCL_GROUPS",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "GroupRsp",
						"_returnType": "void",
						"_arguments": "( zclGroupRsp_t *pRsp )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an received Group Response message."
							},
							{
								"_comment": "/// This means that this app sent the request message."
							}
						]
					},
					{
						"_name": "#endif",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "#ifdef ZCL_SCENES",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "SceneStoreReq",
						"_returnType": "uint8_t",
						"_arguments": "( zclSceneReq_t *pReq )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Scene Store request."
							},
							{
								"_comment": "/// The app will fill in the 'extField' with what is needed to restore its"
							},
							{
								"_comment": "/// current settings. Before overwriting the content of the scene,"
							},
							{
								"_comment": "/// App needs to verify if there was a change with the previous configuration."
							},
							{
								"_comment": "/// App function returns TRUE if there was a change, otherwise returns FALSE."
							}
						]
					},
					{
						"_name": "SceneRecallReq",
						"_returnType": "void",
						"_arguments": "( zclSceneReq_t *pReq )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Scene Recall request"
							},
							{
								"_comment": "/// The app will use what's in the 'extField' to restore to these settings."
							}
						]
					},
					{
						"_name": "SceneRsp",
						"_returnType": "void",
						"_arguments": "( zclSceneRsp_t *pRsp )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Scene responses. This means"
							},
							{
								"_comment": "/// that this app sent the request for this response."
							}
						]
					},
					{
						"_name": "#endif",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "#ifdef ZCL_ALARMS",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "Alarm",
						"_returnType": "void",
						"_arguments": "( uint8_t direction, zclAlarm_t *pAlarm )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Alarm request or response command."
							}
						]
					},
					{
						"_name": "#endif",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "#ifdef SE_UK_EXT",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "GetEventLog",
						"_returnType": "void",
						"_arguments": "( uint8_t srcEP, afAddrType_t *srcAddr, zclGetEventLog_t *pEventLog, uint8_t seqNum )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Alarm Get Event Log command."
							}
						]
					},
					{
						"_name": "PublishEventLog",
						"_returnType": "void",
						"_arguments": "( afAddrType_t *srcAddr, zclPublishEventLog_t *pEventLog )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Alarm Publish Event Log command."
							}
						]
					},
					{
						"_name": "#endif",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "Location",
						"_returnType": "void",
						"_arguments": "( zclLocation_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to to process an incoming RSSI Location command."
							}
						]
					},
					{
						"_name": "LocationRsp",
						"_returnType": "void",
						"_arguments": "( zclLocationRsp_t *pRsp )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming RSSI Location response command."
							},
							{
								"_comment": "/// This means  that this app sent the request for this response."
							}
						]
					}
				],
				"_name": "zclGeneral_AppCallbacks_t",
				"_register": "zclGeneral_RegisterCmdCallbacks"
			},
			"_id": "0x0000",
			"_name": "Basic",
			"_definition": "ZCL_CLUSTER_ID_GENERAL_BASIC",
			"_primary_transaction": "0",
			"_class": "general",
			"_flag": "ZCL_BASIC",
			"_location": "zcl_general.h"
		},
		{
			"server": {
				"attribute": [
					{
						"_id": "0xfffd",
						"_name": "ClusterRevision",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_CLUSTER_REVISION",
						"_location": "zcl.h",
						"_clusterID": "0x0300",
						"_default": "1",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0xfffe",
						"_name": "AttributeReportingStatus",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_ATTRIBUTE_REPORTING_STATUS",
						"_location": "zcl.h",
						"_clusterID": "0x0300",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0000",
						"_name": "CurrentHue",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_COLOR_CONTROL_CURRENT_HUE ",
						"_location": "zcl_lighting.h",
						"_clusterID": "0x0300",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x0001",
						"_name": "CurrentSaturation",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_COLOR_CONTROL_CURRENT_SATURATION ",
						"_location": "zcl_lighting.h",
						"_clusterID": "0x0300",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x0002",
						"_name": "RemainingTime",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_COLOR_CONTROL_REMAINING_TIME ",
						"_location": "zcl_lighting.h",
						"_clusterID": "0x0300",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0003",
						"_name": "CurrentX",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_COLOR_CONTROL_CURRENT_X ",
						"_location": "zcl_lighting.h",
						"_clusterID": "0x0300",
						"_default": "24939",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x0004",
						"_name": "CurrentY",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_COLOR_CONTROL_CURRENT_Y ",
						"_location": "zcl_lighting.h",
						"_clusterID": "0x0300",
						"_default": "24701",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x0005",
						"_name": "DriftCompensation",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_COLOR_CONTROL_DRIFT_COMPENSATION ",
						"_location": "zcl_lighting.h",
						"_clusterID": "0x0300",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0006",
						"_name": "CompensationText",
						"_type": "uint8[]",
						"_zigbeetype": "ZCL_DATATYPE_CHAR_STR",
						"_definition": "ATTRID_COLOR_CONTROL_COMPENSATION_TEXT ",
						"_location": "zcl_lighting.h",
						"_clusterID": "0x0300",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0007",
						"_name": "ColorTemperatureMireds",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_COLOR_CONTROL_COLOR_TEMPERATURE_MIREDS ",
						"_location": "zcl_lighting.h",
						"_clusterID": "0x0300",
						"_default": "250",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x0008",
						"_name": "ColorMode",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_COLOR_CONTROL_COLOR_MODE ",
						"_location": "zcl_lighting.h",
						"_clusterID": "0x0300",
						"_default": "1",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0x000f",
						"_name": "Options",
						"_type": "CCColorOptions",
						"_zigbeetype": "ZCL_DATATYPE_BITMAP8",
						"_definition": "ATTRID_COLOR_CONTROL_OPTIONS ",
						"_location": "zcl_lighting.h",
						"_clusterID": "0x0300",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0x4000",
						"_name": "EnhancedCurrentHue",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_COLOR_CONTROL_ENHANCED_CURRENT_HUE ",
						"_location": "zcl_lighting.h",
						"_clusterID": "0x0300",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x4001",
						"_name": "EnhancedColorMode",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_COLOR_CONTROL_ENHANCED_COLOR_MODE ",
						"_location": "zcl_lighting.h",
						"_clusterID": "0x0300",
						"_default": "1",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0x4002",
						"_name": "ColorLoopActive",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_COLOR_CONTROL_COLOR_LOOP_ACTIVE ",
						"_location": "zcl_lighting.h",
						"_clusterID": "0x0300",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x4003",
						"_name": "ColorLoopDirection",
						"_type": "CCColorLoopDirection",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_COLOR_CONTROL_COLOR_LOOP_DIRECTION ",
						"_location": "zcl_lighting.h",
						"_clusterID": "0x0300",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x4004",
						"_name": "ColorLoopTime",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_COLOR_CONTROL_COLOR_LOOP_TIME ",
						"_location": "zcl_lighting.h",
						"_clusterID": "0x0300",
						"_default": "25",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x4005",
						"_name": "ColorLoopStartEnhancedHue",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_COLOR_CONTROL_COLOR_LOOP_START_ENHANCED_HUE ",
						"_location": "zcl_lighting.h",
						"_clusterID": "0x0300",
						"_default": "8960",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x4006",
						"_name": "ColorLoopStoredEnhancedHue",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_COLOR_CONTROL_COLOR_LOOP_STORED_ENHANCED_HUE ",
						"_location": "zcl_lighting.h",
						"_clusterID": "0x0300",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x400a",
						"_name": "ColorCapabilities",
						"_type": "map16",
						"_zigbeetype": "ZCL_DATATYPE_BITMAP16",
						"_definition": "ATTRID_COLOR_CONTROL_COLOR_CAPABILITIES ",
						"_location": "zcl_lighting.h",
						"_clusterID": "0x0300",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0x400b",
						"_name": "ColorTempPhysicalMinMireds",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_COLOR_CONTROL_COLOR_TEMP_PHYSICAL_MIN_MIREDS ",
						"_location": "zcl_lighting.h",
						"_clusterID": "0x0300",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x400c",
						"_name": "ColorTempPhysicalMaxMireds",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_COLOR_CONTROL_COLOR_TEMP_PHYSICAL_MAX_MIREDS ",
						"_location": "zcl_lighting.h",
						"_clusterID": "0x0300",
						"_default": "65279",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x400d",
						"_name": "CoupleColorTempToLevelMinMireds",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_COLOR_CONTROL_COUPLE_COLOR_TEMP_TO_LEVEL_MIN_MIREDS ",
						"_location": "zcl_lighting.h",
						"_clusterID": "0x0300",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x4010",
						"_name": "StartUpColorTemperatureMireds",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_COLOR_CONTROL_START_UP_COLOR_TEMPERATURE_MIREDS ",
						"_location": "zcl_lighting.h",
						"_clusterID": "0x0300",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0010",
						"_name": "NumberOfPrimaries",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_COLOR_CONTROL_NUMBER_OF_PRIMARIES ",
						"_location": "zcl_lighting.h",
						"_clusterID": "0x0300",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0x0011",
						"_name": "Primary1X",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_COLOR_CONTROL_PRIMARY_1_X ",
						"_location": "zcl_lighting.h",
						"_clusterID": "0x0300",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0012",
						"_name": "Primary1Y",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_COLOR_CONTROL_PRIMARY_1_Y ",
						"_location": "zcl_lighting.h",
						"_clusterID": "0x0300",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0013",
						"_name": "Primary1Intensity",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_COLOR_CONTROL_PRIMARY_1_INTENSITY ",
						"_location": "zcl_lighting.h",
						"_clusterID": "0x0300",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0015",
						"_name": "Primary2X",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_COLOR_CONTROL_PRIMARY_2_X ",
						"_location": "zcl_lighting.h",
						"_clusterID": "0x0300",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0016",
						"_name": "Primary2Y",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_COLOR_CONTROL_PRIMARY_2_Y ",
						"_location": "zcl_lighting.h",
						"_clusterID": "0x0300",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0017",
						"_name": "Primary2Intensity",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_COLOR_CONTROL_PRIMARY_2_INTENSITY ",
						"_location": "zcl_lighting.h",
						"_clusterID": "0x0300",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0019",
						"_name": "Primary3X",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_COLOR_CONTROL_PRIMARY_3_X ",
						"_location": "zcl_lighting.h",
						"_clusterID": "0x0300",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x001a",
						"_name": "Primary3Y",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_COLOR_CONTROL_PRIMARY_3_Y ",
						"_location": "zcl_lighting.h",
						"_clusterID": "0x0300",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x001b",
						"_name": "Primary3Intensity",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_COLOR_CONTROL_PRIMARY_3_INTENSITY ",
						"_location": "zcl_lighting.h",
						"_clusterID": "0x0300",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0020",
						"_name": "Primary4X",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_COLOR_CONTROL_PRIMARY_4_X ",
						"_location": "zcl_lighting.h",
						"_clusterID": "0x0300",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0021",
						"_name": "Primary4Y",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_COLOR_CONTROL_PRIMARY_4_Y ",
						"_location": "zcl_lighting.h",
						"_clusterID": "0x0300",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0022",
						"_name": "Primary4Intensity",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_COLOR_CONTROL_PRIMARY_4_INTENSITY ",
						"_location": "zcl_lighting.h",
						"_clusterID": "0x0300",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0024",
						"_name": "Primary5X",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_COLOR_CONTROL_PRIMARY_5_X ",
						"_location": "zcl_lighting.h",
						"_clusterID": "0x0300",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0025",
						"_name": "Primary5Y",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_COLOR_CONTROL_PRIMARY_5_Y ",
						"_location": "zcl_lighting.h",
						"_clusterID": "0x0300",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0026",
						"_name": "Primary5Intensity",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_COLOR_CONTROL_PRIMARY_5_INTENSITY ",
						"_location": "zcl_lighting.h",
						"_clusterID": "0x0300",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0028",
						"_name": "Primary6X",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_COLOR_CONTROL_PRIMARY_6_X ",
						"_location": "zcl_lighting.h",
						"_clusterID": "0x0300",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0029",
						"_name": "Primary6Y",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_COLOR_CONTROL_PRIMARY_6_Y ",
						"_location": "zcl_lighting.h",
						"_clusterID": "0x0300",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x002a",
						"_name": "Primary6Intensity",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_COLOR_CONTROL_PRIMARY_6_INTENSITY ",
						"_location": "zcl_lighting.h",
						"_clusterID": "0x0300",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0030",
						"_name": "WhitePointX",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_COLOR_CONTROL_WHITE_POINT_X ",
						"_location": "zcl_lighting.h",
						"_clusterID": "0x0300",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0031",
						"_name": "WhitePointY",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_COLOR_CONTROL_WHITE_POINT_Y ",
						"_location": "zcl_lighting.h",
						"_clusterID": "0x0300",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0032",
						"_name": "ColorPointRX",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_COLOR_CONTROL_COLOR_POINT_RX ",
						"_location": "zcl_lighting.h",
						"_clusterID": "0x0300",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0033",
						"_name": "ColorPointRY",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_COLOR_CONTROL_COLOR_POINT_RY ",
						"_location": "zcl_lighting.h",
						"_clusterID": "0x0300",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0034",
						"_name": "ColorPointRIntensity",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_COLOR_CONTROL_COLOR_POINT_R_INTENSITY ",
						"_location": "zcl_lighting.h",
						"_clusterID": "0x0300",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0036",
						"_name": "ColorPointGX",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_COLOR_CONTROL_COLOR_POINT_GX ",
						"_location": "zcl_lighting.h",
						"_clusterID": "0x0300",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0037",
						"_name": "ColorPointGY",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_COLOR_CONTROL_COLOR_POINT_GY ",
						"_location": "zcl_lighting.h",
						"_clusterID": "0x0300",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0038",
						"_name": "ColorPointGIntensity",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_COLOR_CONTROL_COLOR_POINT_G_INTENSITY ",
						"_location": "zcl_lighting.h",
						"_clusterID": "0x0300",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x003a",
						"_name": "ColorPointBX",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_COLOR_CONTROL_COLOR_POINT_BX ",
						"_location": "zcl_lighting.h",
						"_clusterID": "0x0300",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x003b",
						"_name": "ColorPointBY",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_COLOR_CONTROL_COLOR_POINT_BY ",
						"_location": "zcl_lighting.h",
						"_clusterID": "0x0300",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x003c",
						"_name": "ColorPointBIntensity",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_COLOR_CONTROL_COLOR_POINT_B_INTENSITY ",
						"_location": "zcl_lighting.h",
						"_clusterID": "0x0300",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					}
				],
				"command": [
					{
						"_id": "0x00",
						"_name": "MoveToHue",
						"_definition": "COMMAND_COLOR_CONTROL_MOVE_TO_HUE",
						"_location": "zcl_lighting.h",
						"_callback": "ColorControl_MoveToHue",
						"_clusterID": "0x0300",
						"_required": "false"
					},
					{
						"_id": "0x01",
						"_name": "MoveHue",
						"_definition": "COMMAND_COLOR_CONTROL_MOVE_HUE",
						"_location": "zcl_lighting.h",
						"_callback": "ColorControl_MoveHue",
						"_clusterID": "0x0300",
						"_required": "false"
					},
					{
						"_id": "0x02",
						"_name": "StepHue",
						"_definition": "COMMAND_COLOR_CONTROL_STEP_HUE",
						"_location": "zcl_lighting.h",
						"_callback": "ColorControl_StepHue",
						"_clusterID": "0x0300",
						"_required": "false"
					},
					{
						"_id": "0x03",
						"_name": "MoveToSaturation",
						"_definition": "COMMAND_COLOR_CONTROL_MOVE_TO_SATURATION",
						"_location": "zcl_lighting.h",
						"_callback": "ColorControl_MoveToSaturation",
						"_clusterID": "0x0300",
						"_required": "false"
					},
					{
						"_id": "0x04",
						"_name": "MoveSaturation",
						"_definition": "COMMAND_COLOR_CONTROL_MOVE_SATURATION",
						"_location": "zcl_lighting.h",
						"_callback": "ColorControl_MoveSaturation",
						"_clusterID": "0x0300",
						"_required": "false"
					},
					{
						"_id": "0x05",
						"_name": "StepSaturation",
						"_definition": "COMMAND_COLOR_CONTROL_STEP_SATURATION",
						"_location": "zcl_lighting.h",
						"_callback": "ColorControl_StepSaturation",
						"_clusterID": "0x0300",
						"_required": "false"
					},
					{
						"_id": "0x06",
						"_name": "MoveToHueAndSaturation",
						"_definition": "COMMAND_COLOR_CONTROL_MOVE_TO_HUE_AND_SATURATION",
						"_location": "zcl_lighting.h",
						"_callback": "ColorControl_MoveToHueAndSaturation",
						"_clusterID": "0x0300",
						"_required": "false"
					},
					{
						"_id": "0x07",
						"_name": "MoveToColor",
						"_definition": "COMMAND_COLOR_CONTROL_MOVE_TO_COLOR",
						"_location": "zcl_lighting.h",
						"_callback": "ColorControl_MoveToColor",
						"_clusterID": "0x0300",
						"_required": "false"
					},
					{
						"_id": "0x08",
						"_name": "MoveColor",
						"_definition": "COMMAND_COLOR_CONTROL_MOVE_COLOR",
						"_location": "zcl_lighting.h",
						"_callback": "ColorControl_MoveColor",
						"_clusterID": "0x0300",
						"_required": "false"
					},
					{
						"_id": "0x09",
						"_name": "StepColor",
						"_definition": "COMMAND_COLOR_CONTROL_STEP_COLOR",
						"_location": "zcl_lighting.h",
						"_callback": "ColorControl_StepColor",
						"_clusterID": "0x0300",
						"_required": "false"
					},
					{
						"_id": "0x0a",
						"_name": "MoveToColorTemperature",
						"_definition": "COMMAND_COLOR_CONTROL_MOVE_TO_COLOR_TEMPERATURE",
						"_location": "zcl_lighting.h",
						"_callback": "ColorControl_MoveToColorTemperature",
						"_clusterID": "0x0300",
						"_required": "false"
					},
					{
						"_id": "0x40",
						"_name": "EnhancedMoveToHue",
						"_definition": "COMMAND_COLOR_CONTROL_ENHANCED_MOVE_TO_HUE",
						"_location": "zcl_lighting.h",
						"_callback": "ColorControl_EnhancedMoveToHue",
						"_clusterID": "0x0300",
						"_required": "false"
					},
					{
						"_id": "0x41",
						"_name": "EnhancedMoveHue",
						"_definition": "COMMAND_COLOR_CONTROL_ENHANCED_MOVE_HUE",
						"_location": "zcl_lighting.h",
						"_callback": "ColorControl_EnhancedMoveHue",
						"_clusterID": "0x0300",
						"_required": "false"
					},
					{
						"_id": "0x42",
						"_name": "EnhancedStepHue",
						"_definition": "COMMAND_COLOR_CONTROL_ENHANCED_STEP_HUE",
						"_location": "zcl_lighting.h",
						"_callback": "ColorControl_EnhancedStepHue",
						"_clusterID": "0x0300",
						"_required": "false"
					},
					{
						"_id": "0x43",
						"_name": "EnhancedMoveToHueAndSaturation",
						"_definition": "COMMAND_COLOR_CONTROL_ENHANCED_MOVE_TO_HUE_AND_SATURATION",
						"_location": "zcl_lighting.h",
						"_callback": "ColorControl_EnhancedMoveToHueAndSaturation",
						"_clusterID": "0x0300",
						"_required": "false"
					},
					{
						"_id": "0x44",
						"_name": "ColorLoopSet",
						"_definition": "COMMAND_COLOR_CONTROL_COLOR_LOOP_SET",
						"_location": "zcl_lighting.h",
						"_callback": "ColorControl_ColorLoopSet",
						"_clusterID": "0x0300",
						"_required": "false"
					},
					{
						"_id": "0x47",
						"_name": "StopMoveStep",
						"_definition": "COMMAND_COLOR_CONTROL_STOP_MOVE_STEP",
						"_location": "zcl_lighting.h",
						"_callback": "ColorControl_StopMoveStep",
						"_clusterID": "0x0300",
						"_required": "true"
					},
					{
						"_id": "0x4b",
						"_name": "MoveColorTemperature",
						"_definition": "COMMAND_COLOR_CONTROL_MOVE_COLOR_TEMPERATURE",
						"_location": "zcl_lighting.h",
						"_callback": "ColorControl_MoveColorTemperature",
						"_clusterID": "0x0300",
						"_required": "false"
					},
					{
						"_id": "0x4c",
						"_name": "StepColorTemperature",
						"_definition": "COMMAND_COLOR_CONTROL_STEP_COLOR_TEMPERATURE",
						"_location": "zcl_lighting.h",
						"_callback": "ColorControl_StepColorTemperature",
						"_clusterID": "0x0300",
						"_required": "false"
					}
				]
			},
			"client": {
				"attribute": [
					{
						"_id": "0xfffd",
						"_name": "ClusterRevision",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_CLUSTER_REVISION",
						"_location": "zcl.h",
						"_clusterID": "0x0300",
						"_default": "1",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0xfffe",
						"_name": "AttributeReportingStatus",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_ATTRIBUTE_REPORTING_STATUS",
						"_location": "zcl.h",
						"_clusterID": "0x0300",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					}
				],
			},
			"appcallback": {
				"callbacks": [
					{
						"_name": "ColorControl_MoveToHue",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclCCMoveToHue_t *pCmd )",
						"comments": [
							{
								"_comment": "// This callback is called to process a Move To Hue command"
							}
						]
					},
					{
						"_name": "ColorControl_MoveHue",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclCCMoveHue_t *pCmd )",
						"comments": [
							{
								"_comment": "// This callback is called to process a Move Hue command"
							}
						]
					},
					{
						"_name": "ColorControl_StepHue",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclCCStepHue_t *pCmd )",
						"comments": [
							{
								"_comment": "// This callback is called to process a Step Hue command"
							}
						]
					},
					{
						"_name": "ColorControl_MoveToSaturation",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclCCMoveToSaturation_t *pCmd )",
						"comments": [
							{
								"_comment": "// This callback is called to process a Move To Saturation command"
							}
						]
					},
					{
						"_name": "ColorControl_MoveSaturation",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclCCMoveSaturation_t *pCmd )",
						"comments": [
							{
								"_comment": "// This callback is called to process a Move Saturation command"
							}
						]
					},
					{
						"_name": "ColorControl_StepSaturation",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclCCStepSaturation_t *pCmd )",
						"comments": [
							{
								"_comment": "// This callback is called to process a Step Saturation command"
							}
						]
					},
					{
						"_name": "ColorControl_MoveToHueAndSaturation",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclCCMoveToHueAndSaturation_t *pCmd )",
						"comments": [
							{
								"_comment": "// This callback is called to process a Move to Hue and Saturation command"
							}
						]
					},
					{
						"_name": "ColorControl_MoveToColor",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclCCMoveToColor_t *pCmd )",
						"comments": [
							{
								"_comment": "// This callback is called to process a Move to Color command"
							}
						]
					},
					{
						"_name": "ColorControl_MoveColor",
						"_returnType": "void",
						"_arguments": "( zclCCMoveColor_t *pCmd )",
						"comments": [
							{
								"_comment": "// This callback is called to process a Move Color command"
							}
						]
					},
					{
						"_name": "ColorControl_StepColor",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclCCStepColor_t *pCmd )",
						"comments": [
							{
								"_comment": "// This callback is called to process a Step Color command"
							}
						]
					},
					{
						"_name": "ColorControl_MoveToColorTemperature",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclCCMoveToColorTemperature_t *pCmd )",
						"comments": [
							{
								"_comment": "// This callback is called to process a Move to Color Temperature command"
							}
						]
					},
					{
						"_name": "ColorControl_EnhancedMoveToHue",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclCCEnhancedMoveToHue_t *pCmd )",
						"comments": [
							{
								"_comment": "// This callback is called to process an Enhanced Move To Hue command"
							}
						]
					},
					{
						"_name": "ColorControl_EnhancedMoveHue",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclCCEnhancedMoveHue_t *pCmd )",
						"comments": [
							{
								"_comment": "// This callback is called to process an Enhanced Move Hue command"
							}
						]
					},
					{
						"_name": "ColorControl_EnhancedStepHue",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclCCEnhancedStepHue_t *pCmd )",
						"comments": [
							{
								"_comment": "// This callback is called to process an Enhanced Step Hue command"
							}
						]
					},
					{
						"_name": "ColorControl_EnhancedMoveToHueAndSaturation",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclCCEnhancedMoveToHueAndSaturation_t *pCmd )",
						"comments": [
							{
								"_comment": "// This callback is called to process an Enhanced Move to Hue and Saturation command"
							}
						]
					},
					{
						"_name": "ColorControl_ColorLoopSet",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclCCColorLoopSet_t *pCmd )",
						"comments": [
							{
								"_comment": "// This callback is called to process an Color Loop Set command"
							}
						]
					},
					{
						"_name": "ColorControl_StopMoveStep",
						"_returnType": "ZStatus_t",
						"_arguments": "( void )",
						"comments": [
							{
								"_comment": "// This callback is called to process a Stop Step Move command"
							}
						]
					},
					{
						"_name": "ColorControl_MoveColorTemperature",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclCCMoveColorTemperature_t *pCmd )"
					},
					{
						"_name": "ColorControl_StepColorTemperature",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclCCStepColorTemperature_t *pCmd )"
					}
				],
				"_name": "zclLighting_AppCallbacks_t",
				"_register": "zclLighting_RegisterCmdCallbacks"
			},
			"_id": "0x0300",
			"_name": "ColorControl",
			"_definition": "ZCL_CLUSTER_ID_LIGHTING_COLOR_CONTROL",
			"_primary_transaction": "1",
			"_class": "lighting",
			"_flag": "ZCL_LIGHTING",
			"_location": "zcl_lighting.h"
		},
		{
			"server": {
				"attribute": [
					{
						"_id": "0xfffd",
						"_name": "ClusterRevision",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_CLUSTER_REVISION",
						"_location": "zcl.h",
						"_clusterID": "0x0203",
						"_default": "1",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0xfffe",
						"_name": "AttributeReportingStatus",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_ATTRIBUTE_REPORTING_STATUS",
						"_location": "zcl.h",
						"_clusterID": "0x0203",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0000",
						"_name": "RelativeHumidity",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_DEHUMIDIFICATION_CONTROL_RELATIVE_HUMIDITY ",
						"_location": "zcl_hvac.h",
						"_clusterID": "0x0203",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0001",
						"_name": "DehumidificationCooling",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_DEHUMIDIFICATION_CONTROL_DEHUMIDIFICATION_COOLING ",
						"_location": "zcl_hvac.h",
						"_clusterID": "0x0203",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "true",
						"_required": "true"
					},
					{
						"_id": "0x0010",
						"_name": "RHDehumidificationSetpoint",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_DEHUMIDIFICATION_CONTROL_RH_DEHUMIDIFICATION_SETPOINT ",
						"_location": "zcl_hvac.h",
						"_clusterID": "0x0203",
						"_default": "50",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0x0011",
						"_name": "RelativeHumidityMode",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_DEHUMIDIFICATION_CONTROL_RELATIVE_HUMIDITY_MODE ",
						"_location": "zcl_hvac.h",
						"_clusterID": "0x0203",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0012",
						"_name": "DehumidificationLockout",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_DEHUMIDIFICATION_CONTROL_DEHUMIDIFICATION_LOCKOUT ",
						"_location": "zcl_hvac.h",
						"_clusterID": "0x0203",
						"_default": "1",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0013",
						"_name": "DehumidificationHysteresis",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_DEHUMIDIFICATION_CONTROL_DEHUMIDIFICATION_HYSTERESIS ",
						"_location": "zcl_hvac.h",
						"_clusterID": "0x0203",
						"_default": "2",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0x0014",
						"_name": "DehumidificationMaxCool",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_DEHUMIDIFICATION_CONTROL_DEHUMIDIFICATION_MAX_COOL ",
						"_location": "zcl_hvac.h",
						"_clusterID": "0x0203",
						"_default": "20",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0x0015",
						"_name": "RelativeHumidityDisplay",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_DEHUMIDIFICATION_CONTROL_RELATIVE_HUMIDITY_DISPLAY ",
						"_location": "zcl_hvac.h",
						"_clusterID": "0x0203",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					}
				]
			},
			"client": {
				"attribute": [
					{
						"_id": "0xfffd",
						"_name": "ClusterRevision",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_CLUSTER_REVISION",
						"_location": "zcl.h",
						"_clusterID": "0x0203",
						"_default": "1",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0xfffe",
						"_name": "AttributeReportingStatus",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_ATTRIBUTE_REPORTING_STATUS",
						"_location": "zcl.h",
						"_clusterID": "0x0203",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					}
				],
			},
			"appcallback": {
				"callbacks": [
					{
						"_name": "HVAC_SetpointRaiseLower",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclCmdThermostatSetpointRaiseLowerPayload_t *pCmd )",
						"comments": [
							{
								"_comment": "// This callback is called to process a Setpoint Raise/Lower command"
							}
						]
					},
					{
						"_name": "HVAC_SetWeeklySchedule",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclThermostatWeeklySchedule_t *pCmd )",
						"comments": [
							{
								"_comment": "// This callback is called to process a Set Weekly Schedule command"
							}
						]
					},
					{
						"_name": "HVAC_GetWeeklySchedule",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclThermostatGetWeeklySchedule_t *pCmd )",
						"comments": [
							{
								"_comment": "// This callback is called to process a Get Weekly Schedule command"
							}
						]
					},
					{
						"_name": "HVAC_ClearWeeklySchedule",
						"_returnType": "ZStatus_t",
						"_arguments": "( void )",
						"comments": [
							{
								"_comment": "// This callback is called to process a Clear Weekly Schedule command"
							}
						]
					},
					{
						"_name": "HVAC_GetRelayStatusLog",
						"_returnType": "ZStatus_t",
						"_arguments": "( void )",
						"comments": [
							{
								"_comment": "// This callback is called to process a Get Relay Status Log command"
							}
						]
					},
					{
						"_name": "HVAC_GetWeeklyScheduleRsp",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclThermostatWeeklySchedule_t *pCmd )",
						"comments": [
							{
								"_comment": "// This callback is called to process a Get Weekly Schedule Response command"
							}
						]
					},
					{
						"_name": "HVAC_GetRelayStatusLogRsp",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclThermostatGetRelayStatusLogRsp_t *pCmd )",
						"comments": [
							{
								"_comment": "// This callback is called to process a Get Relay Status Log Response command"
							}
						]
					}
				],
				"_name": "zclHVAC_AppCallbacks_t",
				"_register": "zclHVAC_RegisterCmdCallbacks"
			},
			"_id": "0x0203",
			"_name": "DehumidificationControl",
			"_definition": "ZCL_CLUSTER_ID_HVAC_DIHUMIDIFICATION_CONTROL",
			"_primary_transaction": "1",
			"_class": "hvac",
			"_flag": "ZCL_HVAC_CLUSTER",
			"_location": "zcl_hvac.h"
		},
		{
			"server": {
				"attribute": [
					{
						"_id": "0xfffd",
						"_name": "ClusterRevision",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_CLUSTER_REVISION",
						"_location": "zcl.h",
						"_clusterID": "0x0002",
						"_default": "1",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0xfffe",
						"_name": "AttributeReportingStatus",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_ATTRIBUTE_REPORTING_STATUS",
						"_location": "zcl.h",
						"_clusterID": "0x0002",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0000",
						"_name": "CurrentTemperature",
						"_type": "int16",
						"_zigbeetype": "ZCL_DATATYPE_INT16",
						"_definition": "ATTRID_DEVICE_TEMPERATURE_CONFIGURATION_CURRENT_TEMPERATURE ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0002",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0x0001",
						"_name": "MinTempExperienced",
						"_type": "int16",
						"_zigbeetype": "ZCL_DATATYPE_INT16",
						"_definition": "ATTRID_DEVICE_TEMPERATURE_CONFIGURATION_MIN_TEMP_EXPERIENCED ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0002",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0002",
						"_name": "MaxTempExperienced",
						"_type": "int16",
						"_zigbeetype": "ZCL_DATATYPE_INT16",
						"_definition": "ATTRID_DEVICE_TEMPERATURE_CONFIGURATION_MAX_TEMP_EXPERIENCED ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0002",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0003",
						"_name": "OverTempTotalDwell",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_DEVICE_TEMPERATURE_CONFIGURATION_OVER_TEMP_TOTAL_DWELL ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0002",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0010",
						"_name": "DeviceTempAlarmMask",
						"_type": "map8",
						"_zigbeetype": "ZCL_DATATYPE_BITMAP8",
						"_definition": "ATTRID_DEVICE_TEMPERATURE_CONFIGURATION_DEVICE_TEMP_ALARM_MASK ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0002",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0011",
						"_name": "LowTempThreshold",
						"_type": "int16",
						"_zigbeetype": "ZCL_DATATYPE_INT16",
						"_definition": "ATTRID_DEVICE_TEMPERATURE_CONFIGURATION_LOW_TEMP_THRESHOLD ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0002",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0012",
						"_name": "HighTempThreshold",
						"_type": "int16",
						"_zigbeetype": "ZCL_DATATYPE_INT16",
						"_definition": "ATTRID_DEVICE_TEMPERATURE_CONFIGURATION_HIGH_TEMP_THRESHOLD ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0002",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0013",
						"_name": "LowTempDwellTripPoint",
						"_type": "uint24",
						"_zigbeetype": "ZCL_DATATYPE_UINT24",
						"_definition": "ATTRID_DEVICE_TEMPERATURE_CONFIGURATION_LOW_TEMP_DWELL_TRIP_POINT ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0002",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0014",
						"_name": "HighTempDwellTripPoint",
						"_type": "uint24",
						"_zigbeetype": "ZCL_DATATYPE_UINT24",
						"_definition": "ATTRID_DEVICE_TEMPERATURE_CONFIGURATION_HIGH_TEMP_DWELL_TRIP_POINT ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0002",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					}
				]
			},
			"client": {
				"attribute": [
					{
						"_id": "0xfffd",
						"_name": "ClusterRevision",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_CLUSTER_REVISION",
						"_location": "zcl.h",
						"_clusterID": "0x0002",
						"_default": "1",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0xfffe",
						"_name": "AttributeReportingStatus",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_ATTRIBUTE_REPORTING_STATUS",
						"_location": "zcl.h",
						"_clusterID": "0x0002",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					}
				],
			},
			"appcallback": {
				"callbacks": [
					{
						"_name": "BasicReset",
						"_returnType": "void",
						"_arguments": "( void )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Reset to Factory Defaults"
							},
							{
								"_comment": "/// command. On receipt of this command, the device resets all the attributes"
							},
							{
								"_comment": "/// of all its clusters to their factory defaults."
							}
						]
					},
					{
						"_name": "Identify",
						"_returnType": "void",
						"_arguments": "( zclIdentify_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Identify command."
							}
						]
					},
					{
						"_name": "IdentifyQuery",
						"_returnType": "void",
						"_arguments": "( zclIdentifyQuery_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Identify Query command."
							}
						]
					},
					{
						"_name": "IdentifyQueryRsp",
						"_returnType": "void",
						"_arguments": "( zclIdentifyQueryRsp_t *pRsp )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Identify Query Response command."
							}
						]
					},
					{
						"_name": "IdentifyTriggerEffect",
						"_returnType": "void",
						"_arguments": "( zclIdentifyTriggerEffect_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Identify Trigger Effect command."
							}
						]
					},
					{
						"_name": "#ifdef ZCL_ON_OFF",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "OnOff",
						"_returnType": "void",
						"_arguments": "( uint8_t cmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming On, Off or Toggle command."
							}
						]
					},
					{
						"_name": "OnOff_OffWithEffect",
						"_returnType": "void",
						"_arguments": "( zclOffWithEffect_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Off with Effect"
							}
						]
					},
					{
						"_name": "OnOff_OnWithRecallGlobalScene",
						"_returnType": "void",
						"_arguments": "( void )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming On with Recall Global Scene command."
							}
						]
					},
					{
						"_name": "OnOff_OnWithTimedOff",
						"_returnType": "void",
						"_arguments": "( zclOnWithTimedOff_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming On with Timed Off."
							}
						]
					},
					{
						"_name": "#endif",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "#ifdef ZCL_LEVEL_CTRL",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "LevelControlMoveToLevel",
						"_returnType": "void",
						"_arguments": "( zclLCMoveToLevel_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Level Control - Move to Level command"
							}
						]
					},
					{
						"_name": "LevelControlMove",
						"_returnType": "void",
						"_arguments": "( zclLCMove_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Level Control - Move command"
							}
						]
					},
					{
						"_name": "LevelControlStep",
						"_returnType": "void",
						"_arguments": "( zclLCStep_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Level Control - Step command"
							}
						]
					},
					{
						"_name": "LevelControlStop",
						"_returnType": "void",
						"_arguments": "( zclLCStop_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Level Control - Stop command"
							}
						]
					},
					{
						"_name": "LevelControlMoveFreq",
						"_returnType": "void",
						"_arguments": "( zclLCMoveFreq_t *pCmd)",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Level Control - Move to Closest Frequency command"
							}
						]
					},
					{
						"_name": "#endif",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "#ifdef ZCL_GROUPS",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "GroupRsp",
						"_returnType": "void",
						"_arguments": "( zclGroupRsp_t *pRsp )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an received Group Response message."
							},
							{
								"_comment": "/// This means that this app sent the request message."
							}
						]
					},
					{
						"_name": "#endif",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "#ifdef ZCL_SCENES",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "SceneStoreReq",
						"_returnType": "uint8_t",
						"_arguments": "( zclSceneReq_t *pReq )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Scene Store request."
							},
							{
								"_comment": "/// The app will fill in the 'extField' with what is needed to restore its"
							},
							{
								"_comment": "/// current settings. Before overwriting the content of the scene,"
							},
							{
								"_comment": "/// App needs to verify if there was a change with the previous configuration."
							},
							{
								"_comment": "/// App function returns TRUE if there was a change, otherwise returns FALSE."
							}
						]
					},
					{
						"_name": "SceneRecallReq",
						"_returnType": "void",
						"_arguments": "( zclSceneReq_t *pReq )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Scene Recall request"
							},
							{
								"_comment": "/// The app will use what's in the 'extField' to restore to these settings."
							}
						]
					},
					{
						"_name": "SceneRsp",
						"_returnType": "void",
						"_arguments": "( zclSceneRsp_t *pRsp )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Scene responses. This means"
							},
							{
								"_comment": "/// that this app sent the request for this response."
							}
						]
					},
					{
						"_name": "#endif",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "#ifdef ZCL_ALARMS",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "Alarm",
						"_returnType": "void",
						"_arguments": "( uint8_t direction, zclAlarm_t *pAlarm )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Alarm request or response command."
							}
						]
					},
					{
						"_name": "#endif",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "#ifdef SE_UK_EXT",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "GetEventLog",
						"_returnType": "void",
						"_arguments": "( uint8_t srcEP, afAddrType_t *srcAddr, zclGetEventLog_t *pEventLog, uint8_t seqNum )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Alarm Get Event Log command."
							}
						]
					},
					{
						"_name": "PublishEventLog",
						"_returnType": "void",
						"_arguments": "( afAddrType_t *srcAddr, zclPublishEventLog_t *pEventLog )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Alarm Publish Event Log command."
							}
						]
					},
					{
						"_name": "#endif",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "Location",
						"_returnType": "void",
						"_arguments": "( zclLocation_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to to process an incoming RSSI Location command."
							}
						]
					},
					{
						"_name": "LocationRsp",
						"_returnType": "void",
						"_arguments": "( zclLocationRsp_t *pRsp )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming RSSI Location response command."
							},
							{
								"_comment": "/// This means  that this app sent the request for this response."
							}
						]
					}
				],
				"_name": "zclGeneral_AppCallbacks_t",
				"_register": "zclGeneral_RegisterCmdCallbacks"
			},
			"_id": "0x0002",
			"_name": "DeviceTemperatureConfiguration",
			"_definition": "ZCL_CLUSTER_ID_GENERAL_DEVICE_TEMP_CONFIG",
			"_primary_transaction": "0",
			"_class": "general",
			"_flag": "ZCL_TEMP_CONFIG",
			"_location": "zcl_general.h"
		},
		{
			"server": {
				"attribute": [
					{
						"_id": "0xfffd",
						"_name": "ClusterRevision",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_CLUSTER_REVISION",
						"_location": "zcl.h",
						"_clusterID": "0x0b05",
						"_default": "1",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0xfffe",
						"_name": "AttributeReportingStatus",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_ATTRIBUTE_REPORTING_STATUS",
						"_location": "zcl.h",
						"_clusterID": "0x0b05",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0000",
						"_name": "NumberOfResets",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_DIAGNOSTICS_NUMBER_OF_RESETS ",
						"_location": "zcl_ha.h",
						"_clusterID": "0x0b05",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0001",
						"_name": "PersistentMemoryWrites",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_DIAGNOSTICS_PERSISTENT_MEMORY_WRITES ",
						"_location": "zcl_ha.h",
						"_clusterID": "0x0b05",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0100",
						"_name": "MacRxBcast",
						"_type": "uint32",
						"_zigbeetype": "ZCL_DATATYPE_UINT32",
						"_definition": "ATTRID_DIAGNOSTICS_MAC_RX_BCAST ",
						"_location": "zcl_ha.h",
						"_clusterID": "0x0b05",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0101",
						"_name": "MacTxBcast",
						"_type": "uint32",
						"_zigbeetype": "ZCL_DATATYPE_UINT32",
						"_definition": "ATTRID_DIAGNOSTICS_MAC_TX_BCAST ",
						"_location": "zcl_ha.h",
						"_clusterID": "0x0b05",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0102",
						"_name": "MacRxUcast",
						"_type": "uint32",
						"_zigbeetype": "ZCL_DATATYPE_UINT32",
						"_definition": "ATTRID_DIAGNOSTICS_MAC_RX_UCAST ",
						"_location": "zcl_ha.h",
						"_clusterID": "0x0b05",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0103",
						"_name": "MacTxUcast",
						"_type": "uint32",
						"_zigbeetype": "ZCL_DATATYPE_UINT32",
						"_definition": "ATTRID_DIAGNOSTICS_MAC_TX_UCAST ",
						"_location": "zcl_ha.h",
						"_clusterID": "0x0b05",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0104",
						"_name": "MacTxUcastRetry",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_DIAGNOSTICS_MAC_TX_UCAST_RETRY ",
						"_location": "zcl_ha.h",
						"_clusterID": "0x0b05",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0105",
						"_name": "MacTxUcastFail",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_DIAGNOSTICS_MAC_TX_UCAST_FAIL ",
						"_location": "zcl_ha.h",
						"_clusterID": "0x0b05",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0106",
						"_name": "APSRxBcast",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_DIAGNOSTICS_APS_RX_BCAST ",
						"_location": "zcl_ha.h",
						"_clusterID": "0x0b05",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0107",
						"_name": "APSTxBcast",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_DIAGNOSTICS_APS_TX_BCAST ",
						"_location": "zcl_ha.h",
						"_clusterID": "0x0b05",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0108",
						"_name": "APSRxUcast",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_DIAGNOSTICS_APS_RX_UCAST ",
						"_location": "zcl_ha.h",
						"_clusterID": "0x0b05",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0109",
						"_name": "APSTxUcastSuccess",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_DIAGNOSTICS_APS_TX_UCAST_SUCCESS ",
						"_location": "zcl_ha.h",
						"_clusterID": "0x0b05",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x010a",
						"_name": "APSTxUcastRetry",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_DIAGNOSTICS_APS_TX_UCAST_RETRY ",
						"_location": "zcl_ha.h",
						"_clusterID": "0x0b05",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x010b",
						"_name": "APSTxUcastFail",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_DIAGNOSTICS_APS_TX_UCAST_FAIL ",
						"_location": "zcl_ha.h",
						"_clusterID": "0x0b05",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x010c",
						"_name": "RouteDiscInitiated",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_DIAGNOSTICS_ROUTE_DISC_INITIATED ",
						"_location": "zcl_ha.h",
						"_clusterID": "0x0b05",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x010d",
						"_name": "NeighborAdded",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_DIAGNOSTICS_NEIGHBOR_ADDED ",
						"_location": "zcl_ha.h",
						"_clusterID": "0x0b05",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x010e",
						"_name": "NeighborRemoved",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_DIAGNOSTICS_NEIGHBOR_REMOVED ",
						"_location": "zcl_ha.h",
						"_clusterID": "0x0b05",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x010f",
						"_name": "NeighborStale",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_DIAGNOSTICS_NEIGHBOR_STALE ",
						"_location": "zcl_ha.h",
						"_clusterID": "0x0b05",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0110",
						"_name": "JoinIndication",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_DIAGNOSTICS_JOIN_INDICATION ",
						"_location": "zcl_ha.h",
						"_clusterID": "0x0b05",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0111",
						"_name": "ChildMoved",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_DIAGNOSTICS_CHILD_MOVED ",
						"_location": "zcl_ha.h",
						"_clusterID": "0x0b05",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0112",
						"_name": "NWKFCFailure",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_DIAGNOSTICS_NWKFC_FAILURE ",
						"_location": "zcl_ha.h",
						"_clusterID": "0x0b05",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0113",
						"_name": "APSFCFailure",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_DIAGNOSTICS_APSFC_FAILURE ",
						"_location": "zcl_ha.h",
						"_clusterID": "0x0b05",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0114",
						"_name": "APSUnauthorizedKey",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_DIAGNOSTICS_APS_UNAUTHORIZED_KEY ",
						"_location": "zcl_ha.h",
						"_clusterID": "0x0b05",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0115",
						"_name": "NWKDecryptFailures",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_DIAGNOSTICS_NWK_DECRYPT_FAILURES ",
						"_location": "zcl_ha.h",
						"_clusterID": "0x0b05",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0116",
						"_name": "APSDecryptFailures",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_DIAGNOSTICS_APS_DECRYPT_FAILURES ",
						"_location": "zcl_ha.h",
						"_clusterID": "0x0b05",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0117",
						"_name": "PacketBufferAllocateFailures",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_DIAGNOSTICS_PACKET_BUFFER_ALLOCATE_FAILURES ",
						"_location": "zcl_ha.h",
						"_clusterID": "0x0b05",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0118",
						"_name": "RelayedUcast",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_DIAGNOSTICS_RELAYED_UCAST ",
						"_location": "zcl_ha.h",
						"_clusterID": "0x0b05",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0119",
						"_name": "PHYToMACQueueLimitReached",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_DIAGNOSTICS_PHY_TO_MAC_QUEUE_LIMIT_REACHED ",
						"_location": "zcl_ha.h",
						"_clusterID": "0x0b05",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x011a",
						"_name": "PacketValidateDropCount",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_DIAGNOSTICS_PACKET_VALIDATE_DROP_COUNT ",
						"_location": "zcl_ha.h",
						"_clusterID": "0x0b05",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x011b",
						"_name": "AverageMACRetryPerAPSMessageSent",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_DIAGNOSTICS_AVERAGE_MAC_RETRY_PER_APS_MESSAGE_SENT ",
						"_location": "zcl_ha.h",
						"_clusterID": "0x0b05",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x011c",
						"_name": "LastMessageLQI",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_DIAGNOSTICS_LAST_MESSAGE_LQI ",
						"_location": "zcl_ha.h",
						"_clusterID": "0x0b05",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x011d",
						"_name": "LastMessageRSSI",
						"_type": "int8",
						"_zigbeetype": "ZCL_DATATYPE_INT8",
						"_definition": "ATTRID_DIAGNOSTICS_LAST_MESSAGE_RSSI ",
						"_location": "zcl_ha.h",
						"_clusterID": "0x0b05",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					}
				]
			},
			"client": {
				"attribute": [
					{
						"_id": "0xfffd",
						"_name": "ClusterRevision",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_CLUSTER_REVISION",
						"_location": "zcl.h",
						"_clusterID": "0x0b05",
						"_default": "1",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0xfffe",
						"_name": "AttributeReportingStatus",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_ATTRIBUTE_REPORTING_STATUS",
						"_location": "zcl.h",
						"_clusterID": "0x0b05",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					}
				],
			},
			"appcallback": {
				"_name": "None",
				"_register": "None"
			},
			"_id": "0x0b05",
			"_name": "Diagnostics",
			"_definition": "ZCL_CLUSTER_ID_HA_DIAGNOSTIC",
			"_primary_transaction": "0",
			"_class": "ha",
			"_flag": "ZCL_DIAGNOSTICS",
			"_location": "zcl_ha.h"
		},
		{
			"server": {
				"attribute": [
					{
						"_id": "0xfffd",
						"_name": "ClusterRevision",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_CLUSTER_REVISION",
						"_location": "zcl.h",
						"_clusterID": "0x0101",
						"_default": "1",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0xfffe",
						"_name": "AttributeReportingStatus",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_ATTRIBUTE_REPORTING_STATUS",
						"_location": "zcl.h",
						"_clusterID": "0x0101",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0000",
						"_name": "LockState",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_DOOR_LOCK_LOCK_STATE ",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0101",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "true",
						"_required": "true"
					},
					{
						"_id": "0x0001",
						"_name": "LockType",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_DOOR_LOCK_LOCK_TYPE ",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0101",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0x0002",
						"_name": "ActuatorEnabled",
						"_type": "bool",
						"_zigbeetype": "ZCL_DATATYPE_BOOLEAN",
						"_definition": "ATTRID_DOOR_LOCK_ACTUATOR_ENABLED ",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0101",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0x0003",
						"_name": "DoorState",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_DOOR_LOCK_DOOR_STATE ",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0101",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x0004",
						"_name": "DoorOpenEvents",
						"_type": "uint32",
						"_zigbeetype": "ZCL_DATATYPE_UINT32",
						"_definition": "ATTRID_DOOR_LOCK_DOOR_OPEN_EVENTS ",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0101",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0005",
						"_name": "DoorClosedEvents",
						"_type": "uint32",
						"_zigbeetype": "ZCL_DATATYPE_UINT32",
						"_definition": "ATTRID_DOOR_LOCK_DOOR_CLOSED_EVENTS ",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0101",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0006",
						"_name": "OpenPeriod",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_DOOR_LOCK_OPEN_PERIOD ",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0101",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0010",
						"_name": "NumberOfLogRecordsSupported",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_DOOR_LOCK_NUMBER_OF_LOG_RECORDS_SUPPORTED ",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0101",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0011",
						"_name": "NumberOfTotalUsersSupported",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_DOOR_LOCK_NUMBER_OF_TOTAL_USERS_SUPPORTED ",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0101",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0012",
						"_name": "NumberOfPINUsersSupported",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_DOOR_LOCK_NUMBER_OF_PIN_USERS_SUPPORTED ",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0101",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0013",
						"_name": "NumberOfRFIDUsersSupported",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_DOOR_LOCK_NUMBER_OF_RFID_USERS_SUPPORTED ",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0101",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0014",
						"_name": "NumberOfWeekDaySchedulesSupportedPerUser",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_DOOR_LOCK_NUMBER_OF_WEEK_DAY_SCHEDULES_SUPPORTED_PER_USER ",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0101",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0015",
						"_name": "NumberOfYearDaySchedulesSupportedPerUser",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_DOOR_LOCK_NUMBER_OF_YEAR_DAY_SCHEDULES_SUPPORTED_PER_USER ",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0101",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0016",
						"_name": "NumberOfHolidaySchedulesSupported",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_DOOR_LOCK_NUMBER_OF_HOLIDAY_SCHEDULES_SUPPORTED ",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0101",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0017",
						"_name": "MaxPINCodeLength",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_DOOR_LOCK_MAX_PIN_CODE_LENGTH ",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0101",
						"_default": "8",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0018",
						"_name": "MinPINCodeLength",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_DOOR_LOCK_MIN_PIN_CODE_LENGTH ",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0101",
						"_default": "4",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0019",
						"_name": "MaxRFIDCodeLength",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_DOOR_LOCK_MAX_RFID_CODE_LENGTH ",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0101",
						"_default": "20",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x001a",
						"_name": "MinRFIDCodeLength",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_DOOR_LOCK_MIN_RFID_CODE_LENGTH ",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0101",
						"_default": "8",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0020",
						"_name": "EnableLogging",
						"_type": "bool",
						"_zigbeetype": "ZCL_DATATYPE_BOOLEAN",
						"_definition": "ATTRID_DOOR_LOCK_ENABLE_LOGGING ",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0101",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x0021",
						"_name": "Language",
						"_type": "uint8[]",
						"_zigbeetype": "ZCL_DATATYPE_CHAR_STR",
						"_definition": "ATTRID_DOOR_LOCK_LANGUAGE ",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0101",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x0022",
						"_name": "LEDSettings",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_DOOR_LOCK_LED_SETTINGS ",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0101",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x0023",
						"_name": "AutoRelockTime",
						"_type": "uint32",
						"_zigbeetype": "ZCL_DATATYPE_UINT32",
						"_definition": "ATTRID_DOOR_LOCK_AUTO_RELOCK_TIME ",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0101",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x0024",
						"_name": "SoundVolume",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_DOOR_LOCK_SOUND_VOLUME ",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0101",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x0025",
						"_name": "OperatingMode",
						"_type": "DrlkOperMode",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_DOOR_LOCK_OPERATING_MODE ",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0101",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x0026",
						"_name": "SupportedOperatingModes",
						"_type": "map16",
						"_zigbeetype": "ZCL_DATATYPE_BITMAP16",
						"_definition": "ATTRID_DOOR_LOCK_SUPPORTED_OPERATING_MODES ",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0101",
						"_default": "1",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0027",
						"_name": "DefaultConfigurationRegister",
						"_type": "map16",
						"_zigbeetype": "ZCL_DATATYPE_BITMAP16",
						"_definition": "ATTRID_DOOR_LOCK_DEFAULT_CONFIGURATION_REGISTER ",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0101",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x0028",
						"_name": "EnableLocalProgramming",
						"_type": "bool",
						"_zigbeetype": "ZCL_DATATYPE_BOOLEAN",
						"_definition": "ATTRID_DOOR_LOCK_ENABLE_LOCAL_PROGRAMMING ",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0101",
						"_default": "1",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x0029",
						"_name": "EnableOneTouchLocking",
						"_type": "bool",
						"_zigbeetype": "ZCL_DATATYPE_BOOLEAN",
						"_definition": "ATTRID_DOOR_LOCK_ENABLE_ONE_TOUCH_LOCKING ",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0101",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x002a",
						"_name": "EnableInsideStatusLED",
						"_type": "bool",
						"_zigbeetype": "ZCL_DATATYPE_BOOLEAN",
						"_definition": "ATTRID_DOOR_LOCK_ENABLE_INSIDE_STATUS_LED ",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0101",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x002b",
						"_name": "EnablePrivacyModeButton",
						"_type": "bool",
						"_zigbeetype": "ZCL_DATATYPE_BOOLEAN",
						"_definition": "ATTRID_DOOR_LOCK_ENABLE_PRIVACY_MODE_BUTTON ",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0101",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x0030",
						"_name": "WrongCodeEntryLimit",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_DOOR_LOCK_WRONG_CODE_ENTRY_LIMIT ",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0101",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x0031",
						"_name": "UserCodeTemporaryDisableTime",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_DOOR_LOCK_USER_CODE_TEMPORARY_DISABLE_TIME ",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0101",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x0032",
						"_name": "SendPINOverTheAir",
						"_type": "bool",
						"_zigbeetype": "ZCL_DATATYPE_BOOLEAN",
						"_definition": "ATTRID_DOOR_LOCK_SEND_PIN_OVER_THE_AIR ",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0101",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x0033",
						"_name": "RequirePINforRFOperation",
						"_type": "bool",
						"_zigbeetype": "ZCL_DATATYPE_BOOLEAN",
						"_definition": "ATTRID_DOOR_LOCK_REQUIRE_PI_NFOR_RF_OPERATION ",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0101",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x0034",
						"_name": "SecurityLevel",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_DOOR_LOCK_SECURITY_LEVEL ",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0101",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x0040",
						"_name": "AlarmMask",
						"_type": "map16",
						"_zigbeetype": "ZCL_DATATYPE_BITMAP16",
						"_definition": "ATTRID_DOOR_LOCK_ALARM_MASK ",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0101",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x0041",
						"_name": "KeypadOperationEventMask",
						"_type": "map16",
						"_zigbeetype": "ZCL_DATATYPE_BITMAP16",
						"_definition": "ATTRID_DOOR_LOCK_KEYPAD_OPERATION_EVENT_MASK ",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0101",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x0042",
						"_name": "RFOperationEventMask",
						"_type": "map16",
						"_zigbeetype": "ZCL_DATATYPE_BITMAP16",
						"_definition": "ATTRID_DOOR_LOCK_RF_OPERATION_EVENT_MASK ",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0101",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x0043",
						"_name": "ManualOperationEventMask",
						"_type": "map16",
						"_zigbeetype": "ZCL_DATATYPE_BITMAP16",
						"_definition": "ATTRID_DOOR_LOCK_MANUAL_OPERATION_EVENT_MASK ",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0101",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x0044",
						"_name": "RFIDOperationEventMask",
						"_type": "map16",
						"_zigbeetype": "ZCL_DATATYPE_BITMAP16",
						"_definition": "ATTRID_DOOR_LOCK_RFID_OPERATION_EVENT_MASK ",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0101",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x0045",
						"_name": "KeypadProgrammingEventMask",
						"_type": "map16",
						"_zigbeetype": "ZCL_DATATYPE_BITMAP16",
						"_definition": "ATTRID_DOOR_LOCK_KEYPAD_PROGRAMMING_EVENT_MASK ",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0101",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x0046",
						"_name": "RFProgrammingEventMask",
						"_type": "map16",
						"_zigbeetype": "ZCL_DATATYPE_BITMAP16",
						"_definition": "ATTRID_DOOR_LOCK_RF_PROGRAMMING_EVENT_MASK ",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0101",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x0047",
						"_name": "RFIDProgrammingEventMask",
						"_type": "map16",
						"_zigbeetype": "ZCL_DATATYPE_BITMAP16",
						"_definition": "ATTRID_DOOR_LOCK_RFID_PROGRAMMING_EVENT_MASK ",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0101",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "true",
						"_required": "false"
					}
				],
				"command": [
					{
						"_id": "0x00",
						"_name": "LockDoor",
						"_definition": "COMMAND_DOOR_LOCK_LOCK_DOOR",
						"_location": "zcl_closures.h",
						"_callback": "DoorLock",
						"_clusterID": "0x0101",
						"_required": "true"
					},
					{
						"_id": "0x01",
						"_name": "UnlockDoor",
						"_definition": "COMMAND_DOOR_LOCK_UNLOCK_DOOR",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0101",
						"_required": "true"
					},
					{
						"_id": "0x02",
						"_name": "Toggle",
						"_definition": "COMMAND_DOOR_LOCK_TOGGLE",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0101",
						"_required": "false"
					},
					{
						"_id": "0x03",
						"_name": "UnlockWithTimeout",
						"_definition": "COMMAND_DOOR_LOCK_UNLOCK_WITH_TIMEOUT",
						"_location": "zcl_closures.h",
						"_callback": "DoorLockUnlockWithTimeout",
						"_clusterID": "0x0101",
						"_required": "false"
					},
					{
						"_id": "0x04",
						"_name": "GetLogRecord",
						"_definition": "COMMAND_DOOR_LOCK_GET_LOG_RECORD",
						"_location": "zcl_closures.h",
						"_callback": "DoorLockGetLogRecord",
						"_clusterID": "0x0101",
						"_required": "false"
					},
					{
						"_id": "0x05",
						"_name": "SetPINCode",
						"_definition": "COMMAND_DOOR_LOCK_SET_PIN_CODE",
						"_location": "zcl_closures.h",
						"_callback": "DoorLockSetPINCode",
						"_clusterID": "0x0101",
						"_required": "false"
					},
					{
						"_id": "0x06",
						"_name": "GetPINCode",
						"_definition": "COMMAND_DOOR_LOCK_GET_PIN_CODE",
						"_location": "zcl_closures.h",
						"_callback": "DoorLockGetPINCode",
						"_clusterID": "0x0101",
						"_required": "false"
					},
					{
						"_id": "0x07",
						"_name": "ClearPINCode",
						"_definition": "COMMAND_DOOR_LOCK_CLEAR_PIN_CODE",
						"_location": "zcl_closures.h",
						"_callback": "DoorLockClearPINCode",
						"_clusterID": "0x0101",
						"_required": "false"
					},
					{
						"_id": "0x08",
						"_name": "ClearAllPINCodes",
						"_definition": "COMMAND_DOOR_LOCK_CLEAR_ALL_PIN_CODES",
						"_location": "zcl_closures.h",
						"_callback": "DoorLockClearAllPINCodes",
						"_clusterID": "0x0101",
						"_required": "false"
					},
					{
						"_id": "0x09",
						"_name": "SetUserStatus",
						"_definition": "COMMAND_DOOR_LOCK_SET_USER_STATUS",
						"_location": "zcl_closures.h",
						"_callback": "DoorLockSetUserStatus",
						"_clusterID": "0x0101",
						"_required": "false"
					},
					{
						"_id": "0x0a",
						"_name": "GetUserStatus",
						"_definition": "COMMAND_DOOR_LOCK_GET_USER_STATUS",
						"_location": "zcl_closures.h",
						"_callback": "DoorLockGetUserStatus",
						"_clusterID": "0x0101",
						"_required": "false"
					},
					{
						"_id": "0x0b",
						"_name": "SetWeekdaySchedule",
						"_definition": "COMMAND_DOOR_LOCK_SET_WEEKDAY_SCHEDULE",
						"_location": "zcl_closures.h",
						"_callback": "DoorLockSetWeekDaySchedule",
						"_clusterID": "0x0101",
						"_required": "false"
					},
					{
						"_id": "0x0c",
						"_name": "GetWeekdaySchedule",
						"_definition": "COMMAND_DOOR_LOCK_GET_WEEKDAY_SCHEDULE",
						"_location": "zcl_closures.h",
						"_callback": "DoorLockGetWeekDaySchedule",
						"_clusterID": "0x0101",
						"_required": "false"
					},
					{
						"_id": "0x0d",
						"_name": "ClearWeekdaySchedule",
						"_definition": "COMMAND_DOOR_LOCK_CLEAR_WEEKDAY_SCHEDULE",
						"_location": "zcl_closures.h",
						"_callback": "DoorLockClearWeekDaySchedule",
						"_clusterID": "0x0101",
						"_required": "false"
					},
					{
						"_id": "0x0e",
						"_name": "SetYearDaySchedule",
						"_definition": "COMMAND_DOOR_LOCK_SET_YEAR_DAY_SCHEDULE",
						"_location": "zcl_closures.h",
						"_callback": "DoorLockSetYearDaySchedule",
						"_clusterID": "0x0101",
						"_required": "false"
					},
					{
						"_id": "0x0f",
						"_name": "GetYearDaySchedule",
						"_definition": "COMMAND_DOOR_LOCK_GET_YEAR_DAY_SCHEDULE",
						"_location": "zcl_closures.h",
						"_callback": "DoorLockGetYearDaySchedule",
						"_clusterID": "0x0101",
						"_required": "false"
					},
					{
						"_id": "0x10",
						"_name": "ClearYearDaySchedule",
						"_definition": "COMMAND_DOOR_LOCK_CLEAR_YEAR_DAY_SCHEDULE",
						"_location": "zcl_closures.h",
						"_callback": "DoorLockClearYearDaySchedule",
						"_clusterID": "0x0101",
						"_required": "false"
					},
					{
						"_id": "0x11",
						"_name": "SetHolidaySchedule",
						"_definition": "COMMAND_DOOR_LOCK_SET_HOLIDAY_SCHEDULE",
						"_location": "zcl_closures.h",
						"_callback": "DoorLockSetHolidaySchedule",
						"_clusterID": "0x0101",
						"_required": "false"
					},
					{
						"_id": "0x12",
						"_name": "GetHolidaySchedule",
						"_definition": "COMMAND_DOOR_LOCK_GET_HOLIDAY_SCHEDULE",
						"_location": "zcl_closures.h",
						"_callback": "DoorLockGetHolidaySchedule",
						"_clusterID": "0x0101",
						"_required": "false"
					},
					{
						"_id": "0x13",
						"_name": "ClearHolidaySchedule",
						"_definition": "COMMAND_DOOR_LOCK_CLEAR_HOLIDAY_SCHEDULE",
						"_location": "zcl_closures.h",
						"_callback": "DoorLockClearHolidaySchedule",
						"_clusterID": "0x0101",
						"_required": "false"
					},
					{
						"_id": "0x14",
						"_name": "SetUserType",
						"_definition": "COMMAND_DOOR_LOCK_SET_USER_TYPE",
						"_location": "zcl_closures.h",
						"_callback": "DoorLockSetUserType",
						"_clusterID": "0x0101",
						"_required": "false"
					},
					{
						"_id": "0x15",
						"_name": "GetUserType",
						"_definition": "COMMAND_DOOR_LOCK_GET_USER_TYPE",
						"_location": "zcl_closures.h",
						"_callback": "DoorLockGetUserType",
						"_clusterID": "0x0101",
						"_required": "false"
					},
					{
						"_id": "0x16",
						"_name": "SetRFIDCode",
						"_definition": "COMMAND_DOOR_LOCK_SET_RFID_CODE",
						"_location": "zcl_closures.h",
						"_callback": "DoorLockSetRFIDCode",
						"_clusterID": "0x0101",
						"_required": "false"
					},
					{
						"_id": "0x17",
						"_name": "GetRFIDCode",
						"_definition": "COMMAND_DOOR_LOCK_GET_RFID_CODE",
						"_location": "zcl_closures.h",
						"_callback": "DoorLockGetRFIDCode",
						"_clusterID": "0x0101",
						"_required": "false"
					},
					{
						"_id": "0x18",
						"_name": "ClearRFIDCode",
						"_definition": "COMMAND_DOOR_LOCK_CLEAR_RFID_CODE",
						"_location": "zcl_closures.h",
						"_callback": "DoorLockClearRFIDCode",
						"_clusterID": "0x0101",
						"_required": "false"
					},
					{
						"_id": "0x19",
						"_name": "ClearAllRFIDCodes",
						"_definition": "COMMAND_DOOR_LOCK_CLEAR_ALL_RFID_CODES",
						"_location": "zcl_closures.h",
						"_callback": "DoorLockClearAllRFIDCodes",
						"_clusterID": "0x0101",
						"_required": "false"
					}
				]
			},
			"client": {
				"attribute": [
					{
						"_id": "0xfffd",
						"_name": "ClusterRevision",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_CLUSTER_REVISION",
						"_location": "zcl.h",
						"_clusterID": "0x0101",
						"_default": "1",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0xfffe",
						"_name": "AttributeReportingStatus",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_ATTRIBUTE_REPORTING_STATUS",
						"_location": "zcl.h",
						"_clusterID": "0x0101",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					}
				],
				"command": [
					{
						"_id": "0x00",
						"_name": "LockDoorResponse",
						"_definition": "COMMAND_DOOR_LOCK_LOCK_DOOR_RESPONSE",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0101",
						"_required": "true"
					},
					{
						"_id": "0x01",
						"_name": "UnlockDoorResponse",
						"_definition": "COMMAND_DOOR_LOCK_UNLOCK_DOOR_RESPONSE",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0101",
						"_required": "true"
					},
					{
						"_id": "0x02",
						"_name": "ToggleResponse",
						"_definition": "COMMAND_DOOR_LOCK_TOGGLE_RESPONSE",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0101",
						"_required": "false"
					},
					{
						"_id": "0x03",
						"_name": "UnlockWithTimeoutResponse",
						"_definition": "COMMAND_DOOR_LOCK_UNLOCK_WITH_TIMEOUT_RESPONSE",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0101",
						"_required": "false"
					},
					{
						"_id": "0x04",
						"_name": "GetLogRecordResponse",
						"_definition": "COMMAND_DOOR_LOCK_GET_LOG_RECORD_RESPONSE",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0101",
						"_required": "false"
					},
					{
						"_id": "0x05",
						"_name": "SetPINCodeResponse",
						"_definition": "COMMAND_DOOR_LOCK_SET_PIN_CODE_RESPONSE",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0101",
						"_required": "false"
					},
					{
						"_id": "0x06",
						"_name": "GetPINCodeResponse",
						"_definition": "COMMAND_DOOR_LOCK_GET_PIN_CODE_RESPONSE",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0101",
						"_required": "false"
					},
					{
						"_id": "0x07",
						"_name": "ClearPINCodeResponse",
						"_definition": "COMMAND_DOOR_LOCK_CLEAR_PIN_CODE_RESPONSE",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0101",
						"_required": "false"
					},
					{
						"_id": "0x08",
						"_name": "ClearAllPINCodesResponse",
						"_definition": "COMMAND_DOOR_LOCK_CLEAR_ALL_PIN_CODES_RESPONSE",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0101",
						"_required": "false"
					},
					{
						"_id": "0x09",
						"_name": "SetUserStatusResponse",
						"_definition": "COMMAND_DOOR_LOCK_SET_USER_STATUS_RESPONSE",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0101",
						"_required": "false"
					},
					{
						"_id": "0x0a",
						"_name": "GetUserStatusResponse",
						"_definition": "COMMAND_DOOR_LOCK_GET_USER_STATUS_RESPONSE",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0101",
						"_required": "false"
					},
					{
						"_id": "0x0b",
						"_name": "SetWeekdayScheduleResponse",
						"_definition": "COMMAND_DOOR_LOCK_SET_WEEKDAY_SCHEDULE_RESPONSE",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0101",
						"_required": "false"
					},
					{
						"_id": "0x0c",
						"_name": "GetWeekdayScheduleResponse",
						"_definition": "COMMAND_DOOR_LOCK_GET_WEEKDAY_SCHEDULE_RESPONSE",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0101",
						"_required": "false"
					},
					{
						"_id": "0x0d",
						"_name": "ClearWeekdayScheduleResponse",
						"_definition": "COMMAND_DOOR_LOCK_CLEAR_WEEKDAY_SCHEDULE_RESPONSE",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0101",
						"_required": "false"
					},
					{
						"_id": "0x0e",
						"_name": "SetYearDayScheduleResponse",
						"_definition": "COMMAND_DOOR_LOCK_SET_YEAR_DAY_SCHEDULE_RESPONSE",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0101",
						"_required": "false"
					},
					{
						"_id": "0x0f",
						"_name": "GetYearDayScheduleResponse",
						"_definition": "COMMAND_DOOR_LOCK_GET_YEAR_DAY_SCHEDULE_RESPONSE",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0101",
						"_required": "false"
					},
					{
						"_id": "0x10",
						"_name": "ClearYearDayScheduleResponse",
						"_definition": "COMMAND_DOOR_LOCK_CLEAR_YEAR_DAY_SCHEDULE_RESPONSE",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0101",
						"_required": "false"
					},
					{
						"_id": "0x11",
						"_name": "SetHolidayScheduleResponse",
						"_definition": "COMMAND_DOOR_LOCK_SET_HOLIDAY_SCHEDULE_RESPONSE",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0101",
						"_required": "false"
					},
					{
						"_id": "0x12",
						"_name": "GetHolidayScheduleResponse",
						"_definition": "COMMAND_DOOR_LOCK_GET_HOLIDAY_SCHEDULE_RESPONSE",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0101",
						"_required": "false"
					},
					{
						"_id": "0x13",
						"_name": "ClearHolidayScheduleResponse",
						"_definition": "COMMAND_DOOR_LOCK_CLEAR_HOLIDAY_SCHEDULE_RESPONSE",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0101",
						"_required": "false"
					},
					{
						"_id": "0x14",
						"_name": "SetUserTypeResponse",
						"_definition": "COMMAND_DOOR_LOCK_SET_USER_TYPE_RESPONSE",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0101",
						"_required": "false"
					},
					{
						"_id": "0x15",
						"_name": "GetUserTypeResponse",
						"_definition": "COMMAND_DOOR_LOCK_GET_USER_TYPE_RESPONSE",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0101",
						"_required": "false"
					},
					{
						"_id": "0x16",
						"_name": "SetRFIDCodeResponse",
						"_definition": "COMMAND_DOOR_LOCK_SET_RFID_CODE_RESPONSE",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0101",
						"_required": "false"
					},
					{
						"_id": "0x17",
						"_name": "GetRFIDCodeResponse",
						"_definition": "COMMAND_DOOR_LOCK_GET_RFID_CODE_RESPONSE",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0101",
						"_required": "false"
					},
					{
						"_id": "0x18",
						"_name": "ClearRFIDCodeResponse",
						"_definition": "COMMAND_DOOR_LOCK_CLEAR_RFID_CODE_RESPONSE",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0101",
						"_required": "false"
					},
					{
						"_id": "0x19",
						"_name": "ClearAllRFIDCodesResponse",
						"_definition": "COMMAND_DOOR_LOCK_CLEAR_ALL_RFID_CODES_RESPONSE",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0101",
						"_required": "false"
					},
					{
						"_id": "0x20",
						"_name": "OperatingEventNotification",
						"_definition": "COMMAND_DOOR_LOCK_OPERATING_EVENT_NOTIFICATION",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0101",
						"_required": "false"
					},
					{
						"_id": "0x21",
						"_name": "ProgrammingEventNotification",
						"_definition": "COMMAND_DOOR_LOCK_PROGRAMMING_EVENT_NOTIFICATION",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0101",
						"_required": "false"
					}
				]
			},
			"appcallback": {
				"callbacks": [
					{
						"_name": "DoorLock",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclIncoming_t *pInMsg, zclDoorLock_t *pInCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Door Lock command"
							}
						]
					},
					{
						"_name": "DoorLockRsp",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclIncoming_t *pInMsg, uint8_t status )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Door Lock Response command"
							}
						]
					},
					{
						"_name": "DoorLockUnlockWithTimeout",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclIncoming_t *pInMsg, zclDoorLockUnlockTimeout_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Unlock With Timeout command"
							}
						]
					},
					{
						"_name": "DoorLockGetLogRecord",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclIncoming_t *pInMsg, zclDoorLockGetLogRecord_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Get Log Record command"
							}
						]
					},
					{
						"_name": "DoorLockSetPINCode",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclIncoming_t *pInMsg, zclDoorLockSetPINCode_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Set PIN Code command"
							}
						]
					},
					{
						"_name": "DoorLockGetPINCode",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclIncoming_t *pInMsg, zclDoorLockUserID_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Get PIN Code command"
							}
						]
					},
					{
						"_name": "DoorLockClearPINCode",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclIncoming_t *pInMsg, zclDoorLockUserID_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Clear PIN Code command"
							}
						]
					},
					{
						"_name": "DoorLockClearAllPINCodes",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclIncoming_t *pInMsg )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Clear All PIN Codes command"
							}
						]
					},
					{
						"_name": "DoorLockSetUserStatus",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclIncoming_t *pInMsg, zclDoorLockSetUserStatus_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Set User Status command"
							}
						]
					},
					{
						"_name": "DoorLockGetUserStatus",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclIncoming_t *pInMsg, zclDoorLockUserID_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Get User Status command"
							}
						]
					},
					{
						"_name": "DoorLockSetWeekDaySchedule",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclIncoming_t *pInMsg, zclDoorLockSetWeekDaySchedule_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Set Week Day Schedule command"
							}
						]
					},
					{
						"_name": "DoorLockGetWeekDaySchedule",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclIncoming_t *pInMsg, zclDoorLockSchedule_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Get Week Day Schedule command"
							}
						]
					},
					{
						"_name": "DoorLockClearWeekDaySchedule",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclIncoming_t *pInMsg, zclDoorLockSchedule_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Clear Week Day Schedule command"
							}
						]
					},
					{
						"_name": "DoorLockSetYearDaySchedule",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclIncoming_t *pInMsg, zclDoorLockSetYearDaySchedule_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Set Year Day Schedule command"
							}
						]
					},
					{
						"_name": "DoorLockGetYearDaySchedule",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclIncoming_t *pInMsg, zclDoorLockSchedule_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Get Year Day Schedule command"
							}
						]
					},
					{
						"_name": "DoorLockClearYearDaySchedule",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclIncoming_t *pInMsg, zclDoorLockSchedule_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Clear Year Day Schedule command"
							}
						]
					},
					{
						"_name": "DoorLockSetHolidaySchedule",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclIncoming_t *pInMsg, zclDoorLockSetHolidaySchedule_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Set Holiday Schedule command"
							}
						]
					},
					{
						"_name": "DoorLockGetHolidaySchedule",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclIncoming_t *pInMsg, zclDoorLockHolidayScheduleID_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Get Holiday Schedule command"
							}
						]
					},
					{
						"_name": "DoorLockClearHolidaySchedule",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclIncoming_t *pInMsg, zclDoorLockHolidayScheduleID_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Clear Holiday Schedule command"
							}
						]
					},
					{
						"_name": "DoorLockSetUserType",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclIncoming_t *pInMsg, zclDoorLockSetUserType_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Set User Type command"
							}
						]
					},
					{
						"_name": "DoorLockGetUserType",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclIncoming_t *pInMsg, zclDoorLockUserID_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Get User Type command"
							}
						]
					},
					{
						"_name": "DoorLockSetRFIDCode",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclIncoming_t *pInMsg, zclDoorLockSetRFIDCode_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Set RFID Code command"
							}
						]
					},
					{
						"_name": "DoorLockGetRFIDCode",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclIncoming_t *pInMsg, zclDoorLockUserID_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Get RFID Code command"
							}
						]
					},
					{
						"_name": "DoorLockClearRFIDCode",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclIncoming_t *pInMsg, zclDoorLockUserID_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Clear RFID Code command"
							}
						]
					},
					{
						"_name": "DoorLockClearAllRFIDCodes",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclIncoming_t *pInMsg )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Clear All RFID Codes command"
							}
						]
					},
					{
						"_name": "DoorLockUnlockWithTimeoutRsp",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclIncoming_t *pInMsg, uint8_t status )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Unlock With Timeout Response command"
							}
						]
					},
					{
						"_name": "DoorLockGetLogRecordRsp",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclIncoming_t *pInMsg, zclDoorLockGetLogRecordRsp_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Get Log Record Response command"
							}
						]
					},
					{
						"_name": "DoorLockSetPINCodeRsp",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclIncoming_t *pInMsg, uint8_t status )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Set PIN Code Response command"
							}
						]
					},
					{
						"_name": "DoorLockGetPINCodeRsp",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclIncoming_t *pInMsg, zclDoorLockGetPINCodeRsp_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Get PIN Code Response command"
							}
						]
					},
					{
						"_name": "DoorLockClearPINCodeRsp",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclIncoming_t *pInMsg, uint8_t status )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Clear PIN Code Response command"
							}
						]
					},
					{
						"_name": "DoorLockClearAllPINCodesRsp",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclIncoming_t *pInMsg, uint8_t status )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Clear All PIN Codes Response command"
							}
						]
					},
					{
						"_name": "DoorLockSetUserStatusRsp",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclIncoming_t *pInMsg, uint8_t status )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Set User Status Response command"
							}
						]
					},
					{
						"_name": "DoorLockGetUserStatusRsp",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclIncoming_t *pInMsg, zclDoorLockGetUserStatusRsp_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Get User Status Response command"
							}
						]
					},
					{
						"_name": "DoorLockSetWeekDayScheduleRsp",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclIncoming_t *pInMsg, uint8_t status )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Set Week Day Schedule Response command"
							}
						]
					},
					{
						"_name": "DoorLockGetWeekDayScheduleRsp",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclIncoming_t *pInMsg, zclDoorLockGetWeekDayScheduleRsp_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Get Week Day Schedule Response command"
							}
						]
					},
					{
						"_name": "DoorLockClearWeekDayScheduleRsp",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclIncoming_t *pInMsg, uint8_t status )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Clear Week Day Schedule Response command"
							}
						]
					},
					{
						"_name": "DoorLockSetYearDayScheduleRsp",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclIncoming_t *pInMsg, uint8_t status )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Set Year Day Schedule Response command"
							}
						]
					},
					{
						"_name": "DoorLockGetYearDayScheduleRsp",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclIncoming_t *pInMsg, zclDoorLockGetYearDayScheduleRsp_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Get Year Day Schedule Response command"
							}
						]
					},
					{
						"_name": "DoorLockClearYearDayScheduleRsp",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclIncoming_t *pInMsg, uint8_t status )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Clear Year Day Schedule Response command"
							}
						]
					},
					{
						"_name": "DoorLockSetHolidayScheduleRsp",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclIncoming_t *pInMsg, uint8_t status )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Set Holiday Schedule Response command"
							}
						]
					},
					{
						"_name": "DoorLockGetHolidayScheduleRsp",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclIncoming_t *pInMsg, zclDoorLockGetHolidayScheduleRsp_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Get Holiday Schedule Response command"
							}
						]
					},
					{
						"_name": "DoorLockClearHolidayScheduleRsp",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclIncoming_t *pInMsg, uint8_t status )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Clear Holiday Schedule Response command"
							}
						]
					},
					{
						"_name": "DoorLockSetUserTypeRsp",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclIncoming_t *pInMsg, uint8_t status )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Set User Type Response command"
							}
						]
					},
					{
						"_name": "DoorLockGetUserTypeRsp",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclIncoming_t *pInMsg, zclDoorLockGetUserTypeRsp_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Get User Type Response command"
							}
						]
					},
					{
						"_name": "DoorLockSetRFIDCodeRsp",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclIncoming_t *pInMsg, uint8_t status )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Set RFID Code Response command"
							}
						]
					},
					{
						"_name": "DoorLockGetRFIDCodeRsp",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclIncoming_t *pInMsg, zclDoorLockGetRFIDCodeRsp_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Get RFID Code Response command"
							}
						]
					},
					{
						"_name": "DoorLockClearRFIDCodeRsp",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclIncoming_t *pInMsg, uint8_t status )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Clear RFID Code Response command"
							}
						]
					},
					{
						"_name": "DoorLockClearAllRFIDCodesRsp",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclIncoming_t *pInMsg, uint8_t status )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Clear All RFID Codes Response command"
							}
						]
					}
				],
				"_name": "zclClosures_DoorLockAppCallbacks_t",
				"_register": "zclClosures_RegisterDoorLockCmdCallbacks"
			},
			"_id": "0x0101",
			"_name": "DoorLock",
			"_definition": "ZCL_CLUSTER_ID_CLOSURES_DOOR_LOCK",
			"_primary_transaction": "2",
			"_class": "closures",
			"_flag": "ZCL_DOORLOCK",
			"_location": "zcl_closures.h"
		},
		{
			"server": {
				"attribute": [
					{
						"_id": "0xfffd",
						"_name": "ClusterRevision",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_CLUSTER_REVISION",
						"_location": "zcl.h",
						"_clusterID": "0x0b04",
						"_default": "1",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0xfffe",
						"_name": "AttributeReportingStatus",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_ATTRIBUTE_REPORTING_STATUS",
						"_location": "zcl.h",
						"_clusterID": "0x0b04",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0000",
						"_name": "MeasurementType",
						"_type": "map32",
						"_zigbeetype": "ZCL_DATATYPE_BITMAP32",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_MEASUREMENT_TYPE ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "00000000",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0x0100",
						"_name": "DCVoltage",
						"_type": "int16",
						"_zigbeetype": "ZCL_DATATYPE_INT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_DC_VOLTAGE ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "-32768",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x0101",
						"_name": "DCVoltageMin",
						"_type": "int16",
						"_zigbeetype": "ZCL_DATATYPE_INT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_DC_VOLTAGE_MIN ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "-32768",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0102",
						"_name": "DCVoltageMax",
						"_type": "int16",
						"_zigbeetype": "ZCL_DATATYPE_INT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_DC_VOLTAGE_MAX ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "-32768",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0103",
						"_name": "DCCurrent",
						"_type": "int16",
						"_zigbeetype": "ZCL_DATATYPE_INT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_DC_CURRENT ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "-32768",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x0104",
						"_name": "DCCurrentMin",
						"_type": "int16",
						"_zigbeetype": "ZCL_DATATYPE_INT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_DC_CURRENT_MIN ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "-32768",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0105",
						"_name": "DCCurrentMax",
						"_type": "int16",
						"_zigbeetype": "ZCL_DATATYPE_INT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_DC_CURRENT_MAX ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "-32768",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0106",
						"_name": "DCPower",
						"_type": "int16",
						"_zigbeetype": "ZCL_DATATYPE_INT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_DC_POWER ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "-32768",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x0107",
						"_name": "DCPowerMin",
						"_type": "int16",
						"_zigbeetype": "ZCL_DATATYPE_INT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_DC_POWER_MIN ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "-32768",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0108",
						"_name": "DCPowerMax",
						"_type": "int16",
						"_zigbeetype": "ZCL_DATATYPE_INT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_DC_POWER_MAX ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "-32768",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0200",
						"_name": "DCVoltageMultiplier",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_DC_VOLTAGE_MULTIPLIER ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "1",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x0201",
						"_name": "DCVoltageDivisor",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_DC_VOLTAGE_DIVISOR ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "1",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x0202",
						"_name": "DCCurrentMultiplier",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_DC_CURRENT_MULTIPLIER ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "1",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x0203",
						"_name": "DCCurrentDivisor",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_DC_CURRENT_DIVISOR ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "1",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x0204",
						"_name": "DCPowerMultiplier",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_DC_POWER_MULTIPLIER ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "1",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x0205",
						"_name": "DCPowerDivisor",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_DC_POWER_DIVISOR ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "1",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x0300",
						"_name": "ACFrequency",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_AC_FREQUENCY ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "65535",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x0301",
						"_name": "ACFrequencyMin",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_AC_FREQUENCY_MIN ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "65535",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0302",
						"_name": "ACFrequencyMax",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_AC_FREQUENCY_MAX ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "65535",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0303",
						"_name": "NeutralCurrent",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_NEUTRAL_CURRENT ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "65535",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x0304",
						"_name": "TotalActivePower",
						"_type": "int32",
						"_zigbeetype": "ZCL_DATATYPE_INT32",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_TOTAL_ACTIVE_POWER ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x0305",
						"_name": "TotalReactivePower",
						"_type": "int32",
						"_zigbeetype": "ZCL_DATATYPE_INT32",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_TOTAL_REACTIVE_POWER ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x0306",
						"_name": "TotalApparentPower",
						"_type": "uint32",
						"_zigbeetype": "ZCL_DATATYPE_UINT32",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_TOTAL_APPARENT_POWER ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x0307",
						"_name": "Measured1stHarmonicCurrent",
						"_type": "int16",
						"_zigbeetype": "ZCL_DATATYPE_INT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_MEASURED_1ST_HARMONIC_CURRENT ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "-32768",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x0308",
						"_name": "Measured3rdHarmonicCurrent",
						"_type": "int16",
						"_zigbeetype": "ZCL_DATATYPE_INT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_MEASURED_3RD_HARMONIC_CURRENT ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "-32768",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x0309",
						"_name": "Measured5thHarmonicCurrent",
						"_type": "int16",
						"_zigbeetype": "ZCL_DATATYPE_INT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_MEASURED_5TH_HARMONIC_CURRENT ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "-32768",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x030a",
						"_name": "Measured7thHarmonicCurrent",
						"_type": "int16",
						"_zigbeetype": "ZCL_DATATYPE_INT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_MEASURED_7TH_HARMONIC_CURRENT ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "-32768",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x030b",
						"_name": "Measured9thHarmonicCurrent",
						"_type": "int16",
						"_zigbeetype": "ZCL_DATATYPE_INT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_MEASURED_9TH_HARMONIC_CURRENT ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "-32768",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x030c",
						"_name": "Measured11thHarmonicCurrent",
						"_type": "int16",
						"_zigbeetype": "ZCL_DATATYPE_INT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_MEASURED_1_1TH_HARMONIC_CURRENT ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "-32768",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x030d",
						"_name": "MeasuredPhase1stHarmonicCurrent",
						"_type": "int16",
						"_zigbeetype": "ZCL_DATATYPE_INT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_MEASURED_PHASE_1ST_HARMONIC_CURRENT ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "-32768",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x030e",
						"_name": "MeasuredPhase3rdHarmonicCurrent",
						"_type": "int16",
						"_zigbeetype": "ZCL_DATATYPE_INT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_MEASURED_PHASE_3RD_HARMONIC_CURRENT ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "-32768",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x030f",
						"_name": "MeasuredPhase5thHarmonicCurrent",
						"_type": "int16",
						"_zigbeetype": "ZCL_DATATYPE_INT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_MEASURED_PHASE_5TH_HARMONIC_CURRENT ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "-32768",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x0310",
						"_name": "MeasuredPhase7thHarmonicCurrent",
						"_type": "int16",
						"_zigbeetype": "ZCL_DATATYPE_INT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_MEASURED_PHASE_7TH_HARMONIC_CURRENT ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "-32768",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x0311",
						"_name": "MeasuredPhase9thHarmonicCurrent",
						"_type": "int16",
						"_zigbeetype": "ZCL_DATATYPE_INT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_MEASURED_PHASE_9TH_HARMONIC_CURRENT ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "-32768",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x0312",
						"_name": "MeasuredPhase11thHarmonicCurrent",
						"_type": "int16",
						"_zigbeetype": "ZCL_DATATYPE_INT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_MEASURED_PHASE_1_1TH_HARMONIC_CURRENT ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "-32768",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x0400",
						"_name": "ACFrequencyMultiplier",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_AC_FREQUENCY_MULTIPLIER ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "1",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x0401",
						"_name": "ACFrequencyDivisor",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_AC_FREQUENCY_DIVISOR ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "1",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x0402",
						"_name": "PowerMultiplier",
						"_type": "uint32",
						"_zigbeetype": "ZCL_DATATYPE_UINT32",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_POWER_MULTIPLIER ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "1",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x0403",
						"_name": "PowerDivisor",
						"_type": "uint32",
						"_zigbeetype": "ZCL_DATATYPE_UINT32",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_POWER_DIVISOR ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "1",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x0404",
						"_name": "HarmonicCurrentMultiplier",
						"_type": "int8",
						"_zigbeetype": "ZCL_DATATYPE_INT8",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_HARMONIC_CURRENT_MULTIPLIER ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x0405",
						"_name": "PhaseHarmonicCurrentMultiplier",
						"_type": "int8",
						"_zigbeetype": "ZCL_DATATYPE_INT8",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_PHASE_HARMONIC_CURRENT_MULTIPLIER ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x0501",
						"_name": "LineCurrent",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_LINE_CURRENT ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "65535",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x0502",
						"_name": "ActiveCurrent",
						"_type": "int16",
						"_zigbeetype": "ZCL_DATATYPE_INT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_ACTIVE_CURRENT ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "-32768",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x0503",
						"_name": "ReactiveCurrent",
						"_type": "int16",
						"_zigbeetype": "ZCL_DATATYPE_INT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_REACTIVE_CURRENT ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "-32768",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x0505",
						"_name": "RMSVoltage",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_RMS_VOLTAGE ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "65535",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x0506",
						"_name": "RMSVoltageMin",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_RMS_VOLTAGE_MIN ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "65535",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0507",
						"_name": "RMSVoltageMax",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_RMS_VOLTAGE_MAX ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "65535",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0508",
						"_name": "RMSCurrent",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_RMS_CURRENT ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "65535",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x0509",
						"_name": "RMSCurrentMin",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_RMS_CURRENT_MIN ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "65535",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x050a",
						"_name": "RMSCurrentMax",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_RMS_CURRENT_MAX ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "65535",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x050b",
						"_name": "ActivePower",
						"_type": "int16",
						"_zigbeetype": "ZCL_DATATYPE_INT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_ACTIVE_POWER ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "-32768",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x050c",
						"_name": "ActivePowerMin",
						"_type": "int16",
						"_zigbeetype": "ZCL_DATATYPE_INT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_ACTIVE_POWER_MIN ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "-32768",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x050d",
						"_name": "ActivePowerMax",
						"_type": "int16",
						"_zigbeetype": "ZCL_DATATYPE_INT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_ACTIVE_POWER_MAX ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "-32768",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x050e",
						"_name": "ReactivePower",
						"_type": "int16",
						"_zigbeetype": "ZCL_DATATYPE_INT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_REACTIVE_POWER ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "-32768",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x050f",
						"_name": "ApparentPower",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_APPARENT_POWER ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "65535",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x0510",
						"_name": "PowerFactor",
						"_type": "int8",
						"_zigbeetype": "ZCL_DATATYPE_INT8",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_POWER_FACTOR ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0511",
						"_name": "AverageRMSVoltageMeasurementPeriod",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_AVERAGE_RMS_VOLTAGE_MEASUREMENT_PERIOD ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0512",
						"_name": "AverageRMSOverVoltageCounter",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_AVERAGE_RMS_OVER_VOLTAGE_COUNTER ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0513",
						"_name": "AverageRMSUnderVoltageCounter",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_AVERAGE_RMS_UNDER_VOLTAGE_COUNTER ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0514",
						"_name": "RMSExtremeOverVoltagePeriod",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_RMS_EXTREME_OVER_VOLTAGE_PERIOD ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0515",
						"_name": "RMSExtremeUnderVoltagePeriod",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_RMS_EXTREME_UNDER_VOLTAGE_PERIOD ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0516",
						"_name": "RMSVoltageSagPeriod",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_RMS_VOLTAGE_SAG_PERIOD ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0517",
						"_name": "RMSVoltageSwellPeriod",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_RMS_VOLTAGE_SWELL_PERIOD ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0600",
						"_name": "ACVoltageMultiplier",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_AC_VOLTAGE_MULTIPLIER ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "1",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x0601",
						"_name": "ACVoltageDivisor",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_AC_VOLTAGE_DIVISOR ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "1",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x0602",
						"_name": "ACCurrentMultiplier",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_AC_CURRENT_MULTIPLIER ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "1",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x0603",
						"_name": "ACCurrentDivisor",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_AC_CURRENT_DIVISOR ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "1",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x0604",
						"_name": "ACPowerMultiplier",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_AC_POWER_MULTIPLIER ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "1",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x0605",
						"_name": "ACPowerDivisor",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_AC_POWER_DIVISOR ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "1",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x0700",
						"_name": "DCOverloadAlarmsMask",
						"_type": "map8",
						"_zigbeetype": "ZCL_DATATYPE_BITMAP8",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_DC_OVERLOAD_ALARMS_MASK ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0701",
						"_name": "DCVoltageOverload",
						"_type": "int16",
						"_zigbeetype": "ZCL_DATATYPE_INT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_DC_VOLTAGE_OVERLOAD ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "-1",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0702",
						"_name": "DCCurrentOverload",
						"_type": "int16",
						"_zigbeetype": "ZCL_DATATYPE_INT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_DC_CURRENT_OVERLOAD ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "-1",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0800",
						"_name": "ACAlarmsMask",
						"_type": "map16",
						"_zigbeetype": "ZCL_DATATYPE_BITMAP16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_AC_ALARMS_MASK ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "0000",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0801",
						"_name": "ACVoltageOverload",
						"_type": "int16",
						"_zigbeetype": "ZCL_DATATYPE_INT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_AC_VOLTAGE_OVERLOAD ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "-1",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0802",
						"_name": "ACCurrentOverload",
						"_type": "int16",
						"_zigbeetype": "ZCL_DATATYPE_INT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_AC_CURRENT_OVERLOAD ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "-1",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0803",
						"_name": "ACActivePowerOverload",
						"_type": "int16",
						"_zigbeetype": "ZCL_DATATYPE_INT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_AC_ACTIVE_POWER_OVERLOAD ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "-1",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0804",
						"_name": "ACReactivePowerOverload",
						"_type": "int16",
						"_zigbeetype": "ZCL_DATATYPE_INT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_AC_REACTIVE_POWER_OVERLOAD ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "-1",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0805",
						"_name": "AverageRMSOverVoltage",
						"_type": "int16",
						"_zigbeetype": "ZCL_DATATYPE_INT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_AVERAGE_RMS_OVER_VOLTAGE ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0806",
						"_name": "AverageRMSUnderVoltage",
						"_type": "int16",
						"_zigbeetype": "ZCL_DATATYPE_INT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_AVERAGE_RMS_UNDER_VOLTAGE ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0807",
						"_name": "RMSExtremeOverVoltage",
						"_type": "int16",
						"_zigbeetype": "ZCL_DATATYPE_INT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_RMS_EXTREME_OVER_VOLTAGE ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0808",
						"_name": "RMSExtremeUnderVoltage",
						"_type": "int16",
						"_zigbeetype": "ZCL_DATATYPE_INT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_RMS_EXTREME_UNDER_VOLTAGE ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0809",
						"_name": "RMSVoltageSag",
						"_type": "int16",
						"_zigbeetype": "ZCL_DATATYPE_INT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_RMS_VOLTAGE_SAG ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x080a",
						"_name": "RMSVoltageSwell",
						"_type": "int16",
						"_zigbeetype": "ZCL_DATATYPE_INT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_RMS_VOLTAGE_SWELL ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0901",
						"_name": "LineCurrentPhB",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_LINE_CURRENT_PH_B ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "65535",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x0902",
						"_name": "ActiveCurrentPhB",
						"_type": "int16",
						"_zigbeetype": "ZCL_DATATYPE_INT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_ACTIVE_CURRENT_PH_B ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "-32768",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x0903",
						"_name": "ReactiveCurrentPhB",
						"_type": "int16",
						"_zigbeetype": "ZCL_DATATYPE_INT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_REACTIVE_CURRENT_PH_B ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "-32768",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x0905",
						"_name": "RMSVoltagePhB",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_RMS_VOLTAGE_PH_B ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "65535",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x0906",
						"_name": "RMSVoltageMinPhB",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_RMS_VOLTAGE_MIN_PH_B ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "32768",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0907",
						"_name": "RMSVoltageMaxPhB",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_RMS_VOLTAGE_MAX_PH_B ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "32768",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0908",
						"_name": "RMSCurrentPhB",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_RMS_CURRENT_PH_B ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "65535",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x0909",
						"_name": "RMSCurrentMinPhB",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_RMS_CURRENT_MIN_PH_B ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "65535",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x090a",
						"_name": "RMSCurrentMaxPhB",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_RMS_CURRENT_MAX_PH_B ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "65535",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x090b",
						"_name": "ActivePowerPhB",
						"_type": "int16",
						"_zigbeetype": "ZCL_DATATYPE_INT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_ACTIVE_POWER_PH_B ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "-32768",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x090c",
						"_name": "ActivePowerMinPhB",
						"_type": "int16",
						"_zigbeetype": "ZCL_DATATYPE_INT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_ACTIVE_POWER_MIN_PH_B ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "-32768",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x090d",
						"_name": "ActivePowerMaxPhB",
						"_type": "int16",
						"_zigbeetype": "ZCL_DATATYPE_INT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_ACTIVE_POWER_MAX_PH_B ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "-32768",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x090e",
						"_name": "ReactivePowerPhB",
						"_type": "int16",
						"_zigbeetype": "ZCL_DATATYPE_INT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_REACTIVE_POWER_PH_B ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "-32768",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x090f",
						"_name": "ApparentPowerPhB",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_APPARENT_POWER_PH_B ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "65535",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x0910",
						"_name": "PowerFactorPhB",
						"_type": "int8",
						"_zigbeetype": "ZCL_DATATYPE_INT8",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_POWER_FACTOR_PH_B ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0911",
						"_name": "AverageRMSVoltageMeasurementPeriodPhB",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_AVERAGE_RMS_VOLTAGE_MEASUREMENT_PERIOD_PH_B ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0912",
						"_name": "AverageRMSOverVoltageCounterPhB",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_AVERAGE_RMS_OVER_VOLTAGE_COUNTER_PH_B ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0913",
						"_name": "AverageRMSUnderVoltageCounterPhB",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_AVERAGE_RMS_UNDER_VOLTAGE_COUNTER_PH_B ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0914",
						"_name": "RMSExtremeOverVoltagePeriodPhB",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_RMS_EXTREME_OVER_VOLTAGE_PERIOD_PH_B ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0915",
						"_name": "RMSExtremeUnderVoltagePeriodPhB",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_RMS_EXTREME_UNDER_VOLTAGE_PERIOD_PH_B ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0916",
						"_name": "RMSVoltageSagPeriodPhB",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_RMS_VOLTAGE_SAG_PERIOD_PH_B ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0917",
						"_name": "RMSVoltageSwellPeriodPhB",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_RMS_VOLTAGE_SWELL_PERIOD_PH_B ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0a01",
						"_name": "LineCurrentPhC",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_LINE_CURRENT_PH_C ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "65535",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x0a02",
						"_name": "ActiveCurrentPhC",
						"_type": "int16",
						"_zigbeetype": "ZCL_DATATYPE_INT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_ACTIVE_CURRENT_PH_C ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "-32768",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x0a03",
						"_name": "ReactiveCurrentPhC",
						"_type": "int16",
						"_zigbeetype": "ZCL_DATATYPE_INT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_REACTIVE_CURRENT_PH_C ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "-32768",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x0a05",
						"_name": "RMSVoltagePhC",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_RMS_VOLTAGE_PH_C ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "65535",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x0a06",
						"_name": "RMSVoltageMinPhC",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_RMS_VOLTAGE_MIN_PH_C ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "32768",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0a07",
						"_name": "RMSVoltageMaxPhC",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_RMS_VOLTAGE_MAX_PH_C ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "32768",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0a08",
						"_name": "RMSCurrentPhC",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_RMS_CURRENT_PH_C ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "65535",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x0a09",
						"_name": "RMSCurrentMinPhC",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_RMS_CURRENT_MIN_PH_C ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "65535",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0a0a",
						"_name": "RMSCurrentMaxPhC",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_RMS_CURRENT_MAX_PH_C ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "65535",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0a0b",
						"_name": "ActivePowerPhC",
						"_type": "int16",
						"_zigbeetype": "ZCL_DATATYPE_INT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_ACTIVE_POWER_PH_C ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "-32768",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x0a0c",
						"_name": "ActivePowerMinPhC",
						"_type": "int16",
						"_zigbeetype": "ZCL_DATATYPE_INT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_ACTIVE_POWER_MIN_PH_C ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "-32768",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0a0d",
						"_name": "ActivePowerMaxPhC",
						"_type": "int16",
						"_zigbeetype": "ZCL_DATATYPE_INT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_ACTIVE_POWER_MAX_PH_C ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "-32768",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0a0e",
						"_name": "ReactivePowerPhC",
						"_type": "int16",
						"_zigbeetype": "ZCL_DATATYPE_INT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_REACTIVE_POWER_PH_C ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "-32768",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x0a0f",
						"_name": "ApparentPowerPhC",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_APPARENT_POWER_PH_C ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "65535",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x0a10",
						"_name": "PowerFactorPhC",
						"_type": "int8",
						"_zigbeetype": "ZCL_DATATYPE_INT8",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_POWER_FACTOR_PH_C ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0a11",
						"_name": "AverageRMSVoltageMeasurementPeriodPhC",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_AVERAGE_RMS_VOLTAGE_MEASUREMENT_PERIOD_PH_C ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0a12",
						"_name": "AverageRMSOverVoltageCounterPhC",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_AVERAGE_RMS_OVER_VOLTAGE_COUNTER_PH_C ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0a13",
						"_name": "AverageRMSUnderVoltageCounterPhC",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_AVERAGE_RMS_UNDER_VOLTAGE_COUNTER_PH_C ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0a14",
						"_name": "RMSExtremeOverVoltagePeriodPhC",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_RMS_EXTREME_OVER_VOLTAGE_PERIOD_PH_C ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0a15",
						"_name": "RMSExtremeUnderVoltagePeriodPhC",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_RMS_EXTREME_UNDER_VOLTAGE_PERIOD_PH_C ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0a16",
						"_name": "RMSVoltageSagPeriodPhC",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_RMS_VOLTAGE_SAG_PERIOD_PH_C ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0a17",
						"_name": "RMSVoltageSwellPeriodPhC",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_ELECTRICAL_MEASUREMENT_RMS_VOLTAGE_SWELL_PERIOD_PH_C ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					}
				],
				"command": [
					{
						"_id": "0x00",
						"_name": "GetProfileInfo",
						"_definition": "COMMAND_ELECTRICAL_MEASUREMENT_GET_PROFILE_INFO",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_required": "false"
					},
					{
						"_id": "0x01",
						"_name": "GetMeasurementProfile",
						"_definition": "COMMAND_ELECTRICAL_MEASUREMENT_GET_MEASUREMENT_PROFILE",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_required": "false"
					}
				]
			},
			"client": {
				"attribute": [
					{
						"_id": "0xfffd",
						"_name": "ClusterRevision",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_CLUSTER_REVISION",
						"_location": "zcl.h",
						"_clusterID": "0x0b04",
						"_default": "1",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0xfffe",
						"_name": "AttributeReportingStatus",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_ATTRIBUTE_REPORTING_STATUS",
						"_location": "zcl.h",
						"_clusterID": "0x0b04",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					}
				],
				"command": [
					{
						"_id": "0x00",
						"_name": "GetProfileInfoResponse",
						"_definition": "COMMAND_ELECTRICAL_MEASUREMENT_GET_PROFILE_INFO_RESPONSE",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_required": "false"
					},
					{
						"_id": "0x01",
						"_name": "GetMeasurementProfileResponse",
						"_definition": "COMMAND_ELECTRICAL_MEASUREMENT_GET_MEASUREMENT_PROFILE_RESPONSE",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0b04",
						"_required": "false"
					}
				]
			},
			"appcallback": {
				"callbacks": [
					{
						"_name": "MSPlaceHolder",
						"_returnType": "void",
						"_arguments": "( void )"
					}
				],
				"_name": "zclMS_AppCallbacks_t",
				"_register": "zclMS_RegisterCmdCallbacks"
			},
			"_id": "0x0b04",
			"_name": "ElectricalMeasurement",
			"_definition": "ZCL_CLUSTER_ID_MS_ELECTRICAL_MEASUREMENT",
			"_primary_transaction": "1",
			"_class": "ms",
			"_flag": "ZCL_ELECTRICAL_MEASUREMENT",
			"_location": "zcl_ms.h"
		},
		{
			"server": {
				"attribute": [
					{
						"_id": "0xfffd",
						"_name": "ClusterRevision",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_CLUSTER_REVISION",
						"_location": "zcl.h",
						"_clusterID": "0x0202",
						"_default": "1",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0xfffe",
						"_name": "AttributeReportingStatus",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_ATTRIBUTE_REPORTING_STATUS",
						"_location": "zcl.h",
						"_clusterID": "0x0202",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0000",
						"_name": "FanMode",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_FAN_CONTROL_FAN_MODE ",
						"_location": "zcl_hvac.h",
						"_clusterID": "0x0202",
						"_default": "5",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0x0001",
						"_name": "FanModeSequence",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_FAN_CONTROL_FAN_MODE_SEQUENCE ",
						"_location": "zcl_hvac.h",
						"_clusterID": "0x0202",
						"_default": "2",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "true"
					}
				]
			},
			"client": {
				"attribute": [
					{
						"_id": "0xfffd",
						"_name": "ClusterRevision",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_CLUSTER_REVISION",
						"_location": "zcl.h",
						"_clusterID": "0x0202",
						"_default": "1",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0xfffe",
						"_name": "AttributeReportingStatus",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_ATTRIBUTE_REPORTING_STATUS",
						"_location": "zcl.h",
						"_clusterID": "0x0202",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					}
				],
			},
			"appcallback": {
				"callbacks": [
					{
						"_name": "HVAC_SetpointRaiseLower",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclCmdThermostatSetpointRaiseLowerPayload_t *pCmd )",
						"comments": [
							{
								"_comment": "// This callback is called to process a Setpoint Raise/Lower command"
							}
						]
					},
					{
						"_name": "HVAC_SetWeeklySchedule",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclThermostatWeeklySchedule_t *pCmd )",
						"comments": [
							{
								"_comment": "// This callback is called to process a Set Weekly Schedule command"
							}
						]
					},
					{
						"_name": "HVAC_GetWeeklySchedule",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclThermostatGetWeeklySchedule_t *pCmd )",
						"comments": [
							{
								"_comment": "// This callback is called to process a Get Weekly Schedule command"
							}
						]
					},
					{
						"_name": "HVAC_ClearWeeklySchedule",
						"_returnType": "ZStatus_t",
						"_arguments": "( void )",
						"comments": [
							{
								"_comment": "// This callback is called to process a Clear Weekly Schedule command"
							}
						]
					},
					{
						"_name": "HVAC_GetRelayStatusLog",
						"_returnType": "ZStatus_t",
						"_arguments": "( void )",
						"comments": [
							{
								"_comment": "// This callback is called to process a Get Relay Status Log command"
							}
						]
					},
					{
						"_name": "HVAC_GetWeeklyScheduleRsp",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclThermostatWeeklySchedule_t *pCmd )",
						"comments": [
							{
								"_comment": "// This callback is called to process a Get Weekly Schedule Response command"
							}
						]
					},
					{
						"_name": "HVAC_GetRelayStatusLogRsp",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclThermostatGetRelayStatusLogRsp_t *pCmd )",
						"comments": [
							{
								"_comment": "// This callback is called to process a Get Relay Status Log Response command"
							}
						]
					}
				],
				"_name": "zclHVAC_AppCallbacks_t",
				"_register": "zclHVAC_RegisterCmdCallbacks"
			},
			"_id": "0x0202",
			"_name": "FanControl",
			"_definition": "ZCL_CLUSTER_ID_HVAC_FAN_CONTROL",
			"_primary_transaction": "1",
			"_class": "hvac",
			"_flag": "ZCL_HVAC_CLUSTER",
			"_location": "zcl_hvac.h"
		},
		{
			"server": {
				"attribute": [
					{
						"_id": "0xfffd",
						"_name": "ClusterRevision",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_CLUSTER_REVISION",
						"_location": "zcl.h",
						"_clusterID": "0x0404",
						"_default": "1",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0xfffe",
						"_name": "AttributeReportingStatus",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_ATTRIBUTE_REPORTING_STATUS",
						"_location": "zcl.h",
						"_clusterID": "0x0404",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0000",
						"_name": "MeasuredValue",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_FLOW_MEASUREMENT_MEASURED_VALUE ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0404",
						"_default": "65535",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "true",
						"_required": "true"
					},
					{
						"_id": "0x0001",
						"_name": "MinMeasuredValue",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_FLOW_MEASUREMENT_MIN_MEASURED_VALUE ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0404",
						"_default": "65535",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0x0002",
						"_name": "MaxMeasuredValue",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_FLOW_MEASUREMENT_MAX_MEASURED_VALUE ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0404",
						"_default": "65535",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0x0003",
						"_name": "Tolerance",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_FLOW_MEASUREMENT_TOLERANCE ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0404",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					}
				]
			},
			"client": {
				"attribute": [
					{
						"_id": "0xfffd",
						"_name": "ClusterRevision",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_CLUSTER_REVISION",
						"_location": "zcl.h",
						"_clusterID": "0x0404",
						"_default": "1",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0xfffe",
						"_name": "AttributeReportingStatus",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_ATTRIBUTE_REPORTING_STATUS",
						"_location": "zcl.h",
						"_clusterID": "0x0404",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					}
				],
			},
			"appcallback": {
				"callbacks": [
					{
						"_name": "MSPlaceHolder",
						"_returnType": "void",
						"_arguments": "( void )"
					}
				],
				"_name": "zclMS_AppCallbacks_t",
				"_register": "zclMS_RegisterCmdCallbacks"
			},
			"_id": "0x0404",
			"_name": "FlowMeasurement",
			"_definition": "ZCL_CLUSTER_ID_MS_FLOW_MEASUREMENT",
			"_primary_transaction": "2",
			"_class": "ms",
			"_flag": "ZCL_MS",
			"_location": "zcl_ms.h"
		},
		{
			"server": {
				"attribute": [
					{
						"_id": "0xfffd",
						"_name": "ClusterRevision",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_CLUSTER_REVISION",
						"_location": "zcl.h",
						"_clusterID": "0x0004",
						"_default": "1",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0xfffe",
						"_name": "AttributeReportingStatus",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_ATTRIBUTE_REPORTING_STATUS",
						"_location": "zcl.h",
						"_clusterID": "0x0004",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0000",
						"_name": "NameSupport",
						"_type": "map8",
						"_zigbeetype": "ZCL_DATATYPE_BITMAP8",
						"_definition": "ATTRID_GROUPS_NAME_SUPPORT ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0004",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					}
				],
				"command": [
					{
						"_id": "0x00",
						"_name": "AddGroup",
						"_definition": "COMMAND_GROUPS_ADD_GROUP",
						"_location": "zcl_general.h",
						"_clusterID": "0x0004",
						"_required": "true"
					},
					{
						"_id": "0x01",
						"_name": "ViewGroup",
						"_definition": "COMMAND_GROUPS_VIEW_GROUP",
						"_location": "zcl_general.h",
						"_clusterID": "0x0004",
						"_required": "true"
					},
					{
						"_id": "0x02",
						"_name": "GetGroupMembership",
						"_definition": "COMMAND_GROUPS_GET_GROUP_MEMBERSHIP",
						"_location": "zcl_general.h",
						"_clusterID": "0x0004",
						"_required": "true"
					},
					{
						"_id": "0x03",
						"_name": "RemoveGroup",
						"_definition": "COMMAND_GROUPS_REMOVE_GROUP",
						"_location": "zcl_general.h",
						"_clusterID": "0x0004",
						"_required": "true"
					},
					{
						"_id": "0x04",
						"_name": "RemoveAllGroups",
						"_definition": "COMMAND_GROUPS_REMOVE_ALL_GROUPS",
						"_location": "zcl_general.h",
						"_clusterID": "0x0004",
						"_required": "true"
					},
					{
						"_id": "0x05",
						"_name": "AddGroupIfIdentifying",
						"_definition": "COMMAND_GROUPS_ADD_GROUP_IF_IDENTIFYING",
						"_location": "zcl_general.h",
						"_clusterID": "0x0004",
						"_required": "true"
					}
				]
			},
			"client": {
				"attribute": [
					{
						"_id": "0xfffd",
						"_name": "ClusterRevision",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_CLUSTER_REVISION",
						"_location": "zcl.h",
						"_clusterID": "0x0004",
						"_default": "1",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0xfffe",
						"_name": "AttributeReportingStatus",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_ATTRIBUTE_REPORTING_STATUS",
						"_location": "zcl.h",
						"_clusterID": "0x0004",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					}
				],
				"command": [
					{
						"_id": "0x00",
						"_name": "AddGroupResponse",
						"_definition": "COMMAND_GROUPS_ADD_GROUP_RESPONSE",
						"_location": "zcl_general.h",
						"_clusterID": "0x0004",
						"_required": "true"
					},
					{
						"_id": "0x01",
						"_name": "ViewGroupResponse",
						"_definition": "COMMAND_GROUPS_VIEW_GROUP_RESPONSE",
						"_location": "zcl_general.h",
						"_clusterID": "0x0004",
						"_required": "true"
					},
					{
						"_id": "0x02",
						"_name": "GetGroupMembershipResponse",
						"_definition": "COMMAND_GROUPS_GET_GROUP_MEMBERSHIP_RESPONSE",
						"_location": "zcl_general.h",
						"_clusterID": "0x0004",
						"_required": "true"
					},
					{
						"_id": "0x03",
						"_name": "RemoveGroupResponse",
						"_definition": "COMMAND_GROUPS_REMOVE_GROUP_RESPONSE",
						"_location": "zcl_general.h",
						"_clusterID": "0x0004",
						"_required": "true"
					}
				]
			},
			"appcallback": {
				"callbacks": [
					{
						"_name": "BasicReset",
						"_returnType": "void",
						"_arguments": "( void )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Reset to Factory Defaults"
							},
							{
								"_comment": "/// command. On receipt of this command, the device resets all the attributes"
							},
							{
								"_comment": "/// of all its clusters to their factory defaults."
							}
						]
					},
					{
						"_name": "Identify",
						"_returnType": "void",
						"_arguments": "( zclIdentify_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Identify command."
							}
						]
					},
					{
						"_name": "IdentifyQuery",
						"_returnType": "void",
						"_arguments": "( zclIdentifyQuery_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Identify Query command."
							}
						]
					},
					{
						"_name": "IdentifyQueryRsp",
						"_returnType": "void",
						"_arguments": "( zclIdentifyQueryRsp_t *pRsp )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Identify Query Response command."
							}
						]
					},
					{
						"_name": "IdentifyTriggerEffect",
						"_returnType": "void",
						"_arguments": "( zclIdentifyTriggerEffect_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Identify Trigger Effect command."
							}
						]
					},
					{
						"_name": "#ifdef ZCL_ON_OFF",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "OnOff",
						"_returnType": "void",
						"_arguments": "( uint8_t cmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming On, Off or Toggle command."
							}
						]
					},
					{
						"_name": "OnOff_OffWithEffect",
						"_returnType": "void",
						"_arguments": "( zclOffWithEffect_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Off with Effect"
							}
						]
					},
					{
						"_name": "OnOff_OnWithRecallGlobalScene",
						"_returnType": "void",
						"_arguments": "( void )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming On with Recall Global Scene command."
							}
						]
					},
					{
						"_name": "OnOff_OnWithTimedOff",
						"_returnType": "void",
						"_arguments": "( zclOnWithTimedOff_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming On with Timed Off."
							}
						]
					},
					{
						"_name": "#endif",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "#ifdef ZCL_LEVEL_CTRL",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "LevelControlMoveToLevel",
						"_returnType": "void",
						"_arguments": "( zclLCMoveToLevel_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Level Control - Move to Level command"
							}
						]
					},
					{
						"_name": "LevelControlMove",
						"_returnType": "void",
						"_arguments": "( zclLCMove_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Level Control - Move command"
							}
						]
					},
					{
						"_name": "LevelControlStep",
						"_returnType": "void",
						"_arguments": "( zclLCStep_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Level Control - Step command"
							}
						]
					},
					{
						"_name": "LevelControlStop",
						"_returnType": "void",
						"_arguments": "( zclLCStop_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Level Control - Stop command"
							}
						]
					},
					{
						"_name": "LevelControlMoveFreq",
						"_returnType": "void",
						"_arguments": "( zclLCMoveFreq_t *pCmd)",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Level Control - Move to Closest Frequency command"
							}
						]
					},
					{
						"_name": "#endif",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "#ifdef ZCL_GROUPS",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "GroupRsp",
						"_returnType": "void",
						"_arguments": "( zclGroupRsp_t *pRsp )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an received Group Response message."
							},
							{
								"_comment": "/// This means that this app sent the request message."
							}
						]
					},
					{
						"_name": "#endif",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "#ifdef ZCL_SCENES",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "SceneStoreReq",
						"_returnType": "uint8_t",
						"_arguments": "( zclSceneReq_t *pReq )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Scene Store request."
							},
							{
								"_comment": "/// The app will fill in the 'extField' with what is needed to restore its"
							},
							{
								"_comment": "/// current settings. Before overwriting the content of the scene,"
							},
							{
								"_comment": "/// App needs to verify if there was a change with the previous configuration."
							},
							{
								"_comment": "/// App function returns TRUE if there was a change, otherwise returns FALSE."
							}
						]
					},
					{
						"_name": "SceneRecallReq",
						"_returnType": "void",
						"_arguments": "( zclSceneReq_t *pReq )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Scene Recall request"
							},
							{
								"_comment": "/// The app will use what's in the 'extField' to restore to these settings."
							}
						]
					},
					{
						"_name": "SceneRsp",
						"_returnType": "void",
						"_arguments": "( zclSceneRsp_t *pRsp )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Scene responses. This means"
							},
							{
								"_comment": "/// that this app sent the request for this response."
							}
						]
					},
					{
						"_name": "#endif",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "#ifdef ZCL_ALARMS",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "Alarm",
						"_returnType": "void",
						"_arguments": "( uint8_t direction, zclAlarm_t *pAlarm )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Alarm request or response command."
							}
						]
					},
					{
						"_name": "#endif",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "#ifdef SE_UK_EXT",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "GetEventLog",
						"_returnType": "void",
						"_arguments": "( uint8_t srcEP, afAddrType_t *srcAddr, zclGetEventLog_t *pEventLog, uint8_t seqNum )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Alarm Get Event Log command."
							}
						]
					},
					{
						"_name": "PublishEventLog",
						"_returnType": "void",
						"_arguments": "( afAddrType_t *srcAddr, zclPublishEventLog_t *pEventLog )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Alarm Publish Event Log command."
							}
						]
					},
					{
						"_name": "#endif",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "Location",
						"_returnType": "void",
						"_arguments": "( zclLocation_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to to process an incoming RSSI Location command."
							}
						]
					},
					{
						"_name": "LocationRsp",
						"_returnType": "void",
						"_arguments": "( zclLocationRsp_t *pRsp )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming RSSI Location response command."
							},
							{
								"_comment": "/// This means  that this app sent the request for this response."
							}
						]
					}
				],
				"_name": "zclGeneral_AppCallbacks_t",
				"_register": "zclGeneral_RegisterCmdCallbacks"
			},
			"_id": "0x0004",
			"_name": "Groups",
			"_definition": "ZCL_CLUSTER_ID_GENERAL_GROUPS",
			"_primary_transaction": "0",
			"_class": "general",
			"_flag": "ZCL_GROUPS",
			"_location": "zcl_general.h"
		},
		{
			"server": {
				"attribute": [
					{
						"_id": "0xfffd",
						"_name": "ClusterRevision",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_CLUSTER_REVISION",
						"_location": "zcl.h",
						"_clusterID": "0x0501",
						"_default": "1",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0xfffe",
						"_name": "AttributeReportingStatus",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_ATTRIBUTE_REPORTING_STATUS",
						"_location": "zcl.h",
						"_clusterID": "0x0501",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					}
				],
				"command": [
					{
						"_id": "0x00",
						"_name": "Arm",
						"_definition": "COMMAND_IASACE_ARM",
						"_location": "zcl_ss.h",
						"_callback": "ACE_Arm",
						"_clusterID": "0x0501",
						"_required": "true"
					},
					{
						"_id": "0x01",
						"_name": "Bypass",
						"_definition": "COMMAND_IASACE_BYPASS",
						"_location": "zcl_ss.h",
						"_callback": "ACE_Bypass",
						"_clusterID": "0x0501",
						"_required": "true"
					},
					{
						"_id": "0x02",
						"_name": "Emergency",
						"_definition": "COMMAND_IASACE_EMERGENCY",
						"_location": "zcl_ss.h",
						"_callback": "ACE_Emergency",
						"_clusterID": "0x0501",
						"_required": "true"
					},
					{
						"_id": "0x03",
						"_name": "Fire",
						"_definition": "COMMAND_IASACE_FIRE",
						"_location": "zcl_ss.h",
						"_callback": "ACE_Fire",
						"_clusterID": "0x0501",
						"_required": "true"
					},
					{
						"_id": "0x04",
						"_name": "Panic",
						"_definition": "COMMAND_IASACE_PANIC",
						"_location": "zcl_ss.h",
						"_callback": "ACE_Panic",
						"_clusterID": "0x0501",
						"_required": "true"
					},
					{
						"_id": "0x05",
						"_name": "GetZoneIDMap",
						"_definition": "COMMAND_IASACE_GET_ZONE_ID_MAP",
						"_location": "zcl_ss.h",
						"_callback": "ACE_GetZoneIDMap",
						"_clusterID": "0x0501",
						"_required": "true"
					},
					{
						"_id": "0x06",
						"_name": "GetZoneInformation",
						"_definition": "COMMAND_IASACE_GET_ZONE_INFORMATION",
						"_location": "zcl_ss.h",
						"_callback": "ACE_GetZoneInformation",
						"_clusterID": "0x0501",
						"_required": "true"
					},
					{
						"_id": "0x07",
						"_name": "GetPanelStatus",
						"_definition": "COMMAND_IASACE_GET_PANEL_STATUS",
						"_location": "zcl_ss.h",
						"_callback": "ACE_GetPanelStatus",
						"_clusterID": "0x0501",
						"_required": "true"
					},
					{
						"_id": "0x08",
						"_name": "GetBypassedZoneList",
						"_definition": "COMMAND_IASACE_GET_BYPASSED_ZONE_LIST",
						"_location": "zcl_ss.h",
						"_callback": "ACE_GetBypassedZoneList",
						"_clusterID": "0x0501",
						"_required": "true"
					},
					{
						"_id": "0x09",
						"_name": "GetZoneStatus",
						"_definition": "COMMAND_IASACE_GET_ZONE_STATUS",
						"_location": "zcl_ss.h",
						"_callback": "ACE_GetZoneStatus",
						"_clusterID": "0x0501",
						"_required": "true"
					}
				]
			},
			"client": {
				"attribute": [
					{
						"_id": "0xfffd",
						"_name": "ClusterRevision",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_CLUSTER_REVISION",
						"_location": "zcl.h",
						"_clusterID": "0x0501",
						"_default": "1",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0xfffe",
						"_name": "AttributeReportingStatus",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_ATTRIBUTE_REPORTING_STATUS",
						"_location": "zcl.h",
						"_clusterID": "0x0501",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					}
				],
				"command": [
					{
						"_id": "0x00",
						"_name": "ArmResponse",
						"_definition": "COMMAND_IASACE_ARM_RESPONSE",
						"_location": "zcl_ss.h",
						"_clusterID": "0x0501",
						"_required": "true"
					},
					{
						"_id": "0x01",
						"_name": "GetZoneIDMapResponse",
						"_definition": "COMMAND_IASACE_GET_ZONE_ID_MAP_RESPONSE",
						"_location": "zcl_ss.h",
						"_clusterID": "0x0501",
						"_required": "true"
					},
					{
						"_id": "0x02",
						"_name": "GetZoneInformationResponse",
						"_definition": "COMMAND_IASACE_GET_ZONE_INFORMATION_RESPONSE",
						"_location": "zcl_ss.h",
						"_clusterID": "0x0501",
						"_required": "true"
					},
					{
						"_id": "0x03",
						"_name": "ZoneStatusChanged",
						"_definition": "COMMAND_IASACE_ZONE_STATUS_CHANGED",
						"_location": "zcl_ss.h",
						"_clusterID": "0x0501",
						"_required": "true"
					},
					{
						"_id": "0x04",
						"_name": "PanelStatusChanged",
						"_definition": "COMMAND_IASACE_PANEL_STATUS_CHANGED",
						"_location": "zcl_ss.h",
						"_clusterID": "0x0501",
						"_required": "true"
					},
					{
						"_id": "0x05",
						"_name": "GetPanelStatusResponse",
						"_definition": "COMMAND_IASACE_GET_PANEL_STATUS_RESPONSE",
						"_location": "zcl_ss.h",
						"_clusterID": "0x0501",
						"_required": "true"
					},
					{
						"_id": "0x06",
						"_name": "SetBypassedZoneList",
						"_definition": "COMMAND_IASACE_SET_BYPASSED_ZONE_LIST",
						"_location": "zcl_ss.h",
						"_clusterID": "0x0501",
						"_required": "true"
					},
					{
						"_id": "0x07",
						"_name": "BypassResponse",
						"_definition": "COMMAND_IASACE_BYPASS_RESPONSE",
						"_location": "zcl_ss.h",
						"_clusterID": "0x0501",
						"_required": "true"
					},
					{
						"_id": "0x08",
						"_name": "GetZoneStatusResponse",
						"_definition": "COMMAND_IASACE_GET_ZONE_STATUS_RESPONSE",
						"_location": "zcl_ss.h",
						"_clusterID": "0x0501",
						"_required": "true"
					}
				]
			},
			"appcallback": {
				"callbacks": [
					{
						"_name": "ChangeNotification",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclZoneChangeNotif_t *pCmd, afAddrType_t *srcAddr )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Change Notification command"
							}
						]
					},
					{
						"_name": "EnrollRequest",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclZoneEnrollReq_t *pReq, uint8_t endpoint )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Enroll Request command"
							}
						]
					},
					{
						"_name": "EnrollResponse",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclZoneEnrollRsp_t *pRsp )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Enroll Response command"
							}
						]
					},
					{
						"_name": "InitNormalOpMode",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclIncoming_t *pInMsg )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Initiate Normal Operation Mode command"
							}
						]
					},
					{
						"_name": "InitTestMode",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclZoneInitTestMode_t *pCmd, zclIncoming_t *pInMsg )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Initiate Test Mode Response command"
							}
						]
					},
					{
						"_name": "ACE_Arm",
						"_returnType": "uint8_t",
						"_arguments": "( zclACEArm_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an Arm command"
							}
						]
					},
					{
						"_name": "ACE_Bypass",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclACEBypass_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Bypass command"
							}
						]
					},
					{
						"_name": "ACE_Emergency",
						"_returnType": "ZStatus_t",
						"_arguments": "( void )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an Emergency command"
							}
						]
					},
					{
						"_name": "ACE_Fire",
						"_returnType": "ZStatus_t",
						"_arguments": "( void )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Fire command"
							}
						]
					},
					{
						"_name": "ACE_Panic",
						"_returnType": "ZStatus_t",
						"_arguments": "( void )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Panic command"
							}
						]
					},
					{
						"_name": "ACE_GetZoneIDMap",
						"_returnType": "ZStatus_t",
						"_arguments": "( void )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Get Zone ID Map command"
							}
						]
					},
					{
						"_name": "ACE_GetZoneInformation",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclIncoming_t *pInMsg )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Get Zone Information command"
							}
						]
					},
					{
						"_name": "ACE_GetPanelStatus",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclIncoming_t *pInMsg )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Get Panel Status command"
							}
						]
					},
					{
						"_name": "ACE_GetBypassedZoneList",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclIncoming_t *pInMsg )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Get Bypassed Zone List command"
							}
						]
					},
					{
						"_name": "ACE_GetZoneStatus",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclIncoming_t *pInMsg )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Get Zone Status command"
							}
						]
					},
					{
						"_name": "ACE_ArmResponse",
						"_returnType": "ZStatus_t",
						"_arguments": "( uint8_t armNotification )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an Arm Response command"
							}
						]
					},
					{
						"_name": "ACE_GetZoneIDMapResponse",
						"_returnType": "ZStatus_t",
						"_arguments": "( uint16_t *zoneIDMap )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Get Zone ID Map Response command"
							}
						]
					},
					{
						"_name": "ACE_GetZoneInformationResponse",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclACEGetZoneInfoRsp_t *pRsp )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Get Zone Information Response command"
							}
						]
					},
					{
						"_name": "ACE_ZoneStatusChanged",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclACEZoneStatusChanged_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Zone Status Changed command"
							}
						]
					},
					{
						"_name": "ACE_PanelStatusChanged",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclACEPanelStatusChanged_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Panel Status Changed command"
							}
						]
					},
					{
						"_name": "ACE_GetPanelStatusResponse",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclACEGetPanelStatusRsp_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Get Panel Status Response command"
							}
						]
					},
					{
						"_name": "ACE_SetBypassedZoneList",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclACESetBypassedZoneList_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Set Bypassed Zone List command"
							}
						]
					},
					{
						"_name": "ACE_BypassResponse",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclACEBypassRsp_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an Bypass Response command"
							}
						]
					},
					{
						"_name": "ACE_GetZoneStatusResponse",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclACEGetZoneStatusRsp_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an Get Zone Status Response command"
							}
						]
					},
					{
						"_name": "WD_StartWarning",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclWDStartWarning_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Start Warning command"
							}
						]
					},
					{
						"_name": "WD_Squawk",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclWDSquawk_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Squawk command"
							}
						]
					}
				],
				"_name": "zclSS_AppCallbacks_t",
				"_register": "zclSS_RegisterCmdCallbacks"
			},
			"_id": "0x0501",
			"_name": "IASACE",
			"_definition": "ZCL_CLUSTER_ID_SS_IAS_ACE",
			"_primary_transaction": "1",
			"_class": "ss",
			"_flag": "ZCL_ACE",
			"_location": "zcl_ss.h"
		},
		{
			"server": {
				"attribute": [
					{
						"_id": "0xfffd",
						"_name": "ClusterRevision",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_CLUSTER_REVISION",
						"_location": "zcl.h",
						"_clusterID": "0x0502",
						"_default": "1",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0xfffe",
						"_name": "AttributeReportingStatus",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_ATTRIBUTE_REPORTING_STATUS",
						"_location": "zcl.h",
						"_clusterID": "0x0502",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0000",
						"_name": "MaxDuration",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_IASWD_MAX_DURATION ",
						"_location": "zcl_ss.h",
						"_clusterID": "0x0502",
						"_default": "240",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "true"
					}
				],
				"command": [
					{
						"_id": "0x00",
						"_name": "StartWarning",
						"_definition": "COMMAND_IASWD_START_WARNING",
						"_location": "zcl_ss.h",
						"_callback": "WD_StartWarning",
						"_clusterID": "0x0502",
						"_required": "true"
					},
					{
						"_id": "0x01",
						"_name": "Squawk",
						"_definition": "COMMAND_IASWD_SQUAWK",
						"_location": "zcl_ss.h",
						"_callback": "WD_Squawk",
						"_clusterID": "0x0502",
						"_required": "true"
					}
				]
			},
			"client": {
				"attribute": [
					{
						"_id": "0xfffd",
						"_name": "ClusterRevision",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_CLUSTER_REVISION",
						"_location": "zcl.h",
						"_clusterID": "0x0502",
						"_default": "1",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0xfffe",
						"_name": "AttributeReportingStatus",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_ATTRIBUTE_REPORTING_STATUS",
						"_location": "zcl.h",
						"_clusterID": "0x0502",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					}
				],
			},
			"appcallback": {
				"callbacks": [
					{
						"_name": "ChangeNotification",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclZoneChangeNotif_t *pCmd, afAddrType_t *srcAddr )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Change Notification command"
							}
						]
					},
					{
						"_name": "EnrollRequest",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclZoneEnrollReq_t *pReq, uint8_t endpoint )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Enroll Request command"
							}
						]
					},
					{
						"_name": "EnrollResponse",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclZoneEnrollRsp_t *pRsp )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Enroll Response command"
							}
						]
					},
					{
						"_name": "InitNormalOpMode",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclIncoming_t *pInMsg )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Initiate Normal Operation Mode command"
							}
						]
					},
					{
						"_name": "InitTestMode",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclZoneInitTestMode_t *pCmd, zclIncoming_t *pInMsg )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Initiate Test Mode Response command"
							}
						]
					},
					{
						"_name": "ACE_Arm",
						"_returnType": "uint8_t",
						"_arguments": "( zclACEArm_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an Arm command"
							}
						]
					},
					{
						"_name": "ACE_Bypass",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclACEBypass_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Bypass command"
							}
						]
					},
					{
						"_name": "ACE_Emergency",
						"_returnType": "ZStatus_t",
						"_arguments": "( void )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an Emergency command"
							}
						]
					},
					{
						"_name": "ACE_Fire",
						"_returnType": "ZStatus_t",
						"_arguments": "( void )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Fire command"
							}
						]
					},
					{
						"_name": "ACE_Panic",
						"_returnType": "ZStatus_t",
						"_arguments": "( void )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Panic command"
							}
						]
					},
					{
						"_name": "ACE_GetZoneIDMap",
						"_returnType": "ZStatus_t",
						"_arguments": "( void )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Get Zone ID Map command"
							}
						]
					},
					{
						"_name": "ACE_GetZoneInformation",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclIncoming_t *pInMsg )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Get Zone Information command"
							}
						]
					},
					{
						"_name": "ACE_GetPanelStatus",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclIncoming_t *pInMsg )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Get Panel Status command"
							}
						]
					},
					{
						"_name": "ACE_GetBypassedZoneList",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclIncoming_t *pInMsg )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Get Bypassed Zone List command"
							}
						]
					},
					{
						"_name": "ACE_GetZoneStatus",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclIncoming_t *pInMsg )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Get Zone Status command"
							}
						]
					},
					{
						"_name": "ACE_ArmResponse",
						"_returnType": "ZStatus_t",
						"_arguments": "( uint8_t armNotification )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an Arm Response command"
							}
						]
					},
					{
						"_name": "ACE_GetZoneIDMapResponse",
						"_returnType": "ZStatus_t",
						"_arguments": "( uint16_t *zoneIDMap )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Get Zone ID Map Response command"
							}
						]
					},
					{
						"_name": "ACE_GetZoneInformationResponse",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclACEGetZoneInfoRsp_t *pRsp )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Get Zone Information Response command"
							}
						]
					},
					{
						"_name": "ACE_ZoneStatusChanged",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclACEZoneStatusChanged_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Zone Status Changed command"
							}
						]
					},
					{
						"_name": "ACE_PanelStatusChanged",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclACEPanelStatusChanged_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Panel Status Changed command"
							}
						]
					},
					{
						"_name": "ACE_GetPanelStatusResponse",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclACEGetPanelStatusRsp_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Get Panel Status Response command"
							}
						]
					},
					{
						"_name": "ACE_SetBypassedZoneList",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclACESetBypassedZoneList_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Set Bypassed Zone List command"
							}
						]
					},
					{
						"_name": "ACE_BypassResponse",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclACEBypassRsp_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an Bypass Response command"
							}
						]
					},
					{
						"_name": "ACE_GetZoneStatusResponse",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclACEGetZoneStatusRsp_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an Get Zone Status Response command"
							}
						]
					},
					{
						"_name": "WD_StartWarning",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclWDStartWarning_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Start Warning command"
							}
						]
					},
					{
						"_name": "WD_Squawk",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclWDSquawk_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Squawk command"
							}
						]
					}
				],
				"_name": "zclSS_AppCallbacks_t",
				"_register": "zclSS_RegisterCmdCallbacks"
			},
			"_id": "0x0502",
			"_name": "IASWD",
			"_definition": "ZCL_CLUSTER_ID_SS_IAS_WD",
			"_primary_transaction": "1",
			"_class": "ss",
			"_flag": "ZCL_WD",
			"_location": "zcl_ss.h"
		},
		{
			"server": {
				"attribute": [
					{
						"_id": "0xfffd",
						"_name": "ClusterRevision",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_CLUSTER_REVISION",
						"_location": "zcl.h",
						"_clusterID": "0x0500",
						"_default": "1",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0xfffe",
						"_name": "AttributeReportingStatus",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_ATTRIBUTE_REPORTING_STATUS",
						"_location": "zcl.h",
						"_clusterID": "0x0500",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0000",
						"_name": "ZoneState",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_IAS_ZONE_ZONE_STATE ",
						"_location": "zcl_ss.h",
						"_clusterID": "0x0500",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0x0001",
						"_name": "ZoneType",
						"_type": "IasZoneType",
						"_zigbeetype": "ZCL_DATATYPE_ENUM16",
						"_definition": "ATTRID_IAS_ZONE_ZONE_TYPE ",
						"_location": "zcl_ss.h",
						"_clusterID": "0x0500",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0x0002",
						"_name": "ZoneStatus",
						"_type": "IasZoneStatus",
						"_zigbeetype": "ZCL_DATATYPE_BITMAP16",
						"_definition": "ATTRID_IAS_ZONE_ZONE_STATUS ",
						"_location": "zcl_ss.h",
						"_clusterID": "0x0500",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0x0010",
						"_name": "IASCIEAddress",
						"_type": "EUI64",
						"_zigbeetype": "ZCL_DATATYPE_BITMAP64",
						"_definition": "ATTRID_IAS_ZONE_IASCIE_ADDRESS ",
						"_location": "zcl_ss.h",
						"_clusterID": "0x0500",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0x0011",
						"_name": "ZoneID",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_IAS_ZONE_ZONE_ID ",
						"_location": "zcl_ss.h",
						"_clusterID": "0x0500",
						"_default": "255",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0x0012",
						"_name": "NumberOfZoneSensitivityLevelsSupported",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_IAS_ZONE_NUMBER_OF_ZONE_SENSITIVITY_LEVELS_SUPPORTED ",
						"_location": "zcl_ss.h",
						"_clusterID": "0x0500",
						"_default": "2",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0013",
						"_name": "CurrentZoneSensitivityLevel",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_IAS_ZONE_CURRENT_ZONE_SENSITIVITY_LEVEL ",
						"_location": "zcl_ss.h",
						"_clusterID": "0x0500",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					}
				],
				"command": [
					{
						"_id": "0x00",
						"_name": "ZoneEnrollResponse",
						"_definition": "COMMAND_IAS_ZONE_ZONE_ENROLL_RESPONSE",
						"_location": "zcl_ss.h",
						"_callback": "EnrollResponse",
						"_clusterID": "0x0500",
						"_required": "true"
					},
					{
						"_id": "0x01",
						"_name": "InitiateNormalOperationMode",
						"_definition": "COMMAND_IAS_ZONE_INITIATE_NORMAL_OPERATION_MODE",
						"_location": "zcl_ss.h",
						"_callback": "InitNormalOpMode",
						"_clusterID": "0x0500",
						"_required": "false"
					},
					{
						"_id": "0x02",
						"_name": "InitiateTestMode",
						"_definition": "COMMAND_IAS_ZONE_INITIATE_TEST_MODE",
						"_location": "zcl_ss.h",
						"_callback": "InitTestMode",
						"_clusterID": "0x0500",
						"_required": "false"
					}
				]
			},
			"client": {
				"attribute": [
					{
						"_id": "0xfffd",
						"_name": "ClusterRevision",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_CLUSTER_REVISION",
						"_location": "zcl.h",
						"_clusterID": "0x0500",
						"_default": "1",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0xfffe",
						"_name": "AttributeReportingStatus",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_ATTRIBUTE_REPORTING_STATUS",
						"_location": "zcl.h",
						"_clusterID": "0x0500",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					}
				],
				"command": [
					{
						"_id": "0x00",
						"_name": "ZoneStatusChangeNotification",
						"_definition": "COMMAND_IAS_ZONE_ZONE_STATUS_CHANGE_NOTIFICATION",
						"_location": "zcl_ss.h",
						"_clusterID": "0x0500",
						"_required": "true"
					},
					{
						"_id": "0x01",
						"_name": "ZoneEnrollRequest",
						"_definition": "COMMAND_IAS_ZONE_ZONE_ENROLL_REQUEST",
						"_location": "zcl_ss.h",
						"_clusterID": "0x0500",
						"_required": "true"
					}
				]
			},
			"appcallback": {
				"callbacks": [
					{
						"_name": "ChangeNotification",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclZoneChangeNotif_t *pCmd, afAddrType_t *srcAddr )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Change Notification command"
							}
						]
					},
					{
						"_name": "EnrollRequest",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclZoneEnrollReq_t *pReq, uint8_t endpoint )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Enroll Request command"
							}
						]
					},
					{
						"_name": "EnrollResponse",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclZoneEnrollRsp_t *pRsp )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Enroll Response command"
							}
						]
					},
					{
						"_name": "InitNormalOpMode",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclIncoming_t *pInMsg )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Initiate Normal Operation Mode command"
							}
						]
					},
					{
						"_name": "InitTestMode",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclZoneInitTestMode_t *pCmd, zclIncoming_t *pInMsg )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Initiate Test Mode Response command"
							}
						]
					},
					{
						"_name": "ACE_Arm",
						"_returnType": "uint8_t",
						"_arguments": "( zclACEArm_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an Arm command"
							}
						]
					},
					{
						"_name": "ACE_Bypass",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclACEBypass_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Bypass command"
							}
						]
					},
					{
						"_name": "ACE_Emergency",
						"_returnType": "ZStatus_t",
						"_arguments": "( void )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an Emergency command"
							}
						]
					},
					{
						"_name": "ACE_Fire",
						"_returnType": "ZStatus_t",
						"_arguments": "( void )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Fire command"
							}
						]
					},
					{
						"_name": "ACE_Panic",
						"_returnType": "ZStatus_t",
						"_arguments": "( void )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Panic command"
							}
						]
					},
					{
						"_name": "ACE_GetZoneIDMap",
						"_returnType": "ZStatus_t",
						"_arguments": "( void )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Get Zone ID Map command"
							}
						]
					},
					{
						"_name": "ACE_GetZoneInformation",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclIncoming_t *pInMsg )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Get Zone Information command"
							}
						]
					},
					{
						"_name": "ACE_GetPanelStatus",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclIncoming_t *pInMsg )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Get Panel Status command"
							}
						]
					},
					{
						"_name": "ACE_GetBypassedZoneList",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclIncoming_t *pInMsg )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Get Bypassed Zone List command"
							}
						]
					},
					{
						"_name": "ACE_GetZoneStatus",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclIncoming_t *pInMsg )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Get Zone Status command"
							}
						]
					},
					{
						"_name": "ACE_ArmResponse",
						"_returnType": "ZStatus_t",
						"_arguments": "( uint8_t armNotification )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an Arm Response command"
							}
						]
					},
					{
						"_name": "ACE_GetZoneIDMapResponse",
						"_returnType": "ZStatus_t",
						"_arguments": "( uint16_t *zoneIDMap )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Get Zone ID Map Response command"
							}
						]
					},
					{
						"_name": "ACE_GetZoneInformationResponse",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclACEGetZoneInfoRsp_t *pRsp )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Get Zone Information Response command"
							}
						]
					},
					{
						"_name": "ACE_ZoneStatusChanged",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclACEZoneStatusChanged_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Zone Status Changed command"
							}
						]
					},
					{
						"_name": "ACE_PanelStatusChanged",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclACEPanelStatusChanged_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Panel Status Changed command"
							}
						]
					},
					{
						"_name": "ACE_GetPanelStatusResponse",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclACEGetPanelStatusRsp_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Get Panel Status Response command"
							}
						]
					},
					{
						"_name": "ACE_SetBypassedZoneList",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclACESetBypassedZoneList_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Set Bypassed Zone List command"
							}
						]
					},
					{
						"_name": "ACE_BypassResponse",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclACEBypassRsp_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an Bypass Response command"
							}
						]
					},
					{
						"_name": "ACE_GetZoneStatusResponse",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclACEGetZoneStatusRsp_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an Get Zone Status Response command"
							}
						]
					},
					{
						"_name": "WD_StartWarning",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclWDStartWarning_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Start Warning command"
							}
						]
					},
					{
						"_name": "WD_Squawk",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclWDSquawk_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Squawk command"
							}
						]
					}
				],
				"_name": "zclSS_AppCallbacks_t",
				"_register": "zclSS_RegisterCmdCallbacks"
			},
			"_id": "0x0500",
			"_name": "IASZone",
			"_definition": "ZCL_CLUSTER_ID_SS_IAS_ZONE",
			"_primary_transaction": "2",
			"_class": "ss",
			"_flag": "ZCL_ZONE",
			"_location": "zcl_ss.h"
		},
		{
			"server": {
				"attribute": [
					{
						"_id": "0xfffd",
						"_name": "ClusterRevision",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_CLUSTER_REVISION",
						"_location": "zcl.h",
						"_clusterID": "0x0003",
						"_default": "1",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0xfffe",
						"_name": "AttributeReportingStatus",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_ATTRIBUTE_REPORTING_STATUS",
						"_location": "zcl.h",
						"_clusterID": "0x0003",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0000",
						"_name": "IdentifyTime",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_IDENTIFY_IDENTIFY_TIME ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0003",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "true"
					}
				],
				"command": [
					{
						"_id": "0x00",
						"_name": "Identify",
						"_definition": "COMMAND_IDENTIFY_IDENTIFY",
						"_location": "zcl_general.h",
						"_callback": "Identify",
						"_clusterID": "0x0003",
						"_required": "true"
					},
					{
						"_id": "0x01",
						"_name": "IdentifyQuery",
						"_definition": "COMMAND_IDENTIFY_IDENTIFY_QUERY",
						"_location": "zcl_general.h",
						"_callback": "IdentifyQuery",
						"_clusterID": "0x0003",
						"_required": "true"
					},
					{
						"_id": "0x40",
						"_name": "TriggerEffect",
						"_definition": "COMMAND_IDENTIFY_TRIGGER_EFFECT",
						"_location": "zcl_general.h",
						"_callback": "IdentifyTriggerEffect",
						"_clusterID": "0x0003",
						"_required": "false"
					}
				]
			},
			"client": {
				"attribute": [
					{
						"_id": "0xfffd",
						"_name": "ClusterRevision",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_CLUSTER_REVISION",
						"_location": "zcl.h",
						"_clusterID": "0x0003",
						"_default": "1",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0xfffe",
						"_name": "AttributeReportingStatus",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_ATTRIBUTE_REPORTING_STATUS",
						"_location": "zcl.h",
						"_clusterID": "0x0003",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					}
				],
				"command": [
					{
						"_id": "0x00",
						"_name": "IdentifyQueryResponse",
						"_definition": "COMMAND_IDENTIFY_IDENTIFY_QUERY_RESPONSE",
						"_location": "zcl_general.h",
						"_clusterID": "0x0003",
						"_required": "true"
					}
				]
			},
			"appcallback": {
				"callbacks": [
					{
						"_name": "BasicReset",
						"_returnType": "void",
						"_arguments": "( void )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Reset to Factory Defaults"
							},
							{
								"_comment": "/// command. On receipt of this command, the device resets all the attributes"
							},
							{
								"_comment": "/// of all its clusters to their factory defaults."
							}
						]
					},
					{
						"_name": "Identify",
						"_returnType": "void",
						"_arguments": "( zclIdentify_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Identify command."
							}
						]
					},
					{
						"_name": "IdentifyQuery",
						"_returnType": "void",
						"_arguments": "( zclIdentifyQuery_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Identify Query command."
							}
						]
					},
					{
						"_name": "IdentifyQueryRsp",
						"_returnType": "void",
						"_arguments": "( zclIdentifyQueryRsp_t *pRsp )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Identify Query Response command."
							}
						]
					},
					{
						"_name": "IdentifyTriggerEffect",
						"_returnType": "void",
						"_arguments": "( zclIdentifyTriggerEffect_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Identify Trigger Effect command."
							}
						]
					},
					{
						"_name": "#ifdef ZCL_ON_OFF",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "OnOff",
						"_returnType": "void",
						"_arguments": "( uint8_t cmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming On, Off or Toggle command."
							}
						]
					},
					{
						"_name": "OnOff_OffWithEffect",
						"_returnType": "void",
						"_arguments": "( zclOffWithEffect_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Off with Effect"
							}
						]
					},
					{
						"_name": "OnOff_OnWithRecallGlobalScene",
						"_returnType": "void",
						"_arguments": "( void )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming On with Recall Global Scene command."
							}
						]
					},
					{
						"_name": "OnOff_OnWithTimedOff",
						"_returnType": "void",
						"_arguments": "( zclOnWithTimedOff_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming On with Timed Off."
							}
						]
					},
					{
						"_name": "#endif",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "#ifdef ZCL_LEVEL_CTRL",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "LevelControlMoveToLevel",
						"_returnType": "void",
						"_arguments": "( zclLCMoveToLevel_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Level Control - Move to Level command"
							}
						]
					},
					{
						"_name": "LevelControlMove",
						"_returnType": "void",
						"_arguments": "( zclLCMove_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Level Control - Move command"
							}
						]
					},
					{
						"_name": "LevelControlStep",
						"_returnType": "void",
						"_arguments": "( zclLCStep_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Level Control - Step command"
							}
						]
					},
					{
						"_name": "LevelControlStop",
						"_returnType": "void",
						"_arguments": "( zclLCStop_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Level Control - Stop command"
							}
						]
					},
					{
						"_name": "LevelControlMoveFreq",
						"_returnType": "void",
						"_arguments": "( zclLCMoveFreq_t *pCmd)",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Level Control - Move to Closest Frequency command"
							}
						]
					},
					{
						"_name": "#endif",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "#ifdef ZCL_GROUPS",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "GroupRsp",
						"_returnType": "void",
						"_arguments": "( zclGroupRsp_t *pRsp )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an received Group Response message."
							},
							{
								"_comment": "/// This means that this app sent the request message."
							}
						]
					},
					{
						"_name": "#endif",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "#ifdef ZCL_SCENES",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "SceneStoreReq",
						"_returnType": "uint8_t",
						"_arguments": "( zclSceneReq_t *pReq )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Scene Store request."
							},
							{
								"_comment": "/// The app will fill in the 'extField' with what is needed to restore its"
							},
							{
								"_comment": "/// current settings. Before overwriting the content of the scene,"
							},
							{
								"_comment": "/// App needs to verify if there was a change with the previous configuration."
							},
							{
								"_comment": "/// App function returns TRUE if there was a change, otherwise returns FALSE."
							}
						]
					},
					{
						"_name": "SceneRecallReq",
						"_returnType": "void",
						"_arguments": "( zclSceneReq_t *pReq )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Scene Recall request"
							},
							{
								"_comment": "/// The app will use what's in the 'extField' to restore to these settings."
							}
						]
					},
					{
						"_name": "SceneRsp",
						"_returnType": "void",
						"_arguments": "( zclSceneRsp_t *pRsp )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Scene responses. This means"
							},
							{
								"_comment": "/// that this app sent the request for this response."
							}
						]
					},
					{
						"_name": "#endif",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "#ifdef ZCL_ALARMS",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "Alarm",
						"_returnType": "void",
						"_arguments": "( uint8_t direction, zclAlarm_t *pAlarm )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Alarm request or response command."
							}
						]
					},
					{
						"_name": "#endif",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "#ifdef SE_UK_EXT",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "GetEventLog",
						"_returnType": "void",
						"_arguments": "( uint8_t srcEP, afAddrType_t *srcAddr, zclGetEventLog_t *pEventLog, uint8_t seqNum )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Alarm Get Event Log command."
							}
						]
					},
					{
						"_name": "PublishEventLog",
						"_returnType": "void",
						"_arguments": "( afAddrType_t *srcAddr, zclPublishEventLog_t *pEventLog )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Alarm Publish Event Log command."
							}
						]
					},
					{
						"_name": "#endif",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "Location",
						"_returnType": "void",
						"_arguments": "( zclLocation_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to to process an incoming RSSI Location command."
							}
						]
					},
					{
						"_name": "LocationRsp",
						"_returnType": "void",
						"_arguments": "( zclLocationRsp_t *pRsp )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming RSSI Location response command."
							},
							{
								"_comment": "/// This means  that this app sent the request for this response."
							}
						]
					}
				],
				"_name": "zclGeneral_AppCallbacks_t",
				"_register": "zclGeneral_RegisterCmdCallbacks"
			},
			"_id": "0x0003",
			"_name": "Identify",
			"_definition": "ZCL_CLUSTER_ID_GENERAL_IDENTIFY",
			"_primary_transaction": "0",
			"_class": "general",
			"_flag": "ZCL_IDENTIFY",
			"_location": "zcl_general.h"
		},
		{
			"server": {
				"attribute": [
					{
						"_id": "0xfffd",
						"_name": "ClusterRevision",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_CLUSTER_REVISION",
						"_location": "zcl.h",
						"_clusterID": "0x0401",
						"_default": "1",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0xfffe",
						"_name": "AttributeReportingStatus",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_ATTRIBUTE_REPORTING_STATUS",
						"_location": "zcl.h",
						"_clusterID": "0x0401",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0000",
						"_name": "LevelStatus",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_ILLUMINANCE_LEVEL_SENSING_LEVEL_STATUS ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0401",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "true",
						"_required": "true"
					},
					{
						"_id": "0x0001",
						"_name": "LightSensorType",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_ILLUMINANCE_LEVEL_SENSING_LIGHT_SENSOR_TYPE ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0401",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0010",
						"_name": "IlluminanceTargetLevel",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_ILLUMINANCE_LEVEL_SENSING_ILLUMINANCE_TARGET_LEVEL ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0401",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "true"
					}
				]
			},
			"client": {
				"attribute": [
					{
						"_id": "0xfffd",
						"_name": "ClusterRevision",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_CLUSTER_REVISION",
						"_location": "zcl.h",
						"_clusterID": "0x0401",
						"_default": "1",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0xfffe",
						"_name": "AttributeReportingStatus",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_ATTRIBUTE_REPORTING_STATUS",
						"_location": "zcl.h",
						"_clusterID": "0x0401",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					}
				],
			},
			"appcallback": {
				"callbacks": [
					{
						"_name": "MSPlaceHolder",
						"_returnType": "void",
						"_arguments": "( void )"
					}
				],
				"_name": "zclMS_AppCallbacks_t",
				"_register": "zclMS_RegisterCmdCallbacks"
			},
			"_id": "0x0401",
			"_name": "IlluminanceLevelSensing",
			"_definition": "ZCL_CLUSTER_ID_MS_ILLUMINANCE_LEVEL_SENSING_CONFIG",
			"_primary_transaction": "2",
			"_class": "ms",
			"_flag": "ZCL_MS",
			"_location": "zcl_ms.h"
		},
		{
			"server": {
				"attribute": [
					{
						"_id": "0xfffd",
						"_name": "ClusterRevision",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_CLUSTER_REVISION",
						"_location": "zcl.h",
						"_clusterID": "0x0400",
						"_default": "1",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0xfffe",
						"_name": "AttributeReportingStatus",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_ATTRIBUTE_REPORTING_STATUS",
						"_location": "zcl.h",
						"_clusterID": "0x0400",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0000",
						"_name": "MeasuredValue",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_ILLUMINANCE_MEASUREMENT_MEASURED_VALUE ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0400",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "true",
						"_required": "true"
					},
					{
						"_id": "0x0001",
						"_name": "MinMeasuredValue",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_ILLUMINANCE_MEASUREMENT_MIN_MEASURED_VALUE ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0400",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0x0002",
						"_name": "MaxMeasuredValue",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_ILLUMINANCE_MEASUREMENT_MAX_MEASURED_VALUE ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0400",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0x0003",
						"_name": "Tolerance",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_ILLUMINANCE_MEASUREMENT_TOLERANCE ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0400",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0004",
						"_name": "LightSensorType",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_ILLUMINANCE_MEASUREMENT_LIGHT_SENSOR_TYPE ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0400",
						"_default": "255",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					}
				]
			},
			"appcallback": {
				"callbacks": [
					{
						"_name": "MSPlaceHolder",
						"_returnType": "void",
						"_arguments": "( void )"
					}
				],
				"_name": "zclMS_AppCallbacks_t",
				"_register": "zclMS_RegisterCmdCallbacks"
			},
			"_id": "0x0400",
			"_name": "IlluminanceMeasurement",
			"_definition": "ZCL_CLUSTER_ID_MS_ILLUMINANCE_MEASUREMENT",
			"_primary_transaction": "2",
			"_class": "ms",
			"_flag": "ZCL_MS",
			"_location": "zcl_ms.h"
		},
		{
			"server": {
				"attribute": [
					{
						"_id": "0xfffd",
						"_name": "ClusterRevision",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_CLUSTER_REVISION",
						"_location": "zcl.h",
						"_clusterID": "0x0008",
						"_default": "1",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0xfffe",
						"_name": "AttributeReportingStatus",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_ATTRIBUTE_REPORTING_STATUS",
						"_location": "zcl.h",
						"_clusterID": "0x0008",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0000",
						"_name": "CurrentLevel",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_LEVEL_CURRENT_LEVEL ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0008",
						"_default": "255",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "true",
						"_required": "true"
					},
					{
						"_id": "0x0001",
						"_name": "RemainingTime",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_LEVEL_REMAINING_TIME ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0008",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0002",
						"_name": "MinLevel",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_LEVEL_MIN_LEVEL ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0008",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0003",
						"_name": "MaxLevel",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_LEVEL_MAX_LEVEL ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0008",
						"_default": "255",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0004",
						"_name": "CurrentFrequency",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_LEVEL_CURRENT_FREQUENCY ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0008",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x0005",
						"_name": "MinFrequency",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_LEVEL_MIN_FREQUENCY ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0008",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0006",
						"_name": "MaxFrequency",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_LEVEL_MAX_FREQUENCY ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0008",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0010",
						"_name": "OnOffTransitionTime",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_LEVEL_ON_OFF_TRANSITION_TIME ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0008",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0011",
						"_name": "OnLevel",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_LEVEL_ON_LEVEL ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0008",
						"_default": "255",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0012",
						"_name": "OnTransitionTime",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_LEVEL_ON_TRANSITION_TIME ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0008",
						"_default": "65535",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0013",
						"_name": "OffTransitionTime",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_LEVEL_OFF_TRANSITION_TIME ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0008",
						"_default": "65535",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0014",
						"_name": "DefaultMoveRate",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_LEVEL_DEFAULT_MOVE_RATE ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0008",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x000f",
						"_name": "Options",
						"_type": "LevelOptions",
						"_zigbeetype": "ZCL_DATATYPE_BITMAP8",
						"_definition": "ATTRID_LEVEL_OPTIONS ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0008",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x4000",
						"_name": "StartUpCurrentLevel",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_LEVEL_START_UP_CURRENT_LEVEL ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0008",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					}
				],
				"command": [
					{
						"_id": "0x00",
						"_name": "MoveToLevel",
						"_definition": "COMMAND_LEVEL_MOVE_TO_LEVEL",
						"_location": "zcl_general.h",
						"_callback": "LevelControlMoveToLevel",
						"_clusterID": "0x0008",
						"_required": "true"
					},
					{
						"_id": "0x01",
						"_name": "Move",
						"_definition": "COMMAND_LEVEL_MOVE",
						"_location": "zcl_general.h",
						"_callback": "LevelControlMove",
						"_clusterID": "0x0008",
						"_required": "true"
					},
					{
						"_id": "0x02",
						"_name": "Step",
						"_definition": "COMMAND_LEVEL_STEP",
						"_location": "zcl_general.h",
						"_callback": "LevelControlStep",
						"_clusterID": "0x0008",
						"_required": "true"
					},
					{
						"_id": "0x03",
						"_name": "Stop",
						"_definition": "COMMAND_LEVEL_STOP",
						"_location": "zcl_general.h",
						"_callback": "LevelControlStop",
						"_clusterID": "0x0008",
						"_required": "true"
					},
					{
						"_id": "0x04",
						"_name": "MoveToLevelWithOnOff",
						"_definition": "COMMAND_LEVEL_MOVE_TO_LEVEL_WITH_ON_OFF",
						"_location": "zcl_general.h",
						"_callback": "LevelControlMoveToLevel",
						"_clusterID": "0x0008",
						"_required": "true"
					},
					{
						"_id": "0x05",
						"_name": "MoveWithOnOff",
						"_definition": "COMMAND_LEVEL_MOVE_WITH_ON_OFF",
						"_location": "zcl_general.h",
						"_callback": "LevelControlMove",
						"_clusterID": "0x0008",
						"_required": "true"
					},
					{
						"_id": "0x06",
						"_name": "StepWithOnOff",
						"_definition": "COMMAND_LEVEL_STEP_WITH_ON_OFF",
						"_location": "zcl_general.h",
						"_callback": "LevelControlStep",
						"_clusterID": "0x0008",
						"_required": "true"
					},
					{
						"_id": "0x07",
						"_name": "StopWithOnOff",
						"_definition": "COMMAND_LEVEL_STOP_WITH_ON_OFF",
						"_location": "zcl_general.h",
						"_callback": "LevelControlStop",
						"_clusterID": "0x0008",
						"_required": "true"
					},
					{
						"_id": "0x08",
						"_name": "MoveToClosestFrequency",
						"_definition": "COMMAND_LEVEL_MOVE_TO_CLOSEST_FREQUENCY",
						"_location": "zcl_general.h",
						"_callback": "LevelControlMoveFreq",
						"_clusterID": "0x0008",
						"_required": "false"
					}
				]
			},
			"client": {
				"attribute": [
					{
						"_id": "0xfffd",
						"_name": "ClusterRevision",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_CLUSTER_REVISION",
						"_location": "zcl.h",
						"_clusterID": "0x0008",
						"_default": "1",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0xfffe",
						"_name": "AttributeReportingStatus",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_ATTRIBUTE_REPORTING_STATUS",
						"_location": "zcl.h",
						"_clusterID": "0x0008",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					}
				],
			},
			"appcallback": {
				"callbacks": [
					{
						"_name": "BasicReset",
						"_returnType": "void",
						"_arguments": "( void )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Reset to Factory Defaults"
							},
							{
								"_comment": "/// command. On receipt of this command, the device resets all the attributes"
							},
							{
								"_comment": "/// of all its clusters to their factory defaults."
							}
						]
					},
					{
						"_name": "Identify",
						"_returnType": "void",
						"_arguments": "( zclIdentify_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Identify command."
							}
						]
					},
					{
						"_name": "IdentifyQuery",
						"_returnType": "void",
						"_arguments": "( zclIdentifyQuery_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Identify Query command."
							}
						]
					},
					{
						"_name": "IdentifyQueryRsp",
						"_returnType": "void",
						"_arguments": "( zclIdentifyQueryRsp_t *pRsp )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Identify Query Response command."
							}
						]
					},
					{
						"_name": "IdentifyTriggerEffect",
						"_returnType": "void",
						"_arguments": "( zclIdentifyTriggerEffect_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Identify Trigger Effect command."
							}
						]
					},
					{
						"_name": "#ifdef ZCL_ON_OFF",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "OnOff",
						"_returnType": "void",
						"_arguments": "( uint8_t cmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming On, Off or Toggle command."
							}
						]
					},
					{
						"_name": "OnOff_OffWithEffect",
						"_returnType": "void",
						"_arguments": "( zclOffWithEffect_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Off with Effect"
							}
						]
					},
					{
						"_name": "OnOff_OnWithRecallGlobalScene",
						"_returnType": "void",
						"_arguments": "( void )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming On with Recall Global Scene command."
							}
						]
					},
					{
						"_name": "OnOff_OnWithTimedOff",
						"_returnType": "void",
						"_arguments": "( zclOnWithTimedOff_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming On with Timed Off."
							}
						]
					},
					{
						"_name": "#endif",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "#ifdef ZCL_LEVEL_CTRL",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "LevelControlMoveToLevel",
						"_returnType": "void",
						"_arguments": "( zclLCMoveToLevel_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Level Control - Move to Level command"
							}
						]
					},
					{
						"_name": "LevelControlMove",
						"_returnType": "void",
						"_arguments": "( zclLCMove_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Level Control - Move command"
							}
						]
					},
					{
						"_name": "LevelControlStep",
						"_returnType": "void",
						"_arguments": "( zclLCStep_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Level Control - Step command"
							}
						]
					},
					{
						"_name": "LevelControlStop",
						"_returnType": "void",
						"_arguments": "( zclLCStop_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Level Control - Stop command"
							}
						]
					},
					{
						"_name": "LevelControlMoveFreq",
						"_returnType": "void",
						"_arguments": "( zclLCMoveFreq_t *pCmd)",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Level Control - Move to Closest Frequency command"
							}
						]
					},
					{
						"_name": "#endif",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "#ifdef ZCL_GROUPS",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "GroupRsp",
						"_returnType": "void",
						"_arguments": "( zclGroupRsp_t *pRsp )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an received Group Response message."
							},
							{
								"_comment": "/// This means that this app sent the request message."
							}
						]
					},
					{
						"_name": "#endif",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "#ifdef ZCL_SCENES",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "SceneStoreReq",
						"_returnType": "uint8_t",
						"_arguments": "( zclSceneReq_t *pReq )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Scene Store request."
							},
							{
								"_comment": "/// The app will fill in the 'extField' with what is needed to restore its"
							},
							{
								"_comment": "/// current settings. Before overwriting the content of the scene,"
							},
							{
								"_comment": "/// App needs to verify if there was a change with the previous configuration."
							},
							{
								"_comment": "/// App function returns TRUE if there was a change, otherwise returns FALSE."
							}
						]
					},
					{
						"_name": "SceneRecallReq",
						"_returnType": "void",
						"_arguments": "( zclSceneReq_t *pReq )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Scene Recall request"
							},
							{
								"_comment": "/// The app will use what's in the 'extField' to restore to these settings."
							}
						]
					},
					{
						"_name": "SceneRsp",
						"_returnType": "void",
						"_arguments": "( zclSceneRsp_t *pRsp )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Scene responses. This means"
							},
							{
								"_comment": "/// that this app sent the request for this response."
							}
						]
					},
					{
						"_name": "#endif",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "#ifdef ZCL_ALARMS",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "Alarm",
						"_returnType": "void",
						"_arguments": "( uint8_t direction, zclAlarm_t *pAlarm )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Alarm request or response command."
							}
						]
					},
					{
						"_name": "#endif",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "#ifdef SE_UK_EXT",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "GetEventLog",
						"_returnType": "void",
						"_arguments": "( uint8_t srcEP, afAddrType_t *srcAddr, zclGetEventLog_t *pEventLog, uint8_t seqNum )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Alarm Get Event Log command."
							}
						]
					},
					{
						"_name": "PublishEventLog",
						"_returnType": "void",
						"_arguments": "( afAddrType_t *srcAddr, zclPublishEventLog_t *pEventLog )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Alarm Publish Event Log command."
							}
						]
					},
					{
						"_name": "#endif",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "Location",
						"_returnType": "void",
						"_arguments": "( zclLocation_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to to process an incoming RSSI Location command."
							}
						]
					},
					{
						"_name": "LocationRsp",
						"_returnType": "void",
						"_arguments": "( zclLocationRsp_t *pRsp )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming RSSI Location response command."
							},
							{
								"_comment": "/// This means  that this app sent the request for this response."
							}
						]
					}
				],
				"_name": "zclGeneral_AppCallbacks_t",
				"_register": "zclGeneral_RegisterCmdCallbacks"
			},
			"_id": "0x0008",
			"_name": "Level",
			"_definition": "ZCL_CLUSTER_ID_GENERAL_LEVEL_CONTROL",
			"_primary_transaction": "1",
			"_class": "general",
			"_flag": "ZCL_LEVEL_CTRL",
			"_location": "zcl_general.h"
		},
		{
			"server": {
				"attribute": [
					{
						"_id": "0xfffd",
						"_name": "ClusterRevision",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_CLUSTER_REVISION",
						"_location": "zcl.h",
						"_clusterID": "0x0406",
						"_default": "1",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0xfffe",
						"_name": "AttributeReportingStatus",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_ATTRIBUTE_REPORTING_STATUS",
						"_location": "zcl.h",
						"_clusterID": "0x0406",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0000",
						"_name": "Occupancy",
						"_type": "map8",
						"_zigbeetype": "ZCL_DATATYPE_BITMAP8",
						"_definition": "ATTRID_OCCUPANCY_SENSING_OCCUPANCY ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0406",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "true",
						"_required": "true"
					},
					{
						"_id": "0x0001",
						"_name": "OccupancySensorType",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_OCCUPANCY_SENSING_OCCUPANCY_SENSOR_TYPE ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0406",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0x0002",
						"_name": "OccupancySensorTypeBitmap",
						"_type": "map8",
						"_zigbeetype": "ZCL_DATATYPE_BITMAP8",
						"_definition": "ATTRID_OCCUPANCY_SENSING_OCCUPANCY_SENSOR_TYPE_BITMAP ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0406",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0x0010",
						"_name": "PIROccupiedToUnoccupiedDelay",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_OCCUPANCY_SENSING_PIR_OCCUPIED_TO_UNOCCUPIED_DELAY ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0406",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0011",
						"_name": "PIRUnoccupiedToOccupiedDelay",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_OCCUPANCY_SENSING_PIR_UNOCCUPIED_TO_OCCUPIED_DELAY ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0406",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0012",
						"_name": "PIRUnoccupiedToOccupiedThreshold",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_OCCUPANCY_SENSING_PIR_UNOCCUPIED_TO_OCCUPIED_THRESHOLD ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0406",
						"_default": "1",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0020",
						"_name": "UltrasonicOccupiedToUnoccupiedDelay",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_OCCUPANCY_SENSING_ULTRASONIC_OCCUPIED_TO_UNOCCUPIED_DELAY ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0406",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0021",
						"_name": "UltrasonicUnoccupiedToOccupiedDelay",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_OCCUPANCY_SENSING_ULTRASONIC_UNOCCUPIED_TO_OCCUPIED_DELAY ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0406",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0022",
						"_name": "UltrasonicUnoccupiedToOccupiedThreshold",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_OCCUPANCY_SENSING_ULTRASONIC_UNOCCUPIED_TO_OCCUPIED_THRESHOLD ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0406",
						"_default": "1",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0030",
						"_name": "PhysicalContactOccupiedToUnoccupiedDelay",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_OCCUPANCY_SENSING_PHYSICAL_CONTACT_OCCUPIED_TO_UNOCCUPIED_DELAY ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0406",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0031",
						"_name": "PhysicalContactUnoccupiedToOccupiedDelay",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_OCCUPANCY_SENSING_PHYSICAL_CONTACT_UNOCCUPIED_TO_OCCUPIED_DELAY ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0406",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0032",
						"_name": "PhysicalContactUnoccupiedToOccupiedThreshold",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_OCCUPANCY_SENSING_PHYSICAL_CONTACT_UNOCCUPIED_TO_OCCUPIED_THRESHOLD ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0406",
						"_default": "1",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					}
				]
			},
			"client": {
				"attribute": [
					{
						"_id": "0xfffd",
						"_name": "ClusterRevision",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_CLUSTER_REVISION",
						"_location": "zcl.h",
						"_clusterID": "0x0406",
						"_default": "1",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0xfffe",
						"_name": "AttributeReportingStatus",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_ATTRIBUTE_REPORTING_STATUS",
						"_location": "zcl.h",
						"_clusterID": "0x0406",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					}
				],
			},
			"appcallback": {
				"callbacks": [
					{
						"_name": "MSPlaceHolder",
						"_returnType": "void",
						"_arguments": "( void )"
					}
				],
				"_name": "zclMS_AppCallbacks_t",
				"_register": "zclMS_RegisterCmdCallbacks"
			},
			"_id": "0x0406",
			"_name": "OccupancySensing",
			"_definition": "ZCL_CLUSTER_ID_MS_OCCUPANCY_SENSING",
			"_primary_transaction": "2",
			"_class": "ms",
			"_flag": "ZCL_MS",
			"_location": "zcl_ms.h"
		},
		{
			"server": {
				"attribute": [
					{
						"_id": "0xfffd",
						"_name": "ClusterRevision",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_CLUSTER_REVISION",
						"_location": "zcl.h",
						"_clusterID": "0x0006",
						"_default": "1",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0xfffe",
						"_name": "AttributeReportingStatus",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_ATTRIBUTE_REPORTING_STATUS",
						"_location": "zcl.h",
						"_clusterID": "0x0006",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0000",
						"_name": "OnOff",
						"_type": "bool",
						"_zigbeetype": "ZCL_DATATYPE_BOOLEAN",
						"_definition": "ATTRID_ON_OFF_ON_OFF ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0006",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "true",
						"_required": "true"
					},
					{
						"_id": "0x4000",
						"_name": "GlobalSceneControl",
						"_type": "bool",
						"_zigbeetype": "ZCL_DATATYPE_BOOLEAN",
						"_definition": "ATTRID_ON_OFF_GLOBAL_SCENE_CONTROL ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0006",
						"_default": "1",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x4001",
						"_name": "OnTime",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_ON_OFF_ON_TIME ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0006",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x4002",
						"_name": "OffWaitTime",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_ON_OFF_OFF_WAIT_TIME ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0006",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x4003",
						"_name": "StartUpOnOff",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_ON_OFF_START_UP_ON_OFF ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0006",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					}
				],
				"command": [
					{
						"_id": "0x00",
						"_name": "Off",
						"_definition": "COMMAND_ON_OFF_OFF",
						"_location": "zcl_general.h",
						"_callback": "OnOff",
						"_clusterID": "0x0006",
						"_required": "true"
					},
					{
						"_id": "0x01",
						"_name": "On",
						"_definition": "COMMAND_ON_OFF_ON",
						"_location": "zcl_general.h",
						"_callback": "OnOff",
						"_clusterID": "0x0006",
						"_required": "true"
					},
					{
						"_id": "0x02",
						"_name": "Toggle",
						"_definition": "COMMAND_ON_OFF_TOGGLE",
						"_location": "zcl_general.h",
						"_callback": "OnOff",
						"_clusterID": "0x0006",
						"_required": "true"
					},
					{
						"_id": "0x40",
						"_name": "OffWithEffect",
						"_definition": "COMMAND_ON_OFF_OFF_WITH_EFFECT",
						"_location": "zcl_general.h",
						"_callback": "OnOff_OffWithEffect",
						"_clusterID": "0x0006",
						"_required": "false"
					},
					{
						"_id": "0x41",
						"_name": "OnWithRecallGlobalScene",
						"_definition": "COMMAND_ON_OFF_ON_WITH_RECALL_GLOBAL_SCENE",
						"_location": "zcl_general.h",
						"_callback": "OnOff_OnWithRecallGlobalScene",
						"_clusterID": "0x0006",
						"_required": "false"
					},
					{
						"_id": "0x42",
						"_name": "OnWithTimedOff",
						"_definition": "COMMAND_ON_OFF_ON_WITH_TIMED_OFF",
						"_location": "zcl_general.h",
						"_callback": "OnOff_OnWithTimedOff",
						"_clusterID": "0x0006",
						"_required": "false"
					}
				]
			},
			"client": {
				"attribute": [
					{
						"_id": "0xfffd",
						"_name": "ClusterRevision",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_CLUSTER_REVISION",
						"_location": "zcl.h",
						"_clusterID": "0x0006",
						"_default": "1",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0xfffe",
						"_name": "AttributeReportingStatus",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_ATTRIBUTE_REPORTING_STATUS",
						"_location": "zcl.h",
						"_clusterID": "0x0006",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					}
				],
			},
			"appcallback": {
				"callbacks": [
					{
						"_name": "BasicReset",
						"_returnType": "void",
						"_arguments": "( void )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Reset to Factory Defaults"
							},
							{
								"_comment": "/// command. On receipt of this command, the device resets all the attributes"
							},
							{
								"_comment": "/// of all its clusters to their factory defaults."
							}
						]
					},
					{
						"_name": "Identify",
						"_returnType": "void",
						"_arguments": "( zclIdentify_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Identify command."
							}
						]
					},
					{
						"_name": "IdentifyQuery",
						"_returnType": "void",
						"_arguments": "( zclIdentifyQuery_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Identify Query command."
							}
						]
					},
					{
						"_name": "IdentifyQueryRsp",
						"_returnType": "void",
						"_arguments": "( zclIdentifyQueryRsp_t *pRsp )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Identify Query Response command."
							}
						]
					},
					{
						"_name": "IdentifyTriggerEffect",
						"_returnType": "void",
						"_arguments": "( zclIdentifyTriggerEffect_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Identify Trigger Effect command."
							}
						]
					},
					{
						"_name": "#ifdef ZCL_ON_OFF",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "OnOff",
						"_returnType": "void",
						"_arguments": "( uint8_t cmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming On, Off or Toggle command."
							}
						]
					},
					{
						"_name": "OnOff_OffWithEffect",
						"_returnType": "void",
						"_arguments": "( zclOffWithEffect_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Off with Effect"
							}
						]
					},
					{
						"_name": "OnOff_OnWithRecallGlobalScene",
						"_returnType": "void",
						"_arguments": "( void )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming On with Recall Global Scene command."
							}
						]
					},
					{
						"_name": "OnOff_OnWithTimedOff",
						"_returnType": "void",
						"_arguments": "( zclOnWithTimedOff_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming On with Timed Off."
							}
						]
					},
					{
						"_name": "#endif",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "#ifdef ZCL_LEVEL_CTRL",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "LevelControlMoveToLevel",
						"_returnType": "void",
						"_arguments": "( zclLCMoveToLevel_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Level Control - Move to Level command"
							}
						]
					},
					{
						"_name": "LevelControlMove",
						"_returnType": "void",
						"_arguments": "( zclLCMove_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Level Control - Move command"
							}
						]
					},
					{
						"_name": "LevelControlStep",
						"_returnType": "void",
						"_arguments": "( zclLCStep_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Level Control - Step command"
							}
						]
					},
					{
						"_name": "LevelControlStop",
						"_returnType": "void",
						"_arguments": "( zclLCStop_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Level Control - Stop command"
							}
						]
					},
					{
						"_name": "LevelControlMoveFreq",
						"_returnType": "void",
						"_arguments": "( zclLCMoveFreq_t *pCmd)",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Level Control - Move to Closest Frequency command"
							}
						]
					},
					{
						"_name": "#endif",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "#ifdef ZCL_GROUPS",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "GroupRsp",
						"_returnType": "void",
						"_arguments": "( zclGroupRsp_t *pRsp )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an received Group Response message."
							},
							{
								"_comment": "/// This means that this app sent the request message."
							}
						]
					},
					{
						"_name": "#endif",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "#ifdef ZCL_SCENES",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "SceneStoreReq",
						"_returnType": "uint8_t",
						"_arguments": "( zclSceneReq_t *pReq )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Scene Store request."
							},
							{
								"_comment": "/// The app will fill in the 'extField' with what is needed to restore its"
							},
							{
								"_comment": "/// current settings. Before overwriting the content of the scene,"
							},
							{
								"_comment": "/// App needs to verify if there was a change with the previous configuration."
							},
							{
								"_comment": "/// App function returns TRUE if there was a change, otherwise returns FALSE."
							}
						]
					},
					{
						"_name": "SceneRecallReq",
						"_returnType": "void",
						"_arguments": "( zclSceneReq_t *pReq )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Scene Recall request"
							},
							{
								"_comment": "/// The app will use what's in the 'extField' to restore to these settings."
							}
						]
					},
					{
						"_name": "SceneRsp",
						"_returnType": "void",
						"_arguments": "( zclSceneRsp_t *pRsp )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Scene responses. This means"
							},
							{
								"_comment": "/// that this app sent the request for this response."
							}
						]
					},
					{
						"_name": "#endif",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "#ifdef ZCL_ALARMS",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "Alarm",
						"_returnType": "void",
						"_arguments": "( uint8_t direction, zclAlarm_t *pAlarm )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Alarm request or response command."
							}
						]
					},
					{
						"_name": "#endif",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "#ifdef SE_UK_EXT",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "GetEventLog",
						"_returnType": "void",
						"_arguments": "( uint8_t srcEP, afAddrType_t *srcAddr, zclGetEventLog_t *pEventLog, uint8_t seqNum )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Alarm Get Event Log command."
							}
						]
					},
					{
						"_name": "PublishEventLog",
						"_returnType": "void",
						"_arguments": "( afAddrType_t *srcAddr, zclPublishEventLog_t *pEventLog )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Alarm Publish Event Log command."
							}
						]
					},
					{
						"_name": "#endif",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "Location",
						"_returnType": "void",
						"_arguments": "( zclLocation_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to to process an incoming RSSI Location command."
							}
						]
					},
					{
						"_name": "LocationRsp",
						"_returnType": "void",
						"_arguments": "( zclLocationRsp_t *pRsp )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming RSSI Location response command."
							},
							{
								"_comment": "/// This means  that this app sent the request for this response."
							}
						]
					}
				],
				"_name": "zclGeneral_AppCallbacks_t",
				"_register": "zclGeneral_RegisterCmdCallbacks"
			},
			"_id": "0x0006",
			"_name": "OnOff",
			"_definition": "ZCL_CLUSTER_ID_GENERAL_ON_OFF",
			"_primary_transaction": "1",
			"_class": "general",
			"_flag": "ZCL_ON_OFF",
			"_location": "zcl_general.h"
		},
		{
			"server": {
				"attribute": [
					{
						"_id": "0xfffd",
						"_name": "ClusterRevision",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_CLUSTER_REVISION",
						"_location": "zcl.h",
						"_clusterID": "0x0019",
						"_default": "1",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0xfffe",
						"_name": "AttributeReportingStatus",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_ATTRIBUTE_REPORTING_STATUS",
						"_location": "zcl.h",
						"_clusterID": "0x0019",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					}
				],
				"command": [
					{
						"_id": "0x01",
						"_name": "QueryNextImageRequest",
						"_definition": "COMMAND_OTA_UPGRADE_QUERY_NEXT_IMAGE_REQUEST",
						"_location": "zcl_ota.h",
						"_clusterID": "0x0019",
						"_required": "true"
					},
					{
						"_id": "0x03",
						"_name": "ImageBlockRequest",
						"_definition": "COMMAND_OTA_UPGRADE_IMAGE_BLOCK_REQUEST",
						"_location": "zcl_ota.h",
						"_clusterID": "0x0019",
						"_required": "true"
					},
					{
						"_id": "0x04",
						"_name": "ImagePageRequest",
						"_definition": "COMMAND_OTA_UPGRADE_IMAGE_PAGE_REQUEST",
						"_location": "zcl_ota.h",
						"_clusterID": "0x0019",
						"_required": "false"
					},
					{
						"_id": "0x06",
						"_name": "UpgradeEndRequest",
						"_definition": "COMMAND_OTA_UPGRADE_UPGRADE_END_REQUEST",
						"_location": "zcl_ota.h",
						"_clusterID": "0x0019",
						"_required": "true"
					},
					{
						"_id": "0x08",
						"_name": "QueryDeviceSpecificFileRequest",
						"_definition": "COMMAND_OTA_UPGRADE_QUERY_DEVICE_SPECIFIC_FILE_REQUEST",
						"_location": "zcl_ota.h",
						"_clusterID": "0x0019",
						"_required": "false"
					}
				]
			},
			"client": {
				"attribute": [
					{
						"_id": "0xfffd",
						"_name": "ClusterRevision",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_CLUSTER_REVISION",
						"_location": "zcl.h",
						"_clusterID": "0x0019",
						"_default": "1",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0xfffe",
						"_name": "AttributeReportingStatus",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_ATTRIBUTE_REPORTING_STATUS",
						"_location": "zcl.h",
						"_clusterID": "0x0019",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0000",
						"_name": "UpgradeServerID",
						"_type": "EUI64",
						"_zigbeetype": "ZCL_DATATYPE_BITMAP64",
						"_definition": "ATTRID_OTA_UPGRADE_UPGRADE_SERVER_ID ",
						"_location": "zcl_ota.h",
						"_clusterID": "0x0019",
						"_default": "18446744073709551615",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0x0001",
						"_name": "FileOffset",
						"_type": "uint32",
						"_zigbeetype": "ZCL_DATATYPE_UINT32",
						"_definition": "ATTRID_OTA_UPGRADE_FILE_OFFSET ",
						"_location": "zcl_ota.h",
						"_clusterID": "0x0019",
						"_default": "4294967294",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0002",
						"_name": "CurrentFileVersion",
						"_type": "uint32",
						"_zigbeetype": "ZCL_DATATYPE_UINT32",
						"_definition": "ATTRID_OTA_UPGRADE_CURRENT_FILE_VERSION ",
						"_location": "zcl_ota.h",
						"_clusterID": "0x0019",
						"_default": "4294967294",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0003",
						"_name": "CurrentZigBeeStackVersion",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_OTA_UPGRADE_CURRENT_ZIG_BEE_STACK_VERSION ",
						"_location": "zcl_ota.h",
						"_clusterID": "0x0019",
						"_default": "65535",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0004",
						"_name": "DownloadedFileVersion",
						"_type": "uint32",
						"_zigbeetype": "ZCL_DATATYPE_UINT32",
						"_definition": "ATTRID_OTA_UPGRADE_DOWNLOADED_FILE_VERSION ",
						"_location": "zcl_ota.h",
						"_clusterID": "0x0019",
						"_default": "4294967294",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0005",
						"_name": "DownloadedZigBeeStackVersion",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_OTA_UPGRADE_DOWNLOADED_ZIG_BEE_STACK_VERSION ",
						"_location": "zcl_ota.h",
						"_clusterID": "0x0019",
						"_default": "65535",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0006",
						"_name": "ImageUpgradeStatus",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_OTA_UPGRADE_IMAGE_UPGRADE_STATUS ",
						"_location": "zcl_ota.h",
						"_clusterID": "0x0019",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0x0007",
						"_name": "ManufacturerID",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_OTA_UPGRADE_MANUFACTURER_ID ",
						"_location": "zcl_ota.h",
						"_clusterID": "0x0019",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0008",
						"_name": "ImageTypeID",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_OTA_UPGRADE_IMAGE_TYPE_ID ",
						"_location": "zcl_ota.h",
						"_clusterID": "0x0019",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0009",
						"_name": "MinimumBlockPeriod",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_OTA_UPGRADE_MINIMUM_BLOCK_PERIOD ",
						"_location": "zcl_ota.h",
						"_clusterID": "0x0019",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x000a",
						"_name": "ImageStamp",
						"_type": "uint32",
						"_zigbeetype": "ZCL_DATATYPE_UINT32",
						"_definition": "ATTRID_OTA_UPGRADE_IMAGE_STAMP ",
						"_location": "zcl_ota.h",
						"_clusterID": "0x0019",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x000b",
						"_name": "UpgradeActivationPolicy",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_OTA_UPGRADE_UPGRADE_ACTIVATION_POLICY ",
						"_location": "zcl_ota.h",
						"_clusterID": "0x0019",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x000c",
						"_name": "UpgradeTimeoutPolicy",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_OTA_UPGRADE_UPGRADE_TIMEOUT_POLICY ",
						"_location": "zcl_ota.h",
						"_clusterID": "0x0019",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					}
				],
				"command": [
					{
						"_id": "0x00",
						"_name": "ImageNotify",
						"_definition": "COMMAND_OTA_UPGRADE_IMAGE_NOTIFY",
						"_location": "zcl_ota.h",
						"_clusterID": "0x0019",
						"_required": "false"
					},
					{
						"_id": "0x02",
						"_name": "QueryNextImageResponse",
						"_definition": "COMMAND_OTA_UPGRADE_QUERY_NEXT_IMAGE_RESPONSE",
						"_location": "zcl_ota.h",
						"_clusterID": "0x0019",
						"_required": "true"
					},
					{
						"_id": "0x05",
						"_name": "ImageBlockResponse",
						"_definition": "COMMAND_OTA_UPGRADE_IMAGE_BLOCK_RESPONSE",
						"_location": "zcl_ota.h",
						"_clusterID": "0x0019",
						"_required": "true"
					},
					{
						"_id": "0x07",
						"_name": "UpgradeEndResponse",
						"_definition": "COMMAND_OTA_UPGRADE_UPGRADE_END_RESPONSE",
						"_location": "zcl_ota.h",
						"_clusterID": "0x0019",
						"_required": "true"
					},
					{
						"_id": "0x09",
						"_name": "QueryDeviceSpecificFileResponse",
						"_definition": "COMMAND_OTA_UPGRADE_QUERY_DEVICE_SPECIFIC_FILE_RESPONSE",
						"_location": "zcl_ota.h",
						"_clusterID": "0x0019",
						"_required": "false"
					}
				]
			},
			"appcallback": {
				"_name": "None",
				"_register": "None"
			},
			"_id": "0x0019",
			"_name": "OTAUpgrade",
			"_definition": "ZCL_CLUSTER_ID_OTA",
			"_primary_transaction": "0",
			"_class": "ota",
			"_flag": "ZCL_OTA",
			"_location": "zcl_ota.h"
		},
		{
			"server": {
				"attribute": [
					{
						"_id": "0xfffd",
						"_name": "ClusterRevision",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_CLUSTER_REVISION",
						"_location": "zcl.h",
						"_clusterID": "0x0020",
						"_default": "1",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0xfffe",
						"_name": "AttributeReportingStatus",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_ATTRIBUTE_REPORTING_STATUS",
						"_location": "zcl.h",
						"_clusterID": "0x0020",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0000",
						"_name": "CheckInInterval",
						"_type": "uint32",
						"_zigbeetype": "ZCL_DATATYPE_UINT32",
						"_definition": "ATTRID_POLL_CONTROL_CHECK_IN_INTERVAL ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0020",
						"_default": "14400",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0x0001",
						"_name": "LongPollInterval",
						"_type": "uint32",
						"_zigbeetype": "ZCL_DATATYPE_UINT32",
						"_definition": "ATTRID_POLL_CONTROL_LONG_POLL_INTERVAL ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0020",
						"_default": "20",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0x0002",
						"_name": "ShortPollInterval",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_POLL_CONTROL_SHORT_POLL_INTERVAL ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0020",
						"_default": "2",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0x0003",
						"_name": "FastPollTimeout",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_POLL_CONTROL_FAST_POLL_TIMEOUT ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0020",
						"_default": "40",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0x0004",
						"_name": "CheckInIntervalMin",
						"_type": "uint32",
						"_zigbeetype": "ZCL_DATATYPE_UINT32",
						"_definition": "ATTRID_POLL_CONTROL_CHECK_IN_INTERVAL_MIN ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0020",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0005",
						"_name": "LongPollIntervalMin",
						"_type": "uint32",
						"_zigbeetype": "ZCL_DATATYPE_UINT32",
						"_definition": "ATTRID_POLL_CONTROL_LONG_POLL_INTERVAL_MIN ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0020",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0006",
						"_name": "FastPollTimeoutMax",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_POLL_CONTROL_FAST_POLL_TIMEOUT_MAX ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0020",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					}
				],
				"command": [
					{
						"_id": "0x00",
						"_name": "CheckInResponse",
						"_definition": "COMMAND_POLL_CONTROL_CHECK_IN_RESPONSE",
						"_location": "zcl_general.h",
						"_clusterID": "0x0020",
						"_required": "true"
					},
					{
						"_id": "0x01",
						"_name": "FastPollStop",
						"_definition": "COMMAND_POLL_CONTROL_FAST_POLL_STOP",
						"_location": "zcl_general.h",
						"_clusterID": "0x0020",
						"_required": "true"
					},
					{
						"_id": "0x02",
						"_name": "SetLongPollInterval",
						"_definition": "COMMAND_POLL_CONTROL_SET_LONG_POLL_INTERVAL",
						"_location": "zcl_general.h",
						"_clusterID": "0x0020",
						"_required": "false"
					},
					{
						"_id": "0x03",
						"_name": "SetShortPollInterval",
						"_definition": "COMMAND_POLL_CONTROL_SET_SHORT_POLL_INTERVAL",
						"_location": "zcl_general.h",
						"_clusterID": "0x0020",
						"_required": "false"
					}
				]
			},
			"client": {
				"attribute": [
					{
						"_id": "0xfffd",
						"_name": "ClusterRevision",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_CLUSTER_REVISION",
						"_location": "zcl.h",
						"_clusterID": "0x0020",
						"_default": "1",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0xfffe",
						"_name": "AttributeReportingStatus",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_ATTRIBUTE_REPORTING_STATUS",
						"_location": "zcl.h",
						"_clusterID": "0x0020",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					}
				],
				"command": [
					{
						"_id": "0x00",
						"_name": "CheckIn",
						"_definition": "COMMAND_POLL_CONTROL_CHECK_IN",
						"_location": "zcl_general.h",
						"_clusterID": "0x0020",
						"_required": "true"
					}
				]
			},
			"appcallback": {
				"callbacks": [
					{
						"_name": "BasicReset",
						"_returnType": "void",
						"_arguments": "( void )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Reset to Factory Defaults"
							},
							{
								"_comment": "/// command. On receipt of this command, the device resets all the attributes"
							},
							{
								"_comment": "/// of all its clusters to their factory defaults."
							}
						]
					},
					{
						"_name": "Identify",
						"_returnType": "void",
						"_arguments": "( zclIdentify_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Identify command."
							}
						]
					},
					{
						"_name": "IdentifyQuery",
						"_returnType": "void",
						"_arguments": "( zclIdentifyQuery_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Identify Query command."
							}
						]
					},
					{
						"_name": "IdentifyQueryRsp",
						"_returnType": "void",
						"_arguments": "( zclIdentifyQueryRsp_t *pRsp )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Identify Query Response command."
							}
						]
					},
					{
						"_name": "IdentifyTriggerEffect",
						"_returnType": "void",
						"_arguments": "( zclIdentifyTriggerEffect_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Identify Trigger Effect command."
							}
						]
					},
					{
						"_name": "#ifdef ZCL_ON_OFF",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "OnOff",
						"_returnType": "void",
						"_arguments": "( uint8_t cmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming On, Off or Toggle command."
							}
						]
					},
					{
						"_name": "OnOff_OffWithEffect",
						"_returnType": "void",
						"_arguments": "( zclOffWithEffect_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Off with Effect"
							}
						]
					},
					{
						"_name": "OnOff_OnWithRecallGlobalScene",
						"_returnType": "void",
						"_arguments": "( void )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming On with Recall Global Scene command."
							}
						]
					},
					{
						"_name": "OnOff_OnWithTimedOff",
						"_returnType": "void",
						"_arguments": "( zclOnWithTimedOff_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming On with Timed Off."
							}
						]
					},
					{
						"_name": "#endif",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "#ifdef ZCL_LEVEL_CTRL",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "LevelControlMoveToLevel",
						"_returnType": "void",
						"_arguments": "( zclLCMoveToLevel_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Level Control - Move to Level command"
							}
						]
					},
					{
						"_name": "LevelControlMove",
						"_returnType": "void",
						"_arguments": "( zclLCMove_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Level Control - Move command"
							}
						]
					},
					{
						"_name": "LevelControlStep",
						"_returnType": "void",
						"_arguments": "( zclLCStep_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Level Control - Step command"
							}
						]
					},
					{
						"_name": "LevelControlStop",
						"_returnType": "void",
						"_arguments": "( zclLCStop_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Level Control - Stop command"
							}
						]
					},
					{
						"_name": "LevelControlMoveFreq",
						"_returnType": "void",
						"_arguments": "( zclLCMoveFreq_t *pCmd)",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Level Control - Move to Closest Frequency command"
							}
						]
					},
					{
						"_name": "#endif",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "#ifdef ZCL_GROUPS",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "GroupRsp",
						"_returnType": "void",
						"_arguments": "( zclGroupRsp_t *pRsp )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an received Group Response message."
							},
							{
								"_comment": "/// This means that this app sent the request message."
							}
						]
					},
					{
						"_name": "#endif",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "#ifdef ZCL_SCENES",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "SceneStoreReq",
						"_returnType": "uint8_t",
						"_arguments": "( zclSceneReq_t *pReq )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Scene Store request."
							},
							{
								"_comment": "/// The app will fill in the 'extField' with what is needed to restore its"
							},
							{
								"_comment": "/// current settings. Before overwriting the content of the scene,"
							},
							{
								"_comment": "/// App needs to verify if there was a change with the previous configuration."
							},
							{
								"_comment": "/// App function returns TRUE if there was a change, otherwise returns FALSE."
							}
						]
					},
					{
						"_name": "SceneRecallReq",
						"_returnType": "void",
						"_arguments": "( zclSceneReq_t *pReq )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Scene Recall request"
							},
							{
								"_comment": "/// The app will use what's in the 'extField' to restore to these settings."
							}
						]
					},
					{
						"_name": "SceneRsp",
						"_returnType": "void",
						"_arguments": "( zclSceneRsp_t *pRsp )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Scene responses. This means"
							},
							{
								"_comment": "/// that this app sent the request for this response."
							}
						]
					},
					{
						"_name": "#endif",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "#ifdef ZCL_ALARMS",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "Alarm",
						"_returnType": "void",
						"_arguments": "( uint8_t direction, zclAlarm_t *pAlarm )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Alarm request or response command."
							}
						]
					},
					{
						"_name": "#endif",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "#ifdef SE_UK_EXT",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "GetEventLog",
						"_returnType": "void",
						"_arguments": "( uint8_t srcEP, afAddrType_t *srcAddr, zclGetEventLog_t *pEventLog, uint8_t seqNum )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Alarm Get Event Log command."
							}
						]
					},
					{
						"_name": "PublishEventLog",
						"_returnType": "void",
						"_arguments": "( afAddrType_t *srcAddr, zclPublishEventLog_t *pEventLog )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Alarm Publish Event Log command."
							}
						]
					},
					{
						"_name": "#endif",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "Location",
						"_returnType": "void",
						"_arguments": "( zclLocation_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to to process an incoming RSSI Location command."
							}
						]
					},
					{
						"_name": "LocationRsp",
						"_returnType": "void",
						"_arguments": "( zclLocationRsp_t *pRsp )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming RSSI Location response command."
							},
							{
								"_comment": "/// This means  that this app sent the request for this response."
							}
						]
					}
				],
				"_name": "zclGeneral_AppCallbacks_t",
				"_register": "zclGeneral_RegisterCmdCallbacks"
			},
			"_id": "0x0020",
			"_name": "PollControl",
			"_definition": "ZCL_CLUSTER_ID_GENERAL_POLL_CONTROL",
			"_primary_transaction": "0",
			"_class": "general",
			"_flag": "ZCL_POLL_CONTROL",
			"_location": "zcl_general.h"
		},
		{
			"server": {
				"attribute": [
					{
						"_id": "0xfffd",
						"_name": "ClusterRevision",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_CLUSTER_REVISION",
						"_location": "zcl.h",
						"_clusterID": "0x0001",
						"_default": "1",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0xfffe",
						"_name": "AttributeReportingStatus",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_ATTRIBUTE_REPORTING_STATUS",
						"_location": "zcl.h",
						"_clusterID": "0x0001",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0000",
						"_name": "MainsVoltage",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_POWER_CONFIGURATION_MAINS_VOLTAGE ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0001",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0001",
						"_name": "MainsFrequency",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_POWER_CONFIGURATION_MAINS_FREQUENCY ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0001",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0010",
						"_name": "MainsAlarmMask",
						"_type": "map8",
						"_zigbeetype": "ZCL_DATATYPE_BITMAP8",
						"_definition": "ATTRID_POWER_CONFIGURATION_MAINS_ALARM_MASK ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0001",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0011",
						"_name": "MainsVoltageMinThreshold",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_POWER_CONFIGURATION_MAINS_VOLTAGE_MIN_THRESHOLD ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0001",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0012",
						"_name": "MainsVoltageMaxThreshold",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_POWER_CONFIGURATION_MAINS_VOLTAGE_MAX_THRESHOLD ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0001",
						"_default": "65535",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0013",
						"_name": "MainsVoltageDwellTripPoint",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_POWER_CONFIGURATION_MAINS_VOLTAGE_DWELL_TRIP_POINT ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0001",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0020",
						"_name": "BatteryVoltage",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_POWER_CONFIGURATION_BATTERY_VOLTAGE ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0001",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0021",
						"_name": "BatteryPercentageRemaining",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_POWER_CONFIGURATION_BATTERY_PERCENTAGE_REMAINING ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0001",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x0030",
						"_name": "BatteryManufacturer",
						"_type": "uint8[]",
						"_zigbeetype": "ZCL_DATATYPE_CHAR_STR",
						"_definition": "ATTRID_POWER_CONFIGURATION_BATTERY_MANUFACTURER ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0001",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0031",
						"_name": "BatterySize",
						"_type": "BatterySize",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_POWER_CONFIGURATION_BATTERY_SIZE ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0001",
						"_default": "255",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0032",
						"_name": "BatteryAHrRating",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_POWER_CONFIGURATION_BATTERY_A_HR_RATING ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0001",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0033",
						"_name": "BatteryQuantity",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_POWER_CONFIGURATION_BATTERY_QUANTITY ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0001",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0034",
						"_name": "BatteryRatedVoltage",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_POWER_CONFIGURATION_BATTERY_RATED_VOLTAGE ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0001",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0035",
						"_name": "BatteryAlarmMask",
						"_type": "BatteryAlarmMask",
						"_zigbeetype": "ZCL_DATATYPE_BITMAP8",
						"_definition": "ATTRID_POWER_CONFIGURATION_BATTERY_ALARM_MASK ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0001",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0036",
						"_name": "BatteryVoltageMinThreshold",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_POWER_CONFIGURATION_BATTERY_VOLTAGE_MIN_THRESHOLD ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0001",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0037",
						"_name": "BatteryVoltageThreshold1",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_POWER_CONFIGURATION_BATTERY_VOLTAGE_THRESHOLD_1 ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0001",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0038",
						"_name": "BatteryVoltageThreshold2",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_POWER_CONFIGURATION_BATTERY_VOLTAGE_THRESHOLD_2 ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0001",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0039",
						"_name": "BatteryVoltageThreshold3",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_POWER_CONFIGURATION_BATTERY_VOLTAGE_THRESHOLD_3 ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0001",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x003a",
						"_name": "BatteryPercentageMinThreshold",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_POWER_CONFIGURATION_BATTERY_PERCENTAGE_MIN_THRESHOLD ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0001",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x003b",
						"_name": "BatteryPercentageThreshold1",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_POWER_CONFIGURATION_BATTERY_PERCENTAGE_THRESHOLD_1 ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0001",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x003c",
						"_name": "BatteryPercentageThreshold2",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_POWER_CONFIGURATION_BATTERY_PERCENTAGE_THRESHOLD_2 ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0001",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x003d",
						"_name": "BatteryPercentageThreshold3",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_POWER_CONFIGURATION_BATTERY_PERCENTAGE_THRESHOLD_3 ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0001",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x003e",
						"_name": "BatteryAlarmState",
						"_type": "BatteryAlarmState",
						"_zigbeetype": "ZCL_DATATYPE_BITMAP32",
						"_definition": "ATTRID_POWER_CONFIGURATION_BATTERY_ALARM_STATE ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0001",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0040",
						"_name": "Battery2Voltage",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_POWER_CONFIGURATION_BATTERY_2_VOLTAGE ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0001",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0041",
						"_name": "Battery2PercentageRemaining",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_POWER_CONFIGURATION_BATTERY_2_PERCENTAGE_REMAINING ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0001",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x0050",
						"_name": "Battery2Manufacturer",
						"_type": "uint8[]",
						"_zigbeetype": "ZCL_DATATYPE_CHAR_STR",
						"_definition": "ATTRID_POWER_CONFIGURATION_BATTERY_2_MANUFACTURER ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0001",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0051",
						"_name": "Battery2Size",
						"_type": "BatterySize",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_POWER_CONFIGURATION_BATTERY_2_SIZE ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0001",
						"_default": "255",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0052",
						"_name": "Battery2AHrRating",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_POWER_CONFIGURATION_BATTERY_2_A_HR_RATING ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0001",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0053",
						"_name": "Battery2Quantity",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_POWER_CONFIGURATION_BATTERY_2_QUANTITY ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0001",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0054",
						"_name": "Battery2RatedVoltage",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_POWER_CONFIGURATION_BATTERY_2_RATED_VOLTAGE ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0001",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0055",
						"_name": "Battery2AlarmMask",
						"_type": "BatteryAlarmMask",
						"_zigbeetype": "ZCL_DATATYPE_BITMAP8",
						"_definition": "ATTRID_POWER_CONFIGURATION_BATTERY_2_ALARM_MASK ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0001",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0056",
						"_name": "Battery2VoltageMinThreshold",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_POWER_CONFIGURATION_BATTERY_2_VOLTAGE_MIN_THRESHOLD ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0001",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0057",
						"_name": "Battery2VoltageThreshold1",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_POWER_CONFIGURATION_BATTERY_2_VOLTAGE_THRESHOLD_1 ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0001",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0058",
						"_name": "Battery2VoltageThreshold2",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_POWER_CONFIGURATION_BATTERY_2_VOLTAGE_THRESHOLD_2 ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0001",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0059",
						"_name": "Battery2VoltageThreshold3",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_POWER_CONFIGURATION_BATTERY_2_VOLTAGE_THRESHOLD_3 ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0001",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x005a",
						"_name": "Battery2PercentageMinThreshold",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_POWER_CONFIGURATION_BATTERY_2_PERCENTAGE_MIN_THRESHOLD ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0001",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x005b",
						"_name": "Battery2PercentageThreshold1",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_POWER_CONFIGURATION_BATTERY_2_PERCENTAGE_THRESHOLD_1 ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0001",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x005c",
						"_name": "Battery2PercentageThreshold2",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_POWER_CONFIGURATION_BATTERY_2_PERCENTAGE_THRESHOLD_2 ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0001",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x005d",
						"_name": "Battery2PercentageThreshold3",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_POWER_CONFIGURATION_BATTERY_2_PERCENTAGE_THRESHOLD_3 ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0001",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x005e",
						"_name": "Battery2AlarmState",
						"_type": "BatteryAlarmState",
						"_zigbeetype": "ZCL_DATATYPE_BITMAP32",
						"_definition": "ATTRID_POWER_CONFIGURATION_BATTERY_2_ALARM_STATE ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0001",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0060",
						"_name": "Battery3Voltage",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_POWER_CONFIGURATION_BATTERY_3_VOLTAGE ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0001",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0061",
						"_name": "Battery3PercentageRemaining",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_POWER_CONFIGURATION_BATTERY_3_PERCENTAGE_REMAINING ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0001",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x0070",
						"_name": "Battery3Manufacturer",
						"_type": "uint8[]",
						"_zigbeetype": "ZCL_DATATYPE_CHAR_STR",
						"_definition": "ATTRID_POWER_CONFIGURATION_BATTERY_3_MANUFACTURER ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0001",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0071",
						"_name": "Battery3Size",
						"_type": "BatterySize",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_POWER_CONFIGURATION_BATTERY_3_SIZE ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0001",
						"_default": "255",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0072",
						"_name": "Battery3AHrRating",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_POWER_CONFIGURATION_BATTERY_3_A_HR_RATING ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0001",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0073",
						"_name": "Battery3Quantity",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_POWER_CONFIGURATION_BATTERY_3_QUANTITY ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0001",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0074",
						"_name": "Battery3RatedVoltage",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_POWER_CONFIGURATION_BATTERY_3_RATED_VOLTAGE ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0001",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0075",
						"_name": "Battery3AlarmMask",
						"_type": "BatteryAlarmMask",
						"_zigbeetype": "ZCL_DATATYPE_BITMAP8",
						"_definition": "ATTRID_POWER_CONFIGURATION_BATTERY_3_ALARM_MASK ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0001",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0076",
						"_name": "Battery3VoltageMinThreshold",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_POWER_CONFIGURATION_BATTERY_3_VOLTAGE_MIN_THRESHOLD ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0001",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0077",
						"_name": "Battery3VoltageThreshold1",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_POWER_CONFIGURATION_BATTERY_3_VOLTAGE_THRESHOLD_1 ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0001",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0078",
						"_name": "Battery3VoltageThreshold2",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_POWER_CONFIGURATION_BATTERY_3_VOLTAGE_THRESHOLD_2 ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0001",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0079",
						"_name": "Battery3VoltageThreshold3",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_POWER_CONFIGURATION_BATTERY_3_VOLTAGE_THRESHOLD_3 ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0001",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x007a",
						"_name": "Battery3PercentageMinThreshold",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_POWER_CONFIGURATION_BATTERY_3_PERCENTAGE_MIN_THRESHOLD ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0001",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x007b",
						"_name": "Battery3PercentageThreshold1",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_POWER_CONFIGURATION_BATTERY_3_PERCENTAGE_THRESHOLD_1 ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0001",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x007c",
						"_name": "Battery3PercentageThreshold2",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_POWER_CONFIGURATION_BATTERY_3_PERCENTAGE_THRESHOLD_2 ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0001",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x007d",
						"_name": "Battery3PercentageThreshold3",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_POWER_CONFIGURATION_BATTERY_3_PERCENTAGE_THRESHOLD_3 ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0001",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x007e",
						"_name": "Battery3AlarmState",
						"_type": "BatteryAlarmState",
						"_zigbeetype": "ZCL_DATATYPE_BITMAP32",
						"_definition": "ATTRID_POWER_CONFIGURATION_BATTERY_3_ALARM_STATE ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0001",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					}
				]
			},
			"client": {
				"attribute": [
					{
						"_id": "0xfffd",
						"_name": "ClusterRevision",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_CLUSTER_REVISION",
						"_location": "zcl.h",
						"_clusterID": "0x0001",
						"_default": "1",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0xfffe",
						"_name": "AttributeReportingStatus",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_ATTRIBUTE_REPORTING_STATUS",
						"_location": "zcl.h",
						"_clusterID": "0x0001",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					}
				],
			},
			"appcallback": {
				"callbacks": [
					{
						"_name": "BasicReset",
						"_returnType": "void",
						"_arguments": "( void )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Reset to Factory Defaults"
							},
							{
								"_comment": "/// command. On receipt of this command, the device resets all the attributes"
							},
							{
								"_comment": "/// of all its clusters to their factory defaults."
							}
						]
					},
					{
						"_name": "Identify",
						"_returnType": "void",
						"_arguments": "( zclIdentify_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Identify command."
							}
						]
					},
					{
						"_name": "IdentifyQuery",
						"_returnType": "void",
						"_arguments": "( zclIdentifyQuery_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Identify Query command."
							}
						]
					},
					{
						"_name": "IdentifyQueryRsp",
						"_returnType": "void",
						"_arguments": "( zclIdentifyQueryRsp_t *pRsp )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Identify Query Response command."
							}
						]
					},
					{
						"_name": "IdentifyTriggerEffect",
						"_returnType": "void",
						"_arguments": "( zclIdentifyTriggerEffect_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Identify Trigger Effect command."
							}
						]
					},
					{
						"_name": "#ifdef ZCL_ON_OFF",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "OnOff",
						"_returnType": "void",
						"_arguments": "( uint8_t cmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming On, Off or Toggle command."
							}
						]
					},
					{
						"_name": "OnOff_OffWithEffect",
						"_returnType": "void",
						"_arguments": "( zclOffWithEffect_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Off with Effect"
							}
						]
					},
					{
						"_name": "OnOff_OnWithRecallGlobalScene",
						"_returnType": "void",
						"_arguments": "( void )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming On with Recall Global Scene command."
							}
						]
					},
					{
						"_name": "OnOff_OnWithTimedOff",
						"_returnType": "void",
						"_arguments": "( zclOnWithTimedOff_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming On with Timed Off."
							}
						]
					},
					{
						"_name": "#endif",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "#ifdef ZCL_LEVEL_CTRL",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "LevelControlMoveToLevel",
						"_returnType": "void",
						"_arguments": "( zclLCMoveToLevel_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Level Control - Move to Level command"
							}
						]
					},
					{
						"_name": "LevelControlMove",
						"_returnType": "void",
						"_arguments": "( zclLCMove_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Level Control - Move command"
							}
						]
					},
					{
						"_name": "LevelControlStep",
						"_returnType": "void",
						"_arguments": "( zclLCStep_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Level Control - Step command"
							}
						]
					},
					{
						"_name": "LevelControlStop",
						"_returnType": "void",
						"_arguments": "( zclLCStop_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Level Control - Stop command"
							}
						]
					},
					{
						"_name": "LevelControlMoveFreq",
						"_returnType": "void",
						"_arguments": "( zclLCMoveFreq_t *pCmd)",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Level Control - Move to Closest Frequency command"
							}
						]
					},
					{
						"_name": "#endif",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "#ifdef ZCL_GROUPS",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "GroupRsp",
						"_returnType": "void",
						"_arguments": "( zclGroupRsp_t *pRsp )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an received Group Response message."
							},
							{
								"_comment": "/// This means that this app sent the request message."
							}
						]
					},
					{
						"_name": "#endif",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "#ifdef ZCL_SCENES",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "SceneStoreReq",
						"_returnType": "uint8_t",
						"_arguments": "( zclSceneReq_t *pReq )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Scene Store request."
							},
							{
								"_comment": "/// The app will fill in the 'extField' with what is needed to restore its"
							},
							{
								"_comment": "/// current settings. Before overwriting the content of the scene,"
							},
							{
								"_comment": "/// App needs to verify if there was a change with the previous configuration."
							},
							{
								"_comment": "/// App function returns TRUE if there was a change, otherwise returns FALSE."
							}
						]
					},
					{
						"_name": "SceneRecallReq",
						"_returnType": "void",
						"_arguments": "( zclSceneReq_t *pReq )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Scene Recall request"
							},
							{
								"_comment": "/// The app will use what's in the 'extField' to restore to these settings."
							}
						]
					},
					{
						"_name": "SceneRsp",
						"_returnType": "void",
						"_arguments": "( zclSceneRsp_t *pRsp )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Scene responses. This means"
							},
							{
								"_comment": "/// that this app sent the request for this response."
							}
						]
					},
					{
						"_name": "#endif",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "#ifdef ZCL_ALARMS",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "Alarm",
						"_returnType": "void",
						"_arguments": "( uint8_t direction, zclAlarm_t *pAlarm )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Alarm request or response command."
							}
						]
					},
					{
						"_name": "#endif",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "#ifdef SE_UK_EXT",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "GetEventLog",
						"_returnType": "void",
						"_arguments": "( uint8_t srcEP, afAddrType_t *srcAddr, zclGetEventLog_t *pEventLog, uint8_t seqNum )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Alarm Get Event Log command."
							}
						]
					},
					{
						"_name": "PublishEventLog",
						"_returnType": "void",
						"_arguments": "( afAddrType_t *srcAddr, zclPublishEventLog_t *pEventLog )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Alarm Publish Event Log command."
							}
						]
					},
					{
						"_name": "#endif",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "Location",
						"_returnType": "void",
						"_arguments": "( zclLocation_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to to process an incoming RSSI Location command."
							}
						]
					},
					{
						"_name": "LocationRsp",
						"_returnType": "void",
						"_arguments": "( zclLocationRsp_t *pRsp )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming RSSI Location response command."
							},
							{
								"_comment": "/// This means  that this app sent the request for this response."
							}
						]
					}
				],
				"_name": "zclGeneral_AppCallbacks_t",
				"_register": "zclGeneral_RegisterCmdCallbacks"
			},
			"_id": "0x0001",
			"_name": "PowerConfiguration",
			"_definition": "ZCL_CLUSTER_ID_GENERAL_POWER_CFG",
			"_primary_transaction": "0",
			"_class": "general",
			"_flag": "ZCL_POWER_PROFILE",
			"_location": "zcl_general.h"
		},
		{
			"server": {
				"attribute": [
					{
						"_id": "0xfffd",
						"_name": "ClusterRevision",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_CLUSTER_REVISION",
						"_location": "zcl.h",
						"_clusterID": "0x0403",
						"_default": "1",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0xfffe",
						"_name": "AttributeReportingStatus",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_ATTRIBUTE_REPORTING_STATUS",
						"_location": "zcl.h",
						"_clusterID": "0x0403",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0000",
						"_name": "MeasuredValue",
						"_type": "int16",
						"_zigbeetype": "ZCL_DATATYPE_INT16",
						"_definition": "ATTRID_PRESSURE_MEASUREMENT_MEASURED_VALUE ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0403",
						"_default": "-32768",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "true",
						"_required": "true"
					},
					{
						"_id": "0x0001",
						"_name": "MinMeasuredValue",
						"_type": "int16",
						"_zigbeetype": "ZCL_DATATYPE_INT16",
						"_definition": "ATTRID_PRESSURE_MEASUREMENT_MIN_MEASURED_VALUE ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0403",
						"_default": "-32768",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0x0002",
						"_name": "MaxMeasuredValue",
						"_type": "int16",
						"_zigbeetype": "ZCL_DATATYPE_INT16",
						"_definition": "ATTRID_PRESSURE_MEASUREMENT_MAX_MEASURED_VALUE ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0403",
						"_default": "-32768",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0x0003",
						"_name": "Tolerance",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_PRESSURE_MEASUREMENT_TOLERANCE ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0403",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0010",
						"_name": "ScaledValue",
						"_type": "int16",
						"_zigbeetype": "ZCL_DATATYPE_INT16",
						"_definition": "ATTRID_PRESSURE_MEASUREMENT_SCALED_VALUE ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0403",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0011",
						"_name": "MinScaledValue",
						"_type": "int16",
						"_zigbeetype": "ZCL_DATATYPE_INT16",
						"_definition": "ATTRID_PRESSURE_MEASUREMENT_MIN_SCALED_VALUE ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0403",
						"_default": "-32768",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0012",
						"_name": "MaxScaledValue",
						"_type": "int16",
						"_zigbeetype": "ZCL_DATATYPE_INT16",
						"_definition": "ATTRID_PRESSURE_MEASUREMENT_MAX_SCALED_VALUE ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0403",
						"_default": "-32768",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0013",
						"_name": "ScaledTolerance",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_PRESSURE_MEASUREMENT_SCALED_TOLERANCE ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0403",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0014",
						"_name": "Scale",
						"_type": "int8",
						"_zigbeetype": "ZCL_DATATYPE_INT8",
						"_definition": "ATTRID_PRESSURE_MEASUREMENT_SCALE ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0403",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					}
				]
			},
			"client": {
				"attribute": [
					{
						"_id": "0xfffd",
						"_name": "ClusterRevision",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_CLUSTER_REVISION",
						"_location": "zcl.h",
						"_clusterID": "0x0403",
						"_default": "1",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0xfffe",
						"_name": "AttributeReportingStatus",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_ATTRIBUTE_REPORTING_STATUS",
						"_location": "zcl.h",
						"_clusterID": "0x0403",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					}
				],
			},
			"appcallback": {
				"callbacks": [
					{
						"_name": "MSPlaceHolder",
						"_returnType": "void",
						"_arguments": "( void )"
					}
				],
				"_name": "zclMS_AppCallbacks_t",
				"_register": "zclMS_RegisterCmdCallbacks"
			},
			"_id": "0x0403",
			"_name": "PressureMeasurement",
			"_definition": "ZCL_CLUSTER_ID_MS_PRESSURE_MEASUREMENT",
			"_primary_transaction": "2",
			"_class": "ms",
			"_flag": "ZCL_MS",
			"_location": "zcl_ms.h"
		},
		{
			"server": {
				"attribute": [
					{
						"_id": "0xfffd",
						"_name": "ClusterRevision",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_CLUSTER_REVISION",
						"_location": "zcl.h",
						"_clusterID": "0x0200",
						"_default": "1",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0xfffe",
						"_name": "AttributeReportingStatus",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_ATTRIBUTE_REPORTING_STATUS",
						"_location": "zcl.h",
						"_clusterID": "0x0200",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0000",
						"_name": "MaxPressure",
						"_type": "int16",
						"_zigbeetype": "ZCL_DATATYPE_INT16",
						"_definition": "ATTRID_PUMP_CONFIGURATION_AND_CONTROL_MAX_PRESSURE ",
						"_location": "zcl_hvac.h",
						"_clusterID": "0x0200",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0x0001",
						"_name": "MaxSpeed",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_PUMP_CONFIGURATION_AND_CONTROL_MAX_SPEED ",
						"_location": "zcl_hvac.h",
						"_clusterID": "0x0200",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0x0002",
						"_name": "MaxFlow",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_PUMP_CONFIGURATION_AND_CONTROL_MAX_FLOW ",
						"_location": "zcl_hvac.h",
						"_clusterID": "0x0200",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0x0003",
						"_name": "MinConstPressure",
						"_type": "int16",
						"_zigbeetype": "ZCL_DATATYPE_INT16",
						"_definition": "ATTRID_PUMP_CONFIGURATION_AND_CONTROL_MIN_CONST_PRESSURE ",
						"_location": "zcl_hvac.h",
						"_clusterID": "0x0200",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0004",
						"_name": "MaxConstPressure",
						"_type": "int16",
						"_zigbeetype": "ZCL_DATATYPE_INT16",
						"_definition": "ATTRID_PUMP_CONFIGURATION_AND_CONTROL_MAX_CONST_PRESSURE ",
						"_location": "zcl_hvac.h",
						"_clusterID": "0x0200",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0005",
						"_name": "MinCompPressure",
						"_type": "int16",
						"_zigbeetype": "ZCL_DATATYPE_INT16",
						"_definition": "ATTRID_PUMP_CONFIGURATION_AND_CONTROL_MIN_COMP_PRESSURE ",
						"_location": "zcl_hvac.h",
						"_clusterID": "0x0200",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0006",
						"_name": "MaxCompPressure",
						"_type": "int16",
						"_zigbeetype": "ZCL_DATATYPE_INT16",
						"_definition": "ATTRID_PUMP_CONFIGURATION_AND_CONTROL_MAX_COMP_PRESSURE ",
						"_location": "zcl_hvac.h",
						"_clusterID": "0x0200",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0007",
						"_name": "MinConstSpeed",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_PUMP_CONFIGURATION_AND_CONTROL_MIN_CONST_SPEED ",
						"_location": "zcl_hvac.h",
						"_clusterID": "0x0200",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0008",
						"_name": "MaxConstSpeed",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_PUMP_CONFIGURATION_AND_CONTROL_MAX_CONST_SPEED ",
						"_location": "zcl_hvac.h",
						"_clusterID": "0x0200",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0009",
						"_name": "MinConstFlow",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_PUMP_CONFIGURATION_AND_CONTROL_MIN_CONST_FLOW ",
						"_location": "zcl_hvac.h",
						"_clusterID": "0x0200",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x000a",
						"_name": "MaxConstFlow",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_PUMP_CONFIGURATION_AND_CONTROL_MAX_CONST_FLOW ",
						"_location": "zcl_hvac.h",
						"_clusterID": "0x0200",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x000b",
						"_name": "MinConstTemp",
						"_type": "int16",
						"_zigbeetype": "ZCL_DATATYPE_INT16",
						"_definition": "ATTRID_PUMP_CONFIGURATION_AND_CONTROL_MIN_CONST_TEMP ",
						"_location": "zcl_hvac.h",
						"_clusterID": "0x0200",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x000c",
						"_name": "MaxConstTemp",
						"_type": "int16",
						"_zigbeetype": "ZCL_DATATYPE_INT16",
						"_definition": "ATTRID_PUMP_CONFIGURATION_AND_CONTROL_MAX_CONST_TEMP ",
						"_location": "zcl_hvac.h",
						"_clusterID": "0x0200",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0010",
						"_name": "PumpStatus",
						"_type": "map16",
						"_zigbeetype": "ZCL_DATATYPE_BITMAP16",
						"_definition": "ATTRID_PUMP_CONFIGURATION_AND_CONTROL_PUMP_STATUS ",
						"_location": "zcl_hvac.h",
						"_clusterID": "0x0200",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0011",
						"_name": "EffectiveOperationMode",
						"_type": "PumpOperationMode",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_PUMP_CONFIGURATION_AND_CONTROL_EFFECTIVE_OPERATION_MODE ",
						"_location": "zcl_hvac.h",
						"_clusterID": "0x0200",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0x0012",
						"_name": "EffectiveControlMode",
						"_type": "PumpControlMode",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_PUMP_CONFIGURATION_AND_CONTROL_EFFECTIVE_CONTROL_MODE ",
						"_location": "zcl_hvac.h",
						"_clusterID": "0x0200",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0x0013",
						"_name": "Capacity",
						"_type": "int16",
						"_zigbeetype": "ZCL_DATATYPE_INT16",
						"_definition": "ATTRID_PUMP_CONFIGURATION_AND_CONTROL_CAPACITY ",
						"_location": "zcl_hvac.h",
						"_clusterID": "0x0200",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "true",
						"_required": "true"
					},
					{
						"_id": "0x0014",
						"_name": "Speed",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_PUMP_CONFIGURATION_AND_CONTROL_SPEED ",
						"_location": "zcl_hvac.h",
						"_clusterID": "0x0200",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0015",
						"_name": "LifetimeRunningHours",
						"_type": "uint24",
						"_zigbeetype": "ZCL_DATATYPE_UINT24",
						"_definition": "ATTRID_PUMP_CONFIGURATION_AND_CONTROL_LIFETIME_RUNNING_HOURS ",
						"_location": "zcl_hvac.h",
						"_clusterID": "0x0200",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0016",
						"_name": "Power",
						"_type": "uint24",
						"_zigbeetype": "ZCL_DATATYPE_UINT24",
						"_definition": "ATTRID_PUMP_CONFIGURATION_AND_CONTROL_POWER ",
						"_location": "zcl_hvac.h",
						"_clusterID": "0x0200",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0017",
						"_name": "LifetimeEnergyConsumed",
						"_type": "uint32",
						"_zigbeetype": "ZCL_DATATYPE_UINT32",
						"_definition": "ATTRID_PUMP_CONFIGURATION_AND_CONTROL_LIFETIME_ENERGY_CONSUMED ",
						"_location": "zcl_hvac.h",
						"_clusterID": "0x0200",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0020",
						"_name": "OperationMode",
						"_type": "PumpOperationMode",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_PUMP_CONFIGURATION_AND_CONTROL_OPERATION_MODE ",
						"_location": "zcl_hvac.h",
						"_clusterID": "0x0200",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0x0021",
						"_name": "ControlMode",
						"_type": "PumpControlMode",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_PUMP_CONFIGURATION_AND_CONTROL_CONTROL_MODE ",
						"_location": "zcl_hvac.h",
						"_clusterID": "0x0200",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0022",
						"_name": "AlarmMask",
						"_type": "map16",
						"_zigbeetype": "ZCL_DATATYPE_BITMAP16",
						"_definition": "ATTRID_PUMP_CONFIGURATION_AND_CONTROL_ALARM_MASK ",
						"_location": "zcl_hvac.h",
						"_clusterID": "0x0200",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					}
				]
			},
			"client": {
				"attribute": [
					{
						"_id": "0xfffd",
						"_name": "ClusterRevision",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_CLUSTER_REVISION",
						"_location": "zcl.h",
						"_clusterID": "0x0200",
						"_default": "1",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0xfffe",
						"_name": "AttributeReportingStatus",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_ATTRIBUTE_REPORTING_STATUS",
						"_location": "zcl.h",
						"_clusterID": "0x0200",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					}
				],
			},
			"appcallback": {
				"callbacks": [
					{
						"_name": "HVAC_SetpointRaiseLower",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclCmdThermostatSetpointRaiseLowerPayload_t *pCmd )",
						"comments": [
							{
								"_comment": "// This callback is called to process a Setpoint Raise/Lower command"
							}
						]
					},
					{
						"_name": "HVAC_SetWeeklySchedule",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclThermostatWeeklySchedule_t *pCmd )",
						"comments": [
							{
								"_comment": "// This callback is called to process a Set Weekly Schedule command"
							}
						]
					},
					{
						"_name": "HVAC_GetWeeklySchedule",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclThermostatGetWeeklySchedule_t *pCmd )",
						"comments": [
							{
								"_comment": "// This callback is called to process a Get Weekly Schedule command"
							}
						]
					},
					{
						"_name": "HVAC_ClearWeeklySchedule",
						"_returnType": "ZStatus_t",
						"_arguments": "( void )",
						"comments": [
							{
								"_comment": "// This callback is called to process a Clear Weekly Schedule command"
							}
						]
					},
					{
						"_name": "HVAC_GetRelayStatusLog",
						"_returnType": "ZStatus_t",
						"_arguments": "( void )",
						"comments": [
							{
								"_comment": "// This callback is called to process a Get Relay Status Log command"
							}
						]
					},
					{
						"_name": "HVAC_GetWeeklyScheduleRsp",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclThermostatWeeklySchedule_t *pCmd )",
						"comments": [
							{
								"_comment": "// This callback is called to process a Get Weekly Schedule Response command"
							}
						]
					},
					{
						"_name": "HVAC_GetRelayStatusLogRsp",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclThermostatGetRelayStatusLogRsp_t *pCmd )",
						"comments": [
							{
								"_comment": "// This callback is called to process a Get Relay Status Log Response command"
							}
						]
					}
				],
				"_name": "zclHVAC_AppCallbacks_t",
				"_register": "zclHVAC_RegisterCmdCallbacks"
			},
			"_id": "0x0200",
			"_name": "PumpConfigurationAndControl",
			"_definition": "ZCL_CLUSTER_ID_HVAC_PUMP_CONFIG_CONTROL",
			"_primary_transaction": "2",
			"_class": "hvac",
			"_flag": "ZCL_HVAC_CLUSTER",
			"_location": "zcl_hvac.h"
		},
		{
			"server": {
				"attribute": [
					{
						"_id": "0xfffd",
						"_name": "ClusterRevision",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_CLUSTER_REVISION",
						"_location": "zcl.h",
						"_clusterID": "0x0005",
						"_default": "1",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0xfffe",
						"_name": "AttributeReportingStatus",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_ATTRIBUTE_REPORTING_STATUS",
						"_location": "zcl.h",
						"_clusterID": "0x0005",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0000",
						"_name": "SceneCount",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_SCENES_SCENE_COUNT ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0005",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0x0001",
						"_name": "CurrentScene",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_SCENES_CURRENT_SCENE ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0005",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0x0002",
						"_name": "CurrentGroup",
						"_type": "SGroupId",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_SCENES_CURRENT_GROUP ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0005",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0x0003",
						"_name": "SceneValid",
						"_type": "bool",
						"_zigbeetype": "ZCL_DATATYPE_BOOLEAN",
						"_definition": "ATTRID_SCENES_SCENE_VALID ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0005",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0x0004",
						"_name": "NameSupport",
						"_type": "map8",
						"_zigbeetype": "ZCL_DATATYPE_BITMAP8",
						"_definition": "ATTRID_SCENES_NAME_SUPPORT ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0005",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0x0005",
						"_name": "LastConfiguredBy",
						"_type": "EUI64",
						"_zigbeetype": "ZCL_DATATYPE_BITMAP64",
						"_definition": "ATTRID_SCENES_LAST_CONFIGURED_BY ",
						"_location": "zcl_general.h",
						"_clusterID": "0x0005",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					}
				],
				"command": [
					{
						"_id": "0x00",
						"_name": "AddScene",
						"_definition": "COMMAND_SCENES_ADD_SCENE",
						"_location": "zcl_general.h",
						"_clusterID": "0x0005",
						"_required": "true"
					},
					{
						"_id": "0x01",
						"_name": "ViewScene",
						"_definition": "COMMAND_SCENES_VIEW_SCENE",
						"_location": "zcl_general.h",
						"_clusterID": "0x0005",
						"_required": "true"
					},
					{
						"_id": "0x02",
						"_name": "RemoveScene",
						"_definition": "COMMAND_SCENES_REMOVE_SCENE",
						"_location": "zcl_general.h",
						"_clusterID": "0x0005",
						"_required": "true"
					},
					{
						"_id": "0x03",
						"_name": "RemoveAllScenes",
						"_definition": "COMMAND_SCENES_REMOVE_ALL_SCENES",
						"_location": "zcl_general.h",
						"_clusterID": "0x0005",
						"_required": "true"
					},
					{
						"_id": "0x04",
						"_name": "StoreScene",
						"_definition": "COMMAND_SCENES_STORE_SCENE",
						"_location": "zcl_general.h",
						"_callback": "SceneStoreReq",
						"_clusterID": "0x0005",
						"_required": "true"
					},
					{
						"_id": "0x05",
						"_name": "RecallScene",
						"_definition": "COMMAND_SCENES_RECALL_SCENE",
						"_location": "zcl_general.h",
						"_callback": "SceneRecallReq",
						"_clusterID": "0x0005",
						"_required": "true"
					},
					{
						"_id": "0x06",
						"_name": "GetSceneMembership",
						"_definition": "COMMAND_SCENES_GET_SCENE_MEMBERSHIP",
						"_location": "zcl_general.h",
						"_clusterID": "0x0005",
						"_required": "true"
					},
					{
						"_id": "0x40",
						"_name": "EnhancedAddScene",
						"_definition": "COMMAND_SCENES_ENHANCED_ADD_SCENE",
						"_location": "zcl_general.h",
						"_clusterID": "0x0005",
						"_required": "false"
					},
					{
						"_id": "0x41",
						"_name": "EnhancedViewScene",
						"_definition": "COMMAND_SCENES_ENHANCED_VIEW_SCENE",
						"_location": "zcl_general.h",
						"_clusterID": "0x0005",
						"_required": "false"
					},
					{
						"_id": "0x42",
						"_name": "CopyScene",
						"_definition": "COMMAND_SCENES_COPY_SCENE",
						"_location": "zcl_general.h",
						"_clusterID": "0x0005",
						"_required": "false"
					}
				]
			},
			"client": {
				"attribute": [
					{
						"_id": "0xfffd",
						"_name": "ClusterRevision",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_CLUSTER_REVISION",
						"_location": "zcl.h",
						"_clusterID": "0x0005",
						"_default": "1",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0xfffe",
						"_name": "AttributeReportingStatus",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_ATTRIBUTE_REPORTING_STATUS",
						"_location": "zcl.h",
						"_clusterID": "0x0005",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					}
				],
				"command": [
					{
						"_id": "0x00",
						"_name": "AddSceneResponse",
						"_definition": "COMMAND_SCENES_ADD_SCENE_RESPONSE",
						"_location": "zcl_general.h",
						"_clusterID": "0x0005",
						"_required": "true"
					},
					{
						"_id": "0x01",
						"_name": "ViewSceneResponse",
						"_definition": "COMMAND_SCENES_VIEW_SCENE_RESPONSE",
						"_location": "zcl_general.h",
						"_clusterID": "0x0005",
						"_required": "true"
					},
					{
						"_id": "0x02",
						"_name": "RemoveSceneResponse",
						"_definition": "COMMAND_SCENES_REMOVE_SCENE_RESPONSE",
						"_location": "zcl_general.h",
						"_clusterID": "0x0005",
						"_required": "true"
					},
					{
						"_id": "0x03",
						"_name": "RemoveAllScenesResponse",
						"_definition": "COMMAND_SCENES_REMOVE_ALL_SCENES_RESPONSE",
						"_location": "zcl_general.h",
						"_clusterID": "0x0005",
						"_required": "true"
					},
					{
						"_id": "0x04",
						"_name": "StoreSceneResponse",
						"_definition": "COMMAND_SCENES_STORE_SCENE_RESPONSE",
						"_location": "zcl_general.h",
						"_clusterID": "0x0005",
						"_required": "true"
					},
					{
						"_id": "0x06",
						"_name": "GetSceneMembershipResponse",
						"_definition": "COMMAND_SCENES_GET_SCENE_MEMBERSHIP_RESPONSE",
						"_location": "zcl_general.h",
						"_clusterID": "0x0005",
						"_required": "true"
					},
					{
						"_id": "0x40",
						"_name": "EnhancedAddSceneResponse",
						"_definition": "COMMAND_SCENES_ENHANCED_ADD_SCENE_RESPONSE",
						"_location": "zcl_general.h",
						"_clusterID": "0x0005",
						"_required": "false"
					},
					{
						"_id": "0x41",
						"_name": "EnhancedViewSceneResponse",
						"_definition": "COMMAND_SCENES_ENHANCED_VIEW_SCENE_RESPONSE",
						"_location": "zcl_general.h",
						"_clusterID": "0x0005",
						"_required": "false"
					},
					{
						"_id": "0x42",
						"_name": "CopySceneResponse",
						"_definition": "COMMAND_SCENES_COPY_SCENE_RESPONSE",
						"_location": "zcl_general.h",
						"_clusterID": "0x0005",
						"_required": "false"
					}
				]
			},
			"appcallback": {
				"callbacks": [
					{
						"_name": "BasicReset",
						"_returnType": "void",
						"_arguments": "( void )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Reset to Factory Defaults"
							},
							{
								"_comment": "/// command. On receipt of this command, the device resets all the attributes"
							},
							{
								"_comment": "/// of all its clusters to their factory defaults."
							}
						]
					},
					{
						"_name": "Identify",
						"_returnType": "void",
						"_arguments": "( zclIdentify_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Identify command."
							}
						]
					},
					{
						"_name": "IdentifyQuery",
						"_returnType": "void",
						"_arguments": "( zclIdentifyQuery_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Identify Query command."
							}
						]
					},
					{
						"_name": "IdentifyQueryRsp",
						"_returnType": "void",
						"_arguments": "( zclIdentifyQueryRsp_t *pRsp )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Identify Query Response command."
							}
						]
					},
					{
						"_name": "IdentifyTriggerEffect",
						"_returnType": "void",
						"_arguments": "( zclIdentifyTriggerEffect_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Identify Trigger Effect command."
							}
						]
					},
					{
						"_name": "#ifdef ZCL_ON_OFF",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "OnOff",
						"_returnType": "void",
						"_arguments": "( uint8_t cmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming On, Off or Toggle command."
							}
						]
					},
					{
						"_name": "OnOff_OffWithEffect",
						"_returnType": "void",
						"_arguments": "( zclOffWithEffect_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Off with Effect"
							}
						]
					},
					{
						"_name": "OnOff_OnWithRecallGlobalScene",
						"_returnType": "void",
						"_arguments": "( void )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming On with Recall Global Scene command."
							}
						]
					},
					{
						"_name": "OnOff_OnWithTimedOff",
						"_returnType": "void",
						"_arguments": "( zclOnWithTimedOff_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming On with Timed Off."
							}
						]
					},
					{
						"_name": "#endif",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "#ifdef ZCL_LEVEL_CTRL",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "LevelControlMoveToLevel",
						"_returnType": "void",
						"_arguments": "( zclLCMoveToLevel_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Level Control - Move to Level command"
							}
						]
					},
					{
						"_name": "LevelControlMove",
						"_returnType": "void",
						"_arguments": "( zclLCMove_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Level Control - Move command"
							}
						]
					},
					{
						"_name": "LevelControlStep",
						"_returnType": "void",
						"_arguments": "( zclLCStep_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Level Control - Step command"
							}
						]
					},
					{
						"_name": "LevelControlStop",
						"_returnType": "void",
						"_arguments": "( zclLCStop_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Level Control - Stop command"
							}
						]
					},
					{
						"_name": "LevelControlMoveFreq",
						"_returnType": "void",
						"_arguments": "( zclLCMoveFreq_t *pCmd)",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Level Control - Move to Closest Frequency command"
							}
						]
					},
					{
						"_name": "#endif",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "#ifdef ZCL_GROUPS",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "GroupRsp",
						"_returnType": "void",
						"_arguments": "( zclGroupRsp_t *pRsp )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an received Group Response message."
							},
							{
								"_comment": "/// This means that this app sent the request message."
							}
						]
					},
					{
						"_name": "#endif",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "#ifdef ZCL_SCENES",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "SceneStoreReq",
						"_returnType": "uint8_t",
						"_arguments": "( zclSceneReq_t *pReq )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Scene Store request."
							},
							{
								"_comment": "/// The app will fill in the 'extField' with what is needed to restore its"
							},
							{
								"_comment": "/// current settings. Before overwriting the content of the scene,"
							},
							{
								"_comment": "/// App needs to verify if there was a change with the previous configuration."
							},
							{
								"_comment": "/// App function returns TRUE if there was a change, otherwise returns FALSE."
							}
						]
					},
					{
						"_name": "SceneRecallReq",
						"_returnType": "void",
						"_arguments": "( zclSceneReq_t *pReq )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Scene Recall request"
							},
							{
								"_comment": "/// The app will use what's in the 'extField' to restore to these settings."
							}
						]
					},
					{
						"_name": "SceneRsp",
						"_returnType": "void",
						"_arguments": "( zclSceneRsp_t *pRsp )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Scene responses. This means"
							},
							{
								"_comment": "/// that this app sent the request for this response."
							}
						]
					},
					{
						"_name": "#endif",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "#ifdef ZCL_ALARMS",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "Alarm",
						"_returnType": "void",
						"_arguments": "( uint8_t direction, zclAlarm_t *pAlarm )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Alarm request or response command."
							}
						]
					},
					{
						"_name": "#endif",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "#ifdef SE_UK_EXT",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "GetEventLog",
						"_returnType": "void",
						"_arguments": "( uint8_t srcEP, afAddrType_t *srcAddr, zclGetEventLog_t *pEventLog, uint8_t seqNum )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Alarm Get Event Log command."
							}
						]
					},
					{
						"_name": "PublishEventLog",
						"_returnType": "void",
						"_arguments": "( afAddrType_t *srcAddr, zclPublishEventLog_t *pEventLog )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Alarm Publish Event Log command."
							}
						]
					},
					{
						"_name": "#endif",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "Location",
						"_returnType": "void",
						"_arguments": "( zclLocation_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to to process an incoming RSSI Location command."
							}
						]
					},
					{
						"_name": "LocationRsp",
						"_returnType": "void",
						"_arguments": "( zclLocationRsp_t *pRsp )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming RSSI Location response command."
							},
							{
								"_comment": "/// This means  that this app sent the request for this response."
							}
						]
					}
				],
				"_name": "zclGeneral_AppCallbacks_t",
				"_register": "zclGeneral_RegisterCmdCallbacks"
			},
			"_id": "0x0005",
			"_name": "Scenes",
			"_definition": "ZCL_CLUSTER_ID_GENERAL_SCENES",
			"_primary_transaction": "1",
			"_class": "general",
			"_flag": "ZCL_SCENES",
			"_location": "zcl_general.h"
		},
		{
			"server": {
				"attribute": [
					{
						"_id": "0xfffd",
						"_name": "ClusterRevision",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_CLUSTER_REVISION",
						"_location": "zcl.h",
						"_clusterID": "0x0100",
						"_default": "1",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0xfffe",
						"_name": "AttributeReportingStatus",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_ATTRIBUTE_REPORTING_STATUS",
						"_location": "zcl.h",
						"_clusterID": "0x0100",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0000",
						"_name": "PhysicalClosedLimit",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_SHADE_CONFIGURATION_PHYSICAL_CLOSED_LIMIT ",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0100",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0001",
						"_name": "MotorStepSize",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_SHADE_CONFIGURATION_MOTOR_STEP_SIZE ",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0100",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0002",
						"_name": "Status",
						"_type": "map8",
						"_zigbeetype": "ZCL_DATATYPE_BITMAP8",
						"_definition": "ATTRID_SHADE_CONFIGURATION_STATUS ",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0100",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0x0010",
						"_name": "ClosedLimit",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_SHADE_CONFIGURATION_CLOSED_LIMIT ",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0100",
						"_default": "1",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0x0011",
						"_name": "Mode",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_SHADE_CONFIGURATION_MODE ",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0100",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "true"
					}
				]
			},
			"client": {
				"attribute": [
					{
						"_id": "0xfffd",
						"_name": "ClusterRevision",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_CLUSTER_REVISION",
						"_location": "zcl.h",
						"_clusterID": "0x0100",
						"_default": "1",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0xfffe",
						"_name": "AttributeReportingStatus",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_ATTRIBUTE_REPORTING_STATUS",
						"_location": "zcl.h",
						"_clusterID": "0x0100",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					}
				],
			},
			"appcallback": {
				"_name": "None",
				"_register": "None"
			},
			"_id": "0x0100",
			"_name": "ShadeConfiguration",
			"_definition": "ZCL_CLUSTER_ID_CLOSURES_SHADE_CONFIG",
			"_primary_transaction": "2",
			"_class": "closures",
			"_flag": "ZCL_WINDOWCOVERING",
			"_location": "zcl_closures.h"
		},
		{
			"server": {
				"attribute": [
					{
						"_id": "0xfffd",
						"_name": "ClusterRevision",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_CLUSTER_REVISION",
						"_location": "zcl.h",
						"_clusterID": "0x0402",
						"_default": "1",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0xfffe",
						"_name": "AttributeReportingStatus",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_ATTRIBUTE_REPORTING_STATUS",
						"_location": "zcl.h",
						"_clusterID": "0x0402",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0000",
						"_name": "MeasuredValue",
						"_type": "int16",
						"_zigbeetype": "ZCL_DATATYPE_INT16",
						"_definition": "ATTRID_TEMPERATURE_MEASUREMENT_MEASURED_VALUE ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0402",
						"_default": "-32768",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "true",
						"_required": "true"
					},
					{
						"_id": "0x0001",
						"_name": "MinMeasuredValue",
						"_type": "int16",
						"_zigbeetype": "ZCL_DATATYPE_INT16",
						"_definition": "ATTRID_TEMPERATURE_MEASUREMENT_MIN_MEASURED_VALUE ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0402",
						"_default": "-32768",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0x0002",
						"_name": "MaxMeasuredValue",
						"_type": "int16",
						"_zigbeetype": "ZCL_DATATYPE_INT16",
						"_definition": "ATTRID_TEMPERATURE_MEASUREMENT_MAX_MEASURED_VALUE ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0402",
						"_default": "-32768",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0x0003",
						"_name": "Tolerance",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_TEMPERATURE_MEASUREMENT_TOLERANCE ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0402",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					}
				]
			},
			"client": {
				"attribute": [
					{
						"_id": "0xfffd",
						"_name": "ClusterRevision",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_CLUSTER_REVISION",
						"_location": "zcl.h",
						"_clusterID": "0x0402",
						"_default": "1",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0xfffe",
						"_name": "AttributeReportingStatus",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_ATTRIBUTE_REPORTING_STATUS",
						"_location": "zcl.h",
						"_clusterID": "0x0402",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					}
				],
			},
			"appcallback": {
				"callbacks": [
					{
						"_name": "MSPlaceHolder",
						"_returnType": "void",
						"_arguments": "( void )"
					}
				],
				"_name": "zclMS_AppCallbacks_t",
				"_register": "zclMS_RegisterCmdCallbacks"
			},
			"_id": "0x0402",
			"_name": "TemperatureMeasurement",
			"_definition": "ZCL_CLUSTER_ID_MS_TEMPERATURE_MEASUREMENT",
			"_primary_transaction": "2",
			"_class": "ms",
			"_flag": "ZCL_MS",
			"_location": "zcl_ms.h"
		},
		{
			"server": {
				"attribute": [
					{
						"_id": "0xfffd",
						"_name": "ClusterRevision",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_CLUSTER_REVISION",
						"_location": "zcl.h",
						"_clusterID": "0x0201",
						"_default": "1",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0xfffe",
						"_name": "AttributeReportingStatus",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_ATTRIBUTE_REPORTING_STATUS",
						"_location": "zcl.h",
						"_clusterID": "0x0201",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0000",
						"_name": "LocalTemperature",
						"_type": "int16",
						"_zigbeetype": "ZCL_DATATYPE_INT16",
						"_definition": "ATTRID_THERMOSTAT_LOCAL_TEMPERATURE ",
						"_location": "zcl_hvac.h",
						"_clusterID": "0x0201",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "true",
						"_required": "true"
					},
					{
						"_id": "0x0001",
						"_name": "OutdoorTemperature",
						"_type": "int16",
						"_zigbeetype": "ZCL_DATATYPE_INT16",
						"_definition": "ATTRID_THERMOSTAT_OUTDOOR_TEMPERATURE ",
						"_location": "zcl_hvac.h",
						"_clusterID": "0x0201",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0002",
						"_name": "Occupancy",
						"_type": "map8",
						"_zigbeetype": "ZCL_DATATYPE_BITMAP8",
						"_definition": "ATTRID_THERMOSTAT_OCCUPANCY ",
						"_location": "zcl_hvac.h",
						"_clusterID": "0x0201",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0003",
						"_name": "AbsMinHeatSetpointLimit",
						"_type": "int16",
						"_zigbeetype": "ZCL_DATATYPE_INT16",
						"_definition": "ATTRID_THERMOSTAT_ABS_MIN_HEAT_SETPOINT_LIMIT ",
						"_location": "zcl_hvac.h",
						"_clusterID": "0x0201",
						"_default": "700",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0004",
						"_name": "AbsMaxHeatSetpointLimit",
						"_type": "int16",
						"_zigbeetype": "ZCL_DATATYPE_INT16",
						"_definition": "ATTRID_THERMOSTAT_ABS_MAX_HEAT_SETPOINT_LIMIT ",
						"_location": "zcl_hvac.h",
						"_clusterID": "0x0201",
						"_default": "3000",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0005",
						"_name": "AbsMinCoolSetpointLimit",
						"_type": "int16",
						"_zigbeetype": "ZCL_DATATYPE_INT16",
						"_definition": "ATTRID_THERMOSTAT_ABS_MIN_COOL_SETPOINT_LIMIT ",
						"_location": "zcl_hvac.h",
						"_clusterID": "0x0201",
						"_default": "1600",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0006",
						"_name": "AbsMaxCoolSetpointLimit",
						"_type": "int16",
						"_zigbeetype": "ZCL_DATATYPE_INT16",
						"_definition": "ATTRID_THERMOSTAT_ABS_MAX_COOL_SETPOINT_LIMIT ",
						"_location": "zcl_hvac.h",
						"_clusterID": "0x0201",
						"_default": "3200",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0007",
						"_name": "PICoolingDemand",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_THERMOSTAT_PI_COOLING_DEMAND ",
						"_location": "zcl_hvac.h",
						"_clusterID": "0x0201",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x0008",
						"_name": "PIHeatingDemand",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_THERMOSTAT_PI_HEATING_DEMAND ",
						"_location": "zcl_hvac.h",
						"_clusterID": "0x0201",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x0009",
						"_name": "HVACSystemTypeConfiguration",
						"_type": "map8",
						"_zigbeetype": "ZCL_DATATYPE_BITMAP8",
						"_definition": "ATTRID_THERMOSTAT_HVAC_SYSTEM_TYPE_CONFIGURATION ",
						"_location": "zcl_hvac.h",
						"_clusterID": "0x0201",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0010",
						"_name": "LocalTemperatureCalibration",
						"_type": "int8",
						"_zigbeetype": "ZCL_DATATYPE_INT8",
						"_definition": "ATTRID_THERMOSTAT_LOCAL_TEMPERATURE_CALIBRATION ",
						"_location": "zcl_hvac.h",
						"_clusterID": "0x0201",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0011",
						"_name": "OccupiedCoolingSetpoint",
						"_type": "int16",
						"_zigbeetype": "ZCL_DATATYPE_INT16",
						"_definition": "ATTRID_THERMOSTAT_OCCUPIED_COOLING_SETPOINT ",
						"_location": "zcl_hvac.h",
						"_clusterID": "0x0201",
						"_default": "2600",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0012",
						"_name": "OccupiedHeatingSetpoint",
						"_type": "int16",
						"_zigbeetype": "ZCL_DATATYPE_INT16",
						"_definition": "ATTRID_THERMOSTAT_OCCUPIED_HEATING_SETPOINT ",
						"_location": "zcl_hvac.h",
						"_clusterID": "0x0201",
						"_default": "2000",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0013",
						"_name": "UnoccupiedCoolingSetpoint",
						"_type": "int16",
						"_zigbeetype": "ZCL_DATATYPE_INT16",
						"_definition": "ATTRID_THERMOSTAT_UNOCCUPIED_COOLING_SETPOINT ",
						"_location": "zcl_hvac.h",
						"_clusterID": "0x0201",
						"_default": "2600",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0014",
						"_name": "UnoccupiedHeatingSetpoint",
						"_type": "int16",
						"_zigbeetype": "ZCL_DATATYPE_INT16",
						"_definition": "ATTRID_THERMOSTAT_UNOCCUPIED_HEATING_SETPOINT ",
						"_location": "zcl_hvac.h",
						"_clusterID": "0x0201",
						"_default": "2000",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0015",
						"_name": "MinHeatSetpointLimit",
						"_type": "int16",
						"_zigbeetype": "ZCL_DATATYPE_INT16",
						"_definition": "ATTRID_THERMOSTAT_MIN_HEAT_SETPOINT_LIMIT ",
						"_location": "zcl_hvac.h",
						"_clusterID": "0x0201",
						"_default": "700",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0016",
						"_name": "MaxHeatSetpointLimit",
						"_type": "int16",
						"_zigbeetype": "ZCL_DATATYPE_INT16",
						"_definition": "ATTRID_THERMOSTAT_MAX_HEAT_SETPOINT_LIMIT ",
						"_location": "zcl_hvac.h",
						"_clusterID": "0x0201",
						"_default": "3000",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0017",
						"_name": "MinCoolSetpointLimit",
						"_type": "int16",
						"_zigbeetype": "ZCL_DATATYPE_INT16",
						"_definition": "ATTRID_THERMOSTAT_MIN_COOL_SETPOINT_LIMIT ",
						"_location": "zcl_hvac.h",
						"_clusterID": "0x0201",
						"_default": "1600",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0018",
						"_name": "MaxCoolSetpointLimit",
						"_type": "int16",
						"_zigbeetype": "ZCL_DATATYPE_INT16",
						"_definition": "ATTRID_THERMOSTAT_MAX_COOL_SETPOINT_LIMIT ",
						"_location": "zcl_hvac.h",
						"_clusterID": "0x0201",
						"_default": "3200",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0019",
						"_name": "MinSetpointDeadBand",
						"_type": "int8",
						"_zigbeetype": "ZCL_DATATYPE_INT8",
						"_definition": "ATTRID_THERMOSTAT_MIN_SETPOINT_DEAD_BAND ",
						"_location": "zcl_hvac.h",
						"_clusterID": "0x0201",
						"_default": "25",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x001a",
						"_name": "RemoteSensing",
						"_type": "map8",
						"_zigbeetype": "ZCL_DATATYPE_BITMAP8",
						"_definition": "ATTRID_THERMOSTAT_REMOTE_SENSING ",
						"_location": "zcl_hvac.h",
						"_clusterID": "0x0201",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x001b",
						"_name": "ControlSequenceOfOperation",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_THERMOSTAT_CONTROL_SEQUENCE_OF_OPERATION ",
						"_location": "zcl_hvac.h",
						"_clusterID": "0x0201",
						"_default": "4",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0x001c",
						"_name": "SystemMode",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_THERMOSTAT_SYSTEM_MODE ",
						"_location": "zcl_hvac.h",
						"_clusterID": "0x0201",
						"_default": "1",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0x001d",
						"_name": "AlarmMask",
						"_type": "map8",
						"_zigbeetype": "ZCL_DATATYPE_BITMAP8",
						"_definition": "ATTRID_THERMOSTAT_ALARM_MASK ",
						"_location": "zcl_hvac.h",
						"_clusterID": "0x0201",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x001e",
						"_name": "ThermostatRunningMode",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_THERMOSTAT_THERMOSTAT_RUNNING_MODE ",
						"_location": "zcl_hvac.h",
						"_clusterID": "0x0201",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0020",
						"_name": "StartOfWeek",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_THERMOSTAT_START_OF_WEEK ",
						"_location": "zcl_hvac.h",
						"_clusterID": "0x0201",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0021",
						"_name": "NumberOfWeeklyTransitions",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_THERMOSTAT_NUMBER_OF_WEEKLY_TRANSITIONS ",
						"_location": "zcl_hvac.h",
						"_clusterID": "0x0201",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0022",
						"_name": "NumberOfDailyTransitions",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_THERMOSTAT_NUMBER_OF_DAILY_TRANSITIONS ",
						"_location": "zcl_hvac.h",
						"_clusterID": "0x0201",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0023",
						"_name": "TemperatureSetpointHold",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_THERMOSTAT_TEMPERATURE_SETPOINT_HOLD ",
						"_location": "zcl_hvac.h",
						"_clusterID": "0x0201",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0024",
						"_name": "TemperatureSetpointHoldDuration",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_THERMOSTAT_TEMPERATURE_SETPOINT_HOLD_DURATION ",
						"_location": "zcl_hvac.h",
						"_clusterID": "0x0201",
						"_default": "65535",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0025",
						"_name": "ThermostatProgrammingOperationMode",
						"_type": "map8",
						"_zigbeetype": "ZCL_DATATYPE_BITMAP8",
						"_definition": "ATTRID_THERMOSTAT_THERMOSTAT_PROGRAMMING_OPERATION_MODE ",
						"_location": "zcl_hvac.h",
						"_clusterID": "0x0201",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x0029",
						"_name": "ThermostatRunningState",
						"_type": "map16",
						"_zigbeetype": "ZCL_DATATYPE_BITMAP16",
						"_definition": "ATTRID_THERMOSTAT_THERMOSTAT_RUNNING_STATE ",
						"_location": "zcl_hvac.h",
						"_clusterID": "0x0201",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0030",
						"_name": "SetpointChangeSource",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_THERMOSTAT_SETPOINT_CHANGE_SOURCE ",
						"_location": "zcl_hvac.h",
						"_clusterID": "0x0201",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0031",
						"_name": "SetpointChangeAmount",
						"_type": "int16",
						"_zigbeetype": "ZCL_DATATYPE_INT16",
						"_definition": "ATTRID_THERMOSTAT_SETPOINT_CHANGE_AMOUNT ",
						"_location": "zcl_hvac.h",
						"_clusterID": "0x0201",
						"_default": "32768",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0032",
						"_name": "SetpointChangeSourceTimestamp",
						"_type": "UTC",
						"_zigbeetype": "ZCL_DATATYPE_UTC",
						"_definition": "ATTRID_THERMOSTAT_SETPOINT_CHANGE_SOURCE_TIMESTAMP ",
						"_location": "zcl_hvac.h",
						"_clusterID": "0x0201",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0034",
						"_name": "OccupiedSetback",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_THERMOSTAT_OCCUPIED_SETBACK ",
						"_location": "zcl_hvac.h",
						"_clusterID": "0x0201",
						"_default": "255",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0035",
						"_name": "OccupiedSetbackMin",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_THERMOSTAT_OCCUPIED_SETBACK_MIN ",
						"_location": "zcl_hvac.h",
						"_clusterID": "0x0201",
						"_default": "255",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0036",
						"_name": "OccupiedSetbackMax",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_THERMOSTAT_OCCUPIED_SETBACK_MAX ",
						"_location": "zcl_hvac.h",
						"_clusterID": "0x0201",
						"_default": "255",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0037",
						"_name": "UnoccupiedSetback",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_THERMOSTAT_UNOCCUPIED_SETBACK ",
						"_location": "zcl_hvac.h",
						"_clusterID": "0x0201",
						"_default": "255",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0038",
						"_name": "UnoccupiedSetbackMin",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_THERMOSTAT_UNOCCUPIED_SETBACK_MIN ",
						"_location": "zcl_hvac.h",
						"_clusterID": "0x0201",
						"_default": "255",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0039",
						"_name": "UnoccupiedSetbackMax",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_THERMOSTAT_UNOCCUPIED_SETBACK_MAX ",
						"_location": "zcl_hvac.h",
						"_clusterID": "0x0201",
						"_default": "255",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x003a",
						"_name": "EmergencyHeatDelta",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_THERMOSTAT_EMERGENCY_HEAT_DELTA ",
						"_location": "zcl_hvac.h",
						"_clusterID": "0x0201",
						"_default": "255",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0040",
						"_name": "ACType",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_THERMOSTAT_AC_TYPE ",
						"_location": "zcl_hvac.h",
						"_clusterID": "0x0201",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0041",
						"_name": "ACCapacity",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_THERMOSTAT_AC_CAPACITY ",
						"_location": "zcl_hvac.h",
						"_clusterID": "0x0201",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0042",
						"_name": "ACRefrigerantType",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_THERMOSTAT_AC_REFRIGERANT_TYPE ",
						"_location": "zcl_hvac.h",
						"_clusterID": "0x0201",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0043",
						"_name": "ACCompressorType",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_THERMOSTAT_AC_COMPRESSOR_TYPE ",
						"_location": "zcl_hvac.h",
						"_clusterID": "0x0201",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0044",
						"_name": "ACErrorCode",
						"_type": "map32",
						"_zigbeetype": "ZCL_DATATYPE_BITMAP32",
						"_definition": "ATTRID_THERMOSTAT_AC_ERROR_CODE ",
						"_location": "zcl_hvac.h",
						"_clusterID": "0x0201",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0045",
						"_name": "ACLouverPosition",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_THERMOSTAT_AC_LOUVER_POSITION ",
						"_location": "zcl_hvac.h",
						"_clusterID": "0x0201",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0046",
						"_name": "ACCoilTemperature",
						"_type": "int16",
						"_zigbeetype": "ZCL_DATATYPE_INT16",
						"_definition": "ATTRID_THERMOSTAT_AC_COIL_TEMPERATURE ",
						"_location": "zcl_hvac.h",
						"_clusterID": "0x0201",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0047",
						"_name": "ACCapacityFormat",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_THERMOSTAT_AC_CAPACITY_FORMAT ",
						"_location": "zcl_hvac.h",
						"_clusterID": "0x0201",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					}
				],
				"command": [
					{
						"_id": "0x00",
						"_name": "SetpointRaiseOrLower",
						"_definition": "COMMAND_THERMOSTAT_SETPOINT_RAISE_OR_LOWER",
						"_location": "zcl_hvac.h",
						"_clusterID": "0x0201",
						"_required": "true"
					},
					{
						"_id": "0x01",
						"_name": "SetWeeklySchedule",
						"_definition": "COMMAND_THERMOSTAT_SET_WEEKLY_SCHEDULE",
						"_location": "zcl_hvac.h",
						"_clusterID": "0x0201",
						"_required": "false"
					},
					{
						"_id": "0x02",
						"_name": "GetWeeklySchedule",
						"_definition": "COMMAND_THERMOSTAT_GET_WEEKLY_SCHEDULE",
						"_location": "zcl_hvac.h",
						"_clusterID": "0x0201",
						"_required": "false"
					},
					{
						"_id": "0x03",
						"_name": "ClearWeeklySchedule",
						"_definition": "COMMAND_THERMOSTAT_CLEAR_WEEKLY_SCHEDULE",
						"_location": "zcl_hvac.h",
						"_clusterID": "0x0201",
						"_required": "false"
					},
					{
						"_id": "0x04",
						"_name": "GetRelayStatusLog",
						"_definition": "COMMAND_THERMOSTAT_GET_RELAY_STATUS_LOG",
						"_location": "zcl_hvac.h",
						"_clusterID": "0x0201",
						"_required": "false"
					}
				]
			},
			"client": {
				"attribute": [
					{
						"_id": "0xfffd",
						"_name": "ClusterRevision",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_CLUSTER_REVISION",
						"_location": "zcl.h",
						"_clusterID": "0x0201",
						"_default": "1",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0xfffe",
						"_name": "AttributeReportingStatus",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_ATTRIBUTE_REPORTING_STATUS",
						"_location": "zcl.h",
						"_clusterID": "0x0201",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					}
				],
				"command": [
					{
						"_id": "0x00",
						"_name": "GetWeeklyScheduleResponse",
						"_definition": "COMMAND_THERMOSTAT_GET_WEEKLY_SCHEDULE_RESPONSE",
						"_location": "zcl_hvac.h",
						"_clusterID": "0x0201",
						"_required": "false"
					},
					{
						"_id": "0x01",
						"_name": "GetRelayStatusLogResponse",
						"_definition": "COMMAND_THERMOSTAT_GET_RELAY_STATUS_LOG_RESPONSE",
						"_location": "zcl_hvac.h",
						"_clusterID": "0x0201",
						"_required": "false"
					}
				]
			},
			"appcallback": {
				"callbacks": [
					{
						"_name": "HVAC_SetpointRaiseLower",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclCmdThermostatSetpointRaiseLowerPayload_t *pCmd )",
						"comments": [
							{
								"_comment": "// This callback is called to process a Setpoint Raise/Lower command"
							}
						]
					},
					{
						"_name": "HVAC_SetWeeklySchedule",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclThermostatWeeklySchedule_t *pCmd )",
						"comments": [
							{
								"_comment": "// This callback is called to process a Set Weekly Schedule command"
							}
						]
					},
					{
						"_name": "HVAC_GetWeeklySchedule",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclThermostatGetWeeklySchedule_t *pCmd )",
						"comments": [
							{
								"_comment": "// This callback is called to process a Get Weekly Schedule command"
							}
						]
					},
					{
						"_name": "HVAC_ClearWeeklySchedule",
						"_returnType": "ZStatus_t",
						"_arguments": "( void )",
						"comments": [
							{
								"_comment": "// This callback is called to process a Clear Weekly Schedule command"
							}
						]
					},
					{
						"_name": "HVAC_GetRelayStatusLog",
						"_returnType": "ZStatus_t",
						"_arguments": "( void )",
						"comments": [
							{
								"_comment": "// This callback is called to process a Get Relay Status Log command"
							}
						]
					},
					{
						"_name": "HVAC_GetWeeklyScheduleRsp",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclThermostatWeeklySchedule_t *pCmd )",
						"comments": [
							{
								"_comment": "// This callback is called to process a Get Weekly Schedule Response command"
							}
						]
					},
					{
						"_name": "HVAC_GetRelayStatusLogRsp",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclThermostatGetRelayStatusLogRsp_t *pCmd )",
						"comments": [
							{
								"_comment": "// This callback is called to process a Get Relay Status Log Response command"
							}
						]
					}
				],
				"_name": "zclHVAC_AppCallbacks_t",
				"_register": "zclHVAC_RegisterCmdCallbacks"
			},
			"_id": "0x0201",
			"_name": "Thermostat",
			"_definition": "ZCL_CLUSTER_ID_HVAC_THERMOSTAT",
			"_primary_transaction": "2",
			"_class": "hvac",
			"_flag": "ZCL_HVAC_CLUSTER",
			"_location": "zcl_hvac.h"
		},
		{
			"server": {
				"attribute": [
					{
						"_id": "0xfffd",
						"_name": "ClusterRevision",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_CLUSTER_REVISION",
						"_location": "zcl.h",
						"_clusterID": "0x0204",
						"_default": "1",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0xfffe",
						"_name": "AttributeReportingStatus",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_ATTRIBUTE_REPORTING_STATUS",
						"_location": "zcl.h",
						"_clusterID": "0x0204",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0000",
						"_name": "TemperatureDisplayMode",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_THERMOSTAT_USER_INTERFACE_CONFIGURATION_TEMPERATURE_DISPLAY_MODE ",
						"_location": "zcl_hvac.h",
						"_clusterID": "0x0204",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0x0001",
						"_name": "KeypadLockout",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_THERMOSTAT_USER_INTERFACE_CONFIGURATION_KEYPAD_LOCKOUT ",
						"_location": "zcl_hvac.h",
						"_clusterID": "0x0204",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0x0002",
						"_name": "ScheduleProgrammingVisibility",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_THERMOSTAT_USER_INTERFACE_CONFIGURATION_SCHEDULE_PROGRAMMING_VISIBILITY ",
						"_location": "zcl_hvac.h",
						"_clusterID": "0x0204",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					}
				]
			},
			"client": {
				"attribute": [
					{
						"_id": "0xfffd",
						"_name": "ClusterRevision",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_CLUSTER_REVISION",
						"_location": "zcl.h",
						"_clusterID": "0x0204",
						"_default": "1",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0xfffe",
						"_name": "AttributeReportingStatus",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_ATTRIBUTE_REPORTING_STATUS",
						"_location": "zcl.h",
						"_clusterID": "0x0204",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					}
				],
			},
			"appcallback": {
				"callbacks": [
					{
						"_name": "HVAC_SetpointRaiseLower",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclCmdThermostatSetpointRaiseLowerPayload_t *pCmd )",
						"comments": [
							{
								"_comment": "// This callback is called to process a Setpoint Raise/Lower command"
							}
						]
					},
					{
						"_name": "HVAC_SetWeeklySchedule",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclThermostatWeeklySchedule_t *pCmd )",
						"comments": [
							{
								"_comment": "// This callback is called to process a Set Weekly Schedule command"
							}
						]
					},
					{
						"_name": "HVAC_GetWeeklySchedule",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclThermostatGetWeeklySchedule_t *pCmd )",
						"comments": [
							{
								"_comment": "// This callback is called to process a Get Weekly Schedule command"
							}
						]
					},
					{
						"_name": "HVAC_ClearWeeklySchedule",
						"_returnType": "ZStatus_t",
						"_arguments": "( void )",
						"comments": [
							{
								"_comment": "// This callback is called to process a Clear Weekly Schedule command"
							}
						]
					},
					{
						"_name": "HVAC_GetRelayStatusLog",
						"_returnType": "ZStatus_t",
						"_arguments": "( void )",
						"comments": [
							{
								"_comment": "// This callback is called to process a Get Relay Status Log command"
							}
						]
					},
					{
						"_name": "HVAC_GetWeeklyScheduleRsp",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclThermostatWeeklySchedule_t *pCmd )",
						"comments": [
							{
								"_comment": "// This callback is called to process a Get Weekly Schedule Response command"
							}
						]
					},
					{
						"_name": "HVAC_GetRelayStatusLogRsp",
						"_returnType": "ZStatus_t",
						"_arguments": "( zclThermostatGetRelayStatusLogRsp_t *pCmd )",
						"comments": [
							{
								"_comment": "// This callback is called to process a Get Relay Status Log Response command"
							}
						]
					}
				],
				"_name": "zclHVAC_AppCallbacks_t",
				"_register": "zclHVAC_RegisterCmdCallbacks"
			},
			"_id": "0x0204",
			"_name": "ThermostatUserInterfaceConfiguration",
			"_definition": "ZCL_CLUSTER_ID_HVAC_USER_INTERFACE_CONFIG",
			"_primary_transaction": "1",
			"_class": "hvac",
			"_flag": "ZCL_HVAC_CLUSTER",
			"_location": "zcl_hvac.h"
		},
		{
			"server": {
				"attribute": [
					{
						"_id": "0xfffd",
						"_name": "ClusterRevision",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_CLUSTER_REVISION",
						"_location": "zcl.h",
						"_clusterID": "0x000a",
						"_default": "1",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0xfffe",
						"_name": "AttributeReportingStatus",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_ATTRIBUTE_REPORTING_STATUS",
						"_location": "zcl.h",
						"_clusterID": "0x000a",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0000",
						"_name": "Time",
						"_type": "UTC",
						"_zigbeetype": "ZCL_DATATYPE_UTC",
						"_definition": "ATTRID_TIME_TIME ",
						"_location": "zcl_general.h",
						"_clusterID": "0x000a",
						"_default": "4294967295",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0x0001",
						"_name": "TimeStatus",
						"_type": "map8",
						"_zigbeetype": "ZCL_DATATYPE_BITMAP8",
						"_definition": "ATTRID_TIME_TIME_STATUS ",
						"_location": "zcl_general.h",
						"_clusterID": "0x000a",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0x0002",
						"_name": "TimeZone",
						"_type": "int32",
						"_zigbeetype": "ZCL_DATATYPE_INT32",
						"_definition": "ATTRID_TIME_TIME_ZONE ",
						"_location": "zcl_general.h",
						"_clusterID": "0x000a",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0003",
						"_name": "DstStart",
						"_type": "uint32",
						"_zigbeetype": "ZCL_DATATYPE_UINT32",
						"_definition": "ATTRID_TIME_DST_START ",
						"_location": "zcl_general.h",
						"_clusterID": "0x000a",
						"_default": "4294967295",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0004",
						"_name": "DstEnd",
						"_type": "uint32",
						"_zigbeetype": "ZCL_DATATYPE_UINT32",
						"_definition": "ATTRID_TIME_DST_END ",
						"_location": "zcl_general.h",
						"_clusterID": "0x000a",
						"_default": "4294967295",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0005",
						"_name": "DstShift",
						"_type": "int32",
						"_zigbeetype": "ZCL_DATATYPE_INT32",
						"_definition": "ATTRID_TIME_DST_SHIFT ",
						"_location": "zcl_general.h",
						"_clusterID": "0x000a",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0006",
						"_name": "StandardTime",
						"_type": "uint32",
						"_zigbeetype": "ZCL_DATATYPE_UINT32",
						"_definition": "ATTRID_TIME_STANDARD_TIME ",
						"_location": "zcl_general.h",
						"_clusterID": "0x000a",
						"_default": "4294967295",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0007",
						"_name": "LocalTime",
						"_type": "uint32",
						"_zigbeetype": "ZCL_DATATYPE_UINT32",
						"_definition": "ATTRID_TIME_LOCAL_TIME ",
						"_location": "zcl_general.h",
						"_clusterID": "0x000a",
						"_default": "4294967295",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0008",
						"_name": "LastSetTime",
						"_type": "UTC",
						"_zigbeetype": "ZCL_DATATYPE_UTC",
						"_definition": "ATTRID_TIME_LAST_SET_TIME ",
						"_location": "zcl_general.h",
						"_clusterID": "0x000a",
						"_default": "4294967295",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0009",
						"_name": "ValidUntilTime",
						"_type": "UTC",
						"_zigbeetype": "ZCL_DATATYPE_UTC",
						"_definition": "ATTRID_TIME_VALID_UNTIL_TIME ",
						"_location": "zcl_general.h",
						"_clusterID": "0x000a",
						"_default": "4294967295",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					}
				]
			},
			"client": {
				"attribute": [
					{
						"_id": "0xfffd",
						"_name": "ClusterRevision",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_CLUSTER_REVISION",
						"_location": "zcl.h",
						"_clusterID": "0x000a",
						"_default": "1",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0xfffe",
						"_name": "AttributeReportingStatus",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_ATTRIBUTE_REPORTING_STATUS",
						"_location": "zcl.h",
						"_clusterID": "0x000a",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					}
				],
			},
			"appcallback": {
				"callbacks": [
					{
						"_name": "BasicReset",
						"_returnType": "void",
						"_arguments": "( void )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Reset to Factory Defaults"
							},
							{
								"_comment": "/// command. On receipt of this command, the device resets all the attributes"
							},
							{
								"_comment": "/// of all its clusters to their factory defaults."
							}
						]
					},
					{
						"_name": "Identify",
						"_returnType": "void",
						"_arguments": "( zclIdentify_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Identify command."
							}
						]
					},
					{
						"_name": "IdentifyQuery",
						"_returnType": "void",
						"_arguments": "( zclIdentifyQuery_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Identify Query command."
							}
						]
					},
					{
						"_name": "IdentifyQueryRsp",
						"_returnType": "void",
						"_arguments": "( zclIdentifyQueryRsp_t *pRsp )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Identify Query Response command."
							}
						]
					},
					{
						"_name": "IdentifyTriggerEffect",
						"_returnType": "void",
						"_arguments": "( zclIdentifyTriggerEffect_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Identify Trigger Effect command."
							}
						]
					},
					{
						"_name": "#ifdef ZCL_ON_OFF",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "OnOff",
						"_returnType": "void",
						"_arguments": "( uint8_t cmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming On, Off or Toggle command."
							}
						]
					},
					{
						"_name": "OnOff_OffWithEffect",
						"_returnType": "void",
						"_arguments": "( zclOffWithEffect_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Off with Effect"
							}
						]
					},
					{
						"_name": "OnOff_OnWithRecallGlobalScene",
						"_returnType": "void",
						"_arguments": "( void )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming On with Recall Global Scene command."
							}
						]
					},
					{
						"_name": "OnOff_OnWithTimedOff",
						"_returnType": "void",
						"_arguments": "( zclOnWithTimedOff_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming On with Timed Off."
							}
						]
					},
					{
						"_name": "#endif",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "#ifdef ZCL_LEVEL_CTRL",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "LevelControlMoveToLevel",
						"_returnType": "void",
						"_arguments": "( zclLCMoveToLevel_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Level Control - Move to Level command"
							}
						]
					},
					{
						"_name": "LevelControlMove",
						"_returnType": "void",
						"_arguments": "( zclLCMove_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Level Control - Move command"
							}
						]
					},
					{
						"_name": "LevelControlStep",
						"_returnType": "void",
						"_arguments": "( zclLCStep_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Level Control - Step command"
							}
						]
					},
					{
						"_name": "LevelControlStop",
						"_returnType": "void",
						"_arguments": "( zclLCStop_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Level Control - Stop command"
							}
						]
					},
					{
						"_name": "LevelControlMoveFreq",
						"_returnType": "void",
						"_arguments": "( zclLCMoveFreq_t *pCmd)",
						"comments": [
							{
								"_comment": "/// This callback is called to process a Level Control - Move to Closest Frequency command"
							}
						]
					},
					{
						"_name": "#endif",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "#ifdef ZCL_GROUPS",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "GroupRsp",
						"_returnType": "void",
						"_arguments": "( zclGroupRsp_t *pRsp )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an received Group Response message."
							},
							{
								"_comment": "/// This means that this app sent the request message."
							}
						]
					},
					{
						"_name": "#endif",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "#ifdef ZCL_SCENES",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "SceneStoreReq",
						"_returnType": "uint8_t",
						"_arguments": "( zclSceneReq_t *pReq )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Scene Store request."
							},
							{
								"_comment": "/// The app will fill in the 'extField' with what is needed to restore its"
							},
							{
								"_comment": "/// current settings. Before overwriting the content of the scene,"
							},
							{
								"_comment": "/// App needs to verify if there was a change with the previous configuration."
							},
							{
								"_comment": "/// App function returns TRUE if there was a change, otherwise returns FALSE."
							}
						]
					},
					{
						"_name": "SceneRecallReq",
						"_returnType": "void",
						"_arguments": "( zclSceneReq_t *pReq )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Scene Recall request"
							},
							{
								"_comment": "/// The app will use what's in the 'extField' to restore to these settings."
							}
						]
					},
					{
						"_name": "SceneRsp",
						"_returnType": "void",
						"_arguments": "( zclSceneRsp_t *pRsp )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Scene responses. This means"
							},
							{
								"_comment": "/// that this app sent the request for this response."
							}
						]
					},
					{
						"_name": "#endif",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "#ifdef ZCL_ALARMS",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "Alarm",
						"_returnType": "void",
						"_arguments": "( uint8_t direction, zclAlarm_t *pAlarm )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Alarm request or response command."
							}
						]
					},
					{
						"_name": "#endif",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "#ifdef SE_UK_EXT",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "GetEventLog",
						"_returnType": "void",
						"_arguments": "( uint8_t srcEP, afAddrType_t *srcAddr, zclGetEventLog_t *pEventLog, uint8_t seqNum )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Alarm Get Event Log command."
							}
						]
					},
					{
						"_name": "PublishEventLog",
						"_returnType": "void",
						"_arguments": "( afAddrType_t *srcAddr, zclPublishEventLog_t *pEventLog )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Alarm Publish Event Log command."
							}
						]
					},
					{
						"_name": "#endif",
						"_returnType": "None",
						"_arguments": "None"
					},
					{
						"_name": "Location",
						"_returnType": "void",
						"_arguments": "( zclLocation_t *pCmd )",
						"comments": [
							{
								"_comment": "/// This callback is called to to process an incoming RSSI Location command."
							}
						]
					},
					{
						"_name": "LocationRsp",
						"_returnType": "void",
						"_arguments": "( zclLocationRsp_t *pRsp )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming RSSI Location response command."
							},
							{
								"_comment": "/// This means  that this app sent the request for this response."
							}
						]
					}
				],
				"_name": "zclGeneral_AppCallbacks_t",
				"_register": "zclGeneral_RegisterCmdCallbacks"
			},
			"_id": "0x000a",
			"_name": "Time",
			"_definition": "ZCL_CLUSTER_ID_GENERAL_TIME",
			"_primary_transaction": "0",
			"_class": "general",
			"_flag": "ZCL_TIME",
			"_location": "zcl_general.h"
		},
		{
			"server": {
				"attribute": [
					{
						"_id": "0xfffd",
						"_name": "ClusterRevision",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_CLUSTER_REVISION",
						"_location": "zcl.h",
						"_clusterID": "0x0405",
						"_default": "1",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0xfffe",
						"_name": "AttributeReportingStatus",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_ATTRIBUTE_REPORTING_STATUS",
						"_location": "zcl.h",
						"_clusterID": "0x0405",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0000",
						"_name": "MeasuredValue",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_RELATIVITY_HUMIDITY_MEASURED_VALUE ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0405",
						"_default": "65535",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "true",
						"_required": "true"
					},
					{
						"_id": "0x0001",
						"_name": "MinMeasuredValue",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_RELATIVITY_HUMIDITY_MIN_MEASURED_VALUE ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0405",
						"_default": "65535",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0x0002",
						"_name": "MaxMeasuredValue",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_RELATIVITY_HUMIDITY_MAX_MEASURED_VALUE ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0405",
						"_default": "65535",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0x0003",
						"_name": "Tolerance",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_RELATIVITY_HUMIDITY_TOLERANCE ",
						"_location": "zcl_ms.h",
						"_clusterID": "0x0405",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					}
				]
			},
			"client": {
				"attribute": [
					{
						"_id": "0xfffd",
						"_name": "ClusterRevision",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_CLUSTER_REVISION",
						"_location": "zcl.h",
						"_clusterID": "0x0405",
						"_default": "1",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0xfffe",
						"_name": "AttributeReportingStatus",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_ATTRIBUTE_REPORTING_STATUS",
						"_location": "zcl.h",
						"_clusterID": "0x0405",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					}
				],
			},
			"appcallback": {
				"callbacks": [
					{
						"_name": "MSPlaceHolder",
						"_returnType": "void",
						"_arguments": "( void )"
					}
				],
				"_name": "zclMS_AppCallbacks_t",
				"_register": "zclMS_RegisterCmdCallbacks"
			},
			"_id": "0x0405",
			"_name": "RelativityHumidity",
			"_definition": "ZCL_CLUSTER_ID_MS_RELATIVE_HUMIDITY",
			"_primary_transaction": "2",
			"_class": "ms",
			"_flag": "ZCL_MS",
			"_location": "zcl_ms.h"
		},
		{
			"server": {
				"attribute": [
					{
						"_id": "0xfffd",
						"_name": "ClusterRevision",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_CLUSTER_REVISION",
						"_location": "zcl.h",
						"_clusterID": "0x0102",
						"_default": "1",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0xfffe",
						"_name": "AttributeReportingStatus",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_ATTRIBUTE_REPORTING_STATUS",
						"_location": "zcl.h",
						"_clusterID": "0x0102",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0000",
						"_name": "WindowCoveringType",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_WINDOW_COVERING_WINDOW_COVERING_TYPE ",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0102",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0x0001",
						"_name": "PhysicalClosedLimitLift",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_WINDOW_COVERING_PHYSICAL_CLOSED_LIMIT_LIFT ",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0102",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0002",
						"_name": "PhysicalClosedLimitTilt",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_WINDOW_COVERING_PHYSICAL_CLOSED_LIMIT_TILT ",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0102",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0003",
						"_name": "CurrentPositionLift",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_WINDOW_COVERING_CURRENT_POSITION_LIFT ",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0102",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0004",
						"_name": "CurrentPositionTilt",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_WINDOW_COVERING_CURRENT_POSITION_TILT ",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0102",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0005",
						"_name": "NumberOfActuationsLift",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_WINDOW_COVERING_NUMBER_OF_ACTUATIONS_LIFT ",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0102",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0006",
						"_name": "NumberOfActuationsTilt",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_WINDOW_COVERING_NUMBER_OF_ACTUATIONS_TILT ",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0102",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0007",
						"_name": "ConfigOrStatus",
						"_type": "map8",
						"_zigbeetype": "ZCL_DATATYPE_BITMAP8",
						"_definition": "ATTRID_WINDOW_COVERING_CONFIG_OR_STATUS ",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0102",
						"_default": "3",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0x0008",
						"_name": "CurrentPositionLiftPercentage",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_WINDOW_COVERING_CURRENT_POSITION_LIFT_PERCENTAGE ",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0102",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x0009",
						"_name": "CurrentPositionTiltPercentage",
						"_type": "uint8",
						"_zigbeetype": "ZCL_DATATYPE_UINT8",
						"_definition": "ATTRID_WINDOW_COVERING_CURRENT_POSITION_TILT_PERCENTAGE ",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0102",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "true",
						"_required": "false"
					},
					{
						"_id": "0x0010",
						"_name": "InstalledOpenLimitLift",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_WINDOW_COVERING_INSTALLED_OPEN_LIMIT_LIFT ",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0102",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0011",
						"_name": "InstalledClosedLimitLift",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_WINDOW_COVERING_INSTALLED_CLOSED_LIMIT_LIFT ",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0102",
						"_default": "65535",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0012",
						"_name": "InstalledOpenLimitTilt",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_WINDOW_COVERING_INSTALLED_OPEN_LIMIT_TILT ",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0102",
						"_default": "0",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0013",
						"_name": "InstalledClosedLimitTilt",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_WINDOW_COVERING_INSTALLED_CLOSED_LIMIT_TILT ",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0102",
						"_default": "65535",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0014",
						"_name": "VelocityLift",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_WINDOW_COVERING_VELOCITY_LIFT ",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0102",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0015",
						"_name": "AccelerationTimeLift",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_WINDOW_COVERING_ACCELERATION_TIME_LIFT ",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0102",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0016",
						"_name": "DecelerationTimeLift",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_WINDOW_COVERING_DECELERATION_TIME_LIFT ",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0102",
						"_default": "0",
						"_readable": "true",
						"_writable": "true",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0017",
						"_name": "Mode",
						"_type": "map8",
						"_zigbeetype": "ZCL_DATATYPE_BITMAP8",
						"_definition": "ATTRID_WINDOW_COVERING_MODE ",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0102",
						"_default": "4",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0x0018",
						"_name": "IntermediateSetpointsLift",
						"_type": "uint8[]",
						"_zigbeetype": "ZCL_DATATYPE_OCTET_STR",
						"_definition": "ATTRID_WINDOW_COVERING_INTERMEDIATE_SETPOINTS_LIFT ",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0102",
						"_default": "{ 1, 0x0000}",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					},
					{
						"_id": "0x0019",
						"_name": "IntermediateSetpointsTilt",
						"_type": "uint8[]",
						"_zigbeetype": "ZCL_DATATYPE_OCTET_STR",
						"_definition": "ATTRID_WINDOW_COVERING_INTERMEDIATE_SETPOINTS_TILT ",
						"_location": "zcl_closures.h",
						"_clusterID": "0x0102",
						"_default": "{ 1, 0x0000}",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					}
				],
				"command": [
					{
						"_id": "0x00",
						"_name": "UpOrOpen",
						"_definition": "COMMAND_WINDOW_COVERING_UP_OR_OPEN",
						"_location": "zcl_closures.h",
						"_callback": "WindowCoveringUpOpen",
						"_clusterID": "0x0102",
						"_required": "true"
					},
					{
						"_id": "0x01",
						"_name": "DownOrClose",
						"_definition": "COMMAND_WINDOW_COVERING_DOWN_OR_CLOSE",
						"_location": "zcl_closures.h",
						"_callback": "WindowCoveringDownClose",
						"_clusterID": "0x0102",
						"_required": "true"
					},
					{
						"_id": "0x02",
						"_name": "Stop",
						"_definition": "COMMAND_WINDOW_COVERING_STOP",
						"_location": "zcl_closures.h",
						"_callback": "WindowCoveringStop",
						"_clusterID": "0x0102",
						"_required": "true"
					},
					{
						"_id": "0x04",
						"_name": "GoToLiftValue",
						"_definition": "COMMAND_WINDOW_COVERING_GO_TO_LIFT_VALUE",
						"_location": "zcl_closures.h",
						"_callback": "WindowCoveringGotoLiftValue",
						"_clusterID": "0x0102",
						"_required": "false"
					},
					{
						"_id": "0x05",
						"_name": "GoToLiftPercentage",
						"_definition": "COMMAND_WINDOW_COVERING_GO_TO_LIFT_PERCENTAGE",
						"_location": "zcl_closures.h",
						"_callback": "WindowCoveringGotoLiftPercentage",
						"_clusterID": "0x0102",
						"_required": "false"
					},
					{
						"_id": "0x07",
						"_name": "GoToTiltValue",
						"_definition": "COMMAND_WINDOW_COVERING_GO_TO_TILT_VALUE",
						"_location": "zcl_closures.h",
						"_callback": "WindowCoveringGotoTiltValue",
						"_clusterID": "0x0102",
						"_required": "false"
					},
					{
						"_id": "0x08",
						"_name": "GoToTiltPercentage",
						"_definition": "COMMAND_WINDOW_COVERING_GO_TO_TILT_PERCENTAGE",
						"_location": "zcl_closures.h",
						"_callback": "WindowCoveringGotoTiltPercentage",
						"_clusterID": "0x0102",
						"_required": "false"
					}
				]
			},
			"client": {
				"attribute": [
					{
						"_id": "0xfffd",
						"_name": "ClusterRevision",
						"_type": "uint16",
						"_zigbeetype": "ZCL_DATATYPE_UINT16",
						"_definition": "ATTRID_CLUSTER_REVISION",
						"_location": "zcl.h",
						"_clusterID": "0x0102",
						"_default": "1",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "true"
					},
					{
						"_id": "0xfffe",
						"_name": "AttributeReportingStatus",
						"_type": "enum8",
						"_zigbeetype": "ZCL_DATATYPE_ENUM8",
						"_definition": "ATTRID_ATTRIBUTE_REPORTING_STATUS",
						"_location": "zcl.h",
						"_clusterID": "0x0102",
						"_readable": "true",
						"_writable": "false",
						"_reportable": "false",
						"_required": "false"
					}
				],
			},
			"appcallback": {
				"callbacks": [
					{
						"_name": "WindowCoveringUpOpen",
						"_returnType": "void",
						"_arguments": "( void )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Window Covering cluster basic commands"
							}
						]
					},
					{
						"_name": "WindowCoveringDownClose",
						"_returnType": "void",
						"_arguments": "( void )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Window Covering cluster basic commands"
							}
						]
					},
					{
						"_name": "WindowCoveringStop",
						"_returnType": "void",
						"_arguments": "( void )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Window Covering cluster basic commands"
							}
						]
					},
					{
						"_name": "WindowCoveringGotoLiftValue",
						"_returnType": "bool",
						"_arguments": "( uint16_t value )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Window Covering cluster goto value commands"
							}
						]
					},
					{
						"_name": "WindowCoveringGotoLiftPercentage",
						"_returnType": "bool",
						"_arguments": "( uint8_t percentage )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Window Covering cluster goto percentage commands"
							}
						]
					},
					{
						"_name": "WindowCoveringGotoTiltValue",
						"_returnType": "bool",
						"_arguments": "( uint16_t value )",
						"comments": [
							{
								"_comment": "/// This callback is called to process an incoming Window Covering cluster goto value commands"
							}
						]
					}
				],
				"_name": "zclClosures_WindowCoveringAppCallbacks_t",
				"_register": "zclClosures_RegisterWindowCoveringCmdCallbacks"
			},
			"_id": "0x0102",
			"_name": "WindowCovering",
			"_definition": "ZCL_CLUSTER_ID_CLOSURES_WINDOW_COVERING",
			"_primary_transaction": "1",
			"_class": "closures",
			"_flag": "ZCL_WINDOWCOVERING",
			"_location": "zcl_closures.h"
		}
	]
};

exports = {
	ZCL: ZCL,
};
