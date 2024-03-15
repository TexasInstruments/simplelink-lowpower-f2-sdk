# Turbo OAD Image Tool

## Table of Contents

* [Introduction](#Introduction)
* [Software Prerequisites](#Software%20Prerequisites)
* [Usage](#Usage)

## Introduction

The oad_image_tool adds the OAD metadata to an image to prepare it for
over the transfer. The OAD metadata is a collection of information about the image
including length, start address, and crc that will be used throughout the OAD Process.

The toad_image_tool adds delta image metadata to the OAD binary, and performs
delta encoding and image compression on the binary. Delta images should only be
used for devices that have Turbo OAD enabled for the respective OAD protocol.

This tool is intended to run as a post build step. Please refer to the
Turbo OAD enabled sample applications to for a reference invocation of the tool.

Further documentation can be found the SDK's User Guide. There is a chapter
dedicated to Turbo OAD which will cover the tool.

## Software Prerequisites

The Turbo OAD image tool is shipped in both source and binary form. It is recommended
to run the tool in binary form unless source changes are absolutely required.

The following is required to run the binary image:

**Linux:**
 - The binary is 64-bit, will run without modification on 64-bit systems
 - 32-bit distributions are not supported at this time

**Windows**
 - The binary is 64-bit. 32-bit versions of Windows are not supported at this time

**MacOS**
 - WARNING: MacOS support is experimental.
 - The binary is 64-bit, as of MacOS Sierra this is the only supported ArchType.

In order to run the tool from source, the following dependencies are needed:

 - Python 3.x
 - The Python packages listed in the requirements.txt in the tools/common/oad folder.
   These can be installed via pip.

## Usage

More information on the tool can be found in the SDK User's Guide in the Turbo OAD
section. Invoke the tool with the `-h` option to display the help menu and to
see required and optional arguments.
