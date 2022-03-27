# test foward camera
add_executable(testdengine-camera-foward
        camera-foward.c)
target_link_libraries(testdengine-camera-foward
        dengine-core dengine-utils dengine-gui)
set_property(TARGET testdengine-camera-foward PROPERTY C_STANDARD 99)
