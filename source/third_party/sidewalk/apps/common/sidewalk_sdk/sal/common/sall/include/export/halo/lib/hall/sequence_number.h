/*
 * Copyright 2019-2021 Amazon.com, Inc. or its affiliates. All rights reserved.
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
 * @package halo_hall
 *
 * @tableofcontents
 * @brief Halo sequence_number library.
 *
 * @section Problem
 * Sequence number can be duplicated after reboot due to the sequence counter reset.
 *
 * @section Solution
 * Periodically save current sequence number to the flash.
 * To create sequence_number library the following data is required:
 * - configuration for sequence_number.
 * - constructor
 */

#ifndef HALO_LIB_HALL_SEQUENCE_NUMBER_INCLUDE_EXPORT_HALO_LIB_HALL_SEQUENCE_NUMBER_H
#define HALO_LIB_HALL_SEQUENCE_NUMBER_INCLUDE_EXPORT_HALO_LIB_HALL_SEQUENCE_NUMBER_H

#include <stdint.h>
#include <halo/lib/hall/hallerr.h>

typedef const struct halo_lib_sequence_number_ifc_s* halo_lib_sequence_number_ifc;

/**
 * @struct halo_lib_sequence_number_ifc_s
 * @brief interface for sequence_number library
 */
struct halo_lib_sequence_number_ifc_s {

    /**
     * @brief Get incremental sequence number and save periodically to the persistent storage
     * @param ifc   pointer to an interface instance
     * @return      sequence number
     */
    uint16_t (*get)(const halo_lib_sequence_number_ifc * ifc);

    /**
     * @brief destroy sequence number instance, need to call to free used resources
     * @param ifc pointer to an interface instance
     */
    void (*destroy) (const halo_lib_sequence_number_ifc * ifc);
};

/**
 * @brief Factory interface of sequence number lib.
 *
 * "create" method and "config" pointers must by initialized by device/application specific params.
 */
typedef struct {

    /**
     * @brief Pointer to platform specific driver "create" method
     *
     * @param[out] ifc            pointer to address placeholder for the interface instance to be created
     * @param[in] config          application specific lib configuration
     * @param[in] config_manager  halo configuration manager instance
     *
     * This method is used to validate configuration, allocate memory and create the lib obj
     */
    sid_error_t (*create) (const halo_lib_sequence_number_ifc ** ifc, const void * config);

    /**
     * @brief Pointer to application specific lib obj configuration.
     * Configuration that is provided to "create" method
     */
    const void * config;
} halo_lib_sequence_number_factory_t;

#endif // HALO_LIB_HALL_SEQUENCE_NUMBER_INCLUDE_EXPORT_HALO_LIB_HALL_SEQUENCE_NUMBER_H
