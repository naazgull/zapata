set(STORAGE_MYSQLX_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

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
  INTERFACE
    ${STORAGE_MYSQLX_INCLUDE_DIR}/zapata/mysqlx.h
    ${STORAGE_MYSQLX_INCLUDE_DIR}/zapata/mysqlx/translate.h
    ${STORAGE_MYSQLX_INCLUDE_DIR}/zapata/mysqlx/connector.h
)
target_include_directories(zapata-storage-mysqlx
  PRIVATE
    zapata-base
    zapata-parser-json
    zapata-parser-functional
    zapata-storage-connector
    ${STORAGE_MYSQLX_INCLUDE_DIR}
  INTERFACE
    ${STORAGE_MYSQLX_INCLUDE_DIR}
)
target_link_libraries(zapata-storage-mysqlx
  PUBLIC
    zapata-base
    zapata-parser-json
    zapata-parser-functional
    zapata-storage-connector
    mysqlcppconn8
)
set_target_properties(zapata-storage-mysqlx
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fPIC"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS} -shared"
    LIBRARIES ${CMAKE_CURRENT_BINARY_DIR}/libzapata-storage-mysqlx.so
)

include(GNUInstallDirs)
install(TARGETS zapata-storage-mysqlx
  LIBRARY
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY ${STORAGE_MYSQLX_INCLUDE_DIR}/zapata
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.h"
)

add_executable(zapata_test_storage_mysqlx)
add_dependencies(zapata_test_storage_mysqlx
  zapata-base
  zapata-parser-json
  zapata-parser-functional
  zapata-storage-connector
  zapata-storage-mysqlx
)
target_sources(zapata_test_storage_mysqlx
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test.cpp
)
target_include_directories(zapata_test_storage_mysqlx
  PRIVATE
    zapata-base
    zapata-parser-json
    zapata-parser-functional
    zapata-storage-connector
    ${STORAGE_MYSQLX_INCLUDE_DIR}
)
target_link_libraries(zapata_test_storage_mysqlx
  PRIVATE
    zapata-base
    zapata-parser-json
    zapata-parser-functional
    zapata-storage-connector
    mysqlcppconn8
    zapata-storage-mysqlx
)
set_target_properties(zapata_test_storage_mysqlx
  PROPERTIES
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS}"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS}"
)
