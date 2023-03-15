set(GENERATOR_AST_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(zapata-generator-ast SHARED)
add_dependencies(zapata-generator-ast
  zapata-base
)
target_sources(zapata-generator-ast
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ast.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cpp.cpp
  INTERFACE
    ${GENERATOR_AST_INCLUDE_DIR}/zapata/ast.h
    ${GENERATOR_AST_INCLUDE_DIR}/zapata/ast/ast.h
    ${GENERATOR_AST_INCLUDE_DIR}/zapata/ast/cpp.h
)
target_include_directories(zapata-generator-ast
  PRIVATE
    $<TARGET_PROPERTY:zapata-base,INTERFACE_INCLUDE_DIRECTORIES>
    ${GENERATOR_AST_INCLUDE_DIR}
  INTERFACE
    ${GENERATOR_AST_INCLUDE_DIR}
)
target_link_libraries(zapata-generator-ast
  PRIVATE
    $<TARGET_PROPERTY:zapata-base,LIBRARIES>
)
set_target_properties(zapata-generator-ast
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fPIC"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS} -shared"
    LIBRARIES ${CMAKE_CURRENT_BINARY_DIR}/libzapata-generator-ast.so
)

include(GNUInstallDirs)
install(TARGETS zapata-generator-ast
  LIBRARY
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY ${GENERATOR_AST_INCLUDE_DIR}/zapata
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.h"
)

add_executable(zapata_test_ast)
add_dependencies(zapata_test_ast
  zapata-base
  zapata-generator-ast
)
target_sources(zapata_test_ast
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test.cpp
)
target_include_directories(zapata_test_ast
  PRIVATE
    $<TARGET_PROPERTY:zapata-base,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-generator-ast,INTERFACE_INCLUDE_DIRECTORIES>
    ${GENERATOR_AST_INCLUDE_DIR}
)
target_link_libraries(zapata_test_ast
  PRIVATE
    $<TARGET_PROPERTY:zapata-base,LIBRARIES>
    $<TARGET_PROPERTY:zapata-generator-ast,LIBRARIES>
)
set_target_properties(zapata_test_ast
  PROPERTIES
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS}"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS}"
)
