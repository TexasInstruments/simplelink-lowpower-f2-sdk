NuMaker-PFM-M2354
==================

Building TF-M
-------------

Build TF-M regression test with M2354 by following commands:

.. code:: bash

    $ cd tf-m-test/tests_reg
    $ cmake -S spe -B build_spe \
            -G"Unix Makefiles" \
            -DTFM_PLATFORM=nuvoton/m2354 \
            -DTFM_TOOLCHAIN_FILE=[tf-m path]/toolchain_GNUARM.cmake \
            -DCMAKE_BUILD_TYPE=Release \
            -DTEST_S=ON -DTEST_NS=ON
    $ cmake --build build --parallel -- install
    $ cmake -S . -B build_test \
            -G"Unix Makefiles" \
            -DCONFIG_SPE_PATH=[tf-m-tests path]/tests_reg/build_spe/api_ns \
            -DTFM_TOOLCHAIN_FILE=cmake/toolchain_ns_GNUARM.cmake \
            -DCMAKE_BUILD_TYPE=Release \
    $ cmake --build build_test --parallel -- install

Define TEST_NS=ON or TEST_S=ON for non-secure or secure regression test.
CMAKE_BUILD_TYPE could be "Release", "RelWithDebInfo" or "Minsizerel"
Other cmake parameters should not be changed.

Flashing Image with Nuvoton NuLink Tool
---------------------------------------

NuMaker-PFM-M2354 board supports on board ICE called NuLink.
Connect NuLink to PC with USB cable and using
`NuLink Command Tool <https://github.com/OpenNuvoton/Nuvoton_Tools>`__
to flashing the image.

The commands are as follows:

.. code:: doscon

    > NuLink_M2354 -C
    > NuLink_M2354 -E ALL
    > NuLink_M2354 -W NSCBA 0x80000 0
    > NuLink_M2354 -S
    > NuLink_M2354 -C
    > NuLink_M2354 -W APROM .\build_spe\bin\bl2.bin 0
    > NuLink_M2354 -W APROM .\build_spe\bin\tfm_s_signed.bin 0 0x20000 0
    > NuLink_M2354 -W APROMNS .\build_test\bin\tfm_ns_signed.bin 0

--------------

*Copyright (c) 2021-2023, Nuvoton Technology Corp. All rights reserved.*
