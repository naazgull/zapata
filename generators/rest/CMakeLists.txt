set(GENERATOR_REST_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(zapata-generator-rest SHARED)
add_dependencies(zapata-generator-rest
  zapata-base
  zapata-parser-json
  zapata-parser-uri
  zapata-generator-ast
)
target_sources(zapata-generator-rest
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rest.cpp
  INTERFACE
    ${GENERATOR_REST_INCLUDE_DIR}/zapata/rest.h
    ${GENERATOR_REST_INCLUDE_DIR}/zapata/rest/rest.h
)
target_include_directories(zapata-generator-rest
  PRIVATE
    $<TARGET_PROPERTY:zapata-base,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-parser-json,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-parser-uri,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-generator-ast,INTERFACE_INCLUDE_DIRECTORIES>
    ${GENERATOR_REST_INCLUDE_DIR}
  INTERFACE
    ${GENERATOR_REST_INCLUDE_DIR}
)
target_link_libraries(zapata-generator-rest
  PRIVATE
    $<TARGET_PROPERTY:zapata-base,LIBRARIES>
    $<TARGET_PROPERTY:zapata-parser-json,LIBRARIES>
    $<TARGET_PROPERTY:zapata-parser-uri,LIBRARIES>
    $<TARGET_PROPERTY:zapata-generator-ast,LIBRARIES>
)
set_target_properties(zapata-generator-rest
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fPIC"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS} -shared"
    LIBRARIES ${CMAKE_CURRENT_BINARY_DIR}/libzapata-generator-rest.so
)

add_executable(zrestgen)
add_dependencies(zrestgen
  zapata-base
  zapata-parser-json
  zapata-parser-uri
  zapata-generator-ast
  zapata-generator-rest
)
target_sources(zrestgen
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/zrestgen.cpp
)
target_include_directories(zrestgen
  PRIVATE
    $<TARGET_PROPERTY:zapata-base,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-parser-json,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-generator-ast,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-generator-rest,INTERFACE_INCLUDE_DIRECTORIES>
    ${ENGINE_EVENTS_INCLUDE_DIR}
)
target_link_libraries(zrestgen
  PRIVATE
    $<TARGET_PROPERTY:zapata-base,LIBRARIES>
    $<TARGET_PROPERTY:zapata-parser-json,LIBRARIES>
    $<TARGET_PROPERTY:zapata-generator-ast,LIBRARIES>
    $<TARGET_PROPERTY:zapata-generator-rest,LIBRARIES>
)
set_target_properties(zrestgen
  PROPERTIES
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fPIC"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS}"
)

include(GNUInstallDirs)
install(TARGETS zapata-generator-rest zrestgen
  LIBRARY
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME
    DESTINATION ${CMAKE_INSTALL_EXECDIR}
)
install(DIRECTORY ${GENERATOR_REST_INCLUDE_DIR}/zapata
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.h"
)
