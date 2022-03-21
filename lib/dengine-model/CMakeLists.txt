set(dengine-model-src
src/model.c
)

set(dengine-model-hdr
include/dengine-model/model.h
)

if(BUILD_SHARED_LIBS AND DENGINE_MERGE_LIBS)
    add_library(dengine-model STATIC ${dengine-model-hdr} ${dengine-model-src})
    set_target_properties(dengine-model PROPERTIES
                          POSITION_INDEPENDENT_CODE ON)
else()
    add_library(dengine-model ${dengine-model-hdr} ${dengine-model-src})
endif()


target_link_libraries(dengine-model dengine-core dengine-utils)
set_property(TARGET dengine-model PROPERTY C_STANDARD 99)

#remove lib prefix
if(WIN32)
    set_target_properties(dengine-model PROPERTIES PREFIX "")
endif()
