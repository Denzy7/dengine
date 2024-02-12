set(DENGINE_DLL_FILENAME "dengine-script.dll")
set(DENGINE_DLL_FILEDESC "Scripting with Python3")
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../dengine-lib.rc.in src/dengine-script.rc)

set(dengine-script-src
src/script.c
${CMAKE_CURRENT_BINARY_DIR}/src/dengine-script.rc
)

set(dengine-script-hdr
include/dengine-script/script.h
include/dengine-script/py_modules/commonmodule.h
include/dengine-script/py_modules/scenemodule.h
)

set(dengine-script-py_modules
    src/py_modules/inptmodule.c
    src/py_modules/loggingmodule.c
    src/py_modules/timermodule.c
    src/py_modules/filesysmodule.c
    src/py_modules/commonmodule.c
    src/py_modules/scenemodule.c
)
if(DENGINE_SCRIPTING_PYTHON)
    list(APPEND dengine-script-src ${dengine-script-py_modules})
endif()

add_library(dengine-script STATIC ${dengine-script-hdr} ${dengine-script-src})
set(dengine-script-link)
set_target_properties(dengine-script PROPERTIES
                      VERSION ${dengine_VERSION_MAJOR}.${dengine_VERSION_MINOR}
                      SOVERSION ${dengine_VERSION_MAJOR}
                      POSITION_INDEPENDENT_CODE ON
                      C_STANDARD 99)
#remove lib prefix
if(WIN32)
  set_target_properties(dengine-script PROPERTIES PREFIX "")
endif()

if(DENGINE_SCRIPTING_PYTHON)
    list(APPEND dengine-script-link ${python3_LIBRARIES})
endif()

target_link_libraries(dengine-script PRIVATE dengine-core dengine-utils ${dengine-script-link})

if(DENGINE_HAS_BULLET)
    add_library(nsl-bulletphysics-common STATIC 
        src/nsl-bulletphysics/denginebulletcommon.cpp)
    target_link_libraries(nsl-bulletphysics-common PRIVATE
         ${dengine} ${bullet_LIBRARIES})
     set_target_properties(nsl-bulletphysics-common PROPERTIES
         POSITION_INDEPENDENT_CODE ON)

    add_library(nsl-bulletphysics SHARED
        src/nsl-bulletphysics/basic.cpp
    )
    set_target_properties(nsl-bulletphysics PROPERTIES
        PREFIX ""
        SUFFIX ".nsl"
        LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/tests-build/bin"
    )
    target_link_libraries(nsl-bulletphysics ${dengine} ${bullet_LIBRARIES} nsl-bulletphysics-common)

    set(nslbullet-car-res
       src/nsl-bulletphysics/rover.obj)

    # output after compile
    set(outres)

    # compile
    foreach(res ${nslbullet-car-res})
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
    add_library(nsl-bulletphysics-car SHARED
        src/nsl-bulletphysics/car.cpp
        ${outres}
    )
    set_target_properties(nsl-bulletphysics-car PROPERTIES
        PREFIX ""
        SUFFIX ".nsl"
        LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/tests-build/bin"
    )
    target_link_libraries(nsl-bulletphysics-car ${dengine} ${bullet_LIBRARIES} nsl-bulletphysics-common)
    install(TARGETS nsl-bulletphysics nsl-bulletphysics-car  DESTINATION ${tests_install_dir})
endif()

if(DENGINE_HAS_LIBPULSE_SIMPLE)
    set(nsl-pulseaudio-ogg
        src/nsl-pulseaudio/nocturne-chopin.ogg)

    # output after compile
    set(outres)

    # compile
    foreach(res ${nsl-pulseaudio-ogg})
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

    add_library(nsl-pulseaudio src/nsl-pulseaudio/simple.c ${outres})

    set_target_properties(nsl-pulseaudio PROPERTIES
        PREFIX ""
        SUFFIX ".nsl"
        LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/tests-build/bin"
    )
    target_link_libraries(nsl-pulseaudio ${dengine} ${libpulse_simple_LIBRARIES})
    install(TARGETS nsl-pulseaudio DESTINATION ${tests_install_dir})
endif()

add_library(nsl-test SHARED
    src/nsl-test/printmouse.c
    src/nsl-test/constantrotation.c
    src/nsl-test/moveduck.c
    src/nsl-test/pingpongscale.c
    src/nsl-test/pingpongposition.c
)

set_target_properties(nsl-test PROPERTIES
    PREFIX ""
    SUFFIX ".nsl"
    LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/tests-build/bin"
)
target_link_libraries(nsl-test ${dengine})
install(TARGETS nsl-test DESTINATION ${tests_install_dir})

install(TARGETS dengine-script DESTINATION lib)
install(FILES ${dengine-script-hdr} DESTINATION include/dengine-${DENGINE_VERSION}/lib/dengine-script)
