set(STORAGE_MYSQLX_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

find_package(MySQLClient QUIET)

if(NOT MySQLClient_FOUND)
  message(STATUS "zapata-storage-mysqlx skipped: libmysqlclient not found")
  return()
endif()

add_library(zapata-storage-mysqlx SHARED)
add_dependencies(zapata-storage-mysqlx
  zapata-base
  zapata-parser-json
  zapata-parser-functional
  zapata-storage-connector
)
target_sources(zapata-storage-mysqlx
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/connector.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/translate.cpp
)
target_include_directories(zapata-storage-mysqlx
  PRIVATE
    zapata-base
    zapata-parser-json
    zapata-parser-functional
    zapata-storage-connector
    ${STORAGE_MYSQLX_INCLUDE_DIR}
  INTERFACE
    $<BUILD_INTERFACE:${STORAGE_MYSQLX_INCLUDE_DIR}>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(zapata-storage-mysqlx
  PUBLIC
    zapata-base
    zapata-parser-json
    zapata-parser-functional
    zapata-storage-connector
    MySQLClient::MySQLClient
)
set_target_properties(zapata-storage-mysqlx
  PROPERTIES
    EXPORT_NAME storage-mysqlx
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fPIC"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS} -shared"
    LIBRARIES ${CMAKE_CURRENT_BINARY_DIR}/libzapata-storage-mysqlx.so
)

add_library(zapata-storage-mysqlx-plugin SHARED)
add_dependencies(zapata-storage-mysqlx-plugin
  zapata-base
  zapata-engine-startup
  zapata-parser-json
  zapata-parser-functional
  zapata-storage-connector
  zapata-storage-mysqlx
)
target_sources(zapata-storage-mysqlx-plugin
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/plugin.cpp
)
target_include_directories(zapata-storage-mysqlx-plugin
  PRIVATE
    zapata-base
    zapata-engine-startup
    zapata-parser-json
    zapata-parser-functional
    zapata-storage-connector
    zapata-storage-mysqlx
)
target_link_libraries(zapata-storage-mysqlx-plugin
  PUBLIC
    zapata-base
    zapata-engine-startup
    zapata-parser-json
    zapata-parser-functional
    zapata-storage-connector
    zapata-storage-mysqlx
    MySQLClient::MySQLClient
)
set_target_properties(zapata-storage-mysqlx-plugin
  PROPERTIES
    EXPORT_NAME storage-mysqlx-plugin
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fPIC"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS} -shared"
    LIBRARIES ${CMAKE_CURRENT_BINARY_DIR}/libzapata-storage-mysqlx-plugin.so
)

include(GNUInstallDirs)
install(TARGETS zapata-storage-mysqlx zapata-storage-mysqlx-plugin
  EXPORT zapata
  LIBRARY
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY ${STORAGE_MYSQLX_INCLUDE_DIR}/zapata
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.h"
)

add_executable(zapata_example_storage_mysqlx)
add_dependencies(zapata_example_storage_mysqlx
  zapata-base
  zapata-parser-json
  zapata-parser-functional
  zapata-storage-connector
  zapata-storage-mysqlx
)
target_sources(zapata_example_storage_mysqlx
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/examples/mysqlx.cpp
)
target_include_directories(zapata_example_storage_mysqlx
  PRIVATE
    zapata-base
    zapata-parser-json
    zapata-parser-functional
    zapata-storage-connector
    ${STORAGE_MYSQLX_INCLUDE_DIR}
)
target_link_libraries(zapata_example_storage_mysqlx
  PRIVATE
    zapata-base
    zapata-parser-json
    zapata-parser-functional
    zapata-storage-connector
    ${MYSQLCLIENT_LIBRARIES}
    zapata-storage-mysqlx
)
set_target_properties(zapata_example_storage_mysqlx
  PROPERTIES
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS}"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS}"
)
