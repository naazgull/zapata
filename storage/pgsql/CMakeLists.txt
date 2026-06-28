set(STORAGE_PGSQL_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

find_package(PostgreSQL 18 QUIET)

if(NOT PostgreSQL_FOUND)
  message(STATUS "zapata-storage-pgsql skipped: libpq not found")
  return()
endif()

include_directories(
  SYSTEM
    ${PostgreSQL_INCLUDE_DIRS}
)

add_library(zapata-storage-pgsql SHARED)
add_dependencies(zapata-storage-pgsql
  zapata-base
  zapata-parser-json
  zapata-parser-functional
  zapata-storage-connector
)
target_sources(zapata-storage-pgsql
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/connector.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/translate.cpp
  INTERFACE
    ${STORAGE_PGSQL_INCLUDE_DIR}/zapata/pgsql.h
    ${STORAGE_PGSQL_INCLUDE_DIR}/zapata/pgsql/translate.h
    ${STORAGE_PGSQL_INCLUDE_DIR}/zapata/pgsql/connector.h
)
target_include_directories(zapata-storage-pgsql
  PRIVATE
    zapata-base
    zapata-parser-json
    zapata-parser-functional
    zapata-storage-connector
    ${STORAGE_PGSQL_INCLUDE_DIR}
  INTERFACE
    ${STORAGE_PGSQL_INCLUDE_DIR}
)
target_link_libraries(zapata-storage-pgsql
  PUBLIC
    zapata-base
    zapata-parser-json
    zapata-parser-functional
    zapata-storage-connector
    ${PostgreSQL_LIBRARIES}
)
set_target_properties(zapata-storage-pgsql
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fPIC"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS} -shared"
    LIBRARIES ${CMAKE_CURRENT_BINARY_DIR}/libzapata-storage-pgsql.so
)

include(GNUInstallDirs)
install(TARGETS zapata-storage-pgsql
  LIBRARY
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY ${STORAGE_PGSQL_INCLUDE_DIR}/zapata
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.h"
)

add_executable(zapata_example_storage_pgsql)
add_dependencies(zapata_example_storage_pgsql
  zapata-base
  zapata-parser-json
  zapata-parser-functional
  zapata-storage-connector
  zapata-storage-pgsql
)
target_sources(zapata_example_storage_pgsql
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/examples/pgsql.cpp
)
target_include_directories(zapata_example_storage_pgsql
  PRIVATE
    zapata-base
    zapata-parser-json
    zapata-parser-functional
    zapata-storage-connector
    ${STORAGE_PGSQL_INCLUDE_DIR}
)
target_link_libraries(zapata_example_storage_pgsql
  PRIVATE
    zapata-base
    zapata-parser-json
    zapata-parser-functional
    zapata-storage-connector
    ${PostgreSQL_LIBRARIES}
    zapata-storage-pgsql
)
set_target_properties(zapata_example_storage_pgsql
  PROPERTIES
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS}"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS}"
)
