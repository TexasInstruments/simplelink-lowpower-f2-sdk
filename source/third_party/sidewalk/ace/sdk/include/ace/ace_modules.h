/*
 * Copyright 2020 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.TXT file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */
#ifndef ACE_MODULES_DOT_H
#define ACE_MODULES_DOT_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Unique IDs for modules to use in ACE.
 * @details Many ACE modules require an unique ID denoting a module to be
 * passed in to their APIs. This header provides a central location for defining
 * such unique module IDs. All modules supported in ACE including the supported
 * SDKs can add an ID for their module in this header. For applications, a
 * specific range of IDs is reserved and they can define the macros in this
 * range in their own headers.
 *
 * @note The naming conventions for the macros is `ACE_MODULE_<MODULE_NAME>. For
 * example, the module id for KVS is defined as `ACE_MODULE_KV_STORAGE`.
 *
 * @note The range supported for ACE modules and the supported SDKs is from
 * ACE_MODULE_GROUP to ACE_MODULE_MAX.
 *
 * @note The range supported for applications is from ACE_MODULE_MAX + 1 to
 * ACE_APPS_MODULE_MAX. Please look at the ACE modules that you are using for
 * more details.
 */
typedef enum {
    ACE_MODULE_GROUP = 0,            /** ACE shared resources */
    ACE_MODULE_KV_STORAGE = 1,       /** ACE KVS*/
    ACE_MODULE_MAPLITE = 2,          /** Maplite */
    ACE_MODULE_FFS = 3,              /** FFS provisionable */
    ACE_MODULE_BUTTON_MGR = 4,       /** button manager */
    ACE_MODULE_LED_MGR = 5,          /** LED manager */
    ACE_MODULE_INPUT_MGR = 6,        /** input manager */
    ACE_MODULE_OTAD = 7,             /** OTA daemon */
    ACE_MODULE_POWERD = 8,           /** Power daemon */
    ACE_MODULE_THERMALD = 9,         /** Thermal daemon*/
    ACE_MODULE_TCOMM_SHIM = 10,      /** TComm-Shim */
    ACE_MODULE_MESSAGING = 11,       /** Messaging */
    ACE_MODULE_LOG_MGR = 12,         /** Log Manager */
    ACE_MODULE_WIFI = 13,            /** Wifi */
    ACE_MODULE_METRIC_MGR = 14,      /** Metric manager */
    ACE_MODULE_ATZ = 15,             /** Auto Time Zone */
    ACE_MODULE_LIFECYCLE = 16,       /** Lifecycle autogen */
    ACE_MODULE_MEDIA = 17,           /** Media */
    ACE_MODULE_EVENTMGR = 18,        /** EventMgr */
    ACE_MODULE_ATAP = 19,            /** ATAP */
    ACE_MODULE_ETHERNET = 20,        /** Ethernet */
    ACE_MODULE_TIME_SERVICE = 21,    /** Time service */
    ACE_MODULE_ZIGBEE = 22,          /** Zigbee */
    ACE_MODULE_BATTERY = 23,         /** Battery */
    ACE_MODULE_SMARTHOME_SDK = 24,   /** Smarthome SDK */
    ACE_MODULE_AWS_IOT_SDK = 25,     /** AWS IOT */
    ACE_MODULE_AUDIO_MGR = 26,       /** Audio manager */
    ACE_MODULE_LOG_UPLOADER = 27,    /** Log uploader */
    ACE_MODULE_SENSORS = 28,         /** Sensor Fusion Framework */
    ACE_MODULE_ENTRYDS = 29,         /** EntryDs */
    ACE_MODULE_FFS_PROVISIONER = 30, /** FFS provisioner */
    ACE_MODULE_JSON = 31,            /** JSON Parser */
    ACE_MODULE_IPC_FRAMEWORK = 32,   /** IPC Framework (autogen) */
    ACE_MODULE_BT = 33,              /** Bluetooth */
    ACE_MODULE_NETWORK_MANAGER = 34, /** Network Manager */
    ACE_MODULE_CLI = 35,             /** CLI */
    ACE_MODULE_AMA = 37,             /** Alexa Mobile Accessory */

    ACE_MODULE_MAX = 1024, /** Max limit for ACE modules */
    /* Reserved range for applications */
    ACE_APPS_MODULE_MAX = 2048 /** Max limit for application modules */
    /* Reserved for future use */
} aceModules_moduleId_t;

#ifdef __cplusplus
}
#endif

#endif /* ACE_MODULES_DOT_H */
