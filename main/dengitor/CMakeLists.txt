if(GTK3_LIBRARIES)
    set(dengitor-hdr
    include/dengitor/dengitor.h
    )

    set(dengitor-src
    src/dengitor.c
    )

    if(WIN32)
        list(APPEND dengitor-src res/default/win32/dengine-editor-res.c)
    elseif(UNIX)
        list(APPEND dengitor-src res/default/linux/dengine-editor-res.c)
    endif()

    add_executable(dengitor ${dengitor-hdr} ${dengitor-src})
    target_include_directories(dengitor PRIVATE include)
    target_link_libraries(dengitor ${dengine} ${GTK3_LIBRARIES})
endif()
