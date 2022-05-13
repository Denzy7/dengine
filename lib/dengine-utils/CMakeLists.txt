set(DENGINE_DLL_FILENAME "dengine-utils.dll")
set(DENGINE_DLL_FILEDESC "Core Utilities DLL")
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../dengine-lib.rc.in src/dengine-utils.rc)

set(dengine-utils-src
src/logging.c
src/confserialize.c
src/filesys.c
src/os.c
src/rng.c
src/timer.c
src/vtor.c
src/str.c
src/debug.c
${CMAKE_CURRENT_BINARY_DIR}/src/dengine-utils.rc
)

set(dengine-utils-hdr
include/dengine-utils/logging.h
include/dengine-utils/confserialize.h
include/dengine-utils/filesys.h
include/dengine-utils/vtor.h
include/dengine-utils/os.h
include/dengine-utils/rng.h
include/dengine-utils/timer.h
include/dengine-utils/str.h
include/dengine-utils/debug.h
)

if(ANDROID)
    list(APPEND dengine-utils-hdr include/dengine-utils/platform/android.h)
    list(APPEND dengine-utils-src src/platform/android.c )
    #glue
    list(APPEND dengine-utils-src "${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c")
endif()

add_library(dengine-utils ${dengine-utils-hdr} ${dengine-utils-src})
set_target_properties(dengine-utils PROPERTIES
                      VERSION ${dengine_VERSION_MAJOR}.${dengine_VERSION_MINOR}
                      SOVERSION ${dengine_VERSION_MAJOR}
                      POSITION_INDEPENDENT_CODE ON)

if(GTK3_LIBRARIES AND UNIX)
    target_link_libraries(dengine-utils ${GTK3_LIBRARIES})
endif()

if(ANDROID)
    target_include_directories(dengine-utils PUBLIC
            #FOR debugging without Android studio
            #"/home/denzy/Android/Sdk/ndk/23.1.7779620/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/include"
            #"/home/denzy/Android/Sdk/ndk/23.1.7779620/sources"

            ${ANDROID_NDK}/sources
            )
endif()

if(ANDROID)
    target_link_libraries(dengine-utils android log)
endif()

set_property(TARGET dengine-utils PROPERTY C_STANDARD 99)

#remove lib prefix
if(WIN32)
    set_target_properties(dengine-utils PROPERTIES PREFIX "")
endif()

install(TARGETS dengine-utils DESTINATION lib)
install(FILES ${dengine-utils-hdr} DESTINATION include/dengine-${DENGINE_VERSION}/lib/dengine-utils)
