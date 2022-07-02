set(ENGINE_FSM_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(zapata-engine-fsm SHARED)

target_sources(zapata-engine-fsm
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/fsm.cpp
  INTERFACE
    ${ENGINE_FSM_INCLUDE_DIR}/zapata/fsm/fsm.h
    ${ENGINE_FSM_INCLUDE_DIR}/zapata/fsm.h
)
target_include_directories(zapata-engine-fsm
  INTERFACE
    ${ENGINE_FSM_INCLUDE_DIR}
)
target_link_libraries(zapata-engine-fsm
  INTERFACE
    zapata-base
    zapata-parser-json
    zapata-parser-uri
    zapata-events
)

set_target_properties(zapata-engine-fsm
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fno-omit-frame-pointer"
    LINK_FLAGS ""
)

include(GNUInstallDirs)
install(TARGETS zapata-engine-fsm
  LIBRARY
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY ${ENGINE_FSM_INCLUDE_DIR}/zapata
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.h"
)

add_executable(zapata_test_engine_fsm)

target_sources(zapata_test_engine_fsm
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test.cpp
)
target_include_directories(zapata_test_engine_fsm
  PUBLIC
    ${ENGINE_FSM_INCLUDE_DIR}
)
target_link_libraries(zapata_test_engine_fsm
  PUBLIC
    zapata-base
    zapata-parser-json
    zapata-parser-uri
    zapata-events
    zapata-engine-fsm
)

set_target_properties(zapata_test_engine_fsm
  PROPERTIES
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fno-omit-frame-pointer"
    LINK_FLAGS ""
)
