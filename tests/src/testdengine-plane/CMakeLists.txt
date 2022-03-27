#a 3d plane
add_executable(testdengine-3d-plane
        3d-plane.c
        )
target_link_libraries(testdengine-3d-plane
        dengine-core dengine-utils)
set_property(TARGET testdengine-3d-plane PROPERTY C_STANDARD 99)

#a 3d plane textured
add_executable(testdengine-3d-plane-textured
        3d-plane-textured.c
        )
target_link_libraries(testdengine-3d-plane-textured
        dengine-core dengine-utils)
set_property(TARGET testdengine-3d-plane-textured PROPERTY C_STANDARD 99)
