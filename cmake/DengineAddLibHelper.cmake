function (dengine_add_lib target desc)
    set(DENGINE_DLL_FILENAME "${target}.dll")
    set(DENGINE_DLL_FILEDESC "${desc}")
    configure_file(${PROJECT_SOURCE_DIR}/lib/dengine-lib.rc.in src/${target}.rc)
    add_library(${target} 
        ${${target}-src}
        src/${target}.rc)
    set_target_properties(${target} PROPERTIES
        VERSION ${dengine_VERSION_MAJOR}.${dengine_VERSION_MINOR}
        SOVERSION ${dengine_VERSION_MAJOR}
        POSITION_INDEPENDENT_CODE ON
        C_STANDARD 99)
    if(WIN32)
        set_target_properties(${target} PROPERTIES PREFIX "")
    endif()
    target_sources(
        ${target} PUBLIC
        FILE_SET target_hdr_set
        TYPE HEADERS
        BASE_DIRS include
        FILES ${${target}-hdr}
    )
    target_include_directories(${target} PRIVATE ${${target}-incl})
    target_link_libraries(${target} PRIVATE ${${target}-link})
    install(TARGETS ${target} DESTINATION lib
        FILE_SET target_hdr_set DESTINATION include/dengine-${DENGINE_VERSION}/lib)
endfunction()
