if(DENGINE_SCRIPTING_PYTHON)
    add_executable(testdengine-script-python script-python.c)
    target_link_libraries(testdengine-script-python ${dengine})
    set_property(TARGET testdengine-script-python PROPERTY C_STANDARD 99)
endif()

add_executable(testdengine-script-nsl script-nsl.c)
target_link_libraries(testdengine-script-nsl ${dengine})
set_property(TARGET testdengine-script-nsl PROPERTY C_STANDARD 99)

if(DENGINE_HAS_BULLET)
    add_executable(testdengine-script-nslbullet script-nslbullet.c)
    target_link_libraries(testdengine-script-nslbullet ${dengine})
    set_property(TARGET testdengine-script-nslbullet PROPERTY C_STANDARD 99)

    set(testdengine-script-nslbullet-car-res
        rover.obj)

    # output after compile
    set(outres)

    # compile
    foreach(res ${testdengine-script-nslbullet-car-res})
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

    add_executable(testdengine-script-nslbullet-car script-nslbullet-car.c ${outres})
    target_link_libraries(testdengine-script-nslbullet-car ${dengine} dengine-model)
    set_property(TARGET testdengine-script-nslbullet-car PROPERTY C_STANDARD 99)

endif()

if(DENGINE_HAS_LIBPULSE_SIMPLE)
    add_executable(testdengine-script-nslpulseaudio script-nslpulseaudio.c)
    target_link_libraries(testdengine-script-nslpulseaudio ${dengine})
    set_property(TARGET testdengine-script-nslpulseaudio PROPERTY C_STANDARD 99)
endif()
