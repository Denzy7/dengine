set(DENGINE_DLL_FILENAME "dengine-script.dll")
set(DENGINE_DLL_FILEDESC "Scripting with Python3")
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../dengine-lib.rc.in src/dengine-script.rc)

set(dengine-script-src
src/script.c
${CMAKE_CURRENT_BINARY_DIR}/src/dengine-script.rc
)

set(dengine-script-hdr
include/dengine-script/script.h
include/dengine-script/py_modules/commonmodule.h
include/dengine-script/py_modules/scenemodule.h
)

set(dengine-script-py_modules
    src/py_modules/inptmodule.c
    src/py_modules/loggingmodule.c
    src/py_modules/timermodule.c
    src/py_modules/filesysmodule.c
    src/py_modules/commonmodule.c
    src/py_modules/scenemodule.c
)
if(DENGINE_SCRIPTING_PYTHON)
    list(APPEND dengine-script-src ${dengine-script-py_modules})
endif()

add_library(dengine-script ${dengine-script-hdr} ${dengine-script-src})
set_target_properties(dengine-script PROPERTIES
                      VERSION ${dengine_VERSION_MAJOR}.${dengine_VERSION_MINOR}
                      SOVERSION ${dengine_VERSION_MAJOR}
                      POSITION_INDEPENDENT_CODE ON)

target_link_libraries(dengine-script dengine-core dengine-utils)
set_property(TARGET dengine-script PROPERTY C_STANDARD 99)

if(DENGINE_SCRIPTING_PYTHON)
target_link_libraries(dengine-script ${python3_LIBRARIES})
    if(NOT ANDROID)
        #add python modules (shared libraries)
        add_subdirectory(src/py_modules "modules-build")
    endif()
endif()

#remove lib prefix
if(WIN32)
    set_target_properties(dengine-script PROPERTIES PREFIX "")
endif()

if(DENGINE_HAS_BULLET)
    add_library(nsl-bulletphysics SHARED
        src/nsl-bulletphysics/basic.cpp
    )
    set_target_properties(nsl-bulletphysics PROPERTIES
        PREFIX ""
        SUFFIX ".nsl"
        LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/tests-build/bin"
    )
    target_link_libraries(nsl-bulletphysics ${dengine} ${bullet_LIBRARIES})
endif()

add_library(nsl-test SHARED
    src/nsl-test/printmouse.c
    src/nsl-test/constantrotation.c
    src/nsl-test/moveduck.c
    src/nsl-test/pingpongscale.c
    src/nsl-test/pingpongposition.c
)

set_target_properties(nsl-test PROPERTIES
    PREFIX ""
    SUFFIX ".nsl"
    LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/tests-build/bin"
)
target_link_libraries(nsl-test ${dengine})

install(TARGETS dengine-script DESTINATION lib)
install(FILES ${dengine-script-hdr} DESTINATION include/dengine-${DENGINE_VERSION}/lib/dengine-script)
