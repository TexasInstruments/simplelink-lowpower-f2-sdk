#
# Set location of various cgtools
#
# These variables can be set here or on the command line.  Paths must not
# have spaces.
#
# The various *_ARMCOMPILER variables, in addition to pointing to
# their respective locations, also serve as "switches" for disabling a build
# using those cgtools. To disable a build using a specific cgtool, either set
# the cgtool's variable to empty or delete/comment-out its definition:
#     TICLANG_ARMCOMPILER ?=
# or
#     #TICLANG_ARMCOMPILER ?= ...
#
# If a cgtool's *_ARMCOMPILER variable is set (non-empty), various sub-makes
# in the installation will attempt to build with that cgtool.  This means
# that if multiple *_ARMCOMPILER cgtool variables are set, the sub-makes
# will build using each non-empty *_ARMCOMPILER cgtool.
#

XDC_INSTALL_DIR        ?= /home/username/ti/xdctools_3_62_01_15_core
SYSCONFIG_TOOL         ?= /home/username/ti/ccs1220/ccs/utils/sysconfig_1.15.0/sysconfig_cli.sh

FREERTOS_INSTALL_DIR   ?= /home/username/FreeRTOSv202104.00
CMAKE                  ?= /home/username/cmake-3.21.3/bin/cmake
PYTHON                 ?= python3

TICLANG_ARMCOMPILER    ?= /home/username/ti/ccs1220/ccs/tools/compiler/ti-cgt-armllvm_2.1.2.LTS-0
GCC_ARMCOMPILER        ?= /home/username/ti/ccs1220/ccs/tools/compiler/9.2019.q4.major-0

# The IAR compiler is not supported on Linux
# IAR_ARMCOMPILER      ?=

# Uncomment this to enable the TFM build
# ENABLE_TFM_BUILD=1

# For Linux
RM      = rm -f
RMDIR   = rm -rf
DEVNULL = /dev/null
ECHOBLANKLINE = echo
