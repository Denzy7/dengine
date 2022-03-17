set(dengine-scene-src
src/ecs.c
src/scene.c
)

set(dengine-scene-hdr
include/dengine-scene/ecs.h
include/dengine-scene/scene.h
)

add_library(dengine-scene ${dengine-scene-hdr} ${dengine-scene-src})

target_link_libraries(dengine-scene dengine-core)
set_property(TARGET dengine-scene PROPERTY C_STANDARD 99)

#remove lib prefix
if(WIN32)
    set_target_properties(dengine-scene PROPERTIES PREFIX "")
endif()
