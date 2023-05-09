/*
 * Copyright 2019 Amazon.com, Inc. or its affiliates. All rights reserved.
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

#ifndef ACE_SDK_HAL_METRIC_H_
#define ACE_SDK_HAL_METRIC_H_

/**
 * @file hal_metric.h
 * @brief ACE metric platform API
 *
 * Metric HAL provides the interfaces for platform to record metrics with
 * the middleware.
 *
 * The interface uses a callback model, middleware registers callbacks with
 * the platform at initialization with @ref aceMetricHal_init and the
 * platform invokes those callbacks when a new metric needs to be recorded.
 *
 * The API supports PMET and KDM metric types.
 *
 * The callbacks may be invoked on any task running the platform code and
 * hence the callbacks must be thread safe.
 *
 * Note: "Platform" here means metric HAL implementation or consumer of
 * API in layers below the middleware.
 *
 * @addtogroup ACE_HAL_METRIC
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <ace/ace_status.h>

/**
 * @brief This enum defines a PMET datapoint type. PMET can be a counter,
 *        timer or a string type.
 */
typedef enum {
    /** Datapoint type used for counting metrics */
    ACE_METRIC_HAL_DATAPOINT_COUNTER = 0,
    /** Datapoint type used for timing metrics */
    ACE_METRIC_HAL_DATAPOINT_TIMER,
    /** Datapoint type used for string value metrics */
    ACE_METRIC_HAL_DATAPOINT_DISCRETE
} aceMetricHal_pmetDatapointType_t;

/**
 * @brief Priority for PMET.
 *        Normal priority metrics will be uploaded periodically,
 *        High priority metrics will be attempted to upload immediately.
 *        Use High priority cautiously.
 *        Vitals are uploaded at high priority
 */
typedef enum {
    /** Normal Priority */
    ACE_METRIC_HAL_PRIORITY_NORMAL = 0,
    /** High Priority */
    ACE_METRIC_HAL_PRIORITY_HIGH
} aceMetricHal_priority_t;

/**
 * @brief This structure defines a PMET datapoint. A PMET is a performance
 *        metric, a variable to monitor over time, and datapoint represents
 *        one value for that variable.
 */
typedef struct aceMetricHal_pmetDatapoint {
    /** Type of the datapoint, determines value interpretation */
    aceMetricHal_pmetDatapointType_t type;
    /** Name of the datapoint */
    const char* name;
    /** Number of samples in the datapoint */
    uint8_t samples;
    /** Value of the datapoint determined by the type */
    union {
        /** Value of string type */
        const char* str;
        /** Value of counter type */
        int32_t counter;
        /* Value of timer type */
        uint64_t timer;
    } value;
} aceMetricHal_pmetDatapoint_t;

/**
 * @brief KDM metric metadata structure with a key/value pair used to
 *        represent a dimension or an annotation.
 */
typedef struct aceMetricHal_metadata {
    /** Key of the metatada */
    const char* key;
    /** Value of the metadata */
    const char* value;
} aceMetricHal_metadata_t;

/**
 * @brief Metric middleware callback to record a PMET metric
 *
 * This API creates a PMET metric and schedules an upload in the middleware.
 * This callback is registered to platform at metric_mgr init time through
 * @ref aceMetricHal_init.
 *
 * A pmet to log the usage time of an app may look like:
 *  aceMetricHal_pmetDatapoint_t dp[] = {
 *    {METRIC_DATAPOINT_TIMER, "photos", .value.timer = <time>}
 *  }
 *  cb("acePlatform", "appTracking", 1, dp)
 *
 * KDM supercedes PMET in feature set and backend capability, so prefer
 * it over PMET, unless for legacy reasons.
 *
 * @param[in] program        program of the pmet, non-null
 * @param[in] source         source of the pmet, non-null
 * @param[in] num_datapoints number of datapoints, > 0
 * @param[in] datapoints     one or more @ref aceMetricHal_pmetDatapoint_t
 * @param[in] prio           metric priority of type @ref
 * aceMetricHal_priority_t
 *
 * @return ace_status_t ACE_STATUS_OK on success, or an ace_status_t error
 * otherwise
 * @retval ACE_STATUS_BAD_PARAM invalid parameters
 * @retval ACE_STATUS_BUFFER_OVERFLOW metric buffer overflowed
 * @retval ACE_STATUS_GENERAL_ERROR other failures
 */
typedef ace_status_t (*aceMetricHal_recordPmetCb_t)(
    const char* program, const char* source, size_t num_datapoints,
    const aceMetricHal_pmetDatapoint_t* datapoints,
    aceMetricHal_priority_t prio);

/**
 * @brief Metric middleware callback to record a KDM metric (vitals)
 *
 * This API creates a KDM metric (vitals) and schedules an upload in the
 * middleware. This callback is registered to platform at metric_mgr init time
 * through @ref aceMetricHal_init.
 *
 * Dimensions are key-value pairs used for aggregation and kept in KDM backend.
 * Annotations are transient key-values pairs attached to high resolution
 * metrics (usually for debugging) and dropped after aggregation.
 *
 * The concepts are similar to AWS cloudwatch metrics described here:
 * https://docs.aws.amazon.com/AmazonCloudWatch/latest/monitoring/cloudwatch_concepts.html
 *
 * An example KDM metric to log crashes from memory system:
 *   cb("acePlatform", "memorySubSystem", "counter", 1,
 *       3, {{"task", <task_name>}, {"type", "HeapCorruption"},
 *           {"hash", <hashcode_crash>},
 *       1, {{"top_fn_addr", "<top_fn_addr_crash"}})
 * From this data, KDM backend can answer queries about the number of crashes
 * per task/type/hash or a combination. The top function address is just
 * transient data for debugging.
 *
 * There is a maximum number of 7 dimensions allowed by middleware, 2 of
 * these are reserved for "key" and "metadata". This means max number of
 * custom dimensions allowed other than "key" and "metadata" is 5.
 *
 * Annotations do not have such limit, however the constructed metric must not
 * overflow the internal buffer.
 *
 * The middleware metric buffer size can be as small as 256 bytes, this is used
 * for encoding all the metric contents, including key/values in all dimensions
 * and annotations. A buffer of 256 bytes can approximately fit 7 dimensions/3
 * annotations with key/value size 10 bytes each. In case of overflow, the API
 * returns ACE_STATUS_BUFFER_OVERFLOW.
 *
 * @param[in] program         program of the vitals, non-null
 * @param[in] source          source of the vitals, non-null
 * @param[in] unit            unit of the vitals, non-null
 * @param[in] value           value of the vitals
 * @param[in] num_dimensions  number of dimensions
 * @param[in] dimensions      key/value pairs of dimensions
 * @param[in] num_annotations number of annotations
 * @param[in] annotations     key/value pairs of annotations
 *
 * @return ace_status_t ACE_STATUS_OK on success, or an ace_status_t error
 * otherwise
 * @retval ACE_STATUS_BAD_PARAM invalid parameters
 * @retval ACE_STATUS_BUFFER_OVERFLOW metric buffer overflowed
 * @retval ACE_STATUS_GENERAL_ERROR  other failures
 */
typedef ace_status_t (*aceMetricHal_recordVitalsCb_t)(
    const char* program, const char* source, const char* unit, float value,
    size_t num_dimensions, const aceMetricHal_metadata_t* dimensions,
    size_t num_annotations, const aceMetricHal_metadata_t* annotations);

/**
 * @brief This structure defines callbacks to be implemented by the
 *        middleware for recording PMET and KDM metric.
 */
typedef struct aceMetricHal_callbacks {
    /** Callback for recording a PMET metric */
    aceMetricHal_recordPmetCb_t pmet_cb;
    /** Callback for record a KDM vitals metric */
    aceMetricHal_recordVitalsCb_t vitals_cb;
} aceMetricHal_callbacks_t;
/** @} */

/**
 * @brief Set the metric callbacks for platform to record metrics with
 * middleware
 *
 * @param[in] callbacks PMET and Vitals handlers in middleware.
 *
 * @return ace_status_t ACE_STATUS_OK on success, or an ace_status_t error
 * @retval ACE_STATUS_BAD_PARAM input is invalid
 * @retval ACE_STATUS_ALREADY_INITIALIZED api called multiple times
 * @defgroup ACE_HAL_METRIC_API API
 * @ingroup ACE_HAL_METRIC
 * @{
 */
ace_status_t aceMetricHal_init(const aceMetricHal_callbacks_t* callbacks);
/** @} */

#ifdef __cplusplus
}
#endif

#endif /* ACE_SDK_HAL_METRIC_H_ */
