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

#ifndef _H_SRC_MAC_HALO_INCLUDE_RNET_MAC_UTIL_H
#define _H_SRC_MAC_HALO_INCLUDE_RNET_MAC_UTIL_H

#include <freebsd/queue.h>
#include <sid_protocol_defs.h>

#include <stddef.h>

#define NODE_ENTRY slistq_entry

/**
 * @brief Macro to declare FIFO-Q using bsd STAILQ.
 * @param fifo_head: the name of the fifo queue
 * @param fifo_type: the type of elements in the fifo queue
 */
#define RNET_FIFO_HEAD(fifo_head, fifo_type)                            \
    static STAILQ_HEAD(SID_UTIL_CONCAT(fifo_head, _queue), fifo_type)   \
                    fifo_head = STAILQ_HEAD_INITIALIZER(fifo_head);     \
    struct SID_UTIL_CONCAT(fifo_head, _queue) * SID_UTIL_CONCAT(fifo_head, _p)

/**
 * @brief Macro to declare list using bsd STAILQ.
 * @param type: the type of elements in the fifo queue
 */
#define RNET_LIST_ENTRY(type)      STAILQ_ENTRY(type) NODE_ENTRY

/**
 * @brief Macro to return the first element in the fifo
 * @param fifo_head: the head of fifo
 */
#define RNET_FIFO_PEEK(fifo_head)       STAILQ_FIRST(&fifo_head)

/**
 * @brief Macro to add an element in the fifo
 * @param fifo_head: the head of fifo
 * @param fifo_entry: the element to be added to the fifo
 */
#define RNET_FIFO_PUSH(fifo_head, fifo_entry)   STAILQ_INSERT_TAIL(&fifo_head, fifo_entry, NODE_ENTRY)

/**
 * @brief Macro to pop the first element in the fifo
 * @param fifo_head: the head of fifo
 */
#define RNET_FIFO_POP(fifo_head) ({                     \
    void *elem = STAILQ_FIRST(&fifo_head);              \
    if (elem != NULL) {                                 \
        STAILQ_REMOVE_HEAD(&fifo_head, NODE_ENTRY);     \
    }                                                   \
    elem;                                               \
})

/**
 * @brief Macro to get the next element in the fifo
 * @param fifo_head: the head of fifo
 * @param fifo_ele: current fifo_element
 */
#define RNET_FIFO_NEXT(fifo_head, fifo_ele)     STAILQ_NEXT(fifo_ele, NODE_ENTRY)

/**
 * @brief Macro to remove an element in the fifo
 * @param fifo_head: the head of sortlist queue
 * @param fifo_ele: the element that must be removed from fifo
 * @param type: type of element being removed
 */
#define RNET_FIFO_REMOVE(fifo_head, fifo_ele, type) ({                  \
        struct type *p, *temp;                                          \
        bool ret = false;                                               \
        STAILQ_FOREACH_SAFE(p, &fifo_head, NODE_ENTRY, temp) {          \
            if (p == fifo_ele) {                                        \
                ret = true;                                             \
                STAILQ_REMOVE(&fifo_head, fifo_ele, type, NODE_ENTRY);  \
            }                                                           \
        }                                                               \
        ret;                                                            \
})

/* wrapper for SORTED LIST implementation */

/**
 * @brief Macro to declare sortlist using nrf sortlist.
 * @param sortlist_head: the name of the fifo queue
 * @param sortlist_type: the type of elements in the fifo queue
 * @param comp_func: the callback function for element comparision
 */
#define RNET_SORTLIST_HEAD(sortlist_head, sortlist_type, comp_func)  \
        RNET_FIFO_HEAD(sortlist_head, sortlist_type)

#define RNET_SORTLIST_ENTRY_T       void

/**
 * @brief Macro to peek the first element in the sortlist
 * @param sortlist_head: the head of sortlist sortlist
 */
#define RNET_SORTLIST_PEEK(sortlist_head)       RNET_FIFO_PEEK(sortlist_head)

/**
 * @brief Macro to get the next element in the sortlist
 * @param sortlist_head: the head of sortlist
 * @param sortlist_ele: current sortlist_element
 */
#define RNET_SORTLIST_NEXT(sortlist_head, sortlist_ele)     RNET_FIFO_NEXT(sortlist_head, sortlist_ele)

/**
 * @brief Macro to add an element in the sortlist
 * @param sortlist_head: the head of sortlist
 * @param sortlist_ele: the element to be added to the sortlist
 */
#define RNET_SORTLIST_ADD(sortlist_head, sortlist_ele, type, compare_func) do {      \
        struct type *p = NULL;                                                       \
        struct type *prev = NULL;                                                    \
        STAILQ_FOREACH(p, &sortlist_head, NODE_ENTRY) {                              \
            if (!compare_func(&((p)->NODE_ENTRY), &((sortlist_ele)->NODE_ENTRY))) {  \
                break;                                                               \
            }                                                                        \
            prev = p;                                                                \
        }                                                                            \
        if (prev == NULL) {                                                          \
            STAILQ_INSERT_HEAD(&sortlist_head, sortlist_ele, NODE_ENTRY);            \
        } else {                                                                     \
            STAILQ_INSERT_AFTER(&sortlist_head, prev, sortlist_ele, NODE_ENTRY);     \
        }                                                                            \
} while (0)

/**
 * @brief Macro to pop the first element in the sortlist
 * @param sortlist_head: the head of sortlist
 */
#define RNET_SORTLIST_POP(sortlist_head)        RNET_FIFO_POP(sortlist_head)

/**
 * @brief Macro to remove an element in the sortlist
 * @param sortlist_head: the head of sortlist
 * @param sortlist_ele: the element that must be removed from sortlist
 * @param type: type of element being removed
 */
#define RNET_SORTLIST_REMOVE(sortlist_head, sortlist_ele, type) \
        RNET_FIFO_REMOVE(sortlist_head, sortlist_ele, type)

/**
 * @brief Macro to get the container of a list item
 * @param sortlist_ele: element in sortlist queue
 * @param sortlist_type: the type of element in sortlist queue
 */
#define RNET_LIST_CONTAINER_OF(sortlist_ele, sortlist_type) \
                                SID_UTIL_CONTAINER_OF(sortlist_ele, sortlist_type, NODE_ENTRY)

#define RNET_INIT_ENTRY         { NULL }

#endif //_H_SRC_MAC_HALO_INCLUDE_RNET_MAC_UTIL_H
