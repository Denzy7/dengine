set(DENGINE_DLL_FILENAME "dengine-scene.dll")
set(DENGINE_DLL_FILEDESC "Scene structures and parsing")
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../dengine-lib.rc.in src/dengine-scene.rc)

set(dengine-scene-src
src/ecs.c
src/scene.c
${CMAKE_CURRENT_BINARY_DIR}/src/dengine-scene.rc
)

set(dengine-scene-hdr
include/dengine-scene/ecs.h
include/dengine-scene/scene.h
)
if(DENGINE_HAS_PYTHON3)
    list(APPEND dengine-scene-hdr
        include/dengine-scene/scriptingsync/python/entity.h
    )
    list(APPEND dengine-scene-src 
        src/scriptingsync/python/entity.c
    )
endif()

add_library(dengine-scene ${dengine-scene-hdr} ${dengine-scene-src})
set_target_properties(dengine-scene PROPERTIES
                      VERSION ${dengine_VERSION_MAJOR}.${dengine_VERSION_MINOR}
                      SOVERSION ${dengine_VERSION_MAJOR}
                      POSITION_INDEPENDENT_CODE ON
                      C_STANDARD 99)
#remove lib prefix
if(WIN32)
  set_target_properties(dengine-scene PROPERTIES PREFIX "")
endif()

target_link_libraries(dengine-scene PRIVATE dengine-core dengine-utils dengine-script)

install(TARGETS dengine-scene DESTINATION lib/dengine-${DENGINE_VERSION})
install(FILES ${dengine-scene-hdr} DESTINATION include/dengine-${DENGINE_VERSION}/lib/dengine-scene)
