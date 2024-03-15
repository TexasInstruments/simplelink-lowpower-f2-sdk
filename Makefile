include imports.mak

# Parallelisation is handled inside cmake --build
.NOTPARALLEL:

# These variables need to be environment variables for CMake
export TICLANG_ARMCOMPILER
export GCC_ARMCOMPILER
export IAR_ARMCOMPILER

GENERATOR="Unix Makefiles"

build: build-ticlang build-gcc build-iar build-sdk-specific

build-ticlang:
ifdef TICLANG_ARMCOMPILER
	@ $(CMAKE) -G$(GENERATOR) . -B build/ticlang --log-level=VERBOSE
	@ $(CMAKE) --build build/ticlang
	@ $(CMAKE) --install build/ticlang
else
	@ echo "Skipping TICLANG build, no compiler defined"
endif

build-gcc:
ifdef GCC_ARMCOMPILER
	@ $(CMAKE) -G$(GENERATOR) . -B build/gcc --log-level=VERBOSE
	@ $(CMAKE) --build build/gcc
	@ $(CMAKE) --install build/gcc
else
	@ echo "Skipping GCC build, no compiler defined"
endif

build-iar:
ifdef IAR_ARMCOMPILER
	@ $(CMAKE) -G$(GENERATOR) . -B build/iar --log-level=VERBOSE
	@ $(CMAKE) --build build/iar
	@ $(CMAKE) --install build/iar
else
	@ echo "Skipping IAR build, no compiler defined"
endif

clean: clean-sdk-specific
ifneq ($(wildcard build/ticlang),)
	@ echo "Cleaning generated CMake TICLANG content"
	-@ $(CMAKE) --build build/ticlang --target clean > $(DEVNULL)
endif
ifneq ($(wildcard build/iar),)
	@ echo "Cleaning generated CMake IAR content"
	-@ $(CMAKE) --build build/iar --target clean > $(DEVNULL)
endif
ifneq ($(wildcard build/gcc),)
	@ echo "Cleaning generated CMake GCC content"
	-@ $(CMAKE) --build build/gcc --target clean > $(DEVNULL)
endif
	@ $(RMDIR) build

build-sdk-specific:
ifeq ($(ENABLE_TFM_BUILD), 1)
	@$(MAKE) -C tfm_s/cc26x4
endif
	@$(MAKE) -C source/ti/grlib

clean-sdk-specific:
	@$(MAKE) -C source/ti/grlib clean
ifeq ($(ENABLE_TFM_BUILD), 1)
	@$(MAKE) -C tfm_s/cc26x4 clean
endif
