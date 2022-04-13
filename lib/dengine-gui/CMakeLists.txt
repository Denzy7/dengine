set(dengine-gui-src
src/gui.c
)

set(dengine-gui-hdr
include/dengine-gui/gui.h
)

if(DENGINE_FONT_EMBED_OPENSANS_REGULAR)
    list(APPEND dengine-gui-src src/embfonts/opensans/opensans_regular.c)
    list(APPEND dengine-gui-hdr include/dengine-gui/embfonts/opensans/opensans_regular.h)
endif()

if(DENGINE_FONT_EMBED_OPENSANS_ITALIC)
    list(APPEND dengine-gui-src src/embfonts/opensans/opensans_italic.c)
    list(APPEND dengine-gui-hdr include/dengine-gui/embfonts/opensans/opensans_italic.h)
endif()

if(DENGINE_FONT_EMBED_OPENSANS_LIGHT)
    list(APPEND dengine-gui-src src/embfonts/opensans/opensans_light.c)
    list(APPEND dengine-gui-hdr include/dengine-gui/embfonts/opensans/opensans_light.h)
endif()

if(DENGINE_FONT_EMBED_SOURCESANSPRO_REGULAR)
    list(APPEND dengine-gui-src src/embfonts/sourcesanspro/sourcesanspro_regular.c)
    list(APPEND dengine-gui-hdr include/dengine-gui/embfonts/sourcesanspro/sourcesanspro_regular.h)
endif()

if(DENGINE_FONT_EMBED_SOURCESANSPRO_ITALIC)
    list(APPEND dengine-gui-src src/embfonts/sourcesanspro/sourcesanspro_italic.c)
    list(APPEND dengine-gui-hdr include/dengine-gui/embfonts/sourcesanspro/sourcesanspro_italic.h)
endif()

if(DENGINE_FONT_EMBED_SOURCESANSPRO_LIGHT)
    list(APPEND dengine-gui-src src/embfonts/sourcesanspro/sourcesanspro_light.c)
    list(APPEND dengine-gui-hdr include/dengine-gui/embfonts/sourcesanspro/sourcesanspro_light.h)
endif()

add_library(dengine-gui ${dengine-gui-hdr} ${dengine-gui-src})
set_target_properties(dengine-gui PROPERTIES
                      POSITION_INDEPENDENT_CODE ON)

target_link_libraries(dengine-gui dengine-core)
set_property(TARGET dengine-gui PROPERTY C_STANDARD 99)

if(fontconfig_LIBRARIES AND UNIX)
target_link_libraries(dengine-gui ${fontconfig_LIBRARIES})
endif()

#remove lib prefix
if(WIN32)
    set_target_properties(dengine-gui PROPERTIES PREFIX "")
endif()
