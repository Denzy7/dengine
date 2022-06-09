# sliced 3d grid
add_executable(testdengine-tooling-3dgrid
        3d-grid.c)
target_link_libraries(testdengine-tooling-3dgrid
        dengine-core dengine-utils)
set_property(TARGET testdengine-tooling-3dgrid PROPERTY C_STANDARD 99)

# 3d axis at 0,0,0
add_executable(testdengine-tooling-3daxis
        3d-axis.c)
target_link_libraries(testdengine-tooling-3daxis
        dengine-core dengine-utils)
set_property(TARGET testdengine-tooling-3daxis PROPERTY C_STANDARD 99)

# pick something with framebuffer
add_executable(testdengine-tooling-fbpick
        fbpick.c)
target_link_libraries(testdengine-tooling-fbpick
        dengine-core dengine-utils dengine-gui)
set_property(TARGET testdengine-tooling-fbpick PROPERTY C_STANDARD 99)
if(UNIX)
    target_link_libraries(testdengine-tooling-fbpick m)
endif()