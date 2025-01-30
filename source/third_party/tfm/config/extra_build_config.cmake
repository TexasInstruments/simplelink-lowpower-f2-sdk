#-------------------------------------------------------------------------------
# Copyright (c) 2023, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

# This file lists the available config options for extra generated files and
# Secure Partitions. These configs can be set from build command line or CMake
# files/configs.
# Although these configs are set as CACHE variables here by default, it is
# recommended to use list(APPEND) command in CMake files/configs because they
# are treated as lists in the build system. For example:
#
#     list(APPEND TFM_EXTRA_PARTITION_PATHS <Your extra Secure Partition path>)
#
# This can avoid the unexpected override between multiple extra sources.

############################### Extra generated files ##########################
if (NOT TFM_EXTRA_GENERATED_FILE_LIST_PATH)
    set(TFM_EXTRA_GENERATED_FILE_LIST_PATH  ""    CACHE PATH     "List of Paths to extra generated file list. Appended to stardard TFM generated file list.")
endif()

############################### Extra partitions ###############################
if (NOT TFM_EXTRA_MANIFEST_LIST_FILES)
    set(TFM_EXTRA_MANIFEST_LIST_FILES       ""    CACHE FILEPATH "List of Extra manifest list file(s), used to list extra Secure Partition manifests.")
endif()

if (NOT TFM_EXTRA_PARTITION_PATHS)
    set(TFM_EXTRA_PARTITION_PATHS           ""    CACHE PATH     "List of extra Secure Partitions directories. An extra Secure Parition folder contains source code, CMakeLists.txt and manifest files")
endif()
