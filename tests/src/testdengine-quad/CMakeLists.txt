#a 4 point-quad covering the whole screen
add_executable(testdengine-2d-quad
        2d-quad.c
        )
target_link_libraries(testdengine-2d-quad
        dengine-core dengine-utils)
set_property(TARGET testdengine-2d-quad PROPERTY C_STANDARD 99)

#a 4 point-quad covering the whole screen textured
add_executable(testdengine-2d-quad-textured
        2d-quad-textured.c
        )
target_link_libraries(testdengine-2d-quad-textured
        dengine-core dengine-utils)
set_property(TARGET testdengine-2d-quad-textured PROPERTY C_STANDARD 99)


