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
include/dengine/viewport.h
include/dengine/entrygl.h
include/dengine/dgl.h
)

configure_file(
    src/shaders/standard.frag.glsl.in
    standard.frag.glsl
)

# resource list
set(coreres
    lib/dengine-core/src/shaders/debug-normals.frag.glsl
    lib/dengine-core/src/shaders/debug-normals.vert.glsl
    lib/dengine-core/src/shaders/default.frag.glsl
    lib/dengine-core/src/shaders/default.vert.glsl
    lib/dengine-core/src/shaders/gui_text.frag.glsl
    lib/dengine-core/src/shaders/gui_panel.frag.glsl
    lib/dengine-core/src/shaders/gui.vert.glsl
    lib/dengine-core/src/shaders/shadow2d.vert.glsl
    lib/dengine-core/src/shaders/shadow2d.frag.glsl
    lib/dengine-core/src/shaders/shadow3d.vert.glsl
    lib/dengine-core/src/shaders/shadow3d.frag.glsl
    lib/dengine-core/src/shaders/shadow3d.geom.glsl
    lib/dengine-core/src/shaders/shadow3d_es.vert.glsl
    lib/dengine-core/src/shaders/shadow3d_es.frag.glsl
    lib/dengine-core/src/shaders/shadow3d_es.geom.glsl
    lib/dengine-core/src/shaders/skybox.vert.glsl
    lib/dengine-core/src/shaders/skybox2d.frag.glsl
    lib/dengine-core/src/shaders/skyboxcube.frag.glsl
    lib/dengine-core/src/shaders/standard.vert.glsl
    lib/dengine-core/src/shaders/discard.frag.glsl
    LICENSE.md
)

set(coreres_generated
    standard.frag.glsl
)

# compile
foreach(res ${coreres})
    set(inresname ${dengine_SOURCE_DIR}/${res})
    get_filename_component(barename ${res} NAME)
    set(outresname ${CMAKE_CURRENT_BINARY_DIR}/${barename}-res.c)
    add_custom_command(
        OUTPUT ${outresname}
        COMMAND dengine-rc -enb ${inresname} ${outresname}
        DEPENDS ${inresname}
        VERBATIM
    )
# append
    list(APPEND dengine-core-src ${outresname})
endforeach()
# compile
foreach(res ${coreres_generated})
    set(inresname ${CMAKE_CURRENT_BINARY_DIR}/${res})
    get_filename_component(barename ${res} NAME)
    set(outresname ${CMAKE_CURRENT_BINARY_DIR}/${barename}-res.c)
    add_custom_command(
        OUTPUT ${outresname}
        COMMAND dengine-rc -enb ${inresname} ${outresname}
        DEPENDS ${inresname}
        VERBATIM
    )
# append
    list(APPEND dengine-core-src ${outresname})
endforeach()

if(DENGINE_HAS_WAYLAND_CLIENT)
    #generate wl resources
    set(wl-prots
        stable/xdg-shell/xdg-shell.xml
        unstable/xdg-decoration/xdg-decoration-unstable-v1.xml
    )
    foreach(prot ${wl-prots})
        set(in ${wayland_protocols_pkgdatadir}/${prot})
        get_filename_component(barename ${prot} NAME)
        set(outsrc ${CMAKE_CURRENT_BINARY_DIR}/${barename}.c)
        set(outhdr ${CMAKE_CURRENT_BINARY_DIR}/${barename}.h)
        add_custom_command(
            OUTPUT ${outsrc}
            COMMAND ${wayland_scanner} private-code ${in} ${outsrc}
            DEPENDS ${in}
            VERBATIM
        )
        add_custom_command(
            OUTPUT ${outhdr}
            COMMAND ${wayland_scanner} client-header ${in} ${outhdr}
            DEPENDS ${in}
            VERBATIM
        )
        list(APPEND dengine-core-src ${outsrc} ${outhdr})
    endforeach()
endif()

set(dengine-core-incl)
if(DENGINE_HAS_WAYLAND_CLIENT)
    #wl_protos.xml.h
    list(APPEND dengine-core-incl ${CMAKE_CURRENT_BINARY_DIR})
endif()
set(dengine-core-link dengine-utils)

if(DENGINE_HAS_X11)
    list(APPEND dengine-core-link ${x11_LIBRARIES})
endif()

if(DENGINE_HAS_EGL)
    list(APPEND dengine-core-link ${DENGINE_HAS_EGL})
endif()

IF(DENGINE_HAS_GLX)
    list(APPEND dengine-core-link ${glx_LIBRARIES})
endif()

if(DENGINE_HAS_WAYLAND_CLIENT)
    list(APPEND dengine-core-link
        ${wayland_client_LIBRARIES}
        ${wayland_egl_LIBRARIES}
        ${xkbcommon_LIBRARIES}
    )
endif()

if(UNIX)
    list(APPEND dengine-core-link m)
endif()

# xinput = basic vibration, opengl32 = wgl
if(WIN32)
    list(APPEND dengine-core-link xinput opengl32)
endif()

#ANativeWindow_
if(ANDROID_NDK)
    list(APPEND dengine-core-link android)
endif()

dengine_add_lib(dengine-core "Core")
