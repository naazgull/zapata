set(COMMON_EVENTS_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(zapata-events SHARED)
add_dependencies(zapata-events
  zapata-base
  zapata-lockfree
)
target_sources(zapata-events
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dispatcher.cpp
  INTERFACE
    ${COMMON_EVENTS_INCLUDE_DIR}/zapata/events/dispatcher.h
    ${COMMON_EVENTS_INCLUDE_DIR}/zapata/events.h
)
target_include_directories(zapata-events
  PRIVATE
    $<TARGET_PROPERTY:zapata-base,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-lockfree,INTERFACE_INCLUDE_DIRECTORIES>
    ${COMMON_EVENTS_INCLUDE_DIR}
  INTERFACE
    ${COMMON_EVENTS_INCLUDE_DIR}
)
target_link_libraries(zapata-events
  PRIVATE
    $<TARGET_PROPERTY:zapata-base,LIBRARIES>
)
set_target_properties(zapata-events
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fPIC"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS} -shared"
    LIBRARIES ${CMAKE_CURRENT_BINARY_DIR}/libzapata-events.so
)

include(GNUInstallDirs)
install(TARGETS zapata-events
  LIBRARY
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY ${COMMON_EVENTS_INCLUDE_DIR}/zapata
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.h"
)

add_executable(zapata_test_common_events)
add_dependencies(zapata_test_common_events
  zapata-base
  zapata-lockfree
  zapata-events
)
target_sources(zapata_test_common_events
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test.cpp
)
target_include_directories(zapata_test_common_events
  PRIVATE
    $<TARGET_PROPERTY:zapata-base,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-lockfree,INTERFACE_INCLUDE_DIRECTORIES>
    ${COMMON_EVENTS_INCLUDE_DIR}
)
target_link_libraries(zapata_test_common_events
  PRIVATE
    $<TARGET_PROPERTY:zapata-base,LIBRARIES>
    $<TARGET_PROPERTY:zapata-events,LIBRARIES>
)
set_target_properties(zapata_test_common_events
  PROPERTIES
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS}"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS}"
)
