set(dengine-script-src
src/script.c
)

set(dengine-script-hdr
    include/dengine-script/script.h
)

set(dengine-script-link
    dengine-core dengine-utils)
set(dengine-script_res)

set(modules_builddir "${CMAKE_CURRENT_BINARY_DIR}/python-modules/dengine")
if(DENGINE_HAS_PYTHON3)
    set(dengine-script-py_modules
        inpt
        logging
        timer
        filesys
        common
        scene
    )
    foreach(module ${dengine-script-py_modules})
        set(module_hdr ${CMAKE_CURRENT_SOURCE_DIR}/include/dengine-script/python/modules/${module}module.h)
        if(EXISTS ${module_hdr})
            list(APPEND dengine-script-hdr ${module_hdr})
        endif()
        add_library(${module}_obj OBJECT 
            src/python/modules/${module}module.c
        )

        # linker errors on mingw! borked until further norice!
        add_library(${module} SHARED
            $<TARGET_OBJECTS:${module}_obj>
        )

        if(WIN32)
            set_target_properties(${module} PROPERTIES 
                SUFFIX ".pyd"
            )
        endif()

        # we previously infiltrated site-packages(skill issue :D ) 
        # but we simply add the path to the modules build dir
        set_target_properties(${module} ${module}_obj PROPERTIES
            PREFIX ""
            LIBRARY_OUTPUT_DIRECTORY "${modules_builddir}"
            POSITION_INDEPENDENT_CODE ON
            C_STANDARD 99)
        target_link_libraries(${module} PRIVATE dengine-core dengine-utils 
            common_obj
            ${python3_LIBRARIES})

        list(APPEND dengine-script-link ${module}_obj)
    endforeach()

    list(APPEND dengine-script-src src/python/python.c)
    list(APPEND dengine-script-hdr include/dengine-script/python/python.h)
    list(APPEND dengine-script_res src/python/importhook.py)
    list(APPEND dengine-script-link ${python3_LIBRARIES})
endif()

foreach(res ${dengine-script_res})
    set(inresname ${CMAKE_CURRENT_SOURCE_DIR}/${res})
    get_filename_component(barename ${res} NAME)
    set(outresname ${CMAKE_CURRENT_BINARY_DIR}/${barename}-res.c)
    add_custom_command(
        OUTPUT ${outresname}
        COMMAND dengine-rc -enb ${inresname} ${outresname}
        DEPENDS ${inresname}
        VERBATIM
    )
    list(APPEND dengine-script-src ${outresname})
endforeach()


dengine_add_lib(dengine-script "Scripting")

install(TARGETS ${dengine-script-py_modules} DESTINATION lib/dengine-${DENGINE_VERSION}/python-modules/dengine)
install(DIRECTORY src/python/stubs DESTINATION lib/dengine-${DENGINE_VERSION}/python-stubs)

# stdlib
if(MINGW AND DENGINE_SCRIPTING_PYTHON AND NOT PYSRC)
    configure_file(${PROJECT_SOURCE_DIR}/tools/msys/pythonstdlib.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/pythonstdlib.cmake @ONLY)
    install(SCRIPT  ${CMAKE_CURRENT_BINARY_DIR}/pythonstdlib.cmake)
endif()
