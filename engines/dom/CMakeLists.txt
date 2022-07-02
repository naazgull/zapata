set(ENGINE_DOM_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(zapata-engine-dom SHARED)

target_sources(zapata-engine-dom
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dom.cpp
  INTERFACE
    ${ENGINE_DOM_INCLUDE_DIR}/zapata/dom/dom.h
    ${ENGINE_DOM_INCLUDE_DIR}/zapata/dom.h
)
target_include_directories(zapata-engine-dom
  INTERFACE
    ${ENGINE_DOM_INCLUDE_DIR}
)
target_link_libraries(zapata-engine-dom
  INTERFACE
    zapata-base
    zapata-parser-json
    zapata-parser-uri
    zapata-events
    zapata-engine-pipeline
)

set_target_properties(zapata-engine-dom
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fno-omit-frame-pointer"
    LINK_FLAGS ""
)

include(GNUInstallDirs)
install(TARGETS zapata-engine-dom
  LIBRARY
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY ${ENGINE_DOM_INCLUDE_DIR}/zapata
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.h"
)

add_executable(zapata_test_engine_dom)

target_sources(zapata_test_engine_dom
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test.cpp
)
target_include_directories(zapata_test_engine_dom
  PUBLIC
    ${ENGINE_DOM_INCLUDE_DIR}
)
target_link_libraries(zapata_test_engine_dom
  PUBLIC
    zapata-base
    zapata-parser-json
    zapata-parser-uri
    zapata-events
    zapata-engine-pipeline
    zapata-engine-dom
)

set_target_properties(zapata_test_engine_dom
  PROPERTIES
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fno-omit-frame-pointer"
    LINK_FLAGS ""
)
