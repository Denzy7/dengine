if(GTK3_LIBRARIES)
    # check for cross compile
    if(CMAKE_CROSSCOMPILING)
        find_package(DengineRC)
    endif()

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
    )

    set(dengitor-src
    src/dengitor.c
    src/scenetree.c
    src/utils.c
    src/inspector.c
    src/prefs.c
    src/w2v.c
    ${CMAKE_CURRENT_BINARY_DIR}/dengitor.rc
    )

    add_custom_command(
        OUTPUT dengine-editor-res.c
        COMMAND glib-compile-resources ${CMAKE_CURRENT_SOURCE_DIR}/res/default/dengine-editor-res.xml --target=${CMAKE_CURRENT_BINARY_DIR}/dengine-editor-res.c --generate-source
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/res/default
        DEPENDS
            ${CMAKE_CURRENT_SOURCE_DIR}/res/default/dengine-editor-ui.glade
            ${CMAKE_CURRENT_SOURCE_DIR}/res/default/dengine-editor-prefs.glade
            ${CMAKE_CURRENT_SOURCE_DIR}/res/default/dengine-editor-res.xml
        VERBATIM
    )

    add_executable(dengitor ${dengitor-hdr} ${dengitor-src} dengine-editor-res.c ${outresname})
    target_include_directories(dengitor PRIVATE include)
    target_link_libraries(dengitor ${dengine} ${GTK3_LIBRARIES})
    #export all. useful for connecting g_signal
    set_target_properties(dengitor PROPERTIES ENABLE_EXPORTS TRUE)
    set_property(TARGET dengitor PROPERTY C_STANDARD 99)
endif()
