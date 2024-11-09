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
       src/nsl-bulletphysics/rover.obj
       src/nsl-bulletphysics/wheelmap.jpg
       )

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

    #this covers 90% engine code. good candidate for testing dependencies
    if(MINGW)
        set(exe nsl-bulletphysics-car)
        set(ext nsl)
        set(cwd ${PROJECT_SOURCE_DIR}/deps)
        configure_file(${PROJECT_SOURCE_DIR}/tools/mingw/mingw_copylibs_install.cmake.in
            ${CMAKE_CURRENT_BINARY_DIR}/nsl-bulletphysics-car.install.cmake @ONLY)
        install(SCRIPT  ${CMAKE_CURRENT_BINARY_DIR}/nsl-bulletphysics-car.install.cmake)
    endif()
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

add_library(nsl-test-objlib OBJECT

    src/nsl-test/printmouse.c
    src/nsl-test/constantrotation.c
    src/nsl-test/moveduck.c
    src/nsl-test/pingpongscale.c
    src/nsl-test/pingpongposition.c
    src/nsl-test/scenecamera.c
)
set_target_properties(nsl-test-objlib PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)

add_library(nsl-test SHARED
    $<TARGET_OBJECTS:nsl-test-objlib>
)

set_target_properties(nsl-test PROPERTIES
    PREFIX ""
    SUFFIX ".nsl"
    LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/tests-build/bin"
)
target_link_libraries(nsl-test ${dengine})
install(TARGETS nsl-test DESTINATION ${tests_install_dir})

