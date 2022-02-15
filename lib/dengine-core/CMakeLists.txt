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
)

if(ANDROID)
    list(APPEND dengine-core-hdr include/dengine/android.h)
    list(APPEND dengine-core-src src/android.c )
    #glue
    list(APPEND dengine-core-src "${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c")
endif()

add_library(dengine-core ${dengine-core-hdr} ${dengine-core-src})

if(ANDROID)
    target_include_directories(dengine-core PUBLIC
            #FOR debugging without Android studio
            #"/home/denzy/Android/Sdk/ndk/23.1.7779620/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/include"
            #"/home/denzy/Android/Sdk/ndk/23.1.7779620/sources"

            ${ANDROID_NDK}/sources
            )
endif()

#link everything here to avoid cmake link errors
target_link_libraries(dengine-core dengine-utils glad)

if(NOT ANDROID)
    target_link_libraries(dengine-core glfw)
endif()

if(UNIX)
    target_link_libraries(dengine-core m)
endif()

#xinput basic vibration
if(WIN32)
    target_link_libraries(dengine-core xinput)
endif()

if(ANDROID)
    target_link_libraries(dengine-core android)
endif()

set_property(TARGET dengine-core PROPERTY C_STANDARD 99)
