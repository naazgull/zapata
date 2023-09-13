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
    $<TARGET_PROPERTY:zapata-base,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-parser-json,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-storage-connector,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-storage-sqlite,INTERFACE_INCLUDE_DIRECTORIES>
    ${COMMON_CATALOG_INCLUDE_DIR}
 INTERFACE
    $<TARGET_PROPERTY:zapata-storage-connector,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-storage-sqlite,INTERFACE_INCLUDE_DIRECTORIES>
    ${COMMON_CATALOG_INCLUDE_DIR}
)
target_link_libraries(zapata-common-catalog
  PRIVATE
    $<TARGET_PROPERTY:zapata-base,LIBRARIES>
    $<TARGET_PROPERTY:zapata-parser-json,LIBRARIES>
    $<TARGET_PROPERTY:zapata-storage-connector,LIBRARIES>
    ${SQLite3_LIBRARIES}
    $<TARGET_PROPERTY:zapata-storage-sqlite,LIBRARIES>
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
    $<TARGET_PROPERTY:zapata-base,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-parser-json,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-storage-sqlite,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-storage-connector,INTERFACE_INCLUDE_DIRECTORIES>
    ${COMMON_CATALOG_INCLUDE_DIR}
)
target_link_libraries(zapata_test_common_catalog
  PRIVATE
    $<TARGET_PROPERTY:zapata-base,LIBRARIES>
    $<TARGET_PROPERTY:zapata-parser-json,LIBRARIES>
    $<TARGET_PROPERTY:zapata-storage-connector,LIBRARIES>
    ${SQLite3_LIBRARIES}
    $<TARGET_PROPERTY:zapata-storage-sqlite,LIBRARIES>
    $<TARGET_PROPERTY:zapata-common-catalog,LIBRARIES>
)
set_target_properties(zapata_test_common_catalog
  PROPERTIES
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS}"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS}"
)
