"""
/******************************************************************************
 @file  split_image_tool.py

 @brief This tool splits an application image into two parts. One with the
    CCFG and one without

 Group: WCS, BTS
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2022-2025, Texas Instruments Incorporated
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

import os
import argparse
import glob

__version__ = "0.0.1"

def verbose_log(args, log: str):
    if args.verbose:
        print("[SPLIT_IMAGE_TOOL]: " + log)


def find_objcopy(toolchain_root: str):
    toolchain_bin_dir = os.path.join(toolchain_root, "bin")
    objcopy_search_string = os.path.join(toolchain_bin_dir, "*objcopy*")
    search_results = glob.glob(objcopy_search_string)

    verbose_log(args, f"Toolchain binary directory = {toolchain_bin_dir}")
    verbose_log(args, f"Toolchain specific objcopy tool search string = {objcopy_search_string}")
    verbose_log(args, f"Objcopy search results = {search_results}")

    if len(search_results) != 1:
        raise Exception(f"toolchain specific objcopy binary could not be found with '{objcopy_search_string}'")

    verbose_log(args, f"Objcopy binary filename = {search_results[0]}")

    return search_results[0]


def main(args):
    # Verify the input file exists
    if not os.path.exists(args.input):
        raise Exception(f"'{args.input}' does not exist")

    # Split the input file into folder and filename
    input_base_dir, input_file_name = os.path.split(os.path.abspath(args.input))

    # Create output filenames based on the input filename
    output_app_name = os.path.join(input_base_dir, os.path.splitext(input_file_name)[0] + "_noccfg.bin")
    output_ccfg_name = os.path.join(input_base_dir, os.path.splitext(input_file_name)[0] + "_onlyccfg.bin")

    verbose_log(args, f"Output CCFG filename = {output_ccfg_name}")
    verbose_log(args, f"Output app(noCCFG) filename = {output_app_name}")

    # Search the TOOLCHAIN_ROOT for the objcopy tool
    objcopy_binary = find_objcopy(args.tc)

    app_cmd = f"{objcopy_binary} {args.input} --output-target binary {output_app_name} --remove-section=.ccfg"
    ccfg_cmd = f"{objcopy_binary} {args.input} --output-target binary {output_ccfg_name} --only-section=.ccfg"

    verbose_log(args, f"Split app command = '{app_cmd}'")
    verbose_log(args, f"Split CCFG command = '{ccfg_cmd}'")

    os.system(app_cmd)
    os.system(ccfg_cmd)


def parse_args():
    parser = argparse.ArgumentParser(prog="split_image_tool",
                                     description="""
                                     "Split an ELF format image into two binary images separated only by the CCFG section.
                                     Currently supported toolchains are [TIARM, TICLANG, GCC]
                                     """)
    parser.add_argument("-v", "--version", action="version",
                        version="%(prog)s " + __version__)
    parser.add_argument("--verbose", action="store_true")
    parser.add_argument("--tc", metavar="TOOLCHAIN_ROOT", required=True,
                        help="""Path to the toolchain root.
                                In most cases, your toolchain can be found in your CCS installation.
                                (ex) C:\\ti\\ccs1100\\ccs\\tools\\compiler\\<TOOLCHAIN>
                            """)
    parser.add_argument("--input", metavar="INPUT_IMAGE", required=True,
                        help="Path to the input image that is to be split up")
    args = parser.parse_args()

    return args


if __name__ == "__main__":
    args = parse_args()
    main(args)
