set(STORAGE_PGSQL_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

find_package(PQXX QUIET)

if(NOT PQXX_FOUND)
  message(STATUS "zapata-storage-postgresql skipped: libpqxx not found")
  return()
endif()

add_library(zapata-storage-postgresql SHARED)
add_dependencies(zapata-storage-postgresql
  zapata-base
  zapata-parser-json
)
target_sources(zapata-storage-postgresql
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/postgresql/Client.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/postgresql/convert_sql.cpp
  INTERFACE
    ${STORAGE_PGSQL_INCLUDE_DIR}/zapata/postgresql.h
    ${STORAGE_PGSQL_INCLUDE_DIR}/zapata/postgresql/Client.h
    ${STORAGE_PGSQL_INCLUDE_DIR}/zapata/postgresql/convert_sql.h
)
target_include_directories(zapata-storage-postgresql
  PRIVATE
    zapata-base
    zapata-parser-json
    ${STORAGE_PGSQL_INCLUDE_DIR}
  INTERFACE
    ${STORAGE_PGSQL_INCLUDE_DIR}
)
target_link_libraries(zapata-storage-postgresql
  PUBLIC
    ${PQXX_LIBRARIES}
    zapata-base
    zapata-parser-json
)
set_target_properties(zapata-storage-postgresql
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fPIC"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS} -shared"
    LIBRARIES ${CMAKE_CURRENT_BINARY_DIR}/libzapata-storage-postgresql.so
)

include(GNUInstallDirs)
install(TARGETS zapata-storage-postgresql
  LIBRARY
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY ${STORAGE_PGSQL_INCLUDE_DIR}/zapata
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.h"
)
