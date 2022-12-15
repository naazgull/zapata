set(BRIDGE_PYTHON_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(zapata-bridge-python SHARED)
add_dependencies(zapata-bridge-python
  zapata-base
  zapata-parser-json
  zapata-bridge-base
)
target_sources(zapata-bridge-python
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/python.cpp
  INTERFACE
    ${BRIDGE_PYTHON_INCLUDE_DIR}/zapata/python.h
    ${BRIDGE_PYTHON_INCLUDE_DIR}/zapata/python/python.h
)
target_include_directories(zapata-bridge-python
  PRIVATE
    $<TARGET_PROPERTY:zapata-base,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-parser-json,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-bridge-base,INTERFACE_INCLUDE_DIRECTORIES>
    ${BRIDGE_PYTHON_INCLUDE_DIR}
  INTERFACE    
    ${BRIDGE_PYTHON_INCLUDE_DIR}
)
target_link_libraries(zapata-bridge-python
  PUBLIC
    $<TARGET_PROPERTY:zapata-base,LIBRARIES>
    $<TARGET_PROPERTY:zapata-parser-json,LIBRARIES>
    $<TARGET_PROPERTY:zapata-bridge-base,LIBRARIES>
    ${Python3_LIBRARIES}
)
set_target_properties(zapata-bridge-python
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fPIC"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS} -shared"
    LIBRARIES ${CMAKE_CURRENT_BINARY_DIR}/libzapata-bridge-python.so
)

add_library(zapata-bridge-python-plugin SHARED)
add_dependencies(zapata-bridge-python-plugin
  zapata-base
  zapata-parser-json
  zapata-events
  zapata-engine-startup
  zapata-bridge-base
  zapata-bridge-python
)
target_sources(zapata-bridge-python-plugin
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/plugin.cpp
)
target_include_directories(zapata-bridge-python-plugin
  PRIVATE
    $<TARGET_PROPERTY:zapata-base,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-globals,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-events,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-lockfree,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-parser-json,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-engine-startup,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-bridge-base,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-bridge-python,INTERFACE_INCLUDE_DIRECTORIES>
    ${BRIDGE_PYTHON_INCLUDE_DIR}
  INTERFACE
    ${BRIDGE_PYTHON_INCLUDE_DIR}
)
target_link_libraries(zapata-bridge-python-plugin
  PUBLIC
    $<TARGET_PROPERTY:zapata-base,LIBRARIES>
    $<TARGET_PROPERTY:zapata-parser-json,LIBRARIES>
    $<TARGET_PROPERTY:zapata-events,LIBRARIES>
    $<TARGET_PROPERTY:zapata-lockfree,LIBRARIES>
    $<TARGET_PROPERTY:zapata-engine-startup,LIBRARIES>
    $<TARGET_PROPERTY:zapata-bridge-base,LIBRARIES>
    $<TARGET_PROPERTY:zapata-bridge-python,LIBRARIES>
    ${PYTHON_LIBRARIES}
)
set_target_properties(zapata-bridge-python-plugin
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fPIC"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS} -shared"
)

include(GNUInstallDirs)
install(TARGETS zapata-bridge-python zapata-bridge-python-plugin
  LIBRARY
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY ${BRIDGE_PYTHON_INCLUDE_DIR}/zapata
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.h"
)

add_executable(zapata_test_bridge_python)
add_dependencies(zapata_test_bridge_python
  zapata-base
  zapata-parser-json
  zapata-bridge-base
  zapata-bridge-python
)
target_sources(zapata_test_bridge_python
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test.cpp
)
target_include_directories(zapata_test_bridge_python
  PRIVATE
    $<TARGET_PROPERTY:zapata-base,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-parser-json,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-bridge-base,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-bridge-python,INTERFACE_INCLUDE_DIRECTORIES>
    ${BRIDGE_PYTHON_INCLUDE_DIR}
)
target_link_libraries(zapata_test_bridge_python
  PRIVATE
    $<TARGET_PROPERTY:zapata-base,LIBRARIES>
    $<TARGET_PROPERTY:zapata-parser-json,LIBRARIES>
    $<TARGET_PROPERTY:zapata-bridge-base,LIBRARIES>
    $<TARGET_PROPERTY:zapata-bridge-python,LIBRARIES>
    ${Python3_LIBRARIES}
)
set_target_properties(zapata_test_bridge_python
  PROPERTIES
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS}"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS}"
)
