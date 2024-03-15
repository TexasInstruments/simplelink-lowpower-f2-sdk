# Helper file for setting GCC_ARMCOMPILER based on directory name
# which makefile is running from.
#
# Note, this is only internally used and users don't see this file

mkfile_path := $(abspath $(firstword $(MAKEFILE_LIST)))
mkfile_dir := $(dir $(mkfile_path))

ifeq ($(findstring /gcc/a53,$(mkfile_dir)),/gcc/a53)
# Cortex-A64
GCC_ARMCOMPILER = $(GCC_ARMCOMPILER_A64)
else
ifeq ($(findstring /gcc/a,$(mkfile_dir)),/gcc/a)
# Cortex-A32
GCC_ARMCOMPILER = $(GCC_ARMCOMPILER_A32)
endif
endif

ifeq ($(findstring /gcc/m,$(mkfile_dir)),/gcc/m)
# Cortex-M
GCC_ARMCOMPILER = $(GCC_ARMCOMPILER_M)
endif
