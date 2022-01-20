set(dengine-core-src
src/window.c
src/shader.c
src/buffer.c
src/checkgl.c
src/vertex.c
src/texture.c
src/camera.c
src/primitive.c
src/input.c
src/draw.c
src/framebuffer.c
)

set(dengine-core-hdr
#${PROJECT_BINARY_DIR}/dengine_config.h
include/dengine/window.h
include/dengine/loadgl.h
include/dengine/checkgl.h
include/dengine/buffer.h
include/dengine/vertex.h
include/dengine/shader.h
include/dengine/texture.h
include/dengine/camera.h
include/dengine/primitive.h
include/dengine/input.h
include/dengine/draw.h
include/dengine/framebuffer.h
)

add_library(dengine-core ${dengine-core-hdr} ${dengine-core-src})

#link everything here to avoid cmake link errors
target_link_libraries(dengine-core dengine-utils)

if(NOT ANDROID)
    target_link_libraries(dengine-core ${GLFW3_LIBRARIES} glad)
endif()

if(UNIX)
    target_link_libraries(dengine-core dl m)
endif()

set_property(TARGET dengine-core PROPERTY C_STANDARD 99)


