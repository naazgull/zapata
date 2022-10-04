set(STORAGE_LMDB_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(zapata-storage-lmdb SHARED)
add_dependencies(zapata-storage-lmdb
  zapata-base
  zapata-parser-json
  zapata-parser-functional
  zapata-storage-connector
)
target_sources(zapata-storage-lmdb
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/connector.cpp
    ${STORAGE_LMDB_INCLUDE_DIR}/zapata/lmdb.h
    ${STORAGE_LMDB_INCLUDE_DIR}/zapata/lmdb/connector.h
)
target_include_directories(zapata-storage-lmdb
  PRIVATE
    $<TARGET_PROPERTY:zapata-base,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-parser-json,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-parser-functional,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-storage-connector,INTERFACE_INCLUDE_DIRECTORIES>
    ${STORAGE_LMDB_INCLUDE_DIR}
  INTERFACE
    ${STORAGE_LMDB_INCLUDE_DIR}
)
target_link_libraries(zapata-storage-lmdb
  PUBLIC
    $<TARGET_PROPERTY:zapata-base,LIBRARIES>
    $<TARGET_PROPERTY:zapata-parser-json,LIBRARIES>
    $<TARGET_PROPERTY:zapata-parser-functional,LIBRARIES>
    $<TARGET_PROPERTY:zapata-storage-connector,LIBRARIES>
    ${LMDB_LIBS}
)
set_target_properties(zapata-storage-lmdb
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fPIC"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS} -shared"
    LIBRARIES ${CMAKE_CURRENT_BINARY_DIR}/libzapata-storage-lmdb.so
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
add_dependencies(zapata_test_storage_lmdb
  zapata-base
  zapata-parser-json
  zapata-parser-functional
  zapata-storage-connector
  zapata-storage-lmdb
)
target_sources(zapata_test_storage_lmdb
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test.cpp
)
target_include_directories(zapata_test_storage_lmdb
  PRIVATE
    $<TARGET_PROPERTY:zapata-base,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-parser-json,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-parser-functional,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-storage-connector,INTERFACE_INCLUDE_DIRECTORIES>
    ${STORAGE_LMDB_INCLUDE_DIR}
)
target_link_libraries(zapata_test_storage_lmdb
  PRIVATE
    $<TARGET_PROPERTY:zapata-base,LIBRARIES>
    $<TARGET_PROPERTY:zapata-parser-json,LIBRARIES>
    $<TARGET_PROPERTY:zapata-parser-functional,LIBRARIES>
    $<TARGET_PROPERTY:zapata-storage-connector,LIBRARIES>
    ${LMDB_LIBS}
    $<TARGET_PROPERTY:zapata-storage-lmdb,LIBRARIES>
)
set_target_properties(zapata_test_storage_lmdb
  PROPERTIES
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS}"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS}"
)
