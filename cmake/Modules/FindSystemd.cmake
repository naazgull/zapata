# FindSystemd.cmake
#
# Locates the libsystemd library and its headers, and provides the imported
# target Systemd::Systemd.
#
# Result variables:
#   Systemd_FOUND         - True if both the library and headers were found.
#   SYSTEMD_LIBRARIES     - Full path to libsystemd.
#   SYSTEMD_INCLUDE_DIRS  - Directory containing <systemd/sd-daemon.h>.

find_path(SYSTEMD_INCLUDE_DIR NAMES systemd/sd-daemon.h)
find_library(SYSTEMD_LIBRARIES NAMES systemd)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Systemd
  REQUIRED_VARS SYSTEMD_LIBRARIES SYSTEMD_INCLUDE_DIR
)

if(Systemd_FOUND AND NOT TARGET Systemd::Systemd)
  set(SYSTEMD_INCLUDE_DIRS "${SYSTEMD_INCLUDE_DIR}")
  add_library(Systemd::Systemd UNKNOWN IMPORTED)
  set_target_properties(Systemd::Systemd PROPERTIES
    IMPORTED_LOCATION "${SYSTEMD_LIBRARIES}"
    INTERFACE_INCLUDE_DIRECTORIES "${SYSTEMD_INCLUDE_DIR}"
  )
endif()
