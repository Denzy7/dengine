set(dengine-script-src
src/script.c
)

set(dengine-script-hdr
include/dengine-script/script.h
)

set(dengine-script-py_modules
    src/py_modules/inptmodule.c
    src/py_modules/loggingmodule.c
    src/py_modules/timermodule.c
    src/py_modules/filesysmodule.c
    src/py_modules/scenemodule.c
)

if(BUILD_SHARED_LIBS AND DENGINE_MERGE_LIBS)
    add_library(dengine-script STATIC ${dengine-script-hdr} ${dengine-script-src} ${dengine-script-py_modules})
else()
    add_library(dengine-script ${dengine-script-hdr} ${dengine-script-src} ${dengine-script-py_modules})
endif()
set_target_properties(dengine-script PROPERTIES
                      POSITION_INDEPENDENT_CODE ON)

target_link_libraries(dengine-script dengine-core dengine-utils dengine-scene)
set_property(TARGET dengine-script PROPERTY C_STANDARD 99)

if(python3_LIBRARIES)
target_link_libraries(dengine-script ${python3_LIBRARIES})
#add python modules (shared libraries)
add_subdirectory(src/py_modules "modules-build")
endif()

#remove lib prefix
if(WIN32)
    set_target_properties(dengine-script PROPERTIES PREFIX "")
endif()