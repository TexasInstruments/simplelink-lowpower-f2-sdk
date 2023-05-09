/*
 * Copyright 2021 Amazon.com, Inc. or its affiliates. All rights reserved.
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

#ifndef SID_AMA_TRANSPORT_IFC_H
#define SID_AMA_TRANSPORT_IFC_H

#include <sid_error.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum sid_ama_transport_state {
    SID_AMA_TRANSPORT_STATE_ACTIVE = 1,
    SID_AMA_TRANSPORT_STATE_INACTIVE,
};

enum sid_ama_transport_tx_complete_status {
    SID_AMA_TRANSPORT_TX_COMPLETED = 1,
    SID_AMA_TRANSPORT_TX_INCOMPLETED,
};

typedef void (* sid_ama_transport_state_change_cb_t)(const void *const context,
                                                     enum sid_ama_transport_state state);
typedef void (* sid_ama_transport_deserialize_cb_t)(const void *const context,
                                                    const uint8_t *const data,
                                                    uint16_t length);
typedef void (* sid_ama_transport_serialize_cb_t)(const void *const context,
                                                  const uint8_t *const data,
                                                  uint16_t length);
typedef void (* sid_ama_transport_tx_complete_cb_t)(const void *const context,
                                                    enum sid_ama_transport_tx_complete_status status);

struct sid_ama_transport_adapter_cb {
    sid_ama_transport_state_change_cb_t state_change_cb;
    sid_ama_transport_deserialize_cb_t  deserialize_cb;
    sid_ama_transport_serialize_cb_t    serialize_cb;
    sid_ama_transport_tx_complete_cb_t  tx_complete;
};

/** @brief   Initialize transport adapter.
 *
 * @param[in] cb      callback functions for this transport
 * @param[in] context context argument
 *
 * @return  SID_ERROR_NONE on success otherwise appropriate error is returned.
 */
sid_error_t sid_ama_transport_init(const struct sid_ama_transport_adapter_cb *cb,
                                   const void *const context);

/** @brief   Set transport address and connection state.
 *
 * @param[in] addr
 * @param[in] addr_len
 * @param[in] connected
 *
 * @return  SID_ERROR_NONE on success otherwise appropriate error is returned.
 */
sid_error_t sid_ama_transport_set_connection(const uint8_t *const addr, size_t addr_len, bool connected);

/** @brief   Set negotiated mtu size.
 *
 * @param[in] size mtu size.
 *
 * @return  None.
 */
void sid_ama_transport_set_mtu(uint16_t size);

/** @brief   Notify if remote has enabled/disabled subscription on characteristics.
 *
 * @param[in] enable true if subscription is enabled.
 *
 * @return  SID_ERROR_NONE on success otherwise appropriate error is returned
 */
sid_error_t sid_ama_transport_set_subscription(bool enable);

/** @brief   Notify that data was sent over transport.
 *
 * @param[in] status true if data was sent, otherwise false.
 *
 * @return  None
 */
void sid_ama_transport_data_sent(bool status);

/** @brief   Serialize data
 *
 * @param[in] data   data to be sent in ama envelope.
 * @param[in] length length of the data in bytes.
 *
 * @return  SID_ERROR_NONE on success otherwise appropriate error is returned
 */
sid_error_t sid_ama_transport_serialize(const uint8_t *const data, uint16_t length);

/** @brief   De-serialize data
 *
 * @param[in] data   incoming data buffer to be de-serialized.
 * @param[in] length length of the data in bytes.
 *
 * @return  SID_ERROR_NONE on success otherwise appropriate error is returned
 */
sid_error_t sid_ama_transport_deserialize(const uint8_t *const data, uint16_t length);

/** @brief   Removes connected gateway from AMA
 *
 * @return  SID_ERROR_NONE on success otherwise appropriate error is returned
 */
sid_error_t sid_ama_transport_remove_gateway(void);

/** @brief   De-initialize transport adapter
 *
 * @return  None
 */
void sid_ama_transport_deinit(void);

#ifdef __cplusplus
}
#endif

#endif /* SID_AMA_TRANSPORT_IFC_H */
