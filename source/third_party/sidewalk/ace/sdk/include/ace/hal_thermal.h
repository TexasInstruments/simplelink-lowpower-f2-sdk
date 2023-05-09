/*
 * Copyright 2018-2019 Amazon.com, Inc. or its affiliates. All rights reserved.
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

#ifndef ACE_SDK_HAL_THERMAL_H_
#define ACE_SDK_HAL_THERMAL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <ace/ace_status.h>

/**
 * @file hal_thermal.h
 *
 * @brief ACE Thermal HAL provides the interfaces to require thermal zone,
 *  cooling zone data, pass thermal policy to low level for linux system
 *  receive thermal callback for thermal state change. This layer
 * layer will serve as the commands/requests from the Thermal Service
 * middleware.
 * @addtogroup ACE_HAL_THERMAL
 * @{
 */

/**
 * @defgroup ACE_HAL_THERMAL_DS Data Structures and Enum
 * @{
 * @ingroup ACE_HAL_THERMAL
 */

/**
 * @brief String size for name in data structure
 */
#define ACE_THERMALHAL_SIZE 16

/**
 * @brief HAL layer event message to notify certain event happened
 */
typedef enum {
    /**
     * Thermal State Change Event
     */
    ACE_THERMALHAL_THERMAL_STATE = 0,
    /**
     * Cooling Device State Change Event
     */
    ACE_THERMALHAL_COOLING_STATE,
    /**
     * This notification is used for user case policy
     * which come with
     * ucase::MitigationTechnique::value::trip::zone
     */
    ACE_THERMALHAL_THERMAL_MITIGATION,
    /**
     * For thermal shutdown event
     */
    ACE_THERMALHAL_THERMAL_SHUTDOWN
} aceThermalHal_msg_t;

/**
 * @brief Data ID to read sensor or cooling device indivitual data
 */
typedef enum {
        ACE_HAL_THERMAL_TEMP_VIRTUAL = 0,
        ACE_HAL_THERMAL_TRIP_TIME_VIRTUAL,
        ACE_HAL_THERMAL_TRIP_USAGE_VIRTUAL,
        ACE_HAL_THERMAL_LOCAL_MAX_VIRTUAL,
        ACE_HAL_THERMAL_CTRIP_VIRTUAL,
        ACE_HAL_THERMAL_TEMP_CPU,
        ACE_HAL_THERMAL_CPU_STATUS,
        ACE_HAL_THERMAL_CPU_FREQ,
        ACE_HAL_THERMAL_GPU_FREQ,
        ACE_HAL_THERMAL_LCD_LEVEL,
        ACE_HAL_THERMAL_CHARGER_LIMIT,
        ACE_HAL_THERMAL_CPU_FREQ_MAX,
        ACE_HAL_THERMAL_CPU_FREQ_MIN,
} aceThermalHal_dataID_t;

/**
 * @brief Thermal Zone Data
 * For linux type of platform, it directly map to thermal zone.
 * For RTOS, the thermal sensor can still be structure as the
 * same way in HAL,
 */
typedef struct {
    char  name[ACE_THERMALHAL_SIZE];
    int32_t tz_index; /**index for different thermal zone*/
    int32_t temperature; /**current sesnsor reading*/
    int32_t trip_temp[ACE_THERMALHAL_SIZE];
    bool isActive;
} aceThermalHal_tz_t;

/**
 * callback to set cooling device state
 */
typedef int32_t (*aceThermalCDSet_State_Callback_t)(int32_t state);
/**
 * @brief Cooling Device Data
 * For Linux type, it map directly to cooling devices
 * For RTOS, hal will map those value to cool device.
 * i.e, cpu speed is defined as different cooling state.
 * cb_setCDState is used when there is no thermal/
 * cooling manager in low level such as RTOS project.
 * Otherwise, set to NULL such as Linux or NoART
 */
typedef struct {
    char name[ACE_THERMALHAL_SIZE];
    int32_t cd_index;
    int32_t state;
    int32_t max_state;
    aceThermalCDSet_State_Callback_t cb_setCDState;
} aceThermalHal_cd_t;

/**
 * @brief Data for thermal callback registered corresponding to different thermal message.
 * This is for  ACE_THERMALHAL_THERMAL_STATE callback message
 */
typedef struct {
    char name[ACE_THERMALHAL_SIZE];
    int32_t tz_index;
    int32_t thermal_state;
} aceThermalHal_tzData_t;

/**
 * @brief Data for thermal callback registered corresponding to different thermal message.
 * This is for ACE_THERMALHAL_COOLING_STATE callback message
 */
typedef struct{
    char name[ACE_THERMALHAL_SIZE];
    int32_t cd_index;
    int32_t cooling_state;
} aceThermalHal_cdData_t;

/**
 * @brief Data Structure for thermal callback registered corresponding to different thermal message.
 * The data can be differentiated through aceThermalHal_msg_t
 */
typedef struct {
    aceThermalHal_msg_t type;
    union DATA {
        char* rsp;  // string "ucase::MitigationTechnique::value::trip::zone"
        aceThermalHal_tzData_t tzRSP;
        aceThermalHal_cdData_t cdRSP;
    } data;
} aceThermalHal_cbData_t; //response data for callback.

/*@brief Call back function defintion */
typedef void (*aceThermalHalCallback_t)(aceThermalHal_cbData_t* data);

/** @} */

/**
 * @brief apply the thermal policy configuration
 * @param[in] configure Thermal policy configure file or data structure. For Linux, it is a file
 * pointed to location of thermal policy file in flash file system (FFS). For RTOS, it may be NULL
 * when using MW thermal policy manager or a data structure of thermal policy configuration if RTOS
 * level supports thermal policy manager.
 *
 * @return ACE_STATUS_OK on success, otherwise one of the error codes @ref ace_status_t.
 */
ace_status_t aceThermalHal_init(const void * configure);

/**
 * @brief Get the total number of thermal zones
 *
 * @return The total number of thermal zone
 */
int32_t aceThermalHal_getTotalSizeOfTZ(void);

/**
 * @brief Get total number of Cooling Devices
 * @return Total number of Cooling Devices
 */
int32_t aceThermalHal_getTotalSizeOfCD(void);

/**
 * @brief Reload the thermal policy
 * @param[in] configure Thermal policy configure file or data structure
 *
 * @return ACE_STATUS_OK on success, otherwise one of the error codes @ref ace_status_t.
 */
ace_status_t aceThermalHal_setPolicy(const void * configure);


/**
 * @brief Get value for different data ID
 * @param[in] id     Specify the data ID
 * @param[in] arg    integer value from client side
 *
 * @return the value of cooling device or sensor with the queried Data ID
 */
int32_t aceThermalHal_getData(const aceThermalHal_dataID_t id, int32_t arg);

/**
 * @brief Register Callback function for specifying the message id
 * @param[in] callback callback function
 *
 * @return ACE_STATUS_OK on success, otherwise one of the error codes @ref ace_status_t.
 */
ace_status_t aceThermalRegisterCallback(const aceThermalHalCallback_t callback);

/**
 * @brief Read thermal zone data
 *
 * @param[in] index  Index of the thermal zone to be read
 *
 * @return Thermal zone data pointer. NULL for not available for this index
 */
aceThermalHal_tz_t*  aceThermalHal_getTZData(int32_t index);

/**
 * @brief Read Cooling Device data
 *
 * @param[in] index  Index of the cooling device to be read
 *
 * @return cooling device data pointer. NULL for not available for this index
 */
aceThermalHal_cd_t* aceThermalHal_getCDData(int32_t index);


/**
 * @brief Test API to simulate thermal shutdown by set fake temperature
 * @param[in] sensor The sensor to be set with Fake temperature
 * @param[in] temp   Fake temperature value
 *
 * @return ACE_STATUS_OK on success, otherwise one of the error codes @ref ace_status_t.
 */
ace_status_t aceThermalHal_setTFake(const char* sensor, int32_t temp);

/**
 * @brief Test API for simulating sensor reading crossing trip point
 * @param[in] zone   sensor zone number
 * @param[in] trip   trip point
 * @param[in] t      temperature value
 *
 * @return ACE_STATUS_OK on success, otherwise one of the error codes @ref ace_status_t.
 */
ace_status_t aceThermalHal_setTZoneTrip(int32_t zone, int32_t trip, int32_t t);

#ifdef __cplusplus
}
#endif

/** @} */

#endif // ACE_SDK_HAL_THERMAL_H_

