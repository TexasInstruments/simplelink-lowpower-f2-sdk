/*
 *    Copyright (c) 2016, The OpenThread Authors.
 *    All rights reserved.
 *
 *    Redistribution and use in source and binary forms, with or without
 *    modification, are permitted provided that the following conditions are met:
 *    1. Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *    3. Neither the name of the copyright holder nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 *    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 *    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
 *    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  -------------------------------------------------------------------
 *
 *  ## Unit Test ##
 *
 *  This file includes its own unit test. To compile the unit test,
 *  simply compile this file with the macro SPINEL_SELF_TEST set to 1.
 *  For example:
 *
 *      cc spinel.c -Wall -DSPINEL_SELF_TEST=1 -o spinel
 *
 *  -------------------------------------------------------------------
 */

// ----------------------------------------------------------------------------
// MARK: -
// MARK: Headers

#include "spinel.h"

#include <errno.h>
#include <limits.h>

#ifndef SPINEL_PLATFORM_HEADER
/* These are all already included in the spinel platform header
 * if SPINEL_PLATFORM_HEADER was defined.
 */
#include <stdio.h>
#include <stdlib.h>
#include "utils/wrap_string.h"
#endif // #ifndef SPINEL_PLATFORM_HEADER

// ----------------------------------------------------------------------------
// MARK: -

// IAR's errno.h apparently doesn't define EOVERFLOW.
#ifndef EOVERFLOW
// There is no real good choice for what to set
// errno to in this case, so we just pick the
// value '1' somewhat arbitrarily.
#define EOVERFLOW 1
#endif

// IAR's errno.h apparently doesn't define EINVAL.
#ifndef EINVAL
// There is no real good choice for what to set
// errno to in this case, so we just pick the
// value '1' somewhat arbitrarily.
#define EINVAL 1
#endif

// IAR's errno.h apparently doesn't define ENOMEM.
#ifndef ENOMEM
// There is no real good choice for what to set
// errno to in this case, so we just pick the
// value '1' somewhat arbitrarily.
#define ENOMEM 1
#endif

#ifndef SPINEL_PLATFORM_SHOULD_LOG_ASSERTS
#define SPINEL_PLATFORM_SHOULD_LOG_ASSERTS 0
#endif

#ifndef SPINEL_PLATFORM_DOESNT_IMPLEMENT_ERRNO_VAR
#define SPINEL_PLATFORM_DOESNT_IMPLEMENT_ERRNO_VAR 0
#endif

#ifndef SPINEL_PLATFORM_DOESNT_IMPLEMENT_FPRINTF
#define SPINEL_PLATFORM_DOESNT_IMPLEMENT_FPRINTF 0
#endif

#ifndef SPINEL_SELF_TEST
#define SPINEL_SELF_TEST 0
#endif

#if defined(errno) && SPINEL_PLATFORM_DOESNT_IMPLEMENT_ERRNO_VAR
#error "SPINEL_PLATFORM_DOESNT_IMPLEMENT_ERRNO_VAR is set but errno is already defined."
#endif

// Work-around for platforms that don't implement the `errno` variable.
#if !defined(errno) && SPINEL_PLATFORM_DOESNT_IMPLEMENT_ERRNO_VAR
static int spinel_errno_workaround_;
#define errno spinel_errno_workaround_
#endif // SPINEL_PLATFORM_DOESNT_IMPLEMENT_ERRNO_VAR

#ifndef assert_printf
#if SPINEL_PLATFORM_DOESNT_IMPLEMENT_FPRINTF
#define assert_printf(fmt, ...) printf(__FILE__ ":%d: " fmt "\n", __LINE__, __VA_ARGS__)
#else // if SPINEL_PLATFORM_DOESNT_IMPLEMENT_FPRINTF
#define assert_printf(fmt, ...) fprintf(stderr, __FILE__ ":%d: " fmt "\n", __LINE__, __VA_ARGS__)
#endif // else SPINEL_PLATFORM_DOESNT_IMPLEMENT_FPRINTF
#endif

#ifndef require_action
#if SPINEL_PLATFORM_SHOULD_LOG_ASSERTS
#define require_action(c, l, a)                           \
    do                                                    \
    {                                                     \
        if (!(c))                                         \
        {                                                 \
            assert_printf("Requirement Failed (%s)", #c); \
            a;                                            \
            goto l;                                       \
        }                                                 \
    } while (0)
#else // if DEBUG
#define require_action(c, l, a) \
    do                          \
    {                           \
        if (!(c))               \
        {                       \
            a;                  \
            goto l;             \
        }                       \
    } while (0)
#endif // else DEBUG
#endif // ifndef require_action

#ifndef require
#define require(c, l) require_action(c, l, {})
#endif

#ifndef strnlen
static size_t spinel_strnlen(const char *s, size_t maxlen)
{
    size_t ret;

    for (ret = 0; (ret < maxlen) && (s[ret] != 0); ret++)
    {
        // Empty loop.
    }

    return ret;
}
#else
#define spinel_strnlen strnlen
#endif

typedef struct
{
    va_list obj;
} va_list_obj;

#define SPINEL_MAX_PACK_LENGTH 32767

// ----------------------------------------------------------------------------

// This function validates whether a given byte sequence (string) follows UTF8 encoding.
static bool spinel_validate_utf8(const uint8_t *string)
{
    bool    ret = true;
    uint8_t byte;
    uint8_t continuation_bytes = 0;

    while ((byte = *string++) != 0)
    {
        if ((byte & 0x80) == 0)
        {
            continue;
        }

        // This is a leading byte 1xxx-xxxx.

        if ((byte & 0x40) == 0) // 10xx-xxxx
        {
            // We got a continuation byte pattern without seeing a leading byte earlier.
            ret = false;
            goto bail;
        }
        else if ((byte & 0x20) == 0) // 110x-xxxx
        {
            continuation_bytes = 1;
        }
        else if ((byte & 0x10) == 0) // 1110-xxxx
        {
            continuation_bytes = 2;
        }
        else if ((byte & 0x08) == 0) // 1111-0xxx
        {
            continuation_bytes = 3;
        }
        else // 1111-1xxx  (invalid pattern).
        {
            ret = false;
            goto bail;
        }

        while (continuation_bytes-- != 0)
        {
            byte = *string++;

            // Verify the continuation byte pattern 10xx-xxxx
            if ((byte & 0xc0) != 0x80)
            {
                ret = false;
                goto bail;
            }
        }
    }

bail:
    return ret;
}

// ----------------------------------------------------------------------------
// MARK: -

spinel_ssize_t spinel_packed_uint_decode(const uint8_t *bytes, spinel_size_t len, unsigned int *value_ptr)
{
    spinel_ssize_t ret   = 0;
    unsigned int   value = 0;

    unsigned int i = 0;

    do
    {
        if ((len < sizeof(uint8_t)) || (i >= sizeof(unsigned int) * CHAR_BIT))
        {
            ret = -1;
            break;
        }

        value |= (unsigned int)(bytes[0] & 0x7F) << i;
        i += 7;
        ret += sizeof(uint8_t);
        bytes += sizeof(uint8_t);
        len -= sizeof(uint8_t);
    } while ((bytes[-1] & 0x80) == 0x80);

    if ((ret > 0) && (value_ptr != NULL))
    {
        *value_ptr = value;
    }

    return ret;
}

spinel_ssize_t spinel_packed_uint_size(unsigned int value)
{
    spinel_ssize_t ret;

    if (value < (1 << 7))
    {
        ret = 1;
    }
    else if (value < (1 << 14))
    {
        ret = 2;
    }
    else if (value < (1 << 21))
    {
        ret = 3;
    }
    else if (value < (1 << 28))
    {
        ret = 4;
    }
    else
    {
        ret = 5;
    }

    return ret;
}

spinel_ssize_t spinel_packed_uint_encode(uint8_t *bytes, spinel_size_t len, unsigned int value)
{
    const spinel_ssize_t encoded_size = spinel_packed_uint_size(value);

    if ((spinel_ssize_t)len >= encoded_size)
    {
        spinel_ssize_t i;

        for (i = 0; i != encoded_size - 1; ++i)
        {
            *bytes++ = (value & 0x7F) | 0x80;
            value    = (value >> 7);
        }

        *bytes++ = (value & 0x7F);
    }

    return encoded_size;
}

const char *spinel_next_packed_datatype(const char *pack_format)
{
    int depth = 0;

    do
    {
        switch (*++pack_format)
        {
        case '(':
            depth++;
            break;

        case ')':
            depth--;

            if (depth == 0)
            {
                pack_format++;
            }

            break;
        }
    } while ((depth > 0) && *pack_format != 0);

    return pack_format;
}

static spinel_ssize_t spinel_datatype_vunpack_(bool           in_place,
                                               const uint8_t *data_in,
                                               spinel_size_t  data_len,
                                               const char *   pack_format,
                                               va_list_obj *  args)
{
    spinel_ssize_t ret = 0;

    // Buffer length sanity check
    require_action(data_len <= SPINEL_MAX_PACK_LENGTH, bail, (ret = -1, errno = EINVAL));

    for (; *pack_format != 0; pack_format = spinel_next_packed_datatype(pack_format))
    {
        if (*pack_format == ')')
        {
            // Don't go past the end of a struct.
            break;
        }

        switch ((spinel_datatype_t)pack_format[0])
        {
        case SPINEL_DATATYPE_BOOL_C:
        {
            bool *arg_ptr = va_arg(args->obj, bool *);
            require_action(data_len >= sizeof(uint8_t), bail, (ret = -1, errno = EOVERFLOW));

            if (arg_ptr)
            {
                *arg_ptr = data_in[0] != 0;
            }

            ret += sizeof(uint8_t);
            data_in += sizeof(uint8_t);
            data_len -= sizeof(uint8_t);
            break;
        }

        case SPINEL_DATATYPE_INT8_C:
        case SPINEL_DATATYPE_UINT8_C:
        {
            uint8_t *arg_ptr = va_arg(args->obj, uint8_t *);
            require_action(data_len >= sizeof(uint8_t), bail, (ret = -1, errno = EOVERFLOW));

            if (arg_ptr)
            {
                *arg_ptr = data_in[0];
            }

            ret += sizeof(uint8_t);
            data_in += sizeof(uint8_t);
            data_len -= sizeof(uint8_t);
            break;
        }

        case SPINEL_DATATYPE_INT16_C:
        case SPINEL_DATATYPE_UINT16_C:
        {
            uint16_t *arg_ptr = va_arg(args->obj, uint16_t *);
            require_action(data_len >= sizeof(uint16_t), bail, (ret = -1, errno = EOVERFLOW));

            if (arg_ptr)
            {
                *arg_ptr = (uint16_t)((data_in[1] << 8) | data_in[0]);
            }

            ret += sizeof(uint16_t);
            data_in += sizeof(uint16_t);
            data_len -= sizeof(uint16_t);
            break;
        }

        case SPINEL_DATATYPE_INT32_C:
        case SPINEL_DATATYPE_UINT32_C:
        {
            uint32_t *arg_ptr = va_arg(args->obj, uint32_t *);
            require_action(data_len >= sizeof(uint32_t), bail, (ret = -1, errno = EOVERFLOW));

            if (arg_ptr)
            {
                *arg_ptr = (uint32_t)((data_in[3] << 24) | (data_in[2] << 16) | (data_in[1] << 8) | data_in[0]);
            }

            ret += sizeof(uint32_t);
            data_in += sizeof(uint32_t);
            data_len -= sizeof(uint32_t);
            break;
        }

        case SPINEL_DATATYPE_INT64_C:
        case SPINEL_DATATYPE_UINT64_C:
        {
            uint64_t *arg_ptr = va_arg(args->obj, uint64_t *);
            require_action(data_len >= sizeof(uint64_t), bail, (ret = -1, errno = EOVERFLOW));

            if (arg_ptr)
            {
                uint32_t l32 = (uint32_t)((data_in[3] << 24) | (data_in[2] << 16) | (data_in[1] << 8) | data_in[0]);
                uint32_t h32 = (uint32_t)((data_in[7] << 24) | (data_in[6] << 16) | (data_in[5] << 8) | data_in[4]);

                *arg_ptr = ((uint64_t)l32) | (((uint64_t)h32) << 32);
            }

            ret += sizeof(uint64_t);
            data_in += sizeof(uint64_t);
            data_len -= sizeof(uint64_t);
            break;
        }

        case SPINEL_DATATYPE_IPv6ADDR_C:
        {
            require_action(data_len >= sizeof(spinel_ipv6addr_t), bail, (ret = -1, errno = EOVERFLOW));

            if (in_place)
            {
                spinel_ipv6addr_t *arg = va_arg(args->obj, spinel_ipv6addr_t *);
                if (arg)
                {
                    memcpy(arg, data_in, sizeof(spinel_ipv6addr_t));
                }
            }
            else
            {
                const spinel_ipv6addr_t **arg_ptr = va_arg(args->obj, const spinel_ipv6addr_t **);
                if (arg_ptr)
                {
                    *arg_ptr = (const spinel_ipv6addr_t *)data_in;
                }
            }

            ret += sizeof(spinel_ipv6addr_t);
            data_in += sizeof(spinel_ipv6addr_t);
            data_len -= sizeof(spinel_ipv6addr_t);
            break;
        }

        case SPINEL_DATATYPE_EUI64_C:
        {
            require_action(data_len >= sizeof(spinel_eui64_t), bail, (ret = -1, errno = EOVERFLOW));

            if (in_place)
            {
                spinel_eui64_t *arg = va_arg(args->obj, spinel_eui64_t *);
                if (arg)
                {
                    memcpy(arg, data_in, sizeof(spinel_eui64_t));
                }
            }
            else
            {
                const spinel_eui64_t **arg_ptr = va_arg(args->obj, const spinel_eui64_t **);
                if (arg_ptr)
                {
                    *arg_ptr = (const spinel_eui64_t *)data_in;
                }
            }

            ret += sizeof(spinel_eui64_t);
            data_in += sizeof(spinel_eui64_t);
            data_len -= sizeof(spinel_eui64_t);
            break;
        }

        case SPINEL_DATATYPE_EUI48_C:
        {
            require_action(data_len >= sizeof(spinel_eui48_t), bail, (ret = -1, errno = EOVERFLOW));

            if (in_place)
            {
                spinel_eui48_t *arg = va_arg(args->obj, spinel_eui48_t *);
                if (arg)
                {
                    memcpy(arg, data_in, sizeof(spinel_eui48_t));
                }
            }
            else
            {
                const spinel_eui48_t **arg_ptr = va_arg(args->obj, const spinel_eui48_t **);
                if (arg_ptr)
                {
                    *arg_ptr = (const spinel_eui48_t *)data_in;
                }
            }

            ret += sizeof(spinel_eui48_t);
            data_in += sizeof(spinel_eui48_t);
            data_len -= sizeof(spinel_eui48_t);
            break;
        }

        case SPINEL_DATATYPE_UINT_PACKED_C:
        {
            unsigned int * arg_ptr = va_arg(args->obj, unsigned int *);
            spinel_ssize_t pui_len = spinel_packed_uint_decode(data_in, data_len, arg_ptr);

            // Range check
            require_action(NULL == arg_ptr || (*arg_ptr < SPINEL_MAX_UINT_PACKED), bail, (ret = -1, errno = ERANGE));

            require(pui_len > 0, bail);

            require(pui_len <= (spinel_ssize_t)data_len, bail);

            ret += pui_len;
            data_in += pui_len;
            data_len -= (spinel_size_t)pui_len;
            break;
        }

        case SPINEL_DATATYPE_UTF8_C:
        {
            size_t len;

            // Make sure we have at least one byte.
            require_action(data_len > 0, bail, (ret = -1, errno = EOVERFLOW));

            // Add 1 for zero termination. If not zero terminated,
            // len will then be data_len+1, which we will detect
            // in the next check.
            len = spinel_strnlen((const char *)data_in, data_len) + 1;

            // Verify that the string is zero terminated.
            require_action(len <= data_len, bail, (ret = -1, errno = EOVERFLOW));

            // Verify the string follows valid UTF8 encoding.
            require_action(spinel_validate_utf8(data_in), bail, (ret = -1, errno = EINVAL));

            if (in_place)
            {
                char * arg     = va_arg(args->obj, char *);
                size_t len_arg = va_arg(args->obj, size_t);
                if (arg)
                {
                    require_action(len_arg >= len, bail, (ret = -1, errno = ENOMEM));
                    memcpy(arg, data_in, len);
                }
            }
            else
            {
                const char **arg_ptr = va_arg(args->obj, const char **);
                if (arg_ptr)
                {
                    *arg_ptr = (const char *)data_in;
                }
            }

            ret += (spinel_size_t)len;
            data_in += len;
            data_len -= (spinel_size_t)len;
            break;
        }

        case SPINEL_DATATYPE_DATA_C:
        case SPINEL_DATATYPE_DATA_WLEN_C:
        {
            spinel_ssize_t pui_len       = 0;
            uint16_t       block_len     = 0;
            const uint8_t *block_ptr     = data_in;
            void *         arg_ptr       = va_arg(args->obj, void *);
            unsigned int * block_len_ptr = va_arg(args->obj, unsigned int *);
            char           nextformat    = *spinel_next_packed_datatype(pack_format);

            if ((pack_format[0] == SPINEL_DATATYPE_DATA_WLEN_C) || ((nextformat != 0) && (nextformat != ')')))
            {
                pui_len = spinel_datatype_unpack(data_in, data_len, SPINEL_DATATYPE_UINT16_S, &block_len);
                block_ptr += pui_len;

                require(pui_len > 0, bail);
                require(block_len < SPINEL_FRAME_MAX_SIZE, bail);
            }
            else
            {
                block_len = (uint16_t)data_len;
                pui_len   = 0;
            }

            require_action((spinel_ssize_t)data_len >= (block_len + pui_len), bail, (ret = -1, errno = EOVERFLOW));

            if (in_place)
            {
                require_action(NULL != block_len_ptr && *block_len_ptr >= block_len, bail, (ret = -1, errno = EINVAL));
                memcpy(arg_ptr, block_ptr, block_len);
            }
            else
            {
                const uint8_t **block_ptr_ptr = (const uint8_t **)arg_ptr;
                if (NULL != block_ptr_ptr)
                {
                    *block_ptr_ptr = block_ptr;
                }
            }

            if (NULL != block_len_ptr)
            {
                *block_len_ptr = block_len;
            }

            block_len += (uint16_t)pui_len;
            ret += block_len;
            data_in += block_len;
            data_len -= block_len;
            break;
        }

        case 'T':
        case SPINEL_DATATYPE_STRUCT_C:
        {
            spinel_ssize_t pui_len    = 0;
            uint16_t       block_len  = 0;
            spinel_ssize_t actual_len = 0;
            const uint8_t *block_ptr  = data_in;
            char           nextformat = *spinel_next_packed_datatype(pack_format);

            if ((pack_format[0] == SPINEL_DATATYPE_STRUCT_C) || ((nextformat != 0) && (nextformat != ')')))
            {
                pui_len = spinel_datatype_unpack(data_in, data_len, SPINEL_DATATYPE_UINT16_S, &block_len);
                block_ptr += pui_len;

                require(pui_len > 0, bail);
                require(block_len < SPINEL_FRAME_MAX_SIZE, bail);
            }
            else
            {
                block_len = (uint16_t)data_len;
                pui_len   = 0;
            }

            require_action((spinel_ssize_t)data_len >= (block_len + pui_len), bail, (ret = -1, errno = EOVERFLOW));

            actual_len = spinel_datatype_vunpack_(false, block_ptr, block_len, pack_format + 2, args);

            require_action(actual_len > -1, bail, (ret = -1, errno = EOVERFLOW));

            if (pui_len)
            {
                block_len += (uint16_t)pui_len;
            }
            else
            {
                block_len = (uint16_t)actual_len;
            }

            ret += block_len;
            data_in += block_len;
            data_len -= block_len;
            break;
        }

        case '.':
            // Skip.
            break;

        case SPINEL_DATATYPE_ARRAY_C:
        default:
            // Unsupported Type!
            ret   = -1;
            errno = EINVAL;
            goto bail;
        }
    }

    return ret;

bail:
    return ret;
}

spinel_ssize_t spinel_datatype_unpack_in_place(const uint8_t *data_in,
                                               spinel_size_t  data_len,
                                               const char *   pack_format,
                                               ...)
{
    spinel_ssize_t ret;
    va_list_obj    args;
    va_start(args.obj, pack_format);

    ret = spinel_datatype_vunpack_(true, data_in, data_len, pack_format, &args);

    va_end(args.obj);
    return ret;
}

spinel_ssize_t spinel_datatype_unpack(const uint8_t *data_in, spinel_size_t data_len, const char *pack_format, ...)
{
    spinel_ssize_t ret;
    va_list_obj    args;
    va_start(args.obj, pack_format);

    ret = spinel_datatype_vunpack_(false, data_in, data_len, pack_format, &args);

    va_end(args.obj);
    return ret;
}

spinel_ssize_t spinel_datatype_vunpack_in_place(const uint8_t *data_in,
                                                spinel_size_t  data_len,
                                                const char *   pack_format,
                                                va_list        args)
{
    spinel_ssize_t ret;
    va_list_obj    args_obj;
    va_copy(args_obj.obj, args);

    ret = spinel_datatype_vunpack_(true, data_in, data_len, pack_format, &args_obj);

    va_end(args_obj.obj);
    return ret;
}

spinel_ssize_t spinel_datatype_vunpack(const uint8_t *data_in,
                                       spinel_size_t  data_len,
                                       const char *   pack_format,
                                       va_list        args)
{
    spinel_ssize_t ret;
    va_list_obj    args_obj;
    va_copy(args_obj.obj, args);

    ret = spinel_datatype_vunpack_(false, data_in, data_len, pack_format, &args_obj);

    va_end(args_obj.obj);
    return ret;
}

static spinel_ssize_t spinel_datatype_vpack_(uint8_t *     data_out,
                                             spinel_size_t data_len_max,
                                             const char *  pack_format,
                                             va_list_obj * args)
{
    spinel_ssize_t ret = 0;

    // Buffer length sanity check
    require_action(data_len_max <= SPINEL_MAX_PACK_LENGTH, bail, (ret = -1, errno = EINVAL));

    if (data_out == NULL)
    {
        data_len_max = 0;
    }

    for (; *pack_format != 0; pack_format = spinel_next_packed_datatype(pack_format))
    {
        if (*pack_format == ')')
        {
            // Don't go past the end of a struct.
            break;
        }

        switch ((spinel_datatype_t)*pack_format)
        {
        case SPINEL_DATATYPE_BOOL_C:
        {
            bool arg = (bool)va_arg(args->obj, int);
            ret += sizeof(uint8_t);

            if (data_len_max >= sizeof(uint8_t))
            {
                data_out[0] = (arg != false);
                data_out += sizeof(uint8_t);
                data_len_max -= sizeof(uint8_t);
            }
            else
            {
                data_len_max = 0;
            }

            break;
        }

        case SPINEL_DATATYPE_INT8_C:
        case SPINEL_DATATYPE_UINT8_C:
        {
            uint8_t arg = (uint8_t)va_arg(args->obj, int);
            ret += sizeof(uint8_t);

            if (data_len_max >= sizeof(uint8_t))
            {
                data_out[0] = arg;
                data_out += sizeof(uint8_t);
                data_len_max -= sizeof(uint8_t);
            }
            else
            {
                data_len_max = 0;
            }

            break;
        }

        case SPINEL_DATATYPE_INT16_C:
        case SPINEL_DATATYPE_UINT16_C:
        {
            uint16_t arg = (uint16_t)va_arg(args->obj, int);
            ret += sizeof(uint16_t);

            if (data_len_max >= sizeof(uint16_t))
            {
                data_out[1] = (arg >> 8) & 0xff;
                data_out[0] = (arg >> 0) & 0xff;
                data_out += sizeof(uint16_t);
                data_len_max -= sizeof(uint16_t);
            }
            else
            {
                data_len_max = 0;
            }

            break;
        }

        case SPINEL_DATATYPE_INT32_C:
        case SPINEL_DATATYPE_UINT32_C:
        {
            uint32_t arg = (uint32_t)va_arg(args->obj, int);
            ret += sizeof(uint32_t);

            if (data_len_max >= sizeof(uint32_t))
            {
                data_out[3] = (arg >> 24) & 0xff;
                data_out[2] = (arg >> 16) & 0xff;
                data_out[1] = (arg >> 8) & 0xff;
                data_out[0] = (arg >> 0) & 0xff;
                data_out += sizeof(uint32_t);
                data_len_max -= sizeof(uint32_t);
            }
            else
            {
                data_len_max = 0;
            }

            break;
        }

        case SPINEL_DATATYPE_INT64_C:
        case SPINEL_DATATYPE_UINT64_C:
        {
            uint64_t arg = va_arg(args->obj, uint64_t);

            ret += sizeof(uint64_t);

            if (data_len_max >= sizeof(uint64_t))
            {
                data_out[7] = (arg >> 56) & 0xff;
                data_out[6] = (arg >> 48) & 0xff;
                data_out[5] = (arg >> 40) & 0xff;
                data_out[4] = (arg >> 32) & 0xff;
                data_out[3] = (arg >> 24) & 0xff;
                data_out[2] = (arg >> 16) & 0xff;
                data_out[1] = (arg >> 8) & 0xff;
                data_out[0] = (arg >> 0) & 0xff;
                data_out += sizeof(uint64_t);
                data_len_max -= sizeof(uint64_t);
            }
            else
            {
                data_len_max = 0;
            }

            break;
        }

        case SPINEL_DATATYPE_IPv6ADDR_C:
        {
            spinel_ipv6addr_t *arg = va_arg(args->obj, spinel_ipv6addr_t *);
            ret += sizeof(spinel_ipv6addr_t);

            if (data_len_max >= sizeof(spinel_ipv6addr_t))
            {
                *(spinel_ipv6addr_t *)data_out = *arg;
                data_out += sizeof(spinel_ipv6addr_t);
                data_len_max -= sizeof(spinel_ipv6addr_t);
            }
            else
            {
                data_len_max = 0;
            }

            break;
        }

        case SPINEL_DATATYPE_EUI48_C:
        {
            spinel_eui48_t *arg = va_arg(args->obj, spinel_eui48_t *);
            ret += sizeof(spinel_eui48_t);

            if (data_len_max >= sizeof(spinel_eui48_t))
            {
                *(spinel_eui48_t *)data_out = *arg;
                data_out += sizeof(spinel_eui48_t);
                data_len_max -= sizeof(spinel_eui48_t);
            }
            else
            {
                data_len_max = 0;
            }

            break;
        }

        case SPINEL_DATATYPE_EUI64_C:
        {
            spinel_eui64_t *arg = va_arg(args->obj, spinel_eui64_t *);
            ret += sizeof(spinel_eui64_t);

            if (data_len_max >= sizeof(spinel_eui64_t))
            {
                *(spinel_eui64_t *)data_out = *arg;
                data_out += sizeof(spinel_eui64_t);
                data_len_max -= sizeof(spinel_eui64_t);
            }
            else
            {
                data_len_max = 0;
            }

            break;
        }

        case SPINEL_DATATYPE_UINT_PACKED_C:
        {
            uint32_t       arg = va_arg(args->obj, uint32_t);
            spinel_ssize_t encoded_size;

            // Range Check
            require_action(arg < SPINEL_MAX_UINT_PACKED, bail, {
                ret   = -1;
                errno = EINVAL;
            });

            encoded_size = spinel_packed_uint_encode(data_out, data_len_max, arg);
            ret += encoded_size;

            if ((spinel_ssize_t)data_len_max >= encoded_size)
            {
                data_out += encoded_size;
                data_len_max -= (spinel_size_t)encoded_size;
            }
            else
            {
                data_len_max = 0;
            }

            break;
        }

        case SPINEL_DATATYPE_UTF8_C:
        {
            const char *string_arg     = va_arg(args->obj, const char *);
            size_t      string_arg_len = 0;

            if (string_arg)
            {
                string_arg_len = strlen(string_arg) + 1;
            }
            else
            {
                string_arg     = "";
                string_arg_len = 1;
            }

            ret += (spinel_size_t)string_arg_len;

            if (data_len_max >= string_arg_len)
            {
                memcpy(data_out, string_arg, string_arg_len);

                data_out += string_arg_len;
                data_len_max -= (spinel_size_t)string_arg_len;
            }
            else
            {
                data_len_max = 0;
            }

            break;
        }

        case SPINEL_DATATYPE_DATA_WLEN_C:
        case SPINEL_DATATYPE_DATA_C:
        {
            const uint8_t *arg           = va_arg(args->obj, const uint8_t *);
            uint32_t       data_size_arg = va_arg(args->obj, uint32_t);
            spinel_ssize_t size_len      = 0;
            char           nextformat    = *spinel_next_packed_datatype(pack_format);

            if ((pack_format[0] == SPINEL_DATATYPE_DATA_WLEN_C) || ((nextformat != 0) && (nextformat != ')')))
            {
                size_len = spinel_datatype_pack(data_out, data_len_max, SPINEL_DATATYPE_UINT16_S, data_size_arg);
                require_action(size_len > 0, bail, {
                    ret   = -1;
                    errno = EINVAL;
                });
            }

            ret += (spinel_size_t)size_len + data_size_arg;

            if (data_len_max >= (spinel_size_t)size_len + data_size_arg)
            {
                data_out += size_len;
                data_len_max -= (spinel_size_t)size_len;

                if (data_out && arg)
                {
                    memcpy(data_out, arg, data_size_arg);
                }

                data_out += data_size_arg;
                data_len_max -= data_size_arg;
            }
            else
            {
                data_len_max = 0;
            }

            break;
        }

        case 'T':
        case SPINEL_DATATYPE_STRUCT_C:
        {
            spinel_ssize_t struct_len = 0;
            spinel_ssize_t size_len   = 0;
            char           nextformat = *spinel_next_packed_datatype(pack_format);

            require_action(pack_format[1] == '(', bail, {
                ret   = -1;
                errno = EINVAL;
            });

            // First we figure out the size of the struct
            {
                va_list_obj subargs;
                va_copy(subargs.obj, args->obj);
                struct_len = spinel_datatype_vpack_(NULL, 0, pack_format + 2, &subargs);
                va_end(subargs.obj);
            }

            if ((pack_format[0] == SPINEL_DATATYPE_STRUCT_C) || ((nextformat != 0) && (nextformat != ')')))
            {
                size_len = spinel_datatype_pack(data_out, data_len_max, SPINEL_DATATYPE_UINT16_S, struct_len);
                require_action(size_len > 0, bail, {
                    ret   = -1;
                    errno = EINVAL;
                });
            }

            ret += size_len + struct_len;

            if (struct_len + size_len <= (spinel_ssize_t)data_len_max)
            {
                data_out += size_len;
                data_len_max -= (spinel_size_t)size_len;

                struct_len = spinel_datatype_vpack_(data_out, data_len_max, pack_format + 2, args);

                data_out += struct_len;
                data_len_max -= (spinel_size_t)struct_len;
            }
            else
            {
                data_len_max = 0;
            }

            break;
        }

        case '.':
            // Skip.
            break;

        default:
            // Unsupported Type!
            ret   = -1;
            errno = EINVAL;
            goto bail;
        }
    }

bail:
    return ret;
}

spinel_ssize_t spinel_datatype_pack(uint8_t *data_out, spinel_size_t data_len_max, const char *pack_format, ...)
{
    int         ret;
    va_list_obj args;
    va_start(args.obj, pack_format);

    ret = spinel_datatype_vpack_(data_out, data_len_max, pack_format, &args);

    va_end(args.obj);
    return ret;
}

spinel_ssize_t spinel_datatype_vpack(uint8_t *     data_out,
                                     spinel_size_t data_len_max,
                                     const char *  pack_format,
                                     va_list       args)
{
    int         ret;
    va_list_obj args_obj;
    va_copy(args_obj.obj, args);

    ret = spinel_datatype_vpack_(data_out, data_len_max, pack_format, &args_obj);

    va_end(args_obj.obj);
    return ret;
}

// ----------------------------------------------------------------------------
// MARK: -

// LCOV_EXCL_START

const char *spinel_command_to_cstr(spinel_command_t command)
{
    const char *ret = "UNKNOWN";

    switch (command)
    {
        case SPINEL_CMD_NOOP:
            ret = "NOOP";
            break;

        case SPINEL_CMD_RESET:
            ret = "RESET";
            break;

        case SPINEL_CMD_PROP_VALUE_GET:
            ret = "PROP_VALUE_GET";
            break;

        case SPINEL_CMD_PROP_VALUE_SET:
            ret = "PROP_VALUE_SET";
            break;

        case SPINEL_CMD_PROP_VALUE_INSERT:
            ret = "PROP_VALUE_INSERT";
            break;

        case SPINEL_CMD_PROP_VALUE_REMOVE:
            ret = "PROP_VALUE_REMOVE";
            break;

        case SPINEL_CMD_PROP_VALUE_IS:
            ret = "PROP_VALUE_IS";
            break;

        case SPINEL_CMD_PROP_VALUE_INSERTED:
            ret = "PROP_VALUE_INSERTED";
            break;

        case SPINEL_CMD_PROP_VALUE_REMOVED:
            ret = "PROP_VALUE_REMOVED";
            break;

        default:
            break;
    }

    return ret;
}

const char *spinel_prop_key_to_cstr(spinel_prop_key_t prop_key)
{
    const char *ret = "UNKNOWN";

    switch (prop_key)
    {
        case SPINEL_PROP_LAST_STATUS:
            ret = "LAST_STATUS";
            break;

        case SPINEL_PROP_PROTOCOL_VERSION:
            ret = "PROTOCOL_VERSION";
            break;

        case SPINEL_PROP_NCP_VERSION:
            ret = "NCP_VERSION";
            break;

        case SPINEL_PROP_INTERFACE_TYPE:
            ret = "INTERFACE_TYPE";
            break;

        case SPINEL_PROP_HWADDR:
            ret = "HWADDR";
            break;

        case SPINEL_PROP_TRX_FW_VERSION:
            ret = "TRXFWVER";
            break;

        case SPINEL_PROP_PHY_CCA_THRESHOLD:
            ret = "PHY_CCA_THRESHOLD";
            break;

        case SPINEL_PROP_PHY_TX_POWER:
            ret = "PHY_TX_POWER";
            break;

        case SPINEL_PROP_NET_IF_UP:
            ret = "NET_IF_UP";
            break;

        case SPINEL_PROP_NET_STACK_UP:
            ret = "NET_STACK_UP";
            break;

        case SPINEL_PROP_NET_ROLE:
            ret = "NET_ROLE";
            break;

        case SPINEL_PROP_NET_NETWORK_NAME:
            ret = "NET_NETWORK_NAME";
            break;

        case SPINEL_PROP_DODAG_ROUTE_DEST:
            ret = "DODAG_ROUTE_DEST";
            break;

        case SPINEL_PROP_DODAG_ROUTE:
            ret = "DODAG_ROUTE";
            break;

        case SPINEL_PROP_IPV6_ADDRESS_TABLE:
            ret = "IPV6_ADDRESS_TABLE";
            break;

        case SPINEL_PROP_MULTICAST_LIST:
            ret = "IPV6_MULTICAST_LIST";
            break;

        case SPINEL_PROP_NUM_CONNECTED_DEVICES:
            ret = "NUM_CONNECTED_DEVICES";
            break;

        case SPINEL_PROP_CONNECTED_DEVICES:
            ret = "CONNECTED_DEVICES";
            break;

        case SPINEL_PROP_STREAM_NET:
            ret = "STREAM_NET";
            break;

        default:
            break;
    }

    return ret;
}

const char *spinel_net_role_to_cstr(uint8_t net_role)
{
    const char *ret = "NET_ROLE_UNKNOWN";

    switch (net_role)
    {
        case SPINEL_NET_ROLE_BORDER_ROUTER:
            ret = "NET_ROLE_BORDER_ROUTER";
            break;

        case SPINEL_NET_ROLE_ROUTER:
            ret = "NET_ROLE_ROUTER";
            break;

        default:
        break;
    }

    return ret;
}

const char *spinel_status_to_cstr(spinel_status_t status)
{
    const char *ret = "UNKNOWN";

    switch (status)
    {
    case SPINEL_STATUS_OK:
        ret = "OK";
        break;

    case SPINEL_STATUS_FAILURE:
        ret = "FAILURE";
        break;

    case SPINEL_STATUS_UNIMPLEMENTED:
        ret = "UNIMPLEMENTED";
        break;

    case SPINEL_STATUS_INVALID_ARGUMENT:
        ret = "INVALID_ARGUMENT";
        break;

    case SPINEL_STATUS_INVALID_STATE:
        ret = "INVALID_STATE";
        break;

    case SPINEL_STATUS_INVALID_COMMAND:
        ret = "INVALID_COMMAND";
        break;

    case SPINEL_STATUS_INVALID_INTERFACE:
        ret = "INVALID_INTERFACE";
        break;

    case SPINEL_STATUS_INTERNAL_ERROR:
        ret = "INTERNAL_ERROR";
        break;

    case SPINEL_STATUS_SECURITY_ERROR:
        ret = "SECURITY_ERROR";
        break;

    case SPINEL_STATUS_PARSE_ERROR:
        ret = "PARSE_ERROR";
        break;

    case SPINEL_STATUS_IN_PROGRESS:
        ret = "IN_PROGRESS";
        break;

    case SPINEL_STATUS_NOMEM:
        ret = "NOMEM";
        break;

    case SPINEL_STATUS_BUSY:
        ret = "BUSY";
        break;

    case SPINEL_STATUS_PROP_NOT_FOUND:
        ret = "PROP_NOT_FOUND";
        break;

    case SPINEL_STATUS_DROPPED:
        ret = "DROPPED";
        break;

    case SPINEL_STATUS_EMPTY:
        ret = "EMPTY";
        break;

    case SPINEL_STATUS_CMD_TOO_BIG:
        ret = "CMD_TOO_BIG";
        break;

    case SPINEL_STATUS_NO_ACK:
        ret = "NO_ACK";
        break;

    case SPINEL_STATUS_CCA_FAILURE:
        ret = "CCA_FAILURE";
        break;

    case SPINEL_STATUS_ALREADY:
        ret = "ALREADY";
        break;

    case SPINEL_STATUS_ITEM_NOT_FOUND:
        ret = "ITEM_NOT_FOUND";
        break;

    case SPINEL_STATUS_INVALID_COMMAND_FOR_PROP:
        ret = "INVALID_COMMAND_FOR_PROP";
        break;

    case SPINEL_STATUS_RESET_POWER_ON:
        ret = "RESET_POWER_ON";
        break;

    case SPINEL_STATUS_RESET_EXTERNAL:
        ret = "RESET_EXTERNAL";
        break;

    case SPINEL_STATUS_RESET_SOFTWARE:
        ret = "RESET_SOFTWARE";
        break;

    case SPINEL_STATUS_RESET_FAULT:
        ret = "RESET_FAULT";
        break;

    case SPINEL_STATUS_RESET_CRASH:
        ret = "RESET_CRASH";
        break;

    case SPINEL_STATUS_RESET_ASSERT:
        ret = "RESET_ASSERT";
        break;

    case SPINEL_STATUS_RESET_OTHER:
        ret = "RESET_OTHER";
        break;

    case SPINEL_STATUS_RESET_UNKNOWN:
        ret = "RESET_UNKNOWN";
        break;

    case SPINEL_STATUS_RESET_WATCHDOG:
        ret = "RESET_WATCHDOG";
        break;

    default:
        break;
    }

    return ret;
}

// LCOV_EXCL_STOP

/* -------------------------------------------------------------------------- */

#if SPINEL_SELF_TEST

int main(void)
{
    int                  ret             = -1;
    const spinel_eui64_t static_eui64    = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07}};
    const char           static_string[] = "static_string";
    uint8_t              buffer[1024];
    ssize_t              len;

    len =
        spinel_datatype_pack(buffer, sizeof(buffer), "CiiLUE", 0x88, 9, 0xA3, 0xDEADBEEF, static_string, &static_eui64);

    if (len != 30)
    {
        printf("error:%d: len != 30; (%d)\n", __LINE__, (int)len);
        goto bail;
    }

    {
        const char *str = NULL;

        // Length ends right before the string.
        len = spinel_datatype_unpack(buffer, 8, "CiiLU", NULL, NULL, NULL, NULL, &str);

        if (len != -1)
        {
            printf("error:%d: len != -1; (%d)\n", __LINE__, (int)len);
            goto bail;
        }

        if (str != NULL)
        {
            printf("error:%d: str != NULL\n", __LINE__);
            goto bail;
        }
    }

    len = 30;

    {
        uint8_t               c     = 0;
        unsigned int          i1    = 0;
        unsigned int          i2    = 0;
        uint32_t              l     = 0;
        const char *          str   = NULL;
        const spinel_eui64_t *eui64 = NULL;

        len = spinel_datatype_unpack(buffer, (spinel_size_t)len, "CiiLUE", &c, &i1, &i2, &l, &str, &eui64);

        if (len != 30)
        {
            printf("error:%d: len != 30; (%d)\n", __LINE__, (int)len);
            goto bail;
        }

        if (c != 0x88)
        {
            printf("error: x != 0x88; (%d)\n", c);
            goto bail;
        }

        if (i1 != 9)
        {
            printf("error: i1 != 9; (%d)\n", i1);
            goto bail;
        }

        if (i2 != 0xA3)
        {
            printf("error: i2 != 0xA3; (0x%02X)\n", i2);
            goto bail;
        }

        if (l != 0xDEADBEEF)
        {
            printf("error: l != 0xDEADBEEF; (0x%08X)\n", (unsigned int)l);
            goto bail;
        }

        if (strcmp(str, static_string) != 0)
        {
            printf("error:%d: strcmp(str,static_string) != 0\n", __LINE__);
            goto bail;
        }

        if (memcmp(eui64, &static_eui64, sizeof(spinel_eui64_t)) != 0)
        {
            printf("error:%d: memcmp(eui64, &eui64, sizeof(spinel_eui64_t)) != 0\n", __LINE__);
            goto bail;
        }
    }

    {
        uint8_t        c  = 0;
        unsigned int   i1 = 0;
        unsigned int   i2 = 0;
        uint32_t       l  = 0;
        char           str[sizeof(static_string)];
        spinel_eui64_t eui64 = {{0}};

        len = spinel_datatype_unpack_in_place(buffer, (spinel_size_t)len, "CiiLUE", &c, &i1, &i2, &l, &str, sizeof(str),
                                              &eui64);

        if (len != 30)
        {
            printf("error:%d: len != 30; (%d)\n", __LINE__, (int)len);
            goto bail;
        }

        if (c != 0x88)
        {
            printf("error: x != 0x88; (%d)\n", c);
            goto bail;
        }

        if (i1 != 9)
        {
            printf("error: i1 != 9; (%d)\n", i1);
            goto bail;
        }

        if (i2 != 0xA3)
        {
            printf("error: i2 != 0xA3; (0x%02X)\n", i2);
            goto bail;
        }

        if (l != 0xDEADBEEF)
        {
            printf("error: l != 0xDEADBEEF; (0x%08X)\n", (unsigned int)l);
            goto bail;
        }

        if (strcmp(str, static_string) != 0)
        {
            printf("error:%d: strcmp(str,static_string) != 0\n", __LINE__);
            goto bail;
        }

        if (memcmp(&eui64, &static_eui64, sizeof(spinel_eui64_t)) != 0)
        {
            printf("error:%d: memcmp(&eui64, &static_eui64, sizeof(spinel_eui64_t)) != 0\n", __LINE__);
            goto bail;
        }
    }

    // -----------------------------------

    memset(buffer, 0xAA, sizeof(buffer));

    len = spinel_datatype_pack(buffer, sizeof(buffer), "Cit(iL)UE", 0x88, 9, 0xA3, 0xDEADBEEF, static_string,
                               &static_eui64);

    if (len != 32)
    {
        printf("error:%d: len != 32; (%d)\n", __LINE__, (int)len);
        goto bail;
    }

    {
        uint8_t         c     = 0;
        unsigned int    i1    = 0;
        unsigned int    i2    = 0;
        uint32_t        l     = 0;
        const char *    str   = NULL;
        spinel_eui64_t *eui64 = NULL;

        len = spinel_datatype_unpack(buffer, (spinel_size_t)len, "Cit(iL)UE", &c, &i1, &i2, &l, &str, &eui64);

        if (len != 32)
        {
            printf("error:%d: len != 24; (%d)\n", __LINE__, (int)len);
            goto bail;
        }

        if (c != 0x88)
        {
            printf("error: x != 0x88; (%d)\n", c);
            goto bail;
        }

        if (i1 != 9)
        {
            printf("error: i1 != 9; (%d)\n", i1);
            goto bail;
        }

        if (i2 != 0xA3)
        {
            printf("error: i2 != 0xA3; (0x%02X)\n", i2);
            goto bail;
        }

        if (l != 0xDEADBEEF)
        {
            printf("error: l != 0xDEADBEEF; (0x%08X)\n", (unsigned int)l);
            goto bail;
        }

        if (strcmp(str, static_string) != 0)
        {
            printf("error:%d: strcmp(str,static_string) != 0\n", __LINE__);
            goto bail;
        }

        if (memcmp(eui64, &static_eui64, sizeof(spinel_eui64_t)) != 0)
        {
            printf("error:%d: memcmp(eui64, &static_eui64, sizeof(spinel_eui64_t)) != 0\n", __LINE__);
            goto bail;
        }
    }

    {
        uint8_t        c  = 0;
        unsigned int   i1 = 0;
        unsigned int   i2 = 0;
        uint32_t       l  = 0;
        char           str[sizeof(static_string)];
        spinel_eui64_t eui64 = {{0}};

        len = spinel_datatype_unpack_in_place(buffer, (spinel_size_t)len, "Cit(iL)UE", &c, &i1, &i2, &l, &str,
                                              sizeof(str), &eui64);

        if (len != 32)
        {
            printf("error:%d: len != 24; (%d)\n", __LINE__, (int)len);
            goto bail;
        }

        if (c != 0x88)
        {
            printf("error: x != 0x88; (%d)\n", c);
            goto bail;
        }

        if (i1 != 9)
        {
            printf("error: i1 != 9; (%d)\n", i1);
            goto bail;
        }

        if (i2 != 0xA3)
        {
            printf("error: i2 != 0xA3; (0x%02X)\n", i2);
            goto bail;
        }

        if (l != 0xDEADBEEF)
        {
            printf("error: l != 0xDEADBEEF; (0x%08X)\n", (unsigned int)l);
            goto bail;
        }

        if (strcmp(str, static_string) != 0)
        {
            printf("error:%d: strcmp(str,static_string) != 0\n", __LINE__);
            goto bail;
        }

        if (memcmp(&eui64, &static_eui64, sizeof(spinel_eui64_t)) != 0)
        {
            printf("error:%d: memcmp(&eui64, &static_eui64, sizeof(spinel_eui64_t)) != 0\n", __LINE__);
            goto bail;
        }
    }

    {
        // Test UTF8 validation - Good/Valid strings

        // Single symbols
        const uint8_t single1[] = {0};                            // 0000
        const uint8_t single2[] = {0x7F, 0x00};                   // 007F
        const uint8_t single3[] = {0xC2, 0x80, 0x00};             // 080
        const uint8_t single4[] = {0xDF, 0xBF, 0x00};             // 07FF
        const uint8_t single5[] = {0xE0, 0xA0, 0x80, 0x00};       // 0800
        const uint8_t single6[] = {0xEF, 0xBF, 0xBF, 0x00};       // FFFF
        const uint8_t single7[] = {0xF0, 0x90, 0x80, 0x80, 0x00}; // 010000
        const uint8_t single8[] = {0xF4, 0x8F, 0xBF, 0xBF, 0x00}; // 10FFFF

        // Strings
        const uint8_t str1[] = "spinel";
        const uint8_t str2[] = "OpenThread";
        const uint8_t str3[] = {0x41, 0x7F, 0xEF, 0xBF, 0xBF, 0xC2, 0x80, 0x21, 0x33, 0x00};
        const uint8_t str4[] = {0xCE, 0xBA, 0xE1, 0xBD, 0xB9, 0xCF, 0x83, 0xCE, 0xBC, 0xCE, 0xB5, 0x00}; // κόσμε
        const uint8_t str5[] = {0x3D, 0xF4, 0x8F, 0xBF, 0xBF, 0x01, 0xE0, 0xA0, 0x83, 0x22, 0xEF, 0xBF, 0xBF, 0x00};
        const uint8_t str6[] = {0xE5, 0xA2, 0x82, 0xE0, 0xA0, 0x80, 0xC2, 0x83, 0xC2, 0x80, 0xF4,
                                0x8F, 0xBF, 0xBF, 0xF4, 0x8F, 0xBF, 0xBF, 0xDF, 0xBF, 0x21, 0x00};

        const uint8_t * good_strings[] = {single1, single2, single3, single4, single5, single6, single7, single8,
                                         str1,    str2,    str3,    str4,    str5,    str6,    NULL};
        const uint8_t **str_ptr;

        for (str_ptr = &good_strings[0]; *str_ptr != NULL; str_ptr++)
        {
            if (!spinel_validate_utf8(*str_ptr))
            {
                printf("error: spinel_validate_utf8() did not correctly detect a valid UTF8 sequence!\n");
                goto bail;
            }
        }
    }

    {
        // Test UTF8 validation - Bad/Invalid strings

        // Single symbols (invalid)
        const uint8_t single1[] = {0xF8, 0x00};
        const uint8_t single2[] = {0xF9, 0x00};
        const uint8_t single3[] = {0xFA, 0x00};
        const uint8_t single4[] = {0xFF, 0x00};

        // Bad continuations
        const uint8_t bad1[] = {0xDF, 0x0F, 0x00};
        const uint8_t bad2[] = {0xE0, 0xA0, 0x10, 0x00};
        const uint8_t bad3[] = {0xF0, 0x90, 0x80, 0x60, 0x00};
        const uint8_t bad4[] = {0xF4, 0x8F, 0xBF, 0x0F, 0x00};
        const uint8_t bad5[] = {0x21, 0xA0, 0x00};
        const uint8_t bad6[] = {0xCE, 0xBA, 0xE1, 0xBD, 0xB9, 0xCF, 0x83, 0xCE, 0xBC, 0xCE, 0x00};

        const uint8_t * bad_strings[] = {single1, single2, single3, single4, bad1, bad2, bad3, bad4, bad5, bad6, NULL};
        const uint8_t **str_ptr;

        for (str_ptr = &bad_strings[0]; *str_ptr != NULL; str_ptr++)
        {
            if (spinel_validate_utf8(*str_ptr))
            {
                printf("error: spinel_validate_utf8() did not correctly detect an invalid UTF8 sequence\n");
                goto bail;
            }
        }
    }

    printf("OK\n");
    ret = 0;
    return ret;

bail:
    printf("FAILURE\n");
    return ret;
}

#endif // #if SPINEL_SELF_TEST
