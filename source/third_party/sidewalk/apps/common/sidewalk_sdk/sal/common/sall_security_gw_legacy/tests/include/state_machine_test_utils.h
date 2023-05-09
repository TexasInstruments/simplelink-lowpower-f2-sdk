/*
* Copyright 2022 Amazon.com, Inc. or its affiliates.  All rights reserved.
*
* AMAZON PROPRIETARY/CONFIDENTIAL
*
* You may not use this file except in compliance with the terms and conditions
* set forth in the accompanying LICENSE.TXT file.
*
* THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
* DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
* IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
*/

#ifndef STATE_MACHINE_TEST_UTILS_H
#define STATE_MACHINE_TEST_UTILS_H

#include <sid_pal_assert_ifc.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BUFFER_SIZE 256

struct dbuf {
    uint8_t raw[BUFFER_SIZE];
    size_t len;
};

static inline void dbuf_init(struct dbuf *const db)
{
    memset(db->raw, 0, sizeof(db->raw));
    db->len = 0;
}

static inline void dbuf_set(struct dbuf *const db, uint8_t val)
{
    memset(db->raw, val, db->len);
}

static inline size_t sizeof_dbuf(const struct dbuf *const db)
{
    return sizeof(db->raw);
}

static inline size_t dbuf_len(const struct dbuf *const db)
{
    return db->len;
}

static inline uint8_t *dbuf_offset_ptr(const struct dbuf *const db, size_t offset)
{
    SID_PAL_ASSERT(offset < sizeof_dbuf(db));
    return (uint8_t *)&db->raw[offset];
}

static inline uint8_t *dbuf_len_offset_ptr(struct dbuf *const db)
{
    return dbuf_offset_ptr(db, dbuf_len(db));
}

static inline uint8_t *dbuf_zero_offset_ptr(const struct dbuf *const db)
{
    return dbuf_offset_ptr(db, 0);
}

static inline size_t dbuf_len_left(const struct dbuf *const db)
{
    return sizeof_dbuf(db) - dbuf_len(db);
}

static inline void dbuf_set_len(struct dbuf *const db, size_t len)
{
    SID_PAL_ASSERT(len < sizeof_dbuf(db));
    db->len = len;
}

static inline bool is_dbuf_equal(const struct dbuf *const db1, const struct dbuf *const db2)
{
    return dbuf_len(db1) == dbuf_len(db2) && !memcmp(db1->raw, db2->raw, dbuf_len(db1));
}

static inline void copy_dbuf(struct dbuf *const dst, const struct dbuf *const src)
{
   SID_PAL_ASSERT(dbuf_len(src) <= dbuf_len_left(dst));
   memcpy(dbuf_len_offset_ptr(dst), dbuf_zero_offset_ptr(src), dbuf_len(src));
   dbuf_set_len(dst, dbuf_len(dst) + dbuf_len(src));
}

static inline void copy_dbuf_from_offset(struct dbuf *const dst, const struct dbuf *const src, size_t from_offset)
{
    SID_PAL_ASSERT(from_offset < dbuf_len(src));
    const size_t size_to_copy = dbuf_len(src) - from_offset;
    memcpy(dbuf_len_offset_ptr(dst), dbuf_offset_ptr(src, from_offset), size_to_copy);
    dbuf_set_len(dst, dbuf_len(dst) + size_to_copy);
}

static inline void copy_raw_to_dbuf(struct dbuf *const dst, const uint8_t *const src, size_t src_len)
{
    SID_PAL_ASSERT(src_len < dbuf_len_left(dst));
    memcpy(dbuf_len_offset_ptr(dst), src, src_len);
    dbuf_set_len(dst, dbuf_len(dst) + src_len);
}

static inline void copy_dbuf_offset_to_raw(const struct dbuf *const src, size_t offset, uint8_t *const dst, size_t dst_len)
{
    SID_PAL_ASSERT(offset <= dbuf_len(src));
    const size_t copy_len = dbuf_len(src) - offset < dst_len ? dbuf_len(src) - offset : dst_len;
    memcpy(dst, dbuf_offset_ptr(src, offset), copy_len);
}

#ifdef __cplusplus
}
#endif

#endif /*! STATE_MACHINE_TEST_UTILS_H */
