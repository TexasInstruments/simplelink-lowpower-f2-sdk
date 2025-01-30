"""
/******************************************************************************

 @file toad_image_tool.py

 @brief This tool generates the Turbo OAD (delta) image files

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
 
 
 *****************************************************************************/
"""

import argparse
import textwrap
import os
import sys
import struct
import ntpath
import tempfile
import json
from collections import namedtuple

import delta_util

# Import OAD image tool modules
sys.path.append("../oad")
import computeCRC32
import signUtil
import imgBinUtil as util

# -----------------------------------------------------------------------------
#                                   Global Variables
# -----------------------------------------------------------------------------

# Program version number
__version__ = "1.0"
__prog__ = "toad_image_tool"

# -----------------------------------------------------------------------------
#                                   Constants
# -----------------------------------------------------------------------------

FIXED_HEADER_FORMAT = "<8sIBBHBBBBIIIIIHH"
DELTA_SEG_FORMAT = "<BHBIBBBBII"

SEG_TYPE_DELTA = 5
IMG_PAYLOAD_LEN = 12
DELTA_SEG_LEN = 0x14
DELTA_IMG_MEMORY_CFG_OFFCHIP = 0x01
DELTA_IMG_MEMORY_CFG_ONCHIP = 0x02

MEMORY_CFG_OFFCHIP = "offchip"
MEMORY_CFG_ONCHIP = "onchip"


# -----------------------------------------------------------------------------
#                                   Functions
# -----------------------------------------------------------------------------


def print_console_header():
    """Prints the program entry header"""

    print("************************************************")
    print("TI Turbo OAD Image Tool")
    print("Version: " + __version__)
    print("************************************************")


def print_args_info(old_img_path, new_img_path, delta_path, memory_cfg):
    """Prints out the main arguments of the program"""

    print("OAD Configuration: " + memory_cfg)
    print("Old Image: " + ntpath.basename(old_img_path))
    print("New Image: " + ntpath.basename(new_img_path))
    print("Output Delta Image: " + ntpath.basename(delta_path))
    print("************************************************")
    print("Runtime Output:")


def print_comp_savings(original_len, compressed_len):
    """Prints out the size savings of the delta image"""

    savings = (1.0 - float(compressed_len)/original_len) * 100.0

    print()
    print("New Image Length:   {:>9,} {:>5}".format(original_len, "bytes"))
    print("Delta Image Length: {:>9,} {:>5}".format(compressed_len, "bytes"))
    print("                   ================")
    print("Savings:  {:.4}%".format(savings))
    print()
    print("************************************************")


def print_delta_header(path):
    """
    This function prints out information contained in an delta image header

    :param path: Path to the image file
    :return:
    """

    try:
        with open(path, "rb") as file:
            image = bytearray(file.read())

    except (OSError, IOError) as e:
        # Skip verbose printing if error
        return

    delta_seg_offset = util.getSegAddr(path, SEG_TYPE_DELTA)
    header_tuple = namedtuple("deltaSegHdr",
                              """segType wirelessTech rfuSeg payloadLen isDeltaImg
                              toadMetaVer toadVer memoryCfg oldImgCrc newImgLen""")
    delta_header = header_tuple._make(struct.unpack(DELTA_SEG_FORMAT,
                                                    image[delta_seg_offset:(delta_seg_offset + DELTA_SEG_LEN)]))

    hdr = delta_header._asdict()
    # Checks the the wireless tech type
    tech_value = hdr["wirelessTech"]
    hdr["wirelessTech"] = ''
    if tech_value == 0:
        hdr["wirelessTech"] = "No Wireless Technology"
    if tech_value & 1 == 0:
        hdr["wirelessTech"] += "[BLE]"
    if tech_value & 2 == 0:
        hdr["wirelessTech"] += "[TIMAC Sub1G]"
    if tech_value & 4 == 0:
        hdr["wirelessTech"] += "[TIMAC 2.4G]"
    if tech_value & 8 == 0:
        hdr["wirelessTech"] += "[Zigbee]"
    if tech_value & 16 == 0:
        hdr["wirelessTech"] += "[RF4CE]"
    if tech_value & 32 == 0:
        hdr["wirelessTech"] += "[Thread]"
    if tech_value & 64 == 0:
        hdr["wirelessTech"] += "[EasyLink]"

    hdr["memoryCfg"] = "Off-Chip" if hdr["memoryCfg"] == DELTA_IMG_MEMORY_CFG_OFFCHIP else "On-Chip"

    print(textwrap.dedent("""
    ____________________________
             Delta HDR
    ____________________________
    Field            |      Value
    Segment Type     |      Delta Segment
    Wireless Tech    |      {wirelessTech}
    Segment Length   |      0x{payloadLen:X}
    isDeltaImg       |      {isDeltaImg}
    toadMetaVer      |      {toadMetaVer}
    toadVer          |      {toadVer}
    memoryCfg        |      {memoryCfg}
    oldImgCrc        |      0x{oldImgCrc:X}
    newImgLen        |      0x{newImgLen:X}
        """.format(**hdr)))


def write_delta_image(old_header, new_header, new_img, delta_img):
    """
    Creates the OAD header segments for a Turbo OAD delta image

    :param old_header: byte array of the old image header
    :param new_header: byte array of the new image header
    :param new_img: byte array of the new image
    :param delta_img: byte array containing the delta of the current and new application images
    :return:
    """

    total_len = len(new_header) + len(delta_img)

    # Agama OAD binaries are word aligned
    remaining_bytes = 0 if total_len % 4 == 0 else 4 - (total_len % 4)
    pad_buffer = bytearray([0xFF for x in range(remaining_bytes)])
    total_len += len(pad_buffer)

    # A temporary file needs to be created since the OAD image tool operates
    # off of file paths for adding signature and CRC
    temp_new_img = tempfile.NamedTemporaryFile(delete=False)
    temp_new_img.write(new_img)
    temp_new_img.close()

    with open(vargs.output, "w+b") as outFile:
        new_fixed_hdr_tuple = namedtuple("FixedImgHdr",
                                         """imgID crc32 bimVer metaVer techType imgCpStat
                                         crcStat imgType imgNo imgVld len prgEntry softVer
                                         imgEndAddr hdrLen rfu2""")
        new_img_fixed_hdr = new_fixed_hdr_tuple._make(struct.unpack(FIXED_HEADER_FORMAT,
                                                                    new_header[:util.FIXED_HDR_LEN]))

        # Calculate CRC fields for oldImgCrc validation on initiating OAD transfer
        # old_img_data_crc used for validating delta image is compatible with old image
        # new_img_data_crc used for OAD header CRC calculation done on OAD completion
        old_img_data_crc = computeCRC32.crc32_withOffset(vargs.old_img, len(old_header))
        old_img_data_crc = int(old_img_data_crc, 16)
        new_img_data_crc = computeCRC32.crc32_withOffset(vargs.new_img, len(new_header))
        new_img_data_crc = int(new_img_data_crc, 16)

        # Delta information segment
        delta_offset = util.getSegAddr(temp_new_img.name, SEG_TYPE_DELTA)

        delta_hdr_tuple = namedtuple("deltaSegHdr",
                                     """segType wirelessTech rfuSeg payloadLen isDeltaImg
                                     toadMetaVer toadVer memoryCfg oldImgCrc newImgLen""")
        delta_seg_header = delta_hdr_tuple._make(struct.unpack(DELTA_SEG_FORMAT,
                                                               new_header[delta_offset:(delta_offset + DELTA_SEG_LEN)]))
        delta_seg_header = delta_seg_header._replace(isDeltaImg=True, oldImgCrc=new_img_data_crc,
                                                     newImgLen=new_img_fixed_hdr.len)

        # Update header with new signature and CRC after adding delta
        with open(temp_new_img.name, "w+b") as f:
            updated_new_img = bytearray(new_img)
            updated_new_img = updated_new_img.replace(updated_new_img[delta_offset:(delta_offset + DELTA_SEG_LEN)],
                                                      struct.pack(DELTA_SEG_FORMAT, *delta_seg_header), 1)
            f.write(updated_new_img)

        if util.isSecure(temp_new_img.name):
            if vargs.key_file:
                signUtil.signImage(temp_new_img.name, vargs.key_file)
            else:
                raise Exception("Key file not specified for secure image!")

        computeCRC32.computeCRC32(temp_new_img.name)

        # Update header for delta image
        with open(temp_new_img.name, "rb") as f:
            new_header = f.read(len(new_header))

        new_fixed_hdr_tuple = namedtuple("FixedImgHdr",
                                         """imgID crc32 bimVer metaVer techType imgCpStat
                                         crcStat imgType imgNo imgVld len prgEntry softVer
                                         imgEndAddr hdrLen rfu2""")
        new_img_fixed_hdr = new_fixed_hdr_tuple._make(struct.unpack(FIXED_HEADER_FORMAT,
                                                                    new_header[:util.FIXED_HDR_LEN]))

        # Update delta image with length of the delta image instead of the full image
        new_img_fixed_hdr = new_img_fixed_hdr._replace(len=total_len)

        delta_hdr_tuple = namedtuple("deltaSegHdr",
                                     """segType wirelessTech rfuSeg payloadLen isDeltaImg
                                     toadMetaVer toadVer memoryCfg oldImgCrc newImgLen""")
        delta_seg_header = delta_hdr_tuple._make(struct.unpack(DELTA_SEG_FORMAT,
                                                               new_header[delta_offset:(delta_offset + DELTA_SEG_LEN)]))
        delta_seg_header = delta_seg_header._replace(oldImgCrc=old_img_data_crc)

        delta_header = bytearray(new_header)
        delta_header = delta_header.replace(new_header[:util.FIXED_HDR_LEN],
                                            struct.pack(FIXED_HEADER_FORMAT, *new_img_fixed_hdr), 1)
        delta_header = delta_header.replace(new_header[delta_offset:(delta_offset + DELTA_SEG_LEN)],
                                            struct.pack(DELTA_SEG_FORMAT, *delta_seg_header), 1)

        outFile.write(delta_header)
        outFile.write(delta_img)
        outFile.write(pad_buffer)
        os.remove(temp_new_img.name)


def open_binary(path):
    """
    Reads an OAD image from the specified path

    :param path: Path to the image file
    :return: A tuple consisting of the OAD image header and the OAD image data
    """

    try:
        with open(path, "rb") as file:
            header_len = util.getOverheadSize(path) + IMG_PAYLOAD_LEN
            header = bytearray(file.read(header_len))

            file.seek(0x00, os.SEEK_SET)
            image = bytearray(file.read())

    except (OSError, IOError) as e:
        print("Fatal Error: -- {:s}. Exiting.".format(e.strerror))
        sys.exit(1)

    return (header, image)


def parse_json_args(args):
    """
    Parses a JSON file and adds additional arguments to the arguments dictionary

    :param args: Arguments dictionary
    :return:
    """

    try:
        with open(args.json, "rb") as file:
            if sys.version_info.minor < 6:
                json_content = file.read()
                json_args = json.loads(json_content.decode("utf-8"))
            else:
                json_args = json.load(file)
            args.__dict__.update(json_args)

    except (OSError, IOError) as e:
        print("Fatal Error: -- {:s}. Exiting.".format(e.strerror))
        sys.exit(1)


def parse_cmdline_args():
    """Parses command line arguments"""

    parser = argparse.ArgumentParser(
        prog=__prog__,
        formatter_class=argparse.RawDescriptionHelpFormatter,
        description=textwrap.dedent('''
          Generates a Turbo OAD delta image from two OAD images
        '''),
        epilog=textwrap.dedent('''
          Example:
            %(prog)s -oimg app_v1.bin -nimg app_v2.bin -o app_v2.dim -m offchip
        '''))

    parser.add_argument("-oimg", "--old_img",
                        help="Path to an OAD binary of the old image running on the device")
    parser.add_argument("-nimg", "--new_img", required=True,
                        help="Path to an OAD binary of the new image")
    parser.add_argument("-o", "--output", required=True,
                        help="Output path to write the delta image")
    parser.add_argument("-j", "--json", help="Path to json file containing program arguments")
    parser.add_argument("-k", "--key_file", nargs="?",
                        help="Provide the location of the file containing your private key")
    parser.add_argument("-m", "--memory_cfg", choices=[MEMORY_CFG_OFFCHIP, MEMORY_CFG_ONCHIP],
                        help="OAD memory configuration (offchip/onchip)")
    parser.add_argument("-v", "--version", action="version", version="%(prog)s " + __version__)
    parser.add_argument("-verbose", "--verbose", action="store_true")

    return parser.parse_args()


if __name__ == "__main__":
    vargs = parse_cmdline_args()

    # Load additional arguments if passed a JSON file
    if not(vargs.json is None or vargs.json == ""):
        parse_json_args(vargs)

        # exit early if turbo oad is not enabled
        if not vargs.enabled:
            # exit silently
            sys.exit(0)

    # Ensure required arguments are passed in either via cmd line or json file:
    if (vargs.old_img is None or vargs.old_img == "") or (vargs.memory_cfg is None or vargs.memory_cfg == ""):
        if not(vargs.json is None or vargs.json == ""):
            print("Info: Path to old image is not specified. Delta image not created.")
            sys.exit(0)
        else:
            print("Error: --memory_cfg and --old_img must be specified")
            sys.exit(1)

    # Prevent creation of on-chip images
    if vargs.memory_cfg == MEMORY_CFG_ONCHIP:
        print("Error: On-chip memory configuration is not supported yet")
        sys.exit(1)

    # first, print a neat header
    print_console_header()

    # Open image binaries and store data for delta encoding
    old_img_header, old_img = open_binary(vargs.old_img)
    new_img_header, new_img = open_binary(vargs.new_img)

    # Display input file and output file name info
    print_args_info(vargs.old_img, vargs.new_img, vargs.output, vargs.memory_cfg)

    # OAD headers are excluded from the delta image
    delta_img = delta_util.create_delta(old_img[len(old_img_header):],
                                        new_img[len(new_img_header):])
    write_delta_image(old_img_header, new_img_header, new_img, delta_img)

    if vargs.verbose:
        print_delta_header(vargs.output)

    # Print out statistics
    delta_img_size = (len(new_img_header) + len(delta_img))
    print_comp_savings(len(new_img), delta_img_size)

    # Print out warning if it is not advantages to perform a delta update
    savings = (1.0 - float(delta_img_size) / len(new_img)) * 100.0
    if savings < 25.0:
        print("Warning: Recommended to use regular OAD since size savings " +
              "is less than 25% of the new image size")

