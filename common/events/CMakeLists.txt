set(COMMON_EVENTS_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(libzapata-events INTERFACE)

target_sources(libzapata-events
  INTERFACE
    ${COMMON_EVENTS_INCLUDE_DIR}/zapata/events/dispatcher.h
    ${COMMON_EVENTS_INCLUDE_DIR}/zapata/events.h
)
target_include_directories(libzapata-events
  INTERFACE
    ${COMMON_EVENTS_INCLUDE_DIR}
)
target_link_libraries(libzapata-events
  INTERFACE
    $<TARGET_OBJECTS:libzapata-base>
)

include(GNUInstallDirs)
install(TARGETS libzapata-events
  LIBRARY
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY ${COMMON_EVENTS_INCLUDE_DIR}/zapata
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.h"
)
