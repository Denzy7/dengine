set(res res/default/breeze.zip)
set(inresname ${CMAKE_CURRENT_SOURCE_DIR}/${res})
get_filename_component(barename ${res} NAME)
set(outresname ${CMAKE_CURRENT_BINARY_DIR}/${barename}-res.c)
add_custom_command(
    OUTPUT ${outresname}
    COMMAND dengine-rc ${inresname} ${outresname}
    DEPENDS ${inresname}
    VERBATIM
)

configure_file(dengitor.rc.in dengitor.rc)
set(dengitor-hdr
    include/dengitor/dengitor.h
    include/dengitor/scenetree.h
    include/dengitor/utils.h
    include/dengitor/inspector.h
    include/dengitor/prefs.h
    include/dengitor/w2v.h
    include/dengitor/viewportopts.h
    include/dengitor/glarea.h
    include/dengitor/log.h
)

set(dengitor-src
    src/dengitor.c
    src/scenetree.c
    src/utils.c
    src/inspector.c
    src/prefs.c
    src/w2v.c
    src/viewportopts.c
    src/glarea.c
    src/log.c

    src/components/transform.c
    src/components/camera.c
    src/components/light.c
    src/components/mesh.c
    ${CMAKE_CURRENT_BINARY_DIR}/dengitor.rc
)

add_custom_command(
    OUTPUT dengine-editor-res.c
    COMMAND glib-compile-resources ${CMAKE_CURRENT_SOURCE_DIR}/res/default/dengine-editor-res.xml --target=${CMAKE_CURRENT_BINARY_DIR}/dengine-editor-res.c --generate-source
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/res/default
    DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/res/default/dengine-editor-ui.glade
    ${CMAKE_CURRENT_SOURCE_DIR}/res/default/dengine-editor-res.xml
    VERBATIM
)

add_executable(dengitor ${dengitor-hdr} ${dengitor-src} dengine-editor-res.c ${outresname})
target_include_directories(dengitor PRIVATE include)
target_link_libraries(dengitor ${dengine} ${GTK3_LIBRARIES})
set_target_properties(dengitor PROPERTIES 
    #export all. useful for connecting g_signal
    ENABLE_EXPORTS TRUE
    WINDOWS_EXPORT_ALL_SYMBOLS TRUE
    C_STANDARD 99
)

# we use the really good test scripts :)
add_library(nsl-dengitor SHARED
    $<TARGET_OBJECTS:nsl-test-objlib>
)

set_target_properties(nsl-dengitor PROPERTIES
    PREFIX ""
    SUFFIX ".nsl"
)
target_link_libraries(nsl-dengitor ${dengine})
install(TARGETS dengitor DESTINATION lib/dengitor-${DENGINE_VERSION})
install(TARGETS nsl-dengitor DESTINATION lib/dengitor-${DENGINE_VERSION})
if(MINGW)
    set(exe dengitor)
    set(ext exe)
    set(cwd ${PROJECT_SOURCE_DIR}/deps)
    configure_file(${PROJECT_SOURCE_DIR}/tools/mingw/mingw_copylibs_install.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/dengitor_install.cmake @ONLY)
    install(SCRIPT  ${CMAKE_CURRENT_BINARY_DIR}/dengitor_install.cmake)


    foreach(pfx ${CMAKE_SYSTEM_PREFIX_PATH})
        # assumes we have winpthread, may not be case but it's always there
        if(EXISTS "${pfx}/bin/libwinpthread-1.dll")
            install(DIRECTORY ${pfx}/share/glib-2.0/schemas
                DESTINATION share/glib-2.0
            )
            install(DIRECTORY
                    ${pfx}/share/icons/hicolor
                    ${pfx}/share/icons/Adwaita
                DESTINATION share/icons
            )
            install(DIRECTORY
                ${pfx}/share/themes/Default
                ${pfx}/share/themes/Emacs
                DESTINATION share/themes
            )

        endif()
    endforeach()
endif()

if(WIN32)
    configure_file(dengitor.bat.in
        ${CMAKE_CURRENT_BINARY_DIR}/dengitor.bat 
        @ONLY)
    install(
        FILES ${CMAKE_CURRENT_BINARY_DIR}/dengitor.bat
        DESTINATION bin
    )
else()
    configure_file(dengitor.sh.in
        ${CMAKE_CURRENT_BINARY_DIR}/sh/dengitor
        @ONLY)
    install(
        FILES ${CMAKE_CURRENT_BINARY_DIR}/sh/dengitor
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
        DESTINATION bin 
    )
endif()


