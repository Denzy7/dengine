set(dengine-gui-src
src/gui.c
)

set(dengine-gui-hdr
include/dengine-gui/gui.h
include/dengine-gui/embfonts.h
)

set(dengine-gui-embfonts-src)

#OPEN SANS
if(DENGINE_FONT_EMBED_OPENSANS_REGULAR)
    list(APPEND dengine-gui-embfonts-src src/embfonts/opensans/opensans_regular.c)
endif()

if(DENGINE_FONT_EMBED_OPENSANS_ITALIC)
    list(APPEND dengine-gui-embfonts-src src/embfonts/opensans/opensans_italic.c)
endif()

if(DENGINE_FONT_EMBED_OPENSANS_LIGHT)
    list(APPEND dengine-gui-embfonts-src src/embfonts/opensans/opensans_light.c)
endif()

#SOURCE SANS PRO
if(DENGINE_FONT_EMBED_SOURCESANSPRO_REGULAR)
    list(APPEND dengine-gui-embfonts-src src/embfonts/sourcesanspro/sourcesanspro_regular.c)
endif()

if(DENGINE_FONT_EMBED_SOURCESANSPRO_ITALIC)
    list(APPEND dengine-gui-embfonts-src src/embfonts/sourcesanspro/sourcesanspro_italic.c)
endif()

if(DENGINE_FONT_EMBED_SOURCESANSPRO_LIGHT)
    list(APPEND dengine-gui-embfonts-src src/embfonts/sourcesanspro/sourcesanspro_light.c)
endif()

# ROBOTO
if(DENGINE_FONT_EMBED_ROBOTO_REGULAR)
    list(APPEND dengine-gui-embfonts-src src/embfonts/roboto/roboto_regular.c)
endif()

if(DENGINE_FONT_EMBED_ROBOTO_ITALIC)
    list(APPEND dengine-gui-embfonts-src src/embfonts/roboto/roboto_italic.c)
endif()

if(DENGINE_FONT_EMBED_ROBOTO_LIGHT)
    list(APPEND dengine-gui-embfonts-src src/embfonts/roboto/roboto_light.c)
endif()

add_library(dengine-gui
    ${dengine-gui-hdr} ${dengine-gui-src}
    ${dengine-gui-embfonts-hdr} ${dengine-gui-embfonts-src})
set_target_properties(dengine-gui PROPERTIES
                      VERSION ${dengine_VERSION_MAJOR}.${dengine_VERSION_MINOR}
                      SOVERSION ${dengine_VERSION_MAJOR}
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

install(TARGETS dengine-gui DESTINATION lib)
install(FILES ${dengine-gui-hdr} DESTINATION include/dengine-${DENGINE_VERSION}/lib/dengine-gui)
