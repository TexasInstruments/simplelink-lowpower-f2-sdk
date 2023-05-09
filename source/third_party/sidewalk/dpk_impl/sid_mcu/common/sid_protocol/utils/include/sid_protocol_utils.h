/*
 * Copyright 2020 Amazon.com, Inc. or its affiliates. All rights reserved.
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

#ifndef SID_PROTOCOL_UTILS_H
#define SID_PROTOCOL_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    SET_BIT,
    CLR_BIT,
    CHK_BIT
} bit_opt_t;

/** @brief Convert unsigned integer to ascii c-style string in specified base
 *
 *  @param unsigned integer to convert to ascii string
 *  @param pointer to buffer to store result
 *  @param base to convert to (2 to 16)
 *  @return string containing number converted to ascii base
 */
uint8_t *utils_uint_to_str(uint32_t value, uint8_t *result, uint8_t base);

/** @brief Convert unsigned integer to ascii c-style string in specified base
 *
 *  Skips over characters that are not numbers which helps
 *  to automatically convert NMEA float values to binary integers.
 *  @param string to convert to int
 *  @param base to convert from (2 to 16)
 *  @return unsigned integer value of numeric string
 */
uint32_t utils_str_to_uint(uint8_t *str, uint8_t base);

/** @brief Convert integer to ascii c-style string in specified base
 *
 *  Skips over characters that are not numbers which helps
 *  to automatically convert NMEA float values to binary integers.
 *
 *  @param string to convert to int
 *  @param base to convert from (2 to 16)
 *  @return integer value of numeric string
 */
int32_t utils_str_to_int(uint8_t *str, uint8_t base);

/** @brief Print buffer in hexadecimal
 *
 *  @param pointer to buffer
 *  @param size of the buffer
 *  @return none
 */
void utils_print_hex(uint8_t * buf, uint8_t len);

/** @brief parse integer in the passed string buffer
 *
 *  @param pointer to buffer
 *  @return integer present in the buffer
 */
int32_t utils_parse_input_num(uint8_t *buf);

/** @brief convert buffer in hex string format to
 *  string format
 *
 *  @param pointer to hex string buffer
 *  @param pointer to buffer
 *  @return number of bytes filled in output buffer
 */
int16_t utils_parse_hex_in_str(uint8_t *out_buf, uint8_t *buf);

/** @brief convert buffer in hex string format to
 *  decimal string format
 *
 *  @param pointer to hex string buffer
 *  @param output buffer pointer to decimal string
 *  @return number of bytes filled in output buffer
 */
int8_t utils_hex_str_to_bin(uint8_t *str, uint8_t *dst);

/** @brief convert buffer in decimal string format to
 *  hex string format
 *
 *  @param pointer to decimal string buffer
 *  @param output buffer pointer to hex string
 *  @return number of bytes filled in output buffer
 */
void utils_bin_to_hex_str(uint8_t *dst, uint8_t *src, uint16_t len);

/** @brief switch endianess of the buffer
 *
 *  @param pointer to the buffer
 *  @param size of the buffer
 *  @return none
 */
void utils_flip_bytes(uint8_t *ptr, uint8_t sz);

/** @brief Performs bitwise operations on passed uint32_t
 *
 *  Sets the nth bit (n>=0)
 *  Clears the nth bit (n>=0)
 *  Returns if the nth bit (n>=0) is 1 or not
 *
 *  @param pointer to the uint32_t
 *  @param bit position to check or alter
 *  @param desired operation on the bit position
 *  @return always true for set and clear, for check
 *  operation the result whether the bit is set or not
 */
bool utils_set_clr_chk_bit(uint32_t *val, int n, bit_opt_t opr);

/**
 * @brief Convert formatted ASCII char mac string to byte buffer using format string
 *
 * @param str Formatted ascii char null-terminated string: "11:22:33:44:AA:bb" e.g.
 * @param out Output byte buffer
 * @return 0-0xF if hex ascii param is valid or 0xFF if param is invalid
 */
uint8_t utils_parse_mac_str(const uint8_t *str, uint8_t *out);

/** @brief Read 2 bytes from a buffer
 *
 *  @param pointer to the buffer
 *  @param position in the buffer to read from
 *  @return unsigned short value
 */
uint16_t utils_read_word(uint8_t *buf, uint32_t pos);

/** @brief Read 4 bytes to a buffer
 *
 *  @param pointer to the buffer
 *  @param position in the buffer to write
 *  @return unsigned int
 */
uint32_t utils_read_dword(uint8_t* buf, uint32_t pos);

/** @brief Write 2 bytes to a buffer
 *
 *  @param pointer to the buffer
 *  @param value to write
 *  @return none
 */
void utils_write_word(uint8_t* buf, uint16_t val);

/** @brief Write 4 bytes to a buffer
 *
 *  @param pointer to the buffer
 *  @param value to write
 *  @return none
 */
void utils_write_dword(uint8_t* buf, uint32_t val);
#ifdef __cplusplus
}
#endif

#endif
