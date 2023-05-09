/*
 * FreeRTOS BLE HAL V5.0.1
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://aws.amazon.com/freertos
 * http://www.FreeRTOS.org
 */

/**
 * @file bt_hal_sock.h
 *
 * @brief BT Socket provides the interfaces to control Bluetooth sockets.
 * @addtogroup HAL_BLUETOOTH
 * USAGE
 * -----
 *
 * @{
 */

#ifndef _BT_HAL_SOCK_H_
#define _BT_HAL_SOCK_H_

#include "bt_hal_manager_types.h"

/**
 * @brief Socket Type
 */
typedef enum aceBtHal_sock_type
{
    BTHAL_SOCKET_TYPE_RFCOMM = 1, /**< RFCOMM */
    BTHAL_SOCKET_TYPE_SCO = 2,    /**< SCO */
    BTHAL_SOCKET_TYPE_L2CAP = 3   /**< L2CAP */
} aceBtHal_sock_type_t;

/**
 * @brief Socket interface
 */
typedef struct
{
    short size;
    BTBdaddr_t bd_addr;
    int channel;
    int status;

    /* L2CAP only */
    uint16_t max_tx_packet_size;
    uint16_t max_rx_packet_size;
} __attribute__( ( packed ) ) aceBtHal_sock_connect_signal_t;

/**
 * @brief Connection State Changed Callback
 *
 * @param[in] addr Address of the Remote device
 * @param[in] fd File descriptor
 * @param[in] channel Channel
 * @param[in] status Status
 * @param[in] max_tx Maximum size allowed for write
 * @param[in] max_rx Maimum size allowed for read
 */
typedef void (* BTSockConnectionCallback) ( BTBdaddr_t * addr,
                                            int fd,
                                            int channel,
                                            int status,
                                            uint16_t max_tx,
                                            uint16_t max_rx );

/**
 * @brief Data Ready Callback
 *
 * @param[in] fd File descriptor
 * @param[in] bytes Bytes ready to be read
 */
typedef void (* BTSockDataReadyCallback) ( int fd,
                                           int bytes );

typedef struct
{
    size_t xSize;
    BTSockConnectionCallback connection_established_cb;
    BTSockDataReadyCallback data_ready_cb;
} BTSockCallbacks_t;

/** Represents the standard ACE BT SOCKET interface. */
typedef struct
{
    /** Set this to size of BTSockInterface_t */
    size_t size;

    /**
     * @brief Register callbacks
     *
     * @param[in] callbacks Socket callback
     * @return Returns eBTStatusSuccess on successful call.
     */
    BTStatus_t ( * pxSockInit )( BTSockCallbacks_t * callbacks );

    /**
     * @brief Deregister callbacks
     *
     * @return Returns eBTStatusSuccess on successful call.
     */
    BTStatus_t ( * pxSockCleanup )( void );

    /**
     * @brief Write to socket
     *
     * @param[in] fd File descriptor
     * @param[in] data Data payload
     * @param[in] len Length of buffer
     * @return Returns eBTStatusSuccess on successful call.
     */
    BTStatus_t ( * pxSockWrite )( int fd,
                                  uint8_t * data,
                                  uint32_t len );

    /**
     * @brief Read from socket
     *
     * @param[in] fd File descriptor
     * @param[in,out] data Data buffer, will hold read data
     * @param[in] Length of data buffer
     * @param[out] Lenght of read data
     * @return Returns eBTStatusSuccess on successful call.
     */
    BTStatus_t ( * pxSockRead )( int fd,
                                 uint8_t * data,
                                 uint32_t len,
                                 uint32_t * bytes_read );

    /**
     * @brief Close socket
     *
     * @param[in] fd File descriptor
     * @return Returns eBTStatusSuccess on successful call.
     */
    BTStatus_t ( * pxSockClose )( int fd );

    /**
     * @brief Create a socket to listen on a specified channel
     *
     * @param[in] type Socket type
     * @param[in] service_name Service name
     * @param[in] service_uuid Service UUID
     * @param[in] channel Channel
     * @param[in] flags Signal flag
     * @param[in] callingUid User ID
     * @param[out] socket_fd Created socket
     * @return Returns eBTStatusSuccess on successful call.
     */
    BTStatus_t ( * pxListen )( aceBtHal_sock_type_t type,
                               const char * service_name,
                               const char * service_uuid,
                               int channel,
                               int * sock_fd,
                               int flags,
                               int callingUid );

    /**
     * @brief Connect to a remote RFCOMM UUID channel
     *
     * @param[in] bd_addr Address of the Remote device
     * @param[in] type Socket type
     * @param[in] uuid UUID
     * @param[in] channel Channel
     * @param[in] flags Signal flag
     * @param[in] callingUid User ID
     * @param[out] socket_fd Created socket
     * @return Returns eBTStatusSuccess on successful call.
     */
    BTStatus_t ( * pxConnect )( BTBdaddr_t * bd_addr,
                                aceBtHal_sock_type_t type,
                                const uint8_t * uuid,
                                int channel,
                                int * sock_fd,
                                int flags,
                                int callingUid );
} BTSockInterface_t;

const BTSockInterface_t * BT_GetSockInterface( void );

#endif /* _BT_HAL_SOCK_H_ */
/** @} */
