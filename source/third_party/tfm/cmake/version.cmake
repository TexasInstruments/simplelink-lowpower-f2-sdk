#-------------------------------------------------------------------------------
# Copyright (c) 2021-2022, Arm Limited. All rights reserved.
# Copyright (c) 2023, Texas Instruments Incorporated. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

# The 'TFM_VERSION_MANUAL' is used for fallback when Git tags are not available
set(TFM_VERSION_MANUAL "2.0.0")

execute_process(COMMAND git describe --tags --always
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE TFM_VERSION_FULL
    OUTPUT_STRIP_TRAILING_WHITESPACE)

# TI-TFM: Conan builds take place outside of the git repo which causes the 'git
# describe' command above to fail and set TFM_VERSION_FULL to an empty string.
# Set TFM_VERSION_FULL to a dummy value to prevent the 'string' commands below
# from failing.
if(TFM_VERSION_FULL STREQUAL "")
    set(TFM_VERSION_FULL "0.0.0")
endif()

# In a repository cloned with --no-tags option TFM_VERSION_FULL will be a hash
# only hence checking it for a tag format to accept as valid version.

string(FIND ${TFM_VERSION_FULL} "TF-M" TFM_TAG)
if(TFM_TAG EQUAL -1)
    set(TFM_VERSION_FULL v${TFM_VERSION_MANUAL})
    message(WARNING "Actual TF-M version is not available from Git repository. Settled to " ${TFM_VERSION_FULL})
endif()

string(REGEX REPLACE "TF-M" "" TFM_VERSION_FULL ${TFM_VERSION_FULL})
# remove a commit number
string(REGEX REPLACE "-[0-9]+-g" "+" TFM_VERSION_FULL ${TFM_VERSION_FULL})
string(REGEX MATCH "[0-9]+\\.[0-9]+\\.[0-9]+" TFM_VERSION ${TFM_VERSION_FULL})

# Check that manually set version is up to date
if (NOT TFM_VERSION_MANUAL STREQUAL TFM_VERSION)
    message(WARNING "TFM_VERSION_MANUAL mismatches to actual TF-M version. Please update TFM_VERSION_MANUAL in cmake/version.cmake")
endif()
