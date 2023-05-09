/*
 * Copyright 2019-2020 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.txt file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef LIB_RINGNET_WAN_SRC_INCLUDE_RNET_CLD_ADDR_DEF_H_
#define LIB_RINGNET_WAN_SRC_INCLUDE_RNET_CLD_ADDR_DEF_H_

#include <stdint.h>
#include <rnet_errors.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_CLD_ID_SZ_BYT                      3   // 3 msb + 21 effective address bit

/*
 * @brief enumeration of cloud end-points id
 *
 * For details please visit: https://wiki.labcollab.net/confluence/pages/viewpage.action?pageId=859543004
 */
typedef enum{
/*Cloud address definitions*/
    RNET_CLD_BEAMS_SERVICE = 0,
    RNET_CLD_HALO_NETWORK_SERVER            = 1,
    RNET_CLD_HALO_METRICS                   = 2,
    RNET_CLD_HALO_TIME_SYNC_SERVICE         = 3,
    RNET_CLD_HALO_PROV_SERVER               = 4,
    RNET_CLD_HALO_OFFLINE_SERVER            = 5,
    RNET_CLD_HALO_CHIMERA_SERVICE           = 6,
    RNET_CLD_MOBILE_D2D_SERVICE             = 7,
    RNET_CLD_HALO_CONNECTIVITY_SERVICE      = 8,
/*Address 9 to 31 is reserved for the future use*/
/*Application Server definitions*/
    RNET_CLD_TRACKER_APPLICATION            = 32,
    RNET_CLD_SAILBOAT_APPLICATION           = 33,
    RNET_CLD_ALS_APPLICATION                = 34,
    RNET_CLD_BMDS_APPLICATION               = 35,
    RNET_CLD_TILE_APPLICATION               = 36,
    RNET_CLD_ALEXA_PRESENCE_APPLICATION     = 37,


    /*Last address: Should not be used for any valid addressing*/
    RNET_CLD_LAST_ADDR = 0xFFFFFFFF

    /*
     * ToDo: Move this into device config
     * Jira: https://issues.labcollab.net/browse/HALO-3233
     * */

}rnet_cld_addr_t;

/*
 * @brief Function to serialize cloud address encoded with the size in the 1st byte
 *
 * @param[in] cloud address
 * @param[in] buffer to carry out serialized address.
 * @param[in-out] max buffer size as input. Once executed returns the serialized len
 * @return[out] rnet error code
 */
rnet_error_t rnet_serialize_cld_addr(rnet_cld_addr_t cld_addr, uint8_t *addr_buf, uint8_t* max_buf_sz)
    __attribute__((nonnull));

/*
 * @brief Function to de-serialize cloud address.
 *
 * @param[in] cloud address pointer to carry out de-serialized address
 * @param[in] buffer pointer to the serialized address.
 * @return[out] rnet error code
 */
rnet_error_t rnet_deserialize_cld_addr(rnet_cld_addr_t *cld_addr, uint8_t *addr_buf);

#ifdef __cplusplus
}
#endif

#endif /* LIB_RINGNET_WAN_SRC_INCLUDE_RNET_CLD_ADDR_DEF_H_ */
