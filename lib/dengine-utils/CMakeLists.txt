set(dengine-utils-src
src/thread.c
src/logging.c
src/confserialize.c
src/filesys.c
src/os.c
src/rng.c
src/timer.c
src/vtor.c
src/str.c
src/debug.c
src/stream.c
src/zipread.c
src/dynlib.c
src/types.c
src/assets.c
)

set(dengine-utils-hdr
include/dengine-utils/thread.h
include/dengine-utils/logging.h
include/dengine-utils/confserialize.h
include/dengine-utils/filesys.h
include/dengine-utils/vtor.h
include/dengine-utils/os.h
include/dengine-utils/rng.h
include/dengine-utils/timer.h
include/dengine-utils/str.h
include/dengine-utils/debug.h
include/dengine-utils/stream.h
include/dengine-utils/zipread.h
include/dengine-utils/macros.h
include/dengine-utils/dynlib.h
include/dengine-utils/types.h
include/dengine-utils/assets.h
)

if(ANDROID_NDK)
    list(APPEND dengine-utils-hdr include/dengine-utils/platform/android.h)
    list(APPEND dengine-utils-src src/platform/android.c )
endif()

set(dengine-utils-link)

if(GTK3_LIBRARIES AND UNIX)
    list(APPEND dengine-utils-link ${GTK3_LIBRARIES})
endif()

if(NOT WIN32)
    #TODO: find_package (Threads ?)
    #dlsym, dlopen, pthread
    find_library(libpthread pthread)
    if(libpthread)
        list(APPEND dengine-utils-link ${libpthread})
    endif()
    list(APPEND dengine-utils-link dl)
endif()

#android and logcat
if(ANDROID_NDK)
    list(APPEND dengine-utils-link android log)
endif()

# zlib deflate
if(DENGINE_HAS_LIBZ)
    list(APPEND dengine-utils-link ${DENGINE_HAS_LIBZ})
endif()

dengine_add_lib(dengine-utils "Utilities and Tools")
