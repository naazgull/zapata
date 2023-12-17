set(STORAGE_CONNECTOR_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(zapata-storage-connector SHARED)
add_dependencies(zapata-storage-connector
  zapata-base
  zapata-parser-json
  zapata-parser-functional
)
target_sources(zapata-storage-connector
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/connector.cpp
  INTERFACE
    ${STORAGE_CONNECTOR_INCLUDE_DIR}/zapata/connector/connector.h
    ${STORAGE_CONNECTOR_INCLUDE_DIR}/zapata/connector.h
)
target_include_directories(zapata-storage-connector
  PRIVATE
    zapata-base
    zapata-parser-json
    zapata-parser-functional
    ${STORAGE_CONNECTOR_INCLUDE_DIR}
  INTERFACE
    ${STORAGE_CONNECTOR_INCLUDE_DIR}
)
target_link_libraries(zapata-storage-connector
  PUBLIC
    zapata-base
    zapata-parser-json
    zapata-parser-functional
)

set_target_properties(zapata-storage-connector
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fPIC"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS} -shared"
    LIBRARIES ${CMAKE_CURRENT_BINARY_DIR}/libzapata-storage-connector.so
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
add_dependencies(zapata_test_storage_connector
  zapata-base
  zapata-parser-json
  zapata-parser-functional
  zapata-storage-connector
)
target_sources(zapata_test_storage_connector
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test.cpp
)
target_include_directories(zapata_test_storage_connector
  PRIVATE
    zapata-base
    zapata-parser-json
    zapata-parser-functional
    ${STORAGE_CONNECTOR_INCLUDE_DIR}
)
target_link_libraries(zapata_test_storage_connector
  PRIVATE
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
