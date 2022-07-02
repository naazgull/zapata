set(STORAGE_SQLITE_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(zapata-storage-sqlite SHARED)

target_sources(zapata-storage-sqlite
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/connector.cpp
  INTERFACE
    ${STORAGE_SQLITE_INCLUDE_DIR}/zapata/sqlite.h
    ${STORAGE_SQLITE_INCLUDE_DIR}/zapata/sqlite/connector.h
)
target_include_directories(zapata-storage-sqlite
  INTERFACE
    ${STORAGE_SQLITE_INCLUDE_DIR}
)
target_link_libraries(zapata-storage-sqlite
  INTERFACE
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
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fno-omit-frame-pointer"
    LINK_FLAGS ""
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

add_executable(zapata_test_storage_sqlite)

target_sources(zapata_test_storage_sqlite
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test.cpp
)
target_include_directories(zapata_test_storage_sqlite
  PUBLIC
    ${STORAGE_SQLITE_INCLUDE_DIR}
)
target_link_libraries(zapata_test_storage_sqlite
  PUBLIC
    zapata-base
    zapata-parser-json
    zapata-parser-functional
    zapata-storage-connector
    ${SQLite3_LIBRARIES}
    zapata-storage-sqlite
)

set_target_properties(zapata_test_storage_sqlite
  PROPERTIES
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fno-omit-frame-pointer"
    LINK_FLAGS ""
)
