STM32H573I_DK
^^^^^^^^^^^^^^^

Discovery kit for IoT node with STM32H5 series
https://www.st.com/en/evaluation-tools/stm32h573i-dk.html

Configuration and Build
"""""""""""""""""""""""

GNUARM/ARMCLANG/IARARM compilation is available for this target.
and build the selected configuration as follow.

``> mkdir build && cd build``

``> cmake .. -DTFM_PLATFORM=stm/stm32h573i_dk -DTFM_TOOLCHAIN_FILE=../toolchain_GNUARM.cmake -G"Unix Makefiles"``

``> cmake --build ./  -- install``

-------------

*Copyright (c) 2023, STMicroelectronics. All rights reserved.*
*SPDX-License-Identifier: BSD-3-Clause*
