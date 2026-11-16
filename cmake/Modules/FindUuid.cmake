# FindUuid.cmake
#
# Locates the libuuid library and its headers, and provides the imported
# target Uuid::Uuid.
#
# Result variables:
#   Uuid_FOUND         - True if both the library and headers were found.
#   UUID_LIBRARIES     - Full path to libuuid.
#   UUID_INCLUDE_DIRS  - Directory containing <uuid/uuid.h>.

find_path(UUID_INCLUDE_DIR NAMES uuid/uuid.h)
find_library(UUID_LIBRARIES NAMES uuid)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Uuid
  REQUIRED_VARS UUID_LIBRARIES UUID_INCLUDE_DIR
)

if(Uuid_FOUND AND NOT TARGET Uuid::Uuid)
  set(UUID_INCLUDE_DIRS "${UUID_INCLUDE_DIR}")
  add_library(Uuid::Uuid UNKNOWN IMPORTED)
  set_target_properties(Uuid::Uuid PROPERTIES
    IMPORTED_LOCATION "${UUID_LIBRARIES}"
    INTERFACE_INCLUDE_DIRECTORIES "${UUID_INCLUDE_DIR}"
  )
endif()
