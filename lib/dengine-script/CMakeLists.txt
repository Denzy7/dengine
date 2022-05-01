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
)

set(dengine-script-py_modules
    src/py_modules/inptmodule.c
    src/py_modules/loggingmodule.c
    src/py_modules/timermodule.c
    src/py_modules/filesysmodule.c
    src/py_modules/commonmodule.c
    src/py_modules/scenemodule.c
)

add_library(dengine-script ${dengine-script-hdr} ${dengine-script-src} ${dengine-script-py_modules})
set_target_properties(dengine-script PROPERTIES
                      VERSION ${dengine_VERSION_MAJOR}.${dengine_VERSION_MINOR}
                      SOVERSION ${dengine_VERSION_MAJOR}
                      POSITION_INDEPENDENT_CODE ON
                      C_STANDARD 99)

target_link_libraries(dengine-script dengine-core dengine-utils dengine-scene)

if(python3_LIBRARIES)
target_link_libraries(dengine-script ${python3_LIBRARIES})
#add python modules (shared libraries)
add_subdirectory(src/py_modules "modules-build")
endif()

#remove lib prefix
if(WIN32)
    set_target_properties(dengine-script PROPERTIES PREFIX "")
endif()

install(TARGETS dengine-script DESTINATION lib)
install(FILES ${dengine-script-hdr} DESTINATION include/dengine-${DENGINE_VERSION}/lib/dengine-script)
