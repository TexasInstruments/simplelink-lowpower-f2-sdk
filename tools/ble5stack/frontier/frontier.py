#!/usr/bin/python
'''
/******************************************************************************
 @file  frontier.py

 @brief This file contains frontier source. The main purpose of this tool
        is to analyze the outputted map file of a stack project and determine
        the memory boundaries of that stack image.

        The tool will generate a compiler and linker output file which should
        be used by the corresponding application project to prevent the two
        images from overlapping.

        This script was tested using Python v2.7 32bit for Windows. An
        equivalent Windows executable is provided which was created using
        py2exe. For all other systems this script is provided as a starting
        point and is not guaranteed to work.

 Group: WCS, BTS
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2016-2025, Texas Instruments Incorporated
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
'''

import sys
import datetime
import argparse
import os
import re
from xml.etree import ElementTree
from xml.etree.ElementTree import Element
from xml.etree.ElementTree import SubElement

__version__ = '1.0.0'

ENTRY_IDX = 0
FLASH_IDX = 1
RAM_IDX = 2
SNV_IDX = 3


def checkOutput(file):
    '''
    Create directory if not already there
    '''
    outDir = os.path.dirname(file)
    if not os.path.exists(outDir):
        os.makedirs(outDir)


def getAddressPrev(fn, text):
    '''
    Get Flash and RAM addresses from the last generated output file. If
    the file does not exist or the values cannot be read then return a
    string value which will not match the current values during comparison
    '''
    if not os.path.exists(fn):
        return "INVALID"

    find_regex = re.compile("(?<=" + text + "=)(.*?)(?=\n)", re.S)

    with open(fn, 'r') as f:
        f_str = f.read()

    # Search for regex containing specific Flash or RAM symbol
    find_match = find_regex.search(f_str)
    if find_match:
        # Take value from first match. Assuming only one match
        return find_match.group(0)

    return "INVALID"


def getAddressMap(fn, text):
    '''
    Function used to search for specific strings in IAR map files
    '''
    with open(fn) as f:
        for line in f:
            if line.split() and text == line.split()[0]:
                return(line.split()[1])
    raise ValueError("Unable to find string " + text + " in " + os.path.basename(fn))

def getPageFromAddr(address):
    # remove trailing zeros
    adr = address.split("000")
    adr = adr[0].split("0x")
    return adr[1]

def checkFile(filepath):
  '''
  Function checks if the file exists
  '''
  print filepath
  # filepath = raw(filepath)
  # print filepath
  if os.path.isfile(filepath) != True:
    print "Couldn't found the file: " + filepath
    return -1
  else:
    return 0
def getAddressMapSNV(fn, text):
    '''
    Function used to search for specific strings in IAR map files
    '''
    addr = '0x0'
    num_pg = 0
    with open(fn) as f:
        for line in f:
            if line.split() and text == line.split()[0]:
                print "SNV FLASH" + line.split()[2]
                addr = (line.split()[2])

    if addr != '0x0':
        addr = addr.split('0x', 2)
        addr = addr[1]
        addr = addr.split('000', 2)
        addr = addr[0]
        return addr
    else :
        return '0'


    if addr == '0x1000':
        num_pg = '1'
    elif addr == '0x2000':
        num_pg = '2'
    else :
        num_pg = '0'

    return num_pg
    # raise ValueError("Unable to find string " + text + " in " + os.path.basename(fn))


def parseXML(args):
    '''
    Function parses the XML output file provided by CCS and returns
    a dictionary containing the addresses for the Flash Boundary,
    RAM boundary, as well as startup entry.
    '''
    xml = args.map
    flashStart = ' '
    sramStart = '0x0'
    entry = '0x0'

    linkerInfo = ElementTree.parse(xml)
    entry = ''
    sections = linkerInfo.findall('placement_map/memory_area')

    # Search for Flash and RAM Boundaries
    for section in sections:
        for node in section.getchildren():
            if node.tag == 'name':
                if node.text == 'FLASH':
                    # Flash section found, get address
                    test = section.find('usage_details/allocated_space/start_address')
                    flashStart = test.text
                    print "flashStart: " + flashStart
                    break
                if node.text == 'SRAM':
                    # SRAM section found, get address
                    test = section.find('usage_details/allocated_space/start_address')
                    print "SRAM: "+test.text
                    sramStart = test.text
                    break
                if args.boundary == 1:
                    if node.text == 'ENTRY':
                        # SRAM section found, get address
                        test = section.find('usage_details/allocated_space/start_address')
                        entry = test.text
                        print "entry: " + entry
                        break
            else:
                continue

    if args.boundary != 1: #### stack boundary
        # Search for startup_entry
        objectSectionsName = linkerInfo.findall('symbol_table/symbol')
        for section in objectSectionsName:
            for node in section.getchildren():
                if node.tag == 'name':
                    if node.text == "startup_entry":
                        print "We found the entry section"
                        # We found the entry section, let's read the value
                        test = section.find('value')
                        print "Entry: "+ test.text
                        entry = test.text
                        break
                else:
                    continue

    # Search for NVRAM entry
    #logical_group_list
    objectSectionsName = linkerInfo.findall('logical_group_list/logical_group')
    nvramSize = '0'
    for section in objectSectionsName:
        for node in section.getchildren():
            if node.tag == 'name':
                #if node.text == 'NV_FLASH':
                if node.text == ".TI.bound:NV_FLASH":
                    #print "CCS entry NV_FLASh found"
                    # We found the entry section, let's read the value
                    test = section.find('size')
                    #print "NV size" + test.text
                    nvramSize = test.text
                    nvramSize = getPageFromAddr(nvramSize)
                    #print nvramSize
                    break
            if node.tag == 'name' and args.boundary == 1: # if persistent app boundary
                #if node.text == 'NV_FLASH':
                print "Searching for flash start"
                if node.text == ".image_header":
                    # We found the entry section, let's read the value
                    test = section.find('load_address')
                    flashStart = test.text
                    print "flashStart" + flashStart
                    break
            else:
                 continue


    return(entry, flashStart, sramStart, nvramSize)

def stack_boundary(args):
    # Check for old frontier values
    curr = []
    prev_comp = [getAddressPrev(args.cdef, "ICALL_STACK0_ADDR"),
                getAddressPrev(args.cdef, "ICALL_STACK0_START"),
                getAddressPrev(args.cdef, "ICALL_RAM0_START"),
                getAddressPrev(args.cdef, "NV_FLASH_PAGES")]
    prev_link = [getAddressPrev(args.ldef, "ICALL_STACK0_ADDR"),
                getAddressPrev(args.ldef, "ICALL_STACK0_START"),
                getAddressPrev(args.ldef, "ICALL_RAM0_START"),
                getAddressPrev(args.ldef, "NV_FLASH_PAGES")]


    # Get new frontier values
    if args.ide == 'iar':
        curr.append(getAddressMap(args.map, "startup_entry"))
        curr.append(getAddressMap(args.map, "ROSTK$$Base"))
        curr.append(getAddressMap(args.map, "RWDATA$$Base"))
        curr.append(getAddressMapSNV(args.map, "NV_FLASH"))

    elif args.ide == 'ccs':
        curr = list(parseXML(args))
    else:
        sys.stderr.write("Error: Unknown IDE provided - " + args.ide)
        sys.exit(2)

    # If new values are different than previous, update the linker and compiler files.
    # ORDER OF ITEMS IN LIST MUST BE KEPT CONSISTENT
    if any(p != c for (p, c) in zip(prev_comp, curr)) or (prev_comp != prev_link):
        # Open and/or Create Output Files
        checkOutput(args.cdef)
        f_cdef = open(args.cdef, "w")
        checkOutput(args.ldef)
        f_ldef = open(args.ldef, "w")

        # Write Boundary File Headers
        f_cdef.write("/*\n")
        f_cdef.write("** Stack Frontier Generator 1.1.0 (%s" % str(datetime.datetime.now()) + ")\n")
        f_cdef.write("**\n")
        f_cdef.write("** WARNING - Auto-generated file. Modifications could be lost!\n")
        f_cdef.write("*/\n")
        f_cdef.write("\n")

        f_ldef.write("/*\n")
        f_ldef.write("** Stack Frontier Generator 1.1.0 (%s" % str(datetime.datetime.now()) + ")\n")
        f_ldef.write("**\n")
        f_ldef.write("** WARNING - Auto-generated file. Modifications could be lost!\n")
        f_ldef.write("*/\n")
        f_ldef.write("\n")

        # Write new values
        if args.ide == 'iar':
            #f_ldef.write("--config_def OSAL_SNV=" + OSAL_SNV + "\n")
            f_ldef.write("--config_def ICALL_RAM0_START=" + curr[RAM_IDX] + "\n")
            f_ldef.write("--config_def ICALL_STACK0_START=" + curr[FLASH_IDX] + "\n")
            f_ldef.write("--config_def ICALL_STACK0_ADDR=" + curr[ENTRY_IDX] + "\n")
            f_ldef.write("--config_def NV_FLASH_PAGES=" + curr[SNV_IDX] + "\n")
            f_cdef.write("-D ICALL_STACK0_ADDR=" + curr[ENTRY_IDX] + "\n")
            f_cdef.write("-D ICALL_STACK0_START=" + curr[FLASH_IDX] + "\n")
            f_cdef.write("-D ICALL_RAM0_START=" + curr[RAM_IDX] + "\n")
            f_cdef.write("-D NV_FLASH_PAGES=" + curr[SNV_IDX] + "\n")
        else:
            f_ldef.write("--define=ICALL_RAM0_START=" + curr[RAM_IDX] + "\n")
            f_ldef.write("--define=ICALL_STACK0_START=" + curr[FLASH_IDX] + "\n")
            f_ldef.write("--define=ICALL_STACK0_ADDR=" + curr[ENTRY_IDX] + "\n")
            f_ldef.write("--define=NV_FLASH_PAGES=" + curr[SNV_IDX] + "\n")
            f_cdef.write("--define=ICALL_STACK0_ADDR=" + curr[ENTRY_IDX] + "\n")
            f_cdef.write("--define=ICALL_STACK0_START=" + curr[FLASH_IDX] + "\n")
            f_cdef.write("--define=ICALL_RAM0_START=" + curr[RAM_IDX] + "\n")
            f_cdef.write("--define=NV_FLASH_PAGES=" + curr[SNV_IDX] + "\n")

        f_cdef.write("\n")
        f_ldef.write("\n")

        f_cdef.close
        f_ldef.close

def oad_tgt_boundary(args):
    # Check for old frontier values
    curr = []
    prev_comp = [getAddressPrev(args.cdef, "OAD_TGT_ENTRY_ADDR"),
                getAddressPrev(args.cdef, "OAD_TGT_FLASH_START"),
                getAddressPrev(args.cdef, "OAD_TGT_RAM0_START")]
    prev_link = [getAddressPrev(args.ldef, "OAD_TGT_ENTRY_ADDR"),
                getAddressPrev(args.ldef, "OAD_TGT_FLASH_START"),
                getAddressPrev(args.ldef, "OAD_TGT_RAM0_START")]

    # Get new frontier values
    if args.ide == 'iar':
        curr.append(getAddressMap(args.map, "__vector_table"))
        curr.append(getAddressMap(args.map, "ROSTK$$Base"))
        curr.append(getAddressMap(args.map, "RWDATA$$Base"))
    elif args.ide == 'ccs':
        curr = list(parseXML(args))
    else:
        sys.stderr.write("Error: Unknown IDE provided - " + args.ide)
        sys.exit(2)

    # first check if outpout file exists
    #checkFile(args.cdef)

    # If new values are different than previous, update the linker and compiler files.
    # ORDER OF ITEMS IN LIST MUST BE KEPT CONSISTENT
    if any(p != c for (p, c) in zip(prev_comp, curr)) or (prev_comp != prev_link):
        # Open Output Files
        f_cdef = open(args.cdef, "w")
        f_ldef = open(args.ldef, "w")

        # Write Boundary File Headers
        f_cdef.write("/*\n")
        f_cdef.write("** Stack Frontier Generator 1.1.0 (%s" % str(datetime.datetime.now()) + ")\n")
        f_cdef.write("**\n")
        f_cdef.write("** WARNING - Auto-generated file. Modifications could be lost!\n")
        f_cdef.write("*/\n")
        f_cdef.write("\n")

        f_ldef.write("/*\n")
        f_ldef.write("** Stack Frontier Generator 1.1.0 (%s" % str(datetime.datetime.now()) + ")\n")
        f_ldef.write("**\n")
        f_ldef.write("** WARNING - Auto-generated file. Modifications could be lost!\n")
        f_ldef.write("*/\n")
        f_ldef.write("\n")

        # Write new values
        if args.ide == 'iar':
            f_ldef.write("--config_def OAD_TGT_RAM0_START=" + curr[RAM_IDX] + "\n")
            f_ldef.write("--config_def OAD_TGT_FLASH_START=" + curr[FLASH_IDX] + "\n")
            f_ldef.write("--config_def OAD_TGT_ENTRY_ADDR=" + curr[ENTRY_IDX] + "\n")
            f_cdef.write("-D OAD_TGT_ENTRY_ADDR=" + curr[ENTRY_IDX] + "\n")
            f_cdef.write("-D OAD_TGT_FLASH_START=" + curr[FLASH_IDX] + "\n")
            f_cdef.write("-D OAD_TGT_RAM0_START=" + curr[RAM_IDX] + "\n")
        else:
            f_ldef.write("--define=OAD_TGT_RAM0_START=" + curr[RAM_IDX] + "\n")
            f_ldef.write("--define=OAD_TGT_FLASH_START=" + curr[FLASH_IDX] + "\n")
            f_ldef.write("--define=OAD_TGT_ENTRY_ADDR=" + curr[ENTRY_IDX] + "\n")
            f_cdef.write("--define=OAD_TGT_ENTRY_ADDR=" + curr[ENTRY_IDX] + "\n")
            f_cdef.write("--define=OAD_TGT_FLASH_START=" + curr[FLASH_IDX] + "\n")
            f_cdef.write("--define=OAD_TGT_RAM0_START=" + curr[RAM_IDX] + "\n")
        f_cdef.write("\n")
        f_ldef.write("\n")

        f_cdef.close
        f_ldef.close

def main():
    parser = argparse.ArgumentParser(prog='Frontier', description='Stack Frontier Generator')
    parser.add_argument("-b", "--boundary", type=int, choices=[0, 1],
                    help="increase output verbosity")
    parser.add_argument("ide", choices=['iar', 'ccs'], help="IDE")
    parser.add_argument("map", help="IAR Map file, or CCS linker info xml file")
    parser.add_argument("cdef", help="Compiler definition file")
    parser.add_argument("ldef", help="Linker definition file")

    args = parser.parse_args()

    if args.boundary == 1:
        oad_tgt_boundary(args)
    else:
        stack_boundary(args)


if __name__ == '__main__':
    main()
