set(ENGINE_EVENTS_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(zapata-engine-events SHARED)
add_dependencies(zapata-engine-events
    zapata-base
    zapata-events
    zapata-globals
    zapata-io-stream
    zapata-lockfree
    zapata-net-transport
    zapata-parser-json
    zapata-parser-uri
    zapata-engine-startup
)
target_sources(zapata-engine-events
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/engine.cpp
    ${ENGINE_EVENTS_INCLUDE_DIR}/zapata/events/engine.h
)
target_include_directories(zapata-engine-events
  PRIVATE
    $<TARGET_PROPERTY:zapata-base,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-events,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-globals,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-io-stream,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-lockfree,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-net-transport,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-parser-json,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-parser-uri,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-engine-startup,INTERFACE_INCLUDE_DIRECTORIES>
    ${ENGINE_EVENTS_INCLUDE_DIR}
  INTERFACE
    ${ENGINE_EVENTS_INCLUDE_DIR}
)
target_link_libraries(zapata-engine-events
  PUBLIC
    $<TARGET_PROPERTY:zapata-base,LIBRARIES>
    $<TARGET_PROPERTY:zapata-events,LIBRARIES>
    $<TARGET_PROPERTY:zapata-globals,LIBRARIES>
    $<TARGET_PROPERTY:zapata-io-stream,LIBRARIES>
    $<TARGET_PROPERTY:zapata-net-transport,LIBRARIES>
    $<TARGET_PROPERTY:zapata-parser-json,LIBRARIES>
    $<TARGET_PROPERTY:zapata-parser-uri,LIBRARIES>
    $<TARGET_PROPERTY:zapata-engine-startup,LIBRARIES>
)
set_target_properties(zapata-engine-events
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fPIC"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS} -shared"
    LIBRARIES ${CMAKE_CURRENT_BINARY_DIR}/libzapata-engine-events.so
)

add_executable(zpt)
add_dependencies(zpt
    zapata-base
    zapata-events
    zapata-globals
    zapata-io-stream
    zapata-lockfree
    zapata-net-transport
    zapata-parser-json
    zapata-parser-uri
    zapata-engine-startup
    zapata-engine-events
)
target_sources(zpt
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/zpt.cpp
)
target_include_directories(zpt
  PRIVATE
    $<TARGET_PROPERTY:zapata-base,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-events,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-globals,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-io-stream,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-lockfree,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-net-transport,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-parser-json,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-parser-uri,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-engine-startup,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-engine-events,INTERFACE_INCLUDE_DIRECTORIES>
    ${ENGINE_EVENTS_INCLUDE_DIR}
)
target_link_libraries(zpt
  PRIVATE
    $<TARGET_PROPERTY:zapata-base,LIBRARIES>
    $<TARGET_PROPERTY:zapata-events,LIBRARIES>
    $<TARGET_PROPERTY:zapata-globals,LIBRARIES>
    $<TARGET_PROPERTY:zapata-io-stream,LIBRARIES>
    $<TARGET_PROPERTY:zapata-net-transport,LIBRARIES>
    $<TARGET_PROPERTY:zapata-parser-json,LIBRARIES>
    $<TARGET_PROPERTY:zapata-parser-uri,LIBRARIES>
    $<TARGET_PROPERTY:zapata-engine-startup,LIBRARIES>
    $<TARGET_PROPERTY:zapata-engine-events,LIBRARIES>
)
set_target_properties(zpt
  PROPERTIES
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fPIC"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS}"
)

include(GNUInstallDirs)
install(TARGETS zapata-engine-events zpt
  LIBRARY
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME
    DESTINATION ${CMAKE_INSTALL_EXECDIR}
)
install(DIRECTORY ${ENGINE_EVENTS_INCLUDE_DIR}/zapata
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.h"
)
