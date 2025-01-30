##################
First Things First
##################

************
Prerequisite
************
Trusted Firmware M provides a reference implementation of platform security
architecture  reference implementation aligning with PSA Certified guidelines.
It is assumed that the reader is familiar with specifications can be found at
`Platform Security Architecture Resources <https://developer.arm.com/architectures/security-architectures/platform-security-architecture>`__.

The current TF-M implementation specifically targets TrustZone for ARMv8-M so a
good understanding of the v8-M architecture is also necessary. A good place to
get started with ARMv8-M is
`developer.arm.com <https://developer.arm.com/architectures/cpu-architecture/m-profile>`__.

**************************
Build and run instructions
**************************
Trusted Firmware M source code is available on
`git.trustedfirmware.org <https://git.trustedfirmware.org/TF-M/trusted-firmware-m.git/>`__.

To build & run TF-M:

    - Follow the this guide to set up and check your environment.
    - Follow the
      :doc:`Build instructions </building/tfm_build_instruction>`
      to compile and build the TF-M source.
    - Follow the :doc:`Run TF-M examples on Arm platforms </building/run_tfm_examples_on_arm_platforms>`
      for information on running the example.

To port TF-M to a another system or OS, follow the
:doc:`OS Integration Guide </integration_guide/index>`

:doc:`Contributing Guidelines </contributing/contributing_process>` contains guidance on how to
contribute to this project.

#########################
Set up build environments
#########################

TF-M officially supports a limited set of build environments and setups. In
this context, official support means that the environments listed below
are actively used by team members and active developers, hence users should
be able to recreate the same configurations by following the instructions
described below. In case of problems, the TF-M team provides support
only for these environments, but building in other environments can still be
possible.

The following environments are supported:

.. tabs::

    .. group-tab:: Linux

        1. version supported:

           Ubuntu 18.04 x64+

        2. install dependencies:

        .. code-block:: bash

            sudo apt-get install -y git curl wget build-essential libssl-dev python3 \
            python3-pip cmake make

        3. verify cmake version:

        .. code-block:: bash

            cmake --version

        .. note::

            Please download cmake 3.15 or later version from https://cmake.org/download/.

        4. add CMake path into environment:

        .. code-block:: bash

            export PATH=<CMake path>/bin:$PATH

    .. group-tab:: Windows

        1. version supported:

           Windows 10 x64

        2. install dependecies:

        - Git client latest version (https://git-scm.com/download/win)
        - CMake (`native Windows version <https://cmake.org/download/>`__)
        - GNU make (http://gnuwin32.sourceforge.net/packages/make.htm)
        - Python3 `(native Windows version) <https://www.python.org/downloads/>`__ and
          the pip package manager (from Python 3.4 it's included)

        3. add CMake path into environment:

        .. code-block:: bash

            set PATH=<CMake_Path>\bin;%PATH%

###########################
Install python dependencies
###########################

Clone the TF-M source code, and then install the TF-M's additional Python
dependencies.

.. tabs::

    .. group-tab:: Linux

        1. get the TF-M source code:

        .. code-block:: bash

            git clone https://git.trustedfirmware.org/TF-M/trusted-firmware-m.git

        2. TF-M's ``tools/requirements.txt`` file declares additional Python
           dependencies. Install them with ``pip3``:

        .. code-block:: bash

            pip3 install --upgrade pip
            cd trusted-firmware-m
            pip3 install -r tools/requirements.txt

    .. group-tab:: Windows

        1. get the TF-M source code:

        .. code-block:: bash

            git clone https://git.trustedfirmware.org/TF-M/trusted-firmware-m.git

        2. TF-M's ``tools/requirements.txt`` file declares additional Python
           dependencies. Install them with ``pip3``:

        .. code-block:: bash

            cd trusted-firmware-m
            pip3 install -r tools\requirements.txt

###################
Install a toolchain
###################

To compile TF-M code, at least one of the supported compiler toolchains have to
be available in the build environment. The currently supported compiler
versions are:

    - Arm Compiler v6.13 ~ v6.14, v6.18+

      .. tabs::

          .. group-tab:: Linux

              - Download the standalone packages from `here <https://developer.arm.com/products/software-development-tools/compilers/arm-compiler/downloads/version-6>`__.
              - Add Arm Compiler into environment:

                .. code-block:: bash

                    export PATH=<ARM_CLANG_PATH>/bin:$PATH
                    export ARM_PRODUCT_PATH=<ARM_CLANG_PATH>/sw/mappings

              - Configure proper tool variant and license.

          .. group-tab:: Windows

              - Download the standalone packages from `here <https://developer.arm.com/products/software-development-tools/compilers/arm-compiler/downloads/version-6>`__.
              - Add Arm Compiler into environment:

                .. code-block:: bash

                    set PATH=<ARM_CLANG_PATH>\bin;%PATH%
                    set ARM_PRODUCT_PATH=<ARM_CLANG_PATH>\sw\mappings

              - Configure proper tool variant and license.

      .. note::

          Arm compiler v6.15 ~ v6.17 may cause MemManage fault.
          This defect has been fixed since Arm compiler v6.18.
          See [SDCOMP-59788] in Armclang v6.18 `release note`__ for details.

          .. __: https://developer.arm.com/-/media/Arm%20Developer%20Community/Downloads/Arm%20Compiler%20for%20Embedded/6-18/Release%20notes%20for%20Arm%20Compiler%20for%20Embedded%206.pdf

    - GNU Arm compiler v7.3.1+

      .. tabs::

          .. group-tab:: Linux

              - Download the GNU Arm compiler from `here <https://developer.arm.com/open-source/gnu-toolchain/gnu-rm/downloads>`__.
              - Add GNU Arm into environment:

                .. code-block:: bash

                    export PATH=<GNU_ARM_PATH>/bin:$PATH

          .. group-tab:: Windows

              - Download the GNU Arm compiler from `here <https://developer.arm.com/open-source/gnu-toolchain/gnu-rm/downloads>`__.
              - Add GNU Arm into environment:

                .. code-block:: bash

                    set PATH=<GNU_ARM_PATH>\bin;%PATH%

      .. note::

          GNU Arm compiler version *10-2020-q4-major* has an issue in CMSE
          support. The bug is reported in `here <https://gcc.gnu.org/bugzilla/show_bug.cgi?id=99157>`__.
          Select other GNU Arm compiler versions instead.

    - IAR Arm compiler v8.42.x, v8.50.x

      .. tabs::

          .. group-tab:: Linux

              - Download IAR build tools from `here <https://www.iar.com/iar-embedded-workbench/build-tools-for-linux/>`__.
              - Add IAR Arm compiler into environment:

                .. code-block:: bash

                    export PATH=<IAR_COMPILER_PATH>/bin:$PATH

          .. group-tab:: Windows

              - Download IAR build tools from `here <https://www.iar.com/iar-embedded-workbench/#!?architecture=Arm>`__.
              - Add IAR Arm compiler into environment:

                .. code-block:: bash

                    set PATH=<IAR_COMPILER_PATH>\bin;%PATH%

#############################
Build AN521 regression sample
#############################

Here, we take building TF-M for AN521 platform with regression tests using GCC
as an example:

.. tabs::

    .. group-tab:: Linux

        Get the TF-M tests source code:

        .. code-block:: bash

            git clone https://git.trustedfirmware.org/TF-M/tf-m-tests.git

        Build SPE and NSPE.

        .. code-block:: bash

            cd </tf-m-tests/tests_reg>
            cmake -S spe -B build_spe -DTFM_PLATFORM=arm/mps2/an521 -DCONFIG_TFM_SOURCE_PATH=<TF-M source dir> \
                  -DCMAKE_BUILD_TYPE=Debug -DTFM_TOOLCHAIN_FILE=<Absolute path to>/toolchain_GNUARM.cmake \
                  -DTEST_S=ON -DTEST_NS=ON \
            cmake --build build_spe -- install

            cmake -S . -B build_test -DCONFIG_SPE_PATH=<absolute path to>/build_spe/api_ns \
                  -DCMAKE_BUILD_TYPE=Debug -DTFM_TOOLCHAIN_FILE=<Absolute path to>/toolchain_GNUARM.cmake
            cmake --build build_test

    .. group-tab:: Windows

        .. important::
            Use "/" instead of "\\" when assigning Windows paths to CMAKE
            variables, for example, use "c:/build" instead of "c:\\\\build".

        Get the TF-M tests source code:

        .. code-block:: bash

            git clone https://git.trustedfirmware.org/TF-M/tf-m-tests.git

        Build SPE and NSPE.

        .. code-block:: bash

            cd </tf-m-tests/tests_reg>
            cmake -G"Unix Makefiles" -S spe -B build_spe -DTFM_PLATFORM=arm/mps2/an521 -DCONFIG_TFM_SOURCE_PATH=<TF-M source dir> \
                  -DCMAKE_BUILD_TYPE=Debug -DTFM_TOOLCHAIN_FILE=<Absolute path to>/toolchain_GNUARM.cmake \
                  -DTEST_S=ON -DTEST_NS=ON \
            cmake --build build_spe -- install

            cmake -G"Unix Makefiles" -S . -B build_test -DCONFIG_SPE_PATH=<absolute path to>/build_spe/api_ns \
                  -DCMAKE_BUILD_TYPE=Debug -DTFM_TOOLCHAIN_FILE=<Absolute path to>/toolchain_GNUARM.cmake
            cmake --build build_test

        .. note::
           The latest Windows support long paths, but if you are less lucky
           then you can reduce paths by moving the build directory closer to
           the root by changing the ``-B`` option of the commands,  for example,
           to ``C:\build_spe`` and ``C:\build_test`` folders.

###########################
Run AN521 regression sample
###########################

Run the sample code on SSE-200 Fast-Model, using FVP_MPS2_AEMv8M provided by
Arm Development Studio.

.. note::

    Arm Development Studio is not essential to develop TF-M, you can skip this
    section if don't want to try on Arm develop boards.

.. tabs::

    .. group-tab:: Linux

        1. install Arm Development Studio to get the fast-model.

           Download Arm Development Studio from `here <https://developer.arm.com/tools-and-software/embedded/arm-development-studio>`__.

        2. Add ``bl2.axf`` and ``tfm_s_ns_signed.bin`` to symbol files in Debug
           Configuration menu.

        .. code-block:: bash

            <DS_PATH>/sw/models/bin/FVP_MPS2_AEMv8M  \
            --parameter fvp_mps2.platform_type=2 \
            --parameter cpu0.baseline=0 \
            --parameter cpu0.INITVTOR_S=0x10000000 \
            --parameter cpu0.semihosting-enable=0 \
            --parameter fvp_mps2.DISABLE_GATING=0 \
            --parameter fvp_mps2.telnetterminal0.start_telnet=1 \
            --parameter fvp_mps2.telnetterminal1.start_telnet=0 \
            --parameter fvp_mps2.telnetterminal2.start_telnet=0 \
            --parameter fvp_mps2.telnetterminal0.quiet=0 \
            --parameter fvp_mps2.telnetterminal1.quiet=1 \
            --parameter fvp_mps2.telnetterminal2.quiet=1 \
            --application cpu0=<build_spe>/api_ns/bin/bl2.axf \
            --data cpu0=<build_test>/tfm_s_ns_signed.bin@0x10080000

        .. note::

           The log is output to telnet by default.
           It can be also redirected to stdout by adding the following parameter.

           .. code-block:: bash

               --parameter fvp_mps2.UART0.out_file=/dev/stdout

           To automatically terminate the fast-model when it finishes running,
           you can add the following parameters:

           .. code-block:: bash

               --parameter fvp_mps2.UART0.shutdown_on_eot=1

    .. group-tab:: Windows

        1. install Arm Development Studio to get the fast-model.

           Download Arm Development Studio from `here <https://developer.arm.com/tools-and-software/embedded/arm-development-studio>`__.

        2. Add ``bl2.axf`` and ``tfm_s_ns_signed.bin`` to symbol files in Debug
           Configuration menu.

        .. code-block:: bash

            <DS_PATH>\sw\models\bin\FVP_MPS2_AEMv8M  \
            --parameter fvp_mps2.platform_type=2 \
            --parameter cpu0.baseline=0 \
            --parameter cpu0.INITVTOR_S=0x10000000 \
            --parameter cpu0.semihosting-enable=0 \
            --parameter fvp_mps2.DISABLE_GATING=0 \
            --parameter fvp_mps2.telnetterminal0.start_telnet=1 \
            --parameter fvp_mps2.telnetterminal1.start_telnet=0 \
            --parameter fvp_mps2.telnetterminal2.start_telnet=0 \
            --parameter fvp_mps2.telnetterminal0.quiet=0 \
            --parameter fvp_mps2.telnetterminal1.quiet=1 \
            --parameter fvp_mps2.telnetterminal2.quiet=1 \
            --application cpu0=<build_spe>/api_ns/bin/bl2.axf \
            --data cpu0=<build_test>/tfm_s_ns_signed.bin@0x10080000

        .. note::

           To automatically terminate the fast-model when it finishes running,
           you can add the following parameters:

           .. code-block:: bash

               --parameter fvp_mps2.UART0.shutdown_on_eot=1

After completing the procedure you should see the following messages on the
DAPLink UART (baud 115200 8n1)::

    ...
    #### Execute test suites for the Secure area ####
    Running Test Suite PSA protected storage S interface tests (TFM_S_PS_TEST_1XXX)...
    > Executing 'TFM_S_PS_TEST_1001'
    Description: 'Set interface'
    TEST: TFM_S_PS_TEST_1001 - PASSED!
    > Executing 'TFM_S_PS_TEST_1002'
    Description: 'Set interface with create flags'
    TEST: TFM_S_PS_TEST_1002 - PASSED!
    > Executing 'TFM_S_PS_TEST_1003'
    Description: 'Set interface with NULL data pointer'
    TEST: TFM_S_PS_TEST_1003 - PASSED!
    > Executing 'TFM_S_PS_TEST_1005'
    Description: 'Set interface with write once UID'
    TEST: TFM_S_PS_TEST_1005 - PASSED!
    ....

##########################
Tool & Dependency overview
##########################

To build the TF-M firmware the following tools are needed:

   - C compiler of supported toolchains
   - CMake version 3.15 or later
   - Git
   - gmake, aka GNU Make
   - Python v3.x
   - a set of python modules listed in ``tools/requiremtns.txt``

****************
Dependency chain
****************

.. uml::

   @startuml
    skinparam state {
      BackgroundColor #92AEE0
      FontColor black
      FontSize 16
      AttributeFontColor black
      AttributeFontSize 16
    }
    state fw as "Firmware" : TF-M binary
    state c_comp as "C Compiler" : C99
    state python as "Python" : v3.x

    fw --> c_comp
    fw --> CMake
    CMake --> gmake
    CMake --> Ninja
    fw --> cryptography
    fw --> pyasn1
    fw --> yaml
    fw --> jinja2
    fw --> cbor2
    fw --> click
    fw --> imgtool
    c_comp --> GCC
    c_comp --> CLANG
    c_comp --> IAR
    cryptography --> python
    pyasn1 --> python
    yaml --> python
    jinja2 --> python
    cbor2 --> python
    click --> python
    imgtool --> python
    kconfiglib --> python
   @enduml

.. rubric:: Next steps

Here are some next steps for exploring TF-M:

    - Detailed :doc:`Build instructions </building/tfm_build_instruction>`.
    - :doc:`IAR Build instructions </building/tfm_build_instruction_iar>`.
    - Try other :doc:`Samples and Demos </building/run_tfm_examples_on_arm_platforms>`.
    - :doc:`Documentation generation </building/documentation_generation>`.

--------------

*Copyright (c) 2017-2022, Arm Limited. All rights reserved.*
*Copyright (c) 2022 Cypress Semiconductor Corporation (an Infineon company)
or an affiliate of Cypress Semiconductor Corporation. All rights reserved.*
