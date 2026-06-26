set(STORAGE_SQLITE_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

find_package(SQLite3 QUIET)

if(NOT SQLite3_FOUND)
  message(STATUS "zapata-storage-sqlite skipped: SQLite3 not found")
  return()
endif()

add_library(zapata-storage-sqlite SHARED)
add_dependencies(zapata-storage-sqlite
  zapata-base
  zapata-parser-json
  zapata-parser-functional
  zapata-storage-connector
)
target_sources(zapata-storage-sqlite
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/connector.cpp
  INTERFACE
    ${STORAGE_SQLITE_INCLUDE_DIR}/zapata/sqlite.h
    ${STORAGE_SQLITE_INCLUDE_DIR}/zapata/sqlite/connector.h
)
target_include_directories(zapata-storage-sqlite
  PRIVATE
    zapata-base
    zapata-parser-json
    zapata-parser-functional
    zapata-storage-connector
    ${STORAGE_SQLITE_INCLUDE_DIR}
  INTERFACE
    ${STORAGE_SQLITE_INCLUDE_DIR}
)
target_link_libraries(zapata-storage-sqlite
  PUBLIC
    zapata-base
    zapata-parser-json
    zapata-parser-functional
    zapata-storage-connector
    ${SQLite3_LIBRARIES}
)
set_target_properties(zapata-storage-sqlite
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fPIC"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS} -shared"
    LIBRARIES ${CMAKE_CURRENT_BINARY_DIR}/libzapata-storage-sqlite.so
)

include(GNUInstallDirs)
install(TARGETS zapata-storage-sqlite
  LIBRARY
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY ${STORAGE_SQLITE_INCLUDE_DIR}/zapata
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.h"
)

add_executable(zapata_example_storage_sqlite)
add_dependencies(zapata_example_storage_sqlite
  zapata-base
  zapata-parser-json
  zapata-parser-functional
  zapata-storage-connector
  zapata-storage-sqlite
)
target_sources(zapata_example_storage_sqlite
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/examples/sqlite.cpp
)
target_include_directories(zapata_example_storage_sqlite
  PRIVATE
    zapata-base
    zapata-parser-json
    zapata-parser-functional
    zapata-storage-connector
    ${STORAGE_SQLITE_INCLUDE_DIR}
)
target_link_libraries(zapata_example_storage_sqlite
  PRIVATE
    zapata-base
    zapata-parser-json
    zapata-parser-functional
    zapata-storage-connector
    zapata-storage-sqlite
    ${SQLite3_LIBRARIES}
)
set_target_properties(zapata_example_storage_sqlite
  PROPERTIES
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS}"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS}"
)
