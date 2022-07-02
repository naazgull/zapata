set(ENGINE_STARTUP_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(zapata-engine-startup SHARED)

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
  INTERFACE
    ${ENGINE_STARTUP_INCLUDE_DIR}
)
target_link_libraries(zapata-engine-startup
  INTERFACE
    zapata-base
    zapata-regex-graph
    zapata-io-stream
    zapata-parser-json
    zapata-parser-uri
    zapata-events
    zapata-net-transport
)

set_target_properties(zapata-engine-startup
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fno-omit-frame-pointer"
    LINK_FLAGS ""
)

add_executable(zpt)

target_sources(zpt
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/zpt.cpp
)
target_include_directories(zpt
  PUBLIC
    ${ENGINE_STARTUP_INCLUDE_DIR}
)
target_link_libraries(zpt
  PUBLIC
    zapata-base
    zapata-regex-graph
    zapata-io-stream
    zapata-parser-json
    zapata-parser-uri
    zapata-events
    zapata-net-transport
    zapata-engine-startup
)

set_target_properties(zpt
  PROPERTIES
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fno-omit-frame-pointer"
    LINK_FLAGS ""
)

include(GNUInstallDirs)
install(TARGETS zapata-engine-startup zpt
  LIBRARY
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME
    DESTINATION ${CMAKE_INSTALL_EXECDIR}
)
install(DIRECTORY ${ENGINE_STARTUP_INCLUDE_DIR}/zapata
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.h"
)
