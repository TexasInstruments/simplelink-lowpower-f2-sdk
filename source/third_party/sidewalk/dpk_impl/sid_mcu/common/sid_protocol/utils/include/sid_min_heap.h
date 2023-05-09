/*
 * Copyright 2022 Amazon.com, Inc. or its affiliates. All rights reserved.
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
#ifndef SID_MIN_HEAP_H
#define SID_MIN_HEAP_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef bool (*sid_minheap_comp_func_t)(void *, void *);

struct sid_min_heap {
    void    **heap_a;
    uint32_t heap_count;
    uint32_t max_heap_count;
    sid_minheap_comp_func_t heap_comp_func;
};

#define SID_MINHEAP_INITIALIZER(comp_func)              \
        (struct sid_min_heap) {                         \
        .heap_count = 0,                               \
        .heap_comp_func = comp_func,                   \
        }

#define SID_MINHEAP_HEAD(name, type, comp_func)        \
        struct sid_min_heap name = SID_MINHEAP_INITIALIZER(comp_func)

#define SID_MINHEAP_BUFSZ(node_count) ((node_count + 1) * sizeof(void *))

#define SID_MINHEAP_INIT(head, node_count, heap_buffer)\
        sid_min_heap_init(&head, node_count, heap_buffer)

#define SID_MINHEAP_DEINIT(head)                       \
        sid_min_heap_deinit(&head)

#define SID_MINHEAP_PEEK(head, n)                         \
        sid_min_heap_peek(&head, n)

#define SID_MINHEAP_REMOVE(head)       \
        sid_min_heap_remove(&head, head.heap_comp_func)

#define SID_MINHEAP_REMOVE_NODE(head, node_idx)      \
        sid_min_heap_remove_node(&head, node_idx, head.heap_comp_func)

#define SID_MINHEAP_INSERT(head, node)  \
        sid_min_heap_insert(&head, node, head.heap_comp_func)

#define SID_MINHEAP_RESET(head)         \
        sid_min_heap_clean_up(&head)

/**
 * Min heap insert function.
 *
 * This inserts the node in descending order of node value.
 *
 * @param[in]  heap      Pointer to the head of min heap
 * @param[in]  node      Pointer to the node entry
 * @param[in]  comp_func Function pointer to the comparator function
 *
 * @returns false if the heap is full else true
 */
bool sid_min_heap_insert(struct sid_min_heap *heap, void *node, sid_minheap_comp_func_t comp_func);

/**
 * Min heap remove function.
 *
 * This removes the min node in heap. After removal the trickle down
 * operation is done.
 *
 * @param[in] heap      Pointer to the head of min heap
 * @param[in] comp_func Function pointer to the comparator function
 *
 * @returns the min node from the heap
 */
void* sid_min_heap_remove(struct sid_min_heap *heap, sid_minheap_comp_func_t comp_func);

/**
 * Min heap remove node function.
 *
 * This removes a specific node in heap.
 *
 * @param[in] heap      Pointer to the head of min heap
 * @param[in] node_idx  Index of removed node
 * @param[in] comp_func Function pointer to the comparator function
 *
 * @returns the min node from the heap
 */
void *sid_min_heap_remove_node(struct sid_min_heap *heap, uint32_t node_idx, sid_minheap_comp_func_t comp_func);

/**
 * Min heap init function.
 *
 * This initializes min heap array to the number of heap elements &
 * assigns heap buffer.
 *
 * @param[in] heap         Pointer to the head of min heap
 * @param[in] node_count   Max number of elements in heap
 * @param[in] minheap_buf  Buffer allocated by the user to accomodate
 *                         min heap array of node_count
 *
 * @returns success if the heap was successfully initialized.
 */
bool sid_min_heap_init(struct sid_min_heap *heap, uint32_t node_count, void *minheap_buf);

/**
 * Min heap deinit function.
 *
 * This sets min heap array size to 0.
 *
 * @param[in] heap   Pointer to the head of min heap
 *
 * @returns the min heap buffer
 */
void* sid_min_heap_deinit(struct sid_min_heap *heap);

/**
 * Min heap reset function.
 *
 * @param[in] heap head
 */
void sid_min_heap_clean_up(struct sid_min_heap *heap);

/**
 * Function to peek the N_th element from the head.
 *
 * @param[in] heap  Pointer to the head of min heap
 * @param[in] n     int N_th element: 0 points to head.
 *
 * @returns N_th element pointer
 */
void* sid_min_heap_peek(struct sid_min_heap *heap, uint32_t n);

#ifdef __cplusplus
}
#endif
#endif
