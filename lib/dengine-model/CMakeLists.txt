set(dengine-model-src
src/model.c
)

set(dengine-model-hdr
include/dengine-model/model.h
)

add_library(dengine-model ${dengine-model-hdr} ${dengine-model-src})
set_target_properties(dengine-model PROPERTIES
                      POSITION_INDEPENDENT_CODE ON)

target_link_libraries(dengine-model dengine-core dengine-utils)
set_property(TARGET dengine-model PROPERTY C_STANDARD 99)

#remove lib prefix
if(WIN32)
    set_target_properties(dengine-model PROPERTIES PREFIX "")
endif()

install(TARGETS dengine-model DESTINATION lib)
install(FILES ${dengine-model-hdr} DESTINATION include/dengine-${DENGINE_VERSION}/dengine-model)
