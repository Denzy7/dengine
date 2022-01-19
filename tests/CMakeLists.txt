#utils
add_executable(testdengine-util-confserialize
        src/testdengine-utils/util-confserialize.c
    )

add_executable(testdengine-util-filesys
        src/testdengine-utils/util-filesys.c
    )

add_executable(testdengine-util-os
        src/testdengine-utils/util-os.c
    )

add_executable(testdengine-util-rng
        src/testdengine-utils/util-rng.c
    )

#core
add_executable(testdengine-window-basic
        src/testdengine-window/window-basic.c)

add_executable(testdengine-window-colored
        src/testdengine-window/window-colored.c
        )

add_executable(testdengine-2d-triangle
        src/testdengine-triangle/2d-triangle.c
        )

add_executable(testdengine-2d-triangle-textured
        src/testdengine-triangle/2d-triangle-textured.c
        )

add_executable(testdengine-3d-triangle-textured
        src/testdengine-triangle/3d-triangle-textured.c
        )

add_executable(testdengine-2d-quad-textured
        src/testdengine-quad/2d-quad-textured.c
        )

#<<<---link--->>>
#core
target_link_libraries(testdengine-window-basic
        dengine-core dengine-utils)

target_link_libraries(testdengine-window-colored
        dengine-core dengine-utils)

target_link_libraries(testdengine-2d-triangle
        dengine-core dengine-utils)

target_link_libraries(testdengine-2d-triangle-textured
        dengine-core dengine-utils)

target_link_libraries(testdengine-3d-triangle-textured
        dengine-core dengine-utils)

target_link_libraries(testdengine-2d-quad-textured
        dengine-core dengine-utils)

#util
target_link_libraries(testdengine-util-confserialize
        dengine-utils)

target_link_libraries(testdengine-util-filesys
        dengine-utils)

target_link_libraries(testdengine-util-os
        dengine-utils)

target_link_libraries(testdengine-util-rng
        dengine-utils)

#c99
set_property(TARGET testdengine-window-basic PROPERTY C_STANDARD 99)
set_property(TARGET testdengine-window-colored PROPERTY C_STANDARD 99)
set_property(TARGET testdengine-2d-triangle PROPERTY C_STANDARD 99)
set_property(TARGET testdengine-2d-triangle-textured PROPERTY C_STANDARD 99)
set_property(TARGET testdengine-2d-quad-textured PROPERTY C_STANDARD 99)
set_property(TARGET testdengine-3d-triangle-textured PROPERTY C_STANDARD 99)
set_property(TARGET testdengine-util-confserialize PROPERTY C_STANDARD 99)
set_property(TARGET testdengine-util-filesys PROPERTY C_STANDARD 99)
set_property(TARGET testdengine-util-os PROPERTY C_STANDARD 99)
set_property(TARGET testdengine-util-rng PROPERTY C_STANDARD 99)




