#input basics
add_executable(testdengine-input-basic
        input-basic.c
        )
target_link_libraries(testdengine-input-basic
        dengine-core dengine-utils)
set_property(TARGET testdengine-input-basic PROPERTY C_STANDARD 99)
