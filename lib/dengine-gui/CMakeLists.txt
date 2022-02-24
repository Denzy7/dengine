set(dengine-gui-src
src/gui.c
)

set(dengine-gui-hdr
include/dengine-gui/gui.h
)

add_library(dengine-gui ${dengine-gui-hdr} ${dengine-gui-src})

target_link_libraries(dengine-gui dengine-core)
set_property(TARGET dengine-gui PROPERTY C_STANDARD 99)

if(fontconfig_LIBRARIES)
target_link_libraries(dengine-gui ${fontconfig_LIBRARIES})
endif()

#remove lib prefix
if(WIN32)
    set_target_properties(dengine-gui PROPERTIES PREFIX "")
endif()
