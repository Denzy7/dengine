set(dengine-gui-src
src/gui.c
)

set(dengine-gui-hdr
include/dengine-gui/gui.h
include/dengine-gui/embfonts.h
)

set(dengine-gui-embfonts-src)

# OPEN SANS
if(DENGINE_FONT_EMBED_OPENSANS_REGULAR)
    list(APPEND dengine-gui-embfonts-src src/embfonts/opensans/OpenSans-Regular.ttf)
endif()
if(DENGINE_FONT_EMBED_OPENSANS_ITALIC)
    list(APPEND dengine-gui-embfonts-src src/embfonts/opensans/OpenSans-Italic.ttf)
endif()
list(APPEND dengine-gui-embfonts-src src/embfonts/opensans/OpenSans-Light.ttf)

#SOURCE SANS PRO
if(DENGINE_FONT_EMBED_SOURCESANSPRO_REGULAR)
    list(APPEND dengine-gui-embfonts-src src/embfonts/sourcesanspro/SourceSansPro-Regular.ttf)
endif()

if(DENGINE_FONT_EMBED_SOURCESANSPRO_ITALIC)
    list(APPEND dengine-gui-embfonts-src src/embfonts/sourcesanspro/SourceSansPro-Italic.ttf)
endif()

if(DENGINE_FONT_EMBED_SOURCESANSPRO_LIGHT)
    list(APPEND dengine-gui-embfonts-src src/embfonts/sourcesanspro/SourceSansPro-Light.ttf)
endif()

# ROBOTO
if(DENGINE_FONT_EMBED_ROBOTO_REGULAR)
    list(APPEND dengine-gui-embfonts-src src/embfonts/roboto/Roboto-Regular.ttf)
endif()

if(DENGINE_FONT_EMBED_ROBOTO_ITALIC)
    list(APPEND dengine-gui-embfonts-src src/embfonts/roboto/Roboto-Italic.ttf)
endif()

if(DENGINE_FONT_EMBED_ROBOTO_LIGHT)
    list(APPEND dengine-gui-embfonts-src src/embfonts/roboto/Roboto-Light.ttf)
endif()

# compile
foreach(res ${dengine-gui-embfonts-src})
# inres and outres should be absolute paths for fair measure
    set(inresname ${CMAKE_CURRENT_SOURCE_DIR}/${res})
    get_filename_component(barename ${res} NAME)
    set(outresname ${CMAKE_CURRENT_BINARY_DIR}/${barename}-res.c)
    add_custom_command(
        OUTPUT ${outresname}
        COMMAND dengine-rc ${inresname} ${outresname}
        DEPENDS ${inresname}
        VERBATIM
    )

# append
    list(APPEND dengine-gui-src ${outresname})
endforeach()

set(dengine-gui-link dengine-core dengine-utils)

if(fontconfig_LIBRARIES AND UNIX)
    list(APPEND dengine-gui-link ${fontconfig_LIBRARIES})
endif()

dengine_add_lib(dengine-gui "Native GUI")
