set(dengine-scene-src
src/ecs.c
src/scene.c
)

set(dengine-scene-hdr
include/dengine-scene/ecs.h
include/dengine-scene/scene.h
include/dengine-scene/queue.h
)
if(DENGINE_HAS_PYTHON3)
    list(APPEND dengine-scene-hdr
        include/dengine-scene/scriptingsync/python/entity.h
    )
    list(APPEND dengine-scene-src 
        src/scriptingsync/python/entity.c
    )
endif()

set(dengine-scene-link dengine-core dengine-utils dengine-script)

dengine_add_lib(dengine-scene "ECS, Scene structures and parsing")
