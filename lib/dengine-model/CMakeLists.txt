set(dengine-model-src
src/model.c
)

set(dengine-model-hdr
include/dengine-model/model.h
)

add_library(dengine-model ${dengine-model-hdr} ${dengine-model-src})

target_link_libraries(dengine-model dengine-core)
set_property(TARGET dengine-model PROPERTY C_STANDARD 99)

#remove lib prefix
if(WIN32)
    set_target_properties(dengine-model PROPERTIES PREFIX "")
endif()
