set(COMMON_EVENTS_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(zapata-common-events SHARED)

target_sources(zapata-common-events
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dispatcher.cpp
  INTERFACE
    ${COMMON_EVENTS_INCLUDE_DIR}/zapata/events/dispatcher.h
    ${COMMON_EVENTS_INCLUDE_DIR}/zapata/events.h
)
target_include_directories(zapata-common-events
  INTERFACE
    ${COMMON_EVENTS_INCLUDE_DIR}
)
target_link_libraries(zapata-common-events
  INTERFACE
    zapata-base
    zapata-lockfree
)

set_target_properties(zapata-common-events
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS}"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS}"
)

include(GNUInstallDirs)
install(TARGETS zapata-common-events
  LIBRARY
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY ${COMMON_EVENTS_INCLUDE_DIR}/zapata
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.h"
)

add_executable(zapata_test_common_events)

target_sources(zapata_test_common_events
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test.cpp
)
target_include_directories(zapata_test_common_events
  PUBLIC
    ${COMMON_EVENTS_INCLUDE_DIR}
)
target_link_libraries(zapata_test_common_events
  PUBLIC
    zapata-base
    zapata-lockfree
    zapata-common-events
)

set_target_properties(zapata_test_common_events
  PROPERTIES
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS}"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS}"
)
