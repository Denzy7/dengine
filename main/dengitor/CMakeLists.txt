if(GTK3_LIBRARIES)
    set(dengitor-hdr
    include/dengitor/dengitor.h
    include/dengitor/scenetree.h
    )

    set(dengitor-src
    src/dengitor.c
    src/scenetree.c
    )

    add_custom_command(
        OUTPUT dengine-editor-res.c
        COMMAND glib-compile-resources ${CMAKE_CURRENT_SOURCE_DIR}/res/default/dengine-editor-res.xml --target=${CMAKE_CURRENT_BINARY_DIR}/dengine-editor-res.c --generate-source
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/res/default
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/res/default/dengine-editor-res.xml
        VERBATIM
    )

    add_executable(dengitor ${dengitor-hdr} ${dengitor-src} dengine-editor-res.c)
    target_include_directories(dengitor PRIVATE include)
    target_link_libraries(dengitor ${dengine} ${GTK3_LIBRARIES})
endif()
