# test ecs
add_executable(testdengine-scene-ecs
        scene-ecs.c)
target_link_libraries(testdengine-scene-ecs
        dengine-core dengine-utils dengine-gui dengine-scene dengine-model)
set_property(TARGET testdengine-scene-ecs PROPERTY C_STANDARD 99)

