/*
 * Copyright 2020-2021 Amazon.com, Inc. or its affiliates. All rights reserved.
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

#ifndef LIB_HALO_INCLUDE_EXPORT_HALO_SUBSCRIPTION_LIST_H_
#define LIB_HALO_INCLUDE_EXPORT_HALO_SUBSCRIPTION_LIST_H_

#include <stdint.h>
#include <stdbool.h>

#include <halo/error.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef void * halo_subscription_key_t;
typedef void * user_ctx_t;
typedef void * predicate_ctx_t;

/**
 * return 0 for Equal values; -1 - if key_1 < key_2 and 1 if key_1 > key_2
 */
typedef int  (* predicate_t)(predicate_ctx_t predicate_ctx, const halo_subscription_key_t key_1, const halo_subscription_key_t key_2);

typedef halo_error_t (* halo_subscriber_notify_t)(user_ctx_t ctx, const halo_subscription_key_t key, void * arg);

typedef struct {
  user_ctx_t                            ctx;
  halo_subscriber_notify_t              cb;
} halo_subscriber_t;

typedef struct {
    halo_subscriber_t                   subscriber;
    halo_subscription_key_t             key;
} halo_subscriber_entry_t;

typedef enum {
    SUBSCRIPTION_MULTIPLE = 0x00,
    SUBSCRIPTION_UNIQUE = 0x01
}halo_subscription_type;

typedef struct {
    predicate_t                         predicate_cb;
    predicate_ctx_t                     predicate_ctx;
    halo_subscriber_entry_t *           entries;
    uint32_t                            count_subscribers;
    uint32_t                            count_used;
    halo_subscription_type              subscription_type;
    bool                                realloc_enabled;
} halo_subscription_list_t;

typedef struct {
    predicate_t                         predicate_cb;
    predicate_ctx_t                     predicate_ctx;
    uint32_t                            count_subscribers;
    halo_subscription_type              subscription_type;
    bool                                realloc_enabled;
} halo_subscription_list_param_t;

/**
 * Function constructor.
 * @param ctx - instance of subscription_list
 * @param param - configs of creating subscription list
 * @return error code, @see halo_error_t
 */
halo_error_t halo_subscription_list_create(halo_subscription_list_t * ctx, const halo_subscription_list_param_t * param);

/**
 * Function destructor.
 * @param ctx - instance of subscription_list
 * @return error code, @see halo_error_t
 */
void         halo_subscription_list_destroy(halo_subscription_list_t * ctx);

/**
 * Function subscribes the subscriber for notification of callback request.
 * @param ctx - instance of subscription_list
 * @param key - subscription key
 * @param entry_user - reference to the subscriber
 * @param arg - argument to pass into the callback function
 * @return error code, @see halo_error_t
 */
halo_error_t halo_subscription_list_subscribe(halo_subscription_list_t * ctx, const halo_subscriber_t * entry_user, const halo_subscription_key_t key);

/**
 * Function unsubscribes the subscriber for notification of callback request.
 * @param ctx - instance of subscription_list
 * @param key - subscription key
 * @param entry_user - reference to the subscriber
 * @param arg - argument to pass into the callback function
 * @return error code, @see halo_error_t
 */
halo_error_t halo_subscription_list_unsubscribe(halo_subscription_list_t * ctx, const halo_subscriber_t * entry_user, const halo_subscription_key_t key);

/**
 * Function calls the subscriber's callback for each subscriber for the specified key.
 * The results of the subscriber's callback will be ignored.
 * @param ctx - instance of subscription_list
 * @param key - subscription key
 * @param arg - argument to pass into the callback function
 * @return error code of internal error, @see halo_error_t
 */
halo_error_t halo_subscription_list_notify(halo_subscription_list_t * ctx, const halo_subscription_key_t key, void * arg);

/**
 * Function calls the subscriber's callback and returns the result.
 * This function makes sense only if subscription_list was configured
 * for a single subscriber per key, otherwise "HALO_ERROR_NOSUPPORT" will be returned.
 * @param ctx - instance of subscription_list
 * @param key - subscription key
 * @param arg - argument to pass into the callback function
 * @param res - result returned by user callback
 * @return error code, @see halo_error_t
 */
halo_error_t halo_subscription_list_request(halo_subscription_list_t * ctx, const halo_subscription_key_t key, void * arg, halo_error_t* res);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* LIB_HALO_INCLUDE_EXPORT_HALO_SUBSCRIPTION_LIST_H_ */
