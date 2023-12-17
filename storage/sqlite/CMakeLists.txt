set(STORAGE_SQLITE_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

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

add_executable(zapata_test_storage_sqlite)
add_dependencies(zapata_test_storage_sqlite
  zapata-base
  zapata-parser-json
  zapata-parser-functional
  zapata-storage-connector
  zapata-storage-sqlite
)
target_sources(zapata_test_storage_sqlite
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test.cpp
)
target_include_directories(zapata_test_storage_sqlite
  PRIVATE
    zapata-base
    zapata-parser-json
    zapata-parser-functional
    zapata-storage-connector
    ${STORAGE_SQLITE_INCLUDE_DIR}
)
target_link_libraries(zapata_test_storage_sqlite
  PRIVATE
    zapata-base
    zapata-parser-json
    zapata-parser-functional
    zapata-storage-connector
    zapata-storage-sqlite
    ${SQLite3_LIBRARIES}
)
set_target_properties(zapata_test_storage_sqlite
  PROPERTIES
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS}"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS}"
)
