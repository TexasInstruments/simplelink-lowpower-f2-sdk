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

/**
 * @file
 *   This file contains definitions of spinel.
 */

#ifndef SPINEL_HEADER_INCLUDED
#define SPINEL_HEADER_INCLUDED 1

/*
 *   Spinel is a host-controller protocol designed to enable
 *   inter-operation over simple serial connections between general purpose
 *   device operating systems (OS) host and network co-processors (NCP) for
 *   the purpose of controlling and managing the NCP.
 *
 * ---------------------------------------------------------------------------
 *
 *   Frame Format
 *
 *   A frame is defined simply as the concatenation of
 *
 *   -  A header byte
 *   -  A command (up to three bytes)
 *   -  An optional command payload
 *
 *              +---------+--------+-----+-------------+
 *              | Octets: |   1    | 1-3 |      n      |
 *              +---------+--------+-----+-------------+
 *              | Fields: | HEADER | CMD | CMD_PAYLOAD |
 *              +---------+--------+-----+-------------+
 *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *
 *   Header Format
 *
 *   The header byte is broken down as follows:
 *
 *                    0   1   2   3   4   5   6   7
 *                  +---+---+---+---+---+---+---+---+
 *                  |  FLG  |  IID  |      TID      |
 *                  +---+---+---+---+---+---+---+---+
 *
 *
 *   The flag field of the header byte ("FLG") is always set to the value
 *   two (or "10" in binary).  Any frame received with these bits set to
 *   any other value else MUST NOT be considered a Spinel frame.
 *
 *   This convention allows Spinel to be line compatible with BTLE HCI.
 *   By defining the first two bit in this way we can disambiguate between
 *   Spinel frames and HCI frames (which always start with either "0x01"
 *   or "0x04") without any additional framing overhead.
 *
 *   The Interface Identifier (IID) is a number between 0 and 3, which
 *   is associated by the OS with a specific NCP. This allows the protocol
 *   to support up to 4 NCPs under same connection.
 *
 *   The least significant bits of the header represent the Transaction
 *   Identifier (TID). The TID is used for correlating responses to the
 *   commands which generated them.
 *
 *   When a command is sent from the host, any reply to that command sent
 *   by the NCP will use the same value for the TID.  When the host
 *   receives a frame that matches the TID of the command it sent, it can
 *   easily recognize that frame as the actual response to that command.
 *
 *   The TID value of zero (0) is used for commands to which a correlated
 *   response is not expected or needed, such as for unsolicited update
 *   commands sent to the host from the NCP.
 *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *
 *   The command identifier is a 21-bit unsigned integer encoded in up to
 *   three bytes using the packed unsigned integer format described below.
 *   Depending on the semantics of the command in question, a payload MAY
 *   be included in the frame.  The exact composition and length of the
 *   payload is defined by the command identifier.
 *
 * ---------------------------------------------------------------------------
 *
 *   Data Packing
 *
 *   Data serialization for properties is performed using a light-weight
 *   data packing format which was loosely inspired by D-Bus.  The format
 *   of a serialization is defined by a specially formatted string.
 *
 *   This packing format is used for notational convenience.  While this
 *   string-based data-type format has been designed so that the strings
 *   may be directly used by a structured data parser, such a thing is not
 *   required to implement Spinel.
 *
 *   Goals:
 *
 *   -  Be lightweight and favor direct representation of values.
 *   -  Use an easily readable and memorable format string.
 *   -  Support lists and structures.
 *   -  Allow properties to be appended to structures while maintaining
 *      backward compatibility.
 *
 *   Each primitive data-type has an ASCII character associated with it.
 *   Structures can be represented as strings of these characters.  For
 *   example:
 *
 *   -  "C": A single unsigned byte.
 *   -  "C6U": A single unsigned byte, followed by a 128-bit IPv6 address,
 *      followed by a zero-terminated UTF8 string.
 *   -  "A(6)": An array of concatenated IPv6 addresses
 *
 *   In each case, the data is represented exactly as described.  For
 *   example, an array of 10 IPv6 address is stored as 160 bytes.
 *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *
 *   Primitive Types
 *
 *   +----------+----------------------+---------------------------------+
 *   |   Char   | Name                 | Description                     |
 *   +----------+----------------------+---------------------------------+
 *   |   "."    | DATATYPE_VOID        | Empty data type. Used           |
 *   |          |                      | internally.                     |
 *   |   "b"    | DATATYPE_BOOL        | Boolean value. Encoded in       |
 *   |          |                      | 8-bits as either 0x00 or 0x01.  |
 *   |          |                      | All other values are illegal.   |
 *   |   "C"    | DATATYPE_UINT8       | Unsigned 8-bit integer.         |
 *   |   "c"    | DATATYPE_INT8        | Signed 8-bit integer.           |
 *   |   "S"    | DATATYPE_UINT16      | Unsigned 16-bit integer.        |
 *   |   "s"    | DATATYPE_INT16       | Signed 16-bit integer.          |
 *   |   "L"    | DATATYPE_UINT32      | Unsigned 32-bit integer.        |
 *   |   "l"    | DATATYPE_INT32       | Signed 32-bit integer.          |
 *   |   "i"    | DATATYPE_UINT_PACKED | Packed Unsigned Integer. See    |
 *   |          |                      | description below               |
 *   |   "6"    | DATATYPE_IPv6ADDR    | IPv6 Address. (Big-endian)      |
 *   |   "E"    | DATATYPE_EUI64       | EUI-64 Address. (Big-endian)    |
 *   |   "e"    | DATATYPE_EUI48       | EUI-48 Address. (Big-endian)    |
 *   |   "D"    | DATATYPE_DATA        | Arbitrary data. See related     |
 *   |          |                      | section below for details.      |
 *   |   "d"    | DATATYPE_DATA_WLEN   | Arbitrary data with prepended   |
 *   |          |                      | length. See below for details   |
 *   |   "U"    | DATATYPE_UTF8        | Zero-terminated UTF8-encoded    |
 *   |          |                      | string.                         |
 *   | "t(...)" | DATATYPE_STRUCT      | Structured datatype with        |
 *   |          |                      | prepended length.               |
 *   | "A(...)" | DATATYPE_ARRAY       | Array of datatypes. Compound    |
 *   |          |                      | type.                           |
 *   +----------+----------------------+---------------------------------+
 *
 *   All multi-byte values are little-endian unless explicitly stated
 *   otherwise.
 *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *
 *   Packed Unsigned Integer
 *
 *   For certain types of integers, such command or property identifiers,
 *   usually have a value on the wire that is less than 127.  However, in
 *   order to not preclude the use of values larger than 255, we would
 *   need to add an extra byte.  Doing this would add an extra byte to the
 *   majority of instances, which can add up in terms of bandwidth.
 *
 *   The packed unsigned integer format is based on the unsigned integer
 *   format in EXI, except that we limit the maximum value to the
 *   largest value that can be encoded into three bytes (2,097,151).
 *
 *   For all values less than 127, the packed form of the number is simply
 *   a single byte which directly represents the number.  For values
 *   larger than 127, the following process is used to encode the value:
 *
 *   1.  The unsigned integer is broken up into _n_ 7-bit chunks and
 *       placed into _n_ octets, leaving the most significant bit of each
 *       octet unused.
 *   2.  Order the octets from least-significant to most-significant.
 *       (Little-endian)
 *   3.  Clear the most significant bit of the most significant octet.
 *       Set the least significant bit on all other octets.
 *
 *   Where `n` is the smallest number of 7-bit chunks you can use to
 *   represent the given value.
 *
 *   Take the value 1337, for example:
 *
 *                              1337 => 0x0539
 *                                   => [39 0A]
 *                                   => [B9 0A]
 *
 *   To decode the value, you collect the 7-bit chunks until you find an
 *   octet with the most significant bit clear.
 *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *
 *   Data Blobs
 *
 *   There are two types for data blobs: "d" and "D".
 *
 *   -  "d" has the length of the data (in bytes) prepended to the data
 *      (with the length encoded as type "S").  The size of the length
 *      field is not included in the length.
 *   -  "D" does not have a prepended length: the length of the data is
 *      implied by the bytes remaining to be parsed.  It is an error for
 *      "D" to not be the last type in a type in a type signature.
 *
 *   This dichotomy allows for more efficient encoding by eliminating
 *   redundancy.  If the rest of the buffer is a data blob, encoding the
 *   length would be redundant because we already know how many bytes are
 *   in the rest of the buffer.
 *
 *   In some cases we use "d" even if it is the last field in a type
 *   signature.  We do this to allow for us to be able to append
 *   additional fields to the type signature if necessary in the future.
 *   This is usually the case with embedded structs, like in the scan
 *   results.
 *
 *   For example, let's say we have a buffer that is encoded with the
 *   datatype signature of "CLLD".  In this case, it is pretty easy to
 *   tell where the start and end of the data blob is: the start is 9
 *   bytes from the start of the buffer, and its length is the length of
 *   the buffer minus 9. (9 is the number of bytes taken up by a byte and
 *   two longs)
 *
 *   The datatype signature "CLLDU" is illegal because we can't determine
 *   where the last field (a zero-terminated UTF8 string) starts.  But the
 *   datatype "CLLdU" is legal, because the parser can determine the
 *   exact length of the data blob-- allowing it to know where the start
 *   of the next field would be.
 *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *
 *   Structured Data
 *
 *   The structure data type ("t(...)") is a way of bundling together
 *   several fields into a single structure.  It can be thought of as a
 *   "d" type except that instead of being opaque, the fields in the
 *   content are known.  This is useful for things like scan results where
 *   you have substructures which are defined by different layers.
 *
 *   For example, consider the type signature "Lt(ES)t(6C)".  In this
 *   hypothetical case, the first struct is defined by the MAC layer, and
 *   the second struct is defined by the PHY layer.  Because of the use of
 *   structures, we know exactly what part comes from that layer.
 *   Additionally, we can add fields to each structure without introducing
 *   backward compatability problems: Data encoded as "Lt(ESU)t(6C)"
 *   (Notice the extra "U") will decode just fine as "Lt(ES)t(6C)".
 *   Additionally, if we don't care about the MAC layer and only care
 *   about the network layer, we could parse as "Lt()t(6C)".
 *
 *   Note that data encoded as "Lt(ES)t(6C)" will also parse as "Ldd",
 *   with the structures from both layers now being opaque data blobs.
 *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *
 *   Arrays
 *
 *   An array is simply a concatenated set of _n_ data encodings.  For
 *   example, the type "A(6)" is simply a list of IPv6 addresses---one
 *   after the other.  The type "A(6E)" likewise a concatenation of IPv6-
 *   address/EUI-64 pairs.
 *
 *   If an array contains many fields, the fields will often be surrounded
 *   by a structure ("t(...)").  This effectively prepends each item in
 *   the array with its length.  This is useful for improving parsing
 *   performance or to allow additional fields to be added in the future
 *   in a backward compatible way.  If there is a high certainty that
 *   additional fields will never be added, the struct may be omitted
 *   (saving two bytes per item).
 *
 *   This specification does not define a way to embed an array as a field
 *   alongside other fields.
 *
 * ---------------------------------------------------------------------------
 *
 *   Spinel definition guideline:
 *
 *   New NCP firmware should work with an older host driver, i.e., NCP
 *   implementation should remain backward compatible.
 *
 *    - Existing fields in the format of an already implemented spinel
 *      property or command cannot change.
 *
 *    - New fields may be appended at the end of the format (or the end of
 *      a struct) as long as the NCP implementation treats the new fields as
 *      optional (i.e., a driver not aware of and therefore not using the
 *      new fields should continue to function as before).
 *
 * ---------------------------------------------------------------------------
 */

#ifdef SPINEL_PLATFORM_HEADER
#include SPINEL_PLATFORM_HEADER
#else // ifdef SPINEL_PLATFORM_HEADER
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#endif // else SPINEL_PLATFORM_HEADER

// ----------------------------------------------------------------------------

#ifndef DOXYGEN_SHOULD_SKIP_THIS

#if defined(__GNUC__)
#define SPINEL_API_EXTERN extern __attribute__((visibility("default")))
#define SPINEL_API_NONNULL_ALL __attribute__((nonnull))
#define SPINEL_API_WARN_UNUSED_RESULT __attribute__((warn_unused_result))
#endif // ifdef __GNUC__

#endif // ifndef DOXYGEN_SHOULD_SKIP_THIS

#ifndef SPINEL_API_EXTERN
#define SPINEL_API_EXTERN extern
#endif

#ifndef SPINEL_API_NONNULL_ALL
#define SPINEL_API_NONNULL_ALL
#endif

#ifndef SPINEL_API_WARN_UNUSED_RESULT
#define SPINEL_API_WARN_UNUSED_RESULT
#endif

// ----------------------------------------------------------------------------

#define SPINEL_FRAME_MAX_SIZE 1300

/**
 * @def SPINEL_ENCRYPTER_EXTRA_DATA_SIZE
 *
 *  The size of extra data to be allocated for spinel frame buffer,
 *  needed by Spinel Encrypter.
 *
 */
#define SPINEL_ENCRYPTER_EXTRA_DATA_SIZE 0

/**
 * @def SPINEL_FRAME_BUFFER_SIZE
 *
 *  The size of buffer large enough to fit one whole spinel frame with extra data
 *  needed by Spinel Encrypter.
 *
 */
#define SPINEL_FRAME_BUFFER_SIZE (SPINEL_FRAME_MAX_SIZE + SPINEL_ENCRYPTER_EXTRA_DATA_SIZE)

/// Macro for generating bit masks using bit index from the spec
#define SPINEL_BIT_MASK(bit_index, field_bit_count) ((1 << ((field_bit_count)-1)) >> (bit_index))

// ----------------------------------------------------------------------------

#if defined(__cplusplus)
extern "C" {
#endif

enum
{
    SPINEL_STATUS_OK                       = 0,  ///< Operation has completed successfully.
    SPINEL_STATUS_FAILURE                  = 1,  ///< Operation has failed for some undefined reason.
    SPINEL_STATUS_UNIMPLEMENTED            = 2,  ///< Given operation has not been implemented.
    SPINEL_STATUS_INVALID_ARGUMENT         = 3,  ///< An argument to the operation is invalid.
    SPINEL_STATUS_INVALID_STATE            = 4,  ///< This operation is invalid for the current device state.
    SPINEL_STATUS_INVALID_COMMAND          = 5,  ///< This command is not recognized.
    SPINEL_STATUS_INVALID_INTERFACE        = 6,  ///< This interface is not supported.
    SPINEL_STATUS_INTERNAL_ERROR           = 7,  ///< An internal runtime error has occurred.
    SPINEL_STATUS_SECURITY_ERROR           = 8,  ///< A security/authentication error has occurred.
    SPINEL_STATUS_PARSE_ERROR              = 9,  ///< A error has occurred while parsing the command.
    SPINEL_STATUS_IN_PROGRESS              = 10, ///< This operation is in progress.
    SPINEL_STATUS_NOMEM                    = 11, ///< Operation prevented due to memory pressure.
    SPINEL_STATUS_BUSY                     = 12, ///< The device is currently performing a mutually exclusive operation
    SPINEL_STATUS_PROP_NOT_FOUND           = 13, ///< The given property is not recognized.
    SPINEL_STATUS_DROPPED                  = 14, ///< A/The packet was dropped.
    SPINEL_STATUS_EMPTY                    = 15, ///< The result of the operation is empty.
    SPINEL_STATUS_CMD_TOO_BIG              = 16, ///< The command was too large to fit in the internal buffer.
    SPINEL_STATUS_NO_ACK                   = 17, ///< The packet was not acknowledged.
    SPINEL_STATUS_CCA_FAILURE              = 18, ///< The packet was not sent due to a CCA failure.
    SPINEL_STATUS_ALREADY                  = 19, ///< The operation is already in progress.
    SPINEL_STATUS_ITEM_NOT_FOUND           = 20, ///< The given item could not be found.
    SPINEL_STATUS_INVALID_COMMAND_FOR_PROP = 21, ///< The given command cannot be performed on this property.
    SPINEL_STATUS_NO_NEIGHBORTABLE_ENTRY   = 22, ///< packet failed to transmit as corresponding neighbor table entry was not available.

    SPINEL_STATUS_JOIN__BEGIN = 24,

     /// The node successfully joined the network
    SPINEL_STATUS_JOIN_SUCCESS = SPINEL_STATUS_JOIN__BEGIN + 0,

    /// The node was unable to find any other peers with matching network name
    SPINEL_STATUS_JOIN_NO_PEERS = SPINEL_STATUS_JOIN__BEGIN + 1,

    ///Certificates did not match up
    SPINEL_STATUS_AUTHENTIATION_FAILED = SPINEL_STATUS_JOIN__BEGIN + 2,

    /// Devices certificates were good but key exchanged failed due to timeout).
    /// Device will automatically try to reconnect to server.
    SPINEL_STATUS_KEY_EXCHANGE_FAILED = SPINEL_STATUS_JOIN__BEGIN + 3,

    ///Device was unable to get a GUA from DHCPv6 Server
    SPINEL_STATUS_DHCPV6_ADDRESS_ASSIGNMENT_FAILED = SPINEL_STATUS_JOIN__BEGIN + 4,

    /// Generic failure to associate with other peers.
    SPINEL_STATUS_JOIN_FAILED = SPINEL_STATUS_JOIN__BEGIN + 5,

    SPINEL_STATUS_JOIN__END = 30,

    SPINEL_STATUS_RESET__BEGIN   = 112,
    SPINEL_STATUS_RESET_POWER_ON = SPINEL_STATUS_RESET__BEGIN + 0,
    SPINEL_STATUS_RESET_EXTERNAL = SPINEL_STATUS_RESET__BEGIN + 1,
    SPINEL_STATUS_RESET_SOFTWARE = SPINEL_STATUS_RESET__BEGIN + 2,
    SPINEL_STATUS_RESET_FAULT    = SPINEL_STATUS_RESET__BEGIN + 3,
    SPINEL_STATUS_RESET_CRASH    = SPINEL_STATUS_RESET__BEGIN + 4,
    SPINEL_STATUS_RESET_ASSERT   = SPINEL_STATUS_RESET__BEGIN + 5,
    SPINEL_STATUS_RESET_OTHER    = SPINEL_STATUS_RESET__BEGIN + 6,
    SPINEL_STATUS_RESET_UNKNOWN  = SPINEL_STATUS_RESET__BEGIN + 7,
    SPINEL_STATUS_RESET_WATCHDOG = SPINEL_STATUS_RESET__BEGIN + 8,
    SPINEL_STATUS_RESET__END     = 128,

};

typedef uint32_t spinel_status_t;

typedef enum
{
    SPINEL_NET_ROLE_BORDER_ROUTER = 0,
    SPINEL_NET_ROLE_ROUTER    = 1,
} spinel_net_role_t;

typedef enum
{
    SPINEL_HOST_POWER_STATE_OFFLINE    = 0,
    SPINEL_HOST_POWER_STATE_DEEP_SLEEP = 1,
    SPINEL_HOST_POWER_STATE_RESERVED   = 2,
    SPINEL_HOST_POWER_STATE_LOW_POWER  = 3,
    SPINEL_HOST_POWER_STATE_ONLINE     = 4,
} spinel_host_power_state_t;

enum
{
    SPINEL_NCP_LOG_LEVEL_EMERG  = 0,
    SPINEL_NCP_LOG_LEVEL_ALERT  = 1,
    SPINEL_NCP_LOG_LEVEL_CRIT   = 2,
    SPINEL_NCP_LOG_LEVEL_ERR    = 3,
    SPINEL_NCP_LOG_LEVEL_WARN   = 4,
    SPINEL_NCP_LOG_LEVEL_NOTICE = 5,
    SPINEL_NCP_LOG_LEVEL_INFO   = 6,
    SPINEL_NCP_LOG_LEVEL_DEBUG  = 7,
};

enum
{
    SPINEL_NCP_LOG_REGION_NONE        = 0,
    SPINEL_NCP_LOG_REGION_OT_API      = 1,
    SPINEL_NCP_LOG_REGION_OT_MLE      = 2,
    SPINEL_NCP_LOG_REGION_OT_ARP      = 3,
    SPINEL_NCP_LOG_REGION_OT_NET_DATA = 4,
    SPINEL_NCP_LOG_REGION_OT_ICMP     = 5,
    SPINEL_NCP_LOG_REGION_OT_IP6      = 6,
    SPINEL_NCP_LOG_REGION_OT_MAC      = 7,
    SPINEL_NCP_LOG_REGION_OT_MEM      = 8,
    SPINEL_NCP_LOG_REGION_OT_NCP      = 9,
    SPINEL_NCP_LOG_REGION_OT_MESH_COP = 10,
    SPINEL_NCP_LOG_REGION_OT_NET_DIAG = 11,
    SPINEL_NCP_LOG_REGION_OT_PLATFORM = 12,
    SPINEL_NCP_LOG_REGION_OT_COAP     = 13,
    SPINEL_NCP_LOG_REGION_OT_CLI      = 14,
    SPINEL_NCP_LOG_REGION_OT_CORE     = 15,
    SPINEL_NCP_LOG_REGION_OT_UTIL     = 16,
    SPINEL_NCP_LOG_REGION_OT_BBR      = 17,
};

typedef struct
{
    uint8_t bytes[8];
} spinel_eui64_t;

typedef struct
{
    uint8_t bytes[8];
} spinel_net_xpanid_t;

typedef struct
{
    uint8_t bytes[16];
} spinel_net_pskc_t;

typedef struct
{
    uint8_t bytes[6];
} spinel_eui48_t;

typedef struct
{
    uint8_t bytes[16];
} spinel_ipv6addr_t;

typedef int          spinel_ssize_t;
typedef unsigned int spinel_size_t;
typedef uint8_t      spinel_tid_t;

enum
{
    /**
     * No-Operation command (Host -> NCP)
     *
     * Encoding: Empty
     *
     * Induces the NCP to send a success status back to the host. This is
     * primarily used for liveliness checks. The command payload for this
     * command SHOULD be empty.
     *
     * There is no error condition for this command.
     *
     */
    SPINEL_CMD_NOOP = 0,

    /**
     * Reset NCP command (Host -> NCP)
     *
     * Encoding: Empty
     *
     * Causes the NCP to perform a software reset. Due to the nature of
     * this command, the TID is ignored. The host should instead wait
     * for a `CMD_PROP_VALUE_IS` command from the NCP indicating
     * `PROP_LAST_STATUS` has been set to `STATUS_RESET_SOFTWARE`.
     *
     * The command payload for this command SHOULD be empty.
     *
     * If an error occurs, the value of `PROP_LAST_STATUS` will be emitted
     * instead with the value set to the generated status code for the error.
     *
     */
    SPINEL_CMD_RESET = 1,

    /**
     * Get property value command (Host -> NCP)
     *
     * Encoding: `i`
     *   `i` : Property Id
     *
     * Causes the NCP to emit a `CMD_PROP_VALUE_IS` command for the
     * given property identifier.
     *
     * The payload for this command is the property identifier encoded
     * in the packed unsigned integer format `i`.
     *
     * If an error occurs, the value of `PROP_LAST_STATUS` will be emitted
     * instead with the value set to the generated status code for the error.
     *
     */
    SPINEL_CMD_PROP_VALUE_GET = 2,

    /**
     * Set property value command (Host -> NCP)
     *
     * Encoding: `iD`
     *   `i` : Property Id
     *   `D` : Value (encoding depends on the property)
     *
     * Instructs the NCP to set the given property to the specific given
     * value, replacing any previous value.
     *
     * The payload for this command is the property identifier encoded in the
     * packed unsigned integer format, followed by the property value. The
     * exact format of the property value is defined by the property.
     *
     * On success a `CMD_PROP_VALUE_IS` command is emitted either for the
     * given property identifier with the set value, or for `PROP_LAST_STATUS`
     * with value `LAST_STATUS_OK`.
     *
     * If an error occurs, the value of `PROP_LAST_STATUS` will be emitted
     * with the value set to the generated status code for the error.
     *
     */
    SPINEL_CMD_PROP_VALUE_SET = 3,

    /**
     * Insert value into property command (Host -> NCP)
     *
     * Encoding: `iD`
     *   `i` : Property Id
     *   `D` : Value (encoding depends on the property)
     *
     * Instructs the NCP to insert the given value into a list-oriented
     * property without removing other items in the list. The resulting order
     * of items in the list is defined by the individual property being
     * operated on.
     *
     * The payload for this command is the property identifier encoded in the
     * packed unsigned integer format, followed by the value to be inserted.
     * The exact format of the value is defined by the property.
     *
     * If the type signature of the property consists of a single structure
     * enclosed by an array `A(t(...))`, then the contents of value MUST
     * contain the contents of the structure (`...`) rather than the
     * serialization of the whole item (`t(...)`).  Specifically, the length
     * of the structure MUST NOT be prepended to value. This helps to
     * eliminate redundant data.
     *
     * On success, either a `CMD_PROP_VALUE_INSERTED` command is emitted for
     * the given property, or a `CMD_PROP_VALUE_IS` command is emitted of
     * property `PROP_LAST_STATUS` with value `LAST_STATUS_OK`.
     *
     * If an error occurs, the value of `PROP_LAST_STATUS` will be emitted
     * with the value set to the generated status code for the error.
     *
     */
    SPINEL_CMD_PROP_VALUE_INSERT = 4,

    /**
     * Remove value from property command (Host -> NCP)
     *
     * Encoding: `iD`
     *   `i` : Property Id
     *   `D` : Value (encoding depends on the property)

     * Instructs the NCP to remove the given value from a list-oriented property,
     * without affecting other items in the list. The resulting order of items
     * in the list is defined by the individual property being operated on.
     *
     * Note that this command operates by value, not by index!
     *
     * The payload for this command is the property identifier encoded in the
     * packed unsigned integer format, followed by the value to be removed. The
     * exact format of the value is defined by the property.
     *
     * If the type signature of the property consists of a single structure
     * enclosed by an array `A(t(...))`, then the contents of value MUST contain
     * the contents of the structure (`...`) rather than the serialization of the
     * whole item (`t(...)`).  Specifically, the length of the structure MUST NOT
     * be prepended to `VALUE`. This helps to eliminate redundant data.
     *
     * On success, either a `CMD_PROP_VALUE_REMOVED` command is emitted for the
     * given property, or a `CMD_PROP_VALUE_IS` command is emitted of property
     * `PROP_LAST_STATUS` with value `LAST_STATUS_OK`.
     *
     * If an error occurs, the value of `PROP_LAST_STATUS` will be emitted
     * with the value set to the generated status code for the error.
     *
     */
    SPINEL_CMD_PROP_VALUE_REMOVE = 5,

    /**
     * Property value notification command (NCP -> Host)
     *
     * Encoding: `iD`
     *   `i` : Property Id
     *   `D` : Value (encoding depends on the property)
     *
     * This command can be sent by the NCP in response to a previous command
     * from the host, or it can be sent by the NCP in an unsolicited fashion
     * to notify the host of various state changes asynchronously.
     *
     * The payload for this command is the property identifier encoded in the
     * packed unsigned integer format, followed by the current value of the
     * given property.
     *
     */
    SPINEL_CMD_PROP_VALUE_IS = 6,

    /**
     * Property value insertion notification command (NCP -> Host)
     *
     * Encoding:`iD`
     *   `i` : Property Id
     *   `D` : Value (encoding depends on the property)
     *
     * This command can be sent by the NCP in response to the
     * `CMD_PROP_VALUE_INSERT` command, or it can be sent by the NCP in an
     * unsolicited fashion to notify the host of various state changes
     * asynchronously.
     *
     * The payload for this command is the property identifier encoded in the
     * packed unsigned integer format, followed by the value that was inserted
     * into the given property.
     *
     * If the type signature of the property specified by property id consists
     * of a single structure enclosed by an array (`A(t(...))`), then the
     * contents of value MUST contain the contents of the structure (`...`)
     * rather than the serialization of the whole item (`t(...)`). Specifically,
     * the length of the structure MUST NOT be prepended to `VALUE`. This
     * helps to eliminate redundant data.
     *
     * The resulting order of items in the list is defined by the given
     * property.
     *
     */
    SPINEL_CMD_PROP_VALUE_INSERTED = 7,

    /**
     * Property value removal notification command (NCP -> Host)
     *
     * Encoding: `iD`
     *   `i` : Property Id
     *   `D` : Value (encoding depends on the property)
     *
     * This command can be sent by the NCP in response to the
     * `CMD_PROP_VALUE_REMOVE` command, or it can be sent by the NCP in an
     * unsolicited fashion to notify the host of various state changes
     * asynchronously.
     *
     * Note that this command operates by value, not by index!
     *
     * The payload for this command is the property identifier encoded in the
     * packed unsigned integer format described in followed by the value that
     * was removed from the given property.
     *
     * If the type signature of the property specified by property id consists
     * of a single structure enclosed by an array (`A(t(...))`), then the
     * contents of value MUST contain the contents of the structure (`...`)
     * rather than the serialization of the whole item (`t(...)`).  Specifically,
     * the length of the structure MUST NOT be prepended to `VALUE`. This
     * helps to eliminate redundant data.
     *
     * The resulting order of items in the list is defined by the given
     * property.
     *
     */
    SPINEL_CMD_PROP_VALUE_REMOVED = 8,

#ifdef NV_RESTORE
    SPINEL_CMD_NV_ERASE = 9,
#endif //NV_RESTORE
};

typedef uint32_t spinel_command_t;


/**
 * Property Keys
 *
 * The properties are broken up into several sections, each with a
 * reserved ranges of property identifiers:
 *
 *    Name         | Range (Inclusive)              | Description
 *    -------------|--------------------------------|------------------------
 *    Core         | 0x000 - 0x01F, 0x1000 - 0x11FF | Spinel core
 *    PHY          | 0x020 - 0x02F, 0x1200 - 0x12FF | Radio PHY layer
 *    MAC          | 0x030 - 0x03F, 0x1300 - 0x13FF | MAC layer
 *    NET          | 0x040 - 0x04F, 0x1400 - 0x14FF | Network
 *    TI-Wi-SUN-PHY| 0x050 - 0x055, 0x1500 - 0x1555 | TI Wi-SUN stack specific PHY
 *    TI-Wi-SUN-MAC| 0x056 - 0x05A, 0x1556 - 0x15AA | TI Wi-SUN stack specific MAC
 *    TI-Wi-SUN-NET| 0x05B - 0x05F, 0x15AB - 0x15FF | TI Wi-SUN stack specific NET
 *    IPv6         | 0x060 - 0x06F, 0x1600 - 0x16FF | IPv6
 *    Stream       | 0x070 - 0x07F, 0x1700 - 0x17FF | Stream
 *
 */
enum
{
    /// Last Operation Status
    /** Format: `i` - Read-only
     *
     * Describes the status of the last operation. Encoded as a packed
     * unsigned integer (see `SPINEL_STATUS_*` for list of values).
     *
     * This property is emitted often to indicate the result status of
     * pretty much any Host-to-NCP operation.
     *
     * It is emitted automatically at NCP startup with a value indicating
     * the reset reason. It is also emitted asynchronously on an error (
     * e.g., NCP running out of buffer).
     *
     */
    SPINEL_PROP_LAST_STATUS = 0,

    /// Protocol Version
    /** Format: `U` - Read-only
     *
     * Contains a string which describes the firmware currently running on
     * the NCP. Encoded as a zero-terminated UTF-8 string.
     *
     */
    SPINEL_PROP_PROTOCOL_VERSION = 1,

    /// NCP Version
    /** Format: `U` - Read-only
     *
     * Contains a string which describes the firmware currently running on
     * the NCP. Encoded as a zero-terminated UTF-8 string.
     *
     */
    SPINEL_PROP_NCP_VERSION = 2,

    /// NCP Network Protocol Type
    /** Format: 'i' - Read-only
     *
     * This value identifies what the network protocol for this NCP.
     * The valid protocol type values are defined by enumeration
     * `SPINEL_PROTOCOL_TYPE_*`:
     *
     *   `SPINEL_PROTOCOL_TYPE_BOOTLOADER` = 0
     *   `SPINEL_PROTOCOL_TYPE_ZIGBEE_IP`  = 2,
     *   `SPINEL_PROTOCOL_TYPE_THREAD`     = 3,
     *   `SPINEL_PROTOCOL_TYPE_WISUN`      = 4,
     *
     * Wi-SUN NCP supports only `SPINEL_PROTOCOL_TYPE_WISUN`
     *
     */
    SPINEL_PROP_INTERFACE_TYPE = 3,


    /// NCP Hardware Address
    /** Format: 'E` - Read-only
     *
     * The static EUI64 address of the device, used as a serial number.
     *
     */
    SPINEL_PROP_HWADDR = 8,

    /// Host Power State
    /** Format: 'C`
     *
     * Describes the current power state of the host. This property is used
     * by the host to inform the NCP when it has changed power states. The
     * NCP can then use this state to determine which properties need
     * asynchronous updates. Enumeration `spinel_host_power_state_t` defines
     * the valid values (`SPINEL_HOST_POWER_STATE_*`):
     */
    SPINEL_PROP_HOST_POWER_STATE = 12,

    /// TRX Firmware version number if applicable
    /* for those projects which do not run with TRX all
     * zeros are returned
     */
    SPINEL_PROP_TRX_FW_VERSION = 13,


    SPINEL_PROP_BASE_EXT__BEGIN = 0x1000,
    SPINEL_PROP_BASE_EXT__END = 0x1200,

    SPINEL_PROP_PHY__BEGIN         = 0x20,
    SPINEL_PROP_PHY_CCA_THRESHOLD  = SPINEL_PROP_PHY__BEGIN + 4, ///< dBm [c]
    SPINEL_PROP_PHY_TX_POWER       = SPINEL_PROP_PHY__BEGIN + 5, ///< dBm
    SPINEL_PROP_PHY_NUM_NBRS       = SPINEL_PROP_PHY__BEGIN + 6,
    SPINEL_PROP_PHY_NBR_METRICS    = SPINEL_PROP_PHY__BEGIN + 7,
    SPINEL_PROP_PHY_METRICS        = SPINEL_PROP_PHY__BEGIN + 8,
    SPINEL_PROP_PHY__END           = 0x2F,

    SPINEL_PROP_PHY_EXT__BEGIN = 0x1200,
    SPINEL_PROP_PHY_EXT__END = 0x1300,

    SPINEL_PROP_MAC__BEGIN = 0x30,

    /// MAC PAN ID
    /** Format: `S`
     *
     * The 802.15.4 PANID this node is associated with.
     *
     */
    SPINEL_PROP_MAC_15_4_PANID = SPINEL_PROP_MAC__BEGIN + 6,
    SPINEL_PROP_MAC__END = 0x40,

    SPINEL_PROP_MAC_EXT__BEGIN = 0x1300,
    SPINEL_PROP_MAC_EXT__END = 0x1400,

    SPINEL_PROP_NET__BEGIN = 0x40,

    /// Network Interface Status
    /** Format `b` - Read-write
     *
     * Wi-SUN Network interface up/down status. Write true to bring
     * interface up and false to bring interface down.
     *
     */
    SPINEL_PROP_NET_IF_UP = SPINEL_PROP_NET__BEGIN + 1,

    /// Wi-SUN Stack Operational Status
    /** Format `b` - Read-write
     *
     * Wi-SUN stack operational status. Write true to start
     * Wi-SUN stack and false to stop it.
     *
     */
    SPINEL_PROP_NET_STACK_UP = SPINEL_PROP_NET__BEGIN + 2,

    /// Wi-SUN Device Role
    /** Format `C` - Read-write
     *
     * Possible values are from enumeration `spinel_net_role_t`
     */
    SPINEL_PROP_NET_ROLE = SPINEL_PROP_NET__BEGIN + 3,

    /// Wi-SUN Network Name
    /** Format `U` - Read-write
     *
     */
    SPINEL_PROP_NET_NETWORK_NAME = SPINEL_PROP_NET__BEGIN + 4,

    /// UDP Data transfer
    /** Format `b` - Read-write
     *
     * Start UDP packet transfer. Sets #of packets to transfer.
     *
     */
    SPINEL_PROP_NET_UDP_START = SPINEL_PROP_NET__BEGIN + 5,

    SPINEL_PROP_NET__END = 0x50,

    SPINEL_PROP_NET_EXT__BEGIN = 0x1400,
    SPINEL_PROP_NET_EXT__END   = 0x1500,

    /* Tech specific Properties */
    SPINEL_PROP_WISUN__BEGIN = 0x50,

    SPINEL_PROP_WISUN_PHY_BEGIN = SPINEL_PROP_WISUN__BEGIN,
    SPINEL_PROP_PHY_REGION = SPINEL_PROP_WISUN_PHY_BEGIN,
    SPINEL_PROP_PHY_MODE_ID = SPINEL_PROP_WISUN_PHY_BEGIN + 1,
    SPINEL_PROP_PHY_UNICAST_CHANNEL_LIST = SPINEL_PROP_WISUN_PHY_BEGIN + 2,
    SPINEL_PROP_PHY_BROADCAST_CHANNEL_LIST = SPINEL_PROP_WISUN_PHY_BEGIN + 3,
    SPINEL_PROP_PHY_ASYNC_CHANNEL_LIST = SPINEL_PROP_WISUN_PHY_BEGIN + 4,
    SPINEL_PROP_WISUN_PHY_END = 0x55,

    SPINEL_PROP_WISUN_MAC_BEGIN = SPINEL_PROP_WISUN_PHY_END + 1,
    SPINEL_PROP_WISUN_MAC_END = 0x5A,

    SPINEL_PROP_WISUN_NET_BEGIN = SPINEL_PROP_WISUN_MAC_END + 1,
    SPINEL_PROP_NET_STATE = SPINEL_PROP_WISUN_NET_BEGIN,
    SPINEL_PROP_PARENT_LIST = SPINEL_PROP_WISUN_NET_BEGIN + 1,
    SPINEL_PROP_ROUTING_COST = SPINEL_PROP_WISUN_NET_BEGIN + 2,
    SPINEL_PROP_ROUTING_TABLE_UPDATE = SPINEL_PROP_WISUN_NET_BEGIN + 3,
    SPINEL_PROP_DODAG_ROUTE_DEST = SPINEL_PROP_WISUN_NET_BEGIN + 4,
    SPINEL_PROP_DODAG_ROUTE = SPINEL_PROP_WISUN_NET_BEGIN + 5,
    SPINEL_PROP_WISUN_NET_END = 0x60,

    SPINEL_PROP_WISUN__END = SPINEL_PROP_WISUN_NET_END, //0x60

    /* Tech specific Extended Properties */
    SPINEL_PROP_WISUN_EXT__BEGIN = 0x1500,

    SPINEL_PROP_WISUN_EXT_PHY__BEGIN = SPINEL_PROP_WISUN_EXT__BEGIN,
    SPINEL_PROP_PHY_CH_SPACING = SPINEL_PROP_WISUN_EXT_PHY__BEGIN,
    SPINEL_PROP_PHY_CHO_CENTER_FREQ = SPINEL_PROP_WISUN_EXT_PHY__BEGIN + 1,
    SPINEL_PROP_WISUN_EXT_PHY__END = 0x1555,

    SPINEL_PROP_WISUN_EXT_MAC__BEGIN = 0x1556,
    SPINEL_MAC_UC_DWELL_INTERVAL = SPINEL_PROP_WISUN_EXT_MAC__BEGIN,
    SPINEL_MAC_BC_DWELL_INTERVAL = SPINEL_PROP_WISUN_EXT_MAC__BEGIN + 1,
    SPINEL_MAC_BC_INTERVAL = SPINEL_PROP_WISUN_EXT_MAC__BEGIN + 2,
    SPINEL_MAC_UC_CHANNEL_FUNCTION = SPINEL_PROP_WISUN_EXT_MAC__BEGIN + 3,
    SPINEL_MAC_BC_CHANNEL_FUNCTION = SPINEL_PROP_WISUN_EXT_MAC__BEGIN + 4,
    SPINEL_PROP_MAC_MAC_FILTER_LIST = SPINEL_PROP_WISUN_EXT_MAC__BEGIN + 5,
    SPINEL_PROP_MAC_FILTER_MODE = SPINEL_PROP_WISUN_EXT_MAC__BEGIN + 6,
    SPINEL_PROP_TEST_COMMAND = SPINEL_PROP_WISUN_EXT_MAC__BEGIN + 7,
    SPINEL_PROP_VPIE_COMMAND = SPINEL_PROP_WISUN_EXT_MAC__BEGIN + 8,
    SPINEL_PROP_MACMPL_COMMAND = SPINEL_PROP_WISUN_EXT_MAC__BEGIN + 9,
    SPINEL_PROP_EXTERNAL_DHCP_SERVER_ENABLED = SPINEL_PROP_WISUN_EXT_MAC__BEGIN + 10,
    SPINEL_PROP_EXTERNAL_DHCP_SERVER_ADDRESS = SPINEL_PROP_WISUN_EXT_MAC__BEGIN + 11,
    SPINEL_PROP_EXTERNAL_AUTH_SERVER_ENABLED = SPINEL_PROP_WISUN_EXT_MAC__BEGIN + 12,
    SPINEL_PROP_EXTERNAL_AUTH_SERVER_ADDRESS = SPINEL_PROP_WISUN_EXT_MAC__BEGIN + 13,
    SPINEL_PROP_WISUN_EXT_MAC__END = 0x15AA,

    SPINEL_PROP_WISUN_EXT_NET__BEGIN = 0x15AB,
    // TI Wi-SUN specific NET properties
    SPINEL_PROP_REVOKE_GTK_HWADDR = SPINEL_PROP_WISUN_EXT_NET__BEGIN,
    SPINEL_PROP_WISUN_EXT_NET__END = 0x1600,

    SPINEL_PROP_WISUN_EXT__END = SPINEL_PROP_WISUN_EXT_NET__END, // 0x15FF

    SPINEL_PROP_IPV6__BEGIN = 0x60,
    SPINEL_PROP_IPV6_ADDRESS_TABLE = SPINEL_PROP_IPV6__BEGIN + 3,
    SPINEL_PROP_MULTICAST_LIST = SPINEL_PROP_IPV6__BEGIN + 4,
    SPINEL_PROP_NUM_CONNECTED_DEVICES = SPINEL_PROP_IPV6__BEGIN + 5,
    SPINEL_PROP_CONNECTED_DEVICES = SPINEL_PROP_IPV6__BEGIN + 6,
    SPINEL_PROP_IPV6__END = 0x70,

    SPINEL_PROP_IPV6_EXT__BEGIN = 0x1600,
    SPINEL_PROP_IPV6_EXT__END   = 0x1700,

    SPINEL_PROP_STREAM__BEGIN = 0x70,
    SPINEL_PROP_STREAM_DEBUG = SPINEL_PROP_STREAM__BEGIN + 0, //mvtodo: remove

    /// (IPv6) Network Stream
    /** Format: `dD` (stream, read only)
     *
     * This stream provides the capability of sending and receiving (IPv6)
     * data packets to and from the currently attached network. The packets
     * are sent or received securely (encryption and authentication).
     *
     * This property is a streaming property, meaning that you cannot explicitly
     * fetch the value of this property. To receive traffic, you wait for
     * `CMD_PROP_VALUE_IS` commands with this property id from the NCP.
     *
     * To send network packets, you call `CMD_PROP_VALUE_SET` on this property with
     * the value of the packet.
     *
     * The general format of this property is:
     *
     *    `d` : packet data
     *    `D` : packet meta data
     */
    SPINEL_PROP_STREAM_NET = SPINEL_PROP_STREAM__BEGIN + 2,
    SPINEL_PROP_STREAM_LOG = SPINEL_PROP_STREAM__BEGIN + 4, //mvtodo: remove

    SPINEL_PROP_STREAM__END = 0x80,

    SPINEL_PROP_STREAM_EXT__BEGIN = 0x1700,
    SPINEL_PROP_STREAM_EXT__END   = 0x1800,

};

typedef uint32_t spinel_prop_key_t;

// ----------------------------------------------------------------------------

#define SPINEL_HEADER_FLAG 0x80

#define SPINEL_HEADER_TID_SHIFT 0
#define SPINEL_HEADER_TID_MASK (15 << SPINEL_HEADER_TID_SHIFT)

#define SPINEL_HEADER_IID_SHIFT 4
#define SPINEL_HEADER_IID_MASK (3 << SPINEL_HEADER_IID_SHIFT)

#define SPINEL_HEADER_IID_0 (0 << SPINEL_HEADER_IID_SHIFT)
#define SPINEL_HEADER_IID_1 (1 << SPINEL_HEADER_IID_SHIFT)
#define SPINEL_HEADER_IID_2 (2 << SPINEL_HEADER_IID_SHIFT)
#define SPINEL_HEADER_IID_3 (3 << SPINEL_HEADER_IID_SHIFT)

#define SPINEL_HEADER_GET_IID(x) (((x)&SPINEL_HEADER_IID_MASK) >> SPINEL_HEADER_IID_SHIFT)
#define SPINEL_HEADER_GET_TID(x) (spinel_tid_t)(((x)&SPINEL_HEADER_TID_MASK) >> SPINEL_HEADER_TID_SHIFT)

#define SPINEL_GET_NEXT_TID(x) (spinel_tid_t)((x) >= 0xF ? 1 : (x) + 1)

// ----------------------------------------------------------------------------

enum
{
    SPINEL_DATATYPE_NULL_C        = 0,
    SPINEL_DATATYPE_VOID_C        = '.',
    SPINEL_DATATYPE_BOOL_C        = 'b',
    SPINEL_DATATYPE_UINT8_C       = 'C',
    SPINEL_DATATYPE_INT8_C        = 'c',
    SPINEL_DATATYPE_UINT16_C      = 'S',
    SPINEL_DATATYPE_INT16_C       = 's',
    SPINEL_DATATYPE_UINT32_C      = 'L',
    SPINEL_DATATYPE_INT32_C       = 'l',
    SPINEL_DATATYPE_UINT64_C      = 'X',
    SPINEL_DATATYPE_INT64_C       = 'x',
    SPINEL_DATATYPE_UINT_PACKED_C = 'i',
    SPINEL_DATATYPE_IPv6ADDR_C    = '6',
    SPINEL_DATATYPE_EUI64_C       = 'E',
    SPINEL_DATATYPE_EUI48_C       = 'e',
    SPINEL_DATATYPE_DATA_WLEN_C   = 'd',
    SPINEL_DATATYPE_DATA_C        = 'D',
    SPINEL_DATATYPE_UTF8_C        = 'U', //!< Zero-Terminated UTF8-Encoded String
    SPINEL_DATATYPE_STRUCT_C      = 't',
    SPINEL_DATATYPE_ARRAY_C       = 'A',
};

typedef char spinel_datatype_t;

#define SPINEL_DATATYPE_NULL_S ""
#define SPINEL_DATATYPE_VOID_S "."
#define SPINEL_DATATYPE_BOOL_S "b"
#define SPINEL_DATATYPE_UINT8_S "C"
#define SPINEL_DATATYPE_INT8_S "c"
#define SPINEL_DATATYPE_UINT16_S "S"
#define SPINEL_DATATYPE_INT16_S "s"
#define SPINEL_DATATYPE_UINT32_S "L"
#define SPINEL_DATATYPE_INT32_S "l"
#define SPINEL_DATATYPE_UINT64_S "X"
#define SPINEL_DATATYPE_INT64_S "x"
#define SPINEL_DATATYPE_UINT_PACKED_S "i"
#define SPINEL_DATATYPE_IPv6ADDR_S "6"
#define SPINEL_DATATYPE_EUI64_S "E"
#define SPINEL_DATATYPE_EUI48_S "e"
#define SPINEL_DATATYPE_DATA_WLEN_S "d"
#define SPINEL_DATATYPE_DATA_S "D"
#define SPINEL_DATATYPE_UTF8_S "U" //!< Zero-Terminated UTF8-Encoded String

#define SPINEL_DATATYPE_ARRAY_S(x) "A(" x ")"
#define SPINEL_DATATYPE_STRUCT_S(x) "t(" x ")"

#define SPINEL_DATATYPE_ARRAY_STRUCT_S(x) SPINEL_DATATYPE_ARRAY_S(SPINEL_DATATYPE_STRUCT_WLEN_S(x))

#define SPINEL_DATATYPE_COMMAND_S                   \
    SPINEL_DATATYPE_UINT8_S           /* header  */ \
        SPINEL_DATATYPE_UINT_PACKED_S /* command */

#define SPINEL_DATATYPE_COMMAND_PROP_S                    \
    SPINEL_DATATYPE_COMMAND_S         /* prop command  */ \
        SPINEL_DATATYPE_UINT_PACKED_S /* property id */

#define SPINEL_MAX_UINT_PACKED 2097151

SPINEL_API_EXTERN spinel_ssize_t spinel_datatype_pack(uint8_t *     data_out,
                                                      spinel_size_t data_len_max,
                                                      const char *  pack_format,
                                                      ...);
SPINEL_API_EXTERN spinel_ssize_t spinel_datatype_vpack(uint8_t *     data_out,
                                                       spinel_size_t data_len_max,
                                                       const char *  pack_format,
                                                       va_list       args);
SPINEL_API_EXTERN spinel_ssize_t spinel_datatype_unpack(const uint8_t *data_in,
                                                        spinel_size_t  data_len,
                                                        const char *   pack_format,
                                                        ...);
/**
 * This function parses spinel data similar to sscanf().
 *
 * This function actually calls spinel_datatype_vunpack_in_place() to parse data.
 *
 * @param[in]   data_in     A pointer to the data to parse.
 * @param[in]   data_len    The length of @p data_in in bytes.
 * @param[in]   pack_format C string that contains a format string follows the same specification of spinel.
 * @param[in]   ...         Additional arguments depending on the format string @p pack_format.
 *
 * @returns The parsed length in bytes.
 *
 * @note This function behaves different from `spinel_datatype_unpack()`:
 *       - This function expects composite data arguments of pointer to data type, while `spinel_datatype_unpack()`
 *         expects them of pointer to data type pointer. For example, if `SPINEL_DATATYPE_EUI64_C` is present in
 *         @p pack_format, this function expects a `spinel_eui64_t *` is included in variable arguments, while
 *         `spinel_datatype_unpack()` expects a `spinel_eui64_t **` is included.
 *       - For `SPINEL_DATATYPE_UTF8_C`, this function expects two arguments, the first of type `char *` and the
 *         second is of type `size_t` to indicate length of the provided buffer in the first argument just like
 *         `strncpy()`, while `spinel_datatype_unpack()` only expects a `const char **`.
 *
 * @sa spinel_datatype_vunpack_in_place()
 *
 */
SPINEL_API_EXTERN spinel_ssize_t spinel_datatype_unpack_in_place(const uint8_t *data_in,
                                                                 spinel_size_t  data_len,
                                                                 const char *   pack_format,
                                                                 ...);
SPINEL_API_EXTERN spinel_ssize_t spinel_datatype_vunpack(const uint8_t *data_in,
                                                         spinel_size_t  data_len,
                                                         const char *   pack_format,
                                                         va_list        args);
/**
 * This function parses spinel data similar to vsscanf().
 *
 * @param[in]   data_in     A pointer to the data to parse.
 * @param[in]   data_len    The length of @p data_in in bytes.
 * @param[in]   pack_format C string that contains a format string follows the same specification of spinel.
 * @param[in]   args        A value identifying a variable arguments list.
 *
 * @returns The parsed length in bytes.
 *
 * @note This function behaves different from `spinel_datatype_vunpack()`:
 *       - This function expects composite data arguments of pointer to data type, while `spinel_datatype_vunpack()`
 *         expects them of pointer to data type pointer. For example, if `SPINEL_DATATYPE_EUI64_C` is present in
 *         @p pack_format, this function expects a `spinel_eui64_t *` is included in variable arguments, while
 *         `spinel_datatype_vunpack()` expects a `spinel_eui64_t **` is included.
 *       - For `SPINEL_DATATYPE_UTF8_C`, this function expects two arguments, the first of type `char *` and the
 *         second is of type `size_t` to indicate length of the provided buffer in the first argument just like
 *         `strncpy()`, while `spinel_datatype_vunpack()` only expects a `const char **`.
 *
 * @sa spinel_datatype_unpack_in_place()
 *
 */
SPINEL_API_EXTERN spinel_ssize_t spinel_datatype_vunpack_in_place(const uint8_t *data_in,
                                                                  spinel_size_t  data_len,
                                                                  const char *   pack_format,
                                                                  va_list        args);

SPINEL_API_EXTERN spinel_ssize_t spinel_packed_uint_decode(const uint8_t *bytes,
                                                           spinel_size_t  len,
                                                           unsigned int * value_ptr);
SPINEL_API_EXTERN spinel_ssize_t spinel_packed_uint_encode(uint8_t *bytes, spinel_size_t len, unsigned int value);
SPINEL_API_EXTERN spinel_ssize_t spinel_packed_uint_size(unsigned int value);

SPINEL_API_EXTERN const char *spinel_next_packed_datatype(const char *pack_format);

// ----------------------------------------------------------------------------

SPINEL_API_EXTERN const char *spinel_command_to_cstr(spinel_command_t command);

SPINEL_API_EXTERN const char *spinel_prop_key_to_cstr(spinel_prop_key_t prop_key);

SPINEL_API_EXTERN const char *spinel_net_role_to_cstr(uint8_t net_role);

SPINEL_API_EXTERN const char *spinel_status_to_cstr(spinel_status_t status);


// ----------------------------------------------------------------------------

#if defined(__cplusplus)
}
#endif

#endif /* defined(SPINEL_HEADER_INCLUDED) */
