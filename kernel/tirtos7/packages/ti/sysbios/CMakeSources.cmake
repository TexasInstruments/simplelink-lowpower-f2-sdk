# Copyright (c) 2022, Texas Instruments Incorporated All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# * Redistributions of source code must retain the above copyright notice, this
#   list of conditions and the following disclaimer.
#
# * Redistributions in binary form must reproduce the above copyright notice,
#   this list of conditions and the following disclaimer in the documentation
#   and/or other materials provided with the distribution.
#
# * Neither the name of Texas Instruments Incorporated nor the names of its
#   contributors may be used to endorse or promote products derived from this
#   software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
cmake_minimum_required(VERSION ${TI_MIN_CMAKE_VERSION})

# These Boot files from the vxm folder are included in all libraries
set(SOURCES_COMMON_gcc family/arm/vxm/boot/boot_gcc.s family/arm/vxm/boot/startup_gcc.c)
set(SOURCES_COMMON_iar family/arm/vxm/boot/cstartup_M.c family/arm/vxm/boot/boot_iar.s family/arm/vxm/boot/cmain_iar.s)
set(SOURCES_COMMON_ticlang family/arm/vxm/boot/boot_cortex_m.c)

# These files are architecture-specific. Their family/ prefixes are added in CMakeLists.txt
set(SOURCES_gcc TaskSupport_asm_gcc.s Hwi_asm_gcc.s Hwi_asm_switch_gcc.s)
set(SOURCES_iar TaskSupport_asm_iar.s Hwi_asm_iar.s Hwi_asm_switch_iar.s)
set(SOURCES_ticlang ${SOURCES_gcc})

# These files are compiled into the separate TFM-enabled library
set(SOURCES_gcc_ns family/arm/v8m/TaskSupport_asm_gcc.s family/arm/v8m/Hwi_asm_gcc_ns.s
                    family/arm/v8m/Hwi_asm_switch_gcc.s
)
set(SOURCES_iar_ns family/arm/v8m/TaskSupport_asm_iar.s family/arm/v8m/Hwi_asm_iar_ns.s
                    family/arm/v8m/Hwi_asm_switch_iar.s
)
set(SOURCES_ticlang_ns ${SOURCES_gcc_ns})
