#a 3d cube
add_executable(testdengine-3d-cube
        3d-cube.c
        )
target_link_libraries(testdengine-3d-cube
        dengine-core dengine-utils)
set_property(TARGET testdengine-3d-cube PROPERTY C_STANDARD 99)

#a textured 3d cube
add_executable(testdengine-3d-cube-textured
        3d-cube-textured.c
        )
target_link_libraries(testdengine-3d-cube-textured
        dengine-core dengine-utils)
set_property(TARGET testdengine-3d-cube-textured PROPERTY C_STANDARD 99)
