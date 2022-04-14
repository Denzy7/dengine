set(dengine-gui-src
src/gui.c
)

set(dengine-gui-hdr
include/dengine-gui/gui.h
)

set(dengine-gui-embfonts-src)
set(dengine-gui-embfonts-hdr)

#OPEN SANS
if(DENGINE_FONT_EMBED_OPENSANS_REGULAR)
    list(APPEND dengine-gui-embfonts-src src/embfonts/opensans/opensans_regular.c)
    list(APPEND dengine-gui-embfonts-hdr include/dengine-gui/embfonts/opensans/opensans_regular.h)
endif()

if(DENGINE_FONT_EMBED_OPENSANS_ITALIC)
    list(APPEND dengine-gui-embfonts-src src/embfonts/opensans/opensans_italic.c)
    list(APPEND dengine-gui-embfonts-hdr include/dengine-gui/embfonts/opensans/opensans_italic.h)
endif()

if(DENGINE_FONT_EMBED_OPENSANS_LIGHT)
    list(APPEND dengine-gui-embfonts-src src/embfonts/opensans/opensans_light.c)
    list(APPEND dengine-gui-embfonts-hdr include/dengine-gui/embfonts/opensans/opensans_light.h)
endif()

#SOURCE SANS PRO
if(DENGINE_FONT_EMBED_SOURCESANSPRO_REGULAR)
    list(APPEND dengine-gui-embfonts-src src/embfonts/sourcesanspro/sourcesanspro_regular.c)
    list(APPEND dengine-gui-embfonts-hdr include/dengine-gui/embfonts/sourcesanspro/sourcesanspro_regular.h)
endif()

if(DENGINE_FONT_EMBED_SOURCESANSPRO_ITALIC)
    list(APPEND dengine-gui-embfonts-src src/embfonts/sourcesanspro/sourcesanspro_italic.c)
    list(APPEND dengine-gui-embfonts-hdr include/dengine-gui/embfonts/sourcesanspro/sourcesanspro_italic.h)
endif()

if(DENGINE_FONT_EMBED_SOURCESANSPRO_LIGHT)
    list(APPEND dengine-gui-embfonts-src src/embfonts/sourcesanspro/sourcesanspro_light.c)
    list(APPEND dengine-gui-embfonts-hdr include/dengine-gui/embfonts/sourcesanspro/sourcesanspro_light.h)
endif()

# ROBOTO
if(DENGINE_FONT_EMBED_ROBOTO_REGULAR)
    list(APPEND dengine-gui-embfonts-src src/embfonts/roboto/roboto_regular.c)
    list(APPEND dengine-gui-embfonts-hdr include/dengine-gui/embfonts/roboto/roboto_regular.h)
endif()

if(DENGINE_FONT_EMBED_ROBOTO_ITALIC)
    list(APPEND dengine-gui-embfonts-src src/embfonts/roboto/roboto_italic.c)
    list(APPEND dengine-gui-embfonts-hdr include/dengine-gui/embfonts/roboto/roboto_italic.h)
endif()

if(DENGINE_FONT_EMBED_ROBOTO_LIGHT)
    list(APPEND dengine-gui-embfonts-src src/embfonts/roboto/roboto_light.c)
    list(APPEND dengine-gui-embfonts-hdr include/dengine-gui/embfonts/roboto/roboto_light.h)
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
install(FILES ${dengine-gui-embfonts-hdr} DESTINATION include/dengine-${DENGINE_VERSION}/lib/dengine-gui/embfonts)
