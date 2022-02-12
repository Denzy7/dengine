#utils

#confserialize
add_executable(testdengine-util-confserialize
        src/testdengine-utils/util-confserialize.c
    )
target_link_libraries(testdengine-util-confserialize
        dengine-utils)
set_property(TARGET testdengine-util-confserialize PROPERTY C_STANDARD 99)


#filesystem
add_executable(testdengine-util-filesys
        src/testdengine-utils/util-filesys.c
    )
target_link_libraries(testdengine-util-filesys
        dengine-utils)
set_property(TARGET testdengine-util-filesys PROPERTY C_STANDARD 99)


#os specifics
add_executable(testdengine-util-os
        src/testdengine-utils/util-os.c
    )
target_link_libraries(testdengine-util-os
        dengine-utils)
set_property(TARGET testdengine-util-os PROPERTY C_STANDARD 99)

#rng util
add_executable(testdengine-util-rng
        src/testdengine-utils/util-rng.c
    )
target_link_libraries(testdengine-util-rng
        dengine-utils)
set_property(TARGET testdengine-util-rng PROPERTY C_STANDARD 99)

#timer util
add_executable(testdengine-util-timer
        src/testdengine-utils/util-timer.c
    )
target_link_libraries(testdengine-util-timer
        dengine-utils m)
set_property(TARGET testdengine-util-timer PROPERTY C_STANDARD 99)

#strpatch util
add_executable(testdengine-util-strpatch
        src/testdengine-utils/util-strpatch.c
    )
target_link_libraries(testdengine-util-strpatch
        dengine-utils)
set_property(TARGET testdengine-util-strpatch PROPERTY C_STANDARD 99)


#executables for desktop
if(NOT ANDROID)    
#core
#basic window
add_executable(testdengine-window-basic
        src/testdengine-window/window-basic.c)
target_link_libraries(testdengine-window-basic
        dengine-core dengine-utils)
set_property(TARGET testdengine-window-basic PROPERTY C_STANDARD 99)

#window with glClearColor
add_executable(testdengine-window-colored
        src/testdengine-window/window-colored.c
        )
target_link_libraries(testdengine-window-colored
        dengine-core dengine-utils)
set_property(TARGET testdengine-window-colored PROPERTY C_STANDARD 99)

#input basics
add_executable(testdengine-input-basic
        src/testdengine-input/input-basic.c
        )
target_link_libraries(testdengine-input-basic
        dengine-core dengine-utils)
set_property(TARGET testdengine-input-basic PROPERTY C_STANDARD 99)

#an isosceles triangle
add_executable(testdengine-2d-triangle
        src/testdengine-triangle/2d-triangle.c
        )
target_link_libraries(testdengine-2d-triangle
        dengine-core dengine-utils)
set_property(TARGET testdengine-2d-triangle PROPERTY C_STANDARD 99)

#an isosceles triangle textured
add_executable(testdengine-2d-triangle-textured
        src/testdengine-triangle/2d-triangle-textured.c
        )
target_link_libraries(testdengine-2d-triangle-textured
        dengine-core dengine-utils)
set_property(TARGET testdengine-2d-triangle-textured PROPERTY C_STANDARD 99)

#an isosceles triangle textured + is 3D
add_executable(testdengine-3d-triangle-textured
        src/testdengine-triangle/3d-triangle-textured.c
        )
target_link_libraries(testdengine-3d-triangle-textured
        dengine-core dengine-utils)
set_property(TARGET testdengine-3d-triangle-textured PROPERTY C_STANDARD 99)

#a 4 point-quad covering the whole screen
add_executable(testdengine-2d-quad
        src/testdengine-quad/2d-quad.c
        )
target_link_libraries(testdengine-2d-quad
        dengine-core dengine-utils)
set_property(TARGET testdengine-2d-quad PROPERTY C_STANDARD 99)

#a 4 point-quad covering the whole screen textured
add_executable(testdengine-2d-quad-textured
        src/testdengine-quad/2d-quad-textured.c
        )
target_link_libraries(testdengine-2d-quad-textured
        dengine-core dengine-utils)
set_property(TARGET testdengine-2d-quad-textured PROPERTY C_STANDARD 99)


#a 3d plane
add_executable(testdengine-3d-plane
        src/testdengine-plane/3d-plane.c
        )
target_link_libraries(testdengine-3d-plane
        dengine-core dengine-utils)
set_property(TARGET testdengine-3d-plane PROPERTY C_STANDARD 99)

#a 3d plane textured
add_executable(testdengine-3d-plane-textured
        src/testdengine-plane/3d-plane-textured.c
        )
target_link_libraries(testdengine-3d-plane-textured
        dengine-core dengine-utils)
set_property(TARGET testdengine-3d-plane-textured PROPERTY C_STANDARD 99)

#a 3d cube
add_executable(testdengine-3d-cube
        src/testdengine-cube/3d-cube.c
        )
target_link_libraries(testdengine-3d-cube
        dengine-core dengine-utils)
set_property(TARGET testdengine-3d-cube PROPERTY C_STANDARD 99)

#a textured 3d cube
add_executable(testdengine-3d-cube-textured
        src/testdengine-cube/3d-cube-textured.c
        )
target_link_libraries(testdengine-3d-cube-textured
        dengine-core dengine-utils)
set_property(TARGET testdengine-3d-cube-textured PROPERTY C_STANDARD 99)

#text w/ window coordinates
add_executable(testdengine-gui-textwindow
        src/testdengine-gui/textwindow.c
        )
target_link_libraries(testdengine-gui-textwindow
        dengine-core dengine-utils dengine-gui)
set_property(TARGET testdengine-gui-textwindow PROPERTY C_STANDARD 99)

#simple textured panels + some text
add_executable(testdengine-gui-panel
        src/testdengine-gui/panel.c
        )
target_link_libraries(testdengine-gui-panel
        dengine-core dengine-utils dengine-gui)
set_property(TARGET testdengine-gui-panel PROPERTY C_STANDARD 99)

#its a panel with text and input checks
add_executable(testdengine-gui-button
        src/testdengine-gui/button.c
        )
target_link_libraries(testdengine-gui-button
        dengine-core dengine-utils dengine-gui)
set_property(TARGET testdengine-gui-button PROPERTY C_STANDARD 99)


#a scene lit with normals only
add_executable(testdengine-lighting-normals
        src/testdengine-lighting/lighting-normals.c
        )
target_link_libraries(testdengine-lighting-normals
        dengine-core dengine-utils)
set_property(TARGET testdengine-lighting-normals PROPERTY C_STANDARD 99)

#a scene lit with normals + dirlight and shadow, gui debugs shadowmap
add_executable(testdengine-lighting-dirlight
        src/testdengine-lighting/lighting-dirlight.c
        )
target_link_libraries(testdengine-lighting-dirlight
        dengine-core dengine-utils dengine-gui)
set_property(TARGET testdengine-lighting-dirlight PROPERTY C_STANDARD 99)

# sliced 3d grid
add_executable(testdengine-tooling-3dgrid
        src/testdengine-tooling/3d-grid.c)
target_link_libraries(testdengine-tooling-3dgrid
        dengine-core dengine-utils)
set_property(TARGET testdengine-tooling-3dgrid PROPERTY C_STANDARD 99)

# 3d axis at 0,0,0
add_executable(testdengine-tooling-3daxis
        src/testdengine-tooling/3d-axis.c)
target_link_libraries(testdengine-tooling-3daxis
        dengine-core dengine-utils)
set_property(TARGET testdengine-tooling-3daxis PROPERTY C_STANDARD 99)

endif()









