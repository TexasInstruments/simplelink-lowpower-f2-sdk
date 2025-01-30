#-------------------------------------------------------------------------------
# Copyright (c) 2020-2022 Cypress Semiconductor Corporation (an Infineon company)
# or an affiliate of Cypress Semiconductor Corporation. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

fetch_remote_library(
    LIB_NAME                mtb-pdl-cat1
    LIB_SOURCE_PATH_VAR     CY_MTB_PDL_CAT1_LIB_PATH
    LIB_PATCH_DIR           ${CMAKE_CURRENT_LIST_DIR}
    LIB_FORCE_PATCH         CY_MTB_PDL_CAT1_FORCE_PATCH
    FETCH_CONTENT_ARGS
        GIT_REPOSITORY      https://github.com/Infineon/mtb-pdl-cat1.git
        GIT_TAG             ${CY_MTB_PDL_CAT1_LIB_VERSION}
        GIT_SHALLOW         TRUE
        GIT_PROGRESS        TRUE
        GIT_SUBMODULES      ""
)
