set(COMMON_CATALOG_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(zapata-common-catalog SHARED)
add_dependencies(zapata-common-catalog
    zapata-base
    zapata-parser-json
    zapata-storage-connector
    zapata-storage-sqlite
)
target_sources(zapata-common-catalog
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/catalog.cpp
  INTERFACE
    ${COMMON_CATALOG_INCLUDE_DIR}/zapata/catalog/catalog.h
    ${COMMON_CATALOG_INCLUDE_DIR}/zapata/catalog.h
)
target_include_directories(zapata-common-catalog
 PRIVATE
    zapata-base
    zapata-parser-json
    zapata-storage-connector
    zapata-storage-sqlite
    ${COMMON_CATALOG_INCLUDE_DIR}
 INTERFACE
    zapata-storage-connector
    zapata-storage-sqlite
    ${COMMON_CATALOG_INCLUDE_DIR}
)
target_link_libraries(zapata-common-catalog
  PRIVATE
    zapata-base
    zapata-parser-json
    zapata-storage-connector
    ${SQLite3_LIBRARIES}
    zapata-storage-sqlite
)
set_target_properties(zapata-common-catalog
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS}"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS}"
    LIBRARIES ${CMAKE_CURRENT_BINARY_DIR}/libzapata-common-catalog.so
)

include(GNUInstallDirs)
install(TARGETS zapata-common-catalog
  LIBRARY
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY ${COMMON_CATALOG_INCLUDE_DIR}/zapata
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.h"
)

add_executable(zapata_test_common_catalog)
add_dependencies(zapata_test_common_catalog
    zapata-base
    zapata-parser-json
    zapata-storage-sqlite
    zapata-common-catalog
)
target_sources(zapata_test_common_catalog
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test.cpp
)
target_include_directories(zapata_test_common_catalog
 PRIVATE
    zapata-base
    zapata-parser-json
    zapata-storage-sqlite
    zapata-storage-connector
    ${COMMON_CATALOG_INCLUDE_DIR}
)
target_link_libraries(zapata_test_common_catalog
  PRIVATE
    zapata-base
    zapata-parser-json
    zapata-storage-connector
    ${SQLite3_LIBRARIES}
    zapata-storage-sqlite
    zapata-common-catalog
)
set_target_properties(zapata_test_common_catalog
  PROPERTIES
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS}"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS}"
)
