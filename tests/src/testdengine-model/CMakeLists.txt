# load wavefront obj file format
add_executable(testdengine-model-obj
        model-obj.c)
target_link_libraries(testdengine-model-obj
        dengine-core dengine-utils dengine-gui dengine-model)
set_property(TARGET testdengine-model-obj PROPERTY C_STANDARD 99)

