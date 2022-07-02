set(ENGINE_REST_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(zapata-engine-rest SHARED)

target_sources(zapata-engine-rest
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rest.cpp
  INTERFACE
    ${ENGINE_REST_INCLUDE_DIR}/zapata/rest.h
    ${ENGINE_REST_INCLUDE_DIR}/zapata/rest/rest.h
)
target_include_directories(zapata-engine-rest
  INTERFACE
    ${ENGINE_REST_INCLUDE_DIR}
)
target_link_libraries(zapata-engine-rest
  INTERFACE
    zapata-base
    zapata-parser-json
    zapata-parser-uri
    zapata-events
    zapata-engine-pipeline
)

set_target_properties(zapata-engine-rest
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fno-omit-frame-pointer"
    LINK_FLAGS ""
)

add_library(zapata-engine-rest-plugin SHARED)

target_sources(zapata-engine-rest-plugin
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/plugin.cpp
)
target_include_directories(zapata-engine-rest-plugin
  INTERFACE
    ${ENGINE_REST_INCLUDE_DIR}
)
target_link_libraries(zapata-engine-rest-plugin
  INTERFACE
    zapata-base
    zapata-parser-json
    zapata-parser-uri
    zapata-events
    zapata-engine-pipeline
    zapata-engine-rest
)

set_target_properties(zapata-engine-rest-plugin
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fno-omit-frame-pointer"
    LINK_FLAGS ""
)

include(GNUInstallDirs)
install(TARGETS zapata-engine-rest zapata-engine-rest-plugin
  LIBRARY
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY ${ENGINE_REST_INCLUDE_DIR}/zapata
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.h"
)
