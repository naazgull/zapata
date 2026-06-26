set(COMMON_ONTOLOGY_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(zapata-ontology SHARED)
add_dependencies(zapata-ontology
  zapata-base
  zapata-globals
  zapata-lockfree
  zapata-parser-json
  zapata-parser-uri
)
target_sources(zapata-ontology
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/message.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/performative.cpp
  INTERFACE
    ${COMMON_ONTOLOGY_INCLUDE_DIR}/zapata/ontology/message.h
    ${COMMON_ONTOLOGY_INCLUDE_DIR}/zapata/ontology/performative.h
    ${COMMON_ONTOLOGY_INCLUDE_DIR}/zapata/ontology.h
)
target_include_directories(zapata-ontology
  PRIVATE
    zapata-base
    zapata-globals
    zapata-lockfree
    zapata-parser-json
    zapata-parser-uri
    ${COMMON_ONTOLOGY_INCLUDE_DIR}
  INTERFACE
    ${COMMON_ONTOLOGY_INCLUDE_DIR}
)
target_link_libraries(zapata-ontology
  PRIVATE
    zapata-base
    zapata-globals
    zapata-lockfree
    zapata-parser-json
    zapata-parser-uri
)
set_target_properties(zapata-ontology
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fPIC"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS} -shared"
    LIBRARIES ${CMAKE_CURRENT_BINARY_DIR}/libzapata-ontology.so
)

include(GNUInstallDirs)
install(TARGETS zapata-ontology
  LIBRARY
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY ${COMMON_ONTOLOGY_INCLUDE_DIR}/zapata
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.h"
)

add_executable(zapata_example_common_ontology)
add_dependencies(zapata_example_common_ontology
  zapata-base
  zapata-globals
  zapata-lockfree
  zapata-parser-json
  zapata-parser-uri
  zapata-ontology
)
target_sources(zapata_example_common_ontology
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/examples/ontology.cpp
)
target_include_directories(zapata_example_common_ontology
  PRIVATE
    zapata-base
    zapata-globals
    zapata-lockfree
    zapata-parser-json
    zapata-parser-uri
    ${COMMON_ONTOLOGY_INCLUDE_DIR}
)
target_link_libraries(zapata_example_common_ontology
  PRIVATE
    zapata-base
    zapata-globals
    zapata-lockfree
    zapata-parser-json
    zapata-parser-uri
    zapata-ontology
)
set_target_properties(zapata_example_common_ontology
  PROPERTIES
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS}"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS}"
)
