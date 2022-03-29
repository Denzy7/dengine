set(dengine-scene-src
src/ecs.c
src/scene.c
)

set(dengine-scene-hdr
include/dengine-scene/ecs.h
include/dengine-scene/scene.h
)
if(BUILD_SHARED_LIBS AND DENGINE_MERGE_LIBS)
    add_library(dengine-scene STATIC ${dengine-scene-hdr} ${dengine-scene-src})
else()
    add_library(dengine-scene ${dengine-scene-hdr} ${dengine-scene-src})
endif()
set_target_properties(dengine-scene PROPERTIES
                      POSITION_INDEPENDENT_CODE ON)

target_link_libraries(dengine-scene dengine-core)
set_property(TARGET dengine-scene PROPERTY C_STANDARD 99)

#remove lib prefix
if(WIN32)
    set_target_properties(dengine-scene PROPERTIES PREFIX "")
endif()
