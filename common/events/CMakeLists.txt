set(COMMON_EVENTS_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(zapata-events SHARED)
add_dependencies(zapata-events
  zapata-base
  zapata-lockfree
  zapata-ontology
  zapata-parser-json
)
target_sources(zapata-events
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dispatcher.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/resolver.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/system_events.cpp
  INTERFACE
    ${COMMON_EVENTS_INCLUDE_DIR}/zapata/events/dispatcher.h
    ${COMMON_EVENTS_INCLUDE_DIR}/zapata/events/resolver.h
    ${COMMON_EVENTS_INCLUDE_DIR}/zapata/events/system_events.h
    ${COMMON_EVENTS_INCLUDE_DIR}/zapata/events.h
)
target_include_directories(zapata-events
  PRIVATE
    zapata-base
    zapata-lockfree
    zapata-ontology
    zapata-parser-json
    ${COMMON_EVENTS_INCLUDE_DIR}
  INTERFACE
    ${COMMON_EVENTS_INCLUDE_DIR}
)
target_link_libraries(zapata-events
  PRIVATE
    zapata-base
    zapata-lockfree
    zapata-ontology
    zapata-parser-json
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

add_executable(zapata_example_common_events)
add_dependencies(zapata_example_common_events
  zapata-base
  zapata-lockfree
  zapata-ontology
  zapata-parser-json
  zapata-events
)
target_sources(zapata_example_common_events
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/examples/events.cpp
)
target_include_directories(zapata_example_common_events
  PRIVATE
    zapata-base
    zapata-lockfree
    zapata-ontology
    ${COMMON_EVENTS_INCLUDE_DIR}
)
target_link_libraries(zapata_example_common_events
  PRIVATE
    zapata-base
    zapata-lockfree
    zapata-ontology
    zapata-parser-json
    zapata-events
)
set_target_properties(zapata_example_common_events
  PROPERTIES
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS}"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS}"
)
