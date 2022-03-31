# render some text in < 10 lines
add_executable(testdengine-dengine-verybasic
        dengine-verybasic.c)
target_link_libraries(testdengine-dengine-verybasic
        ${dengine})
set_property(TARGET testdengine-dengine-verybasic PROPERTY C_STANDARD 99)
