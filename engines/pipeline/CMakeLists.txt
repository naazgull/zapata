set(ENGINE_PIPELINE_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(zapata-engine-pipeline SHARED)

target_sources(zapata-engine-pipeline
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pipeline.cpp
  INTERFACE
    ${ENGINE_PIPELINE_INCLUDE_DIR}/zapata/pipeline/pipeline.h
    ${ENGINE_PIPELINE_INCLUDE_DIR}/zapata/pipeline.h
)
target_include_directories(zapata-engine-pipeline
  INTERFACE
    ${ENGINE_PIPELINE_INCLUDE_DIR}
)
target_link_libraries(zapata-engine-pipeline
  INTERFACE
    zapata-base
    zapata-parser-json
    zapata-parser-uri
    zapata-common-events
)

set_target_properties(zapata-engine-pipeline
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS}"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS}"
)

include(GNUInstallDirs)
install(TARGETS zapata-engine-pipeline
  LIBRARY
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY ${ENGINE_PIPELINE_INCLUDE_DIR}/zapata
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.h"
)

add_executable(zapata_test_engine_pipeline)

target_sources(zapata_test_engine_pipeline
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test.cpp
)
target_include_directories(zapata_test_engine_pipeline
  PUBLIC
    ${ENGINE_PIPELINE_INCLUDE_DIR}
)
target_link_libraries(zapata_test_engine_pipeline
  PUBLIC
    zapata-base
    zapata-parser-json
    zapata-parser-uri
    zapata-common-events
    zapata-engine-pipeline
)

set_target_properties(zapata_test_engine_pipeline
  PROPERTIES
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS}"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS}"
)
