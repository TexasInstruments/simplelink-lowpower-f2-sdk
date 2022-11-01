#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "ThirdPartyFatFs::fatfs_m4" for configuration "Release"
set_property(TARGET ThirdPartyFatFs::fatfs_m4 APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(ThirdPartyFatFs::fatfs_m4 PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/source/third_party/fatfs/lib/iar/m4/fatfs.a"
  )

list(APPEND _IMPORT_CHECK_TARGETS ThirdPartyFatFs::fatfs_m4 )
list(APPEND _IMPORT_CHECK_FILES_FOR_ThirdPartyFatFs::fatfs_m4 "${_IMPORT_PREFIX}/source/third_party/fatfs/lib/iar/m4/fatfs.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
