set(dengine-script-src
src/script.c
)

set(dengine-script-hdr
include/dengine-script/script.h
)

if(BUILD_SHARED_LIBS AND DENGINE_MERGE_LIBS)
    add_library(dengine-script STATIC ${dengine-script-hdr} ${dengine-script-src})
    set_target_properties(dengine-script PROPERTIES
                          POSITION_INDEPENDENT_CODE ON)
else()
    add_library(dengine-script ${dengine-script-hdr} ${dengine-script-src})
endif()

target_link_libraries(dengine-script dengine-core dengine-utils)
set_property(TARGET dengine-script PROPERTY C_STANDARD 99)

if(python3_LIBRARIES)
target_link_libraries(dengine-script ${python3_LIBRARIES})
#add python modules (shared libraries)
add_subdirectory(src/modules "modules-build")
target_link_libraries(dengine-script inpt timer)
endif()

#remove lib prefix
if(WIN32)
    set_target_properties(dengine-script PROPERTIES PREFIX "")
endif()
