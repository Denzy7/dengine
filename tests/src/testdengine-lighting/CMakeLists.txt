#a scene lit with normals only
add_executable(testdengine-lighting-normals
        lighting-normals.c
        )
target_link_libraries(testdengine-lighting-normals
        dengine-core dengine-utils)
set_property(TARGET testdengine-lighting-normals PROPERTY C_STANDARD 99)

#a scene lit with normals + dirlight and shadow, gui debugs shadowmap
add_executable(testdengine-lighting-dirlight
        lighting-dirlight.c
        )
target_link_libraries(testdengine-lighting-dirlight
        dengine-core dengine-utils dengine-gui)
set_property(TARGET testdengine-lighting-dirlight PROPERTY C_STANDARD 99)

#a scene lit with normals + pointlight and shadow, gui debugs shadowmap?
add_executable(testdengine-lighting-pointlight
        lighting-pointlight.c
        )
target_link_libraries(testdengine-lighting-pointlight
        dengine-core dengine-utils dengine-gui)
set_property(TARGET testdengine-lighting-pointlight PROPERTY C_STANDARD 99)

#a scene lit with normals + spotlight and shadow, gui debugs shadowmap?
add_executable(testdengine-lighting-spotlight
        lighting-spotlight.c
        )
target_link_libraries(testdengine-lighting-spotlight
        dengine-core dengine-utils dengine-gui)
set_property(TARGET testdengine-lighting-spotlight PROPERTY C_STANDARD 99)

# test standard shader
add_executable(testdengine-lighting-standard
        lighting-standard.c)
target_link_libraries(testdengine-lighting-standard
        dengine-core dengine-utils dengine-gui dengine-model)
set_property(TARGET testdengine-lighting-standard PROPERTY C_STANDARD 99)

# normal mapping with pointlight
add_executable(testdengine-lighting-normalmap
        lighting-normalmap.c)
target_link_libraries(testdengine-lighting-normalmap
        dengine)
set_property(TARGET testdengine-lighting-normalmap PROPERTY C_STANDARD 99)


