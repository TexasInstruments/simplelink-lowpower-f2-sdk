/*
 * Copyright 2018-2020 Amazon.com, Inc. or its affiliates. All rights reserved.
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
/**
 * @file hal_ambientLightSensor.h
 *
 * @brief ACE ALS (Ambient Light Sensor) HAL builds on top of platform specific ALS
 * device driver. It enables the upper layer software to retrieve sensor lux value
 * from ambient light sensor.
 * @addtogroup ACE_HAL_AMBIENT_LIGHT
 *
 * USAGE
 * -----
 *
 * Steps to setup and use ALS HAL interface:
 * 1) aceAmbientLightSensorHal_open needs to be called first to have ALS HAL APIs ready for use.
 * 2) Once the Ambient Light Sensor HAL is open, aceAmbientLightSensorHal_getLux,
 *    aceAmbientLightSensorHal_minLux and aceAmbientLightSensorHal_maxLux can be used to retrieve
 *    sensor lux values.
 * 3) aceAmbientLightSensorHal_close needs to be called upon exit.
 *
 * @{
 */

#ifndef ACE_HAL_AMBIENT_LIGHT_SENSOR_H_
#define ACE_HAL_AMBIENT_LIGHT_SENSOR_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <ace/ace_status.h>

/**
 * Open must be called before using any ALS HAL api.
 *
 * @param: None
 * @return ace_status_t: Return zero if Open is successful,
 * or any non-zero error code.
 */
ace_status_t aceAmbientLightSensorHal_open(void);

/**
 * Close must be called when exiting ALS HAL
 *
 * @param: None
 * @return ace_status_t: Return zero if Close is successful,
 * or any non-zero error code.
 */
ace_status_t aceAmbientLightSensorHal_close(void);

/**
 * Sample sensor and return lux reading.
 *
 * @param[out] lux: sensor lux reading
 * @return Return zero if lux reading is successful,
 * or any non-zero error code.
 */
ace_status_t aceAmbientLightSensorHal_getLux(double* lux);

/**
 * Get sensor minimum possible lux value.
 *
 * @param[out] min_lux: minimum possible lux value that will ever be returned.
 * @return ace_status_t: Return zero if minimum lux is available,
 * or any non-zero error code.
 */
ace_status_t aceAmbientLightSensorHal_getMinLux(double* min_lux);

/**
 * Get sensor maximum possible lux value.
 *
 * @param[out] max_lux: maximum possible lux value that will ever be returned.
 * @return ace_status_t: Return zero if maximum lux is available,
 * or any non-zero error code.
 */
ace_status_t aceAmbientLightSensorHal_getMaxLux(double* max_lux);

#ifdef __cplusplus
}
#endif

#endif /* ACE_HAL_AMBIENT_LIGHT_SENSOR_H_ */
/** @} */
