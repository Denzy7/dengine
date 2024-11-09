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
set(dengine-gui-link)
set_target_properties(dengine-gui PROPERTIES
                      VERSION ${dengine_VERSION_MAJOR}.${dengine_VERSION_MINOR}
                      SOVERSION ${dengine_VERSION_MAJOR}
                      POSITION_INDEPENDENT_CODE ON
                      C_STANDARD 99)
#remove lib prefix
if(WIN32)
  set_target_properties(dengine-gui PROPERTIES PREFIX "")
endif()

if(fontconfig_LIBRARIES AND UNIX)
    list(APPEND dengine-gui-link ${fontconfig_LIBRARIES})
endif()
target_link_libraries(dengine-gui PRIVATE dengine-core dengine-utils ${dengine-gui-link})

install(TARGETS dengine-gui DESTINATION lib/dengine-${DENGINE_VERSION})
install(FILES ${dengine-gui-hdr} DESTINATION include/dengine-${DENGINE_VERSION}/lib/dengine-gui)
