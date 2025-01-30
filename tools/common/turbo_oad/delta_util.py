"""
/******************************************************************************

 @file delta_util.py

 @brief This file contains a collection of functions used for the creation of
        delta images. Some functions are modified implementations of bsdiff v4
        source code (http://www.daemonology.net/bsdiff/) and can be found in
        the bsdiff function section.

 Group: LPRF SW RND
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2019-2025, Texas Instruments Incorporated
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions
 are met:

 *  Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.

 *  Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.

 *  Neither the name of Texas Instruments Incorporated nor the names of
    its contributors may be used to endorse or promote products derived
    from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 ******************************************************************************

 Original license and copyright statement of bsdiff follow.

 ******************************************************************************

 Copyright 2003-2005 Colin Percival
 All rights reserved

 Redistribution and use in source and binary forms, with or without
 modification, are permitted providing that the following conditions
 are met:
 1. Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
 2. Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.

 ******************************************************************************
 
 
 *****************************************************************************/
"""

import struct

# -----------------------------------------------------------------------------
#                                   Constants
# -----------------------------------------------------------------------------

DELTA_PAYLOAD_MAGIC = "DELTAIMG"

UINT16_MAX = 65535
INT16_MAX = 32767
INT16_MIN = -32768

# ------------------------------------------------------------------------------
#                                   Functions
# ------------------------------------------------------------------------------

# ====================
# bsdiff functions
# ====================


def split(I, V, start, bufLen, h):
    """
    Larsson and Sadakane's adaptation of Jon L. Bentley and M. Douglas
    McIlroy's Ternary-Split Quicksort

    Reference: https://dl.acm.org/citation.cfm?id=172710

    Remarks:
        Variable names referenced are left as implemented in the open-source
        code for reference to bsdiff, and the papers this was implemented in.
    """

    i = 0
    j = 0
    k = 0
    x = 0
    jj = 0
    kk = 0

    if bufLen < 16:
        j = 1

        k = start
        while k < start + bufLen:
            j = 1
            x = V[I[k] + h]

            i = 1
            while k + i < start + bufLen:
                if V[I[k + i] + h] < x:
                    x = V[I[k + i] + h]
                    j = 0

                if V[I[k + i] + h] == x:
                    I[k + j], I[k + i] = I[k + i], I[k + j]
                    j += 1

                i += 1

            for i in range(j):
                V[I[k + i]] = k + j - 1

            if j == 1:
                I[k] = -1

            k += j

        return

    x = V[I[int(start + bufLen / 2)] + h]

    for i in range(start, start + bufLen, 1):
        if V[I[i] + h] < x:
            jj += 1

        if V[I[i] + h] == x:
            kk += 1

    jj += start
    kk += jj

    i = start
    j = 0
    k = 0

    while i < jj:
        if V[I[i] + h] < x:
            i += 1
        elif V[I[i] + h] == x:
            I[i], I[jj + j] = I[jj + j], I[i]
            j += 1
        else:
            I[i], I[kk + k] = I[kk + k], I[i]
            k += 1

    while jj + j < kk:
        if V[I[jj + j] + h] == x:
            j += 1
        else:
            I[jj + j], I[kk + k] = I[kk + k], I[jj + j]
            k += 1

    if jj > start:
        split(I, V, start, jj - start, h)

    for i in range(0, kk - jj, 1):
        V[I[jj + i]] = kk - 1

    if jj == kk - 1:
        I[jj] = -1

    if start + bufLen > kk:
        split(I, V, kk, start + bufLen - kk, h)


def qsufsort(I, V, old, oldsize):
    """
    Larsson and Sadakane's suffix sorting (qsufsort)
    Paper: http://www.larsson.dogma.net/ssrev-tr.pdf

    Keyword Arguments:
        - I -- The sorted suffix array (32 bit integers)
        - V -- An auxiliary array that holds the positions of the
               suffixes in I (32 bit integers)
        - old -- A byte array containing the contents to be used for
                 suffix sorting. The paper referenced stores this
                 information (X) in V, but there is no need to transfer
                 the contents since this array is used later on when
                 performing the partial matching step
        - oldsize -- An integer that holds the input size. This corresponds
                     to argument 'n' referenced in the paper.

    Remarks:
        Variable names referenced are left as implemented in the open-source
        code for reference to bsdiff, and the papers this was implemented in.
        Comments regarding relevant parts are inserted to give clarity to the
        terse variable names

        Arguments 'k' and 'l' referenced in the paper are not used. The
        reason is that k and l represent the alphabet length to perform
        suffix sorting on, which in this case is is the full range of
        integers [0,INT_MAX].

        variable reference:
        - h: The order (of the suffixes) when sorting lexicographically,
             considering only the initial h symbols of each suffix

        On return, I stores the sorted suffix array and V is I's inverse
        permutation.
    """

    buckets = [0 for i in range(256)]

    # Create histogram the count of each symbol that appear in the old image
    for i in range(0, oldsize, 1):
        buckets[old[i]] += 1

    # Transform the histogram such that if a symbol
    for i in range(1, 256, 1):
        buckets[i] += buckets[i-1]

    for i in range(255, 0, -1):
        buckets[i] = buckets[i-1]

    buckets[0] = 0

    for i in range(oldsize):
        buckets[old[i]] += 1
        I[buckets[old[i]]] = i

    I[0] = oldsize

    for i in range(oldsize):
        V[i] = buckets[old[i]]

    V[oldsize] = 0

    for i in range(1, 256, 1):
        if buckets[i] == buckets[i-1] + 1:
            I[buckets[i]] = -1

    I[0] = -1

    h = 1
    while I[0] != -(oldsize + 1):   # While not single combined sorted group
        sl = 0                      # sl is the length of sorted groups

        i = 0                       # i is the first position of group
        while i < oldsize + 1:
            if I[i] < 0:
                sl -= I[i]          # Add negated length to sl
                i -= I[i]           # Skip over sorted group
            else:
                if sl:
                    I[i - sl] = -sl  # Combine sorted groups before pi

                sl = V[I[i]] + 1 - i
                split(I, V, i, sl, h)
                i += sl
                sl = 0

        if sl:                      # If the array ends with a sorted group
            I[i - sl] = -sl         # Combine sorted groups at end of I

        h += h                      # Double sorted-depth

    # Reconstruct suffix array from inverse
    for i in range(oldsize + 1):
        I[V[i]] = i


def match_len(old, old_index, old_size, new, new_index, new_size):
    """
    Returns the length of an exact match between to byte arrays
    from the given starting locations

    :param old: byte array representing the current 'old' image
    :param old_index: location to start performing the comparison
    :param old_size: maximum length to compare for the given array
    :param new: byte array representing the updated 'new' image
    :param new_index: location to start performing the comparison
    :param new_size: maximum length to compare for the given array
    :return: the length of an exact byte-wise match
    """

    i = 0

    while i < old_size and i < new_size:
        if old[old_index + i] != new[new_index + i]:
            break
        i += 1

    return i


def search(I, old, old_size, new, new_index, new_size, st, en):
    """
    Searches the suffix array for the longest suffix from the given byte

    :param I: the suffix array of the 'old' byte array
    :param old: byte array of the current 'old' image
    :param old_size: length from the starting index of comparison
                     to the end of the array
    :param new: byte array of the updated 'new' image
    :param new_index: location of the byte to find the longest suffix
    :param new_size: length from the starting index of comparison
                     to the end of the array
    :param st: starting boundary for the binary search
    :param en: ending boundary for the binary search
    :return: a tuple of the longest match length and starting index
             in the old image
    """

    # Terminate when fully traversed the tree for the given byte
    # (found the longest suffix)
    if (en - st) < 2:
        # en may not equal st for even arrays, hence have to compare which
        # match is better
        x = match_len(old, I[st], old_size - I[st], new, new_index, new_size)
        y = match_len(old, I[en], old_size - I[en], new, new_index, new_size)

        # Return the match longest match length and the starting index of
        # the match in the old image
        if x > y:
            return (x, I[st])
        else:
            return (y, I[en])

    # Traverse through the suffix tree (via binary search in array form) to
    # find the longest suffix from the initial byte given (at new_index)
    x = int(st + (en - st) / 2)
    compare_len = min(old_size - I[x], new_size)

    for i in range(compare_len):
        if old[I[x] + i] < new[new_index + i]:
            return search(I, old, old_size, new, new_index, new_size, x, en)
        elif old[I[x] + i] > new[new_index + i]:
            return search(I, old, old_size, new, new_index, new_size, st, x)

    return search(I, old, old_size, new, new_index, new_size, st, x)


def create_delta(old_img, new_img):
    """
    Creates a delta binary from the two input buffers

    :param old_img: binary data to base the delta from
    :param new_img: binary data of the new version
    :return: byte array consisting of the delta image payload
    """

    # Initialize suffix array I (and auxiliary array V) for approximate matching
    I = [0 for x in range(len(old_img) + 1)]
    V = [0 for x in range(len(old_img) + 1)]
    qsufsort(I, V, old_img, len(old_img))

    delta_img = bytearray()

    # Payloads start with a magic word to indicate the start of the stream
    delta_img += DELTA_PAYLOAD_MAGIC.encode("utf-8")

    # Initialize data for delta encoding
    scan = 0
    suffix_len = 0
    suffix_pos = 0
    last_scan = 0
    last_pos = 0
    last_offset = 0

    while scan < len(new_img):
        old_score = 0
        scan += suffix_len
        score_scan = scan

        while scan < len(new_img):
            # Find the longest suffix for the scan byte
            suffix_len, suffix_pos = search(I, old_img, len(old_img),
                                            new_img, scan, len(new_img) - scan,
                                            0, len(old_img))

            # Calculate the score of the match (amount of exactly matching bytes)
            while score_scan < scan + suffix_len:
                if score_scan + last_offset < len(old_img) and \
                        old_img[score_scan + last_offset] == new_img[score_scan]:
                    old_score += 1

                score_scan += 1

            # If we have a perfect match, or if the approximate match
            # differs by more than 8 bytes
            if (suffix_len == old_score and suffix_len != 0) or \
                    (suffix_len > old_score + 8):
                break

            # Move the scan pointer forward and decrease the score (these bytes
            # will be accounted for when scanning the backwards-extension later)
            if scan + last_offset < len(old_img) and \
                    old_img[scan + last_offset] == new_img[scan]:
                old_score -= 1

            scan += 1

        if suffix_len != old_score or scan == len(new_img):
            score = 0
            score_forward_ext = 0
            len_forward_ext = 0

            # Scan the forward extension of the exact match, allowing
            # for mismatches
            i = 0
            while last_scan + i < scan and last_pos + i < len(old_img):
                if old_img[last_pos + i] == new_img[last_scan + i]:
                    score += 1

                i += 1
                if score * 2 - i > score_forward_ext * 2 - len_forward_ext:
                    score_forward_ext = score
                    len_forward_ext = i

            # Scan the backward extension of the exact match, allowing
            # for mismatches
            len_backward_ext = 0
            if scan < len(new_img):
                score = 0
                score_backward_ext = 0
                i = 1
                while scan >= last_scan + i and suffix_pos >= i:
                    if old_img[suffix_pos - i] == new_img[scan - i]:
                        score += 1

                    if score * 2 - i > score_backward_ext * 2 - len_backward_ext:
                        score_backward_ext = score
                        len_backward_ext = i

                    i += 1

            # Ensure forward and backward extensions are not overlapping
            if last_scan + len_forward_ext > scan - len_backward_ext:
                overlap = (last_scan + len_forward_ext) - (scan - len_backward_ext)
                score = 0
                sub_score = 0
                len_sub = 0

                for i in range(overlap):
                    if new_img[last_scan + len_forward_ext - overlap + i] == \
                            old_img[last_pos + len_forward_ext - overlap + i]:
                        score += 1
                    if new_img[scan - len_backward_ext + i] == \
                            old_img[suffix_pos - len_backward_ext + i]:
                        score -= 1
                    if score > sub_score:
                        sub_score = score
                        len_sub = i + 1

                len_forward_ext += len_sub - overlap
                len_backward_ext -= len_sub

            # Write delta block
            extra_read_len = (scan - len_backward_ext) - (last_scan + len_forward_ext)
            seek_offset = (suffix_pos - len_backward_ext) - (last_pos + len_forward_ext)

            diff_buffer = bytearray()
            for i in range(len_forward_ext):
                diff_buffer.append((new_img[last_scan + i] - old_img[last_pos + i]) % 256)

            extra_buffer = bytearray()
            for i in range(extra_read_len):
                extra_buffer.append((new_img[last_scan + len_forward_ext + i]) % 256)

            delta_img += add_delta_block((len_forward_ext, extra_read_len, seek_offset),
                                         diff_buffer, extra_buffer)

            last_scan = scan - len_backward_ext
            last_pos = suffix_pos - len_backward_ext
            last_offset = suffix_pos - scan

    return delta_img


# ====================
# ====================


def add_delta_block(ctrl_block, diff_block, extra_block):
    """
    Creates a delta block from the initially given blocks generated
    by the approximate matching process. If control block instructions
    are larger than two bytes, multiple delta blocks are generated.

    :param ctrl_block: tuple consisting the diff and extra block read lengths
                       along with the old image seek offset
    :param diff_block: byte array consisting of the difference block bytes
    :param extra_block: byte array consisting of the extra block bytes
    :return: returns a byte array consisting of a single (or sequence) of
             delta blocks
    """

    delta_block = bytearray()
    rle_buffer = bytearray()
    diff_read_len, extra_read_len, seek_offset = ctrl_block

    # Diff block writing
    if diff_read_len > UINT16_MAX:
        delta_block += struct.pack("HHh", UINT16_MAX, 0, 0)
        run_length_encode(diff_block[:UINT16_MAX], rle_buffer)

        delta_block += rle_buffer
        delta_block += add_delta_block((diff_read_len - UINT16_MAX,
                                        extra_read_len, seek_offset),
                                       diff_block[UINT16_MAX:], extra_block)
        return delta_block
    else:
        delta_block += struct.pack("H", diff_read_len)
        run_length_encode(diff_block[:diff_read_len], rle_buffer)

    # Extra block writing
    if extra_read_len > UINT16_MAX:
        delta_block += struct.pack("Hh", UINT16_MAX, 0)

        delta_block += rle_buffer
        delta_block += extra_block[:UINT16_MAX]
        delta_block += add_delta_block((diff_read_len,
                                        extra_read_len - UINT16_MAX,
                                        seek_offset), diff_block,
                                       extra_block[UINT16_MAX:])
        return delta_block
    else:
        delta_block += struct.pack("H", extra_read_len)

    # Seek offset encoding
    if seek_offset > INT16_MAX:
        delta_block += struct.pack("h", INT16_MAX)

        rle_buffer = bytearray()
        run_length_encode(diff_block[:diff_read_len], rle_buffer)

        delta_block += rle_buffer
        delta_block += extra_block[:extra_read_len]
        delta_block += add_delta_block((0, 0, seek_offset - INT16_MAX),
                                       diff_block[diff_read_len:],
                                       extra_block[extra_read_len:])
    elif seek_offset < INT16_MIN:
        delta_block += struct.pack("h", INT16_MIN)

        rle_buffer = bytearray()
        run_length_encode(diff_block[:diff_read_len], rle_buffer)

        delta_block += rle_buffer
        delta_block += extra_block[:extra_read_len]
        delta_block += add_delta_block((0, 0, seek_offset - INT16_MIN),
                                       diff_block[diff_read_len:],
                                       extra_block[extra_read_len:])
    else:
        delta_block += struct.pack("h", seek_offset)
        delta_block += rle_buffer
        delta_block += extra_block[:extra_read_len]

    return delta_block


def run_length_encode(input_buffer, output_buffer):
    """
    Encodes the input byte array buffer using a variant of run length encoding
    optimized for Turbo OAD. Specifically, only byte values of 0x00 are encoded
    with run length encoding, while all other values are represented literally
    in the encoded data stream.

    The format consists of BB[LL], where BB is the byte value and [LL] is the
    amount of consecutive zeros. LL field applies only when BB is 0x00.

    For example, a stream of "03 00 00 00 ff 00 aa 00 00 00 00 00 00 00 00 4a"
    is encoded to the stream "03 00 03 ff 00 01 aa 00 08 4a"

    :param input_buffer: byte array containing data to run-length encode
    :param output_buffer: output byte array with the run-length encoded data
    :return:
    """

    write_count = 0

    for byte in input_buffer:
        if byte != 0:
            # Add length byte if previous byte value is 0x00
            if write_count > 0:
                output_buffer.append(write_count)

            output_buffer.append(byte)
            write_count = 0
        else:
            if write_count == 0:
                output_buffer.append(byte)

            # Length field is only one byte, so record a new byte value
            elif write_count >= 255:
                output_buffer.append(write_count)
                output_buffer.append(byte)
                write_count = 0

            write_count += 1

    # Flush buffer
    if write_count > 0:
        output_buffer.append(write_count)
