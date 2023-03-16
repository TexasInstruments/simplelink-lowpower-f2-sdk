#-------------------------------------------------------------------------------
# Copyright (c) 2019-2022 Texas Instruments Incorporated
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

add_definitions(-DDeviceFamily_CC26X4)

list(APPEND ALL_SRC_C_S "${SOURCE_REPO}/ti/drivers/nvs/flash/FlashCC26X4_s.c")
