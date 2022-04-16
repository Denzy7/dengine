if(GTK3_LIBRARIES)
    set(dengitor-hdr
    include/dengitor/app.h
    include/dengitor/dengitor.h
    )

    set(dengitor-src
    src/app.c
    src/dengitor.c
    )
    add_executable(dengitor ${dengitor-hdr} ${dengitor-src})
    target_include_directories(dengitor PRIVATE include)
    target_link_libraries(dengitor ${dengine} ${GTK3_LIBRARIES})
endif()
