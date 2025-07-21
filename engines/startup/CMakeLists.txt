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
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/startup.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/configuration.cpp
  INTERFACE
    ${ENGINE_STARTUP_INCLUDE_DIR}/zapata/startup/configuration.h
    ${ENGINE_STARTUP_INCLUDE_DIR}/zapata/startup/startup.h
    ${ENGINE_STARTUP_INCLUDE_DIR}/zapata/startup.h
)
target_include_directories(zapata-engine-startup
  PRIVATE
    zapata-base
    zapata-events
    zapata-globals
    zapata-io-stream
    zapata-lockfree
    zapata-net-transport
    zapata-parser-json
    zapata-parser-uri
    ${ENGINE_STARTUP_INCLUDE_DIR}
  INTERFACE
    ${ENGINE_STARTUP_INCLUDE_DIR}
)
target_link_libraries(zapata-engine-startup
  PUBLIC
    zapata-base
    zapata-events
    zapata-globals
    zapata-io-stream
    zapata-lockfree
    zapata-net-transport
    zapata-parser-json
    zapata-parser-uri
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
