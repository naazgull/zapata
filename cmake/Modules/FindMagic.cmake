# FindMagic.cmake
#
# Locates the libmagic file(1) library and its headers, and provides the
# imported target Magic::Magic.
#
# Result variables:
#   Magic_FOUND        - True if both the library and headers were found.
#   MAGIC_LIBRARIES    - Full path to libmagic.
#   MAGIC_INCLUDE_DIRS - Directory containing <magic.h>.

find_path(MAGIC_INCLUDE_DIR NAMES magic.h)
find_library(MAGIC_LIBRARIES NAMES magic)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Magic
  REQUIRED_VARS MAGIC_LIBRARIES MAGIC_INCLUDE_DIR
)

if(Magic_FOUND AND NOT TARGET Magic::Magic)
  set(MAGIC_INCLUDE_DIRS "${MAGIC_INCLUDE_DIR}")
  add_library(Magic::Magic UNKNOWN IMPORTED)
  set_target_properties(Magic::Magic PROPERTIES
    IMPORTED_LOCATION "${MAGIC_LIBRARIES}"
    INTERFACE_INCLUDE_DIRECTORIES "${MAGIC_INCLUDE_DIR}"
  )
endif()
