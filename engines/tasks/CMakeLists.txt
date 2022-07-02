set(ENGINE_TASKS_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(zapata-engine-tasks SHARED)

target_sources(zapata-engine-tasks
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tasks.cpp
  INTERFACE
    ${ENGINE_TASKS_INCLUDE_DIR}/zapata/tasks/tasks.h
    ${ENGINE_TASKS_INCLUDE_DIR}/zapata/tasks.h
)
target_include_directories(zapata-engine-tasks
  INTERFACE
    ${ENGINE_TASKS_INCLUDE_DIR}
)
target_link_libraries(zapata-engine-tasks
  INTERFACE
    zapata-base
    zapata-parser-json
)

set_target_properties(zapata-engine-tasks
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fno-omit-frame-pointer"
    LINK_FLAGS ""
)

include(GNUInstallDirs)
install(TARGETS zapata-engine-tasks
  LIBRARY
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY ${ENGINE_TASKS_INCLUDE_DIR}/zapata
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.h"
)

add_executable(zapata_test_engine_tasks)

target_sources(zapata_test_engine_tasks
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test.cpp
)
target_include_directories(zapata_test_engine_tasks
  PUBLIC
    ${ENGINE_TASKS_INCLUDE_DIR}
)
target_link_libraries(zapata_test_engine_tasks
  PUBLIC
    zapata-base
    zapata-parser-json
    zapata-engine-tasks
)

set_target_properties(zapata_test_engine_tasks
  PROPERTIES
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fno-omit-frame-pointer"
    LINK_FLAGS ""
)
