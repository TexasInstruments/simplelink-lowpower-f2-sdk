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

#ifndef SID_PAL_SERIAL_IFC_H
#define SID_PAL_SERIAL_IFC_H


#include <sid_error.h>

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @struct sid_pal_serial_callbacks_t
 * @brief Defines types of callbacks.
 *
 * The one of runtime parameters' elements has such type. It is used to get externally
 * defined callbacks which would be used in interface's methods.
 */
typedef struct {

    /**
     * Declaration of function type. The function itself is callback which can be called by
     * inteface's methods. This callback should be defined outside a module which defines
     * implementation for this interface. Most likely it would be defined in the module which
     * uses implementation of the interface.
     * Callback is called when transmission by means of implementation is finished.
     *
     * @param [in] user_ctx   -   context, it is argument which has unique values to the module
     *                            which defines this callback.
     *
     * @retval SID_ERROR_NONE -   in case of no error occurred.
     */
    sid_error_t (*tx_done_cb)(void * user_ctx);

    /* TODO: Delete "rx_done_cb" as soon as all implementations of serial client interface
       are reworked and HTP (Host Transport Protocol) has removed from there. */
    /**
     * Declaration of pointer to function. The function itself is callback which can be called by
     * interface's methods. This callback should be defined outside a module which defines
     * implementation for this interface. Most likely it would be defined in the module which
     * uses implementation of the interface.
     * Callback is called when implementation has finished reception of data by its means.
     *
     * @param [in] user_ctx             -   context, it is argument which has unique values to the
     *                                      module which defines this callback.
     *
     * @param [in] buffer_received      -   buffer with data values which were received by means of
     *                                      implementation.
     *
     * @param [in] buffer_size          -   size of received buffer in bytes.
     *
     * @retval SID_ERROR_NONE           -   in case of no error occurred or if you want to skip incoming msg.
     * @retval SID_ERROR_OOM            -   if there is no memory to store incoming msg.
     * If callback returns other errors the nack can be generated or msg would be skipped. It depends on
     * implementation.
     */
    sid_error_t (*rx_done_cb)(void * user_ctx, const uint8_t * buffer_received, size_t buffer_size);

    /*TODO: Rename "new_rx_done_cb" to "rx_done_cb". */
    /**
     * Notifies that some data has been received and can be fetched by method @ref sid_pal_serial_ifc_s::get_frame
     *
     * @param [in] user_ctx             - context, which was passed to create function
     *                                    in @ref sid_pal_serial_params_t::user_ctx
     */
    sid_error_t (*new_rx_done_cb)(void * user_ctx);

} sid_pal_serial_callbacks_t;


/**
 * @struct sid_pal_serial_params_t
 * @brief This structure type defines all configurations/values for specific "instance"
 *        which could be initialized/derived in runtime.
 *
 * @note It will be extended when more functionality is added to implementation.
 */
typedef struct {
/* TODO: remove "params_ctx" as soon as all implementations of serial client interface
   are reworked and HTP (Host Transport Protocol) has removed from there. */
    void                             * params_ctx; /**< Runtime parameters for specific implementation. Its type defined in
                                                       implementation so create function can cast it to that type. */

    void                             * user_ctx;   /**< External parameters to pass in callbacks. The implementation doesn't
                                                        need to know the type and content of it, since callbacks defined
                                                        outside it and most likely in the same module as context.*/

    const sid_pal_serial_callbacks_t * callbacks;  /**< Pointer to constant structure with defined callbacks. Callbacks will
                                                        be used in methods.*/
} sid_pal_serial_params_t;


typedef const struct sid_pal_serial_ifc_s * sid_pal_serial_ifc_t;

/**
 *  @struct sid_pal_serial_ifc_s
 *  @brief Declaration of pointers to serial client interface's methods.
 *
 *  @var sid_pal_serial_ifc_s:send
 *  Pointer to method which sends data to host.
 *
 *  @var sid_pal_serial_ifc_s:process
 *  Pointer to method which calls standard routine of specific implementation to deal with connection.
 *
 *  @var sid_pal_serial_ifc_s:destroy
 *  Pointer to method which is a destructor for instance. It clear memory which were reserved by instance.
 */
struct sid_pal_serial_ifc_s {

    /**
     * Pointer to implementation-depended method which send data in "buffer_to_send" with
     * size "buffer_size" over serial connection to host.
     *
     * @param [in] _this            -   pointer to interface which is a part of implementation instance.
     * @param [in] buffer_to_send   -   array of data to send over.
     * @param [in] buffer_size      -   size of the buffer to send.
     *
     * @return SID_ERROR_NONE - in case method finished with success.
     *         In case of error the error's type depend upon implementation.
     */
    sid_error_t (*send)(const sid_pal_serial_ifc_t *_this, const uint8_t *frame_to_send, size_t frame_size);

    /**
     * Fetches frame if one was received. It is advisable to copy it instantaneously, since there is no
     * guaranty that frame won't be corrupted or erased after call of this function finished.
     *
     * @param [in] _this            -   pointer to serial client interface itself. The one which
     *                                  calls this method.
     * @param [out] frame_received  -   returns pointer to received frame.
     * @param [out] frame_size      -   returns pointer to received frame size.
     *
     * @retval SID_ERROR_NONE - in case method finished with success.
     * @retval SID_ERROR_NULL_POINTER - in case some of function parameters is null pointer.
     * @return Any other error possible if function not succeeded to execute properly.
     */
    sid_error_t (*get_frame)(const sid_pal_serial_ifc_t *_this, uint8_t **frame_received, size_t *frame_size);

    /* TODO: Remove "process" method as soon as all implementations of serial client interface
       are reworked and HTP (Host Transport Protocol) has removed from there. */
    /**
     * Pointer to implementation-depended method which executes routine to sustain connection with the host.
     *
     * @param [in] _this            -   pointer to interface which is a part of implementation instance.
     *
     * @retval SID_ERROR_NONE       - in case of success.
     *         In case of error the error's type depend upon implementation.
     */
    sid_error_t (*process)(const sid_pal_serial_ifc_t * _this);

    /**
     * Pointer to implementation-depended method. It retrieves mtu (maximum transmit unit).
     *
     * @param [in] _this        -   pointer to interface which is a part of implementation instance.
     *
     * @param [out] mtu         -   method puts retrieved value of mtu into this argument.
     *
     * @retval SID_ERROR_NONE   -   in case of success
     *         In case of error the error's type depend upon implementation.
     */
    sid_error_t (*get_mtu)(const sid_pal_serial_ifc_t * _this, uint16_t * mtu);

    /**
     * Pointer to implementation-depended method which free memory taken by specific instance.
     *
     * @param [in] _this            -   pointer to interface which is a part of implementation instance.
     */
    void (*destroy)(const sid_pal_serial_ifc_t * _this);
};


/**
 * @struct sid_pal_serial_factory_t
 * @brief Generic type for instance factory.
 */
typedef struct {
    /**
     * General type for constructors of this interface. It's advised to use it.
     *
     * @param [out] _this               -   User has to pass pointer to pointer (most likely to nullptr) which
     *                                      he wants to point on interface of specific implementation instance.
     *                                      It will be used to identify instance in methods by passing it
     *                                      as first argument to them.
     *
     * @param [in] config               -   pointer to constant configurations which is defined before execution of code.
     *                                      It is void pointer since configurations implementation-depended.
     *
     * @param [in] params               -   Pointer to structure with parameters which can be defined in the runtime.
     */
    sid_error_t (*sid_pal_serial_client_create)(sid_pal_serial_ifc_t const ** _this, const void * config, sid_pal_serial_params_t const * params);

    /**
     * The configuration to be passed in sid_pal_serial_create.
     */
    const void * config;
} sid_pal_serial_client_factory_t;

#ifdef __cplusplus
} // extern "C"
#endif


#endif /* SID_PAL_SERIAL_IFC_H */
