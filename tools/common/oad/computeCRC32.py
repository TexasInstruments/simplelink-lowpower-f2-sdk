"""
/******************************************************************************
 @file  computeCRC32.py

 @brief This is a script containing helper functions for the BLE OAD Image tool related to the CRC 

 Group: WCS, BTS
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2017-2025, Texas Instruments Incorporated
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
 
 
 *****************************************************************************/
"""
import zlib
import imgBinUtil as util

def computeCRC32(fileName):
    """
    Computes CRC32 over entire file and updates the image header
    """
    crcbytes = crc32_withOffset(fileName, util.CRC_DATA_START_ADDR)
    crcData = int(crcbytes, 16)
    util.writeBytes(fileName, util.FIXED_CRC_ADDR, crcData, util.IMG_LEN_SIZE)

    return 0


def crc32(fileName):
    """
    Computes CRC32 over entire file
    """
    with open(fileName,"rb") as filePtr:
        filePtr.seek(0,0)
        content = filePtr.readlines()
    crc = None
    for eachLine in content:
      if not crc:
          crc = zlib.crc32(eachLine)
      else:
          crc = zlib.crc32(eachLine, crc)

    # Return 8 digits crc
    return format((crc & 0xFFFFFFFF), '08x')


def crc32_withOffset(fileName, dataOffset):
    """
    Computes CRC32 over entire file starting from the dataOffset
    """
    # Read data from file
    with open(fileName,"rb") as filePtr:
        filePtr.seek(dataOffset, 0)  
        content = filePtr.readlines()
    crc = None
    for eachLine in content:
      if not crc:
          crc = zlib.crc32(eachLine)
      else:
          crc = zlib.crc32(eachLine, crc)

    # Return 8 digits crc
    return format((crc & 0xFFFFFFFF), '08x')
