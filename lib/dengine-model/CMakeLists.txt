set(DENGINE_DLL_FILENAME "dengine-model.dll")
set(DENGINE_DLL_FILEDESC "3D Models")
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../dengine-lib.rc.in src/dengine-model.rc)

set(dengine-model-src
src/model.c
${CMAKE_CURRENT_BINARY_DIR}/src/dengine-model.rc
)

set(dengine-model-hdr
include/dengine-model/model.h
)

add_library(dengine-model ${dengine-model-hdr} ${dengine-model-src})
set_target_properties(dengine-model PROPERTIES
                      VERSION ${dengine_VERSION_MAJOR}.${dengine_VERSION_MINOR}
                      SOVERSION ${dengine_VERSION_MAJOR}
                      POSITION_INDEPENDENT_CODE ON
                      C_STANDARD 99)

target_link_libraries(dengine-model PRIVATE dengine-core dengine-utils)

#remove lib prefix
if(WIN32)
    set_target_properties(dengine-model PROPERTIES PREFIX "")
endif()

install(TARGETS dengine-model DESTINATION lib/dengine-${DENGINE_VERSION})
install(FILES ${dengine-model-hdr} DESTINATION include/dengine-${DENGINE_VERSION}/lib/dengine-model)
