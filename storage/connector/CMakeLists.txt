set(STORAGE_CONNECTOR_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(zapata-storage-connector SHARED)

target_sources(zapata-storage-connector
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/connector.cpp
  INTERFACE
    ${STORAGE_CONNECTOR_INCLUDE_DIR}/zapata/connector/connector.h
    ${STORAGE_CONNECTOR_INCLUDE_DIR}/zapata/connector.h
)
target_include_directories(zapata-storage-connector
  INTERFACE
    ${STORAGE_CONNECTOR_INCLUDE_DIR}
)
target_link_libraries(zapata-storage-connector
  INTERFACE
    zapata-base
    zapata-parser-json
    zapata-parser-functional
)

set_target_properties(zapata-storage-connector
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS}"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS}"
)

include(GNUInstallDirs)
install(TARGETS zapata-storage-connector
  LIBRARY
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY ${STORAGE_CONNECTOR_INCLUDE_DIR}/zapata
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.h"
)

add_executable(zapata_test_storage_connector)

target_sources(zapata_test_storage_connector
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test.cpp
)
target_include_directories(zapata_test_storage_connector
  PUBLIC
    ${STORAGE_CONNECTOR_INCLUDE_DIR}
)
target_link_libraries(zapata_test_storage_connector
  PUBLIC
    zapata-base
    zapata-parser-json
    zapata-parser-functional
    zapata-storage-connector
)

set_target_properties(zapata_test_storage_connector
  PROPERTIES
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS}"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS}"
)
