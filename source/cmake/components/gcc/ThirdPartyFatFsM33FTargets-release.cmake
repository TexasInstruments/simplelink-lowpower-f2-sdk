#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "ThirdPartyFatFs::fatfs_m33f" for configuration "Release"
set_property(TARGET ThirdPartyFatFs::fatfs_m33f APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(ThirdPartyFatFs::fatfs_m33f PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/source/third_party/fatfs/lib/gcc/m33f/fatfs.a"
  )

list(APPEND _IMPORT_CHECK_TARGETS ThirdPartyFatFs::fatfs_m33f )
list(APPEND _IMPORT_CHECK_FILES_FOR_ThirdPartyFatFs::fatfs_m33f "${_IMPORT_PREFIX}/source/third_party/fatfs/lib/gcc/m33f/fatfs.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
