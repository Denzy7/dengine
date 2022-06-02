set(DENGINE_DLL_FILENAME "dengine-core.dll")
set(DENGINE_DLL_FILEDESC "Core")
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../dengine-lib.rc.in src/dengine-core.rc)
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
src/material.c
src/lighting.c
src/vao.c
src/renderbuffer.c
src/stb_image.c
src/stb_image_write.c
src/viewport.c
${CMAKE_CURRENT_BINARY_DIR}/src/dengine-core.rc
${PROJECT_SOURCE_DIR}/deps/glad/src/glad.c
#src/dgl.c
src/entrygl.c
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
include/dengine/material.h
include/dengine/lighting.h
include/dengine/vao.h
include/dengine/renderbuffer.h
include/dengine/dengine.h
include/dengine/viewport.h
include/dengine/entrygl.h
include/dengine/dgl.h
)

# resource list
set(coreres
    assets/shaders/debug-normals.frag.glsl
    assets/shaders/debug-normals.vert.glsl
    assets/shaders/default.frag.glsl
    assets/shaders/default.vert.glsl
    assets/shaders/gui.frag.glsl
    assets/shaders/gui.vert.glsl
    assets/shaders/shadow2d.vert.glsl
    assets/shaders/shadow2d.frag.glsl
    assets/shaders/shadow3d.vert.glsl
    assets/shaders/shadow3d.frag.glsl
    assets/shaders/shadow3d.geom.glsl
    assets/shaders/skybox.vert.glsl
    assets/shaders/skybox2d.frag.glsl
    assets/shaders/skyboxcube.frag.glsl
    assets/shaders/standard.frag.glsl
    assets/shaders/standard.vert.glsl
    LICENSE.md
)

# output after compile
set(outres)

# check for cross compile
if(CMAKE_CROSSCOMPILING)
    find_package(DengineRC)
endif()

# compile
foreach(res ${coreres})
    set(inresname ${dengine_SOURCE_DIR}/${res})
    get_filename_component(barename ${res} NAME)
    set(outresname ${CMAKE_CURRENT_BINARY_DIR}/${barename}-res.c)
    add_custom_command(
        OUTPUT ${outresname}
        COMMAND dengine-rc ${inresname} ${outresname}
        DEPENDS ${inresname}
        VERBATIM
    )
# append
    list(APPEND outres ${outresname})
endforeach()

add_library(dengine-core ${dengine-core-hdr} ${dengine-core-src} ${outres})
set_target_properties(dengine-core PROPERTIES
                      VERSION ${dengine_VERSION_MAJOR}.${dengine_VERSION_MINOR}
                      SOVERSION ${dengine_VERSION_MAJOR}
                      POSITION_INDEPENDENT_CODE ON)

#link everything here to avoid cmake link errors
target_link_libraries(dengine-core dengine-utils)

if(DENGINE_HAS_X11)
    target_link_libraries(dengine-core ${x11_LIBRARIES})
endif()

if(DENGINE_HAS_EGL)
    target_link_libraries(dengine-core ${DENGINE_HAS_EGL})
endif()

IF(DENGINE_HAS_GLX)
    target_link_libraries(dengine-core ${glx_LIBRARIES})
endif()
if(UNIX)
    target_link_libraries(dengine-core m)
endif()

# xinput = basic vibration, opengl32 = wgl
if(WIN32)
    target_link_libraries(dengine-core xinput opengl32)
endif()

set_property(TARGET dengine-core PROPERTY C_STANDARD 99)

#remove lib prefix
if(WIN32)
    set_target_properties(dengine-core PROPERTIES PREFIX "")
endif()

install(TARGETS dengine-core DESTINATION lib)
install(FILES ${dengine-core-hdr} DESTINATION include/dengine-${DENGINE_VERSION}/lib/dengine)
