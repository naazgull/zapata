set(ENGINE_STARTUP_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(zapata-engine-startup SHARED)
add_dependencies(zapata-engine-startup
    zapata-base
    zapata-events
    zapata-globals
    zapata-io-stream
    zapata-lockfree
    zapata-net-transport
    zapata-parser-json
    zapata-parser-uri
)
target_sources(zapata-engine-startup
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/startup.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/configuration.cpp
    ${ENGINE_STARTUP_INCLUDE_DIR}/zapata/startup/configuration.h
    ${ENGINE_STARTUP_INCLUDE_DIR}/zapata/startup/startup.h
    ${ENGINE_STARTUP_INCLUDE_DIR}/zapata/startup.h
)
target_include_directories(zapata-engine-startup
  PRIVATE
    $<TARGET_PROPERTY:zapata-base,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-events,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-globals,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-io-stream,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-lockfree,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-net-transport,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-parser-json,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-parser-uri,INTERFACE_INCLUDE_DIRECTORIES>
    ${ENGINE_STARTUP_INCLUDE_DIR}
  INTERFACE
    ${ENGINE_STARTUP_INCLUDE_DIR}
)
target_link_libraries(zapata-engine-startup
  PUBLIC
    $<TARGET_PROPERTY:zapata-base,LIBRARIES>
    $<TARGET_PROPERTY:zapata-events,LIBRARIES>
    $<TARGET_PROPERTY:zapata-globals,LIBRARIES>
    $<TARGET_PROPERTY:zapata-io-stream,LIBRARIES>
    $<TARGET_PROPERTY:zapata-net-transport,LIBRARIES>
    $<TARGET_PROPERTY:zapata-parser-json,LIBRARIES>
    $<TARGET_PROPERTY:zapata-parser-uri,LIBRARIES>
)
set_target_properties(zapata-engine-startup
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fPIC"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS} -shared"
    LIBRARIES ${CMAKE_CURRENT_BINARY_DIR}/libzapata-engine-startup.so
)

include(GNUInstallDirs)
install(TARGETS zapata-engine-startup
  LIBRARY
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME
    DESTINATION ${CMAKE_INSTALL_EXECDIR}
)
install(DIRECTORY ${ENGINE_STARTUP_INCLUDE_DIR}/zapata
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.h"
)
