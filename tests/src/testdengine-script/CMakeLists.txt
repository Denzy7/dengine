if(DENGINE_SCRIPTING_PYTHON)
    add_executable(testdengine-script-python script-python.c)
    target_link_libraries(testdengine-script-python ${dengine} dengine_glue_lib)
    set_property(TARGET testdengine-script-python PROPERTY C_STANDARD 99)
endif()

add_executable(testdengine-script-nsl script-nsl.c)
target_link_libraries(testdengine-script-nsl ${dengine} dengine_glue_lib)
set_property(TARGET testdengine-script-nsl PROPERTY C_STANDARD 99)

if(DENGINE_HAS_BULLET)
    add_executable(testdengine-script-nslbullet script-nslbullet.c)
    target_link_libraries(testdengine-script-nslbullet ${dengine})
    set_property(TARGET testdengine-script-nslbullet PROPERTY C_STANDARD 99)

    add_executable(testdengine-script-nslbullet-car script-nslbullet-car.c )
    target_link_libraries(testdengine-script-nslbullet-car ${dengine} dengine_glue_lib)
    set_property(TARGET testdengine-script-nslbullet-car PROPERTY C_STANDARD 99)

endif()

if(DENGINE_HAS_LIBPULSE_SIMPLE)
    add_executable(testdengine-script-nslpulseaudio script-nslpulseaudio.c)
    target_link_libraries(testdengine-script-nslpulseaudio ${dengine} dengine_glue_lib)
    set_property(TARGET testdengine-script-nslpulseaudio PROPERTY C_STANDARD 99)
endif()
