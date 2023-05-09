include imports.mak

# Parallelisation is handled inside cmake --build
.NOTPARALLEL:

# These variables need to be environment variables for CMake
export FREERTOS_INSTALL_DIR
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

# All clean goals remove the whole lib folder, so we can do this for just one toolchain
clean: clean-sdk-specific
ifneq ($(wildcard build),)
	@ echo "Cleaning generated CMake content"
	-@ $(CMAKE) --build build/ticlang --target clean > $(DEVNULL)
endif
	@ rm -rf build

build-sdk-specific:
ifdef ENABLE_TFM_BUILD
	@$(MAKE) -C tfm_s/cc26x4
endif
	@$(MAKE) -C source/ti/grlib

clean-sdk-specific:
	@$(MAKE) -C source/ti/grlib clean
ifdef ENABLE_TFM_BUILD
	@$(MAKE) -C tfm_s/cc26x4 clean
endif
