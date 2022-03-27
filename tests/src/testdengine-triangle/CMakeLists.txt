#an isosceles triangle
add_executable(testdengine-2d-triangle
        2d-triangle.c
        )
target_link_libraries(testdengine-2d-triangle
        dengine-core dengine-utils)
set_property(TARGET testdengine-2d-triangle PROPERTY C_STANDARD 99)

#an isosceles triangle textured
add_executable(testdengine-2d-triangle-textured
        2d-triangle-textured.c
        )
target_link_libraries(testdengine-2d-triangle-textured
        dengine-core dengine-utils)
set_property(TARGET testdengine-2d-triangle-textured PROPERTY C_STANDARD 99)

#an isosceles triangle textured + is 3D
add_executable(testdengine-3d-triangle-textured
        3d-triangle-textured.c
        )
target_link_libraries(testdengine-3d-triangle-textured
        dengine-core dengine-utils)
set_property(TARGET testdengine-3d-triangle-textured PROPERTY C_STANDARD 99)

