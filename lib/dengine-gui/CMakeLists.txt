set(DENGINE_DLL_FILENAME "dengine-gui.dll")
set(DENGINE_DLL_FILEDESC "Native GUI")
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../dengine-lib.rc.in src/dengine-gui.rc)

set(dengine-gui-src
src/gui.c
${CMAKE_CURRENT_BINARY_DIR}/src/dengine-gui.rc
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

# output after compile
set(outres)

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
    list(APPEND outres ${outresname})
endforeach()

add_library(dengine-gui
    ${dengine-gui-hdr} ${dengine-gui-src} ${outres})
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
