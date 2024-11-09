set(DENGINE_DLL_FILENAME "dengine-script.dll")
set(DENGINE_DLL_FILEDESC "Scripting with Python3")
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../dengine-lib.rc.in src/dengine-script.rc)

set(dengine-script-src
src/script.c
${CMAKE_CURRENT_BINARY_DIR}/src/dengine-script.rc
)

set(dengine-script-hdr
include/dengine-script/script.h
)

set(dengine-script-link)
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

add_library(dengine-script ${dengine-script-hdr} ${dengine-script-src})
set_target_properties(dengine-script PROPERTIES
                      VERSION ${dengine_VERSION_MAJOR}.${dengine_VERSION_MINOR}
                      SOVERSION ${dengine_VERSION_MAJOR}
                      POSITION_INDEPENDENT_CODE ON
                      C_STANDARD 99)
#remove lib prefix
if(WIN32)
  set_target_properties(dengine-script PROPERTIES PREFIX "")
endif()

target_link_libraries(dengine-script PRIVATE dengine-core dengine-utils ${dengine-script-link})

install(TARGETS ${dengine-script-py_modules} DESTINATION lib/dengine-${DENGINE_VERSION}/python-modules/dengine)
install(DIRECTORY src/python/stubs DESTINATION lib/dengine-${DENGINE_VERSION}/python-stubs)
install(TARGETS dengine-script DESTINATION lib/dengine-${DENGINE_VERSION})
install(FILES ${dengine-script-hdr} DESTINATION include/dengine-${DENGINE_VERSION}/lib/dengine-script)
#we need this mostly for zlib
if(MINGW)
    set(exe dengine-utils)
    set(ext dll)
    set(cwd ${PROJECT_SOURCE_DIR}/deps)
    configure_file(${PROJECT_SOURCE_DIR}/tools/mingw/mingw_copylibs_install.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/dengine-utils.install.cmake @ONLY)
    install(SCRIPT  ${CMAKE_CURRENT_BINARY_DIR}/dengine-utils.install.cmake)
endif()
