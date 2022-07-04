set(STORAGE_LMDB_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(zapata-storage-lmdb SHARED)

target_sources(zapata-storage-lmdb
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/connector.cpp
  INTERFACE
    ${STORAGE_LMDB_INCLUDE_DIR}/zapata/lmdb.h
    ${STORAGE_LMDB_INCLUDE_DIR}/zapata/lmdb/connector.h
)
target_include_directories(zapata-storage-lmdb
  INTERFACE
    ${STORAGE_LMDB_INCLUDE_DIR}
)
target_link_libraries(zapata-storage-lmdb
  INTERFACE
    zapata-base
    zapata-parser-json
    zapata-parser-functional
    zapata-storage-connector
    ${LMDB_LIBS}
)

set_target_properties(zapata-storage-lmdb
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS}"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS}"
)

include(GNUInstallDirs)
install(TARGETS zapata-storage-lmdb
  LIBRARY
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY ${STORAGE_LMDB_INCLUDE_DIR}/zapata
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.h"
)

add_executable(zapata_test_storage_lmdb)

target_sources(zapata_test_storage_lmdb
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test.cpp
)
target_include_directories(zapata_test_storage_lmdb
  PUBLIC
    ${STORAGE_LMDB_INCLUDE_DIR}
)
target_link_libraries(zapata_test_storage_lmdb
  PUBLIC
    zapata-base
    zapata-parser-json
    zapata-parser-functional
    zapata-storage-connector
    ${LMDB_LIBS}
    zapata-storage-lmdb
)

set_target_properties(zapata_test_storage_lmdb
  PROPERTIES
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS}"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS}"
)
