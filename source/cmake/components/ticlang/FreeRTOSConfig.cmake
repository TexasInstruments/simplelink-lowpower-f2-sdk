
####### Expanded from @PACKAGE_INIT@ by configure_package_config_file() #######
####### Any changes to this file will be overwritten by the next CMake run ####
####### The input file was ConfigFreeRTOS.cmake.in                            ########

get_filename_component(PACKAGE_PREFIX_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../../" ABSOLUTE)

macro(set_and_check _var _file)
  set(${_var} "${_file}")
  if(NOT EXISTS "${_file}")
    message(FATAL_ERROR "File or directory ${_file} referenced by variable ${_var} does not exist !")
  endif()
endmacro()

macro(check_required_components _NAME)
  foreach(comp ${${_NAME}_FIND_COMPONENTS})
    if(NOT ${_NAME}_${comp}_FOUND)
      if(${_NAME}_FIND_REQUIRED_${comp})
        set(${_NAME}_FOUND FALSE)
      endif()
    endif()
  endforeach()
endmacro()

####################################################################################

list(APPEND SUPPORTED_COMPONENTS freertos)
include("${CMAKE_CURRENT_LIST_DIR}/FreeRTOSALLTargets.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/FreeRTOSM0PTargets.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/FreeRTOSM4Targets.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/FreeRTOSM4FTargets.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/FreeRTOSM33FTargets.cmake")
check_required_components(FreeRTOS)

set_target_properties(FreeRTOS::freertos PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES "${FREERTOS_INSTALL_DIR}/source/third_party/freertos/include"
)
target_include_directories(FreeRTOS::freertos_m0p
  INTERFACE
    "$<INSTALL_INTERFACE:${FREERTOS_INSTALL_DIR}/source/third_party/freertos/portable/IAR/ARM_CM0"
)
target_include_directories(FreeRTOS::freertos_m4
  INTERFACE
    "$<INSTALL_INTERFACE:${FREERTOS_INSTALL_DIR}/source/third_party/freertos/portable/IAR/ARM_CM3"
)
target_include_directories(FreeRTOS::freertos_m4f
  INTERFACE
    "$<INSTALL_INTERFACE:${FREERTOS_INSTALL_DIR}/source/third_party/freertos/portable/IAR/ARM_CM4F"
)
target_include_directories(FreeRTOS::freertos_m33f
  INTERFACE
    "$<INSTALL_INTERFACE:${FREERTOS_INSTALL_DIR}/source/third_party/freertos/portable/IAR/ARM_CM33F_NTZ/non_secure"
)
target_include_directories(FreeRTOS::freertos_m33f_tfm
  INTERFACE
    "$<INSTALL_INTERFACE:${FREERTOS_INSTALL_DIR}/source/third_party/freertos/portable/IAR/ARM_CM33F_NTZ/non_secure"
)
