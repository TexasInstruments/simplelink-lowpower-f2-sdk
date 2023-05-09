/*
 * Copyright 2020-2022 Amazon.com, Inc. or its affiliates. All rights reserved.
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

#ifndef SID_NETWORK_ADDRESS_H
#define SID_NETWORK_ADDRESS_H

#include <sid_error.h>

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SID_ADDRESS_SIZE_MAX 7
#define SID_ADDRESS_WILDCARD 0xFF

/**
 * @enum sid_address_type defines the type of sidewalk address
 */
enum sid_address_type {
    SID_LL_ATYPE_CLOUD = 0,
    SID_LL_ATYPE_DEVICE = 1,
    SID_LL_ATYPE_HNDL = 2,
    SID_LL_ATYPE_GROUP = 3,
    SID_LL_ATYPE_UNDEFINED = 4
};

/**
 * @struct sid_address contains Sidewalk id, used to route messages in Sidewalk Network
 */
struct sid_address {
    uint8_t type;   //!< enum sid_address_type reduced to uint8 to minimize memory footprint
    uint8_t addr[SID_ADDRESS_SIZE_MAX];
};

#define SID_ADDR_FILTER_ANY                           \
    {                                                 \
        .type = SID_LL_ATYPE_UNDEFINED, .addr = { 0 } \
    }

#define SID_ADDR_FILTER_DEVICE_ANY                                    \
    {                                                                 \
        .type = SID_LL_ATYPE_DEVICE, .addr = { SID_ADDRESS_WILDCARD } \
    }

#define SID_ADDR_FILTER_GROUP_ANY                                    \
    {                                                                \
        .type = SID_LL_ATYPE_GROUP, .addr = { SID_ADDRESS_WILDCARD } \
    }

#define SID_ADDR_INT_CLOUD_DEFAULT               \
    {                                            \
        .type = SID_LL_ATYPE_CLOUD, .addr = {0}, \
    }

#define SID_ADDR_INT_DEVICE_DEFAULT               \
    {                                             \
        .type = SID_LL_ATYPE_DEVICE, .addr = {0}, \
    }

/**
 * Initialize sid_address structure using raw_address array
 *
 * @param[out] addr Pointer to sidewalk address
 * @param[in] type Type of sidewalk address
 * @param[in] raw_addr Pointer to the memory location that contains the raw sidewalk address array
 *
 * @returns #SID_ERROR_NONE on success otherwise an appropriate error value is returned
 */
sid_error_t
sid_address_init(struct sid_address *const addr, const enum sid_address_type type, const void *const raw_addr);

/**
 * Fill up a buffer with raw address value from sidewalk address
 *
 * @param[in] addr Pointer to sidewalk address
 * @param[out buffer Pointer to raw buffer that will be filled up the address value
 * @param[in] buffer_size Size of the buffer address
 *
 * @returns #SID_ERROR_NONE on success other an appropriate error value is returned
 */
sid_error_t sid_address_to_raw_buffer(const struct sid_address *const addr, void *const buffer, size_t buffer_size);

/**
 * Get the size in bytes of sidewalk address
 *
 * @note The size of sidewalk address is encoded in first 3 bits of the first byte, and as such
 *       the maximum size if 7 bytes (0b111)
 *
 * @param[in] addr Pointer to sidewalk address
 *
 * @returns uint8_t size of the sidewalk address
 */
uint8_t sid_address_get_size(const struct sid_address *const addr);

/**
 * Get the size in bytes of the raw sidewalk address
 *
 * @param[in] raw_addr Pointer to the memory location that contains the raw sidewalk address array
 *
 * @returns uint8_t size of the sidewalk address
 */
uint8_t sid_address_get_size_from_raw(const void *const raw_addr);

/**
 * Set the size of the raw sidewalk address
 *
 * @param[in] raw_addr Pointer to the memory location that contains the raw sidewalk address array
 * @param[in] size of the address in bytes
 */
void sid_address_set_raw_size(void *raw_addr, uint8_t size);

/**
 * Check if two sidewalk address's are the same
 *
 * @param[in] addr1 Pointer to first sidewalk address
 * @param[in] addr2 Pointer to second sidewalk address
 *
 * @returns bool true if the address's are same or false otherwise
 */
bool sid_address_is_equal(const struct sid_address *const addr1, const struct sid_address *const addr2);

/**
 * Check if two raw sidewalk address's are the same
 *
 * @param[in] raw_addr1 Pointer to first sidewalk raw address
 * @param[in] raw_addr2 Pointer to second sidewalk raw address
 *
 * @returns bool true if the raw address's are same or false otherwise
 */
bool sid_address_is_raw_equal(const void *raw_addr1, const void *raw_addr2);

/**
 * Wildcard filter sidewalk address
 *
 * @note To filter against any address type, initialize filter address using
 *       #SID_ADDR_FILTER_ANY, to filter against any group or device address initialize
 *       filter address using #SID_ADDR_FILTER_GROUP_ANY and #SID_ADDR_FILTER_DEVICE_ANY,
 *       to match against an explicit address initialize filter address normally using
 *       #sid_address_init
 *
 * @param[in] filter Pointer to sidewalk address that acts as a filter
 * @param[in] addr Pointer to sidewalk address that needs to chcked against the filter address
 *
 * @returns bool true if given address matches against filter false otherwise
 */
bool sid_address_match(const struct sid_address *const filter, const struct sid_address *const addr);

/**
 * Convert sidewalk address to cloud service's destination ID.
 *
 * @param[in]  addr A pointer to the sidewalk address.
 * @param[out] id   A pointer to cloud service ID.
 *
 * @returns #SID_ERROR_NONE on success.
 * @returns #SID_ERROR_INVALID_ARGS  if either addr or id is NULL, or addr contents are invalid.
 */
sid_error_t sid_address_to_cloud_id(const struct sid_address *const addr, uint32_t *const id);

/**
 * Convert cloud service's destination ID to sidewalk address.
 *
 * @param[in]  id   cloud service's destination ID.
 * @param[out] addr   A pointer to sidewalk address.
 *
 * @returns #SID_ERROR_NONE on success.
 * @returns #SID_ERROR_INVALID_ARGS  if either addr is NULL, or id is invalid.
 */
sid_error_t sid_cloud_id_to_address(uint32_t id, struct sid_address *const addr);

/**
 * Set the local system address
 *
 * The behaviour is defined below
 * address, flush, result
 *  set   , false, stored in ram
 *  set   , true,  stored in flash
 *  unset , false, error
 *  unset , true   if address is in ram then stored to flash else error
 *
 * @param[in] address New address to be set
 * @param[in] flush if true store in flash else store in ram
 */
void sid_address_set_local(const struct sid_address *const address, bool flush);

/**
 * Get the local system address
 *
 * returns #sid_address which is local system address
 */
struct sid_address sid_address_get_local(void);

/**
 * Initialize the local addres library
 */
void sid_address_lib_init(void);

#ifdef __cplusplus
}
#endif

#endif
