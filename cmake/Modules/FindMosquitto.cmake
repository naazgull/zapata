# FindMosquitto.cmake
#
# Locates the Eclipse Mosquitto MQTT C client library and its headers, and
# provides the imported target Mosquitto::Mosquitto.
#
# Result variables:
#   Mosquitto_FOUND          - True if both the library and headers were found.
#   MOSQUITTO_LIBRARIES      - Full path to libmosquitto.
#   MOSQUITTO_LIBRARY        - Same as MOSQUITTO_LIBRARIES (compatibility).
#   MOSQUITTO_INCLUDE_DIRS   - Directory containing <mosquitto.h>.
#   MOSQUITTO_INCLUDE_DIR    - Same as MOSQUITTO_INCLUDE_DIRS (compatibility).

find_path(MOSQUITTO_INCLUDE_DIR NAMES mosquitto.h)
find_library(MOSQUITTO_LIBRARY NAMES mosquitto)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Mosquitto
  REQUIRED_VARS MOSQUITTO_LIBRARY MOSQUITTO_INCLUDE_DIR
)

if(Mosquitto_FOUND)
  set(MOSQUITTO_LIBRARIES "${MOSQUITTO_LIBRARY}")
  set(MOSQUITTO_INCLUDE_DIRS "${MOSQUITTO_INCLUDE_DIR}")
  if(NOT TARGET Mosquitto::Mosquitto)
    add_library(Mosquitto::Mosquitto UNKNOWN IMPORTED)
    set_target_properties(Mosquitto::Mosquitto PROPERTIES
      IMPORTED_LOCATION "${MOSQUITTO_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${MOSQUITTO_INCLUDE_DIR}"
    )
  endif()
endif()

mark_as_advanced(MOSQUITTO_INCLUDE_DIR MOSQUITTO_LIBRARY)
